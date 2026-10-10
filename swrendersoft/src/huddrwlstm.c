/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file huddrwlstm.c
 *     Making drawlists for the HUD over 3D engine.
 * @par Purpose:
 *     Implements functions for filling drawlists.
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
#include "huddrwlstm.h"

#include <assert.h>
#include "bfanywnd.h"
#include "bfbox.h"
#include "bfgentab.h"
#include "bfscreen.h"
#include "bfsprite.h"
#include "bftext.h"

#include "engincolour.h"
#include "huddrwlstx.h"

/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/

TbBool enlist_hud_draw_line(short beg_x, short beg_y, short end_x, short end_y,
  ushort drwflags, ubyte thickness, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Line.Beg.X = beg_x;
    p_di->U.Line.Beg.Y = beg_y;
    p_di->U.Line.End.X = end_x;
    p_di->U.Line.End.Y = end_y;
    p_di->U.Line.DrwFlags = drwflags;
    p_di->U.Line.Thick = thickness;
    p_di->U.Line.Bright = brig;
    p_di->U.Line.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_line(&p_di->U.Line);
    return true;
}

TbBool enlist_hud_draw_box(short px, short py, short width, short height,
  ushort drwflags, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Box.Rect.X = px;
    p_di->U.Box.Rect.Y = py;
    p_di->U.Box.Rect.Width = width;
    p_di->U.Box.Rect.Height = height;
    p_di->U.Box.DrwFlags = drwflags;
    p_di->U.Box.Bright = brig;
    p_di->U.Box.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_box(&p_di->U.Box);
    return true;
}

TbBool enlist_hud_draw_slant_box(short px, short py, short width, short height,
  ushort drwflags, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Box.Rect.X = px;
    p_di->U.Box.Rect.Y = py;
    p_di->U.Box.Rect.Width = width;
    p_di->U.Box.Rect.Height = height;
    p_di->U.Box.DrwFlags = drwflags;
    p_di->U.Box.Bright = brig;
    p_di->U.Box.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_slant_box(&p_di->U.Box);
    return true;
}

TbBool enlist_hud_draw_vslant_box(short px, short py, short width, short height,
  ushort drwflags, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Box.Rect.X = px;
    p_di->U.Box.Rect.Y = py;
    p_di->U.Box.Rect.Width = width;
    p_di->U.Box.Rect.Height = height;
    p_di->U.Box.DrwFlags = drwflags;
    p_di->U.Box.Bright = brig;
    p_di->U.Box.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_vslant_box(&p_di->U.Box);
    return true;
}

TbBool enlist_hud_draw_low_trans_grey_box(short px, short py,
  short width, short height, ushort drwflags, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Box.Rect.X = px;
    p_di->U.Box.Rect.Y = py;
    p_di->U.Box.Rect.Width = width;
    p_di->U.Box.Rect.Height = height;
    p_di->U.Box.DrwFlags = drwflags;
    p_di->U.Box.Bright = brig;
    p_di->U.Box.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_low_trans_grey_box(&p_di->U.Box);
    return true;
}

TbBool enlist_hud_draw_low_trans_grey_slant_box(short px, short py,
  short width, short height, ushort drwflags, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Box.Rect.X = px;
    p_di->U.Box.Rect.Y = py;
    p_di->U.Box.Rect.Width = width;
    p_di->U.Box.Rect.Height = height;
    p_di->U.Box.DrwFlags = drwflags;
    p_di->U.Box.Bright = brig;
    p_di->U.Box.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_low_trans_grey_slant_box(&p_di->U.Box);
    return true;
}

TbBool enlist_hud_draw_low_trans_grey_vslant_box(short px, short py,
  short width, short height, ushort drwflags, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Box.Rect.X = px;
    p_di->U.Box.Rect.Y = py;
    p_di->U.Box.Rect.Width = width;
    p_di->U.Box.Rect.Height = height;
    p_di->U.Box.DrwFlags = drwflags;
    p_di->U.Box.Bright = brig;
    p_di->U.Box.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_low_trans_grey_vslant_box(&p_di->U.Box);
    return true;
}

