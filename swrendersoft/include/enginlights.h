/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginlights.h
 *     Header file for enginlights.c.
 * @par Purpose:
 *     Lights structures required by the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINLIGHTS_H
#define ENGINLIGHTS_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define MAX_LIGHTS_AFFECTING_FACE 100

struct QuickLight { // sizeof=6
    ushort Ratio;
    ushort Light;
    ushort NextQuick;
};

struct FullLight { // sizeof=32
    short Intensity;
    short TrueIntensity;
    short Command;
    short NextFull;
    short X;
    short Y;
    short Z;
    short lgtfld_E;
    short lgtfld_10;
    short lgtfld_12;
    ubyte lgtfld_14[10];
    ushort Flags;
};

struct FullLightOldV12 { // sizeof=20
    short Intensity;
    short TrueIntensity;
    short Command;
    short NextFull;
    short X;
    short Y;
    short Z;
    short lgtfld_E;
    short lgtfld_10;
    short lgtfld_12;
};

struct LightCommand { // sizeof=36
    ubyte lcfld_0[36];
};

#pragma pack()
/******************************************************************************/
extern struct QuickLight *game_quick_lights;
extern struct FullLight *game_full_lights;
extern struct LightCommand *game_light_commands;
extern ushort next_quick_light;
extern ushort next_full_light;
extern ushort next_light_command;

uint cummulate_shade_from_quick_lights(ushort light_first);

/** Maps fields from old FullLight struct to the current one.
 */
void refresh_old_full_light_format(struct FullLight *p_fulight,
  struct FullLightOldV12 *p_oldfulight, u32 fmtver);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
