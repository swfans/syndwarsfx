/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing_debug.c
 *     Thing related debug modes.
 * @par Purpose:
 *     Implement dump functions and debug modes for things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 11 Feb 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing.h"

#include <string.h>
#include <assert.h>
#include "bfbox.h"
#include "bfline.h"
#include "bfmath.h"
#include "bfpalette.h"

#include "drawtext.h"
#include "engincam.h"
#include "engincolour.h"
#include "engintrns.h"
#include "huddrwlstm.h"

#include "bigmap.h"
#include "building.h"
#include "command.h"
#include "drawtext_wrp.h"
#include "game.h"
#include "game_options.h"
#include "game_sprts.h"
#include "hud_draw.h"
#include "keyboard.h"
#include "pathtrig.h"
#include "pepgroup.h"
#include "scandraw.h"
#include "thing_search.h"
#include "vehicle.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/
TbBool debug_hud_things = false;

extern ubyte execute_commands;

struct Thing *p_track_thing = NULL;
struct Thing *p_track2_thing = NULL;
int dword_1DC7A4 = 0;
short dbgcmds_scroll_pos = 0;
short dbgcmds_count = 0;
short dbgcmds_hilight_cmd = -1;

s32 mfilter_nearest_debug_selectable(ThingIdx thing, short X, short Z, ThingFilterParams *params)
{
    short tng_x, tng_z;
    s32 dtX, dtZ;
    TbBool allow_hidden, basic_types;

    allow_hidden = (params->Arg1 & 0x01) != 0;
    basic_types = (params->Arg1 & 0x02) != 0;

    if (thing <= 0)
    {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        if (!allow_hidden && (p_sthing->Flag & TngF_Unkn04000000) == 0)
        {
            return INT32_MAX;
        }
        if (basic_types && (p_sthing->Type != SmTT_DROPPED_ITEM))
            return INT32_MAX;
    }
    else
    {
        struct Thing *p_thing;
        p_thing = &things[thing];
        if (!allow_hidden && (p_thing->Flag & TngF_Unkn04000000) == 0)
        {
            // Mounted guns do not have this flag set, but we still may want to allow them
            if ((p_thing->Type != TT_BUILDING) || (p_thing->SubType != SubTT_BLD_MGUN))
                return INT32_MAX;
        }
        if (basic_types)
        {
            switch (p_thing->Type)
            {
            case TT_PERSON:
            case TT_VEHICLE:
                break;
            case TT_BUILDING:
                if (p_thing->SubType == SubTT_BLD_MGUN)
                    break;
                return INT32_MAX;
            default:
                return INT32_MAX;
            }
        }
    }

    get_thing_position_mapcoords(&tng_x, NULL, &tng_z, thing);
    dtX = tng_x - X;
    dtZ = tng_z - Z;
    return (dtZ * dtZ + dtX * dtX);
}

int draw_lines_scrcoord_to_all_group_members(short scr_x, short scr_y, ushort group)
{
    struct Thing *p_thing;
    ThingIdx thing;
    short i;

    assert(group < PEOPLE_GROUPS_LIMIT);
    thing = same_type_head[256 + group];
    for (i = 0; thing > 0; thing = p_thing->LinkSameGroup, i++)
    {
        p_thing = &things[thing];
        enlist_hud_draw_line_from_thing_to_scrcoord(thing,
          scr_x, scr_y, 0, ColLU_WHITE);
    }
    return i;
}

/** Searches all existing things in order to find one for debug.
 * @param ttype Thing type, or -1 to catch all basic types, or -2 to catch all possible.
 */
ThingIdx search_for_thing_for_debug(short x, short y, short z, short ttype)
{
    ThingIdx thing;
    ThingFilterParams params;

    params.Arg1 = 0;
    if (ttype == -2) {
        params.Arg1 |= 0x01; // allow hidden things
        ttype = -1;
    } else if (ttype == -1) {
        params.Arg1 |= 0x02; // limit to basic types only
    }

    thing = find_thing_type_within_circle_with_mfilter(x, z, TILE_TO_MAPCOORD(9,0),
      ttype, -1, mfilter_nearest_debug_selectable, &params);

    return thing;
}

int select_thing_for_debug(short x, short y, short z, short ttype)
{
    char locstr[52];
    ThingIdx thing;
    short tng_x, tng_y, tng_z;
    short alt;
    ubyte colu;

    thing = search_for_thing_for_debug(x, y, z, ttype);

    alt = PRCCOORD_TO_YCOORD(alt_at_point(x, z));
    if (thing != 0) {
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, thing);
        colu = ColLU_WHITE;
    } else {
        tng_x = MAP_COORD_WIDTH / 2;
        tng_y = alt;
        tng_z = MAP_COORD_HEIGHT / 2;
        colu = ColLU_GREYMD;
    }
    draw_mapcoord_line_directed_to_target(x, alt, z, tng_x, tng_y, tng_z, colour_lookup[colu]);

    if (thing == 0)
    {
        sprintf(locstr, "MAP CENTER");
    }
    else if (thing > 0)
    {
        struct Thing *p_thing;
        p_thing = &things[thing];
        sprintf(locstr, "TH %d ID %d", thing, p_thing->U.UPerson.UniqueID);
    }
    else if (thing < 0)
    {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        sprintf(locstr, "TH %d ID %d", thing, p_sthing->UniqueID);
    }
    draw_mapcoord_simple_text_at_ground(tng_x, tng_z, locstr);
    return thing;
}

void count_fnavs(TbBool now_draw)
{
    static ushort dbgfnavs_count = 0;
    static ubyte dbgfnavs[50];

    if (!now_draw)
    {
        if (dbgfnavs_count < sizeof(dbgfnavs)/sizeof(dbgfnavs[0])) {
            dbgfnavs[dbgfnavs_count] = 0;
            dbgfnavs_count++;
        }
        return;
    }
    if ((ingame.Flags & GamF_NaviPerfInfo) != 0)
    {
        ushort i;
        TbPixel colour;
        ubyte brig;

        brig = PALETTE_FADE_LEVELS / 2;

        for (i = 0; i < dbgfnavs_count; i++)
        {
            short scr_x, scr_y, w, h;

            w = 23 * (pop1_sprites_scale + 1) / 2;
            h = 3 * (pop1_sprites_scale + 1) / 2;
            scr_x = lbDisplay.GraphicsScreenWidth - 29 * (pop1_sprites_scale + 1) / 2;
            scr_y = lbDisplay.GraphicsScreenHeight - (29 + 6 * i) * (pop1_sprites_scale + 1) / 2;
            if (dbgfnavs[i])
                colour = colour_lookup[ColLU_GREEN];
            else
                colour = colour_lookup[ColLU_RED];
            enlist_hud_draw_box(scr_x, scr_y, w, h, 0, brig, colour);
        }
    }
    dbgfnavs_count = 0;
}

