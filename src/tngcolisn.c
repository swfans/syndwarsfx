/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file tngcolisn.c
 *     Thing collisions support.
 * @par Purpose:
 *     Defines structures and routibes for detecting and handling collisions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     04 Sep 2024 - 11 Nov 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "tngcolisn.h"

#include <stdio.h>
#include "bfline.h"

#include "drawtext.h"
#include "engincam.h"
#include "engincolour.h"
#include "engintrns.h"
#include "enginzoom.h"

#include "bigmap.h"
#include "drawtext_wrp.h"
#include "thing.h"
/******************************************************************************/

struct ColVect *game_col_vects = NULL;
ushort next_col_vect = 1;

struct ColVectList *game_col_vects_list = NULL;
ushort next_vects_list = 1;

ubyte debug_hud_collision = 0;

/******************************************************************************/

void set_dome_col(struct Thing *p_building, ubyte flag)
{
    struct ColVectList *p_cvlist;
    ushort vl_beg, vl_end;
    ushort  vl;

    // The thing data contains properties with range of ColVectList
    // which can switch the passability
    vl_beg = p_building->U.UObject.BuildStartVect;
    vl_end = vl_beg + p_building->U.UObject.BuildNumbVect;
    if (flag)
    {
        for (vl = vl_beg; vl < vl_end; vl++)
        {
            p_cvlist = &game_col_vects_list[vl];
            p_cvlist->NextColList |= 0x8000;
        }
    }
    else
    {
        for (vl = vl_beg; vl < vl_end; vl++)
        {
            p_cvlist = &game_col_vects_list[vl];
            p_cvlist->NextColList &= ~0x8000;
        }
    }
}

void draw_unkn1_bar(ushort cv)
{
#if 0
    asm volatile (
      "call ASM_draw_unkn1_bar\n"
        : : "a" (cv));
#endif
    short scr_x, scr_y;
    struct EnginePoint ep1;
    struct EnginePoint ep2;
    char locstr[8];

    ep1.X3d = game_col_vects[cv].X1 - engn_xc;
    ep1.Z3d = game_col_vects[cv].Z1 - engn_zc;
    ep1.Y3d = game_col_vects[cv].Y1;
    ep1.Flags = 0;
    transform_point(&ep1);
    ep2.X3d = game_col_vects[cv].X2 - engn_xc;
    ep2.Z3d = game_col_vects[cv].Z2 - engn_zc;
    ep2.Y3d = game_col_vects[cv].Y2;
    ep2.Flags = 0;
    transform_point(&ep2);
    LbDrawLine(ep1.pp.X, ep1.pp.Y, ep2.pp.X, ep2.pp.Y, colour_lookup[ColLU_WHITE]);
    scr_x = ep2.pp.X + ep1.pp.X;
    scr_y = ep2.pp.Y + ep1.pp.Y;
    sprintf(locstr, "%d", cv);
    draw_text(scr_x, scr_y, locstr, colour_lookup[ColLU_PINK]);
}

void draw_engine_unk3_last(short x, short z)
{
#if 0
    asm volatile (
      "call ASM_draw_engine_unk3_last\n"
        : : "a" (x), "d" (z));
#endif
    int x_beg, x_end;
    int z_beg, z_end;
    int cx, cz;

    x_beg = (x >> 8) - (render_area_a >> 1);
    x_end = (x >> 8) + (render_area_a >> 1);
    z_beg = (z >> 8) - (render_area_b >> 1);
    z_end = (z >> 8) + (render_area_b >> 1);
    for (cx = x_beg; cx < x_end; cx++)
    {
        for (cz = z_beg; cz < z_end; cz++)
        {
          struct MyMapElement *p_mapel;
          short vl;

          if (cx < 0 || cx > 127 || cz < 0 || cz > 127)
              continue;
          p_mapel = &game_my_big_map[cx + (cz << 7)];

          vl = p_mapel->ColHead;
          while ( vl )
          {
              struct ColVectList *p_cvlist;
              short cor_x, cor_y;
              uint mapel;
              TbPixel col;

              mapel = (p_mapel - game_my_big_map);
              cor_y = (mapel / 128) << 8;
              cor_x = (mapel % 128) << 8;
              col = 0x63u;
              draw_line_transformed_at_ground(cor_x, cor_y, cor_x + 256, cor_y, col);
              draw_line_transformed_at_ground(cor_x + 256, cor_y, cor_x + 256, cor_y + 256, col);
              draw_line_transformed_at_ground(cor_x, cor_y + 256, cor_x + 256, cor_y + 256, col);
              draw_line_transformed_at_ground(cor_x, cor_y + 256, cor_x, cor_y, col);

              p_cvlist = &game_col_vects_list[vl];
              draw_unkn1_bar(p_cvlist->Vect);
              vl = p_cvlist->NextColList & 0x7FFF;
              cor_x += 128;
              cor_y += 128;

              p_cvlist = &game_col_vects_list[vl];
              draw_number_transformed_at_ground(cor_x, cor_y, p_cvlist->Object);
            }
        }
    }
}

ushort dynamic_insert_vect(s32 x1, s32 y1, s32 z1, s32 x2,
  s32 y2, s32 z2, int owner, ubyte vtype)
{
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[4];
    short ret;

    stkargs[0] = (int)(intptr_t)y2;
    stkargs[1] = (int)(intptr_t)z2;
    stkargs[2] = (int)(intptr_t)owner;
    stkargs[3] = (int)(intptr_t)vtype;

    asm volatile (
      "push 12(%5)\n"
      "push 8(%5)\n"
      "push 4(%5)\n"
      "push 0(%5)\n"
      "call ASM_dynamic_insert_vect\n"
        : "=a" (ret) : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "S" (stkargs)
        : "cc", "memory");
    return ret;
}

/******************************************************************************/