TbBool enlist_hud_draw_textured_flow_slant_box(short px, short py,
  short width, short height, ubyte vecmode,
  ubyte brig, ubyte tmapno, ushort num_ua, ushort num_vb)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    p_di = &dih;

    p_di->U.TxtrdBox.Rect.X = px;
    p_di->U.TxtrdBox.Rect.Y = py;
    p_di->U.TxtrdBox.Rect.Width = width;
    p_di->U.TxtrdBox.Rect.Height = height;
    p_di->U.TxtrdBox.VecMode = vecmode;
    p_di->U.TxtrdBox.Bright = brig;
    p_di->U.TxtrdBox.TMapNo = tmapno;
    p_di->U.TxtrdBox.NumUa = num_ua;
    p_di->U.TxtrdBox.NumVb = num_vb;

    //TODO enlist instead of drawing directly
    hud_draw_textured_flow_slant_box(&p_di->U.TxtrdBox);
    return true;
}

TbBool enlist_hud_draw_sprite(short px, short py, struct TbSprite *p_spr,
  ushort drwflags, short brig)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Sprite.Rect.X = px;
    p_di->U.Sprite.Rect.Y = py;
    p_di->U.Sprite.Rect.Width = p_spr->SWidth;
    p_di->U.Sprite.Rect.Height = p_spr->SHeight;
    p_di->U.Sprite.pSpr = p_spr;
    p_di->U.Sprite.Timer = 0;
    p_di->U.Sprite.DrwFlags = drwflags;
    p_di->U.Sprite.Bright = brig;
    p_di->U.Sprite.Col = colour_lookup[ColLU_WHITE];

    //TODO enlist instead of drawing directly
    hud_draw_sprite(&p_di->U.Sprite);
    return true;
}

TbBool enlist_hud_draw_sprite_scaled(short px, short py, struct TbSprite *p_spr,
  short dest_width, short dest_height, ushort drwflags, short brig)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.Sprite.Rect.X = px;
    p_di->U.Sprite.Rect.Y = py;
    p_di->U.Sprite.Rect.Width = dest_width;
    p_di->U.Sprite.Rect.Height = dest_height;
    p_di->U.Sprite.pSpr = p_spr;
    p_di->U.Sprite.Timer = 0;
    p_di->U.Sprite.DrwFlags = drwflags;
    p_di->U.Sprite.Bright = brig;
    p_di->U.Sprite.Col = colour_lookup[ColLU_WHITE];

    //TODO enlist instead of drawing directly
    hud_draw_sprite_scaled(&p_di->U.Sprite);
    return true;
}

TbBool enlist_hud_draw_clipped_text(short px, short py, short width, short height,
  short shift_x, short shift_y, struct TbSprite *p_font, const char *text,
  short units_per_px, short brig, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    if (brig > PALETTE_FADE_LEVELS-1)
        brig = PALETTE_FADE_LEVELS-1;
    if (brig < 0)
        brig = 0;

    p_di = &dih;

    p_di->U.ClpText.Rect.X = px;
    p_di->U.ClpText.Rect.Y = py;
    p_di->U.ClpText.Rect.Width = width;
    p_di->U.ClpText.Rect.Height = height;
    p_di->U.ClpText.pFont = p_font;
    p_di->U.ClpText.Text = text;
    p_di->U.ClpText.Shift.X = shift_x;
    p_di->U.ClpText.Shift.Y = shift_y;
    p_di->U.ClpText.Scale = units_per_px;
    p_di->U.ClpText.Bright = brig;
    p_di->U.ClpText.Col = colour;
    p_di->U.ClpText.Shade = 0;

    //TODO enlist instead of drawing directly
    hud_draw_clipped_text(&p_di->U.ClpText);
    return true;
}

int get_width_shad_cl_flash_wrapped_text(short px, short py,
  struct TbSprite *p_font, const char *text, short units_per_px)
{
    ushort drwflags;

    drwflags = Lb_TEXT_ONE_COLOR | Lb_TEXT_HALIGN_LEFT;
    return hud_width_shad_cl_flash_wrapped_text(px, py,
      p_font, text, drwflags, units_per_px);
}

