/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstm.c
 *     Making drawlists for the 3D engine.
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

#include <assert.h>
#include "bfendian.h"
#include "bfmath.h"
#include "bfutility.h"

#include "bigmap.h"
#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "engindrwlstx.h"
#include "enginfloor.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginshrapn.h"
#include "enginprops.h"
#include "engintrns.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "game_speed.h"
#include "matrix.h"
#include "swlog.h"
#include "thing.h"
#include "vehicle.h"
/******************************************************************************/
#pragma pack(1)

struct BulStart {
    sbyte OffsetX;
    sbyte OffsetY;
};

#pragma pack()
/******************************************************************************/
extern short word_1552F8;

extern ubyte byte_176D49;

extern long dword_176CAC;
extern long dword_176CB0;
extern long dword_152E4C;

extern struct BulStart bul_starts[4000];

extern short word_1AA5F4;
extern short word_1AA5F6;
extern short word_1AA5F8;
extern short word_1AA5FA;

extern ushort zig_zag[55];

ubyte byte_152EF0[] = {
   0, 10,  5, 10,  7,  7,  8, 10,
  10, 10,  5,  7,  5,  7,  7,  0,
};

ushort overall_scale = 256;

/** Add a new draw item and return linked SortLine instance.
 *
 * @param ditype Draw item type, should be one of SortLine related types.
 * @param bckt Destination bucket for this draw item.
 * @return SortLine instance to fill, or NULL if arrays exceeded.
 */
struct SortLine *draw_item_add_line(ubyte ditype, int bckt)
{
    struct SortLine *p_sline;

    if (next_sort_line + 1 > mem_game[33].N)
        return NULL;

    p_sline = p_current_sort_line;
    if (!draw_item_add(ditype, next_sort_line, bckt))
        return NULL;

    p_current_sort_line++;
    next_sort_line++;

    return p_sline;
}

/** Add a new draw item and return linked SortSprite instance.
 *
 * @param ditype Draw item type, should be one of SortSprite related types.
 * @param bckt Destination bucket for this draw item.
 * @return SortSprite instance to fill, or NULL if arrays exceeded.
 */
struct SortSprite *draw_item_add_sprite(ubyte ditype, int bckt)
{
    struct SortSprite *p_sspr;

    if (next_sort_sprite + 1 > mem_game[32].N)
        return NULL;

    p_sspr = p_current_sort_sprite;
    if (!draw_item_add(ditype, next_sort_sprite, bckt))
        return NULL;

    p_current_sort_sprite++;
    next_sort_sprite++;

    return p_sspr;
}

/** Add a new draw item and return first of linked SpecialPoint instances.
 *
 * @param ditype Draw item type, should be one of SpecialPoint related types.
 * @param bckt Destination bucket for this draw item.
 * @param npoints Amount of consecutive points to reserve.
 * @return SpecialPoint instance to fill, or NULL if arrays exceeded.
 */
struct SpecialPoint *draw_item_add_points(ubyte ditype, ushort offset, int bckt, ushort npoints)
{
    struct SpecialPoint *p_scrpoint;

    if (next_screen_point + npoints > screen_points_limit)
        return NULL;

    p_scrpoint = &game_screen_point_pool[next_screen_point];
    if (!draw_item_add(ditype, offset, bckt))
        return NULL;

    next_screen_point += npoints;

    return p_scrpoint;
}

/** Add a new draw item and return linked FloorTile instance.
 *
 * @param ditype Draw item type, should be one of FloorTile related types.
 * @param bckt Destination bucket for this draw item.
 * @return FloorTile instance to fill, or NULL if arrays exceeded.
 */
struct FloorTile *draw_item_add_floor_tile(ubyte ditype, int bckt)
{
    struct FloorTile *p_floortl;

    if (next_floor_tile + 1 > mem_game[26].N)
        return NULL;

    p_floortl = &game_floor_tiles[next_floor_tile];
    if (!draw_item_add(ditype, next_floor_tile, bckt))
        return NULL;

    next_floor_tile++;

    return p_floortl;
}

ushort draw_mapwho_vect(int x1, int y1, int z1, int x2, int y2, int z2, int col)
{
    struct ShEnginePoint sp1, sp2;
    struct SortLine *p_sline;
    int bckt;
    ushort sline;

    transform_shpoint(&sp1, x1, 8 * y1 - 8 * engn_yc, z1);

    transform_shpoint(&sp2, x2, 8 * y2 - 8 * engn_yc, z2);

    if ((sp2.Flags & sp1.Flags & 0xF) != 0)
        return 0;

    bckt = BUCKET_MID + sp1.Depth;

    sline = next_sort_line;
    p_sline = draw_item_add_line(DrIT_Unkn11, bckt);
    if (p_sline == NULL)
        return 0;

    p_sline->Shade = 32;
    p_sline->Flags = 0;
    p_sline->X1 = sp1.X;
    p_sline->Y1 = sp1.Y;
    p_sline->X2 = sp2.X;
    p_sline->Y2 = sp2.Y;
    p_sline->Col = col;

    return sline;
}

void draw_mapwho_vect_len(int x1, int y1, int z1, int x2, int y2, int z2, int len, int col)
{
    struct ShEnginePoint sp1, sp3;
    struct SortLine *p_sline;
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

    transform_shpoint(&sp1, x1, 8 * y1 - 8 * engn_yc, z1);
    transform_shpoint(&sp3, x3, 8 * y3 - 8 * engn_yc, z3);

    if ((sp3.Flags & sp1.Flags & 0xF) != 0)
        return;

    p_sline = draw_item_add_line(DrIT_Unkn11, BUCKET_MID + sp1.Depth);
    if (p_sline == NULL)
        return;

    p_sline->Shade = 32;
    p_sline->Flags = 0;
    p_sline->X1 = sp1.X;
    p_sline->Y1 = sp1.Y;
    p_sline->X2 = sp3.X;
    p_sline->Y2 = sp3.Y;
    p_sline->Col = col;
}

void draw_e_graphic(int x, int y, int z, ushort frame, int radius, int intensity, struct Thing *p_thing)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[gameturn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - radius;
    if ((ingame.DisplayMode != 50) && ((p_thing->Flag2 & TgF2_InsideBuilding) != 0))
        scr_depth += BUCKETS_COUNT;

    p_sspr = draw_item_add_sprite(DrIT_Unkn3, BUCKET_MID + scr_depth);
    if (p_sspr == NULL)
        return;

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y;
    p_sspr->Z = scr_depth;
    p_sspr->Frame = frame;
    p_sspr->Brightness = intensity;
    p_sspr->Scale = 256;
    p_sspr->PThing = p_thing;
}

void draw_e_graphic_scale(int x, int y, int z, ushort frame, int radius, int intensity, int scale)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[gameturn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - radius - 100;

    p_sspr = draw_item_add_sprite(DrIT_Unkn15, BUCKET_MID + scr_depth);
    if (p_sspr == NULL)
        return;

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y;
    p_sspr->Z = scr_depth;
    p_sspr->Frame = frame;
    p_sspr->Brightness = intensity;
    p_sspr->Scale = scale;
    p_sspr->PThing = NULL;
}

