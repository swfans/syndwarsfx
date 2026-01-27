/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file campaign.c
 *     Routines campaign progress and missions definitions.
 * @par Purpose:
 *     Implement general functions for handling campaign progress.
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
#include "campaign.h"

#include "bffile.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfini.h"
#include <ctype.h>
#include <stdlib.h>

#include "cybmod.h"
#include "weapon.h"
#include "game_data.h"
#include "game_options.h"
#include "game.h"
#include "lvfiles.h"
#include "lvobjctv.h"
#include "misstat.h"
#include "mydraw.h"
#include "wadfile.h"
#include "swlog.h"
/******************************************************************************/

enum MissionListConfigCmd {
    MissL_MissionsCount = 1,
    MissL_TextName,
    MissL_TextId,
    MissL_FirstTrigger,
    MissL_NetscanTextId,
    MissL_WeaponsTextIdShift,
    MissL_ResearchWeapons,
    MissL_StandardWeapons,
    MissL_ResearchMods,
    MissL_StandardMods,
    MissL_TextFnMk,
    MissL_ProjectorFnMk,
    MissL_OutroFMV,
    MissL_OutroBkFn,
    MissL_Selectable,
    MissL_Type,
    MissL_SpecialEffectID,
    MissL_SourceID,
    MissL_SuccessID,
    MissL_FailID,
    MissL_SpecialTrigger,
    MissL_SuccessTrigger,
    MissL_FailTrigger,
    MissL_BankTest,
    MissL_SpecialEffectFailID,
    MissL_SpecialEffectSuccessID,
    MissL_StringIndex,
    MissL_ExtraReward,
    MissL_SuccessLevel,
    MissL_FailMap,
    MissL_FailLevel,
    MissL_MapNo,
    MissL_LevelNo,
    MissL_BankTestFail,
    MissL_Complete,
    MissL_MissionCond,
    MissL_ReLevelNo,
    MissL_CashReward,
    MissL_PANStart,
    MissL_PANEnd,
    MissL_WaitToFade,
    MissL_ImmediateNextOnSuccess,
    MissL_RemainUntilSuccess,
    MissL_IsFinalMission,
    MissL_PreProcess,
    MissL_Atmosphere,
};

