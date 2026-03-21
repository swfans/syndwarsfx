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

#include "bfendian.h"
#include "bfmath.h"
#include "bfmemut.h"
#include "bfutility.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "engindrwlstx.h"
#include "enginpeff.h"
#include "enginprops.h"
#include "enginshrapn.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintrns.h"

/******************************************************************************/
#pragma pack(1)

/** Packs sprite frame versions into one, 16-bit field */
#define SPR_FRAME_VERSIONS_PACK(v0, v1, v2, v3, v4) ((v0 << 0) + (v1 << 3) + (v2 << 6) + (v3 << 9) + (v4 << 12))

ushort zig_zag[] = {
  256, 240, 360, 265, 256, 270, 240, 256,
  253, 272, 250, 246, 240, 260, 265, 256,
  270, 240, 156, 203, 272, 250, 266, 240,
  260, 295, 216, 270, 240, 256, 253, 292,
  220, 246, 255, 260, 265, 156, 270, 240,
  256, 203, 272, 210, 266, 245, 260, 265,
  356, 270, 240, 256, 293, 272, 250,
};

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

void enlist_draw_bang_phwoars(ushort phwoar_beg)
{
    struct Phwoar *p_phwoar;
    ushort phw;

    for (phw = phwoar_beg; phw != 0; phw = p_phwoar->child)
    {
        struct ShEnginePoint sp;
        struct SpecialPoint *p_scrpoint;
        int cor_dx, cor_dy, cor_dz;

        p_phwoar = &phwoar[phw];
        cor_dx = (p_phwoar->x >> 8) - engn_xc;
        cor_dz = (p_phwoar->z >> 8) - engn_zc;
        cor_dy = (p_phwoar->y >> 5) - engn_yc;

        transform_shpoint(&sp, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        p_phwoar->PointOffset = next_screen_point;
        p_scrpoint = draw_item_add_points(DrIT_SFrmPhwoar, phw, BUCKET_MID + sp.Depth - 100, 1);
        if (p_scrpoint == NULL) {
            break;
        }

        p_scrpoint->X = sp.X;
        p_scrpoint->Y = sp.Y;
        p_scrpoint->Z = sp.Depth;
    }
}

void enlist_draw_bang_shrapnels(ushort shrapnel_beg)
{
    struct Shrapnel *p_shrapnel;
    ushort shrap;

    for (shrap = shrapnel_beg; shrap != 0; shrap = p_shrapnel->child)
    {
        struct ShEnginePoint sp1, sp2, sp3;
        struct SpecialPoint *p_scrpoint;
        int cor_dx, cor_dy, cor_dz;
        int x_pcc, x_pcs, y_msc, y_mss, x_mmc, y_pps;
        int z_ps, z_ms;
        int scr_depth;

        p_shrapnel = &shrapnel[shrap];
        if ((p_shrapnel->type < 1) || (p_shrapnel->type > 3))
            continue;

        {
            int cos_yaw, cos_pitch, sin_yaw, sin_pitch, tmp;
            int sh_cc, sh_cs, sh_sc, sh_ss, sh_z;
            short shrap_yaw, shrap_pitch;

            shrap_yaw = 8 * p_shrapnel->yaw;
            shrap_pitch = 8 * p_shrapnel->pitch;
            cos_yaw = lbSinTable[shrap_yaw + LbFPMath_PI/2];
            cos_pitch = lbSinTable[shrap_pitch + LbFPMath_PI/2];
            sin_pitch = lbSinTable[shrap_pitch];
            sin_yaw = lbSinTable[shrap_yaw];

            tmp = (cos_pitch * cos_yaw) & 0xFFFF0000;
            tmp |= ((u64)(cos_pitch * (s64)cos_yaw) >> 32) & 0xFFFF;
            sh_cc = (int)bw_rotl32(tmp, 16) >> 10;

            tmp = (cos_pitch * sin_yaw) & 0xFFFF0000;
            tmp |= ((u64)(cos_pitch * (s64)sin_yaw) >> 32) & 0xFFFF;
            sh_cs = (int)bw_rotl32(tmp, 16) >> 10;

            tmp = (sin_pitch * cos_yaw) & 0xFFFF0000;
            tmp |= ((u64)(sin_pitch * (s64)cos_yaw) >> 32) & 0xFFFF;
            sh_sc = (int)bw_rotl32(tmp, 16) >> 10;

            tmp = (sin_pitch * sin_yaw) & 0xFFFF0000;
            tmp |= ((u64)(sin_pitch * (s64)sin_yaw) >> 32) & 0xFFFF;
            sh_ss = (int)bw_rotl32(tmp, 16) >> 10;

            sh_z = sin_yaw >> 10;

            cor_dx = (p_shrapnel->x >> 8) - engn_xc;
            cor_dy = (p_shrapnel->y >> 5) - engn_yc;
            cor_dz = (p_shrapnel->z >> 8) - engn_zc;

            x_pcc = cor_dx + sh_cc;
            y_msc = cor_dy - sh_sc;
            x_pcs = cor_dx + sh_cs;
            y_mss = cor_dy - sh_ss;
            y_pps = cor_dy + sh_sc + sh_ss;
            x_mmc = cor_dx - sh_cc - sh_cs;
            z_ps = cor_dz + sh_z;
            z_ms = cor_dz - sh_z;
        }

        transform_shpoint(&sp1, x_pcc, y_msc - 8 * engn_yc, z_ms);
        transform_shpoint(&sp2, x_pcs, y_mss - 8 * engn_yc, cor_dz);
        transform_shpoint(&sp3, x_mmc, y_pps - 8 * engn_yc, z_ps);

        if (((sp2.Flags & sp1.Flags & sp3.Flags) & 0xF) != 0)
            continue;

        scr_depth = sp2.Depth;
        if (scr_depth >= sp3.Depth)
            scr_depth = sp3.Depth;
        if (scr_depth > sp1.Depth)
            scr_depth = sp1.Depth;

        p_shrapnel->PointOffset = next_screen_point;
        p_scrpoint = draw_item_add_points(DrIT_SharpnlPoly, shrap, BUCKET_MID + scr_depth, 3);
        if (p_scrpoint == NULL) {
            break;
        }

        p_scrpoint->X = sp1.X;
        p_scrpoint->Y = sp1.Y;
        p_scrpoint->Z = sp1.Depth;
        p_scrpoint++;

        p_scrpoint->X = sp2.X;
        p_scrpoint->Y = sp2.Y;
        p_scrpoint->Z = sp2.Depth;
        p_scrpoint++;

        p_scrpoint->X = sp3.X;
        p_scrpoint->Y = sp3.Y;
        p_scrpoint->Z = sp3.Depth;
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

struct SingleObjectFace4 *build_polygon_slice(short x1, short y1,
  short x2, short y2, int w1, int w2, int col, int sort_key, ushort flag)
{
    struct SingleObjectFace4 *p_face4;
    struct SpecialPoint *p_specpt;
    int dx, dy;
    int norm_dx, norm_dy;
    int prop_dx1, prop_dy1;
    int prop_dx2, prop_dy2;
    int scal_dx1, scal_dy1;
    int scal_dx2, scal_dy2;
    int length;
    ushort pt;
    TbBool neg_x, neg_y;

    static short prevpt1_x = 0;
    static short prevpt1_y = 0;
    static short prevpt2_x = 0;
    static short prevpt2_y = 0;

    neg_y = 0;
    neg_x = 0;
    dx = x1 - x2;
    dy = y2 - y1;
    length = LbSqrL(16 * (dx * dx + dy * dy));
    if (length == 0)
        return NULL;
    norm_dy = (dy << 10) / length;
    norm_dx = (dx << 10) / length;
    if (norm_dy < 0) {
        norm_dy = -norm_dy;
        neg_y = 1;
    }
    if (norm_dx < 0) {
        norm_dx = -norm_dx;
        neg_x = 1;
    }

    prop_dy1 = (norm_dy * w1 + 128) >> 8;
    prop_dx1 = (norm_dx * w1 + 128) >> 8;
    prop_dy2 = (norm_dy * w2 + 128) >> 8;
    prop_dx2 = (norm_dx * w2 + 128) >> 8;
    scal_dy1 = (overall_scale * prop_dy1 + 128) >> 8;
    scal_dx1 = (overall_scale * prop_dx1 + 128) >> 8;
    scal_dy2 = (overall_scale * prop_dy2 + 128) >> 8;
    scal_dx2 = (overall_scale * prop_dx2 + 128) >> 8;
    if (neg_y) {
        scal_dy1 = -scal_dy1;
        scal_dy2 = -scal_dy2;
    }
    if (neg_x) {
        scal_dx1 = -scal_dx1;
        scal_dx2 = -scal_dx2;
    }

    p_face4 = draw_item_add_special_obj_face4(DrIT_SpObFace4, sort_key);
    if (p_face4 == NULL) {
        return NULL;
    }

    p_face4->ExCol = col;
    p_face4->Flags = 15;

    if (flag == 0) // First slice
    {
        prevpt1_x = x1 + scal_dy1;
        prevpt1_y = y1 + scal_dx1;
        prevpt2_x = x1 - scal_dy1;
        prevpt2_y = y1 - scal_dx1;
    }

    pt = p_face4->PointNo[0];
    p_specpt = &game_screen_point_pool[pt];
    p_specpt->X = prevpt1_x;
    p_specpt->Y = prevpt1_y;

    pt = p_face4->PointNo[1];
    p_specpt = &game_screen_point_pool[pt];
    p_specpt->X = prevpt2_x;
    p_specpt->Y = prevpt2_y;

    pt = p_face4->PointNo[2];
    p_specpt = &game_screen_point_pool[pt];
    p_specpt->X = x2 + scal_dy2;
    p_specpt->Y = y2 + scal_dx2;

    pt = p_face4->PointNo[3];
    p_specpt = &game_screen_point_pool[pt];
    p_specpt->X = x2 - scal_dy2;
    p_specpt->Y = y2 - scal_dx2;

    prevpt1_x = x2 + scal_dy2;
    prevpt1_y = y2 + scal_dx2;
    prevpt2_x = x2 - scal_dy2;
    prevpt2_y = y2 - scal_dx2;

    return p_face4;
}

void build_polygon_circle_2d(int x1, int y1, int r1, int r2,
  int flag, struct SingleFloorTexture *p_tex, ushort col,
  short bright1, short bright2, int depth_shift)
{
    int pt3, pt4;
    int scrad1;
    int cur_x, cur_y;
    short angle, dt_angle, angle_detail;

    scrad1 = (overall_scale * r1) >> 8;

    if ((x1 + scrad1 < 0) || (x1 - scrad1 > vec_window_width))
        return;
    if ((y1 + scrad1 < 0) || (y1 - scrad1 > vec_window_height))
        return;

    if (scrad1 > 150)
        angle_detail = 16;
    else if (scrad1 > 50)
        angle_detail = 32;
    else if (scrad1 > 10)
        angle_detail = 64;
    else if (scrad1 > 5)
        angle_detail = 128;
    else
        angle_detail = 256;

    cur_x = x1 + scrad1;
    cur_y = y1;

    pt3 = next_screen_point;
    pt4 = pt3 + 1;
    {
        struct SpecialPoint *p_specpt3;
        p_specpt3 = &game_screen_point_pool[pt3];
        p_specpt3->X = x1;
        p_specpt3->Y = y1;
    }

    dt_angle = 2 * angle_detail;
    angle = dt_angle;
    while (angle <= 2048)
    {
        struct SingleObjectFace4 *p_face4;
        struct SpecialPoint *p_specpt;
        int nxt_x, nxt_y;
        int sin_angl, half_angl, cos_angl;
        int hlf_y, hlf_x;

        half_angl = angle - angle_detail;
        cos_angl = lbSinTable[(half_angl & LbFPMath_AngleMask) + LbFPMath_PI/2];
        sin_angl = lbSinTable[(half_angl & LbFPMath_AngleMask)];
        hlf_x = x1 + ((scrad1 * cos_angl) >> 16);
        hlf_y = y1 + ((scrad1 * sin_angl) >> 16);

        cos_angl = lbSinTable[(angle & LbFPMath_AngleMask) + LbFPMath_PI/2];
        sin_angl = lbSinTable[angle & LbFPMath_AngleMask];
        nxt_x = x1 + ((scrad1 * cos_angl) >> 16);
        nxt_y = y1 + ((scrad1 * sin_angl) >> 16);

        if (pt4 + 3 > screen_points_limit) {
            break;
        }

        p_face4 = draw_item_add_special_obj_face4_no_pts(DrIT_SpObFace4, depth_shift);

        if (p_face4 == NULL) {
            break;
        }

        p_face4->Flags = 17;
        p_face4->PointNo[0] = pt4 + 2;
        p_face4->PointNo[1] = pt4 + 1;
        p_face4->PointNo[2] = pt3;
        p_face4->PointNo[3] = pt4 + 0;
        p_face4->Shade0 = bright1;
        p_face4->Shade1 = bright1;
        p_face4->Shade3 = bright1;
        p_face4->Shade2 = bright2;
        p_face4->GFlags = 0;
        p_face4->ExCol = col;

        p_specpt = &game_screen_point_pool[pt4 + 0];
        p_specpt->X = cur_x;
        p_specpt->Y = cur_y;

        p_specpt = &game_screen_point_pool[pt4 + 1];
        p_specpt->X = hlf_x;
        p_specpt->Y = hlf_y;

        p_specpt = &game_screen_point_pool[pt4 + 2];
        p_specpt->X = nxt_x;
        p_specpt->Y = nxt_y;

        pt4 += 3;
        cur_x = nxt_x;
        cur_y = nxt_y;
        angle += dt_angle;
    }
    next_screen_point = pt4;
}

void build_polygon_circle(int x1, int y1, int z1, int r1, int r2, int flag,
  struct SingleFloorTexture *p_tex, int col, int bright1, int bright2)
{
    int pp_X, pp_Y;
    int bckt;

    {
        struct EnginePoint ep;
        ep.X3d = x1 - engn_xc;
        ep.Z3d = z1 - engn_zc;
        ep.Y3d = 8 * y1 - (engn_yc >> 3);
        ep.Flags = 0;
        transform_point(&ep);

        pp_X = ep.pp.X;
        pp_Y = ep.pp.Y;
        bckt = BUCKET_MID + ep.Z3d - 16 * r1;
    }

    build_polygon_circle_2d(pp_X, pp_Y, r1, r2, flag,
      p_tex, col, bright1, bright2, bckt);
}

struct SingleObjectFace4 *build_glare(short x1, short y1, short z1, short r1)
{
    struct EnginePoint ep;
    struct SpecialPoint *p_scrpoint;
    struct SingleObjectFace4 *p_face4;
    struct SingleFloorTexture *p_sftex;
    int pp_X, pp_Y, pp_Z;
    int scaled_r;
    int bckt;
    uint sftex;
    ushort pt;

    ep.X3d = x1 - engn_xc;
    ep.Z3d = z1 - engn_zc;
    ep.Y3d = y1 - (engn_yc >> 3);
    ep.Flags = 0;
    transform_point(&ep);

    pp_X = ep.pp.X;
    pp_Z = ep.Z3d - 16 * r1;
    pp_Y = ep.pp.Y;

    bckt = BUCKET_MID + pp_Z;

    scaled_r = (r1 * overall_scale) >> 8;

    if ((ep.pp.X + scaled_r < 0) || (ep.pp.X - scaled_r > vec_window_width))
        return NULL;

    if ((ep.pp.Y + scaled_r < 0) || (ep.pp.Y - scaled_r > vec_window_height))
        return NULL;

    p_face4 = draw_item_add_special_obj_face4(DrIT_SpObFace4, bckt);
    if (p_face4 == NULL) {
        return NULL;
    }

    pt = p_face4->PointNo[0];
    p_scrpoint = &game_screen_point_pool[pt];
    p_scrpoint->X = pp_X - scaled_r;
    p_scrpoint->Y = pp_Y - scaled_r;

    pt = p_face4->PointNo[1];
    p_scrpoint = &game_screen_point_pool[pt];
    p_scrpoint->X = pp_X + scaled_r;
    p_scrpoint->Y = pp_Y - scaled_r;

    pt = p_face4->PointNo[3];
    p_scrpoint = &game_screen_point_pool[pt];
    p_scrpoint->X = pp_X + scaled_r;
    p_scrpoint->Y = pp_Y + scaled_r;

    pt = p_face4->PointNo[2];
    p_scrpoint = &game_screen_point_pool[pt];
    p_scrpoint->X = pp_X - scaled_r;
    p_scrpoint->Y = pp_Y + scaled_r;

    sftex = tnext_floor_texture;
    //TODO add floor texture limit check
    tnext_floor_texture += 1;
    p_sftex = &game_textures[sftex];
    p_sftex->TMapX1 = 96;
    p_sftex->TMapY1 = 96;
    p_sftex->TMapX2 = 127;
    p_sftex->TMapY2 = 96;
    p_sftex->TMapX4 = 127;
    p_sftex->TMapY4 = 127;
    p_sftex->TMapX3 = 96;
    p_sftex->TMapY3 = 127;
    p_sftex->Page = 4;

    p_face4->Texture = sftex;
    p_face4->Flags = 0x08 | 0x01;
    p_face4->GFlags = FGFlg_Unkn01;

    return p_face4;
}

void enlist_draw_wobble_line(int x1, int y1, int z1,
 int x2, int y2, int z2, int itime, ubyte slflags, TbBool is_player)
{
    int prc_x1, prc_y1, prc_z1;
    int prc_x2, prc_y2, prc_z2;
    int dt_x, dt_y, dt_z;
    int tot_dist;
    int num_steps, step;
    int prc_cur_x2, prc_cur_y2;

    prc_x1 = x1 << 7;
    prc_y1 = y1 << 7;
    prc_z1 = z1 << 7;
    prc_x2 = x2 << 7;
    prc_y2 = y2 << 7;
    prc_z2 = z2 << 7;

    tot_dist = LbSqrL((prc_y2 - prc_y1) * (prc_y2 - prc_y1)
      + (prc_x2 - prc_x1) * (prc_x2 - prc_x1));
    if (tot_dist <= 0) {
        return;
    }

    num_steps = (tot_dist / 10) >> 7;
    if (is_player)
        num_steps *= 2;
    if (num_steps < 1)
        num_steps = 1;

    dt_x = (prc_x2 - prc_x1) / num_steps;
    dt_y = (prc_y2 - prc_y1) / num_steps;
    dt_z = (prc_z2 - prc_z1) / num_steps;

    prc_cur_x2 = prc_x1;
    prc_cur_y2 = prc_y1;
    for (step = 1; step < num_steps + 1; step++)
    {
        struct SortLine *p_sline;
        int prc_cur_x1, prc_cur_y1;
        int shift;
        int bckt;

        prc_cur_x1 = prc_cur_x2;
        prc_cur_y1 = prc_cur_y2;
        prc_y1 += dt_y;
        prc_x1 += dt_x;
        prc_z1 += dt_z;

        if (!is_player)
        {
            shift = ((LbRandomPosShort() % 16) << 7) - 1024;
            prc_cur_x2 = prc_x1 + ((shift * overall_scale) >> 8);
            shift = ((LbRandomPosShort() % 16) << 7) - 1024;
            prc_cur_y2 = prc_y1 + ((shift * overall_scale) >> 8);
        }
        else if (step == 1)
        {
            shift = ((zig_zag[(render_anim_turn + x1) & 0x1F] & 7) << 7) - 512;
            prc_cur_x1 = prc_cur_x2 + ((overall_scale * shift) >> 8);
            shift = ((zig_zag[(render_anim_turn + y1) & 0x1F] & 7) << 7) - 512;
            prc_cur_y1 = prc_cur_y2 + ((shift * overall_scale) >> 8);
            shift = ((LbRandomPosShort() & 7) << 7) - 512;
            prc_cur_x2 = prc_x1 + ((shift * overall_scale) >> 8);
            shift = ((LbRandomPosShort() & 7) << 7) - 512;
            prc_cur_y2 = prc_y1 + ((shift * overall_scale) >> 8);
        }
        else if (step == num_steps)
        {
            shift = ((zig_zag[(render_anim_turn + x2) & 0x1F] & 7) << 7) - 512;
            prc_cur_x2 = prc_x1 + ((shift * overall_scale) >> 8);
            shift = ((zig_zag[(y2 + render_anim_turn) & 0x1F] & 7) << 7) - 512;
            prc_cur_y2 = prc_y1 + ((shift * overall_scale) >> 8);
        }

        if ((prc_x1 < 0) || (prc_x1 >> 7 >= lbDisplay.GraphicsScreenWidth)
          || (prc_y1 < 0) || (prc_y1 >> 7 >= lbDisplay.GraphicsScreenHeight))
            continue;

        bckt = BUCKET_MID + (prc_z1 >> 7);

        p_sline = draw_item_add_line(DrIT_Unkn11, bckt);
        if (p_sline == NULL) {
            break;
        }

        p_sline->X1 = prc_cur_x1 >> 7;
        p_sline->Y1 = prc_cur_y1 >> 7;
        p_sline->X2 = prc_cur_x2 >> 7;
        p_sline->Y2 = prc_cur_y2 >> 7;
        p_sline->Flags = slflags;

        if ((itime & 0xFF) < 100)
        {
            p_sline->Col = colour_lookup[ColLU_BLUE];
            p_sline->Shade = 32 + ((prc_cur_x1 + itime + step) & 0x1F);
        }
        else if ((itime & 0xFF) < 110)
        {
            p_sline->Col = colour_lookup[ColLU_WHITE];
            p_sline->Shade = 32;
            p_sline->Flags = 0;
        }
        else if ((itime & 0xFF) < 142)
        {
            p_sline->Col = colour_lookup[ColLU_BLUE];
            p_sline->Shade = 32 + (31 - (itime - 110));
            p_sline->Flags = 0;
        }
        else
        {
            p_sline->Col = colour_lookup[ColLU_BLUE];
            p_sline->Shade = 32 + ((prc_cur_x1 + itime + step) & 0x1F);
            p_sline->Flags = 0;
        }
    }
}

ushort shrapnel_get_child_type_not_3(struct Shrapnel *p_shraparnt)
{
    struct Shrapnel *p_shrapnel;
    ushort shrap;

    for (shrap = p_shraparnt->child; shrap != 0; shrap = p_shrapnel->child)
    {
        p_shrapnel = &shrapnel[shrap];

        if (p_shrapnel->type != 3)
            return shrap;
    }
    return 0;
}

void enlist_draw_bang_wobble_line(ushort shrapnel_beg)
{
    struct Shrapnel *p_shrapnel1;
    struct Shrapnel *p_shrapnel2;
    ushort shrap1, shrap2;
    ubyte slflags;
    TbBool is_player;

    is_player = 0;
    slflags = 0x01;

    shrap1 = shrapnel_beg;
    if (shrap1 == 0)
        return;
    p_shrapnel1 = &shrapnel[shrap1];

    shrap2 = shrapnel_get_child_type_not_3(p_shrapnel1);
    if (shrap2 == 0)
        return;
    p_shrapnel2 = &shrapnel[shrap2];

    {
        struct ShEnginePoint sp1, sp2;
        int cor_dx, cor_dy, cor_dz;

        cor_dx = (p_shrapnel1->x >> 8) - engn_xc;
        cor_dz = (p_shrapnel1->z >> 8) - engn_zc;
        cor_dy = (p_shrapnel1->y >> 5) - engn_yc;

        transform_shpoint(&sp1, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        cor_dx = (p_shrapnel2->x >> 8) - engn_xc;
        cor_dz = (p_shrapnel2->z >> 8) - engn_zc;
        cor_dy = (p_shrapnel2->y >> 5) - engn_yc;

        transform_shpoint(&sp2, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

        enlist_draw_wobble_line(sp1.X, sp1.Y, sp1.Depth,
          sp2.X, sp2.Y, sp2.Depth, 10, slflags, is_player);
    }
}

void enlist_draw_laser(int x1, int y1, int z1, int x2, int y2, int z2,
  int depth_shift, int itime, short ofs_x, short ofs_y, TbPixel colour)
{
    struct EnginePoint ep1, ep2;
    int scr_x, scr_y, scr_depth;
    int scr_dx, scr_dy, scr_ddepth;
    int thick_x, thick_y;
    int i, iter_count;
    ubyte flg;

    ep1.Flags = 0;
    ep1.X3d = x1 - engn_xc;
    ep1.Z3d = z1 - engn_zc;
    ep1.Y3d = 8 * y1 - (engn_yc >> 3);
    transform_point(&ep1);

    scr_x = ep1.pp.X << 8;
    scr_y = ep1.pp.Y << 8;
    scr_depth = ep1.Z3d << 8;

    scr_x += (overall_scale * ofs_x) >> 1;
    scr_y += (overall_scale * ofs_y) >> 1;

    ep2.Flags = 0;
    ep2.X3d = x2 - engn_xc;
    ep2.Z3d = z2 - engn_zc;
    ep2.Y3d = 8 * y2 - (engn_yc >> 3);
    transform_point(&ep2);

    if ((ep2.Flags & ep1.Flags & 0xF) != 0)
        return;

    scr_dy = (ep2.pp.Y << 8) - scr_y;
    scr_dx = (ep2.pp.X << 8) - scr_x;
    scr_ddepth = (ep2.Z3d << 8) - scr_depth;

    {
        int length, divdr;

        length = LbSqrL(scr_dy * scr_dy + scr_dx * scr_dx);
        if (length == 0)
            return;
        divdr = (length / 30) >> 8;
        if (divdr < 1)
            divdr = 1;
        scr_dx /= divdr;
        scr_dy /= divdr;
        scr_ddepth /= divdr;
        iter_count = divdr + 1;
    }

    switch (itime)
    {
    case 0:
        thick_x = (7 * scr_dx) >> 4;
        thick_y = (7 * scr_dy) >> 4;
        break;
    case 1:
        thick_x = (5 * scr_dx) >> 4;
        thick_y = (5 * scr_dy) >> 4;
        break;
    case 2:
        thick_x = (3 * scr_dx) >> 4;
        thick_y = (3 * scr_dy) >> 4;
        break;
    case 3:
        thick_x = (2 * scr_dx) >> 4;
        thick_y = (2 * scr_dy) >> 4;
        break;
    default:
        thick_x = 0;
        thick_y = 0;
        break;
    }

    flg = 0;
    for (i = 1; i < iter_count; i++)
    {
        if ((scr_x > 0) && (scr_x >> 8 < lbDisplay.GraphicsScreenWidth)
          && (scr_y > 0) && (scr_y >> 8 < lbDisplay.GraphicsScreenHeight))
        {
          struct SortLine *p_sline;
          int cor_x2, cor_y2;
          int cor_x1, cor_y1;
          int bckt;

          bckt = BUCKET_MID + (scr_depth >> 8) + depth_shift;

          cor_x1 = (scr_x + thick_x) >> 8;
          cor_y1 = (scr_y + thick_y) >> 8;
          cor_x2 = (scr_x + scr_dx - thick_x) >> 8;
          cor_y2 = (scr_y + scr_dy - thick_y) >> 8;

          p_sline = draw_item_add_line(DrIT_Unkn11, bckt);
          if (p_sline != NULL)
          {
              p_sline->X1 = cor_x1;
              p_sline->Y1 = cor_y1;
              p_sline->X2 = cor_x2;
              p_sline->Y2 = cor_y2;
              p_sline->Shade = 32;
              p_sline->Flags = 0;
              p_sline->Col = colour;
          }

          if (itime > 10) {
              build_polygon_slice(cor_x1, cor_y1, cor_x2, cor_y2,
                i * 6, (i + 1) * 6, colour, bckt, flg);
          } else if (itime > 4) {
              build_polygon_slice(cor_x1, cor_y1, cor_x2, cor_y2,
                i * (itime - 4), (i + 1) * (itime - 4), colour, bckt, flg);
          } else if (itime >= 0) {
              build_polygon_slice(cor_x1, cor_y1, cor_x2, cor_y2,
                2, 2, colour, bckt, flg);
          } else {
              build_polygon_slice(cor_x1, cor_y1, cor_x2, cor_y2,
                -itime, -itime, colour, bckt, flg);
          }
        }
        scr_x += scr_dx;
        scr_y += scr_dy;
        flg = 1;
        scr_depth += scr_ddepth;
    }
}

TbBool triangle_not_visible(struct SpecialPoint *p_specpt1,
  struct SpecialPoint *p_specpt2, struct SpecialPoint *p_specpt3)
{
    return ((p_specpt2->X - p_specpt1->X) * (p_specpt3->Y - p_specpt2->Y) -
            (p_specpt2->Y - p_specpt1->Y) * (p_specpt3->X - p_specpt2->X) <= 0);
}

int object_face_get_visible_max_depth(short pt1, short pt2, short pt3, short pt4, ushort gflags)
{
    struct SinglePoint *p_snpoint1;
    struct SpecialPoint *p_specpt1;
    struct SinglePoint *p_snpoint2;
    struct SpecialPoint *p_specpt2;
    struct SinglePoint *p_snpoint3;
    struct SpecialPoint *p_specpt3;
    struct SinglePoint *p_snpoint4;
    struct SpecialPoint *p_specpt4;
    int depth_max;
    ushort flags_all, flags_any;

    p_snpoint1 = &game_object_points[pt1];
    p_specpt1 = &game_screen_point_pool[p_snpoint1->PointOffset];

    p_snpoint2 = &game_object_points[pt2];
    p_specpt2 = &game_screen_point_pool[p_snpoint2->PointOffset];

    if (pt3 >= 0) {
        p_snpoint3 = &game_object_points[pt3];
        p_specpt3 = &game_screen_point_pool[p_snpoint3->PointOffset];
    } else {
        p_snpoint3 = NULL;
        p_specpt3 = NULL;
    }

    if (pt4 >= 0) {
        p_snpoint4 = &game_object_points[pt4];
        p_specpt4 = &game_screen_point_pool[p_snpoint4->PointOffset];
    } else {
        p_snpoint4 = NULL;
        p_specpt4 = NULL;
    }

    depth_max = SHRT_MIN;
    if (depth_max < p_specpt1->Z)
        depth_max = p_specpt1->Z;
    if (depth_max < p_specpt2->Z)
        depth_max = p_specpt2->Z;
    if ((p_specpt3 != NULL) && (depth_max < p_specpt3->Z))
        depth_max = p_specpt3->Z;
    if ((p_specpt4 != NULL) && (depth_max < p_specpt4->Z))
        depth_max = p_specpt4->Z;

    flags_all = p_snpoint2->Flags & p_snpoint1->Flags;
    if (p_snpoint3 != NULL)
        flags_all &= p_snpoint3->Flags;
    if (p_snpoint4 != NULL)
        flags_all &= p_snpoint4->Flags;
    flags_any = p_snpoint2->Flags | p_snpoint1->Flags;
    if (p_snpoint3 != NULL)
        flags_any |= p_snpoint3->Flags;
    if (p_snpoint4 != NULL)
        flags_any |= p_snpoint4->Flags;

    if (((gflags & VisMDF_SkipFlg20) == 0) && ((flags_any & 0x20) != 0))
        return SHRT_MIN - 1;

    if ((flags_all & 0xF) != 0)
        return SHRT_MIN - 1;

    if (((gflags & FGFlg_Unkn01) == 0) && (p_specpt3 != NULL)) {
        if (triangle_not_visible(p_specpt1, p_specpt2, p_specpt3))
            return SHRT_MIN - 1;
    }
    return depth_max;
}

TbBool enlist_draw_face3_prealloc(int face, short depth_shift,
  ushort vmdflags, ubyte ditype, int *bckt_max)
{
    struct SingleObjectFace3 *p_face;
    int depth_max, bckt;

    p_face = &game_object_faces3[face];

    depth_max = object_face_get_visible_max_depth(p_face->PointNo[0],
      p_face->PointNo[2], p_face->PointNo[1], -1,
      p_face->GFlags | vmdflags);
    if (depth_max < SHRT_MIN)
        return true;

    bckt = BUCKET_MID + depth_shift + depth_max;
    if (*bckt_max < bckt)
        *bckt_max = bckt;
    stat_drawlist_faces++;
    return draw_item_add(ditype, face, bckt);
}

TbBool enlist_draw_face4_prealloc(int face, short depth_shift,
  ushort vmdflags, ubyte ditype, int *bckt_max)
{
    struct SingleObjectFace4 *p_face4;
    int depth_max, bckt;

    p_face4 = &game_object_faces4[face];

    depth_max = object_face_get_visible_max_depth(p_face4->PointNo[0],
      p_face4->PointNo[2], p_face4->PointNo[1], p_face4->PointNo[3],
      p_face4->GFlags | vmdflags);
    if (depth_max < SHRT_MIN)
        return true;

    bckt = BUCKET_MID + depth_shift + depth_max;
    if (*bckt_max < bckt)
        *bckt_max = bckt;
    stat_drawlist_faces++;
    return draw_item_add(ditype, face, bckt);
}

TbBool enlist_draw_face2_2pt_prealloc(int face, short depth_shift,
  ushort vmdflags, ubyte ditype, int *bckt_max)
{
    struct SingleObjectFace4 *p_face4;
    int depth_max, bckt;

    p_face4 = &game_object_faces4[face];

    depth_max = object_face_get_visible_max_depth(p_face4->PointNo[0],
      p_face4->PointNo[1], -1, -1,
      p_face4->GFlags | vmdflags);
    if (depth_max < SHRT_MIN)
        return true;

    bckt = BUCKET_MID + depth_shift + depth_max;
    if (*bckt_max < bckt)
        *bckt_max = bckt;
    stat_drawlist_faces++;
    return draw_item_add(ditype, face, bckt);
}

TbBool enlist_draw_face4_pole(int cor_dx, int cor_dy, int cor_dz,
  int face, short depth_shift, int *bckt_max)
{
    struct SingleObjectFace4 *p_face4;
    int specpt;

    specpt = next_screen_point;
    if (specpt + 2 > screen_points_limit) {
        return false;
    }
    next_screen_point += 2;

    p_face4 = &game_object_faces4[face];

    {
        struct ShEnginePoint sp;
        struct SinglePoint *p_snpoint;
        struct SpecialPoint *p_specpt;
        int dxc, dyc, dzc;

        p_snpoint = &game_object_points[p_face4->PointNo[0]];
        dxc = p_snpoint->X + cor_dx;
        dzc = p_snpoint->Z + cor_dz;
        dyc = p_snpoint->Y + cor_dy;
        transform_shpoint(&sp, dxc, dyc - 8 * engn_yc, dzc);

        p_snpoint->PointOffset = specpt + 0;
        p_snpoint->Flags = sp.Flags;

        p_specpt = &game_screen_point_pool[p_snpoint->PointOffset];
        p_specpt->X = sp.X;
        p_specpt->Y = sp.Y;
        p_specpt->Z = sp.Depth;
    }

    {
        struct ShEnginePoint sp;
        struct SinglePoint *p_snpoint;
        struct SpecialPoint *p_specpt;
        int dxc, dyc, dzc;

        p_snpoint = &game_object_points[p_face4->PointNo[1]];
        dxc = p_snpoint->X + cor_dx;
        dzc = p_snpoint->Z + cor_dz;
        dyc = p_snpoint->Y + cor_dy;
        transform_shpoint(&sp, dxc, dyc - 8 * engn_yc, dzc);

        p_snpoint->PointOffset = specpt + 1;
        p_snpoint->Flags = sp.Flags;

        p_specpt = &game_screen_point_pool[p_snpoint->PointOffset];
        p_specpt->X = sp.X;
        p_specpt->Y = sp.Y;
        p_specpt->Z = sp.Depth;
    }

    ubyte ditype;

    ditype = DrIT_ObFacePole;

    return enlist_draw_face2_2pt_prealloc(face, depth_shift,
      0, ditype, bckt_max);
}

void enlist_draw_plasma_sparks_on_object(struct SingleObject *point_object)
{
    int points_num, rnd_range;
    int i;
    ubyte slflags;
    TbBool is_player;

    is_player = 0;
    slflags = 0x01;

    points_num = point_object->EndPoint - point_object->StartPoint;
    rnd_range = points_num - 4;
    assert(rnd_range < next_screen_point);

    for (i = 0; i < 10; i++)
    {
        struct SpecialPoint *p_specpt2;
        struct SpecialPoint *p_specpt1;
        int pt1, pt2;

        pt1 = next_screen_point - (((ushort)LbRandomPosShort() % rnd_range) + 1);
        pt2 = next_screen_point - (((ushort)LbRandomPosShort() % rnd_range) + 1);
        p_specpt2 = &game_screen_point_pool[pt2];
        p_specpt1 = &game_screen_point_pool[pt1];

        enlist_draw_wobble_line(p_specpt1->X, p_specpt1->Y, p_specpt1->Z - 1024,
          p_specpt2->X, p_specpt2->Y, p_specpt2->Z - 1024, 10, slflags, is_player);
    }
}

void enlist_draw_long_health_bar_2d(int scr_x, int scr_y,
  int scr_depth, int bckt, int val, int val_max,
  intptr_t p_sitm, TbPixel lvl_col, TbPixel bar_col)
{
    struct SortSprite *p_sspr;

    if (bckt > BUCKET_MID + scr_depth)
        bckt = BUCKET_MID + scr_depth;
    p_sspr = draw_item_add_sprite(DrIT_LongPropBar, bckt);
    if (p_sspr == NULL) {
        return;
    }

    p_sspr->X = scr_x;
    p_sspr->Y = scr_y;
    p_sspr->Z = scr_depth;
    p_sspr->SrcItem = p_sitm;
    p_sspr->Frame = val_max;
    p_sspr->Scale = val;
    p_sspr->Brightness = lvl_col;
    p_sspr->Angle = bar_col;
}

void enlist_draw_long_health_bar(int cor_x, int cor_y, int cor_z,
  int depth_shift, int bckt, int val, int val_max,
  intptr_t p_sitm, TbPixel lvl_col, TbPixel bar_col)
{
    struct ShEnginePoint sp;
    int cor_dx, cor_dy, cor_dz;
    int scr_depth;

    cor_dx = cor_x - engn_xc;
    cor_dy = cor_y - engn_yc;
    cor_dz = cor_z - engn_zc;
    transform_shpoint(&sp, cor_dx, cor_dy - 8 * engn_yc, cor_dz);

    scr_depth = sp.Depth + depth_shift;

    enlist_draw_long_health_bar_2d(sp.X, sp.Y + 20, scr_depth, bckt,
      val, val_max, p_sitm, lvl_col, bar_col);
}

/******************************************************************************/