void draw_pers_e_graphic(struct Thing *p_thing, int x, int y, int z, int frame, int radius, int intensity)
{
    struct ShEnginePoint sp;
    struct SortSprite *p_sspr;
    int scr_depth;
    ubyte bri;

    bri = byte_152EF0[p_thing->SubType] + intensity;

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[gameturn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - radius;
    if (ingame.DisplayMode == 50)
    {
        if ((p_thing->Flag2 & TgF2_InsideBuilding) != 0) {
            if ((p_thing->Flag & TngF_Destroyed) != 0)
                return;
            scr_depth -= 10 * BUCKETS_COUNT;
        }
    }
    else
    {
        if ((p_thing->Flag2 & TgF2_InsideBuilding) != 0)
            scr_depth = -BUCKETS_COUNT;
    }

    p_sspr = draw_item_add_sprite(DrIT_Unkn13, BUCKET_MID + scr_depth);
    if (p_sspr == NULL)
        return;

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y;
    p_sspr->Z = 0;
    p_sspr->Frame = frame;
    p_sspr->PThing = p_thing;
    p_sspr->Brightness = bri;
    p_sspr->Angle = (p_thing->U.UObject.Angle + 8 - byte_176D49) & 7;

    if (((p_thing->Flag2 & TgF2_InsideBuilding) != 0) || p_thing->U.UPerson.OnFace || (p_thing->SubType == SubTT_PERS_MECH_SPIDER))
        return;

    p_sspr = draw_item_add_sprite(DrIT_Unkn19, BUCKET_MID + scr_depth - 200);
    if (p_sspr == NULL)
        return;

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y;
    p_sspr->PThing = p_thing;
}

void FIRE_draw_fire(struct SimpleThing *p_sthing)
{
    struct FireFlame *p_flame;
    ushort flm;

    for (flm = p_sthing->U.UFire.flame; flm; flm = p_flame->next)
    {
        struct ShEnginePoint sp;
        struct SpecialPoint *p_scrpoint;
        int x, y, z;

        p_flame = &FIRE_flame[flm];
        x = p_flame->x - engn_xc;
        y = p_flame->y - engn_yc;
        z = p_flame->z - engn_zc;

        if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
            y += waft_table[gameturn & 0x1F];

        transform_shpoint(&sp, x, y - 8 * engn_yc, z);

        p_flame->PointOffset = next_screen_point;
        p_scrpoint = draw_item_add_points(DrIT_Unkn25, flm, BUCKET_MID + sp.Depth - 50, 1);
        if (p_scrpoint == NULL)
            break;

        p_scrpoint->X = sp.X;
        p_scrpoint->Y = sp.Y;
        p_scrpoint->Z = sp.Depth;
    }
}

void draw_bang_phwoar(struct SimpleThing *p_pow)
{
    struct Phwoar *p_phwoar;
    ushort phw;

    for (phw = p_pow->U.UBang.phwoar; phw != 0; phw = p_phwoar->child)
    {
        struct ShEnginePoint sp;
        struct SpecialPoint *p_scrpoint;
        int x, y, z;

        p_phwoar = &phwoar[phw];
        x = (p_phwoar->x >> 8) - engn_xc;
        z = (p_phwoar->z >> 8) - engn_zc;
        y = PRCCOORD_TO_YCOORD(p_phwoar->y) - engn_yc;

        transform_shpoint(&sp, x, y - 8 * engn_yc, z);

        p_phwoar->PointOffset = next_screen_point;
        p_scrpoint = draw_item_add_points(DrIT_Unkn21, phw, BUCKET_MID + sp.Depth - 100, 1);
        if (p_scrpoint == NULL)
            break;

        p_scrpoint->X = sp.X;
        p_scrpoint->Y = sp.Y;
        p_scrpoint->Z = sp.Depth;
    }
}

void draw_bang_shrapnel(struct SimpleThing *p_pow)
{
    struct Shrapnel *p_shrapnel;
    ushort shrap;

    for (shrap = p_pow->U.UBang.shrapnel; shrap != 0; shrap = p_shrapnel->child)
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
            y = PRCCOORD_TO_YCOORD(p_shrapnel->y) - engn_yc;
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
        p_scrpoint = draw_item_add_points(DrIT_Unkn20, shrap, BUCKET_MID + scr_depth, 3);
        if (p_scrpoint == NULL)
            break;

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

struct SingleObjectFace4 *build_polygon_slice(short x1, short y1, short x2, short y2,
  int w1, int w2, int col, int sort_key, ushort flag)
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
    ushort face;
    ushort pt;
    TbBool neg_x, neg_y;

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

    pt = next_screen_point;
    if (pt + 4 > screen_points_limit)
        return NULL;

    face = next_special_face4;
    if (face + 1 > mem_game[25].N)
        return NULL;

    next_screen_point += 4;
    next_special_face4++;

    p_face4 = &game_special_object_faces4[face];
    p_face4->PointNo[1] = pt + 1;
    p_face4->PointNo[2] = pt + 2;
    p_face4->PointNo[3] = pt + 3;
    p_face4->PointNo[0] = pt + 0;
    p_face4->ExCol = col;
    p_face4->Flags = 15;

    if (flag == 0)
    {
        word_1AA5F4 = x1 + scal_dy1;
        word_1AA5F6 = y1 + scal_dx1;
        word_1AA5F8 = x1 - scal_dy1;
        word_1AA5FA = y1 - scal_dx1;
    }

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = word_1AA5F4;
    p_specpt->Y = word_1AA5F6;

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = word_1AA5F8;
    p_specpt->Y = word_1AA5FA;

    p_specpt = &game_screen_point_pool[pt + 2];
    p_specpt->X = x2 + scal_dy2;
    p_specpt->Y = y2 + scal_dx2;

    p_specpt = &game_screen_point_pool[pt + 3];
    p_specpt->X = x2 - scal_dy2;
    p_specpt->Y = y2 - scal_dx2;

    word_1AA5F6 = y2 + scal_dx2;
    word_1AA5F8 = x2 - scal_dy2;
    word_1AA5FA = y2 - scal_dx2;
    word_1AA5F4 = x2 + scal_dy2;

    draw_item_add(DrIT_Unkn12, face, sort_key);
    return p_face4;
}

void build_wobble_line(int x1, int y1, int z1,
 int x2, int y2, int z2, struct SimpleThing *p_sthing, int itime)
{
#if 0
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_build_wobble_line\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2), "g" (p_sthing), "g" (itime));
    return;
