/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file hud_target.c
 *     Ingame Heads-Up Display targetted thing draw.
 * @par Purpose:
 *     Implement functions drawing the HUD elements around target to be
 *     attacked.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "hud_target.h"

#include "bfbox.h"
#include "bfgentab.h"
#include "bfsprite.h"
#include "insspr.h"

#include "engincam.h"
#include "engincolour.h"
#include "engintrns.h"

#include "bigmap.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "frame_sprani.h"
#include "game_sprts.h"
#include "game.h"
#include "thing.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/

TbBool hud_show_target_health = false;

void draw_hud_lock_target(void)
{
    asm volatile ("call ASM_draw_hud_lock_target\n"
        :  :  : "eax" );
}

void draw_target_person(struct Thing *p_person, uint radius)
{
#if 0
    asm volatile ("call ASM_draw_target_person\n"
        : : "a" (p_person), "d" (radius));
    return;
#endif
    struct EnginePoint ep;
    struct TbSprite *p_bspr;
    struct TbSprite *p_aspr;

    if ((p_person->Flag & TngF_Destroyed) != 0)
        return;

    ep.X3d = PRCCOORD_TO_MAPCOORD(p_person->X) - engn_xc;
    ep.Z3d = PRCCOORD_TO_MAPCOORD(p_person->Z) - engn_zc;
    // TODO Why constant height of 120? Maybe differnt main body position for different thing types?
    ep.Y3d = PRCCOORD_TO_YCOORD(p_person->Y) - engn_yc + 120;
    ep.Flags = 0;
    transform_point(&ep);

    p_aspr = &pop1_sprites[84];
    p_bspr = &pop1_sprites[78];
    LbSpriteDraw(ep.pp.X - radius - p_aspr->SWidth, ep.pp.Y - radius - p_aspr->SHeight, p_bspr);
    p_bspr = &pop1_sprites[79];
    LbSpriteDraw(ep.pp.X + radius, ep.pp.Y - radius - p_aspr->SHeight, p_bspr);
    p_bspr = &pop1_sprites[81];
    LbSpriteDraw(ep.pp.X + radius, ep.pp.Y + radius, p_bspr);
    p_aspr = &pop1_sprites[87];
    p_bspr = &pop1_sprites[80];
    LbSpriteDraw(ep.pp.X - radius - p_aspr->SWidth, ep.pp.Y + radius, p_bspr);
}

void draw_target_vehicle(struct Thing *p_vehicle)
{
#if 0
    asm volatile ("call ASM_draw_target_vehicle\n"
        : : "a" (p_vehicle));
    return;
#endif
    struct ShEnginePoint sp;
    struct TbSprite *p_bspr;
    struct TbSprite *p_aspr;
    int cor_x, cor_y, cor_z;
    int scr_x, scr_y, r;

    cor_x = (p_vehicle->X >> 8) - engn_xc;
    cor_z = (p_vehicle->Z >> 8) - engn_zc;
    cor_y = 8 * (p_vehicle->Y >> 8) - engn_yc;

    {
        int cor_lr, cor_sm;
        if (abs(cor_x) <= abs(cor_z)) {
            cor_sm = abs(cor_x);
            cor_lr = abs(cor_z);
        } else {
            cor_sm = abs(cor_z);
            cor_lr = abs(cor_x);
        }
        if (cor_lr + (cor_sm >> 1) > TILE_TO_MAPCOORD(20,0))
            return;
    }

    transform_shpoint(&sp, cor_x, cor_y - 8 * engn_yc, cor_z);

    r = p_vehicle->Radius >> 4;

    p_aspr = &pop1_sprites[84];
    p_bspr = &pop1_sprites[85];

    scr_x = sp.X - r - p_aspr->SWidth;
    scr_y = sp.Y - r - p_aspr->SHeight;
    LbSpriteDraw(scr_x, scr_y, p_aspr);

    scr_x = sp.X + r;
    scr_y = sp.Y - r - p_aspr->SHeight;
    LbSpriteDraw(scr_x, scr_y, p_bspr);

    p_aspr = &pop1_sprites[87];
    p_bspr = &pop1_sprites[86];

    scr_x = sp.X + r;
    scr_y = sp.Y + r;
    LbSpriteDraw(scr_x, scr_y, p_aspr);

    scr_x = sp.X - r - p_aspr->SWidth;
    scr_y = sp.Y + r;
    LbSpriteDraw(scr_x, scr_y, p_bspr);
}

void draw_hud_health_bar(int x, int y, struct Thing *p_thing)
{
    int dx, dy;
    int hp_per_px, val_cur;
    int h_total, h_cur, h_ext, w;
    TbPixel colour;

    dx = (9 * overall_scale) >> 8;
    dy = (10 * overall_scale) >> 8;
    h_total = -15 * (overall_scale) >> 8;
    w = (2 * overall_scale) >> 8;

    hp_per_px = p_thing->U.UPerson.MaxHealth / dy;
    if (hp_per_px == 0)
        hp_per_px = 1;

    if (p_thing->Health <= p_thing->U.UPerson.MaxHealth)
        val_cur = p_thing->Health;
    else
        val_cur = p_thing->U.UPerson.MaxHealth;

    h_cur = val_cur / hp_per_px;
    if (h_cur > dy)
      h_cur = dy;

    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    colour = colour_lookup[ColLU_BLUE];
    LbDrawBox(dx + x, y + h_total, w, dy, colour);

    if (p_thing->Health >= 0)
    {
        lbDisplay.DrawFlags = 0;
        if (h_cur < 3)
            colour = colour_lookup[ColLU_RED];
        LbDrawBox(x + dx, y + dy + h_total - h_cur, w, h_cur, colour);

        // Extra health if the thing is above max (soul gun bonus)
        if (p_thing->Health > p_thing->U.UPerson.MaxHealth)
        {
            h_ext = (p_thing->Health - p_thing->U.UPerson.MaxHealth) / hp_per_px;
            if (h_ext > dy)
                h_ext = dy;

            lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            colour = colour_lookup[ColLU_YELLOW];
            LbDrawBox(dx + x, y + h_total, w, dy, colour);
            lbDisplay.DrawFlags = 0;
            LbDrawBox(x + dx, y + dy + h_total - h_ext, w, h_ext, colour);
        }
    }
}

