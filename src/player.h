/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file player.h
 *     Header file for player.c.
 * @par Purpose:
 *     Player state and information handling.
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
#ifndef PLAYER_H
#define PLAYER_H

#include "bftypes.h"
#include "game_bstype.h"
#include "cybmod.h"
#include "weapon.h"
#include "plyr_usrinp.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define PLAYERS_LIMIT 8

#define CRYO_PODS_MAX_COUNT 32
#define AGENTS_SQUAD_MAX_COUNT 4
#define LOCAL_USERS_MAX_COUNT 4

struct Thing;

enum PlrTargetType {
    TrgTp_NONE = 0,
    TrgTp_Unkn1,
    TrgTp_Unkn2,
    TrgTp_Unkn3,
    TrgTp_Unkn4,
    TrgTp_DroppedTng,
    TrgTp_Unkn6,
    TrgTp_Unkn7,
};

//TODO would make more sense to have a struct for each agent, and then a merging struct
struct AgentInfo {
    ulong Weapons[CRYO_PODS_MAX_COUNT];
    union Mod Mods[CRYO_PODS_MAX_COUNT];
    ulong Sex;
    char RandomName[CRYO_PODS_MAX_COUNT];
    struct WeaponsFourPack FourPacks[CRYO_PODS_MAX_COUNT];
    ubyte NumAgents;
};

typedef struct {
    /** Per-user agent input control parameters */
    struct SpecialUserInput UserInput[LOCAL_USERS_MAX_COUNT];
    u32 DirectControl[LOCAL_USERS_MAX_COUNT]; /* offs=0x88 */
    u32 ControlPad;
    /* Pointers to agents in the squad of this player.
     * These are guaranteed treference a valid thing (non-NULL),
     * but if an agent is not in use, the reference may be
     * to a dummy thing. Obviously the pointers are valid only
     * during the tactical mission, not in-menu.
     */
    struct Thing *MyAgent[AGENTS_SQUAD_MAX_COUNT];
    /** Previously active weapon in players panel, used when switching weapons with ability to revert. */
    ubyte PrevWeapon[AGENTS_SQUAD_MAX_COUNT];  /* offs=0xAC */
    ubyte PanelState[LOCAL_USERS_MAX_COUNT];
    /** Weapon type which is selected/highlighted on per-agent players panel. */
    ubyte PanelItem[LOCAL_USERS_MAX_COUNT];
    ushort Dummy98; /* offs=0xB8 */
    ubyte Dummy97;
    ubyte MissionAgents;
    ushort LastPacketType;
    ubyte State[4];
    ubyte SubState[4]; /* offs=0xC2 */
    ubyte PrevState[4];
    ubyte SubStateTimer[4];
    /** Double mode is multiple players using the same computer (controlling individual agents). */
    ubyte DoubleMode;
    ubyte PlayerNo;
    ulong Weapons[AGENTS_SQUAD_MAX_COUNT];
    union Mod Mods[AGENTS_SQUAD_MAX_COUNT];
    /** Per-user controlled agent command param value Y */
    short UserVY[LOCAL_USERS_MAX_COUNT];
    short SpecialItems[LOCAL_USERS_MAX_COUNT];
    short GotoFace;
    ushort PanelTimer;
    short PanelX;
    short PanelY;
    short Target;
  short field_102;
    short TargetType;
    ubyte FourPacks[5][AGENTS_SQUAD_MAX_COUNT];
    ubyte WepDelays[AGENTS_SQUAD_MAX_COUNT][32];
    /** Per-user controlled agent command param value X */
    short UserVX[LOCAL_USERS_MAX_COUNT];
    /** Per-user controlled agent command param value Z */
    short UserVZ[LOCAL_USERS_MAX_COUNT];
} PlayerInfo;


#pragma pack()
/******************************************************************************/
extern PlayerInfo players[PLAYERS_LIMIT];
extern PlayerIdx local_player_no;
extern struct AgentInfo cryo_agents;

extern ushort netgame_agent_pos_x[PLAYERS_LIMIT][4];
extern ushort netgame_agent_pos_z[PLAYERS_LIMIT][4];

extern ubyte playable_agents;

/******************************************************************************/

void place_single_player(void);

void player_update_agents_from_cryo(PlayerInfo *p_player);
void cryo_update_agents_from_player(PlayerInfo *p_player);
void players_sync_from_cryo(void);

/** Returns if given agent of the local players team has a free slot for a weapon.
 */
TbBool free_slot(ushort plagent, WeaponType wtype);

TbBool player_cryo_add_weapon_one(ushort cryo_no, WeaponType wtype);
TbBool player_cryo_remove_weapon_one(ushort cryo_no, WeaponType wtype);
TbBool player_cryo_transfer_weapon_between_agents(ushort from_cryo_no,
  ushort to_cryo_no, ubyte weapon);
TbBool player_cryo_add_cybmod(ushort cryo_no, ubyte cybmod);
const char *get_cryo_agent_name(ushort cryo_no);
void remove_agent(ubyte cryo_no);
void add_agent(ulong weapons, ushort mods);

void player_mission_agents_reset(PlayerIdx plyr);
void player_agents_init_prev_weapon(PlayerIdx plyr);
void player_agents_add_random_epidermises(PlayerInfo *p_player);
void player_agent_update_prev_weapon(struct Thing *p_agent);
short player_agent_current_or_prev_weapon(PlayerIdx plyr, ushort plagent);
TbBool player_agent_has_weapon(PlayerIdx plyr, ushort plagent, WeaponType wtype);
short player_agent_weapon_delay(PlayerIdx plyr, ushort plagent, WeaponType wtype);
void player_agent_set_weapon_delay(PlayerIdx plyr, ushort plagent, WeaponType wtype, short delay_turns);
void player_agents_clear_weapon_delays(PlayerIdx plyr);
int place_default_player(PlayerIdx plyr, TbBool replace);
void place_single_player(void);

TbBool player_can_toggle_thermal(PlayerIdx plyr);
void player_toggle_thermal(PlayerIdx plyr);
void player_update_thermal(PlayerIdx plyr);

TbBool player_agent_is_alive(PlayerIdx plyr, ushort plagent);
TbBool player_agent_is_executing_commands(PlayerIdx plyr, ushort plagent);
ThingIdx direct_control_thing_for_player(PlayerIdx plyr);
void set_default_player_control(void);
void player_target_clear(PlayerIdx plyr);
void kill_my_players(PlayerIdx plyr);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
