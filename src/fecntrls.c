/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fecntrls.c
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
#include "fecntrls.h"

#include "bfkeybd.h"
#include "bftext.h"
#include "bfutility.h"
#include "bflib_joyst.h"
#include "ssampply.h"

#include "femain.h"
#include "feshared.h"
#include "guiboxes.h"
#include "guitext.h"
#include "display.h"
#include "game_data.h"
#include "game_save.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "keyboard.h"
#include "mydraw.h"
#include "network.h"
#include "player.h"
#include "purpldrw.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
struct ScreenBox controls_joystick_box = {0};
struct ScreenTextBox controls_list_box = {0};
struct ScreenButton controls_defaults_button = {0};
struct ScreenButton controls_save_button = {0};
struct ScreenButton controls_calibrate_button = {0};

extern ubyte byte_1C4970;
extern ubyte controls_hlight_gkey;

short sheet_columns_x[] = {4, 200, 300};

ubyte ac_do_controls_defaults(ubyte click);
ubyte ac_do_controls_save(ubyte click);
ubyte ac_do_controls_calibrate(ubyte click);
ubyte ac_show_menu_controls_list_box(struct ScreenTextBox *p_box);

/** Game key currently being edited in the controls screen.
 * Max value is 2x max GameKey index - because it also stored distinction
 * between entering keyboard key and joystick key.
 */
ubyte controls_edited_gkey = 0;

/******************************************************************************/

ubyte do_controls_defaults(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_controls_defaults\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    set_default_user_settings();
    return 1;
}

ubyte do_controls_save(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_controls_save\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    const char *msg_str;
    if (save_user_settings())
        msg_str = gui_strings[GSTR_CONTROLS_SAVED_FAIL];
    else
        msg_str = gui_strings[GSTR_CONTROLS_SAVED_SUCC];
    alert_box_text_fmt("%s", msg_str);
    return 1;
}

ubyte do_controls_calibrate(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_controls_calibrate\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    net_unkn_pos_02 = 1;
    alert_box_text_fmt("%s", gui_strings[GSTR_JOY_CAL_TOP_LEFT]);
    return 1;
}

TbBool is_defining_control_key(void)
{
    return (controls_hlight_gkey != 0);
}

