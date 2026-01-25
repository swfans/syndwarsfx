/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file pepgroup.c
 *     People groups support.
 * @par Purpose:
 *     Implement handling parameters of groups to which people are assigned.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2023 - 11 Jul 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pepgroup.h"

#include "bfutility.h"
#include "bfmemut.h"

#include "command.h"
#include "player.h"
#include "thing.h"
#include "game.h"
#include "game_options.h"
/******************************************************************************/

short find_unused_group_id(TbBool largest)
{
    ThingIdx thing;
    short group;
    ulong used_groups;
    struct Thing *p_thing;

    used_groups = 0;
    for (thing = things_used_head; thing != 0; thing = p_thing->LinkChild)
    {
        p_thing = &things[thing];
        used_groups |= (1 << p_thing->U.UPerson.Group);
    }
    if (largest)
    {
        for (group = PEOPLE_GROUPS_COUNT-1; group > 0; group--) {
            if ((used_groups & (1 << group)) == 0)
                return group;
        }
    }
    else
    {
        for (group = 1; group < PEOPLE_GROUPS_COUNT; group++) {
            if ((used_groups & (1 << group)) == 0)
                return group;
        }
    }
    return -1;
}

ushort count_people_in_group(ushort group, short subtype)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort count;

    count = 0;
    for (thing = things_used_head; thing != 0; thing = p_thing->LinkChild)
    {
        p_thing = &things[thing];

        if (p_thing->U.UPerson.Group != group)
            continue;

        if (p_thing->Type != TT_PERSON)
            continue;

        if ((subtype != -1) && (p_thing->SubType != subtype))
            continue;

        count++;
    }
    return count;
}

void thing_group_copy(short pv_group, short nx_group, ubyte allow_kill)
{
    int i;

    for (i = 0; i < PEOPLE_GROUPS_COUNT; i++)
    {
        if (i == pv_group)
        {
            war_flags[i].KillOnSight &= ~(1 << pv_group);
            war_flags[i].KillIfWeaponOut &= ~(1 << pv_group);
            war_flags[i].KillIfArmed &= ~(1 << pv_group);
            war_flags[i].KillOnSight &= ~(1 << nx_group);
            war_flags[i].KillIfWeaponOut &= ~(1 << nx_group);
            war_flags[i].KillIfArmed &= ~(1 << nx_group);
            continue;
        }
        if ((war_flags[i].KillOnSight & (1 << pv_group)) != 0)
            war_flags[i].KillOnSight |= (1 << nx_group);
        else
            war_flags[i].KillOnSight &= ~(1 << nx_group);

        if ((war_flags[i].KillIfWeaponOut & (1 << pv_group)) != 0)
            war_flags[i].KillIfWeaponOut |= (1 << nx_group);
        else
            war_flags[i].KillIfWeaponOut &= ~(1 << nx_group);

        if ((war_flags[i].KillIfArmed & (1 << pv_group)) != 0)
            war_flags[i].KillIfArmed |= (1 << nx_group);
        else
            war_flags[i].KillIfArmed &= ~(1 << nx_group);

        if ((war_flags[i].Truce & (1 << pv_group)) != 0)
            war_flags[i].Truce |= (1 << nx_group);
        else
            war_flags[i].Truce &= ~(1 << nx_group);
    }

    LbMemoryCopy(&war_flags[nx_group], &war_flags[pv_group], sizeof(struct WarFlag));

    if (allow_kill & 0x01) {
        war_flags[pv_group].KillOnSight |= (1 << nx_group);
        war_flags[nx_group].KillOnSight |= (1 << pv_group);
    }
    if (allow_kill & 0x02) {
        war_flags[pv_group].KillIfWeaponOut |= (1 << nx_group);
        war_flags[nx_group].KillIfWeaponOut |= (1 << pv_group);
    }
    if (allow_kill & 0x04) {
        war_flags[pv_group].KillIfArmed |= (1 << nx_group);
        war_flags[nx_group].KillIfArmed |= (1 << pv_group);
    }

    for (i = 0; i < 8; i++)
    {
        if (level_def.PlayableGroups[i] == pv_group) {
            level_def.PlayableGroups[i] = nx_group;
            if (pv_group == 0) // If replacing group 0, don't modify all the filler zeros
                break;
        }
    }
}

void thing_group_set_kill_on_sight(short mod_grp, short target_grp, TbBool state)
{
    mod_grp &= 0x1F;
    if (state)
        war_flags[mod_grp].KillOnSight |= 1 << target_grp;
    else
        war_flags[mod_grp].KillOnSight &= ~(1 << target_grp);
}

