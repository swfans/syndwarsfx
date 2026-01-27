/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feoptions.c
 *     Front-end desktop and menu system, options screen.
 * @par Purpose:
 *     Implement functions for options screen in front-end desktop.
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
#include "feoptions.h"

#include "bftext.h"
#include "bfsprite.h"
#include "bfkeybd.h"
#include "bfaudio.h"
#include "bfscd.h"
#include <assert.h>
#include <stdlib.h>

#include "bfjoyst.h"
#include "engindrwlstm.h"
#include "femain.h"
#include "feshared.h"
#include "guiboxes.h"
#include "guitext.h"
#include "display.h"
#include "game_options.h"
#include "game_sprts.h"
#include "game.h"
#include "mydraw.h"
#include "purpldrw.h"
#include "purpldrwlst.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
/** How many pixels the slider is spill outside of its active rect area.
 */
#define HORIZ_PROSLIDER_MAIN_SPILL_W 6
#define GFX_TOGGLE_OPTIONS_COUNT 7
#define GFX_MULTIVAL_OPTIONS_COUNT 2

struct ScreenBox audio_tracks_box = {0};
struct ScreenBox audio_volume_boxes[3] = {0};
struct ScreenButton options_audio_buttons[7] = {0};
struct ScreenShape audio_volume_sliders[9] = {0};
ubyte audio_volume_sliders_draw_state[3] = {0};

struct ScreenBox options_gfx_box = {0};
struct ScreenButton options_gfx_buttons[GFX_TOGGLE_OPTIONS_COUNT * 2 + GFX_MULTIVAL_OPTIONS_COUNT] = {0};
struct ScreenButton options_gfx_labels[GFX_TOGGLE_OPTIONS_COUNT+GFX_MULTIVAL_OPTIONS_COUNT] = {0};

extern short word_1C4866[3];

extern short textpos[10];

/******************************************************************************/

ubyte ac_change_panel_permutation(ubyte click);
ubyte ac_change_trenchcoat_preference(ubyte click);
ubyte ac_show_options_visual_main_box(struct ScreenBox *box);
ubyte ac_flashy_draw_purple_label(struct ScreenButton *p_button);

void show_audio_volume_box_func_02(short scr_x, short scr_y, short a3, short a4, TbPixel colour)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_show_audio_volume_box_func_02\n"
        : : "a" (scr_x), "d" (scr_y), "b" (a3), "c" (a4), "g" (colour));
#endif
    short i;
    int cx, cy;

    if (a3 == 0) {
        return;
    }

    if (a4 <= a3)
    {
        cx = scr_x - a4;
        cy = scr_y + a4;
        draw_triangle_purple_list(scr_x, scr_y, scr_x, cy, cx, cy, colour);
        draw_triangle_purple_list(scr_x + a3, scr_y, cx + a3, cy, cx + a3, scr_y, colour);
        draw_box_purple_list(scr_x, scr_y, a3 - a4, a4, colour);
    }
    else
    {
        for (i = 0; i < a4; i++)
        {
            cy = scr_y + i;
            cx = scr_x - i;
            draw_line_purple_list(cx, cy, cx + a3 - 1, cy, colour);
        }
    }
}

void draw_horiz_proslider_main_body(struct ScreenShape *p_shp, short *p_value)
{
    struct ScreenBox box;
    short thick_width, thin_height, x2c;
    short wtext1, wtext2, spill_w;
    short val;

    // Size and coords of a box around the shape
    spill_w = HORIZ_PROSLIDER_MAIN_SPILL_W;
    box.Width = p_shp->PtX[4] - p_shp->PtX[0] - 2 * spill_w;
    box.Height = p_shp->PtY[0] - p_shp->PtY[3];
    box.X = p_shp->PtX[0] + spill_w;
    box.Y = p_shp->PtY[3];
    // text in cut-ins at start and at end of slider
    wtext1 = p_shp->PtX[2] - p_shp->PtX[1];
    wtext2 = p_shp->PtX[5] - p_shp->PtX[6];
    // Compute the scaled slider value; subtract half a height to have a proper input value range
    val = *p_value * (box.Width - (box.Height+1)/2) / STARTSCR_VOLUME_MAX;

    lbDisplay.DrawFlags = 0;
    thick_width = p_shp->PtX[7] - p_shp->PtX[3] + box.Height + 2;
    thin_height = p_shp->PtY[0] - p_shp->PtY[1];
    if (val < wtext1)
    {
        x2c = val;
        show_audio_volume_box_func_02(box.X + 9, box.Y + 11, x2c, thin_height+1, 0x0AE);
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        show_audio_volume_box_func_02(box.X + 9 + x2c, box.Y + 11, wtext1 - x2c, thin_height+1, 0x0AE);
        show_audio_volume_box_func_02(box.X + 20 + wtext1, box.Y, thick_width, box.Height+1, 0x0AE);
        show_audio_volume_box_func_02(box.X + box.Width + spill_w + 1 - wtext2, box.Y, wtext2, thin_height+1, 0x0AE);
    }
    else if (val >= box.Width - (box.Height+1) / 2 - wtext2)
    {
        x2c = val - (box.Width - (box.Height+1) / 2 - wtext2);
        show_audio_volume_box_func_02(box.X + 9, box.Y + 11, wtext1, thin_height+1, 0x0AE);
        show_audio_volume_box_func_02(box.X + 20 + wtext1, box.Y, thick_width, box.Height+1, 0x0AE);
        show_audio_volume_box_func_02(box.X + box.Width + spill_w + 1 - wtext2, box.Y, x2c, thin_height+1, 0x0AE);
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        show_audio_volume_box_func_02(box.X + box.Width + spill_w + 1 + x2c - wtext2, box.Y, wtext2 - x2c, thin_height+1, 0x0AE);
    }
    else
    {
        x2c = val - wtext1;
        show_audio_volume_box_func_02(box.X + 9, box.Y + 11, wtext1, thin_height+1, 0x0AE);
        show_audio_volume_box_func_02(box.X + 20 + wtext1, box.Y, x2c, box.Height+1, 0x0AE);
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        show_audio_volume_box_func_02(box.X + 20 + x2c + wtext1, box.Y, thick_width - x2c, box.Height+1, 0x0AE);
        show_audio_volume_box_func_02(box.X + box.Width + spill_w + 1 - wtext2, box.Y, wtext2, thin_height+1, 0x0AE);
    }
}

