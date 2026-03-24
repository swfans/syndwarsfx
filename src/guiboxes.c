/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file guiboxes.c
 *     GUI boxes for menu system.
 * @par Purpose:
 *     Implement functions for handling various boxes for game GUI.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 22 Oct 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "guiboxes.h"

#include "bfkeybd.h"
#include "bfline.h"
#include "bfscreen.h"
#include "bftext.h"

#include "display.h"
#include "mydraw.h"
#include "purpldrw.h"
#include "swlog.h"
/******************************************************************************/

TbBool in_box(ScrCoord x, ScrCoord y, ScrCoord box_x, ScrCoord box_y, short box_w, short box_h)
{
    return x > box_x && x < box_x + box_w
        && y > box_y && y < box_y + box_h;
}

TbBool in_box_coords(ScrCoord x, ScrCoord y, ScrCoord box_x1, ScrCoord box_y1, ScrCoord box_x2, ScrCoord box_y2)
{
    return x > box_x1 && x < box_x2
        && y > box_y1 && y < box_y2;
}

TbBool over_box(ScrCoord x, ScrCoord y, ScrCoord box_x, ScrCoord box_y, short box_w, short box_h)
{
    return x >= box_x && x <= box_x + box_w
        && y >= box_y && y <= box_y + box_h;
}

TbBool over_box_coords(ScrCoord x, ScrCoord y, ScrCoord box_x1, ScrCoord box_y1, ScrCoord box_x2, ScrCoord box_y2)
{
    return x >= box_x1 && x <= box_x2
        && y >= box_y1 && y <= box_y2;
}

TbBool boxes_intersect(ScrCoord box1_x, ScrCoord box1_y, short box1_w, short box1_h,
  ScrCoord box2_x, ScrCoord box2_y, short box2_w, short box2_h)
{
    if ((box1_x + box1_w < box2_x) || (box1_x > box2_x + box2_w))
        return false;
    if ((box1_y + box1_h < box2_y) || (box1_y > box2_y + box2_h))
        return false;
    return true;
}

TbBool base_boxes_intersect(struct ScreenBoxBase *p_box1, struct ScreenBoxBase *p_box2)
{
    if ((p_box1->X + p_box1->Width < p_box2->X) || (p_box1->X > p_box2->X + p_box2->Width))
        return false;
    if ((p_box1->Y + p_box1->Height < p_box2->Y) || (p_box1->Y > p_box2->Y + p_box2->Height))
        return false;
    return true;
}

TbBool mouse_move_over_box_coords(ScrCoord box_x1, ScrCoord box_y1, ScrCoord box_x2, ScrCoord box_y2)
{
    ScrCoord ms_x, ms_y;

    ms_x = lbDisplay.MMouseX;
    ms_y = lbDisplay.MMouseY;
    return over_box_coords(ms_x, ms_y, box_x1, box_y1, box_x2, box_y2);
}

TbBool mouse_down_over_box_coords(ScrCoord box_x1, ScrCoord box_y1, ScrCoord box_x2, ScrCoord box_y2)
{
    ScrCoord ms_x, ms_y;

    ms_x = lbDisplay.MouseX;
    ms_y = lbDisplay.MouseY;
    return over_box_coords(ms_x, ms_y, box_x1, box_y1, box_x2, box_y2);
}

// TODO switch order of arguments and to above function, remove when this is no longer used
TbBool mouse_move_over_rect(ScrCoord box_x1, ScrCoord box_x2, ScrCoord box_y1, ScrCoord box_y2)
{
    ScrCoord ms_x, ms_y;

    ms_x = lbDisplay.MMouseX;
    ms_y = lbDisplay.MMouseY;
    return over_box_coords(ms_x, ms_y, box_x1, box_y1, box_x2, box_y2);
}

TbBool mouse_move_over_rect_adv(ScrCoord x1, ScrCoord y1, short width, short height, TbBool a5)
{
    ScrCoord ms_x, ms_y;
    short dx, dy;

    ms_x = lbDisplay.MMouseX;
    ms_y = lbDisplay.MMouseY;

    if (a5)
    {
        if ((ms_y >= y1) && (ms_y < y1 + height))
        {
            dx = ms_x - x1;
            dy = ms_y - y1;
            if ((dx >= dy) && (dx - dy < width))
                return 1;
        }
    }
    else
    {
        if ((ms_y >= y1) && (ms_y < y1 + height))
        {
            dx = ms_x - x1;
            dy = ms_y - y1;
            if ((dx >= -dy) && (dx + dy < width))
                return 1;
        }
    }
    return 0;
}

short mouse_move_position_horizonal_over_bar_coords(ScrCoord x, short w)
{
    ScrCoord ms_x;

    ms_x = lbDisplay.MMouseX;
    return (ms_x - x);
}

