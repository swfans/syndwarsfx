/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file lvdraw3d.c
 *     Routines for level and map drawing using 3D rendering.
 * @par Purpose:
 *     Implement functions for creating drawlist for rendering the 3D view.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     24 Dec 2023 - 10 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lvdraw3d.h"

#include <assert.h>
#include <limits.h>
#include "bfendian.h"
#include "bfmath.h"
#include "bfplanar.h"

#include "enginbckt.h"
#include "engincolour.h"
#include "enginfloor.h"
#include "enginlights.h"
#include "engintrns.h"
#include "engintxtrmap.h"
#include "enginzoom.h"

#include "bigmap.h"
#include "display.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "enginsngtxtr.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "player.h"
#include "scanner.h"
#include "swlog.h"
#include "thing.h"
#include "tngcolisn.h"
#include "tngobjdrw.h"
/******************************************************************************/

struct Range {
    int beg;
    int fin;
};

/******************************************************************************/
#define SUPER_QUICK_LIGHTS_MAX (RENDER_AREA_MAX+1)*(RENDER_AREA_MAX+1)
extern short super_quick_light[(RENDER_AREA_MAX+1)*(RENDER_AREA_MAX+1)];

extern long dword_152E50;
extern long dword_152E54;
extern long dword_152E58;

extern long dword_176CC0;

extern short word_19CC64;
extern short word_19CC66;
extern long nuclear_overexposure;


int shpoint_compute_coord_y(struct ShEnginePoint *p_sp, struct MyMapElement *p_mapel, int elcr_x, int elcr_z, int mag)
{
    int elcr_y;

    if (game_perspective == 1)
    {
        elcr_y = 0;
        p_sp->ReflShade = 0;
    }
    else if ((p_mapel->Flags & 0x10) == 0)
    {
        elcr_y = 8 * p_mapel->Alt;
        if ((p_mapel->Flags & 0x40) != 0)
            elcr_y += waft_table[gameturn & 0x1F];
        p_sp->ReflShade = 0;
    }
    else
    {
        int wobble, dvfactor;

        elcr_y = 8 * p_mapel->Alt;
        dvfactor = 140 + ((bw_rotl32(0x5D3BA6C3, elcr_z >> 8) ^ bw_rotr32(0xA7B4D8AC, elcr_x >> 8)) & 0x7F);
        wobble = (waft_table2[(gameturn + (elcr_x >> 7)) & 0x1F]
             + waft_table2[(gameturn + (elcr_z >> 7)) & 0x1F]
             + waft_table2[(32 * gameturn / dvfactor) & 0x1F]) >> 3;
        elcr_y += mag * wobble;
        p_sp->ReflShade = (wobble + 32) << 9;
    }
    return elcr_y;
}

/** Compute shade value for drawing given map element.
 */
short shpoint_compute_shade(struct ShEnginePoint *p_sp, struct MyMapElement *p_mapel, short *p_sqlight)
{
    int shd;

    shd = (p_mapel->Ambient << 7) + p_sp->ReflShade + 256 + (*p_sqlight << 8);
    shd += cummulate_shade_from_quick_lights(p_mapel->Shade);
    if (shd > 0x7E00)
        shd = 0x7F00;
    return shd;
}

/** Compute shade value for map element, fading to black beyond some range.
 */
short shpoint_compute_shade_fading(struct ShEnginePoint *p_sp, struct MyMapElement *p_mapel, int dist)
{
    int shd;

    shd = (p_mapel->Ambient << 7) + p_sp->ReflShade + 256;
    shd += cummulate_shade_from_quick_lights(p_mapel->Shade);
    if (dist > 3000) {
        if (3512 - dist > 0)
            shd = ((3512 - dist) * shd) >> 9;
        else
            shd = 0;
    }
    if (shd > 0x7E00)
        shd = 0x7F00;
    return shd;
}

