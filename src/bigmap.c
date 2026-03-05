/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bigmap.c
 *     Routines implementing map elements reference lists.
 * @par Purpose:
 *     Implement functions for handling bigmap handling.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bigmap.h"

#include <stdlib.h>
#include <limits.h>
#include "bfmath.h"
#include "bfmemut.h"

#include "enginprops.h"
#include "enginsngtxtr.h"

#include "swlog.h"
/******************************************************************************/
struct MapOffset spiral_step[SPIRAL_STEPS_COUNT];
ushort dist_tiles_to_spiral_step[MAP_TILE_WIDTH];
ushort spiral_dist_tiles_limit = 0;

const struct Direction angle_direction[] = {
    {   0,  256},
    { 181,  181},
    { 256,    0},
    { 181, -181},
    {   0, -256},
    {-181, -181},
    {-256,    0},
    {-181,  181},
};

TbBool map_coords_limit(MapCoord *cor_x, MapCoord *cor_y, MapCoord *cor_z, long map_x, long map_y, long map_z)
{
    TbBool altered;

    altered = false;
    if (map_x < 0)
        map_x = 0;
    else if (map_x >= MAP_COORD_WIDTH)
        map_x = MAP_COORD_WIDTH - 1;

    if (map_y < -INT16_MAX)
        map_y = -INT16_MAX;
    else if (map_y > INT16_MAX)
        map_y = INT16_MAX;

    if (map_z < 0)
        map_z = 0;
    else if (map_z >= MAP_COORD_HEIGHT)
        map_z = MAP_COORD_HEIGHT - 1;

    if (cor_x != NULL) {
        altered |= (*cor_x != map_x);
        *cor_x = map_x;
    }
    if (cor_y != NULL) {
        altered |= (*cor_y != map_y);
        *cor_y = map_y;
    }
    if (cor_z != NULL) {
        altered |= (*cor_z != map_z);
        *cor_z = map_z;
    }

    return altered;
}

void clear_mapwho_on_whole_map(void)
{
    short tile_x, tile_z;
    for (tile_x = 0; tile_x < MAP_TILE_WIDTH; tile_x++)
    {
        for (tile_z = 0; tile_z < MAP_TILE_HEIGHT; tile_z++)
        {
            struct MyMapElement *p_mapel;
            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            p_mapel->Child = 0;
        }
    }
}

short get_mapwho_thing_index(short tile_x, short tile_z)
{
    struct MyMapElement *p_mapel;

    if ((tile_x < 0) || (tile_x >= MAP_TILE_WIDTH))
        return 0;
    if ((tile_z < 0) || (tile_z >= MAP_TILE_HEIGHT))
        return 0;

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];

    return p_mapel->Child;
}

/** Maps fields from old MyMapElement struct to the current one.
 */
void refresh_old_my_big_map_format(struct MyMapElement *p_mapel, struct MyMapElementOldV7 *p_oldmapel, u32 fmtver)
{
    LbMemorySet(p_mapel, 0, sizeof(struct MyMapElement));

    p_mapel->Texture = p_oldmapel->Texture;
    p_mapel->Shade = p_oldmapel->Shade;
    p_mapel->ShadeR = p_oldmapel->ShadeR;
    p_mapel->Flags = p_oldmapel->Flags;
    p_mapel->Alt = p_oldmapel->Alt;
    p_mapel->Zip = p_oldmapel->Zip;
    p_mapel->Flags2 = p_oldmapel->Flags2;
    p_mapel->Child = p_oldmapel->Child;
    p_mapel->ColHead = p_oldmapel->ColHead;
    p_mapel->ColumnHead = p_oldmapel->ColumnHead;
    p_mapel->Ambient = p_oldmapel->Ambient;

    if (fmtver <= 9)
    {
        if (p_mapel->Texture & 0x8000) {
            short k;
            k = p_mapel->Texture;
            k = -k;
            k = (k & 0xFF) | ((k - (0x80 << 8)) & 0xFF00);
            p_mapel->Texture = k;
        }
    }
}

