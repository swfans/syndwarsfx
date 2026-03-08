/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginsngobjs.c
 *     Engine single objects required by the 3D engine.
 * @par Purpose:
 *     Implement functions for handling single objects in 3D world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Sep 2023 - 02 Jan 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginsngobjs.h"

#include "bfmemut.h"
#include "bfmath.h"
#include <limits.h>
#include <stdlib.h>

#include "engintrns.h"
#include "enginprops.h"
#include "privrdlog.h"
/******************************************************************************/
struct SinglePoint *game_object_points = NULL;
ushort next_object_point = 1;

struct SingleObjectFace3 *game_object_faces3 = NULL;
ushort next_object_face3 = 1;

struct SingleObjectFace4 *game_object_faces4 = NULL;
ushort next_object_face4 = 1;

struct Normal *game_normals = NULL;
ushort next_normal = 1;

struct SingleObject *game_objects = NULL;
ushort next_object = 1;
/******************************************************************************/

void refresh_old_object_face_format(struct SingleObjectFace3 *p_objface,
  struct SingleObjectFace3OldV7 *p_oldobjface, u32 fmtver)
{
    int i;

    LbMemorySet(p_objface, 0, sizeof(struct SingleObjectFace3));

    for (i = 0; i < 3; i++)
        p_objface->PointNo[i] = p_oldobjface->PointNo[i];
    p_objface->Texture = p_oldobjface->Texture;
    p_objface->GFlags = p_oldobjface->GFlags;
    p_objface->Flags = p_oldobjface->Flags;
    p_objface->ExCol = p_oldobjface->ExCol;
    p_objface->Object = p_oldobjface->Object;
    p_objface->Shade0 = p_oldobjface->Shade0;
    p_objface->Shade1 = p_oldobjface->Shade1;
    p_objface->Shade2 = p_oldobjface->Shade2;
    p_objface->Light0 = p_oldobjface->Light0;
    p_objface->Light1 = p_oldobjface->Light1;
    p_objface->Light2 = p_oldobjface->Light2;
    p_objface->FaceNormal = p_oldobjface->FaceNormal;
    p_objface->WalkHeader = p_oldobjface->WalkHeader;
    // Unsure
    p_objface->UnknTringl = p_oldobjface->UnkOFField_2A;
    // Remove the effect of faces using scratch_buf1 (screen in pre-alpha) as texture
    p_objface->GFlags &= ~FGFlg_Unkn02;
}

void refresh_old_object_face4_format(struct SingleObjectFace4 *p_objface4,
  struct SingleObjectFace4OldV7 *p_oldobjface4, u32 fmtver)
{
    int i;

    LbMemorySet(p_objface4, 0, sizeof(struct SingleObjectFace4));

    for (i = 0; i < 4; i++)
        p_objface4->PointNo[i] = p_oldobjface4->PointNo[i];
    p_objface4->Texture = p_oldobjface4->Texture;
    p_objface4->GFlags = p_oldobjface4->GFlags;
    p_objface4->Flags = p_oldobjface4->Flags;
    p_objface4->ExCol = p_oldobjface4->ExCol;
    p_objface4->Object = p_oldobjface4->Object;
    p_objface4->Shade0 = p_oldobjface4->Shade0;
    p_objface4->Shade1 = p_oldobjface4->Shade1;
    p_objface4->Shade2 = p_oldobjface4->Shade2;
    p_objface4->Shade3 = p_oldobjface4->Shade3;
    p_objface4->Light0 = p_oldobjface4->Light0;
    p_objface4->Light1 = p_oldobjface4->Light1;
    p_objface4->Light2 = p_oldobjface4->Light2;
    p_objface4->Light3 = p_oldobjface4->Light3;
    p_objface4->FaceNormal = p_oldobjface4->FaceNormal;
    p_objface4->WalkHeader = p_oldobjface4->WalkHeader;
    // Unsure
    p_objface4->UnknTringl1 = p_oldobjface4->UnkOFField_36;
    p_objface4->UnknTringl2 = p_oldobjface4->UnkOFField_38;
}

/** Finds points within a face with the most sharp slope.
 *
 * @param face The face which corner points will be checked.
 * @param change_xz Set to distance in XZ plane between the same points as for altitude.
 * @return Returns altitude (Y coord) change which offers sharpest slope.
 */
