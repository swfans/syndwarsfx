/******************************************************************************/
// Bullfrog Engine Smacker Playback Library - for use to remake classic games
// like Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file bfsmack.c
 *     External interface of the smacker playback library.
 * @par Purpose:
 *     Surfaces used for drawing on screen or compositing layers.
 * @par Comment:
 *     Depends on the video support library, which is SDL in this implementation.
 * @author   Tomasz Lis
 * @date     12 Nov 2011 - 13 Jun 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bfsmack.h"

#include <assert.h>
#include <string.h>

#include "bfkeybd.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfpalette.h"
#include "bfplanar.h"
#include "bfscreen.h"
#include "bfscrsurf.h"
#include "bfsvaribl.h"

#include "smack2ail.h"
#include "smailfile.h"

/******************************************************************************/
//SmackDrawCallback smack_draw_callback = NULL;
void *(*smack_malloc)(uint32_t);
void (*smack_free)(void *);

void RADAPI SmackTimerSetup(void);
void RADAPI SmackTimerDone(void);

// Currenlty only in the main game
//TODO make a copy here, we only need the bflibrary calls
extern int game_hacky_update(void);

extern ubyte byte_1E56DC[PALETTE_8b_SIZE];
extern uint32_t simspeed;
extern uint32_t forcerate;

#define __DS__ 0
/******************************************************************************/

void set_smack_malloc(void *(*cb)(uint32_t))
{
    smack_malloc = cb;
}

void set_smack_free(void (*cb)(void *ptr))
{
    smack_free = cb;
}

//TODO place SMACK* functions into separate file
int32_t SmackGetSizeTables(void)
{
    return 0x7468;
}

int32_t blockread(struct Smack *p_smk, uint8_t *buf, int32_t sz)
{
#if 1
    int32_t ret;
    asm volatile (
      "call ASM_blockread\n"
        : "=r" (ret) : "a" (p_smk), "d" (buf), "b" (sz));
    return ret;
#endif
}

int32_t readframe(struct Smack *p_smk)
{
    if ((p_smk->OpenFlags & 0x200) != 0)
        return 0;
    p_smk->field_3B0 = p_smk->IOBuffer;
    return blockread(p_smk, p_smk->IOBuffer, p_smk->field_3B8[p_smk->CurrFrameNum]);
}

int backgroundload(struct Smack *p_smk)
{
#if 1
    int ret;
    asm volatile (
      "call ASM_backgroundload\n"
        : "=r" (ret) : "a" (p_smk));
    return ret;
#endif
}

void SMACKSIMULATE(uint32_t speed)
{
    simspeed = speed;
}

uint8_t GetFirstTrack(u32 *p_a1)
{
#if 1
    uint8_t ret;
    asm volatile (
      "call ASM_GetFirstTrack\n"
        : "=r" (ret) : "a" (p_a1));
    return ret;
#endif
}


uint8_t soundopen(struct Smack *p_smk, uint8_t track)
{
#if 1
    uint8_t ret;
    asm volatile (
      "call ASM_soundopen\n"
        : "=r" (ret) : "a" (p_smk), "d" (track));
    return ret;
#endif
}

int32_t gotoframe(struct Smack *p_smk)
{
#if 1
    int32_t ret;
    asm volatile (
      "call ASM_gotoframe\n"
        : "=r" (ret) : "a" (p_smk));
    return ret;
#endif
}

void setuptheframe(struct Smack *p_smk)
{
#if 1
    asm volatile (
      "call ASM_setuptheframe\n"
        :  : "a" (p_smk));
#endif
}

uint32_t SmackDoTables(uint8_t *p_inp, uint8_t *p_buf39c, int32_t codesz,
  int32_t absz, int32_t detailsz, int32_t typesz)
{
#if 1
    uint32_t ret;
    asm volatile (
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "push %3\n"
      "push %2\n"
      "push %1\n"
      "call ASM_SmackDoTables\n"
        : "=r" (ret) : "g" (p_inp), "g" (p_buf39c), "g" (codesz), "g" (absz), "g" (detailsz), "g" (typesz));
    return ret;
#endif
}