TbBool thing_group_have_truce(short check_grp, short target_grp)
{
    check_grp &= 0x1F;
    return (war_flags[check_grp].Truce & (1 << target_grp)) != 0;
}

void thing_group_set_truce(short mod_grp, short target_grp, TbBool state)
{
    mod_grp &= 0x1F;
    if (state)
        war_flags[mod_grp].Truce |= 1 << target_grp;
    else
        war_flags[mod_grp].Truce &= ~(1 << target_grp);
}

int thing_group_transfer_people(short pv_group, short nx_group, short subtype, int stay_limit, int tran_limit)
{
    ThingIdx thing;
    struct Thing *p_thing;
    int count;

    count = 0;
    for (thing = things_used_head; thing != 0; thing = p_thing->LinkChild)
    {
        p_thing = &things[thing];

        if (p_thing->Type != TT_PERSON)
            continue;

        if ((subtype != -1) && (p_thing->SubType != subtype))
            continue;

        if (stay_limit > 0) {
            stay_limit--;
            continue;
        }

        if (p_thing->U.UPerson.Group == pv_group) {
            p_thing->U.UPerson.Group = nx_group;
            count++;
        }
        if (p_thing->U.UPerson.EffectiveGroup == pv_group) {
            p_thing->U.UPerson.EffectiveGroup = nx_group;
        }

        if (count >= tran_limit)
            break;
    }
    return count;
}

void thing_group_clear_action(short group)
{
    struct GroupAction *p_grpact;

    p_grpact = &group_actions[group];
    p_grpact->Alive = 0;
    p_grpact->Dead = 0;
    p_grpact->Hit = 0;
    p_grpact->FiredOn = 0;
    p_grpact->Persuaded = 0;
    p_grpact->Storming = 0;
}

void thing_groups_clear_all_actions(void)
{
    short group;

    for (group = 0; group < PEOPLE_GROUPS_COUNT; group++)
    {
        thing_group_clear_action(group);
    }
}

TbBool all_group_members_destroyed(ushort group)
{
    ThingIdx thing;
    struct Thing *p_thing;

    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (!thing_is_destroyed(thing))
            return false;
    }
    return true;
}

TbBool all_group_persuaded(ushort group)
{
    ThingIdx thing;
    struct Thing *p_thing;

    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (!person_is_persuaded(thing) || ((things[p_thing->Owner].Flag & TngF_PlayerAgent) == 0))
        {
            if (!person_is_dead(thing) && !thing_is_destroyed(thing))
                return false;
        }
    }
    return true;
}

ubyte all_group_arrived(ushort group, short x, short y, short z, int radius)
{
    ubyte ret;
    asm volatile (
      "push %5\n"
      "call ASM_all_group_arrived\n"
        : "=r" (ret) : "a" (group), "d" (x), "b" (y), "c" (z), "g" (radius));
    return ret;
}

TbBool group_has_all_killed_or_persuaded_by_player(ushort group, ushort plyr)
{
    ThingIdx thing;
    struct Thing *p_thing;

    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (!person_is_persuaded_by_player(thing, plyr))
        {
            if (!person_is_dead(thing) && !thing_is_destroyed(thing))
                return false;
        }
    }
    return true;
}

TbBool group_has_all_survivors_in_vehicle(ushort group, ThingIdx vehicle)
{
    ThingIdx thing;
    struct Thing *p_thing;

    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (!person_is_in_vehicle(p_thing, vehicle))
        {
            if (!person_is_dead(thing) && !thing_is_destroyed(thing))
                return false;
        }
    }
    return false;
}

TbBool group_has_no_less_members_in_vehicle(ushort group, ThingIdx vehicle, ushort amount)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (!person_is_dead(thing) && !thing_is_destroyed(thing))
        {
            if (person_is_in_vehicle(p_thing, vehicle))
                n++;
        }
        if (n >= amount)
            return true;
    }
    return false;
}

TbBool group_has_no_less_members_persuaded_by_player(ushort group, ushort plyr, ushort amount)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (person_is_persuaded_by_player(thing, plyr))
            n++;
        if (n >= amount)
            return true;
    }
    return false;
}

TbBool group_has_no_less_members_killed_or_persuaded_by_player(ushort group, ushort plyr, ushort amount)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (person_is_persuaded_by_player(thing, plyr) ||
          person_is_dead(thing) || thing_is_destroyed(thing))
            n++;
        if (n >= amount)
            return true;
    }
    return false;
}

TbBool group_has_no_less_members_dead(ushort group, ushort amount)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (person_is_dead(thing) || thing_is_destroyed(thing))
            n++;
        if (n >= amount)
            return true;
    }
    return false;
}

