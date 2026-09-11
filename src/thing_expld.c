/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_expld.c
 *     Support for creating and progressing explosions of 3D objects on map.
 * @par Purpose:
 *     Implements creation and physics of 3D faces remaining from objects.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 05 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_expld.h"

#include <stdlib.h>
#include "bfutility.h"

#include "enginfexpl.h"
#include "enginprops.h"
#include "enginshrapn.h"
#include "engindrwlstx.h"
#include "enginsngobjs.h"

#include "bigmap.h"
#include "bmbang.h"
#include "scanner.h"
#include "sound.h"
#include "swlog.h"
#include "thing.h"
#include "thing_fire.h"

/******************************************************************************/
#pragma pack(1)

struct rectangle { // sizeof=4
    ubyte x1;
    ubyte y1;
    ubyte x2;
    ubyte y2;
};

#pragma pack()

u32 dont_bother_with_explode_faces = false;
TbBool ex_face_anim_enabled = true;

extern struct rectangle redo_scanner[128];

extern s32 minimum_explode_depth;
extern u32 minimum_explode_and;
extern s32 minimum_explode_size;

extern s32 dword_1AA5C4;
extern s32 dword_1AA5C8;

/******************************************************************************/

void init_object_explode_faces(void)
{
    init_explode_faces();
    dont_bother_with_explode_faces = 1;
}

/** Creates triangural explode face, filled with given point coords and properties.
 *
 * Coord and angle deltas (move data) need to be set separately, as are not initialized here.
 */
ushort create_explode_face_tri(struct SortMapPoint *p_face_pt0,
  struct SortMapPoint *p_face_pt1, struct SortMapPoint *p_face_pt2,
  ushort txtr, ushort flags, ushort excol)
{
    struct ExplodeFace *p_exface;
    struct SortMapPoint cent;
    ushort eface;

    eface = explode_face_alloc();
    if (eface == 0) {
        return 0;
    }

    p_exface = &ex_faces[eface];

    p_exface->Type = 5;
    p_exface->Texture = txtr;
    p_exface->Flags = flags;
    p_exface->Col = (ubyte)excol;

    cent.X = (p_face_pt0->X + p_face_pt1->X + p_face_pt2->X) / 3;
    cent.Y = (p_face_pt0->Y + p_face_pt1->Y + p_face_pt2->Y) / 3;
    cent.Z = (p_face_pt0->Z + p_face_pt1->Z + p_face_pt2->Z) / 3;

    p_exface->X0 = p_face_pt0->X - cent.X;
    p_exface->Y0 = p_face_pt0->Y - cent.Y;
    p_exface->Z0 = p_face_pt0->Z - cent.Z;
    p_exface->X1 = p_face_pt1->X - cent.X;
    p_exface->Y1 = p_face_pt1->Y - cent.Y;
    p_exface->Z1 = p_face_pt1->Z - cent.Z;
    p_exface->X2 = p_face_pt2->X - cent.X;
    p_exface->Y2 = p_face_pt2->Y - cent.Y;
    p_exface->Z2 = p_face_pt2->Z - cent.Z;

    p_exface->X = cent.X;
    p_exface->Y = cent.Y;
    p_exface->Z = cent.Z;

    return eface;
}

/** Creates quad explode face, filled with given point coords and properties.
 *
 * Coord and angle deltas (move data) need to be set separately, as are not initialized here.
 */
ushort create_explode_face_quad(struct SortMapPoint *p_face_pt0,
  struct SortMapPoint *p_face_pt1, struct SortMapPoint *p_face_pt2,
  struct SortMapPoint *p_face_pt3, ushort txtr, ushort flags, ushort excol)
{
    struct ExplodeFace *p_exface;
    struct SortMapPoint cent;
    ushort eface;

    eface = explode_face_alloc();
    if (eface == 0) {
        return 0;
    }

    cent.X = (p_face_pt0->X + p_face_pt1->X + p_face_pt2->X + p_face_pt3->X) / 4;
    cent.Y = (p_face_pt0->Y + p_face_pt1->Y + p_face_pt2->Y + p_face_pt3->Y) / 4;
    cent.Z = (p_face_pt0->Z + p_face_pt1->Z + p_face_pt2->Z + p_face_pt3->Z) / 4;

