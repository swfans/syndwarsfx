/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstx_spr.c
 *     Drawlists execution for the 3D engine, sprite and 2D shapes related items.
 * @par Purpose:
 *     Implements functions for executing previously made drawlists,
 *     meaning the actual drawing based on primitives in the list.
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
#include "engindrwlstx.h"

#include <assert.h>

#include "bfbox.h"
#include "bfgentab.h"
#include "bfline.h"
#include "bfsprite.h"
#include "insspr.h"

#include "drawtext.h"
#include "engincolour.h"
#include "engindrwlstm.h"
#include "enginshrapn.h"
#include "enginzoom.h"
#include "frame_sprani.h"
#include "privrdlog.h"
/******************************************************************************/
extern long dword_176CE0;
extern long dword_176CE4;
extern long dword_176CE8;
extern long dword_176CEC;
extern long dword_176CF0;
extern long dword_176CF4;
extern long dword_176D00;
extern long dword_176D04;

extern short word_1A5834;
extern short word_1A5836;

extern long sprite_over_16x16;

extern long unkn1_pos_x;
extern long unkn1_pos_y;
extern struct TbSprite *unkn1_spr;

extern struct TbSprite *m_sprites;
extern struct TbSprite *m_sprites_end;

ScreenSortSpriteRenderCallback screen_sorted_sprite_render_cb = NULL;

/******************************************************************************/

void draw_sort_line(struct SortLine *p_sline)
{
#if 0
    asm volatile (
      "call ASM_draw_sort_line\n"
        : : "a" (p_sline));
    return;
#endif
    ushort ftcor;
    if ((p_sline->Flags & (0x01|0x02)) != 0)
    {
        int dist_x, dist_y;
        int dtX, dtY;

        dist_x = p_sline->X1 - p_sline->X2;
        dist_y = p_sline->Y1 - p_sline->Y2;
        if (dist_x < 0)
            dist_x = -dist_x;
        if (dist_y < 0)
            dist_y = -dist_y;
        if (dist_x <= dist_y) {
          dtX = 1;
          dtY = 0;
        } else {
          dtX = 0;
          dtY = 1;
        }

        if ((p_sline->Flags & 0x02) != 0)
            lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        ftcor = 256 * p_sline->Shade + p_sline->Col;
        LbDrawLine(p_sline->X1, p_sline->Y1,
          p_sline->X2, p_sline->Y2,
          pixmap.fade_table[ftcor]);

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        ftcor = 256 * 20 + p_sline->Col;
        LbDrawLine(p_sline->X1 + dtX, p_sline->Y1 + dtY,
          p_sline->X2 + dtX, p_sline->Y2 + dtY,
          pixmap.fade_table[ftcor]);

        ftcor = 256 * 20 + p_sline->Col;
        LbDrawLine(p_sline->X1 - dtX, p_sline->Y1 - dtY,
          p_sline->X2 - dtX, p_sline->Y2 - dtY,
          pixmap.fade_table[ftcor]);
        lbDisplay.DrawFlags = 0;
    }
    else
    {
        ftcor = 256 * p_sline->Shade + p_sline->Col;
        LbDrawLine(p_sline->X1, p_sline->Y1,
          p_sline->X2, p_sline->Y2,
          pixmap.fade_table[ftcor]);
    }
}

void draw_sort_line1a(ushort sln)
{
    struct SortLine *p_sline;
    p_sline = &game_sort_lines[sln];
    draw_sort_line(p_sline);
}

