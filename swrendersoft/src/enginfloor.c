/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginfloor.c
 *     Support for 3D map floor tiles.
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
#include "enginfloor.h"

#include "enginzoom.h"
#include "privrdlog.h"
/******************************************************************************/

struct FloorTile *game_floor_tiles = NULL;
short next_floor_tile = 1;

/******************************************************************************/
