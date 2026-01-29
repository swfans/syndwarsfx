/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game.c
 *     Game loop and high level functions.
 * @par Purpose:
 *     Implement functions for high level flows of the game application.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 22 Oct 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bfconfig.h"
#include "bfcircle.h"
#include "bfdata.h"
#include "bfendian.h"
#include "bfsprite.h"
#include "bfscreen.h"
#include "bfkeybd.h"
#include "bfmouse.h"
#include "bftext.h"
#include "bfbox.h"
#include "bfpalette.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfstrut.h"
#include "bffile.h"
#include "bffont.h"
#include "bfgentab.h"
#include "bfwindows.h"
#include "bfpng.h"
#include "bfutility.h"
#include "bfsvaribl.h"
#include "bfmath.h"
#include "bfline.h"
#include "bfaudio.h"
#include "bfmusic.h"
#include "bfsound.h"
#include "bfdanger.h"
#include "bfplanar.h"
#include "bfscrsurf.h"
#include "bfscrcopy.h"
#include "bfstrut.h"
#include "bfsmack.h"
#include "bftringl.h"
#include "bfscd.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "engincam.h"
#include "engincolour.h"
#include "enginprops.h"
#include "engintxtrmap.h"

#include "linksmk.h"
#include "bat.h"
#include "bmbang.h"
#include "svesa.h"
#include "swlog.h"
#include "bflib_vidraw.h"
#include "bfjoyst.h"
#include "ssampply.h"
#include "matrix.h"
#include "dos.h"
#include "drawtext.h"
#include "enginbckt.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "engindrwlstx_tng.h"
#include "enginfexpl.h"
#include "enginfloor.h"
#include "enginlights.h"
#include "enginpriobjs.h"
#include "enginpritxtr.h"
#include "enginsngobjs.h"
#include "enginsngtxtr.h"
#include "enginpeff.h"
#include "enginshadws.h"
#include "engintext.h"
#include "engintrns.h"
#include "enginzoom.h"
#include "game_data.h"
#include "game_options.h"
#include "game_save.h"
#include "game_sprts.h"
#include "guiboxes.h"
#include "guitext.h"
#include "feappbar.h"
#include "femail.h"
#include "femain.h"
#include "febrief.h"
#include "fedebrief.h"
#include "fecntrls.h"
#include "fecryo.h"
#include "feequip.h"
#include "felogin.h"
#include "fenet.h"
#include "feoptions.h"
#include "fepause.h"
#include "fepanet.h"
#include "feresearch.h"
#include "feshared.h"
#include "festorage.h"
#include "feworld.h"
#include "hud_panecfg.h"
#include "hud_panel.h"
#include "purpldrw.h"
#include "purpldrwlst.h"
#include "building.h"
#include "campaign.h"
#include "cybmod.h"
#include "packet.h"
#include "pathtrig.h"
#include "pepgroup.h"
#include "lvdraw3d.h"
#include "lvobjctv.h"
#include "lvfiles.h"
#include "bigmap.h"
#include "display.h"
#include "dos.h"
#include "frame_sprani.h"
#include "game.h"
#include "game_data.h"
#include "game_speed.h"
#include "hud_target.h"
#include "keyboard.h"
#include "misstat.h"
#include "mouse.h"
#include "mydraw.h"
#include "network.h"
#include "sound.h"
#include "osunix.h"
#include "oswindws.h"
#include "util.h"
#include "command.h"
#include "packetfe.h"
#include "player.h"
#include "plyr_usrinp.h"
#include "plyr_packet.h"
#include "research.h"
#include "rules.h"
#include "scandraw.h"
#include "thing.h"
#include "thing_search.h"
#include "thing_onface.h"
#include "tngcolisn.h"
#include "tngobjdrw.h"
#include "vehicle.h"
#include "wadfile.h"
#include "weapon.h"
#include "wrcities.h"

/** Expected sizes for font DAT/TAB files for resolution 320x200.
 * Each file has 205 sprites, TAB has 6 bytes per entry, DAT varies to use empirical value.
 */
#define EXPECTED_FONT_DAT_TAB_SIZE_8 (205 * 6 + 205 * 34)

/** Expected sizes for translation strings TXT files.
 */
#define EXPECTED_LANG_TXT_SIZE 8000

/** How many game turns should pass before intro is replayed.
 */
#define INTRO_REPLAY_TURNS 1100

enum PostRenderAction {
    PRend_NONE = 0,
    PRend_SaveScreenshot,
};

enum ReloadMenuFlags {
    RelMnuF_ColorsSprites = 0x01,
    RelMnuF_BriefScanner = 0x02,
    RelMnuF_ScrBoxesFull = 0x04,
};


extern char *fadedat_fname;
char session_name[20] = "SWARA";

char user_name[50] = "TEMP";

long dword_153194 = 0x100;

ushort word_1531E0 = 1;

ushort next_mission = 1;

ulong stored_l3d_next_object;
ulong stored_l3d_next_object_face;
ulong stored_l3d_next_object_face4;
ulong stored_l3d_next_object_point;
ulong stored_l3d_next_normal;
ulong stored_l3d_next_face_texture;
ulong stored_l3d_next_floor_texture;
ulong stored_l3d_next_local_mat;
ulong stored_level3d_inuse;

extern int data_1c8428;
const char *primvehobj_fname = "qdata/primveh.obj";

extern short word_1C6E08;
extern short word_1C6E0A;

extern long dword_1DDECC;

u32 engine_mem_alloc_size = 5900000;

extern struct GamePanel unknstrct7_arr2[];

extern long gamep_unknval_10;
extern long gamep_unknval_11;
extern long gamep_unknval_12;
extern long nav_stats__ThisTurn;
extern long gamep_unknval_14;
extern long gamep_unknval_15;
extern long gamep_unknval_16;

extern long dword_155010;
extern long dword_155014;
extern long dword_155018;

ubyte simulated_mouse_click;
ubyte clear_left_button;
ubyte clear_right_button;
ubyte clear_wheel_up;
ubyte clear_wheel_down;
ubyte post_render_action;

int mouse_map_x = 0x3200;
int mouse_map_y = 0;
int mouse_map_z = 0x3200;

extern short last_map_for_lights_func_11;

extern short word_1552F8;

extern long dword_176CBC;

extern char unknmsg_str[100];
extern short word_1774E8[2 * 150];

char *data_15319c = unknmsg_str;

extern ubyte billboard_anim_no;
extern ubyte byte_1AAA88;
extern long dword_1AAB74;
extern long dword_1AAB78;
extern ubyte active_anim;
extern ushort word_1AABD0;

extern long mech_unkn_tile_x1;
extern long mech_unkn_tile_y1;
extern long mech_unkn_tile_x2;
extern long mech_unkn_tile_y2;
extern long mech_unkn_tile_x3;
extern long mech_unkn_tile_y3;

//TODO this is not an extern only because I was unable to locate it in asm
ushort next_bezier_pt = 1;

const char *miss_end_sta_names[] = {
  "undecided state",
  "ending success",
  "ending fail",
  "non-ending part success",
  "non-ending part fail",
  "imm. next after success",
  "",
};

/** Miscellaneous text strings.
 *
 * While initialized by constants, it cannot be const array
 * due to where the items are used.
 */
char *misc_text[] = {
  "0",
  "C",
  "/",
  "NC",
  "REF:",
};

/** List of sprites, palettes and animations required to be loaded for the game engine.
 *
 * For files with several variants, it is ought to load the largest - the other
 * files will be stored in the same buffer upon switch, so the buffer allocated
 * here must fit all the variants.
 */
struct TbLoadFiles missionspr_load_files[] =
{
  { "*VESA",			(void **)&lbVesaData,		(void **)NULL,LB_VESA_DATA_SIZE, 1, 0 },
#if 1 // !defined(LB_WSCREEN_CONTROL)
  { "*W_SCREEN",		(void **)&lbDisplay.WScreen,(void **)NULL,MAX_SUPPORTED_SCREEN_WIDTH*(MAX_SUPPORTED_SCREEN_HEIGHT+1), 0, 0 },
#endif
  { "data/pop2-3.dat",	(void **)&pop1_data,		(void **)&pop1_data_end,	0, 0, 0 },
  { "data/pop2-3.tab",	(void **)&pop1_sprites,		(void **)&pop1_sprites_end, 0, 0, 0 },
  { "data/mspr-0.dat",	(void **)&m_sprites_data,	(void **)&m_sprites_data_end,0, 0, 0 },
  { "data/mspr-0.tab",	(void **)&m_sprites,		(void **)&m_sprites_end,	0, 0, 0 },
  { "data/mele-0.ani",	(void **)&melement_ani,		(void **)&mele_ani_end,		0, 0, 0 },
  { "data/nsta-0.ani",	(void **)&nstart_ani,		(void **)&nstart_ani_end,	0, 0, 0 },
  { "data/nfra-0.ani",	(void **)&frame,			(void **)&frame_end,		0, 0, 0 },
  { "data/fontc0-7.dat",(void **)&small_font_data,	(void **)&small_font_data_end,0, 0, 0 },
  { "data/fontc0-7.tab",(void **)&small_font,		(void **)&small_font_end,	0, 0, 0 },
  { "data/pointr0-3.dat",(void **)&pointer_data,	(void **)&pointer_data_end,	0, 0, 0 },
  { "data/pointr0-3.tab",(void **)&pointer_sprites,	(void **)&pointer_sprites_end, 0, 0, 0 },
  { "qdata/pal.pal",	(void **)&display_palette,	(void **)NULL,				0, 0, 0 },
  { "",					(void **)NULL, 				(void **)NULL,				0, 0, 0 }
};

ubyte byte_154BB4[] = {
  220, 224, 224, 222, 220, 220,
};

char unk_credits_text_s[] = "";
char unk_credits_text_z[] = "";
char unk_credits_text_p[] = "";

ubyte reload_menu_flags = 0;

void ac_purple_unkn1_data_to_screen(void);

short arctan(int dx, int dz)
{
    ushort ret;
    asm volatile ("call ASM_arctan\n"
        : "=r" (ret) : "a" (dx), "d" (dz));
    return ret;
}

void debug_trace_place(int place)
{
    LOGDBG("reached place %d", place);
}

bool game_initialise(void)
{
    TbResult ret;
    ret = LbBaseInitialise();
    if (ret != Lb_SUCCESS)
    {
        LOGERR("Bullfrog Library initialization failed");
        return false;
    }

#ifdef __unix__
    /* clean up after SDL messing around where it shouldn't */
    unix_restore_signal_handlers();
#endif

    LbSetUserResourceMapping(SWResourceMapping);
    LbSetTitle(PACKAGE_NAME);
    LbSetIcon(1);

    // Make sure file names are properly converted before opening
    setup_file_names();

    return true;
}

void game_handle_sdl_events(void)
{
    TbBool contn;
    contn = LbWindowsControl();
    if (!contn) {
        game_quit();
    }
}

int existing_texturemaps(void)
{
    char locstr[DISKPATH_SIZE];
    int i;

    for (i = 0; ; i++)
    {
        sprintf(locstr, "data/tex%02d.dat", i);
        if (!LbFileExists(locstr))
          break;
    }
    return i;
}

int reload_texturemaps(void)
{
    char locstr[DISKPATH_SIZE];
    int i;

    for (i = 0; vec_tmap[i] != NULL; i++)
    {
        sprintf(locstr, "data/tex%02d.dat", i);
        LbFileLoadAt(locstr, vec_tmap[i]);
    }
    return i;
}

void load_texturemaps(void)
{
#if 0
    asm volatile ("call ASM_load_texturemaps\n"
        :  :  : "eax" );
#endif
    if (vec_tmap[0] == NULL)
    {
        int tmap_count;
        tmap_count = existing_texturemaps();
        if (!alloc_texturemaps(tmap_count)) {
            exit_game = 1;
            return;
        }
        ingame.LastTmap = tmap_count - 1;
    }
    reload_texturemaps();
}

void test_open(int num)
{
    // Empty for production version
}

void load_prim_quad(void)
{
    unkn2_pos_x = 64;
    unkn2_pos_y = 64;
    unkn2_pos_z = 64;
    data_1c8428 = 0;
    prim_unknprop01 = 1000;
    read_primveh_obj(primvehobj_fname, 1);
    read_textwalk();
    engine_render_lights = 1;
    ingame.DisplayMode = DpM_PURPLEMNU;
    if (cmdln_param_bcg == 99)
        test_open(99);
    if (cmdln_param_bcg == 100)
        test_open(100);
}

void bang_init(void)
{
    asm volatile ("call ASM_bang_init\n"
        :  :  : "eax" );
}

void bang_set_detail(int a1)
{
    asm volatile ("call ASM_bang_set_detail\n"
        : : "a" (a1));
}

void FIRE_init_or_samples_init(void)
{
    asm volatile ("call ASM_FIRE_init_or_samples_init\n"
        :  :  : "eax" );
}

/** Remains of some Bf debug stuff.
 */
void debug_trace_setup(int place)
{
    LOGDBG("reached place %d", place);
}

/** Remains of some Bf debug stuff.
 */
void debug_trace_turn_bound(ulong turn)
{
    LOGDBG("turn %lu", turn);
}

/** NOOP func which compiler merged from several functions.
 */
void merged_noop_unkn1(int a1)
{
}

void player_debug(const char *text)
{
    // TODO place debug/verification code
}

void colour_tables_ghost_fixup(void)
{
    TbPixel *ipal;
    TbPixel *opal;
    ushort i;

    ipal = &pixmap.fade_table[63*PALETTE_8b_COLORS];
    opal = &pixmap.ghost_table[0*PALETTE_8b_COLORS];
    for (i = 0; i < PALETTE_8b_COLORS; i++)
        opal[i] = ipal[i];
}

TbBool game_setup_stuff(void)
{
    TbFileHandle fh;
    ushort i;
    TbBool ret;

    ret = true;
    for (i = 0; i < PALETTE_8b_COLORS; i++)
        linear_vec_pal[i] = i;
    vec_pal = linear_vec_pal;

    fh = LbFileOpen("data/nsta-0.ani", Lb_FILE_MODE_READ_ONLY);
    if (fh != INVALID_FILE) {
        nsta_size = LbFileSeek(fh, 0, Lb_FILE_SEEK_END);
        LbFileClose(fh);
    } else {
        ret = false;
    }

    if (display_palette != NULL) {
        colour_brown2 = LbPaletteFindColour(display_palette, 42, 37, 30);
        colour_grey2 = LbPaletteFindColour(display_palette, 32, 32, 32);
        colour_grey1 = LbPaletteFindColour(display_palette, 16, 16, 16);
    } else {
        colour_brown2 = 60;
        colour_grey2 = 61;
        colour_grey1 = 62;
        LOGERR("Display palette not set, using dummies");
        ret = false;
    }

    return ret;
}

void anim_show_FLI_SS2_NP(void)
{
    struct Animation *p_anim;

    p_anim = &animations[active_anim];
    anim_show_FLI_SS2(p_anim);
}

void anim_show_FLI_BRUN_NP(void)
{
    struct Animation *p_anim;

    p_anim = &animations[active_anim];
    anim_show_FLI_BRUN(p_anim);
}

void anim_show_FLI_LC_NP(void)
{
    struct Animation *p_anim;

    p_anim = &animations[active_anim];
    anim_show_FLI_LC(p_anim);
}

ubyte *anim_type_get_output_buffer(ubyte anislot)
{
    switch (anislot)
    {
    case AniSl_FULLSCREEN:
    default:
        return lbDisplay.WScreen;
    case AniSl_BILLBOARD:
        return vec_tmap[4];
    case AniSl_EQVIEW:
    case AniSl_UNKN4:
    case AniSl_UNKN5:
    case AniSl_UNKN6:
    case AniSl_UNKN7:
    case AniSl_NETSCAN:
        return vec_tmap[5];
    case AniSl_CYBORG_INOUT:
    case AniSl_CYBORG_BRTH:
        return vec_tmap[5] + 0x8000;
    case AniSl_SCRATCH:
        return vec_tmap[4] + 0x8000;
    }
}

void anim_billboard_select_rand(void)
{
    ushort rnd;

    rnd = LbRandomPosShort() & 7;
    if (rnd <= 0)
        billboard_anim_no = 1;
    else if (rnd <= 2)
        billboard_anim_no = 2;
    else if (rnd <= 5)
        billboard_anim_no = 0;
    else
        billboard_anim_no = 3;
}

void anim_billboard_select_next(void)
{
    billboard_anim_no++;
    if (billboard_anim_no > 3)
        billboard_anim_no = 0;
}

void anim_billboard_broadcast_sound(void)
{
    struct Thing *p_thing;
    ushort rnd;
    ubyte smpl_no;

    if (in_network_game)
        return;
    if (ingame.VisibleBillboardThing == 0)
        return;

    p_thing = &things[ingame.VisibleBillboardThing];
    smpl_no = byte_154BB4[billboard_anim_no];
    rnd = LbRandomPosShort() & 1;
    play_dist_sample(p_thing, smpl_no + rnd, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
}

void flic_unkn03(ubyte anislot)
{
    struct Animation *p_anim;
    ubyte *frmbuf;
    PathInfo *pinfo;
    int k;

    k = anim_slots[anislot];
    p_anim = &animations[k];
    if (anim_is_opened(p_anim)) {
        anim_flic_close(p_anim);
    }

    anim_scratch = scratch_buf1;
    anim_flic_init(p_anim, anislot, 0x00);
    frmbuf = anim_type_get_output_buffer(anislot);

    switch (anislot)
    {
    case AniSl_BILLBOARD:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 0, 0, 0, 0x20);
        anim_billboard_select_rand();
        anim_billboard_broadcast_sound();
        pinfo = &game_dirs[DirPlace_QData];
        anim_flic_set_fname(p_anim, "%s/%s-1%d.fli", pinfo->directory, "demo", (int)billboard_anim_no);
        anim_billboard_select_next();
        break;
    case AniSl_EQVIEW:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 0, 0, 0, 0x00);
        break;
    case AniSl_CYBORG_INOUT:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 0, 0, 0, 0x00);
        break;
    case AniSl_UNKN4:
        byte_1AAA88 = 1;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 0, 0, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "intro");
        break;
    case AniSl_UNKN5:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 10, 30, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "mcomp");
        break;
    case AniSl_UNKN6:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 10, 30, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "mcomp");
        break;
    case AniSl_UNKN7:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 10, 30, 0, 0x02);
        pinfo = &game_dirs[DirPlace_Data];
        anim_flic_set_fname(p_anim, "%s/%s.fli", pinfo->directory, "mcomp");
        break;
    case AniSl_CYBORG_BRTH:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 0, 0, 0, 0x20);
        break;
    case AniSl_NETSCAN:
        byte_1AAA88 = 0;
        anim_flic_set_frame_buffer(p_anim, frmbuf, 0, 0, 0, 0x00);
        break;
      default:
        break;
    }

    if (anim_flic_show_open(p_anim) == Lb_FAIL)
    {
        if (anislot == AniSl_BILLBOARD)
            ingame.Flags &= ~GamF_BillboardMovies;
        return;
    }
    p_anim->anfield_4 += 12;
}

void update_danger_music(ubyte a1)
{
    short hurt_agents;
    struct Thing *p_agent;

    switch (a1)
    {
    case 1: // force start danger music
        dword_1DDECC = 101;
        ingame.fld_unkC8B = 1;
        break;
    case 2: // stop danger music
        ingame.fld_unkC8B = 0;
        dword_1DDECC = 0;
        SetMusicVolume(3000, 0);
        break;
    case 0: // standard ingame check for the music
        dword_1DDECC++;
        break;
    }
    if (dword_1DDECC > 100)
    {
        hurt_agents = ingame.fld_unkC8B;
        if (hurt_agents == 0)
        {
            short i;
            for (i = 0; i < playable_agents; i++)
            {
                p_agent = players[local_player_no].MyAgent[i];
                if (p_agent->Type != TT_PERSON) continue;
                if (((p_agent->Flag & TngF_Destroyed) == 0) &&
                  (p_agent->Health < p_agent->U.UPerson.MaxHealth / 2)) {
                    dword_1DDECC = -100;
                    hurt_agents++;
                }
            }
        }
        if (ingame.fld_unk7DA == 1)
        {
            if (hurt_agents) {
                dword_1DDECC = 101;
            } else {
                SetMusicVolume(3000, 0);
                ingame.fld_unk7DA = 0;
            }
        }
        if (ingame.fld_unk7DA == 0)
        {
            if (hurt_agents) {
                SetMusicVolume(300, 127);
                dword_1DDECC = -40;
                ingame.fld_unk7DA = 1;
            } else {
                dword_1DDECC = 101;
            }
        }
    }
    ingame.fld_unkC8B = 0;
}

void sprint_fmv_filename(ushort vid_type, char *fnbuf, ulong buflen)
{
    const char *fname;
    PathInfo *pinfo;
    struct Campaign *p_campgn;

    fname = NULL;
    switch (vid_type)
    {
    case MPly_Intro:
        // Intro name is hard-coded, as it is played before any campaign is loaded
        fname = "intro.smk";
        pinfo = &game_dirs[DirPlace_LangData];
        if (game_dirs[DirPlace_Sound].use_cd == 1)
            sprintf(fnbuf, "%s%s/%s", cd_drive, pinfo->directory, fname);
        else
            sprintf(fnbuf, "intro/%s", fname);
        break;
    case MPly_MissiComplete:
        switch (background_type)
        {
        case 0:
            fname = "syn_mc.smk";
            sprintf(fnbuf, "qdata/%s", fname);
            break;
        case 1:
            fname = "zel-mc.smk";
            sprintf(fnbuf, "qdata/%s", fname);
            break;
        default:
            fnbuf[0] = '\0';
            break;
        }
        break;
    case MPly_MPartComplete:
        switch (background_type)
        {
        case 0:
            // TODO MAPNO case for a specific map, remove
            if (current_map == 46)
            {
                fname = "syn_ele.smk";
                pinfo = &game_dirs[DirPlace_LangData];
                if (game_dirs[DirPlace_Data].use_cd == 1)
                    sprintf(fnbuf, "%s%s/%s", cd_drive, pinfo->directory, fname);
                else
                    sprintf(fnbuf, "data/%s", fname);
            }
            else
            {
                fnbuf[0] = '\0';
            }
            break;
        case 1:
            // TODO MAPNO case for a specific map, remove
            if (current_map == 46)
            {
                fname = "chu_ele.smk";
                pinfo = &game_dirs[DirPlace_LangData];
                if (game_dirs[DirPlace_Data].use_cd == 1)
                    sprintf(fnbuf, "%s%s/%s", cd_drive, pinfo->directory, fname);
                else
                    sprintf(fnbuf, "data/%s", fname);
            }
            else
            {
                fnbuf[0] = '\0';
            }
            break;
        default:
            fnbuf[0] = '\0';
            break;
        }
        break;
    case MPly_MissiFail:
        switch (background_type)
        {
        case 0:
            fname = "syn_fail.smk";
            sprintf(fnbuf, "qdata/%s", fname);
            break;
        case 1:
            fname = "syn_fail.smk";
            sprintf(fnbuf, "qdata/%s", fname);
            break;
        default:
            fnbuf[0] = '\0';
            break;
        }
        break;
    case MPly_GameOver:
        switch (background_type)
        {
        case 0:
            fname = "syn_go.smk";
            sprintf(fnbuf, "qdata/%s", fname);
            break;
        case 1:
            fname = "zel-go.smk";
            sprintf(fnbuf, "qdata/%s", fname);
            break;
        default:
            fnbuf[0] = '\0';
            break;
        }
        break;
    case MPly_Outro:
        p_campgn = &campaigns[background_type];
        //TODO should support use_cd like intro
        sprintf(fnbuf, "%s", p_campgn->OutroFMV);
        break;
    default:
        fnbuf[0] = '\0';
        break;
    }
}

static void clear_smacker_skip_keys(void)
{
    clear_key_pressed(KC_SPACE);
    clear_key_pressed(KC_RETURN);
    clear_key_pressed(KC_ESCAPE);
}

void play_smacker(ushort vid_type)
{
    char fname[FILENAME_MAX];
    u32 plyflags;
    TbScreenMode scr_md_fmvid;
    TbBool prepare_draw_on_last_frame;

    update_danger_music(2);

    sprint_fmv_filename(vid_type, fname, sizeof(fname));

    // For outro, last frame of the video should stay on screen
    // and the screen should be prepared for further drawing
    prepare_draw_on_last_frame = (vid_type == MPly_Outro);

    if ((vid_type == MPly_Intro) || (vid_type == MPly_Outro)) {
        scr_md_fmvid = screen_mode_fmvid_lo;
    } else {
        scr_md_fmvid = screen_mode_fmvid_hi;
    }

    if (prepare_draw_on_last_frame) {
        // If we need to prepare drawing after, do a full mode change
        if (lbDisplay.ScreenMode != scr_md_fmvid)
            setup_screen_mode(scr_md_fmvid);
    } else {
        // Do simple mode change, we will switch again after the video
        // also skip the mode change entirely if no video to play
        if ((lbDisplay.ScreenMode != scr_md_fmvid) && (fname[0] != '\0'))
            setup_simple_screen_mode(scr_md_fmvid);
    }
    LbMouseChangeSprite(NULL);

    plyflags = 0;

    if (fname[0] != '\0') {
        show_black_screen();
        clear_smacker_skip_keys();
        play_smk(fname, 13, plyflags);
        smack_malloc_free_all();
    }

    if (prepare_draw_on_last_frame) {
            struct Campaign *p_campgn;
            short raw_w, raw_h;
            short x, y;
            raw_w = 300;
            raw_h = 150;
            x = (lbDisplay.GraphicsScreenWidth - raw_w) / 2;
            y = (lbDisplay.GraphicsScreenHeight - raw_h) / 2 - 1;

            p_campgn = &campaigns[background_type];
            LbScreenClear(0);
            cover_screen_rect_with_raw_file(x, y, raw_w, raw_h, p_campgn->OutroBkFn);

            ingame_palette_load(0);
            LbScreenWaitVbi();
            LbPaletteSet(display_palette);
            swap_wscreen();
    } else {
        show_black_screen();
    }
    game_hacky_update();
    clear_smacker_skip_keys();
}

