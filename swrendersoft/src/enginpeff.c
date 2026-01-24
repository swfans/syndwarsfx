/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginpeff.c
 *     Engine scene pre-/post-processing effects.
 * @par Purpose:
 *     Implement functions for adding effects while rendering a scene.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Oct 2023 - 02 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginpeff.h"

#include "bfgentab.h"
#include "bfmath.h"
#include "bfpixel.h"
#include "bfscreen.h"
#include "bfutility.h"

#include "enginbckt.h"
#include "engincolour.h"
#include "enginprops.h"
#include "engintrns.h"
#include "engintxtrmap.h"
#include "scanner.h"
#include "privrdlog.h"

#include "display.h"
/******************************************************************************/
ushort gamep_scene_effect_intensity = 1000;
short gamep_scene_effect_change = -1;

ushort word_1A7314;
ushort word_1A7330[1000];
ubyte byte_1A7B00[1000];
ubyte byte_1A7EE8[9004];

void scene_post_effect_rain_init(void)
{
#if 0
    asm volatile ("call ASM_scene_post_effect_rain_init\n"
        :  :  : "eax" );
    return;
#endif
    uint i;
    ushort idx3;

    for (i = 0; i < 999; i++)
    {
        idx3 = (i / 333) % 3;
        if (idx3 == 0)
        {
            uint rnd;
            ushort idx1, idx2;
            ubyte *tmap;

            rnd = (render_anim_turn >> 2) + LbRandomAnyShort();
            idx1 = (rnd >> 3) & 0x1FFF;
            if ((byte_1A7EE8[idx1] & (1 << (idx1 & 7))) == 0)
            {
                idx2 = rnd & 0xFFFF;
                word_1A7330[i] = idx2;
                tmap = vec_tmap[0];
                byte_1A7B00[i] = tmap[idx2];
                tmap[idx2] = 63;
                byte_1A7EE8[idx1] |= (1 << (idx1 & 7));
            }
        }
    }
}

void water_droplets_on_floor(void)
{
#if 0
    asm volatile ("call ASM_water_droplets_on_floor\n"
        :  : );
#else
    ushort i, k;

    for (i = 0; i < 999; i++)
    {
        ubyte *tmap;
        ushort n, step;

        step = (i / 333 + word_1A7314);
        switch (step % 3)
        {
        case 0:
            k = word_1A7330[i];
            tmap = vec_tmap[0];
            if (k != 0) {
                tmap[k] = byte_1A7B00[i];
            }
            n = k >> 3;
            byte_1A7EE8[n] &= ~(1 << (n & 7));
            k = (render_anim_turn >> 2) + LbRandomAnyShort();
            n = k >> 3;
            if (((1 << (n & 7)) & byte_1A7EE8[n]) == 0)
            {
                ubyte colA;

                word_1A7330[i] = k;
                colA = tmap[k];
                byte_1A7B00[i] = colA;
                tmap[k] = pixmap.fade_table[0x3200 + colA];
                byte_1A7EE8[n] |= 1 << (n & 7);
            }
            break;
        case 1:
            k = word_1A7330[i];
            tmap = vec_tmap[0];
            if (k != 0) {
                tmap[k] = lbDisplay.GlassMap[256 * byte_1A7B00[i] + tmap[k]];
            }
            break;
        case 2:
            k = word_1A7330[i];
            tmap = vec_tmap[0];
            if (k != 0) {
                tmap[k] = lbDisplay.GlassMap[256 * byte_1A7B00[i] + tmap[k]];
            }
            break;
        }
    }
    word_1A7314++;
#endif
}

void scene_post_effect_texture_with_snow(void)
{
    int i;

    for (i = 0; i < 10; i++) {
        ushort pos;
        ubyte *ptr;
        pos = LbRandomAnyShort() + (render_anim_turn >> 2);
        ptr = vec_tmap[0] + pos;
        *ptr = pixmap.fade_table[40*PALETTE_8b_COLORS + *ptr];
    }
}

void scene_post_effect_prepare(void)
{
#if 0 // experimental function to alter effect intensity
    if ((LbRandomAnyShort() & 0xfff) > 0xf80)
        gamep_scene_effect_change = -gamep_scene_effect_change;
    gamep_scene_effect_intensity += gamep_scene_effect_change;
    if (gamep_scene_effect_intensity < 250)
        gamep_scene_effect_intensity = 250;
    else if (gamep_scene_effect_intensity > 2000)
        gamep_scene_effect_intensity = 2000;
#endif
    switch (gamep_scene_effect_type)
    {
    case ScEff_RAIN:
        water_droplets_on_floor();
        break;
    case ScEff_SNOW:
        scene_post_effect_texture_with_snow();
        break;
    }
}

static void draw_droplet(TbPixel *o, short scanln, short w, short h, TbPixel *coltb)
{
    short x, y;

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++) {
            o[y * scanln + x] = coltb[o[y * scanln + x]];
        }
    }
}

