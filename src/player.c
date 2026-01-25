/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file player.c
 *     Player state and information handling.
 * @par Purpose:
 *     Implement functions for getting and updating player state.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player.h"

#include <assert.h>
#include "bfutility.h"
#include "ssampply.h"

#include "bigmap.h"
#include "display.h"
#include "game.h"
#include "game_options.h"
#include "guitext.h"
#include "hud_panel.h"
#include "people.h"
#include "pepgroup.h"
#include "sound.h"
#include "thing.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/
ubyte default_agent_tiles_x[8] = {
    45, 64, 40, 60, 30, 70, 20, 80,
};
ubyte default_agent_tiles_z[8] = {
    46, 44, 60, 60, 30, 30, 70, 70,
};

ushort netgame_agent_pos_x[PLAYERS_LIMIT][4];
ushort netgame_agent_pos_z[PLAYERS_LIMIT][4];

ubyte playable_agents;
/******************************************************************************/

void player_mission_agents_reset(PlayerIdx plyr)
{
    PlayerInfo *p_plyr;

    p_plyr = &players[plyr];
    p_plyr->MissionAgents = 0x0F;
}

void player_update_from_cryo_agent(ushort cryo_no, PlayerInfo *p_player, ushort plagent)
{
    ushort wepfp;

    p_player->Weapons[plagent] = cryo_agents.Weapons[cryo_no];
    p_player->Mods[cryo_no].Mods = cryo_agents.Mods[cryo_no].Mods;

    for (wepfp = 0; wepfp < WFRPK_COUNT; wepfp++) {
        p_player->FourPacks[wepfp][plagent] = \
          cryo_agents.FourPacks[cryo_no].Amount[wepfp];
    }
}

void cryo_update_from_player_agent(ushort cryo_no, PlayerInfo *p_player, ushort plagent)
{
    struct Thing *p_agent;
    ushort wepfp;

    p_agent = p_player->MyAgent[plagent];
    if (p_agent->Type != TT_PERSON)
        return;

    cryo_agents.Weapons[cryo_no] = p_agent->U.UPerson.WeaponsCarried & ~(1 << (WEP_ENERGYSHLD-1));
    cryo_agents.Mods[cryo_no].Mods = p_agent->U.UPerson.UMod.Mods;

    for (wepfp = 0; wepfp < WFRPK_COUNT; wepfp++) {
        cryo_agents.FourPacks[cryo_no].Amount[wepfp] = \
          p_player->FourPacks[wepfp][plagent];
    }
}

void player_update_agents_from_cryo(PlayerInfo *p_player)
{
    ushort plagent;

    for (plagent = 0; plagent < 4; plagent++) {
        ushort cryo_no = plagent;
        player_update_from_cryo_agent(cryo_no, p_player, plagent);
    }
}

void cryo_update_agents_from_player(PlayerInfo *p_player)
{
    ushort plagent, nremoved;

    nremoved = 0;
    for (plagent = 0; plagent < playable_agents; plagent++)
    {
        struct Thing *p_agent;

        p_agent = p_player->MyAgent[plagent];
        if (p_agent->Type != TT_PERSON)
            continue;
        if ((p_agent->Flag & TngF_Destroyed) != 0) {
            remove_agent(plagent);
            ++nremoved;
            continue;
        }
        if ((p_agent->SubType == SubTT_PERS_AGENT) || (p_agent->SubType == SubTT_PERS_ZEALOT))
        {
            ushort cryo_no;
            // Removing an agent from cryo shifted all further agents down; account for that
            cryo_no = plagent - nremoved;
            cryo_update_from_player_agent(cryo_no, p_player, plagent);
        }
    }
}

void player_agents_add_random_epidermises(PlayerInfo *p_player)
{
    ushort plagent;
    struct Thing *p_person;

    for (plagent = 0; plagent < playable_agents; plagent++)
    {
        p_person = p_player->MyAgent[plagent];
        if (p_person == NULL)
            continue;
        if (person_mod_skin_level(p_person) == 0)
        {
            ushort rnd;
            rnd = LbRandomAnyShort();
            set_person_mod_skin_level(p_person, 1 + (rnd & 3));
        }
    }
}

