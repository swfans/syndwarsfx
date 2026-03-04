/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_expld.h
 *     Header file for thing_expld.c.
 * @par Purpose:
 *     Support for creating and progressing explosions of 3D objects on map.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 05 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef THING_EXPLD_H
#define THING_EXPLD_H

#include "bftypes.h"
#include "game_bstype.h"

#pragma pack(1)

struct SimpleThing;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

void process_explode(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
