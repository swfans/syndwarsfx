/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginpriobjs.c
 *     Primitive objects support required by the 3D engine.
 * @par Purpose:
 *     Implement functions for handling primitive objects in 3D world.
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
#include "enginpriobjs.h"

#include "bffile.h"
#include "bfmath.h"
#include "bfmemut.h"

#include "enginpritxtr.h"
#include "enginprops.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "privrdlog.h"
/******************************************************************************/

ushort next_prim_object_point = 1;
ushort next_prim_object_face3 = 1;
ushort next_prim_object_face4 = 1;
ushort next_prim_object = 1;

extern ushort word_19CB58[66];

ushort old_next_object, old_next_object_face3, old_next_object_point,
  old_next_normal, old_next_face_texture;

void read_primveh_obj(const char *fname, int a2)
{
    long firstval;
    TbFileHandle fh;

    fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if (fh == INVALID_FILE)
        return;
    LbFileRead(fh, &firstval, sizeof(long));
    if (firstval != 1)
    {
      LbFileRead(fh, &next_prim_object_point, sizeof(ushort));
      LbFileRead(fh, &next_prim_object_face3, sizeof(ushort));
      LbFileRead(fh, &next_prim_object_face4, sizeof(ushort));
      LbFileRead(fh, &next_prim_object, sizeof(ushort));
      LbFileRead(fh, &prim4_textures_count, sizeof(ushort));
      LbFileRead(fh, &prim_face_textures_count, sizeof(ushort));
      LbFileRead(fh, &prim_unknprop01, sizeof(ushort));
      LbFileRead(fh, prim_object_points, sizeof(struct SinglePoint) * next_prim_object_point);
      LbFileRead(fh, prim_object_faces3, sizeof(struct SingleObjectFace3) * next_prim_object_face3);
      LbFileRead(fh, prim_object_faces4, sizeof(struct SingleObjectFace4) * next_prim_object_face4);
      LbFileRead(fh, prim_objects, sizeof(struct SingleObject) * next_prim_object);
      LbFileRead(fh, prim4_textures, sizeof(struct SingleFloorTexture) * prim4_textures_count);
      LbFileRead(fh, prim_face_textures, sizeof(struct SingleTexture) * prim_face_textures_count);
    }
    LbFileClose(fh);
}

void point_unkn_func_03(struct SinglePoint *p_point)
{
    int x, y, z;

    x = unkn2_pos_x * p_point->X;
    y = unkn2_pos_y * p_point->Y;
    z = unkn2_pos_z * p_point->Z;
    p_point->X = x >> 6;
    p_point->Y = y >> 6;
    p_point->Z = z >> 6;
}

static TbBool face_textures_equal(const struct SingleTexture *p_textr1, const struct SingleTexture *p_textr2)
{
    return ((p_textr1->TMapX1 == p_textr2->TMapX1)
         && (p_textr1->TMapX2 == p_textr2->TMapX2)
         && (p_textr1->TMapX3 == p_textr2->TMapX3)
         && (p_textr1->TMapY1 == p_textr2->TMapY1)
         && (p_textr1->TMapY2 == p_textr2->TMapY2)
         && (p_textr1->TMapY3 == p_textr2->TMapY3)
         && (p_textr1->Page == p_textr2->Page));
}

static TbBool floor_textures_equal(const struct SingleFloorTexture *p_textr1, const struct SingleFloorTexture *p_textr2)
{
    return ((p_textr1->TMapX1 == p_textr2->TMapX1)
         && (p_textr1->TMapX2 == p_textr2->TMapX2)
         && (p_textr1->TMapX3 == p_textr2->TMapX3)
         && (p_textr1->TMapX4 == p_textr2->TMapX4)
         && (p_textr1->TMapY1 == p_textr2->TMapY1)
         && (p_textr1->TMapY2 == p_textr2->TMapY2)
         && (p_textr1->TMapY3 == p_textr2->TMapY3)
         && (p_textr1->TMapY4 == p_textr2->TMapY4)
         && (p_textr1->Page == p_textr2->Page));
}

