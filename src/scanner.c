/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file scanner.c
 *     Ingame scanner (minimap/radar) support.
 * @par Purpose:
 *     Implement functions for handling the scanner map and its state.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "scanner.h"

#include <stdlib.h>
#include "bfgentab.h"
#include "bfmath.h"
#include "bfmemut.h"
#include "bfpalette.h"
#include "bfplanar.h"
#include "bfscreen.h"
#include "bfutility.h"

#include "app_sprite.h"
#include "engincolour.h"
#include "enginsngtxtr.h"

#include "bigmap.h"
#include "campaign.h"
#include "building.h"
#include "keyboard.h"
#include "player.h"
#include "thing.h"
#include "thing_search.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "hud_panel.h"
#include "lvobjctv.h"
#include "scandraw.h"
#include "swlog.h"
#include "enginsngobjs.h"
/******************************************************************************/
#pragma pack(1)

#define ARC_ANGLE 150

struct BbpAdds {
    s32 du;
    s32 dv;
};

struct rectangle { // sizeof=4
    ubyte x1;
    ubyte y1;
    ubyte x2;
    ubyte y2;
};

#pragma pack()

#define scanner_map_to_mapcoord(c) (c << 7)
#define mapcoord_to_scanner_map(c) (c >> 7)

/******************************************************************************/

/** String of keycodes for changing arrow mode.
 * Cannot be of type TbKeyCode as contains plain numeric value as well.
 */
const int scanner_arrow_mode_code_keys[] = {
    KC_NUMPAD3, KC_DECIMAL, KC_NUMPAD1, KC_NUMPAD4,
    KC_NUMPAD1, KC_NUMPAD5, KC_NUMPAD9, KC_NUMPAD2,
    KC_NUMPAD6, KC_NUMPAD5, KC_NUMPAD3, KC_NUMPAD5,
    9999,
};

s32 scanner_next_key_no;

extern ushort signal_count;
extern ulong turn_last; // = 999;
extern ulong SCANNER_keep_arcs;
extern struct BbpAdds SCANNER_bbpadds[SCANNER_BBP_ADDS_COUNT];

ushort SCANNER_base_zoom_factor = 180;
ushort SCANNER_user_zoom_factor = 192;
ubyte SCANNER_scale_dots = true;

s32 scanner_arrow_mode = 1;

extern struct rectangle redo_scanner[128];

extern s32 redo_scanner_last;
extern s32 redo_scanner_first;

/******************************************************************************/

void SCANNER_set_zoom(int zoom)
{
    if (zoom < 8)
        ingame.Scanner.Zoom = 8;
    else if (zoom > 556)
        ingame.Scanner.Zoom = 556;
    else
        ingame.Scanner.Zoom = zoom;
}

void SCANNER_init_bbpoints(void)
{
    int angle, k;
    int i;

    k = 0;
    for (i = 0; i < SCANNER_BBP_ADDS_COUNT; i++, k += 2048)
    {
        angle = (k >> 4);
        SCANNER_bbpadds[i].du = lbSinTable[angle] >> 2;
        SCANNER_bbpadds[i].dv = lbSinTable[angle + 512] >> 2;
    }
}

void SCANNER_process_special_input(void)
{
    ushort sckey, nxkey;

    sckey = scanner_arrow_mode_code_keys[scanner_next_key_no];
    if (is_key_pressed(sckey, KMod_DONTCARE))
    {
        clear_key_pressed(sckey);
        nxkey = scanner_arrow_mode_code_keys[++scanner_next_key_no];
        if (nxkey == 9999)
        {
            scanner_next_key_no = 0;
            scanner_arrow_mode = scanner_arrow_mode ^ 1;
        }
    }
}

void SCANNER_process_bbpoints(void)
{
    int i;

    for (i = 0; i < SCANNER_BIG_BLIP_COUNT; i++)
    {
        struct BigBlip *p_bbl;
        int pt_base;
        ubyte counter;
        int k;

        p_bbl = &ingame.Scanner.BigBlip[i];
        if (p_bbl->Period == 0)
            continue;

        pt_base = i * SCANNER_BBP_ADDS_COUNT;
        counter = p_bbl->Counter + 1;
        p_bbl->Counter = counter;

        if ((counter == p_bbl->Period) && (i == SCANNER_BIG_BLIP_COUNT - 1))
        {
            // The last blip slot is disabled once its cycle completes
            p_bbl->Period = 0;
        } else
        if (counter >= p_bbl->Period)
        {
            // Cycle completed (or overrun) - reposition all of this blip's
            // points to the blip's current location, and rebase the counter.
            for (k = 0; k < SCANNER_BBP_ADDS_COUNT; k++)
            {
                SCANNER_bbpoint[pt_base + k].X = p_bbl->Z;
                SCANNER_bbpoint[pt_base + k].Z = p_bbl->X;
            }
            p_bbl->Counter -= p_bbl->Period;
        }

        // Spread the points further apart, at the speed set for this blip.
        for (k = 0; k < SCANNER_BBP_ADDS_COUNT; k++)
        {
            struct SimplePoint *p_pt;

            p_pt = &SCANNER_bbpoint[pt_base + k];
            p_pt->X += SCANNER_bbpadds[k].du << p_bbl->Speed;
            p_pt->Z += SCANNER_bbpadds[k].dv << p_bbl->Speed;
        }
    }
}

void SCANNER_clear(void)
{
    LbMemorySet(SCANNER_data, SCANNER_colour[0], sizeof(SCANNER_data));
}

