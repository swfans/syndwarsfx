/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feworld.c
 *     Front-end desktop and menu system, world view screen.
 * @par Purpose:
 *     Implement functions for world view screen in front-end desktop.
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
#include "feworld.h"

#include "bfkeybd.h"
#include "bfsprite.h"
#include "bftext.h"
#include "bfstrut.h"
#include "poly.h"
#include "ssampply.h"
#include <stdlib.h>

#include "display.h"
#include "femain.h"
#include "fenet.h"
#include "guiboxes.h"
#include "guitext.h"
#include "game_data.h"
#include "game_options.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "packetfe.h"
#include "purpldrw.h"
#include "purpldrwlst.h"
#include "keyboard.h"
#include "mydraw.h"
#include "network.h"
#include "sound.h"
#include "wrcities.h"
#include "swlog.h"
/******************************************************************************/
struct ScreenTextBox world_city_info_box = {0};
struct ScreenButton world_info_ACCEPT_button = {0};
struct ScreenButton world_info_CANCEL_button = {0};
struct ScreenBox world_landmap_box = {0};

extern short word_155110[6];
extern ubyte byte_15511C;// = 1;
extern short word_155744[6];
extern long landmap_8BC;
extern long landmap_8C0;
extern long landmap_8C4;
extern long landmap_8C8;
extern ulong dword_1C48D0;
extern ulong dword_1C48D4;
extern ulong dword_1C48E0[6];
extern ulong dword_1C48F8;
extern ulong dword_1C48FC;
extern ulong dword_1C4908[6];
extern ulong dword_1C4920;
extern ulong dword_1C4924;
extern ulong dword_1C4930[6];
/** whether the map screen was entered from mission brief */
extern ubyte map_from_mission;
extern ubyte byte_1C4888;
extern short word_1C488A[6];
extern short word_1C4896[6];
extern short word_1C48A2[6];
extern short word_1C48AE[6];
extern short word_1C48CC;

extern short word_1C6E08;
extern short word_1C6E0A;

ubyte ac_show_world_city_info_box(struct ScreenTextBox *box);
ubyte ac_do_unkn2_CANCEL(ubyte click);
ubyte ac_do_unkn2_ACCEPT(ubyte click);

ubyte do_unkn2_CANCEL(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_unkn2_CANCEL\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (map_from_mission)
    {
        change_screen = ChSCRT_MISBRIEF;
    }
    else
    {
        reload_background_flag = 1;
        screentype = SCRT_99;
    }
    return 0;
}

ubyte do_unkn2_ACCEPT(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_unkn2_ACCEPT\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (unkn_city_no == -1)
        return 0;

    if ((cities[unkn_city_no].Flags & 0x11) == 0)
    {
        alert_box_text_fmt("%s", gui_strings[569]);
        return 1;
    }
    map_from_mission = 0;
    start_into_mission = 1;
    return 1;
}

