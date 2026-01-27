/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing.c
 *     Thing structure support.
 * @par Purpose:
 *     Implement general functions for handling Thing instances.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing.h"

#include "bfbox.h"
#include "bfutility.h"
#include "bfmemut.h"
#include "bfscreen.h"
#include "ssampply.h"
#include <assert.h>
#include <stdlib.h>

#include "bigmap.h"
#include "building.h"
#include "bmbang.h"
#include "display.h"
#include "engindrwlstx.h"
#include "enginfexpl.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "matrix.h"
#include "packet.h"
#include "pepgroup.h"
#include "player.h"
#include "sound.h"
#include "thing_fire.h"
#include "vehicle.h"
#include "game.h"
#include "swlog.h"
/******************************************************************************/
#pragma pack(1)

struct UnkFLight { // sizeof=0x0A
    short unfulgt_0;
    short unfulgt_2;
    short unfulgt_4;
    short unfulgt_6;
    ubyte unfulgt_8;
    ubyte unfulgt_9;
};

#pragma pack()
/******************************************************************************/

ushort next_unkn_full_light = 1;
extern struct UnkFLight unkn_full_lights[50];

ubyte debug_log_things = 0;

/** Radiuses of Things of type STATIC.
 */
short static_radii[] = {
  40, 50, 80, 15,  0, 40,  0,  0,  0, 40,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0, 70, 70, 70, 70,
 130, 40, 80, 40, 40, 40, 50,100, 40, 40,
   0,  0,  0,  0,  0,  0,  0,  0, 40, 70,
  40, 40, 80,100,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

const char *thing_type_names[] = {
  "NONE",
  "UNKN1",
  "VEHICLE",
  "PERSON",
  "UNKN4",
  "STATIC",
  "ROCKET",
  "UNKN7",
  "UNKN8",
  "BUILDING",
  "UNKN10",
  "LASER11",
  "LASER_GUIDED",
  "UNKN13",
  "UNKN14",
  "MINE",
  "GRENADE",
  "SPARK",
  "UNKN18",
  "LASER_ELEC",
  "INTELLIG_DOOR",
  "SCALE_EFFECT",
  "NUCLEAR_BOMB",
  "UNKN23",
  "SMOKE_GENERTR",
  "DROPPED_ITEM",
  "CARRIED_ITEM",
  "ELECTR_STRAND",
  "RAZOR_WIRE",
  "LASER29",
  "TIME_POD",
  "AIR_STRIKE",
  "CANISTER",
  "VEH_TURRET",
  "UNKN34",
  "UNKN35",
  "STASIS_POD",
  "SOUL",
  "LASER38",
  "UNKN39",
  "UNKN40",
  "UNKN41",
  "UNKN42",
  "UNKN43",
  "UNKN44",
  "UNKN45",
  "UNKN46",
  "UNKN47",
  "UNKN48",
  "UNKN49",
  "BANG",
  "UNKN51",
  "FIRE",
  "SFX",
  "TEMP_LIGHT",
  "UNKN55",
  "UNKN56",
};

const char *state_change_result_names[] = {
    "accepted",
    "already done",
    "change denied",
    "goal unattainable",
};

/******************************************************************************/

TbBool thing_type_is_simple(short ttype)
{
    return (ttype == SmTT_STATIC) ||
     (ttype == SmTT_SPARK) ||
     (ttype == SmTT_INTELLIG_DOOR) ||
     (ttype == SmTT_SCALE_EFFECT) ||
     (ttype == SmTT_NUCLEAR_BOMB) ||
     (ttype == SmTT_SMOKE_GENERATOR) ||
     (ttype == SmTT_DROPPED_ITEM) ||
     (ttype == SmTT_CARRIED_ITEM) ||
     (ttype == SmTT_ELECTRIC_STRAND) ||
     (ttype == SmTT_TIME_POD) ||
     (ttype == SmTT_CANISTER) ||
     (ttype == SmTT_STASIS_POD) ||
     (ttype == SmTT_SOUL) ||
     (ttype == SmTT_BANG) ||
     (ttype == SmTT_FIRE) ||
     (ttype == SmTT_SFX) ||
     (ttype == SmTT_TEMP_LIGHT);
}

struct Thing *get_thing_safe(ThingIdx thing, ubyte ttype)
{
    struct Thing *p_thing;

    if (thing_type_is_simple(ttype))
    {
        if ((thing > -1) || (thing < STHINGS_LIMIT))
            return INVALID_THING;
    }
    else
    {
        if ((thing < 1) || (thing > THINGS_LIMIT))
            return INVALID_THING;
    }
    p_thing = &things[thing];
    if (p_thing->Type != ttype)
            return INVALID_THING;
    return p_thing;
}

TbBool thing_position_uses_y_mul_8(struct Thing *p_thing)
{
    switch (p_thing->Type)
    {
    case TT_BUILDING:
        return (p_thing->SubType == SubTT_BLD_MGUN);
    case TT_VEHICLE:
    case TT_PERSON:
    default:
        return true;
    }
}

void get_thing_position_mapcoords(short *x, short *y, short *z, ThingIdx thing)
{
    short cor_x, cor_y, cor_z;

    if (thing >= 0) {
        struct Thing *p_thing;
        p_thing = &things[thing];
        cor_x = PRCCOORD_TO_MAPCOORD(p_thing->X);
        if (thing_position_uses_y_mul_8(p_thing))
            cor_y = PRCCOORD_TO_YCOORD(p_thing->Y);
        else
            cor_y = PRCCOORD_TO_MAPCOORD(p_thing->Y);
        cor_z = PRCCOORD_TO_MAPCOORD(p_thing->Z);
    } else {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        cor_x = PRCCOORD_TO_MAPCOORD(p_sthing->X);
        cor_y = PRCCOORD_TO_YCOORD(p_sthing->Y);
        cor_z = PRCCOORD_TO_MAPCOORD(p_sthing->Z);
    }
    if (x != NULL)
        *x = cor_x;
    if (y != NULL)
        *y = cor_y;
    if (z != NULL)
        *z = cor_z;
}

u32 get_things_distance_mapcoords_precise(ThingIdx tng1, ThingIdx tng2)
{
    short x1, y1, z1;
    short x2, y2, z2;
    get_thing_position_mapcoords(&x1, &y1, &z1, tng1);
    get_thing_position_mapcoords(&x2, &y2, &z2, tng2);
    return map_distance_deltas_precise(x2 - x1, y2 - y1, z2 - z1);
}

u32 get_things_distance_mapcoords_fast(ThingIdx tng1, ThingIdx tng2)
{
    short x1, y1, z1;
    short x2, y2, z2;
    get_thing_position_mapcoords(&x1, &y1, &z1, tng1);
    get_thing_position_mapcoords(&x2, &y2, &z2, tng2);
    return map_distance_deltas_fast(x2 - x1, y2 - y1, z2 - z1);
}

const char *state_change_result_name(StateChRes res)
{
    return state_change_result_names[res];
}

struct Thing *effective_owner_of_thing(struct Thing *p_thing)
{
    struct Thing *p_owntng;

    if (p_thing == NULL) {
        return NULL;
    }

    // Consider player agent an owner even if it got persuaded
    if ((p_thing->Flag & TngF_PlayerAgent) != 0) {
        return p_thing;
    }

    p_owntng = &things[p_thing->Owner];

    if (((p_thing->Flag & TngF_Persuaded) != 0) && (p_owntng->Flag & TngF_PlayerAgent) != 0) {
        return p_owntng;
    }

    if (p_thing->Type == TT_MINE && p_thing->SubType == 48) {
        return p_owntng;
    }

    return p_thing;
}

ubyte on_mapwho(struct Thing *p_thing)
{
    ubyte ret;
    asm volatile (
      "call ASM_on_mapwho\n"
        : "=r" (ret) : "a" (p_thing));
    return ret;
}

void move_mapwho(struct Thing *p_thing, int x, int y, int z)
{
    asm volatile (
      "call ASM_move_mapwho\n"
        : : "a" (p_thing), "d" (x), "b" (y), "c" (z));
}

void init_things(void)
{
    asm volatile ("call ASM_init_things\n"
        :  :  : "eax" );
}

void quick_light_unkn_func_04(short a1, int a2, short a3, short a4)
{
    asm volatile (
      "call ASM_quick_light_unkn_func_04\n"
        : : "a" (a1), "d" (a2), "b" (a3), "c" (a4));
    return;
}

void unkn_full_update_lights(void)
{
    ushort i;

    for (i = 1; i < next_unkn_full_light; i++)
    {
        struct UnkFLight *p_ufl;
        p_ufl = &unkn_full_lights[i];
        quick_light_unkn_func_04(p_ufl->unfulgt_2, p_ufl->unfulgt_4, p_ufl->unfulgt_6, p_ufl->unfulgt_0);
    }
    next_unkn_full_light = 1;
}

void apply_full_light(short lx, short lz, ushort b, ushort intens, ushort lid)
{
    asm volatile (
      "push %4\n"
      "call ASM_apply_full_light\n"
        : : "a" (lx), "d" (lz), "b" (b), "c" (intens), "g" (lid));
    return;
}

int process_things_mines_explode(int affected_max)
{
    int ret;
    asm volatile ("call ASM_process_things_mines_explode\n"
        : "=r" (ret) : "a" (affected_max));
    return ret;
}

void process_things_unkn_sub2(int tech_lv, int got_many)
{
    asm volatile (
      "call ASM_process_things_unkn_sub2\n"
        : : "a" (tech_lv), "d" (got_many));
}

int process_things_bang(int affected_max)
{
    int ret;
    asm volatile ("call ASM_process_things_bang\n"
        : "=r" (ret) : "a" (affected_max));
    return ret;
}

void unkn_update_lights(void)
{
    asm volatile ("call ASM_unkn_update_lights\n"
        :  :  : "eax" );
    return;
}

void process_rocket(struct Thing *p_rocket)
{
    asm volatile (
      "call ASM_process_rocket\n"
        : : "a" (p_rocket));
}

void process_laser_guided(struct Thing *p_laser)
{
    asm volatile (
      "call ASM_process_laser_guided\n"
        : : "a" (p_laser));
}

void process_mine(struct SimpleThing *p_mine)
{
    asm volatile (
      "call ASM_process_mine\n"
        : : "a" (p_mine));
}

void process_grenade(struct Thing *p_grenade)
{
    asm volatile (
      "call ASM_process_grenade\n"
        : : "a" (p_grenade));
}

void process_laser_elec(struct Thing *p_elec)
{
    asm volatile (
      "call ASM_process_laser_elec\n"
        : : "a" (p_elec));
}

void process_razor_wire(struct Thing *p_thing)
{
    ;
}

struct SimpleThing *init_nuclear_bomb(MapCoord x, MapCoord y, MapCoord z)
{
#if 0
    struct SimpleThing *ret;
    asm volatile (
      "call ASM_init_nuclear_bomb\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z));
    return ret;
#endif
    struct SimpleThing *p_sthing;
    ThingIdx new_sthing;

    if (x < 0)
        return 0;
    if (z < 0)
        return 0;
    if (sthings_used > STHINGS_LIMIT - 5)
        return 0;

    new_sthing = get_new_sthing();
    if (new_sthing == 0)
        return 0;
    if (new_sthing < -STHINGS_LIMIT)
        return 0;

    p_sthing = &sthings[new_sthing];
    p_sthing->Type = SmTT_NUCLEAR_BOMB;
    p_sthing->Radius = 64;
    p_sthing->X = MAPCOORD_TO_PRCCOORD(x, 0);
    p_sthing->Z = MAPCOORD_TO_PRCCOORD(z, 0);
    p_sthing->Y = MAPCOORD_TO_PRCCOORD(y, 0);
    p_sthing->Timer1 = 0;
    p_sthing->Flag = (TngF_InVehicle|TngF_Persuaded|TngF_Unkn0004);
    add_node_sthing(new_sthing);
    set_nuclear_shade_point(x, y, z);

    return p_sthing;
}


void process_air_strike(struct Thing *p_thing)
{
#if 0
    asm volatile (
      "call ASM_process_air_strike\n"
        : : "a" (p_thing));
#endif
    struct SimpleThing *p_sthing;

    p_thing->Timer1--;
    if (p_thing->Timer1 < 0)
    {
        remove_thing(p_thing->ThingOffset);
        return;
    }

    if (p_thing->Timer1 == WEP_AIRSTRIKE_BOMB_GTURNS * WEP_AIRSTRIKE_BOMBS_NUM + WEP_AIRSTRIKE_DELAY_GTURNS)
    {
        play_dist_speech(p_thing, 4, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
    }

    if ((p_thing->Timer1 < WEP_AIRSTRIKE_BOMB_GTURNS * WEP_AIRSTRIKE_BOMBS_NUM) &&
      (p_thing->Timer1 % WEP_AIRSTRIKE_BOMB_GTURNS) == 0)
    {
        MapCoord cor_x, cor_y, cor_z;

        cor_x = PRCCOORD_TO_MAPCOORD(p_thing->X);
        cor_z = PRCCOORD_TO_MAPCOORD(p_thing->Z);
        cor_x += (LbRandomAnyShort() % 3000) - 1500;
        cor_z += (LbRandomAnyShort() % 3000) - 1500;
        cor_y = PRCCOORD_TO_MAPCOORD(alt_at_point(cor_x, cor_z)) + 10;
        bang_new4(MAPCOORD_TO_PRCCOORD(cor_x,0), MAPCOORD_TO_PRCCOORD(cor_y,0),
          MAPCOORD_TO_PRCCOORD(cor_z,0), 50);

        p_sthing = init_nuclear_bomb(cor_x, cor_y, cor_z);
        if (p_sthing != NULL)
        {
            play_dist_ssample(p_sthing, 1, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
            p_sthing->Timer1 = WEP_AIRSTRIKE_IMPACT_GTURNS;
            p_sthing->Owner2 = p_thing->Owner;
            p_sthing->Radius = 0;
            p_sthing->Flag &= ~TngF_Persuaded;
        }
    }
}

void process_unkn35_crashing(struct Thing *p_thing)
{
#if 0
    asm volatile (
      "call ASM_process_unkn35_crashing\n"
        : : "a" (p_thing));
#endif
    ushort rnd;
    int alt_under;

    LOGNO("Thing (%d,%d,%d)", (int)p_thing->X, (int)p_thing->Y, (int)p_thing->Z);
    move_mapwho(p_thing, p_thing->X + p_thing->VX, p_thing->Y + p_thing->VY, p_thing->Z + p_thing->VZ);

    rnd = LbRandomAnyShort();
    p_thing->VX -= p_thing->VX >> 4;
    p_thing->VZ -= p_thing->VZ >> 4;
    p_thing->VY -= ((rnd & 0x7F) + 0x200);

    alt_under = alt_at_point(PRCCOORD_TO_MAPCOORD(p_thing->X), PRCCOORD_TO_MAPCOORD(p_thing->Z));
    if (alt_under > p_thing->Y)
    {
        bang_new4(p_thing->X, p_thing->Y, p_thing->Z, 80);
        remove_thing(p_thing->ThingOffset);
        delete_node(p_thing);
    }
}

void process_unkn35(struct Thing *p_thing)
{
    if (p_thing->State == 13) {
        process_unkn35_crashing(p_thing);
        return;
    }
    //TODO implement the rest of the state update
}

void process_laser(struct Thing *p_laser)
{
    asm volatile (
      "call ASM_process_laser\n"
        : : "a" (p_laser));
}

void process_electric_strand(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_electric_strand\n"
        : : "a" (p_sthing));
}

void process_canister(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_canister\n"
        : : "a" (p_sthing));
}

void process_stasis_pod(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_stasis_pod\n"
        : : "a" (p_sthing));
}

void process_time_pod(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_time_pod\n"
        : : "a" (p_sthing));
}

