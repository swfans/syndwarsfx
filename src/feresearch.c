/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feresearch.c
 *     Front-end desktop and menu system, research screen.
 * @par Purpose:
 *     Implement functions for research screen in front-end desktop.
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
#include "feresearch.h"

#include "bffont.h"
#include "bftext.h"
#include "bfsprite.h"
#include "bfstrut.h"

#include "cybmod.h"
#include "display.h"
#include "femain.h"
#include "game_data.h"
#include "game_options.h"
#include "game_sprts.h"
#include "game.h"
#include "guiboxes.h"
#include "guigraph.h"
#include "guitext.h"
#include "keyboard.h"
#include "mydraw.h"
#include "purpldrw.h"
#include "player.h"
#include "weapon.h"
#include "research.h"
#include "swlog.h"
#include "util.h"
/******************************************************************************/
struct ScreenTextBox research_unkn21_box = {0};
struct ScreenButton research_submit_button = {0};
struct ScreenButton unkn12_WEAPONS_MODS_button = {0};
struct ScreenTextBox research_progress_button = {0};
struct ScreenBox research_graph_box = {0};
struct ScreenButton research_list_buttons[2] = {0};

extern ubyte research_completed;// = 0;
extern ubyte research_on_weapons;// = true;
extern ubyte research_unkn_var_01;
extern sbyte research_selected_wep; // = -1;
extern sbyte research_selected_mod; // = -1;
extern ubyte byte_1551E4[5];

/******************************************************************************/

ubyte ac_do_research_submit(ubyte click);
ubyte ac_do_research_suspend(ubyte click);
ubyte ac_do_unkn12_WEAPONS_MODS(ubyte click);
ubyte ac_show_unkn21_box(struct ScreenTextBox *box);

TbBool research_weapon_daily_progress(void)
{
    short prev, lost;

    prev = research.CurrentWeapon;
    lost = research_daily_progress_for_type(0);
    scientists_lost += lost;
    if (research.CurrentWeapon != prev)
        new_weapons_researched |= 1 << prev;

    return (lost != 0) || (research.CurrentWeapon != prev);
}

TbBool research_cybmod_daily_progress(void)
{
    short prev, lost;

    prev = research.CurrentMod;
    lost = research_daily_progress_for_type(1);
    scientists_lost += lost;
    if (research.CurrentMod != prev)
        new_mods_researched |= 1 << prev;

    return (lost != 0) || (research.CurrentWeapon != prev);
}

/** If an agent in Cryo Chamber owns a weapon, allow its research.
 */
void research_allow_weapons_in_cryo(void)
{
#if 0
    asm volatile ("call ASM_research_allow_weapons_in_cryo\n"
        :  :  : "eax" );
#endif
    short plagent;
    WeaponType wtype;

    for (plagent = 0; plagent < AGENTS_SQUAD_MAX_COUNT; plagent++)
    {
        for (wtype = WEP_NULL + 1; wtype < WEP_TYPES_COUNT; wtype++)
        {
            if (weapons_has_weapon(cryo_agents.Weapons[plagent], wtype) && !is_research_weapon_completed(wtype))
                research_weapon_allow(wtype);
        }
    }
}

void forward_research_progress_after_mission(int num_days)
{
    int i;

    // TODO clear the data after filling research report, not here - there may be items accumulated
    // by time progress while waiting in menu; also clear on game load and new game
    new_mods_researched = 0;
    new_weapons_researched = 0;
    scientists_lost = 0;
    for (i = 0; i < num_days; i++)
    {
        research_weapon_daily_progress();
        research_cybmod_daily_progress();
    }
    research_allow_weapons_in_cryo();
}

void switch_research_screen_boxes_weapons_mods(void)
{
    const char *text;

    if (research_on_weapons)
    {
        unkn12_WEAPONS_MODS_button.Text = gui_strings[451];
        if (research.CurrentWeapon == -1)
        {
            text = gui_strings[417];
            research_submit_button.CallBackFn = ac_do_research_submit;
        }
        else
        {
            text = gui_strings[418];
            research_submit_button.CallBackFn = ac_do_research_suspend;
        }
    }
    else
    {
        unkn12_WEAPONS_MODS_button.Text = gui_strings[450];
        if (research.CurrentMod == -1)
        {
            text = gui_strings[417];
            research_submit_button.CallBackFn = ac_do_research_submit;
        }
        else
        {
            text = gui_strings[418];
            research_submit_button.CallBackFn = ac_do_research_suspend;
        }
    }
    research_submit_button.Text = text;
    research_unkn21_box.Lines = 0;
}

