/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feequip.c
 *     Front-end desktop and menu system, equipment screen.
 * @par Purpose:
 *     Implement functions for equipment screen in front-end desktop.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 22 Oct 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "feequip.h"

#include <assert.h>
#include "bftext.h"
#include "bfsprite.h"
#include "bfkeybd.h"
#include "bfmemut.h"
#include "bfscrcopy.h"
#include "bfutility.h"
#include "bfjoyst.h"
#include "ssampply.h"

#include "feappbar.h"
#include "fecryo.h"
#include "femain.h"
#include "fenet.h"
#include "game_options.h"
#include "game_speed.h"
#include "guiboxes.h"
#include "guitext.h"
#include "display.h"
#include "campaign.h"
#include "cybmod.h"
#include "game_sprts.h"
#include "game_data.h"
#include "game.h"
#include "keyboard.h"
#include "mydraw.h"
#include "network.h"
#include "packetfe.h"
#include "player.h"
#include "purpldrw.h"
#include "purpldrwlst.h"
#include "research.h"
#include "weapon.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
struct ScreenTextBox equip_name_box = {0};
struct ScreenBox weapon_slots = {0};
struct ScreenTextBox equip_list_head_box = {0};
struct ScreenTextBox equip_list_box = {0};
struct ScreenTextBox equip_display_box = {0};
struct ScreenButton equip_offer_buy_button = {0};
struct ScreenInfoBox equip_cost_box = {0};
extern struct ScreenButton equip_all_agents_button;
extern struct ScreenShape equip_agent_select_shapes[5];

extern ubyte byte_155174; // = 166;
extern ubyte byte_155175[];
extern ubyte byte_155180; // = 109;
extern ubyte byte_155181[];
extern ubyte cheat_research_weapon;
extern ubyte equip_agents_panel_draw_state;
extern ubyte equip_agent_name_draw_state;

extern ubyte mo_from_agent;

extern char unkn41_text[];
extern char equip_cost_text[20];
extern ubyte weapon_nrg[31];
extern ubyte weapon_range[31];
extern ubyte weapon_damage[31];

/* Points for shape of the agent selection buttons, X coords.
 */
short agent_panel_shape_points_x[] = {
      0,  23, 120, 103,  35,  22,   7,   0,
};
/* Points for shape of the agent selection buttons, Y coords.
 */
short agent_panel_shape_points_y[] = {
     23,   0,   0,  17,  17,  30,  30,  23,
};
/* Points for skewed rectangle with agent name, X coords.
 */
short agent_name_shape_points_x[] = {
      0, 181, 164,   0,   0,
};
/* Points for skewed rectangle with agent name, Y coords.
 */
short agent_name_shape_points_y[] = {
      0,   0,  17,  17,   0,
};

/******************************************************************************/

ubyte ac_display_weapon_info(struct ScreenTextBox *box);
ubyte ac_show_weapon_name(struct ScreenTextBox *box);
ubyte ac_show_weapon_list(struct ScreenTextBox *box);
ubyte ac_do_equip_offer_buy(ubyte click);
ubyte ac_sell_equipment(ubyte click);
ubyte ac_select_all_agents(ubyte click);
void ac_weapon_flic_data_to_screen(void);
ubyte ac_do_equip_all_agents_set(ubyte click);

ubyte do_equip_offer_buy_cybmod(ubyte click);

TbBool dragged_weapon_can_drop_on_research(void)
{
    return (mo_weapon != -1 && mo_weapon == research.CurrentWeapon);
}

void dragged_weapon_drop_on_research(void)
{
    assert(dragged_weapon_can_drop_on_research());

    LOGSYNC("Transferred weapon %s from agent %d to research",
      weapon_codename(mo_weapon+1), mo_from_agent);
    player_cryo_remove_weapon_one(mo_from_agent, mo_weapon + 1);
    research_unkn_func_003();
    mo_weapon = -1;
}

TbBool weapon_has_display_anim(ubyte weapon)
{
    if (weapon < 1)
        return false;
    return true;
}

void update_equip_cost_text(void)
{
    int cost;

    if (selected_weapon == -1) // No weapon selected
    {
        equip_cost_text[0] = '\0';
        return;
    }

    cost = 100 * weapon_defs[selected_weapon + 1].Cost;
    if (equip_offer_buy_button.CallBackFn == ac_do_equip_offer_buy)
        sprintf(equip_cost_text, "%d", cost);
    else
        sprintf(equip_cost_text, "%d", cost >> 1);
}

void equip_name_box_redraw(struct ScreenTextBox *p_box)
{
    const char *text;

    if (selected_weapon + 1 < 1)
    {
        text = NULL;
    }
    else if (is_research_weapon_completed(selected_weapon + 1) || (login_control__State != LognCt_Unkn6))
    {
        struct Campaign *p_campgn;
        ushort strid;

        p_campgn = &campaigns[background_type];
        strid = p_campgn->WeaponsTextIdShift + selected_weapon + 1 - 1;
        text = gui_strings[strid];
    }
    else
    {
        text = gui_strings[65];
    }
    p_box->Text = text;
    p_box->TextFadePos = -5;
}

void equip_display_box_redraw(struct ScreenTextBox *p_box)
{
    const char *text;
    ubyte real_dbcontent;

    real_dbcontent = weapon_has_display_anim(selected_weapon + 1) ? display_box_content : DiBoxCt_TEXT;
    switch (real_dbcontent)
    {
    case DiBoxCt_TEXT:
        p_box->Flags |= GBxFlg_Unkn0080;
        // Re-add scroll bars
        p_box->Flags |= GBxFlg_RadioBtn;

        p_box->Lines = 0;
        if (selected_weapon + 1 < 1) {
            text = NULL;
        } else if (is_research_weapon_completed(selected_weapon + 1) || (login_control__State != LognCt_Unkn6)) {
            text = &weapon_text[weapon_text_index[selected_weapon]];
        } else {
            text = gui_strings[536];
        }
        p_box->Text = text;
        p_box->TextFadePos = -5;
        break;
    case DiBoxCt_ANIM:
        // Remove scroll bars
        p_box->Flags &= ~GBxFlg_RadioBtn;

        init_weapon_anim(selected_weapon + 1 - 1);
        // Negative value saves the background before starting animation
        p_box->TextFadePos = -2;
        break;
    }
}

