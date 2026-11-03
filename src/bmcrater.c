/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bmcrater.c
 *     On-map explosion effect implementation.
 * @par Purpose:
 *     Implement functions displaying an explosion within the game world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Sep 2023 - 17 Mar 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bmcrater.h"

#include "bfutility.h"

#include "bigmap.h"
#include "game.h"
#include "swlog.h"
/******************************************************************************/

#define MAP_CRATERS_COUNT 128

/******************************************************************************/

extern ubyte map_crater_next;
extern struct MapCreater map_craters[MAP_CRATERS_COUNT];

struct CraterTileSpec {
    sbyte dx, dz;
    ubyte depth;
    TbBool gated; /**< If true, the tile only has 50% chance of being stamped. */
};

/** size=0: only the center tile, always stamped. */
static const struct CraterTileSpec crater_tiles_size0[] = {
  { 0,  0, 0, false},
};

/** size=1: center plus the 4 orthogonal neighbours, each with 50% chance. */
static const struct CraterTileSpec crater_tiles_size1[] = {
  { 0,  0, 1, false},
  { 0, -1, 0, true},
  { 0,  1, 0, true},
  {-1,  0, 0, true},
  { 1,  0, 0, true},
};

/** size=2: center, the 4 orthogonal neighbours (always), and the 4 diagonal
 * neighbours (each with 50% chance).
 */
static const struct CraterTileSpec crater_tiles_size2[] = {
  { 0,  0, 2, false},
  { 0, -1, 1, false},
  { 0,  1, 1, false},
  {-1,  0, 1, false},
  { 1,  0, 1, false},
  {-1, -1, 0, true},
  { 1, -1, 0, true},
  {-1,  1, 0, true},
  { 1,  1, 0, true},
};

/** size=3: center, the 4 orthogonal and 4 diagonal neighbours (always), and
 * the 12 tiles of the distance-2 ring, where the corner tiles of each ring
 * edge have a 50% chance and the middle tile of each edge is always stamped.
 */
static const struct CraterTileSpec crater_tiles_size3[] = {
  { 0,  0, 3, false},
  { 0, -1, 2, false},
  { 0,  1, 2, false},
  {-1,  0, 2, false},
  { 1,  0, 2, false},
  {-1, -1, 1, false},
  { 1, -1, 1, false},
  {-1,  1, 1, false},
  { 1,  1, 1, false},
  {-2, -1, 0, true},
  {-2,  0, 0, false},
  {-2,  1, 0, true},
  { 2, -1, 0, true},
  { 2,  0, 0, false},
  { 2,  1, 0, true},
  {-1, -2, 0, true},
  { 0, -2, 0, false},
  { 1, -2, 0, true},
  {-1,  2, 0, true},
  { 0,  2, 0, false},
  { 1,  2, 0, true},
};

/******************************************************************************/

void create_crater(short tile_x, short tile_z, short depth)
{
    int cratr_no;

    LOGSYNC("crater at (%d,%d) depth %d\n", tile_x, tile_z, depth);
    if (tile_x < 0 || tile_x >= MAP_TILE_WIDTH)
        return;
    if (tile_z < 0 || tile_z >= MAP_TILE_HEIGHT)
        return;
    cratr_no = map_crater_next;
    map_crater_next++;
    if (map_crater_next >= MAP_CRATERS_COUNT)
        map_crater_next = 0;
    map_craters[cratr_no].MapX = tile_x;
    map_craters[cratr_no].MapY = tile_z;
    map_craters[cratr_no].Depth = depth;
    map_craters[cratr_no].Iterations = 0;
}

/** Stamps a single crater tile.
 *
 * Randomizes ground altitude and marks the tile with the crater texture,
 * unless it is a fluid surface, or (for gated tiles) a random chance
 * for damage does not land.
 */
