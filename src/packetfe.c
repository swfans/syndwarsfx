/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file packetfe.c
 *     Handling of front-end packets for syncing player data.
 * @par Purpose:
 *     Besides in-mission packets, some front-end data exchange is also require
 *     to start a network game. This module stores functions for handling
 *     such packets.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packetfe.h"

#include "bfutility.h"
#include <stdlib.h>
#include <string.h>

#include "fecryo.h"
#include "fenet.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "network.h"
#include "packet.h"
#include "player.h"
#include "swlog.h"
/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
extern struct NetworkPlayer network_players[8];

TbBool net_local_player_hosts_the_game(void)
{
#if 0
    TbBool ret;
    asm volatile ("call ASM_net_local_player_hosts_the_game\n"
        : "=r" (ret) : );
    return ret;
#endif
    int plyr;

    plyr = LbNetworkPlayerNumber();
    return (login_control__State == LognCt_Unkn6) || (plyr == net_host_player_no);
}

void net_players_all_set_no_action(void)
{
    int plyr;

    for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++)
    {
        network_players[plyr].Type = NPAct_ProgressOnly;
    }
}

TbBool net_player_action_is(int plyr, ubyte action)
{
    struct NetworkPlayer *p_netplyr;

    p_netplyr = &network_players[plyr];
    return (p_netplyr->Type == action);
}

TbBool net_player_no_action_scheduled(int plyr)
{
    return net_player_action_is(plyr, NPAct_ProgressOnly);
}

TbBool net_player_action_type_has_progress(ubyte nptype)
{
    return (nptype != NPAct_ChatMsg) &&
      (nptype != NPAct_PlyrWeapModsSync) &&
      (nptype != NPAct_PlyrFourPackSync) &&
      (nptype != NPAct_MissionInit);
}

TbBool net_player_packet_has_progress_data(int plyr)
{
    struct NetworkPlayer *p_netplyr;

    if (plyr < 0)
        return false;
    p_netplyr = &network_players[plyr];
    return net_player_action_type_has_progress(p_netplyr->Type & 0x1F);
}


void net_schedule_local_player_logout(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_PlyrLogOut;
}

void net_schedule_local_player_reset(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_NetReset;
}

void net_schedule_player_eject_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_PlyrEject;
}

void net_schedule_player_cryo_equip_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_PlyrWeapModsSync;
}

void net_schedule_player_equip_fourpack_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_PlyrFourPackSync;
}

void net_schedule_player_city_choice_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_SetCity;
}

void net_schedule_game_options_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_SetGameOptions;
}

void net_schedule_player_faction_change_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_SetFaction;
}

void net_schedule_player_team_change_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_SetTeam;
}

void net_schedule_player_chat_message_sync(const char *msg_text)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_ChatMsg;
    strncpy(network_players[plyr].U.Text, msg_text,
      sizeof(network_players[0].U.Text));
}

void net_schedule_player_grpaint_action_sync(ubyte action, short pos_x, short pos_y, TbPixel colour)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = action;
    network_players[plyr].U.Progress.GrPaintX = pos_x;
    network_players[plyr].U.Progress.GrPaintY = pos_y;
    network_players[plyr].U.Progress.GrPaintColour = colour;
}

void net_schedule_player_grpaint_clear_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_GrPaintClear;
}

void net_schedule_player_random_init_sync(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    network_players[plyr].Type = NPAct_MissionInit;
}

void net_immediate_random_seed_sync(void)
{
    if (net_local_player_hosts_the_game())
    {
        net_schedule_player_random_init_sync();
        net_player_action_prepare(net_host_player_no);
        LbNetworkExchange(network_players, sizeof(struct NetworkPlayer));
    } else {
        LbNetworkExchange(network_players, sizeof(struct NetworkPlayer));
        net_player_action_execute(net_host_player_no, net_host_player_no);
    }
}

void agents_copy_fourpacks_netplayer_to_player(int plyr, struct NetworkPlayer *p_netplyr)
{
    short plagent, fp;

    for (plagent = 0; plagent < 4; plagent++)
    {
        for (fp = 0; fp < WFRPK_COUNT; fp++) {
            players[plyr].FourPacks[plagent][fp] = \
              p_netplyr->U.FourPacks.FourPacks[plagent][fp];
        }
    }
}