short mouse_move_position_vertical_over_bar_coords(ScrCoord y, short h)
{
    ScrCoord ms_y;

    ms_y = lbDisplay.MMouseY;
    return (ms_y - y);
}

short mouse_down_position_horizonal_over_bar_coords(ScrCoord x, short w)
{
    ScrCoord ms_x;

    ms_x = lbDisplay.MouseX;
    return (ms_x - x);
}

TbBool is_over_box_base(ScrCoord x, ScrCoord y, struct ScreenBoxBase *p_box)
{
    return (x >= p_box->X) && (x <= p_box->X + p_box->Width)
        && (y >= p_box->Y) && (y <= p_box->Y + p_box->Height);
}

TbBool is_over_slant_box_base(ScrCoord x, ScrCoord y, struct ScreenBoxBase *p_box)
{
    if ((x >= p_box->X) && (x <= p_box->X + p_box->Width + p_box->Height)
        && (y >= p_box->Y) && (y <= p_box->Y + p_box->Height))
    {
        short dx, dy, hh, hw;
        hh = p_box->Height >> 1;
        hw = p_box->Height + p_box->Width;
        dx = x - p_box->X;
        dy = y - p_box->Y;
        return (dy + dx >= hh) && (dy + dx <= hw);
    }
    return false;
}

TbBool mouse_move_over_box_base(struct ScreenBoxBase *p_box)
{
    ScrCoord ms_x, ms_y;

    ms_x = lbDisplay.MMouseX;
    ms_y = lbDisplay.MMouseY;
    return is_over_box_base(ms_x, ms_y, p_box);
}

TbBool mouse_down_over_box_base(struct ScreenBoxBase *p_box)
{
    ScrCoord ms_x, ms_y;

    ms_x = lbDisplay.MouseX;
    ms_y = lbDisplay.MouseY;
    return is_over_box_base(ms_x, ms_y, p_box);
}

TbBool mouse_move_over_slant_box_base(struct ScreenBoxBase *p_box)
{
    ScrCoord ms_x, ms_y;

    ms_x = lbDisplay.MMouseX;
    ms_y = lbDisplay.MMouseY;
    return is_over_slant_box_base(ms_x, ms_y, p_box);
}

short mouse_move_position_horizonal_over_box_base(struct ScreenBoxBase *p_box)
{
    return mouse_move_position_horizonal_over_bar_coords(p_box->X, p_box->Width);
}

short mouse_move_position_vertical_scrollbar_over_text_box(struct ScreenTextBox *p_box)
{
    short pos;

    pos = mouse_move_position_vertical_over_bar_coords(p_box->GrabPos, p_box->Height);
    if (pos < 0)
        pos = 0;
    else if (pos + p_box->ScrollBarSize > p_box->ScrollWindowHeight)
        pos = p_box->ScrollWindowHeight - p_box->ScrollBarSize;
    return pos;
}

short mouse_move_y_coord_over_box_base(struct ScreenBoxBase *box)
{
    ScrCoord ms_y;

    ms_y = lbDisplay.MMouseY;
    return ms_y - box->Y;
}

TbResult ApDrawSlantBox(ScrCoord x, ScrCoord y, ushort w, ushort h, TbPixel col)
{
    short cx, cy;
    short ch;

    cx = x;
    cy = y;
    for (ch = h; ch > 0; ch--)
    {
        LbDrawLine(cx, cy, cx + w, cy, col);
        --cx;
        ++cy;
    }
    return Lb_SUCCESS;
}

void init_screen_box(struct ScreenBox *p_box, ScrCoord x, ScrCoord y, ushort width, ushort height, int drawspeed)
{
#if 0
    asm volatile (
      "push %5\n"
      "push %4\n"
      "call ASM_init_screen_box\n"
        : : "a" (p_box), "d" (x), "b" (y), "c" (width), "g" (height), "g" (drawspeed));
#endif
    p_box->Flags = GBxFlg_Unkn0001;
    p_box->DrawFn = flashy_draw_purple_box;
    p_box->SpecialDrawFn = NULL;
    p_box->Colour = 0xF7;
    p_box->BGColour = 0x38;
    p_box->X = x;
    p_box->Y = y;
    p_box->Width = width;
    p_box->Height = height;
    p_box->DrawSpeed = drawspeed;
}

void init_screen_text_box(struct ScreenTextBox *p_box, ScrCoord x, ScrCoord y,
  ushort width, ushort height, int drawspeed, struct TbSprite *p_font, ushort textspeed)
{
#if 0
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_init_screen_text_box\n"
        : : "a" (box), "d" (x), "b" (y), "c" (width), "g" (height), "g" (drawspeed), "g" (p_font), "g" (textspeed));
