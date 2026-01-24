/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstx_fac.c
 *     Drawlists execution for the 3D engine, 3D face related items.
 * @par Purpose:
 *     Implements functions for executing previously made drawlists,
 *     meaning the actual drawing based on primitives in the list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engindrwlstx.h"

#include <assert.h>

#include "bfgentab.h"
#include "poly.h"

#include "engincolour.h"
#include "enginfexpl.h"
#include "enginfloor.h"
#include "enginprops.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginshrapn.h"
#include "engintrns.h"
#include "engintxtrmap.h"
#include "render_gpoly.h"
#include "privrdlog.h"

#include "engindrwlstm.h"
/******************************************************************************/
#define SINGLE_TEXTURE_DIM 32

extern long dword_176D00;
extern long dword_176D04;

extern s32 engn_nuclear_shade_timer;
extern s32 engn_nuclear_shade_x;
extern s32 engn_nuclear_shade_y;
extern s32 engn_nuclear_shade_z;

ScreenTriangleRenderCallback screen_position_face_render_cb = NULL;

/******************************************************************************/

void set_floor_texture_uv(ushort sftex, struct PolyPoint *p_pt1, struct PolyPoint *p_pt2,
  struct PolyPoint *p_pt3, struct PolyPoint *p_pt4, ubyte gflags)
{
    struct SingleFloorTexture *p_sftex;

    if (sftex >= game_textures_limit) {
        LOGERR("Texture %d above allocated count of %d", (int)sftex, game_textures_limit);
        sftex = 0; // continue with texture 0 instead
    }

    p_sftex = &game_textures[sftex];

    assert(vec_tmap[p_sftex->Page] != NULL);
    vec_map = vec_tmap[p_sftex->Page];
    if ((gflags & FGFlg_Unkn02) != 0) {
        assert(scratch_buf1 != NULL);
        vec_map = scratch_buf1;
    }

    p_pt1->U = p_sftex->TMapX1 << 16;
    p_pt1->V = p_sftex->TMapY1 << 16;
    p_pt2->U = p_sftex->TMapX2 << 16;
    p_pt2->V = p_sftex->TMapY2 << 16;
    p_pt3->U = p_sftex->TMapX3 << 16;
    p_pt3->V = p_sftex->TMapY3 << 16;
    p_pt4->U = p_sftex->TMapX4 << 16;
    p_pt4->V = p_sftex->TMapY4 << 16;
}

/** Set texture as active and put its UV coordinates into given points.
 */
void set_face_texture_uv(ushort stex, struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2, struct PolyPoint *p_pt3, ubyte gflags)
{
    struct SingleTexture *p_stex;

    if (stex >= face_textures_limit) {
        LOGERR("Texture %d above allocated count of %d", (int)stex, face_textures_limit);
        stex = 0; // continue with texture 0 instead
    }

    p_stex = &game_face_textures[stex];

    assert(vec_tmap[p_stex->Page] != NULL);
    vec_map = vec_tmap[p_stex->Page];
    if ((gflags & FGFlg_Unkn02) != 0) {
        assert(scratch_buf1 != NULL);
        vec_map = scratch_buf1;
    }

    p_pt1->U = p_stex->TMapX1 << 16;
    p_pt1->V = p_stex->TMapY1 << 16;
    p_pt2->U = p_stex->TMapX2 << 16;
    p_pt2->V = p_stex->TMapY2 << 16;
    p_pt3->U = p_stex->TMapX3 << 16;
    p_pt3->V = p_stex->TMapY3 << 16;
}

/** Sets UV coordinates for damaged ground, given index with neighbors definitions.
 */
void set_floor_texture_uv_damaged_ground(struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2, struct PolyPoint *p_pt3, struct PolyPoint *p_pt4, ubyte neighbrs)
{
    vec_map = vec_tmap[4];

    switch (neighbrs)
    {
    case 1:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        break;
    case 2:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        break;
    case 3:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        break;
    case 4:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        break;
    case 5:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        break;
    case 6:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        break;
    case 7:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        break;
    case 8:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        break;
    case 9:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        break;
    case 10:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 6 - 1) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 5) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        break;
    case 11:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 4) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 5 - 1) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        break;
    case 12:
        p_pt1->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt1->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt2->U = (SINGLE_TEXTURE_DIM * 6) << 16;
        p_pt2->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        p_pt3->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt3->V = (SINGLE_TEXTURE_DIM * 2) << 16;
        p_pt4->U = (SINGLE_TEXTURE_DIM * 7 - 1) << 16;
        p_pt4->V = (SINGLE_TEXTURE_DIM * 3 - 1) << 16;
        break;
    }
}

