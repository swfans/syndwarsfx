/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file render_gpoly.c
 *     Wrappers for rendering functions for drawing 3D view elements.
 * @par Purpose:
 *     Unified functions for rendering 3D elements, wrapping around bflibrary.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 14 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "render_gpoly.h"

#include "bfendian.h"
#include "bfgentab.h"
#include "poly.h"
#include "poly_gp.h"
#include "poly_trigp.h"

#include "privrdlog.h"
/******************************************************************************/

long gpoly_pro_enable_mode_ofs;

/******************************************************************************/
void gpoly_enable_pentium_pro(TbBool state)
{
  LOGSYNC("Pentium Pro polygon rendering %s",state?"on":"off");
  if (state)
    gpoly_pro_enable_mode_ofs = (1<<6);
  else
    gpoly_pro_enable_mode_ofs = 0;
}

void trigpoly(struct PolyPoint *point_a, struct PolyPoint *point_b, struct PolyPoint *point_c)
{
    switch (vec_mode)
    {
    case 5:
    case 27:
      draw_gpoly(point_a, point_b, point_c);
      break;
    default:
      trig(point_a, point_b, point_c);
      break;
    }
}

/******************************************************************************/
