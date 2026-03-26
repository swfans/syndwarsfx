/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file mydraw.c
 *     Modified drawing routines adjusted for the specific app.
 * @par Purpose:
 *     Functions which one of original devs apparently considered his.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 28 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "mydraw.h"

#include "bffont.h"
#include "bfsprite.h"
#include "bftext.h"

#include "display.h"
#include "game.h"
#include "game_data.h"
#include "game_sprts.h"
#include "swlog.h"
#include "util.h"
/******************************************************************************/
extern ubyte text_colours[15];

ushort my_font_flags = MyFF_NONE;

ubyte my_char_to_upper(ubyte c)
{
    return fontchrtoupper(c);
}

u32 my_str_len(const char *t)
{
    return strlen(t);
}

int font_word_length(const char *text)
{
    const ubyte *p;
    int len;

    len = 0;
    for (p = (const ubyte *)text; *p != '\0'; p++)
    {
        const struct TbSprite *spr;

        if (*p == 32)
            break;
        spr = LbFontCharSprite(small_font, *p);
        len += spr->SWidth;
    }
    return len;
}

TbBool my_font_prefer_upper_case(const struct TbSprite *p_font)
{
    return (p_font == small_font);
}

static int my_font_to_yshift(const struct TbSprite *p_font, char chr)
{
    if ((p_font == small_font) || (p_font == small2_font))
    {
        return 1;
    }
    else if (p_font == small_med_font)
    {
        return 2;
    }
    else if (p_font == med_font || p_font == med2_font)
    {
        return 2;
    }
    else if (p_font == big_font)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

ubyte my_char_padding_top(uchar c)
{
    //TODO depends on open font files, not only on the pointers
    if (lbFontPtr == small_font || lbFontPtr == small2_font)
    {
        return 1;
    }
    else if (lbFontPtr == small_med_font)
    {
        if (c >= 97 && c <= 122) // small letters
            return 0;
        else
            return 2;
    }
    else if (lbFontPtr == med_font || lbFontPtr == med2_font)
    {
        return 2;
    }
    else if (lbFontPtr == big_font)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

ubyte my_char_padding_bottom(uchar c)
{
    //TODO depends on open font files, not only on the pointers
    if (lbFontPtr == small_font || lbFontPtr == small2_font)
    {
        return 0;
    }
    else if (lbFontPtr == small_med_font)
    {
        if (c >= 97 && c <= 122) // small letters
            return 0;
        else
            return 1;
    }
    else if (lbFontPtr == med_font || lbFontPtr == med2_font)
    {
        return 1;
    }
    else if (lbFontPtr == big_font)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

ubyte my_char_height(uchar c)
{
#if 0
    int ret;
    asm volatile ("call ASM_font_height\n"
        : "=r" (ret) : "a" (c));
    return ret;
#endif
    return LbSprFontCharHeight(lbFontPtr, c)
      - my_char_padding_top(c)
      - my_char_padding_bottom(c);
}

ubyte my_char_width(uchar c)
{
    if ((my_font_flags & MyFF_UPPERCASE) != 0) {
        c = my_char_to_upper(c);
    }
    return LbTextCharWidth(c);
}


u32 my_string_width(const char *text)
{
    const char *p_chr;
    u32 str_w;
    ubyte c;

    if (my_font_prefer_upper_case(lbFontPtr))
        my_font_flags |= MyFF_UPPERCASE;
    else
        my_font_flags &= ~MyFF_UPPERCASE;

    str_w = 0;
    for (p_chr = text; *p_chr != '\0'; p_chr++)
    {
        c = *p_chr;
        if (c > 31)
        {
            str_w += my_char_width(c);
        }
    }
    return str_w;
}

ushort my_count_lines(const char *text)
{
#if 0
    ushort ret;
    asm volatile ("call ASM_my_count_lines\n"
        : "=r" (ret) : "a" (text));
    return ret;
#endif
    int pos, line_beg_pos, last_brkpoint_pos;
    int txline_len, last_brkpoint_lnlen;
    ushort nlines;
    ubyte chr;

    pos = 0;
    nlines = 0;
    if (text == NULL)
        return 1;

    last_brkpoint_lnlen = 0;
    txline_len = 0;
    last_brkpoint_pos = 0;
    line_beg_pos = 0;
    while ( 1 )
    {
        chr = text[pos++];
        if (chr == '\0')
            return nlines + 1;
        if (lbFontPtr != small_med_font || language_3str[0] != 'e')
        {
            chr = fontchrtoupper(chr);
        }
        if (chr == 0x0E) // skip next char
        {
            pos++;
        }
        else if ((chr == ' ') || (chr == '-'))
        {
            last_brkpoint_pos = pos;
            last_brkpoint_lnlen = txline_len;
        }
        else if (chr == '\n')
        {
            if (text[pos] != '\0')
                nlines++;
            line_beg_pos = pos;
            last_brkpoint_pos = pos;
            txline_len = 0;
        }
        if (chr >= 0x0E)
        {
            txline_len += LbSprFontCharWidth(lbFontPtr, chr);
        }

      if (txline_len > (text_window_x2 - text_window_x1))
      {
        if (last_brkpoint_pos == line_beg_pos)
        {
            // No to finish a word place, but also no position for proper
            // word break. Back any control chars, and just break where we are.
            do {
                pos--;
            } while (text[pos] < 0x1F);
            do {
                pos--;
            } while (text[pos] < 0x1F);
            last_brkpoint_pos = pos;
            line_beg_pos = pos;
            nlines++;
            txline_len = 0;
            last_brkpoint_lnlen = 0;
        }
        else
        {
            line_beg_pos = last_brkpoint_pos;
            txline_len -= last_brkpoint_lnlen;
            nlines++;
            if (text[last_brkpoint_pos - 1] == ' ') {
                txline_len -= LbSprFontCharWidth(lbFontPtr, ' ');
            }
        }
      }
    }
    return 1;
}

/** Parse control char from given string, return num bytes recognized.
 */
static ubyte my_draw_apply_control_char(const char *ctext)
{
    ubyte cc;

    cc = (ubyte)ctext[0];

    if (cc == 0x01)
    {
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0) {
            lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
        } else {
            lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        }
        return 1;
    }
    if (cc == 0x0C)
    {
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0) {
            lbDisplay.DrawFlags &= Lb_TEXT_ONE_COLOR;
        } else {
            lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
        }
        return 1;
    }
    if (cc == 0x0E)
    {
        lbDisplay.DrawColour = text_colours[ctext[1] - 1];
        return 2;
    }
    if (cc == 0x1B)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
        return 1;
    }
    if (cc == 0x1C)
    {
        lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
        return 1;
    }
    if (cc == 0x1E)
    {
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
        return 1;
    }
    return 0;
}