    p_exface = &ex_faces[eface];

    p_exface->Type = 6;
    p_exface->Texture = txtr;
    p_exface->Flags = flags;
    p_exface->Col = (ubyte)excol;

    p_exface->X0 = p_face_pt0->X - cent.X;
    p_exface->Y0 = p_face_pt0->Y - cent.Y;
    p_exface->Z0 = p_face_pt0->Z - cent.Z;
    p_exface->X1 = p_face_pt1->X - cent.X;
    p_exface->Y1 = p_face_pt1->Y - cent.Y;
    p_exface->Z1 = p_face_pt1->Z - cent.Z;
    p_exface->X2 = p_face_pt2->X - cent.X;
    p_exface->Y2 = p_face_pt2->Y - cent.Y;
    p_exface->Z2 = p_face_pt2->Z - cent.Z;
    p_exface->X3 = p_face_pt3->X - cent.X;
    p_exface->Y3 = p_face_pt3->Y - cent.Y;
    p_exface->Z3 = p_face_pt3->Z - cent.Z;

    p_exface->X = cent.X;
    p_exface->Y = cent.Y;
    p_exface->Z = cent.Z;

    return eface;
}

/** Creates triangural explode face, filled as a slice of given face.
 */
ushort create_explode_face_tri_by_div(struct SortMapPoint *p_face_pt0,
  struct SortMapPoint *p_face_pt1, struct SortMapPoint *p_face_pt2,
  struct ExplodeFace *p_exface)
{
    struct ExplodeFace *p_neface;
    ushort eface;

    eface = explode_face_alloc();
    if (eface == 0) {
        return 0;
    }

    p_neface = &ex_faces[eface];
    p_neface->Type = 3;
    p_neface->Texture = p_exface->Texture;
    p_neface->Flags = p_exface->Flags;
    p_neface->Col = p_exface->Col;

    p_neface->X0 = p_face_pt0->X;
    p_neface->Y0 = p_face_pt0->Y;
    p_neface->Z0 = p_face_pt0->Z;
    p_neface->X1 = p_face_pt1->X;
    p_neface->Y1 = p_face_pt1->Y;
    p_neface->Z1 = p_face_pt1->Z;
    p_neface->X2 = p_face_pt2->X;
    p_neface->Y2 = p_face_pt2->Y;
    p_neface->Z2 = p_face_pt2->Z;

    p_neface->DX = p_exface->DX;
    p_neface->DY = p_exface->DY;
    p_neface->DZ = p_exface->DZ;
    p_neface->Timer = 1;

    return eface;
}

void explode_face_tri_divide_face(struct ExplodeFace *p_exface)
{
    struct SortMapPoint avg_pt0, avg_pt1, avg_pt2, efac_pt;

    avg_pt0.X = (p_exface->X1 + p_exface->X0) >> 1;
    avg_pt0.Y = (p_exface->Y1 + p_exface->Y0) >> 1;
    avg_pt0.Z = (p_exface->Z1 + p_exface->Z0) >> 1;
    avg_pt1.X = (p_exface->X2 + p_exface->X1) >> 1;
    avg_pt1.Y = (p_exface->Y2 + p_exface->Y1) >> 1;
    avg_pt1.Z = (p_exface->Z2 + p_exface->Z1) >> 1;
    avg_pt2.X = (p_exface->X2 + p_exface->X0) >> 1;
    avg_pt2.Z = (p_exface->Z0 + p_exface->Z2) >> 1;
    avg_pt2.Y = (p_exface->Y0 + p_exface->Y2) >> 1;

    efac_pt.X = p_exface->X0;
    efac_pt.Y = p_exface->Y0;
    efac_pt.Z = p_exface->Z0;

    create_explode_face_tri_by_div(&efac_pt, &avg_pt0, &avg_pt2, p_exface);

    efac_pt.X = p_exface->X1;
    efac_pt.Y = p_exface->Y1;
    efac_pt.Z = p_exface->Z1;

    create_explode_face_tri_by_div(&avg_pt0, &efac_pt, &avg_pt1, p_exface);

    efac_pt.X = p_exface->X2;
    efac_pt.Y = p_exface->Y2;
    efac_pt.Z = p_exface->Z2;

    create_explode_face_tri_by_div(&avg_pt1, &efac_pt, &avg_pt2, p_exface);

    create_explode_face_tri_by_div(&avg_pt0, &avg_pt1, &avg_pt2, p_exface);
}