void navi_onscreen_debug(TbBool now_draw)
{
    static ushort navuse_count = 0;

    if (!now_draw)
    {
        navuse_count++;
        return;
    }
    if ((ingame.Flags & GamF_NaviPerfInfo) != 0)
    {
        ushort i;
        TbPixel colour;
        ubyte brig;

        brig = PALETTE_FADE_LEVELS / 2;

        for (i = 0; i < navuse_count; i++)
        {
            short scr_x, scr_y, w, h;

            w = 25 * (pop1_sprites_scale + 1) / 2;
            h = 5 * (pop1_sprites_scale + 1) / 2;
            scr_x = lbDisplay.GraphicsScreenWidth - 30 * (pop1_sprites_scale + 1) / 2;
            scr_y = lbDisplay.GraphicsScreenHeight - (30 + 6 * i) * (pop1_sprites_scale + 1) / 2;
            colour = colour_lookup[ColLU_BLUE];
            enlist_hud_draw_box(scr_x, scr_y, w, h, 0, brig, colour);
        }
    }
    navuse_count = 0;
    count_fnavs(1);
}

/** Make lines to target things or circles around target areas to visualize person command.
 */
int person_command_dbg_point_to_target(short x, short y, ushort cmd, struct Thing *p_person)
{
    struct Command *p_cmd;
    short tng_x, tng_y, tng_z;

    p_cmd = &game_commands[cmd];
    switch (p_cmd->Type)
    {
    case PCmd_GO_TO_PERSON:
    case PCmd_FOLLOW_PERSON:
    case PCmd_SUPPORT_PERSON:
    case PCmd_PROTECT_PERSON:
    case PCmd_USE_VEHICLE:
    case PCmd_WAIT_P_V_DEAD:
    case PCmd_UNTIL_P_V_DEAD:
    case PCmd_UNTIL_OBJT_DESTROY:
    case PCmd_PING_P_V:
    case PCmd_WAIT_OBJT_DESTROY:
    case PCmd_WAND_OBJT_DESTROY:
    case PCmd_WAIT_P_PERSUADE:
        enlist_hud_draw_line_from_thing_to_scrcoord(p_cmd->OtherThing, x, y, 0, ColLU_WHITE);
        return 1;
    case PCmd_KILL_PERSON:
    case PCmd_CAMERA_TRACK:
        enlist_hud_draw_line_from_thing_to_scrcoord(p_cmd->OtherThing, x, y, 0, ColLU_RED);
        return 1;
    case PCmd_GO_TO_POINT:
    case PCmd_GOTOPOINT_FACE:
    case PCmd_RUN_TO_POINT:
        enlist_hud_draw_mapcoord_to_scrcoord_line(x, y,
          p_cmd->X, p_cmd->Y, p_cmd->Z, 0, 1, colour_lookup[ColLU_RED]);
        enlist_hud_draw_mapcoord_flat_circle(p_cmd->X, p_cmd->Y, p_cmd->Z,
          p_cmd->Arg1 << 6, 0, 1, colour_lookup[ColLU_RED]);
        return 1;
    case PCmd_KILL_MEM_GROUP:
    case PCmd_KILL_ALL_GROUP:
    case PCmd_UNTRUCE_GROUP:
      if ((lbShift & KMod_ALT) != 0)
          draw_lines_scrcoord_to_all_group_members(x, y, p_cmd->OtherThing);
      return 1;
    case PCmd_BLOCK_PERSON:
    case PCmd_SCARE_PERSON:
        enlist_hud_draw_line_from_thing_to_scrcoord(p_cmd->OtherThing, x, y, 0, ColLU_BLUE);
        return 1;
    case PCmd_PERSUADE_PERSON:
    case PCmd_GET_ITEM:
        enlist_hud_draw_line_from_thing_to_scrcoord(p_cmd->OtherThing, x, y, 0, ColLU_YELLOW);
        return 1;
    case PCmd_USE_WEAPON:
        enlist_hud_draw_mapcoord_to_scrcoord_line(x, y,
          p_cmd->X, p_cmd->Y, p_cmd->Z, 0, 1, colour_lookup[ColLU_RED]);
        return 1;
    case PCmd_CATCH_FERRY:
    case PCmd_EXIT_FERRY:
        enlist_hud_draw_mapcoord_to_scrcoord_line(x, y,
          p_cmd->X, p_cmd->Y, p_cmd->Z, 0, 1, colour_lookup[ColLU_RED]);
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0) {
            enlist_hud_draw_mapcoord_flat_box(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 - p_cmd->X, p_cmd->Time - p_cmd->Z, Lb_SPRITE_OUTLINE, 1, 2u);
        } else {
            enlist_hud_draw_mapcoord_flat_circle(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 << 6, Lb_SPRITE_OUTLINE, 1, colour_lookup[ColLU_RED]);
        }
        return 1;
    case PCmd_PROTECT_MEM_G:
    case PCmd_WAIT_ALL_G_DEAD:
    case PCmd_WAND_ALL_G_DEAD:
        if ((lbShift & KMod_ALT) != 0)
            draw_lines_scrcoord_to_all_group_members(x, y, p_cmd->OtherThing);
        return 1;
    case PCmd_KILL_EVERYONE:
    case PCmd_WITHIN_AREA:
        enlist_hud_draw_mapcoord_to_scrcoord_line(x, y,
          p_cmd->X, p_cmd->Y, p_cmd->Z, 0, 1, colour_lookup[ColLU_RED]);
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0) {
            enlist_hud_draw_mapcoord_flat_box(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 - p_cmd->X, p_cmd->Time - p_cmd->Z, Lb_SPRITE_OUTLINE, 1, 2u);
        } else {
            enlist_hud_draw_mapcoord_flat_circle(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 << 6, Lb_SPRITE_OUTLINE, 1, colour_lookup[ColLU_RED]);
        }
        return 1;
    case PCmd_WAIT_P_V_I_NEAR:
	case PCmd_WAND_P_V_I_NEAR:
    case PCmd_UNTIL_P_V_I_NEAR:
        enlist_hud_draw_line_from_thing_to_scrcoord(p_cmd->OtherThing, x, y, 0, ColLU_WHITE);
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_person->ThingOffset);

        enlist_hud_draw_mapcoord_flat_circle(tng_x, tng_y, tng_z,
          p_cmd->Arg1 << 6, 0, 1, colour_lookup[ColLU_RED]);
        return 1;
    case PCmd_UNTIL_MEM_G_NEAR:
	case PCmd_WAIT_ALL_G_NEAR:
	case PCmd_WAIT_MEM_G_NEAR:
    case PCmd_UNTIL_ALL_G_NEAR:
	case PCmd_WAND_ALL_G_NEAR:
    case PCmd_WAND_MEM_G_NEAR:
        if ((lbShift & KMod_ALT) != 0)
            draw_lines_scrcoord_to_all_group_members(x, y, p_cmd->OtherThing);
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_person->ThingOffset);
        enlist_hud_draw_mapcoord_flat_circle(tng_x, tng_y, tng_z,
          p_cmd->Arg1 << 6, 0, 1, colour_lookup[ColLU_RED]);
        return 1;
    case PCmd_UNTIL_P_V_I_ARRIVE:
    case PCmd_WAIT_P_V_I_ARRIVE:
    case PCmd_WAND_P_V_I_ARRIVE:
        enlist_hud_draw_line_from_thing_to_scrcoord(p_cmd->OtherThing, x, y, 0, ColLU_WHITE);
        enlist_hud_draw_mapcoord_to_scrcoord_line(x, y,
          p_cmd->X, p_cmd->Y, p_cmd->Z, 0, 1, colour_lookup[ColLU_RED]);
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0) {
            enlist_hud_draw_mapcoord_flat_box(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 - p_cmd->X, p_cmd->Time - p_cmd->Z,
              Lb_SPRITE_OUTLINE, 1, colour_lookup[ColLU_RED]);
        } else {
            enlist_hud_draw_mapcoord_flat_circle(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 << 6, Lb_SPRITE_OUTLINE, 1, colour_lookup[ColLU_RED]);
        }
        return 1;
    case PCmd_UNTIL_MEM_G_ARRIVE:
    case PCmd_UNTIL_ALL_G_ARRIVE:
    case PCmd_WAIT_MEM_G_ARRIVE:
	case PCmd_WAIT_ALL_G_ARRIVE:
    case PCmd_WAND_MEM_G_ARRIVE:
    case PCmd_WAND_ALL_G_ARRIVE:
        if ((lbShift & KMod_ALT) != 0)
            draw_lines_scrcoord_to_all_group_members(x, y, p_cmd->OtherThing);
        enlist_hud_draw_mapcoord_to_scrcoord_line(x, y,
          p_cmd->X, p_cmd->Y, p_cmd->Z, 0, 1, colour_lookup[ColLU_RED]);
        if ((p_cmd->Flags & PCmdF_AreaIsRect) != 0) {
            enlist_hud_draw_mapcoord_flat_box(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 - p_cmd->X, p_cmd->Time - p_cmd->Z,
              Lb_SPRITE_OUTLINE, 1, colour_lookup[ColLU_RED]);
        } else {
            enlist_hud_draw_mapcoord_flat_circle(p_cmd->X, p_cmd->Y, p_cmd->Z,
              p_cmd->Arg1 << 6, Lb_SPRITE_OUTLINE, 1, colour_lookup[ColLU_RED]);
        }
        return 1;
    default:
        break;
    }
    // TODO here we could add some general handling of simple commands, based on flags
    return 0;
}