ubyte lvdraw_fill_bound_points(struct TbPoint *bound_pts)
{
    int fctr_x1, fctr_z1, fctr_x2, fctr_z2;
    int mag_x, mag_z;
    ubyte slt, slt_zmin;
    ushort angXZ;
    int sin_angl, cos_angl;

    angXZ = (engn_anglexz >> 5) & 0x7FF;
    sin_angl = lbSinTable[angXZ];
    cos_angl = lbSinTable[angXZ + LbFPMath_PI/2];

    fctr_x1 = (dword_152E58 * render_area_a * cos_angl) >> 16;
    fctr_z2 = (dword_152E58 * render_area_a * -sin_angl) >> 16;
    fctr_z1 = (render_area_b * cos_angl) >> 8;
    fctr_x2 = (render_area_b * sin_angl) >> 8;
    mag_z = engn_zc & 0xFF00;
    mag_x = engn_xc & 0xFF00;

    bound_pts[0].y = (mag_z + ((dword_152E50 * (fctr_z1 + fctr_z2)) >> 8)) >> 8;
    bound_pts[1].y = (mag_z + ((dword_152E54 * (fctr_z2 - fctr_z1)) >> 8)) >> 8;
    bound_pts[1].x = (mag_x + ((dword_152E54 * (fctr_x1 - fctr_x2)) >> 8)) >> 8;
    bound_pts[2].x = (mag_x + ((dword_152E54 * (-fctr_x1 - fctr_x2)) >> 8)) >> 8;
    bound_pts[0].x = (mag_x + ((dword_152E50 * (fctr_x2 + fctr_x1)) >> 8)) >> 8;
    bound_pts[2].y = (mag_z + ((dword_152E54 * (-fctr_z2 - fctr_z1)) >> 8)) >> 8;
    bound_pts[3].y = (mag_z + ((dword_152E50 * (fctr_z1 - fctr_z2)) >> 8)) >> 8;
    bound_pts[3].x = (mag_x + ((dword_152E50 * (fctr_x2 - fctr_x1)) >> 8)) >> 8;

    slt_zmin = 0;
    for (slt = 1; slt < 4; slt++) {
        if (bound_pts[slt].y < bound_pts[slt_zmin].y)
            slt_zmin = slt;
    }

    return slt_zmin;
}

int lvdraw_fill_ranges_x(int slt_zmin, struct Range *ranges_x, struct TbPoint *bound_pts)
{
    int rn, ranges_used;
    int slt1_fcx, slt2_fcx;
    int slt1_incx, slt2_incx;
    int z_cur, z_max;
    ubyte slt, slt1, slt2;

    z_max = bound_pts[0].y;
    for (slt = 1; slt < 4; slt++) {
        if (z_max < bound_pts[slt].y)
            z_max = bound_pts[slt].y;
    }

    slt2 = slt_zmin;
    slt1 = slt_zmin;
    slt1_fcx = slt2_fcx = 0;
    slt1_incx = slt2_incx = 0;
    z_cur = bound_pts[slt_zmin].y;
    rn = 0;
    while (z_cur < z_max)
    {
        if (z_cur == bound_pts[slt1].y)
        {
            int dt_z;
            int base_fcx;
            int x_cur;

            while (z_cur == bound_pts[slt1].y)
            {
              int x_tmp;
              x_tmp = bound_pts[slt1].x;
              slt1 = (slt1 + 1) & 3;
              base_fcx = x_tmp << 16;
            }
            x_cur = bound_pts[slt1].x;
            dt_z = bound_pts[slt1].y - z_cur;
            slt1_incx = ((x_cur << 16) - base_fcx) / dt_z;
            slt1_fcx = slt1_fcx + 0x10000;
        }

        if (z_cur == bound_pts[slt2].y)
        {
            int dt_z;
            int base_fcx;
            int x_cur;

            while (z_cur == bound_pts[slt2].y)
            {
              int x_tmp;
              x_tmp = bound_pts[slt2].x;
              slt2 = (slt2 - 1) & 3;
              base_fcx = x_tmp << 16;
            }
            x_cur = bound_pts[slt2].x;
            dt_z = bound_pts[slt2].y - z_cur;
            slt2_incx = ((x_cur << 16) - base_fcx) / dt_z;
            slt2_fcx = base_fcx + 0x10000;
        }

        ranges_x[rn + 1].beg = slt1_fcx >> 16 << 8;
        ranges_x[rn + 1].fin = slt2_fcx >> 16 << 8;

        z_cur++;
        slt2_fcx += slt2_incx;
        slt1_fcx += slt1_incx;
        rn++;
    }

    ranges_used = rn;
    for (rn = 0; rn < ranges_used; rn++)
    {
        if (ranges_x[rn + 1].beg < 0)
            ranges_x[rn + 1].beg = 0;
        if (ranges_x[rn + 1].fin > 0x7F00)
            ranges_x[rn + 1].fin = 0x7F00;
    }

    return ranges_used;
}

