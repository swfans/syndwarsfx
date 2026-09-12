/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file cybmod.c
 *     Agent mods related functions.
 * @par Purpose:
 *     Implement functions for handling cybernetic modifications.
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
#include "cybmod.h"

#include <string.h>
#include "bfmemory.h"
#include "bffile.h"
#include "bfini.h"

#include "game_data.h"
#include "swlog.h"
/******************************************************************************/
struct ModDef mod_defs[] = {
    {0, 0,   0, 0, {0}, 0,  0,    0,  0, 0},
    {3, 2,  30, 0, {0}, 0, 16,  300, 10, 0},
    {5, 5, 120, 0, {0}, 0, 17, 1200, 10, 0},
    {8, 8, 300, 0, {0}, 0, 19, 3000, 10, 0},
    {2, 2,  25, 0, {0}, 0, 20,  250, 10, 0},
    {4, 3, 100, 0, {0}, 0, 24, 1000, 10, 0},
    {7, 6, 250, 0, {0}, 0, 21, 2500, 10, 0},
    {3, 3,  50, 0, {0}, 0, 15,  500, 10, 0},
    {5, 6, 200, 0, {0}, 0, 23, 2000, 10, 0},
    {8, 8, 500, 0, {0}, 0, 29, 5000, 10, 0},
    {1, 2,  45, 0, {0}, 0, 33,  450, 10, 0},
    {2, 3, 180, 0, {0}, 0, 30, 1800, 10, 0},
    {4, 6, 450, 0, {0}, 0, 27, 4500, 10, 0},
    {1, 5, 200, 0, {0}, 1, 25, 2000, 10, 0},
    {1, 4, 350, 0, {0}, 2, 25, 3500, 10, 0},
    {1, 7, 600, 0, {0}, 3, 28, 6000, 10, 0},
    {1, 2, 950, 0, {0}, 4, 35, 9500, 10, 0},
};

ubyte mod_tech_level[17] = {
  0, 1, 3, 6, 1, 3, 6, 1, 3, 6, 1, 3, 6, 2, 4, 6, 8,
};

ushort mod_group_type_strid[] = {74, 71, 72, 70, 73, };

struct ModDefAdd mod_defs_a[33] = {0};
struct TbNamedEnum mod_names[33] = {0};

enum CybModsConfigCmd {
    CCMod_ModsCount = 1,
    CCMod_Name,
    CCMod_PowerOutput,
    CCMod_Resilience,
    CCMod_AdditProp,
    CCMod_Sprite,
    CCMod_Cost,
    CCMod_Funding,
    CCMod_PercentPerDay,
};

const struct TbNamedEnum cybmods_conf_common_cmds[] = {
  {"ModsCount",	CCMod_ModsCount},
  {NULL,		0},
};

const struct TbNamedEnum cybmods_conf_mod_cmds[] = {
  {"Name",			CCMod_Name},
  {"PowerOutput",	CCMod_PowerOutput},
  {"Resilience",	CCMod_Resilience},
  {"AdditProp",		CCMod_AdditProp},
  {"Sprite",		CCMod_Sprite},
  {"Cost",			CCMod_Cost},
  {"ResearchFunding",		CCMod_Funding},
  {"ResearchPercentPerDay",	CCMod_PercentPerDay},
  {NULL,		0},
};

void read_cybmods_conf_file(void)
{
    char conf_fname[DISKPATH_SIZE];
    PathInfo *pinfo;
    char *conf_buf;
    TbFileHandle conf_fh;
    TbBool done;
    int i;
    long k;
    int cmd_num;
    struct TbIniParser parser;
    int conf_len;
    int mods_count, mtype;

    pinfo = &game_dirs[DirPlace_Config];
    snprintf(conf_fname, DISKPATH_SIZE-1, "%s/cybmods.ini", pinfo->directory);
    conf_fh = LbFileOpen(conf_fname, Lb_FILE_MODE_READ_ONLY);
    if (conf_fh != INVALID_FILE) {
        conf_len = LbFileLengthHandle(conf_fh);
        if (conf_len > 1024*1024)
            conf_len = 1024*1024;
        conf_buf = LbMemoryAlloc(conf_len+16);
        conf_len = LbFileRead(conf_fh, conf_buf, conf_len);
        LOGSYNC("Processing %s file, %d bytes", conf_fname, conf_len);
        LbFileClose(conf_fh);
    } else {
        LOGERR("Could not open cyb mods config file, going with defaults.");
        conf_buf = LbMemoryAlloc(16);
        conf_len = 0;
    }
    conf_buf[conf_len] = '\0';
    LbIniParseStart(&parser, conf_buf, conf_len);
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
    mods_count = 0;
    // Parse the [common] section of loaded file
    if (LbIniFindSection(&parser, "common") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section, file skipped.", "common");
        LbIniParseEnd(&parser);
        LbMemoryFree(conf_buf);
        return;
    }
    done = false;
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(cybmods_conf_common_cmds,cmd_num)
    while (!done)
    {
        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, cybmods_conf_common_cmds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case CCMod_ModsCount:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            mods_count = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), mods_count);
            break;
        case 0: // comment
            break;
        case -1: // end of buffer
        case -3: // end of section
            done = true;
            break;
        default:
            CONFWRNLOG("Unrecognized command.");
            break;
        }
        LbIniSkipToNextLine(&parser);
    }