void draw_horiz_proslider_main_body_text(struct ScreenShape *p_shp, struct ScreenBox *p_tbox, short *p_value)
{
    struct ScreenBox box;
    const char *text;
    short wtext2, shift_y, spill_w;

    // Size and coords of a box around the shape
    spill_w = HORIZ_PROSLIDER_MAIN_SPILL_W;
    box.Width = p_shp->PtX[4] - p_shp->PtX[0] - 2 * spill_w;
    box.Height = p_shp->PtY[0] - p_shp->PtY[3];
    // Text drawing area is bound to a text box, so make the coords relative to text box position
    box.X = p_shp->PtX[0] - p_tbox->X + spill_w;
    box.Y = p_shp->PtY[3] - p_tbox->Y;
    // text in cut-ins at start and at end of slider
    wtext2 = p_shp->PtX[5] - p_shp->PtX[6];

    lbDisplay.DrawFlags = 0;
    text = gui_strings[421];
    draw_text_purple_list2(box.X + 9, box.Y - 2, text, 0);
    text = gui_strings[422];
    shift_y = box.Height - my_char_height('A');
    draw_text_purple_list2(box.X + box.Width - 13 - wtext2, box.Y + shift_y - 4, text, 0);
}

TbBool input_horiz_proslider_main_body(struct ScreenShape *p_shp, short *p_value)
{
    struct ScreenBox box;
    short spill_w;
    TbBool target_affected;
    short val;

    // Size and coords of a box around the shape
    spill_w = HORIZ_PROSLIDER_MAIN_SPILL_W;
    box.Width = p_shp->PtX[4] - p_shp->PtX[0] - 2 * spill_w;
    box.Height = p_shp->PtY[0] - p_shp->PtY[3];
    box.X = p_shp->PtX[0] + spill_w;
    box.Y = p_shp->PtY[3];

    target_affected = false;
    if (mouse_move_over_box(&box))
    {
        if (lbDisplay.MLeftButton)
        {
            short delta_y, delta_x;

            lbDisplay.LeftButton = 0;
            delta_y = mouse_move_y_coord_over_box(&box) - 14;
            delta_x = mouse_move_position_horizonal_over_box(&box) - 9;
            val = delta_y + delta_x;
            *p_value = val * STARTSCR_VOLUME_MAX / (box.Width - (box.Height+1) / 2);
            if ((*p_value) < 0)
                *p_value = 0;
            else if ((*p_value) > STARTSCR_VOLUME_MAX)
                (*p_value) = STARTSCR_VOLUME_MAX;
            target_affected = 1;
        }
    }
    return target_affected;
}

void draw_horiz_proslider_left_arrow(struct ScreenShape *p_shp)
{
    struct ScreenBox box, actbox;
    struct TbSprite *p_spr;

    box.Width = p_shp->PtX[2] - p_shp->PtX[0] + 1;
    box.Height = p_shp->PtY[0] - p_shp->PtY[2] + 1;
    box.X = p_shp->PtX[0];
    box.Y = p_shp->PtY[2];
    // For active area, cut off the low populated areas - arrow corners
    actbox.Width = box.Width * 3 / 5;
    actbox.Height = box.Height * 7 / 8;
    actbox.X = box.X;
    actbox.Y = box.Y;

    p_spr = &fe_icons_sprites[108];

    lbDisplay.DrawFlags |= 0x8000;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    if (mouse_move_over_box(&actbox))
    {
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    }
    draw_sprite_purple_list(box.X, box.Y, p_spr);
}

TbBool input_horiz_proslider_left_arrow(struct ScreenShape *p_shp, short *p_value)
{
    struct ScreenBox box, actbox;
    TbBool target_affected;

    box.Width = p_shp->PtX[2] - p_shp->PtX[0] + 1;
    box.Height = p_shp->PtY[0] - p_shp->PtY[2] + 1;
    box.X = p_shp->PtX[0];
    box.Y = p_shp->PtY[2];
    // For active area, cut off the low populated areas - arrow corners
    actbox.Width = box.Width * 3 / 5;
    actbox.Height = box.Height * 7 / 8;
    actbox.X = box.X;
    actbox.Y = box.Y;

    target_affected = false;
    // matching mouse move coords is more likely to do what
    // the user wnated than cheching mouse dow coords
    if (mouse_move_over_box(&actbox))
    {
        if (lbDisplay.MLeftButton || joy.Buttons[0])
        {
            lbDisplay.LeftButton = 0;
            p_shp->Flags |= GBxFlg_IsRPushed;
            if ((lbShift & KMod_SHIFT) != 0)
                (*p_value)--;
            else
                (*p_value) -= 10;
            if ((*p_value) < 0)
                (*p_value) = 0;
            target_affected = 1;
        }
    }
    return target_affected;
}