void * RADAPI RADMALLOC(uint32_t size)
{
    if (size == 0)
        return NULL;
    if (size == (uint32_t)-1)
        return NULL;
    return smack_malloc(size);
}

void RADAPI RADFREE(void *mem_ptr)
{
    smack_free(mem_ptr);
}

void *smkmalloc(struct Smack *p_smk, uint32_t size)
{
    uint32_t new_tot_mem;
    uint32_t prev_highest;

    new_tot_mem = size + p_smk->AllocdMemAmount;
    prev_highest = p_smk->HighestMemAmount;
    p_smk->AllocdMemAmount = new_tot_mem;
    if (new_tot_mem > prev_highest)
        p_smk->HighestMemAmount = new_tot_mem;
    return RADMALLOC(size);
}

void smkmfree(struct Smack *p_smk, void *ptr, uint32_t size)
{
    RADFREE(ptr);
    p_smk->AllocdMemAmount -= size;
}

struct Smack * RADAPI SMACKOPEN(uint32_t extrabuf, uint32_t flags, char *name)
{
    struct Smack *p_smk;

    assert(sizeof(struct Smack) == 0x4AC);
    assert(sizeof(struct SmackSndTrk) == 0x70);
#if 0
    asm volatile (
      "push %3\n"
      "push %2\n"
      "push %1\n"
      "call ASM_SMACKOPEN\n"
        : "=r" (p_smk) : "g" (extrabuf), "g" (flags), "g" (name));
    return p_smk;
#else
    uint32_t begin_time;
    uint32_t n_frames;
    uint32_t v59d;
    uint32_t v63;

    SmackTimerSetup();
    begin_time = SmackTimerRead();
    p_smk = (struct Smack *)RADMALLOC(sizeof(Smack));
    if (p_smk == NULL) {
        SmackTimerDone();
        return NULL;
    }

    memset(p_smk, 0, sizeof(struct Smack));
    p_smk->AllocdMemAmount += sizeof(Smack);

    if ((flags & 0x1000) != 0)
    {
        p_smk->FHandle = (int)name;
        p_smk->ReadError = AIL_lowfseekcur(p_smk->FHandle, 0);
    }
    else
    {
        p_smk->FHandle = AIL_lowfopen(name, __DS__, 0);
    }

    if (p_smk->FHandle == -1) {
        goto __fail;
    }
    if ((flags & 0x0800) != 0) {
        p_smk->SimSpeed = simspeed;
    }
    p_smk->Version = 0;
    blockread(p_smk, (uint8_t *)p_smk, 0x68);
    if (p_smk->Version != 0x324B4D53) { // "SMK2"
        goto __fail;
    }
    if ((flags & 0x0080) != 0) {
        p_smk->MSPerFrame = forcerate;
    }
    {
        int ms_per_frm;
        int ms100_per_frm;

        ms_per_frm = p_smk->MSPerFrame;
        if (ms_per_frm >= 0)
            ms100_per_frm = 100 * ms_per_frm;
        else
            ms100_per_frm = -ms_per_frm;
        p_smk->MS100PerFrame = ms100_per_frm;
    }