/** Creates quad explode face, filled as a slice of given face.
 */
ushort create_explode_face_quad_by_div(struct SortMapPoint *p_face_pt0,
  struct SortMapPoint *p_face_pt1, struct SortMapPoint *p_face_pt2,
  struct SortMapPoint *p_face_pt3, struct ExplodeFace *p_exface)
{
    struct ExplodeFace *p_neface;
    ushort eface;

    eface = explode_face_alloc();
    if (eface == 0) {
        return 0;
    }

    p_neface = &ex_faces[eface];
    p_neface->Type = 4;
    p_neface->Texture = p_exface->Texture;
    p_neface->Flags = p_exface->Flags;
    p_neface->Col = p_exface->Col;

    p_neface->X0 = p_face_pt0->X;
    p_neface->Y0 = p_face_pt0->Y;
    p_neface->Z0 = p_face_pt0->Z;
    p_neface->X1 = p_face_pt1->X;
    p_neface->Y1 = p_face_pt1->Y;
    p_neface->Z1 = p_face_pt1->Z;
    p_neface->X2 = p_face_pt2->X;
    p_neface->Y2 = p_face_pt2->Y;
    p_neface->Z2 = p_face_pt2->Z;
    p_neface->X3 = p_face_pt3->X;
    p_neface->Y3 = p_face_pt3->Y;
    p_neface->Z3 = p_face_pt3->Z;

    p_neface->DX = p_exface->DX;
    p_neface->DY = p_exface->DY;
    p_neface->DZ = p_exface->DZ;
    p_neface->Timer = 1;

    return eface;
}

void explode_face_quad_divide_face(struct ExplodeFace *p_exface)
{
    struct SortMapPoint avg_pt0, avg_pt1, avg_pt2, avg_pt3, avg_pt4, efac_pt;

    avg_pt0.Y = (p_exface->Y1 + p_exface->Y0) >> 1;
    avg_pt0.Z = (p_exface->Z1 + p_exface->Z0) >> 1;
    avg_pt1.X = (p_exface->X3 + p_exface->X1) >> 1;
    avg_pt1.Y = (p_exface->Y3 + p_exface->Y1) >> 1;
    avg_pt1.Z = (p_exface->Z3 + p_exface->Z1) >> 1;
    avg_pt2.X = (p_exface->X2 + p_exface->X3) >> 1;
    avg_pt2.Y = (p_exface->Y2 + p_exface->Y3) >> 1;
    avg_pt2.Z = (p_exface->Z2 + p_exface->Z3) >> 1;
    avg_pt3.X = (p_exface->X0 + p_exface->X2) >> 1;
    avg_pt0.X = (p_exface->X0 + p_exface->X1) >> 1;
    avg_pt3.Z = (p_exface->Z0 + p_exface->Z2) >> 1;
    avg_pt3.Y = (p_exface->Y2 + p_exface->Y0) >> 1;
    avg_pt4.X = (avg_pt3.X + avg_pt2.X + avg_pt1.X + avg_pt0.X) >> 2;
    avg_pt4.Y = (avg_pt3.Y + avg_pt2.Y + avg_pt1.Y + avg_pt0.Y) >> 2;
    avg_pt4.Z = (avg_pt3.Z + avg_pt2.Z + avg_pt1.Z + avg_pt0.Z) >> 2;

    efac_pt.X = p_exface->X0;
    efac_pt.Y = p_exface->Y0;
    efac_pt.Z = p_exface->Z0;

    create_explode_face_quad_by_div(&efac_pt, &avg_pt0, &avg_pt3, &avg_pt4, p_exface);

    efac_pt.X = p_exface->X1;
    efac_pt.Y = p_exface->Y1;
    efac_pt.Z = p_exface->Z1;

    create_explode_face_quad_by_div(&avg_pt0, &efac_pt, &avg_pt4, &avg_pt1, p_exface);

    efac_pt.X = p_exface->X3;
    efac_pt.Y = p_exface->Y3;
    efac_pt.Z = p_exface->Z3;

    create_explode_face_quad_by_div(&avg_pt4, &avg_pt1, &avg_pt2, &efac_pt, p_exface);

    efac_pt.X = p_exface->X2;
    efac_pt.Y = p_exface->Y2;
    efac_pt.Z = p_exface->Z2;

    create_explode_face_quad_by_div(&avg_pt3, &avg_pt4, &efac_pt, &avg_pt2, p_exface);
}

