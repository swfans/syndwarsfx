/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file felogin.c
 *     Front-end desktop and menu system, login screen.
 * @par Purpose:
 *     Implement functions for login screen in front-end desktop.
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
#include "felogin.h"

#include "bffont.h"
#include "bfkeybd.h"
#include "bftext.h"
#include "bfutility.h"
#include "ssampply.h"

#include "campaign.h"
#include "display.h"
#include "femail.h"
#include "femain.h"
#include "feoptions.h"
#include "game_options.h"
#include "game_save.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "guiboxes.h"
#include "guitext.h"
#include "keyboard.h"
#include "mydraw.h"
#include "purpldrw.h"
#include "sound.h"
#include "util.h"
#include "swlog.h"
/******************************************************************************/
struct ScreenBox login_campaigns_box = {0};
struct ScreenBox login_name_box = {0};
struct ScreenButton login_continue_button = {0};
struct ScreenButton login_abort_button = {0};

ubyte ac_do_abort_2(ubyte click);
ubyte ac_do_login_2(ubyte click);

ubyte do_login_2(ubyte click)
{
    if (strlen(login_name) == 0)
        return 0;
    strtocapwords(login_name);

    read_user_settings();

    update_options_screen_state();

    if (in_network_game)
    {
        screentype = SCRT_SYSMENU;
        //TODO why go to controls screen in net game? is it a mistake?
        game_system_screen = SySc_CONTROLS;
        reload_background_flag = 1;
        edit_flag = 0;
        return 1;
    }

    if ((ingame.Flags & GamF_MortalGame) != 0)
    {
        sysmnu_button_enable(1,2);
        ingame.Flags &= ~GamF_MortalGame;
    }

    campaign_new_game_prepare();

    if (new_mail)
        play_sample_using_heap(0, 119 + (LbRandomAnyShort() % 3), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);

    return 1;
}

ubyte do_abort_2(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_abort_2\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    redraw_screen_flag = 1;
    screentype = SCRT_MAINMENU;
    edit_flag = 0;
    return 1;
}

ubyte show_campaigns_list(struct ScreenBox *box)
{
    int campgn, ncampgns;
    struct Campaign *p_campgn;
    const char *text;
    int campgn_height, line_height, nlines;
    int box_width, box_height;
    int cy;

    lbFontPtr = small_med_font;
    box_width = box->Width - 8;
    box_height = box->Height - 8;
    my_set_text_window(box->X + 4, box->Y + 4, box_width, box_height);
    byte_197160 = 4;
    ncampgns = selectable_campaigns_count();
    campgn_height = box_height / (ncampgns + 1);
    cy = box_height / (ncampgns + 1);
    line_height = my_char_height('A');
    lbDisplay.DrawColour = 87;

    for (campgn = 0; campgn < ncampgns; campgn++)
    {
        int hbeg;

        p_campgn = &campaigns[campgn];
        text = gui_strings[p_campgn->TextId];
        nlines = my_count_lines(text);
        if (background_type == campgn)
            lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER|Lb_TEXT_ONE_COLOR;
        else
            lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        hbeg = cy - (4 * nlines - 4 + nlines * line_height) / 2;
        lbDisplay.DrawFlags |= 0x8000;
        text = gui_strings[GSTR_CPG_CAMPAIGN_NAME + 100 * campgn];
        draw_text_purple_list2(0, hbeg, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;
        cy += campgn_height;
    }

    cy = box_height / (ncampgns + 1);
    for (campgn = 0; campgn < ncampgns; campgn++)
    {
        int hbeg, hend;

        p_campgn = &campaigns[campgn];
        text = gui_strings[p_campgn->TextId];
        nlines = my_count_lines(text);
        hbeg = cy - (4 * nlines - 4 + nlines * line_height) / 2;
        hend = hbeg + (line_height + 4) * (nlines - 1) + line_height;
        if (lbDisplay.LeftButton)
        {
            short msy, msx;
            short y1, y2;
            msy = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MouseY : lbDisplay.MouseY;
            msx = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MouseX : lbDisplay.MouseX;

            y1 = text_window_y1 + hbeg;
            y2 = text_window_y1 + hend;
            if (in_box_coords(msx, msy, text_window_x1, y1, text_window_x2, y2))
            {
                lbDisplay.LeftButton = 0;
                background_type = campgn;
            }
        }
        cy += campgn_height;
    }
    return 0;
}

ubyte show_login_name(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_login_name\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short scr_x;

    lbFontPtr = med2_font;
    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    scr_x = my_string_width(gui_strings[454]) + 2;
    if ((p_box->Flags & 0x8000) == 0)
    {
        lbFontPtr = med_font;
        draw_text_purple_list2(0, 2, gui_strings[454], 0);
        lbDisplay.DrawFlags = 4;
        draw_box_purple_list(scr_x + text_window_x1, text_window_y1, 0xF4u, 0xEu, 243);
        lbDisplay.DrawFlags = 0;
        copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
          p_box->Width + 6, p_box->Height + 6);
        lbFontPtr = med2_font;
        p_box->Flags |= 0x8000;
        reset_buffered_keys();
  }
    user_read_value(login_name, 0xFu, 1);
    draw_text_purple_list2(scr_x + 2, 2, login_name, 0);
    if (strcmp(login_name, "POOSLICE") == 0)
    {
        ingame.UserFlags |= UsrF_Cheats;
        play_sample_using_heap(0, 71, FULL_VOL, EQUL_PAN, NORM_PTCH, 0, 3u);
    }
    if ((gameturn & 1) != 0)
    {
        const struct TbSprite *p_spr;
        short tx_width;

        p_spr = LbFontCharSprite(lbFontPtr, 45);
        tx_width = my_string_width(login_name);
        if (p_spr != NULL)
            draw_sprite_purple_list(text_window_x1 + scr_x + 2 + tx_width, text_window_y1 + 6, p_spr);
    }
    return 0;
}