#undef COMMAND_TEXT
    for (mtype = 0; mtype < mods_count; mtype++)
    {
        char sect_name[16];
        struct ModDef *mdef;
        struct ModDefAdd *mdefa;

        // Parse the [modN] sections of loaded file
        sprintf(sect_name, "mod%d", mtype);
        mdef = &mod_defs[mtype];
        mdefa = &mod_defs_a[mtype];
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFWRNLOG("Could not find \"[%s]\" section.", sect_name);
            continue;
        }
        done = false;
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(cybmods_conf_mod_cmds,cmd_num)
        while (!done)
        {
            // Finding command number in this line
            i = 0;
            cmd_num = LbIniRecognizeKey(&parser, cybmods_conf_mod_cmds);
            // Now store the config item in correct place
            switch (cmd_num)
            {
            case CCMod_Name:
                i = LbIniValueGetStrWord(&parser, mdefa->Name, sizeof(mdefa->Name));
                if (i <= 0) {
                    CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)mdefa->Name);
                break;
            case CCMod_PowerOutput:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->PowerOutput = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->PowerOutput);
                break;
            case CCMod_Resilience:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->Resilience = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->Resilience);
                break;
            case CCMod_AdditProp:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->AdditProp = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->AdditProp);
                break;
            case CCMod_Sprite:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->Sprite = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->Sprite);
                break;
            case CCMod_Cost:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->Cost = k / 10;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->Cost);
                break;
            case CCMod_Funding:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->Funding = k / 100;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->Funding);
                break;
            case CCMod_PercentPerDay:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                mdef->PercentPerDay = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)mdef->PercentPerDay);
                break;
            case 0: // comment
                break;
            case -1: // end of buffer
            case -3: // end of section
                done = true;
                break;
            default:
                CONFWRNLOG("Unrecognized command.");
                break;
            }
            LbIniSkipToNextLine(&parser);
        }
#undef COMMAND_TEXT
    }
#undef CONFDBGLOG
#undef CONFWRNLOG
    LbIniParseEnd(&parser);
    LbMemoryFree(conf_buf);

    i = 0;
    for (mtype = 1; mtype < mods_count; mtype++)
    {
        struct ModDefAdd *mdefa;

        mdefa = &mod_defs_a[mtype];
        if (strlen(mdefa->Name) > 0) {
            mod_names[i].name = mdefa->Name;
            mod_names[i].num = mtype;
            i++;
        }
    }
    mod_names[i].name = NULL;
    mod_names[i].num = 0;
}

const char *cybmod_codename(ushort mtype)
{
    struct ModDefAdd *mdefa;

    if (mtype >= MOD_TYPES_COUNT)
        return "";

    mdefa = &mod_defs_a[mtype];
    return mdefa->Name;
}

ubyte cybmod_chest_level(union Mod *p_umod)
{
    return (p_umod->Mods >> 6) & 7;
}

void set_cybmod_chest_level(union Mod *p_umod, ubyte nmod)
{
    p_umod->Mods &= ~(7 << 6);
    p_umod->Mods |= (nmod << 6);
}

ubyte cybmod_legs_level(union Mod *p_umod)
{
    return (p_umod->Mods) & 7;
}

void set_cybmod_legs_level(union Mod *p_umod, ubyte nmod)
{
    p_umod->Mods &= ~(7);
    p_umod->Mods |= (nmod);
}

ubyte cybmod_arms_level(union Mod *p_umod)
{
    return (p_umod->Mods >> 3) & 7;
}

void set_cybmod_arms_level(union Mod *p_umod, ubyte nmod)
{
    p_umod->Mods &= ~(7 << 3);
    p_umod->Mods |= (nmod << 3);
}

ubyte cybmod_brain_level(union Mod *p_umod)
{
    return (p_umod->Mods >> 9) & 7;
}

void set_cybmod_brain_level(union Mod *p_umod, ubyte nmod)
{
    p_umod->Mods &= ~(7 << 9);
    p_umod->Mods |= (nmod << 9);
}

