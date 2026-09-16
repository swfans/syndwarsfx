/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file sound.c
 *     Sound related routines.
 * @par Purpose:
 *     Implements routines which help adjust functions from bfsoundlib to
 *     the games needs.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 02 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "sound.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfwindows.h"
#include "bffile.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfmusic.h"
#include "bfsound.h"
#include "bfaudio.h"
#include "bfscd.h"
#include "ailss.h"
#include "aila.h"
#include "sb16.h"
#include "awe32.h"
#include "streamfx.h"
#include "sndtimer.h"
#include "snderr.h"
#include "ssampply.h"
#include "ssamplst.h"

#include "display.h"
#include "game_data.h"
#include "game_options.h"
#include "game_speed.h"
#include "game.h"
#include "thing.h"
#include "util.h"
#include "swlog.h"

/******************************************************************************/

#pragma pack(1)

struct HeapMgrHeader { // sizeof=0x24
    void *field_0;
    void *field_4;
    int field_8;
    int samples_count;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
};

#pragma pack()

extern long sound_heap_size;
extern struct SampleTable *sound_heap_memory;
extern TbFileHandle sound_file; // = INVALID_FILE;
extern struct HeapMgrHeader *hmhead;
extern long samples_in_bank;
extern struct SampleTable *sample_table;

/******************************************************************************/

void set_default_sfx_settings(void)
{
    startscr_samplevol = STARTSCR_VOLUME_MAX;
    startscr_midivol = STARTSCR_VOLUME_MAX;
    startscr_cdvolume = STARTSCR_VOLUME_MAX * 71 / 100;
}

void sfx_apply_samplevol(void)
{
    if (startscr_samplevol < 0)
        startscr_samplevol = 0;
    if (startscr_samplevol > STARTSCR_VOLUME_MAX)
        startscr_samplevol = STARTSCR_VOLUME_MAX;
    SetSoundMasterVolume(127 * startscr_samplevol / STARTSCR_VOLUME_MAX);
}

void sfx_apply_midivol(void)
{
    if (startscr_midivol < 0)
        startscr_midivol = 0;
    if (startscr_midivol > STARTSCR_VOLUME_MAX)
        startscr_midivol = STARTSCR_VOLUME_MAX;
    SetMusicMasterVolume(127 * startscr_midivol / STARTSCR_VOLUME_MAX);
}

void sfx_apply_cdvolume(void)
{
    if (startscr_cdvolume < 0)
        startscr_cdvolume = 0;
    if (startscr_cdvolume > STARTSCR_VOLUME_MAX)
        startscr_cdvolume = STARTSCR_VOLUME_MAX;
    SetCDVolume(70 * (127 * startscr_cdvolume / STARTSCR_VOLUME_MAX) / 100);
}

struct SampleInfo *play_sample_using_heap(ulong bank_id, short smptbl_id,
  ulong volume, ulong pan, ulong pitch, sbyte loop_count, ubyte stype)
{
    struct SampleInfo *ret;
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[3];

    stkargs[0] = (int)(intptr_t)pitch;
    stkargs[1] = (int)(intptr_t)loop_count;
    stkargs[2] = (int)(intptr_t)stype;

    asm volatile (
      "push 8(%5)\n"
      "push 4(%5)\n"
      "push 0(%5)\n"
      "call ASM_play_sample_using_heap\n"
        : "=r" (ret)
        : "a" (bank_id), "d" (smptbl_id), "b" (volume), "c" (pan), "S" (stkargs)
        : "cc", "memory");
    return ret;
}

void stop_sample_using_heap(long source_id, ulong sample_number)
{
    asm volatile (
      "call ASM_stop_sample_using_heap\n"
        : : "a" (source_id), "d" (sample_number));
}

int play_dist_speech(struct Thing *p_thing, ushort speech_no, ushort vol,
  ushort pan, int pitch, int loop, ubyte stype)
{
    if ((p_thing <= &things[0]) || (p_thing >= &things[THINGS_LIMIT])) {
        LOGERR("Speech %hu playback requested on invalid thing", speech_no);
        return -1;
    }
    play_dist_sample(p_thing, 129 + speech_no,  vol, pan, pitch, loop, stype);
    return 0;
}

void play_disk_sample(short id, ushort sample, short vol,
  short pan, int pitch, int loop, int stype)
{
    play_sample_using_heap(9999, 129 + sample, vol, pan, pitch, loop, stype);
}

