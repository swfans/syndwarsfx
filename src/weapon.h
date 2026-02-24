/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file weapon.h
 *     Header file for weapon.c.
 * @par Purpose:
 *     Weapons related functions.
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
#ifndef WEAPON_H
#define WEAPON_H

#include "bftypes.h"
#include "bfini.h"

#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define WEAPONS_CARRIED_MAX_COUNT 6
#define WEAPONS_FOURPACK_MAX_COUNT 4
#define WEP_CATEGORIES_COUNT 11

/** Range of the weapon can never exceed this limit.
 */
#define WEAPON_RANGE_BLOCKS_LIMIT 27

/** Amount of bombs in a single use.
 */
#define WEP_AIRSTRIKE_BOMBS_NUM 10

/** Game turns separating bombs.
 */
#define WEP_AIRSTRIKE_BOMB_GTURNS 20

/** Initial age of explosion; higher value makes the explosion shorter.
 */
#define WEP_AIRSTRIKE_IMPACT_GTURNS 14

/** Game turns between the weapon triggered and the bombs start.
 */
#define WEP_AIRSTRIKE_DELAY_GTURNS 20

/** Speed at which rockets move, in map coords.
 */
#define SHOT_ROCKED_SPEED 256

/** Gravitational pull of throwable weapons, in map coords.
 */
#define SHOT_THROWN_GRAVITY 500

/** Gravitational pull of throwable weapons, in map coords.
 */
#define SHOT_GRENADE_FLIGHT_GTURNS 16

enum WeaponType
{
  WEP_NULL = 0x0,
  WEP_UZI = 0x1,
  WEP_MINIGUN = 0x2,
  WEP_LASER = 0x3,
  WEP_ELLASER = 0x4,
  WEP_RAP = 0x5,
  WEP_NUCLGREN = 0x6,
  WEP_PERSUADRTRN = 0x7,
  WEP_FLAMER = 0x8,
  WEP_H2HTASER = 0x9,
  WEP_CRAZYGAS = 0xA,
  WEP_KOGAS = 0xB,
  WEP_ELEMINE = 0xC,
  WEP_EXPLMINE = 0xD,
  WEP_NAPALMMINE = 0xE,
  WEP_LONGRANGE = 0xF,
  WEP_AIRSTRIKE = 0x10,
  WEP_BEAM = 0x11,
  WEP_RAZORWIRE = 0x12,
  WEP_SONICBLAST = 0x13,
  WEP_QDEVASTATOR = 0x14,
  WEP_PERSUADER2 = 0x15,
  WEP_STASISFLD = 0x16,
  WEP_ENERGYSHLD = 0x17,
  WEP_SOULGUN = 0x18,
  WEP_TIMEGUN = 0x19,
  WEP_CEREBUSIFF = 0x1A,
  WEP_MEDI1 = 0x1B,
  WEP_MEDI2 = 0x1C,
  WEP_EXPLWIRE = 0x1D,
  WEP_CLONESHLD = 0x1E,
  WEP_UNUSED1F = 0x1F,
  WEP_TYPES_COUNT,
};

enum WeaponFourPackIndex
{
    WFRPK_ELEMINE = 0,
    WFRPK_EXPLMINE,
    WFRPK_NUCLGREN,
    WFRPK_CRAZYGAS,
    WFRPK_KOGAS,
    WFRPK_COUNT,
};

enum DamageType
{
  DMG_NULL = 0,
  DMG_UZI = 1,
  DMG_ELLASER = 2,
  DMG_BEAM = 3,
  DMG_LASER = 4,
  DMG_UNKN5 = 5,
  DMG_RAP = 6,
  DMG_MINIGUN = 7,
  DMG_LONGRANGE = 8,
  DMG_UNKN9 = 9,
  DMG_ELSTRAND = 10,
  DMG_TYPES_COUNT,
};

enum ProcessProximityWieldWeaponTargetSelect {
    PTargSelect_Persuader = 0,
    PTargSelect_PersuadeAdv,
    PTargSelect_SoulCollect,
};

struct Thing;