void init_search_spiral_steps(void)
{
    struct MapOffset *sstep;
    long x,y;
    long i;

    y = 0;
    x = 0;
    sstep = &spiral_step[0];
    sstep->h = y;
    sstep->v = x;
    sstep->both = (short)y + ((short)x * MAP_TILE_WIDTH);
    y = -1;
    x = -1;
    for (i=1; i < SPIRAL_STEPS_COUNT; i++)
    {
      sstep = &spiral_step[i];
      sstep->h = y;
      sstep->v = x;
      sstep->both = (short)y + ((short)x * MAP_TILE_WIDTH);
      if ((y < 0) && (x-y == 1))
      {
          y--;
          x -= 2;
      } else
      if (x == y)
      {
          if (y < 0)
            y++;
          else
            y--;
      } else
      if (y+x == 0)
      {
          if (x >= 0)
            x--;
          else
            x++;
      } else
      if (abs(x) >= abs(y))
      {
          if (x < 0)
            y++;
          else
            y--;
      } else
      {
          if (y >= 0)
            x++;
          else
            x--;
      }
    }
}

void init_dist_to_spiral_steps(void)
{
    struct MapOffset *sstep;
    long i, ssi;
    ulong dist, distss;

    ssi = SPIRAL_STEPS_COUNT-1;
    for (i = MAP_TILE_WIDTH; i > 0; i--)
    {
        dist = (i * i) * 2;
        for (; ssi > 0; ssi--) {
            sstep = &spiral_step[ssi];
            distss = (sstep->h * sstep->h) + (sstep->v * sstep->v);
            if (distss < dist) {
                break;
            }
        }
        dist_tiles_to_spiral_step[i-1] = ssi + 1;
    }
    for (i = 0; i < MAP_TILE_WIDTH; i++) {
        if (dist_tiles_to_spiral_step[i] >= SPIRAL_STEPS_COUNT)
            break;
    }
    spiral_dist_tiles_limit = i;
}

void init_search_spiral(void)
{
    init_search_spiral_steps();
    init_dist_to_spiral_steps();
    LOGSYNC("Created for distance up to %hu tiles", spiral_dist_tiles_limit);
}

ushort floor_texture_at_point(MapCoord cor_x, MapCoord cor_z)
{
    struct MyMapElement *p_mapel;
    short tile_x, tile_z;

    tile_x = MAPCOORD_TO_TILE(cor_x);
    tile_z = MAPCOORD_TO_TILE(cor_z);

    if ((tile_x < 0) || (tile_x >= MAP_TILE_WIDTH))
        return 0;
    if ((tile_z < 0) || (tile_z >= MAP_TILE_HEIGHT))
        return 0;

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x)];
    return (p_mapel->Texture & 0x3FFF);
}

int alt_at_point(short x, short z)
{
#if 1
    int ret;
    asm volatile ("call ASM_alt_at_point\n"
        : "=r" (ret) : "a" (x), "d" (z));
    return ret;
#else
    struct MyMapElement *p_mapel;
    short tile_x, tile_z;
    int alt0, alt1, alt2, alt3;

    tile_x = MAPCOORD_TO_TILE(x);
    tile_z = MAPCOORD_TO_TILE(z);

    if ((tile_x < 0) || (tile_x >= MAP_TILE_WIDTH))
        return 0;
    if ((tile_z < 0) || (tile_z >= MAP_TILE_HEIGHT))
        return 0;

    p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x)];
    alt0 = p_mapel->Alt;

    if (tile_x == MAP_TILE_WIDTH - 1) {
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x)];
    } else {
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x+1)];
    }
    alt1 = p_mapel->Alt;

    if (tile_z == MAP_TILE_HEIGHT - 1) {
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x)];
    } else {
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z+1) + (tile_x)];
    }
    alt2 = p_mapel->Alt;

    if (tile_z == MAP_TILE_HEIGHT - 1) {
        alt3 = alt1;
    } else {
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z+1) + (tile_x+1)];
        alt3 = p_mapel->Alt;
    }

    if (alt1 != alt0 || alt0 != alt2 || alt2 != alt3)
    {
        short sub_x, sub_z;

        sub_x = x & 0xff;
        sub_z = z & 0xff;
        if (sub_x + sub_z >= 256) {
            alt0 = alt3 + ((alt2 - alt3) * (256 - sub_x) >> 8) + ((alt1 - alt3) * (256 - sub_z) >> 8);
        } else {
            alt0 = alt0 + ((alt1 - alt0) * sub_x >> 8) + ((alt2 - alt0) * sub_z >> 8);
        }
    }
    return alt0 << 8;
