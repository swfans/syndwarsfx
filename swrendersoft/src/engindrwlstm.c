/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstm.c
 *     Making drawlists for the 3D engine.
 * @par Purpose:
 *     Implements functions for filling drawlists.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engindrwlstm.h"

#include "bfendian.h"
#include "bfmath.h"
#include "bfmemut.h"
#include "bfutility.h"
#include <assert.h>
#include <limits.h>
#include <string.h>

#include "enginbckt.h"
#include "engindrwlstx.h"
#include "enginfloor.h"
#include "enginprops.h"
#include "enginsngobjs.h"

/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/

struct SortLine *draw_item_add_line(ubyte ditype, int bckt)
{
    struct SortLine *p_sline;

    if (next_sort_line + 1 > sort_lines_limit)
        return NULL;

    p_sline = p_current_sort_line;
    if (!draw_item_add(ditype, next_sort_line, bckt))
        return NULL;

    p_current_sort_line++;
    next_sort_line++;

    return p_sline;
}

struct SortSprite *draw_item_add_sprite(ubyte ditype, int bckt)
{
    struct SortSprite *p_sspr;

    if (next_sort_sprite + 1 > sort_sprites_limit)
        return NULL;

    p_sspr = p_current_sort_sprite;
    if (!draw_item_add(ditype, next_sort_sprite, bckt))
        return NULL;

    p_current_sort_sprite++;
    next_sort_sprite++;

    return p_sspr;
}

struct SpecialPoint *draw_item_add_points(ubyte ditype, ushort offset,
  int bckt, ushort npoints)
{
    struct SpecialPoint *p_scrpoint;

    if (next_screen_point + npoints > screen_points_limit)
        return NULL;

    p_scrpoint = &game_screen_point_pool[next_screen_point];
    if (!draw_item_add(ditype, offset, bckt))
        return NULL;

    next_screen_point += npoints;

    return p_scrpoint;
}

struct FloorTile *draw_item_add_floor_tile(ubyte ditype, int bckt)
{
    struct FloorTile *p_floortl;

    if (next_floor_tile + 1 > floor_tiles_limit)
        return NULL;

    p_floortl = &game_floor_tiles[next_floor_tile];
    if (!draw_item_add(ditype, next_floor_tile, bckt))
        return NULL;

    next_floor_tile++;

    return p_floortl;
}

struct SingleObjectFace4 *draw_item_add_special_obj_face4_no_pts(ubyte ditype, int bckt)
{
    struct SingleObjectFace4 *p_face4;
    ushort face;

    face = next_special_obj_face4;
    if (face + 1 > game_special_obj_faces4_limit)
        return NULL;

    p_face4 = &game_special_obj_faces4[face];

    if (!draw_item_add(ditype, face, bckt))
        return NULL;

    next_special_obj_face4++;

    return p_face4;
}

struct SingleObjectFace4 *draw_item_add_special_obj_face4(ubyte ditype, int bckt)
{
    struct SingleObjectFace4 *p_face4;
    ushort pt;

    pt = next_screen_point;
    if (pt + 4 > screen_points_limit)
        return NULL;

    p_face4 = draw_item_add_special_obj_face4_no_pts(ditype, bckt);

    if (p_face4 == NULL)
        return NULL;

    next_screen_point += 4;

    p_face4->PointNo[0] = pt + 0;
    p_face4->PointNo[1] = pt + 1;
    p_face4->PointNo[2] = pt + 2;
    p_face4->PointNo[3] = pt + 3;

    return p_face4;
}
/******************************************************************************/
