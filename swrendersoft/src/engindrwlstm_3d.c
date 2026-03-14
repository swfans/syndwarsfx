/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstm_3d.c
 *     Making drawlists for the 3D engine, 3D items drawing.
 * @par Purpose:
 *     Implements functions for filling drawlists.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engindrwlstm.h"

#include "bfmath.h"
#include "bfmemut.h"
#include "bfutility.h"
#include <string.h>

#include "enginbckt.h"
#include "engincam.h"
#include "engindrwlstx.h"
#include "enginpeff.h"
#include "enginprops.h"
#include "enginshrapn.h"
#include "engintrns.h"

/******************************************************************************/
#pragma pack(1)

/** Packs sprite frame versions into one, 16-bit field */
#define SPR_FRAME_VERSIONS_PACK(v0, v1, v2, v3, v4) ((v0 << 0) + (v1 << 3) + (v2 << 6) + (v3 << 9) + (v4 << 12))

#pragma pack()
/******************************************************************************/
ushort enlist_draw_mapwho_vect(int x1, int y1, int z1, int x2, int y2, int z2, int col)
{
    struct ShEnginePoint sp1, sp2;
    struct SortLine *p_sline;
    int bckt;
    ushort sline;

    transform_shpoint(&sp1, x1, 8 * y1 - 8 * engn_yc, z1);
    transform_shpoint(&sp2, x2, 8 * y2 - 8 * engn_yc, z2);

    if ((sp2.Flags & sp1.Flags & 0xF) != 0) {
        return 0;
    }

    bckt = BUCKET_MID + sp1.Depth;

    sline = next_sort_line;
    p_sline = draw_item_add_line(DrIT_Unkn11, bckt);
    if (p_sline == NULL) {
        return 0;
    }

    p_sline->Shade = 32;
    p_sline->Flags = 0;
    p_sline->X1 = sp1.X;
    p_sline->Y1 = sp1.Y;
    p_sline->X2 = sp2.X;
    p_sline->Y2 = sp2.Y;
    p_sline->Col = col;

    return sline;
}

void enlist_draw_mapwho_vect_len(int x1, int y1, int z1,
  int x2, int y2, int z2, int len, int col)
{
    int dt_x, dt_y, dt_z;
    int dist;
    int x3, y3, z3;

    dt_y = y2 - y1;
    dt_z = z2 - z1;
    dt_x = x2 - x1;

    dist = LbSqrL(dt_z * dt_z + dt_x * dt_x + dt_y * dt_y);
    if (dist == 0)
        dist = 1;
    y3 = y1 + dt_y * len / dist;
    z3 = z1 + dt_z * len / dist;
    x3 = x1 + dt_x * len / dist;

    enlist_draw_mapwho_vect(x1, y1, z1, x3, y3, z3, col);
}

