/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file lvfiles.c
 *     Routines for level and map files handling.
 * @par Purpose:
 *     Implement functions for handling and checking level and map files.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lvfiles.h"

#include <assert.h>
#include <string.h>
#include "bffile.h"
#include "bfmath.h"
#include "bfmemut.h"

#include "drawtext.h"
#include "engincolour.h"
#include "enginfexpl.h"
#include "enginlights.h"

#include "bat.h"
#include "triangls.h"
#include "trpoints.h"
#include "trstate.h"
#include "pathtrig.h"
#include "bigmap.h"
#include "campaign.h"
#include "command.h"
#include "display.h"
#include "enginpriobjs.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginsngobjs.h"
#include "engintxtrmap.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_data.h"
#include "game_options.h"
#include "game_speed.h"
#include "lvobjctv.h"
#include "lvwalk.h"
#include "matrix.h"
#include "building.h"
#include "pepgroup.h"
#include "thing.h"
#include "tngcolisn.h"
#include "vehicle.h"
#include "vehtraffic.h"
#include "weapon.h"
#include "swlog.h"
/******************************************************************************/
#pragma pack(1)

struct BillboardNBreakout {
  ubyte field_0;
  ubyte field_1;
  ubyte field_2;
  ubyte field_3;
};

#pragma pack()
/******************************************************************************/

TbBool level_deep_fix = false;

ulong stored_g3d_next_object;
ulong stored_g3d_next_object_face;
ulong stored_g3d_next_object_face4;
ulong stored_g3d_next_object_point;
ulong stored_g3d_next_normal;
ulong stored_g3d_next_face_texture;
ulong stored_g3d_next_floor_texture;
ulong stored_g3d_next_local_mat;
ulong stored_global3d_inuse;

extern struct QuickLoad quick_load_pc[19];

struct QuickLoad quick_load_pc[] = {
  {NULL,				(void **)&game_my_big_map,	18, 16384},
  {&next_floor_texture,	(void **)&game_textures,		18, 800},
  {&next_face_texture,	(void **)&game_face_textures,	16, 800},
  {&next_object_point,	(void **)&game_object_points,	10, 2000},
  {&next_object_face,	(void **)&game_object_faces,	32, 2000},
  {&next_object,		(void **)&game_objects,		36, 120},
  {&next_quick_light,	(void **)&game_quick_lights,	 6, 4000},
  {&next_full_light,	(void **)&game_full_lights,	32, 10},
  {&next_normal,		(void **)&game_normals,		16, 3500},
  {&next_object_face4,	(void **)&game_object_faces4, 40, 2000},
  {&next_anim_tmap,		(void **)&game_anim_tmaps,	54, 50},
  {&next_traffic_node,	(void **)&game_traffic_nodes, 36, 50},
  {&next_light_command,	(void **)&game_light_commands,36, 50},
  {&next_vects_list,	(void **)&game_col_vects_list, 6, 1000},
  {&next_col_vect,		(void **)&game_col_vects,		14, 600},
  {&next_walk_header,	(void **)&game_walk_headers,	 4, 0},
  {&next_walk_item,		(void **)&game_walk_items,	 2, 0},
  {&next_col_column,	(void **)&game_col_columns,	16, 0},
  {NULL,				NULL,				 0, 0},
};

ushort next_used_lvl_objective = 1;

extern uint dword_177790;
extern struct BillboardNBreakout map_bnb;

void debug_level(const char *text, int player)
{
    ThingIdx thing;

    thing = things_used_head;
    while (thing != 0)
    {
        struct Thing *p_thing;

        p_thing = &things[thing];
        // TODO place debug/verification code
        thing = p_thing->LinkChild;
    }
}

void global_3d_store(int action)
{
    if (action == 2)
    {
        if (stored_global3d_inuse)
            draw_text(100, 120, " GLOBAL 3d STORED ->INUSE", colour_lookup[ColLU_RED]);
    }
    else if (action == 1)
    {
        if (stored_global3d_inuse)
        {
            next_object = stored_g3d_next_object;
            next_object_face = stored_g3d_next_object_face;
            next_object_face4 = stored_g3d_next_object_face4;
            next_object_point = stored_g3d_next_object_point;
            next_normal = stored_g3d_next_normal;
            next_face_texture = stored_g3d_next_face_texture;
            next_floor_texture = stored_g3d_next_floor_texture;
            next_local_mat = stored_g3d_next_local_mat;
            stored_global3d_inuse = 0;
        }
    } else
    {
        if (!stored_global3d_inuse)
        {
            stored_g3d_next_object = next_object;
            stored_g3d_next_object_face = next_object_face;
            stored_g3d_next_object_face4 = next_object_face4;
            stored_g3d_next_object_point = next_object_point;
            stored_g3d_next_normal = next_normal;
            stored_g3d_next_face_texture = next_face_texture;
            stored_g3d_next_floor_texture = next_floor_texture;
            stored_g3d_next_local_mat = next_local_mat;
            stored_global3d_inuse = 1;
        }
    }
}

TbBool is_level_stored_thing(struct Thing *p_thing)
{
    switch (p_thing->Type)
    {
    case TT_VEHICLE:
    case TT_PERSON:
    case TT_UNKN35:
       return true;
    }
    return false;
}

TbBool is_level_stored_sthing(struct SimpleThing *p_sthing)
{
    switch (p_sthing->Type)
    {
    case SmTT_CARRIED_ITEM:
    case SmTT_DROPPED_ITEM:
       return true;
    }
    return false;
}

