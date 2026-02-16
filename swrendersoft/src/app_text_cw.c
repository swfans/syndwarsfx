/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file app_text_cw.c
 *     Functions for drawing text on graphical screen, per-application mod.
 * @par Purpose:
 *     Allows drawing text coloured in a wave-like transsition.
 * @par Comment:
 *     This is a modification of `gtext.c` from bflibrary.
 * @author   Tomasz Lis
 * @date     12 Nov 2008 - 15 Oct 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <stdio.h>
#include <limits.h>
#include "bftext.h"

#include "app_text_cw.h"

#include "bfconfig.h"
#include "bfgentab.h"
#include "bfsprite.h"
#include "bfscreen.h"
#include "bffont.h"
#include "bfmath.h"
#include "bfmemory.h"
#include "bfanywnd.h"

#include "enginprops.h"
#include "privrdlog.h"
/******************************************************************************/

#if defined(LB_ENABLE_SHADOW_COLOUR)
#  define SHADOW_COLOUR lbDisplay.ShadowColour
#else
#  define SHADOW_COLOUR 0x00
#endif

TbBool LbIApplyControlCharToDrawSettings(const char **c);
TbBool LbIAlignMethodSet(ushort fdflags);
TbBool is_wide_charcode(ulong chr);

/** @internal
 * Puts colour wave text sprites on screen.
 * @param sbuf
 * @param ebuf
 * @param x
 * @param y
 * @param space_len
 */
void put_down_colwavetext_sprites(const char *sbuf, const char *ebuf,
  long x, long y, long space_len,
  ubyte cw_base, ubyte cw_vari, ubyte cw_shift)
{
  const char *c;
  const struct TbSprite *p_spr;
  ubyte chr;
  long w,h;
  for (c=sbuf; c < ebuf; c++)
  {
    short fade_lv;
    TbPixel colour, shadcol;

    fade_lv = cw_base + cw_vari/2 - (cw_vari/2 * lbSinTable[LbFPMath_PI/8 * (cw_shift & 0xF)] >> 16);
    if (fade_lv > PALETTE_FADE_LEVELS-1)
        fade_lv = PALETTE_FADE_LEVELS-1;
    if (fade_lv < 0)
        fade_lv = 0;
    colour = lbDisplay.DrawColour;
    colour = pixmap.fade_table[fade_lv * PALETTE_8b_COLORS + colour];
    shadcol = SHADOW_COLOUR;
    chr = (ubyte)(*c);
    if (chr > 32)
    {
      p_spr = LbFontCharSprite(lbFontPtr, chr);
      if (p_spr != NULL)
      {
        // Draw shadow
        LbSpriteDrawOneColour(x + 1, y + 1, p_spr, shadcol);
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0)
          LbSpriteDrawOneColour(x, y, p_spr, colour);
        else
          LbSpriteDraw(x, y, p_spr);
        w = p_spr->SWidth;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight();
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        cw_shift += 1;
      }
    } else
    if (chr == ' ')
    {
        w = space_len;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight();
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        cw_shift += 1;
    } else
    if (chr == '\t')
    {
        w = space_len*(long)lbSpacesPerTab;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight();
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        cw_shift += lbSpacesPerTab;
    } else
    {
        LbIApplyControlCharToDrawSettings(&c);
    }
  }
}

/** @internal
 * Puts scaled colour wave text sprites on screen.
 * @param sbuf
 * @param ebuf
 * @param x
 * @param y
 * @param space_len
 * @param units_per_px
 */
void put_down_colwavetext_sprites_resized(const char *sbuf, const char *ebuf,
  long x, long y, long space_len, int units_per_px,
  ubyte cw_base, ubyte cw_vari, ubyte cw_shift)
{
  const char *c;
  const struct TbSprite *p_spr;
  ubyte chr;
  long w,h;
  for (c=sbuf; c < ebuf; c++)
  {
    short fade_lv;
    TbPixel colour, shadcol;

    fade_lv = cw_base + cw_vari/2 - (cw_vari/2 * lbSinTable[LbFPMath_PI/8 * (cw_shift & 0xF)] >> 16);
    if (fade_lv > PALETTE_FADE_LEVELS-1)
        fade_lv = PALETTE_FADE_LEVELS-1;
    if (fade_lv < 0)
        fade_lv = 0;
    colour = lbDisplay.DrawColour;
    colour = pixmap.fade_table[fade_lv * PALETTE_8b_COLORS + colour];
    shadcol = SHADOW_COLOUR;
    chr = (ubyte)(*c);
    if (chr > 32)
    {
      p_spr = LbFontCharSprite(lbFontPtr,chr);
      if (p_spr != NULL)
      {
        // Draw shadow
        LbSpriteDrawResizedOneColour(x + units_per_px/12, y + units_per_px/12, units_per_px, p_spr, shadcol);
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0) {
            LbSpriteDrawResizedOneColour(x, y, units_per_px, p_spr, colour);
        } else {
            LbSpriteDrawResized(x, y, units_per_px, p_spr);
        }
        w = p_spr->SWidth * units_per_px / 16;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        cw_shift += 1;
      }
    } else
    if (chr == ' ')
    {
        w = space_len;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        cw_shift += 1;
    } else
    if (chr == '\t')
    {
        w = space_len*(long)lbSpacesPerTab;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        cw_shift += lbSpacesPerTab;
    } else
    {
        LbIApplyControlCharToDrawSettings(&c);
    }
  }
}

