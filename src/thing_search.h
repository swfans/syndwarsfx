/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_search.h
 *     Header file for thing_search.c.
 * @par Purpose:
 *     Support for finding a matching Thing instance.
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
#ifndef THING_SEARCH_H
#define THING_SEARCH_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;

typedef struct {
    short Arg1;
    short Arg2;
    short Arg3;
    short Arg4;
    long Arg5;
} ThingFilterParams;

/** Definition of a simple callback type which can only return true/false and has no memory
 * of previous checks.
 * As the first thing which returns true is accepted, searching with this filter is faster
 * than the complex ones which need all things to be checked.
 */
typedef TbBool (*ThingBoolFilter)(ThingIdx thing, ThingFilterParams *params);

/** Definition of a minimizing callback type which returns a number to be compared by the
 * search function and return the thing which gives smallest.
 */
typedef s32 (*ThingMinFilter)(ThingIdx thing, short X, short Z, ThingFilterParams *params);

#pragma pack()
/******************************************************************************/

/** Unified function to find a thing of given type within given circle and matching bool filter.
 *
 * Tries to use mapwho and same type list, and if cannot then just searches all used things.
 *
 * @param X Map coordinate in map units.
 * @param Z Map coordinate in map units.
 * @param R Circle radius in map units.
 * @param ttype Thing Type; to catch all, use -1 (but that will be slower).
 * @param subtype Thing SubType; to catch all, use -1.
 * @param filter Filter callback function.
 * @param param Parameters for filter callback function.
 */
ThingIdx find_thing_type_within_circle_with_bfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingBoolFilter filter, ThingFilterParams *params);

/** Unified function to find a thing of given type within given circle and matching
 * minimizing filter.
 *
 * Tries to use mapwho and same type list, and if cannot then just searches all used things.
 *
 * @param X Map coordinate in map units.
 * @param Z Map coordinate in map units.
 * @param R Circle radius in map units.
 * @param ttype Thing Type; to catch all, use -1 (but that will be slower).
 * @param subtype Thing SubType; to catch all, use -1.
 * @param filter Filter callback function.
 * @param param Parameters for filter callback function.
 */
ThingIdx find_thing_type_within_circle_with_mfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params);

ThingIdx search_things_for_type_farthest_from_xz(short X, short Z, short ttype, short subtype);

ThingIdx find_dropped_weapon_within_circle(short X, short Z, ushort R, short weapon);
ThingIdx find_person_carrying_weapon_within_circle(short X, short Z, ushort R, short weapon);
ThingIdx find_person_carrying_weapon(short weapon);

ThingIdx find_person_which_can_be_persuaded_now(short X, short Z, ushort R,
  ThingIdx attacker, ushort target_select, ushort *energy_reqd);

ThingIdx find_nearest_from_group(struct Thing *p_person, ushort group, ubyte no_persuaded);
ThingIdx search_things_for_index(short index);
ThingIdx find_nearest_object2(short mx, short mz, ushort sub_type);
short search_object_for_qface(ushort object, ubyte gflag, ubyte flag, ushort after);
/** Search for train station building around given coordinates.
 */
ThingIdx search_for_station(short X, short Z);
ThingIdx search_for_ferry(short X, short Y, short Z, ushort R);
ThingIdx search_for_vehicle(short X, short Z);
ThingIdx search_things_for_uniqueid(short index, ubyte flag);
/** Search for any thing, including off-map, nearest within given circle.
 * To be used for editors and debug modes.
 *
 * @param X Map coordinate in map units.
 * @param Z Map coordinate in map units.
 * @param R Circle radius in map units.
 */
ThingIdx search_things_for_any_including_offmap_nearest_within_circle(short X, short Z, ushort R);

struct Thing *check_for_radius_hit_person(int prc_x, int prc_y, int prc_z,
        int radius, struct Thing *p_owner, int flag, int skip);

short find_nearest_person_min(int x, int y, int z,
  int n_dist, int *a_dist, int angle1, u32 group_bits);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