ulong load_level_pc_handle(TbFileHandle lev_fh)
{
    u32 fmtver;
    TbBool mech_initialized;
    int limit;
    int i, k, n;

    mech_initialized = 0;
    fmtver = 0;
    LbFileRead(lev_fh, &fmtver, sizeof(u32));

    if (fmtver >= 1)
    {
        ushort count;
        short new_thing;
        struct Thing loc_thing;
        struct Thing *p_thing;

        count = 0;
        LbFileRead(lev_fh, &count, 2);

        LOGSYNC("Level fmtver=%lu n_things=%hd", fmtver, count);
        for (i = 0; i < count; i++)
        {
            short angle;

            new_thing = get_new_thing();
            if (new_thing <= 0) {
                LOGERR("No thing slots for level content");
            }
            p_thing = &things[new_thing];
            memcpy(&loc_thing, p_thing, sizeof(struct Thing));
            if (fmtver >= 13) {
                assert(sizeof(struct Thing) == 168);
                LbFileRead(lev_fh, p_thing, sizeof(struct Thing));
            } else {
                struct ThingOldV9 s_oldthing;
                assert(sizeof(s_oldthing) == 216); // the sizeof(Thing) was 216 since fmtver=2
                LbFileRead(lev_fh, &s_oldthing, sizeof(s_oldthing));
                refresh_old_thing_format(p_thing, &s_oldthing, fmtver);
            }

            if (!is_level_stored_thing(p_thing))
            {
                LOGWARN("Thing(%hd,%hd) group %hd at (%d,%d,%d) type=%d,%d",
                  p_thing->ThingOffset, p_thing->U.UPerson.UniqueID,
                  p_thing->U.UPerson.Group,
                  PRCCOORD_TO_MAPCOORD(p_thing->X),
                  PRCCOORD_TO_MAPCOORD(p_thing->Y),
                  PRCCOORD_TO_MAPCOORD(p_thing->Z),
                  (int)p_thing->Type, (int)p_thing->SubType);
            }
            else
            {
                LOGNO("Thing(%hd,%hd) group %hd at (%d,%d,%d) type=%d,%d",
                  p_thing->ThingOffset, p_thing->U.UPerson.UniqueID,
                  p_thing->U.UPerson.Group,
                  PRCCOORD_TO_MAPCOORD(p_thing->X),
                  PRCCOORD_TO_MAPCOORD(p_thing->Y),
                  PRCCOORD_TO_MAPCOORD(p_thing->Z),
                  (int)p_thing->Type, (int)p_thing->SubType);
            }

            if ((p_thing->Z >> 8) < 256)
                p_thing->Z += (256 << 8);
            if ((p_thing->X >> 16) >= 128)
                p_thing->X = (64 << 16);
            p_thing->PTarget = 0;
            p_thing->LinkParent = loc_thing.LinkParent;
            p_thing->LinkChild = loc_thing.LinkChild;
            // We have limited amount of group definitions
            if (p_thing->U.UObject.Group >= PEOPLE_GROUPS_COUNT)
                p_thing->U.UObject.Group = 0;
            // All relevant thing types must have the values below at same position
            p_thing->U.UObject.EffectiveGroup = p_thing->U.UObject.Group;

            if (new_thing == 0)
                continue;

            add_node_thing(new_thing);

            if (p_thing->Type == TT_PERSON)
            {
                if (fmtver < 15)
                    // Causes invisible NPCs when non-zero
                    p_thing->Flag2 = 0;
                p_thing->U.UPerson.Flag3 = 0;
                p_thing->Flag2 &= (TgF2_InsideBuilding|TgF2_ExistsOffMap);
                if ((p_thing->Flag & TngF_Unkn02000000) != 0)
                {
                    p_thing->ThingOffset = p_thing - things;
                    remove_thing(new_thing);
                    delete_node(p_thing);
                    continue;
                }
                reset_person_frame(p_thing);
                init_person_thing(p_thing);
                p_thing->Flag |= TngF_Unkn0004;
                if (fmtver < 6)
                    p_thing->Flag |= TngF_Unkn04000000;
                if (fmtver < 12)
                    sanitize_cybmods_fmtver11_flags(&p_thing->U.UPerson.UMod);
            }

            if (p_thing->Type == SmTT_DROPPED_ITEM)
            {
                // SimpleThings should not be on this list. But many level do have them.
                p_thing->Frame = nstart_ani[p_thing->StartFrame + 1];
            }

            if (p_thing->Type == TT_VEHICLE)
            {
                if (fmtver < 17)
                    p_thing->U.UVehicle.Armour = 4;
                p_thing->U.UVehicle.PassengerHead = 0;
                p_thing->Flag2 &= TgF2_ExistsOffMap;
                if (fmtver <= 8)
                    p_thing->Y >>= 3;

                {
                    int matx;
                    assert(sizeof(struct M33) == 36);
                    assert(next_local_mat < LOCAL_MATS_COUNT);
                    matx = next_local_mat++;
                    LbFileRead(lev_fh, &local_mats[matx], sizeof(struct M33));
                    p_thing->U.UVehicle.MatrixIndex = matx;
                }
                byte_1C83D1 = 0;

                n = next_normal;
                copy_prim_obj_to_game_object(PRCCOORD_TO_MAPCOORD(p_thing->X), PRCCOORD_TO_MAPCOORD(p_thing->Z),
                  -prim_unknprop01 - p_thing->StartFrame, PRCCOORD_TO_MAPCOORD(p_thing->Y));
                k = next_normal;
                unkn_object_shift_03(next_object - 1);
                unkn_object_shift_02(n, k, next_object - 1);

                k = p_thing - things;
                p_thing->U.UVehicle.Object = next_object - 1;
                game_objects[next_object - 1].ThingNo = k;
                VNAV_unkn_func_207(p_thing);
                {
                    struct M33 *p_matx;
                    int matx;

                    matx = p_thing->U.UVehicle.MatrixIndex;
                    assert(matx < next_local_mat);
                    p_matx = &local_mats[matx];
                    angle = LbArcTanAngle(p_matx->R[0][2], p_matx->R[2][2]);
                    p_thing->U.UVehicle.AngleY = (angle + LbFPMath_PI) & LbFPMath_AngleMask;
                }

                veh_add(p_thing, p_thing->StartFrame);

                if (p_thing->SubType == SubTT_VEH_TANK)
                {
                    if (p_thing->U.UVehicle.SubThing != 0)
                    {
                        struct Thing *p_mgun;
                        struct M33 *p_matx;

                        // Mounted Gun matrix was created during veh_add()
                        p_mgun = &things[p_thing->U.UVehicle.SubThing];
                        assert(p_thing->U.UVehicle.MatrixIndex < next_local_mat);
                        p_matx = &local_mats[p_mgun->U.UVehicle.MatrixIndex];
                        angle = LbArcTanAngle(p_matx->R[0][2], p_matx->R[2][2]);
                        p_mgun->U.UVehicle.AngleY = (angle + LbFPMath_PI) & LbFPMath_AngleMask;
                    }
                }
                else if (p_thing->SubType == SubTT_VEH_MECH)
                {
                    if (!mech_initialized)
                    {
                        init_mech();
                        mech_unkn_func_02();
                        mech_initialized = 1;
                    }
                    mech_unkn_func_09(new_thing);
                }
                debug_level(" placed vehicle", 1);
            }
        }
    }
    LbFileRead(lev_fh, &next_command, sizeof(ushort));
    limit = get_memory_ptr_allocated_count((void **)&game_commands);
    if ((limit >= 0) && (next_command > limit)) {
        LOGERR("Restricting \"%s\", wanted %d, limit %ld", "game_commands", (int)next_command, limit);
        next_command = limit;
    }
    assert(sizeof(struct Command) == 32);
    LbFileRead(lev_fh, game_commands, sizeof(struct Command) * next_command);

    if (fmtver >= 2)
    {
        LbFileRead(lev_fh, &level_def, 44);
    }
    for (i = 0; i < 8; i++)
    {
        if (level_def.PlayableGroups[i] >= PEOPLE_GROUPS_COUNT)
            level_def.PlayableGroups[i] = 0;
    }
    if (fmtver >= 3)
    {
        LbFileRead(lev_fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 1320 - 33, 1320);
        LbFileRead(lev_fh, war_flags, 32 * sizeof(struct WarFlag));
    }
    for (k = 0; k < PEOPLE_GROUPS_COUNT; k++)
    {
        for (i = 0; i < 8; i++)
        {
            if (war_flags[k].Guardians[i] >= 32)
                war_flags[k].Guardians[i] = 0;
        }
    }
    if (fmtver >= 3)
    {
        LbFileRead(lev_fh, &word_1531E0, sizeof(ushort));
        LbFileRead(lev_fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 32000, 15 * word_1531E0);
        LbFileRead(lev_fh, &unkn3de_len, sizeof(ushort));
        LbFileRead(lev_fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 32000, unkn3de_len);
    }
    LOGSYNC("Level fmtver=%lu n_command=%hu word_1531E0=%hu unkn3de_len=%hu",
      (ulong)fmtver, next_command, word_1531E0, unkn3de_len);
    if (fmtver >= 4)
    {
        ulong count;
        ThingIdx thing;

        count = 0;
        LbFileRead(lev_fh, &count, 2);
        for (i = count; i > 0; i--)
        {
            struct SimpleThing loc_thing;
            struct SimpleThing *p_thing;

            thing = get_new_sthing();
            p_thing = &sthings[thing];
            memcpy(&loc_thing, p_thing, 60);
            LbFileRead(lev_fh, p_thing, 60);

            if (!is_level_stored_sthing(p_thing))
            {
                LOGWARN("SThing(%hd,%hd) at (%d,%d,%d) type=%d,%d",
                  p_thing->ThingOffset, p_thing->UniqueID,
                  PRCCOORD_TO_MAPCOORD(p_thing->X),
                  PRCCOORD_TO_MAPCOORD(p_thing->Y),
                  PRCCOORD_TO_MAPCOORD(p_thing->Z),
                  (int)p_thing->Type, (int)p_thing->SubType);
            }
            else
            {
                LOGNO("SThing(%hd,%hd) at (%d,%d,%d) type=%d,%d",
                  p_thing->ThingOffset, p_thing->UniqueID,
                  PRCCOORD_TO_MAPCOORD(p_thing->X),
                  PRCCOORD_TO_MAPCOORD(p_thing->Y),
                  PRCCOORD_TO_MAPCOORD(p_thing->Z),
                  (int)p_thing->Type, (int)p_thing->SubType);
            }

            if (fmtver <= 17)
            {
                if (p_thing->Type == 0)
                {
                    p_thing->Type = SmTT_DROPPED_ITEM;
                    p_thing->SubType &= 3;
                }
            }

            if (p_thing->Type == SmTT_CARRIED_ITEM)
            {
                p_thing->LinkParent = loc_thing.LinkParent;
                p_thing->LinkChild = loc_thing.LinkChild;
                remove_sthing(thing);
            }
            else
            {
              if (p_thing->Type == SmTT_DROPPED_ITEM) {
                  p_thing->Frame = nstart_ani[p_thing->StartFrame + 1];
              }
              p_thing->LinkParent = loc_thing.LinkParent;
              p_thing->LinkChild = loc_thing.LinkChild;
              if (thing != 0)
                  add_node_sthing(thing);
            }
        }
    }

    if (fmtver >= 6)
    {
        LbFileRead(lev_fh, &game_level_unique_id, 2);
        if (game_level_unique_id < 1000)
            game_level_unique_id = 1000;
        if (game_level_unique_id > 9000)
            game_level_unique_id = 4000;
    }

    if (fmtver >= 7) {
        LbFileRead(lev_fh, &next_used_lvl_objective, sizeof(ushort));
        assert(sizeof(struct Objective) == 32);
        n = LbFileRead(lev_fh, game_used_lvl_objectives, sizeof(struct Objective) * next_used_lvl_objective);
        if (n < (int)sizeof(struct Objective) * next_used_lvl_objective)
            LOGWARN("Array used_lvl_objectives truncated, got %d bytes", n);
    } else {
        next_used_lvl_objective = 1;
    }

    if (fmtver >= 9) {
        LbFileRead(lev_fh, game_level_unkn1, 40);
        LbFileRead(lev_fh, game_level_unkn2, 40);
    }

    if (fmtver >= 10) {
        // An older file format had this struct with sizeof=20,
        // but we don't know the speciifc fmtver range for that
        assert(sizeof(struct LevelMisc) == 22);
        n = LbFileRead(lev_fh, game_level_miscs, sizeof(struct LevelMisc) * 200);
        if (n < (int)sizeof(struct LevelMisc) * 200)
            LOGWARN("Array level_miscs truncated, got %d bytes", n);
    }

    if (fmtver >= 16) {
        n = LbFileRead(lev_fh, &engn_anglexz, 4);
        if (n < 4)
            LOGWARN("Field anglexz truncated, got %d bytes", n);
    }

    return fmtver;
}