void play_smacker_then_back_to_engine(ushort vid_type)
{
    TbScreenMode bkpmode;

    bkpmode = lbDisplay.ScreenMode;
    play_smacker(vid_type);
    setup_screen_mode(bkpmode);
}

void play_smacker_then_back_to_menu(ushort vid_type)
{
    stop_sample_using_heap(0, 122);
    stop_sample_using_heap(0, 122);

    play_smacker(vid_type);
    setup_screen_mode(screen_mode_menu);

    play_sample_using_heap(0, 122, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
    play_sample_using_heap(0, 122, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
}

void play_intro(void)
{
    LOGSYNC("Starting");
    if ( (cmdln_param_bcg || is_single_game) &&
      ((ingame.Flags & GamF_SkipIntro) == 0))
    {
        // After this function, we return either to engine or to menu,
        // depending on how the app was started
        if (cmdln_param_bcg)
            play_smacker_then_back_to_menu(MPly_Intro);
        else
            play_smacker_then_back_to_engine(MPly_Intro);
    }
}

char func_cc638(const char *text1, const char *text2)
{
    char ret;
    asm volatile ("call ASM_func_cc638\n"
        : "=r" (ret) : "a" (text1), "d" (text2));
    return ret;
}

void screen_dark_curtain_down(void)
{
    asm volatile ("call ASM_screen_dark_curtain_down\n"
        :  :  : "eax" );
}

int load_outro_text(ubyte *buf)
{
    int totlen;
    char *s;
    int i;

    totlen = load_file_alltext("outtro.txt", buf);
    if (totlen == -1) {
        buf[0] = '\0';
        return 0;
    }
    buf[totlen] = '\0';
    s = (char *)buf;
    // The font can handle lower case letters, upper case just looks better
    for (i = 0; i < totlen; i++) {
        s[i] = my_char_to_upper(s[i]);
    }

    s = (char *)buf;
    while ((*s != ']') && (*s != '\0')) {
        s++;
    }
    s++;
    outro_text_s = s;

    while ((*s != ']') && (*s != '\0')) {
        s++;
    }
    s++;
    outro_text_z = s;

    return totlen + 1;
}

int load_people_text(ubyte *buf)
{
    int totlen;
    int peptxt_len;
    int groupno;
    char *name;
    char *desc;
    char *s;
    int i;

    totlen = LbFileLoadAt("data/people.txt", buf);
    if (totlen == -1) {
        buf[0] = '\0';
        return 0;
    }
    peptxt_len = totlen;

    s = (char *)buf;
    for (i = 0; i < peptxt_len; i++)
    {
        if (*s == ']')
            *s = '\0';
        s++;
    }

    // List of people
    groupno = 0;
    s = (char *)buf;
    while ( 1 )
    {
        if (*s == '[')
        {
            s++;
            name = s;
            if (strcmp(name, "END") == 0) {
              s += 4;
              break;
            }
            people_credits_desc[2 * groupno + 0] = name;

            while (*s != '[')
            {
                if (*s == '#') {
                    groupno = strtol(s + 1, &desc, 10);
                    s = desc;
                }
                s++;
            }
            s++;
            desc = s;
            people_credits_desc[2 * groupno + 1] = desc;
            continue;
        }
        if (*s == '#') {
            groupno = strtol(s + 1, &desc, 10);
            s = desc;
        }
        s++;
    }

    // Assignment of people to groups
    while ( 1 )
    {
        if (*s == '[')
        {
            ubyte *g;

            s++;
            if (strcmp(s, "END") == 0) {
                s += 4;
                break;
            }
            people_credits_groups[2 * groupno + 0] = s;

            while (*s != '[')
            {
                if (*s == '#') {
                    groupno = strtol(s + 1, &desc, 10);
                    s = desc;
                }
                s++;
            }
            s++;
            g = &buf[totlen];
            people_credits_groups[2 * groupno + 1] = (char *)g; // TODO we should really use a struct here

            // Recognize the list of integers, store them in g
            while ( 1 )
            {
              *g = strtol(s, &desc, 10);
              if (desc > s) {
                  totlen++;
                  s = desc;
                  g++;
              }
              if (*s == '\0')
                  break;
              s++;
            }
            *g = 100; // end-of-list marker
            totlen++;
            continue;
        }
        if (*s == '#')
        {
            groupno = strtol(s + 1, &desc, 10);
            s = desc;
        }
        s++;
    }
    people_groups_count = groupno + 1;

    return totlen;
}

TbResult load_outro_sprites(void)
{
    PathInfo *pinfo;
    ubyte *p_buf;
    ubyte *outtxt_ptr;
    ubyte *peptxt_ptr;
    int next_pos, next_len;
    int total_allocs;
    TbResult tret, ret;
    short max_detail;

    max_detail = 0;//UNKN_sprites_scale / 2;
    tret = Lb_OK;

    pinfo = &game_dirs[DirPlace_Data];

    total_allocs = 3 * EXPECTED_FONT_DAT_TAB_SIZE_8 + 2 * EXPECTED_LANG_TXT_SIZE;
    next_pos = engine_mem_alloc_size - 64000 - total_allocs;

    assert(next_pos >= 0);
    p_buf = engine_mem_alloc_ptr + next_pos;

    ret = load_sprites_med_font(&p_buf, pinfo->directory, 7, max_detail);
    if (tret == Lb_OK)
        tret = ret;

    ret = load_sprites_big_font(&p_buf, pinfo->directory, 6, max_detail + 1);
    if (tret == Lb_OK)
        tret = ret;

    ret = load_sprites_med2_font(&p_buf, pinfo->directory, 12, max_detail);
    if (tret == Lb_OK)
        tret = ret;

    setup_sprites_med_font();
    setup_sprites_med2_font();
    setup_sprites_big_font();

    outtxt_ptr = p_buf;
    next_len = load_outro_text(outtxt_ptr);
    p_buf += next_len;

    peptxt_ptr = p_buf;
    next_len = load_people_text(peptxt_ptr);
    p_buf += next_len;

    assert((p_buf - ((ubyte *)engine_mem_alloc_ptr + next_pos)) <= total_allocs);
    if (tret == Lb_FAIL) {
        LOGERR("Some files were not loaded successfully");
    }
    return tret;
}

void fill_floor_textures(void)
{
    asm volatile ("call ASM_fill_floor_textures\n"
        :  :  : "eax" );
}

void fill_netgame_agent_pos(int plyr, int group, int num_agents)
{
#if 0
    asm volatile (
      "call ASM_fill_netgame_agent_pos\n"
        : : "a" (plyr), "d" (group), "b" (num_agents));
#endif
    ThingIdx thing;
    struct Thing *p_thing;
    short plagent;

    plagent = 0;
    for (thing = things_used_head; thing != 0; thing = p_thing->LinkChild)
    {
        p_thing = &things[thing];

        if (p_thing->Type != TT_PERSON)
            continue;
        if (p_thing->U.UPerson.Group != group)
            continue;

        netgame_agent_pos_x[plyr][plagent] = PRCCOORD_TO_MAPCOORD(p_thing->X);
        netgame_agent_pos_z[plyr][plagent] = PRCCOORD_TO_MAPCOORD(p_thing->Z);
        plagent++;
        if (plagent == num_agents)
            break;
    }
    if (plagent < num_agents) {
        LOGERR("Not enough agents discovered on level");
    }
}

/** Change flags of commands containing "until".
 * This seem to just block execution of commands. Maybe it was
 * used to one-time fix some specific levels?
 * Remove pending.
 */
void unkn_f_pressed_func(void)
{
    ThingIdx thing;
    short i;

    thing = get_thing_same_type_head(TT_PERSON, -1);
    for (i = 0; thing != 0; i++)
    {
        struct Thing *p_thing;
        ushort cmd;
        struct Command *p_cmd_prev;

        if (i >= THINGS_LIMIT) {
            LOGERR("Infinite loop in same type things list");
            break;
        }
        p_thing = &things[thing];
        cmd = p_thing->U.UPerson.ComHead;
        p_cmd_prev = NULL;
        while (cmd != 0)
        {
            struct Command *p_cmd;

            p_cmd = &game_commands[cmd];
            if (!is_command_any_until(p_cmd))
            {
                if (p_cmd_prev != NULL)
                    p_cmd_prev->Flags |= PCmdF_RunUntil;
                p_cmd->Flags |= PCmdF_IsUntil;
            }
            p_cmd_prev = p_cmd;
            cmd = p_cmd->Next;
        }
        thing = p_thing->LinkSame;
    }
}

void load_level_wrap(short level, short missi, ubyte reload)
{
    LbMouseChangeSprite(NULL);
    if (0) { // No need to conserve memory to such extent - mem_game[] was changed
        // Optimization for memory conservation - reserve no space for game_commands,
        // and instead re-use some of triangulation area during map load.
        void **p = mem_game[34].BufferPtr;
        *p = dword_177750;
        mem_game[34].N = 3100;
    }
    read_primveh_obj(primvehobj_fname, 1);

    load_level_pc(level, missi, reload);

    // No idea what exactly pressing F during level load does
    if (is_key_pressed(KC_F, KMod_DONTCARE))
        unkn_f_pressed_func();
}

int sub_73C64(char *a1, ubyte a2)
{
    char ret;
    asm volatile ("call ASM_sub_73C64\n"
        : "=r" (ret) : "a" (a1), "d" (a2));
    return ret;
}

void update_map_thing_and_traffic_refs(void)
{
    asm volatile ("call ASM_update_map_thing_and_traffic_refs\n"
        :  :  : "eax" );
}

void unkn_lights_processing(void)
{
    asm volatile ("call ASM_unkn_lights_processing\n"
        :  :  : "eax" );
}

void change_current_map(ushort mapno)
{
    LbMouseChangeSprite(NULL);
    current_map = mapno;
    init_things();
    load_mad_pc(mapno);
    fill_floor_textures();

    if (current_map == 11) // map011 Orbital Station
        render_floor_flags |= RendFlrF_NonPlanetary;
    else
        render_floor_flags &= ~RendFlrF_NonPlanetary;

    if (current_map == 9) // map009 Singapore on-water map
        render_floor_flags |= RendFlrF_WobblyTerrain;
    else
        render_floor_flags &= ~RendFlrF_WobblyTerrain;
}

void traffic_unkn_func_01(void)
{
    asm volatile ("call ASM_traffic_unkn_func_01\n"
        :  :  : "eax" );
}

/** Step overall scale towards given zoom value.
 *
 * Overal scale changes gradually towards given value; shis
 * function does a one step update of the overall_scale.
 */
void process_overall_scale(int zoom)
{
    int zdelta, sdelta;
    short scmin, scmax, scale;

    zoom = get_scaled_zoom(zoom);

    zdelta = zoom - overall_scale;
    if ((zdelta > 0) && (zdelta < 8))
        sdelta = 1;
    else if ((zdelta < 0) && (zdelta > -8))
        sdelta = -1;
    else
        sdelta = zdelta >> 3;

    scmin = get_overall_scale_min();
    scmax = get_overall_scale_max();

    scale = sdelta + overall_scale;
    if (scale < scmin)
        scale = scmin;
    else if (scale > scmax)
        scale = scmax;
    overall_scale = scale;
}

void process_view_inputs(int thing)
{
    struct Thing *p_person;
    int zoom;

    p_person = &things[thing];
    zoom = get_weapon_zoom_min(p_person->U.UPerson.CurrentWeapon);
    // User zoom is not scaled to resolution
    if (zoom >= ingame.UserZoom)
        ingame.UserZoom = zoom;
    else
        zoom = ingame.UserZoom;

    process_overall_scale(zoom);
}

void draw_hud_target_mouse(short dcthing)
{
    PlayerInfo *p_locplayer;
    struct Thing *p_dcthing;

    p_dcthing = &things[dcthing];
    p_locplayer = &players[local_player_no];
    if (p_locplayer->Target > 0)
    {
        struct Thing *p_targtng;
        int weprange;
        ushort msspr;
        uint range;

        weprange = current_hand_weapon_range(p_dcthing);
        switch (p_locplayer->TargetType)
        {
        case TrgTp_Unkn1:
        case TrgTp_Unkn2:
        case TrgTp_Unkn6:
        case TrgTp_Unkn7:
            p_locplayer->field_102 = p_locplayer->Target;
            p_locplayer->TargetType = TrgTp_Unkn7;
            p_targtng = &things[p_locplayer->Target];
            range = weprange * weprange;
            if (can_i_see_thing(p_dcthing, p_targtng, range, 3) ) {
                msspr = 3;
            } else {
                msspr = 2;
            }
            do_change_mouse(msspr);
            break;
        case TrgTp_Unkn3:
            p_locplayer->field_102 = p_locplayer->Target;
            do_change_mouse(7);
            break;
        case TrgTp_Unkn4:
            p_locplayer->field_102 = p_locplayer->Target;
            p_targtng = &things[p_locplayer->field_102];
            p_dcthing = &things[p_locplayer->DirectControl[mouser]];
            if (can_i_enter_vehicle(p_dcthing, p_targtng)) {
              msspr = 6;
            } else {
              range = p_targtng->Radius * p_targtng->Radius + weprange * weprange;
              if (can_i_see_thing(p_dcthing, p_targtng, range, 3) ) {
                msspr = 3;
              } else {
                msspr = 2;
              }
            }
            do_change_mouse(msspr);
            break;
        default:
            break;
        }
    }
    else if (p_locplayer->Target < 0)
    {
        if (p_locplayer->TargetType == TrgTp_Unkn3) {
          p_locplayer->field_102 = p_locplayer->Target;
          do_change_mouse(7);
        } else {
          p_locplayer->field_102 = p_locplayer->Target;
          do_change_mouse(5);
        }
    }
    else
    {
        do_change_mouse(8);
    }
}

void show_goto_point(uint flag)
{
    asm volatile ("call ASM_show_goto_point\n"
        : : "a" (flag));
    return;
}

void draw_hud(int dcthing)
{
#if 0
    asm volatile ("call ASM_draw_hud\n"
        : : "a" (dcthing));
    return;
#endif
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    if (ingame.TrackThing != 0)
    {
        if (!game_cam_tracked_thing_is_player_agent())
            return;
    }
    if ((ingame.Flags & GamF_HUDPanel) == 0)
        return;

    show_goto_point(0);

    if (target_old_frameno == 0)
        target_old_frameno = nstart_ani[983];

    {
        struct Thing *p_mothing;

        p_mothing = &things[p_locplayer->DirectControl[mouser]];
        if (!lbDisplay.MRightButton
          || (p_mothing->PTarget == NULL)
          || current_weapon_has_targetting(p_mothing)
          || (p_mothing->U.UPerson.WeaponTimer < 14)) {
            p_locplayer->field_102 = 0;
        }
    }

    draw_hud_lock_target();

    if (ingame.DisplayMode == 50)
    {
        short plagent;
        short target;

        for (plagent = 0; plagent < playable_agents; plagent++)
        {
            struct Thing *p_agent;

            p_agent = p_locplayer->MyAgent[plagent];
            number_player(p_agent, plagent);
            if ((p_agent->Flag & TngF_Unkn1000) != 0)
            {
                short ctlmode;
                ctlmode = p_locplayer->UserInput[plagent].ControlMode & ~UInpCtr_AllFlagsMask;
                if (ctlmode != UInpCtr_Mouse)
                {
                    if (p_agent->PTarget != NULL)
                        draw_target_person(p_agent->PTarget, 2);
                }
            }
        }

        if (!PacketRecord_IsPlayback())
        {
          draw_hud_target_mouse(dcthing);
        }

        target = p_locplayer->field_102;
        if (target > 0)
        {
            if (!thing_is_destroyed(target))
                draw_hud_target2(dcthing, target);
        }
        draw_new_panel();
    }
}

void func_6fd1c(int a1, int a2, int a3, int a4, int a5, int a6, ubyte a7)
{
    asm volatile (
      "push %6\n"
      "push %5\n"
      "push %4\n"
      "call ASM_func_6fd1c\n"
        : : "a" (a1), "d" (a2), "b" (a3), "c" (a4), "g" (a5), "g" (a6), "g" (a7));
}

void draw_engine_net_text(void)
{
    asm volatile ("call ASM_draw_engine_net_text\n"
        :  :  : "eax" );
}

void check_mouse_overvehicle(struct Thing *p_thing, ubyte target_assign)
{
#if 0
    asm volatile ("call ASM_check_mouse_overvehicle\n"
        : : "a" (p_thing), "d" (target_assign));
    return;
#endif
    struct ShEnginePoint sp;
    int cor_x, cor_y, cor_z;
    int ms_dx, ms_dy, tng_dim;

    cor_x = PRCCOORD_TO_MAPCOORD(p_thing->X) - engn_xc;
    cor_y = PRCCOORD_TO_YCOORD(p_thing->Y) - engn_yc;
    cor_z = PRCCOORD_TO_MAPCOORD(p_thing->Z) - engn_zc;

    transform_shpoint(&sp, cor_x, cor_y - 8 * engn_yc, cor_z);

    ms_dx = lbDisplay.MMouseX - sp.X;
    ms_dy = lbDisplay.MMouseY - sp.Y;
    tng_dim = ((p_thing->Radius >> 3) * overall_scale) >> 8;

    if (ms_dy * ms_dy + ms_dx * ms_dx < tng_dim * tng_dim)
    {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
        p_locplayer->Target = p_thing->ThingOffset;
        p_locplayer->TargetType = target_assign;
    }
}

int mech_unkn_func_03(struct Thing *p_thing)
{
    int ret;
    asm volatile ("call ASM_mech_unkn_func_03\n"
        : "=r" (ret) : "a" (p_thing));
    return ret;
}

void func_13A78(void)
{
    asm volatile ("call ASM_func_13A78\n"
        :  :  : "eax" );
}

void process_map_craters(void)
{
    asm volatile ("call ASM_process_map_craters\n"
        :  :  : "eax" );
}

void engine_draw_things(int pos_beg_x, int pos_beg_z, int rend_beg_x, int rend_beg_z, short tlcount_x, short tlcount_z)
{
    int tlno_x, pos_x;
    int view_end_x, view_beg_z;
    int view_beg_x, view_end_z;

    view_end_x = rend_beg_x + 512;
    view_beg_z = rend_beg_z - 512;
    view_beg_x = rend_beg_x - ((render_area_a << 8) + 512);
    view_end_z = rend_beg_z + ((render_area_b << 8) + 512);

    for (tlno_x = 0, pos_x = pos_beg_x; tlno_x < tlcount_x; tlno_x++, pos_x += -256)
    {
        int tlno_z, pos_z;

        for (tlno_z = 0, pos_z = pos_beg_z; tlno_z < tlcount_z; tlno_z++, pos_z += 256)
        {
            struct MyMapElement *p_mapel;

            if (pos_x <= TILE_TO_MAPCOORD(0,0) || pos_x >= MAP_COORD_WIDTH)
                continue;
            if (pos_z <= TILE_TO_MAPCOORD(0,0) || pos_z >= MAP_COORD_HEIGHT)
                continue;

            p_mapel = &game_my_big_map[MAPCOORD_TO_TILE(pos_x) + MAPCOORD_TO_TILE(pos_z) * MAP_TILE_WIDTH];

            if (pos_x >= view_beg_x && pos_x <= view_end_x && pos_z >= view_beg_z && pos_z <= view_end_z)
            {
                ThingIdx thing;
                ushort lv;

                lv = p_mapel->ColHead;
                if (lv != 0)
                {
                    thing = game_col_vects_list[lv].Object;
                    if (thing > 0)
                    {
                        struct Thing *p_thing;
                        p_thing = &things[thing];
                        if ((p_thing->Type == TT_BUILDING)
                         && (p_thing->U.UObject.DrawTurn != gameturn)) {
                            draw_thing_object(p_thing);
                        }
                    }
                }
                thing = p_mapel->Child;
                while (thing != 0)
                {
                    if (thing > 0)
                    {
                        struct Thing *p_thing;
                        p_thing = &things[thing];
                        thing = draw_thing_object(p_thing);
                        continue;
                    }
                    else
                    {
                        struct SimpleThing *p_sthing;
                        p_sthing = &sthings[thing];
                        thing = draw_sthing_object(p_sthing);
                        continue;
                    }
                }
            }
            else
            {
                ThingIdx thing;
                ushort lv;

                lv = p_mapel->ColHead;
                if (lv != 0)
                {
                    thing = game_col_vects_list[lv].Object;
                    if (thing > 0)
                    {
                        struct Thing *p_thing;
                        p_thing = &things[thing];
                        if ((p_thing->Type == TT_BUILDING)
                          && (p_thing->U.UObject.DrawTurn != gameturn)
                          && (p_thing->U.UObject.BHeight > 1400)) {
                            draw_thing_object(p_thing);
                        }
                    }
                }
                thing = p_mapel->Child;
                while (thing != 0)
                {
                    if (thing > 0)
                    {
                        struct Thing *p_thing;
                        p_thing = &things[thing];
                        if ( p_thing->Type == TT_BUILDING
                          && (p_thing->U.UObject.DrawTurn != gameturn)
                          && (p_thing->U.UObject.BHeight > 1400)) {
                            thing = draw_thing_object(p_thing);
                            continue;
                        }
                        thing = p_thing->Next;
                    }
                    else
                    {
                        struct SimpleThing *p_sthing;
                        p_sthing = &sthings[thing];
                        thing = p_sthing->Next;
                    }
                }
            }
        }
    }
}

ubyte get_engine_inputs(void)
{
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    if (is_gamekey_pressed(GKey_TRANS_OBJ_SURF_COL))
    {
        if (lbShift & KMod_SHIFT)
            game_option_inc(GOpt_TranspObjSurfaceColr);
        else
            game_option_dec(GOpt_TranspObjSurfaceColr);
        clear_gamekey_pressed(GKey_TRANS_OBJ_SURF_COL);
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_TRANS_OBJ_LINE_COL))
    {
        if (lbShift & KMod_SHIFT)
            game_option_inc(GOpt_TranspObjLineColr);
        else
            game_option_dec(GOpt_TranspObjLineColr);
        clear_gamekey_pressed(GKey_TRANS_OBJ_LINE_COL);
        did_inp |= GINPUT_DIRECT;
    }
    return did_inp;
}

void screen_position_face_render_callback(
  struct PolyPoint *p_pt1,
  struct PolyPoint *p_pt2,
  struct PolyPoint *p_pt3,
  ushort face, ubyte type)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    if (p_locplayer->TargetType < TrgTp_Unkn3) {
        check_mouse_over_face(p_pt1, p_pt2, p_pt3, face, type);
    }
}

void screen_sorted_sprite_1a_render_callback(ushort sspr)
{
    struct Thing *p_thing;
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    p_thing = game_sort_sprites[sspr].PThing;
    if ((p_locplayer->TargetType <= TrgTp_DroppedTng) && (p_thing->Type == SmTT_DROPPED_ITEM)) {
        check_mouse_overlap_item(sspr);
    }

    if ((p_locplayer->TargetType < TrgTp_Unkn6) && (p_thing->Type == TT_MINE))
    {
        if ((p_thing->SubType == 7) || (p_thing->SubType == 3))
            check_mouse_overlap_item(sspr);
        else if (p_thing->SubType == 48)
            check_mouse_overlap(sspr);
    }
}

void process_engine_unk3(void)
{
    PlayerInfo *p_locplayer;

    get_engine_inputs();

    reset_drawlist();
    screen_position_face_render_cb = screen_position_face_render_callback;
    screen_sorted_sprite_render_cb = screen_sorted_sprite_1a_render_callback;
    player_target_clear(local_player_no);
    mech_unkn_dw_1DC880 = mech_unkn_tile_x1;
    mech_unkn_dw_1DC884 = mech_unkn_tile_y1;
    mech_unkn_dw_1DC888 = mech_unkn_tile_x2;
    mech_unkn_dw_1DC88C = mech_unkn_tile_y2;
    mech_unkn_dw_1DC890 = mech_unkn_tile_x3;
    mech_unkn_dw_1DC894 = mech_unkn_tile_y3;

    process_map_craters();
    func_13A78();

    if (((ingame.Flags & GamF_BillboardBAT) == 0) &&
      ((ingame.Flags & GamF_BillboardMovies) != 0))
    {
        dword_176CBC += fifties_per_gameturn;
        if (dword_176CBC > 80)
        {
            dword_176CBC = 0;
            if (!in_network_game && ((ingame.Flags & GamF_Unkn00040000) != 0))
            {
                ingame.Flags &= ~GamF_Unkn00040000;
                xdo_next_frame(AniSl_BILLBOARD);
            }
        }
    }

    int rend_beg_x, rend_beg_z;
    int pos_beg_x, pos_beg_z;
    int tlcount_x, tlcount_z;

    camera_setup_view(&pos_beg_x, &pos_beg_z, &rend_beg_x, &rend_beg_z, &tlcount_x, &tlcount_z);

    if ((ingame.Flags & GamF_RenderScene) != 0)
    {
        engine_draw_things(pos_beg_x, pos_beg_z, rend_beg_x, rend_beg_z, tlcount_x, tlcount_z);
    }

    if ((ingame.Flags & GamF_RenderScene) != 0)
    {
        if ((gamep_scene_effect_type == ScEff_SPACE) && engine_render_lights)
            draw_background_stars();
        if (game_perspective == 6) {
            draw_background_stars();
        } else {
            lvdraw_do_floor();
        }
    }

    if (word_1552F8 != 36 && !byte_1C8444)
    {
        clear_super_quick_lights();
    }
    assert(vec_tmap[1] != NULL);
    vec_map = vec_tmap[1];
    p_locplayer = &players[local_player_no];
    if ((ingame.Flags & GamF_RenderScene) != 0)
    {
        draw_explode();
        draw_screen();
        draw_hud(p_locplayer->DirectControl[0]);
        if (in_network_game)
            draw_engine_net_text();
        if (debug_hud_collision)
            draw_engine_unk3_last(engn_xc, engn_zc);
    }
    else
    {
        draw_hud(p_locplayer->DirectControl[0]);
        reset_drawlist();
    }
}

