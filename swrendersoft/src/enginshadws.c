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

#define MUCOL_SHADOW_UV_ARRAY_SIZE MUCOL_SHADOW_ANIMS_COUNT * MUCOL_SHADOW_ANGLES_COUNT * MUCOL_SHADOW_FRAMES_COUNT

ubyte mucol_shadow_p3_Ua[MUCOL_SHADOW_UV_ARRAY_SIZE];
ubyte mucol_shadow_p1_Vb[MUCOL_SHADOW_UV_ARRAY_SIZE];
ubyte mucol_shadow_p4_Ua[MUCOL_SHADOW_UV_ARRAY_SIZE];
ubyte mucol_shadow_p3_Vb[MUCOL_SHADOW_UV_ARRAY_SIZE];

sbyte mucol_shadow_F850[512];

ushort shadow_tmap_page = 0;

/******************************************************************************/

void draw_multicolor_sprite_shadow(short scr_x, short scr_y, ushort sbfrm,
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

    vec_mode = RendVec_mode10;
    assert(vec_tmap[shadow_tmap_page] != NULL);
    vec_map = vec_tmap[shadow_tmap_page];

    assert(shpak < MUCOL_SHADOW_ANIMS_COUNT);
    assert(angl < MUCOL_SHADOW_ANGLES_COUNT);
    assert(sbfrm < MUCOL_SHADOW_FRAMES_COUNT);

    frgrp =  MUCOL_SHADOW_ANGLES_COUNT * shpak + (((shangl >> 5) - angl + 8) & 7);
    ep3.pp.U = mucol_shadow_p3_Ua[6 * frgrp + sbfrm] << 16;
    ep3.pp.V = mucol_shadow_p3_Vb[6 * frgrp + sbfrm] << 16;
    ep4.pp.U = mucol_shadow_p4_Ua[6 * frgrp + sbfrm] << 16;
    ep4.pp.V = ep3.pp.V;
    ep1.pp.U = ep4.pp.U;
    ep1.pp.V = mucol_shadow_p1_Vb[6 * frgrp + sbfrm] << 16;
    ep2.pp.U = ep3.pp.U;
    ep2.pp.V = ep1.pp.V;

    k = shangl - (engn_cam_yaw >> 8);
    ssh_x = mucol_shadow_F850[2 * k + 1];
    ssh_y = -mucol_shadow_F850[2 * k + 0];
    sh_y = (6 * ssh_y + 64) >> 7;
    sh_x = (6 * ssh_x + 64) >> 7; // We will reverse the sign later
    sh_x = (sh_x * overall_scale) >> 8;
    sh_y = (sh_y * overall_scale) >> 8;

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

    draw_trigpoly(&ep1.pp, &ep4.pp, &ep3.pp);
    stat_drawexec_faces++;

    draw_trigpoly(&ep2.pp, &ep1.pp, &ep3.pp);
    stat_drawexec_faces++;
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
    draw_multicolor_sprite_shadow(p_sspr->X, p_sspr->Y, p_sspr->Frame,
      shpak, shangl, p_sspr->Angle, strng);
}