static short my_draw_one_char(short x, short y, char c)
{
    ubyte uc;
    short dy;

    uc = (ubyte)c;
    if ((my_font_flags & MyFF_UPPERCASE) != 0) {
        uc = fontchrtoupper(c);
    }
    dy = my_font_to_yshift(lbFontPtr, uc);

    if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0) {
        LbSpriteDrawOneColour(x, y - dy,
          LbFontCharSprite(lbFontPtr, uc),
          lbDisplay.DrawColour);
    } else {
        LbSpriteDraw(x, y - dy, LbFontCharSprite(lbFontPtr, uc));
    }
    return LbTextCharWidth(uc);
}

/** Draw characters from `text` between `k_beg` and `k_end`, minding control sequences.
 */
static short my_draw_chunk(short x, short y, short tot_width,
  const char *text, short k_beg, short k_end)
{
    short cx, k;

    cx = x;
    if ((lbDisplay.DrawFlags & Lb_TEXT_HALIGN_RIGHT) != 0)
    {
        cx += (tot_width);
    }
    else if ((lbDisplay.DrawFlags & Lb_TEXT_HALIGN_CENTER) != 0)
    {
        cx += (tot_width >> 1);
    }

    for (k = k_beg; k < k_end; k++)
    {
        char c;
        ubyte nb;

        c = text[k];
        if ((ubyte)c <= 0x1f)
        {
            nb = my_draw_apply_control_char(&text[k]);
            if (nb > 1)
                k += nb - 1;
        }
        else
        {
            cx += my_draw_one_char(cx, y, c);
        }
    }

    return cx - x;
}

/** Skips characters from `text` between `k_beg` and `k_end`, minding control sequences.
 *
 * Replacement for `my_draw_chunk()` when the actual drawing should not be performed.
 */
static void my_skip_chunk(short x, short y, short tot_width,
  const char *text, short k_beg, short k_end)
{
    short k;

    for (k = k_beg; k < k_end; k++)
    {
        char c;
        ubyte nb;

        c = text[k];
        if ((ubyte)c <= 0x1f)
        {
            nb = my_draw_apply_control_char(&text[k]);
            if (nb > 1)
                k += nb - 1;
        }
    }
}