void equip_update_for_selected_weapon(void)
{
    update_equip_cost_text();

    if (selected_weapon == -1) // No weapon selected
    {
        return;
    }
    equip_name_box_redraw(&equip_name_box);
    equip_display_box_redraw(&equip_display_box);
}

ubyte do_equip_offer_buy_weapon(ubyte click)
{
    struct WeaponDef *wdef;
    ubyte nbought;

    wdef = &weapon_defs[selected_weapon + 1];
    nbought = 0;

    if (selected_agent != 4)
    {
        long cost;
        short plagent;
        TbBool added;

        cost = 100 * wdef->Cost;
        plagent = selected_agent;

        if (ingame.Credits - cost < 0)
            added = false;
        else if (!free_slot(plagent, selected_weapon + 1)) {
            added = false;
        } else {
            added = player_cryo_add_weapon_one(plagent, selected_weapon + 1);
        }

        if (added) {
            ingame.Credits -= cost;
            ingame.Expenditure += cost;
            nbought++;
        }
    }
    else
    {
        long cost;
        short plagent;
        TbBool added;

        cost = 100 * wdef->Cost;

        for (plagent = 0; plagent < 4; plagent++)
        {
            if (ingame.Credits - cost < 0)
                break;

            if (!free_slot(plagent, selected_weapon + 1)) {
                added = false;
            } else {
                added = player_cryo_add_weapon_one(plagent, selected_weapon + 1);
            }

            if (added) {
                ingame.Credits -= cost;
                ingame.Expenditure += cost;
                nbought++;
            }
        }
    }

    if (nbought > 0)
    {
        if ((login_control__State == LognCt_Unkn5 && (net_game_play_flags & NGPF_Unkn08) != 0)) {
            net_players_copy_equip_and_cryo();
        }
    }
    return (nbought > 0);
}

ubyte do_equip_offer_buy(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_equip_offer_buy\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    ubyte done;

    if (selected_agent == -1) {
        return 0;
    }

    if ((login_control__State == LognCt_Unkn5) && ((net_game_play_flags & NGPF_Unkn08) != 0))
    {
        if (!net_local_player_hosts_the_game())
            return 0;
    }

    switch (screentype)
    {
    case SCRT_EQUIP:
        done = do_equip_offer_buy_weapon(click);
        break;
    case SCRT_CRYO:
        done = do_equip_offer_buy_cybmod(click);
        break;
    default:
        done = 0;
        break;
    }
    return done;
}

ubyte sell_equipment(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_sell_equipment\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    TbBool sold;

    if ((login_control__State == LognCt_Unkn5) && ((net_game_play_flags & NGPF_Unkn08) != 0))
    {
        if (!net_local_player_hosts_the_game())
            return 0;
    }

    sold = false;
    if (screentype == SCRT_EQUIP)
    {
        struct WeaponDef *wdef;

        wdef = &weapon_defs[selected_weapon + 1];

        if (selected_agent == 4)
        {
            ushort cryo_no;

            for (cryo_no = 0; cryo_no < 4; cryo_no++)
            {
                if (player_cryo_remove_weapon_one(cryo_no, selected_weapon + 1)) {
                    ingame.Credits += (100 * wdef->Cost) >> 1;
                    sold = true;
                }
            }
        }
        else
        {
            if (player_cryo_remove_weapon_one(selected_agent, selected_weapon + 1)) {
                ingame.Credits += (100 * wdef->Cost) >> 1;
                sold = true;
            }

        }
        if ((login_control__State == LognCt_Unkn5) && ((net_game_play_flags & NGPF_Unkn08) != 0))
        {
            net_players_copy_equip_and_cryo();
        }
    }

    if (sold)
    {
        check_buy_sell_button();
    }
    return 1;
}

/** Determines if buy or sell should be available in the equip weapon offer.
 *
 * @return Gives 0 if button unavailable, 1 for buy, 2 for sell.
 */
ubyte equip_offer_can_buy_or_sell(WeaponType wtype)
{
    if (selected_agent < 0)
        return 0;

    if (selected_agent == 4) // All agents selected
    {
        short plagent;

        if (is_research_weapon_completed(wtype) ||
          (login_control__State != LognCt_Unkn6))
            return 1;

        for (plagent = 0; plagent < 4; plagent++)
        {
            if (!player_agent_has_weapon(local_player_no, plagent, wtype))
              break;
        }
        if (plagent == 4) // all agents have that weapon
            return 2;

        return 0;
    }

    if (player_agent_has_weapon(local_player_no, selected_agent, wtype))
        return 2;

    if (is_research_weapon_completed(wtype) ||
      (login_control__State != LognCt_Unkn6))
        return 1;

    return 0;
}

ubyte get_buy_sell_button_mode(void)
{
    ubyte mode;

    mode = 0;
    if (screentype == 5)
    {
        if (selected_weapon == -1)
        {
            // no weapon selected - no check
        }
        else
        {
            mode = equip_offer_can_buy_or_sell(selected_weapon + 1);
        }
    }
    else
    {
        if (selected_mod == -1)
        {
            // no mod selected - no check
        }
        else
        {
            mode = cryo_offer_can_buy_or_sell(selected_mod + 1);
        }
    }
    return mode;
}

void check_buy_sell_button(void)
{
#if 0
    asm volatile ("call ASM_check_buy_sell_button\n"
        :  :  : "eax" );
#endif
    ubyte mode;

    mode = get_buy_sell_button_mode();

    if (mode == 1)
    {
        equip_offer_buy_button.Text = gui_strings[436];
        equip_offer_buy_button.CallBackFn = ac_do_equip_offer_buy;
    }
    else if (mode == 2)
    {
        equip_offer_buy_button.Text = gui_strings[407];
        equip_offer_buy_button.CallBackFn = ac_sell_equipment;
    }

    // The functions below may check callback function to figure out mode
    if (screentype == 5)
    {
        update_equip_cost_text();
    }
    else
    {
        update_cybmod_cost_text();
    }
}

ubyte select_all_agents(ubyte click)
{
    selected_agent = 4;
    check_buy_sell_button();
    return 1;
}

void switch_equip_offer_to_buy(void)
{
    equip_offer_buy_button.Text = gui_strings[436];
    equip_offer_buy_button.CallBackFn = ac_do_equip_offer_buy;
}

