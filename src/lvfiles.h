/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file lvfiles.h
 *     Header file for lvfiles.c.
 * @par Purpose:
 *     Routines for level and map files handling.
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
#ifndef LVFILES_H
#define LVFILES_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define LEVEL_NUM_STRAIN(lv) ( ((lv) < 15) ? (lv) : ((lv) - 1) % 15 + 1 )
#define LEVEL_NUM_VARIANT(lv) ( ((lv) < 15) ? 0 : ((lv) - 1) / 15 )
#define LEVEL_NUM(lv_strain, lv_variant) (((lv_variant) * 15 + (lv_strain) - 1) + 1)

enum LevelMiscEntryType {
    LvMiscT_NONE,
    LvMiscT_MGUN,
};

struct Objective;

struct QuickLoad {
    ushort *Numb;
    void **Ptr;
    ushort Size;
    ushort Extra;
};

struct LevelMisc { // sizeof=22
    short X;
    short Y;
    short Z;
    short Group;
    short Weapon;
    ubyte Flag;
    ubyte Type;
    ubyte field_C[9];
    ubyte field_15;
};

#pragma pack()
/******************************************************************************/
extern struct LevelMisc *game_level_miscs;
extern TbBool level_deep_fix;
extern struct Objective *game_used_lvl_objectives;
extern ushort next_used_lvl_objective;

extern struct UnknBezEdit *bezier_pts;
extern ushort next_bezier_pt;

TbResult load_mad_pc(ushort mapno);

void load_level_pc(short level, short missi, ubyte reload);

/** Get start position for in-mission camera from misc entry with mounted gun.
 *
 * Makes little sense, but original game does that (if there ar any guns on a level).
 */
TbBool level_misc_get_starting_camera_pos(MapCoord *cor_x, MapCoord *cor_z);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