const struct TbNamedEnum missions_conf_common_cmds[] = {
  {"MissionsCount",	MissL_MissionsCount},
  {"TextName",		MissL_TextName},
  {"TextId",		MissL_TextId},
  {"FirstTrigger",	MissL_FirstTrigger},
  {"NetscanTextId",	MissL_NetscanTextId},
  {"WeaponsTextIdShift",MissL_WeaponsTextIdShift},
  {"ResearchWeapons",MissL_ResearchWeapons},
  {"StandardWeapons",MissL_StandardWeapons},
  {"ResearchMods",	MissL_ResearchMods},
  {"StandardMods",	MissL_StandardMods},
  {"TextFnMk",		MissL_TextFnMk},
  {"ProjectorFnMk",	MissL_ProjectorFnMk},
  {"OutroFMV",		MissL_OutroFMV},
  {"OutroBkFn",		MissL_OutroBkFn},
  {"Selectable",	MissL_Selectable},
  {"Type",			MissL_Type},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_mission_cmds[] = {
  {"Name",			MissL_TextName},
  {"TextId",		MissL_TextId},
  {"SpecialEffectID",	MissL_SpecialEffectID},
  {"SourceID",		MissL_SourceID},
  {"SuccessID",		MissL_SuccessID},
  {"FailID",		MissL_FailID},
  {"SpecialTrigger",	MissL_SpecialTrigger},
  {"SuccessTrigger",	MissL_SuccessTrigger},
  {"FailTrigger",	MissL_FailTrigger},
  {"BankTest",		MissL_BankTest},
  {"SpecialEffectFailID",MissL_SpecialEffectFailID},
  {"SpecialEffectSuccessID",MissL_SpecialEffectSuccessID},
  {"StringIndex",	MissL_StringIndex},
  {"ResearchWeapons",MissL_ResearchWeapons},
  {"ExtraReward",	MissL_ExtraReward},
  {"SuccessLevel",	MissL_SuccessLevel},
  {"FailMap",		MissL_FailMap},
  {"FailLevel",		MissL_FailLevel},
  {"MapNo",			MissL_MapNo},
  {"LevelNo"	,	MissL_LevelNo},
  {"BankTestFail",	MissL_BankTestFail},
  {"Complete",		MissL_Complete},
  {"MissionCond",	MissL_MissionCond},
  {"ReLevelNo",		MissL_ReLevelNo},
  {"CashReward",	MissL_CashReward},
  {"PANStart",		MissL_PANStart},
  {"PANEnd",		MissL_PANEnd},
  {"WaitToFade",	MissL_WaitToFade},
  {"ImmediateNextOnSuccess",MissL_ImmediateNextOnSuccess},
  {"RemainUntilSuccess",MissL_RemainUntilSuccess},
  {"IsFinalMission",MissL_IsFinalMission},
  {"PreProcess",	MissL_PreProcess},
  {"Atmosphere",	MissL_Atmosphere},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_common_types[] = {
  {"SP",			1},
  {"MP",			2},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_atmosphere_types[] = {
  {"EARTH",			MAtmsph_Earth},
  {"SPACE",			MAtmsph_Space},
  {"MOON",			MAtmsph_Moon},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_weather_types[] = {
  {"SUNNY",			MWeathr_Sunny},
  {"RAINY",			MWeathr_Rainy},
  {"SNOWY",			MWeathr_Snowy},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_wait_to_fade_types[] = {
  {"NEVER",			1},
  {"ON_SUCCESS",	WTFade_ON_SUCCESS | 1},
  {"ON_FAIL",		WTFade_ON_FAIL | 1},
  {"ALWAYS",		WTFade_ON_SUCCESS|WTFade_ON_FAIL | 1},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_extra_reward_types[] = {
  {"Scientists",	MEReward_Scientists},
  {"ResearchLab",	MEReward_ResearchLab},
  {"CybModResearched",MEReward_CybModResearched},
  {"WeaponResearched",MEReward_WeaponResearched},
  {"CybModSingle",	MEReward_CybModSingle},
  {"WeaponSingle",	MEReward_WeaponSingle},
  {NULL,			0},
};

const struct TbNamedEnum missions_conf_any_bool[] = {
  {"True",			1},
  {"False",			2},
  {NULL,			0},
};

struct Campaign campaigns[CAMPAIGNS_MAX_COUNT];

/** Size of campaign strings within the engine buffer.
 */
ushort campaign_strings_len = 0;

/** Size of mission strings after campaign strings.
 */
extern ushort mission_strings_len; // = 0;
extern ushort display_mode;

void load_campaigns(void)
{
    ushort campgn;

    campaign_strings_len = 0;
    mission_strings_len = 0;

    for (campgn = 0; campgn < CAMPAIGNS_MAX_COUNT; campgn++) {
        if (!read_missions_conf_info(campgn))
            break;
    }
}

ushort selectable_campaigns_count(void)
{
    ushort matches;
    ushort campgn;

    matches = 0;
    for (campgn = 0; campgn < CAMPAIGNS_MAX_COUNT; campgn++) {
        struct Campaign *p_campgn;

        p_campgn = &campaigns[campgn];
        if ((p_campgn->Flags & CmpgF_IsSinglePlayer) != 0)
            if ((p_campgn->Flags & CmpgF_IsSelectable) != 0)
                matches++;
    }
    return matches;
}

void clear_mission_state_slots(void)
{
    ushort mslot;

    for (mslot = 0; mslot < MISSION_STATE_SLOTS_COUNT; mslot++) {
        mission_open[mslot] = 0;
        mission_state[mslot] = MResol_UNDECIDED;
    }
}

ushort find_mission_state_slot(ushort missi)
{
    ushort mslot;

    if (missi < 1)
        return 0;
    for (mslot = 1; mslot < MISSION_STATE_SLOTS_COUNT; mslot++) {
        if (mission_open[mslot] == missi)
            break;
        // First empty slot means all further slots are empty too
        if (mission_open[mslot] == 0) {
            mslot = 0;
            break;
        }
    }
    if (mslot >= MISSION_STATE_SLOTS_COUNT)
        mslot = 0;
    return mslot;
}

ushort find_empty_mission_state_slot(void)
{
    ushort mslot;

    for (mslot = 1; mslot < MISSION_STATE_SLOTS_COUNT; mslot++) {
        if (mission_open[mslot] == 0)
            break;
    }
    if (mslot >= MISSION_STATE_SLOTS_COUNT)
        mslot = 0;
    // since the empty slot may now become used,
    // clear the next to assure 0-terminated list
    if ((mslot > 0) && (mslot < MISSION_STATE_SLOTS_COUNT-1))
        mission_open[mslot+1] = 0;
    return mslot;
}

void remove_mission_state_slot(ushort missi)
{
    ushort mslot, i;

    mslot = find_mission_state_slot(missi);
    if (mslot == 0)
        return;

    for (i = mslot; i < MISSION_STATE_SLOTS_COUNT-1; i++)
    {
        mission_open[i] = mission_open[i + 1];
        mission_state[i] = mission_state[i + 1];
    }
    // Clear the last element, to mark it as unused
    mission_open[i] = 0;
    mission_state[i] = MResol_UNDECIDED;
}

ushort replace_mission_state_slot(ushort old_missi, ushort new_missi)
{
    ushort mslot;

    mslot = find_mission_state_slot(old_missi);
    mission_open[mslot] = new_missi;
    mission_state[mslot] = MResol_UNDECIDED;
    return mslot;
}

short get_mission_state_using_state_slot(ushort missi)
{
    ushort mslot;

    mslot = find_mission_state_slot(missi);
    if (mslot == 0) {
        LOGWARN("Tried to get state for mission %d with no slot assigned", (int)missi);
        return MResol_UNDECIDED;
    }
    return mission_state[mslot];
}

void set_mission_state_using_state_slot(ushort missi, short mstate)
{
    ushort mslot;

    mslot = find_mission_state_slot(missi);
    if (mslot == 0) {
        mslot = find_empty_mission_state_slot();
        if (mslot != 0)
            mission_open[mslot] = missi;
    }
    if (mslot == 0) {
        LOGERR("No free slot found for mission %d state %d", (int)missi, (int)mstate);
        return;
    }
    mission_state[mslot] = MResol_UNDECIDED;
}

ushort find_mission_with_map_and_level(ushort mapno, ushort level)
{
    ushort missi;

    for (missi = 1; missi < MISSIONS_MAX_COUNT; missi++) {
        struct Mission *p_missi;
        p_missi = &mission_list[missi];
        if ((p_missi->MapNo == mapno) && (p_missi->LevelNo == level))
            return missi;
        if ((p_missi->MapNo == mapno) && (p_missi->ReLevelNo == level))
            return missi;
    }
    return 0;
}

void init_mission_states(void)
{
    struct Objective *p_objectv;
    ushort missi, mslot;
    ushort objectv;
    int i;

    mslot = find_mission_state_slot(ingame.CurrentMission);
    if (mslot > 0) {
        missi = mission_open[mslot];
        mission_state[mslot] = MResol_UNDECIDED;
    } else {
        missi = ingame.CurrentMission;
        LOGWARN("Mission %d has no slot; initing anyway", (int)missi);
    }
    if (missi > next_mission) {
        LOGERR("Ignored request for unallocated mission %d", (int)missi);
        return;
    }

    mission_list[missi].BankTest = 0;

    objectv = mission_list[missi].SuccessHead;
    for (i = 0; i < 100; i++) {
        if (objectv == 0)
            break;
        p_objectv = &game_used_objectives[objectv];
        p_objectv->Status = ObvStatu_UNDECIDED;
        objectv = p_objectv->Next;
    }
    if (i >= 100)
        LOGERR("Over %d success objectives attached to mission %d", i, (int)missi);
    LOGSYNC("Prepared %d success objectives for mission %d", i, (int)missi);

    objectv = mission_list[missi].FailHead;
    for (i = 0; i < 100; i++) {
        if (objectv == 0)
            break;
        p_objectv = &game_used_objectives[objectv];
        p_objectv->Status = ObvStatu_UNDECIDED;
        objectv = p_objectv->Next;
    }
    if (i >= 100)
        LOGERR("Over %d fail objectives attached to mission %d", i, (int)missi);
    LOGSYNC("Prepared %d fail objectives for mission %d", i, (int)missi);
}

void load_missions(int num)
{
    read_missions_conf_file(num);
    apply_missions_fixups();
}

void apply_missions_fixups(void)
{
    ushort missi;

    for (missi = 0; missi < next_mission; missi++)
    {
        struct Mission *p_missi;
        int n;

        p_missi = &mission_list[missi];

        for (n=0; n < 3; n++)
        {
            if (p_missi->SpecialTrigger[n] > next_mission) {
                LOGWARN("Reference to invalid mission %d within mission %d %s; cleared",
                    (int)p_missi->SpecialTrigger[n], (int)missi, "SpecialTrigger");
                p_missi->SpecialTrigger[n] = 0;
            }
        }
        for (n=0; n < 3; n++)
        {
            struct Mission *p_trg_missi;

            if (p_missi->SuccessTrigger[n] > next_mission) {
                LOGWARN("Reference to invalid mission %d within mission %d %s; cleared",
                    (int)p_missi->SuccessTrigger[n], (int)missi, "SuccessTrigger");
                p_missi->SuccessTrigger[n] = 0;
            }
            if (p_missi->SuccessTrigger[n] == 0)
                continue;
            p_trg_missi = &mission_list[p_missi->SuccessTrigger[n]];
            // Mark mission which are immediate after previous
            if (p_missi->Flags & MisF_ImmediateNextOnSuccess)
                p_trg_missi->Flags |= MisF_ImmediatePrevious;
             else
                p_trg_missi->Flags &= ~MisF_ImmediatePrevious;
        }
        for (n=0; n < 3; n++)
        {
            if (p_missi->FailTrigger[n] > next_mission) {
                LOGWARN("Reference to invalid mission %d within  mission %d%s; cleared",
                    (int)p_missi->FailTrigger[n], (int)missi, "FailTrigger");
                p_missi->FailTrigger[n] = 0;
            }
        }
    }
}

void fix_mission_used_objectives(short missi)
{
    struct Mission *p_missi;
    struct Objective *p_objectv;
    ushort objectv;
    int i;

    p_missi = &mission_list[missi];

    objectv = p_missi->SuccessHead;
    for (i = 0; i < 100; i++) {
        if (objectv == 0)
            break;
        p_objectv = &game_used_objectives[objectv];
        fix_single_objective(p_objectv, objectv, "U");
        objectv = p_objectv->Next;
    }
    LOGSYNC("Fixed %d success objectives for mission %hd", i, missi);

    objectv = p_missi->FailHead;
    for (i = 0; i < 100; i++) {
        if (objectv == 0)
            break;
        p_objectv = &game_used_objectives[objectv];
        fix_single_objective(p_objectv, objectv, "U");
        objectv = p_objectv->Next;
    }
    LOGSYNC("Fixed %d fail objectives for mission %hd", i, missi);
}

void read_missions_bin_file(int num)
{
    TbFileHandle fh;
    char locstr[52];
    u32 fmtver;
    char *p_str;
    int i;

    fmtver = 1;
    p_str = engine_mem_alloc_ptr + engine_mem_alloc_size - 64000 + campaign_strings_len;
    sprintf(locstr, "%s/all%03d.mis", game_dirs[DirPlace_Levels].directory, num);
    fh = LbFileOpen(locstr, Lb_FILE_MODE_READ_ONLY);
    if (fh != INVALID_FILE)
    {
        LbFileRead(fh, &fmtver, sizeof(u32));
        LbFileRead(fh, &mission_strings_len, sizeof(ushort));
        LbFileRead(fh, p_str, mission_strings_len);
        LbFileRead(fh, &next_mission, sizeof(ushort));
        LbFileRead(fh, mission_list, sizeof(struct Mission) * next_mission);
        LbFileRead(fh, &next_used_objective, sizeof(ushort));
        LbFileRead(fh, game_used_objectives, sizeof(struct Objective) * next_used_objective);
        if (fmtver > 30)
            LbFileRead(fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 1320 - 33, 1320);
        LbFileClose(fh);
        display_mode = 0;
    }
    else
    {
        LOGERR("Missions file could not be opened");
    }

    { // Fill mission names from strings block
        char *p_strend;

        p_strend = p_str + mission_strings_len;
        for (i = 1; i < next_mission; i++) {
            if (p_str >= p_strend) break;
            mission_list[i].TextName = p_str;
            p_str += strlen(p_str) + 1;
        }
    }

    if (!in_network_game)
        clear_mission_status_all();
    if (fmtver < 2)
    {
        for (i = 1; i < next_mission; i++)
          mission_list[i].ReLevelNo = 0;
    }
    for (i = 1; i < next_mission; i++)
        mission_list[i].Complete = 0;
}

void read_mission_netscan_objectives_bin(void)
{
    ushort missi;

    next_mission_netscan_objective = 1;

    for (missi = 0; missi < next_mission; missi++) {
        struct Mission *p_missi;
        ushort nsobv_count;
        struct NetscanObjective *nsobv_arr;

        p_missi = &mission_list[missi];

        nsobv_arr = &mission_netscan_objectives[next_mission_netscan_objective];
        nsobv_count = load_netscan_objectives_bin(nsobv_arr,
          p_missi->MapNo, p_missi->LevelNo);

        if ((p_missi->MapNo|p_missi->LevelNo|nsobv_count) != 0)
            p_missi->NetscanObvIndex = next_mission_netscan_objective;
        else
            p_missi->NetscanObvIndex = 0;
        p_missi->NetscanObvCount = nsobv_count;

        next_mission_netscan_objective += nsobv_count;
    }
}

void save_mission_single_conf(TbFileHandle fh, struct Mission *p_missi, char *buf)
{
    if (p_missi->TextName == NULL) {
        sprintf(buf, "Name = MAP%03d - LEVEL%03d.%d\n", (int)p_missi->MapNo,
          LEVEL_NUM_STRAIN(p_missi->LevelNo), LEVEL_NUM_VARIANT(p_missi->LevelNo));
    } else {
        sprintf(buf, "Name = %s\n", p_missi->TextName);
    }
    LbFileWrite(fh, buf, strlen(buf));
    if (p_missi->TextId != 0) {
        sprintf(buf, "TextId = %hu\n", p_missi->TextId);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->SpecialEffectID != 0) {
        sprintf(buf, "SpecialEffectID = %hu\n", p_missi->SpecialEffectID);
        LbFileWrite(fh, buf, strlen(buf));
    }
    {
        sprintf(buf, "SourceID = %hu\n", p_missi->SourceID);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->SuccessID != 0) {
        sprintf(buf, "SuccessID = %hu\n", p_missi->SuccessID);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->FailID != 0) {
        sprintf(buf, "FailID = %hu\n", p_missi->FailID);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->SpecialTrigger[0]|p_missi->SpecialTrigger[1]|p_missi->SpecialTrigger[2]) != 0) {
        sprintf(buf, "SpecialTrigger = %d %d %d\n",
          (int)p_missi->SpecialTrigger[0], (int)p_missi->SpecialTrigger[1], (int)p_missi->SpecialTrigger[2]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->SuccessTrigger[0]|p_missi->SuccessTrigger[1]|p_missi->SuccessTrigger[2]) != 0) {
        sprintf(buf, "SuccessTrigger = %d %d %d\n",
          (int)p_missi->SuccessTrigger[0], (int)p_missi->SuccessTrigger[1], (int)p_missi->SuccessTrigger[2]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->FailTrigger[0]|p_missi->FailTrigger[1]|p_missi->FailTrigger[2]) != 0) {
        sprintf(buf, "FailTrigger = %d %d %d\n",
          (int)p_missi->FailTrigger[0], (int)p_missi->FailTrigger[1], (int)p_missi->FailTrigger[2]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->BankTest != 0) {
        sprintf(buf, "BankTest = %d\n", (int)p_missi->BankTest);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->SpecialEffectFailID != 0) {
        sprintf(buf, "SpecialEffectFailID = %hu\n", p_missi->SpecialEffectFailID);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->SpecialEffectSuccessID != 0) {
        sprintf(buf, "SpecialEffectSuccessID = %hu\n", p_missi->SpecialEffectSuccessID);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->StringIndex != 0) {
        sprintf(buf, "StringIndex = %hu\n", p_missi->StringIndex);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->ResearchWeapons != 0) {
        sprintf(buf, "ResearchWeapons = 0x%lx\n",
          (ulong)p_missi->ResearchWeapons);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->ExtraRewardType != 0) {
        sprintf(buf, "ExtraReward = %d %d\n",
          (int)p_missi->ExtraRewardType, (int)p_missi->ExtraRewardParam);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->SuccessLevel[0]|p_missi->SuccessLevel[1]|p_missi->SuccessLevel[2]) != 0) {
        sprintf(buf, "SuccessLevel = %d %d\n",
          (int)p_missi->SuccessLevel[0], (int)p_missi->SuccessLevel[1]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->FailMap[0]|p_missi->FailMap[1]|p_missi->FailMap[2]) != 0) {
        sprintf(buf, "FailMap = %d %d %d\n",
          (int)p_missi->FailMap[0], (int)p_missi->FailMap[1], (int)p_missi->FailMap[2]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->FailLevel[0]|p_missi->FailLevel[1]|p_missi->FailLevel[2]) != 0) {
        sprintf(buf, "FailLevel = %d %d %d\n",
          (int)p_missi->FailLevel[0], (int)p_missi->FailLevel[1], (int)p_missi->FailLevel[2]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    {
        sprintf(buf, "MapNo = %d\n", (int)p_missi->MapNo);
        LbFileWrite(fh, buf, strlen(buf));
    }
    {
        sprintf(buf, "LevelNo = %d.%d\n", LEVEL_NUM_STRAIN(p_missi->LevelNo),
          LEVEL_NUM_VARIANT(p_missi->LevelNo));
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->BankTestFail != 0) {
        sprintf(buf, "BankTestFail = %d\n", (int)p_missi->BankTestFail);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->Complete != 0) {
        sprintf(buf, "Complete = %d\n", (int)p_missi->Complete);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->MissionCond[0]|p_missi->MissionCond[1]|p_missi->MissionCond[2]|p_missi->MissionCond[3]|p_missi->MissionCond[4]) != 0) {
        sprintf(buf, "MissionCond = %d %d %d %d %d\n", (int)p_missi->MissionCond[0],
          (int)p_missi->MissionCond[1], (int)p_missi->MissionCond[2],
          (int)p_missi->MissionCond[3], (int)p_missi->MissionCond[4]);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->ReLevelNo != 0) {
        sprintf(buf, "ReLevelNo = %d.%d\n", LEVEL_NUM_STRAIN(p_missi->ReLevelNo),
          LEVEL_NUM_VARIANT(p_missi->ReLevelNo));
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->CashReward != 0) {
        sprintf(buf, "CashReward = %d\n", (int)p_missi->CashReward * 1000);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->PANStart != 0) {
        sprintf(buf, "PANStart = %d\n", (int)p_missi->PANStart);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->PANEnd != 0) {
        sprintf(buf, "PANEnd = %d\n", (int)p_missi->PANEnd);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->WaitToFade != 0) {
        sprintf(buf, "WaitToFade = 0x%x %u\n", (p_missi->WaitToFade & 0xE000),
                  (p_missi->WaitToFade & 0x1FFF));

        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->PreProcess != 0) {
      sprintf(buf, "PreProcess = %hu\n", p_missi->PreProcess);
      LbFileWrite(fh, buf, strlen(buf));
    }
    if ((p_missi->Atmosphere) != 0) {
        sprintf(buf, "Atmosphere = %d %d\n", (int)(p_missi->Atmosphere) & 0x0F, (int)(p_missi->Atmosphere >> 4) & 0x0F);
        LbFileWrite(fh, buf, strlen(buf));
    }
    if (p_missi->field_4B != 0) {
      sprintf(buf, "field_4B = %d\n", (int)p_missi->field_4B);
      LbFileWrite(fh, buf, strlen(buf));
    }
}

void save_missions_conf_file(int num)
{
    TbFileHandle fh;
    char locbuf[160];
    char conf_fname[80];
    int i;

    sprintf(conf_fname, "%s/miss%03d.ini", "conf", num);
    fh = LbFileOpen(conf_fname, Lb_FILE_MODE_NEW);
    if (fh == INVALID_FILE) {
        LOGERR("Could not create '%s' file.", conf_fname);
        return;
    }

    {
        sprintf(locbuf, "[common]\n");
        LbFileWrite(fh, locbuf, strlen(locbuf));

        sprintf(locbuf, "MissionsCount = %hu\n", next_mission);
        LbFileWrite(fh, locbuf, strlen(locbuf));

        sprintf(locbuf, "\n");
        LbFileWrite(fh, locbuf, strlen(locbuf));
    }

    for (i = 0; i < next_mission; i++) {
        struct Mission *p_missi;

        p_missi = &mission_list[i];

        sprintf(locbuf, "[mission%d]\n", i);
        LbFileWrite(fh, locbuf, strlen(locbuf));

        save_mission_single_conf(fh, p_missi, locbuf);

        sprintf(locbuf, "\n");
        LbFileWrite(fh, locbuf, strlen(locbuf));

        if (p_missi->StartHead != 0) {
            sprintf(locbuf, "[misstart%d]\n", i);
            LbFileWrite(fh, locbuf, strlen(locbuf));

            save_objective_chain_conf(fh, p_missi->StartHead, locbuf, sizeof(locbuf));

            sprintf(locbuf, "\n");
            LbFileWrite(fh, locbuf, strlen(locbuf));
        }

        {
            sprintf(locbuf, "[missuccess%d]\n", i);
            LbFileWrite(fh, locbuf, strlen(locbuf));

            save_objective_chain_conf(fh, p_missi->SuccessHead, locbuf, sizeof(locbuf));

            sprintf(locbuf, "\n");
            LbFileWrite(fh, locbuf, strlen(locbuf));
        }

        if (p_missi->FailHead != 0) {
            sprintf(locbuf, "[misfail%d]\n", i);
            LbFileWrite(fh, locbuf, strlen(locbuf));

            save_objective_chain_conf(fh, p_missi->FailHead, locbuf, sizeof(locbuf));

            sprintf(locbuf, "\n");
            LbFileWrite(fh, locbuf, strlen(locbuf));
        }

        if (p_missi->NetscanObvIndex != 0) {
            struct NetscanObjective *nsobv_arr;
            sprintf(locbuf, "[missnetscan%d]\n", i);
            LbFileWrite(fh, locbuf, strlen(locbuf));

            nsobv_arr = &mission_netscan_objectives[p_missi->NetscanObvIndex];
            save_netscan_objectives_conf(fh, nsobv_arr, p_missi->NetscanObvCount,
              locbuf, sizeof(locbuf));

            sprintf(locbuf, "\n");
            LbFileWrite(fh, locbuf, strlen(locbuf));
        }
    }
    LbFileClose(fh);
}

/** Reads common information block only, from missions file.
 */
TbBool read_missions_conf_info(int num)
{
    TbFileHandle conf_fh;
    TbBool done;
    unsigned int i;
    long k;
    char *conf_buf;
    struct TbIniParser parser;
    char conf_fname[80];
    int conf_len;
    struct Campaign *p_campgn;
    char *p_str;

    sprintf(conf_fname, "%s/miss%03d.ini", "conf", num);
    if (!LbFileExists(conf_fname)) {
        LOGSYNC("Could not find '%s' file", conf_fname);
        p_campgn = &campaigns[num];
        LbMemorySet(p_campgn, 0, sizeof(struct Campaign));
        return false;
    }
    conf_fh = LbFileOpen(conf_fname, Lb_FILE_MODE_READ_ONLY);
    if (conf_fh != INVALID_FILE) {
        conf_len = LbFileLengthHandle(conf_fh);
    } else {
        LOGSYNC("Could not open '%s' file", conf_fname);
        conf_len = 0;
    }
    if (conf_len > 1024*1024)
        conf_len = 1024*1024;
    conf_buf = LbMemoryAlloc(conf_len+16);
    if (conf_buf == NULL) {
        LOGSYNC("Could not alloc mem for '%s' file", conf_fname);
        return false;
    }
    if (conf_fh != INVALID_FILE) {
        conf_len = LbFileRead(conf_fh, conf_buf, conf_len);
        LOGSYNC("Processing '%s' file, %d bytes", conf_fname, conf_len);
        LbFileClose(conf_fh);
    }
    conf_buf[conf_len] = '\0';
    LbIniParseStart(&parser, conf_buf, conf_len);
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
    p_campgn = &campaigns[num];
    LbMemorySet(p_campgn, 0, sizeof(struct Campaign));

    p_str = engine_mem_alloc_ptr + engine_mem_alloc_size - 64000 + campaign_strings_len;
    // Parse the [common] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "common") != Lb_SUCCESS) {
        CONFDBGLOG("Could not find \"[%s]\" section, file skipped.", "common");
        LbIniParseEnd(&parser);
        LbMemoryFree(conf_buf);
        return false;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(missions_conf_common_cmds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, missions_conf_common_cmds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case MissL_MissionsCount:
            break;
        case MissL_TextName:
            i = LbIniValueGetStrWhole(&parser, p_str, 80);
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->TextName = p_str;
            p_str += strlen(p_str) + 1;
            CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_campgn->TextName);
            break;
        case MissL_TextId:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->TextId = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_campgn->TextId);
            break;
        case MissL_FirstTrigger:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->FirstTrigger = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_campgn->FirstTrigger);
            break;
        case MissL_NetscanTextId:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->NetscanTextId = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_campgn->NetscanTextId);
            break;
        case MissL_WeaponsTextIdShift:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->WeaponsTextIdShift = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_campgn->WeaponsTextIdShift);
            break;
        case MissL_ResearchWeapons:
        case MissL_StandardWeapons:
        case MissL_ResearchMods:
        case MissL_StandardMods:
            // Not reading at this point - cannot guarantee weapons list and mods list is loaded
            break;
        case MissL_TextFnMk:
            i = LbIniValueGetStrWhole(&parser, p_str, 80);
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->TextFnMk = p_str;
            p_str += strlen(p_str) + 1;
            CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_campgn->TextFnMk);
            break;
        case MissL_ProjectorFnMk:
            i = LbIniValueGetStrWhole(&parser, p_str, 80);
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->ProjectorFnMk = p_str;
            p_str += strlen(p_str) + 1;
            CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_campgn->ProjectorFnMk);
            break;
        case MissL_OutroFMV:
            i = LbIniValueGetStrWhole(&parser, p_str, 80);
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->OutroFMV = p_str;
            p_str += strlen(p_str) + 1;
            CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_campgn->OutroFMV);
            break;
        case MissL_OutroBkFn:
            i = LbIniValueGetStrWhole(&parser, p_str, 80);
            if (i <= 0) {
                CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            p_campgn->OutroBkFn = p_str;
            p_str += strlen(p_str) + 1;
            CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_campgn->OutroBkFn);
            break;
        case MissL_Selectable:
            i = LbIniValueGetNamedEnum(&parser, missions_conf_any_bool);
            if (i <= 0) {
                CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            if (i == 1)
                p_campgn->Flags |= CmpgF_IsSelectable;
            else
                p_campgn->Flags &= ~CmpgF_IsSelectable;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(p_campgn->Flags & CmpgF_IsSelectable));
            break;
        case MissL_Type:
            i = LbIniValueGetNamedEnum(&parser, missions_conf_common_types);
            if (i <= 0) {
                CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            if (i == 1)
                p_campgn->Flags |= CmpgF_IsSinglePlayer;
            else
                p_campgn->Flags &= ~CmpgF_IsSinglePlayer;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(p_campgn->Flags & CmpgF_IsSinglePlayer));
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
        case -3: // end of section
            done = true;
            break;
        default:
            CONFWRNLOG("Unrecognized command.");
            break;
        }
        LbIniSkipToNextLine(&parser);
    }
