/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file vehicle.c
 *     Routines implementing vehicle.
 * @par Purpose:
 *     Implement functions for handling vehicle things.
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
#include "vehicle.h"

#include "bfmath.h"
#include "bfmemut.h"
#include "bfutility.h"
#include "ssampply.h"
#include <assert.h>
#include <stdlib.h>

#include "bigmap.h"
#include "bmbang.h"
#include "enginpriobjs.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintrns.h"
#include "game.h"
#include "game_speed.h"
#include "matrix.h"
#include "pathtrig.h"
#include "sound.h"
#include "thing.h"
#include "swlog.h"
#include "weapon.h"
/******************************************************************************/

#pragma pack(1)

struct CarGlare { // sizeof=7
    short Dix;
    short Diy;
    short Diz;
    ubyte Flag;
};

#pragma pack()

/** Configuration options for each person state.
 */
struct VehStateConfig vehicle_states[] = {
  {"NONE_STATE",},
  {"UNKN_1",},
  {"UNKN_2",},
  {"UNKN_3",},
  {"UNKN_4",},
  {"UNKN_5",},
  {"UNKN_6",},
  {"UNKN_7",},
  {"UNKN_8",},
  {"UNKN_9",},
  {"UNKN_A",},
  {"UNKN_B",},
  {"UNKN_C",},
  {"UNKN_D",},
  {"UNKN_E",},
  {"UNKN_F",},
  {"UNKN_10",},
  {"WANDER",},
  {"UNKN_12",},
  {"UNKN_13",},
  {"UNKN_14",},
  {"UNKN_15",},
  {"UNKN_16",},
  {"UNKN_17",},
  {"UNKN_18",},
  {"UNKN_19",},
  {"UNKN_1A",},
  {"UNKN_1B",},
  {"UNKN_1C",},
  {"UNKN_1D",},
  {"UNKN_1E",},
  {"UNKN_1F",},
  {"UNKN_20",},
  {"PARKED_PARAL",},
  {"UNKN_22",},
  {"UNKN_23",},
  {"UNKN_24",},
  {"UNKN_25",},
  {"UNKN_26",},
  {"UNKN_27",},
  {"UNKN_28",},
  {"UNKN_29",},
  {"UNKN_2A",},
  {"UNKN_2B",},
  {"UNKN_2C",},
  {"UNKN_2D",},
  {"UNKN_2E",},
  {"UNKN_2F",},
  {"UNKN_30",},
  {"UNKN_31",},
  {"GOTO_LOC",},
  {"UNKN_33",},
  {"UNKN_34",},
  {"UNKN_35",},
  {"UNKN_36",},
  {"UNKN_37",},
  {"UNKN_38",},
  {"UNKN_39",},
  {"UNKN_3A",},
  {"FLY_LANDING",},
  {"UNKN_3C",},
  {"PARKED_PERPN",},
  {"UNKN_3E",},
  {"UNKN_3F",},
  {"UNKN_40",},
  {"UNKN_41",},
  {"UNKN_42",},
  {"UNKN_43",},
  {"UNKN_44",},
  {"UNKN_45",},
  {"UNKN_46",},
  {"UNKN_47",},
  {"UNKN_48",},
  {"UNKN_49",},
  {"UNKN_4A",},
  {"UNKN_4B",},
  {"UNKN_4C",},
  {"UNKN_4D",},
  {"UNKN_4E",},
  {"UNKN_4F",},
  {"UNKN_50",},
  {"UNKN_51",},
  {"UNKN_52",},
};

struct CarGlare car_glare[] = {
  {0, 16, 336, 0},
  {-96, -64, 400, 0},
  {128, -64, 400, 0},
  {0, -96, 304, 0},
  {-208, -32, -240, 0},
  {208, -32, -240, 0},
  {128, -144, 240, 0},
  {-112, -144, 240, 0},
  {96, -32, 304, 0},
  {-80, -32, 304, 0},
  {96, -96, 528, 0},
  {-80, -96, 528, 0},
  {-128, -16, 288, 1},
  {144, -16, 288, 2},
  {112, -112, -96, 2},
  {-96, -112, -96, 1},
  {-80, -368, 48, 1},
  {96, -368, 48, 2},
  {-32, -64, 160, 0},
  {112, -16, 304, 0},
  {-96, -16, 304, 0},
  {-96, -224, 224, 1},
  {80, -224, 224, 2},
  {-96, -224, -288, 2},
  {80, -224, -288, 1},
  {-144, 0, 224, 1},
  {176, 0, 224, 2},
  {-112, -32, 448, 0},
  {144, -32, 448, 0},
  {-224, -176, 576, 0},
  {224, -176, 576, 0},
  {0, 16, 336, 0},
  {-96, -64, 400, 0},
  {128, -64, 400, 0},
  {-160, -672, -688, 0},
  {160, -672, -688, 0},
  {-112, -32, 448, 0},
  {144, -32, 448, 0},
  {0, -96, 304, 0},
  {112, -16, 304, 0},
  {-96, -16, 304, 0},
};

const char *vehicle_type_name(ushort vtype)
{
#if 0
// Enable when we have vehicle config file with names
    struct VehicleDef *p_vhdef;

    p_vhdef = &vehicle_def[vtype];
    if (strlen(p_vhdef->Name) == 0)
        return "OUTRNG_VEHICLE";
    return p_vhdef->Name;
#endif
    switch (vtype)
    {
    case SubTT_VEH_TRAIN:
        return "TRAIN";
    case SubTT_VEH_GROUND:
        return "VEH_GROUND";
    case SubTT_VEH_SHUTTLE_POD:
        return "SHUTTLE_POD";
    case SubTT_VEH_FLYING:
        return "VEH_FLYING";
    case SubTT_VEH_TANK:
        return "TANK";
    case SubTT_VEH_SHIP:
        return "SHIP";
    case SubTT_VEH_MECH:
        return "MECH";
    default:
        break;
    }
    return "VEHICLE";
}

void snprint_vehicle_state(char *buf, ulong buflen, struct Thing *p_thing)
{
    char *s;
    //ubyte nparams;
    struct VehStateConfig *p_vstatcfg;

    p_vstatcfg = &vehicle_states[p_thing->State];
    s = buf;

    sprintf(s, "%s( ", p_vstatcfg->Name);
    s += strlen(s);
    //nparams = 0;

    // TODO support parameters of states

    snprintf(s, buflen - (s-buf), " )");
}