void switch_equip_offer_to_sell(void)
{
    equip_offer_buy_button.Text = gui_strings[407];
    equip_offer_buy_button.CallBackFn = ac_sell_equipment;
}

void skip_flashy_draw_equipment_screen_boxes(void)
{
    short i;

    skip_flashy_draw_heading_screen_boxes();

    equip_list_head_box.Flags |= GBxFlg_Unkn0002;
    weapon_slots.Flags |= GBxFlg_Unkn0002;
    equip_display_box.Flags |= GBxFlg_Unkn0002;
    equip_name_box.Flags |= GBxFlg_Unkn0002;
    equip_list_box.Flags |= GBxFlg_Unkn0002;
    equip_offer_buy_button.Flags |= GBxFlg_Unkn0002;
    equip_cost_box.Flags |= GBxFlg_Unkn0002;
    equip_all_agents_button.Flags |= GBxFlg_Unkn0002;
    for (i = 0; i < 5; i++) {
        equip_agent_select_shapes[i].Flags = GBxFlg_Unkn0002;
    }

    equip_agents_panel_draw_state = 1;
    equip_agent_name_draw_state = 1;
}

TbBool weapon_available_for_purchase(WeaponType wtype)
{
    struct WeaponDef *wdef;

    if (wtype < WEP_NULL + 1 || wtype >= WEP_TYPES_COUNT)
        return false;

    wdef = &weapon_defs[wtype];

    return ((wdef->Flags & WEPDFLG_CanPurchease) && (research.WeaponsCompleted & (1 << (wtype-1))))
            || (login_control__State == LognCt_Unkn5 && login_control__TechLevel >= weapon_tech_level[wtype]);
}

ubyte flashy_draw_agent_panel_shape(struct ScreenShape *p_shape, ubyte gbstate)
{
    ubyte drawn;

    if ((gbstate == GBxSta_NORMAL) || (gbstate == GBxSta_HLIGHT1)) {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    } else {
        lbDisplay.DrawFlags = 0;
    }
    drawn = flashy_draw_purple_shape(p_shape);
    lbDisplay.DrawFlags = 0;
    return drawn;
}

void draw_agent_panel_shape(struct ScreenShape *shape, ushort spridx, ubyte gbstate)
{
    if ((gbstate == GBxSta_NORMAL) || (gbstate == GBxSta_HLIGHT1) || (gbstate == GBxSta_HLIGHT2)) {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    } else {
        lbDisplay.DrawFlags = 0;
    }
    draw_sprite_purple_list(shape->PtX[0] - 59, shape->PtY[2] - 3,
      &fe_icons_sprites[139]);
    if ((gbstate == GBxSta_HLIGHT2) || (gbstate == GBxSta_HLIGHT1)) {
        lbDisplay.DrawFlags = 0;
    }
    if (gbstate == GBxSta_HLIGHT2) {
        draw_sprite_purple_list(shape->PtX[0] - 59, shape->PtY[2] - 3,
          &fe_icons_sprites[144]);
    }
    lbDisplay.DrawFlags |= 0x8000;
    draw_sprite_purple_list(shape->PtX[1] - 16, shape->PtY[1] + 8,
      &fe_icons_sprites[spridx]);
    lbDisplay.DrawFlags &= ~0x8000;

    lbDisplay.DrawFlags = 0;
}

/** Checks if mouse is over the shape which is agent panel.
 *
 * This will work properly on a shape which represents agent panel. It is optimized
 * for it by checking two slant boxes rather than any shape.
 */
TbBool mouse_over_agent_panel_shape(struct ScreenShape *shape)
{
    return (mouse_move_over_rect_adv(shape->PtX[0] + 6, shape->PtY[0] - 6, 30, 15, 0) ||
      mouse_move_over_rect_adv(shape->PtX[1], shape->PtY[1], 98, 18, 0));
}

