/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_onface.h
 *     Header file for thing_onface.c.
 * @par Purpose:
 *     Support for checking if thing is on 3D face and putting it on a face.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 17 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef THING_ONFACE_H
#define THING_ONFACE_H

#include "bftypes.h"
#include "game_bstype.h"

#pragma pack(1)

struct PolyPoint;
struct Thing;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

ushort set_thing_height_on_face_tri(struct Thing *p_thing, int x, int z, short face);
ushort set_thing_height_on_face_quad(struct Thing *p_thing, int x, int z, short face);
ushort set_thing_height_on_face(struct Thing *p_thing, int x, int z, short face);

short find_and_set_connected_face(struct Thing *p_thing, int x, int z, short face);

void check_mouse_over_face(struct PolyPoint *p_pt1, struct PolyPoint *p_pt2,
  struct PolyPoint *p_pt3, int face, int type);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