enum WeaponDefFlags
{
    WEPDFLG_None = 0, /**< No flags set. */
    WEPDFLG_UNKN0001 = 0x0001, /**< set for LASER/ELLASER. */
    WEPDFLG_UNKN0002 = 0x0002, /**< set for LASER/ELLASER. */
    WEPDFLG_UNKN0004 = 0x0004,
    WEPDFLG_UNKN0008 = 0x0008,
    WEPDFLG_CanPurchease = 0x0010, /**< Whether the weapon can be bought by player (when researched). */
};

struct WeaponDef {
  ushort RangeBlocks;
  ushort HitDamage;
  ushort ReFireDelay;
  ushort Weight;
  short EnergyUsed;
  ushort Accuracy;
  ushort TargetType;
  ushort TargetMode;
  ushort Flags;
  ubyte Sprite; // TODO would be better to store sprite index in 16-bit var
  ubyte Category;
  ushort Cost;
  ushort Funding;
  ubyte PercentPerDay;
};

/* Additional WeaponDef fields, to be later merged with that struct.
 */
struct WeaponDefAdd {
  char Name[12];
};

struct WeaponsFourPack {
  ubyte Amount[WFRPK_COUNT];
};

#pragma pack()
/******************************************************************************/
extern struct WeaponDef weapon_defs[33];
extern struct TbNamedEnum weapon_names[33];
extern ubyte weapon_tech_level[33];
extern ubyte weapon_sound[32];
extern ubyte weapon_sound_z[32];
extern short persuaded_person_weapons_sell_cost_permil;

void read_weapons_conf_file(void);
void init_weapon_text(void);

/** Gives a string which names the weapon.
 */
const char *weapon_codename(WeaponType wtype);

/** Returns if the weapon should be used by throwing.
 */
TbBool weapon_is_for_throwing(WeaponType wtype);

/** Returns if the weapon should be used by placing activated on the ground.
 */
TbBool weapon_is_for_planting(WeaponType wtype);

/** Returns if the weapon should be used where the wielder stands, which
 * consumes the thing and converts it into something else.
 */
TbBool weapon_is_for_deploying(WeaponType wtype);

/** Returns if the weapon should be used by spreading on the ground while walking.
 */
TbBool weapon_is_for_spreading_on_ground(WeaponType wtype);

/** Returns if the weapon activates at the location of the wielding person.
 */
TbBool weapon_is_deployed_at_wielder_pos(WeaponType wtype);

/** Returns if the weapon is for restoring parameters (ie. health) rather than inflicting damage.
 */
TbBool weapon_is_for_restoration(WeaponType wtype);

/** Returns if the weapon is targeted at / affects the wielding person.
 */
TbBool weapon_is_self_affecting(WeaponType wtype);

/** Returns if a weapon effects are not additive/stackable.
 *
 * Effects are unstackable if a single shot gives the same effect as multiple simultaneous shots.
 */
TbBool weapon_has_unstackable_effect(WeaponType wtype);

/** Returns if a weapon has limited amount of uses before disappearing.
 */
TbBool weapon_is_consumable(WeaponType wtype);

/** Returns if a weapon has ability to charge before firing, to deal more damage.
 */
TbBool weapon_can_be_charged(WeaponType wtype);

/** Returns if a weapon fire can be delayed to achieve target lock.
 *
 * Weapons with targetting cannot be charged, as these functions
 * use the same resources and same player controls.
 */
TbBool weapon_has_targetting(WeaponType wtype);

/** Returns flags for whether leader shooting a weapon should cause follower shoot his weapon as well.
 */
ubyte weapon_simultaneous_fire_in_group(WeaponType lead_wtype, WeaponType follwr_wtype);

/** Returns panel sprite index to be used to represent the weapon.
 */
ushort weapon_sprite_index(WeaponType wtype, TbBool enabled);

TbBool weapons_has_weapon(ulong weapons, WeaponType wtype);

/** Returns weapon set in given flags with index below last.
 */
ushort weapons_prev_weapon(ulong weapons, WeaponType last_wtype);

/** Returns how many weapon slots are occupied in given weapons flags.
 */
