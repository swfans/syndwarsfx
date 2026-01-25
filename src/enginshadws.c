/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginshadws.c
 *     Shadows preparation and drawing required by the 3D engine.
 * @par Purpose:
 *     Implement functions for handling shadows in 3D world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 13 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginshadws.h"

#include <assert.h>
#include <string.h>
#include "bfmath.h"
#include "bfsprite.h"

#include "bigmap.h"
#include "display.h"
#include "enginbckt.h"
#include "engintrns.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintxtrmap.h"
#include "enginprops.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "game_sprts.h"
#include "matrix.h"
#include "render_gpoly.h"
#include "thing.h"
#include "swlog.h"
/******************************************************************************/
struct ShadowTexture {
    ushort Width;
    ushort Length;
    ubyte X1;
    ubyte Y1;
    ubyte X2;
    ubyte Y2;
};

struct ShadowTexture shadowtexture[] = {
  {300, 450,   0, 230,  20, 254},
  {420, 700,  21, 225,  42, 254},
  {150, 700, 143, 227, 151, 254},
  {460, 850,  43, 226,  66, 254},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {500, 800,  67, 236,  84, 254},
  {550, 900, 152, 226, 173, 254},
  {500, 800,  67, 236,  84, 254},
  {500, 800, 121, 228, 142, 254},
  {  0,   0,   0,   0,   0,   0},
  {500, 800,  85, 232,  94, 254},
  {500, 800,  85, 232,  94, 254},
  {400, 700,  67, 236,  84, 254},
  {500, 800,  95, 226, 120, 254},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {700, 900,  67, 236,  84, 254},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {300, 450,   0, 230,  20, 254},
  {420, 700,  21, 225,  42, 254},
  {  0,   0,   0,   0,   0,   0},
  {500, 800,  95, 226, 120, 254},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
  {  0,   0,   0,   0,   0,   0},
};

extern const ubyte byte_154F2C[32];
extern const ushort word_154F4C[14];

extern ubyte sprshadow_EE90[24];
extern ubyte sprshadow_EEA8[600];
extern ubyte sprshadow_F100[24];
extern ubyte sprshadow_F118[600];
extern ubyte sprshadow_F370[24];
extern ubyte sprshadow_F388[600];
extern ubyte sprshadow_F5E0[24];
extern ubyte sprshadow_F5F8[600];
extern sbyte sprshadow_F850[512];

int alt_at_point_under_height(int x, int z, int h)
{
    int ret;
    asm volatile (
      "call ASM_alt_at_point_under_height\n"
        : "=r" (ret) : "a" (x), "d" (z), "b" (h));
    return ret;
}

