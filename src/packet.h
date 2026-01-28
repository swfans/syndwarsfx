/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file packet.h
 *     Header file for packet.c.
 * @par Purpose:
 *     Handling of packets storing player actions.
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
#ifndef PACKET_H
#define PACKET_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;
struct NetworkPlayer;

enum PacketActions
{
    PAct_NONE = 0x0,
    PAct_1 = 0x1,
    PAct_MISSN_ABORT = 0x2,
    PAct_3 = 0x3,
    PAct_4 = 0x4,
    PAct_5 = 0x5,
    PAct_6 = 0x6,
    PAct_7 = 0x7,
    PAct_8 = 0x8,
    PAct_9 = 0x9,
    PAct_A = 0xA,
    PAct_AGENT_GOTO_GND_PT_ABS = 0xB,
    PAct_AGENT_GOTO_GND_PT_REL = 0xC,
    PAct_SELECT_NEXT_WEAPON = 0xD,
    /** Player controlled person drops a currently wielded weapon,
     * so that it lays secured. */
    PAct_DROP_SELC_WEAPON_SECR = 0xE,
    PAct_F = 0xF,
    PAct_PICKUP = 0x10,
    PAct_11 = 0x11,
    PAct_12 = 0x12,
    PAct_13 = 0x13,
    PAct_ENTER_VEHICLE = 0x14,
    PAct_LEAVE_VEHICLE = 0x15,
    PAct_16 = 0x16,
    PAct_SELECT_AGENT = 0x17,
    PAct_18 = 0x18,
    PAct_AGENT_GOTO_GND_PT_REL_FF = 0x19,
    PAct_SHOOT_AT_GND_POINT = 0x1A,
    PAct_SELECT_PREV_WEAPON = 0x1B,
    PAct_PROTECT_INC = 0x1C,
    PAct_PROTECT_TOGGLE = 0x1D,
    PAct_SHOOT_AT_THING = 0x1E,
    PAct_GET_ITEM = 0x1F,
    PAct_PLANT_MINE_AT_GND_PT = 0x20,
    PAct_SELECT_SPECIFIC_WEAPON = 0x21,
    /** Player controlled person drops one of held weapons,
     * so that it lays secured. */
    PAct_DROP_HELD_WEAPON_SECR = 0x22,
    PAct_AGENT_SET_MOOD = 0x23,
    PAct_GO_ENTER_VEHICLE = 0x24,
    PAct_FOLLOW_PERSON = 0x25,
    PAct_CONTROL_MODE = 0x26,
    PAct_AGENT_GOTO_FACE_PT_ABS = 0x27,
    PAct_AGENT_GOTO_GND_PT_ABS_FF = 0x28,
    PAct_29 = 0x29,
    PAct_AGENT_GOTO_FACE_PT_ABS_FF = 0x2A,
    PAct_GO_ENTER_VEHICLE_FF = 0x2B,
    PAct_GET_ITEM_FAST = 0x2C,
    PAct_SHIELD_TOGGLE = 0x2D,
    PAct_PLANT_MINE_AT_GND_PT_FF = 0x2E,
    PAct_SHOOT_AT_GND_POINT_FF = 0x2F,
    PAct_PEEPS_SCATTER = 0x30,
    PAct_SELECT_GRP_SPEC_WEAPON = 0x31,
    PAct_AGENT_USE_MEDIKIT = 0x32,
    PAct_33 = 0x33,
    PAct_GROUP_SET_MOOD = 0x34,
    PAct_AGENT_UNKGROUP_PROT = 0x35,
    PAct_AGENT_UNKGROUP_ADD = 0x36,
    PAct_CHAT_MESSAGE_KEY = 0x37,
    PAct_SHOOT_AT_FACE_POINT = 0x38,
    PAct_SHOOT_AT_FACE_POINT_FF = 0x39,
    PAct_PLANT_MINE_AT_FACE_PT = 0x3A,
    PAct_PLANT_MINE_AT_FACE_PT_FF = 0x3B,
    PAct_3C = 0x3C,
    PAct_3D = 0x3D,
    PAct_3E = 0x3E,
    PAct_3F = 0x3F,
    PAct_THERMAL_TOGGLE = 0x40,
    PAct_CHEAT_AGENT_TELEPORT = 0x41,
    PAct_CHEAT_ALL_AGENTS = 0x42,
    PAct_AGENT_SELF_DESTRUCT = 0xFF,
};

