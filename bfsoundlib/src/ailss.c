/******************************************************************************/
// Bullfrog Sound Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file ailss.c
 *     OpenAL based reimplementation of MSS API.
 * @par Purpose:
 *     SS functions from MSS API.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Jun 2022 - 05 Sep 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "ailss.h"

#include "aildebug.h"
#include "ail.h"
#include "memfile.h"
#include "mssdig.h"
#include "drv_oal.h"
/******************************************************************************/
/** Recognized file types
 */
enum SampleFileTypes {
    SMP_FTYP_VOC = 0,
    SMP_FTYP_WAV = 1,
    SMP_FTYP_ASI = 2,
};

/******************************************************************************/

static int32_t SS_use_locked;

static const uint8_t volume_pan_table[128] = {
    0,   2,   4,   6,   8,  10,  12,  14,  16,  18,  20,  22,  24,  26,  28,  30,
   32,  34,  36,  38,  40,  42,  44,  46,  48,  50,  52,  54,  56,  58,  60,  62,
   64,  66,  68,  70,  72,  74,  76,  78,  80,  82,  84,  86,  88,  90,  92,  94,
   96,  98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
  128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
};

/******************************************************************************/

void AILSFILE_end(void);

/** Lock function, doubling as start of locked code.
 */
void AILSFILE_start(void)
{
    if (SS_use_locked)
        return;

    AIL_VMM_lock_range(AILSFILE_start, AILSFILE_end);
    SS_use_locked = 1;
}

void SS_build_amplitude_tables(SNDSAMPLE *s)
{
    int hwfmt, smfmt;
    int32_t final_volume;
    int32_t i;

    if (s->volume > 127)
        s->volume = 127;
    else if (s->volume < 0)
        s->volume = 0;

    if (s->pan > 127)
        s->pan = 127;
    else if (s->pan < 0)
        s->pan = 0;

    final_volume = s->driver->master_volume * s->volume / 127;
    if (final_volume > 127)
        final_volume = 127;
    else if (final_volume < 0)
        final_volume = 0;

    if (s->format == DIG_F_MONO_16 && s->format == DIG_F_STEREO_16)
    {
        int32_t *vscale0, *vscale1;
        int32_t vamp0, vamp1;

        vscale0 = s->vol_scale[0];
        vscale1 = s->vol_scale[1];
        vamp1 = volume_pan_table[s->pan];
        vamp0 = volume_pan_table[127 - s->pan];

        vscale0[0] = final_volume * vamp0 / 127;
        vscale1[0] = final_volume * vamp1 / 127;
        return;
    }

    if (final_volume)
        final_volume++;

    hwfmt = s->driver->hw_format;
    smfmt = s->format;
    if (hwfmt != DIG_F_STEREO_8 && hwfmt != DIG_F_STEREO_16 &&
      ((hwfmt != DIG_F_MONO_8 && hwfmt != DIG_F_MONO_16) ||
        (smfmt != DIG_F_STEREO_8 && smfmt != DIG_F_STEREO_16)) )
    {
        int32_t *vscale;
        int32_t nxval;

        vscale = s->vol_scale[0];
        if (s->flags & DIG_PCM_SIGN)
        {
            nxval = 0;
            for (i = 0; i < 0x8000; i += 0x100)
            {
              *(vscale) = nxval >> 7;
              nxval += final_volume << 8;
              vscale++;
            }
            nxval = -0x8000 * final_volume;
            for (i = -0x8000; i < 0; i += 0x100)
            {
              *(vscale) = nxval >> 7;
              nxval += final_volume << 8;
              vscale++;
            }
        }
        else
        {
            nxval = -0x8000 * final_volume;
            for (i = -0x8000; i < 0x8000; i += 0x100)
            {
              *(vscale) = nxval >> 7;
              nxval += final_volume << 8;
              vscale++;
            }
        }
    }
    else
    {
        int32_t *vscale0, *vscale1;
        int32_t vamp0, vamp1;
        int32_t nxval;

        vscale0 = s->vol_scale[0];
        vscale1 = s->vol_scale[1];
        vamp1 = volume_pan_table[s->pan];
        vamp0 = volume_pan_table[127 - s->pan];
        if (s->flags & DIG_PCM_SIGN)
        {
            nxval = 0;
            for (i = 0; i < 0x8000; i += 256)
            {
              *vscale0 = vamp0 * (nxval >> 7) >> 7;
              *vscale1 = vamp1 * (nxval >> 7) >> 7;
              nxval += final_volume << 8;
              vscale0++;
              vscale1++;
            }
            nxval = -0x8000 * final_volume;
            for (i = -0x8000; i < 0; i += 256)
            {
              *vscale0 = vamp0 * (nxval >> 7) >> 7;
              *vscale1 = vamp1 * (nxval >> 7) >> 7;
              nxval += final_volume << 8;
              vscale0++;
              vscale1++;
            }
        }
        else
        {
            nxval = -0x8000 * final_volume;
            for (i = -0x8000; i < 0x8000; i += 256)
            {
              *vscale0 = vamp0 * (nxval >> 7) >> 7;
              *vscale1 = vamp1 * (nxval >> 7) >> 7;
              nxval += final_volume << 8;
              vscale0++;
              vscale1++;
            }
        }
    }
}

