/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginpriobjs.h
 *     Header file for enginpriobjs.c.
 * @par Purpose:
 *     Primitive objects support required by the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     29 Sep 2023 - 02 Jan 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINPRIOBJS_H
#define ENGINPRIOBJS_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct SingleObject;
struct SinglePoint;
struct SingleObjectFace3;
struct SingleObjectFace4;

enum PrimitiveElement {
    PriEl_NONE = 0,
    PriEl_PRIM_POINT,
    PriEl_PRIM_FACE3,
    PriEl_PRIM_FACE4,
    PriEl_PRIM_TEXTR3,
    PriEl_PRIM_TEXTR4,
    PriEl_PRIM_OBJ,
    PriEl_GAME_POINT,
    PriEl_GAME_FACE3,
    PriEl_GAME_FACE4,
    PriEl_GAME_TEXTR3,
    PriEl_GAME_TEXTR4,
    PriEl_GAME_OBJ,
};

#pragma pack()
/******************************************************************************/
extern struct SinglePoint *prim_object_points;
extern ushort next_prim_object_point;

extern struct SingleObjectFace3 *prim_object_faces3;
extern ushort next_prim_object_face3;

extern struct SingleObjectFace4 *prim_object_faces4;
extern ushort next_prim_object_face4;

extern struct SingleObject *prim_objects;
extern ushort next_prim_object;

extern ushort prim_unknprop01;

extern ushort unkn2_pos_x;
extern ushort unkn2_pos_y;
extern ushort unkn2_pos_z;

void read_primveh_obj(const char *fname, int a2);

ushort copy_prim_obj_to_game_object(short tx, short tz, short a3, short ty);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
