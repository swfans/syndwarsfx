/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file purpldrw.c
 *     Adding items to drawlist for purple projector screens.
 * @par Purpose:
 *     Implements functions for filling items within drawlist.
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
#include "purpldrw.h"

#include "bfconfig.h"
#include "bfutility.h"
#include "bfmemut.h"
#include "bfscreen.h"
#include "bfsprite.h"
#include "bftext.h"
#include "poly.h"
#include "bfjoyst.h"
#include "ssampply.h"

#include "display.h"
#include "game.h"
#include "game_sprts.h"
#include "guiboxes.h"
#include "guitext.h"
#include "keyboard.h"
#include "mydraw.h"
#include "purpldrwlst.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
enum ScrollBarBoxes {
    ScrlBB_AreaAbv = 0,	/**< empty area above handle */
    ScrlBB_HndlBox,	/**< draggable handle box */
    ScrlBB_AreaBlw,	/**< empty area below handle */
    ScrlBB_ArrowUp,	/**< up arrow at bottom */
    ScrlBB_ArrowDn,	/**< down arrow at bottom */
    ScrlBB_BoxesCount,
};

// TODO avoid referring to a specific box
extern struct ScreenTextBox brief_netscan_box;
extern struct ScreenTextBox world_city_info_box;
extern struct ScreenTextBox equip_display_box;
extern struct ScreenTextBox cryo_cybmod_list_box;
extern struct ScreenTextBox controls_list_box;
extern struct ScreenButton alert_OK_button;

extern long dword_1DC5FC;
extern long dword_1DC600;
extern long dword_1DC624;
extern long dword_1DC628;

extern long purple_box_x1;
extern long purple_box_y1;
extern long purple_box_x2;
extern long purple_box_y2;

void draw_line_purple_list(int x1, int y1, int x2, int y2, int colour)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_draw_line_purple_list\n"
        : : "a" (x1), "d" (y1), "b" (x2), "c" (y2), "g" (colour));
    return;
#endif
    struct PurpleDrawItem *pditem;

    pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    pditem->U.Line.X1 = lbDisplay.GraphicsWindowX + x1;
    pditem->U.Line.Y1 = lbDisplay.GraphicsWindowY + y1;
    pditem->U.Line.X2 = lbDisplay.GraphicsWindowX + x2;
    pditem->U.Line.Y2 = lbDisplay.GraphicsWindowY + y2;
    pditem->U.Line.Colour = colour;
    pditem->Flags = lbDisplay.DrawFlags;

    if ((x1 == x2) || (y1 == y2))
        pditem->Type = PuDT_HVLINE;
    else
        pditem->Type = PuDT_LINE;
}

void draw_box_purple_list(int x, int y, ulong width, ulong height, int colour)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_draw_box_purple_list\n"
        : : "a" (x), "d" (y), "b" (width), "c" (height), "g" (colour));
    return;
#endif
    struct PurpleDrawItem *pditem;

    pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    pditem->U.Box.X = lbDisplay.GraphicsWindowX + x;
    pditem->U.Box.Y = lbDisplay.GraphicsWindowY + y;
    pditem->U.Box.Width = width;
    pditem->U.Box.Colour = colour;
    pditem->U.Box.Height = height;
    pditem->Flags = lbDisplay.DrawFlags;
    pditem->Type = PuDT_BOX;
}

void draw_text_purple_list2(int x, int y, const char *text, ushort line)
{
#if 0
    asm volatile (
      "call ASM_draw_text_purple_list2\n"
        : : "a" (x), "d" (y), "b" (text), "c" (line));
    return;
#endif
    struct PurpleDrawItem *pditem;

    pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    pditem->U.Text.X = x;
    pditem->U.Text.Y = y;
    pditem->U.Text.WindowX = text_window_x1;
    pditem->U.Text.WindowY = text_window_y1;
    pditem->U.Text.Width = text_window_x2 - text_window_x1 + 1;
    pditem->U.Text.Height = text_window_y2 - text_window_y1 + 1;
    pditem->U.Text.Text = text;
    pditem->U.Text.Line = line;
    pditem->U.Text.Colour = lbDisplay.DrawColour;
    pditem->U.Text.Font = lbFontPtr;
    pditem->Flags = lbDisplay.DrawFlags;
    pditem->Type = PuDT_TEXT;
}

void draw_sprite_purple_list(int x, int y, const struct TbSprite *p_sprite)
{
#if 0
    asm volatile (
      "call ASM_draw_sprite_purple_list\n"
        : : "a" (x), "d" (y), "b" (p_sprite));
#endif
    struct PurpleDrawItem *pditem;

    pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    pditem->U.Sprite.Sprite = p_sprite;
    pditem->U.Sprite.Colour = lbDisplay.DrawColour;
    pditem->Flags = lbDisplay.DrawFlags;
    pditem->U.Sprite.X = lbDisplay.GraphicsWindowX + x;
    pditem->U.Sprite.Y = lbDisplay.GraphicsWindowY + y;
    pditem->Type = PuDT_SPRITE;
}

void draw_trig_purple_list(long x2, long y2, long x3, long y3)
{
    asm volatile (
      "call ASM_draw_trig_purple_list\n"
        : : "a" (x2), "d" (y2), "b" (x3), "c" (y3));
}

void copy_box_purple_list(long x, long y, ulong width, ulong height)
{
    asm volatile (
      "call ASM_copy_box_purple_list\n"
        : : "a" (x), "d" (y), "b" (width), "c" (height));
}

void draw_hotspot_purple_list(int x, int y)
{
    asm volatile (
      "call ASM_draw_hotspot_purple_list\n"
        : : "a" (x), "d" (y));
}

ubyte flashy_draw_purple_shape(struct ScreenShape *p_shape)
{
    ubyte ret;
    asm volatile ("call ASM_flashy_draw_purple_shape\n"
        : "=r" (ret) : "a" (p_shape));
    return ret;
}

void flashy_draw_projector_reaching_target_coords(short box_x, short box_y, short box_w, short box_h, short advance, TbPixel colour)
{
    struct TbSprite *p_spr;
    ushort spr;
    short scr_x1, scr_y2;
    short scr_x2, scr_y1;

    spr = 11 + (colour != 0xF7);

    scr_x1 = proj_origin.X + (advance * (box_x - proj_origin.X)) / 24;
    scr_y1 = proj_origin.Y + (advance * (box_y - proj_origin.Y)) / 24;
    scr_x2 = proj_origin.X + (advance * (box_x + box_w - proj_origin.X)) / 24;
    scr_y2 = proj_origin.Y + (advance * (box_y + box_h - proj_origin.Y)) / 24;

    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_line_purple_list(proj_origin.X, proj_origin.Y, scr_x1, scr_y1, colour);
    draw_line_purple_list(proj_origin.X, proj_origin.Y, scr_x2, scr_y2, colour);
    lbDisplay.DrawFlags = 0;

    p_spr = &fe_mouseptr_sprites[spr];
    draw_sprite_purple_list(scr_x1 - 1, scr_y1 - 1, p_spr);
    draw_sprite_purple_list(scr_x2 - 1, scr_y2 - 1, p_spr);
}

