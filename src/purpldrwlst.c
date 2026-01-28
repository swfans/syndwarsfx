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

extern ubyte purple_joy_move;

void draw_purple_screen_hotspots(ushort hsnext)
{
    lbDisplay.DrawFlags = 0;
    if (purple_joy_move)
    {
        if (!joy.DigitalY[0] && !joy.DigitalX[0])
            purple_joy_move = 0;
    }
    else if (joy.DigitalY[0] == 1)
    {
        ulong hmin;
        short imin, i;

        hmin = 0x80000000;
        imin = 0;
        for (i = 0; i < hsnext; i++)
        {
            short ms_x, ms_y;
            short shift_w, shift_h;

            ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
            ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;
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
        if (hmin != 0x80000000)
            LbMouseSetPosition(hotspot_buffer[imin].X, hotspot_buffer[imin].Y);
        purple_joy_move = 1;
    }
    else if (joy.DigitalY[0] == -1)
    {
        ulong hmin;
        short imin, i;

        hmin = 0x80000000;
        imin = 0;
        for (i = 0; i < hsnext; i++)
        {
            short ms_x, ms_y;
            short shift_w, shift_h;

            ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
            ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;
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
        if (hmin != 0x80000000)
            LbMouseSetPosition(hotspot_buffer[imin].X, hotspot_buffer[imin].Y);
        purple_joy_move = 1;
    }
    else if (joy.DigitalX[0] == 1)
    {
        ulong hmin;
        short imin, i;

        hmin = 0x80000000;
        imin = 0;
        for (i = 0; i < hsnext; i++)
        {
            short ms_x, ms_y;
            short shift_w, shift_h;

            ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
            ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;
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
        if (hmin != 0x80000000)
            LbMouseSetPosition(hotspot_buffer[imin].X, hotspot_buffer[imin].Y);
        purple_joy_move = 1;
    }
    else if (joy.DigitalX[0] == -1)
    {
        ulong hmin;
        short imin, i;

        hmin = 0x80000000;
        imin = 0;
        for (i = 0; i < hsnext; i++)
        {
            short ms_x, ms_y;
            short shift_w, shift_h;

            ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
            ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;
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
        if (hmin != 0x80000000)
            LbMouseSetPosition(hotspot_buffer[imin].X, hotspot_buffer[imin].Y);
        purple_joy_move = 1;
    }
}

void draw_purple_screen(void)
{
    struct PurpleDrawItem *pditem;
    struct PolyPoint point_a;
    struct PolyPoint point_c;
    struct PolyPoint point_b;
    ushort hsnext;

    short x, y;
    short w, h;
    short shift_w, shift_h;

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
    my_set_text_window(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
    hsnext = 0;
    point_a.X = proj_origin.X;
    point_a.Y = proj_origin.Y;
    point_a.S = 0x200000;
    point_c.S = 0x200000;
    point_b.S = 0x8000;
    vec_mode = 17;
    for (pditem = purple_draw_list; pditem < &purple_draw_list[purple_draw_index]; pditem++)
    {
        lbDisplay.DrawFlags = pditem->Flags;

        switch (pditem->Type)
        {
        case PuDT_BOX:
            LbDrawBox(pditem->U.Box.X, pditem->U.Box.Y, pditem->U.Box.Width,
                pditem->U.Box.Height, pditem->U.Box.Colour);
            if ((lbDisplay.DrawFlags & 0x8000) != 0)
            {
                hotspot_buffer[hsnext].X = pditem->U.Box.X + (pditem->U.Box.Width >> 1);
                hotspot_buffer[hsnext].Y = pditem->U.Box.Y + (pditem->U.Box.Height >> 1);
                hsnext++;
            }
            break;
        case PuDT_TEXT:
            lbDisplay.DrawColour = pditem->U.Text.Colour;
            lbFontPtr = pditem->U.Text.Font;
            my_set_text_window(pditem->U.Text.WindowX, pditem->U.Text.WindowY,
              pditem->U.Text.Width, pditem->U.Text.Height);
            my_draw_text(pditem->U.Text.X, pditem->U.Text.Y,
              pditem->U.Text.Text, pditem->U.Text.Line);
            if ((lbDisplay.DrawFlags & 0x8000) != 0)
            {
                w = my_string_width(pditem->U.Text.Text);
                if ((w >= pditem->U.Text.Width)
                  || ((lbDisplay.DrawFlags & Lb_TEXT_HALIGN_CENTER)) != 0)
                {
                    x = pditem->U.Text.WindowX;
                    shift_w = pditem->U.Text.Width >> 1;
                }
                else
                {
                    x = pditem->U.Text.X + pditem->U.Text.WindowX;
                    shift_w = w >> 1;
                }
                shift_h = my_char_height('A') >> 1;
                y = pditem->U.Text.Y + pditem->U.Text.WindowY;
                hotspot_buffer[hsnext].X = x + shift_w;
                hotspot_buffer[hsnext].Y = y + shift_h;
                hsnext++;
            }
            break;
        case PuDT_UNK03:
            break;
        case PuDT_COPYBOX:
            x = pditem->U.Box.X;
            y = pditem->U.Box.Y;
            shift_w = pditem->U.Box.Width;
            shift_h = pditem->U.Box.Height;
            LbScreenCopyBox(lbDisplay.WScreen, back_buffer,
                x, y, x, y, shift_w, shift_h);
            break;
        case PuDT_SPRITE:
            lbDisplay.DrawColour = pditem->U.Box.Colour;
            if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0)
                LbSpriteDrawOneColour(pditem->U.Sprite.X, pditem->U.Sprite.Y,
                  pditem->U.Sprite.Sprite, lbDisplay.DrawColour);
            else
                LbSpriteDraw(pditem->U.Sprite.X, pditem->U.Sprite.Y,
                  pditem->U.Sprite.Sprite);
            if ((lbDisplay.DrawFlags & 0x8000) != 0)
            {
                w = pditem->U.Sprite.Sprite->SWidth;
                h = pditem->U.Sprite.Sprite->SHeight;
                hotspot_buffer[hsnext].X = pditem->U.Sprite.X + (w >> 1);
                hotspot_buffer[hsnext].X = pditem->U.Sprite.Y + (h >> 1);
                hsnext++;
            }
            break;
        case PuDT_POTRIG:
            vec_colour = pditem->U.Line.Colour;
            point_c.X = pditem->U.Line.X1;
            point_c.Y = pditem->U.Line.Y1;
            point_b.X = pditem->U.Line.X2;
            point_b.Y = pditem->U.Line.Y2;
            if ((point_c.Y - point_b.Y) * (point_b.X - point_a.X)
                - (point_b.Y - point_a.Y) * (point_c.X - point_b.X) > 0)
                trig(&point_a, &point_b, &point_c);
            else
                trig(&point_a, &point_c, &point_b);
            break;
        case PuDT_FLIC:
            pditem->U.Flic.Function();
            break;
        case PuDT_NOISEBOX:
            draw_noise_box(pditem->U.Box.X, pditem->U.Box.Y, pditem->U.Box.Width, pditem->U.Box.Height);
            break;
        case PuDT_LINE:
            LbDrawLine(pditem->U.Line.X1, pditem->U.Line.Y1,
                pditem->U.Line.X2, pditem->U.Line.Y2, pditem->U.Line.Colour);
            break;
        case PuDT_HVLINE:
            LbDrawHVLine(pditem->U.Line.X1, pditem->U.Line.Y1,
                pditem->U.Line.X2, pditem->U.Line.Y2, pditem->U.Line.Colour);
            break;
        case PuDT_TRIANGLE:
            LbDrawTriangle(pditem->U.Triangle.X1, pditem->U.Triangle.Y1,
                pditem->U.Triangle.X2, pditem->U.Triangle.Y2,
                pditem->U.Triangle.X3, pditem->U.Triangle.Y3, pditem->U.Triangle.Colour);
            break;
        case PuDT_HOTSPOT:
            hotspot_buffer[hsnext].X = pditem->U.Hotspot.X;
            hotspot_buffer[hsnext].Y = pditem->U.Hotspot.Y;
            hsnext++;
            break;
        }
    }
    purple_draw_index = 0;

    draw_purple_screen_hotspots(hsnext);
}

/******************************************************************************/