void process_sound_heap(void)
{
    asm volatile ("call ASM_process_sound_heap\n"
        :  :  : "eax" );
}

void func_cc0d4(char **str)
{
    asm volatile ("call ASM_func_cc0d4\n"
        : : "a" (str));
}

void screen_animate_draw_outro_text(void)
{
    const char *text1;
    const char *text2;

    switch (background_type)
    {
    case 0:
    default:
        text1 = unk_credits_text_s;
        text2 = outro_text_s;
        break;
    case 1:
        text1 = unk_credits_text_z;
        text2 = outro_text_z;
        break;
    case 2:
        text1 = unk_credits_text_p;
        text2 = outro_text_z;
        break;
    }
    func_cc638(text1, text2);
}

TbKeyCode screen_wait_seconds_or_until_continue_key(int sec)
{
    int i;
    TbKeyCode key;

    clear_key_pressed(KC_SPACE);
    clear_key_pressed(KC_ESCAPE);
    clear_key_pressed(KC_RETURN);

    key = KC_UNASSIGNED;
    for (i = sec*game_num_fps; i != 0; i--)
    {
        if (is_key_pressed(KC_SPACE, KMod_DONTCARE)) {
            key = KC_SPACE;
            break;
        }
        if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE)) {
            key = KC_ESCAPE;
            break;
        }
        if (is_key_pressed(KC_RETURN, KMod_DONTCARE)) {
            key = KC_RETURN;
            break;
        }

        swap_wscreen();
        game_update();
    }
    clear_key_pressed(key);

    return key;
}

TbKeyCode screen_wait_seconds_or_until_yesno_key(int sec)
{
    int i;
    TbKeyCode key;

    clear_key_pressed(KC_Y);
    clear_key_pressed(KC_N);

    key = KC_UNASSIGNED;
    for (i = sec*game_num_fps; i != 0; i--)
    {
        if (is_key_pressed(KC_Y, KMod_DONTCARE)) {
            key = KC_Y;
            break;
        }
        if (is_key_pressed(KC_N, KMod_DONTCARE)) {
            key = KC_N;
            break;
        }

        swap_wscreen();
        game_update();
    }
    clear_key_pressed(key);

    return key;
}

