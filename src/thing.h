/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file thing.h
 *     Header file for thing.c.
 * @par Purpose:
 *     Thing structure support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 May 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef THING_H
#define THING_H

#include "bftypes.h"
#include "game_bstype.h"
#include "cybmod.h"
#include "vehtraffic.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define THINGS_LIMIT 1000
#define STHINGS_LIMIT 1500

#define INVALID_THING &things[0]

enum ThingType {
    TT_NONE = 0x0,
    TT_UNKN1 = 0x1,
    TT_VEHICLE = 0x2,
    TT_PERSON = 0x3,
    TT_UNKN4 = 0x4,
    SmTT_STATIC = 0x5,
    TT_ROCKET = 0x6,
    TT_UNKN7 = 0x7,
    TT_UNKN8 = 0x8,
    TT_BUILDING = 0x9,
    SmTT_TRAFFIC = 0xA,
    TT_LASER11 = 0xB,
    TT_LASER_GUIDED = 0xC,
    TT_UNKN13 = 0xD,
    TT_UNKN14 = 0xE,
    TT_MINE = 0xF,
    TT_GRENADE = 0x10,
    SmTT_SPARK = 0x11,
    TT_UNKN18 = 0x12,
    TT_LASER_ELEC = 0x13,
    SmTT_INTELLIG_DOOR = 0x14,
    SmTT_SCALE_EFFECT = 0x15,
    SmTT_NUCLEAR_BOMB = 0x16,
    TT_UNKN23 = 0x17,
    SmTT_SMOKE_GENERATOR = 0x18,
    SmTT_DROPPED_ITEM = 0x19,
    SmTT_CARRIED_ITEM = 0x1A,
    SmTT_ELECTRIC_STRAND = 0x1B,
    TT_RAZOR_WIRE = 0x1C,
    TT_LASER29 = 0x1D,
    SmTT_TIME_POD = 0x1E,
    TT_AIR_STRIKE = 0x1F,
    SmTT_CANISTER = 0x20,
    TT_VEH_TURRET = 0x21,
    TT_UNKN34 = 0x22,
    TT_UNKN35 = 0x23,
    SmTT_STASIS_POD = 0x24,
    SmTT_SOUL = 0x25,
    TT_LASER38 = 0x26,
    TT_UNKN39 = 0x27,
    TT_UNKN40 = 0x28,
    TT_UNKN41 = 0x29,
    TT_UNKN42 = 0x2A,
    TT_UNKN43 = 0x2B,
    TT_UNKN44 = 0x2C,
    TT_UNKN45 = 0x2D,
    TT_UNKN46 = 0x2E,
    TT_UNKN47 = 0x2F,
    TT_UNKN48 = 0x30,
    TT_UNKN49 = 0x31,
    SmTT_BANG = 0x32,
    TT_UNKN51 = 0x33,
    SmTT_FIRE = 0x34,
    SmTT_SFX = 0x35,
    SmTT_TEMP_LIGHT = 0x36,
    TT_UNKN55 = 0x37,
    TT_UNKN56 = 0x38,
};

enum ThingFlags {
    TngF_Unkn0001     = 0x0001,
    TngF_Destroyed    = 0x0002,
    TngF_Unkn0004     = 0x0004,
    TngF_Unkn0008     = 0x0008,
    TngF_Unkn0010     = 0x0010,
    TngF_Unkn0020     = 0x0020,
    TngF_Unkn0040     = 0x0040,
    TngF_Unkn0080     = 0x0080,
    TngF_PersSupShld  = 0x0100,
    TngF_VehUnkn0100  = 0x0100,
    TngF_StationrSht  = 0x0200,
    TngF_WepCharging  = 0x0400,
    TngF_Unkn0800     = 0x0800,
    TngF_Unkn1000     = 0x1000,
    TngF_PlayerAgent  = 0x2000,
    TngF_Unkn4000     = 0x4000,
    TngF_Unkn8000     = 0x8000,
    TngF_WepRecoil    = 0x00010000,
    TngF_Unkn00020000 = 0x00020000,
    TngF_Unkn00040000 = 0x00040000,
    TngF_Persuaded    = 0x00080000,
    TngF_Unkn00100000 = 0x00100000,
    TngF_Unkn00200000 = 0x00200000,
    TngF_Unkn00400000 = 0x00400000,
    TngF_Unkn00800000 = 0x00800000,
    TngF_Unkn01000000 = 0x01000000,
    TngF_Unkn02000000 = 0x02000000,
    TngF_Unkn04000000 = 0x04000000,
    TngF_Unkn08000000 = 0x08000000,
    TngF_InVehicle    = 0x10000000,
    TngF_Unkn20000000 = 0x20000000,
    TngF_Unkn40000000 = 0x40000000,
};

