/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file huddrwlstx.c
 *     Drawlist execution for the HUD over 3D engine.
 * @par Purpose:
 *     Implements functions for executing previously made drawlists,
 *     meaning the actual drawing based on primitives in the list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "huddrwlstx.h"

#include <assert.h>
#include "bfanywnd.h"
#include "bfbox.h"
#include "bfgentab.h"
#include "bfline.h"
#include "bfscreen.h"
#include "bfsprite.h"
#include "bftext.h"

#include "app_box.h"
#include "app_sprite.h"
#include "app_text_ba.h"
#include "app_text_cw.h"
#include "app_text_sf.h"
#include "drawshape.h"
#include "engincam.h"
#include "engindrwlstx.h"
#include "engintrns.h"
#include "sprfontut.h"

/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/

void hud_draw_line(struct DIHudLine *p_diLine)
{
    lbDisplay.DrawFlags = p_diLine->DrwFlags;

    LbDrawLine(p_diLine->Beg.X, p_diLine->Beg.Y,
      p_diLine->End.X, p_diLine->End.Y, p_diLine->Col);
}

void hud_draw_box(struct DIHudBox *p_diBox)
{
    lbDisplay.DrawFlags = p_diBox->DrwFlags;

    LbDrawBox(p_diBox->Rect.X, p_diBox->Rect.Y,
      p_diBox->Rect.Width, p_diBox->Rect.Height, p_diBox->Col);
}

void hud_draw_slant_box(struct DIHudBox *p_diBox)
{
    lbDisplay.DrawFlags = p_diBox->DrwFlags;

    AppDrawSlantBox(p_diBox->Rect.X, p_diBox->Rect.Y,
      p_diBox->Rect.Width, p_diBox->Rect.Height, p_diBox->Col);
}

void hud_draw_vslant_box(struct DIHudBox *p_diBox)
{
    lbDisplay.DrawFlags = p_diBox->DrwFlags;

    AppDrawVSlantBox(p_diBox->Rect.X, p_diBox->Rect.Y,
      p_diBox->Rect.Width, p_diBox->Rect.Height, p_diBox->Col);
}

void hud_draw_low_trans_grey_box(struct DIHudBox *p_diBox)
{
    low_trans_grey_brightness = p_diBox->Bright;
    lbDisplay.DrawFlags = p_diBox->DrwFlags;

    AppDrawBoxLowTransGrey(p_diBox->Rect.X, p_diBox->Rect.Y,
      p_diBox->Rect.Width, p_diBox->Rect.Height, p_diBox->Col);
}

void hud_draw_low_trans_grey_slant_box(struct DIHudBox *p_diBox)
{
    low_trans_grey_brightness = p_diBox->Bright;
    lbDisplay.DrawFlags = p_diBox->DrwFlags;

    AppDrawSlantBoxLowTransGrey(p_diBox->Rect.X, p_diBox->Rect.Y,
      p_diBox->Rect.Width, p_diBox->Rect.Height, p_diBox->Col);
}

void hud_draw_low_trans_grey_vslant_box(struct DIHudBox *p_diBox)
{
    low_trans_grey_brightness = p_diBox->Bright;
    lbDisplay.DrawFlags = p_diBox->DrwFlags;

    AppDrawVSlantBoxLowTransGrey(p_diBox->Rect.X, p_diBox->Rect.Y,
      p_diBox->Rect.Width, p_diBox->Rect.Height, p_diBox->Col);
}

void hud_draw_textured_flow_slant_box(struct DIHudTexturedBox *p_diTxtrdBox)
{
    lbDisplay.DrawFlags = 0;

    AppDrawTexturedFlowSlantBox(p_diTxtrdBox->Rect.X, p_diTxtrdBox->Rect.Y,
      p_diTxtrdBox->Rect.Width, p_diTxtrdBox->Rect.Height, p_diTxtrdBox->VecMode,
      p_diTxtrdBox->Bright, p_diTxtrdBox->TMapNo,
      p_diTxtrdBox->NumUa, p_diTxtrdBox->NumVb);
}

void hud_draw_sprite(struct DIHudSprite *p_diSprite)
{
    lbDisplay.DrawFlags = p_diSprite->DrwFlags;

    if ((lbDisplay.DrawFlags & (Lb_SPRITE_TRANSPAR4|Lb_SPRITE_TRANSPAR8)) != 0)
    {
        low_trans_grey_brightness = p_diSprite->Bright;
        ApSpriteDrawLowTransGreyRemap(p_diSprite->Rect.X, p_diSprite->Rect.Y,
          p_diSprite->pSpr,
          &pixmap.fade_table[0 * PALETTE_8b_COLORS]);
    }
    else if (p_diSprite->Bright != 32)
    {
        LbSpriteDrawRemap(p_diSprite->Rect.X, p_diSprite->Rect.Y,
          p_diSprite->pSpr,
          &pixmap.fade_table[p_diSprite->Bright * PALETTE_8b_COLORS]);
    }
    else
    {
        LbSpriteDraw(p_diSprite->Rect.X, p_diSprite->Rect.Y,
          p_diSprite->pSpr);
    }
}

