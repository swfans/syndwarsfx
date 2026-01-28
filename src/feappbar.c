/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feappbar.c
 *     Front-end desktop and menu system, application bar at the bottom.
 * @par Purpose:
 *     Implement functions for Application Bar in front-end desktop.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Apr 2024 - 22 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "feappbar.h"

#include <assert.h>
#include "bfkeybd.h"
#include "bfsprite.h"
#include "bftext.h"
#include "bfutility.h"
#include "ssampply.h"
#include "bfjoyst.h"

#include "campaign.h"
#include "display.h"
#include "febrief.h"
#include "fecntrls.h"
#include "feequip.h"
#include "femail.h"
#include "femain.h"
#include "game_options.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "guiboxes.h"
#include "guitext.h"
#include "keyboard.h"
#include "packetfe.h"
#include "player.h"
#include "purpldrw.h"
#include "research.h"
#include "sound.h"
#include "swlog.h"
#include "weapon.h"
/******************************************************************************/

#define PURPLE_APPS_EMAIL_ICONS_LIMIT 10

extern ubyte byte_155124[];
extern ubyte byte_15512C[];

extern ubyte byte_1C497E;
extern ubyte byte_1C497F;
extern ubyte byte_1C4980;
extern ubyte byte_1C4984[];
extern short word_1C498A;
extern ubyte byte_1C498C;

extern short word_1C6F3E;
extern short word_1C6F40;

/******************************************************************************/

void draw_app_icon_hilight(short x, short y, ubyte iconid, ubyte aframe)
{
    struct TbSprite *spr;

    lbDisplay.DrawFlags |= 0x8000;
    spr = &fe_icons_sprites[aframe + byte_155124[iconid] + byte_15512C[iconid]];
    draw_sprite_purple_list(x, y, spr);
    lbDisplay.DrawFlags = 0;
    spr = &fe_icons_sprites[aframe + byte_155124[iconid]];
    draw_sprite_purple_list(x, y, spr);
    lbDisplay.DrawFlags = 0;
}

void draw_app_icon_normal(short x, short y, ubyte iconid, ubyte aframe)
{
    struct TbSprite *spr;

    lbDisplay.DrawFlags |= 0x8000;
    spr = &fe_icons_sprites[aframe + byte_155124[iconid] + byte_15512C[iconid]];
    draw_sprite_purple_list(x, y, spr);
    lbDisplay.DrawFlags = 0;
}