#endif
    int prc_x1, prc_y1, prc_z1;
    int prc_x2, prc_y2, prc_z2;
    int dt_x, dt_y, dt_z;
    int tot_dist;
    int num_steps, step;
    int prc_cur_x2, prc_cur_y2;
    TbBool is_player;

    prc_x1 = x1 << 7;
    prc_y1 = y1 << 7;
    prc_z1 = z1 << 7;
    prc_x2 = x2 << 7;
    prc_y2 = y2 << 7;
    prc_z2 = z2 << 7;

    tot_dist = LbSqrL((prc_y2 - prc_y1) * (prc_y2 - prc_y1) + (prc_x2 - prc_x1) * (prc_x2 - prc_x1));
    if (tot_dist <= 0)
        return;

    is_player = 0;
    num_steps = (tot_dist / 10) >> 7;
    if ((p_sthing != NULL) && (p_sthing->Flag & TngF_PlayerAgent) != 0)
    {
        is_player = 1;
        num_steps *= 2;
    }
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
            shift = ((zig_zag[(gameturn + x1) & 0x1F] & 7) << 7) - 512;
            prc_cur_x1 = prc_cur_x2 + ((overall_scale * shift) >> 8);
            shift = ((zig_zag[(gameturn + y1) & 0x1F] & 7) << 7) - 512;
            prc_cur_y1 = prc_cur_y2 + ((shift * overall_scale) >> 8);
            shift = ((LbRandomPosShort() & 7) << 7) - 512;
            prc_cur_x2 = prc_x1 + ((shift * overall_scale) >> 8);
            shift = ((LbRandomPosShort() & 7) << 7) - 512;
            prc_cur_y2 = prc_y1 + ((shift * overall_scale) >> 8);
        }
        else if (step == num_steps)
        {
            shift = ((zig_zag[(gameturn + x2) & 0x1F] & 7) << 7) - 512;
            prc_cur_x2 = prc_x1 + ((shift * overall_scale) >> 8);
            shift = ((zig_zag[(y2 + gameturn) & 0x1F] & 7) << 7) - 512;
            prc_cur_y2 = prc_y1 + ((shift * overall_scale) >> 8);
        }

        if ((prc_x1 < 0) || (prc_x1 >> 7 >= lbDisplay.GraphicsScreenWidth) || (prc_y1 < 0) || (prc_y1 >> 7 >= lbDisplay.GraphicsScreenHeight))
            continue;

        bckt = BUCKET_MID + (prc_z1 >> 7);

        p_sline = draw_item_add_line(DrIT_Unkn11, bckt);
        if (p_sline == NULL)
            break;

        p_sline->X1 = prc_cur_x1 >> 7;
        p_sline->Y1 = prc_cur_y1 >> 7;
        p_sline->X2 = prc_cur_x2 >> 7;
        p_sline->Y2 = prc_cur_y2 >> 7;
        if (is_player && (p_sthing->Timer1 < 1))
            p_sline->Flags = 0x02;
        else
            p_sline->Flags = 0x01;

        if ((itime & 0xFF) < 100)
        {
            p_sline->Col = colour_lookup[4];
            p_sline->Shade = 32 + ((prc_cur_x1 + itime + step) & 0x1F);
        }
        else if ((itime & 0xFF) < 110)
        {
            p_sline->Col = colour_lookup[1];
            p_sline->Shade = 32;
            p_sline->Flags = 0;
        }
        else if ((itime & 0xFF) < 142)
        {
            p_sline->Col = colour_lookup[4];
            p_sline->Shade = 32 + (31 - (itime - 110));
            p_sline->Flags = 0;
        }
        else
        {
            p_sline->Col = colour_lookup[4];
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

void draw_bang_wobble_line(struct SimpleThing *p_pow)
{
    struct Shrapnel *p_shrapnel1;
    struct Shrapnel *p_shrapnel2;
    ushort shrap1, shrap2;

    if (dword_176CAC == 0)
        return;

    if ((dword_152E4C & 0xFF) <= 208)
        return;

    dword_176CAC--;

    shrap1 = p_pow->U.UBang.shrapnel;
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
        y = PRCCOORD_TO_YCOORD(p_shrapnel1->y) - engn_yc;

        transform_shpoint(&sp1, x, y - 8 * engn_yc, z);

        x = (p_shrapnel2->x >> 8) - engn_xc;
        z = (p_shrapnel2->z >> 8) - engn_zc;
        y = PRCCOORD_TO_YCOORD(p_shrapnel2->y) - engn_yc;

        transform_shpoint(&sp2, x, y - 8 * engn_yc, z);

        build_wobble_line(sp1.X, sp1.Y, sp1.Depth, sp2.X, sp2.Y, sp2.Depth, 0, 10);
    }
}

struct SingleObjectFace4 *build_glare(short x1, short y1, short z1, short r1)
{
#if 0
    struct SingleObjectFace4 *ret;
    asm volatile (
      "call ASM_build_glare\n"
        : "=r" (ret) : "a" (x1), "d" (y1), "b" (z1), "c" (r1));
    return ret;
#endif
    struct EnginePoint ep;
    struct SpecialPoint *p_scrpoint;
    struct SingleObjectFace4 *p_face4;
    struct SingleFloorTexture *p_sftex;
    int pp_X, pp_Y, pp_Z;
    int scaled_r;
    int bckt;
    uint sftex;
    ushort face;
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

    pt = next_screen_point;
    if (pt + 4 > screen_points_limit)
        return NULL;
    next_screen_point += 4;

    p_scrpoint = &game_screen_point_pool[pt + 0];
    p_scrpoint->X = pp_X - scaled_r;
    p_scrpoint->Y = pp_Y - scaled_r;
    p_scrpoint = &game_screen_point_pool[pt + 1];
    p_scrpoint->X = pp_X + scaled_r;
    p_scrpoint->Y = pp_Y - scaled_r;
    p_scrpoint = &game_screen_point_pool[pt + 2];
    p_scrpoint->X = pp_X + scaled_r;
    p_scrpoint->Y = pp_Y + scaled_r;
    p_scrpoint = &game_screen_point_pool[pt + 3];
    p_scrpoint->X = pp_X - scaled_r;
    p_scrpoint->Y = pp_Y + scaled_r;

    sftex = tnext_floor_texture;
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

    face = next_special_face4;
    if (face + 1 > mem_game[25].N)
        return NULL;
    next_special_face4++;

    p_face4 = &game_special_object_faces4[face];
    p_face4->Texture = sftex;
    p_face4->PointNo[0] = pt + 0;
    p_face4->PointNo[1] = pt + 1;
    p_face4->PointNo[3] = pt + 2;
    p_face4->PointNo[2] = pt + 3;
    p_face4->Flags = 0x08 | 0x01;
    p_face4->GFlags = 1;

    draw_item_add(DrIT_Unkn12, face, bckt);

    return p_face4;
}