void play_dist_sample(struct Thing *p_thing, ushort smptbl_id, ushort vol,
  ushort pan, int pitch, int loop, ubyte stype)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[3];

    stkargs[0] = (int)(intptr_t)pitch;
    stkargs[1] = (int)(intptr_t)loop;
    stkargs[2] = (int)(intptr_t)stype;

    asm volatile (
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_play_dist_sample\n"
        : : "a" (p_thing), "d" (smptbl_id), "b" (vol), "c" (pan), "S" (stkargs)
        : "cc", "memory");
}

void play_dist_ssample(struct SimpleThing *p_sthing, ushort smptbl_id, ushort vol,
  ushort pan, int pitch, int loop, ubyte stype)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[3];

    stkargs[0] = (int)(intptr_t)pitch;
    stkargs[1] = (int)(intptr_t)loop;
    stkargs[2] = (int)(intptr_t)stype;

    asm volatile (
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_play_dist_ssample\n"
        : : "a" (p_sthing), "d" (smptbl_id), "b" (vol), "c" (pan), "S" (stkargs)
        : "cc", "memory");
}

void stop_looped_weapon_sample(struct Thing *p_person, short weapon)
{
    asm volatile ("call ASM_stop_looped_weapon_sample\n"
        : : "a" (p_person), "d" (weapon));
}

//TODO better name?
void fill_ail_sample_ids(void)
{
    asm volatile ("call ASM_fill_ail_sample_ids\n"
        :  :  : "eax" );
}

void monitor_all_samples(void)
{
    asm volatile ("call ASM_monitor_all_samples\n"
        :  :  : "eax" );
    return;
}

void snd_unkn1_volume_all_samples(void)
{
    asm volatile ("call ASM_snd_unkn1_volume_all_samples\n"
        :  :  : "eax" );
}

void wait_for_sound_sample_finish(ushort smpl_id)
{
    while (IsSamplePlaying(0, smpl_id, NULL)) {
        swap_wscreen();
        game_update();
    }
}

struct HeapMgrHeader *heapmgr_init(void *p_buf, int buf_size, int n_samples)
{
#if 0
    struct HeapMgrHeader *ret;
    asm volatile ("call ASM_heapmgr_init\n"
        : "=r" (ret) : "a" (p_buf), "d" (buf_size), "b" (n_samples));
    return ret;
#endif
    struct HeapMgrHeader *p_hmhead;
    uint offs_after_samples;
    int offs3;

    p_hmhead = (struct HeapMgrHeader *)p_buf;
    offs_after_samples = 0x1C * n_samples + sizeof(struct HeapMgrHeader);
    offs3 = buf_size - offs_after_samples;
    if (offs3 <= 0)
        return NULL;

    p_hmhead->field_10 = 0;
    p_hmhead->field_14 = 0;
    p_hmhead->field_18 = 0;
    p_hmhead->field_1C = 0;
    p_hmhead->field_20 = 0;
    p_hmhead->field_8 = offs3;
    p_hmhead->samples_count = n_samples;
    p_hmhead->field_0 = (ubyte *)p_hmhead + offs_after_samples;
    p_hmhead->field_4 = (ubyte *)p_hmhead + buf_size;
    return p_hmhead;
}

