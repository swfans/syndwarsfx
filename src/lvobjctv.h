/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file lvobjctv.h
 *     Header file for lvobjctv.c.
 * @par Purpose:
 *     Routines for level objectives handling.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LVOBJCTV_H
#define LVOBJCTV_H

#include "bftypes.h"
#include "bffile.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define MISSION_NETSCAN_OBV_COUNT 1000
#define NETSCAN_OBJECTIVES_MAX_COUNT 10
#define NETSCAN_OBJECTIVE_POINTS 5

enum ObjectiveStatuses {
    ObvStatu_UNDECIDED = 0,
    ObvStatu_COMPLETED,
    ObvStatu_FAILED,
};

/* Temporary value for MissionStatus, as it does not use ObvStatu_FAILED as it should */
#define ObvStatu_MissTmp_FAILED -1

/* deprecated - when callbacks are defined within objectv_defs[],
 * there will be no need to reference specific objective.
 */
enum GameObjectiveTypes {
    GAME_OBJ_NONE = 0,
    GAME_OBJ_P_DEAD,
    GAME_OBJ_ALL_G_DEAD,
    GAME_OBJ_MEM_G_DEAD,
    GAME_OBJ_P_NEAR,
    GAME_OBJ_MEM_G_NEAR,
    GAME_OBJ_P_ARRIVES,
    GAME_OBJ_MEM_G_ARRIVES,
    GAME_OBJ_ALL_G_ARRIVES,
    GAME_OBJ_PERSUADE_P,
    GAME_OBJ_PERSUADE_MEM_G,
    GAME_OBJ_PERSUADE_ALL_G,
    GAME_OBJ_TIME,
    GAME_OBJ_GET_ITEM,
    GAME_OBJ_USE_ITEM,
    GAME_OBJ_FUNDS,
    GAME_OBJ_DESTROY_OBJECT,
    GAME_OBJ_PKILL_P,
    GAME_OBJ_PKILL_MEM_G,
    GAME_OBJ_PKILL_ALL_G,
    GAME_OBJ_USE_PANET,
    GAME_OBJ_UNUSED_21,
    GAME_OBJ_PROTECT_G,
    GAME_OBJ_P_PERS_G,
    GAME_OBJ_ALL_G_USE_V,
    GAME_OBJ_MEM_G_USE_V,
    GAME_OBJ_V_ARRIVES,
    GAME_OBJ_DESTROY_V,
    GAME_OBJ_ITEM_ARRIVES,
    GAME_OBJ_TYPES_COUNT,
};

enum GameObjectiveFlags {
    GObjF_NONE = 0x00,
    /** The objective is hidden - its text and radar track is not shown */
    GObjF_HIDDEN = 0x01,
    /** The objective cannot be met - it is shown, but fulfilling its criteria does nothing */
    GObjF_CANT_MET = 0x02,
};

struct Thing;

struct Objective { // sizeof=32
    ushort Next;
    ubyte Map;
    ubyte Level;
    ushort Status;
    ushort Type;
    ushort Flags;
    short Thing;
    short X;
    short Y;
    short Z;
    short Radius;
    ubyte Pri;
    ubyte Arg2;
    ushort StringIndex;
    ushort UniqueID;
    ubyte ObjText;
    ubyte field_1B[4];
    ubyte field_1F;
};

struct NetscanObjective { // sizeof = 20
  ushort CreditCost;
  ushort TextOffset;
  ubyte AnimNo;
  ushort CreditReward;
  ubyte TextLines;
  ubyte X[NETSCAN_OBJECTIVE_POINTS];
  ubyte Z[NETSCAN_OBJECTIVE_POINTS];
  ubyte brobjfld_12;
  ubyte brobjfld_13;
};

#pragma pack()
/******************************************************************************/
extern struct Objective *game_used_objectives;
extern ushort next_used_objective;

extern struct Objective *game_objectives;
extern ushort next_objective;

extern const char *scroll_text;
extern ubyte byte_1C844F;

