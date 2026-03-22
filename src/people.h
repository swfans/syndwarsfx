/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file people.h
 *     Header file for people.c.
 * @par Purpose:
 *     Routines implementing people.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef PEOPLE_H
#define PEOPLE_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum SubTypePerson {
  SubTT_PERS_NONE = 0x0,
  SubTT_PERS_AGENT = 0x1,
  SubTT_PERS_ZEALOT = 0x2,
  SubTT_PERS_PUNK_F = 0x3,
  SubTT_PERS_BRIEFCASE_M = 0x4,
  SubTT_PERS_WHITE_BRUN_F = 0x5,
  SubTT_PERS_MERCENARY = 0x6,
  SubTT_PERS_MECH_SPIDER = 0x7,
  SubTT_PERS_POLICE = 0x8,
  SubTT_PERS_PUNK_M = 0x9,
  SubTT_PERS_SCIENTIST = 0xA,
  SubTT_PERS_SHADY_M = 0xB,
  SubTT_PERS_HIGH_PRIEST = 0xC,
  SubTT_PERS_WHIT_BLOND_F = 0xD,
  SubTT_PERS_LETH_JACKT_M = 0xE,
  SubTT_PERS_FAST_BLOND_F = 0xF,
  SubTT_PERS_COUNT,
};

enum PersonState {
  PerSt_NONE = 0x0,
  PerSt_GOTO_POINT = 0x1,
  PerSt_BOMB_BUILDING = 0x2,
  PerSt_WANDER = 0x3,
  PerSt_EXPLODE_EFFECT = 0x4,
  PerSt_WAIT = 0x5,
  PerSt_AGENT_DEFENSIVE = 0x6,
  PerSt_PICKUP_ITEM = 0x7,
  /** Drop or plant an item (weapon or carried item) where standing.
   */
  PerSt_DROP_ITEM = 0x8,
  PerSt_SHOOT_PERSON_WITH_BGUN = 0x9,
  PerSt_SHOOT_BGUN_AT = 0xA,
  PerSt_DIE = 0xB,
  PerSt_DIEING = 0xC,
  PerSt_DEAD = 0xD,
  PerSt_INIT_SAVE_VICTIM = 0xE,
  PerSt_SAVE_VICTIM = 0xF,
  PerSt_GOTO_PERSON = 0x10,
  PerSt_WAIT_P_DEAD = 0x11,
  PerSt_WAIT_ALL_G_DEAD = 0x12,
  PerSt_WAIT_MEM_G_DEAD = 0x13,
  PerSt_WAIT_P_NEAR = 0x14,
  PerSt_WAIT_MEM_G_NEAR = 0x15,
  PerSt_WAIT_P_ARRIVES = 0x16,
  PerSt_WAIT_MEM_G_ARRIVES = 0x17,
  PerSt_WAIT_TIME = 0x18,
  PerSt_DEFINE_ROUTE = 0x19,
  PerSt_UNUSED_1A = 0x1A,
  PerSt_KILL_PERSON = 0x1B,
  PerSt_BLOCK_PERSON = 0x1C,
  PerSt_SCARE_PERSON = 0x1D,
  PerSt_USE_ITEM = 0x1E,
  PerSt_STATIONARY_SHOT = 0x1F,
  PerSt_RECOIL_HIT = 0x20,
  PerSt_VEH_PARKED = 0x21,
  PerSt_VEH_DRIVEN = 0x22,
  PerSt_GOTO_POINT_REL = 0x23,
  PerSt_PERSON_BURNING = 0x24,
  PerSt_IN_VEHICLE = 0x25,
  PerSt_DRIVING_VEHICLE = 0x26,
  PerSt_V_GOTO_POINT_FLY = 0x27,
  PerSt_PERSUADE_PERSON = 0x28,
  PerSt_FOLLOW_PERSON = 0x29,
  PerSt_SUPPORT_PERSON = 0x2A,
  PerSt_PROTECT_PERSON = 0x2B,
  PerSt_GET_ITEM = 0x2C,
  PerSt_USE_VEHICLE = 0x2D,
  PerSt_USE_VEHICLE_TO_POINT = 0x2E,
  PerSt_WAIT_VEHICLE = 0x2F,
  PerSt_GOTO_VEHICLE = 0x30,
  PerSt_CATCH_TRAIN = 0x31,
  PerSt_WAIT_TRAIN = 0x32,
  PerSt_DESTROY_BUILDING = 0x33,
  PerSt_WANDER_DRIVE = 0x34,
  /** Go to target position, and plant the mine currently in hand.
   */
  PerSt_GO_PLANT_MINE = 0x35,
  PerSt_WAIT_TO_EXIT_VEHICLE = 0x36,
  PerSt_CATCH_FERRY = 0x37,
  PerSt_EXIT_FERRY = 0x38,
  PerSt_AVOID_GROUP = 0x39,
  /** Init the current command on next state update.
   * This is used internally to ensure the thing inits the current command
   * rather than assuming that current means already initialised.
   */
  PerSt_INIT_COMMAND = 0x3A,
  PerSt_BEING_PERSUADED = 0x3B,
};

