/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file festorage.c
 *     Front-end desktop and menu system, file storage screen.
 * @par Purpose:
 *     Implement functions for file storage screen in front-end desktop.
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
#include "festorage.h"

#include "bffont.h"
#include "bftext.h"
#include "ssampply.h"

#include "femain.h"
#include "feshared.h"
#include "game_speed.h"
#include "guiboxes.h"
#include "guitext.h"
#include "display.h"
#include "game_options.h"
#include "game_save.h"
#include "game_sprts.h"
#include "game.h"
#include "keyboard.h"
#include "mydraw.h"
#include "purpldrw.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
struct ScreenButton storage_LOAD_button = {0};
struct ScreenButton storage_SAVE_button = {0};
struct ScreenButton storage_NEW_MORTAL_button = {0};
struct ScreenTextBox storage_slots_box = {0};
extern ubyte byte_1C4880[8];

/******************************************************************************/

ubyte ac_do_storage_NEW_MORTAL(ubyte click);
ubyte ac_load_game_slot(ubyte click);
ubyte ac_save_game_slot(ubyte click);
ubyte ac_show_menu_storage_slots_box(struct ScreenTextBox *p_box);

int autosave_game(void)
{
    // only autosave mortal games
    if ((ingame.Flags & GamF_MortalGame) == 0)
        return 1;

    return save_game_write(0, save_active_desc);
}

