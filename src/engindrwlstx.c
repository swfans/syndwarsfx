/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstx.c
 *     Drawlists execution for the 3D engine.
 * @par Purpose:
 *     Implements functions for executing previously made drawlists,
 *     meaning the actual drawing based on primitives in the list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engindrwlstx.h"

#include "bfkeybd.h"
#include "bfgentab.h"
#include "bfsprite.h"
#include "insspr.h"
#include <assert.h>
#include <stdlib.h>

#include "enginfloor.h"
#include "enginpeff.h"
#include "enginprops.h"
#include "render_gpoly.h"

#include "bigmap.h"
#include "display.h"
#include "drawtext.h"
#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "engindrwlstm.h"
#include "enginlights.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginshadws.h"
#include "enginshrapn.h"
#include "engintrns.h"
#include "frame_sprani.h"
#include "game_options.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "packet.h"
#include "player.h"
#include "scandraw.h"
#include "thing.h"
#include "swlog.h"
#include "vehicle.h"
/******************************************************************************/

extern ushort tnext_screen_point;
extern ushort tnext_draw_item;
extern ushort tnext_sort_sprite;
//extern ushort tnext_sort_line; -- no such var?
//extern ushort tnext_special_face;
extern ushort tnext_special_face4;

ushort next_special_face = 1;
ushort next_special_face4 = 1;

extern long dword_176CC4;

extern struct Thing *dword_176CC8;
extern int dword_176CCC;
extern int dword_176CD0;

extern long unkn1_pos_x;
extern long unkn1_pos_y;
extern struct TbSprite *unkn1_spr;
extern long dword_176CE0;
extern long dword_176CE4;
extern long dword_176CE8;
extern long dword_176CEC;
extern long dword_176CF0;
extern long dword_176CF4;
extern long dword_176D00;
extern long dword_176D04;

extern ubyte byte_176D49;

extern short word_1A5834;
extern short word_1A5836;

struct DrawItem *game_draw_list;
struct DrawItem *p_current_draw_item;
ushort next_draw_item;

struct SortSprite *game_sort_sprites;
struct SortSprite *p_current_sort_sprite;
ushort next_sort_sprite;

struct SortLine *game_sort_lines;
struct SortLine *p_current_sort_line;
ushort next_sort_line;

TbPixel deep_radar_surface_col = 0xd8;
TbPixel deep_radar_line_col = 0x64;

ubyte byte_15399C[] = {
  0, 1, 2, 0, 0,
  0, 2, 1, 0, 0,
  0, 1, 1, 0, 0,
  1, 2, 0, 0, 0,
  1, 1, 0, 0, 0,
  2, 0, 1, 0, 0,
  2, 0, 2, 0, 0,
  2, 0, 2, 0, 0,
};
/******************************************************************************/
// from engindrwlstx_spr
void draw_sort_line1a(ushort sln);
void draw_sorted_sprite1b(ubyte *frv, ushort frm, short x, short y,
  ubyte bri, ubyte angle);
void draw_sort_sprite1c(ushort sspr);
void draw_phwoar(ushort ph);
void draw_horiz_level_bar(short scr_x, short scr_y, ushort w, ushort h,
  short lvl, ushort max_lvl, TbPixel lvl_col, TbPixel bar_col);
void draw_sort_sprite_number(ushort sspr);
void draw_fire_flame(ushort flm);
// from engindrwlstx_fac
void set_face_texture_uv(ushort stex, struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2, struct PolyPoint *p_pt3, ubyte gflags);
void set_floor_texture_uv(ushort sftex, struct PolyPoint *p_pt1, struct PolyPoint *p_pt2,
  struct PolyPoint *p_pt3, struct PolyPoint *p_pt4, ubyte gflags);
void set_floor_texture_uv_damaged_ground(struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2, struct PolyPoint *p_pt3, struct PolyPoint *p_pt4, ubyte neighbrs);
void draw_object_face4g_textrd(ushort face4);
void draw_object_face3_reflect(ushort face3);
void draw_object_face4_reflect(ushort face4);
void draw_object_face3g_textrd(ushort face);
void draw_object_face4d_textrd_dk(ushort face4);
void draw_ex_face(ushort exface);
void draw_special_object_face4(ushort face4);
void draw_object_face4_pole(ushort face4);
void draw_object_face4d_textrd(ushort face4);
void draw_object_face3_textrd(ushort face);
void draw_object_face3_textrd_dk(ushort face);
void draw_object_face3_deep_rdr(ushort face);
void draw_object_face4_deep_rdr(ushort face4);
void draw_shrapnel(ushort shrap);