#endif
    p_box->Flags = GBxFlg_Unkn0001;
    p_box->DrawFn = flashy_draw_purple_text_box;
    p_box->Text = 0;
    p_box->LineSpacing = 4;
    p_box->DrawTextFn = NULL;
    p_box->Buttons[0] = 0;
    p_box->Infos[0] = 0;
    p_box->Infos[1] = 0;
    p_box->Colour1 = 0xF7;
    p_box->BGColour = 0x38;
    p_box->ScrollWindowOffset = 0;
    p_box->LineHeight = 0;
    p_box->Lines = 0;
    p_box->X = x;
    p_box->Y = y;
    p_box->Height = height;
    p_box->DrawSpeed = drawspeed;
    p_box->Font = p_font;
    p_box->TextSpeed = textspeed;
    p_box->Width = width;
    p_box->ScrollWindowHeight = p_box->Height - 31;
}

void init_screen_button(struct ScreenButton *p_box, ScrCoord x, ScrCoord y,
  const char *text, int drawspeed, struct TbSprite *p_font, int textspeed, int flags)
{
#if 0
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_init_screen_button\n"
        : : "a" (p_box), "d" (x), "b" (y), "c" (text), "g" (drawspeed), "g" (p_font), "g" (textspeed), "g" (flags));
#endif
    lbFontPtr = p_font;
    p_box->Y = y;
    p_box->Width = my_string_width(text) + 4;
    p_box->Height = my_char_height('A') + 6;
    p_box->DrawSpeed = drawspeed;
    p_box->Font = p_font;
    p_box->Flags = flags | GBxFlg_Unkn0001;
    if ((flags & 0x80) != 0)
        x -= p_box->Width;
    p_box->X = x;
    p_box->DrawFn = flashy_draw_purple_button;
    p_box->DrawTextFn = button_text;
    p_box->CallBackFn = NULL;
    p_box->Border = 1;
    p_box->Colour = 0xAE;
    p_box->BGColour = 0xF3;
    p_box->AccelKey = 0;
    p_box->Text = text;
    p_box->TextSpeed = textspeed;
    if (*text != '\0')
        p_box->AccelKey = lbAsciiToInkey[*(ubyte *)text];
    p_box->Radio = 0;
    p_box->RadioValue = 0;
}

void init_screen_info_box(struct ScreenInfoBox *p_box, ScrCoord x, ScrCoord y, ushort width,
  char *text1, char *text2, int drawspeed, struct TbSprite *p_font1, struct TbSprite *p_font2, int textspeed)
{
#if 0
    asm volatile (
      "push %9\n"
      "push %8\n"
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_init_screen_info_box\n"
        : : "a" (p_box), "d" (x), "b" (y), "c" (width), "g" (text1), "g" (text2), "g" (drawspeed), "g" (font1), "g" (font2), "g" (textspeed));
#endif
    short font1_h, font2_h, font_h;

    lbFontPtr = p_font1;
    font1_h = my_char_height('A');
    lbFontPtr = p_font2;
    font2_h = my_char_height('A');
    if (font1_h <= font2_h)
        font_h = font2_h;
    else
        font_h = font1_h;

    p_box->X = x;
    p_box->Y = y;
    p_box->Width = width;
    p_box->Height = font_h + 6;
    p_box->Flags = GBxFlg_Unkn0001;
    p_box->DrawFn = flashy_draw_purple_info_box;
    p_box->DrawTextFn = info_box_text;
    p_box->Colour = 0xF7;
    p_box->BGColour = 0x38;
    p_box->DrawSpeed = drawspeed;
    p_box->Font1 = p_font1;
    p_box->Font2 = p_font2;
    p_box->Text1 = text1;
    p_box->Text2 = text2;
    p_box->TextSpeed = textspeed;
}

void init_screen_shape(struct ScreenShape *p_shp, ScrCoord x, ScrCoord y,
  short *pts_w, short *pts_h, ushort pts_len, short scale_w, short scale_h,
  short drawspeed)
{
    ushort k;

    for (k = 0; k < SCREENSHAPE_POINTS_MAX; k++)
    {
        if (k < pts_len)
        {
            p_shp->PtX[k] = x + ((pts_w[k] * scale_w) >> 8);
            p_shp->PtY[k] = y + ((pts_h[k] * scale_h) >> 8);
        }
        else
        {
            // Fill remaining points by repeating the previous ones;
            // the drawing function may use first of these points
            p_shp->PtX[k] = p_shp->PtX[k - pts_len];
            p_shp->PtY[k] = p_shp->PtY[k - pts_len];
        }
    }
    p_shp->DrawSpeed = drawspeed;
    p_shp->Timer = 0;
    p_shp->Flags = GBxFlg_Unkn0001;
    p_shp->Colour = 0xF7;
    p_shp->PtLen = pts_len;
}

/******************************************************************************/