ubyte show_controls_joystick_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_controls_joystick_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    char locstr[52];
    struct ScreenRect active_rect;
    PlayerInfo *p_locplayer;
    const char *text;
    short wpos_x, wpos_y;
    short ln_height;
    short tx_width;
    ubyte dmuser;
    int i;

    lbFontPtr = small_med_font;
    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    ln_height = my_char_height('A');

    if ((p_box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        lbFontPtr = med_font;
        lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
        text = gui_strings[GSTR_JOYSTICK_TYPE];
        draw_text_purple_list2(0, 110, text, 0);
        lbDisplay.DrawFlags &= ~Lb_TEXT_HALIGN_CENTER;

        lbFontPtr = small_med_font;
        wpos_x = 6;
        wpos_y = 10;
        text = gui_strings[GSTR_PLAYERS];
        draw_text_purple_list2(wpos_x, wpos_y, text, 0);
        wpos_y += ln_height + 8;

        for (i = 0; i < 4; i++)
        {
            sprintf(locstr, "%s %d", gui_strings[GSTR_PLAYER], i + 1);
            text = loctext_to_gtext(locstr);
            draw_text_purple_list2(wpos_x, wpos_y, text, 0);
            wpos_y += ln_height + 4;
        }

        copy_box_purple_list(p_box->X + 4, p_box->Y + 4,
          p_box->Width - 8, p_box->Height - 8);
        p_box->Flags |= GBxFlg_BkgndDrawn;

        lbFontPtr = small_med_font;
        ln_height = my_char_height('A');
    }
    wpos_y = 126;

    lbDisplay.DrawFlags |= 0x8000;
    if (ctl_joystick_type == JTyp_EXT_DRIVER)
    {
      if (joy_get_device_name(locstr) != -1)
      {
        text = loctext_to_gtext(locstr);
        tx_width = my_string_width(locstr);
        wpos_x = (p_box->Width - tx_width) / 2;
        draw_text_purple_list2(wpos_x, wpos_y, text, 0);
      }
    }
    else
    {
        text = gui_strings[539 + ctl_joystick_type];
        tx_width = my_string_width(text);
        wpos_x = (p_box->Width - tx_width) / 2;
        draw_text_purple_list2(wpos_x, wpos_y, text, 0);
    }
    lbDisplay.DrawFlags &= ~0x8000;

    active_rect.X = text_window_x1 + wpos_x;
    active_rect.Width = tx_width;
    active_rect.Y = text_window_y1 + wpos_y;
    active_rect.Height = ln_height;
    if (mouse_down_over_box(&active_rect))
    {
        if (lbDisplay.LeftButton)
        {
            sbyte v23;
            ubyte v24;

            if (ctl_joystick_type != JTyp_NONE)
                joy_refresh_devices(&joy);
            lbDisplay.LeftButton = 0;

            v23 = -1;
            v24 = ctl_joystick_type;
            while (v23 != 1)
            {
                if (++ctl_joystick_type >= JTyp_TYPES_COUNT)
                    ctl_joystick_type = JTyp_ANALG_2BTN; // first one
                if (v24 == ctl_joystick_type)
                {
                    v23 = 1;
                    ctl_joystick_type = JTyp_NONE;
                }
                if (unkn01_maskarr[ctl_joystick_type])
                    v23 = joy_setup_device(&joy, ctl_joystick_type);
                if (!v24)
                    v24 = 1;
            }
        }
    }
    p_locplayer = &players[local_player_no];

    wpos_x = 140;
    wpos_y = 10;

    sprintf(locstr, "%d", p_locplayer->DoubleMode + 1);
    lbDisplay.DrawFlags |= 0x8000;
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(wpos_x, wpos_y, text, 0);
    tx_width = LbTextStringWidth(locstr);
    lbDisplay.DrawFlags &= ~0x8000;

    active_rect.X = text_window_x1 + wpos_x;
    active_rect.Width = tx_width;
    active_rect.Y = text_window_y1 + wpos_y;
    active_rect.Height = ln_height;
    if (mouse_down_over_box(&active_rect))
    {
        if (lbDisplay.LeftButton)
        {
            lbDisplay.LeftButton = 0;
            if (login_control__State != LognCt_Unkn5 || nsvc.I.Type == NetSvc_IPX)
            {
                p_locplayer->DoubleMode++;
                if (p_locplayer->DoubleMode > 3)
                    p_locplayer->DoubleMode = 0;
            }
            else
            {
                p_locplayer->DoubleMode = 0;
            }

            for (dmuser = p_locplayer->DoubleMode + 1; dmuser < 4; dmuser++)
            {
                p_locplayer->UserInput[dmuser].ControlMode = UInpCtr_Mouse;
            }
        }
    }

    wpos_x = 100;
    wpos_y = ln_height + 18;
    lbDisplay.DrawFlags |= 0x8000;

    active_rect.Y = text_window_y1 + wpos_y;
    active_rect.Height = ln_height;
    for (dmuser = 0; dmuser < p_locplayer->DoubleMode + 1; dmuser++)
    {
        ushort ctlmode;

        ctlmode = p_locplayer->UserInput[dmuser].ControlMode;
        if (ctlmode >= UInpCtr_Joystick0)
        {
            int n_found;

            i = 0;
            n_found = 0;
            while (n_found < ctlmode - 1)
            {
                if (joy.Init[i])
                    n_found++;
                i++;
            }
            sprintf(locstr, "%s %d", gui_strings[GSTR_JOYSTICK], i);
            text = loctext_to_gtext(locstr);
            draw_text_purple_list2(wpos_x, wpos_y, text, 0);
        }
        else
        {
            sprintf(locstr, "%s", gui_strings[461 + ctlmode]); // Keyboard or Mouse
            text = loctext_to_gtext(locstr);
            draw_text_purple_list2(wpos_x, wpos_y, text, 0);
        }
        tx_width = LbTextStringWidth(locstr);

        active_rect.X = text_window_x1 + wpos_x;
        active_rect.Width = tx_width;
        if (mouse_down_over_box(&active_rect))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;

                ctlmode = p_locplayer->UserInput[dmuser].ControlMode + 1;
                if (ctl_joystick_type == JTyp_NONE && ctlmode == UInpCtr_Joystick0)
                    ctlmode = UInpCtr_Keyboard;
                if (ctlmode >= UInpCtr_Joystick0 + joy.NumberOfDevices)
                    ctlmode = UInpCtr_Keyboard;
                p_locplayer->UserInput[dmuser].ControlMode = ctlmode;
            }
        }
        wpos_y += ln_height + 4;
    }
    lbDisplay.DrawFlags &= ~0x8000;

    //controls_calibrate_button.DrawFn(&controls_calibrate_button); -- incompatible calling convention
    {
        ubyte drawn;
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&controls_calibrate_button), "g" (controls_calibrate_button.DrawFn));
    }
    return 0;
}

