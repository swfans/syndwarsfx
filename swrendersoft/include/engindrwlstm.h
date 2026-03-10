/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstm.h
 *     Header file for engindrwlstm.c.
 * @par Purpose:
 *     Making drawlists for the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINDRWLSTM_H
#define ENGINDRWLSTM_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct SortLine;
struct SortSprite;
struct SpecialPoint;
struct FloorTile;

#pragma pack()
/******************************************************************************/

/** Add a new draw item and return linked SortLine instance.
 *
 * @param ditype Draw item type, should be one of SortLine related types.
 * @param bckt Destination bucket for this draw item.
 * @return SortLine instance to fill, or NULL if arrays exceeded.
 */
struct SortLine *draw_item_add_line(ubyte ditype, int bckt);

/** Add a new draw item and return linked SortSprite instance.
 *
 * @param ditype Draw item type, should be one of SortSprite related types.
 * @param bckt Destination bucket for this draw item.
 * @return SortSprite instance to fill, or NULL if arrays exceeded.
 */
struct SortSprite *draw_item_add_sprite(ubyte ditype, int bckt);

/** Add a new draw item and return first of linked SpecialPoint instances.
 *
 * @param ditype Draw item type, should be one of SpecialPoint related types.
 * @param bckt Destination bucket for this draw item.
 * @param npoints Amount of consecutive points to reserve.
 * @return SpecialPoint instance to fill, or NULL if arrays exceeded.
 */
struct SpecialPoint *draw_item_add_points(ubyte ditype, ushort offset,
  int bckt, ushort npoints);

/** Add a new draw item and return linked FloorTile instance.
 *
 * @param ditype Draw item type, should be one of FloorTile related types.
 * @param bckt Destination bucket for this draw item.
 * @return FloorTile instance to fill, or NULL if arrays exceeded.
 */
struct FloorTile *draw_item_add_floor_tile(ubyte ditype, int bckt);

/** Add a new draw item and return linked special quad face instance.
 * Reserves and sets 4 SpecialPoint instances for the face.
 *
 * @param ditype Draw item type, should be one of SingleObjectFace4 related types.
 * @param bckt Destination bucket for this draw item.
 * @return SingleObjectFace4 instance to fill, or NULL if arrays exceeded.
 */
struct SingleObjectFace4 *draw_item_add_special_obj_face4(ubyte ditype, int bckt);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
