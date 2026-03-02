/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file smemory.cpp
 *     Implementation of memory managing routines.
 * @par Purpose:
 *     Memory related routines - allocation and freeing of memory blocks.
 * @par Comment:
 *     Original games used different funcs for allocating low and extended mem.
 *     This is now outdated way, so functions here are simplified to originals.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 05 Nov 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bfmemory.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "bftypes.h"
#include "privbflog.h"

#if defined(WIN32)

// Selected declarations frow Win32 API - we don't want to use whole API
// since it influences everything

#ifndef VOID
#define VOID void
#endif

#ifndef WINBASEAPI
#ifdef __W32API_USE_DLLIMPORT__
#define WINBASEAPI DECLSPEC_IMPORT
#else
#define WINBASEAPI
#endif
#endif
#define WINAPI __stdcall

typedef unsigned long DWORD;

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS,*LPMEMORYSTATUS;

WINBASEAPI VOID WINAPI GlobalMemoryStatus(LPMEMORYSTATUS);

#endif // defined(WIN32)

#pragma pack(1)

struct TbMemoryAvailable { // sizeof=20
    TbMemSize TotalBytes; // offset=0
    TbMemSize TotalBytesFree; // offset=4
    TbMemSize TotalBytesUsed; // offset=8
    TbMemSize LargestBlock; // offset=12
    TbMemSize SmallestBlock; // offset=16
};

struct TbMemoryAvailable lbMemoryAvailable;
TbBool lbMemorySetup = false;

#if LB_MEMORY_ARENAS

/** Amount of memory blocks and arenas.
 */
#define TABLE_SIZE 256

typedef struct mem_block mem_block;

struct mem_block
{
    void    *Pointer;
    ulong Selector;
    ulong Size;
};

typedef struct mem_arena mem_arena;

struct mem_arena
{
    void    *Pointer;
    ulong Size;
    mem_arena *Child;
    mem_arena *Parent;
    ubyte  Used;
    /** Index in memory_blocks[] of the section containing this areana. */
    ubyte  Section;
};

mem_block memory_blocks[TABLE_SIZE];
mem_arena memory_arenas[TABLE_SIZE];

#endif

#pragma pack()

#if LB_MEMORY_ARENAS

/** @internal
 * Provide the amount of memory blocks currently in use.
 */
static size_t get_block_count(void)
{
    size_t n;

    for (n = 0; n < TABLE_SIZE; n++)
    {
        if (memory_blocks[n].Pointer == NULL)
            return n;
    }

    return TABLE_SIZE;
}

/** @internal
 * Fill memory arenas based on memory blocks.
 * Make each subsequent arena a child of previous one.
 */
static void initialise_block_nodes(void)
{
    size_t n;
    size_t count;

    count = get_block_count();

    for (n = 0; n < count; n++)
    {
        memory_arenas[n].Pointer  = memory_blocks[n].Pointer;
        memory_arenas[n].Size = memory_blocks[n].Size;
        memory_arenas[n].Section = n;

        if (n != 0)
            memory_arenas[n].Parent = &memory_arenas[n - 1];
        else
            memory_arenas[n].Parent = NULL;

        if (n + 1 != count)
            memory_arenas[n].Child = &memory_arenas[n + 1];
        else
            memory_arenas[n].Child = NULL;
    }
}

/** @internal
 * Split given arena, creating its child of given size.
 */
TbBool split_arena(mem_arena *arena, size_t size)
{
    mem_arena *curarena;
    ubyte sect;
    mem_arena *charena;
    ulong n;

    if (size == arena->Size) {
        arena->Used = 1;
        return true;
    }

    for (n = 0; n < TABLE_SIZE; n++)
    {
        curarena = &memory_arenas[n];
        if (curarena->Size == 0)
          break;
    }
    if (n == TABLE_SIZE) {
        return false;
    }
    curarena->Size = arena->Size - size;
    curarena->Pointer = (char *)arena->Pointer + size;
    arena->Used = 1;
    sect = arena->Section;
    arena->Size = size;
    curarena->Section = sect;
    curarena->Child = arena->Child;
    curarena->Parent = arena;
    charena = arena->Child;
    if (charena != NULL)
        charena->Parent = curarena;
    arena->Child = curarena;
    return true;
}

/** @internal
 * Merge arena with its parent.
 */
static void merge_arena_parent(mem_arena *arena)
{
    mem_arena *chlarena;

    chlarena = arena->Child;
    if (chlarena != NULL)
        chlarena->Parent = arena->Parent;
    arena->Parent->Child = arena->Child;
    arena->Parent->Size += arena->Size;
    arena->Size = 0;
}

/** @internal
 * Delete the given arena, adding its size to the parent.
 */