void set_controls_key(ushort hlight_gkey, ushort key)
{
    GameKey gkey;
    TbBool is_joystick;

    if (hlight_gkey > (GKey_KEYS_COUNT - 1)) {
        gkey = hlight_gkey - (GKey_KEYS_COUNT - 1);
        is_joystick = true;
    } else {
        gkey = hlight_gkey;
        is_joystick = false;
    }

    if (is_joystick)
        set_gamekey_joy(gkey, key);
    else
        set_gamekey_kbd(gkey, key);
}

void unset_controls_key(ushort hlight_gkey)
{
    GameKey gkey;
    TbBool is_joystick;

    if (hlight_gkey > (GKey_KEYS_COUNT - 1)) {
        gkey = hlight_gkey - (GKey_KEYS_COUNT - 1);
        is_joystick = true;
    } else {
        gkey = hlight_gkey;
        is_joystick = false;
    }

    if (is_joystick)
        set_gamekey_joy(gkey, 0);
    else
        set_gamekey_kbd(gkey, KC_UNASSIGNED);
}

TbBool is_hardcoded_hlight_gkey(ushort hlight_gkey)
{
    if (hlight_gkey >= (GKey_KEYS_COUNT - 1) + GKey_UP &&
      hlight_gkey <= (GKey_KEYS_COUNT - 1) + GKey_RIGHT)
        return true;

    return false;
}

#define GAMEKEY_ACTIVE_WIDTH_MIN 50
#define SHEET_MOVE_COLUMNS 2

/** Check inputs for controls box in system menu screen.
 *
 * @return Gives 0 on no action, 1 on non-control-changing action, 2 on control key update/
 */