void filter_ranges_max_of_two(int ranges_len, struct Range *ranges_flt, struct Range *ranges_inp)
{
    int rn;

    ranges_flt[0].beg = ranges_inp[1].beg;
    ranges_flt[0].fin = ranges_inp[1].fin + 256;

    for (rn = 1; rn < ranges_len; rn++)
    {
        int x1, x2;

        x1 = ranges_inp[rn].beg;
        if (x1 < ranges_inp[rn + 1].beg)
            x1 = ranges_inp[rn + 1].beg;
        ranges_flt[rn].beg = x1;

        x2 = ranges_inp[rn].fin + 256;
        if (x2 < ranges_inp[rn + 1].fin + 256)
            x2 = ranges_inp[rn + 1].fin + 256;
        ranges_flt[rn].fin = x2;
    }

    ranges_flt[rn].beg = ranges_flt[rn - 1].beg;
    ranges_flt[rn].fin = ranges_flt[rn - 1].fin;
}

void lvdraw_do_objects(int cor_z_beg, uint ranges_x_len, struct Range *ranges_x)
{
    int cor_z;
    int cor_x, cor_x_end;
    uint rn;

    cor_z = cor_z_beg;
    for (rn = 0; rn < ranges_x_len; rn++)
    {
        cor_x = ranges_x[rn + 1].beg;
        cor_x_end = ranges_x[rn + 1].fin;
        for (; cor_x <= cor_x_end; cor_x += (1 << 8))
        {
            struct Thing *p_objtng;
            struct MyMapElement *p_mapel;
            ThingIdx objtng;
            short tile_x, tile_z;

            if ((cor_x <= 0) || (cor_x >= 0x8000) || (cor_z <= 0) || (cor_z >= 0x8000))
                continue;

            tile_z = cor_z >> 8;
            tile_x = cor_x >> 8;
            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            objtng = game_col_vects_list[p_mapel->ColHead].Object;
            if (objtng > 0)
            {
                p_objtng = &things[objtng];
                if (p_objtng->U.UObject.DrawTurn != gameturn)
                    draw_thing_object(p_objtng);
            }
        }
        cor_z += (1 << 8);
    }
}

void fill_floor_tile_pos_and_shade(struct FloorTile *p_floortl, struct MyMapElement *p_mapel,
  ubyte pt, short *p_sqlight, struct ShEnginePoint *p_sp)
{
    p_floortl->X[pt] = p_sp->X;
    p_floortl->Y[pt] = p_sp->Y;
    if (p_sp->Shade < 0) {
        p_sp->Shade = shpoint_compute_shade(p_sp, p_mapel, p_sqlight);
    }
    p_floortl->Shade[pt] = p_sp->Shade;
    p_mapel->ShadeR = p_sp->Shade >> 9;
}

void fill_floor_tile_pos_and_shade_fading(struct FloorTile *p_floortl, struct MyMapElement *p_mapel,
  struct ShEnginePoint *p_dsp, ubyte pt, struct ShEnginePoint *p_ssp)
{
    p_floortl->X[pt] = p_dsp->X;
    p_floortl->Y[pt] = p_dsp->Y;
    if (p_dsp->Shade < 0) {
        //TODO why do we use p_ssp->ReflShade instead of using only one ShEnginePoint (the p_dsp)?
        // is ReflShade unset in the other ShEnginePoint?
        p_dsp->Shade = shpoint_compute_shade_fading(p_ssp, p_mapel, p_dsp->Depth);
    }
    p_floortl->Shade[pt] = p_dsp->Shade;
    p_mapel->ShadeR = p_dsp->Shade >> 9;
}

