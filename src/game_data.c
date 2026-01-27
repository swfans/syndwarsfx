/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_data.c
 *     Memory managing routines.
 * @par Purpose:
 *     Structures and functions defining data used by the game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Feb 2020 - 16 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "game_data.h"

#include <assert.h>
#include "bffile.h"
#include "bfdir.h"
#include "bffnuniq.h"
#include "bfstrut.h"

#include "campaign.h"
#include "command.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "enginfloor.h"
#include "enginlights.h"
#include "enginpriobjs.h"
#include "enginpritxtr.h"
#include "enginprops.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintxtrmap.h"
#include "bigmap.h"
#include "game_options.h"
#include "game.h"
#include "lvfiles.h"
#include "lvobjctv.h"
#include "lvwalk.h"
#include "swlog.h"
#include "tngcolisn.h"
#include "osunix.h"
#include "oswindws.h"
#include "vehtraffic.h"
#include "dos.h"

static char data_path_user[DISKPATH_SIZE] = "";
static char data_path_hdd[DISKPATH_SIZE] = "";
static char game_dir_language[64] = "language/eng";

u32 scratch_malloc_size = 0;

/******************************************************************************/

MemSystem mem_game[] = {
  { "my_big_map",		(void **)&game_my_big_map,		18u, 16513, 0, 0, 0 },
  { "textures",			(void **)&game_textures,		18u, 4512, 0, 0, 0 },
  { "face_textures",	(void **)&game_face_textures,	16u, 4000, 0, 0, 0 },
  { "object_points",	(void **)&game_object_points,	10u, 20000, 0, 0, 0 },
  { "object_faces",		(void **)&game_object_faces,	32u, 15000, 0, 0, 0 },
  { "objects",			(void **)&game_objects,			36u, 2000, 0, 0, 0 },
  { "quick_lights",		(void **)&game_quick_lights,	6u, 64000, 0, 0, 0 },
  { "full_lights",		(void **)&game_full_lights,		32u, 4000, 0, 0, 0 },
  { "normals",			(void **)&game_normals,			16u, 10000, 0, 0, 0 },
  { "object_faces4",	(void **)&game_object_faces4,	40u, 10000, 0, 0, 0 },
  { "anim_tmaps",		(void **)&game_anim_tmaps,		54u, 66, 0, 0, 0 },
  { "traffic_nodes",	(void **)&game_traffic_nodes,	36u, 500, 0, 0, 0 },
  { "light_commands",	(void **)&game_light_commands,	36u, 1000, 0, 0, 0 },
  { "col_vects_list",	(void **)&game_col_vects_list,	6u, 26000, 0, 0, 0 },
  { "col_vects",		(void **)&game_col_vects,		14u, 13000, 0, 0, 0 },
  { "walk_headers",		(void **)&game_walk_headers,	4u, 3000, 0, 0, 0 },
  { "walk_items",		(void **)&game_walk_items,		2u, 18000, 0, 0, 0 },
  { "col_columns",		(void **)&game_col_columns,		16u, 8000, 0, 0, 0 },
  { "prim4_textures",	(void **)&prim4_textures,		18u, 1200, 0, 0, 0 },
  { "prim_face_textures", (void **)&prim_face_textures,	16u, 500, 0, 0, 0 },
  { "prim_object_points", (void **)&prim_object_points,	10u, 2000, 0, 0, 0 },
  { "prim_object_faces", (void **)&prim_object_faces,	32u, 800, 0, 0, 0 },
  { "prim_object_faces4", (void **)&prim_object_faces4,	40u, 1300, 0, 0, 0 },
  { "prim_objects",		(void **)&prim_objects,			36u, 60, 0, 0, 0 },
  { "special_object_faces", (void **)&game_special_object_faces, 32u, 1400, 0, 0, 0 },
  { "special_object_faces4",(void **)&game_special_object_faces4, 40u, 1400, 0, 0, 0 },
  { "floor_tiles",		(void **)&game_floor_tiles,		39u, 18000, 0, 0, 0 },
  { "used_objectives",	(void **)&game_used_objectives,	32u, 1200, 0, 0, 0 },
  { "objectives",		(void **)&game_objectives,		32u, 1200, 0, 0, 0 },
  { "user_heap",		(void **)&game_user_heap,		1u, 292228, 0, 0, 0 },
  { "screen_point_pool",(void **)&game_screen_point_pool,8u, 45000, 0, 0, 0 },
  { "draw_list",		(void **)&game_draw_list,		5u, 45001, 0, 0, 0 },
  { "sort_sprites",		(void **)&game_sort_sprites,	16u, 4001, 0, 0, 0 },
  { "sort_lines",		(void **)&game_sort_lines,		11u, 4001, 0, 0, 0 },
  { "commands",			(void **)&game_commands,		32u, 3100, 0, 0, 0 },
  { "bez edit",			(void **)&bezier_pts,			28u, 2000, 0, 0, 0 },
  { "spare map buffer",	(void **)&spare_map_buffer,		1u, 101, 0, 0, 1 },
  { "used_lvl_objectives", (void **)&game_used_lvl_objectives,32u, 20, 0, 0, 0 },
  { "level_miscs",		(void *)&game_level_miscs,		22u, 200, 0, 0, 0 },
  { NULL,				NULL,							0u, 0, 0, 0, 0 }
};

