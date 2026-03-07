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
struct ExplodeFace3 ex_faces[EXPLODE_FACES_COUNT];
u32 next_ex_face = 1;

u32 dont_bother_with_explode_faces = false;

ushort word_1AA5CC = 1;

s32 dword_1AA5D8 = 0;
s32 dword_1AA5DC = 0;
s32 dword_1AA5E0 = 0;
s32 dword_1AA5E4 = 0;

ushort word_1E08B8 = 0;
s32 dword_1E08BC = 0;

/******************************************************************************/

void init_free_explode_faces(void)
{
#if 0
    asm volatile ("call ASM_init_free_explode_faces\n"
        :  :  : "eax" );
#else
    int i;

    LbMemorySet(ex_faces, 0, sizeof(ex_faces));
    word_1AA5CC = 1;
    for (i = 1; i < EXPLODE_FACES_COUNT - 1; i++)
    {
        ex_faces[i].Timer = 0;
        ex_faces[i].Flags = i + 1;
    }
    ex_faces[i].Timer = 0;
    ex_faces[i].Flags = 0;

    dont_bother_with_explode_faces = 1;
#endif
}

ushort FIRE_spawn_flame(ushort cor_x, ushort cor_y, ushort cor_z, ushort rangemsk, ushort fbig, ushort ftype, ushort count)
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
    ex_faces[exface].Flags = word_1AA5CC;
    ex_faces[exface].Timer = 0;
    word_1AA5CC = exface;
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

void explode_face3_tri_divide_face(struct ExplodeFace3 *p_exface)
{
    struct ExplodeFace3 *p_neface;
    int avg_x0, avg_y0, avg_z0;
    int avg_x1, avg_y1, avg_z1;
    int avg_x2, avg_y2, avg_z2;
    int eface;

    avg_y0 = (p_exface->Y1 + p_exface->Y0) >> 1;
    avg_z0 = (p_exface->Z1 + p_exface->Z0) >> 1;
    avg_x1 = (p_exface->X2 + p_exface->X1) >> 1;
    avg_y1 = (p_exface->Y2 + p_exface->Y1) >> 1;
    avg_z1 = (p_exface->Z2 + p_exface->Z1) >> 1;
    avg_x2 = (p_exface->X2 + p_exface->X0) >> 1;
    avg_z2 = (p_exface->Z0 + p_exface->Z2) >> 1;
    avg_x0 = (p_exface->X1 + p_exface->X0) >> 1;
    avg_y2 = (p_exface->Y0 + p_exface->Y2) >> 1;

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 3;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = p_exface->X0;
        p_neface->Y0 = p_exface->Y0;
        p_neface->Z0 = p_exface->Z0;
        p_neface->X1 = avg_x0;
        p_neface->Y2 = avg_y2;
        p_neface->Y1 = avg_y0;
        p_neface->Z1 = avg_z0;
        p_neface->X2 = avg_x2;
        p_neface->Z2 = avg_z2;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 3;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = avg_x0;
        p_neface->Y0 = avg_y0;
        p_neface->Z0 = avg_z0;
        p_neface->X1 = p_exface->X1;
        p_neface->Y1 = p_exface->Y1;
        p_neface->Z1 = p_exface->Z1;
        p_neface->X2 = avg_x1;
        p_neface->Y2 = avg_y1;
        p_neface->Z2 = avg_z1;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 3;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = avg_x1;
        p_neface->Y0 = avg_y1;
        p_neface->Z0 = avg_z1;
        p_neface->X1 = p_exface->X2;
        p_neface->Y1 = p_exface->Y2;
        p_neface->Z1 = p_exface->Z2;
        p_neface->Y2 = avg_y2;
        p_neface->X2 = avg_x2;
        p_neface->Z2 = avg_z2;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 3;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = avg_x0;
        p_neface->Y2 = avg_y2;
        p_neface->Y0 = avg_y0;
        p_neface->Z2 = avg_z2;
        p_neface->Z0 = avg_z0;
        p_neface->X1 = avg_x1;
        p_neface->Y1 = avg_y1;
        p_neface->Z1 = avg_z1;
        p_neface->X2 = avg_x2;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }
}