ubyte show_world_city_info_box(struct ScreenTextBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_world_city_info_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    ushort i;
    int n_lines;
    int tx_height, ln_height, scr_y;

    if (unkn_city_no == -1)
        return 0;

    if ((p_box->Flags & 0x0080) != 0)
    {
        for (i = 0; i < 6 ; i++) {
            word_155744[i] = -5;
        }
        p_box->Flags &= ~0x0080;
        world_info_ACCEPT_button.Flags |= 0x0001;
        world_info_CANCEL_button.Flags |= 0x0001;
    }

    if (p_box->Timer == 255)
    {
        const char *text;

        for (i = 0; i < 6 ; i++) {
            text = (const char *)&memload[cities[unkn_city_no].TextIndex[i]];
            word_155744[i] = strlen(text);
        }
        world_info_CANCEL_button.Flags |= 0x0002;
        world_info_ACCEPT_button.Flags |= 0x0002;
        p_box->Timer = -2;
    }

    lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    lbFontPtr = small_med_font;
    tx_height = my_char_height('A');
    my_set_text_window(p_box->X + 3, p_box->Y + 4, p_box->Width - 6, p_box->Height - 8);
    scr_y = 4;
    ln_height = tx_height + 4;
    for (i = 0; i < 6; i++)
    {
        const char *text;

        if (i == 4) {
            if (language_3str[0] == 'f' || (language_3str[0] == 's' && language_3str[1] != 'w'))
                continue;
        }
        if (i == 0) {
            text = gui_strings[404];
        } else {
            text = gui_strings[489 + i];
        }
        draw_text_purple_list2(0, scr_y, text, 0);
        n_lines = my_count_lines(text);
        scr_y += ln_height * n_lines;
        text = (const char *)&memload[cities[unkn_city_no].TextIndex[i]];
        flashy_draw_text(0, scr_y, text, p_box->TextSpeed, 0, &word_155744[i], 0);
        n_lines = my_count_lines(text);
        scr_y += tx_height + ln_height * n_lines;
    }
    lbDisplay.DrawFlags = 0;

    if (login_control__State != LognCt_Unkn5 && screentype == SCRT_WORLDMAP)
    {
        //world_info_ACCEPT_button.DrawFn(&world_info_ACCEPT_button); -- incompatible calling convention
        asm volatile ("call *%1\n"
            :  : "a" (&world_info_ACCEPT_button), "g" (world_info_ACCEPT_button.DrawFn));
        //world_info_CANCEL_button.DrawFn(&world_info_CANCEL_button); -- incompatible calling convention
        asm volatile ("call *%1\n"
            :  : "a" (&world_info_CANCEL_button), "g" (world_info_CANCEL_button.DrawFn));
    }
    return 0;
}

TbBool draw_world_landmass_projector_startup(struct ScreenBox *p_box)
{
    short i;

    for (i = 0; i < 6; i++)
    {
        short tx, ty;
        short x2, y2;

        tx = p_box->Timer2 * (word_1C488A[i] + p_box->X - proj_origin.X);
        ty = p_box->Timer2 * (word_1C4896[i] + p_box->Y - proj_origin.Y);
        x2 = proj_origin.X + (tx / 24);
        y2 = proj_origin.Y + (ty / 24);

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_line_purple_list(proj_origin.X, proj_origin.Y, x2, y2, 174);
        lbDisplay.DrawFlags = 0;
        draw_sprite_purple_list(x2 - 1, y2 - 1, &fe_mouseptr_sprites[12]);

        tx = p_box->Timer2 * (word_1C48A2[i] + p_box->X - proj_origin.X);
        ty = p_box->Timer2 * (word_1C48AE[i] + p_box->Y - proj_origin.Y);
        x2 = proj_origin.X + (tx / 24);
        y2 = proj_origin.Y + (ty / 24);

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_line_purple_list(proj_origin.X, proj_origin.Y, x2, y2, 174);
        lbDisplay.DrawFlags = 0;
        draw_sprite_purple_list(x2 - 1, y2 - 1, &fe_mouseptr_sprites[12]);
    }
    p_box->Timer2 += p_box->DrawSpeed;
    return (p_box->Timer2 >= 24);
}