ubyte do_research_submit(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_research_submit\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (research_on_weapons)
    {
        if (research_selected_wep != -1)
        {
            research.CurrentWeapon = research_selected_wep;
            research_selected_wep = -1;
            research_curr_wep_date = global_date;
            research_curr_wep_date.Minute = global_date.Minute - 1;

            switch_research_screen_boxes_weapons_mods();
            return 1;
        }
    }
    {
        if (research_selected_mod != -1)
        {
            research.CurrentMod = research_selected_mod;
            research_selected_mod = -1;
            research_curr_mod_date = global_date;
            research_curr_mod_date.Minute = global_date.Minute - 1;

            switch_research_screen_boxes_weapons_mods();
            return 1;
        }
    }
    return 0;
}

ubyte do_research_suspend(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_research_suspend\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (research_on_weapons)
    {
        research.CurrentWeapon = -1;
    }
    else
    {
        research.CurrentMod = -1;
    }
    research_submit_button.CallBackFn = ac_do_research_submit;
    research_submit_button.Text = gui_strings[417];
    return 0;
}

ubyte do_unkn12_WEAPONS_MODS(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_unkn12_WEAPONS_MODS\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    research_on_weapons = (research_on_weapons == 0);
    research_selected_mod = -1;
    research_selected_wep = -1;
    switch_research_screen_boxes_weapons_mods();
    return 1;
}

