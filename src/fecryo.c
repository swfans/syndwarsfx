/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fecryo.c
 *     Front-end desktop and menu system, cryogenic chamber screen.
 * @par Purpose:
 *     Implement functions for cryogenic chamber screen in front-end desktop.
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
#include "fecryo.h"

#include <assert.h>
#include "bfkeybd.h"
#include "bfmemut.h"
#include "bfscrcopy.h"
#include "bfsprite.h"
#include "bftext.h"
#include "bfutility.h"
#include "bfjoyst.h"
#include "ssampply.h"

#include "app_gentab.h"
#include "app_sprite.h"
#include "specblit.h"
#include "campaign.h"
#include "cybmod.h"
#include "display.h"
#include "feequip.h"
#include "femain.h"
#include "fenet.h"
#include "game_options.h"
#include "guiboxes.h"
#include "guitext.h"
#include "keyboard.h"
#include "game_data.h"
#include "game_sprts.h"
#include "game.h"
#include "mydraw.h"
#include "packetfe.h"
#include "player.h"
#include "purpldrw.h"
#include "purpldrwlst.h"
#include "research.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
struct ScreenBox cryo_blokey_box = {0};
struct ScreenTextBox cryo_agent_list_box = {0};
struct ScreenTextBox cryo_cybmod_list_box = {0};
struct ScreenButton cryo_offer_cancel_button = {0};

extern char cybmod_name_text[];

extern ubyte current_frame;
extern short word_15511E; // = -1;
extern ubyte byte_155174; // = 166;
extern ubyte byte_155175[];
extern ubyte byte_155180; // = 109;
extern ubyte byte_155181[];
extern ubyte byte_1551F4[5];
extern ubyte cheat_research_cybmods;
extern ubyte byte_1C4978;
extern ubyte byte_1C4979;

// Shared with equip screen
extern char equip_cost_text[20];
extern struct ScreenTextBox equip_name_box;
extern struct ScreenTextBox equip_list_box;
extern struct ScreenInfoBox equip_cost_box;
extern struct ScreenButton equip_offer_buy_button;
struct ScreenButton equip_all_agents_button = {0};
struct ScreenShape equip_agent_select_shapes[5] = {0};

ubyte ac_do_cryo_offer_cancel(ubyte click);
ubyte ac_show_cryo_agent_list(struct ScreenTextBox *box);
ubyte ac_show_cryo_cybmod_list_box(struct ScreenTextBox *box);
ubyte ac_do_cryo_all_agents_set(ubyte click);
void ac_weapon_flic_data_to_screen(void);
ubyte ac_do_equip_offer_buy(ubyte click);

struct ScreenRect equip_blokey_rect[] = {
    {23,  0,  93, 197},
    {46,  0,  47,  50},
    { 0, 49, 139, 148},
    {23, 98,  93, 197},
    { 0,  0, 139, 197},
    { 0,  0, 139, 295},
};

/******************************************************************************/

/** Determines if buy or sell should be available in the cryo mod offer.
 *
 * Rather simple as cyborg mods cannot be sold.
 *
 * @return Gives 0 if button unavailable, 1 for buy, 2 for sell (not possible).
 */
ubyte cryo_offer_can_buy_or_sell(ushort mtype)
{
    if (selected_agent < 0)
        return 0;

    return 1;
}

void update_cybmod_cost_text(void)
{
    struct ModDef *mdef;
    int cost;

    if (selected_mod == -1) // No mod selected
    {
        equip_cost_text[0] = '\0';
        return;
    }

    mdef = &mod_defs[selected_mod + 1];
    cost = 10 * (int)mdef->Cost;
    sprintf(equip_cost_text, "%d", cost);
}

void update_cybmod_name_text(void)
{
    ushort mtype;
    ushort mdstr_id, lvstr_id;
    ubyte modgrp, modlv;

    if (selected_mod == -1) // No mod selected
    {
        cybmod_name_text[0] = '\0';
        return;
    }

    mtype = selected_mod + 1;

    modgrp = cybmod_group_type(mtype);
    modlv = cybmod_version(mtype);
    mdstr_id = 70 + byte_1551F4[modgrp];
    if (modgrp != MODGRP_EPIDERM)
       lvstr_id = 76;
    else
       lvstr_id = 75;
    sprintf(cybmod_name_text, "%s %s %d", gui_strings[mdstr_id], gui_strings[lvstr_id], modlv);
}

/** Get global text pointer to a mod level string.
 * @see loctext_to_gtext()
 */
static const char *cryo_gtext_cybmod_list_item_level(ushort mtype)
{
    char locstr[48];
    ubyte modlv;
    ushort lvstr_id;

    modlv = cybmod_version(mtype);

    if (cybmod_group_type(mtype) != MODGRP_EPIDERM)
        lvstr_id = 76;
    else
        lvstr_id = 75;
    sprintf(locstr, "%s %d", gui_strings[lvstr_id], modlv);
    return loctext_to_gtext(locstr);
}

TbBool cybmod_has_display_anim(ubyte mod)
{
    return (1 << (mod - 1) < 0x1000);
}

void cryo_display_box_redraw(struct ScreenTextBox *p_box)
{
    ubyte real_dbcontent;

    real_dbcontent = cybmod_has_display_anim(selected_mod + 1) ? display_box_content : DiBoxCt_TEXT;
    switch (real_dbcontent)
    {
    case DiBoxCt_TEXT:
        p_box->Flags |= GBxFlg_Unkn0080;
        // Re-add scroll bars
        p_box->Flags |= GBxFlg_RadioBtn;

        p_box->TextTopLine = 0;
        p_box->Lines = 0;
        p_box->Text = &weapon_text[cybmod_text_index[selected_mod]];
        lbFontPtr = small_font;
        p_box->LineHeight = byte_197160 + my_char_height('A');
        lbFontPtr = p_box->Font;
        p_box->TextFadePos = -5;
        break;
    case DiBoxCt_ANIM:
        // Remove scroll bars
        p_box->Flags &= ~GBxFlg_RadioBtn;

        init_weapon_anim(selected_mod + 32);
        // Negative value saves the background before starting animation
        p_box->TextFadePos = -2;
        break;
    }
}

void cryo_update_for_selected_cybmod(void)
{
    update_cybmod_name_text();
    update_cybmod_cost_text();

    if (selected_mod == -1) // No mod selected
    {
        cryo_cybmod_list_box.Flags |= GBxFlg_Unkn0080;
        // Re-add scroll bars
        cryo_cybmod_list_box.Flags |= GBxFlg_RadioBtn;
        cryo_cybmod_list_box.Text = NULL;
        return;
    }

    equip_name_box.TextFadePos = -5;
    switch_equip_offer_to_buy();
    cryo_display_box_redraw(&cryo_cybmod_list_box);
}

ubyte do_cryo_offer_cancel(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_cryo_offer_cancel\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    selected_mod = -1;
    cryo_update_for_selected_cybmod();
    refresh_equip_list = 1;
    return 0;
}

void clear_mod_draw_states(void)
{
    ubyte part;
    for (part = 0; part < 4; part++)
    {
        mod_draw_states[part] = 0;
    }
}

void reset_mod_draw_states_flag08(void)
{
    ubyte part;
    for (part = 0; part < 4; part++)
    {
        mod_draw_states[part] = 0;
        if (flic_mods[part] != 0)
            mod_draw_states[part] |= ModDSt_Unkn08;
    }
}

void set_mod_draw_states_flag08(void)
{
    ubyte part;
    for (part = 0; part < 4; part++)
    {
        if (old_flic_mods[part] != flic_mods[part])
            mod_draw_states[part] |= ModDSt_Unkn08;
    }
}

TbBool mod_draw_update_on_change(ushort mtype)
{
    switch (cybmod_group_type(mtype))
    {
    case MODGRP_LEGS:
        mod_draw_states[3] |= ModDSt_Unkn08;
        update_flic_mods(flic_mods);
        break;
    case MODGRP_ARMS:
        mod_draw_states[2] |= ModDSt_Unkn08;
        update_flic_mods(flic_mods);
        break;
    case MODGRP_CHEST:
        mod_draw_states[0] |= ModDSt_Unkn08;
        update_flic_mods(flic_mods);
        break;
    case MODGRP_BRAIN:
        mod_draw_states[1] |= ModDSt_Unkn08;
        update_flic_mods(flic_mods);
        break;
    default:
        return false;
    }
    return true;
}