ubyte input_equip_agent_panel_shape(struct ScreenShape *shape, sbyte nagent)
{
    ubyte gbstate;

    if (is_key_pressed(KC_1 + nagent, KMod_DONTCARE))
    {
        clear_key_pressed(KC_1 + nagent);
        if (nagent < cryo_agents.NumAgents)
        {
            selected_agent = nagent;
            check_buy_sell_button();
        }
    }

    if (mouse_over_agent_panel_shape(shape))
    {
        if ((shape->Flags & 0x0200) == 0) {
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
            shape->Flags |= 0x0200;
        }
        if (lbDisplay.MLeftButton || (joy.Buttons[0] != 0))
        {
            lbDisplay.LeftButton = 0;
            shape->Flags |= 0x0400;
            gbstate = GBxSta_PUSHED;
        }
        else
        {
            if ((nagent >= cryo_agents.NumAgents) || (mo_weapon == -1))
            {
                if ((shape->Flags & 0x0400) != 0)
                {
                    if (nagent < cryo_agents.NumAgents)
                    {
                        play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                        selected_agent = nagent;
                        check_buy_sell_button();
                    }
                    else
                    {
                        play_sample_using_heap(0, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                    }
                    shape->Flags &= ~0x0400;
                }
            }
            else
            {
                if (free_slot(nagent, mo_weapon + 1))
                {
                    LOGSYNC("Transferred weapon %s from agent %d to agent %d",
                      weapon_codename(mo_weapon+1), mo_from_agent, nagent);
                    player_cryo_transfer_weapon_between_agents(mo_from_agent, nagent, mo_weapon+1);
                }
                mo_weapon = -1;
                shape->Flags &= ~0x0400;
            }
            gbstate = GBxSta_HLIGHT2;
        }
    }
    else
    {
        if (shape->Flags & 0x0200)
            shape->Flags &= ~0x0200;
        if (shape->Flags & 0x0400)
            shape->Flags &= ~0x0400;
        if ((selected_agent == nagent) || (selected_agent == 4))
        {
            gbstate = GBxSta_HLIGHT1;
        }
        else
        {
            gbstate = GBxSta_NORMAL;
        }
    }
    return gbstate;
}

ubyte do_equip_all_agents_set(ubyte click)
{
    struct ScreenButton *button = &equip_all_agents_button;
    *(button->Radio) = button->RadioValue;
    check_buy_sell_button();
    return 1;
}

ubyte input_equip_all_agents_button(struct ScreenButton *button)
{
    ubyte gbstate;
    sbyte nagent = 4;

    gbstate = GBxSta_NORMAL;
    if (is_key_pressed(KC_1 + nagent, KMod_DONTCARE))
    {
        clear_key_pressed(KC_1 + nagent);
        selected_agent = nagent;
        check_buy_sell_button();
        gbstate = GBxSta_HLIGHT2;
    }
    return gbstate;
}

ubyte flashy_draw_draw_equip_agent_name_shape(struct ScreenShape *p_shape, ubyte gbstate)
{
    return flashy_draw_purple_shape(p_shape);
}

void draw_equip_agent_name_shape(struct ScreenShape *p_shape, ubyte gbstate)
{
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_box_purple_list(p_shape->PtX[0] - 3, p_shape->PtY[0] - 3,
      168, 24, 56);
    draw_triangle_purple_list(p_shape->PtX[1] + 7, p_shape->PtY[1] - 3,
      p_shape->PtX[2] + 1, p_shape->PtY[2] + 3,
      p_shape->PtX[2] + 1, p_shape->PtY[1] - 3, 56);
    lbDisplay.DrawFlags = 0;
    flashy_draw_purple_shape(p_shape);
    if ((selected_agent >= 0) && (selected_agent < 4))
    {
        const char *name;
        lbFontPtr = med_font;
        my_set_text_window(p_shape->PtX[0] + 4, p_shape->PtY[0] + 4,
          p_shape->PtX[2] - p_shape->PtX[0] - 1, p_shape->PtY[2] - p_shape->PtY[0] + 1);
        name = get_cryo_agent_name(selected_agent);
        draw_text_purple_list2(0, 0, name, 0);
    }
}

void draw_text_property_bk(struct ScreenBoxBase *box, const char *text)
{
    my_set_text_window(box->X, box->Y, box->Width, box->Height);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    lbFontPtr = small_med_font;
    draw_text_purple_list2(0, 0, text, 0);
}

void draw_text_property_lv(struct ScreenBoxBase *box, const char *text)
{
    short cy;
    my_set_text_window(box->X, box->Y, box->Width, box->Height);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    lbFontPtr = small_med_font;
    cy = box->Height - my_char_height('A');
    draw_text_purple_list2(0, cy, text, 0);
}

void draw_discrete_rects_bar_bk(struct ScreenBoxBase *box, const char *text, TbPixel color)
{
    short n;
    short n_rects = 8;
    short rect_w, rect_h;
    short cx, cy;

    // Dimensions of one rectangle on the bar
    rect_w = 9 * box->Width / (n_rects * 10);
    rect_h = 7;

    my_set_text_window(box->X, box->Y, box->Width, box->Height);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        lbFontPtr = small_med_font;
    draw_text_purple_list2(0, 0, text, 0);

    cy = box->Y + box->Height - rect_h;
    for (n = 0; n < n_rects; n++)
    {
        cx = box->X + n * box->Width / n_rects;
        draw_box_purple_list(cx, cy, rect_w, rect_h, color);
    }
}

void draw_discrete_rects_bar_lv(struct ScreenBoxBase *box, int lv, int lv_max, TbPixel *colors)
{
    short n;
    short n_rects = 8;
    short rect_w, rect_h;
    short cx, cy;

    // Dimensions of one rectangle on the bar
    rect_w = 9 * box->Width / (n_rects * 10);
    rect_h = 7;

    cy = box->Y + box->Height - rect_h;
    for (n = 0; n < n_rects; n++)
    {
        if (lv_max * n / n_rects >= lv)
            break;
        cx = box->X + n * box->Width / n_rects;
        draw_box_purple_list(cx, cy, rect_w, rect_h, colors[n]);
    }
}

ubyte show_equipment_screen(void)
{
    ubyte drawn = true;

    if ((equip_agent_select_shapes[0].Flags & GBxFlg_Unkn0001) != 0)
    {
        equip_agents_panel_draw_state = 0;
        equip_agent_name_draw_state = 0;
    }
    if (((game_projector_speed) && is_heading_flag01()) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_equipment_screen_boxes();
    }
    if ((ingame.UserFlags & UsrF_Cheats) != 0)
    {
        if (is_key_pressed(KC_0, KMod_DONTCARE))
        {
            clear_key_pressed(KC_0);
            if (cheat_research_weapon < WEP_TYPES_COUNT) {
                refresh_equip_list = 1;
                research_weapon_complete(cheat_research_weapon + 1);
                cheat_research_weapon++;
            }
        }
    }
    if (refresh_equip_list)
    {
        WeaponType wtype;
        equip_list_box.Lines = 0;
        for (wtype = WEP_NULL + 1; wtype < WEP_TYPES_COUNT; wtype++)
        {
            if (weapon_available_for_purchase(wtype)) {
                equip_list_box.Lines++;
            }
        }
        equip_list_box.Flags |= GBxFlg_Unkn0080;
        refresh_equip_list = 0;
    }

    // Draw sequentially
    if (drawn)
        drawn = draw_heading_box();

    if (drawn)
    {
        sbyte nagent;
        ubyte agnt[4];
        ubyte boxes_drawn;
        boxes_drawn = 1;
        for (nagent = 4; nagent >= 0; nagent--)
        {
            struct ScreenShape *p_shape;
            ubyte gbstate;

            p_shape = &equip_agent_select_shapes[nagent];

            if (nagent == 4) // agent name box
            {
                ubyte drawn;

                // Agents grouping has little to do with name box, but it's convienient to put here
                gbstate = input_equip_all_agents_button(&equip_all_agents_button);

                if (equip_agent_name_draw_state == 0)
                {
                    drawn = flashy_draw_draw_equip_agent_name_shape(p_shape, gbstate);
                }
                else if (equip_agent_name_draw_state == 1)
                {
                    draw_equip_agent_name_shape(p_shape, gbstate);
                    drawn = 3;
                }
                equip_agent_name_draw_state = (drawn == 3);
            }
            else
            {
                ubyte drawn;
                ushort spridx;

                gbstate = input_equip_agent_panel_shape(p_shape, nagent);

                if (equip_agents_panel_draw_state == 0)
                {
                    drawn = flashy_draw_agent_panel_shape(p_shape, gbstate);
                }
                else
                {
                    spridx = 140 + nagent;
                    draw_agent_panel_shape(p_shape, spridx, gbstate);
                    drawn = 3;
                }
                // Is the flashy draw finished for current button
                agnt[nagent] = (drawn == 3);
                // When all buttons started actually drawing, we can begin flashy draw of the panels below
                boxes_drawn &= (drawn > 1);
            }
        }

        if (equip_agents_panel_draw_state == 0) {
            equip_agents_panel_draw_state = agnt[0] && agnt[1] && agnt[2] && agnt[3];
        }
        drawn = boxes_drawn;
    }

    if (drawn)
    {
        //drawn = equip_all_agents_button.DrawFn(&equip_all_agents_button); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_all_agents_button), "g" (equip_all_agents_button.DrawFn));
        //drawn = equip_list_head_box.DrawFn(&equip_list_head_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_list_head_box), "g" (equip_list_head_box.DrawFn));
        //drawn = weapon_slots.DrawFn(&weapon_slots); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&weapon_slots), "g" (weapon_slots.DrawFn));
    }

    if (drawn)
    {
        //drawn = equip_list_box.DrawFn(&equip_list_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_list_box), "g" (equip_list_box.DrawFn));
    }

    if (drawn)
    {
        //drawn = equip_name_box.DrawFn(&equip_name_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_name_box), "g" (equip_name_box.DrawFn));
        //drawn = equip_display_box.DrawFn(&equip_display_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_display_box), "g" (equip_display_box.DrawFn));
    }

    if (mo_weapon != -1)
    {
        short ms_x, ms_y;
        struct TbSprite *spr;

        ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
        ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;
        spr = &fepanel_sprites[weapon_sprite_index(mo_weapon + 1, true)];
        draw_sprite_purple_list(ms_x, ms_y, spr);
    }

    return drawn;
}

void init_weapon_anim(ubyte weapon)
{
    struct Animation *p_anim;
    PathInfo *pinfo;
    ulong k;
    ubyte anislot;

    pinfo = &game_dirs[DirPlace_Equip];

    anislot = AniSl_EQVIEW;
    if (weapon >= 32)
    {
        k = anim_slots[anislot];
        p_anim = &animations[k];
        anim_flic_set_fname(p_anim, "%s/mod-%02d.fli", pinfo->directory, (int)weapon - 32);
    }
    else
    {
        k = anim_slots[anislot];
        p_anim = &animations[k];
        anim_flic_set_fname(p_anim, "%s/wep-%02d.fli", pinfo->directory, (int)weapon);
    }
    flic_unkn03(anislot);
}

void weapon_flic_data_to_screen(void)
{
    struct Animation *p_anim;
    ulong k;
    short w, h;

    w = equip_display_box.Width - 8;
    h = w * 7 / 10;
    k = anim_slots[AniSl_EQVIEW];
    p_anim = &animations[k];

    LbScreenSetGraphicsWindow(equip_display_box.X + 4, equip_display_box.Y + 4, w, h);
    // Frame zero means animation didn't started yet; use the opportunity to copy
    // clean background to the animation playback buffer
    if (p_anim->FrameNumber == 0) {
        LbScreenSave(lbDisplay.GraphicsWindowPtr, p_anim->FrameBuffer,
            lbDisplay.GraphicsWindowHeight);
    } else {
        LbScreenCopy(p_anim->FrameBuffer, lbDisplay.GraphicsWindowPtr,
            lbDisplay.GraphicsWindowHeight);
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
}

TbBool input_display_box_content_wep(struct ScreenTextBox *p_box)
{
    if (mouse_down_over_box_coords(p_box->X + 4,
      p_box->Y + 4, p_box->X + p_box->Width - 4, p_box->Y + 4 + 140))
    {
        if (lbDisplay.LeftButton)
        {
            lbDisplay.LeftButton = 0;
            display_box_content_state_switch();
            equip_display_box_redraw(p_box);
            return true;
        }
    }
    return false;
}

void display_box_content_state_switch(void)
{
    switch (display_box_content)
    {
    case DiBoxCt_TEXT:
        display_box_content = DiBoxCt_ANIM;
        break;
    case DiBoxCt_ANIM:
        display_box_content = DiBoxCt_TEXT;
        break;
    default:
        display_box_content = DiBoxCt_ANIM;
        break;
    }
}

void draw_display_box_content_wep(struct ScreenTextBox *p_box)
{
    ubyte real_dbcontent;

    real_dbcontent = weapon_has_display_anim(selected_weapon + 1) ? display_box_content : DiBoxCt_TEXT;
    switch (real_dbcontent)
    {
    case DiBoxCt_TEXT:
        lbFontPtr = p_box->Font;
        my_set_text_window(p_box->X + 4, p_box->ScrollWindowOffset + p_box->Y + 4,
          p_box->Width - 20, p_box->ScrollWindowHeight + 23);
        flashy_draw_text(0, 0, p_box->Text, p_box->TextSpeed, p_box->TextTopLine,
          &p_box->TextFadePos, 0);
        break;
    case DiBoxCt_ANIM:
        if (p_box->TextFadePos < 0)
            // Mark that we should start animation frames the next time
            p_box->TextFadePos++;
        else
            xdo_next_frame(AniSl_EQVIEW);
        draw_flic_purple_list(ac_weapon_flic_data_to_screen);
        break;
    }
}

ubyte display_weapon_info(struct ScreenTextBox *box)
{
    short stridx;
    struct ScreenBoxBase categ_box = {box->X + 8, box->Y + 148, 192, 17};
    struct ScreenBoxBase power_box = {box->X + 8, box->Y + 177, 192, 17};
    struct ScreenBoxBase range_box = {box->X + 8, box->Y + 202, 192, 17};
    struct ScreenBoxBase energ_box = {box->X + 8, box->Y + 227, 192, 17};

    if ((box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(box->X + 4, box->Y + 158,
            box->Width - 8, 13, 56);
        draw_box_purple_list(box->X + 4, box->Y + 174,
            box->Width - 8, 73, 56);

        draw_text_property_bk(&categ_box, gui_strings[426]);
        draw_discrete_rects_bar_bk(&power_box, gui_strings[427], byte_155174);
        draw_discrete_rects_bar_bk(&range_box, gui_strings[428], byte_155180);
        draw_discrete_rects_bar_bk(&energ_box, gui_strings[429], byte_155174);

        lbDisplay.DrawFlags = 0;
        box->Flags |= GBxFlg_BkgndDrawn;
        copy_box_purple_list(box->X + 4, box->Y + 146, box->Width - 8, box->Height - 143);
    }
    my_set_text_window(box->X + 4, box->Y + 4, box->Width - 8, box->Height - 8);

    if (selected_weapon == -1)
        return 0;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    lbFontPtr = small_med_font;

    // Weapon category
    if (is_research_weapon_completed(selected_weapon + 1) || (login_control__State != LognCt_Unkn6))
        stridx = 59 + weapon_defs[selected_weapon + 1].Category;
    else
        stridx = 65;
    draw_text_property_lv(&categ_box, gui_strings[stridx]);

    draw_discrete_rects_bar_lv(&power_box, weapon_damage[selected_weapon], 8, byte_155175);
    draw_discrete_rects_bar_lv(&range_box, weapon_range[selected_weapon], 8, byte_155181);
    draw_discrete_rects_bar_lv(&energ_box, weapon_nrg[selected_weapon], 8, byte_155175);

    if (equip_offer_can_buy_or_sell(selected_weapon + 1))
    {
        //equip_offer_buy_button.DrawFn(&equip_offer_buy_button); -- incompatible calling convention
        asm volatile ("call *%1\n"
            : : "a" (&equip_offer_buy_button), "g" (equip_offer_buy_button.DrawFn));
    }

    //equip_cost_box.DrawFn(&equip_cost_box); -- incompatible calling convention
    asm volatile ("call *%1\n"
        : : "a" (&equip_cost_box), "g" (equip_cost_box.DrawFn));

    // Add control hotspot for the view / description switch
    draw_hotspot_purple_list(box->X + box->Width / 2, box->Y + 104);
    draw_display_box_content_wep(box);
    input_display_box_content_wep(box);

    return 0;
}

ubyte show_weapon_name(struct ScreenTextBox *box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_weapon_name\n"
        : "=r" (ret) : "a" (box));
    return ret;
#endif
    short scr_x, scr_y;
    short text_w, text_h;

    if (((box->Flags & GBxFlg_Unkn0080) != 0) && (box->Timer != 255))
    {
        box->TextFadePos = -5;
        box->Flags &= ~GBxFlg_Unkn0080;
    }
    if (box->Text == NULL)
        return 0;

    lbFontPtr = box->Font;
    text_w = my_string_width(box->Text);
    text_h = my_char_height('A');
    scr_x = box->X + ((box->Width - text_w) >> 1);
    scr_y = box->Y + ((box->Height - text_h) >> 1);
    my_set_text_window(scr_x, scr_y, 640u, scr_y + text_h);
    flashy_draw_text(0, 0, box->Text, box->TextSpeed,
      box->TextTopLine, &box->TextFadePos, 0);

    return 0;
}

ubyte show_weapon_list(struct ScreenTextBox *box)
{
    int h0;
    int sheight;
    short weapon;
    struct TbSprite *spr;

    if ((box->Flags & 0x8000) == 0)
    {
        short w1, w2;

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(text_window_x1, text_window_y1,
          text_window_x2 - text_window_x1 + 1,
          text_window_y2 - text_window_y1 + 1, 56);
        lbDisplay.DrawFlags = 0;
        my_set_text_window(box->X + 4, box->Y + 4,
          box->Width - 8, box->Height - 8);
        lbFontPtr = med_font;

        w1 = box->Width;
        w2 = my_string_width(gui_strings[425]);
        draw_text_purple_list2((((w1 - w2) >> 1) - 3), 2, gui_strings[425], 0);
        box->Flags |= 0x8000;
        copy_box_purple_list(box->X + 4, box->Y - 3,
          box->Width - 20, box->Height + 6);
    }
    my_set_text_window(box->X + 4, box->ScrollWindowOffset + box->Y + 4,
      box->Width - 20, box->ScrollWindowHeight + 23);
    lbFontPtr = small_med_font;
    h0 = 3;
    spr = &fepanel_sprites[15 + 0];
    sheight = spr->SHeight;

    for (weapon = box->TextTopLine; (weapon < WEP_TYPES_COUNT) && (h0 + sheight < box->ScrollWindowHeight + 23); weapon++)
    {
        short msy, msx;
        short y1, y2;
        const char *text;
        WeaponType wtype;

        wtype = weapon + 1;
        if (!weapon_available_for_purchase(wtype))
            continue;

        msy = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MouseY : lbDisplay.MouseY;
        msx = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MouseX : lbDisplay.MouseX;

        y1 = text_window_y1 + h0 - 1;
        y2 = text_window_y1 + h0 + sheight + 1;
        if (over_box_coords(msx, msy, text_window_x1, y1, text_window_x2, y2))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                selected_weapon = weapon;
                switch_equip_offer_to_buy();
                equip_update_for_selected_weapon();
            }
        }

        if (weapon == selected_weapon) {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 87;
        } else {
            lbDisplay.DrawFlags = 0;
            lbDisplay.DrawColour = 247;
        }
        struct TbSprite *spr;

        spr = &fepanel_sprites[weapon_sprite_index(wtype, true)];
        lbDisplay.DrawFlags |= 0x8000;
        draw_sprite_purple_list(text_window_x1 + 2, h0 + text_window_y1, spr);
        lbDisplay.DrawFlags &= ~0x8000;
        {
            struct Campaign *p_campgn;
            ushort strid;

            p_campgn = &campaigns[background_type];
            strid = p_campgn->WeaponsTextIdShift + wtype - 1;
            text = gui_strings[strid];
        }
        spr = &fepanel_sprites[15 + wtype - 1];
        draw_text_purple_list2(spr->SWidth + 4, h0 + 1, text, 0);
        h0 += sheight + 3;
    }
    return 0;
}