void SS_flush(DIG_DRIVER *digdrv)
{
    // Initialize the build buffer by flushing with 0
    memset(digdrv->build_buffer, 0, digdrv->build_size);
}

void SS_copy(DIG_DRIVER *digdrv, void *pWaveAddr)
{
#if defined(DOS)||defined(GO32)
    AILSSA_DMA_copy((pWaveAddr == dig->DMA[0] ) ? 0 : 1,
          &dig->DMA[0], dig->hw_mode_flags, dig->hw_format,
          dig->n_active_samples, dig->build_buffer,
          dig->samples_per_buffer, dig->half_buffer_size);
#else
    // TODO not sure what is needed here
#endif
}

int32_t SS_configure_buffers(DIG_DRIVER *digdrv)
{
    int32_t i, bsmin, bsmax, hb_samples;
    int32_t n, match, delta;
    int32_t pref[4];
    int32_t DMA_status;

    DMA_status = digdrv->playing;

    if (DMA_status)
    {
        // Pause all playing samples
        for (i = 0; i < digdrv->n_samples; i++)
        {
            // use one of system_data[] slots for temporary storage,
            // the storage place becomes free again when this function ends
            digdrv->samples[i].system_data[SmpSD_TEMP] =
                digdrv->samples[i].status;

            if (digdrv->samples[i].status == SNDSMP_PLAYING)
                digdrv->samples[i].status = SNDSMP_STOPPED;
        }

        // Halt DMA playback and flush DMA buffers with silence
        SS_stop_DIG_driver_playback(digdrv);
        SS_flush(digdrv);
        SS_copy(digdrv, digdrv->DMA[0]);
        SS_copy(digdrv, digdrv->DMA[1]);
    }

    // Select hardware output format based on application's preference
    // If desired mode is unavailable, downgrade/upgrade to other modes
    switch ((AIL_preference[DIG_USE_STEREO] << 1)
        | (AIL_preference[DIG_USE_16_BITS]))
    {
    case DIG_F_MONO_8:
        pref[0] = DIG_F_MONO_8;
        pref[1] = DIG_F_MONO_16;
        pref[2] = DIG_F_STEREO_8;
        pref[3] = DIG_F_STEREO_16;
        break;

    case DIG_F_MONO_16:
        pref[0] = DIG_F_MONO_16;
        pref[1] = DIG_F_MONO_8;
        pref[2] = DIG_F_STEREO_16;
        pref[3] = DIG_F_STEREO_8;
        break;

    case DIG_F_STEREO_8:
        pref[0] = DIG_F_STEREO_8;
        pref[1] = DIG_F_STEREO_16;
        pref[2] = DIG_F_MONO_8;
        pref[3] = DIG_F_MONO_16;
        break;

    default:
    case DIG_F_STEREO_16:
        pref[0] = DIG_F_STEREO_16;
        pref[1] = DIG_F_STEREO_8;
        pref[2] = DIG_F_MONO_16;
        pref[3] = DIG_F_MONO_8;
        break;
    }
    // Check what the driver marked as supported
    for (i = 0; i < 4; i++)
    {
        if (digdrv->DDT->format_supported[pref[i]])
        {
            digdrv->hw_format = pref[i];
            break;
        }
    }

    digdrv->hw_mode_flags = digdrv->DDT->format_data[digdrv->hw_format].flags;

    // Select physical output rate based on application's preference
    switch (AIL_preference[DIG_HARDWARE_SAMPLE_RATE])
    {
    case AILPREF_MIN_VAL:
        digdrv->DMA_rate =
            digdrv->DDT->format_data[digdrv->hw_format].minimum_physical_sample_rate;
        break;

    case AILPREF_NOM_VAL:
        digdrv->DMA_rate =
            digdrv->DDT->format_data[digdrv->hw_format].nominal_physical_sample_rate;
        break;

    case AILPREF_MAX_VAL:
        digdrv->DMA_rate =
            digdrv->DDT->format_data[digdrv->hw_format].maximum_physical_sample_rate;
        break;

    default:
        // If preference set to explicit frequency, use best match with
        // current driver
        n = AIL_preference[DIG_HARDWARE_SAMPLE_RATE];
        pref[0] =
            digdrv->DDT->format_data[digdrv->hw_format].minimum_physical_sample_rate;
        pref[1] =
            digdrv->DDT->format_data[digdrv->hw_format].nominal_physical_sample_rate;
        pref[2] =
            digdrv->DDT->format_data[digdrv->hw_format].maximum_physical_sample_rate;

        match = DIG_F_STEREO_16;
        delta = LONG_MAX;
        for (i = 0; i < 3; i++)
        {
            if (abs(n - pref[i]) <= delta) {
                delta = abs(n - pref[i]);
                match = i;
            }
        }
        digdrv->DMA_rate = pref[match];
        break;
    }

    // Set sample size values
    switch (digdrv->hw_format)
    {
    case DIG_F_MONO_8:
        digdrv->channels_per_sample = 1;
        digdrv->bytes_per_channel = 1;
        break;

    case DIG_F_STEREO_8:
        digdrv->channels_per_sample = 2;
        digdrv->bytes_per_channel = 1;
        break;

    case DIG_F_MONO_16:
        digdrv->channels_per_sample = 1;
        digdrv->bytes_per_channel = 2;
        break;

    case DIG_F_STEREO_16:
        digdrv->channels_per_sample = 2;
        digdrv->bytes_per_channel = 2;
        break;
    }

    // Get # of samples per half-buffer, assuming desired latency
    hb_samples = (digdrv->DMA_rate * AIL_preference[DIG_LATENCY]) / 1000;

    // Calculate half-buffer size in bytes
    digdrv->half_buffer_size = hb_samples * digdrv->bytes_per_channel
        * digdrv->channels_per_sample;

    // Make sure half-buffer size is legal with this driver
    bsmin = digdrv->DDT->format_data[digdrv->hw_format].minimum_DMA_half_buffer_size;
    bsmax = digdrv->DDT->format_data[digdrv->hw_format].maximum_DMA_half_buffer_size;

    // Round half-buffer size to nearest binary power between 8 and
    // DIG_DMA_RESERVE / 2; ensure result within driver limits
    delta = LONG_MAX;
    match = bsmax;

    for (n = 8; n <= (AIL_preference[DIG_DMA_RESERVE] / 2); n <<= 1)
    {
        if (abs(n - digdrv->half_buffer_size) <= delta)
        {
            delta = abs(n - digdrv->half_buffer_size);
            match = n;
        }
    }

    digdrv->half_buffer_size = match;
    if (digdrv->half_buffer_size < bsmin)
        digdrv->half_buffer_size = bsmin;
    if (digdrv->half_buffer_size > bsmax)
        digdrv->half_buffer_size = bsmax;

    // Set DMA half-buffer segments in driver DST, and store equivalent
    // protected-mode pointers in driver descriptor structure
    digdrv->DST->DMA_buffer_A = digdrv->DMA_buf;
    digdrv->DST->DMA_buffer_B = (digdrv->DMA_buf + digdrv->half_buffer_size);

    digdrv->DMA[0] = digdrv->DST->DMA_buffer_A;
    digdrv->DMA[1] = digdrv->DST->DMA_buffer_B;

    if (DMA_status)
    {
        // Resume any interrupted samples
        for (i = 0; i < digdrv->n_samples; i++)
            digdrv->samples[i].status =
                digdrv->samples[i].system_data[SmpSD_TEMP];

        // Flush new DMA buffers with silence and resume DMA playback
        SS_flush(digdrv);
        SS_copy(digdrv, digdrv->DMA[0]);
        SS_copy(digdrv, digdrv->DMA[1]);
        SS_start_DIG_driver_playback(digdrv);
    }

    return 1;
}

