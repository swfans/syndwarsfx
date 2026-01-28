/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file febrief.c
 *     Front-end desktop and menu system, e-mail mission brief screen.
 * @par Purpose:
 *     Implement functions for e-mail mission brief screen in front-end desktop.
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
#include "febrief.h"

#include "bfkeybd.h"
#include "bftext.h"
#include "bfmath.h"
#include "bfmemut.h"
#include "bfutility.h"
#include "bfscrcopy.h"
#include "ssampply.h"

#include "campaign.h"
#include "femail.h"
#include "femain.h"
#include "game_data.h"
#include "game_options.h"
#include "guiboxes.h"
#include "guitext.h"
#include "purpldrw.h"
#include "display.h"
#include "game_sprts.h"
#include "game.h"
#include "keyboard.h"
#include "lvobjctv.h"
#include "mydraw.h"
#include "scanner.h"
#include "sound.h"
#include "wadfile.h"
#include "wrcities.h"
#include "swlog.h"
/******************************************************************************/

short next_brief = 0;
short old_mission_brief = 0;
short open_brief = 0;

struct ScreenTextBox brief_mission_text_box = {0};
struct ScreenButton unkn1_ACCEPT_button = {0};
struct ScreenButton unkn1_CANCEL_button = {0};

struct ScreenButton brief_NETSCAN_button = {0};
struct ScreenInfoBox brief_NETSCAN_COST_box = {0};
struct ScreenTextBox brief_netscan_box = {0};

struct ScreenBox brief_graphical_box = {0};

extern sbyte selected_netscan_objective;// = -1;
extern char unkn39_text[];

ubyte brief_state_city_selected = 0;
ubyte brief_citymap_content = BriCtM_AUTO_SCANNER;

/** Amount of cities available in brief of a selected mail message.
 */
long mail_num_active_cities = 0;

long dword_1C47E0 = 0;
ubyte byte_1C47E4 = 0;
short word_1C47E6 = 0;
short word_1C47E8 = 0;

/******************************************************************************/

ubyte ac_brief_do_netscan_enhance(ubyte click);
ubyte ac_show_brief_netscan_box(struct ScreenTextBox *box);
ubyte ac_accept_mission(ubyte click);
ubyte ac_do_unkn1_CANCEL(ubyte click);
void ac_purple_unkn2_data_to_screen(void);
void ac_SCANNER_data_to_screen(void);
void update_netscan_cost_button(ubyte city_id)
{
    int k, max_width;
    ushort ninfo;
    char *text;

    ninfo = cities[city_id].Info;
    if (ninfo >= netscan_objectives_count) {
        brief_NETSCAN_COST_box.Text2[0] = '\0';
        text = gui_strings[495];
    } else {
        k = 100 * netscan_objectives[ninfo].CreditCost;
        sprintf(brief_NETSCAN_COST_box.Text2, "%d", k);
        text = gui_strings[442];
    }
    brief_NETSCAN_COST_box.Text1 = text;

    // Update label size, in case its text got longer
    max_width = brief_NETSCAN_button.X - brief_NETSCAN_COST_box.X;
    // Width of the cost field can normally only reach until the
    // netscan button. Unless said button is hidden - then it can
    // go through the button width as well
    if (brief_NETSCAN_COST_box.Text2[0] == '\0') {
        max_width += brief_NETSCAN_button.Width;
    }
    // width algorithm from init_screen_button()
    lbFontPtr = brief_NETSCAN_COST_box.Font1;
    k = my_string_width(brief_NETSCAN_COST_box.Text1) + 4;
    k = max(k, 213);
    brief_NETSCAN_COST_box.Width = min(k, max_width);
    brief_NETSCAN_COST_box.Flags |= GBxFlg_Unkn0001;

    if (game_projector_speed)
        brief_NETSCAN_COST_box.Flags |= GBxFlg_Unkn0002;
}

void reveal_netscan_objective(short nsobv)
{
    struct NetscanObjective *p_nsobv;

    p_nsobv = &netscan_objectives[nsobv];
    brief_netscan_box.Lines += p_nsobv->TextLines;
}

