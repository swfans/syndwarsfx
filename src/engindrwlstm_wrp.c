/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstm_wrp.c
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
#include "engindrwlstm_wrp.h"

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
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "enginfloor.h"
#include "enginpeff.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginshrapn.h"
#include "enginprops.h"
#include "engintrns.h"
#include "frame_sprani.h"

#include "bigmap.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "game_speed.h"
#include "matrix.h"
#include "people.h"
#include "swlog.h"
#include "thing.h"
#include "vehicle.h"
/******************************************************************************/
#pragma pack(1)

struct BulStart {
    sbyte OffsetX;
    sbyte OffsetY;
};

struct unkn_mech_struc4 { // sizeof=0x7A
    short thing;
    ubyte field_2[33];
    short field_23;
    int field_25;
    int field_29;
    int field_2D;
    int field_31;
    int field_35;
    int field_39;
    int field_3D;
    int field_41;
    int field_45;
    int angle_49;
    int angle_4D;
    int angle_51;
    int field_55;
    int field_59;
    ubyte field_5D[4];
    struct M31 field_61;
    struct M31 field_6D;
    ubyte field_79;
};

struct unkn_mech_struc3 { // sizeof=0x76
    struct unkn_mech_struc4 *mech3_arr4_ptr;
    int mech3_unkn_arr4_idx;
    int mech3_unkn_fld_8;
    struct M33 mech3_unkn_mat_C;
    ubyte mech3_unkn_fld_30[25];
    int mech3_unkn_fld_49;
    ubyte mech3_unkn_fld_4D[7];
    int mech3_unkn_fld_54[3];
    ubyte field_60;
    int field_61;
    int field_65;
    int field_69;
    ubyte field_6D[3];
    int field_70;
    ubyte field_74;
    ubyte field_75;
};

#pragma pack()
/******************************************************************************/
extern short word_1552F8;

extern ubyte byte_176D49;

extern long dword_176CAC;
extern long dword_176CB0;
extern long dword_152E4C;

extern const ubyte byte_154F2C[32];

extern struct BulStart bul_starts[4000];

extern struct unkn_mech_struc3 *unkn_mech_arr3;

ubyte pers_shield_spr_vers[][5] = {
  {0, 1, 2, 0, 0,},
  {0, 2, 1, 0, 0,},
  {0, 1, 1, 0, 0,},
  {1, 2, 0, 0, 0,},
  {1, 1, 0, 0, 0,},
  {2, 0, 1, 0, 0,},
  {2, 0, 2, 0, 0,},
  {2, 0, 2, 0, 0,},
};

ubyte byte_152EF0[] = {
   0, 10,  5, 10,  7,  7,  8, 10,
  10, 10,  5,  7,  5,  7,  7,  0,
};


void draw_thing_e_graphic(struct Thing *p_thing, int x, int y, int z, ushort frame,
  int radius, int intensity)
{
    int depth_shift;

    if ((ingame.DisplayMode != 50) && ((p_thing->Flag2 & TgF2_InsideBuilding) != 0))
        depth_shift = BUCKETS_COUNT;
    else
        depth_shift = 0;

    enlist_draw_frame_graphic(x, y, z, frame, radius,
      intensity, depth_shift, (intptr_t)p_thing);
}

