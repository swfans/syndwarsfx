/******************************************************************************/
// Bullfrog Engine Smacker Playback Library - for use to remake classic games
// like Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file smack2ail.c
 *     Interface for bounding the Smacer library to AIL interface.
 * @par Purpose:
 *     Allows to easily play audio paths from SMK videos through AIL interface.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Nov 2011 - 13 Jun 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "smack2ail.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

#include "bftime.h"
#include "bfutility.h"
#include "bfwindows.h"
#include "aildebug.h"
#include "memfile.h"
/******************************************************************************/
extern void *SmackMSSDigDriver;
extern uint32_t MSSSpeed;
extern uint32_t MSSTimerPeriod;

extern uint32_t timeradjust;
extern uint32_t lasttimerread;
extern uint32_t mss_i_count;
extern uint32_t sndinit;
extern uint32_t msstimer;

uint32_t MSSLiteInit = 0;
SmackSndTrk *fss = NULL;

extern uint8_t RADAPI (*LowSoundOpenAddr)(uint8_t, SmackSndTrk *);
extern void RADAPI (*LowSoundCloseAddr)(SmackSndTrk *);
extern uint32_t RADAPI (*LowSoundPlayedAddr)(SmackSndTrk *);
extern void RADAPI (*LowSoundPurgeAddr)(SmackSndTrk *);
extern void RADAPI (*LowSoundOffAddr)(SmackSndTrk *);
extern void RADAPI (*SmackTimerSetupAddr)(void);
extern uint32_t RADAPI (*SmackTimerReadAddr)(void);
extern void RADAPI (*SmackTimerDoneAddr)(void);
extern void RADAPI (*LowSoundCheckAddr)(void);
extern void RADAPI (*LowSoundOnAddr)(void);
extern void RADAPI (*LowSoundVolPanAddr)(uint32_t, uint32_t, SmackSndTrk *);

/******************************************************************************/
void RADAPI MSSSMACKTIMERSETUP(void);


bool bothdone(struct SNDSAMPLE *p_smp)
{
    return p_smp->last_buffer >= 0 && p_smp->len[0] == p_smp->pos[0] && p_smp->len[1] == p_smp->pos[1];
}

uint32_t get_track_format(struct SmackSndTrk *p_sstrk)
{
    uint32_t format;
    if (p_sstrk->field_48)
    {
        if (p_sstrk->field_4C)
            format = DIG_F_STEREO_16;
        else
            format = DIG_F_MONO_16;
    }
    else
    {
        if (p_sstrk->field_4C)
            format = DIG_F_STEREO_8;
        else
            format = DIG_F_MONO_8;
    }
    return format;
}

void doinit(struct SmackSndTrk *p_sstrk)
{
    uint32_t fld48, format;

    AIL_init_sample(p_sstrk->smp);
    fld48 = p_sstrk->field_48;
    format = get_track_format(p_sstrk);
    AIL_set_sample_type(p_sstrk->smp, format, fld48 != 0);
    AIL_set_sample_playback_rate(p_sstrk->smp, p_sstrk->field_40);
    AIL_set_sample_user_data(p_sstrk->smp, 0, (intptr_t)p_sstrk);
    AIL_set_sample_volume(p_sstrk->smp, 127);
}

uint8_t *SMACKWRAPCOPY(uint32_t buf_len, uint8_t *pv_tail,
  uint8_t *nx_tail, uint8_t *pv_head, uint8_t *nx_head)
{
    uint8_t *cp_start;
    uint remain;

    cp_start = pv_head;
    remain = pv_tail - pv_head;
    if (remain <= buf_len)
    {
        if (remain > 0)
        {
            memcpy(nx_head, pv_head, remain);
            nx_head += remain;
            buf_len -= remain;
        }
        cp_start = nx_tail;
    }
    if (buf_len > 0)
    {
        memcpy(nx_head, cp_start, buf_len);
        cp_start += buf_len;
    }
    return cp_start;
}