#undef COMMAND_TEXT
    LbIniParseEnd(&parser);
    LbMemoryFree(conf_buf);
    campaign_strings_len = p_str - (char *)(engine_mem_alloc_ptr + engine_mem_alloc_size - 64000);
    return true;
}

int ApIniValueGetIntDotInt(struct TbIniParser *parser, long *dst1, long *dst2)
{
    char buf[64];
    char *bend;
    int i;
    i = LbIniValueGetStrWord(parser, buf, sizeof(buf));
    if (i < 1)
        return i;
    *dst1 = strtol(buf,&bend,0);
    if (*bend != '.')
        return 0;
    bend++; // Skip the dot
    *dst2 = strtol(bend,&bend,0);
    return (int)(bend - buf);
}

/** Reads missions file, with information on all missions included.
 */
void read_missions_conf_file(int num)
{
    TbFileHandle conf_fh;
    TbBool done;
    int i, n;
    long k, l;
    char *conf_buf;
    struct TbIniParser parser;
    struct Campaign *p_campgn;
    char locbuf[320];
    char conf_fname[80];
    int conf_len;
    int missi;
    char *p_str;

    sprintf(conf_fname, "%s/miss%03d.ini", "conf", num);
    conf_fh = LbFileOpen(conf_fname, Lb_FILE_MODE_READ_ONLY);
    if (conf_fh != INVALID_FILE) {
        conf_len = LbFileLengthHandle(conf_fh);
        if (conf_len > 1024*1024)
            conf_len = 1024*1024;
        conf_buf = LbMemoryAlloc(conf_len+16);
        conf_len = LbFileRead(conf_fh, conf_buf, conf_len);
        LOGSYNC("Processing '%s' file, %d bytes", conf_fname, conf_len);
        LbFileClose(conf_fh);
    } else {
        LOGERR("Could not open '%s' file, mission list empty.", conf_fname);
        conf_buf = LbMemoryAlloc(16);
        conf_len = 0;
    }
    conf_buf[conf_len] = '\0';
    LbIniParseStart(&parser, conf_buf, conf_len);
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
    p_campgn = &campaigns[num];
    next_mission = 0;
    next_used_objective = 0;
    // Add empty objective 0
    add_used_objective(0, 0);
    next_mission_netscan_objective = 1;

    p_str = engine_mem_alloc_ptr + engine_mem_alloc_size - 64000 + campaign_strings_len;
    // Parse the [common] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "common") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section, file skipped.", "common");
        LbIniParseEnd(&parser);
        LbMemoryFree(conf_buf);
        return;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(missions_conf_common_cmds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, missions_conf_common_cmds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case MissL_MissionsCount:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            next_mission = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)next_mission);
            break;
        case MissL_ResearchWeapons:
            n = 0;
            while (1)
            {
                i = LbIniValueGetNamedEnum(&parser, weapon_names);
                if (i <= 0) {
                    if (i == 0)
                        break;
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                n |= (1 << (i-1));
            }
            p_campgn->ResearchWeapons = n;
            CONFDBGLOG("%s 0x%lx", COMMAND_TEXT(cmd_num), p_campgn->ResearchWeapons);
            break;
        case MissL_StandardWeapons:
            n = 0;
            while (1)
            {
                i = LbIniValueGetNamedEnum(&parser, weapon_names);
                if (i <= 0) {
                    if (i == 0)
                        break;
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                n |= (1 << (i-1));
            }
            p_campgn->StandardWeapons = n;
            CONFDBGLOG("%s 0x%lx", COMMAND_TEXT(cmd_num), p_campgn->StandardWeapons);
            break;
        case MissL_ResearchMods:
            n = 0;
            while (1)
            {
                i = LbIniValueGetNamedEnum(&parser, mod_names);
                if (i <= 0) {
                    if (i == 0)
                        break;
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                n |= (1 << (i-1));
            }
            p_campgn->ResearchMods = n;
            CONFDBGLOG("%s 0x%lx", COMMAND_TEXT(cmd_num), p_campgn->ResearchMods);
            break;
        case MissL_StandardMods:
            n = 0;
            while (1)
            {
                i = LbIniValueGetNamedEnum(&parser, mod_names);
                if (i <= 0) {
                    if (i == 0)
                        break;
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                n |= (1 << (i-1));
            }
            p_campgn->StandardMods = n;
            CONFDBGLOG("%s 0x%lx", COMMAND_TEXT(cmd_num), p_campgn->StandardMods);
            break;
        case MissL_TextName:
        case MissL_TextId:
        case MissL_FirstTrigger:
        case MissL_NetscanTextId:
        case MissL_WeaponsTextIdShift:
        case MissL_TextFnMk:
        case MissL_ProjectorFnMk:
        case MissL_OutroFMV:
        case MissL_OutroBkFn:
        case MissL_Selectable:
        case MissL_Type:
            // These properties are handled when reading campaign info
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
        case -3: // end of section
            done = true;
            break;
        default:
            CONFWRNLOG("Unrecognized command.");
            break;
        }
        LbIniSkipToNextLine(&parser);
    }
#undef COMMAND_TEXT
    for (missi = 0; missi < next_mission; missi++)
    {
        char sect_name[32];
        struct Mission *p_missi;

        p_missi = &mission_list[missi];
        LbMemorySet(p_missi, 0, sizeof(struct Mission));

        // Parse the [missionN] sections of loaded file
        done = false;
        sprintf(sect_name, "mission%d", missi);
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFWRNLOG("Could not find \"[%s]\" section.", sect_name);
            continue;
        }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(missions_conf_mission_cmds,cmd_num)
        while (!done)
        {
            int cmd_num;

            // Finding command number in this line
            i = 0;
            cmd_num = LbIniRecognizeKey(&parser, missions_conf_mission_cmds);
            // Now store the config item in correct place
            switch (cmd_num)
            {
            case MissL_TextName:
                i = LbIniValueGetStrWhole(&parser, p_str, 80);
                if (i <= 0) {
                    CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->TextName = p_str;
                p_str += strlen(p_str) + 1;
                CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_missi->TextName);
                break;
            case MissL_TextId:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->TextId = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->TextId);
                break;
            case MissL_SpecialEffectID:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->SpecialEffectID = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->SpecialEffectID);
                break;
            case MissL_SourceID:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->SourceID = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->SourceID);
                break;
            case MissL_SuccessID:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->SuccessID = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->SuccessID);
                break;
            case MissL_FailID:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->FailID = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->FailID);
                break;
            case MissL_SpecialTrigger:
                for (n=0; n < 3; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->SpecialTrigger[n] = k;
                }
                CONFDBGLOG("%s %d %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->SpecialTrigger[0],
                  (int)p_missi->SpecialTrigger[1], (int)p_missi->SpecialTrigger[2]);
                break;
            case MissL_SuccessTrigger:
                for (n=0; n < 3; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->SuccessTrigger[n] = k;
                }
                CONFDBGLOG("%s %d %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->SuccessTrigger[0],
                  (int)p_missi->SuccessTrigger[1], (int)p_missi->SuccessTrigger[2]);
                break;
            case MissL_FailTrigger:
                for (n=0; n < 3; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->FailTrigger[n] = k;
                }
                CONFDBGLOG("%s %d %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->FailTrigger[0],
                  (int)p_missi->FailTrigger[1], (int)p_missi->FailTrigger[2]);
                break;
            case MissL_BankTest:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->BankTest = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->BankTest);
                break;
            case MissL_SpecialEffectFailID:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->SpecialEffectFailID = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->SpecialEffectFailID);
                break;
            case MissL_SpecialEffectSuccessID:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->SpecialEffectSuccessID = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->SpecialEffectSuccessID);
                break;
            case MissL_StringIndex:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->StringIndex = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->StringIndex);
                break;
            case MissL_ResearchWeapons:
                n = 0;
                while (1)
                {
                    i = LbIniValueGetNamedEnum(&parser, weapon_names);
                    if (i <= 0) {
                        if (i == 0)
                            break;
                        CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                        break;
                    }
                    n |= (1 << (i-1));
                }
                p_missi->ResearchWeapons = n;
                CONFDBGLOG("%s 0x%lx", COMMAND_TEXT(cmd_num), p_missi->ResearchWeapons);
                break;
            case MissL_ExtraReward:
                i = LbIniValueGetNamedEnum(&parser, missions_conf_extra_reward_types);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->ExtraRewardType = i;
                i = 0;
                switch (p_missi->ExtraRewardType)
                {
                case MEReward_Scientists:
                case MEReward_ResearchLab:
                    i = LbIniValueGetLongInt(&parser, &k);
                    break;
                case MEReward_CybModResearched:
                case MEReward_CybModSingle:
                    i = LbIniValueGetNamedEnum(&parser, mod_names);
                    k = i;
                    break;
                case MEReward_WeaponResearched:
                case MEReward_WeaponSingle:
                    i = LbIniValueGetNamedEnum(&parser, weapon_names);
                    k = i;
                    break;
                }
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->ExtraRewardParam = k;
                CONFDBGLOG("%s %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->ExtraRewardType, (int)p_missi->ExtraRewardParam);
                break;
            case MissL_SuccessLevel:
                for (n=0; n < 2; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->SuccessLevel[n] = k;
                }
                CONFDBGLOG("%s %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->SuccessLevel[0],
                  (int)p_missi->SuccessLevel[1]);
                break;
            case MissL_FailMap:
                for (n=0; n < 3; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->FailMap[n] = k;
                }
                CONFDBGLOG("%s %d %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->FailMap[0],
                  (int)p_missi->FailMap[1], (int)p_missi->FailMap[2]);
                break;
            case MissL_FailLevel:
                for (n=0; n < 3; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->FailLevel[n] = k;
                }
                CONFDBGLOG("%s %d %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->FailLevel[0],
                  (int)p_missi->FailLevel[1], (int)p_missi->FailLevel[2]);
                break;
            case MissL_MapNo:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->MapNo = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->MapNo);
                break;
            case MissL_LevelNo:
                i = ApIniValueGetIntDotInt(&parser, &k, &l);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->LevelNo = LEVEL_NUM(k,l);
                CONFDBGLOG("%s %d.%d", COMMAND_TEXT(cmd_num), LEVEL_NUM_STRAIN(p_missi->LevelNo),
                  LEVEL_NUM_VARIANT(p_missi->LevelNo));
                break;
            case MissL_BankTestFail:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->BankTestFail = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->BankTestFail);
                break;
            case MissL_Complete:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->Complete = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->Complete);
                break;
            case MissL_MissionCond:
                for (n=0; n < 5; n++)
                {
                    i = LbIniValueGetLongInt(&parser, &k);
                    if (i <= 0) {
                        CONFWRNLOG("Could not read \"%s\" command parameter %u.", COMMAND_TEXT(cmd_num), n);
                        break;
                    }
                    p_missi->MissionCond[n] = k;
                }
                CONFDBGLOG("%s %d %d %d %d %d", COMMAND_TEXT(cmd_num), (int)p_missi->MissionCond[0],
                  (int)p_missi->MissionCond[1], (int)p_missi->MissionCond[2],
                  (int)p_missi->MissionCond[3], (int)p_missi->MissionCond[4]);
                break;
            case MissL_ReLevelNo:
                i = ApIniValueGetIntDotInt(&parser, &k, &l);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->ReLevelNo = LEVEL_NUM(k,l);
                CONFDBGLOG("%s %d.%d", COMMAND_TEXT(cmd_num), LEVEL_NUM_STRAIN(p_missi->ReLevelNo),
                  LEVEL_NUM_VARIANT(p_missi->ReLevelNo));
                break;
            case MissL_CashReward:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->CashReward = k / 1000;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->CashReward);
                break;
            case MissL_PANStart:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->PANStart = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->PANStart);
                break;
            case MissL_PANEnd:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->PANEnd = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->PANEnd);
                break;
            case MissL_WaitToFade:
                i = LbIniValueGetNamedEnum(&parser, missions_conf_wait_to_fade_types);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command 1st parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->WaitToFade = (i & 0xE000);
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command 2nd parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->WaitToFade |= (k & 0x1FFF);
                CONFDBGLOG("%s 0x%x %d", COMMAND_TEXT(cmd_num), (p_missi->WaitToFade & 0xE000),
                  (p_missi->WaitToFade & 0x1FFF));
                break;

            case MissL_ImmediateNextOnSuccess:
                i = LbIniValueGetNamedEnum(&parser, missions_conf_any_bool);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                if (i == 1)
                    p_missi->Flags |= MisF_ImmediateNextOnSuccess;
                else
                    p_missi->Flags &= ~MisF_ImmediateNextOnSuccess;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(p_missi->Flags & MisF_ImmediateNextOnSuccess));
                break;
            case MissL_RemainUntilSuccess:
                i = LbIniValueGetNamedEnum(&parser, missions_conf_any_bool);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                if (i == 1)
                    p_missi->Flags |= MisF_RemainUntilSuccess;
                else
                    p_missi->Flags &= ~MisF_RemainUntilSuccess;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(p_missi->Flags & MisF_RemainUntilSuccess));
                break;
            case MissL_IsFinalMission:
                i = LbIniValueGetNamedEnum(&parser, missions_conf_any_bool);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                if (i == 1)
                    p_missi->Flags |= MisF_IsFinalMission;
                else
                    p_missi->Flags &= ~MisF_IsFinalMission;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(p_missi->Flags & MisF_IsFinalMission));
                break;
            case MissL_PreProcess:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->PreProcess = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_missi->PreProcess);
                break;
            case MissL_Atmosphere:
                i = LbIniValueGetNamedEnum(&parser, missions_conf_atmosphere_types);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->Atmosphere = (p_missi->Atmosphere & 0xF0) | (i << 0);
                i = LbIniValueGetNamedEnum(&parser, missions_conf_weather_types);
                if (i <= 0) {
                    CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_missi->Atmosphere = (p_missi->Atmosphere & 0x0F) | (i << 4);
                break;
            case 0: // comment
                break;
            case -1: // end of buffer
            case -3: // end of section
                done = true;
                break;
            default:
                CONFWRNLOG("Unrecognized command.");
                break;
            }
            LbIniSkipToNextLine(&parser);
        }
