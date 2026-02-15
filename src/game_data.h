/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_data.h
 *     Header file for game_data.c.
 * @par Purpose:
 *     Structures and functions defining data used by the game.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2020 - 16 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum DirPlacement {
    DirPlace_Data = 0,	/**< Normal data directory; files for which read speed can be low */
    DirPlace_QData,		/**< Quick data dir; files expected to be accessible without much delay */
    DirPlace_Maps,		/**< Map files storage directory */
    DirPlace_Levels,	/**< Level files storage directory */
    DirPlace_Intro,		/**< Directory with game intro animations */
    DirPlace_Sound,		/**< Directory with sound banks and sound driver config */
    DirPlace_Equip,		/**< Normal data directory for equip/mods frontend screen */
    DirPlace_QEquip,	/**< Quick data directory for equip/mods  frontend screen */
    DirPlace_LangData,	/**< Directory with language-specific data */
    DirPlace_Config,	/**< Directory with configuration files which can be altred by user */
    DirPlace_Savegame,	/**< Saved games directory; needs write access */
    DirPlace_Scrnshots,	/**< Screenshots and movies directory; needs write access */
    DirPlace_None,
};

#pragma pack(1)

typedef struct {
    char *directory;
    TbBool use_cd;
} PathInfo;

typedef struct {
    char *Name;
    void **BufferPtr;
    ushort ESize;
    long N;
    void *PrivBuffer;
    long dum2;
    ubyte dum3;
} MemSystem;

#pragma pack()

extern PathInfo game_dirs[];
extern char cd_drive[52];
extern char language_3str[4];

extern MemSystem mem_game[];

/** Scratch memory, reused for different purposes in different parts of the game.
 */
extern void *scratch_malloc_mem;
extern u32 scratch_malloc_size;

/******************************************************************************/

/** Returns absolute path to user files directory for the application.
 */
const char *GetDirectoryUser(void);

/** Returns path (absolute or relative) to HDD-stored files for the application.
 */
const char *GetDirectoryHdd(void);

/** Returns absolute path to CD-stored files for the application.
 */
const char *GetDirectoryCd(void);

/** Prepares global base paths and sets up path transformation on file open.
 *
 * After this call, any files to be opened can use "/" as separator and
 * relative paths within the game folder.
 */
void setup_file_names(void);

/** Final part of setup_file_names(), called after language is set.
 */
void setup_language_file_names(void);

int get_highest_used_packet_record_no(int campgn, int missi);
void get_packet_record_fname(char *fname, int campgn, int missi, int file_no);
void get_user_settings_fname(char *fname, const char *name);
void get_saved_game_fname(char *fname, ushort slot);

void adjust_memory_use(void);
TbResult init_memory(MemSystem *mem_table);
int get_memory_ptr_allocated_count(void **mgptr);
TbResult propagate_memory_sizes(void);
void init_things_memory_with_user_heap(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
