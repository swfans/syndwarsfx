/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engincolour.c
 *     Colours used by the engine, palette and selected named colours.
 * @par Purpose:
 *     Implement functions for handling map floor tiles.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     13 Oct 2024 - 06 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engincolour.h"

#include "bfpalette.h"

#include "enginzoom.h"
#include "privrdlog.h"
/******************************************************************************/

TbPixel fade_unaffected_colours[] = {
  1,2,3,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  224,225,226,227,228,229,230,231,
  240,241,242,243,244,245,246,
  254,255,
  0};

/******************************************************************************/

void update_unkn_changing_colors(void)
{
    ubyte col1, col2;

    unkn_changing_color_counter1++;

    if (unkn_changing_color_counter1 & 0x01)
        col1 = colour_lookup[ColLU_YELLOW];
    else
        col1 = colour_lookup[ColLU_BLACK];
    colour_lookup[ColLU_BLINK_YLW_BLK] = col1;

    if (unkn_changing_color_counter1 & 0x01)
        col2 = colour_lookup[ColLU_RED];
    else
        col2 = colour_lookup[ColLU_WHITE];
    colour_lookup[ColLU_BLINK_RED_WHT] = col2;
}

void setup_color_lookups(void)
{
#if 0
    asm volatile ("call ASM_setup_color_lookups\n"
        :  :  : "eax" );
#endif
    if (display_palette == NULL) {
        LOGERR("Display palette not set, skipping");
        return;
    }
    colour_lookup_lock = 1;
    colour_lookup[ColLU_BLACK] = LbPaletteFindColour(display_palette, 0, 0, 0);
    colour_lookup[ColLU_WHITE] = LbPaletteFindColour(display_palette, 63, 63, 63);
    colour_lookup[ColLU_RED] = LbPaletteFindColour(display_palette, 63, 0, 0);
    colour_lookup[ColLU_GREEN] = LbPaletteFindColour(display_palette, 0, 63, 0);
    colour_lookup[ColLU_BLUE] = LbPaletteFindColour(display_palette, 0, 0, 63);
    colour_lookup[ColLU_YELLOW] = LbPaletteFindColour(display_palette, 63, 63, 0);
    colour_lookup[ColLU_CYAN] = LbPaletteFindColour(display_palette, 0, 63, 63);
    colour_lookup[ColLU_PINK] = LbPaletteFindColour(display_palette, 63, 0, 63);
    colour_lookup[ColLU_GREYLT] = LbPaletteFindColour(display_palette, 50, 50, 50);
    colour_lookup[ColLU_GREYMD] = LbPaletteFindColour(display_palette, 30, 30, 30);
    colour_lookup[ColLU_GREYDK] = LbPaletteFindColour(display_palette, 10, 10, 10);
    colour_lookup_lock = 0;
    update_unkn_changing_colors();

    colour_mix_lookup[0] = LbPaletteFindColour(display_palette, 0, 63, 0);
    colour_mix_lookup[1] = LbPaletteFindColour(display_palette, 38, 48, 63);
    colour_mix_lookup[2] = LbPaletteFindColour(display_palette, 0, 63, 63);
    colour_mix_lookup[3] = LbPaletteFindColour(display_palette, 63, 63, 0);
    colour_mix_lookup[4] = LbPaletteFindColour(display_palette, 63, 0, 63);
    colour_mix_lookup[5] = LbPaletteFindColour(display_palette, 63, 32, 32);
    colour_mix_lookup[6] = LbPaletteFindColour(display_palette, 32, 63, 32);
    colour_mix_lookup[7] = LbPaletteFindColour(display_palette, 32, 32, 63);
    colour_mix_lookup[8] = LbPaletteFindColour(display_palette, 32, 32, 32);
    colour_mix_lookup[9] = LbPaletteFindColour(display_palette, 32, 63, 63);
    colour_mix_lookup[10] = LbPaletteFindColour(display_palette, 63, 63, 32);
    colour_mix_lookup[11] = LbPaletteFindColour(display_palette, 63, 32, 63);

    colour_sel_grey[0] = LbPaletteFindColour(display_palette, 48, 48, 48);
    colour_sel_grey[1] = LbPaletteFindColour(display_palette, 40, 40, 40);
    colour_sel_grey[2] = LbPaletteFindColour(display_palette, 32, 32, 32);
    colour_sel_grey[3] = LbPaletteFindColour(display_palette, 24, 24, 24);

    colour_sel_green[0] = LbPaletteFindColour(display_palette, 0, 48, 0);
    colour_sel_green[1] = LbPaletteFindColour(display_palette, 0, 40, 0);
    colour_sel_green[2] = LbPaletteFindColour(display_palette, 0, 32, 0);
    colour_sel_green[3] = LbPaletteFindColour(display_palette, 0, 24, 0);

    colour_sel_blue[0] = LbPaletteFindColour(display_palette, 0, 0, 48);
    colour_sel_blue[1] = LbPaletteFindColour(display_palette, 0, 0, 40);
    colour_sel_blue[2] = LbPaletteFindColour(display_palette, 0, 0, 32);
    colour_sel_blue[3] = LbPaletteFindColour(display_palette, 0, 0, 24);

    colour_sel_red[0] = LbPaletteFindColour(display_palette, 48, 0, 0);
    colour_sel_red[1] = LbPaletteFindColour(display_palette, 40, 0, 0);
    colour_sel_red[2] = LbPaletteFindColour(display_palette, 32, 0, 0);
    colour_sel_red[3] = LbPaletteFindColour(display_palette, 24, 0, 0);

    colour_sel_purple[0] = LbPaletteFindColour(display_palette, 48, 0, 48);
    colour_sel_purple[1] = LbPaletteFindColour(display_palette, 40, 0, 40);
    colour_sel_purple[2] = LbPaletteFindColour(display_palette, 32, 0, 32);
    colour_sel_purple[3] = LbPaletteFindColour(display_palette, 24, 0, 24);
}

void palette_apply_brightness(ubyte *pal)
{
    int colr, cmpn;

    for (colr = 0; colr < 0x300; colr += 3)
    {
        int sum, bri;
        sum = 0;
        for (cmpn = 0; cmpn < 3; cmpn++)
            sum += pal[colr + cmpn];
        if ((sum < 56) && (momentary_brightness < -1)) {
            bri = momentary_brightness * (sum + 8) >> 6;
        } else if ((sum > 3*63 - 56) && (momentary_brightness > 1)) {
            bri = momentary_brightness * (sum - (3*63 - 56) + 8) >> 6;
        } else {
            bri = momentary_brightness;
        }
        for (cmpn = 0; cmpn < 3; cmpn++)
        {
            ubyte *p_intens;
            int i;
            p_intens = &pal[colr + cmpn];
            i = (*p_intens) + bri;
            if (i < 0)
              i = 0;
            if (i > 63)
              i = 63;
            *p_intens = i;
        }
    }
}

void change_brightness_from_normal(short amount)
{
    momentary_brightness += amount;
    if (momentary_brightness < -63)
        momentary_brightness = -63;
    if (momentary_brightness > 63)
        momentary_brightness = 63;

    palette_apply_brightness(display_palette);
    LbPaletteSet(display_palette);
}

/******************************************************************************/