#undef COMMAND_TEXT

        if ((p_missi->ReLevelNo != 0) &&
          (LEVEL_NUM_STRAIN(p_missi->LevelNo) != LEVEL_NUM_STRAIN(p_missi->ReLevelNo))) {
            CONFWRNLOG("ReLevelNo of different strain than LevelNo in \"%s\" section.", sect_name);
        }
        // Parse the [missuccessN] sections of loaded file
        done = false;
        sprintf(sect_name, "missuccess%d", missi);
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFWRNLOG("Could not find \"[%s]\" section.", sect_name);
        } else
        while (!done)
        {
            long pri;
            struct Objective *p_objectv;

            // Get the key, as it holds priority
            k = LbIniGetKey(&parser, locbuf, sizeof(locbuf));
            // Now store the config item in correct place
            switch (k)
            {
            case 0: // comment
                break;
            case -1: // end of buffer
            case -3: // end of section
                done = true;
                break;
            default:
                if ((locbuf[0] != 'P') || !isdigit(locbuf[1])) {
                    CONFWRNLOG("Unrecognized key in \"%s\" section.", sect_name);
                    break;
                }
                // Priority is in key name
                pri = atol(&locbuf[1]);
                // Now get the objective command from value
                k = LbIniValueGetStrWhole(&parser, locbuf, sizeof(locbuf));
                if (k <= 0) {
                    CONFWRNLOG("Could not read the latter of key-value pair in \"%s\" section.", sect_name);
                    break;
                }
                k = parse_next_used_objective(locbuf, sizeof(locbuf), pri, p_missi->MapNo, p_missi->LevelNo);
                if (k <= 0) {
                    CONFWRNLOG("Could parse objective command in \"%s\" section.", sect_name);
                    break;
                }
                p_objectv = &game_used_objectives[k];
                p_objectv->Next = p_missi->SuccessHead;
                p_missi->SuccessHead = k;
            }

            LbIniSkipToNextLine(&parser);
        }

        // Parse the [misfailN] sections of loaded file
        done = false;
        sprintf(sect_name, "misfail%d", missi);
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFDBGLOG("Could not find \"[%s]\" section.", sect_name);
        } else
        while (!done)
        {
            long pri;
            struct Objective *p_objectv;

            // Get the key, as it holds priority
            k = LbIniGetKey(&parser, locbuf, sizeof(locbuf));
            // Now store the config item in correct place
            switch (k)
            {
            case 0: // comment
                break;
            case -1: // end of buffer
            case -3: // end of section
                done = true;
                break;
            default:
                if ((locbuf[0] != 'P') || !isdigit(locbuf[1])) {
                    CONFWRNLOG("Unrecognized key in \"%s\" section.", sect_name);
                    break;
                }
                // Priority is in key name
                pri = atol(&locbuf[1]);
                // Now get the objective command from value
                k = LbIniValueGetStrWhole(&parser, locbuf, sizeof(locbuf));
                if (k <= 0) {
                    CONFWRNLOG("Could not read the latter of key-value pair in \"%s\" section.", sect_name);
                    break;
                }
                k = parse_next_used_objective(locbuf, sizeof(locbuf), pri, p_missi->MapNo, p_missi->LevelNo);
                if (k <= 0) {
                    CONFWRNLOG("Could parse objective command in \"%s\" section.", sect_name);
                    break;
                }
                p_objectv = &game_used_objectives[k];
                p_objectv->Next = p_missi->FailHead;
                p_missi->FailHead = k;
            }

            LbIniSkipToNextLine(&parser);
        }

        // Parse the [missnetscanN] sections of loaded file
        n = 0;
        done = false;
        sprintf(sect_name, "missnetscan%d", missi);
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFDBGLOG("Could not find \"[%s]\" section.", sect_name);
        } else
        while (!done)
        {
            // Get the key, it holds index
            k = LbIniGetKey(&parser, locbuf, sizeof(locbuf));
            // Now store the config item in correct place
            switch (k)
            {
            case 0: // comment
                break;
            case -1: // end of buffer
            case -3: // end of section
                done = true;
                break;
            default:
                if ((locbuf[0] != 'N') || !isdigit(locbuf[1])) {
                    CONFWRNLOG("Unrecognized key in \"%s\" section.", sect_name);
                    break;
                }
                // Netscan item number is in key name
                i = atol(&locbuf[1]);
                if (n < i+1) n = i+1;
                // Now get the objective command from value
                k = LbIniValueGetStrWhole(&parser, locbuf, sizeof(locbuf));
                if (k <= 0) {
                    CONFWRNLOG("Could not read the latter of key-value pair in \"%s\" section.", sect_name);
                    break;
                }
                k = parse_next_netscan_objective(locbuf, sizeof(locbuf), next_mission_netscan_objective + i);
                if (k <= 0) {
                    CONFWRNLOG("Could parse objective command in \"%s\" section.", sect_name);
                    break;
                }
            }

            LbIniSkipToNextLine(&parser);
        }
        p_missi->NetscanObvIndex = next_mission_netscan_objective;
        p_missi->NetscanObvCount = n;
        next_mission_netscan_objective += n;
    }