int calculate_enginepoint_shade_1(struct PolyPoint *p_pt1,
  struct SingleObjectFace3 *p_face, ushort pt2)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_calculate_enginepoint_shade_1\n"
        : "=r" (ret) : "a" (p_pt1), "d" (p_face), "b" (pt2));
    return ret;
#endif
    struct SinglePoint *p_pt2;
    struct SingleObject *p_sobj;
    int dist_x, dist_y, dist_z;
    int distance;

    p_pt2 = &game_object_points[p_face->PointNo[pt2]];
    p_sobj = &game_objects[p_face->Object];

    dist_x = engn_nuclear_shade_x - p_pt2->X - p_sobj->MapX;
    dist_y = engn_nuclear_shade_y - p_pt2->Y - p_sobj->OffsetY;
    dist_z = engn_nuclear_shade_z - p_pt2->Z - p_sobj->MapZ;
    distance = (dist_y * dist_y + dist_x * dist_x + dist_z * dist_z) >> 17;

    if (distance != 0)
        p_pt1->S += engn_nuclear_shade_timer * (0x1000000 / distance);
    else
        p_pt1->S = 0x3F0000;

    if (p_pt1->S > 0x3F0000)
        p_pt1->S = 0x3F0000;

    return p_pt1->S;
}

int calculate_enginepoint_shade_2(struct PolyPoint *p_pt1,
  struct SingleObjectFace4 *p_face4, ushort pt2)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_calculate_enginepoint_shade_2\n"
        : "=r" (ret) : "a" (p_pt1), "d" (p_face4), "b" (pt2));
    return ret;
#endif
    struct SinglePoint *p_pt2;
    struct SingleObject *p_sobj;
    int dist_x, dist_y, dist_z;
    int distance;

    p_pt2 = &game_object_points[p_face4->PointNo[pt2]];
    p_sobj = &game_objects[p_face4->Object];

    dist_x = engn_nuclear_shade_x - p_pt2->X - p_sobj->MapX;
    dist_y = engn_nuclear_shade_y - p_pt2->Y - p_sobj->OffsetY;
    dist_z = engn_nuclear_shade_z - p_pt2->Z - p_sobj->MapZ;
    distance = (dist_y * dist_y + dist_x * dist_x + dist_z * dist_z) >> 17;

    if (distance != 0)
        p_pt1->S += engn_nuclear_shade_timer * (0x1000000 / distance);
    else
        p_pt1->S = 0x3F0000;

    if (p_pt1->S > 0x3F0000)
        p_pt1->S = 0x3F0000;

    return p_pt1->S;
}

void set_nuclear_shade_point(s32 x, s32 y, s32 z)
{
    engn_nuclear_shade_x = x;
    engn_nuclear_shade_y = y;
    engn_nuclear_shade_z = z;
    engn_nuclear_shade_timer = 0;
}

void set_nuclear_shade_timer(ulong tmval)
{
    engn_nuclear_shade_timer = tmval;
}

void set_floor_texture_uv_shade3(ushort face, struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2, struct PolyPoint *p_pt3)
{
    struct SingleObjectFace3 *p_face;

    p_face = &game_object_faces[face];
    assert(vec_tmap[4] != NULL);
    vec_map = vec_tmap[4];
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face->Shade0];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt1->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt1->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face->Shade1];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt2->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt2->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face->Shade2];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt3->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt3->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
}

void set_floor_texture_uv_shade4(ushort face4, struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2, struct PolyPoint *p_pt3, struct PolyPoint *p_pt4)
{
    struct SingleObjectFace4 *p_face4;

    p_face4 = &game_object_faces4[face4];
    assert(vec_tmap[4] != NULL);
    vec_map = vec_tmap[4];
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face4->Shade0];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt1->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt1->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face4->Shade1];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt2->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt2->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face4->Shade2];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt3->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt3->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
    {
        struct Normal *p_nrml;
        sbyte texU, texV;

        p_nrml = &game_normals[p_face4->Shade3];
        texU = p_nrml->LightRatio;
        texV = p_nrml->LightRatio >> 8;
        p_pt4->U = (texU + SINGLE_TEXTURE_DIM * 1) << 16;
        p_pt4->V = (texV + SINGLE_TEXTURE_DIM * 4) << 16;
    }
}