void build_laser(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner, int colour)
{
    struct EnginePoint ep1, ep2;
    ubyte flg;
    int scr_x, scr_y, scr_depth;
    int scr_dx, scr_dy, scr_ddepth;
    int thick_x, thick_y;
    int i, iter_count;

    flg = 0;
    if ((p_owner != NULL) && (p_owner->Type == TT_BUILDING))
    {
        short angle;

        if (p_owner->U.UObject.Angle != 0)
            angle = p_owner->U.UObject.ZZ_unused_but_pads_to_long_ObjectNo + 48;
        else
            angle = p_owner->U.UObject.ZZ_unused_but_pads_to_long_ObjectNo - 48;
        angle = ((angle + 0x0800) & 0x0700) | (angle & 0xFF);
        x1 = (3 * lbSinTable[angle] / 2 + p_owner->X) >> 8;
        y1 = p_owner->Y >> 8;
        z1 = (p_owner->Z - 3 * lbSinTable[angle + LbFPMath_PI/2] / 2) >> 8;
    }
    ep1.Flags = 0;
    ep1.X3d = x1 - engn_xc;
    ep1.Z3d = z1 - engn_zc;
    ep1.Y3d = 8 * y1 - (engn_yc >> 3);
    transform_point(&ep1);

    scr_x = ep1.pp.X << 8;
    scr_y = ep1.pp.Y << 8;
    scr_depth = ep1.Z3d << 8;

    if ((p_owner != NULL) && (p_owner->Type != TT_BUILDING))
    {
        short ofs_x, ofs_y;
        ushort mangle, frame;
        mangle = (p_owner->U.UPerson.Angle + 8 - byte_176D49) & 7;
        frame = p_owner->StartFrame + 1 + mangle;
        ofs_x = bul_starts[frame].OffsetX;
        ofs_y = bul_starts[frame].OffsetY;
        scr_x += (overall_scale * ofs_x) >> 1;
        scr_y += (overall_scale * ofs_y) >> 1;
    }
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

    for (i = 1; i < iter_count; i++)
    {
        if ((scr_x > 0) && (scr_x >> 8 < lbDisplay.GraphicsScreenWidth)
          && (scr_y > 0) && (scr_y >> 8 < lbDisplay.GraphicsScreenHeight))
        {
          struct SortLine *p_sline;
          int cor_x2, cor_y2;
          int cor_x1, cor_y1;
          int bckt;

          bckt = BUCKET_MID + (scr_depth >> 8) - 641;
          if ((itime < 0) || (colour == colour_lookup[ColLU_GREEN]))
              bckt -= 400;

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

void draw_bang(struct SimpleThing *p_pow)
{
    if (p_pow->State != 0)
    {
        short st;
        TbPixel col;
        st = p_pow->State;
        if (st < 100) {
            col = colour_lookup[ColLU_GREEN];
        } else {
            st = -10;
            col = colour_lookup[ColLU_PINK];
        }
        build_laser(p_pow->X >> 8, p_pow->Y >> 8, p_pow->Z >> 8,
          (p_pow->X >> 8), (p_pow->Y >> 8) + 400, p_pow->Z >> 8, st, 0, col);
    }

    dword_152E4C = bw_rotr32(dword_152E4C, 7) + 0x16365267;
    {
        short tmp;
        tmp = (dword_152E4C & 0xFF);
        if (tmp > 240)
            dword_176CAC = tmp - 240;
    }
    dword_152E4C = bw_rotr32(dword_152E4C, 7) + 0x16365267;

    draw_bang_wobble_line(p_pow);
    draw_bang_shrapnel(p_pow);
    draw_bang_phwoar(p_pow);
}

/**
 *
 * Before this call, the caller needs to ensure there is a free screen point.
 */
static void transform_rot_object_shpoint(struct ShEnginePoint *p_sp, int offset_x, int offset_y, int offset_z, ushort matx, ushort pt)
{
    struct SinglePoint *p_snpoint;
    struct SpecialPoint *p_specpt;

    p_snpoint = &game_object_points[pt];
    if ((p_snpoint->Flags & 0x40) != 0) // has sub-item alocated in `PointOffset`
    {
        p_specpt = &game_screen_point_pool[p_snpoint->PointOffset];
        p_sp->X = p_specpt->X;
        p_sp->Y = p_specpt->Y;
        p_sp->Depth = p_specpt->Z;
        p_sp->Flags = p_snpoint->Flags;
    }
    else
    {
        struct M31 vec_inp, vec_rot;
        int dxc, dyc, dzc;
        ushort pt;

        vec_inp.R[0] = 2 * p_snpoint->X;
        vec_inp.R[1] = 2 * p_snpoint->Y;
        vec_inp.R[2] = 2 * p_snpoint->Z;
        assert(matx < LOCAL_MATS_COUNT);
        matrix_transform(&vec_rot, &local_mats[matx], &vec_inp);

        dxc = offset_x + (vec_rot.R[0] >> 15);
        dyc = offset_y + (vec_rot.R[1] >> 15);
        dzc = offset_z + (vec_rot.R[2] >> 15);
        transform_shpoint(p_sp, dxc, dyc - 8 * engn_yc, dzc);

        pt = next_screen_point;
        next_screen_point++;

        p_specpt = &game_screen_point_pool[pt];
        p_specpt->X = p_sp->X;
        p_specpt->Y = p_sp->Y;
        p_specpt->Z = p_sp->Depth;

        p_snpoint->PointOffset = pt;
        p_snpoint->Flags = p_sp->Flags;
    }
}

int draw_rot_object(int offset_x, int offset_y, int offset_z, struct SingleObject *point_object, struct Thing *p_thing)
{
    int i, bckt_max;
    int face_beg, face;
    short faces_num;
    ushort faceWH, faceGF;

    bckt_max = 0;

    faceGF = 0;
    if ((p_thing->Type != TT_UNKN35) && (p_thing->SubType != SubTT_VEH_TRAIN))
    {
        short pos_x, pos_z;
        ushort darken;

        // Cannot get absolute map position from p_thing as it might be relative; so get it from the offset
        pos_x = engn_xc + offset_x;
        pos_z = engn_zc + offset_z;

        darken = 9;
        if (pos_x < TILE_TO_MAPCOORD(8, 0))
            darken = min(darken, pos_x >> 7);
        else if (pos_x > TILE_TO_MAPCOORD(MAP_TILE_WIDTH - 8, 0))
            darken = min(darken, (MAP_COORD_WIDTH - pos_x) >> 7);
        else if (pos_z < TILE_TO_MAPCOORD(8, 0))
            darken = min(darken, pos_z >> 7);
        else if (pos_z > TILE_TO_MAPCOORD(MAP_TILE_HEIGHT - 8, 0))
            darken = min(darken, (MAP_COORD_HEIGHT - pos_z) >> 7);

        if (darken < 9)
        {
            if (darken > 7)
                darken = 7;
            if (darken <= 0)
                darken = 1;
            faceGF |= (darken << 2);
        }
    }

    if ((p_thing->Flag & TngF_Unkn01000000) != 0)
    {
        p_thing->Flag &= ~TngF_Unkn01000000;
        faceWH = 11 - (gameturn & 3);
    }
    else
    {
      faceWH = 0;
    }

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        offset_y += waft_table[gameturn & 0x1F];

    for (i = point_object->StartPoint; i < point_object->EndPoint; i++)
    {
        struct SinglePoint *p_snpoint;

        p_snpoint = &game_object_points[i];
        p_snpoint->Flags = 0;
    }

    // This function can be called for objects, vehicles, mguns and rockets
    assert(offsetof(struct Thing, U.UObject.MatrixIndex) == offsetof(struct Thing, U.UVehicle.MatrixIndex));
    assert(offsetof(struct Thing, U.UObject.MatrixIndex) == offsetof(struct Thing, U.UMGun.MatrixIndex));
    assert(offsetof(struct Thing, U.UObject.MatrixIndex) == offsetof(struct Thing, U.UEffect.MatrixIndex));
    // Matrix for anything other than rocket shall respect the allocated entries counter
    assert((p_thing->U.UObject.MatrixIndex < next_local_mat) || (p_thing->Type == TT_ROCKET));

    for (i = point_object->OffsetX; i < point_object->OffsetY; i++)
    {
        struct M31 vec_nx, vec_rot;
        struct Normal *p_nrml;
        int fctr_p, fctr_s, fctr_o, fctr_r;

        p_nrml = &game_normals[i];
        vec_nx = *(struct M31 *)&p_nrml->NX;
        matrix_transform(&vec_rot, &local_mats[p_thing->U.UObject.MatrixIndex], &vec_nx);

        fctr_o = dword_176D14 * (vec_rot.R[0] >> 14) - dword_176D10 * (vec_rot.R[2] >> 14);
        fctr_p = (dword_176D14 * (vec_rot.R[2] >> 14) + dword_176D10 * (vec_rot.R[0] >> 14)) >> 16;
        fctr_r = dword_176D1C * (vec_rot.R[1] >> 14) - dword_176D18 * fctr_p;
        fctr_s = (dword_176D18 * (vec_rot.R[1] >> 14) + dword_176D1C * fctr_p) >> 16;

        p_nrml->LightRatio = 0;
        p_nrml->LightRatio |= ((fctr_o >> 19) & 0xFF);
        p_nrml->LightRatio |= (((fctr_r >> 19) & 0xFF) << 8);
        p_nrml->LightRatio |= ((fctr_s & 0xFFFF) << 16);
    }

    faces_num = point_object->NumbFaces;
    face_beg = point_object->StartFace;

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct ShEnginePoint sp1, sp2, sp3;
        struct SingleObjectFace3 *p_face;
        int depth_max, bckt;

        // each transform_rot_object_shpoint() call could reserve a point
        if (next_screen_point + 4 > screen_points_limit)
            break;

        p_face = &game_object_faces[face];
        p_face->GFlags &= ~0x1C;
        p_face->GFlags |= faceGF;
        p_face->WalkHeader = faceWH;

        transform_rot_object_shpoint(&sp1, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face->PointNo[0]);

        transform_rot_object_shpoint(&sp2, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face->PointNo[2]);

        transform_rot_object_shpoint(&sp3, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face->PointNo[1]);

        depth_max = SHRT_MIN;
        if (depth_max < sp1.Depth)
            depth_max = sp1.Depth;
        if (depth_max < sp2.Depth)
            depth_max = sp2.Depth;
        if (depth_max < sp3.Depth)
            depth_max = sp3.Depth;

        if ((sp1.Flags & sp2.Flags & sp3.Flags & 0xF) != 0)
            continue;
        if ((p_face->GFlags & 0x01) == 0) {
            if  ((sp2.X - sp1.X) * (sp3.Y - sp2.Y) -
              (sp3.X - sp2.X) * (sp2.Y - sp1.Y) <= 0)
                continue;
        }

        ubyte ditype;
        if ((p_face->GFlags & 0x80u) == 0)
            ditype = 7;
        else
            ditype = 17;
        bckt = BUCKET_MID + depth_max - 250;
        if (bckt_max < bckt)
            bckt_max = bckt;
        dword_176D68++;
        if (!draw_item_add(ditype, face, bckt))
            break;
    }

    faces_num = point_object->NumbFaces4;
    face_beg = point_object->StartFace4;

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct ShEnginePoint sp1, sp2, sp3, sp4;
        struct SingleObjectFace4 *p_face4;
        int depth_max, bckt;

        if (next_screen_point + 5 > screen_points_limit)
            break;

        p_face4 = &game_object_faces4[face];
        p_face4->GFlags &= ~0x1C;
        p_face4->GFlags |= faceGF;
        p_face4->WalkHeader = faceWH;

        transform_rot_object_shpoint(&sp1, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[0]);

        transform_rot_object_shpoint(&sp2, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[2]);

        transform_rot_object_shpoint(&sp3, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[1]);

        transform_rot_object_shpoint(&sp4, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[3]);

        depth_max = SHRT_MIN;
        if (depth_max < sp1.Depth)
            depth_max = sp1.Depth;
        if (depth_max < sp2.Depth)
            depth_max = sp2.Depth;
        if (depth_max < sp3.Depth)
            depth_max = sp3.Depth;
        if (depth_max < sp4.Depth)
            depth_max = sp4.Depth;

        if ((sp1.Flags & sp2.Flags & sp3.Flags & sp4.Flags & 0xF) != 0)
            continue;

        if ((p_face4->GFlags & 0x01) == 0) {
            if  ((sp2.X - sp1.X) * (sp3.Y - sp2.Y) -
              (sp3.X - sp2.X) * (sp2.Y - sp1.Y) <= 0)
                continue;
        }

        ubyte ditype;
        if ((p_face4->GFlags & 0x80) == 0)
            ditype = DrIT_Unkn16;
        else
            ditype = DrIT_Unkn18;
        bckt = BUCKET_MID + depth_max - 250;
        if (bckt_max < bckt)
            bckt_max = bckt;
        dword_176D68++;
        if (!draw_item_add(ditype, face, bckt))
            break;
    }

    // Plasma jumps when a vehicle got influenced by explosion or is crashing
    if ((p_thing->Type == TT_VEHICLE) && (p_thing->State == VehSt_UNKN_45 ||
      (p_thing->U.UVehicle.WorkPlace & VWPFlg_Unkn0080) != 0))
    {
        if ((LbRandomPosShort() & 0xFF) > 0xE0)
            dword_176CB0 = (LbRandomPosShort() & 0xFF) - 0xD0;

        if (dword_176CB0 && (LbRandomPosShort() & 0xFF) > 0x90)
        {
            int points_num, rnd_range;

            points_num = point_object->EndPoint - point_object->StartPoint;
            dword_176CB0--;
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

              build_wobble_line(p_specpt1->X, p_specpt1->Y, p_specpt1->Z - 1024,
                p_specpt2->X, p_specpt2->Y, p_specpt2->Z - 1024, 0, 10);
            }
        }
    }
    return bckt_max;
}