void save_level_pc_handle(TbFileHandle lev_fh)
{
    u32 fmtver;
    ushort count;
    int i;

    assert(sizeof(struct Thing) == 168);
    assert(sizeof(struct Command) == 32);
    assert(sizeof(struct Objective) == 32);
    assert(sizeof(struct LevelMisc) == 22);

    fmtver = 18;
    LbFileWrite(lev_fh, &fmtver, sizeof(u32));

    {// Things are partially stored in map, partially in level file
        struct Thing *p_thing;
        ThingIdx thing;

        i = 0;
        for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
        {
            p_thing = &things[thing];
            // Per thing code start
            if (!is_level_stored_thing(p_thing))
                continue;
            i++;
            // Per thing code end
        }
        count = i;
        LbFileWrite(lev_fh, &count, 2);

        i = 0;
        LOGSYNC("Level fmtver=%lu n_things=%hd", (ulong)fmtver, count);
        for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
        {
            p_thing = &things[thing];
            // Per thing code start
            if (!is_level_stored_thing(p_thing))
                continue;
            i++;
            LbFileWrite(lev_fh, p_thing, sizeof(struct Thing));
            if (p_thing->Type == TT_VEHICLE)
            {
                int matx;
                assert(sizeof(struct M33) == 36);
                matx = p_thing->U.UVehicle.MatrixIndex;
                assert(matx < next_local_mat);
                LbFileWrite(lev_fh, &local_mats[matx], sizeof(struct M33));

            }
            // Per thing code end
        }
    }
    assert(count == i);

    LbFileWrite(lev_fh, &next_command, sizeof(ushort));
    LbFileWrite(lev_fh, game_commands, sizeof(struct Command) * next_command);

    LbFileWrite(lev_fh, &level_def, 44);

    LbFileWrite(lev_fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 1320 - 33, 1320);
    LbFileWrite(lev_fh, war_flags, 32 * sizeof(struct WarFlag));

    LbFileWrite(lev_fh, &word_1531E0, sizeof(ushort));
    LbFileWrite(lev_fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 32000, 15 * word_1531E0);
    LbFileWrite(lev_fh, &unkn3de_len, sizeof(ushort));
    LbFileWrite(lev_fh, engine_mem_alloc_ptr + engine_mem_alloc_size - 32000, unkn3de_len);

    {// SimpleThings are partially stored in map, partially in level file
        struct SimpleThing *p_sthing;
        ThingIdx thing;

        i = 0;
        for (thing = sthings_used_head; thing < 0; thing = p_sthing->LinkChild)
        {
            p_sthing = &sthings[thing];
            // Per thing code start
            if (!is_level_stored_sthing(p_sthing))
                continue;
            i++;
            // Per thing code end
        }
        count = i;
        LbFileWrite(lev_fh, &count, 2);

        i = 0;
        LOGSYNC("Level fmtver=%lu n_sthings=%hd", (ulong)fmtver, count);
        for (thing = sthings_used_head; thing < 0; thing = p_sthing->LinkChild)
        {
            p_sthing = &sthings[thing];
            // Per thing code start
            if (!is_level_stored_sthing(p_sthing))
                continue;
            i++;
            LbFileWrite(lev_fh, p_sthing, sizeof(struct SimpleThing));
            // Per thing code end
        }
    }
    assert(count == i);

    LbFileWrite(lev_fh, &game_level_unique_id, 2);

    LbFileWrite(lev_fh, &next_used_lvl_objective, sizeof(ushort));
    LbFileWrite(lev_fh, game_used_lvl_objectives, sizeof(struct Objective) * next_used_lvl_objective);

    LbFileWrite(lev_fh, game_level_unkn1, 40);
    LbFileWrite(lev_fh, game_level_unkn2, 40);

    LbFileWrite(lev_fh, game_level_miscs, sizeof(struct LevelMisc) * 200);

    LbFileWrite(lev_fh, &engn_anglexz, 4);
}