/** Transformation into closed was requested on the building thing.
 * A flag with meaning for people, is reused for different purpose
 * when set on a building.
 */
#define TngF_TransOpenRq TngF_Unkn0040

/** Transformation into open was requested on the building thing.
 * A flag with meaning for people, is reused for different purpose
 * when set on a building.
 */
#define TngF_TransCloseRq TngF_Unkn0080

/** Thing on which a person is standing is a valid and really near vehicle.
 * The flag has this meaning for people, different meaning for other things.
 */
#define TngF_StandOnVehicle TngF_Unkn01000000

enum ThingFlags2 {
    TgF2_Unkn0001     = 0x0001,
    TgF2_Unkn0002     = 0x0002,
    TgF2_Unkn0004     = 0x0004,
    TgF2_Unkn0008     = 0x0008,
    TgF2_KnockedOut   = 0x0010,
    TgF2_Unkn0020     = 0x0020,
    TgF2_Unkn0040     = 0x0040,
    TgF2_Unkn0080     = 0x0080,
    TgF2_Unkn0100     = 0x0100,
    TgF2_Unkn0200     = 0x0200,
    TgF2_Unkn0400     = 0x0400,
    TgF2_Unkn0800     = 0x0800,
    TgF2_Unkn1000     = 0x1000,
    TgF2_Unkn2000     = 0x2000,
    TgF2_Unkn4000     = 0x4000,
    TgF2_Unkn8000     = 0x8000,
    TgF2_Unkn00010000 = 0x00010000,
    TgF2_Unkn00020000 = 0x00020000,
    TgF2_Unkn00040000 = 0x00040000,
    TgF2_Unkn00080000 = 0x00080000,
    TgF2_Unkn00100000 = 0x00100000,
    TgF2_Unkn00200000 = 0x00200000,
    TgF2_Unkn00400000 = 0x00400000,
    TgF2_Unkn00800000 = 0x00800000,
    /** The thing is not added to map content lists and is invisible.
     *
     * If set, the thing is invisible and on-map things cannot affect it.
     * Non-existent thing can still execute commands though, working as
     * invisible helper for creating level mechanics.
     * The flag can be controlled by CMD_PING_EXIST, spawning and despawning
     * the thing on the map.
     */
    TgF2_ExistsOffMap = 0x01000000,
    TgF2_Unkn02000000 = 0x02000000,
    TgF2_SoulDepleted = 0x04000000,
    TgF2_Unkn08000000 = 0x08000000,
    TgF2_Unkn10000000 = 0x10000000,
    TgF2_InsideBuilding = 0x20000000,
    TgF2_Unkn40000000 = 0x40000000,
    TgF2_IgnoreEnemies = 0x80000000,
};

enum StateChangeResult {
    StCh_ACCEPTED = 0,  /**< The new state was set to the world element. */
    StCh_ALREADY,       /**< The conditions to finalize the state were already met, state not set as it is completed. */
    StCh_DENIED,        /**< The current state of either target or other world elements prevents entering the state at this time. */
    StCh_UNATTAIN,      /**< The current state of the world elements makes it impossible to ever enter that state, ie. target does not exist. */
};

typedef ubyte StateChRes;

struct M33;

/** State of Thing of type Object/Building.
 */
struct TngUObject
{
    short NextThing;
    short PrevThing;
    ubyte Group;
    ubyte EffectiveGroup;
    short Object;
    short MatrixIndex;
    ubyte NumbObjects;
    ubyte Angle;
    ubyte Token;
    char TokenDir;
    char OffX;
    char OffZ;
    short TargetDX; /**< Thing offs=0x5C */
    short TargetDY;
    short TargetDZ;
    ushort BuildStartVect; /**< Index of the first ColVectList for toggling passability which are associated to this object. */
    ushort BuildNumbVect; /**< Amount of ColVectList items for toggling passability which are associated to this object. */
    ushort ZZ_unused_but_pads_to_long_ObjectNo;
    ushort ComHead;
    short ComCur;
    short Mood; /**< Thing offs=0x6C */
    short RaiseDY[2];
    short RaiseY[2];
    short MinY[2];
    short Timer[2];
    short MaxY[2]; /**< Thing offs=0x7E */
    short TNode;
    short Cost;
    short Shite;
    ulong BHeight;
    ushort Turn;
    ushort TurnPadOnPS;
    short tnode[4];  /**< Thing offs=0x90 */
    ubyte player_in_me;
    ubyte unkn_4D;
    ulong DrawTurn; // set within draw_thing_object() for a building
    short tnode_50[4];
};