void delete_arena(mem_arena *arena)
{
    mem_arena *pararena;

    // Cannot delete the initial arena
    if (arena->Parent == NULL)
        return;
    arena->Used = 0;
    // If the arenas represent the same mem section, and parent is unused,
    // then merge with it
    pararena = arena->Parent;
    if (arena->Section == pararena->Section && !pararena->Used)
    {
        merge_arena_parent(arena);
    }
}

#endif

void * LbMemoryAllocLow(TbMemSize size)
{
    void *ptr;
    unsigned long algn_size;
#if LB_MEMORY_ARENAS
    mem_arena *curarena;
    unsigned long last_size;
    mem_arena *splarena;
    int sect;
#endif

    LbMemorySetup();
#if LB_MEMORY_ARENAS
    algn_size = (size + 0x0f) & ~0x0f;
    last_size = -1;
    splarena = NULL;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (algn_size <= curarena->Size && last_size > curarena->Size)
        {
            if ( !curarena->Used )
            {
                sect = curarena->Section;
                if (memory_blocks[sect].Selector != 0)
                {
                  splarena = curarena;
                  last_size = curarena->Size;
                }
            }
        }
    }
    if (splarena == NULL || !split_arena(splarena, algn_size)) {
        LOGERR("failed memory allocation of %lu bytes", (ulong)size);
        ptr = NULL;
    } else {
        ptr = splarena->Pointer;
    }
#else
    algn_size = size;
    ptr = malloc(algn_size);
#endif
    LbMemoryCheck();
    if (ptr != NULL)
        memset(ptr, 0, algn_size);
    return ptr;
}

void * LbMemoryAlloc(TbMemSize size)
{
    void *ptr;
    unsigned long algn_size;
#if LB_MEMORY_ARENAS
    mem_arena *curarena;
    unsigned long last_size;
    mem_arena *splarena;
    int sect;
#endif

    LbMemorySetup();
#if LB_MEMORY_ARENAS
    algn_size = (size + 0x03) & ~0x03;
    last_size = -1;
    splarena = NULL;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (algn_size <= curarena->Size && last_size > curarena->Size)
        {
            if ( !curarena->Used )
            {
                sect = curarena->Section;
                if (memory_blocks[sect].Selector == 0)
                {
                    splarena = curarena;
                    last_size = curarena->Size;
                }
            }
        }
    }
    if (splarena == NULL || !split_arena(splarena, algn_size)) {
        return LbMemoryAllocLow(algn_size);
    }
    ptr = splarena->Pointer;
#else
    algn_size = size;
    ptr = malloc(algn_size);
#endif
    LbMemoryCheck();
    if (ptr != NULL)
        memset(ptr, 0, algn_size);
    LOGNO("memory allocation of %lu bytes ptr=0x%p", algn_size, ptr);
    return ptr;
}

TbResult LbMemoryFree(void *mem_ptr)
{
#if LB_MEMORY_ARENAS
    mem_arena *curarena;
    TbBool found;
#endif

    if (mem_ptr == NULL)
        return Lb_FAIL;

#if LB_MEMORY_ARENAS
    curarena = memory_arenas;
    found = 0;
    while (curarena != NULL)
    {
        if (mem_ptr == curarena->Pointer) {
            found = 1;
            curarena->Used = 0;
            break;
        }
        curarena = curarena->Child;
    }
    if (found != 1)
        return Lb_FAIL;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (!curarena->Used)
            delete_arena(curarena);
    }
#else
    free(mem_ptr);
#endif
    LbMemoryCheck();
    return Lb_SUCCESS;
}

TbResult LbMemoryCheck(void)
{
#if LB_MEMORY_ARENAS
    mem_arena *arena;
    unsigned int csize;

    lbMemoryAvailable.TotalBytes = 0;
    lbMemoryAvailable.TotalBytesFree = 0;
    lbMemoryAvailable.TotalBytesUsed = 0;
    lbMemoryAvailable.LargestBlock = 0;
    lbMemoryAvailable.SmallestBlock = -1;
    for (arena = memory_arenas; arena != NULL; arena = arena->Child)
    {
        if (arena->Used) {
            csize = arena->Size;
            lbMemoryAvailable.TotalBytesUsed += csize;
            lbMemoryAvailable.TotalBytes += csize;
        } else {
            csize = arena->Size;
            lbMemoryAvailable.TotalBytesFree += csize;
            if (csize > lbMemoryAvailable.LargestBlock)
                lbMemoryAvailable.LargestBlock = csize;
            if (arena->Size < lbMemoryAvailable.SmallestBlock)
                lbMemoryAvailable.SmallestBlock = arena->Size;
            lbMemoryAvailable.TotalBytes += arena->Size;
        }
    }
    lbMemoryAvailable.TotalBytes &= ~0x03;
    lbMemoryAvailable.TotalBytesFree &= ~0x03;
    lbMemoryAvailable.LargestBlock &= ~0x03;
    lbMemoryAvailable.TotalBytesUsed &= ~0x03;
    lbMemoryAvailable.SmallestBlock &= ~0x03;
#else
    struct _MEMORYSTATUS msbuffer;

    msbuffer.dwLength = 32;
    GlobalMemoryStatus(&msbuffer);
    lbMemoryAvailable.TotalBytes = msbuffer.dwTotalPhys;
    // We do not care for the rest - without arenas, memory usage
    // is not tracked on application level
    lbMemoryAvailable.TotalBytesFree = msbuffer.dwAvailPhys;
    lbMemoryAvailable.TotalBytesUsed = 0;
    lbMemoryAvailable.LargestBlock = msbuffer.dwAvailPhys;
    lbMemoryAvailable.SmallestBlock = 4;
#endif
  return Lb_SUCCESS;
}

