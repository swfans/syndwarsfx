/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file embedanim.h
 *     Header file for embedanim.c.
 * @par Purpose:
 *     Embedded animations, for use in both GUI and in-game.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Jan 2023 - 03 Aug 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef EMBEDANIM_H
#define EMBEDANIM_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum AnimSlot {
  AniSl_FULLSCREEN = 0,
  AniSl_BILLBOARD = 1,
  AniSl_EQVIEW = 2,	/**< equipment (weapon or mod) presentation in buy/sell window */
  AniSl_CYBORG_INOUT = 3,	/**< cyborg mod insertion or removal anim */
  AniSl_UNKN4 = 4,
  AniSl_UNKN5 = 5,
  AniSl_UNKN6 = 6,
  AniSl_UNKN7 = 7,
  AniSl_CYBORG_BRTH = 8,
  AniSl_NETSCAN = 9,
  AniSl_SCRATCH = 10,	/**< scratch buffer for some transparent menu animations */
};

/******************************************************************************/
uint embanim_current_frame_number(ubyte anislot);

/** Decode and draw next frame of the animation.
 */
int embanim_do_next_frame(ubyte anislot);

/** Decode and draw previous frame of the animation.
 *
 * Note that printing a previous frame of the FLI file requires
 * decoding all frames from start - these files do not use
 * bi-directional FLIC format.
 */
int embanim_do_prev_frame(ubyte anislot);

/** Retrieve pointer to output buffer of the animation at given slot.
 */
TbPixel *embanim_type_get_output_buffer(ubyte anislot);

/** Clear output buffer of the animation at given slot.
 *
 * The animation must be opened, but its frame buffer
 * doesn't have to be set for this function to work.
 */
void embanim_clear_output_buffer(ubyte anislot);

/** Initialize the slots to unused, discarding existing values.
 *
 * To be used once, during application setup; later, use reinit.
 */
void embanim_init(void);

/** Finalize previous playback and initialize new animation at given slot.
 */
void embanim_reinit(ubyte anislot);

void embanim_set_netscan_file(ubyte anislot, ubyte netno);
void embanim_set_weapon_model_file(ubyte anislot, ubyte wtype);
void embanim_set_cybmod_model_file(ubyte anislot, ubyte mtype);
void embanim_set_cyborg_part_file(ubyte anislot, ubyte part, ubyte stage);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