ubyte show_menu_storage_slots_box(struct ScreenTextBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_menu_storage_slots_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short scr_x, scr_y;
    short i, k;
    char locstr[8];
    short ln_height;
    short entry_height;
    short slot;

    if (p_box->TextTopLine != save_slot_base)
    {
        save_slot_base = p_box->TextTopLine;
        load_save_slot_names();
    }
    if ((p_box->Flags & 0x0080) != 0)
    {
        p_box->Flags &= ~0x0080;
        storage_LOAD_button.Flags |= 0x0001;
        storage_NEW_MORTAL_button.Flags |= 0x0001;
        storage_SAVE_button.Flags |= 0x0001;
    }
    if (p_box->Timer == 255)
    {
        p_box->TextFadePos = 15;
        storage_SAVE_button.Flags |= 0x0002;
        storage_LOAD_button.Flags |= 0x0002;
        storage_NEW_MORTAL_button.Flags |= 0x0002;
    }

    if ((p_box->Flags & 0x8000) == 0)
    {
        const char *text;

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        scr_x = 3;

        for (i = 0; i < 8; i++)
        {
            short h;

            h = 26 * i;
            draw_box_purple_list(text_window_x1 + scr_x,      text_window_y1 + 3 + h, 30, 22, 243);
            draw_box_purple_list(text_window_x1 + scr_x + 34, text_window_y1 + 3 + h, 362, 22, 243);
        }
        draw_box_purple_list(text_window_x1 + scr_x,      text_window_y1 + 237, 30, 22, 243);
        draw_box_purple_list(text_window_x1 + scr_x + 34, text_window_y1 + 237, 362, 22, 243);
        lbDisplay.DrawFlags = 0;

        my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
        lbFontPtr = med_font;
        text = gui_strings[408];
        scr_x = ((p_box->Width - my_string_width(text)) >> 1) - 3;
        draw_text_purple_list2(scr_x, 2, text, 0);
        p_box->Flags |= 0x8000;
        copy_box_purple_list(p_box->X + 4, p_box->Y - 3, p_box->Width - 20, p_box->Height + 6);
    }

    my_set_text_window(p_box->X + 4, p_box->Y + p_box->ScrollWindowOffset + 4,
      p_box->Width - 20, p_box->ScrollWindowHeight + 23);
    lbFontPtr = p_box->Font;

    scr_x = 3;
    scr_y = 3;
    ln_height = 22;
    slot = save_slot_base;
    entry_height = ln_height + 4;
    while (slot < (save_slot_base + SAVE_SLOTS_VISIBLE_COUNT) && scr_y + ln_height < p_box->ScrollWindowHeight + 23)
    {
        const char *text;
        char *slot_str;
        short tx_width;

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y - 1,
          text_window_x2, text_window_y1 + scr_y + ln_height + 1))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                edit_flag = 1;
                save_slot = slot + 1;
                reset_buffered_keys();
            }
        }

        if (mouse_move_over_box_coords(text_window_x1, text_window_y1 + scr_y - 1,
          text_window_x2, text_window_y1 + scr_y + ln_height + 1))
        {
            k = slot - save_slot_base;
            if (!byte_1C4880[k])
            {
                byte_1C4880[k] = 1;
                play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
            }
            lbDisplay.DrawFlags = 0;
        }
        else
        {
            k = slot - save_slot_base;
            byte_1C4880[k] = 0;
        }

        if (slot + 1 == save_slot)
            lbDisplay.DrawFlags = 0;
        sprintf(locstr, "%d", slot + 1);
        text = loctext_to_gtext(locstr);
        lbDisplay.DrawFlags |= 0x8000;
        tx_width = LbTextStringWidth(text);
        draw_text_purple_list2(scr_x + ((32 - tx_width) >> 1), scr_y + 6, text, 0);
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;

        if (slot + 1 == save_slot)
        {
            lbDisplay.DrawFlags = 0;
            if (edit_flag)
            {
                slot_str = save_slot_names[slot - save_slot_base];
                if (user_read_value(slot_str, 23, 1))
                    edit_flag = 0;
            }
        }
        slot_str = save_slot_names[slot - save_slot_base];
        draw_text_purple_list2(scr_x + 36, scr_y + 6, slot_str, 0);
        if (edit_flag && (save_slot == slot + 1) && ((gameturn & 1) != 0))
        {
          const struct TbSprite *p_spr;

          slot_str = save_slot_names[slot - save_slot_base];
          p_spr = LbFontCharSprite(lbFontPtr, '-');
          tx_width = my_string_width(slot_str);
          draw_sprite_purple_list(text_window_x1 + scr_x + 36 + tx_width, text_window_y1 + scr_y + 11, p_spr);
        }
        scr_y += entry_height;
        slot++;
    }

    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    if (save_active_desc[0] != '\0')
    {
        short w;

        scr_y += ln_height + 4;
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y - 1,
          text_window_x2, text_window_y1 + scr_y + ln_height + 1))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                save_slot = 0;
            }
        }
        if (mouse_move_over_box_coords(text_window_x1, text_window_y1 + scr_y - 1,
          text_window_x2, text_window_y1 + scr_y + ln_height + 1) || (slot + 1 == save_slot))
        {
            lbDisplay.DrawFlags = 0;
        }
        lbDisplay.DrawFlags |= 0x8000;

        w = LbTextCharWidth('M');
        draw_sprite_purple_list(((30 - w) >> 1) + text_window_x1 + scr_x,
          text_window_y1 + scr_y + 6, LbFontCharSprite(lbFontPtr, 'M'));
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if (!save_slot)
            lbDisplay.DrawFlags = 0;
        draw_text_purple_list2(scr_x + 36, scr_y + 6, save_active_desc, 0);
    }
    lbDisplay.DrawFlags = 0;
    //storage_LOAD_button.DrawFn(&storage_LOAD_button); -- incompatible calling convention
    asm volatile ("call *%1\n"
        :  : "a" (&storage_LOAD_button), "g" (storage_LOAD_button.DrawFn));
    if (!restore_savegame) {
        //storage_SAVE_button.DrawFn(&storage_SAVE_button); -- incompatible calling convention
        asm volatile ("call *%1\n"
            :  : "a" (&storage_SAVE_button), "g" (storage_SAVE_button.DrawFn));
    }
    //storage_NEW_MORTAL_button.DrawFn(&storage_NEW_MORTAL_button); -- incompatible calling convention
    asm volatile ("call *%1\n"
        :  : "a" (&storage_NEW_MORTAL_button), "g" (storage_NEW_MORTAL_button.DrawFn));
    return 0;
}

ubyte show_storage_screen(void)
{
    ubyte drawn;

    //drawn = storage_slots_box.DrawFn(&storage_slots_box); -- incompatible calling convention
    asm volatile ("call *%2\n"
        : "=r" (drawn) : "a" (&storage_slots_box), "g" (storage_slots_box.DrawFn));
    return drawn;
}