int alt_change_at_face(short face, int *change_xz)
{
    uint best_dy, best_dxz;

    best_dy = 0;
    best_dxz = INT_MAX;

    if (face < 0)
    {
        struct SingleObjectFace4 *p_face;
        short cor1, cor2;

        p_face = &game_object_faces4[-face];

        for (cor1 = 0; cor1 < 4; cor1++)
        {
            struct SinglePoint *p_snpoint1;

            p_snpoint1 = &game_object_points[p_face->PointNo[cor1]];

            for (cor2 = cor1 + 1; cor2 < 4; cor2++)
            {
                struct SinglePoint *p_snpoint2;
                int dtx, dtz;
                uint dy, dxz;

                p_snpoint2 = &game_object_points[p_face->PointNo[cor2]];
                dtx = p_snpoint1->X - p_snpoint2->X;
                dtz = p_snpoint1->Z - p_snpoint2->Z;
                dy = abs(p_snpoint1->Y - p_snpoint2->Y);
                dxz = LbSqrL(dtx * dtx + dtz * dtz);
                // Multiply dy to make the expected range of values larger
                if (dy * 256 / (dxz+1) > best_dy * 256 / (best_dxz+1)) {
                    best_dy = dy;
                    best_dxz = dxz;
                } else if ((dy >= best_dy) && (dxz < best_dxz)) {
                    // Even if no angle change expected, we may still want to optimize values
                    best_dy = dy;
                    best_dxz = dxz;
                }
            }
        }
    }
    else if (face > 0)
    {
        struct SingleObjectFace3 *p_face;
        short cor1, cor2;

        p_face = &game_object_faces3[face];

        for (cor1 = 0; cor1 < 3; cor1++)
        {
            struct SinglePoint *p_snpoint1;

            p_snpoint1 = &game_object_points[p_face->PointNo[cor1]];

            for (cor2 = cor1 + 1; cor2 < 3; cor2++)
            {
                struct SinglePoint *p_snpoint2;
                int dtx, dtz;
                uint dy, dxz;

                p_snpoint2 = &game_object_points[p_face->PointNo[cor2]];
                dtx = p_snpoint1->X - p_snpoint2->X;
                dtz = p_snpoint1->Z - p_snpoint2->Z;
                dy = abs(p_snpoint1->Y - p_snpoint2->Y);
                dxz = LbSqrL(dtx * dtx + dtz * dtz);

                if (dy * 256 / (dxz+1) > best_dy * 256 / (best_dxz+1)) {
                    best_dy = dy;
                    best_dxz = dxz;
                } else if ((dy >= best_dy) && (dxz < best_dxz)) {
                    best_dy = dy;
                    best_dxz = dxz;
                }
            }
        }
    }

    if (change_xz != NULL)
        *change_xz = best_dxz;
    return best_dy;
}

TbBool get_mapcoord_on_face_points(int *cor_x, int *cor_z,
  struct PolyPoint *p_scrpt0, struct PolyPoint *p_scrpt1, struct PolyPoint *p_scrpt2,
  struct SinglePoint *p_mappt0, struct SinglePoint *p_mappt1, struct SinglePoint *p_mappt2,
  int scr_x, int scr_y)
{
    int scr_dist10_x, scr_dist10_y;
    int scr_dist20_x, scr_dist20_y;
    int scr_distc0_x, scr_distc0_y;
    int map_dist02_x, map_dist02_z;
    int map_dist12_x, map_dist12_z;
    int m1, m2, m3, m4;
    int factorA, factorB;

    scr_dist10_x = (p_scrpt1->X - p_scrpt0->X) << 16;
    scr_dist10_y = (p_scrpt1->Y - p_scrpt0->Y) << 16;

    scr_dist20_x = (p_scrpt2->X - p_scrpt0->X) << 16;
    scr_dist20_y = (p_scrpt2->Y - p_scrpt0->Y) << 16;

    scr_distc0_x = (scr_x - p_scrpt0->X) << 16;
    scr_distc0_y = (scr_y - p_scrpt0->Y) << 16;

    map_dist02_x = (p_mappt0->X - p_mappt2->X) << 16;
    map_dist02_z = (p_mappt0->Z - p_mappt2->Z) << 16;

    map_dist12_x = (p_mappt1->X - p_mappt2->X) << 16;
    map_dist12_z = (p_mappt1->Z - p_mappt2->Z) << 16;

    m3 = mul_shift16_sign_pad_lo(scr_dist10_x, scr_dist20_y);
    m4 = mul_shift16_sign_pad_lo(scr_dist10_y, scr_dist20_x);

    m1 = mul_shift16_sign_pad_lo(scr_dist20_y, scr_distc0_x);
    m2 = mul_shift16_sign_pad_lo(scr_dist20_x, scr_distc0_y);
    factorA = ((m1 - (s64)m2) << 16) / (m3 - m4);

    m1 = mul_shift16_sign_pad_lo(scr_dist10_x, scr_distc0_y);
    m2 = mul_shift16_sign_pad_lo(scr_dist10_y, scr_distc0_x);
    factorB = ((m1 - (s64)m2) << 16) / (m3 - m4);

    if ((factorA <= 0 || factorA > 0x10000)) {
        return false;
    }
    if ((factorB <= 0 || factorB > 0x10000)) {
        return false;
    }

    if ((factorA + factorB) > 0x20000) {
        return false;
    }

    *cor_x += p_mappt2->X;
    *cor_x += mul_shift16_sign_pad_lo(map_dist02_x, factorA) >> 16;
    *cor_x += mul_shift16_sign_pad_lo(map_dist12_x, factorB) >> 16;

    *cor_z += p_mappt2->Z;
    *cor_z += mul_shift16_sign_pad_lo(map_dist02_z, factorA) >> 16;
    *cor_z += mul_shift16_sign_pad_lo(map_dist12_z, factorB) >> 16;

    return true;
}

