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

void scene_post_effect_prepare(void);
void scene_post_effect_for_bucket(short bckt);

void scene_post_effect_rain_init(void);

void draw_background_stars(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
