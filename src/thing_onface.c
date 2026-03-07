/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_onface.c
 *     Support for checking if thing is on 3D face and putting it on a face.
 * @par Purpose:
 *     Check thing position to determine whether it is placed on a 3D face.
 *     Adjust thing properties to put it on the 3D face (triangle or quad).
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 17 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_onface.h"

#include <assert.h>
#include "bfendian.h"
#include "bfscreen.h"
#include "poly.h"

#include "bigmap.h"
#include "enginpriobjs.h"
#include "enginsngobjs.h"
#include "game.h"
#include "packet.h"
#include "player.h"
#include "swlog.h"
#include "thing.h"

/******************************************************************************/

extern struct Thing thing_on_face;

/******************************************************************************/

/**
 * Multiplication with shift and special quirks.
 *
 * Fills lower bits with sign, but that's not all - results are quite unique.
 * Decompiler generates the following pseudo-C for it:
 *    HIWORD(tmp) = (ar1 * ar2) >> 16;
 *    LOWORD(tmp) = (ar1 * ar2) >> 32;
 *    ret = bw_rotl32(tmp, 16);
 * Needs testing whether something similar can really represent this ic C.
 */
s32 mul_shift16_sign_pad_lo(s32 ar1, s32 ar2)
{
#if 0
    s32 tmp;
    tmp = (ar1 * ar2) & 0xFFFF0000;
    tmp |= ((ar1 * (s64)ar2) >> 32) & 0xFFFF;
    return bw_rotl32(tmp, 16);
#else
    s32 ret;
    asm volatile (
      "imul   %%edx\n"
      "mov    %%dx,%%ax\n"
      "rol    $0x10,%%eax\n"
        : "=r" (ret) : "a" (ar1), "d" (ar2));
    return ret;
#endif
}

ubyte check_big_point_triangle(int x, int y, int ux, int uy, int vx, int vy, int wx, int wy)
{
    ubyte ret;
    asm volatile (
      "push %8\n"
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_check_big_point_triangle\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (ux), "c" (uy), "g" (vx), "g" (vy), "g" (wx), "g" (wy));
    return ret;
}

ushort set_thing_height_on_face_tri(struct Thing *p_thing, int x, int z, short face)
{
    ushort ret;
    asm volatile (
      "call ASM_set_thing_height_on_face_tri\n"
        : "=r" (ret) : "a" (p_thing), "d" (x), "b" (z), "c" (face));
    return ret;
}

ushort set_thing_height_on_face_quad(struct Thing *p_thing, int x, int z, short face)
{
    ushort ret;
    asm volatile (
      "call ASM_set_thing_height_on_face_quad\n"
        : "=r" (ret) : "a" (p_thing), "d" (x), "b" (z), "c" (face));
    return ret;
}

ushort set_thing_height_on_face(struct Thing *p_thing, int x, int z, short face)
{
    if (face > 0) {
        return set_thing_height_on_face_tri(p_thing, x, z, face);
    } else if (face < 0) {
        return set_thing_height_on_face_quad(p_thing, x, z, -face);
    }
    return 0;
}

short find_and_set_connected_face(struct Thing *p_thing, int x, int z, short face)
{
    short ret;
    asm volatile (
      "call ASM_find_and_set_connected_face\n"
        : "=r" (ret) : "a" (p_thing), "d" (x), "b" (z), "c" (face));
    return ret;
}

void check_mouse_over_face(struct PolyPoint *p_pt1, struct PolyPoint *p_pt2,
  struct PolyPoint *p_pt3, int face, int type)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_check_mouse_over_face\n"
        : : "a" (p_pt1), "d" (p_pt2), "b" (p_pt3), "c" (face), "g" (type));
    return;
