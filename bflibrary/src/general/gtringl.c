/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file gtringl.c
 *     Implementation of related functions.
 * @par Purpose:
 *     Drawing triangles.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 22 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <string.h>
#include "bftringl.h"

#include "bfscreen.h"
#include "bfline.h"
#include "privbflog.h"

#define DRAW_RANGES_COUNT (MAX_SUPPORTED_SCREEN_HEIGHT * 6 / 5)

enum TriangleCase {
    TriCase_Unkn24 = 0,
    TriCase_Unkn50,
    TriCase_Unkn77,
    TriCase_Unkn92,
};

enum RampType {
    Ramp_Single = 0,
    Ramp_SmoothLeft,
    Ramp_SmoothRight,
};

struct TbSPoint {
  short x;
  short y;
};

struct DrawRange { // sizeof = 8
    long x;
    long w;
};

struct DrawRange draw_ranges[DRAW_RANGES_COUNT];

void LbDrawTriangleOutline(short x1, short y1, short x2, short y2, short x3, short y3, TbPixel colour)
{
    LbDrawLine(x1, y1, x2, y2, colour);
    LbDrawLine(x2, y2, x3, y3, colour);
    LbDrawLine(x3, y3, x1, y1, colour);
}

/** Blit given colour to given graphics window buffer, using a range to fill for each line.
 *
 * @param p_screen Output graphics window buffer.
 * @param ranges Array of ranges to fill; contains X coords and widths, each shifted left by 16.
 *
 * This function honors transparency set within DrawFlags.
 */
static void LbBlitSolidRanges(TbPixel *p_screen, struct DrawRange *ranges, int range_num, TbPixel colour)
{
    struct DrawRange *rng;

    rng = ranges;
    for (; range_num > 0; range_num--, rng++)
    {
        ushort m;
        int w, shift;
        ubyte *p_map;
        ubyte *o;

        shift = rng->x >> 16;
        w = rng->w >> 16;
        p_screen += lbDisplay.GraphicsScreenWidth;
        if (shift > 0)
        {
            if (w > lbDisplay.GraphicsWindowWidth)
                w = lbDisplay.GraphicsWindowWidth;
            w -= shift;
            if (w <= 0)
                continue;
            o = &p_screen[shift];
        }
        else
        {
            if (w <= 0)
                continue;
            if (w > lbDisplay.GraphicsWindowWidth)
                w = lbDisplay.GraphicsWindowWidth;
            o = p_screen;
        }
        p_map = lbDisplay.GlassMap;
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) != 0)
        {
            m = (colour << 8);
            for (; w > 0; w--)
            {
                m = (m & 0xFF00) | (*o & 0xFF);
                *o++ = p_map[m];
            }
        }
        else if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR8) != 0)
        {
            m = colour;
            for (; w > 0; w--)
            {
                m = (m & 0x00FF) | ((*o << 8) & 0xFF00);
                *o++ = p_map[m];
            }
        }
        else
        {
            m = colour;
            memset(o, m, w);
        }
    }
}

static void LbI_RangeFillSingleRamp(struct DrawRange *ranges, int blt1height,
  int blt1xval, int blt1xinc, int blt1wval, int blt1winc)
{
    struct DrawRange *rng;

    rng = ranges;
    for (; blt1height > 0; blt1height--)
    {
        rng->x = blt1xval;
        blt1xval += blt1xinc;
        rng->w = blt1wval;
        blt1wval += blt1winc;
        rng++;
    }
}

static void LbI_RangeFillTwoRampsSmoothLeft(struct DrawRange *ranges, int blt1height,
  int blt1xval, int blt1xinc, int blt1wval, int blt1winc,
  int blt2height, int blt2wval, int blt2winc)
{
    struct DrawRange *rng;

    rng = ranges;
    for (; blt1height > 0; blt1height--)
    {
        rng->x = blt1xval;
        blt1xval += blt1xinc;
        rng->w = blt1wval;
        blt1wval += blt1winc;
        rng++;
    }
    for (; blt2height > 0; blt2height--)
    {
        rng->x = blt1xval;
        blt1xval += blt1xinc;
        rng->w = blt2wval;
        blt2wval += blt2winc;
        rng++;
    }
}