void draw_horiz_proslider_right_arrow(struct ScreenShape *p_shp)
{
    struct ScreenBox box, actbox;
    struct TbSprite *p_spr;

    box.Width = p_shp->PtX[2] - p_shp->PtX[4] + 1;
    box.Height = p_shp->PtY[4] - p_shp->PtY[2] + 1;
    box.X = p_shp->PtX[4];
    box.Y = p_shp->PtY[2];
    // For active area, cut off the low populated areas - arrow corners
    actbox.Width = box.Width * 3 / 5;
    actbox.X = box.X + box.Width - actbox.Width;
    actbox.Height = box.Height * 7 / 8;
    actbox.Y = box.Y + box.Height - actbox.Height;

    p_spr = &fe_icons_sprites[109];

    lbDisplay.DrawFlags |= 0x8000;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    if (mouse_move_over_box(&actbox))
    {
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    }
    draw_sprite_purple_list(box.X, box.Y, p_spr);
}

TbBool input_horiz_proslider_right_arrow(struct ScreenShape *p_shp, short *p_value)
{
    struct ScreenBox box, actbox;
    TbBool target_affected;

    box.Width = p_shp->PtX[2] - p_shp->PtX[4] + 1;
    box.Height = p_shp->PtY[4] - p_shp->PtY[2] + 1;
    box.X = p_shp->PtX[4];
    box.Y = p_shp->PtY[2];
    // For active area, cut off the low populated areas - arrow corners
    actbox.Width = box.Width * 3 / 5;
    actbox.X = box.X + box.Width - actbox.Width;
    actbox.Height = box.Height * 7 / 8;
    actbox.Y = box.Y + box.Height - actbox.Height;

    target_affected = false;
    // matching mouse move coords is more likely to do what
    // the user wnated than cheching mouse dow coords
    if (mouse_move_over_box(&actbox))
    {
        if (lbDisplay.MLeftButton || joy.Buttons[0])
        {
            lbDisplay.LeftButton = 0;
            p_shp->Flags |= GBxFlg_IsRPushed;
            if ((lbShift & KMod_SHIFT) != 0)
                (*p_value)++;
            else
                (*p_value) += 10;
            if ((*p_value) > STARTSCR_VOLUME_MAX)
                (*p_value) = STARTSCR_VOLUME_MAX;
            target_affected = 1;
        }
    }
    return target_affected;
}

void update_options_gfx_state(void)
{
    const char *text;
    int i, val;

    i = GFX_TOGGLE_OPTIONS_COUNT * 2 + 0;
    val = ingame.PanelPermutation;
    if (val < 0)
        text = gui_strings[579 + abs(val)];
    else
        text = gui_strings[580 + val];
    options_gfx_buttons[i].Text = text;

    i = GFX_TOGGLE_OPTIONS_COUNT * 2 + 1;
    val = ingame.TrenchcoatPreference;
    text = gui_strings[583 + val];
    options_gfx_buttons[i].Text = text;
}

ubyte show_options_visual_main_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_options_visual_main_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    int scr_x, scr_y;
    int i;

    if ((p_box->Flags & 0x0080) != 0) {
        p_box->Flags &= ~0x0080;
    }

    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);

    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT+GFX_MULTIVAL_OPTIONS_COUNT; i++)
    {
        ubyte drawn;
        //drawn = options_gfx_labels[i].DrawFn(&options_gfx_labels[i]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&options_gfx_labels[i]), "g" (options_gfx_labels[i].DrawFn));
        if (drawn < 2) break;
    }

    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT * 2 + GFX_MULTIVAL_OPTIONS_COUNT; i++)
    {
        //options_gfx_buttons[i].DrawFn(&options_gfx_buttons[i]); -- incompatible calling convention
        asm volatile ("call *%1\n"
            : : "a" (&options_gfx_buttons[i]), "g" (options_gfx_buttons[i].DrawFn));
    }

    for (i = GFX_TOGGLE_OPTIONS_COUNT * 2; i < GFX_TOGGLE_OPTIONS_COUNT * 2 + GFX_MULTIVAL_OPTIONS_COUNT; i++)
    {
      scr_x = options_gfx_buttons[i].X - 19;
      scr_y = options_gfx_buttons[i].Y + 1;
      lbDisplay.DrawFlags = 0x8000 | Lb_SPRITE_TRANSPAR4;

      if (mouse_move_over_box_coords(scr_x, scr_y, scr_x + 9, scr_y + 14))
      {
          if (lbDisplay.MLeftButton || joy.Buttons[0])
          {
              lbDisplay.LeftButton = 0;
              switch (i)
              {
              case GFX_TOGGLE_OPTIONS_COUNT * 2 + 1:
                  game_option_dec(GOpt_TrenchcoatPreference);
                  break;
              case GFX_TOGGLE_OPTIONS_COUNT * 2 + 0:
                  game_option_dec(GOpt_PanelPermutation);
                  break;
              }
              update_options_gfx_state();
          }
          lbDisplay.DrawFlags = 0x8000;
          draw_sprite_purple_list(scr_x, scr_y, &fe_icons_sprites[108]);
          lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
      }
      else
      {
          draw_sprite_purple_list(scr_x, scr_y, &fe_icons_sprites[108]);
      }
      scr_x = options_gfx_buttons[i].X + options_gfx_buttons[i].Width + 10;

      if (mouse_move_over_box_coords(scr_x, scr_y, scr_x + 9, scr_y + 14))
      {
          if ( lbDisplay.MLeftButton || joy.Buttons[0] )
          {
              lbDisplay.LeftButton = 0;
              switch (i)
              {
              case 15:
                  game_option_inc(GOpt_TrenchcoatPreference);
                  break;
              case 14:
                  game_option_inc(GOpt_PanelPermutation);
                  break;
              }
              update_options_gfx_state();
          }
          lbDisplay.DrawFlags = 0x8000;
      }
      draw_sprite_purple_list(scr_x - 7, scr_y, &fe_icons_sprites[109]);
    }
    lbDisplay.DrawFlags = 0;
    apply_user_gfx_settings();
    return 0;
}