void SCANNER_init_people_colours(void)
{
    SCANNER_people_colours[SubTT_PERS_AGENT] =
      pixmap.fade_table[24 * PALETTE_8b_COLORS + colour_lookup[ColLU_RED]];
    SCANNER_people_colours[SubTT_PERS_MERCENARY] =
      pixmap.fade_table[10 * PALETTE_8b_COLORS + colour_lookup[ColLU_PINK]];
    SCANNER_people_colours[SubTT_PERS_POLICE] =
      pixmap.fade_table[40 * PALETTE_8b_COLORS + colour_lookup[ColLU_BLUE]];
    SCANNER_people_colours[SubTT_PERS_SCIENTIST] =
      pixmap.fade_table[24 * PALETTE_8b_COLORS + colour_lookup[ColLU_YELLOW]];
    SCANNER_people_colours[SubTT_PERS_PUNK_F] =
      pixmap.fade_table[24 * PALETTE_8b_COLORS + colour_lookup[ColLU_GREEN]];
    SCANNER_people_colours[SubTT_PERS_PUNK_M] = SCANNER_people_colours[SubTT_PERS_PUNK_F];
    SCANNER_people_colours[SubTT_PERS_ZEALOT] =
      pixmap.fade_table[32 * PALETTE_8b_COLORS + colour_lookup[ColLU_WHITE]];
    SCANNER_people_colours[SubTT_PERS_HIGH_PRIEST] = SCANNER_people_colours[SubTT_PERS_ZEALOT];
    SCANNER_people_colours[SubTT_PERS_MECH_SPIDER] =  SCANNER_people_colours[SubTT_PERS_ZEALOT];
    SCANNER_people_colours[SubTT_PERS_SHADY_M] =
      pixmap.fade_table[40 * PALETTE_8b_COLORS + colour_lookup[ColLU_GREYMD]];
    SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M] =
      pixmap.fade_table[32 * PALETTE_8b_COLORS + colour_lookup[ColLU_GREYMD]];
    SCANNER_people_colours[SubTT_PERS_WHITE_BRUN_F] =  SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
    SCANNER_people_colours[SubTT_PERS_WHIT_BLOND_F] = SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
    SCANNER_people_colours[SubTT_PERS_LETH_JACKT_M] = SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
}

void SCANNER_init(void)
{
#if 0
    asm volatile ("call ASM_SCANNER_init\n"
        :  :  : "eax" );
#else
    LowTransGrey_InitPaletteBright();
    SCANNER_init_bbpoints();
    LowTransGrey_InitBrightLimitTable();
    SCANNER_init_people_colours();
#endif
}

void SCANNER_set_colours(struct PanelStyle *p_style)
{
    TbPixel bcol1;

    bcol1 = p_style->Colours[PanColr_Liquid];
    SCANNER_colour[ScnClr_Text] = p_style->Colours[PanColr_Text];
    SCANNER_colour[ScnClr_Roadway] = p_style->Colours[PanColr_Roadway];
    SCANNER_colour[ScnClr_LiquidDk] = pixmap.fade_table[10 * PALETTE_8b_COLORS + bcol1];
    SCANNER_colour[ScnClr_Outline] = p_style->Colours[PanColr_Outline];
    SCANNER_colour[ScnClr_Frame] = p_style->Colours[PanColr_Frame];
}

void SCANNER_fe_process_turn(void)
{
    SCANNER_process_bbpoints();
}

void SCANNER_process_turn(void)
{
    SCANNER_process_special_input();
    SCANNER_process_bbpoints();
}

/** Fill the scanner map for a single scanline of a rasterized triangle.
 *
 *  Coordinates are given in 16.16 fixed point; the left boundary is excluded
 *  and the right boundary is included (so adjoining triangles sharing an
 *  edge don't double-draw it).
 */
static void SCANNER_scanconvert_fill_row(int left_fx, int right_fx, int row, ubyte colour)
{
    int left = left_fx >> 16;
    int right = right_fx >> 16;
    int col;

    if (right <= left)
        return;
    for (col = left + 1; col <= right; col++)
        SCANNER_data[col - 1][row] = colour;
}

/** Fills in a triangle/quad shape on the scanner map, using the map-projected
 *  coordinates of the given 3 or 6 points.
 */
void SCANNER_scanconvert(int x0, int y0, int x1, int y1, int x2, int y2, int colour)
{
#if 0
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[3];

    stkargs[0] = (int)(intptr_t)x2;
    stkargs[1] = (int)(intptr_t)y2;
    stkargs[2] = (int)(intptr_t)colour;

    asm volatile (
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_SCANNER_scanconvert\n"
        : : "a" (x0), "d" (y0), "b" (x1), "c" (y1), "S" (stkargs)
        : "cc", "memory");
    return;
#endif
    int total_height, dx02, long_slope;
    int long_tracker, other_tracker;
    int row;

    if ((x0 < 0) || (x0 > 256) || (x1 < 0) || (x1 > 256) || (x2 < 0) || (x2 > 256)
     || (y0 < 0) || (y0 > 256) || (y1 < 0) || (y1 > 256) || (y2 < 0) || (y2 > 256)) {
        LOGWARN("cannot draw scanner triangle outside of map bounds: (%d,%d), (%d,%d), (%d,%d)",
          x0, y0, x1, y1, x2, y2);
        return;
    }

    // Sort the 3 points by Y ascending (compare-exchange network of 3).
    if (y0 > y1) {
        int tx = x0, ty = y0;
        x0 = x1; y0 = y1;
        x1 = tx; y1 = ty;
    }
    if (y1 > y2) {
        int tx = x1, ty = y1;
        x1 = x2; y1 = y2;
        x2 = tx; y2 = ty;
    }
    if (y0 > y1) {
        int tx = x0, ty = y0;
        x0 = x1; y0 = y1;
        x1 = tx; y1 = ty;
    }

    if (y0 == y1)
    {
        int dx12, edge_slope;

        if (y1 == y2)
            return; // fully degenerate (zero-height) triangle

        total_height = y2 - y0;
        dx02 = x2 - x0;
        long_slope = (dx02 << 16) / total_height;
        dx12 = x2 - x1;
        edge_slope = (dx12 << 16) / total_height;

        if (x0 < x1) {
            long_tracker = x0 << 16;
            other_tracker = x1 << 16;
            for (row = y0; row < y2; row++) {
                SCANNER_scanconvert_fill_row(long_tracker, other_tracker, row, colour);
                long_tracker += long_slope;
                other_tracker += edge_slope;
            }
        } else {
            long_tracker = x1 << 16;
            other_tracker = x0 << 16;
            for (row = y0; row < y2; row++) {
                SCANNER_scanconvert_fill_row(long_tracker, other_tracker, row, colour);
                other_tracker += long_slope;
                long_tracker += edge_slope;
            }
        }
        return;
    }

    total_height = y2 - y0;
    dx02 = x2 - x0;
    long_slope = (dx02 << 16) / total_height;

    {
        int height_top, dx01, short_slope_top;
        int short_tracker;
        TbBool long_is_left;

        height_top = y1 - y0;
        dx01 = x1 - x0;
        short_slope_top = (dx01 << 16) / height_top;

        long_tracker = x0 << 16;
        short_tracker = x0 << 16;
        long_is_left = (long_slope < short_slope_top);

        if (long_is_left) {
            for (row = y0; row < y1; row++) {
                SCANNER_scanconvert_fill_row(long_tracker, short_tracker, row, (ubyte)colour);
                long_tracker += long_slope;
                short_tracker += short_slope_top;
            }
        } else {
            for (row = y0; row < y1; row++) {
                SCANNER_scanconvert_fill_row(short_tracker, long_tracker, row, (ubyte)colour);
                short_tracker += short_slope_top;
                long_tracker += long_slope;
            }
        }

        if (y1 == y2)
            return; // flat-bottom triangle, done after the top phase

        {
            int height_bottom, dx12, short_slope_bottom;
            int short_tracker2;

            height_bottom = y2 - y1;
            dx12 = x2 - x1;
            short_slope_bottom = (dx12 << 16) / height_bottom;
            short_tracker2 = x1 << 16;

            if (long_is_left) {
                for (row = y1; row < y2; row++) {
                    SCANNER_scanconvert_fill_row(long_tracker, short_tracker2, row, (ubyte)colour);
                    long_tracker += long_slope;
                    short_tracker2 += short_slope_bottom;
                }
            } else {
                for (row = y1; row < y2; row++) {
                    SCANNER_scanconvert_fill_row(short_tracker2, long_tracker, row, (ubyte)colour);
                    short_tracker2 += short_slope_bottom;
                    long_tracker += long_slope;
                }
            }
        }
    }
}

