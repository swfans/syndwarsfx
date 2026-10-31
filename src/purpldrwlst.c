/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file purpldrwlst.c
 *     Drawlists handling for the purple projector screens.
 * @par Purpose:
 *     Implements functions for filling and using drawlists.
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
#include "purpldrwlst.h"

#include <stdlib.h>
#include "bfjoyst.h"
#include "bflib_vidraw.h"
#include "bfbox.h"
#include "bfline.h"
#include "bfmouse.h"
#include "bftext.h"
#include "bftringl.h"
#include "bfscreen.h"
#include "bfscrcopy.h"
#include "bfsprite.h"
#include "poly.h"

#include "display.h"
#include "guiboxes.h"
#include "mydraw.h"
#include "swlog.h"
/******************************************************************************/

struct ScreenPoint proj_origin = {319, 269};

struct PurpleDrawItem *purple_draw_list = NULL;
ushort purple_draw_index = 0;

struct ScreenPoint *hotspot_buffer = NULL;
ushort hotspot_next = 1;

ubyte purple_joy_move = 0;

/******************************************************************************/

ushort find_closest_hotspot_down(void)
{
    ulong hmin;
    short imin, i;

    hmin = 0x80000000;
    imin = 0;
    for (i = 1; i < hotspot_next; i++)
    {
        short ms_x, ms_y;
        short shift_w, shift_h;

        ms_x = lbDisplay.MMouseX;
        ms_y = lbDisplay.MMouseY;
        shift_w = hotspot_buffer[i].X - ms_x;
        shift_h = hotspot_buffer[i].Y - ms_y;
        if ((shift_h > 0) && (shift_h > abs(shift_w)))
        {
            ulong hcur;
            if (shift_h <= abs(shift_w))
                hcur = (shift_h >> 1) + abs(shift_w);
            else
                hcur = shift_h + (abs(shift_w) >> 1);
            if ((hcur < hmin) && (hcur != 0)) {
                hmin = hcur;
                imin = i;
            }
        }
    }
    if (hmin == 0x80000000)
        return 0;
    return imin;
}

ushort find_closest_hotspot_up(void)
{
    ulong hmin;
    short imin, i;

    hmin = 0x80000000;
    imin = 0;
    for (i = 1; i < hotspot_next; i++)
    {
        short ms_x, ms_y;
        short shift_w, shift_h;

        ms_x = lbDisplay.MMouseX;
        ms_y = lbDisplay.MMouseY;
        shift_w = hotspot_buffer[i].X - ms_x;
        shift_h = ms_y - hotspot_buffer[i].Y;
        if ((shift_h > 0) && (shift_h > abs(shift_w)))
        {
            ulong hcur;
            if (shift_h <= abs(shift_w))
                hcur = (shift_h >> 1) + abs(shift_w);
            else
                hcur = shift_h + (abs(shift_w) >> 1);
            if ((hcur < hmin) && (hcur != 0)) {
                hmin = hcur;
                imin = i;
            }
        }
    }
    if (hmin == 0x80000000)
        return 0;
    return imin;
}

ushort find_closest_hotspot_right(void)
{
    ulong hmin;
    short imin, i;

    hmin = 0x80000000;
    imin = 0;
    for (i = 1; i < hotspot_next; i++)
    {
        short ms_x, ms_y;
        short shift_w, shift_h;

        ms_x = lbDisplay.MMouseX;
        ms_y = lbDisplay.MMouseY;
        shift_w = hotspot_buffer[i].X - ms_x;
        shift_h = hotspot_buffer[i].Y - ms_y;
        if ((shift_w > 0) && (shift_w > abs(shift_h)))
        {
            ulong hcur;
            if (abs(shift_h) <= shift_w)
                hcur = shift_w + (abs(shift_h) >> 1);
            else
                hcur = (shift_w >> 1) + abs(shift_h);
            if ((hcur < hmin) && (hcur != 0)) {
                hmin = hcur;
                imin = i;
            }
        }
    }
    if (hmin == 0x80000000)
        return 0;
    return imin;
}

ushort find_closest_hotspot_left(void)
{
    ulong hmin;
    short imin, i;

    hmin = 0x80000000;
    imin = 0;
    for (i = 1; i < hotspot_next; i++)
    {
        short ms_x, ms_y;
        short shift_w, shift_h;

        ms_x = lbDisplay.MMouseX;
        ms_y = lbDisplay.MMouseY;
        shift_w = ms_x - hotspot_buffer[i].X;
        shift_h = hotspot_buffer[i].Y - ms_y;
        if ((shift_w > 0) && (shift_w > abs(shift_h)))
        {
            ulong hcur;
            if (abs(shift_h) <= shift_w)
                hcur = shift_w + (abs(shift_h) >> 1);
            else
                hcur = (shift_w >> 1) + abs(shift_h);
            if ((hcur < hmin) && (hcur != 0)) {
                hmin = hcur;
                imin = i;
            }
        }
    }
    if (hmin == 0x80000000)
        return 0;
    return imin;
}