ubyte show_audio_volume_box(struct ScreenBox *p_box)
{
    short *target_ptr;
    const char *s;
    ushort w;
    ubyte target_var;
    TbBool change;
    ubyte text_drawn, shapes_drawn;

    change = false;
    if (p_box->Flags & GBxFlg_Unkn0080)
    {
        p_box->Flags &= ~GBxFlg_Unkn0080;
        word_1C4866[0] = -5;
        word_1C4866[1] = -5;
        word_1C4866[2] = -5;
    }
    if (p_box->Timer == 255)
    {
        word_1C4866[0] = 99;
        word_1C4866[1] = 99;
        word_1C4866[2] = 99;
    }
    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 6, 480);

    if (p_box == &audio_volume_boxes[0])
    {
        target_ptr = &startscr_samplevol;
        s = game_option_desc(GOpt_SampleVolume);
        target_var = 0;
    }
    else if (p_box == &audio_volume_boxes[1])
    {
        target_ptr = &startscr_midivol;
        s = game_option_desc(GOpt_DangerVolume);
        target_var = 1;
    }
    else
    {
        target_ptr = &startscr_cdvolume;
        s = game_option_desc(GOpt_CDAVolume);
        target_var = 2;
    }

    if ((audio_volume_sliders[3 * target_var + 1].Flags & GBxFlg_Unkn0001) != 0)
    {
        audio_volume_sliders_draw_state[target_var] = 0;
    }

    lbFontPtr = med_font;
    w = (p_box->Width - my_string_width(s)) >> 1;
    text_drawn = flashy_draw_text(1 + w, 1, s, 1, 0, &word_1C4866[target_var], 0);

    if (audio_volume_sliders_draw_state[target_var] == 0)
    {
        int i;

        shapes_drawn = 3;
        for (i = 0; i < 3; i++) {
            struct ScreenShape *p_shp;
            ubyte drawn;

            p_shp = &audio_volume_sliders[3 * target_var + i];
            drawn = flashy_draw_purple_shape(p_shp);
            if (shapes_drawn > drawn)
                shapes_drawn = drawn;
        }
    }
    else if (audio_volume_sliders_draw_state[target_var] == 1)
    {
        struct ScreenShape *p_shp;

        lbFontPtr = small_med_font;

        p_shp = &audio_volume_sliders[3 * target_var + 1]; // Main bar
        draw_horiz_proslider_main_body(p_shp, target_ptr);
        change |= input_horiz_proslider_main_body(p_shp, target_ptr);

        p_shp = &audio_volume_sliders[3 * target_var + 0]; // Left triangle
        draw_horiz_proslider_left_arrow(p_shp);
        change |= input_horiz_proslider_left_arrow(p_shp, target_ptr);

        p_shp = &audio_volume_sliders[3 * target_var + 2]; // Right triangle
        draw_horiz_proslider_right_arrow(p_shp);
        change |= input_horiz_proslider_right_arrow(p_shp, target_ptr);

        p_shp = &audio_volume_sliders[3 * target_var + 1]; // Main bar
        draw_horiz_proslider_main_body_text(p_shp, p_box, target_ptr);

        shapes_drawn = 3;
    }
    else
    {
        LOGERR("Unexpected audio_volume_sliders_draw_state=%d", (int)audio_volume_sliders_draw_state[target_var]);
        shapes_drawn = 3;
    }
    audio_volume_sliders_draw_state[target_var] = (shapes_drawn == 3) ? 1 : 0;

    lbDisplay.DrawFlags = 0;
    if (!change)
        return 0;

    if (target_var == 0)
        sfx_apply_samplevol();
    else if (target_var == 1)
        sfx_apply_midivol();
    else if (target_var == 2)
        sfx_apply_cdvolume();

    return 0;
}

ubyte show_audio_tracks_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_audio_tracks_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    int i;
    ubyte drawn1 = true;
    ubyte drawn2 = true;

    if ((p_box->Flags & 0x0080) != 0)
    {
        p_box->Flags &= ~0x0080;
        for (i = 0; i < 3; i++) {
            textpos[i] = -5;
        }
    }
    if (p_box->Timer == 255)
    {
        for (i = 0; i < 3; i++) {
            textpos[i] = strlen(gui_strings[528 + i]);
        }
    }

    lbFontPtr = med_font;
    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);

    if (drawn1)
        drawn1 = flashy_draw_text(20, 4 + 0 * 18, game_option_desc(GOpt_CDATrack), 1, 0, &textpos[0], 0);
    if (drawn2)
        drawn2 = flashy_draw_text(20, 4 + 1 * 18, game_option_desc(GOpt_DangerTrack), 1, 0, &textpos[1], 0);
#ifdef HAS_MULTIMEDIA_EXTENSIONS
    if (drawn2)
        drawn2 = flashy_draw_text(20, 4 + 2 * 18, game_option_desc(GOpt_UseMultiMedia), 1, 0, &textpos[2], 0);
