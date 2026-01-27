#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>

#include "bffile.h"
#include "bfflic.h"
#include "people.h"
#include "guiboxes.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

enum GameModes {
    GamM_None        = 0,
    GamM_Unkn1       = 1,
    GamM_Unkn2       = 2,
    GamM_Unkn3       = 3,
};

enum UserFlags {
    UsrF_WonBAT       =  0x0001,
    UsrF_Cheats       =  0x0004,
};

enum ScreenType {
  SCRT_NONE = 0x0,
  SCRT_MISSION,
  SCRT_PANET,
  SCRT_WORLDMAP,
  SCRT_CRYO,
  SCRT_EQUIP,
  SCRT_MAINMENU,
  SCRT_SYSMENU,
  SCRT_RESEARCH,
  SCRT_DEBRIEF,
  SCRT_LOGIN,
  SCRT_NETDEBRF,
  SCRT_ALERTBOX,
  SCRT_D,
  SCRT_E,
  SCRT_F,
  SCRT_10,
  SCRT_11,
  SCRT_12,
  SCRT_13,
  SCRT_14,
  SCRT_15, // 0x15
  SCRT_99 = 99,
};

enum LoginControlStates {
    LognCt_Unkn0 = 0,
    LognCt_Unkn1,
    LognCt_Unkn2,
    LognCt_Unkn3,
    LognCt_Unkn4,
    LognCt_Unkn5,
    LognCt_Unkn6,
    LognCt_Unkn7,
    LognCt_Unkn8,
    LognCt_Unkn9,
    LognCt_Unkn10,
};

enum AnimSlot {
  AniSl_FULLSCREEN = 0,
  AniSl_BILLBOARD = 1,
  AniSl_EQVIEW = 2,	/**< equipment (weapon or mod) presentation in buy/sell window */
  AniSl_CYBORG_INOUT = 3,	/**< cyborg mod insertion or removal anim */
  AniSl_UNKN4 = 4,
  AniSl_UNKN5 = 5,
  AniSl_UNKN6 = 6,
  AniSl_UNKN7 = 7,
  AniSl_CYBORG_BRTH = 8,
  AniSl_NETSCAN = 9,
  AniSl_SCRATCH = 10,	/**< scratch buffer for some transparent menu animations */
};

// For some reason, we have different values for change_screen
//TODO consolidate with ScreenType, use the same values
enum ChangeScreenType {
  ChSCRT_NONE = 0,
  ChSCRT_SYSMENU,
  ChSCRT_PANET,
  ChSCRT_WORLDMAP,
  ChSCRT_CRYO,
  ChSCRT_EQUIP,
  ChSCRT_RESEARCH,
  ChSCRT_MISBRIEF,
};

enum OpenMissionEndStatus {
  OMiSta_NONE,
  /** Mission completed successfully, and ended - no continuation exists */
  OMiSta_EndSuccess,
  /** Mission failed, and ended - no continuation allowed */
  OMiSta_EndFailed,
  /** Mission completed successfully, but is a part of unfinished multi-mission chain - requires continuation */
  OMiSta_ContSuccess,
  /** Mission failed, but non-failable - is left active and can be repeated later */
  OMiSta_ContFailed,
  /** Mission completed successfully, but is a part of multi-mission chain with immediately starting next mission */
  OMiSta_ContImmSuccess,
  /** Final mission completed successfully, campaign finished. */
  OMiSta_CampaignDone,
};

enum MissionFMVPlay {
    MPly_Intro,
    MPly_MissiComplete,
    MPly_MissiFail,
    MPly_MPartComplete,
    MPly_GameOver,
    MPly_Outro,
};

struct Thing;

struct ColColumn { // sizeof=16
    uint QBits[4];
};

struct BezierPt { // sizeof=28
    ubyte field_0[26];
    ushort field_2A;
};

struct LevelDef {
  ubyte PlayableGroups[8];
  ubyte field_8[33];
  ubyte field_29;
  ubyte field_2A;
  ubyte field_2B;
};

#pragma pack()

extern char session_name[20];
extern char user_name[50];
extern char unkn2_names[8][16];

extern ubyte in_network_game;
extern ubyte is_single_game;
extern ubyte cmdln_colour_tables;
extern ubyte cmdln_param_bcg;
extern ubyte keyboard_mode_direct;
extern ubyte unkn01_maskarr[28];
extern long map_editor;

extern ubyte login_control__State;
extern ulong login_control__Money;
extern sbyte login_control__City;
extern ubyte login_control__TechLevel;
extern ubyte byte_181183;
extern ubyte byte_181189;

extern ubyte cmdln_param_n;
extern ubyte exit_game;
extern ubyte input_char;

extern ulong active_flags_general_unkn01;

extern long unkn01_downcount;
extern long unkn01_pos_x;
extern long unkn01_pos_y;

extern ushort current_map;
extern short current_level;

extern void *engine_mem_alloc_ptr;
extern u32 engine_mem_alloc_size;

extern long navi2_unkn_counter;
extern long navi2_unkn_counter_max;

extern ubyte anim_slots[];
extern struct Animation animations[2];

extern ubyte *scratch_buf1;

extern ushort game_level_unique_id;
extern ubyte game_level_unkn1[40];
extern ubyte game_level_unkn2[40];

extern ushort word_1531E0;

extern struct ColColumn *game_col_columns;
extern ushort next_col_column;
extern struct SingleObjectFace3 *game_special_object_faces;
extern struct SingleObjectFace4 *game_special_object_faces4;
extern ubyte *game_user_heap;
extern struct UnknBezEdit *bezier_pts;
extern ushort next_bezier_pt;
extern ubyte *spare_map_buffer;
extern long target_old_frameno;
extern ushort word_176E38;

