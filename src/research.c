/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file research.c
 *     Routines implementing weapons and mods research.
 * @par Purpose:
 *     Implement general functions for handling research progress.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "research.h"

#include "bfutility.h"

#include "weapon.h"
#include "cybmod.h"
#include "feresearch.h"
#include "game_options.h"
#include "game.h"
#include "guitext.h"
#include "wadfile.h"
#include "swlog.h"
/******************************************************************************/
short daily_scientist_death_chance_permil = 20;
short scientists_per_group = 4;
short weapon_donate_research_incr_permil = 125;
short research_progress_rtc_minutes = 0;

struct ResearchInfo research;

/******************************************************************************/

void load_scientist_lost_reason(ushort reason_no, ubyte *scratch_buf)
{
    int totlen;
    char *s;
    char c;
    ushort i;

    totlen = load_file_alltext("lost.txt", scratch_buf);
    if (totlen == -1) {
        return;
    }
    s = (char *)scratch_buf;
    for (i = 0; i != reason_no; i++)
    {
        do
            c = *s++;
        while (c != '\n');
    }
    for (i = 0; i < sizeof(scientist_lost_reason)-1; i++,s++)
    {
        c = *s;
        if ((c == '\r') || (c == '\0'))
            break;
        scientist_lost_reason[i] = c;
    }
    scientist_lost_reason[i] = '\0';
}

void research_wep_store_daily_progress(int progress)
{
    short cwep;
    ushort cday;

    cwep = research.CurrentWeapon;
    if (research.WeaponDaysDone[cwep] < RESEARCH_DAYS_STORED-1)
    {
        cday = research.WeaponDaysDone[cwep] + 1;
    } else
    {
        // Shift the data left to make place for the next day
        for (cday = 0; cday < RESEARCH_DAYS_STORED-1; cday++) {
            research.WeaponProgress[cwep][cday] = research.WeaponProgress[cwep][cday + 1];
        }
        cday = RESEARCH_DAYS_STORED-1;
    }
    if (cday > 0)
        research.WeaponProgress[cwep][cday] = research.WeaponProgress[cwep][cday - 1];
    else
        research.WeaponProgress[cwep][cday] = 0;
    research.WeaponProgress[cwep][cday] += progress;
}

int research_wep_get_progress(short cwep)
{
    ushort cday;

    if (research.WeaponDaysDone[cwep] >= RESEARCH_DAYS_STORED-1)
        cday = RESEARCH_DAYS_STORED-1;
    else
        cday = research.WeaponDaysDone[cwep] + 1;
    return research.WeaponProgress[cwep][cday];
}

TbBool is_research_weapon_completed(WeaponType wtype)
{
    ulong wflag;
    wflag = 1 << (wtype - 1);
    return (research.WeaponsCompleted & wflag) != 0;
}

TbBool is_research_weapon_allowed(WeaponType wtype)
{
    ulong wflag;
    wflag = 1 << (wtype - 1);
    return (research.WeaponsAllowed & wflag) != 0;
}

void research_weapon_allow(WeaponType wtype)
{
    ulong wflag;
    wflag = 1 << (wtype - 1);
    research.WeaponsAllowed |= wflag;
}

void research_weapon_flags_allow(ulong wpflags)
{
    ulong oneflag;
    WeaponType wtype;

    for (wtype = WEP_NULL + 1; wtype < WEP_TYPES_COUNT; wtype++)
    {
        oneflag = 1 << (wtype - 1);
        if ((wpflags & oneflag) == 0)
            continue;
        if ((research.WeaponsCompleted & oneflag) != 0)
            continue;
        research.WeaponsAllowed |= oneflag;
    }
}

void research_weapon_complete(WeaponType wtype)
{
    ulong wflag;
    wflag = 1 << (wtype - 1);
    research.WeaponsAllowed &= ~wflag;
    // if already researching this weapon, finish research
    if (research.CurrentWeapon == (wtype - 1))
        research.CurrentWeapon = -1;
    research.WeaponsCompleted |= wflag;
}

void research_current_weapon_complete(void)
{
    short cwep;

    cwep = research.CurrentWeapon;
    research.WeaponsCompleted |= 1 << cwep;
    research.WeaponsAllowed &= ~(1 << cwep);
    research.CurrentWeapon = -1;
    clear_research_screen();
    refresh_equip_list = 1;
}