void SS_stream_to_buffer(SNDSAMPLE *s)
{
  //TODO though this is not used with OpenAL
}

void AIL2OAL_API_init_sample(SNDSAMPLE *s)
{
#if 0
    asm volatile (
      "push %0\n"
      "call ASM_AIL_API_init_sample\n"
      "add $0x4, %%esp\n"
        :  : "g" (s) : "eax" );
#endif
    if (s == NULL)
        return;

    // Set status to FREE while manipulating vars, to keep callback thread
    // from reading invalid data
    s->status = SNDSMP_FREE;

    // Shut down any previously-active pipeline providers
    //   and init pipeline stages
    AIL_set_sample_processor(s, SNDSMST_SAMPLE_ALL_STAGES, 0);

    // Initialize sample vars
    s->start[0] = NULL;
    s->len[0] = 0;
    s->pos[0] = 0;
    s->done[0] = 0;

    s->start[1] = NULL;
    s->len[1] = 0;
    s->pos[1] = 0;
    s->done[1] = 1;

    s->current_buffer = 0;
    s->last_buffer = -2;

    s->loop_count = 1;
    s->format = 0;
    s->flags = 0;
    s->volume = AIL_preference[5];
    s->playback_rate = 11025;
    if ((s->driver->hw_format != 0) && (s->driver->hw_format != 1))
        s->pan = 64;
    else
        s->pan = 0;
    s->vol_scale[1][255] = 0;

    s->SOB = NULL;
    s->EOB = NULL;
    s->EOS = NULL;

    // Mark sample initialized
    s->status = SNDSMP_DONE;

    SS_build_amplitude_tables(s);
}

