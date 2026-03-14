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

/** Add a new draw item and return linked special quad face instance, but no points.
 * Reserves the face, but leaves points for the caller to set.
 *
 * @param ditype Draw item type, should be one of SingleObjectFace4 related types.
 * @param bckt Destination bucket for this draw item.
 * @return SingleObjectFace4 instance to fill, or NULL if arrays exceeded.
 */
struct SingleObjectFace4 *draw_item_add_special_obj_face4_no_pts(ubyte ditype, int bckt);

/** Add a new draw item and return linked special quad face instance.
 * Reserves and sets 4 SpecialPoint instances for the face.
 *
 * @param ditype Draw item type, should be one of SingleObjectFace4 related types.
 * @param bckt Destination bucket for this draw item.
 * @return SingleObjectFace4 instance to fill, or NULL if arrays exceeded.
 */
struct SingleObjectFace4 *draw_item_add_special_obj_face4(ubyte ditype, int bckt);

void enlist_draw_mapwho_vect_len(int x1, int y1, int z1,
  int x2, int y2, int z2, int len, int col);

void enlist_draw_frame_graphic(int x, int y, int z, ushort frame,
  int radius, int intensity, int depth_shift, intptr_t p_sitm);

void enlist_draw_frame_graphic_scale(int x, int y, int z, ushort frame,
  int radius, int intensity, int scale, intptr_t p_sitm);

void enlist_draw_tall_spr_shadow(int scr_x, int scr_y, int scr_depth, ushort frm,
  ubyte angl, ubyte shangl, ushort shpak, short strng, intptr_t p_sitm);

void enlist_draw_frame_pers_basic(int scr_x, int scr_y, int scr_depth, int frame,
  ubyte angl, short bright, intptr_t p_sitm);

void enlist_draw_frame_pers_rot_versioned(int scr_x, int scr_y, int scr_depth,
  int frame, ubyte *frv, ubyte angl, short bright, intptr_t p_sitm);

/** Enlist drawing a frame containing versioned effect.
 *
 * @param scr_x Screen coordinate.
 * @param scr_y Screen coordinate.
 */
void enlist_draw_frame_effect_versioned(int scr_x, int scr_y, int scr_depth,
  int frame, ubyte *frv, ubyte angl, short bright, intptr_t p_sitm);

/** Enlist drawing flame sprites in a list.
 *
 * @param flame_beg Index in `FIRE_flame` array of a first item in linked list.
 */
void enlist_draw_fire_flames(ushort flame_beg);

/** Enlist drawing a number at given map coordinates.
 *
 * @param x Map coodrinate.
 * @param y Map coodrinate.
 * @param z Map coodrinate.
 * @param num The number to draw.
 * @param radius Value which alters depth (distance from camera).
 * @param colour The text colour.
 */
void enlist_draw_number(int x, int y, int z, short scr_dx, short scr_dy,
  int num, int radius, TbPixel colour);

/** Enlist drawing a text at given map coordinates.
 *
 * @param x Map coodrinate.
 * @param y Map coodrinate.
 * @param z Map coodrinate.
 * @param text The text to draw.
 * @param radius Value which alters depth (distance from camera).
 * @param colour The text colour.
 */
void enlist_draw_text(int x, int y, int z, short scr_dx, short scr_dy,
  const char *text, int radius, TbPixel colour);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