void reset_drawlist(void)
{
    tnext_screen_point = next_screen_point;
    next_screen_point = 0;

    next_sort_line = 0;
    p_current_sort_line = &game_sort_lines[next_sort_line];

    tnext_draw_item = next_draw_item;
    next_draw_item = 1;
    p_current_draw_item = &game_draw_list[next_draw_item];

    tnext_sort_sprite = next_sort_sprite;
    next_sort_sprite = 0;
    p_current_sort_sprite = &game_sort_sprites[next_sort_sprite];

    next_special_face = 1;

    tnext_special_face4 = next_special_face4;
    next_special_face4 = 1;

    ingame.NextRocket = 0;

    tnext_floor_texture = next_floor_texture;

    next_floor_tile = 1;

    dword_176CC4 = 0;
}

ubyte check_mouse_overlap(ushort sspr)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_check_mouse_overlap\n"
        : "=r" (ret) : "a" (sspr));
    return ret;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    if (box.Width < 16)
    {
        box.X -= ((16 + 1 - box.Width) >> 1);
        box.Width = 16;
    }
    if (box.Height < 20) {
        box.Y -= ((20 + 1 - box.Height) >> 1);
        box.Height = 20;
    }

    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
        p_locplayer->Target = p_sspr->PThing->ThingOffset;
        p_locplayer->TargetType = TrgTp_Unkn7;
        return 1;
    }
    return 0;
}

ubyte check_mouse_overlap_item(ushort sspr)
{
#if 0
    asm volatile (
      "call ASM_check_mouse_overlap_item\n"
        : : "a" (sspr));
    return 0;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;
    PlayerInfo *p_locplayer;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    p_locplayer = &players[local_player_no];
    if (p_locplayer->TargetType == TrgTp_DroppedTng)
    {
        ushort VX;
        VX = p_sspr->PThing->VX;
        if ( VX )
        {
            if (VX < 12 || VX > 13)
                return 0;
            box.X -= (box.Width >> 1);
            box.Y -= (box.Height >> 1);
            box.Width *= 2;
            box.Height *= 2;
        }
    }
    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        p_locplayer->Target = p_sspr->PThing->ThingOffset;
        p_locplayer->TargetType = TrgTp_DroppedTng;
        return 1;
    }
    return 0;
}

ubyte check_mouse_overlap_corpse(ushort sspr)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_check_mouse_overlap_corpse\n"
        : "=r" (ret) : "a" (sspr));
    return ret;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;
    PlayerInfo *p_locplayer;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    p_locplayer = &players[local_player_no];
    if (box.Width < 16)
    {
        box.X -= ((17 - box.Width) >> 1);
        box.Width = 16;
    }
    if (box.Height < 20) {
        box.Y -= ((21 - box.Height) >> 1);
        box.Height = 20;
    }

    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        p_locplayer->Target = p_sspr->PThing->ThingOffset;
        p_locplayer->TargetType = TrgTp_Unkn1;
        return 1;
    }
    return 0;
}

ubyte check_mouse_over_unkn2(ushort sspr, struct Thing *p_thing)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_check_mouse_over_unkn2\n"
        : "=r" (ret) : "a" (sspr), "d" (p_thing));
    return ret;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    if (box.Width < 16)
    {
        box.X -= ((17 - box.Width) >> 1);
        box.Width = 16;
    }
    if (box.Height < 20) {
        box.Y -= ((21 - box.Height) >> 1);
        box.Height = 20;
    }

    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        dword_176CC8 = p_thing;
        dword_176CD0 = box.Y - 8;
        dword_176CCC = box.X + (box.Height >> 1);
        return 1;
    }
    return 0;
}