extern ubyte net_service_started;
extern ubyte linear_vec_pal[256];
extern ulong nsta_size;

extern short *dword_1C529C[6];
extern short *landmap_2B4;

extern ubyte *memload;
#define memload_len 16384

extern ubyte byte_1C83D1;
extern ubyte net_host_player_no;
extern ubyte byte_1C6D4A;
extern ubyte byte_1C6DDC[5];
extern ushort word_1C8446;
extern ushort unkn3de_len;

extern ushort weapon_text_index[32];
extern ushort cybmod_text_index[16];
extern ubyte background_type;
extern ubyte old_screentype;
extern ubyte screentype;
extern long data_155704;
extern ubyte data_1c498d;
extern ubyte mouse_sprite_anim_frame;
extern char *outro_text_s;
extern char *outro_text_z;
extern long data_197150;
extern long data_1dd91c;
extern ubyte unkn_flags_01;
extern long outro_credits_enabled;
extern long outro_unkn02;
extern long outro_unkn03;
extern long people_groups_count;
extern long data_1ddb68;
extern ubyte byte_1DDC40;

extern ubyte debug_hud_collision;

extern void *dword_177750;
extern void *unkn_mech_arr7;

extern long mech_unkn_dw_1DC880;
extern long mech_unkn_dw_1DC884;
extern long mech_unkn_dw_1DC888;
extern long mech_unkn_dw_1DC88C;
extern long mech_unkn_dw_1DC890;
extern long mech_unkn_dw_1DC894;

extern char *people_credits_desc[];
extern char *people_credits_groups[];

extern char *mission_briefing_text;
#define mission_briefing_text_len 16384

extern sbyte mission_result;

extern char *weapon_text;
#define weapon_text_len 32768

extern ubyte scientists_lost;
extern ulong new_mods_researched;
extern ulong new_weapons_researched;
extern ubyte redraw_screen_flag;

extern sbyte byte_15516C;
extern sbyte byte_15516D;
extern ubyte byte_1C5C28[8];
extern long dword_153194;
extern ulong starting_cash_amounts[8];
extern sbyte unkn_city_no;
extern ubyte group_types[8];
extern ubyte byte_1C4AA3;
extern ubyte net_unkn_pos_02;
extern ubyte data_1c4aa2;
extern ubyte start_into_mission;
extern ubyte edit_flag;
extern ubyte change_screen;
extern ubyte restore_savegame;
extern ubyte current_drawing_mod;
extern ubyte mod_draw_states[4];
extern ubyte new_current_drawing_mod;
extern ubyte refresh_equip_list;
extern ubyte flic_mods[5];
extern ubyte old_flic_mods[5];
extern ubyte reload_background_flag;

extern struct LevelDef level_def;

/** Index of the local user who controls mouse. */
extern ubyte mouser;
extern long dword_1AA5C4;
extern long dword_1AA5C8;

extern ushort next_mission;
extern ushort replay_intro_timer;
extern ubyte show_alert;
extern sbyte mo_weapon;

extern sbyte selected_agent;

extern int mouse_map_x;
extern int mouse_map_y;
extern int mouse_map_z;

extern ubyte game_gfx_advanced_lights;
extern ubyte game_billboard_movies;
extern ubyte game_gfx_deep_radar;
extern ubyte byte_1C4A6F;

extern char net_unkn2_text[];
extern char brief_netscan_cost_text[];
extern char *misc_text[5];

extern ubyte game_system_screen;
extern ubyte byte_197160;

extern ubyte execute_commands;

// To be replaced by LbArcTanAngle()
short arctan(int dx, int dz);

bool game_initialise (void);
void game_handle_sdl_events (void);
void game_update (void);
int game_hacky_update(void);
void game_quit (void);

void read_conf_file(void);
TbBool game_setup(void);
void game_process(void);
void game_reset(void);
void host_reset(void);
void init_variables(void);
void init_agents(void);
void srm_reset_research(void);
void net_new_game_prepare(void);

/** Decode and draw next frame of the animation.
 */
int xdo_next_frame(ubyte anislot);

/** Decode and draw previous frame of the animation.
 *
 * Note that printing a previous frame of the FLI file requires
 * decoding all frames from start - these files do not use
 * bi-directional FLIC format.
 */
int xdo_prev_frame(ubyte anislot);

void flic_unkn03(ubyte a1);

void my_preprocess_text(char *text);

TbBool player_try_spend_money(long cost);
void campaign_new_game_prepare(void);

void process_sound_heap(void);
void update_danger_music(ubyte a1);

void check_mouse_overvehicle(struct Thing *p_thing, ubyte target_assign);
int mech_unkn_func_03(struct Thing *p_thing);

void draw_new_panel(void);

void update_map_thing_and_traffic_refs(void);
void unkn_lights_processing(void);
void bang_set_detail(int a1);
int sub_73C64(char *a1, ubyte a2);
void func_6fd1c(int a1, int a2, int a3, int a4, int a5, int a6, ubyte a7);
void show_goto_point(uint flag);

void ingame_palette_reload(void);
void game_set_cam_track_thing_xz(ThingIdx thing);
TbBool game_cam_tracked_thing_is_player_agent(void);

ubyte process_send_person(ushort player, int i);

ubyte *anim_type_get_output_buffer(ubyte anislot);

short test_missions(ubyte flag);
void init_level_3d(ubyte flag);
void restart_back_into_mission(ushort missi);

#ifdef __cplusplus
};
#endif

#endif
