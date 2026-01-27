/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_bstype.h
 *     Basic data types declaration.
 * @par Purpose:
 *     Declares very basic data types, which are widely used in other headers.
 *     Expands upon data types from bflibrary, adding game-specific types.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Jan 2016 - 03 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GAME_BSTYPE_H
#define GAME_BSTYPE_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/** Game turn number, used for in-game time computations. */
typedef ulong GameTurn;
/** Signed game turn, for storing game turns difference. */
typedef long SGameTurn;

/** Type which stores index/offset of a Thing/SimpleThing.
 *
 * The value may refer to a Thing (if > 0) or SimpleThing (if < 0).
 */
typedef short ThingIdx;

/** Type which stores index/offset of a Player.
 *
 * The value is a valid index in players[] array.
 */
typedef ubyte PlayerIdx;

/** Type which stores type of a weapon.
 */
typedef ubyte WeaponType;

/** Coordinate on the screen surface.
 * The type shall allow safety store values beyond (below/above) real screen
 * coordinates, up to another screen size. This means it needs to allow to
 * store -MAX_SUPPORTED_SCREEN_WIDTH .. 2*MAX_SUPPORTED_SCREEN_WIDTH, or the
 * same for height.
 */
typedef short ScrCoord;

/** Type which stores index of a Game Key binding.
 */
typedef ushort GameKey;

/**
 *  store a set of joystick buttons pressed.
 */
typedef ushort JoyButtonSet;

/** Type which stores coordinate on the ingame map.
 *
 * Can be separated into a tile and position within.
 * Position within a tile is scaled 0..255.
 */
typedef short MapCoord;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
