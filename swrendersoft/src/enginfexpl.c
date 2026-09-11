/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginfexpl.c
 *     Make 3D objects explode into a cloud of faces.
 * @par Purpose:
 *     Implement functions for handling object explosions and debrit movement.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     13 Oct 2024 - 06 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginfexpl.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include "bfmemut.h"
#include "bfutility.h"

#include "enginbckt.h"
#include "engincam.h"
#include "engindrwlstx.h"
#include "enginprops.h"
#include "enginshrapn.h"
#include "engintrns.h"
#include "frame_sprani.h"
#include "privrdlog.h"
/******************************************************************************/
struct ExplodeFace ex_faces[EXPLODE_FACES_COUNT];
ushort free_ex_face = 1;

u32 dont_bother_with_explode_faces = false;

s32 dword_1AA5D8 = 0;
s32 dword_1AA5DC = 0;
s32 dword_1AA5E0 = 0;
s32 dword_1AA5E4 = 0;

ushort word_1E08B8 = 0;
s32 dword_1E08BC = 0;

/******************************************************************************/

void init_free_explode_faces(void)
{
    int i;

    LbMemorySet(ex_faces, 0, sizeof(ex_faces));
    free_ex_face = 1;
    for (i = 1; i < EXPLODE_FACES_COUNT - 1; i++)
    {
        ex_faces[i].Timer = 0;
        ex_faces[i].Flags = i + 1;
    }
    ex_faces[i].Timer = 0;
    ex_faces[i].Flags = 0;

    dont_bother_with_explode_faces = 1;
}

void FIRE_init(void)
{
#if 0
    asm volatile ("call ASM_FIRE_init\n"
        :  :  : "eax" );
#endif
    int i;

    LbMemorySet(FIRE_flame, 0, sizeof(FIRE_flame));

    for (i = 1; i < 511; i++) {
        FIRE_flame[i].next = i + 1;
    }
    FIRE_flame[i].next = 0;

    word_1E08B8 = 1;
    dword_1E08BC = 0;
}

ushort FIRE_spawn_flame(ushort cor_x, ushort cor_y, ushort cor_z, ushort rangemsk,
  ushort fbig, ushort ftype, ushort count)
{
    struct FireFlame *p_fflame;
    ushort fflame;
    ushort anim;
    ushort flife;
    short flame_x, flame_z;
    ubyte flame_life;
    sbyte flame_fvel;

    switch (ftype)
    {
    case 21:
    case 10:
        anim = 923;
        flife = 53;
        break;
    case 9:
    case 8:
        anim = 924;
        flife = 53;
        break;
    case 7:
    case 6:
        anim = 923;
        flife = 43;
        break;
    case 5:
        anim = 924;
        flife = 53;
        break;
    case 4:
        anim = 923;
        flife = 43;
        break;
    default:
        assert(!"bad flame type");
        break;
    }

    flame_life = flife + (LbRandomAnyShort() & 0xF);
    flame_z = cor_z + (LbRandomAnyShort() & rangemsk) - (rangemsk >> 1);
    flame_x = cor_x + (LbRandomAnyShort() & rangemsk) - (rangemsk >> 1);
    flame_fvel = (LbRandomAnyShort() & 0x3F) + 50;
    if (word_1E08B8 != 0)
    {
        ushort nxflame;

        fflame = word_1E08B8;
        nxflame = FIRE_flame[fflame].next;
        ++dword_1E08BC;
        word_1E08B8 = nxflame;
    }
    else
    {
        fflame = 0;
    }

    if (fflame != 0)
    {
        ushort frm;

        p_fflame = &FIRE_flame[fflame];
        p_fflame->x = flame_x;
        p_fflame->z = flame_z;
        p_fflame->y = cor_y;
        p_fflame->type = ftype;
        p_fflame->big = fbig;
        p_fflame->dbig = 0;
        p_fflame->ddbig = -1;
        for (frm = nstart_ani[anim]; ; frm = frame[frm].Next)
        {
            p_fflame->frame = frm;
            if ((LbRandomAnyShort() & 3) == 0)
                break;
        }
        p_fflame->life = flame_life;
        p_fflame->count = count;
        p_fflame->fvel = flame_fvel;
        p_fflame->fcount = LbRandomAnyShort() & 0x7F;
    }
    return fflame;
}

void explode_face_delete(int exface)
{
    ex_faces[exface].Flags = free_ex_face;
    ex_faces[exface].Timer = 0;
    free_ex_face = exface;
}

void set_explode_face_rotate_angle(ushort angX, ushort angY)
{
    dword_1AA5D8 = lbSinTable[(angX) & LbFPMath_AngleMask];
    dword_1AA5DC = lbSinTable[(angX + LbFPMath_PI/2) & LbFPMath_AngleMask];
    dword_1AA5E0 = lbSinTable[(angY) & LbFPMath_AngleMask];
    dword_1AA5E4 = lbSinTable[(angY + LbFPMath_PI/2) & LbFPMath_AngleMask];
}

