/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feshared.c
 *     Front-end desktop and menu system, items shared between screens.
 * @par Purpose:
 *     Implement functions for parts shared between screens in front-end desktop.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 03 Sep 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "feshared.h"

#include "bfscreen.h"

#include "femain.h"
#include "game_sprts.h"
#include "guiboxes.h"
#include "guitext.h"
#include "purpldrw.h"
#include "swlog.h"
/******************************************************************************/

struct ScreenTextBox system_screen_shared_header_box = {0};

/******************************************************************************/

void init_sys_scr_shared_header_box(void)
{
    short scr_w, start_x, content_box_width;

    scr_w = lbDisplay.GraphicsWindowWidth;

    init_screen_text_box(&system_screen_shared_header_box, 213u, 25u, 420u, 38, 6, big_font, 1);
    system_screen_shared_header_box.DrawTextFn = show_title_box;

    content_box_width = 420;
    start_x = (scr_w - unkn13_SYSTEM_button.Width - 16 - content_box_width - 7) / 2;

    system_screen_shared_header_box.X = start_x + 7 + unkn13_SYSTEM_button.Width + 9;
}

TbBool is_sys_scr_shared_header_flag01(void)
{
    return ((system_screen_shared_header_box.Flags & 0x0001) != 0);
}

void mark_sys_scr_shared_header_box_redraw(void)
{
    system_screen_shared_header_box.Flags &= ~(GBxFlg_BkgndDrawn | GBxFlg_TextRight | GBxFlg_BkCopied);
}

void update_sys_scr_shared_header(ubyte sysscrn_no)
{
    system_screen_shared_header_box.Text = gui_strings[377 + sysscrn_no];
    mark_sys_scr_shared_header_box_redraw();
}

ubyte show_sys_scr_shared_header(void)
{
    ubyte drawn;

    drawn = system_screen_shared_header_box.DrawFn(&system_screen_shared_header_box);
    return drawn;
}

void reset_sys_scr_shared_boxes_flags(void)
{
    system_screen_shared_header_box.Flags = GBxFlg_Unkn0001;
}

void skip_flashy_draw_sys_scr_shared_boxes(void)
{
    system_screen_shared_header_box.Flags |= GBxFlg_Unkn0002;
}

void init_sys_scr_shared_boxes(void)
{
    init_sys_scr_shared_header_box();
}


/******************************************************************************/