void draw_weapon_slot(short x, short y)
{
    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_box_purple_list(x, y, 0x9Au, 0xFu, 243);
    draw_box_purple_list(x + 27, y + 15, 0x7Fu, 0xCu, 243);
    draw_box_purple_list(x + 27, y + 27, 0x9Au, 0xFu, 243);
    draw_triangle_purple_list(x + 1, y + 15, x + 27, y + 15, x + 27, y + 41, 243);
    draw_triangle_purple_list(x + 154, y, x + 181, y + 27, x + 154, y + 27, 243);
}

void draw_fourpack_slots(short x, short y, ubyte fp)
{
    ubyte fpcount;

    if (selected_agent < 0)
        return;

    fpcount = WEAPONS_FOURPACK_MAX_COUNT;
    if (selected_agent != 4)
    {
        fpcount = cryo_agents.FourPacks[selected_agent].Amount[fp];
    }
    else
    {
        short plagent, plagent_count;

        plagent_count = min(4,cryo_agents.NumAgents);
        for (plagent = 0; plagent < plagent_count; plagent++)
        {
            if (cryo_agents.FourPacks[plagent].Amount[fp] < fpcount)
                fpcount = cryo_agents.FourPacks[plagent].Amount[fp];
        }
    }

    lbDisplay.DrawFlags = 0;
    draw_box_purple_list(x + 28, y + 8, 4u, 4u, 174);

    if (fpcount == 1)
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_box_purple_list(x + 28, y + 30, 4u, 4u, 174);

    if (fpcount == 2)
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_box_purple_list(x + 150, y + 8, 4u, 4u, 174);

    if (fpcount == 3)
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_box_purple_list(x + 150, y + 30, 4u, 4u, 174);
}