static void draw_pers_shadow(struct Thing *p_thing, int scr_x, int scr_y, int scr_depth)
{
    ushort frm, anmode;
    ushort shpak;
    short strng;
    ubyte shangl, angl;

    angl = p_thing->U.UObject.Angle;
    frm = p_thing->Frame - nstart_ani[p_thing->StartFrame + 1 + angl];

    anmode = p_thing->U.UPerson.AnimMode;
    if ((anmode == ANIM_PERS_WEPHEAVY_IDLE) ||
      (anmode == ANIM_PERS_WEPHEAVY_Unkn15) ||
      (anmode == ANIM_PERS_WEPHEAVY_Unkn07))
        shpak = 12;
    else if ((anmode == ANIM_PERS_WEPLIGHT_IDLE) ||
      (anmode == ANIM_PERS_Unkn14) ||
      (anmode == ANIM_PERS_Unkn06))
        shpak = byte_154F2C[2 * p_thing->SubType + 1];
    else
        shpak = byte_154F2C[2 * p_thing->SubType + 0];

    shangl = p_thing->U.UPerson.Shadows[0];
    strng = p_thing->U.UPerson.Shadows[1];

    enlist_draw_tall_spr_shadow(scr_x, scr_y, scr_depth, frm,
      angl, shangl, shpak, strng, (intptr_t)p_thing);
}

static void draw_pers_frame_basic(struct Thing *p_thing, int scr_x, int scr_y, int scr_depth, int frame, short bright)
{
    ubyte angl;

    angl = (p_thing->U.UObject.Angle + 8 - byte_176D49) & 7;

    enlist_draw_frame_pers_basic(scr_x, scr_y, scr_depth, frame,
      angl, bright, (intptr_t)p_thing);
}

static void draw_pers_frame_versioned(struct Thing *p_thing, int scr_x, int scr_y, int scr_depth, int frame, short bright)
{
    ubyte *frv;
    ubyte angl;

    if ((p_thing->U.UPerson.AnimMode == ANIM_PERS_Unkn12) ||
      ((ingame.Flags & GamF_ThermalView) != 0))
        bright = 32;
    if (((p_thing->Flag2 & TgF2_Unkn00080000) != 0) &&
      (p_thing->SubType == SubTT_PERS_ZEALOT))
        bright = 32;
    frv = p_thing->U.UPerson.FrameId.Version;
    angl = (p_thing->U.UObject.Angle + 8 - byte_176D49) & 7;

    enlist_draw_frame_pers_rot_versioned(scr_x, scr_y, scr_depth,
      frame, frv, angl, bright, (intptr_t)p_thing);
}

static void draw_pers_shield(struct Thing *p_thing, int scr_x, int scr_y, int scr_depth, short bright)
{
    ubyte *frv;
    ushort frame, k;
    ubyte angl;

    frame = shield_frm[p_thing->ThingOffset & 3];
    k = ((gameturn + 16 * p_thing->ThingOffset) >> 2) & 7;
    angl = (p_thing->U.UObject.Angle + 8 - byte_176D49) & 7;
    frv = pers_shield_spr_vers[k];

    enlist_draw_frame_effect_versioned(scr_x, scr_y, scr_depth,
      frame, frv, angl, bright, (intptr_t)p_thing);
}

void draw_pers_e_graphic(struct Thing *p_thing, int x, int y, int z, int frame, int radius, int intensity)
{
    struct ShEnginePoint sp;
    int scr_depth;
    short br_inc;
    short bri;

    bri = byte_152EF0[p_thing->SubType] + intensity;
    br_inc = person_shield_glow_brightness(p_thing);

    if ((render_floor_flags & RendFlrF_WobblyTerrain) != 0)
        y += waft_table[gameturn & 0x1F] >> 3;

    transform_shpoint(&sp, x, 8 * y - 8 * engn_yc, z);

    scr_depth = sp.Depth - ((radius * overall_scale) >> 8);
    if (ingame.DisplayMode == 50)
    {
        if ((p_thing->Flag2 & TgF2_InsideBuilding) != 0) {
            if ((p_thing->Flag & TngF_Destroyed) != 0) {
                return;
            }
            scr_depth -= 10 * BUCKETS_COUNT;
        }
    }
    else
    {
        if ((p_thing->Flag2 & TgF2_InsideBuilding) != 0)
            scr_depth = -BUCKETS_COUNT;
    }

    if (((p_thing->Flag2 & TgF2_InsideBuilding) != 0) && (ingame.DisplayMode == 50))
    {
        if ((ingame.Flags & GamF_ThermalView) != 0) {
            ushort frm;
            frm = nstart_ani[1066];
            bri = 32;
            draw_pers_frame_basic(p_thing, sp.X, sp.Y, scr_depth, frm, bri);
        }
    }
    else
    {
        draw_pers_frame_versioned(p_thing, sp.X, sp.Y, scr_depth, frame, bri + br_inc);
    }

    if (br_inc > 0) {
        draw_pers_shield(p_thing, sp.X, sp.Y, scr_depth - 1, br_inc);
    }

    if (((p_thing->Flag2 & TgF2_InsideBuilding) == 0) &&
      (p_thing->U.UPerson.OnFace == 0) &&
      (p_thing->SubType != SubTT_PERS_MECH_SPIDER))
    {
        draw_pers_shadow(p_thing, sp.X, sp.Y, scr_depth - ((200 * overall_scale) >> 8));
    }
}