PathInfo game_dirs[] = {
  {"data",		1},
  {"qdata",		0},
  {"maps",		1},
  {"levels",	1},
  {"intro",		1},
  {"sound",		1},
  {"data/equip",1},
  {"qdata/equip",0},
  {game_dir_language,	0},
  {"conf",		0},
  {"qdata/savegame",	0},
  {"qdata/screenshots",0},
  {NULL,		0},
};

void SetupBflibScreenshotsDir(void)
{
    PathInfo *pinfo;

    pinfo = &game_dirs[DirPlace_Scrnshots];
    LbSetImagesDirectory(pinfo->directory);
}

const char *
GetDirectoryUser(void)
{
    if (data_path_user[0] == '\0')
    {
        PathInfo *pinfo;

        if (!sys_get_user_path(data_path_user, sizeof(data_path_user)))
        {
            snprintf(data_path_user, sizeof(data_path_user), ".");
        }
        LOGSYNC_F("Dir for user files '%s'", data_path_user);

        pinfo = &game_dirs[DirPlace_Savegame];
        LbDirectoryMake(pinfo->directory, true);

        pinfo = &game_dirs[DirPlace_Scrnshots];
        LbDirectoryMake(pinfo->directory, true);
    }
    return data_path_user;
}

const char *
GetDirectoryHdd(void)
{
    if (data_path_hdd[0] == '\0')
    {
        if (!sys_get_data_path(data_path_hdd, sizeof(data_path_hdd)))
        {
            snprintf(data_path_hdd, sizeof(data_path_hdd), "%s", ".");
        }
        LOGSYNC_F("Dir with HDD data '%s'", data_path_hdd);
    }
    return data_path_hdd;
}


const char *
GetDirectoryCd(void)
{
    return cd_drive;
}

int
GetDirPlaceFromPath(const char *fname)
{
    char *last_fs_sep;
    int path_len;
    int dir_place;

    last_fs_sep = strrchr(fname, '/');
    if (last_fs_sep == NULL)
        last_fs_sep = strrchr(fname, '\\');
    if (last_fs_sep == NULL)
        return DirPlace_None;
    path_len = (int)(last_fs_sep - fname);
    // First pass - try matching whole path
    for (dir_place = 0; dir_place < (int)(sizeof(game_dirs)/sizeof(game_dirs[0])); dir_place++)
    {
        PathInfo *pinfo;
        pinfo = &game_dirs[dir_place];
        if (pinfo->directory != NULL) {
            if (strncasecmp(fname, pinfo->directory, path_len) == 0)
                return dir_place;
        }
    }
    // Second pass - match starting part of the path
    for (dir_place = sizeof(game_dirs)/sizeof(game_dirs[0]) - 1; dir_place >= 0; dir_place--)
    {
        PathInfo *pinfo;
        pinfo = &game_dirs[dir_place];
        if (pinfo->directory != NULL) {
            path_len = strlen(pinfo->directory);
            if (strncasecmp(fname, pinfo->directory, path_len) == 0) {
                if ((fname[path_len] == '/') || (fname[path_len] == '\\'))
                    return dir_place;
            }
        }
    }
    return DirPlace_None;
}

static void
replace_fs_separator_to_native(char *path)
{
    size_t n;
    for (n = 0; path[n] != '\0'; n++)
    {
#if FS_SEP == '\\'
        if (path[n] == '/')
#else
        if (path[n] == '\\')
#endif
            path[n] = FS_SEP;
    }
}