void draw_sort_sprite_pers_e(int sspr)
{
#if 0
    asm volatile (
      "call ASM_draw_sort_sprite_pers_e\n"
        : : "a" (sspr));
    return;
#endif
    struct SortSprite *p_sspr;
    struct Thing *p_thing;
    short br_inc;
    ubyte bright;

    p_sspr = &game_sort_sprites[sspr];
    p_thing = p_sspr->PThing;
    if (p_sspr->Frame > 10000)
        return;

    br_inc = 0;
    bright = p_sspr->Brightness;
    if ((p_thing->Flag & TngF_Destroyed) == 0)
    {
        if ((p_thing->Flag & TngF_Unkn00200000) != 0)
        {
            br_inc += 16;
            if (p_thing->U.UPerson.ShieldGlowTimer) {
                br_inc += 16;
            }
        }
    }
    bright += br_inc;
    if ((p_thing->U.UPerson.AnimMode == ANIM_PERS_Unkn12) || ((ingame.Flags & GamF_ThermalView) != 0))
        bright = 32;

    word_1A5834 = 120;
    word_1A5836 = 120;

    if (((p_thing->Flag2 & TgF2_InsideBuilding) != 0) && (ingame.DisplayMode == 50))
    {
        if ((ingame.Flags & GamF_ThermalView) != 0) {
            ushort fr;
            fr = nstart_ani[1066];
            draw_sorted_sprite1a(fr, p_sspr->X, p_sspr->Y, 32);
        }
    }
    else
    {
        ubyte *frv;
        if (((p_thing->Flag2 & TgF2_Unkn00080000) != 0) && (p_thing->SubType == 2))
            bright = 32;
        frv = p_thing->U.UPerson.FrameId.Version;
        draw_sorted_sprite1b(frv, p_sspr->Frame, p_sspr->X, p_sspr->Y, bright, p_sspr->Angle);
    }

    if (p_thing->U.UPerson.EffectiveGroup != ingame.MyGroup)
    {
        PlayerInfo *p_locplayer;

        p_locplayer = &players[local_player_no];
        if ((p_thing->Flag & TngF_Destroyed) != 0)
        {
            if (p_locplayer->TargetType < TrgTp_Unkn1)
                check_mouse_overlap_corpse(sspr);
        }
        else
        {
            if (p_locplayer->TargetType < TrgTp_Unkn7)
                check_mouse_overlap(sspr);
        }
    }

    if (in_network_game)
    {
        struct Thing *p_owntng;

        p_owntng = NULL;
        if (((p_thing->Flag & TngF_PlayerAgent) != 0) && (p_thing->U.UPerson.ComCur >> 2 != local_player_no))
        {
            p_owntng = p_thing;
        }
        else if ((p_thing->Flag & TngF_Persuaded) != 0)
        {
            p_owntng = &things[p_thing->Owner];
            if (((p_owntng->Flag & TngF_PlayerAgent) == 0) || (p_owntng->U.UPerson.ComCur >> 2 == local_player_no))
                p_owntng = NULL;
        }
        if ((p_owntng != NULL) && (p_owntng->U.UPerson.CurrentWeapon != 30)) {
            check_mouse_over_unkn2(sspr, p_owntng);
        }
    }

    if (br_inc != 0)
    {
        ubyte *frv;
        ushort fr, k;
        fr = shield_frm[p_thing->ThingOffset & 3];
        k = ((gameturn + 16 * p_thing->ThingOffset) >> 2) & 7;
        frv = byte_15399C + 5 * k;
        draw_sorted_sprite1b(frv, fr, p_sspr->X, p_sspr->Y, br_inc, 0);
    }

    if (debug_hud_collision) {
        char locstr[32];
        short dy;
        sprintf(locstr, "%d ", p_thing->U.UPerson.RecoilTimer);
        dy = (37 * overall_scale) >> 8;
        draw_text(p_sspr->X, p_sspr->Y - dy, locstr, colour_lookup[ColLU_RED]);
    }

    if ((p_thing->Flag2 & TgF2_ExistsOffMap) != 0) {
        short dx, dy;
        dx = (2 * overall_scale) >> 8;
        dy = (37 * overall_scale) >> 8;
        draw_text(p_sspr->X - dx, p_sspr->Y - dy, "E", colour_lookup[ColLU_RED]);
    }
    if ((ingame.DisplayMode != 50) && ((p_thing->Flag2 & TgF2_InsideBuilding) != 0)) {
        short dx, dy;
        dx = (2 * overall_scale) >> 8;
        dy = (37 * overall_scale) >> 8;
        draw_text(p_sspr->X + dx, p_sspr->Y - dy, "B", colour_lookup[ColLU_RED]);
    }
}