int setup_heap_manager(struct SampleTable *smptable, size_t smptb_len,
  const char *fname, ushort sndtype)
{
    TbFileHandle fh;
    ubyte tpno;
    long totlen;
    long pos;
    struct SampleTable *smptb;
    size_t tab_smptb_len;
    struct BfSoundBankHead *sbh;
    int i;
    long smptb_len_diff;
    ubyte *p_smptb_end;
    struct BfSoundBankHead sbharr[9];
    struct BfSfxInfo sfxi;
    ubyte unkhead1[18];

    LbMemorySet(smptable, 0, smptb_len);
    fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
    sound_file = fh;
    if (fh == INVALID_FILE) {
        reset_heaps();
        return 0;
    }
    tpno = GetSoundTpNo(sndtype);
    if (tpno >= 255) {
        reset_heaps();
        return 0;
    }

    totlen = LbFileLengthHandle(fh);
    LbFileSeek(fh, totlen - 4, Lb_FILE_SEEK_BEGINNING);
    LbFileRead(fh, &pos, 4);
    LbFileSeek(fh, pos, Lb_FILE_SEEK_BEGINNING);
    LbFileRead(fh, &unkhead1, 18);
    LbFileRead(fh, &sbharr, sizeof(struct BfSoundBankHead) * 9);
    sbh = &sbharr[tpno];
    if ((sbh->TabPos == 0) || (sbh->TabSize == 0)) {
        reset_heaps();
        return 0;
    }
    samples_in_bank = sbh->TabSize / sizeof(struct BfSfxInfo);
    tab_smptb_len = sizeof(struct SampleTable) * samples_in_bank;
    if (tab_smptb_len >= smptb_len) {
        reset_heaps();
        return 0;
    }
    LbFileSeek(fh, sbh->TabPos, Lb_FILE_SEEK_BEGINNING);
    smptb = smptable;
    for (i = 0; i < samples_in_bank; i++)
    {
        LbFileRead(fh, &sfxi, sizeof(struct BfSfxInfo));
        smptb->field_0 = sbh->DatPos + (long)sfxi.DataBeg;
        smptb->hmhandle = 0;
        smptb->field_4 = (long)sfxi.DataEnd;
        smptb++;
    }
    if (samples_in_bank <= 0) {
        reset_heaps();
        return 0;
    }
    sample_table = smptable;
    smptb_len_diff = smptb_len - tab_smptb_len;
    p_smptb_end = (ubyte *)smptable + tab_smptb_len;
    if (smptb_len_diff <= 0)
        return 0;
    hmhead = heapmgr_init(p_smptb_end, smptb_len_diff, samples_in_bank);
    if (hmhead == NULL) {
        reset_heaps();
        return 0;
    }
    reset_heaps();
    return 1;
}

void reset_heaps(void)
{
}

TbResult sound_banks_fname(char *fname, const char *dir, const char *lang)
{
    sprintf(fname, "%s/sound_%s.dat", dir, lang);
    if (LbFileExists(fname)) {
        return Lb_OK;
    }
    // If no language-specific version found, use the english as default
    sprintf(fname, "%s/sound_%s.dat", dir, "eng");
    if (LbFileExists(fname)) {
        return Lb_OK;
    }

    // As last effort, try the old file name from original game
    sprintf(fname, "%s/sound.dat", dir);
    if (LbFileExists(fname)) {
        return Lb_OK;
    }

    return Lb_FAIL;
}

void setup_heaps(short setup_cmd, const char *lang)
{
    char locstr[DISKPATH_SIZE];
    PathInfo *pinfo;
    long sz;
    unsigned int n;

    if ((ingame.Flags & GamF_Unkn00020000) == 0)
        return;
    pinfo = &game_dirs[DirPlace_Sound];

    switch (setup_cmd)
    {
    case SHSC_CreditsSnd:
        sprintf(locstr, "%s/syncreds.dat", pinfo->directory);
        sz = sound_heap_size;
        setup_heap_manager(sound_heap_memory, sz, locstr, 1622);
        break;
    case 99:
        break;
    case SHSC_GameSndAutoQ:
        sz = 1500000;
        while ((sound_heap_memory == NULL) && (sz > 100))
        {
            sound_heap_memory = LbMemoryAlloc(sz);
            if (sound_heap_memory == NULL)
                sz -= 30000;
        }
        if ((sound_heap_memory == NULL) || sz < 120000) {
            ingame.Flags &= ~GamF_Unkn00020000;
            break;
        }
        if (sz <= 500000)
            n = 822;
        else
            n = 1622;
        if (sound_banks_fname(locstr, pinfo->directory, lang) == Lb_FAIL) {
            LOGERR("Matching sound banks file not found.");
            ingame.Flags &= ~GamF_Unkn00020000;
            break;
        }
        if (!setup_heap_manager(sound_heap_memory, sz, locstr, n))
            ingame.Flags &= ~GamF_Unkn00020000;
        sound_heap_size = sz;
        break;
    case SHSC_GameSndBestQ:
        sz = 1500000;
        if (sound_banks_fname(locstr, pinfo->directory, lang) == Lb_FAIL) {
            LOGERR("Matching sound banks file not found.");
            ingame.Flags &= ~GamF_Unkn00020000;
            break;
        }
        if (!setup_heap_manager(sound_heap_memory, sz, locstr, 1622))
            ingame.Flags &= ~GamF_Unkn00020000;
        sound_heap_size = sz;
        break;
    case SHSC_ResetGameSnd:
    default:
        sz = 1500000;
        reset_heaps();
        LbMemoryFree(sound_heap_memory);
        sound_heap_size = sz;
        break;
    }
}

/******************************************************************************/