void process_soul(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_soul\n"
        : : "a" (p_sthing));
}

void process_bang(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_bang\n"
        : : "a" (p_sthing));
}

void finish_bang(struct SimpleThing *p_sthing, ThingIdx thing)
{
    remove_sthing(thing);
    delete_snode(p_sthing);
}

void FIRE_process_flame(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_FIRE_process_flame\n"
        : : "a" (p_sthing));
}

void FIRE_finish_flame(struct SimpleThing *p_sthing, ThingIdx thing)
{
    ReleaseLoopedSample(thing, 16);
    play_dist_ssample(p_sthing, 0x17, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
    remove_sthing(thing);
    delete_snode(p_sthing);
}

void process_spark(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_spark\n"
        : : "a" (p_sthing));
}

void process_intelligent_door(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_intelligent_door\n"
        : : "a" (p_sthing));
}

void process_scale_effect(struct SimpleThing *p_sthing, ThingIdx thing)
{
    asm volatile (
      "call ASM_process_scale_effect\n"
        : : "a" (p_sthing), "d" (thing));
}

void process_nuclear_bomb(struct SimpleThing *p_sthing, ThingIdx thing_item)
{
    asm volatile (
      "call ASM_process_nuclear_bomb\n"
        : : "a" (p_sthing), "d" (thing_item));
}