static TbBool normals_equal(const struct Normal *p_nrml1, const struct Normal *p_nrml2)
{
    return ((p_nrml1->NX == p_nrml2->NX) &&
          (p_nrml1->NY == p_nrml2->NY) &&
          (p_nrml1->NZ == p_nrml2->NZ));
}

/** Find within game_face_textures index of a texture equivalent to given.
 */
ushort find_face_texture(struct SingleTexture *p_textr)
{
    struct SingleTexture *p_ctextr;
    ushort txtr;

    for (txtr = 1; txtr < next_face_texture; txtr++)
    {
        p_ctextr = &game_face_textures[txtr];
        if (face_textures_equal(p_textr, p_ctextr))
            return txtr;
    }
    return 0;
}

/** Find within game_textures index of a texture equivalent to given.
 */
ushort find_floor_texture(struct SingleFloorTexture *p_textr)
{
    struct SingleFloorTexture *p_ctextr;
    ushort txtr;

    for (txtr = 1; txtr < next_floor_texture; txtr++)
    {
        p_ctextr = &game_textures[txtr];
        if (floor_textures_equal(p_textr, p_ctextr))
            return txtr;
    }
    return 0;
}

ushort find_normal(struct Normal *p_normal)
{
    struct Normal *p_cnormal;
    ushort nrml;

    for (nrml = 1; nrml < next_normal; nrml++)
    {
        p_cnormal = &game_normals[nrml];
        if (normals_equal(p_normal, p_cnormal))
            return nrml;
    }
    return 0;
}

void calc_normal(short face, struct Normal *p_normal)
{
#if 0
    asm volatile ("call ASM_calc_normal\n"
        : : "a" (face), "d" (p_normal));
    return;
#endif
    struct SingleObjectFace3 *p_face;
    struct SinglePoint *p_objpt1;
    struct SinglePoint *p_objpt2;
    struct SinglePoint *p_objpt3;
    int fctB_x, fctB_y, fctB_z, fctB_len;
    int fctC_x, fctC_y, fctC_z, fctC_len;
    int fctD_x, fctD_y, fctD_z;
    int fctE_a, fctE_b;
    int dirvec_x, dirvec_y, dirvec_z, dirvec_len;
    int nx, ny, nz;

    p_face = &game_object_faces3[face];
    p_objpt1 = &game_object_points[p_face->PointNo[0]];
    p_objpt2 = &game_object_points[p_face->PointNo[1]];
    p_objpt3 = &game_object_points[p_face->PointNo[2]];

    fctB_x = p_objpt2->X - p_objpt1->X;
    fctB_y = p_objpt2->Y - p_objpt1->Y;
    fctB_z = p_objpt2->Z - p_objpt1->Z;

    fctC_x = p_objpt3->X - p_objpt2->X;
    fctC_y = p_objpt3->Y - p_objpt2->Y;
    fctC_z = p_objpt3->Z - p_objpt2->Z;

    if (!(fctB_x || fctB_y || fctB_z) || !(fctC_x || fctC_y || fctC_z))
    {
        p_normal->NX = 0;
        p_normal->NY = 255;
        p_normal->NZ = 0;
        return;
    }

    fctB_len = LbSqrL(fctB_x * fctB_x + fctB_y * fctB_y + fctB_z * fctB_z);
    if (fctB_len == 0)
        fctB_len = 1;

    fctD_x = (fctB_x << 8) / fctB_len;
    fctD_y = (fctB_y << 8) / fctB_len;
    fctD_z = (fctB_z << 8) / fctB_len;

    fctC_len = LbSqrL(fctC_x * fctC_x + fctC_y * fctC_y + fctC_z * fctC_z);
    if (fctC_len == 0)
        fctC_len = 1;

    fctE_a = (fctC_x << 8) / fctC_len;
    fctE_b = (fctC_y << 8) / fctC_len;

    dirvec_x = ((fctC_z << 8) / fctC_len * fctD_y - fctD_z * fctE_b) >> 8;
    dirvec_y = (fctE_a * fctD_z - fctD_x * ((fctC_z << 8) / fctC_len)) >> 8;
    dirvec_z = (fctE_b * fctD_x - fctD_y * fctE_a) >> 8;

    dirvec_len = LbSqrL(dirvec_y * dirvec_y + dirvec_x * dirvec_x + dirvec_z * dirvec_z);
    if (dirvec_len == 0)
        dirvec_len = 1;

    nx = (dirvec_x << 8) / dirvec_len;
    ny = (dirvec_y << 8) / dirvec_len;
    nz = (dirvec_z << 8) / dirvec_len;
    if (!nx && !ny && !nz)
        ny = 255;

    p_normal->NX = nx;
    p_normal->NY = ny;
    p_normal->NZ = nz;
}

