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

enum ProjectionMode {
  /** Standard isometric projection mode */
  ProjM_Isometric = 0,
  /** Isometric projection with no height change on the floor. */
  ProjM_IsomFloorFlat,
  /** Isometric projection with no buildings drawn. */
  ProjM_IsomNoBuildng,
  /** Isometric projection with parts of objects drawn as wireframe, for debug purposes. */
  ProjM_IsomObjWirefr,
  /** Isometric projection with custom debug alterations, for debug purposes. */
  ProjM_IsomDbgCust4,
  /** Perspective mode, lens-distorted isometric projection. */
  ProjM_Perspective,
  /** Isometric projection with stars background instead of the floor. */
  ProjM_IsomFloorStars,
  /** Isometric projection with simplified light casting. */
  ProjM_IsomSimpLight,
};

struct CameraState {
    s32 xc;
    s32 yc;
    s32 zc;
    s32 yaw;
    s32 tilt;
    s32 x_vel;
    s32 z_vel;
    s32 yaw_vel;
    ushort overall_scale;
    ubyte perspective;
    ushort area_a;
    ushort area_b;
};

#pragma pack()
/******************************************************************************/

extern s32 engn_xc;
extern s32 engn_yc;
extern s32 engn_zc;

/** Camera yaw/pan axis (XZ axis) rotation angle.
 *
 * Controls left-to-right rotation. In bflib angular units x 32.
 */
extern s32 engn_cam_yaw;

/** Camera tilt/pitch axis rotation angle.
 *
 * Controls up-and-down rotation. In bflib angular units, no multiplier.
 */
extern s32 engn_cam_tilt;

extern s32 engn_x_vel;
extern s32 engn_z_vel;

/** Velocity of camera yaw angle change.
 *
 * The value is added to engn_cam_yaw once per turn.
 */
extern s32 engn_cam_yaw_vel;

extern ushort overall_scale;
extern ubyte game_perspective;
/******************************************************************************/

void camera_setup_view(int *p_pos_beg_x, int *p_pos_beg_z,
  int *p_rend_beg_x, int *p_rend_beg_z, int *p_tlcount_x, int *p_tlcount_z);

/** Partial camera view setup, which only prepare fractional angles.
 */
void camera_setup_angle_fractions(void);

void camera_save_backup_state(struct CameraState *p_bkp);
void camera_load_backup_state(const struct CameraState *p_bkp);

void camera_apply_velocity(void);

/** Checks if given coordinates are within the map area which will be rendered.
 *
 * Being rendered does not neccesarily mean that the point will be visible on
 * screen. But related area will be added to drawlist.
 */
TbBool coords_within_render_area(int cor_x, int cor_z);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