static void explode_face3_move_above_ground(struct ExplodeFace *p_exface, ushort npoints)
{
    int rndv;
    {
        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y0 > rndv)
        {
            p_exface->X0 -= p_exface->DX;
            p_exface->Z0 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y0 += p_exface->DY - rndv;
    }
    {
        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y1 > rndv)
        {
            p_exface->X1 -= p_exface->DX;
            p_exface->Z1 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y1 += p_exface->DY - rndv;
    }
    {
        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y2 > rndv)
        {
            p_exface->X2 -= p_exface->DX;
            p_exface->Z2 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 0x7;
        p_exface->Y2 += p_exface->DY - rndv;
    }
    if (npoints >= 4)
    {
        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y3 > rndv)
        {
            p_exface->X3 -= p_exface->DX;
            p_exface->Z3 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y3 += p_exface->DY - rndv;
    }
    if (p_exface->DY > -120)
        p_exface->DY -= 3;
}

static void animate_explode_face1(ushort exface, ushort npoints)
{
    struct ExplodeFace *p_exface;

    p_exface = &ex_faces[exface];

    set_explode_face_rotate_angle(4 * ((2 * exface) & 0xF) + 40, 4 * (exface & 7) + 20);
    p_exface->Timer--;
    if (p_exface->Timer == 0)
    {
        explode_face_delete(exface);
        bang_new4(p_exface->X << 8, p_exface->Y, p_exface->Z << 8, 35);
        return;
    }
    p_exface->X += p_exface->DX;
    p_exface->Y += p_exface->DY;
    p_exface->Z += p_exface->DZ;

    if ((p_exface->X < 0) || (p_exface->Z < 0))
    {
        p_exface->Timer = 0;
        explode_face_delete(exface);
        return;
    }
    // bounce from ground
    if (p_exface->DY < 0
      && ((p_exface->Y + p_exface->Y0 < 0)
       || (p_exface->Y + p_exface->Y1 < 0)
       || (p_exface->Y + p_exface->Y2 < 0)
       || ((npoints >= 4) && (p_exface->Y + p_exface->Y3 < 0))))
    {
        p_exface->Y -= p_exface->DY;
        p_exface->DY = -((200 * p_exface->DY) >> 8);
        if (p_exface->Timer < 0)
            p_exface->Timer = 10;
    }
    p_exface->DY -= 10;

    explode_face_point_rotate(&p_exface->X0, &p_exface->Y0, &p_exface->Z0);
    explode_face_point_rotate(&p_exface->X1, &p_exface->Y1, &p_exface->Z1);
    explode_face_point_rotate(&p_exface->X2, &p_exface->Y2, &p_exface->Z2);
    if (npoints >= 4) {
        explode_face_point_rotate(&p_exface->X3, &p_exface->Y3, &p_exface->Z3);
    }
}

static void explode_face3_tri_final_ground_hit(int exface, int cor_gnd_y)
{
    struct ExplodeFace *p_exface;
    int cor_x, cor_z;
    int rndv;

    p_exface = &ex_faces[exface];

    rndv = LbRandomAnyShort() & 0x3F;
    cor_z = (p_exface->Z0 + rndv - 31) << 8;
    rndv = LbRandomAnyShort() & 0x3F;
    cor_x = (p_exface->X0 + rndv - 31) << 8;
    bang_new4(cor_x, 32 * cor_gnd_y, cor_z, 65);
}