void brief_citymap_readd_scanner_signals(void)
{
    struct NetscanObjective *p_nsobv;

    p_nsobv = &netscan_objectives[selected_netscan_objective];
    add_netscan_signal_to_scanner(p_nsobv, 1);
}

ubyte brief_do_netscan_enhance(ubyte click)
{
    short nsobv;
    struct NetscanObjective *p_nsobv;

    nsobv = cities[selected_city_id].Info;
    p_nsobv = &netscan_objectives[nsobv];

    if (!player_try_spend_money(100 * p_nsobv->CreditCost))
        return 0;

    cities[selected_city_id].Info++;
    update_netscan_cost_button(selected_city_id);
    selected_netscan_objective = nsobv;
    reveal_netscan_objective(selected_netscan_objective);
    brief_netscan_box.Flags |= GBxFlg_Unkn0080;
    recount_city_credit_reward(selected_city_id);
    brief_citymap_readd_scanner_signals();
    return 1;
}

TbBool mouse_over_text_window_item(short tx_height, short margin, short start_shift, short n_lines)
{
    short ln_height;
    short lines_y1, lines_y2;

    ln_height = tx_height + margin;
    // Allow one pixel higher or lower
    lines_y1 = text_window_y1 + start_shift - 1;
    lines_y2 = lines_y1 + n_lines * ln_height - margin + 1;

    return mouse_down_over_box_coords(text_window_x1, lines_y1, text_window_x2, lines_y2);
}

ubyte show_brief_netscan_box(struct ScreenTextBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_brief_netscan_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    int nlines;
    short start_shift;
    short nsobv;
    short tx_height, ln_height;
    short border, margin, scrollbar_width;
    ubyte drawn = true;

    border = 4;
    margin = 4;
    scrollbar_width = 12;
    if (open_brief < 0)
    {
        draw_noise_box_purple_list(p_box->X, p_box->Y, p_box->Width, p_box->Height);
        return 0;
    }

    my_set_text_window(p_box->X + border, p_box->ScrollWindowOffset + p_box->Y + border,
      p_box->Width - 2 * border - scrollbar_width, p_box->ScrollWindowHeight);
    lbFontPtr = small_med_font;
    tx_height = my_char_height('A');
    ln_height = tx_height + margin;
    nlines = 0;
    start_shift = border - ln_height * p_box->TextTopLine;
    lbDisplay.DrawColour = 87;
    lbDisplay.DrawFlags = 0;
    if (selected_city_id != -1)
    {
        for (nsobv = 0; nsobv < cities[selected_city_id].Info; nsobv++)
        {
            struct NetscanObjective *p_nsobv;

            p_nsobv = &netscan_objectives[nsobv];
            if (nlines + p_nsobv->TextLines >= p_box->TextTopLine)
            {
                if (lbDisplay.LeftButton)
                {
                    if (mouse_over_text_window_item(tx_height, margin, start_shift, p_nsobv->TextLines))
                    {
                        selected_netscan_objective = nsobv;
                        brief_citymap_readd_scanner_signals();
                    }
                }
                if (selected_netscan_objective == nsobv)
                    lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
                draw_text_purple_list2(0, start_shift, netscan_text + p_nsobv->TextOffset, 0);
                lbDisplay.DrawFlags = 0;
                start_shift += ln_height * p_nsobv->TextLines;
                if (start_shift + tx_height >= text_window_y2 - text_window_y1)
                    break;
            }
            nlines += p_nsobv->TextLines;
        }
        if (cities[selected_city_id].Info < netscan_objectives_count) {
            //drawn = brief_NETSCAN_button.DrawFn(&brief_NETSCAN_button); -- incompatible calling convention
            asm volatile ("call *%2\n"
                : "=r" (drawn) : "a" (&brief_NETSCAN_button), "g" (brief_NETSCAN_button.DrawFn));
        }
    }
    if (drawn) {
        //brief_NETSCAN_COST_box.DrawFn(&brief_NETSCAN_COST_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
          : "=r" (drawn) : "a" (&brief_NETSCAN_COST_box), "g" (brief_NETSCAN_COST_box.DrawFn));
    }
    return 0;
}

