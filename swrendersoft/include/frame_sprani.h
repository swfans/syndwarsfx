/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file frame_sprani.h
 *     Header file for frame_sprani.c.
 * @par Purpose:
 *     List of sprite animation frames used by the game.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 22 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef FRAME_SPRANI_H
#define FRAME_SPRANI_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#include "bftypes.h"

/******************************************************************************/
#pragma pack(1)

struct TbSprite;

struct Frame {
    /** Index of the first of elements used to draw this frame. */
    ushort FirstElement;
    ubyte SWidth;
    ubyte SHeight;
    ubyte FX;
    ubyte Flags;
    /** Index of the next frame in the animation. */
    ushort Next;
};

struct Element { // sizeof=0x0A
    /** Memory shift of the sprite.
     * Note that this is not an index in sprites array, but a byte shift.
     */
    ushort ToSprite;
    /** Position to draw the sprite within a frame, X coord. */
    short X;
    /** Position to draw the sprite within a frame, Y coord. */
    short Y;
    /** Flags for selection of draw effects of the sprite. */
    ushort Flags;
    /** Next element making up this chain of elements. */
    ushort Next;
};

#pragma pack()
/******************************************************************************/
/** Array of animation starts.
 *
 * This array provides a constant index for animations.
 * It stores indexes of frames to be used for initiating an animation.
 */
extern ushort *nstart_ani;
extern ushort *nstart_ani_end;

/** Array of animation frames.
 *
 * Stores definitions of each frame of animation, each frame linking to the next.
 */
extern struct Frame *frame;
extern struct Frame *frame_end;

/** Array of elements of frames.
 *
 * A frame consists of one or more elements, each representing drawing a sprite
 * at selected position and with selected draw flags.
 */
extern struct Element *melement_ani;
extern struct Element *mele_ani_end;

/** Array of in-mission sprites, used within elements or drawn independently.
 */
extern struct TbSprite *m_sprites;
extern struct TbSprite *m_sprites_end;

extern ushort shield_frm[4];


void shield_frames_init(void);
void shield_frames_cycle(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
