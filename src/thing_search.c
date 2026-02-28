/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_search.c
 *     Support for finding a matching Thing instance.
 * @par Purpose:
 *     Implement searching through various lists of things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_search.h"

#include "bfutility.h"

#include "bigmap.h"
#include "building.h"
#include "people.h"
#include "pepgroup.h"
#include "swlog.h"
#include "thing.h"
#include "thing_fire.h"
#include "vehicle.h"
#include "weapon.h"

/******************************************************************************/

/** Searches for a thing of given type on the specific mapwho tile.
 * Besides being given a title, the thing must also meet circular range condition.
 */
ThingIdx find_thing_on_mapwho_tile_within_circle_with_bfilter(short tile_x, short tile_z, short X, short Z, ushort R,
  short ttype, short subtype, ThingBoolFilter filter, ThingFilterParams *params)
{
    ThingIdx thing;
    ulong k;

    k = 0;
    thing = get_mapwho_thing_index(tile_x, tile_z);
    while (thing != 0)
    {
        if (thing <= 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            // Per thing code start
            if (p_sthing->Type == ttype) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    // Our search radius could have exceeded expected one a bit
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        if (filter(thing, params))
                            return thing;
                    }
                }
            }
            // Per thing code end
            thing = p_sthing->Next;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            // Per thing code start
            if (p_thing->Type == ttype) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    // Our search radius could have exceeded expected one a bit
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        if (filter(thing, params))
                            return thing;
                    }
                }
            }
            // Per thing code end
            thing = p_thing->Next;
        }
        k++;
        if (k >= STHINGS_LIMIT+THINGS_LIMIT) {
            LOGERR("Infinite loop in mapwho things list");
            break;
        }
    }
    return 0;
}

/** Searches for a thing of given type on the specific mapwho tile.
 * Besides being given a title, the thing must also meet circular range condition.
 */
ThingIdx find_thing_on_mapwho_tile_within_circle_with_mfilter(s32 *p_min_fval, short tile_x, short tile_z, short X, short Z, ushort R,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params)
{
    s32 min_fval, fval;
    ThingIdx min_thing, thing;
    ulong k;

    k = 0;
    min_fval = INT32_MAX;
    min_thing = 0;
    thing = get_mapwho_thing_index(tile_x, tile_z);
    while (thing != 0)
    {
        if (thing <= 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            // Per thing code start
            if (p_sthing->Type == ttype) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    // Our search radius could have exceeded expected one a bit
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
            thing = p_sthing->Next;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            // Per thing code start
            if (p_thing->Type == ttype) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    // Our search radius could have exceeded expected one a bit
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
            thing = p_thing->Next;
        }
        k++;
        if (k >= STHINGS_LIMIT+THINGS_LIMIT) {
            LOGERR("Infinite loop in mapwho things list");
            break;
        }
    }
    *p_min_fval = min_fval;
    return min_thing;
}

/**
 * Searches for thing of given type and subtype around tile under given coords, with bool filter.
 * Uses `spiral` checking of surrounding `mapwho` tiles, up to given number of tiles.
 *
 * @return Gives thing index, or 0 if not found.
 */
static ThingIdx find_thing_type_on_spiral_near_tile_with_bfilter(short X, short Z, ushort R, long spiral_len,
  short ttype, short subtype, ThingBoolFilter filter, ThingFilterParams *params)
{
    short tile_x, tile_z;
    int around;

    tile_x = MAPCOORD_TO_TILE(X);
    tile_z = MAPCOORD_TO_TILE(Z);
    for (around = 0; around < spiral_len; around++)
    {
        struct MapOffset *sstep;
        ThingIdx thing;
        long sX, sZ;

        sstep = &spiral_step[around];
        sX = tile_x + sstep->h;
        sZ = tile_z + sstep->v;
        thing = find_thing_on_mapwho_tile_within_circle_with_bfilter(sX, sZ, X, Z, R,
          ttype, subtype, filter, params);
        if (thing != 0)
            return thing;
    }
    return 0;
}

s32 mfilter_nearest(ThingIdx thing, short X, short Z, ThingFilterParams *params)
{
    short dtX, dtZ;

    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_sthing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_sthing->Z) - Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_thing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_thing->Z) - Z;
    }
    return (dtZ * dtZ + dtX * dtX);
}