void flic_netscan_open_anim(ubyte netno)
{
    struct Animation *p_anim;
    PathInfo *pinfo;
    int k;
    ubyte anislot;

    anislot = AniSl_NETSCAN;
    k = anim_slots[anislot];
    p_anim = &animations[k];
    pinfo = &game_dirs[DirPlace_Equip];
    anim_flic_set_fname(p_anim, "%s/net%02d.fli", pinfo->directory, netno);
    flic_unkn03(anislot);
}

void purple_unkn2_data_to_screen(void)
{
    ubyte *buf;
    buf = anim_type_get_output_buffer(AniSl_NETSCAN);
    LbScreenSetGraphicsWindow(brief_graphical_box.X + 1, brief_graphical_box.Y + 1,
      brief_graphical_box.Width - 2, brief_graphical_box.Height - 2);
    LbScreenCopy(buf, lbDisplay.GraphicsWindowPtr, lbDisplay.GraphicsWindowHeight);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
}

void load_netscan_map(ushort mapno)
{
    char locstr[DISKPATH_SIZE];
    PathInfo *pinfo;
    TbFileHandle fh;

    pinfo = &game_dirs[DirPlace_Maps];
    snprintf(locstr, DISKPATH_SIZE-1, "%s/map%03d.scn", pinfo->directory, mapno);
    fh = LbFileOpen(locstr, Lb_FILE_MODE_READ_ONLY);
    if (fh != INVALID_FILE) {
        LbFileRead(fh, SCANNER_data, SCANNER_MAPDATA_WIDTH * SCANNER_MAPDATA_HEIGHT);
        LbFileClose(fh);
    } else {
        LbMemorySet(SCANNER_data, 0, SCANNER_MAPDATA_WIDTH * SCANNER_MAPDATA_HEIGHT);
    }
}

/** Counts amount of selectable cities, sets selected if single found.
 */
void count_selectable_cities(void)
{
    short city_id, last_city_id;

    mail_num_active_cities = 0;
    for (city_id = 0; city_id < num_cities; city_id++)
    {
        if (cities[city_id].Flags & CitF_Unkn01) {
            last_city_id = city_id;
            mail_num_active_cities++;
        }
    }
    // If found only one city, set it as selected
    if (mail_num_active_cities == 1)
        selected_city_id = last_city_id;
}

void show_citymap_city_selection(struct ScreenBox *box)
{
    short city_id;
    short text_h;
    short dy;
    ulong bufpos;

    text_h = my_char_height('A');
    dy = text_h + 4;
    for (city_id = 0; city_id < num_cities; city_id++)
    {
        if ((cities[city_id].Flags & CitF_Unkn01) == 0)
            continue;

        dy += text_h + 4;
        lbDisplay.DrawFlags |= 0x8000;
        bufpos = cities[city_id].TextIndex[0];
        draw_text_purple_list2(0, dy, (char*) &memload[bufpos], 0);
        lbDisplay.DrawFlags &= ~0x8000;
    }
}

ubyte input_citymap_city_selection(struct ScreenBox *p_box)
{
    short city_id;
    short text_h;
    short dy;

    text_h = my_char_height('A');
    dy = text_h + 4;
    for (city_id = 0; city_id < num_cities; city_id++)
    {
        if ((cities[city_id].Flags & CitF_Unkn01) == 0)
            continue;

        dy += text_h + 4;
        if (lbDisplay.LeftButton)
        {
            if (mouse_move_over_box_coords(text_window_x1, text_window_y1 + dy,
              text_window_x2, text_window_y1 + dy + text_h))
            {
                lbDisplay.LeftButton = 0;
                selected_city_id = city_id;
                play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                return 1;
            }
        }
    }
    return 0;
}