void lvdraw_do_floor(void)
{
#if 0
    asm volatile ("call ASM_lvdraw_do_floor\n"
        :  :  : "eax" );
    return;
#endif
    struct ShEnginePoint loc_unknarrD[(RENDER_AREA_MAX+1)*4];
    int shift_a, shift_b;
    int elcr_z, elpv_z; // Coord Z for current and previous map element
    short *p_sqlight;

    word_19CC64 = (engn_xc & 0xFF00) - (render_area_a << 7);
    word_19CC66 = (engn_zc & 0xFF00) - (render_area_b << 7);
    if (word_19CC66 < 0)
        word_19CC66 = 0;
    p_sqlight = super_quick_light;

    elcr_z = word_19CC66;
    shift_b = 0;
    { // Separate first row from the rest as it has no previous
        struct MyMapElement *p_mapel;
        struct ShEnginePoint *p_spcr;
        int elcr_x;

        p_spcr = &loc_unknarrD[shift_b & 1];
        shift_a = 0;
        elcr_x = word_19CC64;
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elcr_z >> 8) + (elcr_x >> 8)];

        while (shift_a < render_area_a + 1)
        {
            int elcr_y;

            elcr_y = shpoint_compute_coord_y(p_spcr, p_mapel, elcr_x, elcr_z, 4);
            transform_shpoint(p_spcr, elcr_x - engn_xc, elcr_y - 8 * engn_yc, elcr_z - engn_zc);
            p_spcr->Shade = shpoint_compute_shade(p_spcr, p_mapel, p_sqlight);

            p_spcr += 2;
            p_mapel++;
            shift_a++;
            elcr_x += TILE_TO_MAPCOORD(1, 0);
        }
    }

    elpv_z = elcr_z;
    elcr_z += TILE_TO_MAPCOORD(1, 0);
    shift_b++;
    while (shift_b < render_area_b && elcr_z < 0x8000)
    {
        struct MyMapElement *p_mapel;
        struct ShEnginePoint *p_spcr;
        struct ShEnginePoint *p_spnx;
        int elcr_x;

        p_spcr = &loc_unknarrD[(shift_b) & 1];
        shift_a = 0;
        elcr_x = word_19CC64;
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elcr_z >> 8) + (elcr_x >> 8)];

        while (shift_a < render_area_a + 1)
        {
            int elcr_y;

            elcr_y = shpoint_compute_coord_y(p_spcr, p_mapel, elcr_x, elcr_z, 4);
            transform_shpoint(p_spcr, elcr_x - engn_xc, elcr_y - 8 * engn_yc, elcr_z - engn_zc);
            p_spcr->Shade = -1;

            p_spcr += 2;
            p_mapel++;
            shift_a++;
            elcr_x += TILE_TO_MAPCOORD(1, 0);
        }

        p_spnx = &loc_unknarrD[(shift_b + 1) & 1];
        p_spcr = &loc_unknarrD[(shift_b) & 1];
        shift_a = 0;
        elcr_x = word_19CC64;
        while (shift_a < render_area_a)
        {
            struct FloorTile *p_floortl;
            int depth, dpthalt;
            ushort floor_flags2;
            ubyte ditype;

            dpthalt = 0;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elpv_z >> 8) + (elcr_x >> 8)];

            if ( (((p_spcr[2].Flags | p_spnx[2].Flags | p_spcr[0].Flags | p_spnx[0].Flags) & 0x20) != 0)
              || (((p_spnx[2].Flags & p_spcr[0].Flags & p_spnx[0].Flags & p_spcr[2].Flags) & 0x0F) != 0)
              || (elcr_x <= 0) || (elcr_x >= 0x8000) || (elcr_z <= 0) || (elcr_z >= 0x8000)
              || ((game_perspective != 2) && ((p_mapel->Flags & 0x80) != 0)))
            {
                p_sqlight++;
                p_spcr += 2;
                p_spnx += 2;
                shift_a++;
                elcr_x += TILE_TO_MAPCOORD(1, 0);
                continue;
            }

            depth = INT_MIN;

            if (depth < p_spnx[0].Depth)
                depth = p_spnx[0].Depth;
            if (depth < p_spnx[2].Depth)
                depth = p_spnx[2].Depth;
            if (depth < p_spcr[2].Depth)
                depth = p_spcr[2].Depth;
            if (depth < p_spcr[0].Depth)
                depth = p_spcr[0].Depth;

            floor_flags2 = 0;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elpv_z >> 8) + (elcr_x >> 8)];
            if ((p_mapel->Texture & 0x8000) != 0)
            {
                floor_flags2 |= 0x01;
                if (byte_1C8444)
                {
                    int alt;
                    if (p_mapel->Alt <= 0)
                      alt = 15000 * overall_scale;
                    else
                      alt = 500 * overall_scale;
                    dpthalt = alt >> 8;
                }
                else
                {
                    if (p_mapel->Alt <= 0)
                      dpthalt = 3500;
                    else
                      dpthalt = 2500;
                }
            }
            dpthalt += 200;

            ditype = (p_mapel->Texture & 0x4000) != 0 ? DrIT_Unkn6 : DrIT_Unkn4;
            p_floortl = draw_item_add_floor_tile(ditype, BUCKET_MID + depth + dpthalt);
            if (p_floortl == NULL)
                break;

            fill_floor_tile_pos_and_shade(p_floortl, p_mapel, 0, p_sqlight, p_spnx);

            p_spnx += 2;
            p_sqlight += 1;
            fill_floor_tile_pos_and_shade(p_floortl, p_mapel + 1, 1, p_sqlight, p_spnx);

            p_spcr += 2;
            p_sqlight += render_area_a;
            fill_floor_tile_pos_and_shade(p_floortl, p_mapel + MAP_TILE_WIDTH + 1, 2, p_sqlight, p_spcr);

            p_spcr -= 2;
            p_sqlight -= 1;
            fill_floor_tile_pos_and_shade(p_floortl, p_mapel + MAP_TILE_WIDTH, 3, p_sqlight, p_spcr);

            if (p_mapel->Texture != 0)
            {
                struct SingleFloorTexture *p_fltextr;
                p_fltextr = &game_textures[p_mapel->Texture & 0x3FFF];
                p_floortl->Texture = p_fltextr;
                if ((p_mapel->Flags & 0x20) != 0)
                    p_floortl->Flags = RendVec_mode21;
                else
                    p_floortl->Flags = RendVec_mode05;
            }
            else
            {
                p_floortl->Flags = RendVec_mode04;
                p_floortl->Col = colour_grey2;
            }

            if ((p_mapel->Flags & 0x01) != 0)
            {
                p_floortl->Shade[0] = 0x3F00;
                p_floortl->Shade[1] = 0x3F00;
                p_floortl->Shade[2] = 0x3F00;
                p_floortl->Shade[3] = 0x3F00;
            }
            if ((p_mapel->Flags & 0x08) != 0)
                floor_flags2 |= 0x02;

            p_floortl->Flags2 = p_mapel->Flags | floor_flags2;
            p_floortl->Flags2b = p_mapel->Flags2;
            p_floortl->Offset = p_mapel - game_my_big_map;
            p_floortl->Page = p_mapel->ColumnHead >> 12;

            p_sqlight += -render_area_a + 1;
            p_spcr += 2;
            shift_a++;
            elcr_x += TILE_TO_MAPCOORD(1, 0);
        }
        shift_b++;
        elpv_z += TILE_TO_MAPCOORD(1, 0);
        elcr_z += TILE_TO_MAPCOORD(1, 0);
    }
}