static void quick_crater_stamp_tile(int x, int z, int depth, TbBool gated,
  int *min_x, int *max_x, int *min_z, int *max_z)
{
    struct MyMapElement *p_mapel;
    ushort textr;

    if (gated && ((LbRandomAnyShort() & 1) == 0))
        return;
    if ((x < 0) || (x > MAP_TILE_WIDTH - 1))
        return;
    if ((z < 0) || (z > MAP_TILE_HEIGHT - 1))
        return;

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * z + x];
    textr = p_mapel->Texture & 0x3FFF;
    if (floor_texture_is_fluid(textr))
        return;

    p_mapel->Texture = (p_mapel->Texture & 0xC000) | crater_unkn_texture_flags;
    p_mapel->ColumnHead &= 0x0FFF; // clear TearAround, shared zip-pass will fill it in
    p_mapel->Alt -= (short)(2 * depth + (LbRandomAnyShort() & 3));

    if (x < *min_x) *min_x = x;
    if (x > *max_x) *max_x = x;
    if (z < *min_z) *min_z = z;
    if (z > *max_z) *max_z = z;
}

static void quick_crater_stamp_shape(int cx, int cz, const struct CraterTileSpec *tiles, int count,
  int *min_x, int *max_x, int *min_z, int *max_z)
{
    int i;
    for (i = 0; i < count; i++)
    {
        quick_crater_stamp_tile(cx + tiles[i].dx, cz + tiles[i].dz, tiles[i].depth, tiles[i].gated,
          min_x, max_x, min_z, max_z);
    }
}

/** Digs a crater of given size (0..3) centered at map tile (x,z).
 *
 * Randomizes ground altitude and sets the crater texture/flags on the
 * stamped tiles (a shape depending on `size`, with most non-center tiles
 * having only 50% chance to be stamped), then smooths the result: tiles
 * neighbouring the crater on at least 3 of their 4 orthogonal sides are
 * absorbed into the crater, and a marching-squares pass selects a
 * TearAround (diagonal/edge blend) value for every non-crater tile
 * touching the crater, to avoid harsh texture transitions.
 */