void init_mech(void)
{
    asm volatile ("call ASM_init_mech\n"
        :  :  : "eax" );
}

TbBool vehicle_is_destroyed(ThingIdx thing)
{
    struct Thing *p_thing;

    if (thing <= 0)
        return false;

    p_thing = &things[thing];
    return thing_is_destroyed(thing) || (p_thing->Type != TT_VEHICLE);
}

void mech_unkn_func_02(void)
{
    asm volatile ("call ASM_mech_unkn_func_02\n"
        :  :  : "eax" );
}

void mech_unkn_func_09(ThingIdx thing)
{
    asm volatile ("call ASM_mech_unkn_func_09\n"
        : : "a" (thing));
}

void veh_add(struct Thing *p_vehicle, short frame)
{
#if 0
    asm volatile ("call ASM_veh_add\n"
        : : "a" (p_vehicle), "d" (frame));
    return;
#endif
    struct Thing *p_mgun;
    struct SingleObject *p_snobj;
    short coord_x, coord_y, coord_z;
    short mgun;
    ushort snobj;
    ushort matx;

    switch (frame)
    {
    case VehOM_MBTANK:
        mgun = get_new_thing();
        if (mgun <= 0) {
            LOGERR("No thing slots for mounted gun");
            break;
        }
        p_vehicle->U.UVehicle.SubThing = mgun;
        p_vehicle->SubType = SubTT_VEH_TANK;

        p_mgun = &things[mgun];
        p_mgun->U.UMGun.NextThing = 0;
        p_mgun->U.UVehicle.SubThing = 0;
        assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
        assert(next_local_mat < LOCAL_MATS_COUNT);
        matx = next_local_mat++;
        LbMemoryCopy(&local_mats[matx], &local_mats[p_vehicle->U.UVehicle.MatrixIndex], sizeof(struct M33));
        p_mgun->U.UMGun.MatrixIndex = matx;
        p_mgun->StartFrame = frame;

        coord_x = PRCCOORD_TO_MAPCOORD(p_vehicle->X);
        coord_y = PRCCOORD_TO_MAPCOORD(p_vehicle->Y);
        coord_z = PRCCOORD_TO_MAPCOORD(p_vehicle->Z);
        byte_1C83D1 = 0;
        snobj = copy_prim_obj_to_game_object(coord_x, coord_z, -19 - prim_unknprop01, coord_y + 20);
        p_mgun->X = 0;
        p_mgun->Y = 0x2800;
        p_mgun->Z = 0;
        unkn_object_shift_03(snobj);
        p_mgun->Type = TT_VEH_TURRET;
        p_mgun->U.UMGun.Object = snobj;
        p_snobj = &game_objects[snobj];
        p_snobj->ThingNo = p_mgun - things;
        break;
    case 26: // Replaced tank (object slots reused for mech)
        mgun = get_new_thing();
        if (mgun <= 0) {
            LOGERR("No thing slots for mounted gun");
            break;
        }
        p_vehicle->U.UVehicle.SubThing = mgun;
        p_vehicle->SubType = SubTT_VEH_TANK;

        p_mgun = &things[mgun];
        p_mgun->U.UMGun.NextThing = 0;
        p_mgun->U.UVehicle.SubThing = 0;
        assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
        assert(next_local_mat < LOCAL_MATS_COUNT);
        matx = next_local_mat++;
        LbMemoryCopy(&local_mats[matx], &local_mats[p_vehicle->U.UVehicle.MatrixIndex], sizeof(struct M33));
        p_mgun->U.UMGun.MatrixIndex = matx;
        p_mgun->StartFrame = frame;

        coord_x = PRCCOORD_TO_MAPCOORD(p_vehicle->X);
        coord_y = PRCCOORD_TO_MAPCOORD(p_vehicle->Y);
        coord_z = PRCCOORD_TO_MAPCOORD(p_vehicle->Z);
        byte_1C83D1 = 0;
        snobj = copy_prim_obj_to_game_object(coord_x, coord_z, -27 - prim_unknprop01, coord_y + 20);
        p_mgun->X = 0;
        p_mgun->Y = 0x1E00;
        p_mgun->Z = 0;
        unkn_object_shift_03(snobj);
        p_mgun->Type = TT_VEH_TURRET;
        p_mgun->U.UMGun.Object = snobj;
        p_snobj = &game_objects[snobj];
        p_snobj->ThingNo = p_mgun - things;
        break;
    case VehOM_CLAWTANK:
        mgun = get_new_thing();
        if (mgun <= 0) {
            LOGERR("No thing slots for mounted gun");
            break;
        }
        p_vehicle->U.UVehicle.SubThing = mgun;
        p_vehicle->SubType = SubTT_VEH_TANK;

        p_mgun = &things[mgun];
        p_mgun->U.UMGun.NextThing = 0;
        p_mgun->U.UVehicle.SubThing = 0;
        assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
        assert(next_local_mat < LOCAL_MATS_COUNT);
        matx = next_local_mat++;
        LbMemoryCopy(&local_mats[matx], &local_mats[p_vehicle->U.UVehicle.MatrixIndex], sizeof(struct M33));
        p_mgun->U.UMGun.MatrixIndex = matx;
        p_mgun->StartFrame = frame;

        byte_1C83D1 = 0;
        coord_x = PRCCOORD_TO_MAPCOORD(p_vehicle->X);
        coord_y = PRCCOORD_TO_MAPCOORD(p_vehicle->Y);
        coord_z = PRCCOORD_TO_MAPCOORD(p_vehicle->Z);
        snobj = copy_prim_obj_to_game_object(coord_x, coord_z, -30 - prim_unknprop01, coord_y + 20);
        p_mgun->X = 0;
        p_mgun->Y = 0x1E00;
        p_mgun->Z = 0;
        unkn_object_shift_03(snobj);
        p_mgun->Type = TT_VEH_TURRET;
        p_mgun->U.UMGun.Object = snobj;
        p_snobj = &game_objects[snobj];
        p_snobj->ThingNo = p_mgun - things;
        break;
    default:
        p_vehicle->U.UVehicle.SubThing = 0;
        break;
    }
    p_vehicle->U.UVehicle.WobbleZP = 0;
    p_vehicle->U.UVehicle.WobbleZV = 0;
}