ubyte input_citymap_scanner(struct ScreenBox *p_box)
{
    int dx, dy;
    short sdx, sdy;
    ubyte ret;

    ret = 0;
    if (lbDisplay.RightButton)
    {
        short ms_x, ms_y;
        lbDisplay.RightButton = 0;
        ms_x = (lbDisplay.GraphicsScreenHeight < 400) ? 2 * lbDisplay.MouseX : lbDisplay.MouseX;
        ms_y = (lbDisplay.GraphicsScreenHeight < 400) ? 2 * lbDisplay.MouseY : lbDisplay.MouseY;
        word_1C47E6 = ms_x;
        word_1C47E8 = ms_y;
        ret = 1;
    }
    if (is_key_pressed(KC_PGDOWN, KMod_DONTCARE))
    {
        if (dword_1C47E0 > 0)
            dword_1C47E0 = -dword_1C47E0;
        dword_1C47E0--;
        dword_1C47E0 += dword_1C47E0 >> 2;
        if (dword_1C47E0 < -256)
            dword_1C47E0 = -256;
        ret = 1;
    }
    else if (is_key_pressed(KC_DELETE, KMod_DONTCARE))
    {
        if (dword_1C47E0 < 0)
            dword_1C47E0 = -dword_1C47E0;
        dword_1C47E0++;
        dword_1C47E0 += dword_1C47E0 >> 2;
        if (dword_1C47E0 > 256)
            dword_1C47E0 = 256;
        ret = 1;
    }
    else
    {
        dword_1C47E0 -= dword_1C47E0 >> 2;
        if ((dword_1C47E0 >= 0) && (dword_1C47E0 <= 4))
            dword_1C47E0--;
        if ((dword_1C47E0 <= 0) && (dword_1C47E0 >= -4))
            dword_1C47E0++;
    }
    dx = 0;
    dy = 0;
    ingame.Scanner.Angle = ((dword_1C47E0 >> 2) + ingame.Scanner.Angle) & 0x7FF;
    if (is_key_pressed(KC_RIGHT, KMod_DONTCARE)) {
        dx++;
        ret = 1;
    }
    if (is_key_pressed(KC_LEFT, KMod_DONTCARE)) {
        dx--;
        ret = 1;
    }
    if (is_key_pressed(KC_UP, KMod_DONTCARE)) {
        dy--;
        ret = 1;
    }
    if (is_key_pressed(KC_DOWN, KMod_DONTCARE)) {
        dy++;
        ret = 1;
    }
    ingame.Scanner.MX += dx * lbSinTable[ingame.Scanner.Angle + LbFPMath_PI/2] >> 13;
    ingame.Scanner.MX += dy * lbSinTable[ingame.Scanner.Angle] >> 13;
    sdx = dx * lbSinTable[ingame.Scanner.Angle] >> 13;
    sdy = dy * lbSinTable[ingame.Scanner.Angle + LbFPMath_PI/2] >> 13;
    ingame.Scanner.MZ += sdx - sdy;
    if (ingame.Scanner.MX < 0)
        ingame.Scanner.MX = 0;
    if (ingame.Scanner.MZ < 0)
        ingame.Scanner.MZ = 0;
    if (ingame.Scanner.MX > 256)
        ingame.Scanner.MX = 256;
    if (ingame.Scanner.MZ > 256)
        ingame.Scanner.MZ = 256;
    return ret;
}

ubyte input_brief_mission_text_box(struct ScreenTextBox *p_box)
{
    ubyte ret;

    if (!mouse_move_over_box(p_box))
        return 0;

    ret = 0;
    if (is_key_pressed(KC_F, KMod_DONTCARE))
    {
        clear_key_pressed(KC_F);
        if ((p_box->Flags & GBxFlg_TextCopied) == 0)
        {
            if (p_box->Font == small_font)
            {
                p_box->Font = small_med_font;
            }
            else if (p_box->Font == small_med_font)
            {
                p_box->Font = med_font;
            }
            else
            {
                p_box->Font = small_font;
            }
            p_box->Lines = 0;
            p_box->LineHeight = 0;
            p_box->Flags |= GBxFlg_Unkn0080;
        }
        ret = 1;
    }
    return ret;
}

