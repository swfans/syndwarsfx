/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file wrcities.h
 *     Header file for wrcities.c.
 * @par Purpose:
 *     Routines for handling a list of cities for world map.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 11 Sep 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef WRCITIES_H
#define WRCITIES_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum CityFlags {
    CitF_Unkn01     = 0x01,
    CitF_Unkn02     = 0x02,
    CitF_Unkn04     = 0x04,
    CitF_Unkn08     = 0x08,
    CitF_Unkn10     = 0x10,
    CitF_Unkn20     = 0x20,
    CitF_Unkn40     = 0x40,
    CitF_Unkn80     = 0x80,
};

struct City { // sizeof=40
    /** Coordinate of the city on mission brief map. */
    short X;
    /** Coordinate of the city on mission brief map. */
    short Y;
    /** Map assigned to this city by last active mission. */
    ubyte MapID;
    /** Level assigned to this city by last active mission. */
    ubyte Level;
    ubyte Flags;
    /** Amount of additional infos in brief screen (both unlocked by default
     * and by cash).
     */
    ubyte Info;
    /** National text IDs: city name, then 5 additional properties. */
    ushort TextIndex[6];
    /** Map numbers which can be assigned to this city by active mission. */
    ubyte MapsNo[3];
    /** Total cash reward expected from the active mission in this city.
     * Multiply x100 to get the value in credits.
     */
    ushort CreditReward;
    ubyte Dummy2[15];
};

#pragma pack()
/******************************************************************************/
extern ubyte num_cities;
extern struct City cities[];
extern sbyte selected_city_id;

void save_cities_conf_file(void);
void read_cities_conf_file(void);
void load_city_txt(void);
void load_city_data(ubyte type);

void activate_cities(ubyte brief);
void recount_city_credit_reward(ubyte city);
void clear_city_netscan(ubyte city);

/** Searches for mission taking place in given city, within mission chain in the brief.
 */
ushort find_mission_for_city_in_brief(short brief, sbyte city);

sbyte find_closest_city(ushort x, ushort y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