void input_screen_hotspots(void)
{
    ushort hs;

    hs = 0;
    if (purple_joy_move)
    {
        if ((joy.DigitalY[0] == 0) && (joy.DigitalX[0] == 0))
            purple_joy_move = 0;
    }
    else if (joy.DigitalY[0] == 1)
    {
        hs = find_closest_hotspot_down();
        purple_joy_move = 1;
    }
    else if (joy.DigitalY[0] == -1)
    {
        hs = find_closest_hotspot_up();
        purple_joy_move = 1;
    }
    else if (joy.DigitalX[0] == 1)
    {
        hs = find_closest_hotspot_right();
        purple_joy_move = 1;
    }
    else if (joy.DigitalX[0] == -1)
    {
        hs = find_closest_hotspot_left();
        purple_joy_move = 1;
    }

    if (hs > 0) {
        LbMouseSetPosition(hotspot_buffer[hs].X, hotspot_buffer[hs].Y);
    }
}

void screen_hotspots_clear(void)
{
    hotspot_next = 1;
    hotspot_buffer[0].X = lbDisplay.GraphicsScreenWidth / 2;
    hotspot_buffer[0].Y = lbDisplay.GraphicsScreenHeight / 2;
}

void screen_hotspot_add(int x, int y)
{
    ushort hs;
    hs = hotspot_next;
    if (hs + 1 > hotspot_buffer_len)
        return;
    hotspot_next++;
    hotspot_buffer[hs].X = x;
    hotspot_buffer[hs].Y = y;
}

static void draw_holo_box(struct DIBox *p_pdbox)
{
    short x, y;
    short w, h;

    x = p_pdbox->X;
    y = p_pdbox->Y;
    w = p_pdbox->Width;
    h = p_pdbox->Height;

    LbDrawBox(x, y, w, h, p_pdbox->Colour);

    if ((lbDisplay.DrawFlags & 0x8000) != 0)
    {
        short shift_w, shift_h;

        shift_w = (w >> 1);
        shift_h = (h >> 1);
        screen_hotspot_add(x + shift_w, y + shift_h);
    }
}

static void draw_holo_text(struct DIText *p_pdtext)
{
    short x, y;

    lbDisplay.DrawColour = p_pdtext->Colour;
    lbFontPtr = p_pdtext->Font;
    my_set_text_window(p_pdtext->WindowX, p_pdtext->WindowY,
      p_pdtext->Width, p_pdtext->Height);
    my_draw_text(p_pdtext->X, p_pdtext->Y,
      p_pdtext->Text, p_pdtext->Line);

    if ((lbDisplay.DrawFlags & 0x8000) != 0)
    {
        short text_w;
        short shift_w, shift_h;

        text_w = my_string_width(p_pdtext->Text);
        if ((text_w >= p_pdtext->Width)
          || ((lbDisplay.DrawFlags & Lb_TEXT_HALIGN_CENTER)) != 0)
        {
            x = p_pdtext->WindowX;
            shift_w = p_pdtext->Width >> 1;
        }
        else
        {
            x = p_pdtext->X + p_pdtext->WindowX;
            shift_w = text_w >> 1;
        }
        shift_h = my_char_height('A') >> 1;
        y = p_pdtext->WindowY + p_pdtext->Y;
        screen_hotspot_add(x + shift_w, y + shift_h);
    }
}

static void draw_holo_copy_box(struct DIBox *p_pdbox)
{
    short x, y;
    short w, h;

    x = p_pdbox->X;
    y = p_pdbox->Y;
    w = p_pdbox->Width;
    h = p_pdbox->Height;

    LbScreenCopyBox(lbDisplay.WScreen, back_buffer,
        x, y, x, y, w, h);
}

static void draw_holo_sprite(struct DISprite *p_pdsprite)
{
    const struct TbSprite *p_spr;
    short x, y;

    x = p_pdsprite->X;
    y = p_pdsprite->Y;
    p_spr = p_pdsprite->Sprite;
    lbDisplay.DrawColour = p_pdsprite->Colour;
    if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0)
        LbSpriteDrawOneColour(x, y, p_spr, lbDisplay.DrawColour);
    else
        LbSpriteDraw(x, y, p_spr);

    if ((lbDisplay.DrawFlags & 0x8000) != 0)
    {
        short w, h;
        short shift_w, shift_h;

        w = p_spr->SWidth;
        h = p_spr->SHeight;
        shift_w = (w >> 1);
        shift_h = (h >> 1);
        screen_hotspot_add(x + shift_w, y + shift_h);
    }
}

