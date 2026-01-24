/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fepause.c
 *     Front-end desktop and menu system, in-game pause screen.
 * @par Purpose:
 *     Implement functions for pause screen displayed in-game.
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
#include "fepause.h"

#include "bfbox.h"
#include "bfline.h"
#include "bfscreen.h"
#include "bfsprite.h"
#include "bftext.h"
#include "bftringl.h"
#include "bfkeybd.h"
#include "bfaudio.h"
#include "bfmusic.h"
#include "bfscd.h"
#include "ssampply.h"

#include "engincolour.h"

#include "bfjoyst.h"
#include "campaign.h"
#include "display.h"
#include "game_data.h"
#include "game_options.h"
#include "game_sprts.h"
#include "game.h"
#include "guiboxes.h"
#include "guitext.h"
#include "keyboard.h"
#include "mouse.h"
#include "mydraw.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
static struct ScreenBox pause_main_box;
static TbPixel ingame_boxes_colr1, ingame_boxes_colr2;
static struct ScreenBox samplevol_slider_box;
static struct ScreenBox samplevol_arrow_l_box;
static struct ScreenBox samplevol_arrow_r_box;
static struct ScreenBox midivol_slider_box;
static struct ScreenBox midivol_arrow_l_box;
static struct ScreenBox midivol_arrow_r_box;
static struct ScreenBox cdvolume_slider_box;
static struct ScreenBox cdvolume_arrow_l_box;
static struct ScreenBox cdvolume_arrow_r_box;
static struct ScreenButton detail_hi_btn;
static struct ScreenButton detail_lo_btn;
static struct ScreenButton continue_btn;
static struct ScreenButton abort_btn;

ubyte draw_ingame_button(struct ScreenButton *p_button, ubyte enabled)
{
    short text_x, text_y;
    short line_h, text_w, margin;

    margin = p_button->Border + 1;

    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbDrawBox(p_button->X - margin, p_button->Y - margin, p_button->Width + 2 * margin, p_button->Height + 2 * margin, p_button->BGColour);

    lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
    lbDisplay.DrawColour = colour_lookup[ColLU_WHITE];
    if (enabled || mouse_move_over_box(p_button))
    {
        int i;
        for (i = margin / 2; i >= 0; i--)
            LbDrawBox(p_button->X + i, p_button->Y + i, p_button->Width - 2 * i, p_button->Height - 2 * i, lbDisplay.DrawColour);
        lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
    }
    else
    {
        if (ingame.PanelPermutation == 2 || ingame.PanelPermutation == -3) {
            lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = p_button->Colour;
        }
        int i;
        for (i = margin / 2; i >= 0; i--)
            LbDrawBox(p_button->X + i, p_button->Y + i, p_button->Width - 2 * i, p_button->Height - 2 * i, p_button->Colour);
    }

    line_h = my_char_height('A');
    text_w = my_string_width(p_button->Text);
    text_x = p_button->X + 2 * margin + (p_button->Width - (text_w + 4 * margin)) / 2;
    text_y = p_button->Y + 2 * margin + (p_button->Height - (line_h + 4 * margin)) / 2;
    my_draw_text(text_x, text_y, p_button->Text, 0);

    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;

    return 0;
}

ubyte input_ingame_button(struct ScreenButton *p_button, ubyte enabled)
{
    if (enabled || mouse_move_over_box(p_button))
    {
      if (lbDisplay.LeftButton)
      {
        if (mouse_move_over_box(p_button))
            lbDisplay.LeftButton = 0;

        if (mouse_down_over_box(p_button))
            return 1;
      }
    }
    return 0;
}

TbBool input_slant_left_arrow(struct ScreenBox *box, short *target)
{
    if (mouse_move_over_slant_box(box))
    {
        if (lbDisplay.MLeftButton)
        {
            lbDisplay.LeftButton = 0;
            (*target)--;
            if ((*target) < 0)
                (*target) = 0;
            return true;
        }
    }
    return false;
}

