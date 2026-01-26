/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginzoom.c
 *     Zoom level handling for the 3D engine.
 * @par Purpose:
 *     Implements functions for modifying zoom level and its limits.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Sep 2023 - 17 Mar 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginzoom.h"

#include "bfmath.h"
#include "bfutility.h"

#include "engincolour.h"
#include "privrdlog.h"
/******************************************************************************/

ushort render_area_a = 24;
ushort render_area_b = 24;

short user_zoom_min = 127;
short user_zoom_max = 260;

ushort zoom_levels[ZOOM_RANGE_BLOCKS_LIMIT+1] = {
    256, 256, 256, 256, 256, 256, 240, 230,
    220, 210, 200, 190, 180, 170, 170, 170,
    170, 170, 170, 170, 170, 165, 160, 155,
    150, 145, 140, 135,
};

#define RANGE1 7
#define RANGE2 15
#define FACTOR1 2
#define FACTOR2 8
#define FACTOR3 21
void zoom_update(short zoom_min, short zoom_max)
{
    int i, dt;
    int zoom_arr_min, zoom_arr_max;

    dt = (zoom_max - zoom_min);
    zoom_arr_min = zoom_min + ((dt * 16) >> 8);
    zoom_arr_max = zoom_max - ((dt * 8) >> 8);
    for (i = 0; i < ZOOM_RANGE_BLOCKS_LIMIT+1; i++)
    {
        int n, val;
        // The curve has 3 steepness levels, set (from right) in FACTOR defines
        val = zoom_arr_min;
        if (i > 0) {
            if (i < RANGE1)
                n = FACTOR1 * (i - 0);
            else
                n = FACTOR1 * (RANGE1 - 0);
            val += ((dt * n) >> 8);
        }

        if (i > RANGE1) {
            if (i < RANGE2)
                n = FACTOR2 * (i - RANGE1);
            else
                n = FACTOR2 * (RANGE2 - RANGE1);
            val += ((dt * n) >> 8);
        }

        if (i > RANGE2) {
            n = FACTOR3 * (i - RANGE2);
            val += ((dt * n) >> 8);
        }

        if (val > zoom_arr_max)
            val = zoom_arr_max;
        zoom_levels[ZOOM_RANGE_BLOCKS_LIMIT-i] = val;
    }
    user_zoom_min = zoom_min;
    user_zoom_max = zoom_max;
}
#undef RANGE1
#undef RANGE2
#undef FACTOR1
#undef FACTOR2
#undef FACTOR3

ushort get_scaled_zoom(ushort zoom)
{
    short h;

    h = min(lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    if (h < 400)
        return zoom;
    else
        return  h * zoom / 240;
}

ushort get_unscaled_zoom(ushort szoom)
{
    short h;

    h = min(lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    if (h < 400)
        return szoom;
    else
        return  240 * szoom / h;
}

int get_zoom_from_range_bloks(ushort range_blocks)
{
    int zoom;

    zoom = zoom_levels[range_blocks];
    if (zoom > user_zoom_max)
        zoom = user_zoom_max;
    if (zoom < user_zoom_min)
        zoom = user_zoom_min;
    return zoom;
}

short get_overall_scale_min(void)
{
    return get_scaled_zoom(user_zoom_min);
}

short get_overall_scale_max(void)
{
    return get_scaled_zoom(user_zoom_max);
}

short bound_render_area(short rarea)
{
    if (rarea < RENDER_AREA_MIN)
        rarea = RENDER_AREA_MIN;
    if (rarea > RENDER_AREA_MAX)
        rarea = RENDER_AREA_MAX;
    // Odd size of the area would produce shifted terrain
    if (rarea & 1) rarea++;
    return rarea;
}

short get_render_area_for_zoom(short zoom)
{
    short rarea;
    int w, h, dt;
    // Original value was 30 for zoom=127 and screen proportion 4:3.
    // The algorithm below was found experimentally, by lowering
    // camera angle on London map and finding value at which view
    // corner is black when ground bevel edge is near that corner.
    dt = 14;
    // Increase delta by 1 for 16:9 (round to over 16:10)
    w = lbDisplay.GraphicsScreenWidth;
    h = lbDisplay.GraphicsScreenHeight;
    if (h > w) {
        h = lbDisplay.GraphicsScreenWidth;
        w = lbDisplay.GraphicsScreenHeight;
    }
    if (h != 0)
        dt += (w - h * 4 / 3) * 10 / (h * (16/4));
    // Compute the area
    rarea = ((106*14/4) - zoom) * 4 / dt;
    // In low resolution, use even lower render area
    if (h < 400)
        rarea  = rarea * 4 / 5;
    LOGSYNC("Render area %hd dt %d zoom %hd", rarea, dt, zoom);
    return bound_render_area(rarea);
}

/******************************************************************************/
