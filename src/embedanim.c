/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file embedanim.c
 *     Embedded animations, for use in both GUI and in-game.
 * @par Purpose:
 *     Implement functions for handling embedded FLIC animations, in menu
 *     and in-game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Jan 2023 - 03 Aug 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "embedanim.h"

#include <assert.h>
#include "bfflic.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfpalette.h"
#include "bfscreen.h"
#include "bfutility.h"
#include "engintxtrmap.h"
#include "ssampply.h"

#include "fecryo.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "sound.h"
#include "swlog.h"
#include "thing.h"
/******************************************************************************/
ubyte byte_154BB4[] = {
  220, 224, 224, 222, 220, 220,
};

ubyte billboard_anim_no;
TbBool anim_can_change_palette = false;

ubyte active_anim;
extern ubyte anim_slots[];
extern struct Animation animations[2];

uint embanim_current_frame_number(ubyte anislot)
{
    struct Animation *p_anim;
    ulong k;

    k = anim_slots[anislot];
    p_anim = &animations[k];

    return p_anim->FrameNumber;
}

static void embanim_show_draw_next_frame(struct Animation *p_anim)
{
    ubyte pal_change;

    pal_change = anim_show_frame(p_anim);
    p_anim->FrameNumber++;

    if (pal_change)
    {
        LbScreenWaitVbi();
        if (anim_can_change_palette) {
            LbPaletteSet(anim_palette);
        }
    }
}

int embanim_do_next_frame(ubyte anislot)
{
    struct Animation *p_anim;
    ushort k;

    k = anim_slots[anislot];
    active_anim = k;
    p_anim = &animations[k];

    if (anislot >= AniSl_EQVIEW && anislot <= AniSl_CYBORG_INOUT)
    {
        if (p_anim->FrameNumber == 0) {
            play_sample_using_heap(0, 135, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        } else if (p_anim->FrameNumber == p_anim->FLCFileHeader.NumberOfFrames >> 1) {
            play_sample_using_heap(0, 115, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        }
    }

    if (p_anim->FrameNumber >= p_anim->FLCFileHeader.NumberOfFrames)
    {
        anim_flic_close(p_anim);
        if ((p_anim->Flags & 0x20) != 0) {
            embanim_reinit(p_anim->Type);
        }
        return 1;
    }

    anim_show_prep_next_frame(p_anim, embanim_type_get_output_buffer(p_anim->Type));
    embanim_show_draw_next_frame(p_anim);

    return 0;
}

int embanim_do_prev_frame(ubyte anislot)
{
    struct Animation *p_anim;
    ubyte *p_frmbuf;
    uint i, rq_frame;
    ushort k;

    k = anim_slots[anislot];
    active_anim = k;
    p_anim = &animations[k];

    if (p_anim->FrameNumber == 0)
        rq_frame = p_anim->FLCFileHeader.NumberOfFrames;
    else
        rq_frame = p_anim->FrameNumber - 1;

    p_frmbuf = embanim_type_get_output_buffer(p_anim->Type);

    if (rq_frame == 0)
    {
        LbMemorySet(p_frmbuf, 0, p_anim->FLCFileHeader.Width * p_anim->FLCFileHeader.Height);
        anim_flic_close(p_anim);
        if ((p_anim->Flags & 0x20) != 0) {
            embanim_reinit(p_anim->Type);
        }
        return 1;
    }

    anim_flic_show_replay(p_anim);
    LbMemorySet(p_frmbuf, 0, p_anim->FLCFileHeader.Width * p_anim->FLCFileHeader.Height);
    anim_show_prep_next_frame(p_anim, p_frmbuf);
    embanim_show_draw_next_frame(p_anim);
    for (i = 1; i < rq_frame; i++)
    {
        anim_show_prep_next_frame(p_anim, NULL);
        embanim_show_draw_next_frame(p_anim);
    }
    return 0;
}

void anim_show_FLI_SS2_NP(void)
{
    struct Animation *p_anim;

    p_anim = &animations[active_anim];
    anim_show_FLI_SS2(p_anim);
}

void anim_show_FLI_BRUN_NP(void)
{
    struct Animation *p_anim;

    p_anim = &animations[active_anim];
    anim_show_FLI_BRUN(p_anim);
}

void anim_show_FLI_LC_NP(void)
{
    struct Animation *p_anim;

    p_anim = &animations[active_anim];
    anim_show_FLI_LC(p_anim);
}

ubyte *embanim_type_get_output_buffer(ubyte anislot)
{
    switch (anislot)
    {
    case AniSl_FULLSCREEN:
    default:
        return lbDisplay.WScreen;
    case AniSl_BILLBOARD:
        return vec_tmap[4];
    case AniSl_EQVIEW:
    case AniSl_UNKN4:
    case AniSl_UNKN5:
    case AniSl_UNKN6:
    case AniSl_UNKN7:
    case AniSl_NETSCAN:
        return vec_tmap[5];
    case AniSl_CYBORG_INOUT:
    case AniSl_CYBORG_BRTH:
        return vec_tmap[5] + 0x8000;
    case AniSl_SCRATCH:
        return vec_tmap[4] + 0x8000;
    }
}

static void embanim_billboard_select_rand(void)
{
    ushort rnd;

    rnd = LbRandomPosShort() & 7;
    if (rnd <= 0)
        billboard_anim_no = 1;
    else if (rnd <= 2)
        billboard_anim_no = 2;
    else if (rnd <= 5)
        billboard_anim_no = 0;
    else
        billboard_anim_no = 3;
}

static void embanim_billboard_select_next(void)
{
    billboard_anim_no++;
    if (billboard_anim_no > 3)
        billboard_anim_no = 0;
}

static void embanim_billboard_broadcast_sound(void)
{
    struct Thing *p_thing;
    ushort rnd;
    ubyte smpl_no;

    if (in_network_game)
        return;
    if (ingame.VisibleBillboardThing == 0)
        return;

    p_thing = &things[ingame.VisibleBillboardThing];
    smpl_no = byte_154BB4[billboard_anim_no];
    rnd = LbRandomPosShort() & 1;
    play_dist_sample(p_thing, smpl_no + rnd, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
}

void embanim_clear_output_buffer(ubyte anislot)
{
    struct Animation *p_anim;
    int k;

    k = anim_slots[anislot];
    p_anim = &animations[k];
    if (anim_is_opened(p_anim))
    {
        ubyte *obuf;
        short h;

        obuf = embanim_type_get_output_buffer(p_anim->Type);

        for (h = p_anim->FLCFileHeader.Height; h > 0; h--)
        {
            LbMemorySet(obuf, '\0', p_anim->FLCFileHeader.Width);
            obuf += p_anim->FLCFileHeader.Width;
        }
    }
}

void embanim_reinit(ubyte anislot)
{
    struct Animation *p_anim;
    ubyte *p_frmbuf;
    PathInfo *pinfo;
    int k;

    k = anim_slots[anislot];
    p_anim = &animations[k];
    if (anim_is_opened(p_anim)) {
        anim_flic_close(p_anim);
    }

    anim_scratch = scratch_buf1;
    anim_flic_init(p_anim, anislot, 0x00);
    p_frmbuf = embanim_type_get_output_buffer(anislot);

    switch (anislot)
    {
    case AniSl_BILLBOARD:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 0, 0, 0, 0x20);
        embanim_billboard_select_rand();
        embanim_billboard_broadcast_sound();
        pinfo = &game_dirs[DirPlace_QData];
        anim_flic_set_fname(p_anim, "%s/%s-1%d.fli", pinfo->directory, "demo", (int)billboard_anim_no);
        embanim_billboard_select_next();
        break;
    case AniSl_EQVIEW:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 0, 0, 0, 0x00);
        break;
    case AniSl_CYBORG_INOUT:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 0, 0, 0, 0x00);
        break;
    case AniSl_UNKN4:
        anim_can_change_palette = true;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 0, 0, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "intro");
        break;
    case AniSl_UNKN5:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 10, 30, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "mcomp");
        break;
    case AniSl_UNKN6:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 10, 30, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "mcomp");
        break;
    case AniSl_UNKN7:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 10, 30, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "mcomp");
        break;
    case AniSl_CYBORG_BRTH:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 0, 0, 0, 0x20);
        break;
    case AniSl_NETSCAN:
        anim_can_change_palette = false;
        anim_flic_set_frame_buffer(p_anim, p_frmbuf, 0, 0, 0, 0x00);
        break;
      default:
        break;
    }

    if (anim_flic_show_open(p_anim) == Lb_FAIL)
    {
        if (anislot == AniSl_BILLBOARD)
            ingame.Flags &= ~GamF_BillboardMovies;
        return;
    }
    p_anim->anfield_4 += 12;
}