#endif
    if (drawn1)
    {
        //drawn1 = options_audio_buttons[0].DrawFn(&options_audio_buttons[0]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn1) : "a" (&options_audio_buttons[0]), "g" (options_audio_buttons[0].DrawFn));
        //drawn1 = options_audio_buttons[1].DrawFn(&options_audio_buttons[1]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn1) : "a" (&options_audio_buttons[1]), "g" (options_audio_buttons[1].DrawFn));
        //drawn1 = options_audio_buttons[2].DrawFn(&options_audio_buttons[2]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn1) : "a" (&options_audio_buttons[2]), "g" (options_audio_buttons[2].DrawFn));
    }
    if (drawn2)
    {
        //drawn2 = options_audio_buttons[3].DrawFn(&options_audio_buttons[3]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn2) : "a" (&options_audio_buttons[3]), "g" (options_audio_buttons[3].DrawFn));
        //drawn2 = options_audio_buttons[4].DrawFn(&options_audio_buttons[4]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn2) : "a" (&options_audio_buttons[4]), "g" (options_audio_buttons[4].DrawFn));
#ifdef HAS_MULTIMEDIA_EXTENSIONS
        //drawn2 = options_audio_buttons[5].DrawFn(&options_audio_buttons[5]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn2) : "a" (&options_audio_buttons[5]), "g" (options_audio_buttons[5].DrawFn));
        //drawn2 = options_audio_buttons[6].DrawFn(&options_audio_buttons[6]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn2) : "a" (&options_audio_buttons[6]), "g" (options_audio_buttons[6].DrawFn));
#endif
    }
    return drawn1 && drawn2;
}

ubyte show_options_audio_screen(void)
{
    ubyte drawn;
    int i;

    for (i = 0; i < 3; i++)
    {
        assert(audio_volume_boxes[i].DrawFn != NULL);
        //drawn = audio_volume_boxes[i].DrawFn(&audio_volume_boxes[i]); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&audio_volume_boxes[i]), "g" (audio_volume_boxes[i].DrawFn));
    }
    {
        assert(audio_tracks_box.DrawFn != NULL);
        //drawn = audio_tracks_box.DrawFn(&audio_tracks_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&audio_tracks_box), "g" (audio_tracks_box.DrawFn));
    }
    return drawn;
}

ubyte show_options_visual_screen(void)
{
    ubyte drawn;

    //drawn = options_gfx_box.DrawFn(&options_gfx_box); -- incompatible calling convention
    asm volatile ("call *%2\n"
        : "=r" (drawn) : "a" (&options_gfx_box), "g" (options_gfx_box.DrawFn));
    if (drawn == 3) {
        show_options_visual_main_box(&options_gfx_box);
    }
    return drawn;
}

short horiz_proslider_prepare_main_body_pts(short *pts_x, short *pts_y, short width, short height)
{
    short wtext1, wtext2, htext;

    lbFontPtr = small_med_font;
    // cut-ins for text at start and at end of slider
    wtext2 = my_string_width(gui_strings[422]) + 2;
    wtext1 = my_string_width(gui_strings[421]) + 2;
    htext = my_char_height('A') + 4;

    pts_x[0] = 0;
    pts_y[0] = height - 1;

    pts_x[1] = pts_x[0] + (height - 1 - htext);
    pts_y[1] = htext;

    pts_x[2] = pts_x[1] + wtext1;
    pts_y[2] = htext;

    pts_x[3] = pts_x[2] + htext;
    pts_y[3] = 0;

    pts_x[4] = width + 2 * 6;
    pts_y[4] = 0;

    pts_x[5] = pts_x[4] - (height - 1 - htext);
    pts_y[5] = height - 1 - htext;

    pts_x[6] = pts_x[5] - wtext2;
    pts_y[6] = height - 1 - htext;

    pts_x[7] = pts_x[6] - htext;
    pts_y[7] = height - 1;

    pts_x[8] = pts_x[0];
    pts_y[8] = pts_y[0];

    return 9;
}

short horiz_proslider_prepare_left_arrow_pts(short *pts_x, short *pts_y, short width, short height)
{
    pts_x[0] = 0;
    pts_y[0] = height - 1;

    pts_x[1] = 0;
    pts_y[1] = (height + 1) / 2;

    pts_x[2] = width - 2;
    pts_y[2] = 0;

    pts_x[3] = width - 1;
    pts_y[3] = 0;

    pts_x[4] = 1; // not a mistake, one pixel shift is required for symmetry with right button
    pts_y[4] = height - 1;

    return 5;
}

short horiz_proslider_prepare_right_arrow_pts(short *pts_x, short *pts_y, short width, short height)
{
    pts_x[0] = 0;
    pts_y[0] = height - 1;

    pts_x[1] = width - 2;
    pts_y[1] = 0;

    pts_x[2] = width - 1;
    pts_y[2] = 0;

    pts_x[3] = width - 1;
    pts_y[3] = height / 2;

    pts_x[4] = 0;
    pts_y[4] = height - 1;

    return 5;
}

ubyte change_panel_permutation(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_change_panel_permutation\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (click)
    {
        game_option_dec(GOpt_PanelPermutation);
    }
    else
    {
        game_option_inc(GOpt_PanelPermutation);
    }
    update_options_gfx_state();
    return 1;
}

ubyte change_trenchcoat_preference(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_change_trenchcoat_preference\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (click)
    {
        game_option_dec(GOpt_TrenchcoatPreference);
    }
    else
    {
        game_option_inc(GOpt_TrenchcoatPreference);
    }
    update_options_gfx_state();
    return 1;
}

