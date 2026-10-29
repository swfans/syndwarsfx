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
#include "bfgentab.h"
#include "bfmemut.h"
#include "bfpalette.h"
#include "bfutility.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "enginbckt.h"
#include "engincam.h"
#include "engincolour.h"
#include "enginshadws.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "engintrns.h"
#include "engintxtrmap.h"
#include "enginzoom.h"

#include "bigmap.h"
#include "building.h"
#include "display.h"
#include "engindrwlstm.h"
#include "engindrwlstm_wrp.h"
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
#include "tngcolisn.h"
#include "vehicle.h"
/******************************************************************************/
const ushort pers_anims_wth_shadows[] = {
  1, 33, 113, 241, 273, 161, 193, 721, 753, 321, 401, 433, 33,
};

ubyte pers_subtype_to_shpak[] = {
   0,  0,
   0,  1,
   2,  2,
   3,  4,
   9,  9,
  10, 11,
   5,  6,
   0,  0,
   0,  1,
   5,  6,
   0,  1,
   7,  8,
   2,  2,
  10, 11,
   5,  6,
  10, 11,
};

ubyte byte_1C83E4 = 0;

//TODO make some kind of config file for 3D objects, which would include glare locations
struct CarGlare car_glare[] = {
  {   0,   16,  336, 0},
  { -96,  -64,  400, 0},
  { 128,  -64,  400, 0},
  {   0,  -96,  304, 0},
  {-208,  -32, -240, 0},
  { 208,  -32, -240, 0},
  { 128, -144,  240, 0},
  {-112, -144,  240, 0},
  {  96,  -32,  304, 0},
  { -80,  -32,  304, 0},
  {  96,  -96,  528, 0},
  { -80,  -96,  528, 0},
  {-128,  -16,  288, 1},
  { 144,  -16,  288, 2},
  { 112, -112,  -96, 2},
  { -96, -112,  -96, 1},
  { -80, -368,   48, 1},
  {  96, -368,   48, 2},
  { -32,  -64,  160, 0},
  { 112,  -16,  304, 0},
  { -96,  -16,  304, 0},
  { -96, -224,  224, 1},
  {  80, -224,  224, 2},
  { -96, -224, -288, 2},
  {  80, -224, -288, 1},
  {-144,    0,  224, 1},
  { 176,    0,  224, 2},
  {-112,  -32,  448, 0},
  { 144,  -32,  448, 0},
  {-224, -176,  576, 0},
  { 224, -176,  576, 0},
  {   0,   16,  336, 0},
  { -96,  -64,  400, 0},
  { 128,  -64,  400, 0},
  {-160, -672, -688, 0},
  { 160, -672, -688, 0},
  {-112,  -32,  448, 0},
  { 144,  -32,  448, 0},
  {   0,  -96,  304, 0},
  { 112,  -16,  304, 0},
  { -96,  -16,  304, 0},
};

/** For every car model StartFrame (3D object selection), gives a pair of
 * {index of the first glare in `car_glare`, amount of glare dots to draw}.
 */
extern ubyte master_glare[][2];

/** Callback for setting height of shadow corners.
 */
s32 (*get_flat_surface_height_below_point_cb)(struct SortMapPoint *p_cor) = NULL;

/******************************************************************************/

void do_car_glare(struct Thing *p_car)
{
    struct M31 vec_src, vec_dst;
    struct M33 *p_matx;
    ushort glare_beg, glare_num, i;
    short dx, dy, dz;

    if (p_car->StartFrame > 31)
        return;

    glare_beg = master_glare[p_car->StartFrame][0];
    glare_num = master_glare[p_car->StartFrame][1];
    if (glare_num == 0)
        return;

    for (i = glare_beg; i < glare_beg + glare_num; i++)
    {
        struct CarGlare *p_glare;

        p_glare = &car_glare[i];
        vec_src.R[0] = 2 * p_glare->Dix;
        vec_src.R[1] = -2 * p_glare->Diy;
        vec_src.R[2] = -2 * p_glare->Diz;

        p_matx = &local_mats[p_car->U.UVehicle.MatrixIndex];
        matrix_transform(&vec_dst, p_matx, &vec_src);

        dx = PRCCOORD_TO_MAPCOORD(p_car->X) + (vec_dst.R[0] >> 15);
        dy = PRCCOORD_TO_YCOORD(p_car->Y) + (vec_dst.R[1] >> 15);
        dz = PRCCOORD_TO_MAPCOORD(p_car->Z) + (vec_dst.R[2] >> 15);
        build_glare(dx, dy, dz, 48);
    }
}