short draw_rot_object2(int offset_x, int offset_y, int offset_z,
  struct SingleObject *point_object, struct Thing *p_thing)
{
    int i, bckt_max;
    int face_beg, face;
    short faces_num;

    bckt_max = 0;

    faces_num = point_object->NumbFaces;
    face_beg = point_object->StartFace;

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct SingleObjectFace3 *p_face;
        struct SinglePoint *p_snpoint;

        p_face = &game_object_faces[face];

        p_snpoint = &game_object_points[p_face->PointNo[0]];
        p_snpoint->Flags = 0;
        p_snpoint = &game_object_points[p_face->PointNo[1]];
        p_snpoint->Flags = 0;
        p_snpoint = &game_object_points[p_face->PointNo[2]];
        p_snpoint->Flags = 0;
    }

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct ShEnginePoint sp1, sp2, sp3;
        struct SingleObjectFace3 *p_face;
        int depth_max, bckt;

        if (next_screen_point + 4 > screen_points_limit)
            break;

        p_face = &game_object_faces[face];

        transform_rot_object_shpoint(&sp1, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face->PointNo[0]);

        transform_rot_object_shpoint(&sp2, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face->PointNo[2]);

        transform_rot_object_shpoint(&sp3, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face->PointNo[1]);

        depth_max = SHRT_MIN;
        if (depth_max < sp1.Depth)
            depth_max = sp1.Depth;
        if (depth_max < sp2.Depth)
            depth_max = sp2.Depth;
        if (depth_max < sp3.Depth)
            depth_max = sp3.Depth;

        if ((sp1.Flags & sp2.Flags & sp3.Flags & 0xF) != 0)
            continue;
        if ((p_face->GFlags & 0x01) == 0) {
            if  ((sp2.X - sp1.X) * (sp3.Y - sp2.Y) -
              (sp3.X - sp2.X) * (sp2.Y - sp1.Y) <= 0)
                continue;
        }

        ubyte ditype;
        ditype = DrIT_Unkn1;
        bckt = BUCKET_MID + depth_max - 150;
        if (bckt_max < bckt)
            bckt_max = bckt;
        dword_176D68++;
        if (!draw_item_add(ditype, face, bckt))
            break;
    }

    faces_num = point_object->NumbFaces4;
    face_beg = point_object->StartFace4;

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct SingleObjectFace4 *p_face4;
        struct SinglePoint *p_snpoint;

        p_face4 = &game_object_faces4[face];

        p_snpoint = &game_object_points[p_face4->PointNo[0]];
        p_snpoint->Flags = 0;
        p_snpoint = &game_object_points[p_face4->PointNo[1]];
        p_snpoint->Flags = 0;
        p_snpoint = &game_object_points[p_face4->PointNo[2]];
        p_snpoint->Flags = 0;
        p_snpoint = &game_object_points[p_face4->PointNo[3]];
        p_snpoint->Flags = 0;
    }

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct ShEnginePoint sp1, sp2, sp3, sp4;
        struct SingleObjectFace4 *p_face4;
        int depth_max, bckt;

        if (next_screen_point + 5 > screen_points_limit)
            break;

        p_face4 = &game_object_faces4[face];

        transform_rot_object_shpoint(&sp1, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[0]);

        transform_rot_object_shpoint(&sp2, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[2]);

        transform_rot_object_shpoint(&sp3, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[1]);

        transform_rot_object_shpoint(&sp4, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[3]);

        depth_max = SHRT_MIN;
        if (depth_max < sp1.Depth)
            depth_max = sp1.Depth;
        if (depth_max < sp2.Depth)
            depth_max = sp2.Depth;
        if (depth_max < sp3.Depth)
            depth_max = sp3.Depth;
        if (depth_max < sp4.Depth)
            depth_max = sp4.Depth;

        if ((sp1.Flags & sp2.Flags & sp3.Flags & sp4.Flags & 0xF) != 0)
            continue;

        if ((p_face4->GFlags & 0x01) == 0) {
            if  ((sp2.X - sp1.X) * (sp3.Y - sp2.Y) -
              (sp3.X - sp2.X) * (sp2.Y - sp1.Y) <= 0)
                continue;
        }

        ubyte ditype;
        ditype = DrIT_Unkn9;
        bckt = BUCKET_MID + depth_max - 250;
        if (bckt_max < bckt)
            bckt_max = bckt;
        dword_176D68++;
        if (!draw_item_add(ditype, face, bckt))
            break;
    }

    return bckt_max;
}