void process_smoke_generator(struct SimpleThing *p_sthing)
{
    asm volatile (
      "call ASM_process_smoke_generator\n"
        : : "a" (p_sthing));
}

void process_thing_checksum(struct Thing *p_thing, ThingIdx thing)
{
    ingame.fld_unkC4B += thing;
    ingame.fld_unkC4B += p_thing->Y + p_thing->X + p_thing->Type + p_thing->State;
}

TbBool process_thing_unkflag02000000(struct Thing *p_thing, ThingIdx thing)
{
    if (((p_thing->Flag2 & TgF2_Unkn02000000) != 0) && ((gameturn + thing) & 3) != 0) {
        return true;
    }
    p_thing->Flag2 &= ~TgF2_Unkn02000000;
    return false;
}

TbBool process_thing_unkflag0002(struct Thing *p_thing)
{
    if ((p_thing->Flag2 & TgF2_Unkn0002) != 0)
    {
        if ((gameturn & 3) != 0)
            return true;
        p_thing->U.UPerson.SpecialTimer--;
        if (p_thing->U.UPerson.SpecialTimer >= 0)
            return true;
        if ((p_thing->Flag & TngF_InVehicle) == 0)
            add_node_thing(p_thing->ThingOffset);
        p_thing->Flag2 &= ~TgF2_Unkn0002;
        return true;
    }
    return false;
}

void process_thing(struct Thing *p_thing, ThingIdx thing)
{
    switch (p_thing->Type)
    {
    case TT_VEHICLE:
        process_vehicle(p_thing);
        break;
    case TT_PERSON:
    case TT_UNKN4:
        process_shield(p_thing);
        process_person(p_thing);
        if ((p_thing->Flag & TngF_Unkn0800) != 0)
            p_thing->Flag2 |= TgF2_Unkn0400;
        else
            p_thing->Flag2 &= ~TgF2_Unkn0400;
        break;
    case TT_ROCKET:
        process_rocket(p_thing);
        break;
    case TT_BUILDING:
        process_building(p_thing);
        break;
    case TT_LASER_GUIDED:
        process_laser_guided(p_thing);
        break;
    case TT_MINE:
        process_mine((struct SimpleThing *)p_thing);
        break;
    case TT_GRENADE:
        process_grenade(p_thing);
        break;
    case TT_LASER_ELEC:
        process_laser_elec(p_thing);
        break;
    case TT_RAZOR_WIRE: // exploding wire it a subtype of razor wire
        process_razor_wire(p_thing);
        break;
    case TT_AIR_STRIKE:
        process_air_strike(p_thing);
        break;
    case TT_UNKN35:
        process_unkn35(p_thing);
        break;
    case TT_LASER11:
    case TT_LASER29:
    case TT_LASER38: // on PSX, laser type=38 has additional light effect
        process_laser(p_thing);
        break;
    default:
        break;
    }
}

void process_sthing_checksum(struct SimpleThing *p_sthing, ThingIdx thing)
{
    ingame.fld_unkC4B += p_sthing->Y + p_sthing->X + p_sthing->Type + p_sthing->State;
}

void process_carried_item(struct SimpleThing *p_item)
{
    ThingIdx item;
    struct Thing *p_owner;

    p_owner = &things[p_item->U.UWeapon.Owner];
    p_item->X = p_owner->X;
    p_item->Y = p_owner->Y;
    p_item->Z = p_owner->Z;
    if (p_owner->State == PerSt_DEAD) {
        item = p_item->ThingOffset;
        p_item->Type = SmTT_DROPPED_ITEM;
        add_node_sthing(item);
    }
}

void process_temp_light(struct SimpleThing *p_sthing)
{
    int bri, rng;
    short lgtng_x, lgtng_y, lgtng_z;

    p_sthing->Timer1--;
    if (p_sthing->Timer1 < 0) {
        remove_sthing(p_sthing->ThingOffset);
        return;
    }
    bri = p_sthing->U.ULight.MinBright;
    if (p_sthing->U.ULight.RangeBright != 0) {
        rng = p_sthing->U.ULight.RangeBright;
        bri += (ushort)LbRandomAnyShort() % rng;
    }
    get_thing_position_mapcoords(&lgtng_x, &lgtng_y, &lgtng_z, p_sthing->ThingOffset);
    apply_full_light(lgtng_x, lgtng_y >> 3, lgtng_z, bri, 0);
}

void set_thing_frame(struct Thing *p_thing, ushort anim_start)
{
    assert(p_thing->Type != TT_PERSON);
    p_thing->StartFrame = anim_start;
    p_thing->Frame = nstart_ani[p_thing->StartFrame];
}

void reset_thing_frame(struct Thing *p_thing)
{
    assert(p_thing->Type != TT_PERSON);
    p_thing->Frame = nstart_ani[p_thing->StartFrame];
}

void reset_sthing_frame(struct SimpleThing *p_sthing)
{
    p_sthing->Frame = nstart_ani[p_sthing->StartFrame];
}

void process_burning_static(struct SimpleThing *p_sthing)
{
    p_sthing->Timer1--;
    if (p_sthing->Timer1 != 0)
        return;
    p_sthing->State = 13;
    reset_sthing_frame(p_sthing);
    stop_sample_using_heap(p_sthing->ThingOffset, 29);
}