enum PersonFlags3 {
    PrsF3_Unkn01     = 0x01,
    PrsF3_Unkn02     = 0x02,
    PrsF3_Unkn04     = 0x04,
    PrsF3_Unkn08     = 0x08,
    PrsF3_Unkn10     = 0x10,
    PrsF3_Unkn20     = 0x20,
    PrsF3_Unkn40     = 0x40,
    PrsF3_Unkn80     = 0x80,
};

enum PersonAnims {
    ANIM_PERS_IDLE  = 0,
    ANIM_PERS_WEPLIGHT_IDLE = 1,
    ANIM_PERS_WEPHEAVY_IDLE = 2,
    ANIM_PERS_Unkn03  = 3,
    ANIM_PERS_Unkn04  = 4,
    ANIM_PERS_Unkn05  = 5,
    ANIM_PERS_Unkn06  = 6,
    ANIM_PERS_WEPHEAVY_Unkn07 = 7,
    ANIM_PERS_Unkn08  = 8,
    ANIM_PERS_Unkn09  = 9,
    ANIM_PERS_Unkn10  = 10,
    ANIM_PERS_LAY_DOWN = 11,
    ANIM_PERS_Unkn12  = 12,
    ANIM_PERS_Unkn13  = 13,
    ANIM_PERS_Unkn14  = 14,
    ANIM_PERS_WEPHEAVY_Unkn15 = 15,
    ANIM_PERS_PUSH_BACK = 16,
    ANIM_PERS_Unkn17  = 17,
    ANIM_PERS_DEAD_ASH = 18,
    ANIM_PERS_Unkn19  = 19,
    ANIM_PERS_DEAD_BODY  = 20,
    ANIM_PERS_Unkn21  = 21,
    ANIM_PERS_TOTAL_COUNT,
};

enum PersonSex {
    PERSON_MALE  = 0,
    PERSON_FEMALE,
};

/** Max health of a person; cannot safely go beyond that.
 */
#define PERSON_MAX_HEALTH_LIMIT 16383

/** Max weapon energy of a person; cannot safely go beyond that.
 */
#define PERSON_MAX_ENERGY_LIMIT 32255

/** Max shield charge of a person.
 */
#define PERSON_MAX_SHIELD 1024

#define PERSON_MAX_SPEED 2048

/** Length from person center to tip of the persons weapon, for beam weapons.
 *
 * Used as starting point of shots.
 */
#define PERSON_CENTER_TO_BEAM_WEAPON_TIP_MAPCOORD 128

/** Length from person center to tip of the persons weapon, for rocket weapons.
 *
 * Used as starting point of shots.
 */
#define PERSON_CENTER_TO_ROCKT_WEAPON_TIP_MAPCOORD 32

/** Length from person center to tip of the persons weapon, for throwing weapons.
 *
 * Used as starting point of shots.
 */
#define PERSON_CENTER_TO_THROW_WEAPON_TIP_MAPCOORD 32