ushort veh_passenger_count(struct Thing *p_veh)
{
    struct Thing *p_thing;
    ThingIdx thing;
    ushort c;

    c = 0;

    thing = p_veh->U.UVehicle.PassengerHead;
    while (thing > 0 && c < THINGS_LIMIT)
    {
        c++;
        p_thing = &things[thing];
        thing = p_thing->U.UPerson.LinkPassenger;
    }
    return c;
}

void VNAV_unkn_func_207(struct Thing *p_thing)
{
    asm volatile ("call ASM_VNAV_unkn_func_207\n"
        : : "a" (p_thing));
}

int check_for_a_vehicle_here(int x, int z, struct Thing *p_vehicle)
{
    int ret;
    asm volatile (
      "call ASM_check_for_a_vehicle_here\n"
        : "=r" (ret) : "a" (x), "d" (z), "b" (p_vehicle));
    return ret;
}

void person_hit_by_car(struct Thing *p_person, struct Thing *p_vehicle)
{
    asm volatile ("call ASM_person_hit_by_car\n"
        : : "a" (p_person), "d" (p_vehicle));
}

/** Checks if the vehicle collides with another object, provided as separate params.
 */
static TbBool vehicle_check_collide_with_area(struct Thing *p_vehA,
  int posBx, int posBy, int posBz, int radB, int matBid)
{
    struct M33 *p_matA;
    int radA_incl_speed, matA02, matA22;
    int futureAx, futureAz;
    int posAx, posAz;
    struct M33 *p_matB;
    int radB_incl_speed, matB02, matB22;
    int futureBx, futureBz;
    int posB_beg_x, posB_beg_z;
    int posB_end_x, posB_end_z;
    int dist_begX, dist_begZ;
    int dist_endX, dist_endZ;
    int distXZ_sq;

    assert(p_vehA->U.UVehicle.MatrixIndex < next_local_mat);
    p_matA = &local_mats[p_vehA->U.UVehicle.MatrixIndex];
    matA02 = -4 * p_matA->R[0][2];
    matA22 = -4 * p_matA->R[2][2];
    radA_incl_speed = (386 * p_vehA->Radius >> 8)
      + (p_vehA->Speed >> 3) + (p_vehA->Speed >> 4);
    futureAx = radA_incl_speed * matA02 >> 8;
    futureAz = radA_incl_speed * matA22 >> 8;

    posAx = p_vehA->X + futureAx - (matA22 >> 2);
    posAz = p_vehA->Z + futureAz + (matA02 >> 2);

    if (abs(posBy - p_vehA->Y) >= 2048)
        return false;

    if ((p_vehA->X >> 16 == posBx >> 16) && (p_vehA->Z >> 16 == posBz >> 16))
        return true;

    assert(matBid < next_local_mat);
    p_matB = &local_mats[matBid];
    matB02 = -4 * p_matB->R[0][2];
    matB22 = -4 * p_matB->R[2][2];
    radB_incl_speed = (384 * radB >> 9);
    futureBx = radB_incl_speed * matB02 >> 8;
    futureBz = radB_incl_speed * matB22 >> 8;

    posB_beg_x = posBx - futureBx;
    posB_beg_z = posBz - futureBz;
    posB_end_x = posBx + futureBx;
    posB_end_z = posBz + futureBz;

    dist_begX = (posB_beg_x - posAx) >> 8;
    dist_begZ = (posB_beg_z - posAz) >> 8;
    dist_endX = (posB_end_x - posAx) >> 8;
    dist_endZ = (posB_end_z - posAz) >> 8;

    distXZ_sq = dist_endZ * dist_endZ + dist_endX * dist_endX;
    if (distXZ_sq >= dist_begZ * dist_begZ + dist_begX * dist_begX)
        distXZ_sq = dist_begZ * dist_begZ + dist_begX * dist_begX;

    if (distXZ_sq >= radB_incl_speed * radB_incl_speed)
        return false;

    return true;
}

/** Check if the vehicle is stopped due to state of tnode it rides on.
 */
TbBool vehicle_stopped_on_tnode(struct Thing *p_vehicle)
{
    if (p_vehicle->U.UVehicle.TNode >= 0)
        return false;

    // Repeat the stop conditions from process_vehicle_stop_for_pedestrians() exactly
    if ((p_vehicle->U.UVehicle.WorkPlace & VWPFlg_Unkn0008) != 0)
    {
        if (p_vehicle->State == VehSt_GOTO_LOC || p_vehicle->State == VehSt_WANDER)
        {
            struct TrafficNode *p_tnode;
            p_tnode = &game_traffic_nodes[-p_vehicle->U.UVehicle.TNode];
            if ((p_tnode->Flags & TNdF_Unkn0040) != 0)
            {
                if (p_tnode->GateLink == p_vehicle->ThingOffset)
                    return false;
                return true;
            }
        }
    }
    return false;
}

/** Check if the second vehicle is in the way of first vehicle.
 */
TbBool check_two_vehicles(struct Thing *p_vehA, struct Thing *p_vehB)
{
#if 0
    TbBool ret;
    asm volatile (
      "call ASM_check_two_vehicles\n"
        : "=r" (ret) : "a" (p_vehA), "d" (p_vehB));
    return ret;
#endif
    if (abs(p_vehA->U.UVehicle.AngleDY) > 20)
        return false;

    if (p_vehB->ThingOffset == p_vehA->ThingOffset)
        return false;

    if ((p_vehB->Flag & TngF_Unkn0004) != 0)
        return false;

    return vehicle_check_collide_with_area(p_vehA, p_vehB->X, p_vehB->Y, p_vehB->Z,
      p_vehB->Radius, p_vehB->U.UVehicle.MatrixIndex);
}

