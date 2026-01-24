/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file scandraw.c
 *     Ingame scanner (minimap/radar) drawing.
 * @par Purpose:
 *     Implement functions drawing the scanner map elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "scandraw.h"

#include "bfanywnd.h"
#include "bfendian.h"
#include "bfgentab.h"
#include "bfline.h"
#include "bfmath.h"
#include "bfpixel.h"
#include "bfplanar.h"

#include "engincolour.h"
#include "engintrns.h"

#include "campaign.h"
#include "display.h"
#include "thing.h"
#include "thing_onface.h"
#include "game_options.h"
#include "game_speed.h"
#include "game.h"
#include "keyboard.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/

enum ScannerArrowModes {
    SCANNER_ARROW_NEAREST_TARGET = 0,
    SCANNER_ARROW_ORIENTATION,
};

struct scanstr1 {
    long u;
    long v;
};

struct scanstr2 {
    long du;
    long dv;
};

struct scanstr3 {
    long u1;
    long v1;
    long u2;
    long v2;
};

struct NearestPos {
    ulong dist;
    short x;
    short y;
};

/** String of keycodes for changing arrow mode.
 * Cannot be of type TbKeyCode as contains plain numeric value as well.
 */
const int scanner_arrow_mode_code_keys[] = {
    KC_NUMPAD3, KC_DECIMAL, KC_NUMPAD1, KC_NUMPAD4,
    KC_NUMPAD1, KC_NUMPAD5, KC_NUMPAD9, KC_NUMPAD2,
    KC_NUMPAD6, KC_NUMPAD5, KC_NUMPAD3, KC_NUMPAD5,
    9999,
};

/** Points for dotted symmetric circle of size above 15 but below 17.
 */
const struct TbPoint circle_dots_sz16[] = {
    { 1,-7},
    { 3,-7},
    { 5,-6},
    { 6,-5},
    { 7,-3},
    { 7,-1},
    { 7, 1},
    { 7, 3},
    { 6, 5},
    { 5, 6},
    { 3, 7},
    { 1, 7},
    {-1, 7},
    {-3, 7},
    {-5, 6},
    {-6, 5},
    {-7, 3},
    {-7, 1},
    {-7,-1},
    {-7,-3},
    {-6,-5},
    {-5,-6},
    {-3,-7},
    {-1,-7},
};
#define circle_dots_sz16_count (sizeof(circle_dots_sz16)/sizeof(circle_dots_sz16[0]))

/** Points for symmetric circle of size 15.
 */
const struct TbPoint circle_line_sz15[] = {
    { 0,-7},
    { 1,-7},
    { 2,-7},
    { 2,-6},
    { 3,-6},
    { 4,-6},
    { 4,-5},
    { 5,-5},
    { 5,-4},
    { 6,-4},
    { 6,-3},
    { 6,-2},
    { 7,-2},
    { 7,-1},
    { 7, 0},
    { 7, 1},
    { 7, 2},
    { 6, 2},
    { 6, 3},
    { 6, 4},
    { 5, 4},
    { 5, 5},
    { 4, 5},
    { 4, 6},
    { 3, 6},
    { 2, 6},
    { 2, 7},
    { 1, 7},
    { 0, 7},
    {-1, 7},
    {-2, 7},
    {-2, 6},
    {-3, 6},
    {-4, 6},
    {-4, 5},
    {-5, 5},
    {-5, 4},
    {-6, 4},
    {-6, 3},
    {-6, 2},
    {-7, 2},
    {-7, 1},
    {-7, 0},
    {-7,-1},
    {-7,-2},
    {-6,-2},
    {-6,-3},
    {-6,-4},
    {-5,-4},
    {-5,-5},
    {-4,-5},
    {-4,-6},
    {-3,-6},
    {-2,-6},
    {-2,-7},
    {-1,-7},
};
#define circle_line_sz15_count (sizeof(circle_line_sz15)/sizeof(circle_line_sz15[0]))

/** Points for dotted symmetric circle of size above 13 but below 15.
 */
const struct TbPoint circle_dots_sz14[] = {
    { 0,-6},
    { 2,-6},
    { 4,-5},
    { 5,-4},
    { 6,-2},
    { 6, 0},
    { 6, 2},
    { 5, 4},
    { 4, 5},
    { 2, 6},
    { 0, 6},
    {-2, 6},
    {-4, 5},
    {-5, 4},
    {-6, 2},
    {-6, 0},
    {-6,-2},
    {-5,-4},
    {-4,-5},
    {-2,-6},
};
#define circle_dots_sz14_count (sizeof(circle_dots_sz14)/sizeof(circle_dots_sz14[0]))

/** Points for symmetric circle of size 13.
 */
const struct TbPoint circle_line_sz13[] = {
    { 0,-6},
    { 1,-6},
    { 2,-5},
    { 3,-5},
    { 4,-4},
    { 5,-3},
    { 5,-2},
    { 6,-1},
    { 6, 0},
    { 6, 1},
    { 5, 2},
    { 5, 3},
    { 4, 4},
    { 3, 5},
    { 2, 5},
    { 1, 6},
    { 0, 6},
    {-1, 6},
    {-2, 5},
    {-3, 5},
    {-4, 4},
    {-5, 3},
    {-5, 2},
    {-6, 1},
    {-6, 0},
    {-6,-1},
    {-5,-2},
    {-5,-3},
    {-4,-4},
    {-3,-5},
    {-2,-5},
    {-1,-6},
};
#define circle_line_sz13_count (sizeof(circle_line_sz13)/sizeof(circle_line_sz13[0]))

/** Points for dotted symmetric circle of size above 11 but below 13.
 */
const struct TbPoint circle_dots_sz12[] = {
    { 0,-5},
    { 2,-5},
    { 4,-4},
    { 5,-2},
    { 5, 0},
    { 5, 2},
    { 4, 4},
    { 2, 5},
    { 0, 5},
    {-2, 5},
    {-4, 4},
    {-5, 2},
    {-5, 0},
    {-5,-2},
    {-4,-4},
    {-2,-5},
};
#define circle_dots_sz12_count (sizeof(circle_dots_sz12)/sizeof(circle_dots_sz12[0]))

/** Points for symmetric circle of size 11.
 */
const struct TbPoint circle_line_sz11[] = {
    { 0,-5},
    { 1,-5},
    { 2,-4},
    { 3,-4},
    { 4,-3},
    { 4,-2},
    { 5,-1},
    { 5, 0},
    { 5, 1},
    { 4, 2},
    { 4, 3},
    { 3, 4},
    { 2, 4},
    { 1, 5},
    { 0,-5},
    {-1,-5},
    {-2,-4},
    {-3,-4},
    {-4,-3},
    {-4,-2},
    {-5,-1},
    {-5, 0},
    {-5, 1},
    {-4, 2},
    {-4, 3},
    {-3, 4},
    {-2, 4},
    {-1, 5},
};
#define circle_line_sz11_count (sizeof(circle_line_sz11)/sizeof(circle_line_sz11[0]))