/** State of Thing of type Mounted Gun.
 */
struct TngUMGun
{
    short PathIndex;
    ushort UniqueID;
    ubyte Group;
    ubyte EffectiveGroup;
    short Object;
    short MatrixIndex;
    ubyte NumbObjects;
    ubyte ShotTurn;
    short WeaponTurn;
    ushort ObjectNo;
    short PrevThing; /**< Thing offs=0x5C */
    short NextThing;
    ubyte Token;
    char TokenDir;
    short AngleDY;
    short AngleX;
    short AngleY;
    short AngleZ;
    short GotoX;
    short GotoY; /**< Thing offs=0x6C */
    short GotoZ;
    int Dummy11[5];
    short RecoilTimer; /**< Thing offs=0x84 */
    ushort MaxHealth;
    ubyte CurrentWeapon;
    ubyte Dummy12;
    ubyte Dummy13[6];
};

/** State of Thing of type Vehicle.
 */
struct TngUVehicle
{
  short PathIndex;
  ushort UniqueID;
  ubyte Group;
  ubyte EffectiveGroup;
  short Object;
  short MatrixIndex;
  ubyte NumbObjects;
  ubyte Dummy2;
  short WeaponTurn;
  short ReqdSpeed;
  ushort MaxSpeed; // within Thing, pos=92
  /** Index of a first thing in a linked list of passengers.
   */
  short PassengerHead;
  /** Index of a Traffic Node (negative) or Station thing (positive) where the vehicle is moving.
   * Most vehicles travel between traffic nodes. Trains travel beween stations.
   */
  short TNode;
  short AngleDY;
  short AngleX;
  short AngleY;
  short AngleZ;
  short GotoX;
  short GotoY;
  short GotoZ;
  short VehicleAcceleration;
  ushort LeisurePlace;
  short TargetDX;
  short TargetDY;
  short TargetDZ;
  short OnFace;
  ushort WorkPlace;
  ushort ComHead;
  short ComCur;
  short Timer2;
  short RecoilTimer;
  ushort MaxHealth;
  short Dummy4a;
  short Dummy4b;
  short Dummy5a;
  short Dummy5b;
  short AccelZ;
  short AccelX;
  ushort SubThing;
  short Agok;
  int WobbleZP;
  int WobbleZV;
  ubyte Armour;
  ubyte PissedOffWithWaiting;
  short ZebraOldHealth;
  ushort destx;
  ushort destz;
};

/** State of Thing of type Effect.
 */
struct TngUEffect
{
  ushort MaxSpeed;
  short ReqdSpeed;
  ubyte Group;
  ubyte EffectiveGroup;
  short Object;
  short MatrixIndex;
  ubyte NumbObjects;
  ubyte Angle;
  short PassengerHead;
  short TNode;
  short AngleDY; // within Thing, pos=92
  short AngleX;
  short AngleY;
  short AngleZ;
  short GotoX;
  short GotoY;
  short GotoZ;
  short VehicleAcceleration;
  short MatrixIndexBAD;
  ushort LeisurePlace;
};

/** Person Draw Sprite Frame.
 */
struct DrawFrameId
{
  ubyte Version[5];
};

/** State of Thing of type Person.
 */
struct TngUPerson
{
  short PathIndex;
  ushort UniqueID;
  /** Normal GroupId assignment of a thing.
   * Used to reset `EffectiveGroup` when neccessary.
   */
  ubyte Group;
  /** Effective group is the GroupId of a thing which is actually in effect.
   * It may diverge from normal `Group` ie when a Person is persuaded.
   */
  ubyte EffectiveGroup;
  ushort ComHead;
  /** Index of the currently active entry in the Commands array.
   * Unless it's a player-controlled agent, then this is the player index and MyAgent index.
   */
  ushort ComCur;
  sbyte SpecialTimer;
  ubyte Angle;
  short WeaponTurn;
  ubyte Brightness;
  ubyte ComRange;
  ubyte BumpMode; // within Thing, pos=92
  ubyte BumpCount;
  /** Index of a thing under this one (item available for pickup or vehicle).
   */
  union {
  ThingIdx Vehicle;
  ThingIdx StandOnThing;
  };
  /** Index of a next thing in a linked list of passengers.
   */
  short LinkPassenger;
  ushort Within;
  ushort LastDist;
  short ComTimer;
  short Timer2;
  short StartTimer2;
  ubyte AnimMode;
  ubyte OldAnimMode;
  short OnFace;
  union Mod UMod;
  short Mood;
  struct DrawFrameId FrameId;
  ubyte Shadows[4];
  ubyte RecoilTimer;
  ushort MaxHealth;
  ubyte Flag3;
  ubyte OldSubType;
  short ShieldEnergy;
  ubyte ShieldGlowTimer;
  ubyte WeaponDir;
  ushort SpecialOwner;
  ushort WorkPlace;
  ushort LeisurePlace;
  /** The weapon timer is used for both re-fire delay
   * and weapon overcharging.
   */
  short WeaponTimer;
  short Target2;
  short MaxShieldEnergy;
  short PersuadePower;
  short MaxEnergy;
  short Energy;
  ubyte RecoilDir;
  ubyte CurrentWeapon;
  short GotoX;
  short GotoZ;
  short TempWeapon;
  short Stamina;
  short MaxStamina;
  ulong WeaponsCarried;
};

