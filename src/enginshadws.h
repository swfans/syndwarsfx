/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginshadws.h
 *     Header file for enginshadws.c.
 * @par Purpose:
 *     Shadows preparation and drawing required by the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 13 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINSHADWS_H
#define ENGINSHADWS_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/

void draw_sort_sprite_person_shadow(ushort sspr);
void draw_vehicle_shadow(ushort veh, ushort sort);

void generate_shadows_for_multicolor_sprites(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