TbBool enlist_hud_draw_shad_cl_flash_wrapped_text(short px, short py,
  short width, short height, struct TbSprite *p_font, const char *text,
  short units_per_px, short timer, TbPixel colour, TbPixel shcolour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    p_di = &dih;

    p_di->U.WrpText.Rect.X = px;
    p_di->U.WrpText.Rect.Y = py;
    p_di->U.WrpText.Rect.Width = width;
    p_di->U.WrpText.Rect.Height = height;
    p_di->U.WrpText.pFont = p_font;
    p_di->U.WrpText.Text = text;
    p_di->U.WrpText.Timer = timer;
    p_di->U.WrpText.DrwFlags = Lb_TEXT_ONE_COLOR | Lb_TEXT_HALIGN_LEFT;
    p_di->U.WrpText.Scale = units_per_px;
    p_di->U.WrpText.Bright = 32;
    p_di->U.WrpText.Col = colour;
    p_di->U.WrpText.Shade = shcolour;

    //TODO enlist instead of drawing directly
    hud_draw_shad_cl_flash_wrapped_text(&p_di->U.WrpText);
    return true;
}

TbBool enlist_hud_draw_colour_wave_wrapped_text(short px, short py,
  short width, short height, struct TbSprite *p_font, const char *text,
  short units_per_px, TbPixel colour, TbPixel shcolour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    p_di = &dih;

    p_di->U.WrpText.Rect.X = px;
    p_di->U.WrpText.Rect.Y = py;
    p_di->U.WrpText.Rect.Width = width;
    p_di->U.WrpText.Rect.Height = height;
    p_di->U.WrpText.pFont = p_font;
    p_di->U.WrpText.Text = text;
    p_di->U.WrpText.Timer = 0;
    p_di->U.WrpText.DrwFlags = Lb_TEXT_ONE_COLOR | Lb_TEXT_HALIGN_LEFT;
    p_di->U.WrpText.Scale = units_per_px;
    p_di->U.WrpText.Bright = 32;
    p_di->U.WrpText.Col = colour;
    p_di->U.WrpText.Shade = shcolour;

    //TODO enlist instead of drawing directly
    hud_draw_colour_wave_wrapped_text(&p_di->U.WrpText);
    return true;
}

TbBool enlist_hud_draw_mapcoord_line(short cor1_x, short cor1_y,
  short cor1_z, short cor2_x, short cor2_y, short cor2_z,
  ushort drwflags, ubyte thickness, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    p_di = &dih;

    p_di->U.MapCorLine.PtBeg.X = cor1_x;
    p_di->U.MapCorLine.PtBeg.Y = cor1_y;
    p_di->U.MapCorLine.PtBeg.Z = cor1_z;
    p_di->U.MapCorLine.PtEnd.X = cor2_x;
    p_di->U.MapCorLine.PtEnd.Y = cor2_y;
    p_di->U.MapCorLine.PtEnd.Z = cor2_z;
    p_di->U.MapCorLine.DrwFlags = drwflags;
    p_di->U.MapCorLine.Thick = thickness;
    p_di->U.MapCorLine.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_mapcoord_line(&p_di->U.MapCorLine);
    return true;
}

TbBool enlist_hud_draw_mapcoord_frame_one_colour(short cor_x, short cor_y,
  short cor_z, ushort frm, ushort drwflags, TbPixel colour)
{
    struct DrawItemHud dih;
    struct DrawItemHud *p_di;

    p_di = &dih;

    p_di->U.MapCorFrame.Pt.X = cor_x;
    p_di->U.MapCorFrame.Pt.Y = cor_y;
    p_di->U.MapCorFrame.Pt.Z = cor_z;
    p_di->U.MapCorFrame.DrwFlags = drwflags;
    p_di->U.MapCorFrame.Frame = frm;
    p_di->U.MapCorFrame.Scale = 16;
    p_di->U.MapCorFrame.Bright = 32;
    p_di->U.MapCorFrame.Col = colour;

    //TODO enlist instead of drawing directly
    hud_draw_mapcoord_frame(&p_di->U.MapCorFrame);
    return true;
}

/******************************************************************************/