void SyndFileNameTransform(char *out_fname, const char *inp_fname)
{
    char fs_fname[DISKPATH_SIZE];
    int dir_place;
    const char *base_dir;

    // Figure out whether the base folder should be data folder, user folder or CD
    dir_place = GetDirPlaceFromPath(inp_fname);
    if ((dir_place == DirPlace_Savegame) || (dir_place == DirPlace_Scrnshots)) {
        base_dir = GetDirectoryUser();
    }
    else if (dir_place != DirPlace_None) {
        if (game_dirs[dir_place].use_cd) {
            base_dir = GetDirectoryCd();
        } else {
            base_dir = GetDirectoryHdd();
        }
    } else {
        base_dir = GetDirectoryHdd();
    }

    strncpy(fs_fname, inp_fname, DISKPATH_SIZE);
    // Switch the input folder separators to proper ones for current os
    replace_fs_separator_to_native(fs_fname);
    // Add base path only if the input one is not absolute
    if (fs_fname[0] == FS_SEP || (strlen(fs_fname) >= 2 && fs_fname[1] == ':')) {
        snprintf(out_fname, FILENAME_MAX-1, "%s", fs_fname);
    } else {
        snprintf(out_fname, FILENAME_MAX-1, "%s" FS_SEP_STR "%s", base_dir, fs_fname);
    }
}

void setup_file_names(void)
{
#if LB_FILENAME_TRANSFORM
    lbFileNameTransform = SyndFileNameTransform;
#endif
    SetupBflibScreenshotsDir();
    // This fills the path variable; for user, it also creates the folder
    GetDirectoryHdd();
    GetDirectoryUser();
}

void setup_language_file_names(void)
{
    snprintf(game_dir_language, sizeof(game_dir_language), "language/%s", language_3str);
}

int get_highest_used_packet_record_no(int campgn, int missi)
{
    char fname[DISKPATH_SIZE];
    PathInfo *pinfo;

    pinfo = &game_dirs[DirPlace_Savegame];
    sprintf(fname, "%s/rc%02dm%03d.s*", pinfo->directory, campgn, missi);
    return get_highest_file_no(fname, strrchr(fname, '*') - fname, pinfo->directory);
}

void get_packet_record_fname(char *fname, int campgn, int missi, int file_no)
{
    PathInfo *pinfo;

    pinfo = &game_dirs[DirPlace_Savegame];
    sprintf(fname, "%s/rc%02dm%03d.s%02d", pinfo->directory, campgn, missi, file_no);
}

void get_user_settings_fname(char *fname, const char *name)
{
    PathInfo *pinfo;

    pinfo = &game_dirs[DirPlace_Savegame];
    if (name[0] == '\0')
        name = "default";
    sprintf(fname, "%s/%.8s.ini", pinfo->directory, name);
    LbStringToLower(fname+strlen(pinfo->directory));
}

void get_saved_game_fname(char *fname, ushort slot)
{
    PathInfo *pinfo;

    pinfo = &game_dirs[DirPlace_Savegame];
    if (slot == 0)
        sprintf(fname, "%s/synwarsm.sav", pinfo->directory);
    else if (slot < 9)
        sprintf(fname, "%s/synwars%d.sav", pinfo->directory, (int)slot - 1);
    else
        sprintf(fname, "%s/swars%03d.sav", pinfo->directory, (int)slot - 1);
}

int get_memory_ptr_index(void **mgptr)
{
    int i;
    for (i = 0; mem_game[i].Name != NULL; i++)
    {
        if (mem_game[i].BufferPtr == mgptr)
            return i;
    }
    return -1;
}

int get_memory_ptr_allocated_count(void **mgptr)
{
    int i;
    for (i = 0; mem_game[i].Name != NULL; i++)
    {
        if (mem_game[i].BufferPtr == mgptr)
            return mem_game[i].N;
    }
    return -1;
}

TbBool mem_game_index_is_prim(int index)
{
    static int prim_first = 18;
    static int prim_last = 23;

    return (index >= prim_first && index <= prim_last);
}

/** Decrease the size of some arrays to reduce memory usage.
 */