short find_group_which_looks_like_human_player(TbBool strict)
{
    short group;
    short partial_group;
    short n_partial;

    n_partial = 0;
    for (group = 0; group < PEOPLE_GROUPS_COUNT; group++)
    {
        int n_all, n_agents, n_zealots, n_punks;

        n_all = count_people_in_group(group, -1);
        if (n_all < 1)
            continue;

        if (strict && (n_all > 4))
            continue;

        n_agents = count_people_in_group(group, SubTT_PERS_AGENT);
        if (n_agents == 4)
            return group;
        n_zealots = count_people_in_group(group, SubTT_PERS_ZEALOT);
        if (n_zealots == 4)
            return group;
        n_punks = count_people_in_group(group, SubTT_PERS_PUNK_F) +
          count_people_in_group(group, SubTT_PERS_PUNK_M);
        if (n_punks == 4)
            return group;

        if ((n_agents > 0) && (!strict || (n_agents <= 4)))
        {
            if (n_partial < n_agents) {
                partial_group = group;
                n_partial = n_agents;
            }
        }
        if ((n_zealots > 0) && (!strict || (n_zealots <= 4)))
        {
            if (n_partial < n_zealots) {
                partial_group = group;
                n_partial = n_zealots;
            }
        }
        if ((n_punks > 0) && (!strict || (n_punks <= 4)))
        {
            if (n_partial < n_punks) {
                partial_group = group;
                n_partial = n_punks;
            }
        }
    }

    if (n_partial > 0)
        return partial_group;
    return -1;
}

ushort person_add_command(ThingIdx person, ubyte cmdtype)
{
    struct Thing *p_person;
    struct Command *p_cmd;
    ushort cmd;

    if (person <= 0)
        return 0;

    p_person = &things[person];

    cmd = get_new_command();
    p_cmd = &game_commands[cmd];

    p_cmd->Next = p_person->U.UPerson.ComHead;
    p_person->U.UPerson.ComHead = cmd;
    p_cmd->Type = cmdtype;

    return cmd;
}

void add_commands_from_person_states(void)
{
    struct Thing *p_thing;
    ThingIdx thing;

    for (thing = things_used_head; thing > 0; thing = p_thing->LinkChild)
    {
        p_thing = &things[thing];
        // Per thing code start
        if (p_thing->Type == TT_PERSON && p_thing->U.UPerson.ComHead == 0)
        {
            ushort cmd;
            struct Command *p_cmd;

            switch (p_thing->Type)
            {
            case PerSt_WANDER:
                cmd = person_add_command(thing, PCmd_WAND_TIME);
                p_cmd = &game_commands[cmd];
                p_cmd->Time = 9999;
                break;
            default:
                break;
            }
        }
        // Per thing code end
    }
}

void level_perform_deep_fix(void)
{
    {
        short pv_group, nx_group;
        TbBool need_fix;
        int i;

        pv_group = level_def.PlayableGroups[0];
        i = count_people_in_group(pv_group, -1);
        need_fix = (i < 1) || (i > 4);
        if (need_fix) {
            nx_group = find_group_which_looks_like_human_player(true);
            if (nx_group >= 0) {
                LOGWARN("Local player group %d has no team; switching to better group %d",
                  (int)pv_group, (int)nx_group);
                level_def.PlayableGroups[0] = nx_group;
                need_fix = false;
            }
        }
        if (need_fix) {
            short lp_group;
            int n;

            nx_group = find_group_which_looks_like_human_player(false);
            lp_group = -1;
            if (nx_group >= 0)
                lp_group = find_unused_group_id(true);
            if (lp_group >= 0) {
                thing_group_copy(pv_group, nx_group, 0x01|0x02|0x04);
                n = thing_group_transfer_people(nx_group, lp_group, SubTT_PERS_AGENT, 0, 4);
                if (n <= 0)
                    n = thing_group_transfer_people(nx_group, lp_group, SubTT_PERS_ZEALOT, 0, 4);
                if (n <= 0)
                    n = thing_group_transfer_people(nx_group, lp_group, -1, 0, 4);
                if (n > 0) {
                    LOGWARN("Local player group %d has no team; switching to new group %d based on %d",
                      (int)pv_group, (int)lp_group, (int)nx_group);
                    level_def.PlayableGroups[0] = lp_group;
                    need_fix = false;
                }
            }
        }
        if (need_fix) {
            LOGWARN("Local player group %d has no team; fix attempts failed",
              (int)pv_group);
        }
    }

#if 0
    // While the group 0 sometimes looks suspicious, it generally works correctly
    // We should transfer other things away from that group, instead. So, this got disabled.
    // Also, switching the group would require fixing its index in all objectives and commands.
    if (level_def.PlayableGroups[0] == 0) {
        short pv_group, nx_group;

        pv_group = level_def.PlayableGroups[0];
        nx_group = find_unused_group_id(true);
        LOGWARN("Local player group is %d; switching to %d",
          (int)pv_group, (int)nx_group);
        if (nx_group > 0) {
            thing_group_copy(pv_group, nx_group, 0x01|0x02);
            thing_group_transfer_people(pv_group, nx_group, -1, 0, 4);
        }
    }
#endif
}

void fix_level_indexes(short missi, u32 fmtver, ubyte reload, TbBool deep)
{
    ushort objectv;
    ThingIdx thing;

    LOGSYNC("Fixing mission %d fmtver %lu reload %d deep %d",
      (int)missi, (ulong)fmtver, (int)reload, (int)deep);
    fix_thing_commands_indexes(deep);

    for (objectv = 1; objectv < next_used_lvl_objective; objectv++)
    {
        struct Objective *p_objectv;

        p_objectv = &game_used_lvl_objectives[objectv];
        p_objectv->Level = LEVEL_NUM_STRAIN(current_level);
        p_objectv->Map = current_map;
        fix_single_objective(p_objectv, objectv, "UL");
    }

    for (objectv = 1; objectv < next_objective; objectv++)
    {
        struct Objective *p_objectv;

        p_objectv = &game_objectives[objectv];
        fix_single_objective(p_objectv, objectv, "S");
    }

    // Used objectives are not part of the level file, but part of
    // the mission file. If restarting level, leave these intact,
    // as fixups were already applied on first load. Though loading
    // a different level as part of the same mission needs to be
    // taken into account when setting the 'reload' parameter.
    if (!reload)
    {
        fix_mission_used_objectives(missi);
    }

    for (thing = 1; thing < THINGS_LIMIT; thing++)
    {
        things[thing].ThingOffset = thing;
    }

    for (thing = 1; thing < STHINGS_LIMIT; thing++)
    {
        sthings[-thing].ThingOffset = -thing;
    }
}

int map_things_unkn_func_04(short subtype)
{
    ubyte ret;
    asm volatile ("call ASM_map_things_unkn_func_04\n"
        : "=r" (ret) : "a" (subtype));
    return ret;
}

TbResult level_misc_update_mgun(struct LevelMisc *p_lvmsc)
{
    struct Thing *p_mgun;
    s32 bkp_engn_xc, bkp_engn_yc, bkp_engn_zc;
    ThingIdx mgun;

    bkp_engn_xc = engn_xc;
    bkp_engn_yc = engn_yc;
    bkp_engn_zc = engn_zc;

    engn_xc = p_lvmsc->X;
    engn_zc = p_lvmsc->Z;
    engn_yc = 0;
    mgun = map_things_unkn_func_04(SubTT_BLD_MGUN);
    if (mgun == 0) {
        return Lb_FAIL;
    }

    p_mgun = &things[mgun];
    p_mgun->U.UMGun.Group = p_lvmsc->Group;
    p_mgun->U.UMGun.RecoilTimer = 0;
    p_mgun->U.UMGun.CurrentWeapon = p_lvmsc->Weapon;
    LOGSYNC("Mounted gun at (%d,%d) set to %s(%d)", (int)p_lvmsc->X, (int)p_lvmsc->Z,
      weapon_codename(p_lvmsc->Weapon), (int)p_lvmsc->Weapon);

    engn_xc = bkp_engn_xc;
    engn_yc = bkp_engn_yc;
    engn_zc = bkp_engn_zc;

    return Lb_SUCCESS;
}

