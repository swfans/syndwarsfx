/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file vehicle.h
 *     Header file for vehicle.c.
 * @par Purpose:
 *     Routines implementing vehicle.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef VEHICLE_H
#define VEHICLE_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define VEHICLE_PASSENGER_LIMIT 500

/** Distance from bottom of the mech body to where the barrel is, for beam weapons.
 *
 * Used as starting point of shots.
 */
#define MECH_BOTTOM_TO_BEAM_WEAPON_HEIGHT 24

/** Length from mech body center to tip of the mech weapon, for beam weapons.
 *
 * Used as starting point of shots.
 */
#define MECH_CENTER_TO_BEAM_WEAPON_TIP_MAPCOORD -96

enum SubTypeVehicle {
  SubTT_VEH_TRAIN = 0x1D,
  SubTT_VEH_UNKN30 = 0x1E,
  SubTT_VEH_UNKN31 = 0x1F,
  SubTT_VEH_UNKN32 = 0x20,
  SubTT_VEH_UNKN33 = 0x21,
  SubTT_VEH_UNKN34 = 0x22,
  SubTT_VEH_UNKN35 = 0x23,
  SubTT_VEH_UNKN36 = 0x24,
  SubTT_VEH_UNKN37 = 0x25,
  SubTT_VEH_UNKN38 = 0x26,
  SubTT_VEH_UNKN39 = 0x27,
  SubTT_VEH_GROUND = 0x28,
  SubTT_VEH_SHUTTLE_POD = 0x29,
  SubTT_VEH_UNKN42 = 0x2A,
  SubTT_VEH_UNKN43 = 0x2B,
  SubTT_VEH_UNKN44 = 0x2C,
  SubTT_VEH_UNKN45 = 0x2D,
  SubTT_VEH_UNKN46 = 0x2E,
  SubTT_VEH_UNKN47 = 0x2F,
  SubTT_VEH_UNKN48 = 0x30,
  SubTT_VEH_UNKN49 = 0x31,
  SubTT_VEH_FLYING = 0x32,
  SubTT_VEH_TANK = 0x33,
  SubTT_VEH_UNKN52 = 0x34,
  SubTT_VEH_UNKN53 = 0x35,
  SubTT_VEH_SHIP = 0x36,
  SubTT_VEH_UNKN55 = 0x37,
  SubTT_VEH_UNKN56 = 0x38,
  SubTT_VEH_UNKN57 = 0x39,
  SubTT_VEH_UNKN58 = 0x3A,
  SubTT_VEH_MECH = 0x3B,
  SubTT_VEH_UNKN60 = 0x3C,
};

enum VehicleObjectModel {
  VehOM_CIVIL_CAR_BLACK = 0x00,
  VehOM_DELOREAN_GREY = 0x01,
  VehOM_BIKE_CLOSED_BROWN = 0x02,
  VehOM_ZEALOT_POINTY_RED = 0x03,
  VehOM_IML_TRAIN = 0x04,
  VehOM_ARMORED_SM_GREEN = 0x06,
  VehOM_ARMORED_LG_GREEN = 0x07,
  VehOM_POLICE_CAR = 0x08,
  VehOM_POLICE_TRUCK = 0x09,
  VehOM_UTILITY_SM_YELLOW = 0x0a,
  VehOM_BRANDED_TRUCK_BF = 0x0b,
  VehOM_FIRE_TRUCK = 0x0c,
  VehOM_ABULANCE_CAR = 0x0d,
  VehOM_TAXI_YELLOW = 0x0e,
  VehOM_MERCH_BOAT = 0x0f,
  VehOM_ROCKET_BOAT = 0x10,
  VehOM_YACHT_WHITE = 0x11,
  VehOM_MBTANK = 0x12,
  VehOM_CIVIL_CAR_RED = 0x15,
  VehOM_DELOREAN_YELLOW = 0x16,
  VehOM_SHUTTLE_LG_WHITE = 0x17,
  VehOM_TAXI_RED = 0x18,
  VehOM_BIKE_CLOSED_BROWN_ACCEL = 0x1c,
  VehOM_CLAWTANK = 0x1d,
  VehOM_BRANDED_TRUCK_MG = 0x1f,
};

