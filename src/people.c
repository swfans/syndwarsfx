/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file people.c
 *     Routines implementing people.
 * @par Purpose:
 *     Implement general functions for handling person things.
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
#include "people.h"

#include "bfmath.h"
#include "bfmemory.h"
#include "bffile.h"
#include "bfini.h"
#include "bfutility.h"
#include "ssampply.h"
#include <assert.h>
#include <stdlib.h>

#include "engincolour.h"
#include "drawtext.h"

#include "bmbang.h"
#include "bigmap.h"
#include "building.h"
#include "command.h"
#include "drawtext_wrp.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintrns.h"
#include "febrief.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "game_speed.h"
#include "lvdraw3d.h"
#include "lvobjctv.h"
#include "misstat.h"
#include "pepgroup.h"
#include "player.h"
#include "scandraw.h"
#include "sound.h"
#include "thing.h"
#include "thing_fire.h"
#include "thing_onface.h"
#include "thing_search.h"
#include "tngcolisn.h"
#include "vehicle.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/
struct PeepStat peep_type_stats[] = {
    {   0,    0,    0,    0,   0, 0,   0, 0, 0, 0},
    {1000, 1024, 2048, 1024,  20, 5, 512, 0, 0, 0},
    {1000, 1024, 2048, 1024,  20, 1, 512, 0, 0, 0},
    { 600,  524, 2048,  601,  10, 2, 312, 0, 0, 0},
    { 100,  100,  400,  300,   0, 1, 172, 0, 0, 0},
    { 100,  100,  400,  300,   0, 1, 160, 0, 0, 0},
    { 600,  600,  900,  601,  15, 3, 312, 0, 0, 0},
    {4000, 1000,  800, 2001,9999, 2, 362, 0, 0, 0},
    { 600,  400,  800,  601,   6, 2, 362, 0, 0, 0},
    { 600,  524, 2024,  601,  10, 2, 312, 0, 0, 0},
    { 200,  100,  300,  200,   0, 1, 212, 0, 0, 0},
    { 600,  524, 2024,  601,   0, 1, 312, 0, 0, 0},
    {2000, 1024, 2048, 1024,  20, 5, 512, 0, 0, 0},
    { 100,  100,  400,  300,   0, 1, 150, 0, 0, 0},
    { 110,  100,  400,  300,   0, 1, 212, 0, 0, 0},
    { 100,  100,  400,  300,   0, 1, 212, 0, 0, 0},
    {   0,    0,    0,    0,   0, 0,   0, 0, 0, 0},
};

ushort people_frames[SubTT_PERS_COUNT][ANIM_PERS_TOTAL_COUNT] = {
  {   1,   33,   33,    1,   40,   40,   33,   33,   33,   33,   73,   65,    9,   41,   25,   25,   57, 1010, 1018,   17, 1200,   49,},
  {   1,   33,   33,    1,   40,   40,   33,   33,   33,   33,   73,   65,    9,   41,   25,   25,   57, 1010, 1018,   17, 1200,   49,},
  { 113,  113,  113,   81,  113,  113,  113,  113,  113,  113,  153,  145,   89,  121,  105,  105,  137, 1010, 1018,   97, 1212,  129,},
  { 241,  273,  273,    1,   40,   40,  273,  273,  273,  273,  313,  305,  249,  281,  265,  265,  297, 1010, 1018,  241, 1236,  289,},
  { 321,  353,  353,  321,  353,  353,  353,  353,  353,  353,  393,  385,  329,  361,  345,  345,  377, 1010, 1018,  337, 1248,  369,},
  { 401,  433,  433,    1,  433,  433,  433,  433,  433,  433,  473,  465,  409,  441,  425,  425,  457, 1010, 1018,  401, 1260,  449,},
  { 561,  593,  593,  561,  593,  593,  593,  593,  593,  593,  633,  625,  569,  601,  593,  593,  617, 1010, 1018,  577, 1415,  609,},
  {1040, 1040, 1040,    1,   40,   40, 1040, 1040, 1040, 1040,  313,  313, 1018, 1040, 1040, 1040, 1040, 1010, 1018, 1040, 1040, 1040,},
  { 481,  513,  513,  513,  513,  513,  513,  513,  513,  513,  553,  545,  489,  521,  505,  505,  537, 1010, 1018,  497, 1272,  529,},
  { 161,  193,  193,  161,  193,  193,  193,  193,  193,  193,  233,  225,  169,  201,  185,  185,  217, 1010, 1018,  177, 1224,  209,},
  { 641,  673,  673,  673,  673,  673,  673,  673,  673,  673,  713,  705,  649,  665,  689,  689,  697, 1010, 1018,  657, 1295,  689,},
  { 721,  753,  753,  753,  753,  753,  753,  753,  753,  753,  713,  785,  729,  761,  753,  753,  777, 1010, 1018,  737, 1307,  769,},
  { 801,  833,  833,  833,  833,  833,  833,  833,  833,  833,  873,  865,  809,  841,  825,  825,  857, 1010, 1018,  817, 1319,  825,},
  {1331, 1363, 1363, 1363, 1363, 1363, 1363, 1363, 1363, 1363, 1395, 1387, 1339, 1371, 1355, 1355, 1379, 1010, 1018, 1331, 1403, 1347,},
  {1427, 1459, 1459, 1459, 1459, 1459, 1459, 1459, 1459, 1459, 1491, 1483, 1435, 1467, 1451, 1451, 1475, 1010, 1018, 1427, 1499, 1443,},
  {1331, 1363, 1363, 1363, 1363, 1363, 1363, 1363, 1363, 1363, 1395, 1387, 1339, 1371, 1355, 1355, 1379, 1010, 1018, 1331, 1403, 1331,},
};

/** Configuration options for each person state.
 */
struct PeepStateConfig person_states[] = {
  {"NONE_STATE",},
  {"GOTO_POINT",},
  {"BOMB_BUILDING",},
  {"WANDER",},
  {"EXPLODE_EFFECT",},
  {"WAIT",},
  {"AGENT_DEFENSIVE",},
  {"PICKUP_ITEM",},
  {"DROP_ITEM",},
  {"SHOOT_PERS_WT_BG",},
  {"SHOOT_BGUN_AT",},
  {"DIE",},
  {"DIEING",},
  {"DEAD",},
  {"INIT_SAVE_VICTIM",},
  {"SAVE_VICTIM",},
  {"GOTO_PERSON",},
  {"WAIT_P_DEAD",},
  {"WAIT_ALL_G_DEAD",},
  {"WAIT_MEM_G_DEAD",},
  {"WAIT_P_NEAR",},
  {"WAIT_MEM_G_NEAR",},
  {"WAIT_P_ARRIVES",},
  {"WAIT_MEM_G_ARRIV",},
  {"WAIT_TIME",},
  {"DEFINE_ROUTE",},
  {"STATE_UNUSED_1A",},
  {"KILL_PERSON",},
  {"BLOCK_PERSON",},
  {"SCARE_PERSON",},
  {"USE_ITEM",},
  {"STATIONARY_SHOT",},
  {"RECOIL_HIT",},
  {"VEHICLE_PARKED",},
  {"VEHICLE_DRIVEN",},
  {"GOTO_POINT_REL",},
  {"PERSON_BURNING",},
  {"IN_VEHICLE",},
  {"DRIVING_VEHICLE",},
  {"V_GOTO_POINT_FLY",},
  {"PERSUADE_PERSON",},
  {"FOLLOW_PERSON",},
  {"SUPPORT_PERSON",},
  {"PROTECT_PERSON",},
  {"GET_ITEM",},
  {"USE_VEHICLE",},
  {"USE_VEH_TO_POINT",},
  {"WAIT_VEHICLE",},
  {"GOTO_VEHICLE",},
  {"CATCH_TRAIN",},
  {"WAIT_TRAIN",},
  {"DESTROY_BUILDING",},
  {"WANDER_DRIVE",},
  {"GO_PLANT_MINE",},
  {"WAIT_TO_EXIT_VEH",},
  {"CATCH_FERRY",},
  {"EXIT_FERRY",},
  {"AVOID_GROUP",},
  {"INIT_COMMAND",},
  {"BEING_PERSUADED",},
};

struct PeepStatAdd peep_type_stats_a[SubTT_PERS_COUNT] = {0};
struct TbNamedEnum peep_names[SubTT_PERS_COUNT] = {0};

enum PeepTypesConfigCmd {
    PpTyp_PersonTypesCount = 1,
    PpTyp_Name,
    PpTyp_MaxHealth,
    PpTyp_MaxShield,
    PpTyp_MaximumStamina,
    PpTyp_MaxEnergy,
    PpTyp_PersuadeReqd,
    PpTyp_PersuadeWorth,
    PpTyp_Speed,

};

const struct TbNamedEnum people_conf_common_cmds[] = {
  {"PersonTypesCount",	PpTyp_PersonTypesCount},
  {NULL,		0},
};

const struct TbNamedEnum people_conf_person_cmds[] = {
  {"Name",			PpTyp_Name},
  {"MaxHealth",		PpTyp_MaxHealth},
  {"MaxShield",		PpTyp_MaxShield},
  {"MaximumStamina",PpTyp_MaximumStamina},
  {"MaxEnergy",		PpTyp_MaxEnergy},
  {"PersuadeRequired",PpTyp_PersuadeReqd},
  {"PersuadeWorth",	PpTyp_PersuadeWorth},
  {"Speed",			PpTyp_Speed},
  {NULL,			0},
};

const ubyte follow_dist[4][4] = {
  { 0, 3, 6, 9 },
  { 3, 0, 6, 9 },
  { 3, 6, 0, 9 },
  { 3, 6, 9, 0 },
};

ubyte sfx_man_shot[] = {
  11, 26, 41, 42, 43, 11, 26, 41,
};
ubyte sfx_woman_shot[] = {
  12, 27,
};

extern ushort female_peep;
extern sbyte byte_154F6C[8];
extern short word_1AA38E;
extern short word_1AA390;
extern short word_1AA392;
extern short word_1AA394;

void read_people_conf_file(void)
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
    int peep_count, ptype;

    pinfo = &game_dirs[DirPlace_Config];
    snprintf(conf_fname, DISKPATH_SIZE-1, "%s/people.ini", pinfo->directory);
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
        LOGERR("Could not open people types config file, going with defaults.");
        conf_buf = LbMemoryAlloc(16);
        conf_len = 0;
    }
    conf_buf[conf_len] = '\0';
    LbIniParseStart(&parser, conf_buf, conf_len);
#define CONFWRNLOG(format,args...) LOGWARN("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
#define CONFDBGLOG(format,args...) LOGDBG("%s(line %lu): " format, conf_fname, parser.line_num, ## args)
    peep_count = 0;
    // Parse the [common] section of loaded file
    if (LbIniFindSection(&parser, "common") != Lb_SUCCESS) {
        CONFWRNLOG("Could not find \"[%s]\" section, file skipped.", "common");
        LbIniParseEnd(&parser);
        LbMemoryFree(conf_buf);
        return;
    }
    done = false;
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(people_conf_common_cmds,cmd_num)
    while (!done)
    {
        // Finding command number in this line
        i = 0;
        cmd_num = LbIniRecognizeKey(&parser, people_conf_common_cmds);
        // Now store the config item in correct place
        switch (cmd_num)
        {
        case PpTyp_PersonTypesCount:
            i = LbIniValueGetLongInt(&parser, &k);
            if (i <= 0) {
                CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                break;
            }
            peep_count = k;
            CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), peep_count);
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
    for (ptype = 0; ptype < peep_count; ptype++)
    {
        char sect_name[32];
        struct PeepStat *p_pestat;
        struct PeepStatAdd *p_pestata;

        // Parse the [personN] sections of loaded file
        sprintf(sect_name, "person%d", ptype);
        p_pestat = &peep_type_stats[ptype];
        p_pestata = &peep_type_stats_a[ptype];
        if (LbIniFindSection(&parser, sect_name) != Lb_SUCCESS) {
            CONFWRNLOG("Could not find \"[%s]\" section.", sect_name);
            continue;
        }
        done = false;
#define COMMAND_TEXT(cmd_num) LbNamedEnumGetName(people_conf_person_cmds,cmd_num)
        while (!done)
        {
            // Finding command number in this line
            i = 0;
            cmd_num = LbIniRecognizeKey(&parser, people_conf_person_cmds);
            // Now store the config item in correct place
            switch (cmd_num)
            {
            case PpTyp_Name:
                i = LbIniValueGetStrWord(&parser, p_pestata->Name, sizeof(p_pestata->Name));
                if (i <= 0) {
                    CONFWRNLOG("Couldn't read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                CONFDBGLOG("%s \"%s\"", COMMAND_TEXT(cmd_num), (int)p_pestata->Name);
                break;
            case PpTyp_MaxHealth:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->MaxHealth = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->MaxHealth);
                break;
            case PpTyp_MaxShield:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->MaxShield = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->MaxShield);
                break;
            case PpTyp_MaximumStamina:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->MaximumStamina = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->MaximumStamina);
                break;
            case PpTyp_MaxEnergy:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->MaxEnergy = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->MaxEnergy);
                break;
            case PpTyp_PersuadeReqd:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->PersuadeReqd = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->PersuadeReqd);
                break;
            case PpTyp_PersuadeWorth:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->PersuadeWorth = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->PersuadeWorth);
                break;
            case PpTyp_Speed:
                i = LbIniValueGetLongInt(&parser, &k);
                if (i <= 0) {
                    CONFWRNLOG("Could not read \"%s\" command parameter.", COMMAND_TEXT(cmd_num));
                    break;
                }
                p_pestat->Speed = k;
                CONFDBGLOG("%s %d", COMMAND_TEXT(cmd_num), (int)p_pestat->Speed);
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
    for (ptype = 1; ptype < peep_count; ptype++)
    {
        struct PeepStatAdd *p_pestata;

        p_pestata = &peep_type_stats_a[ptype];
        if (strlen(p_pestata->Name) > 0) {
            peep_names[i].name = p_pestata->Name;
            peep_names[i].num = ptype;
            i++;
        }
    }
    peep_names[i].name = NULL;
    peep_names[i].num = 0;
}

void load_peep_type_stats_bin(void)
{
    char defs_fname[DISKPATH_SIZE];
    PathInfo *pinfo;
    TbFileHandle fp;

    pinfo = &game_dirs[DirPlace_Data];
    snprintf(defs_fname, DISKPATH_SIZE-1, "%s/peepdefs.dat", pinfo->directory);
    fp = LbFileOpen(defs_fname, Lb_FILE_MODE_READ_ONLY);
    if (fp != INVALID_FILE)
    {
        LbFileRead(fp, peep_type_stats, 12 * sizeof(struct PeepStat));
        LbFileClose(fp);
    }
}

void load_peep_type_stats(void)
{
#if 0
    load_peep_type_stats_bin();
#else
    read_people_conf_file();
#endif
}

const char *person_type_name(ushort ptype)
{
    struct PeepStatAdd *p_pestata;

    p_pestata = &peep_type_stats_a[ptype];
    if (strlen(p_pestata->Name) == 0)
        return "OUTRNG_PERSON";
    return p_pestata->Name;
}

TbBool person_type_only_affected_by_adv_persuader(ushort ptype)
{
    return (ptype == SubTT_PERS_ZEALOT);
}

void snprint_person_state(char *buf, ulong buflen, struct Thing *p_thing)
{
    char *s;
    ubyte nparams;
    struct PeepStateConfig *pstatcfg;

    pstatcfg = &person_states[p_thing->State];
    s = buf;

    sprintf(s, "%s( ", pstatcfg->Name);
    s += strlen(s);
    nparams = 0;

    if (p_thing->State == PerSt_PROTECT_PERSON)
    {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Person(%d)", (int)p_thing->GotoThingIndex);
        s += strlen(s);
        nparams++;
    }
    if (p_thing->State == PerSt_GOTO_POINT)
    {
        if (nparams) { sprintf(s, ", "); s += strlen(s); }
        sprintf(s, "Coord(%d,%d,%d)", (int)p_thing->U.UPerson.GotoX,
          0, (int)p_thing->U.UPerson.GotoZ);
        s += strlen(s);
        nparams++;
    }
    // TODO support more properties

    snprintf(s, buflen - (s-buf), " )");
}


TbBool person_is_dead(ThingIdx thing)
{
    struct Thing *p_thing;

    if (thing <= 0)
        return false;

    p_thing = &things[thing];

    if (p_thing->Type != TT_PERSON)
        return false;

    return (p_thing->State == PerSt_DEAD);
}

TbBool person_is_dead_or_dying(ThingIdx thing)
{
    struct Thing *p_thing;

    if (thing <= 0)
        return false;

    p_thing = &things[thing];

    if (p_thing->Type != TT_PERSON)
        return false;

    return (p_thing->State == PerSt_DEAD)
      || (p_thing->State == PerSt_PERSON_BURNING)
      || (p_thing->State == PerSt_DIEING);
}

ubyte person_mod_chest_level(struct Thing *p_person)
{
    return cybmod_chest_level(&p_person->U.UPerson.UMod);
}

void set_person_mod_chest_level(struct Thing *p_person, ubyte nmod)
{
    set_cybmod_chest_level(&p_person->U.UPerson.UMod, nmod);
}

ubyte person_mod_legs_level(struct Thing *p_person)
{
    return cybmod_legs_level(&p_person->U.UPerson.UMod);
}

void set_person_mod_legs_level(struct Thing *p_person, ubyte nmod)
{
    set_cybmod_legs_level(&p_person->U.UPerson.UMod, nmod);
}

ubyte person_mod_arms_level(struct Thing *p_person)
{
    return cybmod_arms_level(&p_person->U.UPerson.UMod);
}

void set_person_mod_arms_level(struct Thing *p_person, ubyte nmod)
{
    set_cybmod_arms_level(&p_person->U.UPerson.UMod, nmod);
}

ubyte person_mod_brain_level(struct Thing *p_person)
{
    return cybmod_brain_level(&p_person->U.UPerson.UMod);
}

void set_person_mod_brain_level(struct Thing *p_person, ubyte nmod)
{
    set_cybmod_brain_level(&p_person->U.UPerson.UMod, nmod);
}

ubyte person_mod_skin_level(struct Thing *p_person)
{
    return cybmod_skin_level(&p_person->U.UPerson.UMod);
}

void set_person_mod_skin_level(struct Thing *p_person, ubyte nmod)
{
    set_cybmod_skin_level(&p_person->U.UPerson.UMod, nmod);
}

TbBool person_carries_weapon(struct Thing *p_person, ubyte weapon)
{
    return weapons_has_weapon(p_person->U.UPerson.WeaponsCarried, weapon);
}

TbBool person_carries_any_medikit(ThingIdx person)
{
    struct Thing *p_person;

    if (person <= 0)
        return false;

    p_person = &things[person];
    return person_carries_weapon(p_person, WEP_MEDI2) || person_carries_weapon(p_person, WEP_MEDI1);
}

TbBool person_can_accept_control(ThingIdx person)
{
    return !person_is_dead_or_dying(person)
      && !thing_is_destroyed(person);
}

TbBool person_can_use_medikit(ThingIdx person)
{
    if (!person_carries_any_medikit(person))
        return false;
    return true;
}

void person_give_best_mods(struct Thing *p_person)
{
    set_person_mod_brain_level(p_person, 3);
    set_person_mod_legs_level(p_person, 3);
    set_person_mod_arms_level(p_person, 3);
    set_person_mod_chest_level(p_person, 3);
}

TbBool person_is_executing_commands(ThingIdx person)
{
    struct Thing *p_person;

    if (person <= 0)
        return false;

    p_person = &things[person];
    return ((p_person->Flag2 & TgF2_Unkn0800) != 0);
}

TbBool person_only_affected_by_adv_persuader(ThingIdx person)
{
    struct Thing *p_person;
    ushort ptype;

    if (person <= 0)
        return false;

    p_person = &things[person];

    if ((p_person->Flag2 & TgF2_Unkn00400000) != 0)
        ptype = p_person->U.UPerson.OldSubType;
    else
        ptype = p_person->SubType;

    return person_type_only_affected_by_adv_persuader(ptype);
}

TbBool person_is_persuaded(ThingIdx person)
{
    struct Thing *p_person;

    if (person <= 0)
        return false;

    p_person = &things[person];
    return ((p_person->Flag & TngF_Persuaded) != 0);
}

TbBool person_is_persuaded_by_person(ThingIdx thing, ThingIdx owntng)
{
    struct Thing *p_person;

    if (thing <= 0)
        return false;

    p_person = &things[thing];
    if ((p_person->Flag & TngF_Persuaded) == 0)
        return false;

    return (p_person->Owner == owntng);
}

TbBool person_is_persuaded_by_player(ThingIdx thing, ushort plyr)
{
    struct Thing *p_thing;
    short plyagent, plygroup;
    struct Thing *p_person;

    if (thing <= 0)
        return false;

    p_thing = &things[thing];
    if ((p_thing->Flag & TngF_Persuaded) == 0)
        return false;

    plyagent = players[plyr].DirectControl[0];
    plygroup = things[plyagent].U.UPerson.Group;

    p_person = &things[p_thing->Owner];
    return (p_person->U.UPerson.Group == plygroup);
}

ubyte person_get_selected_weapon(ThingIdx thing)
{
    struct Thing *p_thing;

    if (thing <= 0)
        return WEP_NULL;

    p_thing = &things[thing];
    return p_thing->U.UPerson.CurrentWeapon;
}

short calc_person_speed(struct Thing *p_person)
{
    struct PeepStat *pstat;
    short mood;
    short speed;

    pstat = &peep_type_stats[p_person->SubType];

    mood = p_person->U.UPerson.Mood;
    if (mood < 0)
        mood = -mood;

    speed = pstat->Speed + 2 * mood + 100 * person_mod_legs_level(p_person);

    // Flamer slows you down? Why? This makes it even more useless..
    if (p_person->U.UPerson.CurrentWeapon == WEP_FLAMER)
        speed >>= 1;

    if ((p_person->Flag & TngF_Persuaded) != 0)
        speed += 250;

    if ((p_person->Flag2 & TgF2_Unkn00080000) != 0)
        speed += 512;

    if (speed > PERSON_MAX_SPEED)
        speed = PERSON_MAX_SPEED;

    return speed;
}

void person_set_helath_to_max_limit(struct Thing *p_person)
{
    p_person->U.UPerson.MaxHealth = PERSON_MAX_HEALTH_LIMIT;
    p_person->Health = 2 * p_person->U.UPerson.MaxHealth; // double health - fill red bar
}

void person_set_energy_to_max_limit(struct Thing *p_person)
{
    p_person->U.UPerson.MaxEnergy = PERSON_MAX_ENERGY_LIMIT;
    p_person->U.UPerson.Energy = p_person->U.UPerson.MaxEnergy;
}

void set_person_health_shield_type(struct Thing *p_person, ushort stype)
{
    struct PeepStat *pstat;

    pstat = &peep_type_stats[stype];

    p_person->U.UPerson.ShieldEnergy = pstat->MaxShield +
        (person_mod_brain_level(p_person) * pstat->MaxShield * 50 / 100);

    p_person->Health = pstat->MaxHealth +
       ((person_mod_chest_level(p_person) * pstat->MaxHealth * 50 / 100) +
        (person_mod_brain_level(p_person) * pstat->MaxHealth * 50 / 100) +
        (person_mod_legs_level(p_person) * pstat->MaxHealth * 25 / 100) +
        (person_mod_arms_level(p_person) * pstat->MaxHealth * 25 / 100)) / 4;

    p_person->U.UPerson.MaxHealth = p_person->Health;
    p_person->U.UPerson.MaxShieldEnergy = p_person->U.UPerson.ShieldEnergy;

    if (person_mod_chest_level(p_person) == 4)
    {
        person_set_helath_to_max_limit(p_person);
    }
}

void set_person_energy_stamina_type(struct Thing *p_person, ushort stype)
{
    struct PeepStat *pstat;

    pstat = &peep_type_stats[stype];

    p_person->U.UPerson.Energy = pstat->MaxEnergy +
        (person_mod_chest_level(p_person) * pstat->MaxEnergy * 50 / 100);

    p_person->U.UPerson.Stamina = pstat->MaximumStamina;

    p_person->U.UPerson.MaxEnergy = p_person->U.UPerson.Energy;
    p_person->U.UPerson.MaxStamina = p_person->U.UPerson.Stamina;

    if (person_mod_chest_level(p_person) == 4)
    {
        person_set_energy_to_max_limit(p_person);
    }
}

void person_init_wait(struct Thing *p_person, short wait_turns)
{
    p_person->State = PerSt_WAIT;
    p_person->U.UPerson.ComTimer = wait_turns;
}

void person_start_executing_commands(struct Thing *p_person)
{
    p_person->Flag2 |= TgF2_Unkn0800;
    p_person->Flag |= TngF_Unkn0040;
    ingame.Flags |= GamF_Unkn0100;
}