ubyte show_citymap_box(struct ScreenBox *p_box)
{
    ubyte anim_no;

    if (selected_city_id == -1)
    {
        brief_state_city_selected = 0;
        brief_citymap_content = BriCtM_AUTO_SCANNER;
        selected_netscan_objective = -1;
    }
    if (selected_netscan_objective != byte_1C47E4)
        brief_citymap_content = BriCtM_AUTO_SCANNER;
    byte_1C47E4 = selected_netscan_objective;
    if ((brief_citymap_content == BriCtM_AUTO_SCANNER) && (selected_netscan_objective != -1))
    {
        anim_no = netscan_objectives[selected_netscan_objective].AnimNo;
        if (anim_no) {
            brief_citymap_content = BriCtM_ANIM;
            flic_netscan_open_anim(anim_no);
        }
    }
    if (!brief_state_city_selected)
    {
        lbFontPtr = small_med_font;
        my_set_text_window(p_box->X + 4, p_box->Y + 4,
            p_box->Width - 8, p_box->Height - 8);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        draw_text_purple_list2(0, 0, gui_strings[483], 0);
        brief_NETSCAN_COST_box.Text2[0] = '\0';
        brief_NETSCAN_COST_box.Text1 = gui_strings[495];
        mail_num_active_cities = 0;
        selected_city_id = -1;
        count_selectable_cities();
        if (mail_num_active_cities != 1)
        {
            show_citymap_city_selection(p_box);
            input_citymap_city_selection(p_box);
        }
        if (selected_city_id != -1)
        {
            selected_netscan_objective = -1;
            load_netscan_map(cities[selected_city_id].MapID);
            load_netscan_objectives(cities[selected_city_id].MapID, cities[selected_city_id].Level);
            load_netscan_data(selected_city_id, cities[selected_city_id].Level);
            if (cities[selected_city_id].Info != 0)
            {
                selected_netscan_objective = 0;
                brief_citymap_readd_scanner_signals();
            }
            brief_state_city_selected = 1;
        }
    }

    if (brief_citymap_content == BriCtM_ANIM)
    {
        if (mouse_move_over_box(p_box))
        {
            if (lbDisplay.LeftButton)
            {
                brief_citymap_content = BriCtM_SCANNER;
            }
        }
        anim_no = netscan_objectives[selected_netscan_objective].AnimNo;
        if (anim_no == 0)
            brief_citymap_content = BriCtM_AUTO_SCANNER;
        if (xdo_next_frame(AniSl_NETSCAN))
            brief_citymap_content = BriCtM_AUTO_SCANNER;
        draw_flic_purple_list(ac_purple_unkn2_data_to_screen);
    }
    else if (brief_state_city_selected)
    {
        if (mouse_move_over_box(p_box))
        {
            input_citymap_scanner(p_box);
        }
        draw_flic_purple_list(ac_SCANNER_data_to_screen);
        if (mail_num_active_cities != 1)
            draw_hotspot_purple_list(p_box->X + (p_box->Width >> 1), p_box->Y + (p_box->Height >> 1));
        if (mouse_move_over_box(p_box))
        {
            if (lbDisplay.LeftButton)
            {
                if (brief_citymap_content == BriCtM_SCANNER)
                    brief_citymap_content = BriCtM_AUTO_SCANNER;
                else if (mail_num_active_cities != 1)
                    brief_state_city_selected = 0;
            }
        }
    }
    return 0;
}

ubyte load_mail_text(const char *filename)
{
    int totlen;
    char *p;

    p = mission_briefing_text;
    p[0] = '\\';
    p[1] = 'c';
    p[2] = '3';

    totlen = load_file_alltext(filename, p + 3);
    if (totlen == Lb_FAIL) {
        return 0;
    }
    if (3+totlen >= mission_briefing_text_len) {
        LOGERR("Insufficient memory for mission_briefing_text - %d instead of %d",
          mission_briefing_text_len, 3+totlen);
        totlen = mission_briefing_text_len - 3 - 1;
    }
    p = mission_briefing_text;
    p[3 + totlen] = '\0';
    my_preprocess_text(p);
    return Lb_SUCCESS;
}

void brief_load_mission_info(void)
{
    char fname[FILENAME_MAX];

    set_heading_box_text(gui_strings[372]);
    data_1c4aa2 = 0;
    brief_netscan_box.Lines = 0;

    if (open_brief != 0)
    {
        if (open_brief < 0) {
            short email;
            email = -open_brief - 1;
            sprintf(fname, "%s/mail%03d.txt", "textdata", email_store[email].Mission);
        } else if (open_brief > 0) {
            ushort missi;
            missi = brief_store[open_brief - 1].Mission;
            sprintf(fname, "%s/miss%03d.txt", "textdata", mission_list[missi].SourceID);
        }
        load_mail_text(fname);
        brief_mission_text_box.Lines = 0;
    }
}

