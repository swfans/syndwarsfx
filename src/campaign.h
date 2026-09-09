/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file campaign.h
 *     Header file for campaign.c.
 * @par Purpose:
 *     Campaign progress and missions definitions.
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
#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define CAMPAIGNS_MAX_COUNT 6
#define MISSIONS_MAX_COUNT 120
#define MISSION_STATE_SLOTS_COUNT 50

enum CampaignFlags {
    CmpgF_IsSinglePlayer = 0x0001,
    CmpgF_IsSelectable   = 0x0002,
};

enum MissionFlags {
    /** Set if after this mission ends with success, a next one should start
     * immediatelly without going through menu. It is not possible to save
     * game or reequip your agents before the next mission starts.
     */
    MisF_ImmediateNextOnSuccess = 0x0001,
    /** Set if the current mission in set as SuccessTrigger in a mission
     * which has ImmediateNextOnSuccess flag set. This flag has no config file
     * mnemonic - it is set automatically for all missions pointed at by
     * other missions with expected flags. If this flag is set, the mission
     * will not have its own briefing slot, as it should replace values
     * within slot of the previous mission.
     */
    MisF_ImmediatePrevious = 0x0002,
    /** Set if failing this mission does not cause game over nor the mission
     * disappearing from selection. The mission can be played any amount
     * of times, and disappears only if it is finally completed.
     */
    MisF_RemainUntilSuccess = 0x0004,
    /** Set if the current mission it the final one in the campaign. After
     * the final mission is completed, outro sequence will play.
     */
    MisF_IsFinalMission = 0x0008,
};

enum MissionExtraRewards {
    MEReward_None = 0,
    MEReward_Scientists,
    MEReward_ResearchLab,
    MEReward_CybModResearched,
    MEReward_WeaponResearched,
    MEReward_CybModSingle,
    MEReward_WeaponSingle,
};

enum MissionAtmosphere {
    MAtmsph_None = 0,
    MAtmsph_Earth,
    MAtmsph_Space,
    MAtmsph_Moon,
};

enum MissionWeather {
    MWeathr_None = 0,
    MWeathr_Sunny,
    MWeathr_Rainy,
    MWeathr_Snowy,
};

//TODO use the same values as ObjectiveStatuses?
enum MissionResolutionStatus {
    MResol_FAILED = -1,
    MResol_UNDECIDED = 0,
    MResol_COMPLETED = 1,
};

enum WaitToFadeFlags {
    WTFade_ON_SUCCESS = 0x8000,
    WTFade_ON_FAIL = 0x4000,
};

struct Campaign {
    /** Default campaign title. */
    const char *TextName;
    /** Translated campaign title text ID. */
    ushort TextId;
    /** Index of first mission in the campaign. */
    ushort FirstTrigger;
    /** Translated text ID for netscan button. */
    ushort NetscanTextId;
    /** Shift for translated text IDs for all weapon names. */
    ushort WeaponsTextIdShift;
    ulong ResearchWeapons;
    ulong StandardWeapons;
    ulong ResearchMods;
    ulong StandardMods;
    /** Char marking text translations file names for the campaign. */
    const char *TextFnMk;
    /** Char marking projector file names for the campaign. */
    const char *ProjectorFnMk;
    /** Outro movie file name. */
    const char *OutroFMV;
    /** Outro background after the FMV ends. */
    const char *OutroBkFn;
    /** Flag switches for the campaign. */
    ushort Flags;
};