/** Structure for storing State of any Thing.
 * Everything in game besides ground terrain, are Things.
 */
struct Thing { // sizeof=168
    /** Previous thing in the `mapwho` linked list?
     * May be more complicated - see this field is SimpleThing.
     */
    short Parent;
    /** Next thing in the `mapwho` linked list.
     */
    ThingIdx Next;
    short LinkParent;
    short LinkChild;
    ubyte SubType;
    ubyte Type;
    short State;
    ulong Flag;
    short LinkSame;
    short LinkSameGroup;
    short Radius;
    ThingIdx ThingOffset;
    long X;
    long Y;
    long Z;
    short Frame;
    ushort StartFrame;
    short Timer1;
    short StartTimer1;
    /** Move velocity vector or shoot target position; X coord.
     * The vector set is expected to be normalized, length od 256 means full speed.
     */
    s32 VX;
    /** Move velocity vector, Y coord.
     */
    s32 VY;
    /** Move velocity vector, Z coord.
     */
    s32 VZ;
    short Speed;
    short Health;
    short Owner;
    ubyte PathOffset;
    ubyte SubState;
    struct Thing *PTarget;
    ulong Flag2;
    ThingIdx GotoThingIndex;
    short OldTarget;
    union { // pos=76
        struct TngUObject UObject;
        struct TngUMGun UMGun;
        struct TngUVehicle UVehicle;
        struct TngUEffect UEffect;
        struct TngUPerson UPerson;
    } U;
};

struct STngUEffect {
    short VX;
    short VY;
    short VZ;
    short OX;
    short OY;
    short OZ;
};

struct STngUScenery {
    ushort Frame;
    ushort StartFrame;
    short LightHead;
    short LightDie;
    short LightAnim;
    short Health;
};

struct STngUWeapon {
    short WeaponType;
    short LastFired;
    short Ammo;
    short Owner;
    short OnFace;
};

struct STngULight {
    short MinBright;
    short RangeBright;
};

struct STngUBang {
    short shrapnel;
    short phwoar;
};

struct STngUFire {
    short flame;
};

struct SimpleThing
{
    /** Index of some kind of entity which generated the thing.
     * Speciifics depend on thing type; often it's another thing index,
     * but not always; ie. for doors, the Parent is rectangular face index.
     */
    short Parent;
    /** Next thing in the `mapwho` linked list.
     */
    ThingIdx Next;
    short LinkParent;
    short LinkChild;
    ubyte SubType;
    ubyte Type;
    short State;
    ulong Flag;
    short LinkSame;
    short Object;
    short Radius;
    ThingIdx ThingOffset;
    long X;
    long Y;
    long Z;
    short Frame;
    short StartFrame;
    short Timer1;
    short StartTimer1;
    union {
        struct STngUEffect UEffect;
        struct STngUScenery UScenery;
        struct STngUWeapon UWeapon;
        struct STngUTraffic UTraffic;
        struct STngULight ULight;
        struct STngUBang UBang;
        struct STngUFire UFire;
    } U;
    short Owner2;
    ushort UniqueID;
};

/** Old structure for storing State of any Thing.
 * Used only to allow reading old, pre-release levels.
 */