ubyte flashy_draw_purple_label(struct ScreenButton *p_btn)
{
    my_set_text_window(p_btn->X + 2, p_btn->Y + 2, p_btn->Width - 18, p_btn->Height - 4);
    lbFontPtr = p_btn->Font;

    if ((p_btn->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_btn->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
        p_btn->Timer = -1;
        if (p_btn->Text != NULL)
            p_btn->TextFadePos = strlen(p_btn->Text);
        else
            p_btn->TextFadePos = -5;
    }
    if ((p_btn->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_btn->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0001);
        p_btn->Timer = 0;
        p_btn->Flags |= GBxFlg_Unkn0080;
    }

    ubyte ret;

    if ((p_btn->Flags & GBxFlg_TextRight) != 0)
    {
          lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_RIGHT;
    }
    else if ((p_btn->Flags & GBxFlg_TextCenter) != 0)
    {
          lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    }
    if (p_btn->DrawTextFn != NULL) {
        ubyte drawn;
        //p_btn->DrawTextFn(p_btn); -- incompatible calling convention
        asm volatile ("call *%2\n"
          : "=r" (drawn) : "a" (p_btn), "g" (p_btn->DrawTextFn));
    }
    ret = (p_btn->Timer > 1) ? 3 : 0;
    if (p_btn->Timer < 24) {
        p_btn->Timer += p_btn->DrawSpeed;
    }
    lbDisplay.DrawFlags = 0;
    return ret;
}

void init_screen_label(struct ScreenButton *p_box, ScrCoord x, ScrCoord y,
  const char *text, int drawspeed, struct TbSprite *p_font, int textspeed, int flags)
{
    init_screen_button(p_box, x, y, text, drawspeed, p_font, textspeed, flags);
    p_box->Border = 0;
    p_box->AccelKey = 0;
    p_box->DrawFn = ac_flashy_draw_purple_label;
    p_box->DrawTextFn = ac_label_text;
}

void init_options_audio_screen_boxes(void)
{
    int i, h;
    int val;
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

    h = 72;
    for (i = 0; i < 3; i++)
    {
        init_screen_box(&audio_volume_boxes[i], 213, h, 420, 62, 6);
        h += 71;
    }
    init_screen_box(&audio_tracks_box, 213, h, 420, 62, 6);

    init_screen_button(&options_audio_buttons[0], 393, 289,
      gui_strings[531], 6, med2_font, 1, 0);
    init_screen_button(&options_audio_buttons[1], 458, 289,
      gui_strings[532], 6, med2_font, 1, 0);
    init_screen_button(&options_audio_buttons[2], 523, 289,
      gui_strings[533], 6, med2_font, 1, 0);
    init_screen_button(&options_audio_buttons[3], 458, 307,
      gui_strings[531], 6, med2_font, 1, 0);
    init_screen_button(&options_audio_buttons[4], 523, 307,
      gui_strings[532], 6, med2_font, 1, 0);
    init_screen_button(&options_audio_buttons[5], 458, 325,
      gui_strings[478], 6, med2_font, 1, 0);
    init_screen_button(&options_audio_buttons[6], 523, 325,
      gui_strings[479], 6, med2_font, 1, 0);

    val = 2;
    for (i = 0; i < 3; i++)
    {
        options_audio_buttons[0+i].Radio = &ingame.CDTrack;
        options_audio_buttons[0+i].RadioValue = val++;
        options_audio_buttons[0+i].Flags |= GBxFlg_RadioBtn;
    }

    val = 1;
    for (i = 0; i < 2; i++)
    {
        options_audio_buttons[3+i].Radio = &ingame.DangerTrack;
        options_audio_buttons[3+i].RadioValue = val++;
        options_audio_buttons[3+i].Flags |= GBxFlg_RadioBtn;
    }

    val = 0;
    for (i = 0; i < 2; i++)
    {
        options_audio_buttons[5+i].Radio = &ingame.UseMultiMedia;
        options_audio_buttons[5+i].RadioValue = val++;
        options_audio_buttons[5+i].Flags |= GBxFlg_RadioBtn;
    }

    audio_volume_boxes[0].SpecialDrawFn = show_audio_volume_box;
    audio_volume_boxes[1].SpecialDrawFn = show_audio_volume_box;
    audio_volume_boxes[2].SpecialDrawFn = show_audio_volume_box;
    audio_tracks_box.SpecialDrawFn = show_audio_tracks_box;

    // Reposition the components to current resolution

    start_x = unkn13_SYSTEM_button.X + unkn13_SYSTEM_button.Width;
    // On the X axis, we're going for centering on the screen. So subtract the previous
    // button position two times - once for the left, and once to make the same space on
    // the right.
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - audio_volume_boxes[0].Width;

    start_y = system_screen_shared_header_box.Y + system_screen_shared_header_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y + border;
    for (i = 0; i < 3; i++)
    {
        space_h -= audio_volume_boxes[i].Height;
    }
    space_h -= audio_tracks_box.Height;
    // Some unused space below
    space_h -= audio_tracks_box.Height * 5 / 4;

    h = space_h / 5;
    for (i = 0; i < 3; i++)
    {
        struct ScreenBox *p_box;
        struct ScreenShape *p_shp;
        short shape_pts_x[9];
        short shape_pts_y[9];
        short pts_len;

        p_box = &audio_volume_boxes[i];
        // There is one box only to position, and no space is needed after it - the whole
        // available empty space goes into one place.
        p_box->X = start_x + space_w;
        // There is one box only to position, so space goes into two parts - before and after.
        p_box->Y = start_y + h;

        // Each volume bar consists of 3 shapes: main body and two arrows

        p_shp = &audio_volume_sliders[3*i+1];
        pts_len = horiz_proslider_prepare_main_body_pts(shape_pts_x, shape_pts_y, 336, 28);
        init_screen_shape(p_shp, p_box->X + 33 + 4, p_box->Y + 26,
          shape_pts_x, shape_pts_y, pts_len, 0x0100, 0x0100, 4);
        p_shp->Colour = 0x0AE;

        p_shp = &audio_volume_sliders[3*i+0];
        pts_len = horiz_proslider_prepare_left_arrow_pts(shape_pts_x, shape_pts_y, 16, 15);
        init_screen_shape(p_shp, p_box->X + 33, p_box->Y + 26 + 12,
          shape_pts_x, shape_pts_y, pts_len, 0x0100, 0x0100, 4);
        p_shp->Colour = 0x0AE;

        p_shp = &audio_volume_sliders[3*i+2];
        pts_len = horiz_proslider_prepare_right_arrow_pts(shape_pts_x, shape_pts_y, 16, 15);
        init_screen_shape(p_shp, p_box->X + 33 + 1 + 3 + 336 + 1, p_box->Y + 26 + 1,
          shape_pts_x, shape_pts_y, pts_len, 0x0100, 0x0100, 4);
        p_shp->Colour = 0x0AE;

        h += p_box->Height + space_h / 5;
    }

    audio_tracks_box.X = start_x + space_w;
    audio_tracks_box.Y = start_y + h;

    for (i = 0; i < 3; i++)
    {
        options_audio_buttons[0+i].X = audio_tracks_box.X +
          audio_tracks_box.Width - 45 - 65 * (3-i);
        options_audio_buttons[0+i].Y = audio_tracks_box.Y + 4;
    }

    for (i = 0; i < 2; i++)
    {
        options_audio_buttons[3+i].X = audio_tracks_box.X +
          audio_tracks_box.Width - 45 - 65 * (2-i);
        options_audio_buttons[3+i].Y = audio_tracks_box.Y + 4 + 1 * 18;
    }

    for (i = 0; i < 2; i++)
    {
        options_audio_buttons[5+i].X = audio_tracks_box.X +
          audio_tracks_box.Width - 45 - 65 * (2-i);
        options_audio_buttons[5+i].Y = audio_tracks_box.Y + 4 + 2 * 18;
    }
}

void init_options_gfx_screen_boxes(void)
{
    int i, n;
    int val;
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

    start_y = 72;
    init_screen_box(&options_gfx_box, 213u, start_y, 420u, 354, 6);
    start_y += 22;
    n = 0;

    assert(n == 0);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_ProjectorSpeed), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[0], 213 + 243, start_y,
      gui_strings[465], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[1], 213 + 331, start_y,
      gui_strings[466], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == 1);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_HighResolution), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[2], 213 + 243, start_y,
      gui_strings[473], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[3], 213 + 331, start_y,
      gui_strings[474], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == 2);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_DetailLevel), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[4], 213 + 243, start_y,
      gui_strings[475], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[5], 213 + 331, start_y,
      gui_strings[477], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == 3);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_CameraPerspective), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[6], 213 + 243, start_y,
      gui_strings[478], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[7], 213 + 331, start_y,
      gui_strings[479], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == 4);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_AdvancedLights), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[8], 213 + 243, start_y,
      gui_strings[478], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[9], 213 + 331, start_y,
      gui_strings[479], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == 5);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_BillboardMovies), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[10], 213 + 243, start_y,
      gui_strings[478], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[11], 213 + 331, start_y,
      gui_strings[479], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == 6);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_DeepRadar), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[12], 213 + 243, start_y,
      gui_strings[478], 6, med2_font, 1, 0);
    init_screen_button(&options_gfx_buttons[13], 213 + 331, start_y,
      gui_strings[479], 6, med2_font, 1, 0);
    n++;
    start_y += 18;

    assert(n == GFX_TOGGLE_OPTIONS_COUNT + 0);
    val = abs(ingame.PanelPermutation);
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_PanelPermutation), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[14], 320u, 274u,
        gui_strings[579 + val], 6, med2_font, 1, 0);
    options_gfx_buttons[14].CallBackFn = ac_change_panel_permutation;
    options_gfx_buttons[14].Width += 60;
    n++;
    start_y += 18;

    assert(n == GFX_TOGGLE_OPTIONS_COUNT + 1);
    val = ingame.TrenchcoatPreference;
    init_screen_label(&options_gfx_labels[n],  213 + 20,  start_y,
      game_option_desc(GOpt_TrenchcoatPreference), 6, med_font, 1, 0);
    init_screen_button(&options_gfx_buttons[15], 320u, 310u,
        gui_strings[583 + val], 6, med2_font, 1, 0);
    options_gfx_buttons[15].CallBackFn = ac_change_trenchcoat_preference;
    options_gfx_buttons[15].Width = options_gfx_buttons[14].Width;

    val = 0;
    for (i = 0; i < 2; i++)
    {
        options_gfx_buttons[i].Radio = &game_projector_speed;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        val++;
    }

    val = 0;
    for (i = 2; i < 4; i++)
    {
        options_gfx_buttons[i].Radio = &game_high_resolution;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        val++;
    }

    val = 0;
    for (i = 4; i < 6; i++)
    {
        options_gfx_buttons[i].Radio = &ingame.DetailLevel;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        val++;
    }

    val = 0;
    for (i = 6; i < 8; i++)
    {
        options_gfx_buttons[i].Radio = &game_perspective;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        val += 5;
    }

    val = 0;
    for (i = 8; i < 10; i++)
    {
        options_gfx_buttons[i].Radio = &game_gfx_advanced_lights;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        ingame.Flags |= GamF_AdvLights;
        val++;
    }

    val = 0;
    for (i = 10; i < 12; i++)
    {
        options_gfx_buttons[i].Radio = &game_billboard_movies;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        ingame.Flags |= GamF_BillboardMovies;
        val++;
    }

    val = 0;
    for (i = 12; i < 14; i++)
    {
        options_gfx_buttons[i].Radio = &game_gfx_deep_radar;
        options_gfx_buttons[i].RadioValue = val;
        options_gfx_buttons[i].Flags |= GBxFlg_RadioBtn;
        ingame.Flags &= ~GamF_DeepRadar;
        val++;
    }

    // Reposition the components to current resolution

    start_x = unkn13_SYSTEM_button.X + unkn13_SYSTEM_button.Width;
    // On the X axis, we're going for centering on the screen. So subtract the previous
    // button position two times - once for the left, and once to make the same space on
    // the right.
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - options_gfx_box.Width;

    start_y = system_screen_shared_header_box.Y + system_screen_shared_header_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y - options_gfx_box.Height + border;

    // There is one box only to position, and no space is needed after it - the whole
    // available empty space goes into one place.
    options_gfx_box.X = start_x + space_w;
    // There is one box only to position, so space goes into two parts - before and after.
    options_gfx_box.Y = start_y + space_h / 2;

    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT; i++)
    {
        options_gfx_labels[i].X = options_gfx_box.X + 20;
        options_gfx_labels[i].Y = options_gfx_box.Y + 22 + i * 18;
        options_gfx_buttons[i*2+0].X = options_gfx_box.X +
          options_gfx_box.Width - 89 - 88;
        options_gfx_buttons[i*2+0].Y = options_gfx_labels[i].Y;
        options_gfx_buttons[i*2+1].X = options_gfx_box.X +
          options_gfx_box.Width - 89;
        options_gfx_buttons[i*2+1].Y = options_gfx_labels[i].Y;
    }

    for (n = 0; n < GFX_MULTIVAL_OPTIONS_COUNT; n++)
    {
        int k;

        i = GFX_TOGGLE_OPTIONS_COUNT * 2 + n;
        k = GFX_TOGGLE_OPTIONS_COUNT + n;
        val = (options_gfx_box.Width -
          options_gfx_buttons[i].Width) / 2;
        options_gfx_buttons[i].X = options_gfx_box.X + val + 9;
        options_gfx_buttons[i].Y = options_gfx_buttons[12].Y + (i - 12) * 36;

        val = (options_gfx_box.Width -
          options_gfx_labels[k].Width) / 2;
        options_gfx_labels[k].X = options_gfx_box.X + val + 9;
        options_gfx_labels[k].Y = options_gfx_buttons[i].Y - 18;
    }
}

