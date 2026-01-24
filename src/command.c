/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file command.c
 *     Routines for person command handling.
 * @par Purpose:
 *     Implement functions for person obeying commands.
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
#include "command.h"

#include <stdio.h>
#include <string.h>
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfutility.h"

#include "building.h"
#include "thing.h"
#include "thing_search.h"
#include "pepgroup.h"
#include "swlog.h"
/******************************************************************************/

enum CommandDefFlags {
    CmDF_None = 0x0000,
    /* === Parameters used when OtherThing is set directly === */
    /** Group index is required in OtherThing. */
    CmDF_ReqGroup = 0x0001,
    /** Person index is required in OtherThing. */
    CmDF_ReqPersonThing = 0x0002,
    /** Person, Vehicle or Item Thing index is required in OtherThing. */
    CmDF_ReqPVIThing = 0x0004,
    /** Any Thing index is required in OtherThing. */
    CmDF_ReqOtherThing = 0x0008,
    /** A non-thing index is required in OtherThing. */
    CmDF_ReqOtherIndex = 0x0010,
    /* === Parameters used when OtherThing is updated by map search. === */
    /** Thing location coordinates are required to find generic OtherThing. */
    CmDF_ReqThingCoord = 0x0020,
    /** Station location Coordinates are required to find target train OtherThing. */
    CmDF_ReqStationCoord = 0x0040,
    /** Dome location Coordinates are required to find target dome building OtherThing. */
    CmDF_ReqDomeCoord = 0x0080,
    /** Building location Coordinates are required to find target building OtherThing. */
    CmDF_ReqBuildingCoord = 0x0100,
    /** Vehicle location Coordinates are required to find the target Vehicle OtherThing. */
    CmDF_ReqVehicleCoord = 0x0200,
    /* === Parameters beyond OtherThing. === */
    /** Map coordinates are required, to be used directly during command execution. */
    CmDF_ReqCoord = 0x0400,
    /** Second set of map coordinates are required, replacing Arg1,Arg2,Time. */
    CmDF_ReqCoord2 = 0x0800,
    /** Range is required in Arg1. */
    CmDF_ReqRange1 = 0x4000,
    /** Any value is required in Arg1. */
    CmDF_ReqArg1 = 0x8000,
    CmDF_ReqSubType = 0x00010000,
    /** Any Thing index is required in MyThing. */
    CmDF_ReqMyThing = 0x00020000,
    /** Amount of targets is required in Arg2 field. If used with CmDF_ReqCoord2, then
     * there are only planar XZ coordinates, and the middle field is used for the Count.
     */
    CmDF_ReqCountT = 0x00100000,
    /** Game turns amount required in Time. */
    CmDF_ReqTime = 0x00200000,
    CmDF_ReqParent = 0x00400000,
};