/** Distance from bottom of the person to where the weapon barrel is.
 *
 * Used as starting point of shots.
 */
#define PERSON_BOTTOM_TO_WEAPON_HEIGHT 22

/** Distance from bottom of the person to where the throwing height is.
 *
 * Used as starting point of shots.
 */
#define PERSON_BOTTOM_TO_THROW_HEIGHT 10

/** Multiplayer when transferring weapon energy points to shield points.
 */
#define PERSON_ENERGY_TO_SHIELD_MUL 4

enum ThingWeaponSelectFlags {
    WepSel_TOGGLE = 0,
    WepSel_HIDE,
    WepSel_SELECT,
    WepSel_SKIP,
};

struct Thing;

struct PeepStat
{
    short MaxHealth;
    short MaxShield;
    short MaximumStamina;
    short MaxEnergy;
    short PersuadeReqd;
    short PersuadeWorth;
    short Speed;
    short dum2;
    short dum3;
    short dum4;
};

struct PeepStatAdd
{
    char Name[12];
};

/** Stores configuration for a state of a person.
 */
struct PeepStateConfig
{
    char Name[17];
};


struct MyPath {
    ubyte Flag;
    short X[4];
    short Z[4];
    ushort Next;
};

#pragma pack()
/******************************************************************************/
extern struct PeepStat peep_type_stats[];
extern ushort people_frames[SubTT_PERS_COUNT][22];

extern ushort word_1531DA;
extern ushort word_17FA58[400];

extern ushort head_my_path; // = 1;
extern ushort path_count;
extern struct MyPath my_paths[1600];

extern short unkn01_thing_idx;

void remove_path(struct Thing *p_thing);
void set_person_animmode_walk(struct Thing *p_person);

void load_peep_type_stats(void);

/** Get a string up to 14 chars containing person type name.
 */
const char *person_type_name(ushort ptype);

TbBool person_type_is_synd_agent(ushort ptype);
TbBool person_type_is_scientist(ushort ptype);
TbBool person_type_is_security(ushort ptype);

TbBool person_type_is_wide_definition_civilian(ushort ptype);

TbBool person_type_faction_is_syndicate(ushort ptype);

TbBool person_type_faction_is_church(ushort ptype);

TbBool person_type_faction_is_punks(ushort ptype);

TbBool person_type_is_any_major_faction(ushort ptype);

int person_type_get_persuasion_credit(ushort ptype);

/** Returns if a given type of person requires advanced persuadertron to be affected.
 */
TbBool person_type_only_affected_by_adv_persuader(ushort ptype);
TbBool person_only_affected_by_adv_persuader(ThingIdx person);

/** Print person state in function-like style to a buffer.
 */
void snprint_person_state(char *buf, ulong buflen, struct Thing *p_thing);

TbBool person_is_dead(ThingIdx thing);
TbBool person_is_dead_or_dying(ThingIdx thing);
ubyte person_get_selected_weapon(ThingIdx thing);

TbBool person_carries_weapon(struct Thing *p_person, WeaponType wtype);

/** Returns if a person carries medikit of any kind.
 */
TbBool person_carries_any_medikit(ThingIdx person);

/** Reset previous weapon, ignoring one currently in hand.
 *
 * Resets previously selected weapon visible in the players panel.
 */
void person_weapons_reset_previous(struct Thing *p_person);

/** Update previous weapon, either to one in hand or another carried one.
 */
void person_weapons_update_previous(struct Thing *p_person);

TbBool person_can_accept_control(ThingIdx person);

TbBool person_can_use_medikit(ThingIdx person);

TbBool person_has_supershield_active(ThingIdx person);
TbBool person_can_toggle_supershield(ThingIdx person);
void person_supershield_toggle(struct Thing *p_person);

/** Returns if a target thing is within shooting range of persons current weapon.
 */
TbBool person_has_weapon_target_within_range(struct Thing *p_person, ThingIdx target);

/** Returns if a person can be used as energy source to enable thermal view.
 */
TbBool person_can_sustain_thermal(ThingIdx person);