short draw_object(int x, int y, int z, struct SingleObject *point_object)
{
    int i, bckt_max;
    int face;
    int snpoint;
    int obj_x, obj_y, obj_z;
    int points_num;
    short depth_shift;
    TbBool starts_below_window;

    bckt_max = 0;

    starts_below_window = 0;

    if ((game_perspective == 2) && engine_render_lights)
        return 0;

    obj_x = point_object->MapX - engn_xc;
    obj_y = point_object->OffsetY;
    obj_z = point_object->MapZ - engn_zc;
    depth_shift = point_object->field_1E;

    struct ShEnginePoint sp1, sp2, sp3;

    if (((ingame.Flags & GamF_DeepRadar) != 0) && (current_map != 11)) // map011 Orbital Station
    {
        int scr_y;

        scr_y = transform_shpoint_y(obj_x, obj_y - 8 * engn_yc, obj_z);
        if (scr_y >= vec_window_height + 10)
            starts_below_window = true;
    }

    if (things[point_object->ThingNo].U.UObject.BHeight <= 1400)
        starts_below_window = 0;
    if ((point_object->field_1C & 0x0100) != 0 && (word_1552F8 != 5))
        obj_y += waft_table[gameturn & 0x1F];

    // Make sure we have enough free points to start drawing the object
    points_num = point_object->EndPoint - point_object->StartPoint;
    if (next_screen_point + 1 * points_num > screen_points_limit)
        return 0;

    for (snpoint = point_object->StartPoint; snpoint <= point_object->EndPoint; snpoint++)
    {
        struct SinglePoint *p_snpoint;
        struct SpecialPoint *p_specpt;
        int dxc, dyc, dzc;

        p_snpoint = &game_object_points[snpoint];
        int specpt;

        dxc = p_snpoint->X + obj_x;
        dzc = p_snpoint->Z + obj_z;
        dyc = p_snpoint->Y + obj_y;
        transform_shpoint(&sp1, dxc, dyc - 8 * engn_yc, dzc);

        specpt = next_screen_point;
        next_screen_point++;

        p_specpt = &game_screen_point_pool[specpt];
        p_specpt->X = sp1.X;
        p_specpt->Y = sp1.Y;
        p_specpt->Z = sp1.Depth;

        p_snpoint->PointOffset = specpt;
        p_snpoint->Flags = sp1.Flags;
    }

    face = point_object->StartFace4;
    if (face > 0)
    {
        for (i = 0; i < point_object->NumbFaces4; i++, face++)
        {
            struct SingleObjectFace4 *p_face4;
            struct SinglePoint *p_snpoint1;
            struct SpecialPoint *p_specpt1;
            struct SinglePoint *p_snpoint2;
            struct SpecialPoint *p_specpt2;
            struct SinglePoint *p_snpoint3;
            struct SpecialPoint *p_specpt3;
            struct SinglePoint *p_snpoint4;
            struct SpecialPoint *p_specpt4;

            p_face4 = &game_object_faces4[face];
            if ((p_face4->GFlags & 0x08) != 0)
            {
                int specpt;
                int depth_max, bckt;
                int dxc, dyc, dzc;

                p_snpoint1 = &game_object_points[p_face4->PointNo[0]];
                dxc = p_snpoint1->X + obj_x;
                dzc = p_snpoint1->Z + obj_z;
                dyc = p_snpoint1->Y + obj_y;
                transform_shpoint(&sp2, dxc, dyc - 8 * engn_yc, dzc);

                specpt = next_screen_point;
                next_screen_point += 2;

                p_specpt1 = &game_screen_point_pool[specpt];
                p_specpt1->X = sp2.X;
                p_specpt1->Y = sp2.Y;
                p_specpt1->Z = sp2.Depth;

                p_snpoint1->PointOffset = specpt;
                p_snpoint1->Flags = sp2.Flags;

                p_snpoint2 = &game_object_points[p_face4->PointNo[1]];
                dxc = p_snpoint2->X + obj_x;
                dzc = p_snpoint2->Z + obj_z;
                dyc = p_snpoint2->Y + obj_y;
                transform_shpoint(&sp3, dxc, dyc - 8 * engn_yc, dzc);

                specpt++;

                p_specpt2 = &game_screen_point_pool[specpt];
                p_specpt2->X = sp3.X;
                p_specpt2->Y = sp3.Y;
                p_specpt2->Z = sp3.Depth;

                p_snpoint2->PointOffset = specpt;
                p_snpoint2->Flags = sp3.Flags;

                depth_max = SHRT_MIN;
                if (depth_max < sp2.Depth)
                    depth_max = sp2.Depth;
                if (depth_max < sp3.Depth)
                    depth_max = sp3.Depth;

                if ((sp3.Flags & 0x20) != 0 || (sp2.Flags & 0x20) != 0)
                    continue;
                if ((sp3.Flags & sp2.Flags & 0xF) != 0)
                    continue;

                ubyte ditype;
                ditype = DrIT_Unkn14;
                bckt = BUCKET_MID + depth_max;
                if (bckt_max < bckt)
                    bckt_max = bckt;
                dword_176D68++;
                if (!draw_item_add(ditype, face, bckt))
                    break;
            }
            else
            {
                int depth_max, bckt;
                ushort flags_all, flags_any;

                p_snpoint1 = &game_object_points[p_face4->PointNo[0]];
                p_specpt1 = &game_screen_point_pool[p_snpoint1->PointOffset];

                p_snpoint2 = &game_object_points[p_face4->PointNo[2]];
                p_specpt2 = &game_screen_point_pool[p_snpoint2->PointOffset];

                p_snpoint3 = &game_object_points[p_face4->PointNo[1]];
                p_specpt3 = &game_screen_point_pool[p_snpoint3->PointOffset];

                p_snpoint4 = &game_object_points[p_face4->PointNo[3]];
                p_specpt4 = &game_screen_point_pool[p_snpoint4->PointOffset];

                depth_max = SHRT_MIN;
                if (depth_max < p_specpt1->Z)
                    depth_max = p_specpt1->Z;
                if (depth_max < p_specpt2->Z)
                    depth_max = p_specpt2->Z;
                if (depth_max < p_specpt3->Z)
                    depth_max = p_specpt3->Z;
                if (depth_max < p_specpt4->Z)
                    depth_max = p_specpt4->Z;

                flags_all = p_snpoint4->Flags & p_snpoint3->Flags & p_snpoint2->Flags & p_snpoint1->Flags;
                flags_any = p_snpoint4->Flags | p_snpoint3->Flags | p_snpoint2->Flags | p_snpoint1->Flags;

                if ((flags_any & 0x20) != 0 || (flags_all & 0xF) != 0)
                    continue;

                if ((p_face4->GFlags & 0x01) == 0) {
                    if ((p_specpt2->X - p_specpt1->X) * (p_specpt3->Y - p_specpt2->Y) -
                      (p_specpt3->X - p_specpt2->X) * (p_specpt2->Y - p_specpt1->Y) <= 0)
                        continue;
                }

                ubyte ditype;
                if (starts_below_window)
                    ditype = DrIT_Unkn23;
                else
                    ditype = DrIT_Unkn9;
                bckt = BUCKET_MID + depth_shift + depth_max;
                if (bckt_max < bckt)
                    bckt_max = bckt;
                dword_176D68++;
                if (!draw_item_add(ditype, face, bckt))
                    break;
            }
        }
    }

    face = point_object->StartFace;
    if (face > 0)
    {
        for (i = 0; i < point_object->NumbFaces; i++, face++)
        {
            struct SingleObjectFace3 *p_face;
            struct SinglePoint *p_snpoint1;
            struct SpecialPoint *p_specpt1;
            struct SinglePoint *p_snpoint2;
            struct SpecialPoint *p_specpt2;
            struct SinglePoint *p_snpoint3;
            struct SpecialPoint *p_specpt3;

            p_face = &game_object_faces[face];
            {
                int depth_max, bckt;
                ushort flags_all, flags_any;

                p_snpoint1 = &game_object_points[p_face->PointNo[0]];
                p_specpt1 = &game_screen_point_pool[p_snpoint1->PointOffset];

                p_snpoint2 = &game_object_points[p_face->PointNo[1]];
                p_specpt2 = &game_screen_point_pool[p_snpoint2->PointOffset];

                p_snpoint3 = &game_object_points[p_face->PointNo[2]];
                p_specpt3 = &game_screen_point_pool[p_snpoint3->PointOffset];

                depth_max = SHRT_MIN;
                if (depth_max < p_specpt1->Z)
                    depth_max = p_specpt1->Z;
                if (depth_max < p_specpt2->Z)
                    depth_max = p_specpt2->Z;
                if (depth_max < p_specpt3->Z)
                    depth_max = p_specpt3->Z;

                flags_all = p_snpoint3->Flags & p_snpoint2->Flags & p_snpoint1->Flags;
                flags_any = p_snpoint3->Flags | p_snpoint2->Flags | p_snpoint1->Flags;

                if ((flags_any & 0x20) != 0 || (flags_all & 0xF) != 0)
                    continue;

                if ((p_face->GFlags & 0x01) == 0) {
                    if (((p_specpt3->X - p_specpt1->X) * (p_specpt2->Y - p_specpt3->Y) -
                      (p_specpt2->X - p_specpt3->X) * (p_specpt3->Y - p_specpt1->Y) <= 0))
                        continue;
                }

                ubyte ditype;
                if (starts_below_window)
                    ditype = DrIT_Unkn24;
                else
                    ditype = DrIT_Unkn1;
                bckt = BUCKET_MID + depth_shift + depth_max;
                if (bckt_max < bckt)
                    bckt_max = bckt;
                dword_176D68++;
                if (!draw_item_add(ditype, face, bckt))
                    break;
            }
        }
    }
    return bckt_max;
}

