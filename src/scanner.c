/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file scanner.c
 *     Ingame scanner (minimap/radar) support.
 * @par Purpose:
 *     Implement functions for handling the scanner map and its state.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "scanner.h"

#include "bfgentab.h"
#include "bfmath.h"
#include "bfscreen.h"
#include "bfutility.h"

#include "app_sprite.h"
#include "engincolour.h"

#include "bigmap.h"
#include "campaign.h"
#include "player.h"
#include "thing.h"
#include "thing_search.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "lvobjctv.h"
#include "scandraw.h"
#include "swlog.h"
/******************************************************************************/
#pragma pack(1)

struct BbpAdds {
    s32 du;
    s32 dv;
};

#pragma pack()

/******************************************************************************/

extern ushort signal_count;
extern ulong turn_last; // = 999;
extern ulong SCANNER_keep_arcs;
extern ulong dword_1DB1A0;
extern struct BbpAdds SCANNER_bbpadds[16];

ushort SCANNER_base_zoom_factor = 180;
ushort SCANNER_user_zoom_factor = 192;
ubyte SCANNER_scale_dots = true;

void SCANNER_set_zoom(int zoom)
{
    if (zoom < 8)
        ingame.Scanner.Zoom = 8;
    else if (zoom > 556)
        ingame.Scanner.Zoom = 556;
    else
        ingame.Scanner.Zoom = zoom;
}

void SCANNER_init_bbpoints(void)
{
    int angle, k;
    int i;

    k = 0;
    for (i = 0; i < 16; i++, k += 2048)
    {
      angle = (k >> 4);
      SCANNER_bbpadds[i+1].du = lbSinTable[angle] >> 2;
      SCANNER_bbpadds[i+1].dv = lbSinTable[angle + 512] >> 2;
    }
}

void SCANNER_init_people_colours(void)
{
    SCANNER_people_colours[SubTT_PERS_AGENT] =
      pixmap.fade_table[24 * PALETTE_8b_COLORS + colour_lookup[ColLU_RED]];
    SCANNER_people_colours[SubTT_PERS_MERCENARY] =
      pixmap.fade_table[10 * PALETTE_8b_COLORS + colour_lookup[ColLU_PINK]];
    SCANNER_people_colours[SubTT_PERS_POLICE] =
      pixmap.fade_table[40 * PALETTE_8b_COLORS + colour_lookup[ColLU_BLUE]];
    SCANNER_people_colours[SubTT_PERS_SCIENTIST] =
      pixmap.fade_table[24 * PALETTE_8b_COLORS + colour_lookup[ColLU_YELLOW]];
    SCANNER_people_colours[SubTT_PERS_PUNK_F] =
      pixmap.fade_table[24 * PALETTE_8b_COLORS + colour_lookup[ColLU_GREEN]];
    SCANNER_people_colours[SubTT_PERS_PUNK_M] = SCANNER_people_colours[SubTT_PERS_PUNK_F];
    SCANNER_people_colours[SubTT_PERS_ZEALOT] =
      pixmap.fade_table[32 * PALETTE_8b_COLORS + colour_lookup[ColLU_WHITE]];
    SCANNER_people_colours[SubTT_PERS_HIGH_PRIEST] = SCANNER_people_colours[SubTT_PERS_ZEALOT];
    SCANNER_people_colours[SubTT_PERS_MECH_SPIDER] =  SCANNER_people_colours[SubTT_PERS_ZEALOT];
    SCANNER_people_colours[SubTT_PERS_SHADY_M] =
      pixmap.fade_table[40 * PALETTE_8b_COLORS + colour_lookup[ColLU_GREYMD]];
    SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M] =
      pixmap.fade_table[32 * PALETTE_8b_COLORS + colour_lookup[ColLU_GREYMD]];
    SCANNER_people_colours[SubTT_PERS_WHITE_BRUN_F] =  SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
    SCANNER_people_colours[SubTT_PERS_WHIT_BLOND_F] = SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
    SCANNER_people_colours[SubTT_PERS_LETH_JACKT_M] = SCANNER_people_colours[SubTT_PERS_BRIEFCASE_M];
}

