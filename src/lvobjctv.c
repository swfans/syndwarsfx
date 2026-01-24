/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file lvobjctv.c
 *     Routines for level objectives handling.
 * @par Purpose:
 *     Implement functions for handling and checking level objectives.
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
#include "lvobjctv.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "bffile.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfini.h"

#include "engincolour.h"

#include "bigmap.h"
#include "campaign.h"
#include "drawtext.h"
#include "keyboard.h"
#include "lvfiles.h"
#include "thing.h"
#include "thing_search.h"
#include "pepgroup.h"
#include "player.h"
#include "scanner.h"
#include "scandraw.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "vehicle.h"
#include "wadfile.h"
#include "swlog.h"
/******************************************************************************/

enum ObjectiveDefFlags {
    ObDF_None = 0x0000,
    /* === What to place in Thing/UniqueId === */
    /** Group index in Thing field */
    ObDF_ReqGroup = 0x0001,
    /** Person reference in Thing and UniqueID fields */
    ObDF_ReqPerson = 0x0002,
    /** Vehicle reference in Thing and UniqueID fields */
    ObDF_ReqVehicle = 0x0004,
    /** Carried Item reference in Thing and UniqueID fields; this also uses
     * Arg2 field, filling it with type of weapon stored in the item */
    ObDF_ReqItem = 0x0008,
    /** Object (ie building) index in Thing field */
    ObDF_ReqObject = 0x0010,
    /** Any Thing reference in Thing and UniqueID fields */
    ObDF_ReqThing = 0x0020,
    /** Count/Amount in Thing field */
    ObDF_ReqCount = 0x0040,
    /* === What to place in Coord/Radius === */
    /** Coordinates in X/Y/Z fields (or only X/Z, if the Y is reused) */
    ObDF_ReqCoord = 0x0100,
    /** Radius/Range in Radius field */
    ObDF_ReqRadius = 0x0200,
    /** Amount in the Y field; if used, it leaves only X/Z for coords */
    ObDF_ReqAmountY = 0x0400,
    /** Thing index in the Y field; if used, it leaves only X/Z for coords */
    ObDF_ReqThingY = 0x0800,
    /* === What to place in Arg2 === */
    ObDF_ReqAmount = 0x1000,
    ObDF_ReqSecTng = 0x2000, // TODO this can cause bugs, use only Thing for thing index
    ObDF_ReqSecGrp = 0x4000,
    /* === Type of target, used for displaying on scanner; default is enemy === */
    /** The target is to be acquired (picked/persuaded). */
    ObDF_IsAcquire = 0x01000000,
    /** The target is an ally (to be protected). */
    ObDF_IsAlly = 0x02000000,
};

struct ObjectiveDef {
    const char *CmdName;
    const char *DefText;
    ulong Flags;
};

struct ObjectiveDef objectv_defs[] = {
    /* Unreachable. */
    {"GAME_OBJ_NONE",		"DO NOTHING",		ObDF_None },
    /* Require the target person to be dead or destroyed. */
    {"GAME_OBJ_P_DEAD",		"ASSASSINATE",		ObDF_ReqPerson },
    /* Require whole group to be neutralized. */
    {"GAME_OBJ_ALL_G_DEAD",	"ELIMINATE GROUP",	ObDF_ReqGroup },
    /* Require at least specified amount of group members to be dead or destroyed. */
    {"GAME_OBJ_MEM_G_DEAD",	"KILL GROUP MEM",	ObDF_ReqGroup|ObDF_ReqAmount },
    /* Require the Person and the Thing to be within Radius around each other. */
    {"GAME_OBJ_P_NEAR",		"RENDEZVOUS",		ObDF_ReqPerson|ObDF_ReqThingY|ObDF_ReqRadius|ObDF_IsAcquire },
    /* Require specified amount of group members to be within radius around given thing. */
    {"GAME_OBJ_MEM_G_NEAR",	"RENDEZVOUS MEM",	ObDF_ReqThing|ObDF_ReqSecGrp|ObDF_ReqAmountY|ObDF_ReqRadius|ObDF_IsAcquire },
    /* Require the target person to be within given radius around given coordinates. */
    {"GAME_OBJ_P_ARRIVES",	"GOTO LOCATION",	ObDF_ReqPerson|ObDF_ReqCoord|ObDF_ReqRadius },
    /* Require at least specified amount of group members to be within radius around given coords. */
    {"GAME_OBJ_MEM_G_ARRIVES", "GOTO LOCATION", ObDF_ReqGroup|ObDF_ReqCoord|ObDF_ReqRadius|ObDF_ReqAmount },
    /* Require all of group members to be within radius around given coords. */
    {"GAME_OBJ_ALL_G_ARRIVES", "ALL GOTO LOCATION", ObDF_ReqGroup|ObDF_ReqCoord|ObDF_ReqRadius },
    /* Require target person to be within the group belonging to local player. */
    {"GAME_OBJ_PERSUADE_P",	"PERSUADE",			ObDF_ReqPerson|ObDF_IsAcquire },
    /* Require at least specified amount of group members to be within the local player group. */
    {"GAME_OBJ_PERSUADE_MEM_G", "PERSUADE GANG MEM", ObDF_ReqGroup|ObDF_ReqAmount|ObDF_IsAcquire },
    /* Require all of group members to be persuaded. */
    {"GAME_OBJ_PERSUADE_ALL_G", "PERSUADE ALL GANG", ObDF_ReqGroup|ObDF_IsAcquire },
    /* Require specified amount of game turns to pass. */
    {"GAME_OBJ_TIME",		"TIMER",			ObDF_ReqCount },
    /* Require specified carried item to change owner to a person belonging to local player.
     * If the item is a weapon, it should be unique for the level. */
    {"GAME_OBJ_GET_ITEM",	"COLLECT ITEM",		ObDF_ReqItem|ObDF_IsAlly },
    /* Unreachable. Require specified item to be used? */
    {"GAME_OBJ_USE_ITEM",	"USE ITEM",			ObDF_ReqItem|ObDF_IsAlly },
    /* Unreachable. Require acquiring specified amount of funds? */
    {"GAME_OBJ_FUNDS",		"GET BULLION",		ObDF_None },
    /* Require given thing to have DESTROYED flag set.
     * Coords need to be provided, rather than UniqueId, to find the object thing in case it changed. */
    {"GAME_OBJ_DESTROY_OBJECT", "DESTROY BUILDING", ObDF_ReqObject|ObDF_ReqCoord },
    /** Require the target person to either be DESTROYED or change owner to local player group. */
    {"GAME_OBJ_PKILL_P",	"NEUTRALISE",		ObDF_ReqPerson },
    /* Require all of group members to be killed or persuaded by local player group. */
    {"GAME_OBJ_PKILL_MEM_G", "NEUTRALISE MEM",	ObDF_ReqGroup|ObDF_ReqAmount },
    /* Require all of group members to either be DESTROYED or change owner to local player group. */
    {"GAME_OBJ_PKILL_ALL_G", "NEUTRALISE G",	ObDF_ReqGroup },
    /* Unreachable. Require using P.A. Network? */
    {"GAME_OBJ_USE_PANET",	"USE P.A.NET",		ObDF_None },
    /* Unreachable. */
    {"GAME_OBJ_UNUSED_21",	"UNEXPECT 21",		ObDF_None },
    /* Fail if any of group members are dead/destroyed. The only negative objective. */
    {"GAME_OBJ_PROTECT_G",	"PROTECT GROUP",	ObDF_ReqGroup|ObDF_IsAlly },
    /* Require all of group members to change owner to specified person. */
    {"GAME_OBJ_P_PERS_G",	"PEEP PERSUADE ALL", ObDF_ReqGroup|ObDF_ReqSecTng|ObDF_IsAcquire },
    /* Require all of group members to either be dead/destroyed or within specified vehicle. */
    {"GAME_OBJ_ALL_G_USE_V", "ALL USE VEHICLE",	ObDF_ReqVehicle|ObDF_ReqSecGrp|ObDF_IsAlly },
    /* Require at least specified amount of group members to be within specified vehicle. */
    {"GAME_OBJ_MEM_G_USE_V", "MEM USE VEHICLE",	ObDF_ReqVehicle|ObDF_ReqSecGrp|ObDF_ReqAmountY|ObDF_IsAlly },
    /* Require the target vehicle to be within given radius around given coordinates. */
    {"GAME_OBJ_V_ARRIVES",	"DRIVE TO LOCATION",ObDF_ReqVehicle|ObDF_ReqCoord|ObDF_ReqRadius },
    /* Require given thing to have DESTROYED flag set. */
    {"GAME_OBJ_DESTROY_V", "DESTROY VEHICLE",   ObDF_ReqVehicle },
    /* Require the target item to be within given radius around given coordinates, either dropped or carried.
     * If the item is a weapon, it should be unique for the level. */
    {"GAME_OBJ_ITEM_ARRIVES", "BRING TO LOCATION",	ObDF_ReqItem|ObDF_ReqCoord|ObDF_ReqRadius },
    {NULL,					NULL,				ObDF_None },
};