void FIRE_draw_fire(struct SimpleThing *p_sthing)
{
    enlist_draw_fire_flames(p_sthing->U.UFire.flame);
}

void draw_bang_phwoar(struct SimpleThing *p_pow)
{
    enlist_draw_bang_phwoars(p_pow->U.UBang.phwoar);
}

void draw_bang_shrapnel(struct SimpleThing *p_pow)
{
    enlist_draw_bang_shrapnels(p_pow->U.UBang.shrapnel);
}

void build_wobble_line(int x1, int y1, int z1,
 int x2, int y2, int z2, struct SimpleThing *p_sthing, int itime)
{
    ubyte slflags;
    TbBool is_player;

    if ((p_sthing != NULL) && (p_sthing->Flag & TngF_PlayerAgent) != 0)
        is_player = 1;
    else
        is_player = 0;

    if (is_player && (p_sthing->Timer1 < 1))
        slflags = 0x02;
    else
        slflags = 0x01;

    enlist_draw_wobble_line(x1, y1, z1, x2, y2, z2, itime, slflags, is_player);
}

void draw_bang_wobble_line(struct SimpleThing *p_pow)
{
    if (dword_176CAC == 0)
        return;

    if ((dword_152E4C & 0xFF) <= 208)
        return;

    dword_176CAC--;

    enlist_draw_bang_wobble_line(p_pow->U.UBang.shrapnel);
}