/**
 * Draw health bar of a vehicle.
 *
 * @param sspr Index of SortSprite instance which stores reference to the vehicle thing.
 */
void draw_sort_sprite_veh_health_bar(short sspr)
{
#if 0
    asm volatile (
      "call ASM_draw_sort_sprite_veh_health_bar\n"
        : : "a" (sspr));
    return;
#endif
    struct SortSprite *p_sspr;
    struct Thing *p_thing;
    TbPixel lvl_col, bar_col;

    p_sspr = &game_sort_sprites[sspr];
    p_thing = p_sspr->PThing;

    if (ingame.PanelPermutation == -3) {
        lvl_col = 33;
        bar_col = 42;
    } else {
        lvl_col = 15;
        bar_col = 19;
    }
    draw_horiz_level_bar(p_sspr->X, p_sspr->Y, 44, 5, p_thing->Health,
      p_thing->U.UVehicle.MaxHealth, lvl_col, bar_col);
}

void draw_frame_on_map_coords_unscaled(MapCoord cor_x, MapCoord cor_y, MapCoord cor_z,
  short scr_sh_x, short scr_sh_y, ushort frm)
{
    struct ShEnginePoint sp;
    struct Frame *p_frm;
    struct Element *p_el;
    int cor_dt_x, cor_dt_y, cor_dt_z;

    cor_dt_x = cor_x - engn_xc;
    cor_dt_y = cor_y - (engn_yc >> 3);
    cor_dt_z = cor_z - engn_zc;
    {
        int cor_lr, cor_sm;
        if (abs(cor_dt_x) <= abs(cor_dt_z)) {
            cor_sm = abs(cor_dt_x);
            cor_lr = abs(cor_dt_z);
        } else {
            cor_sm = abs(cor_dt_z);
            cor_lr = abs(cor_dt_x);
        }
        if (cor_lr + (cor_sm >> 1) > TILE_TO_MAPCOORD(18,0))
            return;
    }

    transform_shpoint(&sp, cor_dt_x, cor_dt_y - 8 * engn_yc, cor_dt_z);
    sp.X += ((overall_scale * scr_sh_x) >> 8);
    sp.Y += ((overall_scale * scr_sh_y) >> 8);

    p_frm = &frame[frm];
    for (p_el = &melement_ani[p_frm->FirstElement]; p_el > melement_ani; p_el = &melement_ani[p_el->Next])
    {
        struct TbSprite *p_spr;
        short x, y;

        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_el->ToSprite);
        if (p_spr <= m_sprites)
           continue;

        lbDisplay.DrawFlags = p_el->Flags & 7;
        if ((p_el->Flags & 0xFE00) != 0)
            continue;

        x = sp.X + ((overall_scale * p_el->X) >> 9);
        y = sp.Y + ((overall_scale * p_el->Y) >> 9);
        LbSpriteDraw(x, y, p_spr);
    }
}

void draw_frame_on_map_coords(MapCoord cor_x, MapCoord cor_y, MapCoord cor_z,
  short scr_sh_x, short scr_sh_y, ushort frm)
{
    struct ShEnginePoint sp;
    struct Frame *p_frm;
    struct Element *p_el;
    int cor_dt_x, cor_dt_y, cor_dt_z;

    cor_dt_x = cor_x - engn_xc;
    cor_dt_y = cor_y - (engn_yc >> 3);
    cor_dt_z = cor_z - engn_zc;
    {
        int cor_lr, cor_sm;
        if (abs(cor_dt_x) <= abs(cor_dt_z)) {
            cor_sm = abs(cor_dt_x);
            cor_lr = abs(cor_dt_z);
        } else {
            cor_sm = abs(cor_dt_z);
            cor_lr = abs(cor_dt_x);
        }
        if (cor_lr + (cor_sm >> 1) > TILE_TO_MAPCOORD(18,0))
            return;
    }

    transform_shpoint(&sp, cor_dt_x, cor_dt_y - 8 * engn_yc, cor_dt_z);
    sp.X += ((overall_scale * scr_sh_x) >> 8);
    sp.Y += ((overall_scale * scr_sh_y) >> 8);

    p_frm = &frame[frm];
    for (p_el = &melement_ani[p_frm->FirstElement]; p_el > melement_ani; p_el = &melement_ani[p_el->Next])
    {
        struct TbSprite *p_spr;
        short x, y;

        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_el->ToSprite);
        if (p_spr <= m_sprites)
           continue;

        lbDisplay.DrawFlags = p_el->Flags & 7;
        if ((p_el->Flags & 0xFE00) != 0)
            continue;

        x = sp.X + ((overall_scale * p_el->X) >> 9);
        y = sp.Y + ((overall_scale * p_el->Y) >> 9);
        LbSpriteDrawScaled(x, y, p_spr, (overall_scale * p_spr->SWidth + 127) >> 9, (overall_scale * p_spr->SHeight + 127) >> 9);
    }
}