/* deprecated */
const char *gameobjctv_names[] = {
    "GAME_OBJ_NONE",
    "GAME_OBJ_P_DEAD",
    "GAME_OBJ_ALL_G_DEAD",
    "GAME_OBJ_MEM_G_DEAD",
    "GAME_OBJ_P_NEAR",
    "GAME_OBJ_MEM_G_NEAR",
    "GAME_OBJ_P_ARRIVES",
    "GAME_OBJ_MEM_G_ARRIVES",
    "GAME_OBJ_ALL_G_ARRIVES",
    "GAME_OBJ_PERSUADE_P",
    "GAME_OBJ_PERSUADE_MEM_G",
    "GAME_OBJ_PERSUADE_ALL_G",
    "GAME_OBJ_TIME",
    "GAME_OBJ_GET_ITEM",
    "GAME_OBJ_USE_ITEM",
    "GAME_OBJ_FUNDS",
    "GAME_OBJ_DESTROY_OBJECT",
    "GAME_OBJ_PKILL_P",
    "GAME_OBJ_PKILL_MEM_G",
    "GAME_OBJ_PKILL_ALL_G",
    "GAME_OBJ_USE_PANET",
    "GAME_OBJ_UNUSED_21",
    "GAME_OBJ_PROTECT_G",
    "GAME_OBJ_P_PERS_G",
    "GAME_OBJ_ALL_G_USE_V",
    "GAME_OBJ_MEM_G_USE_V",
    "GAME_OBJ_V_ARRIVES",
    "GAME_OBJ_DESTROY_V",
    "GAME_OBJ_ITEM_ARRIVES",
};

enum ObjectiveConfigParam {
    ObvP_Thing = 1,
    ObvP_Group,
    ObvP_Count,
    ObvP_UniqueID,
    ObvP_Coord,
    ObvP_CoordY, /**< Field only for usage verification, no corresponding text param */
    ObvP_Radius,
    ObvP_Amount,
    ObvP_SecGroup,
    ObvP_SecThing,
    ObvP_Arg2,
    ObvP_StringIndex,
    ObvP_Flags,
    ObvP_TextId,
};

const struct TbNamedEnum missions_conf_objective_params[] = {
  {"Thing",			ObvP_Thing},
  {"Group",			ObvP_Group},
  {"Count",			ObvP_Count},
  {"UniqueID",		ObvP_UniqueID},
  {"Coord",			ObvP_Coord},
  {"Radius",		ObvP_Radius},
  {"Amount",		ObvP_Amount},
  {"SecGroup",		ObvP_SecGroup},
  {"SecThing",		ObvP_SecThing},
  {"Arg2",			ObvP_Arg2},
  {"StringIndex",	ObvP_StringIndex},
  {"Flags",			ObvP_Flags},
  {"TextId",		ObvP_TextId},
  {NULL,			0},
};

enum NetObjctvConfigParam {
    NOvP_CreditCost = 1,
    NOvP_AnimNo,
    NOvP_Coord,
    NOvP_CreditReward,
    NOvP_Fld12,
    NOvP_Fld13,
};

const struct TbNamedEnum missions_conf_netscan_objctv_params[] = {
  {"Cost",		NOvP_CreditCost},
  {"Anim",		NOvP_AnimNo},
  {"Coord",		NOvP_Coord},
  {"Reward",	NOvP_CreditReward},
  {"Fld12",		NOvP_Fld12},
  {"Fld13",		NOvP_Fld13},
  {NULL,		0},
};

#define PARAM_TOKEN_MAX 16
#define COMMAND_TOKEN_MAX 32
#define OBJECTIVE_TEXT_MAX 300

struct NetscanObjective mission_netscan_objectives[MISSION_NETSCAN_OBV_COUNT];
ushort next_mission_netscan_objective;

ushort next_objective = 1;
ushort next_used_objective = 1;

extern ulong dword_1C8460;
extern ulong dword_1C8464;
extern short draw_objectv_x;
extern short draw_objectv_y;

char *objective_text[OBJECTIVE_TEXT_MAX];

int add_used_objective(long mapno, long levelno)
{
    struct Objective *p_objectv;
    int objectv;

    objectv = next_used_objective;
    next_used_objective++;

    p_objectv = &game_used_objectives[objectv];
    LbMemorySet(p_objectv, 0, sizeof(struct Objective));
    p_objectv->Type = 0;
    p_objectv->Pri = 0;
    p_objectv->Map = mapno;
    p_objectv->Level = LEVEL_NUM_STRAIN(levelno);
    p_objectv->Status = ObvStatu_UNDECIDED;

    return objectv;
}

TbBool objective_target_is_to_be_acquired(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_IsAcquire) != 0);
}

TbBool objective_target_is_ally(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_IsAlly) != 0);
}

TbBool objective_target_is_group(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqGroup) != 0);
}

TbBool objective_target_is_group_to_area(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqGroup) != 0) &&
      ((p_odef->Flags & ObDF_ReqCoord) != 0) &&
      ((p_odef->Flags & ObDF_ReqRadius) != 0);
}

TbBool objective_target_is_group_to_thing(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqSecGrp) != 0) &&
      ((p_odef->Flags & ObDF_ReqThing) != 0) &&
      ((p_odef->Flags & ObDF_ReqRadius) != 0);
}

TbBool objective_target_is_person(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqPerson) != 0);
}

TbBool objective_target_is_person_to_area(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqPerson) != 0) &&
      ((p_odef->Flags & ObDF_ReqCoord) != 0) &&
      ((p_odef->Flags & ObDF_ReqRadius) != 0);
}

TbBool objective_target_is_person_to_thing(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqPerson) != 0) &&
      ((p_odef->Flags & ObDF_ReqThingY) != 0) &&
      ((p_odef->Flags & ObDF_ReqRadius) != 0);
}

TbBool objective_target_is_vehicle(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqVehicle) != 0);
}

TbBool objective_target_is_vehicle_to_area(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqVehicle) != 0) &&
      ((p_odef->Flags & ObDF_ReqCoord) != 0) &&
      ((p_odef->Flags & ObDF_ReqRadius) != 0);
}

TbBool objective_target_is_group_to_vehicle(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqSecGrp) != 0) &&
      ((p_odef->Flags & ObDF_ReqVehicle) != 0);
}

TbBool objective_target_is_item(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqItem) != 0);
}

TbBool objective_target_is_item_to_area(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqItem) != 0) &&
      ((p_odef->Flags & ObDF_ReqCoord) != 0) &&
      ((p_odef->Flags & ObDF_ReqRadius) != 0);
}

TbBool objective_target_is_object(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqObject) != 0);
}

TbBool objective_target_is_any_thing(struct Objective *p_objectv)
{
    struct ObjectiveDef *p_odef;
    p_odef = &objectv_defs[p_objectv->Type];
    return ((p_odef->Flags & ObDF_ReqPerson) != 0) ||
      ((p_odef->Flags & ObDF_ReqVehicle) != 0) ||
      ((p_odef->Flags & ObDF_ReqItem) != 0) ||
      ((p_odef->Flags & ObDF_ReqObject) != 0) ||
      ((p_odef->Flags & ObDF_ReqThing) != 0);
}

void draw_objective_group_whole_on_engine_scene(ushort group)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ubyte colk;

    colk = dword_1C8460 & 7;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
    }
}

void draw_objective_group_non_flag2_on_engine_scene(ushort group)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ubyte colk;

    colk = dword_1C8460 & 7;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if ((p_thing->Flag & TngF_Destroyed) == 0) {
            draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        }
    }
}

void draw_objective_group_non_pers_on_engine_scene(ushort group)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ubyte colk;

    colk = dword_1C8460 & 7;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if ((p_thing->Flag & TngF_Persuaded) == 0) {
            draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        }
    }
}

void draw_objective_group_not_own_by_plyr_on_engine_scene(ushort group, ushort plyr)
{
    short plyagent, plygroup;
    ThingIdx thing;
    struct Thing *p_thing;
    ubyte colk;

    colk = dword_1C8460 & 7;
    plyagent = players[plyr].DirectControl[0];
    plygroup = things[plyagent].U.UPerson.Group;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (((p_thing->Flag & TngF_Persuaded) == 0) || things[p_thing->Owner].U.UPerson.Group != plygroup) {
            if ((p_thing->Flag & TngF_Destroyed) == 0)
                draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        }
    }
}