void players_sync_from_cryo(void)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    player_update_agents_from_cryo(p_locplayer);
}

void player_agents_init_prev_weapon(PlayerIdx plyr)
{
    PlayerInfo *p_locplayer;
    ushort plagent;

    p_locplayer = &players[local_player_no];
    for (plagent = 0; plagent < playable_agents; plagent++)
    {
        struct Thing *p_agent;
        WeaponType wtype;

        p_agent = p_locplayer->MyAgent[plagent];
        if (p_agent->Type == TT_PERSON)
            wtype = find_nth_weapon_held(p_agent->ThingOffset, 1);
        else
            wtype = WEP_NULL;
        p_locplayer->PrevWeapon[plagent] = wtype;
    }
    for (; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
    {
        p_locplayer->PrevWeapon[plagent] = WEP_NULL;
    }
}

void player_agent_update_prev_weapon(struct Thing *p_agent)
{
    PlayerInfo *p_player;
    PlayerIdx plyr;
    ushort plagent;

    plyr = p_agent->U.UPerson.ComCur >> 2;
    plagent = p_agent->U.UPerson.ComCur & 3;
    p_player = &players[plyr];

    if (p_agent->U.UPerson.CurrentWeapon != 0)
        p_player->PrevWeapon[plagent] = p_agent->U.UPerson.CurrentWeapon;
    else
        p_player->PrevWeapon[plagent] = find_nth_weapon_held(p_agent->ThingOffset, 1);
}

short player_agent_current_or_prev_weapon(PlayerIdx plyr, ushort plagent)
{
    PlayerInfo *p_player;
    struct Thing *p_agent;
    short curwep;

    p_player = &players[plyr];
    p_agent = p_player->MyAgent[plagent];
    if (p_agent->Type != TT_PERSON)
        return WEP_NULL;
    if (plagent != (p_agent->U.UPerson.ComCur & 3)) {
        LOGERR("Player %d agent (thing %d) claims it has slot %d while in fact it fills %d",
          (int)plyr, (int)p_agent->ThingOffset, (int)(p_agent->U.UPerson.ComCur & 3), (int)plagent);
        return WEP_NULL;
    }

    curwep = p_agent->U.UPerson.CurrentWeapon;
    if (curwep == WEP_NULL) {
        curwep = p_player->PrevWeapon[plagent];
    }
    return curwep;
}

short player_agent_weapon_delay(PlayerIdx plyr, ushort plagent, WeaponType wtype)
{
    PlayerInfo *p_player;

    p_player = &players[plyr];
    return p_player->WepDelays[plagent][wtype];
}

void player_agent_set_weapon_delay(PlayerIdx plyr, ushort plagent, WeaponType wtype, short delay_turns)
{
    PlayerInfo *p_player;

    if (delay_turns < 0)
        delay_turns = 0;
    else if (delay_turns > 255)
        delay_turns = 255;

    p_player = &players[plyr];
    p_player->WepDelays[plagent][wtype] = delay_turns;
}

void player_agent_clear_weapon_delays(PlayerIdx plyr, ushort plagent)
{
    PlayerInfo *p_player;
    WeaponType wtype;

    p_player = &players[plyr];
    for (wtype = WEP_NULL; wtype < WEP_TYPES_COUNT; wtype++)
    {
        p_player->WepDelays[plagent][wtype] = 0;
    }
}

void player_agents_clear_weapon_delays(PlayerIdx plyr)
{
    ushort plagent;

    for (plagent = 0; plagent < playable_agents; plagent++)
    {
        player_agent_clear_weapon_delays(plyr, plagent);
    }
}

TbBool player_agent_has_weapon(PlayerIdx plyr, ushort plagent, WeaponType wtype)
{
    PlayerInfo *p_player;

    p_player = &players[plyr];
    return weapons_has_weapon(p_player->Weapons[plagent], wtype);
}

TbBool player_agent_is_alive(PlayerIdx plyr, ushort plagent)
{
    PlayerInfo *p_player;
    struct Thing *p_agent;

    p_player = &players[plyr];
    p_agent = p_player->MyAgent[plagent];
    if (p_agent->Type != TT_PERSON)
        return false;

    return ((p_agent->Flag & TngF_Destroyed) == 0);
}

TbBool player_agent_is_executing_commands(PlayerIdx plyr, ushort plagent)
{
    PlayerInfo *p_player;
    struct Thing *p_agent;

    p_player = &players[plyr];
    p_agent = p_player->MyAgent[plagent];
    if (p_agent->Type != TT_PERSON)
        return false;

    return person_is_executing_commands(p_agent->ThingOffset);
}

TbBool free_slot(ushort plagent, WeaponType wtype)
{
#if 0
    TbBool ret;
    asm volatile ("call ASM_free_slot\n"
        : "=r" (ret) : "a" (plagent & 0xff), "d" (wtype-1));
    return ret;
#endif
    PlayerInfo *p_locplyr;
    TbBool has_free;
    ushort fp;

    p_locplyr = &players[local_player_no];

    fp = weapon_fourpack_index(wtype);
    if ((fp < WFRPK_COUNT) && weapons_has_weapon(p_locplyr->Weapons[plagent], wtype))
    {
        has_free = cryo_agents.FourPacks[plagent].Amount[fp] < WEAPONS_FOURPACK_MAX_COUNT;
    }
    else
    {
        ushort used_slots;
        used_slots = weapons_count_used_slots(p_locplyr->Weapons[plagent]);
        has_free = used_slots < WEAPONS_CARRIED_MAX_COUNT;
    }
    return has_free;
}

TbBool player_cryo_add_weapon_one(ushort cryo_no, WeaponType wtype)
{
    TbBool added;

    added = weapons_add_one(&cryo_agents.Weapons[cryo_no], &cryo_agents.FourPacks[cryo_no], wtype);
    if (!added)
        return false;

    if (cryo_no < 4) {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
#if 0
        // TODO re-enable when player->FourPacks is unified and in the same format as cryo_agents.FourPacks
        weapons_add_one(&p_locplayer->Weapons[cryo_no], &p_player->FourPacks[cryo_no], weapon);
#else
        // Copying all weapons will work as well
        player_update_from_cryo_agent(cryo_no, p_locplayer, cryo_no);
#endif
    }
    return added;
}

TbBool player_cryo_add_cybmod(ushort cryo_no, ubyte cybmod)
{
    if (!check_mod_allowed_to_flags(&cryo_agents.Mods[cryo_no], cybmod))
        return false;

    add_mod_to_flags(&cryo_agents.Mods[cryo_no], cybmod);

    if (cryo_no < 4) {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];

        add_mod_to_flags(&p_locplayer->Mods[cryo_no], cybmod);
    }

    return true;
}