TbBool vehicles_check_second_is_blocked(struct Thing *p_vehA, struct Thing *p_vehB)
{
    if (!vehicle_stopped_on_tnode(p_vehB))
        return false;

    // Check if the vehicles are riding on the same TNode, or the second has Agok node set
    if ((p_vehA->U.UVehicle.TNode == p_vehB->U.UVehicle.TNode)
     || ((p_vehB->U.UVehicle.WorkPlace & VWPFlg_HasAgok) != 0))
    {
        // If one of the vehicles already claimed the TNode, it should go first
        struct TrafficNode *p_tnode;
        p_tnode = &game_traffic_nodes[-p_vehB->U.UVehicle.TNode];
        if (p_tnode->GateLink == p_vehA->ThingOffset)
            return true;
    }
    return false;
}

/** Of two competing (potentially colliding) vehicles, check if the first one has right-of-way to move forward.
 */
TbBool vehicle_first_has_right_of_way(struct Thing *p_vehA, struct Thing *p_vehB)
{
    // If no special circumstances, allow the one with higher ThingOffset first
    return (p_vehA->ThingOffset >= p_vehB->ThingOffset);
}

/** Collision check between two vehicles.
 */
static TbBool check_vehicle_col_with_veh(struct Thing *p_vehA, struct Thing *p_vehB,
  int posAx, int posAy, int posAz)
{
    struct M33 *p_matB;
    int radB_incl_speed, matB02, matB22;
    int futureBx, futureBz;
    int posB_beg_x, posB_beg_z;
    int posB_end_x, posB_end_z;
    int dist_begZ, dist_begX;
    int dist_endX, dist_endZ;
    int distXZ_sq;

    if (p_vehB->ThingOffset == p_vehA->ThingOffset)
        return false;

    if ((p_vehB->Flag & TngF_Unkn0004) != 0)
        return false;

    // The following is the same as in vehicle_check_collide_with_area(), but some position computations were moved
    // to upper function as speed optimization (maybe there's no need for that anymore?)
    if (abs(p_vehB->Y - posAy) >= 2048)
        return false;

    assert(p_vehB->U.UVehicle.MatrixIndex < next_local_mat);
    p_matB = &local_mats[p_vehB->U.UVehicle.MatrixIndex];
    matB02 = -4 * p_matB->R[0][2];
    matB22 = -4 * p_matB->R[2][2];
    radB_incl_speed = 384 * p_vehB->Radius >> 9;
    futureBx = radB_incl_speed * matB02 >> 8;
    futureBz = radB_incl_speed * matB22 >> 8;

    posB_beg_x = p_vehB->X - futureBx;
    posB_beg_z = p_vehB->Z - futureBz;
    posB_end_x = p_vehB->X + futureBx;
    posB_end_z = p_vehB->Z + futureBz;

    dist_begX = (posB_beg_x - posAx) >> 8;
    dist_begZ = (posB_beg_z - posAz) >> 8;
    dist_endX = (posB_end_x - posAx) >> 8;
    dist_endZ = (posB_end_z - posAz) >> 8;

    distXZ_sq = dist_endZ * dist_endZ + dist_endX * dist_endX;
    if (distXZ_sq >= dist_begZ * dist_begZ + dist_begX * dist_begX)
        distXZ_sq = dist_begZ * dist_begZ + dist_begX * dist_begX;

    if (distXZ_sq >= radB_incl_speed * radB_incl_speed)
        return false;

    // Check if the current vehicle has right-of-way over the second
    if (vehicle_first_has_right_of_way(p_vehA, p_vehB) && check_two_vehicles(p_vehB, p_vehA))
        return false;

    // Check if the 2nd vehicle is blocked, in which case the first should assume
    // right of way to avoid blockade; this workarounds an issue where on two crossroads
    // close to each other, two vehicles will get blocked on each other. The issue
    // was prominent on `-m 0,21`, where it would happen sooner or later without any
    // actions required from the player. A better solution to such jams may be developed
    // later, when the workings of the traffic system are better understood.
    if (vehicles_check_second_is_blocked(p_vehA, p_vehB))
        return false;

    LOGSYNC("Stopping %s thing %d rqspeed %d due to %s thing %d rqspeed %d",
      vehicle_type_name(p_vehA->SubType), (int)p_vehA->ThingOffset,
      (int)p_vehA->U.UVehicle.ReqdSpeed,
      vehicle_type_name(p_vehB->SubType), (int)p_vehB->ThingOffset,
      (int)p_vehB->U.UVehicle.ReqdSpeed);

    p_vehA->SubState = 3;
    p_vehA->U.UVehicle.ReqdSpeed = 0;
    p_vehA->U.UVehicle.AngleDY = 0;
    return true;
}

/** Simplified vehicle collision check for when the vehicles are on the same tile.
 * We don't have to check for overlap, as every vehicle is larger than a tile.
 */
static TbBool check_vehicle_col_same_mapel_with_veh(struct Thing *p_vehA, struct Thing *p_vehB,
  int pos_x, int pos_y, int pos_z)
{
    if ((p_vehB->Flag & TngF_Unkn0004) != 0)
        return false;

    if (abs(p_vehB->Y - pos_y) >= 2048)
        return false;

    // Check if the current vehicle has right-of-way over the second
    if (vehicle_first_has_right_of_way(p_vehA, p_vehB))
        return false;

    LOGSYNC("Stopping %s thing %d rqspeed %d due to %s thing %d rqspeed %d",
      vehicle_type_name(p_vehA->SubType), (int)p_vehA->ThingOffset,
      (int)p_vehA->U.UVehicle.ReqdSpeed,
      vehicle_type_name(p_vehB->SubType), (int)p_vehB->ThingOffset,
      (int)p_vehB->U.UVehicle.ReqdSpeed);

    p_vehA->SubState = 3;
    p_vehA->U.UVehicle.ReqdSpeed = 0;
    p_vehA->U.UVehicle.AngleDY = 0;
    return false; //TODO is that the best result?
}

static TbBool check_vehicle_col_with_pers(struct Thing *p_vehicle, struct Thing *p_person,
  int pos_x, int pos_y, int pos_z)
{
    int dx, dz, per_r_sq, veh_r_sq;

    if ((p_person->Flag2 & TgF2_KnockedOut) != 0)
        return false;
    if (p_person->State == PerSt_DEAD)
        return false;
    if ((p_person->Flag & TngF_Destroyed) != 0)
        return false;
    if (p_person->State == PerSt_PERSON_BURNING)
        return false;
    if (abs((p_person->Y >> 3) - pos_y) >= 2048)
        return false;

    dx = (p_person->X - pos_x) >> 8;
    dz = (p_person->Z - pos_z) >> 8;
    per_r_sq = p_person->Radius * p_person->Radius;
    veh_r_sq = p_vehicle->Radius * p_vehicle->Radius;

    if ((dx) * (dx) + (dz) * (dz) >= veh_r_sq + per_r_sq)
        return false;

    person_hit_by_car(p_person, p_vehicle);

    return false;
}

