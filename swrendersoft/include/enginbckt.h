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
    DrIT_ObFace3Txtr = 0x1,
    DrIT_Unkn2 = 0x2,
    /** Frame of sprite elements, representing a static thing. */
    DrIT_SFrmStatc = 0x3,
    DrIT_Unkn4 = 0x4,
    DrIT_Unkn5 = 0x5,
    DrIT_Unkn6 = 0x6,
    DrIT_Unkn7 = 0x7,
    DrIT_Unkn8 = 0x8,
    DrIT_ObFace4Txtr = 0x9,
    DrIT_Unkn10 = 0xA,
    DrIT_Unkn11 = 0xB,
    DrIT_SpObFace4 = 0xC,
    /** Frame of sprite elements with version selections, representing a person thing. */
    DrIT_SFrmPersV = 0xD,
    /** Pole stored as quadrangular face, with only two points storing real points. */
    DrIT_ObFacePole,
    DrIT_Unkn15 = 0xF,
    DrIT_Unkn16 = 0x10,
    /** Reflective triangular face, from the object faces array. */
    DrIT_ObFace3Refl,
    /** Reflective quadrangular face, from the object faces array. */
    DrIT_ObFace4Refl,
    /** Sprite-based shadow of a person. */
    DrIT_SPersShdw,
    /** Triangular face from dedicated shrapnel array. */
    DrIT_SharpnlPoly,
    /** Frame of sprite elements from dedicated phwoar array. */
    DrIT_SFrmPhwoar,
    DrIT_LongPropBar,
    /** Semi-transparent quadrangular face, tinted with predefined colour. Used for deep radar. */
    DrIT_ObFace4Tran,
    /** Semi-transparent triangular face, tinted with predefined colour. Used for deep radar. */
    DrIT_ObFace3Tran,
    /** Flame sprite from dedicated fire array. */
    DrIT_SFireFlame,
    /** Numeric value written is simple (non-sprite) font. */
    DrIT_Number,
    /** Short text message (without extra buffer) written is simple (non-sprite) font. */
    DrIT_ShortText,
    /** Frame of sprite elements with basic properties, representing a person thing. */
    DrIT_SFrmPersB,
    /** Frame of sprite elements with version selections, representing an effect or overlay for a thing. */
    DrIT_SFrmEfctV,
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
