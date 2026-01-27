/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file tngobjdrw.c
 *     Draws thing objects using drawlist.
 * @par Purpose:
 *     Implements functions for adding object things to drawlist.
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
#include "tngobjdrw.h"

#include "bfkeybd.h"
#include <assert.h>
#include <stdlib.h>

#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "enginshadws.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintrns.h"
#include "enginzoom.h"

#include "bigmap.h"
#include "building.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "keyboard.h"
#include "matrix.h"
#include "player.h"
#include "swlog.h"
#include "thing.h"
#include "vehicle.h"
/******************************************************************************/
extern ubyte byte_1C83E4;

void do_car_glare(struct Thing *p_car)
{
    asm volatile ("call ASM_do_car_glare\n"
        : : "a" (p_car));
}

void process_child_object(struct Thing *p_vehicle)
{
#if 0
    asm volatile ("call ASM_process_child_object\n"
        : : "a" (p_vehicle));
#endif
    struct SingleObject *p_sobj;
    struct Thing *p_mgun;
    struct M33 *p_matx;
    struct M31 vec1;
    struct M31 vec2;
    struct M31 gear;
    short vhtng_x, vhtng_y, vhtng_z;

    get_thing_position_mapcoords(&vhtng_x, &vhtng_y, &vhtng_z, p_vehicle->ThingOffset);
    gear.R[0] = vhtng_x;
    gear.R[1] = vhtng_y;
    gear.R[2] = vhtng_z;

    p_mgun = &things[p_vehicle->U.UVehicle.SubThing];
    // the mounted gun position is relative, so no need to subtract the base here
    vec2.R[0] = PRCCOORD_TO_MAPCOORD(p_mgun->X);
    vec2.R[1] = p_mgun->Y >> 4;
    vec2.R[2] = PRCCOORD_TO_MAPCOORD(p_mgun->Z);

    assert(p_vehicle->U.UVehicle.MatrixIndex < next_local_mat);
    p_matx = &local_mats[p_vehicle->U.UVehicle.MatrixIndex];
    matrix_transform(&vec1, p_matx, &vec2);

    p_sobj = &game_objects[p_mgun->U.UMGun.Object];
    draw_rot_object(
      gear.R[0] + (vec1.R[0] >> 15) - engn_xc,
      gear.R[1] + (vec1.R[1] >> 15),
      gear.R[2] + (vec1.R[2] >> 15) - engn_zc,
      p_sobj, p_mgun);
}

void build_vehicle(struct Thing *p_thing)
{
    PlayerInfo *p_locplayer;
    int i;

    if (((p_thing->Flag2 & TgF2_ExistsOffMap) != 0) && (byte_1C83E4 & 0x01) != 0)
        return;
    if (p_thing->SubType == SubTT_VEH_SHUTTLE_POD)
        return;

    p_locplayer = &players[local_player_no];

    if (p_locplayer->TargetType < TrgTp_Unkn4)
        check_mouse_overvehicle(p_thing, TrgTp_Unkn4);
    if (p_thing->SubType == SubTT_VEH_MECH)
    {
        if ((p_thing->Flag & TngF_Destroyed) == 0)
            mech_unkn_func_03(p_thing);
        i = 0;
    }
    else
    {
        struct SingleObject *p_sobj;

        p_sobj = &game_objects[p_thing->U.UVehicle.Object];
        i = draw_rot_object(
             PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc,
             PRCCOORD_TO_YCOORD(p_thing->Y),
             PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc,
             p_sobj, p_thing);
    }
    if (p_thing->SubType != SubTT_VEH_TRAIN)
        draw_vehicle_shadow(p_thing->ThingOffset, i);

    if (p_thing->Health < p_thing->U.UVehicle.MaxHealth)
        draw_vehicle_health(p_thing);

    if ((p_thing->U.UVehicle.SubThing != 0) && (p_thing->SubType == SubTT_VEH_TANK))
        process_child_object(p_thing);

    if (p_thing->U.UVehicle.RecoilTimer != 0)
    {
        short tng_x, tng_y, tng_z;
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);
        build_polygon_circle(tng_x, (tng_y >> 3) + 10, tng_z,
          3 * p_thing->U.UVehicle.RecoilTimer + 15, 30, 15,
          game_textures, colour_lookup[ColLU_BLUE], 16,
           16 * ((6 - p_thing->U.UVehicle.RecoilTimer) & 0x0F));
    }

    do_car_glare(p_thing);
}