void draw_unkn1_scaled_alpha_sprite(ushort frm, int scr_x, int scr_y, ushort scale, ushort alpha)
{
    struct Frame *p_frm;
    struct Element *p_el;
    int pos_x, pos_y;
    int swidth, sheight;

    p_frm = &frame[frm];
    lbSpriteReMapPtr = &pixmap.fade_table[256 * alpha];
    //TODO would probably make more sense to set the ghost ptr somewhere during game setup
    render_ghost = &pixmap.ghost_table[0*PALETTE_8b_COLORS];

    pos_x = 99999;
    pos_y = 99999;
    for (p_el = &melement_ani[p_frm->FirstElement]; p_el > melement_ani; p_el = &melement_ani[p_el->Next])
    {
        if (pos_x > p_el->X >> 1)
            pos_x = p_el->X >> 1;
        if (pos_y > p_el->Y >> 1)
            pos_y = p_el->Y >> 1;
    }

    swidth = p_frm->SWidth;
    sheight = p_frm->SHeight;
    word_1A5834 = pos_x;
    word_1A5836 = pos_y;
    if ((scale * swidth) >> 9 <= 1)
        return;
    if ((scale * sheight) >> 9 <= 1)
        return;

    dword_176CF0 = (scale * swidth) >> 9;
    dword_176CF4 = (scale * sheight) >> 9;
    dword_176CE0 = scr_x + ((scale * pos_x) >> 8);
    dword_176CE4 = scr_y + ((scale * pos_y) >> 8);
    dword_176CE8 = swidth >> 1;
    dword_176CEC = sheight >> 1;
    SetAlphaScalingData(dword_176CE0, dword_176CE4, dword_176CE8, dword_176CEC, dword_176CF0, dword_176CF4);

    for (p_el = &melement_ani[p_frm->FirstElement]; p_el > melement_ani; p_el = &melement_ani[p_el->Next])
    {
        struct TbSprite *p_spr;

        if ((p_el->Flags & 0xFE00) != 0)
            continue;
        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_el->ToSprite);
        if (p_spr <= m_sprites)
            continue;

        lbDisplay.DrawFlags = p_el->Flags & 0x0F;
        if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) == 0)
            lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;

        unkn1_pos_x = (p_el->X >> 1) - pos_x;
        unkn1_pos_y = (p_el->Y >> 1) - pos_y;
        unkn1_spr = p_spr;
        DrawSpriteWthShadowUsingScalingData(unkn1_pos_x, unkn1_pos_y, p_spr);
    }
    lbDisplay.DrawFlags = 0;
}

void draw_unkn2_scaled_alpha_sprite(ubyte *frv, ushort frm, short x, short y,
  ubyte bri)
{
    struct Frame *p_frm;
    struct Element *p_elem;
    int max_x, max_y;
    int min_x, min_y;
    int range_x, range_y;
    TbBool really_draw;

    really_draw = 0;
    max_x = -99999;
    max_y = -99999;
    min_x = 99999;
    min_y = 99999;
    p_frm = &frame[frm];

    for (p_elem = &melement_ani[p_frm->FirstElement]; p_elem > melement_ani;
      p_elem = &melement_ani[p_elem->Next])
    {
        struct TbSprite *p_spr;

        if (frv[(p_elem->Flags >> 4) & 0x1F] == ((p_elem->Flags >> 9) & 0x07))
        {
            int tmp;

            really_draw = 1;
            p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_elem->ToSprite);
            if (min_x > p_elem->X >> 1)
                min_x = p_elem->X >> 1;
            if (min_y > p_elem->Y >> 1)
                min_y = p_elem->Y >> 1;
            tmp = p_spr->SWidth + (p_elem->X >> 1);
            if (max_x < tmp)
                max_x = tmp;
            tmp = p_spr->SHeight + (p_elem->Y >> 1);
            if (max_y < tmp)
                max_y = tmp;
        }
    }

    if (!really_draw)
        return;

    lbSpriteReMapPtr = &pixmap.fade_table[256 * bri];
    word_1A5834 = min_x;
    word_1A5836 = min_y;

    range_x = max_x - min_x;
    range_y = max_y - min_y;
    if ((range_x > 0) && (range_x <= 128) && (range_y > 0) && (range_y <= 128))
    {
        if ( (overall_scale * p_frm->SWidth) >> 9 > 1 && (overall_scale * p_frm->SHeight) >> 9 > 1 )
        {
            dword_176CE0 = ((min_x * overall_scale) >> 8) + x;
            dword_176CE4 = ((min_y * overall_scale) >> 8) + y;
            dword_176CE8 = range_x;
            dword_176CEC = range_y;
            dword_176CF0 = (range_x * overall_scale) >> 8;
            dword_176CF4 = (range_y * overall_scale) >> 8;
            SetAlphaScalingData(dword_176CE0, dword_176CE4, dword_176CE8,
              dword_176CEC, dword_176CF0, dword_176CF4);

            for (p_elem = &melement_ani[p_frm->FirstElement]; p_elem > melement_ani;
              p_elem = &melement_ani[p_elem->Next])
            {
                struct TbSprite *p_spr;

                p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_elem->ToSprite);
                if (p_spr <= m_sprites)
                    continue;

                lbDisplay.DrawFlags = p_elem->Flags & 7;
                if ((lbDisplay.DrawFlags & Lb_SPRITE_TRANSPAR4) == 0)
                    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;

                if (frv[(p_elem->Flags >> 4) & 0x1F] == ((p_elem->Flags >> 9) & 0x07))
                {
                    unkn1_pos_x = (p_elem->X >> 1) - min_x;
                    unkn1_pos_y = (p_elem->Y >> 1) - min_y;
                    unkn1_spr = p_spr;
                    DrawSpriteWthShadowUsingScalingData(unkn1_pos_x, unkn1_pos_y, unkn1_spr);
                }
            }
            lbDisplay.DrawFlags = 0;
        }
    }
}


