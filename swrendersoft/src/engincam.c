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
ushort overall_scale = 256;
ubyte game_perspective = ProjM_Isometric;

s32 engn_xc = 0x3280;
s32 engn_yc = 0;
s32 engn_zc = 0x3280;

s32 engn_cam_tilt = -(303 * LbFPMath_PI / 1800); // -30.3 degree in bflib units

s32 engn_cam_yaw = 0;

s32 engn_x_vel = 0;
s32 engn_z_vel = 0;
s32 engn_cam_yaw_vel = 0;

s32 dword_176D70;
s32 dword_176D74;
s32 dword_176D78;
s32 dword_176D7C;
s32 dword_176D80;
s32 dword_176D84;
s32 dword_176D88;
s32 dword_176D8C;

ubyte byte_176D48;
ubyte byte_176D49;
ubyte byte_176D4A;
ubyte byte_176D4B;
/******************************************************************************/

void camera_setup_angle_fractions(void)
{
    int angXZ;

    angXZ = (engn_cam_yaw >> 5) & LbFPMath_AngleMask;

    byte_176D48 = ((angXZ + 256) >> 9) & 0x3;
    byte_176D49 = ((angXZ + 128) >> 8) & 0x7;
    byte_176D4B = ((angXZ + 64) >> 7) & 0xF;
    byte_176D4A = ((angXZ + 85) / 170) % 12;
}

void camera_setup_view(int *p_pos_beg_x, int *p_pos_beg_z,
  int *p_rend_beg_x, int *p_rend_beg_z, int *p_tlcount_x, int *p_tlcount_z)
{
    int angXZ;
    int rend_beg_x, rend_beg_z, tlreach_x, tlreach_z;
    int pos_beg_x, pos_beg_z;
    int tlcount_x, tlcount_z;

    camera_setup_angle_fractions();

    angXZ = (engn_cam_yaw >> 5) & LbFPMath_AngleMask;

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

TbBool coords_within_render_area(int cor_x, int cor_z)
{
    int rend_beg_x, rend_beg_z;
    int rend_end_x, rend_end_z;

    rend_beg_x = (engn_xc & 0xFF00) + (render_area_a << 7);
    rend_beg_z = (engn_zc & 0xFF00) - (render_area_b << 7);

    if ((cor_x > rend_beg_x) || (cor_z < rend_beg_z))
        return false;

    rend_end_x = rend_beg_x - (render_area_a << 8);
    rend_end_z = rend_beg_z + (render_area_b << 8);

    if ((cor_x < rend_end_x) || (cor_z > rend_end_z))
        return false;

    return true;
}

void camera_save_backup_state(struct CameraState *p_bkp)
{
    p_bkp->xc = engn_xc;
    p_bkp->yc = engn_yc;
    p_bkp->zc = engn_zc;
    p_bkp->yaw = engn_cam_yaw;
    p_bkp->tilt = engn_cam_tilt;
    p_bkp->x_vel = engn_x_vel;
    p_bkp->z_vel = engn_z_vel;
    p_bkp->yaw_vel = engn_cam_yaw_vel;
    p_bkp->overall_scale = overall_scale;
    p_bkp->perspective = game_perspective;
    p_bkp->area_a = render_area_a;
    p_bkp->area_b = render_area_b;
}

void camera_load_backup_state(const struct CameraState *p_bkp)
{
    engn_xc = p_bkp->xc;
    engn_yc = p_bkp->yc;
    engn_zc = p_bkp->zc;
    engn_cam_yaw = p_bkp->yaw;
    engn_cam_tilt = p_bkp->tilt;
    engn_x_vel = p_bkp->x_vel;
    engn_z_vel = p_bkp->z_vel;
    engn_cam_yaw_vel = p_bkp->yaw_vel;
    overall_scale = p_bkp->overall_scale;
    game_perspective = p_bkp->perspective;
    render_area_a = p_bkp->area_a;
    render_area_b = p_bkp->area_b;
}

void camera_apply_velocity(void)
{
    engn_cam_yaw += engn_cam_yaw_vel;
}

/******************************************************************************/