ubyte do_equip_offer_buy_cybmod(ubyte click)
{
    struct ModDef *mdef;
    ubyte nbought;

    mdef = &mod_defs[selected_mod + 1];
    nbought = 0;

    if (selected_agent != 4)
    {
        long cost;
        short nagent;
        TbBool added;

        cost = 10 * mdef->Cost;
        nagent = selected_agent;

        if (ingame.Credits - cost < 0)
            added = false;
        else
            added = player_cryo_add_cybmod(nagent, selected_mod + 1);

        if (added) {
            mod_draw_update_on_change(selected_mod + 1);
        }

        if (added) {
            ingame.Expenditure += cost;
            ingame.Credits -= cost;
            nbought++;
        }
    }
    else
    {
        long cost;
        short nagent;
        TbBool added;

        cost = 10 * mdef->Cost;

        for (nagent = 0; nagent < 4; nagent++)
        {
            if (ingame.Credits - cost < 0)
                break;

            added = player_cryo_add_cybmod(nagent, selected_mod + 1);

            if (added) {
                mod_draw_update_on_change(selected_mod + 1);
            }

            if (added) {
                ingame.Credits -= cost;
                ingame.Expenditure += cost;
                nbought++;
            }
        }
    }

    if (nbought > 0)
    {
        if ((login_control__State == LognCt_Unkn5) && ((net_game_play_flags & NGPF_Unkn08) != 0)) {
            net_schedule_player_cryo_equip_sync();
        }
        selected_mod = -1;
        cryo_update_for_selected_cybmod();
        refresh_equip_list = 1;
    }

    return (nbought > 0);
}

void sprint_cryo_cyborg_mods_static_fname(char *str, ubyte part, ubyte *p_mods_arr)
{
    PathInfo *pinfo;

    pinfo = &game_dirs[DirPlace_QEquip];

    switch (part)
    {
    case ModDPt_CHEST:
        sprintf(str, "%s/m%db.raw", pinfo->directory, p_mods_arr[0]);
        break;
    case ModDPt_BRAIN:
        sprintf(str, "%s/m%dbb.raw", pinfo->directory, p_mods_arr[0]);
        break;
    case ModDPt_ARMS:
        sprintf(str, "%s/m%da%d.raw", pinfo->directory, p_mods_arr[0], p_mods_arr[2]);
        break;
    case ModDPt_LEGS:
        sprintf(str, "%s/m%dl%d.raw", pinfo->directory, p_mods_arr[0], p_mods_arr[3]);
        break;
    default:
        str[0] = '\0';
        break;
    }
}

void cryo_cyborg_mods_anim_set_fname(ubyte anislot, ubyte part, ubyte stage)
{
    struct Animation *p_anim;
    PathInfo *pinfo;
    int k;

    k = anim_slots[anislot];
    p_anim = &animations[k];

    pinfo = &game_dirs[DirPlace_QEquip];

    switch (stage)
    {
    case ModDSt_BRT:
        anim_flic_set_fname(p_anim, "%s/m%da%d.fli", pinfo->directory, flic_mods[0], flic_mods[2]);
        break;
    case ModDSt_OUT:
        switch (part)
        {
        case ModDPt_CHEST:
            anim_flic_set_fname(p_anim, "%s/m%dbo.fli", pinfo->directory, old_flic_mods[0]);
            break;
        case ModDPt_BRAIN:
            anim_flic_set_fname(p_anim, "%s/m%dbbo.fli", pinfo->directory, old_flic_mods[0]);
            break;
        case ModDPt_ARMS:
            anim_flic_set_fname(p_anim, "%s/m%da%do.fli", pinfo->directory, old_flic_mods[0], old_flic_mods[2]);
            break;
        case ModDPt_LEGS:
            anim_flic_set_fname(p_anim, "%s/m%dl%do.fli", pinfo->directory, old_flic_mods[0], old_flic_mods[3]);
            break;
        default:
            assert(!"unreachable");
            break;
        }
        break;
    case ModDSt_IN:
        switch (part)
        {
          case ModDPt_CHEST:
            anim_flic_set_fname(p_anim, "%s/m%dbi.fli", pinfo->directory, flic_mods[0]);
            break;
          case ModDPt_BRAIN:
            anim_flic_set_fname(p_anim, "%s/m%dbbi.fli", pinfo->directory, flic_mods[0]);
            break;
          case ModDPt_ARMS:
            anim_flic_set_fname(p_anim, "%s/m%da%di.fli", pinfo->directory, flic_mods[0], flic_mods[2]);
            break;
          case ModDPt_LEGS:
            anim_flic_set_fname(p_anim, "%s/m%dl%di.fli", pinfo->directory, flic_mods[0], flic_mods[3]);
            break;
          default:
            assert(!"unreachable");
            break;
        }
        break;
    case 3:
        // No animation
        break;
    }
}

/** Returns scanline width for given RAW image width.
 *
 * RAW images are padded, meaning amount of bytes between lines
 * is not neccessarily equal to image width.
 */
short raw_file_scanline(short w)
{
    return (w + 3) & ~3;
}

uint cryo_cyborg_part_buf_max_size(void)
{
    uint len;
    short h, scanln;
    ubyte part;

    len = 0;
    for (part = 0; part < 4; part++)
    {
        h = equip_blokey_rect[part].Height;
        scanln = raw_file_scanline(equip_blokey_rect[part].Width);
        len += scanln * h;
    }

    h = equip_blokey_rect[ModDPt_BKGND].Height;
    scanln = raw_file_scanline(equip_blokey_rect[ModDPt_BKGND].Width);

    return max(len, (uint)(scanln * h));
}

/** Get the buffer containing colour keyed images of cyborg mods,
 *  or their background.
 *
 * For each mod, either FLIC file frame or static image is stored
 * in this buffer.
 * During initial part of cyborg view rectangle drawing, the same
 * buffer is also used for storing cyborg shape background.
 * A third mode of using the buffer comes in during breathing
 * animation. At that time, the buffer stores two parts:
 * chest+brain+arms, and separately legs.
 */
inline static ubyte *cryo_cyborg_part_buf_ptr(ubyte target_part)
{
    TbPixel *p_buf;
    ubyte part;
    p_buf = back_buffer - cryo_cyborg_part_buf_max_size();

    // For breathing animation, reuse chest+brain+arms area
    if (target_part >= ModDPt_BREATH)
        return p_buf;

    for (part = 0; part < target_part; part++)
    {
        short h, scanln;

        h = equip_blokey_rect[part].Height;
        scanln = raw_file_scanline(equip_blokey_rect[part].Width);
        p_buf += scanln * h;
    }

    return p_buf;
}

/** Clear part buffer content for given part.
 */
void cryo_cyborg_part_buf_clear(ubyte part)
{
    TbPixel *p_partbuf;
    short h, scanln;

    p_partbuf = cryo_cyborg_part_buf_ptr(part);

    scanln = raw_file_scanline(equip_blokey_rect[part].Width);
    h = equip_blokey_rect[part].Height;

    LbMemorySet(p_partbuf, 0, scanln * h);
}

/** Load cyborg shape background into part buffer.
 *
 * The background occupies the buffers for all parts,
 * so it cannot be used at the same time as individual
 * part buffers.
 */
void cryo_cyborg_part_buf_blokey_bkgnd_load(void)
{
    TbPixel *p_partbuf;
    long len;
    short h, scanln;
    ubyte part;

    part = ModDPt_BKGND;
    p_partbuf = cryo_cyborg_part_buf_ptr(part);

    scanln = raw_file_scanline(equip_blokey_rect[part].Width);
    h = equip_blokey_rect[part].Height;

    {
        char locstr[DISKPATH_SIZE];
        PathInfo *pinfo;

        pinfo = &game_dirs[DirPlace_QEquip];
        sprintf(locstr, "%s/bgman.raw", pinfo->directory);
        len = LbFileLoadAt(locstr, p_partbuf);
    }
    if (len < 4) {
        LbMemorySet(p_partbuf, 0, scanln * h);
    }
}

