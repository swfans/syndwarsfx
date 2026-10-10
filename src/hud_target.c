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
#include <stdlib.h>

#include "engincam.h"
#include "engincolour.h"
#include "enginsngobjs.h"
#include "engintrns.h"
#include "huddrwlstm.h"

#include "bigmap.h"
#include "engindrwlstm_wrp.h"
#include "engindrwlstx.h"
#include "frame_sprani.h"
#include "game_sprts.h"
#include "game.h"
#include "mouse.h"
#include "player.h"
#include "sound.h"
#include "thing.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/

TbBool hud_show_target_health = false;

s32 target_old_frameno = 0;

short goto_point_frame_no = 0;
short goto_point_frame_count = 0;

/******************************************************************************/

void show_goto_point(u32 flag)
{
    ushort frame_count;
    short frm;
    struct Thing *p_thing;
    short face;
    ThingIdx dcthing;

    if (flag & 0xff)
    {
        goto_point_frame_count = 0;
        goto_point_frame_no = nstart_ani[926];
        return;
    }
    frm = goto_point_frame_no;
    if (frm == 0) {
        return;
    }
    frame_count = goto_point_frame_count++;
    if (frame_count > 5)
        goto_point_frame_no = 0;
    else
        goto_point_frame_no = frame[goto_point_frame_no].Next;

    dcthing = players[local_player_no].DirectControl[mouser];
    p_thing = &things[dcthing];
    if ((((p_thing->Flag & TngF_InVehicle) != 0) || (p_thing->State == PerSt_GOTO_POINT)) &&
      ((p_thing->Flag2 & TgF2_Unkn0040) == 0))
    {
        int height;
        int cor_x, cor_y, cor_z;
        TbPixel colour;

        if (goto_point_frame_count == 1)
            play_sample_using_heap(0, 92, 127, 64, 100, 0, 3u);

        colour = 0;
        cor_x = p_thing->U.UPerson.GotoX;
        cor_z = p_thing->U.UPerson.GotoZ;
        face = players[local_player_no].GotoFace;
        if (face != 0)
        {
            int prc_x, prc_z;

            prc_x = MAPCOORD_TO_PRCCOORD(cor_x,0);
            prc_z = MAPCOORD_TO_PRCCOORD(cor_z,0);
            if (face <= 0)
                height = get_height_on_face_quad(prc_x, prc_z, -face);
            else
                height = get_height_on_face(prc_x, prc_z, face);
        }
        else
        {
            height = alt_at_point(cor_x, cor_z);
        }

        cor_y = PRCCOORD_TO_MAPCOORD(height);
        if ((p_thing->Flag2 & TgF2_Unkn00080000) != 0)
            colour = 48;
        enlist_hud_draw_mapcoord_frame_one_colour(cor_x, cor_y, cor_z, frm, 0, colour);
    }
}

void draw_hud_target_mouse(ThingIdx dcthing)
{
    PlayerInfo *p_locplayer;
    struct Thing *p_dcthing;

    p_dcthing = &things[dcthing];
    p_locplayer = &players[local_player_no];
    if (p_locplayer->Target > 0)
    {
        struct Thing *p_targtng;
        int weprange;
        ushort msspr;
        uint range;

        weprange = current_hand_weapon_range(p_dcthing);
        switch (p_locplayer->TargetType)
        {
        case TrgTp_Unkn1:
        case TrgTp_Unkn2:
        case TrgTp_Unkn6:
        case TrgTp_Unkn7:
            p_locplayer->field_102 = p_locplayer->Target;
            p_locplayer->TargetType = TrgTp_Unkn7;
            p_targtng = &things[p_locplayer->Target];
            range = weprange * weprange;
            if (can_i_see_thing(p_dcthing, p_targtng, range, 3) ) {
                msspr = 3;
            } else {
                msspr = 2;
            }
            do_change_mouse(msspr);
            break;
        case TrgTp_Unkn3:
            p_locplayer->field_102 = p_locplayer->Target;
            do_change_mouse(7);
            break;
        case TrgTp_Unkn4:
            p_locplayer->field_102 = p_locplayer->Target;
            p_targtng = &things[p_locplayer->field_102];
            p_dcthing = &things[p_locplayer->DirectControl[mouser]];
            if (can_i_enter_vehicle(p_dcthing, p_targtng)) {
              msspr = 6;
            } else {
              range = p_targtng->Radius * p_targtng->Radius + weprange * weprange;
              if (can_i_see_thing(p_dcthing, p_targtng, range, 3) ) {
                msspr = 3;
              } else {
                msspr = 2;
              }
            }
            do_change_mouse(msspr);
            break;
        default:
            break;
        }
    }
    else if (p_locplayer->Target < 0)
    {
        if (p_locplayer->TargetType == TrgTp_Unkn3) {
          p_locplayer->field_102 = p_locplayer->Target;
          do_change_mouse(7);
        } else {
          p_locplayer->field_102 = p_locplayer->Target;
          do_change_mouse(5);
        }
    }
    else
    {
        do_change_mouse(8);
    }
}

void init_draw_target(void)
{
    if (target_old_frameno == 0)
        target_old_frameno = nstart_ani[983];
}

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
    h_total = -(15 * overall_scale) >> 8;
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

void draw_hud_target_old_frame(struct Thing *p_target, int frm)
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
        draw_frame_unscaled(ep.pp.X - sh_x, ep.pp.Y, frm +  0);
        draw_frame_unscaled(ep.pp.X + sh_x, ep.pp.Y, frm + 10);
    }
    else
    {
        int sh_x;
        sh_x = (12 * overall_scale) >> 9;
        draw_frame_scaled_alpha(ep.pp.X - sh_x, ep.pp.Y, frm +  0, overall_scale, PALETTE_FADE_LEVELS / 2);
        draw_frame_scaled_alpha(ep.pp.X + sh_x, ep.pp.Y, frm + 10, overall_scale, PALETTE_FADE_LEVELS / 2);
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
