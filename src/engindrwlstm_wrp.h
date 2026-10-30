/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstm_wrp.h
 *     Header file for engindrwlstm_wrp.c.
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
#ifndef ENGINDRWLSTM_WRP_H
#define ENGINDRWLSTM_WRP_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct SimpleThing;
struct SingleObject;
struct SingleFloorTexture;

#pragma pack()
/******************************************************************************/

void calc_bul_offsets(void);
void get_bullet_start_offsets(short *p_ofs_x, short *p_ofs_y, const struct Thing *p_person);

void draw_thing_e_graphic(struct Thing *p_thing, int x, int y, int z, ushort frame,
  int radius, int intensity);
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
void draw_vehicle_health(struct Thing *p_thing, int bckt);

int mech_unkn_func_03(struct Thing *p_thing);

void build_laser(int cor1_x, int cor1_y, int cor1_z,
  int cor2_x, int cor2_y, int cor2_z,
  int itime, struct Thing *p_owner, TbPixel colour);

void build_electricity_strand(struct SimpleThing *p_sthing, ubyte itime);

void build_electricity(int cor1_x, int cor1_y, int cor1_z,
  int cor2_x, int cor2_y, int cor2_z, int itime, struct Thing *p_owner);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