void draw_hud_shield_bar(int x, int y, struct Thing *p_thing)
{
    int dx, dy;
    int sp_per_px;
    int h_total, h_cur, w;
    TbPixel colour;

    dx = (15 * overall_scale) >> 8;
    dy = (10 * overall_scale) >> 8;
    h_total = -(20 * overall_scale) >> 8;
    w = (2 * overall_scale) >> 8;

    sp_per_px = p_thing->U.UPerson.MaxShieldEnergy / dy;
    if (sp_per_px == 0)
        sp_per_px = 1;

    h_cur = p_thing->U.UPerson.ShieldEnergy / sp_per_px;
    if (h_cur > dy)
      h_cur = dy;

    colour = colour_lookup[ColLU_BLUE];
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbDrawBox(dx + x, h_total + y, w, dy, colour);
    if (p_thing->U.UPerson.ShieldEnergy > 0)
    {
        lbDisplay.DrawFlags = 0;
        if (h_cur < 3)
            colour = colour_lookup[ColLU_RED];
        LbDrawBox(x + dx, y + dy + h_total - h_cur, w, h_cur, colour);
    }
}

void draw_unkn1_standard_sprite(ushort fr, int scr_x, int scr_y)
{
    struct Frame *p_frm;
    struct Element *p_el;
    int el;

    p_frm = &frame[fr];
    for (el = p_frm->FirstElement; ; el = p_el->Next)
    {
        struct TbSprite *spr;
        int sscr_x, sscr_y;

        p_el = &melement_ani[el];
        if (p_el <= melement_ani)
            break;
        if ((p_el->Flags & 0xFE00) != 0)
            continue;
        spr = (struct TbSprite *)((ubyte *)m_sprites + p_el->ToSprite);
        if (spr <= m_sprites)
            continue;

        lbDisplay.DrawFlags = p_el->Flags & 0x07;
        sscr_y = scr_y + (p_el->Y >> 1);
        sscr_x = scr_x + (p_el->X >> 1);
        LbSpriteDraw(sscr_x, sscr_y, spr);
    }
    lbDisplay.DrawFlags = 0;
}

void draw_hud_target_old_frame(struct Thing *p_target, int fr)
{
    struct EnginePoint ep;

    ep.X3d = PRCCOORD_TO_MAPCOORD(p_target->X) - engn_xc;
    ep.Z3d = PRCCOORD_TO_MAPCOORD(p_target->Z) - engn_zc;
    ep.Y3d = PRCCOORD_TO_YCOORD(p_target->Y) - engn_yc;
    ep.Flags = 0;
    transform_point(&ep);

    if ((overall_scale == 256) || (overall_scale <= 0) || (overall_scale >= 4096))
    {
        int sh_x;
        sh_x = (12 * overall_scale) >> 9;
        draw_unkn1_standard_sprite(fr +  0, ep.pp.X - sh_x, ep.pp.Y);
        draw_unkn1_standard_sprite(fr + 10, ep.pp.X + sh_x, ep.pp.Y);
    }
    else
    {
        int sh_x;
        sh_x = (12 * overall_scale) >> 9;
        draw_unkn1_scaled_alpha_sprite(fr +  0, ep.pp.X - sh_x, ep.pp.Y, overall_scale, PALETTE_FADE_LEVELS / 2);
        draw_unkn1_scaled_alpha_sprite(fr + 10, ep.pp.X + sh_x, ep.pp.Y, overall_scale, PALETTE_FADE_LEVELS / 2);
    }

    draw_hud_health_bar(ep.pp.X, ep.pp.Y, p_target);
    draw_hud_shield_bar(ep.pp.X, ep.pp.Y, p_target);
}

void draw_hud_target2(short dcthing, short target)
{
    struct Thing *p_dcthing;
    struct Thing *p_target;

    p_dcthing = &things[dcthing];
    p_target = &things[target];

    if (current_weapon_has_targetting(p_dcthing))
    {
        struct Thing *p_dctarget;
        p_dctarget = p_dcthing->PTarget;
        if ((p_dctarget != NULL) && ((p_dctarget->Flag & TngF_Destroyed) == 0))
        {
            int sz;

            sz = 4 * (18 - p_dcthing->U.UPerson.WeaponTimer);
            if (sz < 6)
                sz = 6;
            draw_target_person(p_dctarget, sz);
        }
    }
    else
    {
        struct Thing *p_dctarget;
        switch (p_target->Type)
        {
        case TT_VEHICLE:
            draw_target_vehicle(p_target);
            break;
        case TT_PERSON:
            if ((p_target->Flag & TngF_InVehicle) != 0) {
                p_dctarget = &things[p_target->U.UPerson.Vehicle];
            } else {
                p_dctarget = p_target;
            }
            draw_target_person(p_dctarget, 2);
            break;
        }
    }
    // Vehicles have their own health drawing method
    if ((p_target->Type != TT_VEHICLE) && hud_show_target_health)
    {
        draw_hud_target_old_frame(p_target, target_old_frameno);
    }
    target_old_frameno = frame[target_old_frameno].Next;
}

/******************************************************************************/