TbBool input_slant_right_arrow(struct ScreenBox *box, short *target)
{
    if (mouse_move_over_slant_box(box))
    {
        if (lbDisplay.MLeftButton)
        {
            lbDisplay.LeftButton = 0;
            (*target)++;
            if ((*target) > STARTSCR_VOLUME_MAX)
                (*target) = STARTSCR_VOLUME_MAX;
            return true;
        }
    }
    return false;
}

TbBool input_slant_box(struct ScreenBox *box, short *target)
{
    int ms_x, ms_y;
    short border;

    border = lbDisplay.GraphicsScreenHeight < 400 ? 1 : 2;

    if (lbDisplay.MLeftButton)
    {
        ms_x = lbDisplay.MMouseX;
        ms_y = lbDisplay.MMouseY;
        if ((ms_y >= box->Y + border) && (ms_y <= box->Y + box->Height - 2 * border))
        {
            int dx, dy;
            dx = ms_x - box->X - border;
            dy = ms_y - box->Y - border;
            if (dx + dy >= 0 && dx + dy <= box->Width - 2 * border - border)
            {
                (*target) = STARTSCR_VOLUME_MAX * (dx + dy) / (box->Width - 2 * border - border);
                return true;
            }
        }
    }
    return false;
}

void draw_box_cutedge(struct ScreenBox *box, TbPixel colr1)
{
    short cut, stp;

    stp = pop1_sprites_scale;
    cut = 25 * pop1_sprites_scale;

    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    LbDrawBox(box->X + 0, box->Y + 0, box->Width - cut, box->Height - cut, colr1);
    LbDrawBox(box->X + box->Width - cut, box->Y +  cut, cut, box->Height - 2*cut, colr1);
    LbDrawBox(box->X + cut, box->Y + box->Height - cut, box->Width - cut, cut, colr1);
    LbDrawTriangle(box->X + box->Width - cut, box->Y + 0,
      box->X + box->Width, box->Y + cut,
      box->X + box->Width - cut, box->Y + cut, colr1);
    LbDrawTriangle(box->X + stp, box->Y + box->Height - cut,
      box->X + cut, box->Y + box->Height - cut,
      box->X + cut, box->Y + box->Height - stp, colr1);

    lbDisplay.DrawFlags = 0;
    LbDrawLine(box->X + 0, box->Y + 0, box->X + box->Width - cut, box->Y + 0, colr1);
    LbDrawLine(box->X + 0, box->Y + 0, box->X + 0, box->Y + box->Height - cut - stp, colr1);
    LbDrawLine(box->X + box->Width, box->Y + cut, box->X + box->Width, box->Y + box->Height - stp, colr1);
    LbDrawLine(box->X + cut, box->Y + box->Height - stp, box->X + box->Width, box->Y + box->Height - stp, colr1);
    LbDrawLine(box->X + box->Width - cut, box->Y + 0,
      box->X + box->Width, box->Y + cut, colr1);
    LbDrawLine(box->X + stp, box->Y + box->Height - cut,
      box->X + cut, box->Y + box->Height - stp, colr1);
}

void draw_slant_box(struct ScreenBox *box, TbPixel colr2)
{
    ApDrawSlantBox(box->X, box->Y, box->Width, box->Height, colr2);
}

void draw_slant_left_arrow_spr(struct ScreenBox *box, TbPixel colr2)
{
    short stp;

    stp = pop1_sprites_scale;

    if (mouse_move_over_slant_box(box))
    {
        LbSpriteDrawOneColour(box->X, box->Y - stp, &pop1_sprites[100],
          colour_lookup[ColLU_WHITE]);
    }
    else
    {
        LbSpriteDraw(box->X, box->Y - stp, &pop1_sprites[100]);
    }
}

void draw_slant_right_arrow_spr(struct ScreenBox *box, TbPixel colr2)
{
    short stp;

    stp = pop1_sprites_scale;

    if (mouse_move_over_slant_box(box))
    {
        LbSpriteDrawOneColour(box->X - stp, box->Y - stp,
          &pop1_sprites[101], colour_lookup[ColLU_WHITE]);
    }
    else
    {
        LbSpriteDraw(box->X - stp, box->Y - stp, &pop1_sprites[101]);
    }
}

