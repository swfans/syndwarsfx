/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engintrns.h
 *     Header file for engintrns.c.
 * @par Purpose:
 *     Math transformations required by the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINTRNS_H
#define ENGINTRNS_H

#include "bftypes.h"
#include "bfmath.h"
#include "poly.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

// TODO figure out why we have PolyPoint and EnginePoint
struct EnginePoint {
  struct PolyPoint pp;
  /*long X;
  long Y;
  long TMapX;
  long TMapY;
  long Shade;*/
  long X3d;
  long Y3d;
  long Z3d;
  long DistSqr;
  short EPUnkn24;
  ubyte Flags; // TODO check if it is ushort
};

/** Short and Shaded version of Engine Point.
 */
struct ShEnginePoint {
    short X;
    short Y;
    /** Depth at which the object is on screen */
    int Depth;
    ubyte Flags;
    /** Shade value from atmosphere reflection */
    short ReflShade;
    /** Shade value from lights */
    short Shade;
};

#pragma pack()
/******************************************************************************/
extern long cam_tilt;

extern long dword_176D0C;
extern long dword_176D10;
extern long dword_176D14;
extern long dword_176D18;
extern long dword_176D1C;
extern long dword_176D3C;
extern long dword_176D40;
extern long dword_176D44;
extern long dword_176D4C;
extern long cam_rotation_velocity;
extern long dword_176D64;

short angle_between_points(int x1, int z1, int x2, int z2);

void local_to_worldr(int *dx, int *dy, int *dz);

/** Transform map coordinates from EnginePoint into screen position.
 *
 * Given map coordinates within the game engine, transforms them
 * to 2D screen coordinated based on the current camera and stores
 * within a PolyPoint. Depth value is not computed.
 */
void transform_point(struct EnginePoint *p_ep);

/** Transform engine map coordinates into screen position within ShEnginePoint.
 *
 * Given map coordinates within the game engine, transforms them
 * to 2D screen coordinated (and depth) based on the current camera.
 */
void transform_shpoint(struct ShEnginePoint *p_sp, int dxc, int dyc, int dzc);
void transform_shpoint_fpv(struct ShEnginePoint *p_sp, int dxc, int dyc, int dzc);

/** Transform coordinates Like transform_shpoint(), but only Y coord is returned.
 *
 * If you need both coords, transforming them at the same time is much faster.
 */
int transform_shpoint_y(int dxc, int dyc, int dzc);

void process_engine_unk1(void);
void process_engine_unk2(void);
void setup_engine_nullsub4(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
