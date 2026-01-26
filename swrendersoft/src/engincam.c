/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engincam.c
 *     cam level handling for the 3D engine.
 * @par Purpose:
 *     Implements functions for storing and altering camera view.
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
#include "engincam.h"

#include "bfmath.h"

#include "enginzoom.h"
#include "privrdlog.h"
/******************************************************************************/

extern long dword_176D70;
extern long dword_176D74;
extern long dword_176D78;
extern long dword_176D7C;
extern long dword_176D80;
extern long dword_176D84;
extern long dword_176D88;
extern long dword_176D8C;

void camera_setup_view(int *p_pos_beg_x, int *p_pos_beg_z,
  int *p_rend_beg_x, int *p_rend_beg_z, int *p_tlcount_x, int *p_tlcount_z)
{
    int angXZ;
    int rend_beg_x, rend_beg_z, tlreach_x, tlreach_z;
    int pos_beg_x, pos_beg_z;
    int tlcount_x, tlcount_z;

    angXZ = (engn_anglexz >> 5) & 0x7FF;

    byte_176D48 = ((angXZ + 256) >> 9) & 0x3;
    byte_176D49 = ((angXZ + 128) >> 8) & 0x7;
    byte_176D4A = ((angXZ + 85) / 170) % 12;
    byte_176D4B = ((angXZ + 64) >> 7) & 0xF;
    byte_19EC7A = ((angXZ + 256) >> 9) & 0x3;

    rend_beg_x = (engn_xc & 0xFF00) + (render_area_a << 7);
    rend_beg_z = (engn_zc & 0xFF00) - (render_area_b << 7);
    tlreach_x = ((-lbSinTable[angXZ]) >> 12) + ((-lbSinTable[angXZ]) >> 13);
    if (tlreach_x <= 0) {
        tlcount_x = render_area_a - tlreach_x;
        pos_beg_x = rend_beg_x;
    } else {
        tlcount_x = render_area_a + tlreach_x;
        pos_beg_x = rend_beg_x + (tlreach_x << 8);
    }
    tlreach_z = (lbSinTable[angXZ + LbFPMath_PI/2] >> 12) + (lbSinTable[angXZ + LbFPMath_PI/2] >> 13);
    if (tlreach_z <= 0) {
        tlcount_z = render_area_b - tlreach_z;
        pos_beg_z = rend_beg_z;
    } else {
        tlcount_z = render_area_b + tlreach_z;
        pos_beg_z = rend_beg_z - (tlreach_z << 8);
    }
    dword_176D80 = (rend_beg_x >> 8) + 50;
    dword_176D84 = (rend_beg_z >> 8) + 50;
    dword_176D70 = (pos_beg_x >> 8) + 50;
    dword_176D74 = (pos_beg_z >> 8) + 50;
    dword_176D88 = dword_176D80 - render_area_a;
    dword_176D8C = dword_176D84 + render_area_b;
    dword_176D78 = dword_176D70 - tlcount_x;
    dword_176D7C = dword_176D74 + tlcount_z;

    *p_pos_beg_x = pos_beg_x;
    *p_pos_beg_z = pos_beg_z;
    *p_rend_beg_x = rend_beg_x;
    *p_rend_beg_z = rend_beg_z;
    *p_tlcount_x = tlcount_x;
    *p_tlcount_z = tlcount_z;
}

/******************************************************************************/