void load_netscan_data(ubyte city_id, ubyte level)
{
    int i;

    my_set_text_window(brief_netscan_box.X + 4, brief_netscan_box.ScrollWindowOffset + brief_netscan_box.Y + 4,
      brief_netscan_box.Width - 20, brief_netscan_box.ScrollWindowHeight);
    lbFontPtr = small_med_font;
    brief_netscan_box.Lines = 0;
    load_netscan_text_data(cities[city_id].MapID, level);

    for (i = 0; i < netscan_objectives_count; i++)
    {
        if (netscan_objectives[i].CreditCost == 0)
            reveal_netscan_objective(i);
    }

    if (cities[city_id].Info == 0)
    {
        for (i = 0; i < netscan_objectives_count; i++)
        {
            if (netscan_objectives[i].CreditCost)
                break;
        }
        cities[city_id].Info = i;
    }
    recount_city_credit_reward(city_id);
    update_netscan_cost_button(city_id);
}

void skip_flashy_draw_mission_screen_boxes(void)
{
    skip_flashy_draw_heading_screen_boxes();
    brief_graphical_box.Flags |= GBxFlg_Unkn0002;
    brief_NETSCAN_button.Flags |= GBxFlg_Unkn0002;
    brief_mission_text_box.Flags |= GBxFlg_Unkn0002;
    brief_netscan_box.Flags |= GBxFlg_Unkn0002;
    brief_NETSCAN_COST_box.Flags |= GBxFlg_Unkn0002;
}

ubyte show_mission_screen(void)
{
#if 0
    asm volatile ("call ASM_show_mission_screen\n"
        :  :  : "eax" );
#endif
    ubyte drawn = true;

    if (((game_projector_speed) && is_heading_flag01()) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_mission_screen_boxes();
    }
    // Draw sequentially
    if (drawn)
        drawn = draw_heading_box();

    if (drawn)
    {
        //drawn = brief_mission_text_box.DrawFn(&brief_mission_text_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&brief_mission_text_box), "g" (brief_mission_text_box.DrawFn));
    }

    if (drawn)
    {
        input_brief_mission_text_box(&brief_mission_text_box);
        //drawn = brief_graphical_box.DrawFn(&brief_graphical_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&brief_graphical_box), "g" (brief_graphical_box.DrawFn));
    }

    if (drawn)
    {
        //drawn = brief_netscan_box.DrawFn(&brief_netscan_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&brief_netscan_box), "g" (brief_netscan_box.DrawFn));
    }

    return drawn;
}

void init_brief_screen_scanner(void)
{
    ingame.Scanner.MX = 127;
    ingame.Scanner.MZ = 127;
    ingame.Scanner.Angle = 0;
    ingame.Scanner.Zoom = 256;

    SCANNER_set_screen_box(brief_graphical_box.X + 1, brief_graphical_box.Y + 1,
      brief_graphical_box.Width - 4, brief_graphical_box.Height - 3, 0);
    clear_all_scanner_signals();
    SCANNER_width = ingame.Scanner.Width;
    SCANNER_init();
}

#define SCROLL_BAR_WIDTH 12