void draw_slant_left_arrow(struct ScreenBox *box, TbPixel colr2)
{
    short x, y;
    short stp;
    TbPixel colour;

    if (mouse_move_over_slant_box(box))
        colour = colour_lookup[ColLU_WHITE];
    else
        colour = colr2;

    // We expect the rect to be a multiplication of 4x9; so we use height only
    stp = (box->Height + 4) / 9;

    x = box->X;
    y = box->Y - stp;

    LbDrawTriangle(x + 0, y + stp * 5, x + stp * 11, y - 1, x +  0, y + stp * 11, colour);
    ApDrawSlantBox(x + stp * 11 - 1, y + 0, stp * 2 / 2, stp * 11, colour);
}

void draw_slant_right_arrow(struct ScreenBox *box, TbPixel colr2)
{
    short x, y;
    short stp;
    TbPixel colour;

    if (mouse_move_over_slant_box(box))
        colour = colour_lookup[ColLU_WHITE];
    else
        colour = colr2;

    // We expect the rect to be a multiplication of 4x9; so we use height only
    stp = (box->Height + 4) / 9;

    x = box->X;
    y = box->Y - stp;

    ApDrawSlantBox(x + stp * 10 - 1, y + 0, stp * 2 / 2, stp * 11, colour);
    LbDrawTriangle(x + stp * 11, y + stp * 5 + 1, x + 0, y + stp * 11, x +  stp * 11, y + 0, colour);
}

void init_slider_with_arrows_centered(struct ScreenBox *slider_box, struct ScreenBox *arrow_l_box,
  struct ScreenBox *arrow_r_box, const struct ScreenBox *parent_box, int y, int slider_w, int arrow_w, int cutin_w, int h)
{
    slider_box->Width = slider_w;
    slider_box->Height = h;
    slider_box->X = parent_box->X + (parent_box->Width - slider_box->Width) / 2 + 4;
    slider_box->Y = y;

    arrow_l_box->Width = arrow_w;
    arrow_l_box->Height = h;
    arrow_l_box->X = slider_box->X - arrow_w - 2 * cutin_w;
    arrow_l_box->Y = slider_box->Y;

    arrow_r_box->Width = arrow_w;
    arrow_r_box->Height = h;
    arrow_r_box->X = slider_box->X + slider_box->Width - cutin_w;
    arrow_r_box->Y = slider_box->Y;
}

void init_ingame_screen_button(struct ScreenButton *p_button, ushort x, ushort y, const char *text, struct TbSprite *font, int flags)
{
    short line_h, text_w, border;
    short fnt_scale, margin;

    init_screen_button(p_button, x, y, text, 0, font, 0, 0);

    border = pop1_sprites_scale - 1;
    line_h = my_char_height('A');
    text_w = my_string_width(p_button->Text);
    fnt_scale = pop1_sprites_scale / 2 + 1;
    if (line_h < fnt_scale * 6) { // detail 0 font has height equal 6
        fnt_scale = (fnt_scale * 6) / line_h;
        line_h *= fnt_scale;
        text_w *= fnt_scale;
    }

    p_button->Border = border;
    p_button->BGColour = ingame_boxes_colr1;
    p_button->Colour = ingame_boxes_colr2;

    margin = p_button->Border + 1;
    p_button->Width = text_w + 4 * margin;
    p_button->Height = line_h + 4 * margin;
    p_button->X -= (p_button->Width >> 1);
}