void show_weapon_slot(short scr_x, short scr_y, WeaponType wtype)
{
    ushort fp;

    lbDisplay.DrawColour = 174;
    lbDisplay.DrawFlags = 0x8000 | Lb_TEXT_ONE_COLOR;
    draw_sprite_purple_list(scr_x, scr_y, &wepicons_sprites[wtype + 1]);
    lbDisplay.DrawFlags &= ~0x8000;

    fp = weapon_fourpack_index(wtype);
    if (fp < WFRPK_COUNT) {
        draw_fourpack_slots(scr_x, scr_y, fp);
    }

    lbDisplay.DrawFlags = 0;
    if ((mo_weapon == -1) && lbDisplay.LeftButton)
    {
        if (mouse_down_over_box_coords(scr_x, scr_y, scr_x + 181, scr_y + 42))
        {
            lbDisplay.LeftButton = 0;
            mo_weapon = (int)wtype - 1;
            mo_from_agent = selected_agent;
            LOGSYNC("Dragging weapon %s from agent %d",
              weapon_codename(mo_weapon+1), mo_from_agent);
        }
    }

    if ((mo_weapon != -1) && !lbDisplay.MLeftButton)
    {
        if (mouse_move_over_box_coords(scr_x, scr_y, scr_x + 181, scr_y + 42))
        {
            selected_weapon = (int)wtype - 1;
            switch_equip_offer_to_sell();
            equip_update_for_selected_weapon();
        }
        // Cancel the dragging, unless the mouse is on active areas handled by other functions
        // The specific active areas have to be defined here, as we really want to stop
        // the dragging if mouse was released anywhere else
        if ((int)wtype - 1 >= mo_weapon && !mouse_over_purple_apps_icon(ApBar_RESEARCH)) {
            LOGSYNC("Return dragged weapon %s to agent %d",
              weapon_codename(mo_weapon+1), mo_from_agent);
            mo_weapon = -1;
        }
    }
}