TbBool draw_world_landmass_shape(struct ScreenBox *p_box)
{
    TbBool to_next;
    short i;

    to_next = true;
    dword_1C48E0[0] = 0x200000;
    dword_1C4908[0] = 0x200000;
    dword_1C4930[0] = 0x8000;
    dword_1C48D0 = proj_origin.X;
    dword_1C48D4 = proj_origin.Y;
    vec_mode = 17;
    vec_colour = 174;
    for (i = 0; i < 6; i++)
    {
        short k;
        short *arr;
        long tmc, tmend;

        k = word_155110[i];
        arr = dword_1C529C[i];
        tmend = word_155110[i] >> 1;
        landmap_8BC = arr[0];
        landmap_8C0 = arr[1];
        landmap_8C4 = arr[k - 2];
        landmap_8C8 = arr[k - 1];

        for (tmc = 0; tmc < p_box->Timer2; tmc += 2)
        {
            short px, py;

            if (tmc > tmend)
                break;
            // Left bottom edges of continents, actual border
            px = arr[tmc + 0];
            py = arr[tmc + 1];
            if ((landmap_8BC != 0) && (px != 0))
            {
                draw_line_purple_list(p_box->X + landmap_8BC, p_box->Y + landmap_8C0,
                  p_box->X + px, p_box->Y + py, 174);
            }
            landmap_8BC = px;
            landmap_8C0 = py;
            // Top right edges of continents, actual border
            px = arr[k - tmc - 2];
            py = arr[k - tmc - 1];
            if ((landmap_8C4 != 0) && (px != 0))
            {
                draw_line_purple_list(p_box->X + landmap_8C4, p_box->Y + landmap_8C8,
                  p_box->X + px, p_box->Y + py, 174);
            }
            landmap_8C4 = px;
            landmap_8C8 = py;
        }
        if (tmc <= tmend)
            to_next = false;
    }
    return to_next;
}

TbBool draw_world_landmass_projector(struct ScreenBox *p_box)
{
    TbBool to_next;
    short i;
    short tm_mul;

    tm_mul = 2;
    to_next = true;
    for (i = 0; i < 6; i++)
    {
        short k;
        short *arr;
        long tmc, tmn, tmend, tmiter;

        k = word_155110[i];
        arr = dword_1C529C[i];
        tmend = word_155110[i] >> 1;
        for (tmc = p_box->Timer2;; tmc += 2)
        {
            short px, py;

            // Left bottom edges of continents, projector ray
            px = arr[tmc + 0];
            py = arr[tmc + 1];
            tmiter = 2 * tm_mul + p_box->Timer2;
            if (tmc >= tmiter || tmc >= tmend)
                break;
            if ((word_1C488A[i] != 0) && (px != 0))
            {
                tmn = tmc + 2;
                if (tmn >= tmiter || tmn >= tmend)
                {
                    dword_1C48F8 = p_box->X + px;
                    dword_1C48FC = p_box->Y + py;
                    dword_1C4920 = p_box->X + word_1C488A[i];
                    dword_1C4924 = p_box->Y + word_1C4896[i];
                    draw_trig_purple_list(dword_1C48F8, dword_1C48FC,
                        dword_1C4920, dword_1C4924);
                }
                draw_line_purple_list(p_box->X + word_1C488A[i], p_box->Y + word_1C4896[i],
                  p_box->X + px, p_box->Y + py, 174);
            }
            word_1C488A[i] = px;
            word_1C4896[i] = py;
            // Top right edges of continents, projector ray
            px = arr[k - tmc - 2];
            py = arr[k - tmc - 1];
            if ((word_1C48A2[i] != 0) && px)
            {
                tmn = tmc + 2;
                if (tmn >= tmiter || tmn >= tmend)
                {
                    dword_1C48F8 = p_box->X + px;
                    dword_1C48FC = p_box->Y + py;
                    dword_1C4920 = p_box->X + word_1C48A2[i];
                    dword_1C4924 = p_box->Y + word_1C48AE[i];
                    draw_trig_purple_list(dword_1C48F8, dword_1C48FC,
                        dword_1C4920, dword_1C4924);
                }
                draw_line_purple_list(p_box->X + word_1C48A2[i], p_box->Y + word_1C48AE[i],
                  p_box->X + px, p_box->Y + py, 174);
            }
            word_1C48A2[i] = px;
            word_1C48AE[i] = py;
        }
        if (tmc < tmend)
        {
            short px, py;

            to_next = false;
            // Dots at the point of end of projector ray
            px = arr[tmc - 2];
            py = arr[tmc - 1];
            if (px != 0)
            {
                draw_sprite_purple_list(p_box->X + px - 1, p_box->Y + py - 1,
                  &fe_mouseptr_sprites[12]);
            }
            px = arr[k - tmc + 0];
            py = arr[k - tmc + 1];
            if (px)
                draw_sprite_purple_list(p_box->X + px - 1, p_box->Y + py - 1,
                  &fe_mouseptr_sprites[12]);
        }
    }
    p_box->Timer2 += 2 * tm_mul;
    return to_next;
}