void init_pause_screen_boxes(void)
{
    short slider_w, arrow_w, cutin_w, slider_h;
    int x, y;

    if ((ingame.PanelPermutation != 2) && (ingame.PanelPermutation != -3)) {
        ingame_boxes_colr1 = 20;
        ingame_boxes_colr2 = 15;
    } else {
        ingame_boxes_colr1 = 40;
        ingame_boxes_colr2 = 35;
    }

    pause_main_box.Width = 233 * pop1_sprites_scale;
    pause_main_box.Height = 122 * pop1_sprites_scale;
    cutin_w = 5 * pop1_sprites_scale;
    slider_h = 9 * pop1_sprites_scale;

    pause_main_box.X = (lbDisplay.GraphicsScreenWidth - pause_main_box.Width) / 2;
    pause_main_box.Y = (lbDisplay.GraphicsScreenHeight - pause_main_box.Height) / 2;

    slider_w = pause_main_box.Width * 131 / 256;
    arrow_w = slider_w * 9 / 256;

    y = pause_main_box.Y + pause_main_box.Height * 17 / 60;
    init_slider_with_arrows_centered(&samplevol_slider_box, &samplevol_arrow_l_box,
      &samplevol_arrow_r_box, &pause_main_box, y, slider_w, arrow_w, cutin_w, slider_h);

    y += pause_main_box.Height * 11 / 60;
    init_slider_with_arrows_centered(&midivol_slider_box, &midivol_arrow_l_box,
      &midivol_arrow_r_box, &pause_main_box, y, slider_w, arrow_w, cutin_w, slider_h);

    y += pause_main_box.Height * 11 / 60;
    init_slider_with_arrows_centered(&cdvolume_slider_box, &cdvolume_arrow_l_box,
      &cdvolume_arrow_r_box, &pause_main_box, y, slider_w, arrow_w, cutin_w, slider_h);

    x = pause_main_box.X + pause_main_box.Width * 67 / 114;
    y = pause_main_box.Y + pause_main_box.Height * 46 / 60;
    init_ingame_screen_button(&detail_hi_btn, x, y, gui_strings[477], small_font, 0);

    x = pause_main_box.X + pause_main_box.Width * 87 / 114;
    y = pause_main_box.Y + pause_main_box.Height * 46 / 60;
    init_ingame_screen_button(&detail_lo_btn, x, y, gui_strings[475], small_font, 0);

    x = pause_main_box.X + pause_main_box.Width * 47 / 114;
    y = pause_main_box.Y + pause_main_box.Height * 53 / 60;
    init_ingame_screen_button(&continue_btn, x, y, gui_strings[455], small_font, 0);

    x = pause_main_box.X + pause_main_box.Width * 76 / 114;
    y = pause_main_box.Y + pause_main_box.Height * 53 / 60;
    init_ingame_screen_button(&abort_btn, x, y, gui_strings[445], small_font, 0);
}

void start_pause_screen(void)
{
    snd_unkn1_volume_all_samples();
    update_danger_music(2);

    do_change_mouse(8);

    pause_main_box.Flags &= ~GBxFlg_BkgndDrawn;

}

/** Wait for the pause toggle key to be released.
 */
void wait_for_keypress_end(ushort game_key, TbBool impatient)
{
    TbBool ended;

    if (impatient) {
        clear_gamekey_pressed(game_key);
    }

    ended = false;
    while (!ended)
    {
        joy_update_inputs(&joy);

        if (!is_gamekey_pressed(game_key))
            ended = true;

        PlayCDTrack(ingame.CDTrack);

        swap_wscreen();
        game_update();
    }
}

