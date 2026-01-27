/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file weapon.c
 *     Weapons related functions.
 * @par Purpose:
 *     Implement functions for handling weapons.
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
#include "weapon.h"

#include "bfmath.h"
#include "bfmemory.h"
#include "bfendian.h"
#include "bffile.h"
#include "bfini.h"
#include "bfutility.h"
#include "ssampply.h"
#include <assert.h>
#include <stdlib.h>

#include "bigmap.h"
#include "bmbang.h"
#include "building.h"
#include "enginsngtxtr.h"
#include "enginzoom.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_data.h"
#include "game_speed.h"
#include "matrix.h"
#include "network.h"
#include "thing.h"
#include "player.h"
#include "research.h"
#include "thing_search.h"
#include "wadfile.h"
#include "sound.h"
#include "swlog.h"
#include "vehicle.h"
/******************************************************************************/

struct WeaponDef weapon_defs[] = {
    { 0,    0,  0,  0,   0,  0, 0, 0, WEPDFLG_None,          0, 0,     0,     0,  0},
    { 5,   50,  4,  5,   8, 10, 1, 1, WEPDFLG_CanPurchease, 16, 1,    40,    40, 10},
    { 8,  100,  3,  8,  20, 10, 1, 1, WEPDFLG_CanPurchease, 17, 1,   120,   120, 10},
    { 9,  200,  5,  8,   2, 10, 2, 2, WEPDFLG_CanPurchease|0x01|0x02, 19, 7,   370,   370, 10},
    { 9,  200,  5,  8,   2, 10, 2, 2, WEPDFLG_CanPurchease|0x01|0x02, 20, 7,   420,   420, 10},
    {12,  800, 30,  8, 300, 10, 1, 1, WEPDFLG_CanPurchease,  24, 1,   750,   750, 10},
    { 6,  800, 40,  8, 600, 10, 1, 1, WEPDFLG_CanPurchease,  21, 3,  1000,  1000, 10},
    { 1,    0, 10,  8,   2, 10, 1, 1, WEPDFLG_CanPurchease,  15, 9,   150,   150, 10},
    { 4,   80,  1, 16,   6,  5, 0, 0, WEPDFLG_CanPurchease,  23, 2,   160,   160, 10},
    { 0, 9999,  3, 16,   4,  5, 0, 0, WEPDFLG_CanPurchease,  29, 7,   200,   200, 10},
    { 6,  800, 40,  8, 600, 10, 0, 0, WEPDFLG_CanPurchease,  33, 2,   100,   100, 10},
    { 6,  800, 40,  8, 600, 10, 0, 0, WEPDFLG_CanPurchease,  30, 2,    80,    80, 10},
    { 4,  300,  5, 16,   4,  5, 0, 0, WEPDFLG_CanPurchease,  25, 7,   620,   620, 10},
    { 4, 9999, 40, 16, 200,  5, 0, 0, WEPDFLG_CanPurchease,  27, 1,   480,   480, 10},
    { 4, 9999,  5, 16,   4,  5, 0, 0, WEPDFLG_None,          25, 2,   400,   400, 10},
    {16, 1300, 80,  8, 200, 10, 1, 1, WEPDFLG_CanPurchease,  28, 1,   300,   300, 10},
    { 4, 9999,  5, 16,   4,  5, 0, 0, WEPDFLG_CanPurchease,  35, 3,  1050,  1050, 10},
    {10,  550,  2, 16,   4,  5, 0, 0, WEPDFLG_CanPurchease,  26,10,  2000,  2000, 10},
    {10,   50,  1,  8,   4,  5, 0, 0, WEPDFLG_CanPurchease,  18, 1,   100,   100, 10},
    { 4, 9999,  1, 16,   4,  5, 0, 0, WEPDFLG_None,          24, 6,   850,   850, 10},
    {11, 1500, 10, 16,   4,  5, 0, 0, WEPDFLG_CanPurchease,  38, 6, 65535, 65535, 10},
    { 2,    0, 10,  8,   2, 10, 1, 1, WEPDFLG_CanPurchease,  41, 9,   450,   450, 10},
    { 8, 9999, 20, 16, 200,  5, 0, 0, WEPDFLG_CanPurchease,  39, 6,  1100,  1100, 10},
    { 0, 9999,  1, 16,   4,  5, 0, 0, WEPDFLG_None,          14,10,    80,    80, 10},
    { 4, 9999,  1, 16, 200,  5, 0, 0, WEPDFLG_CanPurchease,  37, 6,  1200,  1200, 10},
    { 8,   25, 80, 16, 600,  5, 0, 0, WEPDFLG_CanPurchease,  40, 6,  1300,  1300, 10},
    { 2,  500, 50, 16, 600,  5, 0, 0, WEPDFLG_CanPurchease,  36, 6,   650,   650, 10},
    { 1, 9999,  1, 16,  60,  5, 0, 0, WEPDFLG_CanPurchease,  22, 5,    30,    30, 10},
    { 1, 9999,  1, 16,  60,  5, 0, 0, WEPDFLG_CanPurchease,  31, 5,   130,   130, 10},
    {10,   50,  1,  8,   4,  5, 0, 0, WEPDFLG_CanPurchease,  32, 1,   950,   950, 10},
    { 4, 9999,  1, 16,   4,  5, 0, 0, WEPDFLG_CanPurchease,  34, 5,  1000,  1000, 10},
    { 0,    0,  0, 16,   4,  5, 0, 0, WEPDFLG_None,          72, 5,  3000,    10,  8},
    { 0,    0,  0, 16,   4,  5, 0, 0, WEPDFLG_None,          72, 5,  3000,    10,  8},
};

/** Tank rocket weapon settings.
 */
ushort weapon_tank_rocket_ReFireDelay = 20;

ubyte weapon_tech_level[33] = {
  0, 1, 1, 3, 3, 5, 6, 2, 4, 3, 3, 2, 4, 4, 255, 5, 7, 8, 1, 255, 9, 6, 6, 255, 8, 7, 5, 2, 6, 7, 5, 255, 255,
};

struct WeaponDefAdd weapon_defs_a[33] = {0};
struct TbNamedEnum weapon_names[33] = {0};

short persuaded_person_weapons_sell_cost_permil = 0;

short low_energy_alarm_level = 50;

enum WeaponsConfigCmd {
    CCWep_WeaponsCount = 1,
    CCWep_Name,
    CCWep_Category,
    CCWep_RangeBlocks,
    CCWep_HitDamage,
    CCWep_ReFireDelay,
    CCWep_Weight,
    CCWep_EnergyUsed,
    CCWep_Accuracy,
    CCWep_TargetType,
    CCWep_TargetMode,
    CCWep_Sprite,
    CCWep_Cost,
    CCWep_Funding,
    CCWep_PercentPerDay,
};

const struct TbNamedEnum weapons_conf_common_cmds[] = {
  {"WeaponsCount",	CCWep_WeaponsCount},
  {NULL,		0},
};

const struct TbNamedEnum weapons_conf_weapon_cmds[] = {
  {"Name",			CCWep_Name},
  {"Category",	    CCWep_Category},
  {"RangeBlocks",	CCWep_RangeBlocks},
  {"HitDamage",		CCWep_HitDamage},
  {"ReFireDelay",	CCWep_ReFireDelay},
  {"Weight",		CCWep_Weight},
  {"EnergyUsed",	CCWep_EnergyUsed},
  {"Accuracy",		CCWep_Accuracy},
  {"TargetType",	CCWep_TargetType},
  {"TargetMode",	CCWep_TargetMode},
  {"Sprite",		CCWep_Sprite},
  {"Cost",			CCWep_Cost},
  {"ResearchFunding",		CCWep_Funding},
  {"ResearchPercentPerDay",	CCWep_PercentPerDay},
  {NULL,		0},
};

extern ubyte byte_1DD8F8;

/******************************************************************************/

void read_weapons_conf_file(void)
{
    char conf_fname[DISKPATH_SIZE];
    PathInfo *pinfo;
    TbFileHandle conf_fh;
    TbBool done;
    int i;
    long k;
    int cmd_num;
    char *conf_buf;
    struct TbIniParser parser;
    int conf_len;
    WeaponType weapons_count, wtype;

    pinfo = &game_dirs[DirPlace_Config];
    snprintf(conf_fname, DISKPATH_SIZE-1, "%s/weapons.ini", pinfo->directory);
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
        LOGERR("Could not open weapons config file, going with defaults.");
        conf_buf = LbMemoryAlloc(16);
        conf_len = 0;
    }
    conf_buf[conf_len] = '\0';
    LbIniParseStart(&parser, conf_buf, conf_len);
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
    weapons_count = 0;
    // Parse the [common] section of loaded file
    if (LbIniFindSection(&parser, "common") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section, file skipped.", "common");
        LbIniParseEnd(&parser);
        LbMemoryFree(conf_buf);
        return;
    }
    done = false;
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(weapons_conf_common_cmds,cmd_num)
    while (!done)
    {
        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, weapons_conf_common_cmds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case CCWep_WeaponsCount:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            weapons_count = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), weapons_count);
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

    for (wtype = 0; wtype < weapons_count; wtype++)
    {
        char sect_name[32];
        struct WeaponDef *wdef;
        struct WeaponDefAdd *wdefa;

        // Parse the [weaponN] sections of loaded file
        sprintf(sect_name, "weapon%d", (int)wtype);
        wdef = &weapon_defs[wtype];
        wdefa = &weapon_defs_a[wtype];
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFWRNLOG("Could not find \"[%s]\" section.", sect_name);
            continue;
        }
        done = false;
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(weapons_conf_weapon_cmds,cmd_num)
        while (!done)
        {
            // Finding command number in this line
            i = 0;
            cmd_num = LbIniRecognizeKey(&parser, weapons_conf_weapon_cmds);
            // Now store the config item in correct place
            switch (cmd_num)
            {
            case CCWep_Name:
                i = LbIniValueGetStrWord(&parser, wdefa->Name, sizeof(wdefa->Name));
                if (i <= 0) {
                    CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)wdefa->Name);
                break;
            case CCWep_Category:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                if ((k < 0) || (k > WEP_CATEGORIES_COUNT)) {
                    CONFWRNLOG("Outranged value of \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    k = WEP_CATEGORIES_COUNT;
                }
                wdef->Category = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(wdef->Category));
                break;
            case CCWep_RangeBlocks:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                if ((k < 0) || (k > WEAPON_RANGE_BLOCKS_LIMIT)) {
                    CONFWRNLOG("Outranged value of \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    k = WEAPON_RANGE_BLOCKS_LIMIT;
                }
                wdef->RangeBlocks = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->RangeBlocks);
                break;
            case CCWep_HitDamage:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->HitDamage = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->HitDamage);
                break;
            case CCWep_ReFireDelay:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->ReFireDelay = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->ReFireDelay);
                break;
            case CCWep_Weight:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->Weight = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->Weight);
                break;
            case CCWep_EnergyUsed:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->EnergyUsed = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->EnergyUsed);
                break;
            case CCWep_Accuracy:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->Accuracy = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->Accuracy);
                break;
            case CCWep_TargetType:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->TargetType = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->TargetType);
                break;
            case CCWep_TargetMode:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->TargetMode = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->TargetMode);
                break;
            case CCWep_Sprite:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->Sprite = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)(wdef->Sprite));
                break;
            case CCWep_Cost:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->Cost = k / 100;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->Cost);
                break;
            case CCWep_Funding:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->Funding = k / 100;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->Funding);
                break;
            case CCWep_PercentPerDay:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                wdef->PercentPerDay = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)wdef->PercentPerDay);
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
    for (wtype = WEP_NULL + 1; wtype < weapons_count; wtype++)
    {
        struct WeaponDefAdd *wdefa;

        wdefa = &weapon_defs_a[wtype];
        if (strlen(wdefa->Name) > 0) {
            weapon_names[i].name = wdefa->Name;
            weapon_names[i].num = wtype;
            i++;
        }
    }
    weapon_names[i].name = NULL;
    weapon_names[i].num = 0;
}

const char *weapon_codename(WeaponType wtype)
{
    struct WeaponDefAdd *wdefa;

    if (wtype >= WEP_TYPES_COUNT)
        return "";

    wdefa = &weapon_defs_a[wtype];
    return wdefa->Name;
}

void init_weapon_text(void)
{
    char locstr[512];
    int weptxt_pos;
    int totlen;
    char *s;
    int i, n;

    totlen = load_file_alltext("textdata/wms.txt", weapon_text);
    if (totlen == Lb_FAIL)
        return;
    if (totlen >= weapon_text_len) {
        LOGERR("Insufficient memory for weapon_text - %d instead of %d", weapon_text_len, totlen);
        totlen = weapon_text_len - 1;
    }

    // TODO change the format to use our INI parser
    s = weapon_text;
    weapon_text[totlen] = '\0';

    // section_start = s;
    weptxt_pos = 0;

    s = strchr(s, '[');
    s++;
    s = strchr(s, ']'); // position at start of WEAPONS section
    s++;

    s += 2;
    while (1)
    {
        if (*s == '[')
            break;

        // Read weapon name
        n = 0;
        while ((*s != '\r') && (*s != '\n'))
        {
            locstr[n] = *s++;
            n++;
        }
        locstr[n] = '\0';
        s += 2;

        // Recognize the weapon name
        for (i = 1; i < WEP_TYPES_COUNT; i++)
        {
            const char *codename;
            codename = weapon_codename(i);
            if (strcmp(codename, locstr) == 0) {
                i--;
                break;
            }
        }
        if (i < WEP_TYPES_COUNT)
        {
            weapon_text_index[i] = weptxt_pos;

            while ((*s != '\r') && (*s != '\n')) {
                weapon_text[weptxt_pos] = *s++;
                weptxt_pos++;
            }
            weapon_text[weptxt_pos] = '\0';
            weptxt_pos++;
            s += 2;

            n = weapon_text_index[i];
            my_preprocess_text(&weapon_text[n]);
        } else {
            LOGERR("Weapon name not recognized: \"%s\"", locstr);
            if (s) s = strpbrk(s, "\r\n");
            if (s) s += 2;
        }
    }

    s = strchr(s, '[');
    s++;
    s = strchr(s, ']'); // position at start of MODS section
    s++;

    s += 2;
    while (1)
    {
        if ((*s == '[') || (*s == '\0'))
            break;

        // Read mod name
        n = 0;
        while ((*s != '\r') && (*s != '\n') && (*s != '\0'))
        {
            locstr[n] = *s++;
            n++;
        }
        locstr[n] = '\0';
        s += 2;

        for (i = 1; i < MOD_TYPES_COUNT; i++)
        {
            const char *codename;
            codename = cybmod_codename(i);
            if (strcmp(codename, locstr) == 0) {
                i--;
                break;
            }
        }
        if (i < MOD_TYPES_COUNT)
        {
            cybmod_text_index[i] = weptxt_pos;

            while ((*s != '\r') && (*s != '\n') && (*s != '\0')) {
                weapon_text[weptxt_pos] = *s++;
                weptxt_pos++;
            }
            weapon_text[weptxt_pos] = '\0';
            weptxt_pos++;
            s += 2;

            n = cybmod_text_index[i];
            my_preprocess_text(&weapon_text[n]);
        } else {
            LOGERR("Cyb Mod name not recognized: \"%s\"", locstr);
            if (s) s = strpbrk(s, "\r\n");
            if (s) s += 2;
        }
    }
}

