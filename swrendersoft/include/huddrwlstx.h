/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file huddrwlstx.h
 *     Header file for huddrwlstm.c.
 * @par Purpose:
 *     Drawlist execution for the HUD over 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef HUDDRWLSTX_H
#define HUDDRWLSTX_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct TbSprite;

struct DIScrPoint {
    short X;
    short Y;
};

struct DIScrRect {
    short X;
    short Y;
    short Width;
    short Height;
};

struct DIMapPoint {
    short X;
    short Y;
    short Z;
};

struct DIHudLine {
    struct DIScrPoint Beg;
    struct DIScrPoint End;
    short Timer;
    ushort DrwFlags;
    ubyte Thick;
    ubyte Bright;
    ubyte Col;
};

struct DIHudBox {
    struct DIScrRect Rect;
    short Timer;
    ushort DrwFlags;
    ubyte Bright;
    ubyte Col;
};

struct DIHudTexturedBox {
    struct DIScrRect Rect;
    ubyte VecMode;
    ubyte Bright;
    ubyte TMapNo;
    ushort NumUa;
    ushort NumVb;
};

struct DIHudSprite {
    struct DIScrRect Rect;
    struct TbSprite *pSpr;
    short Timer;
    ushort DrwFlags;
    ubyte Bright;
    ubyte Col;
};

struct DIHudClippedText {
    struct DIScrRect Rect;
    struct TbSprite *pFont;
    const char *Text;
    struct DIScrPoint Shift;
    ubyte Scale;
    ubyte Bright;
    ubyte Col;
    ubyte Shade;
};

struct DIHudWrappedText {
    struct DIScrRect Rect;
    struct TbSprite *pFont;
    const char *Text;
    short Timer;
    ushort DrwFlags;
    ubyte Scale;
    ubyte Bright;
    ubyte Col;
    ubyte Shade;
};

struct DIHudMapCoordLine {
    struct DIMapPoint PtBeg;
    struct DIMapPoint PtEnd;
    ushort DrwFlags;
    ubyte Thick;
    ubyte Col;
};

struct DIHudMapCoordFrame {
    struct DIMapPoint Pt;
    ushort Frame;
    ushort DrwFlags;
    ubyte Scale;
    ubyte Bright;
    ubyte Col;
};

struct DIHudMapCoordSprList {
    struct DIMapPoint Pt;
    struct TbSprite * const *pSprLst;
    ushort DrwFlags;
    ushort Radius;
    ubyte Scale;
    ubyte Bright;
    ubyte Col;
};

struct DrawItemHud {
	union {
        struct DIHudLine Line;
        struct DIHudBox Box;
        struct DIHudTexturedBox TxtrdBox;
        struct DIHudSprite Sprite;
		struct DIHudClippedText ClpText;
		struct DIHudWrappedText WrpText;
        struct DIHudMapCoordLine MapCorLine;
        struct DIHudMapCoordFrame MapCorFrame;
        struct DIHudMapCoordSprList MapCorSprLst;
	} U;
	ubyte Type;
	ushort Flags;
};

#pragma pack()
/******************************************************************************/

void hud_draw_line(struct DIHudLine *p_diLine);

void hud_draw_box(struct DIHudBox *p_diBox);
void hud_draw_slant_box(struct DIHudBox *p_diBox);
void hud_draw_vslant_box(struct DIHudBox *p_diBox);

void hud_draw_low_trans_grey_box(struct DIHudBox *p_diBox);
void hud_draw_low_trans_grey_slant_box(struct DIHudBox *p_diBox);
void hud_draw_low_trans_grey_vslant_box(struct DIHudBox *p_diBox);
void hud_draw_textured_flow_slant_box(struct DIHudTexturedBox *p_diBox);

void hud_draw_sprite(struct DIHudSprite *p_diSprite);
void hud_draw_sprite_scaled(struct DIHudSprite *p_diSprite);

void hud_draw_clipped_text(struct DIHudClippedText *p_diClpText);
void hud_draw_colour_wave_wrapped_text(struct DIHudWrappedText *p_diWrpText);
void hud_draw_shad_cl_flash_wrapped_text(struct DIHudWrappedText *p_diWrpText);

int hud_width_shad_cl_flash_wrapped_text(short px, short py,
  struct TbSprite *p_font, const char *text, ushort drwflags, short units_per_px);

void hud_draw_mapcoord_line(struct DIHudMapCoordLine *p_diMapCorLine);
void hud_draw_mapcoord_frame(struct DIHudMapCoordFrame *p_diMapCorFrame);
void hud_draw_mapcoord_sprites_in_quarters(struct DIHudMapCoordSprList *p_diMapCorSprLst);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
