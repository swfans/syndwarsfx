/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file sound.h
 *     Header file for sound.c.
 * @par Purpose:
 *     Sound related routines.
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
#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>
#include <stdint.h>

#include "bftypes.h"
#include "mssal.h"

/******************************************************************************/

#pragma pack(1)

enum SounsHeapSetupCmd {
    /** Load sounds from a bank selected based on available memory. */
    SHSC_GameSndAutoQ = 0,
    /** Unload sounds and free memory. */
    SHSC_ResetGameSnd,
    /** Load best quality sounds only */
    SHSC_GameSndBestQ,
    /** Load sounds for game credits screen. */
    SHSC_CreditsSnd = 100,
};

struct Thing;
struct SimpleThing;

struct HeapMgrHandle {
  ubyte field_0[14];
  ubyte field_E;
};

struct SampleTable {
  long field_0;
  long field_4;
  struct HeapMgrHandle *hmhandle;
};

#define STARTSCR_VOLUME_MAX 322

#pragma pack()

extern short startscr_samplevol;
extern short startscr_midivol;
extern short startscr_cdvolume;

/******************************************************************************/

void set_default_sfx_settings(void);
void sfx_apply_samplevol(void);
void sfx_apply_midivol(void);
void sfx_apply_cdvolume(void);
void fill_ail_sample_ids(void);
void monitor_all_samples(void);

struct SampleInfo *play_sample_using_heap(ulong bank_id, short smptbl_id,
  ulong volume, ulong pan, ulong pitch, sbyte loop_count, ubyte type);

void stop_sample_using_heap(long source_id, ulong sample_number);
void play_dist_sample(struct Thing *p_thing, ushort smptbl_id, ushort vol, ushort pan, int pitch, int loop, ubyte type);
int play_dist_speech(struct Thing *p_thing, ushort samp, ushort vol, ushort pan, int pitch, int loop, ubyte type);
void play_dist_ssample(struct SimpleThing *p_sthing, ushort smptbl_id, ushort vol, ushort pan, int pitch, int loop, ubyte type);
void play_disk_sample(short id, ushort sample, short vol, short pan, int pitch, int loop, int type);
void stop_looped_weapon_sample(struct Thing *p_person, short weapon);

void snd_unkn1_volume_all_samples(void);
void wait_for_sound_sample_finish(ushort smpl_id);

void setup_heaps(short setup_cmd, const char *lang);
void reset_heaps(void);

/******************************************************************************/
#endif