ushort weapon_sprite_index(WeaponType wtype, TbBool enabled)
{
    ushort sprid;
    struct WeaponDef *wdef;

    if (wtype >= WEP_TYPES_COUNT)
        return 0;

    wdef = &weapon_defs[wtype];
    // Block of normal weapon sprites is 15..41, lighter are 42..68.
    // Sprites added after that have normal image and ligher image adjacent.
    if (enabled) {
        if (wdef->Sprite < 42)
            sprid = 27 + wdef->Sprite;
        else
            sprid = wdef->Sprite + 1;
    } else {
        sprid = 0 + wdef->Sprite;
    }
    return sprid;
}

TbBool weapon_is_for_throwing(WeaponType wtype)
{
    return (wtype == WEP_NUCLGREN) || (wtype == WEP_CRAZYGAS) || (wtype == WEP_KOGAS);
}

TbBool weapon_is_for_planting(WeaponType wtype)
{
    return (wtype == WEP_ELEMINE) || (wtype == WEP_EXPLMINE);
}

TbBool weapon_is_for_deploying(WeaponType wtype)
{
    return (wtype == WEP_AIRSTRIKE) || (wtype == WEP_CEREBUSIFF);
}

TbBool weapon_is_for_spreading_on_ground(WeaponType wtype)
{
    return (wtype == WEP_RAZORWIRE) || (wtype == WEP_EXPLWIRE);
}

TbBool weapon_is_deployed_at_wielder_pos(WeaponType wtype)
{
    return weapon_is_for_planting(wtype) ||
      weapon_is_for_deploying(wtype) ||
      weapon_is_for_spreading_on_ground(wtype);
}

TbBool weapon_is_breaking_will(WeaponType wtype)
{
    return (wtype == WEP_PERSUADRTRN) || (wtype == WEP_PERSUADER2);
}

TbBool weapon_is_self_affecting(WeaponType wtype)
{
    return (wtype == WEP_MEDI1) || (wtype == WEP_MEDI2) || (wtype == WEP_CLONESHLD);
}

ushort weapon_fourpack_index(WeaponType wtype)
{
    switch (wtype)
    {
    case WEP_NUCLGREN:
        return WFRPK_NUCLGREN;
    case WEP_ELEMINE:
        return WFRPK_ELEMINE;
    case WEP_EXPLMINE:
        return WFRPK_EXPLMINE;
    case WEP_KOGAS:
        return WFRPK_KOGAS;
    case WEP_CRAZYGAS:
        return WFRPK_CRAZYGAS;
    }
    return WFRPK_COUNT;
}

TbBool weapon_is_consumable(WeaponType wtype)
{
    ushort fp;

    if ((wtype == WEP_AIRSTRIKE) || (wtype == WEP_CEREBUSIFF)
      || (wtype == WEP_MEDI1) || (wtype == WEP_MEDI2))
        return true;

    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT)
        return true;

    return false;
}

TbBool weapon_has_targetting(WeaponType wtype)
{
    return (wtype == WEP_RAP);
}

TbBool weapon_can_be_charged(WeaponType wtype)
{
    return (wtype == WEP_LASER) || (wtype == WEP_ELLASER)
      || (wtype == WEP_BEAM) || (wtype == WEP_QDEVASTATOR);
}

TbBool weapons_has_weapon(ulong weapons, WeaponType wtype)
{
    ulong wepflg = 1 << (wtype-1);
    return (weapons & wepflg) != 0;
}

ushort weapons_prev_weapon(ulong weapons, WeaponType last_wtype)
{
    WeaponType wtype;

    if (last_wtype < 2)
        return 0;

    for (wtype = last_wtype - 1; wtype > WEP_NULL; wtype--)
    {
        ulong wepflg = 1 << (wtype-1);
        if ((weapons & wepflg) != 0)
            return wtype;
    }
    return 0;
}

ushort weapons_count_used_slots(ulong weapons)
{
    ushort used_slots;
    WeaponType wtype;

    used_slots = 0;
    for (wtype = WEP_NULL + 1; wtype < WEP_TYPES_COUNT; wtype++)
    {
        if (weapons_has_weapon(weapons, wtype))
            used_slots++;
    }
    return used_slots;
}

void weapons_remove_weapon(ulong *p_weapons, struct WeaponsFourPack *p_fourpacks, WeaponType wtype)
{
    ushort fp;

    *p_weapons &= ~(1 << (wtype-1));

    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT)
        p_fourpacks->Amount[fp] = 0;
}

TbBool weapons_remove_one_from_npc(ulong *p_weapons, WeaponType wtype)
{
    ushort fp;
    TbBool was_last;

    if ((*p_weapons & (1 << (wtype-1))) == 0)
        return false;

    was_last = true;
    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT) {
        was_last = false;
    }
    if (was_last)
        *p_weapons &= ~(1 << (wtype-1));
    return true;
}

TbBool weapons_remove_one(ulong *p_weapons, struct WeaponsFourPack *p_fourpacks, WeaponType wtype)
{
    ushort fp;
    TbBool was_last;

    if ((*p_weapons & (1 << (wtype-1))) == 0)
        return false;

    was_last = true;
    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT) {
        was_last = (p_fourpacks->Amount[fp] <= 1);
        p_fourpacks->Amount[fp]--;
    }
    if (was_last)
        *p_weapons &= ~(1 << (wtype-1));
    return true;
}

/** Remove one weapon from player-controlled person in-game.
 * Player struct contains dumb own array rather than uniform WeaponsFourPack, so it requires
 * this special function. To be removed when possible.
 */
TbBool weapons_remove_one_for_player(ulong *p_weapons,
  ubyte p_plfourpacks[][4], ushort plagent, WeaponType wtype)
{
    ushort fp;
    TbBool was_last;

    if ((*p_weapons & (1 << (wtype-1))) == 0)
        return false;

    was_last = true;
    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT) {
        was_last = (p_plfourpacks[fp][plagent] <= 1);
        p_plfourpacks[fp][plagent]--;
    }
    if (was_last)
        *p_weapons &= ~(1 << (wtype-1));
    return true;
}

TbBool weapons_add_one(ulong *p_weapons, struct WeaponsFourPack *p_fourpacks, WeaponType wtype)
{
    ushort fp;
    TbBool is_first;

    is_first = ((*p_weapons & (1 << (wtype-1))) == 0);

    if ((is_first) && (weapons_count_used_slots(*p_weapons) >= WEAPONS_CARRIED_MAX_COUNT))
        return false;

    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT) {
        if ((!is_first) && (p_fourpacks->Amount[fp] >= WEAPONS_FOURPACK_MAX_COUNT))
            return false;

        if (is_first)
            p_fourpacks->Amount[fp] = 1;
        else
            p_fourpacks->Amount[fp]++;
    } else {
        if (!is_first)
            return false;
    }

    if (is_first)
        *p_weapons |= (1 << (wtype-1));

    return true;
}

/** Add one weapon to player-controlled person in-game.
 * Player struct contains dumb own array rather than uniform WeaponsFourPack, so it requires
 * this special function. To be removed when possible.
 */
TbBool weapons_add_one_for_player(ulong *p_weapons,
  ubyte p_plfourpacks[][4], ushort plagent, WeaponType wtype)
{
    ushort fp;
    TbBool is_first;

    is_first = ((*p_weapons & (1 << (wtype-1))) == 0);

    if ((is_first) && (weapons_count_used_slots(*p_weapons) >= WEAPONS_CARRIED_MAX_COUNT))
        return false;

    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT) {
        if ((!is_first) && (p_plfourpacks[fp][plagent] >= WEAPONS_FOURPACK_MAX_COUNT))
            return false;

        if (is_first)
            p_plfourpacks[fp][plagent] = 1;
        else
            p_plfourpacks[fp][plagent]++;
    } else {
        if (!is_first)
            return false;
    }

    if (is_first)
        *p_weapons |= (1 << (wtype-1));

    return true;
}

void sanitize_weapon_quantities(ulong *p_weapons, struct WeaponsFourPack *p_fourpacks)
{
    WeaponType wtype;
    ushort n_weapons;

    n_weapons = 0;
    for (wtype = WEP_TYPES_COUNT-1; wtype > 0; wtype--)
    {
        ushort fp, n;
        TbBool has_weapon;

        has_weapon = weapons_has_weapon(*p_weapons, wtype);

        if (has_weapon && (n_weapons > WEAPONS_CARRIED_MAX_COUNT)) {
            weapons_remove_weapon(p_weapons, p_fourpacks, wtype);
            has_weapon = false;
        }

        fp = weapon_fourpack_index(wtype);
        if (fp >= WFRPK_COUNT)
            continue;

        n = p_fourpacks->Amount[fp];

        if (!has_weapon)
            n = 0;
        else if (n < 1)
            n = 1;
        else if (n > 4)
            n = 4;
        p_fourpacks->Amount[fp] = n;
    }
}

short get_hand_weapon_range(struct Thing *p_person, WeaponType wtype)
{
    struct WeaponDef *wdef;
    short range;

    if (wtype >= WEP_TYPES_COUNT)
        return 0;

    wdef = &weapon_defs[wtype];

    range = TILE_TO_MAPCOORD(wdef->RangeBlocks, 0);

    if (weapon_is_for_throwing(wtype))
        range = (85 * range * (3 + cybmod_arms_level(&p_person->U.UPerson.UMod)) + range) >> 8;

    if (weapon_is_breaking_will(wtype))
        range = (85 * range * (3 + cybmod_brain_level(&p_person->U.UPerson.UMod)) + range) >> 8;

    return range;
}

int get_weapon_zoom_min(WeaponType wtype)
{
    struct WeaponDef *wdef;

    // There is no reason for those two to not be equal
    assert(WEAPON_RANGE_BLOCKS_LIMIT == ZOOM_RANGE_BLOCKS_LIMIT);
    wdef = &weapon_defs[wtype];
    return get_zoom_from_range_bloks(wdef->RangeBlocks);
}