    if (p_smk->MS100PerFrame != 0)
        v63 = 100000 / p_smk->MS100PerFrame;
    else
        v63 = 100000;
    if (v63 == 0)
        v63 = 1;
    n_frames = p_smk->Frames + (p_smk->SmackerType & 1);
    p_smk->TrackTable = smkmalloc(p_smk, n_frames + 4 * n_frames + 4);
    if (p_smk->TrackTable == NULL) {
        goto __fail;
    }
    p_smk->field_3B8 = (uint32_t *)&p_smk->TrackTable[n_frames];
    blockread(p_smk, (uint8_t *)p_smk->field_3B8, 4 * n_frames);
    p_smk->ReadError += 4 * n_frames;
    blockread(p_smk, p_smk->TrackTable, n_frames);
    p_smk->ReadError += n_frames;
    {
        uint32_t unk_buf39c_size;
        unk_buf39c_size = p_smk->typesize + p_smk->detailsize + p_smk->absize + p_smk->codesize + SmackGetSizeTables();
        p_smk->UnkBuf39C = smkmalloc(p_smk, unk_buf39c_size);
        if (p_smk->UnkBuf39C == NULL)
        {
            RADFREE(p_smk->TrackTable);
            goto __fail;
        }
    }
    {
        uint8_t *input_buffer;
        uint32_t in_buf_sz;

        in_buf_sz = p_smk->tablesize + 4096;
        in_buf_sz = (in_buf_sz + 3) & ~0x03;
        if (in_buf_sz < 0x2000)
           in_buf_sz = 0x2000;
        input_buffer = smkmalloc(p_smk, in_buf_sz);
        if (input_buffer == NULL)
        {
            RADFREE(p_smk->UnkBuf39C);
            RADFREE(p_smk->TrackTable);
            goto __fail;
        }
        blockread(p_smk, input_buffer + 4096, p_smk->tablesize);
        SmackDoTables(input_buffer, p_smk->UnkBuf39C, p_smk->codesize,
           p_smk->absize, p_smk->detailsize, p_smk->typesize);
        smkmfree(p_smk, input_buffer, in_buf_sz);
    }

    uint32_t v3;
    uint32_t v29;
    uint32_t v31;

    int v27;
    int v28;
    uint32_t *v30;
    int v62;
    uint32_t largest_frame_sz;
    uint32_t extrabuf_real;

    largest_frame_sz = 0;
    v27 = p_smk->Frames - 1;
    v28 = p_smk->SmackerType & 1;
    p_smk->ReadError += p_smk->tablesize;
    p_smk->ReadError += 104;
    v3 = 0;
    v59d = v28 + v27;
    v29 = 0;
    v30 = p_smk->field_3B8;
    if (v59d >= v63)
    {
        uint32_t v33;
        uint32_t i;
        uint32_t *v35;
        int v36;
        uint32_t limit32;
        uint32_t *v64;
        v62 = 0;
        v64 = 0;
        v33 = 0;
        for (i = 0; i < v63; i++)
        {
            v33 += *v30;
            v30++;
        }
        v35 = p_smk->field_3B8;
        limit32 = v59d - v63;
        v31 = 0;
        v64 = &v35[v63];
        do
        {
            if ( v33 > v29 && v31 )
            {
              v29 = v33;
              v62 = v31;
            }
            v33 = *v64 + v33 - *v35;
            if (largest_frame_sz < *v35)
                largest_frame_sz = *v35;
            v3 += *v35;
            ++v31;
            ++v64;
            ++v35;
        }
        while ( v31 <= limit32 );

        for (; v31 <= v59d; v31++)
        {
            if (*v35 > largest_frame_sz)
                largest_frame_sz = *v35;
            v36 = *v35++;
            v3 += v36;
        }
    }
    else
    {
        int v36;
        v62 = 0;
        for (v31 = 0; v31 <= v59d; v31++)
        {
            v29 += *v30;
            if (*v30 > largest_frame_sz)
                largest_frame_sz = *v30;
            v36 = *v30++;
            v3 += v36;
        }
    }
    p_smk->field_4A8 = v62;
    p_smk->Highest1SecRate_2 = v29;
    p_smk->AverageFrameSize = v3 / v31;
    if ((flags & 0x200) != 0)
    {
        p_smk->IOBufferSize = v3;
    }
    else
    {
        if (extrabuf == (uint32_t)-1)
        {
            if (p_smk->MS100PerFrame)
                extrabuf = p_smk->Highest1SecRate_2;
            else
                extrabuf = 0x3000;
            if (extrabuf > v3 >> 2)
                extrabuf = v3 >> 2;
            if (extrabuf < 0x3000)
                extrabuf = 0x3000;
        }

        extrabuf_real = (extrabuf + 4095) >> 12 << 12;
        if (extrabuf_real < 0x2000)
            extrabuf_real = 0x2000;
        if (4 * v3 / 5 > extrabuf_real)
        {
            void *p_exbuf;

            p_smk->field_440 = extrabuf_real;
            p_smk->IOBufferSize = largest_frame_sz;
            p_exbuf = smkmalloc(p_smk, extrabuf_real + 4096);
            p_smk->field_42C = p_exbuf;
            p_smk->field_434 = p_exbuf;
            p_smk->field_438 = p_exbuf;
            if (p_exbuf == NULL) {
                RADFREE(p_smk->UnkBuf39C);
                RADFREE(p_smk->TrackTable);
                goto __fail;
            }
            p_smk->field_430 = p_exbuf + p_smk->field_440;
        }
        else
        {
            flags |= 0x0200;
            p_smk->IOBufferSize = v3;
        }
    }