ubyte menu_controls_inputs(struct ScreenTextBox *p_box, short *p_tx_kbd_width, short *p_tx_joy_width, int i_limit)
{
    short ln_height;
    short wpos_x, wpos_y;
    int i;
    ubyte ret;

    ret = 0;

    lbFontPtr = p_box->Font;
    ln_height = my_char_height('A');

    if (lbDisplay.LeftButton || joy.Buttons[0])
    {
        wpos_x = sheet_columns_x[1];
        wpos_y = p_box->ScrollWindowOffset + 1;
        for (i = p_box->TextTopLine; i < i_limit; i++)
        {
            short col_width;
            GameKey gkey;

            gkey = i + 1;
            col_width = max(p_tx_kbd_width[gkey], GAMEKEY_ACTIVE_WIDTH_MIN);
            if (mouse_down_over_box_coords(text_window_x1 + wpos_x, text_window_y1 + wpos_y,
              text_window_x1 + col_width + wpos_x, text_window_y1 + wpos_y + ln_height))
            {
                lbDisplay.LeftButton = 0;
                controls_hlight_gkey = gkey;
                if (is_hardcoded_hlight_gkey(controls_hlight_gkey)) {
                    controls_edited_gkey = 0;
                } else {
                    controls_edited_gkey = controls_hlight_gkey;
                }
                clear_key_pressed(lbInkey);
                break;
            }
            wpos_y += p_box->LineHeight;
        }
    }

    if (lbDisplay.LeftButton || joy.Buttons[0])
    {
        wpos_x = sheet_columns_x[2];
        wpos_y = p_box->ScrollWindowOffset + 1;
        for (i = p_box->TextTopLine; i < i_limit; i++)
        {
            short col_width;
            GameKey gkey;

            gkey = i + 1;
            col_width = max(p_tx_joy_width[gkey], GAMEKEY_ACTIVE_WIDTH_MIN);
            if (mouse_down_over_box_coords(text_window_x1 + wpos_x, text_window_y1 + wpos_y,
              text_window_x1 + wpos_x + col_width, text_window_y1 + wpos_y + ln_height))
            {
                lbDisplay.LeftButton = 0;
                controls_hlight_gkey = gkey + (GKey_KEYS_COUNT - 1);
                if (is_hardcoded_hlight_gkey(controls_hlight_gkey)) {
                    controls_edited_gkey = 0;
                } else {
                    controls_edited_gkey = controls_hlight_gkey;
                }
                clear_key_pressed(lbInkey);
                break;
            }
            wpos_y += p_box->LineHeight;
        }
    }

    if (lbDisplay.LeftButton || joy.Buttons[0])
    {
        if (mouse_down_over_box(p_box))
        {
            // Clicked outside of the keys area - exit editing a binding
            // Do not clear the click
            controls_edited_gkey = 0;
        }
    }

    if (controls_edited_gkey == 0 && !net_unkn_pos_02)
    {
        if (is_key_pressed(KC_DOWN, KMod_DONTCARE))
        {
            clear_key_pressed(KC_DOWN);
            controls_hlight_gkey++;
            if (controls_hlight_gkey > SHEET_MOVE_COLUMNS * (GKey_KEYS_COUNT - 1))
                controls_hlight_gkey = 1;
            ret = 1;
        }
        if (is_key_pressed(KC_UP, KMod_DONTCARE))
        {
            clear_key_pressed(KC_UP);
            if (controls_hlight_gkey < 1 + 1)
                controls_hlight_gkey = SHEET_MOVE_COLUMNS * (GKey_KEYS_COUNT - 1);
            else
                controls_hlight_gkey--;
            ret = 1;
        }
        if (is_key_pressed(KC_RIGHT, KMod_DONTCARE))
        {
            // Next column
            clear_key_pressed(KC_RIGHT);
            controls_hlight_gkey += (GKey_KEYS_COUNT - 1);
            if (controls_hlight_gkey > SHEET_MOVE_COLUMNS * (GKey_KEYS_COUNT - 1))
                controls_hlight_gkey -= SHEET_MOVE_COLUMNS * (GKey_KEYS_COUNT - 1);
            ret = 1;
        }
        if (is_key_pressed(KC_LEFT, KMod_DONTCARE))
        {
            // Prev column
            clear_key_pressed(KC_LEFT);
            if (controls_hlight_gkey < (GKey_KEYS_COUNT - 1) + 1)
                controls_hlight_gkey += (SHEET_MOVE_COLUMNS - 1) * (GKey_KEYS_COUNT - 1);
            else
                controls_hlight_gkey -= (GKey_KEYS_COUNT - 1);
            ret = 1;
        }

        if (byte_1C4970)
        {
            if (byte_1C4970 == 1 && !is_key_pressed(KC_RETURN, KMod_DONTCARE))
            {
                if (!is_hardcoded_hlight_gkey(controls_hlight_gkey)) {
                    clear_key_pressed(lbInkey);
                    byte_1C4970 = 0;
                    controls_edited_gkey = controls_hlight_gkey;
                    ret = 2;
                }
            }
        }
        else
        {
            if (is_key_pressed(KC_RETURN, KMod_DONTCARE))
            {
                if (is_hardcoded_hlight_gkey(controls_hlight_gkey)) {
                    play_sample_using_heap(0, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                } else {
                    byte_1C4970++;
                    ret = 2;
                }
            }
            if (is_key_pressed(KC_BACK, KMod_DONTCARE))
            {
                if (is_hardcoded_hlight_gkey(controls_hlight_gkey)) {
                    play_sample_using_heap(0, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                } else {
                    clear_key_pressed(KC_BACK);
                    unset_controls_key(controls_hlight_gkey);
                    ret = 2;
                }
            }
        }
    }

    if (controls_edited_gkey != 0)
    {
        GameKey edited_gkey;

        edited_gkey = controls_edited_gkey;
        if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
        {
            clear_key_pressed(KC_ESCAPE);
            controls_edited_gkey = 0;
        }
        else if (edited_gkey > (GKey_KEYS_COUNT - 1))
        {
            if (is_joy_pressed_any(0))
            {
                ushort jskey;

                jskey = get_joy_pressed_key(0);
                set_controls_key(edited_gkey, jskey);
                ret = 2;
            }
        }
        else
        {
            if (lbInkey != KC_UNASSIGNED)
            {
#if defined(DOS)||defined(GO32)
                if (lbExtendedKeyPress)
                {
                  set_controls_key(edited_gkey, lbInkey | 0x80);
                }
                else
                {
                  if ((lbInkey & 0x7F) != KC_BACKSLASH)
                      set_controls_key(edited_gkey, lbInkey & 0x7F);
                }
#else
                set_controls_key(edited_gkey, lbInkey);
#endif
                clear_key_pressed(lbInkey);
                ret = 2;
            }
        }
    }
    return ret;
}

const char *gamekey_text_action_name_for_draw(GameKey gkey)
{
    char locstr[52];
    const char *text;

    if ((gkey >= GKey_SEL_AGENT_1) && (gkey <= GKey_SEL_AGENT_4))
    {
        //TODO should we place Agent/Zealot/Outcast text ID in campaign config file, or keep at const position within campaign area?
        text = gui_strings[GSTR_CPG_CYBORG_NOUN_FULL + 100 * background_type];
        sprintf(locstr, "%s %d", text, (gkey - GKey_SEL_AGENT_1 + 1));
        text = loctext_to_gtext(locstr);
    }
    else
    {
        text = gui_strings[GSTR_SYS_GAME_KEYS + gkey - 1];
    }
    return text;
}

const char *gamekey_text_kbkey_name_for_draw(GameKey gkey)
{
    char locstr[52];
    const char *text;

    sprint_gamekey_combination_kbd(locstr, gkey);
    if (strlen(locstr) == 0) {
      strcpy(locstr, "...");
    }
    text = loctext_to_gtext(locstr);

    return text;
}

const char *gamekey_text_jskey_name_for_draw(GameKey gkey)
{
    char locstr[52];
    const char *text;

    sprint_gamekey_combination_joy(locstr, gkey);
    if (strlen(locstr) == 0) {
      strcpy(locstr, "...");
    }
    text = loctext_to_gtext(locstr);

    return text;
}

ubyte show_menu_controls_list_box(struct ScreenTextBox *p_box)
{
    short wpos_x, wpos_y;
    short tx_kbd_width[GKey_KEYS_COUNT];
    short tx_joy_width[GKey_KEYS_COUNT];
    int i, i_limit;
    ubyte kchange;

    // This needs to be done before setting my_text_window
    i_limit = p_box->TextTopLine + get_text_box_lines_visible(p_box);
    if (i_limit > GKey_KEYS_COUNT - 1)
        i_limit = GKey_KEYS_COUNT - 1;

    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    if ((p_box->Flags & GBxFlg_BkgndDrawn) == 0) // never set anyway
    {
        const char *text;

        lbFontPtr = med_font;
        text = gui_strings[GSTR_CONTROLS];
        draw_text_purple_list2(sheet_columns_x[0], 4, text, 0);
        text = gui_strings[GSTR_KEYS];
        draw_text_purple_list2(sheet_columns_x[1], 4, text, 0);
        text = gui_strings[GSTR_JOYSTICK2];
        draw_text_purple_list2(sheet_columns_x[2], 4, text, 0);

        lbDisplay.DrawFlags = 0;
    }

    lbFontPtr = p_box->Font;

    // Names column
    wpos_x = sheet_columns_x[0];
    wpos_y = p_box->ScrollWindowOffset + 1;
    for (i = p_box->TextTopLine; i < i_limit; i++)
    {
        const char *text;
        GameKey gkey;

        gkey = i + 1;
        text = gamekey_text_action_name_for_draw(gkey);
        draw_text_purple_list2(wpos_x, wpos_y, text, 0);
        wpos_y += p_box->LineHeight;
    }

    // Keyboard keys column
    wpos_x = sheet_columns_x[1];
    wpos_y = p_box->ScrollWindowOffset + 1;
    for (i = p_box->TextTopLine; i < i_limit; i++)
    {
        const char *text;
        GameKey gkey;

        gkey = i + 1;
        if (controls_edited_gkey == gkey)
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 0x32; // white
        }
        else if (controls_hlight_gkey == gkey)
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 0x57; // med purple
        }
        else if (is_hardcoded_hlight_gkey(gkey))
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 0x69; // darker purple
        }
        else
        {
            lbDisplay.DrawFlags = 0;
            // Original colour - blue
        }
        text = gamekey_text_kbkey_name_for_draw(gkey);
        tx_kbd_width[gkey] = my_string_width(text);

        lbDisplay.DrawFlags |= 0x8000;
        draw_text_purple_list2(wpos_x, wpos_y, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;

        wpos_y += p_box->LineHeight;
    }

    // Joystick keys column
    wpos_x = sheet_columns_x[2];
    wpos_y = p_box->ScrollWindowOffset + 1;
    for (i = p_box->TextTopLine; i < i_limit; i++)
    {
        const char *text;
        GameKey gkey;

        gkey = i + 1;
        if (controls_edited_gkey == gkey + (GKey_KEYS_COUNT - 1))
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 0xF8; // light purple
        }
        else if (controls_hlight_gkey == gkey + (GKey_KEYS_COUNT - 1))
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 0x57; // med purple
        }
        else if (is_hardcoded_hlight_gkey(gkey + (GKey_KEYS_COUNT - 1)))
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = 0x69; // darker purple
        }
        else
        {
            lbDisplay.DrawFlags = 0;
            // Original colour - blue
        }
        text = gamekey_text_jskey_name_for_draw(gkey);
        tx_joy_width[gkey] = my_string_width(text);

        lbDisplay.DrawFlags |= 0x8000;
        draw_text_purple_list2(wpos_x, wpos_y, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;

        wpos_y += p_box->LineHeight;
    }
    lbDisplay.DrawFlags = 0;

    kchange = menu_controls_inputs(p_box, tx_kbd_width, tx_joy_width, i_limit);

    // If moved key, make sure the new highlight is visible
    if ((kchange != 0) && (controls_hlight_gkey != 0))
    {
        int i;

        if (controls_hlight_gkey < GKey_KEYS_COUNT)
            i = controls_hlight_gkey - 1;
        else
            i = controls_hlight_gkey - 1 - (GKey_KEYS_COUNT - 1);
        if (p_box->TextTopLine > i)
            p_box->TextTopLine = i;
        else if (p_box->TextTopLine + i_limit < i + 1)
            p_box->TextTopLine = i + 1 - i_limit;

    }

    return 0;
}