s32 mfilter_farthest(ThingIdx thing, short X, short Z, ThingFilterParams *params)
{
    short dtX, dtZ;

    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_sthing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_sthing->Z) - Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        dtX = PRCCOORD_TO_MAPCOORD(p_thing->X) - X;
        dtZ = PRCCOORD_TO_MAPCOORD(p_thing->Z) - Z;
    }
    return (MAP_COORD_WIDTH * MAP_COORD_HEIGHT) - (dtZ * dtZ + dtX * dtX);
}

/**
 * Searches for thing of given type and subtype around tile under given coords, with minimizing
 * filter.
 * Uses `spiral` checking of surrounding `mapwho` tiles, up to given number of tiles.
 *
 * @return Gives thing index, or 0 if not found.
 */
static ThingIdx find_thing_type_on_spiral_near_tile_with_mfilter(short X, short Z, ushort R, long spiral_len,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params)
{
    s32 min_fval;
    ThingIdx min_thing;
    int around;
    short tile_x, tile_z;

    min_fval = INT32_MAX;
    min_thing = 0;
    tile_x = MAPCOORD_TO_TILE(X);
    tile_z = MAPCOORD_TO_TILE(Z);
    for (around = 0; around < spiral_len; around++)
    {
        struct MapOffset *sstep;
        s32 fval;
        ThingIdx thing;
        long sX, sZ;

        sstep = &spiral_step[around];
        sX = tile_x + sstep->h;
        sZ = tile_z + sstep->v;
        thing = find_thing_on_mapwho_tile_within_circle_with_mfilter(&fval, sX, sZ, X, Z, R,
          ttype, subtype, filter, params);
        if (fval < min_fval) {
            min_fval = fval;
            min_thing = thing;
            // If the minimizing factor is distance, then finding any match narrows the spiral
            // length to check (because we expect lower values further away)
            if (filter == mfilter_nearest)
                spiral_len = min(spiral_len, around * 2 + 4); // good enough estimate of where to finish
        }
    }
    return min_thing;
}

static ThingIdx find_thing_type_on_same_type_list_within_circle_with_bfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingBoolFilter filter, ThingFilterParams *params)
{
    ThingIdx thing;
    ulong k;

    k = 0;
    thing = get_thing_same_type_head(ttype, subtype);
    while (thing != 0)
    {
        if (thing <= 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            // Per thing code start
            if (p_sthing->Type == ttype) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
#if 0
                    func_6fd1c(PRCCOORD_TO_MAPCOORD(p_sthing->X), PRCCOORD_TO_MAPCOORD(p_sthing->Y),
                      PRCCOORD_TO_MAPCOORD(p_sthing->Z), X, PRCCOORD_TO_MAPCOORD(p_sthing->Y), Z, colour_lookup[ColLU_RED]);
#endif

                    if (thing_is_within_circle(thing, X, Z, R)) {
                        if (filter(thing, params))
                            return thing;
                    }
                }
            }
            // Per thing code end
            thing = p_sthing->LinkSame;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            // Per thing code start
            if (p_thing->Type == ttype) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
#if 0
                    func_6fd1c(PRCCOORD_TO_MAPCOORD(p_thing->X), PRCCOORD_TO_MAPCOORD(p_thing->Y),
                      PRCCOORD_TO_MAPCOORD(p_thing->Z), X, PRCCOORD_TO_MAPCOORD(p_thing->Y), Z, colour_lookup[ColLU_RED]);
#endif
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        if (filter(thing, params))
                            return thing;
                    }
                }
            }
            // Per thing code end
            thing = p_thing->LinkSame;
        }
        // If searching for all subtypes, make sure we really catch them all; switch to
        // second linked list if first one did not gave results
        if ((thing == 0) && (subtype == -1) && (ttype == TT_VEHICLE)) {
            subtype = SubTT_VEH_SHIP;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        if ((thing == 0) && (subtype == -1) && (ttype == TT_BUILDING)) {
            subtype = SubTT_BLD_MGUN;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        k++;
        if (k >= STHINGS_LIMIT+THINGS_LIMIT) {
            LOGERR("Infinite loop in same type things list");
            break;
        }
    }
    return 0;
}