void process_static(struct SimpleThing *p_sthing)
{
    switch (p_sthing->State)
    {
    case 36:
        play_dist_ssample(p_sthing, 29, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
        process_burning_static(p_sthing);
        break;
    case 12:
        process_burning_static(p_sthing);
        break;
    }
}

void process_sfx(struct SimpleThing *p_sthing)
{
    switch (p_sthing->State)
    {
    case 1:
        if (!dont_bother_with_explode_faces)
            break;
        stop_sample_using_heap(p_sthing->ThingOffset, 0x2E);
        play_dist_ssample(p_sthing, 0x2F, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        p_sthing->State = 2;
        p_sthing->Timer1 = 100;
        break;
    default:
        p_sthing->Timer1--;
        if (p_sthing->Timer1 >= 0)
            break;
        remove_sthing(p_sthing->ThingOffset);
        if (p_sthing->State != 2)
            break;
        ingame.SoundThing = 0;
        break;
    }
}

void process_sthing(struct SimpleThing *p_sthing, ThingIdx thing)
{
    switch (p_sthing->Type)
    {
    case SmTT_CARRIED_ITEM:
        process_carried_item(p_sthing);
        break;
    case SmTT_ELECTRIC_STRAND:
        process_electric_strand(p_sthing);
        break;
    case SmTT_CANISTER:
        process_canister(p_sthing);
        break;
    case SmTT_STASIS_POD:
        process_stasis_pod(p_sthing);
        break;
    case SmTT_TIME_POD:
        process_time_pod(p_sthing);
        break;
    case SmTT_SOUL:
        process_soul(p_sthing);
        break;
    case SmTT_BANG:
        if ((p_sthing->U.UEffect.VY != 0) || (p_sthing->U.UEffect.VX != 0)) {
            process_bang(p_sthing);
        } else {
            finish_bang(p_sthing, thing);
        }
        break;
    case SmTT_FIRE:
        if (p_sthing->U.UFire.flame != 0) {
            FIRE_process_flame(p_sthing);
        } else {
            FIRE_finish_flame(p_sthing, thing);
        }
        break;
    case SmTT_SFX:
        process_sfx(p_sthing);
        break;
    case SmTT_TEMP_LIGHT:
        process_temp_light(p_sthing);
        break;
    case SmTT_SPARK:
        process_spark(p_sthing);
        break;
    case SmTT_STATIC:
        process_static(p_sthing);
        break;
    case SmTT_INTELLIG_DOOR:
        process_intelligent_door(p_sthing);
        break;
    case SmTT_SCALE_EFFECT:
        process_scale_effect(p_sthing, thing);
        break;
    case SmTT_NUCLEAR_BOMB:
        process_nuclear_bomb(p_sthing, thing);
        break;
    case SmTT_SMOKE_GENERATOR:
        process_smoke_generator(p_sthing);
        break;
    case SmTT_DROPPED_ITEM:
        if (p_sthing->SubType == WEP_EXPLMINE || p_sthing->SubType == WEP_ELEMINE) {
            process_mine(p_sthing);
        } else {
            if (!in_network_game || p_sthing->U.UWeapon.WeaponType == 31)
                break;
            p_sthing->U.UWeapon.Ammo++;
        }
        break;
    default:
        break;
    }
}

void process_things(void)
{
#if 0
    asm volatile ("call ASM_process_things\n"
        :  :  : "eax" );
    return;
#endif
    int i;
    ushort plyr;

    for (plyr = 0; plyr < 8; plyr++)
    {
        PlayerInfo *p_player;
        struct Thing *p_dcthing;
        ThingIdx dcthing;

        p_player = &players[plyr];
        dcthing = p_player->DirectControl[0];
        p_dcthing = &things[dcthing];
        if (((1 << plyr) & ingame.InNetGame_UNSURE) != 0
          && (p_dcthing->Flag & TngF_Unkn1000) == 0)
        {
#if 0
            for (i = 0; i < playable_agents; i++)
              ;
#endif
            p_dcthing->Flag |= TngF_Unkn1000;
        }
    }

    if (in_network_game)
    {
        if (ingame.fld_unkCB7 > 150)
            process_things_mines_explode((rand() & 0x1F) + 2);
        if ((net_game_play_flags & NGPF_Unkn10) != 0 && (gameturn & 0xF) == 0)
            process_things_unkn_sub2(login_control__TechLevel, ingame.fld_unkCB7 > 100);
        if (things_used > 900)
            process_things_bang(16);
        else if ( things_used > 700 || ((gameturn & 0xF) == 0))
            process_things_bang(1);
    }

    if ((gameturn & 0x1F) == 0)
    {
        for (i = 0; i < PEOPLE_GROUPS_COUNT+1; i++) {
            group_actions[i].Storming &= ~0x4000;
        }
    }

    shield_frames_cycle();

    player_update_thermal(local_player_no);

#if 0
    merged_noop_unkn1(gameturn);
#endif
    build_same_type_headers();
    ingame.fld_unkC4B = 0;
    animate_textures();
    unkn_update_lights();
    unkn_full_update_lights();

    if (!execute_commands)
        return;
    if ((ingame.Flags & GamF_StopThings) == 0)
        return;
    monitor_all_samples();

    if (!in_network_game && (pktrec_mode == PktR_NONE)
      && ((ingame.Flags & GamF_Unkn0004) != 0) && ((gameturn & 0xF) != 0))
        return;

    if (execute_commands)
    {
        struct Thing *p_thing;
        int remain;
        ThingIdx thing, nxthing;

        remain = things_used;
        for (thing = things_used_head; thing > 0; thing = nxthing)
        {
            if (--remain == -1) {
                break;
            }
            p_thing = &things[thing];
            nxthing = p_thing->LinkChild;

            process_thing_checksum(p_thing, thing);

            if (process_thing_unkflag02000000(p_thing, thing))
                continue;

            if (process_thing_unkflag0002(p_thing))
                continue;

            process_thing(p_thing, thing);
        }
    }

    if (execute_commands)
    {
        struct SimpleThing *p_sthing;
        int remain;
        ThingIdx thing, nxthing;

        remain = sthings_used;
        for (thing = sthings_used_head; thing < 0; thing = nxthing)
        {
            if (--remain == -1) {
                break;
            }
            p_sthing = &sthings[thing];
            nxthing = p_sthing->LinkChild;

            process_sthing_checksum(p_sthing, thing);

            process_sthing(p_sthing, thing);
        }
    }

    if (execute_commands)
    {
        people_intel(0);
    }
    navi_onscreen_debug(1);
}

const char *thing_type_name(ubyte tngtype, ubyte subtype)
{
    if (tngtype >= sizeof(thing_type_names)/sizeof(thing_type_names[1]))
        return "OUTRANGED";
    if (tngtype == TT_PERSON)
        return person_type_name(subtype);
    if (tngtype == TT_VEHICLE)
        return vehicle_type_name(subtype);
    if (tngtype == TT_BUILDING)
        return building_type_name(subtype);
    // TODO support some sub-types
    return thing_type_names[tngtype];
}

void snprint_thing(char *buf, ulong buflen, struct Thing *p_thing)
{
    char *s;
    ubyte nparams;

    s = buf;

    sprintf(s, "T%02d = %s( ", (int)p_thing->ThingOffset, thing_type_name(p_thing->Type, p_thing->SubType));
    s += strlen(s);
    nparams = 0;

    {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Coord(%d,%d,%d)", (int)PRCCOORD_TO_MAPCOORD(p_thing->X),
          (int)PRCCOORD_TO_MAPCOORD(p_thing->Y), (int)PRCCOORD_TO_MAPCOORD(p_thing->Z));
        s += strlen(s);
        nparams++;
    }
    if (p_thing->Health != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Health(%hd)", p_thing->Health);
        s += strlen(s);
        nparams++;
    }
    if (p_thing->Flag != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Flag(0x%lx)", p_thing->Flag);
        s += strlen(s);
        nparams++;
    }
    if (p_thing->Type == TT_PERSON) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        snprint_person_state(s, buflen - (s-buf), p_thing);
        s += strlen(s);
        nparams++;
    }
    // TODO support more properties

    snprintf(s, buflen - (s-buf), " )");
}

