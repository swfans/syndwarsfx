/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file hud_draw.c
 *     Ingame Heads-Up Display drawing routines.
 * @par Purpose:
 *     Implement functions drawing the HUD as a whole.
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
#include "hud_draw.h"

#include "bfmath.h"
#include "bfscreen.h"
#include "bftext.h"

#include "engincolour.h"

#include "engindrwlstx_tng.h"
#include "game_options.h"
#include "game_sprts.h"
#include "game.h"
#include "hud_panel.h"
#include "hud_target.h"
#include "huddrwlstm.h"
#include "mydraw.h"
#include "packet.h"
#include "player.h"
#include "scandraw.h"
#include "swlog.h"
#include "thing.h"
/******************************************************************************/

struct Thing *mouse_over_unkn2_tng = NULL;
int mouse_over_unkn2_x;
int mouse_over_unkn2_y;

/******************************************************************************/

void draw_mapcoord_line_directed_to_target(int probe_x, int probe_y, int probe_z,
  int target_x, int target_y, int target_z, TbPixel colour)
{
    int dist_x, dist_y, dist_z;
    int tdist;
    int cur_x, cur_y, cur_z;
    int prv_x, prv_z;
    ushort step, steps_tot;
    TbPixel col2;

    dist_x = target_x - probe_x;
    dist_y = target_y - probe_y;
    dist_z = target_z - probe_z;
    tdist = LbSqrL(dist_x * dist_x + dist_z * dist_z);

    if (tdist == 0) {
        return;
    }
    dist_x = 50 * dist_x / tdist;
    dist_y = 50 * dist_y / tdist;
    dist_z = 50 * dist_z / tdist;
    col2 = colour - 1;
    steps_tot = tdist / 50;

    // Arrows toward target
    for (step = 1; step < steps_tot; step += 8)
    {
        cur_x = probe_x + dist_x * (step);
        cur_y = probe_y + dist_y * (step);
        cur_z = probe_z + dist_z * (step);
        prv_x = probe_x + dist_x * (step - 1);
        prv_z = probe_z + dist_z * (step - 1);

        enlist_hud_draw_mapcoord_line_only_whole(cur_x, cur_y, cur_z,
          prv_x + dist_z, cur_y, prv_z - dist_x, 0, 1, col2);

        enlist_hud_draw_mapcoord_line_only_whole(cur_x, cur_y, cur_z,
          prv_x - dist_z, cur_y, prv_z + dist_x, 0, 1, col2);
    }

    // Line from floor under mouse to target
    enlist_hud_draw_mapcoord_line_only_whole(probe_x, probe_y, probe_z,
      target_x, target_y, target_z, 0, 1, colour);
}

TbBool enlist_hud_draw_line_from_thing_to_mapcoord(ThingIdx thing,
  short cor2_x, short cor2_y_m8, short cor2_z, ushort drwflags, ubyte colkp)
{
    short cor1_x, cor1_y_m8, cor1_z;
    short thickness;

    get_thing_position_mapcoords(&cor1_x, &cor1_y_m8, &cor1_z, thing);
    thickness = 1;

    return enlist_hud_draw_mapcoord_line(cor1_x, cor1_y_m8, cor1_z,
      cor2_x, cor2_y_m8, cor2_z, drwflags, thickness, colour_lookup[colkp]);
}

TbBool enlist_hud_draw_line_from_thing_ptr_to_mapcoord(struct Thing *p_thing,
  short cor2_x, short cor2_y_m8, short cor2_z, ushort drwflags, ubyte colkp)
{
    if ((p_thing <= &things[0]) || (p_thing >= &things[THINGS_LIMIT]))
        return false;
    return enlist_hud_draw_line_from_thing_to_mapcoord(p_thing->ThingOffset,
      cor2_x, cor2_y_m8, cor2_z, drwflags, colkp);
}