void draw_person_shadow(ushort face)
{
#if 0
    asm volatile (
      "call ASM_draw_person_shadow\n"
        : : "a" (face));
    return;
#endif
    struct Thing *p_thing;
    ushort fr, anmode;
    ushort shpak;
    int frgrp;
    ubyte k;
    int ssh_y, ssh_x;
    int sh_x, sh_y;
    int sc_a, sc_b;
    short strng;

    struct SortSprite *sspr;
    struct EnginePoint point4;
    struct EnginePoint point2;
    struct EnginePoint point1;
    struct EnginePoint point3;

    p_thing = game_sort_sprites[face].PThing;
    vec_mode = 10;
    assert(vec_tmap[ingame.LastTmap] != NULL);
    vec_map = vec_tmap[ingame.LastTmap];

    fr = p_thing->Frame - nstart_ani[p_thing->StartFrame + 1 + p_thing->U.UObject.Angle];
    anmode = p_thing->U.UPerson.AnimMode;
    if ((anmode == ANIM_PERS_WEPHEAVY_IDLE) || (anmode == ANIM_PERS_WEPHEAVY_Unkn15) || (anmode == ANIM_PERS_WEPHEAVY_Unkn07))
        shpak = 12;
    else if ((anmode == ANIM_PERS_WEPLIGHT_IDLE) || (anmode == ANIM_PERS_Unkn14) || (anmode == ANIM_PERS_Unkn06))
        shpak = byte_154F2C[2 * p_thing->SubType + 1];
    else
        shpak = byte_154F2C[2 * p_thing->SubType + 0];

    frgrp =  8 * shpak + (((p_thing->U.UPerson.Shadows[0] >> 5) - p_thing->U.UObject.Angle + 8) & 7);
    point3.pp.U = sprshadow_EE90[6 * frgrp + fr] << 16;
    point3.pp.V = sprshadow_F5E0[6 * frgrp + fr] << 16;
    point4.pp.U = sprshadow_F370[6 * frgrp + fr] << 16;
    point4.pp.V = point3.pp.V;
    point1.pp.U = point4.pp.U;
    point1.pp.V = sprshadow_F100[6 * frgrp + fr] << 16;
    point2.pp.U = point3.pp.U;
    point2.pp.V = point1.pp.V;

    k = p_thing->U.UPerson.Shadows[0] - (engn_anglexz >> 8);
    ssh_x = sprshadow_F850[2 * k + 1];
    ssh_y = -sprshadow_F850[2 * k + 0];
    sh_y = (6 * ssh_y + 64) >> 7;
    sh_x = (6 * ssh_x + 64) >> 7; // We will reverse the sign later
    sh_x = (overall_scale * sh_x) >> 8;
    sh_y = (overall_scale * sh_y) >> 8;
    strng = p_thing->U.UPerson.Shadows[1];
    if (strng > 128)
        strng = 128;
    vec_colour = (strng >> 3) + 16;
    sc_a = (strng * sh_y) >> 6;
    sc_b = (strng * sh_x) >> 6;
    sh_x = -sh_x;

    sspr = &game_sort_sprites[face];
    point3.pp.X = sspr->X - sh_x;
    point3.pp.Y = sspr->Y - sh_y;
    point4.pp.X = sspr->X + sh_x;
    point4.pp.Y = sspr->Y + sh_y;

    if (strng > 64) {
        sh_x = (sh_x * strng) >> 6;
        sh_y = (sh_y * strng) >> 6;
    }

    sspr = &game_sort_sprites[face];
    point1.pp.X = 4 * sc_a + sspr->X + sh_x;
    point1.pp.Y = 4 * sc_b + sspr->Y + sh_y;
    point2.pp.X = 4 * sc_a + sspr->X - sh_x;
    point2.pp.Y = 4 * sc_b + sspr->Y - sh_y;

    dword_176D4C++;
    if (vec_mode == 2)
        vec_mode = 27;
    draw_trigpoly(&point1.pp, &point4.pp, &point3.pp);
    dword_176D4C++;
    if (vec_mode == 2)
        vec_mode = 27;
    draw_trigpoly(&point2.pp, &point1.pp, &point3.pp);
}