void draw_objective_group_not_own_by_pers_on_engine_scene(ushort group, short owntng)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ubyte colk;

    colk = dword_1C8460 & 7;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (((p_thing->Flag & TngF_Persuaded) == 0) && (p_thing->Owner != owntng)) {
            if ((p_thing->Flag & TngF_Destroyed) == 0)
                draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        }
    }
}

void draw_objective_dirctly_on_engine_scene(ushort objectv)
{
    struct Objective *p_objectv;
    ThingIdx thing;
    short group;
    ubyte colk;

    p_objectv = &game_used_objectives[objectv];

    colk = dword_1C8460 & 7;

    switch (p_objectv->Type)
    {
    case GAME_OBJ_PERSUADE_ALL_G:
    case GAME_OBJ_PERSUADE_MEM_G:
        group = p_objectv->Thing;
        draw_objective_group_non_pers_on_engine_scene(group);
        break;
    case GAME_OBJ_GET_ITEM:
    case GAME_OBJ_DESTROY_OBJECT:
        thing = p_objectv->Thing;
        draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        break;
    case GAME_OBJ_TIME:
    case GAME_OBJ_USE_ITEM:
    case GAME_OBJ_FUNDS:
    case GAME_OBJ_PKILL_P:
        break;
    case GAME_OBJ_PKILL_ALL_G:
    case GAME_OBJ_PKILL_MEM_G:
        group = p_objectv->Thing;
        draw_objective_group_not_own_by_plyr_on_engine_scene(group, local_player_no);
        draw_objectv_y += 8;
        break;
    case GAME_OBJ_P_PERS_G:
        group = p_objectv->Thing;
        draw_objective_group_not_own_by_pers_on_engine_scene(group, p_objectv->Arg2);
        break;
    case GAME_OBJ_ALL_G_USE_V:
    case GAME_OBJ_MEM_G_USE_V:
        group = p_objectv->Arg2;
        draw_objective_group_whole_on_engine_scene(group);
        draw_objectv_y += 8;
        thing = p_objectv->Thing;
        draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk+1]);
        break;
    case GAME_OBJ_PROTECT_G:
    case GAME_OBJ_ALL_G_DEAD:
    case GAME_OBJ_MEM_G_DEAD:
        group = p_objectv->Thing;
        draw_objective_group_non_flag2_on_engine_scene(group);
        break;
    case GAME_OBJ_P_NEAR:
    case GAME_OBJ_USE_PANET:
    case GAME_OBJ_UNUSED_21:
        break;
    case GAME_OBJ_P_DEAD:
    case GAME_OBJ_DESTROY_V:
        thing = p_objectv->Thing;
        draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        break;
    case GAME_OBJ_MEM_G_NEAR:
    case GAME_OBJ_MEM_G_ARRIVES:
    case GAME_OBJ_ALL_G_ARRIVES:
        // Arrival place
        draw_map_flat_circle(p_objectv->X, p_objectv->Y, p_objectv->Z, p_objectv->Radius << 6, colour_lookup[ColLU_RED]);
        unkn_draw_transformed_point(draw_objectv_x - 10, draw_objectv_y,
          p_objectv->X, p_objectv->Y, p_objectv->Z, colour_lookup[colk]);
        break;
    case GAME_OBJ_P_ARRIVES:
    case GAME_OBJ_V_ARRIVES:
    case GAME_OBJ_ITEM_ARRIVES:
        // Person / Vehicle / Item
        thing = p_objectv->Thing;
        draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        // Arrival place
        draw_map_flat_circle(p_objectv->X, p_objectv->Y, p_objectv->Z, p_objectv->Radius << 6, colour_lookup[ColLU_RED]);
        unkn_draw_transformed_point(draw_objectv_x - 10, draw_objectv_y,
          p_objectv->X, p_objectv->Y, p_objectv->Z, colour_lookup[colk]);
        break;
    case GAME_OBJ_PERSUADE_P:
        thing = p_objectv->Thing;
        draw_objective_point(draw_objectv_x - 10, draw_objectv_y, thing, 0, colour_lookup[colk]);
        break;
    }
}

void draw_objective(ushort objectv, ubyte flag)
{
    struct Objective *p_objectv;
    struct ObjectiveDef *p_odef;
    ubyte colk;

    p_objectv = &game_used_objectives[objectv];
    if (flag == 1)
    {
        dword_1C8464 = 0;
        draw_objectv_x = 200;
        draw_objectv_y = 10;
        dword_1C8460 = 0;
        return;
    }

    if (p_objectv->Type < sizeof(objectv_defs)/sizeof(objectv_defs[0]))
        p_odef = &objectv_defs[p_objectv->Type];
    else
        p_odef = &objectv_defs[0];

    if (!byte_1C844F)
    {
        if ((p_objectv->Flags & GObjF_HIDDEN) != 0)
            scroll_text = "-";
        else if (p_objectv->ObjText != 0) {
            assert(p_objectv->ObjText < OBJECTIVE_TEXT_MAX);
            scroll_text = objective_text[p_objectv->ObjText];
        } else {
            scroll_text = p_odef->DefText;
        }
        ++dword_1C8464;
    }
    else
    {
        char locstr[112];
        short group;

        group = p_objectv->Thing;
        if (group >= PEOPLE_GROUPS_COUNT) group = 0;
        sprintf(locstr, "[%d] %20s t %d id %d", group_actions[group].Alive,
          p_odef->CmdName, p_objectv->Thing, p_objectv->UniqueID);
        if (gameturn & 4)
            colk = dword_1C8460 & 7;
        else
            colk = 1;
        draw_text(draw_objectv_x, draw_objectv_y, locstr, colour_lookup[colk]);
    }

    if (byte_1C844F) {
        draw_objective_dirctly_on_engine_scene(objectv);
    }
    draw_objectv_y += 8;
    dword_1C8460++;
}

TbBool screen_objective_text_set_failed(void)
{
    if (ingame.fld_unkCB5 > 0) {
        scroll_text = objective_text[ingame.fld_unkCB5];
        return true;
    }
    return false;
}

/** Crude version of thing_arrived_at_objectv(), deprecated.
 */
TbBool thing_arrived_at_obj(ThingIdx thing, struct Objective *p_objectv)
{
    return thing_is_within_circle(thing, p_objectv->X, p_objectv->Z, p_objectv->Radius << 6);
}

/** Returns whether given thing has arrived at given objective position.
 *
 * @param thing Thing index (of any type) to to check.
 * @param p_objectv Pointer to the objective which XYZ coords and radius will be checked.
 */
TbBool thing_arrived_at_objectv(ThingIdx thing, struct Objective *p_objectv)
{
    if (thing == 0)
        return false;
    if (thing_is_destroyed(thing))
        return false;
    return thing_is_within_circle(thing, p_objectv->X, p_objectv->Z, p_objectv->Radius << 6);
}

/** Returns whether given item (which may be weapon) has arrived at given objective position.
 *
 * @param thing Thing index for the item to check.
 * @param wtype Weapon type, in case it was picked up and therfore is no longer at expected Thing index.
 * @param p_objectv Pointer to the objective which XYZ coords and radius will be checked.
 */
TbBool item_arrived_at_objectv(ThingIdx thing, WeaponType wtype, struct Objective *p_objectv)
{
    struct SimpleThing *p_sthing;

    if (thing >= 0)
        return false;

    p_sthing = &sthings[thing];

    if ((p_sthing->Type == SmTT_DROPPED_ITEM) || (p_sthing->Type == SmTT_CARRIED_ITEM))
    {
        // Make sure the dropped thing exists, and was not reused for a different dropped item
        if (!thing_is_destroyed(thing) && (p_sthing->U.UWeapon.WeaponType == wtype))
            // Note that having two identical items in game (ie. two same weapons) could lead to
            // the 2nd one accidentally dropping into thing slot from the first, and locking out
            // this objective; make sure there is only one such item on a level to avoid that
            return thing_is_within_circle(thing, p_objectv->X, p_objectv->Z, p_objectv->Radius << 6);
    }
    // If the target is no longer a correct thing, then it is now either carried weapon or
    // a different dropped weapon (dropping it created another thing)
    thing = find_dropped_weapon_within_circle(p_objectv->X, p_objectv->Z, p_objectv->Radius << 6, wtype);
    if (thing != 0)
        return true;
    thing = find_person_carrying_weapon_within_circle(p_objectv->X, p_objectv->Z, p_objectv->Radius << 6, wtype);
    return (thing != 0);
}