void skip_flashy_draw_login_screen_boxes(void)
{
    login_campaigns_box.Flags |= GBxFlg_Unkn0002;
    login_name_box.Flags |= GBxFlg_Unkn0002;
    login_abort_button.Flags |= GBxFlg_Unkn0002;
    login_continue_button.Flags |= GBxFlg_Unkn0002;
}

ubyte show_login_screen(void)
{
    ubyte drawn = true;

    if ((game_projector_speed && (login_name_box.Flags & GBxFlg_Unkn0001)) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_login_screen_boxes();
    }
    drawn = login_name_box.DrawFn(&login_name_box);
    drawn = login_campaigns_box.DrawFn(&login_campaigns_box);
    drawn = login_continue_button.DrawFn(&login_continue_button);
    drawn = login_abort_button.DrawFn(&login_abort_button);
    return drawn;
}

void init_login_screen_boxes(void)
{
    ScrCoord scr_w, scr_h;
    short border, ln_height, ncampgns;

    // Border value represents how much the box background goes
    // out of the box area.
    border = 3;
    scr_w = lbDisplay.GraphicsWindowWidth;
#ifdef EXPERIMENTAL_MENU_CENTER_H
    scr_h = lbDisplay.GraphicsWindowHeight;
#else
    scr_h = 480;
#endif

    init_screen_box(&login_campaigns_box, 219u, 159u, 200u, 100, 6);
    init_screen_box(&login_name_box, 150u, 128u, 337u, 22, 6);
    init_screen_button(&login_continue_button, 260u, 300u,
      gui_strings[455], 6, med2_font, 1, 0);
    init_screen_button(&login_abort_button, 260u, 329u,
      gui_strings[388], 6, med2_font, 1, 0);

    login_continue_button.CallBackFn = do_login_2;
    login_abort_button.CallBackFn = do_abort_2;
    login_campaigns_box.SpecialDrawFn = show_campaigns_list;
    login_name_box.SpecialDrawFn = show_login_name;

    login_continue_button.AccelKey = KC_RETURN;
    login_abort_button.AccelKey = KC_ESCAPE;

    // Reposition the components to current resolution

    login_abort_button.Border = border;
    login_continue_button.Border = border;

    lbFontPtr = med2_font;
    login_name_box.Width = my_string_width(gui_strings[454]) + 254;
    login_campaigns_box.X = (scr_w - login_campaigns_box.Width) / 2 - 1;
    login_name_box.X = (scr_w - login_name_box.Width) / 2 - 1;
    login_continue_button.X = (scr_w - login_continue_button.Width) / 2 - 1;
    login_abort_button.X = (scr_w - login_abort_button.Width) / 2  - 1;

    lbFontPtr = small_med_font;
    ln_height = my_char_height('A') * 5 / 4;
    ncampgns = selectable_campaigns_count();

    login_campaigns_box.Height = 2 * border + 2 * ln_height + 3 * ln_height * (ncampgns + 1);

    login_name_box.Y = scr_h * 4 / 16;
    login_campaigns_box.Y = login_name_box.Y + scr_h / 16;
    login_continue_button.Y = login_campaigns_box.Y + login_campaigns_box.Height + scr_h / 16;
    login_abort_button.Y = login_continue_button.Y + scr_h / 16;
}

void reset_login_screen_boxes_flags(void)
{
    login_name_box.Flags = GBxFlg_Unkn0001;
    login_campaigns_box.Flags = GBxFlg_Unkn0001;
}

void set_flag01_login_screen_boxes(void)
{
    login_continue_button.Flags |= GBxFlg_Unkn0001;
    login_abort_button.Flags |= GBxFlg_Unkn0001;
}

/******************************************************************************/
