/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_save.h
 *     Header file for game_save.c.
 * @par Purpose:
 *     Loading and saving the game and user profile.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 02 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GAME_SAVE_H
#define GAME_SAVE_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define SAVE_SLOTS_VISIBLE_COUNT 8

extern ulong save_mortal_salt;

extern ubyte *save_game_buffer;
extern char save_active_desc[25];
extern char login_name[16];

extern short save_slot; // = -1;
extern char save_slot_names[SAVE_SLOTS_VISIBLE_COUNT][25];
extern s32 save_slot_base;

/******************************************************************************/

void read_user_settings(void);
TbBool save_user_settings(void);
void set_default_user_settings(void);

void load_save_slot_names(void);
int save_game_write(ubyte slot, char *desc);
ubyte load_game(int slot, char *desc);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
