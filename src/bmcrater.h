/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bmcrater.h
 *     Header file for bmcrater.c.
 * @par Purpose:
 *     On-map explosion effect implementation.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Sep 2023 - 17 Mar 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BMCRATER_H
#define BMCRATER_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct MapCreater {
    short MapX;
    short MapY;
    short Depth;
    short Iterations;
};

struct Thing;

#pragma pack()
/******************************************************************************/

void create_crater(short tile_x, short tile_y, short depth);

void quick_crater(int tl_cent_x, int tl_cent_z, int size);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