void draw_vehicle_health(struct Thing *p_thing)
{
    struct ShEnginePoint sp;
    int x, y, z;
    struct SortSprite *p_sspr;
    int bckt;
    int scr_depth;

    x = (p_thing->X >> 8) - engn_xc;
    y = PRCCOORD_TO_YCOORD(p_thing->Y) - engn_yc;
    z = (p_thing->Z >> 8) - engn_zc;
    transform_shpoint(&sp, x, y - 8 * engn_yc, z);

    scr_depth = sp.Depth - 2 * p_thing->Radius;
    bckt = BUCKET_MID + scr_depth;
    p_sspr = draw_item_add_sprite(DrIT_Unkn22, bckt);
    if (p_sspr == NULL)
        return;

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y + 20;
    p_sspr->Z = scr_depth;
    p_sspr->PThing = p_thing;
}

void build_polygon_circle_2d(int x1, int y1, int r1, int r2, int flag,
  struct SingleFloorTexture *p_tex, int col, int bright1, int bright2, int sort_key)
{
#if 0
    asm volatile (
      "push %9\n"
      "push %8\n"
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_build_polygon_circle_2d\n"
        : : "a" (x1), "d" (y1), "b" (r1), "c" (r2), "g" (flag), "g" (p_tex), "g" (col), "g" (bright1), "g" (bright2), "g" (sort_key));
    return;
#endif
    int pt3, pt4;
    int scrad1;
    int cur_x, cur_y;
    short angle, dt_angle, angle_detail;