TbBool draw_world_country_borders(struct ScreenBox *p_box)
{
    short i;

        i = 0;
        landmap_8BC = landmap_2B4[i + 0];
        landmap_8C0 = landmap_2B4[i + 1];
        if (byte_1C4888 == 2)
            lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        for (i = 2; i < 418; i += 2)
        {
            short px, py;

            px = landmap_2B4[i + 0];
            py = landmap_2B4[i + 1];
            if (px != 0)
                draw_line_purple_list(p_box->X + landmap_8BC, p_box->Y + landmap_8C0,
                    p_box->X + px, p_box->Y + py, 247);
            else
                i += 2;
            landmap_8BC = landmap_2B4[i + 0];
            landmap_8C0 = landmap_2B4[i + 1];
        }

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_line_purple_list(p_box->X + 1, p_box->Y + 181,
          p_box->X + 70, p_box->Y + 181, 247);
        draw_line_purple_list(p_box->X + 109, p_box->Y + 181,
          p_box->X + 247, p_box->Y + 181, 247);
        draw_line_purple_list(p_box->X + 352, p_box->Y + 181,
          p_box->X + 404, p_box->Y + 181, 247);
        draw_line_purple_list(p_box->X + 410, p_box->Y + 181,
          p_box->X + 436, p_box->Y + 181, 247);
        draw_line_purple_list(p_box->X + 446, p_box->Y + 181,
          p_box->X + 482, p_box->Y + 181, 247);
        draw_line_purple_list(p_box->X + 490, p_box->Y + 181,
            p_box->X + p_box->Width - 2, p_box->Y + 181, 247);

        lbDisplay.DrawFlags = 0;
        if (byte_1C4888 == 3)
            copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
               p_box->Width + 6, p_box->Height + 6);

    return true;
}

void draw_world_detached_city_square(struct ScreenBox *p_box, short city)
{
    lbDisplay.DrawFlags = 0;
    draw_line_purple_list(cities[city].X - 8, cities[city].Y - 8,
      cities[city].X + 8, cities[city].Y - 8, 174);
    draw_line_purple_list(cities[city].X + 8, cities[city].Y - 8,
      cities[city].X + 8, cities[city].Y + 8, 174);
    draw_line_purple_list(cities[city].X + 8, cities[city].Y + 8,
      cities[city].X - 8, cities[city].Y + 8, 174);
    draw_line_purple_list(cities[city].X - 8, cities[city].Y + 8,
      cities[city].X - 8, cities[city].Y - 8, 174);
}