static void explode_face3_quad_final_ground_hit(int exface, int cor_gnd_y)
{
    struct ExplodeFace *p_exface;
    int cor_x, cor_z;
    int base_x, base_z;
    short tile_x, tile_z;
    int rndv;

    p_exface = &ex_faces[exface];

    rndv = LbRandomAnyShort() & 0x1FF;
    base_x = rndv + p_exface->X0 - 255;
    rndv = LbRandomAnyShort() & 0x1FF;
    base_z = p_exface->Z0 + rndv - 255;
    bang_new4(base_x << 8, 32 * cor_gnd_y, base_z << 8, 65);

    cor_x = base_x - 16 * p_exface->DX;
    cor_z = base_z - 16 * p_exface->DZ;
    tile_x = cor_x >> 8;
    tile_z = cor_z >> 8;
    if (tile_x >= 0 && tile_x < 128)
    {
      if (tile_z >= 0 && tile_z < 128)
      {
          if ((minimum_explode_and & LbRandomAnyShort()) == 0)
          {
              quick_crater(tile_x, tile_z, minimum_explode_depth);
              bang_new4(cor_x << 8, 32 * cor_gnd_y, cor_z << 8, 20);
              if ((LbRandomAnyShort() & 7) == 0)
              {
                  FIRE_new(cor_x << 8, cor_gnd_y, cor_z << 8, 3u);
              }
          }
      }
    }
}

static void animate_explode_face3_tri(int exface)
{
    struct ExplodeFace *p_exface;
    int cor_gnd_y;
    int dist_x, dist_y, dist_z;

    p_exface = &ex_faces[exface];

    cor_gnd_y = alt_at_point(p_exface->X0, p_exface->Z0) >> 5;
    if ((p_exface->Y0 >= cor_gnd_y) &&
      (p_exface->Y1 >= cor_gnd_y) &&
      (p_exface->Y2 >= cor_gnd_y))
    {
        explode_face3_move_above_ground(p_exface, 3);
        return;
    }

    dist_x = abs(p_exface->X2 - p_exface->X0) + abs(p_exface->X1 - p_exface->X0);
    dist_y = abs(p_exface->Y2 - p_exface->Y0) + abs(p_exface->Y1 - p_exface->Y0);
    dist_z = abs(p_exface->Z2 - p_exface->Z0) + abs(p_exface->Z1 - p_exface->Z0);
    if ((dist_y + dist_x + dist_z) < 400)
    {
        explode_face3_tri_final_ground_hit(exface, cor_gnd_y);
    }
    else
    {
        explode_face_tri_divide_face(p_exface);
    }

    p_exface->Timer = 0;
    explode_face_delete(exface);
}

static void animate_explode_face3_quad(ushort exface)
{
    struct ExplodeFace *p_exface;
    int cor_gnd_y;
    int dist_x, dist_y, dist_z;

    p_exface = &ex_faces[exface];

    cor_gnd_y = alt_at_point(p_exface->X0, p_exface->Z0) >> 5;
    if ((p_exface->Y0 >= cor_gnd_y) &&
      (p_exface->Y1 >= cor_gnd_y) &&
      (p_exface->Y2 >= cor_gnd_y) &&
      (p_exface->Y3 >= cor_gnd_y))
    {
        explode_face3_move_above_ground(p_exface, 4);
        return;
    }

    dist_x = abs(p_exface->X2 - p_exface->X0) + abs(p_exface->X1 - p_exface->X0);
    dist_y = abs(p_exface->Y2 - p_exface->Y0) + abs(p_exface->Y1 - p_exface->Y0);
    dist_z = abs(p_exface->Z2 - p_exface->Z0) + abs(p_exface->Z1 - p_exface->Z0);
    if ((dist_y + dist_x + dist_z) < minimum_explode_size)
    {
        explode_face3_quad_final_ground_hit(exface, cor_gnd_y);
    }
    else
    {
        explode_face_quad_divide_face(p_exface);
    }

    p_exface->Timer = 0;
    explode_face_delete(exface);
}