void build_person(struct Thing *p_thing)
{
    ushort frame, bri;

    if (p_thing->State == PerSt_BEING_PERSUADED)
    {
        frame = p_thing->Frame;
        bri = 32;
    }
    else if (p_thing->U.UPerson.AnimMode == ANIM_PERS_DEAD_BODY)
    {
        ushort stframe_old, stframe_new;

        stframe_old = p_thing->StartFrame + 1 + p_thing->U.UPerson.Angle;
        stframe_new = p_thing->StartFrame + 1 + ((3 * p_thing->U.UPerson.Angle >> 1) + 12 - byte_176D4A) % 12;
        frame = p_thing->Frame + nstart_ani[stframe_new] - nstart_ani[stframe_old];
        bri = p_thing->U.UPerson.Brightness;
    }
    else if ((p_thing->Flag & TngF_Unkn02000000) != 0)
    {
        return;
    }
    else if (p_thing->StartFrame == 1066)
    {
        frame = p_thing->Frame;
        bri = p_thing->U.UPerson.Brightness;
    }
    else
    {
        ushort stframe_old, stframe_new;

        stframe_old = p_thing->StartFrame + 1 + p_thing->U.UPerson.Angle;
        stframe_new = p_thing->StartFrame + 1 + ((p_thing->U.UObject.Angle + 8 - byte_176D49) & 7);
        // Allow increment the frame by the currently set frame, but do not allow decrement
        // Use abs instead of max, to make animation visible even if the difference is negative
        //TODO would be better if a negative frame difference was fixed by states properly updating the Frame
        // The issue workarounded by this is reproducible on stress test level 0,109 - police can
        // look like exploding while still alive; may be related to multiple pushes by explosions
        frame = abs(p_thing->Frame - (int)nstart_ani[stframe_old]) + nstart_ani[stframe_new];
        bri = p_thing->U.UPerson.Brightness;
    }

    draw_pers_e_graphic(p_thing,
      PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_thing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc,
      frame, p_thing->Radius, bri);
}

/** Build rendering drawlist items for a rocket.
 *
 * Unlike most shots, a rocket is an UObject.
 */
void build_rocket(struct Thing *p_thing)
{
    struct SingleObject *p_sobj;
    struct M33 *p_matx;
    struct M31 vec1, vec2, vec3;
    ushort obj;
    short tng_x, tng_y, tng_z;

    get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);
    build_glare(tng_x, tng_y, tng_z, 64);

    // Use transformation matrix for the rocket only temporarly, while adding
    // it to drawlist. Since we will not be using the transform matrix
    // beyond this function, do not mark it as allocated.
    if (next_local_mat + 1 < LOCAL_MATS_COUNT) {
        p_thing->U.UObject.MatrixIndex = next_local_mat + 1;
    } else {
        LOGWARN("Rocket cannot get temp matrix, next_local_mat=%d", (int)next_local_mat);
        // Entry 0 should be unused; but its better than just diappearing the rocket
        p_thing->U.UObject.MatrixIndex = 0;
    }
    p_matx = &local_mats[p_thing->U.UObject.MatrixIndex];
    p_matx->R[0][2] = -64 * p_thing->VX;
    p_matx->R[1][2] = -512 * p_thing->VY;
    p_matx->R[2][2] = -64 * p_thing->VZ;
    p_matx->R[0][1] = 0;
    p_matx->R[1][1] = 0x4000;
    p_matx->R[2][1] = 0;
    vec2.R[0] = p_matx->R[0][2];
    vec2.R[1] = p_matx->R[1][2];
    vec2.R[2] = p_matx->R[2][2];
    vec1.R[0] = p_matx->R[0][1];
    vec1.R[1] = p_matx->R[1][1];
    vec1.R[2] = p_matx->R[2][1];
    vec_cross_prod(&vec3, &vec1, &vec2);
    p_matx->R[0][0] = vec3.R[0] >> 14;
    p_matx->R[1][0] = vec3.R[1] >> 14;
    p_matx->R[2][0] = vec3.R[2] >> 14;
    object_vec_normalisation(p_matx, 0);

    if (ingame.NextRocket >= WEP_ROCKETS_FIRED_LIMIT) {
        LOGWARN("Rockets limit exceeded, next=%d", (int)ingame.NextRocket);
        return;
    }

    obj = ingame.Rocket1[ingame.NextRocket++];
    p_sobj = &game_objects[obj];

    get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);
    draw_rot_object(
      tng_x + (p_thing->VX >> 1) - engn_xc,
      tng_y + 30,
      tng_z + (p_thing->VZ >> 1) - engn_zc,
      p_sobj, p_thing);
}

