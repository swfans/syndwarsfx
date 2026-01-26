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

#include "bigmap.h"
#include "bmbang.h"
#include "enginbckt.h"
#include "engincam.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "enginprops.h"
#include "enginshrapn.h"
#include "engintrns.h"
#include "frame_sprani.h"
#include "scanner.h"
#include "sound.h"
#include "swlog.h"
#include "thing.h"
/******************************************************************************/
#pragma pack(1)

struct rectangle { // sizeof=4
    ubyte x1;
    ubyte y1;
    ubyte x2;
    ubyte y2;
};

#pragma pack()

u32 next_ex_face = 1;

extern ushort word_1AA5CC;
extern struct rectangle redo_scanner[128];

extern s32 minimum_explode_depth;
extern u32 minimum_explode_and;
extern s32 minimum_explode_size;

extern s32 dword_1AA5C4;
extern s32 dword_1AA5C8;
extern s32 dword_1AA5D8;
extern s32 dword_1AA5DC;
extern s32 dword_1AA5E0;
extern s32 dword_1AA5E4;

extern ushort word_1E08B8;
extern s32 dword_1E08BC;

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

void FIRE_add_flame(ThingIdx firetng, ushort fflame)
{
    struct SimpleThing *p_fire;
    struct FireFlame *p_fflame;

    if (fflame == 0) {
        return;
    }
    p_fire = &sthings[firetng];
    p_fflame = &FIRE_flame[fflame];

    p_fflame->next = p_fire->U.UFire.flame;
    p_fire->U.UFire.flame = fflame;
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

void FIRE_new(int x, int y, int z, ubyte type)
{
#if 0
    asm volatile ("call ASM_FIRE_new\n"
        : : "a" (x), "d" (y), "b" (z), "c" (type));
#endif
    struct SimpleThing *p_fire;
    struct MyMapElement *p_mapel;
    int cor_x, cor_z;
    int cor_y;
    ThingIdx firetng;
    ushort fflame;
    short tile_x, tile_z;
    short sib_tl_x, sib_tl_z;
    ubyte flame_count;

    if ((PRCCOORD_TO_MAPCOORD(x) >= MAP_COORD_WIDTH)
      || (PRCCOORD_TO_MAPCOORD(z) >= MAP_COORD_HEIGHT)) {
        return;
    }
    if (word_1E08B8 == 0) {
        return;
    }

    firetng = get_new_sthing();
    if (firetng == 0) {
        return;
    }

    p_fire = &sthings[firetng];
    p_fire->Type = SmTT_FIRE;
    p_fire->U.UFire.flame = 0;
    p_fire->Z = z;
    p_fire->Y = y;
    p_fire->X = x;
    play_dist_ssample(p_fire, 0x10u, 0x7Fu, 0x40u, 100, -1, 1);

    cor_x = x >> 8;
    cor_z = z >> 8;
    cor_y = y;

    switch (type)
    {
    case 1u:
        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 18 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 4, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 24 + (LbRandomAnyShort() & 0xF);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 5, flame_count);
        FIRE_add_flame(firetng, fflame);
        break;
    case 2u:
        flame_count = 2;
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 30, 10, flame_count);
        FIRE_add_flame(firetng, fflame);
        break;
    case 3u:
        tile_z = cor_z >> 8;
        tile_x = cor_x >> 8;
        p_mapel = &game_my_big_map[128 * (z >> 16) + (cor_x >> 8)];
        p_mapel->Flags2 &= ~0x80;

        sib_tl_z = tile_z - 1;
        sib_tl_x = tile_x - 1;
        if ( sib_tl_x <= 0x7F && sib_tl_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * sib_tl_z + sib_tl_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0x70;
        }
        sib_tl_z = tile_z - 1;
        if ( tile_x <= 0x7F && sib_tl_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * sib_tl_z + tile_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0x50;
        }
        sib_tl_x = tile_x + 1;
        sib_tl_z = tile_z - 1;
        if ( (tile_x + 1) >= 0 && sib_tl_x <= 127 && sib_tl_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * sib_tl_z + sib_tl_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0x30;
        }
        sib_tl_x = tile_x + 1;
        if ( (tile_x + 1) >= 0 && sib_tl_x <= 127 && tile_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * tile_z + sib_tl_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0x10;
        }
        sib_tl_z = tile_z + 1;
        sib_tl_x = tile_x + 1;
        if ( (tile_x + 1) >= 0 && sib_tl_x <= 127 && sib_tl_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * sib_tl_z + sib_tl_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0xF0;
        }
        sib_tl_z = tile_z + 1;
        if ( tile_x <= 0x7F && sib_tl_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * sib_tl_z + tile_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0xD0;
        }
        sib_tl_z = tile_z + 1;
        sib_tl_x = tile_x - 1;
        if ( (tile_x - 1) >= 0 && sib_tl_x <= 127 && sib_tl_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[128 * sib_tl_z + sib_tl_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0xB0;
        }
        sib_tl_x = tile_x - 1;
        if ( sib_tl_x <= 0x7F && tile_z <= 0x7F )
        {
          p_mapel = &game_my_big_map[(tile_z << 7) + sib_tl_x];
          p_mapel->Flags2 &= 0xF;
          p_mapel->Flags2 |= 0x90;
        }

        flame_count = 43 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 6, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 43 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 6, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 43 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 100, 6, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 43 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 7, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 43 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 7, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 43 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 30, 7, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 53 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 8, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 53 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 8, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 53 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0xFF, 100, 8, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 53 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 9, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 53 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 9, flame_count);
        FIRE_add_flame(firetng, fflame);

        flame_count = 53 - (LbRandomAnyShort() & 3);
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x1FF, 30, 9, flame_count);
        FIRE_add_flame(firetng, fflame);
        break;
    case 4u:
        flame_count = 6;
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 30, 21, flame_count);
        FIRE_add_flame(firetng, fflame);
        goto LABEL_184;
    case 5u:
LABEL_184:
        flame_count = 12;
        fflame = FIRE_spawn_flame(cor_x, cor_y, cor_z, 0x3F, 30, 21, flame_count);
        FIRE_add_flame(firetng, fflame);
        break;
    default:
        break;
    }
    add_node_sthing(firetng);
}

static void explode_face_delete(int exface)
{
    ex_faces[exface].Flags = word_1AA5CC;
    ex_faces[exface].Timer = 0;
    word_1AA5CC = exface;
}

static void set_explode_face_rotate_angle(ushort angX, ushort angY)
{
    dword_1AA5D8 = lbSinTable[(angX) & LbFPMath_AngleMask];
    dword_1AA5DC = lbSinTable[(angX + LbFPMath_PI/2) & LbFPMath_AngleMask];
    dword_1AA5E0 = lbSinTable[(angY) & LbFPMath_AngleMask];
    dword_1AA5E4 = lbSinTable[(angY + LbFPMath_PI/2) & LbFPMath_AngleMask];
}

static void explode_face_point_rotate(short *p_cor_x, short *p_cor_y, short *p_cor_z)
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

static void animate_explode_face1_tri(int exface)
{
    struct ExplodeFace3 *p_exface;

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
       || (p_exface->Y + p_exface->Y2 < 0)))
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
}

static void animate_explode_face1_quad(int exface)
{
    struct ExplodeFace3 *p_exface;

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
       || (p_exface->Y + p_exface->Y3 < 0)))
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
    explode_face_point_rotate(&p_exface->X3, &p_exface->Y3, &p_exface->Z3);
}

