/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bflib_vidraw.c
 *     Routines to be moved to bflibrary.
 * @par Purpose:
 *     Implement functions from bflibrary but not yet integrated there.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_vidraw.h"
#include "bftringl.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bfscreen.h"
#include "bfline.h"
#include "bfutility.h"

#include "engintrns.h"
/******************************************************************************/

ubyte byte_1DDC44[640];
ubyte byte_1DDEC4 = 0;
extern ubyte byte_1C4B7F;// = 0;
ubyte byte_1C6DED = 0;
extern ubyte byte_155189;// = 0xF7;

void draw_noise_box(short x, short y, ushort w, ushort h)
{
    uint dx, dy;
    ushort rnd;

    if (!byte_1DDEC4 || byte_1C6DED)
    {
        for (dx = 0; dx < sizeof(byte_1DDC44); dx++)
        {
            byte_1DDC44[dx] = 0;
            rnd = LbRandomAnyShort();
            if ((rnd % 9) > 4) {
                byte_1DDC44[dx] = byte_155189;
            }
        }
        byte_1C6DED = 0;
        byte_1DDEC4 = 1;
    }

    if (!byte_1C4B7F)
    {
        ubyte *out;
        ubyte *inp;

        for (dy = 1; dy < (uint)h - 1; dy++)
        {
            rnd = LbRandomAnyShort();
            out = &lbDisplay.WScreen[x + 1 + (y + dy) * lbDisplay.GraphicsScreenWidth];
            inp = &byte_1DDC44[rnd % (sizeof(byte_1DDC44) - w)];
            memcpy(out, inp, w - 2);
        }
    }
}

/******************************************************************************/