struct ThingOldV9 { // sizeof=216
    short Parent;
    short Next;
    short LinkParent;
    short LinkChild;
    ubyte SubType;
    ubyte Type;
    short State; // pos=10
    ulong Flag;
    /* Since fmtver=4, this and all previous fields confirmed to match
     * file layout in final release (from Pre-Alpha Demo code analysis
     * and from comparative analysis of binary data in level files).
     */
    short LinkSame;
    /** Stores index of first item within game_objects[].
     * Since fmtver=4, most types (Object,Vehicle,MGun,Effect) have `Object`
     * and it is at the same place for all types (from Pre-Alpha Demo code
     * analysis). The Vehicle `Object` confirmed in fmtver=8-11 files (from
     * comparative analysis of binary data in level files).
     */
    union {
        short ObjectObject;
        short VehicleObject;
        short MGunObject;
        short EffectObject;
    };
    short Radius; // pos=20
    /* Since fmtver=4, most types (Object,Vehicle,MGun,Effect) have `NumbObjects`
     * and it is at the same place (from Pre-Alpha Demo code analysis).
     */
    union {
        ubyte ObjectNumbObjects;
        ubyte VehicleNumbObjects;
        ubyte MGunNumbObjects;
        ubyte EffectNumbObjects;
    };
    ubyte TngUnkn23;
    long X;
    long Y;
    long Z;
    /* Since fmtver=4, most types have `Frame`, only Object type GATE reuses it
     * for `MinY[0]` (from Pre-Alpha Demo code analysis).
     */
    union {
        short Frame;
        short ObjectMinY0;
    };
    /* Since fmtver=4, most types have `StartFrame`, only Object type GATE
     *  reuses it for `RaiseY[1]` (from Pre-Alpha Demo code analysis).
     */
    union {
        ushort StartFrame;
        short ObjectRaiseY1;
    };
    short Timer1; // pos=40
    short StartTimer1;
    /* Since fmtver=4, `Timer2` for Person and Vehicle, but Objects have
     * `Timer[0]` (from Pre-Alpha Demo code analysis).
     */
    union {
        short Timer2;
        short ObjectTimer0;
    };
    /* Since fmtver=4, `StartTimer2` for Person, but Objects have `Timer[1]`
     * (from Pre-Alpha Demo code analysis).
     */
    union {
        short StartTimer2;
        short ObjectTimer1;
    };
    /* Since fmtver=4, Person `AnimMode` (from Pre-Alpha Demo code analysis).
     * Confirmed to be low value used against Person in fmtver=8-11 files
     * (from comparative analysis of binary data in level files).
     */
    ubyte PersonAnimMode; // pos=48
    /* Since fmtver=4, Person `OldAnimMode` (from Pre-Alpha Demo code analysis).
     */
    ubyte PersonOldAnimMode;
    ushort ThingOffset; // pos=50
    /* Since fmtver=4, usually `VX`, but Object type GATE reuses it for
     * `RaiseDY[]` (from Pre-Alpha Demo code analysis).
     * The `VX`/`VY`/`VZ` confirmed in fmtver=8-11 (from comparative analysis
     * of binary data in level files).
     */
    union {
        long VX;
        short ObjectRaiseDY[2];
    };
    long VY;
    long VZ;
    /* Since fmtver=4, `AngleX` for Vehicle and MGun (from Pre-Alpha Demo code
     * analysis). Stamina is only here in fmtver=12 files - does not seem to
     * exist in 9-11 (from comparative analysis of binary data in level files).
     */
    union {
        short VehicleAngleX;
        short MGunAngleX;
        short PersonStamina;
    };
    /* Since fmtver=4, `AngleY` for Vehicle and MGun (from Pre-Alpha Demo code
     * analysis). MaxStamina is only here in fmtver=12 files (from comparative
     * analysis of binary data in level files).
     */
    union {
        short VehicleAngleY;
        short MGunAngleY;
        short PersonMaxStamina;
    };
    /* Since fmtver=4, `AngleZ` for Vehicle and MGun (from Pre-Alpha Demo code
     * analysis).
     */
    union { // pos=68
        short VehicleAngleZ;
        short MGunAngleZ;
    };
    /* The `LinkSameGroup` confirmed in fmtver=8-11 (from comparative analysis
     * of binary data in level files).
     */
    short LinkSameGroup; // pos=70
    short TngUnkn72;
    /* Since fmtver=4, `Angle` for Person and Object (from Pre-Alpha Demo code
     * analysis). Confirmed to be some Person stat in fmtver=8-11 (from
     * comparative analysis of binary data in level files).
     */
    union {
        ubyte PersonAngle;
        ubyte ObjectAngle;
    };
    /** Effective group is the GroupId of a thing which is actually in effect.
     * Configrmed to be  Person `EffectiveGroup` since fmtver=4 (from Pre-Alpha
     * Demo code analysis). Also xonfirmed in fmtver=8-11 (from comparative
     * analysis of binary data in level files). These sources do not provide a
     * definite proof about which group field is effective and which the default.
     */
    union { // pos=75
        ubyte PersonEffectiveGroup;
        ubyte VehicleEffectiveGroup;
        ubyte ObjectEffectiveGroup;
        ubyte MGunEffectiveGroup;
    };
    /* Since fmtver=4, `Speed` for Vehicle (from Pre-Alpha Demo code analysis).
     */
    short Speed; // pos=76
    short Health; // pos=78
    /* Since fmtver=4, `GotoX` for Person (from Pre-Alpha Demo code analysis).
     * Assumed to be `GotoX` for most types (MGun,Vehicle,Effect).
     */
    union { // pos=80
        short PersonGotoX;
        short MGunGotoX;
        short VehicleGotoX;
        short EffectGotoX;
    };
    /* Since fmtver=4, `GotoY` for Person (from Pre-Alpha Demo code analysis).
     * Though there is no Parson `GotoY` in final, this is assumed to be `GotoY`
     * for most types (MGun,Vehicle,Effect).
     */
    union { // pos=82
        short PersonGotoY;
        short MGunGotoY;
        short VehicleGotoY;
        short EffectGotoY;
    };
    /* Since fmtver=4, `GotoZ` for Person (from Pre-Alpha Demo code analysis).
     * Assumed to be `GotoZ` for most types (MGun,Vehicle,Effect).
     */
    union { // pos=84
        short PersonGotoZ;
        short MGunGotoZ;
        short VehicleGotoZ;
        short EffectGotoZ;
    };
    /* Since fmtver=4, `Group` for Person (from Pre-Alpha Demo code analysis).
     * Confirmed the same in fmtver=8-11 (from comparative analysis of
     * binary data in level files).
     */
    union {  // pos=86
        short VehicleUnknTng86;
        struct {
            ubyte PersonGroup;
            ubyte TngUnkn87;
        };
    };
    ulong PersonWeaponsCarried; // pos=88
    /** Next command assigned to the Person.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis).
     */
    ushort PersonComHead; // pos=92
    ushort PersonComCur; // pos=94
    short PersonComTimer; // pos=96
    short TngUnkn98; // pos=98
    /** Index of a thing which owns this one.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis). Also
     * confirmed  in fmtver=8-11 (from comparative analysis of binary data
     * in level files).
     */
    ushort Owner; // pos=100
    /* Confirmed to be Person `CurrentWeapon` since fmtver=4 (from
     * Pre-Alpha Demo code analysis). Uses low values, for Person only
     * (from comparative analysis of binary data in level files).
     */
    short PersonCurrentWeapon; // pos=102
    short TngUnkn104; // pos=104
    /* Confirmed to be Person `WeaponTurn` since fmtver=4 (from
     * Pre-Alpha Demo code analysis).
     */
    ushort PersonWeaponTurn; // pos=106
    /** Rotation matrix index for a Vehicle.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis). Also
     * confirmed in fmtver=8-11 (from comparative analysis of binary data
     * in level files).
     */
    ushort VehicleMatrixIndex; // pos=108
    /* Confirmed to be Person `Brightness` since fmtver=4 (from Pre-Alpha
     * Demo code analysis).
     */
    ubyte PersonBrightness; // pos=110
    /* Confirmed to be Person `PathOffset` since fmtver=4 (from
     * Pre-Alpha Demo code analysis).
     */
    ubyte PathOffset;
    /* Since fmtver=4, Person sprite frame of some kind (from Pre-Alpha
     * Demo code analysis).
     */
    ushort PersonFrameUnkn112; // pos=112
    /** Index of the path a person is walking.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis).
     */
    ushort PersonPathIndex; // pos=114
    /* Person `MaxShieldEnergy` recognized in fmtver=8-11 (from comparative
     * analysis of binary data in level files).
     */
    short PersonMaxShieldEnergy;
    short TngUnkn118;
    long  TngUnkn120;
    short TngUnkn124; // pos=124
    ushort TngUnkn126;
    union { // pos=128
        ushort PersonUniqueID;
        ushort MGunUniqueID;
    };
    ushort TngUnkn130; // pos=130
    short PersonShieldEnergy; // pos=132
    char PersonSpecialTimer;
    ubyte TngUnkn135;
    short TngUnkn136;
    short TngUnkn138; // pos=138
    ubyte PersonBumpMode;
    ubyte PersonBumpCount;
    short PersonVehicle;
    short PersonLinkPassenger;
    /* Since fmtver=4, `TargetDX` for Object type GATE; Person sets it
     * as well, though purpose is uncertain (from Pre-Alpha Demo code
     * analysis). Assumed to be `Within` for Person, no proof.
     */
    union { // pos=146
        short ObjectTargetDX;
        ushort PersonWithin;
    };
    /* Since fmtver=4, `TargetDY` for Object type GATE (from Pre-Alpha Demo
     * code analysis). Assumed to be `LastDist` for Person, no proof.
     */
    union { // pos=148
        short ObjectTargetDY;
        ushort PersonLastDist;
    };
    /* Since fmtver=4, `TargetDZ` for Object type GATE (from Pre-Alpha Demo
     * code analysis).
     */
    union { // pos=150
        short ObjectTargetDZ;
        short TngUnkn150;
    };
    /** Pointer to a Thing being targeted by this one; cleared during load.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis).
     */
    ulong PTarget; // pos=152
    short TngUnkn156; // pos=156
    /* Assumed to be `OnFace` for Person, no proof.
     */
    short PersonOnFace;
    ushort PersonUnkn160; // pos=160
    ubyte SubState;
    ubyte PersonComRange;
    /* Vehicle `ReqdSpeed`, confirmed since fmtver=4 (from Pre-Alpha Demo
     * code analysis). In that version there was no MaxSpeed, so this counted
     * for both.
     */
    ushort VehicleReqdSpeed; // pos=164
    /* Is Vehicle `UniqueID`, Person `WeaponTimer`, and also used in Objects
     * since fmtver=4 (from Pre-Alpha Demo code analysis).
     */
    union { // pos=166
        ushort VehicleUniqueID;
        short PersonWeaponTimer;
    };
    /* Vehicle `PassengerHead`, confirmed since fmtver=4 (from Pre-Alpha Demo
     * code analysis).
     */
    short VehiclePassengerHead; // pos=168
    /** Traffic node where the vehicle is moving.
     * Vehicle `TNode`, confirmed since fmtver=4 (from Pre-Alpha Demo code
     * analysis). Also confirmed in fmtver=8-11 (from comparative analysis
     * of binary data in level files).
     */
    short VehicleTNode;
    /* Vehicle `AngleDY`, confirmed since fmtver=4 (from Pre-Alpha Demo code
     * analysis).
     */
    short VehicleAngleDY; // pos=172
    ushort TngUnkn174;
    /* Person weapon `MaxEnergy`, confirmed in fmtver=8-11 (from comparative analysis
     * of binary data in level files).
     */
    ushort PersonMaxEnergy; // pos=176
    /* Person weapon `Energy`, confirmed since fmtver=4 (from Pre-Alpha Demo
     * code analysis).
     */
    ushort PersonEnergy;
    ushort TngUnkn180; // pos=180
    ushort TngUnkn182;
    ubyte TngUnkn184; // pos=184
    /** Used by lights processing routine.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis).
     */
    ubyte PersonShadows[4]; // pos=185
    /* `RecoilTimer` at the same position for People and Vehicle.
     * Confirmed since fmtver=4 (from Pre-Alpha Demo code analysis).
     */
    union { // pos=189
        ubyte VehicleRecoilTimer;
        ubyte PersonRecoilTimer;
    };
    ushort PersonMaxHealth; // pos=190
    ubyte PersonRecoilDir; // pos=192
    ubyte TngUnkn193; // pos=193
    short GotoThingIndex; // pos=194
    short TngUnkn196;
    short TngUnkn198;
    short TngUnkn200;
    short TngUnkn202;
    /* Contains vehicle data in fmtver=8-11, specifics unknown
     * (from comparative analysis of binary data in level files).
     */
    short TngUnkn204;
    /* In fmtver=8-11, these map to values in the third and fourth bytes
     * of the "Dummy" vehicle value of the final level structure; vehicle
     * stats, similar to VehiclePassengerHead, specifics unknown
     * (from comparative analysis of binary data in level files).
     */
    short TngUnkn206;
    short TngUnkn208;
    short TngUnkn210;
    short TngUnkn212; // pos=212
    short TngUnkn214;
};