ubyte update_settings_controls_alert(struct ScreenTextBox *p_box)
{
    if (!net_unkn_pos_02 || (net_unkn_pos_02 - 1) > 5)
        return 0;

    switch (net_unkn_pos_02)
    {
    case 1:
        if (screentype == SCRT_ALERTBOX)
        {
            if (!joy.Buttons[0])
                break;
            net_unkn_pos_02++;
            break;
        }
        net_unkn_pos_02 = 0;
        break;
    case 2:
        if (joy.Buttons[0])
            break;
        alert_box_text_fmt("%s", gui_strings[GSTR_JOY_CAL_BOTTOM_RIGHT]);
        net_unkn_pos_02++;
        break;
    case 3:
        if (screentype == SCRT_ALERTBOX)
        {
            if (!joy.Buttons[0])
                break;
            net_unkn_pos_02++;
            break;
        }
        net_unkn_pos_02 = 0;
        break;
    case 4:
        if (joy.Buttons[0])
            break;
        alert_box_text_fmt("%s", gui_strings[GSTR_JOY_CAL_LEAVE_CENTRE]);
        net_unkn_pos_02++;
        break;
    case 5:
        if (screentype == SCRT_ALERTBOX)
        {
            if (!joy.Buttons[0])
                break;
            net_unkn_pos_02++;
            break;
        }
        net_unkn_pos_02 = 0;
        break;
    case 6:
        show_alert = 1;
        if (joy.Buttons[0])
            break;
        net_unkn_pos_02 = 0;
        show_alert = 0;
        break;
    }
    return 0;
}