ubyte cybmod_skin_level(union Mod *p_umod)
{
    return (p_umod->Mods >> 12) & 7;
}

void set_cybmod_skin_level(union Mod *p_umod, ubyte nmod)
{
    p_umod->Mods &= ~(7 << 12);
    p_umod->Mods |= (nmod << 12);
}

TbBool cybmod_fix_all(union Mod *p_umod)
{
    TbBool fixed = false;
    if (p_umod->Mods >> 15 > 0) {
        p_umod->Mods &= ~(7 << 15);
        fixed = true;
    }
    if (cybmod_skin_level(p_umod) > 4) {
        set_cybmod_skin_level(p_umod, 1);
        fixed = true;
    }
    if (cybmod_brain_level(p_umod) > 3) {
        set_cybmod_brain_level(p_umod, 1);
        fixed = true;
    }
    if (cybmod_arms_level(p_umod) > 3) {
        set_cybmod_arms_level(p_umod, 1);
        fixed = true;
    }
    if (cybmod_legs_level(p_umod) > 3) {
        set_cybmod_legs_level(p_umod, 1);
        fixed = true;
    }
    if (cybmod_chest_level(p_umod) > 3) {
        set_cybmod_chest_level(p_umod, 1);
        fixed = true;
    }
    return fixed;
}

ubyte cybmod_level(union Mod *p_umod, ubyte mgroup)
{
    ubyte lv;

    switch (mgroup)
    {
    case MODGRP_LEGS:
        lv = cybmod_legs_level(p_umod);
        break;
    case MODGRP_ARMS:
        lv = cybmod_arms_level(p_umod);
        break;
    case MODGRP_CHEST:
        lv = cybmod_chest_level(p_umod);
        break;
    case MODGRP_BRAIN:
        lv = cybmod_brain_level(p_umod);
        break;
    case MODGRP_EPIDERM:
        lv = cybmod_skin_level(p_umod);
        break;
    default:
        lv = 0;
        break;
    }
    return lv;
}

TbBool check_mod_allowed_to_flags(union Mod *p_umod, ushort mtype)
{
    ushort mgroup;

    mgroup = cybmod_group_type(mtype);

    switch (mgroup)
    {
    case MODGRP_LEGS:
        if (cybmod_legs_level(p_umod) == cybmod_version(mtype))
            return false;
        if (cybmod_version(mtype) - (int)cybmod_chest_level(p_umod) > 1)
            return false;
        if (cybmod_chest_level(p_umod) < 1)
            return false;
        break;
    case MODGRP_ARMS:
        if (cybmod_arms_level(p_umod) == cybmod_version(mtype))
            return false;
        if (cybmod_version(mtype) - (int)cybmod_chest_level(p_umod) > 1)
            return false;
        if (cybmod_chest_level(p_umod) < 1)
            return false;
        break;
    case MODGRP_CHEST:
        if (cybmod_chest_level(p_umod) == cybmod_version(mtype))
            return false;
        if (cybmod_legs_level(p_umod) - (int)cybmod_version(mtype) > 1)
            return false;
        if (cybmod_arms_level(p_umod) - (int)cybmod_version(mtype) > 1)
            return false;
        if (cybmod_brain_level(p_umod) - (int)cybmod_version(mtype) > 1)
            return false;
        break;
    case MODGRP_BRAIN:
        if (cybmod_brain_level(p_umod) == cybmod_version(mtype))
            return false;
        if (cybmod_version(mtype) - (int)cybmod_chest_level(p_umod) > 1)
            return false;
        if (cybmod_chest_level(p_umod) < 1)
            return false;
        break;
    case MODGRP_EPIDERM:
        if (cybmod_skin_level(p_umod) == cybmod_version(mtype))
            return false;
        break;
    }
    return true;
}

void add_mod_to_flags(union Mod *p_umod, ushort mtype)
{
    switch (cybmod_group_type(mtype))
    {
    case MODGRP_LEGS:
        set_cybmod_legs_level(p_umod, cybmod_version(mtype));
        break;
    case MODGRP_ARMS:
        set_cybmod_arms_level(p_umod, cybmod_version(mtype));
        break;
    case MODGRP_CHEST:
        set_cybmod_chest_level(p_umod, cybmod_version(mtype));
        break;
    case MODGRP_BRAIN:
        set_cybmod_brain_level(p_umod, cybmod_version(mtype));
        break;
    case MODGRP_EPIDERM:
        set_cybmod_skin_level(p_umod, cybmod_version(mtype));
        break;
    }
}