void enlist_draw_frame_graphic(int x, int y, int z, ushort frame,
  int radius, int intensity, int depth_shift, intptr_t p_sitm)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[render_anim_turn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - ((radius * overall_scale) >> 8) + depth_shift;

    p_sspr = draw_item_add_sprite(DrIT_SFrmStatc, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y;
    p_sspr->Z = scr_depth;
    p_sspr->Frame = frame;
    p_sspr->Brightness = intensity;
    p_sspr->Scale = 256;
    p_sspr->SrcItem = p_sitm;
}

void enlist_draw_frame_graphic_scale(int x, int y, int z, ushort frame,
  int radius, int intensity, int scale, intptr_t p_sitm)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[render_anim_turn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - (((radius - 100) * overall_scale) >> 8);

    p_sspr = draw_item_add_sprite(DrIT_Unkn15, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y;
    p_sspr->Z = scr_depth;
    p_sspr->Frame = frame;
    p_sspr->Brightness = intensity;
    p_sspr->Scale = scale;
    p_sspr->SrcItem = p_sitm;
}

void enlist_draw_tall_spr_shadow(int scr_x, int scr_y, int scr_depth, ushort frm,
  ubyte angl, ubyte shangl, ushort shpak, short strng, intptr_t p_sitm)
{
    struct SortSprite *p_sspr;

    p_sspr = draw_item_add_sprite(DrIT_SPersShdw, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = scr_x;
    p_sspr->Y = scr_y;
    p_sspr->Z = shpak;
    p_sspr->Frame = frm;
    p_sspr->Brightness = shangl;
    p_sspr->Angle = angl;
    p_sspr->Scale = strng;
    p_sspr->SrcItem = p_sitm;
}

void enlist_draw_frame_pers_basic(int scr_x, int scr_y, int scr_depth, int frame,
  ubyte angl, short bright, intptr_t p_sitm)
{
    struct SortSprite *p_sspr;

    p_sspr = draw_item_add_sprite(DrIT_SFrmPersB, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = scr_x;
    p_sspr->Y = scr_y;
    p_sspr->Z = 0;
    p_sspr->Frame = frame;
    p_sspr->Brightness = bright;
    p_sspr->Angle = angl;
    p_sspr->Scale = 256;
    p_sspr->SrcItem = p_sitm;
}

void enlist_draw_frame_pers_rot_versioned(int scr_x, int scr_y, int scr_depth,
  int frame, ubyte *frv, ubyte angl, short bright, intptr_t p_sitm)
{
    struct SortSprite *p_sspr;

    p_sspr = draw_item_add_sprite(DrIT_SFrmPersV, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = scr_x;
    p_sspr->Y = scr_y;
    p_sspr->Z = 0;
    p_sspr->Frame = frame;
    p_sspr->Brightness = bright;
    p_sspr->Angle = angl;
    p_sspr->Scale = SPR_FRAME_VERSIONS_PACK(frv[0], frv[1], frv[2], frv[3], frv[4]);
    p_sspr->SrcItem = p_sitm;
}

void enlist_draw_frame_effect_versioned(int scr_x, int scr_y, int scr_depth,
  int frame, ubyte *frv, ubyte angl, short bright, intptr_t p_sitm)
{
    struct SortSprite *p_sspr;

    p_sspr = draw_item_add_sprite(DrIT_SFrmEfctV, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = scr_x;
    p_sspr->Y = scr_y;
    p_sspr->Z = 0;
    p_sspr->Frame = frame;
    p_sspr->Brightness = bright;
    p_sspr->Angle = angl;
    p_sspr->Scale = SPR_FRAME_VERSIONS_PACK(frv[0], frv[1], frv[2], frv[3], frv[4]);
    p_sspr->SrcItem = p_sitm;
}

void enlist_draw_fire_flames(ushort flame_beg)
{
    struct FireFlame *p_flame;
    ushort flm;

    for (flm = flame_beg; flm != 0; flm = p_flame->next)
    {
        struct ShEnginePoint sp;
        struct SpecialPoint *p_scrpoint;
        int cor_dx, cor_dy, cor_dz;

        p_flame = &FIRE_flame[flm];
        cor_dx = p_flame->x - engn_xc;
        cor_dy = p_flame->y - engn_yc;
        cor_dz = p_flame->z - engn_zc;

        if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
            cor_dy += waft_table[render_anim_turn & 0x1F];

        transform_shpoint(&sp, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        p_flame->PointOffset = next_screen_point;
        p_scrpoint = draw_item_add_points(DrIT_SFireFlame, flm, BUCKET_MID + sp.Depth - 50, 1);
        if (p_scrpoint == NULL) {
            break;
        }

        p_scrpoint->X = sp.X;
        p_scrpoint->Y = sp.Y;
        p_scrpoint->Z = sp.Depth;
    }
}

void enlist_draw_number(int x, int y, int z, short scr_dx, short scr_dy,
  int num, int radius, TbPixel colour)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[render_anim_turn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - ((radius * overall_scale) >> 8);

    p_sspr = draw_item_add_sprite(DrIT_Number, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = sp.X + scr_dx;
    p_sspr->Y = sp.Y + scr_dy;
    p_sspr->Z = scr_depth;
    p_sspr->Frame = colour;
    p_sspr->Brightness = 0;
    p_sspr->Scale = 256;
    p_sspr->SrcItem = (intptr_t)num;
}

void enlist_draw_text(int x, int y, int z, short scr_dx, short scr_dy,
  const char *text, int radius, TbPixel colour)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[render_anim_turn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - ((radius * overall_scale) >> 8);

    p_sspr = draw_item_add_sprite(DrIT_ShortText, BUCKET_MID + scr_depth);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = sp.X + scr_dx;
    p_sspr->Y = sp.Y + scr_dy;
    p_sspr->Z = scr_depth;
    p_sspr->Frame = colour;
    LbMemoryCopy(&p_sspr->SrcItem, text, min(strlen(text)+1, 8));
}

/******************************************************************************/
