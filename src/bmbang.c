/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bmbang.c
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
#include "bmbang.h"

#include "enginshrapn.h"

#include "bigmap.h"
#include "thing.h"
#include "swlog.h"
/******************************************************************************/

#pragma pack(1)

struct CircleDetail {
    s32 dangle;
    s32 babynumber;
};

struct SphereDetail {
    s32 dangley;
    s32 babyshift;
    s32 babynumber;
};

struct BangDetail {
    struct CircleDetail circle[4];
    struct SphereDetail sphere[4];
    u32 mask;
};

#pragma pack()

void bang_init(void)
{
#if 0
    asm volatile ("call ASM_bang_init\n"
        :  :  : "eax" );
#endif
    shrapnel_init();
    phwoar_init();
}

/** Fill in the type-specific fields of a freshly allocated Phwoar entry.
 */
void fillin_phwoar_type_info(struct Phwoar *p_phwoar)
{
    asm volatile ("call ASM_fillin_phwoar_type_info\n"
        : : "a" (p_phwoar) : "cc", "memory");
}

void new_bang(int x, int y, int z, int type, int owner, int c)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[2];

    stkargs[0] = (int)(intptr_t)owner;
    stkargs[1] = (int)(intptr_t)c;

    asm volatile (
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_new_bang\n"
        : : "a" (x), "d" (y), "b" (z), "c" (type), "S" (stkargs)
        : "cc", "memory");
}

void bang_new5(int x, int y, int z, int type, int owner)
{
    new_bang(x, y, z, type, owner, 0);
}

void bang_new4(int x, int y, int z, int type)
{
    new_bang(x, y, z, type, 0, 0);
}

ubyte unused_func_026(ubyte a1)
{
    ubyte ret;
    asm volatile (
      "call ASM_unused_func_026\n"
        : "=r" (ret) : "a" (a1));
    return ret;
}

void unused_func_025(short a1, short a2, short a3)
{
    asm volatile (
      "call ASM_unused_func_025\n"
        : : "a" (a1), "d" (a2), "b" (a3));
}

void do_shockwave(int x, int y, int z, int radius, int intensity, struct Thing *p_owner)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[2];

    stkargs[0] = (int)(intptr_t)intensity;
    stkargs[1] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_do_shockwave\n"
        : : "a" (x), "d" (y), "b" (z), "c" (radius), "S" (stkargs)
        : "cc", "memory");
}


void do_shockwave_building(int dist, int intensity, struct Thing *p_thing, struct Thing *p_owner)
{
    asm volatile (
      "call ASM_do_shockwave_building\n"
        : : "a" (dist), "d" (intensity), "b" (p_thing), "c" (p_owner));
}

void do_shockwave_vehicle(int dx, int dz, int dist, int intensity,
  struct Thing *p_vevicle, struct Thing *p_owner)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[2];

    stkargs[0] = (int)(intptr_t)p_vevicle;
    stkargs[1] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_do_shockwave_vehicle\n"
        : : "a" (dx), "d" (dz), "b" (dist), "c" (intensity), "S" (stkargs)
        : "cc", "memory");
}

void do_shockwave_person(int dx, int dz, int dist, int intensity,
  struct Thing *p_person, struct Thing *p_owner)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[2];

    stkargs[0] = (int)(intptr_t)p_person;
    stkargs[1] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_do_shockwave_person\n"
        : : "a" (dx), "d" (dz), "b" (dist), "c" (intensity), "S" (stkargs)
        : "cc", "memory");
}

void do_shockwave_scale_effect(int dx, int dz, int dist, int intensity,
  struct SimpleThing *p_sthing, struct Thing *p_owner)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[2];

    stkargs[0] = (int)(intptr_t)p_sthing;
    stkargs[1] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_do_shockwave_scale_effect\n"
        : : "a" (dx), "d" (dz), "b" (dist), "c" (intensity), "S" (stkargs)
        : "cc", "memory");
}
/******************************************************************************/
