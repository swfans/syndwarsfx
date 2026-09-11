/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file misstat.h
 *     Header file for misstat.c.
 * @par Purpose:
 *     Mission stats gathering and maintaining.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Oct 2024 - 23 Dec 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef MISSTAT_H
#define MISSTAT_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#pragma pack(1)

#define MISSION_STATUS_LIMIT 120

struct Thing;

struct MissionStatus { // sizeof=40
    ulong CityDays;
    ulong CityHours;
    ulong Days;
    ulong Hours;
    union {
      struct {
      ushort CivsKilled;
      ushort CivsPersuaded;
      ushort SecurityKilled;
      ushort SecurityPersuaded;
      ushort EnemiesKilled;
      ushort EnemiesPersuaded;
      ulong CashAtStart;
      } SP;
      struct {
      /** How many agents of a player were killed by each other player. */
      ushort AgentsKilled[8];
      } MP;
    };
    ulong Expenditure;
    ubyte HitAccuracy;
    ubyte ObjectivesComplete;
    ubyte AgentsLost;
    ubyte AgentsGained;
};

#pragma pack()
/******************************************************************************/
extern struct MissionStatus mission_status[MISSION_STATUS_LIMIT];

long time_difference(struct SynTime *tm1, struct SynTime *tm2);

/** Increase timestamp stored in given SynTime by given amount of days.
 */
void syntime_inc_days(struct SynTime *tm, ushort ndays);

/** Increase timestamp stored in given SynTime by given amount of hours.
 */
void syntime_inc_hours(struct SynTime *tm, ushort nhours);

void clear_mission_status_all(void);
void clear_mission_status(ushort brief);
void clear_open_mission_status(void);

void mission_status_time_rand_progress(ushort brief);

void killed_person_add_to_stats(struct Thing *p_person, ushort brief);
void persuaded_person_add_to_stats(struct Thing *p_person, ushort brief);
void persuaded_person_remove_from_stats(struct Thing *p_person, ushort brief);
void killed_mp_agent_add_to_stats(struct Thing *p_victim, PlayerIdx attack_plyr);

void stats_mp_add_person_kills_person(ThingIdx killer, ThingIdx victim);

int stats_mp_count_players_agents_killed(PlayerIdx plyr);
int stats_mp_count_net_players_agents_kills(PlayerIdx plyr);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