TbBool add_mod_to_flags_no_replace(union Mod *p_umod, ushort mtype)
{
    switch (cybmod_group_type(mtype))
    {
    case MODGRP_LEGS:
        if (cybmod_legs_level(p_umod) == 0) {
            set_cybmod_legs_level(p_umod, cybmod_version(mtype));
            return true;
        }
        break;
    case MODGRP_ARMS:
        if (cybmod_arms_level(p_umod) == 0) {
            set_cybmod_arms_level(p_umod, cybmod_version(mtype));
            return true;
        }
        break;
    case MODGRP_CHEST:
        if (cybmod_chest_level(p_umod) == 0) {
            set_cybmod_chest_level(p_umod, cybmod_version(mtype));
            return true;
        }
        break;
    case MODGRP_BRAIN:
        if (cybmod_brain_level(p_umod) == 0) {
            set_cybmod_brain_level(p_umod, cybmod_version(mtype));
            return true;
        }
        break;
    case MODGRP_EPIDERM:
        if (cybmod_skin_level(p_umod) == 0) {
            set_cybmod_skin_level(p_umod, cybmod_version(mtype));
            return true;
        }
        break;
    }
    return false;
}

/** Converts flag-per-mod to  3bit-per-mod-type.
 */
void sanitize_cybmods_fmtver11_flags(union Mod *p_umod)
{
    ushort modflg, f;

    modflg = p_umod->Mods;

    f = modflg & 0x0007;
    if (f & 0x04)
        set_cybmod_legs_level(p_umod, 3);
    else if (f & 0x02)
        set_cybmod_legs_level(p_umod, 2);
    else if (f & 0x01)
        set_cybmod_legs_level(p_umod, 1);
    else
        set_cybmod_legs_level(p_umod, 0);

    f = modflg & 0x0038;
    if (f & 0x20)
        set_cybmod_arms_level(p_umod, 3);
    else if (f & 0x10)
        set_cybmod_arms_level(p_umod, 2);
    else if (f & 0x08)
        set_cybmod_arms_level(p_umod, 1);
    else
        set_cybmod_arms_level(p_umod, 0);

    f = modflg & 0x01C0;
    if (f & 0x0100)
        set_cybmod_chest_level(p_umod, 3);
    else if (f & 0x0080)
        set_cybmod_chest_level(p_umod, 2);
    else if (f & 0x0040)
        set_cybmod_chest_level(p_umod, 1);
    else
        set_cybmod_chest_level(p_umod, 0);

    f = modflg & 0x0E00;
    if (f & 0x0800)
        set_cybmod_brain_level(p_umod, 3);
    else if (f & 0x0400)
        set_cybmod_brain_level(p_umod, 2);
    else if (f & 0x0200)
        set_cybmod_brain_level(p_umod, 1);
    else
        set_cybmod_brain_level(p_umod, 0);

    f = modflg & 0xF000;
    if (f & 0x8000)
        set_cybmod_skin_level(p_umod, 4);
    else if (f & 0x4000)
        set_cybmod_skin_level(p_umod, 3);
    else if (f & 0x2000)
        set_cybmod_skin_level(p_umod, 2);
    else if (f & 0x1000)
        set_cybmod_skin_level(p_umod, 1);
    else
        set_cybmod_skin_level(p_umod, 0);
}

/** Returns cyborg mod set in given flags with index below last.
 *
 * Note that it only works for mods stored as flag-per-mod. The
 * usual way of storage within people is 3bit-per-mod-type.
 */
ushort cybmodflags_prev_mod(ulong modflags, ushort last_mtype)
{
    ushort mtype;

    if (last_mtype < 2)
        return 0;

    for (mtype = last_mtype - 1; mtype > 0; mtype--)
    {
        ulong modflg = 1 << (mtype-1);
        if ((modflags & modflg) != 0)
            return mtype;
    }
    return 0;
}

/** Returns mod group type for given mod type.
 */
ushort cybmod_group_type(ushort mtype)
{
    if (mtype == MOD_NULL)
        return 0;
    if (mtype >= MOD_EPIDERM1)
        return 4;
    return ((mtype-1) / 3);
}

/** Returns mod version for given mod type.
 */
ushort cybmod_version(ushort mtype)
{
    if (mtype == MOD_NULL)
        return 0;
    if (mtype >= MOD_EPIDERM1)
        return mtype - MOD_EPIDERM1 + 1;
    return ((mtype-1) % 3) + 1;
}

ushort cybmod_type(ushort mgroup, ushort mver)
{
    ushort mtype;
    // the mver is already counted from 1, so no +1 needed
    mtype = (mgroup * 3) + mver;
    return mtype;
}

/******************************************************************************/
