/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file huddrwlstm.h
 *     Header file for huddrwlstm.c.
 * @par Purpose:
 *     Making drawlists for the HUD over 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef HUDDRWLSTM_H
#define HUDDRWLSTM_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

 struct TbSprite;

#pragma pack()
/******************************************************************************/

TbBool enlist_hud_draw_line(short beg_x, short beg_y, short end_x, short end_y,
  ushort drwflags, ubyte thickness, short brig, TbPixel colour);

TbBool enlist_hud_draw_box(short px, short py, short width, short height,
  ushort drwflags, short brig, TbPixel colour);

TbBool enlist_hud_draw_slant_box(short px, short py, short width, short height,
  ushort drwflags, short brig, TbPixel colour);

TbBool enlist_hud_draw_vslant_box(short px, short py, short width, short height,
  ushort drwflags, short brig, TbPixel colour);

TbBool enlist_hud_draw_low_trans_grey_box(short px, short py,
  short width, short height, ushort drwflags, short brig, TbPixel colour);

TbBool enlist_hud_draw_low_trans_grey_slant_box(short px, short py,
  short width, short height, ushort drwflags, short brig, TbPixel colour);

TbBool enlist_hud_draw_low_trans_grey_vslant_box(short px, short py,
  short width, short height, ushort drwflags, short brig, TbPixel colour);

TbBool enlist_hud_draw_textured_flow_slant_box(short px, short py,
  short width, short height, ubyte vecmode,
  ubyte brig, ubyte tmapno, ushort num_ua, ushort num_vb);

TbBool enlist_hud_draw_sprite(short px, short py, struct TbSprite *p_spr,
  ushort drwflags, short brig);

TbBool enlist_hud_draw_sprite_scaled(short px, short py,
  struct TbSprite *p_spr, short dest_width, short dest_height,
  ushort drwflags, short brig);

TbBool enlist_hud_draw_clipped_text(short px, short py,
  short width, short height, short shift_x, short shift_y,
  struct TbSprite *p_font, const char *text,
  short units_per_px, short brig, TbPixel colour);

/** Check height of line-wrapped text with shadow colour flash effect.
 */
int get_width_shad_cl_flash_wrapped_text(short px, short py,
  struct TbSprite *p_font, const char *text, short units_per_px);

/** Enlist drawing line-wrapped text with shadow colour flash effect.
 */
TbBool enlist_hud_draw_shad_cl_flash_wrapped_text(short px, short py,
  short width, short height, struct TbSprite *p_font, const char *text,
  short units_per_px, short timer, TbPixel colour, TbPixel shcolour);

/** Enlist drawing line-wrapped text with colour brightness wave effect.
 */
TbBool enlist_hud_draw_colour_wave_wrapped_text(short px, short py,
  short width, short height, struct TbSprite *p_font, const char *text,
  short units_per_px, TbPixel colour, TbPixel shcolour);

/** Enlist drawing line between the provided map coordinates.
 */
TbBool enlist_hud_draw_mapcoord_line(short cor1_x, short cor1_y,
  short cor1_z, short cor2_x, short cor2_y, short cor2_z,
  ushort drwflags, ubyte thickness, TbPixel colour);

/** Enlist drawing a frame on the provided map coordinates.
 */
TbBool enlist_hud_draw_mapcoord_frame_one_colour(short cor_x, short cor_y,
  short cor_z, ushort frm, ushort drwflags, TbPixel colour);

TbBool enlist_hud_draw_mapcoord_sprites_in_quarters_unscaled(short cor_x,
  short cor_y_m8, short cor_z, struct TbSprite * const p_sprlst[],
  ushort drwflags, short radius);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