#pragma pack()
/******************************************************************************/
extern struct Thing *things;
extern ThingIdx things_used_head;
extern ThingIdx things_empty_head;
extern ushort things_used;
extern ThingIdx same_type_head[256+32];
extern short static_radii[];

extern struct SimpleThing *sthings;
extern short sthings_used_head;
extern ushort sthings_used;

extern TbBool debug_hud_things;
extern ubyte debug_log_things;

struct Thing *get_thing_safe(ThingIdx thing, ubyte ttype);

void init_things(void);
void refresh_old_thing_format(struct Thing *p_thing, struct ThingOldV9 *p_oldthing, u32 fmtver);
void process_things(void);

/** Get a string up to 14 chars containing thing/sthing type name.
 */
const char *thing_type_name(ubyte tngtype, ubyte subtype);

/** Fill buffer with function-like declaration of thing properties.
 */
void snprint_thing(char *buf, ulong buflen, struct Thing *p_thing);

/** Fill buffer with function-like declaration of simple thing properties.
 */
void snprint_sthing(char *buf, ulong buflen, struct SimpleThing *p_sthing);

/** Returns if given type represents SimpleThing rather than a full featured Thing.
 */
TbBool thing_type_is_simple(short ttype);

/** Given thing index, sets its position in map coordinates to three variables.
 *
 * Different kinds of things have different quirks in regard to position on map.
 * This function deals with all that and just gives the straight, simple position.
 */