void snprint_sthing(char *buf, ulong buflen, struct SimpleThing *p_sthing)
{
    char *s;
    ubyte nparams;

    s = buf;

    sprintf(s, "T%02d = %s( ", (int)p_sthing->ThingOffset, thing_type_name(p_sthing->Type, p_sthing->SubType));
    s += strlen(s);
    nparams = 0;

    {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Coord(%d,%d,%d)", (int)PRCCOORD_TO_MAPCOORD(p_sthing->X),
          (int)PRCCOORD_TO_MAPCOORD(p_sthing->Y), (int)PRCCOORD_TO_MAPCOORD(p_sthing->Z));
        s += strlen(s);
        nparams++;
    }
    if (p_sthing->Flag != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Flag(0x%lx)", p_sthing->Flag);
        s += strlen(s);
        nparams++;
    }
    // TODO support more properties

    snprintf(s, buflen - (s-buf), " )");
}

TbBool thing_is_destroyed(ThingIdx thing)
{
    if (thing == 0) {
        return true;
    } else if (thing > 0) {
        struct Thing *p_thing;
        p_thing = &things[thing];
        return ((p_thing->Flag & TngF_Destroyed) != 0);
    } else {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        return ((p_sthing->Flag & TngF_Destroyed) != 0);
    }
}

TbResult delete_node(struct Thing *p_thing)
{
    TbResult ret;
    asm volatile ("call ASM_delete_node\n"
        : "=r" (ret) : "a" (p_thing));
    return ret;
}

TbResult delete_snode(struct SimpleThing *p_sthing)
{
    TbResult ret;
    asm volatile ("call ASM_delete_snode\n"
        : "=r" (ret) : "a" (p_sthing));
    return ret;
}

void add_node_thing(ThingIdx new_thing)
{
    asm volatile ("call ASM_add_node_thing\n"
        : : "a" (new_thing));
}

short get_new_thing(void)
{
    short ret;
    asm volatile ("call ASM_get_new_thing\n"
        : "=r" (ret) : );
    return ret;
}

void remove_thing(short tngno)
{
    asm volatile ("call ASM_remove_thing\n"
        : : "a" (tngno));
}

void add_node_sthing(ThingIdx new_thing)
{
    asm volatile ("call ASM_add_node_sthing\n"
        : : "a" (new_thing));
}

short get_new_sthing(void)
{
    short ret;
    asm volatile ("call ASM_get_new_sthing\n"
        : "=r" (ret) : );
    return ret;
}

void remove_sthing(short tngno)
{
    asm volatile ("call ASM_remove_sthing\n"
        : : "a" (tngno));
}

short add_static(int x, int y, int z, ushort frame, int timer)
{
#if 0
    short ret;
    asm volatile (
      "push %5\n"
      "call ASM_add_static\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z), "c" (frame), "g" (timer));
    return ret;
#else
    ThingIdx thing;
    struct SimpleThing *p_sthing;

    if (map_coords_limit(NULL, NULL, NULL, x, y, z))
        return 0;
    if (sthings_used > STHINGS_LIMIT - 5)
        return 0;

    thing = get_new_sthing();
    if (thing == 0)
        return 0;
    if (thing <= -STHINGS_LIMIT-1)
        return 0;
    p_sthing = &sthings[thing];
    p_sthing->Z = MAPCOORD_TO_PRCCOORD(z,127);
    p_sthing->X = MAPCOORD_TO_PRCCOORD(x,127);
    p_sthing->Y = y;
    p_sthing->Parent = 0;
    p_sthing->StartFrame = frame - 1;
    p_sthing->Frame = nstart_ani[p_sthing->StartFrame + 1];
    add_node_sthing(thing);
    p_sthing->Type = SmTT_STATIC;
    p_sthing->Radius = 64;
    p_sthing->Flag = 4;
    p_sthing->Timer1 = timer;
    return thing;
#endif
}

TbBool thing_is_within_circle(ThingIdx thing, short X, short Z, ushort R)
{
    s32 dtX, dtZ, r2;

    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_sthing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_sthing->Z) - Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_thing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_thing->Z) - Z;
    }
    r2 = R * R;
    return ((dtZ * dtZ + dtX * dtX) < r2);
}

TbBool thing_intersects_circle(ThingIdx thing, short X, short Z, ushort R)
{
    s32 dtX, dtZ, cyl_r2, tng_r;

    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_sthing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_sthing->Z) - Z;
        tng_r = p_sthing->Radius;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_thing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_thing->Z) - Z;
        tng_r = p_thing->Radius;
    }
    cyl_r2 = R * R;
    return ((dtZ * dtZ + dtX * dtX) < cyl_r2 + tng_r * tng_r);
}

TbBool thing_intersects_cylinder(ThingIdx thing, short X, short Y, short Z, ushort R, ushort H)
{
    s32 dtX, dtY, dtZ, cyl_r2, tng_r, h_min, h_max;

    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_sthing->X) - X;
        dtY = PRCCOORD_TO_MAPCOORD(p_sthing->Y) - Y;
        dtZ = PRCCOORD_TO_MAPCOORD(p_sthing->Z) - Z;
        tng_r = p_sthing->Radius;
        h_min = H;
        h_max = H;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_thing->X) - X;
        dtY = PRCCOORD_TO_MAPCOORD(p_thing->Y) - Y;
        dtZ = PRCCOORD_TO_MAPCOORD(p_thing->Z) - Z;
        tng_r = p_thing->Radius;
        h_min = H;
        h_max = H;
        // Things represented by 3D objects require lower minimum
        if ((p_thing->Type == TT_VEHICLE) || (p_thing->Type == TT_BUILDING)) {
            h_min += 80;
        }
    }
    cyl_r2 = R * R;
    return ((dtZ * dtZ + dtX * dtX) < cyl_r2 + tng_r * tng_r)
        && ((dtY > -h_min) && (dtY < h_max));
}

void build_same_type_headers(void)
{
    asm volatile ("call ASM_build_same_type_headers\n"
        :  :  : "eax" );
}

short get_thing_same_type_head(short ttype, short subtype)
{
    ThingIdx thing;

    switch (ttype)
    {
    case TT_PERSON:
    case TT_UNKN4:
        thing = same_type_head[1];
        break;
    case TT_VEHICLE:
        if (subtype == SubTT_VEH_SHIP)
            thing = same_type_head[5];
        else
            thing = same_type_head[2];
        break;
    case TT_BUILDING:
        if (subtype == SubTT_BLD_MGUN)
            thing = same_type_head[7];
        else
            thing = same_type_head[3];
        break;
    case TT_MINE:
        if (subtype == 48)
            thing = same_type_head[6];
        break;
    default:
        thing = 0;
        break;
    }
    return thing;
}