void init_outro(void)
{
    gamep_scene_effect_type = ScEff_NONE;
    gamep_scene_effect_intensity = 1000;
    StopAllSamples();
    StopCD();

    play_smacker(MPly_Outro);

    screen_dark_curtain_down();

    data_155704 = -1;
    init_things();
    //TODO hard-coded map ID
    change_current_map(51);
    load_outro_sprites();

    outro_credits_enabled = 0;
    outro_unkn02 = 0;
    outro_unkn03 = 0;
    gameturn = 0;
    render_anim_turn = gameturn;

    screen_animate_draw_outro_text();
    // Sleep for up to 10 seconds
    screen_wait_seconds_or_until_continue_key(10);

    LbPaletteFade(0, 200, 1);

    show_black_screen();
    StopAllSamples();
    reset_heaps();
    setup_heaps(SHSC_CreditsSnd, language_3str);
    play_sample_using_heap(0, 1, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);

    data_197150 = 1;
    data_1dd91c = 0;
    unkn_flags_01 = 1;
    overall_scale = 40;
    set_brightness_fadedout();

    while (1)
    {
        if (is_key_pressed(KC_SPACE, KMod_DONTCARE))
            break;
        if (is_key_pressed(KC_ESCAPE, KMod_DONTCARE))
            break;
        if (is_key_pressed(KC_RETURN, KMod_DONTCARE))
            break;

        if (gameturn < 128)
        {
            // 64 steps of brightness increase - fade in
            if (gameturn & 1)
                change_brightness(1);
            if (gameturn == 127)
                outro_credits_enabled = 1;
        } else {
            // Randomly play seagull sample if applause is not currently playing
            if (((LbRandomAnyShort() & 0xF) == 0) && (data_155704 == -1 || !IsSamplePlaying(0, data_155704, NULL)) )
            {
                play_sample_using_heap(0, 7 + (LbRandomAnyShort() % 5), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
            }
        }

        gameturn++;
        render_anim_turn = gameturn;
        traffic_unkn_func_01();
        process_engine_unk1();
        process_sound_heap();
        func_2e440();
        if (outro_credits_enabled)
        {
            outro_unkn02++;
            func_cc0d4((char **)&people_credits_groups[2 * outro_unkn03]);
            if (outro_unkn02 > data_1ddb68 + 50)
            {
                outro_unkn02 = 0;
                outro_unkn03++;
                if (outro_unkn03 == people_groups_count)
                    outro_unkn03 = 0;
            }
          }
          swap_wscreen();
          game_update();
          LbScreenClear(0);
    }
    clear_key_pressed(KC_SPACE);
    clear_key_pressed(KC_ESCAPE);
    clear_key_pressed(KC_RETURN);

    StopAllSamples();
    reset_heaps();
    setup_heaps(SHSC_GameSndBestQ, language_3str);
}

void srm_scanner_set_size_at_bottom_left(short margin, short width, short height)
{
    short hlimit;

    // Limit the height here, to make sure reduced rectangle is still put at bottom
    hlimit = sizeof(ingame.Scanner.Width)/sizeof(ingame.Scanner.Width[0]);
    if (height >= hlimit)
        height = hlimit - 1;

    SCANNER_set_screen_box(1, lbDisplay.GraphicsScreenHeight - margin - height,
        width, height, 24);
}

void srm_scanner_size_update(void)
{
    short margin, width, height;

    panel_get_scanner_screen_size(&margin, &width, &height,
      lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight, pop1_sprites_scale);
    srm_scanner_set_size_at_bottom_left(margin, width, height);
}

void init_scanner_colour(void)
{
    sbyte panperm;
    ubyte col;

    panperm = ingame.PanelPermutation;
    if ((panperm == 2) || (panperm == -3)) {
        col = 1;
    } else
    if ((panperm == 0) || (panperm == -1)) {
        col = 2;
    } else {
        col = 2;
    }
    SCANNER_set_colour(col);
    SCANNER_fill_in();
}

void init_scanner(void)
{
    init_scanner_colour();
    dword_1AA5C4 = 0;
    dword_1AA5C8 = 0;
    ingame.Scanner.Brightness = 8;
    ingame.Scanner.Contrast = 5;
    SCANNER_width = ingame.Scanner.Width;
    ingame.Scanner.Zoom = 128;
    ingame.Scanner.Angle = 0;
    srm_scanner_size_update();
    SCANNER_init();
}

/**
 * Updates engine parameters for best display for current video mode within the tactical mission.
 */
TbBool adjust_mission_engine_to_video_mode(void)
{
    TbBool ret;

    ret = true;
    game_high_resolution = (lbDisplay.ScreenMode == screen_mode_game_hi);
    // Set scale 15% over the min, to create a nice pan effect
    overall_scale = (get_overall_scale_min() * 295) >> 8;

    if (load_pop_sprites_for_current_mode() == Lb_FAIL)
        ret = false;
    if (load_mouse_pointers_sprites_for_current_mode() == Lb_FAIL)
        ret = false;
    if (load_small_font_for_current_ingame_mode() == Lb_FAIL)
        ret = false;

    render_area_a = render_area_b = \
      get_render_area_for_zoom(user_zoom_min);
    srm_scanner_size_update();

    return ret;
}

void show_simple_load_screen(void)
{
    // TODO TEXT instead of repeating "Initiating.." show "Established"
    char *text = gui_strings[376];
    int w,h;

    LbScreenClear(0);
    lbFontPtr = small_font;
    w = LbTextStringWidth(text);
    h = LbTextStringHeight(text);
    LbTextDraw((lbDisplay.GraphicsScreenWidth - w) >> 1,
      (lbDisplay.GraphicsScreenHeight - h) >> 1, text);
    swap_wscreen();
}

void setup_engine_screen_mode(void)
{
    LbFileLoadAt("qdata/pal.pal", display_palette);
    LbPaletteSet(display_palette);

    if (game_high_resolution)
    {
        if (lbDisplay.ScreenMode != screen_mode_game_hi)
            setup_screen_mode(screen_mode_game_hi);
    }
    else
    {
        if (lbDisplay.ScreenMode != screen_mode_game_lo)
            setup_screen_mode(screen_mode_game_lo);
    }
    // Setup colour conversion tables
    LbColourTablesLoad(display_palette, "data/tables.dat");
    LbGhostTableLoad(display_palette, 50, "data/synghost.tab");
}

void setup_initial_screen_mode(void)
{
    if (game_high_resolution)
        lbDisplay.ScreenMode = screen_mode_game_hi;
    else
        lbDisplay.ScreenMode = screen_mode_game_lo;
    setup_simple_screen_mode(lbDisplay.ScreenMode);
    show_black_screen();
    // Setup colour conversion tables, allowing generation
    if (display_palette != NULL)
    {
        LbColourTablesLoad(display_palette, "data/tables.dat");
        LbGhostTableGenerate(display_palette, 50, "data/synghost.tab");
    }
    else
    {
        LOGERR("Palette not ready - skipping colour tables setup");
    }
    if (screen_idle_update_initialize() != Lb_SUCCESS)
    {
        LOGERR("Idle screen update not registered - expect choppy mouse");
    }
#if 0
    // Not sure why we would do this fixup. Maybe it's only for
    // sprite generation? If used before gameplay, it causes bad
    // while fading beyond map borders (always fades to full white)
    colour_tables_ghost_fixup();
#endif
}

void screen_mode_switch_to_next(void)
{
    TbScreenMode nmode;

    if (lbDisplay.ScreenMode == screen_mode_game_lo)
        nmode = screen_mode_game_hi;
    else
        nmode = screen_mode_game_lo;
    LOGSYNC("Switching to mode %d", (int)nmode);

    setup_screen_mode(nmode);
    adjust_mission_engine_to_video_mode();
}

ubyte game_graphics_inputs(void)
{
    PlayerInfo *p_locplayer;
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    p_locplayer = &players[local_player_no];
    if ((ingame.DisplayMode != DpM_ENGINEPLY) &&
        (ingame.DisplayMode != DpM_UNKN_3B))
        return did_inp;
    if (in_network_game && p_locplayer->PanelState[mouser]
      == PANEL_STATE_SEND_MESSAGE)
        return did_inp;

    if (is_gamekey_pressed(GKey_CAMERA_PERSPECTV))
    {
        clear_gamekey_pressed(GKey_CAMERA_PERSPECTV);
        game_option_inc(GOpt_CameraPerspective);
        did_inp |= GINPUT_DIRECT;
    }

    if (is_key_pressed(KC_F8, KMod_DONTCARE))
    {
        clear_key_pressed(KC_F8);
        screen_mode_switch_to_next();
        did_inp |= GINPUT_DIRECT;
    }

    return did_inp;
}

ubyte input_agents_cheats(void)
{
    PlayerInfo *p_locplayer;
    struct Packet *p_pckt;
    ThingIdx dcthing;
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    p_locplayer = &players[local_player_no];

    if (in_network_game && p_locplayer->PanelState[mouser]
      == PANEL_STATE_SEND_MESSAGE)
        return did_inp;

    if ((ingame.UserFlags & UsrF_Cheats) != 0)
    {
        if (is_key_pressed(KC_T, KMod_ALT))
        {
            clear_key_pressed(KC_T);

            p_pckt = &packets[local_player_no];
            dcthing = p_locplayer->DirectControl[0];
            my_build_packet(p_pckt, PAct_CHEAT_AGENT_TELEPORT, dcthing, mouse_map_x, 0, mouse_map_z);
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
    }

    if ((ingame.UserFlags & UsrF_Cheats) && !in_network_game)
    {
        if (is_key_pressed(KC_Q, KMod_ALT))
        {
            clear_key_pressed(KC_Q);

            p_pckt = &packets[local_player_no];
            my_build_packet(p_pckt, PAct_CHEAT_ALL_AGENTS, PCheatAA_BEEFUP_AND_MODS, 0, 0, 0);
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
        if (is_key_pressed(KC_Q, KMod_SHIFT))
        {
            clear_key_pressed(KC_Q);

            p_pckt = &packets[local_player_no];
            my_build_packet(p_pckt, PAct_CHEAT_ALL_AGENTS, PCheatAA_RESURRECT_AND_WEPAPN, 0, 0, 0);
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
    }

    return did_inp;
}

void init_syndwars(void)
{
    //char locstr[100];
    AudioInitOptions audOpts;

    LOGSYNC("Starting");
    //sprintf(locstr, "%sSound", cd_drive); -- unused

    audOpts.SoundDataPath = "sound";
    audOpts.SoundDriverPath = "sound";
    audOpts.IniPath = "sound";
    audOpts.AutoScan = 1;
    audOpts.StereoOption = 1;
    audOpts.DisableLoadSounds = 1;
    audOpts.InitRedbookAudio = 2;
    audOpts.UseCurrentAwe32Soundfont = 1;
    audOpts.AbleFlags = 3;
    audOpts.SoundType = 1622;
    audOpts.MaxSamples = 10;
    InitAudio(&audOpts);

    if (!GetCDAble())
        StopMusicIfActive();
    fill_ail_sample_ids();
    if (GetSoundInstalled() && GetSoundAble() && GetSoundActive())
        ingame.Flags |= GamF_Unkn00020000;
    setup_heaps(SHSC_GameSndAutoQ, language_3str);
}

void setup_host_sub6(void)
{
    asm volatile ("call ASM_setup_host_sub6\n"
        :  :  : "eax" );
}

void setup_debug_obj_trace(void)
{
    // Code for setting memory ranges was here
    // Removed, as we no longer have such tracking/profiling mechanism
    debug_trace_place(0);
}

TbResult prep_multicolor_sprites(void)
{
    PathInfo *pinfo;
    TbResult ret;

    pinfo = &game_dirs[DirPlace_Data];
    ret = load_multicolor_sprites(pinfo->directory);
    setup_multicolor_sprites();
    if (ret == Lb_FAIL) {
        LOGERR("Some files were not loaded successfully");
    }
    return ret;
}

TbBool setup_host(void)
{
    TbBool ret;

    ret = true;
    BAT_unknsub_20(0, 0, 0, 0, vec_tmap[4] + 160 * 256 + 64);
    smack_malloc_setup();
    LOGDBG("&setup_host() = 0x%p", (void *)setup_host);
    setup_initial_screen_mode();

    if (keyboard_mode_direct)
        LbKeyboardOpen();
    else
        LbIKeyboardOpen();
    init_buffered_keys();

    setup_mouse_pointers();
    lbMouseAutoReset = false;
    mouse_setup(true);

    setup_debug_obj_trace();

    // Default Trenchcoat color loaded from missionspr_load_files[]
    // should be set earlier with default user options
    assert(ingame.TrenchcoatPreference == 0);
    setup_multicolor_sprites();
    assert(ingame.PanelPermutation == -2);

    if (load_pop_sprites_for_current_mode() == Lb_FAIL) {
        LOGERR("Load pop sprites failed.");
        ret = false;
    }
    if (load_mouse_pointers_sprites_for_current_mode() == Lb_FAIL) {
        LOGERR("Load mouse pinters sprites failed.");
        ret = false;
    }
    if (load_small_font_for_current_ingame_mode() == Lb_FAIL) {
        LOGERR("Load small font sprites failed.");
        ret = false;
    }
    if (init_memory(mem_game) == Lb_FAIL) {
        LOGERR("Init memory failed.");
        ret = false;
    }
    propagate_memory_sizes();

    init_syndwars();
    LoadSounds(0);
    LoadMusic(0);

    setup_host_sub6();
    play_intro();
    flic_unkn03(AniSl_BILLBOARD);

    return ret;
}

void init_engine(void)
{
    asm volatile ("call ASM_init_engine\n"
        :  :  : "eax" );
}

void unkn_truce_groups(void)
{
    asm volatile ("call ASM_unkn_truce_groups\n"
        :  :  : "eax" );
}

void blind_progress_game(ulong nturns)
{
    ulong n;

    for (n = 0; n < nturns; n++)
    {
        process_things();
        gameturn++;
    }
}

void map_lights_update(void)
{
    asm volatile ("call ASM_map_lights_update\n"
        :  :  : "eax" );
}

void func_74934(void)
{
    asm volatile ("call ASM_func_74934\n"
        :  :  : "eax" );
}

void init_crater_textures(void)
{
    asm volatile ("call ASM_init_crater_textures\n"
        :  :  : "eax" );
    return;
}

void find_the_tall_buildings(void)
{
    asm volatile ("call ASM_find_the_tall_buildings\n"
        :  :  : "eax" );
    return;
}

void func_749fc(void)
{
    asm volatile ("call ASM_func_749fc\n"
        :  :  : "eax" );
    return;
}

void clear_word_1774E8(void)
{
    short i;

    for (i = 0; i < 150; i++)
    {
        word_1774E8[2 * i + 0] = 0;
    }
}

void init_my_paths(void)
{
    asm volatile ("call ASM_init_my_paths\n"
        :  :  : "eax" );
    return;
}

void init_level_unknsub01_person(struct Thing *p_person)
{
    person_init_preplay_command(p_person);

    if (((p_person->Flag2 & TgF2_ExistsOffMap) == 0)
      && ((p_person->Flag & TngF_Destroyed) == 0))
    {
        struct GroupAction *p_grpact;

        p_grpact = &group_actions[p_person->U.UPerson.Group];
        p_grpact->Alive++;
    }
    p_person->U.UPerson.CurrentWeapon = 0;
    p_person->U.UPerson.PathIndex = 0;
    p_person->U.UPerson.Target2 = 0;
    p_person->U.UPerson.Within = 0;
    p_person->U.UPerson.EffectiveGroup = p_person->U.UPerson.Group;
    p_person->PTarget = NULL;
    p_person->U.UPerson.WeaponsCarried |= (1 << (WEP_ENERGYSHLD-1));
    p_person->OldTarget = 0;

    if ((p_person->Flag & TngF_Destroyed) != 0) {
        p_person->State = PerSt_DEAD;
    } else {
        p_person->State = PerSt_NONE;
    }
    if ((p_person->Flag & TngF_PlayerAgent) != 0) {
        p_person->U.UPerson.ComCur = 0;
    } else {
        p_person->U.UPerson.ComCur = p_person->U.UPerson.ComHead;
        p_person->Flag |= TngF_Unkn0040;
    }

    if ((p_person->Flag2 & TgF2_ExistsOffMap) != 0)
        delete_node(p_person);
    else
        p_person->Flag2 &= ~TgF2_InsideBuilding;

    if ((p_person->Flag & TngF_Destroyed) == 0)
    {
        if (p_person->U.UPerson.CurrentWeapon != 0)
            switch_person_anim_mode(p_person, ANIM_PERS_WEPLIGHT_IDLE);
        else
            switch_person_anim_mode(p_person, ANIM_PERS_IDLE);
    }
}

void init_level_unknsub01_building(struct Thing *p_buildng)
{
    p_buildng->Flag &= TngF_Unkn0800;
    if (p_buildng->SubType == SubTT_BLD_MGUN)
    {
        p_buildng->PTarget = NULL;
        p_buildng->U.UObject.EffectiveGroup = p_buildng->U.UObject.Group;
    }
    if ((p_buildng->Flag2 & TgF2_ExistsOffMap) != 0)
    {
        delete_node(p_buildng);
    }
}

void init_level_unknsub01_vehicle(struct Thing *p_vehicle)
{
    if ((p_vehicle->Flag2 & TgF2_ExistsOffMap) != 0)
    {
        delete_node(p_vehicle);
    }
}

void init_level_unknsub01(void)
{
    ThingIdx thing;

    thing = things_used_head;
    while (thing > 0)
    {
        struct Thing *p_thing;

        p_thing = &things[thing];
        switch (p_thing->Type)
        {
        case TT_VEHICLE:
            init_level_unknsub01_vehicle(p_thing);
            break;
        case TT_PERSON:
            init_level_unknsub01_person(p_thing);
            break;
        case TT_BUILDING:
            init_level_unknsub01_building(p_thing);
            break;
        default:
            break;
        }
        thing = p_thing->LinkChild;
    }
}

void setup_ingame_scene_effect(void)
{
    struct Mission *p_missi;
    ubyte atmos, weather;

    p_missi = &mission_list[ingame.CurrentMission];
    atmos = ((p_missi->Atmosphere) & 0x0F);
    weather = ((p_missi->Atmosphere >> 4) & 0x0F);

    // Reload texture regardless of the effect
    LbFileLoadAt("data/tex00.dat", vec_tmap[0]);

    switch (atmos)
    {
    case MAtmsph_Space:
        gamep_scene_effect_type = ScEff_SPACE;
        gamep_scene_effect_intensity = 1000;
        break;
    case MAtmsph_Moon:
        gamep_scene_effect_type = ScEff_NONE;
        gamep_scene_effect_intensity = 1000;
        break;
    case MAtmsph_Earth:
        stop_sample_using_heap(0, 78);
        if (weather == MWeathr_Rainy) {
            gamep_scene_effect_type = ScEff_RAIN;
            gamep_scene_effect_intensity = 1000;
            scene_post_effect_rain_init();
        } else if (weather == MWeathr_Snowy) {
            gamep_scene_effect_type = ScEff_SNOW;
            gamep_scene_effect_intensity = 1000;
            //TODO add some initial effect to textures?
        } else {
            gamep_scene_effect_type = ScEff_NONE;
            gamep_scene_effect_intensity = 1000;
        }
        break;
    default:
        gamep_scene_effect_type = ScEff_NONE;
        gamep_scene_effect_intensity = 1000;
        break;
    }
}

void start_ingame_ambient_sound(void)
{
    struct Mission *p_missi;
    ubyte atmos;

    p_missi = &mission_list[ingame.CurrentMission];
    atmos = ((p_missi->Atmosphere) & 0x0F);

    switch (atmos)
    {
    case MAtmsph_Space:
    case MAtmsph_Moon:
        stop_sample_using_heap(0, 8);
        stop_sample_using_heap(0, 77);
        play_sample_using_heap(0, 78, 64, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 2);
        break;
    case MAtmsph_Earth:
        stop_sample_using_heap(0, 78);
        if (gamep_scene_effect_type == ScEff_RAIN) {
            stop_sample_using_heap(0, 8);
            play_sample_using_heap(0, 77, 64, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 2);
        } else {
            stop_sample_using_heap(0, 77);
            play_sample_using_heap(0, 8, 64, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 2);
        }
        break;
    default:
        stop_sample_using_heap(0, 78);
        stop_sample_using_heap(0, 8);
        stop_sample_using_heap(0, 77);
        break;
    }
}

void init_level(void)
{
#if 0
    asm volatile ("call ASM_init_level\n"
        :  :  : "eax" );
    return;
#endif
    short plyr_no;

    people_intel(1);
    if (in_network_game)
    {
        ingame.DetailLevel = 1;
        for (plyr_no = 0; plyr_no < PLAYERS_LIMIT; plyr_no++)
        {
            player_message_timer[plyr_no] = 0;
            player_message_text[plyr_no][0] =  '\0';
        }
    }
    else
    {
        init_mission_states();
    }

    plyr_no = 0;
    for (plyr_no = 0; plyr_no < PLAYERS_LIMIT; plyr_no++)
    {
        PlayerInfo *p_player;
        short plagent;

        p_player = &players[plyr_no];
        for (plagent = 0; plagent < LOCAL_USERS_MAX_COUNT; plagent++)
        {
            p_player->UserVX[plagent] = 0;
            p_player->UserVY[plagent] = 0;
            p_player->UserVZ[plagent] = 0;
            p_player->SpecialItems[plagent] = 0;
            p_player->PanelItem[plagent] = 0;
            p_player->PanelState[plagent] = PANEL_STATE_NORMAL;
        }
        p_player->GotoFace = 0;
        p_player->field_102 = 0;
        player_target_clear(plyr_no);
    }

    set_user_selected_brightness();
    ingame.Flags &= ~TngF_Unkn8000;
    if (!in_network_game)
        ingame.InNetGame_UNSURE = 1;
    word_1531DA = 1;
    shield_frames_init();
    ingame.fld_unkCB7 = 0;
    ingame.fld_unkC59 = 0;
    ingame.FlameCount = 0;
    ingame.SoundThing = 0;
    ingame.fld_unkCB5 = 0;
    clear_open_mission_status();
    init_free_explode_faces();
    StopAllSamples();
    ingame.TrackThing = 0;
    func_74934();
    ingame.TrackX = engn_xc;
    ingame.TrackZ = engn_zc;
    ingame.fld_unkCA6 = 0;
    ingame.UserZoom = 120;
    word_1AABD0 = next_floor_texture;
    init_crater_textures();
    bang_init();
    FIRE_init_or_samples_init();
    func_749fc();
    preprogress_trains_turns(50);
    tnext_floor_texture = next_floor_texture + 1;
    init_col_vects_linked_list();
    ingame.fld_unkC91 = dos_clock();
    setup_ingame_scene_effect();
    start_ingame_ambient_sound();
    gamep_unknval_10 = 0;
    gamep_unknval_12 = 0;
    nav_stats__ThisTurn = 0;
    gamep_unknval_16 = 0;
    ingame.fld_unkCB1 = 1;
    ingame.fld_unkCB2 = 1;
    clear_word_1774E8();
    missions_clear_bank_tests();
    thing_groups_clear_all_actions();
    init_my_paths();

    init_level_unknsub01();

    VNAV_preprocess_bezier_turns(1);
    VNAV_init_new_traffic_system();
#if 0
    if (last_map_for_lights_func_11 != current_map) // maps are often reloaded, so we cannot skip lights update
#endif
    map_lights_update();
    last_map_for_lights_func_11 = current_map;
    init_scanner();
    find_the_tall_buildings();
    PlayCDTrack(ingame.CDTrack);
    StartMusic(ingame.DangerTrack, 0);
}

void init_level_3d(ubyte flag)
{
#if 0
    asm volatile ("call ASM_init_level_3d\n"
        : : "a" (flag));
#endif
    ushort i;

    if (flag)
    {
        next_object = stored_l3d_next_object;
        next_object_face = stored_l3d_next_object_face;
        next_object_face4 = stored_l3d_next_object_face4;
        next_object_point = stored_l3d_next_object_point;
        next_normal = stored_l3d_next_normal;
        next_face_texture = stored_l3d_next_face_texture;
        next_floor_texture = stored_l3d_next_floor_texture;
        next_local_mat = stored_l3d_next_local_mat;
        stored_level3d_inuse = 0;
    }
    else
    {
        stored_l3d_next_object = next_object;
        stored_l3d_next_object_face = next_object_face;
        stored_l3d_next_object_face4 = next_object_face4;
        stored_l3d_next_object_point = next_object_point;
        stored_l3d_next_normal = next_normal;
        stored_l3d_next_face_texture = next_face_texture;
        stored_l3d_next_floor_texture = next_floor_texture;
        stored_l3d_next_local_mat = next_local_mat;
        stored_level3d_inuse = 1;

        // Prepare objects for rockets
        for (i = 0; i < WEP_ROCKETS_FIRED_LIMIT; i++)
        {
            ushort snobj;
            snobj = copy_prim_obj_to_game_object(0, 0, -prim_unknprop01 - 20, 0);
            unkn_object_shift_03(snobj);
            ingame.Rocket1[i] = snobj;
        }
        unkn2_pos_x = 64;
        unkn2_pos_y = 64;
        unkn2_pos_z = 64;
    }
}

void unkn1_handle_agent_groups(void)
{
    asm volatile ("call ASM_unkn1_handle_agent_groups\n"
        :  :  : "eax" );
}

void init_game_controls(void)
{
#if 0
    asm volatile ("call ASM_init_game_controls\n"
        :  :  : "eax" );
#endif
    reset_user_input();

    init_user_input_local_controls();
}

void simulated_level(void)
{
    asm volatile ("call ASM_simulated_level\n"
        :  :  : "eax" );
}

void init_local_player_group(void)
{
    PlayerInfo *p_locplayer;
    struct Thing *p_agent;
    ushort grp;

    p_locplayer = &players[local_player_no];
    p_agent = p_locplayer->MyAgent[0];

    if (p_agent->Type == TT_PERSON) {
        grp = p_agent->U.UPerson.EffectiveGroup;
    } else {
        grp = 0;
    }
    ingame.MyGroup = grp;
}

void validate_player_double_mode(void)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    switch (playable_agents)
    {
    case 1:
        p_locplayer->DoubleMode = 0;
        break;
    case 2:
        if (p_locplayer->DoubleMode > 1)
            p_locplayer->DoubleMode = 1;
        break;
    case 3:
        if (p_locplayer->DoubleMode > 2)
              p_locplayer->DoubleMode = 2;
        break;
    default:
        break;
    }
}

void game_set_cam_starting_location_xz(void)
{
    MapCoord agent_x, agent_z;
    ThingIdx thing;

    {
        PlayerInfo *p_player;
        struct Thing *p_agent;

        p_player = &players[local_player_no];
        p_agent = p_player->MyAgent[0];
        if (p_agent->Type == TT_PERSON) {
            agent_x = PRCCOORD_TO_MAPCOORD(p_agent->X);
            agent_z = PRCCOORD_TO_MAPCOORD(p_agent->Z);
        } else {
            agent_x = MAP_COORD_WIDTH / 2;
            agent_z = MAP_COORD_HEIGHT / 2;
        }
    }
    engn_xc = MAP_COORD_WIDTH - agent_x;
    engn_zc = MAP_COORD_HEIGHT - agent_z;
    thing = 0;

    if (thing == 0)
    {
        MapCoord cor_x, cor_z;

        if (level_misc_get_starting_camera_pos(&cor_x, &cor_z))
        {
            engn_xc = cor_x;
            engn_zc = cor_z;
            thing = -1;
        }
    }

    if (thing == 0)
    {
        thing = search_things_for_type_farthest_from_xz(agent_x, agent_z, TT_BUILDING, SubTT_BLD_MGUN);
        if (thing > 0)
        {
            struct Thing *p_thing;

            p_thing = &things[thing];
            engn_xc = PRCCOORD_TO_MAPCOORD(p_thing->X);
            engn_zc = PRCCOORD_TO_MAPCOORD(p_thing->Z);
        }
    }
    if (thing == 0)
    {
        thing = search_things_for_type_farthest_from_xz(agent_x, agent_z, TT_BUILDING, SubTT_BLD_BILLBOARD);
        if (thing > 0)
        {
            struct Thing *p_thing;

            p_thing = &things[thing];
            engn_xc = PRCCOORD_TO_MAPCOORD(p_thing->X);
            engn_zc = PRCCOORD_TO_MAPCOORD(p_thing->Z);
        }
    }
}

TbBool game_cam_tracked_thing_is_player_agent(void)
{
    struct Thing *p_thing;

    p_thing = &things[ingame.TrackThing];
    return ((p_thing->Flag & TngF_PlayerAgent) != 0);
}

void game_set_cam_track_thing_xz(ThingIdx thing)
{
    short tng_x, tng_z;

    get_thing_position_mapcoords(&tng_x, NULL, &tng_z, thing);
    ingame.TrackX = tng_x;
    ingame.TrackZ = tng_z;
}

void game_set_cam_track_player_agent_xz(PlayerIdx plyr, ushort plagent)
{
    PlayerInfo *p_player;
    struct Thing *p_agent;

    p_player = &players[plyr];
    p_agent = p_player->MyAgent[plagent];
    if (p_agent->Type != TT_PERSON)
        return;
    game_set_cam_track_thing_xz(p_agent->ThingOffset);
}

void init_random_seed(void)
{
    if (in_network_game)
    {
        mission_open[1] = ingame.CurrentMission;
        mission_state[1] = MResol_UNDECIDED;
        mission_open[2] = 0;
        mission_state[2] = MResol_UNDECIDED;

        if (net_local_player_hosts_the_game()) {
            lbSeed = time(0);
        }
        net_immediate_random_seed_sync();
    }
    else
    {
        lbSeed = time(0);
    }
    srand(lbSeed);
}

void init_random_seed_default(void)
{
    lbSeed = 0xD15C1234;
    srand(lbSeed);
}

void preprogress_game_turns(void)
{
    struct Mission *p_missi;

    p_missi = &mission_list[ingame.CurrentMission];
    LOGSYNC("PreProcess %d turns for mission %d, starting at %lu",
      (int)p_missi->PreProcess, (int)ingame.CurrentMission, (ulong)gameturn);
    blind_progress_game(p_missi->PreProcess);
}

/** Initializes player presence on a level.
 *
 * CurrentMission needs to be set before this funcion is called.
 */
void init_player(void)
{
    gamep_unknval_10 = 0;
    gamep_unknval_12 = 0;
    nav_stats__ThisTurn = 0;
    ingame.Flags &= ~GamF_Unkn0100;
    ingame.Flags &= ~GamF_ThermalView;
    gamep_unknval_16 = 0;
    init_level_3d(0);
    init_level();
    if (in_network_game)
    {
        unkn1_handle_agent_groups();
        unkn_truce_groups();
    }
    else
    {
        place_single_player();
        game_set_cam_track_player_agent_xz(local_player_no, 0);
    }
    player_debug(" after place player");
    if (current_level == 0)
        simulated_level();
    player_debug(" after sim level");
    gameturn = 1;
    player_debug(" after group action");
    init_local_player_group();
    validate_player_double_mode();
    player_agents_init_prev_weapon(local_player_no);
    player_agents_clear_weapon_delays(local_player_no);

    init_game_controls();
    preprogress_game_turns();
}

/** Macro for returning given array of elements in random order.
 *
 * Provided input and output arrays, type of element and count of elements,
 * this routine will randomize the order of elements.
 */
#define array_elements_in_random_order(out_nubers, in_nubers, elem_type, count) \
{ \
    ushort pos, remain, next; \
    elem_type nums[count]; \
    memcpy(nums, in_nubers, sizeof(elem_type)*count); \
    remain = count; \
    for (pos = 0; pos < count; pos++) \
    { \
        elem_type nbak; \
        next = LbRandomAnyShort() % remain; \
        remain--; \
        out_nubers[pos] = nums[next]; \
        nbak = nums[remain]; \
        nums[remain] = nums[next]; \
        nums[next] = nbak; \
    } \
}

void randomize_playable_groups_order(void)
{
    static long incrementing_nubers[] = {0, 1, 2, 3, 4, 5, 6, 7,};
    array_elements_in_random_order(level_def.PlayableGroups, incrementing_nubers, long, 8);
}

void init_game(ubyte reload)
{
    ushort missi, next_mapno;
    short next_level;
    TbBool reload_map, reload_level;

    debug_trace_setup(1);
    missi = ingame.CurrentMission;
    next_mapno = mission_list[missi].MapNo;

    if (PacketRecord_IsPlayback()) {
        next_level = packet_rec_use_levelno;
    } else if ((reload) && (mission_list[missi].ReLevelNo != 0)) {
        next_level = mission_list[missi].ReLevelNo;
    } else {
        next_level = mission_list[missi].LevelNo;
    }

    // Whether we use the same map and the same level as currently loaded
    reload_map = (current_map == next_mapno) && reload;
    reload_level = (current_level == next_level) && reload;

    LOGSYNC("Init %s mission %hu on map %hu level %hd", in_network_game ? "MP" : "SP",
      missi, next_mapno, next_level);

    if (!reload_map)
        change_current_map(next_mapno);
    debug_trace_setup(0);

    if (next_level != 0)
        load_level_wrap(-next_level, missi, reload_level);
    else
        LOGWARN("Requested level %hd; load skipped", next_level);
    // The file name is formatted in original code, but doesn't seem to be used
    //sprintf(fname, "maps/map%03d.scn", next_mapno);
    debug_trace_setup(2);

    if (ingame.GameMode == GamM_None)
        ingame.GameMode = GamM_Unkn2;
    if (in_network_game) {
        randomize_playable_groups_order();
    }
    debug_trace_setup(3);
    init_player();
    if (!reload)
        game_set_cam_starting_location_xz();
    debug_trace_setup(4);
    execute_commands = 1;
    ingame.DisplayMode = DpM_ENGINEPLY;
    debug_trace_setup(5);
}

void prep_single_mission(void)
{
    load_missions(background_type);
    load_objectives_text();
    if (PacketRecord_IsPlayback())
    {
        PacketRecord_OpenRead();
        packet_read_whole_player_init();
    }
    init_game(0);
    if (!in_network_game)
    {
        if (PacketRecord_IsRecord()) {
            PacketRecord_OpenWrite();
            packet_write_whole_player_init();
        }
    }
    prep_multicolor_sprites();
    LbScreenClear(0);
    generate_shadows_for_multicolor_sprites();
    adjust_mission_engine_to_video_mode();
}

void restart_back_into_mission(ushort missi)
{
    ushort mapno;
    ubyte reload;

    reload = (missi == ingame.CurrentMission);
    mapno = mission_list[missi].MapNo;
    mission_result = 0;
    ingame.MissionEndFade = 0;
    ingame.CurrentMission = missi;
    mission_list[missi].Complete = 0;
    change_current_map(mapno);
    map_lights_update();
    if (ingame.GameMode == GamM_Unkn2)
        execute_commands = 0;
    engn_yc = 0;
    init_game(reload);
    init_random_seed_default();
    if (!in_network_game)
    {
        if (PacketRecord_IsRecord()) {
            PacketRecord_Close();
            PacketRecord_OpenWrite();
            packet_write_whole_player_init();
        }
    }
}

void compound_mission_brief_store_next(void)
{
    ushort old_missi, new_missi;

    old_missi = ingame.CurrentMission;
    new_missi = mission_list[old_missi].SuccessTrigger[0];
    brief_store[open_brief - 1].Mission = new_missi;
    replace_mission_state_slot(old_missi, new_missi);
}

void compound_mission_immediate_start_next(void)
{
    ushort missi;
    missi = brief_store[open_brief - 1].Mission;
    restart_back_into_mission(missi);
}

short test_single_mission(short missi)
{
    short ret;
    asm volatile ("call ASM_test_single_mission\n"
        : "=r" (ret) : "a" (missi));
    return ret;
}

void post_process_blips(void)
{
    asm volatile ("call ASM_post_process_blips\n"
        :  :  : "eax" );
}

short test_missions(ubyte flag)
{
#if 0
    short ret;
    asm volatile ("call ASM_test_missions\n"
        : "=r" (ret) : "a" (flag));
    return ret;
#endif
    ushort mslot;
    short missi;
    short res;

    if (flag == 0 && ingame.TrackThing)
        return 0;

    if (mission_open[1] == 0)
        mission_open[1] = ingame.CurrentMission;

    if (flag == 0) {
        draw_objective(flag, 1);
        add_signal_to_scanner(0, 1);
    }

    if (flag != 0) {
        set_mission_state_using_state_slot(ingame.CurrentMission, MResol_UNDECIDED);
        return 0;
    }

    if (in_network_game) {
        mslot = 1;
        mission_open[mslot] = ingame.CurrentMission;
    } else {
        mslot = find_mission_state_slot(ingame.CurrentMission);
        if (mslot == 0) {
            mslot = find_empty_mission_state_slot();
        }
    }

    if (mission_open[mslot] > next_mission)
        mission_open[mslot] = 1;
    if (mission_state[mslot] == MResol_FAILED)
    {
        screen_objective_text_set_failed();
        return -1;
    }
    missi = mission_open[mslot];
    mission_state[mslot] = MResol_UNDECIDED;

    res = test_single_mission(missi);
    if (ingame.CurrentMission == mission_open[mslot])
    {
        if (mission_result == 1)
            res = 1;
        if (mission_result == -1)
            res = -1;
    }
    if (res == 0)
    {
        short i;

        res = -1;
        for (i = 0; i < playable_agents; i++)
        {
            PlayerInfo *p_locplayer;
            struct Thing *p_agent;

            p_locplayer = &players[local_player_no];
            p_agent = p_locplayer->MyAgent[i];
            if ((p_agent->Type == TT_PERSON) && (p_agent->State != PerSt_DEAD)) {
                res = 0;
                break;
            }
        }
    }
    if (res > 0)
    {
        mission_state[mslot] = MResol_COMPLETED;
        return 1;
    }
    if (res < 0)
    {
        mission_state[mslot] = MResol_FAILED;
        screen_objective_text_set_failed();
        return -1;
    }
    post_process_blips();
    return 0;
}

void create_tables_file_from_fade(void)
{
    long len;
    int i, k;
    unsigned char *curbuf;
    // Note that the input file is not normally available with the game
    len = LbFileLoadAt("data/fade.dat", scratch_buf1);
    if (len == Lb_FAIL) {
        LOGERR("Cannot find input file for tables update");
        return;
    }
    curbuf = scratch_buf1;
    for (i = 0; i < 256; i++) {
        for (k = 0; k < 64; k++) {
          pixmap.fade_table[256 * k + i] = *(curbuf + (256+64) * k + i);
        }
    }
    LbFileSaveAt("data/tables.dat", &pixmap, sizeof(pixmap));
}

/** Universal way to generate the colour tables.
 */
void create_tables_file_from_palette(void)
{
    LbColourTablesGenerate(display_palette, fade_unaffected_colours,
      "data/tables.dat");
}

/** Shows a screen with startup fail message.
 *
 * This special screen requires zero data files, so it can shown even
 * if the data load has failed. It is supposed to be the last information
 * message before the game terminates.
 */
void game_startup_fail_screen(void)
{
    ubyte vga_pal[PALETTE_8b_SIZE];
    TbBool was_locked;
    int i, step;
    short cent_x, cent_y, circle_diam;
    short base_x, base_y, embeded_square_dim;
    TbPixel col_grey1, col_grey2, col_white, col_blue1;

    was_locked = LbScreenIsLocked();
    if (!was_locked) {
        while (LbScreenLock() != Lb_SUCCESS)
            ;
    }

    if (lbDisplay.WScreen == NULL) {
        if (!was_locked)
            LbScreenUnlock();
        return;
    }

    lbDisplay.DrawFlags = 0;
    LbPaletteDataFillDefVGA(vga_pal);
    LbPaletteSet(vga_pal);
    LbScreenClear(0);

    col_white = LbPaletteFindColour(vga_pal, 63, 63, 63);
    col_grey2 = LbPaletteFindColour(vga_pal, 32, 32, 32);
    col_grey1 = LbPaletteFindColour(vga_pal, 16, 16, 16);
    col_blue1 = LbPaletteFindColour(vga_pal,  0,  0, 16);

    cent_x = lbDisplay.GraphicsScreenWidth / 2;
    cent_y = lbDisplay.GraphicsScreenHeight / 2;
    circle_diam = min(lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);

    step = circle_diam / 20;
    base_x = ((lbDisplay.GraphicsScreenWidth % step) / 2) - circle_diam*1/256;
    base_y = ((lbDisplay.GraphicsScreenHeight % step) / 2) - circle_diam*1/256;
    for (i = 0; i < lbDisplay.GraphicsScreenWidth; i += step) {
        LbDrawBox(base_x + i, 0, circle_diam*1/128, lbDisplay.GraphicsScreenHeight, col_grey2);
    }
    for (i = 0; i < lbDisplay.GraphicsScreenHeight; i += step) {
        LbDrawBox(0, base_y + i, lbDisplay.GraphicsScreenWidth, circle_diam*1/128, col_grey2);
    }

    LbDrawCircle(cent_x, cent_y, circle_diam*32/64, col_grey1);
    LbDrawCircle(cent_x, cent_y, circle_diam*31/64, col_blue1);

    embeded_square_dim = circle_diam * 128 / 181; // circle_diam divided by sqrt(2)

    LbPaletteDraw(cent_x - embeded_square_dim/2, cent_y - embeded_square_dim/2, embeded_square_dim/16, embeded_square_dim/16, 0);
    LbDrawBox(cent_x - embeded_square_dim/6, cent_y - circle_diam*2/64, embeded_square_dim * 2 / 6, circle_diam*5/64, 0);

    draw_text(cent_x - 40, cent_y - circle_diam*1/64, "U.T.O.P.I.A.", col_white);
    draw_text(cent_x - 40, cent_y - circle_diam*0/64, "Game startup failed!", col_white);
    draw_text(cent_x - 40, cent_y + circle_diam*1/64, "Check \"error.log\".", col_white);

    if (!was_locked)
        LbScreenUnlock();
    swap_wscreen();

    for (i = 5*game_num_fps; i > 0; i--)
    {
        game_update();
    }
}

TbBool game_setup(void)
{
    TbBool ret;

    ret = true;
    engine_mem_alloc_ptr = LbMemoryAlloc(engine_mem_alloc_size);
    if (engine_mem_alloc_ptr == NULL) {
        LOGERR("Allocating engine memory failed.");
        ret = false;
    }
    load_texturemaps();
    if (LbDataLoadAll(missionspr_load_files) != 0) {
        LOGERR("Initial loading of mission sprite files failed.");
        ret = false;
    }
    if (!read_rules_file()) {
        LOGERR("Rules config file loading failed.");
        ret = false;
    }
    read_weapons_conf_file();
    read_cybmods_conf_file();
    bang_init();
    init_free_explode_faces();
    init_search_spiral();
    bang_set_detail(0);
    FIRE_init_or_samples_init();
    ingame.draw_unknprop_01 = 0;
    debug_trace_setup(-5);
    if (!game_setup_stuff()) {
        LOGERR("Setting up some misc stuff failed.");
        ret = false;
    }
    if (!create_strings_list(gui_strings, gui_strings_data, gui_strings_data_end)) {
        LOGERR("National strings list creation failed.");
        ret = false;
    }
    if (!setup_host())
        ret = false;
    debug_trace_setup(-4);
    read_user_settings();
    debug_trace_setup(-3);
    setup_color_lookups();
    init_things();
    debug_trace_setup(-2);
    setup_sprites_small_font();
    load_peep_type_stats();
    load_campaigns();
    player_mission_agents_reset(local_player_no);
    debug_trace_setup(-1);
    if ( is_single_game || cmdln_param_bcg )
    {
        load_prim_quad();
    }
    debug_trace_setup(0);
    init_engine();
    if ( !cmdln_param_bcg )
    {
        LbScreenClear(0);
        swap_wscreen();
        LbPaletteSet(display_palette);
    }
    test_open(15);
    debug_trace_setup(1);
    if (is_single_game && (ingame.CurrentMission != 0) && ret)
    {
        prep_single_mission();
    }
    if (in_network_game || cmdln_param_bcg) {
        ingame.DisplayMode = DpM_PURPLEMNU;
        reload_menu_flags |= RelMnuF_ColorsSprites;
    }
    debug_trace_setup(2);
    switch (cmdln_colour_tables)
    {
    case 1:
        create_tables_file_from_palette();
        break;
    case 2:
        create_tables_file_from_fade();
        break;
    }
    if (!ret) {
        LOGERR("Part of the game startup has failed. See the log lines above for a cause.");
        game_startup_fail_screen();
    }
    return ret;
}

void anim_show_draw_next_frame(struct Animation *p_anim)
{
    ubyte pal_change;

    pal_change = anim_show_frame(p_anim);
    p_anim->FrameNumber++;

    if (pal_change)
    {
        LbScreenWaitVbi();
        if (byte_1AAA88) {
            LbPaletteSet(anim_palette);
        }
    }
}

int xdo_next_frame(ubyte anislot)
{
    struct Animation *p_anim;
    ushort k;

    k = anim_slots[anislot];
    active_anim = k;
    p_anim = &animations[k];

    if (anislot >= AniSl_EQVIEW && anislot <= AniSl_CYBORG_INOUT)
    {
        if (p_anim->FrameNumber == 0) {
            play_sample_using_heap(0, 135, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        } else if (p_anim->FrameNumber == p_anim->FLCFileHeader.NumberOfFrames >> 1) {
            play_sample_using_heap(0, 115, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        }
    }

    if (p_anim->FrameNumber >= p_anim->FLCFileHeader.NumberOfFrames)
    {
        anim_flic_close(p_anim);
        if ((p_anim->Flags & 0x20) != 0) {
            flic_unkn03(p_anim->Type);
        }
        return 1;
    }

    anim_show_prep_next_frame(p_anim, anim_type_get_output_buffer(p_anim->Type));
    anim_show_draw_next_frame(p_anim);

    return 0;
}

int xdo_prev_frame(ubyte anislot)
{
    struct Animation *p_anim;
    ubyte *p_frmbuf;
    uint i, rq_frame;
    ushort k;

    k = anim_slots[anislot];
    active_anim = k;
    p_anim = &animations[k];

    if (p_anim->FrameNumber == 0)
        rq_frame = p_anim->FLCFileHeader.NumberOfFrames;
    else
        rq_frame = p_anim->FrameNumber - 1;

    p_frmbuf = anim_type_get_output_buffer(p_anim->Type);

    if (rq_frame == 0)
    {
        LbMemorySet(p_frmbuf, 0, p_anim->FLCFileHeader.Width * p_anim->FLCFileHeader.Height);
        anim_flic_close(p_anim);
        if ((p_anim->Flags & 0x20) != 0) {
            flic_unkn03(p_anim->Type);
        }
        return 1;
    }

    anim_flic_show_replay(p_anim);
    LbMemorySet(p_frmbuf, 0, p_anim->FLCFileHeader.Width * p_anim->FLCFileHeader.Height);
    anim_show_prep_next_frame(p_anim, p_frmbuf);
    anim_show_draw_next_frame(p_anim);
    for (i = 1; i < rq_frame; i++)
    {
        anim_show_prep_next_frame(p_anim, NULL);
        anim_show_draw_next_frame(p_anim);
    }
    return 0;
}

void mapwho_unkn01(int a1, int a2)
{
    asm volatile ("call ASM_mapwho_unkn01\n"
        : : "a" (a1), "d" (a2));
}

void show_unkn3A_screen(int a1)
{
    // Empty
}

void compute_scanner_zoom(void)
{
    short zoom, scmin, scmax;

    if (ingame.Scanner.X2 > ingame.Scanner.X1)
        zoom = SCANNER_base_zoom_factor * 128 / (ingame.Scanner.X2 - ingame.Scanner.X1);
    else
        zoom = SCANNER_base_zoom_factor;
    scmin = get_overall_scale_min();
    scmax = get_overall_scale_max();
    if (scmax <= scmin)
        scmin = scmax - 1;
    zoom += SCANNER_user_zoom_factor * (user_zoom_max - get_unscaled_zoom(overall_scale)) / (scmax - scmin);
    SCANNER_set_zoom(zoom);
}

void show_game_engine(void)
{
    PlayerInfo *p_locplayer;
    short dcthing;

    p_locplayer = &players[local_player_no];
    dcthing = p_locplayer->DirectControl[0];
    process_view_inputs(dcthing);// inlined call gengine_ctrl

    compute_scanner_zoom();
    process_engine_unk1();
    process_engine_unk2();
    process_engine_unk3();
    setup_engine_nullsub4();
}

void gproc3_unknsub2(void)
{
#if 0
    asm volatile ("call ASM_gproc3_unknsub2\n"
        :  :  : "eax" );
    return;
#endif
    short ms_x, ms_y;
    int i;

    int bkp_ingame_flags;
    int long bkp_engn_anglexz;
    ushort bkp_render_area_a, bkp_render_area_b;
    long bkp_dword_152EEC;
    ubyte bkp_unkn_flags_01;
    ushort bkp_overall_scale;
    s32 bkp_engn_xc, bkp_engn_yc, bkp_engn_zc;

    ingame.Flags &= ~GamF_BillboardMovies;
    if (is_key_pressed(KC_Q, KMod_DONTCARE))
    {
        dword_155010 = 0x4000;
        dword_155014 = 0x4000;
        dword_1AAB74 = 0;
        dword_1AAB78 = 0;
        dword_155018 = 50;
    }

    bkp_render_area_a = render_area_a;
    bkp_render_area_b = render_area_b;
    bkp_overall_scale = overall_scale;
    bkp_engn_xc = engn_xc;
    bkp_engn_yc = engn_yc;
    bkp_engn_zc = engn_zc;
    bkp_engn_anglexz = engn_anglexz;
    bkp_ingame_flags = ingame.Flags;
    bkp_dword_152EEC = dword_152EEC;
    bkp_unkn_flags_01 = unkn_flags_01;

    render_area_a = 24;
    render_area_b = 24;
    ingame.Flags = 0;

    ms_x = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseX : lbDisplay.MMouseX;
    ms_y = lbDisplay.GraphicsScreenHeight < 400 ? 2 * lbDisplay.MMouseY : lbDisplay.MMouseY;

    if (ms_x < 200)
      dword_1AAB74 -= 16;
    if (ms_x > 440)
      dword_1AAB74 += 16;
    dword_1AAB74 &= 0x7FF;

    if (!lbDisplay.MRightButton)
    {
        dword_155010 += lbSinTable[dword_1AAB74] >> 9;
        dword_155014 += lbSinTable[dword_1AAB74 + LbFPMath_PI/2] >> 9;
    }

    if (ms_y < 180)
        dword_1AAB78 -= (180 - ms_y) >> 5;
    if (ms_y > 220)
        dword_1AAB78 -= (220 - ms_y) >> 5;
    if (dword_1AAB78 > 300)
        dword_1AAB78 = 300;
    if (dword_1AAB78 < -300)
        dword_1AAB78 = -300;

    i = (alt_at_point(dword_155010, dword_155014) >> 8) - 50;
    if (i > dword_155018)
        dword_155018 = i;

    if (dword_155010 < 0)
        dword_155010 = 0x8000;
    if (dword_155010 > 0x8000)
        dword_155010 = 0;

    if (dword_155014 < 0)
        dword_155014 = 0x8000;
    if (dword_155014 > 0x8000)
        dword_155014 = 0;

    dword_152EEC = dword_1AAB78;
    engn_xc = dword_155010;
    engn_yc = dword_155018;
    engn_zc = dword_155014;
    engn_anglexz = 32 * dword_1AAB74;

    setup_vecs(vec_tmap[5], vec_tmap[0], 0x100u, 0x60u, 64);
    process_engine_unk1();

    unkn_flags_01 = 1;
    overall_scale = 18;
    memset(vec_tmap[5], 0, 0x4000);
    gameturn -= 10;
    func_2e440();
    gameturn += 10;

    setup_vecs(lbDisplay.WScreen, vec_tmap[0],
      lbDisplay.PhysicalScreenWidth,
      lbDisplay.PhysicalScreenWidth,
      lbDisplay.PhysicalScreenHeight);
    dword_176D3C = vec_window_width / 2;
    dword_176D40 = vec_window_height / 2;

    render_area_a = bkp_render_area_a;
    render_area_b = bkp_render_area_b;
    overall_scale = bkp_overall_scale;
    engn_xc = bkp_engn_xc;
    engn_yc = bkp_engn_yc;
    engn_zc = bkp_engn_zc;
    engn_anglexz = bkp_engn_anglexz;
    ingame.Flags = bkp_ingame_flags;
    dword_152EEC = bkp_dword_152EEC;
    unkn_flags_01 = bkp_unkn_flags_01;

    process_engine_unk1();
}

ubyte accept_mission(ubyte click)
{
    ubyte ret;
    asm volatile ("call ASM_accept_mission\n"
        : "=r" (ret) : "a" (click));
    return ret;
}

ubyte do_unkn1_CANCEL(ubyte click)
{
    ubyte ret;
    asm volatile ("call ASM_do_unkn1_CANCEL\n"
        : "=r" (ret) : "a" (click));
    return ret;
}

ubyte load_game_slot(ubyte click)
{
    char *ldname;
    int ldslot;
    int ret;

    if (login_control__State != LognCt_Unkn6) {
        return 0;
    }
    if (save_slot == -1) {
        alert_box_text_fmt("%s", gui_strings[564]);
        return 1;
    }
    if (save_slot != 0) {
        ldname = save_slot_names[save_slot - save_slot_base - 1];
        ldslot = save_slot;
    } else {
        ldname = save_active_desc;
        ldslot = 0;
    }

    ret = load_game(ldslot, ldname);
    if (ret == 1) {
        alert_box_text_fmt("%s", gui_strings[565]);
        return 1;
    } else if (ret == 2) {
        alert_box_text_fmt("%s", gui_strings[567]);
        return 1;
    }

    alert_box_text_fmt("%s", gui_strings[572]);

    // Reading the save might have caused campaign switch
    load_objectives_text();
    init_weapon_text();

    mark_system_menu_screen_boxes_redraw();
    mark_sys_scr_shared_header_box_redraw();
    mark_storage_screen_boxes_redraw();
    if (save_slot == 0) {
        ingame.Flags |= GamF_MortalGame;
    }
    reset_world_screen_player_state();
    selected_agent = 0;
    screentype = SCRT_99;
    game_system_screen = SySc_NONE;
    reload_background_flag = 1;
    if (restore_savegame) {
        restore_savegame = 0;
        sysmnu_button_enable(0, 5);
    }
    if ((ingame.Flags & GamF_MortalGame) != 0) {
        sysmnu_button_disable(1,2);
    }
    return 1;
}

ubyte save_game_slot(ubyte click)
{
    ubyte ret;
    asm volatile ("call ASM_save_game_slot\n"
        : "=r" (ret) : "a" (click));
    return ret;
}

void init_variables(void)
{
#if 0
    asm volatile ("call ASM_init_variables\n"
        :  :  : "eax" );
#endif
    selected_city_id = -1;
    reset_equip_screen_player_state();
    reset_cryo_screen_player_state();
    reset_world_screen_player_state();
    reset_brief_screen_player_state();
    reset_research_screen_player_state();
    clear_all_scanner_signals();
    reset_app_bar_player_state();
    //word_1C6F48 = 0; -- set but never used - remove pending
    global_date.Day = 2;
    global_date.Month = 6;
    global_date.Year = 74;
    //word_15518A = -1; -- set but never used - remove pending
    ingame.MissionStatus = ObvStatu_COMPLETED;
    login_control__Money = starting_cash_amounts[4];
    if (login_control__State == LognCt_Unkn6)
    {
        ingame.Credits = 50000;
        ingame.CashAtStart = 50000;
    }
    else
    {
        ingame.Credits = login_control__Money;
        ingame.CashAtStart = login_control__Money;
    }
    ingame.Expenditure = 0;
    login_control__City = 19;
    login_control__State = LognCt_Unkn6;
    byte_181189 = 0;
    net_game_play_flags = NGPF_Unkn20 | NGPF_Unkn10 | NGPF_Unkn08 | NGPF_Unkn04;
    login_control__TechLevel = 4;
}

void init_agents(void)
{
    asm volatile ("call ASM_init_agents\n"
        :  :  : "eax" );
}

/** Initializes the research data for a new game.
 */
void srm_reset_research(void)
{
    struct Campaign *p_campgn;
    int i;

    p_campgn = &campaigns[background_type];

    for (i = 0; i < 32; i++)
    {
        research.WeaponDaysDone[i] = 0;
        research.ModDaysDone[i] = 0;
        research.WeaponProgress[i][0] = 0;
        research.ModProgress[i][0] = 0;
    }
    research.WeaponsCompleted = p_campgn->StandardWeapons;
    research.WeaponsAllowed = p_campgn->ResearchWeapons;
    research.ModsCompleted = p_campgn->StandardMods;
    research.ModsAllowed = p_campgn->ResearchMods;
    research.CurrentWeapon = -1;
    research.CurrentMod = -1;
    research.Scientists = 0;
    research.NumBases = 0;
}

void my_preprocess_text(char *text)
{
    asm volatile ("call ASM_my_preprocess_text\n"
        :  : "a" (text));
}

void research_unkn_func_006(ushort missi)
{
    struct Mission *p_missi;
    int i;

    p_missi = &mission_list[missi];

    if (p_missi->ResearchWeapons != 0) {
        research_weapon_flags_allow(p_missi->ResearchWeapons);
    }

    switch (p_missi->ExtraRewardType)
    {
    case MEReward_ResearchLab:
        research.NumBases++;
        // Fall through
    case MEReward_Scientists:
        research.Scientists += p_missi->ExtraRewardParam;
        break;
    case MEReward_CybModResearched:
        research_cymod_complete(p_missi->ExtraRewardParam);
        break;
    case MEReward_CybModSingle:
        for (i = 0; i < cryo_agents.NumAgents; i++)
        {
            TbBool added;

            added = add_mod_to_flags_no_replace(&cryo_agents.Mods[i], p_missi->ExtraRewardParam);
            if (!added)
                continue;

            if (i < 4) {
                PlayerInfo *p_locplayer;
                p_locplayer = &players[local_player_no];
                add_mod_to_flags(&p_locplayer->Mods[i], p_missi->ExtraRewardParam);
            }
            break;
        }
        break;
    case MEReward_WeaponResearched:
        research_weapon_complete(p_missi->ExtraRewardParam);
        break;
    case MEReward_WeaponSingle:
        for (i = 0; i < cryo_agents.NumAgents; i++)
        {
            TbBool added;

            added = weapons_add_one(&cryo_agents.Weapons[i], &cryo_agents.FourPacks[i], p_missi->ExtraRewardParam);
            if (!added)
                continue;

            /* TODO re-ebable when player->FourPacks is unified and in the same format as cryo_agents.FourPacks
            if (i < 4) {
                PlayerInfo *p_locplayer;
                p_locplayer = &players[local_player_no];
                weapons_add_one(&p_locplayer->Weapons[i], &p_player->FourPacks[i], p_missi->ExtraRewardParam);
            }*/
            break;
        }
        break;
    }
}

void hide_weapons_for_agents_from_player(PlayerInfo *p_locplayer)
{
    struct Thing *p_agent;
    short i;
    for (i = 0; i < playable_agents; i++)
    {
        p_agent = p_locplayer->MyAgent[i];

        if (p_agent->Type != TT_PERSON) continue;
        if ((p_agent->Flag & TngF_Destroyed) != 0) continue;

        p_agent->U.UPerson.CurrentWeapon = WEP_NULL;
        process_clone_disguise(p_agent);
    }
}

void mission_over_prepare_agents(void)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    hide_weapons_for_agents_from_player(p_locplayer);
}

void mission_over_update_players(void)
{
    PlayerInfo *p_locplayer;

    p_locplayer = &players[local_player_no];
    cryo_update_agents_from_player(p_locplayer);
}

ulong calculate_cash_gain_from_persuaded_person(struct Thing *p_person)
{
    ulong credits;

    credits = 0;
    switch (p_person->SubType)
    {
    case SubTT_PERS_AGENT:
        credits += 1000;
        break;
    case SubTT_PERS_ZEALOT:
        credits += 1000;
        break;
    case SubTT_PERS_PUNK_M:
    case SubTT_PERS_PUNK_F:
        credits += 150;
        break;
    case SubTT_PERS_SCIENTIST:
        credits += 500;
        break;
    default:
        credits += 100;
        break;
    }
    credits += person_carried_weapons_pesuaded_sell_value(p_person);

    return credits;
}

ulong mission_over_calculate_cash_gain_from_persuaded_crowd(ushort tgroup)
{
    ulong credits;
    struct Thing *p_person;
    ThingIdx person;

    credits = 0;
    person = get_thing_same_type_head(TT_PERSON, -1);
    for (; person > 0; person = p_person->LinkSame)
    {
        p_person = &things[person];
        if ((p_person->Flag & TngF_Persuaded) == 0)
            continue;
        if (p_person->U.UPerson.EffectiveGroup != tgroup)
            continue;
        credits += calculate_cash_gain_from_persuaded_person(p_person);
    }
    return credits;
}

void mission_over_gain_personnel_from_persuaded_crowd(void)
{
    struct Thing *p_person;
    ThingIdx person;

    person = get_thing_same_type_head(TT_PERSON, -1);
    for (; person > 0; person = p_person->LinkSame)
    {
        p_person = &things[person];
        if ((p_person->Flag & TngF_Persuaded) == 0)
            continue;
        if (p_person->U.UPerson.EffectiveGroup != ingame.MyGroup)
            continue;
        switch (p_person->SubType)
        {
        case SubTT_PERS_AGENT:
            add_agent(p_person->U.UPerson.WeaponsCarried, p_person->U.UPerson.UMod.Mods);
            break;
        case SubTT_PERS_SCIENTIST:
            research.Scientists++;
            break;
        }
    }
}

void mission_over_gain_persuaded_crowd_rewards(void)
{
    ingame.Credits += mission_over_calculate_cash_gain_from_persuaded_crowd(ingame.MyGroup);
    mission_over_gain_personnel_from_persuaded_crowd();
}

ulong mission_over_calculate_player_cash_gain_from_items(void)
{
    ulong credits;
    short sthing;
    struct SimpleThing *p_sthing;

    credits = 0;
    for (sthing = sthings_used_head; sthing != 0; sthing = p_sthing->LinkChild)
    {
        p_sthing = &sthings[sthing];
        if (p_sthing->Type == SmTT_CARRIED_ITEM)
        {
            struct Thing *p_owntng;
            p_owntng = &things[p_sthing->U.UWeapon.Owner];
            // Item carried by non-persuaded player agents
            if (((p_owntng->Flag & TngF_PlayerAgent) != 0) &&
              ((p_owntng->Flag & TngF_Persuaded) == 0))
            {
                if (p_sthing->U.UWeapon.WeaponType == 0)
                    credits += 100 * p_sthing->U.UWeapon.Ammo;
                LOGSYNC("Carried item thing %hd picked by the player", sthing);
            }
            // Item carried by people persuaded by the player
            else if (((p_owntng->Flag & TngF_Persuaded) != 0) &&
              (p_owntng->U.UPerson.EffectiveGroup == ingame.MyGroup))
            {
                if (p_sthing->U.UWeapon.WeaponType == 0)
                    credits += 100 * p_sthing->U.UWeapon.Ammo;
                LOGSYNC("Carried item thing %hd picked by persuadee", sthing);
            }
            else
            {
                LOGSYNC("Carried item thing %hd not under player control", sthing);
            }
        }
    }
    return credits;
}

void update_player_cash(void)
{
    ulong credits;

    credits = mission_over_calculate_player_cash_gain_from_items();
    ingame.Credits += credits;
}

void do_start_triggers(short missi)
{
    short nxmissi, sptrig;

    for (nxmissi = missi; 1; nxmissi = sptrig)
    {
        if (true)
            sptrig = mission_list[nxmissi].SpecialTrigger[0];
        if (sptrig == 0)
            sptrig = mission_list[nxmissi].SpecialTrigger[1];
        if (sptrig == 0)
            break;
        set_mission_state_using_state_slot(sptrig, MResol_UNDECIDED);
    }
}

ushort open_new_mission(short missi)
{
    if (mission_has_immediate_previous(missi)) {
        LOGSYNC("No slot needed for mission %d", (int)missi);
        return 0;
    }

    set_mission_state_using_state_slot(missi, MResol_UNDECIDED);
    do_start_triggers(missi);
    queue_up_new_mail(1, missi);

    return 0;
}

TbBool check_mission_conds(ushort missi)
{
    int i;
    ushort cmissi;

    for (i = 0; i < 5; i++)
    {
        cmissi = mission_list[missi].MissionCond[i];
        if ((cmissi > 0) && (mission_list[cmissi].Complete != 1))
          return false;
    }
    return true;
}

TbBool mission_has_no_special_triggers(ushort missi)
{
    struct Mission *p_missi;
    int i;

    p_missi = &mission_list[missi];

    for (i = 0; i < 3; i++)
    {
        if (p_missi->SpecialTrigger[i] != 0)
          return false;
    }
    return true;
}

TbBool mission_special_triggers_2_is_self(ushort missi)
{
    struct Mission *p_missi;

    p_missi = &mission_list[missi];

    return (p_missi->SpecialTrigger[2] == missi);
}

void mission_reset_spec_triggers_2_chain(ushort missi)
{
    ushort next_missi;

    next_missi = missi;
    while (next_missi != 0)
    {
        ushort tmp_missi;

        LOGSYNC("Setting mission=%d owner to mission=%d", (int)next_missi, (int)missi);
        mission_list[next_missi].SpecialTrigger[2] = missi;

        // TODO Why only one these? If replacing, shouldn't we replace both?
        tmp_missi = mission_list[next_missi].SpecialTrigger[0];
        if (tmp_missi == 0)
            tmp_missi = mission_list[next_missi].SpecialTrigger[1];
        next_missi = tmp_missi;
    }
}

void mission_special_triggers_0_1_set_fail(ushort missi)
{
    ushort next_missi;

    next_missi = missi;
    while (next_missi > 0)
    {
        ushort tmp_missi;

        mission_list[next_missi].Complete = -1;

        // TODO Why only one these? If failing, shouldn't we fail both?
        tmp_missi = mission_list[next_missi].SpecialTrigger[0];
        if (tmp_missi == 0)
            tmp_missi = mission_list[next_missi].SpecialTrigger[1];
        next_missi = tmp_missi;
    }
}

void mission_copy_conds_and_succ_fail_triggers(ushort dst_missi, ushort src_missi)
{
    int i;

    LOGSYNC("Copying mission=%d props to mission=%d", (int)src_missi, (int)dst_missi);
    for (i = 0; i < 5; i++) {
        mission_list[dst_missi].MissionCond[i] = mission_list[src_missi].MissionCond[i];
    }
    for (i = 0; i < 3; i++) {
        mission_list[dst_missi].SuccessTrigger[i] = mission_list[src_missi].SuccessTrigger[i];
        mission_list[dst_missi].FailTrigger[i] = mission_list[src_missi].FailTrigger[i];
    }
}

ushort mission_fire_success_triggers(ushort missi)
{
    struct Mission *p_missi;
    ushort n;
    int i;

    p_missi = &mission_list[missi];

    n = 0;
    for (i = 0; i < 3; i++)
    {
        ushort new_missi;
        new_missi = p_missi->SuccessTrigger[i];
        if (new_missi != 0) {
            open_new_mission(new_missi);
            n++;
        }
    }
    return n;
}

ushort mission_fire_fail_triggers(ushort missi)
{
    struct Mission *p_missi;
    ushort n;
    int i;

    p_missi = &mission_list[missi];

    n = 0;
    for (i = 0; i < 3; i++)
    {
        ushort new_missi;
        new_missi = p_missi->FailTrigger[i];
        if (new_missi != 0) {
            open_new_mission(new_missi);
            n++;
        }
    }
    return n;
}

void update_mission_list_to_mission_state(ushort missi, sbyte state)
{
    if (state == MResol_COMPLETED) {
        mission_list[missi].Complete = state;
    } else if (mission_remain_until_success(missi)) {
          mission_list[missi].Complete = 0;
          set_mission_state_using_state_slot(missi, MResol_UNDECIDED);
    } else {
          mission_list[missi].Complete = state;
    }
}

ubyte check_open_next_mission(ushort missi, sbyte state)
{
    if (mission_has_no_special_triggers(missi))
    {
        LOGSYNC("SpecialTriggers none, mission=%d, state=%d",
          (int)missi, (int)state);
        if (state == MResol_COMPLETED)
        {
            if (mission_has_immediate_next_on_success(missi))
                return OMiSta_ContImmSuccess;
            else
                return OMiSta_EndSuccess;
        }
        else if (state == MResol_FAILED)
        {
            return OMiSta_EndFailed;
        }
    }
    else if (mission_special_triggers_2_is_self(missi))
    {
        ushort trg_missi;

        trg_missi = mission_list[missi].SpecialTrigger[0];
        if (trg_missi == 0)
            trg_missi = mission_list[missi].SpecialTrigger[1];
        if (trg_missi != 0)
        {
            LOGSYNC("SpecialTriggers self-owned and set, mission=%d, state=%d",
              (int)missi, (int)state);
            if (state == MResol_COMPLETED)
            {
                brief_store[open_brief - 1].Mission = trg_missi;

                mission_copy_conds_and_succ_fail_triggers(trg_missi, missi);
                mission_reset_spec_triggers_2_chain(trg_missi);

                ingame.MissionStatus = ObvStatu_UNDECIDED;

                return OMiSta_ContSuccess;
            }
            else if (state == MResol_FAILED)
            {
                if (mission_remain_until_success(missi))
                {
                    ingame.MissionStatus = ObvStatu_UNDECIDED;

                    return OMiSta_ContFailed;
                }
                else
                {
                    mission_special_triggers_0_1_set_fail(trg_missi);

                    return OMiSta_EndFailed;
                }
            }
        }
        else
        {
            LOGSYNC("SpecialTriggers self-owned but unset, mission=%d, state=%d",
              (int)missi, (int)state);
            if (state == MResol_COMPLETED)
            {
                if (mission_has_immediate_next_on_success(missi))
                    return OMiSta_ContImmSuccess;
                else
                    return OMiSta_EndSuccess;
            }
            else if (state == MResol_FAILED)
            {
                if (mission_remain_until_success(missi))
                {
                    ingame.MissionStatus = ObvStatu_UNDECIDED;

                    return OMiSta_ContFailed;
                }
                else
                {
                    return OMiSta_EndFailed;
                }
            }
        }
    }
    else
    {
        ushort trg_missi;

        trg_missi = mission_list[missi].SpecialTrigger[2];

        LOGSYNC("SpecialTriggers owner=%d, mission=%d, state=%d", (int)trg_missi, (int)missi, (int)state);
        if (state == MResol_COMPLETED)
        {
            ushort tmp_missi, tmp2_missi, next_missi;

            next_missi = trg_missi;
            while (1)
            {
                tmp_missi = mission_list[next_missi].SpecialTrigger[0];
                if ((tmp_missi == missi) || (next_missi == 0))
                    break;
                next_missi = tmp_missi;
            }
            if (next_missi > 0)
            {
                tmp_missi = mission_list[next_missi].SpecialTrigger[0];
                tmp2_missi = mission_list[tmp_missi].SpecialTrigger[0];
                if (tmp2_missi > 0)
                    mission_list[next_missi].SpecialTrigger[0] = tmp2_missi;
                else
                    mission_list[next_missi].SpecialTrigger[0] = mission_list[tmp_missi].SpecialTrigger[1];
            }
            ingame.MissionStatus = ObvStatu_UNDECIDED;

            return OMiSta_ContSuccess;
        }
        else if (state == MResol_FAILED)
        {
            if (mission_remain_until_success(missi))
            {
                ingame.MissionStatus = ObvStatu_UNDECIDED;

                return OMiSta_ContFailed;
            }
            else
            {
                mission_special_triggers_0_1_set_fail(trg_missi);

                return OMiSta_EndFailed;
            }
        }
    }
    return OMiSta_NONE;
}

void play_post_mission_fmv(ubyte misend)
{
    switch (misend)
    {
    case OMiSta_EndSuccess:
        play_smacker(MPly_MissiComplete);
        break;
    case OMiSta_ContImmSuccess:
        // Reload the palette, potentially damaged by fading
        show_black_screen();
        LbFileLoadAt("qdata/pal.pal", display_palette);
        // The setup_screen_mode() within play_smacker() will
        // automatically switch current pal to display_palette
        play_smacker_then_back_to_engine(MPly_MPartComplete);
        break;
    case OMiSta_EndFailed:
        play_smacker(MPly_MissiFail);
        break;
    case OMiSta_ContSuccess:
        break;
    case OMiSta_ContFailed:
        break;
    case OMiSta_CampaignDone:
        init_outro();
        overall_scale = get_overall_scale_max();
        unkn_flags_01 &= ~0x01;
        break;
    }
}

ubyte check_delete_open_mission(ushort missi, sbyte state)
{
    TbBool conds_met;
    ubyte misend;

    update_mission_list_to_mission_state(missi, state);

    conds_met = check_mission_conds(missi);

    research_unkn_func_006(missi);

    misend = check_open_next_mission(missi, state);

    LOGSYNC("Reached %s, mission=%d, current=%d, conds_met=%d",
      miss_end_sta_names[misend], (int)missi,
      (int)ingame.CurrentMission, (int)conds_met);

    switch (misend)
    {
    case OMiSta_EndSuccess:
        if (conds_met) {
            mission_fire_success_triggers(missi);
        }
        remove_mission_state_slot(missi);
        break;
    case OMiSta_ContImmSuccess:
        if (conds_met) {
            mission_fire_success_triggers(missi);
        }
        compound_mission_brief_store_next();
        break;
    case OMiSta_EndFailed:
        if (conds_met) {
            mission_fire_fail_triggers(missi);
        }
        break;
    case OMiSta_ContSuccess:
        break;
    case OMiSta_ContFailed:
        break;
    }
    return misend;
}

void mission_over(void)
{
    ubyte misend;

    // Hide weapon to cancel any effects
    mission_over_prepare_agents();

    ingame.DisplayMode = DpM_PURPLEMNU;
    reload_menu_flags |= RelMnuF_ColorsSprites;

    LbMouseChangeSprite(0);
    StopCD();
    StopAllSamples();
    SetMusicVolume(100, 0);

    // Some updates take place no matter whether the mission
    // was finished with success or fail
    update_player_cash();
    mission_over_update_players();
    mission_over_gain_persuaded_crowd_rewards();
    players_sync_from_cryo();

    ushort missi;
    short mstate;

    missi = ingame.CurrentMission;
    mstate = get_mission_state_using_state_slot(missi);

    if (mstate == MResol_UNDECIDED) {
        //TODO this is a bug - these statuses have different values (for no reason)
        mstate = ingame.MissionStatus;
        set_mission_state_using_state_slot(missi, mstate);
    }

    long cr_award;
    short email;

    switch (mstate)
    {
    case MResol_COMPLETED:
        cr_award = 1000 * mission_list[missi].CashReward;
        ingame.fld_unkC57++;
        email = mission_list[missi].SuccessID;
        ingame.Credits += cr_award;
        if (email != 0)
            queue_up_new_mail(0, -email);
        misend = check_delete_open_mission(missi, mstate);
        break;
    case MResol_FAILED:
        email = mission_list[missi].FailID;
        ingame.fld_unkC57++;
        if (email != 0)
            queue_up_new_mail(0, -email);
        misend = check_delete_open_mission(missi, mstate);
        break;
    default:
        mstate = MResol_UNDECIDED;
        break;
    }

    if (misend == OMiSta_EndSuccess) {
        if (mission_is_final_at_game_end(missi))
            misend = OMiSta_CampaignDone;
    }

    play_post_mission_fmv(misend);

    if (misend == OMiSta_ContImmSuccess) {
        compound_mission_immediate_start_next();
    }

    ingame.GameOver = 0;
    if (mstate == MResol_FAILED)
    {
        if (!mission_remain_until_success(ingame.CurrentMission) &&
          !mission_has_fail_email_or_triggers(ingame.CurrentMission))
            ingame.GameOver = 1;
    }
    if (cryo_agents.NumAgents == 0)
        ingame.GameOver = 1;
    LOGSYNC("Mission %d ended with state=%d gameover=%d",
      (int)missi, (int)mstate, (int)ingame.GameOver);
    if (ingame.GameOver)
        play_smacker(MPly_GameOver);
}

TbBool player_try_spend_money(long cost)
{
    if (ingame.Credits - cost < 0)
        return false;
    ingame.Credits -= cost;
    ingame.Expenditure += cost;
    return true;
}

void campaign_new_game_prepare(void)
{
    struct Campaign *p_campgn;

    p_campgn = &campaigns[background_type];

    screentype = SCRT_99;
    game_system_screen = SySc_NONE;

    load_missions(background_type);
    clear_mission_state_slots();
    load_objectives_text();

    load_city_data(0);
    init_weapon_text();
    load_city_txt();
    player_mission_agents_reset(local_player_no);
    init_variables();
    srm_reset_research();
    init_agents();

    {
        open_new_mission(p_campgn->FirstTrigger);
        update_brief_screen_netscan_button(p_campgn->NetscanTextId);
    }

    reload_background_flag = 1;
    edit_flag = 0;
}

ubyte goto_savegame(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_goto_savegame\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    restore_savegame = 1;
    game_system_screen = SySc_STORAGE;
    screentype = SCRT_SYSMENU;
    sysmnu_button_disable(0, 5);
    update_sys_scr_shared_header(game_system_screen);
    ingame.Flags &= ~GamF_MortalGame;

    load_city_data(0);
    init_weapon_text();
    load_city_txt();
    player_mission_agents_reset(local_player_no);
    init_variables();
    srm_reset_research();
    init_agents();

    edit_flag = 0;
    save_slot_base = 0;
    redraw_screen_flag = 1;

    load_save_slot_names();

    return 1;
}

ubyte do_storage_NEW_MORTAL(ubyte click)
{
    if (login_control__State != LognCt_Unkn6)
        return 0;

    if (strlen(login_name) == 0)
        strcpy(login_name, "ANON");
    read_user_settings();

    ingame.Flags |= GamF_MortalGame;

    campaign_new_game_prepare();

    if (autosave_game()) {
        alert_box_text_fmt("%s", gui_strings[566]);
    }

    if (restore_savegame)  {
        restore_savegame = 0;
        sysmnu_button_enable(0, 5);
    }

    if ((ingame.Flags & GamF_MortalGame) != 0) {
        sysmnu_button_disable(1,2);
    }

    if (new_mail)
      play_sample_using_heap(0, 119 + (LbRandomAnyShort() % 3), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);

    return 1;
}

void init_screen_boxes(void)
{
    init_global_boxes();
    init_alert_screen_boxes();
    init_main_screen_boxes();
    init_system_menu_boxes();

    init_sys_scr_shared_boxes();
    init_controls_screen_boxes();
    init_options_audio_screen_boxes();
    init_options_gfx_screen_boxes();
    init_net_screen_boxes();
    init_storage_screen_boxes();

    init_login_screen_boxes();

    init_brief_screen_boxes();
    init_world_screen_boxes();
    init_debrief_screen_boxes();
    init_equip_screen_boxes();
    init_cryo_screen_boxes();
    init_research_screen_boxes();
    init_equip_screen_shapes();
}

void move_camera(int x, int y, int z)
{
    asm volatile (
      "call ASM_move_camera\n"
        : : "a" (x), "d" (y), "b" (z));
}

void track_player(int thing)
{
    asm volatile ("call ASM_track_player\n"
        : : "a" (thing));
}

void track_angle(void)
{
    asm volatile ("call ASM_track_angle\n"
        :  :  : "eax" );
}

void track_y(int y)
{
    engn_yc += (y - engn_yc) >> 3;
}

void scroll_map_update_alt(void)
{
    int cumm_alt;

    cumm_alt = alt_at_point(engn_xc, engn_zc) >> 8;
    cumm_alt += alt_at_point(engn_xc + 2048, engn_zc + 2048) >> 8;
    cumm_alt += alt_at_point(engn_xc + 2048, engn_zc - 2048) >> 8;
    cumm_alt += alt_at_point(engn_xc - 2048, engn_zc + 2048) >> 8;
    cumm_alt += alt_at_point(engn_xc - 2048, engn_zc - 2048) >> 8;
    track_y(cumm_alt / 5);
}

void scroll_map_input(int *p_dx, int *p_dy)
{
    PlayerInfo *p_locplayer;
    int mv_border;
    int dx, dy;

    dx = 0;
    dy = 0;
    p_locplayer = &players[local_player_no];
    // Define a move border, getting the mouse beyond it causes map scroll
    if (lbDisplay.GraphicsScreenWidth >= 640)
        mv_border = 3;
    else
        mv_border = 2;

    // Only allow scrolling via keyboard arrows if not DoubleMode, and if the arrows
    // are not used for something else currntly (like changing weapon/mood)
    if ((!p_locplayer->DoubleMode) && ((p_locplayer->UserInput[0].Bits & SpUIn_ChangeMoodOrWep) == 0))
    {
        dx = (is_gamekey_kbd_pressed(GKey_RIGHT) & 1) - (is_gamekey_kbd_pressed(GKey_LEFT) & 1);
        dy = (is_gamekey_kbd_pressed(GKey_DOWN) & 1) - (is_gamekey_kbd_pressed(GKey_UP) & 1);
    }

    {
        if (dx == 0)
        {
            int mx;
            mx = lbDisplay.MMouseX;
            if (mx >= lbDisplay.GraphicsScreenWidth - mv_border)
                dx = 1;
            if (mx < mv_border)
                dx = -1;
        }
        if (dy == 0)
        {
            int my;
            my = lbDisplay.MMouseY;
            if (my >= lbDisplay.GraphicsScreenHeight - mv_border)
                dy = 1;
            if (my < mv_border)
                dy = -1;
        }
    }
    *p_dx = dx;
    *p_dy = dy;
}

void do_scroll_map(void)
{
    PlayerInfo *p_locplayer;
    s32 engn_xc_orig, engn_zc_orig;
    ushort ctlmode;
    long abase, angle;
    int dx, dy, dz;
    int dampr;

    dx = 0;
    dy = 0;
    dz = 0;
    dampr = 10;
    if (ingame.fld_unkCA6)
        track_angle();
    p_locplayer = &players[local_player_no];
    if (p_locplayer->State[0] == 1)
    {
        short bitx, bitz;
        bitx = get_agent_move_direction_delta_x(&p_locplayer->UserInput[0]);
        bitz = get_agent_move_direction_delta_z(&p_locplayer->UserInput[0]);
        dx = bitx << 8;
        dz = bitz << 8;
        local_to_worldr(&dx, &dy, &dz);
        engn_xc += dx;
        engn_zc += dz;
    }
    else
    {
        short dcthing;
        dcthing = direct_control_thing_for_player(local_player_no);
        if (p_locplayer->DoubleMode)
        {
            track_player(dcthing);
            return;
        }
        if (dcthing)
        {
            ctlmode = p_locplayer->UserInput[0].ControlMode & ~UInpCtr_AllFlagsMask;
            if (ctlmode == UInpCtr_Mouse || PacketRecord_IsPlayback())
                move_camera(ingame.TrackX, engn_yc, ingame.TrackZ);
            else
                track_player(dcthing);
        }
    }
    dy = 0;
    dx = 0;
    ctlmode = p_locplayer->UserInput[byte_153198-1].ControlMode & ~UInpCtr_AllFlagsMask;
    engn_xc_orig = engn_xc;
    engn_zc_orig = engn_zc;
    if (ctlmode == UInpCtr_Mouse || PacketRecord_IsPlayback())
    {
        // Only allow scroll view if panel is not used
        if (p_locplayer->PanelState[mouser] == PANEL_STATE_NORMAL)
        {
            scroll_map_input(&dx, &dy);
            scroll_map_update_alt();
            dampr = 9;
        }
    }

    abase = -engn_anglexz >> 5;
    angle = -1;
    if (dx > 0)
        angle = (abase + 3583) & LbFPMath_AngleMask;
    if (dx < 0)
        angle = (abase + 2559) & LbFPMath_AngleMask;
    if (angle >= 0) {
        int wibl_x, wibl_y;
        wibl_x = dword_153194 * lbSinTable[angle] >> 8 >> dampr;
        wibl_y = dword_153194 * lbSinTable[angle + LbFPMath_PI/2] >> 8 >> dampr;
        ingame.TrackX -= wibl_x;
        ingame.TrackZ += wibl_y;
        engn_xc -= wibl_x;
        engn_zc += wibl_y;
    }
    angle = -1;
    if (dy > 0)
        angle = (abase + 3 * LbFPMath_PI - 1) & LbFPMath_AngleMask;
    if (dy < 0)
        angle = (abase + 2 * LbFPMath_PI - 1) & LbFPMath_AngleMask;
    if (angle >= 0) {
        int wibl_x, wibl_y;
        wibl_x = dword_153194 * lbSinTable[angle] >> 8 >> dampr;
        wibl_y = dword_153194 * lbSinTable[angle + LbFPMath_PI/2] >> 8 >> dampr;
        ingame.TrackX -= wibl_x;
        ingame.TrackZ += wibl_y;
        engn_xc -= wibl_x;
        engn_zc += wibl_y;
    }

    if (engn_xc < 0)
        engn_xc = 0;
    else if (engn_xc > MAP_COORD_WIDTH - 1)
        engn_xc = MAP_COORD_WIDTH - 1;
    if (engn_zc < 0)
        engn_zc = 0;
    else if (engn_zc > MAP_COORD_HEIGHT - 1)
        engn_zc = MAP_COORD_HEIGHT - 1;
    // If unsigned value is exceeded by more than half of the range, treat as below minimum
    if (ingame.TrackX > MAP_COORD_WIDTH + MAP_COORD_WIDTH/2)
        ingame.TrackX = 0;
    else if (ingame.TrackX > MAP_COORD_WIDTH - 1)
        ingame.TrackX = MAP_COORD_WIDTH - 1;
    if (ingame.TrackZ > MAP_COORD_HEIGHT + MAP_COORD_HEIGHT/2)
        ingame.TrackZ = 0;
    else if (ingame.TrackZ > MAP_COORD_HEIGHT - 1)
        ingame.TrackZ = MAP_COORD_HEIGHT - 1;

    if (dx) {
        dword_153194 += 4 + ((dword_153194 - 252) >> 5);
        if (dword_153194 > 800)
            dword_153194 = 800;
    } else {
        dword_153194 -= (dword_153194 - 256) >> 2;
    }
    if ((engn_zc - engn_zc_orig) || (engn_xc - engn_xc_orig)) {
        engn_x_vel = engn_xc - engn_xc_orig;
        engn_y_vel = engn_zc - engn_zc_orig;
    } else {
        engn_x_vel >>= 2;
        engn_y_vel >>= 2;
        if (abs(engn_x_vel) < 5)
            engn_x_vel = 0;
        if (abs(engn_y_vel) < 5)
            engn_y_vel = 0;
        engn_xc += engn_x_vel;
        engn_zc += engn_y_vel;
    }
}

ubyte weapon_select_input(void)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_weapon_select_input\n"
        : "=r" (ret) : );
    return ret;
#endif
    PlayerInfo *p_locplayer;
    struct Packet *p_pckt;
    ThingIdx dcthing;
    WeaponType wtype;
    int n;

    static const GameKey sel_weapon_gkeys[] = {
        GKey_SEL_WEP_1, GKey_SEL_WEP_2, GKey_SEL_WEP_3, GKey_SEL_WEP_4, GKey_SEL_WEP_5, GKey_SEL_WEP_6,
    };

    static GameTurn last_sel_weapon_turn[WEAPONS_CARRIED_MAX_COUNT] = {0};

    p_locplayer = &players[local_player_no];
    p_pckt = &packets[local_player_no];
    dcthing = p_locplayer->DirectControl[0];
    wtype = WEP_NULL;

    if (is_gamekey_pressed(GKey_USE_MEDIKIT))
    {
        clear_gamekey_pressed(GKey_USE_MEDIKIT);
        if (person_can_use_medikit(dcthing))
        {
            my_build_packet(p_pckt, PAct_AGENT_USE_MEDIKIT, dcthing, 0, 0, 0);
            return GINPUT_PACKET;
        }
    }

#ifdef MORE_GAME_KEYS
    if (is_gamekey_pressed(GKey_SUPERSHIELD))
    {
        clear_gamekey_pressed(GKey_SUPERSHIELD);
        if (person_can_toggle_supershield(dcthing))
        {
            my_build_packet(p_pckt, PAct_SHIELD_TOGGLE, dcthing, 0, 0, 0);
            return GINPUT_PACKET;
        }
    }

    if (is_gamekey_pressed(GKey_VIEW_THERMAL))
    {
        clear_gamekey_pressed(GKey_VIEW_THERMAL);
        if (player_can_toggle_thermal(local_player_no))
        {
            my_build_packet(p_pckt, PAct_THERMAL_TOGGLE, dcthing, 0, 0, 0);
            return GINPUT_PACKET;
        }
    }
#endif

    assert(sizeof(sel_weapon_gkeys)/sizeof(sel_weapon_gkeys[0]) <= WEAPONS_CARRIED_MAX_COUNT);

    for (n = 0; n < (int)(sizeof(sel_weapon_gkeys)/sizeof(sel_weapon_gkeys[0])); n++)
    {
        GameKey gkey = sel_weapon_gkeys[n];
        if (is_gamekey_pressed(gkey))
        {
            clear_gamekey_pressed(gkey);
            wtype = find_nth_weapon_held(dcthing, n+1);
            if (wtype != WEP_NULL)
                break;
        }
    }
    if (wtype != WEP_NULL)
    {
        // Double tapping - select for all agents
        if (gameturn - last_sel_weapon_turn[n] < game_num_fps / 2)
        {
            ubyte flag;

            flag = (person_get_selected_weapon(dcthing) == wtype) ? WepSel_SELECT : WepSel_HIDE;
            my_build_packet(p_pckt, PAct_SELECT_GRP_SPEC_WEAPON, dcthing, wtype, flag, 0);
            last_sel_weapon_turn[n] -= game_num_fps / 2;
        }
        else
        {
            my_build_packet(p_pckt, PAct_SELECT_SPECIFIC_WEAPON, dcthing, wtype, WepSel_TOGGLE, 0);
            last_sel_weapon_turn[n] = gameturn;
        }

        return GINPUT_PACKET;
    }

    return GINPUT_NONE;
}

void do_rotate_map(void)
{
    asm volatile ("call ASM_do_rotate_map\n"
        :  :  : "eax" );
}

ubyte process_mouse_inputs(void)
{
#if 0
    int ret;
    asm volatile ("call ASM_process_mouse_inputs\n"
        : "=r" (ret) : );
    return ret;
#endif
    PlayerInfo *p_locplayer;
    struct Thing *p_agent;
    struct Packet *p_pckt;
    ThingIdx dcthing;
    int map_y;
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    p_locplayer = &players[local_player_no];

    if (!lbDisplay.MLeftButton)
        p_locplayer->UserInput[mouser].ControlMode &= ~UInpCtrF_Unkn8000;
    if (!lbDisplay.MRightButton)
        p_locplayer->UserInput[mouser].ControlMode &= ~UInpCtrF_Unkn4000;
    if ((ingame.DisplayMode != DpM_ENGINEPLY) && (ingame.DisplayMode != DpM_UNKN_3B))
        return did_inp;
    did_inp |= process_panel_state();
    if ((did_inp & GINPUT_PACKET) != 0)
        return did_inp;
    if (lbDisplay.LeftButton || lbDisplay.RightButton || lbDisplay.MLeftButton || lbDisplay.MRightButton)
    {
        did_inp = check_panel_button();
        if ((did_inp & GINPUT_PACKET) != 0)
            return did_inp;
    }

    dcthing = p_locplayer->DirectControl[mouser];
    p_agent = &things[dcthing];
    if ((p_agent->State == PerSt_PERSON_BURNING) || ((p_agent->Flag & TngF_Destroyed) != 0)) {
        return did_inp;
    }

    if (lbDisplay.LeftButton)
    {
        lbDisplay.LeftButton = 0;

        p_pckt = &packets[local_player_no];
        if (p_locplayer->field_102 != 0)
        {
            switch (p_locplayer->TargetType)
            {
            case 1:
            case 7:
                my_build_packet(p_pckt, PAct_FOLLOW_PERSON, dcthing,
                  p_locplayer->field_102, 0, 0);
                did_inp |= GINPUT_PACKET;
                return did_inp;
            case 2:
            case 6:
                break;
            case 3:
                if (p_locplayer->Target == 0)
                    break;
                if ((gameturn & 0x7FFF) - p_locplayer->UserInput[mouser].Turn >= 7)
                {
                    p_locplayer->UserInput[mouser].Turn = (gameturn & 0x7FFF);
                    my_build_packet(p_pckt, PAct_AGENT_GOTO_FACE_PT_ABS, dcthing,
                      mouse_map_x, p_locplayer->Target, mouse_map_z);
                }
                else
                {
                    p_locplayer->UserInput[mouser].Turn = 0;
                    my_build_packet(p_pckt, PAct_AGENT_GOTO_FACE_PT_ABS_FF, dcthing,
                      mouse_map_x, p_locplayer->Target, mouse_map_z);
                }
                did_inp |= GINPUT_PACKET;
                return did_inp;
            case 4:
                if ((gameturn & 0x7FFF) - p_locplayer->UserInput[mouser].Turn >= 7)
                {
                    p_locplayer->UserInput[mouser].Turn = (gameturn & 0x7FFF);
                    my_build_packet(p_pckt, PAct_GO_ENTER_VEHICLE, dcthing,
                      p_locplayer->field_102, 0, 0);
                }
                else
                {
                    p_locplayer->UserInput[mouser].Turn = 0;
                    my_build_packet(p_pckt, PAct_GO_ENTER_VEHICLE_FF, dcthing,
                      p_locplayer->field_102, 0, 0);
                }
                did_inp |= GINPUT_PACKET;
                return did_inp;
            case 5:
                if ((gameturn & 0x7FFF) - p_locplayer->UserInput[mouser].Turn >= 7)
                {
                    p_locplayer->UserInput[mouser].Turn = (gameturn & 0x7FFF);
                    my_build_packet(p_pckt, PAct_GET_ITEM, dcthing,
                      p_locplayer->field_102, 0, 0);
                }
                else
                {
                    p_locplayer->UserInput[mouser].Turn = 0;
                    my_build_packet(p_pckt, PAct_GET_ITEM_FAST, dcthing,
                      p_locplayer->field_102, 0, 0);
                }
                did_inp |= GINPUT_PACKET;
                return did_inp;
            default:
                break;
            }
        }

        if (!p_locplayer->DoubleMode)
        {
            short ctlmode;
            ctlmode = p_locplayer->UserInput[0].ControlMode & ~UInpCtr_AllFlagsMask;
            if (ctlmode != UInpCtr_Mouse)
            {
                do_change_mouse(8);
                build_packet(p_pckt, PAct_CONTROL_MODE, 1, 0, 0, 0);
                did_inp |= GINPUT_PACKET;
                return did_inp;
            }
        }
        if ((mouse_map_x > 0) && (mouse_map_x < MAP_COORD_WIDTH) &&
          (mouse_map_z > 0) && (mouse_map_z < MAP_COORD_HEIGHT))
        {
            short map_y;

            map_y = (alt_at_point(mouse_map_x, mouse_map_z) >> 8) + 20;
            if ((gameturn & 0x7FFF) - p_locplayer->UserInput[mouser].Turn >= 7)
            {
                p_locplayer->UserInput[mouser].Turn = (gameturn & 0x7FFF);
                my_build_packet(p_pckt, PAct_AGENT_GOTO_GND_PT_ABS, dcthing,
                  mouse_map_x, map_y, mouse_map_z);
            }
            else
            {
                p_locplayer->UserInput[mouser].Turn = 0;
                my_build_packet(p_pckt, PAct_AGENT_GOTO_GND_PT_ABS_FF, dcthing,
                  mouse_map_x, map_y, mouse_map_z);
            }
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
        return did_inp;
    }

    if (lbDisplay.RightButton && ((p_locplayer->UserInput[mouser].ControlMode & 0x4000) == 0))
    {
        WeaponType wtype;
        lbDisplay.RightButton = 0;

        p_pckt = &packets[local_player_no];
        map_y = (alt_at_point(mouse_map_x, mouse_map_z) >> 8) + 20;
        wtype = p_agent->U.UPerson.CurrentWeapon;
        if (weapon_is_for_planting(wtype))
        {
            if ((gameturn & 0x7FFF) - p_locplayer->UserInput[mouser].Turn >= 7)
            {
                p_locplayer->UserInput[mouser].Turn = (gameturn & 0x7FFF);
                if (p_locplayer->TargetType == 3)
                    my_build_packet(p_pckt, PAct_PLANT_MINE_AT_FACE_PT, dcthing,
                      mouse_map_x, p_locplayer->Target, mouse_map_z);
                else
                    my_build_packet(p_pckt, PAct_PLANT_MINE_AT_GND_PT, dcthing,
                      mouse_map_x, map_y, mouse_map_z);
                did_inp |= GINPUT_PACKET;
                return did_inp;
            }
            else
            {
                p_locplayer->UserInput[mouser].Turn = 0;
                if (p_locplayer->TargetType == 3)
                    my_build_packet( p_pckt, PAct_PLANT_MINE_AT_FACE_PT_FF, dcthing,
                      mouse_map_x, p_locplayer->Target, mouse_map_z);
                else
                    my_build_packet(p_pckt, PAct_PLANT_MINE_AT_GND_PT_FF, dcthing,
                      mouse_map_x, map_y, mouse_map_z);
                did_inp |= GINPUT_PACKET;
                return did_inp;
            }
        }
        else if (weapon_is_for_spreading_on_ground(wtype))
        {
            if ((gameturn & 0x7FFF) - p_locplayer->UserInput[mouser].Turn >= 7)
            {
                p_locplayer->UserInput[mouser].Turn = (gameturn & 0x7FFF);
                if (p_locplayer->TargetType == 3)
                    my_build_packet(p_pckt, PAct_SHOOT_AT_FACE_POINT, dcthing,
                      mouse_map_x, p_locplayer->Target, mouse_map_z);
                else
                    my_build_packet(p_pckt, PAct_SHOOT_AT_GND_POINT, dcthing,
                      mouse_map_x, map_y, mouse_map_z);
                did_inp |= GINPUT_PACKET;
                return did_inp;
            }
            else
            {
                p_locplayer->UserInput[mouser].Turn = 0;
                if (p_locplayer->TargetType == 3)
                    my_build_packet(p_pckt, PAct_SHOOT_AT_FACE_POINT_FF, dcthing,
                      mouse_map_x, p_locplayer->Target, mouse_map_z);
                else
                    my_build_packet(p_pckt, PAct_SHOOT_AT_GND_POINT_FF, dcthing,
                      mouse_map_x, map_y, mouse_map_z);
                did_inp |= GINPUT_PACKET;
                return did_inp;
            }
        }
    }

    if ( lbDisplay.MRightButton && ((p_locplayer->UserInput[mouser].ControlMode & 0x4000) == 0))
    {
        p_pckt = &packets[local_player_no];
        map_y = (alt_at_point(mouse_map_x, mouse_map_z) >> 8) + 20;
        if (p_locplayer->TargetType == 3)
        {
            my_build_packet(p_pckt, PAct_SHOOT_AT_FACE_POINT, dcthing,
              mouse_map_x, p_locplayer->Target, mouse_map_z);
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
        if (p_locplayer->field_102 > 0)
        {
            my_build_packet(p_pckt, PAct_SHOOT_AT_THING, dcthing,
              p_locplayer->field_102, 0, 0);
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
        if ((mouse_map_x > 0) && (mouse_map_x < MAP_COORD_WIDTH) &&
          (mouse_map_z > 0) && (mouse_map_z < MAP_COORD_HEIGHT))
        {
            my_build_packet(p_pckt, PAct_SHOOT_AT_GND_POINT, dcthing,
              mouse_map_x, map_y, mouse_map_z);
            did_inp |= GINPUT_PACKET;
            return did_inp;
        }
    }

    return did_inp;
}

ubyte do_music_user_input(void)
{
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    // MIDI Music (tension) volume control
    if (is_gamekey_pressed(GKey_DANGR_VOL_INC))
    {
        if (lbShift & KMod_SHIFT)
            startscr_midivol += 1;
        else
            startscr_midivol += 10;
        sfx_apply_midivol();
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_DANGR_VOL_DEC))
    {
        if (lbShift & KMod_SHIFT)
            startscr_midivol -= 1;
        else
            startscr_midivol -= 10;
        sfx_apply_midivol();
        did_inp |= GINPUT_DIRECT;
    }

    // Sample volume control
    if (is_gamekey_pressed(GKey_SOUND_VOL_INC))
    {
        if (lbShift & KMod_SHIFT)
            startscr_samplevol += 1;
        else
            startscr_samplevol += 10;
        sfx_apply_samplevol();
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_SOUND_VOL_DEC))
    {
        if (lbShift & KMod_SHIFT)
            startscr_samplevol -= 1;
        else
            startscr_samplevol -= 10;
        sfx_apply_samplevol();
        did_inp |= GINPUT_DIRECT;
    }

    // CD Music volume control
    if (is_gamekey_pressed(GKey_MUSIC_VOL_INC))
    {
        if (lbShift & KMod_SHIFT)
            startscr_cdvolume += 1;
        else
            startscr_cdvolume += 10;
        sfx_apply_cdvolume();
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_MUSIC_VOL_DEC))
    {
        if (lbShift & KMod_SHIFT)
            startscr_cdvolume -= 1;
        else
            startscr_cdvolume -= 10;
        sfx_apply_cdvolume();
        did_inp |= GINPUT_DIRECT;
    }

    // Music track control
    if (is_gamekey_pressed(GKey_MUSIC_TRACK))
    {
        clear_gamekey_pressed(GKey_MUSIC_TRACK);
        game_option_inc(GOpt_CDATrack);
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_DANGR_TRACK))
    {
        clear_gamekey_pressed(GKey_DANGR_TRACK);
        game_option_inc(GOpt_DangerTrack);
        StartMusic(ingame.DangerTrack, 0);
        did_inp |= GINPUT_DIRECT;
    }

    return did_inp;
}

ubyte do_user_interface(void)
{
    PlayerInfo *p_locplayer;
    int n;
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    p_locplayer = &players[local_player_no];
    update_danger_music(0);
    do_scroll_map();
    do_rotate_map();
    if (in_network_game)
    {
        if (is_key_pressed(KC_RETURN, KMod_DONTCARE))
        {
            clear_key_pressed(KC_RETURN);
            if ((p_locplayer->PanelState[mouser] != PANEL_STATE_SEND_MESSAGE)
              && (player_message_timer[local_player_no] <= 140))
            {
                p_locplayer->PanelState[mouser] = PANEL_STATE_SEND_MESSAGE;
                reset_buffered_keys();
                player_message_text[local_player_no][0] = '\0';
                player_message_timer[local_player_no] = 0;
                scanner_unkn370 = 0;
                scanner_unkn3CC = 0;
                did_inp |= GINPUT_DIRECT;
            }
        }
    }
    if (p_locplayer->PanelState[mouser] == PANEL_STATE_SEND_MESSAGE)
    {
        did_inp |= process_mouse_inputs();
        // do not allow other keyboard inputs when writing chat message
        return did_inp;
    }

    // screenshot
    if (is_gamekey_pressed(GKey_SCREENSHOT))
    {
        clear_gamekey_pressed(GKey_SCREENSHOT);
        LbPngSaveScreen("synII", lbDisplay.WScreen, display_palette, 0);
        did_inp |= GINPUT_DIRECT;
    }

#if 0
    // This looks like some kind of early idea for grouping agents like in C&C
    // clearly unfinished; remove pending
    for (n = 0; n < 5; n++)
    {
        if (is_key_pressed(KC_F1+n, KMod_ALT))
        {
            clear_key_pressed(KC_F1+n);
            my_build_packet(&packets[local_player_no], PAct_AGENT_UNKGROUP_ADD, n, 0, 0, 0);
            return 1;
        }
        if (is_key_pressed(KC_F1+n, KMod_SHIFT))
        {
            clear_key_pressed(KC_F1+n);
            my_build_packet(&packets[local_player_no], PAct_AGENT_UNKGROUP_PROT, n, 0, 0, 0);
            return 1;
        }
    }
#endif

    // change panel style
    if (is_key_pressed(KC_F9, KMod_NONE))
    {
        clear_key_pressed(KC_F9);
        StopCD();
        game_option_inc(GOpt_PanelPermutation);
        init_scanner_colour();
        load_pop_sprites_for_current_mode();
        did_inp |= GINPUT_DIRECT;
    }

    // change agents colours
    if (is_key_pressed(KC_F10, KMod_NONE))
    {
        clear_key_pressed(KC_F10);
        StopCD();
        game_option_inc(GOpt_TrenchcoatPreference);
        prep_multicolor_sprites();
        did_inp |= GINPUT_DIRECT;
    }

    // adjust palette brightness
    if (is_key_pressed(KC_F11, KMod_CONTROL))
    {
        clear_key_pressed(KC_F11);
        reset_user_selected_brightness();
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F11, KMod_SHIFT))
    {
        clear_key_pressed(KC_F11);
        change_brightness(-1);
        user_sel_brightness--;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F11, KMod_NONE))
    {
        clear_key_pressed(KC_F11);
        change_brightness(1);
        user_sel_brightness++;
        did_inp |= GINPUT_DIRECT;
    }

    if (is_key_pressed(KC_F1, KMod_CONTROL))
    {
        clear_key_pressed(KC_F1);
        game_option_inc(GOpt_BillboardMovies);
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F2, KMod_CONTROL))
    {
        clear_key_pressed(KC_F2);
        if (ingame.Flags & GamF_RenderScene)
            ingame.Flags &= ~GamF_RenderScene;
        else
            ingame.Flags |= GamF_RenderScene;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F3, KMod_CONTROL))
    {
        clear_key_pressed(KC_F3);
        if (ingame.Flags & GamF_StopThings)
            ingame.Flags &= ~GamF_StopThings;
        else
            ingame.Flags |= GamF_StopThings;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F4, KMod_CONTROL))
    {
        clear_key_pressed(KC_F4);
        game_option_inc(GOpt_AdvancedLights);
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F6, KMod_CONTROL))
    {
        clear_key_pressed(KC_F6);
        game_option_inc(GOpt_DeepRadar);
        did_inp |= GINPUT_DIRECT;
    }
    if (is_key_pressed(KC_F10, KMod_CONTROL))
    {
        clear_key_pressed(KC_F10);
        if (ingame.Flags & GamF_HUDPanel)
            ingame.Flags &= ~GamF_HUDPanel;
        else
            ingame.Flags |= GamF_HUDPanel;
        did_inp |= GINPUT_DIRECT;
    }

    // Game Speed control
    if (!in_network_game)
    {
        did_inp |= get_speed_control_inputs();
    }

    // Toggle Scanner beep
    if (is_gamekey_pressed(GKey_SCANNER_PULSE))
    {
        clear_gamekey_pressed(GKey_SCANNER_PULSE);
        game_option_inc(GOpt_ScannerPulse);
        did_inp |= GINPUT_DIRECT;
    }

    // Control map area to draw
    if ((ingame.UserFlags & UsrF_Cheats) != 0)
    {
        if (is_key_pressed(KC_E, KMod_ALT) || is_key_pressed(KC_E, KMod_ALT|KMod_SHIFT))
        {
            if (lbShift & KMod_SHIFT)
                n = -2;
            else
                n = 2;
            render_area_a = bound_render_area(render_area_a + n);
            render_area_b = bound_render_area(render_area_b + n);
            did_inp |= GINPUT_DIRECT;
        }
    }

    // Entering pause screen
    if (!in_network_game)
    {
        if (is_gamekey_pressed(GKey_PAUSE))
        {
            if (pause_screen_handle()) {
                return GINPUT_EXIT;
            }
        }
    }

    // Scanner appearence control
    if (is_gamekey_pressed(GKey_SCANNR_BRIGH_INC)) {
        ingame.Scanner.Brightness += 4;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_SCANNR_BRIGH_DEC)) {
        ingame.Scanner.Brightness -= 4;
        did_inp |= GINPUT_DIRECT;
    }
    if (ingame.Scanner.Brightness < 0)
        ingame.Scanner.Brightness = 0;
    if (ingame.Scanner.Brightness > 64)
        ingame.Scanner.Brightness = 64;

    do_music_user_input();
    // Restart level
    if (!in_network_game && (ingame.Flags & GamF_MortalGame) == 0)
    {
        if (is_gamekey_pressed(GKey_MISSN_RESTART))
        {
            clear_gamekey_pressed(GKey_MISSN_RESTART);
            StopCD();
            test_missions(1);
            init_level_3d(1);

            restart_back_into_mission(ingame.CurrentMission);
            did_inp |= GINPUT_DIRECT;
        }
    }

    if (ingame.TrackThing)
    {
        short dcthing;
        dcthing = p_locplayer->DirectControl[0];
        build_packet(&packets[local_player_no], 0, dcthing, 0, 0, 0);
        did_inp |= GINPUT_PACKET;
        return did_inp;
    }

    static GameKey sel_agent_gkeys[] = {
        GKey_SEL_AGENT_1, GKey_SEL_AGENT_2, GKey_SEL_AGENT_3, GKey_SEL_AGENT_4
    };
    static GameTurn last_sel_agent_turn[AGENTS_SQUAD_MAX_COUNT] = {0};
    assert(sizeof(sel_agent_gkeys)/sizeof(sel_agent_gkeys[0]) <= AGENTS_SQUAD_MAX_COUNT);
    for (n = 0; n < (int)(sizeof(sel_agent_gkeys)/sizeof(sel_agent_gkeys[0])); n++)
    {
        GameKey gkey = sel_agent_gkeys[n];
        if (is_gamekey_pressed(gkey))
        {
            struct Thing *p_agent;

            p_agent = p_locplayer->MyAgent[n];
            if (p_agent->Type != TT_PERSON) continue;

            if (person_can_accept_control(p_agent->ThingOffset) && ((p_agent->Flag2 & TgF2_KnockedOut) == 0))
            {
                clear_gamekey_pressed(gkey);
                if (p_locplayer->DoubleMode)
                {
                    byte_153198 = n+1;
                }
                else
                {
                    short dcthing;
                    dcthing = p_locplayer->DirectControl[n];
                    my_build_packet(&packets[local_player_no], PAct_SELECT_AGENT, dcthing, p_agent->ThingOffset, 0, 0);
                    p_locplayer->UserInput[0].ControlMode |= UInpCtrF_Unkn8000;
                    // Double tapping - center view on the agent
                    if (gameturn - last_sel_agent_turn[n] < 7)
                    {
                      struct Packet *p_pckt;

                      p_pckt = &packets[local_player_no];

                      game_set_cam_track_thing_xz(p_agent->ThingOffset);
                      engn_yc = PRCCOORD_TO_MAPCOORD(p_agent->Y);
                      dcthing = p_locplayer->DirectControl[mouser];
                      build_packet(p_pckt, PAct_SELECT_AGENT, dcthing, p_agent->ThingOffset, 0, 0);
                      if (p_agent->ThingOffset == (short)p_locplayer->DirectControl[mouser])
                      {
                          engn_xc = PRCCOORD_TO_MAPCOORD(p_agent->X);
                          engn_zc = PRCCOORD_TO_MAPCOORD(p_agent->Z);
                      }
                    }
                    last_sel_agent_turn[n] = gameturn;
                }
                did_inp |= GINPUT_PACKET;
                return did_inp;
            }
        }
    }

    if ((ingame.Flags & GamF_Unkn0100) != 0)
    {
        short dcthing;
        dcthing = p_locplayer->DirectControl[0];
        build_packet(&packets[local_player_no], 0, dcthing, 0, 0, 0);
        did_inp |= GINPUT_PACKET;
        return did_inp;
    }

    struct SpecialUserInput *p_usrinp;
    short ctlmode;

    if (p_locplayer->DoubleMode)
    {
        for (n = 0; n < p_locplayer->DoubleMode + 1; n++)
        {
            short dcthing;

            p_usrinp = &p_locplayer->UserInput[n];
            do_user_input_bits_control_clear_nonmove(p_usrinp);
            ctlmode = p_usrinp->ControlMode & ~UInpCtr_AllFlagsMask;
            if (ctlmode == UInpCtr_Mouse)
            {
                do_user_input_bits_control_clear_all(p_usrinp);
                process_mouse_inputs();
            }
            else if (ctlmode <= UInpCtr_Keyboard)
            {
                dcthing = p_locplayer->DirectControl[n];
                if (person_can_accept_control(dcthing))
                {
                    did_inp = weapon_select_input();
                    if ((did_inp & GINPUT_PACKET) == 0)
                    {
                        do_user_input_bits_direction_clear(p_usrinp);
                        do_user_input_bits_direction_from_kbd(p_usrinp);
                        do_user_input_bits_actions_from_kbd(p_usrinp);
                    }
                }
            }
            else
            {
                dcthing = p_locplayer->DirectControl[n];
                if (!person_can_accept_control(dcthing))
                    return did_inp;

                do_user_input_bits_direction_clear(p_usrinp);
                do_user_input_bits_direction_from_joy(p_usrinp, ctlmode - 2);
                do_user_input_bits_actions_from_joy(p_usrinp, ctlmode - 2);
            }
            ctlmode = p_usrinp->ControlMode & ~UInpCtr_AllFlagsMask;
            if (ctlmode != UInpCtr_Mouse)
            {
                update_agent_move_direction_deltas(p_usrinp);
            }
        }
    }
    else
    {
        short dcthing;

        p_usrinp = &p_locplayer->UserInput[0];
        ctlmode = p_usrinp->ControlMode & ~UInpCtr_AllFlagsMask;
        if ((ctlmode == UInpCtr_Mouse) && is_gamekey_pressed(GKey_KEY_CONTROL))
        {
            clear_gamekey_pressed(GKey_KEY_CONTROL);
            p_usrinp->ControlMode &= UInpCtr_AllFlagsMask;
            p_usrinp->ControlMode |= UInpCtr_Keyboard;
            do_change_mouse(8);
            p_locplayer->State[0] = 0;
            did_inp |= GINPUT_DIRECT;
        }
        do_user_input_bits_control_clear_all(p_usrinp);
        do_user_input_bits_direction_clear(p_usrinp);

        did_inp |= process_mouse_inputs();
        if ((did_inp & GINPUT_PACKET) != 0)
            return did_inp;

        did_inp |= weapon_select_input();
        if ((did_inp & GINPUT_PACKET) != 0)
            return did_inp;

        dcthing = p_locplayer->DirectControl[0];
        if (person_can_accept_control(dcthing))
        {
            do_user_input_bits_actions_from_joy_and_kbd(p_usrinp);

            ctlmode = p_usrinp->ControlMode & ~UInpCtr_AllFlagsMask;
            if (ctlmode != UInpCtr_Mouse)
            {
                do_user_input_bits_direction_clear(p_usrinp);
                do_user_input_bits_direction_from_kbd(p_usrinp);
                do_user_input_bits_direction_from_joy(p_usrinp, 0);
                update_agent_move_direction_deltas(p_usrinp);
            }
            else if ((p_usrinp->Bits & SpUIn_ChangeMoodOrWep) != 0)
            {
                // Changing weapon or mood - prepare direction values, but do not treat them as movement
                do_user_input_bits_direction_clear(p_usrinp);
                do_user_input_bits_direction_from_kbd(p_usrinp);
                do_user_input_bits_direction_from_joy(p_usrinp, 0);
            }
        }
    }
    return did_inp;
}