int research_wep_next_type(void)
{
    WeaponType wtype;

    wtype = WEP_NULL + 1;
    while ((research.WeaponsAllowed & (1 << (wtype-1))) == 0)
    {
        wtype++;
        if (wtype >= WEP_TYPES_COUNT)
            return -1;
    }
    if ((research.CurrentWeapon == -1) ||
      (weapon_tech_level[wtype + 1] > weapon_tech_level[research.CurrentWeapon + 1])) {
        research.CurrentWeapon = wtype - 1;
        return wtype;
    }
    return -1;
}

void research_cymod_store_daily_progress(int progress)
{
    short cmod;
    ushort cday;

    cmod = research.CurrentMod;
    if (research.ModDaysDone[cmod] < RESEARCH_DAYS_STORED-1)
    {
        cday = research.ModDaysDone[cmod] + 1;
    } else
    {
        // Shift the data left to make place for the next day
        for (cday = 0; cday < RESEARCH_DAYS_STORED-1; cday++) {
            research.ModProgress[cmod][cday] = research.ModProgress[cmod][cday + 1];
        }
        cday = RESEARCH_DAYS_STORED-1;
    }
    if (cday > 0)
        research.ModProgress[cmod][cday] = research.ModProgress[cmod][cday - 1];
    else
        research.ModProgress[cmod][cday] = 0;
    research.ModProgress[cmod][cday] += progress;
}

int research_cymod_get_progress(short cmod)
{
    ushort cday;

    if (research.ModDaysDone[cmod] >= RESEARCH_DAYS_STORED-1)
        cday = RESEARCH_DAYS_STORED-1;
    else
        cday = research.ModDaysDone[cmod] + 1;
    return research.ModProgress[cmod][cday];
}

TbBool is_research_cymod_completed(ushort mtype)
{
    ulong mflag;
    mflag = 1 << (mtype - 1);
    return (research.ModsCompleted & mflag) != 0;
}

TbBool is_research_cymod_allowed(ushort mtype)
{
    ulong mflag;
    mflag = 1 << (mtype - 1);
    return (research.ModsAllowed & mflag) != 0;
}

void research_cymod_allow(ushort mtype)
{
    ulong mflag;
    mflag = 1 << (mtype - 1);
    research.ModsAllowed |= mflag;
}

void research_cymod_complete(ushort mtype)
{
    ulong mflag;
    mflag = 1 << (mtype - 1);
    research.ModsAllowed &= ~mflag;
    // if already researching this mod, finish research
    if (research.CurrentMod == (mtype - 1))
        research.CurrentMod = -1;
    research.ModsCompleted |= mflag;
}

void research_unkn_func_003(void)
{
    struct WeaponDef *wdef;
    WeaponType wtype;

    wtype = research.CurrentWeapon + 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (wtype < 0 || wtype >= WEP_TYPES_COUNT)
        return;
#pragma GCC diagnostic pop

    wdef = &weapon_defs[wtype];
    //TODO this modifies configuration which is not saved.
    // instead we should count the dropped weapons in
    // a separate variable within structs which are saved/loaded
    wdef->PercentPerDay += wdef->PercentPerDay * weapon_donate_research_incr_permil / 1000;
}

void research_current_cymod_complete(void)
{
    short mtype;

    mtype = research.CurrentMod + 1;
    research.ModsCompleted |= 1 << (mtype - 1);
    research.ModsAllowed &= ~(1 << (mtype - 1));
    //TODO CONFIG store required research in mod config (and weapon config as well)
    // Allow research of next level of the mod
    if (research.CurrentMod % 3 != 2 && mtype < MOD_EPIDERM1)
        research_cymod_allow(mtype + 1);
    research.CurrentMod = -1;
    clear_research_screen();
    refresh_equip_list = 1;
}

int research_cymod_next_type(void)
{
    int mdtype;

    mdtype = 0;
    while ((research.ModsAllowed & (1 << mdtype)) == 0)
    {
        mdtype++;
        if (mdtype >= MOD_TYPES_COUNT)
            return -1;
    }
    if ((research.CurrentMod == -1) ||
      (mod_tech_level[mdtype + 1] > mod_tech_level[research.CurrentMod + 1])) {
        research.CurrentMod = mdtype;
        return mdtype;
    }
    return -1;
}