short person_slot_as_player_agent(struct Thing *p_person, PlayerIdx plyr)
{
    PlayerInfo *p_player;
    struct Thing *p_agent;
    short plagent;

    p_player = &players[plyr];

    for (plagent = 0; plagent < playable_agents; plagent++)
    {
        p_agent = p_player->MyAgent[plagent];
        if (p_person->ThingOffset == p_agent->ThingOffset)
        {
            return plagent;
        }
    }
    return -1;
}

TbBool person_is_player_agent_in_slot(struct Thing *p_person, PlayerIdx plyr, short plagent)
{
    PlayerInfo *p_player;
    struct Thing *p_agent;

    p_player = &players[plyr];
    p_agent = p_player->MyAgent[0];

    return (p_person->ThingOffset == p_agent->ThingOffset);
}

TbBool person_has_slot_in_any_player_dcontrol(ThingIdx person)
{
    struct Thing *p_person;

    if (person <= 0)
        return false;

    if (person_is_executing_commands(person))
        return false;

    p_person = &things[person];

    return ((p_person->Flag & TngF_PlayerAgent) != 0);
}

short person_get_dcontrol_player(ThingIdx person)
{
    struct Thing *p_person;

    if (!person_has_slot_in_any_player_dcontrol(person))
        return -1;

    p_person = &things[person];

    return (p_person->U.UPerson.ComCur >> 2);
}

short person_slot_in_player_dcontrol(ThingIdx person, PlayerIdx plyr)
{
    struct Thing *p_person;
    PlayerIdx pers_plyr;

    if (!person_has_slot_in_any_player_dcontrol(person)) {
        return -1;
    }

    p_person = &things[person];

    pers_plyr = p_person->U.UPerson.ComCur >> 2;
    if (pers_plyr != plyr) {
        return -1;
    }

    return (p_person->U.UPerson.ComCur & 3);
}

ubyte person_sex(struct Thing *p_person)
{
    return (((1 << p_person->SubType) & female_peep) != 0) ? PERSON_FEMALE : PERSON_MALE;
}

void set_peep_comcur(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_set_peep_comcur\n"
        : : "a" (p_person));
#endif
    ushort plyr;
    short plagent;

    plyr = local_player_no;
    plagent = person_slot_as_player_agent(p_person, plyr);

    if (plagent >= 0)
    {
        p_person->U.UPerson.ComHead = 0;
        p_person->U.UPerson.ComCur = (plyr << 2) | plagent;
    }

    if (person_is_player_agent_in_slot(p_person, plyr, 0))
    {
        game_set_cam_track_thing_xz(p_person->ThingOffset);
    }
}

void person_finish_executing_commands(struct Thing *p_person)
{
    if (((p_person->Flag & TngF_PlayerAgent) != 0) && ((p_person->Flag2 & TgF2_Unkn0800) != 0))
    {
        p_person->Flag2 &= ~TgF2_Unkn0800;
        ingame.Flags &= ~GamF_Unkn0100;
        set_peep_comcur(p_person);
    }
}

void person_resurrect(struct Thing *p_person)
{
    p_person->Flag &= ~TngF_Destroyed;
    p_person->Flag &= ~TngF_Unkn02000000;
    p_person->State = PerSt_WAIT;
    remove_path(p_person);
    p_person->Health = p_person->U.UPerson.MaxHealth * 3 / 4;
    set_person_anim_mode(p_person, ANIM_PERS_WEPLIGHT_IDLE);
}

void person_set_persuade_power__to_allow_all(struct Thing *p_person)
{
    short max_required_pers_power;

    max_required_pers_power = 20;
    p_person->U.UPerson.PersuadePower = max(p_person->U.UPerson.PersuadePower,
      max_required_pers_power);
}

ushort calc_person_radius_type(struct Thing *p_person, ushort stype)
{
    ushort r;
    switch (stype)
    {
    case SubTT_PERS_AGENT:
    case SubTT_PERS_PUNK_F:
    case SubTT_PERS_POLICE:
    case SubTT_PERS_PUNK_M:
    default:
        r = 80;
        break;
    case SubTT_PERS_ZEALOT:
    case SubTT_PERS_HIGH_PRIEST:
        r = 100;
        break;
    case SubTT_PERS_MERCENARY:
    case SubTT_PERS_SCIENTIST:
    case SubTT_PERS_SHADY_M:
    case SubTT_PERS_BRIEFCASE_M:
    case SubTT_PERS_WHITE_BRUN_F:
    case SubTT_PERS_WHIT_BLOND_F:
    case SubTT_PERS_LETH_JACKT_M:
        r = 100;
        break;
    case SubTT_PERS_MECH_SPIDER:
        r = 384;
        break;
    }
    return r;
}

void set_person_stats_type(struct Thing *p_person, ushort stype)
{
    set_person_health_shield_type(p_person, stype);
    set_person_energy_stamina_type(p_person, stype);
    p_person->U.UPerson.PersuadePower = 0;
    p_person->Speed = calc_person_speed(p_person);
}

/** Set a person frame with single-angle sprite.
 * Person frames are a bit different from other things, even the
 * single-angle ones, because StartFrame in incremented when used as frame index.
 */
void set_person_frame_noangle(struct Thing *p_person, ushort anim_start)
{
    p_person->StartFrame = anim_start - 1;
    p_person->Frame = nstart_ani[p_person->StartFrame + 1];
}

void reset_person_frame(struct Thing *p_person)
{
    ushort person_anim;

    person_anim = people_frames[p_person->SubType][p_person->U.UPerson.AnimMode];
    p_person->StartFrame = person_anim - 1;
    p_person->Frame = nstart_ani[p_person->StartFrame + 1 + p_person->U.UPerson.Angle];
}

void switch_person_anim_mode(struct Thing *p_person, ubyte animode)
{
    ushort person_anim;

    person_anim = people_frames[p_person->SubType][p_person->U.UPerson.AnimMode];
    p_person->Frame -= nstart_ani[person_anim + p_person->U.UPerson.Angle];
    p_person->U.UPerson.AnimMode = animode;
    person_anim = people_frames[p_person->SubType][p_person->U.UPerson.AnimMode];
    p_person->StartFrame = person_anim - 1;
    p_person->Frame += nstart_ani[p_person->StartFrame + 1 + p_person->U.UPerson.Angle];
}

void set_person_anim_mode(struct Thing *p_person, ubyte animode)
{
    p_person->U.UPerson.AnimMode = animode;
    reset_person_frame(p_person);
}

void change_player_angle(struct Thing *p_person, ushort angle)
{
    short person_anim, sframe;

    if (angle == p_person->U.UPerson.Angle)
        return;

    p_person->U.UPerson.Angle = angle;
    person_anim = people_frames[p_person->SubType][p_person->U.UPerson.AnimMode];

    // TODO why are we not updating p_person->StartFrame here?
    sframe = person_anim - 1;
    p_person->Frame = nstart_ani[sframe + 1 + p_person->U.UPerson.Angle];
}

void init_person_thing(struct Thing *p_person)
{
    set_person_health_shield_type(p_person, p_person->SubType);
    set_person_energy_stamina_type(p_person, p_person->SubType);

    p_person->U.UPerson.PersuadePower = 0;
    p_person->Radius = calc_person_radius_type(p_person, p_person->SubType);

    if (current_level != 0)
        p_person->U.UPerson.CurrentWeapon = 0;

    if ((p_person->Flag & TngF_Destroyed) != 0)
    {
        p_person->U.UPerson.AnimMode = ANIM_PERS_DEAD_BODY;
        p_person->State = PerSt_DEAD;
        p_person->U.UPerson.FrameId.Version[3] = 1;
        p_person->U.UPerson.FrameId.Version[4] = 0;
    }
    else
    {
        switch_person_anim_mode(p_person, ANIM_PERS_IDLE);
    }
}

/** Frees a linked list of paths, and adds the paths to free list.
 */
void path_free_linked_list(ushort first_path)
{
    ushort free_head, path;
    ushort count;

    free_head = head_my_path;
    count = path_count;

    path = first_path;
    while (path != 0)
    {
        ushort tmpath;

        tmpath = my_paths[path].Next;
        my_paths[path].Flag = 0;
        my_paths[path].Next = free_head;
        free_head = path;
        path = tmpath;
        count--;
    }
    path_count = count;
    head_my_path = free_head;
}

void remove_path(struct Thing *p_thing)
{
    ushort path;

    p_thing->Flag2 &= ~TgF2_Unkn0040;
    if (p_thing->U.UPerson.PathIndex != 0)
    {
        if (p_thing->Type == TT_PERSON)
            p_thing->U.UPerson.LastDist = 32000;
        path = p_thing->U.UPerson.PathIndex;
        p_thing->U.UPerson.PathIndex = 0;
        path_free_linked_list(path);
        p_thing->PathOffset = 0;
        p_thing->Flag &= ~TngF_Unkn00020000;
    }
}

void set_person_animmode_walk(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_set_person_animmode_walk\n"
        : : "a" (p_person));
#endif
    if (p_person->State == PerSt_PERSON_BURNING)
        return;
    if ((p_person->Flag & TngF_Destroyed) != 0)
        return;

    p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
    reset_person_frame(p_person);
    p_person->Timer1 = 48;
    p_person->StartTimer1 = 48;
    p_person->Flag2 &= ~(0x80000000|TngF_Persuaded);
    p_person->Speed = calc_person_speed(p_person);
}

int can_i_see_thing(struct Thing *p_me, struct Thing *p_him, int max_dist, ushort flags)
{
    int ret;
    asm volatile ("call ASM_can_i_see_thing\n"
        : "=r" (ret) : "a" (p_me), "d" (p_him), "b" (max_dist), "c" (flags));
    return ret;
}

TbBool can_i_enter_vehicle(struct Thing *p_me, struct Thing *p_vehicle)
{
#if 0
    TbBool ret;
    asm volatile ("call ASM_can_i_enter_vehicle\n"
        : "=r" (ret) : "a" (p_me), "d" (p_vehicle));
    return ret;
#endif
    struct Thing *p_thing;
    ThingIdx thing;
    ushort tngroup, mygroup;

    if (person_is_executing_commands(p_me->ThingOffset)) {
        // if person is executing commands, allow unconditionally
        return true;
    }

    thing = p_vehicle->U.UVehicle.PassengerHead;
    if (thing <= 0)
        return true;
    p_thing = &things[thing];

    tngroup = p_thing->U.UObject.EffectiveGroup;
    mygroup = p_me->U.UObject.EffectiveGroup;
    if (tngroup == mygroup)
        return true;
    if (thing_group_have_truce(tngroup, mygroup) || thing_group_have_truce(mygroup, tngroup))
        return true;

    return false;
}

void set_person_persuaded(struct Thing *p_person, struct Thing *p_attacker, ushort energy)
{
    // Cannot persuade other players agent
    if ((p_person->Flag & TngF_PlayerAgent) != 0)
        return;

    if (p_person->State == PerSt_DEAD)
        return;

    play_dist_sample(p_person, 20, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
    remove_path(p_person);

    p_attacker->U.UPerson.Energy -= energy;
    if (p_attacker->U.UPerson.Energy < 0)
        p_attacker->U.UPerson.Energy = 0;

    p_person->PTarget = NULL;
    p_person->U.UPerson.Target2 = 0;
    p_person->State = PerSt_NONE;

    p_person->Flag |= TngF_Unkn40000000 | TngF_Unkn0004;
    p_person->Owner = p_attacker->ThingOffset;
    p_person->Flag &= ~(TngF_Unkn00800000|TngF_Unkn00040000|TngF_Unkn00020000|TngF_Unkn0800|TngF_Unkn0080);

    set_person_animmode_walk(p_person);
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 3;
    p_person->U.UPerson.Timer2 = 5;
    p_person->U.UPerson.StartTimer2 = 5;
    p_person->SubState = 0;

    p_person->U.UPerson.EffectiveGroup = p_attacker->U.UPerson.EffectiveGroup;
    p_person->U.UPerson.Within = 0;
    {
        struct PeepStat *pstat;

        pstat = &peep_type_stats[p_person->SubType];
        p_attacker->U.UPerson.PersuadePower += pstat->PersuadeWorth;
    }
    if ((p_person->Flag2 & TgF2_KnockedOut) == 0)
    {
          set_person_frame_noangle(p_person, 1060);
    }
    p_person->Timer1 = 48;
    p_person->StartTimer1 = 48;
    {
        word_17FA58[word_1531DA] = p_person->ThingOffset;
        word_1531DA++;
    }
    p_person->State = PerSt_BEING_PERSUADED;
    {
        ushort group;
        group = p_person->U.UPerson.Group & 0x1F;
        group_actions[group].Persuaded++;
    }
    if (!in_network_game && (p_attacker->Flag & TngF_PlayerAgent) &&
      (p_attacker->U.UPerson.EffectiveGroup == ingame.MyGroup))
    {
        short plagent;

        persuaded_person_add_to_stats(p_person, open_brief);

        for (plagent = 0; plagent < playable_agents; plagent++)
        {
            struct Thing *p_agent;

            p_agent = players[local_player_no].MyAgent[plagent];
            if (p_person == p_agent->PTarget)
                p_agent->PTarget = NULL;
            if (p_agent->U.UPerson.Target2 == p_person->ThingOffset)
                p_agent->U.UPerson.Target2 = 0;
        }
    }
}

void unpersuade_my_peeps(struct Thing *p_owntng)
{
    ThingIdx person;
    struct Thing *p_person;
    ushort count;
    int i, k;

    count = word_1531DA;
    for (i = 1; i < count; i++)
    {
        person = word_17FA58[i];
        p_person = &things[person];
        if (p_person->Owner != p_owntng->ThingOffset)
            continue;
        p_person->Flag &= ~TngF_Persuaded;
        p_person->U.UPerson.EffectiveGroup = p_owntng->U.UPerson.Group;
        p_person->State = PerSt_NONE;
        p_person->Flag |= TngF_Unkn0040 | TngF_Unkn0004;
        --group_actions[p_person->U.UPerson.Group].Persuaded;
        for (k = i; k < count - 1; k++) {
            word_17FA58[k] = word_17FA58[k+1];
        }
        count--;
        if (!in_network_game && (p_owntng->U.UPerson.EffectiveGroup == ingame.MyGroup))
        {
            persuaded_person_remove_from_stats(p_person, open_brief);
        }
    }
    p_owntng->U.UPerson.PersuadePower = 0;
    word_1531DA = count;
}

struct Thing *new_sim_person(int x, int y, int z, ubyte subtype)
{
#if 0
    struct Thing *p_person;
    asm volatile ("call ASM_new_sim_person\n"
        : "=r" (p_person) : "a" (x), "d" (y), "b" (z), "c" (subtype));
    return p_person;
#endif
    ubyte ptype;
    struct PeepStat *p_pestat;
    ThingIdx person;
    struct Thing *p_person;
    ushort rnd;
    int ry;

    ptype = subtype;
    if (subtype <= 100) {
        person = get_new_thing();
    } else {
        ptype = SubTT_PERS_AGENT;
        person = unkn01_thing_idx;
    }
    if (person <= 0)
        return &things[0];
    p_pestat = &peep_type_stats[ptype];
    rnd = LbRandomAnyShort();

    p_person = &things[person];
    p_person->X = MAPCOORD_TO_PRCCOORD(x,0);
    p_person->Z = MAPCOORD_TO_PRCCOORD(z,0);
    p_person->U.UPerson.Target2 = 0;
    ry = alt_at_point(x, z);
    p_person->Speed = 512;
    p_person->U.UPerson.Angle = (rnd >> 1) % 8;
    p_person->U.UPerson.AnimMode = ANIM_PERS_IDLE;
    p_person->StartTimer1 = 48;
    p_person->Timer1 = 48;
    p_person->Y = ry;
    p_person->U.UPerson.OnFace = 0;
    p_person->Type = TT_PERSON;
    p_person->Radius = calc_person_radius_type(p_person, ptype);
    switch (ptype)
    {
    case SubTT_PERS_AGENT:
        p_person->U.UPerson.Angle = 0;
        p_person->U.UPerson.CurrentWeapon = WEP_MINIGUN;
        break;
    case SubTT_PERS_ZEALOT:
    case SubTT_PERS_HIGH_PRIEST:
        p_person->U.UPerson.CurrentWeapon = WEP_ELLASER;
        break;
    case SubTT_PERS_PUNK_F:
        p_person->U.UPerson.CurrentWeapon = WEP_UZI;
        p_person->U.UPerson.FrameId.Version[0] = rnd % 3;
        break;
    case SubTT_PERS_BRIEFCASE_M:
    case SubTT_PERS_WHITE_BRUN_F:
    case SubTT_PERS_MERCENARY:
    case SubTT_PERS_SCIENTIST:
    case SubTT_PERS_WHIT_BLOND_F:
    case SubTT_PERS_LETH_JACKT_M:
        p_person->U.UPerson.CurrentWeapon = WEP_NULL;
        break;
    case SubTT_PERS_MECH_SPIDER:
        p_person->U.UPerson.CurrentWeapon = WEP_LASER;
        break;
    case SubTT_PERS_POLICE:
        p_person->U.UPerson.CurrentWeapon = WEP_LASER;
        break;
    case SubTT_PERS_PUNK_M:
        p_person->U.UPerson.CurrentWeapon = WEP_UZI;
        break;
    case SubTT_PERS_SHADY_M:
        p_person->U.UPerson.CurrentWeapon = WEP_UZI;
        break;
    default:
        break;
    }
    p_person->SubType = ptype;
    p_person->U.UPerson.Group = ptype + 4;
    p_person->U.UPerson.EffectiveGroup = p_person->U.UPerson.Group;
    p_person->U.UPerson.AnimMode = (p_person->U.UPerson.CurrentWeapon != WEP_NULL) ? ANIM_PERS_WEPLIGHT_IDLE : ANIM_PERS_IDLE;
    reset_person_frame(p_person);
    init_person_thing(p_person);
    p_person->U.UPerson.WeaponsCarried = 0;
    if (p_person->U.UPerson.CurrentWeapon)
        p_person->U.UPerson.WeaponsCarried |= (1 << (p_person->U.UPerson.CurrentWeapon - 1));
    p_person->U.UPerson.WeaponsCarried |= (1 << (WEP_ENERGYSHLD-1));
    p_person->U.UPerson.EffectiveGroup = p_person->U.UPerson.Group;
    p_person->Speed = p_pestat->Speed;
    add_node_thing(person);
    p_person->U.UPerson.ComHead = 0;
    p_person->U.UPerson.ComCur = 0;
    return p_person;
}

short check_for_other_people(int x, int y, int z, struct Thing *p_person)
{
    short ret;
    asm volatile (
      "call ASM_check_for_other_people\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z), "c" (p_person));
    return ret;
}

void check_persons_target(struct Thing *p_person)
{
    asm volatile ("call ASM_check_persons_target\n"
        : : "a" (p_person));
}

void check_persons_target2(struct Thing *p_person)
{
    asm volatile ("call ASM_check_persons_target2\n"
        : : "a" (p_person));
}

void process_stamina(struct Thing *p_person)
{
    asm volatile ("call ASM_process_stamina\n"
        : : "a" (p_person));
}

void process_shield(struct Thing *p_person)
{
#if 0
    asm volatile (
      "call ASM_process_shield\n"
        : : "a" (p_person));
#endif
    struct PeepStat *p_pestat;

    if (p_person->U.UPerson.ShieldEnergy < -300)
        p_person->U.UPerson.ShieldEnergy = -300;

    if (p_person->U.UPerson.ShieldGlowTimer > 0)
        p_person->U.UPerson.ShieldGlowTimer--;

    p_pestat = &peep_type_stats[p_person->SubType];

    if ((p_person->Flag & TngF_Unkn00200000) != 0)
    {
        p_person->U.UPerson.ShieldEnergy -= 16;
        if (p_person->U.UPerson.ShieldEnergy <= 0)
            p_person->Flag &= ~TngF_Unkn00200000;
    }
    else
    {
        p_person->U.UPerson.ShieldEnergy += 8;
        if (p_person->U.UPerson.ShieldEnergy > p_pestat->MaxShield)
            p_person->U.UPerson.ShieldEnergy = p_pestat->MaxShield;
    }

    if ((p_person->Flag & TngF_PersSupShld) != 0)
    {
        short delta, en_drain;

        delta = p_pestat->MaxShield - p_person->U.UPerson.ShieldEnergy;
        if (delta > 0)
        {
            en_drain = delta / PERSON_ENERGY_TO_SHIELD_MUL;
            if (en_drain > p_person->U.UPerson.Energy) {
                delta = PERSON_ENERGY_TO_SHIELD_MUL * p_person->U.UPerson.Energy;
                p_person->Flag &= ~TngF_PersSupShld;
            }
            en_drain = delta / PERSON_ENERGY_TO_SHIELD_MUL;
            p_person->U.UPerson.ShieldEnergy += delta;
            p_person->U.UPerson.Energy -= en_drain;
        }
    }
}

ubyte conditional_command_state_true(ushort cmd, struct Thing *p_me, ubyte from)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_conditional_command_state_true\n"
        : "=r" (ret) : "a" (cmd), "d" (p_me), "b" (from));
    return ret;
#endif
    struct Command *p_cmd;
    struct Thing *p_othertng;
    short tng_x, tng_y, tng_z;
    ubyte unmet;

    p_cmd = &game_commands[cmd];
    unmet = (p_cmd->Flags & PCmdF_RevertFunct) != 0;

    switch (p_cmd->Type)
    {
    case PCmd_WAIT_MEM_G_ARRIVE:
    case PCmd_WAND_MEM_G_ARRIVE:
    case PCmd_UNTIL_MEM_G_ARRIVE:
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0)
        {
            if (mem_group_arrived_square2(p_me, p_cmd->OtherThing,
              p_cmd->X, p_cmd->Z, p_cmd->Arg1, p_cmd->Time, p_cmd->Arg2)) {
                return !unmet;
            }
        }
        else
        {
            if (mem_group_arrived(p_cmd->OtherThing,
              p_cmd->X, p_cmd->Y, p_cmd->Z, p_cmd->Arg1, p_cmd->Arg2)) {
                return !unmet;
            }
        }
        return unmet;

    case PCmd_WAIT_P_V_DEAD:
    case PCmd_WAND_P_V_DEAD:
    case PCmd_UNTIL_P_V_DEAD:
        p_othertng = &things[p_cmd->OtherThing];
        if (p_othertng->State == PerSt_DEAD)
        {
            return !unmet;
        }
        return unmet;

    case PCmd_WAIT_P_V_I_ARRIVE:
    case PCmd_WAND_P_V_I_ARRIVE:
    case PCmd_UNTIL_P_V_I_ARRIVE:
        if (thing_is_destroyed(p_cmd->OtherThing))
        {
            return unmet;
        }
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0)
        {
            if (thing_arrived_at_obj_square2(p_cmd->OtherThing,
              p_cmd->X, p_cmd->Z, p_cmd->Arg1, p_cmd->Time)) {
                return !unmet;
            }
        }
        else
        {
            int radius_sq;
            radius_sq = (p_cmd->Arg1 * p_cmd->Arg1) << 12;
            if (thing_arrived_at_obj_radius(p_cmd->OtherThing,
              p_cmd->X, p_cmd->Y, p_cmd->Z, radius_sq)) {
                return !unmet;
            }
        }
        return unmet;

    case PCmd_WAIT_ALL_G_NEAR:
    case PCmd_WAND_ALL_G_NEAR:
    case PCmd_UNTIL_ALL_G_NEAR:
        if (all_group_arrived( p_cmd->OtherThing, PRCCOORD_TO_MAPCOORD(p_me->X),
          PRCCOORD_TO_MAPCOORD(p_me->Y), PRCCOORD_TO_MAPCOORD(p_me->Z), p_cmd->Arg1))
        {
            return !unmet;
        }
        return unmet;

    case PCmd_WAND_MEM_G_NEAR:
    case PCmd_WAIT_MEM_G_NEAR:
    case PCmd_UNTIL_MEM_G_NEAR:
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_me->ThingOffset);
        if (mem_group_arrived(p_cmd->OtherThing,
          tng_x, tng_y >> 3, tng_z, p_cmd->Arg1, p_cmd->Arg2))
        {
            return !unmet;
        }
        return unmet;

    case PCmd_WAIT_MEM_G_PERSUADE:
    case PCmd_WAND_MEM_G_PERSUADE:
    case PCmd_UNTIL_MEM_G_PERSUADE:
        if (group_actions[p_cmd->OtherThing].Persuaded >= p_cmd->Arg2)
            return !unmet;
        return unmet;

    case PCmd_WAIT_P_V_I_NEAR:
    case PCmd_WAND_P_V_I_NEAR:
    case PCmd_UNTIL_P_V_I_NEAR:
        if (thing_arrived_at_obj_radius(p_cmd->OtherThing,
          PRCCOORD_TO_MAPCOORD(p_me->X), PRCCOORD_TO_MAPCOORD(p_me->Y), PRCCOORD_TO_MAPCOORD(p_me->Z),
          ((p_cmd->Arg1) * (p_cmd->Arg1)) << 12))
        {
            return !unmet;
        }
        return unmet;

    case PCmd_WAIT_ALL_G_PERSUADE:
    case PCmd_WAND_ALL_G_PERSUADE:
    case PCmd_UNTIL_ALL_G_PERSUADE:
        if (group_actions[p_cmd->OtherThing].Persuaded == group_actions[p_cmd->OtherThing].Alive)
            return !unmet;
        return unmet;

    case PCmd_UNTIL_ALL_G_DEAD:
    case PCmd_WAIT_ALL_G_DEAD:
    case PCmd_WAND_ALL_G_DEAD:
        if (group_actions[p_cmd->OtherThing].Alive == 0)
        {
            return 1;
        }
        return unmet;

    case PCmd_UNTIL_TIME:
    case PCmd_WAND_TIME:
    case PCmd_WAIT_TIME:
        if (p_cmd->Time < 9999)
        {
            p_cmd->Arg1++;
            if (p_cmd->Arg1 >= p_cmd->Time) {
                p_cmd->Arg1 = 0;
                return 1;
            }
        }
        return unmet;

    case PCmd_UNTIL_G_NOT_SEEN:
        if (group_not_seen(p_cmd->OtherThing))
        {
            return !unmet;
        }
        if (group_actions[p_cmd->OtherThing].Alive == 0)
        {
            return 1;
        }
        return unmet;

    case PCmd_WAIT_OBJV:
    case PCmd_WAND_OBJV:
    case PCmd_UNTIL_OBJV:
        return test_objective(p_cmd->OtherThing, 2);

    case PCmd_WAIT_OBJT_DESTROY:
    case PCmd_WAND_OBJT_DESTROY:
    case PCmd_UNTIL_OBJT_DESTROY:
        if (thing_is_destroyed(p_cmd->OtherThing))
        {
            return !unmet;
        }
        return unmet;

    case PCmd_WAIT_P_PERSUADE:
    case PCmd_WAND_P_PERSUADE:
    case PCmd_UNTIL_P_PERSUADE:
        p_othertng = &things[p_cmd->OtherThing];
        if ((p_othertng->Flag & TngF_Persuaded) != 0)
            return !unmet;
        return unmet;

    case PCmd_WAIT_MEM_G_DEAD:
    case PCmd_WAND_MEM_G_DEAD:
    case PCmd_UNTIL_MEM_G_DEAD:
        if (group_actions[p_cmd->OtherThing].Dead >= p_cmd->Arg2)
            return 1;
        return unmet;

    case PCmd_WAIT_ALL_G_ARRIVE:
    case PCmd_WAND_ALL_G_ARRIVE:
    case PCmd_UNTIL_ALL_G_ARRIVE:
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0)
        {
            if (all_group_arrived_square(p_cmd->OtherThing,
              p_cmd->X, p_cmd->Z, p_cmd->Arg1, p_cmd->Time)) {
                return !unmet;
            }
        }
        else
        {
            if (all_group_arrived(p_cmd->OtherThing,
              p_cmd->X, p_cmd->Y, p_cmd->Z, p_cmd->Arg1)) {
                return !unmet;
            }
        }
        return unmet;

    case PCmd_NONE:
    case PCmd_STAY:
    case PCmd_GO_TO_POINT:
        return 0;

    default:
        break;
    }
    return unmet;
}