TbBool player_cryo_remove_weapon_one(ushort cryo_no, WeaponType wtype)
{
    TbBool rmved;

    rmved = weapons_remove_one(&cryo_agents.Weapons[cryo_no], &cryo_agents.FourPacks[cryo_no], wtype);
    if (!rmved)
        return false;

    if (cryo_no < 4) {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
#if 0
        // TODO re-enable when player->FourPacks is unified and in the same format as cryo_agents.FourPacks
        weapons_remove_one(&p_locplayer->Weapons[cryo_no], &p_player->FourPacks[cryo_no], weapon);
#else
        // Copying all weapons will work as well
        player_update_from_cryo_agent(cryo_no, p_locplayer, cryo_no);
#endif
    }
    return rmved;
}

TbBool player_cryo_transfer_weapon_between_agents(ushort from_cryo_no,
  ushort to_cryo_no, ubyte weapon)
{
    TbBool added;

    added = player_cryo_add_weapon_one(to_cryo_no, weapon);
    if (!added)
        return false;
    player_cryo_remove_weapon_one(from_cryo_no, weapon);
    return added;
}

const char *get_cryo_agent_name(ushort cryo_no)
{
    ushort rndname;

    if (selected_agent < 0)
        return gui_strings[536];

    rndname = cryo_agents.RandomName[cryo_no];
    if (background_type == 1)
    {
        if (cryo_agents.Sex & (1 << selected_agent))
            return gui_strings[227 + rndname];
        else
            return gui_strings[177 + rndname];
    }
    else
    {
        return gui_strings[77 + rndname];
    }

}