void cryo_cyborg_part_buf_blokey_static_load(ubyte *p_mods_arr, ubyte part)
{
    TbPixel *p_scratch;
    TbPixel *p_partbuf;
    long len;
    short h, scanln;

    p_scratch = anim_type_get_output_buffer(AniSl_SCRATCH);
    p_partbuf = cryo_cyborg_part_buf_ptr(part);

    scanln = raw_file_scanline(equip_blokey_rect[part].Width);
    h = equip_blokey_rect[part].Height;

    // Read the raw image; we expect its lines to already be padded
    {
        char locstr[DISKPATH_SIZE];
        sprint_cryo_cyborg_mods_static_fname(locstr, part, p_mods_arr);
        len = LbFileLoadAt(locstr, p_scratch);
    }
    if (len < 4) {
        LbMemorySet(p_scratch, 0, scanln * h);
    }

    // Blit the current part image onto part buffer
    ApScreenCopyRect(p_scratch, p_partbuf, scanln, scanln, h);
}

/** Copy image data from given animation slot buffer to part buffer.
 */
void cryo_cyborg_part_buf_blokey_fli_frame_copy(ubyte part, ubyte anislot)
{
    TbPixel *p_flicbuf;
    TbPixel *p_partbuf;
    short partbuf_scanln;
    short w, h;

    w = equip_blokey_rect[part].Width;
    h = equip_blokey_rect[part].Height;

    // Blit the current part image onto framebuf
    p_flicbuf = anim_type_get_output_buffer(anislot);
    p_partbuf = cryo_cyborg_part_buf_ptr(part);
    partbuf_scanln = raw_file_scanline(w);

    // Fill buffer padding with colour key (transparent colour)
    if (partbuf_scanln != w)
        ApScreenSetRect(p_partbuf + w, 0,  partbuf_scanln - w, partbuf_scanln, h);
    // Blit the current animation frame buffer onto part buffer
    ApScreenCopyRect(p_flicbuf, p_partbuf, w, partbuf_scanln, h);
}

void cryo_cyborg_part_buf_blokey_static_clear_all(void)
{
    ubyte part;

    for (part = 0; part < 4; part++)
    {
        cryo_cyborg_part_buf_clear(part);
    }
}

void cryo_cyborg_part_buf_blokey_static_load_all(ubyte *p_mods_arr)
{
    ubyte part;

    for (part = 0; part < 4; part++)
    {
        if (p_mods_arr[part] == 0)
            continue;

        cryo_cyborg_part_buf_blokey_static_load(p_mods_arr, part);
    }
}

/** Clears output buffer of the animation at given slot.
 *
 * The animation must be opened, but its frame buffer
 * doesn't have to be set for this function to work.
 */
void flic_clear_output_buffer(ubyte anislot)
{
    struct Animation *p_anim;
    int k;

    k = anim_slots[anislot];
    p_anim = &animations[k];
    if (anim_is_opened(p_anim))
    {
        ubyte *obuf;
        short h;

        obuf = anim_type_get_output_buffer(p_anim->Type);

        for (h = p_anim->FLCFileHeader.Height; h > 0; h--)
        {
            LbMemorySet(obuf, '\0', p_anim->FLCFileHeader.Width);
            obuf += p_anim->FLCFileHeader.Width;
        }
    }
}

ubyte cryo_cyborg_mods_anim_get_stage(ubyte *p_part)
{
    ubyte part, stage;

    stage = ModDSt_BRT;

    if (stage == ModDSt_BRT)
    {
        for (part = 1; part < 4; part = (part+1) % 4)
        {
            if (((mod_draw_states[part] & ModDSt_Unkn08) != 0) &&
              ((mod_draw_states[part] & ModDSt_Unkn04) != 0)) {
                stage = ModDSt_OUT;
                break;
            }
            if (part == 0)
                break;
        }
    }

    if (stage == ModDSt_BRT)
    {
        for (part = 0; part < 4; part++)
        {
            if (((mod_draw_states[part] & ModDSt_Unkn08) != 0) &&
              ((mod_draw_states[part] & ModDSt_Unkn04) == 0) &&
              (flic_mods[part] != 0)) {
                stage = ModDSt_IN;
                break;
            }
        }
    }

    if (stage == ModDSt_BRT)
    {
        if (current_drawing_mod == ModDPt_BREATH) {
            part = current_drawing_mod;
            stage = 3;
        } else
        for (part = 0; part < 4; part++)
        {
            if (flic_mods[part] == 0) {
                stage = ModDSt_END;
                break;
            }
        }
    }

    if (stage == ModDSt_BRT)
    {
        part = ModDPt_BREATH;
    }

    *p_part = part;
    return stage;
}

/** Prepares for playback of the next FLIC animation.
 *
 * Note that this function is called while the drawitem to draw
 * the last frame is still in drawlist. That means animation
 * frame buffer can be modified by this function, but the parts
 * buffers need to be left unchanged.
 */
