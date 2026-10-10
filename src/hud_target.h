/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file hud_target.h
 *     Header file for hud_target.c.
 * @par Purpose:
 *     Ingame Heads-Up Display targetted thing draw.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef HUD_TARGET_H
#define HUD_TARGET_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/
extern TbBool hud_show_target_health;

void show_goto_point(u32 flag);

void init_draw_target(void);
void draw_hud_lock_target(void);
void draw_hud_target_mouse(ThingIdx dcthing);
void draw_target_person(struct Thing *p_person, int radius);
void draw_hud_target2(short dcthing, short target);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