int get_height_on_face(int x, int z, ushort face)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_get_height_on_face\n"
        : "=r" (ret) : "a" (x), "d" (z), "b" (face));
    return ret;
#endif
    struct SingleObjectFace3 *p_oface;
    int pt0_x, pt0_y, pt0_z;
    int pt1_x, pt1_y, pt1_z;
    int pt2_x, pt2_y, pt2_z;
    int cor_x, cor_z;
    int dt_x, dt_z;
    int dt_a, dt_b;
    int len_a, len_b;

    p_oface = &game_object_faces3[face];
    {
        struct SingleObject *p_sobj;
        struct SinglePoint *p_opt0;
        struct SinglePoint *p_opt1;
        struct SinglePoint *p_opt2;
        int base_x, base_y, base_z;

        p_sobj = &game_objects[p_oface->Object];
        base_x = p_sobj->MapX;
        base_z = p_sobj->MapZ;
        base_y = p_sobj->OffsetY;

        p_opt0 = &game_object_points[p_oface->PointNo[0]];
        pt0_x = base_x + p_opt0->X;
        pt0_y = base_y + p_opt0->Y;
        pt0_z = base_z + p_opt0->Z;

        p_opt1 = &game_object_points[p_oface->PointNo[1]];
        pt1_x = base_x + p_opt1->X;
        pt1_y = base_y + p_opt1->Y;
        pt1_z = base_z + p_opt1->Z;

        p_opt2 = &game_object_points[p_oface->PointNo[2]];
        pt2_x = base_x + p_opt2->X;
        pt2_y = base_y + p_opt2->Y;
        pt2_z = base_z + p_opt2->Z;
    }
    cor_x = x >> 8;
    cor_z = z >> 8;
    dt_x = pt2_x - pt0_x;
    dt_z = pt2_z - pt0_z;

    len_a = dt_x * (pt1_z - pt0_z) - dt_z * (pt1_x - pt0_x);
    if (len_a == 0) {
        return len_a;
    }

    len_b = dt_z * (pt0_x - cor_x) + dt_x * (cor_z - pt0_z);
    dt_a = (len_b << 9) / len_a;
    if (dt_a < 0) {
        return 0;
    }

    if (dt_x)
        dt_b = ((cor_x << 9) - (pt0_x << 9) - dt_a * (pt1_x - pt0_x)) / dt_x;
    else
        dt_b = ((cor_z << 9) - (pt0_z << 9) - dt_a * (pt1_z - pt0_z)) / dt_z;
    if (dt_b < 0)
        return 0;
    if (dt_a + dt_b >= 512)
        return 0;

    return 32 * (pt0_y + (((pt2_y - pt0_y) * dt_b) >> 9) + ((dt_a * (pt1_y - pt0_y)) >> 9));
}

int get_height_on_face_quad(int x, int z, ushort face)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_get_height_on_face_quad\n"
        : "=r" (ret) : "a" (x), "d" (z), "b" (face));
    return ret;