void draw_vehicle_shadow(ushort veh, ushort sort)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct M31 vec_inp;
    struct M31 vec_rot;
    struct Thing *p_vehicle;
    struct ShadowTexture *p_shtextr;
    struct SingleObjectFace4 *p_face4;
    struct SingleFloorTexture *p_sftex;
    struct SpecialPoint *p_specpt;
    int shd_w, shd_l;
    short cor1_x, cor1_y, cor1_z;
    short cor2_x, cor2_y, cor2_z;
    short cor3_x, cor3_y, cor3_z;
    short cor4_x, cor4_y, cor4_z;
    ushort face, pt;
    short sftex;
    int bckt;

    p_vehicle = &things[veh];
    p_shtextr = &shadowtexture[p_vehicle->StartFrame];
    if (p_shtextr->Width == 0)
        return;

    shd_w = p_shtextr->Width;
    shd_l = p_shtextr->Length;

    vec_inp.R[0] = -shd_w;
    vec_inp.R[2] = -shd_l;
    vec_inp.R[1] = 0;
    assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[p_vehicle->U.UVehicle.MatrixIndex], &vec_inp);
    cor1_x = (p_vehicle->X >> 8) - engn_xc + (vec_rot.R[0] >> 15);
    cor1_z = (p_vehicle->Z >> 8) - engn_zc + (vec_rot.R[2] >> 15);

    vec_inp.R[1] = 0;
    vec_inp.R[0] = shd_w;
    vec_inp.R[2] = -shd_l;
    assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[p_vehicle->U.UVehicle.MatrixIndex], &vec_inp);
    cor2_x = (p_vehicle->X >> 8) - engn_xc + (vec_rot.R[0] >> 15);
    cor2_z = (p_vehicle->Z >> 8) - engn_zc + (vec_rot.R[2] >> 15);

    vec_inp.R[0] = p_shtextr->Width;
    vec_inp.R[1] = 0;
    vec_inp.R[2] = shd_l;
    assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[p_vehicle->U.UVehicle.MatrixIndex], &vec_inp);
    cor3_x = (p_vehicle->X >> 8) - engn_xc + (vec_rot.R[0] >> 15);
    cor3_z = (p_vehicle->Z >> 8) - engn_zc + (vec_rot.R[2] >> 15);

    vec_inp.R[0] = -p_shtextr->Width;
    vec_inp.R[1] = 0;
    vec_inp.R[2] = shd_l;
    assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[p_vehicle->U.UVehicle.MatrixIndex], &vec_inp);
    cor4_x = (p_vehicle->X >> 8) - engn_xc + (vec_rot.R[0] >> 15);
    cor4_z = (p_vehicle->Z >> 8) - engn_zc + (vec_rot.R[2] >> 15);

    if (p_vehicle->SubType == 40) {
        cor1_y = alt_at_point_under_height(engn_xc + cor1_x, engn_zc + cor1_z, p_vehicle->Y) >> 8;
        cor2_y = alt_at_point_under_height(engn_xc + cor2_x, engn_zc + cor2_z, p_vehicle->Y) >> 8;
        cor3_y = alt_at_point_under_height(engn_xc + cor3_x, engn_zc + cor3_z, p_vehicle->Y) >> 8;
        cor4_y = alt_at_point_under_height(engn_xc + cor4_x, engn_zc + cor4_z, p_vehicle->Y) >> 8;
    } else {
        cor1_y = PRCCOORD_TO_YCOORD(alt_at_point(engn_xc + cor1_x, engn_zc + cor1_z));
        cor2_y = PRCCOORD_TO_YCOORD(alt_at_point(engn_xc + cor2_x, engn_zc + cor2_z));
        cor3_y = PRCCOORD_TO_YCOORD(alt_at_point(engn_xc + cor3_x, engn_zc + cor3_z));
        cor4_y = PRCCOORD_TO_YCOORD(alt_at_point(engn_xc + cor4_x, engn_zc + cor4_z));
    }

    transform_shpoint(&sp1, cor1_x, cor1_y - 8 * engn_yc, cor1_z);

    transform_shpoint(&sp2, cor2_x, cor2_y - 8 * engn_yc, cor2_z);

    transform_shpoint(&sp3, cor3_x, cor3_y - 8 * engn_yc, cor3_z);

    transform_shpoint(&sp4, cor4_x, cor4_y - 8 * engn_yc, cor4_z);

    face = next_special_face4;
    if (face + 1 > mem_game[25].N)
        return;

    pt = next_screen_point;
    if (pt + 4 > screen_points_limit)
        return;

    next_special_face4++;
    next_screen_point += 4;

    p_face4 = &game_special_object_faces4[face];
    p_face4->Flags = 10;
    p_face4->GFlags = 0x01;
    p_face4->ExCol = 16;
    p_face4->PointNo[1] = pt + 1;
    p_face4->PointNo[2] = pt + 3;
    p_face4->PointNo[0] = pt + 0;
    p_face4->PointNo[3] = pt + 2;

    sftex = tnext_floor_texture;
    if (sftex == 0)
        tnext_floor_texture = next_floor_texture;
    p_face4->Texture = tnext_floor_texture;
    p_sftex = &game_textures[tnext_floor_texture];
    tnext_floor_texture++;

    p_sftex->TMapX1 = p_shtextr->X1;
    p_sftex->TMapY1 = p_shtextr->Y1;
    p_sftex->TMapX2 = p_shtextr->X2;
    p_sftex->TMapY2 = p_shtextr->Y1;
    p_sftex->TMapX4 = p_shtextr->X2;
    p_sftex->TMapY4 = p_shtextr->Y2;
    p_sftex->TMapX3 = p_shtextr->X1;
    p_sftex->TMapY3 = p_shtextr->Y2;
    p_sftex->Page = 4;

    p_specpt = &game_screen_point_pool[pt + 0];
    p_specpt->X = sp1.X;
    p_specpt->Y = sp1.Y;

    p_specpt = &game_screen_point_pool[pt + 1];
    p_specpt->X = sp2.X;
    p_specpt->Y = sp2.Y;

    p_specpt = &game_screen_point_pool[pt + 2];
    p_specpt->X = sp3.X;
    p_specpt->Y = sp3.Y;

    p_specpt = &game_screen_point_pool[pt + 3];
    p_specpt->X = sp4.X;
    p_specpt->Y = sp4.Y;

    bckt = sort + 1;
    draw_item_add(DrIT_Unkn12, face, bckt);
}