#undef CONFDBGLOG
#undef CONFWRNLOG
    LbIniParseEnd(&parser);
    LbMemoryFree(conf_buf);
    mission_strings_len = p_str - (char *)(engine_mem_alloc_ptr + engine_mem_alloc_size - 64000 + campaign_strings_len);
}

TbResult load_netscan_text_data(ushort mapno, ushort level)
{
    char *p;
    int i, k;
    TbBool found;
    int totlen;
    short cmapno, clevel;
    char secnum_str[5];
    int secnum_int;

    found = 0;
    totlen = load_file_alltext("textdata/netscan.txt", netscan_text);
    if (totlen == Lb_FAIL) {
        return Lb_FAIL;
    }
    if (totlen >= netscan_text_len) {
        LOGERR("Insufficient memory for netscan_text - %d instead of %d", netscan_text_len, totlen);
        totlen = netscan_text_len - 1;
    }
    p = netscan_text;
    while ( !found )
    {
        // Find section
        char c;
        do
            c = *p++;
        while ((c != '[') && (c != '\0'));
        if (c != '[') break;
        
        // Get section name
        for (k = 0; k < 4; k++)
        {
            secnum_str[k] = *p++;
        }
        secnum_str[4] = 0;
        // Go to EOLN
        do
            c = *p++;
        while ((c != '\n') && (c != '\0'));
        // Get number from section name
        sscanf(secnum_str, "%d", &secnum_int);
        cmapno = secnum_int / 100;
        clevel = secnum_int % 100;
        if ((clevel == level) && (cmapno == mapno))
        {
            found = 1;
            for (i = 0; i < netscan_objectives_count; i++)
            {
                char *text;
                text = p;
                do
                    c = *p++;
                while ((c != '\n') && (c != '\0'));
                *(p - 1) = '\0';

                netscan_objectives[i].TextOffset = text - netscan_text;
                my_preprocess_text(netscan_text + netscan_objectives[i].TextOffset);
                k = my_count_lines(netscan_text + netscan_objectives[i].TextOffset);
                netscan_objectives[i].TextLines = k;
            }
        }
    }
    return found ? Lb_SUCCESS : Lb_OK;
}