static void LbI_RangeFillTwoRampsSmoothRight(struct DrawRange *ranges, int blt1height,
  int blt1xval, int blt1xinc, int blt1wval, int blt1winc,
  int blt2height, int blt2xval, int blt2xinc)
{
    struct DrawRange *rng;

    rng = ranges;
    for (; blt1height > 0; blt1height--)
    {
        rng->x = blt1xval;
        blt1xval += blt1xinc;
        rng->w = blt1wval;
        blt1wval += blt1winc;
        rng++;
    }
    for (; blt2height > 0; blt2height--)
    {
        rng->x = blt2xval;
        blt2xval += blt2xinc;
        rng->w = blt1wval;
        blt1wval += blt1winc;
        rng++;
    }
}

// overflow flag of subtraction (x-y)
ubyte __OFSUBW__(short x, short y)
{
    int y2 = y;
    ubyte sx = (x < 0);
    return (sx ^ (y2 < 0)) & (sx ^ (x-y2 < 0));
}

void LbDrawTriangleFilled(short x1, short y1, short x2, short y2, short x3, short y3, TbPixel colour)
{
#if 0
    asm volatile (
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_LbDrawTriangleFilled\n"
        :  : "a" (x1), "d" (y1), "b" (x2), "c" (y2), "g" (x3), "g" (y3), "g" (colour));
    return;
#endif
    struct TbSPoint *p_pt1;
    struct TbSPoint *p_pt2;
    struct TbSPoint *p_pt3;
    int dist_y32, dist_y31;
    char blt2skip;
    TbBool pt2y_overflow;
    TbBool pt3y_overflow;
    TbBool pt1y_underflow;
    enum TriangleCase tricase;
    enum RampType ramp;
    short blt1height, blt2height;
    int blt1xval, blt2xval;
    int blt1xinc, blt2xinc;
    int blt1wval, blt2wval;
    int blt1winc, blt2winc;
    int totaheight;
    TbPixel *p_screen;
    struct TbSPoint pt3;
    struct TbSPoint pt2;
    struct TbSPoint pt1;

    pt1.x = x1;
    pt2.x = x2;
    pt3.x = x3;
    pt1.y = y1;
    pt2.y = y2;
    pt3.y = y3;

    if (pt1.y == pt2.y)
    {
        if (pt1.y == pt3.y)
            return;
        if (pt1.y < pt3.y)
        {
            if (pt2.x <= pt1.x)
                return;
            p_pt1 = &pt1;
            p_pt2 = &pt2;
            p_pt3 = &pt3;
            tricase = TriCase_Unkn92;
        }
        else
        {
            if (pt1.x <= pt2.x)
                return;
            p_pt1 = &pt3;
            p_pt2 = &pt1;
            p_pt3 = &pt2;
            tricase = TriCase_Unkn77;
        }
    }
    else if (pt1.y <= pt2.y)
    {
        if (pt1.y == pt3.y)
        {
            if (pt1.x <= pt3.x)
                return;
            p_pt1 = &pt3;
            p_pt2 = &pt1;
            p_pt3 = &pt2;
            tricase = TriCase_Unkn92;
        }
        else if (pt1.y >= pt3.y)
        {
            p_pt1 = &pt3;
            p_pt2 = &pt1;
            p_pt3 = &pt2;
            tricase = TriCase_Unkn24;
        }
        else if (pt2.y == pt3.y)
        {
            if (pt2.x <= pt3.x)
                return;
            p_pt1 = &pt1;
            p_pt2 = &pt2;
            p_pt3 = &pt3;
            tricase = TriCase_Unkn77;
        }
        else if (pt2.y <= pt3.y)
        {
            p_pt1 = &pt1;
            p_pt2 = &pt2;
            p_pt3 = &pt3;
            tricase = TriCase_Unkn24;
        }
        else
        {
            p_pt1 = &pt1;
            p_pt2 = &pt2;
            p_pt3 = &pt3;
            tricase = TriCase_Unkn50;
        }
    }
    else
    {
        if (pt1.y == pt3.y)
        {
            if (pt3.x <= pt1.x)
                return;
            p_pt1 = &pt2;
            p_pt2 = &pt3;
            p_pt3 = &pt1;
            tricase = TriCase_Unkn77;
        }
        else if (pt1.y < pt3.y)
        {
            p_pt1 = &pt2;
            p_pt2 = &pt3;
            p_pt3 = &pt1;
            tricase = TriCase_Unkn50;
        }
        else if (pt2.y == pt3.y)
        {
            if (pt3.x <= pt2.x)
                return;
            p_pt1 = &pt2;
            p_pt2 = &pt3;
            p_pt3 = &pt1;
            tricase = TriCase_Unkn92;
        }
        else if (pt2.y < pt3.y)
        {
            p_pt1 = &pt2;
            p_pt2 = &pt3;
            p_pt3 = &pt1;
            tricase = TriCase_Unkn24;
        }
        else
        {
            p_pt1 = &pt3;
            p_pt2 = &pt1;
            p_pt3 = &pt2;
            tricase = TriCase_Unkn50;
        }
    }

    if (p_pt1->y < 0)
    {
        p_screen = lbDisplay.WScreen;
        pt1y_underflow = 1;
    }
    else if (p_pt1->y < lbDisplay.GraphicsWindowHeight)
    {
        p_screen = &lbDisplay.WScreen[lbDisplay.GraphicsScreenWidth * p_pt1->y];
        pt1y_underflow = 0;
    }
    else
    {
        return;
    }

    switch (tricase)
    {
    case TriCase_Unkn50:
        pt3y_overflow = p_pt3->y > lbDisplay.GraphicsWindowHeight;
        pt2y_overflow = p_pt2->y > lbDisplay.GraphicsWindowHeight;
        blt1height = p_pt3->y - p_pt1->y;
        totaheight = p_pt2->y - p_pt1->y;
        blt1xinc = ((p_pt3->x - p_pt1->x) << 16) / blt1height;
        blt1winc = ((p_pt2->x - p_pt1->x) << 16) / totaheight;
        if (blt1winc <= blt1xinc)
            return;
        blt2height = p_pt2->y - p_pt3->y;
        blt2xinc = ((p_pt2->x - p_pt3->x) << 16) / blt2height;
        blt2xval = p_pt3->x << 16;
        blt1xval = p_pt1->x << 16;
        blt1wval = blt1xval;
        if (!pt1y_underflow)
        {
            blt2skip = pt3y_overflow;
            if (pt2y_overflow)
            {
                short tmp1;
                ubyte tmp1_overflow;

                totaheight = lbDisplay.GraphicsWindowHeight - p_pt1->y;
                if (blt2skip) {
                    blt1height = lbDisplay.GraphicsWindowHeight - p_pt1->y;
                } else {
                    tmp1_overflow = __OFSUBW__(totaheight, blt1height);
                    tmp1 = totaheight - blt1height;
                    blt2skip = ((tmp1 < 0) ^ tmp1_overflow) | (tmp1 == 0);
                    blt2height = tmp1;
                }
            }
        }
        else
        {
            int mcor_y1, dist_m1;

            mcor_y1 = -p_pt1->y;
            totaheight -= mcor_y1;
            if (totaheight <= 0)
                return;
            blt2skip = pt3y_overflow;
            if (mcor_y1 - blt1height >= 0)
            {
                dist_m1 = mcor_y1 - blt1height;
                blt2height -= dist_m1;
                blt2xval += dist_m1 * blt2xinc;
                blt1wval += dist_m1 * blt1winc + blt1height * blt1winc;
                if (pt2y_overflow)
                {
                    blt2height = lbDisplay.GraphicsWindowHeight;
                    totaheight = lbDisplay.GraphicsWindowHeight;
                }
                blt1height = 0;
            }
            else
            {
                blt1height -= mcor_y1;
                blt1xval += mcor_y1 * blt1xinc;
                blt1wval += mcor_y1 * blt1winc;
                if (pt2y_overflow)
                {
                    totaheight = lbDisplay.GraphicsWindowHeight;
                    if (blt2skip) {
                        blt1height = lbDisplay.GraphicsWindowHeight;
                    } else {
                        blt2skip = lbDisplay.GraphicsWindowHeight <= blt1height;
                        blt2height = lbDisplay.GraphicsWindowHeight - blt1height;
                    }
                }
            }
        }
        if (blt2skip)
            blt2height = 0;
        ramp = Ramp_SmoothRight;
        break;

    case TriCase_Unkn77:
        pt3y_overflow = p_pt3->y > lbDisplay.GraphicsWindowHeight;
        blt1height = p_pt3->y - p_pt1->y;
        totaheight = blt1height;
        blt1xinc = ((p_pt3->x - p_pt1->x) << 16) / blt1height;
        blt1winc = ((p_pt2->x - p_pt1->x) << 16) / blt1height;
        blt1xval = p_pt1->x << 16;
        blt1wval = blt1xval;
        if (!pt1y_underflow)
        {
            if (pt3y_overflow)
            {
                blt1height = lbDisplay.GraphicsWindowHeight - p_pt1->y;
                totaheight = blt1height;
            }
        }
        else
        {
            int mcor_y1;

            mcor_y1 = -p_pt1->y;
            blt1height -= mcor_y1;
            totaheight -= mcor_y1;
            if (totaheight <= 0)
                return;
            blt1xval += blt1xinc * mcor_y1;
            blt1wval += mcor_y1 * blt1winc;
            if (pt3y_overflow)
            {
                blt1height = lbDisplay.GraphicsWindowHeight;
                totaheight = blt1height;
            }
        }
        ramp = Ramp_Single;
        break;

    case TriCase_Unkn24:
        pt3y_overflow = p_pt3->y > lbDisplay.GraphicsWindowHeight;
        dist_y31 = p_pt3->y - p_pt1->y;
        totaheight = dist_y31;
        blt2skip = p_pt2->y > lbDisplay.GraphicsWindowHeight;
        dist_y32 = p_pt3->y - p_pt2->y;
        blt1height = p_pt2->y - p_pt1->y;
        blt1xinc = ((p_pt3->x - p_pt1->x) << 16) / dist_y31;
        blt1winc = ((p_pt2->x - p_pt1->x) << 16) / blt1height;
        if (blt1winc <= blt1xinc)
            return;
        blt2winc = ((p_pt3->x - p_pt2->x) << 16) / dist_y32;
        blt2height = p_pt3->y - p_pt2->y;
        blt2wval = p_pt2->x << 16;
        blt1xval = p_pt1->x << 16;
        blt1wval = blt1xval;
        if (!pt1y_underflow)
        {
            if (pt3y_overflow)
            {
                short tmp1;
                ubyte tmp1_overflow;

                totaheight = lbDisplay.GraphicsWindowHeight - p_pt1->y;
                if (blt2skip) {
                    blt1height = totaheight;
                } else {
                    tmp1_overflow = __OFSUBW__(totaheight, blt1height);
                    tmp1 = totaheight - blt1height;
                    blt2skip = ((tmp1 < 0) ^ tmp1_overflow) | (tmp1 == 0);
                    blt2height = tmp1;
                }
            }
        }
        else
        {
            int mcor_y1, dist_m1;

            mcor_y1 = -p_pt1->y;
            totaheight -= mcor_y1;
            if (totaheight <= 0)
                return;
            if (mcor_y1 - blt1height >= 0)
            {
                blt2height -= mcor_y1 - blt1height;
                dist_m1 = mcor_y1 - blt1height;
                blt1xval += blt1xinc * dist_m1 + blt1height * blt1xinc;
                blt2wval += blt2winc * dist_m1;
                if (pt3y_overflow)
                {
                    blt2height = lbDisplay.GraphicsWindowHeight;
                    totaheight = blt2height;
                }
                blt1height = 0;
            }
            else
            {
                blt1height += p_pt1->y;
                blt1xval += blt1xinc * mcor_y1;
                blt1wval += mcor_y1 * blt1winc;
                if (pt3y_overflow)
                {
                    totaheight = lbDisplay.GraphicsWindowHeight;
                    if (blt2skip) {
                        blt1height = lbDisplay.GraphicsWindowHeight;
                    } else {
                        blt2skip = lbDisplay.GraphicsWindowHeight <= blt1height;
                        blt2height = lbDisplay.GraphicsWindowHeight - blt1height;
                    }
                }
            }
        }
        if (blt2skip)
            blt2height = 0;
        ramp = Ramp_SmoothLeft;
        break;

    case TriCase_Unkn92:
        pt3y_overflow = p_pt3->y > lbDisplay.GraphicsWindowHeight;
        blt1height = p_pt3->y - p_pt1->y;
        totaheight = blt1height;
        blt1xinc = ((p_pt3->x - p_pt1->x) << 16) / blt1height;
        blt1winc = ((p_pt3->x - p_pt2->x) << 16) / blt1height;
        blt1xval = p_pt1->x << 16;
        blt1wval = p_pt2->x << 16;
        if (!pt1y_underflow)
        {
            if (pt3y_overflow)
            {
                blt1height = lbDisplay.GraphicsWindowHeight - p_pt1->y;
                totaheight = blt1height;
            }
        }
        else
        {
            int mcor_y1;

            mcor_y1 = -p_pt1->y;
            blt1height -= mcor_y1;
            totaheight -= mcor_y1;
            if (totaheight <= 0)
                return;
            blt1xval += mcor_y1 * blt1xinc;
            blt1wval += mcor_y1 * blt1winc;
            if (pt3y_overflow)
            {
                blt1height = lbDisplay.GraphicsWindowHeight;
                totaheight = blt1height;
            }
        }
        ramp = Ramp_Single;
        break;
    default:
        LOGERR("internal error - bad triangle case");
        return;
    }

    // Fill the ranges array with two areas
    switch (ramp)
    {
    case Ramp_SmoothLeft:
        LbI_RangeFillTwoRampsSmoothLeft(draw_ranges, blt1height, blt1xval,
          blt1xinc, blt1wval, blt1winc, blt2height, blt2wval, blt2winc);
        break;
    case Ramp_SmoothRight:
        LbI_RangeFillTwoRampsSmoothRight(draw_ranges, blt1height, blt1xval,
          blt1xinc, blt1wval, blt1winc, blt2height, blt2xval, blt2xinc);
        break;
    default:
        LbI_RangeFillSingleRamp(draw_ranges, blt1height, blt1xval,
          blt1xinc, blt1wval, blt1winc);
        break;
    }
    // Draw on graphics buffer
    p_screen += lbDisplay.GraphicsWindowX + lbDisplay.GraphicsScreenWidth * (lbDisplay.GraphicsWindowY - 1);
    LbBlitSolidRanges(p_screen, draw_ranges, totaheight, colour);
}

void LbDrawTriangle(short x1, short y1, short x2, short y2, short x3, short y3, TbPixel colour)
{
    if (lbDisplay.DrawFlags & Lb_SPRITE_OUTLINE)
    {
        LbDrawTriangleOutline(x1, y1, x2, y2, x3, y3, colour);
    }
    else
    {
        LbDrawTriangleFilled(x1, y1, x2, y2, x3, y3, colour);
    }
}

/******************************************************************************/
