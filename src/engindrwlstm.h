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

struct FloorTile;
struct Thing;
struct SimpleThing;
struct SingleObject;
struct SingleFloorTexture;

#pragma pack()
/******************************************************************************/
extern ubyte game_perspective;
extern ushort overall_scale;
extern long dword_176D68;

void draw_mapwho_vect_len(int x1, int y1, int z1, int x2, int y2, int z2, int len, int col);
void draw_e_graphic(int x, int y, int z, ushort frame, int radius, int intensity, struct Thing *p_thing);
void draw_e_graphic_scale(int x, int y, int z, ushort frame, int radius, int intensity, int scale);
void draw_pers_e_graphic(struct Thing *p_thing, int x, int y, int z, int frame, int radius, int intensity);
void FIRE_draw_fire(struct SimpleThing *p_sthing);
void draw_bang(struct SimpleThing *p_pow);

/** Adds primitives for a rotated 3D object to drawlist.
 *
 * @return Gives max depth value shifted to bucket index.
 */
int draw_rot_object(int offset_x, int offset_y, int offset_z, struct SingleObject *point_object, struct Thing *p_thing);
short draw_rot_object2(int offset_x, int offset_y, int offset_z, struct SingleObject *point_object, struct Thing *p_thing);
short draw_object(int x, int y, int z, struct SingleObject *point_object);
void draw_vehicle_health(struct Thing *p_thing);

void build_polygon_circle(int x1, int y1, int z1, int r1, int r2, int flag, struct SingleFloorTexture *p_tex, int col, int bright1, int bright2);
void build_laser(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner, int colour);
struct SingleObjectFace4 *build_glare(short x1, short y1, short z1, short r1);

// Lowest level functions, to be used only if previous ones really do not match

struct FloorTile *draw_item_add_floor_tile(ubyte ditype, int bckt);

// Mouse overlap, to be moved somewhere matching

ubyte check_mouse_overlap_item(ushort sspr);
ubyte check_mouse_overlap(ushort sspr);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