/** Updates person state due to thermal enabled; returns true if thermal is sustained.
 */
TbBool person_update_thermal(ThingIdx person);

/** Returns slot at which given person is in given players agents, or -1.
 */
short person_slot_as_player_agent(struct Thing *p_person, PlayerIdx plyr);

/** Returns if the person contains a link to players direct control slot in ComCur field.
 */
TbBool person_has_slot_in_any_player_dcontrol(ThingIdx person);

/** Returns player for which given person is in direct control list, or -1.
 */
short person_get_dcontrol_player(ThingIdx person);

/** Returns slot at which given person is in given players direct control list, or -1.
 */
short person_slot_in_player_dcontrol(ThingIdx person, PlayerIdx plyr);

/** Returns sex of a person, either PERSON_MALE or PERSON_FEMALE.
 */
ubyte person_sex(struct Thing *p_person);

ubyte person_mod_chest_level(struct Thing *p_person);
void set_person_mod_chest_level(struct Thing *p_person, ubyte nmod);
ubyte person_mod_legs_level(struct Thing *p_person);
void set_person_mod_legs_level(struct Thing *p_person, ubyte nmod);
ubyte person_mod_arms_level(struct Thing *p_person);
void set_person_mod_arms_level(struct Thing *p_person, ubyte nmod);
ubyte person_mod_brain_level(struct Thing *p_person);
void set_person_mod_brain_level(struct Thing *p_person, ubyte nmod);
ubyte person_mod_skin_level(struct Thing *p_person);
void set_person_mod_skin_level(struct Thing *p_person, ubyte nmod);

void set_person_stats_type(struct Thing *p_person, ushort type);
void set_person_health_shield_type(struct Thing *p_person, ushort stype);
void set_person_energy_stamina_type(struct Thing *p_person, ushort stype);

void init_person_thing(struct Thing *p_person);
void person_give_best_mods(struct Thing *p_person);
short calc_person_speed(struct Thing *p_person);

void check_persons_target(struct Thing *p_person);
void check_persons_target2(struct Thing *p_person);
ushort check_col_collision(int x, int y, int z);
void process_stamina(struct Thing *p_person);
void process_shield(struct Thing *p_person);
void process_person(struct Thing *p_person);
void process_random_speech(struct Thing *p_person, ubyte a2);

/** Bring killed person back to life.
 */
void person_resurrect(struct Thing *p_person);

/** Bring burning person back to life.
 */
void person_burning_stifle_fire(struct Thing *p_person);

/** Artificially increases health and max health of a person to maximal reasonable value.
 */
void person_set_helath_to_max_limit(struct Thing *p_person);

/** Artificially increases weapon energy and max energy of a person to maximal reasonable value.
 */
void person_set_energy_to_max_limit(struct Thing *p_person);

/** Artificially increases persuasion power of a person to allow parsuade anyone.
 */
void person_set_persuade_power__to_allow_all(struct Thing *p_person);


/** Switches AnimMode of a person without removing any shifts to current frame.
 */
void switch_person_anim_mode(struct Thing *p_person, ubyte animode);

/** Sets new AnimMode of a person, replacing the old frame number.
 */
void set_person_anim_mode(struct Thing *p_person, ubyte animode);

/** Resets AnimMode of a person, to some default based on state.
 */
void person_reset_default_anim_mode(struct Thing *p_person);

/** Sets new direction angle of a person, replacing the old frame number.
 */
void change_person_angle(struct Thing *p_person, ubyte angl);

/** Sets new direction angle of a person, using full precision angle as input.
 */
void change_person_angle_full(struct Thing *p_person, short full_angle);

/** Resets Frame number of a person, using its current properties.
 */
void reset_person_frame(struct Thing *p_person);

int can_i_see_thing(struct Thing *p_me, struct Thing *p_him, int max_dist, ushort flags);
TbBool can_i_enter_vehicle(struct Thing *p_me, struct Thing *p_vehicle);

int limit_mood(struct Thing *p_thing, short mood);

/** Init some commands which should be executed before the person is completely set up.
 */
