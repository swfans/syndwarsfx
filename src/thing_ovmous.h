/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_ovmous.h
 *     Header file for thing_ovmous.c.
 * @par Purpose:
 *     Support for checking if thing sprite overlaps with (is under) mouse.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     09 Oct 2025 - 22 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef THING_OVMOUS_H
#define THING_OVMOUS_H

#include "bftypes.h"
#include "game_bstype.h"

#pragma pack(1)

struct Thing;

#pragma pack()

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

ubyte check_mouse_overlap_item(ushort sspr);
ubyte check_mouse_overlap(ushort sspr);
ubyte check_mouse_overlap_corpse(ushort sspr);
ubyte check_mouse_over_unkn2(ushort sspr, struct Thing *p_thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