TbResult LbMemorySetup(void)
{
    if (lbMemorySetup)
        return Lb_OK;

#if LB_MEMORY_ARENAS
    memset(&memory_blocks, 0, sizeof (memory_blocks));
    memset(&memory_arenas, 0, sizeof (memory_arenas));

    // Simulated extended memory block
    memory_blocks[0].Pointer = malloc(LB_MEMORY_AVAILABLE * 1024);
    memory_blocks[0].Size    = LB_MEMORY_AVAILABLE * 1024;
    memory_blocks[0].Selector = 0;

    // Simulated conventional memory block
    memory_blocks[1].Pointer = malloc(LB_MEMORY_LOW_AVAILABLE * 1024);
    memory_blocks[1].Size    = LB_MEMORY_LOW_AVAILABLE * 1024;
    memory_blocks[1].Selector = 1;

    assert(memory_blocks[0].Pointer != NULL);
    assert(memory_blocks[1].Pointer != NULL);

    initialise_block_nodes();
#endif

    lbMemorySetup = true;
    return Lb_SUCCESS;
}

TbResult LbMemoryReset(void)
{
#if LB_MEMORY_ARENAS
    mem_block *cblock;
#endif
    if (!lbMemorySetup)
        return Lb_OK;

#if LB_MEMORY_ARENAS
    cblock = memory_blocks;
    while (cblock->Size != 0)
    {
        if (cblock->Selector == 1) {
# if defined(DOS)||defined(GO32)
            dos_free(cblock->Selector);
# else
            free(cblock->Pointer);
# endif
        } else {
            free(cblock->Pointer);
        }
        cblock->Size = 0;
        cblock->Pointer = 0;
        cblock->Selector = 0;
        ++cblock;
    }
#endif

    lbMemorySetup = false;
    return Lb_SUCCESS;
}

TbResult LbMemoryGrow(void **ptr, TbMemSize size)
{
    unsigned long algn_size;
#if LB_MEMORY_ARENAS
    mem_arena *chlarena;
    unsigned long join_size;
    mem_arena *curarena;
    TbBool found;

    algn_size = (size + 0x03) & ~0x03;
    found = 0;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (curarena->Pointer == *ptr)
        {
            if ( algn_size <= curarena->Size )
                return algn_size == curarena->Size;
            curarena->Used = 0;
            if ( !curarena->Child )
                return Lb_FAIL;
            chlarena = curarena->Child;
            if ( chlarena->Used )
                return Lb_FAIL;
            join_size = curarena->Size + chlarena->Size;
            if ( join_size <= algn_size )
            {
              if ( join_size < algn_size )
                return Lb_FAIL;
              delete_arena(chlarena);
            }
            else
            {
              chlarena->Size = join_size - algn_size;
              curarena->Size = algn_size;
              curarena->Child->Used = 1;
            }
            curarena->Used = 1;
            found = 1;
            break;
        }
    }

    if (!found)
        return Lb_FAIL;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (!curarena->Used)
            delete_arena(curarena);
    }
#else
    algn_size = size;
    *ptr = realloc(*ptr, algn_size);
#endif
    LbMemoryCheck();
    return Lb_SUCCESS;
}

TbResult LbMemoryShrink(void **ptr, TbMemSize size)
{
    unsigned long algn_size;
#if LB_MEMORY_ARENAS
    mem_arena *curarena;
    TbBool found;

    algn_size = (size + 0x03) & ~0x03;
    found = 0;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (curarena->Pointer == *ptr)
        {
            if (algn_size >= curarena->Size) {
                if (algn_size != curarena->Size)
                    return Lb_FAIL;
                return 1;
            }
            curarena->Used = 0;
            split_arena(curarena, algn_size);
            found = 1;
            break;
        }
    }
    if (!found)
        return Lb_FAIL;
    for (curarena = memory_arenas; curarena != NULL; curarena = curarena->Child)
    {
        if (!curarena->Used)
            delete_arena(curarena);
    }
#else
    algn_size = size;
    *ptr = realloc(*ptr, algn_size);
#endif
    LbMemoryCheck();
    return Lb_SUCCESS;
}

/******************************************************************************/