void draw_unkn3_scaled_alpha_sprite(ushort frm, short x, short y, ubyte bri)
{
    struct Frame *p_frm;
    struct Element *p_elem;

    p_frm = &frame[frm];
    for (p_elem = &melement_ani[p_frm->FirstElement]; p_elem > melement_ani;
      p_elem = &melement_ani[p_elem->Next])
    {
        struct TbSprite *p_spr;

        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_elem->ToSprite);
        if (p_spr <= m_sprites)
            continue;

        lbDisplay.DrawFlags = p_elem->Flags & 0x07;
        if ((p_elem->Flags & 0xFE00) == 0) {
            LbSpriteDrawRemap((p_elem->X >> 1) + x, (p_elem->Y >> 1) + y,
              p_spr, &pixmap.fade_table[256 * bri]);
        }
        if (word_1A5834 > p_elem->X >> 1)
            word_1A5834 = p_elem->X >> 1;
        if (word_1A5836 > p_elem->Y >> 1)
            word_1A5836 = p_elem->Y >> 1;
    }
    lbDisplay.DrawFlags = 0;
}

void draw_unkn4_scaled_alpha_sprite(ubyte *frv, ushort frm, short x, short y,
  ubyte bri)
{
    struct Frame *p_frm;
    struct Element *p_elem;

    p_frm = &frame[frm];
    for (p_elem = &melement_ani[p_frm->FirstElement]; p_elem > melement_ani;
      p_elem = &melement_ani[p_elem->Next])
    {
        struct TbSprite *p_spr;

        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_elem->ToSprite);
        if (p_spr <= m_sprites)
            continue;

        lbDisplay.DrawFlags = p_elem->Flags & 7;
        if (frv[(p_elem->Flags >> 4) & 0x1F] == ((p_elem->Flags >> 9) & 0x07))
        {
            if (((p_elem->Flags >> 4) & 0x1F) == 4)
                LbSpriteDraw((p_elem->X >> 1) + x, (p_elem->Y >> 1) + y, p_spr);
            else
                LbSpriteDrawRemap((p_elem->X >> 1) + x, (p_elem->Y >> 1) + y,
                  p_spr, &pixmap.fade_table[256 * bri]);
            if (word_1A5834 > p_elem->X >> 1)
                word_1A5834 = p_elem->X >> 1;
            if (word_1A5836 > p_elem->Y >> 1)
                word_1A5836 = p_elem->Y >> 1;
        }
    }
    lbDisplay.DrawFlags = 0;
}