uint8_t RADAPI MSSLOWSOUNDOPEN(uint8_t flags, SmackSndTrk *p_sstrk)
{
#if 0
    uint8_t ret;
    asm volatile (
      "push %2\n"
      "push %1\n"
      "call ASM_MSSLOWSOUNDOPEN\n"
        : "=r" (ret) : "g" (flags), "g" (p_sstrk));
    return ret;
#endif
    int format;
    uint32_t bsize;

    if ((SmackMSSDigDriver == NULL) && MSSLiteInit)
    {
        AIL_set_preference(DIG_USE_STEREO, ((flags & 0x20) != 0) || p_sstrk->field_4C);
        AIL_set_preference(DIG_USE_16_BITS, p_sstrk->field_48 != 0);

        if (SmackMSSDigDriver == NULL)
            SmackMSSDigDriver = AIL_install_DIG_driver_file("SB16.DIG", 0);
        if (SmackMSSDigDriver == NULL)
            SmackMSSDigDriver = AIL_install_DIG_driver_file("SBPRO.DIG", 0);
        if (SmackMSSDigDriver == NULL)
            SmackMSSDigDriver = AIL_install_DIG_driver_file("SBLASTER.DIG", 0);
    }
    if (SmackMSSDigDriver == NULL)
        SmackMSSDigDriver = (DIG_DRIVER *)-1;

    if (SmackMSSDigDriver == (DIG_DRIVER *)-1)
    {
        return false;
    }

    format = get_track_format(p_sstrk);
    bsize = AIL_minimum_sample_buffer_size(SmackMSSDigDriver, p_sstrk->field_40, format);
    p_sstrk->field_64 = (bsize + 3) & ~3;

    bsize = (p_sstrk->field_14 >> 2);
    p_sstrk->field_3C = max(p_sstrk->field_64, (bsize + 0x0FFF) & ~0x0FFF);

    p_sstrk->field_54[0] = RADMALLOC(2 * p_sstrk->field_3C);
    if (p_sstrk->field_54[0] == NULL)
    {
        return false;
    }
    AIL_vmm_lock(p_sstrk->field_54[0], 2 * p_sstrk->field_3C);
    p_sstrk->smp = AIL_allocate_sample_handle(SmackMSSDigDriver);
    if (p_sstrk->smp == NULL)
    {
        RADFREE(p_sstrk->field_54[0]);
        p_sstrk->field_54[0] = NULL;
        return false;
    }
    p_sstrk->field_54[1] = p_sstrk->field_54[0] + p_sstrk->field_3C;

    doinit(p_sstrk);
    {
        struct SmackSndTrk *p_nx_strk;
        p_nx_strk = fss;
        fss = p_sstrk;
        p_sstrk->next = p_nx_strk;
    }
    return 1;
}

void rm_smack_track(SmackSndTrk *p_sstrk)
{
    SmackSndTrk *cr_sstrk;
    SmackSndTrk *nx_sstrk;

    cr_sstrk = fss;
    while (cr_sstrk->next != NULL)
    {
        nx_sstrk = cr_sstrk->next;
        if (p_sstrk == nx_sstrk) {
            cr_sstrk->next = nx_sstrk->next;
            break;
        }
        cr_sstrk = cr_sstrk->next;
    }
}

void RADAPI MSSLOWSOUNDCLOSE(SmackSndTrk *p_sstrk)
{
#if 0
    asm volatile (
      "push %0\n"
      "call ASM_MSSLOWSOUNDCLOSE\n"
        :  : "g" (p_sstrk));
    return;
#endif
    if (fss == NULL) {
        return;
    }

    AIL_end_sample(p_sstrk->smp);
    AIL_release_sample_handle(p_sstrk->smp);
    AIL_vmm_unlock(p_sstrk->field_54, 2 * p_sstrk->field_3C);
    RADFREE(p_sstrk->field_54);

    if (p_sstrk == fss) {
        fss = fss->next;
        return;
    }
    rm_smack_track(p_sstrk);
}

void RADAPI MSSLOWSOUNDCHECK(void)
{
#if 0
    asm volatile (
      "call ASM_MSSLOWSOUNDCHECK\n"
        :  : );
    return;
#endif
    struct SmackSndTrk *p_sstrk;

    for (p_sstrk = fss; p_sstrk != NULL; p_sstrk = p_sstrk->next)
    {
        uint32_t mnlen;
        uint fld10;
        int bufno;

        fld10 = p_sstrk->field_10;
        if ((fld10 <= p_sstrk->field_64) && (!p_sstrk->field_44 || fld10 <= 3)) {
            continue;
        }
        bufno = AIL_sample_buffer_ready(p_sstrk->smp);
        if (bufno < 0) {
            continue;
        }
        mnlen = min(p_sstrk->field_10, p_sstrk->field_3C);
        mnlen &= ~3;

        p_sstrk->field_8 = SMACKWRAPCOPY(mnlen, p_sstrk->field_4, p_sstrk->field_0,
          p_sstrk->field_8, p_sstrk->field_54[bufno]);
        if ( !p_sstrk->field_6C || bothdone(p_sstrk->smp) || AIL_sample_status(p_sstrk->smp) == SNDSMP_DONE)
        {
            int32_t n;
            p_sstrk->field_6C = SmackTimerRead();
            n = p_sstrk->field_68;
            p_sstrk->field_68 = 0;
            p_sstrk->field_50 += n;
        }
        AIL_load_sample_buffer(p_sstrk->smp, bufno, p_sstrk->field_54[bufno], mnlen);
        p_sstrk->field_68 += mnlen;
        p_sstrk->field_10 -= mnlen;
    }
}

