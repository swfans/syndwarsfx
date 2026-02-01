/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engintrns.c
 *     Math transformations required by the 3D engine.
 * @par Purpose:
 *     Implement functions for transforming positions in 3D world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engintrns.h"

#include "bigmap.h"
#include "engincam.h"
#include "engindrwlstm.h"
#include "enginzoom.h"
#include "game.h"
#include "game_options.h"
#include "swlog.h"
/******************************************************************************/
#define SCREEN_POINT_COORD_MIN (-MAX_SUPPORTED_SCREEN_WIDTH)
#define SCREEN_POINT_COORD_MAX (2 * MAX_SUPPORTED_SCREEN_WIDTH)

short angle_between_points(int x1, int z1, int x2, int z2)
{
  return LbArcTanAngle(x2 - x1, z1 - z2);
}

void local_to_worldr(int *dx, int *dy, int *dz)
{
#if 0
    asm volatile (
      "call ASM_local_to_worldr\n"
        : : "a" (dx), "d" (dy), "b" (dz));
#endif
    int x, z;

    z = *dz;
    x = *dx;
    *dx = (z * dword_176D10 + x * dword_176D14) >> 16;
    *dz = (z * dword_176D14 - x * dword_176D10) >> 16;
}

void transform_point(struct EnginePoint *p_ep)
{
#if 0
    asm volatile ("call ASM_transform_point\n"
        :  : "a" (p_ep));
    return;
#endif
    int fctr_a, fctr_b, fctr_c;
    int scr_shx, scr_shy;

    p_ep->Y3d -= 8 * engn_yc;
    fctr_a = (dword_176D14 * p_ep->X3d - dword_176D10 * p_ep->Z3d) >> 16;
    fctr_b = (dword_176D14 * p_ep->Z3d + dword_176D10 * p_ep->X3d) >> 16;
    fctr_c = (dword_176D1C * p_ep->Y3d - dword_176D18 * fctr_b) >> 16;

    p_ep->Z3d = (dword_176D1C * fctr_b + dword_176D18 * p_ep->Y3d) >> 16;
    p_ep->X3d = overall_scale * fctr_a;
    p_ep->Y3d = overall_scale * fctr_c;

    if ((game_perspective == 5) && (p_ep->Z3d > 0x4000 / 16))
        // With large depth (upper parts of the screen), the simplification of
        // using depth buffer for perspective would cause wrap-around effect.
        // To avoid that, switch to more complex algorithm before the values
        // diverge too much.
        p_ep->Z3d = 0x4000 * p_ep->Z3d / (p_ep->Z3d + 0x4000);

    scr_shx = p_ep->X3d >> 11;
    if (game_perspective == 5)
        scr_shx = scr_shx * (0x4000 - p_ep->Z3d) >> 14;

    p_ep->pp.X = dword_176D3C + scr_shx;
    if (p_ep->pp.X < 0)
    {
        if (p_ep->pp.X < SCREEN_POINT_COORD_MIN)
            p_ep->pp.X = SCREEN_POINT_COORD_MIN;
        p_ep->Flags |= 0x01;
    }
    else if (p_ep->pp.X >= vec_window_width)
    {
        if (p_ep->pp.X > SCREEN_POINT_COORD_MAX)
            p_ep->pp.X = SCREEN_POINT_COORD_MAX;
        p_ep->Flags |= 0x02;
    }

    scr_shy = p_ep->Y3d >> 11;
    if (game_perspective == 5)
        scr_shy = scr_shy * (0x4000 - p_ep->Z3d) >> 14;

    p_ep->pp.Y = dword_176D40 - scr_shy;
    if (p_ep->pp.Y < 0)
    {
        if (p_ep->pp.Y < SCREEN_POINT_COORD_MIN)
            p_ep->pp.Y = SCREEN_POINT_COORD_MIN;
        p_ep->Flags |= 0x04;
    }
    else if (p_ep->pp.Y >= vec_window_height)
    {
        if (p_ep->pp.Y > SCREEN_POINT_COORD_MAX)
            p_ep->pp.Y = SCREEN_POINT_COORD_MAX;
        p_ep->Flags |= 0x08;
    }
    p_ep->Flags |= 0x40;
}

