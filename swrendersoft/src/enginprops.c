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
u32 render_floor_flags = 0;

s32 screen_points_limit = 16;
s32 draw_items_limit = 16;
s32 game_textures_limit = 16;
s32 face_textures_limit = 16;
/******************************************************************************/

/******************************************************************************/