static void animate_explode_face3_tri(int exface)
{
    struct ExplodeFace3 *p_exface;
    struct ExplodeFace3 *p_neface;
    int rndv;
    int cor_x, cor_y, cor_z;
    int dist_x, dist_y, dist_z;
    int avg_x0, avg_y0, avg_z0;
    int avg_x1, avg_y1, avg_z1;
    int avg_x2, avg_y2, avg_z2;
    int eface;

    p_exface = &ex_faces[exface];

    cor_y = alt_at_point(p_exface->X0, p_exface->Z0) >> 5;
    if (p_exface->Y0 >= cor_y && p_exface->Y1 >= cor_y && p_exface->Y2 >= cor_y)
    {
        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y0 > rndv)
        {
            p_exface->X0 -= p_exface->DX;
            p_exface->Z0 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y0 += p_exface->DY - rndv;

        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y1 > rndv)
        {
            p_exface->X1 -= p_exface->DX;
            p_exface->Z1 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y1 += p_exface->DY - rndv;

        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y2 > rndv)
        {
            p_exface->X2 -= p_exface->DX;
            p_exface->Z2 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 0x7;
        p_exface->Y2 += p_exface->DY - rndv;

        if (p_exface->DY > -120)
            p_exface->DY -= 3;
        return;
    }

    dist_x = abs(p_exface->X2 - p_exface->X0) + abs(p_exface->X1 - p_exface->X0);
    dist_y = abs(p_exface->Y2 - p_exface->Y0) + abs(p_exface->Y1 - p_exface->Y0);
    dist_z = abs(p_exface->Z2 - p_exface->Z0) + abs(p_exface->Z1 - p_exface->Z0);
    if ((dist_y + dist_x + dist_z) < 400)
    {
        p_exface->Timer = 0;
        explode_face_delete(exface);

        rndv = LbRandomAnyShort() & 0x3F;
        cor_z = (p_exface->Z0 + rndv - 31) << 8;
        rndv = LbRandomAnyShort() & 0x3F;
        cor_x = (p_exface->X0 + rndv - 31) << 8;
        bang_new4(cor_x, 32 * cor_y, cor_z, 65);
        return;
    }

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
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
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
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
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
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
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
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
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
    p_exface->Timer = 0;
    explode_face_delete(exface);
}

static void animate_explode_face3_quad(int exface)
{
    struct ExplodeFace3 *p_exface;
    struct ExplodeFace3 *p_neface;
    int rndv;
    int cor_x, cor_y, cor_z;
    int dist_x, dist_y, dist_z;
    int avg_x0, avg_y0, avg_z0;
    int avg_x1, avg_y1, avg_z1;
    int avg_x2, avg_y2, avg_z2;
    int avg_x3, avg_y3, avg_z3;
    int avg_x4, avg_y4, avg_z4;
    int eface;

    p_exface = &ex_faces[exface];

    cor_y = alt_at_point(p_exface->X0, p_exface->Z0) >> 5;
    if (p_exface->Y0 >= cor_y
        && p_exface->Y1 >= cor_y
        && p_exface->Y2 >= cor_y
        && p_exface->Y3 >= cor_y)
    {
        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y0 > rndv)
        {
            p_exface->X0 -= p_exface->DX;
            p_exface->Z0 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y0 += p_exface->DY - rndv;

        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y1 > rndv)
        {
            p_exface->X1 -= p_exface->DX;
            p_exface->Z1 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y1 += p_exface->DY - rndv;

        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y2 > rndv)
        {
            p_exface->X2 -= p_exface->DX;
            p_exface->Z2 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 0x7;
        p_exface->Y2 += p_exface->DY - rndv;

        rndv = LbRandomAnyShort() & 0x3FF;
        if (p_exface->Y3 > rndv)
        {
            p_exface->X3 -= p_exface->DX;
            p_exface->Z3 -= p_exface->DZ;
        }
        rndv = LbRandomAnyShort() & 7;
        p_exface->Y3 += p_exface->DY - rndv;
        if (p_exface->DY > -120)
            p_exface->DY -= 3;
        return;
    }
    dist_x = abs(p_exface->X2 - p_exface->X0) + abs(p_exface->X1 - p_exface->X0);
    dist_y = abs(p_exface->Y2 - p_exface->Y0) + abs(p_exface->Y1 - p_exface->Y0);
    dist_z = abs(p_exface->Z2 - p_exface->Z0) + abs(p_exface->Z1 - p_exface->Z0);
    if ((dist_y + dist_x + dist_z) < minimum_explode_size)
    {
        int base_x, base_z;
        int tile_x, tile_z;

        p_exface->Timer = 0;
        explode_face_delete(exface);

        rndv = LbRandomAnyShort() & 0x1FF;
        base_x = rndv + p_exface->X0 - 255;
        rndv = LbRandomAnyShort() & 0x1FF;
        base_z = p_exface->Z0 + rndv - 255;
        bang_new4(base_x << 8, 32 * cor_y, base_z << 8, 65);

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
                  bang_new4(cor_x << 8, 32 * cor_y, cor_z << 8, 20);
                  if ((LbRandomAnyShort() & 7) == 0)
                  {
                      FIRE_new(cor_x << 8, cor_y, cor_z << 8, 3u);
                  }
              }
          }
        }
        return;
    }
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
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
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
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
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
    {
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
    }
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
    {
        word_1AA5CC = ex_faces[word_1AA5CC].Flags;
    }
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
    p_exface->Timer = 0;
    explode_face_delete(exface);
}

