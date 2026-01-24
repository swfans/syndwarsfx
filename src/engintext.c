/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engintext.c
 *     Drawing text on screen within the game engine.
 * @par Purpose:
 *     Implement functions for drawing text over the 3D world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engintext.h"

#include "bfmemut.h"
#include "bftext.h"
#include "bfscreen.h"
#include "bfsprite.h"

#include "app_text_cw.h"
#include "app_text_sf.h"
#include "engincolour.h"

#include "game_sprts.h"
#include "hud_panel.h"
#include "mydraw.h"
#include "swlog.h"
/******************************************************************************/

/** Modifies spacing of given font, by altering width of space character.
 */
ushort FontSpacingAlter(struct TbSprite *font, int units_per_px)
{
    struct TbSprite *p_spr;
    ushort space_bkp;

    p_spr = AppFontCharSpriteRW(font, ' ');
    if (p_spr == NULL)
        return 0;
    space_bkp = p_spr->SWidth;
    p_spr->SWidth = (space_bkp * units_per_px) / 16;
    return space_bkp;
}

void FontSpacingRestore(struct TbSprite *font, ushort space_bkp)
{
    struct TbSprite *p_spr;

    p_spr = AppFontCharSpriteRW(font, ' ');
    if (p_spr == NULL)
        return;
    p_spr->SWidth = space_bkp;
}


#if 0
void draw_text_linewrap1b(int base_x, int *p_pos_y, const char *text)
{
    const char *str;
    int pos_x, pos_y;
    int base_shift;
    TbPixel col2;

    col2 = lbDisplay.DrawColour;
    str = text;
    pos_x = base_x;
    base_shift = 0;
    pos_y = *p_pos_y;
    while (*str != '\0')
    {
        if (*str == 32)
        {
            const char *sstr;
            int w;

            w = 0;
            sstr = str + 1;
            while (*sstr != '\0')
            {
                const struct TbSprite *p_spr;

                if (*sstr == 32)
                    break;
                p_spr =  LbFontCharSprite(lbFontPtr, my_char_to_upper(*sstr));
                w += p_spr->SWidth;
                sstr++;
            }
            if (pos_x + 2 * w < lbDisplay.PhysicalScreenWidth - 16) {
                pos_x += 8;
            } else {
                pos_x = base_x;
                pos_y += 12;
            }
        }
        else
        {
            const struct TbSprite *p_spr;
            ushort fade_lv;

            fade_lv = 40 - (lbSinTable[128 * ((render_anim_turn + base_shift) & 0xF)] >> 13);
            p_spr =  LbFontCharSprite(lbFontPtr, my_char_to_upper(*str));
            AppSpriteDrawDoubleOneColour(p_spr, pos_x + 1, pos_y + 1, colour_lookup[ColLU_BLACK]);
            AppSpriteDrawDoubleOneColour(p_spr, pos_x, pos_y, pixmap.fade_table[256 * fade_lv + col2]);
            pos_x += p_spr->SWidth + p_spr->SWidth;
        }
        base_shift++;
        str++;
    }
    pos_y += 12;
    *p_pos_y = pos_y;
}

void draw_text_linewrap2b(int base_x, int *p_pos_y, const char *text)
{
    const char *str;
    int pos_x, pos_y;
    int base_shift;
    TbPixel col2;
    int cw_base, cw_vari;

    cw_base = 32;
    cw_vari = 16;
    col2 = lbDisplay.DrawColour;
    pos_x = base_x;
    str = text;
    pos_y = *p_pos_y;
    base_shift = 0;
    while (*str != '\0')
    {
        if (*str == 32)
        {
            const char *sstr;
            int w;

            w = 0;
            sstr = str + 1;
            while (*sstr != '\0')
            {
                const struct TbSprite *p_spr;

                if (*sstr == 32)
                  break;
                p_spr =  LbFontCharSprite(lbFontPtr, my_char_to_upper(*sstr));
                w += p_spr->SWidth;
                sstr++;
            }
            if (pos_x + w < lbDisplay.PhysicalScreenWidth - 8) {
                pos_x += 4;
            } else {
                pos_x = base_x;
                pos_y += 6;
            }
        }
        else
        {
            const struct TbSprite *p_spr;
            ushort fade_lv;

            fade_lv = cw_base + cw_vari/2 - (cw_vari/2 * lbSinTable[LbFPMath_PI/8 * ((render_anim_turn + base_shift) & 0xF)] >> 16);
            p_spr =  LbFontCharSprite(lbFontPtr, my_char_to_upper(*str));
            LbSpriteDrawOneColour(pos_x + 1, pos_y + 1, p_spr, colour_lookup[ColLU_BLACK]);
            LbSpriteDrawOneColour(pos_x, pos_y,  p_spr, pixmap.fade_table[fade_lv * PALETTE_8b_COLORS + col2]);
            pos_x += p_spr->SWidth;
        }
        str++;
        base_shift++;
    }
    pos_y += 6;
    *p_pos_y = pos_y;
}
#endif