void SCANNER_init(void)
{
#if 0
    asm volatile ("call ASM_SCANNER_init\n"
        :  :  : "eax" );
#else
    LowTransGrey_InitPaletteBright();
    SCANNER_init_bbpoints();
    LowTransGrey_InitBrightLimitTable();
    SCANNER_init_people_colours();
#endif
}

void SCANNER_set_colour(ubyte col)
{
#if 0
    asm volatile ("call ASM_SCANNER_set_colour\n"
        :  : "a" ((long)col));
#endif
    switch (col)
    {
    case 1:
        SCANNER_colour[0] = 40;
        SCANNER_colour[1] = 68;
        SCANNER_colour[2] = pixmap.fade_table[10 * PALETTE_8b_COLORS + 20];
        SCANNER_colour[4] = 40;
        SCANNER_colour[3] = 32;
        break;
    case 2:
        SCANNER_colour[0] = 20;
        SCANNER_colour[1] = 68;
        SCANNER_colour[2] = pixmap.fade_table[10 * PALETTE_8b_COLORS + 20];
        SCANNER_colour[4] = 20;
        SCANNER_colour[3] = colour_lookup[ColLU_CYAN];
        break;
    default:
        break;
    }
    byte_1DB2E9 = col;
}

void SCANNER_fill_in(void)
{
    asm volatile ("call ASM_SCANNER_fill_in\n"
        :  :  : "eax" );
}

void SCANNER_init_arcpoint(int x1, int z1, int x2, int z2, int c)
{
    asm volatile (
      "push %4\n"
      "call ASM_SCANNER_init_arcpoint\n"
        : : "a" (x1), "d" (z1), "b" (x2), "c" (z2), "g" (c));
}

void SCANNER_unkn_func_196(void)
{
    // TODO when rewriting, use mul_shift16_sign_pad_lo()
    asm volatile ("call ASM_SCANNER_unkn_func_196\n"
        :  :  : "eax" );
}

void SCANNER_data_to_screen(void)
{
    SCANNER_unkn_func_196();
}

void SCANNER_set_screen_box(short x, short y, short width, short height, short cutout)
{
    short i;
    short hlimit;

    hlimit = sizeof(ingame.Scanner.Width)/sizeof(ingame.Scanner.Width[0]);
    if (height >= hlimit)
        height = hlimit - 1;

    ingame.Scanner.X1 = x;
    ingame.Scanner.Y1 = y;
    ingame.Scanner.X2 = ingame.Scanner.X1 + width;
    ingame.Scanner.Y2 = ingame.Scanner.Y1 + height;

    if (cutout != 0)
    {
        for (i = 0; i + ingame.Scanner.Y1 <= ingame.Scanner.Y2; i++) {
            ingame.Scanner.Width[i] = min(width - cutout + i, width);
        }
    }
    else
    {
        for (i = 0; i + ingame.Scanner.Y1 <= ingame.Scanner.Y2; i++) {
            ingame.Scanner.Width[i] = width;
        }
    }
}

void SCANNER_update_arcpoint(ushort arc_no, short fromX, short fromZ, short toX, short toZ)
{
    if (arc_no >= SCANNER_ARC_COUNT)
        return;
    ingame.Scanner.Arc[arc_no].X1 = fromX;
    ingame.Scanner.Arc[arc_no].Z1 = fromZ;
    ingame.Scanner.Arc[arc_no].X2 = toX;
    ingame.Scanner.Arc[arc_no].Z2 = toZ;
}

void SCANNER_init_blippoint(ushort blip_no, int x, int z, int colour)
{
    if (blip_no >= SCANNER_BIG_BLIP_COUNT)
        return;
    ingame.Scanner.BigBlip[blip_no].X = x;
    ingame.Scanner.BigBlip[blip_no].Z = z;
    ingame.Scanner.BigBlip[blip_no].Speed = 4;
    ingame.Scanner.BigBlip[blip_no].Colour = colour;
    ingame.Scanner.BigBlip[blip_no].Period = 32;
}

void SCANNER_find_position(int x, int y, int *U, int *V)
{
    asm volatile (
      "call ASM_SCANNER_find_position\n"
        : : "a" (x), "d" (y), "b" (U), "c" (V));
}