void flashy_draw_projector_horizontal_lines(short box_x, short box_y, short box_w, short box_h, short advance, short advance_p, short delta_p, TbPixel colour)
{
    struct TbSprite *p_spr;
    ushort spr;
    short scr_x1, scr_x2;
    short prog_x;

    spr = 11 + (colour != 0xF7);
    vec_colour = colour;

    prog_x = box_x + advance_p;
    purple_box_x1 = prog_x;
    purple_box_y1 = box_y;
    purple_box_x2 = purple_box_x1 - 15;
    scr_x1 = box_x;
    if (purple_box_x2 < scr_x1)
        purple_box_x2 = scr_x1;
    purple_box_y2 = box_y;

    draw_trig_purple_list(purple_box_x1, purple_box_y1, purple_box_x2, purple_box_y2);
    draw_line_purple_list(scr_x1, purple_box_y2, purple_box_x1, purple_box_y1, colour);

    purple_box_x1 = box_x + box_w - advance;
    purple_box_y1 = box_y + box_h;
    purple_box_x2 = purple_box_x1 + 15;
    scr_x2 = box_x + box_w;
    if (purple_box_x2 > scr_x2)
      purple_box_x2 = scr_x2;
    purple_box_y2 = box_y + box_h;

    draw_trig_purple_list(purple_box_x1, purple_box_y1, purple_box_x2, purple_box_y2);
    draw_line_purple_list(purple_box_x1, purple_box_y1, scr_x2, purple_box_y2, colour);

    lbDisplay.DrawFlags = 0;
    p_spr = &fe_mouseptr_sprites[spr];
    draw_sprite_purple_list(prog_x - 1, box_y - 1, p_spr);
    draw_sprite_purple_list(purple_box_x1 - 1, box_y + box_h - 1, p_spr);
}

void flashy_draw_projector_vertical_lines(short box_x, short box_y, short box_w, short box_h, short advance, short advance_p, short delta_p, TbPixel colour)
{
    struct TbSprite *p_spr;
    ushort spr;
    short scr_y1, scr_y2;
    short prog_y;

    spr = 11 + (colour != 0xF7);
    vec_colour = colour;

    prog_y = box_y + delta_p + advance_p;
    purple_box_x1 = box_x + box_w;
    purple_box_y1 = prog_y;
    purple_box_x2 = purple_box_x1;
    scr_y1 = box_y + delta_p;
    purple_box_y2 = prog_y - 15;
    if (purple_box_y2 < scr_y1)
      purple_box_y2 = scr_y1;

    draw_trig_purple_list(purple_box_x1, purple_box_y1, purple_box_x2, purple_box_y2);
    draw_line_purple_list(purple_box_x2, scr_y1, purple_box_x1, purple_box_y1, colour);

    purple_box_x1 = box_x;
    purple_box_y1 = box_y + box_h - advance;
    purple_box_x2 = purple_box_x1;
    purple_box_y2 = purple_box_y1 + 15;
    scr_y2 = box_y + box_h;
    if (purple_box_y2 > scr_y2)
      purple_box_y2 = scr_y2;

    draw_trig_purple_list(purple_box_x1, purple_box_y1, purple_box_x2, purple_box_y2);
    draw_line_purple_list(purple_box_x1, purple_box_y1, purple_box_x2, scr_y2, colour);

    draw_line_purple_list(box_x, box_y + box_h, box_x + box_w, box_y + box_h, colour);
    if (delta_p != 0) {
        draw_line_purple_list(box_x, box_y, box_x + box_w - delta_p, box_y, colour);
        draw_line_purple_list(box_w + box_x - delta_p, box_y, box_w + box_x, box_y + delta_p, colour);
    } else {
        draw_line_purple_list(box_x, box_y, box_x + box_w, box_y, colour);
    }

    lbDisplay.DrawFlags = 0;
    p_spr = &fe_mouseptr_sprites[spr];
    draw_sprite_purple_list(purple_box_x1 - 1, purple_box_y1 - 1, p_spr);
    draw_sprite_purple_list(box_x + box_w - 1, prog_y - 1, p_spr);
}