void draw_falling_rain(int bckt)
{
    ulong icol;
    short limit_y;
    ushort rnd, m;
    ulong shift_y;
    TbPixel *o;
    TbPixel *coltb;
    ulong seed_bkp;
    short x, y;
    short scanln, w, h;

    seed_bkp = lbSeed;
    scanln = lbDisplay.GraphicsScreenWidth;

    icol = (BUCKETS_COUNT - bckt) / 416 << 7;
    shift_y = render_anim_turn * (BUCKETS_COUNT - bckt);
    limit_y = 236 - (bckt >> 5);
    if (limit_y < 20)
        return;

    m = lbDisplay.GraphicsScreenHeight / 200;
    if (m == 0) m++;

    lbSeed = bckt;
    rnd = LbRandomPosShort();
    x = (rnd + (engn_xc >> 4) + (engn_anglexz >> 7)) % scanln;
    rnd = LbRandomPosShort();
    y = m * ((rnd + (shift_y >> 10)) % limit_y);
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    o = &lbDisplay.WScreen[scanln * y + x];
    w = m;
    h = m;
    if (bckt < 4000) h += m;
    if (bckt < 3000) h += m;
    if (bckt < 1000) h += m;
    coltb = &pixmap.ghost_table[256 * pixmap.fade_table[15*PALETTE_8b_COLORS + 63 + icol]];
    draw_droplet(o, scanln, w, h, coltb);

    lbDisplay.DrawFlags = 0;
    lbSeed = seed_bkp;
}

static void draw_static_dot(short x, short y, short w, short h, short ftpos)
{
    short dx, dy;

    for (dy = 0; dy < h; dy++)
    {
        for (dx = 0; dx < w; dx++) {
            LbDrawPixelClip(x + dx, y + dy, pixmap.fade_table[20*PALETTE_8b_COLORS + ftpos]);
        }
    }
}

void draw_falling_snow(int bckt)
{
#if 0
    asm volatile (
      "call ASM_draw_falling_snow\n"
        : : "a" (a1));
#endif
    int height;
    uint seed_bkp;
    ushort m, scanln;

    scanln = lbDisplay.GraphicsScreenWidth;
    m = lbDisplay.GraphicsScreenHeight / 200;
    if (m == 0) m++;
    height = 240 - (bckt >> 5);
    seed_bkp = lbSeed;
    if (height >= 20)
    {
        int shift1;
        uint shift2;
        uint x, y;
        ushort speed;
        ushort angXZs, angXZc;

        angXZs = ((engn_anglexz >> 5)) & 0x7FF;
        angXZc = ((engn_anglexz >> 5) + LbFPMath_PI/2) & 0x7FF;

        lbSeed = bckt;
        speed = (bckt >> 5) & 0x3;
        shift1 = waft_table[(bckt + render_anim_turn) & 0x1F];
        // Moving with full background speed would be >> 19, dividing by half to make rotation look beter
        x = (shift1 >> (speed + 1)) + ((engn_zc * lbSinTable[angXZs]) >> 20) - ((engn_xc * lbSinTable[angXZc]) >> 20) + LbRandomAnyShort();
        shift2 = (BUCKETS_COUNT - bckt) * render_anim_turn;
        y = (shift2 >> (12 - speed/2)) + ((engn_xc * lbSinTable[angXZs]) >> 20) + ((engn_zc * lbSinTable[angXZc]) >> 20) + LbRandomAnyShort();
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_static_dot((x * m) % scanln, (y % height) * m, m, m, 128 * ((BUCKETS_COUNT - bckt) / 416) + colour_lookup[ColLU_WHITE]);
        lbSeed = seed_bkp;
        lbDisplay.DrawFlags = 0;
    }
}

void scene_post_effect_for_bucket(short bckt)
{
    uint every;

    switch (gamep_scene_effect_type)
    {
    case ScEff_RAIN:
        every = 8 * 1000 / gamep_scene_effect_intensity;
        if ((bckt % every) == 0) {
            draw_falling_rain(bckt);
        }
        break;
    case ScEff_SNOW:
        every = 8 * 1000 / gamep_scene_effect_intensity;
        if ((bckt % every) == 0) {
            draw_falling_snow(bckt);
        }
        break;
    default:
        break;
    }
}

static void draw_distant_stars(short x, short y, short w, short h, TbPixel color)
{
    short dx, dy;

    for (dy = 0; dy < h; dy++)
    {
        for (dx = 0; dx < w; dx++) {
            LbDrawPixelClip(x + dx, y + dy, color);
        }
    }
}

void draw_background_stars(void)
{
#if 0
    asm volatile ("call ASM_draw_background_stars\n"
        :  :  : "eax" );
#endif
    ulong seed_bkp;
    int i, limit;
    int scr_x0, scr_y0;
    ushort m;

    m = lbDisplay.GraphicsScreenHeight / 300;
    if (m == 0) m++;
    scr_x0 = lbDisplay.GraphicsScreenWidth / 2;
    scr_y0 = lbDisplay.GraphicsScreenHeight / 2;

    seed_bkp = lbSeed;
    lbSeed = 0x8D747;
    limit = 1223 * gamep_scene_effect_intensity / 1000;
    for (i = 0; i < limit; i++)
    {
        int gt, plane, speed;
        int simp_x, simp_y;
        int scr_x, scr_y;

        gt = render_anim_turn & 0x7FF;
        plane = (i >> 4) + 1;
#if 0 // some testing code which remained for no reason, remove later
        if (lbShift == KMod_SHIFT)
            speed = 32 * gt;
        else
#endif
        speed = plane * gt;
        simp_x = (LbRandomAnyShort() - (speed >> 6)) % 800 - 400;
        simp_y = LbRandomAnyShort() % 800 - 400;

        scr_x = scr_x0 + ((simp_x * m * dword_176D14 - simp_y * m * dword_176D10) >> 16);
        scr_y = scr_y0 - ((simp_x * m * dword_176D10 + simp_y * m * dword_176D14) >> 16);

        draw_distant_stars(scr_x, scr_y, m, m, 79 - (plane >> 1));
    }
    lbSeed = seed_bkp;
}

/******************************************************************************/
