/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engincolour.h
 *     Header file for engincolour.c.
 * @par Purpose:
 *     Colours used by the engine, palette and selected named colours.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     13 Oct 2024 - 20 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINCOLOUR_H
#define ENGINCOLOUR_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum ColourLookUp {
    ColLU_BLACK     = 0,
    ColLU_WHITE,
    ColLU_RED,
    ColLU_GREEN,
    ColLU_BLUE,
    ColLU_YELLOW,
    ColLU_CYAN,
    ColLU_PINK,
    ColLU_GREYLT,//looks more like another white
    ColLU_GREYMD,
    ColLU_GREYDK,
    ColLU_BLINK_YLW_BLK,
    ColLU_BLINK_RED_WHT,
};


#pragma pack()
/******************************************************************************/
extern ubyte *display_palette;

extern TbPixel fade_unaffected_colours[];
extern TbPixel colour_lookup[];
extern u32 unkn_changing_color_counter1;
extern int colour_lookup_lock;

extern TbPixel colour_mix_lookup[];
extern TbPixel colour_sel_grey[];
extern TbPixel colour_sel_green[];
extern TbPixel colour_sel_blue[];
extern TbPixel colour_sel_red[];
extern TbPixel colour_sel_purple[];

extern TbPixel colour_grey1;
extern TbPixel colour_grey2;
extern TbPixel colour_brown2;

/** Momentary in-game brightess; base from user settings, but with automatic adjustments. */
extern short momentary_brightness;
/******************************************************************************/

void update_unkn_changing_colors(void);
void setup_color_lookups(void);

/** Change the palette brightness, from normal to given amount.
 *
 * Shifts brightness of the display palette. Requires the current
 * palette to store normal brightness values, not altered by any
 * previous shift.
 */
void change_brightness_from_normal(short amount);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
