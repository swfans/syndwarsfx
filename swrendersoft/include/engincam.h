/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engincam.h
 *     Header file for engincam.c.
 * @par Purpose:
 *     Camera handling for the 3D engine.
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
#ifndef ENGINCAM_H
#define ENGINCAM_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#pragma pack(1)

#pragma pack()
/******************************************************************************/
extern s32 engn_xc;
extern s32 engn_yc;
extern s32 engn_zc;
extern s32 engn_anglexz;

extern s32 engn_x_vel;
extern s32 engn_y_vel;
/******************************************************************************/

void camera_setup_view(int *p_pos_beg_x, int *p_pos_beg_z,
  int *p_rend_beg_x, int *p_rend_beg_z, int *p_tlcount_x, int *p_tlcount_z);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