TbResult load_mission_name_text(ubyte missi)
{
    int totlen;
    ushort len;
    int cmissi;
    char *p;
    char c;

    totlen = load_file_alltext("textdata/names.txt", memload);
    if (totlen == Lb_FAIL) {
        mission_name[0] = '\0';
        return Lb_FAIL;
    }
    if (totlen >= memload_len) {
        LOGERR("Insufficient memory for memload - %d instead of %d", memload_len, totlen);
        totlen = memload_len - 1;
    }
    memload[totlen] = '\0';

    p = (char *)memload;
    cmissi = -1;
    while ( 1 )
    {
        if (*p == '\0')
            break;
        if ((*p != '#') && (*p != '\r'))
            sscanf(p, "%d", &cmissi);
        if (missi == cmissi)
            break;
        do
            c = *p++;
        while ((c != '\n') && (c != '\0'));
    }

    do
        c = *p++;
    while ((c != '.') && (c != '\n') && (c != '\0'));

    for (len = 0; len < sizeof(mission_name)-1; len++)
    {
        c = *p++;
        if ((c == '\r') || (c == '\0'))
            break;
        c = toupper(c);
        mission_name[len] = c;
    }
    mission_name[len] = '\0';

    return Lb_SUCCESS;
}

TbBool mission_remain_until_success(ushort missi)
{
    struct Mission *p_missi;
    p_missi = &mission_list[missi];
    return ((p_missi->Flags & MisF_RemainUntilSuccess) != 0);
}