void show_menu_screen_st0(void)
{
    LOGSYNC("Init main menu, %s game", in_network_game ? "MP" : "SP");

    debug_trace_place(16);

    lbInkeyToAscii[KC_OEM_102] = '\\';
    lbInkeyToAsciiShift[KC_OEM_102] = '|';

    players_init_default_control_mode();

    login_control__State = LognCt_Unkn6;
    sprintf(net_unkn2_text, "01234567890");

    {
        long pos = 0;
        hotspot_buffer = (struct ScreenPoint *)((ubyte *)scratch_malloc_mem + pos);
        pos += hotspot_buffer_len;

        mission_briefing_text = (char *)scratch_malloc_mem + pos;
        pos += mission_briefing_text_len;

        netscan_text = (char *)scratch_malloc_mem + pos;
        pos += netscan_text_len;

        weapon_text = (char *)scratch_malloc_mem + pos;
        pos += weapon_text_len;

        memload = (ubyte *)scratch_malloc_mem + pos;
        pos += memload_len;

        purple_draw_list = (struct PurpleDrawItem *)((ubyte *)scratch_malloc_mem + pos);
    }

    ingame.Credits = 50000;

    global_date.Day = 2;
    global_date.Month = 6;
    global_date.Year = 74;

    load_city_data(0);
    load_city_txt();
    player_mission_agents_reset(local_player_no);

    debug_trace_place(17);
    // Need to set screen type before gfx background is reloaded
    if (in_network_game)
        screentype = SCRT_LOGIN;
    else
        screentype = SCRT_MAINMENU;
    reload_menu_flags |= RelMnuF_ColorsSprites | RelMnuF_ScrBoxesFull | RelMnuF_BriefScanner;

    debug_trace_place(18);
    save_game_buffer = vec_tmap[5];

    net_system_init0();
}