void draw_unread_email_icon(short x, short y, ubyte aframe)
{
    struct TbSprite *spr;

    lbDisplay.DrawFlags |= 0x8000;
    switch (aframe)
    {
    case 1:
        spr = &fe_icons_sprites[79];
        draw_sprite_purple_list(x, y, spr);
        break;
    case 2:
        play_sample_using_heap(0, 112, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        // fall through
    case 3:
    case 4:
    case 5:
        spr = &fe_icons_sprites[77 + aframe];
        draw_sprite_purple_list(x, y, spr);
        lbDisplay.DrawFlags = 0;
        spr = &fe_icons_sprites[96 + aframe];
        draw_sprite_purple_list(x, y, spr);
        break;
    case 6:
        spr = &fe_icons_sprites[82];
        draw_sprite_purple_list(x, y, spr);
        lbDisplay.DrawFlags = 0;
        break;
    default:
        break;
    }
    lbDisplay.DrawFlags = 0;
}

TbBool is_purple_apps_selection_bar_visible(void)
{
    return (screentype != SCRT_MAINMENU) && (screentype != SCRT_LOGIN) && !restore_savegame;
}

TbBool is_purple_apps_utility_space_reserved(short iconid)
{
    return  (iconid != ApBar_PANET);
}

TbBool is_purple_apps_utility_visible(short iconid)
{
    // Show research icon only if the player has research facility
    if ((iconid == ApBar_RESEARCH) && (research.NumBases == 0))
        return false;

    if (iconid == ApBar_NEWMAIL) {
        return (new_mail &&
          (game_system_screen != SySc_NETGAME || screentype != SCRT_SYSMENU));
    }


    if (login_control__State == LognCt_Unkn5)
    {
        TbBool visible;

        if (net_local_player_hosts_the_game()) {
            visible = (iconid != ApBar_PANET && iconid != ApBar_RESEARCH);
        } else {
            visible = (iconid != ApBar_PANET &&
              iconid != ApBar_WORLDMAP &&
              iconid != ApBar_RESEARCH);
        }
        if ((net_game_play_flags & NGPF_Unkn02) == 0 || (net_game_play_flags & NGPF_Unkn01) == 0)
            visible = (iconid == ApBar_SYSTEM);
        return visible;
    }

    // Completely hide Public Access Network icon
    return (iconid != ApBar_PANET);
}

TbBool get_purple_apps_icon_rect(struct ScreenRect *p_rect, short iconid)
{
    struct TbSprite *spr;
    short cicnid;
    short cx, cy;

    p_rect->X = 0;
    p_rect->Y = 0;
    // Show utility icons in bottom left
    cx = global_apps_bar_box.X;
    cy = global_apps_bar_box.Y;

    for (cicnid = ApBar_SYSTEM; cicnid < ApBar_NEWMAIL; cicnid++)
    {
        if (cicnid == iconid)
        {
            p_rect->X = cx;
            p_rect->Y = cy;
            if (!is_purple_apps_utility_visible(cicnid)) {
                p_rect->Width = 0;
                p_rect->Height = 0;
                return false;
            }
            spr = &fe_icons_sprites[byte_155124[cicnid]];
            p_rect->Width = spr->SWidth;
            p_rect->Height = spr->SHeight;
            return true;
        }
        if (is_purple_apps_utility_space_reserved(cicnid))
        {
            spr = &fe_icons_sprites[byte_155124[cicnid]];
            cx += spr->SWidth + 3;
        }
    }

    { // Get size of unread mail notification icon
        if (cicnid == iconid)
        {
            p_rect->X = cx;
            p_rect->Y = cy;
            if (!is_purple_apps_utility_visible(cicnid)) {
                p_rect->Width = 0;
                p_rect->Height = 0;
                return false;
            }
            spr = &fe_icons_sprites[79];
            p_rect->Width = spr->SWidth;
            p_rect->Height = spr->SHeight;
            return true;
        }
    }
    return false;
}

TbBool mouse_over_purple_apps_icon(short iconid)
{
    struct ScreenRect rect;

    if (!get_purple_apps_icon_rect(&rect, iconid))
        return false;

    return mouse_move_over_box(&rect);
}

void draw_purple_app_utility_icon(short iconid)
{
    struct ScreenRect rect;

    if (!get_purple_apps_icon_rect(&rect, iconid))
        return;

    if (mouse_move_over_box(&rect))
    {
        if ((byte_1C497E & (1 << iconid)) == 0) {
            byte_1C497E |= (1 << iconid);
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        }
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        // If clicked, draw the icon without transparency
        if (lbDisplay.MLeftButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.DrawFlags = 0;
        }
        if (lbDisplay.MRightButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.DrawFlags = 0;
        }
        draw_app_icon_hilight(rect.X, rect.Y, iconid, byte_1C4984[iconid]);
        byte_1C4984[iconid]++;
        if (byte_1C4984[iconid] == byte_15512C[iconid])
            byte_1C4984[iconid] = 0;
    }
    else
    {
        byte_1C497E &= ~(1 << iconid);
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_app_icon_normal(rect.X, rect.Y, iconid, byte_1C4984[iconid]);
        if (byte_1C4984[iconid])
        {
            byte_1C4984[iconid]++;
            if (byte_1C4984[iconid] == byte_15512C[iconid])
                byte_1C4984[iconid] = 0;
        }
        if (word_1C498A == 2 * (iconid + 1)
            || word_1C498A == 2 * (iconid + 1) + 1)
            word_1C498A = 0;
    }
    lbDisplay.DrawFlags = 0;
}

TbBool get_purple_app_utility_icon_inputs(short iconid)
{
    struct ScreenRect rect;

    if (!get_purple_apps_icon_rect(&rect, iconid))
        return false;

    if (mouse_move_over_box(&rect))
    {
        if (lbDisplay.MLeftButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.LeftButton = 0;
            word_1C498A = 2 * (iconid + 1);
        }
        else if (word_1C498A == 2 * (iconid + 1) + 0)
        {
            if (dragged_weapon_can_drop_on_research())
            {
                dragged_weapon_drop_on_research();
            }
            else
            {
                change_screen = iconid + 1;
                play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
            }
            word_1C498A = 0;
        }

        if (lbDisplay.MRightButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.RightButton = 0;
            word_1C498A = 2 * (iconid + 1) + 1;
        }
        else if (word_1C498A == 2 * (iconid + 1) + 1)
        {
            change_screen = iconid + 1;
            play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);

            word_1C498A = 0;
        }
    }
    // If not mouse over, make sure to clear the state
    else if (word_1C498A == 2 * (iconid + 1) + 0)
        word_1C498A = 0;
    else if (word_1C498A == 2 * (iconid + 1) + 1)
        word_1C498A = 0;

    return false;
}