uint32_t RADAPI MSSLOWSOUNDPLAYED(SmackSndTrk *p_sstrk)
{
#if 0
    uint32_t ret;
    asm volatile (
      "push %1\n"
      "call ASM_MSSLOWSOUNDPLAYED\n"
        : "=r" (ret) : "g" (p_sstrk));
    return ret;
#endif
    uint32_t dt;

    MSSLOWSOUNDCHECK();
    dt = (SmackTimerRead() - p_sstrk->field_6C)
      * (uint64_t)p_sstrk->field_14 / 1000;
    if (dt > p_sstrk->field_68)
        dt = p_sstrk->field_68;
    return p_sstrk->field_50 + dt;
}

void RADAPI MSSLOWSOUNDPURGE(SmackSndTrk *p_sstrk)
{
#if 0
    asm volatile (
      "push %0\n"
      "call ASM_MSSLOWSOUNDPURGE\n"
        :  : "g" (p_sstrk));
    return;
#else
    AIL_end_sample(p_sstrk->smp);
    p_sstrk->field_50 = 0;
    p_sstrk->field_68 = 0;
    p_sstrk->field_6C = 0;
    doinit(p_sstrk);
#endif
}

/** Timer routine when linked to AIL; returns a timer value, in miliseconds.
 */
uint32_t RADAPI MSSSMACKTIMERREAD(void)
{
    return LbTimerClock();
}

void RADAPI MSSSMACKTIMERDONE(void)
{
#if 0
    asm volatile (
      "call ASM_MSSSMACKTIMERDONE\n"
        :  : );
    return;
#endif
    if (--sndinit != 0)
        return;
    AIL_release_timer_handle(msstimer);
    AIL_VMM_unlock_range(MSSSMACKTIMERREAD, MSSSMACKTIMERSETUP);
    AIL_vmm_unlock(&mss_i_count, sizeof(mss_i_count));
}

void mss_int(void *clientval)
{
    mss_i_count++;
}

void RADAPI MSSSMACKTIMERSETUP(void)
{
#if 0
    asm volatile (
      "call ASM_MSSSMACKTIMERSETUP\n"
        :  : );
    return;
#endif
    if (++sndinit != 1)
        return;
    AIL_VMM_lock_range(MSSSMACKTIMERREAD, MSSSMACKTIMERSETUP);
    AIL_vmm_lock(&mss_i_count, sizeof(mss_i_count));
    msstimer = AIL_register_timer(mss_int);
    AIL_set_timer_frequency(msstimer, MSSSpeed);
    AIL_start_timer(msstimer);
#if defined(DOS)||defined(GO32)
    uint16_t divsr;
    divsr = AIL_interrupt_divisor();
    // Configure 82C54 timer
    outb(0x43, 0x34);
    outb(0x40, (divsr) & 0xFF);
    outb(0x40, (divsr >> 8) & 0xFF);
#endif
    // Wait for the timers first tick
    uint32_t volatile prev_i_count;
    prev_i_count = mss_i_count;
    while (prev_i_count == mss_i_count) {
        LbWindowsControl();
    }
}

void RADAPI MSSLOWSOUNDVOLPAN(uint32_t pan, uint32_t volume, SmackSndTrk *sstrk)
{
#if 0
    asm volatile (
      "push %2\n"
      "push %1\n"
      "push %0\n"
      "call ASM_MSSLOWSOUNDVOLPAN\n"
        :  : "g" (pan), "g" (volume), "g" (sstrk));
    return;
#endif
    if (volume > 0x8000)
        volume = 0x8000;
    AIL_set_sample_volume(sstrk->smp, ((unsigned int)(1270 * (uint64_t)volume / 0x8000) + 5) / 10);
    AIL_set_sample_pan(sstrk->smp, ((unsigned int)(1270 * (uint64_t)pan / 0x10000) + 5) / 10);
}

/** Default timer routine; returns a timer value, in miliseconds.
 */