#endif
    struct SingleObjectFace4 *p_face;
    struct SingleObject *p_sobj;
    struct SinglePoint *p_pt0;
    struct SinglePoint *p_pt1;
    struct SinglePoint *p_pt2;
    int prc_dist10_x, prc_dist10_y, prc_dist10_z;
    int prc_dist20_x, prc_dist20_y, prc_dist20_z;
    int prc_distc0_x, prc_distc0_z;

    p_face = &game_object_faces4[face];
    p_sobj = &game_objects[p_face->Object];
    p_pt0 = &game_object_points[p_face->PointNo[0]];
    p_pt1 = &game_object_points[p_face->PointNo[1]];
    p_pt2 = &game_object_points[p_face->PointNo[2]];

    prc_dist10_x = (p_pt1->X - p_pt0->X) << 8;
    prc_dist10_y = (p_pt1->Y - p_pt0->Y) << 8;
    prc_dist10_z = (p_pt1->Z - p_pt0->Z) << 8;
    prc_dist20_y = (p_pt2->Y - p_pt0->Y) << 8;
    prc_dist20_z = (p_pt2->Z - p_pt0->Z) << 8;
    prc_dist20_x = (p_pt2->X - p_pt0->X) << 8;
    prc_distc0_x = x - ((p_sobj->MapX + p_pt0->X) << 8);
    prc_distc0_z = z - ((p_sobj->MapZ + p_pt0->Z) << 8);

    int factorA, factorB, factorLen;
    int m1, m2, m3, m4;
    int height;

    m3 = mul_shift16_sign_pad_lo(prc_dist10_x, prc_dist20_z);
    m4 = mul_shift16_sign_pad_lo(prc_dist10_z, prc_dist20_x);
    factorLen = m3 - m4;
    if (factorLen == 0)
      factorLen = 1;

    m1 = mul_shift16_sign_pad_lo(prc_dist20_z, prc_distc0_x);
    m2 = mul_shift16_sign_pad_lo(prc_dist20_x, prc_distc0_z);
    factorA = ((m1 - (s64)m2) << 16) / factorLen;

    m1 = mul_shift16_sign_pad_lo(prc_dist10_x, prc_distc0_z);
    m2 = mul_shift16_sign_pad_lo(prc_dist10_z, prc_distc0_x);
    factorB = ((m1 - (s64)m2) << 16) / factorLen;

    if ((factorA <= 0 || factorA > 0x10000)) {
        return 0;
    }
    if ((factorB <= 0 || factorB > 0x10000)) {
        return 0;
    }

    height = ((p_pt0->Y + p_sobj->OffsetY) << 8);
    height += mul_shift16_sign_pad_lo(prc_dist10_y, factorA);
    height += mul_shift16_sign_pad_lo(prc_dist20_y, factorB);

    return height >> 3;
}

/** Checks if a face should not be allowed to walk on due to sharp slope.
 *
 * To do such check during gameplay, SingleObjectFace flags should be
 * used - this one is only to update these flags, if neccessary.
 */
TbBool compute_face_is_blocking_walk(short face)
{
    int alt_dt, gnd_dt;
    int angle;

    alt_dt = alt_change_at_face(face, &gnd_dt);

    angle = LbArcTanAngle(alt_dt,-gnd_dt);

    // If steepness is higher than the set limit, then it is blocking
    if (angle > MAX_WALKABLE_STEEPNESS)
        return true;

   return false;
}

void update_object_faces_flags(void)
{
    short face;

    for (face = 1; face < next_object_face3; face++)
    {
        struct SingleObjectFace3 *p_face;

        p_face = &game_object_faces3[face];
        p_face->GFlags |= FGFlg_Unkn04;
        if (compute_face_is_blocking_walk(face))
            p_face->GFlags &= ~FGFlg_Unkn04;
    }
    for (face = 1; face < next_object_face4; face++)
    {
        struct SingleObjectFace4 *p_face;

        p_face = &game_object_faces4[face];
        p_face->GFlags |= FGFlg_Unkn04;
        if (compute_face_is_blocking_walk(-face))
            p_face->GFlags &= ~FGFlg_Unkn04;
    }
}

TbBool face_is_blocking_walk(short face)
{
    if (face < 0)
    {
        struct SingleObjectFace4 *p_face;
        p_face = &game_object_faces4[-face];
        return ((p_face->GFlags & FGFlg_Unkn04) == 0);
    }
    else if (face > 0)
    {
        struct SingleObjectFace3 *p_face;
        p_face = &game_object_faces3[face];
        return ((p_face->GFlags & FGFlg_Unkn04) == 0);
    }

   return false;
}

void unkn_object_shift_03(ushort objectno)
{
    asm volatile ("call ASM_unkn_object_shift_03\n"
        : : "a" (objectno));
}

void unkn_object_shift_02(int norm1, int norm2, ushort objectno)
{
    asm volatile ("call ASM_unkn_object_shift_02\n"
        : : "a" (norm1), "d" (norm2), "b" (objectno));
}

/******************************************************************************/