void build_building(struct Thing *p_thing)
{
    struct SingleObject *p_sobj;

    if (ingame.DisplayMode == DpM_ENGINEPLY)
    {
        if (is_gamekey_pressed(GKey_TRANS_OBJECTS))
            return;
    }

    if (gameturn == p_thing->U.UObject.DrawTurn)
        return;
    p_thing->U.UObject.DrawTurn = gameturn;

    if (p_thing->SubType == SubTT_BLD_BILLBOARD)
    {
        ingame.VisibleBillboardThing = p_thing->ThingOffset;
        ingame.Flags |= GamF_Unkn00040000;
    }

    if (p_thing->SubType == SubTT_BLD_MOVN_ROTOR)
    {
        short tng_x, tng_y, tng_z;
        p_sobj = &game_objects[p_thing->U.UObject.Object];
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);
        draw_rot_object2(tng_x - engn_xc, tng_y, tng_z - engn_zc, p_sobj, p_thing);
    }
    else if (p_thing->SubType == SubTT_BLD_MGUN)
    {
        PlayerInfo *p_locplayer;
        short tng_x, tng_y, tng_z;

        p_locplayer = &players[local_player_no];
        if (p_locplayer->TargetType < TrgTp_Unkn2)
            check_mouse_overvehicle(p_thing, TrgTp_Unkn2);
        p_sobj = &game_objects[p_thing->U.UObject.Object];
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);
        draw_rot_object2(tng_x - engn_xc, tng_y, tng_z - engn_zc, p_sobj, p_thing);
        if (p_thing->Health < p_thing->U.UMGun.MaxHealth)
            draw_vehicle_health(p_thing);
    }
    else
    {
        ushort beg_obj, end_obj;
        ushort obj;

        beg_obj = p_thing->U.UObject.Object;
        end_obj = beg_obj + p_thing->U.UObject.NumbObjects;
        for (obj = beg_obj; obj < end_obj; obj++) {
            p_sobj = &game_objects[obj];
            draw_object(0, 0, 0, p_sobj);
        }
    }
}

void build_laser11(struct Thing *p_thing)
{
    struct Thing *p_owntng;
    short owtng_x, owtng_y, owtng_z;
    TbPixel colour;

    if ((p_thing->Flag & TngF_Unkn1000) != 0)
        colour = colour_lookup[ColLU_BLUE];
    else
        colour = colour_lookup[ColLU_RED];
    get_thing_position_mapcoords(&owtng_x, &owtng_y, &owtng_z, p_thing->Owner);
    p_owntng = &things[p_thing->Owner];

    build_laser(owtng_x, owtng_y >> 3, owtng_z,
      p_thing->VX, p_thing->VY, p_thing->VZ,
      p_thing->Timer1, p_owntng, colour);
}

void build_grenade(struct Thing *p_thing)
{
    struct MyMapElement *p_mapel;
    short tng_x, tng_y, tng_z;
    ushort frame;

    frame = p_thing->Frame;
    p_mapel = &game_my_big_map[128 * (p_thing->Z >> 16) + (p_thing->X >> 16)];
    get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);

    draw_e_graphic(tng_x - engn_xc, tng_y >> 3, tng_z - engn_zc,
      frame, p_thing->Radius, p_mapel->ShadeR, p_thing);
}

void build_static(struct SimpleThing *p_sthing)
{
    struct MyMapElement *p_mapel;
    ushort frame;

    frame = p_sthing->Frame;
    if (frame == nstart_ani[1008])
        return;
    p_mapel = &game_my_big_map[128 * (p_sthing->Z >> 16) + (p_sthing->X >> 16)];

    draw_e_graphic(
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      frame, p_sthing->Radius, p_mapel->ShadeR, (struct Thing *)p_sthing);
}

