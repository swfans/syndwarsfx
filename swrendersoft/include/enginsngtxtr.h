/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginsngtxtr.h
 *     Header file for enginsngtxtr.c.
 * @par Purpose:
 *     Engine single objects required by the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     29 Sep 2023 - 02 Jan 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINSNGTXTR_H
#define ENGINSNGTXTR_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct SingleFloorTexture { // sizeof=18
    ubyte TMapX1;
    ubyte TMapY1;
    ubyte TMapX2;
    ubyte TMapY2;
    ubyte TMapX3;
    ubyte TMapY3;
    ubyte TMapX4;
    ubyte TMapY4;
    ubyte Page;
    ubyte field_9[3];
    ubyte field_C[5];
    ubyte field_11;
};

struct SingleTexture { // sizeof=16
    ubyte TMapX1;
    ubyte TMapY1;
    ubyte TMapX2;
    ubyte TMapY2;
    ubyte TMapX3;
    ubyte TMapY3;
    ubyte Page;
    ubyte padding1;
    short pal;
    ubyte field_A[6];
};

struct AnimTmap { // sizeof=54
    ushort Texture;
    ushort TMap[10];
    ubyte Delay[10];
    ushort Flags;
    ubyte field_22;
    ubyte field_23;
    ushort field_24;
    ubyte field_26[15];
    ubyte field_35;
};

#pragma pack()
/******************************************************************************/
extern struct SingleFloorTexture *game_textures;
extern struct SingleTexture *game_face_textures;
extern struct AnimTmap *game_anim_tmaps;

extern ushort next_face_texture;
extern ushort next_floor_texture;
extern ushort next_anim_tmap;

extern ushort tnext_floor_texture;

ushort floor_texture_index(struct SingleFloorTexture *p_sftex);

/** Maps content of old SingleFloorTexture struct to the current one.
 */
void refresh_old_floor_texture_format(struct SingleFloorTexture *p_fltextr,
  struct SingleFloorTexture *p_oldfltextr, u32 fmtver);

TbResult read_textwalk(void);
ubyte get_my_texture_bits(short tex);

void animate_textures(void);

/** Maps content of old SingleTexture struct to the current one.
 */
void refresh_old_face_texture_format(struct SingleTexture *p_fctextr,
  struct SingleTexture *p_oldfctextr, u32 fmtver);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
