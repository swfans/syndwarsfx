/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feworld.h
 *     Header file for feworld.c.
 * @par Purpose:
 *     Front-end desktop and menu system, world view screen.
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
#ifndef FEWORLD_H
#define FEWORLD_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/

/** Index of the city highlighted on world map screen.
 */
extern sbyte map_hl_city_id;

ubyte show_worldmap_screen(void);
void init_world_screen_boxes(void);

/** Reset players part of world screen state.
 *
 * Resets all parts of world screen which are set by the player
 * or related to players game progress.
 */
void reset_world_screen_player_state(void);

void reset_world_screen_boxes_flags(void);
void set_flag01_world_screen_boxes(void);
void skip_flashy_draw_world_screen_boxes(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