TbBool group_has_no_less_members_near_thing(ThingIdx neartng, ushort group, ushort amount, ushort radius)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;
    short nearX, nearZ;

    if ((neartng == 0) || person_is_dead(neartng) || thing_is_destroyed(neartng))
        return false;
    if (neartng <= 0) {
        struct SimpleThing *p_neartng;
        p_neartng = &sthings[neartng];
        nearX = p_neartng->X;
        nearZ = p_neartng->Z;
    } else {
        struct Thing *p_neartng;
        p_neartng = &things[neartng];
        nearX = p_neartng->X;
        nearZ = p_neartng->Z;
    }

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (!person_is_dead(thing) && !thing_is_destroyed(thing))
        {
            if (thing_is_within_circle(thing, nearX, nearZ, radius << 6))
                n++;
        }
        if (n >= amount)
            return true;
    }
    return false;
}

TbBool group_has_no_less_members_persuaded_by_person(ushort group, ThingIdx owntng, ushort amount)
{
    ThingIdx thing;
    struct Thing *p_thing;
    ushort n;

    n = 0;
    thing = same_type_head[256 + group];
    for (; thing > 0; thing = p_thing->LinkSameGroup)
    {
        p_thing = &things[thing];
        if (person_is_persuaded_by_person(thing, owntng))
            n++;
        if (n >= amount)
            return true;
    }
    return false;
}

void reset_default_player_agent(PlayerIdx plyr, short plagent, struct Thing *p_agent, short new_type)
{
    PlayerInfo *p_player;
    ubyte grp;

    p_player = &players[plyr];

    p_player->MyAgent[plagent] = p_agent;
    p_agent->Flag = TngF_PlayerAgent;

    grp = level_def.PlayableGroups[plyr];
    p_agent->U.UPerson.Group = grp;
    p_agent->U.UPerson.EffectiveGroup = grp;

    if (plagent > p_player->DoubleMode)
    {
        if (in_network_game && p_player->DoubleMode) {
            p_agent->State = PerSt_DEAD;
            p_agent->Flag |= TngF_Unkn02000000 | TngF_Destroyed;
        }
        p_player->DirectControl[plagent] = 0;
    }
    else
    {
        p_player->DirectControl[plagent] = p_agent->ThingOffset;
        p_agent->Flag |= TngF_Unkn1000;
        if ((local_player_no == plyr) && (plagent == 0)) {
            game_set_cam_track_thing_xz(p_agent->ThingOffset);
        }
    }

    p_agent->State = PerSt_NONE;
    { // Why are we tripling the health?
        uint health;
        health = 3 * p_agent->Health;
        if (health > PERSON_MAX_HEALTH_LIMIT)
            health = PERSON_MAX_HEALTH_LIMIT;
        p_agent->Health = health;
        p_agent->U.UPerson.MaxHealth = health;
    }
    p_agent->U.UPerson.Mood = 0;
    p_agent->U.UPerson.ComHead = 0;
    p_agent->U.UPerson.Target2 = 0;
    p_agent->PTarget = NULL;
    p_agent->U.UPerson.ComCur = (plyr << 2) + plagent;
    p_agent->OldTarget = 0;

    {
        p_agent->U.UPerson.WeaponsCarried = p_player->Weapons[plagent] | (1 << (WEP_ENERGYSHLD-1));
        p_agent->U.UPerson.UMod = p_player->Mods[plagent];
        p_agent->U.UPerson.CurrentWeapon = WEP_NULL;
    }
    if (in_network_game)
        do_weapon_quantities_net_to_player(p_agent);
    else
        player_agent_set_weapon_quantities_max(p_agent);

    switch (new_type)
    {
    case 0:
        p_agent->SubType = SubTT_PERS_AGENT;
        reset_person_frame(p_agent);
        break;
    case 1:
        p_agent->SubType = SubTT_PERS_ZEALOT;
        reset_person_frame(p_agent);
        break;
    case 2:
        p_agent->SubType = SubTT_PERS_PUNK_M;
        reset_person_frame(p_agent);
        break;
    default:
        break;
    }
    switch_person_anim_mode(p_agent, 0);
}