ubyte show_unkn21_box(struct ScreenTextBox *p_box)
{
    char locstr[32];
    const char *text;
    short scr_x, scr_y;
    short tx_width, tx_height, ln_height;
    short i, line;

    if ((p_box->Flags & 0x8000) == 0)
    {
        short box_w;
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X + 4, p_box->Y + 26, 188u, 16u, 56);
        draw_box_purple_list(text_window_x1, text_window_y1,
          text_window_x2 - text_window_x1 + 1, text_window_y2 - text_window_y1 + 1, 56);
        draw_box_purple_list(p_box->X + 4, p_box->Y + 263, 200u, 18u, 243);
        draw_box_purple_list(p_box->X + 4, p_box->Y + 297, 200u, 18u, 243);
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X + 5, p_box->Y + 264, 198u, 16u, 174);
        draw_box_purple_list(p_box->X + 5, p_box->Y + 298, 198u, 16u, 174);
        lbDisplay.DrawFlags = 0;
        my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
        lbFontPtr = med_font;

        box_w = p_box->Width - 8;

        text = gui_strings[448];
        tx_width = my_string_width(text);
        draw_text_purple_list2((box_w - tx_width) >> 1, 2, text, 0);

        text = gui_strings[410];
        tx_width = my_string_width(text);
        draw_text_purple_list2((box_w - tx_width) >> 1, 247, text, 0);

        text = gui_strings[411];
        tx_width = my_string_width(text);
        draw_text_purple_list2((box_w - tx_width) >> 1, 281, text, 0);

        p_box->Flags |= 0x8000;
        copy_box_purple_list(p_box->X + 4, p_box->Y - 3, p_box->Width - 20, 0xE6u);
        copy_box_purple_list(p_box->X + 4, p_box->Y + 251, p_box->Width - 8, 0x64u);
        my_set_text_window(p_box->X + 4, p_box->ScrollWindowOffset + p_box->Y + 4,
          p_box->Width - 20, p_box->ScrollWindowHeight);
        lbFontPtr = small_med_font;
    }

    scr_y = 3;
    tx_height = my_char_height('A');
    line = p_box->TextTopLine;
    ln_height = tx_height + 4;
    while (line < 32)
    {
        if (scr_y + tx_height >= p_box->ScrollWindowHeight)
          break;

        if (research_on_weapons)
        {
          if (is_research_weapon_allowed(line + 1))
          {
              if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y - 2,
                text_window_x2, text_window_y1 + tx_height + scr_y + 2))
              {
                  if (lbDisplay.LeftButton)
                  {
                      lbDisplay.LeftButton = 0;
                      research_selected_wep = line;
                      if (research.CurrentWeapon == line) {
                          text = gui_strings[418];
                          research_submit_button.CallBackFn = ac_do_research_suspend;
                      } else {
                          research_submit_button.CallBackFn = ac_do_research_submit;
                          text = gui_strings[417];
                      }
                      research_submit_button.Text = text;
                  }
              }
              if (research_selected_wep == line)
              {
                  lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
                  lbDisplay.DrawColour = 87;
              }
              else
              {
                  lbDisplay.DrawFlags = 0;
              }
              lbDisplay.DrawFlags |= 0x8000;
              if (background_type == 1)
                  text = gui_strings[30 + line];
              else
                  text = gui_strings[0 + line];
              draw_text_purple_list2(3, scr_y, text, 0);
              lbDisplay.DrawFlags = 0;
              scr_y += ln_height;
          }
        }
        else if (is_research_cymod_allowed(line + 1))
        {
            short mtype, mlev;

            if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y - 2,
              text_window_x2, text_window_y1 + tx_height + scr_y + 2))
            {
                if (lbDisplay.LeftButton)
                {
                    lbDisplay.LeftButton = 0;
                    research_selected_mod = line;
                    if (research.CurrentMod == line)
                    {
                        text = gui_strings[418];
                        research_submit_button.CallBackFn = ac_do_research_suspend;
                    }
                    else
                    {
                        text = gui_strings[417];
                        research_submit_button.CallBackFn = ac_do_research_submit;
                    }
                    research_submit_button.Text = text;
                }
            }
            if (research_selected_mod == line)
            {
                lbDisplay.DrawFlags |= 0x0040;
                lbDisplay.DrawColour = 87;
            }
            else
            {
                lbDisplay.DrawFlags = 0;
            }

            if (line == 15) {
                mtype = 4;
                mlev = line - 11;
            } else {
                mtype = line / 3;
                mlev = line % 3 + 1;
            }
            text = gui_strings[70 + byte_1551E4[mtype]];
            draw_text_purple_list2(3, scr_y + 1, text, 0);

            lbDisplay.DrawFlags |= 0x0080;
            if ((1 << line < 4096) || (1 << line > 0x8000)) {
                sprintf(locstr, "%s %d", gui_strings[76], (int)mlev);
            } else {
                sprintf(locstr, "%s %d", gui_strings[75], (int)mlev);
            }
            text = loctext_to_gtext(locstr);
            lbDisplay.DrawFlags |= 0x8000;
            draw_text_purple_list2(-1, scr_y + 1, text, 0);
            lbDisplay.DrawFlags = 0;
            scr_y += tx_height + p_box->LineSpacing;
        }
        ++line;
    }

    lbDisplay.DrawFlags = 0;
    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    lbFontPtr = med_font;
    if (research_on_weapons)
    {
        if (research.CurrentWeapon != -1)
        {
            if (background_type == 1)
                text = gui_strings[30 + research.CurrentWeapon];
            else
                text = gui_strings[0 + research.CurrentWeapon];
            draw_text_purple_list2(4, 25, text, 0);
        }
    }
    else
    {
        if (research.CurrentMod != -1)
        {
            short mtype, mlev;

            if (research.CurrentMod == 15) {
                mtype = 4;
                mlev = research.CurrentMod - 11;
            } else {
                mtype = research.CurrentMod / 3;
                mlev = research.CurrentMod % 3 + 1;
            }
            text = gui_strings[70 + byte_1551E4[mtype]];
            if ( 1 << research.CurrentMod < 4096 || 1 << research.CurrentMod > 0x8000 )
                sprintf(locstr, "%s %s %d", text, gui_strings[76], mlev);
            else
                sprintf(locstr, "%s %s %d", text, gui_strings[75], mlev);
            text = loctext_to_gtext(locstr);
            draw_text_purple_list2(4, 25, text, 0);
        }
    }

    if ((research_on_weapons && research.CurrentWeapon != -1)
      || (!research_on_weapons && research.CurrentMod != -1))
    {
        if (research_on_weapons)
            sprintf(locstr, "%ld", research.WeaponFunding);
        else
            sprintf(locstr, "%ld", research.ModFunding);
        text = loctext_to_gtext(locstr);
        tx_width = LbTextStringWidth(text);
        scr_x = (100 - tx_width - 5) >> 1;
        lbDisplay.DrawFlags |= 0x8000;
        draw_text_purple_list2(scr_x, 263, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;
        lbFontPtr = small_font;
        draw_text_purple_list2(scr_x + tx_width, 268, misc_text[1], 0);

        lbFontPtr = med_font;
        tx_width = LbSprFontCharWidth(lbFontPtr, '/');
        draw_text_purple_list2((200 - tx_width) >> 1, 263, misc_text[2], 0);

        if (research_on_weapons)
            sprintf(locstr, "%d", 100 * weapon_defs[research.CurrentWeapon + 1].Funding);
        else
            sprintf(locstr, "%d", 100 * mod_defs[research.CurrentMod + 1].Funding);
        text = loctext_to_gtext(locstr);
        tx_width = LbTextStringWidth(text);
        scr_x = ((100 - tx_width - 5) >> 1) + 100;
        draw_text_purple_list2(scr_x, 263, text, 0);
        lbFontPtr = small_font;
        draw_text_purple_list2(scr_x + tx_width, 268, misc_text[1], 0);

        lbFontPtr = med_font;

        if (mouse_down_over_box_coords(p_box->X + 5, p_box->Y + 262,
          p_box->X + 203, p_box->Y + 278))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                if (research_on_weapons)
                {
                    research.WeaponFunding += 1000;
                }
                else
                {
                    research.ModFunding += 1000;
                }
            }
            else if (lbDisplay.RightButton)
            {
                lbDisplay.RightButton = lbDisplay.LeftButton;
                if (research_on_weapons)
                {
                    research.WeaponFunding -= 1000;
                    if (research.WeaponFunding < 0)
                      research.WeaponFunding = 0;
                }
                else
                {
                    research.ModFunding -= 1000;
                    if (research.ModFunding < 0)
                      research.ModFunding = 0;
                }
            }
        }
    }

    {
        sprintf(locstr, "%d", research.Scientists);
        scr_x = (200 - LbTextStringWidth(locstr)) >> 1;
        text = loctext_to_gtext(locstr);
        draw_text_purple_list2(scr_x, 297, text, 0);
        text = gui_strings[535];
        tx_width = my_string_width(text);
        draw_text_purple_list2((p_box->Width - 8 - tx_width) >> 1, 315, text, 0);
    }

    for (i = 0; i < 2; i++)
    {
        struct ScreenButton *p_btn;
        p_btn = &research_list_buttons[i];
        p_btn->DrawFn(p_btn);
    }
    return 0;
}