void hud_draw_sprite_scaled(struct DIHudSprite *p_diSprite)
{
    lbDisplay.DrawFlags = p_diSprite->DrwFlags;

    if ((lbDisplay.DrawFlags & (Lb_SPRITE_TRANSPAR4|Lb_SPRITE_TRANSPAR8)) != 0)
    {
        low_trans_grey_brightness = p_diSprite->Bright;
        ApSpriteDrawScaledLowTransGreyRemap(p_diSprite->Rect.X, p_diSprite->Rect.Y,
          p_diSprite->pSpr, p_diSprite->Rect.Width, p_diSprite->Rect.Height,
          &pixmap.fade_table[0 * PALETTE_8b_COLORS]);
    }
    else if (p_diSprite->Bright != 32)
    {
        LbSpriteDrawScaledRemap(p_diSprite->Rect.X, p_diSprite->Rect.Y,
          p_diSprite->pSpr, p_diSprite->Rect.Width, p_diSprite->Rect.Height,
          &pixmap.fade_table[p_diSprite->Bright * PALETTE_8b_COLORS]);
    }
    else
    {
        LbSpriteDrawScaled(p_diSprite->Rect.X, p_diSprite->Rect.Y,
          p_diSprite->pSpr, p_diSprite->Rect.Width, p_diSprite->Rect.Height);
    }
}

void hud_draw_clipped_text(struct DIHudClippedText *p_diClpText)
{
    LbTextSetWindow(p_diClpText->Rect.X, p_diClpText->Rect.Y,
      p_diClpText->Rect.Width, p_diClpText->Rect.Height);

    lbFontPtr = p_diClpText->pFont;
    lbDisplay.DrawColour = p_diClpText->Col;
    lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
    AppTextDrawLineBrigAdjWthPartsResized(p_diClpText->Shift.X, p_diClpText->Shift.Y,
      p_diClpText->Scale, p_diClpText->Bright, p_diClpText->Text);

    LbTextSetWindow(lbDisplay.GraphicsWindowX, lbDisplay.GraphicsWindowY,
      lbDisplay.GraphicsWindowWidth, lbDisplay.GraphicsWindowHeight);
}

int hud_width_shad_cl_flash_wrapped_text(short px, short py,
  struct TbSprite *p_font, const char *text, ushort drwflags, short units_per_px)
{
    ushort space_bkp;
    int height;

    lbFontPtr = p_font;
    lbDisplay.DrawFlags = drwflags;
    space_bkp = FontSpacingAlter(p_font, 12);
    height = LbTextWrapStringHeightResized(px, py, units_per_px, text);
    FontSpacingRestore(p_font, space_bkp);
    return height;
}

void hud_draw_shad_cl_flash_wrapped_text(struct DIHudWrappedText *p_diWrpText)
{
    ushort space_bkp;

    lbFontPtr = p_diWrpText->pFont;
    lbDisplay.DrawColour = p_diWrpText->Col;
    lbDisplay.DrawFlags = p_diWrpText->DrwFlags;
#if defined(LB_ENABLE_SHADOW_COLOUR)
    lbDisplay.ShadowColour = p_diWrpText->Shade;
#endif
    space_bkp = FontSpacingAlter(p_diWrpText->pFont, 12);
    AppTextDrawShadClFlashResized(p_diWrpText->Rect.X, p_diWrpText->Rect.Y,
      p_diWrpText->Scale, p_diWrpText->Timer, p_diWrpText->Text);
    FontSpacingRestore(p_diWrpText->pFont, space_bkp);
}

void hud_draw_colour_wave_wrapped_text(struct DIHudWrappedText *p_diWrpText)
{
    ushort space_bkp;

    lbFontPtr = p_diWrpText->pFont;
    lbDisplay.DrawColour = p_diWrpText->Col;
    lbDisplay.DrawFlags = p_diWrpText->DrwFlags;
#if defined(LB_ENABLE_SHADOW_COLOUR)
    lbDisplay.ShadowColour = p_diWrpText->Shade;
#endif
    space_bkp = FontSpacingAlter(p_diWrpText->pFont, 12);
    AppTextDrawColourWaveResized(p_diWrpText->Rect.X, p_diWrpText->Rect.Y,
      p_diWrpText->Scale, p_diWrpText->Text);
    FontSpacingRestore(p_diWrpText->pFont, space_bkp);
}

void hud_draw_mapcoord_line(struct DIHudMapCoordLine *p_diMapCorLine)
{
    lbDisplay.DrawFlags = p_diMapCorLine->DrwFlags;

    draw_line_transformed_col(
      p_diMapCorLine->PtBeg.X, p_diMapCorLine->PtBeg.Y, p_diMapCorLine->PtBeg.Z,
      p_diMapCorLine->PtEnd.X, p_diMapCorLine->PtEnd.Y, p_diMapCorLine->PtEnd.Z,
      p_diMapCorLine->Col);
}

void hud_draw_mapcoord_frame(struct DIHudMapCoordFrame *p_diMapCorFrame)
{
    struct EnginePoint ep;
    int pp_X, pp_Y;

    ep.X3d = p_diMapCorFrame->Pt.X - engn_xc;
    ep.Y3d = 8 * p_diMapCorFrame->Pt.Y - engn_yc;
    ep.Z3d = p_diMapCorFrame->Pt.Z - engn_zc;
    ep.Flags = 0;
    transform_point(&ep);

    pp_X = ep.pp.X;
    pp_Y = ep.pp.Y;

    lbDisplay.DrawFlags = p_diMapCorFrame->DrwFlags;

    if (p_diMapCorFrame->Col != 0) {
        draw_frame_unscaled_one_colour(pp_X, pp_Y, p_diMapCorFrame->Frame,
          p_diMapCorFrame->Col);
    } else {
        draw_frame_unscaled(pp_X, pp_Y, p_diMapCorFrame->Frame);
    }
}

/******************************************************************************/
