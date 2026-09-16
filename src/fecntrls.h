/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fecntrls.h
 *     Header file for fecntrls.c.
 * @par Purpose:
 *     Front-end desktop and menu system, research screen.
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
#ifndef FECNTRLS_H
#define FECNTRLS_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#pragma pack()
/******************************************************************************/
extern ubyte net_unkn_pos_02;

void init_controls_screen_boxes(void);
void reset_controls_screen_boxes_flags(void);
ubyte show_options_controls_screen(void);
void set_flag01_controls_screen_boxes(void);
void skip_flashy_draw_controls_screen_boxes(void);

/** Returns whether the game is currently defining a key in controls screen.
 *
 * Most of control buttons should be disabled during control key definition.
 */
TbBool is_defining_control_key(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