#endif
}

int alt_at_point_under_height(int cor_x, int cor_z, int h)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_alt_at_point_under_height\n"
        : "=r" (ret) : "a" (x), "d" (z), "b" (h));
    return ret;
#endif
    short tile_x, tile_z;
    ushort col;
    ubyte quarter;

    tile_x = MAPCOORD_TO_TILE(cor_x);
    tile_z = MAPCOORD_TO_TILE(cor_z);

    if ((tile_x < 0) || (tile_x >= MAP_TILE_WIDTH))
        return 0;
    if ((tile_z < 0) || (tile_z >= MAP_TILE_HEIGHT))
        return 0;
    {
        struct MyMapElement *p_mapel;
        p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z) + (tile_x)];
        col = p_mapel->ColumnHead & 0xFFF;
        if ((cor_x & 0x80) == 0)
        {
            if ((cor_z & 0x80) == 0)
                quarter = 0;
            else
                quarter = 3;
        }
        else
        {
            if ((cor_z & 0x80) == 0)
                quarter = 1;
            else
                quarter = 2;
        }
    }

    int alt_curr, alt_best, h_max;

    h_max = 8 * h;
    alt_curr = 8 * alt_at_point(cor_x, cor_z);
    alt_best = alt_curr;

    if (col != 0)
    {
        struct ColColumn *p_col;
        u32 mask;

        p_col = &game_col_columns[col];

        for (mask = 1; mask; mask *= 2)
        {
            if (alt_curr > h_max)
                break;
            if ((mask & p_col->QBits[quarter]) != 0)
                alt_best = alt_curr;
            alt_curr += 0x8000;
        }
    }
    return alt_best;
}

int alt_change_at_tile(short tile_x, short tile_z, int *change_xz)
{
    int alt_min, alt_max;
    int dtx, dtz;

    if (tile_x <= 0 || tile_x >= MAP_TILE_WIDTH-1)
        return 63;
    if (tile_z <= 0 || tile_z >= MAP_TILE_HEIGHT-1)
        return 63;

    alt_min = INT_MAX;
    alt_max = INT_MIN;
    for (dtz = 0; dtz <= 1; dtz++)
    {
        for (dtx = 0; dtx <= 1; dtx++)
        {
            struct MyMapElement *p_mapel;
            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * (tile_z + dtz) + (tile_x + dtx)];
            if (alt_min > p_mapel->Alt)
                alt_min = p_mapel->Alt;
            if (alt_max < p_mapel->Alt)
                alt_max = p_mapel->Alt;
        }
    }
    // A bit of simplification here - if min and max are diagonal, the distance
    // is larger by sqrt(2). But it matters little in this case, plus min-max
    // selection would have to directly compute steepness to take the diagonal
    // into account properly
    if (change_xz != NULL)
        *change_xz = 256;
    return abs(alt_max - alt_min);
}

static ushort count_tiles_around_steeper_than(short tile_x, short tile_z, short steepness)
{
    int dtx, dtz;
    ushort matches;

    matches = 0;
    for (dtz = -1; dtz <= 1; dtz++)
    {
        for (dtx = -1; dtx <= 1; dtx++)
        {
            int alt_dt, gnd_dt;

            alt_dt = 8 * alt_change_at_tile(tile_x + dtx, tile_z + dtz, &gnd_dt);
            if (alt_dt > steepness)
                matches++;
        }
    }
    return matches;
}

