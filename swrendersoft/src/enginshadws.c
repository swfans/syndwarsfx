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

#include "enginbckt.h"
#include "engincam.h"
#include "engintrns.h"
#include "engindrwlstx.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintxtrmap.h"
#include "enginprops.h"
#include "frame_sprani.h"
#include "render_gpoly.h"
/******************************************************************************/
//TODO load the shadow data from a config file
/** Per-object-model shadow data.
 */
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

extern const ushort word_154F4C[] = {
  1, 33, 113, 241, 273, 161, 193, 721, 753, 321, 401, 433, 33,
};


ubyte sprshadow_EE90[24];
ubyte sprshadow_EEA8[600];
ubyte sprshadow_F100[24];
ubyte sprshadow_F118[600];
ubyte sprshadow_F370[24];
ubyte sprshadow_F388[600];
ubyte sprshadow_F5E0[24];
ubyte sprshadow_F5F8[600];
sbyte sprshadow_F850[512];

ushort shadow_tmap_page = 0;

/******************************************************************************/

void draw_person_shadow(short scr_x, short scr_y, ushort frm,
  ushort shpak, ubyte shangl, ubyte angl, short strng)
{
    int ssh_y, ssh_x;
    int sh_x, sh_y;
    int sc_a, sc_b;
    int frgrp;
    ubyte k;

    struct EnginePoint ep4;
    struct EnginePoint ep2;
    struct EnginePoint ep1;
    struct EnginePoint ep3;

    vec_mode = 10;
    assert(vec_tmap[shadow_tmap_page] != NULL);
    vec_map = vec_tmap[shadow_tmap_page];

    frgrp =  8 * shpak + (((shangl >> 5) - angl + 8) & 7);
    ep3.pp.U = sprshadow_EE90[6 * frgrp + frm] << 16;
    ep3.pp.V = sprshadow_F5E0[6 * frgrp + frm] << 16;
    ep4.pp.U = sprshadow_F370[6 * frgrp + frm] << 16;
    ep4.pp.V = ep3.pp.V;
    ep1.pp.U = ep4.pp.U;
    ep1.pp.V = sprshadow_F100[6 * frgrp + frm] << 16;
    ep2.pp.U = ep3.pp.U;
    ep2.pp.V = ep1.pp.V;

    k = shangl - (engn_anglexz >> 8);
    ssh_x = sprshadow_F850[2 * k + 1];
    ssh_y = -sprshadow_F850[2 * k + 0];
    sh_y = (6 * ssh_y + 64) >> 7;
    sh_x = (6 * ssh_x + 64) >> 7; // We will reverse the sign later
    sh_x = (overall_scale * sh_x) >> 8;
    sh_y = (overall_scale * sh_y) >> 8;

    if (strng > 128)
        strng = 128;
    vec_colour = 16 + (strng >> 3);
    sc_a = (strng * sh_y) >> 6;
    sc_b = (strng * sh_x) >> 6;
    sh_x = -sh_x;

    ep3.pp.X = scr_x - sh_x;
    ep3.pp.Y = scr_y - sh_y;
    ep4.pp.X = scr_x + sh_x;
    ep4.pp.Y = scr_y + sh_y;

    if (strng > 64) {
        sh_x = (sh_x * strng) >> 6;
        sh_y = (sh_y * strng) >> 6;
    }

    ep1.pp.X = 4 * sc_a + scr_x + sh_x;
    ep1.pp.Y = 4 * sc_b + scr_y + sh_y;
    ep2.pp.X = 4 * sc_a + scr_x - sh_x;
    ep2.pp.Y = 4 * sc_b + scr_y - sh_y;

    dword_176D4C++;
    if (vec_mode == 2)
        vec_mode = 27;
    draw_trigpoly(&ep1.pp, &ep4.pp, &ep3.pp);
    dword_176D4C++;
    if (vec_mode == 2)
        vec_mode = 27;
    draw_trigpoly(&ep2.pp, &ep1.pp, &ep3.pp);
}

void draw_sort_sprite_person_shadow(ushort sspr)
{
    struct SortSprite *p_sspr;
    ushort shpak;
    short strng;
    ubyte shangl;

    p_sspr = &game_sort_sprites[sspr];

    shpak = p_sspr->Z;
    shangl = p_sspr->Brightness;
    strng = p_sspr->Scale;
    draw_person_shadow(p_sspr->X, p_sspr->Y, p_sspr->Frame,
      shpak, shangl, p_sspr->Angle, strng);
}

ushort draw_shadow_at_coords(struct SortMapPoint *p_cor1,
  struct SortMapPoint *p_cor2, struct SortMapPoint *p_cor3,
  struct SortMapPoint *p_cor4, struct ShadowTexture *p_shtextr,
  ushort sort)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct SingleObjectFace4 *p_face4;
    struct SingleFloorTexture *p_sftex;
    struct SpecialPoint *p_specpt;
    int bckt;
    ushort face, pt;
    short sftex;

    transform_shpoint(&sp1, p_cor1->X, p_cor1->Y - 8 * engn_yc, p_cor1->Z);

    transform_shpoint(&sp2, p_cor2->X, p_cor2->Y - 8 * engn_yc, p_cor2->Z);

    transform_shpoint(&sp3, p_cor3->X, p_cor3->Y - 8 * engn_yc, p_cor3->Z);

    transform_shpoint(&sp4, p_cor4->X, p_cor4->Y - 8 * engn_yc, p_cor4->Z);

    face = next_special_obj_face4;
    if (face + 1 > game_special_obj_faces4_limit)
        return 0;

    pt = next_screen_point;
    if (pt + 4 > screen_points_limit)
        return 0;

    next_special_obj_face4++;
    next_screen_point += 4;

    p_face4 = &game_special_obj_faces4[face];
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
    draw_item_add(DrIT_SpObFace4, face, bckt);
    return face;
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

void get_frame_bounds_05(ushort frm, short *x1, short *x2, short *y1, short *y2)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_get_frame_bounds_05\n"
        : : "a" (frm), "d" (x1), "b" (x2), "c" (y1), "g" (y2));
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
}

void draw_shadows_for_multicolor_sprites(void)
{
    int shpak;
    short v23mw;
    short v23hw;
    int v25a;

    overall_scale = 256;

    v23hw = 0;
    v25a = 0;
    v23mw = 0;
    for (shpak = 12; shpak >= 0; shpak--)
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

                get_frame_bounds_05(fr, &a2b, &a3a, &v25b, &a5a);
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
}

void generate_shadows_angle_shifts(void)
{
    int i;

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
}

/******************************************************************************/