    p_smk->IOBuffer = smkmalloc(p_smk, p_smk->IOBufferSize + 8);
    if (p_smk->IOBuffer == NULL)
    {
        if (p_smk->field_42C != NULL)
            RADFREE(p_smk->field_42C);
        RADFREE(p_smk->UnkBuf39C);
        RADFREE(p_smk->TrackTable);
        goto __fail;
    }
    int v46;
    v46 = p_smk->ReadError;
    p_smk->field_444 = v46;

    if ((flags & 0x0200) != 0)
    {
        blockread(p_smk, p_smk->IOBuffer, v3);
        if ((flags & 0x1000) == 0)
            AIL_lowfclose(p_smk->FHandle);
        p_smk->FHandle = -1;
        p_smk->field_3B0 = p_smk->IOBuffer;
    }
    else
    {
        int new_f43C;

        new_f43C = v46 & 0xFFF;
        if (new_f43C != 0)
        {
            size_t bytes_read;
            bytes_read = new_f43C;
            AIL_fread(p_smk->FHandle, p_smk->field_42C, __DS__, 4096 - new_f43C, &bytes_read);
            p_smk->field_438 = p_smk->field_42C + bytes_read;
            p_smk->field_43C = bytes_read;
        }
        if ((p_smk->OpenFlags & 0x0200) == 0)
        {
            p_smk->field_3B0 = p_smk->IOBuffer;
            blockread(p_smk, p_smk->IOBuffer, p_smk->field_3B8[p_smk->CurrFrameNum]);
        }
    }
    p_smk->field_3B4 = -1;
    setuptheframe(p_smk);
    p_smk->field_420 = -1;
    p_smk->OpenFlags = flags & ~0x3FE000;

    while ((flags & 0xFE000) != 0)
    {
        struct SmackSndTrk *p_sndtrk;
        int trk;
        trk = GetFirstTrack(&flags);
        p_sndtrk = (struct SmackSndTrk *)smkmalloc(p_smk, sizeof(struct SmackSndTrk));
        p_smk->Unkn404PerTrack[trk] = p_sndtrk;
        if (p_sndtrk != NULL)
        {
            if (soundopen(p_smk, trk)) {
                p_smk->OpenFlags |= 1 << (trk + 13);
            } else {
                p_smk->Unkn404PerTrack[trk] = NULL;
                smkmfree(p_smk, p_sndtrk, sizeof(struct SmackSndTrk));
            }
        }
    }
    p_smk->field_3FC = -1;
    if ((flags & 0x100) != 0)
    {
        while (backgroundload(p_smk))
            ;
    }

    switch (flags & 0x300000)
    {
    case 0x100000:
        p_smk->SmackerType = (p_smk->SmackerType & ~0x06) | 0x02;
        break;
    case 0x200000:
        p_smk->SmackerType = (p_smk->SmackerType & ~0x06) | 0x04;
        break;
    case 0x300000:
        p_smk->SmackerType &= ~0x06;
        break;
    }

