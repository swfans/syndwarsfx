/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file rules.c
 *     Ingame rules loading from file.
 * @par Purpose:
 *     Implement functions for reading file with in-game rules.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Sep 2023 - 17 Mar 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "rules.h"

#include "bffile.h"
#include "bfini.h"
#include "bfmemory.h"

#include "enginpeff.h"
#include "enginzoom.h"
#include "game_data.h"
#include "hud_target.h"
#include "research.h"
#include "scanner.h"
#include "swlog.h"
#include "weapon.h"
/******************************************************************************/

enum RulesEngineConfigCmd {
    REnginCmd_ZoomMin = 1,
    REnginCmd_ZoomMax,
};

const struct TbNamedEnum rules_conf_engine_cmnds[] = {
  {"ZoomMin",		REnginCmd_ZoomMin},
  {"ZoomMax",		REnginCmd_ZoomMax},
  {NULL,			0},
};

enum RulesScannerConfigCmd {
    RScanrCmd_BaseZoomFactor = 1,
    RScanrCmd_UserZoomFactor,
    RScanrCmd_ScaleDotsCircles,
};

const struct TbNamedEnum rules_conf_scanner_cmnds[] = {
  {"BaseZoomFactor",RScanrCmd_BaseZoomFactor},
  {"UserZoomFactor",RScanrCmd_UserZoomFactor},
  {"ScaleDotsCircles",RScanrCmd_ScaleDotsCircles},
  {NULL,			0},
};

enum RulesInterfaceonfigCmd {
    RIfaceCmd_ShowTargetHealth = 1,
};

const struct TbNamedEnum rules_conf_interface_cmnds[] = {
  {"ShowTargetHealth",				RIfaceCmd_ShowTargetHealth},
  {NULL,							0},
};

enum RulesResearchConfigCmd {
    RResrchCmd_DailyScientistDeathChance = 1,
    RResrchCmd_ScientistsPerGroup,
    RResrchCmd_WeaponDonateResearchIncrPermil,
    RResrchCmd_DailyProgressRtcMinutes,
};

enum RulesRevenueConfigCmd {
    RRevenuCmd_PersuadedPersonWeaponsSellCostPermil = 1,
};

enum RulesAtmosphericConfigCmd {
    RAtmosCmd_RainDropMaxWidth = 1,
    RAtmosCmd_SnowFlakeMaxSize,
    RAtmosCmd_StarMaxSize,
};

const struct TbNamedEnum rules_conf_research_cmnds[] = {
  {"DailyScientistDeathChancePermil",	RResrchCmd_DailyScientistDeathChance},
  {"ScientistsPerGroup",			RResrchCmd_ScientistsPerGroup},
  {"WeaponDonateResearchIncrPermil",	RResrchCmd_WeaponDonateResearchIncrPermil},
  {"DailyProgressRtcMinutes",		RResrchCmd_DailyProgressRtcMinutes},
  {NULL,							0},
};

const struct TbNamedEnum rules_conf_revenue_cmnds[] = {
  {"PersuadedPersonWeaponsSellCostPermil",	RRevenuCmd_PersuadedPersonWeaponsSellCostPermil},
  {NULL,							0},
};

const struct TbNamedEnum rules_conf_atmospheric_cmnds[] = {
  {"RainDropMaxWidth",				RAtmosCmd_RainDropMaxWidth},
  {"SnowFlakeMaxSize",				RAtmosCmd_SnowFlakeMaxSize},
  {"StarMaxSize",					RAtmosCmd_StarMaxSize},
  {NULL,							0},
};

const struct TbNamedEnum rules_conf_any_bool[] = {
  {"True",			1},
  {"False",			2},
  {NULL,			0},
};