void person_init_drop_special(struct Thing *p_person, ThingIdx item)
{
    asm volatile ("call ASM_person_init_drop_special\n"
        : : "a" (p_person), "d" (item));
}

int start_goto_vehicle(struct Thing *p_vehicle, int x, int z)
{
    int ret;
    asm volatile (
      "call ASM_start_goto_vehicle\n"
        : "=r" (ret) : "a" (p_vehicle), "d" (x), "b" (z));
    return ret;
}

short find_peep_in_area(struct Thing *p_me, struct Command *p_cmd)
{
    short ret;
    asm volatile (
      "call ASM_find_peep_in_area\n"
        : "=r" (ret) : "a" (p_me), "d" (p_cmd));
    return ret;
}

void set_person_animmode_run(struct Thing *p_person)
{
    asm volatile ("call ASM_set_person_animmode_run\n"
        : : "a" (p_person));
}

ushort build_navigate_path(struct Thing *p_thing, int x, int z, int face)
{
    ushort ret;
    asm volatile (
      "call ASM_build_navigate_path\n"
        : "=r" (ret) : "a" (p_thing), "d" (x), "b" (z), "c" (face));
    return ret;
}

ushort build_navigate_path_to_person(struct Thing *p_thing, struct Thing *p_thing_to)
{
    ushort ret;
    asm volatile (
      "call ASM_build_navigate_path_to_person\n"
        : "=r" (ret) : "a" (p_thing), "d" (p_thing_to));
    return ret;
}

void build_navigate_path_to_face(struct Thing *p_thing, short face)
{
    asm volatile ("call ASM_build_navigate_path_to_face\n"
        : : "a" (p_thing), "d" (face));
}

void build_navigate_path_to_face_xz(struct Thing *p_thing, short face, int x, int z)
{
    asm volatile ("call ASM_build_navigate_path_to_face_xz\n"
        : : "a" (p_thing), "d" (face), "b" (x), "c" (z));
}

void check_weapon(struct Thing *p_person, int range)
{
    asm volatile ("call ASM_check_weapon\n"
        : : "a" (p_person), "d" (range));
}

void get_weapon_out(struct Thing *p_person)
{
    asm volatile ("call ASM_get_weapon_out\n"
        : : "a" (p_person));
}

#define CMD_CONDITION_CONTINUE 0xFFFF

/** When one of "until" conditions of a command is met, give the next command.
 *
 * A command can have multiple "until" conditions below. Meeting at least one
 * of the conditions terminates the command.
 *
 * @return If loop should terminate and the next command should start, this call
 *   returns its index; otherwise, CMD_CONDITION_CONTINUE is returned.
 */
ushort person_command_until_check_condition(struct Thing *p_person, ushort cond_cmd)
{
    struct Command *p_cmd;
    ushort cmd;
    TbBool until_met, has_more_conditions;

    p_cmd = &game_commands[cond_cmd];
    cmd = p_cmd->Next;
    until_met = false;
    has_more_conditions = ((p_cmd->Flags & PCmdF_RunUntil) != 0);

    while (1)
    {
        if (cmd == 0)
            break;
        p_cmd = &game_commands[cmd];
        if ((p_cmd->Flags & PCmdF_RunUntil) == 0)
            has_more_conditions = false;
        if (!has_more_conditions) {
            if ((p_cmd->Flags & PCmdF_IsUntil) == 0)
                break;
            // After RunUntil commands and then IsUntil command, this loop should be broken
            // If it is not, then something is wrong with flags of the commands.
            if ((p_cmd->Flags & PCmdF_RunUntil) != 0) {
                LOGWARN("Person %s %d command %d has broken chain of UNTIL commands at %d",
                  person_type_name(p_person->SubType), (int)p_person->ThingOffset, cond_cmd, cmd);
            }
        }
        if (conditional_command_state_true(cmd, p_person, 3)) {
            if (gameturn <= 1) {
                LOGWARN("Person %s %d command %d end condition %d met just at start of the game",
                  person_type_name(p_person->SubType), (int)p_person->ThingOffset, cond_cmd, cmd);
            }
            if ((debug_log_things & 0x01) != 0) {
                char cond_locstr[192];
                char locstr[192];
                snprint_command(cond_locstr, sizeof(cond_locstr), cond_cmd);
                snprint_command(locstr, sizeof(locstr), cmd);
                LOGSYNC("Person %s %d %s %d ends, met %s %d, state %d.%d",
                  person_type_name(p_person->SubType), (int)p_person->ThingOffset,
                  cond_locstr, cond_cmd, locstr, cmd, p_person->State, p_person->SubState);
            }
            until_met = true;
        }
        cmd = p_cmd->Next;
    }

    if (!until_met)
        return CMD_CONDITION_CONTINUE;

    return cmd;
}

/** For given command, return the next command beyond any "until" conditions.
 *
 * @return Gives index of next command beyond "until" conditions, regardless
 *   whether any of the conditions is met or not.
 */
ushort person_command_until_skip_condition(struct Thing *p_person, ushort cond_cmd)
{
    struct Command *p_cmd;
    ushort cmd;
    TbBool has_more_conditions;

    p_cmd = &game_commands[cond_cmd];
    cmd = p_cmd->Next;
    has_more_conditions = ((p_cmd->Flags & PCmdF_RunUntil) != 0);

    while (1)
    {
        if (cmd == 0)
            break;
        p_cmd = &game_commands[cmd];
        if ((p_cmd->Flags & PCmdF_RunUntil) == 0)
            has_more_conditions = false;
        if (!has_more_conditions) {
            if ((p_cmd->Flags & PCmdF_IsUntil) == 0)
                break;
            if ((p_cmd->Flags & PCmdF_RunUntil) != 0) {
                LOGWARN("Person %s %d command %d has broken chain of UNTIL commands at %d",
                  person_type_name(p_person->SubType), (int)p_person->ThingOffset, cond_cmd, cmd);
            }
        }
        cmd = p_cmd->Next;
    }

    return cmd;
}

/** Select next command on a person.
 */
void person_command_select_next(struct Thing *p_person)
{
    ushort cmd;

    cmd = p_person->U.UPerson.ComCur;
    if (cmd == 0)
        return;

    p_person->U.UPerson.ComCur = person_command_until_skip_condition(p_person, cmd);
}

/** Discards a command at top of the person stack.
 * The command is then never used again.
 */
void person_command_skip_at_start(struct Thing *p_person)
{
    ushort cmd;

    cmd = p_person->U.UPerson.ComHead;
    if (cmd == 0)
        return;

    p_person->U.UPerson.ComHead = person_command_until_skip_condition(p_person, cmd);
}

/** Jump to specific command on a person.
 */
StateChRes person_command_jump(struct Thing *p_person, ushort cmd)
{
    p_person->State = PerSt_INIT_COMMAND;
    p_person->U.UPerson.ComCur = cmd;
    return StCh_ACCEPTED;
}

StateChRes person_init_cmd_wander(struct Thing *p_person, ubyte sstate)
{
    if ((p_person->Flag & TngF_InVehicle) != 0)
    {
        struct Thing *p_vehicle;

        p_vehicle = &things[p_person->U.UPerson.Vehicle];
        if ((p_vehicle->SubType == SubTT_VEH_GROUND) &&
          ((p_vehicle->Flag & TngF_Destroyed) == 0)) {
            p_vehicle->State = VehSt_WANDER;
        }
    }
    p_person->State = PerSt_WANDER;
    p_person->Timer1 = 48;
    p_person->StartTimer1 = 48;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.Timer2 = -1;
    p_person->SubState = sstate;

    if (sstate == PCmd_NONE) {
        p_person->SubState = PCmd_WAND_TIME;
        p_person->U.UPerson.ComTimer = 9999;
        p_person->U.UPerson.Timer2 = 5;
        p_person->U.UPerson.StartTimer2 = 50;
    } else if ((p_person->Flag & TngF_InVehicle) == 0) {
        p_person->U.UPerson.Timer2 = 10;
        p_person->U.UPerson.StartTimer2 = 10;
    }
    return StCh_ACCEPTED;
}

StateChRes person_init_go_to_point(struct Thing *p_person, short x, short y,
  short z, ushort range, TbBool fast_run)
{
    if ((p_person->Flag & TngF_InVehicle) != 0)
    {
        struct Thing *p_vehicle;

        p_vehicle = &things[p_person->U.UPerson.Vehicle];
        p_vehicle->Flag &= ~TngF_PlayerAgent;
        start_goto_vehicle(p_vehicle, x, z);
        p_person->State = PerSt_WAIT_VEHICLE;
        return StCh_ACCEPTED;
    }

    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);

    if (thing_arrived_at_obj_radius(p_person->ThingOffset, x, y, z, (range * range) << 12)) {
        p_person->State = PerSt_NONE;
        return StCh_ALREADY;
    }

    if (fast_run)
        set_person_animmode_run(p_person);

    p_person->State = PerSt_GOTO_POINT;
    p_person->U.UPerson.GotoX = x;
    p_person->U.UPerson.GotoZ = z;
    p_person->U.UPerson.ComRange = range;
    p_person->U.UPerson.ComTimer = -1;
    p_person->SubState = 0;
    p_person->Timer1 = 48;
    p_person->StartTimer1 = 48;
    return StCh_ACCEPTED;
}

StateChRes person_init_go_to_person(struct Thing *p_person, short target,
  ushort range, TbBool fast_run)
{
    if (fast_run)
        set_person_animmode_run(p_person);

    p_person->State = PerSt_GOTO_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->GotoThingIndex = target;
    p_person->U.UPerson.Timer2 = 50;
    p_person->U.UPerson.StartTimer2 = 50;
    p_person->SubState = 0;
    p_person->U.UPerson.ComRange = range;
    return StCh_ACCEPTED;
}