ubyte flashy_draw_purple_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_flashy_draw_purple_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short box_w, box_h;

    box_w = p_box->Width - 1;
    box_h = p_box->Height - 1;

    if ((p_box->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_box->Timer = -1;
        p_box->Timer2 = -1;
        p_box->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
    }
    if ((p_box->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_box->Timer2 = 0;
        p_box->Flags &= ~(GBxFlg_Unkn0001|GBxFlg_Unkn0080);
        p_box->Timer = p_box->Timer2;
        p_box->Flags |= GBxFlg_Unkn0080;
    }

    // Handle sound
    if (p_box->Timer <= 72)
    {
        if (p_box->Timer > 24)
        {
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_box->Timer <= 24)
    {
        short advance;

        advance = p_box->Timer;
        flashy_draw_projector_reaching_target_coords(p_box->X, p_box->Y, box_w, box_h, advance, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw horizontal lines
    if (p_box->Timer <= 48)
    {
        short advance;

        advance = ((p_box->Timer - 24) * box_w) / 24;
        flashy_draw_projector_horizontal_lines(p_box->X, p_box->Y, box_w, box_h, advance, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_box->Timer <= 72)
    {
        short advance;

        advance = ((p_box->Timer - 48) * box_h) / 24;
        flashy_draw_projector_vertical_lines(p_box->X, p_box->Y, box_w, box_h, advance, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 2;
    }

    // Now as drawing animation finished, the background became static
    // Draw and store the static background
    if ((p_box->Flags & GBxFlg_BkCopied) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X - 3, p_box->Y - 3,
          p_box->Width + 6, p_box->Height + 6, p_box->BGColour);
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X, p_box->Y,
          p_box->Width, p_box->Height, p_box->Colour);
        lbDisplay.DrawFlags = 0;

        if ((p_box->Flags & GBxFlg_NoBkCopy) == 0)
        {
            copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
              p_box->Width + 6, p_box->Height + 6);
            p_box->Flags |= GBxFlg_BkCopied;
        }
    }
    if (p_box->SpecialDrawFn != NULL) {
          p_box->SpecialDrawFn(p_box);
    }
    return 3;
}

void my_set_text_window_for_text_box(struct ScreenTextBox *p_box)
{
    short scr_scroll_w, scr_scroll_h;

    if ((p_box->Flags & GBxFlg_RadioBtn) != 0)
        scr_scroll_w = 12;
    else
        scr_scroll_w = 0;

    if ((p_box->Buttons[0] != NULL) || (p_box->Infos[0] != NULL) || (p_box == &brief_netscan_box))
        scr_scroll_h = p_box->ScrollWindowHeight + 2;
    else
        scr_scroll_h = p_box->ScrollWindowHeight + 23;

    my_set_text_window(p_box->X + 4, p_box->ScrollWindowOffset + p_box->Y + 4, p_box->Width - 8 - scr_scroll_w, scr_scroll_h);
}

/** Get amount of lines within currently set text window.
 */
short get_text_box_window_lines_visible(struct ScreenTextBox *p_box)
{
    short text_window_h;
    text_window_h = text_window_y2 - text_window_y1 - 1;
    return (text_window_h + 2) / p_box->LineHeight;
}

/** Get amount of lines within given text box.
 * This function alters my_text_window and the current font.
 */
short get_text_box_lines_visible(struct ScreenTextBox *p_box)
{
    my_set_text_window_for_text_box(p_box);

    lbFontPtr = p_box->Font;
    if (p_box->LineHeight == 0)
        p_box->LineHeight = p_box->LineSpacing + my_char_height('A');

    return get_text_box_window_lines_visible(p_box);
}

TbBool flashy_draw_text(int x, int y, const char *text, ubyte speed, int top_line, short *textpos, int cyan_flag)
{
#if 0
    TbBool ret;
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "call ASM_flashy_draw_text\n"
        : "=r" (ret) : "a" (x), "d" (y), "b" (text), "c" (speed), "g" (top_line), "g" (textpos), "g" (cyan_flag));
    return ret;
#endif
    char *gtext;
    int text_tot_len, chunk_end;
    int i, pos;
    sbyte chunk_no;

    if (text == NULL) {
        return 1;
    }
    text_tot_len = strlen(text);
    pos = *textpos;
    if (pos >= text_tot_len) {
        draw_text_purple_list2(x, y, text, top_line);
        return 1;
    }
    gtext = (char *)back_buffer + text_buf_pos;
    if (pos > 0)
        LbMemoryCopy(gtext, text, pos);
    if (pos <= 0)
        pos = 0;
    if (cyan_flag)
    {
        chunk_no = 12;
        gtext[pos] = 27;
        pos += 1;
        for (i = 0; i < 4; i++)
        {
            chunk_end = i + *textpos;
            if ((chunk_end >= 0) && (chunk_end < text_tot_len))
            {
              gtext[pos + 0] = 14;
              gtext[pos + 1] = chunk_no;
              gtext[pos + 2] = text[i + *textpos];
              pos += 3;
            }
            ++chunk_no;
        }
    }
    else
    {
        chunk_no = 6;
        gtext[pos] = 27;
        pos += 1;
        for (i = 0; i < 6; i++)
        {
            chunk_end = i + *textpos;
            if ((chunk_end >= 0) && (chunk_end < text_tot_len))
            {
              gtext[pos + 0] = 14;
              gtext[pos + 1] = chunk_no;
              gtext[pos + 2] = text[i + *textpos];
              pos += 3;
            }
            ++chunk_no;
        }
    }
    gtext[pos + 0] = 28;
    gtext[pos + 1] = 0;
    pos += 2;
    text_buf_pos += pos;

    *textpos += speed;
    draw_text_purple_list2(x, y, gtext, top_line);
    play_sample_using_heap(0, 114, 127, 64, 100, 0, 3);
    return 0;
}

ubyte flashy_draw_purple_text_box_text(struct ScreenTextBox *p_box)
{
    ubyte text_drawn;

    lbDisplay.DrawFlags = 0;
    if ((p_box->Flags & GBxFlg_TextRight) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_RIGHT;
    }
    else if ((p_box->Flags & GBxFlg_TextCenter) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    }

    text_drawn = true;

    if ((p_box->Flags & GBxFlg_TextCopied) != 0)
        return text_drawn;

    if (p_box->DrawTextFn != NULL)
    {
        ubyte drawn;
        //drawn = p_box->DrawTextFn(p_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
          : "=r" (drawn) : "a" (p_box), "g" (p_box->DrawTextFn));
        text_drawn = text_drawn && drawn;
    }
    else if (p_box->Text != NULL)
    {
        ubyte drawn;
        drawn = flashy_draw_text(0,  0, p_box->Text, p_box->TextSpeed, p_box->TextTopLine,
              &p_box->TextFadePos, p_box->DrawTextFn != NULL);
        text_drawn = text_drawn && drawn;
    }

    if (text_drawn && ((p_box->Flags & GBxFlg_RadioBtn) == 0))
    {
          copy_box_purple_list(p_box->X - 3, p_box->Y - 3, p_box->Width + 6, p_box->Height + 6);
          p_box->Flags |= GBxFlg_TextCopied;
    }

    return text_drawn;
}

ubyte flashy_draw_purple_text_box_children(struct ScreenTextBox *p_box)
{
    ushort i;
    ubyte all_drawn;

    lbDisplay.DrawFlags = 0;
    for (i = 0; i < 2; i++)
    {
        struct ScreenButton *p_button;
        struct ScreenInfoBox *p_info;

        p_button = p_box->Buttons[i];
        if (p_button != NULL) {
            ubyte drawn;
            //p_button->DrawFn(p_button); -- incompatible calling convention
            asm volatile ("call *%2\n"
                : "=r" (drawn) : "a" (p_button), "g" (p_button->DrawFn));
            all_drawn = all_drawn && drawn;
        }
        p_info = p_box->Infos[i];
        if (p_info != NULL) {
            ubyte drawn;
            //p_info->DrawFn(p_info); -- incompatible calling convention
            asm volatile ("call *%2\n"
                : "=r" (drawn) : "a" (p_info), "g" (p_info->DrawFn));
            all_drawn = all_drawn && drawn;
        }
    }
    return all_drawn;
}

void purple_text_box_set_scroll_vertical_pos(struct ScreenTextBox *p_box, short scroll_pos)
{
    if (scroll_pos < 0)
        scroll_pos = 0;
    else if (scroll_pos + p_box->ScrollBarSize > p_box->ScrollWindowHeight)
        scroll_pos = p_box->ScrollWindowHeight - p_box->ScrollBarSize;

    p_box->ScrollBarPos = scroll_pos;
    p_box->TextTopLine = p_box->Lines * p_box->ScrollBarPos / p_box->ScrollWindowHeight;
}

void init_scroll_bar_for_text_box(struct ScreenBox *ar_scroll_bar, struct ScreenTextBox *p_box)
{
    init_screen_box(&ar_scroll_bar[ScrlBB_AreaAbv], p_box->X + p_box->Width - 12,
      p_box->Y + p_box->ScrollWindowOffset + 5,
      6, p_box->ScrollBarPos, p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_HndlBox], p_box->X + p_box->Width - 12,
      p_box->Y + p_box->ScrollWindowOffset + 5 + p_box->ScrollBarPos,
      6, p_box->ScrollBarSize, p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_AreaBlw], p_box->X + p_box->Width - 12,
      p_box->Y + p_box->ScrollWindowOffset + 5 + p_box->ScrollBarPos + p_box->ScrollBarSize,
      6, p_box->ScrollWindowHeight + 1 - (p_box->ScrollBarPos + p_box->ScrollBarSize), p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_ArrowUp], p_box->X + p_box->Width - 13,
      p_box->Y + p_box->ScrollWindowHeight + p_box->ScrollWindowOffset + 9,
      8, 9, p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_ArrowDn], p_box->X + p_box->Width - 13,
      p_box->Y + p_box->ScrollWindowHeight + p_box->ScrollWindowOffset + 18,
      8, 9, p_box->DrawSpeed);
}