/**
 * Draw rectangular face with textured surface, version G.
 * TODO: figure out how this version is unique.
 *
 * @param face4 Index of SingleObjectFace4 instance.
 */
void draw_object_face4g_textrd(ushort face4)
{
#if 0
    asm volatile (
      "call ASM_draw_object_face4g_textrd\n"
        : : "a" (face4));
    return;
#endif
    struct SingleObjectFace4 *p_face4;
    struct PolyPoint point4;
    struct PolyPoint point3;
    struct PolyPoint point2;
    struct PolyPoint point1;

    p_face4 = &game_object_faces4[face4];
    vec_colour = p_face4->ExCol;
    vec_mode = p_face4->Flags;
    if (p_face4->Texture != 0)
    {
        {
            if ((p_face4->GFlags & (FGFlg_Unkn40|FGFlg_Unkn02)) != 0) {
                uint frame;
                frame = render_anim_turn + 4 * p_face4->Object;
                if ((frame & 0x0F) <= 7) {
                    vec_mode = 2;
                } else {
                    vec_colour = 0;
                    vec_mode = 0;
                }
            }
        }
        if ((p_face4->GFlags & FGFlg_Unkn20) != 0) {
            set_floor_texture_uv(p_face4->Texture, &point1, &point3, &point4, &point2, p_face4->GFlags);
        } else {
            set_floor_texture_uv(p_face4->Texture, &point1, &point3, &point2, &point4, p_face4->GFlags);
        }
    }
#if 0 // unused, and breaks render-app separation, so disabled
    if (ingame.draw_unknprop_01 == 0)
        ingame.draw_unknprop_01 = face4;
#endif
    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[0]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point1.X = p_scrpoint->X + dword_176D00;
        point1.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face4->Shade0];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point1.S = shade << 14;
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[2]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point2.X = p_scrpoint->X + dword_176D00;
        point2.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face4->Shade2];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point2.S = shade << 14;
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[1]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point3.X = p_scrpoint->X + dword_176D00;
        point3.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face4->Shade1];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point3.S = shade << 14;
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[3]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point4.X = p_scrpoint->X + dword_176D00;
        point4.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face4->Shade3];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point4.S = shade << 14;
    }

    {
        ushort mag;
        mag = (p_face4->WalkHeader + (p_face4->GFlags >> 2)) & 7;
        if (mag != 0)
        {
            point1.S = (mag * point1.S) >> 3;
            point2.S = (mag * point2.S) >> 3;
            point3.S = (mag * point3.S) >> 3;
            point4.S = (mag * point4.S) >> 3;
        }
    }
    dword_176D4C++;

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
    }

    if ((p_face4->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point3, &point2);
    }

    if (p_face4->Texture != 0)
    {
        if ((p_face4->GFlags & FGFlg_Unkn20) != 0) {
            set_floor_texture_uv(p_face4->Texture, &point1, &point3, &point4, &point2, p_face4->GFlags);
        } else {
            set_floor_texture_uv(p_face4->Texture, &point1, &point3, &point2, &point4, p_face4->GFlags);
        }
    }
    dword_176D4C++;

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point3, &point2);
    }

    if ((p_face4->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point2, &point3);
    }
}

/**
 * Draw triangular face with reflective (mirror) surface.
 *
 * @param face Index of SingleObjectFace3 instance.
 */
void draw_object_face3_reflect(ushort face3)
{
#if 0
    asm volatile (
      "call ASM_draw_object_face3_reflect\n"
        : : "a" (face3));
    return;
#endif
    struct SingleObjectFace3 *p_face;
    struct PolyPoint point3;
    struct PolyPoint point1;
    struct PolyPoint point2;

    p_face = &game_object_faces[face3];
    vec_colour = p_face->ExCol;
    vec_mode = 27;
    set_floor_texture_uv_shade3(face3, &point1, &point3, &point2);

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face->PointNo[0]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point1.X = p_scrpoint->X + dword_176D00;
        point1.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face->PointNo[2]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point2.X = p_scrpoint->X + dword_176D00;
        point2.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face->PointNo[1]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point3.X = p_scrpoint->X + dword_176D00;
        point3.Y = p_scrpoint->Y + dword_176D04;
    }
    dword_176D4C++;

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
    }

    if ((p_face->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point3, &point2);
    }
}