void process_child_object(struct Thing *p_vehicle)
{
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

/** Prepare buffer with sprite shadows.
 * Clear the wscreen buffer before this call. Also make sure m_sprites are loaded.
 */
void generate_shadows_for_multicolor_sprites(void)
{
    struct ScreenBufBkp bkp;

    // TODO would be better to use some back buffer instead of normal screen buf
    screen_switch_to_custom_buffer(&bkp, lbDisplay.WScreen,
      lbDisplay.GraphicsScreenWidth, 256);
    LbScreenClear(0);

    draw_shadows_for_multicolor_sprites(pers_anims_wth_shadows,
      sizeof(pers_anims_wth_shadows)/sizeof(pers_anims_wth_shadows[0]));

    copy_from_screen_ani(vec_tmap[shadow_tmap_page]);

    generate_shadows_angle_shifts();

    screen_load_backup_buffer(&bkp);
}

void get_object_shadow_bound_points_xz(struct SortMapPoint *p_cor1,
  struct SortMapPoint *p_cor2, struct SortMapPoint *p_cor3,
  struct SortMapPoint *p_cor4, const struct SortMapPoint *p_tngcor,
  const struct ShadowTexture *p_shtextr, short matx)
{
    struct M31 vec_inp;
    struct M31 vec_rot;
    int shd_w, shd_l;

    shd_w = p_shtextr->Width;
    shd_l = p_shtextr->Length;

    vec_inp.R[0] = -shd_w;
    vec_inp.R[2] = -shd_l;
    vec_inp.R[1] = 0;
    assert(matx < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[matx], &vec_inp);
    p_cor1->X = p_tngcor->X - engn_xc + (vec_rot.R[0] >> 15);
    p_cor1->Z = p_tngcor->Z - engn_zc + (vec_rot.R[2] >> 15);

    vec_inp.R[1] = 0;
    vec_inp.R[0] = shd_w;
    vec_inp.R[2] = -shd_l;
    assert(matx < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[matx], &vec_inp);
    p_cor2->X = p_tngcor->X - engn_xc + (vec_rot.R[0] >> 15);
    p_cor2->Z = p_tngcor->Z - engn_zc + (vec_rot.R[2] >> 15);

    vec_inp.R[0] = p_shtextr->Width;
    vec_inp.R[1] = 0;
    vec_inp.R[2] = shd_l;
    assert(matx < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[matx], &vec_inp);
    p_cor3->X = p_tngcor->X - engn_xc + (vec_rot.R[0] >> 15);
    p_cor3->Z = p_tngcor->Z - engn_zc + (vec_rot.R[2] >> 15);

    vec_inp.R[0] = -p_shtextr->Width;
    vec_inp.R[1] = 0;
    vec_inp.R[2] = shd_l;
    assert(matx < next_local_mat);
    matrix_transform(&vec_rot, &local_mats[matx], &vec_inp);
    p_cor4->X = p_tngcor->X - engn_xc + (vec_rot.R[0] >> 15);
    p_cor4->Z = p_tngcor->Z - engn_zc + (vec_rot.R[2] >> 15);
}

void get_object_shadow_bound_points_y(struct SortMapPoint *p_cor1,
  struct SortMapPoint *p_cor2, struct SortMapPoint *p_cor3,
  struct SortMapPoint *p_cor4, struct SortMapPoint *p_tngcor)
{
    p_cor1->Y = p_cor2->Y = p_cor3->Y = p_cor4->Y = p_tngcor->Y;
    if (get_flat_surface_height_below_point_cb == NULL) {
        return;
    }
    p_cor1->Y = get_flat_surface_height_below_point_cb(p_cor1);
    p_cor2->Y = get_flat_surface_height_below_point_cb(p_cor2);
    p_cor3->Y = get_flat_surface_height_below_point_cb(p_cor3);
    p_cor4->Y = get_flat_surface_height_below_point_cb(p_cor4);
}

void draw_object_model_shadow(struct SortMapPoint *p_tngcor, ushort obmodl,
  short matx, int bckt)
{
    struct SortMapPoint cor1, cor2, cor3, cor4;
    struct ShadowTexture *p_shtextr;

    p_shtextr = &shadowtexture[obmodl];
    if (p_shtextr->Width == 0)
        return;

    get_object_shadow_bound_points_xz(&cor1, &cor2, &cor3, &cor4,
      p_tngcor, p_shtextr, matx);

    get_object_shadow_bound_points_y(&cor1, &cor2, &cor3, &cor4,
      p_tngcor);

    draw_shadow_at_coords(&cor1, &cor2, &cor3, &cor4, p_shtextr, bckt);
}

s32 get_flat_surface_height_at_ground_callback(struct SortMapPoint *p_cor)
{
    return PRCCOORD_TO_YCOORD(alt_at_point(
      engn_xc + p_cor->X, engn_zc + p_cor->Z));
}

s32 get_flat_surface_height_below_real_pos_callback(struct SortMapPoint *p_cor)
{
    return PRCCOORD_TO_MAPCOORD(alt_at_point_under_height(
      engn_xc + p_cor->X, engn_zc + p_cor->Z,
      MAPCOORD_TO_PRCCOORD(p_cor->Y,0)));
}


void draw_vehicle_shadow(ushort veh, int bckt)
{
    struct SortMapPoint tngcor;
    struct Thing *p_vehicle;
    short matx;
    ushort obmodl;

    p_vehicle = &things[veh];

    matx = p_vehicle->U.UVehicle.MatrixIndex;
    obmodl = p_vehicle->StartFrame;
    tngcor.X = PRCCOORD_TO_MAPCOORD(p_vehicle->X);
    tngcor.Y = PRCCOORD_TO_MAPCOORD(p_vehicle->Y);
    tngcor.Z = PRCCOORD_TO_MAPCOORD(p_vehicle->Z);
    if (p_vehicle->SubType == SubTT_VEH_GROUND)
        get_flat_surface_height_below_point_cb = get_flat_surface_height_below_real_pos_callback;
    else
        get_flat_surface_height_below_point_cb = get_flat_surface_height_at_ground_callback;

    draw_object_model_shadow(&tngcor, obmodl, matx, bckt);
}

void build_vehicle(struct Thing *p_thing)
{
    PlayerInfo *p_locplayer;
    int bckt;

    if (((p_thing->Flag2 & TgF2_ExistsOffMap) != 0) && (byte_1C83E4 & 0x01) != 0)
        return;
    if (p_thing->SubType == SubTT_VEH_SHUTTLE_POD)
        return;

    p_locplayer = &players[local_player_no];

    if (p_locplayer->TargetType < TrgTp_Unkn4)
        check_mouse_overvehicle(p_thing, TrgTp_Unkn4);
    if (p_thing->SubType == SubTT_VEH_MECH)
    {
        if ((p_thing->Flag & TngF_Destroyed) == 0) {
            mech_unkn_func_03(p_thing);
        }
        bckt = 10; //TODO get proper bucket from object draw calls
    }
    else
    {
        struct SingleObject *p_sobj;

        p_sobj = &game_objects[p_thing->U.UVehicle.Object];
        bckt = draw_rot_object(
             PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc,
             PRCCOORD_TO_YCOORD(p_thing->Y),
             PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc,
             p_sobj, p_thing);
    }
    if (p_thing->SubType != SubTT_VEH_TRAIN)
        draw_vehicle_shadow(p_thing->ThingOffset, bckt);

    if (p_thing->Health < p_thing->U.UVehicle.MaxHealth)
        draw_vehicle_health(p_thing, bckt);

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
    ushort frame, brig;
    int cor_dx, cor_dy, cor_dz;
    char locstr[12];

    if (p_thing->State == PerSt_BEING_PERSUADED)
    {
        frame = p_thing->Frame;
        brig = PALETTE_FADE_LEVELS / 2;
    }
    else if (p_thing->U.UPerson.AnimMode == ANIM_PERS_DEAD_BODY)
    {
        ushort sbfrm;
        ushort angl;

        sbfrm = get_person_anim_subframe(p_thing);
        angl = ((3 * p_thing->U.UPerson.Angle >> 1) + 12 - byte_176D4A) % 12;
        frame = nstart_ani[p_thing->StartFrame + 1 + angl] + sbfrm;
        brig = p_thing->U.UPerson.Brightness;
    }
    else if ((p_thing->Flag & TngF_Unkn02000000) != 0)
    {
        return;
    }
    else if (p_thing->StartFrame == 1066)
    {
        frame = p_thing->Frame;
        brig = p_thing->U.UPerson.Brightness;
    }
    else
    {
        ushort sbfrm;
        ushort angl;

        sbfrm = get_person_anim_subframe(p_thing);
        angl = (p_thing->U.UObject.Angle + 8 - byte_176D49) & 7;
        frame = nstart_ani[p_thing->StartFrame + 1 + angl] + sbfrm;
        brig = p_thing->U.UPerson.Brightness;
    }

    cor_dx = PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc;
    cor_dy = PRCCOORD_TO_YCOORD(p_thing->Y) >> 3;
    cor_dz = PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc;

    draw_pers_e_graphic(p_thing, cor_dx, cor_dy, cor_dz,
      frame, p_thing->Radius, brig);

    if (debug_hud_collision) {
        enlist_draw_number(cor_dx, cor_dy, cor_dz,
          0, 37, p_thing->U.UPerson.RecoilTimer,
          p_thing->Radius - 1, colour_lookup[ColLU_RED]);
    }

    locstr[0] = '\0';

    if ((p_thing->Flag2 & TgF2_ExistsOffMap) != 0) {
        strcat(locstr, "E");
    }
    if ((ingame.DisplayMode != 50) && ((p_thing->Flag2 & TgF2_InsideBuilding) != 0)) {
        strcat(locstr, "B");
    }
    if (locstr[0] != '\0') {
        enlist_draw_text(cor_dx, cor_dy, cor_dz, -8, 37, locstr,
          p_thing->Radius - 1, colour_lookup[ColLU_RED]);
    }
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

    if (drawturn == p_thing->U.UObject.DrawTurn)
        return;
    p_thing->U.UObject.DrawTurn = drawturn;

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
        int bckt;

        p_locplayer = &players[local_player_no];
        if (p_locplayer->TargetType < TrgTp_Unkn2)
            check_mouse_overvehicle(p_thing, TrgTp_Unkn2);
        p_sobj = &game_objects[p_thing->U.UObject.Object];
        get_thing_position_mapcoords(&tng_x, &tng_y, &tng_z, p_thing->ThingOffset);
        bckt = draw_rot_object2(tng_x - engn_xc, tng_y, tng_z - engn_zc, p_sobj, p_thing);
        if (p_thing->Health < p_thing->U.UMGun.MaxHealth)
            draw_vehicle_health(p_thing, bckt);
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

    if ((p_thing->Flag & TngF_SelectedAgent) != 0)
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

    draw_thing_e_graphic(p_thing, tng_x - engn_xc, tng_y >> 3, tng_z - engn_zc,
      frame, p_thing->Radius, p_mapel->ShadeR);
}

void build_static(struct SimpleThing *p_sthing)
{
    struct MyMapElement *p_mapel;
    ushort frame;

    frame = p_sthing->Frame;
    if (frame == nstart_ani[1008])
        return;
    p_mapel = &game_my_big_map[128 * (p_sthing->Z >> 16) + (p_sthing->X >> 16)];

    draw_thing_e_graphic((struct Thing *)p_sthing,
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      frame, p_sthing->Radius, p_mapel->ShadeR);
}

/** Computes an angular turn rate to steer a laser beam.
 *
 *  The turn rate is to guide the beam segment from cur_angle
 *  towards target_angle, in fixed (stepped) increments.
 *  Also outputs the current glow size, depending on how
 *  far off the current angle is.
 */
short calc_angle_rate(short target_angle, short cur_angle, short *p_glow_size)
{
    short delta;
    short absdelta;

    delta = target_angle - cur_angle;
    if (delta < -LbFPMath_PI)
        delta += 2 * LbFPMath_PI;
    else if (delta > LbFPMath_PI)
        delta -= 2 * LbFPMath_PI;

    absdelta = (delta < 0) ? -delta : delta;
    *p_glow_size = (absdelta > 256) ? 0x30 : (((LbFPMath_PI - absdelta) >> 2) + 0x10);

    if (delta < -LbFPMath_PI/2)
        return -100;
    else if (delta < -100)
        return -70;
    else if (delta < 0)
        return -10;
    else if (delta == 0)
        return 0;
    else if (delta <= 100)
        return 10;
    else if (delta <= LbFPMath_PI/2)
        return 70;
    return 100;
}

/** Builds a single slice (quad) of a 3D beam betweeen two points.
 *
 *  The slice is connecting a near point to a far point. When use_cache
 *  is set, the near point is not re-computed - the screen position
 *  left over from the previous  call (the earlier far point) is reused.
 */
struct SingleObjectFace4 *build_continuos_slice_3d(int near_x, int near_y, int near_z,
  int far_x, int far_y, int far_z, int w1, int w2, ubyte use_cache, ubyte col)
{
#if 0
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[6];

    stkargs[0] = (int)(intptr_t)far_y;
    stkargs[1] = (int)(intptr_t)far_z;
    stkargs[2] = (int)(intptr_t)w1;
    stkargs[3] = (int)(intptr_t)w2;
    stkargs[4] = (int)(intptr_t)use_cache;
    stkargs[5] = (int)(intptr_t)col;

    asm volatile (
      "push 20(%4)\n"
      "push 16(%4)\n"
      "push 12(%4)\n"
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_build_continuos_slice_3d\n"
        : : "a" (near_x), "d" (near_y), "b" (near_z), "c" (far_x), "S" (stkargs)
        : "cc", "memory");
#endif
    static short near_scr_x = 0;
    static short near_scr_y = 0;
    struct EnginePoint ep;
    struct SingleObjectFace4 *p_face4;
    short x1, y1, x2, y2;
    int sort_key;

    if (!use_cache) {
        LbMemorySet(&ep, 0, sizeof(struct EnginePoint));
        ep.X3d = near_x - engn_xc;
        ep.Y3d = (near_y << 3); // subtracted within transform_point(): - 8 * engn_yc;
        ep.Z3d = near_z - engn_zc;
        transform_point(&ep);
        near_scr_x = ep.pp.X;
        near_scr_y = ep.pp.Y;
    }

    LbMemorySet(&ep, 0, sizeof(struct EnginePoint));
    ep.X3d = far_x - engn_xc;
    ep.Y3d = (far_y << 3); // subtracted within transform_point(): - 8 * engn_yc;
    ep.Z3d = far_z - engn_zc;
    transform_point(&ep);

    sort_key = ep.Z3d + 0x1288;
    if (sort_key < 0)
        sort_key = 0;
    else if (sort_key > 9999)
        sort_key = 9999;

    x2 = ep.pp.X;
    y2 = ep.pp.Y;
    x1 = near_scr_x;
    y1 = near_scr_y;

    p_face4 = build_polygon_slice(x1, y1, x2, y2,
      w1, w2, col, sort_key, use_cache);

    near_scr_x = x2;
    near_scr_y = y2;
    return p_face4;
}

void build_laser_guided_piece(struct Thing *p_laser)
{
#if 0
    asm volatile ("call ASM_build_laser_guided_piece\n"
        : : "a" (p_laser));
    return;
#endif
    struct Thing *p_target;
    int cur_x, cur_y, cur_z;
    int target_x, target_z;
    short full_angle;
    short cur_angle;
    short glow_size;
    short rate;
    int count;
    ushort use_cache;
    ubyte base_colour;

    cur_x = PRCCOORD_TO_MAPCOORD(p_laser->X);
    cur_y = PRCCOORD_TO_MAPCOORD(p_laser->Y);
    cur_z = PRCCOORD_TO_MAPCOORD(p_laser->Z);

    p_target = p_laser->PTarget;
    if (p_target == NULL) {
        return;
    }

    target_x = PRCCOORD_TO_MAPCOORD(p_target->X);
    target_z = PRCCOORD_TO_MAPCOORD(p_target->Z);

    full_angle = angle_between_points(cur_x, cur_z, target_x, target_z);
    cur_angle = p_laser->U.UEffect.AngleY;
    calc_angle_rate(full_angle, cur_angle, &glow_size);

    base_colour = colour_lookup[2];
    use_cache = 0;

    for (count = 5; count > 0; count--)
    {
        struct SingleObjectFace4 *p_face4;
        int new_x, new_z;
        ubyte brig;
        TbPixel colour;

        full_angle = angle_between_points(cur_x, cur_z, target_x, target_z);
        rate = calc_angle_rate(full_angle, cur_angle, &glow_size);

        new_x = cur_x + ((lbSinTable[cur_angle] * glow_size) >> 17);
        new_z = cur_z + ((lbSinTable[cur_angle + LbFPMath_PI/2] * glow_size) >> 17);

        brig = 31 + (count << 3);
        colour = pixmap.fade_table[(brig << 8) + base_colour];

        p_face4 = build_continuos_slice_3d(cur_x, cur_y, cur_z,
          new_x, cur_y, new_z, 2, 2, use_cache, colour);
        if (p_face4 == NULL)
            break;

        if (count == 1) {
            p_face4->Flags = 0;
            p_face4->ExCol = colour_lookup[ColLU_WHITE];
        } else {
            p_face4->Flags = 0x08 | 0x02;
            p_face4->ExCol = 0x35 - (count * 4);
        }

        cur_angle = (cur_angle + rate) & LbFPMath_AngleMask;
        use_cache = 1;
        cur_x = new_x;
        cur_z = new_z;
    }
}

void build_dropped_item(struct SimpleThing *p_sthing)
{
    struct MyMapElement *p_mapel;
    ushort frame;

    frame = p_sthing->Frame;
    p_mapel = &game_my_big_map[128 * (p_sthing->Z >> 16) + (p_sthing->X >> 16)];

    draw_thing_e_graphic((struct Thing *)p_sthing,
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      frame, p_sthing->Radius, p_mapel->ShadeR);
}

void build_spark(struct SimpleThing *p_sthing)
{
    enlist_draw_mapwho_vect_len(
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
    draw_thing_e_graphic(p_thing,
      PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_thing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc,
      nstart_ani[900], p_thing->Radius, 63);
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
    enlist_draw_frame_graphic_scale(
      PRCCOORD_TO_MAPCOORD(p_sthing->X) - engn_xc,
      PRCCOORD_TO_YCOORD(p_sthing->Y) >> 3,
      PRCCOORD_TO_MAPCOORD(p_sthing->Z) - engn_zc,
      frame, p_sthing->Radius, 32, p_sthing->Object,
      (intptr_t)p_sthing);
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

/** Draws a single jittered "lightning bolt" line effect between two 3D points.
 *
 *  The line is split into 17 steps; at each step (other than the end points)
 *  a sine-based perpendicular jitter (independently re-randomized per axis)
 *  is added to the otherwise straight interpolated point, and a line segment
 *  is drawn from the previous jittered point to the current one.
 */
void do_beam_electric(int x1, int y1, int z1,
  int x2, int y2, int z2)//TODO add, ubyte colkp)
{
#if 1
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[2];

    stkargs[0] = (int)(intptr_t)y2;
    stkargs[1] = (int)(intptr_t)z2;

    asm volatile (
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_do_beam_electric\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "S" (stkargs)
        : "cc", "memory");
    return;
#endif
    int dx, dy, dz;
    int nx, ny, nz;
    int step_dx, step_dy, step_dz;
    int prc_x, prc_y, prc_z;
    int length;
    int step;
    int jit_x, jit_y, jit_z;
    int beg_cor_x, beg_cor_y, beg_cor_z;
    TbPixel colour;

    //TODO make coords relative later, only for transformation
    x1 -= engn_xc;
    y1 -= (engn_yc >> 3);
    z1 -= engn_zc;
    x2 -= engn_xc;
    y2 -= (engn_yc >> 3);
    z2 -= engn_zc;

    dx = x2 - x1;
    dy = y2 - y1;
    dz = z2 - z1;

    length = LbSqrL(dx * dx + dy * dy + dz * dz);
    if (length == 0)
        return;

    nx = (dx << 8) / length;
    ny = (dy << 8) / length;
    nz = -((dz << 8) / length);

    step_dx = dx << 4;
    step_dy = dy << 4;
    step_dz = dz << 4;

    if ((LbRandomPosShort() & 1) != 0) {
        nx = -nx;
        nz = -nz;
    }

    prc_x = (x1 << 8) + (step_dx << 1);
    prc_y = (y1 << 8) + (step_dy << 1);
    prc_z = (z1 << 8) + (step_dz << 1);

    colour = colour_lookup[ColLU_WHITE];

    jit_x = jit_y = jit_z = 0;
    beg_cor_x = beg_cor_y = beg_cor_z = 0;

    for (step = 0; step <= LbFPMath_PI; step += LbFPMath_PI/16)
    {
        int new_jit_x, new_jit_y, new_jit_z;

        new_jit_x = (lbSinTable[step] * nz) >> 7;
        new_jit_y = (lbSinTable[step] * ny) >> 7;
        new_jit_z = (lbSinTable[step] * nx) >> 7;

        if (step != LbFPMath_PI) {
            new_jit_x = (new_jit_x * ((LbRandomPosShort() & 0xFF) + 0x20)) >> 8;
            new_jit_y = (new_jit_y * ((LbRandomPosShort() & 0xFF) + 0x20)) >> 8;
            new_jit_z = (new_jit_z * ((LbRandomPosShort() & 0xFF) + 0x20)) >> 8;
        }

        if (step != 0) {
            int end_cor_x, end_cor_y, end_cor_z;

            end_cor_x = (prc_x + new_jit_x) >> 8;
            end_cor_y = (prc_y + new_jit_y) >> 8;
            end_cor_z = (prc_z + new_jit_z) >> 8;
            enlist_draw_mapwho_vect(beg_cor_x, beg_cor_y, beg_cor_z,
              end_cor_x, end_cor_y, end_cor_z, colour);
        }

        beg_cor_x = (prc_x + jit_x) >> 8;
        beg_cor_y = (prc_y + jit_y) >> 8;
        beg_cor_z = (prc_z + jit_z) >> 8;
        prc_x += step_dx;
        prc_y += step_dy;
        prc_z += step_dz;

        jit_x = new_jit_x;
        jit_y = new_jit_y;
        jit_z = new_jit_z;
    }
}

/** Draws one pass of a segmented laser beam line.
 *
 *  Draws beam chunk from a starting screen position, moving forward by
 *  the given per-segment step, for num_segments segments.
 *
 *  Screen coords are fixed point, with 8 fractional bits.
 */
static void build_laser_beam_segments(int start_x, int start_y, int start_depth,
  int step_x, int step_y, int step_depth, int num_segments, short width,
  int itime, TbPixel beam_colour)
{
    int cur_x, cur_y, cur_depth;
    ubyte use_cache;
    int i;

    cur_x = start_x;
    cur_y = start_y;
    cur_depth = start_depth;
    use_cache = 0;

    for (i = 0; i < num_segments; i++)
    {
        int next_x, next_y, next_depth;

        next_x = cur_x + step_x;
        next_y = cur_y + step_y;
        next_depth = cur_depth + step_depth;

        if ((cur_x > 0) && ((cur_x >> 8) < lbDisplay.GraphicsScreenWidth)
         && (cur_y > 0) && ((cur_y >> 8) < lbDisplay.GraphicsScreenHeight))
        {
            struct SingleObjectFace4 *p_face4;
            int sort_key;
            ushort brig;
            TbPixel fade_colour;

            sort_key = (cur_depth >> 8) + 0x1107;
            if (sort_key < 0)
                sort_key = 0;
            else if (sort_key > 9999)
                sort_key = 9999;

            brig = 32 + 2 * itime;
            fade_colour = pixmap.fade_table[256 * brig + colour_lookup[ColLU_BLUE]];

            p_face4 = build_polygon_slice(cur_x >> 8, cur_y >> 8,
              next_x >> 8, next_y >> 8, width, width, fade_colour, sort_key, use_cache);
            if (p_face4 != NULL) {
                p_face4->Flags = 0x0f;
                p_face4->ExCol = beam_colour;
            }
        }

        cur_x = next_x;
        cur_y = next_y;
        cur_depth = next_depth;
        use_cache = 1;
    }
}

static void build_laser_beam_glare(int scr_x, int scr_y,
  TbPixel beam_colour, int circle_depth)
{
    int circle_sort_key;

    circle_sort_key = circle_depth;
    if (circle_sort_key < 0)
        circle_sort_key = 0;
    else if (circle_sort_key > 9999)
        circle_sort_key = 9999;

    build_polygon_circle_2d(scr_x >> 8, scr_y >> 8, 13, 13,
      0xf, game_textures, beam_colour, 32, 127, circle_sort_key);
    build_polygon_circle_2d(scr_x >> 8, scr_y >> 8, 10, 10,
      0xf, game_textures, beam_colour, 32, 127, circle_sort_key);
}

/** Draws a glowing laser beam line between two 3D points, including a muzzle flash.
 *
 *  The flash is drawn at the weapon-specific muzzle offset of p_owner.
 *
 * @param beam_colour Colour used for the muzzle flash circles and for the
 *   ExCol of the drawn beam segments (colour_lookup[] index).
 */
static void build_laser_beam_generic(int x1, int y1, int z1, int x2, int y2, int z2,
  int itime, struct Thing *p_owner, int num_electric_strands, TbPixel beam_colour,
  int circle_depth_bias)
{
    struct EnginePoint ep_near, ep_far;
    int circle_x, circle_y;
    int dx, dy, dz;
    int screen_dist;
    int num_segments;
    int spread_x, spread_y;
    int step_x, step_y, step_z;
    short width1, width2, width3;
    int i;
    short ofs_x, ofs_y;

    for (i = 0; i < num_electric_strands; i++) {
        do_beam_electric(x1, y1 + 20, z1, x2, y2, z2);
    }

    ep_near.Flags = 0;
    ep_near.X3d = x1 - engn_xc;
    ep_near.Y3d = (y1 << 3); // subtracted within transform_point(): - 8 * engn_yc;
    ep_near.Z3d = z1 - engn_zc;
    transform_point(&ep_near);

    get_bullet_start_offsets(&ofs_x, &ofs_y, p_owner);

    ep_far.Flags = 0;
    ep_far.X3d = x2 - engn_xc;
    ep_far.Y3d = (y2 << 3); // subtracted within transform_point(): - 8 * engn_yc;
    ep_far.Z3d = z2 - engn_zc;
    transform_point(&ep_far);

    if (((ep_near.Flags & ep_far.Flags) & 0x0F) != 0)
        return;

    circle_x = (ep_near.pp.X << 8) + ((ofs_x * overall_scale) >> 1);
    circle_y = (ep_near.pp.Y << 8) + ((ofs_y * overall_scale) >> 1);

    dx = (ep_far.pp.X << 8) - circle_x;
    dy = (ep_far.pp.Y << 8) - circle_y;
    dz = (ep_far.Z3d << 8) - (ep_near.Z3d << 8);

    screen_dist = LbSqrL(dx * dx + dy * dy);
    if (screen_dist == 0)
        return;

    num_segments = (screen_dist / 20) >> 8;
    if (num_segments < 1)
        num_segments = 1;

    circle_x += ((dx * 150) / screen_dist) << 2;
    circle_y += ((dy * 150) / screen_dist) << 2;

    build_laser_beam_glare(circle_x, circle_y, beam_colour, ep_near.Z3d + circle_depth_bias);

    circle_x += ((dx * 150) / screen_dist) << 4;
    circle_y += ((dy * 150) / screen_dist) << 4;

    if (itime <= 2) {
        spread_x = 0;
        spread_y = 0;
    } else {
        int perp_x, perp_y, perp_len;

        perp_x = dy >> 3;
        perp_y = -(dx >> 3);
        perp_len = LbSqrL((perp_x * perp_x + perp_y * perp_y) << 4);
        if (perp_len == 0)
            perp_len = 1;
        spread_x = ((perp_x << 10) / perp_len) << 2;
        spread_y = ((perp_y << 10) / perp_len) << 2;
    }

    step_x = dx / num_segments;
    step_y = dy / num_segments;
    step_z = dz / num_segments;

    width1 = (itime > 2) ? 8 : itime;
    width2 = (itime > 2) ? 6 : itime;
    width3 = (itime > 2) ? 2 : itime;

    build_laser_beam_segments(circle_x, circle_y, ep_near.Z3d << 8,
      step_x, step_y, step_z, num_segments, width1, itime, beam_colour);

    build_laser_beam_segments(circle_x + spread_x, circle_y + spread_y, ep_near.Z3d << 8,
      step_x, step_y, step_z, num_segments, width2, itime, beam_colour);

    build_laser_beam_segments(circle_x - spread_x, circle_y - spread_y, ep_near.Z3d << 8,
      step_x, step_y, step_z, num_segments, width3, itime, beam_colour);
}

void build_laser_beam(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner)
{
#if 1
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[4];

    stkargs[0] = (int)(intptr_t)y2;
    stkargs[1] = (int)(intptr_t)z2;
    stkargs[2] = (int)(intptr_t)itime;
    stkargs[3] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 12(%4)\n"
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_build_laser_beam\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "S" (stkargs)
        : "cc", "memory");
    return;
#endif
    build_laser_beam_generic(x1, y1, z1, x2, y2, z2, itime, p_owner,
      2, colour_lookup[ColLU_WHITE], 0x1107);
}

void build_laser_beam_q(int x1, int y1, int z1, int x2, int y2, int z2, int itime, struct Thing *p_owner)
{
#if 1
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[4];

    stkargs[0] = (int)(intptr_t)y2;
    stkargs[1] = (int)(intptr_t)z2;
    stkargs[2] = (int)(intptr_t)itime;
    stkargs[3] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 12(%4)\n"
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_build_laser_beam_q\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "S" (stkargs)
        : "cc", "memory");
    return;
#endif
    build_laser_beam_generic(x1, y1, z1, x2, y2, z2, itime, p_owner,
      4, colour_lookup[ColLU_BLUE], 0x1288);
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
    // Pushed through a register holding them: a "g" operand may be placed
    // relative to the stack pointer, which each push moves.
    int stkargs[4];

    stkargs[0] = (int)(intptr_t)y2;
    stkargs[1] = (int)(intptr_t)z2;
    stkargs[2] = (int)(intptr_t)itime;
    stkargs[3] = (int)(intptr_t)p_owner;

    asm volatile (
      "push 12(%4)\n"
      "push 8(%4)\n"
      "push 4(%4)\n"
      "push 0(%4)\n"
      "call ASM_build_razor_wire_strand\n"
        : : "a" (x1), "d" (y1), "b" (z1), "c" (x2), "S" (stkargs)
        : "cc", "memory");
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