void reset_options_audio_boxes_flags(void)
{
    int i;

    for (i = 0; i < 3; i++) {
        audio_volume_boxes[i].Flags = GBxFlg_Unkn0001;
    }
    audio_tracks_box.Flags = GBxFlg_Unkn0001;
    for (i = 0; i < 9; i++) {
        audio_volume_sliders[i].Flags = GBxFlg_Unkn0001;
    }
    for (i = 0; i < 7; i++) {
        options_audio_buttons[i].Flags = GBxFlg_RadioBtn | GBxFlg_Unkn0001;
    }
    for (i = 0; i < 3; i++) {
        audio_volume_sliders_draw_state[i] = 0;
    }
}

void reset_options_gfx_boxes_flags(void)
{
    int i;

    options_gfx_box.Flags = GBxFlg_Unkn0001;

    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT+GFX_MULTIVAL_OPTIONS_COUNT; i++) {
        options_gfx_labels[i].Flags = GBxFlg_Unkn0001;
    }
    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT * 2; i++) {
        options_gfx_buttons[i].Flags = GBxFlg_RadioBtn | GBxFlg_Unkn0001;
    }
    for (; i < GFX_TOGGLE_OPTIONS_COUNT * 2 + GFX_MULTIVAL_OPTIONS_COUNT; i++) {
        options_gfx_buttons[i].Flags = GBxFlg_Unkn0001;
    }
}