void quick_crater(int x, int z, int size)
{
#if 0
    asm volatile ("call ASM_quick_crater\n"
        :  : "a" (x), "d" (z), "b" (size));
    return;
#endif
    int min_x, max_x, min_z, max_z;
    int tl_x, tl_z;

    if (current_map == 11) // map011 Orbital Station
        return;
    if ((size < 0) || (size > 3)) {
        LOGWARN("crater at (%d,%d) has invalid size %d\n", x, z, size);
        return;
    }

    min_x = max_x = x;
    min_z = max_z = z;

    switch (size)
    {
    case 0:
        quick_crater_stamp_shape(x, z, crater_tiles_size0,
          sizeof(crater_tiles_size0) / sizeof(crater_tiles_size0[0]),
          &min_x, &max_x, &min_z, &max_z);
        break;
    case 1:
        quick_crater_stamp_shape(x, z, crater_tiles_size1,
          sizeof(crater_tiles_size1) / sizeof(crater_tiles_size1[0]),
          &min_x, &max_x, &min_z, &max_z);
        break;
    case 2:
        quick_crater_stamp_shape(x, z, crater_tiles_size2,
          sizeof(crater_tiles_size2) / sizeof(crater_tiles_size2[0]),
          &min_x, &max_x, &min_z, &max_z);
        break;
    case 3:
        quick_crater_stamp_shape(x, z, crater_tiles_size3,
          sizeof(crater_tiles_size3) / sizeof(crater_tiles_size3[0]),
          &min_x, &max_x, &min_z, &max_z);
        break;
    }

    min_x--; max_x++;
    min_z--; max_z++;
    if (min_x < 0) min_x = 0;
    if (max_x > MAP_TILE_WIDTH - 1) max_x = MAP_TILE_WIDTH - 1;
    if (min_z < 0) min_z = 0;
    if (max_z > MAP_TILE_HEIGHT - 1) max_z = MAP_TILE_HEIGHT - 1;

    // Pass 1: absorb tiles bordering the crater on at least 3 of 4 sides
    // into the crater, to get rid of thin un-cratered slivers.
    for (tl_z = min_z; tl_z <= max_z; tl_z++)
    {
        for (tl_x = min_x; tl_x <= max_x; tl_x++)
        {
            ushort own_tex;
            TbBool match_e, match_w, match_s, match_n;

            own_tex = floor_texture_at_tile(tl_x, tl_z);
            if (own_tex == crater_unkn_texture_flags)
                continue;

            match_e = (floor_texture_at_tile(tl_x + 1, tl_z + 0) == crater_unkn_texture_flags);
            match_w = (floor_texture_at_tile(tl_x - 1, tl_z + 0) == crater_unkn_texture_flags);
            match_s = (floor_texture_at_tile(tl_x + 0, tl_z + 1) == crater_unkn_texture_flags);
            match_n = (floor_texture_at_tile(tl_x + 0, tl_z - 1) == crater_unkn_texture_flags);

            if ((match_e && match_w && match_s) || (match_e && match_w && match_n)
              || (match_w && match_s && match_n) || (match_e && match_s && match_n))
            {
                struct MyMapElement *p_mapel;
                p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tl_z + tl_x];
                p_mapel->Texture = (p_mapel->Texture & 0xC000) | crater_unkn_texture_flags;
                p_mapel->ColumnHead &= 0x0FFF;
            }
        }
    }

    // Pass 2: marching-squares style selection of a TearAround (diagonal
    // edge blend) value for tiles neighbouring the crater, based on which
    // of the 8 surrounding tiles are crater texture.
    for (tl_z = min_z; tl_z <= max_z; tl_z++)
    {
        for (tl_x = min_x; tl_x <= max_x; tl_x++)
        {
            ushort own_tex;
            ushort tear;
            TbBool match_nw, match_n, match_ne, match_w, match_e, match_sw, match_s, match_se;

            own_tex = floor_texture_at_tile(tl_x, tl_z);
            if (own_tex == crater_unkn_texture_flags)
                continue;
            if (sea_texture(own_tex))
                continue;

            match_nw = (floor_texture_at_tile(tl_x - 1, tl_z - 1) == crater_unkn_texture_flags);
            match_n  = (floor_texture_at_tile(tl_x + 0, tl_z - 1) == crater_unkn_texture_flags);
            match_ne = (floor_texture_at_tile(tl_x + 1, tl_z - 1) == crater_unkn_texture_flags);
            match_w  = (floor_texture_at_tile(tl_x - 1, tl_z + 0) == crater_unkn_texture_flags);
            match_e  = (floor_texture_at_tile(tl_x + 1, tl_z + 0) == crater_unkn_texture_flags);
            match_sw = (floor_texture_at_tile(tl_x - 1, tl_z + 1) == crater_unkn_texture_flags);
            match_s  = (floor_texture_at_tile(tl_x + 0, tl_z + 1) == crater_unkn_texture_flags);
            match_se = (floor_texture_at_tile(tl_x + 1, tl_z + 1) == crater_unkn_texture_flags);

            tear = 0;
            if (match_e && match_s)
                tear = 12;
            else if (match_n && match_e)
                tear = 9;
            else if (match_w && match_n)
                tear = 6;
            else if (match_s && match_w)
                tear = 3;
            else if (match_s)
                tear = 1;
            else if (match_e)
                tear = 10;
            else if (match_n)
                tear = 7;
            else if (match_w)
                tear = 4;
            else if (match_se)
                tear = 11;
            else if (match_ne)
                tear = 8;
            else if (match_nw)
                tear = 5;
            else if (match_sw)
                tear = 2;

            if (tear != 0) {
                struct MyMapElement *p_mapel;
                p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tl_z + tl_x];
                p_mapel->ColumnHead = (p_mapel->ColumnHead & 0x0FFF) | (tear << 12);
            }
        }
    }
}

/******************************************************************************/
