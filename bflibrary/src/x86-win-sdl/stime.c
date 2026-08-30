/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file stime.cpp
 *     Implementation of related functions.
 * @par Purpose:
 *     Unknown.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Nov 2008 - 05 Nov 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <time.h>
#include <SDL.h>
#include "bftime.h"

#include "bfwindows.h"

TbResult LbDateTimeDecode(const time_t *datetime, struct TbDate *curr_date,
  struct TbTime *curr_time);

TbResult LbTime(struct TbTime *curr_time)
{
    time_t dtime;

    time(&dtime);
    return LbDateTimeDecode(&dtime, NULL, curr_time);
}

TbTimeSec LbTimeSec(void)
{
    time_t dtime;

    time(&dtime);
    return dtime;
}

TbResult LbDate(struct TbDate *curr_date)
{
    time_t dtime;

    time(&dtime);
    return LbDateTimeDecode(&dtime, curr_date, NULL);
}

TbResult LbDateTime(struct TbDate *curr_date, struct TbTime *curr_time)
{
    time_t dtime;

    time(&dtime);
    return LbDateTimeDecode(&dtime,curr_date,curr_time);
}

TbClockMSec LbTimerClock(void)
{
#ifndef __unix__
    // Unfortuately, CLOCKS_PER_SEC cannot be safely used in preprocessor directives
    if (CLOCKS_PER_SEC >= 10000)
        return clock() / (CLOCKS_PER_SEC / 1000);
    else if (CLOCKS_PER_SEC > 1000)
        return ((TbClockMSec)clock() << 3) / (CLOCKS_PER_SEC / 125);
    else if (CLOCKS_PER_SEC == 1000)
        return clock();
    else if (CLOCKS_PER_SEC >= 500)
        return ((TbClockMSec)clock() * (8000 / CLOCKS_PER_SEC)) >> 3;
    else
        return (TbClockMSec)clock() * (1000 / CLOCKS_PER_SEC);
#else
    static struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1E6);
#endif
}

/** @internal
 * Sleeps for the given amount of milliseconds.
 */
static void LbI_SleepShort(TbClockMSec delay)
{
    if (delay > 0)
        SDL_Delay(delay);
}

TbBool LbSleepUntil(TbClockMSec endtime)
{
    TbClockMSec currclk;
    currclk = LbTimerClock();
    while ((currclk+LB_LARGE_DELAY_TIME) < endtime)
    {
        LbDoMultitasking();
        currclk = LbTimerClock();
    }
    if (currclk < endtime)
        LbI_SleepShort(endtime - currclk);
    return true;
}

TbResult LbDateTimeDecode(const time_t *datetime, struct TbDate *curr_date,
  struct TbTime *curr_time)
{
    struct tm *ltime;

    ltime = localtime(datetime);
    if (curr_date != NULL)
    {
        curr_date->Day = ltime->tm_mday;
        curr_date->Month = ltime->tm_mon + 1;
        curr_date->Year = 1900 + ltime->tm_year;
        curr_date->DayOfWeek = ltime->tm_wday;
    }
    if (curr_time != NULL)
    {
        curr_time->Hour = ltime->tm_hour;
        curr_time->Minute = ltime->tm_min;
        curr_time->Second = ltime->tm_sec;
        curr_time->HSecond = 0;
    }
    return Lb_SUCCESS;
}

/******************************************************************************/