void person_init_preplay_command(struct Thing *p_person);

TbBool person_is_executing_commands(ThingIdx person);
void person_start_executing_commands(struct Thing *p_person);

TbBool person_is_persuaded(ThingIdx thing);
TbBool person_is_persuaded_by_person(ThingIdx thing, ThingIdx owntng);
TbBool person_is_persuaded_by_player(ThingIdx thing, ushort plyr);

/** Check if a person is an agent controlled by a player different than given.
 */
TbBool person_is_other_players_agent(struct Thing *p_person, PlayerIdx plyr);

short person_shield_glow_brightness(struct Thing *p_thing);

void player_change_person(short thing, ushort plyr);
void make_peeps_scatter(struct Thing *p_person, int x, int z);

/** Performs hitting thing by bullet.
 *
 * @return 0 if dealt no damage this time (ie. protected by vehicle or has shield active),
 *   1 if cannot do damage permanently (ie. ally or already killed),
 *   < 0 if if dealt enough damage to destroy the thing. TODO: better return values?
 */
int thing_hit_by_bullet(struct Thing *p_person, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type);

/** Restores agents health by consuming a medikit, or just restores if no medikit available.
 */
TbBool person_use_medikit(struct Thing *p_person, PlayerIdx plyr);

void set_person_persuaded(struct Thing *p_person, struct Thing *p_attacker, ushort energy);

StateChRes person_init_drop_item_where_standing(struct Thing *p_person, ThingIdx item);
StateChRes person_init_plant_mine_where_standing(struct Thing *p_person, WeaponType wtype);
void person_init_pickup(struct Thing *p_person, ThingIdx item);

TbBool person_is_in_a_vehicle(struct Thing *p_person);
TbBool person_is_in_a_train(struct Thing *p_person);
TbBool person_is_standing_on_vehicle(struct Thing *p_person);
TbBool person_is_in_vehicle(struct Thing *p_person, ThingIdx vehicle);
void person_enter_vehicle(struct Thing *p_person, struct Thing *p_vehicle);

void thing_shoot_at_thing(struct Thing *p_thing, short target);
ubyte person_attempt_to_leave_vehicle(struct Thing *p_thing);
void thing_shoot_at_point(struct Thing *p_thing, short x, short y, short z, uint fast_flag);
void call_protect(struct Thing *p_thing, ushort plyr);
ushort count_protect(struct Thing *p_thing, ushort plyr);
void call_unprotect(struct Thing *p_thing, ushort plyr, ubyte flag);
void person_init_get_item(struct Thing *p_person, short item, ushort plyr);
void person_init_get_item_fast(struct Thing *p_person, short item, ushort plyr);
void person_init_plant_mine_fast(struct Thing *p_thing, short x, short y, short z, int face);
void person_init_plant_mine(struct Thing *p_person, short x, short y, short z, int face);
void set_person_dead(struct Thing *p_person, ushort anim_mode);

/** Selects a wielded weapon for the thing, or hides the weapon currently in hands.
 *
 * @param p_person The thing which will have current weapon switched.
 * @param wtype The weapon type to be used.
 * @param flag Value from ThingWeaponSelectFlags enum, telling whether to select or hide weapon.
 * @return Gives Value from ThingWeaponSelectFlags enum based on results of the action.
 */
ubyte thing_select_specific_weapon(struct Thing *p_person, WeaponType wtype, ubyte flag);
ubyte thing_select_best_weapon_for_range(struct Thing *p_person, int range);
ubyte thing_deselect_weapon(struct Thing *p_person);

void person_go_enter_vehicle_fast(struct Thing *p_person, struct Thing *p_vehicle, ushort plyr);
void person_go_enter_vehicle(struct Thing *p_person, struct Thing *p_vehicle);
void person_init_follow_person(struct Thing *p_person, struct Thing *p_other);
void person_self_destruct(struct Thing *p_person);

struct Thing *new_sim_person(int x, int y, int z, ubyte subtype);

void people_intel(ubyte flag);
void alert_peeps(int x, int y, int z, struct Thing *p_madman);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
