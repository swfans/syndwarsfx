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
#include "enginprops.h"

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
    space_bkp = FontSpacingAlter(small_font, 12);
    ret = AppTextDrawColourWaveResized(posx, posy, units_per_px, text);
    FontSpacingRestore(small_font, space_bkp);
    return ret;
}


TbBool AppTextDrawMissionChatMessage(int posx, int *posy, int plyr, int timer,
  const char *text)
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
    space_bkp = FontSpacingAlter(small_font, 12);
    ret = AppTextDrawShadClFlashResized(posx, posy, units_per_px, timer, text);
    FontSpacingRestore(small_font, space_bkp);
    return ret;
}

/******************************************************************************/