void number_player(struct Thing *p_person, ubyte n)
{
#if 0
    asm volatile ("call ASM_number_player\n"
        : : "a" (p_person), "d" (n));
    return;
#endif
    int shift_x, shift_y;
    ushort ani_mdsh, ani_base;
    ushort frm;

    if (lbDisplay.GraphicsScreenHeight < 400)
        ani_mdsh = 0;
    else
        ani_mdsh = 4;
    if (byte_1DB2E9 == 1) // green or blue
        ani_base = 1528;
    else
        ani_base = 1520;

    frm = nstart_ani[ani_base + ani_mdsh + n];
    {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
        if (p_locplayer->DoubleMode == 0)
        {
            if ((p_person->ThingOffset != (ThingIdx)p_locplayer->DirectControl[0]) || ((render_anim_turn & 4) != 0))
            {
                frm = frame[frm].Next;
            }
            else
            {
                ushort i;
                for (i = 0; i <= (render_anim_turn & 3); i++)
                    frm = frame[frm].Next;
            }
        }
    }

    if ((p_person->Flag2 & TgF2_Unkn0002) != 0)
        return;

    int tng_cor_x, tng_cor_y, tng_cor_z;

    if (person_is_in_a_train(p_person))
    {
        tng_cor_x = PRCCOORD_TO_MAPCOORD(p_person->X);
        tng_cor_y = PRCCOORD_TO_MAPCOORD(p_person->Y);
        tng_cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z);
    }
    else if ((p_person->Flag & TngF_Unkn4000) != 0)
    {
        struct Thing *p_vehicle;
        p_vehicle = &things[p_person->U.UPerson.Vehicle];
        tng_cor_x = PRCCOORD_TO_MAPCOORD(p_vehicle->X);
        tng_cor_y = PRCCOORD_TO_MAPCOORD(p_vehicle->Y);
        tng_cor_z = PRCCOORD_TO_MAPCOORD(p_vehicle->Z);
    }
    else
    {
        tng_cor_x = PRCCOORD_TO_MAPCOORD(p_person->X);
        tng_cor_y = PRCCOORD_TO_MAPCOORD(p_person->Y);
        tng_cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z);
    }

    if ((p_person->Flag & TngF_InVehicle) != 0)
    {
        shift_x = 7 * n - 14;
    }
    else
    {
        shift_x = -lbSinTable[256 * ((p_person->U.UObject.Angle + 2 - byte_176D49 + 8) & 7) + LbFPMath_PI/2] >> 14;
        if ((p_person->Flag2 & TgF2_Unkn00080000) == 0)
            shift_x /= 2;
    }
    shift_y = 0;

    if (lbDisplay.GraphicsScreenHeight < 400)
    {
        draw_frame_on_map_coords_unscaled(tng_cor_x, tng_cor_y, tng_cor_z, shift_x, shift_y, frm);
    }
    else
    {
        draw_frame_on_map_coords(tng_cor_x, tng_cor_y, tng_cor_z, shift_x, shift_y, frm);
    }
}