static void draw_holo_trig(struct PolyPoint *p_ptA,
  struct PolyPoint *p_ptB, struct PolyPoint *p_ptC, TbPixel color)
{
    vec_colour = color;
    if ((p_ptC->Y - p_ptB->Y) * (p_ptB->X - p_ptA->X)
        - (p_ptB->Y - p_ptA->Y) * (p_ptC->X - p_ptB->X) > 0)
        trig(p_ptA, p_ptB, p_ptC);
    else
        trig(p_ptA, p_ptC, p_ptB);
}

static void draw_holo_flic(struct DIFlic *p_pdflic)
{
    //TODO avoid function callbacks from render
    p_pdflic->Function();
}

static void draw_holo_line(struct DILine *p_pdline)
{
    LbDrawLine(p_pdline->X1, p_pdline->Y1,
      p_pdline->X2, p_pdline->Y2, p_pdline->Colour);
}

static void draw_holo_hvline(struct DILine *p_pdline)
{
    LbDrawHVLine(p_pdline->X1, p_pdline->Y1,
      p_pdline->X2, p_pdline->Y2, p_pdline->Colour);
}

static void draw_holo_triangle(struct DITriangle *p_pdtrngl)
{
    LbDrawTriangle(p_pdtrngl->X1, p_pdtrngl->Y1,
      p_pdtrngl->X2, p_pdtrngl->Y2,
      p_pdtrngl->X3, p_pdtrngl->Y3, p_pdtrngl->Colour);
}

static void draw_purple_drawitems(void)
{
    struct PolyPoint point_a;
    struct PolyPoint point_c;
    struct PolyPoint point_b;
    ushort pditm;

    point_a.X = proj_origin.X;
    point_a.Y = proj_origin.Y;
    point_a.S = 0x200000;
    point_c.S = 0x200000;
    point_b.S = 0x8000;

    for (pditm = 0; pditm < purple_draw_index; pditm++)
    {
        struct PurpleDrawItem *p_pditem;

        p_pditem = &purple_draw_list[pditm];

        lbDisplay.DrawFlags = p_pditem->Flags;

        switch (p_pditem->Type)
        {
        case PuDT_BOX:
            draw_holo_box(&p_pditem->U.Box);
            break;
        case PuDT_TEXT:
            draw_holo_text(&p_pditem->U.Text);
            break;
        case PuDT_UNK03:
            break;
        case PuDT_COPYBOX:
            draw_holo_copy_box(&p_pditem->U.Box);
            break;
        case PuDT_SPRITE:
            draw_holo_sprite(&p_pditem->U.Sprite);
            break;
        case PuDT_HOLORAY:
            point_c.X = p_pditem->U.Line.X1;
            point_c.Y = p_pditem->U.Line.Y1;
            point_b.X = p_pditem->U.Line.X2;
            point_b.Y = p_pditem->U.Line.Y2;
            draw_holo_trig(&point_a, &point_b, &point_c, p_pditem->U.Line.Colour);
            break;
        case PuDT_FLIC:
            draw_holo_flic(&p_pditem->U.Flic);
            break;
        case PuDT_NOISEBOX:
            draw_noise_box(p_pditem->U.Box.X, p_pditem->U.Box.Y,
              p_pditem->U.Box.Width, p_pditem->U.Box.Height);
            break;
        case PuDT_LINE:
            draw_holo_line(&p_pditem->U.Line);
            break;
        case PuDT_HVLINE:
            draw_holo_hvline(&p_pditem->U.Line);
            break;
        case PuDT_TRIANGLE:
            draw_holo_triangle(&p_pditem->U.Triangle);
            break;
        case PuDT_HOTSPOT:
            screen_hotspot_add(p_pditem->U.Hotspot.X, p_pditem->U.Hotspot.Y);
            break;
        }
    }
}

void draw_purple_screen(void)
{
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
    my_set_text_window(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
    screen_hotspots_clear();
    vec_mode = RendVec_mode17;
    draw_purple_drawitems();
    purple_draw_index = 0;
    lbDisplay.DrawFlags = 0;
    // TODO Input should be separated from drawing
    input_screen_hotspots();
}

/******************************************************************************/
