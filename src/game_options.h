/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_options.h
 *     Header file for game_options.c.
 * @par Purpose:
 *     Game options storage and manipulation.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 02 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include "bftypes.h"
#include "game_bstype.h"
#include "scanner.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define OPT_PANEL_PERMUT_ALPHA_MIN 0
#define OPT_PANEL_PERMUT_ALPHA_MAX 2

#define OPT_PANEL_PERMUT_MIN -3
#define OPT_PANEL_PERMUT_MAX -1

#define OPT_TRENCHCOAT_PREF_MIN 0
#define OPT_TRENCHCOAT_PREF_MAX 5

#define OPT_PAL_BRIGHTNESS_MIN -63
#define OPT_PAL_BRIGHTNESS_MAX 63

#define OPT_CD_TRACK_MIN 2
#define OPT_CD_TRACK_MAX 5

#define OPT_DANGER_TRACK_MIN 1
#define OPT_DANGER_TRACK_MAX 2

#define WEP_ROCKETS_FIRED_LIMIT 15

enum GameOptions {
    GOpt_None,
    // GFX options
    GOpt_ProjectorSpeed,
    GOpt_HighResolution,
    GOpt_DetailLevel,
    GOpt_CameraPerspective,
    GOpt_AdvancedLights,
    GOpt_BillboardMovies,
    GOpt_DeepRadar,
    GOpt_PanelPermutation,
    GOpt_TrenchcoatPreference,
    GOpt_PaletteBrightness,
    // Audio options
    GOpt_SampleVolume,
    GOpt_DangerVolume,
    GOpt_CDAVolume,
    GOpt_CDATrack,
    GOpt_DangerTrack,
    GOpt_UseMultiMedia,
    GOpt_ScannerPulse,
    GOpt_TranspObjSurfaceColr,
    GOpt_TranspObjLineColr,
};

enum DisplayModes {
  DpM_NONE = 0x0,
  DpM_UNKN_1 = 0x1,
  DpM_2 = 0x2,
  DpM_31 = 0x31,
  DpM_ENGINEPLY = 0x32,
  DpM_UNKN_33 = 0x33,
  DpM_34 = 0x34,
  DpM_36 = 0x36,
  DpM_PURPLEMNU = 0x37,
  DpM_38 = 0x38,
  DpM_39 = 0x39,
  DpM_UNKN_3A = 0x3A,
  DpM_UNKN_3B = 0x3B,
};

enum NetGamePlayFlags {
  NGPF_NONE = 0x0,
  NGPF_Unkn01 = 0x01,
  NGPF_Unkn02 = 0x02,
  NGPF_Unkn04 = 0x04,
  NGPF_Unkn08 = 0x08,
  NGPF_Unkn10 = 0x10,
  NGPF_Unkn20 = 0x20,
  NGPF_Unkn40 = 0x40,
  NGPF_Unkn60 = 0x80,
};

enum GameFlags {
    GamF_BillboardMovies = 0x0001,
    GamF_AdvLights    = 0x0002,
    GamF_Unkn0004     = 0x0004,
    GamF_Unkn0008     = 0x0008,
    GamF_MortalGame   = 0x0010,
    GamF_Unkn0020     = 0x0020,
    GamF_RenderScene  = 0x0040,
    GamF_StopThings   = 0x0080, /**< Do not process things, stopping ingame action. */
    GamF_Unkn0100     = 0x0100,
    GamF_Unkn0200     = 0x0200,
    GamF_DeepRadar    = 0x0400,
    GamF_Unkn0800     = 0x0800,
    GamF_Unkn1000     = 0x1000,
    GamF_HUDPanel     = 0x2000,
    GamF_Unkn4000     = 0x4000,
    GamF_ThermalView  = 0x8000,
    GamF_Unkn00010000 = 0x00010000,
    GamF_Unkn00020000 = 0x00020000,
    GamF_Unkn00040000 = 0x00040000,
    GamF_SkipIntro    = 0x00080000,
    GamF_Unkn00100000 = 0x00100000,
    GamF_NoScannerBeep = 0x00200000,
    GamF_BillboardBAT = 0x00400000,
    GamF_Unkn00800000 = 0x00800000,
    GamF_Unkn01000000 = 0x01000000,
    GamF_Unkn02000000 = 0x02000000,
    GamF_Unkn04000000 = 0x04000000,
    GamF_Unkn08000000 = 0x08000000,
    GamF_Unkn10000000 = 0x10000000,
};

struct InGame {
    short GameMode;
    short DisplayMode;
    short fld_unk7D8;
    ubyte fld_unk7DA;
    ubyte CDTrack;
    ubyte DangerTrack;
    ubyte UseMultiMedia;
    ubyte AutoResearch;
    ubyte GameOver;
    struct Scanner Scanner; // offset=0x0C
    long Credits; // offset=0x473 (on original Scanner struct)
    ulong fld_unkC4B;	// offset=0x477
    short MissionEndFade;
    short MissionStatus;
    long Flags;
    ushort fld_unkC57;
    short fld_unkC59;
    short draw_unknprop_01;
    short Rocket1[WEP_ROCKETS_FIRED_LIMIT];	// offset=0x489
    short NextRocket;
    short TrainMode;
    short MyGroup;
    short CurrentMission;
    ubyte fld_unk4AF;
    ubyte DetailLevel;
    short UserZoom;
    short cmdln_param_a;
    short LowerMemoryUse;
    short fld_unkC8B;
    short fld_unkC8D[2];
    long fld_unkC91;
    ushort TrackX;
    ushort TrackZ;
    short TrackThing;
    ubyte fld_unkC98[5];
    short fld_unkCA0;
    ubyte fld_unkCA2;
    sbyte PanelPermutation;
    ubyte TrenchcoatPreference;
    ubyte MissionNo;
    short fld_unkCA6;	// offset=0x4D2
    short fld_unkCA8;
    ubyte fld_unkCAA;
    ubyte PalType;
    short FlameCount;
    ubyte LastTmap;
    short SoundThing;
    ubyte fld_unkCB1;
    ubyte fld_unkCB2;
    short VisibleBillboardThing;
    short fld_unkCB5;
    short fld_unkCB7;
    /** Flags representing active human players, in both net and local games. */
    ubyte InNetGame_UNSURE;
    ubyte fld_unkCBA[5];
    ushort UserFlags;
    long CashAtStart;
    long Expenditure;
};

#pragma pack()
/******************************************************************************/
extern struct InGame ingame;
extern ubyte net_game_play_flags;

/** User set in-game brightess. */
extern short user_sel_brightness;

/******************************************************************************/

/** Gives national text with description of the action.
 */
const char *game_option_desc(int option_no);

/** Decrements value of the option, with wrapping.
 */
void game_option_dec(int option_no);

/** Increments value of the option, with wrapping.
 */
void game_option_inc(int option_no);

/** Shifts value of the option by given amount, without wrapping.
 */
void game_option_shift(int option_no, int amount);

/** Get value of game option; related variables can be accessed directly instead.
 */
short game_option_get(int option_no);

/** Set value of game option; skip any verification, set the raw value.
 */
void game_option_set(int option_no, int sval);

void set_default_gfx_settings(void);
void set_default_visual_prefernces(void);
void set_default_audio_tracks(void);

void apply_user_gfx_settings(void);
void apply_user_sfx_settings(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