TbBool mouse_move_over_scanner(void)
{
    short dx, dy;
    dx = lbDisplay.MMouseX - ingame.Scanner.X1;
    dy = lbDisplay.MMouseY - ingame.Scanner.Y1;

    return (dy >= 0) && (ingame.Scanner.Y1 + dy <= ingame.Scanner.Y2)
        && (dx >= 0) && (dx <= SCANNER_width[dy]);
}

ushort do_group_scanner(struct Objective *p_objectv, ushort next_signal)
{
    ushort n;
    ubyte colr;
    short dcthing;
    short nearthing;
    ushort group;
    int X, Z;

    group = p_objectv->Thing;
    dcthing = players[local_player_no].DirectControl[0];

    n = next_signal;
    // Find thing in group close to dcthing
    if (objective_target_is_ally(p_objectv)) {
        nearthing = find_nearest_from_group(&things[dcthing],group, 0);
        colr = colour_lookup[ColLU_WHITE];
    } else if (objective_target_is_to_be_acquired(p_objectv)) {
        nearthing = find_nearest_from_group(&things[dcthing], group, 1);
        colr = colour_lookup[ColLU_GREEN];
    } else {
        nearthing = find_nearest_from_group(&things[dcthing], group, 0);
        colr = colour_lookup[ColLU_RED];
    }
    ingame.Scanner.NearThing1 = nearthing;

    if (ingame.Scanner.NearThing1 != 0)
    {
        if (ingame.Scanner.NearThing1 <= 0) {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[ingame.Scanner.NearThing1];
            X = p_sthing->X;
            Z = p_sthing->Z;
        } else {
            struct Thing *p_thing;
            p_thing = &things[ingame.Scanner.NearThing1];
            X = p_thing->X;
            Z = p_thing->Z;
        }
        SCANNER_init_blippoint(n, X, Z, colr);
        n++;
        if (ingame.Scanner.GroupCount < SCANNER_GROUP_COUNT)
        {
            int sgroup;

            sgroup = ingame.Scanner.GroupCount;
            ++ingame.Scanner.GroupCount;
            ingame.Scanner.Group[sgroup] = group;
            ingame.Scanner.GroupCol[sgroup] = colr;
        }
    }
    return n;
}

ushort do_group_near_thing_scanner(struct Objective *p_objectv, ushort next_signal)
{
    ushort n;
    ubyte colr;
    short tgthing;
    short nearthing;
    ushort group;
    long X1, Z1;
    int X2, Z2;

    group = p_objectv->Arg2;
    tgthing = p_objectv->Thing;

    n = next_signal;
    // Find thing in group close to tgthing
    if (objective_target_is_ally(p_objectv)) {
        nearthing = find_nearest_from_group(&things[tgthing], group, 0);
        colr = colour_lookup[ColLU_WHITE];
    } else if (objective_target_is_to_be_acquired(p_objectv)) {
        nearthing = find_nearest_from_group(&things[tgthing], group, 1);
        colr = colour_lookup[ColLU_GREEN];
    } else {
        nearthing = find_nearest_from_group(&things[tgthing], group, 0);
        colr = colour_lookup[ColLU_RED];
    }
    ingame.Scanner.NearThing1 = nearthing;

    // Blip the target thing
    n = next_signal;
    if (tgthing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[tgthing];
        X1 = p_sthing->X;
        Z1 = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[tgthing];
        X1 = p_thing->X;
        Z1 = p_thing->Z;
    }
    SCANNER_init_blippoint(n, X1, Z1, colr);
    n++;

    // Arc the nearest group member
    if (nearthing != 0)
    {
        if (nearthing <= 0) {
            struct SimpleThing *p_sthing;
            p_sthing = &sthings[nearthing];
            X2 = p_sthing->X;
            Z2 = p_sthing->Z;
        } else {
            struct Thing *p_thing;
            p_thing = &things[nearthing];
            X2 = p_thing->X;
            Z2 = p_thing->Z;
        }
        if (dword_1DB1A0)
        {
            SCANNER_update_arcpoint(0, Z2, X2, Z1, X1);
        }
        else
        {
            if (((ingame.TrackThing == 0) || game_cam_tracked_thing_is_player_agent()) && (ingame.Flags & GamF_HUDPanel))
                SCANNER_init_arcpoint(Z2, X2, Z1, X1, 1);
        }
        SCANNER_keep_arcs = 1;

        n++;
        if (ingame.Scanner.GroupCount < SCANNER_GROUP_COUNT)
        {
            int sgroup;

            sgroup = ingame.Scanner.GroupCount;
            ++ingame.Scanner.GroupCount;
            ingame.Scanner.Group[sgroup] = group;
            ingame.Scanner.GroupCol[sgroup] = colr;
        }
    }
    return n;
}