void calc_normal4(short face, struct Normal *p_normal)
{
#if 0
    asm volatile ("call ASM_calc_normal4\n"
        : : "a" (face), "d" (p_normal));
    return;
#endif
    struct SingleObjectFace4 *p_face;
    struct SinglePoint *p_objpt1;
    struct SinglePoint *p_objpt2;
    struct SinglePoint *p_objpt3;
    int fctB_x, fctB_y, fctB_z, fctB_len;
    int fctC_x, fctC_y, fctC_z, fctC_len;
    int fctD_x, fctD_y, fctD_z;
    int fctE_a, fctE_b, fctE_c;
    int dirvec_x, dirvec_y, dirvec_z, dirvec_len;
    int nx, ny, nz;

    p_face = &game_object_faces4[face];
    p_objpt1 = &game_object_points[p_face->PointNo[0]];
    p_objpt2 = &game_object_points[p_face->PointNo[1]];
    p_objpt3 = &game_object_points[p_face->PointNo[2]];

    fctB_x = p_objpt2->X - p_objpt1->X;
    fctB_y = p_objpt2->Y - p_objpt1->Y;
    fctB_z = p_objpt2->Z - p_objpt1->Z;

    fctC_x = p_objpt3->X - p_objpt2->X;
    fctC_y = p_objpt3->Y - p_objpt2->Y;
    fctC_z = p_objpt3->Z - p_objpt2->Z;

    if (!(fctB_x || fctB_y || fctB_z) || !(fctC_x || fctC_y || fctC_z))
    {
        p_normal->NX = 0;
        p_normal->NY = 255;
        p_normal->NZ = 0;
        return;
    }

    fctB_len = LbSqrL(fctB_z * fctB_z + fctB_x * fctB_x + fctB_y * fctB_y);
    if (fctB_len == 0)
        fctB_len = 1;

    fctD_x = (fctB_x << 8) / fctB_len;
    fctD_y = (fctB_y << 8) / fctB_len;
    fctD_z = (fctB_z << 8) / fctB_len;

    fctC_len = LbSqrL(fctC_y * fctC_y + fctC_x * fctC_x + fctC_z * fctC_z);
    if (fctC_len == 0)
        fctC_len = 1;

    fctE_a = (fctC_x << 8) / fctC_len;
    fctE_b = (fctC_y << 8) / fctC_len;
    fctE_c = (fctC_z << 8) / fctC_len;

    dirvec_x = (fctD_y * fctE_c - fctE_b * fctD_z) >> 8;
    dirvec_y = (fctE_a * fctD_z - fctD_x * fctE_c) >> 8;
    dirvec_z = (fctE_b * fctD_x - fctE_a * fctD_y) >> 8;

    dirvec_len = LbSqrL(dirvec_z * dirvec_z + dirvec_x * dirvec_x + dirvec_y * dirvec_y);
    if ( !dirvec_len )
      dirvec_len = 1;

    nx = (dirvec_x << 8) / dirvec_len;
    ny = (dirvec_y << 8) / dirvec_len;
    nz = (dirvec_z << 8) / dirvec_len;

    if ( !nx && !ny && !nz )
        ny = 255;
    p_normal->NX = nx;
    p_normal->NY = ny;
    p_normal->NZ = nz;
}

ushort obj_face3_create_normal(short face)
{
#if 0
    ushort ret;
    asm volatile (
      "call ASM_obj_face3_create_normal\n"
        : "=r" (ret) : "a" (face));
    return ret;
#else
    struct Normal loc_nrml;
    struct Normal *p_nnrml;
    ushort i;

    loc_nrml.LightRatio = 0;
    calc_normal(face, &loc_nrml);
    i = find_normal(&loc_nrml);
    if (i == 0)
    {
        ushort nrml;

        if (next_normal + 1 >= game_normals_limit) {
            return 0;
        }
        nrml = next_normal++;
        p_nnrml = &game_normals[nrml];
        LbMemoryCopy(p_nnrml, &loc_nrml, sizeof(struct Normal));
        i = nrml;
    }
    return i;
#endif
}