TbResult level_misc_verify_mgun(struct LevelMisc *p_lvmsc)
{
    s32 bkp_engn_xc, bkp_engn_yc, bkp_engn_zc;
    ThingIdx mgun;

    if ((p_lvmsc->Group < 0) || (p_lvmsc->Group >= PEOPLE_GROUPS_COUNT))
        return Lb_FAIL;
    if ((p_lvmsc->Weapon < 1) || (p_lvmsc->Weapon >= WEP_TYPES_COUNT))
        return Lb_FAIL;

    bkp_engn_xc = engn_xc;
    bkp_engn_yc = engn_yc;
    bkp_engn_zc = engn_zc;

    engn_xc = p_lvmsc->X;
    engn_zc = p_lvmsc->Z;
    engn_yc = 0;
    mgun = map_things_unkn_func_04(SubTT_BLD_MGUN);
    engn_xc = bkp_engn_xc;
    engn_yc = bkp_engn_yc;
    engn_zc = bkp_engn_zc;

    if (mgun == 0)
        return Lb_FAIL;

    return Lb_OK;
}

/** Removes invalid entries from level_miscs[].
 */
void level_misc_validate(void)
{
    int i, n, limit, last_used;

    limit = get_memory_ptr_allocated_count((void **)&game_level_miscs);
    if (limit < 0) {
        LOGERR("No memory for \"%s\", limit %d", "level_miscs", limit);
        return;
    }

    // Get last used slot
    last_used = 0;
    for (i = 0; i < limit; i++)
    {
        struct LevelMisc *p_lvmsc;
        p_lvmsc = &game_level_miscs[i];
        if (p_lvmsc->Type != LvMiscT_NONE)
            last_used = i;
    }

    for (i = 0; i <= last_used; i++)
    {
        struct LevelMisc *p_lvmsc;
        TbResult ret;

        p_lvmsc = &game_level_miscs[i];
        switch (p_lvmsc->Type)
        {
        case LvMiscT_NONE: // empty entry
            ret = Lb_FAIL;
            break;
        case LvMiscT_MGUN:
            ret = level_misc_verify_mgun(p_lvmsc);
            break;
        default:
            ret = Lb_FAIL;
            break;
        }
        if (ret == Lb_FAIL) {
            LOGERR("Invalid \"%s\" entry %d type %d, removed",
              "level_miscs", i, (int)p_lvmsc->Type);
            for (n = i + 1; n <= last_used; n++)
                LbMemoryCopy(&game_level_miscs[n - 1], &game_level_miscs[n], sizeof(struct LevelMisc));
            LbMemorySet(&game_level_miscs[last_used], '\0', sizeof(struct LevelMisc));
            last_used--;
            i--;
        }
    }
}

void level_misc_update(u32 fmtver)
{
#if 0
    asm volatile ("call ASM_level_misc_update\n"
        :  :  : "eax" );
#endif
    int limit, i;

    limit = get_memory_ptr_allocated_count((void **)&game_level_miscs);
    if (limit < 0) {
        LOGERR("No memory for \"%s\", limit %d", "level_miscs", limit);
        return;
    }

    for (i = 0; i < limit; i++)
    {
        struct LevelMisc *p_lvmsc;
        TbResult ret;

        p_lvmsc = &game_level_miscs[i];
        switch (p_lvmsc->Type)
        {
        case LvMiscT_NONE: // empty entry
            ret = Lb_OK;
            break;
        case LvMiscT_MGUN:
            ret = level_misc_update_mgun(p_lvmsc);
            break;
        default:
            ret = Lb_FAIL;
            break;
        }
        if (ret == Lb_FAIL) {
            LOGERR("Invalid \"%s\" entry %d type %d",
              "level_miscs", i, (int)p_lvmsc->Type);
        }
    }
}

TbBool level_misc_get_starting_camera_pos(MapCoord *cor_x, MapCoord *cor_z)
{
    int i, limit, last_match;

    limit = get_memory_ptr_allocated_count((void **)&game_level_miscs);

    // Get last used slot
    last_match = -1;
    for (i = 0; i < limit; i++)
    {
        struct LevelMisc *p_lvmsc;
        p_lvmsc = &game_level_miscs[i];
        if (p_lvmsc->Type == LvMiscT_MGUN)
            last_match = i;
    }

    if (last_match >= 0)
    {
        struct LevelMisc *p_lvmsc;
        p_lvmsc = &game_level_miscs[last_match];
        *cor_x = p_lvmsc->X;
        *cor_z = p_lvmsc->Z;
        return true;
    }
    return false;
}

void load_level_pc(short level, short missi, ubyte reload)
{
    short next_level, prev_level;
    TbFileHandle lev_fh;
    char lev_fname[52];

    next_level = level;
    gameturn = 0;

    prev_level = current_level;
    if (next_level < 0)
    {
        next_level = -next_level;
        word_1C8446 = 1;
        if (next_level <= 15)
            sprintf(lev_fname, "%s/c%03dl%03d.dat", game_dirs[DirPlace_Levels].directory,
                current_map, next_level);
        else
            sprintf(lev_fname, "%s/c%03dl%03d.d%d", game_dirs[DirPlace_Levels].directory,
               current_map, LEVEL_NUM_STRAIN(next_level), LEVEL_NUM_VARIANT(next_level));
        if (next_level > 0)
            current_level = next_level;
    }
    if (next_level <= 0) {
        LOGERR("Next level index is not positive, load skipped");
        return;
    }
    LOGSYNC("Next level %hd prev level %hd mission %hd reload 0x%x",
      next_level, prev_level, missi, (uint)reload);

    /* In campaign mode, this flag is always set; so is this editor feature?
     */
    if ((ingame.Flags & GamF_Unkn0008) == 0)
    {
        if (prev_level)
            global_3d_store(1);
        global_3d_store(0);
    }
    debug_level(" load level restart coms", 1);

    lev_fh = LbFileOpen(lev_fname, Lb_FILE_MODE_READ_ONLY);
    if (lev_fh != INVALID_FILE)
    {
        u32 fmtver;
        int i;

        word_1C8446 = 1;
        word_176E38 = 0;

        fmtver = load_level_pc_handle(lev_fh);

        LbFileClose(lev_fh);

        if (fmtver < 5)
            add_commands_from_person_states();

        if (fmtver <= 10)
        {
            for (i = 1; i < next_command; i++)
                game_commands[i].Arg2 = 1;
            for (i = 1; i < next_used_lvl_objective; i++)
                game_used_lvl_objectives[i].Arg2 = 1;
        }

        check_and_fix_commands();

        build_same_type_headers();

        check_and_fix_thing_commands();

        if (fmtver >= 10)
            level_misc_update(fmtver);

        if (level_deep_fix) {
            level_perform_deep_fix();
        }
        fix_level_indexes(missi, fmtver, reload, level_deep_fix);
    } else
    {
        LOGERR("Could not open mission file, load skipped");
    }
}

