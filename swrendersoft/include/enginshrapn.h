/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginshrapn.h
 *     Header file for enginshrapn.c.
 * @par Purpose:
 *     Shrapnel structures required by the 3D engine.
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
#ifndef ENGINSHRAPN_H
#define ENGINSHRAPN_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Shrapnel {
    int x;
    int y;
    int z;
    sbyte vx;
    sbyte vy;
    sbyte vz;
    ubyte yaw;
    ubyte pitch;
    sbyte vyaw;
    sbyte vpitch;
    ubyte type;
    ushort PointOffset;
    ushort child;
};

struct Phwoar {
    int x;
    int y;
    int z;
    sbyte vx;
    sbyte vy;
    sbyte vz;
    ubyte type;
    ubyte rabbit;
    ubyte gestation;
    sbyte die;
    ubyte vf;
    ushort f;
    ushort PointOffset;
    ushort child;
    ubyte fc;
    ubyte shit;
};

struct FireFlame { // sizeof=20
    ubyte type;
    ubyte count;
    sbyte fvel;
    sbyte fcount;
    sbyte big;
    sbyte dbig;
    sbyte ddbig;
    ubyte life;
    ushort frame;
    short x;
    short y;
    short z;
    ushort PointOffset;
    ushort next;
};

#pragma pack()
/******************************************************************************/
extern struct Shrapnel shrapnel[512];
extern struct Phwoar phwoar[1024];
extern struct FireFlame FIRE_flame[512];

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