    switch (p_smk->SmackerType & 6)
    {
    case 2:
        p_smk->OpenFlags |= 0x00100000;
        break;
    case 4:
        p_smk->OpenFlags |= 0x00200000;
        break;
    }
    if ((p_smk->OpenFlags & 0x300000) != 0) {
        p_smk->Height *= 2;
    }

    {
        uint32_t tot_bk_read_time, unk_read_time2;
        p_smk->TotalOpenTime = SmackTimerRead() - begin_time;
        tot_bk_read_time = p_smk->TotalBackReadTime;
        p_smk->TotalBackReadTime = 0;
        unk_read_time2 = p_smk->UnkReadTime2;
        p_smk->LastReadTime += tot_bk_read_time;
        p_smk->UnkReadTime1 += unk_read_time2;
        p_smk->FirstReadTime = p_smk->LastReadTime;
    }
    return p_smk;

__fail:
    if (((flags & 0x1000) == 0) && (p_smk->FHandle != -1))
        AIL_lowfclose(p_smk->FHandle);
    RADFREE(p_smk);
    SmackTimerDone();
    return NULL;
#endif
}

uint32_t RADAPI SMACKDOFRAME(struct Smack *p_smk)
{
    uint32_t ret;
    asm volatile (
      "push %1\n"
      "call ASM_SMACKDOFRAME\n"
        : "=r" (ret) : "g" (p_smk));
    return ret;
}

void RADAPI SMACKNEXTFRAME(struct Smack *p_smk)
{
    asm volatile (
      "push %0\n"
      "call ASM_SMACKNEXTFRAME\n"
        : : "g" (p_smk));
}

uint32_t RADAPI SMACKWAIT(struct Smack *p_smk)
{
    uint32_t ret;
    asm volatile (
      "push %1\n"
      "call ASM_SMACKWAIT\n"
        : "=r" (ret) : "g" (p_smk));
    return ret;
}

void RADAPI SMACKCLOSE(struct Smack *p_smk)
{
    asm volatile (
      "push %0\n"
      "call ASM_SMACKCLOSE\n"
        : : "g" (p_smk));
}

void RADAPI SMACKTOBUFFER(uint32_t Flags, const void *buf,
 uint32_t destheight, uint32_t Pitch, uint32_t top, uint32_t left, struct Smack *p_smk)
{
    asm volatile (
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "push %3\n"
      "push %2\n"
      "push %1\n"
      "push %0\n"
      "call ASM_SMACKTOBUFFER\n"
        : : "g" (Flags), "g" (buf), "g" (destheight), "g" (Pitch), "g" (top), "g" (left), "g" (p_smk));
}

/** Play SMK video file by decoding it directly to work screen buffer.
 *
 * Allows fast playback, without the need of additional copy, as long as pixel format
 * change or resize is not required. Can even decode into hardware frame buffer directly,
 * if only the platform allows such access (modern OSes do not).
 */