void build_laser(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner, int colour)
{
    struct EnginePoint ep1, ep2;
    ubyte flg;
    int scr_x, scr_y, scr_depth;
    int scr_dx, scr_dy, scr_ddepth;
    int thick_x, thick_y;
    int i, iter_count;

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
static void transform_rot_object_shpoint(struct ShEnginePoint *p_sp,
  int offset_x, int offset_y, int offset_z, ushort matx, ushort pt)
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

TbBool triangle_not_visible(struct SpecialPoint *p_specpt1,
  struct SpecialPoint *p_specpt2, struct SpecialPoint *p_specpt3)
{
    return ((p_specpt2->X - p_specpt1->X) * (p_specpt3->Y - p_specpt2->Y) -
            (p_specpt2->Y - p_specpt1->Y) * (p_specpt3->X - p_specpt2->X) <= 0);
}

#define VisMDF_SkipFlg20 0x0100

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

int draw_rot_object(int offset_x, int offset_y, int offset_z, struct SingleObject *point_object, struct Thing *p_thing)
{
    int i, bckt_max;
    int face_beg, face;
    short depth_shift;
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

    depth_shift = -250;

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
        struct ShEnginePoint sp;
        struct SingleObjectFace3 *p_face;
        int depth_max, bckt;

        // each transform_rot_object_shpoint() call could reserve a point
        if (next_screen_point + 4 > screen_points_limit) {
            break;
        }

        p_face = &game_object_faces3[face];
        p_face->GFlags &= ~(FGFlg_Unkn10|FGFlg_Unkn08|FGFlg_Unkn04);
        p_face->GFlags |= faceGF;
        p_face->WalkHeader = faceWH;

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face->PointNo[0]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face->PointNo[2]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face->PointNo[1]);

        depth_max = object_face_get_visible_max_depth(p_face->PointNo[0],
          p_face->PointNo[2], p_face->PointNo[1], -1,
          p_face->GFlags | VisMDF_SkipFlg20);
        if (depth_max < SHRT_MIN)
            continue;

        ubyte ditype;
        if ((p_face->GFlags & FGFlg_Unkn80) == 0)
            ditype = DrIT_Unkn7;
        else
            ditype = DrIT_ObFace3Refl;
        bckt = BUCKET_MID + depth_shift + depth_max;
        if (bckt_max < bckt)
            bckt_max = bckt;
        stat_drawlist_faces++;
        if (!draw_item_add(ditype, face, bckt)) {
            break;
        }
    }

    faces_num = point_object->NumbFaces4;
    face_beg = point_object->StartFace4;

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct ShEnginePoint sp;
        struct SingleObjectFace4 *p_face4;
        int depth_max, bckt;

        if (next_screen_point + 5 > screen_points_limit) {
            break;
        }

        p_face4 = &game_object_faces4[face];
        p_face4->GFlags &= ~(FGFlg_Unkn10|FGFlg_Unkn08|FGFlg_Unkn04);
        p_face4->GFlags |= faceGF;
        p_face4->WalkHeader = faceWH;

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[0]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[2]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[1]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UVehicle.MatrixIndex, p_face4->PointNo[3]);

        depth_max = object_face_get_visible_max_depth(p_face4->PointNo[0],
          p_face4->PointNo[2], p_face4->PointNo[1], p_face4->PointNo[3],
          p_face4->GFlags | VisMDF_SkipFlg20);
        if (depth_max < SHRT_MIN)
            continue;

        ubyte ditype;
        if ((p_face4->GFlags & FGFlg_Unkn80) == 0)
            ditype = DrIT_Unkn16;
        else
            ditype = DrIT_ObFace4Refl;
        bckt = BUCKET_MID + depth_shift + depth_max;
        if (bckt_max < bckt)
            bckt_max = bckt;
        stat_drawlist_faces++;
        if (!draw_item_add(ditype, face, bckt)) {
            break;
        }
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
    short depth_shift;
    short faces_num;

    bckt_max = 0;

    faces_num = point_object->NumbFaces;
    face_beg = point_object->StartFace;
    depth_shift = -150;

    face = face_beg;
    for (i = 0; i < faces_num; i++, face++)
    {
        struct SingleObjectFace3 *p_face;
        struct SinglePoint *p_snpoint;

        p_face = &game_object_faces3[face];

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
        struct ShEnginePoint sp;
        struct SingleObjectFace3 *p_face;
        int depth_max, bckt;

        if (next_screen_point + 4 > screen_points_limit) {
            break;
        }

        p_face = &game_object_faces3[face];

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face->PointNo[0]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face->PointNo[2]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face->PointNo[1]);

        depth_max = object_face_get_visible_max_depth(p_face->PointNo[0],
          p_face->PointNo[2], p_face->PointNo[1], -1,
          p_face->GFlags | VisMDF_SkipFlg20);
        if (depth_max < SHRT_MIN)
            continue;

        ubyte ditype;
        ditype = DrIT_ObFace3Txtr;
        bckt = BUCKET_MID + depth_shift + depth_max;
        if (bckt_max < bckt)
            bckt_max = bckt;
        stat_drawlist_faces++;
        if (!draw_item_add(ditype, face, bckt)) {
            break;
        }
    }

    faces_num = point_object->NumbFaces4;
    face_beg = point_object->StartFace4;
    depth_shift = -250;

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
        struct ShEnginePoint sp;
        struct SingleObjectFace4 *p_face4;
        int depth_max, bckt;

        if (next_screen_point + 5 > screen_points_limit) {
            break;
        }

        p_face4 = &game_object_faces4[face];

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[0]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[2]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[1]);

        transform_rot_object_shpoint(&sp, offset_x, offset_y, offset_z,
          p_thing->U.UObject.MatrixIndex, p_face4->PointNo[3]);

        depth_max = object_face_get_visible_max_depth(p_face4->PointNo[0],
          p_face4->PointNo[2], p_face4->PointNo[1], p_face4->PointNo[3],
          p_face4->GFlags | VisMDF_SkipFlg20);
        if (depth_max < SHRT_MIN)
            continue;

        ubyte ditype;
        ditype = DrIT_ObFace4Txtr;
        bckt = BUCKET_MID + depth_shift + depth_max;
        if (bckt_max < bckt)
            bckt_max = bckt;
        stat_drawlist_faces++;
        if (!draw_item_add(ditype, face, bckt)) {
            break;
        }
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

            p_face4 = &game_object_faces4[face];
            if ((p_face4->GFlags & FGFlg_Unkn08) != 0)
            {
                int specpt;
                int depth_max, bckt;
                int dxc, dyc, dzc;

                specpt = next_screen_point;
                next_screen_point += 2;

                p_snpoint1 = &game_object_points[p_face4->PointNo[0]];
                dxc = p_snpoint1->X + obj_x;
                dzc = p_snpoint1->Z + obj_z;
                dyc = p_snpoint1->Y + obj_y;
                transform_shpoint(&sp2, dxc, dyc - 8 * engn_yc, dzc);

                p_snpoint1->PointOffset = specpt + 0;
                p_snpoint1->Flags = sp2.Flags;

                p_specpt1 = &game_screen_point_pool[p_snpoint1->PointOffset];
                p_specpt1->X = sp2.X;
                p_specpt1->Y = sp2.Y;
                p_specpt1->Z = sp2.Depth;

                p_snpoint2 = &game_object_points[p_face4->PointNo[1]];
                dxc = p_snpoint2->X + obj_x;
                dzc = p_snpoint2->Z + obj_z;
                dyc = p_snpoint2->Y + obj_y;
                transform_shpoint(&sp3, dxc, dyc - 8 * engn_yc, dzc);

                p_snpoint2->PointOffset = specpt + 1;
                p_snpoint2->Flags = sp3.Flags;

                p_specpt2 = &game_screen_point_pool[p_snpoint2->PointOffset];
                p_specpt2->X = sp3.X;
                p_specpt2->Y = sp3.Y;
                p_specpt2->Z = sp3.Depth;

                depth_max = object_face_get_visible_max_depth(p_face4->PointNo[0],
                  p_face4->PointNo[1], -1, -1,
                  p_face4->GFlags);
                if (depth_max < SHRT_MIN)
                    continue;

                ubyte ditype;
                ditype = DrIT_Unkn14;
                bckt = BUCKET_MID + depth_max;
                if (bckt_max < bckt)
                    bckt_max = bckt;
                stat_drawlist_faces++;
                if (!draw_item_add(ditype, face, bckt)) {
                    break;
                }
            }
            else
            {
                int depth_max, bckt;

                depth_max = object_face_get_visible_max_depth(p_face4->PointNo[0],
                  p_face4->PointNo[2], p_face4->PointNo[1], p_face4->PointNo[3],
                  p_face4->GFlags);
                if (depth_max < SHRT_MIN)
                    continue;

                ubyte ditype;
                if (starts_below_window)
                    ditype = DrIT_ObFace4Tran;
                else
                    ditype = DrIT_ObFace4Txtr;
                bckt = BUCKET_MID + depth_shift + depth_max;
                if (bckt_max < bckt)
                    bckt_max = bckt;
                stat_drawlist_faces++;
                if (!draw_item_add(ditype, face, bckt)) {
                    break;
                }
            }
        }
    }

    face = point_object->StartFace;
    if (face > 0)
    {
        for (i = 0; i < point_object->NumbFaces; i++, face++)
        {
            struct SingleObjectFace3 *p_face;

            p_face = &game_object_faces3[face];
            {
                int depth_max, bckt;

                depth_max = object_face_get_visible_max_depth(p_face->PointNo[0],
                  p_face->PointNo[2], p_face->PointNo[1], -1,
                  p_face->GFlags);
                if (depth_max < SHRT_MIN)
                    continue;

                ubyte ditype;
                if (starts_below_window)
                    ditype = DrIT_ObFace3Tran;
                else
                    ditype = DrIT_ObFace3Txtr;
                bckt = BUCKET_MID + depth_shift + depth_max;
                if (bckt_max < bckt)
                    bckt_max = bckt;
                stat_drawlist_faces++;
                if (!draw_item_add(ditype, face, bckt)) {
                    break;
                }
            }
        }
    }
    return bckt_max;
}