void explode_face3_quad_divide_face(struct ExplodeFace3 *p_exface)
{
    struct ExplodeFace3 *p_neface;
    int avg_x0, avg_y0, avg_z0;
    int avg_x1, avg_y1, avg_z1;
    int avg_x2, avg_y2, avg_z2;
    int avg_x3, avg_y3, avg_z3;
    int avg_x4, avg_y4, avg_z4;
    int eface;

    avg_y0 = (p_exface->Y1 + p_exface->Y0) >> 1;
    avg_z0 = (p_exface->Z1 + p_exface->Z0) >> 1;
    avg_x1 = (p_exface->X3 + p_exface->X1) >> 1;
    avg_y1 = (p_exface->Y3 + p_exface->Y1) >> 1;
    avg_z1 = (p_exface->Z3 + p_exface->Z1) >> 1;
    avg_x2 = (p_exface->X2 + p_exface->X3) >> 1;
    avg_y2 = (p_exface->Y2 + p_exface->Y3) >> 1;
    avg_z2 = (p_exface->Z2 + p_exface->Z3) >> 1;
    avg_x3 = (p_exface->X0 + p_exface->X2) >> 1;
    avg_x0 = (p_exface->X0 + p_exface->X1) >> 1;
    avg_z3 = (p_exface->Z0 + p_exface->Z2) >> 1;
    avg_y3 = (p_exface->Y2 + p_exface->Y0) >> 1;
    avg_x4 = (avg_x3 + avg_x2 + avg_x0 + avg_x1) >> 2;
    avg_y4 = (avg_y3 + avg_y2 + avg_y1 + avg_y0) >> 2;
    avg_z4 = (avg_z3 + avg_z2 + avg_z1 + avg_z0) >> 2;

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 4;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = p_exface->X0;
        p_neface->Y0 = p_exface->Y0;
        p_neface->Z0 = p_exface->Z0;
        p_neface->X1 = avg_x0;
        p_neface->Y2 = avg_y3;
        p_neface->Y1 = avg_y0;
        p_neface->Z1 = avg_z0;
        p_neface->X2 = avg_x3;
        p_neface->Z2 = avg_z3;
        p_neface->X3 = avg_x4;
        p_neface->Y3 = avg_y4;
        p_neface->Z3 = avg_z4;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 4;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = avg_x0;
        p_neface->Y0 = avg_y0;
        p_neface->Z0 = avg_z0;
        p_neface->X1 = p_exface->X1;
        p_neface->Y1 = p_exface->Y1;
        p_neface->Z1 = p_exface->Z1;
        p_neface->X2 = avg_x4;
        p_neface->Y2 = avg_y4;
        p_neface->Z2 = avg_z4;
        p_neface->X3 = avg_x1;
        p_neface->Y3 = avg_y1;
        p_neface->Z3 = avg_z1;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 4;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->X0 = avg_x4;
        p_neface->Y0 = avg_y4;
        p_neface->Z0 = avg_z4;
        p_neface->X1 = avg_x1;
        p_neface->Y1 = avg_y1;
        p_neface->Z1 = avg_z1;
        p_neface->X2 = avg_x2;
        p_neface->Y2 = avg_y2;
        p_neface->Z2 = avg_z2;
        p_neface->X3 = p_exface->X3;
        p_neface->Y3 = p_exface->Y3;
        p_neface->Z3 = p_exface->Z3;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }

    eface = word_1AA5CC;
    if (eface != 0)
        word_1AA5CC = ex_faces[eface].Flags;
    if (eface != 0)
    {
        p_neface = &ex_faces[eface];
        p_neface->Type = 4;
        p_neface->Texture = p_exface->Texture;
        p_neface->Flags = p_exface->Flags;
        p_neface->Col = p_exface->Col;
        p_neface->Y0 = avg_y3;
        p_neface->X0 = avg_x3;
        p_neface->Z0 = avg_z3;
        p_neface->X1 = avg_x4;
        p_neface->Y1 = avg_y4;
        p_neface->Z1 = avg_z4;
        p_neface->X2 = p_exface->X2;
        p_neface->Y2 = p_exface->Y2;
        p_neface->Z2 = p_exface->Z2;
        p_neface->X3 = avg_x2;
        p_neface->Y3 = avg_y2;
        p_neface->Z3 = avg_z2;
        p_neface->DX = p_exface->DX;
        p_neface->DY = p_exface->DY;
        p_neface->DZ = p_exface->DZ;
        p_neface->Timer = 1;
    }
}