/** Returns if given item, or the weapon which it represented, is carried
 * by given player.
 *
 * Many pickups are CARRIED_ITEM things, converted from DROPPED_ITEM.
 * But for weapons, the DROPPED_ITEM is deleted rather than converted.
 * So to make the function quasi-work for weapons, we check if the player
 * acquired said weapon. We can't check Unique ID to make sure this is the
 * exact same weapon instance, but that should not matter in most practical
 * uses of the objective.
 */
TbBool item_is_carried_by_player(ThingIdx thing, WeaponType wtype, ushort plyr)
{
    struct SimpleThing *p_sthing;
    short plyagent, plygroup;
    struct Thing *p_person;
    PlayerInfo *p_player;
    int i;

    if (thing >= 0)
        return false;

    p_sthing = &sthings[thing];
    p_player = &players[plyr];

    if ((p_sthing->Type == SmTT_DROPPED_ITEM) &&
      (p_sthing->U.UWeapon.WeaponType == wtype))
        return false;

    plyagent = p_player->DirectControl[0];
    plygroup = things[plyagent].U.UPerson.Group;

    if (p_sthing->Type == SmTT_CARRIED_ITEM)
    {
        p_person = &things[p_sthing->U.UWeapon.Owner];
        return (p_person->U.UPerson.Group == plygroup);
    }

    for (i = 0; i < AGENTS_SQUAD_MAX_COUNT; i++)
    {
        struct Thing *p_agent;

        p_agent = p_player->MyAgent[i];
        if (p_agent->Type != TT_PERSON) continue;
        if (person_carries_weapon(p_agent, wtype))
            return true;
    }
    return false;
}

ubyte group_not_seen(ushort group)
{
    ubyte ret;
    asm volatile (
      "call ASM_group_not_seen\n"
        : "=r" (ret) : "a" (group));
    return ret;
}

TbBool thing_arrived_at_obj_radius(ThingIdx thing, int x, int y, int z, int radius_sq)
{
#if 0
    ubyte ret;
    asm volatile (
      "push %5\n"
      "call ASM_thing_arrived_at_obj_radius\n"
        : "=r" (ret) : "a" (thing), "d" (x), "b" (y), "c" (z), "g" (radius_sq));
    return ret;
#endif
    int dist_x, dist_z;

    if (thing >= 0)
    {
        struct Thing *p_thing;
        p_thing = &things[thing];
        dist_x = (p_thing->X >> 8) - x;
        dist_z = (p_thing->Z >> 8) - z;
    }
    else
    {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        dist_x = (p_sthing->X >> 8) - x;
        dist_z = (p_sthing->Z >> 8) - z;
    }
    return dist_x * dist_x + dist_z * dist_z < radius_sq;
}

TbBool thing_arrived_at_obj_square2(ThingIdx thing, int x1, int z1, int x2, int z2)
{
    short cor_x, cor_z;

    get_thing_position_mapcoords(&cor_x, NULL, &cor_z, thing);

    if ((cor_x > x1) && (cor_x < x2))
    {
        if ((cor_z > z1) && (cor_z < z2))
        {
            return true;
        }
    }
    return false;
}

ubyte all_group_arrived_square(ushort group, short x, short z, short x2, int z2)
{
    ubyte ret;
    asm volatile (
      "push %5\n"
      "call ASM_all_group_arrived_square\n"
        : "=r" (ret) : "a" (group), "d" (x), "b" (z), "c" (x2), "g" (z2));
    return ret;
}

ubyte mem_group_arrived_square2(struct Thing *p_person, ushort group, short x, short z,
  int x2, int z2, int count)
{
    ubyte ret;
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_mem_group_arrived_square2\n"
        : "=r" (ret) : "a" (p_person), "d" (group), "b" (x), "c" (z), "g" (x2), "g" (z2), "g" (count));
    return ret;
}

ubyte mem_group_arrived(ushort group, short x, short y, short z,
  int radius, int count)
{
    ubyte ret;
    asm volatile (
      "push %6\n"
      "push %5\n"
      "call ASM_mem_group_arrived\n"
        : "=r" (ret) : "a" (group), "d" (x), "b" (y), "c" (z), "g" (radius), "g" (count));
    return ret;
}

TbBool person_is_near_thing(ThingIdx neartng, ThingIdx thing, ushort radius)
{
    short nearX, nearZ;

    if ((neartng == 0) || person_is_dead(neartng) || thing_is_destroyed(neartng))
        return false;
    if (neartng <= 0) {
        struct SimpleThing *p_neartng;
        p_neartng = &sthings[neartng];
        nearX = p_neartng->X;
        nearZ = p_neartng->Z;
    } else {
        struct Thing *p_neartng;
        p_neartng = &things[neartng];
        nearX = p_neartng->X;
        nearZ = p_neartng->Z;
    }

    if ((thing == 0) || person_is_dead(thing) || thing_is_destroyed(thing))
        return false;

    return (thing_is_within_circle(thing, nearX, nearZ, radius << 6));
}


TbBool group_members_arrived_at_objectv(ushort group, struct Objective *p_objectv, ushort amount)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (thing_arrived_at_obj(thing, p_objectv))
            n++;
        if (n >= amount)
            return true;
    }
    return false;
}

ubyte fix_single_objective(struct Objective *p_objectv, ushort objectv, const char *srctext)
{
    struct ObjectiveDef *p_odef;
    ThingIdx thing;
    ubyte ret;

    // Skip objectives for other levels
    if (p_objectv->Level != LEVEL_NUM_STRAIN(current_level))
        return 1;
    if (p_objectv->Map != current_map)
        return 1;

    p_odef = &objectv_defs[p_objectv->Type];

    ret = 1;
    if ((p_odef->Flags & ObDF_ReqGroup) != 0)
    {
        if ((p_objectv->Thing < 0) || (p_objectv->Thing >= PEOPLE_GROUPS_COUNT)) {
            LOGERR("Objv%s%d = %s Group(%hd) out of range",
              srctext, objectv, p_odef->CmdName, p_objectv->Thing);
            p_objectv->Thing = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqPerson) != 0)
    {
        {
            thing = search_things_for_index(p_objectv->Thing);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_PERSON)
                    thing = 0;
                else if (p_thing->U.UPerson.UniqueID != p_objectv->UniqueID)
                    thing = 0;
            }
        }
        if (thing == 0) {
            thing = search_things_for_uniqueid(p_objectv->UniqueID, 1);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_PERSON)
                    thing = 0;
            }
        }
#if 0 // If UniqueID does not match, it is probably better to give up
        if (thing == 0) {
            thing = p_objectv->Thing;
            if ((thing <= 0) || (thing >= THINGS_LIMIT)) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_PERSON)
                    thing = 0;
            }
        }