void SCANNER_fill_triangle(int z1, int x1, int z2, int x2, int z3, int x3, TbPixel colour)
{
    SCANNER_scanconvert(mapcoord_to_scanner_map(z1), mapcoord_to_scanner_map(x1),
      mapcoord_to_scanner_map(z2), mapcoord_to_scanner_map(x2),
      mapcoord_to_scanner_map(z3), mapcoord_to_scanner_map(x3), colour);
}

/** A quad face is filled in as two triangles: {0,1,2} and {1,2,3}.
 */
void SCANNER_fill_quad(int z1, int x1, int z2, int x2, int z3, int x3, int z4, int x4, TbPixel colour)
{
    SCANNER_scanconvert(mapcoord_to_scanner_map(z1), mapcoord_to_scanner_map(x1),
      mapcoord_to_scanner_map(z2), mapcoord_to_scanner_map(x2),
      mapcoord_to_scanner_map(z3), mapcoord_to_scanner_map(x3), colour);
    SCANNER_scanconvert(mapcoord_to_scanner_map(z2), mapcoord_to_scanner_map(x2),
      mapcoord_to_scanner_map(z3), mapcoord_to_scanner_map(x3),
      mapcoord_to_scanner_map(z4), mapcoord_to_scanner_map(x4), colour);
}

void SCANNER_fill_in_object(int object_idx, TbPixel colour)
{
    struct SingleObject *p_obj;
    int i;

    p_obj = &game_objects[object_idx];

    for (i = p_obj->StartFace; i < p_obj->StartFace + p_obj->NumbFaces; i++)
    {
        struct SingleObjectFace3 *p_face;
        int map_x[3], map_z[3];
        int k;

        p_face = &game_object_faces3[i];

        for (k = 0; k < 3; k++)
        {
            struct SinglePoint *p_pt;
            p_pt = &game_object_points[p_face->PointNo[k]];
            map_x[k] = p_pt->X + p_obj->MapX;
            //map_y[k] = (p_pt->Y + p_obj->OffsetY) >> 3; // unused
            map_z[k] = p_pt->Z + p_obj->MapZ;
        }
        SCANNER_fill_triangle(map_z[0], map_x[0], map_z[1], map_x[1], map_z[2], map_x[2], colour);
    }

    for (i = p_obj->StartFace4; i < p_obj->StartFace4 + p_obj->NumbFaces4; i++)
    {
        struct SingleObjectFace4 *p_face4;
        int map_x[4], map_z[4];
        int k;

        p_face4 = &game_object_faces4[i];

        for (k = 0; k < 4; k++)
        {
            struct SinglePoint *p_pt;
            p_pt = &game_object_points[p_face4->PointNo[k]];
            map_x[k] = p_pt->X + p_obj->MapX;
            map_z[k] = p_pt->Z + p_obj->MapZ;
        }
        SCANNER_fill_quad(map_z[0], map_x[0], map_z[1], map_x[1],
          map_z[2], map_x[2], map_z[3], map_x[3], colour);
    }
}

void SCANNER_fill_in_road(int thing_idx)
{
#if 0
    asm volatile ("call ASM_SCANNER_fill_in_road\n"
        : : "a" (thing_idx) : );
    return;
#endif
    struct Thing *p_thing;
    int off_x, off_z, off_x_scaled, off_z_scaled;
    int x8, z8, x_minus, x_plus, z_minus, z_plus;
    int p1z, p1x, p2z, p2x, p3z, p3x, p4z, p4x;
    TbPixel colour;

    p_thing = &things[thing_idx];
    x8 = PRCCOORD_TO_MAPCOORD(p_thing->X);
    z8 = PRCCOORD_TO_MAPCOORD(p_thing->Z);
    off_x = p_thing->U.UObject.OffX;
    off_z = p_thing->U.UObject.OffZ;
    // Strength-reduced form of off_x*3.5 / off_z*3.5,
    // arithmetic shifts round toward -inf.
    off_x_scaled = off_x + (off_x * 2 + (off_x >> 1));
    off_z_scaled = off_z + (off_z * 2 + (off_z >> 1));

    x_minus = x8 - off_x * 8;
    x_plus = x8 + off_x * 8;
    z_minus = z8 - off_z * 8;
    z_plus = z8 + off_z * 8;

    p1z = z_minus + off_x_scaled;
    p1x = x_minus - off_z_scaled;
    p2z = z_minus - off_x_scaled;
    p2x = x_minus + off_z_scaled;
    p3z = z_plus + off_x_scaled;
    p3x = x_plus - off_z_scaled;
    p4z = z_plus - off_x_scaled;
    p4x = x_plus + off_z_scaled;

    colour = SCANNER_colour[ScnClr_Roadway];

    SCANNER_fill_quad(p1z, p1x, p2z, p2x, p3z, p3x, p4z, p4x, colour);
}