void input_purple_text_box_wth_scroll(struct ScreenTextBox *p_box, struct ScreenBox *ar_scroll_bar)
{
    short lines_visible;
    TbBool no_keyboard_inputs;

    lines_visible = get_text_box_window_lines_visible(p_box);
    //TODO make this into a flag within the box
    no_keyboard_inputs = (p_box == &controls_list_box);

    if (lbDisplay.MLeftButton && ((p_box->Flags & GBxFlg_IsPushed) != 0))
    {
        short scroll_pos;

        scroll_pos = mouse_move_position_vertical_scrollbar_over_text_box(p_box);
        purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) != 0) && (p_box->ScrollBarPos >= 0))
    {
        // Clicking the scroll bar area above the handle
        if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_AreaAbv]))
        {
            if (lbDisplay.LeftButton)
            {
                short scroll_pos;

                scroll_pos = p_box->ScrollBarPos - (lines_visible - 1) * p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }
        // Clicking the scroll bar area below the handle
        else if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_AreaBlw]))
        {
            if (lbDisplay.LeftButton)
            {
                short scroll_pos;

                scroll_pos = p_box->ScrollBarPos + (lines_visible - 1) * p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }

        // Input draging the scroll bar handle
        if (((p_box->Flags & GBxFlg_IsPushed) != 0) || mouse_move_over_box(&ar_scroll_bar[ScrlBB_HndlBox]))
        {
            if (lbDisplay.LeftButton)
            {
                int ms_y;
                lbDisplay.LeftButton = 0;
                p_box->Flags |= GBxFlg_IsPushed;
                ms_y = lbDisplay.ScreenMode == 1 ? 2 * lbDisplay.MouseY : lbDisplay.MouseY;
                p_box->GrabPos = ms_y - p_box->ScrollBarPos;
                play_sample_using_heap(0, 125, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
            }
        }

        if (mouse_move_over_box(p_box) && ((p_box->Flags & GBxFlg_IsPushed) == 0))
        {
            // Using keyboard to scroll
            if (no_keyboard_inputs)
            {
                // User requested disable - the same keys are probably used for something else
            }
            else if (is_key_pressed(KC_UP, KMod_DONTCARE) || is_key_pressed(KC_DOWN, KMod_DONTCARE) ||
              is_key_pressed(KC_PGUP, KMod_DONTCARE) || is_key_pressed(KC_PGDOWN, KMod_DONTCARE))
            {
                short scroll_pos, delta;

                scroll_pos = p_box->ScrollBarPos;

                delta = 0;
                if (is_key_pressed(KC_DOWN, KMod_DONTCARE))
                    delta = p_box->ScrollWindowHeight / p_box->Lines;
                else if (is_key_pressed(KC_UP, KMod_DONTCARE))
                    delta = - p_box->ScrollWindowHeight / p_box->Lines;
                scroll_pos += delta;

                delta = 0;
                if (is_key_pressed(KC_PGDOWN, KMod_DONTCARE))
                    delta = p_box->ScrollWindowHeight * (lines_visible - 1) / p_box->Lines;
                else if (is_key_pressed(KC_PGUP, KMod_DONTCARE))
                    delta = - (lines_visible - 1) * p_box->ScrollWindowHeight / p_box->Lines;
                scroll_pos += delta;

                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
#if defined(LB_ENABLE_MOUSE_WHEEL)
            // Using mouse wheel to scroll
            if (lbDisplay.WheelMoveUp || lbDisplay.WheelMoveDown)
            {
                short scroll_pos, delta;

                scroll_pos = p_box->ScrollBarPos;

                delta = 0;
                if (lbDisplay.WheelMoveDown) {
                    delta = p_box->ScrollWindowHeight / p_box->Lines * lbDisplay.WheelMoveDown;
                    lbDisplay.WheelMoveDown = 0;
                }
                else if (lbDisplay.WheelMoveUp) {
                    delta = - p_box->ScrollWindowHeight / p_box->Lines * lbDisplay.WheelMoveUp;
                    lbDisplay.WheelMoveUp = 0;
                }
                scroll_pos += delta;

                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
#endif
        }

        // Input from up arrow in bottom part of the scroll bar
        if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_ArrowUp]))
        {
            if (lbDisplay.MLeftButton || joy.Buttons[0])
            {
                short scroll_pos;

                if (lbDisplay.LeftButton)
                    play_sample_using_heap(0, 125, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
                lbDisplay.LeftButton = 0;
                p_box->Flags |= GBxFlg_IsRPushed;
                ar_scroll_bar[ScrlBB_ArrowUp].Flags |= GBxFlg_IsRPushed;

                scroll_pos = p_box->ScrollBarPos - p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }

        // Input from down arrow in bottom part of the scroll bar
        if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_ArrowDn]))
        {
            if (lbDisplay.MLeftButton || joy.Buttons[0])
            {
                short scroll_pos;

                if (lbDisplay.LeftButton)
                    play_sample_using_heap(0, 125, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
                lbDisplay.LeftButton = 0;
                p_box->Flags |= GBxFlg_IsRPushed;
                ar_scroll_bar[ScrlBB_ArrowDn].Flags |= GBxFlg_IsRPushed;

                scroll_pos = p_box->ScrollBarPos + p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }
    }

    if (!lbDisplay.MLeftButton && !joy.Buttons[0])
        p_box->Flags &= ~(GBxFlg_IsRPushed|GBxFlg_IsPushed);
}

