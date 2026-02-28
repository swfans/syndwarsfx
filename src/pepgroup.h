/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file pepgroup.h
 *     Header file for pepgroup.c.
 * @par Purpose:
 *     People groups support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2023 - 11 Jul 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef PEPGROUP_H
#define PEPGROUP_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define PEOPLE_GROUPS_COUNT 32
#define PEOPLE_GROUPS_INDEX_MASK 0x1F

struct Thing;

/** Data presenting group attitude toward other groups.
 */
struct WarFlag { // sizeof=40
 /** Bitfields of groups whose members are attacked  on sight.
  */
  u32 KillOnSight;
 /** Bitfields of groups whose members are attacked if wielding visible weapons.
  */
  u32 KillIfWeaponOut;
 /** Bitfields of groups whose members are attacked if have any weapons, even concealed.
  */
  u32 KillIfArmed;
 /** Bitfields of groups whose members are not attacked.
  */
  u32 Truce;
  ubyte Guardians[8];
  long ugfld_24;
  long ugfld_28;
  long ugfld_32;
  long ugfld_36;
};

struct GroupAction {
  u32 FiredOn;
  u32 Hit;
  short Dead;
  short Alive;
  short Persuaded;
  ushort Storming;
};

#pragma pack()
/******************************************************************************/
extern struct WarFlag war_flags[PEOPLE_GROUPS_COUNT];
extern struct GroupAction group_actions[PEOPLE_GROUPS_COUNT+1];

/** Checks if two things have equal effective groups.
 */
TbBool things_check_same_group(ThingIdx tng1, ThingIdx tng2);

/** Checks the two thing groups are equal.
 */
TbBool groups_equal(short grp1, short grp2);

/** Finds Group ID for which there are no things created.
 */
short find_unused_group_id(TbBool largest);

/** Count how many people of given kind are in given group.
 *
 * This can be called during level load, as it does not use per-type linked lists.
 */
ushort count_people_in_group(ushort group, short subtype);

/** Copy all properties of one group into another group.
 */
void groups_copy(short pv_group, short nx_group, ubyte allow_kill);

/** Returns if given group has any guardians set.
 */
TbBool group_has_guardians(short check_grp);

TbBool groups_have_kill_if_weapon_out(short check_grp, short target_grp);

TbBool groups_have_kill_if_armed(short check_grp, short target_grp);

TbBool groups_have_kill_on_sight(short check_grp, short target_grp);
void groups_set_kill_on_sight(short mod_grp, short target_grp, TbBool state);

TbBool groups_have_truce(short check_grp, short target_grp);
void groups_set_truce(short mod_grp, short target_grp, TbBool state);

/** Transfer some people of given subtype from one group to the other.
 * Skips `stay_limit` of people, then transfers the next `tran_limit`.
 */
int group_to_group_transfer_people(short pv_group, short nx_group,
  short subtype, int stay_limit, int tran_limit);

void thing_groups_clear_all_actions(void);

TbBool all_group_members_destroyed(ushort group);
TbBool all_group_persuaded(ushort group);
ubyte all_group_arrived(ushort group, short x, short y, short z, int radius);
TbBool group_has_all_killed_or_persuaded_by_player(ushort group, ushort plyr);
TbBool group_has_all_survivors_in_vehicle(ushort group, ThingIdx vehicle);

TbBool group_has_no_less_members_in_vehicle(ushort group, ThingIdx vehicle, ushort amount);
TbBool group_has_no_less_members_persuaded_by_player(ushort group, ushort plyr, ushort amount);
TbBool group_has_no_less_members_killed_or_persuaded_by_player(ushort group, ushort plyr, ushort amount);
TbBool group_has_no_less_members_dead(ushort group, ushort amount);
TbBool group_has_no_less_members_near_thing(ThingIdx neartng, ushort group, ushort amount, ushort radius);
TbBool group_has_no_less_members_persuaded_by_person(ushort group, ThingIdx owntng, ushort amount);

void reset_default_player_agent(PlayerIdx plyr, short plagent, struct Thing *p_agent, short new_type);
ushort make_group_into_players(ushort group, ushort plyr, ushort max_agent, short new_type);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