void agents_copy_fourpacks_netplayer_to_cryo(struct NetworkPlayer *p_netplyr)
{
    short plagent, fp;

    for (plagent = 0; plagent < 4; plagent++)
    {
        for (fp = 0; fp < WFRPK_COUNT; fp++) {
            cryo_agents.FourPacks[plagent].Amount[fp] = \
              p_netplyr->U.FourPacks.FourPacks[plagent][fp];
        }
    }
}

void agents_copy_fourpacks_cryo_to_netplayer(struct NetworkPlayer *p_netplyr)
{
    short plagent, fp;

    for (plagent = 0; plagent < 4; plagent++)
    {
        for (fp = 0; fp < WFRPK_COUNT; fp++) {
            p_netplyr->U.FourPacks.FourPacks[plagent][fp] =
              cryo_agents.FourPacks[plagent].Amount[fp];
        }
    }
}

void agents_copy_wepmod_cryo_to_netplayer(struct NetworkPlayer *p_netplyr)
{
    short plagent;

    for (plagent = 0; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
    {
        p_netplyr->U.WepMod.Weapons[plagent] = cryo_agents.Weapons[plagent];
        p_netplyr->U.WepMod.Mods[plagent] = cryo_agents.Mods[plagent];
    }
}

void agents_copy_wepmod_netplayer_to_player(int plyr, struct NetworkPlayer *p_netplyr)
{
    short plagent;

    for (plagent = 0; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
    {
        players[plyr].Weapons[plagent] = p_netplyr->U.WepMod.Weapons[plagent];
        players[plyr].Mods[plagent] = p_netplyr->U.WepMod.Mods[plagent];
    }
}

void agents_copy_wepmod_netplayer_to_cryo(struct NetworkPlayer *p_netplyr)
{
    short plagent;

    for (plagent = 0; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
    {
        cryo_agents.Weapons[plagent] = p_netplyr->U.WepMod.Weapons[plagent];
        cryo_agents.Mods[plagent] = p_netplyr->U.WepMod.Mods[plagent];
    }
}

void net_player_copy_to_progress_packet(struct NetworkPlayer *p_netplyr)
{
    int plyr;
    int i;

    plyr = net_host_player_no;
    p_netplyr->U.Progress.SelectedCity = login_control__City;
    if (((gameturn & 1) == 0) && ((net_game_play_flags & NGPF_Unkn08) != 0))
        p_netplyr->U.Progress.Credits = -ingame.Credits;
    else
        p_netplyr->U.Progress.Credits = login_control__Money;

    p_netplyr->U.Progress.TechLevel = login_control__TechLevel;
    p_netplyr->U.Progress.val_flags_08 = net_game_play_flags;
    p_netplyr->U.Progress.val_181189 = byte_181189;
    p_netplyr->U.Progress.val_181183 = byte_181183;
    p_netplyr->U.Progress.val_15516D = byte_15516D;
    p_netplyr->U.Progress.Expenditure = ingame.Expenditure;

    for (i = 0; i < 4; i++)
    {
        p_netplyr->U.Progress.ControlMode[i] =
          players[plyr].UserInput[i].ControlMode;
    }
    p_netplyr->U.Progress.DoubleMode = players[plyr].DoubleMode;
}

void net_player_update_from_progress_packet(int plyr)
{
    struct NetworkPlayer *p_netplyr;
    int i;

    p_netplyr = &network_players[plyr];
    group_types[plyr] = p_netplyr->U.Progress.val_181183;
    byte_1C5C28[plyr] = p_netplyr->U.Progress.val_181189;
    if (net_host_player_no == plyr)
    {
        if ((net_game_play_flags & NGPF_Unkn02) == 0)
            login_control__TechLevel = p_netplyr->U.Progress.TechLevel;
        if ((net_game_play_flags & NGPF_Unkn01) == 0) {
            login_control__Money = abs(p_netplyr->U.Progress.Credits);
            ingame.Credits = login_control__Money;
            ingame.CashAtStart = login_control__Money;
        }
        if ((net_game_play_flags & NGPF_Unkn08) != 0)
        {
            long credits;

          credits = p_netplyr->U.Progress.Credits;
          if (credits >= 0) {
              login_control__Money = credits;
              ingame.CashAtStart = credits;
          } else {
              ingame.Credits = -credits;
          }
          ingame.Expenditure = p_netplyr->U.Progress.Expenditure;
        }
    }
    for ( i = 0; i < 4; i++)
    {
        players[plyr].UserInput[i].ControlMode =
          p_netplyr->U.Progress.ControlMode[i];
    }
    players[plyr].DoubleMode = p_netplyr->U.Progress.DoubleMode;
}

void net_player_update_from_progress_packet_hostonly(void)
{
    struct NetworkPlayer *p_netplyr;

    p_netplyr = &network_players[net_host_player_no];
    login_control__TechLevel = p_netplyr->U.Progress.TechLevel;
    net_game_play_flags = p_netplyr->U.Progress.val_flags_08;
    login_control__City = p_netplyr->U.Progress.SelectedCity;
    ingame.Expenditure = p_netplyr->U.Progress.Expenditure;
    login_control__Money = abs(p_netplyr->U.Progress.Credits);
    ingame.Credits = login_control__Money;
    ingame.CashAtStart = login_control__Money;
}

void net_player_action_prepare(int plyr)
{
    struct NetworkPlayer *p_netplyr;

    p_netplyr = &network_players[plyr];

    switch (p_netplyr->Type)
    {
    case NPAct_PlyrWeapModsSync:
        agents_copy_wepmod_cryo_to_netplayer(p_netplyr);
        break;
    case NPAct_PlyrFourPackSync:
        p_netplyr->U.FourPacks.MissionAgents = players[plyr].MissionAgents;
        agents_copy_fourpacks_cryo_to_netplayer(p_netplyr);
        break;
    case NPAct_MissionInit:
        p_netplyr->U.MissInit.Seed = lbSeed;
        p_netplyr->U.MissInit.GameMode = ingame.GameMode;
        break;
    case NPAct_ChatMsg:
        // Chat mssage packet is filled while scheduling
        break;
    case NPAct_ProgressOnly:
        p_netplyr->Type = NPAct_NONE;
        // Fall through
    default:
        net_player_copy_to_progress_packet(p_netplyr);
        break;
    }
}

TbBool net_players_immediate_exchange(void)
{
    if (LbNetworkExchange(network_players, sizeof(struct NetworkPlayer)) != 1)
    {
        LbNetworkSessionStop();
        net_new_game_prepare();
        if (nsvc.I.Type != NetSvc_IPX)
        {
            if (byte_1C4A6F)
                LbNetworkHangUp();
            LbNetworkReset();
            net_service_started = 0;
        }
        return false;
    }
    return true;
}

void net_player_action_execute(int plyr, int netplyr)
{
    struct NetworkPlayer *p_netplyr;
    int i;

    if (net_player_no_action_scheduled(plyr)) {
        LOGSYNC("Skip player %d packet, no action", plyr);
        return;
    }

    p_netplyr = &network_players[plyr];
    LOGSYNC("Execute player %d packet, action=%d", plyr, (int)p_netplyr->Type);
    if (net_player_packet_has_progress_data(plyr)) {
        net_player_update_from_progress_packet(plyr);
    }

    switch (p_netplyr->Type & 0x1F)
    {
    case NPAct_NetReset:
        login_control__State = LognCt_Unkn8;
        LbNetworkShutDownListeners();
        net_sessionlist_clear();
        break;
    case NPAct_GrPaintClear:
        net_grpaint_clear_op();
        break;
    case NPAct_SetTechLvl:
        login_control__TechLevel = p_netplyr->U.Progress.TechLevel;
        break;
    case NPAct_SetStCredits:
        i = p_netplyr->U.Progress.Credits;
        ingame.Expenditure = 0;
        login_control__Money = i;
        ingame.Credits = i;
        ingame.CashAtStart = i;
        break;
    case NPAct_SetGameOptions:
        refresh_equip_list = 1;
        net_game_play_flags = p_netplyr->U.Progress.val_flags_08;
        break;
    case NPAct_SetCity:
        login_control__City = p_netplyr->U.Progress.SelectedCity;
        break;
    case NPAct_MissionInit:
        lbSeed = p_netplyr->U.MissInit.Seed;
        ingame.GameMode = p_netplyr->U.MissInit.GameMode;
        break;
    case NPAct_ChatMsg:
        // Free last net_players[] slot
        {
            struct NetPlayer2 *p_nplyr1;
            struct NetPlayer2 *p_nplyr2;
            p_nplyr2 = &net_players[1];
            p_nplyr1 = &net_players[0];
            for (i = 0; i < 3; i++)
            {
                byte_1C6DDC[i] = byte_1C6DDC[i+1];
                strcpy(p_nplyr1->field_0, p_nplyr2->field_0);
                p_nplyr1++;
                p_nplyr2++;
            }
        }
        // Fill the slot from packet
        {
            struct NetPlayer2 *p_nplyr1;
            const char *p_text;
            byte_1C6DDC[4] = plyr;
            p_nplyr1 = &net_players[4];
            p_text = p_netplyr->U.Text;
            strcpy(p_nplyr1->field_0, p_text);
        }
        break;
    case NPAct_GrPaintDrawLn:
        net_grpaint_draw_op(
          p_netplyr->U.Progress.GrPaintX,
          p_netplyr->U.Progress.GrPaintY,
          p_netplyr->U.Progress.GrPaintColour,
            1, plyr);
        break;
    case NPAct_PlyrEject:
        byte_15516D = -1;
        reset_net_screen_EJECT_flags();
        LbNetworkSessionStop();
        if (nsvc.I.Type == NetSvc_IPX)
        {
            if (p_netplyr->U.Progress.val_15516D == netplyr)
            {
                net_new_game_prepare();
                if (screentype == SCRT_CRYO)
                {
                    update_flic_mods(flic_mods);
                    set_mod_draw_states_flag08();
                }
            }
        } else {
            if (p_netplyr->U.Progress.val_15516D != netplyr)
                LbNetworkSessionStop();
            net_new_game_prepare();
            if (byte_1C4A6F)
                LbNetworkHangUp();
            LbNetworkReset();
            net_service_started = 0;
        }
        break;
    case NPAct_PlyrLogOut:
        LbNetworkSessionStop();
        if (nsvc.I.Type == NetSvc_IPX)
        {
            if (plyr == netplyr || net_host_player_no == plyr)
            {
                net_new_game_prepare();
                net_sessionlist_clear();
                net_unkn2_names_clear();
            }
        }
        else
        {
            net_new_game_prepare();
            net_unkn2_names_clear();
            if (byte_1C4A6F)
                LbNetworkHangUp();
            LbNetworkReset();
            net_service_started = 0;
        }
        if (screentype == SCRT_CRYO)
        {
            update_flic_mods(flic_mods);
            set_mod_draw_states_flag08();
        }
        break;
    case NPAct_PlyrWeapModsSync:
        if (!in_network_game)
        {
            agents_copy_wepmod_netplayer_to_player(plyr, p_netplyr);
            agents_copy_wepmod_netplayer_to_cryo(p_netplyr);
        }
        else if ((net_host_player_no == plyr) && ((net_game_play_flags & NGPF_Unkn08) != 0))
        {
            for (i = 0; i < PLAYERS_LIMIT; i++)
            {
                if (unkn2_names[i][0] == '\0')
                    continue;
                agents_copy_wepmod_netplayer_to_player(i, p_netplyr);
            }
            agents_copy_wepmod_netplayer_to_cryo(p_netplyr);
            if (net_host_player_no != netplyr)
            {
                update_flic_mods(flic_mods);
                set_mod_draw_states_flag08();
            }
        }
        else if ((net_game_play_flags & NGPF_Unkn08) == 0)
        {
            agents_copy_wepmod_netplayer_to_player(plyr, p_netplyr);
        }
        break;
    case NPAct_PlyrFourPackSync:
        if (!in_network_game)
        {
            agents_copy_fourpacks_netplayer_to_player(plyr, p_netplyr);
            agents_copy_fourpacks_netplayer_to_cryo(p_netplyr);
        }
        else if ((net_host_player_no == plyr) && ((net_game_play_flags & NGPF_Unkn08) != 0))
        {
            for (i = 0; i < PLAYERS_LIMIT; i++)
            {
                if (unkn2_names[i][0] == '\0')
                    continue;
                agents_copy_fourpacks_netplayer_to_player(i, p_netplyr);
            }
            agents_copy_fourpacks_netplayer_to_cryo(p_netplyr);
        }
        else if ((net_game_play_flags & NGPF_Unkn08) == 0)
        {
            agents_copy_fourpacks_netplayer_to_player(plyr, p_netplyr);
        }

        players[plyr].MissionAgents = p_netplyr->U.FourPacks.MissionAgents;
        if (players[plyr].MissionAgents != 0x0F)
        {
            if (in_network_game) {
                LOGWARN("Partial team in network game, mask 0x%x; switching to full",
                  (uint)players[plyr].MissionAgents);
                player_mission_agents_reset(plyr);
            }
            if ((players[plyr].MissionAgents & 0x0F) == 0) {
                LOGWARN("Cannot start a game with empty team, switching to full");
                player_mission_agents_reset(plyr);
            }
        }
        break;
    case NPAct_GrPaintDrawPt:
        net_grpaint_draw_op(
          p_netplyr->U.Progress.GrPaintX,
          p_netplyr->U.Progress.GrPaintY,
          p_netplyr->U.Progress.GrPaintColour,
          0, plyr);
        break;
    case NPAct_GrPaintPt1Upd:
        net_grpaint_draw_op(
          p_netplyr->U.Progress.GrPaintX,
          p_netplyr->U.Progress.GrPaintY,
          p_netplyr->U.Progress.GrPaintColour,
          2, plyr);
        break;
    default:
        break;
    }
}

void net_player_scheduled_action_prepare_packet(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    net_player_action_prepare(plyr);
}

void packet_file_net_player_write(void)
{
    struct NetworkPlayer *p_netplyr;
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;
    p_netplyr = &network_players[plyr];

    PacketRecord_WriteNP(p_netplyr);
}

TbResult packet_file_net_player_read(void)
{
    struct NetworkPlayer *p_netplyr;
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;
    p_netplyr = &network_players[plyr];

    return PacketRecord_ReadNP(p_netplyr);
}

void packet_write_whole_player_init(void)
{
    net_schedule_player_cryo_equip_sync();
    net_player_scheduled_action_prepare_packet();
    packet_file_net_player_write();

    net_schedule_player_equip_fourpack_sync();
    net_player_scheduled_action_prepare_packet();
    packet_file_net_player_write();

    net_schedule_player_faction_change_sync();
    net_player_scheduled_action_prepare_packet();
    packet_file_net_player_write();

    net_schedule_player_random_init_sync();
    net_player_scheduled_action_prepare_packet();
    packet_file_net_player_write();
}

void packet_read_whole_player_init(void)
{
    int plyr;

    if (LbNetworkSessionActive())
        plyr = LbNetworkPlayerNumber();
    else
        plyr = local_player_no;

    while (1)
    {
        if (packet_file_net_player_read() != Lb_SUCCESS) {
            LOGWARN("Cannot read more packets");
            break;
        }
        net_player_action_execute(plyr, plyr);
        if (net_player_action_is(plyr, NPAct_MissionInit))
            break;
    }
}

void net_unkn_func_33(void)
{
    int player;
    int i;

    player = LbNetworkPlayerNumber();
    net_players_num = LbNetworkSessionNumberPlayers();
    net_host_player_no = LbNetworkHostPlayerNumber();

    net_player_action_prepare(player);

    net_players_immediate_exchange();

    for (i = 0; i < 8; i++)
    {
        net_player_action_execute(i, player);
    }

    if (byte_1C6D4A)
    {
        if (net_player_packet_has_progress_data(net_host_player_no))
        {
            net_player_update_from_progress_packet_hostonly();
        }
        byte_1C6D4A = 0;
    }

    net_players_all_set_no_action();
}

void net_players_copy_equip_and_cryo(void)
{
    net_schedule_player_cryo_equip_sync();
    net_unkn_func_33();
    net_schedule_player_equip_fourpack_sync();
    gameturn++;
}

void net_players_copy_equip_and_cryo_now(void)
{
    net_schedule_player_cryo_equip_sync();
    net_unkn_func_33();
    net_schedule_player_equip_fourpack_sync();
    net_unkn_func_33();
}

/******************************************************************************/