void draw_purple_app_unread_email_icon(void)
{
    struct ScreenRect rect;

    if (!get_purple_apps_icon_rect(&rect, ApBar_NEWMAIL))
        return;

    if ((is_key_pressed(KC_RETURN, KMod_DONTCARE)
        && ((game_system_screen != SySc_CONTROLS && game_system_screen != SySc_NETGAME)
            || screentype != SCRT_SYSMENU) && !edit_flag)
        || mouse_move_over_box(&rect))
    {
        if (!byte_1C4980 && !is_key_pressed(KC_RETURN, KMod_DONTCARE))
        {
            byte_1C4980 = 1;
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        }
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        // If clicked, draw the icon without transparency
        if (lbDisplay.MLeftButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.DrawFlags = 0;
        }
        draw_unread_email_icon(rect.X, rect.Y, byte_1C498C);
        if (gameturn & 1)
        {
            if (++byte_1C498C > 5)
                byte_1C498C = 2;
        }
    }
    else
    {
        byte_1C4980 = 0;
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_unread_email_icon(rect.X, rect.Y, byte_1C498C);
        if (gameturn & 1)
        {
            if (++byte_1C498C > 6)
                byte_1C498C = 0;
        }
    }
    lbDisplay.DrawFlags = 0;
}

TbBool get_purple_app_unread_email_icon_inputs(void)
{
    struct ScreenRect rect;
    const char *subtext;

    if (!get_purple_apps_icon_rect(&rect, ApBar_NEWMAIL))
        return false;

    if ((is_key_pressed(KC_RETURN, KMod_DONTCARE)
        && ((game_system_screen != SySc_CONTROLS && game_system_screen != SySc_NETGAME)
            || screentype != SCRT_SYSMENU) && !edit_flag)
        || mouse_move_over_box(&rect))
    {
        if (lbDisplay.MLeftButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.LeftButton = 0;
            word_1C498A = 50;
        }
        else
        {
            if (word_1C498A == 50 || is_key_pressed(KC_RETURN, KMod_DONTCARE))
            {
                word_1C498A = 0;
                clear_key_pressed(KC_RETURN);
                if (!is_purple_alert_on_top())
                {
                    if (activate_queued_mail() == 1)
                    {
                        word_1C6F40 = next_brief - 5;
                        if (word_1C6F40 < 0)
                            word_1C6F40 = 0;
                        open_brief = next_brief;
                        change_screen = ChSCRT_MISBRIEF;
                        subtext = gui_strings[372];
                    }
                    else
                    {
                        word_1C6F3E = next_email - 4;
                        if (word_1C6F3E < 0)
                            word_1C6F3E = 0;
                        change_screen = ChSCRT_MISBRIEF;
                        subtext = gui_strings[373];
                        open_brief = -next_email;
                    }
                    set_heading_box_text(subtext);
                    play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);
                    if (new_mail)
                    {
                        play_sample_using_heap(0,
                          119 + (LbRandomAnyShort() % 3), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
                    }
                    else
                    {
                        byte_1C4980 = new_mail;
                    }
                }
            }
        }
    }
    // If not mouse over, make sure to clear the state
    else if (word_1C498A == 50)
        word_1C498A = 0;

    return false;
}

