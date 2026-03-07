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

#include <assert.h>

#include "enginbckt.h"
#include "enginfloor.h"
#include "enginshadws.h"
#include "enginsngtxtr.h"
/******************************************************************************/

ushort tnext_draw_item = 0;

ushort tnext_sort_sprite = 0;
//extern ushort tnext_sort_line; -- no such var?
//extern ushort tnext_special_obj_face;
ushort tnext_special_obj_face4 = 1;

ushort tnext_screen_point = 0;

/******************************************************************************/
// from engindrwlstx_spr
void draw_sort_line1a(ushort sln);
void draw_sort_sprite1c(ushort sspr);
void draw_hud_frame_on_screen_unscaled_but_scale_pos(short scr_x, short scr_y, ushort frm, int sscale);
void draw_hud_frame_on_screen(short scr_x, short scr_y, ushort frm, int sscale);
void draw_sort_sprite_frame_pers_v(int sspr);
void draw_sort_sprite_frame_pers_b(int sspr);
void draw_sort_sprite_frame_efct_v(int sspr);
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

    next_special_obj_face = 1;

    tnext_special_obj_face4 = next_special_obj_face4;
    next_special_obj_face4 = 1;

    tnext_floor_texture = next_floor_texture;

    next_floor_tile = 1;
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
      case DrIT_SFrmStatc:
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
      case DrIT_SpObFace4:
          draw_special_object_face4(itm->Offset);
          break;
      case DrIT_SFrmPersV:
          draw_sort_sprite_frame_pers_v(itm->Offset);
          break;
      case DrIT_SFrmPersB:
          draw_sort_sprite_frame_pers_b(itm->Offset);
          break;
      case DrIT_SFrmEfctV:
          draw_sort_sprite_frame_efct_v(itm->Offset);
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
      case DrIT_SFrmStatc:
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
      case DrIT_SpObFace4:
          draw_special_object_face4(itm->Offset);
          break;
      case DrIT_SFrmPersV:
          draw_sort_sprite_frame_pers_v(itm->Offset);
          break;
      case DrIT_SFrmPersB:
          draw_sort_sprite_frame_pers_b(itm->Offset);
          break;
      case DrIT_SFrmEfctV:
          draw_sort_sprite_frame_efct_v(itm->Offset);
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
      case DrIT_SPersShdw:
          draw_sort_sprite_person_shadow(itm->Offset);
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