ubyte flashy_draw_purple_text_box(struct ScreenTextBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_flashy_draw_purple_text_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short box_w, box_h;
    short lines_visible;
    TbBool text_remains_dynamic; /**< text drawing callback never sets GBxFlg_TextCopied (maybe make this into a box flag?) */

    text_remains_dynamic = (p_box == &world_city_info_box) || (p_box == &equip_display_box) || (p_box == &cryo_cybmod_list_box);
    my_set_text_window_for_text_box(p_box);

    lbFontPtr = p_box->Font;
    byte_197160 = p_box->LineSpacing;
    if (p_box->LineHeight == 0)
        p_box->LineHeight = byte_197160 + my_char_height('A');

    lines_visible = get_text_box_window_lines_visible(p_box);

    box_w = p_box->Width - 1;
    box_h = p_box->Height - 1;

    if ((p_box->Flags & GBxFlg_Unkn0001) != 0)
    {
        int i;

        p_box->Timer = 0;
        p_box->Flags &= ~(GBxFlg_Unkn0001|GBxFlg_Unkn0080);
        p_box->Flags |= GBxFlg_Unkn0080;

        for (i = 0; i < 2; i++)
        {
            struct ScreenButton *p_button;
            struct ScreenInfoBox *p_info;

            p_button = p_box->Buttons[i];
            if (p_button != NULL)
                p_button->Flags |= GBxFlg_Unkn0001;

            p_info = p_box->Infos[i];
            if (p_info != NULL)
                p_info->Flags |= GBxFlg_Unkn0001;
        }
    }

    if ((p_box->Flags & GBxFlg_Unkn0002) != 0)
    {
        const char *p_text;
        int i;

        p_box->Flags &= ~(GBxFlg_Unkn0002|GBxFlg_Unkn0001);
        p_box->Timer = -1;

        p_text = p_box->Text;
        if (p_text != NULL)
            p_box->TextFadePos = my_str_len(p_text);
        else
            p_box->TextFadePos = -5;

        for (i = 0; i < 2; i++)
        {
            struct ScreenButton *p_button;
            struct ScreenInfoBox *p_info;

            p_button = p_box->Buttons[i];
            if (p_button != NULL)
                p_button->Flags |= GBxFlg_Unkn0002;

            p_info = p_box->Infos[i];
            if (p_info != NULL)
                p_info->Flags |= GBxFlg_Unkn0002;
        }
    }

    // Handle sound
    if (p_box->Timer <= 72)
    {
        if (p_box->Timer > 24)
        {
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_box->Timer <= 24)
    {
        short advance;

        advance = p_box->Timer;
        flashy_draw_projector_reaching_target_coords(p_box->X, p_box->Y, box_w, box_h, advance, p_box->Colour1);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw horizontal lines
    if (p_box->Timer <= 48)
    {
        short advance;

        advance = ((p_box->Timer - 24) * box_w) / 24;
        flashy_draw_projector_horizontal_lines(p_box->X, p_box->Y, box_w, box_h, advance, advance, 0, p_box->Colour1);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_box->Timer <= 72)
    {
        short advance;

        advance = ((p_box->Timer - 48) * box_h) / 24;
        flashy_draw_projector_vertical_lines(p_box->X, p_box->Y, box_w, box_h, advance, advance, 0, p_box->Colour1);
        p_box->Timer += p_box->DrawSpeed;
        return 2;
    }

    if ((p_box->Flags & GBxFlg_RadioBtn) == 0)
    {
        if ((p_box->Flags & GBxFlg_Unkn0080) != 0) {
            p_box->TextFadePos = -5;
            p_box->Flags &= ~GBxFlg_Unkn0080;
        }
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) != 0) && ((p_box->Flags & GBxFlg_Unkn0080) != 0))
    {
        if (p_box->Lines == 0) {
            p_box->Lines = my_count_lines(p_box->Text);
        }
        p_box->ScrollBarSize = p_box->ScrollWindowHeight * lines_visible / p_box->Lines;
        if ((p_box->ScrollBarSize < p_box->ScrollWindowHeight) || ((p_box->Flags & GBxFlg_IsMouseOver) != 0))
        {
            if (p_box->ScrollBarSize >= p_box->ScrollWindowHeight)
                p_box->ScrollBarSize = p_box->ScrollWindowHeight;
            p_box->ScrollBarPos = 0;
        }
        else
        {
            p_box->Flags &= ~GBxFlg_RadioBtn;
        }
        p_box->TextTopLine = 0;
        p_box->Flags &= ~(GBxFlg_IsPushed|GBxFlg_Unkn0080);
        if (p_box->Timer != 255) {
            p_box->TextFadePos = -5;
        }
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) == 0) &&
      (((p_box->Flags & GBxFlg_BkCopied) != 0) || ((p_box->Flags & GBxFlg_NoBkCopy) != 0)))
    {
        // If childern drawing has also finished, it is safe to cap the scrollbar hole
        if (((p_box->Flags & GBxFlg_TextCopied) != 0) || text_remains_dynamic)
        {
            if ((p_box->Width > 13) && (p_box->ScrollWindowHeight > 0) && (p_box->ScrollWindowOffset >= 0))
            {
                lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
                draw_box_purple_list(p_box->X + p_box->Width - 13,
                  p_box->Y + p_box->ScrollWindowOffset + 4,
                  8, p_box->ScrollWindowHeight + 2,
                  p_box->BGColour);
            }
        }
        flashy_draw_purple_text_box_text(p_box);
        flashy_draw_purple_text_box_children(p_box);
        return 3;
    }

    struct ScreenBox scroll_bar[ScrlBB_BoxesCount];

    init_scroll_bar_for_text_box(scroll_bar, p_box);

    input_purple_text_box_wth_scroll(p_box, scroll_bar);

    // Now as drawing animation finished, the background became static
    // Draw and store the static background
    if ((p_box->Flags & GBxFlg_BkCopied) == 0)
    {
        short w, h;
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        // main area, left of scrollbar
        w = max(p_box->Width - 16, 0);
        draw_box_purple_list(p_box->X - 3, p_box->Y - 3,
          w + 6, p_box->Height + 6, p_box->BGColour);
        // strip on right of scrollbar
        if (p_box->Width >= 5)
            draw_box_purple_list(p_box->X + p_box->Width - 5,  p_box->Y - 3,
              8, p_box->Height + 6, p_box->BGColour);
        if (p_box->Width > 13)
        {
            // above the scrollbar
            draw_box_purple_list(p_box->X + p_box->Width - 13, p_box->Y - 3,
              8, p_box->ScrollWindowOffset + 7, p_box->BGColour);
            // below the scrollbar
            h = max(p_box->ScrollWindowHeight + 3 + p_box->ScrollWindowOffset, 1);
            if (p_box->Height > h)
                draw_box_purple_list(p_box->X + p_box->Width - 13,
                  p_box->Y + h + 3,
                  8, p_box->Height - h,
                  p_box->BGColour);
        }
        // box outline
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X, p_box->Y, p_box->Width, p_box->Height, p_box->Colour1);
        lbDisplay.DrawFlags = 0;

        if ((p_box->Flags & GBxFlg_NoBkCopy) == 0)
        {
            copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
              p_box->Width + 6, p_box->Height + 6);
            p_box->Flags |= GBxFlg_BkCopied;
        }
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) != 0) && (p_box->ScrollBarPos >= 0))
    {
        struct TbSprite *p_spr;

        // Draw the scroll bar handle
        draw_box_purple_list(scroll_bar[ScrlBB_HndlBox].X, scroll_bar[ScrlBB_HndlBox].Y,
          scroll_bar[ScrlBB_HndlBox].Width, scroll_bar[ScrlBB_HndlBox].Height, 174);

        // Draw scroll arrows
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((scroll_bar[ScrlBB_ArrowUp].Flags & GBxFlg_IsRPushed) != 0)
            lbDisplay.DrawFlags = 0;
        if (p_box->Text != NULL)
            lbDisplay.DrawFlags |= 0x8000;

        p_spr = &fe_mouseptr_sprites[9];
        draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowUp].X, scroll_bar[ScrlBB_ArrowUp].Y, p_spr);

        lbDisplay.DrawFlags = 0;

        if (mouse_move_over_box(&scroll_bar[ScrlBB_ArrowUp]))
        {
            p_spr = &fe_mouseptr_sprites[13];
            draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowUp].X, scroll_bar[ScrlBB_ArrowUp].Y, p_spr);
        }

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((scroll_bar[ScrlBB_ArrowDn].Flags & GBxFlg_IsRPushed) != 0)
            lbDisplay.DrawFlags = 0;
        if (p_box->Text != NULL)
            lbDisplay.DrawFlags |= 0x8000;

        p_spr = &fe_mouseptr_sprites[10];
        draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowDn].X, scroll_bar[ScrlBB_ArrowDn].Y, p_spr);

        lbDisplay.DrawFlags = 0;

        if (mouse_move_over_box(&scroll_bar[ScrlBB_ArrowDn]))
        {
            p_spr = &fe_mouseptr_sprites[14];
            draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowDn].X, scroll_bar[ScrlBB_ArrowDn].Y, p_spr);
        }
    }

    flashy_draw_purple_text_box_text(p_box);
    flashy_draw_purple_text_box_children(p_box);
    return 3;
}