void copy_from_screen_ani(ubyte *buf)
{
    int y;
    ubyte *o;
    const ubyte *inp;

    o = buf;
    inp = lbDisplay.WScreen;
    for (y = 0; y < 256; y++)
    {
        memcpy(o, inp, 256);
        o += 256;
        inp += lbDisplay.GraphicsScreenWidth;
    }
}

struct Element *element_unkn_func_05(ushort frm, short *x1, short *x2, short *y1, short *y2)
{
#if 0
    struct Element *ret;
    asm volatile (
      "push %5\n"
      "call ASM_element_unkn_func_05\n"
        : "=r" (ret) : "a" (frm), "d" (x1), "b" (x2), "c" (y1), "g" (y2));
    return ret;
#endif
    struct Element *p_el;
    struct Frame *p_frm;
    ushort el;

    *y1 = 32000;
    *x1 = *y1;
    *y2 = -32000;
    *x2 = *y2;

    p_frm = &frame[frm];
    for (el = p_frm->FirstElement; el > 0; el = p_el->Next)
    {
        struct TbSprite *p_spr;

        p_el = &melement_ani[el];
        if ((p_el < melement_ani) || (p_el >= mele_ani_end))
            break;

        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_el->ToSprite);
        if ((p_spr < m_sprites) || (p_spr >= m_sprites_end))
            continue;

        if ((p_el->Flags & 0xFE00) == 0)
        {
            short scr_beg_x, scr_beg_y;
            short scr_fin_x, scr_fin_y;

            scr_beg_x = p_el->X >> 1;
            scr_beg_y = p_el->Y >> 1;
            scr_fin_x = scr_beg_x + p_spr->SWidth;
            scr_fin_y = scr_beg_y + p_spr->SHeight;
            if (scr_fin_x > *x2)
                *x2 = scr_fin_x;
            if (scr_beg_x < *x1)
                *x1 = scr_beg_x;
            if (scr_fin_y > *y2)
                *y2 = scr_fin_y;
            if (scr_beg_y < *y1)
                *y1 = scr_beg_y;
        }
    }
    return p_el;
}

/** Prepare buffer with sprite shadows.
 * Clear the wscreen buffer before this call. Also make sure m_sprites are loaded.
 */