extern struct NetscanObjective mission_netscan_objectives[MISSION_NETSCAN_OBV_COUNT];
extern ushort next_mission_netscan_objective;

extern struct NetscanObjective netscan_objectives[NETSCAN_OBJECTIVES_MAX_COUNT];
extern ubyte netscan_objectives_count;

int add_used_objective(long mapno, long levelno);

TbBool objective_target_is_to_be_acquired(struct Objective *p_objectv);
TbBool objective_target_is_ally(struct Objective *p_objectv);

TbBool objective_target_is_group(struct Objective *p_objectv);
TbBool objective_target_is_group_to_area(struct Objective *p_objectv);
TbBool objective_target_is_group_to_thing(struct Objective *p_objectv);
TbBool objective_target_is_group_to_vehicle(struct Objective *p_objectv);
TbBool objective_target_is_person(struct Objective *p_objectv);
TbBool objective_target_is_person_to_area(struct Objective *p_objectv);
TbBool objective_target_is_person_to_thing(struct Objective *p_objectv);
TbBool objective_target_is_vehicle(struct Objective *p_objectv);
TbBool objective_target_is_vehicle_to_area(struct Objective *p_objectv);
TbBool objective_target_is_item(struct Objective *p_objectv);
TbBool objective_target_is_item_to_area(struct Objective *p_objectv);
TbBool objective_target_is_object(struct Objective *p_objectv);
TbBool objective_target_is_any_thing(struct Objective *p_objectv);

short test_objective(ushort objectv, ushort show_obj);
ubyte group_not_seen(ushort group);

/** Checks if given thing is within circle defined by parameters.
 * @param thing Thing index to check position.
 * @param x Target circle center X coord.
 * @param y Target circle center Y coord.
 * @param z Target circle center Z coord.
 * @param radius_sq Squared target circle radius.
 */
TbBool thing_arrived_at_obj_radius(ThingIdx thing, int x, int y, int z, int radius_sq);

/** Checks if given thing is within 2D rectangle defined by parameters.
 * @param thing Thing index to check position.
 * @param x1 Target rectangle point 1 X coord.
 * @param z1 Target rectangle point 1 Z coord.
 * @param x2 Target rectangle point 2 X coord.
 * @param z2 Target rectangle point 2 Z coord.
 */
TbBool thing_arrived_at_obj_square2(ThingIdx thing, int x1, int z1, int x2, int z2);

ubyte all_group_arrived_square(ushort group, short x, short z, short x2, int z2);
ubyte mem_group_arrived_square2(struct Thing *p_person, ushort group, short x, short z,
  int x2, int z2, int count);
ubyte mem_group_arrived(ushort group, short x, short y, short z,
  int radius, int count);

/** Fixes parameters within objectives.
 *
 * @return Gives 2 if a parameter was updated, 1 if no update was neccessary, 0 if update failed.
 */
ubyte fix_single_objective(struct Objective *p_objectv, ushort objectv, const char *srctext);

void save_objective_chain_conf(TbFileHandle fh, ushort objectv_head, char *buf, ulong buflen);
int parse_next_used_objective(const char *buf, long buflen, long pri, long mapno, long levelno);

void save_netscan_objectives_conf(TbFileHandle fh, struct NetscanObjective *nsobv_arr,
  ushort nsobv_count, char *buf, ulong buflen);
int parse_next_netscan_objective(const char *buf, long buflen, long nsobv);
int load_netscan_objectives_bin(struct NetscanObjective *nsobv_arr, ubyte mapno, ubyte level);
void load_netscan_objectives(ubyte mapno, ubyte level);
TbResult load_objectives_text(void);
TbBool screen_objective_text_set_failed(void);

/** Prepares objective text. Can also draw objective data and additional info directly on 3D scene.
 *
 * @param objectv Index of the target objective.
 * @param flag Resets objective drawind if 1, otherwise draws the objective.
 */
void draw_objective(ushort objectv, ubyte flag);

void snprint_objective(char *buf, ulong buflen, struct Objective *p_objectv, ushort objectv);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