static TbBool check_vehicle_col_with_mine(struct Thing *p_vehicle, struct SimpleThing *p_minetng,
  int pos_x, int pos_y, int pos_z)
{
    if (p_minetng->SubType != 12)
        return false;

    if ((p_minetng->Timer1 != 999) && (p_minetng->Timer1 > 0))
        p_minetng->Timer1 = 0;

    return false;
}

static TbBool check_vehicle_col_on_mapel(struct Thing *p_vehicle, struct MyMapElement *p_mapel,
  int pos_x, int pos_y, int pos_z)
{
    ThingIdx thing;

    thing = p_mapel->Child;
    while (thing != 0)
    {
      if (thing > 0)
      {
          struct Thing *p_thing;
          p_thing = &things[thing];
          switch (p_thing->Type)
          {
          case TT_VEHICLE:
              if (check_vehicle_col_with_veh(p_vehicle, p_thing, pos_x, pos_y, pos_z))
                  return true;
              break;
          case TT_PERSON:
              if (check_vehicle_col_with_pers(p_vehicle, p_thing, p_vehicle->X, pos_y, p_vehicle->Z))
                  return true;
              break;
          default:
              break;
          }
          thing = p_thing->Next;
      }
      else
      {
          struct SimpleThing *p_sthing;
          p_sthing = &sthings[thing];
          switch (p_sthing->Type)
          {
          case TT_MINE:
              if (check_vehicle_col_with_mine(p_vehicle, p_sthing, p_vehicle->X, pos_y, p_vehicle->Z))
                  return true;
              break;
          default:
              break;
          }
          thing = p_sthing->Next;
      }
    }
    return false;
}

static TbBool check_vehicle_col_on_same_mapel(struct Thing *p_vehicle, struct MyMapElement *p_mapel,
  int pos_x, int pos_y, int pos_z)
{
    ThingIdx thing;

    thing = p_mapel->Child;
    while (thing != 0)
    {
      if (thing > 0)
      {
          struct Thing *p_thing;
          p_thing = &things[thing];
          switch (p_thing->Type)
          {
          case TT_VEHICLE:
              if (check_vehicle_col_same_mapel_with_veh(p_vehicle, p_thing, pos_x, pos_y, pos_z))
                  return true;
              break;
          default:
              break;
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
    return false;
}

/** Checks vehicle collisions.
 */
TbBool check_vehicle_col(struct Thing *p_vehicle)
{
#if 0
    TbBool ret;
    asm volatile (
      "call ASM_check_vehicle_col\n"
        : "=r" (ret) : "a" (p_vehicle));
    return ret;
#endif
    ushort r;
    struct M33 *p_matA;
    int rad_incl_speed, matA02, matA22;
    int tile_ctr_x, tile_ctr_z;
    int tile_x, tile_z;
    int pos_x, pos_y, pos_z;

    if (abs(p_vehicle->U.UVehicle.AngleDY) > 20)
        return false;

    r = p_vehicle->Radius;
    assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
    p_matA = &local_mats[p_vehicle->U.UVehicle.MatrixIndex];
    pos_x = p_vehicle->X;
    pos_y = p_vehicle->Y;
    pos_z = p_vehicle->Z;
    rad_incl_speed = (p_vehicle->Speed >> 4) + (p_vehicle->Speed >> 3) + (386 * r >> 8);
    matA02 = -4 * p_matA->R[0][2];
    matA22 = -4 * p_matA->R[2][2];
    pos_x = (matA02 * rad_incl_speed >> 8) + pos_x - (matA22 >> 2);
    pos_z = (matA22 * rad_incl_speed >> 8) + pos_z + (matA02 >> 2);
    tile_ctr_z = pos_z >> 16;
    tile_ctr_x = pos_x >> 16;

    for (tile_x = tile_ctr_x - 2; tile_x <= tile_ctr_x + 2; tile_x++)
    {
        if ((tile_x < 0) || (tile_x >= 128))
            continue;
        for (tile_z = tile_ctr_z - 2; tile_z <= tile_ctr_z + 2; tile_z++)
        {
            if ((tile_z < 0) || (tile_z >= 128))
                continue;
            struct MyMapElement *p_mapel;
            p_mapel = &game_my_big_map[128 * tile_z + tile_x];
            if (check_vehicle_col_on_mapel(p_vehicle, p_mapel, pos_x, pos_y, pos_z))
                return true;
        }
    }

    struct MyMapElement *p_mapel;
    p_mapel = &game_my_big_map[128 * (p_vehicle->Z >> 16) + (p_vehicle->X >> 16)];
    if (check_vehicle_col_on_same_mapel(p_vehicle, p_mapel, pos_x, pos_y, pos_z))
        return true;

    return false;
}

void start_crashing(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_start_crashing\n"
        : : "a" (p_vehicle));
}

void start_landing(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_start_landing\n"
        : : "a" (p_vehicle));
}

void process_shuttle_pod(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_shuttle_pod\n"
        : : "a" (p_vehicle));
}

void init_vehicle_explode(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_init_vehicle_explode\n"
        : : "a" (p_vehicle));
}

void process_tank_stationary(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_tank_stationary\n"
        : : "a" (p_vehicle));
}

void process_tank(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_tank\n"
        : : "a" (p_vehicle));
}

void set_passengers_location(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_set_passengers_location\n"
        : : "a" (p_vehicle));
}

void move_flying_vehicle(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_move_flying_vehicle\n"
        : : "a" (p_vehicle));
}