TbBool read_rules_file(void)
{
    //char locbuf[320];
    char conf_fname[DISKPATH_SIZE];
    PathInfo *pinfo;
    char *conf_buf;
    TbFileHandle conf_fh;
    TbBool done;
    int i;
    long k;
    struct TbIniParser parser;
    int conf_len;
    short zoom_min, zoom_max;

    pinfo = &game_dirs[DirPlace_Config];
    snprintf(conf_fname, DISKPATH_SIZE-1, "%s/rules.ini", pinfo->directory);
    conf_fh = LbFileOpen(conf_fname, Lb_FILE_MODE_READ_ONLY);
    if (conf_fh != INVALID_FILE) {
        conf_len = LbFileLengthHandle(conf_fh);
        if (conf_len > 1024*1024)
            conf_len = 1024*1024;
        conf_buf = LbMemoryAlloc(conf_len+16);
        conf_len = LbFileRead(conf_fh, conf_buf, conf_len);
        LOGSYNC("Processing '%s' file, %d bytes", conf_fname, conf_len);
        LbFileClose(conf_fh);
    } else {
        LOGERR("Could not open '%s' file, rules left at default.", conf_fname);
        conf_buf = LbMemoryAlloc(16);
        conf_len = 0;
    }
    conf_buf[conf_len] = '\0';
    LbIniParseStart(&parser, conf_buf, conf_len);
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
    zoom_min = user_zoom_min;
    zoom_max = user_zoom_max;

    // Parse the [engine] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "engine") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section.", "engine");
        done = true;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(rules_conf_engine_cmnds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, rules_conf_engine_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case REnginCmd_ZoomMin:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            zoom_min = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)zoom_min);
            break;
        case REnginCmd_ZoomMax:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            zoom_max = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)zoom_max);
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

    // Parse the [scanner] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "scanner") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section.", "scanner");
        done = true;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(rules_conf_scanner_cmnds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, rules_conf_scanner_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case RScanrCmd_BaseZoomFactor:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            SCANNER_base_zoom_factor = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)SCANNER_base_zoom_factor);
            break;
        case RScanrCmd_UserZoomFactor:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            SCANNER_user_zoom_factor = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)SCANNER_user_zoom_factor);
            break;
        case RScanrCmd_ScaleDotsCircles:
            i = LbIniValueGetNamedEnum(&parser, rules_conf_any_bool);
            if (i <= 0) {
                CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            SCANNER_scale_dots = (i == 1);
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)SCANNER_scale_dots);
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

    // Parse the [interface] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "interface") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section.", "interface");
        done = true;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(rules_conf_interface_cmnds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, rules_conf_interface_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case RIfaceCmd_ShowTargetHealth:
            i = LbIniValueGetNamedEnum(&parser, rules_conf_any_bool);
            if (i <= 0) {
                CONFWRNLOG("Could not recognize \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            hud_show_target_health = (i == 1);
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)hud_show_target_health);
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

    // Parse the [research] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "research") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section.", "research");
        done = true;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(rules_conf_research_cmnds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, rules_conf_research_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case RResrchCmd_DailyScientistDeathChance:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            daily_scientist_death_chance_permil = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)daily_scientist_death_chance_permil);
            break;
        case RResrchCmd_ScientistsPerGroup:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            scientists_per_group = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)scientists_per_group);
            break;
        case RResrchCmd_WeaponDonateResearchIncrPermil:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            weapon_donate_research_incr_permil = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)weapon_donate_research_incr_permil);
            break;
        case RResrchCmd_DailyProgressRtcMinutes:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            research_progress_rtc_minutes = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)research_progress_rtc_minutes);
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

    // Parse the [revenue] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "revenue") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section.", "revenue");
        done = true;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(rules_conf_revenue_cmnds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, rules_conf_revenue_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case RRevenuCmd_PersuadedPersonWeaponsSellCostPermil:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            persuaded_person_weapons_sell_cost_permil = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)persuaded_person_weapons_sell_cost_permil);
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

    // Parse the [atmospheric] section of loaded file
    done = false;
    if (LbIniFindSection(&parser, "atmospheric") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section.", "atmospheric");
        done = true;
    }
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(rules_conf_atmospheric_cmnds,cmd_num)
    while (!done)
    {
        int cmd_num;

        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, rules_conf_atmospheric_cmnds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case RAtmosCmd_RainDropMaxWidth:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            rain_drop_max_width = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)rain_drop_max_width);
            break;
        case RAtmosCmd_SnowFlakeMaxSize:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            snow_flake_max_size = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)snow_flake_max_size);
            break;
        case RAtmosCmd_StarMaxSize:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            star_max_size = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)star_max_size);
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

    zoom_update(zoom_min, zoom_max);
    LbIniParseEnd(&parser);
    LbMemoryFree(conf_buf);
    return true;
}
/******************************************************************************/