void lvdraw_do_floor_flyby(int cor_z_beg, int ranges_x_len, struct Range *smrang_x, struct Range *ranges_x)
{
    struct ShEnginePoint loc_unknarrD[(RENDER_AREA_MAX+1)*4];
    int elcr_z, elpv_z; // Coord Z for current and previous map element
    int rn;

    word_19CC64 = (engn_xc & 0xFF00) - (render_area_a << 7);
    word_19CC66 = (engn_zc & 0xFF00) - (render_area_b << 7);

    elcr_z = cor_z_beg;
    elpv_z = cor_z_beg - TILE_TO_MAPCOORD(1, 0);

    rn = 0;
    { // Separate first row from the rest as it has no previous
        struct MyMapElement *p_mapel;
        struct ShEnginePoint *p_spcr;
        int elcr_x;

        elcr_x = smrang_x[rn].beg;
        p_spcr = &loc_unknarrD[2 * (elcr_x >> 8) + ((rn) & 1)];
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elcr_z >> 8) + (elcr_x >> 8)];
        while (elcr_x <= smrang_x[rn].fin)
        {
            int elcr_y;

            elcr_y = shpoint_compute_coord_y(p_spcr, p_mapel, elcr_x, elcr_z, 8);
            transform_shpoint_fpv(p_spcr, elcr_x - engn_xc, elcr_y - 8 * engn_yc, elcr_z - engn_zc);
            p_spcr->Shade = shpoint_compute_shade_fading(p_spcr, p_mapel, p_spcr->Depth);

            p_spcr += 2;
            p_mapel++;
            elcr_x += TILE_TO_MAPCOORD(1, 0);
        }
    }

    for (rn = 1; rn < ranges_x_len; rn++)
    {
        struct MyMapElement *p_mapel;
        struct ShEnginePoint *p_spcr;
        struct ShEnginePoint *p_spnx;
        int elcr_x;

        elcr_x = smrang_x[rn].beg;
        p_spcr = &loc_unknarrD[2 * (elcr_x >> 8) + (rn & 1)];
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elcr_z >> 8) + (elcr_x >> 8)];
        while (elcr_x <= smrang_x[rn].fin)
        {
            int elcr_y;

            elcr_y = shpoint_compute_coord_y(p_spcr, p_mapel, elcr_x, elcr_z, 8);
            transform_shpoint_fpv(p_spcr, elcr_x - engn_xc, elcr_y - 8 * engn_yc, elcr_z - engn_zc);
            p_spcr->Shade = -1;

            p_spcr += 2;
            elcr_x += TILE_TO_MAPCOORD(1, 0);
            p_mapel++;
        }

        elcr_x = ranges_x[rn].beg;
        p_spnx = &loc_unknarrD[2 * (elcr_x >> 8) + ((rn + 1) & 1)];
        p_spcr = &loc_unknarrD[2 * (elcr_x >> 8) + ((rn) & 1)];
        while (elcr_x <= ranges_x[rn].fin)
        {
            struct FloorTile *p_floortl;
            struct MyMapElement *p_mapel;
            struct ShEnginePoint *p_spad;
            int depth, dpthalt;
            ushort floor_flags2;
            ubyte ditype;

            dpthalt = 0;
            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elpv_z >> 8) + (elcr_x >> 8)];

            if ( (((p_spcr[2].Flags | p_spnx[2].Flags | p_spcr[0].Flags | p_spnx[0].Flags) & 0x20) != 0)
              || (((p_spnx[2].Flags & p_spcr[0].Flags & p_spnx[0].Flags & p_spcr[2].Flags) & 0x0F) != 0)
              || (elcr_x <= 0) || (elcr_x >= 0x8000) || (elcr_z <= 0) || (elcr_z >= 0x8000))
            {
                p_spcr += 2;
                p_spnx += 2;
                elcr_x += TILE_TO_MAPCOORD(1, 0);
                continue;
            }

            depth = INT_MIN;

            if (depth < p_spnx[0].Depth)
                depth = p_spnx[0].Depth;
            if (depth < p_spnx[2].Depth)
                depth = p_spnx[2].Depth;
            if (depth < p_spcr[2].Depth)
                depth = p_spcr[2].Depth;
            if (depth < p_spcr[0].Depth)
                depth = p_spcr[0].Depth;

            floor_flags2 = 0;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (elpv_z >> 8) + (elcr_x >> 8)];
            if ((p_mapel->Texture & 0x8000) != 0)
            {
                floor_flags2 |= 0x01;
                if (byte_1C8444)
                {
                    uint tmp;
                    if (p_mapel->Alt <= 0)
                        tmp = 15000 * overall_scale;
                    else
                        tmp = 500 * overall_scale;
                    dpthalt = tmp >> 8;
                }
                else
                {
                    if (p_mapel->Alt <= 0)
                        dpthalt = 2000;
                    else
                        dpthalt = 1000;
                }
            }

            ditype = (p_mapel->Texture & 0x4000) != 0 ? DrIT_Unkn6 : DrIT_Unkn4;
            p_floortl = draw_item_add_floor_tile(ditype, BUCKET_MID + depth + dpthalt);
            if (p_floortl == NULL)
                break;

            fill_floor_tile_pos_and_shade_fading(p_floortl, p_mapel, p_spnx, 0, p_spnx);

            p_spnx += 2;
            fill_floor_tile_pos_and_shade_fading(p_floortl, p_mapel + 1, p_spnx, 1, p_spnx);

            p_spad = p_spcr + 2;
            fill_floor_tile_pos_and_shade_fading(p_floortl, p_mapel + 128 + 1, p_spad, 2, p_spnx);

            p_spad = p_spcr;
            fill_floor_tile_pos_and_shade_fading(p_floortl, p_mapel + 128, p_spad, 3, p_spnx);

            if (p_mapel->Texture != 0)
            {
                struct SingleFloorTexture *p_fltextr;
                p_fltextr = &game_textures[p_mapel->Texture & 0x3FFF];
                p_floortl->Texture = p_fltextr;
                if ((p_mapel->Flags & 0x20) != 0)
                    p_floortl->Flags = RendVec_mode21;
                else
                    p_floortl->Flags = RendVec_mode05;
                p_floortl->Page = (int)(ushort)p_mapel->ColumnHead >> 12;
            }
            else
            {
                p_floortl->Flags = RendVec_mode04;
                p_floortl->Col = colour_grey2;
            }

            if ((p_mapel->Flags & 0x01) != 0)
            {
                p_floortl->Shade[0] = 0x3F00;
                p_floortl->Shade[1] = 0x3F00;
                p_floortl->Shade[2] = 0x3F00;
                p_floortl->Shade[3] = 0x3F00;
            }
            if ((p_mapel->Flags & 0x08) != 0)
                floor_flags2 |= 0x02;

            p_floortl->Flags2 = p_mapel->Flags | floor_flags2;
            p_floortl->Flags2b = p_mapel->Flags2;
            p_floortl->Offset = p_mapel - game_my_big_map;

            p_spcr = p_spad + 2;
            elcr_x += TILE_TO_MAPCOORD(1, 0);
        }
        elpv_z += TILE_TO_MAPCOORD(1, 0);
        elcr_z += TILE_TO_MAPCOORD(1, 0);
    }
}