ushort weapons_count_used_slots(ulong weapons);

ushort weapon_fourpack_index(WeaponType wtype);
void weapons_remove_weapon(ulong *p_weapons,
  struct WeaponsFourPack *p_fourpacks, WeaponType wtype);

/** Remove one weapon from an npc person in-game.
 * NPCs have no FourPacks, meaning removing one consumable weapon does nothing.
 * For non-consumable weapons, this removes the related weapon normally.
 */
TbBool weapons_remove_one_from_npc(ulong *p_weapons, WeaponType wtype);

/** Remove one weapon from a player character, in Cryo Chamber.
 * Currently this is only for cryo chamber, as in-game fourpacks have different format.
 */
TbBool weapons_remove_one(ulong *p_weapons,
  struct WeaponsFourPack *p_fourpacks, WeaponType wtype);

/** Remove one weapon from player-controlled person in-game.
 * Player struct contains dumb own array rather than uniform WeaponsFourPack, so it requires
 * this special function.
 * DEPRECATED: To be removed when possible.
 */
TbBool weapons_remove_one_for_player(ulong *p_weapons,
  ubyte p_plfourpacks[][4], ushort plagent, WeaponType wtype);

void give_take_me_weapon(struct Thing *p_person, int item, int giveortake, short id);

TbBool weapons_add_one(ulong *p_weapons,
  struct WeaponsFourPack *p_fourpacks, WeaponType wtype);

TbBool weapons_add_one_for_player(ulong *p_weapons,
  ubyte p_plfourpacks[][4], ushort plagent, WeaponType wtype);

void sanitize_weapon_quantities(ulong *p_weapons,
  struct WeaponsFourPack *p_fourpacks);

ubyte find_nth_weapon_held(ushort index, ubyte n);

/** Gives range (in normal map coord points) of given weapon when wielded by given person.
 *
 * Does not check if the person is in a situation where it uses different
 * weapon than the one in hand, ie. ignores if the person is inside a tank.
 */
short get_hand_weapon_range(struct Thing *p_person, WeaponType wtype);

/** Gives range (in normal map coord points) of the person current weapon.
 *
 * Does not check if the person is in a situation where it uses different
 * weapon than the one in hand, ie. ignores if the person is inside a tank.
 */
short current_hand_weapon_range(struct Thing *p_person);

/** Returns range (in normal map coord points) of the person current weapon.
 *
 * Implements modifications for vehicle weapons and throwing weapons.
 */
int get_weapon_range(struct Thing *p_person);

int get_weapon_zoom_min(WeaponType wtype);

void choose_best_weapon_for_range(struct Thing *p_person, int dist);

TbBool person_can_be_persuaded_now(ThingIdx attacker, ThingIdx target,
  short weapon_range, ubyte target_select, ushort *energy_reqd);

/** Returns whether the person current weapon requires some time to achieve target lock.
 */
TbBool current_weapon_has_targetting(struct Thing *p_person);

ulong person_carried_weapons_pesuaded_sell_value(struct Thing *p_person);

TbBool person_weapons_remove_one(struct Thing *p_person, WeaponType wtype);

void do_weapon_quantities_net_to_player(struct Thing *p_person);
void player_agent_set_weapon_quantities_max(struct Thing *p_person);
void player_agent_set_weapon_quantities_proper(struct Thing *p_person);

void process_weapon(struct Thing *p_person);
short process_persuadertron(struct Thing *p_person, ubyte target_select, ushort *energy_reqd);
void process_weapon_wind_down(struct Thing *p_person);
int gun_out_anim(struct Thing *p_person, ubyte shoot_flag);

ushort set_player_weapon_turn(struct Thing *p_person, ushort delay_turns);

s32 laser_hit_at(s32 x1, s32 y1, s32 z1, s32 *x2, s32 *y2, s32 *z2, struct Thing *p_shot);
void finalise_razor_wire(struct Thing *p_person);
void init_lay_razor(struct Thing *p_thing, short x, short y, short z, int flag);
void init_mgun_laser(struct Thing *p_owner, ushort start_age);

void process_clone_disguise(struct Thing *p_person);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