void init_net_players(void)
{
    int i;
    for (i = 0; i < 5; i++) {
        LbMemorySet(&net_players[i], '\0', sizeof(struct NetPlayer2));
    }
}

void net_new_game_prepare(void)
{
    switch_net_screen_boxes_to_initiate();
    login_control__State = LognCt_Unkn6;
    byte_15516D = -1;
    byte_15516C = -1;
    ingame.Credits = 50000;
    ingame.CashAtStart = 50000;
    login_control__TechLevel = 4;
    reset_world_screen_player_state();
    login_control__City = -1;
    ingame.Expenditure = 0;
    net_game_play_flags = NGPF_Unkn20 | NGPF_Unkn10 | NGPF_Unkn08 | NGPF_Unkn04;
    login_control__Money = starting_cash_amounts[4];
    init_agents();
    load_missions(background_type);
    load_objectives_text();
    srm_reset_research();
    init_net_players();
    draw_flic_purple_list(ac_purple_unkn1_data_to_screen);
}


void update_mission_time(char a1)
{
    asm volatile ("call ASM_update_mission_time\n"
        : : "a" (a1));
}

void show_menu_screen_st2(void)
{
    LOGSYNC("Init debrief, %s game", in_network_game ? "MP" : "SP");

    if (in_network_game)
    {
        local_player_no = 0;
        net_new_game_prepare();
        net_sessionlist_clear();
        scientists_lost = 0;
        update_mission_time(0);
        in_network_game = 0;

        reset_cryo_screen_player_state();
        reset_equip_screen_player_state();
        screentype = SCRT_NETDEBRF;
        redraw_screen_flag = 1;
        set_heading_box_text(gui_strings[374]);
    }
    else
    {
      update_mission_time(0);
      selected_city_id = -1;
      byte_1C4AA3 = brief_store[open_brief - 1].RefNum;
      // Original code compared MissionStatus to 0 and 2, is 2 a valid value?
      if ((ingame.MissionStatus != ObvStatu_UNDECIDED) && (ingame.MissionStatus != ObvStatu_FAILED))
      {
            memcpy(&mission_status[0], &mission_status[open_brief],
              sizeof(struct MissionStatus));
            delete_mail(open_brief - 1, MlTp_Mission);
            open_brief = 0;
            old_mission_brief = 0;
            cities[unkn_city_no].Info = 0;
      }
      else
      {
            old_mission_brief = open_brief;
      }
      if (ingame.GameOver)
      {
            screentype = SCRT_MAINMENU;
            if ((ingame.Flags & GamF_MortalGame) != 0) {
                char fname[DISKPATH_SIZE];
                get_saved_game_fname(fname, 0);
                LbFileDelete(fname);
            }
            ingame.GameOver = 0;
      }
      else
      {
            struct MissionStatus *p_mistat;
            p_mistat = &mission_status[open_brief];
            forward_research_progress_after_mission(p_mistat->CityDays);
            autosave_game();
            screentype = SCRT_DEBRIEF;
            set_heading_box_text(gui_strings[374]);
            redraw_screen_flag = 1;
      }
    }

    init_weapon_text();
    load_city_txt();

    update_options_screen_state();

    reload_menu_flags |= RelMnuF_ColorsSprites | RelMnuF_BriefScanner;

    if ((new_mail != 0) && (screentype != SCRT_MAINMENU))
        play_sample_using_heap(0, 119 + (LbRandomAnyShort() % 3), FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);

    net_system_init2();
}