ushort obj_face4_create_normal(short face)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_obj_face4_create_normal\n"
        : "=r" (ret) : "a" (a1));
    return ret;
#endif
    struct Normal loc_nrml;
    struct Normal *p_nnrml;
    int i;

    loc_nrml.LightRatio = 0;
    calc_normal4(face, &loc_nrml);
    i = find_normal(&loc_nrml);
    if (i == 0)
    {
        ushort nrml;

        if (next_normal + 1 > game_normals_limit) {
            return 0;
        }
        nrml = next_normal++;
        p_nnrml = &game_normals[nrml];
        LbMemoryCopy(p_nnrml, &loc_nrml, sizeof(struct Normal));
        i = nrml;
    }
    return i;
}

void update_texture_from_anim_tmap(ushort ani_tmap)
{
#if 0
    asm volatile (
      "call ASM_update_texture_from_anim_tmap\n"
        :  : "a" (ani_tmap));
#else
    struct AnimTmap *p_panitmap;
    ushort i;

    p_panitmap = &game_anim_tmaps[ani_tmap];

    for (i = 0; i < 10; i++)
    {
        ushort prv_txtr;

        prv_txtr = p_panitmap->TMap[i];
        if (prv_txtr != 0)
        {
            struct SingleFloorTexture *p_nftextr;
            struct SingleFloorTexture *p_pftextr;
            ushort new_txtr;

            new_txtr = next_floor_texture++;
            p_nftextr = &game_textures[new_txtr];
            p_pftextr = &game_textures[prv_txtr];
            LbMemoryCopy(p_nftextr, p_pftextr, sizeof(struct SingleFloorTexture));
            p_panitmap->TMap[i] = new_txtr;
        }
    }
#endif
}

