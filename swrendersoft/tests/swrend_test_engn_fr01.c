/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file swrend_test_engn_fr01.c
 *     Test application rendering an example frame in SW 3D engine.
 * @par Purpose:
 *     Testing implementation of swrender routines.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Jan 2025 - 04 Mar 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <stdlib.h>
#include "bfmemory.h"
#include "bfwindows.h"
#include "swrtstlog.h"

/******************************************************************************/

/** Test memory module setup and allocation.
 */
TbBool test_engn_fr01(void)
{
    void *p;
    //TODO implement

    if (LbBaseInitialise() != Lb_SUCCESS) {
        LOGERR("bullfrog library initialization failed");
        return false;
    }
    LbMemorySetup();

    p = LbMemoryAlloc(2048);
    LbMemoryFree(p);

    LbMemoryReset();
    LOGSYNC("passed");
    return true;
}

int main(int argc, char *argv[])
{
    if (!test_engn_fr01())
        exit(52);
    exit(0);
}

/******************************************************************************/