void fix_map_outranged_properties(void)
{
    ushort tile_x, tile_y;
    int i;

    for (tile_y = 0; tile_y < MAP_TILE_HEIGHT; tile_y++) {
        for (tile_x = 0; tile_x < MAP_TILE_WIDTH; tile_x++) {
            struct MyMapElement *p_mapel;
            ushort texture;

            p_mapel = &game_my_big_map[tile_y * MAP_TILE_WIDTH + tile_x];
            texture = p_mapel->Texture & 0x3FFF;
            if (texture >= next_floor_texture) {
                LOGERR("Outranged texture %d used in mapel at %d,%d",
                  (int)texture, (int)tile_x, (int)tile_y);
                p_mapel->Texture &= 0xC000;
                p_mapel->Texture |= texture % next_floor_texture;
            }
        }
    }
    for (i = 0; i < next_object_face; i++) {
        struct SingleObjectFace3 *p_face;
        ushort texture;

        p_face = &game_object_faces[i];
        texture = p_face->Texture & 0x3FFF;
        if (texture >= next_face_texture) {
            LOGERR("Outranged texture %d used in face3 %d", (int)texture, (int)i);
            p_face->Texture &= 0xC000;
            p_face->Texture |= texture % next_face_texture;
        }
    }
    for (i = 0; i < next_object_face4; i++) {
        struct SingleObjectFace4 *p_face;
        ushort texture;

        p_face = &game_object_faces4[i];
        texture = p_face->Texture & 0x3FFF;
        if (texture >= next_floor_texture) {
            LOGERR("Outranged texture %d used in face4 %d", (int)texture, (int)i);
            p_face->Texture &= 0xC000;
            p_face->Texture |= texture % next_floor_texture;
        }
    }
    for (i = 0; i < next_floor_texture; i++) {
        struct SingleFloorTexture *p_fltextr;
        ushort page;

        p_fltextr = &game_textures[i];
        page = p_fltextr->Page;
        if (page >= sizeof(vec_tmap)/sizeof(vec_tmap[0]) || vec_tmap[page] == NULL) {
            LOGERR("Outranged texture atlas page %d used in floor texture %d", (int)page, (int)i);
            p_fltextr->Page = 0;
        }
    }
    for (i = 0; i < next_face_texture; i++) {
        struct SingleTexture *p_fctextr;
        ushort page;

        p_fctextr = &game_face_textures[i];
        page = p_fctextr->Page;
        if (page >= sizeof(vec_tmap)/sizeof(vec_tmap[0]) || vec_tmap[page] == NULL) {
            LOGERR("Outranged texture atlas page %d used in face texture %d", (int)page, (int)i);
            p_fctextr->Page = 0;
        }
    }
}