/** Points for dotted symmetric circle of size above 9 but below 11.
 */
const struct TbPoint circle_dots_sz10[] = {
    { 1,-4},
    { 3,-3},
    { 4,-1},
    { 4, 1},
    { 3, 3},
    { 1, 4},
    {-1, 4},
    {-3, 3},
    {-4, 1},
    {-4,-1},
    {-3,-3},
    {-1,-4},
};
#define circle_dots_sz10_count (sizeof(circle_dots_sz10)/sizeof(circle_dots_sz10[0]))

/** Points for symmetric circle of size 9.
 */
const struct TbPoint circle_line_sz9[] = {
    { 0,-4},
    { 1,-4},
    { 2,-3},
    { 3,-3},
    { 3,-2},
    { 4,-1},
    { 4, 0},
    { 4, 1},
    { 3, 2},
    { 3, 3},
    { 2, 3},
    { 1, 4},
    { 0, 4},
    {-1, 4},
    {-2, 3},
    {-3, 3},
    {-3, 2},
    {-4, 1},
    {-4, 0},
    {-4,-1},
    {-3,-2},
    {-3,-3},
    {-2,-3},
    {-1,-4},
};
#define circle_line_sz9_count (sizeof(circle_line_sz9)/sizeof(circle_line_sz9[0]))

/** Points for dotted symmetric circle of size above 7 but below 9.
 */
const struct TbPoint circle_dots_sz8[] = {
    { 0,-3},
    { 2,-3},
    { 3,-2},
    { 3, 0},
    { 3, 2},
    { 2, 3},
    { 0, 3},
    {-2, 3},
    {-3, 2},
    {-3, 0},
    {-3,-2},
    {-2,-3},
};
#define circle_dots_sz8_count (sizeof(circle_dots_sz8)/sizeof(circle_dots_sz8[0]))

/** Points for symmetric circle of size 7.
 */
const struct TbPoint circle_line_sz7[] = {
    { 0,-3},
    { 1,-3},
    { 2,-2},
    { 3,-1},
    { 3, 0},
    { 3, 1},
    { 2, 2},
    { 1, 3},
    { 0, 3},
    {-1, 3},
    {-2, 2},
    {-3, 1},
    {-3, 0},
    {-3,-1},
    {-2,-2},
    {-1,-3},
};
#define circle_line_sz7_count (sizeof(circle_line_sz7)/sizeof(circle_line_sz7[0]))

/** Points for symmetric circle of size 5.
 */
const struct TbPoint circle_line_sz5[] = {
    { 0,-2},
    { 1,-2},
    { 2,-1},
    { 2, 0},
    { 2, 1},
    { 1, 2},
    { 0, 2},
    {-1, 2},
    {-2, 1},
    {-2, 0},
    {-2,-1},
    {-1,-2},
};
#define circle_line_sz5_count (sizeof(circle_line_sz5)/sizeof(circle_line_sz5[0]))

extern long scanner_next_key_no;

extern long SCANNER_dw064;
extern long SCANNER_dw068;
extern long SCANNER_dw06C;
extern long SCANNER_dw070;
extern long SCANNER_dw074;
extern long SCANNER_dw07C;
extern long SCANNER_dw080;

extern ubyte SCANNER_bt084;
extern ubyte SCANNER_bt085;
extern ubyte SCANNER_brig;
extern ubyte SCANNER_cont;

extern struct scanstr1 SCANNER_bbpoint[255];
extern long dword_1DBB64[];
extern long dword_1DBB6C[512];
extern TbPixel *SCANNER_screenptr;
extern ulong SCANNER_keep_arcs;
extern long scanner_arrow_mode; // = 1;

extern struct scanstr3 SCANNER_arcpoint[20];

void SCANNER_process_special_input(void)
{
    ushort sckey, nxkey;

    sckey = scanner_arrow_mode_code_keys[scanner_next_key_no];
    if (is_key_pressed(sckey, KMod_DONTCARE))
    {
        clear_key_pressed(sckey);
        nxkey = scanner_arrow_mode_code_keys[++scanner_next_key_no];
        if (nxkey == 9999)
        {
            scanner_next_key_no = 0;
            scanner_arrow_mode = scanner_arrow_mode ^ 1;
        }
    }
}

void SCANNER_dnt_SCANNER_dw070_update(ushort flags1)
{
    s64 nm_prec;
    int nm_shft;
    int pv_val, mn_val, mn_of2;
    int n1_cand, n2_cand;

    n1_cand = 0xFFFFFFF;
    n2_cand = 0xFFFFFFF;

    if ((flags1 & 0x01) != 0)
    {
        nm_prec = (s64)SCANNER_dw06C << 16;
        nm_shft = -(nm_prec / SCANNER_dw064);
        n2_cand = nm_shft >> 16;
    }
    else if ((flags1 & 0x02) != 0)
    {
        nm_prec = (0x1000000 - (s64)SCANNER_dw06C) << 16;
        nm_shft = nm_prec / SCANNER_dw064;
        n2_cand = nm_shft >> 16;
    }

    if ((flags1 & 0x04) != 0)
    {
        nm_prec = (s64)SCANNER_dw070 << 16;
        nm_shft = -(nm_prec / SCANNER_dw068);
        n1_cand = nm_shft >> 16;
    }
    else if ((flags1 & 0x08) != 0)
    {
        nm_prec = (0x1000000 - (s64)SCANNER_dw070) << 16;
        nm_shft = nm_prec / SCANNER_dw068;
        n1_cand = nm_shft >> 16;
    }

    // set SCANNER_dw074 = min(SCANNER_dw074, n2_cand, n1_cand);
    pv_val = SCANNER_dw074;
    if (n1_cand >= pv_val)
        mn_of2 = pv_val;
    else
        mn_of2 = n1_cand;
    if (mn_of2 > n2_cand)
      mn_val = n2_cand;
    else if (n1_cand >= pv_val)
        mn_val = pv_val;
    else
        mn_val = n1_cand;
    SCANNER_dw074 = mn_val;
}

