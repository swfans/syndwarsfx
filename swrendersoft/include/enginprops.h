/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginprops.h
 *     Header file for enginprops.c.
 * @par Purpose:
 *     Stores render engine properties, used for altering the rendering.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     13 Oct 2024 - 22 Dec 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINPROPS_H
#define ENGINPROPS_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum RenderFloorFlags {
  RendFlrF_NonPlanetary = 0x01,
  RendFlrF_WobblyTerrain = 0x02,
};

#pragma pack()
/******************************************************************************/
/** Animation turn for the animations controlled within the render engine.
 *
 * Animations which are independent of game action, like moving colours
 * on shiny text, may be implemented within the render engine completely.
 *
 * Such animations use this value as a measure of progressing time, and
 * therefore progressing animation frames.
 *
 * The value is expected to be incremented or set to game turns within
 * the game code.
 */
extern u32 render_anim_turn;

/** Floor rendering flags.
 *
 * Affects how the floor is rendered.
 */
extern u32 render_floor_flags;

/** Amount of available screen points, cached in render module to optimize speed.
 *
 * This variable is used many times while creating drawlist. It needs to be set
 * by the app based on memory allocation.
 */
extern s32 screen_points_limit;

/** Amount of available draw items, cached in render module to optimize speed.
 *
 * This variable is used many times while creating drawlist. It needs to be set
 * by the app based on memory allocation.
 */
extern s32 draw_items_limit;

/** Amount of available quadrangular game textures, cached in render module to optimize speed.
 *
 * This variable is used many times while creating drawlist. It needs to be set
 * by the app based on memory allocation.
 */
extern s32 game_textures_limit;

/** Amount of available triangular face textures, cached in render module to optimize speed.
 *
 * This variable is used many times while creating drawlist. It needs to be set
 * by the app based on memory allocation.
 */
extern s32 face_textures_limit;

/** Extra buffer, used as texture or mapping data if flags demand.
 * Declared and controlled by the app.
 */
extern ubyte *scratch_buf1;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
