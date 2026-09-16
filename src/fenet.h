/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fenet.h
 *     Header file for fenet.c.
 * @par Purpose:
 *     Front-end desktop and menu system, multiplayer screen.
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
#ifndef FENET_H
#define FENET_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/
extern const uint starting_cash_amounts[];

void init_net_screen_boxes(void);
void net_sessionlist_clear(void);
void net_unkn2_names_clear(void);
void show_netgame_unkn_case1(void);
void reset_net_screen_boxes_flags(void);
void reset_net_screen_EJECT_flags(void);
void set_flag01_net_screen_boxes(void);
void skip_flashy_draw_net_screen_boxes(void);
void switch_net_screen_boxes_to_initiate(void);
void switch_net_screen_boxes_to_execute(void);
void net_grpaint_draw_op(short a1, short a2, ubyte a3, sbyte a4, ubyte a5);
void net_grpaint_clear_op(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