/**
 * Draw rectangular face with reflective (mirror) surface.
 *
 * @param face4 Index of SingleObjectFace4 instance.
 */
void draw_object_face4_reflect(ushort face4)
{
#if 0
    asm volatile (
      "call ASM_draw_object_face4_reflect\n"
        : : "a" (face4));
    return;
#endif
    struct SingleObjectFace4 *p_face4;
    struct PolyPoint point4;
    struct PolyPoint point1;
    struct PolyPoint point3;
    struct PolyPoint point2;

    p_face4 = &game_object_faces4[face4];
    vec_colour = p_face4->ExCol;
    vec_mode = 27;
    set_floor_texture_uv_shade4(face4, &point1, &point3, &point2, &point4);

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[0]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point1.X = p_scrpoint->X + dword_176D00;
        point1.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[2]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point2.X = p_scrpoint->X + dword_176D00;
        point2.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[1]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point3.X = p_scrpoint->X + dword_176D00;
        point3.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[3]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point4.X = p_scrpoint->X + dword_176D00;
        point4.Y = p_scrpoint->Y + dword_176D04;
    }
    dword_176D4C++;

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
    }

    if ((p_face4->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point3, &point2);
    }
    dword_176D4C++;

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point3, &point2);
    }

    if ((p_face4->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point2, &point3);
    }
}

void draw_ex_face(ushort exface)
{
#if 0
    asm volatile (
      "call ASM_draw_ex_face\n"
        : : "a" (exface));
    return;
#endif
    struct ExplodeFace3 *p_exface;
    struct PolyPoint point2;
    struct PolyPoint point3;
    struct PolyPoint point1;
    struct PolyPoint point4;
    struct SpecialPoint *p_scrpoint;

    p_exface = &ex_faces[exface];
    p_scrpoint = &game_screen_point_pool[p_exface->PointOffset];
    vec_colour = p_exface->Col;
    vec_mode = p_exface->Flags;

    switch (p_exface->Type)
    {
    case 1:
    case 3:
    case 5:
      set_face_texture_uv(p_exface->Texture, &point1, &point3, &point2, 0);

      point1.X = p_scrpoint[0].X;
      point1.Y = p_scrpoint[0].Y;
      point1.S = 0x100000;

      point3.X = p_scrpoint[1].X;
      point3.Y = p_scrpoint[1].Y;
      point3.S = 0x100000;

      point2.X = p_scrpoint[2].X;
      point2.Y = p_scrpoint[2].Y;
      point2.S = 0x100000;

      if (vec_mode == 2)
          vec_mode = 27;
      draw_trigpoly(&point1, &point2, &point3);

      if (vec_mode == 2)
          vec_mode = 27;
      draw_trigpoly(&point3, &point2, &point1);
      dword_176D4C++;
      break;
    case 2:
    case 4:
    case 6:
      set_floor_texture_uv(p_exface->Texture, &point1, &point2, &point3, &point4, 0);

      point1.X = p_scrpoint[0].X;
      point1.Y = p_scrpoint[0].Y;
      point1.S = 0x100000;

      point2.X = p_scrpoint[1].X;
      point2.Y = p_scrpoint[1].Y;
      point2.S = 0x100000;

      point3.X = p_scrpoint[2].X;
      point3.Y = p_scrpoint[2].Y;
      point3.S = 0x100000;

      point4.X = p_scrpoint[3].X;
      point4.Y = p_scrpoint[3].Y;
      point4.S = 0x100000;

      if (vec_mode == 2)
          vec_mode = 27;
      draw_trigpoly(&point1, &point2, &point3);

      if (vec_mode == 2)
          vec_mode = 27;
      draw_trigpoly(&point3, &point2, &point1);

      if (vec_mode == 2)
          vec_mode = 27;
      draw_trigpoly(&point2, &point3, &point4);

      if (vec_mode == 2)
          vec_mode = 27;
      draw_trigpoly(&point4, &point3, &point2);
      dword_176D4C++;
      break;
    default:
      break;
    }
}

/**
 * Draw triangular face with textured surface, version G.
 * TODO: figure out how this version is unique.
 *
 * @param face Index of SingleObjectFace3 instance.
 */