/** Use border detection filter on buildings.
 *
 * Replaces pixels surrounded by a specific outline colour with
 * the ground colour instead.
 */
void SCANNER_outline(void)
{
#if 0
    asm volatile ("call ASM_SCANNER_outline\n"
        : : : "eax" );
    return;
#endif
    ubyte *const map = &SCANNER_data[0][0];
    ubyte scratch[256];
    ubyte outline_colour;
    ubyte text_colour;
    int row, col;

    // The decided value for a row is written one row late (buffered
    // in `scratch[]`), so a row's own neighbour tests can still see
    // the not-yet-overwritten value of the row above it.
    outline_colour = SCANNER_colour[ScnClr_Outline];
    text_colour = SCANNER_colour[ScnClr_Text];

    // Seed `scratch[]` with the decided values for row 1.
    row = 1;
    {
        for (col = 1; col < 255; col++)
        {
            uint off;
            TbBool erode;

            off = (row << 8) + col + 1;
            erode = (map[off] == outline_colour) &&
              (map[off + 256 - 1] == outline_colour) &&
              (map[off - 256 - 1] == outline_colour) &&
              (map[off + 1] == outline_colour) &&
              (map[off - 1] == outline_colour);

            scratch[col] = erode ? text_colour : map[off];
        }
    }

    // Sweep rows 2..255; decide the thinned value for every column
    // of the current row (buffering it in `scratch[]`), while
    // flushing the previous row's already-decided value into the map.
    for (row = 2; row < 256; row++)
    {
        for (col = 1; col < 255; col++)
        {
            uint off;
            ubyte flushed;
            TbBool erode;

            off = (row << 8) + col;
            erode = (map[off] == outline_colour) &&
              (map[off + 256 + 1] == outline_colour) &&
              (map[off - 256] == outline_colour) &&
              (map[off + 1] == outline_colour) &&
              (map[off - 1] == outline_colour);

            flushed = scratch[col];
            map[off - 256] = flushed;
            scratch[col] = erode ? text_colour : map[off];
        }
    }

    // Flush the final buffered row.
    row = 255;
    {
        for (col = 1; col < 255; col++)
        {
            uint off;

            off = (row << 8) + col;
            map[off] = scratch[col];
        }
    }
}

//TODO why coordinates are backward?
int SCANNER_find_colour(int mapx, int mapy)
{
#if 0
    int ret;
    asm volatile ("call ASM_SCANNER_find_colour\n"
        : "=r" (ret) : "a" (mapx), "d" (mapy));
    return ret;
#endif
    ushort textr;
    ubyte txtr_bits, tlpos_bits;
    int result;

    tlpos_bits =  (mapx >> 6) & 2;
    tlpos_bits |= (mapy >> 7) & 1;
    textr = floor_texture_at_point(mapy, mapx);
    txtr_bits = get_my_texture_bits(textr);

    switch (tlpos_bits)
    {
    case 0:
        if ((txtr_bits & 0x40) != 0)
            result = 1;
        else if ((txtr_bits & 0x80) != 0)
            result = 2;
        else
            result = 0;
        break;
    case 1:
        if ((txtr_bits & 0x10) != 0)
          result = 1;
        else if ((txtr_bits & 0x20) != 0)
            result = 2;
        else
            result = 0;
        break;
    case 2:
        if ((txtr_bits & 0x01) != 0)
            result = 1;
        else if ((txtr_bits & 0x02) != 0)
            result = 2;
        else
            result = 0;
        break;
    case 3:
        if ((txtr_bits & 0x04) != 0)
            result = 1;
        else if ((txtr_bits & 0x08) != 0)
            result = 2;
        else
            result = 0;
        break;
      default:
        result = 0;
        break;
    }
    return result;
}

static void SCANNER_fill_in_floor(int x1, int z1, int x2, int z2)
{
    int map_x, map_z;

    for (map_x = x1; map_x <= x2; map_x++)
    {
        for (map_z = z1; map_z <= z2; map_z++)
        {
            int alt1, alt2, alt3, alt4;
            int cor_x, cor_z;
            ushort sc_col;
            short brig;
            TbPixel col1;

            cor_x = scanner_map_to_mapcoord(map_x);
            cor_z = scanner_map_to_mapcoord(map_z);

            sc_col = SCANNER_find_colour(cor_x, cor_z);
            if (sc_col == 0)
                col1 = SCANNER_colour[ScnClr_Text];
            else if (sc_col == 1)
                col1 = SCANNER_colour[ScnClr_Roadway];
            else if (sc_col == 2)
                col1 = SCANNER_colour[ScnClr_LiquidDk];
            else {
                LOGWARN("invalid scanner colour found");
                col1 = SCANNER_colour[ScnClr_Outline];
            }

            alt1 = alt_at_point(cor_z, cor_x + 128);
            alt2 = alt_at_point(cor_z, cor_x - 128);
            alt3 = alt_at_point(cor_z + 128, cor_x);
            alt4 = alt_at_point(cor_z - 128, cor_x);
            brig = ((alt1 - alt2) >> 9) + ((alt3 - alt4) >> 9) + 32;
            if (brig < 0)
                brig = 0;
            if (brig > PALETTE_FADE_LEVELS - 1)
                brig = PALETTE_FADE_LEVELS - 1;

            SCANNER_data[map_x][map_z] = pixmap.fade_table[256 * brig + col1];
        }
    }
}