ubyte show_options_controls_screen(void)
{
    ubyte drawn;

    //drawn = controls_list_box.DrawFn(&controls_list_box); -- incompatible calling convention
    asm volatile ("call *%2\n"
        : "=r" (drawn) : "a" (&controls_list_box), "g" (controls_list_box.DrawFn));
    if (drawn == 3) {
        update_settings_controls_alert(&controls_list_box);
        //drawn = controls_joystick_box.DrawFn(&controls_joystick_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&controls_joystick_box), "g" (controls_joystick_box.DrawFn));
    }
    return drawn;
}

#define SCROLL_BAR_WIDTH 12

void init_controls_screen_boxes(void)
{
    ScrCoord scr_w, scr_h, start_x, start_y;
    short space_w, space_h, border;
    short part_w;

    // Border value represents how much the box background goes
    // out of the box area.
    border = 3;
    scr_w = lbDisplay.GraphicsWindowWidth;
#ifdef EXPERIMENTAL_MENU_CENTER_H
    scr_h = global_apps_bar_box.Y;
#else
    scr_h = 432;
#endif

    init_screen_text_box(&controls_list_box, 213u, 72u, 420u, 354, 6, small_med_font, 1);
    controls_list_box.DrawTextFn = ac_show_menu_controls_list_box;
    controls_list_box.ScrollWindowHeight = 296;
    controls_list_box.Lines = GKey_KEYS_COUNT;
    controls_list_box.Flags |= (GBxFlg_RadioBtn | GBxFlg_IsMouseOver);

    controls_list_box.ScrollWindowOffset += 27;
    init_screen_button(&controls_defaults_button, 219u, 405u,
      gui_strings[484], 6, med2_font, 1, 0);
    controls_defaults_button.CallBackFn = ac_do_controls_defaults;
    init_screen_button(&controls_save_button, 627u, 405u,
      gui_strings[439], 6, med2_font, 1, 0x80);
    controls_save_button.CallBackFn = ac_do_controls_save;

    controls_list_box.Buttons[0] = &controls_defaults_button;
    controls_list_box.Buttons[1] = &controls_save_button;

    init_screen_box(&controls_joystick_box, 7u, 252u, 197u, 174, 6);
    controls_joystick_box.SpecialDrawFn = show_controls_joystick_box;
    init_screen_button(&controls_calibrate_button, 57u, 405u,
      gui_strings[485], 6, med2_font, 1, 0);
    controls_calibrate_button.CallBackFn = ac_do_controls_calibrate;

    // Reposition the components to current resolution

    start_x = unkn13_SYSTEM_button.X + unkn13_SYSTEM_button.Width;
    // On the X axis, we're going for centering on the screen. So subtract the previous
    // button position two times - once for the left, and once to make the same space on
    // the right.
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - controls_list_box.Width;

    start_y = system_screen_shared_header_box.Y + system_screen_shared_header_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y - controls_list_box.Height + border;

    // There is one box only to position, and no space is needed after it - the whole
    // available empty space goes into one place.
    controls_list_box.X = start_x + space_w;
    // There is one box only to position, so space goes into two parts - before and after.
    controls_list_box.Y = start_y + space_h / 2;

    start_x = unkn13_SYSTEM_button.X;

    controls_joystick_box.X = start_x;
    controls_calibrate_button.X = controls_joystick_box.X + 50;

    part_w = controls_list_box.Width - 6 - SCROLL_BAR_WIDTH;
    sheet_columns_x[0] = 3 + 1;
    sheet_columns_x[1] = 3 + part_w - 2 * (part_w / 4);
    sheet_columns_x[2] = 3 + part_w - (part_w / 4);
    controls_defaults_button.X = controls_list_box.X + 6;
    controls_defaults_button.Y = controls_list_box.Y + controls_list_box.Height - controls_defaults_button.Height - 5;
    controls_save_button.X = controls_list_box.X + controls_list_box.Width - controls_save_button.Width - 6 - SCROLL_BAR_WIDTH;
    controls_save_button.Y = controls_list_box.Y + controls_list_box.Height - controls_save_button.Height - 5;

    controls_joystick_box.Y = controls_list_box.Y + controls_list_box.Height - controls_joystick_box.Height;
    controls_calibrate_button.Y = controls_joystick_box.Y + controls_joystick_box.Height - 21;
}

void reset_controls_screen_boxes_flags(void)
{
    controls_list_box.Flags = GBxFlg_Unkn0001;
    controls_list_box.Flags |= (GBxFlg_RadioBtn | GBxFlg_IsMouseOver);
    controls_joystick_box.Flags = GBxFlg_Unkn0001;
}

void set_flag01_controls_screen_boxes(void)
{
    controls_list_box.Flags |= GBxFlg_Unkn0001;
    controls_save_button.Flags |= GBxFlg_Unkn0001;
    controls_defaults_button.Flags |= GBxFlg_Unkn0001;
    controls_calibrate_button.Flags |= GBxFlg_Unkn0001;
}

void skip_flashy_draw_controls_screen_boxes(void)
{
    controls_defaults_button.Flags |= GBxFlg_Unkn0002;
    controls_calibrate_button.Flags |= GBxFlg_Unkn0002;
    controls_save_button.Flags |= GBxFlg_Unkn0002;
    controls_joystick_box.Flags |= GBxFlg_Unkn0002;
    controls_list_box.Flags |= GBxFlg_Unkn0002;
}

/******************************************************************************/