void func_2e440(void)
{
    int angXZ;
    ubyte slt_zmin;

    struct Range smrang_x[160];
    struct Range ranges_x[160];
    struct TbPoint bound_pts[4];
    int cor_z_beg, ranges_x_len;

    reset_drawlist();

    slt_zmin = lvdraw_fill_bound_points(bound_pts);

    cor_z_beg = bound_pts[slt_zmin].y << 8;

    ranges_x_len = lvdraw_fill_ranges_x(slt_zmin, ranges_x, bound_pts);

    filter_ranges_max_of_two(ranges_x_len, smrang_x, ranges_x);

    player_target_clear(local_player_no);

    if ((ingame.Flags & GamF_BillboardMovies) != 0)
    {
        dword_176CC0 += fifties_per_gameturn;
        if (dword_176CC0 > 80) {
            dword_176CC0 = 0;
            xdo_next_frame(AniSl_BILLBOARD);
        }
    }
    angXZ = (engn_anglexz >> 5) & 0x7FF;
    byte_176D4B = ((angXZ + 64) >> 7) & 0xF;
    byte_176D48 = ((angXZ + 256) >> 9) & 0x3;
    byte_176D49 = ((angXZ + 128) >> 8) & 0x7;
    byte_19EC7A = byte_176D48;

    lvdraw_do_objects(cor_z_beg, ranges_x_len, ranges_x);

    lvdraw_do_floor_flyby(cor_z_beg, ranges_x_len, smrang_x, ranges_x);

    assert(vec_tmap[1] != NULL);
    vec_map = vec_tmap[1];

    draw_screen();
}