void build_laser_guided_piece(struct Thing *p_laser)
{
    asm volatile ("call ASM_build_laser_guided_piece\n"
        : : "a" (p_laser));
}

void build_dropped_item(struct SimpleThing *p_sthing)
{
    struct MyMapElement *p_mapel;
    ushort frame;

    frame = p_sthing->Frame;
    p_mapel = &game_my_big_map[128 * (p_sthing->Z >> 16) + (p_sthing->X >> 16)];

    draw_e_graphic(
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      frame, p_sthing->Radius, p_mapel->ShadeR, (struct Thing *)p_sthing);
}

void build_spark(struct SimpleThing *p_sthing)
{
    draw_mapwho_vect_len(
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      p_sthing->U.UEffect.OX - engn_xc,
      p_sthing->U.UEffect.OY,
      p_sthing->U.UEffect.OZ - engn_zc,
      8, p_sthing->Object & 0xFF);
}

void build_unkn18(struct Thing *p_thing)
{
    draw_e_graphic(
      PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_thing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc,
      nstart_ani[900], p_thing->Radius, 63, p_thing);
}

void build_electricity(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner)
{
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_build_electricity\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2), "g" (itime), "g" (p_owner));
}

void build_laser_elec(struct Thing *p_thing)
{
    struct Thing *p_owntng;

    p_owntng = &things[p_thing->Owner];
    if (p_thing->State != 0)
    {
        build_electricity(
          PRCCOORD_TO_MAPCOORD(p_owntng->X),
          PRCCOORD_TO_YCOORD(p_owntng->Y) >> 3,
          PRCCOORD_TO_MAPCOORD(p_owntng->Z),
          p_thing->VX, p_thing->VY, p_thing->VZ,
          100 + p_thing->Timer1, p_owntng);
    }
    else
    {
        int i;

        for (i = 0; i < p_thing->SubType >> 1; i++)
            build_electricity(
              PRCCOORD_TO_MAPCOORD(p_owntng->X),
              PRCCOORD_TO_YCOORD(p_owntng->Y) >> 3,
              PRCCOORD_TO_MAPCOORD(p_owntng->Z),
              p_thing->VX, p_thing->VY, p_thing->VZ,
              p_thing->Timer1, p_owntng);
    }
}

void build_scale_effect(struct SimpleThing *p_sthing)
{
    ushort frame;

    frame = p_sthing->Frame;
    draw_e_graphic_scale(
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      frame, p_sthing->Radius, 32, p_sthing->Object);
}

void build_nuclear_bomb(struct SimpleThing *p_sthing)
{
    if (p_sthing->Radius <= 0 || ((p_sthing->Flag & TngF_InVehicle) == 0))
        return;

    build_polygon_circle(
      PRCCOORD_TO_MAPCOORD(p_sthing->X),
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z),
      p_sthing->Radius, 20, 15,
      game_textures, colour_lookup[ColLU_WHITE], 32, 96);
}

void build_laser_beam(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner)
{
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_build_laser_beam\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2), "g" (itime), "g" (p_owner));
}

void build_laser_beam_q(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner)
{
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_build_laser_beam_q\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2), "g" (itime), "g" (p_owner));
}

void build_laser29(struct Thing *p_thing)
{
    struct Thing *p_owntng;

    p_owntng = &things[p_thing->Owner];
    build_laser_beam(
      PRCCOORD_TO_MAPCOORD(p_owntng->X),
      PRCCOORD_TO_YCOORD(p_owntng->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_owntng->Z),
      p_thing->VX, p_thing->VY, p_thing->VZ,
      p_thing->Timer1, p_owntng);
}

void build_electricity_strand(struct SimpleThing *p_sthing, ubyte itime)
{
    asm volatile (
      "call ASM_build_electricity_strand\n"
        : : "a" (p_sthing), "d" (itime));
}

void build_razor_wire_strand(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner)
{
    asm volatile (
      "push %7\n"
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_build_razor_wire_strand\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "g" (y2), "g" (z2), "g" (itime), "g" (p_owner));
}

void build_soul(struct SimpleThing *p_sthing)
{
    build_glare(PRCCOORD_TO_MAPCOORD(p_sthing->X),
      PRCCOORD_TO_YCOORD(p_sthing->Y),
      PRCCOORD_TO_MAPCOORD(p_sthing->Z), 32);
}