/** Draw scanner outlines for all buildings located on the map.
 */
static void SCANNER_fill_in_all_buildings(void)
{
    int tile_x, tile_z;

    for (tile_z = 0; tile_z < MAP_TILE_HEIGHT - 1; tile_z++)
    {
        for (tile_x = 0; tile_x < MAP_TILE_WIDTH - 1; tile_x++)
        {
            struct MyMapElement *p_mapel;
            ThingIdx thing;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            thing = p_mapel->Child;
            while (thing != 0)
            {
                if (thing <= 0) {
                    struct SimpleThing *p_sthing;

                    p_sthing = &sthings[thing];
                    thing = p_sthing->Next;
                } else {
                    struct Thing *p_thing;

                    p_thing = &things[thing];
                    if ((p_thing->Type == TT_BUILDING) && (p_thing->SubType != SubTT_BLD_BEZIER_ROAD))
                    {
                        int i;

                        for (i = 0; i < p_thing->U.UObject.NumbObjects; i++)
                        {
                            SCANNER_fill_in_object(p_thing->U.UObject.Object + i,
                              SCANNER_colour[ScnClr_Outline]);
                        }
                    }
                    thing = p_thing->Next;
                }
            }
        }
    }
}

/** Draw the scanner bezier roadway surfaces.
 */
static void SCANNER_fill_in_roadways(void)
{
    int tile_x, tile_z;

    for (tile_z = 0; tile_z < MAP_TILE_HEIGHT - 1; tile_z++)
    {
        for (tile_x = 0; tile_x < MAP_TILE_WIDTH - 1; tile_x++)
        {
            struct MyMapElement *p_mapel;
            ThingIdx thing;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            thing = p_mapel->Child;
            while (thing != 0)
            {
                if (thing <= 0) {
                    struct SimpleThing *p_sthing;

                    p_sthing = &sthings[thing];
                    thing = p_sthing->Next;
                } else {
                    struct Thing *p_thing;

                    p_thing = &things[thing];
                    if ((p_thing->Type == TT_BUILDING) && (p_thing->SubType == SubTT_BLD_BEZIER_ROAD))
                    {
                        SCANNER_fill_in_road(thing);
                    }
                    thing = p_thing->Next;
                }
            }
        }
    }
}

void SCANNER_fill_in(void)
{
#if 0
    asm volatile ("call ASM_SCANNER_fill_in\n"
        :  :  : "eax" );
    return;
#endif
    SCANNER_fill_in_floor(0, 0, 255, 255);
    SCANNER_fill_in_all_buildings();
    SCANNER_outline();
    SCANNER_fill_in_roadways();
}

void SCANNER_fill_in_a_little_bit(int x1, int z1, int x2, int z2)
{
#if 0
    asm volatile ("call ASM_SCANNER_fill_in_a_little_bit\n"
        : : "a" (x1), "d" (z1), "b" (x2), "c" (z2));
#endif
    if (x1 > x2) {
        return;
    }
    if (z1 > z2) {
        return;
    }

    SCANNER_fill_in_floor(x1, z1, x2, z2);
}

static int SCANNER_arcpoint_compute_mag(int x1, int z1, int x2, int z2)
{
    int dx, dz, abs_dx, abs_dz;
    int mag;

    dx = x2 - x1;
    dz = z2 - z1;
    abs_dx = abs(dx);
    abs_dz = abs(dz);

    // Fast alpha-max-plus-beta-min approximation of hypot(dx, dz):
    // mag ~= max*(1 - 1/32 - 1/128) + min*(1/4 + 1/8 + 1/64 + 1/128)
    if (abs_dx >= abs_dz) {
        mag = (abs_dx - (abs_dx >> 5) - (abs_dx >> 7))
            + (abs_dz >> 2) + (abs_dz >> 3) + (abs_dz >> 6) + (abs_dz >> 7);
    } else {
        mag = (abs_dz - (abs_dz >> 5) - (abs_dz >> 7))
            + (abs_dx >> 2) + (abs_dx >> 3) + (abs_dx >> 6) + (abs_dx >> 7);
    }

    mag <<= 7;
    mag >>= 8;

    return mag;
}

/** Spread the points linked to given arc across a small angle range.
 */
static void SCANNER_arcpoint_set_points(int arc_idx, int x1, int z1, int x2, int z2)
{
    int dx, dz;
    short angle;
    int base_i, k;

    dx = x2 - x1;
    dz = z2 - z1;

    angle = arctan(dx, dz);

    angle -= 2 * (ARC_ANGLE / SCANNER_POINTS_PER_ARC);
    base_i = arc_idx * SCANNER_POINTS_PER_ARC;
    for (k = 0; k < SCANNER_POINTS_PER_ARC; k++)
    {
        struct MovingPoint *p_pt;
        ushort widx;
        long sin_v, cos_v;

        p_pt = &SCANNER_arcpoint[base_i + k];
        widx = angle & (2 * LbFPMath_PI - 1);
        sin_v = lbSinTable[widx];
        cos_v = -lbSinTable[widx + LbFPMath_PI / 2];

        p_pt->X = x1;
        p_pt->Z = z1;
        p_pt->VelX = (sin_v * 0x200) >> 8;
        p_pt->VelZ = (cos_v * 0x200) >> 8;

        angle += ARC_ANGLE / SCANNER_POINTS_PER_ARC;
    }
}

/** Advance every arc point by its previously computed per-turn velocity.
 */
static void SCANNER_arcpoint_advance(int arc_idx)
{
    int base_i;
    int k;

    base_i = arc_idx * SCANNER_POINTS_PER_ARC;
    for (k = 0; k < SCANNER_POINTS_PER_ARC; k++)
    {
        struct MovingPoint *p_pt;

        p_pt = &SCANNER_arcpoint[base_i + k];
        p_pt->X += p_pt->VelX;
        p_pt->Z += p_pt->VelZ;
    }
}