TbBool mission_has_immediate_next_on_success(ushort missi)
{
    struct Mission *p_missi;
    p_missi = &mission_list[missi];
    return ((p_missi->Flags & MisF_ImmediateNextOnSuccess) != 0);
}

TbBool mission_has_immediate_previous(ushort missi)
{
    struct Mission *p_missi;
    p_missi = &mission_list[missi];
    return ((p_missi->Flags & MisF_ImmediatePrevious) != 0);
}

TbBool mission_has_fail_email_or_triggers(ushort missi)
{
    struct Mission *p_missi;
    p_missi = &mission_list[missi];
    if (p_missi->FailID != 0)
        return true;
    if ((p_missi->FailTrigger[0]|p_missi->FailTrigger[1]|p_missi->FailTrigger[2]) != 0)
        return true;
    return false;
}

TbBool mission_is_final_at_game_end(ushort missi)
{
    struct Mission *p_missi;
    p_missi = &mission_list[missi];
    return ((p_missi->Flags & MisF_IsFinalMission) != 0);
}

void missions_clear_bank_tests(void)
{
    short missi;

    for (missi = 1; missi < next_mission; missi++)
    {
        mission_list[missi].BankTest = 0;
        mission_list[missi].BankTestFail = 0;
    }
}

/******************************************************************************/