void remove_agent(ubyte cryo_no)
{
    asm volatile ("call ASM_remove_agent\n"
        : : "a" (cryo_no));
}

void add_agent(ulong weapons, ushort mods)
{
    asm volatile ("call ASM_add_agent\n"
        : : "a" (weapons), "d" (mods));
}

ThingIdx direct_control_thing_for_player(PlayerIdx plyr)
{
    PlayerInfo *p_player;
    ThingIdx dcthing;

    p_player = &players[plyr];
    if (p_player->DoubleMode)
        dcthing = p_player->DirectControl[byte_153198-1];
    else
        dcthing = p_player->DirectControl[0];
    return dcthing;
}

void set_default_player_control(void)
{
    PlayerInfo *p_locplayer;
    short i;

    p_locplayer = &players[local_player_no];
    p_locplayer->DoubleMode = 0;
    for (i = 0; i < 4; i++)
        p_locplayer->UserInput[i].ControlMode = UInpCtr_Mouse;
}

void player_target_clear(PlayerIdx plyr)
{
    PlayerInfo *p_player;

    p_player = &players[plyr];
    p_player->Target = 0;
    p_player->TargetType = TrgTp_NONE;
}

void kill_my_players(PlayerIdx plyr)
{
    asm volatile ("call ASM_kill_my_players\n"
        : : "a" (plyr));
}

TbBool player_can_toggle_thermal(PlayerIdx plyr)
{
    PlayerInfo *p_player;
    ThingIdx dcthing;

    // Cannot enable in network mode - this would lead to desync
    if (in_network_game)
        return false;
    // Only local player can enable thermal - state is stored outside of players array
    if (plyr != local_player_no)
        return false;

    p_player = &players[plyr];
    dcthing = p_player->DirectControl[mouser];
    if (!person_can_sustain_thermal(dcthing))
        return false;

    return true;
}