static void SCANNER_arcpoint_restart(int arc_idx)
{
    struct Arc *p_arc;
    int mag;

    p_arc = &ingame.Scanner.Arc[arc_idx];

    mag = SCANNER_arcpoint_compute_mag(p_arc->X1, p_arc->Z1, p_arc->X2, p_arc->Z2);
    SCANNER_arcpoint_set_points(arc_idx, p_arc->X1, p_arc->Z1, p_arc->X2, p_arc->Z2);

    p_arc->Period = mag >> 16;
    p_arc->Counter = p_arc->Period;
    p_arc->ColourIsUnused = colour_lookup[1];
}

void SCANNER_process_arcpoints(void)
{
#if 0
    asm volatile (
      "call ASM_SCANNER_process_arcpoints\n"
        :  :  : "eax" );
    return;
#endif
    int arc_idx;

    dword_1DB1A0 = 0;
    for (arc_idx = 0; arc_idx < SCANNER_ARC_COUNT; arc_idx++)
    {
        struct Arc *p_arc;

        p_arc = &ingame.Scanner.Arc[arc_idx];
        if (p_arc->Counter == 0)
            continue;

        dword_1DB1A0++;

        SCANNER_arcpoint_advance(arc_idx);

        p_arc->Counter--;
        // If counter depleted, re-generate the arc points and restart the timer.
        if (p_arc->Counter == 0)
        {
            SCANNER_arcpoint_restart(arc_idx);
        }
    }
}

short SCANNER_init_arcpoint(int x1, int z1, int x2, int z2, int c)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_SCANNER_init_arcpoint\n"
        : : "a" (x1), "d" (z1), "b" (x2), "c" (z2), "g" (c));
    return -1;
#endif
    struct Arc *p_arc;
    int arc_idx;

    // Find a free arc slot (one currently not animating/counting down).
    p_arc = NULL;
    for (arc_idx = 0; arc_idx < SCANNER_ARC_COUNT; arc_idx++)
    {
        p_arc = &ingame.Scanner.Arc[arc_idx];

        if ((p_arc->Period == 0) && (p_arc->Counter == 0)) {
            break;
        }
    }
    if (arc_idx == SCANNER_ARC_COUNT) {
        return -1;
    }

    p_arc->X1 = x1;
    p_arc->Z1 = z1;
    p_arc->X2 = x2;
    p_arc->Z2 = z2;

    SCANNER_arcpoint_restart(arc_idx);

    return arc_idx;
}

void SCANNER_data_to_screen(void)
{
    SCANNER_fe_process_turn();
    SCANNER_fe_draw_solid();
}

void SCANNER_set_screen_box(short x, short y, short width, short height, short cutout)
{
    short i;
    short hlimit;

    hlimit = sizeof(ingame.Scanner.Width)/sizeof(ingame.Scanner.Width[0]);
    if (height >= hlimit)
        height = hlimit - 1;

    ingame.Scanner.X1 = x;
    ingame.Scanner.Y1 = y;
    ingame.Scanner.X2 = ingame.Scanner.X1 + width;
    ingame.Scanner.Y2 = ingame.Scanner.Y1 + height;

    if (cutout != 0)
    {
        for (i = 0; i + ingame.Scanner.Y1 <= ingame.Scanner.Y2; i++) {
            ingame.Scanner.Width[i] = min(width - cutout + i, width);
        }
    }
    else
    {
        for (i = 0; i + ingame.Scanner.Y1 <= ingame.Scanner.Y2; i++) {
            ingame.Scanner.Width[i] = width;
        }
    }
}

void SCANNER_update_arcpoint(ushort arc_no, short fromX, short fromZ, short toX, short toZ)
{
    if (arc_no >= SCANNER_ARC_COUNT)
        return;
    ingame.Scanner.Arc[arc_no].X1 = fromX;
    ingame.Scanner.Arc[arc_no].Z1 = fromZ;
    ingame.Scanner.Arc[arc_no].X2 = toX;
    ingame.Scanner.Arc[arc_no].Z2 = toZ;
}

void SCANNER_init_blippoint(ushort blip_no, int x, int z, int colour)
{
    if (blip_no >= SCANNER_BIG_BLIP_COUNT)
        return;
    ingame.Scanner.BigBlip[blip_no].X = x;
    ingame.Scanner.BigBlip[blip_no].Z = z;
    ingame.Scanner.BigBlip[blip_no].Speed = 4;
    ingame.Scanner.BigBlip[blip_no].Colour = colour;
    ingame.Scanner.BigBlip[blip_no].Period = 32;
}

void SCANNER_find_position(int x, int y, int *Ua, int *Vb)
{
#if 0
    asm volatile (
      "call ASM_SCANNER_find_position\n"
        : : "a" (x), "d" (y), "b" (Ua), "c" (Vb));
#endif
    struct TbPoint s1, s2, dt;
    int mz, mx, zoom, angle;
    int sin_z, cos_z;
    int half_w, half_h;
    int base_u, base_v;
    int raw_u, raw_v;

    s1.x = ingame.Scanner.X1;
    s1.y = ingame.Scanner.Y1;
    s2.x = ingame.Scanner.X2;
    s2.y = ingame.Scanner.Y2;
    mz = ingame.Scanner.MZ;
    mx = ingame.Scanner.MX;
    zoom = ingame.Scanner.Zoom;
    angle = ingame.Scanner.Angle;

    sin_z = (lbSinTable[angle] * zoom) >> 8;
    cos_z = (lbSinTable[angle + LbFPMath_PI/2] * zoom) >> 8;

    half_w = (s2.x - s1.x) >> 1;
    half_h = (s2.y - s1.y) >> 1;

    // Position of the scanner view center, rotated by scanner angle.
    base_u = (mz << 16) - half_w * sin_z + half_h * cos_z;
    base_v = (mx << 16) - half_w * cos_z - half_h * sin_z;

    dt.x = x - s1.x;
    dt.y = y - s1.y;

    raw_u = base_u + sin_z * dt.x - cos_z * dt.y;
    raw_v = base_v + cos_z * dt.x + sin_z * dt.y;

    raw_u = scanner_map_to_mapcoord(raw_u);
    raw_v = scanner_map_to_mapcoord(raw_v);

    // Rescale, rounding toward zero (as opposed to a plain
    // arithmetic shift, which would round toward -infinity).
    raw_u = (raw_u - ((raw_u >> 31) << 7)) >> 8;
    raw_v = (raw_v - ((raw_v >> 31) << 7)) >> 8;

    *Ua = raw_u >> 8;
    *Vb = raw_v >> 8;
}

