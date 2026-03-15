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
#include <string.h>

#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "engindrwlstx.h"
#include "enginpeff.h"
#include "enginprops.h"
#include "enginshrapn.h"
#include "enginsngobjs.h"
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
        int x, y, z;
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

            x = (p_shrapnel->x >> 8) - engn_xc;
            y = (p_shrapnel->y >> 5) - engn_yc;
            z = (p_shrapnel->z >> 8) - engn_zc;

            x_pcc = x + sh_cc;
            y_msc = y - sh_sc;
            x_pcs = x + sh_cs;
            y_mss = y - sh_ss;
            y_pps = y + sh_sc + sh_ss;
            x_mmc = x - sh_cc - sh_cs;
            z_ps = z + sh_z;
            z_ms = z - sh_z;
        }

        transform_shpoint(&sp1, x_pcc, y_msc - 8 * engn_yc, z_ms);
        transform_shpoint(&sp2, x_pcs, y_mss - 8 * engn_yc, z);
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
        int x, y, z;

        x = (p_shrapnel1->x >> 8) - engn_xc;
        z = (p_shrapnel1->z >> 8) - engn_zc;
        y = (p_shrapnel1->y >> 5) - engn_yc;

        transform_shpoint(&sp1, x, y - 8 * engn_yc, z);

        x = (p_shrapnel2->x >> 8) - engn_xc;
        z = (p_shrapnel2->z >> 8) - engn_zc;
        y = (p_shrapnel2->y >> 5) - engn_yc;

        transform_shpoint(&sp2, x, y - 8 * engn_yc, z);

        enlist_draw_wobble_line(sp1.X, sp1.Y, sp1.Depth,
          sp2.X, sp2.Y, sp2.Depth, 10, slflags, is_player);
    }
}

/******************************************************************************/