ubyte flashy_draw_purple_info_box(struct ScreenInfoBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_flashy_draw_purple_info_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short box_w, box_h;

    box_w = p_box->Width - 1;
    box_h = p_box->Height - 1;

    if ((p_box->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_box->Timer = -1;
        p_box->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
    }
    if ((p_box->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_box->Timer = 0;
        p_box->Flags &= ~(GBxFlg_Unkn0001|GBxFlg_Unkn0080);
        p_box->Flags |= GBxFlg_Unkn0080;
    }

    // Handle sound
    if (p_box->Timer <= 72)
    {
        if (p_box->Timer > 24)
        {
            // Projector drawing sound
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_box->Timer <= 24)
    {
        short advance;

        advance = p_box->Timer;
        flashy_draw_projector_reaching_target_coords(p_box->X, p_box->Y, box_w, box_h, advance, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw horizontal lines
    if (p_box->Timer <= 48)
    {
        short advance;

        advance = ((p_box->Timer - 24) * box_w) / 24;
        flashy_draw_projector_horizontal_lines(p_box->X, p_box->Y, box_w, box_h, advance, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_box->Timer <= 72)
    {
        short advance;

        advance = ((p_box->Timer - 48) * box_h) / 24;
        flashy_draw_projector_vertical_lines(p_box->X, p_box->Y, box_w, box_h, advance, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 2;
    }

    // Now as drawing animation finished, the background became static
    // Draw and store the static background
    if ((p_box->Flags & GBxFlg_BkCopied) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X - 1, p_box->Y - 1,
          p_box->Width + 2, p_box->Height + 2, p_box->BGColour);
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X, p_box->Y,
          p_box->Width, p_box->Height, p_box->Colour);
        lbDisplay.DrawFlags = 0;

        if ((p_box->Flags & GBxFlg_NoBkCopy) == 0)
        {
            copy_box_purple_list(p_box->X - 1, p_box->Y - 1,
              p_box->Width + 2, p_box->Height + 2);
            p_box->Flags |= GBxFlg_BkCopied;
        }
    }
    if (p_box->DrawTextFn != NULL) {
            ubyte drawn;
            //p_box->DrawTextFn(p_box); -- incompatible calling convention
            asm volatile ("call *%2\n"
                : "=r" (drawn) : "a" (p_box), "g" (p_box->DrawTextFn));
    }
    return 3;
}

ubyte flashy_draw_purple_button(struct ScreenButton *p_btn)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_flashy_draw_purple_button\n"
        : "=r" (ret) : "a" (p_btn));
    return ret;
#else
    short box_w, box_h;
    TbKeyCode akey;
    TbBool mouse_over, event_from_key;

    my_set_text_window(p_btn->X + 2, p_btn->Y + 2, p_btn->Width - 18, p_btn->Height - 4);
    lbFontPtr = p_btn->Font;
    box_w = p_btn->Width - 1;
    box_h = p_btn->Height - 1;

    if ((p_btn->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_btn->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
        p_btn->Timer = -1;
        if (p_btn->Text != NULL)
            p_btn->TextFadePos = strlen(p_btn->Text);
        else
            p_btn->TextFadePos = -5;
    }
    if ((p_btn->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_btn->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0001);
        p_btn->Timer = 0;
        p_btn->Flags |= GBxFlg_Unkn0080;
    }

    // Handle sound
    if (p_btn->Timer <= 72)
    {
        if (p_btn->Timer > 24)
        {
            // Projector drawing sound
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_btn->Timer <= 24)
    {
        short advance;

        advance = p_btn->Timer;
        flashy_draw_projector_reaching_target_coords(p_btn->X, p_btn->Y, box_w, box_h, advance, p_btn->Colour);
        p_btn->Timer += p_btn->DrawSpeed;
        return 0;
    }

    if (!mouse_move_over_box(p_btn))
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;

    // Draw horizontal lines
    if (p_btn->Timer <= 48)
    {
        short advance, advance_p, delta_p;

        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
            delta_p = 5;
        else
            delta_p = 0;
        advance = ((p_btn->Timer - 24) * (box_w)) / 24;
        advance_p = ((p_btn->Timer - 24) * (box_w - delta_p)) / 24;
        flashy_draw_projector_horizontal_lines(p_btn->X, p_btn->Y, box_w, box_h, advance, advance_p, delta_p, p_btn->Colour);
        p_btn->Timer += p_btn->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_btn->Timer <= 72)
    {
        short advance, advance_p, delta_p;

        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
            delta_p = 5;
        else
            delta_p = 0;
        advance = ((p_btn->Timer - 48) * (box_h)) / 24;
        advance_p = ((p_btn->Timer - 48) * (box_h - delta_p)) / 24;
        flashy_draw_projector_vertical_lines(p_btn->X, p_btn->Y, box_w, box_h, advance, advance_p, delta_p, p_btn->Colour);
        p_btn->Timer += p_btn->DrawSpeed;
        return 2;
    }

    // Mouse input
    if (mouse_down_over_box(p_btn))
    {
        if (lbDisplay.LeftButton)
        {
            lbDisplay.LeftButton = 0;
            p_btn->Flags |= GBxFlg_IsPushed;
        }
        if (lbDisplay.RightButton)
        {
            lbDisplay.RightButton = 0;
            p_btn->Flags |= GBxFlg_IsRPushed;
        }
    }
    event_from_key = false;
    akey = p_btn->AccelKey  & 0xFF;
    if ((akey == KC_RETURN || akey == KC_ESCAPE) && is_key_pressed(akey, KMod_DONTCARE))
    {
        clear_key_pressed(akey);
        p_btn->Flags |= GBxFlg_IsPushed;
        event_from_key = true;
    } else
    if (is_key_pressed(akey, KMod_ALT))
    {
        clear_key_pressed(akey);
        event_from_key = true;
        p_btn->Flags |= GBxFlg_IsPushed;
    } else
    if (is_key_pressed(akey, KMod_CONTROL))
    {
        clear_key_pressed(akey);
        p_btn->Flags |= GBxFlg_IsRPushed;
        event_from_key = true;
    }

    mouse_over = mouse_move_over_box(p_btn);
    if (mouse_over)
    {
        if ((p_btn->Flags & GBxFlg_IsMouseOver) == 0)
        {
            // Mouse over sound
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
            p_btn->Flags |= GBxFlg_IsMouseOver;
        }
    }
    else
    {
        p_btn->Flags &= ~GBxFlg_IsMouseOver;

    }

    ubyte box_b;
    ubyte ret;

    ret = 3;
    box_b = p_btn->Border;

    if (!mouse_over && !event_from_key)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((p_btn->Flags & GBxFlg_RadioBtn) != 0 && p_btn->RadioValue == *p_btn->Radio)
            lbDisplay.DrawFlags = 0;
        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
        {
            int v15, v16;

            v15 = box_b;
            v16 = box_w;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              box_b + (box_w) - 4, box_b + 5, p_btn->BGColour);
            lbDisplay.DrawFlags |= 0x8000;
            draw_box_purple_list(p_btn->X - v15, p_btn->Y + 5,
              box_w + 1 + 2 * v15, v15 + box_h - 4, p_btn->BGColour);
            lbDisplay.DrawFlags &= ~0x8000;
            draw_triangle_purple_list(v16 + p_btn->X - 4, p_btn->Y - v15,
              v16 + p_btn->X + 1 + v15, p_btn->Y + 5,
              v16 + p_btn->X - 4, p_btn->Y + 5, p_btn->BGColour);
            lbDisplay.DrawFlags |= Lb_SPRITE_OUTLINE;
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X, box_h + p_btn->Y,
              p_btn->X + box_w, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X + box_w - 5, p_btn->Y, p_btn->Colour);
            draw_line_purple_list(p_btn->X + v16, p_btn->Y + 5,
              p_btn->X + v16, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X + v16 - 5, p_btn->Y,
              p_btn->X + v16, p_btn->Y + 5, p_btn->Colour);
        }
        else
        {
            int v17, v18;
            lbDisplay.DrawFlags |= 0x8000;
            v17 = box_h + 1;
            v18 = box_w + 1;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b, v18 + 2 * box_b, 2 * box_b + v17, p_btn->BGColour);
            lbDisplay.DrawFlags &= ~(Lb_SPRITE_OUTLINE|0x8000);
            lbDisplay.DrawFlags |= Lb_SPRITE_OUTLINE;
            draw_box_purple_list(p_btn->X, p_btn->Y, v18, v17, p_btn->Colour);
        }
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((p_btn->Flags & (GBxFlg_IsPushed|GBxFlg_IsRPushed)) != 0)
            p_btn->Flags &= ~(GBxFlg_IsPushed|GBxFlg_IsRPushed);
        if (((p_btn->Flags & GBxFlg_RadioBtn) != 0) && (p_btn->RadioValue == *p_btn->Radio))
            lbDisplay.DrawFlags = 0;
    }
    else
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((p_btn->Flags & (GBxFlg_IsPushed|GBxFlg_IsRPushed)) != 0)
            lbDisplay.DrawFlags = 0;
        if ((p_btn->Flags & GBxFlg_RadioBtn) != 0 && p_btn->RadioValue == *p_btn->Radio)
            lbDisplay.DrawFlags = 0;
        if (lbDisplay.MLeftButton || joy.Buttons[0])
        {
            lbDisplay.LeftButton = 0;
            ret = 4;
            lbDisplay.DrawFlags = 0;
            p_btn->Flags |= GBxFlg_IsPushed;
        }
        else if ((p_btn->Flags & GBxFlg_IsPushed) != 0)
        {
            if (!show_alert || p_btn == &alert_OK_button)
            {
                ushort smpl_id;

                if (p_btn->CallBackFn != NULL)
                {
                    ubyte clicked;
                    //p_btn->CallBackFn(0); -- incompatible calling convention
                    asm volatile ("call *%2\n"
                      : "=r" (clicked) : "a" (0), "g" (p_btn->CallBackFn));
                    if (clicked)
                        smpl_id = 111;
                    else
                        smpl_id = 129;
                }
                else
                {
                    if ((p_btn->Flags & GBxFlg_RadioBtn) != 0)
                        // Click sound for do action button
                        smpl_id = 111;
                    else
                        // Click sound for cancel / abort button
                        smpl_id = 129;
                }
                play_sample_using_heap(0, smpl_id, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);

                if ((p_btn->Flags & GBxFlg_RadioBtn) != 0)
                    *p_btn->Radio = p_btn->RadioValue;
            }
            p_btn->Flags &= ~GBxFlg_IsPushed;
        }

        if (lbDisplay.MRightButton)
        {
            lbDisplay.RightButton = 0;
            ret = 5;
            lbDisplay.DrawFlags = 0;
            p_btn->Flags |= GBxFlg_IsRPushed;
        }
        else if ((p_btn->Flags & GBxFlg_IsRPushed) != 0)
        {
            if (!show_alert || p_btn == &alert_OK_button)
            {
                ushort smpl_id;

                if (p_btn->CallBackFn != NULL)
                {
                    ubyte clicked;
                    //clicked = p_btn->CallBackFn(1); -- incompatible calling convention
                    asm volatile ("call *%2\n"
                      : "=r" (clicked) : "a" (1), "g" (p_btn->CallBackFn));
                    if (clicked)
                        smpl_id = 111;
                    else
                        smpl_id = 129;
                }
                else
                {
                    // Click sound for cancel / abort button
                    smpl_id = 129;
                }
                play_sample_using_heap(0, smpl_id, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);

                if ((p_btn->Flags & GBxFlg_RadioBtn) != 0)
                  *p_btn->Radio = p_btn->RadioValue;
            }
            p_btn->Flags &= ~GBxFlg_IsRPushed;
        }
        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
        {
            int v13, v14;

            if ((p_btn->Flags & GBxFlg_Unkn0020) != 0)
              lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            v13 = box_b;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              box_b + (box_w) - 4, box_b + 5, p_btn->BGColour);
            lbDisplay.DrawFlags |= 0x8000;
            draw_box_purple_list(p_btn->X - v13, p_btn->Y + 5,
              2 * v13 + box_w + 1, box_h - 4 + v13, p_btn->BGColour);
            lbDisplay.DrawFlags &= ~0x8000;
            draw_triangle_purple_list(p_btn->X + (box_w) - 4, p_btn->Y - v13,
              v13 + p_btn->X + (box_w) + 1, p_btn->Y + 5,
              p_btn->X + (box_w) - 4, p_btn->Y + 5, p_btn->BGColour);
            lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
            if ((p_btn->Flags & GBxFlg_Unkn0040) != 0)
              lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            v14 = box_w;
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X, box_h + p_btn->Y,
              p_btn->X + (box_w), box_h + p_btn->Y, p_btn->Colour);
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X + (box_w) - 5, p_btn->Y, p_btn->Colour);
            draw_line_purple_list(v14 + p_btn->X, p_btn->Y + 5,
              v14 + p_btn->X, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(v14 + p_btn->X - 5, p_btn->Y,
              v14 + p_btn->X, p_btn->Y + 5, p_btn->Colour);
            lbDisplay.DrawFlags = 0;
        }
        else
        {
            if ((p_btn->Flags & GBxFlg_Unkn0020) != 0)
                lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            lbDisplay.DrawFlags |= 0x8000;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              2 * box_b + (box_w) + 1, box_h + 1 + 2 * box_b, p_btn->BGColour);
            lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
            if ((p_btn->Flags & GBxFlg_Unkn0040) != 0)
              lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            draw_box_purple_list(p_btn->X, p_btn->Y, box_w + 1, box_h + 1, p_btn->Colour);
            lbDisplay.DrawFlags = 0;
        }
    }

    if ((p_btn->Flags & GBxFlg_TextRight) != 0)
    {
          lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_RIGHT;
    }
    else if ((p_btn->Flags & GBxFlg_TextCenter) != 0)
    {
          lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    }
    if (p_btn->DrawTextFn != NULL) {
        ubyte drawn;
        //p_btn->DrawTextFn(p_btn); -- incompatible calling convention
        asm volatile ("call *%2\n"
          : "=r" (drawn) : "a" (p_btn), "g" (p_btn->DrawTextFn));
    }
    lbDisplay.DrawFlags = 0;
    return ret;