static void animate_explode_face5_tri(int exface)
{
    struct ExplodeFace3 *p_exface;
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
}

static void animate_explode_face5_quad(int exface)
{
    struct ExplodeFace3 *p_exface;
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
    explode_face_point_rotate(&p_exface->X3, &p_exface->Y3, &p_exface->Z3);
}

void animate_explode(void)
{
#if 0
    asm volatile ("call ASM_animate_explode\n"
        :  :  : "eax" );
    return;
#endif
    struct ExplodeFace3 *p_exface;
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
            animate_explode_face1_tri(i);
            break;

        case 2:
            animate_explode_face1_quad(i);
            break;

        case 3:
            animate_explode_face3_tri(i);
            break;

        case 4:
            animate_explode_face3_quad(i);
            break;

        case 5:
            animate_explode_face5_tri(i);
            break;

        case 6:
            animate_explode_face5_quad(i);
            break;
        }
    }
    if (remain == 0)
        dont_bother_with_explode_faces = 1;
}


void draw_explode(void)
{
    ushort exface;

    if (next_ex_face != 0)
        animate_explode();

    if (dont_bother_with_explode_faces)
        return;

    for (exface = 1; exface < EXPLODE_FACES_COUNT; exface++)
    {
        struct ShEnginePoint sp1, sp2, sp3, sp4;
        struct ExplodeFace3 *p_exface;
        struct SpecialPoint *p_specpt;
        int cor_x, cor_y, cor_z;
        ushort npoints;
        ushort flags_all;
        short depth_max;
        ushort pt;

        p_exface = &ex_faces[exface];

        if (next_screen_point + 4 > screen_points_limit)
            break;

        if (p_exface->Timer == 0)
            continue;

        switch (p_exface->Type)
        {
        case 1:
        case 2:
            npoints = (p_exface->Type == 2) ? 4 : 3;
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

            if (npoints > 3)
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
                break;

            dword_176D68++;
            draw_item_add(DrIT_Unkn5, exface, BUCKET_MID + depth_max);
            break;
        case 3:
        case 4:
            npoints = (p_exface->Type == 4) ? 4 : 3;
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

            if (npoints > 3)
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
                break;

            dword_176D68++;
            draw_item_add(DrIT_Unkn5, exface, BUCKET_MID + depth_max);
            break;
        case 5:
        case 6:
            npoints = (p_exface->Type == 6) ? 4 : 3;
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

            if (npoints > 3)
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
                break;

            dword_176D68++;
            draw_item_add(DrIT_Unkn5, exface, BUCKET_MID + depth_max);
            break;
        case 0:
        default:
            break;
        }
    }
}

/******************************************************************************/