static void animate_explode_face5(ushort exface, ushort npoints)
{
    struct ExplodeFace *p_exface;
    int rndv;

    p_exface = &ex_faces[exface];

    if (p_exface->Timer > 1000)
    {
        p_exface->Timer--;
        if (p_exface->Timer == 1000 && (LbRandomAnyShort() & 0x1F) == 0)
            bang_new4(p_exface->X << 8, p_exface->Y, p_exface->Z << 8, 100);
        return;
    }
    rndv = LbRandomAnyShort() & 0x3FF;
    if ((rndv > p_exface->Timer) && (LbRandomAnyShort() & 0xF) == 0)
    {
        p_exface->Timer = 0;
        bang_new4(p_exface->X << 8, p_exface->Y, p_exface->Z << 8, 35);
        explode_face_delete(exface);
        return;
    }
    set_explode_face_rotate_angle(p_exface->AngleDX, p_exface->AngleDY);
    p_exface->Timer--;
    p_exface->X += 8 * p_exface->DX;
    p_exface->Y += 8 * p_exface->DY;
    p_exface->Z += 8 * p_exface->DZ;
    if ((p_exface->X < 0) || (p_exface->Z < 0))
    {
        p_exface->Timer = 0;
        explode_face_delete(exface);
        return;
    }
    p_exface->DX -= (p_exface->DX >> 5);
    p_exface->DY -= (p_exface->DY >> 5);
    p_exface->DZ -= (p_exface->DZ >> 5);

    explode_face_point_rotate(&p_exface->X0, &p_exface->Y0, &p_exface->Z0);
    explode_face_point_rotate(&p_exface->X1, &p_exface->Y1, &p_exface->Z1);
    explode_face_point_rotate(&p_exface->X2, &p_exface->Y2, &p_exface->Z2);
    if (npoints >= 4) {
        explode_face_point_rotate(&p_exface->X3, &p_exface->Y3, &p_exface->Z3);
    }
}

void animate_explode(void)
{
#if 0
    asm volatile ("call ASM_animate_explode\n"
        :  :  : "eax" );
    return;
#endif
    struct ExplodeFace *p_exface;
    int i;
    int remain;

    if (dont_bother_with_explode_faces)
    {
        while (dword_1AA5C8 != dword_1AA5C4)
        {
            struct rectangle *p_rct;
            p_rct = &redo_scanner[dword_1AA5C8];
            SCANNER_fill_in_a_little_bit(p_rct->x1, p_rct->y1, p_rct->x2, p_rct->y2);
            dword_1AA5C8 = (dword_1AA5C8 + 1) & 0x7F;
        }
        return;
    }

    remain = 0;

    for (i = 1; i < EXPLODE_FACES_COUNT; i++)
    {
        p_exface = &ex_faces[i];
        if (p_exface->Timer == 0) {
            continue;
        }
        ++remain;

        switch (p_exface->Type)
        {
        case 1:
            animate_explode_face1(i, 3);
            break;

        case 2:
            animate_explode_face1(i, 4);
            break;

        case 3:
            animate_explode_face3_tri(i);
            break;

        case 4:
            animate_explode_face3_quad(i);
            break;

        case 5:
            animate_explode_face5(i, 3);
            break;

        case 6:
            animate_explode_face5(i, 4);
            break;
        }
    }
    if (remain == 0)
        dont_bother_with_explode_faces = 1;
}

void process_explode(void)
{
    if (ex_face_anim_enabled != 0)
        animate_explode();
}

void explode_face_setup_move_from_epicenter(ushort eface, struct SortMapPoint *p_obj_cor)
{
    struct ExplodeFace *p_exface;
    int dist_x, dist_z;
    int rndv;

    p_exface = &ex_faces[eface];

    rndv = LbRandomAnyShort();
    p_exface->AngleDX = (rndv & 0xF) - 7;
    rndv = LbRandomAnyShort();
    p_exface->AngleDY = (rndv & 0xF) - 7;

    p_exface->DX = (p_exface->X - p_obj_cor->X) >> 5;
    rndv = LbRandomAnyShort();
    p_exface->DY = (1 - (rndv & 7)) >> 3;
    p_exface->DZ = (p_exface->Z - p_obj_cor->Z) >> 5;

    dist_x = (expl_unkn_cor_x - p_exface->X) >> 8;
    dist_z = (expl_unkn_cor_z - p_exface->Z) >> 8;
    p_exface->Timer = ((dist_x * dist_x + dist_z * dist_z) >> 3) + 1002;
}