void debug_check_unkn_sprite_size(const char *src_fname, int src_line)
{
    if (!sprite_over_16x16 && (m_sprites[1158].SWidth > 16 || m_sprites[1158].SHeight > 16))
        sprite_over_16x16 = 1;
}

void draw_sorted_sprite1b(ubyte *frv, ushort frm, short x, short y,
  ubyte bri, ubyte angle)
{
#if 0
    debug_check_unkn_sprite_size(__FILE__, __LINE__);
#endif

    if ((frv[4] != 0) && (angle > 1) && (angle < 7))
        bri += 15;
    if (bri < 10)
        bri = 10;
    if (bri > 60)
        bri = 60;

    if ((overall_scale == 256) || (overall_scale <= 0) || (overall_scale >= 4096))
    {
        draw_unkn4_scaled_alpha_sprite(frv, frm, x, y, bri);
    }
    else
    {
        draw_unkn2_scaled_alpha_sprite(frv, frm, x, y, bri);
    }
}

void draw_sorted_sprite1a(ushort frm, short x, short y, ubyte bright)
{
#if 0
    asm volatile (
      "call ASM_draw_sorted_sprite1a\n"
        : : "a" (frm), "d" (x), "b" (y), "c" (bright));
    return;
#endif
    int sscale;
    ubyte bri;

    sscale = overall_scale;
    bri = bright;
    if (bri < 10)
        bri = 10;
    if (bri > 48)
        bri = 48;

    if ((sscale == 256) || (sscale <= 0) || (sscale >= 4096))
    {
        draw_unkn3_scaled_alpha_sprite(frm, x, y, bri);
    }
    else
    {
        draw_unkn1_scaled_alpha_sprite(frm, x, y, sscale, bri);
    }
}

void draw_sort_sprite1c_sub(ushort frm, short x, short y, ubyte bright, ushort scale)
{
    int sscale;
    ubyte bri;

    sscale = (scale * overall_scale) >> 8;

    bri = bright;
    if (bri < 10)
        bri = 10;
    if (bri > 48)
        bri = 48;

    if (sscale == 256 || sscale == 0 || sscale >= 0x1000)
    {
        draw_unkn3_scaled_alpha_sprite(frm, x, y, bri);
    }
    else
    {
        draw_unkn1_scaled_alpha_sprite(frm, x, y, sscale, bri);
    }
}

void draw_sort_sprite1c(ushort sspr)
{
    struct SortSprite *p_sspr;
    p_sspr = &game_sort_sprites[sspr];
    draw_sort_sprite1c_sub(p_sspr->Frame, p_sspr->X, p_sspr->Y, p_sspr->Brightness, p_sspr->Scale);
}

void draw_sort_sprite1a(ushort sspr)
{
#if 0
    asm volatile (
      "call ASM_draw_sort_sprite1a\n"
        : : "a" (a1));
    return;
#endif
    struct SortSprite *p_sspr;

    p_sspr = &game_sort_sprites[sspr];

    word_1A5834 = 120;
    word_1A5836 = 120;
    draw_sorted_sprite1a(p_sspr->Frame, p_sspr->X, p_sspr->Y, p_sspr->Brightness);
    screen_sorted_sprite_render_cb(sspr);
}

/**
 * Draw smoke cloud sprite.
 *
 * @param ph Index of Phwoar instance.
 */