enum VehicleState {
  VehSt_NONE = 0x0,
  VehSt_UNKN_1 = 0x1,
  VehSt_UNKN_2 = 0x2,
  VehSt_UNKN_3 = 0x3,
  VehSt_UNKN_4 = 0x4,
  VehSt_UNKN_5 = 0x5,
  VehSt_UNKN_6 = 0x6,
  VehSt_UNKN_7 = 0x7,
  VehSt_UNKN_8 = 0x8,
  VehSt_UNKN_9 = 0x9,
  VehSt_UNKN_A = 0xA,
  VehSt_UNKN_B = 0xB,
  VehSt_UNKN_C = 0xC,
  VehSt_UNKN_D = 0xD,
  VehSt_UNKN_E = 0xE,
  VehSt_UNKN_F = 0xF,
  VehSt_UNKN_10 = 0x10,
  VehSt_WANDER = 0x11,
  VehSt_UNKN_12 = 0x12,
  VehSt_UNKN_13 = 0x13,
  VehSt_UNKN_14 = 0x14,
  VehSt_UNKN_15 = 0x15,
  VehSt_UNKN_16 = 0x16,
  VehSt_UNKN_17 = 0x17,
  VehSt_UNKN_18 = 0x18,
  VehSt_UNKN_19 = 0x19,
  VehSt_UNKN_1A = 0x1A,
  VehSt_UNKN_1B = 0x1B,
  VehSt_UNKN_1C = 0x1C,
  VehSt_UNKN_1D = 0x1D,
  VehSt_UNKN_1E = 0x1E,
  VehSt_UNKN_1F = 0x1F,
  VehSt_UNKN_20 = 0x20,
  VehSt_PARKED_PARAL = 0x21,
  VehSt_UNKN_22 = 0x22,
  VehSt_UNKN_23 = 0x23,
  VehSt_UNKN_24 = 0x24,
  VehSt_UNKN_25 = 0x25,
  VehSt_UNKN_26 = 0x26,
  VehSt_UNKN_27 = 0x27,
  VehSt_UNKN_28 = 0x28,
  VehSt_UNKN_29 = 0x29,
  VehSt_UNKN_2A = 0x2A,
  VehSt_UNKN_2B = 0x2B,
  VehSt_UNKN_2C = 0x2C,
  VehSt_UNKN_2D = 0x2D,
  VehSt_UNKN_2E = 0x2E,
  VehSt_UNKN_2F = 0x2F,
  VehSt_UNKN_30 = 0x30,
  VehSt_UNKN_31 = 0x31,
  VehSt_GOTO_LOC = 0x32,
  VehSt_UNKN_33 = 0x33,
  VehSt_UNKN_34 = 0x34,
  VehSt_UNKN_35 = 0x35,
  VehSt_UNKN_36 = 0x36,
  VehSt_UNKN_37 = 0x37,
  VehSt_UNKN_38 = 0x38,
  VehSt_UNKN_39 = 0x39,
  VehSt_UNKN_3A = 0x3A,
  VehSt_FLY_LANDING = 0x3B,
  VehSt_UNKN_3C = 0x3C,
  VehSt_PARKED_PERPN = 0x3D,
  VehSt_UNKN_3E = 0x3E,
  VehSt_UNKN_3F = 0x3F,
  VehSt_UNKN_40 = 0x40,
  VehSt_UNKN_41 = 0x41,
  VehSt_UNKN_42 = 0x42,
  VehSt_UNKN_43 = 0x43,
  VehSt_UNKN_44 = 0x44,
  VehSt_UNKN_45 = 0x45,
  VehSt_UNKN_46 = 0x46,
  VehSt_UNKN_47 = 0x47,
  VehSt_UNKN_48 = 0x48,
  VehSt_UNKN_49 = 0x49,
  VehSt_UNKN_4A = 0x4A,
  VehSt_UNKN_4B = 0x4B,
  VehSt_UNKN_4C = 0x4C,
  VehSt_UNKN_4D = 0x4D,
  VehSt_UNKN_4E = 0x4E,
  VehSt_UNKN_4F = 0x4F,
  VehSt_UNKN_50 = 0x50,
  VehSt_UNKN_51 = 0x51,
  VehSt_UNKN_52 = 0x52,
};

enum VehWorkPlaceFlags {
    VWPFlg_Unkn0001     = 0x0001,
    VWPFlg_Unkn0002     = 0x0002,
    VWPFlg_Unkn0004     = 0x0004,
    VWPFlg_Unkn0008     = 0x0008,
    VWPFlg_HasAgPhase1  = 0x0010,
    VWPFlg_HasAgPhase2  = 0x0020,
    VWPFlg_HasAgok      = 0x0040,
    VWPFlg_Unkn0080     = 0x0080,
    VWPFlg_Unkn0100     = 0x0100,
    VWPFlg_Unkn0200     = 0x0200,
    VWPFlg_Unkn0400     = 0x0400,
    VWPFlg_Unkn0800     = 0x0800,
    VWPFlg_Unkn1000     = 0x1000,
    VWPFlg_Unkn2000     = 0x2000,
    VWPFlg_Unkn4000     = 0x4000,
    VWPFlg_Unkn8000     = 0x8000,
};

struct Thing;

/** Stores configuration for a state of a vehicle.
 */
struct VehStateConfig
{
    char Name[17];
};


#pragma pack()
/******************************************************************************/
void init_mech(void);
void mech_unkn_func_02(void);
void mech_unkn_func_09(ThingIdx thing);
void init_mech_explode(struct Thing *p_vehicle);

TbBool vehicle_is_destroyed(ThingIdx thing);

void veh_add(struct Thing *p_thing, short frame);

ushort veh_passenger_count(struct Thing *p_veh);

void VNAV_unkn_func_207(struct Thing *p_thing);

void process_tank_turret(struct Thing *p_tank);

void process_vehicle(struct Thing *p_vehicle);

void start_crashing(struct Thing *p_vehicle);
void start_landing(struct Thing *p_vehicle);

int check_for_a_vehicle_here(int x, int z, struct Thing *p_vehicle);

void preprogress_trains_turns(ulong nturns);

const char *vehicle_type_name(ushort vtype);
void snprint_vehicle_state(char *buf, ulong buflen, struct Thing *p_thing);

int vehicle_hit_by_bullet(struct Thing *p_vehicle, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
