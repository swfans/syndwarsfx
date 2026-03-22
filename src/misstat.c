/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file misstat.c
 *     Mission stats gathering and maintaining.
 * @par Purpose:
 *     Functions for handling mission statistics during the game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Oct 2024 - 23 Dec 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "misstat.h"

#include "bfmemut.h"

#include "febrief.h"
#include "game.h"
#include "people.h"
#include "player.h"
#include "swlog.h"
#include "thing.h"
/******************************************************************************/

void clear_mission_status_all(void)
{
    LbMemorySet(mission_status, 0, MISSION_STATUS_LIMIT * sizeof(struct MissionStatus));
}

void clear_mission_status(ushort brief)
{
    struct MissionStatus *p_mistat;

    p_mistat = &mission_status[brief];

    p_mistat->SP.CivsKilled = 0;
    p_mistat->SP.EnemiesKilled = 0;
    p_mistat->SP.CivsPersuaded = 0;
    p_mistat->SP.SecurityPersuaded = 0;
    p_mistat->SP.EnemiesPersuaded = 0;
    p_mistat->SP.SecurityKilled = 0;
    p_mistat->AgentsGained = 0;
    p_mistat->AgentsLost = 0;
    p_mistat->CityDays = 0;
    p_mistat->CityHours = 0;
}

void clear_open_mission_status(void)
{
    ulong id;

    if (in_network_game)
    {
        // In network game, mission status is per-player rather than per-mission
        for (id = 0; id < PLAYERS_LIMIT; id++)
        {
            clear_mission_status(id);
        }
    }
    else if (!in_network_game)
    {
        // Each mission has its status (unless in network game)
        clear_mission_status(open_brief);
    }
}

void persuaded_person_add_to_stats(struct Thing *p_person, ushort brief)
{
    struct MissionStatus *p_mistat;

    p_mistat = &mission_status[brief];

    if (person_type_is_synd_agent(p_person->SubType))
        p_mistat->AgentsGained++;

    if (person_type_is_any_major_faction(p_person->SubType))
        p_mistat->SP.EnemiesPersuaded++;
    else if (person_type_is_wide_definition_civilian(p_person->SubType))
        p_mistat->SP.CivsPersuaded++;
    else if (person_type_is_security(p_person->SubType))
        p_mistat->SP.SecurityPersuaded++;
}

void killed_person_add_to_stats(struct Thing *p_person, ushort brief)
{
    struct MissionStatus *p_mistat;

    p_mistat = &mission_status[brief];

#if 0 // only in MP
    if (person_type_is_synd_agent(p_person->SubType))
        p_mistat->AgentsKilled++;
#endif

    if (person_type_is_any_major_faction(p_person->SubType))
        p_mistat->SP.EnemiesKilled++;
    else if (person_type_is_wide_definition_civilian(p_person->SubType))
        p_mistat->SP.CivsKilled++;
    else if (person_type_is_security(p_person->SubType))
        p_mistat->SP.SecurityKilled++;
}

void killed_mp_agent_add_to_stats(struct Thing *p_victim, PlayerIdx attack_plyr)
{
    struct MissionStatus *p_mistat;
    PlayerIdx victim_plyr;

    if (attack_plyr >= PLAYERS_LIMIT) {
        LOGERR("Attacking player %d out of range", (int)attack_plyr);
        return;
    }
    victim_plyr = p_victim->U.UPerson.ComCur >> 2;
    if (victim_plyr >= PLAYERS_LIMIT) {
        LOGERR("Victim player %d out of range", (int)victim_plyr);
        return;
    }
    p_mistat = &mission_status[attack_plyr];

    p_mistat->MP.AgentsKilled[victim_plyr]++;
}

void persuaded_person_remove_from_stats(struct Thing *p_person, ushort brief)
{
    struct MissionStatus *p_mistat;

    p_mistat = &mission_status[brief];

    if (person_type_is_synd_agent(p_person->SubType))
        p_mistat->AgentsGained--;

    if (person_type_is_any_major_faction(p_person->SubType))
        p_mistat->SP.EnemiesPersuaded--;
    else if (person_type_is_wide_definition_civilian(p_person->SubType))
        p_mistat->SP.CivsPersuaded--;
    else if (person_type_is_security(p_person->SubType))
        p_mistat->SP.SecurityPersuaded--;
}

int stats_mp_count_players_agents_killed(PlayerIdx plyr)
{
    struct MissionStatus *p_mistat;
    int n;
    PlayerIdx k;

    n = 0;
    p_mistat = &mission_status[plyr];
    for (k = 0; k < PLAYERS_LIMIT; k++)
    {
        if (unkn2_names[k][0] == '\0')
            continue;

        n += p_mistat->MP.AgentsKilled[k];
    }
    return n;
}

int stats_mp_count_net_players_agents_kills(PlayerIdx plyr)
{
    struct MissionStatus *p_mistat;
    int n;
    PlayerIdx k;

    n = 0;
    p_mistat = &mission_status[plyr];
    for (k = 0; k < PLAYERS_LIMIT; k++)
    {
        if (unkn2_names[k][0] == '\0')
            continue;

        if (k == plyr)
            n -= p_mistat->MP.AgentsKilled[k];
        else
            n += p_mistat->MP.AgentsKilled[k];
    }
    return n;
}

void stats_mp_add_player_kills_player_agent(PlayerIdx plyr_kil, PlayerIdx plyr_vic)
{
    struct MissionStatus *p_mistat;
    p_mistat = &mission_status[plyr_kil];
    p_mistat->MP.AgentsKilled[plyr_vic]++;
}

void stats_mp_add_person_kills_person(ThingIdx killer, ThingIdx victim)
{
    struct Thing *p_killer;
    struct Thing *p_victim;

    if ((killer == 0) || (victim == 0))
        return;

    p_killer = &things[killer];
    p_victim = &things[victim];

    if (((p_victim->Flag & TngF_PlayerAgent) != 0) &&
      ((p_killer->Flag & TngF_PlayerAgent) != 0))
    {
        PlayerIdx plyr_kil, plyr_vic;

        plyr_vic = (p_victim->U.UPerson.ComCur & 0x1C) >> 2;
        plyr_kil = (p_killer->U.UPerson.ComCur & 0x1C) >> 2;
        stats_mp_add_player_kills_player_agent(plyr_kil, plyr_vic);
    }
}
/******************************************************************************/