void init_storage_screen_boxes(void)
{
    ScrCoord scr_w, scr_h, start_x, start_y;
    short space_w, space_h, border;

    // Border value represents how much the box background goes
    // out of the box area.
    border = 3;
    scr_w = lbDisplay.GraphicsWindowWidth;
#ifdef EXPERIMENTAL_MENU_CENTER_H
    scr_h = global_apps_bar_box.Y;
#else
    scr_h = 432;
#endif

    init_screen_text_box(&storage_slots_box, 213u, 72u, 420u, 354, 6, med2_font, 1);
    storage_slots_box.DrawTextFn = show_menu_storage_slots_box;
    storage_slots_box.ScrollWindowHeight = 208;
    storage_slots_box.Lines = 99;
    storage_slots_box.Flags |= (GBxFlg_RadioBtn | GBxFlg_IsMouseOver);
    storage_slots_box.LineHeight = 26;
    storage_slots_box.ScrollWindowOffset += 27;

    init_screen_button(&storage_LOAD_button, 219u, 405u,
      gui_strings[438], 6, med2_font, 1, 0);
    init_screen_button(&storage_SAVE_button, 219 + storage_LOAD_button.Width + 4, 405u,
      gui_strings[439], 6, med2_font, 1, 0);
    init_screen_button(&storage_NEW_MORTAL_button, 627u, 405u,
      gui_strings[482], 6, med2_font, 1, 128);
    storage_LOAD_button.CallBackFn = ac_load_game_slot;
    storage_SAVE_button.CallBackFn = ac_save_game_slot;
    storage_NEW_MORTAL_button.CallBackFn = ac_do_storage_NEW_MORTAL;

    // Reposition the components to current resolution

    start_x = unkn13_SYSTEM_button.X + unkn13_SYSTEM_button.Width;
    // On the X axis, we're going for centering on the screen. So subtract the previous
    // button position two times - once for the left, and once to make the same space on
    // the right.
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - storage_slots_box.Width;

    start_y = system_screen_shared_header_box.Y + system_screen_shared_header_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y - storage_slots_box.Height + border;

    // There is one box only to position, and no space is needed after it - the whole
    // available empty space goes into one place.
    storage_slots_box.X = start_x + space_w;
    // There is one box only to position, so space goes into two parts - before and after.
    storage_slots_box.Y = start_y + space_h / 2;

    space_w = 5;
    space_h = 5;
    storage_LOAD_button.X = storage_slots_box.X + (space_w + 1);
    storage_LOAD_button.Y = storage_slots_box.Y + storage_slots_box.Height - space_w - storage_LOAD_button.Height;
    storage_SAVE_button.X = storage_LOAD_button.X + storage_LOAD_button.Width + (space_w - 1);
    storage_SAVE_button.Y = storage_LOAD_button.Y;
    storage_NEW_MORTAL_button.X = storage_slots_box.X + storage_slots_box.Width -
      storage_NEW_MORTAL_button.Width - (space_w + 1);
    storage_NEW_MORTAL_button.Y = storage_LOAD_button.Y;
}

void reset_storage_screen_boxes_flags(void)
{
    storage_slots_box.Flags = GBxFlg_Unkn0001;
    storage_slots_box.Flags |= (GBxFlg_RadioBtn | GBxFlg_IsMouseOver);
}

void set_flag01_storage_screen_boxes(void)
{
    storage_LOAD_button.Flags |= GBxFlg_Unkn0001;
    storage_SAVE_button.Flags |= GBxFlg_Unkn0001;
    storage_NEW_MORTAL_button.Flags |= GBxFlg_Unkn0001;
}

void skip_flashy_draw_storage_screen_boxes(void)
{
    storage_slots_box.Flags |= GBxFlg_Unkn0002;
}

void mark_storage_screen_boxes_redraw(void)
{
    storage_slots_box.Flags &= ~(GBxFlg_BkgndDrawn | GBxFlg_TextRight | GBxFlg_BkCopied);
}

/******************************************************************************/
