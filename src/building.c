/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file building.c
 *     Buildings related functions.
 * @par Purpose:
 *     Implement functions for handling buildings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "building.h"

#include <assert.h>
#include "bfmath.h"
#include "bfmemory.h"
#include "bfutility.h"

#include "enginprops.h"
#include "enginsngobjs.h"

#include "ssampply.h"
#include "bigmap.h"
#include "bmbang.h"
#include "game_options.h"
#include "game_speed.h"
#include "game.h"
#include "matrix.h"
#include "sound.h"
#include "thing.h"
#include "tngcolisn.h"
#include "vehtraffic.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/
ubyte dome_open_speed = 4;

struct BuildingStat bldng_type_stats[] = {
  {"BLD_NONE",},
  {"BLD_1",},
  {"BLD_2",},
  {"BLD_3",},
  {"BLD_4",},
  {"BLD_5",},
  {"BLD_6",},
  {"BLD_7",},
  {"BLD_8",},
  {"BLD_9",},
  {"BLD_A",},
  {"BLD_B",},
  {"BLD_C",},
  {"BLD_D",},
  {"BLD_E",},
  {"BLD_F",},
  {"BLD_10",},
  {"BLD_11",},
  {"BLD_12",},
  {"BLD_13",},
  {"SHUTTL_LDR",},
  {"BEZIER_ROAD",},
  {"BLD_16",},
  {"BLD_17",},
  {"DOME",},
  {"BLD_19",},
  {"BLD_1A",},
  {"STANDRD BLD",},
  {"TRAIN_TRACK",},
  {"BLD_1D",},
  {"BLD_1E",},
  {"BLD_1F",},
  {"MOUNT_GUN",},
  {"BLD_21",},
  {"BLD_22",},
  {"BLD_23",},
  {"BLD_24",},
  {"BLD_GATE",},
  {"BLD_26",},
  {"BLD_27",},
  {"BLD_28",},
  {"BLD_29",},
  {"BLD_2A",},
  {"BLD_2B",},
  {"BLD_2C",},
  {"BLD_2D",},
  {"BLD_2E",},
  {"TRAIN_STATN",},
  {"BLD_30",},
  {"BLD_31",},
  {"BLD_32",},
  {"BLD_33",},
  {"BLD_34",},
  {"BLD_35",},
  {"MOVN_ROTOR",},
  {"BLD_37",},
  {"BLD_38",},
  {"BLD_39",},
  {"BLD_3A",},
  {"BLD_3B",},
  {"BLD_3C",},
  {"BILLBOARD",},
  {"BLD_3E",},
  {"BLD_3F",},
  {"BLD_40",},
  {"BLD_41",},
  {"BLD_42",},
  {"BLD_43",},
  {"BLD_44",},
};


const char *building_type_name(ushort btype)
{
    struct BuildingStat *p_bldstat;

    p_bldstat = &bldng_type_stats[btype];
    if (strlen(p_bldstat->Name) == 0)
        return "OUTRNG_BLDNG";
    return p_bldstat->Name;
}

TbBool building_can_transform_open(ThingIdx bldng)
{
    struct Thing *p_building;

    p_building = &things[bldng];
    if (p_building->Type != TT_BUILDING)
        return false;
    if ((p_building->Flag & TngF_Destroyed) != 0)
        return false;
    return (p_building->SubType == SubTT_BLD_DOME);
}