static ThingIdx find_thing_type_on_same_type_list_within_circle_with_mfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params)
{
    s32 min_fval, fval;
    ThingIdx min_thing, thing;
    ulong k;

    k = 0;
    min_fval = INT32_MAX;
    min_thing = 0;
    thing = get_thing_same_type_head(ttype, subtype);
    while (thing != 0)
    {
        if (thing <= 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            // Per thing code start
            if (p_sthing->Type == ttype) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
            thing = p_sthing->LinkSame;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            // Per thing code start
            if (p_thing->Type == ttype) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
            thing = p_thing->LinkSame;
        }
        // If searching for all subtypes, make sure we really catch them all; switch to
        // second linked list if first one did not gave results
        if ((thing == 0) && (subtype == -1) && (ttype == TT_VEHICLE)) {
            subtype = SubTT_VEH_SHIP;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        if ((thing == 0) && (subtype == -1) && (ttype == TT_BUILDING)) {
            subtype = SubTT_BLD_MGUN;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        k++;
        if (k >= STHINGS_LIMIT+THINGS_LIMIT) {
            LOGERR("Infinite loop in same type things list");
            break;
        }
    }
    return min_thing;
}

static ThingIdx find_thing_type_on_same_type_list_outside_circle_with_mfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params)
{
    s32 min_fval, fval;
    ThingIdx min_thing, thing;
    ulong k;

    k = 0;
    min_fval = INT32_MAX;
    min_thing = 0;
    thing = get_thing_same_type_head(ttype, subtype);
    while (thing != 0)
    {
        if (thing <= 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            // Per thing code start
            if (p_sthing->Type == ttype) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    if (!thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
            thing = p_sthing->LinkSame;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            // Per thing code start
            if (p_thing->Type == ttype) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    if (!thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
            thing = p_thing->LinkSame;
        }
        // If searching for all subtypes, make sure we really catch them all; switch to
        // second linked list if first one did not gave results
        if ((thing == 0) && (subtype == -1) && (ttype == TT_VEHICLE)) {
            subtype = SubTT_VEH_SHIP;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        if ((thing == 0) && (subtype == -1) && (ttype == TT_BUILDING)) {
            subtype = SubTT_BLD_MGUN;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        k++;
        if (k >= STHINGS_LIMIT+THINGS_LIMIT) {
            LOGERR("Infinite loop in same type things list");
            break;
        }
    }
    return min_thing;
}

static short find_thing_type_on_same_type_list_with_bfilter(short ttype, short subtype,
  ThingBoolFilter filter, ThingFilterParams *params)
{
    ThingIdx thing;
    ulong k;

    k = 0;
    thing = get_thing_same_type_head(ttype, subtype);
    while (thing != 0)
    {
        if (thing <= 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            // Per thing code start
            if (p_sthing->Type == ttype) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    if (filter(thing, params))
                        return thing;
                }
            }
            // Per thing code end
            thing = p_sthing->LinkSame;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            // Per thing code start
            if (p_thing->Type == ttype) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    if (filter(thing, params))
                        return thing;
                }
            }
            // Per thing code end
            thing = p_thing->LinkSame;
        }
        // If searching for all subtypes, make sure we really catch them all; switch to
        // second linked list if first one did not gave results
        if ((thing == 0) && (subtype == -1) && (ttype == TT_VEHICLE)) {
            subtype = SubTT_VEH_SHIP;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        if ((thing == 0) && (subtype == -1) && (ttype == TT_BUILDING)) {
            subtype = SubTT_BLD_MGUN;
            thing = get_thing_same_type_head(ttype, subtype);
        }
        k++;
        if (k >= STHINGS_LIMIT+THINGS_LIMIT) {
            LOGERR("Infinite loop in same type things list");
            break;
        }
    }
    return 0;
}

/** Slowest but most comprehensive search for things within given circle and meeting given
 * bool filter.
 * Unlike most search functions, this one allows searching for multiple thing types.
 *
 * @param X Map coordinate in map units.
 * @param Z Map coordinate in map units.
 * @param R Circle radius in map units.
 * @param ttype Thing Type; to catch all, use -1.
 * @param subtype Thing SubType; to catch all, use -1.
 * @param filter Filter callback function.
 * @param param Parameters for filter callback function.
 */
static ThingIdx find_thing_type_on_used_list_within_circle_with_bfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingBoolFilter filter, ThingFilterParams *params)
{
    ThingIdx thing;

    if ((ttype == -1) || thing_type_is_simple(ttype))
    {
        struct SimpleThing *p_sthing;
        for (thing = sthings_used_head; thing < 0; thing = p_sthing->LinkChild)
        {
            p_sthing = &sthings[thing];
            // Per thing code start
            if ((p_sthing->Type == ttype) || (ttype == -1)) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        if (filter(thing, params))
                            return thing;
                    }
                }
            }
            // Per thing code end
        }
    }
    if ((ttype == -1) || !thing_type_is_simple(ttype))
    {
        struct Thing *p_thing;
        for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
        {
            p_thing = &things[thing];
            // Per thing code start
            if ((p_thing->Type == ttype) || (ttype == -1)) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        if (filter(thing, params))
                            return thing;
                    }
                }
            }
            // Per thing code end
        }
    }
    return 0;
}