void draw_world_cities_names(struct ScreenBox *p_box)
{
    int city;

    lbFontPtr = small2_font;
    for (city = 0; city < num_cities; city++)
    {
        char locstr[40];
        short ms_x, ms_y;
        short px, py;
        int k;

        ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
        ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;

        k = cities[city].TextIndex[0];
        strncpy(locstr, (char *)&memload[k], sizeof(locstr));
        LbStringToUpper(locstr);

        if (byte_15511C < 3)
        {
            short w, dim;

            draw_sprite_purple_list(cities[city].X - 1, cities[city].Y - 1, &fe_mouseptr_sprites[12]);
            if ((cities[city].Flags & CitF_Unkn20) != 0)
                draw_sprite_purple_list(cities[city].X - 4, cities[city].Y - 14, &fe_icons_sprites[169]);
            w = 4 - (gameturn & 3);
            dim = 2 * w + 1;
            if ((cities[city].Flags & CitF_Unkn01) != 0)
            {
                lbDisplay.DrawFlags |= (0x8000|Lb_SPRITE_OUTLINE);
                draw_box_purple_list(cities[city].X - w, cities[city].Y - w, dim, dim, 174);
                lbDisplay.DrawFlags &= ~(0x8000|Lb_SPRITE_OUTLINE);
                draw_hotspot_purple_list(cities[city].X, cities[city].Y);
            }
            else if ((cities[city].Flags & CitF_Unkn10) != 0)
            {
                lbDisplay.DrawFlags |= (0x8000|Lb_SPRITE_OUTLINE);
                if (gameturn & 8)
                    draw_box_purple_list(cities[city].X - w, cities[city].Y - w, dim, dim, 87);
                lbDisplay.DrawFlags &= ~(0x8000|Lb_SPRITE_OUTLINE);
                draw_hotspot_purple_list(cities[city].X, cities[city].Y);
            }
        }

        px = ms_x - cities[city].X - p_box->X - 1;
        py = ms_y - cities[city].Y - p_box->Y - 1;
        if ((cities[city].Flags & (CitF_Unkn10|CitF_Unkn01)) || (byte_15511C == 2)
           || (city == unkn_city_no) || ((px >= 0) && (py >= 0) && (px <= 3) && (py <= 3)))
        {
            const char *text;

            if ((px > 3) || (py > 3))
            {
                cities[city].Flags &= ~CitF_Unkn08;
            }
            else if ((cities[city].Flags & CitF_Unkn08) == 0)
            {
                cities[city].Flags |= CitF_Unkn08;
                play_sample_using_heap(0, 128, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
            }

            lbDisplay.DrawColour = 50;
            if ((cities[city].Flags & CitF_Unkn01) != 0)
                lbDisplay.DrawColour = 174;
            else if ((cities[city].Flags & CitF_Unkn10) != 0)
                lbDisplay.DrawColour = 87;

            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            landmap_8BC = my_string_width(locstr);
            text = loctext_to_gtext(locstr);

            if (cities[city].X + 5 + landmap_8BC < p_box->Width - 2) {
                px = cities[city].X + 5;
                py = cities[city].Y - 3;
            } else {
                px = cities[city].X - 3 - landmap_8BC;
                py = cities[city].Y - 3;
            }
            draw_text_purple_list2(px, py, text, 0);
            lbDisplay.DrawFlags = 0;
        }
        else
        {
            cities[city].Flags &= ~CitF_Unkn08;
        }
    }
}

void input_world_cities(struct ScreenBox *p_box)
{
    if (byte_15511C < 3 && lbDisplay.LeftButton && mouse_move_over_box(p_box))
    {
        short ms_x, ms_y;

        lbDisplay.LeftButton = 0;
        ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
        ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;
        landmap_8C4 = find_closest_city(ms_x - p_box->X, ms_y - p_box->Y);
        if (unkn_city_no != landmap_8C4)
        {
            unkn_city_no = landmap_8C4;
            if (login_control__State == LognCt_Unkn5) {
                login_control__City = landmap_8C4;
                net_schedule_player_city_choice_sync();
            }
            word_1C48CC = 0;
            world_city_info_box.Flags |= GBxFlg_Unkn0080;
        }
    }
}

ubyte show_world_landmap_box(struct ScreenBox *p_box)
{
    short i;
    TbBool next_state;

    if (p_box->Timer2 == 0xFFFF)
    {
        for (i = 0; i < 6; i++)
        {
            short k;
            short *arr;

            k = word_155110[i];
            arr = dword_1C529C[i];
            word_1C488A[i] = arr[0];
            word_1C48A2[i] = arr[k - 2];
            word_1C4896[i] = arr[1];
            word_1C48AE[i] = arr[k - 1];
        }
        byte_1C4888 = 1;
        p_box->Flags &= ~GBxFlg_Unkn0080;
    }

    if (is_key_pressed(KC_C, KMod_DONTCARE))
    {
        clear_key_pressed(KC_C);
        if (unkn_city_no == -1)
        {
            for (i = 0; i < num_cities; i++)
            {
                if (cities[i].Flags & (CitF_Unkn10|CitF_Unkn01)) {
                    unkn_city_no = i;
                    word_1C48CC = 0;
                }
            }
        }
        else
        {
            i = unkn_city_no + 1;
            if (i == num_cities)
                i = 0;
            while (i != unkn_city_no)
            {
                if (cities[i].Flags & (CitF_Unkn10|CitF_Unkn01)) {
                    unkn_city_no = i;
                    word_1C48CC = 0;
                    break;
                }
                i++;
                if (i == num_cities)
                    i = 0;
            }
        }
    }

    if (p_box->Flags & GBxFlg_Unkn0080)
    {
        for (i = 0; i < 6; i++)
        {
            short k;
            short *arr;

            k = word_155110[i];
            arr = dword_1C529C[i];
            word_1C488A[i] = arr[0];
            word_1C48A2[i] = arr[k - 2];
            word_1C4896[i] = arr[1];
            word_1C48AE[i] = arr[k - 1];
        }
        byte_1C4888 = 0;
        p_box->Flags &= ~GBxFlg_Unkn0080;
    }

    switch (byte_1C4888)
    {
    case 0:
        next_state = draw_world_landmass_projector_startup(p_box);
        if (next_state)
        {
            p_box->Timer2 = 0;
            byte_1C4888++;
        }
        return 0;
    case 1:
        if (!IsSamplePlaying(0, 113, 0))
            play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        next_state = draw_world_landmass_shape(p_box);
        next_state &= draw_world_landmass_projector(p_box);
        if (next_state)
        {
            byte_1C4888++;
            p_box->Timer2 = 0;
            copy_box_purple_list(p_box->X - 3, p_box->Y - 3, p_box->Width + 6, p_box->Height + 6);
        }
        return 1;
    case 2:
    case 3:
        next_state = draw_world_country_borders(p_box);
        if (next_state)
            byte_1C4888++;
        return byte_1C4888;
    case 4:
        LbScreenSetGraphicsWindow(p_box->X + 1, p_box->Y + 1,
          p_box->Width - 2, p_box->Height - 2);
        my_set_text_window(p_box->X + 1, p_box->Y + 1,
          p_box->Width - 2, p_box->Height - 2);

        if (word_1C6E08 != cities[unkn_city_no].X
         || word_1C6E0A != cities[unkn_city_no].Y)
            word_1C48CC = 0;

        if ((unkn_city_no >= 0) && (word_1C48CC == 0))
        {
            short nshift, nchange;

            landmap_8BC = cities[unkn_city_no].X - word_1C6E08;
            landmap_8C0 = cities[unkn_city_no].Y - word_1C6E0A;
            play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);

            nshift = 10;
            if (abs(landmap_8BC) < nshift)
            {
                word_1C6E08 = cities[unkn_city_no].X;
            }
            else if (landmap_8BC != 0)
            {
                nchange = nshift;
                if (landmap_8BC <= 0)
                    nchange = -nshift;
                word_1C6E08 += nchange;
            }

            if (abs(landmap_8C0) < nshift)
            {
                word_1C6E0A = cities[unkn_city_no].Y;
            }
            else if (landmap_8C0 != 0)
            {
                nchange = nshift;
                if (landmap_8C0 <= 0)
                    nchange = -nshift;
                word_1C6E0A += nchange;
            }

            if ((landmap_8BC == 0) && (landmap_8C0 == 0))
            {
                word_1C48CC = 1;
                play_sample_using_heap(0, 124, FULL_VOL, EQUL_PAN, NORM_PTCH, landmap_8BC, 1);
            }
        }

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_line_purple_list(0, word_1C6E0A, p_box->Width - 3, word_1C6E0A, 87);
        draw_line_purple_list(word_1C6E08, 0, word_1C6E08, p_box->Height - 3, 87);

        // Square around orbital station
        draw_world_detached_city_square(p_box, 29);
        break;
    default:
        return byte_1C4888;
    }

    draw_world_cities_names(p_box);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    input_world_cities(p_box);

    return 4;
}