void skip_flashy_draw_audio_screen_boxes(void)
{
    int i;

    for (i = 0; i < 3; i++)
        audio_volume_boxes[i].Flags |= GBxFlg_Unkn0002;
    audio_tracks_box.Flags |= GBxFlg_Unkn0002;
    for (i = 0; i < 9; i++)
        audio_volume_sliders[i].Flags |= GBxFlg_Unkn0002;
    for (i = 0; i < 7; i++)
        options_audio_buttons[i].Flags |= GBxFlg_Unkn0002;
    for (i = 0; i < 3; i++)
        audio_volume_sliders_draw_state[i] = 1;
}

void skip_flashy_draw_gfx_screen_boxes(void)
{
    int i;

    options_gfx_box.Flags |= GBxFlg_Unkn0002;

    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT+GFX_MULTIVAL_OPTIONS_COUNT; i++) {
        options_gfx_labels[i].Flags |= GBxFlg_Unkn0002;
    }
    for (i = 0; i < GFX_TOGGLE_OPTIONS_COUNT * 2 + GFX_MULTIVAL_OPTIONS_COUNT; i++) {
        options_gfx_buttons[i].Flags |= GBxFlg_Unkn0002;
    }
}

void mark_gfx_screen_boxes_redraw(void)
{
    options_gfx_box.Flags &= ~(GBxFlg_BkgndDrawn|GBxFlg_TextRight|GBxFlg_BkCopied);
}

void update_options_screen_state(void)
{
    update_options_gfx_state();
    mark_gfx_screen_boxes_redraw();
}

/******************************************************************************/