void build_laser38(struct Thing *p_thing)
{
    struct Thing *p_owntng;

    p_owntng = &things[p_thing->Owner];
    build_laser_beam_q(
      PRCCOORD_TO_MAPCOORD(p_owntng->X),
      PRCCOORD_TO_YCOORD(p_owntng->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_owntng->Z),
      p_thing->VX, p_thing->VY, p_thing->VZ,
      p_thing->Timer1, p_owntng);
}

void build_razor_wire(struct Thing *p_thing)
{
    PlayerInfo *p_locplayer;
    struct Thing *p_dcthing;
    ushort mask;

    mask = 0x7F;
    if (p_thing->U.UEffect.LeisurePlace == (gameturn & 0xFFFF))
        return;

    p_locplayer = &players[local_player_no];
    p_dcthing = &things[p_locplayer->DirectControl[0]];

    p_thing->U.UEffect.LeisurePlace = (gameturn & 0xFFFF);
    if (p_thing->SubType != 0)
        mask = 0x1F;
    if ((p_thing->U.UEffect.Group != p_dcthing->U.UPerson.EffectiveGroup)
      && ((gameturn & mask) != 0))
        return;

    build_razor_wire_strand(
      PRCCOORD_TO_MAPCOORD(p_thing->X),
      PRCCOORD_TO_YCOORD(p_thing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_thing->Z),
      p_thing->VX, p_thing->VY, p_thing->VZ,
      p_thing->Timer1, p_thing);
}

void build_time_pod(struct SimpleThing *p_sthing)
{
    asm volatile ("call ASM_build_time_pod\n"
        : : "a" (p_sthing));
}

void build_stasis_pod(struct SimpleThing *p_sthing)
{
    asm volatile ("call ASM_build_stasis_pod\n"
        : : "a" (p_sthing));
}

short draw_thing_object(struct Thing *p_thing)
{
    struct SimpleThing *p_sthing;

    p_sthing = (struct SimpleThing *)p_thing;
    if (p_sthing < &sthings[-1500])
        return -9999;
    if (p_thing > &things[1000])
        return -9999;

    switch (p_thing->Type)
    {
    case TT_VEHICLE:
        build_vehicle(p_thing);
        break;
    case TT_PERSON:
    case TT_UNKN4:
        build_person(p_thing);
        break;
    case SmTT_STATIC:
        build_static(p_sthing);
        break;
    case TT_ROCKET:
        build_rocket(p_thing);
        break;
    case TT_BUILDING:
        build_building(p_thing);
        break;
    case TT_LASER11:
        build_laser11(p_thing);
        break;
    case TT_LASER_GUIDED:
        build_laser_guided_piece(p_thing);
        break;
    case TT_MINE:
    case TT_GRENADE:
        build_grenade(p_thing);
        break;
    case SmTT_DROPPED_ITEM:
        build_dropped_item(p_sthing);
        break;
    case SmTT_SPARK:
        build_spark(p_sthing);
        break;
    case TT_UNKN18:
        build_unkn18(p_thing);
        break;
    case TT_LASER_ELEC:
        build_laser_elec(p_thing);
        break;
    case SmTT_SCALE_EFFECT:
        build_scale_effect(p_sthing);
        break;
    case SmTT_NUCLEAR_BOMB:
        build_nuclear_bomb(p_sthing);
        break;
    case SmTT_ELECTRIC_STRAND:
        build_electricity_strand(p_sthing, 0);
        break;
    case TT_RAZOR_WIRE:
        build_razor_wire(p_thing);
        break;
    case TT_LASER29:
        build_laser29(p_thing);
        break;
    case SmTT_TIME_POD:
        build_time_pod(p_sthing);
        break;
    case SmTT_STASIS_POD:
        build_stasis_pod(p_sthing);
        break;
    case SmTT_SOUL:
        build_soul(p_sthing);
        break;
    case TT_LASER38:
        build_laser38(p_thing);
        break;
    case SmTT_BANG:
        draw_bang(p_sthing);
        break;
    case SmTT_FIRE:
        FIRE_draw_fire(p_sthing);
        break;
    default:
        break;
    }
    return p_thing->Next;
}

/******************************************************************************/
