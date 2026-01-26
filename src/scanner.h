/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file scanner.h
 *     Header file for scanner.c.
 * @par Purpose:
 *     Ingame scanner (minimap/radar) support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef SCANNER_H
#define SCANNER_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define SCANNER_BIG_BLIP_COUNT 16
#define SCANNER_GROUP_COUNT 6
#define SCANNER_ARC_COUNT 4

#define SCANNER_MAPDATA_WIDTH 256
#define SCANNER_MAPDATA_HEIGHT 256

struct Objective;
struct NetscanObjective;

struct BigBlip // sizeof=12
{
  long X;
  long Z;
  ubyte Colour;
  ubyte Period;
  ubyte Speed;
  ubyte Counter;
};

struct Arc // sizeof=20
{
  long X1;
  long Z1;
  long X2;
  long Z2;
  ubyte ColourIsUnused;
  ubyte Period;
  ubyte SpeedIsUnused;
  ubyte Counter;
};

struct Scanner // sizeof=0x467 (before resize)
{
  long X1; // offset=ingame+0x00C
  long Y1;
  long X2;
  long Y2;
  long MX;
  long MZ;
  long Zoom;
  long Angle;
  long Contrast;
  long Brightness;
  long Width[500]; // offset=ingame+0x034; modified 200 -> 500
  struct BigBlip BigBlip[SCANNER_BIG_BLIP_COUNT];
  struct Arc Arc[SCANNER_ARC_COUNT];
  ubyte Group[SCANNER_GROUP_COUNT];
  ubyte GroupCol[SCANNER_GROUP_COUNT];
  ubyte GroupCount;
  short NearThing1;
};

#pragma pack()
/******************************************************************************/
extern long *SCANNER_width;
extern ubyte SCANNER_data[SCANNER_MAPDATA_HEIGHT][SCANNER_MAPDATA_WIDTH];
extern ushort SCANNER_base_zoom_factor;
extern ushort SCANNER_user_zoom_factor;
extern ubyte SCANNER_scale_dots;

extern short waft_table2[32];
extern short waft_table[32];

void SCANNER_init(void);
void SCANNER_fill_in(void);
void SCANNER_fill_in_a_little_bit(int x1, int y1, int x2, int y2);
void SCANNER_set_colour(ubyte col);
void SCANNER_find_position(int x, int y, int *U, int *V);
void SCANNER_data_to_screen(void);
void SCANNER_set_zoom(int zoom);

/** Set screen location box coordinates of the scanner view.
 *
 * @param x Screen buffer location X coordinate.
 * @param y Screen buffer location Y coordinate.
 * @param width Screen rectangle width.
 * @param height Screen rectangle height.
 * @param cutout Depth of bottom right cutout of the screen rectangle.
 */
void SCANNER_set_screen_box(short x, short y, short width, short height, short cutout);

TbBool mouse_move_over_scanner(void);
void clear_all_scanner_signals(void);
void add_signal_to_scanner(struct Objective *p_objectv, ubyte flag);
void add_netscan_signal_to_scanner(struct NetscanObjective *p_nsobv, ubyte flag);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