TbBool compute_map_tile_is_blocking_walk(short tile_x, short tile_z)
{
    int alt_dt, gnd_dt;

    alt_dt = 8 * alt_change_at_tile(tile_x, tile_z, &gnd_dt);

    // We will compare linear steepness, as simplification of computing angle
    //int angle = LbArcTanAngle(alt_dt,-gnd_dt);

    // If steepness is higher than 133% of the set limit, then it is plainly blocking
    if (alt_dt > MAX_WALKABLE_STEEPNESS_PER_256 * 4 / 3)
        return true;

    // If steepness is lower than the set limit, then it is plainly non-blocking
    if (alt_dt <= MAX_WALKABLE_STEEPNESS_PER_256)
        return false;

    // For remaining range, do more complex check: block only if more than 3
    // tiles in the vicinity are also meeting the blocking critaria.
    // This avoids having single blocking tiles or rows of tiles which look
    // walkable but are not; it makes sense that a single tile can be
    // traversed at higher steepness that a larger steep area

    if ((tile_x <= 0) || (tile_x >= MAP_TILE_WIDTH))
        return true;

    if ((tile_z <= 0) || (tile_z >= MAP_TILE_HEIGHT))
        return true;

    if (count_tiles_around_steeper_than(tile_x, tile_z, MAX_WALKABLE_STEEPNESS_PER_256) > 3)
        return true;

   return false;
}

void update_map_flags(void)
{
    ushort tile_x, tile_z;

    for (tile_x = 0; tile_x < MAP_TILE_WIDTH; tile_x++)
    {
        for (tile_z = 0; tile_z < MAP_TILE_HEIGHT; tile_z++)
        {
            struct MyMapElement *p_mapel;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            // set having a walkable tile or too steep tile
            p_mapel->Flags2 &= ~MEF2_Unkn04;
            if (compute_map_tile_is_blocking_walk(tile_x, tile_z))
                p_mapel->Flags2 |= MEF2_Unkn04;
        }
    }
}

TbBool map_floor_is_water(MapCoord cor_x, MapCoord cor_z)
{
    ushort textr;

    textr = floor_texture_at_point(cor_x, cor_z);
    if ((get_my_texture_bits(textr) & 0x02) != 0)
        return true;
    return false;
}

TbBool map_floor_is_sludge(MapCoord cor_x, MapCoord cor_z)
{
    ushort textr;

    textr = floor_texture_at_point(cor_x, cor_z);
    if (my_texture_is_only_using_index(textr, 29))
        return true;
    return false;
}

void quick_crater(int x, int z, int size)
{
    asm volatile ("call ASM_quick_crater\n"
        :  : "a" (x), "d" (z), "b" (size));
}

u32 map_distance_deltas_precise(int dt_x, int dt_y, int dt_z)
{
    return LbSqrL(dt_x * dt_x + dt_y * dt_y + dt_z * dt_z);
}

u32 map_distance_deltas_fast(int dt_x, int dt_y, int dt_z)
{
    u32 ln_x, ln_z, dist;

    ln_x = abs(dt_x);
    ln_z = abs(dt_z);
    if (ln_x >= ln_z)
        dist = ln_x + (ln_z >> 2) + (ln_z >> 3) + (ln_z >> 6) + (ln_z >> 7) - (ln_x >> 5) - (ln_x >> 7);
    else
        dist = ln_z + (ln_x >> 2) + (ln_x >> 3) + (ln_x >> 6) + (ln_x >> 7) - (ln_z >> 5) - (ln_z >> 7);

    return dist;
}

u32 map_distance_coords_fast(short pt1_x, short pt1_y, short pt1_z, short pt2_x, short pt2_y, short pt2_z)
{
    return map_distance_deltas_fast(pt2_x - pt1_x, pt2_y - pt1_y, pt2_z - pt1_z);
}

void map_limit_distance_to_target_fast(short base_x, short base_y, short base_z,
  short *targ_x, short *targ_y, short *targ_z, int range_limit)
{
    int dt_x, dt_y, dt_z;
    int dist;

    dt_x = *targ_x - base_x;
    dt_y = *targ_y - base_y;
    dt_z = *targ_z - base_z;
    dist = map_distance_deltas_fast(dt_x, dt_y, dt_z);
    if (dist <= range_limit)
        return;
    if (dist == 0)
        dist = 1;
    *targ_x = base_x + dt_x * range_limit / dist;
    *targ_y = base_y + dt_y * range_limit / dist;
    *targ_z = base_z + dt_z * range_limit / dist;
}

/******************************************************************************/