void new_thing_traffic_clone(struct SimpleThing *p_clsthing)
{
    asm volatile ("call ASM_new_thing_traffic_clone\n"
        : : "a" (p_clsthing));
}

short new_thing_smoke_gen_clone(struct SimpleThing *p_clsthing)
{
    struct SimpleThing *p_sthing;
    ThingIdx thing;

    thing = add_static(PRCCOORD_TO_MAPCOORD(p_clsthing->X), p_clsthing->Y,
      PRCCOORD_TO_MAPCOORD(p_clsthing->Z), 0, p_clsthing->Timer1);
    p_sthing = &sthings[thing];
    p_sthing->Type = SmTT_SMOKE_GENERATOR;
    p_sthing->U.UEffect.VX = p_clsthing->U.UEffect.VX;
    p_sthing->U.UEffect.VY = p_clsthing->U.UEffect.VY;
    p_sthing->U.UEffect.VZ = p_clsthing->U.UEffect.VZ;
    p_sthing->StartTimer1 = p_clsthing->StartTimer1;
    p_sthing->U.UEffect.OX = p_clsthing->U.UEffect.OX;
    p_sthing->U.UEffect.OY = p_clsthing->U.UEffect.OY;
    p_sthing->U.UEffect.OZ = p_clsthing->U.UEffect.OZ;
    return thing;
}

short new_thing_static_clone(struct SimpleThing *p_clsthing)
{
    struct SimpleThing *p_sthing;
    ThingIdx thing;
    ushort frame;

    thing = add_static(PRCCOORD_TO_MAPCOORD(p_clsthing->X), p_clsthing->Y,
      PRCCOORD_TO_MAPCOORD(p_clsthing->Z), p_clsthing->StartFrame + 1, p_clsthing->Timer1);
    p_sthing = &sthings[thing];
    p_sthing->U.UEffect.VZ = p_clsthing->U.UEffect.VZ;
    frame = p_sthing->StartFrame;
    if (frame != 999 && frame != 1002 && frame != 1004 && frame != 1008 &&
      frame != 1032 && frame != 1037 && frame != 1038 && frame != 1050) {
        p_sthing->SubType = 2;
    } else {
        p_sthing->SubType = 1;
        p_sthing->Radius = 128;
    }
    return thing;
}

ThingIdx new_thing_building_clone(struct Thing *p_clthing, struct M33 *p_clmatx, short shut_h)
{
    struct Thing *p_thing;
    struct SingleObject *p_sobj;
    int i;

    p_thing = create_building_thing(PRCCOORD_TO_MAPCOORD(p_clthing->X), p_clthing->Y,
      PRCCOORD_TO_MAPCOORD(p_clthing->Z), p_clthing->U.UObject.Object,
      p_clthing->U.UObject.NumbObjects, p_clthing->ThingOffset);

    p_thing->ThingOffset = p_clthing->ThingOffset;
    p_thing->Flag = p_clthing->Flag;
    p_thing->VX = p_clthing->VX;
    p_thing->VY = p_clthing->VY;
    p_thing->VZ = p_clthing->VZ;
    p_thing->SubType = p_clthing->SubType;

    ubyte styp;
    styp = p_thing->SubType;

    if (styp == SubTT_BLD_MGUN)
    {
        p_thing->U.UMGun.PathIndex = p_clthing->U.UMGun.PathIndex;
        p_thing->U.UMGun.UniqueID = p_clthing->U.UMGun.UniqueID;
        p_thing->U.UMGun.NextThing = p_clthing->U.UMGun.NextThing;
        p_thing->U.UMGun.PrevThing = p_clthing->U.UMGun.PrevThing;
        p_thing->U.UMGun.Token = p_clthing->U.UMGun.Token;
        p_thing->U.UMGun.TokenDir = p_clthing->U.UMGun.TokenDir;
        p_thing->U.UMGun.ObjectNo = p_clthing->U.UMGun.ObjectNo;
        p_thing->U.UMGun.CurrentWeapon = p_clthing->U.UMGun.CurrentWeapon;
    }
    else
    {
        p_thing->U.UObject.Token = p_clthing->U.UObject.Token;
        p_thing->U.UObject.TokenDir = p_clthing->U.UObject.TokenDir;
        p_thing->U.UObject.NextThing = p_clthing->U.UObject.NextThing;
        p_thing->U.UObject.PrevThing = p_clthing->U.UObject.PrevThing;
        p_thing->U.UObject.OffX = p_clthing->U.UObject.OffX;
        p_thing->U.UObject.OffZ = p_clthing->U.UObject.OffZ;
        p_thing->U.UObject.BuildStartVect = p_clthing->U.UObject.BuildStartVect;
        p_thing->U.UObject.BuildNumbVect = p_clthing->U.UObject.BuildNumbVect;
        // Copy all fields from Turn down
        p_thing->U.UObject.Turn = p_clthing->U.UObject.Turn;
        p_thing->U.UObject.TurnPadOnPS = p_clthing->U.UObject.TurnPadOnPS;
        for (i = 0; i < 4; i++) {
            p_thing->U.UObject.tnode[i] = p_clthing->U.UObject.tnode[i];
        }
        p_thing->U.UObject.player_in_me = p_clthing->U.UObject.player_in_me;
        p_thing->U.UObject.unkn_4D = p_clthing->U.UObject.unkn_4D;
        p_thing->U.UObject.DrawTurn = p_clthing->U.UObject.DrawTurn;
        for (i = 0; i < 4; i++) {
            p_thing->U.UObject.tnode_50[i] = p_clthing->U.UObject.tnode_50[i];
        }
    }

    p_sobj = &game_objects[p_clthing->U.UObject.Object];
    p_thing->U.UObject.MinY[0] = p_sobj->OffsetY - 500;
    p_thing->U.UObject.MaxY[0] = p_sobj->OffsetY;
    
    if (styp == SubTT_BLD_SHUTLDR)
    {
        if (((p_thing->Flag & TngF_Unkn0001) == 0)
          && (p_thing->U.UObject.PrevThing == 0 || p_thing->U.UObject.NextThing == 0))
        {
            p_thing->VX = p_thing->X >> 16;
            p_thing->VZ = p_thing->Z >> 16;
            p_thing->Flag |=  TngF_Unkn0001;
        }
        if (shut_h < 15)
            p_thing->Y >>= 3;
    }
    else if (styp == SubTT_BLD_MGUN)
    {
        assert(next_local_mat < LOCAL_MATS_COUNT);
        p_thing->U.UMGun.MatrixIndex = next_local_mat++;
        if (p_clmatx != NULL)
            memcpy(&local_mats[p_thing->U.UMGun.MatrixIndex], p_clmatx, sizeof(struct M33));
        else
            matrix_identity_fill(&local_mats[p_thing->U.UMGun.MatrixIndex]);
        p_thing->U.UMGun.AngleX = 1024;
        p_thing->U.UMGun.AngleY = 0;
        p_thing->Radius = 256;
        p_thing->U.UMGun.RecoilTimer = 0;
        p_thing->U.UMGun.MaxHealth = 6000;
        p_thing->U.UMGun.RecoilTimer = 0;
        p_thing->Health = p_thing->U.UMGun.MaxHealth;
    }
    else if (styp >= SubTT_BLD_MOVN_ROTOR && styp <= SubTT_BLD_37)
    {
        assert(next_local_mat < LOCAL_MATS_COUNT);
        p_thing->U.UObject.MatrixIndex = next_local_mat++;
        if (p_clmatx != NULL)
            memcpy(&local_mats[p_thing->U.UObject.MatrixIndex], p_clmatx, sizeof(struct M33));
        else
            matrix_identity_fill(&local_mats[p_thing->U.UObject.MatrixIndex]);
        p_thing->Flag |=  TngF_Unkn1000;
    }
    p_thing->SubState = p_clthing->SubState;
    p_thing->Timer1 = p_clthing->Timer1;

    // Should be have a separate UGate struct?
    if (styp >= SubTT_BLD_GATE && styp <= SubTT_BLD_26
      && p_thing->U.UObject.MinY[0] == *(ushort *)&p_thing->U.UObject.Group )
    {
        p_thing->U.UObject.MinY[0] = -500;
        p_thing->U.UObject.MaxY[0] = 0;
    }
    p_thing->State = p_clthing->State;
    if (p_thing->State == 9)
        p_thing->State = 0;
    p_thing->Frame = p_clthing->Frame;

    return p_thing->ThingOffset;
}