void put_down_cw_sprites(const char *sbuf, const char *ebuf,
  long x, long y, long space_len, int units_per_px)
{
    if (units_per_px == 16)
    {
        put_down_colwavetext_sprites(sbuf, ebuf, x, y, space_len, 32, 16, render_anim_turn);
    } else
    {
        put_down_colwavetext_sprites_resized(sbuf, ebuf, x, y, space_len, units_per_px, 32, 16, render_anim_turn);
    }
}

/* draws a sprite scaled to double size; remove pending */
void AppSpriteDrawDoubleOneColour(const struct TbSprite *p_spr, int x, int y, ubyte col)
{
    int xwind_beg;
    int xwind_end;
    int xwind_start;
    sbyte *inp;
    ubyte *oline;
    int opitch;
    int h;
    TbBool needs_window_bounding;

    xwind_beg = lbDisplay.GraphicsWindowX;
    xwind_end = lbDisplay.GraphicsWindowX + lbDisplay.GraphicsWindowWidth;
    xwind_start = lbDisplay.GraphicsWindowX + x;
    inp = (sbyte *)p_spr->Data;
    opitch = lbDisplay.GraphicsScreenWidth;
    oline = &lbDisplay.WScreen[opitch * (lbDisplay.GraphicsWindowY + y) + lbDisplay.GraphicsWindowX + x];
    if (xwind_start < lbDisplay.GraphicsWindowX) {
        if (xwind_start + 2 * p_spr->SWidth <= lbDisplay.GraphicsWindowX)
            return;
        needs_window_bounding = true;
    } else {
        if (xwind_start >= xwind_end)
            return;
        needs_window_bounding = (xwind_start + 2 * p_spr->SWidth > xwind_end);
    }

    if (!needs_window_bounding)
    {
        // Simplified and faster drawing when we do not have to check bounds
        for (h = 0; h < p_spr->SHeight; h++)
        {
            ubyte *o;

            o = oline;
            while (*inp)
            {
                int ival;
                int i;

                ival = *inp;
                if (ival < 0)
                {
                    inp++;
                    o -= 2 * ival;
                    continue;
                }
                inp += ival + 1;
                for (i = 0; i < ival; i++)
                {
                    o[0] = col;
                    o[opitch + 0] = col;
                    o[1] = col;
                    o[opitch + 1] = col;
                    o += 2;
                }
            }
            inp++;
            oline += 2 * opitch;
        }
    }
    else
    {
        for (h = 0; h < p_spr->SHeight; h++)
        {
            ubyte *o;
            int xwind_curr;

            o = oline;
            xwind_curr = xwind_start;
            while (*inp)
            {
                int ival;
                int i;

                ival = *inp;
                if (ival < 0)
                {
                    inp++;
                    o -= 2 * ival;
                    xwind_curr -= 2 * ival;
                    continue;
                }
                inp += ival + 1;
                for (i = 0; i < ival; i++)
                {
                    if (xwind_curr >= xwind_beg && xwind_curr < xwind_end) {
                        o[0] = col;
                        o[opitch] = col;
                    }
                    xwind_curr++;
                    o++;
                    if (xwind_curr >= xwind_beg && xwind_curr < xwind_end) {
                        o[0] = col;
                        o[opitch] = col;
                    }
                    xwind_curr++;
                    o++;
                }
            }
            inp++;
            oline += 2 * opitch;
        }
    }
}