/** Slowest but most comprehensive search for things within given circle and meeting given
 * minimizing filter.
 * Unlike most search functions, this one allows searching for multiple thing types.
 *
 * @param X Map coordinate in map units.
 * @param Z Map coordinate in map units.
 * @param R Circle radius in map units.
 * @param ttype Thing Type; to catch all, use -1.
 * @param subtype Thing SubType; to catch all, use -1.
 * @param filter Filter callback function.
 * @param param Parameters for filter callback function.
 */
static ThingIdx find_thing_type_on_used_list_within_circle_with_mfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params)
{
    s32 min_fval, fval;
    ThingIdx min_thing, thing;

    min_fval = INT32_MAX;
    min_thing = 0;
    if ((ttype == -1) || thing_type_is_simple(ttype))
    {
        struct SimpleThing *p_sthing;
        for (thing = sthings_used_head; thing < 0; thing = p_sthing->LinkChild)
        {
            p_sthing = &sthings[thing];
            // Per thing code start
            if ((p_sthing->Type == ttype) || (ttype == -1)) {
                if ((p_sthing->SubType == subtype) || (subtype == -1)) {
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
        }
    }
    if ((ttype == -1) || !thing_type_is_simple(ttype))
    {
        struct Thing *p_thing;
        for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
        {
            p_thing = &things[thing];
            // Per thing code start
            if ((p_thing->Type == ttype) || (ttype == -1)) {
                if ((p_thing->SubType == subtype) || (subtype == -1)) {
                    if (thing_is_within_circle(thing, X, Z, R)) {
                        fval = filter(thing, X, Z, params);
                        if (fval < min_fval) {
                            min_fval = fval;
                            min_thing = thing;
                        }
                    }
                }
            }
            // Per thing code end
        }
    }
    return min_thing;
}

ThingIdx find_thing_type_within_circle_with_bfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingBoolFilter filter, ThingFilterParams *params)
{
    ushort tile_dist;
    ThingIdx thing;

    tile_dist = MAPCOORD_TO_TILE(R + 255);
    if ((tile_dist <= spiral_dist_tiles_limit) && (ttype != -1))
    {
        thing = find_thing_type_on_spiral_near_tile_with_bfilter(X, Z, R,
          dist_tiles_to_spiral_step[tile_dist], ttype, subtype, filter, params);
    }
    else if ((ttype == TT_PERSON) || (ttype == TT_UNKN4) || (ttype == TT_VEHICLE) || (ttype == TT_BUILDING))
    {
        thing = find_thing_type_on_same_type_list_within_circle_with_bfilter(
          X, Z, R, ttype, subtype, filter, params);
    }
    else
    {
        // do slow search on all existing things
        // if ttype == -1, then this is the only option possible
        thing = find_thing_type_on_used_list_within_circle_with_bfilter(
          X, Z, R, ttype, subtype, filter, params);
    }
    return thing;
}

ThingIdx find_thing_type_within_circle_with_mfilter(short X, short Z, ushort R,
  short ttype, short subtype, ThingMinFilter filter, ThingFilterParams *params)
{
    ushort tile_dist;
    ThingIdx thing;

    tile_dist = MAPCOORD_TO_TILE(R + 255);
    if ((tile_dist <= spiral_dist_tiles_limit) && (ttype != -1))
    {
        thing = find_thing_type_on_spiral_near_tile_with_mfilter(X, Z, R,
          dist_tiles_to_spiral_step[tile_dist], ttype, subtype, filter, params);
    }
    else if ((ttype == TT_PERSON) || (ttype == TT_UNKN4) || (ttype == TT_VEHICLE) || (ttype == TT_BUILDING))
    {
        thing = find_thing_type_on_same_type_list_within_circle_with_mfilter(
          X, Z, R, ttype, subtype, filter, params);
    }
    else
    {
        // do slow search on all existing things
        thing = find_thing_type_on_used_list_within_circle_with_mfilter(
          X, Z, R, ttype, subtype, filter, params);
    }
    return thing;
}

TbBool bfilter_match_all(ThingIdx thing, ThingFilterParams *params)
{
    return true;
}

TbBool bfilter_item_is_weapon(ThingIdx thing, ThingFilterParams *params)
{
    struct SimpleThing *p_sthing;
    short weapon;

    if (thing >= 0)
        return false;
    weapon = params->Arg1;

    p_sthing = &sthings[thing];
    if ((p_sthing->Type != SmTT_DROPPED_ITEM) && (p_sthing->Type != SmTT_CARRIED_ITEM))
        return false;

    return (weapon == -1) || (p_sthing->U.UWeapon.WeaponType == weapon);
}

TbBool bfilter_person_carries_weapon(ThingIdx thing, ThingFilterParams *params)
{
    struct Thing *p_thing;
    short weapon;

    if (thing <= 0)
        return false;
    weapon = params->Arg1;

    p_thing = &things[thing];
    if (p_thing->Type != TT_PERSON)
        return false;

    return (weapon == -1) || person_carries_weapon(p_thing, weapon);
}

TbBool bfilter_person_can_be_persuaded_now(ThingIdx thing, ThingFilterParams *params)
{
    struct Thing *p_thing;
    ThingIdx attacker;
    short weapon_range;
    ushort target_select;
    ushort energy_reqd;
    TbBool found;

    if (thing <= 0)
        return false;

    attacker = params->Arg1;
    weapon_range = params->Arg2;
    target_select = params->Arg3;
    energy_reqd = -1;

    p_thing = &things[thing];
    if (p_thing->Type != TT_PERSON)
        return false;

    found = person_can_be_persuaded_now(attacker, thing, weapon_range, target_select, &energy_reqd);
    if (found)
        params->Arg4 = energy_reqd;
    return found;
}

TbBool bfilter_waiting_stopped(ThingIdx thing, ThingFilterParams *params)
{
    struct Thing *p_thing;

    if (thing <= 0)
        return false;

    p_thing = &things[thing];

    if ((p_thing->U.UVehicle.LeisurePlace == 0) || (p_thing->Speed != 0))
        return false;

    return true;
}

short find_dropped_weapon_within_circle(short X, short Z, ushort R, short weapon)
{
    ThingIdx thing;
    ThingFilterParams params;

    params.Arg1 = weapon;
    thing = find_thing_type_within_circle_with_bfilter(X, Z, R, SmTT_DROPPED_ITEM, 0, bfilter_item_is_weapon, &params);

    return thing;
}

ThingIdx find_person_carrying_weapon_within_circle(short X, short Z, ushort R, short weapon)
{
    ThingIdx thing;
    ThingFilterParams params;

    params.Arg1 = weapon;
    thing = find_thing_type_within_circle_with_bfilter(X, Z, R, TT_PERSON, -1,
      bfilter_person_carries_weapon, &params);

    return thing;
}

ThingIdx find_person_which_can_be_persuaded_now(short X, short Z, ushort R,
  ThingIdx attacker, ushort target_select, ushort *energy_reqd)
{
    ThingIdx thing;
    ThingFilterParams params;

    params.Arg1 = attacker;
    params.Arg2 = R;
    params.Arg3 = target_select;
    params.Arg4 = -1;
    thing = find_thing_type_within_circle_with_bfilter(X, Z, R, TT_PERSON, -1,
      bfilter_person_can_be_persuaded_now, &params);
    *energy_reqd = params.Arg4;

    return thing;
}

short find_nearest_from_group(struct Thing *p_person, ushort group, ubyte no_persuaded)
{
    short ret;
    asm volatile ("call ASM_find_nearest_from_group\n"
        : "=r" (ret) : "a" (p_person), "d" (group), "b" (no_persuaded));
    return ret;
}

ThingIdx find_person_carrying_weapon(short weapon)
{
    ThingIdx thing;
    ThingFilterParams params;

    params.Arg1 = weapon;
    thing = find_thing_type_on_same_type_list_with_bfilter(TT_PERSON, -1, bfilter_person_carries_weapon, &params);

    return thing;
}

ThingIdx search_for_vehicle(short X, short Z)
{
    ThingIdx thing;
    ThingFilterParams params;

    // Try finding very close to target coords
    thing = find_thing_type_within_circle_with_bfilter(X, Z, 48, TT_VEHICLE, 0, bfilter_match_all, &params);
    // If very clos search failed, retry with bigger radius
    if (thing == 0)
        thing = find_thing_type_within_circle_with_bfilter(X, Z, 1024, TT_VEHICLE, 0, bfilter_match_all, &params);

    return thing;
}

ThingIdx search_things_for_index(short index)
{
    ThingIdx thing;
    if (index <= 0)
    {
        struct SimpleThing *p_sthing;
        for (thing = sthings_used_head; thing < 0; thing = p_sthing->LinkChild)
        {
            p_sthing = &sthings[thing];
            if (index == p_sthing->ThingOffset) {
                return thing;
            }
        }
    }
    else
    {
        struct Thing *p_thing;
        for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
        {
            p_thing = &things[thing];
            if (index == p_thing->ThingOffset) {
                if (p_thing->Type != TT_VEH_TURRET)
                    return thing;
            }
        }
    }
    return 0;
}

ThingIdx search_things_for_any_including_offmap_nearest_within_circle(short X, short Z, ushort R)
{
    ThingIdx thing;
    ThingFilterParams params;

    thing = find_thing_type_within_circle_with_mfilter(X, Z, R, -1, -1, mfilter_nearest, &params);

    return thing;
}

ThingIdx search_things_for_type_farthest_from_xz(short X, short Z, short ttype, short subtype)
{
    ThingIdx thing;
    ThingFilterParams params;

    thing = find_thing_type_on_same_type_list_outside_circle_with_mfilter(X, Z,
      TILE_TO_MAPCOORD(1,0), ttype, subtype, mfilter_farthest, &params);

    return thing;
}

ThingIdx search_things_for_uniqueid(short uniqid, ubyte flag)
{
    ThingIdx ret;
    asm volatile ("call ASM_search_things_for_uniqueid\n"
        : "=r" (ret) : "a" (uniqid), "d" (flag));
    return ret;
}

ThingIdx find_nearest_object2(short mx, short mz, ushort sub_type)
{
    ThingIdx ret;
    asm volatile ("call ASM_find_nearest_object2\n"
        : "=r" (ret) : "a" (mx), "d" (mz), "b" (sub_type));
    return ret;
}

short search_object_for_qface(ushort object, ubyte gflag, ubyte flag, ushort after)
{
    short ret;
    asm volatile ("call ASM_search_object_for_qface\n"
        : "=r" (ret) : "a" (object), "d" (gflag), "b" (flag), "c" (after));
    return ret;
}

ThingIdx search_for_station(short X, short Z)
{
#if 0
    ushort ret;
    asm volatile ("call ASM_search_for_station\n"
        : "=r" (ret) : "a" (x), "d" (z));
    return ret;
#endif
    ThingIdx thing;
    ThingFilterParams params;

    thing = find_thing_type_within_circle_with_bfilter(X, Z, TILE_TO_MAPCOORD(15,0),
      TT_BUILDING, SubTT_BLD_STATION, bfilter_match_all, &params);

    return thing;
}

ThingIdx search_for_ferry(short X, short Y, short Z, ushort R)
{
    ThingIdx thing;
    ThingFilterParams params;

    params.Arg1 = Y;
    params.Arg2 = R;
    thing = find_thing_type_within_circle_with_bfilter(X, Z, R,
      TT_VEHICLE, SubTT_VEH_SHIP, bfilter_waiting_stopped, &params);

    return thing;
}

struct Thing *check_for_radius_hit_person(int prc_x, int prc_y, int prc_z,
        int radius, struct Thing *p_owner, int flag, int skip)
{
#if 0
    struct Thing *ret;
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_check_for_radius_hit_person\n"
        : "=r" (ret) : "a" (prc_x), "d" (prc_y), "b" (prc_z), "c" (radius), "g" (p_owner), "g" (flag), "g" (skip));
    return ret;
#else
    ThingIdx thing;
    short dtx, dtz;
    int cor_x, cor_y, cor_z;

    cor_x = PRCCOORD_TO_MAPCOORD(prc_x);
    cor_y = PRCCOORD_TO_MAPCOORD(prc_y);
    cor_z = PRCCOORD_TO_MAPCOORD(prc_z);

    for (dtx = -1; dtx <= 1; dtx++)
    {
      for (dtz = -1; dtz <= 1; dtz++)
      {
        uint tile_x, tile_z;

        tile_x = MAPCOORD_TO_TILE(cor_x) + dtx;
        tile_z = MAPCOORD_TO_TILE(cor_z) + dtz;
        if ((tile_x >= MAP_TILE_WIDTH) || (tile_z >= MAP_TILE_HEIGHT))
            continue;
        thing = game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x].Child;
        while (thing != 0)
        {
            if (thing <= 0)
            {
              struct SimpleThing *p_sthing;

              p_sthing = &sthings[thing];
              if (p_sthing->Type == SmTT_STATIC && (p_sthing->Flag & TngF_Destroyed) == 0 && p_sthing->Frame != 1008)
              {
                  if (thing_intersects_cylinder(thing, cor_x, cor_y, cor_z, radius, 25)) {
                      if (--skip < 0) {
                          if ((p_sthing->StartFrame <= 1004) || (p_sthing->StartFrame >= 1008))
                              return (struct Thing *)p_sthing;
                          set_static_on_fire(p_sthing);
                      }
                  }
              }
              thing = p_sthing->Next;
            }
            else
            {
              struct Thing *p_thing;
              ushort group;

              p_thing = &things[thing];
              if (p_owner != NULL)
                  group = p_owner->U.UPerson.EffectiveGroup;
              else
                  group = 99;
              if ((p_thing->U.UPerson.EffectiveGroup != group) || (group >= 100))
              {
                if (!groups_have_truce(group, p_thing->U.UPerson.EffectiveGroup)
                  && (p_thing->Type == TT_PERSON) && (p_thing != p_owner))
                {
                    if ((p_thing->State != PerSt_DEAD) && (p_thing->State != PerSt_PERSON_BURNING) && (p_thing->Flag & TngF_Destroyed) == 0)
                    {
                        if (thing_intersects_cylinder(thing, cor_x, cor_y, cor_z, radius, 25)) {
                            if (--skip < 0)
                                return p_thing;
                        }
                    }
                }
                if (--skip < 0)
                {
                  if (flag)
                  {
                    if ((p_thing->Type == TT_VEHICLE) && (p_thing->State != VehSt_UNKN_D)
                      && (p_owner == NULL || p_thing->ThingOffset != p_owner->U.UPerson.Vehicle))
                    {
                        if (thing_intersects_cylinder(thing, cor_x, cor_y, cor_z, radius, 25)) {
                            if (--skip < 0)
                                return p_thing;
                        }
                    }
                  }
                }
                if (flag)
                {
                    if ((p_thing->Type == TT_BUILDING) && (p_thing->SubType == SubTT_BLD_MGUN) && ((p_thing->Flag & TngF_Destroyed) == 0))
                    {
                        if (thing_intersects_cylinder(thing, cor_x, cor_y, cor_z, radius, 25)) {
                            if (--skip < 0)
                                return p_thing;
                        }
                    }
                }
                if (flag)
                {
                    if ((p_thing->Type == TT_MINE) && (p_thing->SubType == 48) && (p_thing->Flag & TngF_Destroyed) == 0 && (p_thing->State != 13))
                    {
                        if (thing_intersects_cylinder(thing, cor_x, cor_y, cor_z, radius, 25)) {
                            if (--skip < 0)
                                return p_thing;
                        }
                    }
                }
              }
              thing = p_thing->Next;
            }
        }
      }
    }
    return NULL;
#endif
}

short find_nearest_person_min(int x, int y, int z,
  int n_dist, int *a_dist, int angle1, u32 group_bits)
{
    short ret;
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_find_nearest_person_min\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z), "c" (n_dist), "g" (a_dist), "g" (angle1), "g" (group_bits));
    return ret;
}


/******************************************************************************/