SNDSAMPLE *AIL2OAL_API_allocate_sample_handle(DIG_DRIVER *digdrv)
{
    int32_t i;
    SNDSAMPLE *s;

    if (digdrv == NULL)
        return NULL;

    AIL_lock();

    // Look for an unallocated sample structure
    for (i = 0; i < digdrv->n_samples; i++)
    {
        if (digdrv->samples[i].status == SNDSMP_FREE)
            break;
    }

    if (i == digdrv->n_samples)
    {
        AIL_set_error("Out of sample handles.");
        AIL_unlock();
        return NULL;
    }

    s = &digdrv->samples[i];

    // Initialize sample to status SNDSMP_DONE with nominal
    // sample attributes
    AIL_init_sample(s);

    AIL_unlock();
    return s;
}

/** Instead of setting hardware volume, scales volume for all samples.
 */
void set_master_hardware_volume(DIG_DRIVER *digdrv)
{
    SNDSAMPLE *s;
    int i;

    for (i = 0; i < digdrv->n_samples; i++)
    {
        s = &digdrv->samples[i];
        if (s->status != SNDSMP_FREE)
            SS_build_amplitude_tables(s);
    }
}

int32_t AIL2OAL_API_digital_master_volume(DIG_DRIVER *digdrv)
{
    if (digdrv == NULL)
        return 0;

    return digdrv->master_volume;
}