TbResult play_smk_direct(char *fname, u32 smkflags, ushort plyflags, ushort mode)
{
#if 0
    TbResult ret;
    asm volatile ("call ASM_play_smk_direct\n"
        : "=r" (ret) : "a" (fname), "d" (smkflags), "b" (plyflags), "c" (mode));
    return ret;
#else
    struct Smack *p_smk;
    int scr_x, scr_y;
    uint frm_no;
    TbBool finish, update_pal;
    uint32_t soflags;

    if ((plyflags & 0x0001) == 0)
    {
        struct DIG_DRIVER *p_snddrv;

        p_snddrv = GetSoundDriver();
        if (p_snddrv != NULL) {
            SMACKSOUNDUSEMSS(0, p_snddrv);
        } else {
            plyflags |= 0x0001;
        }
    }

    if ((plyflags & 0x0001) != 0)
        soflags = 0;
    else
        soflags = 0xFE000;
    p_smk = SMACKOPEN(0xFFFFFFFF, soflags, fname);
    if (p_smk == NULL) {
        return Lb_FAIL;
    }

    if ((plyflags & 0x0100) != 0)
    {
#if defined(DOS)||defined(GO32)
        scr_x = (320 - p_smk->Width) >> 1;
        scr_y = (200 - p_smk->Height) >> 1;
        SMACKTOBUFFER(0, 0xA0000, 200, 320, scr_y, scr_x, p_smk);
#else
        assert(!"hardware framebuffer access not supported");
#endif
    }
    else
    {
        scr_x = (lbDisplay.GraphicsScreenWidth - p_smk->Width) >> 1;
        scr_y = (lbDisplay.GraphicsScreenHeight - p_smk->Height) >> 1;
        SMACKTOBUFFER(0, lbDisplay.WScreen, lbDisplay.GraphicsScreenHeight,
          lbDisplay.GraphicsScreenWidth, scr_y,  scr_x, p_smk);
    }

    finish = false;
    for (frm_no = 0; !finish; frm_no++)
    {
        if ((plyflags & 0x0400) == 0)
        {
            if (frm_no >= p_smk->Frames)
                break;
        }
        if ((plyflags & 0x0100) != 0)
        {
            if (p_smk != (struct Smack *)-108)
            {
                LbMemoryCopy(byte_1E56DC, p_smk->Palette, PALETTE_8b_SIZE);
                LbPaletteSet(byte_1E56DC);
            }
            SMACKDOFRAME(p_smk);
        }
        else
        {
            update_pal = false;
            if (p_smk != (struct Smack *)-108)
            {
                update_pal = true;
                LbMemoryCopy(byte_1E56DC, p_smk->Palette, PALETTE_8b_SIZE);
            }
            SMACKDOFRAME(p_smk);
            if (update_pal)
            {
                LbScreenWaitVbi();
                LbPaletteSet(byte_1E56DC);
            }
            LbScreenSwap();
        }
        SMACKNEXTFRAME(p_smk);

        while (SMACKWAIT(p_smk))
        {
            game_hacky_update();
            if ((plyflags & 0x0002) != 0)
                continue;
            if (lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_RETURN] || lbKeyOn[KC_SPACE] || lbDisplay.MLeftButton)
            {
                finish = true;
            }
        }
    }
    SMACKCLOSE(p_smk);
    return Lb_SUCCESS;
#endif
}

// used only from old ASM - remove pending
void copy_to_screen_smk(uint8_t *p_buf, u32 width, u32 height, ushort plyflags)
{
#if 1
    asm volatile ("call ASM_copy_to_screen_smk\n"
        : : "a" (p_buf), "d" (width), "b" (height), "c" (plyflags));
#endif
}

