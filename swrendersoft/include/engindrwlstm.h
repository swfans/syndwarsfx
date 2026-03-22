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

/** Skip drawing the face if flag20 set by transform.
 *
 * This flag is for `enlist_draw_face*()`.
 */
#define EnFaceF_SkipFlg20 0x0100

/** The face is reflective - it reflects sky box texture.
 *
 * This flag is for `enlist_draw_face*()`.
 */
#define EnFaceF_Reflective 0x0200

/** The face is semi-transparent - the area behind it is visible.
 *
 * This flag is for `enlist_draw_face*()`.
 */
#define EnFaceF_SemiTranspr 0x0400

/** The face is belongs to a moving objects - has different light calc.
 *
 * This flag is for `enlist_draw_face*()`.
 */
#define EnFaceF_MovingObject 0x0800

struct SortLine;
struct SortSprite;
struct SpecialPoint;
struct FloorTile;
struct SingleObject;
struct SingleObjectFace4;
struct SingleFloorTexture;

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

/** Enlist drawing flame sprites from a list.
 *
 * @param flame_beg Index in `FIRE_flame` array of a first item in linked list.
 */
void enlist_draw_fire_flames(ushort flame_beg);

/** Enlist drawing phwoar sprites from a list.
 *
 * @param phwoar_beg Index in `phwoar` array of a first item in linked list.
 */
void enlist_draw_bang_phwoars(ushort phwoar_beg);

/** Enlist drawing shrapnel sprites from a list.
 *
 * @param shrapnel_beg Index in `shrapnel` array of a first item in linked list.
 */
void enlist_draw_bang_shrapnels(ushort shrapnel_beg);

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

struct SingleObjectFace4 *build_polygon_slice(short x1, short y1,
  short x2, short y2, int w1, int w2, int col, int sort_key, ushort flag);

void build_polygon_circle(int x1, int y1, int z1, int r1, int r2, int flag,
  struct SingleFloorTexture *p_tex, int col, int bright1, int bright2);

struct SingleObjectFace4 *build_glare(short x1, short y1, short z1, short r1);

void enlist_draw_wobble_line(int x1, int y1, int z1,
 int x2, int y2, int z2, int itime, ubyte slflags, TbBool is_player);

void enlist_draw_bang_wobble_line(ushort shrapnel_beg);

void enlist_draw_laser(int x1, int y1, int z1, int x2, int y2, int z2,
  int depth_shift, int itime, short ofs_x, short ofs_y, TbPixel colour);

TbBool enlist_draw_face3_prealloc(int face, short depth_shift,
  ushort edflags, int *bckt_max);

TbBool enlist_draw_face4_prealloc(int face, short depth_shift,
  ushort edflags, int *bckt_max);

/** Enlist drawing a pole, made of 2 points within quad face.
 */
TbBool enlist_draw_face4_pole(int cor_dx, int cor_dy, int cor_dz,
  int face, short depth_shift, int *bckt_max);

void enlist_draw_plasma_sparks_on_object(struct SingleObject *point_object);

void enlist_draw_long_health_bar(int cor_x, int cor_y, int cor_z,
  int depth_shift, int bckt, int val, int val_max,
  intptr_t p_sitm, TbPixel lvl_col, TbPixel bar_col);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