ushort person_commands_count(ThingIdx person)
{
    struct Thing *p_person;
    ushort cmds_count;
    short cmd, cmdhead;

    p_person = &things[person];
    cmdhead = p_person->U.UPerson.ComHead;
    cmds_count = 0;
    for (cmd = cmdhead; cmd != 0; cmds_count++)
        cmd = game_commands[cmd].Next;

    return cmds_count;
}

// TODO separate get_person_commands_debug_hud_inputs() from the below
void person_commands_debug_hud(int x, int y, int w, int h, ThingIdx person, ubyte col1, ubyte col2, ubyte col3)
{
    struct Thing *p_person;
    ushort cmds_count;
    short cmd, cmdhead;
    short cy;
    short ms_y;
    ushort drwflags;
    char locstr[52];
    short box_x, box_y;
    short box_width, box_height;
    short row_height;
    short padding;
    short brig;
    int j;

    dbgcmds_hilight_cmd = -1;
    brig = PALETTE_FADE_LEVELS / 2;
    box_width = w;
    box_height = h;
    box_x = x;
    box_y = y;
    if (lbDisplay.GraphicsScreenHeight < 400) {
        row_height = 8;
        padding = 2;
    } else {
        row_height = 16;
        padding = 4;
    }

    p_person = &things[person];
    cmds_count = person_commands_count(person);
    if (cmds_count == 0)
        return;

    if (row_height * cmds_count + 2 * padding < box_height)
        box_height = row_height * cmds_count + 2 * padding;
    if ((dbgcmds_scroll_pos >> 2) > dbgcmds_count - 4)
        dbgcmds_scroll_pos = 0;
    dbgcmds_count = cmds_count;

    drwflags = Lb_SPRITE_TRANSPAR4;
    enlist_hud_draw_box(box_x, box_y, box_width, box_height, drwflags,
      brig, col3);
    drwflags = Lb_SPRITE_OUTLINE;
    enlist_hud_draw_box(box_x, box_y, box_width, box_height, drwflags,
      brig, col1);

    // Find first command to be displayed
    cmdhead = p_person->U.UPerson.ComHead;
    cmd = cmdhead;
    for (j = dbgcmds_scroll_pos >> 2; j > 0; j--)
    {
        if (cmd == 0)
            break;
        cmd = game_commands[cmd].Next;
    }

    cy = box_y + padding - row_height * (dbgcmds_scroll_pos & 3) / 4;
    // Skip half-visible command (our simple text draw does not support partial draw)
    if (dbgcmds_scroll_pos & 3) {
        if (cmd != 0)
            cmd = game_commands[cmd].Next;
        cy += row_height;
    }

    while (cmd != 0)
    {
        if (cy + row_height > box_y + box_height)
            break;

        if (mouse_move_over_box_coords(box_x + 8, cy, box_x + 8 + box_width, cy + row_height - 1))
        {
            dbgcmds_hilight_cmd = cmd;
        }

        drwflags = 0;
        if (dbgcmds_hilight_cmd == cmd)
        {
            enlist_hud_draw_box(box_x + padding, cy - 2,
              box_width - 2 * padding, row_height - 1, drwflags,
              brig, col1);
            drwflags |= Lb_SPRITE_TRANSPAR4;
        }
        if ((p_person != NULL) && (p_person->U.UPerson.ComCur == cmd))
        {
            enlist_hud_draw_box(box_x + padding, cy - 2,
              box_width - 2 * padding, row_height - 1, drwflags,
              brig, colour_lookup[ColLU_GREEN]);
        }
        lbDisplay.DrawFlags = 0;

        if (p_person != NULL) {
            person_command_dbg_point_to_target(box_x - 2 * padding, cy + row_height/3, cmd, p_person);
        }
        if (person_command_to_text(locstr, cmd, 0)) {
            draw_simple_text(box_x + 2 * padding, cy, locstr, col2);
        } else {
            cy -= row_height;
        }

        cy += row_height;
        cmd = game_commands[cmd].Next;
        ++j;
    }

    ms_y = lbDisplay.MMouseY;
    if ((ms_y < box_y - 20) && (dbgcmds_scroll_pos > 0))
        dbgcmds_scroll_pos--;
    if ((ms_y > box_y + box_height + 10) && ((dbgcmds_scroll_pos >> 2) < dbgcmds_count - (box_height / row_height)))
        dbgcmds_scroll_pos++;
}