ushort draw_shadow_at_coords(struct SortMapPoint *p_cor1,
  struct SortMapPoint *p_cor2, struct SortMapPoint *p_cor3,
  struct SortMapPoint *p_cor4, struct ShadowTexture *p_shtextr,
  int bckt)
{
    struct ShEnginePoint sp1, sp2, sp3, sp4;
    struct SingleObjectFace4 *p_face4;
    struct SingleFloorTexture *p_sftex;
    struct SpecialPoint *p_specpt;
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
    p_face4->Flags = RendVec_mode10;
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

    bckt++;
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

/** Get bounds of a frame, around its standing position.
 *
 * Returned screen coordinate shifts can be negative, when required.
 */
void get_frame_bounds_05(ushort frm, short *x1, short *x2, short *y1, short *y2)
{
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

void draw_shadows_for_multicolor_sprites(const ushort *anims, ushort anims_len)
{
    int shpak;
    short fr_max_height;
    TbScreenCoord cur_scr_x, cur_scr_y;

    assert(anims_len <= MUCOL_SHADOW_ANIMS_COUNT);

    overall_scale = 256;

    cur_scr_x = 0;
    cur_scr_y = 0;
    fr_max_height = 0;
    for (shpak = anims_len - 1; shpak >= 0; shpak--)
    {
        int base_idx;
        ushort angl;

        base_idx = shpak * MUCOL_SHADOW_ANGLES_COUNT;
        for (angl = 0; angl < MUCOL_SHADOW_ANGLES_COUNT/2; angl++)
        {
            ushort frm;
            ushort sbfrm;

            frm = nstart_ani[anims[shpak] + angl];
            for (sbfrm = 0; sbfrm < MUCOL_SHADOW_FRAMES_COUNT; sbfrm += 2)
            {
                int idx;
                TbScreenCoord fr_beg_x, fr_end_x, fr_beg_y, fr_end_y;
                TbScreenCoord stand_x, stand_y;
                // shadows are expected to fit in bitmap with both sizes
                // MUCOL_SHADOW_BITMAP_DIM, so coords within can be 8-bit
                ubyte fr_scr_beg_x, fr_scr_beg_y, fr_scr_end_x, fr_scr_end_y;

                get_frame_bounds_05(frm, &fr_beg_x, &fr_end_x, &fr_beg_y, &fr_end_y);

                // if cannot fit the sprite in this line, move to next line
                if (cur_scr_x + fr_end_x - fr_beg_x + 1 >= MUCOL_SHADOW_BITMAP_DIM)
                {
                    cur_scr_x = 0;
                    cur_scr_y += fr_max_height;
                    fr_max_height = 0;
                    assert(cur_scr_y < MUCOL_SHADOW_BITMAP_DIM);
                }
                if (fr_end_y - fr_beg_y + 1 > fr_max_height)
                    fr_max_height = fr_end_y - fr_beg_y + 1;

                stand_x = cur_scr_x - fr_beg_x;
                stand_y = cur_scr_y - fr_beg_y;
                draw_frame_alpha(stand_x, stand_y, frm, overall_scale, 10);

                fr_scr_beg_x = stand_x + fr_beg_x;
                fr_scr_beg_y = stand_y + fr_beg_y;
                fr_scr_end_x = stand_x + fr_end_x;
                fr_scr_end_y = stand_y + fr_end_y;

                // Prepare UV coordinates for drawing the shadow as texture
                idx = (base_idx + angl) * MUCOL_SHADOW_FRAMES_COUNT;
                mucol_shadow_p3_Ua[idx + sbfrm + 0] = fr_scr_beg_x;
                mucol_shadow_p1_Vb[idx + sbfrm + 0] = fr_scr_beg_y;
                mucol_shadow_p4_Ua[idx + sbfrm + 0] = fr_scr_end_x;
                mucol_shadow_p1_Vb[idx + sbfrm + 1] = fr_scr_beg_y;
                mucol_shadow_p4_Ua[idx + sbfrm + 1] = fr_scr_end_x;
                mucol_shadow_p3_Vb[idx + sbfrm + 0] = fr_scr_end_y;
                mucol_shadow_p3_Ua[idx + sbfrm + 1] = fr_scr_beg_x;
                mucol_shadow_p3_Vb[idx + sbfrm + 1] = fr_scr_end_y;
                if (angl != 0)
                {
                    idx = (base_idx + (MUCOL_SHADOW_ANGLES_COUNT - angl)) * MUCOL_SHADOW_FRAMES_COUNT;
                    mucol_shadow_p4_Ua[idx + sbfrm + 0] = fr_scr_beg_x;
                    mucol_shadow_p1_Vb[idx + sbfrm + 0] = fr_scr_beg_y;
                    mucol_shadow_p3_Ua[idx + sbfrm + 0] = fr_scr_end_x;
                    mucol_shadow_p3_Vb[idx + sbfrm + 0] = fr_scr_end_y;
                    mucol_shadow_p4_Ua[idx + sbfrm + 1] = fr_scr_beg_x;
                    mucol_shadow_p1_Vb[idx + sbfrm + 1] = fr_scr_beg_y;
                    mucol_shadow_p3_Ua[idx + sbfrm + 1] = fr_scr_end_x;
                    mucol_shadow_p3_Vb[idx + sbfrm + 1] = fr_scr_end_y;
                }
                else
                {
                    idx = (base_idx + MUCOL_SHADOW_ANGLES_COUNT/2) * MUCOL_SHADOW_FRAMES_COUNT;
                    mucol_shadow_p4_Ua[idx + sbfrm + 0] = fr_scr_beg_x;
                    mucol_shadow_p1_Vb[idx + sbfrm + 0] = fr_scr_beg_y;
                    mucol_shadow_p3_Ua[idx + sbfrm + 0] = fr_scr_end_x;
                    mucol_shadow_p3_Vb[idx + sbfrm + 0] = fr_scr_end_y;
                    mucol_shadow_p4_Ua[idx + sbfrm + 1] = fr_scr_beg_x;
                    mucol_shadow_p1_Vb[idx + sbfrm + 1] = fr_scr_beg_y;
                    mucol_shadow_p3_Ua[idx + sbfrm + 1] = fr_scr_end_x;
                    mucol_shadow_p3_Vb[idx + sbfrm + 1] = fr_scr_end_y;
                }
                frm = frame[frame[frm].Next].Next;
                cur_scr_x += fr_end_x - fr_beg_x + 1;
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

        mucol_shadow_F850[2 * i + 0] = x;
        mucol_shadow_F850[2 * i + 1] = y;
    }
}

/******************************************************************************/