void get_thing_position_mapcoords(short *x, short *y, short *z, ThingIdx thing);

/** Computes distance between two things.
 *
 * Uses precise algorithm for vector length. Returns value in map coords.
 */
u32 get_things_distance_mapcoords_precise(ThingIdx tng1, ThingIdx tng2);

/** Computes distance between two things.
 *
 * Uses some simplifications, but is fast. Returns value in map coords.
 */
u32 get_things_distance_mapcoords_fast(ThingIdx tng1, ThingIdx tng2);

/** Get a string representing text name of a state change result.
 */
const char *state_change_result_name(StateChRes res);

TbBool person_command_to_text(char *out, ushort cmd, ubyte a3);

void things_debug_hud(void);
void navi_onscreen_debug(TbBool a1);

TbBool thing_is_destroyed(ThingIdx thing);

struct Thing *effective_owner_of_thing(struct Thing *p_thing);

/** Set a thing frame with single-angle sprite.
 * Can be used for all sprite things except people.
 */
void set_thing_frame(struct Thing *p_thing, ushort anim_start);

/** Reset a thing frame with single-angle sprite.
 * Can be used for all sprite things except people.
 */
void reset_thing_frame(struct Thing *p_thing);

/** Reset a simple thing frame with single-angle sprite.
 */