/** Maps fields from old Thing struct to the current one.
 */
void refresh_old_thing_format(struct Thing *p_thing, struct ThingOldV9 *p_oldthing, u32 fmtver)
{
    ushort len;

    LbMemorySet(p_thing, 0, sizeof(struct Thing));

    p_thing->Parent = p_oldthing->Parent;
    p_thing->Next = p_oldthing->Next;
    p_thing->LinkParent = p_oldthing->LinkParent;
    p_thing->LinkChild = p_oldthing->LinkChild;
    p_thing->SubType = p_oldthing->SubType;
    p_thing->Type = p_oldthing->Type;
    p_thing->State = p_oldthing->State;
    p_thing->Flag = p_oldthing->Flag;
    p_thing->LinkSame = p_oldthing->LinkSame;
    p_thing->Radius = p_oldthing->Radius;
    p_thing->X = p_oldthing->X;
    p_thing->Y = p_oldthing->Y;
    p_thing->Z = p_oldthing->Z;
    p_thing->Frame = p_oldthing->Frame;
    p_thing->StartFrame = p_oldthing->StartFrame;
    p_thing->Timer1 = p_oldthing->Timer1;
    p_thing->StartTimer1 = p_oldthing->StartTimer1;
    p_thing->LinkSameGroup = p_oldthing->LinkSameGroup;
    p_thing->ThingOffset = p_oldthing->ThingOffset;
    p_thing->VX = p_oldthing->VX;
    p_thing->VY = p_oldthing->VY;
    p_thing->VZ = p_oldthing->VZ;
    p_thing->Speed = p_oldthing->Speed;
    p_thing->Health = p_oldthing->Health;
    p_thing->Owner = p_oldthing->Owner;
    p_thing->PathOffset = p_oldthing->PathOffset;
    // The target pointer will be cleared anyway
    p_thing->PTarget = NULL; // p_oldthing->PTarget;
    p_thing->GotoThingIndex = p_oldthing->GotoThingIndex;
    p_thing->SubState = p_oldthing->SubState;

    // Type-dependent fields
    if (p_thing->Type == TT_PERSON)
    {
        p_thing->U.UPerson.AnimMode = p_oldthing->PersonAnimMode;
        p_thing->U.UPerson.OldAnimMode = p_oldthing->PersonOldAnimMode;
        p_thing->U.UPerson.Timer2 = p_oldthing->Timer2;
        p_thing->U.UPerson.StartTimer2 = p_oldthing->StartTimer2;
        p_thing->U.UPerson.Angle = p_oldthing->PersonAngle;
        p_thing->U.UPerson.GotoX = p_oldthing->PersonGotoX;
        p_thing->U.UPerson.GotoZ = p_oldthing->PersonGotoZ;
        p_thing->U.UPerson.Group = p_oldthing->PersonGroup;
        p_thing->U.UPerson.EffectiveGroup = p_oldthing->PersonEffectiveGroup;
        p_thing->U.UPerson.WeaponsCarried = p_oldthing->PersonWeaponsCarried;
        p_thing->U.UPerson.CurrentWeapon = p_oldthing->PersonCurrentWeapon;
        p_thing->U.UPerson.ComHead = p_oldthing->PersonComHead;
        p_thing->U.UPerson.ComCur = p_oldthing->PersonComCur;
        p_thing->U.UPerson.ComTimer = p_oldthing->PersonComTimer;
        p_thing->U.UPerson.Brightness = p_oldthing->PersonBrightness;
        p_thing->U.UPerson.MaxShieldEnergy = p_oldthing->PersonMaxShieldEnergy;
        p_thing->U.UPerson.UniqueID = p_oldthing->PersonUniqueID;
        p_thing->U.UPerson.PathIndex = p_oldthing->PersonPathIndex;
        p_thing->U.UPerson.WeaponTimer = p_oldthing->PersonWeaponTimer;
        p_thing->U.UPerson.WeaponTurn = p_oldthing->PersonWeaponTurn;
        p_thing->U.UPerson.Energy = p_oldthing->PersonEnergy;
        p_thing->U.UPerson.MaxEnergy = p_oldthing->PersonMaxEnergy;
        // Uncertain fields
        p_thing->U.UPerson.ShieldEnergy = p_oldthing->PersonShieldEnergy;
        p_thing->U.UPerson.SpecialTimer = p_oldthing->PersonSpecialTimer;
        p_thing->U.UPerson.ComRange = p_oldthing->PersonComRange;
        p_thing->U.UPerson.BumpMode = p_oldthing->PersonBumpMode;
        p_thing->U.UPerson.BumpCount = p_oldthing->PersonBumpCount;
        p_thing->U.UPerson.Vehicle = p_oldthing->PersonVehicle;
        p_thing->U.UPerson.LinkPassenger = p_oldthing->PersonLinkPassenger;
        p_thing->U.UPerson.Within = p_oldthing->PersonWithin;
        p_thing->U.UPerson.LastDist = p_oldthing->PersonLastDist;
        p_thing->U.UPerson.OnFace = p_oldthing->PersonOnFace;
        // End of uncertain fields
        p_thing->U.UPerson.ComRange = p_oldthing->PersonComRange;
        p_thing->U.UPerson.Shadows[0] = p_oldthing->PersonShadows[0];
        p_thing->U.UPerson.Shadows[1] = p_oldthing->PersonShadows[1];
        p_thing->U.UPerson.Shadows[2] = p_oldthing->PersonShadows[2];
        p_thing->U.UPerson.Shadows[3] = p_oldthing->PersonShadows[3];
        p_thing->U.UPerson.RecoilTimer = p_oldthing->PersonRecoilTimer;
        p_thing->U.UPerson.MaxHealth = p_oldthing->PersonMaxHealth;
        p_thing->U.UPerson.RecoilDir = p_oldthing->PersonRecoilDir;
        // Having cybernetic mods is kind of important; but we do not have location of these
        //p_thing->U.UPerson.UMod.Mods = p_oldthing->?;
        if (fmtver >= 12) {
            p_thing->U.UPerson.Stamina = p_oldthing->PersonStamina;
            p_thing->U.UPerson.MaxStamina = p_oldthing->PersonMaxStamina;
        }
        // Field FrameId
        if (p_thing->SubType == SubTT_PERS_PUNK_F) {
            // Randomize hair color - 50% normal red(0), 25% blonde(1), 25% blue(2)
            // The ThingOffset should be random enough
            len = (p_thing->ThingOffset ^ p_thing->U.UPerson.UniqueID) & 0xF;
            if (len < 4)
                p_thing->U.UPerson.FrameId.Version[0] = 1;
            else if (len < 8)
                p_thing->U.UPerson.FrameId.Version[0] = 2;
        }
        if ((fmtver < 8) && (p_thing->U.UPerson.Group == 0)) {
            // Some very old formats may have effective group set and normal zeroed out
            p_thing->U.UPerson.Group = p_thing->U.UPerson.EffectiveGroup;
        }
        // The current weapon should always be in a list of carried wepons (issue mostly for fmtver < 5)
        if (p_thing->U.UPerson.CurrentWeapon > 0)
            p_thing->U.UPerson.WeaponsCarried |= (1 << (p_thing->U.UPerson.CurrentWeapon - 1));
    }
    else if (p_thing->Type == TT_VEHICLE)
    {
        p_thing->U.UVehicle.Object = p_oldthing->VehicleObject;
        p_thing->U.UVehicle.NumbObjects = p_oldthing->VehicleNumbObjects;
        p_thing->U.UVehicle.Timer2 = p_oldthing->Timer2;
        p_thing->U.UVehicle.AngleX = p_oldthing->VehicleAngleX;
        p_thing->U.UVehicle.AngleY = p_oldthing->VehicleAngleY;
        p_thing->U.UVehicle.AngleZ = p_oldthing->VehicleAngleZ;
        p_thing->U.UVehicle.GotoX = p_oldthing->VehicleGotoX;
        p_thing->U.UVehicle.GotoY = p_oldthing->VehicleGotoY;
        p_thing->U.UVehicle.GotoZ = p_oldthing->VehicleGotoZ;
        p_thing->U.UVehicle.MatrixIndex = p_oldthing->VehicleMatrixIndex;
        p_thing->U.UVehicle.UniqueID = p_oldthing->VehicleUniqueID;
        p_thing->U.UVehicle.MaxSpeed = p_oldthing->VehicleReqdSpeed;
        p_thing->U.UVehicle.ReqdSpeed = p_oldthing->VehicleReqdSpeed;
        p_thing->U.UVehicle.PassengerHead = p_oldthing->VehiclePassengerHead;
        p_thing->U.UVehicle.TNode = p_oldthing->VehicleTNode;
        p_thing->U.UVehicle.AngleDY = p_oldthing->VehicleAngleDY;
        p_thing->U.UVehicle.RecoilTimer = p_oldthing->VehicleRecoilTimer;
        // Only one group seem to exist in the old vehicles
        p_thing->U.UVehicle.Group = p_oldthing->VehicleEffectiveGroup;
        p_thing->U.UVehicle.EffectiveGroup = p_oldthing->VehicleEffectiveGroup;
        // In old format, MaxHealth is stored in additional vehicle block, not in the thing
        //p_thing->U.UVehicle.MaxHealth = ?;
    }
    else if (p_thing->Type == TT_BUILDING)
    {
        p_thing->U.UObject.Object = p_oldthing->ObjectObject;
        p_thing->U.UObject.NumbObjects = p_oldthing->ObjectNumbObjects;
        p_thing->U.UObject.Timer[0] = p_oldthing->ObjectTimer0;
        p_thing->U.UObject.Timer[1] = p_oldthing->ObjectTimer1;
        p_thing->U.UObject.Angle = p_oldthing->ObjectAngle;
        p_thing->U.UObject.TargetDX = p_oldthing->ObjectTargetDX;
        p_thing->U.UObject.TargetDY = p_oldthing->ObjectTargetDY;
        p_thing->U.UObject.TargetDZ = p_oldthing->ObjectTargetDZ;
        // Only one group seem to exist in the old Objects
        p_thing->U.UObject.Group = p_oldthing->ObjectEffectiveGroup;
        p_thing->U.UObject.EffectiveGroup = p_oldthing->ObjectEffectiveGroup;
        if (p_thing->SubType == SubTT_BLD_GATE) {
            p_thing->U.UObject.RaiseDY[0] = p_oldthing->ObjectRaiseDY[0];
            p_thing->U.UObject.RaiseDY[1] = p_oldthing->ObjectRaiseDY[1];
            p_thing->VX = 0;
            p_thing->VY = 0;
            p_thing->VZ = 0;
            p_thing->U.UObject.MinY[0] = p_oldthing->ObjectMinY0;
            p_thing->U.UObject.RaiseY[1] = p_oldthing->ObjectRaiseY1;
            p_thing->Frame = 0;
            p_thing->StartFrame = 0;
        }
        else if (p_thing->SubType == SubTT_BLD_MGUN) {
            p_thing->U.UMGun.Object = p_oldthing->MGunObject;
            p_thing->U.UMGun.NumbObjects = p_oldthing->MGunNumbObjects;
            p_thing->U.UMGun.AngleX = p_oldthing->MGunAngleX;
            p_thing->U.UMGun.AngleY = p_oldthing->MGunAngleY;
            p_thing->U.UMGun.AngleZ = p_oldthing->MGunAngleZ;
            p_thing->U.UMGun.GotoX = p_oldthing->MGunGotoX;
            p_thing->U.UMGun.GotoY = p_oldthing->MGunGotoY;
            p_thing->U.UMGun.GotoZ = p_oldthing->MGunGotoZ;
            p_thing->U.UMGun.UniqueID = p_oldthing->MGunUniqueID;
            // Only one group seem to exist in the old MGun
            p_thing->U.UMGun.Group = p_oldthing->MGunEffectiveGroup;
            p_thing->U.UMGun.EffectiveGroup = p_oldthing->MGunEffectiveGroup;
        }
    }
}

