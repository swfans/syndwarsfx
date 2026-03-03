/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstx.h
 *     Header file for engindrwlstx.c.
 * @par Purpose:
 *     Drawlists execution for the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINDRWLSTX_H
#define ENGINDRWLSTX_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#pragma pack(1)

struct DrawItem {
    ubyte Type;
    ushort Offset;
    ushort Child;
};

struct SpecialPoint {
    short X;
    short Y;
    short Z;
    short PadTo8;
};

struct SortSprite {
    short X;
    short Y;
    short Z;
    ushort Frame;
    intptr_t SrcItem;
    ubyte Brightness;
    ubyte Angle;
    short Scale;
};

struct SortLine {
    short X1;
    short Y1;
    short X2;
    short Y2;
    ubyte Col;
    ubyte Shade;
    ubyte Flags;
};

/** Map coordinates storage used when handling drawlists.
 *
 * The game should have its own types for map coordinates, this one is for
 * the rendering only.
 */
struct SortMapPoint {
    s32 X;
    s32 Y;
    s32 Z;
};

struct TbSprite;
struct PolyPoint;

typedef void (*ScreenTriangleRenderCallback)(
  struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2,
  struct PolyPoint *p_pt3,
  ushort face, ubyte type);
typedef void (*ScreenSortSpriteRenderCallback)(ushort sspr);

#pragma pack()
/******************************************************************************/
extern struct DrawItem *game_draw_list;
extern struct DrawItem *p_current_draw_item;
extern ushort next_draw_item;

/** Array of triangular faces valid only as part of drawlist for a single frame.
 */
extern struct SingleObjectFace3 *game_special_object_faces;
extern ushort next_special_face;

/** Array of rectangular faces valid only as part of drawlist for a single frame.
 */
extern struct SingleObjectFace4 *game_special_object_faces4;
extern ushort next_special_face4;

/** Array of points with items valid only as part of drawlist for a single frame.
 */
extern struct SpecialPoint *game_screen_point_pool;
extern ushort next_screen_point;

extern struct SortSprite *game_sort_sprites;
extern struct SortSprite *p_current_sort_sprite;
extern ushort next_sort_sprite;

extern struct SortLine *game_sort_lines;
extern struct SortLine *p_current_sort_line;
extern ushort next_sort_line;

extern TbPixel deep_radar_surface_col;
extern TbPixel deep_radar_line_col;

extern ubyte engine_render_lights;

extern ScreenTriangleRenderCallback screen_position_face_render_cb;
extern ScreenSortSpriteRenderCallback screen_sorted_sprite_statc_render_cb;
extern ScreenSortSpriteRenderCallback screen_sorted_sprite_persn_render_cb;

/******************************************************************************/

void draw_frame_scaled_alpha(int scr_x, int scr_y, ushort frm,
  ushort scale, ushort alpha);
void draw_sorted_sprite1a(ushort frm, short x, short y, ubyte csel);
void draw_sort_sprite1a(ushort sspr);

void draw_floor_tile1a(ushort tl);
void draw_floor_tile1b(ushort tl);

void set_nuclear_shade_point(s32 x, s32 y, s32 z);
void set_nuclear_shade_timer(ulong tmval);

void draw_drawitem_1(ushort dihead);
void draw_drawitem_2(ushort dihead);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