void blit_to_screen_smk(struct SSurface *surf, u32 width, u32 height, ushort plyflags)
{
    struct TbRect srect;
    struct TbRect brect;
    u32 scr_w, scr_h;

    if ((plyflags & SMK_PixelDoubleWidth) == 0)
        scr_w = width;
    else
        scr_w = 2 * width;
    if ((plyflags & SMK_PixelDoubleLine) == 0)
        scr_h = height;
    else
        scr_h = 2 * height;
    LbSetRect(&srect, 0, 0, scr_w, scr_h);
    LbSetRect(&brect, 0, 0, width, height);
    LbScreenUnlock();
    if ((scr_w == width) && (scr_h == height))
        LbScreenSurfaceBlit(surf, 0, 0, &srect, SSBlt_TO_SCREEN);
    else
        LbScreenSurfaceBlitScaled(surf, &srect, &brect, SSBlt_TO_SCREEN);
    LbScreenLock();
    if (smack_draw_callback != NULL) {
        smack_draw_callback(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    }
    LbScreenSwap();
}

/** Play SMK video file by decoding it into temporary surface before blitting onto screen.
 *
 * Uses fast blitting methods, as the decode buffer is in a surface with hardware-backing capability
 * (whether it is actually hardware backed depends on platform). The blitting allows for resizing
 * or changing pixel format of the image. After bit but before displaying the new image, a callback
 * is executed allowing for applying effects or other modifications to the working screen buffer.
 */
TbResult play_smk_via_buffer(char *fname, u32 smkflags, ushort plyflags, SmackDrawCallback callback)
{
#if 0
    TbResult ret;
    asm volatile ("call ASM_play_smk_via_buffer\n"
        : "=r" (ret) : "a" (fname), "d" (smkflags), "b" (plyflags), "c" (callback));
    return ret;
#else
    struct SSurface surf;
    struct Smack *p_smk;
    uint8_t *frame_buf, *pv_frame_buf;
    uint frm_no;
    TbBool finish;
    uint32_t soflags;

    if ((plyflags & 0x0001) == 0)
    {
        struct DIG_DRIVER *p_snddrv;

        p_snddrv = GetSoundDriver();
        if (p_snddrv != NULL) {
            SMACKSOUNDUSEMSS(0, p_snddrv);
        } else {
            plyflags |= 0x0001;
        }
    }

    if ((plyflags & 0x0001) != 0)
        soflags = 0;
    else
        soflags = 0xFE000;
    p_smk = SMACKOPEN(0xFFFFFFFF, soflags, fname);
    if (p_smk == NULL) {
        return Lb_FAIL;
    }

    LbScreenSurfaceInit(&surf);
    if (LbScreenSurfaceCreate(&surf, p_smk->Width, p_smk->Height) == Lb_FAIL) {
        SMACKCLOSE(p_smk);
        return Lb_FAIL;
    }
    pv_frame_buf = NULL;
    finish = false;
    for (frm_no = 0; !finish; frm_no++)
    {
        if ((plyflags & 0x0400) == 0)
        {
            if (frm_no >= p_smk->Frames)
                break;
        }
        {
            if (p_smk != (struct Smack *)-108)
            {
                LbMemoryCopy(byte_1E56DC, p_smk->Palette, PALETTE_8b_SIZE);
            }
            frame_buf = LbScreenSurfaceLock(&surf);
            if (frame_buf == NULL)
                break;
            if (pv_frame_buf != frame_buf) {
                // If frame buffer address changed, update the reference in player
                SMACKTOBUFFER(0, frame_buf, p_smk->Height, p_smk->Width, 0, 0, p_smk);
                pv_frame_buf = frame_buf;
            }
            SMACKDOFRAME(p_smk);
            LbScreenSurfaceUnlock(&surf);
            {
                LbScreenWaitVbi();
                LbPaletteSet(byte_1E56DC);
            }
            blit_to_screen_smk(&surf, p_smk->Width, p_smk->Height, plyflags);
        }
        SMACKNEXTFRAME(p_smk);

        while (SMACKWAIT(p_smk))
        {
            game_hacky_update();
            if ((plyflags & 0x0002) != 0)
                continue;
            if (lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_RETURN] || lbKeyOn[KC_SPACE] || lbDisplay.MLeftButton)
            {
                finish = true;
            }
        }
    }
    SMACKCLOSE(p_smk);
    LbScreenSurfaceRelease(&surf);
    return Lb_SUCCESS;
#endif
}

TbResult play_smk(char *fname, u32 smkflags, ushort plyflags)
{
    TbResult ret;

    lbDisplay.MLeftButton = 0;
    if ( (smack_draw_callback != NULL) || ((plyflags & SMK_PixelDoubleWidth) != 0)
        /*|| ((plyflags & SMK_InterlaceLine) != 0)*/ || ((plyflags & SMK_PixelDoubleLine) != 0)
        /*|| (LbScreenIsDoubleBufferred())*/ ) {
        plyflags &= ~SMK_UnknFlag100;
        ret = play_smk_via_buffer(fname, smkflags, plyflags, smack_draw_callback);
    } else {
        if (lbDisplay.GraphicsScreenHeight >= 400)
          plyflags &= ~SMK_UnknFlag100;
        ret = play_smk_direct(fname, smkflags, plyflags, lbDisplay.ScreenMode);
    }
    return ret;
}

/******************************************************************************/