void draw_object_face3g_textrd(ushort face)
{
#if 0
    asm volatile (
      "call ASM_draw_object_face3g_textrd\n"
        : : "a" (face));
    return;
#endif
    struct SingleObjectFace3 *p_face;
    struct PolyPoint point1;
    struct PolyPoint point3;
    struct PolyPoint point2;

    p_face = &game_object_faces[face];
    vec_colour = p_face->ExCol;
    vec_mode = p_face->Flags;

    if (p_face->Texture != 0)
    {
        if ((p_face->GFlags & FGFlg_Unkn40) != 0) {
            uint frame;
            frame = render_anim_turn + p_face->Object;
            if ((frame & 0x1F) > 0x10)
                vec_mode = 5;
        }
        set_face_texture_uv(p_face->Texture, &point1, &point3, &point2, p_face->GFlags);
    }
    else
    {
        vec_mode = 4;
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face->PointNo[0]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point1.X = p_scrpoint->X + dword_176D00;
        point1.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face->Shade0];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point1.S = shade << 14;
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face->PointNo[2]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point2.X = p_scrpoint->X + dword_176D00;
        point2.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face->Shade2];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point2.S = shade << 14;
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face->PointNo[1]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point3.X = p_scrpoint->X + dword_176D00;
        point3.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct Normal *p_nrml;
        int shade;

        p_nrml = &game_normals[p_face->Shade1];
        shade = p_nrml->LightRatio >> 16;
        if (shade < 0)
            shade = 0;
        point3.S = shade << 14;
    }

    {
        ushort mag;
        mag = (p_face->WalkHeader + (p_face->GFlags >> 2)) & 7;
        if (mag != 0)
        {
            point1.S = (mag * point1.S) >> 3;
            point2.S = (mag * point2.S) >> 3;
            point3.S = (mag * point3.S) >> 3;
        }
    }

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
        dword_176D4C++;
    }

    if ((p_face->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point3, &point2);
        dword_176D4C++;
    }
}

void draw_object_face4d_textrd_dk(ushort face4)
{
#if 0
    asm volatile (
      "call ASM_draw_object_face4d_textrd_dk\n"
        : : "a" (face4));
    return;
#endif
    struct SingleObjectFace4 *p_face4;
    struct PolyPoint point4;
    struct PolyPoint point1;
    struct PolyPoint point2;
    struct PolyPoint point3;

    p_face4 = &game_object_faces4[face4];
    vec_mode = 4;
    vec_colour = 64;

    if (p_face4->Texture != 0)
    {
        set_floor_texture_uv(p_face4->Texture, &point1, &point3, &point2, &point4, p_face4->GFlags);
    }

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[0]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point1.X = p_scrpoint->X + dword_176D00;
        point1.Y = p_scrpoint->Y + dword_176D04;
    }
    if (vec_mode == 2)
    {
        point2.S = 0x200000;
    }
    else
    {
        uint shade;

        shade = p_face4->Shade0 << 7;
        shade += cummulate_shade_from_quick_lights(p_face4->Light0);
        if (shade > 0x7E00)
            shade = 0x7F00;
        point1.S = shade << 7;
    }
    point1.S = calculate_enginepoint_shade_2(&point1, p_face4, 0);

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[2]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point2.X = p_scrpoint->X + dword_176D00;
        point2.Y = p_scrpoint->Y + dword_176D04;
    }
    if (vec_mode == 2)
    {
        point2.S = 0x200000;
    }
    else
    {
        uint shade;

        shade = p_face4->Shade2 << 7;
        shade += cummulate_shade_from_quick_lights(p_face4->Light2);
        if (shade > 0x7E00)
            shade = 0x7F00;
        point2.S = shade << 7;
    }
    point2.S = calculate_enginepoint_shade_2(&point2, p_face4, 2);

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[1]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point3.X = p_scrpoint->X + dword_176D00;
        point3.Y = p_scrpoint->Y + dword_176D04;
    }
    if (vec_mode == 2)
    {
        point3.S = 0x200000;
    }
    else
    {
        uint shade;

        shade = p_face4->Shade1 << 7;
        shade += cummulate_shade_from_quick_lights(p_face4->Light1);
        if (shade > 0x7E00)
            shade = 0x7F00;
        point3.S = shade << 7;
    }
    point3.S = calculate_enginepoint_shade_2(&point2, p_face4, 1); //TODO why point2? is that a coding mistake?

    {
        struct SinglePoint *p_point;
        struct SpecialPoint *p_scrpoint;

        p_point = &game_object_points[p_face4->PointNo[3]];
        p_scrpoint = &game_screen_point_pool[p_point->PointOffset];
        point4.X = p_scrpoint->X + dword_176D00;
        point4.Y = p_scrpoint->Y + dword_176D04;
    }
    if (vec_mode == 2)
    {
        point4.S = 0x200000;
    }
    else
    {
        uint shade;

        shade = p_face4->Shade3 << 7;
        shade += cummulate_shade_from_quick_lights(p_face4->Light3);
        if (shade > 0x7E00)
            shade = 0x7F00;
        point4.S = shade << 7;
    }
    point4.S = calculate_enginepoint_shade_2(&point4, p_face4, 3);

    if (!engine_render_lights)
    {
        point1.S = 0x200000;
        point2.S = 0x200000;
        point3.S = 0x200000;
        point4.S = 0x200000;
    }

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
    }
    if ((p_face4->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point3, &point2);
    }
    dword_176D4C++;

    if (p_face4->Texture != 0)
    {
        struct SingleFloorTexture *p_sftex;

        p_sftex = &game_textures[p_face4->Texture];
        point4.U = p_sftex->TMapX4 << 16;
        point4.V = p_sftex->TMapY4 << 16;
        point2.U = p_sftex->TMapX3 << 16;
        point2.V = p_sftex->TMapY3 << 16;
        point3.U = p_sftex->TMapX2 << 16;
        point3.V = p_sftex->TMapY2 << 16;
    }

    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point3, &point2);
    }
    if ((p_face4->GFlags & FGFlg_Unkn01) != 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point2, &point3);
    }
    dword_176D4C++;
}