void init_brief_screen_boxes(void)
{
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

    init_screen_text_box(&brief_netscan_box, 7, 281, 322, 145,
      6, small_med_font, 3);
    init_screen_button(&brief_NETSCAN_button, 312u, 405u,
      gui_strings[441], 6, med2_font, 1, 0x80);
    init_screen_info_box(&brief_NETSCAN_COST_box, 12u, 405u, 213u,
      gui_strings[442], unkn39_text, 6, med_font, small_med_font, 1);
    brief_NETSCAN_COST_box.Text2 = brief_netscan_cost_text;
    brief_NETSCAN_button.CallBackFn = ac_brief_do_netscan_enhance;
    brief_netscan_box.DrawTextFn = ac_show_brief_netscan_box;

    init_screen_text_box(&brief_mission_text_box, 338u, 72u, 295u, 354, 6, small_font, 3);
    init_screen_button(&unkn1_ACCEPT_button, 343u, 405u,
      gui_strings[436], 6, med2_font, 1, 0x00);
    init_screen_button(&unkn1_CANCEL_button, 616u, 405u,
      gui_strings[437], 6, med2_font, 1, 0x80);
    brief_mission_text_box.Buttons[0] = &unkn1_ACCEPT_button;
    brief_mission_text_box.Buttons[1] = &unkn1_CANCEL_button;
    brief_mission_text_box.Text = mission_briefing_text;
    unkn1_ACCEPT_button.CallBackFn = ac_accept_mission;
    unkn1_CANCEL_button.CallBackFn = ac_do_unkn1_CANCEL;

    init_screen_box(&brief_graphical_box, 7, 72, 322, 200, 6);
    brief_graphical_box.SpecialDrawFn = show_citymap_box;

    reset_brief_screen_boxes_flags();

    // Reposition the components to current resolution

    start_x = heading_box.X;
    // On the X axis, we're going for aligning below heading box, to both left and right
    space_w = heading_box.Width - brief_graphical_box.Width - brief_mission_text_box.Width;

    start_y = heading_box.Y + heading_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y - brief_mission_text_box.Height + border;

    // On the X axis, aligning to heading box left
    brief_graphical_box.X = start_x;
    brief_netscan_box.X = start_x;
    // Ot to heading box right
    brief_mission_text_box.X = brief_graphical_box.X + brief_graphical_box.Width + space_w;

    // There is one box only to Y-position in 2nd column, so space goes into two parts - before and after
    brief_mission_text_box.Y = start_y + space_h / 2;
    // The remaining boxes should be Y-aligned to the one box in 2nd column
    brief_graphical_box.Y = brief_mission_text_box.Y;
    brief_netscan_box.Y = brief_mission_text_box.Y + brief_mission_text_box.Height - brief_netscan_box.Height;

    // Boxes defining areas done; now reposition components inside

    space_w = 5;
    space_h = 5;
    brief_NETSCAN_COST_box.X = brief_netscan_box.X + space_w;
    brief_NETSCAN_COST_box.Y = brief_netscan_box.Y + brief_netscan_box.Height - brief_NETSCAN_COST_box.Height - space_h;
    // no need to update brief_NETSCAN_button.X - it will happen on the update below
    update_brief_screen_netscan_button(441);

    unkn1_ACCEPT_button.X = brief_mission_text_box.X + space_w;
    unkn1_ACCEPT_button.Y = brief_mission_text_box.Y + brief_mission_text_box.Height - unkn1_ACCEPT_button.Height - space_h;
    unkn1_CANCEL_button.X = brief_mission_text_box.X + brief_mission_text_box.Width - unkn1_CANCEL_button.Width - space_w - SCROLL_BAR_WIDTH;
    unkn1_CANCEL_button.Y = brief_mission_text_box.Y + brief_mission_text_box.Height - unkn1_CANCEL_button.Height - space_h;
}

void update_brief_screen_netscan_button(ushort text_id)
{
    const char *text;

    text = gui_strings[text_id];
    init_screen_button(&brief_NETSCAN_button,
      brief_netscan_box.X + brief_netscan_box.Width - 17, brief_NETSCAN_COST_box.Y,
      text, 6, med2_font, 1, 0x80);
    brief_NETSCAN_COST_box.Width = brief_netscan_box.Width - 10 - brief_NETSCAN_button.Width - 17;
    brief_NETSCAN_button.CallBackFn = ac_brief_do_netscan_enhance;
}

void reset_brief_screen_player_state(void)
{
    selected_netscan_objective = -1;
    old_mission_brief = 0;
}

void reset_brief_screen_boxes_flags(void)
{
    brief_NETSCAN_COST_box.Flags = GBxFlg_Unkn0001 | GBxFlg_NoBkCopy;
    brief_netscan_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
    brief_graphical_box.Flags = GBxFlg_Unkn0001;
    brief_mission_text_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
}

void set_flag01_brief_screen_boxes(void)
{
    brief_NETSCAN_button.Flags |= GBxFlg_Unkn0001;
}

/******************************************************************************/
