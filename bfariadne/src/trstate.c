/******************************************************************************/
// Bullfrog Ariadne Pathfinding Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file trstate.c
 *     Triangulation state declaration and support.
 * @par Purpose:
 *     Implement functions for handling whole triangulation states.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Sep 2023 - 02 Jan 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "triangls.h"

#include <limits.h>
#include <stdlib.h>
#include "trstate.h"
#include "trlog.h"
/******************************************************************************/

void triangulation_initialize(void)
{
    int n;

    for (n = 0; n < TRIANGULATIONS_COUNT; n++)
    {
        triangulation[n].tri_allocated = 0;
        triangulation[n].tri_initialised = 0;
        triangulation[n].last_tri = -1;
        triangulation[n].ix_Triangles = 0;
        triangulation[n].count_Triangles = 0;
        triangulation[n].free_Triangles = -1;
        triangulation[n].triangle_top = 0;
        triangulation[n].max_Triangles = 0;
        triangulation[n].Triangles = 0;
        triangulation[n].ix_Points = 0;
        triangulation[n].count_Points = 0;
        triangulation[n].free_Points = -1;
        triangulation[n].point_top = 0;
        triangulation[n].max_Points = 0;
        triangulation[n].Points = 0;
    }

    triangulation_initied = 1;
}

void triangulation_select(int trglno)
{
    asm volatile ("call ASM_triangulation_select\n"
        : : "a" (trglno));
}

void triangulation_clear_enter_into_solid(void)
{
    ubyte seltr;

    seltr = selected_triangulation_no;

    triangulation_select(1);
    triangulation_clear_enter_into_solid_gnd();

    triangulation_select(2);
    triangulation_clear_enter_into_solid_air();

    triangulation_select(seltr);
}

/******************************************************************************/
