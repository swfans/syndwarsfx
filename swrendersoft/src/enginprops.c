/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginprops.c
 *     Stores render engine properties, used for altering the rendering.
 * @par Purpose:
 *     Implement functions for maintaining the properties.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     13 Oct 2024 - 22 Dec 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginprops.h"

#include "privrdlog.h"
/******************************************************************************/
u32 render_anim_turn = 0;
u32 render_anim_speed = 80;
u32 render_floor_flags = 0;
u32 render_faces_flags = 0;

s32 screen_points_limit = 16;
s32 draw_items_limit = 16;
s32 game_textures_limit = 16;
s32 game_anim_tmaps_limit = 16;
s32 game_object_points_limit = 16;
s32 game_object_faces3_limit = 16;
s32 game_object_faces4_limit = 16;
s32 game_normals_limit = 16;
s32 game_objects_limit = 8;
s32 face_textures_limit = 16;

s32 game_special_obj_faces3_limit = 16;
s32 game_special_obj_faces4_limit = 16;

s32 prim_object_faces3_limit = 16;
s32 prim_object_faces4_limit = 16;

u32 stat_drawlist_faces = 0;

ubyte *scratch_buf1 = NULL;

void (*prim_obj_mem_debug)(ubyte itm_kind, int itm_beg, int itm_end) = NULL;
/******************************************************************************/

/******************************************************************************/