void draw_unkn20_subfunc_01(int x, int y, char *text, ubyte a4)
{
#if 0
    asm volatile (
      "call ASM_draw_unkn20_subfunc_01\n"
        : : "a" (x), "d" (y), "b" (text), "c" (a4));
#endif
    int i;
    short scr_x, scr_y;

    if (text == NULL)
        return;

    scr_x = text_window_x1 + x;
    scr_y = text_window_y1 + y;
    i = 0;
    while ( 1 )
    {
        short dx, dy;
        ubyte ch;

        while ( 1 )
        {
            short w;

            ch = text[i++];
            if (ch == '\0')
              return;
            if (my_font_prefer_upper_case(lbFontPtr)) {
                ch = fontchrtoupper(ch);
            }
            w = LbSprFontCharWidth(lbFontPtr, ch);
            dx = w >> 1;
            if (ch != ' ')
                break;
            scr_y += 3 * a4;
        }

        if (lbFontPtr == small_font || lbFontPtr == small2_font)
        {
            dy = 1;
        }
        else if (lbFontPtr == small_med_font)
        {
            if (ch >= 'a' && ch <= 'z') {
                dy = 0;
            } else {
                dy = 2;
            }
        }
        else if (lbFontPtr == med_font || lbFontPtr == med2_font)
        {
            dy = 2;
        }
        else if (lbFontPtr == big_font)
        {
            dy = 4;
        }
        else
        {
            dy = 0;
        }
        draw_sprite_purple_list(scr_x - dx, scr_y - dy, LbFontCharSprite(lbFontPtr, ch));
        scr_y += a4 + my_char_height(ch);
    }
}

