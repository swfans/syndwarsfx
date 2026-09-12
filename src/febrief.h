/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file febrief.h
 *     Header file for febrief.c.
 * @par Purpose:
 *     Front-end desktop and menu system, e-mail mission brief screen.
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
#ifndef FEBRIEF_H
#define FEBRIEF_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum BriefCityMapContentKinds {
    BriCtM_AUTO_SCANNER, /**< Scanner which allows auto-switching to other content */
    BriCtM_SCANNER,	/**< Scanner forced by a click on other content */
    BriCtM_ANIM,	/**< Animation plays */
};

#pragma pack()
/******************************************************************************/
extern short next_brief;
extern short old_mission_brief;
extern short open_brief;
extern ubyte open_ref;

extern char *mission_briefing_text;
#define mission_briefing_text_len 16384

void reveal_netscan_objective(short nsobv);
void brief_load_mission_info(void);
void load_netscan_data(ubyte city_id, ubyte level);

void init_brief_screen_boxes(void);
void init_brief_screen_scanner(void);

/** Reset players part of brief screen state.
 *
 * Resets all parts of brief screen which are set by the player
 * or related to players game progress.
 */
void reset_brief_screen_player_state(void);

void update_brief_screen_netscan_button(ushort text_id);
void reset_brief_screen_boxes_flags(void);
void set_flag01_brief_screen_boxes(void);

ubyte show_mission_screen(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