void explode_face_setup_move_random(ushort eface)
{
    struct ExplodeFace *p_exface;
    int dist_x, dist_z;
    int rndv;

    p_exface = &ex_faces[eface];

    rndv = LbRandomAnyShort();
    p_exface->AngleDX = (rndv & 0xF) - 7;
    rndv = LbRandomAnyShort();
    p_exface->AngleDY = (rndv & 0xF) - 7;

    rndv = LbRandomAnyShort();
    p_exface->DX = (rndv & 7) - 3;
    rndv = LbRandomAnyShort();
    p_exface->DY = (rndv & 7) - 3;
    rndv = LbRandomAnyShort();
    p_exface->DZ = (rndv & 7) - 3;

    dist_x = (expl_unkn_cor_x - p_exface->X) >> 8;
    dist_z = (expl_unkn_cor_z - p_exface->Z) >> 8;
    p_exface->Timer = ((dist_x * dist_x + dist_z * dist_z) >> 3) + 1002;
}

void object_explode_faces(short obj)
{
    struct SortMapPoint face_pt0, face_pt1, face_pt2, face_pt3;
    struct SortMapPoint obj_cor;
    struct SingleObject *p_gobj;
    struct SinglePoint *p_pt0, *p_pt1, *p_pt2, *p_pt3;
    int k;
    ushort eface;

    p_gobj = &game_objects[obj];
    obj_cor.X = p_gobj->MapX;
    obj_cor.Y = p_gobj->OffsetY;
    obj_cor.Z = p_gobj->MapZ;

    for (k = 0; k < p_gobj->NumbFaces; k++)
    {
        struct SingleObjectFace3 *p_face3;

        p_face3 = &game_object_faces3[p_gobj->StartFace + k];

        p_pt0 = &game_object_points[p_face3->PointNo[0]];
        face_pt0.X = obj_cor.X + p_pt0->X;
        face_pt0.Y = obj_cor.Y + p_pt0->Y;
        face_pt0.Z = obj_cor.Z + p_pt0->Z;

        p_pt1 = &game_object_points[p_face3->PointNo[1]];
        face_pt1.X = obj_cor.X + p_pt1->X;
        face_pt1.Y = obj_cor.Y + p_pt1->Y;
        face_pt1.Z = obj_cor.Z + p_pt1->Z;

        p_pt2 = &game_object_points[p_face3->PointNo[2]];
        face_pt2.X = obj_cor.X + p_pt2->X;
        face_pt2.Y = obj_cor.Y + p_pt2->Y;
        face_pt2.Z = obj_cor.Z + p_pt2->Z;

        eface = create_explode_face_tri(&face_pt0, &face_pt1, &face_pt2,
          p_face3->Texture, p_face3->Flags, p_face3->ExCol);

        if (eface == 0)
            continue;

        explode_face_setup_move_from_epicenter(eface, &obj_cor);
    }

    for (k = 0; k < p_gobj->NumbFaces4; k++)
    {
        struct SingleObjectFace4 *p_face4;

        p_face4 = &game_object_faces4[p_gobj->StartFace4 + k];

        p_pt0 = &game_object_points[p_face4->PointNo[0]];
        face_pt0.X = obj_cor.X + p_pt0->X;
        face_pt0.Y = obj_cor.Y + p_pt0->Y;
        face_pt0.Z = obj_cor.Z + p_pt0->Z;

        p_pt1 = &game_object_points[p_face4->PointNo[1]];
        face_pt1.X = obj_cor.X + p_pt1->X;
        face_pt1.Y = obj_cor.Y + p_pt1->Y;
        face_pt1.Z = obj_cor.Z + p_pt1->Z;

        p_pt2 = &game_object_points[p_face4->PointNo[2]];
        face_pt2.X = obj_cor.X + p_pt2->X;
        face_pt2.Y = obj_cor.Y + p_pt2->Y;
        face_pt2.Z = obj_cor.Z + p_pt2->Z;

        p_pt3 = &game_object_points[p_face4->PointNo[3]];
        face_pt3.X = obj_cor.X + p_pt3->X;
        face_pt3.Y = obj_cor.Y + p_pt3->Y;
        face_pt3.Z = obj_cor.Z + p_pt3->Z;

        eface = create_explode_face_quad(&face_pt0, &face_pt1, &face_pt2, &face_pt3,
          p_face4->Texture, p_face4->Flags, p_face4->ExCol);

        if (eface == 0)
            continue;

        explode_face_setup_move_from_epicenter(eface, &obj_cor);
    }
}