#endif
}

ubyte label_text(struct ScreenButton *p_btn)
{
    short text_x, text_y;
    short text_w, text_h;

    lbFontPtr = p_btn->Font;
    text_w = my_string_width(p_btn->Text);
    text_h = my_char_height('A');
    text_x = p_btn->X + ((p_btn->Width - text_w) >> 1) + 1;
    text_y = p_btn->Y + ((p_btn->Height - text_h) >> 1);
    my_set_text_window(text_x, text_y, lbDisplay.GraphicsScreenWidth, text_y + text_h);
    if (p_btn->Flags & 0x80) {
        p_btn->TextFadePos = -3;
        p_btn->Flags &= ~GBxFlg_Unkn0080;
    }
    return flashy_draw_text(0, 0, p_btn->Text, p_btn->TextSpeed, p_btn->TextTopLine,
      &p_btn->TextFadePos, 0);
}

ubyte button_text(struct ScreenButton *p_btn)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_button_text\n"
        : "=r" (ret) : "a" (p_btn));
    return ret;
#endif
    short text_x, text_y;
    short text_w, text_h;

    lbFontPtr = p_btn->Font;
    text_w = my_string_width(p_btn->Text);
    text_h = my_char_height('A');
    text_x = p_btn->X + ((p_btn->Width - text_w) >> 1) + 1;
    text_y = p_btn->Y + ((p_btn->Height - text_h) >> 1);
    my_set_text_window(text_x, text_y, lbDisplay.GraphicsScreenWidth, text_y + text_h);
    if (p_btn->Flags & 0x80) {
        p_btn->TextFadePos = -3;
        p_btn->Flags &= ~GBxFlg_Unkn0080;
    }
    return flashy_draw_text(0, 0, p_btn->Text, p_btn->TextSpeed, p_btn->TextTopLine,
      &p_btn->TextFadePos, 1);
}

ubyte info_box_text(struct ScreenInfoBox *p_box)
{
    ubyte ret;
    asm volatile ("call ASM_info_box_text\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
}

void draw_triangle_purple_list(int x1, int y1, int x2, int y2, int x3, int y3, TbPixel colour)
{
    asm volatile (
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_draw_triangle_purple_list\n"
        : : "a" (x1), "d" (y1), "b" (x2), "c" (y2), "g" (x3), "g" (y3), "g" (colour));
}

void draw_flic_purple_list(void (*fn)())
{
#if 0
    asm volatile ("call ASM_draw_flic_purple_list\n"
        : : "a" (fn));
    return;
#endif
    struct PurpleDrawItem *pditem;

    pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    pditem->U.Flic.Function = fn;
    pditem->U.Flic.Colour = lbDisplay.DrawColour;
    pditem->Flags = lbDisplay.DrawFlags;
    pditem->Type = PuDT_FLIC;
}

void draw_noise_box_purple_list(int x, int y, ulong width, ulong height)
{
    struct PurpleDrawItem *pditem;

    draw_box_purple_list(x, y, width, height, 0);
    pditem = &purple_draw_list[purple_draw_index - 1];
    pditem->Type = PuDT_NOISEBOX;
}

/******************************************************************************/