void skip_flashy_draw_world_screen_boxes(void)
{
    skip_flashy_draw_heading_screen_boxes();
    world_landmap_box.Flags |= GBxFlg_Unkn0002;
    world_city_info_box.Flags |= GBxFlg_Unkn0002;
}

ubyte show_worldmap_screen(void)
{
    ubyte drawn = true;

    if ((game_projector_speed && is_heading_flag01()) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_world_screen_boxes();
    }

    // Draw sequentially
    if (drawn)
        drawn = draw_heading_box();

    if (drawn)
    {
        //drawn = world_landmap_box.DrawFn(&world_landmap_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&world_landmap_box), "g" (world_landmap_box.DrawFn));
    }
    if (drawn)
    {
        //drawn = world_city_info_box.DrawFn(&world_city_info_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&world_city_info_box), "g" (world_city_info_box.DrawFn));
    }
    return drawn;
}

void init_world_screen_boxes(void)
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

    init_screen_box(&world_landmap_box, 7u, 72u, 518u, 354, 6);
    init_screen_text_box(&world_city_info_box, 534u, 72u, 99u, 354, 6, small_med_font, 3);
    init_screen_button(&world_info_ACCEPT_button, 548u, 384u, gui_strings[436], 6,
        med2_font, 1, 0);
    init_screen_button(&world_info_CANCEL_button, 548u, 405u, gui_strings[437], 6,
        med2_font, 1, 0);

    world_city_info_box.DrawTextFn = ac_show_world_city_info_box;
    world_city_info_box.Flags |= GBxFlg_TextCenter;
    world_info_CANCEL_button.CallBackFn = ac_do_unkn2_CANCEL;
    world_info_ACCEPT_button.CallBackFn = ac_do_unkn2_ACCEPT;
    world_landmap_box.SpecialDrawFn = show_world_landmap_box;

    // Reposition the components to current resolution

    start_x = heading_box.X;
    // On the X axis, we're going for aligning below heading box, to both left and right
    space_w = heading_box.Width - world_landmap_box.Width - world_city_info_box.Width;

    start_y = heading_box.Y + heading_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y - world_city_info_box.Height + border;

    world_city_info_box.Y = start_y + space_h / 2;
    world_landmap_box.X = start_x;
    world_landmap_box.Y = world_city_info_box.Y;
    world_city_info_box.X = world_landmap_box.X + world_landmap_box.Width + space_w;

    space_h = 5;
    world_info_CANCEL_button.X = world_city_info_box.X
      + ((world_city_info_box.Width - world_info_CANCEL_button.Width) >> 1);
    world_info_CANCEL_button.Y = world_city_info_box.Y + world_city_info_box.Height
      - space_h - world_info_CANCEL_button.Height;
    world_info_ACCEPT_button.X = world_city_info_box.X
      + ((world_city_info_box.Width - world_info_ACCEPT_button.Width) >> 1);
    world_info_ACCEPT_button.Y = world_info_CANCEL_button.Y
      - space_h - world_info_ACCEPT_button.Height;
}

void reset_world_screen_player_state(void)
{
    unkn_city_no = -1;
    word_1C6E0A = 0;
    word_1C6E08 = 0;
}

void reset_world_screen_boxes_flags(void)
{
    world_landmap_box.Flags = GBxFlg_Unkn0001;
    world_city_info_box.Flags = GBxFlg_Unkn0001 | GBxFlg_TextCenter;
}

void set_flag01_world_screen_boxes(void)
{
    world_info_ACCEPT_button.Flags |= GBxFlg_Unkn0001;
    world_info_CANCEL_button.Flags |= GBxFlg_Unkn0001;
}

/******************************************************************************/