int mech_unkn_func_11(struct unkn_mech_struc4 *p_itm4, struct M31 *p_bodypos, struct M33 *p_mat)
{
    int ret;
    asm volatile ("call ASM_mech_unkn_func_11\n"
        : "=r" (ret) : "a" (p_itm4),  "d" (p_bodypos),  "b" (p_mat));
    return ret;
}

int mech_unkn_func_03(struct Thing *p_thing)
{
#if 0
    int ret;
    asm volatile ("call ASM_mech_unkn_func_03\n"
        : "=r" (ret) : "a" (p_thing));
    return ret;
#endif
    struct M31 bodypos;
    int mechno;

    mechno = p_thing->Owner;
    bodypos.R[0] = PRCCOORD_TO_MAPCOORD(p_thing->X);
    bodypos.R[2] = PRCCOORD_TO_MAPCOORD(p_thing->Z);
    bodypos.R[1] = PRCCOORD_TO_YCOORD(p_thing->Y) + 600;
    return mech_unkn_func_11(unkn_mech_arr3[mechno].mech3_arr4_ptr,
      &bodypos, &unkn_mech_arr3[mechno].mech3_unkn_mat_C);
}

void draw_vehicle_health(struct Thing *p_thing, int bckt)
{
    struct ShEnginePoint sp;
    int x, y, z;
    struct SortSprite *p_sspr;
    int scr_depth;
    TbPixel lvl_col, bar_col;

    x = (p_thing->X >> 8) - engn_xc;
    y = PRCCOORD_TO_YCOORD(p_thing->Y) - engn_yc;
    z = (p_thing->Z >> 8) - engn_zc;
    transform_shpoint(&sp, x, y - 8 * engn_yc, z);

    scr_depth = sp.Depth - 2 * p_thing->Radius;
    if (bckt > BUCKET_MID + scr_depth)
        bckt = BUCKET_MID + scr_depth;
    p_sspr = draw_item_add_sprite(DrIT_LongPropBar, bckt);
    if (p_sspr == NULL) {
        return;
    }

    if (ingame.PanelPermutation == -3) {
        lvl_col = 33;
        bar_col = 42;
    } else {
        lvl_col = 15;
        bar_col = 19;
    }

    p_sspr->X = sp.X;
    p_sspr->Y = sp.Y + 20;
    p_sspr->Z = scr_depth;
    p_sspr->SrcItem = (intptr_t)p_thing;
    p_sspr->Frame = p_thing->U.UVehicle.MaxHealth;
    p_sspr->Scale = p_thing->Health;
    p_sspr->Brightness = lvl_col;
    p_sspr->Angle = bar_col;
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

        p_face4 = draw_item_add_special_obj_face4_no_pts(DrIT_SpObFace4, sort_key);

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
/******************************************************************************/