void load_map_dat_pc_handle(TbFileHandle fh)
{
    u32 fmtver;
    ushort num_sthings, num_things;
    short i;

    LbFileRead(fh, &fmtver, sizeof(fmtver));
    if (fmtver >= 19)
    {
        assert(sizeof(struct MyMapElement) == 18);
        LbFileRead(fh, game_my_big_map, sizeof(struct MyMapElement) * 128 * 128);
    }
    else
    {
        struct MyMapElementOldV7 old_my_map_el;
        assert(sizeof(old_my_map_el) == 24);
        for (i = 0; i < 128 * 128; i++) {
            LbFileRead(fh, &old_my_map_el, sizeof(old_my_map_el));
            refresh_old_my_big_map_format(&game_my_big_map[i], &old_my_map_el, fmtver);
        }
    }
    if (fmtver <= 14)
    {
        struct SingleFloorTexture old_floor_texture; // Same fields, only the textures need matching
        LbFileRead(fh, &next_floor_texture, sizeof(next_floor_texture));
        for (i = 0; i < next_floor_texture; i++) {
            LbFileRead(fh, &old_floor_texture, sizeof(old_floor_texture));
            refresh_old_floor_texture_format(&game_textures[i], &old_floor_texture, fmtver);
        }
    }
    else
    {
        LbFileRead(fh, &next_floor_texture, sizeof(next_floor_texture));
        LbFileRead(fh, game_textures, sizeof(struct SingleFloorTexture) * next_floor_texture);
    }

    if (fmtver <= 14)
    {
        struct SingleTexture old_face_texture; // Same fields, only the textures need matching
        LbFileRead(fh, &next_face_texture, sizeof(next_face_texture));
        for (i = 0; i < next_face_texture; i++) {
            LbFileRead(fh, &old_face_texture, sizeof(old_face_texture));
            refresh_old_face_texture_format(&game_face_textures[i], &old_face_texture, fmtver);
        }
    }
    else
    {
        LbFileRead(fh, &next_face_texture, sizeof(next_face_texture));
        LbFileRead(fh, game_face_textures, sizeof(struct SingleTexture) * next_face_texture);
    }
    {
        LbFileRead(fh, &next_object_point, sizeof(next_object_point));
        LbFileRead(fh, game_object_points, sizeof(struct SinglePoint) * next_object_point);
    }
    if (fmtver >= 19)
    {
        LbFileRead(fh, &next_object_face, sizeof(next_object_face));
        assert(sizeof(struct SingleObjectFace3) == 32);
        LbFileRead(fh, game_object_faces, sizeof(struct SingleObjectFace3) * next_object_face);
    }
    else
    {
        struct SingleObjectFace3OldV7 old_object_face;
        LbFileRead(fh, &next_object_face, sizeof(next_object_face));
        assert(sizeof(old_object_face) == 48);
        for (i = 0; i < next_object_face; i++) {
            LbFileRead(fh, &old_object_face, sizeof(old_object_face));
            refresh_old_object_face_format(&game_object_faces[i], &old_object_face, fmtver);
        }
    }
    {
        LbFileRead(fh, &next_object, sizeof(next_object));
        LbFileRead(fh, game_objects, sizeof(struct SingleObject) * next_object);
    }
    {
        LbFileRead(fh, &next_quick_light, sizeof(next_quick_light));
        LbFileRead(fh, game_quick_lights, sizeof(struct QuickLight) * next_quick_light);
    }
    if (fmtver >= 14)
    {
        LbFileRead(fh, &next_full_light, sizeof(next_full_light));
        assert(sizeof(struct FullLight) == 32);
        LbFileRead(fh, game_full_lights, sizeof(struct FullLight) * next_full_light);
    }
    else
    {
        struct FullLightOldV12 old_full_light;
        LbFileRead(fh, &next_full_light, sizeof(next_full_light));
        assert(sizeof(old_full_light) == 40);
        for (i = 0; i < next_full_light; i++) {
            LbFileRead(fh, &old_full_light, sizeof(old_full_light));
            refresh_old_full_light_format(&game_full_lights[i], &old_full_light, fmtver);
        }
    }
    {
        LbFileRead(fh, &next_normal, sizeof(next_normal));
        LbFileRead(fh, game_normals, sizeof(struct Normal) * next_normal);
    }
    if (fmtver >= 19)
    {
        LbFileRead(fh, &next_object_face4, sizeof(next_object_face4));
        assert(sizeof(struct SingleObjectFace4) == 40);
        LbFileRead(fh, game_object_faces4, sizeof(struct SingleObjectFace4) * next_object_face4);
    }
    else if (fmtver >= 7)
    {
        struct SingleObjectFace4OldV7 old_object_face4;
        LbFileRead(fh, &next_object_face4, sizeof(next_object_face4));
        assert(sizeof(old_object_face4) == 60);
        for (i = 0; i < next_object_face4; i++) {
            LbFileRead(fh, &old_object_face4, sizeof(old_object_face4));
            refresh_old_object_face4_format(&game_object_faces4[i], &old_object_face4, fmtver);
        }
    }
    else
    {
        next_object_face4 = 1;
    }
    LOGSYNC("stats: object_faces=%hu objects=%hu quick_lights=%hu full_lights=%hu normals=%hu object_faces4=%hu",
      next_object_face, next_object, next_quick_light, next_full_light, next_normal, next_object_face4);

    clear_mapwho_on_whole_map();

    if (fmtver >= 5)
    {
        struct SimpleThing loc_sthing;

        LbFileRead(fh, &num_sthings, sizeof(num_sthings));
        assert(sizeof(struct SimpleThing) == 60);
        for (i = num_sthings - 1; i != -1; i--)
        {
            LbFileRead(fh, &loc_sthing, sizeof(struct SimpleThing));
            switch (loc_sthing.Type)
            {
            case SmTT_TRAFFIC:
                new_thing_traffic_clone(&loc_sthing);
                break;
            case SmTT_SMOKE_GENERATOR:
                new_thing_smoke_gen_clone(&loc_sthing);
                break;
            case SmTT_STATIC:
                new_thing_static_clone(&loc_sthing);
                break;
            default:
                {
                  char locbuf[256];
                  snprint_sthing(locbuf, sizeof(locbuf), &loc_sthing);
                  LOGWARN("Discarded: %s", locbuf);
                }
                break;
            }
        }
    }
    else
    {
        num_sthings = 0;
    }

    if (fmtver >= 6)
    {
        LbFileRead(fh, &next_anim_tmap, sizeof(next_anim_tmap));
        assert(sizeof(struct AnimTmap) == 54);
        LbFileRead(fh, game_anim_tmaps, sizeof(struct AnimTmap) * next_anim_tmap);
    }
    else
    {
        next_anim_tmap = 1;
        game_anim_tmaps[1].Texture = 0;
    }

    if (fmtver >= 19)
    {
        ushort num_things;
        struct Thing loc_thing;
        struct M33 loc_mat;
        short shut_h;

        shut_h = 100;
        LbFileRead(fh, &num_things, sizeof(num_things));
        assert(sizeof(struct Thing) == 168);
        for (i = num_things - 1; i > -1; i--)
        {
            LbFileRead(fh, &loc_thing, sizeof(struct Thing));
            if (loc_thing.U.UObject.Object <= 0) {
                char locbuf[256];
                snprint_thing(locbuf, sizeof(locbuf), &loc_thing);
                LOGWARN("Bad object %d in %s",
                  (int)loc_thing.U.UObject.Object, locbuf);
                continue;
            }
            switch (loc_thing.SubType)
            {
            case SubTT_BLD_MOVN_ROTOR:
            case SubTT_BLD_37:
                LbFileRead(fh, &loc_mat, sizeof(struct M33));
                new_thing_building_clone(&loc_thing, &loc_mat, shut_h);
                break;
            default:
                new_thing_building_clone(&loc_thing, NULL, shut_h);
                break;
            }
        }
    }
    else if (fmtver >= 8)
    {
        struct Thing loc_thing;
        struct ThingOldV9 old_thing;

        LbFileRead(fh, &num_things, sizeof(num_things));
        assert(sizeof(old_thing) == 216);
        for (i = num_things - 1; i > -1; i--)
        {
            struct SingleObject *p_sobj;

            LbFileRead(fh, &old_thing, sizeof(old_thing));
            //TODO map fmtver is unlikely to match level fmtver
            refresh_old_thing_format(&loc_thing, &old_thing, fmtver);
            if (loc_thing.U.UObject.Object <= 0) {
                char locbuf[256];
                snprint_thing(locbuf, sizeof(locbuf), &loc_thing);
                LOGWARN("Bad object %d in %s",
                  (int)loc_thing.U.UObject.Object, locbuf);
                continue;
            }
            p_sobj = &game_objects[loc_thing.U.UObject.Object];
            if (p_sobj->OffsetY == 32000) {
                continue;
            }
            new_thing_building_clone(&loc_thing, NULL, 100);
        }
    }
    else
    {
        num_things = 0;
    }

    if (fmtver >= 6)
    {
        LbFileRead(fh, &next_traffic_node, sizeof(next_traffic_node));
        assert(sizeof(struct TrafficNode) == 36);
        LbFileRead(fh, game_traffic_nodes, sizeof(struct TrafficNode) * next_traffic_node);
    }
    else
    {
        next_traffic_node = 1;
    }

    if (fmtver >= 12)
    {
        LbFileRead(fh, &next_light_command, sizeof(next_light_command));
        assert(sizeof(struct LightCommand) == 36);
        LbFileRead(fh, game_light_commands, sizeof(struct LightCommand) * next_light_command);
    }
    else
    {
       next_light_command = 1;
    }
    if (fmtver >= 13)
    {
        LbFileRead(fh, &next_bezier_pt, sizeof(next_bezier_pt));
        assert(sizeof(struct BezierPt) == 28);
        LbFileRead(fh, bezier_pts, sizeof(struct BezierPt) * next_bezier_pt);
    }
    else
    {
        next_bezier_pt = 1;
    }
    LOGSYNC("stats: sthings=%hu things=%hu traffic_nodes=%hu light_commands=%hu bezier_pts=%hu",
      num_sthings, num_things, next_traffic_node, next_light_command, next_bezier_pt);
}