void draw_purple_app_email_icon(short cx, short cy, short bri)
{
    struct TbSprite *spr;
    char locstr[48];
    const char *text;
    short iconid;
    short tx;

    iconid = bri - word_1C6F40;
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    spr = &fe_icons_sprites[102];
    if (mouse_move_over_rect(cx, cx + spr->SWidth + 1, cy,
        cy + 1 + spr->SHeight))
    {
        if ((byte_1C497F & (1 << iconid)) == 0)
        {
            byte_1C497F |= (1 << iconid);
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
        // If clicked, draw the icon without transparency
        if (lbDisplay.MLeftButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.DrawFlags = 0;
        }
        lbDisplay.DrawFlags |= 0x8000;
        draw_sprite_purple_list(cx, cy, spr);
        lbDisplay.DrawFlags = 0;
    }
    else
    {
        byte_1C497F &= ~(1 << iconid);
        lbDisplay.DrawFlags |= 0x8000;
        draw_sprite_purple_list(cx, cy, spr);
        lbDisplay.DrawFlags &= ~0x8000;
    }
    lbFontPtr = small2_font;
    lbDisplay.DrawColour = 87;
    if (mission_remain_until_success(brief_store[bri].Mission))
        lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
    my_set_text_window(cx, cy, spr->SWidth + 2, spr->SHeight);
    draw_text_purple_list2(8, 3, misc_text[4], 0);

    lbFontPtr = med2_font;
    sprintf(locstr, "%d", brief_store[bri].RefNum);
    text = loctext_to_gtext(locstr);
    tx = (35 - LbTextStringWidth(locstr)) >> 1;
    draw_text_purple_list2(tx, 10, text, 0);

    lbFontPtr = small2_font;
    sprintf(locstr, "%02d/%02d", (int) brief_store[bri].RecvDay,
        (int) brief_store[bri].RecvMonth);
    text = loctext_to_gtext(locstr);
    tx = (35 - LbTextStringWidth(locstr)) >> 1;
    draw_text_purple_list2(tx, 23, text, 0);

    sprintf(locstr, "%02dNC", (int) brief_store[bri].RecvYear);
    text = loctext_to_gtext(locstr);
    tx = (35 - LbTextStringWidth(locstr)) >> 1;
    draw_text_purple_list2(tx, 30, text, 0);

    draw_text_purple_list2(4, 37, gui_strings[375], 0);
    lbDisplay.DrawFlags = 0;
}

TbBool get_purple_app_email_icon_inputs(short cx, short cy, short bri)
{
    struct TbSprite *spr;

    spr = &fe_icons_sprites[102];
    if (mouse_move_over_rect(cx, cx + spr->SWidth + 1, cy,
        cy + 1 + spr->SHeight))
    {
        if (lbDisplay.MLeftButton || (joy.Buttons[0] && !net_unkn_pos_02))
        {
            lbDisplay.LeftButton = 0;
            word_1C498A = 2 * (bri + 1) + 100;
        }
        else if (word_1C498A == 2 * (bri + 1) + 100)
        {
            change_screen = ChSCRT_MISBRIEF;
            set_heading_box_text(gui_strings[372]);
            open_brief = bri + 1;
            play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2);

            word_1C498A = 0;
        }
    }
    // If not mouse over, make sure to clear the state
    else if (word_1C498A == 2 * (bri + 1) + 100)
        word_1C498A = 0;

    return false;
}

/** Show a collection of icons at bottom of the screen.
 */
