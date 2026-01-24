/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file display.h
 *     Header file for display.c.
 * @par Purpose:
 *     Display and graphics mode related function.
 * @par Comment:
 *     Extended wrappers for bflibrary functionalities.
 * @author   Tomasz Lis
 * @date     12 Nov 2008 - 25 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stddef.h>
#include "bfscreen.h"
#include "bfanywnd.h"

/******************************************************************************/

struct ScreenBufBkp {
    ubyte *WScreen;
    long PhysicalScreenWidth;
    long PhysicalScreenHeight;
    long GraphicsScreenWidth;
    long GraphicsScreenHeight;
    struct TbAnyWindow GWindow;
};

extern TbScreenMode screen_mode_game_hi;
extern TbScreenMode screen_mode_game_lo;
extern TbScreenMode screen_mode_menu;
extern TbScreenMode screen_mode_fmvid_lo;
extern TbScreenMode screen_mode_fmvid_hi;

extern ubyte game_high_resolution;

extern ubyte *back_buffer;

extern s32 engn_xc;
extern s32 engn_yc;
extern s32 engn_zc;
extern s32 engn_anglexz;

extern s32 engn_x_vel;
extern s32 engn_y_vel;

extern ushort text_window_x1;
extern ushort text_window_y1;
extern ushort text_window_x2;
extern ushort text_window_y2;

/******************************************************************************/

void display_set_full_screen(bool full_screen);
void display_set_lowres_stretch(bool stretch);
TbResult screen_idle_update_initialize(void);
void display_lock(void);
void display_unlock(void);

void setup_screen_mode(TbScreenMode mode);
void setup_simple_screen_mode(TbScreenMode mode);

void screen_save_backup_buffer(struct ScreenBufBkp *bkp);
void screen_switch_to_custom_buffer(struct ScreenBufBkp *bkp,
  TbPixel *buf, short width, short height);
void screen_load_backup_buffer(struct ScreenBufBkp *bkp);

const char * SWResourceMapping(short index);
void swap_wscreen(void);
void show_black_screen(void);
TbResult cover_screen_rect_with_sprite(short x, short y,
  ushort w, ushort h, struct TbSprite *spr);
TbResult cover_screen_rect_with_raw_file(short x, short y,
  ushort w, ushort h, const char *fname);


void my_set_text_window(ushort x1, ushort y1, ushort w, ushort h);

void set_user_selected_brightness(void);
void reset_user_selected_brightness(void);

void set_brightness_fadedout(void);
void ingame_palette_load(int pal_id);
void change_brightness(short val);

/******************************************************************************/
#endif