void embanim_init(void)
{
    uint k;

    for (k = 0; k < sizeof(animations)/sizeof(animations[0]); k++) {
        struct Animation *p_anim;

        p_anim = &animations[k];
        anim_flic_init(p_anim, AniSl_SCRATCH, 0x00);
    }
}

void embanim_set_netscan_file(ubyte anislot, ubyte netno)
{
    struct Animation *p_anim;
    PathInfo *pinfo;
    int k;

    k = anim_slots[anislot];
    p_anim = &animations[k];

    pinfo = &game_dirs[DirPlace_Equip];

    anim_flic_set_fname(p_anim, "%s/net%02d.fli", pinfo->directory, (int)netno);
}

void embanim_set_weapon_model_file(ubyte anislot, ubyte wtype)
{
    struct Animation *p_anim;
    PathInfo *pinfo;
    ulong k;

    k = anim_slots[anislot];
    p_anim = &animations[k];

    pinfo = &game_dirs[DirPlace_Equip];

    anim_flic_set_fname(p_anim, "%s/wep-%02d.fli", pinfo->directory, (int)wtype);
}

void embanim_set_cybmod_model_file(ubyte anislot, ubyte mtype)
{
    struct Animation *p_anim;
    PathInfo *pinfo;
    ulong k;

    k = anim_slots[anislot];
    p_anim = &animations[k];

    pinfo = &game_dirs[DirPlace_Equip];

    anim_flic_set_fname(p_anim, "%s/mod-%02d.fli", pinfo->directory, (int)mtype);
}

void embanim_set_cyborg_part_file(ubyte anislot, ubyte part, ubyte stage)
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

/******************************************************************************/