short current_hand_weapon_range(struct Thing *p_person)
{
#if 0
    short ret;
    asm volatile ("call ASM_current_hand_weapon_range\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
#endif
    return get_hand_weapon_range(p_person, p_person->U.UPerson.CurrentWeapon);
}

int get_weapon_range(struct Thing *p_person)
{
    int ret;
    asm volatile ("call ASM_get_weapon_range\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
}

TbBool current_weapon_has_targetting(struct Thing *p_person)
{
    WeaponType wtype;

    wtype = p_person->U.UPerson.CurrentWeapon;
    if (wtype >= WEP_TYPES_COUNT)
        return false;

    return weapon_has_targetting(wtype);
}

ubyte find_nth_weapon_held(ushort index, ubyte n)
{
    char ret;
    asm volatile ("call ASM_find_nth_weapon_held\n"
        : "=r" (ret) : "a" (index), "d" (n));
    return ret;
}

ulong person_carried_weapons_pesuaded_sell_value(struct Thing *p_person)
{
    ulong credits;
    WeaponType wtype;

    credits = 0;
    for (wtype = WEP_TYPES_COUNT-1; wtype > 0; wtype--)
    {
        struct WeaponDef *wdef;

        if (!person_carries_weapon(p_person, wtype))
            continue;

        wdef = &weapon_defs[wtype];
        credits += wdef->Cost * persuaded_person_weapons_sell_cost_permil / 1000;
    }
    return credits;
}

void do_weapon_quantities_net_to_player(struct Thing *p_person)
{
    ushort plyr, plagent;
    WeaponType wtype;

    plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
    plagent = p_person->U.UPerson.ComCur & 3;

    for (wtype = WEP_TYPES_COUNT-1; wtype > 0; wtype--)
    {
        ushort fp, n;

        fp = weapon_fourpack_index(wtype);
        if (fp >= WFRPK_COUNT)
            continue;

        if (person_carries_weapon(p_person, wtype))
            n = net_agents__FourPacks[plyr][plagent].Amount[fp];
        else
            n = 0;
        players[plyr].FourPacks[fp][plagent] = n;
    }
}

void do_weapon_quantities_player_to_net(struct Thing *p_person)
{
    ushort plyr, plagent;
    WeaponType wtype;

    plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
    plagent = p_person->U.UPerson.ComCur & 3;

    for (wtype = WEP_TYPES_COUNT-1; wtype > 0; wtype--)
    {
        ushort fp, n;

        fp = weapon_fourpack_index(wtype);
        if (fp >= WFRPK_COUNT)
            continue;

        if (person_carries_weapon(p_person, wtype))
            n = players[plyr].FourPacks[fp][plagent];
        else
            n = 0;
        net_agents__FourPacks[plyr][plagent].Amount[fp] = n;
    }
}

void do_weapon_quantities_cryo_to_player(struct Thing *p_person)
{
    ushort plyr, plagent;
    WeaponType wtype;

    plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
    plagent = p_person->U.UPerson.ComCur & 3;

    for (wtype = WEP_TYPES_COUNT-1; wtype > 0; wtype--)
    {
        ushort fp, n;

        fp = weapon_fourpack_index(wtype);
        if (fp >= WFRPK_COUNT)
            continue;

        if (person_carries_weapon(p_person, wtype))
            n = cryo_agents.FourPacks[plagent].Amount[fp];
        else
            n = 0;
        players[plyr].FourPacks[fp][plagent] = n;
    }
}

void do_weapon_quantities_max_to_player(struct Thing *p_person)
{
    ushort plyr, plagent;
    WeaponType wtype;

    plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
    plagent = p_person->U.UPerson.ComCur & 3;

    for (wtype = WEP_TYPES_COUNT-1; wtype > 0; wtype--)
    {
        ushort fp, n;

        fp = weapon_fourpack_index(wtype);
        if (fp >= WFRPK_COUNT)
            continue;

        if (person_carries_weapon(p_person, wtype))
            n = WEAPONS_FOURPACK_MAX_COUNT;
        else
            n = 0;
        players[plyr].FourPacks[fp][plagent] = n;
    }
}

void player_agent_set_weapon_quantities_max(struct Thing *p_person)
{
    if ((in_network_game) || ((p_person->Flag & TngF_PlayerAgent) == 0))
    {
        // No action
    }
    else
    {
        do_weapon_quantities_max_to_player(p_person);
    }
}

void player_agent_set_weapon_quantities_proper(struct Thing *p_person)
{
    if ((p_person->Flag & TngF_PlayerAgent) == 0)
    {
        // No action - only player agents have quantities of weapons
    }
    else if (in_network_game)
    {
        do_weapon_quantities_player_to_net(p_person);
    }
    else
    {
        do_weapon_quantities_cryo_to_player(p_person);
    }
}

void elec_hit_building(int x, int y, int z, short col)
{
    asm volatile ("call ASM_elec_hit_building\n"
        : : "a" (x), "d" (y), "b" (z), "c" (col));
}

void init_shoot_recoil(struct Thing *p_person, short vx, short vy, short vz)
{
#if 1
    asm volatile ("call ASM_init_shoot_recoil\n"
        : : "a" (p_person), "d" (vx), "b" (vy), "c" (vz));
#endif
}

TbBool thing_fire_shot_start_position(struct M31 *prc_beg_pt, struct Thing *p_owner, WeaponType wtype, ushort barrel)
{
    switch (p_owner->Type)
    {
    case TT_PERSON:
        if ((p_owner->Flag2 & TgF2_ExistsOffMap) != 0)
        {
            prc_beg_pt->R[0] = p_owner->X;
            prc_beg_pt->R[1] = p_owner->Y + MAPCOORD_TO_PRCCOORD(PERSON_BOTTOM_TO_WEAPON_HEIGHT, 0);
            prc_beg_pt->R[2] = p_owner->Z;
        }
        else
        {
            ubyte angl;
            angl = p_owner->U.UPerson.Angle;

            switch (wtype)
            {
            case WEP_RAP:
                prc_beg_pt->R[0] = p_owner->X + PERSON_CENTER_TO_ROCKT_WEAPON_TIP_MAPCOORD * angle_direction[angl].DiX;
                prc_beg_pt->R[1] = p_owner->Y + MAPCOORD_TO_PRCCOORD(PERSON_BOTTOM_TO_WEAPON_HEIGHT, 0);
                prc_beg_pt->R[2] = p_owner->Z + PERSON_CENTER_TO_ROCKT_WEAPON_TIP_MAPCOORD * angle_direction[angl].DiY;
                break;
            default:
                prc_beg_pt->R[0] = p_owner->X + PERSON_CENTER_TO_BEAM_WEAPON_TIP_MAPCOORD * angle_direction[angl].DiX;
                prc_beg_pt->R[1] = p_owner->Y + MAPCOORD_TO_PRCCOORD(PERSON_BOTTOM_TO_WEAPON_HEIGHT, 0);
                prc_beg_pt->R[2] = p_owner->Z + PERSON_CENTER_TO_BEAM_WEAPON_TIP_MAPCOORD * angle_direction[angl].DiY;
                break;
            }
        }
        break;
    case TT_VEHICLE:
        if (p_owner->SubType == SubTT_VEH_TANK)
        {
            struct Thing *p_mgun;
            short angl;

            p_mgun = &things[p_owner->U.UVehicle.SubThing];

            if ((barrel & 1) != 0) // which rocket pack to shoot from
                angl = p_mgun->U.UMGun.AngleY + 7 * LbFPMath_PI / 16;
            else
                angl = p_mgun->U.UMGun.AngleY - 7 * LbFPMath_PI / 16;
            angl = (angl + 2 * LbFPMath_PI) & LbFPMath_AngleMask;

            // the mounted gun position is relative; use trigonometry to switch rocket packs
            prc_beg_pt->R[0] = p_owner->X + p_mgun->X - 3 * lbSinTable[angl] / 2;
            prc_beg_pt->R[2] = p_owner->Z + p_mgun->Z - 3 * lbSinTable[angl + LbFPMath_PI/2] / 2;
            if (p_mgun->StartFrame == VehOM_MBTANK)
            {
                 // MBT has 6-rocket pack; but simplify to 2 - select upper or lower rocket within pack
                if ((barrel & 2) != 0)
                    prc_beg_pt->R[1] = p_owner->Y + p_mgun->Y + (50 << 5);
                else
                    prc_beg_pt->R[1] = p_owner->Y + p_mgun->Y + (200 << 5);
            }
            else
            {
                // Claw tank (and others) do not have multiple rockets in a pack
                prc_beg_pt->R[1] = p_owner->Y + p_mgun->Y + (170 << 5);
            }
        }
        else if (p_owner->SubType == SubTT_VEH_MECH)
        {
            assert(!"Not implemented");
        }
        else
        {
            assert(!"Vehicle subtype with no shooting ability");
        }
        break;
    case TT_BUILDING:
        // The only building with shooting capability is mounted gun
        assert(p_owner->SubType == SubTT_BLD_MGUN);
        {
            short angl;
            if (barrel != 0) // which barrel to shoot from
                angl = p_owner->U.UMGun.AngleY + 48;
            else
                angl = p_owner->U.UMGun.AngleY - 48;
            angl = (angl + 2 * LbFPMath_PI) & LbFPMath_AngleMask;
            prc_beg_pt->R[0] = p_owner->X + 3 * lbSinTable[angl] / 2;
            prc_beg_pt->R[2] = p_owner->Z - 3 * lbSinTable[angl + LbFPMath_PI/2] / 2;
            prc_beg_pt->R[1] = p_owner->Y;
        }
        break;
    default:
        prc_beg_pt->R[0] = p_owner->X;
        prc_beg_pt->R[1] = p_owner->Y + MAPCOORD_TO_PRCCOORD(PERSON_BOTTOM_TO_WEAPON_HEIGHT, 0);
        prc_beg_pt->R[2] = p_owner->Z;
        break;
    }

    if ((PRCCOORD_TO_MAPCOORD(prc_beg_pt->R[0]) < 0) || (PRCCOORD_TO_MAPCOORD(prc_beg_pt->R[0]) >= MAP_COORD_WIDTH) ||
      (PRCCOORD_TO_MAPCOORD(prc_beg_pt->R[2]) < 0) || (PRCCOORD_TO_MAPCOORD(prc_beg_pt->R[2]) >= MAP_COORD_HEIGHT)) {
        LOGERR("Start position for %s shot is beyond map area", weapon_codename(wtype));
        return false;
    }
    return true;
}

void thing_fire_shot_finish_position_straight_forward(struct M31 *prc_fin_pt, const struct M31 *prc_beg_pt, struct Thing *p_owner, WeaponType wtype)
{
    struct WeaponDef *wdef;
    ushort range;
    ubyte angl;

    wdef = &weapon_defs[wtype];
    range = wdef->RangeBlocks;
    angl = p_owner->U.UPerson.Angle;
    prc_fin_pt->R[0] = prc_beg_pt->R[0] + MAPCOORD_TO_PRCCOORD(range * angle_direction[angl].DiX, 0);
    prc_fin_pt->R[1] = prc_beg_pt->R[1];
    prc_fin_pt->R[2] = prc_beg_pt->R[2] + MAPCOORD_TO_PRCCOORD(range * angle_direction[angl].DiY, 0);
}

void init_laser(struct Thing *p_owner, ushort start_age)
{
#if 0
    asm volatile ("call ASM_init_laser\n"
        : : "a" (p_owner), "d" (start_age));
#endif
    struct Thing *p_shot;
    struct WeaponDef *wdef;
    struct M31 prc_beg_pt, prc_fin_pt;
    MapCoord cor_x, cor_y, cor_z;
    u32 rhit;
    int damage;
    ThingIdx shottng, target;
    ubyte wdmgtyp;

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }
    p_shot = &things[shottng];

    p_shot->U.UObject.Angle = p_owner->U.UPerson.Angle;

    if (!thing_fire_shot_start_position(&prc_beg_pt, p_owner, WEP_LASER, 0)) {
        remove_thing(shottng);
        return;
    }

    target = 0;
    wdef = &weapon_defs[WEP_LASER];
    wdmgtyp = DMG_LASER;

    if ((p_owner->Flag & TngF_Unkn20000000) != 0)
    {
        prc_fin_pt.R[0] = MAPCOORD_TO_PRCCOORD(p_owner->VX, 0);
        prc_fin_pt.R[1] = MAPCOORD_TO_PRCCOORD(p_owner->VY, 0);
        prc_fin_pt.R[2] = MAPCOORD_TO_PRCCOORD(p_owner->VZ, 0);
        p_owner->Flag &= ~TngF_Unkn20000000;
    }
    else if (p_owner->PTarget != NULL)
    {
        struct Thing *p_target;
        p_target = p_owner->PTarget;
        prc_fin_pt.R[0] = p_target->X;
        prc_fin_pt.R[1] = p_target->Y + MAPCOORD_TO_PRCCOORD(10, 0);
        prc_fin_pt.R[2] = p_target->Z;
        target = p_target->ThingOffset;
    }
    else if ((p_owner->Flag & TngF_Unkn1000) != 0)
    {
        thing_fire_shot_finish_position_straight_forward(&prc_fin_pt, &prc_beg_pt, p_owner, WEP_LASER);
    }
    else
    {
        remove_thing(shottng);
        return;
    }

    p_shot->X = prc_beg_pt.R[0];
    p_shot->Y = prc_beg_pt.R[1];
    p_shot->Z = prc_beg_pt.R[2];

    p_shot->VX = PRCCOORD_TO_MAPCOORD(prc_fin_pt.R[0]);
    p_shot->VY = PRCCOORD_TO_MAPCOORD(prc_fin_pt.R[1]);
    p_shot->VZ = PRCCOORD_TO_MAPCOORD(prc_fin_pt.R[2]);

    p_shot->Radius = 50;
    p_shot->Owner = p_owner->ThingOffset;

    cor_x = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]);
    cor_y = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
    cor_z = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]);
    rhit = laser_hit_at(cor_x, cor_y, cor_z, &p_shot->VX, &p_shot->VY, &p_shot->VZ, p_shot);

    if (start_age > 15)
        start_age = 15;
    if (start_age < 5)
        start_age = 5;
    // For every consecutive turn of damage, deal more of it
    damage = wdef->HitDamage + ((wdef->HitDamage * (start_age - 5)) >> 3);

    if ((rhit & 0x80000000) != 0) // hit 3D object collision vector
    {
        short hitvec;

        hitvec = rhit;
        if ((p_owner->Flag2 & TgF2_ExistsOffMap) == 0)
        {
            bul_hit_vector(p_shot->VX, p_shot->VY, p_shot->VZ, -hitvec, 4 * start_age, wdmgtyp);
            p_owner->U.UPerson.Flag3 |= 0x40;
        }
    }
    else if ((rhit & 0x20000000) != 0)
    {
        // no action
    }
    else if ((rhit & 0x40000000) != 0) // hit SimpleThing
    {
        struct SimpleThing *p_hitstng;
        ThingIdx hitstng;
        hitstng = rhit & ~0x60000000;
        p_hitstng = &sthings[-hitstng];
        person_hit_by_bullet((struct Thing *)p_hitstng, damage, p_shot->VX - cor_x,
          p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
    }
    else if (rhit != 0) // hit normal thing
    {
        struct Thing *p_hittng;
        ThingIdx hittng;
        hittng = rhit & ~0x60000000;
        p_hittng = &things[hittng];
        person_hit_by_bullet(p_hittng, damage, p_shot->VX - cor_x,
          p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
    }
    else // if did not hit anything else, go for original target
    {
        if (target != 0)
        {
            person_hit_by_bullet(&things[target], damage, p_shot->VX - cor_x,
              p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
        }
        else if ((p_owner->Flag2 & TgF2_ExistsOffMap) != 0)
        {
            p_shot->VY = p_shot->Y >> 8;
        }
    }
    p_shot->StartTimer1 = start_age;
    p_shot->Timer1 = start_age;
    p_shot->Flag = TngF_Unkn0004;
    p_shot->Type = TT_LASER11;
    add_node_thing(p_shot->ThingOffset);

    if ((byte_1DD8F8 & 1) != 0)
        p_shot->Flag |= TngF_Unkn1000;
    else
        p_shot->Flag &= ~TngF_Unkn1000;
}

void init_mgun_laser(struct Thing *p_owner, ushort start_age)
{
#if 0
    asm volatile (
      "call ASM_init_mgun_laser\n"
        : : "a" (p_owner), "d" (start_age));
    return;
#endif
    struct Thing *p_shot;
    struct WeaponDef *wdef;
    struct M31 prc_beg_pt;
    MapCoord cor_x, cor_y, cor_z;
    short tgtng_x, tgtng_y, tgtng_z;
    u32 rhit;
    ThingIdx shottng;
    short damage;
    ubyte wdmgtyp;

    if (p_owner->PTarget == NULL)
        return;

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }
    p_shot = &things[shottng];

    if (!thing_fire_shot_start_position(&prc_beg_pt, p_owner, WEP_LASER, p_owner->U.UMGun.ShotTurn)) {
        remove_thing(shottng);
        return;
    }

    wdef = &weapon_defs[WEP_LASER];
    wdmgtyp = DMG_LASER;

    get_thing_position_mapcoords(&tgtng_x, &tgtng_y, &tgtng_z, p_owner->PTarget->ThingOffset);

    p_shot->U.UEffect.Angle = p_owner->U.UMGun.AngleY;
    p_shot->X = prc_beg_pt.R[0];
    p_shot->Y = prc_beg_pt.R[1];
    p_shot->Z = prc_beg_pt.R[2];
    p_shot->VX = tgtng_x;
    p_shot->VY = (tgtng_y >> 3) + 10;
    p_shot->VZ = tgtng_z;
    p_shot->Radius = 50;
    p_shot->Owner = p_owner->ThingOffset;

    cor_x = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]);
    cor_y = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
    cor_z = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]);

    rhit = laser_hit_at(cor_x, cor_y, cor_z, &p_shot->VX, &p_shot->VY, &p_shot->VZ, p_shot);

    if (start_age > 15)
        start_age = 15;
    if (start_age < 5)
        start_age = 5;
    // For every consecutive turn of damage, deal more of it
    damage = wdef->HitDamage + (start_age - 5) * wdef->HitDamage;

    if ((rhit & 0x80000000) != 0) // hit 3D object collision vector
    {
        short hitvec;

        hitvec = rhit;
        bul_hit_vector(p_shot->VX, p_shot->VY, p_shot->VZ, -hitvec, 2 * start_age, wdmgtyp);
    }
    else if ((rhit & 0x20000000) != 0)
    {
        // no action
    }
    else if ((rhit & 0x40000000) != 0) // hit SimpleThing
    {
        struct SimpleThing *p_hitstng;
        ThingIdx hittng;

        hittng = rhit & ~0x60000000;
        p_hitstng = &sthings[-hittng];
        person_hit_by_bullet((struct Thing *)p_hitstng, damage, p_shot->VX - cor_x,
          p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
    }
    else if (rhit != 0) // hit normal thing
    {
        struct Thing *p_hittng;
        ThingIdx hittng;

        hittng = rhit & ~0x60000000;
        p_hittng = &things[hittng];
        person_hit_by_bullet(p_hittng, damage, p_shot->VX - cor_x,
          p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
    }
    p_shot->StartTimer1 = start_age;
    p_shot->Timer1 = start_age;
    p_shot->Flag = TngF_Unkn0004;
    p_shot->Type = TT_LASER11;
    add_node_thing(p_shot->ThingOffset);
}

void init_laser_6shot(struct Thing *p_person, ushort timer)
{
    struct Thing *p_target;
    struct Thing *p_thing;
    ThingIdx thing;
    ushort group, n_targets;

    p_target = p_person->PTarget;
    if (p_target == NULL)
        return;

    group = p_target->U.UPerson.EffectiveGroup & 0x1F;
    init_laser(p_person, timer);

    n_targets = 0;
    thing = same_type_head[256 + group];
    for (; thing != 0; thing = p_thing->LinkSameGroup)
    {
        if (n_targets >= 5)
            break;
        p_thing = &things[thing];
        if ((p_thing->Flag & TngF_Destroyed) == 0)
        {
            int dist_x, dist_z;
            dist_x = abs(PRCCOORD_TO_MAPCOORD(p_thing->X - p_person->X));
            dist_z = abs(PRCCOORD_TO_MAPCOORD(p_thing->Z - p_person->Z));
            // Simplification to avoid multiplication and square root to get proper distance
            if (dist_x <= dist_z)
                dist_x >>= 1;
            else
                dist_z >>= 1;
            if (dist_x + dist_z + 128 < TILE_TO_MAPCOORD(weapon_defs[WEP_LASER].RangeBlocks,0))
            {
                n_targets++;
                p_person->PTarget = p_thing;
                init_laser(p_person, timer);
            }
        }
    }
    p_person->PTarget = p_target;
}

short init_taser(struct Thing *p_person)
{
    short ret;
    asm volatile ("call ASM_init_taser\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
}

void init_rocket(struct Thing *p_owner)
{
#if 0
    asm volatile ("call ASM_init_rocket\n"
        : : "a" (p_owner));
#endif
    struct Thing *p_shot;
    struct Thing *p_target;
    struct M31 prc_beg_pt, prc_fin_pt;
    int pos_dt_x, pos_dt_z, pos_dt_y;
    int dist;
    ThingIdx shottng;

    LOGSYNC("Shot fired by %s offs=%d", thing_type_name(p_owner->Type, p_owner->SubType),
      (int)p_owner->ThingOffset);

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }

    p_shot = &things[shottng];

    if (!thing_fire_shot_start_position(&prc_beg_pt, p_owner, WEP_RAP, 0)) {
        remove_thing(shottng);
        return;
    }

    p_shot->PTarget = NULL;
    p_shot->Flag = 0;
    p_shot->U.UEffect.Angle = p_owner->U.UPerson.Angle;
    p_target = p_owner->PTarget;
    if ((p_owner->Flag & TngF_Unkn20000000) != 0)
    {
        prc_fin_pt.R[0] = MAPCOORD_TO_PRCCOORD(p_owner->VX, 0);
        prc_fin_pt.R[1] = MAPCOORD_TO_PRCCOORD(p_owner->VY + 20, 0);
        prc_fin_pt.R[2] = MAPCOORD_TO_PRCCOORD(p_owner->VZ, 0);
        // Transfer the flag from owner to shot
        p_shot->Flag |= TngF_Unkn20000000;
        p_owner->Flag &= ~TngF_Unkn20000000;
    }
    else if (p_target != NULL)
    {
        prc_fin_pt.R[0] = p_target->X;
        prc_fin_pt.R[1] = p_target->Y + MAPCOORD_TO_PRCCOORD(20, 0);
        prc_fin_pt.R[2] = p_target->Z;
        p_shot->PTarget = p_target;
    }
    else
    {
        thing_fire_shot_finish_position_straight_forward(&prc_fin_pt, &prc_beg_pt, p_owner, WEP_RAP);
    }

    p_shot->U.UEffect.GotoX = PRCCOORD_TO_MAPCOORD(prc_fin_pt.R[0]);
    p_shot->U.UEffect.GotoY = PRCCOORD_TO_MAPCOORD(prc_fin_pt.R[1]);
    p_shot->U.UEffect.GotoZ = PRCCOORD_TO_MAPCOORD(prc_fin_pt.R[2]);

    pos_dt_x = p_shot->U.UEffect.GotoX - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]);
    pos_dt_y = p_shot->U.UEffect.GotoY - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
    pos_dt_z = p_shot->U.UEffect.GotoZ - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]);

    dist = LbSqrL(pos_dt_z * pos_dt_z + pos_dt_x * pos_dt_x + pos_dt_y * pos_dt_y);
    if (dist == 0)
        dist = 1;

    p_shot->VX = (SHOT_ROCKED_SPEED * pos_dt_x) / dist;
    p_shot->VY = (SHOT_ROCKED_SPEED * pos_dt_y) / dist;
    p_shot->VZ = (SHOT_ROCKED_SPEED * pos_dt_z) / dist;

    init_shoot_recoil(p_owner, -p_shot->VX, 0, -p_shot->VZ);
    p_shot->X = prc_beg_pt.R[0];
    p_shot->Z = prc_beg_pt.R[2];
    p_shot->Y = prc_beg_pt.R[1];
    p_shot->StartTimer1 = 20;
    p_shot->Timer1 = 30;
    p_shot->Speed = 400;
    p_shot->Frame = 0;
    p_shot->StartFrame = 0;
    p_shot->U.UEffect.Object = 0;
    p_shot->U.UEffect.MatrixIndex = 0;
    p_shot->Parent = 0;
    p_shot->Owner = p_owner->ThingOffset;
    p_shot->Flag |= 0x0004;
    add_node_thing(shottng);
    p_shot->Type = TT_ROCKET;
    p_shot->Radius = 50;
    if (p_owner->U.UPerson.WeaponTimer <= 14)
    {
        p_shot->SubType = 0;
        p_shot->Timer1 = 20;
        p_shot->Speed = 700;
    }
    else
    {
        p_shot->SubType = 1;
        p_shot->Timer1 = 32;
    }
    play_dist_sample(p_shot, 0x18u, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
}