void draw_phwoar(ushort ph)
{
    struct Phwoar *p_phwoar;
    struct Element *p_elem;
    ushort el;
    int point_x, point_y;

    p_phwoar = &phwoar[ph];
    {
        struct SpecialPoint *p_scrpoint;

        p_scrpoint = &game_screen_point_pool[p_phwoar->PointOffset];
        point_x = p_scrpoint->X + dword_176D00;
        point_y = p_scrpoint->Y + dword_176D04;
    }

    el = frame[p_phwoar->f].FirstElement;
    for (p_elem = &melement_ani[el]; p_elem > melement_ani; p_elem = &melement_ani[el])
    {
        struct TbSprite *p_spr;

        el = p_elem->Next;
        p_spr = (struct TbSprite *)((ubyte *)m_sprites + p_elem->ToSprite);
        if ((p_spr <= m_sprites) || (p_spr >= m_sprites_end))
            continue;

        lbDisplay.DrawFlags = p_elem->Flags & 0x07;
        if ((p_elem->Flags & 0xFE00) == 0)
        {
            int el_x, el_y;
            el_x = point_x + ((p_elem->X * overall_scale) >> 9);
            el_y = point_y + ((p_elem->Y * overall_scale) >> 9);
            LbSpriteDrawResized(el_x, el_y, (16 * overall_scale) >> 8, p_spr);
        }
        if (word_1A5834 > p_elem->X >> 1)
            word_1A5834 = p_elem->X >> 1;
        if (word_1A5836 > p_elem->Y >> 1)
            word_1A5836 = p_elem->Y >> 1;
    }
    lbDisplay.DrawFlags = 0;
}

void draw_fire_flame(ushort flm)
{
    struct FireFlame *p_flame;
    struct SpecialPoint *p_scrpoint;

    p_flame = &FIRE_flame[flm];
    if (p_flame->big != 0)
    {
        p_scrpoint = &game_screen_point_pool[p_flame->PointOffset];
        draw_unkn1_scaled_alpha_sprite(p_flame->frame, p_scrpoint->X + dword_176D00,
          p_scrpoint->Y + dword_176D04, (overall_scale * (p_flame->big + 128)) >> 7, 0x20);
    }
    else
    {
        p_scrpoint = &game_screen_point_pool[p_flame->PointOffset];
        draw_sorted_sprite1a(p_flame->frame, p_scrpoint->X + dword_176D00,
          p_scrpoint->Y + dword_176D04, 0x20);
    }
}

/**
 * Draw a bar showing some kind of level parameter, usually health.
 *
 * @param sspr Index of SortSprite instance which stores the draw parameters.
 */
void draw_sort_sprite_level_bar(short sspr, ushort w, ushort h, short lvl, ushort max_lvl, TbPixel lvl_col, TbPixel bar_col)
{
    struct SortSprite *p_sspr;
    ushort range_w, range_h, bar_w, bar_h;
    short level_x;

    p_sspr = &game_sort_sprites[sspr];

    if (max_lvl == 0)
        max_lvl = 1;
    if (lvl < 0)
        lvl = 0;
    else if (lvl > max_lvl)
        lvl = max_lvl + 1;

    bar_w = ((w) * overall_scale) >> 8;
    bar_h = ((h) * overall_scale) >> 8;

    range_w = ((w * 15 / 16) * overall_scale) >> 8;
    range_h = ((h * 12 / 16) * overall_scale) >> 8;

    // The bar looks better if marins on opposite sides are the same,
    // that is both dimensions are odd or both are even
    if ((bar_w & 1) != 0)
        range_w |= 1;
    else
        range_w &= ~1;
    if ((bar_h & 1) != 0)
        range_h |= 1;
    else
        range_h &= ~1;

    LbDrawBox(p_sspr->X - bar_w / 2, p_sspr->Y, bar_w, bar_h, bar_col);

    level_x = range_w * lvl / max_lvl;
    LbDrawBox(p_sspr->X - bar_w / 2, p_sspr->Y + (bar_h - range_h) / 2, level_x, range_h, lvl_col);
}

void draw_sort_sprite_number(ushort sspr)
{
    char locstr[50];
    struct SortSprite *p_sspr;

    p_sspr = &game_sort_sprites[sspr];
    sprintf(locstr, "%d", (int)p_sspr->PThing);
    draw_text(p_sspr->X, p_sspr->Y, locstr, colour_lookup[ColLU_RED]);
}

/******************************************************************************/