int research_unkn_func_004(ushort percent_per_day, int expect_funding, int real_funding)
{
    int n_remain, overhead;
    int points_total, ppd, delta;
    int points_by_group;

    // Adjust points to number of scientists
    points_total = percent_per_day << 8;
    n_remain = research.Scientists;
    delta = scientists_per_group;
    ppd = points_total;
    if (n_remain > delta)
    {
        overhead = 2;
        while (n_remain > delta)
        {
            if (overhead >= 0x40000000)
                break;
            n_remain -= delta;
            if (n_remain <= delta)
                points_by_group = n_remain * (long long)ppd / delta;
            else
                points_by_group = ppd;
            points_total += points_by_group / overhead;
            overhead *= 2;
        }
    }
    else
    {
        points_total = n_remain * (long long)ppd / delta;
    }

    // Adjust points to amount of funding
    // The integers multiplied here may go beyond 16-bit - so the result
    // may not fit 32-bit variable; use 64-bit for intermediate values
    n_remain = real_funding;
    delta = 100 * expect_funding;
    ppd = points_total;
    if (n_remain > delta)
    {
        // Our current value of points_total is as if we've already
        // passed the iteration for overhead=1; so start with 2
        overhead = 2;
        while (n_remain > delta)
        {
            if (overhead >= 0x40000000)
                break;
            n_remain -= delta;
            if (n_remain <= delta)
                points_by_group = n_remain * (long long)ppd / delta;
            else
                points_by_group = ppd;
            points_total += points_by_group / overhead;
            overhead *= 2;
        }
    }
    else
    {
        points_total = n_remain * (long long)ppd / delta;
    }
    return points_total;
}

int research_daily_progress_for_type(ubyte rstype)
{
    int real_funding;
    short progress;
    struct ModDef *mdef;
    struct WeaponDef *wdef;
    int scientists_died;

    real_funding = 0;
    progress = 0;

    switch (rstype)
    {
    case 0:
        if (research.CurrentWeapon == -1)
            break;
        real_funding = research.WeaponFunding;
        if (ingame.Credits < real_funding)
            real_funding = ingame.Credits;
        wdef = &weapon_defs[research.CurrentWeapon + 1];
        progress = research_unkn_func_004(wdef->PercentPerDay, wdef->Funding, real_funding);
        research_wep_store_daily_progress(progress);
        if (research_wep_get_progress(research.CurrentWeapon) < RESEARCH_COMPLETE_POINTS)
        {
            if (research.WeaponFunding && research.Scientists && research.NumBases)
                research.WeaponDaysDone[research.CurrentWeapon]++;
        }
        else
        {
            research_current_weapon_complete();
            if (ingame.AutoResearch)
                research_wep_next_type();
        }
        break;

    case 1:
        if (research.CurrentMod == -1)
            break;
        real_funding = research.ModFunding;
        if (ingame.Credits < real_funding)
            real_funding = ingame.Credits;
        mdef = &mod_defs[research.CurrentMod + 1];
        progress = research_unkn_func_004(mdef->PercentPerDay, mdef->Funding, real_funding);
        research_cymod_store_daily_progress(progress);
        if (research_cymod_get_progress(research.CurrentMod) < RESEARCH_COMPLETE_POINTS)
        {
            if (research.ModFunding && research.Scientists && research.NumBases)
                research.ModDaysDone[research.CurrentMod]++;
        }
        else
        {
            research_current_cymod_complete();
            if (ingame.AutoResearch)
                research_cymod_next_type();
        }
        break;

    default:
        break;
    }
    scientists_died = 0;
    if ((real_funding > 0) && (progress > 0))
    {
        ingame.Credits -= real_funding;
        ingame.Expenditure += real_funding;
        // Each day there is a chance of a scientist dying
        if (LbRandomAnyShort() % 1000 < daily_scientist_death_chance_permil)
        {
            scientists_died++;
            if (LbRandomAnyShort() & 1)
                scientists_died++;
            if (scientists_died > research.Scientists)
                scientists_died = research.Scientists;
            research.Scientists -= scientists_died;
        }
    }
    return scientists_died;
}

/******************************************************************************/
