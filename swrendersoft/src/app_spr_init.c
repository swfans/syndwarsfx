/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file app_spr_init.c
 *     Init functions for sprites drawing, per-application modification for panel.
 * @par Purpose:
 *     Setup of arrays used for sprite drawing.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2024 - 21 Jan 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "app_sprite.h"

#include <stdlib.h>
#include <assert.h>
#include "bfgentab.h"
#include "bfsprite.h"
#include "insspr.h"
#include "bfscreen.h"

#include "engincolour.h"
/******************************************************************************/

void LowTransGrey_InitPaletteBright(void)
{
    ubyte *pal;
    int col;
    ubyte col_r, col_g, col_b;
    ubyte col_C;

    pal = display_palette;
    for (col = 0; col < PALETTE_8b_COLORS; col++)
    {
        col_r = pal[col * 3 + 0];
        col_g = pal[col * 3 + 1];
        col_b = pal[col * 3 + 2];

        if ((col_r > col_g) && (col_r > col_b))
            col_C = col_r;
        else if (col_g > col_b)
            col_C = col_g;
        else
            col_C = col_b;

        low_trans_grey_pal_bright[col] = (col_C + ((col_b + col_r + col_g) >> 1)) / 3;
    }
}

void LowTransGrey_InitBrightLimitTable(void)
{
    int i;

    for (i = 0; i < 64; i++)
        low_trans_grey_bright_limit[i] = i;
    for (; i < 256; i++)
        low_trans_grey_bright_limit[i] = 63;
}

/******************************************************************************/