ushort find_mission_with_mapid(short mapID, short mission_limit)
{
    ushort i;
    for (i = 1; i < mission_limit; i++)
    {
        if (mission_list[i].MapNo == mapID) {
            return i;
        }
    }
    return 0;
}

/** Searches for mission taking place in given city, within mission chain specified by the brief.
 */
ushort find_mission_for_city_in_brief(short brief, sbyte city_no)
{
    ushort missi;

    for (missi = brief_store[brief].Mission; missi != 0;
      missi = mission_list[missi].SpecialTrigger[0])
    {
        if (mission_list[missi].MapNo == cities[city_no].MapID)
            break;
    }
    return missi;
}

void update_open_brief(void)
{
    short brief;
    open_brief = 0;
    for (brief = 0; brief < next_brief; brief++)
    {
        ushort missi;

        missi = find_mission_for_city_in_brief(brief, unkn_city_no);
        if (missi != 0) {
            open_brief = brief + 1;
            break;
        }
    }
}

void show_load_and_prep_mission(void)
{
    init_random_seed();
    show_mission_loading_screen();

    if ( start_into_mission )
    {
        if (!in_network_game) {
            update_open_brief();
        }

        ingame.MissionEndFade = 0;
        engine_render_lights = 1;

        debug_trace_place(10);
        if ( in_network_game )
        {
            ushort missi;
            ingame.MissionNo = 1;
            missi = find_mission_with_mapid(cities[login_control__City].MapID, next_mission);
            if (missi > 0) {
                ingame.MissionNo = missi;
            }
            missi = ingame.MissionNo;
            ingame.CurrentMission = missi;
            debug_trace_place(11);
        }
        else
        {
            ushort missi;
            missi = find_mission_for_city_in_brief(open_brief - 1, unkn_city_no);
            load_mission_name_text(missi);
            ingame.CurrentMission = missi;
            debug_trace_place(12);
        }
    }
    else
    {
        LbMouseChangeSprite(&pointer_sprites[1]);
        ingame.DisplayMode = DpM_UNKN_1;
    }

    reload_background_flag = 1;
    // Setup screen, palette and colour tables
    debug_trace_place(13);
    setup_engine_screen_mode();
    // Show something on screen until the load finishes
    show_simple_load_screen();
    // Now we can call the init; it uses current video mode and colour tables
    // to scale correctly, so has to be done after video setup
    // (though we should at some point separate the part linked to current video settings)
    if (start_into_mission)
    {
        if (PacketRecord_IsPlayback())
        {
            packet_read_whole_player_init();
        }
        init_game(0);
    }

    // Update game progress and prepare level to play
    if (start_into_mission)
    {
        clear_open_mission_status();
        if (in_network_game)
        {
            update_mission_time(1);
            gameturn = 0;
        }
        else
        {
            net_unkn2_names_clear();
            strncpy(unkn2_names[0], login_name, 16);

            update_mission_time(1);
            cities[unkn_city_no].Info = 0;
            mission_result = 0;
        }
        if (!in_network_game)
        {
            if (PacketRecord_IsRecord()) {
                PacketRecord_OpenWrite();
                packet_write_whole_player_init();
            }
        }
        debug_trace_place(19);
    }
    render_anim_turn = gameturn;

    // Set up remaining graphics data and controls
    if (start_into_mission)
    {
        prep_multicolor_sprites();
        LbScreenClear(0);
        generate_shadows_for_multicolor_sprites();
        adjust_mission_engine_to_video_mode();

        flic_unkn03(AniSl_BILLBOARD);
        xdo_next_frame(AniSl_BILLBOARD);

        if (in_network_game)
        {
            if (nsvc.I.Type != NetSvc_IPX)
                ingame.InNetGame_UNSURE = ((1 << 0) | (1 << 1)); // two players
            ingame.DetailLevel = 0;
            bang_set_detail(ingame.DetailLevel == 0);
        }

        lbDisplay.MLeftButton = 0;
        lbDisplay.LeftButton = 0;
    }

    map_editor = 0;
    start_into_mission = 0;
    reload_background_flag = 0;
    net_system_reset();
    stop_sample_using_heap(0, 122);
}

void mouse_sprite_animate(void)
{
    if (gameturn & 1)
    {
      struct TbSprite *p_mspr;
      if (++mouse_sprite_anim_frame > 7)
          mouse_sprite_anim_frame = 0;
      p_mspr = &fe_mouseptr_sprites[1 + mouse_sprite_anim_frame];
      LbMouseChangeSprite(p_mspr);
    }
}