TbBool mouse_move_over_scanner(void)
{
    short dx, dy;
    dx = lbDisplay.MMouseX - ingame.Scanner.X1;
    dy = lbDisplay.MMouseY - ingame.Scanner.Y1;

    return (dy >= 0) && (ingame.Scanner.Y1 + dy <= ingame.Scanner.Y2)
        && (dx >= 0) && (dx <= SCANNER_width[dy]);
}

ushort do_group_scanner(struct Objective *p_objectv, ushort next_signal)
{
    ushort n;
    ubyte colr;
    short dcthing;
    short nearthing;
    ushort group;
    int X, Z;

    group = p_objectv->Thing;
    dcthing = players[local_player_no].DirectControl[0];

    n = next_signal;
    // Find thing in group close to dcthing
    if (objective_target_is_ally(p_objectv)) {
        nearthing = find_nearest_from_group(&things[dcthing],group, 0);
        colr = colour_lookup[ColLU_WHITE];
    } else if (objective_target_is_to_be_acquired(p_objectv)) {
        nearthing = find_nearest_from_group(&things[dcthing], group, 1);
        colr = colour_lookup[ColLU_GREEN];
    } else {
        nearthing = find_nearest_from_group(&things[dcthing], group, 0);
        colr = colour_lookup[ColLU_RED];
    }
    ingame.Scanner.NearThing1 = nearthing;

    if (ingame.Scanner.NearThing1 != 0)
    {
        if (ingame.Scanner.NearThing1 <= 0) {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[ingame.Scanner.NearThing1];
            X = p_sthing->X;
            Z = p_sthing->Z;
        } else {
            struct Thing *p_thing;
            p_thing = &things[ingame.Scanner.NearThing1];
            X = p_thing->X;
            Z = p_thing->Z;
        }
        SCANNER_init_blippoint(n, X, Z, colr);
        n++;
        if (ingame.Scanner.GroupCount < SCANNER_GROUP_COUNT)
        {
            int sgroup;

            sgroup = ingame.Scanner.GroupCount;
            ++ingame.Scanner.GroupCount;
            ingame.Scanner.Group[sgroup] = group;
            ingame.Scanner.GroupCol[sgroup] = colr;
        }
    }
    return n;
}

ushort do_group_near_thing_scanner(struct Objective *p_objectv, ushort next_signal)
{
    ushort n;
    ubyte colr;
    short tgthing;
    short nearthing;
    ushort group;
    long X1, Z1;
    int X2, Z2;

    group = p_objectv->Arg2;
    tgthing = p_objectv->Thing;

    n = next_signal;
    // Find thing in group close to tgthing
    if (objective_target_is_ally(p_objectv)) {
        nearthing = find_nearest_from_group(&things[tgthing], group, 0);
        colr = colour_lookup[ColLU_WHITE];
    } else if (objective_target_is_to_be_acquired(p_objectv)) {
        nearthing = find_nearest_from_group(&things[tgthing], group, 1);
        colr = colour_lookup[ColLU_GREEN];
    } else {
        nearthing = find_nearest_from_group(&things[tgthing], group, 0);
        colr = colour_lookup[ColLU_RED];
    }
    ingame.Scanner.NearThing1 = nearthing;

    // Blip the target thing
    n = next_signal;
    if (tgthing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[tgthing];
        X1 = p_sthing->X;
        Z1 = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[tgthing];
        X1 = p_thing->X;
        Z1 = p_thing->Z;
    }
    SCANNER_init_blippoint(n, X1, Z1, colr);
    n++;

    // Arc the nearest group member
    if (nearthing != 0)
    {
        if (nearthing <= 0) {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[nearthing];
            X2 = p_sthing->X;
            Z2 = p_sthing->Z;
        } else {
            struct Thing *p_thing;
            p_thing = &things[nearthing];
            X2 = p_thing->X;
            Z2 = p_thing->Z;
        }
        if (dword_1DB1A0)
        {
            SCANNER_update_arcpoint(0, Z2, X2, Z1, X1);
        }
        else
        {
            if (panel_any_visible())
                SCANNER_init_arcpoint(Z2, X2, Z1, X1, 1);
        }
        SCANNER_keep_arcs = 1;

        n++;
        if (ingame.Scanner.GroupCount < SCANNER_GROUP_COUNT)
        {
            int sgroup;

            sgroup = ingame.Scanner.GroupCount;
            ++ingame.Scanner.GroupCount;
            ingame.Scanner.Group[sgroup] = group;
            ingame.Scanner.GroupCol[sgroup] = colr;
        }
    }
    return n;
}

ushort do_target_thing_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X, Z;
    ushort n;
    ubyte colr;
    ThingIdx thing;

    thing = p_objectv->Thing;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        X = p_sthing->X;
        Z = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        X = p_thing->X;
        Z = p_thing->Z;
    }
    SCANNER_init_blippoint(n, X, Z, colr);
    n++;
    return n;
}

ushort do_target_item_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X, Z;
    ushort n;
    ushort weapon;
    ubyte colr;
    ThingIdx thing;

    thing = p_objectv->Thing;
    weapon = p_objectv->Arg2;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    if (thing <= 0)
    {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        if ((p_sthing->Type == SmTT_DROPPED_ITEM) &&
          (p_sthing->U.UWeapon.WeaponType == weapon)) {
            X = p_sthing->X;
            Z = p_sthing->Z;
        } else if (p_sthing->Type == SmTT_CARRIED_ITEM) {
            struct Thing *p_person;
            p_person = &things[p_sthing->U.UWeapon.Owner];
            X = p_person->X;
            Z = p_person->Z;
        } else {
            struct Thing *p_person;
            ThingIdx person;
            person = find_person_carrying_weapon(weapon);
            p_person = &things[person];
            X = p_person->X;
            Z = p_person->Z;
        }
    }
    else
    {
        return n;
    }
    SCANNER_init_blippoint(n, X, Z, colr);
    n++;
    return n;
}

