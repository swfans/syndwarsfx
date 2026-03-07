/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file tngobjdrw.h
 *     Header file for tngobjdrw.c.
 * @par Purpose:
 *     Draws thing objects using drawlist.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     04 Sep 2024 - 11 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef TNGOBJDRW_H
#define TNGOBJDRW_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/

void generate_shadows_for_multicolor_sprites(void);

short draw_thing_object(struct Thing *p_thing);

#define draw_sthing_object(p_sthing) draw_thing_object((struct Thing *)p_sthing)

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