struct Thing *create_building_thing(int x, int y, int z, ushort obj, ushort nobj, ushort a6)
{
    struct Thing *ret;
    asm volatile (
      "push %6\n"
      "push %5\n"
      "call ASM_create_building_thing\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z), "c" (obj), "g" (nobj), "g" (a6));
    return ret;
}

void do_dome_rotate1(struct Thing *p_building)
{
#if 0
    asm volatile ("call ASM_do_dome_rotate1\n"
        : : "a" (p_building));
#endif
    ushort obj, i;
    ushort pt1_beg, pt1_num;

    obj = p_building->U.UObject.Object;
    pt1_beg = game_objects[obj].StartPoint;
    pt1_num = game_objects[obj].EndPoint - pt1_beg;
    for (i = obj + 1; i < obj + 8; i++)
    {
        int factrS, factrC;
        short angle;
        ushort pt2_beg;
        ushort dpt;

        angle = (i - obj) * p_building->SubState;
        pt2_beg = game_objects[i].StartPoint;
        factrC = lbSinTable[angle + LbFPMath_PI/2];
        factrS = lbSinTable[angle];
        for (dpt = 0; dpt <= pt1_num; dpt++)
        {
            struct SinglePoint *p_point1;
            struct SinglePoint *p_point2;
            p_point1 = &game_object_points[pt1_beg + dpt];
            p_point2 = &game_object_points[pt2_beg + dpt];
            p_point2->Y = (factrC * p_point1->Y - factrS * p_point1->Z) >> 16;
            p_point2->Z = (factrC * p_point1->Z + factrS * p_point1->Y) >> 16;
        }
    }
}

void process_dome1(struct Thing *p_building)
{
#if 0
    asm volatile ("call ASM_process_dome1\n"
        : : "a" (p_building));
#endif
    uint timer0;

    switch (p_building->State)
    {
    case BldSt_TRA_OPENING:
        timer0 = p_building->SubState - dome_open_speed;
        p_building->SubState = timer0;
        if (timer0 <= 127)
        {
            if ((timer0 >= 111u) && (timer0 < 111u + dome_open_speed)) {
                set_dome_col(p_building, 1);
            }
            do_dome_rotate1(p_building);
        }
        else
        {
            play_dist_sample(p_building, 47, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
            p_building->SubState = 0;
            p_building->Timer1 = 100;
            p_building->State = BldSt_TRA_OPENED;
        }
        break;
    case BldSt_TRA_CLOSING:
        timer0 = p_building->SubState + dome_open_speed;
        p_building->SubState = timer0;
        if (timer0 <= 127)
        {
            do_dome_rotate1(p_building);
        }
        else
        {
            play_dist_sample(p_building, 47, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
            p_building->SubState = 127;
            p_building->Timer1 = 100;
            p_building->State = BldSt_TRA_CLOSED;
        }
        break;
    case BldSt_TRA_OPENED:
        if ((p_building->Flag & TngF_TransCloseRq) != 0) {
            play_dist_sample(p_building, 100, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
            p_building->State = BldSt_TRA_CLOSING;
        }
        p_building->Flag &= ~(TngF_TransCloseRq|TngF_TransOpenRq);
        break;
    case BldSt_TRA_CLOSED:
        if ((p_building->Flag & TngF_TransOpenRq) != 0) {
            play_dist_sample(p_building, 100, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
            p_building->State = BldSt_TRA_OPENING;
        }
        p_building->Flag &= ~(TngF_TransCloseRq|TngF_TransOpenRq);
        break;
    default:
        p_building->SubState = 127;
        p_building->Timer1 = 100;
        p_building->State = BldSt_TRA_CLOSED;
        break;
    }
}

ushort explode_thing_building(short thing, short x, short y, short z)
{
    ushort ret;
    asm volatile (
      "call ASM_explode_thing_building\n"
        : "=r" (ret) : "a" (thing), "d" (x), "b" (y), "c" (z));
    return ret;
}

void collapse_building_process_tnodes(struct Thing *p_building)
{
    short tnode1, tnode2;
    int i;

    for (i = 0; i < 4; i++)
    {
        short tnode;
        tnode = p_building->U.UObject.tnode_50[i];
        if (tnode != 0) {
            struct TrafficNode *p_tnode;
            p_tnode = &game_traffic_nodes[tnode];
            p_tnode->Flags |= (0x2000|0x1000);
        }
    }

    tnode1 = p_building->U.UObject.tnode_50[0];
    tnode2 = p_building->U.UObject.tnode_50[1];
    if ((tnode1 != 0) && (tnode2 != 0))
    {
        struct TrafficNode *p_tnode;
        p_tnode = &game_traffic_nodes[tnode1];
        for (i = 0; i < 4; i++)
        {
            if (p_tnode->UTraffic.Link[i] == -tnode2) {
                break;
            }
            if (p_tnode->UTraffic.Link[i] == 0) {
                p_tnode->UTraffic.Link[i] = -tnode2;
                p_tnode->UTraffic.Flags[i] = 0x40;
                break;
            }
        }
    }

    tnode1 = p_building->U.UObject.tnode_50[2];
    tnode2 = p_building->U.UObject.tnode_50[3];
    if ((tnode1 != 0) && (tnode2 != 0))
    {
        struct TrafficNode *p_tnode;
        p_tnode = &game_traffic_nodes[tnode1];
        for (i = 0; i < 4; i++)
        {
            if (p_tnode->UTraffic.Link[i] == -tnode2) {
                break;
            }
            if (p_tnode->UTraffic.Link[i] == 0) {
                p_tnode->UTraffic.Link[i] = -tnode2;
                p_tnode->UTraffic.Flags[i] = 0x40;
                break;
            }
        }
    }

    tnode1 = p_building->U.UObject.tnode_50[0];
    tnode2 = p_building->U.UObject.tnode_50[3];
    if ((tnode1 != 0) && (tnode2 != 0))
    {
        struct TrafficNode *p_tnode;
        p_tnode = &game_traffic_nodes[tnode1];
        for (i = 0; i < 4; i++)
        {
            if (p_tnode->UTraffic.Link[i] == -tnode2) {
                p_tnode->UTraffic.Link[i] = 0;
                break;
            }
        }
    }

    tnode1 = p_building->U.UObject.tnode_50[2];
    tnode2 = p_building->U.UObject.tnode_50[1];
    if ((tnode1 != 0) && (tnode2 != 0))
    {
        struct TrafficNode *p_tnode;
        p_tnode = &game_traffic_nodes[tnode1];
        for (i = 0; i < 4; i++)
        {
            if (p_tnode->UTraffic.Link[i] == -tnode2) {
                p_tnode->UTraffic.Link[i] = 0;
                break;
            }
        }
    }
}

void collapse_building_shuttle_loader(short x, short y, short z, struct Thing *p_building)
{
    struct Thing *p_thing;
    short thing;

    p_thing = p_building;
    thing = p_building->ThingOffset;
    while ( 1 )
    {
        short pvthing;
        pvthing = thing;
        thing = p_thing->U.UObject.NextThing;
        if (thing == 0)
            break;
        if (thing <= 0)
        {
            if (p_building->SubType == SubTT_BLD_BEZIER_ROAD) {
                tnode_all_unlink_thing(pvthing);
            }
            break;
        }
        else
        {
            p_thing = &things[thing];
            if ((p_thing->Flag & TngF_Destroyed) != 0)
                break;
            p_thing->Flag |= TngF_Destroyed;
            explode_thing_building(thing, x, y, z);
            p_thing->Type = TT_UNKN55;
        }
    }

    p_thing = p_building;
    thing = p_building->ThingOffset;
    while ( 1 )
    {
        short nxthing;
        nxthing = thing;
        thing = p_thing->U.UObject.PrevThing;
        if (thing == 0)
            break;
        if (thing <= 0)
        {
            if (p_building->SubType == SubTT_BLD_BEZIER_ROAD) {
                tnode_all_unlink_thing(nxthing);
            }
            break;
        }
        else
        {
            p_thing = &things[thing];
            if ((p_thing->Flag & TngF_Destroyed) != 0)
                break;
            p_thing->Flag |= TngF_Destroyed;
            explode_thing_building(thing, x, y, z);
            p_thing->Type = TT_UNKN55;
        }
    }
}

void collapse_building_station(struct Thing *p_building)
{
    MapCoord cntr_cor_x, cntr_cor_z;
    short cntr_x, cntr_z;
    short x, z;

    get_thing_position_mapcoords(&cntr_cor_x, NULL, &cntr_cor_z, p_building->ThingOffset);
    cntr_x = MAPCOORD_TO_TILE(cntr_cor_x);
    cntr_z = MAPCOORD_TO_TILE(cntr_cor_z);

    for (x = cntr_x - 8; x <= cntr_x + 8; x++)
    {
        if ((x < 0) || (x > MAP_TILE_WIDTH-1))
            continue;
        for (z = cntr_z - 8; z <= cntr_z + 8; z++)
        {
            struct MyMapElement *p_mapel;
            short thing;

            if ((z < 0) || (z > MAP_TILE_HEIGHT-1))
                continue;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * z + x];
            thing = p_mapel->Child;
            while (thing != 0)
            {
                if (thing > 0)
                {
                    struct Thing *p_thing;
                    p_thing = &things[thing];
                    if ((p_thing->Type == TT_BUILDING)
                      && (p_thing->SubType == SubTT_BLD_TRAINTRK)
                      && ((p_thing->Flag & TngF_Destroyed) == 0))
                    {
                        collapse_building(TILE_TO_MAPCOORD(x,0), 0, TILE_TO_MAPCOORD(z,0), p_thing);
                        break; // do not expect more than one building on a tile
                    }
                    thing = p_thing->Next;
                }
                else
                {
                    struct SimpleThing *p_sthing;
                    p_sthing = &sthings[thing];
                    thing = p_sthing->Next;
                }
            }
        }
    }
}

void init_mgun_explode(struct Thing *p_thing)
{
    asm volatile (
      "call ASM_init_mgun_explode\n"
        :  : "a" (p_thing));
    return;
}

void collapse_building(short x, short y, short z, struct Thing *p_building)
{
#if 0
    asm volatile (
      "call ASM_collapse_building\n"
        :  : "a" (x), "d" (y), "b" (z), "c" (p_thing));
    return;
#endif
    struct SimpleThing *p_sthing;
    short thing;
    int i;

    if (ingame.SoundThing != 0)
    {
        p_sthing = &sthings[ingame.SoundThing];
        p_sthing->X = p_building->X;
        p_sthing->Y = p_building->Y;
        p_sthing->Z = p_building->Z;
        p_sthing->State = 1;
    }
    else
    {
        short tng_x, tng_y, tng_z;
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_building->ThingOffset);
        play_dist_sample(p_building, 45, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        p_sthing = create_sound_effect(tng_x, tng_y, tng_z, 46, FULL_VOL, -1);
        if (p_sthing != NULL)
        {
            p_sthing->State = 1;
            ingame.SoundThing = p_sthing->ThingOffset;
        }
    }
    thing = p_building->ThingOffset;

    collapse_building_process_tnodes(p_building);

    switch (p_building->SubType)
    {
    case SubTT_BLD_SHUTLDR:
    case SubTT_BLD_1D:
    case SubTT_BLD_TRAINTRK:
    case SubTT_BLD_BEZIER_ROAD:
        collapse_building_shuttle_loader(x, y, z, p_building);
        break;
    case SubTT_BLD_STATION:
        collapse_building_station(p_building);
        break;
    }

    if ((p_building->Flag & TngF_Destroyed) == 0)
    {
        struct SingleObject *p_sobj;

        p_sobj = &game_objects[p_building->U.UObject.Object];
        if (((p_sobj->field_1C & 0x0100) == 0) || ((render_floor_flags & RendFlrF_WobblyTerrain) != 0))
        {
            short cra_cor_x, cra_cor_z;

            get_thing_position_mapcoords(&cra_cor_x, NULL, &cra_cor_z, p_building->ThingOffset);
            quick_crater(MAPCOORD_TO_TILE(cra_cor_x), MAPCOORD_TO_TILE(cra_cor_z), 3);
            for (i = 0; i < 32; i++)
            {
                int dx, dz;
                int bang_x, bang_z;
                dz = (LbRandomAnyShort() & 0xFF) - 127;
                dx = (LbRandomAnyShort() & 0xFF) - 127;
                bang_z = ((z + dz) << 8);
                bang_x = ((x + dx) << 8);
                bang_new4(bang_x, 0, bang_z, 65);
            }
        }
        p_building->Flag |= TngF_Destroyed;
        explode_thing_building(thing, x, y, z);
        p_building->Type = TT_UNKN55;
    }
}

short create_a_pod(struct Thing *p_thing, struct Thing *p_loader, ushort pod_id)
{
    short ret;
    asm volatile (
      "call ASM_create_a_pod\n"
        : "=r" (ret) : "a" (p_thing), "d" (p_loader), "b" (pod_id));
    return ret;
}

void process_shuttle_loader(struct Thing *p_building)
{
    asm volatile ("call ASM_process_shuttle_loader\n"
        : : "a" (p_building));
}

void bul_hit_vector(int x, int y, int z, short col, int hp, int type)
{
    asm volatile (
      "push %5\n"
      "push %4\n"
      "call ASM_bul_hit_vector\n"
        : : "a" (x), "d" (y), "b" (z), "c" (col), "g" (hp), "g" (type));
}

void rotate_object(struct Thing *p_object)
{
    asm volatile ("call ASM_rotate_object\n"
        : : "a" (p_object));
}

int mounted_los(int x1, int y1, int z1, int x2, int y2, int z2)
{
    int ret;
    asm volatile (
      "push %6\n"
      "push %5\n"
      "call ASM_mounted_los\n"
        : "=r" (ret) : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2));
    return ret;
}

ThingIdx aquire_target(struct Thing *p_gun)
{
    ThingIdx ret;
    asm volatile (
      "call ASM_aquire_target\n"
        : "=r" (ret) : "a" (p_gun));
    return ret;
}

ubyte track_target(struct Thing *p_mgun)
{
    ubyte ret;
    asm volatile (
      "call ASM_track_target\n"
        : "=r" (ret) : "a" (p_mgun));
    return ret;
}

ubyte mgun_shoot_at_target(struct Thing *p_mgun)
{
    ubyte turn;

    if (p_mgun->U.UMGun.WeaponTurn != 0) {
        return 0;
    }
    turn = p_mgun->U.UMGun.ShotTurn;
    p_mgun->U.UMGun.WeaponTurn = 8;
    p_mgun->U.UMGun.ShotTurn = (turn == 0);
    switch (p_mgun->U.UMGun.CurrentWeapon)
    {
    default:
        LOGWARN("Mounted gun weapon %d not supported, using laser",
          (int)p_mgun->U.UMGun.CurrentWeapon);
        // fall through
    case WEP_LASER:
        init_mgun_laser(p_mgun, 7);
        break;
    }
    return 0;
}

void process_mounted_gun(struct Thing *p_building)
{
#if 0
    asm volatile ("call ASM_process_mounted_gun\n"
        : : "a" (p_building));
#endif
    struct Thing *p_target;

    if (p_building->U.UMGun.RecoilTimer > 0)
        p_building->U.UMGun.RecoilTimer--;
    if (p_building->PTarget == NULL)
        p_building->State = 10;

    switch(p_building->State)
    {
    case 10:
        if (((gameturn + p_building->ThingOffset) & 0x07) == 0 && aquire_target(p_building) > 0)
            p_building->State = 11;
        break;
    case 11:
        if (((gameturn + p_building->ThingOffset) & 0x1F) == 0 && aquire_target(p_building) == 0)
            p_building->State = 10;
        if (!track_target(p_building))
        {
            rotate_object(p_building);
            break;
        }
        p_target = p_building->PTarget;
        if (p_target != NULL) // Invalid pointer correction
        {
            if ((p_target <= &things[0]) || (p_target >= &things[THINGS_LIMIT])) {
                LOGERR("Tried to shoot at invalid target 0x%p", p_target);
                p_building->PTarget = p_target = NULL;
            }
        }
        if (p_target != NULL)
        {
            MapCoord tg_cor_x, tg_cor_y, tg_cor_z; // Target/victim coords
            MapCoord at_cor_x, at_cor_y, at_cor_z; // Attacker coords

            tg_cor_x = PRCCOORD_TO_MAPCOORD(p_target->X);
            tg_cor_y = PRCCOORD_TO_MAPCOORD(p_target->Y);
            tg_cor_z = PRCCOORD_TO_MAPCOORD(p_target->Z);
            if (p_target->Type == TT_PERSON)
                tg_cor_y += 20;
            at_cor_x = PRCCOORD_TO_MAPCOORD(p_building->X);
            at_cor_y = PRCCOORD_TO_MAPCOORD(p_building->Y) + 40;
            at_cor_z = PRCCOORD_TO_MAPCOORD(p_building->Z);
            if (mounted_los(at_cor_x, at_cor_y, at_cor_z, tg_cor_x, tg_cor_y, tg_cor_z))
            {
                mgun_shoot_at_target(p_building);
            }
            rotate_object(p_building);
        }
        break;
    }
}

void process_gate1(struct Thing *p_building)
{
    asm volatile ("call ASM_process_gate1\n"
        : : "a" (p_building));
}

void process_bld36(struct Thing *p_building)
{
    struct M33 *p_matx;
    assert(p_building->U.UObject.MatrixIndex < next_local_mat);
    p_matx = &local_mats[p_building->U.UObject.MatrixIndex];
    rotate_object_axis(p_matx, 0, 0, 32);
}

void process_building(struct Thing *p_building)
{
#if 0
    asm volatile ("call ASM_process_building\n"
        : : "a" (p_building));
#endif
    if ((p_building->Flag & TngF_Unkn00040000) != 0)
    {
        if ((p_building->Flag & TngF_Destroyed) != 0)
        {
            p_building->Flag &= ~TngF_Unkn00040000;
            return;
        }
        collapse_building(p_building->X >> 8, p_building->Y >> 8, p_building->Z >> 8, p_building);
        return;
    }

    switch (p_building->SubType)
    {
    case SubTT_BLD_SHUTLDR:
        if ((p_building->Flag & TngF_Destroyed) == 0)
        {
            if ((p_building->U.UObject.PrevThing == 0) && ((p_building->Flag & TngF_Unkn08000000) == 0)) {
                process_shuttle_loader(p_building);
            }
            if ((p_building->U.UObject.PrevThing == 0) && ((p_building->Flag & TngF_Unkn08000000) != 0)
              && ((LbRandomAnyShort() & 3) == 0) && (((gameturn + p_building->ThingOffset) & 7) == 0)) {
                create_a_pod(0, p_building, 0);
            }
        }
        break;
    case SubTT_BLD_DOME:
        if ((p_building->Flag & TngF_Destroyed) == 0)
            process_dome1(p_building);
        break;
    case SubTT_BLD_MGUN:
        if ((p_building->Flag & TngF_Destroyed) == 0)
            process_mounted_gun(p_building);
        break;
    case SubTT_BLD_GATE:
        process_gate1(p_building);
        break;
    case SubTT_BLD_MOVN_ROTOR:
        process_bld36(p_building);
        break;
    default:
        if (p_building->State == BldSt_OBJ_UNKN09) {
            collapse_building(p_building->X >> 8, p_building->Y >> 8, p_building->Z >> 8, p_building);
        }
        break;
    }
}

int building_hit_by_bullet(struct Thing *p_thing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type)
{
    if (p_attacker != NULL) {
        p_attacker->U.UPerson.Flag3 |= 0x40;
    }
    if (p_thing->SubType != 32)
    {
        if ((p_thing->Flag & TngF_Destroyed) == 0)
        {
            int health;

            play_dist_sample(p_thing, 65, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
            health = p_thing->Health - (hp >> 1);
            if (health < 0) {
                collapse_building(PRCCOORD_TO_MAPCOORD(p_thing->X),
                  PRCCOORD_TO_MAPCOORD(p_thing->Y), PRCCOORD_TO_MAPCOORD(p_thing->Z), p_thing);
            }
            p_thing->Health = health;
        }
        return hp;
    }
    p_thing->Flag |= TngF_Unkn01000000;
    if ((p_thing->Flag & TngF_Destroyed) == 0)
    {
        int health_decr;

        health_decr = hp >> 1 >> 1;
        p_thing->Health -= health_decr;
        p_thing->U.UObject.Cost = 5;
        if (p_thing->Health <= 0)
        {
            init_mgun_explode(p_thing);
            return p_thing->Health + health_decr;
        }
    }
    return 0;
}

/******************************************************************************/