void draw_pause_screen_static(struct ScreenBox *box)
{
    const char *s;
    int x, y, w;

    draw_box_cutedge(box, ingame_boxes_colr1);

    lbFontPtr = small_font;
    my_set_text_window(0, 0, lbDisplay.PhysicalScreenWidth,
      lbDisplay.PhysicalScreenHeight);

    if ((ingame.PanelPermutation == 2) || (ingame.PanelPermutation == -3)) {
        lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
        lbDisplay.DrawColour = ingame_boxes_colr2;
    }
    {
        s = mission_name;
        w = my_string_width(s);
        x = pause_main_box.X + (pause_main_box.Width - w) / 2 + pause_main_box.Width / 114;
        y = pause_main_box.Y + pause_main_box.Height * 9 / 60;
        my_draw_text(x, y, s, 0);

        s = gui_strings[419]; // "sound volume"
        w = my_string_width(s);
        x = pause_main_box.X + (pause_main_box.Width - w) / 2 + pause_main_box.Width / 114;
        y += pause_main_box.Height * 5 / 60;
        my_draw_text(x, y, s, 0);

        s = gui_strings[420]; // "tension volume"
        w = my_string_width(s);
        x = pause_main_box.X + (pause_main_box.Width - w) / 2 + pause_main_box.Width / 114;
        y += pause_main_box.Height * 11 / 60;
        my_draw_text(x, y, s, 0);

        s = gui_strings[516]; // "cd volume"
        w = my_string_width(s);
        x = pause_main_box.X + (pause_main_box.Width - w) / 2 + pause_main_box.Width / 114;
        y += pause_main_box.Height * 11 / 60;
        my_draw_text(x, y, s, 0);
    }
    if (ingame.PanelPermutation == 2 || ingame.PanelPermutation == -3) {
        lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    }

    if (language_3str[0] == 'e')
    {
        // Special sprites with large text are only available for some languages
        struct TbSprite *p_spr;

        if (lbDisplay.GraphicsScreenHeight < 400)
        {
            p_spr = &pop1_sprites[102]; // "uplink paused"
            w = p_spr->SWidth;
            x = pause_main_box.X + (pause_main_box.Width - w) / 2;
            y = pause_main_box.Y + pause_main_box.Height * 2 / 60;
            LbSpriteDraw(x, y, p_spr);
        }
        else
        {
            w = pop1_sprites[102].SWidth + pop1_sprites[104].SWidth;
            p_spr = &pop1_sprites[102]; // "uplink"
            x = pause_main_box.X + (pause_main_box.Width - w) / 2;
            y = pause_main_box.Y + pause_main_box.Height * 2 / 60;
            LbSpriteDraw(x, y, p_spr);
            x += p_spr->SWidth;
            p_spr = &pop1_sprites[104]; // "paused"
            LbSpriteDraw(x, y, p_spr);

        }
        p_spr = &pop1_sprites[103]; // "detail"
        w = p_spr->SWidth;
        x = pause_main_box.X + (pause_main_box.Width / 2 - w) / 2 + pause_main_box.Width * 4 / 114;
        y = pause_main_box.Y + pause_main_box.Height * 46 / 60;
        LbSpriteDraw(x, y, p_spr);
    }
    else
    {
        // If no special sprites, draw the text in the normal font available in-game
        s = gui_strings[633]; // "uplink paused"
        w = my_string_width(s);
        x = pause_main_box.X + (pause_main_box.Width - w) / 2 + 3;
        y = pause_main_box.Y + pause_main_box.Height * 2 / 60;
        my_draw_text(x, y, s, 0);

        s = gui_strings[470]; // "detail"
        w = my_string_width(s);
        x = pause_main_box.X + (pause_main_box.Width / 2 - w) / 2 + pause_main_box.Width * 4 / 114;
        y = pause_main_box.Y + pause_main_box.Height * 48 / 60;
        my_draw_text(x, y, s, 0);
    }
}

void draw_pause_volume_bar(struct ScreenBox *p_box1, struct ScreenBox *p_box2, struct ScreenBox *p_box3, short *p_target)
{
    // Draw the main slider box
    draw_slant_box(p_box1, ingame_boxes_colr2);
    // Draw the side arrows
    draw_slant_left_arrow(p_box2, ingame_boxes_colr2);
    draw_slant_right_arrow(p_box3, ingame_boxes_colr2);

    if (*p_target) // Draw slider box filling
    {
        struct ScreenBox box4;
        short stp;

        stp = pop1_sprites_scale+1;
        box4.X = p_box1->X + 1 * stp;
        box4.Y = p_box1->Y + 1 * stp;
        box4.Width = (p_box1->Width - 3 * stp) * (*p_target) / STARTSCR_VOLUME_MAX;
        box4.Height = p_box1->Height - 2 * stp;
        draw_slant_box(&box4, colour_lookup[ColLU_WHITE]);
    }
}