#define SUPER_QUICK_RADIUS 5
void apply_super_quick_light(short lx, short lz, ushort b)
{
    short tile_x_beg, tile_x_end;
    short tile_z_beg, tile_z_end;
    short tile_x, tile_z;
    int mapcor_x, mapcor_z;
    short ratile_x_beg, ratile_z_beg;
    short ratile_x, ratile_z;

    tile_z_beg = MAPCOORD_TO_TILE(lz) - SUPER_QUICK_RADIUS/2;
    tile_x_beg = MAPCOORD_TO_TILE(lx) - SUPER_QUICK_RADIUS/2;
    if (tile_z_beg <= -SUPER_QUICK_RADIUS || tile_z_beg >= MAP_TILE_HEIGHT)
        return;
    if (tile_x_beg <= -SUPER_QUICK_RADIUS || tile_x_beg >= MAP_TILE_WIDTH)
        return;

    mapcor_z = TILE_TO_MAPCOORD(render_area_b,0) / 2;
    mapcor_x = TILE_TO_MAPCOORD(render_area_a,0) / 2;
    if ((lz <= engn_zc - mapcor_z) || (lz >= engn_zc + mapcor_z))
        return;
    if ((lx <= engn_xc - mapcor_x) || (lx >= engn_xc + mapcor_x))
        return;

    ratile_z_beg = (render_area_b >> 1) - MAPCOORD_TO_TILE(engn_zc);
    if (ratile_z_beg > 0) // required to avoid shifting light to terrain near map border
        ratile_z_beg = 0;
    ratile_z_beg += tile_z_beg;
    ratile_x_beg = (render_area_a >> 1) - MAPCOORD_TO_TILE(engn_xc);
    ratile_x_beg += tile_x_beg;
    tile_x_end = tile_x_beg + SUPER_QUICK_RADIUS;
    tile_z_end = tile_z_beg + SUPER_QUICK_RADIUS;

    for (tile_z = tile_z_beg, ratile_z = ratile_z_beg; tile_z < tile_z_end; tile_z++, ratile_z++)
    {
        mapcor_z = TILE_TO_MAPCOORD(tile_z,0);
        for (tile_x = tile_x_beg, ratile_x = ratile_x_beg; tile_x < tile_x_end; tile_x++, ratile_x++)
        {
            short *p_sqlight;
            int f, dist;
            short intensity;

            mapcor_x = TILE_TO_MAPCOORD(tile_x,0);

            if (ratile_x < 0 || ratile_x >= render_area_a)
                continue;
            if (ratile_z < 0 || ratile_z >= render_area_b)
                continue;
            if (tile_x < 0 || tile_x >= MAP_TILE_WIDTH)
                continue;
            if (tile_z < 0 || tile_z >= MAP_TILE_HEIGHT)
                continue;

            p_sqlight = &super_quick_light[ratile_x + render_area_a * ratile_z];

            dist = (mapcor_x - lx) * (mapcor_x - lx)
                + (mapcor_z - lz) * (mapcor_z - lz);
            if (dist > 0)
              f = 1088608 / dist;
            else
              f = 32;
            intensity = b * f >> 5;
            if (intensity > 32)
                intensity = 32;
            *p_sqlight += intensity;
        }
    }
}

void clear_super_quick_lights(void)
{
    int i;

    for (i = 0; i < render_area_a * (render_area_b + 1); i++)
    {
        short *p_sqlight;
        p_sqlight = &super_quick_light[i];
        *p_sqlight = 0;
    }
}

void draw_screen(void)
{
    if (nuclear_overexposure)
    {
        draw_drawlist_1();
    }
    else
    {
        draw_drawlist_2();
    }
#if 0
    //TODO Setting first palette colour was often used as debug helper; to be removed
    outp(0x3C8u, 0);
    outp(0x3C9u, byte_1C83E0);
    outp(0x3C9u, 0);
    outp(0x3C9u, 0);
#endif
    reset_drawlist();
}

/******************************************************************************/
