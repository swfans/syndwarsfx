/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginfloor.h
 *     Header file for enginfloor.c.
 * @par Purpose:
 *     Support for 3D map floor tiles.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     13 Oct 2024 - 06 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINFLOOR_H
#define ENGINFLOOR_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct SingleFloorTexture;

struct FloorTile { // sizeof=39
    short X[4];
    short Y[4];
    struct SingleFloorTexture *Texture;
    ubyte V[4];
    short Shade[4];
    ubyte Col;
    ubyte Flags;
    ubyte Flags2;
    ubyte Flags2b;
    ubyte TearAround;	/**< Which around tiles have surface damaged/torn */
    short Offset;
};

#pragma pack()
/******************************************************************************/
extern struct FloorTile *game_floor_tiles;
extern short next_floor_tile;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