void transform_shpoint(struct ShEnginePoint *p_sp, int dxc, int dyc, int dzc)
{
    int fctr_a, fctr_b, fctr_c, scr_d;
    int scr_shx, scr_shy, sca_x, sca_y;
    int scr_x, scr_y;
    ubyte flg;

    fctr_a = (dword_176D14 * dxc - dword_176D10 * dzc) >> 16;
    fctr_b = (dword_176D10 * dxc + dword_176D14 * dzc) >> 16;
    fctr_c = (dword_176D1C * dyc - dword_176D18 * fctr_b) >> 16;
    scr_d = (dword_176D18 * dyc + dword_176D1C * fctr_b) >> 16;
    sca_x = overall_scale * fctr_a;
    sca_y = overall_scale * fctr_c;
    flg = 0;

    if ((game_perspective == 5) && (scr_d > 0x4000 / 16))
        // Mitigate wrap-around effect by using non-simplified computations.
        scr_d = 0x4000 * scr_d / (scr_d + 0x4000);

    scr_shx = sca_x >> 11;
    if (game_perspective == 5)
        scr_shx = scr_shx * (0x4000 - scr_d) >> 14;

    scr_x = dword_176D3C + scr_shx;
    if (scr_x < 0)
    {
        if (scr_x < SCREEN_POINT_COORD_MIN)
            scr_x = SCREEN_POINT_COORD_MIN;
        flg |= 0x01;
    }
    else if (scr_x >= vec_window_width)
    {
        if (scr_x > SCREEN_POINT_COORD_MAX)
            scr_x = SCREEN_POINT_COORD_MAX;
        flg |= 0x02;
    }

    scr_shy = sca_y >> 11;
    if (game_perspective == 5)
        scr_shy = scr_shy * (0x4000 - scr_d) >> 14;

    scr_y = dword_176D40 - scr_shy;
    if (scr_y < 0)
    {
        if (scr_y < SCREEN_POINT_COORD_MIN)
            scr_y = SCREEN_POINT_COORD_MIN;
        flg |= 0x04;
    }
    else if (scr_y >= vec_window_height)
    {
        if (scr_y > SCREEN_POINT_COORD_MAX)
            scr_y = SCREEN_POINT_COORD_MAX;
        flg |= 0x08;
    }

    flg |= 0x40;

    p_sp->Flags = flg;
    p_sp->X = scr_x;
    p_sp->Y = scr_y;
    p_sp->Depth = scr_d;
}

void transform_shpoint_fpv(struct ShEnginePoint *p_sp, int dxc, int dyc, int dzc)
{
    int fctr_a, fctr_b, fctr_c, scr_d;
    int scr_shx, scr_shy, sca_x, sca_y;
    int scr_x, scr_y;
    ubyte flg;

    fctr_a = (dword_176D14 * dxc - dword_176D10 * dzc) >> 16;
    fctr_b = (dword_176D10 * dxc + dword_176D14 * dzc) >> 16;
    fctr_c = (dword_176D1C * dyc - dword_176D18 * fctr_b) >> 16;
    scr_d = (dword_176D18 * dyc + dword_176D1C * fctr_b) >> 16;
    sca_x = overall_scale * fctr_a;
    sca_y = overall_scale * fctr_c;
    flg = 0;

    if (scr_d >= -500)
    {
        int dvfactor;
        dvfactor = (scr_d >> 2) + 500;
        scr_shx = (1500 * sca_x / dvfactor) >> 9;
        scr_shy = -(1500 * sca_y / dvfactor) >> 9;
        flg |= 0x40;
        scr_d /= render_area_a / 20 + 1;
    }
    else
    {
        scr_shx = sca_x >> 8;
        scr_shy = sca_y >> 8;
        flg |= 0x20;
    }

    scr_x = dword_176D3C + scr_shx;
    if (scr_x < 0)
    {
        if (scr_x < SCREEN_POINT_COORD_MIN)
            scr_x = SCREEN_POINT_COORD_MIN;
        flg |= 0x01;
    }
    else if (scr_x >= vec_window_width)
    {
        if (scr_x > SCREEN_POINT_COORD_MAX)
            scr_x = SCREEN_POINT_COORD_MAX;
        flg |= 0x02;
    }

    scr_y = dword_176D40 + scr_shy;
    if (scr_y < 0)
    {
        if (scr_y < SCREEN_POINT_COORD_MIN)
            scr_y = SCREEN_POINT_COORD_MIN;
        flg |= 0x04;
    }
    else if (scr_y >= vec_window_height)
    {
        if (scr_y > SCREEN_POINT_COORD_MAX)
            scr_y = SCREEN_POINT_COORD_MAX;
        flg |= 0x08;
    }

    p_sp->Flags = flg;
    p_sp->X = scr_x;
    p_sp->Y = scr_y;
    p_sp->Depth = scr_d;
}

