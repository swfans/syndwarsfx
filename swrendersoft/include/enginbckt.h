/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginbckt.h
 *     Header file for enginbckt.c.
 * @par Purpose:
 *     Drawlist buckets handling for the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINBCKT_H
#define ENGINBCKT_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Amount of buckets for draw list elements.
 *
 * The buckets are a way of sorting draw items according to depth - like
 * a simplified replacement for the depth buffer.
 */
#define BUCKETS_COUNT 24000

#define BUCKET_MID (BUCKETS_COUNT / 2)

/** Max amount of draw list elements within a bucket.
 */
#define BUCKET_ITEMS_MAX 2000

#pragma pack(1)

enum DrawItemType {
    DrIT_NONE = 0x0,
    DrIT_Unkn1 = 0x1,
    DrIT_Unkn2 = 0x2,
    DrIT_Unkn3 = 0x3,
    DrIT_Unkn4 = 0x4,
    DrIT_Unkn5 = 0x5,
    DrIT_Unkn6 = 0x6,
    DrIT_Unkn7 = 0x7,
    DrIT_Unkn8 = 0x8,
    DrIT_Unkn9 = 0x9,
    DrIT_Unkn10 = 0xA,
    DrIT_Unkn11 = 0xB,
    DrIT_Unkn12 = 0xC,
    DrIT_SprPersE = 0xD,
    DrIT_Unkn14 = 0xE,
    DrIT_Unkn15 = 0xF,
    DrIT_Unkn16 = 0x10,
    DrIT_Unkn17 = 0x11,
    DrIT_Unkn18 = 0x12,
    DrIT_Unkn19 = 0x13,
    DrIT_Unkn20 = 0x14,
    DrIT_Unkn21 = 0x15,
    DrIT_LongPropBar,
    DrIT_Unkn23 = 0x17,
    DrIT_Unkn24 = 0x18,
    DrIT_Unkn25 = 0x19,
    DrIT_Number,
    DrIT_ShortText,
};

#pragma pack()
/******************************************************************************/

TbBool draw_item_add(ubyte ditype, ushort offset, int bckt);

void draw_drawlist_1(void);
void draw_drawlist_2(void);
void reset_drawlist(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