void load_mad_pc_buffer(ubyte *mad_ptr, long rdsize)
{
    short shut_h;
    u32 fmtver;
    short i;

    shut_h = 100;
    fmtver = *(u32 *)mad_ptr;
    mad_ptr += 4;

    if (fmtver != 1) {
        LOGWARN("Unexpected MAD version %lu", (ulong)fmtver);
    }

    // Set amounts of quick_load array items
    for (i = 0; quick_load_pc[i].Size != 0; i++)
    {
        ushort *p_numb;
        p_numb = quick_load_pc[i].Numb;
        if (p_numb != NULL) {
            *p_numb = *(ushort *)mad_ptr;
            mad_ptr += 2;
        }
    }
    // Set data pointers within the quick_load items
    for (i = 0; quick_load_pc[i].Size != 0; i++)
    {
        int entsize, nentries;
        ushort *p_numb;
        *quick_load_pc[i].Ptr = mad_ptr;
        p_numb = quick_load_pc[i].Numb;
        if (p_numb != NULL) {
            entsize = quick_load_pc[i].Size;
            nentries = quick_load_pc[i].Extra + *p_numb;
        } else {
            nentries = quick_load_pc[i].Size;
            entsize = quick_load_pc[i].Extra;
        }
        mad_ptr += nentries * entsize + 2;
    }
    // Update mem_game[] to the new amounts
    for (i = 1; i < 17; i++)
    {
        ushort *p_numb;
        p_numb = quick_load_pc[i].Numb;
        mem_game[i].N = quick_load_pc[i].Extra + *p_numb;
    }

    memcpy(&selected_triangulation_no, mad_ptr, sizeof(selected_triangulation_no));
    mad_ptr += sizeof(selected_triangulation_no);
    memcpy(&triangulation_initied, mad_ptr, sizeof(triangulation_initied));
    mad_ptr += sizeof(triangulation_initied);
    assert(sizeof(struct Triangulation) == 60);
    memcpy(triangulation, mad_ptr, sizeof(struct Triangulation) * 4);
    mad_ptr += sizeof(struct Triangulation) * 4;
    assert(sizeof(struct TrTriangle) == 16);
    triangulation[0].Triangles = (struct TrTriangle *)mad_ptr;
    mad_ptr += sizeof(struct TrTriangle) * triangulation[0].max_Triangles;
    triangulation[1].Triangles = (struct TrTriangle *)mad_ptr;
    mad_ptr += sizeof(struct TrTriangle) * triangulation[1].max_Triangles;
    triangulation[2].Triangles = (struct TrTriangle *)mad_ptr;
    mad_ptr += sizeof(struct TrTriangle) * triangulation[2].max_Triangles;
    triangulation[3].Triangles = (struct TrTriangle *)mad_ptr;
    mad_ptr += sizeof(struct TrTriangle) * triangulation[3].max_Triangles;
    assert(sizeof(struct TrPoint) == 8);
    triangulation[0].Points = (struct TrPoint *)mad_ptr;
    mad_ptr += sizeof(struct TrPoint) * triangulation[0].max_Points;
    triangulation[1].Points = (struct TrPoint *)mad_ptr;
    mad_ptr += sizeof(struct TrPoint) * triangulation[1].max_Points;
    triangulation[2].Points = (struct TrPoint *)mad_ptr;
    mad_ptr += sizeof(struct TrPoint) * triangulation[2].max_Points;
    triangulation[3].Points = (struct TrPoint *)mad_ptr;
    mad_ptr += sizeof(struct TrPoint) * triangulation[3].max_Points;

    clear_mapwho_on_whole_map();

    ushort num_sthings;
    struct SimpleThing *p_clsthing;

    dword_177750 = mad_ptr;
    num_sthings = *(ushort *)mad_ptr;
    mad_ptr += 2;
    for (i = num_sthings - 1; i > -1; i--)
    {
        p_clsthing = (struct SimpleThing *)mad_ptr;
        mad_ptr += sizeof(struct SimpleThing);
        switch (p_clsthing->Type)
        {
        case SmTT_TRAFFIC:
            new_thing_traffic_clone(p_clsthing);
            break;
        case SmTT_SMOKE_GENERATOR:
            new_thing_smoke_gen_clone(p_clsthing);
            break;
        case SmTT_STATIC:
            new_thing_static_clone(p_clsthing);
            break;
        default:
            {
                char locbuf[256];
                snprint_sthing(locbuf, sizeof(locbuf), p_clsthing);
                LOGWARN("Discarded: %s", locbuf);
            }
            break;
        }
    }

    ushort num_things;
    struct Thing *p_clthing;

    num_things = *(ushort *)mad_ptr;
    mad_ptr += 2;
    for (i = num_things - 1; i > -1; i--)
    {
        p_clthing = (struct Thing *)mad_ptr;
        mad_ptr += sizeof(struct Thing);
        if (p_clthing->U.UObject.Object <= 0) {
            char locbuf[256];
            snprint_thing(locbuf, sizeof(locbuf), p_clthing);
            LOGWARN("Bad object %d in %s",
              (int)p_clthing->U.UObject.Object, locbuf);
            continue;
        }
        switch (p_clthing->SubType)
        {
        case SubTT_BLD_MOVN_ROTOR:
        case SubTT_BLD_37:
            new_thing_building_clone(p_clthing, (struct M33 *)mad_ptr, shut_h);
            mad_ptr += sizeof(struct M33);
            break;
        default:
            new_thing_building_clone(p_clthing, NULL, shut_h);
            break;
        }
    }

    if (mad_ptr - (ubyte *)dword_177750 >= 100000)
        unkn_mech_arr7 = mad_ptr;
    else
        unkn_mech_arr7 = dword_177750 + 100000;
}

TbResult load_map_dat(ushort mapno)
{
    char dat_fname[52];
    TbFileHandle fh;

    next_local_mat = 1;

    sprintf(dat_fname, "%s/map%03d.dat", "maps", mapno);
    fh = LbFileOpen(dat_fname, Lb_FILE_MODE_READ_ONLY);
    if (fh == INVALID_FILE) {
        return Lb_FAIL;
    }
    load_map_dat_pc_handle(fh);
    LbFileClose(fh);

    fix_map_outranged_properties();

    return Lb_SUCCESS;
}

void prepare_map_dat_to_play(void)
{
    update_map_thing_and_traffic_refs();
    unkn_lights_processing();
    update_map_flags();

    triangulation_initied = 0;
    selected_triangulation_no = -1;
    triangulation_allocate(1,9000);
    triangulation_allocate(2,9000);

    triangulation_select(2);
    triangulation_clear();
    generate_map_triangulation();

    triangulation_select(1);
    triangulation_clear();
    generate_map_triangulation();
}

TbResult load_map_mad(ushort mapno)
{
    char mad_fname[DISKPATH_SIZE];
    PathInfo *pinfo;
    long fsize;

    next_local_mat = 1;

    pinfo = &game_dirs[DirPlace_Maps];
    snprintf(mad_fname, DISKPATH_SIZE-1, "%s/map%03d.mad", pinfo->directory, mapno);
    fsize = LbFileLoadAt(mad_fname, scratch_malloc_mem);
    if (fsize == Lb_FAIL)
        return Lb_FAIL;

    load_mad_pc_buffer(scratch_malloc_mem, fsize);

    fix_map_outranged_properties();

    update_map_thing_and_traffic_refs();
    unkn_lights_processing();
    triangulation_select(1);

    return Lb_SUCCESS;
}

void load_map_bnb(ushort mapno)
{
#if 0
    asm volatile ("call ASM_load_map_bnb\n"
        : : "a" (mapno));
#endif
    char locstr[DISKPATH_SIZE];
    PathInfo *pinfo;
    TbFileHandle fh;
    ubyte Amin, Amax;
    ubyte Bmin, Bmax;

    fh = INVALID_FILE;
    pinfo = &game_dirs[DirPlace_Maps];
    snprintf(locstr, DISKPATH_SIZE-1, "%s/map%03d.b&b", pinfo->directory, mapno);
    if (LbFileExists(locstr))
        fh = LbFileOpen(locstr, Lb_FILE_MODE_READ_ONLY);

    if (fh == INVALID_FILE)
    {
        LOGSYNC("Using defaults in place of '%s' file", locstr);
        map_bnb.field_0 = 0;
        map_bnb.field_1 = 0;
        map_bnb.field_2 = 0;
        map_bnb.field_3 = 0;
        dword_177790 = 0;
    }
    else
    {
        LbFileRead(fh, &map_bnb, 4);
        LbFileClose(fh);
        dword_177790 = 2;
    }
    Amin = map_bnb.field_0;
    if (map_bnb.field_0 >= map_bnb.field_2)
        Amin = map_bnb.field_2;
    Bmin = map_bnb.field_1;
    if (map_bnb.field_1 >= map_bnb.field_3)
        Bmin = map_bnb.field_3;
    Amax = map_bnb.field_0;
    if (map_bnb.field_0 <= map_bnb.field_2)
        Amax = map_bnb.field_2;
    Bmax = map_bnb.field_1;
    if (map_bnb.field_1 <= map_bnb.field_3)
        Bmax = map_bnb.field_3;

    BAT_unknsub_20(Amin, Bmin, Amax, Bmax, vec_tmap[4] + 0xA040);
}

TbResult load_mad_pc(ushort mapno)
{
    TbResult ret;

    ingame.Flags |= GamF_Unkn00010000;
    init_free_explode_faces();
    if (mapno != 0) {
        load_map_bnb(mapno);
        ret = load_map_mad(mapno);
    } else {
        ret = Lb_OK;
    }
    sub_73C64("", 1);
    return ret;
}

/******************************************************************************/
