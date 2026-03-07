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

#define MAX_WALKABLE_STEEPNESS (21 * LbFPMath_PI / 180)

enum RenderFloorFlags {
  RendFlrF_NonPlanetary = 0x01,
  RendFlrF_WobblyTerrain = 0x02,
};

enum RenderFacesFlags {
  RendFacF_Perspectv3SkipWireframe = 0x01,
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

/** Animation speed limiter.
 *
 * Lowering this will speed up animation. Increasing will only have effect
 * to some point, as values have in-code limits. TODO - maybe turn this into
 * just speed, not a limit?
 */
extern u32 render_anim_speed;

/** Floor rendering flags.
 *
 * Affects how the floor is rendered.
 */
extern u32 render_floor_flags;

/** Faces rendering flags.
 *
 * Affects how the faces are rendered.
 */
extern u32 render_faces_flags;

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

/** Amount of available animated texture maps.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_anim_tmaps_limit;

/** Amount of available points/vertices for game objects, making up faces.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_object_points_limit;

/** Amount of available triangular faces for game objects.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_object_faces3_limit;

/** Amount of available quadrangular faces for game objects.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_object_faces4_limit;

/** Amount of available normal vectors, to be linked to both tri and quad faces.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_normals_limit;

/** Amount of available game objects, representing the whole 3D primitives.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_objects_limit;

/** Amount of available triangular faces for game special objects.
 *
 * This variable is used while creating 3D primitives of special objects.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_special_obj_faces3_limit;

/** Amount of available quadrangular faces for game special objects.
 *
 * This variable is used while creating 3D primitives of special objects.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 game_special_obj_faces4_limit;

/** Amount of available triangular faces for primitives objects.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 prim_object_faces3_limit;

/** Amount of available quadrangular faces for primitives objects.
 *
 * This variable is used while creating, altering or making a copy of 3D primitive.
 * It needs to be set by the app based on memory allocation.
 */
extern s32 prim_object_faces4_limit;

/** Extra buffer, used as texture or mapping data if flags demand.
 * Declared and controlled by the app.
 */
extern ubyte *scratch_buf1;

/** Amount of faces for whom drawlist add was called, for statistics.
 */
extern u32 stat_drawlist_faces;

/** Callback for debug of 3D objects primitives.
 */
extern void (*prim_obj_mem_debug)(ubyte itm_kind, int itm_beg, int itm_end);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