void things_debug_hud(void)
{
    ThingIdx thing;
    short path;
    short pasngr;
    char locstr[100];
    short cmdf_scr_w, cmdf_scr_h;
    short tng_x, tng_y, tng_z;
    short scr_x, scr_y, ln;
    short cmdf_scr_x, cmdf_scr_y;
    MapCoord cor_x, cor_y, cor_z;

    map_coords_limit(&cor_x, &cor_y, &cor_z, mouse_map_x, 0, mouse_map_z);
    thing = select_thing_for_debug(cor_x, cor_y, cor_z, -1);
    // Lock on current thing
    if (is_key_pressed(KC_W, KMod_SHIFT))
    {
        clear_key_pressed(KC_W);
        dword_1DC7A4 = 0;
    }
    if (is_key_pressed(KC_W, KMod_NONE))
    {
        clear_key_pressed(KC_W);
        if (thing > 0) {
            dword_1DC7A4 = thing;
        }
    }
    if (dword_1DC7A4)
        thing = dword_1DC7A4;

    if (lbShift == KMod_SHIFT)
        return;
    if (thing == 0)
        return;

    if (lbDisplay.GraphicsScreenHeight < 400) {
        cmdf_scr_w = 160;
        cmdf_scr_h = 75;
    } else {
        cmdf_scr_w = 250;
        cmdf_scr_h = 150;
    }
    ln = 15;
    // Left column (properties dump)
    scr_x = 16 * pop1_sprites_scale;
    scr_y = 30 * pop1_sprites_scale;
    // Right column (commands list)
    if (lbDisplay.GraphicsScreenHeight < 400) {
        cmdf_scr_x = lbDisplay.GraphicsScreenWidth - cmdf_scr_w - 8 * pop1_sprites_scale;
    } else {
        cmdf_scr_x = lbDisplay.GraphicsScreenWidth - cmdf_scr_w - 32 * pop1_sprites_scale;
    }
    cmdf_scr_y = scr_y + 38 * pop1_sprites_scale;

    if (thing < 0)
    {
        struct SimpleThing *p_sthing;

        if (execute_commands)
        {
            p_sthing = &sthings[thing];
            snprintf(locstr, sizeof(locstr), "State %d ",
              (int)p_sthing->State);
            snprintf(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr), " th %d",
              (int)p_sthing->ThingOffset);
            draw_simple_text(scr_x, scr_y + ln*0, locstr, colour_lookup[ColLU_WHITE]);

            sprintf(locstr, "F  %08x SF %d F %d",
              (uint)p_sthing->Flag,
              (int)p_sthing->StartFrame,
              (int)p_sthing->Frame);
            draw_simple_text(scr_x, scr_y + ln*1, locstr, colour_lookup[ColLU_WHITE]);

            sprintf(locstr, "%s",
              thing_type_name(p_sthing->Type, p_sthing->SubType));
            draw_simple_text(cmdf_scr_x + 8 * pop1_sprites_scale, cmdf_scr_y - ln, locstr, colour_lookup[ColLU_PINK]);
        }
        return;
    }
    p_track_thing = &things[thing];
    p_track2_thing = &things[thing];
    get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, thing);
    draw_mapcoord_line_directed_to_target(mouse_map_x, mouse_map_y, mouse_map_z,  tng_x, tng_y, tng_z,
      colour_lookup[ColLU_WHITE]);
    // Show commands list
    {
        if (p_track_thing->Type == TT_PERSON)
             person_commands_debug_hud(cmdf_scr_x, cmdf_scr_y, cmdf_scr_w, cmdf_scr_h, thing,
                colour_lookup[ColLU_WHITE], colour_lookup[ColLU_RED], colour_lookup[ColLU_BLUE]);
        else if ((p_track_thing->Type == TT_VEHICLE) && (p_track_thing->U.UVehicle.PassengerHead > 0))
              person_commands_debug_hud(cmdf_scr_x, cmdf_scr_y, cmdf_scr_w, cmdf_scr_h, p_track_thing->U.UVehicle.PassengerHead,
                colour_lookup[ColLU_WHITE], colour_lookup[ColLU_RED], colour_lookup[ColLU_BLUE]);
    }
    if (execute_commands)
    {
        snprintf(locstr, sizeof(locstr), "State %d.%d ",
          (int)p_track_thing->State, (int)p_track_thing->SubState);
        switch (p_track_thing->Type)
        {
        case TT_VEHICLE:
            snprint_vehicle_state(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr), p_track_thing);
            snprintf(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr),
              " Pasng %d G %d comcur %x EG %d th %d wb %d",
              (int)veh_passenger_count(p_track_thing),
              (int)p_track_thing->U.UVehicle.Group,
              (uint)p_track_thing->U.UVehicle.ComCur,
              (int)p_track_thing->U.UVehicle.EffectiveGroup,
              (int)p_track_thing->ThingOffset,
              (int)p_track_thing->U.UVehicle.WobbleZP);
            break;
        case TT_PERSON:
            snprint_person_state(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr), p_track_thing);
            snprintf(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr),
              " Mood %d G %d comcur %x EG %d th %d am %d",
              (int)p_track_thing->U.UPerson.Mood,
              (int)p_track_thing->U.UPerson.Group,
              (uint)p_track_thing->U.UPerson.ComCur,
              (int)p_track_thing->U.UPerson.EffectiveGroup,
              (int)p_track_thing->ThingOffset,
              (int)p_track_thing->U.UPerson.AnimMode);
            break;
        case TT_BUILDING:
            //snprint_building_state(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr), p_track_thing);
            if (p_track_thing->SubType == SubTT_BLD_MGUN) {
                snprintf(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr),
                  " G %d EG %d th %d",
                 (int)p_track_thing->U.UMGun.Group,
                  (int)p_track_thing->U.UMGun.EffectiveGroup,
                  (int)p_track_thing->ThingOffset);
            } else {
                snprintf(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr),
                  " Mood %d G %d comcur %x EG %d th %d",
                  (int)p_track_thing->U.UObject.Mood,
                 (int)p_track_thing->U.UObject.Group,
                  (uint)p_track_thing->U.UObject.ComCur,
                  (int)p_track_thing->U.UObject.EffectiveGroup,
                  (int)p_track_thing->ThingOffset);
            }
            break;
        default:
            snprintf(locstr+strlen(locstr), sizeof(locstr)-strlen(locstr), " th %d",
              (int)p_track_thing->ThingOffset);
            break;
        }
        draw_simple_text(scr_x, scr_y, locstr, colour_lookup[ColLU_WHITE]);

        switch (p_track_thing->Type)
        {
        case TT_VEHICLE:
            sprintf(locstr, "F  %08x Spd %d OF %d He %d/%d Armr %d SF %d F %d",
              (uint)p_track_thing->Flag,
              (int)p_track_thing->Speed,
              (int)p_track_thing->U.UVehicle.OnFace,
              (int)p_track_thing->Health,
              (int)p_track_thing->U.UVehicle.MaxHealth,
              (int)p_track_thing->U.UVehicle.Armour,
              (int)p_track_thing->StartFrame,
              (int)p_track_thing->Frame);
            break;
        case TT_PERSON:
            sprintf(locstr, "F  %08x Spd %d OF %d He %d/%d Se %d/%d SF %d F %d",
              (uint)p_track_thing->Flag,
              (int)p_track_thing->Speed,
              (int)p_track_thing->U.UPerson.OnFace,
              (int)p_track_thing->Health,
              (int)p_track_thing->U.UPerson.MaxHealth,
              (int)p_track_thing->U.UPerson.ShieldEnergy,
              (int)p_track_thing->U.UPerson.MaxShieldEnergy,
              (int)p_track_thing->StartFrame,
              (int)p_track_thing->Frame);
            break;
        case TT_BUILDING:
            if (p_track_thing->SubType == SubTT_BLD_MGUN) {
                sprintf(locstr, "F  %08x Spd %d Obj %d He %d/%d ObN %d ShTn %d SF %d F %d",
                  (uint)p_track_thing->Flag,
                  (int)p_track_thing->Speed,
                  (int)p_track_thing->U.UMGun.Object,
                  (int)p_track_thing->Health,
                  (int)p_track_thing->U.UMGun.MaxHealth,
                  (int)p_track_thing->U.UMGun.ObjectNo,
                  (int)p_track_thing->U.UMGun.ShotTurn,
                  (int)p_track_thing->StartFrame,
                  (int)p_track_thing->Frame);
            } else {
                sprintf(locstr, "F  %08x Spd %d Obj %d He %d ToD %d Off %d,%d SF %d F %d",
                  (uint)p_track_thing->Flag,
                  (int)p_track_thing->Speed,
                  (int)p_track_thing->U.UObject.Object,
                  (int)p_track_thing->Health,
                  (int)p_track_thing->U.UObject.TokenDir,
                  (int)p_track_thing->U.UObject.OffX,
                  (int)p_track_thing->U.UObject.OffZ,
                  (int)p_track_thing->StartFrame,
                  (int)p_track_thing->Frame);
            }
            break;
        default:
            sprintf(locstr, "F  %08x Spd %d He %d SF %d F %d",
              (uint)p_track_thing->Flag,
              (int)p_track_thing->Speed,
              (int)p_track_thing->Health,
              (int)p_track_thing->StartFrame,
              (int)p_track_thing->Frame);
            break;
        }
        draw_simple_text(scr_x, scr_y + ln*1, locstr, colour_lookup[ColLU_WHITE]);

        switch (p_track_thing->Type)
        {
        case TT_VEHICLE:
            sprintf(locstr, "F2 %08x mi %d TN %d poww %x wt %d PC %d",
              (uint)p_track_thing->Flag2,
              (int)p_track_thing->U.UVehicle.MatrixIndex,
              (int)p_track_thing->U.UVehicle.TNode,
              (uint)p_track_thing->U.UVehicle.PissedOffWithWaiting,
              (int)p_track_thing->U.UVehicle.WeaponTurn,
              my_paths[p_track_thing->U.UVehicle.PathIndex].Flag - p_track_thing->PathOffset);
            if (p_track_thing->SubType == SubTT_VEH_TANK) {
                struct Thing *p_mgun;
                p_mgun = &things[p_track_thing->U.UVehicle.SubThing];
                sprintf(locstr+strlen(locstr), " MGun %d F %08x SF %d F %d ang %d sht %d",
                  (int)p_track_thing->U.UVehicle.SubThing,
                  (uint)p_mgun->Flag,
                  (int)p_mgun->StartFrame,
                  (int)p_mgun->Frame,
                  (int)p_mgun->U.UMGun.AngleY,
                  (int)p_mgun->U.UMGun.ShotTurn);
            }
            break;
        case TT_PERSON:
            sprintf(locstr, "F2 %08x F3 %x cw %d wc %x WE %d wti %d wt %d PC %d",
              (uint)p_track_thing->Flag2,
              (uint)p_track_thing->U.UPerson.Flag3,
              (int)p_track_thing->U.UPerson.CurrentWeapon,
              (uint)p_track_thing->U.UPerson.WeaponsCarried,
              (int)p_track_thing->U.UPerson.Energy,
              (int)p_track_thing->U.UPerson.WeaponTimer,
              (int)p_track_thing->U.UPerson.WeaponTurn,
              my_paths[p_track_thing->U.UPerson.PathIndex].Flag - p_track_thing->PathOffset);
            break;
        default:
            sprintf(locstr, "F2 %08x",
              (uint)p_track_thing->Flag2);
            break;
        }
        draw_simple_text(scr_x, scr_y + ln*2, locstr, colour_lookup[ColLU_WHITE]);

        // Print targets separately as we need their positions to draw line to the things they relate to

        short trg2_scr_x, trg2_scr_y;
        sprintf(locstr, "Targ2 %d", (int)p_track_thing->U.UPerson.Target2);
        trg2_scr_x = scr_x;
        trg2_scr_y = scr_y + ln*3;
        draw_simple_text(trg2_scr_x, trg2_scr_y, locstr, colour_lookup[ColLU_WHITE]);

        short targ_scr_x, targ_scr_y;
        sprintf(locstr, "pTarg %x",  (uint)p_track_thing->PTarget);
        targ_scr_x = scr_x + 45;
        targ_scr_y = scr_y + ln*3;
        draw_simple_text(targ_scr_x, targ_scr_y, locstr, colour_lookup[ColLU_WHITE]);

        short gtti_scr_x, gtti_scr_y;
        sprintf(locstr, "gotoTI %d", (int)p_track_thing->GotoThingIndex);
        gtti_scr_x = scr_x + 110;
        gtti_scr_y = scr_y + ln*3;
        draw_simple_text(gtti_scr_x, gtti_scr_y, locstr, colour_lookup[ColLU_WHITE]);

        if (p_track_thing->Flag & TngF_DangerFlee)
            draw_simple_text(scr_x + 0, scr_y + ln*4, "Da", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_WepRecoil)
            draw_simple_text(scr_x + 20, scr_y + ln*4, "Re", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_Unkn00020000)
            draw_simple_text(scr_x + 40, scr_y + ln*4, "Si", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_Destroyed)
            draw_simple_text(scr_x + 60, scr_y + ln*4, "De", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_WepCharging)
            draw_simple_text(scr_x + 80, scr_y + ln*4, "Ch", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_Unkn0040)
            draw_simple_text(scr_x + 100, scr_y + ln*4, "CI", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_ShootAtPos)
            draw_simple_text(scr_x + 120, scr_y + ln*4, "SAP", colour_lookup[ColLU_WHITE]);
        if (p_track_thing->Flag & TngF_StationrSht)
            draw_simple_text(scr_x + 160, scr_y + ln*4, "Sta", colour_lookup[ColLU_RED]);
        if (p_track_thing->Flag & TngF_TriggerUse)
            draw_simple_text(scr_x + 230, scr_y + ln*4, "TRIG", colour_lookup[ColLU_WHITE]);

        switch (p_track_thing->Type)
        {
        case TT_VEHICLE:
                pasngr = p_track_thing->U.UVehicle.PassengerHead;
                if (pasngr == 0) {
                    sprintf(locstr, "%s: empty",
                      thing_type_name(p_track_thing->Type, p_track_thing->SubType));
                } else if (p_track_thing->U.UVehicle.PassengerHead > 0) {
                    struct Thing *p_pasngr;

                    p_pasngr = &things[pasngr];
                    sprintf(locstr, "%s: passenger %s offs %d lastdist %d",
                      thing_type_name(p_track_thing->Type, p_track_thing->SubType),
                      thing_type_name(p_pasngr->Type, p_pasngr->SubType),
                      (int)pasngr, (int)p_pasngr->U.UPerson.LastDist);
                } else { // Not expected to happen; but just in case
                    struct SimpleThing *p_spasngr;

                    p_spasngr = &sthings[pasngr];
                    sprintf(locstr, "%s: passenger %s offs %d",
                      thing_type_name(p_track_thing->Type, p_track_thing->SubType),
                      thing_type_name(p_spasngr->Type, p_spasngr->SubType), (int)pasngr);
                }
            draw_simple_text(cmdf_scr_x + 8 * pop1_sprites_scale, cmdf_scr_y - ln, locstr, colour_lookup[ColLU_RED]);
            break;
        case TT_PERSON:
            sprintf(locstr, "%s: lastdist %d VX,VZ (%d,%d)",
              thing_type_name(p_track_thing->Type, p_track_thing->SubType),
              (int)p_track_thing->U.UPerson.LastDist,
              (int)p_track_thing->VX,
              (int)p_track_thing->VZ);
            draw_simple_text(cmdf_scr_x + 8 * pop1_sprites_scale, cmdf_scr_y - ln, locstr, colour_lookup[ColLU_GREEN]);
            break;
        default:
            sprintf(locstr, "%s",
              thing_type_name(p_track_thing->Type, p_track_thing->SubType));
            draw_simple_text(cmdf_scr_x + 8 * pop1_sprites_scale, cmdf_scr_y - ln, locstr, colour_lookup[ColLU_PINK]);
            break;
        }

        sprintf(locstr, "b %d l %d a %d br %d sk %d",
          (int)cybmod_chest_level(&p_track_thing->U.UPerson.UMod),
          (int)cybmod_legs_level(&p_track_thing->U.UPerson.UMod),
          (int)cybmod_arms_level(&p_track_thing->U.UPerson.UMod),
          (int)cybmod_brain_level(&p_track_thing->U.UPerson.UMod),
          (int)cybmod_skin_level(&p_track_thing->U.UPerson.UMod));
        draw_simple_text(scr_x, scr_y + ln*5, locstr, colour_lookup[ColLU_WHITE]);

        sprintf(locstr, "T1 %d T2 %d ct %d RT %d BC %d",
          (int)p_track_thing->Timer1,
          (int)p_track_thing->U.UPerson.Timer2,
          (int)p_track_thing->U.UPerson.ComTimer,
          (int)p_track_thing->U.UPerson.RecoilTimer,
          (int)p_track_thing->U.UPerson.BumpCount);
        draw_simple_text(scr_x, scr_y + ln*6, locstr, colour_lookup[ColLU_WHITE]);

        path = p_track_thing->U.UPerson.PathIndex;
        if (path != 0)
        {
            short cy;

            cy = scr_y + ln*7 + ln/2;
            while (path != 0)
            {
                sprintf(locstr, " n %d  f %d  x %d z %d",
                  (int)path,
                  (int)my_paths[path].Flag,
                  (int)my_paths[path].X[0] >> 8,
                  (int)my_paths[path].Z[0] >> 8);
                draw_simple_text(scr_x + 22, cy, locstr, colour_lookup[0]);
                draw_simple_text(scr_x + 20, cy, locstr, colour_lookup[ColLU_WHITE]);
                path = my_paths[path].Next;
                cy += ln;
            }
        }

        if (p_track_thing->U.UPerson.Target2 != 0)
        {
            enlist_hud_draw_line_from_thing_to_scrcoord(p_track_thing->U.UPerson.Target2,
              trg2_scr_x, trg2_scr_y + ln/2, 0, ColLU_RED);
        }

        if (p_track_thing->GotoThingIndex != 0)
        {
            enlist_hud_draw_line_from_thing_to_scrcoord(p_track_thing->GotoThingIndex,
              gtti_scr_x, gtti_scr_y + ln/2, 0, ColLU_GREEN);
        }

        if (p_track_thing->PTarget != NULL)
        {
            enlist_hud_draw_line_from_thing_ptr_to_mapcoord(p_track_thing->PTarget,
              tng_x, tng_y, tng_z, 0, ColLU_BLUE);
        }

        unkn_path_func_001(p_track_thing, 0);
    }
}