void draw_floor_tile1a(ushort tl)
{
#if 0
    asm volatile (
      "call ASM_draw_floor_tile1a\n"
        : : "a" (tl));
    return;
#endif
    struct FloorTile *p_floortl;
    struct PolyPoint point3;
    struct PolyPoint point1;
    struct PolyPoint point2;
    struct PolyPoint point4;
    int dist;

    p_floortl = &game_floor_tiles[tl];
    vec_colour = p_floortl->Col;
    vec_mode = p_floortl->Flags;
    if ((p_floortl->Flags == 5) || (p_floortl->Flags == 21))
    {
        if (engine_render_lights) {
            if ((render_floor_flags & RendFlrF_NonPlanetary) != 0)
                vec_mode = 6; // Different light mode
        } else {
            if (p_floortl->Flags == 5)
                vec_mode = 2;
            else
                vec_mode = 19;
        }
        set_floor_texture_uv(floor_texture_index(p_floortl->Texture), &point2, &point4, &point3, &point1, 0);
    }
    point1.X = p_floortl->X[0];
    point1.Y = p_floortl->Y[0];
    point1.S = p_floortl->Shade[0] << 7;
    point2.X = p_floortl->X[3];
    point2.Y = p_floortl->Y[3];
    point2.S = p_floortl->Shade[3] << 7;
    point3.X = p_floortl->X[1];
    point3.Y = p_floortl->Y[1];
    point3.S = p_floortl->Shade[1] << 7;
    point4.X = p_floortl->X[2];
    point4.Y = p_floortl->Y[2];
    point4.S = p_floortl->Shade[2] << 7;

    if (game_perspective == 7) {
        vec_mode = 7;
        vec_colour = point3.S >> 16;
    }

    dist = (point3.Y - point2.Y) * (point2.X - point1.X)
       - (point3.X - point2.X) * (point2.Y - point1.Y);
    if (dist > 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
    }

    dist = (point2.X - point3.X) * (point4.Y - point2.Y)
       - (point2.Y - point3.Y) * (point4.X - point2.X);
    if (dist > 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point3, &point2, &point4);
    }
    dword_176D4C += 2;

    // damage overlays
    if ((p_floortl->Page > 0) && (p_floortl->Page <= 12))
    {
        vec_mode = 6;
        set_floor_texture_uv_damaged_ground(&point1, &point2, &point3, &point4, p_floortl->Page);
        draw_trigpoly(&point1, &point2, &point3);
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point3, &point2, &point4);
        dword_176D4C += 2;
    }
}