void draw_explode_type1(ushort exface, ushort npoints)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct ExplodeFace3 *p_exface;
    struct SpecialPoint *p_specpt;
    int cor_x, cor_y, cor_z;
    ushort flags_all;
    short depth_max;
    ushort pt;

    p_exface = &ex_faces[exface];

    pt = next_screen_point;
    p_exface->PointOffset = pt;
    next_screen_point += npoints;

    cor_x = p_exface->X + p_exface->X0 - engn_xc;
    cor_z = p_exface->Z + p_exface->Z0 - engn_zc;
    cor_y = p_exface->Y + p_exface->Y0 - engn_yc;
    transform_shpoint(&sp1, cor_x, cor_y - 8 * engn_yc, cor_z);

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    cor_x = p_exface->X + p_exface->X1 - engn_xc;
    cor_z = p_exface->Z + p_exface->Z1 - engn_zc;
    cor_y = p_exface->Y + p_exface->Y1 - engn_yc;
    transform_shpoint(&sp2, cor_x, cor_y - 8 * engn_yc, cor_z);

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    cor_x = p_exface->X + p_exface->X2 - engn_xc;
    cor_z = p_exface->Z + p_exface->Z2 - engn_zc;
    cor_y = p_exface->Y + p_exface->Y2 - engn_yc;
    transform_shpoint(&sp3, cor_x, cor_y - 8 * engn_yc, cor_z);

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
        cor_x = p_exface->X + p_exface->X3 - engn_xc;
        cor_z = p_exface->Z + p_exface->Z3 - engn_zc;
        cor_y = p_exface->Y + p_exface->Y3 - engn_yc;
        transform_shpoint(&sp4, cor_x, cor_y - 8 * engn_yc, cor_z);

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
    struct ExplodeFace3 *p_exface;
    struct SpecialPoint *p_specpt;
    int cor_x, cor_y, cor_z;
    ushort flags_all;
    short depth_max;
    ushort pt;

    p_exface = &ex_faces[exface];

    pt = next_screen_point;
    p_exface->PointOffset = pt;
    next_screen_point += npoints;

    cor_x = p_exface->X0 - engn_xc;
    cor_z = p_exface->Z0 - engn_zc;
    cor_y = p_exface->Y0 - engn_yc;
    transform_shpoint(&sp1, cor_x, cor_y - 8 * engn_yc, cor_z);

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    cor_x = p_exface->X1 - engn_xc;
    cor_z = p_exface->Z1 - engn_zc;
    cor_y = p_exface->Y1 - engn_yc;
    transform_shpoint(&sp2, cor_x, cor_y - 8 * engn_yc, cor_z);

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    cor_x = p_exface->X2 - engn_xc;
    cor_z = p_exface->Z2 - engn_zc;
    cor_y = p_exface->Y2 - engn_yc;
    transform_shpoint(&sp3, cor_x, cor_y - 8 * engn_yc, cor_z);

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
        cor_x = p_exface->X3 - engn_xc;
        cor_z = p_exface->Z3 - engn_zc;
        cor_y = p_exface->Y3 - engn_yc;
        transform_shpoint(&sp4, cor_x, cor_y - 8 * engn_yc, cor_z);

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
    struct ExplodeFace3 *p_exface;
    struct SpecialPoint *p_specpt;
    int cor_x, cor_y, cor_z;
    ushort flags_all;
    short depth_max;
    ushort pt;

    p_exface = &ex_faces[exface];

    pt = next_screen_point;
    p_exface->PointOffset = pt;
    next_screen_point += npoints;

    cor_x = p_exface->X + p_exface->X0 - engn_xc;
    cor_z = p_exface->Z + p_exface->Z0 - engn_zc;
    cor_y = p_exface->Y + p_exface->Y0 - engn_yc;
    transform_shpoint(&sp1, cor_x, cor_y - 8 * engn_yc, cor_z);

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    cor_x = p_exface->X + p_exface->X1 - engn_xc;
    cor_z = p_exface->Z + p_exface->Z1 - engn_zc;
    cor_y = p_exface->Y + p_exface->Y1 - engn_yc;
    transform_shpoint(&sp2, cor_x, cor_y - 8 * engn_yc, cor_z);

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    cor_x = p_exface->X + p_exface->X2 - engn_xc;
    cor_z = p_exface->Z + p_exface->Z2 - engn_zc;
    cor_y = p_exface->Y + p_exface->Y2 - engn_yc;
    transform_shpoint(&sp3, cor_x, cor_y - 8 * engn_yc, cor_z);

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
        cor_x = p_exface->X + p_exface->X3 - engn_xc;
        cor_z = p_exface->Z + p_exface->Z3 - engn_zc;
        cor_y = p_exface->Y + p_exface->Y3 - engn_yc;
        transform_shpoint(&sp4, cor_x, cor_y - 8 * engn_yc, cor_z);

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
        struct ExplodeFace3 *p_exface;

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
