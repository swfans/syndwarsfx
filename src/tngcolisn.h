/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file tngcolisn.h
 *     Header file for tngcolisn.c.
 * @par Purpose:
 *     Thing collisions support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     04 Sep 2024 - 11 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef TNGCOLISN_H
#define TNGCOLISN_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;

struct ColVect { // sizeof=14
  short X1;
  short Y1;
  short Z1;
  short X2;
  short Y2;
  short Z2;
  short Face;
};

/** Collision vectors list.
 *
 * Contains a list of references to boundary vectors used for stepping
 * between ground faces and object faces (buildings).
 */
struct ColVectList { // sizeof=6
  ushort Vect; /**< Index of the ColVect with geometry vector. */
  ushort NextColList; /**< Index of the next ColVectList entry in a chain list, top bit is passability. */
  short Object; /**< Index of a Thing containing the object whose geometry has that vector. */
};

#pragma pack()
/******************************************************************************/
extern struct ColVectList *game_col_vects_list;
extern ushort next_vects_list;

extern struct ColVect *game_col_vects;
extern ushort next_col_vect;

extern ubyte debug_hud_collision;

/******************************************************************************/

void set_dome_col(struct Thing *p_building, ubyte flag);

void draw_engine_unk3_last(short x, short z);

/** Creates a new collision vector between given points.
 *
 * Alloccates `ColVect`, and inserts collision markers for it
 * into the `mapwho`.
 *
 * @return Returns the new `ColVect` index, or 0 on failure.
 */
ushort dynamic_insert_vect(s32 x1, s32 y1, s32 z1, s32 x2,
  s32 y2, s32 z2, int owner, ubyte vtype);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