ushort my_draw_text(short x, short y, const char *text, ushort startline)
{
#if 0
    ushort ret;
    asm volatile ("call ASM_my_draw_text\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (text), "c" (startline));
    return ret;
#endif
    int beg_x, scr_x;
    ubyte uch;
    int ck_end;
    int ck_width, cur_ck_width;
    ushort fin_ck_width;
    ushort ck_beg, pv_ck_end;
    short scr_y;
    ushort wndw_width;
    int cur_line;

    ck_end = 0;
    cur_line = 0;
    ck_width = 0;
    cur_ck_width = 0;
    if (text == NULL)
        return 0;
    pv_ck_end = 0;
    beg_x = text_window_x1 + x;
    scr_x = beg_x;
    scr_y = text_window_y1 + y;
    wndw_width = text_window_x2 - scr_x;
    ck_beg = 0;

    if (my_font_prefer_upper_case(lbFontPtr))
        my_font_flags |= MyFF_UPPERCASE;
    else
        my_font_flags &= ~MyFF_UPPERCASE;

    while ( 1 )
    {
        uch = text[ck_end++];
        if (uch == '\0')
            break;
        if ((my_font_flags & MyFF_UPPERCASE) != 0) {
            uch = fontchrtoupper(uch);
        }

        if (uch == 0x0A)
        {
            if (cur_line < startline)
            {
                my_skip_chunk(scr_x, scr_y, wndw_width - cur_ck_width, text, ck_beg, ck_end - 1);
            }
            else
            {
                if (scr_y >= text_window_y1) {
                    scr_x += my_draw_chunk(scr_x, scr_y, wndw_width - cur_ck_width, text, ck_beg, ck_end - 1);
                }
                scr_y += my_char_height('A') + byte_197160;
                if (scr_y + my_char_height('A') > text_window_y2)
                    return cur_line;
            }

            scr_x = beg_x;
            if (text[ck_end] != '\0')
                  ++cur_line;
            ck_beg = ck_end;
            pv_ck_end = ck_end;
            cur_ck_width = 0;
        }
        else if (uch == 0x0E)
        {
            ck_end++;
        }
        if (uch > 0x1F)
        {
            if (uch == '-')
            {
                pv_ck_end = ck_end;
                ck_width = cur_ck_width + LbTextCharWidth(uch);
            }
            else if (uch == ' ')
            {
                pv_ck_end = ck_end;
                ck_width = cur_ck_width;
            }
            cur_ck_width += LbTextCharWidth(uch);
        }

        if ((ushort)cur_ck_width > wndw_width)
        {
          if (pv_ck_end == ck_beg)
          {
            uch = text[ck_end];
            if (uch != '\0') {
                cur_ck_width -= LbTextCharWidth(uch);
            }

            // go back with ck_end to the previous non-control char
            do {
                ck_end--;
            } while ((ubyte)text[ck_end] <= 0x1F);

            uch = text[ck_end];
            fin_ck_width = cur_ck_width - LbTextCharWidth(uch);

            // go back with ck_end to the previous non-control char again
            do {
                ck_end--;
            } while ((ubyte)text[ck_end] <= 0x1F);

            uch = text[ck_end];
            fin_ck_width = fin_ck_width - LbTextCharWidth(uch);
            fin_ck_width = fin_ck_width + LbTextCharWidth('-');

            if (startline > cur_line)
            {
                my_skip_chunk(scr_x, scr_y, wndw_width - fin_ck_width, text, ck_beg, ck_end - 1);
            }
            else
            {
                if (scr_y >= text_window_y1) {
                    scr_x += my_draw_chunk(scr_x, scr_y, wndw_width - fin_ck_width, text, ck_beg, ck_end);
                    scr_x += my_draw_one_char(scr_x, scr_y, '-');
                }
                scr_y += my_char_height('A') + byte_197160;
                if (scr_y + my_char_height('A') > text_window_y2)
                    return cur_line;
            }

            pv_ck_end = ck_end;
            ck_width = 0;
            cur_ck_width = 0;
          }
          else
          {
            if (cur_line < startline)
            {
                my_skip_chunk(scr_x, scr_y, wndw_width - ck_width, text, ck_beg, ck_end - 1);
            }
            else
            {
                if (scr_y >= text_window_y1) {
                    scr_x += my_draw_chunk(scr_x, scr_y, wndw_width - ck_width, text, ck_beg, pv_ck_end);
                }
                scr_y += my_char_height('A') + byte_197160;
                if (scr_y + my_char_height('A') > text_window_y2)
                    return cur_line;
            }


            cur_ck_width -= ck_width;
            if (text[pv_ck_end - 1] == ' ') {
              cur_ck_width -= LbTextCharWidth(' ');
            }
          }
          scr_x = beg_x;
          ck_beg = pv_ck_end;
          cur_line++;
        }
    }

    if (cur_line < startline)
    {
        my_draw_chunk(scr_x, scr_y, wndw_width - cur_ck_width, text, ck_beg, ck_end - 1);
    }
    else
    {
        if (scr_y >= text_window_y1) {
            scr_x += my_draw_chunk(scr_x, scr_y, wndw_width - cur_ck_width, text, ck_beg, ck_end - 1);
        }
        cur_line += 1;
    }
    return cur_line;
}

/******************************************************************************/