TbBool AppTextDrawColourWaveResized(int posx, int posy, int units_per_px, const char *text)
{
    struct TbAnyWindow grwnd;
    // Counter for amount of blank characters in a line
    long count;
    long justifyx,justifyy;
    long startx,starty;
    const char *sbuf;
    const char *ebuf;
    const char *prev_ebuf;
    long chr;
    long x, y, len;
    long w, h;

    if ((lbFontPtr == NULL) || (text == NULL))
        return true;
    LbScreenStoreGraphicsWindow(&grwnd);
    LbScreenLoadGraphicsWindow(&lbTextClipWindow);
    count = 0;
    justifyx = lbTextJustifyWindow.x - lbTextClipWindow.x;
    justifyy = lbTextJustifyWindow.y - lbTextClipWindow.y;
    posx += justifyx;
    startx = posx;
    starty = posy + justifyy;

    h = LbTextLineHeight() * units_per_px / 16;
    sbuf = text;
    for (ebuf=text; *ebuf != '\0'; ebuf++)
    {
        prev_ebuf=ebuf-1;
        chr = (ubyte)*ebuf;
        if (is_wide_charcode(chr))
        {
            ebuf++;
            if (*ebuf == '\0') break;
            chr = (chr<<8) + (ubyte)*ebuf;
        }

        if (chr > 32)
        {
            w = LbTextCharWidth(chr) * units_per_px / 16;
            if ((posx+w-justifyx <= (int)lbTextJustifyWindow.width) || (count > 0) ||
              !LbIAlignMethodSet(lbDisplay.DrawFlags))
            {
                posx += w;
                continue;
            }
            // If the char exceeds screen, and there were no spaces in that line,
            // and alignment is set - divide the line here
            w = LbTextCharWidth(' ') * units_per_px / 16;
            posx += w;
            x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbGetJustifiedCharWidth(posx, w, count, units_per_px, lbDisplay.DrawFlags);
            ebuf = prev_ebuf;
            put_down_cw_sprites(sbuf, ebuf, x, y, len, units_per_px);
            // We already know that alignment is set - don't re-check
            {
                posx = startx;
                sbuf = ebuf + 1; // sbuf points at start of char, while ebuf points at end of char
                starty += h;
            }
            count = 0;
        } else

        if (chr == ' ')
        {
            w = LbTextCharWidth(' ') * units_per_px / 16;
            len = LbSprFontWordWidth(lbFontPtr,ebuf+1) * units_per_px / 16;
            if (posx+w+len-justifyx <= (int)lbTextJustifyWindow.width)
            {
                count++;
                posx += w;
                continue;
            }
            posx += w;
            x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbGetJustifiedCharWidth(posx, w, count, units_per_px, lbDisplay.DrawFlags);
            put_down_cw_sprites(sbuf, ebuf, x, y, len, units_per_px);
            // End the line only if align method is set
            if (LbIAlignMethodSet(lbDisplay.DrawFlags))
            {
              posx = startx;
              sbuf = ebuf + 1; // sbuf points at start of char, while ebuf points at end of char
              starty += h;
            }
            count = 0;
        } else

        if (chr == '\n')
        {
            w = 0;
            x = LbGetJustifiedCharPosX(startx, posx, w, 1, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbTextCharWidth(' ') * units_per_px / 16;
            y = starty;
            put_down_cw_sprites(sbuf, ebuf, x, y, len, units_per_px);
            // We've got EOL sign - end the line
            sbuf = ebuf;
            posx = startx;
            starty += h;
            count = 0;
        } else

        if (chr == '\t')
        {
            w = LbTextCharWidth(' ') * units_per_px / 16;
            posx += lbSpacesPerTab*w;
            len = LbSprFontWordWidth(lbFontPtr,ebuf+1) * units_per_px / 16;
            if (posx+len-justifyx <= (int)lbTextJustifyWindow.width)
            {
              count += lbSpacesPerTab;
              continue;
            }
            x = LbGetJustifiedCharPosX(startx, posx, w, lbSpacesPerTab, lbDisplay.DrawFlags);
            y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
            len = LbGetJustifiedCharWidth(posx, w, count, units_per_px, lbDisplay.DrawFlags);
            put_down_cw_sprites(sbuf, ebuf, x, y, len, units_per_px);
            if (LbIAlignMethodSet(lbDisplay.DrawFlags))
            {
              posx = startx;
              sbuf = ebuf + 1;
              starty += h;
            }
            count = 0;
            continue;

        } else

        if ((chr == 6) || (chr == 7) || (chr == 8) || (chr == 9))
        {
            if (posx-justifyx > (int)lbTextJustifyWindow.width)
            {
              x = startx;
              y = starty;
              len = LbTextCharWidth(' ') * units_per_px / 16;
              put_down_cw_sprites(sbuf, ebuf, x, y, len, units_per_px);
              posx = startx;
              sbuf = ebuf;
              count = 0;
              starty += h;
            }
            switch (*ebuf)
            {
            case 6:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_LEFT;
              break;
            case 7:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_RIGHT;
              break;
            case 8:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_CENTER;
              break;
            case 9:
              lbDisplay.DrawFlags ^= Lb_TEXT_HALIGN_JUSTIFY;
              break;
            }
        } else

        if (chr == 14)
        {
            ebuf++;
            if (*ebuf == '\0')
              break;
        }
    }
    x = LbGetJustifiedCharPosX(startx, posx, 0, 1, lbDisplay.DrawFlags);
    y = LbGetJustifiedCharPosY(starty, h, h, lbDisplay.DrawFlags);
    len = LbTextCharWidth(' ') * units_per_px / 16;
    put_down_cw_sprites(sbuf, ebuf, x, y, len, units_per_px);
    LbScreenLoadGraphicsWindow(&grwnd);
    return true;
}

TbBool AppTextDrawColourWave(int posx, int posy, const char *text)
{
    // Using resized version - it will end up with version optimized for no resize anyway
    return AppTextDrawColourWaveResized(posx, posy, 16, text);
}

struct TbSprite *AppFontCharSpriteRW(struct TbSprite *font,
  const ulong chr)
{
    if (font == NULL)
        return NULL;
    if ((chr >= 31) && (chr < 256))
        return &font[(chr-31)];
    return NULL;
}

/******************************************************************************/