    pt4 = next_screen_point;
    scrad1 = (overall_scale * r1) >> 8;
    angle_detail = 128;
    if ((scrad1 + x1 < 0) || (x1 - scrad1 > vec_window_width) || (scrad1 + y1 < 0) || (y1 - scrad1 > vec_window_height))
        return;

    if (scrad1 > 150)
        angle_detail = 16;
    else if (scrad1 > 50)
        angle_detail = 32;
    else if (scrad1 > 10)
        angle_detail = 64;

    cur_x = scrad1 + x1;
    cur_y = y1;
    pt4 = next_screen_point;
    pt3 = pt4;
    pt4++;

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
        struct SpecialPoint *p_specpt1;
        struct SpecialPoint *p_specpt2;
        struct SpecialPoint *p_specpt4;
        int nxt_x, nxt_y;
        int sin_angl, half_angl, cos_angl;
        int hlf_y, hlf_x;
        ushort face;

        half_angl = angle - angle_detail;
        cos_angl = lbSinTable[(half_angl & LbFPMath_AngleMask) + LbFPMath_PI/2];
        sin_angl = lbSinTable[(half_angl & LbFPMath_AngleMask)];
        hlf_x = x1 + ((scrad1 * cos_angl) >> 16);
        hlf_y = y1 + ((scrad1 * sin_angl) >> 16);
        cos_angl = lbSinTable[(angle & LbFPMath_AngleMask) + LbFPMath_PI/2];
        sin_angl = lbSinTable[angle & LbFPMath_AngleMask];
        nxt_x = x1 + ((scrad1 * cos_angl) >> 16);
        nxt_y = y1 + ((scrad1 * sin_angl) >> 16);

        face = next_special_face4;
        if (face + 1 > mem_game[25].N)
            break;
        next_special_face4++;

        p_face4 = &game_special_object_faces4[face];
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

        p_specpt1 = &game_screen_point_pool[pt4 + 2];
        p_specpt2 = &game_screen_point_pool[pt4 + 1];
        p_specpt4 = &game_screen_point_pool[pt4 + 0];
        pt4 += 3;

        p_specpt4->X = cur_x;
        p_specpt4->Y = cur_y;
        p_specpt2->X = hlf_x;
        p_specpt2->Y = hlf_y;
        p_specpt1->X = nxt_x;
        p_specpt1->Y = nxt_y;

        if (!draw_item_add(DrIT_Unkn12, face, sort_key))
            break;

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
    int scrad1;
    int cur_x, cur_y;
    int pt3, pt4;
    short angle, dt_angle, angle_detail;

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

    scrad1 = (overall_scale * r1) >> 8;
    if ((pp_X + scrad1 < 0) || (pp_X - scrad1 > vec_window_width))
        return;
    if ((pp_Y + scrad1 < 0) || (pp_Y - scrad1 > vec_window_height))
        return;

    if (scrad1 > 150)
        angle_detail = 16;
    else if (scrad1 > 50)
        angle_detail = 32;
    else if (scrad1 > 10)
        angle_detail = 64;
    else
        angle_detail = 128;

    pt3 = next_screen_point;
    cur_x = pp_X + scrad1;
    cur_y = pp_Y;
    {
        struct SpecialPoint *p_specpt3;
        p_specpt3 = &game_screen_point_pool[pt3];
        p_specpt3->X = pp_X;
        p_specpt3->Y = pp_Y;
    }

    pt4 = pt3 + 1;
    dt_angle = 2 * angle_detail;
    angle = dt_angle;
    while (angle <= 2048)
    {
        struct SingleObjectFace4 *p_face4;
        struct SpecialPoint *p_specpt1;
        struct SpecialPoint *p_specpt2;
        struct SpecialPoint *p_specpt4;
        int nxt_x, nxt_y;
        int sin_angl, half_angl, cos_angl;
        int hlf_y, hlf_x;
        ushort face;

        half_angl = angle - angle_detail;
        cos_angl = lbSinTable[(half_angl & 0x7FF) + LbFPMath_PI/2];
        sin_angl = lbSinTable[(half_angl & 0x7FF)];
        hlf_x = pp_X + ((scrad1 * cos_angl) >> 16);
        hlf_y = pp_Y + ((scrad1 * sin_angl) >> 16);
        cos_angl = lbSinTable[(angle & 0x7FF) + LbFPMath_PI/2];
        sin_angl = lbSinTable[angle & 0x7FF];
        nxt_x = pp_X + ((scrad1 * cos_angl) >> 16);
        nxt_y = pp_Y + ((scrad1 * sin_angl) >> 16);

        face = next_special_face4;
        if (face + 1 > mem_game[25].N)
            break;
        next_special_face4++;

        p_face4 = &game_special_object_faces4[face];
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

        p_specpt1 = &game_screen_point_pool[pt4 + 2];
        p_specpt2 = &game_screen_point_pool[pt4 + 1];
        p_specpt4 = &game_screen_point_pool[pt4 + 0];
        pt4 += 3;

        p_specpt4->X = cur_x;
        p_specpt4->Y = cur_y;
        p_specpt2->X = hlf_x;
        p_specpt2->Y = hlf_y;
        p_specpt1->X = nxt_x;
        p_specpt1->Y = nxt_y;

        if (!draw_item_add(DrIT_Unkn12, face, bckt))
            break;

        cur_x = nxt_x;
        cur_y = nxt_y;
        angle += dt_angle;
    }
  next_screen_point = pt4;
}
/******************************************************************************/