void init_razor_wire(struct Thing *p_person, ubyte flag)
{
    asm volatile ("call ASM_init_razor_wire\n"
        : : "a" (p_person), "d" (flag));
}

void finalise_razor_wire(struct Thing *p_person)
{
    asm volatile ("call ASM_finalise_razor_wire\n"
        : : "a" (p_person));
}

void init_lay_razor(struct Thing *p_thing, short x, short y, short z, int flag)
{
    asm volatile (
      "push %4\n"
      "call ASM_init_lay_razor\n"
        : : "a" (p_thing), "d" (x), "b" (y), "c" (z), "g" (flag));
}

void update_razor_wire(struct Thing *p_person)
{
    asm volatile ("call ASM_update_razor_wire\n"
        : : "a" (p_person));
}

void init_laser_beam(struct Thing *p_owner, ushort start_age, ubyte type)
{
#if 0
    asm volatile ("call ASM_init_laser_beam\n"
        : : "a" (p_owner), "d" (start_age), "b" (type));
#endif
    struct Thing *p_shot;
    struct WeaponDef *wdef;
    struct M31 prc_beg_pt;
    ubyte wdmgtyp;

    ushort shottng;
    int damage, dmg_delta;
    u32 rhit;
    int cor_x, cor_y, cor_z;
    int cor_beg_x, cor_beg_y, cor_beg_z;
    ThingIdx target;

    target = 0;

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }
    p_shot = &things[shottng];

    if (!thing_fire_shot_start_position(&prc_beg_pt, p_owner, WEP_BEAM, 0)) {
        remove_thing(shottng);
        return;
    }

    wdef = &weapon_defs[WEP_BEAM];
    wdmgtyp = DMG_BEAM;
    p_shot->U.UEffect.Angle = p_owner->U.UObject.Angle;

    if ((p_owner->Flag & TngF_Unkn20000000) != 0)
    {
        p_shot->VX = p_owner->VX;
        p_shot->VY = p_owner->VY;
        p_shot->VZ = p_owner->VZ;
        p_owner->Flag &= ~TngF_Unkn20000000;
    }
    else if (p_owner->PTarget != NULL)
    {
        struct Thing *p_target;
        p_target = p_owner->PTarget;
        p_shot->VX = PRCCOORD_TO_MAPCOORD(p_target->X);
        p_shot->VY = PRCCOORD_TO_MAPCOORD(p_target->Y) + 10;
        p_shot->VZ = PRCCOORD_TO_MAPCOORD(p_target->Z);
        target = p_target->ThingOffset;
    }
    else if ((p_owner->Flag & 0x1000) != 0)
    {
        ushort range;
        ubyte angl;
        range = wdef->RangeBlocks;
        angl = p_owner->U.UObject.Angle;
        p_shot->VX = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]) + (range * angle_direction[angl].DiX);
        p_shot->VY = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
        p_shot->VZ = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]) + (range * angle_direction[angl].DiY);
    }
    else
    {
        remove_thing(shottng);
        return;
    }

    p_shot->X = prc_beg_pt.R[0];
    p_shot->Y = prc_beg_pt.R[1];
    p_shot->Z = prc_beg_pt.R[2];
    p_shot->Owner = p_owner->ThingOffset;
    p_shot->Radius = 50;
    {
        int push_x, push_y;
        push_x = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]) - p_shot->VX;
        push_y = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]) - p_shot->VZ;
        init_shoot_recoil(p_owner, push_x, 0, push_y);
    }
    if (start_age > 15)
        start_age = 15;
    if (start_age < 5)
        start_age = 5;

    damage = wdef->HitDamage + (((start_age - 5) * wdef->HitDamage) >> 3);
    dmg_delta = 0;
    for (; ; damage -= dmg_delta)
    {
        cor_x = p_shot->VX;
        cor_y = p_shot->VY;
        cor_z = p_shot->VZ;
        cor_beg_z = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]);
        cor_beg_y = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
        cor_beg_x = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]);
        rhit = laser_hit_at(PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]),
          PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]),
          PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]),
          &cor_x, &cor_y, &cor_z, p_shot);

        if ((rhit & 0x80000000) != 0) // hit 3D object collision vector
          break;
        if (damage <= 0)
          break;

        if ((rhit & 0x20000000) != 0)
        {
            p_shot->VX = cor_x;
            p_shot->VY = cor_y;
            p_shot->VZ = cor_z;
            break;
        }

        else if ((rhit & 0x40000000) != 0) // hit SimpleThing
        {
            struct SimpleThing *p_hitstng;
            ThingIdx hitstng;
            hitstng = rhit & ~0x60000000;
            p_hitstng = &sthings[-hitstng];
            dmg_delta = person_hit_by_bullet((struct Thing *)p_hitstng, damage, p_shot->VX - cor_beg_x,
              p_shot->VY - cor_beg_y, p_shot->VZ - cor_beg_z, p_owner, wdmgtyp);
            if (dmg_delta < 0)
                break;
        }
        else if (rhit != 0) // hit normal thing
        {
            struct Thing *p_hittng;
            ThingIdx hittng;
            hittng = rhit & ~0x60000000;
            p_hittng = &things[hittng];
            dmg_delta = person_hit_by_bullet(p_hittng, damage, p_shot->VX - cor_beg_x,
              p_shot->VY - cor_beg_y, p_shot->VZ - cor_beg_z, p_owner, wdmgtyp);
            if (dmg_delta < 0)
                break;
        }

        prc_beg_pt.R[0] = MAPCOORD_TO_PRCCOORD(cor_x, 0);
        prc_beg_pt.R[1] = MAPCOORD_TO_PRCCOORD(cor_y, 0);
        prc_beg_pt.R[2] = MAPCOORD_TO_PRCCOORD(cor_z, 0);

        if (dmg_delta == 0)
        {
          p_shot->VX = cor_x;
          p_shot->VY = cor_y;
          p_shot->VZ = cor_z;
          break;
        }
    }

    if ((rhit & 0x80000000) != 0) // hit 3D object collision vector
    {
        p_shot->VX = cor_x;
        p_shot->VY = cor_y;
        p_shot->VZ = cor_z;
    }

    damage = wdef->HitDamage + ((wdef->HitDamage * (start_age - 5)) >> 3);

    if ((rhit & 0x80000000) == 0)
    {
        if (target > 0)
        {
          person_hit_by_bullet(&things[target], damage,
            p_shot->VX - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]),
            p_shot->VY - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]),
            p_shot->VZ - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]),
            p_owner, wdmgtyp);
        }
        else if ((p_owner->Flag2 & TgF2_ExistsOffMap) != 0)
        {
          p_shot->VY = PRCCOORD_TO_MAPCOORD(p_shot->Y);
        }
    }
    else
    {
        if ((p_owner->Flag2 & TgF2_ExistsOffMap) == 0)
        {
            short hitvec;

            hitvec = rhit & ~0x60000000;
            p_owner->U.UPerson.Flag3 |= 0x0040;
            // Why laser damage, not beam damage?
            bul_hit_vector(p_shot->VX, p_shot->VY, p_shot->VZ, -hitvec, damage, DMG_LASER);
        }
    }
    p_shot->StartTimer1 = start_age;
    p_shot->Timer1 = start_age;
    p_shot->Flag = TngF_Unkn0004;
    add_node_thing(p_shot->ThingOffset);

    switch (type)
    {
    case 17:
        p_shot->Type = TT_LASER29;
        break;
    case 20:
        p_shot->Type = 38;
        play_dist_sample(p_shot, 0x25u, 0x7Fu, 0x40u, 100, 0, 3);
        bang_new4(MAPCOORD_TO_PRCCOORD(p_shot->VX, 0),
          MAPCOORD_TO_PRCCOORD(p_shot->VY, 0),
          MAPCOORD_TO_PRCCOORD(p_shot->VZ, 0), 20);
        quick_crater(MAPCOORD_TO_TILE(p_shot->VX), MAPCOORD_TO_TILE(p_shot->VZ), 0);
        break;
    }
}

void init_laser_guided(struct Thing *p_owner, ushort start_age)
{
    asm volatile ("call ASM_init_laser_guided\n"
        : : "a" (p_owner), "d" (start_age));
}

void weapon_shooting_floor_creates_smoke(MapCoord cor_x, MapCoord cor_z)
{
    struct SimpleThing *p_sthing;
    MapCoord cor_y;
    ushort textr;

    textr = floor_texture_at_point(cor_x, cor_z);
    if ((get_my_texture_bits(textr) & 2) != 0)
    {
        // Create small smoke effect for weapon discharge into water
        cor_y = alt_at_point(cor_x, cor_z) >> 8;
        p_sthing = create_scale_effect(cor_x, cor_y, cor_z, 1090, 8);
        if (p_sthing != NULL) {
            p_sthing->SubType = 58;
        }
    }
}