struct SimpleThing *create_scale_effect(int x, int y, int z, ushort frame, int timer)
{
    struct SimpleThing *ret;
    asm volatile (
      "push %5\n"
      "call ASM_create_scale_effect\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z), "c" (frame), "g" (timer));
    return ret;
}

struct SimpleThing *create_sound_effect(int x, int y, int z, ushort sample, int vol, int loop)
{
    struct SimpleThing *ret;
    asm volatile (
      "push %6\n"
      "push %5\n"
      "call ASM_create_sound_effect\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z), "c" (sample), "g" (vol), "g" (loop));
    return ret;
}

void mine_detonate(struct Thing *p_thing)
{
    play_dist_sample(p_thing, 37, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
    bang_new4(p_thing->X, p_thing->Y, p_thing->Z, 20);
    p_thing->State = 13;
    p_thing->Flag |= TngF_Destroyed;
    set_thing_frame(p_thing, 1069);
}

int mine_hit_by_bullet(struct Thing *p_thing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type)
{
    if (p_thing->State == 13) {
        return 0;
    }
    p_thing->Health -= hp;
    if (p_thing->Health <= 0) {
        mine_detonate(p_thing);
        return p_thing->Health + hp;
    }
    return 0;
}

int static_hit_by_bullet(struct SimpleThing *p_sthing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type)
{
    p_sthing->U.UScenery.Health -= hp;
    if (p_sthing->U.UScenery.Health <= 0) {
        set_static_on_fire(p_sthing);
    }
    return 100;
}

struct Thing *replace_thing_given_thing_idx(int x, int y, int z, short owner, ThingIdx pv_thing)
{
    struct Thing *p_pv_thing;
    struct Thing *p_nx_thing;

    p_pv_thing = &things[pv_thing];
    if (on_mapwho(p_pv_thing))
        delete_node(p_pv_thing);
    unkn01_thing_idx = pv_thing;
    p_nx_thing = new_sim_person(x, y, z, 101u);
    p_nx_thing->State = 0;
    p_nx_thing->Owner = owner;
    return p_nx_thing;
}

/******************************************************************************/