struct CommandDef command_defs[] = {
    /* Means end of command chain. */
    {"TNG_CMD_NONE",				NULL,				CmDF_None },
    {"TNG_CMD_STAY",				NULL,				CmDF_None },
    {"TNG_CMD_GO_TO_POINT",			NULL,				CmDF_ReqCoord|CmDF_ReqRange1 },
    {"TNG_CMD_GO_TO_PERSON",		NULL,				CmDF_ReqPersonThing|CmDF_ReqRange1 },
    {"TNG_CMD_KILL_PERSON",			NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_KILL_MEM_GROUP",		NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_KILL_ALL_GROUP",		NULL,				CmDF_ReqGroup },
    {"TNG_CMD_PERSUADE_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_PERSUADE_MEM_GROUP",	NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_PERSUADE_ALL_GROUP",	NULL,				CmDF_ReqGroup },
    {"TNG_CMD_BLOCK_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_SCARE_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_FOLLOW_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_SUPPORT_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_PROTECT_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_HIDE",				NULL,				CmDF_None },
    {"TNG_CMD_GET_ITEM",			NULL,				CmDF_ReqPVIThing },
    {"TNG_CMD_USE_WEAPON",			NULL,				CmDF_ReqOtherIndex|CmDF_ReqCoord }, // OtherIndex = weapon type
    {"TNG_CMD_DROP_SPEC_ITEM",		NULL,				CmDF_ReqPVIThing },
    {"TNG_CMD_AVOID_PERSON",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_WAND_AVOID_GROUP",	NULL,				CmDF_ReqGroup },
    {"TNG_CMD_DESTROY_BUILDING",	NULL,				CmDF_ReqBuildingCoord },
    {"TNG_CMD_UNKN16",				NULL,				CmDF_None },
    {"TNG_CMD_USE_VEHICLE",			NULL,				CmDF_ReqVehicleCoord },
    {"TNG_CMD_EXIT_VEHICLE",		NULL,				CmDF_None },
    {"TNG_CMD_CATCH_TRAIN",			NULL,				CmDF_ReqStationCoord },
    {"TNG_CMD_OPEN_DOME",			NULL,				CmDF_ReqDomeCoord },
    {"TNG_CMD_CLOSE_DOME",			NULL,				CmDF_ReqDomeCoord },
    {"TNG_CMD_DROP_WEAPON",			NULL,				CmDF_ReqOtherIndex }, // OtherIndex = weapon type
    {"TNG_CMD_CATCH_FERRY",			NULL,				CmDF_ReqCoord|CmDF_ReqRange1 },
    {"TNG_CMD_EXIT_FERRY",			NULL,				CmDF_ReqOtherThing },
    {"TNG_CMD_PING_EXIST",			NULL,				CmDF_None },
    {"TNG_CMD_GOTOPOINT_FACE",		NULL,				CmDF_ReqOtherIndex|CmDF_ReqCoord|CmDF_ReqRange1 }, // OtherIndex = face
    {"TNG_CMD_SELF_DESTRUCT",		NULL,				CmDF_None },
    {"TNG_CMD_PROTECT_MEM_G",		NULL,				CmDF_ReqGroup },
    {"TNG_CMD_RUN_TO_POINT",		NULL,				CmDF_ReqCoord|CmDF_ReqRange1 },
    {"TNG_CMD_KILL_EVERYONE",		NULL,				CmDF_ReqCoord|CmDF_ReqCoord2 },
    {"TNG_CMD_GUARD_OFF",			NULL,				CmDF_None },
    {"TNG_CMD_EXECUTE_COMS",		NULL,				CmDF_None },
    {"TNG_CMD_UNKN27",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN28",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN29",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN2A",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN2B",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN2C",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN2D",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN2E",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN2F",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN30",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN31",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN32",				NULL,				CmDF_None },
    /* Stand waiting for event commands */
    {"TNG_CMD_WAIT_P_V_DEAD",		NULL,				CmDF_ReqPVIThing },
    {"TNG_CMD_WAIT_MEM_G_DEAD",		NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_WAIT_ALL_G_DEAD",		NULL,				CmDF_ReqGroup },
    {"TNG_CMD_WAIT_P_V_I_NEAR",		NULL,				CmDF_ReqPVIThing|CmDF_ReqRange1 },
    {"TNG_CMD_WAIT_MEM_G_NEAR",		NULL,				CmDF_ReqGroup|CmDF_ReqCountT|CmDF_ReqRange1 },
    {"TNG_CMD_WAIT_ALL_G_NEAR",		NULL,				CmDF_ReqGroup|CmDF_ReqRange1 },
    {"TNG_CMD_WAIT_P_V_I_ARRIVE",	NULL,				CmDF_ReqPVIThing|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WAIT_MEM_G_ARRIVE",	NULL,				CmDF_ReqGroup|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1|CmDF_ReqCountT }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WAIT_ALL_G_ARRIVE",	NULL,				CmDF_ReqGroup|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WAIT_P_PERSUADE",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_WAIT_MEM_G_PERSUADE",	NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_WAIT_ALL_G_PERSUADE",	NULL,				CmDF_ReqGroup },
    {"TNG_CMD_WAIT_MISSION_SUCC",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_WAIT_MISSION_FAIL",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_WAIT_MISSION_START",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_WAIT_OBJT_DESTROY",	NULL,				CmDF_ReqBuildingCoord },
    {"TNG_CMD_WAIT_TIME",			NULL,				CmDF_ReqTime|CmDF_ReqArg1 }, // Arg1 counts, Time is end val
    {"TNG_CMD_UNKN44",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN45",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN46",				NULL,				CmDF_None },
    /* Wander waiting for event commands */
    {"TNG_CMD_WAND_P_V_DEAD",		NULL,				CmDF_ReqPVIThing },
    {"TNG_CMD_WAND_MEM_G_DEAD",		NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_WAND_ALL_G_DEAD",		NULL,				CmDF_ReqGroup },
    {"TNG_CMD_WAND_P_V_I_NEAR",		NULL,				CmDF_ReqPVIThing|CmDF_ReqRange1 },
    {"TNG_CMD_WAND_MEM_G_NEAR",		NULL,				CmDF_ReqGroup|CmDF_ReqCountT|CmDF_ReqRange1 },
    {"TNG_CMD_WAND_ALL_G_NEAR",		NULL,				CmDF_ReqGroup|CmDF_ReqRange1 },
    {"TNG_CMD_WAND_P_V_I_ARRIVE",	NULL,				CmDF_ReqPVIThing|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WAND_MEM_G_ARRIVE",	NULL,				CmDF_ReqGroup|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1|CmDF_ReqCountT }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WAND_ALL_G_ARRIVE",	NULL,				CmDF_ReqGroup|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WAND_P_PERSUADE",		NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_WAND_MEM_G_PERSUADE",	NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_WAND_ALL_G_PERSUADE",	NULL,				CmDF_ReqGroup },
    {"TNG_CMD_WAND_MISSION_SUCC",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_WAND_MISSION_FAIL",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_WAND_MISSION_START",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_WAND_OBJT_DESTROY",	NULL,				CmDF_ReqBuildingCoord },
    {"TNG_CMD_WAND_TIME",			NULL,				CmDF_ReqTime|CmDF_ReqArg1 }, // Arg1 counts, Time is end val
    {"TNG_CMD_UNKN58",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN59",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN5A",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN5B",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN5C",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN5D",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN5E",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN5F",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN60",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN61",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN62",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN63",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN64",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN65",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN66",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN67",				NULL,				CmDF_None },
    {"TNG_CMD_ADD_STATIC",			NULL,				CmDF_ReqCoord },
    {"TNG_CMD_WAIT_TIME2",			NULL,				CmDF_ReqTime },
    {"TNG_CMD_UNKN6A",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN6B",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN6C",				NULL,				CmDF_None },
    {"TNG_CMD_UNKN6D",				NULL,				CmDF_None },
    {"TNG_CMD_LOOP_COM",			NULL,				CmDF_ReqOtherIndex }, // OtherIndex = command index
    /* Repeat until event commands */
    {"TNG_CMD_UNTIL_P_V_DEAD",		NULL,				CmDF_ReqPVIThing },
    {"TNG_CMD_UNTIL_MEM_G_DEAD",	NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_UNTIL_ALL_G_DEAD",	NULL,				CmDF_ReqGroup },
    {"TNG_CMD_UNTIL_P_V_I_NEAR",	NULL,				CmDF_ReqPVIThing|CmDF_ReqRange1 },
    {"TNG_CMD_UNTIL_MEM_G_NEAR",	NULL,				CmDF_ReqGroup|CmDF_ReqCountT|CmDF_ReqRange1 },
    {"TNG_CMD_UNTIL_ALL_G_NEAR",	NULL,				CmDF_ReqGroup|CmDF_ReqRange1 },
    {"TNG_CMD_UNTIL_P_V_I_ARRIVE",	NULL,				CmDF_ReqPVIThing|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_UNTIL_MEM_G_ARRIVE",	NULL,				CmDF_ReqGroup|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1|CmDF_ReqCountT }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_UNTIL_ALL_G_ARRIVE",	NULL,				CmDF_ReqGroup|CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_UNTIL_P_PERSUADE",	NULL,				CmDF_ReqPersonThing },
    {"TNG_CMD_UNTIL_MEM_G_PERSUADE",NULL,				CmDF_ReqGroup|CmDF_ReqCountT },
    {"TNG_CMD_UNTIL_ALL_G_PERSUADE",NULL,				CmDF_ReqGroup },
    {"TNG_CMD_UNTIL_MISSION_SUCC",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_UNTIL_MISSION_FAIL",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_UNTIL_MISSION_START",	NULL,				CmDF_ReqOtherIndex }, // OtherIndex = player?
    {"TNG_CMD_UNTIL_OBJT_DESTROY",	NULL,				CmDF_ReqBuildingCoord },
    {"TNG_CMD_UNTIL_TIME",			NULL,				CmDF_ReqTime|CmDF_ReqArg1 },
    /* Commands added later in development */
    {"TNG_CMD_WAIT_OBJV",			NULL,				CmDF_ReqOtherIndex }, // OtherIndex = lvl objective
    {"TNG_CMD_WAND_OBJV",			NULL,				CmDF_ReqOtherIndex }, // OtherIndex = lvl objective
    {"TNG_CMD_UNTIL_OBJV",			NULL,				CmDF_ReqOtherIndex }, // OtherIndex = lvl objective
    {"TNG_CMD_WITHIN_AREA",			NULL,				CmDF_ReqCoord|CmDF_ReqCoord2|CmDF_ReqRange1 }, // Coord2 or Range1 depending on Flags
    {"TNG_CMD_WITHIN_OFF",			NULL,				CmDF_None },
    {"TNG_CMD_LOCK_BUILDN",			NULL,				CmDF_ReqBuildingCoord },
    {"TNG_CMD_UNLOCK_BUILDN",		NULL,				CmDF_ReqBuildingCoord },
    {"TNG_CMD_SELECT_WEAPON",		NULL,				CmDF_ReqOtherIndex }, // OtherIndex = weapon type
    {"TNG_CMD_HARD_AS_AGENT",		NULL,				CmDF_None },
    {"TNG_CMD_UNTIL_G_NOT_SEEN",	NULL,				CmDF_ReqGroup },
    {"TNG_CMD_START_DANGER_MUSIC",	NULL,				CmDF_None },
    {"TNG_CMD_PING_P_V",			NULL,				CmDF_ReqPVIThing },
    {"TNG_CMD_CAMERA_TRACK",		NULL,				CmDF_ReqOtherThing },
    {"TNG_CMD_UNTRUCE_GROUP",		NULL,				CmDF_ReqGroup },
    {"TNG_CMD_PLAY_SAMPLE",			NULL,				CmDF_ReqOtherIndex }, // OtherIndex = sample
    {"TNG_CMD_IGNORE_ENEMIES",		NULL,				CmDF_None },
    {"TNG_CMD_FIT_AS_AGENT",		NULL,				CmDF_None },
    {"TNG_CMD_CAMERA_ROTATE",		NULL,				CmDF_ReqOtherIndex|CmDF_ReqArg1 }, //  OtherIndex = degrees delta, Arg1 = speed
    {NULL,							NULL,				CmDF_None },
};

ushort next_command = 1;

const char *command_codename(ushort cmd)
{
    struct Command *p_cmd;
    struct CommandDef *p_cdef;

    if (cmd >= next_command) {
        return "TNG_CMD_INVALID";
    }
    p_cmd = &game_commands[cmd];

    if (p_cmd->Type > PCmd_TYPES_COUNT) {
        return "TNG_CMD_BADTYPE";
    }
    p_cdef = &command_defs[p_cmd->Type];

    return p_cdef->CmdName;
}

void snprint_command(char *buf, ulong buflen, ushort cmd)
{
    struct Command *p_cmd;
    struct CommandDef *p_cdef;

    char *s;
    ubyte nparams;

    p_cmd = &game_commands[cmd];

    s = buf;

    if (p_cmd->Type > PCmd_TYPES_COUNT) {
        snprintf(s, buflen - (s - buf), "%s%hu()", "INVALID", cmd);
        return;
    }

    p_cdef = &command_defs[p_cmd->Type];

    snprintf(s, buflen - (s - buf), "%s( ", p_cdef->CmdName);
    s += strlen(s);
    nparams = 0;

    if (((p_cdef->Flags & CmDF_ReqSubType) != 0) ||
      (p_cmd->SubType != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "SubType(%d)", (int)p_cmd->SubType);
        s += strlen(s);
        nparams++;
    }

    if (((p_cdef->Flags & CmDF_ReqCoord) != 0) ||
      (p_cmd->X|p_cmd->Y|p_cmd->Z) != 0) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Coord(%hd,%hd,%hd)", p_cmd->X, p_cmd->Y, p_cmd->Z);
        s += strlen(s);
        nparams++;
    }

    if ((p_cdef->Flags & (CmDF_ReqPersonThing|CmDF_ReqPVIThing|CmDF_ReqOtherThing)) != 0) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Thing(%hd)", p_cmd->OtherThing);
        s += strlen(s);
        nparams++;
    }
    else if ((p_cdef->Flags & CmDF_ReqGroup) != 0) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Group(%hd)", p_cmd->OtherThing);
        s += strlen(s);
        nparams++;
    }
    else if (((p_cdef->Flags & CmDF_ReqOtherIndex) != 0) || (p_cmd->OtherThing != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Index(%hd)", p_cmd->OtherThing);
        s += strlen(s);
        nparams++;
    }

    // some commands have both CmDF_ReqCoord2 and CmDF_ReqRange1; which one is used depends on AreaIsRect flag
    if ((((p_cdef->Flags & CmDF_ReqCoord2) != 0) && ((p_cdef->Flags & CmDF_ReqRange1) == 0)) ||
     (((p_cdef->Flags & CmDF_ReqCoord2) != 0) && ((p_cdef->Flags & CmDF_ReqRange1) != 0) &&
      ((p_cmd->Flags & PCmdF_AreaIsRect) != 0))) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        if ((p_cdef->Flags & CmDF_ReqCountT) != 0)
            snprintf(s, buflen - (s - buf), "Coord2(%hd,0,%hd)", p_cmd->Arg1, p_cmd->Time);
        else
            snprintf(s, buflen - (s - buf), "Coord2(%hd,%hd,%hd)", p_cmd->Arg1, p_cmd->Arg2, p_cmd->Time);
        s += strlen(s);
        nparams++;
    }
    else if ((p_cdef->Flags & CmDF_ReqRange1) != 0) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Range(%hd)", p_cmd->Arg1);
        s += strlen(s);
        nparams++;
    }
    else if (((p_cdef->Flags & CmDF_ReqArg1) != 0) || (p_cmd->Arg1 != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Arg1(%hd)", p_cmd->Arg1);
        s += strlen(s);
        nparams++;
    }

    if (((p_cdef->Flags & CmDF_ReqCoord2) != 0) && ((p_cdef->Flags & CmDF_ReqCountT) == 0)) {
        // Skip printing Arg2 since we've alredy used it for Coord2
    }
    else if ((p_cdef->Flags & CmDF_ReqCountT) != 0) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "CountT(%hd)", p_cmd->Arg2);
        s += strlen(s);
        nparams++;
    }
    else if (p_cmd->Arg2 != 0) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Arg2(%hd)", p_cmd->Arg2);
        s += strlen(s);
        nparams++;
    }

    if (((p_cdef->Flags & CmDF_ReqMyThing) != 0) ||
      (p_cmd->MyThing != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "MyThing(%hd)", p_cmd->MyThing);
        s += strlen(s);
        nparams++;
    }

    if (((p_cdef->Flags & CmDF_ReqParent) != 0) ||
      (p_cmd->Parent != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Parent(%hd)", p_cmd->Parent);
        s += strlen(s);
        nparams++;
    }

    if ((p_cdef->Flags & CmDF_ReqCoord2) != 0) {
        // Skip printing Time since we've alredy used it for Coord2
    }
    else if (((p_cdef->Flags & CmDF_ReqTime) != 0) || (p_cmd->Time != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Time(%hd)", p_cmd->Time);
        s += strlen(s);
        nparams++;
    }

    {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Flags(0x%02lX)", p_cmd->Flags);
        s += strlen(s);
        nparams++;
    }

    if ((p_cmd->field_1C != 0) || (p_cmd->field_1E != 0)) {
        if (nparams) { snprintf(s, buflen - (s - buf), ", "); s += strlen(s); }
        snprintf(s, buflen - (s - buf), "Padding(%hd,%hd)", p_cmd->field_1C, p_cmd->field_1E);
        s += strlen(s);
        nparams++;
    }


    snprintf(s, buflen - (s-buf), " )");
}

TbBool is_command_any_until(struct Command *p_cmd)
{

    if (p_cmd->Type < PCmd_UNTIL_P_PERSUADE)
    {
        if (p_cmd->Type < PCmd_UNTIL_P_V_DEAD)
            return false;
        return true;
    }
    else if (p_cmd->Type > PCmd_UNTIL_OBJT_DESTROY)
    {
        if (p_cmd->Type < PCmd_UNTIL_OBJV)
        {
          if (p_cmd->Type != PCmd_UNTIL_TIME)
            return false;
        }
        else if (p_cmd->Type > PCmd_UNTIL_OBJV && p_cmd->Type != PCmd_UNTIL_G_NOT_SEEN)
        {
            return false;
        }
        return true;
    }
    return true;
}

ushort get_new_command(void)
{
    struct Command *p_cmd;
    ushort cmd;

    for (cmd = 1; cmd < next_command; cmd++)
    {
        p_cmd = &game_commands[cmd];
        if (game_commands[cmd].Type == PCmd_NONE)
        {
            p_cmd->Type = PCmd_STAY;
            p_cmd->Next = 0;
            return cmd;
        }
    }
    p_cmd = &game_commands[cmd];
    next_command++;
    p_cmd->Type = PCmd_STAY;
    p_cmd->Next = 0;
    return cmd;
}

short find_station_platform(short x, short z)
{
    struct Thing *p_station;
    ThingIdx station;

    station = search_for_station(x, z);
    if (station <= 0) {
        LOGWARN("No train station found at Coord(%hd,%hd)", x, z);
        return 0;
    }
    p_station = &things[station];
    return search_object_for_qface(p_station->U.UObject.Object, 4, 2, 0);
}

#define MAP_BORDER_MARGIN 32

/** Fixes parameters within a command.
 *
 * @param cmd Index of the Command instance to fix.
 * @param deep Perform deep/aggresive fixing.
 * @return Gives 2 if a parameter was updated, 1 if no update was neccessary, 0 if update failed.
 */
ubyte fix_thing_command_indexes(ushort cmd, TbBool deep)
{
    struct Command *p_cmd;
    struct CommandDef *p_cdef;
    ThingIdx thing;
    ubyte ret;

    p_cmd = &game_commands[cmd];
    p_cdef = &command_defs[p_cmd->Type];

    ret = 1;
    if ((p_cdef->Flags & CmDF_ReqGroup) != 0)
    {
        if ((p_cmd->OtherThing < 0) || (p_cmd->OtherThing >= PEOPLE_GROUPS_COUNT)) {
            LOGERR("Cmd%hu = %s Group(%hd) out of range",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqStationCoord) != 0)
    {
        short face;
        face = find_station_platform(p_cmd->X, p_cmd->Z);
        if (face != 0) {
            p_cmd->OtherThing = face;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s matching platform face on target station not found based on Coord(%hd,%hd)",
              cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqDomeCoord) != 0)
    {
        thing = find_nearest_object2(p_cmd->X, p_cmd->Z, SubTT_BLD_DOME);
        if (thing == 0) {
            LOGSYNC("Cmd%hu = %s target dome not found based on Coord(%hd,%hd); using object index",
              cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            thing = search_things_for_index(p_cmd->OtherThing);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_BUILDING)
                    thing = 0;
            }
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target dome %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqBuildingCoord) != 0)
    {
        thing = find_nearest_object2(p_cmd->X, p_cmd->Z, 0);
        if (thing == 0) {
            LOGSYNC("Cmd%hu = %s target building not found based on Coord(%hd,%hd); using object index",
              cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            thing = search_things_for_index(p_cmd->OtherThing);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_BUILDING)
                    thing = 0;
            }
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target building %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqVehicleCoord) != 0)
    {
        thing = 0;
        if (thing == 0 && deep) {
            // vehicle coords in original maps are often outdated, like vehicles were moved after adding
            // the command; to avoid linking wrong vehicles, use this only for deep fixing
            thing = search_for_vehicle(p_cmd->X, p_cmd->Z);
            if (thing == 0) {
                LOGSYNC("Cmd%hu = %s target vehicle not found based on Coord(%hd,%hd); using vehicle index",
                  cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            }
        }
        if (thing == 0) {
            thing = search_things_for_index(p_cmd->OtherThing);
            if (thing < 0) {
                struct SimpleThing *p_sthing = &sthings[thing];
                if (p_sthing->Type == SmTT_STATIC) {
                    LOGWARN("Cmd%hu = %s target vehicle %hd is a static; jumping to parent",
                      cmd, p_cdef->CmdName, thing);
                    thing = p_sthing->Parent;
                }
            }
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_VEHICLE)
                    thing = 0;
            }
        }
        if (thing == 0 && deep) {
            // Mechas consist of two things, and vehicle index often point to a wrong one; so try previous
            // vehicke index if the exact one fails. Only while deep fixing, as it greatly affect gameplay
            // when mechas rather than being empty have enemy drivers.
            thing = search_things_for_index(p_cmd->OtherThing - 1);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_VEHICLE)
                    thing = 0;
            }
            if (thing != 0) {
                LOGWARN("Cmd%hu = %s target vehicle found using vehicle index previous to %hd",
                  cmd, p_cdef->CmdName, p_cmd->OtherThing);
            }
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target vehicle %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqThingCoord) != 0)
    {
        // using this is not expected; use more specific search cases, which allow for type checking
        thing = 0; // no function to search for any thing type based on coords
        if (thing == 0) {
            LOGNO("Cmd%hu = %s target thing not found based on Coord(%hd,%hd); using thing index",
              cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            thing = search_things_for_index(p_cmd->OtherThing);
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target thing %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqPersonThing) != 0)
    {
        thing = 0;
        if (thing == 0) {
            thing = search_things_for_index(p_cmd->OtherThing);
            if (thing <= 0) {
                thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if (p_thing->Type != TT_PERSON)
                    thing = 0;
            }
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target person %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqPVIThing) != 0)
    {
        thing = 0;
        if (thing == 0) {
            thing = search_things_for_index(p_cmd->OtherThing);
            // One mistake which often happens on the map, is that we have static
            // lights assigned as items. This should be really fixed in the level,
            // but there is also no reson not to try fix it here.
            if (thing < 0) {
                struct SimpleThing *p_sthing = &sthings[thing];
                if (p_sthing->Type == SmTT_STATIC) {
                    LOGWARN("Cmd%hu = %s target p/v/i %hd is a static; jumping to parent",
                      cmd, p_cdef->CmdName, thing);
                    thing = p_sthing->Parent;
                }
            }
            if (thing <= 0) {
                struct SimpleThing *p_sthing = &sthings[thing];
                if ((p_sthing->Type != SmTT_CARRIED_ITEM) && (p_sthing->Type != SmTT_DROPPED_ITEM))
                    thing = 0;
            } else {
                struct Thing *p_thing = &things[thing];
                if ((p_thing->Type != TT_PERSON) && (p_thing->Type != TT_VEHICLE))
                    thing = 0;
            }
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target person/vehicle/item %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }
    else if ((p_cdef->Flags & CmDF_ReqOtherThing) != 0)
    {
        // If this flag is set directly, the thing can't be searched for using coordinates
        // This is used when coordinates have different function for the command.
        // This flag also indicated we do not really know what type of thing is expected;
        // when we understand all thing types used in commands, this case is expected to
        // become unused
        thing = 0;
        if (thing == 0) {
            LOGNO("Cmd%hu = %s target thing can only be found using thing index",
              cmd, p_cdef->CmdName);
            thing = search_things_for_index(p_cmd->OtherThing);
        }
        if (thing != 0) {
            p_cmd->OtherThing = thing;
            if (ret) ret = 2;
        } else {
            LOGERR("Cmd%hu = %s target thing %hd not found",
              cmd, p_cdef->CmdName, p_cmd->OtherThing);
            p_cmd->OtherThing = 0;
            ret = 0;
        }
    }

    if ((p_cdef->Flags & CmDF_ReqCoord) != 0)
    {
        if ((p_cmd->X < MAP_BORDER_MARGIN) || (p_cmd->X > 32767 - MAP_BORDER_MARGIN)) {
            LOGERR("Cmd%hu = %s target coord X of (%hd,%hd) out of range",
              cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            if ((p_cmd->X < MAP_BORDER_MARGIN) && (p_cmd->X >= -16384)) {
                p_cmd->X = MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
            if ((p_cmd->X > 32767 - MAP_BORDER_MARGIN) || (p_cmd->X < -16384)) {
                p_cmd->X = 32767 - MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
        }
        if ((p_cmd->Z < MAP_BORDER_MARGIN) || (p_cmd->Z > 32767 - MAP_BORDER_MARGIN)) {
            LOGERR("Cmd%hu = %s target coord Z of (%hd,%hd) out of range",
              cmd, p_cdef->CmdName, p_cmd->X, p_cmd->Z);
            if ((p_cmd->Z < MAP_BORDER_MARGIN) && (p_cmd->Z >= -16384)) {
                p_cmd->Z = MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
            if ((p_cmd->Z > 32767 - MAP_BORDER_MARGIN) || (p_cmd->Z < -16384)) {
                p_cmd->Z = 32767 - MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
        }
    }

    if (((p_cdef->Flags & CmDF_ReqCoord2) != 0) && (((p_cdef->Flags & CmDF_ReqRange1) == 0) ||
        (((p_cdef->Flags & CmDF_ReqRange1) != 0) && ((p_cmd->Flags & PCmdF_AreaIsRect) != 0))))
    {
        if ((p_cmd->Arg1 < MAP_BORDER_MARGIN) || (p_cmd->Arg1 > 32767 - MAP_BORDER_MARGIN)) {
            LOGERR("Cmd%hu = %s target coord X/Arg1 of (%d,%d) out of range",
              cmd, p_cdef->CmdName, (int)p_cmd->Arg1, (int)p_cmd->Time);
            if ((p_cmd->Arg1 < MAP_BORDER_MARGIN) && (p_cmd->Arg1 >= -16384)) {
                p_cmd->Arg1 = MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
            if ((p_cmd->Arg1 > 32767 - MAP_BORDER_MARGIN) || (p_cmd->Arg1 < -16384)) {
                p_cmd->Arg1 = 32767 - MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
        }
        if ((p_cmd->Time < MAP_BORDER_MARGIN) || (p_cmd->Time > 32767 - MAP_BORDER_MARGIN)) {
            LOGERR("Cmd%hu = %s target coord Z/Time of (%d,%d) out of range",
              cmd, p_cdef->CmdName, (int)p_cmd->Arg1, (int)p_cmd->Time);
            if ((p_cmd->Time < MAP_BORDER_MARGIN) && (p_cmd->Time >= -16384)) {
                p_cmd->Time = MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
            if ((p_cmd->Time > 32767 - MAP_BORDER_MARGIN) || (p_cmd->Time < -16384)) {
                p_cmd->Time = 32767 - MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
        }
    } else
    if (((p_cdef->Flags & CmDF_ReqRange1) != 0) && (((p_cdef->Flags & CmDF_ReqCoord2) == 0) ||
        (((p_cdef->Flags & CmDF_ReqCoord2) != 0) && ((p_cmd->Flags & PCmdF_AreaIsRect) == 0))))
    {
        if ((p_cmd->Arg1 < 1) || (p_cmd->Arg1 > 32767 - MAP_BORDER_MARGIN)) {
            LOGERR("Cmd%hu = %s target Range/Arg1 of %d out of range",
              cmd, p_cdef->CmdName, (int)p_cmd->Arg1);
            if ((p_cmd->Arg1 < 1) && (p_cmd->Arg1 >= -16384)) {
                p_cmd->Arg1 = 1;
                if (ret) ret = 2;
            }
            if ((p_cmd->Arg1 > 32767 - MAP_BORDER_MARGIN) || (p_cmd->Arg1 < -16384)) {
                p_cmd->Arg1 = 32767 - MAP_BORDER_MARGIN;
                if (ret) ret = 2;
            }
        }
    }

    return ret;
}

void fix_thing_commands_indexes(TbBool deep)
{
    ushort cmd;

    for (cmd = 1; cmd < next_command; cmd++)
    {
        fix_thing_command_indexes(cmd, deep);
    }
}

void check_and_fix_commands(void)
{
    ushort cmd;

    for (cmd = 1; cmd < next_command ; cmd++)
    {
        struct Command *p_cmd;

        p_cmd = &game_commands[cmd];

        if (p_cmd->Next > next_command)
            p_cmd->Next = 0;

        if ((p_cmd->Flags & PCmdF_RunUntil) != 0)
        {
            struct Command *p_nxcmd;

            p_nxcmd = &game_commands[p_cmd->Next];
            if (!is_command_any_until(p_nxcmd)) {
                p_cmd->Flags &= ~PCmdF_RunUntil;
            }
        }
    }
}

void check_and_fix_thing_commands(void)
{
    ThingIdx thing;
    short i;
    ushort cmd;

    thing = same_type_head[1];
    for (i = 0; thing != 0; i++)
    {
        struct Thing *p_thing;

        if (i >= max(STHINGS_LIMIT,THINGS_LIMIT)) {
            LOGERR("Infinite loop in same type things list");
            break;
        }
        p_thing = &things[thing];

        cmd = p_thing->U.UPerson.ComHead;
        if (cmd > next_command) {
            cmd = 0;
            p_thing->U.UPerson.ComHead = cmd;
        }
        while (cmd != 0)
        {
            struct Command *p_cmd;

            p_cmd = &game_commands[cmd];

            if (0) { // Commands debug code
                char locbuf[256];
                snprint_command(locbuf, sizeof(locbuf), cmd);
                LOGSYNC("Person %hd Command %hu: %s", thing, cmd, locbuf);
            }

            cmd = p_cmd->Next;
        }
        thing = p_thing->LinkSame;
    }
}

/******************************************************************************/
