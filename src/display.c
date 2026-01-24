/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file display.c
 *     Display and graphics mode related function.
 * @par Purpose:
 *     Wrappers for bflibrary functions and other helpers related to display.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "display.h"

#include "bffile.h"
#include "bfscreen.h"
#include "bfscrsurf.h"
#include "bfpalette.h"
#include "bfsprite.h"
#include "bffont.h"
#include "bftext.h"
#include "bfplanar.h"
#include "bfutility.h"
#include "bfwindows.h"
#include "poly.h"

#include "engincolour.h"
#include "engintxtrmap.h"

#include "game_options.h"
#include "game_sprts.h"
#include "mouse.h"
#include "util.h"
#include "swlog.h"

/******************************************************************************/

TbScreenMode screen_mode_game_hi = Lb_SCREEN_MODE_640_480_8;
TbScreenMode screen_mode_game_lo = Lb_SCREEN_MODE_320_200_8;
TbScreenMode screen_mode_menu = Lb_SCREEN_MODE_640_480_8;
TbScreenMode screen_mode_fmvid_lo = Lb_SCREEN_MODE_320_200_8;
TbScreenMode screen_mode_fmvid_hi = Lb_SCREEN_MODE_640_480_8;

extern ushort data_1aa330;
extern ushort data_1aa332;

#if defined(WIN32)

const char * SWResourceMapping(short index)
{
    switch (index)
    {
    case 1:
        return "A";
        //return MAKEINTRESOURCE(110); -- may work for other resource compilers
    default:
        return NULL;
    }
}

#else

const char * SWResourceMapping(short index)
{
    switch (index)
    {
    case 1:
        return "syndwarsfx_icon.png";
    default:
        return NULL;
    }
}

#endif

void swap_wscreen(void)
{
    TbBool was_locked;
    was_locked = LbScreenIsLocked();
    if ( was_locked )
        LbScreenUnlock();
    LbScreenSwap();
    if ( was_locked )
    {
      while ( LbScreenLock() != Lb_SUCCESS )
        ;
    }
}

TbBool screen_idle_update(void)
{
    //TODO when this gets modified to run it separate thread, it should only run if screen not locked
    swap_wscreen();
    return true;
}

TbResult screen_idle_update_initialize(void)
{
    if (LbRegisterIdleHandler(screen_idle_update) != Lb_SUCCESS)
        return Lb_FAIL;
    return Lb_SUCCESS;
}

void
display_set_full_screen (bool full_screen)
{
    int i;

    if (lbScreenSurface != NULL)
        return;

    for (i = 1; i < LB_MAX_SCREEN_MODES_COUNT; ++i)
    {
        TbScreenModeInfo *mdinfo;

        mdinfo = LbScreenGetModeInfo(i);
        if (mdinfo->Width == 0) break;
        if (full_screen) {
            mdinfo->VideoMode &= ~Lb_VF_WINDOWED;
        } else {
            mdinfo->VideoMode |= Lb_VF_WINDOWED;
        }
    }
}

void display_set_lowres_stretch(bool stretch)
{
    if (stretch)
        LbScreenSetMinScreenSurfaceDimension(400);
    else
        LbScreenSetMinScreenSurfaceDimension(1);
}

void display_lock(void)
{
    if (!LbScreenIsLocked()) {
        while (LbScreenLock() != Lb_SUCCESS)
            ;
    }
}

void display_unlock(void)
{
    LbScreenUnlock();
}

void setup_simple_screen_mode(TbScreenMode mode)
{
    TbScreenModeInfo *mdinfo;

    printf("%s %d\n", __func__, (int)mode);
    mdinfo = LbScreenGetModeInfo(mode);
    if (mdinfo->Width == 0) {
        LOGERR("Simple video mode %d is invalid", (int)mode);
        return;
    }
    LbScreenSetup(mode, mdinfo->Width, mdinfo->Height, display_palette);

    mouse_update_on_screen_mode_change(false);
}

