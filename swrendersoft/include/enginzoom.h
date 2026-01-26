/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginzoom.h
 *     Header file for enginzoom.c.
 * @par Purpose:
 *     Zoom level handling for the 3D engine.
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
#ifndef ENGINZOOM_H
#define ENGINZOOM_H

#include "bftypes.h"
#include "poly.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Smallest size of render area. Must be even to avoid shifted terrain.
 */
#define RENDER_AREA_MIN 2

/** Largest size of render area. Must be even to avoid shifted terrain.
 */
#define RENDER_AREA_MAX 88

/** Range of allowed blocks-to-zoom conversion.
 */
#define ZOOM_RANGE_BLOCKS_LIMIT 27

#pragma pack(1)

#pragma pack()
/******************************************************************************/
extern ubyte byte_176D48;
extern ubyte byte_176D49;
extern ubyte byte_176D4A;
extern ubyte byte_176D4B;

extern ubyte byte_19EC7A;

/** Minimum user zoom (when most area is visible). */
extern short user_zoom_min;
/** Maxumum user zoom (largest magnification). */
extern short user_zoom_max;

extern ushort render_area_a;
extern ushort render_area_b;

void zoom_update(short zoom_min, short zoom_max);

ushort get_scaled_zoom(ushort zoom);
ushort get_unscaled_zoom(ushort szoom);
int get_zoom_from_range_bloks(ushort range_blocks);

short get_overall_scale_min(void);
short get_overall_scale_max(void);

short get_render_area_for_zoom(short zoom);
short bound_render_area(short rarea);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