ushort copy_prim_obj_to_game_object(short tx, short tz, short prim_obj, short ty)
{
#if 0
    asm volatile ("call ASM_copy_prim_obj_to_game_object\n"
        : : "a" (tx), "d" (tz), "b" (prim_obj), "c" (ty));
#else
    struct SingleObject *p_psngobj;
    struct SingleObject *p_nsngobj;
    ushort face_beg, face_num, face_dt;
    ushort pt_beg, pt_end, pt;
    ushort new_obj;
    ushort new_pt_beg;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if ((tx < 0) || (tx > 0x7fff))
        return 0;
    if ((tz < 0) || (tz > 0x7fff))
        return 0;
#pragma GCC diagnostic pop

    old_next_object = next_object;
    old_next_object_face3 = next_object_face3;
    old_next_object_point = next_object_point;
    old_next_normal = next_normal;
    old_next_face_texture = next_face_texture;

    if (prim_obj < 0)
        prim_obj = -prim_obj;

    new_pt_beg = next_object_point;
    p_psngobj = &prim_objects[prim_obj];
    face_num = p_psngobj->NumbFaces;
    face_beg = p_psngobj->StartFace;
    pt_beg = p_psngobj->StartPoint;
    pt_end = p_psngobj->EndPoint;

    if (next_object > game_objects_limit)
        return 0;
    new_obj = next_object++;
    p_nsngobj = &game_objects[new_obj];
    p_nsngobj->OffsetX = tx;
    p_nsngobj->OffsetY = ty;
    p_nsngobj->MapX = tx;
    p_nsngobj->MapZ = tz;
    p_nsngobj->OffsetZ = tz;
    p_nsngobj->ThingNo = 0;
    p_nsngobj->NumbFaces = 0;
    p_nsngobj->NumbFaces4 = 0;

    if (next_object_point + (pt_end - pt_beg) > game_object_points_limit)
        return new_obj;

    // All points/vertices (for all faces) used by the primitive
    for (pt = pt_beg; pt <= pt_end; pt++)
    {
        struct SinglePoint *p_psngpt;
        struct SinglePoint *p_nsngpt;
        ushort new_pt;

        new_pt = next_object_point++;
        p_psngpt = &prim_object_points[pt];
        p_nsngpt = &game_object_points[new_pt];

        LbMemoryCopy(p_nsngpt, p_psngpt, sizeof(struct SinglePoint));

        point_unkn_func_03(p_nsngpt);
        if (prim_obj_mem_debug != NULL)
            prim_obj_mem_debug(PriEl_PRIM_POINT, pt, pt+1);
    }

    p_nsngobj->StartFace = next_object_face3;
    p_nsngobj->NumbFaces = face_num;

    for (face_dt = 0; face_dt < face_num; face_dt++)
    {
        struct SingleObjectFace3 *p_pface;
        struct SingleObjectFace3 *p_nface;
        ushort new_face;

        if (prim_obj_mem_debug != NULL)
            prim_obj_mem_debug(PriEl_PRIM_FACE3, face_beg + face_dt, face_beg + face_dt + 1);

        p_pface = &prim_object_faces3[face_beg + face_dt];
        if (next_object_face3 + 3 > game_object_faces3_limit) {
            p_nsngobj->NumbFaces = face_dt;
            return new_obj;
        }
        new_face = next_object_face3++;
        p_nface = &game_object_faces3[new_face];
        if (p_pface->Texture != 0)
        {
            struct SingleTexture *p_nstxtr;
            struct SingleTexture *p_pstxtr;
            ushort new_txtr;

            p_pstxtr = &prim_face_textures[p_pface->Texture];
            if (prim_obj_mem_debug != NULL)
                prim_obj_mem_debug(PriEl_PRIM_TEXTR3, p_pface->Texture, p_pface->Texture + 1);
            new_txtr = 0;
            if ((p_pface->GFlags & 0x10) == 0) // If disallow reuse of textures is not set
            {
                  new_txtr = find_face_texture(p_pstxtr);
            }
            if (new_txtr == 0)
            {
                if (next_face_texture + 2 > face_textures_limit) {
                    p_nsngobj->NumbFaces = face_dt;
                    return new_obj;
                }
                new_txtr = next_face_texture++;
                p_nstxtr = &game_face_textures[new_txtr];
                LbMemoryCopy(p_nstxtr, p_pstxtr, sizeof(struct SingleTexture));
            }
            p_nface->Texture = new_txtr;
            if (prim_obj_mem_debug != NULL)
                prim_obj_mem_debug(PriEl_GAME_TEXTR3, new_txtr, new_txtr + 1);
        }
        p_nface->Flags = p_pface->Flags;
        p_nface->GFlags = p_pface->GFlags;
        p_nface->ExCol = p_pface->ExCol;
        p_nface->Object = new_obj;
        p_nface->PointNo[0] = new_pt_beg + p_pface->PointNo[0] - pt_beg;
        p_nface->PointNo[1] = new_pt_beg + p_pface->PointNo[1] - pt_beg;
        p_nface->PointNo[2] = new_pt_beg + p_pface->PointNo[2] - pt_beg;
        p_nface->FaceNormal = obj_face3_create_normal(new_face);
        p_nface->Light0 = 0;
        p_nface->Light1 = 0;
        p_nface->Light2 = 0;
        if (prim_obj_mem_debug != NULL)
            prim_obj_mem_debug(PriEl_GAME_FACE3, new_face, new_face + 1);
    }

    p_psngobj = &prim_objects[prim_obj];
    face_num = p_psngobj->NumbFaces4;
    face_beg = p_psngobj->StartFace4;
    if (face_beg > prim_object_faces4_limit)
        return new_obj;

    p_nsngobj->StartFace4 = next_object_face4;
    p_nsngobj->NumbFaces4 = face_num;
    pt_end = p_psngobj->EndPoint;
    pt_beg = p_psngobj->StartPoint;
    p_nsngobj->StartPoint = new_pt_beg;
    p_nsngobj->EndPoint = new_pt_beg + pt_end - pt_beg;

    for (face_dt = 0; face_dt < face_num; face_dt++)
    {
        struct SingleObjectFace4 *p_pface;
        struct SingleObjectFace4 *p_nface;
        ushort new_face;

        if (prim_obj_mem_debug != NULL)
            prim_obj_mem_debug(PriEl_PRIM_FACE4, face_beg, face_beg + face_dt);

        p_pface = &prim_object_faces4[face_beg + face_dt];
        if (next_object_face4 + 2 > game_object_faces4_limit)
            return new_obj;
        new_face = next_object_face4++;
        p_nface = &game_object_faces4[new_face];

        if ((p_pface->GFlags & 0x20) != 0)
        {
            ushort new_txtr;

            new_txtr = word_19CB58[p_pface->Texture];
            if (new_txtr == 0)
            {
                struct AnimTmap *p_nanitmap;
                struct AnimTmap *p_panitmap;
                ushort new_anitmap;

                if (next_anim_tmap > game_anim_tmaps_limit) {
                    p_nsngobj->NumbFaces4 = face_dt;
                    return new_obj;
                }
                new_anitmap = next_anim_tmap++;
                p_nanitmap = &game_anim_tmaps[new_anitmap];
                p_panitmap = &game_anim_tmaps[p_pface->Texture + 1];
                LbMemoryCopy(p_nanitmap, p_panitmap, sizeof(struct AnimTmap));
                update_texture_from_anim_tmap(new_anitmap);

                if (next_floor_texture + 1 > game_textures_limit) {
                    p_nsngobj->NumbFaces4 = face_dt;
                    return new_obj;
                }
                new_txtr = next_floor_texture++;
                p_nanitmap->field_24 = 0;
                p_nanitmap->Texture = new_txtr;
                word_19CB58[p_pface->Texture] = new_txtr;
            }
            p_nface->Texture = new_txtr;
        }
        else if (p_pface->Texture != 0)
        {
            struct SingleFloorTexture *p_pftxtr;
            struct SingleFloorTexture *p_nftxtr;
            ushort new_txtr;

            p_pftxtr = &prim4_textures[p_pface->Texture];
            if (prim_obj_mem_debug != NULL)
                prim_obj_mem_debug(PriEl_PRIM_TEXTR4, p_pface->Texture, p_pface->Texture + 1);
            new_txtr = 0;
            if ((p_pface->GFlags & 0x10) == 0)
            {
                new_txtr = find_floor_texture(p_pftxtr);
            }
            if (new_txtr == 0)
            {
                if (next_floor_texture + 1 > game_textures_limit) {
                    p_nsngobj->NumbFaces4 = face_dt;
                    return new_obj;
                }
                new_txtr = next_floor_texture++;
                p_nftxtr = &game_textures[new_txtr];
                LbMemoryCopy(p_nftxtr, p_pftxtr, sizeof(struct SingleFloorTexture));
            }
            p_nface->Texture = new_txtr;
            if (prim_obj_mem_debug != NULL)
                prim_obj_mem_debug(PriEl_GAME_TEXTR4, new_txtr, new_txtr + 1);
        }
        p_nface->Flags = p_pface->Flags;
        p_nface->GFlags = p_pface->GFlags;
        p_nface->ExCol = p_pface->ExCol;
        p_nface->Object = new_obj;
        p_nface->PointNo[0] = p_pface->PointNo[0] - pt_beg + new_pt_beg;
        p_nface->PointNo[1] = p_pface->PointNo[1] - pt_beg + new_pt_beg;
        p_nface->PointNo[2] = p_pface->PointNo[2] - pt_beg + new_pt_beg;
        p_nface->PointNo[3] = p_pface->PointNo[3] - pt_beg + new_pt_beg;
        p_nface->FaceNormal = obj_face4_create_normal(new_face);
        p_nface->Light0 = 0;
        p_nface->Light1 = 0;
        p_nface->Light2 = 0;
        p_nface->Light3 = 0;
        if (prim_obj_mem_debug != NULL)
            prim_obj_mem_debug(PriEl_GAME_FACE4, new_face, new_face + 1);
    }

    if (prim_obj_mem_debug != NULL)
        prim_obj_mem_debug(PriEl_NONE, -10000, 0);
    return new_obj;
#endif
}

/******************************************************************************/