void explode_face_point_rotate(short *p_cor_x, short *p_cor_y, short *p_cor_z)
{
    int dist;
    int cor_x, cor_y, cor_z;

    cor_x = *p_cor_x;
    cor_y = *p_cor_y;
    cor_z = *p_cor_z;
    dist = (dword_1AA5D8 * cor_x + dword_1AA5DC * cor_z) >> 16;
    *p_cor_x = (dword_1AA5DC * cor_x - dword_1AA5D8 * cor_z) >> 16;
    *p_cor_y = (dword_1AA5E4 * cor_y - dist * dword_1AA5E0) >> 16;
    *p_cor_z = (dword_1AA5E4 * dist + dword_1AA5E0 * cor_y) >> 16;
}

ushort create_explode_face_tri(struct SortMapPoint *p_face_pt0,
  struct SortMapPoint *p_face_pt1, struct SortMapPoint *p_face_pt2,
  ushort txtr, ushort flags, ushort excol)
{
    struct ExplodeFace *p_exface;
    struct SortMapPoint cent;
    ushort eface;

    eface = free_ex_face;
    if (eface != 0) {
        free_ex_face = ex_faces[eface].Flags;
    }

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

ushort create_explode_face_quad(struct SortMapPoint *p_face_pt0,
  struct SortMapPoint *p_face_pt1, struct SortMapPoint *p_face_pt2,
  struct SortMapPoint *p_face_pt3, ushort txtr, ushort flags, ushort excol)
{
    struct ExplodeFace *p_exface;
    struct SortMapPoint cent;
    ushort eface;

    eface = free_ex_face;
    if (eface != 0) {
        free_ex_face = ex_faces[eface].Flags;
    }

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

    eface = free_ex_face;
    if (eface != 0) {
        free_ex_face = ex_faces[eface].Flags;
    }
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

    eface = free_ex_face;
    if (eface != 0) {
        free_ex_face = ex_faces[eface].Flags;
    }
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

void draw_explode_type1(ushort exface, ushort npoints)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct ExplodeFace *p_exface;
    struct SpecialPoint *p_specpt;
    int cor_dx, cor_dy, cor_dz;
    ushort flags_all;
    short depth_max;
    ushort pt;

    p_exface = &ex_faces[exface];

    pt = next_screen_point;
    p_exface->PointOffset = pt;
    next_screen_point += npoints;

    cor_dx = p_exface->X + p_exface->X0 - engn_xc;
    cor_dz = p_exface->Z + p_exface->Z0 - engn_zc;
    cor_dy = p_exface->Y + p_exface->Y0 - engn_yc;
    transform_shpoint(&sp1, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    cor_dx = p_exface->X + p_exface->X1 - engn_xc;
    cor_dz = p_exface->Z + p_exface->Z1 - engn_zc;
    cor_dy = p_exface->Y + p_exface->Y1 - engn_yc;
    transform_shpoint(&sp2, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    cor_dx = p_exface->X + p_exface->X2 - engn_xc;
    cor_dz = p_exface->Z + p_exface->Z2 - engn_zc;
    cor_dy = p_exface->Y + p_exface->Y2 - engn_yc;
    transform_shpoint(&sp3, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 2];
    p_specpt->X = sp3.X;
    p_specpt->Y = sp3.Y;

    depth_max = SHRT_MIN;
    if (depth_max < sp1.Depth)
        depth_max = sp1.Depth;
    if (depth_max < sp2.Depth)
        depth_max = sp2.Depth;
    if (depth_max < sp3.Depth)
        depth_max = sp3.Depth;

    flags_all = sp3.Flags & sp2.Flags & sp1.Flags;

    if (npoints >= 4)
    {
        cor_dx = p_exface->X + p_exface->X3 - engn_xc;
        cor_dz = p_exface->Z + p_exface->Z3 - engn_zc;
        cor_dy = p_exface->Y + p_exface->Y3 - engn_yc;
        transform_shpoint(&sp4, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        p_specpt = &game_screen_point_pool[pt + 3];
        p_specpt->X = sp4.X;
        p_specpt->Y = sp4.Y;

        flags_all &= sp4.Flags;
        if (depth_max < sp4.Depth)
            depth_max = sp4.Depth;
    }

    if ((flags_all & 0xF) != 0)
        return;

    stat_drawlist_faces++;
    draw_item_add(DrIT_Unkn5, exface, BUCKET_MID + depth_max);
}

void draw_explode_type3(ushort exface, ushort npoints)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct ExplodeFace *p_exface;
    struct SpecialPoint *p_specpt;
    int cor_dx, cor_dy, cor_dz;
    ushort flags_all;
    short depth_max;
    ushort pt;

    p_exface = &ex_faces[exface];

    pt = next_screen_point;
    p_exface->PointOffset = pt;
    next_screen_point += npoints;

    cor_dx = p_exface->X0 - engn_xc;
    cor_dz = p_exface->Z0 - engn_zc;
    cor_dy = p_exface->Y0 - engn_yc;
    transform_shpoint(&sp1, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    cor_dx = p_exface->X1 - engn_xc;
    cor_dz = p_exface->Z1 - engn_zc;
    cor_dy = p_exface->Y1 - engn_yc;
    transform_shpoint(&sp2, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    cor_dx = p_exface->X2 - engn_xc;
    cor_dz = p_exface->Z2 - engn_zc;
    cor_dy = p_exface->Y2 - engn_yc;
    transform_shpoint(&sp3, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 2];
    p_specpt->X = sp3.X;
    p_specpt->Y = sp3.Y;

    depth_max = SHRT_MIN;
    if (depth_max < sp1.Depth)
        depth_max = sp1.Depth;
    if (depth_max < sp2.Depth)
        depth_max = sp2.Depth;
    if (depth_max < sp3.Depth)
        depth_max = sp3.Depth;

    flags_all = sp3.Flags & sp2.Flags & sp1.Flags;

    if (npoints >= 4)
    {
        cor_dx = p_exface->X3 - engn_xc;
        cor_dz = p_exface->Z3 - engn_zc;
        cor_dy = p_exface->Y3 - engn_yc;
        transform_shpoint(&sp4, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        p_specpt = &game_screen_point_pool[pt + 3];
        p_specpt->X = sp4.X;
        p_specpt->Y = sp4.Y;

        flags_all &= sp4.Flags;
        if (depth_max < sp4.Depth)
            depth_max = sp4.Depth;
    }

    if ((flags_all & 0xF) != 0)
        return;

    stat_drawlist_faces++;
    draw_item_add(DrIT_Unkn5, exface, BUCKET_MID + depth_max);
}

void draw_explode_type5(ushort exface, ushort npoints)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct ExplodeFace *p_exface;
    struct SpecialPoint *p_specpt;
    int cor_dx, cor_dy, cor_dz;
    ushort flags_all;
    short depth_max;
    ushort pt;

    p_exface = &ex_faces[exface];

    pt = next_screen_point;
    p_exface->PointOffset = pt;
    next_screen_point += npoints;

    cor_dx = p_exface->X + p_exface->X0 - engn_xc;
    cor_dz = p_exface->Z + p_exface->Z0 - engn_zc;
    cor_dy = p_exface->Y + p_exface->Y0 - engn_yc;
    transform_shpoint(&sp1, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    cor_dx = p_exface->X + p_exface->X1 - engn_xc;
    cor_dz = p_exface->Z + p_exface->Z1 - engn_zc;
    cor_dy = p_exface->Y + p_exface->Y1 - engn_yc;
    transform_shpoint(&sp2, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    cor_dx = p_exface->X + p_exface->X2 - engn_xc;
    cor_dz = p_exface->Z + p_exface->Z2 - engn_zc;
    cor_dy = p_exface->Y + p_exface->Y2 - engn_yc;
    transform_shpoint(&sp3, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    p_specpt = &game_screen_point_pool[pt + 2];
    p_specpt->X = sp3.X;
    p_specpt->Y = sp3.Y;

    depth_max = SHRT_MIN;
    if (depth_max < sp1.Depth)
        depth_max = sp1.Depth;
    if (depth_max < sp2.Depth)
        depth_max = sp2.Depth;
    if (depth_max < sp3.Depth)
        depth_max = sp3.Depth;

    flags_all = sp3.Flags & sp2.Flags & sp1.Flags;

    if (npoints >= 4)
    {
        cor_dx = p_exface->X + p_exface->X3 - engn_xc;
        cor_dz = p_exface->Z + p_exface->Z3 - engn_zc;
        cor_dy = p_exface->Y + p_exface->Y3 - engn_yc;
        transform_shpoint(&sp4, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        p_specpt = &game_screen_point_pool[pt + 3];
        p_specpt->X = sp4.X;
        p_specpt->Y = sp4.Y;

        flags_all &= sp4.Flags;
        if (depth_max < sp4.Depth)
            depth_max = sp4.Depth;
    }

    if ((flags_all & 0xF) != 0)
        return;

    stat_drawlist_faces++;
    draw_item_add(DrIT_Unkn5, exface, BUCKET_MID + depth_max);
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
            draw_explode_type1(exface, 3);
            break;
        case 2:
            draw_explode_type1(exface, 4);
            break;
        case 3:
            draw_explode_type3(exface, 3);
            break;
        case 4:
            draw_explode_type3(exface, 4);
            break;
        case 5:
            draw_explode_type5(exface, 3);
            break;
        case 6:
            draw_explode_type5(exface, 4);
            break;
        case 0:
        default:
            break;
        }
    }
}

/******************************************************************************/