void AIL2OAL_API_set_digital_master_volume(DIG_DRIVER *digdrv, int32_t master_volume)
{
    if (digdrv == NULL)
        return;

    digdrv->master_volume = master_volume;

    set_master_hardware_volume(digdrv);
}

void AIL2OAL_API_start_sample(SNDSAMPLE *s)
{
    if (s == NULL)
        return;

    if (s->status == SNDSMP_FREE)
        return;

    // Make sure valid sample data exists
    if ((s->len[s->current_buffer] == 0) ||
     (s->start[s->current_buffer] == NULL))
        return;

    // Rewind sample to beginning
    s->pos[s->current_buffer] = 0;

    s->status = SNDSMP_PLAYING;

    // If sample's driver is not already transmitting data, start it
    SS_start_DIG_driver_playback(s->driver);
}

void AIL2OAL_API_end_sample(SNDSAMPLE *s)
{
    if (s == NULL)
        return;

    // Make sure sample has been allocated
    if (s->status == SNDSMP_FREE)
        return;

    // If sample is not already done, halt it and invoke its end-of-buffer
    // and end-of-sample callback functions
    if (s->status != SNDSMP_DONE)
    {
        s->status = SNDSMP_DONE;

        OPENAL_stop_sample(s);

        if (s->EOB != NULL)
            s->EOB(s);
        if (s->EOS != NULL)
            s->EOS(s);
    }
}

void AIL2OAL_API_stop_sample(SNDSAMPLE *s)
{
    if (s == NULL)
        return;

    // Make sure sample is currently playing
    if (s->status != SNDSMP_PLAYING)
        return;

    // Mask 'playing' status
    s->status = SNDSMP_STOPPED;
}

void AIL2OAL_API_resume_sample(SNDSAMPLE *s)
{
    if (s == NULL)
        return;

    if (s->status == SNDSMP_FREE)
        return;

    // See if sample has been previously stopped or not managed to start yet
    if ((s->status != SNDSMP_STOPPED) && (s->status != SNDSMP_FREE))
        return;

    // Extra checks if resume got called before start (good for non-zero start offsets)
    if (s->status == SNDSMP_FREE)
    {
        // Make sure valid sample data exists
        if ((s->len[s->current_buffer] == 0) ||
         (s->start[s->current_buffer] == NULL))
            return;
    }

    // Start reverb if parameters say it is needed - NO SUPPORT

    s->status = SNDSMP_PLAYING;

    // If driver is not already transmitting data, start it
    SS_start_DIG_driver_playback(s->driver);
}

uint32_t AIL2OAL_API_sample_status(SNDSAMPLE *s)
{
    assert(s != NULL);
    return s->status;
}

/** Get double-buffer playback status for sample.
 *
 * @return Gives one of:
 *   0: Buffer 0 is ready to be filled (buffer 1 or neither buffer playing),
 *   1: Buffer 1 is ready to be filled (buffer 0 playing),
 *  -1: Both buffers are already full.
 */
int32_t AIL2OAL_API_sample_buffer_ready(SNDSAMPLE *s)
{
    if (s == NULL)
        return -1;

    // For first two calls after sample initialization, return 0 and 1,
    // respectively
    //
    // This allows the application to "prime" the buffers for continued
    // playback
    switch (s->last_buffer)
    {
    case -2:
        // First call after AIL_init_sample() must clear second buffer's
        // "done" flag to permit buffer-switching
        s->done[1] = 0;

        // Set up to load buffer 0 this call, and "bootstrap" buffer 1 at
        // next call
        s->last_buffer = -1;
        return 0;

    case -1:
        // Return 1 to force load of second buffer immediately
        // Subsequent calls should operate on alternating buffers
        s->last_buffer = s->current_buffer;
        return 1;
    }

    // If buffer has not switched since last call
    if (s->last_buffer == s->current_buffer)
        return -1;

    // New current_buffer exists -- set last_buffer equal to
    // current_buffer and return exhausted buffer
    s->last_buffer = s->current_buffer;

    return (s->current_buffer ^ 1);
}