void show_purple_apps_selection_bar(void)
{
#if 0
    asm volatile ("call ASM_show_purple_apps_selection_bar\n"
        :  :  : "eax" );
    return;
#endif
    ushort bri;
    short iconid;
    short cx, cy;

    // Show utility icons in bottom left
    for (iconid = ApBar_SYSTEM; iconid < ApBar_NEWMAIL; iconid++)
    {
        draw_purple_app_utility_icon(iconid);
    }

    // Show unread mail notification icon
    draw_purple_app_unread_email_icon();

    // Show email icons in bottom right
    {
        struct TbSprite *spr;
        spr = &fe_icons_sprites[102];
        cx = global_apps_bar_box.X + global_apps_bar_box.Width - spr->SWidth;
        cy = global_apps_bar_box.Y;
    }

    for (bri = word_1C6F40; bri < next_brief; bri++)
    {
        if (bri >= word_1C6F40 + PURPLE_APPS_EMAIL_ICONS_LIMIT)
            break;

        draw_purple_app_email_icon(cx, cy, bri);

        {
            struct TbSprite *spr;
            spr = &fe_icons_sprites[102];
            cx -= spr->SWidth + 3;
        }
    }
}

TbBool input_purple_apps_selection_bar(void)
{
    short iconid;
    ushort bri;
    short cx, cy;

    // Get inputs from utility icons in bottom left
    for (iconid = ApBar_SYSTEM; iconid < ApBar_NEWMAIL; iconid++)
    {
        if (!is_purple_alert_on_top())
            get_purple_app_utility_icon_inputs(iconid);
    }

    // Get inputs from unread mail notification icon
    get_purple_app_unread_email_icon_inputs();

    // Get inputs from email icons in bottom right
    {
        struct TbSprite *spr;
        spr = &fe_icons_sprites[102];
        cx = global_apps_bar_box.X + global_apps_bar_box.Width - spr->SWidth;
        cy = global_apps_bar_box.Y;
    }

    for (bri = word_1C6F40; bri < next_brief; bri++)
    {
        if (bri >= word_1C6F40 + PURPLE_APPS_EMAIL_ICONS_LIMIT)
            break;

        get_purple_app_email_icon_inputs(cx, cy, bri);

        {
            struct TbSprite *spr;
            spr = &fe_icons_sprites[102];
            cx -= spr->SWidth + 3;
        }
    }

    if (!is_defining_control_key())
    {
        if (is_key_pressed(KC_F1, KMod_DONTCARE))
        {
            clear_key_pressed(KC_F1);
            change_screen = ChSCRT_SYSMENU;
        }
        if (is_key_pressed(KC_F2, KMod_DONTCARE))
        {
            clear_key_pressed(KC_F2);
            change_screen = ChSCRT_WORLDMAP;
        }
        if (is_key_pressed(KC_F3, KMod_DONTCARE))
        {
            clear_key_pressed(KC_F3);
            change_screen = ChSCRT_CRYO;
        }
        if (is_key_pressed(KC_F4, KMod_DONTCARE))
        {
            clear_key_pressed(KC_F4);
            change_screen = ChSCRT_EQUIP;
        }
        if (is_key_pressed(KC_F5, KMod_DONTCARE))
        {
            clear_key_pressed(KC_F5);
            if (research.NumBases > 0)
                change_screen = ChSCRT_RESEARCH;
        }
        if (is_key_pressed(KC_F6, KMod_DONTCARE))
        {
            clear_key_pressed(KC_F6);
            if (open_brief != 0)
                change_screen = ChSCRT_MISBRIEF;
        }
    }

    return false;
}

void init_global_app_bar_box(void)
{
    short scr_w, start_x;

    scr_w = lbDisplay.GraphicsWindowWidth;
    start_x = (scr_w - global_apps_bar_box.Width) / 2;
    global_apps_bar_box.X = start_x;
    global_apps_bar_box.Y = lbDisplay.GraphicsWindowHeight - global_apps_bar_box.Height;
}

void reset_app_bar_player_state(void)
{
    new_mail = 0;
    next_brief = 0;
    word_1C6F3E = 0;
    word_1C6F40 = 0;
    next_email = 0;
    next_ref = 0;
    open_brief = 0;
}

/******************************************************************************/
