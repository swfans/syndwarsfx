/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file guitext.h
 *     Header file for guitext.c.
 * @par Purpose:
 *     GUI text for menu and in-game display.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 22 Oct 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GUITEXT_H
#define GUITEXT_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define STRINGS_MAX 2000

#pragma pack(1)

enum GuiStrings {
    
    GSTR_PLAYERS = 459,
    GSTR_PLAYER = 460,
    GSTR_JOYSTICK = 463,
    GSTR_CONTROLS = 486,
    GSTR_KEYS = 487,
    GSTR_JOYSTICK2 = 488,
    GSTR_JOYSTICK_TYPE = 489,
    // Names for network protocols
    GSTR_NET_PROTO_NAMES = 497,
    GSTR_JOY_CAL_TOP_LEFT = 574,
    GSTR_JOY_CAL_BOTTOM_RIGHT = 575,
    GSTR_JOY_CAL_LEAVE_CENTRE = 576,
    GSTR_CONTROLS_SAVED_SUCC = 577,
    GSTR_CONTROLS_SAVED_FAIL = 578,
    // Extra names for network protocols if modem is in use
    GSTR_NET_PROTO_MODEM_NAMES = 619,
    // Name for mission metric
    GSTR_MTR_SCIENTISTS_DEAD = 537,
    // Names for mission status values
    GSTR_ENM_MISSION_STATUS = 634,
    // Chunk of a sentence to put before mission status
    GSTR_CHK_MISSION_STA_PRE = 638,
    // Chunk of a sentence to put after mission status to tell about exit keys
    GSTR_CHK_MISSION_STA_SUF_KEYS = 639,
    // Names for mods, per type
    GSTR_MOD_EPIDERMIS = 645,
    // Group: Names of control actions assignable to keys - 800..849
    GSTR_SYS_GAME_KEYS = 799,
    // Group: Mission metrics - 900..949
    // Metrics on mission debrief screen
    GSTR_MTR_MISSION_REF = 901,
    GSTR_MTR_MISSION_STATUS = 902,
    GSTR_MTR_OBJCTV_N_COMPL = 903,
    GSTR_MTR_CITY_NAME = 904,
    GSTR_MTR_MISS_TIME_ELP = 905,
    GSTR_MTR_CREDS_NET_GAIN = 906,
    GSTR_MTR_EFFICIENCY = 907,
    GSTR_MTR_PERSD_CIVIL = 908,
    GSTR_MTR_PERSD_MILIT = 909,
    GSTR_MTR_PERSD_ADVERS = 910,
    GSTR_MTR_KILLD_CIVIL = 911,
    GSTR_MTR_KILLD_MILIT = 912,
    GSTR_MTR_KILLD_ADVERS = 913,
    GSTR_MTR_CYBORGS_LOST = 914,
    GSTR_MTR_LAST_CITY_NAME = 919,
    GSTR_MTR_CYBORGS_GAIN = 920,
    GSTR_MTR_RESEARCH_DONE = 921,
    GSTR_MTR_CREDS_EXPENDIT = 922,
    GSTR_MTR_CREDS_INCOME = 923,
    // Group: Network messages - 950..999
    // Network messages during mission
    GSTR_NET_LEFT_GAME = 960,
    GSTR_NET_HOST_QUIT = 961,
    // Group: Per-campaign names - 1000..1099
    // Campaign name
    GSTR_CPG_CAMPAIGN_NAME = 999,
    // Name for players cyborgs
    GSTR_CPG_CYBORG_NOUN_FULL = 1000,
};

#pragma pack()
/******************************************************************************/
extern char *gui_strings[STRINGS_MAX];
extern char *gui_strings_data;
extern char *gui_strings_data_end;
extern ulong text_buf_pos;

void snprint_dh_time_duration(char *out, ulong outlen, long ndays, short nhours);

/** Converts any kind of text pointer to a global text pointer.
 *
 * Global text pointers are pointers within scratch buffers, valid for
 * whole drawing the current frame. Such pointers can be used in drawlists.
 */
const char *loctext_to_gtext(const char *ltext);

void read_strings_file(void);
TbBool create_strings_list(char **strings, char *strings_data, char *strings_data_end);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