void process_tank_turret(struct Thing *p_tank)
{
    struct Thing *p_turret;
    int target_x, target_y;
    int turret;
    short angle;
    int dt_angle;

    turret = p_tank->U.UVehicle.SubThing;
    if (turret == 0) {
        p_tank->OldTarget = LbFPMath_PI+1;
        return;
    }
    p_turret = &things[turret];
    if ((p_tank->Flag & TngF_Unkn20000000) != 0)
    {
        target_x = p_tank->U.UVehicle.TargetDX;
        target_y = p_tank->U.UVehicle.TargetDZ;
    }
    else
    {
        struct Thing *p_target;
        p_target = p_tank->PTarget;
        if (p_target == NULL)
        {
            p_tank->OldTarget = 20000;
            return;
        }
        target_x = PRCCOORD_TO_MAPCOORD(p_target->X);
        target_y = PRCCOORD_TO_MAPCOORD(p_target->Z);
    }
    angle = p_turret->U.UMGun.AngleY
        - angle_between_points(target_x, target_y, PRCCOORD_TO_MAPCOORD(p_tank->X), PRCCOORD_TO_MAPCOORD(p_tank->Z));
    if (angle < -LbFPMath_PI)
        angle += 2*LbFPMath_PI;
    else if (angle > LbFPMath_PI)
        angle -= 2*LbFPMath_PI;

    //TODO CONFIG how fast the tank can target could be a difficulty-related setting
    // Travel 1/8 of the distance in each game turn
    dt_angle = angle / 8;
    if (dt_angle > LbFPMath_PI/17)
        dt_angle = LbFPMath_PI/17;
    if (dt_angle < -LbFPMath_PI/17)
        dt_angle = -LbFPMath_PI/17;
    if (dt_angle == 0)
    {
        if (angle > 0)
            dt_angle = 1;
        if (angle < 0)
            dt_angle = -1;
    }

    // Despite being caused by the turret, we bind the sound samples to the vehicle part
    // of the tank. This is because turrets do not contain full position on map, so the
    // sound update would misplace the sound source if it was bound to the turret
    if ((p_turret->Flag2 & TgF2_Unkn0200) != 0)
    {
        if (dt_angle <= 1) {
            // Play rotation stop sample
            if (!IsSamplePlaying(p_tank->ThingOffset, 47, 0))
                play_dist_sample(p_tank, 47, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
            p_turret->Flag2 &= ~TgF2_Unkn0200;
        }
    }
    else
    {
        // Play rotation sample if moving over 1.2 degree per turn and the angle is not getting smaller.
        // Huge values of OldTarget (beyond pi) indicate that previously we had no target.
        if ((abs(dt_angle) >= LbFPMath_PI/75) && (p_tank->OldTarget < abs(angle) || p_tank->OldTarget > LbFPMath_PI)) {
            if (!IsSamplePlaying(p_tank->ThingOffset, 48, 0))
                play_dist_sample(p_tank, 48, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
            p_turret->Flag2 |= TgF2_Unkn0200;
        }
    }

    p_turret->U.UMGun.AngleY -= dt_angle;
    p_tank->OldTarget = abs(angle);
    p_turret->U.UMGun.AngleY = (p_turret->U.UMGun.AngleY + 2*LbFPMath_PI) & LbFPMath_AngleMask;
}

void process_ship(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_ship\n"
        : : "a" (p_vehicle));
}

void process_mech_stationary(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_mech_stationary\n"
        : : "a" (p_vehicle));
}

void process_mech(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_mech\n"
        : : "a" (p_vehicle));
}

void process_mech_unknown1(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_mech_unknown1\n"
        : : "a" (p_vehicle));
}

int process_my_crashing_vehicle(struct Thing *p_vehicle)
{
    int ret;
    asm volatile ("call ASM_process_my_crashing_vehicle\n"
        : "=r" (ret) : "a" (p_vehicle));
    return ret;
}

int train_unkn_st21_exit_func_1(struct Thing *p_vehicle)
{
    int ret;
    asm volatile ("call ASM_train_unkn_st21_exit_func_1\n"
        : "=r" (ret) : "a" (p_vehicle));
    return ret;
}

void train_unkn_st18_func_1(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_train_unkn_st18_func_1\n"
        : : "a" (p_vehicle));
}

void train_unkn_st18_func_2(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_train_unkn_st18_func_2\n"
        : : "a" (p_vehicle));
}

void move_vehicle(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_move_vehicle\n"
        : : "a" (p_vehicle));
}

void train_unkn_st20_func_1(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_train_unkn_st20_func_1\n"
        : : "a" (p_vehicle));
}

void process_my_flying_vehicle(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_my_flying_vehicle\n"
        : : "a" (p_vehicle));
}

void process_unplacing(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_unplacing\n"
        : : "a" (p_vehicle));
}

void process_stop_as_soon_as_you_can(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_stop_as_soon_as_you_can\n"
        : : "a" (p_vehicle));
}

void process_hovering_vehicle(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_hovering_vehicle\n"
        : : "a" (p_vehicle));
}

void process_my_takeoff(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_my_takeoff\n"
        : : "a" (p_vehicle));
}

void process_my_land_vehicle(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_my_land_vehicle\n"
        : : "a" (p_vehicle));
}

void set_vehicle_alt(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_set_vehicle_alt\n"
        : : "a" (p_vehicle));
}

void process_unstopping(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_unstopping\n"
        : : "a" (p_vehicle));
}

void process_stopping(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_stopping\n"
        : : "a" (p_vehicle));
}

void process_vehicle_goto_point_land(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_vehicle_goto_point_land\n"
        : : "a" (p_vehicle));
}

void process_vehicle_goto_point_fly(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_vehicle_goto_point_fly\n"
        : : "a" (p_vehicle));
}

void process_parked_flyer(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_parked_flyer\n"
        : : "a" (p_vehicle));
}

void init_mech_explode(struct Thing *p_vehicle)
{
    asm volatile ("call ASM_init_mech_explode\n"
        : : "a" (p_vehicle));
}