void SCANNER_process_bbpoints(void)
{
    asm volatile (
      "call ASM_SCANNER_process_bbpoints\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub1(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub1\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub2(void)
{
    // TODO when rewriting, use mul_shift16_sign_pad_lo()
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub2\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub3(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub3\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub4(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub4\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub5(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub5\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub6(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub6\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub7(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub7\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub8(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub8\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub9(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub9\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub10(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub10\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub11(void)
{
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub11\n"
        :  :  : "eax" );
}

void SCANNER_dnt_sub1_sub12(void)
{
    // TODO when rewriting, use mul_shift16_sign_pad_lo()
    asm volatile (
      "call ASM_SCANNER_dnt_sub1_sub12\n"
        :  :  : "eax" );
}

void SCANNER_draw_new_transparent_map_line(int cu_x2, int cu_y2)
{
    ushort flags1, flags2;

    flags1 = (0x01 * (cu_y2 < 0))
           | (0x02 * (cu_y2 >= 0x1000000))
           | (0x04 * (cu_x2 < 0))
           | (0x08 * (cu_x2 >= 0x1000000));

    while ( 2 )
    {
        int dt_val, pv_val, cu_val;

        flags2 = (0x01 * (SCANNER_dw06C < 0))
               | (0x02 * (SCANNER_dw06C >= 0x1000000))
               | (0x04 * (SCANNER_dw070 < 0))
               | (0x08 * (SCANNER_dw070 >= 0x1000000));

        if ((flags1 | flags2) == 0)
        {
            SCANNER_dnt_sub1_sub12();
            break;
        }
        if ((flags1 & flags2) != 0)
        {
            SCANNER_dnt_sub1_sub1();
            break;
        }

        switch (flags2)
        {
        case 0x00:
            pv_val = SCANNER_dw074;
            SCANNER_dnt_SCANNER_dw070_update(flags1);
            cu_val = SCANNER_dw074;
            dt_val = pv_val - cu_val;
            if ((cu_val > 0) && (cu_val <= 400)) {
                SCANNER_dnt_sub1_sub2();
            }
            if ((dt_val > 0) && (dt_val <= 400)) {
                SCANNER_dw074 = dt_val;
                SCANNER_dnt_sub1_sub3();
            }
            break;
        case 0x01:
            SCANNER_dnt_sub1_sub4();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x02:
            SCANNER_dnt_sub1_sub5();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x04:
            SCANNER_dnt_sub1_sub6();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x05:
            SCANNER_dnt_sub1_sub8();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x06:
            SCANNER_dnt_sub1_sub9();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x08:
            SCANNER_dnt_sub1_sub7();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x09:
            SCANNER_dnt_sub1_sub10();
            if (SCANNER_dw074)
                continue;
            break;
        case 0x0A:
            SCANNER_dnt_sub1_sub11();
            if (SCANNER_dw074)
                continue;
            break;
        default:
            break;
        }
        break;
    }
}

void SCANNER_draw_new_transparent_map(void)
{
#if 0
    asm volatile ("call ASM_SCANNER_draw_new_transparent_map\n"
        :  :  : "eax" );
    return;
#endif
    int dt_x, dt_y;
    int sh_x, sh_y;
    int cu_x1, cu_y1, cu_x2, cu_y2;
    long *p_width;
    TbPixel *p_out;
    int cu_y;

    dt_x = (ingame.Scanner.X2 - ingame.Scanner.X1) >> 1;
    dt_y = (ingame.Scanner.Y2 - ingame.Scanner.Y1) >> 1;
    sh_y = (ingame.Scanner.Zoom * lbSinTable[ingame.Scanner.Angle]) >> 8;
    sh_x = (ingame.Scanner.Zoom * lbSinTable[ingame.Scanner.Angle + LbFPMath_PI/2]) >> 8;

    SCANNER_dw07C = sh_y << 16;
    SCANNER_dw080 = sh_x << 16;
    SCANNER_bt084 = (sh_y >> 16);
    SCANNER_bt085 = (sh_x >> 16);
    SCANNER_brig = ingame.Scanner.Brightness;
    SCANNER_cont = ingame.Scanner.Contrast;
    SCANNER_dw064 = sh_y;
    SCANNER_dw068 = sh_x;

    cu_y2 = (ingame.Scanner.MZ << 16) + sh_y * dt_x + sh_x * dt_y;
    cu_x2 = (ingame.Scanner.MX << 16) + sh_x * dt_x - sh_y * dt_y;
    cu_x1 = (ingame.Scanner.MZ << 16) + sh_x * dt_y - sh_y * dt_x;
    cu_y1 = (ingame.Scanner.MX << 16) - sh_x * dt_x - sh_y * dt_y;
    p_width = SCANNER_width;
    p_out = &lbDisplay.WScreen[ingame.Scanner.X1 + lbDisplay.PhysicalScreenWidth * ingame.Scanner.Y1];

    for (cu_y = ingame.Scanner.Y1; cu_y <= ingame.Scanner.Y2; cu_y++)
    {
        SCANNER_dw06C = cu_x1;
        SCANNER_dw070 = cu_y1;
        SCANNER_dw074 = *p_width + 1;
        SCANNER_screenptr = p_out;

        SCANNER_draw_new_transparent_map_line(cu_x2, cu_y2);

        p_width++;
        p_out += lbDisplay.PhysicalScreenWidth;
        cu_y1 += sh_y;
        cu_x1 -= sh_x;
        cu_y2 -= sh_x;
        cu_x2 += sh_y;
    }
}

void unkn_draw_transformed_point(short x, short y, long ptX, long ptY, long ptZ, ubyte colour)
{
    struct EnginePoint ep;

    ep.X3d = ptX - engn_xc;
    ep.Z3d = ptZ - engn_zc;
    ep.Y3d = ptY - engn_yc;
    ep.Flags = 0;
    transform_point(&ep);
    LbDrawLine(x, y, ep.pp.X, ep.pp.Y, colour);
}

void draw_objective_point(long x, long y, ThingIdx thing, short a4, ubyte colour)
{
    long ptX, ptY, ptZ;

    if (a4)
        return;

    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        ptX = p_sthing->X;
        ptY = p_sthing->Y;
        ptZ = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        ptX = p_thing->X;
        ptY = p_thing->Y;
        ptZ = p_thing->Z;
    }
    unkn_draw_transformed_point(x, y, ptX, ptY, ptZ, colour);
}

void draw_map_flat_circle(short cor_x, short cor_y, short cor_z, short radius, TbPixel colour)
{
#if 0
    asm volatile (
      "push %4\n"
      "call ASM_draw_map_flat_circle\n"
        : : "a" (cor_x), "d" (cor_y), "b" (cor_z), "c" (radius), "g" (colour));
#endif
    struct EnginePoint ep1;
    struct EnginePoint ep2;
    int cir_cor_x, cir_cor_z;
    int cir_nxt_x, cir_nxt_z;
    ushort slice;

    cir_cor_x = cor_x;
    cir_cor_z = cor_z + radius;
    for (slice = 1; slice < 0x40; slice++)
    {
        short cir_dt_x, cir_dt_z;
        cir_dt_x = (radius * lbSinTable[slice * 2 * LbFPMath_PI / 0x40]) >> 16;
        cir_dt_z = (radius * lbSinTable[LbFPMath_PI/2 + slice * 2*LbFPMath_PI / 0x40]) >> 16;
        cir_nxt_x = cor_x + cir_dt_x;
        cir_nxt_z = cor_z + cir_dt_z;

        ep1.X3d = cir_cor_x - engn_xc;
        ep1.Y3d = cor_y - engn_yc;
        ep1.Z3d = cir_cor_z - engn_zc;
        ep1.Flags = 0;
        transform_point(&ep1);
        ep2.X3d = cir_nxt_x - engn_xc;
        ep2.Y3d = cor_y - engn_yc;
        ep2.Z3d = cir_nxt_z - engn_zc;
        ep2.Flags = 0;
        transform_point(&ep2);
        LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour);

        cir_cor_x = cir_nxt_x;
        cir_cor_z = cir_nxt_z;
    }
    {
        cir_nxt_x = cor_x;
        cir_nxt_z = cor_z + radius;

        ep1.X3d = cir_cor_x - engn_xc;
        ep1.Y3d = cor_y - engn_yc;
        ep1.Z3d = cir_cor_z - engn_zc;
        ep1.Flags = 0;
        transform_point(&ep1);
        ep2.X3d = cir_nxt_x - engn_xc;
        ep2.Y3d = cor_y - engn_yc;
        ep2.Z3d = cir_nxt_z - engn_zc;
        ep2.Flags = 0;
        transform_point(&ep2);
        LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour);
    }
}

void draw_map_flat_rect(int cor_x, int cor_y, int cor_z, int size_x, int size_z, TbPixel colour)
{
#if 0
    asm volatile (
      "push %5\n"
      "push %4\n"
      "call ASM_draw_map_flat_rect\n"
        : : "a" (a1), "d" (a2), "b" (a3), "c" (a4), "g" (a5), "g" (a6));
#endif
    struct EnginePoint ep1;
    struct EnginePoint ep2;
    int cor_x2, cor_z2;

    cor_x2 = cor_x + size_x;
    cor_z2 = cor_z + size_z;

    ep1.X3d = cor_x  - engn_xc;
    ep1.Y3d = cor_y  - engn_yc;
    ep1.Z3d = cor_z  - engn_zc;
    ep1.Flags = 0;
    transform_point(&ep1);
    ep2.X3d = cor_x2 - engn_xc;
    ep2.Y3d = cor_y  - engn_yc;
    ep2.Z3d = cor_z  - engn_zc;
    ep2.Flags = 0;
    transform_point(&ep2);
    LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour);

    ep1.X3d = cor_x2 - engn_xc;
    ep1.Y3d = cor_y  - engn_yc;
    ep1.Z3d = cor_z  - engn_zc;
    ep1.Flags = 0;
    transform_point(&ep1);
    ep2.X3d = cor_x2 - engn_xc;
    ep2.Y3d = cor_y  - engn_yc;
    ep2.Z3d = cor_z2 - engn_zc;
    ep2.Flags = 0;
    transform_point(&ep2);
    LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour);

    ep1.X3d = cor_x2 - engn_xc;
    ep1.Y3d = cor_y  - engn_yc;
    ep1.Z3d = cor_z2 - engn_zc;
    ep1.Flags = 0;
    transform_point(&ep1);
    ep2.X3d = cor_x  - engn_xc;
    ep2.Y3d = cor_y  - engn_yc;
    ep2.Z3d = cor_z2 - engn_zc;
    ep2.Flags = 0;
    transform_point(&ep2);
    LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour);

    ep1.X3d = cor_x  - engn_xc;
    ep1.Y3d = cor_y  - engn_yc;
    ep1.Z3d = cor_z2 - engn_zc;
    ep1.Flags = 0;
    transform_point(&ep1);
    ep2.Flags = 0;
    ep2.X3d = cor_x  - engn_xc;
    ep2.Y3d = cor_y  - engn_yc;
    ep2.Z3d = cor_z  - engn_zc;
    transform_point(&ep2);
    LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour);
}

void SCANNER_draw_shape_from_points(int x, int y, const struct TbPoint *points, ushort count, TbPixel col)
{
    int i;

    for (i = 0; i < count; i++)
    {
        int pt_x, pt_y;
        int h;

        pt_y = y + points[i].y;
        if ((pt_y <= ingame.Scanner.Y1) || (pt_y >= ingame.Scanner.Y2))
            continue;

        h = y - ingame.Scanner.Y1;
        pt_x = x + points[i].x;
        if ((pt_x <= ingame.Scanner.X1) || (pt_x >= ingame.Scanner.X1 + ingame.Scanner.Width[h]))
            continue;

        LbDrawPixel(pt_x, pt_y, col);
    }
}

/** Draws a range of points from given array.
 *
 * This function assumes graphics window set to the scanner map area.
 */
void SCANNER_draw_shape_part_from_points(int x, int y, const struct TbPoint *points, ushort count, ushort frame, ushort part, TbPixel col)
{
    ushort i;

    for (i = 0; i < part; i++)
    {
        int pt_x, pt_y;
        int pt;
        uint fade;

        pt = (frame + count - i) % count;

        pt_y = y + points[pt].y;
        if ((pt_y <= 0) || (pt_y >= ingame.Scanner.Y2))
            continue;

        pt_x = x + points[pt].x;
        if ((pt_x <= 0) || (pt_x >= SCANNER_width[pt_y]))
            continue;

        fade = 0x10 + 0x10 * (part - i - 1) / (part - 1);

        LbDrawPixel(pt_x, pt_y, pixmap.fade_table[(fade << 8) + col]);
    }
}

/** Draw a symmetric circle of solid line, or filled disk.
 */
void SCANNER_draw_circle_line(int x, int y, ushort sz, TbPixel col, TbBool filled)
{
    int h;

    switch (sz)
    {
    default:
        break;
    case 16:
    case 15:
        SCANNER_draw_shape_from_points(x, y, circle_line_sz15, circle_line_sz15_count, col);
        if (!filled)
            break;
        //  fall through
    case 14:
    case 13:
        SCANNER_draw_shape_from_points(x, y, circle_line_sz13, circle_line_sz13_count, col);
        if (!filled)
            break;
        //  fall through
    case 12:
    case 11:
        SCANNER_draw_shape_from_points(x, y, circle_line_sz11, circle_line_sz11_count, col);
        if (!filled)
            break;
        //  fall through
    case 10:
    case 9:
        SCANNER_draw_shape_from_points(x, y, circle_line_sz9, circle_line_sz9_count, col);
        if (!filled)
            break;
        //  fall through
    case 8:
    case 7:
        SCANNER_draw_shape_from_points(x, y, circle_line_sz7, circle_line_sz7_count, col);
        if (!filled)
            break;
        //  fall through
    case 6:
    case 5:
        // We could use circle_line_sz5, doing the draw directly is just an optimization
        if (x > ingame.Scanner.X1 + 1)
        {
            LbDrawPixel(x - 2, y - 1, col);
            LbDrawPixel(x - 2, y,     col);
            LbDrawPixel(x - 2, y + 1, col);
        }
        if (y < ingame.Scanner.Y2 - 2)
        {
            LbDrawPixel(x - 1, y + 2, col);
            LbDrawPixel(x,     y + 2, col);
            LbDrawPixel(x + 1, y + 2, col);
        }
        h = y - ingame.Scanner.Y1;
        if (x < ingame.Scanner.X1 + ingame.Scanner.Width[h])
        {
            if (x < ingame.Scanner.X1 + ingame.Scanner.Width[h-1])
                LbDrawPixel(x + 2, y - 1, col);
            LbDrawPixel(x + 2, y, col);
            LbDrawPixel(x + 2, y + 1, col);
        }
        if (y > ingame.Scanner.Y1 + 1)
        {
            if (x < ingame.Scanner.X1 + ingame.Scanner.Width[h-2] + 2)
                LbDrawPixel(x - 1, y - 2, col);
            if (x < ingame.Scanner.X1 + ingame.Scanner.Width[h-2] + 1)
                LbDrawPixel(x,     y - 2, col);
            if (x < ingame.Scanner.X1 + ingame.Scanner.Width[h-2] + 0)
                LbDrawPixel(x + 1, y - 2, col);
        }
        if (!filled)
            break;
        //  fall through
    case 4:
        LbDrawPixel(x - 1, y - 1, col);
        LbDrawPixel(x - 1, y + 1, col);
        LbDrawPixel(x + 1, y - 1, col);
        LbDrawPixel(x + 1, y + 1, col);
        //  fall through
    case 3:
        LbDrawPixel(x, y - 1, col);
        LbDrawPixel(x, y + 1, col);
        LbDrawPixel(x - 1, y, col);
        LbDrawPixel(x + 1, y, col);
        if (!filled)
            break;
        //  fall through
    case 2:
    case 1:
        LbDrawPixel(x, y, col);
        break;
    case 0:
        break;
    }
}

/** Draw a symmetric circle of dotted line, or partially filled disk.
 *
 * The dots drawn here are typically beyond ones drawn by SCANNER_draw_circle_line()
 * for the same size of the circle. Except for sizes 6 and below, where both functions
 * lead to identical output - hard to draw dotted circle that small.
 */
void SCANNER_draw_circle_dotted(int x, int y, ushort sz, TbPixel col, TbBool filled)
{
    switch (sz)
    {
    default:
        break;
    case 16:
    case 15:
        SCANNER_draw_shape_from_points(x, y, circle_dots_sz16, circle_dots_sz16_count, col);
        if (!filled)
            break;
        //  fall through
    case 14:
    case 13:
        SCANNER_draw_shape_from_points(x, y, circle_dots_sz14, circle_dots_sz14_count, col);
        if (!filled)
            break;
        //  fall through
    case 12:
    case 11:
        SCANNER_draw_shape_from_points(x, y, circle_dots_sz12, circle_dots_sz12_count, col);
        if (!filled)
            break;
        //  fall through
    case 10:
    case 9:
        SCANNER_draw_shape_from_points(x, y, circle_dots_sz10, circle_dots_sz10_count, col);
        if (!filled)
            break;
        //  fall through
    case 8:
    case 7:
        SCANNER_draw_shape_from_points(x, y, circle_dots_sz8, circle_dots_sz8_count, col);
        if (!filled)
            break;
        //  fall through
    case 6:
    case 5:
        SCANNER_draw_circle_line(x, y, 5, col, false);
        if (!filled)
            break;
        //  fall through
    case 4:
    case 3:
        SCANNER_draw_circle_line(x, y, 3, col, false);
        if (!filled)
            break;
        //  fall through
    case 2:
    case 1:
        SCANNER_draw_circle_line(x, y, 1, col, false);
        break;
    case 0:
        break;
    }
}

/** Draw an arc (part of a circle) with solid, shaded line.
 */
void SCANNER_draw_circle_arc_line(int x, int y, ushort sz, short angle, short anpart, TbPixel col)
{
    const struct TbPoint *points;
    ushort count, frame, part;

    switch (sz)
    {
    default:
    case 16:
    case 15:
        points = circle_line_sz15;
        count = circle_line_sz15_count;
        break;
    case 14:
    case 13:
        points = circle_line_sz13;
        count = circle_line_sz13_count;
        break;
    case 12:
    case 11:
        points = circle_line_sz11;
        count = circle_line_sz11_count;
        break;
    case 10:
    case 9:
        points = circle_line_sz9;
        count = circle_line_sz9_count;
        break;
    case 8:
    case 7:
        points = circle_line_sz7;
        count = circle_line_sz7_count;
        break;
    case 6:
    case 5:
        points = circle_line_sz5;
        count = circle_line_sz5_count;
        break;
    case 4:
    case 3:
        points = NULL;
        count = 4;
        break;
    case 2:
    case 1:
    case 0:
        points = NULL;
        count = 1;
        break;
    }

    if (points == NULL)
        return;

    frame = count * angle / (2 * LbFPMath_PI);
    part = count * anpart / (2 * LbFPMath_PI);

    SCANNER_draw_shape_part_from_points(x, y, points, count, frame, part, col);
}

/** Scale size of dots and curves drawn on the scanner.
 */
ushort SCANNER_scale_size(short base_size)
{
    short sz;

    if (!SCANNER_scale_dots)
        return base_size;
    // For lower resolutions, keep the smaller size for wider zoom range
    // This assumes the game is played on a large monitor - in such case,
    // switching to larger dots doesn't look that good
    if (lbDisplay.GraphicsScreenHeight < 600)
        sz = base_size * (450 - ingame.Scanner.Zoom) / 208;
    else
        sz = base_size * (450 - ingame.Scanner.Zoom) / 192;
    if (sz > 15)
        sz = 15;
    else if (sz < 1)
        sz = 1;

    return sz;
}

void SCANNER_draw_mark_point3_blink2_filled(int x, int y, TbPixel col)
{
    ushort frame, sz;

    frame = gameturn & 1;
    sz = SCANNER_scale_size(2 * frame + 1);
    if ((frame == 1) && (sz < 3))
        sz = 3;

    SCANNER_draw_circle_line(x, y, sz, col, true);
}

void SCANNER_draw_mark_point5_blink4(int x, int y, TbPixel col)
{
#if 0
    asm volatile (
      "call ASM_SCANNER_draw_mark_point5_blink4\n"
        : : "a" (x), "d" (y), "b" (col));
#endif
    ushort frame, sz;

    frame = gameturn & 3;
    if (frame == 0)
        return;
    sz = SCANNER_scale_size(2 * (frame - 1) + 1);
    if ((frame == 3) && (sz < 5))
        sz = 5;
    else if ((frame == 2) && (sz < 3))
        sz = 3;

    SCANNER_draw_circle_dotted(x, y, sz, col, false);
}

void SCANNER_draw_mark_point7(int x, int y, TbPixel col)
{
#if 0
    asm volatile (
      "call ASM_SCANNER_draw_mark_point7\n"
        : : "a" (x), "d" (y), "b" (col));
#endif
    ushort sz;

    sz = SCANNER_scale_size(7);
    if (sz < 5)
        sz = 5;

    SCANNER_draw_circle_dotted(x, y, sz, col, false);
}

void SCANNER_draw_circle_point7_flowing(int x, int y, TbPixel col)
{
    ushort angle, anpart, sz;

    angle = (gameturn - 1) * LbFPMath_PI / 8;
    anpart = 2 * LbFPMath_PI / 5;
    sz = SCANNER_scale_size(7);
    if (sz < 5)
        sz = 5;

    SCANNER_draw_circle_arc_line(x, y, sz, angle, anpart, col);
}

void SCANNER_process_arcpoints(void)
{
    asm volatile (
      "call ASM_SCANNER_process_arcpoints\n"
        :  :  : "eax" );
}

void SCANNER_draw_orientation_arrow(int pos_x1, int pos_y1, int range, int angle)
{
    int x1, y1, x2, y2, len_x, len_y;

    x1 = ((range * lbSinTable[angle]) >> 16) + pos_x1;
    len_y = (range * lbSinTable[angle]) >> 19;
    y1 = ((range * -lbSinTable[angle + LbFPMath_PI/2]) >> 16) + pos_y1;
    len_x = (range * -lbSinTable[angle + LbFPMath_PI/2]) >> 19;
    x2 = x1 - len_y;
    y2 = y1 - len_x;
    LbDrawLine(x1, y1, x2 - len_x, y2 + len_y, colour_lookup[ColLU_WHITE]);
    LbDrawLine(x1, y1, x2 + len_x, y2 - len_y, colour_lookup[ColLU_WHITE]);
}

TbBool thing_visible_on_scanner(struct Thing *p_thing)
{
    TbBool ret;

    if ((p_thing->Flag2 & (TgF2_ExistsOffMap|TgF2_InsideBuilding|TgF2_Unkn0002)) != 0)
        return false;

    switch (p_thing->Type)
    {
    case TT_VEHICLE:
        ret = true;
        break;
    case TT_PERSON:
        ret = true;
        break;
    case TT_MINE:
        ret = (p_thing->SubType == 48);
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

TbBool thing_visible_on_scanner_this_turn(struct Thing *p_thing)
{
    TbBool ret;
    if ((gameturn & 8) != 0)
    {
        switch (p_thing->Type)
        {
        case TT_PERSON:
            ret = (p_thing->Flag & TngF_InVehicle) == 0;
            break;
        default:
            ret = true;
            break;
        }
    } else
    {
        switch (p_thing->Type)
        {
        case TT_VEHICLE:
            ret = p_thing->U.UVehicle.PassengerHead == 0;
            break;
        default:
            ret = true;
            break;
        }
    }
    return ret;
}

static void scanner_coords_line_clip(int *x1, int *y1, int *x2, int *y2, int sc_width)
{
    int dist_x, dist_z;
    s64 magn;
    int factr;
    u32 tmp;

    if (*x2 - sc_width > *y2)
    {
        dist_x = *x2 - *x1;
        dist_z = *y2 - *y1;

        magn = (*y1 - *x1 + (s64)sc_width) << 32;
        factr = magn / ((dist_x - dist_z) << 16);

        tmp = (factr * dist_x) & 0xFFFF0000;
        tmp |= ((factr * (s64)dist_x) >> 32) & 0xFFFF;
        *x2 = bw_rotl32(tmp, 16) + *x1;

        tmp = (factr * dist_z) & 0xFFFF0000;
        tmp |= ((factr * (s64)dist_z) >> 32) & 0xFFFF;
        *y2 = bw_rotl32(tmp, 16) + *y1;
    }
    else if (*x1 - sc_width > *y1)
    {
        dist_x = *x1 - *x2;
        dist_z = *y1 - *y2;

        magn = (*y2 - *x2 + (s64)sc_width) << 32;
        factr = magn / ((dist_x - dist_z) << 16);

        tmp = (factr * dist_x) & 0xFFFF0000;
        tmp |= ((factr * (s64)dist_x) >> 32) & 0xFFFF;
        *x1 = bw_rotl32(tmp, 16) + *x2;

        tmp = (factr * dist_z) & 0xFFFF0000;
        tmp |= ((factr * (s64)dist_z) >> 32) & 0xFFFF;
        *y1 = bw_rotl32(tmp, 16) + *y2;
    }
}

static void map_coords_to_scanner(int *sc_x, int *sc_y, int sh_x, int sh_y, int bsh_x, int bsh_y)
{
    int rval_xy, rval_yy, rval_yx, rval_xx, rval_div;
    long prec_x, prec_y;

    rval_xy = mul_shift16_sign_pad_lo(sh_x, bsh_y);

    rval_yy = mul_shift16_sign_pad_lo(sh_y, bsh_y);

    rval_yx = mul_shift16_sign_pad_lo(sh_y, bsh_x);

    rval_xx = mul_shift16_sign_pad_lo(sh_x, bsh_x);

    rval_div = mul_shift16_sign_pad_lo(sh_y, sh_y)
      + mul_shift16_sign_pad_lo(sh_x, sh_x);

    prec_y = ((rval_yx - (s64)rval_xy) << 16) / rval_div;
    prec_x = ((rval_xx + (s64)rval_yy) << 16) / rval_div;

    *sc_y = prec_y >> 16;
    *sc_x = prec_x >> 16;
}

void SCANNER_draw_blips(int pos_mx, int pos_mz, int sh_x, int sh_y)
{
    short bn;
    int sc_width;

    sc_width = (ingame.Scanner.X2 - ingame.Scanner.X1) - 24;

    for (bn = 0; bn < SCANNER_BIG_BLIP_COUNT; bn++)
    {
        int base_x, base_y;
        int base_i, i;

        if (ingame.Scanner.BigBlip[bn].Period == 0)
            continue;

        {
            int bsh_x, bsh_y;

            bsh_y = 2 * ingame.Scanner.BigBlip[bn].Z - pos_mz;
            bsh_x = 2 * ingame.Scanner.BigBlip[bn].X - pos_mx;
            map_coords_to_scanner(&base_x, &base_y, sh_x, sh_y, bsh_x, bsh_y);
        }

        SCANNER_draw_circle_point7_flowing(base_x, base_y, ingame.Scanner.BigBlip[bn].Colour);

        base_i = bn * 16;

        for (i = 0; i < 16; i++)
        {
            int bsh_x, bsh_y;
            int sc_x, sc_y;

            bsh_y = 2 * SCANNER_bbpoint[i].u - pos_mz;
            bsh_x = 2 * SCANNER_bbpoint[i].v - pos_mx;
            map_coords_to_scanner(&sc_x, &sc_y, sh_x, sh_y, bsh_x, bsh_y);

            dword_1DBB6C[2 * (base_i + i) + 0] = sc_x;
            dword_1DBB6C[2 * (base_i + i) + 1] = sc_y;
        }

        for (i = 0; i < 16; i++)
        {
            int ri;
            int x1, y1, x2, y2;

            ri = base_i + i;
            x1 = dword_1DBB6C[2 * ri + 0];
            y1 = dword_1DBB6C[2 * ri + 1];
            if (i == 15)
              ri = base_i + i - 15;
            else
              ri = base_i + i + 1;
            x2 = dword_1DBB6C[2 * ri + 0];
            y2 = dword_1DBB6C[2 * ri + 1];

            if ((x1 - sc_width <= y1) || (x2 - sc_width <= y2))
            {
                ushort ft_idx;

                scanner_coords_line_clip(&x1, &y1, &x2, &y2, sc_width);
                ft_idx = 512 * (31 - ingame.Scanner.BigBlip[bn].Counter)
                           + ingame.Scanner.BigBlip[bn].Colour;
                LbDrawLine(x1, y1, x2, y2, pixmap.fade_table[ft_idx]);
            }
        }
    }
}

void SCANNER_draw_arcs(int pos_mx, int pos_mz, int sh_x, int sh_y)
{
    short bn;
    int sc_width;

    sc_width = (ingame.Scanner.X2 - ingame.Scanner.X1) - 24;

    if (!SCANNER_keep_arcs)
    {
        for (bn = 0; bn < SCANNER_ARC_COUNT; bn++)
        {
            ingame.Scanner.Arc[bn].Period = 0;
            ingame.Scanner.Arc[bn].Counter = 0;
        }
    }
    SCANNER_keep_arcs = 0;
    SCANNER_process_arcpoints();

    for (bn = 0; bn < SCANNER_ARC_COUNT; bn++)
    {
        int base_x, base_y;
        int base_i, i;
        int x1, y1, x2, y2;

        if ((ingame.Scanner.Arc[bn].Period == 0) && (ingame.Scanner.Arc[bn].Counter == 0))
            continue;

        {
            int bsh_x, bsh_y;

            bsh_y = 2 * SCANNER_arcpoint[bn].u1 - pos_mz;
            bsh_x = 2 * SCANNER_arcpoint[bn].v1 - pos_mx;
            map_coords_to_scanner(&base_x, &base_y, sh_x, sh_y, bsh_x, bsh_y);
        }

        base_i = bn * 5;

        for (i = 1; i < 5; i++)
        {
            int bsh_x, bsh_y;
            int ri;
            int ssb_x, ssb_y;

            ri = base_i + i;

            bsh_y = 2 * SCANNER_arcpoint[ri].u1 - pos_mz;
            bsh_x = 2 * SCANNER_arcpoint[ri].v1 - pos_mx;
            map_coords_to_scanner(&ssb_x, &ssb_y, sh_x, sh_y, bsh_x, bsh_y);

            x1 = base_x;
            y1 = base_y;
            x2 = ssb_x;
            y2 = ssb_y;

            if ((x1 - sc_width <= y1) || (x2 - sc_width <= y2))
            {
                scanner_coords_line_clip(&x1, &y1, &x2, &y2, sc_width);
                LbDrawLine(x1, y1, x2, y2, ingame.Scanner.Arc[bn].ColourIsUnused);
            }

            base_x = ssb_x;
            base_y = ssb_y;
        }
    }
}

void SCANNER_check_nearest(struct Thing *p_thing, struct NearestPos *p_nearest, int base_x, int base_y, int pos_x1, int pos_y1)
{
    sbyte group_col;

    if (scanner_arrow_mode != SCANNER_ARROW_NEAREST_TARGET)
        return;

    int dist_x, dist_y;
    ulong dist;
    int i;

    group_col = 0;
    for (i = 0; i < ingame.Scanner.GroupCount; ++i)
    {
        if (ingame.Scanner.Group[i] == p_thing->U.UObject.EffectiveGroup) {
            group_col = ingame.Scanner.GroupCol[i];
            break;
        }
    }
    dist_x = ingame.Scanner.X1 + base_x - pos_x1;
    dist_y = ingame.Scanner.Y1 + base_y - pos_y1;
    dist = dist_x * dist_x + dist_y * dist_y;
    if (group_col  && dist < p_nearest->dist)
    {
        p_nearest->x = dist_x;
        p_nearest->y = dist_y;
        p_nearest->dist = dist;
    }
}

TbPixel SCANNER_thing_colour(struct Thing *p_thing)
{
    TbPixel col;

    switch (p_thing->Type)
    {
    case TT_VEHICLE:
        col = 1;
        break;
    case TT_PERSON:
        if ((p_thing->Flag & TngF_Persuaded) != 0)
            col = colour_lookup[ColLU_YELLOW];
        else if ((p_thing->Flag & TngF_PlayerAgent) != 0 && p_thing->U.UPerson.CurrentWeapon == WEP_CLONESHLD)
            col = SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
        else
            col = SCANNER_people_colours[p_thing->SubType];
        break;
    case TT_MINE:
        col = 1;
        break;
    default:
        col = 0;
        break;
    }
    return col;
}

#define __CFSHL__(a, b)

void SCANNER_draw_thing(struct Thing *p_thing, struct NearestPos *p_nearest, int pos_mx, int pos_mz, int sh_x, int sh_y, int pos_x1, int pos_y1)
{
    int base_x, base_y;
    int x, y;
    TbPixel col;

    {
        int tng_z, tng_x;
        int bsh_x, bsh_y;

        if ((p_thing->Type == TT_PERSON) && ((p_thing->Flag & TngF_Unkn4000) != 0)) {
            struct Thing *p_vehicle;

            p_vehicle = &things[p_thing->U.UPerson.Vehicle];
            tng_z = p_vehicle->Z;
            tng_x = p_vehicle->X;
        } else {
            tng_z = p_thing->Z;
            tng_x = p_thing->X;
        }
        tng_z = ((tng_z << 8) - (__CFSHL__(tng_z << 8 >> 31, 7) + (tng_z << 8 >> 31 << 7))) >> 7;
        tng_x = ((tng_x << 8) - (__CFSHL__(tng_x << 8 >> 31, 7) + (tng_x << 8 >> 31 << 7))) >> 7;
        bsh_y = tng_z - pos_mz;
        bsh_x = tng_x - pos_mx;
        map_coords_to_scanner(&base_x, &base_y, sh_x, sh_y, bsh_x, bsh_y);
    }

    SCANNER_check_nearest(p_thing, p_nearest, base_x, base_y, pos_x1, pos_y1);

    if ((base_y < 0) || (ingame.Scanner.Y1 + base_y > ingame.Scanner.Y2)
      || (base_x < 0) || (base_x > SCANNER_width[base_y])) {
        return;
    }

    col = SCANNER_thing_colour(p_thing);

    if (col == 0)
        return;

    x = ingame.Scanner.X1 + base_x;
    y = ingame.Scanner.Y1 + base_y;
    if ((p_thing->Flag & TngF_Destroyed) == 0)
    {
        if (((p_thing->Flag & TngF_PlayerAgent) == 0) || (p_thing->U.UPerson.CurrentWeapon == WEP_CLONESHLD))
        {
            SCANNER_draw_mark_point3_blink2_filled(x, y, col);
        }
        else
        {
            if (in_network_game)
                SCANNER_draw_mark_point5_blink4(x, y, net_player_colours[p_thing->U.UPerson.ComCur >> 2]);
            else
                SCANNER_draw_mark_point5_blink4(x, y, col);
        }
    }
    if ((gameturn & 7) == 0)
    {
        int i;
        TbPixel gcol;

        gcol = 0;
        for (i = 0; i < ingame.Scanner.GroupCount; i++)
        {
            if (ingame.Scanner.Group[i] == p_thing->U.UPerson.EffectiveGroup) {
                gcol = ingame.Scanner.GroupCol[i];
                break;
            }
        }
        if (gcol != 0) {
              SCANNER_draw_mark_point7(x, y, gcol);
        }
    }
}

void SCANNER_draw_sthing(struct SimpleThing *p_sthing, int pos_mx, int pos_mz, int sh_x, int sh_y)
{
    int base_x, base_y;
    int x, y;
    TbPixel col;

    {
        int tng_z, tng_x;
        int bsh_x, bsh_y;

        tng_z = p_sthing->Z;
        tng_x = p_sthing->X;
        tng_z = (((tng_z << 8) - (__CFSHL__(tng_z << 8 >> 31, 7) + (tng_z << 8 >> 31 << 7))) >> 7);
        tng_x = (((tng_x << 8) - (__CFSHL__(tng_x << 8 >> 31, 7) + (tng_x << 8 >> 31 << 7))) >> 7);
        bsh_y = tng_z - pos_mz;
        bsh_x = tng_x - pos_mx;
        map_coords_to_scanner(&base_x, &base_y, sh_x, sh_y, bsh_x, bsh_y);
    }

    x = ingame.Scanner.X1 + base_x;
    y = ingame.Scanner.Y1 + base_y;

    col = p_sthing->U.UWeapon.WeaponType ? colour_lookup[8] : colour_lookup[ColLU_YELLOW];

    if ((base_y < 0) || (base_y > ingame.Scanner.Y2 - ingame.Scanner.Y1)
      || (base_x < 0) || (base_x > SCANNER_width[base_y])) {
        return;
    }

    SCANNER_draw_circle_line(x, y, 1, col, true);
}

void SCANNER_draw_things_dots(int pos_mx, int pos_mz, int sh_x, int sh_y, int pos_x1, int pos_y1, int range)
{
    struct Thing *p_thing;
    struct SimpleThing *p_sthing;
    struct NearestPos nearest;
    long limit;
    short thing;

    nearest.dist = 0x7FFFFFFF;
    limit = things_used;
    for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
    {
        if (--limit == -1)
            break;
        p_thing = &things[thing];

        if (!thing_visible_on_scanner(p_thing))
            continue;

        if (thing_visible_on_scanner_this_turn(p_thing)) {
            SCANNER_draw_thing(p_thing, &nearest, pos_mx, pos_mz, sh_x, sh_y, pos_x1, pos_y1);
        }
    }

    limit = sthings_used;
    for (thing = sthings_used_head; thing < 0; thing = p_sthing->LinkChild)
    {
        if (--limit == -1)
            break;
        p_sthing = &sthings[thing];

        if (p_sthing->Type != SmTT_DROPPED_ITEM)
            continue;

        SCANNER_draw_sthing(p_sthing, pos_mx, pos_mz, sh_x, sh_y);
    }

    switch (scanner_arrow_mode)
    {
    case SCANNER_ARROW_ORIENTATION:
        SCANNER_draw_orientation_arrow(pos_x1, pos_y1, range, ingame.Scanner.Angle);
        break;
    case SCANNER_ARROW_NEAREST_TARGET:
        if ((nearest.dist > range * range) && (nearest.dist != 0x7FFFFFFF))
        {
            long angle;
            angle = arctan(nearest.x, nearest.y);
            SCANNER_draw_orientation_arrow(pos_x1, pos_y1, range, angle);
        }
        break;
    }
}

void SCANNER_draw_area_frame(void)
{
    LbDrawLine(ingame.Scanner.X1 - 1, ingame.Scanner.Y1 - 1,
      ingame.Scanner.X1 - 1, ingame.Scanner.Y2 + 1, SCANNER_colour[4]);
    LbDrawLine(ingame.Scanner.X1 - 1, ingame.Scanner.Y1 - 1,
      ingame.Scanner.X2 - 24, ingame.Scanner.Y1 - 1, SCANNER_colour[4]);
    LbDrawLine(ingame.Scanner.X1 - 1, ingame.Scanner.Y2 + 1,
      ingame.Scanner.X2, ingame.Scanner.Y2 + 1, SCANNER_colour[4]);
    LbDrawLine(ingame.Scanner.X2 - 24, ingame.Scanner.Y1 - 1,
      ingame.Scanner.X2 + 1, ingame.Scanner.Y1 + 24, SCANNER_colour[4]);
    LbDrawLine(ingame.Scanner.X2 + 1, ingame.Scanner.Y1 + 24,
      ingame.Scanner.X2 + 1, ingame.Scanner.Y2 + 1, SCANNER_colour[4]);
}

void SCANNER_draw_signals(void)
{
#if 0
    asm volatile ("call ASM_SCANNER_draw_signals\n"
        :  :  : "eax" );
    return;
#endif
    int sh_x, sh_y;
    int range;
    int pos_mx, pos_mz;
    int pos_x1, pos_y1;
    struct TbAnyWindow window_bkp;

    {
        int dt_x, dt_y;

        dt_x = (ingame.Scanner.X2 - ingame.Scanner.X1) >> 1;
        dt_y = (ingame.Scanner.Y2 - ingame.Scanner.Y1) >> 1;
        sh_y = (ingame.Scanner.Zoom * lbSinTable[ingame.Scanner.Angle]) >> 8;
        sh_x = (ingame.Scanner.Zoom * lbSinTable[ingame.Scanner.Angle + LbFPMath_PI/2]) >> 8;
        pos_x1 = ingame.Scanner.X1 + dt_x;
        pos_y1 = ingame.Scanner.Y1 + dt_y;
        if (dt_x >= dt_y)
            range = (ingame.Scanner.Y2 - ingame.Scanner.Y1) >> 1;
        else
            range = (ingame.Scanner.X2 - ingame.Scanner.X1) >> 1;
        range -= 5;
        pos_mz = (ingame.Scanner.MZ << 16) + sh_x * dt_y - sh_y * dt_x;
        pos_mx = (ingame.Scanner.MX << 16) - sh_x * dt_x - sh_y * dt_y;
    }

    LbScreenStoreGraphicsWindow(&window_bkp);
    LbScreenSetGraphicsWindow(ingame.Scanner.X1, ingame.Scanner.Y1,
      ingame.Scanner.X2 - ingame.Scanner.X1 + 1,
      ingame.Scanner.Y2 - ingame.Scanner.Y1 + 1);

    SCANNER_draw_blips(pos_mx, pos_mz, sh_x, sh_y);

    SCANNER_draw_arcs(pos_mx, pos_mz, sh_x, sh_y);

    LbScreenLoadGraphicsWindow(&window_bkp);

    SCANNER_draw_things_dots(pos_mx, pos_mz, sh_x, sh_y, pos_x1, pos_y1, range);
}

void SCANNER_draw_new_transparent(void)
{
    SCANNER_process_special_input();
    SCANNER_process_bbpoints();
    SCANNER_draw_new_transparent_map();
    SCANNER_draw_signals();
    SCANNER_draw_area_frame();
}

/******************************************************************************/