// Special non-textured draw; used during nuclear explosions?
void draw_drawitem_1(ushort dihead)
{
    struct DrawItem *itm;
    ushort iidx;

    for (iidx = dihead; iidx != 0; iidx = itm->Child)
    {
      itm = &game_draw_list[iidx];
      switch (itm->Type)
      {
      case DrIT_Unkn1:
      case DrIT_Unkn10:
          draw_object_face3_textrd_dk(itm->Offset);
          break;
      case DrIT_Unkn2:
      case DrIT_Unkn8:
          break;
      case DrIT_Unkn3:
          draw_sort_sprite1a(itm->Offset);
          break;
      case DrIT_Unkn4:
          draw_floor_tile1a(itm->Offset);
          break;
      case DrIT_Unkn5:
          draw_ex_face(itm->Offset);
          break;
      case DrIT_Unkn6:
          draw_floor_tile1b(itm->Offset);
          break;
      case DrIT_Unkn7:
          draw_object_face3g_textrd(itm->Offset);
          break;
      case DrIT_Unkn9:
          draw_object_face4d_textrd_dk(itm->Offset);
          break;
      case DrIT_Unkn11:
          draw_sort_line1a(itm->Offset);
          break;
      case DrIT_Unkn12:
          draw_special_object_face4(itm->Offset);
          break;
      case DrIT_SprPersE:
          draw_sort_sprite_pers_e(itm->Offset);
          break;
      case DrIT_Unkn14:
          draw_object_face4_pole(itm->Offset);
          break;
      case DrIT_Unkn15:
          draw_sort_sprite1c(itm->Offset);
          break;
      }
    }
}

void draw_drawitem_2(ushort dihead)
{
    struct DrawItem *itm;
    ushort iidx;
    ushort i;

    assert(screen_position_face_render_cb != NULL);
    assert(screen_sorted_sprite_render_cb != NULL);

    i = 0;
    for (iidx = dihead; iidx != 0; iidx = itm->Child)
    {
      i++;
      if (i > BUCKET_ITEMS_MAX)
          break;
      itm = &game_draw_list[iidx];
      switch (itm->Type)
      {
      case DrIT_Unkn1:
      case DrIT_Unkn10:
          draw_object_face3_textrd(itm->Offset);
          break;
      case DrIT_Unkn3:
          draw_sort_sprite1a(itm->Offset);
          break;
      case DrIT_Unkn4:
          draw_floor_tile1a(itm->Offset);
          break;
      case DrIT_Unkn5:
          draw_ex_face(itm->Offset);
          break;
      case DrIT_Unkn6:
          draw_floor_tile1b(itm->Offset);
          break;
      case DrIT_Unkn7:
          draw_object_face3g_textrd(itm->Offset);
          break;
      case DrIT_Unkn9:
          draw_object_face4d_textrd(itm->Offset);
          break;
      case DrIT_Unkn11:
          draw_sort_line1a(itm->Offset);
          break;
      case DrIT_Unkn12:
          draw_special_object_face4(itm->Offset);
          break;
      case DrIT_SprPersE:
          draw_sort_sprite_pers_e(itm->Offset);
          break;
      case DrIT_Unkn14:
          draw_object_face4_pole(itm->Offset);
          break;
      case DrIT_Unkn15:
          draw_sort_sprite1c(itm->Offset);
          break;
      case DrIT_Unkn16:
          draw_object_face4g_textrd(itm->Offset);
          break;
      case DrIT_Unkn17:
          draw_object_face3_reflect(itm->Offset);
          break;
      case DrIT_Unkn18:
          draw_object_face4_reflect(itm->Offset);
          break;
      case DrIT_Unkn19:
          draw_person_shadow(itm->Offset);
          break;
      case DrIT_Unkn20:
          draw_shrapnel(itm->Offset);
          break;
      case DrIT_Unkn21:
          draw_phwoar(itm->Offset);
          break;
      case DrIT_Unkn22:
          draw_sort_sprite_veh_health_bar(itm->Offset);
          break;
      case DrIT_Unkn23:
          draw_object_face4_deep_rdr(itm->Offset);
          break;
      case DrIT_Unkn24:
          draw_object_face3_deep_rdr(itm->Offset);
          break;
      case DrIT_Unkn25:
          draw_fire_flame(itm->Offset);
          break;
      case DrIT_Unkn26:
          draw_sort_sprite_number(itm->Offset);
          break;
      default:
          break;
      }
    }
}

/******************************************************************************/
