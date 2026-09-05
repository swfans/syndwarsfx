/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginpeff.h
 *     Header file for enginpeff.c.
 * @par Purpose:
 *     Engine scene pre-/post-processing effects.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     29 Oct 2023 - 02 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINPEFF_H
#define ENGINPEFF_H

#include "bftypes.h"

#include "enginprops.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum GamePostSceneEffectType {
    ScEff_NONE = 0,
    ScEff_RAIN,
    ScEff_SNOW,
    ScEff_SPACE,
};

#pragma pack()
/******************************************************************************/
extern ushort gamep_scene_effect_type;
extern ushort gamep_scene_effect_intensity;

/** Widest a rain drop may be drawn, in pixels; 0 means no limit.
 *
 * The drop is sized from the screen height, so the one pixel streak of the
 * original display becomes a wide bar on a modern one. Only the width is
 * capped; the length still follows the resolution.
 */
extern ushort rain_drop_max_width;

/** Largest a snow flake may be drawn, in pixels; 0 means no limit.
 *
 * The flake is a square sized the same way as the rain drop above.
 */
extern ushort snow_flake_max_size;

/** Largest a background star may be drawn, in pixels; 0 means no limit. */
extern ushort star_max_size;

/* Wavy float table with even number of elements.
 *
 * The period of this table is 31, last element repeats first.
 */
extern const short waft_table2[32];

/* Wavy float table with odd number of elements.
 *
 * The period of this table is 32, last element repeats first.
 */
extern const short waft_table[33];

/** Value of waft_table between the given animation turn and the next one.
 *
 * Everything which bobs on a wobbly terrain map moves by one table entry per
 * animation turn. When several frames are drawn within a turn, this places it
 * where it should be for the frame instead of leaving it still until the turn
 * ends. With one frame per turn the value is the one the table holds.
 *
 * Inline on purpose: its callers sit next to inline assembly, and a call
 * there would move registers around for no gain.
 */
static inline int waft_between_turns(uint anim_turn)
{
    int v0, v1;

    v0 = waft_table[anim_turn & 0x1F];
    v1 = waft_table[(anim_turn + 1) & 0x1F];
    return v0 + ((v1 - v0) * (int)render_anim_subturn) / 256;
}

void scene_post_effect_prepare(void);
void scene_post_effect_for_bucket(short bckt);

void scene_post_effect_rain_init(void);

void draw_background_stars(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