void init_laser_elec(struct Thing *p_owner, ushort start_age)
{
#if 0
    asm volatile ("call ASM_init_laser_elec\n"
        : : "a" (p_owner), "d" (start_age));
#endif
    struct Thing *p_shot;
    struct WeaponDef *wdef;
    struct M31 prc_beg_pt;
    MapCoord cor_x, cor_y, cor_z;
    u32 rhit;
    int damage;
    ThingIdx shottng, target;
    ubyte wdmgtyp;
    TbBool allow_gnd_hit_eff;

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }
    p_shot = &things[shottng];

    p_shot->U.UEffect.Angle = p_owner->U.UPerson.Angle;

    if (!thing_fire_shot_start_position(&prc_beg_pt, p_owner, WEP_ELLASER, 0)) {
        remove_thing(shottng);
        return;
    }

    target = 0;
    allow_gnd_hit_eff = false;
    wdef = &weapon_defs[WEP_ELLASER];
    wdmgtyp = DMG_ELLASER;

    if ((p_owner->Flag & TngF_Unkn20000000) != 0)
    {
        p_shot->VX = p_owner->VX;
        p_shot->VY = p_owner->VY;
        p_shot->VZ = p_owner->VZ;
        p_owner->Flag &= ~TngF_Unkn20000000;
        allow_gnd_hit_eff = true;
    }
    else if (p_owner->PTarget != NULL)
    {
        struct Thing *p_target;
        p_target = p_owner->PTarget;
        p_shot->VX = PRCCOORD_TO_MAPCOORD(p_target->X);
        p_shot->VY = PRCCOORD_TO_MAPCOORD(p_target->Y) + 10;
        p_shot->VZ = PRCCOORD_TO_MAPCOORD(p_target->Z);
        target = p_target->ThingOffset;
    }
    else if ((p_owner->Flag & TngF_Unkn1000) != 0)
    {
        ushort range;
        ubyte angl;
        range = wdef->RangeBlocks;
        angl = p_owner->U.UObject.Angle;
        p_shot->VX = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]) + (range * angle_direction[angl].DiX);
        p_shot->VY = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
        p_shot->VZ = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]) + (range * angle_direction[angl].DiY);
    }
    else
    {
        remove_thing(shottng);
        return;
    }

    p_shot->X = prc_beg_pt.R[0];
    p_shot->Y = prc_beg_pt.R[1];
    p_shot->Z = prc_beg_pt.R[2];
    p_shot->Radius = 50;
    p_shot->Owner = p_owner->ThingOffset;

    cor_x = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]);
    cor_y = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
    cor_z = PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]);
    rhit = laser_hit_at(cor_x, cor_y, cor_z, &p_shot->VX, &p_shot->VY, &p_shot->VZ, p_shot);

    if (start_age > 15)
        start_age = 15;
    if (start_age < 5)
        start_age = 5;
    // For every consecutive turn of damage, deal more of it
    damage = wdef->HitDamage + ((wdef->HitDamage * (start_age - 5)) >> 3);

    if ((rhit & 0x80000000) != 0) // hit 3D object collision vector
    {
        short hitvec;

        hitvec = rhit;
        if (hitvec != -9999)
        {
            elec_hit_building(p_shot->VX, p_shot->VY, p_shot->VZ, -hitvec);
        }
    }
    else if ((rhit & 0x20000000) != 0)
    {
        // no action
    }
    else if ((rhit & 0x40000000) != 0) // hit SimpleThing
    {
        struct SimpleThing *p_hitstng;
        ThingIdx hitstng;
        hitstng = rhit & ~0x60000000;
        p_hitstng = &sthings[-hitstng];
        person_hit_by_bullet((struct Thing *)p_hitstng, damage, p_shot->VX - cor_x,
          p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
    }
    else if (rhit != 0) // hit normal thing
    {
        struct Thing *p_hittng;
        ThingIdx hittng;
        hittng = rhit & ~0x60000000;
        p_hittng = &things[hittng];
        person_hit_by_bullet(p_hittng, damage, p_shot->VX - cor_x,
          p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
    }
    else // if did not hit anything else, go for original target
    {
        if (target != 0)
        {
            person_hit_by_bullet(&things[target], damage, p_shot->VX - cor_x,
              p_shot->VY - cor_y, p_shot->VZ - cor_z, p_owner, wdmgtyp);
        }
        else
        {
            if ((p_owner->Flag2 & TgF2_ExistsOffMap) != 0) {
                p_shot->VY = PRCCOORD_TO_MAPCOORD(p_shot->Y);
            }
            if (allow_gnd_hit_eff) {
                weapon_shooting_floor_creates_smoke(p_shot->VX, p_shot->VZ);
            }
        }
    }
    p_shot->StartTimer1 = start_age;
    p_shot->Timer1 = start_age;
    if (start_age > 8)
        start_age = 8;
    p_shot->SubType = start_age;
    p_shot->Flag = TngF_Unkn0004;
    p_shot->Type = TT_LASER_ELEC;
    p_shot->State = 0;
    add_node_thing(p_shot->ThingOffset);
}

void init_laser_q_sep(struct Thing *p_owner, ushort start_age)
{
    init_laser_guided(p_owner, start_age);
    init_laser_guided(p_owner, start_age);
    init_laser_guided(p_owner, start_age);
    init_laser_guided(p_owner, start_age);
    init_laser_guided(p_owner, start_age);
    init_laser_beam(p_owner, start_age, 0x14u);
}

void init_uzi(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_uzi\n"
        : : "a" (p_owner));
}

void init_minigun(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_minigun\n"
        : : "a" (p_owner));
}

void init_flamer(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_flamer\n"
        : : "a" (p_owner));
}

void init_long_range(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_long_range\n"
        : : "a" (p_owner));
}

void init_air_strike(struct Thing *p_owner)
{
#if 0
    asm volatile ("call ASM_init_air_strike\n"
        : : "a" (p_owner));
#endif
    ThingIdx shottng;
    struct Thing *p_thing;
    ubyte grp;

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }
    p_thing = &things[shottng];
    p_thing->Type = TT_AIR_STRIKE;
    p_thing->X = p_owner->X;
    p_thing->Z = p_owner->Z;
    p_thing->Y = p_owner->Y;
    p_thing->Owner = p_owner->ThingOffset;
    p_thing->Timer1 = 400;
    p_thing->StartFrame = 1004;
    p_thing->Frame = nstart_ani[p_thing->StartFrame];
    p_thing->U.UEffect.Object = 0;
    p_thing->Flag = TngF_Unkn0004;
    p_thing->Radius = 50;
    grp = p_owner->U.UPerson.EffectiveGroup;
    p_thing->U.UEffect.EffectiveGroup = grp;
    p_thing->U.UEffect.Group = grp;

    play_dist_sample(p_thing, 66, FULL_VOL, EQUL_PAN, NORM_PTCH, 990, 3);
}

void init_stasis_gun(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_stasis_gun\n"
        : : "a" (p_owner));
}

void init_time_gun(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_time_gun\n"
        : : "a" (p_owner));
}

void init_c_iff(struct Thing *p_owner)
{
    asm volatile ("call ASM_init_c_iff\n"
        : : "a" (p_owner));
}

void init_grenade(struct Thing *p_owner, ushort gtype)
{
    asm volatile ("call ASM_init_grenade\n"
        : : "a" (p_owner), "d" (gtype));
}

void init_v_rocket(struct Thing *p_owner)
{
#if 0
    asm volatile ("call ASM_init_v_rocket\n"
        : : "a" (p_owner));
#else
    ThingIdx shottng;
    struct Thing *p_shot;
    struct Thing *p_veh;
    struct Thing *p_mgun;
    struct Thing *p_target;
    struct M31 prc_beg_pt;
    int pos_dt_x, pos_dt_z, pos_dt_y;
    int dist;

    LOGSYNC("Shot fired by %s offs=%d", thing_type_name(p_owner->Type, p_owner->SubType),
      (int)p_owner->ThingOffset);

    shottng = get_new_thing();
    if (shottng == 0) {
        LOGERR("No thing slots for a shot");
        return;
    }

    p_shot = &things[shottng];
    p_veh = &things[p_owner->U.UPerson.Vehicle];

    p_mgun = &things[p_veh->U.UVehicle.SubThing];
    LOGDBG("Shot from vehicle %s offs=%d, mgun %s offs=%d",
      thing_type_name(p_veh->Type, p_veh->SubType), (int)p_veh->ThingOffset,
      thing_type_name(p_mgun->Type, p_mgun->SubType), (int)p_mgun->ThingOffset);

    if (!thing_fire_shot_start_position(&prc_beg_pt, p_veh, WEP_RAP, p_mgun->U.UMGun.ShotTurn)) {
        remove_thing(shottng);
        return;
    }

    p_shot->PTarget = NULL;
    p_shot->U.UEffect.Angle = p_owner->U.UPerson.Angle;

    p_target = p_veh->PTarget;

    if ((p_owner->Flag & TngF_Unkn20000000) != 0)
    {
        p_shot->U.UEffect.GotoX = p_veh->U.UVehicle.TargetDX;
        p_shot->U.UEffect.GotoY = p_veh->U.UVehicle.TargetDY;
        p_shot->U.UEffect.GotoZ = p_veh->U.UVehicle.TargetDZ;
        // Transfer the flag from owner to shot
        p_owner->Flag &= ~TngF_Unkn20000000;
        p_shot->Flag |= TngF_Unkn20000000;
    }
    else if (p_target != NULL)
    {
        p_shot->PTarget = p_target;
        p_shot->U.UEffect.GotoX = PRCCOORD_TO_MAPCOORD(p_target->X);
        p_shot->U.UEffect.GotoY = PRCCOORD_TO_MAPCOORD(p_target->Y);
        p_shot->U.UEffect.GotoZ = PRCCOORD_TO_MAPCOORD(p_target->Z);
    }
    else
    {
        // The function should be only called if the vehicle has a target set,
        // so this should be safe
        p_shot->U.UEffect.GotoX = p_veh->U.UVehicle.TargetDX;
        p_shot->U.UEffect.GotoY = p_veh->U.UVehicle.TargetDY;
        p_shot->U.UEffect.GotoZ = p_veh->U.UVehicle.TargetDZ;
    }
    pos_dt_x = p_shot->U.UEffect.GotoX - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[0]);
    pos_dt_y = p_shot->U.UEffect.GotoY - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[1]);
    pos_dt_z = p_shot->U.UEffect.GotoZ - PRCCOORD_TO_MAPCOORD(prc_beg_pt.R[2]);

    dist = LbSqrL(pos_dt_z * pos_dt_z + pos_dt_x * pos_dt_x);
    if (dist == 0)
        dist = 1;
    p_shot->VX = (SHOT_ROCKED_SPEED * pos_dt_x) / dist;
    p_shot->VY = (SHOT_ROCKED_SPEED * pos_dt_y) / dist;
    p_shot->VZ = (SHOT_ROCKED_SPEED * pos_dt_z) / dist;
    p_shot->X = prc_beg_pt.R[0];
    p_shot->Y = prc_beg_pt.R[1];
    p_shot->Z = prc_beg_pt.R[2];

    p_shot->StartTimer1 = 20;
    p_shot->Timer1 = 30;
    p_shot->Speed = 600;
    p_shot->Frame = 0;
    p_shot->StartFrame = 0;
    p_shot->U.UEffect.Object = 0;
    p_shot->U.UEffect.MatrixIndex = 0;
    p_shot->Parent = 0;
    p_shot->Owner = p_owner->ThingOffset;
    p_shot->Flag |= 0x0004;
    p_shot->Type = TT_ROCKET;
    p_shot->Radius = 50;
    add_node_thing(p_shot->ThingOffset);

    play_dist_sample(p_shot, 24, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
#endif
}

void init_mech_rocket(struct Thing *p_owner, struct Thing *p_mech, int x, int y, int z)
{
    asm volatile (
      "push %4\n"
      "call ASM_init_mech_rocket\n"
        : : "a" (p_owner), "d" (p_mech), "b" (x), "c" (y), "g" (z));
}

void give_take_me_weapon(struct Thing *p_person, int item, int giveortake, short id)
{
    asm volatile (
      "call ASM_give_take_me_weapon\n"
        : : "a" (p_person), "d" (item), "b" (giveortake), "c" (id));
}

ushort set_player_weapon_turn(struct Thing *p_person, ushort time)
{
    ushort ret;
    asm volatile ("call ASM_set_player_weapon_turn\n"
        : "=r" (ret) : "a" (p_person), "d" (time));
    return ret;
}

void init_fire_weapon(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_init_fire_weapon\n"
        : : "a" (p_person));