void process_train(struct Thing *p_vehicle)
{
    int dtvel;

    dtvel = p_vehicle->U.UVehicle.ReqdSpeed - p_vehicle->Speed;
    if (abs(dtvel) >= 4) {
        p_vehicle->Speed += dtvel >> 1;
    } else {
        p_vehicle->Speed = p_vehicle->U.UVehicle.ReqdSpeed;
    }

    if (p_vehicle->State == VehSt_UNKN_45)
    {
          p_vehicle->Y <<= 3;
          if (process_my_crashing_vehicle(p_vehicle))
          {
              move_flying_vehicle(p_vehicle);
              p_vehicle->Y >>= 3;
              set_passengers_location(p_vehicle);
          }
          return;
    }

    if (p_vehicle->U.UVehicle.TNode != 0)
    {
        struct Thing *p_station;

        assert(p_vehicle->SubType == SubTT_VEH_TRAIN); // only trains have stations
        assert(p_vehicle->U.UVehicle.TNode > 0); // trains stations have positive indexes
        p_station = &things[p_vehicle->U.UVehicle.TNode];
        if ((p_station->Type != TT_BUILDING) || (p_station->Flag & TngF_Destroyed) != 0)
        {
            p_vehicle->Flag |= TngF_Destroyed;
            start_crashing(p_vehicle);
            p_vehicle->U.UVehicle.ReqdSpeed = 0;
            return;
        }
    }

    switch (p_vehicle->State)
    {
    case VehSt_UNKN_12:
        if (p_vehicle->U.UVehicle.TNode != 0)
        {
            if ((p_vehicle->Flag & TngF_Unkn08000000) != 0)
                train_unkn_st18_func_1(p_vehicle);
            else
                train_unkn_st18_func_2(p_vehicle);
        }
        set_passengers_location(p_vehicle);
        move_vehicle(p_vehicle);
        break;
    case VehSt_UNKN_13:
        if (!train_unkn_st21_exit_func_1(p_vehicle))
        {
            p_vehicle->State = VehSt_UNKN_14;
            p_vehicle->U.UVehicle.Timer2 = 100;
            play_dist_sample(p_vehicle, 225 + (LbRandomAnyShort() % 4), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        }
        break;
    case VehSt_UNKN_14:
        train_unkn_st20_func_1(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    case VehSt_UNKN_15:
        train_unkn_st21_exit_func_1(p_vehicle);
        if (p_vehicle->U.UVehicle.GotoX != 0 && things[p_vehicle->U.UVehicle.GotoX].State == VehSt_UNKN_12) {
            p_vehicle->State = VehSt_UNKN_12;
        }
        else if (p_vehicle->Owner != 0 && things[p_vehicle->Owner].State == VehSt_UNKN_12) {
            p_vehicle->State = VehSt_UNKN_12;
        }
        set_passengers_location(p_vehicle);
        break;
    default:
        LOGERR("Shagged %s %d state %d",
          vehicle_type_name(p_vehicle->SubType), (int)p_vehicle->ThingOffset,
          (int)p_vehicle->State);
        break;
    }
}

void process_vehicle_goto_point_liftoff(struct Thing *p_vehicle)
{
    int i;

    set_passengers_location(p_vehicle);
    i = alt_at_point(PRCCOORD_TO_MAPCOORD(p_vehicle->X), PRCCOORD_TO_MAPCOORD(p_vehicle->Z));
    if (p_vehicle->Y < MAPCOORD_TO_PRCCOORD(400, 0) + (i << 3)) {
        p_vehicle->Speed = 0;
        p_vehicle->Y += MAPCOORD_TO_PRCCOORD(8, 0);
    } else {
        p_vehicle->SubState = 6;
    }
}

void process_veh_ground(struct Thing *p_vehicle)
{
    int dtvel;

    dtvel = p_vehicle->U.UVehicle.ReqdSpeed - p_vehicle->Speed;
    if (dtvel != 0)
    {
        if (dtvel > 100)
            p_vehicle->Speed += 100;
        else if (dtvel < -400)
            p_vehicle->Speed -= 400;
        else
            p_vehicle->Speed = p_vehicle->U.UVehicle.ReqdSpeed;
    }

    switch (p_vehicle->State)
    {
    case VehSt_UNKN_3A:
    case VehSt_UNKN_42:
    case VehSt_UNKN_43:
        p_vehicle->Y <<= 3;
        process_my_flying_vehicle(p_vehicle);
        move_flying_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        p_vehicle->Y >>= 3;
        break;
    case VehSt_UNKN_3E:
        process_unplacing(p_vehicle);
        move_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    case VehSt_UNKN_41:
        p_vehicle->U.UVehicle.ReqdSpeed = 0;
        p_vehicle->U.UVehicle.AngleDY = 0;
        p_vehicle->Y <<= 3;
        process_hovering_vehicle(p_vehicle);
        move_flying_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        p_vehicle->Y >>= 3;
        break;
    case VehSt_UNKN_45:
        p_vehicle->Y <<= 3;
        if (process_my_crashing_vehicle(p_vehicle)) {
            move_flying_vehicle(p_vehicle);
            set_passengers_location(p_vehicle);
        }
        p_vehicle->Y >>= 3;
        break;
    case VehSt_UNKN_39:
    case VehSt_UNKN_44:
        p_vehicle->Y <<= 3;
        process_my_takeoff(p_vehicle);
        move_flying_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        p_vehicle->Y >>= 3;
        break;
    case VehSt_FLY_LANDING:
        p_vehicle->Y <<= 3;
        process_my_land_vehicle(p_vehicle);
        move_flying_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        p_vehicle->Y >>= 3;
        break;
    case VehSt_UNKN_3C:
        // Pretend we are wandering
        p_vehicle->State = VehSt_WANDER;
        if (p_vehicle->U.UVehicle.TNode != 0)
              process_next_tnode(p_vehicle);
        move_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        // Get back to specifics of this state
        p_vehicle->State = VehSt_UNKN_3C;
        process_stop_as_soon_as_you_can(p_vehicle);
        break;
    case VehSt_NONE:
    case VehSt_PARKED_PERPN:
    case VehSt_PARKED_PARAL:
        if (p_vehicle->SubType == SubTT_VEH_FLYING)
            process_parked_flyer(p_vehicle);
        break;
    case VehSt_UNKN_27:
        switch (p_vehicle->SubState)
        {
        case 5:
            process_vehicle_goto_point_liftoff(p_vehicle);
            move_vehicle(p_vehicle);
            set_passengers_location(p_vehicle);
            break;
        case 6:
            process_vehicle_goto_point_fly(p_vehicle);
            move_vehicle(p_vehicle);
            set_passengers_location(p_vehicle);
            break;
        case 7:
            process_vehicle_goto_point_land(p_vehicle);
            move_vehicle(p_vehicle);
            set_passengers_location(p_vehicle);
            break;
        default:
            move_vehicle(p_vehicle);
            set_passengers_location(p_vehicle);
            break;
        }
        break;
    case VehSt_UNKN_36:
        process_stopping(p_vehicle);
        move_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    case VehSt_UNKN_38:
        process_unstopping(p_vehicle);
        move_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    case VehSt_WANDER:
    case VehSt_GOTO_LOC:
    case VehSt_UNKN_33:
    case VehSt_UNKN_34:
        if (p_vehicle->U.UVehicle.TNode != 0)
            process_next_tnode(p_vehicle);
        move_vehicle(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    default:
        LOGERR("Unexpected %s %d state %d",
          vehicle_type_name(p_vehicle->SubType), (int)p_vehicle->ThingOffset,
          (int)p_vehicle->State);
        break;
    }

    if ((p_vehicle->SubType != SubTT_VEH_FLYING)
     && ((p_vehicle->Flag & TngF_Destroyed) == 0)) {
        set_vehicle_alt(p_vehicle);
    }
}

void vehicle_check_outside_map(struct Thing *p_vehicle)
{
    if ((p_vehicle->X <= MAPCOORD_TO_PRCCOORD(0,0))
      || (p_vehicle->X >= MAPCOORD_TO_PRCCOORD(MAP_COORD_WIDTH,0))
      || (p_vehicle->Z <= MAPCOORD_TO_PRCCOORD(0,0))
      || (p_vehicle->Z >= MAPCOORD_TO_PRCCOORD(MAP_COORD_HEIGHT,0)))
    {
        if (p_vehicle->State != VehSt_UNKN_45)
            start_crashing(p_vehicle);
    }
}

void process_vehicle(struct Thing *p_vehicle)
{
#if 0
    asm volatile ("call ASM_process_vehicle\n"
        : : "a" (p_vehicle));
    return;
#endif
    if ((p_vehicle->Flag & TngF_Destroyed) == 0)
        p_vehicle->OldTarget = 0;
    if (p_vehicle->U.UVehicle.RecoilTimer > 0)
        p_vehicle->U.UVehicle.RecoilTimer--;
    if ((p_vehicle->U.UVehicle.WorkPlace & VWPFlg_Unkn0080) != 0)
    {
        p_vehicle->Health -= 16;
        if (p_vehicle->Health < 0)
            start_crashing(p_vehicle);
        if (((gameturn & 7) == 0) && (LbRandomAnyShort() & 7) == 0)
            p_vehicle->U.UVehicle.WorkPlace &= ~VWPFlg_Unkn0080;
    }

    switch (p_vehicle->SubType)
    {
    case SubTT_VEH_TRAIN:
        if (p_vehicle->State == VehSt_UNKN_D)
            break;
        process_train(p_vehicle);
        break;
    case SubTT_VEH_SHUTTLE_POD:
        process_shuttle_pod(p_vehicle);
        break;
    case SubTT_VEH_GROUND:
    case SubTT_VEH_UNKN43:
        process_veh_ground(p_vehicle);
        break;
    case SubTT_VEH_FLYING:
        vehicle_check_outside_map(p_vehicle);
        process_veh_ground(p_vehicle);
        break;
    case SubTT_VEH_TANK:
        if (p_vehicle->State == VehSt_UNKN_45) {
            bang_new4(p_vehicle->X, p_vehicle->Y, p_vehicle->Z, 10);
            init_vehicle_explode(p_vehicle);
        } else if (p_vehicle->State == VehSt_PARKED_PARAL) {
            process_tank_stationary(p_vehicle);
        } else {
            process_tank(p_vehicle);
            set_passengers_location(p_vehicle);
        }
        process_tank_turret(p_vehicle);
        break;
    case SubTT_VEH_SHIP:
        process_ship(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    case SubTT_VEH_MECH:
        unkn_path_func_001(p_vehicle, 0);
        if (p_vehicle->State == VehSt_PARKED_PARAL)
            process_mech_stationary(p_vehicle);
        else
            process_mech(p_vehicle);
        process_mech_unknown1(p_vehicle);
        set_passengers_location(p_vehicle);
        break;
    default:
        break;
    }
}

void preprogress_trains_turns(ulong nturns)
{
    ulong turns;

    for (turns = nturns; turns > 0; turns--)
    {
        ThingIdx thing;
        int tng_remain;
        struct Thing *p_thing;

        thing = things_used_head;
        tng_remain = things_used;
        while (thing > 0)
        {
            if (tng_remain == 0)
                break;
            p_thing = &things[thing];
            thing = p_thing->LinkChild;
            tng_remain--;

            if ((p_thing->Type == TT_VEHICLE)
              && (p_thing->SubType == SubTT_VEH_TRAIN))
                process_vehicle(p_thing);
        }
    }
}

int vehicle_hit_by_bullet(struct Thing *p_vehicle, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type)
{
    int health_decr;

    if ((p_vehicle->Flag & TngF_Destroyed) != 0) {
        return 0;
    }

    p_vehicle->Flag |= TngF_Unkn01000000;
    if (p_vehicle->SubType == SubTT_VEH_TANK)
        hp = hp >> 1;
    if (p_vehicle->SubType == SubTT_VEH_MECH)
        hp >>= 2;

    play_dist_sample(p_vehicle, 65, FULL_VOL, EQUL_PAN, NORM_PTCH, 0, 1);

    if (type == DMG_UZI || type == DMG_MINIGUN)
        hp >>= 1;
    health_decr = hp >> 1;

    p_vehicle->Health -= health_decr;
    p_vehicle->U.UVehicle.RecoilTimer = 5;
    if (p_vehicle->Health > 0) {
        return health_decr;
    }

    p_vehicle->OldTarget = p_attacker->ThingOffset;
    if (p_vehicle->SubType == SubTT_VEH_MECH)
    {
        p_vehicle->Flag |= TngF_Destroyed;
        init_mech_explode(p_vehicle);
    }
    else
    {
        p_vehicle->Flag |= TngF_Destroyed;
        start_crashing(p_vehicle);
        play_dist_sample(p_vehicle, 95, FULL_VOL, EQUL_PAN, NORM_PTCH, 0, 1);
    }
    return p_vehicle->Health + health_decr;
}

/******************************************************************************/
