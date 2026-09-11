/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_expld.h
 *     Header file for thing_expld.c.
 * @par Purpose:
 *     Support for creating and progressing explosions of 3D objects on map.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 05 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef THING_EXPLD_H
#define THING_EXPLD_H

#include "bftypes.h"
#include "game_bstype.h"

#pragma pack(1)

struct SimpleThing;
struct Thing;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern u32 dont_bother_with_explode_faces;
extern s32 expl_unkn_cor_x;
extern s32 expl_unkn_cor_z;

void init_object_explode_faces(void);
void process_explode(void);
void draw_explode(void);

void thing_explode_faces(struct Thing *p_thing);
void floor_explode_faces(short tile_x, short tile_z);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