#endif
        if (thing != 0) {
            p_objectv->Thing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Objv%s%d = %s Thing(%hd,%hu) not found",
              srctext, objectv, p_odef->CmdName,
              p_objectv->Thing, p_objectv->UniqueID);
            p_objectv->Thing = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqVehicle) != 0)
    {
        {
            thing = search_things_for_index(p_objectv->Thing);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_VEHICLE)
                    thing = 0;
                else if (p_thing->U.UVehicle.UniqueID != p_objectv->UniqueID)
                    thing = 0;
            }
        }
        if (thing == 0) {
            thing = search_things_for_uniqueid(p_objectv->UniqueID, 1);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_VEHICLE)
                    thing = 0;
            }
        }
        if (thing != 0) {
            p_objectv->Thing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Objv%s%d = %s Thing(%hd,%hu) not found",
              srctext, objectv, p_odef->CmdName,
              p_objectv->Thing, p_objectv->UniqueID);
            p_objectv->Thing = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqItem) != 0)
    {
        {
            thing = search_things_for_index(p_objectv->Thing);
            // One mistake which often happens on the map, is that we have static
            // lights assigned as items. This should be really fixed in the level,
            // but there is also no reason not to try fix it here.
            if (thing < 0) {
                struct SimpleThing *p_sthing = &sthings[thing];
                if (p_sthing->Type == SmTT_STATIC) {
                    LOGWARN("Objv%s%d = %s target Thing(%hd,%hu) index %hd is a static; jumping to parent",
                      srctext, objectv, p_odef->CmdName,
                      p_objectv->Thing, p_objectv->UniqueID, thing);
                    thing = p_sthing->Parent;
                }
            }
            if (thing >= 0) {
                thing = 0;
            } else {
                struct SimpleThing *p_sthing = &sthings[thing];
                if ((p_sthing->Type != SmTT_CARRIED_ITEM) && (p_sthing->Type != SmTT_DROPPED_ITEM))
                    thing = 0;
                else if (p_sthing->UniqueID != p_objectv->UniqueID)
                    thing = 0;
            }
        }
        if (thing == 0) {
            thing = search_things_for_uniqueid(p_objectv->UniqueID, 0);
            if (thing >= 0) {
                thing = 0;
            } else {
                struct SimpleThing *p_sthing = &sthings[thing];
                if ((p_sthing->Type != SmTT_CARRIED_ITEM) && (p_sthing->Type != SmTT_DROPPED_ITEM))
                    thing = 0;
                else if (p_sthing->UniqueID != p_objectv->UniqueID)
                    thing = 0;
            }
        }
        if (thing != 0) {
            struct SimpleThing *p_sthing = &sthings[thing];
            p_objectv->Thing = thing;
            p_objectv->Arg2 = p_sthing->U.UWeapon.WeaponType;
            if (ret) ret = 2;
        } else {
            LOGERR("Objv%s%d = %s Thing(%hd,%hu) not found",
              srctext, objectv, p_odef->CmdName,
              p_objectv->Thing, p_objectv->UniqueID);
            p_objectv->Thing = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqObject) != 0)
    {
        {
            thing = find_nearest_object2(p_objectv->X, p_objectv->Z, 0);
        }
        if (thing == 0) {
            thing = search_things_for_index(p_objectv->Thing);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_BUILDING)
                    thing = 0;
            }
        }
        if (thing != 0) {
            p_objectv->Thing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Objv%s%d = %s Thing(%hd,%hu) not found",
              srctext, objectv, p_odef->CmdName,
              p_objectv->Thing, p_objectv->UniqueID);
            p_objectv->Thing = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqThing) != 0)
    {
        {
            thing = search_things_for_index(p_objectv->Thing);
        }
        if (thing == 0) { // Not all things have UniqueID, but worth to try
            thing = search_things_for_uniqueid(p_objectv->UniqueID, 1);
        }
        if (thing == 0) {
            thing = search_things_for_uniqueid(p_objectv->UniqueID, 0);
        }
        if (thing != 0) {
            p_objectv->Thing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Objv%s%d = %s Thing(%hd,%hu) not found",
              srctext, objectv, p_odef->CmdName,
              p_objectv->Thing, p_objectv->UniqueID);
            p_objectv->Thing = 0;
            ret = 0;
        }
    }

    if ((p_odef->Flags & ObDF_ReqSecGrp) != 0)
    {
        if ((p_objectv->Arg2 <= 0) || (p_objectv->Arg2 >= PEOPLE_GROUPS_COUNT)) {
            LOGERR("Objv%s%d = %s SecGroup(%d) out of range",
              srctext, objectv, p_odef->CmdName, (int)p_objectv->Arg2);
            p_objectv->Arg2 = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqSecTng) != 0)
    {
        {
            thing = search_things_for_index(p_objectv->Arg2);
        }
        if (thing > 255) { // TODO when possible , stop storing thing index in Arg2
            LOGERR("Objv%s%d = %s SecThing(%d) exceeds 8-bit Arg2 range",
              srctext, objectv, p_odef->CmdName, (int)p_objectv->Arg2);
            ret = 0;
        } else if (thing != 0) {
            p_objectv->Arg2 = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Objv%s%d = %s SecThing(%d) not found",
              srctext, objectv, p_odef->CmdName, (int)p_objectv->Arg2);
            p_objectv->Arg2 = 0;
            ret = 0;
        }
    }
    else if ((p_odef->Flags & ObDF_ReqAmount) != 0)
    {
        if (p_objectv->Arg2 < 1) {
            LOGERR("Objv%s%d = %s Amount(%d) out of range",
              srctext, objectv, p_odef->CmdName, (int)p_objectv->Arg2);
            p_objectv->Arg2 = 1;
            ret = 0;
        }
    }

    return ret;
}