enum PacketCheatAllAgents
{
    PCheatAA_RESURRECT_AND_WEPAPN = 0,
    PCheatAA_BEEFUP_AND_MODS = 1,
};

enum PacketActionResults
{
    PARes_DONE = 0,	/**< Action was performed, with success or not. */
    PARes_SUCCESS,	/**< Action was performed, success confirmed. */
    PARes_EBADRQC,	/**< Action skipped, unsupported action code. */
    PARes_EINVAL,	/**< Action skipped, invalid argument (ie. number out of range, thing of wrong type). */
    PARes_EBADSLT,	/**< Action skipped, invalid agent slot (player action but thing is not players agent). */
    PARes_EALREADY,	/**< Action skipped, already in requested state or already performing the command. */
    PARes_TNGBADST,	/**< Action skipped, thing in bad state (ie. agent is performing contradictory command). */
};

enum PacketRecordMode {
  PktR_NONE = 0x0,
  PktR_RECORD = 0x1,
  PktR_PLAYBACK = 0x2,
};

/** Per-player packet, for players input exchange.
 *
 * A Packet represents an information from a specific player, required
 * to achieve sync between players. If a player does something which
 * results in a change in game world, this action needs to generate
 * a packet so that the change can happen for all clients in a network
 * game. Actions which are irrelevent for other players (like moving
 * the camera, or switching view mode) should not generate packets,
 * or generate them only if not claimed by an important action.
 *
 */
struct Packet
{
    ushort Action;
    ushort Data;
    short X;
    short Y;
    short Z;
    ushort Action2;
    ushort Data2;
    short X2;
    short Y2;
    short Z2;
    ushort Action3;
    ushort Data3;
    short X3;
    short Y3;
    short Z3;
    ushort Action4;
    ushort Data4;
    short X4;
    short Y4;
    short Z4;
    s32 D1Seed;
    s32 D2Check;
};

/** Per-player packet, for players input exchange.
 *
 * Short version, for low bandwidth links (serial port and modem).
 */
struct ShortPacket
{
    ushort Action;
    ushort Data;
    short X;
    short Y;
    short Z;
    ubyte BCheck;
};

#pragma pack()
/******************************************************************************/
extern struct Packet packets[8];
extern void (*my_build_packet)(struct Packet *packet, ushort action, u32 param1, s32 x, s32 y, s32 z);
extern ubyte pktrec_mode;
extern ushort packet_rec_no;
extern ubyte packet_rec_use_levelno;

const char * get_packet_action_name(ushort atype);
const char * get_packet_action_result_text(short result);

void build_packet(struct Packet *packet, ushort action, u32 param1, s32 x, s32 y, s32 z);
void build_packet2(struct Packet *packet, ushort action, u32 param1, s32 x, s32 y, s32 z);
void build_packet3(struct Packet *packet, ushort action, u32 param1, s32 x, s32 y, s32 z);
void build_packet4(struct Packet *packet, ushort action, u32 param1, s32 x, s32 y, s32 z);

void PacketRecord_Close(void);
void PacketRecord_OpenWrite(void);
void PacketRecord_OpenRead(void);
TbResult PacketRecord_Read(struct Packet *p_pckt);
void PacketRecord_Write(struct Packet *p_pckt);
TbResult PacketRecord_ReadNP(struct NetworkPlayer *p_netplyr);
void PacketRecord_WriteNP(struct NetworkPlayer *p_netplyr);
TbBool PacketRecord_IsPlayback(void);
TbBool PacketRecord_IsRecord(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
