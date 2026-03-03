/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_fire.h
 *     Header file for thing_fire.c.
 * @par Purpose:
 *     Support for creating fires burning on the map.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 25 Aug 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef THING_FIRE_H
#define THING_FIRE_H

#include "bftypes.h"
#include "game_bstype.h"

#pragma pack(1)

struct SimpleThing;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void FIRE_new(int x, int y, int z, ubyte type);

void set_static_on_fire(struct SimpleThing *p_static);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