TbBool enlist_hud_draw_line_from_thing_to_scrcoord(ThingIdx thing,
  short scr_x, short scr_y, ushort drwflags, ubyte colkp)
{
    short cor1_x, cor1_y_m8, cor1_z;
    short thickness;

    get_thing_position_mapcoords(&cor1_x, &cor1_y_m8, &cor1_z, thing);
    thickness = 1;

    return enlist_hud_draw_mapcoord_to_scrcoord_line(scr_x, scr_y,
      cor1_x, cor1_y_m8, cor1_z, drwflags, thickness, colour_lookup[colkp]);
}

TbBool enlist_hud_draw_line_from_thing_ptr_to_scrcoord(struct Thing *p_thing,
  short scr_x, short scr_y, ushort drwflags, ubyte colkp)
{
    if ((p_thing <= &things[0]) || (p_thing >= &things[THINGS_LIMIT]))
        return false;
    return enlist_hud_draw_line_from_thing_to_scrcoord(p_thing->ThingOffset,
      scr_x, scr_y, drwflags, colkp);
}

void set_netplayer_name_text_over_thing(ScrCoord x, ScrCoord y,
  struct Thing *p_thing)
{
    mouse_over_unkn2_tng = p_thing;
    mouse_over_unkn2_x = x;
    mouse_over_unkn2_y = y;
}

void draw_netplayer_name_text_over_thing(void)
{
    char locstr[32];
    struct Thing *p_thing;
    int scr_x, scr_y;
    int width, height;
    int sh_x, sh_y;
    int units_per_px;
    PlayerIdx plyr;
    TbPixel colour;

    p_thing = mouse_over_unkn2_tng;

    if (p_thing == NULL)
        return;

    mouse_over_unkn2_tng = NULL;

    plyr = p_thing->U.UPerson.ComCur >> 2;

    strncpy(locstr, net_player_names[plyr], sizeof(locstr)-1);
    locstr[sizeof(locstr)-1] = '\0';
    my_str_to_upper(locstr);

    units_per_px = 16;
    sh_x = sh_y = 0;
    colour = net_player_colours[plyr];
    width = LbTextStringWidthResized(locstr, units_per_px);
    height = my_char_height('A') * units_per_px / 16;
    scr_x = mouse_over_unkn2_x - (width >> 1);
    if (scr_x < 0) {
        sh_x = scr_x;
        scr_x = 0;
    }
    else if (scr_x + width > lbDisplay.GraphicsScreenWidth) {
        scr_x = lbDisplay.GraphicsScreenWidth - width;
    }
    scr_y = mouse_over_unkn2_y;
    enlist_hud_draw_clipped_text(scr_x, scr_y, width, height,
      sh_x, sh_y, small_font, locstr, units_per_px, 32, colour);
}

void draw_hud(int dcthing)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    if (!panel_any_visible()) {
        return;
    }

    show_goto_point(0);
    init_draw_target();

    {
        struct Thing *p_mothing;

        p_mothing = &things[p_locplayer->DirectControl[mouser]];
        if (!lbDisplay.MRightButton
          || (p_mothing->PTarget == NULL)
          || current_weapon_has_targetting(p_mothing)
          || (p_mothing->U.UPerson.WeaponTimer < 14)) {
            p_locplayer->field_102 = 0;
        }
    }

    draw_hud_lock_target();

    if (ingame.DisplayMode == 50)
    {
        short plagent;
        short target;

        for (plagent = 0; plagent < playable_agents; plagent++)
        {
            struct Thing *p_agent;

            p_agent = p_locplayer->MyAgent[plagent];
            number_player(p_agent, plagent);
            if ((p_agent->Flag & TngF_SelectedAgent) != 0)
            {
                ushort ctlmode;
                ctlmode = user_input_control_mode_get(local_player_no, plagent);
                if (ctlmode != UInpCtr_Mouse)
                {
                    if (p_agent->PTarget != NULL)
                        draw_target_person(p_agent->PTarget, 2);
                }
            }
        }

        if (!PacketRecord_IsPlayback())
        {
          draw_hud_target_mouse(dcthing);
        }

        target = p_locplayer->field_102;
        if (target > 0)
        {
            if (!thing_is_destroyed(target))
                draw_hud_target2(dcthing, target);
        }
        draw_new_panel();
    }
}

/******************************************************************************/