short test_objective(ushort objectv, ushort show_obj)
{
    struct Objective *p_objectv;
    ThingIdx thing, thing2;
    short group, amount;

    if (show_obj == 2)
    {
        p_objectv = &game_used_lvl_objectives[objectv];
        if ((ingame.UserFlags & UsrF_Cheats) != 0)
        {
            if (is_key_pressed(KC_SLASH, KMod_ALT))
            {
                if (p_objectv->Status != ObvStatu_FAILED)
                {
                    clear_key_pressed(KC_SLASH);
                    p_objectv->Status = ObvStatu_FAILED;
                }
            }
        }
        if (p_objectv->Status == ObvStatu_FAILED)
            return 1;
        if (p_objectv->Status == ObvStatu_COMPLETED)
            return 0;
    }
    else
    {
        p_objectv = &game_used_objectives[objectv];
        if ((ingame.UserFlags & UsrF_Cheats) != 0)
        {
            if (is_key_pressed(KC_SLASH, KMod_ALT))
            {
                if (p_objectv->Status != ObvStatu_FAILED)
                {
                    clear_key_pressed(KC_SLASH);
                    p_objectv->Status = ObvStatu_FAILED;
                }
            }
        }
        if (p_objectv->Status == ObvStatu_FAILED)
            return 1;
        if (((p_objectv->Flags & GObjF_HIDDEN) == 0) && word_1C8446 && (show_obj != 0))
            draw_objective(objectv, 0);
        if (show_obj != 0)
            add_signal_to_scanner(p_objectv, 0);
        if ((p_objectv->Flags & GObjF_CANT_MET) != 0)
            return 1;
        if (p_objectv->Status == ObvStatu_COMPLETED)
            return -1;
    }

    switch (p_objectv->Type)
    {
    case GAME_OBJ_P_DEAD:
        thing = p_objectv->Thing;
        if (person_is_dead(thing) || thing_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        p_objectv->Status = ObvStatu_UNDECIDED;
        break;
    case GAME_OBJ_ALL_G_DEAD:
        group = p_objectv->Thing;
        if (group_actions[group].Alive <= 0) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_MEM_G_DEAD:
        group = p_objectv->Thing;
        amount = p_objectv->Arg2;
        if (group_has_no_less_members_dead(group, amount)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_P_NEAR:
        thing = p_objectv->Thing;
        thing2 = p_objectv->Y;
        if (person_is_near_thing(thing, thing2, p_objectv->Radius)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_MEM_G_NEAR:
        thing = p_objectv->Thing;
        group = p_objectv->Arg2;
        amount = p_objectv->Y;
        if (group_has_no_less_members_near_thing(thing, group, amount, p_objectv->Radius)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_P_ARRIVES:
        thing = p_objectv->Thing;
        if (thing_arrived_at_objectv(thing, p_objectv)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_MEM_G_ARRIVES:
        group = p_objectv->Thing;
        amount = p_objectv->Arg2;
        if (group_members_arrived_at_objectv(group, p_objectv, amount)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_ALL_G_ARRIVES:
        group = p_objectv->Thing;
        if (all_group_arrived(group, p_objectv->X, p_objectv->Y, p_objectv->Z, p_objectv->Radius)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_PERSUADE_P:
        thing = p_objectv->Thing;
        if (person_is_persuaded_by_player(thing, local_player_no)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_PERSUADE_MEM_G:
        group = p_objectv->Thing;
        amount = p_objectv->Arg2;
        if (group_has_no_less_members_persuaded_by_player(group, local_player_no, amount)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_PERSUADE_ALL_G:
        group = p_objectv->Thing;
        if (all_group_persuaded(group)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_TIME:
        p_objectv->Radius++;
        if (p_objectv->Radius >= p_objectv->Thing) {
            p_objectv->Radius = 0;
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_GET_ITEM:
        thing = p_objectv->Thing;
        if (item_is_carried_by_player(thing, p_objectv->Arg2, local_player_no)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_USE_ITEM:
    case GAME_OBJ_FUNDS:
        //TODO implement
        break;
    case GAME_OBJ_DESTROY_OBJECT:
        thing = p_objectv->Thing;
        if (thing_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_PKILL_P:
        thing = p_objectv->Thing;
        if (person_is_persuaded_by_player(thing, local_player_no) ||
          person_is_dead(thing) || thing_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_PKILL_MEM_G:
        group = p_objectv->Thing;
        amount = p_objectv->Arg2;
        if (group_has_no_less_members_killed_or_persuaded_by_player(group, local_player_no, amount)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_PKILL_ALL_G:
        group = p_objectv->Thing;
        if (group_has_all_killed_or_persuaded_by_player(group, local_player_no)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_USE_PANET:
    case GAME_OBJ_UNUSED_21:
        //TODO implement
        break;
    case GAME_OBJ_PROTECT_G:
        group = p_objectv->Thing;
        if (group_has_no_less_members_dead(group, 1)) {
            p_objectv->Status = ObvStatu_COMPLETED;
            return 0;
        }
        break;
    case GAME_OBJ_P_PERS_G:
        group = p_objectv->Thing;
        thing = p_objectv->Arg2;
        if (group_has_no_less_members_persuaded_by_person(group, thing, 1)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_ALL_G_USE_V:
        group = p_objectv->Arg2;
        thing = p_objectv->Thing;
        if (vehicle_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_COMPLETED;
            return 0;
        }
        if (group_has_all_survivors_in_vehicle(group, thing)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_MEM_G_USE_V:
        group = p_objectv->Arg2;
        thing = p_objectv->Thing;
        amount = p_objectv->Y;
        if (vehicle_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_COMPLETED;
            return 0;
        }
        if (group_has_no_less_members_in_vehicle(group, thing, amount)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_V_ARRIVES:
        thing = p_objectv->Thing;
        if (vehicle_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_COMPLETED;
            return 0;
        }
        if (thing_arrived_at_objectv(thing, p_objectv)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    case GAME_OBJ_DESTROY_V:
        thing = p_objectv->Thing;
        if (vehicle_is_destroyed(thing)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        // TODO why would we set it to 0 here? And why unconditionally?
        p_objectv->Status = ObvStatu_UNDECIDED;
        break;
    case GAME_OBJ_ITEM_ARRIVES:
        thing = p_objectv->Thing;
        if (item_arrived_at_objectv(thing, p_objectv->Arg2, p_objectv)) {
            p_objectv->Status = ObvStatu_FAILED;
            return 1;
        }
        break;
    }
    return 0;
}

void snprint_objective(char *buf, ulong buflen, struct Objective *p_objectv, ushort objectv)
{
    struct ObjectiveDef *p_odef;
    char *s;
    ubyte nparams;

    p_odef = &objectv_defs[p_objectv->Type];

    s = buf;

    sprintf(s, "P%02d = %s( ", (int)p_objectv->Pri, p_odef->CmdName);
    s += strlen(s);
    nparams = 0;

    if ((p_odef->Flags & ObDF_ReqGroup) != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Group(%hd)", p_objectv->Thing);
        s += strlen(s);
        nparams++;
    } else if ((p_odef->Flags & ObDF_ReqCount) != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Count(%hd)", p_objectv->Thing);
        s += strlen(s);
        nparams++;
    } else if (((p_odef->Flags & ObDF_ReqThing) != 0) ||
      ((p_odef->Flags & ObDF_ReqPerson) != 0) ||
      ((p_odef->Flags & ObDF_ReqVehicle) != 0) ||
      ((p_odef->Flags & ObDF_ReqItem) != 0) ||
      ((p_odef->Flags & ObDF_ReqObject) != 0) ||
      (p_objectv->Thing != 0) || (p_objectv->UniqueID != 0)) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Thing(%hd,%hu)", p_objectv->Thing, p_objectv->UniqueID);
        s += strlen(s);
        nparams++;
    }

    // Unexpected to have unique id without thing
    if (((p_odef->Flags & (ObDF_ReqThing|ObDF_ReqPerson|ObDF_ReqVehicle|
      ObDF_ReqItem|ObDF_ReqObject)) == 0) && (p_objectv->UniqueID != 0)) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "UniqueID(%hu)", p_objectv->UniqueID);
        s += strlen(s);
    }

    if (((p_odef->Flags & ObDF_ReqCoord) != 0) ||
      (p_objectv->X|p_objectv->Y|p_objectv->Z) != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Coord(%hd,%hd,%hd)", p_objectv->X, p_objectv->Y, p_objectv->Z);
        s += strlen(s);
        nparams++;
    }
    if (((p_odef->Flags & ObDF_ReqRadius) != 0) ||
      (p_objectv->Radius != 0)) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Radius(%hd)", p_objectv->Radius);
        s += strlen(s);
        nparams++;
    }
    if ((p_odef->Flags & ObDF_ReqAmount) != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Amount(%d)", (int)p_objectv->Arg2);
        s += strlen(s);
    } else if ((p_odef->Flags & ObDF_ReqSecGrp) != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "SecGroup(%d)", (int)p_objectv->Arg2);
        s += strlen(s);
    } else if ((p_odef->Flags & ObDF_ReqSecTng) != 0) {
        // TODO storing a ThingOffset in 8-bit value is risky; when possible, switch Arg2 with Thing
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "SecThing(%d)", (int)p_objectv->Arg2);
        s += strlen(s);
    } else if (p_objectv->Arg2 != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Arg2(%d)", (int)p_objectv->Arg2);
        s += strlen(s);
    }
    if (p_objectv->StringIndex != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "StringIndex(%hu)", p_objectv->StringIndex);
        s += strlen(s);
    }
    if (p_objectv->Flags != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Flags(%hu)", p_objectv->Flags);
        s += strlen(s);
        nparams++;
    }
    if (p_objectv->ObjText != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "TextId(%d)", (int)p_objectv->ObjText);
        s += strlen(s);
    }
    if ((p_objectv->field_1B[0]|p_objectv->field_1B[1]|p_objectv->field_1B[2]|p_objectv->field_1B[3]) != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "field_1B(%d,%d,%d,%d)", (int)p_objectv->field_1B[0],
          (int)p_objectv->field_1B[1], (int)p_objectv->field_1B[2], (int)p_objectv->field_1B[3]);
        s += strlen(s);
    }
    if (p_objectv->field_1F != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "field_1F(%d)", (int)p_objectv->field_1F);
        s += strlen(s);
    }

    snprintf(s, buflen - (s-buf), " )");
}

void save_objective_chain_conf(TbFileHandle fh, ushort objectv_head, char *buf, ulong buflen)
{
    ushort objectv;

    objectv = 0;
    while (objectv != objectv_head)
    {
        struct Objective *p_objectv;

        { // Go backwards through single-directional chain
            ushort nxobjectv;
            nxobjectv = objectv_head;
            while (game_used_objectives[nxobjectv].Next != objectv)
                nxobjectv = game_used_objectives[nxobjectv].Next;
            objectv = nxobjectv;
        }
        p_objectv = &game_used_objectives[objectv];
        snprint_objective(buf, buflen, p_objectv, objectv);
        strncat(buf, "\n", buflen);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (objectv_head == 0) {
        sprintf(buf, "; no objectives defined\n");
        LbFileWrite(fh, buf, strlen(buf));
    }
}

void snprint_netscan_objctv(char *buf, ulong buflen, struct NetscanObjective *p_nsobv, ushort nsobv)
{
    char *s;
    ubyte nparams;
    int i;

    s = buf;

    sprintf(s, "N%02d = %s( ", (int)nsobv, p_nsobv->AnimNo ? "NETSCAN_OBJ_VIDEO" : "NETSCAN_OBJ_TARGET");
    s += strlen(s);
    nparams = 0;

    if (p_nsobv->CreditCost != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Cost(%d)", (int)(p_nsobv->CreditCost * 100));
        s += strlen(s);
        nparams++;
    }

    for (i = 0; i < NETSCAN_OBJECTIVE_POINTS; i++)
    {
        if ((p_nsobv->X[i]|p_nsobv->Z[i]) != 0) {
            if (nparams) { sprintf(s, ", "); s += strlen(s); }
            sprintf(s, "Coord(%d,0,%d)", (int)(p_nsobv->X[i] << 7), (int)(p_nsobv->Z[i] << 7));
            s += strlen(s);
            nparams++;
        }
    }

    if (p_nsobv->AnimNo != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Anim(%hu)", (ushort)p_nsobv->AnimNo);
        s += strlen(s);
    }

    if (p_nsobv->CreditReward != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Reward(%d)", (int)(p_nsobv->CreditReward * 100));
        s += strlen(s);
        nparams++;
    }

    if (p_nsobv->brobjfld_12 != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Fld12(%hu)", (ushort)p_nsobv->brobjfld_12);
        s += strlen(s);
    }

    if (p_nsobv->brobjfld_13 != 0) {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Fld13(%hu)", (ushort)p_nsobv->brobjfld_13);
        s += strlen(s);
    }

    snprintf(s, buflen - (s-buf), " )");
}

void save_netscan_objectives_conf(TbFileHandle fh, struct NetscanObjective *nsobv_arr,
  ushort nsobv_count, char *buf, ulong buflen)
{
    ushort nsobv;
    ushort nfilled;

    nfilled = 0;
    for (nsobv = 0; nsobv < nsobv_count; nsobv++)
    {
        struct NetscanObjective *p_nsobv;

        p_nsobv = &nsobv_arr[nsobv];
		snprint_netscan_objctv(buf, buflen, p_nsobv, nsobv);
        strncat(buf, "\n", buflen);
        LbFileWrite(fh, buf, strlen(buf));
        nfilled++;
    }
    if (nfilled == 0) {
        sprintf(buf, "; no objectives defined\n");
        LbFileWrite(fh, buf, strlen(buf));
    }
}

int tokenize_script_func(char *olist[], char *obuf, const char *ibuf, long ibuflen)
{
    TbBool in_quotes, token_end, parse_end;
    int li;
    int opos, pos, in_parath;

    li = 0;
    pos = 0;
    in_quotes = false;
    token_end = true;
    parse_end = false;
    in_parath = 0;
    if (pos >= ibuflen) return 0;
    // Skipping spaces after previous parameter
    while ((ibuf[pos] == ' ') || (ibuf[pos] == '\t'))
    {
        pos++;
        if (pos >= ibuflen) return 0;
    }
    if (ibuf[pos] == '\"') {
        in_quotes = true;
        pos++;
        if (pos >= ibuflen) return 0;
    }
    opos = 0;
    while (!parse_end)
    {
        if (token_end) {
            olist[li] = &obuf[opos];
            li++;
            token_end = false;
            if (ibuf[pos] == '\0')
                break;
        }
        for (; pos < ibuflen; pos++)
        {
            if (ibuf[pos] == '\0') {
                in_quotes = false;
                in_parath = 0;
                token_end = true;
                break; // there already is null char write outside the for()
            } else if (in_quotes) {
                if ((ibuf[pos] == '\"') ||
                     (ibuf[pos] == '\r') ||
                     (ibuf[pos] == '\n')) {
                    in_quotes = false;
                    if (in_parath == 0) {
                        token_end = true;
                        continue; // do not copy quote to output
                    }
                }
            } else if (ibuf[pos] == '\"') {
                in_quotes = true;
                if (in_parath == 0)
                    continue; // do not copy quote to output
            } else if (ibuf[pos] == '(') {
                in_parath++;
                if (in_parath == 1) {
                    token_end = true;
                    // do not copy first parenthesis to output
                    continue;
                }
            } else if (ibuf[pos] == ')') {
                in_parath--;
                if (in_parath == 0) {
                    // do not copy last parenthesis to output, and end here
                    parse_end = true;
                    break;
                }
            } else if (in_parath < 2) {
                if (ibuf[pos] == ',') {
                    token_end = true;
                    continue;
                } else if ((ibuf[pos] == ' ') ||
                     (ibuf[pos] == '\t') ||
                     ((uchar)ibuf[pos] < 7)) {
                    continue;
                } else if ((ibuf[pos] == '\r') ||
                     (ibuf[pos] == '\n')) {
                    parse_end = true;
                    break;
                } else if (token_end) {
                    // we found a char to include in a token, but that's a new one
                    break;
                }
            }

            obuf[opos] = ibuf[pos];
            opos++;
            if (pos >= ibuflen) {
                opos++;
                parse_end = true;
                break;
            }
        }
        obuf[opos] = '\0';
        opos++;
    }
    olist[li] = NULL;
    return li;
}

int parse_objective_param(struct Objective *p_objectv, ulong *fields_used, const char *buf, long buflen)
{
    struct ObjectiveDef *p_odef;
    char *toklist[PARAM_TOKEN_MAX];
    char tokbuf[128];
    ulong stored_field;
    int i;

    p_odef = &objectv_defs[p_objectv->Type];
    LbMemorySet(toklist, 0, sizeof(toklist));
    i = tokenize_script_func(toklist, tokbuf, buf, buflen);
    if (i < 2) {
        LOGWARN("Objective \"%s\" parameter consists of less than 2 tokens.",
          p_odef->CmdName);
        return -1;
    }
    if (i >= PARAM_TOKEN_MAX) {
        // If too many params, tokbuf[] have been overwritten partially
        LOGWARN("Objective \"%s\" parameter consists of too many (%d) tokens.",
          p_odef->CmdName, i);
        return -1;
    }

    // Finding parameter number
    stored_field = 0;
    i = 0;
    while (1)
    {
        const struct TbNamedEnum *param;

        if (missions_conf_objective_params[i].name == NULL) {
            i = -1;
            break;
        }
        param = &missions_conf_objective_params[i];
        if (strcasecmp(toklist[0], param->name) == 0) {
            i = param->num;
            break;
        }
        i++;
    }
    switch (i)
    {
    case ObvP_Group:
        if ((p_odef->Flags & ObDF_ReqGroup) == 0) {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected.",
              p_odef->CmdName, toklist[0]);
        }
        stored_field = (1 << ObvP_Thing);
        p_objectv->Thing = atoi(toklist[1]);
        break;
    case ObvP_Count:
        if ((p_odef->Flags & ObDF_ReqCount) == 0) {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected.",
              p_odef->CmdName, toklist[0]);
        }
        stored_field = (1 << ObvP_Thing);
        p_objectv->Thing = atoi(toklist[1]);
        break;
    case ObvP_Thing:
        if (toklist[2] == NULL)  {
            LOGWARN("Objective \"%s\" parameter \"%s\" requires 2 numbers, got less.",
              p_odef->CmdName, toklist[0]);
            return -1;
        }
        if ((p_odef->Flags & (ObDF_ReqThing|ObDF_ReqPerson
          |ObDF_ReqVehicle|ObDF_ReqItem|ObDF_ReqObject)) == 0) {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected.",
              p_odef->CmdName, toklist[0]);
        }
        stored_field = (1 << ObvP_Thing) | (1 << ObvP_UniqueID);
        p_objectv->Thing = atoi(toklist[1]);
        p_objectv->UniqueID = atoi(toklist[2]);
        break;
    case ObvP_UniqueID: // deprecated / testing only
        LOGWARN("Objective \"%s\" parameter \"%s\" has no need of being used directly.",
          p_odef->CmdName, toklist[0]);
        stored_field = (1 << ObvP_UniqueID);
        p_objectv->UniqueID = atoi(toklist[1]);
        break;
    case ObvP_Coord:
        if (toklist[3] == NULL)  {
            LOGWARN("Objective \"%s\" parameter \"%s\" requires 3 numbers, got less.",
              p_odef->CmdName, toklist[0]);
            return -1;
        }
        if ((p_odef->Flags & ObDF_ReqCoord) == 0) {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected.",
              p_odef->CmdName, toklist[0]);
        }
        stored_field = (1 << ObvP_Coord);
        p_objectv->X = atoi(toklist[1]);
        if ((p_odef->Flags & (ObDF_ReqAmountY|ObDF_ReqThingY)) == 0) {
            stored_field |= (1 << ObvP_CoordY);
            p_objectv->Y = atoi(toklist[2]);
        }
        p_objectv->Z = atoi(toklist[3]);
        break;
    case ObvP_Radius:
        if ((p_odef->Flags & ObDF_ReqRadius) == 0) {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected.",
              p_odef->CmdName, toklist[0]);
        }
        stored_field = (1 << ObvP_Radius);
        p_objectv->Radius = atoi(toklist[1]);
        break;
    case ObvP_Amount:
        if ((p_odef->Flags & ObDF_ReqAmount) != 0) {
            stored_field = (1 << ObvP_Arg2);
            p_objectv->Arg2 = atoi(toklist[1]);
        }
        else if ((p_odef->Flags & ObDF_ReqAmountY) != 0) {
            stored_field = (1 << ObvP_CoordY);
            p_objectv->Y = atoi(toklist[1]);
        }
        else {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected; ignored.",
              p_odef->CmdName, toklist[0]);
        }
        break;
    case ObvP_SecGroup:
        if ((p_odef->Flags & ObDF_ReqSecGrp) == 0) {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected.",
              p_odef->CmdName, toklist[0]);
        }
        stored_field = (1 << ObvP_Arg2);
        p_objectv->Arg2 = atoi(toklist[1]);
        break;
    case ObvP_SecThing:
        if ((p_odef->Flags & ObDF_ReqSecTng) != 0) {
            stored_field = (1 << ObvP_Arg2);
            p_objectv->Arg2 = atoi(toklist[1]);
        }
        else if ((p_odef->Flags & ObDF_ReqThingY) != 0) {
            stored_field = (1 << ObvP_CoordY);
            p_objectv->Y = atoi(toklist[1]);
        }
        else {
            LOGWARN("Objective \"%s\" parameter \"%s\" is not expected; ignored.",
              p_odef->CmdName, toklist[0]);
        }
        break;
    case ObvP_Arg2: // deprecated / testing only
        LOGWARN("Objective \"%s\" parameter \"%s\" has no need of being used directly.",
          p_odef->CmdName, toklist[0]);
        stored_field = (1 << ObvP_Arg2);
        p_objectv->Arg2 = atoi(toklist[1]);
        break;
    case ObvP_StringIndex:
        stored_field = (1 << ObvP_StringIndex);
        p_objectv->StringIndex = atoi(toklist[1]);
        break;
    case ObvP_Flags:
        stored_field = (1 << ObvP_Flags);
        p_objectv->Flags = atoi(toklist[1]);
        break;
    case ObvP_TextId:
        stored_field = (1 << ObvP_TextId);
        p_objectv->ObjText = atoi(toklist[1]);
        break;
    default:
        LOGWARN("Objective \"%s\" parameter name \"%s\" not recognized.",
          p_odef->CmdName, toklist[0]);
        return -1;
    }
    if ((*fields_used & stored_field) != 0) {
        LOGWARN("Objective \"%s\" parameter \"%s\" overwites previously set param, used=0x%x.",
          p_odef->CmdName, toklist[0], (*fields_used & stored_field));
    }
    *fields_used |= stored_field;
    return 1;
}

int parse_next_used_objective(const char *buf, long buflen, long pri, long mapno, long levelno)
{
    struct ObjectiveDef *p_odef;
    struct Objective *p_objectv;
    char *toklist[COMMAND_TOKEN_MAX];
    char tokbuf[256];
    ulong fields_used;
    int i, objectv, nret;

    i = tokenize_script_func(toklist, tokbuf, buf, buflen);
    if (i < 1) {
        LOGWARN("Objective consists of less than 1 token.");
        return -1;
    }
    if (i >= COMMAND_TOKEN_MAX) {
        // If too many params, tokbuf[] have been overwritten partially
        LOGWARN("Objective consists of too many (%d) tokens.", i);
        return -1;
    }

    // Finding command number
    i = 0;
    while (objectv_defs[i].CmdName != NULL)
    {
        p_odef = &objectv_defs[i];
        if (strcasecmp(toklist[0], p_odef->CmdName) == 0)
            break;
        i++;
    }
    p_odef = &objectv_defs[i];
    if (p_odef->CmdName == NULL) {
        LOGWARN("Objective name \"%s\" not recognized.", toklist[0]);
        return -1;
    }
    objectv = add_used_objective(mapno, levelno);
    p_objectv = &game_used_objectives[objectv];
    p_objectv->Type = i;
    p_objectv->Pri = pri;

    nret = objectv;
    fields_used = 0;
    for (i = 1; toklist[i] != NULL; i++)
    {
        int ret;
        ret = parse_objective_param(p_objectv, &fields_used, toklist[i], sizeof(tokbuf) - (toklist[i] - tokbuf) );
        if (ret != 1)
            nret = -1;
    }

    return nret;
}

int parse_netscan_obv_param(struct NetscanObjective *p_nsobv, const char *buf, long buflen)
{
    char *toklist[PARAM_TOKEN_MAX];
    char tokbuf[128];
    int i;
    ulong n;

    LbMemorySet(toklist, 0, sizeof(toklist));
    i = tokenize_script_func(toklist, tokbuf, buf, buflen);
    if (i < 1) {
        LOGWARN("Objective parameter consists of less than 1 token.");
        return -1;
    }
    if (i >= PARAM_TOKEN_MAX) {
        // If too many params, tokbuf[] have been overwritten partially
        LOGWARN("Objective parameter consists of too many (%d) tokens.", i);
        return -1;
    }

    // Finding parameter number
    i = 0;
    while (1)
    {
        const struct TbNamedEnum *param;

        if (missions_conf_netscan_objctv_params[i].name == NULL) {
            i = -1;
            break;
        }
        param = &missions_conf_netscan_objctv_params[i];
        if (strcasecmp(toklist[0], param->name) == 0) {
            i = param->num;
            break;
        }
        i++;
    }
    switch (i)
    {
    case NOvP_CreditCost:
        p_nsobv->CreditCost = atoi(toklist[1]) / 100;
        break;
    case NOvP_CreditReward:
        p_nsobv->CreditReward = atoi(toklist[1]) / 100;
        break;
    case NOvP_Coord:
        // Find unused index
        for (n = 0; n < 4; n++) {
            if ((p_nsobv->X[n]|p_nsobv->Z[n]) == 0)
                break;
        }
        if (n > 4)  {
            LOGWARN("Objective parameter \"%s\" used too many times.", toklist[0]);
            return -1;
        }
        if (toklist[3] == NULL)  {
            LOGWARN("Objective parameter \"%s\" requires 3 numbers, got less.", toklist[0]);
            return -1;
        }
        p_nsobv->X[n] = atoi(toklist[1]) >> 7;
        p_nsobv->Z[n] = atoi(toklist[3]) >> 7;
        break;
    case NOvP_AnimNo:
        p_nsobv->AnimNo = atoi(toklist[1]);
        break;
    case NOvP_Fld12:
        p_nsobv->brobjfld_12 = atoi(toklist[1]);
        break;
    case NOvP_Fld13:
        p_nsobv->brobjfld_13 = atoi(toklist[1]);
        break;
    default:
        LOGWARN("Objective parameter name \"%s\" not recognized.", toklist[0]);
        return -1;
    }
    return 1;
}

int parse_next_netscan_objective(const char *buf, long buflen, long nsobv)
{
    struct NetscanObjective *p_nsobv;
    char *toklist[COMMAND_TOKEN_MAX];
    char tokbuf[256];
    int i;
    int nret;

    nret = nsobv;
    i = tokenize_script_func(toklist, tokbuf, buf, buflen);
    if (i < 1) {
        LOGWARN("Objective consists of less than 1 token.");
        return -1;
    }
    if (i >= COMMAND_TOKEN_MAX) {
        // If too many params, tokbuf[] have been overwritten partially
        LOGWARN("Objective consists of too many (%d) tokens.", i);
        return -1;
    }

    // The command name (toklist[0]) is not important currently - ignore

    p_nsobv = &mission_netscan_objectives[nsobv];
    LbMemorySet(p_nsobv, '\0', sizeof(struct NetscanObjective));

    for (i = 1; toklist[i] != NULL; i++)
    {
        int ret;
        ret = parse_netscan_obv_param(p_nsobv, toklist[i], sizeof(tokbuf) - (toklist[i] - tokbuf) );
        if (ret != 1)
            nret = -1;
    }

    return nret;
}

int load_netscan_objectives_bin(struct NetscanObjective *nsobv_arr, ubyte mapno, ubyte level)
{
    char locstr[52];
    TbFileHandle fp;
    ubyte nsobv_count;

    sprintf(locstr, "obj%02d%02d.dat", (int)mapno, (int)level);
    fp = open_file_from_wad(locstr, "qdata/posdefs");
    if (fp == INVALID_FILE) {
        return 0;
    }
    LbFileRead(fp, &nsobv_count, 1);
    LbFileRead(fp, nsobv_arr, 20 * nsobv_count);
    LbFileClose(fp);
    return nsobv_count;
}

void load_netscan_objectives(ubyte mapno, ubyte level)
{
    struct Mission *p_missi;
    ushort missi;
    int remain;

    missi = find_mission_with_map_and_level(mapno, level);
    if (missi == 0) {
        LOGERR("Unable to map map and level to mission.");
    }
    p_missi = &mission_list[missi];
    LbMemoryCopy(netscan_objectives, &mission_netscan_objectives[p_missi->NetscanObvIndex],
      sizeof(struct NetscanObjective) * p_missi->NetscanObvCount);
    netscan_objectives_count = p_missi->NetscanObvCount;
    remain = NETSCAN_OBJECTIVES_MAX_COUNT - netscan_objectives_count;
    if (remain > 0)
        LbMemorySet(&netscan_objectives[netscan_objectives_count], '\0',
          sizeof(struct NetscanObjective) * remain);
}

int read_objectives_text(void *data)
{
    char *p;
    int totlen;
    int i, n;
    char c;

    totlen = load_file_alltext("textdata/obj.txt", data);
    if (totlen <= 0)
      return 0;
    p = (char *)data;
    n = 0;
    objective_text[n++] = NULL;
    objective_text[n++] = p;
    for (i = 0; i < totlen; i++, p++)
    {
        c = *p;
        if (c == '\r' || c == '\n')
        {
            *p = '\0';
            i++; p++;
            c = *p;
            if (c == '\r' || c == '\n') {
                i++; p++;
            }
            objective_text[n++] = p;
        }
    }

    assert(n <= OBJECTIVE_TEXT_MAX);

    while (n < OBJECTIVE_TEXT_MAX)
    {
        objective_text[n++] = NULL;
    }
    return (totlen + 5) & ~0x03;
}

TbResult load_objectives_text(void)
{
    int len;
    len = read_objectives_text(engine_mem_alloc_ptr);
    if (len < 1) {
        LOGERR("Objective text read failed.");
        return Lb_FAIL;
    }
    else if (len > 8192) {
        LOGERR("Objective text exceeded assigned max size.");
        return Lb_FAIL;
    }
    return Lb_SUCCESS;
}

/******************************************************************************/
