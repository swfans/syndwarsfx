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
#include "enginzoom.h"

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
#include "hud_panel.h"
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
void draw_hud_frame_on_screen_unscaled_but_scale_pos(short scr_x, short scr_y, ushort frm, int sscale);
void draw_hud_frame_on_screen(short scr_x, short scr_y, ushort frm, int sscale);
void draw_phwoar(ushort ph);
void draw_sort_sprite_long_prop_bar(short sspr);
void draw_sort_sprite_number(ushort sspr);
void draw_sort_sprite_short_text(ushort sspr);
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

void draw_sort_sprite_pers_e(int sspr)
{
    struct SortSprite *p_sspr;
    struct Thing *p_thing;
    short br_inc;
    ubyte bright;

    p_sspr = &game_sort_sprites[sspr];
    p_thing = (struct Thing *)p_sspr->SrcItem;
    // TODO kind of redundant, as we have asserts if below frame_end; but check for count of frames instead of hard-coded val
    if (p_sspr->Frame > 10000)
        return;

    br_inc = person_shield_glow_brightness(p_thing);

    word_1A5834 = 120;
    word_1A5836 = 120;

    if (((p_thing->Flag2 & TgF2_InsideBuilding) != 0) && (ingame.DisplayMode == 50))
    {
        if ((ingame.Flags & GamF_ThermalView) != 0) {
            ushort frm;
            frm = nstart_ani[1066];
            bright = 32;
            draw_sorted_sprite1a(frm, p_sspr->X, p_sspr->Y, bright);
        }
    }
    else
    {
        ubyte *frv;

        bright = p_sspr->Brightness + br_inc;
        if ((p_thing->U.UPerson.AnimMode == ANIM_PERS_Unkn12) || ((ingame.Flags & GamF_ThermalView) != 0))
            bright = 32;
        if (((p_thing->Flag2 & TgF2_Unkn00080000) != 0) && (p_thing->SubType == SubTT_PERS_ZEALOT))
            bright = 32;
        frv = p_thing->U.UPerson.FrameId.Version;

        draw_sorted_sprite1b(frv, p_sspr->Frame, p_sspr->X, p_sspr->Y, bright, p_sspr->Angle);
    }

    screen_sorted_sprite_persn_render_cb(sspr);

    if (br_inc != 0)
    {
        ubyte *frv;
        ushort frm, k;
        frm = shield_frm[p_thing->ThingOffset & 3];
        k = ((gameturn + 16 * p_thing->ThingOffset) >> 2) & 7;
        frv = byte_15399C + 5 * k;

        draw_sorted_sprite1b(frv, frm, p_sspr->X, p_sspr->Y, br_inc, 0);
    }
}

ushort number_player_get_frame(struct Thing *p_person, ubyte n)
{
    struct PanelStyle *p_style;
    ushort ani_mdsh, ani_base;
    ushort frm;
    ushort screen_scale;

    p_style = game_panel_style;

    ani_base = p_style->AgentNumAnim;
    screen_scale = min(lbDisplay.GraphicsScreenHeight / 200, p_style->AgentNumDetails);
    if (screen_scale > 0)
        screen_scale--;
    ani_mdsh = 4 * screen_scale;

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
    return frm;
}

void draw_frame_on_map_coords(MapCoord cor_x, MapCoord cor_y, MapCoord cor_z,
  short scr_sh_x, short scr_sh_y, ushort frm, TbBool unscaled)
{
    struct ShEnginePoint sp;
    int cor_dt_x, cor_dt_y, cor_dt_z;

    cor_dt_x = cor_x - engn_xc;
    cor_dt_y = cor_y;
    cor_dt_z = cor_z - engn_zc;
    if ((cor_dt_x > TILE_TO_MAPCOORD(render_area_a,0)) ||
      (cor_dt_z > TILE_TO_MAPCOORD(render_area_b,0))) {
        return;
    }

    transform_shpoint(&sp, cor_dt_x, 8 * cor_dt_y - 8 * engn_yc, cor_dt_z);
    sp.X += ((overall_scale * scr_sh_x) >> 8);
    sp.Y += ((overall_scale * scr_sh_y) >> 8);

    //TODO switch to drawlists
    if (unscaled)
        draw_hud_frame_on_screen_unscaled_but_scale_pos(sp.X, sp.Y, frm, overall_scale);
    else
        draw_hud_frame_on_screen(sp.X, sp.Y, frm, overall_scale);
}

void number_player(struct Thing *p_person, ubyte n)
{
    int tng_cor_x, tng_cor_y, tng_cor_z;
    int shift_x, shift_y;
    ushort frm;
    TbBool unscaled;

    frm = number_player_get_frame(p_person, n);

    if ((p_person->Flag2 & TgF2_Unkn0002) != 0)
        return;

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
        ubyte angl;
        angl = (p_person->U.UObject.Angle + 2 - byte_176D49 + 8) & 7;
        shift_x = -lbSinTable[256 * angl + LbFPMath_PI/2] >> 14;
        if ((p_person->Flag2 & TgF2_Unkn00080000) == 0)
            shift_x /= 2;
    }
    shift_y = 0;

    unscaled = (lbDisplay.GraphicsScreenHeight < 400);

    draw_frame_on_map_coords(tng_cor_x, tng_cor_y, tng_cor_z, shift_x, shift_y, frm, unscaled);
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
    assert(screen_sorted_sprite_statc_render_cb != NULL);
    assert(screen_sorted_sprite_persn_render_cb != NULL);

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
      case DrIT_LongPropBar:
          draw_sort_sprite_long_prop_bar(itm->Offset);
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
      case DrIT_Number:
          draw_sort_sprite_number(itm->Offset);
          break;
      case DrIT_ShortText:
          draw_sort_sprite_short_text(itm->Offset);
          break;
      default:
          break;
      }
    }
}
/******************************************************************************/