uint32_t RADAPI DEFSMACKTIMERREAD(void)
{
#if defined(DOS)||defined(GO32)
    // Every 82C54 timer tick (18.2 per second) activates INT 08h,
    // which increases this value by 1.
    uint32_t tick = PEEKL(0x46C);
    if (tick < lasttimerread)
        timeradjust += lasttimerread - tick;
    lasttimerread = tick;
    return 2746 * (uint64_t)(timeradjust + tick) / 50;
#else
    clock_t tick = clock();
    if ((uint32_t)tick < lasttimerread)
        timeradjust += lasttimerread - tick;
    lasttimerread = tick;
    return 1000 * (uint64_t)(timeradjust + tick) / CLOCKS_PER_SEC;
#endif
}

/** Default timer finish routine.
 */
void RADAPI DEFSMACKTIMERDONE(void)
{
}

uint32_t RADAPI ac_DEFSMACKTIMERREAD(void);
void RADAPI ac_DEFSMACKTIMERDONE(void);

/** Default timer initialization routine.
 */
void RADAPI DEFSMACKTIMERSETUP(void)
{
    SmackTimerReadAddr = ac_DEFSMACKTIMERREAD;
    SmackTimerDoneAddr = ac_DEFSMACKTIMERDONE;
#if defined(DOS)||defined(GO32)
    uint32_t tick = PEEKL(0x46C);
    timeradjust = -tick;
    // Setup 82C54 timer tick to count to 65536 (results in 18.2 IRQs per second)
    outb(0x43, 0x34);
    outb(0x40, 0);
    outb(0x40, 0);
#else
    clock_t tick = clock();
    timeradjust = -tick;
#endif
}

/* define asm-to-c functions to set as callbacks */
uint8_t RADAPI ac_MSSLOWSOUNDOPEN(uint8_t flags, SmackSndTrk *sstrk);
void RADAPI ac_MSSLOWSOUNDCLOSE(SmackSndTrk *sstrk);
uint32_t RADAPI ac_MSSLOWSOUNDPLAYED(SmackSndTrk *sstrk);
void RADAPI ac_MSSLOWSOUNDPURGE(SmackSndTrk *sstrk);
void RADAPI ac_MSSSMACKTIMERSETUP(void);
void RADAPI ac_MSSSMACKTIMERDONE(void);
void RADAPI ac_MSSLOWSOUNDCHECK(void);
void RADAPI ac_MSSLOWSOUNDVOLPAN(uint32_t pan, uint32_t volume, SmackSndTrk *sstrk);
uint32_t RADAPI ac_MSSSMACKTIMERREAD(void);


uint8_t RADAPI SMACKSOUNDUSEMSS(uint32_t speed, void *digdrv)
{
#if 0
    uint8_t ret;
    asm volatile (
      "push %2\n"
      "push %1\n"
      "call ASM_SMACKSOUNDUSEMSS\n"
        : "=r" (ret) : "g" (speed), "g" (digdrv));
    return ret;
#endif
    if (SmackTimerReadAddr != NULL)
        return 0;
    SmackMSSDigDriver = digdrv;
    if (speed < 200)
        speed = 200;
    MSSSpeed = speed;
    LowSoundOpenAddr = ac_MSSLOWSOUNDOPEN;
    LowSoundCloseAddr = ac_MSSLOWSOUNDCLOSE;
    LowSoundPlayedAddr = ac_MSSLOWSOUNDPLAYED;
    LowSoundPurgeAddr = ac_MSSLOWSOUNDPURGE;
    LowSoundOffAddr = ac_MSSLOWSOUNDPURGE;
    SmackTimerSetupAddr = ac_MSSSMACKTIMERSETUP;
    SmackTimerReadAddr = ac_MSSSMACKTIMERREAD;
    SmackTimerDoneAddr = ac_MSSSMACKTIMERDONE;
    MSSTimerPeriod = 1193181 / speed;
    LowSoundCheckAddr = ac_MSSLOWSOUNDCHECK;
    LowSoundOnAddr = NULL;
    LowSoundVolPanAddr = ac_MSSLOWSOUNDVOLPAN;
    return 1;
}

void RADAPI SmackTimerSetup(void)
{
    //SmackTimerSetupAddr(); -- incompatible calling convention
    asm volatile ("call *%0\n"
      :  : "g" (SmackTimerSetupAddr) : "eax" );
}

uint32_t RADAPI SmackTimerRead(void)
{
    uint32_t ret;
    //return SmackTimerReadAddr(); -- incompatible calling convention
    asm volatile ("call *%1\n"
      : "=r" (ret) : "g" (SmackTimerReadAddr));
    return ret;
}

void RADAPI SmackTimerDone(void)
{
    //SmackTimerDoneAddr(); -- incompatible calling convention
    asm volatile ("call *%0\n"
      : : "g" (SmackTimerDoneAddr) : "eax" );
}

/******************************************************************************/
