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

#include "enginpritxtr.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "game_data.h"
#include "game_options.h"
#include "thing.h"
#include "swlog.h"
/******************************************************************************/

ushort prim_object_points_count = 1;
ushort prim_object_faces_count = 1;
ushort prim_object_faces4_count = 1;
ushort prim_objects_count = 1;

extern ushort word_19CB58[66];

ushort old_next_object, old_next_object_face, old_next_object_point,
  old_next_normal, old_next_face_texture;

void read_primveh_obj(const char *fname, int a2)
{
    long firstval;
    TbFileHandle fh;

    fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    if ( fh == INVALID_FILE )
        return;
    LbFileRead(fh, &firstval, sizeof(long));
    if ( firstval != 1 )
    {
      LbFileRead(fh, &prim_object_points_count, sizeof(ushort));
      LbFileRead(fh, &prim_object_faces_count, sizeof(ushort));
      LbFileRead(fh, &prim_object_faces4_count, sizeof(ushort));
      LbFileRead(fh, &prim_objects_count, sizeof(ushort));
      LbFileRead(fh, &prim4_textures_count, sizeof(ushort));
      LbFileRead(fh, &prim_face_textures_count, sizeof(ushort));
      LbFileRead(fh, &prim_unknprop01, sizeof(ushort));
      LbFileRead(fh, prim_object_points, sizeof(struct SinglePoint) * prim_object_points_count);
      LbFileRead(fh, prim_object_faces, sizeof(struct SingleObjectFace3) * prim_object_faces_count);
      LbFileRead(fh, prim_object_faces4, sizeof(struct SingleObjectFace4) * prim_object_faces4_count);
      LbFileRead(fh, prim_objects, sizeof(struct SingleObject) * prim_objects_count);
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
    asm volatile ("call ASM_calc_normal\n"
        : : "a" (face), "d" (p_normal));
}

void calc_normal4(short face, struct Normal *p_normal)
{
    asm volatile ("call ASM_calc_normal4\n"
        : : "a" (face), "d" (p_normal));
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

        if (next_normal + 1 >= mem_game[8].N) {
            return 0;
        }
        nrml = next_normal++;
        p_nnrml = &game_normals[nrml];
        memcpy(p_nnrml, &loc_nrml, sizeof(struct Normal));
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

        if (next_normal + 1 > mem_game[8].N) {
            return 0;
        }
        nrml = next_normal++;
        p_nnrml = &game_normals[nrml];
        memcpy(p_nnrml, &loc_nrml, sizeof(struct Normal));
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
            memcpy(p_nftextr, p_pftextr, sizeof(struct SingleFloorTexture));
            p_panitmap->TMap[i] = new_txtr;
        }
    }
#endif
}

void prim_obj_mem_debug(int itm_beg, int itm_end)
{
}

void copy_prim_obj_to_game_object(short tx, short tz, short prim_obj, short ty)
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
        return;
    if ((tz < 0) || (tz > 0x7fff))
        return;
#pragma GCC diagnostic pop

    old_next_object = next_object;
    old_next_object_face = next_object_face;
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

    if (next_object > mem_game[5].N)
        return;
    new_obj = next_object++;
    p_nsngobj = &game_objects[new_obj];
    p_nsngobj->OffsetX = tx;
    p_nsngobj->OffsetY = ty;
    p_nsngobj->MapX = tx;
    p_nsngobj->MapZ = tz;
    p_nsngobj->OffsetZ = tz;
    p_nsngobj->ThingNo = things_empty_head;
    p_nsngobj->StartFace = next_object_face;
    p_nsngobj->NumbFaces = face_num;

    if (next_object_point + (pt_end - pt_beg) > mem_game[3].N)
        return;

    for (pt = pt_beg; pt <= pt_end; pt++)
    {
        struct SinglePoint *p_psngpt;
        struct SinglePoint *p_nsngpt;
        ushort new_pt;

        new_pt = next_object_point++;
        p_psngpt = &prim_object_points[pt];
        p_nsngpt = &game_object_points[new_pt];

        memcpy(p_nsngpt, p_psngpt, sizeof(struct SinglePoint));

        point_unkn_func_03(p_nsngpt);
        if (ingame.LowerMemoryUse == 3)
            prim_obj_mem_debug(pt, pt+1);
    }

    for (face_dt = 0; face_dt < face_num; face_dt++)
    {
        struct SingleObjectFace3 *p_pface;
        struct SingleObjectFace3 *p_nface;
        ushort new_face;

        if (ingame.LowerMemoryUse == 3)
            prim_obj_mem_debug(face_beg, face_beg + face_dt);

        p_pface = &prim_object_faces[face_beg + face_dt];
        if (next_object_face + 3 > mem_game[4].N)
            return;
        new_face = next_object_face++;
        p_nface = &game_object_faces[new_face];
        if (p_pface->Texture != 0)
        {
            struct SingleTexture *p_nstxtr;
            struct SingleTexture *p_pstxtr;
            ushort new_txtr;

            p_pstxtr = &prim_face_textures[p_pface->Texture];
            new_txtr = 0;
            if ((p_pface->GFlags & 0x10) == 0) // If disallow reuse of textures is not set
            {
                  new_txtr = find_face_texture(p_pstxtr);
            }
            if (new_txtr == 0)
            {
                if (next_face_texture + 2 > mem_game[2].N)
                    return;
                new_txtr = next_face_texture++;
                p_nstxtr = &game_face_textures[new_txtr];
                memcpy(p_nstxtr, p_pstxtr, sizeof(struct SingleTexture));
            }
            p_nface->Texture = new_txtr;
            if (ingame.LowerMemoryUse == 3)
              prim_obj_mem_debug(-next_face_texture, 0);
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
    }

    p_psngobj = &prim_objects[prim_obj];
    face_num = p_psngobj->NumbFaces4;
    face_beg = p_psngobj->StartFace4;
    if (face_beg > mem_game[22].N)
        return;

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

        if (ingame.LowerMemoryUse == 3)
            prim_obj_mem_debug(-face_beg, -(face_beg + face_dt));

        p_pface = &prim_object_faces4[face_beg + face_dt];
        if (next_object_face4 + 2 > mem_game[9].N)
            return;
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

                if (next_anim_tmap > mem_game[10].N)
                    return;
                new_anitmap = next_anim_tmap++;
                p_nanitmap = &game_anim_tmaps[new_anitmap];
                p_panitmap = &game_anim_tmaps[p_pface->Texture + 1];
                memcpy(p_nanitmap, p_panitmap, sizeof(struct AnimTmap));
                update_texture_from_anim_tmap(new_anitmap);

                if (next_floor_texture + 1 > mem_game[1].N)
                    return;
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
            new_txtr = 0;
            if ((p_pface->GFlags & 0x10) == 0)
            {
                new_txtr = find_floor_texture(p_pftxtr);
            }
            if (new_txtr == 0)
            {
                if (next_floor_texture + 1 > mem_game[1].N)
                    return;
                new_txtr = next_floor_texture++;
                p_nftxtr = &game_textures[new_txtr];
                memcpy(p_nftxtr, p_pftxtr, sizeof(struct SingleFloorTexture));
            }
            p_nface->Texture = new_txtr;
            if (ingame.LowerMemoryUse == 3)
                prim_obj_mem_debug(next_floor_texture, 0);
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
    }

    if (ingame.LowerMemoryUse == 3)
        prim_obj_mem_debug(-10005, 0);
    if (ingame.LowerMemoryUse == 3)
        prim_obj_mem_debug(-10000, 0);
#endif
}

/******************************************************************************/