struct Mission { // sizeof=76
    /** Mission code name from developers. */
    char *TextName;
    /** Translated mission title text ID. */
    ushort TextId;
    ushort StartHead;
    ushort SpecialEffectID;
    /** Head of mission objective chain which indicates success in achieving goals. */
    ushort SuccessHead;
    /** Head of mission objective chain which indicates failure to achieve goals. */
    ushort FailHead;
    ushort SourceID;
    ushort SuccessID;
    ushort FailID;
    /** Special triggers; replaced by load game. */
    ubyte SpecialTrigger[3];
    ubyte SuccessTrigger[3];
    ubyte FailTrigger[3];
    /** Objective priority number reached for chain within SuccessHead. */
    ubyte BankTest;
    ushort SpecialEffectFailID;
    ushort SpecialEffectSuccessID;
    ushort StringIndex;
    /** Index of first of netscan objectives (mission brief objectives) assigned to this mission. */
    ushort NetscanObvIndex;
    /** Amount of netscan objectives (mission brief objectives) assigned to this mission. */
    ubyte NetscanObvCount;
    /** Weapons made researchable by completing this mission. */
    ulong ResearchWeapons;
    /** Type of an extra, unique reward given to the player for mission complete. */
    ubyte ExtraRewardType;
    /** Parameter for the extra unique reward, meaning depends on its type. */
    ushort ExtraRewardParam;
    ubyte SuccessLevel[2];
    ubyte FailMap[3];
    ubyte FailLevel[3];
    /** Map number on which this mission takes place. */
    ubyte MapNo;
    /** Level number identifying level file to play. */
    ubyte LevelNo;
    /** Objective priority number reached for chain within FailHead. */
    ubyte BankTestFail;
    /** Whether the mission was completed by the current player; replaced by load game. */
    sbyte Complete;
    ubyte MissionCond[5];
    /** Level number when replaying the mission, if it's different than the first LevelNo (ie. skips cutscene). */
    ubyte ReLevelNo;
    /** Lump sum cash reward for mission complete, added to the rewards from briefcases and persuasion. */
    ushort CashReward;
    ubyte PANStart;
    ubyte PANEnd;
    /** When fade to black is expected at end of mission, sets how many game turns that will take. */
    ushort WaitToFade;
    /** Amount of game turns to blindly process before the level starts. */
	ushort PreProcess;
    /** Flag switches for the mission. */
    ushort Flags;
    /** Atmosphere and weather during the mission. */
    ubyte Atmosphere;
    ubyte field_4B;
};

#pragma pack()
/******************************************************************************/
extern struct Campaign campaigns[CAMPAIGNS_MAX_COUNT];
extern struct Mission mission_list[MISSIONS_MAX_COUNT];

extern short mission_open[MISSION_STATE_SLOTS_COUNT];
extern short mission_state[MISSION_STATE_SLOTS_COUNT];

/** Text name of the current mission (citydrop name).
 */
extern char mission_name[50];

extern char *netscan_text;
#define netscan_text_len 16384

void load_campaigns(void);
ushort selectable_campaigns_count(void);

void clear_mission_state_slots(void);
ushort find_mission_state_slot(ushort missi);
ushort find_empty_mission_state_slot(void);
void remove_mission_state_slot(ushort missi);
ushort replace_mission_state_slot(ushort old_missi, ushort new_missi);
short get_mission_state_using_state_slot(ushort missi);
void set_mission_state_using_state_slot(ushort missi, short mstate);
void init_mission_states(void);
void missions_clear_bank_tests(void);

ushort find_mission_with_map_and_level(ushort mapno, ushort level);
ushort find_first_mission_with_map(short mapno);
void fix_mission_used_objectives(short missi);

TbBool mission_remain_until_success(ushort missi);
TbBool mission_has_immediate_next_on_success(ushort missi);
TbBool mission_has_immediate_previous(ushort missi);

/** Returns if the mission has either email message or triggers to be received
 * by player on mission fail.
 */
TbBool mission_has_fail_email_or_triggers(ushort missi);
TbBool mission_is_final_at_game_end(ushort missi);

TbBool read_missions_conf_info(int num);
void load_missions(int num);
void save_missions_conf_file(int num);
void read_missions_conf_file(int num);
void read_missions_bin_file(int num);
void read_mission_netscan_objectives_bin(void);
void apply_missions_fixups(void);

TbResult load_netscan_text_data(ushort mapno, ushort level);
TbResult load_mission_name_text(ubyte missi);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
