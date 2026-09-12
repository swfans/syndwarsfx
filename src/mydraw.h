/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file mydraw.h
 *     Header file for mydraw.c.
 * @par Purpose:
 *     Modified drawing routines adjusted for the specific app.
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
#ifndef MYDRAW_H
#define MYDRAW_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#pragma pack(1)

enum MyFontFlags {
    MyFF_NONE,
    MyFF_UPPERCASE,
};

struct TbSprite;

#pragma pack()
/******************************************************************************/
extern ushort my_font_flags;

extern ubyte byte_197160;

u32 my_str_len(const char *t);
int font_word_length(const char *text);

ubyte my_char_to_upper(ubyte c);
ubyte my_char_height(uchar c);

/** Amount of padding pixels at top of the char.
 */
ubyte my_char_padding_top(uchar c);

/** Amount of padding pixels at bottom of the char.
 */
ubyte my_char_padding_bottom(uchar c);

u32 my_string_width(const char *text);

ushort my_count_lines(const char *text);

ushort my_draw_text(short x, short y, const char *text, ushort startline);

/** When drawing using this font, prefer converting strings to upper case.
 */
TbBool my_font_prefer_upper_case(const struct TbSprite *p_font);

void my_preprocess_text(char *text);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
