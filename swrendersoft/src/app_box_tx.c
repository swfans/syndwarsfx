/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file app_box_tx.c
 *     Implementation of related functions.
 * @par Purpose:
 *     App-specific box drawing, for textured box slanted at 45 degrees.
 * @par Comment:
 *     This is a modification of `gbox.c` from bflibrary.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 02 Oct 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "app_box.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "bfscreen.h"

#include "enginpeff.h"
#include "enginprops.h"
#include "engintrns.h"
#include "engintxtrmap.h"
#include "render_gpoly.h"

/******************************************************************************/

TbResult AppDrawTexturedSlantBoxClip(s32 destX, s32 destY, s32 width, s32 height,
  ubyte vecMode, ubyte brig, ubyte tmapNo, ushort nUa, ushort nVb,
  ushort shiftUa, ushort shiftVb, ushort maxLenUa, ushort maxLenVb)
{
    struct EnginePoint point4;
    struct EnginePoint point2;
    struct EnginePoint point1;
    struct EnginePoint point3;
    ushort startUa, startVb;
    u32 lenUa, lenVb;
    short sh_x;

    // Slant amount
    sh_x = 3;
    if (lbDisplay.GraphicsScreenHeight < 400)
        sh_x /= 2;

    point1.pp.X = destX;
    point1.pp.Y = destY;
    point4.pp.X = (destX + width);
    point4.pp.Y = destY;
    point2.pp.Y = (destY + height);
    point2.pp.X = (destX + width - sh_x);
    point3.pp.Y = (destY + height);
    point3.pp.X = (destX - sh_x);

    startUa = SINGLE_TEXTURE_DIM * nUa + shiftUa;
    startVb = SINGLE_TEXTURE_DIM * nVb + shiftVb;

    if (width >= height)
    {
        lenUa = maxLenUa;
        lenVb = maxLenUa * height / width;
        if (lenVb > maxLenVb) {
            lenVb = maxLenVb;
            lenUa = maxLenVb * width / height;
        }
    }
    else
    {
        lenVb = maxLenVb;
        lenUa = maxLenVb * width / height;
        if (lenUa > maxLenUa) {
            lenUa = maxLenUa;
            lenVb = maxLenUa * height / width;
        }
    }

    point1.pp.U = (startUa) << 16;
    point1.pp.V = (startVb) << 16;

    point2.pp.U = (startUa + lenUa) << 16;
    point2.pp.V = (startVb + lenVb) << 16;

    point3.pp.U = (startUa) << 16;
    point3.pp.V = (startVb + lenVb) << 16;

    point4.pp.U = (startUa + lenUa) << 16;
    point4.pp.V = (startVb) << 16;

    point1.pp.S = brig << 16;
    point2.pp.S = brig << 16;
    point3.pp.S = brig << 16;
    point4.pp.S = brig << 16;

    vec_mode = vecMode;
    assert(vec_tmap[tmapNo] != NULL);
    vec_map = vec_tmap[tmapNo];

    draw_trigpoly(&point1.pp, &point4.pp, &point3.pp);
    draw_trigpoly(&point4.pp, &point2.pp, &point3.pp);
    return Lb_SUCCESS;
}

TbResult AppDrawTexturedFlowSlantBox(s32 X, s32 Y, s32 Width, s32 Height,
  ubyte vecMode, ubyte brig, ubyte tmapNo, ushort nUa, ushort nVb)
{
    TbResult ret;

    u32 waftx, wafty;
    uint anim_speed_x, anim_speed_y;
    ushort shiftUa, shiftVb;
    // The box texture is animated, even if it's not always possible to see
    anim_speed_x = (render_anim_turn >> (RENDER_ANIM_TURN_SHIFT + 3));
    anim_speed_y = (render_anim_turn >> RENDER_ANIM_TURN_SHIFT);
    waftx = waft_table[(anim_speed_x) & 0x1F];
    wafty = waft_table[(anim_speed_y + 16) & 0x1F];
    // The table returns values -28..28
    // aim for shiftUa 0 .. (SINGLE_TEXTURE_DIM)-1
    shiftUa = ((waftx + 30) >> 1);
    // aim for shiftVb 0 .. (SINGLE_TEXTURE_DIM/4)-1
    shiftVb = ((wafty + 30) >> 3);

    ret = AppDrawTexturedSlantBoxClip(X, Y, Width, Height, vecMode,
        brig, tmapNo, nUa, nVb, shiftUa, shiftVb,
        SINGLE_TEXTURE_DIM * 2, SINGLE_TEXTURE_DIM * 3 / 4);
    return ret;
}

/******************************************************************************/
