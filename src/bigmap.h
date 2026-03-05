/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bigmap.h
 *     Header file for bigmap.c.
 * @par Purpose:
 *     Routines implementing map elements reference lists.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BIGMAP_H
#define BIGMAP_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define MAP_TILE_WIDTH 128
#define MAP_TILE_HEIGHT 128

#define MAP_COORD_WIDTH TILE_TO_MAPCOORD(MAP_TILE_WIDTH, 0)
#define MAP_COORD_HEIGHT TILE_TO_MAPCOORD(MAP_TILE_HEIGHT, 0)

/** Convert precise map coordinate to normal map coordinate.
 */
#define PRCCOORD_TO_MAPCOORD(pcor) ((pcor) >> 8)

/** Convert precise map Y coordinate to normal engine coordinate.
 * For unknown reason, Y has often different shift than other coords..
 */
#define PRCCOORD_TO_YCOORD(pcor) ((pcor) >> 5)

/** Convert normal map coordinate to precise map coordinate.
 */
#define MAPCOORD_TO_PRCCOORD(mcor, ppos) (((mcor) << 8) + (ppos))

/** Convert normal map coordinate to tile no.
 */
#define MAPCOORD_TO_TILE(mcor) ((mcor) >> 8)

/** Convert tile(block) no to normal map coordinate.
 */
#define TILE_TO_MAPCOORD(tile, mpos) (((tile) << 8) + (mpos))

/** Length of the spiral, which speeds up searching for things in vicinity
 * of given map coords. It does not make sense to search this way if the
 * amount of tiles searched exceeds half of things limit; so this should
 * be set to no more than (THINGS_LIMIT+STHINGS_LIMIT)/2.
 */
#define SPIRAL_STEPS_COUNT   1700

/** Length of the vectors stored in angle_direction[].
 */
#define ANGLE_DIRECTION_DISTANCE 256

/* Linear distance equal to tan(MAX_WALKABLE_STEEPNESS) * 256
 */
#define MAX_WALKABLE_STEEPNESS_PER_256 98

enum MyMapElementFlags2 {
    MEF2_Unkn01     = 0x01,
    MEF2_Unkn02     = 0x02,
    MEF2_Unkn04     = 0x04,
    MEF2_Unkn08     = 0x08,
    MEF2_Unkn10     = 0x10,
    MEF2_Unkn20     = 0x20,
    MEF2_Unkn40     = 0x40,
    MEF2_Unkn80     = 0x80,
};

struct MyMapElement { // sizeof=18
  ushort Texture; /**< offs=0x00 Index of SingleFloorTexture for this tile, and texture flags. */
  ushort Shade;
  ubyte ShadeR;
  ubyte Flags;
  short Alt; /**< offs=0x06 Ground element altitude (at the beginning of the tile). */
  short Child; /**< offs=0x08 Index of a first Thing in a chain of things located on this element. */
  ushort ColHead; /**< offs=0x0A Index of ColVectList for this tile. */
  ushort Ambient; /**< offs=0x0C Background illumination. */
  ubyte Zip;
  ubyte Flags2;
  ushort ColumnHead; /**< offs=0x10 Index of ColColumn for this tile, storing collision properties. */
};

struct MyMapElementOldV7 { // sizeof=24
  ushort Texture; /**< offs=0x00 Index of SingleFloorTexture for this tile, and texture flags. */
  ushort Shade;
  ubyte ShadeR;
  ubyte Flags;
  ushort mefield_6;
  ushort mefield_8;
  short Alt; /**< offs=0x0A Ground element altitude. */
  ubyte Zip;
  ubyte Flags2;
  short Child; /**< offs=0x0E Index of a first Thing in a chain of things located on this element. */
  ushort ColHead; /**< offs=0x10 Index of ColVectList for this tile. */
  ushort Ambient;
  ushort mefield_14;
  ushort ColumnHead; /**< offs=0x16 Index of ColColumn for this tile, storing collision properties. */
};

struct MapOffset {
  sbyte v;
  sbyte h;
  short both;
};

struct ColColumn { // sizeof=16
    uint QBits[4];
};

struct Direction {
  short DiX;
  short DiY;
};

#pragma pack()
/******************************************************************************/
extern struct MyMapElement *game_my_big_map;

extern struct ColColumn *game_col_columns;
extern ushort next_col_column;

extern struct MapOffset spiral_step[SPIRAL_STEPS_COUNT];
extern ushort dist_tiles_to_spiral_step[MAP_TILE_WIDTH];
extern ushort spiral_dist_tiles_limit;

extern const struct Direction angle_direction[];

/** Limit the map coordinates boundaries of valid map positions and ranges of data types used.
 *
 * @return Returns if the coordinates were altered (or would have been altered if not NULL).
 */
TbBool map_coords_limit(MapCoord *cor_x, MapCoord *cor_y, MapCoord *cor_z, long map_x, long map_y, long map_z);

void clear_mapwho_on_whole_map(void);
void refresh_old_my_big_map_format(struct MyMapElement *p_mapel,
 struct MyMapElementOldV7 *p_oldmapel, u32 fmtver);
short get_mapwho_thing_index(short tile_x, short tile_z);
void init_search_spiral(void);
int alt_at_point(short x, short z);
int alt_at_point_under_height(int cor_x, int cor_z, int h);
ushort floor_texture_at_point(MapCoord cor_x, MapCoord cor_z);

ubyte map_coord_to_collision_qbit_index(short x, short z);
void set_mapel_col_columns(struct MyMapElement *p_mapel, short setbit, ushort qb);

/** Computes length of vector defined by given 3 coordinates.
 * Uses some simplifications, but is fast. Requires the resulting
 * length to fit in 32-bit value, but accepts both map coords and precise coords.
 */
u32 map_distance_deltas_fast(int dt_x, int dt_y, int dt_z);

u32 map_distance_deltas_precise(int dt_x, int dt_y, int dt_z);

/** Computes length of vector defined by two 3D points.
 * Uses some simplifications, but is fast. Requires the given values
 * to be map coords - they can not exceed 15-bit.
 */
u32 map_distance_coords_fast(short pt1_x, short pt1_y, short pt1_z,
  short pt2_x, short pt2_y, short pt2_z);

/** Given two points, alters the target one to be within given range.
 * Uses some simplifications, but is fast. Requires the givn values
 * to be map coords - they can not exceed 15-bit. Good for computing
 * speed or weapon firing distance, if the values are map coords.
 */
void map_limit_distance_to_target_fast(short base_x, short base_y, short base_z,
  short *targ_x, short *targ_y, short *targ_z, int range_limit);

/** Returns maximal change of altitude between corners of given tile.
 */
int alt_change_at_tile(short tile_x, short tile_z, int *change_xz);

/** Sets some of the map elements flags based on other properties.
 */
void update_map_flags(void);

TbBool map_floor_is_water(MapCoord cor_x, MapCoord cor_z);
TbBool map_floor_is_sludge(MapCoord cor_x, MapCoord cor_z);

/** Checks if a tile should not be allowed to walk on due to terrain.
 *
 * To do such check during gameplay, MapElement flags should be used - this one
 * is mostly to update these flags, if neccessary. Unless we really want to
 * disregard any changes to the computed flags - then using this makes sense.
 */
TbBool compute_map_tile_is_blocking_walk(short tile_x, short tile_z);

void quick_crater(int x, int z, int size);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