void setup_screen_mode(TbScreenMode mode)
{
    TbBool was_locked;
    TbScreenModeInfo *mdinfo;

    printf("%s %d\n", __func__, (int)mode);
    mdinfo = LbScreenGetModeInfo(mode);
    if (mdinfo->Width == 0) {
        LOGERR("Game video mode %d is invalid", (int)mode);
        mode = 1;
        mdinfo = LbScreenGetModeInfo(mode);
    }
    was_locked = LbScreenIsLocked();
    if (was_locked)
        LbScreenUnlock();
    if (LbScreenSetup(mode, mdinfo->Width, mdinfo->Height, display_palette) != 1)
        exit(1);
    if (was_locked) {
        while (LbScreenLock() != Lb_SUCCESS)
            ;
    }

    mouse_update_on_screen_mode_change(true);

    setup_vecs(lbDisplay.WScreen, vec_tmap[0], lbDisplay.PhysicalScreenWidth,
        lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
}

void screen_save_backup_buffer(struct ScreenBufBkp *bkp)
{
    LbScreenStoreGraphicsWindow(&bkp->GWindow);
    bkp->WScreen = lbDisplay.WScreen;
    bkp->PhysicalScreenWidth = lbDisplay.GraphicsScreenWidth;
    bkp->PhysicalScreenHeight = lbDisplay.PhysicalScreenHeight;
    bkp->GraphicsScreenWidth = lbDisplay.GraphicsScreenWidth;
    bkp->GraphicsScreenHeight = lbDisplay.GraphicsScreenHeight;
}

void screen_switch_to_custom_buffer(struct ScreenBufBkp *bkp,
  TbPixel *buf, short width, short height)
{
    screen_save_backup_buffer(bkp);
    lbDisplay.WScreen = buf;
    lbDisplay.GraphicsScreenWidth = width;
    lbDisplay.PhysicalScreenHeight = height;
    lbDisplay.GraphicsScreenWidth = width;
    lbDisplay.GraphicsScreenHeight = height;
    LbScreenSetGraphicsWindow(0, 0, width, height);
}

void screen_load_backup_buffer(struct ScreenBufBkp *bkp)
{
    lbDisplay.WScreen = bkp->WScreen;
    lbDisplay.GraphicsScreenWidth = bkp->PhysicalScreenWidth;
    lbDisplay.PhysicalScreenHeight = bkp->PhysicalScreenHeight;
    lbDisplay.GraphicsScreenWidth = bkp->GraphicsScreenWidth;
    lbDisplay.GraphicsScreenHeight = bkp->GraphicsScreenHeight;
    LbScreenLoadGraphicsWindow(&bkp->GWindow);
}

void show_black_screen(void)
{
    TbBool was_locked;

    was_locked = LbScreenIsLocked();
    if (!was_locked) {
        while (LbScreenLock() != Lb_SUCCESS)
            ;
    }
    LbScreenClear(0);
    if (!was_locked)
        LbScreenUnlock();
    swap_wscreen();
}

TbResult cover_screen_rect_with_sprite(short x, short y, ushort w, ushort h, struct TbSprite *spr)
{
    short cx, cy;
    TbResult ret;

    ret = Lb_FAIL;
    for (cy = y; cy < y+h; cy += spr->SHeight)
    {
        for (cx = x; cx < x+w; cx += spr->SWidth) {
            ret = LbSpriteDraw(cx, cy, spr);
        }
    }
    return ret;
}

TbResult cover_screen_rect_with_raw_file(short x, short y, ushort w, ushort h, const char *fname)
{
    struct SSurface surf;
    struct TbRect srect;
    ubyte *inp_buf;
    TbResult ret;

    LbSetRect(&srect, 0, 0, w, h);
    LbScreenSurfaceInit(&surf);
    LbScreenSurfaceCreate(&surf, w, h);
    inp_buf = LbScreenSurfaceLock(&surf);
    ret = LbFileLoadAt(fname, inp_buf);
    LbScreenSurfaceUnlock(&surf);
    if (ret == Lb_FAIL) {
        LbScreenSurfaceRelease(&surf);
        return ret;
    }
    LbScreenUnlock();
    ret = LbScreenSurfaceBlit(&surf, x, y, &srect, SSBlt_TO_SCREEN);
    LbScreenSurfaceRelease(&surf);
    LbScreenLock();
    return ret;
}

void my_set_text_window(ushort x1, ushort y1, ushort w, ushort h)
{
#if 0
    asm volatile (
      "call ASM_my_set_text_window\n"
        : : "a" (x1), "d" (y1), "b" (w), "c" (h));
#endif
    text_window_x1 = x1;
    text_window_y1 = y1;
    text_window_x2 = x1 + w - 1;
    text_window_y2 = y1 + h - 1;
}

void ingame_palette_load(int pal_id)
{
    char locstr[DISKPATH_SIZE];

    sprintf(locstr, "qdata/pal%d.dat", pal_id);
    LbFileLoadAt(locstr, display_palette);
}

void ingame_palette_reload(void)
{
    if ((ingame.Flags & GamF_ThermalView) != 0) {
        ingame_palette_load(3);
    } else {
        ingame_palette_load(ingame.PalType);
    }
}

void change_brightness(short amount)
{
#if 0
    asm volatile ("call ASM_change_brightness\n"
        : : "a" (val));
#endif
    ingame_palette_reload();
    change_brightness_from_normal(amount);
}

void set_user_selected_brightness(void)
{
#if 0
    asm volatile ("call ASM_set_user_selected_brightness\n"
        :  :  : "eax" );
    return;
#endif
    momentary_brightness = user_sel_brightness;
    change_brightness(0);
}

void reset_user_selected_brightness(void)
{
#if 0
    TbResult ret;
    asm volatile ("call ASM_reset_user_selected_brightness\n"
        : "=r" (ret) : );
    return ret;
#endif
    ingame_palette_reload();
    user_sel_brightness = 0;
    set_user_selected_brightness();
}

void set_brightness_fadedout(void)
{
    momentary_brightness = 0;
    change_brightness(-64);
}

/******************************************************************************/
