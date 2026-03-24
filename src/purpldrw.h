/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file purpldrw.h
 *     Header file for purpldrw.c.
 * @par Purpose:
 *     Adding items to drawlist for purple projector screens.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 28 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef PURPLDRW_H
#define PURPLDRW_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#pragma pack(1)

struct TbSprite;
struct ScreenShape;
struct ScreenBox;
struct ScreenTextBox;
struct ScreenInfoBox;
struct ScreenButton;

#pragma pack()
/******************************************************************************/

void draw_text_purple_list2(int x, int y, const char *text, ushort line);
void draw_sprite_purple_list(int x, int y, const struct TbSprite *sprite);
void draw_trig_purple_list(long x2, long y2, long x3, long y3);
void copy_box_purple_list(long x, long y, ulong width, ulong height);
void draw_box_purple_list(int x, int y, ulong width, ulong height, int colour);
void draw_line_purple_list(int x1, int y1, int x2, int y2, int colour);
void draw_triangle_purple_list(int x1, int y1, int x2, int y2, int x3, int y3, TbPixel colour);
void draw_hotspot_purple_list(int x, int y);
ubyte flashy_draw_purple_shape(struct ScreenShape *shape);
ubyte flashy_draw_purple_box(struct ScreenBox *p_box);
TbBool flashy_draw_text(int x, int y, const char *text, ubyte speed,
  int top_line, short *textpos, int cyan_flag);
ubyte flashy_draw_purple_text_box(struct ScreenTextBox *p_box);
void draw_flic_purple_list(void (*fn)());
void draw_noise_box_purple_list(int x, int y, ulong width, ulong height);

short get_text_box_lines_visible(struct ScreenTextBox *p_box);

// Watcom calling convention - do not call from C
ubyte ac_flashy_draw_purple_box(struct ScreenBox *p_box);
ubyte flashy_draw_purple_button(struct ScreenButton *p_button);
ubyte ac_flashy_draw_purple_text_box(struct ScreenTextBox *p_box);
ubyte flashy_draw_purple_info_box(struct ScreenInfoBox *p_box);
ubyte button_text(struct ScreenButton *p_button);
ubyte label_text(struct ScreenButton *p_btn);
ubyte info_box_text(struct ScreenInfoBox *p_box);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
