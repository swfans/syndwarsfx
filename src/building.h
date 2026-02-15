/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file building.h
 *     Header file for building.c.
 * @par Purpose:
 *     Buildings related functions.
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
#ifndef BUILDING_H
#define BUILDING_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define MGUN_BOTTOM_TO_WEAPON_HEIGHT 0

enum SubTypeBuilding {
  SubTT_BLD_NONE = 0x0,
  SubTT_BLD_1 = 0x1,
  SubTT_BLD_2 = 0x2,
  SubTT_BLD_3 = 0x3,
  SubTT_BLD_4 = 0x4,
  SubTT_BLD_5 = 0x5,
  SubTT_BLD_6 = 0x6,
  SubTT_BLD_7 = 0x7,
  SubTT_BLD_8 = 0x8,
  SubTT_BLD_9 = 0x9,
  SubTT_BLD_A = 0xA,
  SubTT_BLD_B = 0xB,
  SubTT_BLD_C = 0xC,
  SubTT_BLD_D = 0xD,
  SubTT_BLD_E = 0xE,
  SubTT_BLD_F = 0xF,
  SubTT_BLD_10 = 0x10,
  SubTT_BLD_11 = 0x11,
  SubTT_BLD_12 = 0x12,
  SubTT_BLD_13 = 0x13,
  SubTT_BLD_SHUTLDR = 0x14, /**< Shuttle loader, aka transport tube. */
  SubTT_BLD_BEZIER_ROAD = 0x15, /**< Curvy roads above the ground level. */
  SubTT_BLD_16 = 0x16,
  SubTT_BLD_17 = 0x17,
  SubTT_BLD_DOME = 0x18, /**< Half-spherical dome, capable of opening and closing. */
  SubTT_BLD_19 = 0x19,
  SubTT_BLD_1A = 0x1A,
  /** Standard building with no special mechanics. */
  SubTT_BLD_STANDARD = 0x1B, /**< Most generic buildings, big and small. */
  SubTT_BLD_TRAINTRK = 0x1C, /**< Track over which the IML trains travel. */
  SubTT_BLD_1D = 0x1D,
  SubTT_BLD_1E = 0x1E,
  SubTT_BLD_1F = 0x1F,
  SubTT_BLD_MGUN = 0x20, /**< Mounted gun, aka stationary turret. */
  SubTT_BLD_21 = 0x21,
  SubTT_BLD_22 = 0x22,
  SubTT_BLD_23 = 0x23,
  SubTT_BLD_24 = 0x24,
  SubTT_BLD_GATE = 0x25, /**< Retractable gate, allows only vehicles to pass. */
  SubTT_BLD_26 = 0x26,
  SubTT_BLD_27 = 0x27,
  SubTT_BLD_28 = 0x28,
  SubTT_BLD_29 = 0x29,
  SubTT_BLD_2A = 0x2A,
  SubTT_BLD_2B = 0x2B,
  SubTT_BLD_2C = 0x2C,
  SubTT_BLD_2D = 0x2D,
  SubTT_BLD_2E = 0x2E,
  SubTT_BLD_STATION = 0x2F, /**< Station at which a train stops. */
  SubTT_BLD_30 = 0x30,
  SubTT_BLD_31 = 0x31,
  SubTT_BLD_32 = 0x32,
  SubTT_BLD_33 = 0x33,
  SubTT_BLD_34 = 0x34,
  SubTT_BLD_35 = 0x35,
  SubTT_BLD_MOVN_ROTOR = 0x36, /**< Rotating object, wind rotor or part of future machinery. */
  SubTT_BLD_37 = 0x37,
  SubTT_BLD_38 = 0x38,
  SubTT_BLD_39 = 0x39,
  SubTT_BLD_3A = 0x3A,
  SubTT_BLD_3B = 0x3B,
  SubTT_BLD_3C = 0x3C,
  SubTT_BLD_BILLBOARD = 0x3D, /**< Screen capable of displaying advert movies. */
  SubTT_BLD_3E = 0x3E,
  SubTT_BLD_3F = 0x3F,
  SubTT_BLD_40 = 0x40,
  SubTT_BLD_41 = 0x41,
  SubTT_BLD_42 = 0x42,
  SubTT_BLD_43 = 0x43,
  SubTT_BLD_44 = 0x44,
};

enum BuildingStates {
  BldSt_OBJ_NONE = 0x0,
  BldSt_TRA_OPENING,
  BldSt_OBJ_UNKN02,
  BldSt_OBJ_UNKN03,
  BldSt_TRA_CLOSING,
  BldSt_OBJ_UNKN05,
  BldSt_OBJ_UNKN06,
  BldSt_TRA_OPENED,
  BldSt_TRA_CLOSED,
  BldSt_OBJ_UNKN09,
};

struct BuildingStat
{
    char Name[12];
};


#pragma pack()
/******************************************************************************/

const char *building_type_name(ushort btype);

/** Returns whether given thing is a building which can transform into open structure.
 * Only domes can do such transformation.
 */
TbBool building_can_transform_open(ThingIdx bldng);

struct Thing *create_building_thing(int x, int y, int z, ushort a4, ushort a5, ushort a6);

void init_mgun_explode(struct Thing *p_thing);

void collapse_building(short x, short y, short z, struct Thing *p_building);

void bul_hit_vector(int x, int y, int z, short col, int hp, int type);

void process_building(struct Thing *p_building);

int building_hit_by_bullet(struct Thing *p_thing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
