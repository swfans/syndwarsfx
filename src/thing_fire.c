/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_fire.c
 *     Support for creating fires burning on the map.
 * @par Purpose:
 *     Implement creation and updating of the representation of fire.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Aug 2025 - 25 Aug 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_fire.h"

#include "bfutility.h"

#include "enginshrapn.h"

#include "bigmap.h"
#include "enginfexpl.h"
#include "sound.h"
#include "swlog.h"
#include "thing.h"

/******************************************************************************/

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

void set_static_on_fire(struct SimpleThing *p_static)
{
    asm volatile (
      "call ASM_set_static_on_fire\n"
        :  : "a" (p_static));
}

/******************************************************************************/