StateChRes person_init_kill_person(struct Thing *p_person, short target)
{
    int weapon_range;

    p_person->Flag2 &= ~TgF2_IgnoreEnemies;
    p_person->GotoThingIndex = target;
    // Command to kill target 0 is equivalent to taking out weapon
    if (target == 0)
    {
        get_weapon_out(p_person);
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    check_weapon(p_person, 1280);
    p_person->State = PerSt_KILL_PERSON;
    p_person->PTarget = &things[target];
    p_person->U.UPerson.ComTimer = -1;
    p_person->SubState = 0;

    get_weapon_out(p_person);

    weapon_range = get_weapon_range(p_person);
    p_person->U.UPerson.ComRange = weapon_range >> 6;

    p_person->U.UPerson.Timer2 = 10;
    p_person->U.UPerson.StartTimer2 = 10;
    return StCh_ACCEPTED;
}

short person_get_command_range_for_persuade(struct Thing *p_person)
{
    int weapon_range;
    int cmd_range;

    if (person_carries_weapon(p_person, WEP_PERSUADER2))
        weapon_range = get_hand_weapon_range(p_person, WEP_PERSUADER2);
    else
        weapon_range = get_hand_weapon_range(p_person, WEP_PERSUADRTRN);

    cmd_range = (weapon_range >> 6) * 3 / 4;
    if (cmd_range < 1)
        cmd_range = 1;
    return cmd_range;
}

StateChRes person_init_persuade_person(struct Thing *p_person, short target)
{
    if (target == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_PERSUADE_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->PTarget = &things[target];

    p_person->U.UPerson.ComRange = person_get_command_range_for_persuade(p_person);
    p_person->SubState = 0;

    p_person->U.UPerson.Timer2 = 10;
    p_person->U.UPerson.StartTimer2 = 10;
    return StCh_ACCEPTED;
}

StateChRes person_init_block_person(struct Thing *p_person, short target)
{
    if (target == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_BLOCK_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->PTarget = &things[target];
    p_person->U.UPerson.ComRange = 2;
    p_person->U.UPerson.Timer2 = 20;
    p_person->U.UPerson.StartTimer2 = 20;
    p_person->SubState = 0;
    return StCh_ACCEPTED;
}

StateChRes person_init_scare_person(struct Thing *p_person, short target)
{
    if (target == 0)
    {
        get_weapon_out(p_person);
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_SCARE_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->PTarget = &things[target];
    p_person->U.UPerson.ComRange = 2;
    p_person->U.UPerson.Timer2 = 50;
    p_person->U.UPerson.StartTimer2 = 50;
    p_person->SubState = 0;
    get_weapon_out(p_person);
    return StCh_ACCEPTED;
}

void person_init_follow_person(struct Thing *p_person, struct Thing *p_other)
{
    asm volatile (
      "call ASM_person_init_follow_person\n"
        : : "a" (p_person), "d" (p_other));
}

StateChRes person_init_cmd_follow_person(struct Thing *p_person, short target)
{
    //TODO it would probably make sense to reuse person_init_follow_person() here
    if (target == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_FOLLOW_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 1;
    p_person->U.UPerson.Timer2 = 50;
    p_person->U.UPerson.StartTimer2 = 50;
    p_person->SubState = 0;
    return StCh_ACCEPTED;
}

StateChRes person_init_support_person(struct Thing *p_person, short target)
{
    if (target == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_SUPPORT_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 3;
    p_person->U.UPerson.Timer2 = 50;
    p_person->U.UPerson.StartTimer2 = 50;
    p_person->SubState = 0;
    return StCh_ACCEPTED;
}

StateChRes person_init_protect_person(struct Thing *p_person, short target, TbBool one_target)
{
    if ((target == 0) && one_target)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_PROTECT_PERSON;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 8;
    p_person->U.UPerson.Timer2 = 50;
    p_person->U.UPerson.StartTimer2 = 50;
    p_person->SubState = 0;
    if (one_target)
        p_person->Owner = target;
    return StCh_ACCEPTED;
}

void person_init_get_item(struct Thing *p_person, short item, ushort plyr)
{
    asm volatile ("call ASM_person_init_get_item\n"
        : : "a" (p_person), "d" (item), "b" (plyr));
}

void person_init_get_item_fast(struct Thing *p_person, short item, ushort plyr)
{
    asm volatile ("call ASM_person_init_get_item_fast\n"
        : : "a" (p_person), "d" (item), "b" (plyr));
}

StateChRes person_init_cmd_get_item(struct Thing *p_person, short target)
{
    short tgtng_x, tgtng_z;

    //TODO it would probably make sense to reuse person_init_get_item() here
    if (target == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    get_thing_position_mapcoords(&tgtng_x, NULL, &tgtng_z, target);
    p_person->GotoThingIndex = target;
    p_person->State = PerSt_GET_ITEM;
    p_person->U.UPerson.GotoX = tgtng_x;
    p_person->U.UPerson.GotoZ = tgtng_z;
    p_person->U.UPerson.StandOnThing = 0;
    p_person->U.UPerson.ComTimer = -1;
    p_person->SubState = 0;
    p_person->U.UPerson.ComRange = 0;
    return StCh_ACCEPTED;
}

StateChRes person_init_cmd_use_weapon(struct Thing *p_person, short x, short y,
  short z, ushort weapon)
{
    struct WeaponDef *wdef;

    if (weapon == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    wdef = &weapon_defs[weapon];
    if (p_person->U.UPerson.Energy < wdef->EnergyUsed + 1)
        p_person->U.UPerson.Energy = wdef->EnergyUsed + 1;
    p_person->U.UPerson.CurrentWeapon = weapon;
    p_person->U.UPerson.WeaponDir = 32 * p_person->U.UPerson.Angle;
    thing_shoot_at_point(p_person, x, y, z, 0);
    process_weapon(p_person);
    p_person->Flag &= ~TngF_Unkn0800;
    return StCh_ACCEPTED;
}

StateChRes person_init_wand_avoid_group(struct Thing *p_person, ushort group, ubyte sstate)
{
    // Note we are not storing `group` - the one from `Command` will be used
    // (we should probably change that later though, state should not need to rely on command)
    p_person->State = PerSt_AVOID_GROUP;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.Timer2 = 5;
    p_person->U.UPerson.StartTimer2 = 5;
    p_person->Timer1 = 48;
    p_person->StartTimer1 = 48;
    p_person->SubState = sstate;
    return StCh_ACCEPTED;
}

StateChRes person_init_destroy_building(struct Thing *p_person, short x, short z, short target)
{
    int weapon_range;

    p_person->GotoThingIndex = target;
    p_person->State = PerSt_DESTROY_BUILDING;
    p_person->U.UPerson.ComTimer = -1;
    p_person->PTarget = &things[target];
    p_person->SubState = 0;

    p_person->U.UPerson.Timer2 = 10;
    p_person->U.UPerson.StartTimer2 = 10;

    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);

    get_weapon_out(p_person);

    // Make sure the target position is within current weapon range; our ComRange is in strange
    // units, they seem to be quarters of a tile; weapon_range is in normal map coords
    weapon_range = get_weapon_range(p_person);
    // Narrow range for drop weapons, wide range for throwing / shooting weapons
    if (((p_person->Flag & TngF_InVehicle) == 0) &&
      weapon_is_deployed_at_wielder_pos(p_person->U.UPerson.CurrentWeapon)) {
        p_person->U.UPerson.ComRange = (weapon_range >> 6);
    } else {
        p_person->U.UPerson.ComRange = (weapon_range >> 6) * 2 / 3;
    }
    //  the range needs to be larger than the person step distance
    p_person->U.UPerson.ComRange += PRCCOORD_TO_MAPCOORD(p_person->Speed + 255) >> 6;
    // Avoid zero range
    if (p_person->U.UPerson.ComRange < 2)
        p_person->U.UPerson.ComRange = 2;
    p_person->U.UPerson.GotoX = x;
    p_person->U.UPerson.GotoZ = z;

    return StCh_ACCEPTED;
}

StateChRes person_init_use_vehicle(struct Thing *p_person, short vehicle)
{
    short vhtng_x, vhtng_z;

    if (vehicle == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    get_thing_position_mapcoords(&vhtng_x, NULL, &vhtng_z, vehicle);
    p_person->GotoThingIndex = vehicle;
    p_person->State = PerSt_USE_VEHICLE;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 1;
    p_person->SubState = 0;
    p_person->U.UPerson.GotoX = vhtng_x;
    p_person->U.UPerson.GotoZ = vhtng_z;

    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);
    return StCh_ACCEPTED;
}

StateChRes person_init_exit_vehicle(struct Thing *p_person)
{
    if ((p_person->Flag & TngF_InVehicle) == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_ALREADY;
    }
    person_attempt_to_leave_vehicle(p_person);
    return StCh_ACCEPTED;
}

StateChRes person_init_catch_train(struct Thing *p_person, short face)
{
    // Station coords were converted to face index during level load
    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);

    build_navigate_path_to_face(p_person, face);
    p_person->State = PerSt_CATCH_TRAIN;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 0;
    p_person->SubState = 0;

    p_person->U.UPerson.Timer2 = 10;
    p_person->U.UPerson.StartTimer2 = 10;
    return StCh_ACCEPTED;
}

StateChRes person_open_dome(struct Thing *p_person, short dome)
{
    struct Thing *p_dome;

    if ((dome == 0) || !building_can_transform_open(dome))
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_dome = &things[dome];
    if (p_dome->SubType == SubTT_BLD_DOME)
        p_dome->Flag |= TngF_TransOpenRq;
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_close_dome(struct Thing *p_person, short dome)
{
    struct Thing *p_dome;

    if ((dome == 0) || !building_can_transform_open(dome))
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_dome = &things[dome];
    p_dome->Flag |= TngF_TransCloseRq;
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_lock_building(struct Thing *p_person, short bldng)
{
    struct Thing *p_building;

    if (bldng == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_building = &things[bldng];
    p_building->Flag |= TngF_Unkn0800;
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_unlock_building(struct Thing *p_person, short bldng)
{
    struct Thing *p_building;

    if (bldng == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    p_building = &things[bldng];
    p_building->Flag &= ~TngF_Unkn0800;
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_cmd_select_weapon(struct Thing *p_person, ushort weapon)
{
    p_person->U.UPerson.CurrentWeapon = weapon;
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_cmd_start_danger_music(struct Thing *p_person, TbBool revert)
{
    ubyte flag;

    flag = revert ? 0 : 1;
    update_danger_music(flag);
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_init_catch_ferry(struct Thing *p_person, short cor_x, short cor_z, ushort radius)
{
    p_person->State = PerSt_CATCH_FERRY;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 1;
    p_person->SubState = 0;
    p_person->U.UPerson.GotoX = cor_x;
    p_person->U.UPerson.GotoZ = cor_z;

    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);
    return StCh_ACCEPTED;
}

StateChRes person_init_exit_ferry(struct Thing *p_person, short portbld)
{
    short prtng_x, prtng_z;

    if (portbld == 0)
    {
        p_person->State = PerSt_NONE;
        return StCh_UNATTAIN;
    }
    get_thing_position_mapcoords(&prtng_x, NULL, &prtng_z, portbld);
    p_person->State = PerSt_EXIT_FERRY;
    p_person->U.UPerson.ComTimer = -1;
    p_person->U.UPerson.ComRange = 1;
    p_person->SubState = 0;
    p_person->U.UPerson.GotoX = prtng_x;
    p_person->U.UPerson.GotoZ = prtng_z;

    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);
    return StCh_ACCEPTED;
}

StateChRes person_or_vehicle_ping_exist(struct Thing *p_thing, TbBool revert)
{
    if (revert)
    {
        if (on_mapwho(p_thing))
            delete_node(p_thing);
        p_thing->Flag2 |= TgF2_ExistsOffMap;
    }
    else if ((p_thing->Flag2 & TgF2_ExistsOffMap) != 0)
    {
        p_thing->Flag2 &= ~TgF2_ExistsOffMap;
        if ((p_thing->Flag & TngF_InVehicle) == 0) {
            if (p_thing->Type == TT_PERSON)
                group_actions[p_thing->U.UPerson.Group].Alive++;
            add_node_thing(p_thing->ThingOffset);
        }
    }
    return StCh_ACCEPTED;
}

StateChRes person_guard_switch(struct Thing *p_person, TbBool revert)
{
    if ((revert && ((p_person->Flag & TngF_Unkn0008) == 0)) ||
      (!revert && ((p_person->Flag & TngF_Unkn0008) != 0)))
    {
        p_person->State = PerSt_NONE;
        return StCh_ALREADY;
    }
    if (revert)
    {
        p_person->Flag &= ~TngF_Unkn0008;
    }
    else
    {
        p_person->Flag |= TngF_Unkn0008;
    }
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_init_go_to_point_face(struct Thing *p_person, short x,
  short z, short face, ushort range)
{
    if ((p_person->Flag & TngF_InVehicle) != 0) {
        p_person->State = PerSt_NONE;
        return StCh_DENIED;
    }

    if (p_person->U.UPerson.PathIndex != 0)
        remove_path(p_person);

    build_navigate_path_to_face_xz(p_person, -face, x, z);
    p_person->U.UPerson.GotoX = x;
    p_person->U.UPerson.GotoZ = z;
    p_person->U.UPerson.ComRange = range;
    p_person->U.UPerson.ComTimer = -1;
    p_person->State = PerSt_GOTO_POINT;
    p_person->SubState = 0;

    p_person->Timer1 = 48;
    p_person->StartTimer1 = 48;
    return StCh_ACCEPTED;
}

StateChRes person_make_hard_as_agent(struct Thing *p_person)
{
    set_person_health_shield_type(p_person, SubTT_PERS_AGENT);
    set_person_energy_stamina_type(p_person, SubTT_PERS_AGENT);
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_make_fit_as_agent(struct Thing *p_person)
{
    set_person_energy_stamina_type(p_person, SubTT_PERS_AGENT);
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_init_cmd_wait_patiently(struct Thing *p_person, ubyte sstate)
{
    p_person->State = PerSt_WAIT;
    p_person->U.UPerson.ComTimer = -1;
    p_person->SubState = sstate;
    return StCh_ACCEPTED;
}

StateChRes person_init_cmd_wait_wth_timeout(struct Thing *p_person,
  ubyte sstate, short timeout)
{
    p_person->State = PerSt_WAIT;
    p_person->U.UPerson.ComTimer = timeout;
    p_person->SubState = sstate;
    return StCh_ACCEPTED;
}

StateChRes person_cmd_ignore_enemies(struct Thing *p_person, TbBool revert)
{
    if ((revert && ((p_person->Flag2 & TgF2_IgnoreEnemies) == 0)) ||
      (!revert && ((p_person->Flag2 & TgF2_IgnoreEnemies) != 0)))
    {
        p_person->State = PerSt_NONE;
        return StCh_ALREADY;
    }
    if (revert)
    {
        p_person->Flag2 &= ~TgF2_IgnoreEnemies;
    }
    else
    {
        p_person->Flag2 |= TgF2_IgnoreEnemies;
    }
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_cmd_stay_within_area(struct Thing *p_person, ushort cmd)
{
    if (p_person->U.UPerson.Within == cmd)
    {
        p_person->State = PerSt_NONE;
        return StCh_ALREADY;
    }
    p_person->U.UPerson.Within = cmd;
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

StateChRes person_cmd_play_sample(struct Thing *p_person, short smptbl_id,
  TbBool revert)
{
    if (revert)
    {
        play_sample_using_heap(0, smptbl_id, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
    }
    else
    {
        play_dist_sample(p_person, smptbl_id, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
    }
    p_person->State = PerSt_NONE;
    return StCh_ACCEPTED;
}

void camera_track_thing(short thing, TbBool revert)
{
    if (revert)
    {
        ThingIdx dcthing;
        dcthing = players[local_player_no].DirectControl[0];
        LOGDBG("Revert track to dcthing offs=%d", (int)dcthing);
        ingame.TrackThing = 0;
        game_set_cam_track_thing_xz(dcthing);
        return;
    }
    LOGDBG("Begin track thing offs=%d", (int)thing);
    ingame.TrackThing = thing;
}

void camera_rotate_view(short ang1, short ang2, TbBool revert)
{
    if (revert)
    {
        ingame.fld_unkCA6 = 0;
        return;
    }
    ingame.fld_unkCA6 = ang1;
    ingame.fld_unkCA8 = ang2;
}

TbBool person_init_specific_command(struct Thing *p_person, ushort cmd)
{
    struct Command *p_cmd;
    struct Thing *p_othertng;
    StateChRes res;
    short othertng;

    if ((debug_log_things & 0x01) != 0) {
        LOGSYNC("%s %d inits %s %d",
          thing_type_name(p_person->Type, p_person->SubType),
          (int)p_person->ThingOffset, command_codename(cmd), (int)cmd);
    }

    p_cmd = &game_commands[cmd];
    switch (p_cmd->Type)
    {
    case PCmd_NONE:
        res = person_init_cmd_wander(p_person, p_cmd->Type);
        break;
    case PCmd_GO_TO_POINT:
    case PCmd_RUN_TO_POINT:
        res = person_init_go_to_point(p_person, p_cmd->X, p_cmd->Y, p_cmd->Z, p_cmd->Arg1,
          (p_cmd->Type == PCmd_RUN_TO_POINT));
        break;
    case PCmd_GO_TO_PERSON:
        res = person_init_go_to_person(p_person, p_cmd->OtherThing, p_cmd->Arg1, false);
        break;
    case PCmd_KILL_PERSON:
        res = person_init_kill_person(p_person, p_cmd->OtherThing);
        break;
    case PCmd_KILL_MEM_GROUP:
        p_person->Flag2 &= ~TgF2_IgnoreEnemies;
        othertng = find_nearest_from_group(p_person, p_cmd->OtherThing, 0);
        if (othertng != 0)
        {
            //TODO is it really supposed to modify a vehicle? What is this?
            struct Thing *p_vehicle;
            p_vehicle = &things[p_person->U.UPerson.Vehicle];
            p_vehicle->Flag |= TngF_PlayerAgent;
        }
        res = person_init_kill_person(p_person, othertng);
        break;
    case PCmd_KILL_ALL_GROUP:
        othertng = find_nearest_from_group(p_person, p_cmd->OtherThing, 0);
        thing_group_set_kill_on_sight(p_person->U.UPerson.Group, p_cmd->OtherThing, true);
        res = person_init_kill_person(p_person, othertng);
        break;
    case PCmd_PERSUADE_PERSON:
        res = person_init_persuade_person(p_person, p_cmd->OtherThing);
        break;
    case PCmd_PERSUADE_MEM_GROUP:
    case PCmd_PERSUADE_ALL_GROUP:
        othertng = find_nearest_from_group(p_person, p_cmd->OtherThing, 1);
        res = person_init_persuade_person(p_person, othertng);
        break;
    case PCmd_BLOCK_PERSON:
        res = person_init_block_person(p_person, p_cmd->OtherThing);
        break;
    case PCmd_SCARE_PERSON:
        res = person_init_scare_person(p_person, p_cmd->OtherThing);
        break;
    case PCmd_FOLLOW_PERSON:
        res = person_init_cmd_follow_person(p_person, p_cmd->OtherThing);
        break;
    case PCmd_SUPPORT_PERSON:
        res = person_init_support_person(p_person, p_cmd->OtherThing);
        break;
    case PCmd_PROTECT_PERSON:
        res = person_init_protect_person(p_person, p_cmd->OtherThing, true);
        break;
    case PCmd_GET_ITEM:
        res = person_init_cmd_get_item(p_person, p_cmd->OtherThing);
        break;
    case PCmd_USE_WEAPON:
        res = person_init_cmd_use_weapon(p_person, p_cmd->X, p_cmd->Y, p_cmd->Z, p_cmd->OtherThing);
        break;
    case PCmd_DROP_SPEC_ITEM:
        person_init_drop_special(p_person, p_cmd->OtherThing);
        res = StCh_ACCEPTED;
        break;
    case PCmd_WAND_AVOID_GROUP:
        res = person_init_wand_avoid_group(p_person, p_cmd->OtherThing, p_cmd->Type);
        break;
    case PCmd_DESTROY_BUILDING:
        res = person_init_destroy_building(p_person, p_cmd->X, p_cmd->Z, p_cmd->OtherThing);
        break;
    case PCmd_USE_VEHICLE:
        res = person_init_use_vehicle(p_person, p_cmd->OtherThing);
        break;
    case PCmd_EXIT_VEHICLE:
        res = person_init_exit_vehicle(p_person);
        break;
    case PCmd_CATCH_TRAIN:
        res = person_init_catch_train(p_person, p_cmd->OtherThing);
        break;
    case PCmd_OPEN_DOME:
        res = person_open_dome(p_person, p_cmd->OtherThing);
        break;
    case PCmd_CLOSE_DOME:
        res = person_close_dome(p_person, p_cmd->OtherThing);
        break;
    case PCmd_DROP_WEAPON:
        person_init_drop(p_person, p_cmd->OtherThing);
        res = StCh_ACCEPTED;
        break;
    case PCmd_CATCH_FERRY:
        res = person_init_catch_ferry(p_person, p_cmd->X, p_cmd->Z, p_cmd->Arg1);
        break;
    case PCmd_EXIT_FERRY:
        res = person_init_exit_ferry(p_person, p_cmd->OtherThing);
        break;
    case PCmd_PING_EXIST:
        res = person_or_vehicle_ping_exist(p_person, ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        p_person->State = PerSt_NONE;
        break;
    case PCmd_GOTOPOINT_FACE:
        res = person_init_go_to_point_face(p_person, p_cmd->X, p_cmd->Z,
          p_cmd->OtherThing, p_cmd->Arg1);
        break;
    case PCmd_SELF_DESTRUCT:
        if (cybmod_chest_level(&p_person->U.UPerson.UMod) < 2)
            set_cybmod_chest_level(&p_person->U.UPerson.UMod, 2);
        person_self_destruct(p_person);
        res = StCh_ACCEPTED;
        break;
    case PCmd_PROTECT_MEM_G:
        othertng = find_nearest_from_group(p_person, p_cmd->OtherThing, 0);
        res = person_init_protect_person(p_person, othertng, false);
        break;
    case PCmd_KILL_EVERYONE:
        othertng = find_peep_in_area(p_person, p_cmd);
        res = person_init_kill_person(p_person, othertng);
        break;
    case PCmd_GUARD_OFF:
        res = person_guard_switch(p_person, ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        break;
    case PCmd_WAIT_TIME:
        p_cmd->Arg1 = 0;
        res = person_init_cmd_wait_patiently(p_person, p_cmd->Type);
        break;
    case PCmd_WAIT_P_V_DEAD:
    case PCmd_WAIT_MEM_G_DEAD:
    case PCmd_WAIT_ALL_G_DEAD:
    case PCmd_WAIT_P_V_I_NEAR:
    case PCmd_WAIT_MEM_G_NEAR:
    case PCmd_WAIT_ALL_G_NEAR:
    case PCmd_WAIT_P_V_I_ARRIVE:
    case PCmd_WAIT_MEM_G_ARRIVE:
    case PCmd_WAIT_ALL_G_ARRIVE:
    case PCmd_WAIT_P_PERSUADE:
    case PCmd_WAIT_MEM_G_PERSUADE:
    case PCmd_WAIT_ALL_G_PERSUADE:
    case PCmd_WAIT_MISSION_SUCC:
    case PCmd_WAIT_MISSION_FAIL:
    case PCmd_WAIT_MISSION_START:
    case PCmd_WAIT_OBJT_DESTROY:
    case PCmd_WAIT_OBJV:
        res = person_init_cmd_wait_patiently(p_person, p_cmd->Type);
        break;
    case PCmd_WAND_TIME:
        p_cmd->Arg1 = 0;
        res = person_init_cmd_wander(p_person, p_cmd->Type);
        break;
    case PCmd_WAND_P_V_DEAD:
    case PCmd_WAND_MEM_G_DEAD:
    case PCmd_WAND_ALL_G_DEAD:
    case PCmd_WAND_P_V_I_NEAR:
    case PCmd_WAND_MEM_G_NEAR:
    case PCmd_WAND_ALL_G_NEAR:
    case PCmd_WAND_P_V_I_ARRIVE:
    case PCmd_WAND_MEM_G_ARRIVE:
    case PCmd_WAND_ALL_G_ARRIVE:
    case PCmd_WAND_P_PERSUADE:
    case PCmd_WAND_MEM_G_PERSUADE:
    case PCmd_WAND_ALL_G_PERSUADE:
    case PCmd_WAND_MISSION_SUCC:
    case PCmd_WAND_MISSION_FAIL:
    case PCmd_WAND_MISSION_START:
    case PCmd_WAND_OBJT_DESTROY:
    case PCmd_WAND_OBJV:
        res = person_init_cmd_wander(p_person, p_cmd->Type);
        break;
    case PCmd_UNKN66:
        // Nothing done here
        person_command_select_next(p_person);
        return true;
    case PCmd_ADD_STATIC:
        add_static(p_cmd->X, p_cmd->Y, p_cmd->Z, 900, 10);
        person_command_select_next(p_person);
        return true;
    case PCmd_WAIT_TIME2:
        res = person_init_cmd_wait_wth_timeout(p_person, 0, p_cmd->Time);
        break;
    case PCmd_LOOP_COM:
        res = person_command_jump(p_person, p_cmd->OtherThing);
        break;
    case PCmd_WITHIN_AREA:
        res = person_cmd_stay_within_area(p_person, cmd);
        break;
    case PCmd_WITHIN_OFF:
        res = person_cmd_stay_within_area(p_person, 0);
        break;
    case PCmd_LOCK_BUILDN:
        res = person_lock_building(p_person, p_cmd->OtherThing);
        break;
    case PCmd_UNLOCK_BUILDN:
        res = person_unlock_building(p_person, p_cmd->OtherThing);
        break;
    case PCmd_SELECT_WEAPON:
        res = person_cmd_select_weapon(p_person, p_cmd->OtherThing);
        break;
    case PCmd_HARD_AS_AGENT:
        res = person_make_hard_as_agent(p_person);
        break;
    case PCmd_START_DANGER_MUSIC:
        res = person_cmd_start_danger_music(p_person, (p_cmd->Flags & PCmdF_RevertFunct) != 0);
        break;
    case PCmd_PING_P_V:
        p_othertng = &things[p_cmd->OtherThing];
        res = person_or_vehicle_ping_exist(p_othertng, ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        p_person->State = PerSt_NONE;
        break;
    case PCmd_CAMERA_TRACK:
        camera_track_thing(p_cmd->OtherThing, ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        p_person->State = PerSt_NONE;
        res = StCh_ACCEPTED;
        break;
    case PCmd_UNTRUCE_GROUP:
        thing_group_set_truce(p_person->U.UPerson.Group, p_cmd->OtherThing, false);
        p_person->State = PerSt_NONE;
        res = StCh_ACCEPTED;
        break;
    case PCmd_PLAY_SAMPLE:
        res = person_cmd_play_sample(p_person, p_cmd->OtherThing,
          ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        break;
    case PCmd_IGNORE_ENEMIES:
        res = person_cmd_ignore_enemies(p_person, ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        break;
    case PCmd_FIT_AS_AGENT:
        res = person_make_fit_as_agent(p_person);
        break;
    case PCmd_CAMERA_ROTATE:
        camera_rotate_view(p_cmd->OtherThing, p_cmd->Arg1, ((p_cmd->Flags & PCmdF_RevertFunct) != 0));
        res = StCh_ACCEPTED;
        break;
    case PCmd_STAY:
    case PCmd_HIDE:
    case PCmd_AVOID_PERSON:
    case PCmd_UNKN16:
    case PCmd_EXECUTE_COMS:
    case PCmd_UNKN27:
    case PCmd_UNKN28:
    case PCmd_UNKN29:
    case PCmd_UNKN2A:
    case PCmd_UNKN2B:
    case PCmd_UNKN2C:
    case PCmd_UNKN2D:
    case PCmd_UNKN2E:
    case PCmd_UNKN2F:
    case PCmd_UNKN30:
    case PCmd_UNKN31:
    case PCmd_UNKN32:
    case PCmd_UNKN44:
    case PCmd_UNKN45:
    case PCmd_UNKN46:
    case PCmd_UNKN58:
    case PCmd_UNKN59:
    case PCmd_UNKN5A:
    case PCmd_UNKN5B:
    case PCmd_UNKN5C:
    case PCmd_UNKN5D:
    case PCmd_UNKN5E:
    case PCmd_UNKN5F:
    case PCmd_UNKN60:
    case PCmd_UNKN61:
    case PCmd_UNKN62:
    case PCmd_UNKN63:
    case PCmd_UNKN64:
    case PCmd_UNKN65:
    case PCmd_UNKN67:
    case PCmd_UNKN6A:
    case PCmd_UNKN6B:
    case PCmd_UNKN6C:
    case PCmd_UNKN6D:
    default:
        res = StCh_ACCEPTED;
        break;
    case PCmd_UNTIL_P_V_DEAD:
    case PCmd_UNTIL_MEM_G_DEAD:
    case PCmd_UNTIL_ALL_G_DEAD:
    case PCmd_UNTIL_P_V_I_NEAR:
    case PCmd_UNTIL_MEM_G_NEAR:
    case PCmd_UNTIL_ALL_G_NEAR:
    case PCmd_UNTIL_P_V_I_ARRIVE:
    case PCmd_UNTIL_MEM_G_ARRIVE:
    case PCmd_UNTIL_ALL_G_ARRIVE:
    case PCmd_UNTIL_P_PERSUADE:
    case PCmd_UNTIL_MEM_G_PERSUADE:
    case PCmd_UNTIL_ALL_G_PERSUADE:
    case PCmd_UNTIL_MISSION_SUCC:
    case PCmd_UNTIL_MISSION_FAIL:
    case PCmd_UNTIL_MISSION_START:
    case PCmd_UNTIL_OBJT_DESTROY:
    case PCmd_UNTIL_TIME:
    case PCmd_UNTIL_OBJV:
    case PCmd_UNTIL_G_NOT_SEEN:
        // It should not be possible to reach this function with `until` command
        // is the `until` command not properly marked by flag?
        res = StCh_DENIED;
        break;
    }

    if ((debug_log_things & 0x01) != 0)
    {
        char locstr[192];

        snprint_command(locstr, sizeof(locstr), cmd);

        if (res <= StCh_ALREADY)
            LOGSYNC("Person %s %d %s %d %s, state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset, locstr,
              cmd, state_change_result_name(res), p_person->State, p_person->SubState);
        else
            LOGWARN("Person %s %d %s %d %s, state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset, locstr,
              cmd, state_change_result_name(res), p_person->State, p_person->SubState);
    }

    return false;
}

TbBool person_init_specific_preplay_command(struct Thing *p_person, ushort cmd)
{
    struct Command *p_cmd;

    p_cmd = &game_commands[cmd];

    switch (p_cmd->Type)
    {
    case PCmd_HARD_AS_AGENT:
        set_person_stats_type(p_person, SubTT_PERS_AGENT);
        break;
    case PCmd_FIT_AS_AGENT:
        set_person_energy_stamina_type(p_person, SubTT_PERS_AGENT);
        break;
    default:
        // found in-play command; terminate the pre-play loop
        return false;
    }

    if ((debug_log_things & 0x01) != 0)
    {
        char locstr[192];

        snprint_command(locstr, sizeof(locstr), cmd);

        LOGSYNC("Person %s %d %s %d %s, state %d.%d",
          person_type_name(p_person->SubType), (int)p_person->ThingOffset, locstr,
          cmd, "pre-played", p_person->State, p_person->SubState);
    }
    return true;
}

void person_init_preplay_command(struct Thing *p_person)
{
    ushort cmd;
    ubyte do_next;

    //TODO why are we discarding the commands completely from stack? just jump above them instead?
    for (cmd = p_person->U.UPerson.ComHead; cmd != 0; cmd = p_person->U.UPerson.ComHead)
    {
        do_next = person_init_specific_preplay_command(p_person, cmd);
        if (!do_next)
            break;
        person_command_skip_at_start(p_person);
    }
}

void person_init_command(struct Thing *p_person, ushort from)
{
#if 0
    asm volatile ("call ASM_person_init_command\n"
        : : "a" (p_person), "d" (from));
    return;
#endif
    struct Command *p_cmd;
    ushort cmd;
    ushort nxcmd;
    ubyte do_next;

    for (cmd = p_person->U.UPerson.ComCur; 1; )
    {
        p_person->Flag &= ~TngF_Unkn0800;
        p_person->U.UPerson.Target2 = 0;

        p_cmd = &game_commands[cmd];

        if ((p_person->Flag & TngF_Destroyed) != 0)
        {
            p_person->State = PerSt_DEAD;
            break;
        }

        if (cmd == 0)
        {
            if ((p_person->Flag2 & TgF2_Unkn0800) != 0)
            {
                p_person->Flag2 &= ~TgF2_Unkn0800;
                //TODO should we really keep previous substate?
                person_init_cmd_wait_wth_timeout(p_person, p_person->SubState, 50);
                ingame.Flags &= ~GamF_Unkn0100;
                set_peep_comcur(p_person);
            }
            break;
        }

        if ((p_cmd->Flags & PCmdF_RunUntil) != 0)
        {
            nxcmd = person_command_until_check_condition(p_person, cmd);
            if (nxcmd != CMD_CONDITION_CONTINUE) {
                person_command_jump(p_person, nxcmd);
                break;
            }
        }

        do_next = person_init_specific_command(p_person, cmd);
        if (!do_next)
            break;
    }
}

void assign_next_target_from_group(struct Thing *p_person, ushort group, ubyte flag)
{
    asm volatile ("call ASM_assign_next_target_from_group\n"
        : : "a" (p_person), "d" (group), "b" (flag));
    return;
}

short is_group_all_persuaded_by_me(ushort group, struct Thing *p_me, short count)
{
    // looks like a variation on group_has_no_less_members_persuaded_by_person(); reuse?
    short ret;
    asm volatile (
      "call ASM_is_group_all_persuaded_by_me\n"
        : "=r" (ret) : "a" (group), "d" (p_me), "b" (count));
    return ret;
}

ubyte is_command_completed(struct Thing *p_person)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_is_command_completed\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
#else
    struct Command *p_cmd;

    p_cmd = &game_commands[p_person->U.UPerson.ComCur];
    switch (p_cmd->Type)
    {
    case PCmd_KILL_MEM_GROUP:
        if (group_actions[p_cmd->OtherThing].Dead >= p_cmd->Arg2)
            return 1;
        assign_next_target_from_group(p_person, p_cmd->OtherThing, 0);
        p_person->State = PerSt_KILL_PERSON;
        return 0;
    case PCmd_KILL_ALL_GROUP:
        if (all_group_members_destroyed(p_cmd->OtherThing))
            return 1;
        assign_next_target_from_group(p_person, p_cmd->OtherThing, 0);
        p_person->State = PerSt_KILL_PERSON;
        return 0;
    case PCmd_PERSUADE_MEM_GROUP:
        if (is_group_all_persuaded_by_me(p_cmd->OtherThing, p_person, p_cmd->Arg2))
            return 1;
        assign_next_target_from_group(p_person, p_cmd->OtherThing, 1);
        p_person->State = PerSt_PERSUADE_PERSON;
        p_person->U.UPerson.ComRange = person_get_command_range_for_persuade(p_person);
        return 0;
    case PCmd_PERSUADE_ALL_GROUP:
        if (is_group_all_persuaded_by_me(p_cmd->OtherThing, p_person, -1))
            return 1;
        assign_next_target_from_group(p_person, p_cmd->OtherThing, 1);
        p_person->State = PerSt_PERSUADE_PERSON;
        p_person->U.UPerson.ComRange = person_get_command_range_for_persuade(p_person);
        return 0;
    default:
        return 1;
    }
#endif
}

int can_i_see_building(struct Thing *p_i, struct Thing *p_thing,
        int max_dist, ushort flags)
{
    int ret;
    asm volatile (
      "call ASM_can_i_see_building\n"
        : "=r" (ret) : "a" (p_i), "d" (p_thing), "b" (max_dist), "c" (flags));
    return ret;
}

void process_random_speech(struct Thing *p_person, ubyte a2)
{
#if 0
    asm volatile ("call ASM_process_random_speech\n"
        : : "a" (p_person), "d" (a2));
    return;
#endif
    ushort rndval;

    rndval = LbRandomAnyShort();
    if (ingame.TrackThing != 0)
        return;
    if (((p_person->Flag2 & TgF2_ExistsOffMap) != 0) || ((p_person->Flag & TngF_Persuaded) != 0))
        return;
    if (a2)
        return;

    switch (p_person->SubType)
    {
    case SubTT_PERS_MERCENARY:
        play_dist_speech(p_person, 57 + (rndval % 3), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    case SubTT_PERS_POLICE:
        play_dist_speech(p_person, 50 + (rndval & 3), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    case SubTT_PERS_SCIENTIST:
        play_dist_speech(p_person, 60 + (rndval % 2), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        break;
    }
}

void set_interrupt_target(struct Thing *p_person, struct Thing *p_target)
{
    asm volatile ("call ASM_set_interrupt_target\n"
        : : "a" (p_person), "d" (p_target));
    return;
}

void set_person_dead(struct Thing *p_person, ushort anim_mode)
{
#if 1
    asm volatile ("call ASM_set_person_dead\n"
        : : "a" (p_person), "d" (anim_mode));
    return;
#endif
}

uint find_and_alert_guardian(struct Thing *p_person, struct Thing *p_attacker)
{
    uint ret;
    asm volatile (
      "call ASM_find_and_alert_guardian\n"
        : "=r" (ret) : "a" (p_person), "d" (p_attacker));
    return ret;
}

void init_recoil(struct Thing *p_person, short vx, short vy, short vz, ushort type)
{
    asm volatile (
      "push %4\n"
      "call ASM_init_recoil\n"
        :  : "a" (p_person), "d" (vx), "b" (vy), "c" (vz), "g" (type));
}

void person_update_kill_stats(struct Thing *p_attacker, struct Thing *p_victim)
{
    if (p_attacker == NULL)
        return;

    if (in_network_game)
    {
        if ((p_victim->Flag & TngF_PlayerAgent) != 0)
        {
          struct Thing *p_realowner;
          PlayerIdx attack_plyr;

          p_realowner = effective_owner_of_thing(p_attacker);

          if ((p_realowner != NULL) && ((p_realowner->Flag & TngF_PlayerAgent) != 0))
          {
              attack_plyr = p_realowner->U.UPerson.ComCur >> 2;
              killed_mp_agent_add_to_stats(p_victim, attack_plyr);
          }
        }
    }
    else
    {
        if (p_attacker->U.UObject.EffectiveGroup == ingame.MyGroup) {
            killed_person_add_to_stats(p_victim, open_brief);
        }
    }
}

/** Set a person into dying state.
 *
 * @param p_person The person which will start dying.
 * @param hp Amount of Hit Points subtracted from health by the final blow.
 * @param type Type of damage which caused the death.
 */
void person_start_dying(struct Thing *p_person, int hp, ushort type)
{
    switch (type)
    {
    case DMG_ELLASER:
    case DMG_BEAM:
    case DMG_LASER:
    case DMG_ELSTRAND:
        set_person_dead(p_person, ANIM_PERS_Unkn12);
        break;
    default:
    case DMG_UNKN5:
    case DMG_MINIGUN:
    case DMG_UNKN9:
        set_person_dead(p_person, ANIM_PERS_LAY_DOWN);
        break;
    case DMG_RAP:
    case DMG_LONGRANGE:
        set_person_dead(p_person, ANIM_PERS_Unkn10);
        break;
    }
}

TbBool persons_have_truce(struct Thing *p_person1, struct Thing *p_person2)
{
    ubyte pers1grp, pers2grp;

    if (p_person1 == NULL) {
        return false;
    }

    if (p_person2 == NULL) {
        return false;
    }

    pers1grp = p_person1->U.UPerson.EffectiveGroup;
    pers2grp = p_person2->U.UPerson.EffectiveGroup & 0x7F;

    return thing_group_have_truce(pers1grp, pers2grp);
}

void persons_set_groups_kill_on_sight(struct Thing *p_attacker, struct Thing *p_victim)
{
    ubyte attack_grp, victim_grp;

    if ((p_attacker == NULL) || ((p_attacker->Flag & TngF_PlayerAgent) == 0)) {
        return;
    }

    if ((p_victim == NULL) || (p_victim != p_attacker->PTarget)) {
        return;
    }

    attack_grp = p_attacker->U.UPerson.EffectiveGroup;
    victim_grp = p_victim->U.UPerson.EffectiveGroup & 0x7F;

    if (p_victim->SubType == SubTT_PERS_BRIEFCASE_M || p_victim->SubType == SubTT_PERS_WHITE_BRUN_F ||
      p_victim->SubType == SubTT_PERS_WHIT_BLOND_F || p_victim->SubType == SubTT_PERS_LETH_JACKT_M) {
        return;
    }
    thing_group_set_kill_on_sight(attack_grp, victim_grp, true);
}

int mods_affect_hit_points(struct Thing *p_thing, ushort type, int hp)
{
    switch (type)
    {
    case DMG_UZI:
    case DMG_MINIGUN:
    case DMG_LONGRANGE:
        if (person_mod_skin_level(p_thing) == 1)
            hp >>= 1;
        break;
    case DMG_ELLASER:
    case DMG_BEAM:
    case DMG_LASER:
    case DMG_ELSTRAND:
        if (person_mod_skin_level(p_thing) == 3)
            hp >>= 1;
        break;
    case DMG_UNKN5:
    case DMG_RAP:
        break;
    case DMG_UNKN9:
        if (person_mod_skin_level(p_thing) == 1)
            hp = 3 * (hp >> 2);
        break;
    default:
        break;
    }
    return hp;
}

int dead_person_hit_by_bullet(struct Thing *p_thing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type)
{
    if (p_thing->U.UPerson.AnimMode == ANIM_PERS_DEAD_BODY)
    {
        if (type == DMG_RAP)
        {
            p_thing->Timer1 = 16;
            p_thing->StartTimer1 = 16;
            p_thing->SubState = 26;
            p_thing->U.UPerson.FrameId.Version[4] = 0;
            p_thing->U.UPerson.FrameId.Version[3] = 0;
            p_thing->U.UPerson.AnimMode = ANIM_PERS_Unkn10;
            reset_person_frame(p_thing);
            return 1;
        }
        if (type == DMG_UZI || type == DMG_MINIGUN || type == DMG_LONGRANGE || type == DMG_UNKN9)
        {
            p_thing->Frame = frame[p_thing->Frame].Next;
            return 1;
        }
        p_thing->Timer1 = 16;
        p_thing->StartTimer1 = 16;
        p_thing->SubState = 26;
        p_thing->U.UPerson.FrameId.Version[4] = 0;
        p_thing->U.UPerson.FrameId.Version[3] = 0;
        p_thing->U.UPerson.AnimMode = ANIM_PERS_Unkn12;
        set_person_frame_noangle(p_thing, 1067);
        play_dist_sample(p_thing, 57, FULL_VOL, EQUL_PAN, NORM_PTCH, 0, 3);
    }
    return 1;
}

int person_hit_by_bullet(struct Thing *p_thing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type)
{
#if 0
    int ret;
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_person_hit_by_bullet\n"
        : "=r" (ret) : "a" (p_thing), "d" (hp), "b" (vx), "c" (vy), "g" (vz), "g" (p_attacker), "g" (type));
    return ret;
#endif
    short hp1;
    int energy_decr;

    hp1 = hp;

    if ((p_thing->Flag & TngF_Unkn40000000) != 0)
        return 1;

    switch (p_thing->Type)
    {
    case TT_PERSON:
        if (p_thing->State == PerSt_PERSON_BURNING)
            return 1;
        if ((p_thing->Flag & TngF_InVehicle) != 0)
        {
            p_thing = &things[p_thing->U.UPerson.Vehicle];
            if (p_thing->Type != TT_VEHICLE)
                return 0;
        }
        if (p_thing->State == PerSt_DEAD)
        {
            return dead_person_hit_by_bullet(p_thing, hp, vx, vy, vz, p_attacker, type);
        }
        if ((p_thing->Flag & TngF_Destroyed) != 0)
            return 1;
        if ((p_thing->Flag & TngF_Unkn40000000) != 0)
            return 1;
        break;
    default:
        break;
    }

    switch (p_thing->Type)
    {
    case SmTT_STATIC:
        return static_hit_by_bullet((struct SimpleThing *)p_thing,
          hp, vx, vy, vz, p_attacker, type);
    case TT_VEHICLE:
        return vehicle_hit_by_bullet(p_thing, hp, vx, vy, vz, p_attacker, type);
    case TT_PERSON:
          if (persons_have_truce(p_attacker, p_thing)) {
              return 1;
          }
          persons_set_groups_kill_on_sight(p_attacker, p_thing);
          hp1 = mods_affect_hit_points(p_thing, type, hp);

          if ((p_attacker != NULL) && (type != DMG_ELSTRAND) && ((p_thing->Flag & TngF_Unkn1000) == 0)
            && ((p_thing->Flag & TngF_Destroyed) == 0) && ((p_attacker->Flag2 & TgF2_ExistsOffMap) == 0)
            && ((p_thing->Flag2 & TgF2_KnockedOut) == 0))
          {
              set_interrupt_target(p_thing, p_attacker);
          }
          if ((p_thing->Flag & TngF_Destroyed) != 0) {
              return 1;
          }
          if ((p_attacker != NULL) && (type != DMG_ELSTRAND)
            && !thing_group_have_truce(p_attacker->U.UPerson.EffectiveGroup, p_thing->U.UPerson.EffectiveGroup & 0x7F)
            && (((p_attacker->Flag & TngF_Unkn1000) != 0) || ((p_thing == p_attacker->PTarget) && (p_attacker->Flag2 & TgF2_ExistsOffMap) == 0)))
          {
              ubyte attack_grp, victim_grp;
              victim_grp = p_thing->U.UPerson.EffectiveGroup;
              attack_grp = p_attacker->U.UPerson.EffectiveGroup;
              if ( victim_grp <= 0x63u && attack_grp <= 0x63u && victim_grp != attack_grp )
              {
                if ((p_thing->Flag & TngF_Persuaded) == 0)
                    thing_group_set_kill_on_sight(victim_grp, attack_grp, true);
                if (((p_thing->Flag & TngF_Persuaded) == 0) && war_flags[victim_grp].Guardians[0])
                    find_and_alert_guardian(p_thing, p_attacker);
              }
          }
          energy_decr = 0;
          if ((p_thing->Flag & TngF_Unkn00200000) != 0)
          {
              p_thing->U.UPerson.ShieldEnergy -= hp1;
              p_thing->U.UPerson.ShieldGlowTimer = 4;
              if (p_thing->U.UPerson.ShieldEnergy >= 0)
              {
                  if (p_thing->U.UPerson.ShieldEnergy > PERSON_MAX_SHIELD)
                      p_thing->U.UPerson.ShieldEnergy = PERSON_MAX_SHIELD;
                  return 0;
              }
              p_thing->Health += p_thing->U.UPerson.ShieldEnergy;
              energy_decr = -p_thing->U.UPerson.ShieldEnergy;
          }
          else
          {
              if (p_thing->U.UPerson.RecoilTimer <= 1)
              {
                  ushort smpl_no;

                  switch (person_sex(p_thing))
                  {
                  case PERSON_FEMALE:
                      smpl_no = sfx_woman_shot[(gameturn + p_thing->ThingOffset) & 1];
                      break;
                  case PERSON_MALE:
                  default:
                      smpl_no = sfx_man_shot[(gameturn + p_thing->ThingOffset) & 7];
                      break;
                  }
                  play_dist_sample(p_thing, smpl_no, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
              }
              p_thing->Health -= hp1;
              p_thing->U.UPerson.ShieldEnergy -= hp1;
          }

        if (p_thing->Health <= 0)
        {
            int prev_health;
            prev_health = p_thing->Health + hp1 + energy_decr;
            person_start_dying(p_thing, hp1 + energy_decr, type);
            person_update_kill_stats(p_attacker, p_thing);
            return prev_health;
        }

        if ((p_thing->Flag2 & TgF2_KnockedOut) == 0)
        {
            init_recoil(p_thing, vx, vy, vz, type);
            return 0;
        }
        break;
    case TT_BUILDING:
        return building_hit_by_bullet(p_thing, hp, vx, vy, vz, p_attacker, type);
    case TT_MINE:
        return mine_hit_by_bullet(p_thing, hp, vx, vy, vz, p_attacker, type);
        break;
    default:
        break;
    }
    return 0;
}

TbBool person_use_medikit(struct Thing *p_person, PlayerIdx plyr)
{
    if (plyr == local_player_no)
        play_sample_using_heap(0, 2, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
    p_person->Health = p_person->U.UPerson.MaxHealth;
    if (person_carries_weapon(p_person, WEP_MEDI1)) {
        person_weapons_remove_one(p_person, WEP_MEDI1);
    } else if (person_carries_weapon(p_person, WEP_MEDI2)) {
        person_weapons_remove_one(p_person, WEP_MEDI2);
    }
    return true;
}

int person_goto_person_nav(struct Thing *p_person)
{
    int ret;
    asm volatile (
      "call ASM_person_goto_person_nav\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
}

void person_goto_point(struct Thing *p_person)
{
    asm volatile ("call ASM_person_goto_point\n"
        : : "a" (p_person));
}

void person_goto_point_rel(struct Thing *p_person)
{
    asm volatile ("call ASM_person_goto_point_rel\n"
        : : "a" (p_person));
}

void person_init_drop(struct Thing *p_person, ThingIdx item)
{
    asm volatile ("call ASM_person_init_drop\n"
        : : "a" (p_person), "d" (item));
}

void person_init_pickup(struct Thing *p_person, ThingIdx item)
{
    asm volatile ("call ASM_person_init_pickup\n"
        : : "a" (p_person), "d" (item));
}

void vehicle_passenger_list_add_first(struct Thing *p_vehicle, ThingIdx passngr)
{
    struct Thing *p_passngr;
    ThingIdx nxpassngr;

    p_passngr = &things[passngr];
    nxpassngr = p_vehicle->U.UVehicle.PassengerHead;
    p_vehicle->U.UVehicle.PassengerHead = passngr;
    p_passngr->U.UPerson.LinkPassenger = nxpassngr;
}

void vehicle_passenger_list_remove(struct Thing *p_vehicle, ThingIdx passngr)
{
    struct Thing *p_passngr;
    struct Thing *p_pvpassngr;
    ThingIdx crpassngr, pvpassngr;
    ushort k;

    p_passngr = &things[passngr];

    // If removing first passenger
    if (passngr == p_vehicle->U.UVehicle.PassengerHead)
    {
        p_vehicle->U.UVehicle.PassengerHead = p_passngr->U.UPerson.LinkPassenger;
        return;
    }

    k = 0;
    crpassngr = p_vehicle->U.UVehicle.PassengerHead;
    pvpassngr = 0;
    while (crpassngr != 0)
    {
        if (k >= VEHICLE_PASSENGER_LIMIT)
            break;
        if (crpassngr == passngr) {
            p_pvpassngr = &things[pvpassngr];
            p_pvpassngr->U.UPerson.LinkPassenger = p_passngr->U.UPerson.LinkPassenger;
            break;
        }
        pvpassngr = crpassngr;
        p_pvpassngr = &things[pvpassngr];
        crpassngr = p_pvpassngr->U.UPerson.LinkPassenger;
        k++;
    }
}

TbBool person_is_in_a_vehicle(struct Thing *p_person)
{
    return ((p_person->Flag & TngF_InVehicle) != 0);
}

TbBool person_is_in_a_train(struct Thing *p_person)
{
    struct Thing *p_vehicle;

    if ((p_person->Flag & TngF_InVehicle) == 0)
        return false;

    p_vehicle = &things[p_person->U.UPerson.Vehicle];
    return (p_vehicle->SubType == SubTT_VEH_TRAIN);
}

TbBool person_is_in_vehicle(struct Thing *p_person, ThingIdx vehicle)
{
    if ((p_person->Flag & TngF_InVehicle) == 0)
        return false;
    return (vehicle == p_person->U.UPerson.Vehicle);
}

TbBool person_is_standing_on_vehicle(struct Thing *p_person)
{
    struct Thing *p_standtng;

    if (p_person->U.UPerson.StandOnThing <= 0)
        return false;
    if ((p_person->Flag & TngF_StandOnVehicle) == 0)
        return false;
    p_standtng = &things[p_person->U.UPerson.StandOnThing];

    return (p_standtng->Type == TT_VEHICLE);
}

void person_enter_vehicle(struct Thing *p_person, struct Thing *p_vehicle)
{
#if 0
    asm volatile ("call ASM_person_enter_vehicle\n"
        : : "a" (p_person), "d" (p_vehicle));
#endif
    if ((p_vehicle->SubType == SubTT_VEH_SHIP) && (p_vehicle->State == VehSt_UNKN_40))
    {
        // Allow ships to be entered when in motion, because we just do not have the proper functionality to stop them
    }
    else if ((p_vehicle->State != VehSt_PARKED_PARAL) && (p_vehicle->State != VehSt_PARKED_PERPN) && (p_vehicle->State != VehSt_NONE))
    {
        if ((debug_log_things & 0x01) != 0) {
            LOGSYNC("Person %s %d state %d.%d cannot enter %s %d state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              p_person->State, p_person->SubState,
              vehicle_type_name(p_vehicle->SubType), (int)p_vehicle->ThingOffset,
              p_vehicle->State, p_vehicle->SubState);
        }
        return;
    }

    p_person->Flag |= TngF_InVehicle|TngF_Unkn02000000;
    p_person->Flag &= ~TngF_StandOnVehicle;
    p_person->State = PerSt_DRIVING_VEHICLE;

    vehicle_passenger_list_add_first(p_vehicle, p_person->ThingOffset);

    if ((p_person->Flag2 & TgF2_ExistsOffMap) == 0)
        delete_node(p_person);
    p_person->X = p_vehicle->X;
    p_person->Y = p_vehicle->Y;
    p_person->Z = p_vehicle->Z;
    p_vehicle->U.UVehicle.EffectiveGroup = p_person->U.UPerson.EffectiveGroup;

    if ((p_person->Flag2 & TgF2_Unkn00080000) != 0)
        set_person_animmode_walk(p_person);
}

void person_go_sleep(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_go_sleep\n"
        : : "a" (p_person));
    return;
#endif
    if (p_person->State == PerSt_PERSON_BURNING) {
        return;
    }
    if (((p_person->Flag & TngF_Destroyed) != 0) ||
      (p_person->State == PerSt_DIEING)) {
        return;
    }
    if ((p_person->Flag2 & TgF2_KnockedOut) != 0)
    {
        p_person->U.UPerson.BumpCount = 150;
    }
    else
    {
        ubyte PrevAnimMode;
        p_person->Flag |= 0x0001;

        PrevAnimMode = p_person->U.UPerson.AnimMode;
        if (PrevAnimMode != ANIM_PERS_PUSH_BACK)
            p_person->U.UPerson.OldAnimMode = PrevAnimMode;
        p_person->U.UPerson.AnimMode = ANIM_PERS_LAY_DOWN;
         // make sure to draw without blood
        p_person->U.UPerson.FrameId.Version[4] = 0;
        p_person->U.UPerson.FrameId.Version[3] = 0;
        reset_person_frame(p_person);

        p_person->U.UPerson.RecoilTimer = 0;
        p_person->Timer1 = 48;
        p_person->StartTimer1 = 48;
        p_person->U.UPerson.BumpCount = 150;
        p_person->Flag &= ~(TngF_Unkn0800|TngF_WepRecoil);
        p_person->Flag2 |= TgF2_KnockedOut;
    }
}

ubyte vector_in_way(struct Thing *p_thing, int dx, int dz)
{
#if 1
    ubyte ret;
    asm volatile (
      "call ASM_vector_in_way\n"
        : "=r" (ret) : "a" (p_thing), "d" (dx), "b" (dz));
    return ret;
#endif
}

ushort check_col_collision(int x, int y, int z)
{
#if 1
    ushort ret;
    asm volatile (
      "call ASM_check_col_collision\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (z));
    return ret;
#endif
}

ThingIdx thing_collide_with_thing_when_moved_by(struct Thing *p_person, short sh_x, short sh_z)
{
    int nxcor_x, nxcor_z;
    ThingIdx thing, nxthing;

    nxcor_x = PRCCOORD_TO_MAPCOORD(p_person->X) + sh_x;
    nxcor_z = PRCCOORD_TO_MAPCOORD(p_person->Z) + sh_z;

    if ((nxcor_x < 0) || (nxcor_x >= MAP_COORD_WIDTH) || (nxcor_z < 0) || (nxcor_z >= MAP_COORD_HEIGHT))
        return 0;

    thing = get_mapwho_thing_index(MAPCOORD_TO_TILE(nxcor_x), MAPCOORD_TO_TILE(nxcor_z));
    for (; thing != 0; thing = nxthing)
    {
        int dist_x, dist_z, r;
        if (thing < 0)
        {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[thing];
            dist_x = nxcor_x - PRCCOORD_TO_MAPCOORD(p_sthing->X);
            dist_z = nxcor_z - PRCCOORD_TO_MAPCOORD(p_sthing->Z);
            r = p_person->Radius + p_sthing->Radius;
            nxthing = p_sthing->Next;
        }
        else
        {
            struct Thing *p_thing;
            p_thing = &things[thing];
            dist_x = nxcor_x - PRCCOORD_TO_MAPCOORD(p_thing->X);
            dist_z = nxcor_z - PRCCOORD_TO_MAPCOORD(p_thing->Z);
            r = p_person->Radius + p_thing->Radius;
            nxthing = p_thing->Next;
        }
        if (LbSqrL(dist_x * dist_x + dist_z * dist_z) <= r)
        {
            break;
        }
    }
    return thing;
}

TbBool check_ground_unkn01(struct Thing *p_person, short sh_x, short sh_z)
{
    struct MyMapElement *p_mapel;
    ushort tflags;
    MapCoord cor_x, cor_z;
    short tile_x, tile_z;
    ushort txtr;
    TbBool tile_blocking;

    cor_x = PRCCOORD_TO_MAPCOORD(p_person->X) + sh_x;
    cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z) + sh_z;
    tile_x = MAPCOORD_TO_TILE(cor_x);
    tile_z = MAPCOORD_TO_TILE(cor_z);

    tile_blocking = compute_map_tile_is_blocking_walk(tile_x, tile_z);

    txtr = floor_texture_at_point(cor_x, cor_z);
    tflags = ((get_my_texture_bits(txtr) & 0xC0) >> 6) & 0x02;

    if ((tile_x < 0) || (tile_x >= MAP_TILE_WIDTH) || (tile_z < 0) || (tile_z >= MAP_TILE_HEIGHT))
    {
      return true;
    }

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];

    if (tile_blocking)
        tflags |= 0x04;
    if ((p_mapel->Flags2 & MEF2_Unkn02) != 0)
        tflags &= ~0x04;
    if (((tflags & 0x04) != 0) || (p_mapel->Flags2 & (MEF2_Unkn01|MEF2_Unkn04)) != 0)
    {
        return true;
    }
    return false;
}


short check_col_collision_when_moved_by(struct Thing *p_person, short sh_x, short sh_z)
{
    int cor_x, cor_y, cor_z;
    int dist;
    int dt_x, dt_z;

    dist = LbSqrL(sh_x * sh_x + sh_z * sh_z) >> 6;
    if (dist == 0)
        dist = 1;
    cor_x = p_person->X >> 8;
    cor_z = p_person->Z >> 8;
    cor_y = (alt_at_point(cor_x, cor_z) >> 8) + 3;
    dt_x = sh_x / dist;
    dt_z = sh_z / dist;
    while (dist != 0)
    {
        short qbit;

        qbit = check_col_collision(cor_x, cor_y, cor_z);
        if (qbit != 0)
        {
            return qbit;
        }
        cor_z += dt_z;
        cor_x += dt_x;
        dist--;
        cor_y = (alt_at_point(cor_x, cor_z) >> 8) + 3;
    }
    return 0;
}

ubyte person_leave_vehicle(struct Thing *p_person, struct Thing *p_vehicle)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_person_leave_vehicle\n"
        : "=r" (ret) : "a" (p_person), "d" (p_vehicle));
    return ret;
#endif
    int sh_x, sh_z;
    int can_move;

    if ((p_vehicle->State != VehSt_PARKED_PARAL) && (p_vehicle->State != VehSt_PARKED_PERPN) &&
      (p_vehicle->State != VehSt_UNKN_D))
    {
        if ((p_vehicle->State != VehSt_UNKN_33) &&  (p_vehicle->State != VehSt_UNKN_36) &&
          (p_vehicle->State != VehSt_UNKN_3E) && (p_vehicle->State != VehSt_UNKN_38))
            p_vehicle->State = VehSt_UNKN_3C;
        return 1;
    }
    if ((p_person->Flag & TngF_InVehicle) == 0)
    {
        return 2;
    }

    can_move = 0;
    sh_x = 0;
    sh_z = 0;
    if ((p_vehicle->SubType != SubTT_VEH_FLYING)
      || alt_at_point(p_vehicle->X >> 8, p_vehicle->Z >> 8) + 3500 >= p_vehicle->Y)
    {
        int dt_angle;

        for (dt_angle = 256; dt_angle < 2048; )
        {
            short angle;
            ushort radius;
            int cor_x, cor_z;

            angle = (dt_angle + p_vehicle->U.UVehicle.AngleY) & 0x7FF;
            radius = p_vehicle->Radius;
            sh_z = -(radius * lbSinTable[angle + LbFPMath_PI/2]) >> 16;
            sh_x = (radius * lbSinTable[angle]) >> 16;

            cor_x = (p_person->X >> 8) + sh_x;
            cor_z = (p_person->Z >> 8) + sh_z;
            can_move = (cor_z >= 0) && (cor_z < MAP_COORD_HEIGHT) && (cor_x >= 0) && (cor_x < MAP_COORD_WIDTH);

            if (can_move && check_ground_unkn01(p_person, sh_x, sh_z)) {
                can_move = 0;
            }
            if (can_move) {
                can_move = vector_in_way(p_person, sh_x, sh_z);
            }
            if (can_move && check_col_collision_when_moved_by(p_person, sh_x, sh_z)) {
                can_move = 0;
            }
            if (can_move && thing_collide_with_thing_when_moved_by(p_person, sh_x, sh_z)) {
                can_move = 0;
            }
            dt_angle += 180;
            if (dt_angle >= 1024 && dt_angle < 1280)
                dt_angle = 1280;
            if (can_move != 0)
                break;
        }
    }

    if ( can_move || (p_vehicle->State == VehSt_UNKN_D))
    {
      struct Command *p_cmd;
      struct Command *p_nxcmd;

      p_person->Flag &= ~(TngF_InVehicle|TngF_Unkn02000000);
      vehicle_passenger_list_remove(p_vehicle, p_person->ThingOffset);
      p_person->State = PerSt_NONE;

      p_cmd = &game_commands[p_person->U.UPerson.ComCur];
      p_nxcmd = &game_commands[p_cmd->Next];

      if ((p_person->U.UPerson.ComHead != 0) &&
        ((p_nxcmd->Type == PCmd_GO_TO_POINT) || (p_nxcmd->Type == PCmd_GOTOPOINT_FACE)
         || (p_nxcmd->Type == PCmd_RUN_TO_POINT)))
      {
          sh_x >>= 2;
          sh_z >>= 2;
      }
      p_person->X += sh_x << 8;
      p_person->Z += sh_z << 8;
      if ((p_person->X >> 8) >= MAP_COORD_WIDTH || (p_person->Z >> 8) >= MAP_COORD_HEIGHT)
      {
          p_person->X -= (sh_x << 8);
          p_person->Z -= (sh_z << 8);
      }
      p_person->Y = alt_at_point(p_person->X >> 8, p_person->Z >> 8);
      p_person->Flag |= TngF_Unkn0010|TngF_Unkn0004;

      struct MyMapElement *p_mapel;
      short tile_x, tile_z;

      tile_z = (p_person->Z >> 16);
      tile_x = (p_person->X >> 16);
      p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];

      if (((p_mapel->Flags2 & MEF2_Unkn01) != 0) && can_move)
          can_move = 0;

      if ((p_person->Flag2 & TgF2_ExistsOffMap) == 0)
          add_node_thing(p_person->ThingOffset);
      p_person->Timer1 = 48;
      p_person->StartTimer1 = 48;
      if (p_vehicle->U.UVehicle.PassengerHead == 0)
          p_vehicle->U.UVehicle.EffectiveGroup = 99;
      remove_path(p_person);

      if (((p_person->Flag & TngF_PlayerAgent) != 0) && (p_person->State == PerSt_PROTECT_PERSON))
      {
          short plagent, owagent;

          plagent = p_person->U.UPerson.ComCur & 3;
          owagent = things[p_person->Owner].U.UPerson.ComCur & 3;
          p_person->U.UPerson.ComRange = follow_dist[owagent][plagent];
      }

      if ((p_vehicle->Flag & TngF_Destroyed) != 0)
      {
        if (!can_move)
        {
          // SP stats are updated inside set_person_dead(), but MP stats not? maybe add a wrapper to include both?
          if (in_network_game)
              stats_mp_add_person_kills_person(p_vehicle->OldTarget, p_person->ThingOffset);
          set_person_dead(p_person, ANIM_PERS_Unkn10);
          return 0;
        }
        p_person->Health -= 800;
        if (p_person->Health <= 0)
        {
            set_person_dead(p_person, ANIM_PERS_Unkn10);
            return 0;
        }
        if ((p_person->Flag & TngF_PlayerAgent) == 0)
        {
            person_go_sleep(p_person);
        }
      }
    }
    return 0;
}

ubyte person_attempt_to_leave_vehicle(struct Thing *p_person)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_person_attempt_to_leave_vehicle\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
#endif
    struct Thing *p_vehicle;

    p_vehicle = &things[p_person->U.UPerson.Vehicle];
    if (p_vehicle->State == VehSt_UNKN_45) {
        return 0;
    }
    if ((p_vehicle->Flag & TngF_Destroyed) != 0) {
        return 0;
    }
    switch (p_vehicle->SubType)
    {
    case SubTT_VEH_TRAIN:
        break;
    case SubTT_VEH_TANK:
        if (p_vehicle->State != SubTT_VEH_UNKN33) {
            p_vehicle->U.UVehicle.GotoX = p_vehicle->X >> 8;
            p_vehicle->U.UVehicle.GotoZ = p_vehicle->Z >> 8;
            break;
        }
        person_leave_vehicle(p_person, p_vehicle);
        return 1;
    case SubTT_VEH_FLYING:
        if (p_vehicle->State != VehSt_PARKED_PARAL)
        {
            if (p_vehicle->State != VehSt_FLY_LANDING)
                start_landing(p_vehicle);
            p_person->State = VehSt_UNKN_36;
            break;
        }
        if (!person_leave_vehicle(p_person, p_vehicle))
        {
            if ((p_person->Flag & TngF_PlayerAgent) != 0)
                p_person->State = PerSt_NONE;
            break;
        }
        return 1;
    case SubTT_VEH_SHIP:
    default:
        if ((p_vehicle->State != VehSt_PARKED_PARAL) && (p_vehicle->State != VehSt_PARKED_PERPN)
          && (p_vehicle->State != VehSt_NONE)
          && (p_vehicle->State != VehSt_UNKN_3E) && (p_vehicle->State != VehSt_UNKN_38))
        {
            if ((p_vehicle->State != VehSt_UNKN_33) && (p_vehicle->State != VehSt_UNKN_36))
                p_vehicle->State = VehSt_UNKN_3C;
            p_person->State = PerSt_WAIT_TO_EXIT_VEHICLE;
            break;
        }
        person_leave_vehicle(p_person, p_vehicle);
        return 1;
    }
    return 0;
}

void player_change_person(short thing, ushort plyr)
{
    asm volatile ("call ASM_player_change_person\n"
        : : "a" (thing), "d" (plyr));
}

ThingIdx person_find_ferry_to_catch(struct Thing *p_person)
{
    struct Command *p_cmd;
    int radius;
    ThingIdx thing;
    short cor_x, cor_y, cor_z;

    p_cmd = &game_commands[p_person->U.UPerson.ComCur];
    radius = (p_cmd->Arg1 << 6);
    cor_x = PRCCOORD_TO_MAPCOORD(p_person->X);
    cor_y = PRCCOORD_TO_MAPCOORD(p_person->Y);
    cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z);

    thing = search_for_ferry(cor_x, cor_y, cor_z, radius);

    return thing;
}

void person_catch_ferry(struct Thing *p_person)
{
    struct Thing *p_vehicle;
    ThingIdx veh;

    if ((p_person->Flag & TngF_InVehicle) != 0)
        return;
    person_goto_point(p_person);
    p_person->State = PerSt_CATCH_FERRY;
    veh = person_find_ferry_to_catch(p_person);
    if (veh == 0)
        return;

    p_vehicle = &things[veh];
    person_enter_vehicle(p_person, p_vehicle);
    p_person->State = PerSt_NONE;
    p_person->U.UPerson.Vehicle = veh;
}

void alert_peeps(int x, int y, int z, struct Thing *p_madman)
{
    asm volatile (
      "call ASM_alert_peeps\n"
        : : "a" (x), "d" (y), "b" (z), "c" (p_madman));
}

void person_attempt_to_leave_ferry(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_attempt_to_leave_ferry\n"
        : : "a" (p_person));
#endif
    struct Command *p_cmd;
    struct Thing *p_vehicle;
    int radius_sqr;
    short state_bkp;

    p_cmd = &game_commands[p_person->U.UPerson.ComCur];
    radius_sqr = (p_cmd->Arg1 << 6) * (p_cmd->Arg1 << 6);
    p_vehicle = &things[p_person->U.UPerson.Vehicle];
    func_6fd1c(PRCCOORD_TO_MAPCOORD(p_person->X),
      PRCCOORD_TO_MAPCOORD(p_person->Y),
      PRCCOORD_TO_MAPCOORD(p_person->Z),
      p_cmd->X, p_cmd->Y, p_cmd->Z, colour_lookup[ColLU_RED]);
    func_6fd1c(PRCCOORD_TO_MAPCOORD(p_vehicle->X),
      PRCCOORD_TO_MAPCOORD(p_vehicle->Y),
      PRCCOORD_TO_MAPCOORD(p_vehicle->Z),
      p_cmd->X, p_cmd->Y, p_cmd->Z, colour_lookup[ColLU_BLUE]);

    if ((p_vehicle->U.UVehicle.LeisurePlace == 0) || (p_vehicle->Speed != 0)) {
        return;
    }
    if (!thing_arrived_at_obj_radius(p_person->ThingOffset,
      p_cmd->X, p_cmd->Y, p_cmd->Z, radius_sqr)) {
        return;
    }

    state_bkp = p_vehicle->State;
    p_vehicle->State = VehSt_NONE;
    person_leave_vehicle(p_person, p_vehicle);
    p_vehicle->State = state_bkp;

    delete_node(p_person);
    p_person->X = MAPCOORD_TO_PRCCOORD(p_cmd->X,0);
    p_person->Y = alt_at_point(p_cmd->X, p_cmd->Z);
    p_person->Z = MAPCOORD_TO_PRCCOORD(p_cmd->Z,0);
    add_node_thing(p_person->ThingOffset);
    p_person->State = PerSt_NONE;
    p_person->Flag |= TngF_Unkn0004;
}

void thing_shoot_at_point(struct Thing *p_thing, short x, short y, short z, uint fast_flag)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_thing_shoot_at_point\n"
        : : "a" (p_thing), "d" (x), "b" (y), "c" (z), "g" (fast_flag));
    return;
#endif
    PlayerInfo *p_player;
    short src_x, src_y, src_z;
    int weapon_range;
    short face;
    PlayerIdx plyr;
    ushort plagent;
    ushort angle;

    face = 0;
    if ((fast_flag & 0x02) != 0)
    {
        int height;

        face = y;
        if (y <= 0)
            height = get_height_on_face_quad(x << 8, z << 8, -y);
        else
            height = get_height_on_face(x << 8, z << 8, y);
        y = (height >> 8) + 20;
    }

    p_thing->Flag |= TngF_Unkn0800;
    if ((p_thing->Flag & TngF_InVehicle) != 0) {
        struct Thing *p_vehicle;
        p_vehicle = &things[p_thing->U.UPerson.Vehicle];
        p_vehicle->Flag |= TngF_Unkn01000000;
    }

    if ((p_thing->Flag & (TngF_Unkn40000000|TngF_StationrSht|TngF_Destroyed)) != 0)
        return;

    if ((p_thing->Flag2 & TgF2_Unkn0001) != 0)
        finalise_razor_wire(p_thing);

    if (p_thing->State == PerSt_DROP_ITEM || p_thing->State == PerSt_PICKUP_ITEM ||
      p_thing->State == PerSt_DEAD || p_thing->State == PerSt_DIEING)
        return;
    if ((p_thing->Flag & TngF_Destroyed) != 0)
        return;

    angle = angle_between_points(PRCCOORD_TO_MAPCOORD(p_thing->X),
      PRCCOORD_TO_MAPCOORD(p_thing->Z), x, z);
    change_player_angle(p_thing, (((angle + 128) >> 8) + 8) & 7);

    p_thing->PTarget = 0;
    p_thing->Flag |= TngF_Unkn20000000|TngF_Unkn0800;

    plyr = p_thing->U.UPerson.ComCur >> 2;
    plagent = p_thing->U.UPerson.ComCur & 3;
    p_player = &players[plyr];

    if ((p_thing->Flag & TngF_PlayerAgent) != 0)
    {
        p_player->SpecialItems[0] = x;
        p_player->SpecialItems[1] = y;
        p_player->SpecialItems[2] = z;
    }

    src_x = PRCCOORD_TO_MAPCOORD(p_thing->X);
    src_y = PRCCOORD_TO_MAPCOORD(p_thing->Y);
    src_z = PRCCOORD_TO_MAPCOORD(p_thing->Z);
    weapon_range = get_weapon_range(p_thing);
    map_limit_distance_to_target_fast(src_x, src_y, src_z,
      &x, &y, &z, weapon_range);

    if ((p_thing->Flag2 & TgF2_ExistsOffMap) == 0)
        alert_peeps(x, y, z, p_thing);

    if ((p_thing->Flag & TngF_PlayerAgent) != 0)
    {
        p_player->UserVX[plagent] = x;
        p_player->UserVZ[plagent] = z;
        p_player->UserVY[plagent] = y;
    }
    else
    {
        p_thing->VX = x;
        p_thing->VY = y;
        p_thing->VZ = z;
        p_thing->State = PerSt_NONE;
    }

    switch (p_thing->U.UPerson.CurrentWeapon)
    {
    case WEP_RAZORWIRE:
        init_lay_razor(p_thing, x, y, z, 0);
        if ((fast_flag & 0x02) == 0)
        {
            build_navigate_path(p_thing, x, z, 0);
            p_player->GotoFace = 0;
        } else
        {
            build_navigate_path_to_face_xz(p_thing, -face, x, z);
            p_player->GotoFace = face;
        }
        break;
    case WEP_EXPLWIRE:
        init_lay_razor(p_thing, x, y, z, 1);
        if ((fast_flag & 0x02) == 0)
        {
            build_navigate_path(p_thing, x, z, 0);
            p_player->GotoFace = 0;
        } else
        {
            build_navigate_path_to_face_xz(p_thing, -face, x, z);
            p_player->GotoFace = face;
        }
        break;
    }

    if ((fast_flag & 0x01) != 0) {
        set_person_animmode_run(p_thing);
    }
}

void call_protect(struct Thing *p_thing, ushort plyr)
{
    asm volatile ("call ASM_call_protect\n"
        : : "a" (p_thing), "d" (plyr));
}

void call_protect_specific(struct Thing *p_person_client, struct Thing *p_protector)
{
    asm volatile ("call ASM_call_protect_specific\n"
        : : "a" (p_person_client), "d" (p_protector));
}

ushort count_protect(struct Thing *p_thing, ushort plyr)
{
    ushort ret;
    asm volatile (
      "call ASM_count_protect\n"
        : "=r" (ret) : "a" (p_thing), "d" (plyr));
    return ret;
}

void call_unprotect(struct Thing *p_thing, ushort plyr, ubyte flag)
{
    asm volatile ("call ASM_call_unprotect\n"
        : : "a" (p_thing), "d" (plyr), "b" (flag));
}

void thing_shoot_at_thing(struct Thing *p_thing, short target)
{
    asm volatile ("call ASM_thing_shoot_at_thing\n"
        : : "a" (p_thing), "d" (target));
}

void person_init_plant_mine(struct Thing *p_person, short x, short y, short z, int face)
{
    asm volatile (
      "push %4\n"
      "call ASM_person_init_plant_mine\n"
        : : "a" (p_person), "d" (x), "b" (y), "c" (z), "g" (face));
}

ubyte thing_select_specific_weapon(struct Thing *p_person, WeaponType wtype, ubyte flag)
{
#if 0
    int ret;
    asm volatile ("call ASM_thing_select_specific_weapon\n"
        : "=r" (ret) : "a" (p_person), "d" (wtype), "b" (flag));
    return ret;
#endif
    ubyte animode;

    if ((p_person->Flag & TngF_Destroyed) != 0)
        return WepSel_SKIP;
    if (wtype == WEP_AIRSTRIKE && current_map == 65) // map065 The Moon
    {
        play_dist_sample(p_person, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        p_person->U.UPerson.CurrentWeapon = WEP_NULL;
        return WepSel_SKIP;
    }

    if ((p_person->Flag2 & TgF2_Unkn0001) != 0)
        finalise_razor_wire(p_person);
    stop_looped_weapon_sample(p_person, p_person->U.UPerson.CurrentWeapon);

    if (flag == WepSel_SKIP)
        return WepSel_SKIP;
    if (!person_carries_weapon(p_person, wtype))
        return WepSel_SKIP;

#if 0 // Disabled by community consensus - medikit should not be auto-consumed when selected
    if (wtype == WEP_MEDI1)
    {
        if ((p_person->Flag & TngF_PlayerAgent) != 0) {
            PlayerIdx plyr;

            plyr = p_person->U.UPerson.ComCur >> 2;
            if (plyr == local_player_no)
                play_sample_using_heap(0, 2, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        }

        p_person->Health = p_person->U.UPerson.MaxHealth;
        person_weapons_remove_one(p_person, wtype);
        return WepSel_HIDE;
    }
#endif

    if (((p_person->U.UPerson.CurrentWeapon == wtype) || (flag == WepSel_HIDE)) && (flag != WepSel_SELECT))
    {
        if ((p_person->Flag & TngF_PlayerAgent) != 0 &&
          (p_person->Flag2 & TgF2_Unkn0800) == 0 &&
          p_person->U.UPerson.CurrentWeapon != WEP_NULL) {
            player_agent_update_prev_weapon(p_person);
        }
        p_person->U.UPerson.CurrentWeapon = WEP_NULL;

        animode = ANIM_PERS_IDLE;
    }
    else
    {
        p_person->U.UPerson.CurrentWeapon = wtype;

        animode = gun_out_anim(p_person, 0);
    }
    switch_person_anim_mode(p_person, animode);
    p_person->Speed = calc_person_speed(p_person);

    return (p_person->U.UPerson.CurrentWeapon != WEP_NULL) ? WepSel_SELECT : WepSel_HIDE;
}

ubyte thing_select_best_weapon_for_range(struct Thing *p_person, int range)
{
    choose_best_weapon_for_range(p_person, range);
    p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
    reset_person_frame(p_person);
    p_person->Speed = calc_person_speed(p_person);

    return (p_person->U.UPerson.CurrentWeapon != WEP_NULL) ? WepSel_SELECT : WepSel_HIDE;
}

ubyte thing_deselect_weapon(struct Thing *p_person)
{
    player_agent_update_prev_weapon(p_person);
    p_person->U.UPerson.CurrentWeapon = WEP_NULL;
    p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
    reset_person_frame(p_person);
    p_person->Speed = calc_person_speed(p_person);

    return WepSel_HIDE;
}

void person_go_enter_vehicle_fast(struct Thing *p_person, struct Thing *p_vehicle, ushort plyr)
{
    asm volatile (
      "call ASM_person_go_enter_vehicle_fast\n"
        : : "a" (p_person), "d" (p_vehicle), "b" (plyr));
}

void person_go_enter_vehicle(struct Thing *p_person, struct Thing *p_vehicle)
{
    asm volatile (
      "call ASM_person_go_enter_vehicle\n"
        : : "a" (p_person), "d" (p_vehicle));
}

TbBool person_has_supershield_active(ThingIdx person)
{
    struct Thing *p_person;

    if (person <= 0)
        return false;

    p_person = &things[person];

    if (p_person->Type != TT_PERSON)
        return false;

    return ((p_person->Flag & TngF_PersSupShld) != 0);
}

TbBool person_can_toggle_supershield(ThingIdx person)
{
    if (person_is_executing_commands(person))
        return false;
    // Restrict only enabling supershield; for disabling, it can be done almost always
    if (!person_has_supershield_active(person))
    {
        short plyr; // stores PlayerIdx or -1

        if (thing_is_destroyed(person))
            return false;
        plyr = person_get_dcontrol_player(person);
        if (plyr >= 0)
        {
            PlayerInfo *p_player;

            p_player = &players[plyr];
            //TODO not sure why cannot enable supershield in DoubleMode; verify, maybe remove.
            if (p_player->DoubleMode != 0)
                return false;
        }
    }
    return true;
}

void person_supershield_toggle(struct Thing *p_person)
{
    if ((p_person->Flag & TngF_PersSupShld) != 0)
    {
        p_person->Flag &= ~(TngF_Unkn00200000|TngF_PersSupShld);
    }
    else
    {
        short plagent;
        p_person->Flag |= (TngF_Unkn00200000|TngF_PersSupShld);
        plagent = person_slot_in_player_dcontrol(p_person->ThingOffset, local_player_no);
        if (plagent >= 0)
            play_sample_using_heap(0, 96, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
    }
}

TbBool person_can_sustain_thermal(ThingIdx person)
{
    struct Thing *p_person;

    p_person = &things[person];

    if (p_person->U.UPerson.Energy <= 100)
        return false;

    return true;
}

TbBool person_update_thermal(ThingIdx person)
{
    struct Thing *p_person;

    p_person = &things[person];

    p_person->U.UPerson.Energy -= 3;

    if (p_person->U.UPerson.Energy <= 0) {
        return false;
    }

    return true;
}

void make_peeps_scatter(struct Thing *p_person, int x, int z)
{
    asm volatile (
      "call ASM_make_peeps_scatter\n"
        : : "a" (p_person), "d" (x), "b" (z));
}

int limit_mood(struct Thing *p_thing, short mood)
{
    int ret;
    asm volatile ("call ASM_limit_mood\n"
        : "=r" (ret) : "a" (p_thing), "d" (mood));
    return ret;
}

void person_init_plant_mine_fast(struct Thing *p_thing, short x, short y, short z, int face)
{
    asm volatile (
      "push %4\n"
      "call ASM_person_init_plant_mine_fast\n"
        : : "a" (p_thing), "d" (x), "b" (y), "c" (z), "g" (face));
}

void person_self_destruct(struct Thing *p_person)
{
    asm volatile (
      "call ASM_person_self_destruct\n"
        : : "a" (p_person));
}

void person_scare_person(struct Thing *p_person)
{
    struct Thing *p_target;

    person_goto_person_nav(p_person);
    if (p_person->State == PerSt_NONE)
    {
        int range;

        p_person->State = PerSt_SCARE_PERSON;
        range = get_weapon_range(p_person);
        if (can_i_see_thing(p_person, p_person->PTarget, range, 0) > 0)
        {
            p_person->Flag |= TngF_Unkn0800;
            if ((p_person->Flag & TngF_InVehicle) != 0)
            {
                struct Thing *p_vehicle;

                p_vehicle = &things[p_person->U.UPerson.Vehicle];
                p_vehicle->Flag |= TngF_Unkn01000000;
            }
        }
    }
    p_target = p_person->PTarget;
    if (p_target->State == PerSt_DEAD)
    {
        p_person->State = PerSt_NONE;
    }
}

void person_block_person(struct Thing *p_person)
{
    struct Thing *p_target;

    person_goto_person_nav(p_person);
    if (p_person->State == PerSt_NONE)
    {
        int range;

        p_person->State = PerSt_BLOCK_PERSON;
        range = get_weapon_range(p_person);
        if (can_i_see_thing(p_person, p_person->PTarget, range * range, 0) > 0)
        {
            p_person->Flag |= TngF_Unkn0800;
            if ((p_person->Flag & TngF_InVehicle) != 0)
            {
                struct Thing *p_vehicle;

                p_vehicle = &things[p_person->U.UPerson.Vehicle];
                p_vehicle->Flag |= TngF_Unkn01000000;
            }
        }
    }
    p_target = p_person->PTarget;
    if (p_target->State == PerSt_DEAD)
    {
        p_person->State = PerSt_NONE;
    }
}

void process_stationary_shot(struct Thing *p_person)
{
    if (p_person->U.UPerson.WeaponTurn == 0)
    {
        if ((p_person->Flag & TngF_Unkn0800) == 0)
            p_person->Flag &= ~TngF_StationrSht;
    }
}

void process_wait_train(struct Thing *p_person)
{
    if ((p_person->Flag & TngF_Unkn4000) != 0)
    {
        p_person->SubState = 25;
    }
    else if (p_person->SubState == 25)
    {
        p_person->State = PerSt_NONE;
    }
}

void person_follow_person(struct Thing *p_person)
{
    struct Thing *p_target;

    person_goto_person_nav(p_person);
    if (p_person->State == PerSt_NONE)
    {
        p_person->State = PerSt_FOLLOW_PERSON;
    }
    p_target = &things[p_person->GotoThingIndex];
    if ((p_target->Flag & TngF_Destroyed) != 0)
    {
        p_person->State = PerSt_NONE;
    }
}

void person_go_plant_mine(struct Thing *p_person)
{
    person_goto_point(p_person);
    if (p_person->State == PerSt_NONE)
    {
        WeaponType wtype;

        wtype = p_person->U.UPerson.CurrentWeapon;
        if (weapon_is_for_planting(wtype)) {
            person_init_drop(p_person, wtype);
        } else {
            LOGWARN("Weapon %s is not desgined for planting",
              weapon_codename(wtype));
        }
    }
}

void process_knocked_out(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_process_knocked_out\n"
        : : "a" (p_person));
#endif
    ushort nxframe;

    p_person->Timer1 -= fifties_per_gameturn;
    if (p_person->Timer1 < 0)
    {
        nxframe = frame[p_person->Frame].Next;
        if ((frame[nxframe].Flags & 0x01) != 0) {
            p_person->Timer1 = 0;
        } else {
            p_person->Timer1 = p_person->StartTimer1;
            p_person->Frame = nxframe;
        }
    }

    p_person->U.UPerson.BumpCount--;
    if (p_person->U.UPerson.BumpCount == 0)
    {
        p_person->U.UPerson.AnimMode = p_person->U.UPerson.OldAnimMode;
        reset_person_frame(p_person);

        p_person->Flag2 &= ~TgF2_KnockedOut;
        p_person->Timer1 = 48;
        p_person->Flag &= ~TngF_StandOnVehicle;
    }
}

void process_tasered_person(struct Thing *p_person)
{
    asm volatile ("call ASM_process_tasered_person\n"
        : : "a" (p_person));
}

void person_intel(struct Thing *p_person)
{
    asm volatile ("call ASM_person_intel\n"
        : : "a" (p_person));
}

void process_im_shoved(struct Thing *p_person)
{
    asm volatile ("call ASM_process_im_shoved\n"
        : : "a" (p_person));
}

void process_protect_person(struct Thing *p_person)
{
    asm volatile ("call ASM_process_protect_person\n"
        : : "a" (p_person));
}

void process_wander(struct Thing *p_person)
{
    asm volatile ("call ASM_process_wander\n"
        : : "a" (p_person));
}

void person_recoil(struct Thing *p_person)
{
    asm volatile ("call ASM_person_recoil\n"
        : : "a" (p_person));
}

void calc_lighting(struct Thing *p_person)
{
    asm volatile ("call ASM_calc_lighting\n"
        : : "a" (p_person));
}

void process_persuaded(struct Thing *p_person)
{
    asm volatile ("call ASM_process_persuaded\n"
        : : "a" (p_person));
}

void process_danger(struct Thing *p_person)
{
    asm volatile ("call ASM_process_danger\n"
        : : "a" (p_person));
}

void person_kill_target2(struct Thing *p_person)
{
    asm volatile ("call ASM_person_kill_target2\n"
        : : "a" (p_person));
}

void person_wait(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_wait\n"
        : : "a" (p_person));
#endif
    if ((p_person->Flag2 & TgF2_Unkn00080000) != 0)
    {
        p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
        reset_person_frame(p_person);
        p_person->Timer1 = 48;
        p_person->StartTimer1 = 48;
        p_person->Flag2 &= ~TgF2_Unkn00080000;
        p_person->Speed = calc_person_speed(p_person);
    }
    if ((p_person->U.UPerson.AnimMode != ANIM_PERS_Unkn21) && (p_person->U.UPerson.CurrentWeapon == WEP_NULL))
    {
        p_person->U.UPerson.AnimMode = ANIM_PERS_Unkn21;
        reset_person_frame(p_person);
    }
    p_person->Flag &= ~TngF_Unkn0001;
    if (((p_person->Flag & TngF_WepCharging) != 0) || (p_person->U.UPerson.WeaponTurn != 0))
    {
        if (p_person->U.UPerson.AnimMode == ANIM_PERS_Unkn14 || p_person->U.UPerson.AnimMode == ANIM_PERS_WEPHEAVY_Unkn15)
        {
            p_person->Timer1 -= fifties_per_gameturn;
            if (p_person->Timer1 < 0) {
                p_person->Timer1 = p_person->StartTimer1;
                p_person->Frame = frame[p_person->Frame].Next;
            }
        }
    }
    else if (((gameturn + 32 * p_person->ThingOffset) & 0x7F) == 0)
    {
        process_random_speech(p_person, 0);
    }

    if (((p_person->Flag & TngF_PlayerAgent) == 0 || (p_person->Flag2 & TgF2_Unkn0800) != 0)
      && (p_person->U.UPerson.ComHead != 0)
      && conditional_command_state_true(p_person->U.UPerson.ComCur, p_person, 2))
    {
        if (gameturn <= 1) {
            LOGWARN("Person %s %d command %d condition met just at start of the game",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              (int)p_person->U.UPerson.ComCur);
        }
        if ((debug_log_things & 0x01) != 0) {
            char locstr[192];
            snprint_command(locstr, sizeof(locstr), p_person->U.UPerson.ComCur);
            LOGSYNC("Person %s %d %s %d condition met, state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              locstr, (int)p_person->U.UPerson.ComCur, p_person->State, p_person->SubState);
        }
        p_person->State = PerSt_NONE;
    }
    if (p_person->State < 0)
        p_person->Frame = frame[p_person->Frame].Next;
}

void person_pickup(struct Thing *p_person)
{
    asm volatile ("call ASM_person_pickup\n"
        : : "a" (p_person));
}

void person_drop_item(struct Thing *p_person)
{
    asm volatile ("call ASM_person_drop_item\n"
        : : "a" (p_person));
}

void person_dieing(struct Thing *p_person)
{
    asm volatile ("call ASM_person_dieing\n"
        : : "a" (p_person));
}

void person_dead(struct Thing *p_person)
{
    asm volatile ("call ASM_person_dead\n"
        : : "a" (p_person));
}

void person_kill_person(struct Thing *p_person)
{
    asm volatile ("call ASM_person_kill_person\n"
        : : "a" (p_person));
}

void person_save_victim(struct Thing *p_person)
{
    asm volatile ("call ASM_person_save_victim\n"
        : : "a" (p_person));
}

void person_get_item(struct Thing *p_person)
{
    asm volatile ("call ASM_person_get_item\n"
        : : "a" (p_person));
}

void person_run_away(struct Thing *p_person)
{
    asm volatile ("call ASM_person_run_away\n"
        : : "a" (p_person));
}

void person_becomes_persuaded(struct Thing *p_person, ushort energy)
{
    asm volatile ("call ASM_person_becomes_persuaded\n"
        : : "a" (p_person), "d" (energy));
}

void person_persuade_person(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_persuade_person\n"
        : : "a" (p_person));
#endif
    struct Thing *p_target;
    int dist_sq;
    short weapon_range;
    TbBool reached_target;

    if (person_carries_weapon(p_person, WEP_PERSUADER2))
        weapon_range = get_hand_weapon_range(p_person, WEP_PERSUADER2);
    else
        weapon_range = get_hand_weapon_range(p_person, WEP_PERSUADRTRN);

    if (((p_person->Flag & (TngF_Unkn0800|TngF_WepCharging)) != 0) &&
      (p_person->U.UPerson.WeaponTimer > 5)) {
        p_person->Flag &= ~TngF_Unkn0800;
    }

    p_target = &things[p_person->GotoThingIndex];
    p_person->PTarget = p_target;

    if (p_person->GotoThingIndex == 0) {
        p_person->State = PerSt_NONE;
        return;
    }
    if ((p_target->Flag & TngF_Persuaded) != 0) {
        p_person->State = PerSt_NONE;
        return;
    }

    dist_sq = person_goto_person_nav(p_person);

    reached_target = 0;
    if ((dist_sq != 0) && (dist_sq < weapon_range * weapon_range))
        reached_target = 1;

    if (reached_target)
    {
        ushort energy_reqd;
        ubyte target_select;
        ThingIdx target;

        if (person_carries_weapon(p_person, WEP_PERSUADER2))
            target_select = PTargSelect_PersuadeAdv;
        else
            target_select = PTargSelect_Persuader;

        target = process_persuadertron(p_person, target_select, &energy_reqd);
        if (target != 0)
        {
            p_person->PTarget = &things[p_person->GotoThingIndex];
            person_becomes_persuaded(p_person, 0);
            p_person->State = PerSt_NONE;
            return;
        }
        if (p_person->U.UPerson.ComRange > 2)
            p_person->U.UPerson.ComRange -= 2;
        else
            p_person->U.UPerson.ComRange = 2;
    }
    if (p_person->State == 0) {
        p_person->State = PerSt_PERSUADE_PERSON;
        p_person->Flag &= ~TngF_Unkn0800;
    }

    p_target = p_person->PTarget;
    if ((p_target != NULL) && ((p_target->Flag & TngF_Destroyed) != 0)) {
        p_person->State = PerSt_NONE;
        p_person->Flag &= ~TngF_Unkn0800;
    }
}

void process_support_person(struct Thing *p_person)
{
    asm volatile ("call ASM_process_support_person\n"
        : : "a" (p_person));
}

void person_use_vehicle(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_use_vehicle\n"
        : : "a" (p_person));
#endif
    struct Thing *p_vehicle;

    if ((p_person->Flag & TngF_InVehicle) != 0)
    {
        if ((debug_log_things & 0x01) != 0) {
            LOGSYNC("Person %s %d has finished state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              p_person->State, p_person->SubState);
        }
        p_person->State = PerSt_NONE;
        return;
    }

    person_goto_point(p_person);
    if ((p_person->Flag & TngF_StandOnVehicle) == 0) {
        if ((debug_log_things & 0x01) != 0) {
            LOGSYNC("Person %s %d has wrong flags to enter, state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              p_person->State, p_person->SubState);
        }
        return;
    }

    if (p_person->U.UPerson.Vehicle != p_person->GotoThingIndex) {
        if ((debug_log_things & 0x01) != 0) {
            LOGSYNC("Person %s %d went to wrong thing, state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              p_person->State, p_person->SubState);
        }
        return;
    }

    p_vehicle = &things[p_person->U.UPerson.Vehicle];

    if (!can_i_enter_vehicle(p_person, p_vehicle)) {
        if ((debug_log_things & 0x01) != 0) {
            LOGSYNC("Person %s %d state %d.%d cannot enter %s %d state %d.%d",
              person_type_name(p_person->SubType), (int)p_person->ThingOffset,
              p_person->State, p_person->SubState,
              vehicle_type_name(p_vehicle->SubType), (int)p_vehicle->ThingOffset,
              p_vehicle->State, p_vehicle->SubState);
        }
        return;
    }

    person_enter_vehicle(p_person, p_vehicle);
    p_person->State = PerSt_NONE;
}

void person_wait_vehicle(struct Thing *p_person)
{
    asm volatile ("call ASM_person_wait_vehicle\n"
        : : "a" (p_person));
}

void person_destroy_building(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_destroy_building\n"
        : : "a" (p_person));
#endif
    struct Thing *p_target;
    TbBool in_range;

    if (((p_person->Flag & (TngF_Unkn0800|TngF_WepCharging)) != 0) &&
      (p_person->U.UPerson.WeaponTimer > 5))
        p_person->Flag &= ~TngF_Unkn0800;

    {
        int dist_x, dist_z, range;

        dist_x = PRCCOORD_TO_MAPCOORD(p_person->X) - p_person->U.UPerson.GotoX;
        dist_z = PRCCOORD_TO_MAPCOORD(p_person->Z) - p_person->U.UPerson.GotoZ;
        range = p_person->U.UPerson.ComRange << 6;

        in_range = 0;
        if (dist_x * dist_x + dist_z * dist_z < range * range)
            in_range = 1;
    }

    if (!in_range)
        person_goto_point(p_person);

    p_target = p_person->PTarget;
    if (p_target == NULL)
    {
        p_person->State = PerSt_NONE;
        p_person->Flag &= ~TngF_Unkn0800;
        return;
    }

    if (in_range && (p_person->U.UPerson.WeaponTurn == 0) && ((p_target->Flag & TngF_Destroyed) == 0))
    {
        struct Thing *p_building;
        int weapon_range;
        weapon_range = get_weapon_range(p_person);
        p_building = &things[p_person->GotoThingIndex];
        if (can_i_see_building(p_person, p_building, weapon_range * weapon_range, 1) <= 0)
        {
            in_range = 0;
            p_person->Flag &= ~TngF_Unkn0800;
        }
        else
        {
            p_person->Flag &= ~(TngF_Unkn00200000|TngF_Unkn0800|TngF_PersSupShld);
            p_person->Flag |= TngF_Unkn0800;
            if ((p_person->Flag & TngF_InVehicle) != 0) {
                struct Thing *p_vehicle;
                p_vehicle = &things[p_person->U.UPerson.Vehicle];
                p_vehicle->Flag |= TngF_Unkn01000000;
            }
            p_person->U.UPerson.ComRange += 2;
            if (p_person->U.UPerson.ComRange > 5 * 4)
                p_person->U.UPerson.ComRange = 5 * 4;
        }
        if (!in_range)
        {
            if (p_person->U.UPerson.ComRange < 4)
                p_person->U.UPerson.ComRange = 2;
            else
                p_person->U.UPerson.ComRange -= 2;
        }
    }

    if (p_person->State == PerSt_NONE)
    {
        p_person->State = PerSt_DESTROY_BUILDING;
        p_person->Flag &= ~TngF_Unkn0800;
    }
    if ((p_target->Flag & TngF_Destroyed) != 0)
    {
        p_person->State = PerSt_NONE;
        p_person->Flag &= ~TngF_Unkn0800;
    }
}

void person_catch_train(struct Thing *p_person)
{
    asm volatile ("call ASM_person_catch_train\n"
        : : "a" (p_person));
}

void set_angle_to_avoid_group(struct Thing *p_person)
{
#if 1
    asm volatile ("call ASM_set_angle_to_avoid_group\n"
        : : "a" (p_person));
#endif
}

ubyte check_person_within(struct Command *p_cmd, int x, int z)
{
    ubyte ret;
    asm volatile (
      "call ASM_check_person_within\n"
        : "=r" (ret) : "a" (p_cmd), "d" (x), "b" (z));
    return ret;
}

short do_move_colide(struct Thing *p_person, int dx, int dz, struct MyMapElement *p_mapel)
{
    short ret;
    asm volatile (
      "call ASM_do_move_colide\n"
        : "=r" (ret) : "a" (p_person), "d" (dx), "b" (dz), "c" (p_mapel));
    return ret;
}

short person_hit_razor_wire(struct Thing *p_person, ThingIdx thing)
{
    short ret;
    asm volatile (
      "call ASM_person_hit_razor_wire\n"
        : "=r" (ret) : "a" (p_person), "d" (thing));
    return ret;
}

ubyte create_intelligent_door(short col)
{
    ubyte ret;
    asm volatile (
      "call ASM_create_intelligent_door\n"
        : "=r" (ret) : "a" (col));
    return ret;
}

/** Get collision vector which would collide with given thing at given position.
 */
short move_colide_on_tile(struct Thing *p_person, short tl_x, short tl_z, int speed_x, int speed_z)
{
    struct MyMapElement *p_mapel;

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tl_z + tl_x];
    return do_move_colide(p_person, speed_x, speed_z, p_mapel);
}

TbBool move_blocked_by_collision_vect(struct Thing *p_person, short colvect)
{
    short face;

    if (colvect == 0)
        return false;
    if ((p_person->Flag & TngF_Persuaded) != 0)
        return false;
    if (game_col_vects_list[word_1AA38E].Object < 0)
        return false;
    face = game_col_vects[colvect].Face;
    if ((face >= 0) || (game_object_faces4[-face].GFlags & (FGFlg_Unkn10|FGFlg_Unkn04)) != 0)
        return false;
    return true;
}

// overflow flag of subtraction (x-y)
ubyte __OFSUB__(int x, int y)
{
    int y2 = y;
    ubyte sx = (x < 0);
    return (sx ^ (y2 < 0)) & (sx ^ (x-y2 < 0));
}

void adjust_speed_for_colvect_collision(int *p_speed_x, int *p_speed_z, struct Thing *p_person, short colvect)
{
    struct ColVect *p_colvect;
    s64 ldt;
    int dvdr;
    int dt_x, dt_z;
    int dist_sum;
    s64 chk_z, chk_x;
    int sgnhi;
    int norm_z, norm_x;
    int speed_x2, speed_z2;
    sbyte sgn;
    int speed_z1, speed_x1;
    TbBool lower_z, v34;

    p_colvect = &game_col_vects[colvect];

    dt_x = (p_colvect->X2 - p_colvect->X1) << 8;
    dt_z = (p_colvect->Z2 - p_colvect->Z1) << 8;
    dvdr = map_distance_deltas_fast(dt_x, 0, dt_z);
    if (dvdr < 10)
      dvdr = 10;
    ldt = (s64)(p_colvect->X2 - p_colvect->X1) << 24;
    norm_x = ldt / dvdr;
    ldt = (s64)(p_colvect->Z2 - p_colvect->Z1) << 24;
    norm_z = ldt / dvdr;

#if 0 // original implementation
    dist_sum = mul_shift16_sign_pad_lo(*p_speed_x, norm_x) + mul_shift16_sign_pad_lo(*p_speed_z, norm_z);
    speed_x2 = mul_shift16_sign_pad_lo(norm_x, dist_sum);
    speed_z2 = mul_shift16_sign_pad_lo(norm_z, dist_sum);
#else // easier to read version, seem to produce the same results as original implementation? verify!
    dist_sum = (((*p_speed_x) * norm_x) >> 16) + (((*p_speed_z) * norm_z) >> 16);
    speed_x2 = (norm_x * dist_sum) >> 16;
    speed_z2 = (norm_z * dist_sum) >> 16;
#endif

    chk_z = (p_person->Z - (p_colvect->Z1 << 8)) * norm_x;
    chk_x = (p_person->X - (p_colvect->X1 << 8)) * norm_z;

    lower_z = chk_z < chk_x;
    sgn = chk_z != chk_x;
    v34 = __OFSUB__(chk_z >> 32, (chk_x >> 32) + lower_z);
    sgnhi = (chk_z >> 32) - ((chk_x >> 32) + lower_z);
    if (sgnhi)
        sgn = !((sgnhi < 0) ^ v34) - ((sgnhi < 0) ^ v34);

    if (sgn > 0) {
        speed_x1 = -norm_z >> 5;
        speed_z1 = norm_x >> 5;
    } else if (sgn < 0) {
        speed_x1 = norm_z >> 5;
        speed_z1 = -norm_x >> 5;
    } else {
        speed_x1 = 0;
        speed_z1 = 0;
    }

    *p_speed_x = speed_x1 + speed_x2;
    *p_speed_z = speed_z1 + speed_z2;
}

void set_thing_height_on_ground_mesh(struct Thing *p_person, int x, int z)
{
    struct MyMapElement *p_mapel;
    int alt_cc, alt_cr, alt_uc, alt_ur;
    short tile_x, tile_z;
    short subcor_x, subcor_z;
    short new_alt;

    tile_x = MAPCOORD_TO_TILE(PRCCOORD_TO_MAPCOORD(x));
    tile_z = MAPCOORD_TO_TILE(PRCCOORD_TO_MAPCOORD(z));

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + 0) + (tile_x + 1)];
    alt_cr = p_mapel->Alt;
    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + 0) + (tile_x + 0)];
    alt_cc = p_mapel->Alt;
    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + 1) + (tile_x + 0)];
    alt_uc = p_mapel->Alt;
    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + 1) + (tile_x + 1)];
    alt_ur = p_mapel->Alt;

    subcor_x = PRCCOORD_TO_MAPCOORD(x) & 0xFF;
    subcor_z = PRCCOORD_TO_MAPCOORD(z) & 0xFF;
    if (subcor_x + subcor_z >= 256)
        new_alt = alt_ur + (((alt_uc - alt_ur) * (256 - subcor_x)) >> 8) + (((256 - subcor_z) * (alt_cr - alt_ur)) >> 8);
    else
        new_alt = alt_cc + (((alt_cr - alt_cc) * subcor_x) >> 8) + (((alt_uc - alt_cc) * subcor_z) >> 8);
    move_mapwho(p_person, x, MAPCOORD_TO_PRCCOORD(new_alt, 0), z);
}

short person_move(struct Thing *p_person)
{
#if 0
    short ret;
    asm volatile ("call ASM_person_move\n"
        : "=r" (ret) : "a" (p_person));
    return ret;
#endif
    int speed_x, speed_z;
    int x, y, z;
    short colvect;
    short retry;

    word_1AA390 = 0;
    if (((gameturn + 32 * p_person->ThingOffset) & 0x7F) == 0) {
        process_random_speech(p_person, 0);
    }
    if ((p_person->SubType == SubTT_PERS_MECH_SPIDER) && !IsSamplePlaying(p_person->ThingOffset, 79, 0)) {
        play_dist_sample(p_person, 79, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
    }
    if (p_person->U.UPerson.AnimMode == ANIM_PERS_Unkn21) {
        p_person->U.UPerson.AnimMode = ANIM_PERS_IDLE;
        reset_person_frame(p_person);
    }
    if ((p_person->Flag & (TngF_InVehicle|TngF_Unkn4000)) != 0) {
        return 1;
    }

    {
        short used_stamina;
        used_stamina = abs(p_person->U.UPerson.Mood >> 5) - cybmod_chest_level(&p_person->U.UPerson.UMod);
        if (used_stamina < 0)
            used_stamina = 0;
        p_person->U.UPerson.Stamina -= used_stamina;
    }

    if ((p_person->Flag2 & TgF2_Unkn00080000) != 0)
    {
        if ((p_person->Flag & TngF_PlayerAgent) != 0) {
            p_person->U.UPerson.Stamina = p_person->U.UPerson.Stamina - (7 - 2 * cybmod_legs_level(&p_person->U.UPerson.UMod));
        }
        if ((p_person->U.UPerson.Stamina < p_person->U.UPerson.MaxStamina >> 2) && (p_person->State != PerSt_PERSON_BURNING)) {
            p_person->U.UPerson.AnimMode = gun_out_anim(p_person, 0);
            reset_person_frame(p_person);
            p_person->Timer1 = 48;
            p_person->StartTimer1 = 48;
            p_person->Flag2 &= ~TgF2_Unkn00080000;
            p_person->Speed = calc_person_speed(p_person);
        }
    }
    speed_x = (p_person->Speed * p_person->VX) >> 4;
    speed_z = (p_person->Speed * p_person->VZ) >> 4;
    p_person->Flag2 &= ~TgF2_Unkn0100;
    y = p_person->Y;

    retry = 2;
    if (p_person->State == PerSt_WANDER)
        retry = 0;
    if (p_person->U.UPerson.OnFace != 0)
        retry = 0;

    for (; 1; retry--)
    {
        int tile_x, tile_z;
        int tile_dt_x, tile_dt_z;
        ThingIdx thing;
        TbBool bumped_colvect;

        x = p_person->X + speed_x;
        z = p_person->Z + speed_z;

        if ((x < 0) || (PRCCOORD_TO_MAPCOORD(x) >= MAP_COORD_WIDTH))
            return 1;
        if ((z < 0) || (PRCCOORD_TO_MAPCOORD(z) >= MAP_COORD_HEIGHT))
            return 1;

        if ((p_person->Flag & (TngF_Unkn0010|TngF_Unkn0004)) != 0)
        {
            thing = check_for_other_people(x, y, z, p_person);
            if (thing != 0) {
                p_person->U.UPerson.BumpCount++;
                if (((p_person->Flag & TngF_Unkn0010) == 0) && ((p_person->Flag & TngF_StandOnVehicle) != 0))
                    return 1;
            } else {
                p_person->Flag &= ~(TngF_Unkn0004|TngF_Unkn0010);
            }
        }
        else
        {
            thing = check_for_other_people(x, y, z, p_person);
            if (thing != 0) {
                p_person->U.UPerson.BumpCount++;
                return thing;
            }
        }

        if (p_person->U.UPerson.Within != 0)
        {
            struct Command *p_cmd;

            p_cmd = &game_commands[p_person->U.UPerson.Within];
            if (!check_person_within(p_cmd, PRCCOORD_TO_MAPCOORD(speed_x + x),
              PRCCOORD_TO_MAPCOORD(speed_z + z))) {
                p_person->Flag2 |= TgF2_Unkn0080;
                return 1;
            }
            p_person->Flag2 &= ~TgF2_Unkn0080;
        }

        tile_x = MAPCOORD_TO_TILE(PRCCOORD_TO_MAPCOORD(p_person->X));
        tile_dt_x = MAPCOORD_TO_TILE(PRCCOORD_TO_MAPCOORD(x)) - tile_x;
        tile_z = MAPCOORD_TO_TILE(PRCCOORD_TO_MAPCOORD(p_person->Z));
        tile_dt_z = MAPCOORD_TO_TILE(PRCCOORD_TO_MAPCOORD(z)) - tile_z;

        word_1AA394 = 0;
        word_1AA392 = 0;

        bumped_colvect = false;
        {
            colvect = move_colide_on_tile(p_person, tile_x, tile_z, speed_x, speed_z);
            if ((retry != 0) && move_blocked_by_collision_vect(p_person, colvect))
                bumped_colvect = true;
        }
        if (!bumped_colvect)
        {
            if ((tile_dt_z != 0) && (colvect == 0))
            {
                struct MyMapElement *p_mapel;
                p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + tile_dt_z) + (tile_x)];
                if ((p_person->U.UPerson.OnFace == 0) && ((p_mapel->Flags2 & (MEF2_Unkn04|MEF2_Unkn01)) != 0))
                {
                    if (!retry || ((p_person->Flag & TngF_Persuaded) != 0)) {
                        p_person->U.UPerson.BumpCount++;
                        return 1;
                    }
                    speed_z = 0;
                    continue;
                }
                colvect = move_colide_on_tile(p_person, tile_x, tile_z + tile_dt_z, speed_x, speed_z);
                if ((retry != 0) && move_blocked_by_collision_vect(p_person, colvect))
                    bumped_colvect = true;
            }
            if (!bumped_colvect && (tile_dt_x != 0) && (colvect == 0))
            {
                struct MyMapElement *p_mapel;
                p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x + tile_dt_x)];
                if ((p_person->U.UPerson.OnFace == 0) && ((p_mapel->Flags2 & (MEF2_Unkn04|MEF2_Unkn01)) != 0))
                {
                    if ((retry == 0) || ((p_person->Flag & TngF_Persuaded) != 0))
                    {
                        p_person->U.UPerson.BumpCount++;
                        return 1;
                    }
                    speed_x = 0;
                    continue;
                }
                colvect = move_colide_on_tile(p_person, tile_x + tile_dt_x, tile_z, speed_x, speed_z);
                if ((retry != 0) && move_blocked_by_collision_vect(p_person, colvect))
                    bumped_colvect = true;
            }
            if (!bumped_colvect && (tile_dt_z != 0) && (tile_dt_x != 0) && (colvect == 0))
            {
                struct MyMapElement *p_mapel;
                p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + tile_dt_z) + (tile_x + tile_dt_x)];
                if ((p_person->U.UPerson.OnFace == 0) && (p_mapel->Flags2 & (MEF2_Unkn04|MEF2_Unkn01)) != 0)
                {
                    p_person->U.UPerson.BumpCount++;
                    return 1;
                }
                colvect = move_colide_on_tile(p_person, tile_x + tile_dt_x, tile_z + tile_dt_z, speed_x, speed_z);
                if ((retry != 0) && move_blocked_by_collision_vect(p_person, colvect))
                    bumped_colvect = true;
            }
            if (!bumped_colvect)
                break;
        }

        adjust_speed_for_colvect_collision(&speed_x, &speed_z, p_person, colvect);
    }

    if (debug_hud_collision || byte_1C844F) {
        draw_line_transformed_at_ground(PRCCOORD_TO_MAPCOORD(p_person->X), PRCCOORD_TO_MAPCOORD(p_person->Z),
          PRCCOORD_TO_MAPCOORD(p_person->X) + PRCCOORD_TO_MAPCOORD(2 * speed_x),
          PRCCOORD_TO_MAPCOORD(p_person->Z) + PRCCOORD_TO_MAPCOORD(2 * speed_z),
          0xE6u);
    }

    if (colvect < 0)
        return 1;
    if (colvect != 0)
    {
        struct ColVectList *p_cvlist;

        p_cvlist = &game_col_vects_list[word_1AA38E];
        if (p_cvlist->Object < 0)
        {
            x = p_person->X + (speed_x >> 3);
            z = p_person->Z + (speed_z >> 3);
            if (!person_is_dead_or_dying(p_person->ThingOffset) &&
              ((p_person->Flag & TngF_Destroyed) == 0) &&
              person_hit_razor_wire(p_person, -p_cvlist->Object)) {
                return 0;
            }
            colvect = 0;
        }
    }
    if (word_1AA392)
    {
        p_person->Flag2 |= TgF2_Unkn40000000;
        if (create_intelligent_door(word_1AA392) != 1) {
            p_person->U.UPerson.LastDist = 32000;
            return 1;
        }
        p_person->U.UPerson.LastDist = 32000;
        colvect = 0;
        if (word_1AA394 != 0)
        {
            if (word_1AA394 <= 0)
                p_person->Flag2 |= TgF2_InsideBuilding;
            else
                p_person->Flag2 &= ~TgF2_InsideBuilding;
        }
    }

    if (p_person->U.UPerson.OnFace != 0)
    {
        if (set_thing_height_on_face(p_person, x, z, p_person->U.UPerson.OnFace)) {
            move_mapwho(p_person, x, p_person->Y, z);
            p_person->U.UPerson.BumpMode = 0;
            p_person->Flag2 |= TgF2_Unkn0100;
            return 0;
        }
        if (!find_and_set_connected_face(p_person, x, z, p_person->U.UPerson.OnFace))
        {
            if ((colvect == 0) || (p_person->U.UPerson.OnFace != game_col_vects[colvect].Face))
                return 1;
            p_person->U.UPerson.OnFace = 0;
            p_person->U.UPerson.Flag3 &= ~PrsF3_Unkn80;
        }
        move_mapwho(p_person, x, p_person->Y, z);
        p_person->U.UPerson.BumpMode = 0;
        p_person->Flag2 |= TgF2_Unkn0100;
        return 0;
    }

    if (colvect != 0)
    {
        short face;

        face = game_col_vects[colvect].Face;
        if (face == 0)
        {
            // No action
        }
        else if (!face_is_blocking_walk(face))
        {
            if (set_thing_height_on_face(p_person, x, z, face)) {
                p_person->U.UPerson.BumpMode = 0;
                p_person->U.UPerson.OnFace = face;
                p_person->Flag2 |= TgF2_Unkn0100;
                move_mapwho(p_person, x, p_person->Y, z);
                return 0;
            }
            colvect = 0;
        }
        else if (p_person->U.UPerson.OnFace != 0)
        {
            colvect = 0;
        }
    }

    if ((colvect == 0) && (p_person->U.UPerson.OnFace == 0))
    {
        set_thing_height_on_ground_mesh(p_person, x, z);
    }
    if (colvect != 0)
        return 1;
    p_person->U.UPerson.BumpMode = 0;
    p_person->Flag2 |= TgF2_Unkn0100;
    return 0;
}

void process_avoid_group(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_process_avoid_group\n"
        : : "a" (p_person));
#endif
    if ((p_person->Flag & TngF_Destroyed) != 0)
        return;
    if (p_person->U.UPerson.RecoilTimer != 0) {
        return;
    }

    p_person->VX = angle_direction[p_person->U.UPerson.Angle].DiX;
    p_person->VZ = angle_direction[p_person->U.UPerson.Angle].DiY;

    if (person_move(p_person))
    {
        ubyte oangle;
        sbyte change;
        // Select changed, but not to opposite direction - only one of: -2,-1,1,2
        change = (LbRandomAnyShort() & 3) - 2;
        if (change >= 0) change++;
        oangle = (p_person->U.UPerson.Angle + change) & 7;
        change_player_angle(p_person, oangle);
    }

    p_person->Timer1 -= fifties_per_gameturn;
    if (p_person->Timer1 < 0)
    {
        p_person->Timer1 = p_person->StartTimer1;
        p_person->Frame = frame[p_person->Frame].Next;
    }

    p_person->U.UPerson.Timer2--;
    if (p_person->U.UPerson.Timer2 < 0)
    {
        ushort rnd;
        rnd = LbRandomAnyShort();
        p_person->U.UPerson.Timer2 = p_person->U.UPerson.StartTimer2 + ((rnd & 0xF) >> 1);
        set_angle_to_avoid_group(p_person);
    }
}

void person_burning(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_person_burning\n"
        : : "a" (p_person));
    return;
#endif
    p_person->U.UPerson.Brightness = 32;
    apply_super_quick_light(p_person->X >> 8, p_person->Z >> 8, 16 + (LbRandomAnyShort() & 0xF));

    if (((gameturn + p_person->ThingOffset) & 0xF) == 0)
    {
        ushort smpl_no;

        switch (person_sex(p_person))
        {
        case PERSON_FEMALE:
            smpl_no = 2;
            break;
        case PERSON_MALE:
        default:
            smpl_no = 1;
            break;
        }
        play_dist_speech(p_person, smpl_no, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
    }

    if (p_person->U.UPerson.RecoilTimer > 95)
        p_person->U.UPerson.RecoilTimer = 94;
    if ((p_person->Flag & TngF_PlayerAgent) != 0)
        p_person->U.UPerson.RecoilTimer--;
    else
        p_person->U.UPerson.RecoilTimer -= byte_154F6C[LbRandomAnyShort() & 3];

    if (p_person->U.UPerson.RecoilTimer != 0)
    {
        process_weapon_wind_down(p_person);

        p_person->VX = angle_direction[p_person->U.UPerson.Angle].DiX;
        p_person->VZ = angle_direction[p_person->U.UPerson.Angle].DiY;

        if (person_move(p_person))
            p_person->U.UPerson.Timer2 = -1;

        if ((p_person->Flag & TngF_Destroyed) == 0)
        {
            p_person->Timer1 -= fifties_per_gameturn;
            if (p_person->Timer1 < 0)
            {
                p_person->Timer1 = p_person->StartTimer1;
                p_person->Frame = frame[p_person->Frame].Next;
            }

            p_person->U.UPerson.Timer2--;
            if (p_person->U.UPerson.Timer2 < 0)
            {
                ushort angle;

                p_person->U.UPerson.Timer2 = (LbRandomAnyShort() & 7) + p_person->U.UPerson.StartTimer2;
                angle = p_person->U.UPerson.Angle;
                if ((p_person->U.UPerson.Timer2 & 1) != 0)
                    angle++;
                else
                    angle--;
                angle = (angle + 8) & 7;
                change_player_angle(p_person,angle);
            }
        }
    }
    else
    {
        stop_sample_using_heap(p_person->ThingOffset, 29);
        set_person_dead(p_person, ANIM_PERS_DEAD_ASH);
    }
}

void person_being_persuaded(struct Thing *p_person)
{
    asm volatile ("call ASM_person_being_persuaded\n"
        : : "a" (p_person));
}

void process_wander_and_fly(struct Thing *p_person, struct Thing *p_vehicle)
{
    asm volatile ("call ASM_process_wander_and_fly\n"
        : : "a" (p_person), "d" (p_vehicle));
}

void process_lighting_unkn1(struct Thing *p_person)
{
    asm volatile ("call ASM_process_lighting_unkn1\n"
        : : "a" (p_person));
}

void person_find_next_state(struct Thing *p_person)
{
    short cmd;

    if (((p_person->Flag & TngF_PlayerAgent) != 0) && ((p_person->Flag2 & TgF2_Unkn0800) == 0))
    {
        person_init_wait(p_person, 50);
        return;
    }

    if (p_person->U.UPerson.ComCur == 0)
    {
        person_finish_executing_commands(p_person);
        person_init_wait(p_person, 50);
        return;
    }

    if ((p_person->Flag & TngF_Unkn0040) != 0)
    {
        p_person->Flag &= ~TngF_Unkn0040;
        person_init_command(p_person, PCmd_GET_ITEM);
        return;
    }

    if (!is_command_completed(p_person))
        return;

    if (p_person->U.UPerson.ComHead == 0)
    {
        person_init_wait(p_person, 50);
        return;
    }

    cmd = p_person->U.UPerson.ComCur;
    cmd = game_commands[cmd].Next;
    p_person->U.UPerson.ComCur = cmd;

    // Repeat with new ComCur
    if (p_person->U.UPerson.ComCur == 0)
    {
        person_finish_executing_commands(p_person);
        person_init_wait(p_person, 100);
        return;
    }

    person_init_command(p_person, PCmd_USE_WEAPON);
}

void make_peep_protect_peep(struct Thing *p_protector, struct Thing *p_leader)
{
    asm volatile ("call ASM_make_peep_protect_peep\n"
        : : "a" (p_protector), "d" (p_leader));
}

void people_intel(ubyte flag)
{
    asm volatile ("call ASM_people_intel\n"
        : : "a" (flag));
}

void process_person(struct Thing *p_person)
{
#if 0
    asm volatile ("call ASM_process_person\n"
        : : "a" (p_person));
    return;
#endif
    struct MyMapElement *p_mapel;
    short state;

    if ( (p_person->Flag & TngF_PlayerAgent) != 0 && (p_person->Flag2 & TgF2_Unkn10000000) != 0 )
    {
        state = p_person->State;
        if ((state != PerSt_GET_ITEM) && (state != PerSt_DROP_ITEM) && (state != PerSt_PICKUP_ITEM)
          && ((p_person->Flag & (TngF_Unkn40000000|TngF_Destroyed)) == 0)
          && ((p_person->Flag2 & (TgF2_KnockedOut|TgF2_Unkn0008)) == 0))
        {
            struct Thing *p_target;

            p_person->GotoThingIndex = p_person->Owner;
            p_target = &things[p_person->GotoThingIndex];
            p_person->State = PerSt_PROTECT_PERSON;
            p_person->U.UPerson.ComRange = follow_dist[p_target->U.UPerson.ComCur & 3][p_person->U.UPerson.ComCur & 3];
            p_person->Flag2 &= ~TgF2_Unkn10000000;
        }
    }
    p_person->Flag2 &= ~TgF2_Unkn8000;
    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (p_person->Z >> 16) + (p_person->X >> 16)];
    if (((p_person->Flag2 & TgF2_InsideBuilding) != 0) && (p_mapel->ColumnHead == 0))
        p_person->Flag2 &= ~TgF2_InsideBuilding;

    if ( ((gameturn + p_person->ThingOffset) & 0x7F) == 0)
    {
        p_person->U.UPerson.Flag3 &= ~PrsF3_Unkn20;
        if ((p_person->Flag2 & TgF2_Unkn00020000) != 0)
        {
            p_person->U.UPerson.ComTimer = -1;
            p_person->Flag2 &= ~TgF2_Unkn00020000;
            remove_path(p_person);
        }
    }
    if (((p_person->Flag2 & TgF2_Unkn08000000) != 0) && ((p_person->Flag & TngF_PlayerAgent) != 0))
    {
        state = p_person->State;
        if ((state != PerSt_GET_ITEM) && (state != PerSt_PICKUP_ITEM))
        {
            short pstng_x, pstng_y, pstng_z;
            make_peep_protect_peep(p_person, &things[p_person->Owner]);
            get_thing_position_mapcoords(&pstng_x, &pstng_y, &pstng_z, p_person->ThingOffset);
            draw_map_flat_circle(pstng_x, pstng_y, pstng_z, 200, colour_lookup[ColLU_WHITE]);
        }
    }
    if ((p_person->Flag2 & TgF2_Unkn0020) != 0)
    {
        return;
    }
    if ((p_person->Flag2 & TgF2_Unkn00100000) != 0)
    {
        process_tasered_person(p_person);
        return;
    }
    if ((p_person->Flag2 & TgF2_KnockedOut) != 0)
    {
        process_knocked_out(p_person);
        stop_looped_weapon_sample(p_person, p_person->U.UPerson.CurrentWeapon);
        return;
    }
    if ((p_person->Flag2 & TgF2_Unkn0008) != 0)
    {
        p_person->U.UPerson.BumpCount--;
        if (p_person->U.UPerson.BumpCount != 0)
        {
            if (((gameturn + p_person->ThingOffset) & 7) == 0)
                person_intel(p_person);
        }
        else
        {
            ushort group;

            group = p_person->U.UPerson.EffectiveGroup;
            p_person->Flag2 &= ~TgF2_Unkn0008;
            if (group >= 128)
              p_person->U.UPerson.EffectiveGroup = group + 128;
            p_person->U.UPerson.Target2 = 0;
            p_person->PTarget = NULL;
            p_person->State = PerSt_NONE;
            p_person->Flag |= TngF_Unkn0040;
            calc_person_speed(p_person);
        }
    }
    else
    {
        ushort limit;
        limit = 8;
        if ((gameturn & 0x7FF) == 0)
            p_person->U.UPerson.BumpCount = 0;
        if ((p_person->Flag & TngF_PlayerAgent) != 0)
            limit = 2;
        if (p_person->U.UPerson.BumpCount > limit)
        {
            p_person->U.UPerson.BumpCount = 0;
            p_person->Flag &= ~TngF_Unkn00020000;
            if ((p_person->Flag & TngF_Persuaded) == 0)
                p_person->Flag |= TngF_Unkn0004;
            remove_path(p_person);
            p_person->U.UPerson.ComTimer = -1;
        }
    }
    if ((gameturn & 3) == 0)
    {
        state = p_person->State;
        if ((state != PerSt_WAIT) && (state != 0))
        {
            p_person->Flag &= ~TngF_Unkn08000000;
            p_person->Flag &= ~TngF_StandOnVehicle;
        }
    }
    if ((p_person->U.UPerson.BumpMode != 0) && ((p_person->Flag & TngF_Destroyed) == 0))
    {
        if ((p_person->Flag2 & TgF2_Unkn0100) != 0)
            p_person->U.UPerson.BumpMode = 0;
        else
            process_im_shoved(p_person);
    }
    if (((p_person->Flag & TngF_Unkn40000000) == 0) || (((p_person->Flag & TngF_Destroyed) == 0)
        && ((p_person->Flag & (TngF_Persuaded|TngF_Unkn00040000|TngF_WepRecoil|TngF_Unkn4000|TngF_StationrSht)) != 0)))
    {

        if ((p_person->Flag & TngF_Unkn4000) != 0)
          return;
        if ((p_person->Flag & TngF_WepRecoil) != 0)
        {
          stop_looped_weapon_sample(p_person, p_person->U.UPerson.CurrentWeapon);
          person_recoil(p_person);
          calc_lighting(p_person);
          process_weapon(p_person);
          return;
        }
        if (((p_person->Flag & TngF_Persuaded) != 0) && ((p_person->Flag & TngF_Destroyed) == 0))
        {
          process_persuaded(p_person);
          calc_lighting(p_person);
          process_weapon(p_person);
          return;
        }
        if (((p_person->Flag & TngF_Unkn00040000) != 0) && ((p_person->Flag & (TngF_Unkn1000|TngF_Destroyed)) == 0))
        {
          person_run_away(p_person);
          calc_lighting(p_person);
          return;
        }

        if ((p_person->Flag & TngF_StationrSht) != 0)
        {
          if ((p_person->U.UPerson.WeaponTurn == 0) && ((p_person->Flag & TngF_Unkn0800) == 0))
          {
              p_person->Flag &= ~TngF_StationrSht;
          }
          process_weapon(p_person);
          return;
        }
    }

    if (((p_person->Flag & (TngF_InVehicle|TngF_Unkn40000000|TngF_Persuaded|TngF_Unkn4000)) == 0)
      && ((p_person->Flag2 & TgF2_Unkn0008) == 0)
      && ((gameturn + p_person->ThingOffset) & 3) == 0
      && (p_person->Flag & TngF_Destroyed) == 0)
    {
        ushort subType;

        subType = p_person->SubType;
        if (subType == SubTT_PERS_BRIEFCASE_M
          || subType == SubTT_PERS_WHITE_BRUN_F
          || subType == SubTT_PERS_WHIT_BLOND_F
          || subType == SubTT_PERS_LETH_JACKT_M
          || subType == SubTT_PERS_SCIENTIST)
        {
            process_danger(p_person);
        }
    }

    if (((p_person->Flag & (TngF_Unkn40000000|TngF_PlayerAgent)) == 0 || (p_person->Flag2 & TgF2_Unkn0800) != 0)
      && ((gameturn + p_person->ThingOffset) & 7) == 0)
    {
        if (p_person->U.UPerson.ComHead != 0)
        {
          struct Command *p_cmd;
          ushort cmd;

          cmd = p_person->U.UPerson.ComCur;
          p_cmd = &game_commands[cmd];
          if (((p_cmd->Flags & PCmdF_RunUntil) != 0) && ((p_person->Flag & TngF_Destroyed) == 0))
          {
              cmd = person_command_until_check_condition(p_person, cmd);
              if (cmd != CMD_CONDITION_CONTINUE)
              {
                  p_person->U.UPerson.ComCur = cmd;
                  p_person->State = PerSt_NONE;
                  person_init_command(p_person, PCmd_PROTECT_PERSON);
              }
          }
        }
    }

    if ((p_person->Flag & (TngF_Unkn40000000|TngF_Destroyed)) != 0)
        p_person->Flag &= ~TngF_Unkn0800;
    else
        process_weapon(p_person);

    if ((debug_hud_collision == 1) && (p_person->Y != 0))
    {
        char locstr[150];
        short pstng_x, pstng_y, pstng_z;

        sprintf(locstr, "%ld", (long)p_person->Y);
        get_thing_position_mapcoords(&pstng_x, &pstng_y, &pstng_z, p_person->ThingOffset);
        draw_text_transformed(pstng_x, pstng_y, pstng_z, locstr);
    }

    if ((p_person->U.UPerson.Target2 != 0) && ((p_person->Flag & (TngF_Unkn40000000|TngF_Destroyed)) == 0))
    {
        person_kill_target2(p_person);
        if (p_person->State == PerSt_PROTECT_PERSON)
          process_protect_person(p_person);
    }
    else if ((p_person->Flag2 & TgF2_Unkn0008) != 0)
    {
        process_wander(p_person);
        if (p_person->U.UPerson.Timer2 == 1)
            p_person->Speed = 500 + (LbRandomAnyShort() % 700);
    }
    else
    {
        switch (p_person->State & 0x7FFF)
        {
        case PerSt_NONE:
              person_find_next_state(p_person);
              break;
        case PerSt_GOTO_POINT:
              person_goto_point(p_person);
              if ((p_person->State == 0) && ((p_person->U.UPerson.Flag3 & PrsF3_Unkn04) != 0) && ((p_person->Flag & TngF_Unkn1000) == 0))
              {
                  struct Thing *p_target;

                  p_target = &things[p_person->GotoThingIndex];
                  p_person->State = PerSt_PROTECT_PERSON;
                  p_person->U.UPerson.ComRange = follow_dist[p_target->U.UPerson.ComCur & 3][p_person->U.UPerson.ComCur & 3];
              }
              break;
        case PerSt_WANDER:
              process_wander(p_person);
              break;
        case PerSt_WAIT:
              person_wait(p_person);
              break;
        case PerSt_PICKUP_ITEM:
              person_pickup(p_person);
              break;
        case PerSt_DROP_ITEM:
              person_drop_item(p_person);
              break;
        case PerSt_DIEING:
              person_dieing(p_person);
              break;
        case PerSt_DEAD:
              person_dead(p_person);
              break;
        case PerSt_INIT_SAVE_VICTIM:
        case PerSt_SAVE_VICTIM:
              person_save_victim(p_person);
              break;
        case PerSt_GOTO_PERSON:
              person_goto_person_nav(p_person);
              break;
        case PerSt_KILL_PERSON:
              person_kill_person(p_person);
              break;
        case PerSt_BLOCK_PERSON:
              person_block_person(p_person);
              break;
        case PerSt_SCARE_PERSON:
              person_scare_person(p_person);
              break;
        case PerSt_STATIONARY_SHOT:
              process_stationary_shot(p_person);
              break;
        case PerSt_RECOIL_HIT:
              person_recoil(p_person);
              break;
        case PerSt_GOTO_POINT_REL:
              person_goto_point_rel(p_person);
              break;
        case PerSt_PERSON_BURNING:
              person_burning(p_person);
              break;
        case PerSt_PERSUADE_PERSON:
              person_persuade_person(p_person);
              break;
        case PerSt_FOLLOW_PERSON:
              person_follow_person(p_person);
              break;
        case PerSt_SUPPORT_PERSON:
              process_support_person(p_person);
              break;
        case PerSt_PROTECT_PERSON:
              process_protect_person(p_person);
              break;
        case PerSt_GET_ITEM:
              person_get_item(p_person);
              break;
        case PerSt_USE_VEHICLE:
              person_use_vehicle(p_person);
              break;
        case PerSt_WAIT_VEHICLE:
              person_wait_vehicle(p_person);
              break;
        case PerSt_CATCH_TRAIN:
              person_catch_train(p_person);
              break;
        case PerSt_WAIT_TRAIN:
              process_wait_train(p_person);
              break;
        case PerSt_DESTROY_BUILDING:
              person_destroy_building(p_person);
              break;
        case PerSt_GO_PLANT_MINE:
              person_go_plant_mine(p_person);
              break;
        case PerSt_WAIT_TO_EXIT_VEHICLE:
              person_attempt_to_leave_vehicle(p_person);
              break;
        case PerSt_CATCH_FERRY:
              person_catch_ferry(p_person);
              break;
        case PerSt_EXIT_FERRY:
              person_attempt_to_leave_ferry(p_person);
              break;
        case PerSt_AVOID_GROUP:
              process_avoid_group(p_person);
              break;
        case PerSt_INIT_COMMAND:
              person_init_command(p_person, PCmd_DROP_SPEC_ITEM);
              break;
        case PerSt_BEING_PERSUADED:
              person_being_persuaded(p_person);
              break;
        default:
              break;
        }
        calc_lighting(p_person);
        process_lighting_unkn1(p_person);
    }
}

/******************************************************************************/