ubyte show_pause_screen(struct ScreenBox *box)
{
    if ((box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        draw_pause_screen_static(box);
        box->Flags |= GBxFlg_BkgndDrawn;
    }

    lbDisplay.DrawFlags = 0;
    draw_pause_volume_bar(&samplevol_slider_box, &samplevol_arrow_l_box, &samplevol_arrow_r_box, &startscr_samplevol);
    draw_pause_volume_bar(&midivol_slider_box, &midivol_arrow_l_box, &midivol_arrow_r_box, &startscr_midivol);
    draw_pause_volume_bar(&cdvolume_slider_box, &cdvolume_arrow_l_box, &cdvolume_arrow_r_box, &startscr_cdvolume);

    draw_ingame_button(&detail_hi_btn, (ingame.DetailLevel == 1));
    draw_ingame_button(&detail_lo_btn, (ingame.DetailLevel == 0));
    draw_ingame_button(&continue_btn, 0);
    draw_ingame_button(&abort_btn, 0);

    return 0;
}

void *input_pause_screen(void)
{
    short *target;
    void *affected;

    affected = NULL;

    {
    target = &startscr_samplevol;

    if (input_slant_left_arrow(&samplevol_arrow_l_box, target))
        affected = target;
    if (input_slant_right_arrow(&samplevol_arrow_r_box, target))
        affected = target;
    if (input_slant_box(&samplevol_slider_box, target))
        affected = target;
    }

    {
    target = &startscr_midivol;

    if (input_slant_left_arrow(&midivol_arrow_l_box, target))
        affected = target;
    if (input_slant_right_arrow(&midivol_arrow_r_box, target))
        affected = target;
    if (input_slant_box(&midivol_slider_box, target))
        affected = target;
    }

    {
    target = &startscr_cdvolume;

    if (input_slant_left_arrow(&cdvolume_arrow_l_box, target))
        affected = target;
    if (input_slant_right_arrow(&cdvolume_arrow_r_box, target))
        affected = target;
    if (input_slant_box(&cdvolume_slider_box, target))
        affected = target;
    }

    {
    target = &startscr_cdvolume;

        if (input_ingame_button(&detail_hi_btn, ingame.DetailLevel == 1)) {
            ingame.DetailLevel = 1;
            affected = target;
        }
        if (input_ingame_button(&detail_lo_btn, ingame.DetailLevel == 0)) {
            ingame.DetailLevel = 0;
            affected = target;
        }
    }

    return affected;
}

TbBool pause_screen_handle(void)
{
    void *affected;
    TbBool resume_game;

    init_pause_screen_boxes();
    start_pause_screen();

    // Wait for the pause key to be released
    wait_for_keypress_end(GKey_PAUSE, true);

    show_pause_screen(&pause_main_box);

    swap_wscreen();
    if (!ingame.fld_unk7DA)
        SetMusicVolume(100, 0x7F);

    resume_game = false;
    while (!resume_game)
    {
        joy_update_inputs(&joy);

        affected = input_pause_screen();

        if (affected == &startscr_samplevol)
        {
            sfx_apply_samplevol();
            if (!IsSamplePlaying(0, 80, 0))
                play_sample_using_heap(0, 80, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        }
        else if (affected == &startscr_midivol)
        {
            sfx_apply_midivol();
        }
        else if (affected == &startscr_cdvolume)
        {
            sfx_apply_cdvolume();
        }
        else if (affected == &ingame.DetailLevel)
        {
            bang_set_detail(1 - ingame.DetailLevel);
        }

        show_pause_screen(&pause_main_box);

        if (input_ingame_button(&continue_btn, 0))
            resume_game = true;

        if (input_ingame_button(&abort_btn, 0))
        {
            swap_wscreen();
            SetMusicVolume(100, 0);
            StopAllSamples();
            StopCD();
            return 1;
        }

        if (is_gamekey_pressed(GKey_PAUSE))
            resume_game = true;

        lbDisplay.DrawFlags = 0;
        process_sound_heap();
        PlayCDTrack(ingame.CDTrack);

        swap_wscreen();
        game_update();
    }

    // Wait for the pause key to be released
    wait_for_keypress_end(GKey_PAUSE, false);

    lbDisplay.RightButton = 0;
    lbDisplay.LeftButton = 0;
    if (!ingame.fld_unk7DA)
        SetMusicVolume(100, 0);
    return 0;
}

/******************************************************************************/
