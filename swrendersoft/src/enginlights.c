/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginlights.c
 *     Lights structures required by the 3D engine.
 * @par Purpose:
 *     Implement functions for handling lights in 3D world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "enginlights.h"

#include "bfmemut.h"

#include "privrdlog.h"
/******************************************************************************/

ushort next_quick_light = 1;
ushort next_full_light = 1;
ushort next_light_command = 1;

void refresh_old_full_light_format(struct FullLight *p_fulight,
  struct FullLightOldV12 *p_oldfulight, u32 fmtver)
{
    LbMemorySet(p_fulight, 0, sizeof(struct FullLight));

    p_fulight->Intensity = p_oldfulight->Intensity;
    p_fulight->TrueIntensity = p_oldfulight->TrueIntensity;
    p_fulight->Command = p_oldfulight->Command;
    p_fulight->NextFull = p_oldfulight->NextFull;
    p_fulight->X = p_oldfulight->X;
    p_fulight->Y = p_oldfulight->Y;
    p_fulight->Z = p_oldfulight->Z;
    p_fulight->lgtfld_E = p_oldfulight->lgtfld_E;
    p_fulight->lgtfld_10 = p_oldfulight->lgtfld_10;
    p_fulight->lgtfld_12 = p_oldfulight->lgtfld_12;

    if (fmtver <= 11) {
        p_fulight->TrueIntensity = p_fulight->Intensity;
    }
}

/******************************************************************************/