void show_research_screen(void)
{
#if 0
    asm volatile ("call ASM_show_research_screen\n"
        :  :  : "eax" );
#endif
    int i;
    ubyte drawn;

    if ((game_projector_speed && is_heading_flag01()) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_research_screen_boxes();
    }

    if (research_unkn21_box.Lines == 0)
    {
        if (research_on_weapons)
        {
            for (i = WEP_NULL + 1; i < WEP_TYPES_COUNT; i++)
            {
                if (is_research_weapon_allowed(i))
                    research_unkn21_box.Lines++;
            }
        }
        else
        {
            for (i = MOD_NULL + 1; i < MOD_TYPES_COUNT; i++)
            {
                if (is_research_cymod_allowed(i))
                    research_unkn21_box.Lines++;
            }
        }
        research_unkn21_box.Flags |= 0x0080;
    }

    drawn = 1;
    if (drawn) {
        drawn = draw_heading_box();
    }
    if (drawn) {
        drawn = research_progress_button.DrawFn(&research_progress_button);
    }
    if (drawn) {
        drawn = research_graph_box.DrawFn(&research_graph_box);
    }
    if (drawn) {
        drawn = research_unkn21_box.DrawFn(&research_unkn21_box);
    }

    if ((ingame.UserFlags & UsrF_Cheats) != 0)
    {
        if (is_key_pressed(KC_U, KMod_DONTCARE))
        {
            clear_key_pressed(KC_U);
            research_daily_progress_for_type(0);
            research_daily_progress_for_type(1);
        }
        if (is_key_pressed(KC_0, KMod_DONTCARE))
        {
            clear_key_pressed(KC_0);
            if (research_completed + 1 < MOD_TYPES_COUNT)
            {
                refresh_equip_list = 1;
                research_cymod_allow(research_completed + 1);
            }
            if (research_completed + 1 < WEP_TYPES_COUNT)
            {
                refresh_equip_list = 1;
                research_weapon_allow(research_completed + 1);
                research_completed++;
            }
        }
    }
}