int32_t AIL2OAL_API_set_sample_file(SNDSAMPLE *s, const void *file_image, int32_t block)
{
    int32_t ftype;

    if ((s == NULL) || (file_image == NULL))
        return 0;

    AIL_init_sample(s);

    ftype = -1;
    if (ftype == -1)
    {
        if (strncasecmp(file_image, "Creative", 8) == 0)
            ftype = SMP_FTYP_VOC;
    }
    if (ftype == -1)
    {
        if (strncasecmp(file_image + 8, "WAVE", 4) == 0)
            ftype = SMP_FTYP_WAV;
    }
    if (ftype == -1)
    {
        AIL_set_error("Unknown digital audio file type.");
        return 0;
    }

    // Copy file attributes to sample
    switch (ftype)
    {
    case SMP_FTYP_WAV:
        s->system_data[SmpSD_RELEASE] = 0;
        AIL_process_WAV_image(file_image, s);
        break;
    case SMP_FTYP_VOC:
#if !defined(LBS_ENABLE_STRUCTS_EXPAND)
        AIL_set_error("VOC samples are not compatible with OpenAL unless STRUCTS_EXPAND is used.");
#endif
        // Store pointer to sample data
        s->system_data[SmpSD_VOC_BLK_PTR] =
          (uintptr_t)file_image + *(uint16_t *)(file_image + 20);
        s->system_data[SmpSD_RELEASE] = 0;
#if defined(LBS_ENABLE_STRUCTS_EXPAND) // without extra fields, these are used by OpenAL
        s->system_data[SmpSD_VOC_MRKR] = block;
        s->system_data[SmpSD_VOC_MRKR_FND] = (block != -1);
#endif
        AIL_process_VOC_block(s, 0);
        break;
    default: // s->system_data[6] == -1
        AIL_set_error("Invalid or missing data block.");
        return 0;
    }
    return 1;
}

void AIL2OAL_API_release_sample_handle(SNDSAMPLE *s)
{
    if (s == NULL)
        return;

    s->status = SNDSMP_FREE;

    // Shut down any installed pipeline providers
    AIL_set_sample_processor(s, SNDSMST_SAMPLE_ALL_STAGES, 0);
}

AILSAMPLECB AIL2OAL_API_register_EOS_callback(SNDSAMPLE *s, AILSAMPLECB EOS)
{
    AILSAMPLECB old;

    if (s == NULL)
        return NULL;

    old = s->EOS;
    s->EOS = EOS;

    return old;
}

HAILPROVIDER AIL2OAL_API_set_sample_processor(SNDSAMPLE *s,
    uint32_t pipeline_stage, HAILPROVIDER provider)
{
    // No need to implement - filters are rarely used
    return 0;
}

void AIL2OAL_API_set_sample_address(SNDSAMPLE *s, const void *start, uint32_t len)
{
    if (s == NULL)
        return;

    s->start[0] = (void *)start;
    s->len[0]   = len;

    s->start[1] = NULL;
    s->len[1]   = 0;
}

int32_t AIL2OAL_API_sample_playback_rate(SNDSAMPLE *s)
{
    if (s == NULL)
        return 0;

    return s->playback_rate;
}

void AIL2OAL_API_set_sample_playback_rate(SNDSAMPLE *s, int32_t playback_rate)
{
    if (s == NULL)
        return;

    if (playback_rate < 1)
        return;

    s->playback_rate = playback_rate;
}

int32_t AIL2OAL_API_sample_volume(SNDSAMPLE *s)
{
    if (s == NULL)
        return 0;

    return s->volume;
}

