/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file app_text_sf.c
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

#include "app_text_sf.h"

#include "bfconfig.h"
#include "bfgentab.h"
#include "bfsprite.h"
#include "bfscreen.h"
#include "bffont.h"
#include "bfmath.h"
#include "bfmemory.h"
#include "bfanywnd.h"

#include "engincolour.h"
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

void AppSpriteDrawDoubleOneColour(const struct TbSprite *p_spr, int x, int y, ubyte col);

/** @internal
 * Puts shadow colour flash text sprites on screen.
 * @param sbuf
 * @param ebuf
 * @param x
 * @param y
 * @param space_len
 */
void put_down_shdcflshtext_sprites(const char *sbuf, const char *ebuf,
  long x, long y, long space_len,
  short sf_base, short sf_shift)
{
  const char *c;
  const struct TbSprite *spr;
  ubyte chr;
  long w,h;
  for (c=sbuf; c < ebuf; c++)
  {
    short fade_lv;
    TbPixel colour, shadcol;

    fade_lv = sf_base + sf_shift;
    if (fade_lv > PALETTE_FADE_LEVELS-1)
        fade_lv = (PALETTE_FADE_LEVELS-1) - (fade_lv - (PALETTE_FADE_LEVELS-1));
    if (fade_lv > PALETTE_FADE_LEVELS-1)
        fade_lv = PALETTE_FADE_LEVELS-1;
    if (fade_lv < 0)
        fade_lv = 0;
    colour = lbDisplay.DrawColour;
    shadcol = colour_lookup[ColLU_GREYLT];
    shadcol = pixmap.fade_table[fade_lv * PALETTE_8b_COLORS + shadcol];
    chr = (ubyte)(*c);
    if (chr > 32)
    {
      spr = LbFontCharSprite(lbFontPtr,chr);
      if (spr != NULL)
      {
        // Draw shadow
        LbSpriteDrawOneColour(x + 1, y + 1, spr, shadcol);
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0)
          LbSpriteDrawOneColour(x, y, spr, colour);
        else
          LbSpriteDraw(x, y, spr);
        w = spr->SWidth;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight();
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        sf_shift += 1;
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
        sf_shift += 1;
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
        sf_shift += lbSpacesPerTab;
    } else
    {
        LbIApplyControlCharToDrawSettings(&c);
    }
  }
}

/** @internal
 * Puts scaled shadow colour flash text sprites on screen.
 * @param sbuf
 * @param ebuf
 * @param x
 * @param y
 * @param space_len
 * @param units_per_px
 */
void put_down_shdcflshtext_sprites_resized(const char *sbuf, const char *ebuf,
  long x, long y, long space_len, int units_per_px,
  short sf_base, short sf_shift)
{
  const char *c;
  const struct TbSprite *spr;
  ubyte chr;
  long w,h;
  for (c=sbuf; c < ebuf; c++)
  {
    short fade_lv;
    TbPixel colour, shadcol;

    fade_lv = sf_base + sf_shift;
    if (fade_lv > PALETTE_FADE_LEVELS-1)
        fade_lv = (PALETTE_FADE_LEVELS-1) - (fade_lv - (PALETTE_FADE_LEVELS-1));
    if (fade_lv > PALETTE_FADE_LEVELS-1)
        fade_lv = PALETTE_FADE_LEVELS-1;
    if (fade_lv < 0)
        fade_lv = 0;
    colour = lbDisplay.DrawColour;
    shadcol = colour_lookup[ColLU_GREYLT];
    shadcol = pixmap.fade_table[fade_lv * PALETTE_8b_COLORS + shadcol];
    chr = (ubyte)(*c);
    if (chr > 32)
    {
      spr = LbFontCharSprite(lbFontPtr,chr);
      if (spr != NULL)
      {
        // Draw shadow
        LbSpriteDrawResizedOneColour(x + units_per_px/12, y + units_per_px/12, units_per_px, spr, shadcol);
        if ((lbDisplay.DrawFlags & Lb_TEXT_ONE_COLOR) != 0) {
            LbSpriteDrawResizedOneColour(x, y, units_per_px, spr, colour);
        } else {
            LbSpriteDrawResized(x, y, units_per_px, spr);
        }
        w = spr->SWidth * units_per_px / 16;
        if ((lbDisplay.DrawFlags & Lb_TEXT_UNDERLINE) != 0)
        {
            h = LbTextLineHeight() * units_per_px / 16;
            LbDrawCharUnderline(x, y, w, h, colour, shadcol);
        }
        x += w;
        sf_shift += 1;
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
        sf_shift += 1;
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
        sf_shift += lbSpacesPerTab;
    } else
    {
        LbIApplyControlCharToDrawSettings(&c);
    }
  }
}

void put_down_sf_sprites(const char *sbuf, const char *ebuf,
  long x, long y, long space_len, int units_per_px, ushort ttl_turns)
{
    if (units_per_px == 16)
    {
        put_down_shdcflshtext_sprites(sbuf, ebuf, x, y, space_len,
          -180, 4 * ttl_turns);
    } else
    {
        put_down_shdcflshtext_sprites_resized(sbuf, ebuf, x, y, space_len,
          units_per_px, -180, 4 * ttl_turns);
    }
}

TbBool AppTextDrawShadClFlashResized(int posx, int *posy, int units_per_px, ushort ttl_turns, const char *text)
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
    starty = *posy + justifyy;

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
            put_down_sf_sprites(sbuf, ebuf, x, y, len, units_per_px, ttl_turns);
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
            put_down_sf_sprites(sbuf, ebuf, x, y, len, units_per_px, ttl_turns);
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
            put_down_sf_sprites(sbuf, ebuf, x, y, len, units_per_px, ttl_turns);
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
            put_down_sf_sprites(sbuf, ebuf, x, y, len, units_per_px, ttl_turns);
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
              put_down_sf_sprites(sbuf, ebuf, x, y, len, units_per_px, ttl_turns);
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
    put_down_sf_sprites(sbuf, ebuf, x, y, len, units_per_px, ttl_turns);
    LbScreenLoadGraphicsWindow(&grwnd);
    *posy = starty - justifyy + h;
    return true;
}

/******************************************************************************/