void init_next_blokey_flic(void)
{
    ubyte part, stage;
    ubyte anislot;

    stage = cryo_cyborg_mods_anim_get_stage(&part);

    switch (stage)
    {
    case ModDSt_BRT:
        anislot = AniSl_CYBORG_BRTH;
        if (!byte_1DDC40)
        {
            byte_1DDC40 = 1;
            play_sample_using_heap(0, 134, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        }
        else if (!IsSamplePlaying(0, 134, 0))
        {
            cryo_cyborg_mods_anim_set_fname(anislot, part, stage);
            flic_unkn03(anislot);
            flic_clear_output_buffer(anislot);
            play_sample_using_heap(0, 126, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
            current_frame = 0;
            new_current_drawing_mod = ModDPt_BREATH;
            byte_1DDC40 = 0;
        }
        break;
    case ModDSt_OUT:
        anislot = AniSl_CYBORG_INOUT;
        cryo_cyborg_mods_anim_set_fname(anislot, part, stage);
        flic_unkn03(anislot);
        old_flic_mods[part] = 0;
        new_current_drawing_mod = part;
        mod_draw_states[part] |= ModDSt_ModAnimOut;
        play_sample_using_heap(0, 132, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3);
        cryo_blokey_box.Flags &= ~GBxFlg_RadioBtn;
        byte_1DDC40 = 0;
        break;
    case ModDSt_IN:
        anislot = AniSl_CYBORG_INOUT;
        cryo_cyborg_mods_anim_set_fname(anislot, part, stage);
        flic_unkn03(anislot);
        flic_clear_output_buffer(anislot);
        new_current_drawing_mod = part;
        mod_draw_states[part] |= ModDSt_ModAnimIn;
        mod_draw_states[part] &= ~ModDSt_Unkn08;
        old_flic_mods[part] = flic_mods[part];
        cryo_blokey_box.Flags &= ~GBxFlg_RadioBtn;
        byte_1DDC40 = 0;
        break;
    case ModDSt_END:
        // No further action
        break;
    }
}

/** Blit cyborg parts screen background to screen and back buffer.
 *
 * Requires the additive background image to be already loaded
 * into framebuf_back buffer. Should be executed once per screen
 * refresh - in consecutive frames, the background from back_buffer
 * is used, and this function updates the content of back buffer.
 */
void blokey_bkgnd_data_to_screen(void)
{
    TbPixel *p_inp;
    short scr_x, scr_y;
    short h, scanln;

    scr_x = cryo_blokey_box.X + 63;
    scr_y = cryo_blokey_box.Y + 1;
    scanln = raw_file_scanline(equip_blokey_rect[ModDPt_BKGND].Width);
    h = equip_blokey_rect[ModDPt_BKGND].Height;

    p_inp = cryo_cyborg_part_buf_ptr(ModDPt_BKGND);

    LbScreenSetGraphicsWindow(scr_x, scr_y, scanln, h);

    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
    ApScreenCopyRemap(p_inp, lbDisplay.GraphicsWindowPtr,
        lbDisplay.GraphicsWindowHeight, appixmap.ghost_add_table);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR8;

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);

    // Copy to back buffer - the back buffer should contain background shape,
    // but any mods on it should be redrawn each frame
    LbScreenCopyBox(lbDisplay.WScreen, back_buffer,
        scr_x, scr_y, scr_x, scr_y, scanln, h);
}

/** Blit all color keyed cyborg parts from part buffer to screen.
 */
void blokey_part_buf_static_data_to_screen(void)
{
    short scr_base_x, scr_base_y;
    ubyte part;

    scr_base_x = cryo_blokey_box.X + 63;
    scr_base_y = cryo_blokey_box.Y + 1;

    for (part = 0; part < 4; part++)
    {
        TbPixel *p_inp;
        short scr_x, scr_y;
        short scanln, h;

        scr_x = scr_base_x + equip_blokey_rect[part].X;
        scr_y = scr_base_y + equip_blokey_rect[part].Y;
        scanln = raw_file_scanline(equip_blokey_rect[part].Width);
        h = equip_blokey_rect[part].Height;

        p_inp = cryo_cyborg_part_buf_ptr(part);

        LbScreenSetGraphicsWindow(scr_x, scr_y, scanln, h);

        ApScreenCopyColorKey(p_inp, lbDisplay.GraphicsWindowPtr,
            lbDisplay.GraphicsWindowHeight, 0);
    }

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
}

/** Blit color keyed breathing cyborg parts from part buffer to screen.
 *
 * While static parts are all separate, breathing parts consist
 * of two elements: chest+brain+arms, and legs.
 */
void blokey_part_buf_breath_data_to_screen(void)
{
    static ubyte ordered_parts[] = { ModDPt_BREATH, ModDPt_LEGS };
    short scr_base_x, scr_base_y;
    ubyte ordno;

    scr_base_x = cryo_blokey_box.X + 63;
    scr_base_y = cryo_blokey_box.Y + 1;

    for (ordno = 0; ordno < sizeof(ordered_parts)/sizeof(ordered_parts[0]); ordno++)
    {
        TbPixel *p_inp;
        short scr_x, scr_y;
        short scanln, h;

        ubyte part = ordered_parts[ordno];
        scr_x = scr_base_x + equip_blokey_rect[part].X;
        scr_y = scr_base_y + equip_blokey_rect[part].Y;
        scanln = raw_file_scanline(equip_blokey_rect[part].Width);
        h = equip_blokey_rect[part].Height;

        p_inp = cryo_cyborg_part_buf_ptr(part);

        LbScreenSetGraphicsWindow(scr_x, scr_y, scanln, h);

        ApScreenCopyColorKey(p_inp, lbDisplay.GraphicsWindowPtr,
            lbDisplay.GraphicsWindowHeight, 0);
    }

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
}

void cryo_cyborg_part_buf_blokey_static_reload(void)
{
    ubyte part;

    cryo_cyborg_part_buf_blokey_static_clear_all();
    cryo_cyborg_part_buf_blokey_static_load_all(flic_mods);

    for (part = 0; part < 4; part++)
    {
        if (flic_mods[part] == 0)
            continue;
        mod_draw_states[part] = ModDSt_Unkn04;
    }
}

ushort cryo_ordpart_to_mod_type(ubyte ordpart, ubyte mver)
{
    ushort mgroup;

    switch (ordpart)
    {
    default:
    case 0:
        mgroup = MODGRP_BRAIN;
        break;
    case 1:
        mgroup = MODGRP_ARMS;
        break;
    case 2:
        mgroup = MODGRP_CHEST;
        break;
    case 3:
        mgroup = MODGRP_EPIDERM;
        break;
    case 4:
        mgroup = MODGRP_LEGS;
        break;
    }
    return cybmod_type(mgroup, mver);
}

ubyte cryo_blokey_mod_level(ubyte ordpart)
{
    PlayerInfo *p_locplayer;
    ubyte mver;

    if (selected_agent < 0)
        return 0;

    switch (ordpart)
    {
    case 0:
        mver = flic_mods[ModDPt_BRAIN];
        break;
    case 1:
        mver = flic_mods[ModDPt_ARMS];
        break;
    case 2:
        mver = flic_mods[ModDPt_CHEST];
        break;
    case 3:
        p_locplayer = &players[local_player_no];
        if (selected_agent == 4)
        {
            mver = cybmod_skin_level(&p_locplayer->Mods[0]);
            if ((cybmod_skin_level(&p_locplayer->Mods[1]) != mver)
              || (cybmod_skin_level(&p_locplayer->Mods[2]) != mver)
              || (cybmod_skin_level(&p_locplayer->Mods[3]) != mver))
            {
              mver = 0;
            }
        }
        else
        {
            mver = cybmod_skin_level(&p_locplayer->Mods[selected_agent]);
        }
        break;
    case 4:
        mver = flic_mods[ModDPt_LEGS];
        break;
    }
    return mver;
}

void update_flic_mods(ubyte *mods)
{
#if 0
    asm volatile ("call ASM_update_flic_mods\n"
        : : "a" (mods));
#endif
    short plagent, i;
    ubyte lv;

    plagent = selected_agent;
    if (plagent < 4)
    {
        mods[0] = cybmod_chest_level(&cryo_agents.Mods[plagent]);
        mods[1] = cybmod_brain_level(&cryo_agents.Mods[plagent]);
        mods[2] = cybmod_arms_level(&cryo_agents.Mods[plagent]);
        mods[3] = cybmod_legs_level(&cryo_agents.Mods[plagent]);
        return;
    }

    for (i = 0; i < 4; i++)
        mods[i] = 3;

    for (plagent = 0; plagent < 4; plagent++)
    {
        lv = cybmod_chest_level(&cryo_agents.Mods[plagent]);
        if (lv < mods[0])
            mods[0] = lv;

        lv = cybmod_brain_level(&cryo_agents.Mods[plagent]);
        if (lv < mods[1])
            mods[1] = lv;

        lv = cybmod_arms_level(&cryo_agents.Mods[plagent]);
        if (lv < mods[2])
            mods[2] = lv;

        lv = cybmod_legs_level(&cryo_agents.Mods[plagent]);
        if (lv < mods[3])
            mods[3] = lv;
    }
}

/** Draws body mods, either images or anims, on pre-drawn background.
 *
 * A background with person outline is expected to be already drawn
 * when this function is called. This function draws the actual mods only.
 */
void draw_blokey_body_mods(void)
{
    int part;
    TbBool done, still_playing;

    if ((current_drawing_mod == ModDPt_BKGND) &&
      (new_current_drawing_mod != ModDPt_BKGND))
        // If previously we were in background drawing, all part buffers
        // need to be cleared (background occupied the same buffer)
        cryo_cyborg_part_buf_blokey_static_clear_all();
    else if ((current_drawing_mod == ModDPt_BREATH) &&
      (new_current_drawing_mod != ModDPt_BREATH))
        // If previously we were in breathing animation, chest+brain+arms
        // images have to be redrawn (breathing anim used the same buffer)
        cryo_cyborg_part_buf_blokey_static_load_all(old_flic_mods);

    current_drawing_mod = new_current_drawing_mod;
    if ((game_projector_speed || is_key_pressed(KC_SPACE, KMod_DONTCARE)) && (cryo_blokey_box.Flags & GBxFlg_RadioBtn) == 0)
    {
        clear_key_pressed(KC_SPACE);
        cryo_cyborg_part_buf_blokey_static_reload();
        update_flic_mods(old_flic_mods);
        update_flic_mods(flic_mods);

        clear_mod_draw_states();
        new_current_drawing_mod = ModDPt_CHEST;
        current_drawing_mod = ModDPt_CHEST;
        current_frame = 0;
        cryo_blokey_box.Flags |= GBxFlg_RadioBtn;
    }

    if ((mod_draw_states[0] & ModDSt_Unkn08) != 0)
    {
        if ((mod_draw_states[1] & ModDSt_Unkn04) != 0)
            mod_draw_states[1] |= ModDSt_Unkn08;
        if ((mod_draw_states[2] & ModDSt_Unkn04) != 0)
            mod_draw_states[2] |= ModDSt_Unkn08;
        if ((mod_draw_states[3] & ModDSt_Unkn04) != 0)
            mod_draw_states[3] |= ModDSt_Unkn08;
    }

    still_playing = 0;

    if (!still_playing)
    {
        for (part = 0; part < 4; part++)
        {
            if ((mod_draw_states[part] & ModDSt_ModAnimIn) == 0)
                continue;
            done = xdo_next_frame(AniSl_CYBORG_INOUT);
            cryo_cyborg_part_buf_blokey_fli_frame_copy(part, AniSl_CYBORG_INOUT);
            still_playing = 1;
            if (done)
            {
                mod_draw_states[part] &= ~(ModDSt_ModAnimIn | ModDSt_Unkn04);
                mod_draw_states[part] |= ModDSt_Unkn04;
                if (old_flic_mods[part] != flic_mods[part])
                    mod_draw_states[part] |= (ModDSt_Unkn08 | ModDSt_Unkn04);
                copy_box_purple_list(cryo_blokey_box.X - 3, cryo_blokey_box.Y - 3,
                  cryo_blokey_box.Width + 6, cryo_blokey_box.Height + 6);
            }
            break;
        }
    }

    if (!still_playing)
    {
        for (part = 0; part < 4; part++)
        {
            if ((mod_draw_states[part] & ModDSt_ModAnimOut) == 0)
                continue;
            done = xdo_prev_frame(AniSl_CYBORG_INOUT);
            cryo_cyborg_part_buf_blokey_fli_frame_copy(part, AniSl_CYBORG_INOUT);
            still_playing = 1;
            if (done)
            {
                mod_draw_states[part] &= ~(ModDSt_ModAnimOut | ModDSt_Unkn04);
                if (flic_mods[part] != 0)
                    mod_draw_states[part] |= ModDSt_Unkn08;
                copy_box_purple_list(cryo_blokey_box.X - 3, cryo_blokey_box.Y - 3,
                  cryo_blokey_box.Width + 6, cryo_blokey_box.Height + 6);
            }
        }
    }

    // If not drawing breathing animation, then draw static parts
    if (still_playing || (current_drawing_mod != ModDPt_BREATH))
    {
        draw_flic_purple_list(blokey_part_buf_static_data_to_screen);
    }

    if (!still_playing && (current_drawing_mod == ModDPt_BREATH))
    {
        done = xdo_next_frame(AniSl_CYBORG_BRTH);
        cryo_cyborg_part_buf_blokey_fli_frame_copy(current_drawing_mod, AniSl_CYBORG_BRTH);
        draw_flic_purple_list(blokey_part_buf_breath_data_to_screen);
        still_playing = !done;
        current_frame++;
        if (current_frame == 26) {
            play_sample_using_heap(0, 127, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        } else if (current_frame == 52) {
            current_frame = 0;
            play_sample_using_heap(0, 126, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
        }
    }
    if (!still_playing) {
        init_next_blokey_flic();
    }
}

ubyte draw_blokey_body_mods_names(struct ScreenBox *p_box)
{
    short cx, cy;
    short hline;
    ubyte ordpart;

    cx = p_box->X + 4;
    cy = p_box->Y + 20;
    hline = my_char_height('A');

    for (ordpart = 0; ordpart < 5; ordpart++)
    {
        const char *text;
        ubyte mver;

        mver = cryo_blokey_mod_level(ordpart);

        if (mver == 0)
        {
            if (ordpart == 3)
                cy += 2 * hline + 70;
            else
                cy += 2 * hline + 37;
            continue;
        }

        text = gui_strings[70 + ordpart];
        lbDisplay.DrawColour = 247;
        draw_text_purple_list2(cx, cy, text, 0);
        cy += hline + 3;

        text = cryo_gtext_cybmod_list_item_level(cryo_ordpart_to_mod_type(ordpart, mver));
        draw_text_purple_list2(cx, cy, text, 0);
        lbDisplay.DrawFlags = 0;
        if (ordpart == 3)
        {
            lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
            draw_box_purple_list(cx, cy + hline + 3, 40, 40, lbDisplay.DrawColour);
            draw_sprite_purple_list(cx + 1, cy + hline + 4, &fe_icons_sprites[163 + mver]);
            lbDisplay.DrawFlags = 0;
            cy += hline + 67;
        }
        else
        {
            cy += hline + 34;
        }
    }
    return 0;
}

/** Draws cryo agent with his cybernetic mods.
 *
 * The general flow is:
 * - store additive background (shape of the agent) in framebuf_back
 * - draw the background within drawlist, update back_buffer with
 *   the background image
 * - clear framebuf_back and store color keyed images/frames with mods
 *   into that buffer
 * - continue updating and blitting colour keyed framebuf_back
 *   on each frame
 */
ubyte show_cryo_blokey(struct ScreenBox *p_box)
{
    if ((p_box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        draw_flic_purple_list(blokey_bkgnd_data_to_screen);
        p_box->Flags |= GBxFlg_BkgndDrawn;
        update_flic_mods(old_flic_mods);
        update_flic_mods(flic_mods);
        reset_mod_draw_states_flag08();
        current_drawing_mod = ModDPt_BKGND;

        cryo_cyborg_part_buf_blokey_bkgnd_load();
        return 0;
    }

    if (word_15511E != selected_agent)
        p_box->Flags &= ~GBxFlg_RadioBtn;
    word_15511E = selected_agent;
    lbFontPtr = small_med_font;
    my_set_text_window(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);

    if (selected_agent < 0)
        return 0;

    draw_blokey_body_mods();
    draw_blokey_body_mods_names(p_box);
    return 0;
}

void switch_local_player_agents(ushort plagent1, ushort plagent2)
{
    PlayerInfo *p_locplayer;
    uint tmp;
    ushort wepfp;
    TbBool plagent1_female, plagent2_female;

    p_locplayer = &players[local_player_no];

    tmp = cryo_agents.RandomName[plagent2];
    cryo_agents.RandomName[plagent2] = cryo_agents.RandomName[plagent1];
    cryo_agents.RandomName[plagent1] = tmp;

    tmp = cryo_agents.Weapons[plagent2];
    cryo_agents.Weapons[plagent2] = cryo_agents.Weapons[plagent1];
    cryo_agents.Weapons[plagent1] = tmp;

    tmp = cryo_agents.Mods[plagent2].Mods;
    cryo_agents.Mods[plagent2].Mods = cryo_agents.Mods[plagent1].Mods;
    cryo_agents.Mods[plagent1].Mods = tmp;

    for (wepfp = 0; wepfp < 5; wepfp++)
    {
        tmp = cryo_agents.FourPacks[plagent2].Amount[wepfp];
        cryo_agents.FourPacks[plagent2].Amount[wepfp] = cryo_agents.FourPacks[plagent1].Amount[wepfp];
        cryo_agents.FourPacks[plagent1].Amount[wepfp] = tmp;

        p_locplayer->FourPacks[wepfp][plagent2] = cryo_agents.FourPacks[plagent2].Amount[wepfp];
        if (plagent1 < 4) {
            p_locplayer->FourPacks[wepfp][plagent1] = cryo_agents.FourPacks[plagent1].Amount[wepfp];
        }
    }

    plagent2_female = cryo_agents.Sex & (1 << plagent2);
    plagent1_female = cryo_agents.Sex & (1 << plagent1);
    if ((plagent2_female && !plagent1_female) || (!plagent2_female && plagent1_female))
    {
        if (plagent2_female)
            cryo_agents.Sex |= (1 << plagent1);
        else
            cryo_agents.Sex &= ~(1 << plagent1);
        if (plagent1_female)
            cryo_agents.Sex |= (1 << plagent2);
        else
            cryo_agents.Sex &= ~(1 << plagent2);
    }

    {
        p_locplayer->Weapons[plagent2] = cryo_agents.Weapons[plagent2];
        p_locplayer->Mods[plagent2] = cryo_agents.Mods[plagent2];
        if (plagent1 < 4) {
            p_locplayer->Weapons[plagent1] = cryo_agents.Weapons[plagent1];
            p_locplayer->Mods[plagent1] = cryo_agents.Mods[plagent1];
        }
    }
}

ubyte show_cryo_agent_list(struct ScreenTextBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_cryo_agent_list\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    int tx_width, ln_height;
    int lines_y1, lines_y2, shift_x;
    ushort plagent1;
    short i;
    short pos_y;

    if ((p_box->Flags & 0x8000) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(text_window_x1, text_window_y1,
        text_window_x2 - text_window_x1 + 1, text_window_y2 - text_window_y1 + 1, 56);
        lbDisplay.DrawFlags = 0;
        my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
        lbFontPtr = med_font;
        tx_width = my_string_width(gui_strings[430]);
        draw_text_purple_list2(((p_box->Width - tx_width) >> 1) - 3, 2, gui_strings[430], 0);
        p_box->Flags |= 0x8000;
        copy_box_purple_list(p_box->X + 4, p_box->Y - 3, p_box->Width - 20, p_box->Height + 6);
    }
    my_set_text_window(p_box->X + 4, p_box->ScrollWindowOffset + p_box->Y + 4,
      p_box->Width - 20, p_box->ScrollWindowHeight + 23);

    pos_y = 3;
    lbFontPtr = med_font;
    lbDisplay.DrawColour = 87;
    ln_height = 25;

    plagent1 = p_box->TextTopLine;
    while (plagent1 < p_box->Lines && pos_y + 22 < p_box->ScrollWindowHeight + 23)
    {
      // Check for enabling/disabling a team member
      lines_y1 = p_box->Y + pos_y + 31;
      lines_y2 = p_box->Y + pos_y + 53;
      if (plagent1 < 4 && mouse_down_over_box_coords(p_box->X + 7, lines_y1, p_box->X + 31, lines_y2))
      {
          if (lbDisplay.LeftButton)
          {
              lbDisplay.LeftButton = 0;

              if (login_control__State != LognCt_Unkn5) {
                  PlayerInfo *p_locplayer;

                  play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);

                  p_locplayer = &players[local_player_no];
                  if ((p_locplayer->MissionAgents & (1 << plagent1)) == 0) {
                      for (i = 0; i <= plagent1; i++)
                          p_locplayer->MissionAgents |= (1 << i);
                  } else {
                      for (i = plagent1; i < 4; i++)
                          p_locplayer->MissionAgents &= ~(1 << i);
                      if (p_locplayer->MissionAgents == 0)
                          p_locplayer->MissionAgents |= (1 << plagent1);
                  }
              } else {
                  play_sample_using_heap(0, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
              }
          }
      }

      // Check for replacing a team member with unused agent in cryo, or reordering team members
      lines_y1 = text_window_y1 + pos_y - 1;
      lines_y2 = text_window_y1 + pos_y + 23;
      if (mouse_down_over_box_coords(text_window_x1, lines_y1, text_window_x2, lines_y2))
      {
          if (lbDisplay.LeftButton)
          {
              lbDisplay.LeftButton = 0;

              if (login_control__State != LognCt_Unkn5 && selected_agent != -1) {
                  play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);

                  switch_local_player_agents(plagent1, selected_agent);
                  word_15511E = plagent1;
                  check_buy_sell_button();
                  update_flic_mods(flic_mods);
                  set_mod_draw_states_flag08();
              } else {
                  play_sample_using_heap(0, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
              }
          }
      }

      lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
      draw_box_purple_list(p_box->X + 7, pos_y + p_box->Y + 31, 0x18u, 0x16u, 56);

      lbDisplay.DrawFlags = 0;
      if (((1 << plagent1) & players[local_player_no].MissionAgents) != 0)
          lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
      if (plagent1 < 4)
          lbDisplay.DrawFlags |= 0x8000;

      {
          char locstr[16];
          const char *text;

          sprintf(locstr, "%d", plagent1 + 1);
          tx_width = LbTextStringWidth(locstr);
          shift_x = (24 - tx_width) >> 1;
          text = loctext_to_gtext(locstr);
          draw_text_purple_list2(4 + shift_x, pos_y + 6, text, 0);
      }

      lbDisplay.DrawFlags |= 0x8000;
      {
          const char *text;

          if (background_type == 1)
          {
              if ((cryo_agents.Sex & (1 << plagent1)) != 0)
                  text = gui_strings[227 + cryo_agents.RandomName[plagent1]];
              else
                  text = gui_strings[177 + cryo_agents.RandomName[plagent1]];
          }
          else
          {
              text = gui_strings[77 + cryo_agents.RandomName[plagent1]];
          }
          draw_text_purple_list2(30, pos_y + 6, text, 0);
      }
      lbDisplay.DrawFlags = 0;
      pos_y += ln_height;
      plagent1++;
    }
    return 0;
}

TbBool cybmod_available_for_purchase(short mtype)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];

    if (!is_research_cymod_completed(mtype)
      && ((login_control__State != LognCt_Unkn5) || mod_tech_level[mtype] > login_control__TechLevel))
        return false;

    if (selected_agent < 0)
        return false;

    if (selected_agent == 4)
    {
        ushort plagent;

        // If not purcheasing chest mods, require chest mods on all agents
        if (cybmod_group_type(mtype) != MODGRP_CHEST)
        {
            for (plagent = 0; plagent < 4; plagent++)
            {
                if (!cybmod_chest_level(&p_locplayer->Mods[plagent]))
                    break;
            }
            if (plagent < 4)
                return false;
        }
        return true;
    }
    {
        // If not purcheasing chest mod, require chest mod
        if (cybmod_group_type(mtype) != MODGRP_CHEST)
        {
            if (!cybmod_chest_level(&p_locplayer->Mods[selected_agent]))
                return false;
        }
    }
    return true;
}

void draw_display_box_content_mod(struct ScreenTextBox *p_box)
{
    ubyte real_dbcontent;

    real_dbcontent = cybmod_has_display_anim(selected_mod + 1) ? display_box_content : DiBoxCt_TEXT;
    switch (real_dbcontent)
    {
    case DiBoxCt_TEXT:
        lbFontPtr = small_font;
        my_set_text_window(p_box->X + 4, p_box->ScrollWindowOffset + p_box->Y + 4,
          p_box->Width - 20, p_box->ScrollWindowHeight + 23);
        flashy_draw_text(0, 0, p_box->Text, p_box->TextSpeed, p_box->TextTopLine,
          &p_box->TextFadePos, 0);
        break;
    case DiBoxCt_ANIM:
        if (p_box->TextFadePos < 0)
            // Mark that we should start animation frames the next time
            p_box->TextFadePos++;
        else
            xdo_next_frame(AniSl_EQVIEW);
        draw_flic_purple_list(ac_weapon_flic_data_to_screen);
        break;
    }
}

TbBool input_display_box_content_mod(struct ScreenTextBox *p_box)
{
    if (mouse_down_over_box_coords(p_box->X + 4, p_box->Y + 4,
      p_box->X + p_box->Width - 4, p_box->Y + 4 + 140))
    {
        if (lbDisplay.LeftButton)
        {
            lbDisplay.LeftButton = 0;
            display_box_content_state_switch();
            cryo_display_box_redraw(p_box);
            return true;
        }
    }
    return false;
}

/** Get global text pointer to a mod group name string.
 */
static const char *cryo_gtext_cybmod_list_item_name(ushort mtype)
{
    ubyte modgrp;
    ushort mdstr_id;

    modgrp = cybmod_group_type(mtype);
    mdstr_id = 70 + byte_1551F4[modgrp];
    return gui_strings[mdstr_id];
}

ubyte show_cryo_cybmod_list_box(struct ScreenTextBox *p_box)
{
    struct ScreenBoxBase power_box = {p_box->X + 8, p_box->Y + 152, 192, 17};
    struct ScreenBoxBase resil_box = {p_box->X + 8, p_box->Y + 177, 192, 17};
    struct ScreenBoxBase addit_box = {p_box->X + 8, p_box->Y + 200, 192, 19};

    if ((p_box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        // Highlight behind both power_box and resil_box
        draw_box_purple_list(p_box->X + 4, p_box->Y + 149, p_box->Width - 8, 48, 56);
        // Highlight in bottom part of additiional info box
        draw_box_purple_list(addit_box.X - 4, addit_box.Y + 10, addit_box.Width + 8, addit_box.Height - 10 + 4, 56);

        draw_discrete_rects_bar_bk(&power_box, gui_strings[432], byte_155174);
        draw_discrete_rects_bar_bk(&resil_box, gui_strings[433], byte_155180);
        draw_text_property_bk(&addit_box, gui_strings[434]);

        lbDisplay.DrawFlags = 0;
        p_box->Flags |= GBxFlg_BkgndDrawn;
        copy_box_purple_list(p_box->X + 4, p_box->Y + 4 + p_box->ScrollWindowOffset,
          p_box->Width - 20, p_box->ScrollWindowHeight + 23);

        copy_box_purple_list(p_box->X + 4, p_box->Y + 149, p_box->Width - 8, p_box->Height - 146);

        my_set_text_window(p_box->X + 4, p_box->ScrollWindowOffset + p_box->Y + 4,
          p_box->Width - 20, p_box->ScrollWindowHeight + 23);
        lbFontPtr = small_med_font;
    }

    if (selected_mod == -1) // No mod selected - show list of available ones
    {
        ushort mtype;
        short text_h;
        short cy;

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        // Highlight behind the main list
        draw_box_purple_list(text_window_x1, text_window_y1,
          text_window_x2 - text_window_x1 + 1, text_window_y2 - text_window_y1 + 1, 56);

        cy = 3;
        text_h = my_char_height('A');
        for (mtype = p_box->TextTopLine + 1; mtype < MOD_TYPES_COUNT; mtype++)
        {
            if (text_h + cy >= p_box->ScrollWindowHeight + 23)
                return 0;
            if (cybmod_available_for_purchase(mtype))
            {
                  const char *text;

                  if (mouse_down_over_box_coords(text_window_x1, cy + text_window_y1 - 1,
                    text_window_x2, cy + text_window_y1 + 1 + text_h))
                  {
                      if (lbDisplay.LeftButton) {
                          lbDisplay.LeftButton = 0;
                          selected_mod = mtype - 1;
                          cryo_update_for_selected_cybmod();
                      }
                  }
                  if (selected_mod == mtype - 1) {
                      lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
                      lbDisplay.DrawColour = 87;
                  } else {
                      lbDisplay.DrawFlags = 0;
                  }

                  text = cryo_gtext_cybmod_list_item_name(mtype);
                  lbDisplay.DrawFlags |= 0x8000;
                  draw_text_purple_list2(3, cy + 1, text, 0);
                  lbDisplay.DrawFlags &= ~(0x8000|Lb_TEXT_HALIGN_RIGHT);

                  text = cryo_gtext_cybmod_list_item_level(mtype);
                  lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_RIGHT;
                  draw_text_purple_list2(-1, cy + 1, text, 0);
                  lbDisplay.DrawFlags = 0;

                  cy += text_h + p_box->LineSpacing;
            }
        }
    }
    else
    {
        struct ModDef *mdef;

        mdef = &mod_defs[selected_mod + 1];

        draw_discrete_rects_bar_lv(&power_box, mdef->PowerOutput, 8, byte_155175);
        draw_discrete_rects_bar_lv(&resil_box, mdef->Resilience, 8, byte_155181);
        draw_text_property_lv(&addit_box, gui_strings[GSTR_MOD_EPIDERMIS + mdef->AdditProp]);
        lbDisplay.DrawFlags = 0;

        // Add control hotspot for the view / description switch
        draw_hotspot_purple_list(p_box->X + p_box->Width / 2, p_box->Y + 104);
        draw_display_box_content_mod(p_box);
        input_display_box_content_mod(p_box);

        //equip_offer_buy_button.DrawFn(&equip_offer_buy_button); -- incompatible calling convention
        asm volatile ("call *%1\n"
            : : "a" (&equip_offer_buy_button), "g" (equip_offer_buy_button.DrawFn));
        //cryo_offer_cancel_button.DrawFn(&cryo_offer_cancel_button); -- incompatible calling convention
        asm volatile ("call *%1\n"
            : : "a" (&cryo_offer_cancel_button), "g" (cryo_offer_cancel_button.DrawFn));
        //equip_cost_box.DrawFn(&equip_cost_box); -- incompatible calling convention
        asm volatile ("call *%1\n"
            : : "a" (&equip_cost_box), "g" (equip_cost_box.DrawFn));

        if (selected_mod == -1)
        {
            equip_cost_box.Flags = (GBxFlg_NoBkCopy|GBxFlg_Unkn0001);
            equip_offer_buy_button.Flags |= GBxFlg_Unkn0001;
            cryo_offer_cancel_button.Flags |= GBxFlg_Unkn0001;
        }
    }
    return 0;
}

void skip_flashy_draw_cryo_chamber_screen_boxes(void)
{
    short i;

    skip_flashy_draw_heading_screen_boxes();

    cryo_agent_list_box.Flags |= GBxFlg_Unkn0002;
    cryo_blokey_box.Flags |= GBxFlg_Unkn0002;
    equip_offer_buy_button.Flags |= GBxFlg_Unkn0002;
    equip_name_box.Flags |= GBxFlg_Unkn0002;
    cryo_cybmod_list_box.Flags |= GBxFlg_Unkn0002;
    equip_cost_box.Flags |= GBxFlg_Unkn0002;
    cryo_offer_cancel_button.Flags |= GBxFlg_Unkn0002;
    equip_all_agents_button.Flags |= GBxFlg_Unkn0002;
    for (i = 0; i < 5; i++) {
        equip_agent_select_shapes[i].Flags = GBxFlg_Unkn0002;
    }

    byte_1C4978 = 1;
    byte_1C4979 = 1;
}

ubyte do_cryo_all_agents_set(ubyte click)
{
    struct ScreenButton *button = &equip_all_agents_button;
    *(button->Radio) = button->RadioValue;
    check_buy_sell_button();
    update_flic_mods(flic_mods);
    set_mod_draw_states_flag08();
    return 1;
}

ubyte input_cryo_agent_panel_shape(struct ScreenShape *shape, sbyte nagent)
{
    ubyte gbstate;

    if (is_key_pressed(KC_1 + nagent, KMod_DONTCARE))
    {
        clear_key_pressed(KC_1 + nagent);
        if (nagent < cryo_agents.NumAgents)
        {
            selected_agent = nagent;
            check_buy_sell_button();
            update_flic_mods(flic_mods);
            set_mod_draw_states_flag08();
        }
    }

    if (mouse_over_agent_panel_shape(shape))
    {
        if ((shape->Flags & 0x0200) == 0) {
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
            shape->Flags |= 0x0200;
        }
        if (lbDisplay.MLeftButton || (joy.Buttons[0] != 0))
        {
            lbDisplay.LeftButton = 0;
            shape->Flags |= 0x0400;
            gbstate = GBxSta_PUSHED;
        }
        else
        {
            if ((nagent >= cryo_agents.NumAgents) || (mo_weapon == -1))
            {
                if ((shape->Flags & 0x0400) != 0)
                {
                    if (nagent < cryo_agents.NumAgents)
                    {
                        play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                        selected_agent = nagent;
                        check_buy_sell_button();
                        update_flic_mods(flic_mods);
                        set_mod_draw_states_flag08();
                    }
                    else
                    {
                        play_sample_using_heap(0, 129, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);
                    }
                    shape->Flags &= ~0x0400;
                }
            }
            else
            {
                // No drag and drop ability in mods screen
                mo_weapon = -1;
                shape->Flags &= ~0x0400;
            }
            gbstate = GBxSta_HLIGHT2;
        }
    }
    else
    {
        if (shape->Flags & 0x0200)
            shape->Flags &= ~0x0200;
        if (shape->Flags & 0x0400)
            shape->Flags &= ~0x0400;
        if ((selected_agent == nagent) || (selected_agent == 4))
        {
            gbstate = GBxSta_HLIGHT1;
        }
        else
        {
            gbstate = GBxSta_NORMAL;
        }
    }

    return gbstate;
}

ubyte input_cryo_all_agents_button(struct ScreenButton *button)
{
    ubyte gbstate;
    sbyte nagent = 4;

    gbstate = GBxSta_NORMAL;
    if (is_key_pressed(KC_1 + nagent, KMod_DONTCARE))
    {
        clear_key_pressed(KC_1 + nagent);
        selected_agent = nagent;
        check_buy_sell_button();
        update_flic_mods(flic_mods);
        set_mod_draw_states_flag08();
        gbstate = GBxSta_HLIGHT2;
    }
    return gbstate;
}

ubyte show_cryo_chamber_screen(void)
{
    ubyte drawn = true;

    if ((equip_agent_select_shapes[0].Flags & GBxFlg_Unkn0001) != 0)
    {
        byte_1C4978 = 0;
        byte_1C4979 = 0;
    }
    if (cryo_agent_list_box.Lines == 0)
        cryo_agent_list_box.Lines = cryo_agents.NumAgents;
    if (((game_projector_speed) && is_heading_flag01()) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_cryo_chamber_screen_boxes();
    }
    if ((ingame.UserFlags & UsrF_Cheats) != 0)
    {
        if (is_key_pressed(KC_0, KMod_DONTCARE))
        {
            clear_key_pressed(KC_0);
            refresh_equip_list = 1;
            research_cymod_complete(cheat_research_cybmods + 1);
            cheat_research_cybmods++;
        }
    }

    if (refresh_equip_list && selected_mod == -1)
    {
        ushort mtype;

        cryo_cybmod_list_box.Lines = 0;
        for (mtype = 1; mtype < MOD_TYPES_COUNT; mtype++)
        {
            if (cybmod_available_for_purchase(mtype)) {
                cryo_cybmod_list_box.Lines++;
            }
        }
        cryo_cybmod_list_box.Flags |= GBxFlg_Unkn0080;
        cryo_cybmod_list_box.LineHeight = 0;
        refresh_equip_list = 0;
    }

    if (drawn)
        drawn = draw_heading_box();

    if (drawn)
    {
        sbyte nagent;
        ubyte agnt[4];
        ubyte boxes_drawn;

        boxes_drawn = 1;
        for (nagent = 4; nagent >= 0; nagent--)
        {
            struct ScreenShape *shape;
            ubyte gbstate;

            shape = &equip_agent_select_shapes[nagent];

            if (nagent == 4) // agent name box
            {
                ubyte name_drawn;
                // Agents grouping has little to do with name box, but it's convienient to put here
                gbstate = input_cryo_all_agents_button(&equip_all_agents_button);

                if (byte_1C4979 == 0)
                {
                    name_drawn = flashy_draw_draw_equip_agent_name_shape(shape, gbstate);
                }
                else if (byte_1C4979 == 1)
                {
                    draw_equip_agent_name_shape(shape, gbstate);
                    name_drawn = 3;
                }
                byte_1C4979 = (name_drawn == 3);
            }
            else
            {
                ubyte drawn;
                ushort spridx;

                gbstate = input_cryo_agent_panel_shape(shape, nagent);

                if (byte_1C4978 == 0)
                {
                    drawn = flashy_draw_agent_panel_shape(shape, gbstate);
                }
                else
                {
                    spridx = 140 + nagent;
                    draw_agent_panel_shape(shape, spridx, gbstate);
                    drawn = 3;
                }
                // Is the flashy draw finished for current button
                agnt[nagent] = (drawn == 3);
                // When all buttons started actually drawing, we can begin flashy draw of the panels below
                boxes_drawn &= (drawn > 1);
            }
        }
        if (byte_1C4978 == 0)
        {
            byte_1C4978 = agnt[0] && agnt[1] && agnt[2] && agnt[3];
        }
        drawn = boxes_drawn;
    }

    if (drawn)
    {
        //drawn = equip_all_agents_button.DrawFn(&equip_all_agents_button); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_all_agents_button), "g" (equip_all_agents_button.DrawFn));
        //drawn = cryo_agent_list_box.DrawFn(&cryo_agent_list_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&cryo_agent_list_box), "g" (cryo_agent_list_box.DrawFn));
    }

    if (drawn)
    {
        //drawn = cryo_blokey_box.DrawFn(&cryo_blokey_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&cryo_blokey_box), "g" (cryo_blokey_box.DrawFn));
    }

    if (drawn)
    {
        //drawn = cryo_cybmod_list_box.DrawFn(&cryo_cybmod_list_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&cryo_cybmod_list_box), "g" (cryo_cybmod_list_box.DrawFn));
        //drawn = equip_name_box.DrawFn(&equip_name_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&equip_name_box), "g" (equip_name_box.DrawFn));
    }

    return drawn;
}

void init_cryo_screen_boxes(void)
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

    init_screen_text_box(&cryo_agent_list_box, 7u, 122u, 196u, 303, 6,
        small_med_font, 1);
    cryo_agent_list_box.LineHeight = 25;
    cryo_agent_list_box.DrawTextFn = ac_show_cryo_agent_list;
    cryo_agent_list_box.ScrollWindowOffset += 27;
    cryo_agent_list_box.Flags |= (GBxFlg_RadioBtn|GBxFlg_IsMouseOver);
    cryo_agent_list_box.ScrollWindowHeight -= 27;

    init_screen_box(&cryo_blokey_box, 212u, 122u, 203u, 303, 6);
    cryo_blokey_box.SpecialDrawFn = show_cryo_blokey;

    init_screen_text_box(&cryo_cybmod_list_box, 425u, 153u, 208u, 272,
      6, small_med_font, 1);
    cryo_cybmod_list_box.DrawTextFn = ac_show_cryo_cybmod_list_box;
    cryo_cybmod_list_box.Flags |= (GBxFlg_RadioBtn|GBxFlg_IsMouseOver);
    cryo_cybmod_list_box.ScrollWindowHeight = 117;
    // Re-use equip_name_box above cryo_cybmod_list_box
    // Re-use agests selection with equip_all_agents_button, agent name label, and sprites representing agents

    init_screen_button(&cryo_offer_cancel_button, 628u, 404u,
      gui_strings[437], 6, med2_font, 1, 0x80);
    cryo_offer_cancel_button.CallBackFn = ac_do_cryo_offer_cancel;

    // Reposition the components to current resolution

    start_x = heading_box.X;
    // On the X axis, we're going for aligning below heading box, to both left and right
    space_w = heading_box.Width - cryo_agent_list_box.Width - cryo_blokey_box.Width - cryo_cybmod_list_box.Width;

    start_y = heading_box.Y + heading_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    // One re-used box - cyborg name - does not exist as global instance, so count all agents button twice.
    space_h = scr_h - start_y - 2 * equip_all_agents_button.Height - cryo_blokey_box.Height + border;

    // On the X axis, aligning to heading box left
    cryo_agent_list_box.X = start_x;
    cryo_blokey_box.X = cryo_agent_list_box.X + cryo_agent_list_box.Width + space_w / 2;
    cryo_cybmod_list_box.X = cryo_blokey_box.X + cryo_blokey_box.Width + space_w - space_w / 2;

    // There is one box only to Y-position in 1st column, so space goes into two parts - before and after
    cryo_blokey_box.Y = start_y + 2 * equip_all_agents_button.Height + 3 * space_h / 4;
    // The remaining boxes should be Y-aligned to the one box in 1st column
    cryo_agent_list_box.Y = cryo_blokey_box.Y;
    cryo_cybmod_list_box.Y = cryo_blokey_box.Y + cryo_blokey_box.Height - cryo_cybmod_list_box.Height;

    // Boxes defining areas done; now reposition components inside

    space_w = 5;
    space_h = 5;
    cryo_offer_cancel_button.X = cryo_cybmod_list_box.X + cryo_cybmod_list_box.Width - cryo_offer_cancel_button.Width - space_w;
    cryo_offer_cancel_button.Y = cryo_cybmod_list_box.Y + cryo_cybmod_list_box.Height - cryo_offer_cancel_button.Height - space_h;
    // No need to update equip_cost_box - that is done in switch_shared_equip_screen_buttons_to_cybmod()
}

void reset_cryo_screen_player_state(void)
{
    selected_mod = -1;
    selected_agent = 0;
}

void switch_shared_equip_screen_buttons_to_cybmod(void)
{
    short space_w, space_h;

    space_w = 5;
    space_h = 5;
    set_heading_box_text(gui_strings[369]);

    equip_cost_box.X = cryo_cybmod_list_box.X + space_w;
    equip_cost_box.Width = cryo_cybmod_list_box.Width - 2 * space_h;
    equip_cost_box.Y = cryo_offer_cancel_button.Y - space_h - equip_cost_box.Height;

    equip_name_box.Text = cybmod_name_text;
    equip_all_agents_button.CallBackFn = ac_do_cryo_all_agents_set;

    update_cybmod_name_text();
    switch_equip_offer_to_buy();
    cryo_display_box_redraw(&cryo_cybmod_list_box);
}

void set_flag01_cryo_screen_boxes(void)
{
    cryo_offer_cancel_button.Flags |= GBxFlg_Unkn0001;
}

void reset_cryo_screen_boxes_flags(void)
{
    cryo_cybmod_list_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
    cryo_agent_list_box.Flags = GBxFlg_Unkn0001 | GBxFlg_RadioBtn | GBxFlg_IsMouseOver;
    cryo_blokey_box.Flags = GBxFlg_Unkn0001;
}

/******************************************************************************/