void AIL2OAL_API_set_sample_volume(SNDSAMPLE *s, int32_t level)
{
    if (s == NULL)
        return;

    if (level == s->volume)
        return;

    s->volume = level;
    SS_build_amplitude_tables(s);
}

int32_t AIL2OAL_API_sample_pan(SNDSAMPLE *s)
{
    if (s == NULL)
        return 0;

    return s->pan;
}

void AIL2OAL_API_set_sample_pan(SNDSAMPLE *s, int32_t level)
{
    if (s == NULL)
        return;

    if (level == s->pan)
        return;

    s->pan = level;
    SS_build_amplitude_tables(s);
}

void AIL2OAL_API_set_sample_type(SNDSAMPLE *s, int32_t format, uint32_t flags)
{
    if (s == NULL)
        return;

    if ((format == s->format) && (flags == s->flags))
        return;

    s->format = format;
    s->flags  = flags;

    SS_build_amplitude_tables(s);
}

intptr_t AIL2OAL_API_sample_user_data(SNDSAMPLE *s, uint32_t index)
{
    if (s == NULL)
        return 0;

    return s->user_data[index];
}

void AIL2OAL_API_set_sample_user_data(SNDSAMPLE *s, uint32_t index, intptr_t value)
{
    if (s == NULL)
        return;
    s->user_data[index] = value;
}

static int32_t nibbles_per_sample(int32_t format)
{
    switch (format)
    {
    case DIG_F_MONO_8:
        return 1;
    case DIG_F_MONO_16:
    case DIG_F_STEREO_8:
        return 2;
    case DIG_F_STEREO_16:
        return 4;
    default:
        return 8;
    }
}

int32_t AIL2OAL_API_minimum_sample_buffer_size(DIG_DRIVER *digdrv,
  int32_t playback_rate, int32_t format)
{
    int32_t app_nibbles_per_sample;
    int32_t hw_nibbles_per_sample;
    int32_t n;

    // Get # of nibbles per sample unit
    app_nibbles_per_sample = nibbles_per_sample(format);
    hw_nibbles_per_sample = digdrv->bytes_per_channel * digdrv->channels_per_sample;

    n = digdrv->half_buffer_size * app_nibbles_per_sample / hw_nibbles_per_sample;

    n = (playback_rate * n) / digdrv->DMA_rate;

    // Scale n by 2X resampling tolerance to provide safety margin
    n = n + ( (n * AIL_preference[DIG_RESAMPLING_TOLERANCE]) / 32768);

    // If DMA rate is not 1X, 2X, or 4X times playback rate, round buffer
    // size up 1 sample to avoid possible truncation errors
    if ((digdrv->DMA_rate != 1 * playback_rate) &&
      (digdrv->DMA_rate != 2 * playback_rate) &&
      (digdrv->DMA_rate != 4 * playback_rate))
        n += 4;

    // Round n up to nearest multiple of 256 bytes
    return (n + 255) & ~255;
}

void AIL2OAL_API_load_sample_buffer(SNDSAMPLE *s, int32_t buff_num, void *buffer, uint32_t len)
{
    if (s == NULL)
        return;

    s->done[buff_num] = (len == 0);
    s->start[buff_num] = buffer;
    s->len[buff_num] = len;
    s->pos[buff_num] = 0;

    if (len)
    {
        if (s->status != SNDSMP_PLAYING) {
            s->status = SNDSMP_PLAYING;
            SS_start_DIG_driver_playback(s->driver);
        }
    }
}

void AIL2OAL_API_set_sample_loop_count(SNDSAMPLE *s, int32_t loop_count)
{
    if (s == NULL)
        return;

    s->loop_count = loop_count;
}

/** Unlock function, doubling as end of locked code.
 */
void AILSFILE_end(void)
{
    if (!SS_use_locked)
        return;

    AIL_VMM_unlock_range(AILSFILE_start, AILSFILE_end);
    SS_use_locked = 0;
}

/******************************************************************************/