#else
    struct WeaponDef *wdef;
    int plagent;
    struct Thing *p_target;
    WeaponType wtype;
    short i;

    wtype = p_person->U.UPerson.CurrentWeapon;
    wdef = &weapon_defs[wtype];
    if ((p_person->Flag & TngF_Unkn0001) != 0)
        return;

    if (((gameturn + p_person->ThingOffset) & 0x7F) == 0)
        process_random_speech(p_person, 1);

    if ((p_person->Type != TT_MINE) && ((p_person->State == PerSt_WAIT) || (p_person->State == PerSt_NONE)))
    {
        p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 1);
        reset_person_frame(p_person);
    }
    if (p_person->U.UPerson.Energy <= wdef->EnergyUsed)
    {
        if (wtype == WEP_MINIGUN) {
            plagent = p_person->U.UPerson.ComCur & 3;
            play_dist_sample(p_person, 88 + plagent, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
        }
    }
    else
    {
        if (!in_network_game && !is_single_game && (p_person->Flag & TngF_PlayerAgent) != 0
          && !is_research_weapon_completed(wtype) && (LbRandomAnyShort() % 17 < 4))
        {
            // Deplete energy to the level which is barely enough for the shot
            // After the shot takes its energy, the level remaining will be near zero
            p_person->U.UPerson.Energy = wdef->EnergyUsed + 1;
        }

        if (weapon_can_be_charged(wtype) || weapon_has_targetting(wtype)) {
            p_person->Flag |= TngF_WepCharging;
            p_person->U.UPerson.WeaponTimer = 0;
        }

        switch (wtype)
        {
        case WEP_UZI:
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            p_person->Flag &= ~TngF_WepCharging;
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            p_person->U.UPerson.FrameId.Version[4] = 1;
            init_uzi(p_person);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            if ((p_person->Flag2 & TgF2_Unkn0200) == 0)
            {
                if ((p_person->Flag & TngF_PlayerAgent) != 0)
                    i = 0;
                else
                    i = 10;

                if ((p_person->Flag & TngF_PlayerAgent) != 0)
                    plagent = p_person->U.UPerson.ComCur & 3;
                else
                    plagent = 0;

                play_dist_sample(p_person, 80 + plagent, FULL_VOL, EQUL_PAN, NORM_PTCH + i, LOOP_4EVER, 3);
            }
            p_person->Flag2 |= TgF2_Unkn0200;
            break;
        case WEP_MINIGUN:
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            p_person->Flag &= ~TngF_WepCharging;
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            p_person->U.UPerson.FrameId.Version[4] = 3;
            init_minigun(p_person);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            if ((p_person->Flag2 & TgF2_Unkn0200) == 0)
            {
                if ((p_person->Flag & TngF_PlayerAgent) != 0)
                    plagent = p_person->U.UPerson.ComCur & 3;
                else
                    plagent = 0;

                play_dist_sample(p_person, 84 + plagent, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
            }
            p_person->Flag2 |= TgF2_Unkn0200;
            break;
        case WEP_LASER:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            if ((p_person->Flag & TngF_Unkn1000) != 0)
                play_dist_sample(p_person, 7, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
            break;
        case WEP_ELLASER:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            if (((p_person->Flag2 & TgF2_ExistsOffMap) == 0)
              && ((p_person->Flag & TngF_Unkn1000) != 0))
                play_dist_sample(p_person, 7, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
            break;
        case WEP_RAP:
            // Targetted shots are created somewhere else
            break;
        case WEP_NUCLGREN:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                person_weapons_remove_one(p_person, wtype);
            }
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            init_grenade(p_person, 3);
            play_dist_sample(p_person, 36, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            if ((p_person->Flag & TngF_PlayerAgent) == 0)
                choose_best_weapon_for_range(p_person, 1280);
            break;
        case WEP_FLAMER:
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            init_flamer(p_person);
            if (p_person->U.UPerson.WeaponTurn == 0)
                p_person->U.UPerson.WeaponTurn = 10;
            break;
        case WEP_CRAZYGAS:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                person_weapons_remove_one(p_person, wtype);
            }
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            init_grenade(p_person, 4);
            play_dist_sample(p_person, 36, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            if ((p_person->Flag & TngF_PlayerAgent) == 0)
                choose_best_weapon_for_range(p_person, 1280);
            break;
        case WEP_KOGAS:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                person_weapons_remove_one(p_person, wtype);
            }
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            init_grenade(p_person, 5);
            play_dist_sample(p_person, 36, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            if ((p_person->Flag & TngF_PlayerAgent) == 0)
                choose_best_weapon_for_range(p_person, 1280);
            break;
        case WEP_ELEMINE:
        case WEP_EXPLMINE:
            if ((p_person->Flag2 & TgF2_Unkn00800000) == 0)
            {
                p_person->Flag2 |= TgF2_Unkn00800000;
                person_init_drop(p_person, wtype);
                p_person->U.UPerson.Energy -= wdef->EnergyUsed;
                p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            }
            break;
        case WEP_LONGRANGE:
            p_person->Flag &= ~TngF_WepCharging;
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            p_person->U.UPerson.FrameId.Version[4] = 1;
            init_long_range(p_person);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            play_dist_sample(p_person, 33, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
            break;
        case WEP_AIRSTRIKE:
            give_take_me_weapon(p_person, wtype, -1, p_person->ThingOffset);
            init_air_strike(p_person);
            break;
        case WEP_BEAM:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            if ((p_person->Flag & TngF_Unkn1000) != 0)
                play_dist_sample(p_person, 7, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
            break;
        case WEP_QDEVASTATOR:
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
            {
                p_target = p_person->PTarget;
                if (p_target != NULL)
                {
                    if ((p_target->Flag & TngF_Destroyed) == 0)
                        p_person->Flag2 |= TgF2_Unkn00200000;
                }
            }
            if ((p_person->Flag & TngF_Unkn1000) != 0)
                play_dist_sample(p_person, 7, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
            break;
        case WEP_STASISFLD:
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            p_person->Flag &= ~TngF_WepCharging;
            p_person->U.UPerson.FrameId.Version[4] = 1;
            init_stasis_gun(p_person);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            break;
        case WEP_TIMEGUN:
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            p_person->Flag &= ~TngF_WepCharging;
            p_person->U.UPerson.FrameId.Version[4] = 1;
            init_time_gun(p_person);
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            break;
        case WEP_CEREBUSIFF:
            //TODO all other consumables are kept if NPC uses them; why do we break the rule for IFF?
            { // Remove weapon even if it is NPC
                person_weapons_remove_one(p_person, WEP_CEREBUSIFF);
            }
            p_person->U.UPerson.Energy -= wdef->EnergyUsed;
            p_person->Flag &= ~TngF_WepCharging;
            p_person->U.UPerson.FrameId.Version[4] = 1;
            init_c_iff(p_person);
            p_person->U.UPerson.FrameId.Version[4] = 0;
            p_person->U.UPerson.WeaponTurn = wdef->ReFireDelay;
            break;
        default:
          break;
        }
    }
    if (((p_person->Flag & TngF_WepCharging) == 0) && (p_person->U.UPerson.CurrentWeapon != WEP_FLAMER))
        p_person->Flag &= ~TngF_Unkn20000000;
    if ((p_person->Flag & TngF_PlayerAgent) != 0)
        set_player_weapon_turn(p_person, p_person->U.UPerson.WeaponTurn);
#endif
}

void init_clone_disguise(struct Thing *p_person)
{
    if ((p_person->Flag2 & TgF2_Unkn00400000) != 0)
        return;

    p_person->U.UPerson.AnimMode = ANIM_PERS_IDLE;
    p_person->U.UPerson.OldSubType = p_person->SubType;
    switch (LbRandomAnyShort() & 3)
    {
    case 0:
        p_person->SubType = SubTT_PERS_BRIEFCASE_M;
        break;
    case 1:
        p_person->SubType = SubTT_PERS_WHITE_BRUN_F;
        break;
    case 2:
        p_person->SubType = SubTT_PERS_WHIT_BLOND_F;
        break;
    case 3:
        p_person->SubType = SubTT_PERS_LETH_JACKT_M;
        break;
    default:
        break;
    }
    reset_person_frame(p_person);
    p_person->Flag2 |= TgF2_Unkn00400000;
}

void reset_clone_disguise(struct Thing *p_person)
{
    if ((p_person->Flag2 & TgF2_Unkn00400000) == 0)
        return;

    p_person->Flag2 &= ~TgF2_Unkn00400000;
    p_person->SubType = p_person->U.UPerson.OldSubType;
    reset_person_frame(p_person);
}

int gun_out_anim(struct Thing *p_person, ubyte shoot_flag)
{
#if 0
    int ret;
    asm volatile ("call ASM_gun_out_anim\n"
        : "=r" (ret) : "a" (p_person), "d" (shoot_flag));
    return ret;
#endif
    if ((p_person->Flag & TngF_Destroyed) != 0) {
        return p_person->U.UPerson.AnimMode;
    }

    switch (p_person->U.UPerson.CurrentWeapon)
    {
    case WEP_FLAMER:
    case WEP_QDEVASTATOR:
    case WEP_MINIGUN:
    case WEP_LASER:
    case WEP_ELLASER:
    case WEP_LONGRANGE:
    case WEP_BEAM:
        if (p_person->SubType == SubTT_PERS_AGENT)
            p_person->U.UPerson.FrameId.Version[3] = 1;
        if (shoot_flag == 2)
            return ANIM_PERS_WEPHEAVY_Unkn07;
        if (shoot_flag == 1)
            return ANIM_PERS_WEPHEAVY_Unkn15;
        return ANIM_PERS_WEPHEAVY_IDLE;

    default:
    case WEP_NULL:
    case WEP_NUCLGREN:
    case WEP_H2HTASER:
    case WEP_CRAZYGAS:
    case WEP_KOGAS:
    case WEP_ELEMINE:
    case WEP_EXPLMINE:
    case WEP_NAPALMMINE:
    case WEP_AIRSTRIKE:
    case WEP_RAZORWIRE:
    case WEP_PERSUADER2:
    case WEP_ENERGYSHLD:
    case WEP_CEREBUSIFF:
    case WEP_MEDI1:
    case WEP_MEDI2:
    case WEP_EXPLWIRE:
    case WEP_CLONESHLD:
    case WEP_UNUSED1F:
        if (p_person->SubType == SubTT_PERS_AGENT)
            p_person->U.UPerson.FrameId.Version[3] = 0;
        if (p_person->State == PerSt_WAIT)
            return ANIM_PERS_Unkn21;
        return ANIM_PERS_IDLE;

    case WEP_UZI:
    case WEP_RAP:
    case WEP_PERSUADRTRN:
    case WEP_SONICBLAST:
    case WEP_STASISFLD:
    case WEP_SOULGUN:
    case WEP_TIMEGUN:
        if (p_person->SubType == SubTT_PERS_AGENT)
            p_person->U.UPerson.FrameId.Version[3] = 0;
        if (shoot_flag == 2)
            return ANIM_PERS_Unkn06;
        if (shoot_flag == 1)
            return ANIM_PERS_Unkn14;
        return ANIM_PERS_WEPLIGHT_IDLE;
    }
}

ushort player_weapon_time(struct Thing *p_person)
{
    ushort ret;
    asm volatile ("call ASM_player_weapon_time\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
}

ThingIdx person_is_non_selected_agent(struct Thing *p_person)
{
    return ((p_person->Flag & TngF_PlayerAgent) != 0) && ((p_person->Flag & TngF_Unkn1000) == 0);
}

ThingIdx person_is_selected_agent(struct Thing *p_person)
{
    return ((p_person->Flag & TngF_PlayerAgent) != 0) && ((p_person->Flag & TngF_Unkn1000) != 0);
}

ThingIdx get_vehicle_passenger_in_player_control(struct Thing *p_vehicle)
{
    ThingIdx passener;

    passener = p_vehicle->U.UVehicle.PassengerHead;
    while (passener != 0)
    {
        struct Thing *p_passenger;

        p_passenger = &things[passener];
        if (p_passenger->Type == TT_PERSON)
        {
            if (person_is_selected_agent(p_passenger))
                return passener;
        }
        passener = p_passenger->U.UPerson.LinkPassenger;
    }
    return 0;
}

ubyte vehicle_with_person_shoot_at_target(struct Thing *p_owner)
{
    struct Thing *p_veh;
    struct Thing *p_mgun;
    ubyte turn;

    if (p_owner->U.UPerson.WeaponTurn != 0) {
        return 0;
    }

    p_veh = &things[p_owner->U.UPerson.Vehicle];

    if (p_veh->U.UVehicle.SubThing == 0) {
        return 0;
    }

    p_mgun = &things[p_veh->U.UVehicle.SubThing];

    turn = p_mgun->U.UMGun.ShotTurn;
    init_v_rocket(p_owner);
    p_mgun->U.UMGun.ShotTurn = (turn + 1) & 0x03;
    p_owner->U.UPerson.WeaponTurn = weapon_tank_rocket_ReFireDelay;
    return 0;
}

void process_vehicle_weapon(struct Thing *p_vehicle, struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_process_vehicle_weapon\n"
        : : "a" (p_vehicle), "d" (p_person));
#else
    // If another passenger controls the weapon of this vehicle, no action
    if (person_is_non_selected_agent(p_person)
      && (get_vehicle_passenger_in_player_control(p_vehicle) != 0))
        return;

    if ((p_person->Flag & TngF_Unkn20000000) != 0)
    {
        short tdx, tdy, tdz;

        if ((p_person->Flag & TngF_PlayerAgent) != 0)
        {
            PlayerInfo *p_player;
            PlayerIdx plyr;
            ushort plagent;

            plyr = p_person->U.UPerson.ComCur >> 2;
            plagent = p_person->U.UPerson.ComCur & 3;
            p_player = &players[plyr];

            tdx = p_player->UserVX[plagent];
            tdz = p_player->UserVZ[plagent];
            if (p_player->UserVY[plagent] != 0)
            {
                tdy = p_player->UserVY[plagent];
                p_player->UserVY[plagent] = 0;
            }
            else
            {
                tdy = (alt_at_point(tdx, tdz) >> 8) + 20;
            }
        }
        else
        {
            tdx = p_vehicle->U.UVehicle.TargetDX;
            tdy = p_vehicle->U.UVehicle.TargetDY;
            tdz = p_vehicle->U.UVehicle.TargetDZ;
        }

        if (p_vehicle->U.UVehicle.TargetDX != tdx || p_vehicle->U.UVehicle.TargetDZ != tdz)
            p_vehicle->OldTarget = 20000;
        p_vehicle->PTarget = NULL;
        p_vehicle->U.UVehicle.TargetDX = tdx;
        p_vehicle->U.UVehicle.TargetDZ = tdz;
        p_vehicle->U.UVehicle.TargetDY = tdy;
        p_vehicle->Flag |= TngF_Unkn20000000;
    }
    else if (p_person->PTarget != NULL)
    {
        struct Thing *p_target;

        p_vehicle->Flag &= ~TngF_Unkn20000000;
        p_target = p_person->PTarget;
        if (p_vehicle->PTarget != p_target)
        {
            p_vehicle->OldTarget = 2000;
            p_vehicle->PTarget = p_target;
        }
    }

    if (((p_person->Flag & TngF_Unkn0800) != 0)
      && (p_vehicle->OldTarget < 24)
      && ((p_vehicle->PTarget != NULL && p_person->PTarget != NULL)
      || (p_vehicle->Flag & TngF_Unkn20000000) != 0))
    {
        vehicle_with_person_shoot_at_target(p_person);
    }
#endif
}

void mech_unkn_func_05(int owntng, int a2, int a3)
{
    asm volatile ("call ASM_mech_unkn_func_05\n"
        : : "a" (owntng), "d" (a2), "b" (a3));
}

void process_mech_weapon(struct Thing *p_vehicle, struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_process_mech_weapon\n"
        : : "a" (p_vehicle), "d" (p_person));
#else
    if ((p_person->Flag & TngF_Unkn20000000) != 0)
    {
        short tdx, tdy, tdz;

        p_vehicle->PTarget = NULL;
        p_vehicle->Flag |= TngF_Unkn20000000;
        if ((p_person->Flag & TngF_PlayerAgent) != 0)
        {
            PlayerInfo *p_player;
            PlayerIdx plyr;
            ushort plagent;

            plyr = p_person->U.UPerson.ComCur >> 2;
            plagent = p_person->U.UPerson.ComCur & 3;
            p_player = &players[plyr];

            tdx = p_player->UserVX[plagent];
            tdz = p_player->UserVZ[plagent];
            if (p_player->UserVY[plagent] != 0)
            {
                tdy = p_player->UserVY[plagent];
                p_player->UserVY[plagent] = 0;
            }
            else
            {
                tdy = (alt_at_point(tdx, tdz) >> 8) + 20;
            }
        }
        else
        {
            tdy = p_person->VY;
            tdz = p_person->VZ;
            tdx = p_person->VX;
        }
        if ((p_vehicle->U.UVehicle.TargetDX != tdx) || (p_vehicle->U.UVehicle.TargetDZ != tdz))
        {
            p_vehicle->OldTarget = 20000;
            p_vehicle->U.UVehicle.TargetDY = tdy;
            p_vehicle->U.UVehicle.TargetDZ = tdz;
            p_vehicle->U.UVehicle.TargetDX = tdx;
        }
        p_vehicle->PTarget = NULL;
    }
    else if (p_person->PTarget != NULL)
    {
        struct Thing *p_target;

        p_vehicle->Flag &= ~TngF_Unkn20000000;
        p_target = p_person->PTarget;
        if (p_vehicle->PTarget != p_target)
        {
            p_vehicle->OldTarget = 20000;
            p_vehicle->PTarget = p_target;
        }
    }

    if ((p_person->Flag & TngF_PlayerAgent) == 0)
        p_person->Flag |= TngF_Unkn0800;

    if (((p_person->Flag & TngF_Unkn0800) != 0) && ((p_vehicle->U.UVehicle.TNode & 0x0004) != 0)
      && (p_person->U.UPerson.WeaponTurn == 0)
      && ((p_vehicle->PTarget != NULL && p_person->PTarget != NULL) || (p_vehicle->Flag & TngF_Unkn20000000) != 0))
    {
        p_person->U.UVehicle.WeaponTurn = 10;
        if (p_vehicle->OldTarget < 24)
        {
            int val;

            if ((LbRandomAnyShort() & 0x1F) != 0)
                val = ((gameturn & 1) != 0) + 5;
            else
                val = 7;
            mech_unkn_func_05(p_vehicle->Owner, 0, val);
            p_vehicle->U.UVehicle.TNode &= ~0x0004;
        }
    }
    if ((p_vehicle->U.UVehicle.TNode & 0x0001) != 0)
    {
        p_vehicle->U.UVehicle.TNode &= ~0x0001;
        init_mech_rocket(p_person, p_vehicle, mech_unkn_dw_1DC880, mech_unkn_dw_1DC884, mech_unkn_dw_1DC888);
    }
    if ((p_vehicle->U.UVehicle.TNode & 0x0002) != 0)
    {
        p_vehicle->U.UVehicle.TNode &= ~0x0002;
        init_mech_rocket(p_person, p_vehicle, mech_unkn_dw_1DC88C, mech_unkn_dw_1DC890, mech_unkn_dw_1DC894);
    }
    if ((p_vehicle->U.UVehicle.TNode & 0x0008) != 0)
    {
        p_vehicle->U.UVehicle.TNode &= ~0x0008;
        init_laser_guided(p_person, 32);
        init_laser_guided(p_person, 32);
        init_laser_guided(p_person, 32);
        init_laser_guided(p_person, 32);
    }
#endif
}

ushort persuade_power_required(ThingIdx victim)
{
    struct Thing *p_victim;
    ushort ptype;
    ushort persd_pwr_rq;
    short brain_lv;

    if (victim <= 0)
        return 9999;

    p_victim = &things[victim];
    if ((p_victim->Flag2 & TgF2_Unkn00400000) != 0)
        ptype = p_victim->U.UPerson.OldSubType;
    else
        ptype = p_victim->SubType;

    persd_pwr_rq = peep_type_stats[ptype].PersuadeReqd;
    brain_lv = cybmod_brain_level(&p_victim->U.UPerson.UMod);
    if (brain_lv == 5)
        return 9999;
    if (brain_lv == 4)
        persd_pwr_rq = peep_type_stats[SubTT_PERS_AGENT].PersuadeReqd;

    return persd_pwr_rq;
}

TbBool person_can_be_persuaded_now(ThingIdx attacker, ThingIdx target,
  short weapon_range, ubyte target_select, ushort *energy_reqd)
{
    struct Thing *p_target;
    struct Thing *p_attacker;
    short target_persd_pwr_rq;

    if ((target <= 0) || (attacker <= 0))
        return false;

    p_attacker = &things[attacker];

    p_target = &things[target];
    if (p_target->Type != TT_PERSON)
        return false;

    {
        int dist_y;
        short cntr_cor_y;

        cntr_cor_y = PRCCOORD_TO_YCOORD(p_attacker->Y);
        dist_y = abs(PRCCOORD_TO_YCOORD(p_target->Y) - cntr_cor_y);
        if (dist_y >= 1240)
            return false;
    }

    if (((target_select == PTargSelect_Persuader) || (target_select == PTargSelect_PersuadeAdv)) &&
      ((p_target->Flag & (TngF_Unkn40000000|TngF_Persuaded|TngF_Destroyed)) != 0))
        return false;

    // If already harvested the soul
    if ((target_select == PTargSelect_SoulCollect) && ((p_target->Flag2 & TgF2_SoulDepleted) != 0))
        return false;

    // Cannot persuade people from own group
    if (((target_select == PTargSelect_Persuader) || (target_select == PTargSelect_PersuadeAdv)) &&
      (p_target->U.UPerson.EffectiveGroup == p_attacker->U.UPerson.EffectiveGroup))
        return false;

    // Holding a taser prevents both persuasion and soul harvest
    if (p_target->U.UPerson.CurrentWeapon == WEP_H2HTASER)
        return false;

    // Self-affecting not allowed for both persuasion and soul harvest
    if (target == attacker)
        return false;

    // Some people can only be affected by advanced persuader
    if ((target_select == PTargSelect_Persuader) &&
      person_only_affected_by_adv_persuader(target))
        return false;

    target_persd_pwr_rq = persuade_power_required(target);

    // Check if we have enough persuade power to overwhelm the target
    if (((target_select == PTargSelect_Persuader) || (target_select == PTargSelect_PersuadeAdv)) &&
      (target_persd_pwr_rq > p_attacker->U.UPerson.PersuadePower))
        return false;

    // Only player agents require energy to persuade
    *energy_reqd = 0;
    if ((p_attacker->Flag & TngF_PlayerAgent) != 0)
        *energy_reqd = 30 * (target_persd_pwr_rq + 1);
    if (*energy_reqd > 600)
        *energy_reqd = 600;

    // Check if we have enough weapon energy
    if (*energy_reqd > p_attacker->U.UPerson.Energy)
        return false;

    // If under commands to persuade a specific person, accept only that person ignoring anyone else
    if (((target_select == PTargSelect_Persuader) || (target_select == PTargSelect_PersuadeAdv)) &&
      (p_attacker->State == PerSt_PERSUADE_PERSON) && (target != p_attacker->GotoThingIndex))
        return false;

    return true;
}

short process_persuadertron(struct Thing *p_person, ubyte target_select, ushort *energy_reqd)
{
#if 0
    short ret;
    asm volatile ("call ASM_process_persuadertron\n"
        : "=r" (ret) : "a" (p_person), "d" (target_select), "b" (energy_reqd));
    return ret;
#endif
    short cor_x, cor_z;
    short weapon_range;

    cor_x = PRCCOORD_TO_MAPCOORD(p_person->X);
    cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z);

    switch (target_select)
    {
    case PTargSelect_Persuader:
    default:
        weapon_range = get_hand_weapon_range(p_person, WEP_PERSUADRTRN);
        break;
    case PTargSelect_PersuadeAdv:
        weapon_range = get_hand_weapon_range(p_person, WEP_PERSUADER2);
        break;
    case PTargSelect_SoulCollect:
        weapon_range = get_hand_weapon_range(p_person, WEP_SOULGUN);
        break;
    }

    return find_person_which_can_be_persuaded_now(cor_x, cor_z, weapon_range,
      p_person->ThingOffset, target_select, energy_reqd);
}

void get_soul(struct Thing *p_dead, struct Thing *p_person)
{
    asm volatile ("call ASM_get_soul\n"
        : : "a" (p_dead), "d" (p_person));
}

void choose_best_weapon_for_range(struct Thing *p_person, int dist)
{
    asm volatile ("call ASM_choose_best_weapon_for_range\n"
        : : "a" (p_person), "d" (dist));
}

void process_weapon_recoil(struct Thing *p_person)
{
    if (((p_person->Flag2 & TgF2_Unkn0800) == 0) &&
      ((p_person->Flag & (TngF_Unkn20000000|TngF_PlayerAgent)) == (TngF_Unkn20000000|TngF_PlayerAgent)))
    {
        PlayerInfo *p_player;
        ushort plyr, plagent;

        plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
        plagent = p_person->U.UPerson.ComCur & 3;
        p_player = &players[plyr];
        p_person->U.UPerson.ComTimer = -1;
        p_person->VX = p_player->UserVX[plagent];
        p_person->VY = p_player->UserVY[plagent];
        p_person->VZ = p_player->UserVZ[plagent];
    }
    else if ((p_person->Flag & (TngF_Unkn20000000|TngF_Persuaded)) == (TngF_Unkn20000000|TngF_Persuaded))
    {
        PlayerInfo *p_player;
        struct Thing *p_owner;
        ushort plyr;
        short cor_x, cor_y, cor_z;
        short src_x, src_y, src_z;
        int range;

        p_owner = &things[p_person->Owner];
        plyr = (p_owner->U.UPerson.ComCur & 0x1C) >> 2;
        p_player = &players[plyr];
        cor_x = p_player->SpecialItems[0];
        cor_y = p_player->SpecialItems[1];
        cor_z = p_player->SpecialItems[2];

        range = get_weapon_range(p_person);
        src_x = PRCCOORD_TO_MAPCOORD(p_person->X);
        src_y = PRCCOORD_TO_MAPCOORD(p_person->Y);
        src_z = PRCCOORD_TO_MAPCOORD(p_person->Z);
        map_limit_distance_to_target_fast(src_x, src_y, src_z,
          &cor_x, &cor_y, &cor_z, range);

        p_person->U.UPerson.ComTimer = -1;
        p_person->VX = cor_x;
        p_person->VY = cor_y;
        p_person->VZ = cor_z;
    }
}

static void process_energy_recovery(struct Thing *p_person)
{
    if (p_person->U.UPerson.Energy < p_person->U.UPerson.MaxEnergy)
    {
        if ((p_person->Flag2 & TgF2_ExistsOffMap) != 0)
        {
            p_person->U.UPerson.Energy = p_person->U.UPerson.MaxEnergy;
        }
        else if (p_person->U.UPerson.WeaponTurn == 0)
        {
            p_person->U.UPerson.Energy += 2;
        }
    }
}

static void process_health_recovery(struct Thing *p_person)
{
    ulong mask;

    if ((p_person->Flag & TngF_PlayerAgent) != 0)
        mask = 1;
    else
        mask = 7;
    // Chest level 4 means invincible
    if (cybmod_chest_level(&p_person->U.UPerson.UMod) == 4)
    {
        person_set_helath_to_max_limit(p_person);
    }
    if (((mask & gameturn) == 0) && p_person->Health < p_person->U.UPerson.MaxHealth)
    {
        int i;

        i = 4 * (cybmod_chest_level(&p_person->U.UPerson.UMod) + 1);
        if (abs(p_person->U.UPerson.Mood) > 32)
            i >>= 2;
        else if (abs(p_person->U.UPerson.Mood) > 16)
            i >>= 1;
        if (abs(p_person->U.UPerson.Mood) > 64 && (gameturn & 2) != 0)
            i = 0;
        p_person->Health += i;
    }
}

void set_person_weapon_turn(struct Thing *p_person, short n_turn)
{
    if ((p_person->Flag & TngF_PlayerAgent) == 0)
    {
        p_person->U.UPerson.WeaponTurn = n_turn;
    }
    else if ((p_person->Flag2 & TgF2_Unkn0800) == 0)
    {
        PlayerIdx plyr;
        ushort plagent;
        WeaponType wtype;

        plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
        plagent = p_person->U.UPerson.ComCur & 3;
        wtype = p_person->U.UPerson.CurrentWeapon;

        p_person->U.UPerson.WeaponTurn = n_turn;
        player_agent_set_weapon_delay(plyr, plagent, wtype, n_turn);
    }
}

void weapon_consume_energy(struct Thing *p_person, WeaponType wtype)
{
    struct WeaponDef *wdef;
    short en_used;

    wdef = &weapon_defs[wtype];

    switch (wtype)
    {
    case WEP_CLONESHLD:
        if (in_network_game)
            en_used = wdef->EnergyUsed >> 1;
        else
            en_used = wdef->EnergyUsed;
        break;
    default:
        en_used = wdef->EnergyUsed;
        break;
    }
    p_person->U.UPerson.Energy -= en_used;
}

void process_clone_disguise(struct Thing *p_person)
{
    if ((p_person->Flag2 & TgF2_Unkn00400000) == 0)
        return;

    weapon_consume_energy(p_person, WEP_CLONESHLD);

    if (((p_person->Flag & TngF_PlayerAgent) != 0) && (p_person->U.UPerson.Energy < 0))
        p_person->U.UPerson.CurrentWeapon = WEP_NULL;

    if (p_person->U.UPerson.CurrentWeapon != WEP_CLONESHLD)
    {
        reset_clone_disguise(p_person);
    }
}

TbBool person_weapons_remove_one(struct Thing *p_person, WeaponType wtype)
{
    PlayerInfo *p_player;
    ushort plagent;
    TbBool done;

    p_player = NULL;
    plagent = 0;
    if ((p_person->Flag & TngF_PlayerAgent) != 0)
    {
        PlayerIdx plyr;
        plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
        plagent = p_person->U.UPerson.ComCur & 3;
        p_player = &players[plyr];
    }

    if (p_player != NULL)
        //TODO replace  with weapons_remove_one() call, when FourPacks have unified format
        done = weapons_remove_one_for_player(&p_person->U.UPerson.WeaponsCarried, p_player->FourPacks, plagent, wtype);
    else
        done = weapons_remove_one_from_npc(&p_person->U.UPerson.WeaponsCarried, wtype);

    // If the weapon got depleted, remove it from all properties
    if (!person_carries_weapon(p_person, wtype)) {
        if (p_person->U.UPerson.CurrentWeapon == wtype)
            p_person->U.UPerson.CurrentWeapon = WEP_NULL;
        if ((p_player != NULL) && (p_player->PrevWeapon[plagent] == wtype))
            person_weapons_reset_previous(p_person);
    }

    return done;
}

void person_weapons_reset_previous(struct Thing *p_person)
{
    PlayerInfo *p_player;
    ushort plagent;

    p_player = NULL;
    plagent = 0;
    if ((p_person->Flag & TngF_PlayerAgent) != 0)
    {
        PlayerIdx plyr;
        plyr = (p_person->U.UPerson.ComCur & 0x1C) >> 2;
        plagent = p_person->U.UPerson.ComCur & 3;
        p_player = &players[plyr];
    }

    if (p_player != NULL) {
        p_player->PrevWeapon[plagent] = find_nth_weapon_held(p_person->ThingOffset, 1);
    }

}

void process_automedkit(struct Thing *p_person)
{
    if (!weapons_has_weapon(p_person->U.UPerson.WeaponsCarried, WEP_MEDI2))
        return;
    if (p_person->Health >= p_person->U.UPerson.MaxHealth / 8)
        return;

    p_person->Health = p_person->U.UPerson.MaxHealth;
    person_weapons_remove_one(p_person, WEP_MEDI2);
    play_dist_sample(p_person, 2, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
}

void low_energy_alarm_stop(void)
{
    if (IsSamplePlaying(0, 93, 0))
        stop_sample_using_heap(0, 93);
}

void low_energy_alarm_play(void)
{
    if (!IsSamplePlaying(0, 93, 0))
        play_sample_using_heap(0, 93, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
}

void process_energy_alarm(struct Thing *p_person)
{
    ThingIdx dcthing;

    if ((p_person->Flag & TngF_Unkn1000) == 0)
        return;

    dcthing = players[local_player_no].DirectControl[0];
    if (p_person->ThingOffset == dcthing)
    {
        if (p_person->U.UPerson.Energy >= low_energy_alarm_level)
        {
            if ((gameturn & 7) == 0)
                low_energy_alarm_stop();
        }
        else
        {
            low_energy_alarm_play();
        }
    }
}

void process_move_while_firing(struct Thing *p_person)
{
    struct Thing *p_vehicle;
    ubyte currWeapon;

    if ((p_person->Flag & TngF_Unkn0800) == 0)
        return;

    currWeapon = p_person->U.UPerson.CurrentWeapon;
    if (currWeapon != 0 && currWeapon != WEP_RAZORWIRE
        && currWeapon != WEP_EXPLWIRE && (p_person->Flag2 & TgF2_Unkn00080000) != 0)
    {
        p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
        reset_person_frame(p_person);
        p_person->Timer1 = 48;
        p_person->StartTimer1 = 48;
        p_person->Flag2 &= ~TgF2_Unkn00080000;
        p_person->Speed = calc_person_speed(p_person);
    }
    p_vehicle = &things[p_person->U.UPerson.Vehicle];
    if (((p_person->Flag & TngF_InVehicle) != 0) && (p_vehicle->State == VehSt_UNKN_45))
    {
        p_person->Flag &= ~TngF_Unkn0800;
    }
    else
    {
        if ((p_person->Flag & TngF_Unkn20000000) == 0)
            check_persons_target(p_person);
        if (p_person->U.UPerson.Target2 != 0)
        {
            check_persons_target2(p_person);
        }
        else if ((p_person->Type != TT_MINE) && (p_person->Flag & TngF_PlayerAgent) == 0)
        {
            struct Thing *p_target;

            p_target = p_person->PTarget;
            if (p_target != NULL)
            {
                int dt_x, dt_z;
                ushort i;

                dt_x = PRCCOORD_TO_MAPCOORD(p_target->X - p_person->X);
                dt_z = PRCCOORD_TO_MAPCOORD(p_target->Z - p_person->Z);
                i = (arctan(dt_x, -dt_z) + 128) & 0x7FF;
                change_player_angle(p_person, i >> 8);
            }
        }
    }
}

void process_weapon_wind_down(struct Thing *p_person)
{
    ushort plagent;

    if ((p_person->Flag2 & TgF2_Unkn0200) == 0)
        return;

    switch (p_person->U.UPerson.CurrentWeapon)
    {
    case WEP_UZI:
        if ((p_person->Flag & TngF_PlayerAgent) != 0)
            plagent = p_person->U.UPerson.ComCur & 3;
        else
            plagent = 0;
        ReleaseLoopedSample(p_person->ThingOffset, 80 + plagent);
        p_person->Flag2 &= ~TgF2_Unkn0200;
        break;
    case WEP_MINIGUN:
        if ((p_person->Flag & TngF_PlayerAgent) != 0)
            plagent = p_person->U.UPerson.ComCur & 3;
        else
            plagent = 0;
        ReleaseLoopedSample(p_person->ThingOffset, 84 + plagent);
        plagent = p_person->U.UPerson.ComCur & 3;
        play_dist_sample(p_person, 88 + plagent, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        p_person->Flag2 &= ~TgF2_Unkn0200;
        break;
    }
}

void process_weapon_continuous_fire(struct Thing *p_person)
{
    struct WeaponDef *wdef;
    short prevWepTurn;

    wdef = &weapon_defs[p_person->U.UPerson.CurrentWeapon];
    prevWepTurn = p_person->U.UPerson.WeaponTurn;
    if ((p_person->Flag & TngF_PlayerAgent) != 0)
        p_person->U.UPerson.WeaponTurn = player_weapon_time(p_person);
    if (prevWepTurn)
    {
        if ((p_person->Flag & TngF_PlayerAgent) == 0)
            p_person->U.UPerson.WeaponTurn--;
        if (p_person->U.UPerson.WeaponTurn == 0)
        {
            int animMode;

            animMode = p_person->U.UPerson.AnimMode;
            if ((animMode == ANIM_PERS_Unkn14 || animMode == ANIM_PERS_WEPHEAVY_Unkn15)
              && p_person->Type != TT_MINE)
            {
                p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
                reset_person_frame(p_person);
            }
            switch (p_person->U.UPerson.CurrentWeapon)
            {
            case WEP_UZI:
                if ((p_person->U.UPerson.Energy <= wdef->EnergyUsed) ||
                  ((p_person->Flag & TngF_Unkn0800) == 0))
                {
                    ushort plagent;

                    if ((p_person->Flag & TngF_PlayerAgent) != 0)
                        plagent = p_person->U.UPerson.ComCur & 3;
                    else
                        plagent = 0;
                    ReleaseLoopedSample(p_person->ThingOffset, 80 + plagent);
                    p_person->Flag2 &= ~TgF2_Unkn0200;
                }
                break;
            case WEP_MINIGUN:
                if ((p_person->U.UPerson.Energy <= wdef->EnergyUsed) ||
                  ((p_person->Flag & TngF_Unkn0800) == 0))
                {
                    ushort plagent;

                    if ((p_person->Flag & TngF_PlayerAgent) != 0)
                        plagent = p_person->U.UPerson.ComCur & 3;
                    else
                        plagent = 0;
                    ReleaseLoopedSample(p_person->ThingOffset, 84 + plagent);
                    plagent = p_person->U.UPerson.ComCur & 3;
                    play_dist_sample(p_person, 88 + plagent, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
                    p_person->Flag2 &= ~TgF2_Unkn0200;
                }
                break;
            case WEP_FLAMER:
                if ((p_person->U.UPerson.Energy <= wdef->EnergyUsed) ||
                  ((p_person->Flag & TngF_Unkn0800) == 0))
                {
                    stop_sample_using_heap(p_person->ThingOffset, 14);
                    play_dist_sample(p_person, 15, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
                    p_person->Flag2 &= ~TgF2_Unkn0200;
                }
                break;
            }
        }
    }
}

void process_weapon_wield_affecting_area(struct Thing *p_person, WeaponType wtype)
{
    ThingIdx targtng;
    ushort energy_rq;

    switch (wtype)
    {
    case WEP_SOULGUN:
        if ((p_person->Health < 2 * p_person->U.UPerson.MaxHealth) &&
          (((gameturn + p_person->ThingOffset) & 7) == 0))
        {
            targtng = process_persuadertron(p_person, PTargSelect_SoulCollect, &energy_rq);
            if (targtng > 0)
                get_soul(&things[targtng], p_person);
        }
        return;
    case WEP_PERSUADRTRN:
        {
            targtng = process_persuadertron(p_person, PTargSelect_Persuader, &energy_rq);
            if (targtng > 0)
                set_person_persuaded(&things[targtng], p_person, energy_rq);
        }
        return;
    case WEP_PERSUADER2:
        {
            targtng = process_persuadertron(p_person, PTargSelect_PersuadeAdv, &energy_rq);
            if (targtng > 0)
                set_person_persuaded(&things[targtng], p_person, energy_rq);
        }
        return;
    case WEP_H2HTASER:
        init_taser(p_person);
        break;
    case WEP_CLONESHLD:
        init_clone_disguise(p_person);
        break;
    default:
        break;
    }
}

void weapon_init_shot(struct Thing *p_person, WeaponType wtype)
{
    switch (wtype)
    {
    case WEP_LASER:
        if (p_person->SubType == SubTT_PERS_MECH_SPIDER)
            init_laser_6shot(p_person, p_person->U.UPerson.WeaponTimer);
        else
            init_laser(p_person, p_person->U.UPerson.WeaponTimer);
        stop_sample_using_heap(p_person->ThingOffset, 7);
        stop_sample_using_heap(p_person->ThingOffset, 52);
        play_dist_sample(p_person, 18, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    case WEP_ELLASER:
        init_laser_elec(p_person, p_person->U.UPerson.WeaponTimer);
        stop_sample_using_heap(p_person->ThingOffset, 7);
        stop_sample_using_heap(p_person->ThingOffset, 52);
        if ((p_person->Flag2 & TgF2_ExistsOffMap) == 0)
            play_dist_sample(p_person, 6, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    case WEP_RAP:
        init_rocket(p_person);
        stop_sample_using_heap(p_person->ThingOffset, 7);
        stop_sample_using_heap(p_person->ThingOffset, 52);
        break;
    case WEP_BEAM:
         init_laser_beam(p_person, p_person->U.UPerson.WeaponTimer, 17);
         stop_sample_using_heap(p_person->ThingOffset, 7);
         stop_sample_using_heap(p_person->ThingOffset, 52);
         play_dist_sample(p_person, 5, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    case WEP_QDEVASTATOR:
        init_laser_q_sep(p_person, p_person->U.UPerson.WeaponTimer);
        stop_sample_using_heap(p_person->ThingOffset, 7);
        stop_sample_using_heap(p_person->ThingOffset, 52);
        play_dist_sample(p_person, 28, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    default:
        break;
    }
}

void process_wielded_weapon_fire(struct Thing *p_person, WeaponType wtype)
{
    struct WeaponDef *wdef;
    short wepTurn;

    wdef = &weapon_defs[wtype];
    wepTurn = p_person->U.UPerson.WeaponTurn;
    if ((wepTurn == 0) || (wepTurn < wdef->ReFireDelay - 6))
        p_person->U.UPerson.FrameId.Version[4] = 0;

    switch (wtype)
    {
    case WEP_EXPLWIRE:
        if ((p_person->Flag2 & TgF2_Unkn0001) == 0)
        {
            if ((p_person->Flag & TngF_Unkn0800) != 0)
                init_razor_wire(p_person, 1);
        }
        else if (((p_person->Flag & TngF_Unkn0800) != 0)
            || ((p_person->Flag2 & TgF2_Unkn0004) != 0))
        {
            update_razor_wire(p_person);
        }
        else
        {
            finalise_razor_wire(p_person);
        }
        break;
    case WEP_RAZORWIRE:
        if ((p_person->Flag2 & TgF2_Unkn0001) == 0)
        {
            if ((p_person->Flag & TngF_Unkn0800) != 0)
                init_razor_wire(p_person, 0);
        }
        else if (((p_person->Flag & TngF_Unkn0800) != 0)
            || ((p_person->Flag2 & TgF2_Unkn0004) != 0))
        {
            update_razor_wire(p_person);
        }
        else
        {
            finalise_razor_wire(p_person);
        }
        break;
    case WEP_FLAMER:
        if (p_person->U.UPerson.WeaponTurn == 1)
        {
            if ((p_person->Flag & TngF_PlayerAgent) == 0)
                p_person->Flag &= ~TngF_Unkn0800;
        }
        if (p_person->U.UPerson.WeaponTurn <= 1)
        {
            if ((p_person->Flag & TngF_Unkn0800) != 0)
            {
                if ((p_person->U.UPerson.WeaponTurn == 0)
                    && ((p_person->Flag2 & TgF2_Unkn0400) == 0))
                {
                    play_dist_sample(p_person, 14, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 2);
                    play_dist_sample(p_person, 13, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
                    p_person->Flag2 |= TgF2_Unkn0200;
                }
                init_fire_weapon(p_person);
                p_person->U.UPerson.WeaponTurn = 5;
            }
            else
            {
                p_person->U.UPerson.WeaponDir = 32
                    * p_person->U.UPerson.Angle;
            }
        }
        else
        {
            init_fire_weapon(p_person);
        }
        break;
    default:
        if (((p_person->Flag & TngF_Unkn0800) != 0) && (p_person->U.UPerson.CurrentWeapon != WEP_NULL)
            && (p_person->U.UPerson.WeaponTurn == 0) && ((p_person->Flag & TngF_WepCharging) == 0))
        {
            process_weapon_recoil(p_person);
            init_fire_weapon(p_person);
        }
        break;
    }
}

void weapon_inc_timer(struct Thing *p_person, WeaponType wtype)
{
    p_person->U.UPerson.WeaponTimer++;
    if (wtype == WEP_RAP)
        p_person->U.UPerson.WeaponTimer++;
}

void weapon_set_timer_for_refire(struct Thing *p_person, WeaponType wtype)
{
    struct WeaponDef *wdef;
    int i;
    short reFireShift;

    wdef = &weapon_defs[wtype];

    reFireShift = 0;
    if (p_person->U.UPerson.WeaponTimer >= 5)
    {
        i = p_person->U.UPerson.WeaponTimer - 3;
        set_person_weapon_turn(p_person, i);
    }
    else
    {
        i = p_person->U.UPerson.WeaponTimer;
        set_person_weapon_turn(p_person, i);
        p_person->U.UPerson.WeaponTimer = 5;
        reFireShift = 5 - i;
    }

    switch (wtype)
    {
    case WEP_LASER:
        if (p_person->SubType == SubTT_PERS_MECH_SPIDER)
            p_person->U.UPerson.WeaponTurn = 1;
        else
            p_person->U.UPerson.WeaponTurn = reFireShift + wdef->ReFireDelay;
        break;
    case WEP_RAP:
    case WEP_ELLASER:
    case WEP_BEAM:
    case WEP_QDEVASTATOR:
        p_person->U.UPerson.WeaponTurn = reFireShift + wdef->ReFireDelay;
        break;
    default:
        break;
    }
    // We probably updated WeaponTurn, now do the update properly
    set_person_weapon_turn(p_person, p_person->U.UPerson.WeaponTurn);
}

void process_wielded_weapon(struct Thing *p_person)
{
    WeaponType wtype;

    wtype = p_person->U.UPerson.CurrentWeapon;

    process_weapon_wield_affecting_area(p_person, wtype);

    process_clone_disguise(p_person);

    process_wielded_weapon_fire(p_person, wtype);

    if ((p_person->Flag & TngF_WepCharging) != 0)
    {
        if (!weapon_has_targetting(wtype))
        {
            if (p_person->U.UPerson.WeaponTimer > 15)
                p_person->U.UPerson.WeaponTimer = 15;
            else
                p_person->U.UPerson.Energy -= 8;
        }

        switch (wtype)
        {
        case WEP_LASER:
            p_person->U.UPerson.FrameId.Version[4] = 2;
            break;
        case WEP_RAP:
            p_person->U.UPerson.FrameId.Version[4] = 0;
            break;
        default:
            p_person->U.UPerson.FrameId.Version[4] = 4;
            break;
        }

        if ((p_person->Flag & TngF_Unkn0800) != 0)
        {
            if (p_person->U.UPerson.WeaponTimer == 14)
            {
                stop_sample_using_heap(p_person->ThingOffset, 7);
                play_dist_sample(p_person, 52, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
            }
            else
            {
                SetSamplePitch(p_person->ThingOffset, 7,
                    2 * p_person->U.UPerson.WeaponTimer + 100);
            }
            weapon_inc_timer(p_person, wtype);

            if ((p_person->Flag & (TngF_Unkn20000000|TngF_PlayerAgent)) != (TngF_Unkn20000000|TngF_PlayerAgent))
            {
                int resp_time;

                if ((p_person->Health < (p_person->U.UPerson.MaxHealth >> 1)
                    || ((p_person->Flag & TngF_InVehicle) != 0)
                    || (p_person->ThingOffset & 3) != 0)
                    && ((p_person->Flag & TngF_PlayerAgent) == 0))
                {
                    resp_time = 5;
                }
                else
                {
                    resp_time = 4 * (cybmod_brain_level(&p_person->U.UPerson.UMod) + 1);
                    if (resp_time > 15)
                        resp_time = 15;
                }
                if (p_person->U.UPerson.WeaponTimer >= resp_time)
                {
                    if ((p_person->Flag & TngF_Unkn1000) == 0)
                        p_person->Flag &= ~TngF_Unkn0800;
                }
            }
            if (weapon_has_targetting(wtype) && (p_person->PTarget == NULL))
                p_person->U.UPerson.WeaponTimer = 0;
        }

        if ((p_person->Flag & TngF_Unkn0800) == 0)
        {
            process_weapon_recoil(p_person);

            weapon_init_shot(p_person, wtype);

            if (wtype == WEP_RAP)
                weapon_consume_energy(p_person, wtype);

            weapon_set_timer_for_refire(p_person, wtype);

            p_person->Flag &= ~TngF_WepCharging;
            if ((p_person->U.UPerson.WeaponTimer > 5)
              && (wtype != WEP_RAP))
                p_person->Flag |= TngF_StationrSht;
        }
    }
}

void process_weapon(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_process_weapon\n"
        : : "a" (p_person));
    return;
#endif
    process_energy_alarm(p_person);

    p_person->U.UPerson.Flag3 &= ~PrsF3_Unkn40;

    process_move_while_firing(p_person);

    if (((p_person->Flag & TngF_Persuaded) != 0) || (p_person->State == PerSt_PROTECT_PERSON))
    {
        struct Thing *p_owner;
        struct Thing *p_target;

        p_owner = &things[p_person->Owner];
        p_target = p_owner->PTarget;
        if (p_target != NULL)
        {
            if ((p_target->State == PerSt_DEAD) && (p_owner->Flag & (TngF_WepCharging|TngF_Unkn0800)) != 0)
            {
                p_person->Flag |= TngF_Unkn0800;
                p_person->PTarget = p_target;
                p_person->Flag &= ~TngF_Unkn20000000;
            }
        }
    }
    process_automedkit(p_person);

    if (p_person->U.UPerson.WeaponTurn == 0)
        process_weapon_wind_down(p_person);

    if (p_person->U.UPerson.MaxStamina != 0)
        process_stamina(p_person);
    process_energy_recovery(p_person);
    process_health_recovery(p_person);

    process_weapon_continuous_fire(p_person);

    if ((p_person->Flag & TngF_Unkn40000000) == 0)
    {
        if ((p_person->Flag & TngF_InVehicle) != 0)
        {
            struct Thing *p_vehicle;

            p_vehicle = &things[p_person->U.UPerson.Vehicle];
            if ((p_vehicle->Flag2 & TgF2_Unkn0002) != 0)
                return;
            switch (p_vehicle->SubType)
            {
            case SubTT_VEH_TANK:
                process_vehicle_weapon(p_vehicle, p_person);
                return;
            case SubTT_VEH_MECH:
                process_mech_weapon(p_vehicle, p_person);
                return;
            default:
                break;
            }
        }
        process_wielded_weapon(p_person);
    }
}

s32 laser_hit_at(s32 x1, s32 y1, s32 z1, s32 *x2, s32 *y2, s32 *z2, struct Thing *p_shot)
{
    s32 ret;
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_laser_hit_at\n"
        : "=r" (ret) : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2), "g" (p_shot));
    return ret;
}

/******************************************************************************/