ushort do_target_thing_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X, Z;
    ushort n;
    ubyte colr;
    ThingIdx thing;

    thing = p_objectv->Thing;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        X = p_sthing->X;
        Z = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        X = p_thing->X;
        Z = p_thing->Z;
    }
    SCANNER_init_blippoint(n, X, Z, colr);
    n++;
    return n;
}

ushort do_target_item_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X, Z;
    ushort n;
    ushort weapon;
    ubyte colr;
    ThingIdx thing;

    thing = p_objectv->Thing;
    weapon = p_objectv->Arg2;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    if (thing <= 0)
    {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        if ((p_sthing->Type == SmTT_DROPPED_ITEM) &&
          (p_sthing->U.UWeapon.WeaponType == weapon)) {
            X = p_sthing->X;
            Z = p_sthing->Z;
        } else if (p_sthing->Type == SmTT_CARRIED_ITEM) {
            struct Thing *p_person;
            p_person = &things[p_sthing->U.UWeapon.Owner];
            X = p_person->X;
            Z = p_person->Z;
        } else {
            struct Thing *p_person;
            ThingIdx person;
            person = find_person_carrying_weapon(weapon);
            p_person = &things[person];
            X = p_person->X;
            Z = p_person->Z;
        }
    }
    else
    {
        return n;
    }
    SCANNER_init_blippoint(n, X, Z, colr);
    n++;
    return n;
}

ushort do_thing_arrive_area_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X, Z;
    ushort n;
    ubyte colr;
    ThingIdx thing;

    thing = p_objectv->Thing;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    SCANNER_init_blippoint(n, MAPCOORD_TO_PRCCOORD(p_objectv->X,0), MAPCOORD_TO_PRCCOORD(p_objectv->Z,0), colr);
    n++;
    if (thing <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing];
        X = p_sthing->X;
        Z = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing];
        X = p_thing->X;
        Z = p_thing->Z;
    }
    if (dword_1DB1A0)
    {
        SCANNER_update_arcpoint(0, Z, X,
          MAPCOORD_TO_PRCCOORD(p_objectv->Z,0),
          MAPCOORD_TO_PRCCOORD(p_objectv->X,0));
    }
    else
    {
        if (((ingame.TrackThing == 0) || game_cam_tracked_thing_is_player_agent()) && (ingame.Flags & GamF_HUDPanel))
            SCANNER_init_arcpoint(Z, X,
              MAPCOORD_TO_PRCCOORD(p_objectv->Z,0),
              MAPCOORD_TO_PRCCOORD(p_objectv->X,0), 1);
    }
    SCANNER_keep_arcs = 1;
    return n;
}

ushort do_thing_near_thing_scanner(struct Objective *p_objectv, ushort next_signal)
{
    long X1, Z1;
    long X2, Z2;
    ushort n;
    ubyte colr;
    ThingIdx thing1, thing2;

    thing1 = p_objectv->Thing;
    thing2 = p_objectv->Y;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    if (thing1 <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing1];
        X1 = p_sthing->X;
        Z1 = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing1];
        X1 = p_thing->X;
        Z1 = p_thing->Z;
    }
    SCANNER_init_blippoint(n, X1, Z1, colr);
    n++;
    if (thing2 <= 0) {
        struct SimpleThing *p_sthing;
        p_sthing = &sthings[thing2];
        X2 = p_sthing->X;
        Z2 = p_sthing->Z;
    } else {
        struct Thing *p_thing;
        p_thing = &things[thing2];
        X2 = p_thing->X;
        Z2 = p_thing->Z;
    }
    if (dword_1DB1A0)
    {
        SCANNER_update_arcpoint(0, Z2, X2, Z1, X1);
    }
    else
    {
        if (((ingame.TrackThing == 0) || game_cam_tracked_thing_is_player_agent()) && (ingame.Flags & GamF_HUDPanel))
            SCANNER_init_arcpoint(Z2, X2, Z1, X1, 1);
    }
    SCANNER_keep_arcs = 1;
    return n;
}