ushort do_thing_arrive_area_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X, Z;
    ushort n;
    ubyte colr;
    ThingIdx thing;

    thing = p_objectv->Thing;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    SCANNER_init_blippoint(n, MAPCOORD_TO_PRCCOORD(p_objectv->X,0), MAPCOORD_TO_PRCCOORD(p_objectv->Z,0), colr);
    n++;
    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        X = p_sthing->X;
        Z = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        X = p_thing->X;
        Z = p_thing->Z;
    }
    if (dword_1DB1A0)
    {
        SCANNER_update_arcpoint(0, Z, X,
          MAPCOORD_TO_PRCCOORD(p_objectv->Z,0),
          MAPCOORD_TO_PRCCOORD(p_objectv->X,0));
    }
    else
    {
        if (panel_any_visible())
            SCANNER_init_arcpoint(Z, X,
              MAPCOORD_TO_PRCCOORD(p_objectv->Z,0),
              MAPCOORD_TO_PRCCOORD(p_objectv->X,0), 1);
    }
    SCANNER_keep_arcs = 1;
    return n;
}

ushort do_thing_near_thing_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X1, Z1;
    long X2, Z2;
    ushort n;
    ubyte colr;
    ThingIdx thing1, thing2;

    thing1 = p_objectv->Thing;
    thing2 = p_objectv->Y;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    if (thing1 <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing1];
        X1 = p_sthing->X;
        Z1 = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing1];
        X1 = p_thing->X;
        Z1 = p_thing->Z;
    }
    SCANNER_init_blippoint(n, X1, Z1, colr);
    n++;
    if (thing2 <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing2];
        X2 = p_sthing->X;
        Z2 = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing2];
        X2 = p_thing->X;
        Z2 = p_thing->Z;
    }
    if (dword_1DB1A0)
    {
        SCANNER_update_arcpoint(0, Z2, X2, Z1, X1);
    }
    else
    {
        if (panel_any_visible())
            SCANNER_init_arcpoint(Z2, X2, Z1, X1, 1);
    }
    SCANNER_keep_arcs = 1;
    return n;
}

ushort do_group_arrive_area_scanner(struct Objective *p_objectv, ushort next_signal)
{
    ushort n;
    ubyte colr;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    SCANNER_init_blippoint(n, MAPCOORD_TO_PRCCOORD(p_objectv->X,0),
      MAPCOORD_TO_PRCCOORD(p_objectv->Z,0), colr);
    n++;
    return n;
}

void clear_all_scanner_signals(void)
{
    int n;

    signal_count = 0;
    for (n = 0; n < SCANNER_BIG_BLIP_COUNT; n++)
        ingame.Scanner.BigBlip[n].Period = 0;
    for (n = 0; n < SCANNER_ARC_COUNT; n++)
        ingame.Scanner.Arc[n].Period = 0;
}

void fill_blippoint_scanner(int x, int z, ubyte colour, ushort n)
{
    SCANNER_init_blippoint(n, x, z, colour);
    ingame.Scanner.BigBlip[n].Counter = ingame.Scanner.BigBlip[n].Period;
}

ushort do_netscan_blippoint_scanner(struct NetscanObjective *p_nsobv, ushort next_signal)
{
    int i;
    ushort n;

    n = next_signal;
    for (i = 0; i < NETSCAN_OBJECTIVE_POINTS; i++)
    {
        int x, z;

        if ((p_nsobv->Z[i] == 0) && (p_nsobv->X[i] == 0))
            continue;
        x = p_nsobv->X[i] << 15;
        z = p_nsobv->Z[i] << 15;
        fill_blippoint_scanner(x, z, 87, n);
        n++;
    }
    return n;
}

void add_signal_to_scanner(struct Objective *p_objectv, ubyte flag)
{
    if (flag)
        clear_all_scanner_signals();
    if (gameturn != turn_last)
    {
        turn_last = gameturn;
        ingame.Scanner.GroupCount = 0;
    }
    if ((p_objectv == NULL) || ((p_objectv->Flags & GObjF_HIDDEN) != 0))
        return;

    if (signal_count >= SCANNER_BIG_BLIP_COUNT) {
        LOGWARN("Scaner blips limit reached, blip discarded.");
        return;
    }

    if (objective_target_is_group_to_area(p_objectv))
    {
        signal_count = do_group_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_group_to_vehicle(p_objectv))
    {
        signal_count = do_group_near_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_group_to_thing(p_objectv))
    {
        signal_count = do_group_near_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_group(p_objectv))
    {
        signal_count = do_group_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_person_to_area(p_objectv))
    {
        signal_count = do_thing_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_person_to_thing(p_objectv))
    {
        signal_count = do_thing_near_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_person(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_vehicle_to_area(p_objectv))
    {
        signal_count = do_thing_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_vehicle(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_item_to_area(p_objectv))
    {
        signal_count = do_thing_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_item(p_objectv))
    {
        signal_count = do_target_item_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_object(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_any_thing(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
}

void add_netscan_signal_to_scanner(struct NetscanObjective *p_nsobv, ubyte flag)
{
    if (flag)
        clear_all_scanner_signals();

    if (p_nsobv == NULL)
        return;

    if (signal_count >= SCANNER_BIG_BLIP_COUNT) {
        LOGWARN("Scaner blips limit reached, blip discarded.");
        return;
    }

    // Netscan scanner only supports blippoints
    signal_count = do_netscan_blippoint_scanner(p_nsobv, signal_count);
}

void redo_scanner_init(void)
{
    redo_scanner_last = 0;
    redo_scanner_first = 0;
}

void redo_scanner_process(void)
{
    while (redo_scanner_first != redo_scanner_last)
    {
        struct rectangle *p_rct;
        p_rct = &redo_scanner[redo_scanner_first];
        SCANNER_fill_in_a_little_bit(p_rct->x1, p_rct->y1, p_rct->x2, p_rct->y2);
        redo_scanner_first = (redo_scanner_first + 1) & 0x7F;
    }
}

/******************************************************************************/