//TODO remove last arg and add out_len instead?
TbBool person_command_to_text(char *out, ushort cmd, ubyte a3)
{
    struct Command *p_cmd;
    struct CommandDef *p_cdef;
    struct Thing *p_thing;
    struct SimpleThing *p_sthing;
    const char *cmd_name;
    char *o;
    TbBool result;

    o = out;
    if (a3 == 1)
        return 0;

    p_cmd = &game_commands[cmd];

    if (p_cmd->Flags & PCmdF_Unkn0001) {
        sprintf(o, "%d ", cmd);
        o += strlen(o);
    }
    if (p_cmd->Flags & PCmdF_RunUntil) {
        sprintf(o, "U ");
        o += strlen(o);
    }
    if ( p_cmd->Flags & PCmdF_IsUntil) {
        sprintf(o, "IU ");
        o += strlen(o);
    }
    if ( p_cmd->Flags & PCmdF_RevertFunct) {
        sprintf(o, "!!!");
        o += strlen(o);
    }

    p_cdef = &command_defs[p_cmd->Type];
    cmd_name = p_cdef->CmdName;
    if (strncmp(cmd_name, "TNG_CMD_", 8) == 0)
        cmd_name += 8;

    switch (p_cmd->Type)
    {
    case PCmd_GO_TO_POINT:
      sprintf(o, "%s X%d Y%d Z%d", cmd_name, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_KILL_MEM_GROUP:
      sprintf(o, "KILL %d MEM GROUP %d", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_PERSUADE_MEM_GROUP:
      sprintf(o, "PERSUADE %d MEM G %d", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_KILL_ALL_GROUP:
    case PCmd_PERSUADE_ALL_GROUP:
    case PCmd_WAND_AVOID_GROUP:
    case PCmd_PROTECT_MEM_G:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_GO_TO_PERSON:
    case PCmd_KILL_PERSON:
    case PCmd_PERSUADE_PERSON:
    case PCmd_BLOCK_PERSON:
    case PCmd_SCARE_PERSON:
    case PCmd_FOLLOW_PERSON:
    case PCmd_SUPPORT_PERSON:
    case PCmd_PROTECT_PERSON:
    case PCmd_AVOID_PERSON:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "%s %d idx %d", cmd_name, p_thing->U.UPerson.UniqueID, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_HIDE:
    case PCmd_UNKN16:
      sprintf(o, "%s", cmd_name);
      result = 1;
      break;
    case PCmd_GET_ITEM:
    case PCmd_DROP_SPEC_ITEM:
      p_sthing = &sthings[p_cmd->OtherThing];
      sprintf(o, "%s %d", cmd_name, p_sthing->UniqueID);
      result = 1;
      break;
    case PCmd_USE_WEAPON:
    case PCmd_DROP_WEAPON:
    case PCmd_SELECT_WEAPON:
      sprintf(o, "%s %s", cmd_name, weapon_codename(p_cmd->OtherThing));
      result = 1;
      break;
    case PCmd_DESTROY_BUILDING:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_USE_VEHICLE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "%s %d", cmd_name, p_thing->U.UVehicle.UniqueID);
      result = 1;
      break;
    case PCmd_EXIT_VEHICLE:
    case PCmd_PING_EXIST:
    case PCmd_SELF_DESTRUCT:
    case PCmd_KILL_EVERYONE:
    case PCmd_GUARD_OFF:
      sprintf(o, "%s", cmd_name);
      result = 1;
      break;
    case PCmd_CATCH_TRAIN:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_OPEN_DOME:
    case PCmd_CLOSE_DOME:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_CATCH_FERRY:
    case PCmd_EXIT_FERRY:
      sprintf(o, "%s X%d Z%d", cmd_name, p_cmd->X, p_cmd->Z);
      result = 1;
      break;
    case PCmd_GOTOPOINT_FACE:
      sprintf(o, "%s %d X%d Z%d", cmd_name, p_cmd->OtherThing, p_cmd->X, p_cmd->Z);
      result = 1;
      break;
    case PCmd_RUN_TO_POINT:
      sprintf(o, "%s X%d Z%d", cmd_name, p_cmd->X, p_cmd->Z);
      result = 1;
      break;
    case PCmd_EXECUTE_COMS:
      sprintf(o, "%s (plyr)", cmd_name);
      result = 1;
      break;
    case PCmd_WAIT_P_V_DEAD:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "WAIT P/V %d DEAD", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_WAIT_MEM_G_DEAD:
      sprintf(o, "WAIT %d MEM G %d DEAD", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_ALL_G_DEAD:
    case PCmd_WAIT_ALL_G_ARRIVE:
    case PCmd_WAIT_ALL_G_PERSUADE:
    case PCmd_WAND_ALL_G_DEAD:
    case PCmd_WAND_ALL_G_ARRIVE:
    case PCmd_WAND_ALL_G_PERSUADE:
    case PCmd_UNTIL_ALL_G_DEAD:
    case PCmd_UNTIL_ALL_G_ARRIVE:
    case PCmd_UNTIL_ALL_G_PERSUADE:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_P_V_I_NEAR:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "WAIT P/V/I %d NEAR", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_WAIT_MEM_G_NEAR:
      sprintf(o, "WAIT %d MEM G %d NEAR", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_ALL_G_NEAR:
      sprintf(o, "WAIT ALL G %d NEAR", p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_P_V_I_ARRIVE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "WAIT P/V/I %d ARRIVES X%d Y%d Z%d",
        p_thing->U.UPerson.UniqueID, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_WAIT_MEM_G_ARRIVE:
      sprintf(o, "WAIT %d MEM G %d ARRIVE", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_P_PERSUADE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "%s %d", cmd_name, p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_WAIT_MEM_G_PERSUADE:
      sprintf(o, "WAIT %d MEM G %d PERSUADED", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_MISSION_FAIL:
    case PCmd_WAIT_MISSION_START:
    case PCmd_WAIT_MISSION_SUCC:
    case PCmd_WAND_MISSION_SUCC:
    case PCmd_WAND_MISSION_FAIL:
    case PCmd_WAND_MISSION_START:
    case PCmd_UNTIL_MISSION_SUCC:
    case PCmd_UNTIL_MISSION_FAIL:
    case PCmd_UNTIL_MISSION_START:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAIT_OBJT_DESTROY:
      sprintf(o, "%s %d X%d Y%d Z%d", cmd_name,
        p_cmd->OtherThing, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_WAIT_TIME:
      sprintf(o, "%s %d", cmd_name, p_cmd->Time);
      result = 1;
      break;
    case PCmd_WAND_P_V_DEAD:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "WAND P/V %d DEAD", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_WAND_MEM_G_DEAD:
      sprintf(o, "WAND %d MEM G %d DEAD", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAND_P_V_I_NEAR:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "WAND P/V/I %d NEAR", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_WAND_MEM_G_NEAR:
      sprintf(o, "WAND %d MEM G %d NEAR", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAND_ALL_G_NEAR:
      sprintf(o, "WAND ALL G %d NEAR", p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAND_P_V_I_ARRIVE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "WAND P/V/I %d ARRIVES X%d Y%d Z%d",
        p_thing->U.UPerson.UniqueID, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_WAND_MEM_G_ARRIVE:
      sprintf(o, "WAND %d MEM G %d ARRIVE", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAND_P_PERSUADE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "%s %d", cmd_name, p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_WAND_MEM_G_PERSUADE:
      sprintf(o, "WAND %d MEM G %d PERSUADED", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_WAND_OBJT_DESTROY:
      sprintf(o, "%s %d X%d Y%d Z%d",
        cmd_name, p_cmd->OtherThing, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_WAND_TIME:
      sprintf(o, "%s %d", cmd_name, p_cmd->Time);
      result = 1;
      break;
    case PCmd_LOOP_COM:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_P_V_DEAD:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "UNTIL P/V %d DEAD", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_UNTIL_MEM_G_DEAD:
      sprintf(o, "UNTIL %d MEM G %d DEAD", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_P_V_I_NEAR:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "UNTIL P/V/I %d NEAR", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_UNTIL_MEM_G_NEAR:
      sprintf(o, "UNTIL %d MEM G %d NEAR", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_ALL_G_NEAR:
      sprintf(o, "UNTIL ALL G %d NEAR", p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_P_V_I_ARRIVE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "UNTIL P/V/I %d ARRIVES X%d Y%d Z%d",
        p_thing->U.UPerson.UniqueID, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_UNTIL_MEM_G_ARRIVE:
      sprintf(o, "UNTIL %d MEM G %d ARRIVE", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_P_PERSUADE:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "%s %d", cmd_name, p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_UNTIL_MEM_G_PERSUADE:
      sprintf(o, "UNTIL %d MEM G %d PERSUADED", p_cmd->Arg2, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_OBJT_DESTROY:
      sprintf(o, "%s %d X%d Y%d Z%d", cmd_name, p_cmd->OtherThing, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_UNTIL_TIME:
      sprintf(o, "%s %d", cmd_name, p_cmd->Time);
      result = 1;
      break;
    case PCmd_WAIT_OBJV:
    case PCmd_WAND_OBJV:
    case PCmd_UNTIL_OBJV:
      sprintf(o, "%s", cmd_name);
      result = 1;
      break;
    case PCmd_WITHIN_AREA:
      sprintf(o, "%s X%d Y%d Z%d", cmd_name, p_cmd->X, p_cmd->Y, p_cmd->Z);
      result = 1;
      break;
    case PCmd_WITHIN_OFF:
    case PCmd_HARD_AS_AGENT:
    case PCmd_START_DANGER_MUSIC:
    case PCmd_IGNORE_ENEMIES:
      sprintf(o, "%s", cmd_name);
      result = 1;
      break;
    case PCmd_LOCK_BUILDN:
    case PCmd_UNLOCK_BUILDN:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_UNTIL_G_NOT_SEEN:
      sprintf(o, "UNTIL G %d NOT SEEN", p_cmd->Time);
      result = 1;
      break;
    case PCmd_PING_P_V:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "PING P/V %d", p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_CAMERA_TRACK:
      p_thing = &things[p_cmd->OtherThing];
      sprintf(o, "%s %d", cmd_name, p_thing->U.UPerson.UniqueID);
      result = 1;
      break;
    case PCmd_UNTRUCE_GROUP:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    case PCmd_PLAY_SAMPLE:
      sprintf(o, "%s %d", cmd_name, p_cmd->OtherThing);
      result = 1;
      break;
    default:
      result = 0;
      break;
    }
    return result;
}

/******************************************************************************/
