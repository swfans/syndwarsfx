/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file hud_draw.h
 *     Header file for hud_draw.c.
 * @par Purpose:
 *     Ingame Heads-Up Display drawing routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef HUD_DRAW_H
#define HUD_DRAW_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/

void draw_mapcoord_line_directed_to_target(int probe_x, int probe_y, int probe_z,
  int target_x, int target_y, int target_z, TbPixel colour);

TbBool enlist_hud_draw_line_from_thing_to_mapcoord(ThingIdx thing,
  short cor2_x, short cor2_y_m8, short cor2_z, ushort drwflags, ubyte colkp);
TbBool enlist_hud_draw_line_from_thing_ptr_to_mapcoord(struct Thing *p_thing,
  short cor2_x, short cor2_y_m8, short cor2_z, ushort drwflags, ubyte colkp);

TbBool enlist_hud_draw_line_from_thing_to_scrcoord(ThingIdx thing,
  short scr_x, short scr_y, ushort drwflags, ubyte colkp);
TbBool enlist_hud_draw_line_from_thing_ptr_to_scrcoord(struct Thing *p_thing,
  short scr_x, short scr_y, ushort drwflags, ubyte colkp);

void set_netplayer_name_text_over_thing(ScrCoord x, ScrCoord y,
  struct Thing *p_thing);

void draw_netplayer_name_text_over_thing(void);

void draw_hud(int dcthing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