ubyte show_weapon_slots(struct ScreenBox *p_box)
{
    short scr_x, scr_y;
    short slot;
    WeaponType wtype;

    scr_x = p_box->X + 5;
    if ((p_box->Flags & GBxFlg_TextCopied) == 0)
    {
        short slot;

        scr_y = p_box->Y + 5;
        for (slot = 0; slot < 6; slot++)
        {
            draw_weapon_slot(scr_x, scr_y);
            scr_y += 44;
        }
        copy_box_purple_list(p_box->X - 3, p_box->Y - 3, p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= GBxFlg_TextCopied;
    }

    if (selected_agent < 0)
        return 0;

    scr_y = p_box->Y + 5;
    slot = 0;
    for (wtype = WEP_NULL + 1; wtype < WEP_TYPES_COUNT; wtype++)
    {
        TbBool has_weapon;

        if (slot >= 6)
            break;

        if (selected_agent == 4)
        {
          has_weapon = player_agent_has_weapon(local_player_no, 0, wtype)
            && player_agent_has_weapon(local_player_no, 1, wtype)
            && player_agent_has_weapon(local_player_no, 2, wtype)
            && player_agent_has_weapon(local_player_no, 3, wtype);
        }
        else
        {
          has_weapon = player_agent_has_weapon(local_player_no, selected_agent, wtype);
        }

        if (has_weapon)
        {
            show_weapon_slot(scr_x, scr_y, wtype);
            slot++;
            scr_y += 44;
        }
    }
    return 0;
}

void init_equip_screen_boxes(void)
{
    const char *text;
    ScrCoord scr_h, start_x, start_y;
    short space_w, space_h, border;

    // Border value represents how much the box background goes
    // out of the box area.
    border = 3;
#ifdef EXPERIMENTAL_MENU_CENTER_H
    scr_h = global_apps_bar_box.Y;
#else
    scr_h = 432;
#endif

    init_screen_text_box(&equip_list_head_box, 7u, 122u, 191u, 22,
      6, small_med_font, 1);
    init_screen_text_box(&equip_name_box, 425u, 122u, 208u, 22,
      6, small_med_font, 1);
    init_screen_box(&weapon_slots, 7u, 153u, 191u, 272, 6);
    init_screen_text_box(&equip_list_box, 207u, 122u, 209u, 303,
      6, small_med_font, 1);
    init_screen_text_box(&equip_display_box, 425u, 153u, 208u, 272,
      6, small_font, 3);
    init_screen_button(&equip_offer_buy_button, 430u, 404u,
      gui_strings[436], 6, med2_font, 1, 0);
    init_screen_info_box(&equip_cost_box, 504u, 404u, 124u,
      gui_strings[442], misc_text[0], 6, med_font, small_med_font, 1);
    weapon_slots.SpecialDrawFn = show_weapon_slots;
    equip_name_box.DrawTextFn = ac_show_weapon_name;
    equip_name_box.Text = unkn41_text;
    equip_name_box.Font = med_font;

    equip_cost_box.Text2 = equip_cost_text;
    equip_display_box.DrawTextFn = ac_display_weapon_info;
    equip_display_box.Flags |= (GBxFlg_RadioBtn|GBxFlg_IsMouseOver);
    equip_display_box.ScrollWindowHeight = 117;

    equip_list_head_box.DrawTextFn = ac_show_title_box;
    equip_list_head_box.Text = gui_strings[408];
    equip_list_head_box.Font = med_font;

    equip_list_box.ScrollWindowOffset += 27;
    equip_list_box.DrawTextFn = ac_show_weapon_list;
    equip_list_box.Flags |= (GBxFlg_RadioBtn|GBxFlg_IsMouseOver);
    equip_list_box.LineHeight = fepanel_sprites[15].SHeight + 3;
    equip_list_box.ScrollWindowHeight -= 27;

    lbFontPtr = med2_font;
    if (my_string_width(gui_strings[436]) <= my_string_width(gui_strings[407]))
        text = gui_strings[407];
    else
        text = gui_strings[436];
    equip_offer_buy_button.Width = my_string_width(text) + 4;
    equip_offer_buy_button.CallBackFn = ac_do_equip_offer_buy;

    init_screen_button(&equip_all_agents_button, 7u, 96u,
      gui_strings[534], 6, med2_font, 1, 0);
    equip_all_agents_button.Width = 165;
    equip_all_agents_button.RadioValue = 4;
    equip_all_agents_button.Flags |= GBxFlg_RadioBtn;
    equip_all_agents_button.Radio = (ubyte *)&selected_agent;

    // Reposition the components to current resolution

    start_x = heading_box.X;
    // On the X axis, we're going for aligning below heading box, to both left and right
    space_w = heading_box.Width - weapon_slots.Width - equip_list_box.Width - equip_name_box.Width;

    start_y = heading_box.Y + heading_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    // One re-used box - cyborg name - does not exist as global instance, so count all agents button twice.
    space_h = scr_h - start_y - 2 * equip_all_agents_button.Height - equip_list_box.Height + border;

    // On the X axis, aligning to heading box left
    equip_all_agents_button.X = start_x;
    // Agent name field has no global box - assume it has the same height as equip_all_agents_button,
    // so add that height to Y-position of equip_all_agents_button. Aleso move the box a tiny bit up from
    // the normal position grid.
    equip_all_agents_button.Y = start_y + equip_all_agents_button.Height + 2 * space_h / 4 - space_h / 24;

    equip_list_head_box.X = start_x;
    equip_list_head_box.Y = start_y + 2 * equip_all_agents_button.Height + 3 * space_h / 4;

    weapon_slots.X = start_x;

    equip_list_box.X = weapon_slots.X + weapon_slots.Width + space_w / 2;
    equip_list_box.Y = equip_list_head_box.Y;

    weapon_slots.Y = equip_list_box.Y + equip_list_box.Height - weapon_slots.Height;

    equip_name_box.X = equip_list_box.X + equip_list_box.Width + space_w - space_w / 2;
    equip_name_box.Y = equip_list_box.Y;

    equip_display_box.X = equip_name_box.X;
    equip_display_box.Y = equip_list_box.Y + equip_list_box.Height - equip_display_box.Height;

    // Boxes defining areas done; now reposition components inside

    space_w = 5;
    space_h = 5;
    equip_offer_buy_button.X = equip_display_box.X + space_w;
    equip_offer_buy_button.Y = equip_display_box.Y + equip_display_box.Height - space_h - equip_offer_buy_button.Height;
    // No need to update equip_cost_box - that is done in switch_shared_equip_screen_buttons_to_equip()
}

void switch_shared_equip_screen_buttons_to_equip(void)
{
    short space_w, space_h;

    space_w = 5;
    space_h = 5;
    set_heading_box_text(gui_strings[370]);
    refresh_equip_list = 1;

    equip_cost_box.Width = equip_display_box.Width - equip_offer_buy_button.Width - 3 * space_w - 1;
    equip_cost_box.X = equip_display_box.X + equip_display_box.Width - (space_w - 1) - equip_cost_box.Width;
    equip_cost_box.Y = equip_display_box.Y + equip_display_box.Height - space_h - equip_cost_box.Height;

    equip_all_agents_button.CallBackFn = ac_do_equip_all_agents_set;

    equip_display_box_redraw(&equip_display_box);
    equip_name_box_redraw(&equip_name_box);
}

void init_equip_screen_shapes(void)
{
    ushort i;
    short x, y;
    short scr_w, start_x;

    scr_w = lbDisplay.GraphicsWindowWidth;

    start_x = (scr_w - 640) / 2;

    x = 178 + start_x;
    y = 72;
    for (i = 0; i < 4; i++)
    {
        struct ScreenShape *p_shp;

        p_shp = &equip_agent_select_shapes[i];
        init_screen_shape(p_shp, x, y,
          agent_panel_shape_points_x, agent_panel_shape_points_y,
          sizeof(agent_panel_shape_points_x)/sizeof(agent_panel_shape_points_x[0]),
          0x0100, 0x0100, 6);
        p_shp->Colour = 0xAE;
        x += 110;
    }
    x = 7 + start_x;
    y = 72;
    i = 4;
    {
        struct ScreenShape *p_shp;

        p_shp = &equip_agent_select_shapes[i];
        init_screen_shape(p_shp, x, y,
          agent_name_shape_points_x, agent_name_shape_points_y,
          sizeof(agent_name_shape_points_x)/sizeof(agent_name_shape_points_x[0]),
          0x0100, 0x0100, 6);
        p_shp->Colour = 0x0F7;
    }
}

void reset_equip_screen_player_state(void)
{
    selected_weapon = -1;
    selected_agent = 0;
    refresh_equip_list = 0;
}

void reset_equip_screen_boxes_flags(void)
{
    short i;

    equip_list_head_box.Flags = GBxFlg_Unkn0001;
    weapon_slots.Flags = GBxFlg_Unkn0001;
    equip_name_box.Flags = GBxFlg_Unkn0001;
    equip_cost_box.Flags = GBxFlg_Unkn0001;
    equip_list_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
    equip_display_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
    for (i = 0; i < 5; i++) {
        equip_agent_select_shapes[i].Flags = GBxFlg_Unkn0001;
    }
}

void set_flag01_equip_screen_boxes(void)
{
    equip_all_agents_button.Flags |= GBxFlg_Unkn0001;
    equip_offer_buy_button.Flags |= GBxFlg_Unkn0001;
    if (screentype == SCRT_CRYO)
        equip_cost_box.Flags |= GBxFlg_NoBkCopy;
}

/******************************************************************************/
