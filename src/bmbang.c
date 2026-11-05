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

#include "bfmath.h"
#include "bfutility.h"
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
#if 0
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
    return;
#endif
    short thing;
    struct SimpleThing *p_sthing;
    int level; // "phwoar type"/level parameter, stored into the phwoar entry
    int ang0, ang1, ang2; // quadrant angle-spread sizes (also the shrapnel counts)
    int fixed_dir; // if set, shrapnel fly in one fixed (not random) direction
    int fixed_vy; // the fixed vy value used when fixed_dir is set
    int state_override; // -1 if State keeps its default (0) value
    int total, i;
    ThingIdx tng;

    if (x < 0)
        return;
    if ((x >> 8) >= MAP_COORD_WIDTH)
        return;
    if (z < 0)
        return;
    if ((z >> 8) >= MAP_COORD_HEIGHT)
        return;
    if (phwoar_free == 0)
        return;
    c++;
    if (shrapnel_free == 0)
        return;

    // Light-only types
    switch (type)
    {
    case 0xa:
        new_bang(x, y, z, 0xb, owner, c);
        new_bang(x, y, z, 0xc, owner, c);
        create_temp_light(x, y + 0x6400, z, 0x80, 0x19, 0x80);
        return;
    case 0xf:
        new_bang(x, y, z, 0x10, owner, c);
        new_bang(x, y, z, 0x11, owner, c);
        new_bang(x, y, z, 0x12, owner, c);
        create_temp_light(x, y + 0x6400, z, 0x50, 0x14, 0x50);
        return;
    case 0x14:
        new_bang(x, y, z, 0x15, owner, c);
        new_bang(x, y, z, 0x16, owner, c);
        create_temp_light(x, y + 0x6400, z, 0x32, 0x5, 0x32);
        return;
    case 0x19:
        new_bang(x, y, z, 0x1a, owner, c);
        new_bang(x, y, z, 0x1b, owner, c);
        new_bang(x, y, z, 0x1c, owner, c);
        create_temp_light(x, y + 0xc800, z, 0x100, 0x5, 0xa);
        return;
    case 0x2d:
        new_bang(x, y, z, 0x2e, owner, c);
        new_bang(x, y, z, 0x2f, owner, c);
        new_bang(x, y, z, 0x30, owner, c);
        create_temp_light(x, y + 0x9600, z, 0x80, 0x19, 0x80);
        return;
    default:
        break;
    }

    tng = get_new_sthing();
    if (tng == 0)
        return;

    thing = tng;
    p_sthing = &sthings[thing];
    p_sthing->Type = SmTT_BANG;
    p_sthing->State = 0;
    p_sthing->X = x;
    p_sthing->Y = y;
    p_sthing->Z = z;
    p_sthing->Owner2 = owner;
    p_sthing->U.UBang.shrapnel = 0;
    p_sthing->U.UBang.phwoar = 0;

    ang0 = 0;
    ang1 = 0;
    ang2 = 0;
    fixed_dir = 0;
    fixed_vy = 0;
    state_override = -1;

    switch (type)
    {
    case 0xb:
        ang0 = (LbRandomAnyShort() & 0x3) + 6;
        ang1 = (LbRandomAnyShort() & 0x3) + 6;
        ang2 = (LbRandomAnyShort() & 0x3) + 6;
        level = 0x35;
        break;
    case 0xc:
        level = 0x33;
        break;
    case 0x10:
        ang0 = (LbRandomAnyShort() & 0x3) + 4;
        ang1 = (LbRandomAnyShort() & 0x3) + 4;
        ang2 = (LbRandomAnyShort() & 0x3) + 4;
        level = 0x40;
        break;
    case 0x11:
    case 0x16:
        level = 0x3d;
        break;
    case 0x12:
        level = 0x5a;
        break;
    case 0x15:
        ang1 = (LbRandomAnyShort() & 0x7) + 4;
        level = 0x3f;
        break;
    case 0x1e:
        level = 0x64;
        break;
    case 0x1b:
        level = 0x56;
        break;
    case 0x1c:
        level = 0x58;
        break;
    case 0x1a:
        level = 0x54;
        break;
    case 0x23:
        level = 0x65;
        break;
    case 0x28:
        level = 0x52;
        break;
    case 0x2e:
        level = 0x63;
        break;
    case 0x2f:
        level = 0x5f;
        break;
    case 0x30:
        level = 0x61;
        break;
    case 0x32:
        state_override = 0x64;
        level = 0x68;
        break;
    case 0x35:
        level = 0x68;
        break;
    case 0x37:
        state_override = 0xf;
        level = 0x6a;
        break;
    case 0x3c:
        level = 0x6f;
        break;
    case 0x41:
        level = 0x72;
        break;
    case 0x4b:
        level = 0x96;
        break;
    case 0x50:
        level = 0x8c;
        break;
    case 0x55:
        level = 0x82;
        break;
    case 0x5a:
        level = 0x78;
        break;
    case 0x5f:
        level = 0xbe;
        break;
    case 0x64:
        level = 0xb4;
        break;
    case 0x69:
        level = 0xaa;
        break;
    case 0x6e:
        level = 0xa0;
        break;
    case 0x73:
        create_temp_light(x, y + 0x6400, z, 0x32, 0xf, 0x32);
        level = 0x96;
        fixed_dir = 1;
        fixed_vy = 0x7f;
        break;
    case 0x78:
        create_temp_light(x, y + 0x6400, z, 0x32, 0xa, 0x32);
        level = 0x96;
        fixed_dir = 1;
        fixed_vy = 0x40;
        break;
    default:
        LOGERR("bang at (0x%x,0x%x,0x%x) owner=%d type=%d c=%d, bad type",
            x, y, z, owner, type, c);
        return;
    }

    if (state_override >= 0)
        p_sthing->State = state_override;

    total = ang0 + ang1 + ang2 + (fixed_dir ? 1 : 0);
    for (i = 0; i < total; i++) {
        short shr_idx;
        struct Shrapnel *p_shrap;
        ubyte quadrant;

        shr_idx = shrapnel_free;
        if (shr_idx == 0)
            break;
        shrapnel_free = shrapnel[shr_idx].child;

        if (i < ang0)
            quadrant = 1;
        else if (i < ang0 + ang1)
            quadrant = 3;
        else if (i < ang0 + ang1 + ang2)
            quadrant = 2;
        else
            quadrant = 4;

        p_shrap = &shrapnel[shr_idx];
        p_shrap->type = quadrant;
        p_shrap->x = x;
        p_shrap->y = y;
        p_shrap->z = z;
        if (fixed_dir) {
            p_shrap->vx = 0;
            p_shrap->vz = 0;
            p_shrap->vy = fixed_vy;
        } else {
            int angle1, angle2;
            int vx, vy, vz;

            angle1 = LbRandomAnyShort() & 0x7ff;
            angle2 = LbRandomAnyShort() & 0x1ff;

            vx = (lbSinTable[angle1] >> 10) * lbSinTable[angle2 + LbFPMath_PI/2];
            vx >>= 16;
            vz = (lbSinTable[angle1 + LbFPMath_PI/2] >> 10) * lbSinTable[angle2 + LbFPMath_PI/2];
            vz >>= 16;
            vy = lbSinTable[angle2] >> 10;

            p_shrap->vx = (sbyte)vx;
            p_shrap->vz = (sbyte)vz;
            p_shrap->vy = (sbyte)vy;
        }
        p_shrap->vyaw = LbRandomAnyShort() & 0xf;
        p_shrap->vpitch = LbRandomAnyShort() & 0x1f;

        p_shrap->child = p_sthing->U.UBang.shrapnel;
        p_sthing->U.UBang.shrapnel = shr_idx;
    }

    if (level != 0) {
        short ph_idx;

        ph_idx = phwoar_free;
        if (ph_idx != 0) {
            phwoar_free = phwoar[ph_idx].child;
            phwoar_num_used++;
            phwoar_num_free--;
        }
        if (ph_idx != 0) {
            struct Phwoar *p_phwoar;

            p_sthing->U.UBang.phwoar = ph_idx;
            p_phwoar = &phwoar[ph_idx];
            p_phwoar->x = x;
            p_phwoar->y = y;
            p_phwoar->z = z;
            p_phwoar->vx = 0;
            p_phwoar->vz = 0;
            p_phwoar->vy = 0;
            p_phwoar->type = level;
            p_phwoar->child = 0;
            p_phwoar->fc = 0;
            fillin_phwoar_type_info(p_phwoar);
        }
    }

    add_node_sthing(thing);
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