void player_toggle_thermal(PlayerIdx plyr)
{
    //TODO thermal view state should be stored in player - now it cannot be used in net games
    if ((ingame.Flags & GamF_ThermalView) == 0) {
        ingame.Flags |= GamF_ThermalView;
    } else {
        ingame.Flags &= ~GamF_ThermalView;
    }
    if ((ingame.Flags & GamF_ThermalView) != 0) {
        play_sample_using_heap(0, 35, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
    }
    // Make zero change to brightness - will reload palette and apply momentary brightness
    change_brightness(0);
}

void player_update_thermal(PlayerIdx plyr)
{
    PlayerInfo *p_player;
    ThingIdx dcthing;

    if (plyr != local_player_no) {
        return;
    }
    if ((ingame.Flags & GamF_ThermalView) == 0) {
        return;
    }

    p_player = &players[plyr];
    dcthing = p_player->DirectControl[mouser];

    if (!person_update_thermal(dcthing))
    {
        ingame.Flags &= ~GamF_ThermalView;
        change_brightness(0);
    }
}

/** Figure out how many agents are attending the mission.
 */
ushort player_agents_place_in_mission_count(PlayerIdx plyr)
{
    PlayerInfo *p_player;
    ushort nagents;

    p_player = &players[plyr];
    if (p_player->DoubleMode)
        nagents = p_player->DoubleMode + 1;
    else
        nagents = 4;

    if (p_player->MissionAgents < (1 << nagents) - 1)
    {
        switch (p_player->MissionAgents)
        {
        case 1:
            nagents = 1;
            break;
        case 3:
            nagents = 2;
            break;
        case 7:
            nagents = 3;
            break;
        }
    }
    return nagents;
}

int place_default_player(PlayerIdx plyr, TbBool replace)
{
#if 0
    int ret;
    asm volatile ("call ASM_place_default_player\n"
        : "=r" (ret) : "a" (plyr), "d" (replace));
    return ret;
#endif
    PlayerInfo *p_player;
    int cor_x, cor_z;
    ushort nagents, plagent;
    short new_type;
    ubyte net_plyr_id;

    nagents = player_agents_place_in_mission_count(plyr);
    if (in_network_game)
        nagents = 4;

    cor_x = TILE_TO_MAPCOORD(default_agent_tiles_x[plyr], 0);
    cor_z = TILE_TO_MAPCOORD(default_agent_tiles_z[plyr], 0);

    net_plyr_id = plyr;
    if (in_network_game)
    {
        if ((net_game_play_flags & NGPF_Unkn20) != 0)
            net_plyr_id = LbRandomAnyShort() & 7;
    }

    p_player = &players[plyr];
    if (in_network_game)
        new_type = group_types[plyr];
    else
        new_type = -1;

    for (plagent = 0; plagent < nagents; plagent++)
    {
        struct Thing *p_pv_agent;
        struct Thing *p_agent;

        if (in_network_game) {
            cor_x = netgame_agent_pos_x[net_plyr_id][plagent];
            cor_z = netgame_agent_pos_z[net_plyr_id][plagent];
        }
        p_pv_agent = p_player->MyAgent[plagent];
        if ((p_pv_agent == NULL) || ((p_pv_agent->Flag & TngF_Destroyed) != 0) || (replace))
        {
            ThingIdx pv_agent;

            if (p_pv_agent != NULL)
                pv_agent = p_pv_agent->ThingOffset;
            else
                pv_agent = 0;
            p_agent = replace_thing_given_thing_idx(cor_x, 0, cor_z, 1, pv_agent);
            reset_default_player_agent(plyr, plagent, p_agent, new_type);
        }
        cor_x += 256;
    }
    // Fill the rest of agents array, to avoid using leftovers
    for (; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
        p_player->MyAgent[plagent] = &things[0];

    playable_agents = AGENTS_SQUAD_MAX_COUNT;
    player_agents_init_prev_weapon(plyr);
    player_agents_clear_weapon_delays(plyr);
    return 0;
}

void place_single_player(void)
{
    ulong n;
    ushort nagents, pl_agents, pl_group;

    nagents = player_agents_place_in_mission_count(local_player_no);

    pl_group = level_def.PlayableGroups[0];
    pl_agents = make_group_into_players(pl_group, local_player_no, nagents, -1);
    if (pl_agents == 0) {
        struct Thing *p_person;
        LOGERR("Player %d group %d playable agents not found amongst %d things",
          (int)local_player_no, (int)pl_group, (int)things_used_head);
        p_person = new_sim_person(513, 1, 513, SubTT_PERS_AGENT);
        p_person->U.UPerson.Group = pl_group;
        p_person->U.UPerson.EffectiveGroup = pl_group;
        pl_agents = make_group_into_players(pl_group, local_player_no, 1, -1);
    } else {
        LOGSYNC("Player %d group %d playable agents found %d expected %d",
          (int)local_player_no, (int)pl_group, (int)pl_agents, (int)nagents);
    }

    n = things_used_head;
    while (n != 0)
    {
        struct Thing *p_thing;

        p_thing = &things[n];
        n = p_thing->LinkChild;
        if ((p_thing->U.UPerson.Group == pl_group) && (p_thing->Type == TT_PERSON)
          && !(p_thing->Flag & TngF_PlayerAgent))
        {
            remove_thing(p_thing->ThingOffset);
            delete_node(p_thing);
        }
    }
    playable_agents = pl_agents;
    if (pl_agents == 0)
      place_default_player(0, 1);
}

/******************************************************************************/