ushort do_group_arrive_area_scanner(struct Objective *p_objectv, ushort next_signal)
{
    ushort n;
    ubyte colr;

    if (objective_target_is_ally(p_objectv))
        colr = colour_lookup[ColLU_WHITE];
    else if (objective_target_is_to_be_acquired(p_objectv))
        colr = colour_lookup[ColLU_GREEN];
    else
        colr = colour_lookup[ColLU_RED];

    n = next_signal;
    SCANNER_init_blippoint(n, MAPCOORD_TO_PRCCOORD(p_objectv->X,0),
      MAPCOORD_TO_PRCCOORD(p_objectv->Z,0), colr);
    n++;
    return n;
}

void clear_all_scanner_signals(void)
{
    int n;

    signal_count = 0;
    for (n = 0; n < SCANNER_BIG_BLIP_COUNT; n++)
        ingame.Scanner.BigBlip[n].Period = 0;
    for (n = 0; n < SCANNER_ARC_COUNT; n++)
        ingame.Scanner.Arc[n].Period = 0;
}

void fill_blippoint_scanner(int x, int z, ubyte colour, ushort n)
{
    SCANNER_init_blippoint(n, x, z, colour);
    ingame.Scanner.BigBlip[n].Counter = ingame.Scanner.BigBlip[n].Period;
}

ushort do_netscan_blippoint_scanner(struct NetscanObjective *p_nsobv, ushort next_signal)
{
    int i;
    ushort n;

    n = next_signal;
    for (i = 0; i < NETSCAN_OBJECTIVE_POINTS; i++)
    {
        int x, z;

        if ((p_nsobv->Z[i] == 0) && (p_nsobv->X[i] == 0))
            continue;
        x = p_nsobv->X[i] << 15;
        z = p_nsobv->Z[i] << 15;
        fill_blippoint_scanner(x, z, 87, n);
        n++;
    }
    return n;
}

void add_signal_to_scanner(struct Objective *p_objectv, ubyte flag)
{
    if (flag)
        clear_all_scanner_signals();
    if (gameturn != turn_last)
    {
        turn_last = gameturn;
        ingame.Scanner.GroupCount = 0;
    }
    if ((p_objectv == NULL) || ((p_objectv->Flags & GObjF_HIDDEN) != 0))
        return;

    if (signal_count >= SCANNER_BIG_BLIP_COUNT) {
        LOGWARN("Scaner blips limit reached, blip discarded.");
        return;
    }

    if (objective_target_is_group_to_area(p_objectv))
    {
        signal_count = do_group_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_group_to_vehicle(p_objectv))
    {
        signal_count = do_group_near_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_group_to_thing(p_objectv))
    {
        signal_count = do_group_near_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_group(p_objectv))
    {
        signal_count = do_group_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_person_to_area(p_objectv))
    {
        signal_count = do_thing_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_person_to_thing(p_objectv))
    {
        signal_count = do_thing_near_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_person(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_vehicle_to_area(p_objectv))
    {
        signal_count = do_thing_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_vehicle(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_item_to_area(p_objectv))
    {
        signal_count = do_thing_arrive_area_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_item(p_objectv))
    {
        signal_count = do_target_item_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_object(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
    else if (objective_target_is_any_thing(p_objectv))
    {
        signal_count = do_target_thing_scanner(p_objectv, signal_count);
    }
}

void add_netscan_signal_to_scanner(struct NetscanObjective *p_nsobv, ubyte flag)
{
    if (flag)
        clear_all_scanner_signals();

    if (p_nsobv == NULL)
        return;

    if (signal_count >= SCANNER_BIG_BLIP_COUNT) {
        LOGWARN("Scaner blips limit reached, blip discarded.");
        return;
    }

    // Netscan scanner only supports blippoints
    signal_count = do_netscan_blippoint_scanner(p_nsobv, signal_count);
}

/******************************************************************************/