void generate_shadows_for_multicolor_sprites(void)
{
#if 0
    asm volatile ("call ASM_generate_shadows_for_multicolor_sprites\n"
        :  :  : "eax" );
    return;
#endif
    struct ScreenBufBkp bkp;
    int shpak;
    short v23mw;
    short v23hw;
    int v25a;
    int i;

    // TODO would be better to use some back buffer instead of normal screen buf
    screen_switch_to_custom_buffer(&bkp, lbDisplay.WScreen,
      lbDisplay.GraphicsScreenWidth, 256);
    LbScreenClear(0);

    overall_scale = 256;

    v23hw = 0;
    v25a = 0;
    v23mw = 0;
    for (shpak = 12; shpak > -1; shpak--)
    {
        int base_idx;
        ushort spr;

        base_idx = shpak * 8 * 6;
        for (spr = 0; spr < 4; spr++)
        {
            ushort fr;
            ushort kk;

            fr = nstart_ani[spr + word_154F4C[shpak]];
            for (kk = 0; kk < 6; kk += 2)
            {
                int idx;
                short a2b, a3a, v25b, a5a;
                ubyte val3a, val5a, val5b, val6a, val8a;
                short v21;

                element_unkn_func_05(fr, &a2b, &a3a, &v25b, &a5a);
                if (v23hw + a3a - a2b + 1 > 255)
                {
                    v23hw = 0;
                    v25a += v23mw;
                    v23mw = 0;
                }
                if (a5a - v25b + 1 > v23mw)
                    v23mw = a5a - v25b + 1;
                v23hw -= a2b;
                v21 = v25a - v25b;
                draw_sorted_sprite1a(fr, v23hw, v21, 10);

                val3a = v23hw + a2b;
                val5a = v21 + v25b;
                val5b = v23hw + a2b;
                val6a = v23hw + a3a;
                val8a = v21 + a5a;

                idx = base_idx + 6 * spr;
                sprshadow_EE90[kk + idx + 0] = val5b;
                sprshadow_F100[kk + idx + 0] = val5a;
                sprshadow_F370[kk + idx + 0] = val6a;
                sprshadow_F100[kk + idx + 1] = val5a;
                sprshadow_F370[kk + idx + 1] = val6a;
                sprshadow_F5E0[kk + idx + 0] = val8a;
                sprshadow_EE90[kk + idx + 1] = val3a;
                sprshadow_F5E0[kk + idx + 1] = val8a;
                if (spr != 0)
                {
                  idx = base_idx + 6 * (8 - spr);
                  sprshadow_F370[kk + idx + 0] = val3a;
                  sprshadow_F100[kk + idx + 0] = val5a;
                  sprshadow_EE90[kk + idx + 0] = val6a;
                  sprshadow_F5E0[kk + idx + 0] = val8a;
                  sprshadow_F370[kk + idx + 1] = val3a;
                  sprshadow_F100[kk + idx + 1] = val5a;
                  sprshadow_EE90[kk + idx + 1] = val6a;
                  sprshadow_F5E0[kk + idx + 1] = val8a;
                }
                else
                {
                  idx = base_idx;
                  sprshadow_F388[kk + idx + 0] = val3a;
                  sprshadow_F118[kk + idx + 0] = val5a;
                  sprshadow_EEA8[kk + idx + 0] = val6a;
                  sprshadow_F5F8[kk + idx + 0] = val8a;
                  sprshadow_F388[kk + idx + 1] = val3a;
                  sprshadow_F118[kk + idx + 1] = val5a;
                  sprshadow_EEA8[kk + idx + 1] = val6a;
                  sprshadow_F5F8[kk + idx + 1] = val8a;
                }
                fr = frame[frame[fr].Next].Next;
                v23hw += a3a + 1;
            }
        }
    }

    copy_from_screen_ani(vec_tmap[ingame.LastTmap]);

    for (i = 0; i < 256; i++)
    {
        int x, y, angle;

        angle = 8 * i;
        x = lbSinTable[angle] >> 9;
        y = lbSinTable[angle + LbFPMath_PI/2] >> 9;
        if (x > 127)
            x = 127;
        if (x < -128)
            x = -128;
        if (y > 127)
            y = 127;
        if (y < -128)
            y = -128;

        sprshadow_F850[2 * i + 0] = x;
        sprshadow_F850[2 * i + 1] = y;
    }

    screen_load_backup_buffer(&bkp);
}

/******************************************************************************/