ubyte show_research_graph(struct ScreenBox *box)
{
    char *text;
    ushort *y_vals;
    ushort y_trend_delta;
    int n_y_vals;
    int graph_days, done_days;
    int x, y, w, h;

    // Make actual graph grid not larger than x=80%/y=75% of the box,
    // and with exact same grid squares
    w = box->Width * 80 / 100;
    w -= w % 10; // for original resolution it's 320
    h = box->Height * 75 / 100;
    h -= h % 10; //  // for original resolution it's 240
    // Prepare position where the graph area starts (the actual graph grid)
    x = box->X + (box->Width - w) * 65 / 100; // for original res 7 + 57
    y = box->Y + (box->Height - h) * 31 / 100; // for original res 103 + 25

    if ((box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        int twidth;

        lbFontPtr = small_med_font;
        my_set_text_window(0, 0, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);

        draw_chartxy_axis_y_values(x, y, h, 0, 100, 10);

        draw_chartxy_axis_y_grid(x, y, w, h, 10);
        draw_chartxy_axis_x_grid(x, y, w, h, 10);

        draw_text_purple_list2(x - 12, y + h + 6, misc_text[0], 0);
        lbFontPtr = med_font;
        my_set_text_window(box->X + 4, box->Y + 4, box->Width - 8,  box->Height - 8);
        text = gui_strings[453];
        twidth = my_string_width(text);
        draw_text_purple_list2((box->Width - 8 - twidth) >> 1, 290, text, 0);
        text = gui_strings[452];
        draw_unkn20_subfunc_01(10, 31, text, 2);

        box->Flags |= GBxFlg_BkgndDrawn;
        copy_box_purple_list(box->X, box->Y, box->Width, box->Height);
    }

    graph_days = 0;
    if (research_on_weapons)
        done_days = research.WeaponDaysDone[research.CurrentWeapon];
    else
        done_days = research.ModDaysDone[research.CurrentMod];
    if (done_days >= 10) {
        graph_days = done_days - 9;
        n_y_vals = 10;
    } else {
        n_y_vals = done_days + 1;
    }
    lbFontPtr = small_med_font;
    my_set_text_window(0, 0, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    draw_chartxy_axis_x_values(x, y + h, w, graph_days+1, graph_days+11, 10);

    LbScreenSetGraphicsWindow(x - 1, y, w + 3, h + 2);
    if (research_on_weapons)
    {
        if (research.CurrentWeapon != -1)
        {
            struct WeaponDef *wdef;

            y_vals = &research.WeaponProgress[research.CurrentWeapon][0];
            wdef = &weapon_defs[research.CurrentWeapon + 1];
            y_trend_delta = research_unkn_func_004(wdef->PercentPerDay, wdef->Funding, research.WeaponFunding);

            draw_chartxy_curve(1, 0, w, h, y_vals, n_y_vals, RESEARCH_COMPLETE_POINTS, y_trend_delta, 10);
        }
    }
    else
    {
        if (research.CurrentMod != -1)
        {
            struct ModDef *mdef;

            y_vals = &research.ModProgress[research.CurrentMod][0];
            mdef = &mod_defs[research.CurrentMod + 1];
            y_trend_delta = research_unkn_func_004(mdef->PercentPerDay, mdef->Funding, research.ModFunding);

            draw_chartxy_curve(1, 0, w, h, y_vals, n_y_vals, RESEARCH_COMPLETE_POINTS, y_trend_delta, 10);
        }
    }

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    draw_chartxy_axis_x_main(x, y + h, w);
    draw_chartxy_axis_y_main(x, y, h);
    return 0;
}

#define SCROLL_BAR_WIDTH 12

void init_research_screen_boxes(void)
{
    int i, val;
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

    init_screen_box(&research_graph_box, 7u, 103u, 409u, 322, 6);
    init_screen_text_box(&research_progress_button, 7u, 72u, 409u, 23,
      6, med_font, 1);
    init_screen_text_box(&research_unkn21_box, 425u, 72u, 208u, 353,
      6, small_med_font, 3);

    init_screen_button(&research_submit_button, 430u, 302u,
      gui_strings[418], 6, med2_font, 1, 0);
    init_screen_button(&unkn12_WEAPONS_MODS_button, 616u, 302u,
        gui_strings[450], 6, med2_font, 1, 0x80);

    init_screen_button(&research_list_buttons[0], 425u, 404u,
     gui_strings[478], 6, med2_font, 1, 0);
    init_screen_button(&research_list_buttons[1], 425u, 404u,
      gui_strings[479], 6, med2_font, 1, 0);

    val = 0;
    for (i = 0; i < 2; i++)
    {
        research_list_buttons[i].Radio = &ingame.AutoResearch;
        research_list_buttons[i].RadioValue = val;
        research_list_buttons[i].Flags |= GBxFlg_RadioBtn;
        val++;
    }

    unkn12_WEAPONS_MODS_button.CallBackFn = ac_do_unkn12_WEAPONS_MODS;
    unkn12_WEAPONS_MODS_button.Text = gui_strings[451];

    research_unkn21_box.DrawTextFn = show_unkn21_box;
    research_unkn21_box.Buttons[0] = &research_submit_button;
    research_unkn21_box.Buttons[1] = &unkn12_WEAPONS_MODS_button;
    research_unkn21_box.Flags |= GBxFlg_RadioBtn|GBxFlg_IsMouseOver;
    research_submit_button.Text = gui_strings[417];
    research_submit_button.CallBackFn = ac_do_research_submit;
    research_progress_button.DrawTextFn = show_title_box;
    research_progress_button.Text = gui_strings[449];

    research_graph_box.SpecialDrawFn = show_research_graph;

    research_unkn21_box.ScrollWindowHeight = 180;
    research_unkn21_box.ScrollWindowOffset += 41;

    lbFontPtr = med2_font;

    if (my_string_width(gui_strings[418]) <= my_string_width(gui_strings[417]))
        text = gui_strings[417];
    else
        text = gui_strings[418];
    research_submit_button.Width = my_string_width(text) + 4;

    if (my_string_width(gui_strings[451]) <= my_string_width(gui_strings[450]))
        text = gui_strings[450];
    else
        text = gui_strings[451];
    unkn12_WEAPONS_MODS_button.Width = my_string_width(text) + 4;

    // Reposition the components to current resolution

    start_x = heading_box.X;
    // On the X axis, we're going for aligning below heading box, to both left and right
    space_w = heading_box.Width - research_graph_box.Width - research_unkn21_box.Width;

    start_y = heading_box.Y + heading_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    // One re-used box - cyborg name - does not exist as global instance, so count all agents button twice.
    space_h = scr_h - start_y - research_unkn21_box.Height + border;

    research_graph_box.X = start_x;
    research_progress_button.X = start_x;
    research_progress_button.Y = start_y + space_h / 2;
    research_unkn21_box.X = research_graph_box.X + research_graph_box.Width + space_w;
    research_unkn21_box.Y = research_progress_button.Y;
    research_graph_box.Y = research_unkn21_box.Y + research_unkn21_box.Height - research_graph_box.Height;

    space_w = 5;
    space_h = 5;
    research_submit_button.X = research_unkn21_box.X + space_w;
    research_submit_button.Y = research_unkn21_box.Y +
      research_unkn21_box.ScrollWindowOffset + research_unkn21_box.ScrollWindowHeight + 9;
    unkn12_WEAPONS_MODS_button.X = research_unkn21_box.X + research_unkn21_box.Width -
      SCROLL_BAR_WIDTH - space_w - unkn12_WEAPONS_MODS_button.Width;
    unkn12_WEAPONS_MODS_button.Y = research_submit_button.Y;

    // Middle of first half od the panel
    research_list_buttons[0].X = research_unkn21_box.X +
      (research_unkn21_box.Width / 2 - research_list_buttons[0].Width) / 2;
    research_list_buttons[0].Y = research_unkn21_box.Y + research_unkn21_box.Height -
      research_list_buttons[0].Height - space_h;
    // Middle of 2nd half od the panel
    research_list_buttons[1].X = research_unkn21_box.X + research_unkn21_box.Width -
      research_unkn21_box.Width / 2 + (research_unkn21_box.Width / 2 - research_list_buttons[1].Width) / 2;
    research_list_buttons[1].Y = research_list_buttons[0].Y;
}

void reset_research_screen_player_state(void)
{
    research_on_weapons = 1;
    research_selected_mod = -1;
    research_selected_wep = -1;
    switch_research_screen_boxes_weapons_mods();
}

void reset_research_screen_boxes_flags(void)
{
    research_unkn21_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
    research_graph_box.Flags = GBxFlg_Unkn0001;
    research_progress_button.Flags = GBxFlg_Unkn0001;
}

void set_flag01_research_screen_boxes(void)
{
    research_submit_button.Flags |= GBxFlg_Unkn0001;
    research_list_buttons[1].Flags |= GBxFlg_Unkn0001;
    research_list_buttons[0].Flags |= GBxFlg_Unkn0001;
    unkn12_WEAPONS_MODS_button.Flags |= GBxFlg_Unkn0001;
}

void skip_flashy_draw_research_screen_boxes(void)
{
    skip_flashy_draw_heading_screen_boxes();
    research_progress_button.Flags |= GBxFlg_Unkn0002;
    research_submit_button.Flags |= GBxFlg_Unkn0002;
    research_unkn21_box.Flags |= GBxFlg_Unkn0002;
    research_graph_box.Flags |= GBxFlg_Unkn0002;
    research_list_buttons[0].Flags |= GBxFlg_Unkn0002;
    research_list_buttons[1].Flags |= GBxFlg_Unkn0002;
}

void clear_research_screen(void)
{
    research_unkn21_box.Lines = 0;
}

/******************************************************************************/
