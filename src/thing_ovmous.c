/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_ovmous.c
 *     Support for checking if thing sprite overlaps with (is under) mouse.
 * @par Purpose:
 *     Check rendering bounds of a sprite vs mouse position.
 *     Use as callbacks during rendering, so that sprite position and scale
 *     calculated there can be reused.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     09 Oct 2025 - 22 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_ovmous.h"

#include <assert.h>
#include "bfscreen.h"

#include "frame_sprani.h"
#include "engindrwlstx.h"

#include "engindrwlstm.h"
#include "guiboxes.h"
#include "player.h"
#include "swlog.h"
#include "thing.h"

/******************************************************************************/

extern struct Thing *dword_176CC8;
extern int dword_176CCC;
extern int dword_176CD0;

extern short word_1A5834;
extern short word_1A5836;

/******************************************************************************/

ubyte check_mouse_overlap(ushort sspr)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_check_mouse_overlap\n"
        : "=r" (ret) : "a" (sspr));
    return ret;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    if (box.Width < 16)
    {
        box.X -= ((16 + 1 - box.Width) >> 1);
        box.Width = 16;
    }
    if (box.Height < 20) {
        box.Y -= ((20 + 1 - box.Height) >> 1);
        box.Height = 20;
    }

    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        PlayerInfo *p_locplayer;
        struct Thing *p_thing;
        p_thing = (struct Thing *)p_sspr->SrcItem;
        p_locplayer = &players[local_player_no];
        p_locplayer->Target = p_thing->ThingOffset;
        p_locplayer->TargetType = TrgTp_Unkn7;
        return 1;
    }
    return 0;
}

ubyte check_mouse_overlap_item(ushort sspr)
{
#if 0
    asm volatile (
      "call ASM_check_mouse_overlap_item\n"
        : : "a" (sspr));
    return 0;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;
    PlayerInfo *p_locplayer;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    p_locplayer = &players[local_player_no];
    if (p_locplayer->TargetType == TrgTp_DroppedTng)
    {
        struct Thing *p_thing;
        ushort VX;
        p_thing = (struct Thing *)p_sspr->SrcItem;
        VX = p_thing->VX;
        if ( VX )
        {
            if (VX < 12 || VX > 13)
                return 0;
            box.X -= (box.Width >> 1);
            box.Y -= (box.Height >> 1);
            box.Width *= 2;
            box.Height *= 2;
        }
    }
    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        struct Thing *p_thing;
        p_thing = (struct Thing *)p_sspr->SrcItem;
        p_locplayer->Target = p_thing->ThingOffset;
        p_locplayer->TargetType = TrgTp_DroppedTng;
        return 1;
    }
    return 0;
}

ubyte check_mouse_overlap_corpse(ushort sspr)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_check_mouse_overlap_corpse\n"
        : "=r" (ret) : "a" (sspr));
    return ret;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;
    PlayerInfo *p_locplayer;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    p_locplayer = &players[local_player_no];
    if (box.Width < 16)
    {
        box.X -= ((17 - box.Width) >> 1);
        box.Width = 16;
    }
    if (box.Height < 20) {
        box.Y -= ((21 - box.Height) >> 1);
        box.Height = 20;
    }

    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        struct Thing *p_thing;
        p_thing = (struct Thing *)p_sspr->SrcItem;
        p_locplayer->Target = p_thing->ThingOffset;
        p_locplayer->TargetType = TrgTp_Unkn1;
        return 1;
    }
    return 0;
}

ubyte check_mouse_over_unkn2(ushort sspr, struct Thing *p_thing)
{
#if 0
    ubyte ret;
    asm volatile (
      "call ASM_check_mouse_over_unkn2\n"
        : "=r" (ret) : "a" (sspr), "d" (p_thing));
    return ret;
#endif
    struct ScreenBoxBase box;
    struct SortSprite *p_sspr;
    struct Frame *p_frm;

    p_sspr = &game_sort_sprites[sspr];
    box.X = p_sspr->X + ((overall_scale * word_1A5834) >> 8);
    box.Y = p_sspr->Y + ((overall_scale * word_1A5836) >> 8);

    p_frm = &frame[p_sspr->Frame];
    box.Width = (overall_scale * p_frm->SWidth) >> 9;
    box.Height = (overall_scale * p_frm->SHeight) >> 9;

    if (box.Width < 16)
    {
        box.X -= ((17 - box.Width) >> 1);
        box.Width = 16;
    }
    if (box.Height < 20) {
        box.Y -= ((21 - box.Height) >> 1);
        box.Height = 20;
    }

    if (in_box(lbDisplay.MMouseX, lbDisplay.MMouseY, box.X, box.Y, box.Width, box.Height))
    {
        dword_176CC8 = p_thing;
        dword_176CD0 = box.Y - 8;
        dword_176CCC = box.X + (box.Height >> 1);
        return 1;
    }
    return 0;
}

/******************************************************************************/
