/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file packetfe.h
 *     Header file for packetfe.c.
 * @par Purpose:
 *     Handling of front-end packets for syncing player data.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef PACKETFE_H
#define PACKETFE_H

#include "bftypes.h"
#include "cybmod.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum NetPacketActions
{
    NPAct_NONE = 0x0,
    NPAct_Unkn01,
    NPAct_NetReset,
    NPAct_GrPaintClear,
    NPAct_SetTechLvl,
    NPAct_SetStCredits,
    NPAct_SetGameOptions,
    NPAct_SetTeam,
    NPAct_SetFaction,
    NPAct_SetCity,
    NPAct_ChatMsg,
    /** Draw a line in group paint box. */
    NPAct_GrPaintDrawLn,
    NPAct_PlyrEject,
    NPAct_PlyrLogOut,
    NPAct_PlyrWeapModsSync,
    NPAct_PlyrFourPackSync,
    NPAct_GrPaintDrawPt,
    /** Progress with syncing general variables, no specific action taken. */
    NPAct_ProgressOnly,
    NPAct_GrPaintPt1Upd,
    NPAct_MissionInit,
};

struct NetworkPlayerUFourPacks {
  ubyte FourPacks[4][5];
  ubyte MissionAgents;
};

struct NetworkPlayerUProgress {
  s32 Credits;
  ubyte Team;
  ubyte Faction;
  ubyte TechLevel;
  ubyte SelectedCity;
  ushort GrPaintX;
  ushort GrPaintY;
  ubyte ControlMode[4];
  ubyte DoubleMode;
  ubyte val_flags_08;
  ubyte GrPaintColour;
  ubyte SelectedUser;
  long Expenditure;
};

struct NetworkPlayerUWepMod {
  u32 Weapons[4];
  union Mod Mods[4];
};

struct NetworkPlayerUMissInit {
  ulong Seed;
  short GameMode;
};

struct NetworkPlayerUUnkn {
    long npfield_1;
    sbyte npfield_5;
    sbyte npfield_6;
    sbyte npfield_7;
    sbyte SelectedCity;
    sbyte npfield_9[2];
    sbyte npfield_b;
    sbyte npfield_c;
    ushort npfield_d;
    ushort npfield_e;
    sbyte DoubleMode;
    sbyte npfield_12;
    sbyte npfield_13;
    sbyte npfield_14;
    long npfield_15;
};

struct NetworkPlayer { // sizeof=26
    ubyte Type;
    union {
      struct NetworkPlayerUProgress Progress;
      struct NetworkPlayerUFourPacks FourPacks;
      struct NetworkPlayerUWepMod WepMod;
      struct NetworkPlayerUMissInit MissInit;
      char Text[24];
      struct NetworkPlayerUUnkn Unkn;
    } U;
    sbyte npfield_19;
};

#pragma pack()
/******************************************************************************/
TbBool net_local_player_hosts_the_game(void);

void net_schedule_local_player_logout(void);
void net_schedule_local_player_reset(void);
void net_schedule_player_eject_sync(void);
void net_schedule_player_cryo_equip_sync(void);
void net_schedule_player_equip_fourpack_sync(void);
void net_schedule_player_city_choice_sync(void);
void net_schedule_game_options_sync(void);
void net_schedule_player_faction_change_sync(void);
void net_schedule_player_team_change_sync(void);
void net_schedule_player_chat_message_sync(const char *msg_text);
void net_schedule_player_grpaint_action_sync(ubyte action,
  short pos_x, short pos_y, TbPixel colour);
void net_schedule_player_grpaint_clear_sync(void);
void net_immediate_random_seed_sync(void);

void net_players_all_set_no_action(void);
TbBool net_player_no_action_scheduled(int plyr);
TbBool net_player_packet_has_progress_data(int plyr);

void net_unkn_func_33(void);
void net_player_scheduled_action_prepare_packet(void);

void net_player_action_prepare(int plyr);
void net_player_action_execute(int plyr, int netplyr);
void net_player_update_from_progress_packet_hostonly(void);
TbBool net_players_immediate_exchange(void);

void net_players_copy_equip_and_cryo(void);
void net_players_copy_equip_and_cryo_now(void);

void packet_read_whole_player_init(void);
void packet_write_whole_player_init(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