void menu_screen_redraw(void)
{
    mo_weapon = -1;
    reload_background_flag = 1;
    if (screentype == SCRT_WORLDMAP)
    {
        open_brief = old_mission_brief;
        activate_cities(0);
    }
    else if (screentype == SCRT_MISSION)
    {
        activate_cities(open_brief);
    }

    reset_system_menu_boxes_flags();
    reset_sys_scr_shared_boxes_flags();

    reset_brief_screen_boxes_flags();
    reset_heading_screen_boxes_flags();
    reset_debrief_screen_boxes_flags();
    reset_net_screen_boxes_flags();
    reset_world_screen_boxes_flags();
    reset_login_screen_boxes_flags();
    reset_controls_screen_boxes_flags();
    reset_storage_screen_boxes_flags();
    reset_cryo_screen_boxes_flags();
    reset_equip_screen_boxes_flags();
    reset_research_screen_boxes_flags();

    reset_options_audio_boxes_flags();
    reset_options_gfx_boxes_flags();

    set_flag01_storage_screen_boxes();
    set_flag01_login_screen_boxes();
    set_flag01_main_screen_boxes();
    set_flag01_cryo_screen_boxes();
    set_flag01_research_screen_boxes();
    set_flag01_net_screen_boxes();
    set_flag01_equip_screen_boxes();
    set_flag01_controls_screen_boxes();
    set_flag01_brief_screen_boxes();
    set_flag01_world_screen_boxes();

    if (!game_projector_speed && screentype != SCRT_99)
        play_sample_using_heap(0, 113, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
}

/** Mark beginning of user input processing code.
 */
void input_processing_beg(void)
{
    if (joy.Buttons[0] && !net_unkn_pos_02)
    {
        if (!simulated_mouse_click)
        {
            int i;

            simulated_mouse_click = 1;

            // Simulate mouse click at current move position
            lbDisplay.LeftButton = 1;
            if (lbDisplay.GraphicsScreenWidth > 320)
                i = lbDisplay.MMouseX * lbDisplay.GraphicsScreenWidth / 320;
            else
                i = lbDisplay.MMouseX;
            lbDisplay.MouseX = i;
            if (lbDisplay.GraphicsScreenHeight > 200)
                i = lbDisplay.MMouseY * lbDisplay.GraphicsScreenHeight / 200;
            else
                i = lbDisplay.MMouseY;
            lbDisplay.MouseY = i;
        }
    }
    else if (simulated_mouse_click)
    {
        simulated_mouse_click = 0;
    }

    clear_left_button = lbDisplay.LeftButton;
    clear_right_button = lbDisplay.RightButton;
#if defined(LB_ENABLE_MOUSE_WHEEL)
    clear_wheel_up = lbDisplay.WheelMoveUp;
    clear_wheel_down = lbDisplay.WheelMoveDown;
#endif
}

/** Mark end of user input processing code.
 */
void input_processing_end(void)
{
    if (clear_left_button && lbDisplay.LeftButton) {
        clear_left_button = 0;
        lbDisplay.LeftButton = 0;
    }
    if (clear_right_button && lbDisplay.RightButton) {
        clear_right_button = 0;
        lbDisplay.RightButton = 0;
    }
#if defined(LB_ENABLE_MOUSE_WHEEL)
    // Clear cummulative inputs if they remained unused after processing all inputs
    if (clear_wheel_up && lbDisplay.WheelMoveUp) {
        clear_wheel_up = 0;
        lbDisplay.WheelMoveUp = 0;
    }
    if (clear_wheel_down && lbDisplay.WheelMoveDown) {
        clear_wheel_down = 0;
        lbDisplay.WheelMoveDown = 0;
    }
#endif
}

void apply_change_screen(void)
{
    if (change_screen == ChSCRT_SYSMENU)
    {
        screentype = SCRT_SYSMENU;
        redraw_screen_flag = 1;
        set_heading_box_text("");
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
    }
    if (change_screen == ChSCRT_PANET)
    {
        screentype = SCRT_PANET;
        redraw_screen_flag = 1;
        set_heading_box_text(gui_strings[367]);
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
    }
    if (change_screen == ChSCRT_WORLDMAP)
    {
        set_heading_box_text(gui_strings[368]);
        redraw_screen_flag = 1;
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
        screentype = SCRT_WORLDMAP;
        if (selected_city_id != -1)
          unkn_city_no = selected_city_id;
    }
    if (change_screen == ChSCRT_CRYO)
    {
        screentype = SCRT_CRYO;
        switch_shared_equip_screen_buttons_to_cybmod();

        update_cybmod_cost_text();
        redraw_screen_flag = 1;
        reset_mod_draw_states_flag08();
        current_drawing_mod = 0;
        new_current_drawing_mod = 0;
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
    }
    if (change_screen == ChSCRT_EQUIP)
    {
        screentype = SCRT_EQUIP;
        switch_shared_equip_screen_buttons_to_equip();
        update_equip_cost_text();
        redraw_screen_flag = 1;
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
    }
    if (change_screen == ChSCRT_RESEARCH)
    {
        screentype = SCRT_RESEARCH;
        set_heading_box_text(gui_strings[371]);
        clear_research_screen();
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
        redraw_screen_flag = 1;
    }
    if (change_screen == ChSCRT_MISBRIEF)
    {
        selected_city_id = -1;
        screentype = SCRT_MISSION;
        brief_load_mission_info();
        redraw_screen_flag = 1;
        edit_flag = 0;
        change_screen = ChSCRT_NONE;
    }

    assert(change_screen == 0);
}

void show_menu_screen(void)
{
    switch (data_1c498d)
    {
    case 2:
        data_1c498d = 1;
        show_menu_screen_st2();
        play_sample_using_heap(0, 122, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
        break;
    case 0:
        data_1c498d = 1;
        show_menu_screen_st0();
        play_sample_using_heap(0, 122, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 3);
        break;
    default:
        break;
    }

    if (lbDisplay.ScreenMode != screen_mode_menu)
    {
        LbMouseReset();
        LbScreenClear(0);
        setup_screen_mode(screen_mode_menu);
        reload_menu_flags |= RelMnuF_ColorsSprites;
    }

    if ((reload_menu_flags & RelMnuF_ColorsSprites) != 0)
    {
        reload_menu_flags &= ~RelMnuF_ColorsSprites;
        init_purple_mode_colors_and_sprites();
        my_set_text_window(0, 0, lbDisplay.GraphicsScreenWidth, lbDisplay.GraphicsScreenHeight);
    }

    if ((reload_menu_flags & RelMnuF_ScrBoxesFull) != 0)
    {
        reload_menu_flags &= ~RelMnuF_ScrBoxesFull;
        init_screen_boxes();
    }

    if ((reload_menu_flags & RelMnuF_BriefScanner) != 0)
    {
        reload_menu_flags &= ~RelMnuF_BriefScanner;
        init_brief_screen_scanner();
    }

    if (screentype == SCRT_MAINMENU)
    {
        replay_intro_timer++;
        if (replay_intro_timer > INTRO_REPLAY_TURNS)
        {
            play_smacker_then_back_to_menu(MPly_Intro);
            replay_intro_timer = 0;
            data_1c498d = 0;
            return;
          }
    }

    if (screentype != SCRT_MAINMENU) {
        replay_intro_timer = 0;
    }
    text_buf_pos = lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsScreenHeight;

    show_purple_status_top_bar();

    if (is_purple_apps_selection_bar_visible())
          show_purple_apps_selection_bar();

    if ((screentype == SCRT_DEBRIEF || screentype == SCRT_NETDEBRF) && change_screen == ChSCRT_MISBRIEF)
    {
        change_screen = ChSCRT_MISBRIEF;
        apply_change_screen();
    }

    input_processing_beg();

    input_date_time();

    // TODO separate input functions from draw functions and update functions
    switch (screentype)
    {
    case SCRT_MISSION:
        show_mission_screen();
        break;
    case SCRT_WORLDMAP:
        show_worldmap_screen();
        break;
    case SCRT_CRYO:
        show_cryo_chamber_screen();
        break;
    case SCRT_EQUIP:
        show_equipment_screen();
        break;
    case SCRT_MAINMENU:
        show_main_screen();
        break;
    case SCRT_SYSMENU:
        show_sysmenu_screen();
        break;
    case SCRT_RESEARCH:
        show_research_screen();
        break;
    case SCRT_PANET:
        show_panet_screen();
        break;
    case SCRT_DEBRIEF:
        show_debrief_screen();
        break;
    case SCRT_LOGIN:
        show_login_screen();
        break;
    case SCRT_NETDEBRF:
        show_debrief_screen();
        break;
    case SCRT_ALERTBOX:
        show_alert_box();
        break;
    }

    if (is_purple_apps_selection_bar_visible() && !is_purple_alert_on_top())
        input_purple_apps_selection_bar();

    if (is_key_pressed(KC_F12, KMod_DONTCARE)) {
        clear_key_pressed(KC_F12);
        post_render_action = PRend_SaveScreenshot;
    }

    input_processing_end();

    if (login_control__State == LognCt_Unkn5)
    {
        net_unkn_func_33();
    }
    else if (login_control__State == LognCt_Unkn8)
    {
        start_into_mission = 1;
        in_network_game = 1;
        redraw_screen_flag = 1;
        local_player_no = LbNetworkPlayerNumber();
        net_players_num = LbNetworkSessionNumberPlayers();
        switch_net_screen_boxes_to_initiate();
        net_players_copy_equip_and_cryo_now();
        init_net_players();
    }

    memcpy(lbDisplay.WScreen, back_buffer, lbDisplay.GraphicsScreenWidth * lbDisplay.GraphicsScreenHeight);
    draw_purple_screen();

    switch (post_render_action)
    {
    case PRend_SaveScreenshot:
        LbPngSaveScreen("synII", lbDisplay.WScreen, display_palette, 0);
        break;
    default:
        break;
    }

    update_date_time();

    apply_change_screen();

    if (show_alert)
    {
        reset_alert_screen_boxes_flags();
        edit_flag = 0;
        old_screentype = screentype;
        show_alert = 0;
        screentype = SCRT_ALERTBOX;
        set_flag01_alert_screen_boxes();
        redraw_screen_flag = 1;
    }

    if (redraw_screen_flag && !edit_flag)
    {
        redraw_screen_flag = 0;
        menu_screen_redraw();
    }

    mouse_sprite_animate();

    if ( start_into_mission || map_editor )
    {
        show_load_and_prep_mission();
        data_1c498d = 2;
    }
    else if (reload_background_flag)
    {
        reload_background();
        reload_background_flag = 0;
    }

    if (exit_game)
    {
        wait_for_sound_sample_finish(111);
        stop_sample_using_heap(0, 122);
    }
}

TbBool skip_redraw_this_turn(void)
{
    if ((ingame.Flags & GamF_Unkn0020) != 0)
    {
        return ((gameturn & 0xF) != 0);
    }
    return false;
}

void show_game_screen(void)
{
    PlayCDTrack(ingame.CDTrack);

    if (skip_redraw_this_turn())
        return;

    show_game_engine();

    if ((ingame.Flags & GamF_Unkn0800) != 0)
        gproc3_unknsub2();

    BAT_play();

    if (execute_commands)
    {
        long tmp;
        gamep_unknval_16 = nav_stats__ThisTurn;
        nav_stats__ThisTurn = 0;
        gamep_unknval_12++;
        gamep_unknval_10 += gamep_unknval_16;
        gamep_unknval_15 = gamep_unknval_14;
        tmp = gamep_unknval_14 + gamep_unknval_11;
        gamep_unknval_14 = 0;
        gamep_unknval_11 = tmp;
    }
}

void draw_game(void)
{
    switch (ingame.DisplayMode)
    {
    case DpM_UNKN_1:
        // No action
        break;
    case DpM_ENGINEPLY:
        show_game_screen();
        break;
    case DpM_PURPLEMNU:
        show_menu_screen();
        break;
    case DpM_UNKN_3A:
        show_unkn3A_screen(0);
        break;
    default:
        LOGERR("DisplayMode %d empty\n", (int)ingame.DisplayMode);
        break;
    }
}

/** Draws simple purple rect directly, without drawlists.
 */
void draw_purple_rect(int x, int y, int w, int h, ubyte active)
{
    TbPixel col1, col2;

    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    if (active) {
        col1 = 0x0E;
        col2 = 0x0C;
    } else {
        col1 = 0x10;
        col2 = 0x0E;
    }
    LbDrawBox(x, y, w, h, col1);
    LbDrawLine(x, y, x + w - 2, y, col2);
    LbDrawLine(x, y, x, y + h - 2, col2);
}

ubyte critical_action_input(void)
{
    char locstr[52];
    TbKeyCode key;

    strcpy(locstr, "Confirm Critical Action Y/N?");
    DangerMusicFadeSwitch(2, 4u);
    SetMusicTempo(110, 1000);
    {
        int x, y, w, h;
        short tx_height;

        tx_height = my_char_height('A');
        w = my_string_width(locstr) + 2 * tx_height;
        h = 3 * tx_height;
        x = (lbDisplay.GraphicsWindowWidth - w) / 2;
        y = (lbDisplay.GraphicsWindowHeight - h) / 2;

        draw_purple_rect(x, y, w, h, 0);
        draw_text(x + tx_height, y + tx_height, locstr, 0xC);
    }

    key = screen_wait_seconds_or_until_yesno_key(20);

    DangerMusicFadeSwitch(1, 1);
    SetMusicTempoNormal();

    return (key == KC_Y);
}

ubyte process_send_person(ushort player, int i)
{
    ubyte ret;
    asm volatile ("call ASM_process_send_person\n"
        : "=r" (ret) : "a" (player), "d" (i));
    return ret;
}

void do_agent_track_only(void)
{
    static const TbKeyCode sel_agent_keys[] = {
        KC_1, KC_2, KC_3, KC_4,
    };
    PlayerInfo *p_locplayer;
    int n;

    p_locplayer = &players[local_player_no];

    for (n = 0; n < (int)(sizeof(sel_agent_keys)/sizeof(sel_agent_keys[0])); n++)
    {
        TbKeyCode kkey = sel_agent_keys[n];
        if (is_key_pressed(kkey, KMod_DONTCARE))
        {
            struct Thing *p_agent;

            p_agent = p_locplayer->MyAgent[n];
            ingame.TrackX = PRCCOORD_TO_MAPCOORD(p_agent->X);
            ingame.TrackZ = PRCCOORD_TO_MAPCOORD(p_agent->Z);
            engn_xc = ingame.TrackX;
            engn_zc = ingame.TrackZ;
        }
    }
}

void input_packet_playback(void)
{
    struct Packet *p_pckt;

    p_pckt = &packets[local_player_no];

    if (is_key_pressed(KC_ESCAPE, KMod_SHIFT))
    {
        clear_key_pressed(KC_ESCAPE);
        if (critical_action_input())
        {
            exit_game = 1;
            p_pckt->Action = PAct_MISSN_ABORT;
        }
    }
    do_scroll_map();
    do_rotate_map();

    do_agent_track_only();
}

ubyte input_mission_cheats(void)
{
    if ((ingame.UserFlags & UsrF_Cheats) != 0)
    {
        if (is_key_pressed(KC_C, KMod_ALT))
        {
            clear_key_pressed(KC_C);
            mission_result = 1;
        }
        if (is_key_pressed(KC_F, KMod_ALT))
        {
            clear_key_pressed(KC_F);
            mission_result = -1;
        }
    }
    return GINPUT_NONE;
}

void draw_mission_concluded(void)
{
    uint tm;

    tm = (dos_clock() - ingame.fld_unkC91) / 100;
    if (ingame.fld_unkCB5)
    {
        sprintf(unknmsg_str, "%s %s: %s ", gui_strings[GSTR_CHK_MISSION_STA_PRE],
          gui_strings[GSTR_ENM_MISSION_STATUS + 1 + ingame.MissionStatus], scroll_text);
        data_15319c = unknmsg_str;
    }
    else
    {
        uint tm_h, tm_m, tm_s;

        tm_h = tm / 3600;
        tm_m = tm / 60;
        tm_s = tm % 60;

        sprintf(unknmsg_str, "%s %s %s %s %02d:%02d:%02d", gui_strings[GSTR_CHK_MISSION_STA_PRE],
          gui_strings[GSTR_ENM_MISSION_STATUS + 1 + ingame.MissionStatus],
          gui_strings[GSTR_CHK_MISSION_STA_SUF_KEYS], gui_strings[GSTR_CHK_MISSION_STA_TIME],
          tm_h, tm_m % 60, tm_s);
        data_15319c = unknmsg_str;
        scroll_text = unknmsg_str;
        LbStringToUpper(unknmsg_str);
    }
    {
        int scr_x, scr_y;

        // TODO the text position should be computed based on position of panels loaded from file
        scr_x = 11 * pop1_sprites_scale;
        scr_y = 26 * pop1_sprites_scale;

        lbDisplay.DrawColour = SCANNER_colour[0];
        AppTextDrawMissionStatus(scr_x, scr_y, data_15319c);
    }
}

void input_mission_concluded(void)
{
    if (is_key_pressed(KC_RETURN, KMod_DONTCARE) ||
      is_key_pressed(KC_SPACE, KMod_DONTCARE) ||
      is_key_pressed(KC_ESCAPE, KMod_NONE))
    {
        if (ingame.MissionStatus != ObvStatu_MissTmp_FAILED || is_key_pressed(KC_ESCAPE, KMod_NONE))
        {
          clear_key_pressed(KC_ESCAPE);
          clear_key_pressed(KC_SPACE);
          clear_key_pressed(KC_RETURN);
          ingame.MissionEndFade = 1;
        }
        else
        {
          simulate_key_press(KC_R);
          clear_key_pressed(KC_SPACE);
          clear_key_pressed(KC_RETURN);
        }
    }
}

void load_packet(void)
{
    PlayerInfo *p_locplayer;
    struct Packet *p_pckt;
    ubyte did_inp;

    p_locplayer = &players[local_player_no];
    p_pckt = &packets[local_player_no];

    game_graphics_inputs();
    if (PacketRecord_IsPlayback()) // packet replay controls
    {
        if (!in_network_game)
            PacketRecord_Read(p_pckt);
        input_packet_playback();
        ingame.MissionStatus = test_missions(0);
        return;
    }

    did_inp = GINPUT_NONE;
    if (ingame.DisplayMode == DpM_ENGINEPLY || ingame.DisplayMode == DpM_UNKN_3B)
    {
        did_inp |= do_user_interface();
        if ((did_inp & GINPUT_PACKET) == 0)
            did_inp |= input_agents_cheats();
        if ((did_inp & GINPUT_PACKET) == 0)
            did_inp |= input_mission_cheats();
        ingame.MissionStatus = test_missions(0);
        if ((ingame.MissionStatus != ObvStatu_UNDECIDED) && !in_network_game)
        {
            draw_mission_concluded();
            input_mission_concluded();
            if (ingame.MissionEndFade == 0)
            {
                struct Mission *p_missi;
                int fade_turns;

                fade_turns = 0;
                p_missi = &mission_list[ingame.CurrentMission];
                if ((ingame.MissionStatus == ObvStatu_COMPLETED) &&
                  ((p_missi->WaitToFade & WTFade_ON_SUCCESS) != 0))
                    fade_turns = (p_missi->WaitToFade & 0x1FFF);
                if ((ingame.MissionStatus == ObvStatu_MissTmp_FAILED) &&
                  ((p_missi->WaitToFade & WTFade_ON_FAIL) != 0))
                    fade_turns = (p_missi->WaitToFade & 0x1FFF);
                if (fade_turns != 0) {
                    LOGSYNC("Mission end fade initiated, status=%d, turns=%d",
                      (int)ingame.MissionStatus, fade_turns);
                    ingame.MissionEndFade = fade_turns;
                }
            }
        }
    }

    if (ingame.MissionEndFade != 0)
    {
        if (ingame.MissionEndFade < 30)
            change_brightness(-2);
        ingame.MissionEndFade--;
        if (ingame.MissionEndFade == 0)
        {
            init_level_3d(1);
            if (is_single_game)
                exit_game = 1;
            mission_over();
        }
    }
    if (did_inp == GINPUT_EXIT)
    {
        if (is_single_game) {
            exit_game = 1;
        } else {
            init_level_3d(1);
            if (ingame.MissionStatus != ObvStatu_COMPLETED)
                ingame.MissionStatus = ObvStatu_MissTmp_FAILED;
            mission_over();
        }
    }

    if ((did_inp == 0) || (p_locplayer->DoubleMode != 0))
    {
        short dmuser;

        for (dmuser = 0; dmuser < p_locplayer->DoubleMode + 1; dmuser++)
        {
            if (p_locplayer->DoubleMode != 0) {
                ulong ctlmode;
                ctlmode = p_locplayer->UserInput[dmuser].ControlMode & ~UInpCtr_AllFlagsMask;
                if (ctlmode == UInpCtr_Mouse)
                    continue;
            }
            if (ingame.TrackThing != 0)
                continue;

            did_inp |= input_user_control_agent(local_player_no, dmuser);
        }
    }

    if (p_locplayer->PanelState[mouser] != PANEL_STATE_SEND_MESSAGE)
    {
        if (in_network_game || (ingame.UserFlags & UsrF_Cheats) != 0)
        {
            if (!in_network_game && is_key_pressed(KC_ESCAPE, KMod_CONTROL))
            {
                clear_key_pressed(KC_ESCAPE);
                exit_game = 1;
            }
            else if (is_key_pressed(KC_ESCAPE, KMod_SHIFT))
            {
                clear_key_pressed(KC_ESCAPE);
                if (in_network_game || critical_action_input()) {
                    change_brightness(-32);
                    p_pckt->Action = PAct_MISSN_ABORT;
                    did_inp |= GINPUT_EXIT;
                }
            }
        }

        if (PacketRecord_IsRecord() && !in_network_game)
        {
            PacketRecord_Write(p_pckt);
        }
    }
}

void joy_input(void)
{
    asm volatile ("call ASM_joy_input\n"
        :  :  : "eax" );
}

/** Orbital station explosion code.
 */
void game_process_orbital_station_explode(void)
{
    if (unkn01_downcount > 0)
    {
        unkn01_downcount--;
        LOGDBG("unkn01_downcount = %ld", unkn01_downcount);
        if ( unkn01_downcount == 40 ) {
            mapwho_unkn01(unkn01_pos_x, unkn01_pos_y);
        }
        else if (unkn01_downcount < 40) {
            ushort stl_x, stl_y;

            stl_y = unkn01_pos_y + (LbRandomAnyShort() & 0xF) - 7;
            stl_x = unkn01_pos_x + (LbRandomAnyShort() & 0xF) - 7;
            bang_new4(stl_x << 16, 0, stl_y << 16, 95);
            stl_y = unkn01_pos_y + (LbRandomAnyShort() & 0xF) - 7;
            stl_x = unkn01_pos_x + (LbRandomAnyShort() & 0xF) - 7;
            bang_new4(stl_x << 16, 0, stl_y << 16, 95);
        }
    }
}

void game_process(void)
{
    debug_multicolor_sprite(193);
    LOGDBG("WSCREEN 0x%p", (void *)lbDisplay.WScreen);

    while ( !exit_game )
    {
        process_sound_heap();
        navi2_unkn_counter -= 2;
        if (navi2_unkn_counter < 0)
            navi2_unkn_counter = 0;
        if (navi2_unkn_counter > navi2_unkn_counter_max)
            navi2_unkn_counter_max = navi2_unkn_counter;
        if (keyboard_mode_direct)
            input_char = LbKeyboard();
        if (ingame.DisplayMode == DpM_PURPLEMNU) {
            LOGDBG("id=%d  trial alloc = %d turn %lu", 0, triangulation, gameturn);
        }
        input();
        update_tick_time();
        draw_game();
        debug_trace_turn_bound(gameturn);
        load_packet();
        if ( ((active_flags_general_unkn01 & 0x8000) != 0) !=
          ((ingame.Flags & GamF_ThermalView) != 0) )
            LbPaletteSet(display_palette);
        active_flags_general_unkn01 = ingame.Flags;
        if ((ingame.DisplayMode == DpM_ENGINEPLY)
          || (ingame.DisplayMode == DpM_UNKN_1)
          || (ingame.DisplayMode == DpM_UNKN_3B))
            process_things();
        if (debug_hud_things)
            things_debug_hud();
        if (ingame.DisplayMode != DpM_PURPLEMNU)
            process_packets();
        joy_input();

        if (ingame.DisplayMode == DpM_PURPLEMNU)
        {
            game_update();
            swap_wscreen();
        }
        else if (!skip_redraw_this_turn())
        {
            game_update();
            LbScreenSwapClear(0);
        }

        update_unkn_changing_colors();
        game_process_orbital_station_explode();
        gameturn++;
        render_anim_turn = gameturn;
        scene_post_effect_prepare();
    }
    PacketRecord_Close();
    LbPaletteFade(NULL, 0x10u, 1);
}

void game_quit(void)
{
    host_reset();
    LbBaseReset();
    exit(0);
}

static void game_update_full(bool wait)
{
    display_unlock();

    game_handle_sdl_events();

    if (wait)
    {
        wait_next_gameturn();
    }

    display_lock();
}

int game_hacky_update(void)
{
    game_update_full(false);
    return 1;
}

void game_update(void)
{
    game_update_full(true);
}

void engine_reset(void)
{
    LbMemoryFree(engine_mem_alloc_ptr);
}

void host_reset(void)
{
    StopCD();
    setup_heaps(SHSC_ResetGameSnd, language_3str);
    FreeAudio();
    engine_reset();
    reset_multicolor_sprites();
    reset_mouse_pointers();
    LbMouseReset();
    LbKeyboardClose();
    LbScreenReset();
    LbNetworkReset();
}

void game_reset(void)
{
    host_reset();
    free_texturemaps();
    LbDataFreeAll(missionspr_load_files);
}

/******************************************************************************/