void adjust_memory_use(void)
{
    int i;
    ulong total;

    total = 0;

    // Bezier points are only used when editing maps
    assert((i = get_memory_ptr_index((void **)&bezier_pts)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_objectives)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&spare_map_buffer)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_used_objectives)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_floor_tiles)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1124;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_draw_list)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 2500;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_sort_sprites)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_sort_lines)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 700;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_screen_point_pool)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 18000; // every face and every face4 needs a few
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&prim_object_points)) != -1);
    if ((ingame.LowerMemoryUse == 1) && (is_single_game || cmdln_param_bcg))
        mem_game[i].N = 2000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&prim_object_faces)) != -1);
    if ((ingame.LowerMemoryUse == 1) && (is_single_game || cmdln_param_bcg))
        mem_game[i].N = 2000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&prim_object_faces4)) != -1);
    if ((ingame.LowerMemoryUse == 1) && (is_single_game || cmdln_param_bcg))
        mem_game[i].N = 2000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_object_faces)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 11000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_object_faces4)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 11000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_objects)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1500;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_full_lights)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 1000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_col_vects_list)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 16000;
    total += mem_game[i].ESize * mem_game[i].N;

    assert((i = get_memory_ptr_index((void **)&game_col_vects)) != -1);
    if (ingame.LowerMemoryUse == 1)
        mem_game[i].N = 9000;
    total += mem_game[i].ESize * mem_game[i].N;

    if (ingame.LowerMemoryUse == 1)
    {
        engine_mem_alloc_size = (2000000 + total + 0xFFFF) & ~0xFFFF;
        game_perspective = 5;
    } else
    {
        engine_mem_alloc_size = (2850000 + total + 0xFFFF) & ~0xFFFF;
    }
    LOGSYNC("Set engine_mem_alloc_size=%u total=%lu", engine_mem_alloc_size, total);
}

uint memory_table_entries(MemSystem *mem_table)
{
    MemSystem *ment;
    uint i;

    i = 0;
    ment = mem_table;
    while (ment->BufferPtr != NULL) {
        ment++;
        i++;
    }
    return i;
}

TbResult init_memory(MemSystem *mem_table)
{
    MemSystem *ment;
    int mem_table_len;
    ulong totlen;
    ubyte *p;
    int i;
    ulong k;
    TbResult ret;

    ret = Lb_SUCCESS;
    totlen = 8192;
    mem_table_len = memory_table_entries(mem_table);

    if (engine_mem_alloc_ptr == NULL) {
        LOGWARN("Memory for engine not allocated; further init will not work correctly");
        ret = Lb_FAIL;
    }

    if (tmaps_extra_buf == NULL) {
        LOGWARN("Texture maps not allocated; cannor reuse their extra memory");
    }

    p = NULL;
    for (i = mem_table_len - 1; i >= 0; i--)
    {
        ment = &mem_table[i];
        if (i == 17)
            p = engine_mem_alloc_ptr + totlen;

        if (ment->PrivBuffer == NULL)
        {
            if ((engine_mem_alloc_size & 0x80000000) == 0)
              ment->PrivBuffer = engine_mem_alloc_ptr + totlen;
            else
              ret = Lb_FAIL;

            if (ment->N * (ulong)ment->ESize < tmaps_extra_len && !mem_game_index_is_prim(i))
            {
                // Reuse extra texture maps atlas memory
                ment->PrivBuffer = tmaps_extra_buf;
                k = ment->N * ment->ESize;
                k = (k + 4) & ~0x3;
                tmaps_extra_len -= k;
                tmaps_extra_buf += k;
            }
            else
            {
                k = ment->N * ment->ESize;
                k = (k + 4) & ~0x3;
                totlen += k;
                assert(totlen <= engine_mem_alloc_size);
            }
            *(ment->BufferPtr) = ment->PrivBuffer;
        }
    }
    if (p != NULL) {
        scratch_malloc_mem = p;
        assert((ubyte *)engine_mem_alloc_ptr + engine_mem_alloc_size > p);
        scratch_malloc_size = (ubyte *)engine_mem_alloc_ptr + engine_mem_alloc_size - p;
    } else {
        LOGWARN("Memory for scratch_malloc not prepared");
        scratch_malloc_size = 0;
        ret = Lb_FAIL;
    }

    memset(game_sort_sprites, 0, 0x20u);

    scratch_buf1 = *ment[23].BufferPtr; // prim_objects
    if (scratch_buf1 == NULL) {
        LOGWARN("Memory for scratch_buf1 not prepared");
        ret = Lb_FAIL;
    }

    if (ret == Lb_FAIL) {
        LOGSYNC("Memory init failed, game will exit");
        exit_game = 1;
    } else {
        LOGSYNC("Memory init performed");
    }

    return ret;
}

TbResult propagate_memory_sizes(void)
{
    TbResult ret;

    ret = Lb_SUCCESS;

    screen_points_limit = mem_game[30].N;
    assert(screen_points_limit > 0);
    draw_items_limit = mem_game[31].N;
    assert(draw_items_limit > 0);
    game_textures_limit = get_memory_ptr_allocated_count((void **)&game_textures);
    assert(game_textures_limit > 0);
    face_textures_limit = get_memory_ptr_allocated_count((void **)&game_face_textures);
    assert(face_textures_limit > 0);

    return ret;
}

/******************************************************************************/
