/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file drawshape.h
 *     Header file for drawshape.c.
 * @par Purpose:
 *     Drawing simple 2D shapes within the 3D world.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 20 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DRAWSHAPE_H
#define DRAWSHAPE_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/

void draw_line_transformed_col(int x1, int y1, int z1, int x2, int y2, int z2,
  TbPixel colour);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
