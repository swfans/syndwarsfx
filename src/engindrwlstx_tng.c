/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engindrwlstx.c
 *     Drawlists execution for the 3D engine.
 * @par Purpose:
 *     Implements functions for executing previously made drawlists,
 *     meaning the actual drawing based on primitives in the list.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 12 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engindrwlstx.h"

#include "bfkeybd.h"
#include "bfgentab.h"
#include "bfsprite.h"
#include "insspr.h"
#include <assert.h>
#include <stdlib.h>

#include "enginfloor.h"
#include "enginpeff.h"
#include "enginprops.h"
#include "render_gpoly.h"
#include "enginzoom.h"

#include "bigmap.h"
#include "display.h"
#include "drawtext.h"
#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "engindrwlstm_wrp.h"
#include "enginlights.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginshadws.h"
#include "enginshrapn.h"
#include "engintrns.h"
#include "frame_sprani.h"
#include "game_options.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "hud_panel.h"
#include "packet.h"
#include "player.h"
#include "scandraw.h"
#include "thing.h"
#include "swlog.h"
#include "vehicle.h"
/******************************************************************************/

extern ubyte byte_176D49;

extern short word_1A5834;
extern short word_1A5836;

/******************************************************************************/
// from engindrwlstx_spr
void draw_hud_frame_on_screen_unscaled_but_scale_pos(short scr_x, short scr_y, ushort frm, int sscale);
void draw_hud_frame_on_screen(short scr_x, short scr_y, ushort frm, int sscale);

ushort number_player_get_frame(struct Thing *p_person, ubyte n)
{
    struct PanelStyle *p_style;
    ushort ani_mdsh, ani_base;
    ushort frm;
    ushort screen_scale;

    p_style = game_panel_style;

    ani_base = p_style->AgentNumAnim;
    screen_scale = min(lbDisplay.GraphicsScreenHeight / 200, p_style->AgentNumDetails);
    if (screen_scale > 0)
        screen_scale--;
    ani_mdsh = 4 * screen_scale;

    frm = nstart_ani[ani_base + ani_mdsh + n];
    {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
        if (p_locplayer->DoubleMode == 0)
        {
            if ((p_person->ThingOffset != (ThingIdx)p_locplayer->DirectControl[0]) || ((render_anim_turn & 4) != 0))
            {
                frm = frame[frm].Next;
            }
            else
            {
                ushort i;
                for (i = 0; i <= (render_anim_turn & 3); i++)
                    frm = frame[frm].Next;
            }
        }
    }
    return frm;
}

void draw_frame_on_map_coords(MapCoord cor_x, MapCoord cor_y, MapCoord cor_z,
  short scr_sh_x, short scr_sh_y, ushort frm, TbBool unscaled)
{
    struct ShEnginePoint sp;
    int cor_dt_x, cor_dt_y, cor_dt_z;

    cor_dt_x = cor_x - engn_xc;
    cor_dt_y = cor_y;
    cor_dt_z = cor_z - engn_zc;
    if ((cor_dt_x > TILE_TO_MAPCOORD(render_area_a,0)) ||
      (cor_dt_z > TILE_TO_MAPCOORD(render_area_b,0))) {
        return;
    }

    transform_shpoint(&sp, cor_dt_x, 8 * cor_dt_y - 8 * engn_yc, cor_dt_z);
    sp.X += ((overall_scale * scr_sh_x) >> 8);
    sp.Y += ((overall_scale * scr_sh_y) >> 8);

    //TODO switch to drawlists
    if (unscaled)
        draw_hud_frame_on_screen_unscaled_but_scale_pos(sp.X, sp.Y, frm, overall_scale);
    else
        draw_hud_frame_on_screen(sp.X, sp.Y, frm, overall_scale);
}

void number_player(struct Thing *p_person, ubyte n)
{
    int tng_cor_x, tng_cor_y, tng_cor_z;
    int shift_x, shift_y;
    ushort frm;
    TbBool unscaled;

    frm = number_player_get_frame(p_person, n);

    if ((p_person->Flag2 & TgF2_Unkn0002) != 0)
        return;

    if (person_is_in_a_train(p_person))
    {
        tng_cor_x = PRCCOORD_TO_MAPCOORD(p_person->X);
        tng_cor_y = PRCCOORD_TO_MAPCOORD(p_person->Y);
        tng_cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z);
    }
    else if ((p_person->Flag & TngF_Unkn4000) != 0)
    {
        struct Thing *p_vehicle;
        p_vehicle = &things[p_person->U.UPerson.Vehicle];
        tng_cor_x = PRCCOORD_TO_MAPCOORD(p_vehicle->X);
        tng_cor_y = PRCCOORD_TO_MAPCOORD(p_vehicle->Y);
        tng_cor_z = PRCCOORD_TO_MAPCOORD(p_vehicle->Z);
    }
    else
    {
        tng_cor_x = PRCCOORD_TO_MAPCOORD(p_person->X);
        tng_cor_y = PRCCOORD_TO_MAPCOORD(p_person->Y);
        tng_cor_z = PRCCOORD_TO_MAPCOORD(p_person->Z);
    }

    if ((p_person->Flag & TngF_InVehicle) != 0)
    {
        shift_x = 7 * n - 14;
    }
    else
    {
        ubyte angl;
        angl = (p_person->U.UObject.Angle + 2 - byte_176D49 + 8) & 7;
        shift_x = -lbSinTable[256 * angl + LbFPMath_PI/2] >> 14;
        if ((p_person->Flag2 & TgF2_Unkn00080000) == 0)
            shift_x /= 2;
    }
    shift_y = 0;

    unscaled = (lbDisplay.GraphicsScreenHeight < 400);

    draw_frame_on_map_coords(tng_cor_x, tng_cor_y, tng_cor_z,
      shift_x, shift_y, frm, unscaled);
}
/******************************************************************************/