void draw_floor_tile1b(ushort tl)
{
#if 0
    asm volatile (
      "call ASM_draw_floor_tile1b\n"
        : : "a" (tl));
    return;
#endif
    struct FloorTile *p_floortl;
    struct PolyPoint point2;
    struct PolyPoint point1;
    struct PolyPoint point4;
    struct PolyPoint point3;
    int dist;

    p_floortl = &game_floor_tiles[tl];
    vec_colour = p_floortl->Col;
    vec_mode = p_floortl->Flags;
    if ((p_floortl->Flags == 5) || (p_floortl->Flags == 21))
    {
        if (engine_render_lights) {
            if ((render_floor_flags & RendFlrF_NonPlanetary) != 0)
                vec_mode = 6; // Different light mode
        } else {
            if (p_floortl->Flags == 5)
                vec_mode = 2;
            else
                vec_mode = 19;
        }
        set_floor_texture_uv(floor_texture_index(p_floortl->Texture), &point2, &point4, &point3, &point1, 0);
    }
    point1.X = p_floortl->X[0];
    point1.Y = p_floortl->Y[0];
    point1.S = p_floortl->Shade[0] << 7;
    point2.X = p_floortl->X[3];
    point2.Y = p_floortl->Y[3];
    point2.S = p_floortl->Shade[3] << 7;
    point3.X = p_floortl->X[1];
    point3.Y = p_floortl->Y[1];
    point3.S = p_floortl->Shade[1] << 7;
    point4.X = p_floortl->X[2];
    point4.Y = p_floortl->Y[2];
    point4.S = p_floortl->Shade[2] << 7;

    if (game_perspective == 7) {
        vec_mode = 7;
        vec_colour = point3.S << 16;
    }

    dist = (point2.X - point1.X) * (point4.Y - point2.Y)
       - (point2.Y - point1.Y) * (point4.X - point2.X);
    if (dist > 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point4);
    }
    dist = (point3.X - point4.X) * (point1.Y - point3.Y)
       - (point3.Y - point4.Y) * (point1.X - point3.X);
    if (dist > 0)
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point3, &point1);
    }
    dword_176D4C += 2;

    // damage overlays
    if ((p_floortl->Page > 0) && (p_floortl->Page <= 12))
    {
        vec_mode = 6;
        set_floor_texture_uv_damaged_ground(&point1, &point2, &point3, &point4, p_floortl->Page);
        draw_trigpoly(&point1, &point2, &point4);
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point3, &point1);
        dword_176D4C += 2;
    }
    //TODO why the second time using the same page?
    if ((p_floortl->Page > 0) && (p_floortl->Page <= 12))
    {
        vec_mode = 6;
        set_floor_texture_uv_damaged_ground(&point1, &point2, &point3, &point4, p_floortl->Page);
        draw_trigpoly(&point1, &point2, &point4);
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point4, &point3, &point1);
        dword_176D4C += 2;
    }
}

/**
 * Draw shrapnel polygon.
 *
 * @param shrap Index of Shrapnel instance.
 */
void draw_shrapnel(ushort shrap)
{
    struct Shrapnel *p_shrap;
    struct PolyPoint point3;
    struct PolyPoint point2;
    struct PolyPoint point1;

    p_shrap = &shrapnel[shrap];

    {
        struct SpecialPoint *p_scrpoint;

        p_scrpoint = &game_screen_point_pool[p_shrap->PointOffset + 0];
        point1.X = p_scrpoint->X + dword_176D00;
        point1.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SpecialPoint *p_scrpoint;

        p_scrpoint = &game_screen_point_pool[p_shrap->PointOffset + 1];
        point2.X = p_scrpoint->X + dword_176D00;
        point2.Y = p_scrpoint->Y + dword_176D04;
    }
    {
        struct SpecialPoint *p_scrpoint;

        p_scrpoint = &game_screen_point_pool[p_shrap->PointOffset + 2];
        point3.X = p_scrpoint->X + dword_176D00;
        point3.Y = p_scrpoint->Y + dword_176D04;
    }

    vec_mode = 0;
    vec_colour = colour_lookup[8];
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point2, &point3);
    }
    vec_colour = colour_lookup[9];
    {
        if (vec_mode == 2)
            vec_mode = 27;
        draw_trigpoly(&point1, &point3, &point2);
    }
}

/******************************************************************************/