void thing_explode_faces(struct Thing *p_thing)
{
#if 0
    asm volatile ("call ASM_thing_explode_faces\n"
        : : "a" (p_thing));
    return;
#endif
    int i;
    short obj;

    dont_bother_with_explode_faces = 0;

    obj = p_thing->U.UObject.Object;
    for (i = 0; i < p_thing->U.UObject.NumbObjects; i++, obj++)
    {
        object_explode_faces(obj);
    }
}

void floor_explode_faces(short tile_x, short tile_z)
{
#if 0
    asm volatile ("call ASM_floor_explode_faces\n"
        : : "a" (tile_x), "d" (tile_z));
    return;
#endif
    struct SortMapPoint face_pt0, face_pt1, face_pt2, face_pt3;
    struct MyMapElement *p_mapel;
    ushort eface;

    dont_bother_with_explode_faces = 0;

    if ((tile_x < 0) || (tile_x >= MAP_TILE_WIDTH))
        return;
    if ((tile_z < 0) || (tile_z >= MAP_TILE_HEIGHT))
        return;

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
    if ((p_mapel->Flags & MEF1_Unkn80) != 0)
        return;

    face_pt0.X = TILE_TO_MAPCOORD(tile_x, 0);
    face_pt0.Z = TILE_TO_MAPCOORD(tile_z, 0);
    face_pt0.Y = alt_at_point(face_pt0.X, face_pt0.Z);

    face_pt3.X = TILE_TO_MAPCOORD(tile_x + 1, 0);
    face_pt3.Z = TILE_TO_MAPCOORD(tile_z + 1, 0);;
    face_pt3.Y = alt_at_point(face_pt3.X, face_pt3.Z);

    face_pt1.X = face_pt3.X;
    face_pt1.Z = face_pt0.Z;
    face_pt1.Y = alt_at_point(face_pt1.X, face_pt1.Z);

    face_pt2.X = face_pt0.X;
    face_pt2.Z = face_pt3.Z;
    face_pt2.Y = alt_at_point(face_pt2.X, face_pt2.Z);

    eface = create_explode_face_quad(&face_pt0, &face_pt1, &face_pt2, &face_pt3,
      p_mapel->Texture & 0x3FFF, 0x04 | 0x02, 0);

    explode_face_setup_move_random(eface);
}

void draw_explode(void)
{
    ushort exface;

    if (dont_bother_with_explode_faces)
        return;

    for (exface = 1; exface < EXPLODE_FACES_COUNT; exface++)
    {
        struct ExplodeFace *p_exface;

        p_exface = &ex_faces[exface];

        if (next_screen_point + 4 > screen_points_limit)
            break;

        if (p_exface->Timer == 0)
            continue;

        switch (p_exface->Type)
        {
        case 1:
            enlist_draw_explode_type1(exface, 3);
            break;
        case 2:
            enlist_draw_explode_type1(exface, 4);
            break;
        case 3:
            enlist_draw_explode_type3(exface, 3);
            break;
        case 4:
            enlist_draw_explode_type3(exface, 4);
            break;
        case 5:
            enlist_draw_explode_type5(exface, 3);
            break;
        case 6:
            enlist_draw_explode_type5(exface, 4);
            break;
        case 0:
        default:
            break;
        }
    }
}

/******************************************************************************/