int transform_shpoint_y(int dxc, int dyc, int dzc)
{
    int fctr_b, fctr_c, scr_d;
    int scr_shy, sca_y;
    int scr_y;

    fctr_b = (dword_176D10 * dxc + dword_176D14 * dzc) >> 16;
    fctr_c = (dword_176D1C * dyc - dword_176D18 * fctr_b) >> 16;
    scr_d = (dword_176D18 * dyc + dword_176D1C * fctr_b) >> 16;
    sca_y = overall_scale * fctr_c;

    if ((game_perspective == 5) && (scr_d > 0x4000 / 16))
        // Mitigate wrap-around effect by using non-simplified computations.
        scr_d = 0x4000 * scr_d / (scr_d + 0x4000);

    scr_shy = sca_y >> 11;
    if (game_perspective == 5)
        scr_shy = scr_shy * (0x4000 - scr_d) >> 14;

    scr_y = dword_176D40 - scr_shy;
    if (scr_y < 0)
    {
        if (scr_y < SCREEN_POINT_COORD_MIN)
            scr_y = SCREEN_POINT_COORD_MIN;
    }
    else if (scr_y >= vec_window_height)
    {
        if (scr_y > SCREEN_POINT_COORD_MAX)
            scr_y = SCREEN_POINT_COORD_MAX;
    }

    return scr_y;
}

void process_engine_unk1(void)
{
    int angle;

    dword_176D4C = 0;
    dword_176D64 = -70;
    dword_176D3C = vec_window_width / 2;
    dword_176D40 = vec_window_height / 2;
    engn_anglexz += cam_rotation_velocity;
    dword_176D44 = 4 * (vec_window_width / 2) / 3;
    angle = (engn_anglexz >> 5) & LbFPMath_AngleMask;
    dword_176D0C = angle;
    dword_176D14 = lbSinTable[angle + LbFPMath_PI/2];
    dword_176D10 = lbSinTable[angle];
    angle = cam_tilt & LbFPMath_AngleMask;
    dword_176D18 = lbSinTable[angle];
    dword_176D1C = lbSinTable[angle + LbFPMath_PI/2];
}

void setup_engine_nullsub4(void)
{
}

void calc_mouse_pos(void)
{
    int cor_dx, cor_dy, cor_dz;
    int fctr_xz;
    int chk_x, chk_y, chk_z;
    short mag;
    short i;

    cor_dy = (dword_176D18 >> 8);
    fctr_xz = (dword_176D1C >> 8);
    cor_dx = (fctr_xz * dword_176D10) >> 16;
    cor_dz = (fctr_xz * dword_176D14) >> 16;

    chk_x = 200 * cor_dx + 16 * mouse_map_x;
    chk_y = 200 * cor_dy;
    chk_z = 200 * cor_dz + 16 * mouse_map_z;

    mag = 0;
    for (i = 0; i < 400; i++)
    {
        if ( chk_y >> 4 < PRCCOORD_TO_YCOORD(alt_at_point(chk_x >> 4, chk_z >> 4)))
            mag = i;
        chk_x -= cor_dx;
        chk_y -= cor_dy;
        chk_z -= cor_dz;
    }

    if (mag != 0)
    {
        mag -= 200;
        mouse_map_x -= (mag * cor_dx) >> 4;
        mouse_map_z -= (mag * cor_dz) >> 4;
        mouse_map_y = alt_at_point(mouse_map_x, mouse_map_z) >> 8;
    }
}

void transform_screen_to_map_isometric(int *dxc, int *dzc, int scr_x, int scr_y)
{
    int fctr_a, fctr_b_part;
    int fctr_c;
    int scr_shx, scr_shy;

    if (dword_176D18 == 0) {
        *dxc = *dzc = 0;
        return;
    }

    scr_shx =  scr_x - dword_176D3C;
    scr_shy = -scr_y + dword_176D40;
    fctr_a = (scr_shx << 11) / overall_scale;
    fctr_c = (scr_shy << 11) / overall_scale;
    fctr_b_part = ((fctr_c << 16)) / dword_176D18;

    *dxc =  ((dword_176D14 * fctr_a - dword_176D10 * fctr_b_part) >> 16);
    *dzc = -((dword_176D10 * fctr_a + dword_176D14 * fctr_b_part) >> 16);
}

void process_engine_unk2(void)
{
    short msx, msy;
    int offs_y;
    int scr_x, scr_y;
    int map_dxc, map_dzc;

    if (ingame.DisplayMode == DpM_ENGINEPLY)
      offs_y = overall_scale * engn_yc >> 8;
    else
      offs_y = 0;
    msx = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
    msy = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;

    if (lbDisplay.GraphicsScreenHeight < 400)
    {
        scr_y = (msy >> 1) - offs_y;
        scr_x = msx >> 1;
    }
    else
    {
        scr_y = msy - offs_y;
        scr_x = msx;
    }

    transform_screen_to_map_isometric(&map_dxc, &map_dzc, scr_x, scr_y);

    mouse_map_x = engn_xc + map_dxc;
    mouse_map_z = engn_zc + map_dzc;
    if (ingame.DisplayMode == DpM_ENGINEPLY)
        calc_mouse_pos();
    setup_engine_nullsub4();
}

/******************************************************************************/