void reset_group_member_player_agent(PlayerIdx plyr, ushort plagent, ushort high_tier, struct Thing *p_agent, short new_type)
{
    PlayerInfo *p_player;

    p_player = &players[plyr];

    p_player->MyAgent[plagent] = p_agent;
    p_agent->Flag |= TngF_PlayerAgent;

    if (plagent > p_player->DoubleMode)
    {
        if (in_network_game && p_player->DoubleMode) {
            p_agent->State = PerSt_DEAD;
            p_agent->Flag |= TngF_Unkn02000000 | TngF_Destroyed;
        }
        p_player->DirectControl[plagent] = 0;
    }
    else
    {
        p_player->DirectControl[plagent] = p_agent->ThingOffset;
        p_agent->Flag |= TngF_Unkn1000;
        if ((plyr == local_player_no) && (plagent == 0)) {
            game_set_cam_track_thing_xz(p_agent->ThingOffset);
        }
    }

    if (in_network_game)
        set_person_stats_type(p_agent, SubTT_PERS_AGENT);

    if (ingame.GameMode != GamM_Unkn2)
    {
        if ((p_agent->SubType == SubTT_PERS_AGENT) || (p_agent->SubType == SubTT_PERS_ZEALOT))
        {
            p_agent->U.UPerson.WeaponsCarried = p_player->Weapons[high_tier] | (1 << (WEP_ENERGYSHLD-1));
            p_agent->U.UPerson.UMod.Mods = p_player->Mods[high_tier].Mods;
        }
        p_agent->U.UPerson.CurrentWeapon = 0;
    }

    person_init_preplay_command(p_agent);

    // Player agents can go with default loadout for the level, but usually we want them to
    // use either the equipment selected by the player (either local one or from the net).
    // Setting command to player control is required to properly update weapons
    p_agent->U.UPerson.ComCur = (plyr << 2) + plagent;
    if (ingame.GameMode == GamM_Unkn3)
        player_agent_set_weapon_quantities_proper(p_agent);
    else
        player_agent_set_weapon_quantities_max(p_agent);

    // Using any commands other than preplay on player agents requires explicit marking
    // in form of use of EXECUTE_COMS.
    if ((p_agent->U.UPerson.ComHead != 0) &&
        (game_commands[p_agent->U.UPerson.ComHead].Type == PCmd_EXECUTE_COMS))
    {
        // Now we can re-set current command to the real command
        p_agent->U.UPerson.ComCur = p_agent->U.UPerson.ComHead;
        person_start_executing_commands(p_agent);
    }
    else
    {
        p_agent->U.UPerson.ComCur = (plyr << 2) + plagent;
        p_agent->U.UPerson.ComHead = 0;
    }

    {
        short cor_x, cor_z;
        get_thing_position_mapcoords(&cor_x, NULL, &cor_z, p_agent->ThingOffset);
        netgame_agent_pos_x[plyr][plagent] = cor_x;
        netgame_agent_pos_z[plyr][plagent] = cor_z;
    }
    p_agent->State = PerSt_NONE;
    { // Why are we tripling the health?
        uint health;
        health = 3 * p_agent->Health;
        if (health > PERSON_MAX_HEALTH_LIMIT)
            health = PERSON_MAX_HEALTH_LIMIT;
        p_agent->Health = health;
        p_agent->U.UPerson.MaxHealth = health;
    }

    switch (new_type)
    {
    case 0:
        p_agent->SubType = SubTT_PERS_AGENT;
        reset_person_frame(p_agent);
        break;
    case 1:
        p_agent->SubType = SubTT_PERS_ZEALOT;
        reset_person_frame(p_agent);
        break;
    case 2:
        p_agent->SubType = SubTT_PERS_PUNK_M;
        reset_person_frame(p_agent);
        break;
    }
    p_agent->U.UPerson.FrameId.Version[0] = 0;
    if (p_agent->U.UPerson.CurrentWeapon == 0)
    {
        switch_person_anim_mode(p_agent, ANIM_PERS_IDLE);
    }
}

ushort make_group_into_players(ushort group, ushort plyr, ushort max_agent, short new_type)
{
    struct Thing *p_person;
    ushort plagent, high_tier;
    ulong n;

    p_person = NULL;
    plagent = 0;
    high_tier = 0;
    for (n = things_used_head; n != 0; n = p_person->LinkChild)
    {
        p_person = &things[n];
        if ((p_person->U.UPerson.Group != group) || (p_person->Type != TT_PERSON))
            continue;

        reset_group_member_player_agent(plyr, plagent, high_tier, p_person, new_type);

        if ((p_person->SubType == SubTT_PERS_AGENT) || (p_person->SubType == SubTT_PERS_ZEALOT))
            high_tier++;

        if (++plagent == max_agent)
            break;
    }
    // At this point, plagent is a count of filled agents
    n = plagent;
    // Fill the rest of agents array, to avoid using leftovers
    for (; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
        players[plyr].MyAgent[plagent] = &things[0];

    return n;
}
/******************************************************************************/