TbBool AppTextDrawMissionStatus(int posx, int posy, const char *text)
{
    ushort space_bkp;
    int tx_height;
    int units_per_px;
    TbBool ret;

    lbFontPtr = small_font;
    tx_height = my_char_height('A');
    // For window width=320, expect text height=5; so that should
    // produce unscaled sprite, which is 16 units per px.
    units_per_px = (lbDisplay.GraphicsWindowWidth * 5 / tx_height)  / (320 / 16);
    // Do not allow any scale, only n * 50%
    units_per_px = (units_per_px + 4) & ~0x07;

    lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR | Lb_TEXT_HALIGN_LEFT;
#if defined(LB_ENABLE_SHADOW_COLOUR)
    lbDisplay.ShadowColour = colour_lookup[ColLU_BLACK];
#endif
#if 0 // old way of drawing mission status - remove pending
    if (render_anim_turn & 0x40) {
    if (units_per_px < 24)
        draw_text_linewrap2b(posx, &posy, text);
    else
        draw_text_linewrap1b(posx, &posy, text);
    return true;
    }
#endif
    space_bkp = FontSpacingAlter(small_font, 12);
    ret = AppTextDrawColourWaveResized(posx, posy, units_per_px, text);
    FontSpacingRestore(small_font, space_bkp);
    return ret;
}

#if 0
void draw_text_linewrap1(int base_x, int *p_pos_y, int plyr, const char *text)
{
    const char *str;
    int pos_x, pos_y;
    int base_shift;
    TbPixel col2;

    str = text;
    pos_x = base_x;
    pos_y = *p_pos_y;
    col2 = lbDisplay.DrawColour;
    base_shift = -180;
    while (*str != '\0')
    {
        if (*str == 32)
        {
            if (pos_x + 2 * font_word_length(str + 1) < lbDisplay.PhysicalScreenWidth - 16) {
                pos_x += 8;
            } else {
                pos_x = base_x;
                pos_y += 12;
            }
        }
        else
        {
            const struct TbSprite *p_spr;
            int fd;
            ubyte ch;
            TbPixel col1;

            ch = my_char_to_upper(*str);
            p_spr =  LbFontCharSprite(lbFontPtr, ch);
            fd = base_shift + 4 * player_unkn0C9[plyr];
            if (fd > 63)
                fd = 63 - (fd - 63);
            if (fd > 63)
                fd = 63;
            if (fd < 0)
                fd = 0;
            col1 = pixmap.fade_table[256 * fd + colour_lookup[ColLU_GREYLT]];
            AppSpriteDrawDoubleOneColour(p_spr, pos_x + 1, pos_y + 1, col1);
            AppSpriteDrawDoubleOneColour(p_spr, pos_x, pos_y, col2);
            pos_x += 2 * p_spr->SWidth;
        }
        str++;
        base_shift++;
    }
    pos_y += 12;
    *p_pos_y = pos_y;
}

void draw_text_linewrap2(int base_x, int *p_pos_y, int plyr, const char *text)
{
    const char *str;
    int pos_x, pos_y;
    int base_shift;
    TbPixel col2;

    str = text;
    pos_x = base_x;
    pos_y = *p_pos_y;
    col2 = lbDisplay.DrawColour;
    base_shift = -180;
    while (*str != '\0')
    {
        if (*str == 32)
        {
            if (pos_x + font_word_length(str + 1) < lbDisplay.PhysicalScreenWidth - 8) {
                pos_x += 4;
            } else {
                pos_x = base_x;
                pos_y += 6;
            }
        }
        else
        {
            const struct TbSprite *p_spr;
            int fd;
            ubyte ch;
            TbPixel col1;

            ch = my_char_to_upper(*str);
            p_spr =  LbFontCharSprite(lbFontPtr, ch);
            fd = base_shift + 4 * (ubyte)player_unkn0C9[plyr];
            if (fd > 63)
                fd = 63 - (fd - 63);
            if (fd > 63)
                fd = 63;
            if (fd < 0)
                fd = 0;
            col1 = pixmap.fade_table[256 * fd + colour_lookup[ColLU_GREYLT]];
            LbSpriteDrawOneColour(pos_x + 1, pos_y + 1, p_spr, col1);
            LbSpriteDrawOneColour(pos_x, pos_y, p_spr, col2);
            pos_x += p_spr->SWidth;
        }
        str++;
        base_shift++;
    }
    pos_y += 6;
    *p_pos_y = pos_y;
}
#endif

TbBool AppTextDrawMissionChatMessage(int posx, int *posy, int plyr, const char *text)
{
    ushort space_bkp;
    int tx_height;
    int units_per_px;
    TbBool ret;

    lbFontPtr = small_font;
    tx_height = my_char_height('A');
    // For window width=320, expect text height=5; so that should
    // produce unscaled sprite, which is 16 units per px.
    units_per_px = (lbDisplay.GraphicsWindowWidth * 5 / tx_height)  / (320 / 16);
    // Do not allow any scale, only n * 50%
    units_per_px = (units_per_px + 4) & ~0x07;

    lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR | Lb_TEXT_HALIGN_LEFT;
#if defined(LB_ENABLE_SHADOW_COLOUR)
    lbDisplay.ShadowColour = colour_lookup[ColLU_GREYLT];
#endif
#if 0 // old way of drawing mission status - remove pending
    if (render_anim_turn & 0x20) {
    if (units_per_px < 24)
        draw_text_linewrap2(posx, posy, plyr, text);
    else
        draw_text_linewrap1(posx, posy, plyr, text);
    return true;
    }
#endif
    space_bkp = FontSpacingAlter(small_font, 12);
    ret = AppTextDrawShadClFlashResized(posx, posy, units_per_px, player_unkn0C9[plyr], text);
    FontSpacingRestore(small_font, space_bkp);
    return ret;
}

/******************************************************************************/