void reset_sthing_frame(struct SimpleThing *p_sthing);

ubyte on_mapwho(struct Thing *p_thing);

/** Delete the thing from `mapwho` chain.
 */
TbResult delete_node(struct Thing *p_thing);

/** Delete the simple thing from `mapwho` chain.
 */
TbResult delete_snode(struct SimpleThing *p_sthing);

void add_node_thing(ThingIdx new_thing);
short get_new_thing(void);
void remove_thing(short tngno);

void add_node_sthing(ThingIdx new_thing);
short get_new_sthing(void);
void remove_sthing(short tngno);

void move_mapwho(struct Thing *p_thing, int x, int y, int z);

short add_static(int x, int y, int z, ushort frame, int timer);

void new_thing_traffic_clone(struct SimpleThing *p_clsthing);
short new_thing_smoke_gen_clone(struct SimpleThing *p_clsthing);
short new_thing_static_clone(struct SimpleThing *p_clsthing);
short new_thing_building_clone(struct Thing *p_clthing,
  struct M33 *p_clmat, short shut_h);

void build_same_type_headers(void);
short get_thing_same_type_head(short ttype, short subtype);

/** Tells whether center of given thing is located on map within given circle.
 *
 * For this function to return true, the thing needs to be within given circle
 * to at least half its depth - the center needs to be within.
 */
TbBool thing_is_within_circle(ThingIdx thing, short X, short Z, ushort R);

/** Tells whether some part of given thing is located on map within given circle.
 *
 * For this function to return true, it is enough that the thing intersects
 * the circle only a little. Even a small intersection is enough.
 */
TbBool thing_intersects_circle(ThingIdx thing, short X, short Z, ushort R);

/** Tells whether some part of given thing is located on map within given cylinder.
 *
 * A version of thing_intersects_circle() which does 3D coordinates comparison.
 * For this function to return true, it is enough that the thing intersects
 * the cylinder only a little. Even a small intersection is enough.
 */
TbBool thing_intersects_cylinder(ThingIdx thing, short X, short Y, short Z, ushort R, ushort H);

struct SimpleThing *create_scale_effect(int x, int y, int z, ushort frame, int timer);

struct SimpleThing *create_sound_effect(int x, int y, int z, ushort sample, int vol, int loop);

int mine_hit_by_bullet(struct Thing *p_thing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type);

int static_hit_by_bullet(struct SimpleThing *p_sthing, short hp,
  int vx, int vy, int vz, struct Thing *p_attacker, ushort type);

struct Thing *replace_thing_given_thing_idx(int x, int y, int z, short owner, ThingIdx pv_thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