#endif
    struct SingleObjectFace3 *p_face3;
    struct SingleObjectFace4 *p_face4;
    struct SingleObject *p_sobj;
    struct SinglePoint *p_point1;
    struct SinglePoint *p_point2;
    struct SinglePoint *p_point3;
    int scr_dist_x1, scr_dist_y1;
    int scr_dist_x2, scr_dist_y2;
    int scr_dist_x3, scr_dist_y3;
    int map_dist_x1, map_dist_z1;
    int map_dist_x2, map_dist_z2;
    int m1, m2, m3, m4;
    int ms_x, ms_y;
    int factorA, factorB;

    if (pktrec_mode == 2) {
        return;
    }

    ms_x = lbDisplay.MMouseX;
    ms_y = lbDisplay.MMouseY;

    if (!check_big_point_triangle(ms_x, ms_y, p_pt1->X, p_pt1->Y,
        p_pt2->X, p_pt2->Y, p_pt3->X, p_pt3->Y)) {
        return;
    }

    switch (type)
    {
    case 1:
        players[local_player_no].Target = face;
        p_face3 = &game_object_faces3[face];
        p_sobj = &game_objects[p_face3->Object];
        p_point3 = &game_object_points[p_face3->PointNo[0]];
        p_point1 = &game_object_points[p_face3->PointNo[2]];
        p_point2 = &game_object_points[p_face3->PointNo[1]];
        break;
    case 2:
        players[local_player_no].Target = -face;
        p_face4 = &game_object_faces4[face];
        p_sobj = &game_objects[p_face4->Object];
        p_point3 = &game_object_points[p_face4->PointNo[0]];
        p_point1 = &game_object_points[p_face4->PointNo[2]];
        p_point2 = &game_object_points[p_face4->PointNo[1]];
        break;
    case 3:
        players[local_player_no].Target = -face;
        p_face4 = &game_object_faces4[face];
        p_sobj = &game_objects[p_face4->Object];
        p_point3 = &game_object_points[p_face4->PointNo[3]];
        p_point1 = &game_object_points[p_face4->PointNo[1]];
        p_point2 = &game_object_points[p_face4->PointNo[2]];
        break;
    default:
        LOGERR("Invalid type=%d", (int)type);
        return;
    }

    players[local_player_no].TargetType = 3;

    scr_dist_x1 = (p_pt2->X - p_pt1->X) << 16;
    scr_dist_y1 = (p_pt2->Y - p_pt1->Y) << 16;

    scr_dist_x2 = (p_pt3->X - p_pt1->X) << 16;
    scr_dist_y2 = (p_pt3->Y - p_pt1->Y) << 16;

    scr_dist_x3 = (ms_x - p_pt1->X) << 16;
    scr_dist_y3 = (ms_y - p_pt1->Y) << 16;

    map_dist_x1 = (p_point1->X - p_point3->X) << 16;
    map_dist_z1 = (p_point1->Z - p_point3->Z) << 16;

    map_dist_x2 = (p_point2->X - p_point3->X) << 16;
    map_dist_z2 = (p_point2->Z - p_point3->Z) << 16;

    m1 = mul_shift16_sign_pad_lo(scr_dist_y2, scr_dist_x3);
    m2 = mul_shift16_sign_pad_lo(scr_dist_x2, scr_dist_y3);
    m3 = mul_shift16_sign_pad_lo(scr_dist_x1, scr_dist_y2);
    m4 = mul_shift16_sign_pad_lo(scr_dist_y1, scr_dist_x2);
    factorA = ((m1 - (s64)m2) << 16) / (m3 - m4);

    m1 = mul_shift16_sign_pad_lo(scr_dist_x1, scr_dist_y3);
    m2 = mul_shift16_sign_pad_lo(scr_dist_y1, scr_dist_x3);
    m3 = mul_shift16_sign_pad_lo(scr_dist_x1, scr_dist_y2);
    m4 = mul_shift16_sign_pad_lo(scr_dist_y1, scr_dist_x2);
    factorB = ((m1 - (s64)m2) << 16) / (m3 - m4);

    if ((factorA <= 0 || factorA > 0x10000)) {
        return;
    }
    if ((factorB <= 0 || factorB > 0x10000)) {
        return;
    }

    if ((factorA + factorB) <= 0x20000)
    {
        int prc_x, prc_z;
        int ms_cor_x, ms_cor_z;

        ms_cor_x = p_sobj->MapX + p_point3->X;
        ms_cor_x += mul_shift16_sign_pad_lo(map_dist_x1, factorA) >> 16;
        ms_cor_x += mul_shift16_sign_pad_lo(map_dist_x2, factorB) >> 16;

        ms_cor_z = p_sobj->MapZ + p_point3->Z;
        ms_cor_z += mul_shift16_sign_pad_lo(map_dist_z1, factorA) >> 16;
        ms_cor_z += mul_shift16_sign_pad_lo(map_dist_z2, factorB) >> 16;

        prc_x = MAPCOORD_TO_PRCCOORD(ms_cor_x, 0);
        prc_z = MAPCOORD_TO_PRCCOORD(ms_cor_z, 0);
        switch (type)
        {
        case 1:
            set_thing_height_on_face_tri(&thing_on_face, prc_x, prc_z, face);
            break;
        case 2:
        case 3:
            set_thing_height_on_face_quad(&thing_on_face, prc_x, prc_z, face);
            break;
        }
        mouse_map_x = ms_cor_x;
        mouse_map_z = ms_cor_z;
    }
}

/******************************************************************************/
