/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_save.c
 *     Loading and saving the game and user profile.
 * @par Purpose:
 *     Functions for storing and loading game state data and settings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 02 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "game_save.h"

#include "bffile.h"
#include "bfstrut.h"
#include <assert.h>

#include "bfjoyst.h"

#include "campaign.h"
#include "display.h"
#include "engindrwlstm.h"
#include "febrief.h"
#include "femail.h"
#include "femain.h"
#include "game_data.h"
#include "game_options.h"
#include "game.h"
#include "keyboard.h"
#include "misstat.h"
#include "player.h"
#include "research.h"
#include "sound.h"
#include "swlog.h"
#include "wrcities.h"

/******************************************************************************/

ubyte *save_game_buffer = NULL;

extern ubyte save_crypto_tables_state[3];
extern ubyte save_crypto_data_state[3];

char save_slot_names[SAVE_SLOTS_VISIBLE_COUNT][25] = {0};
char save_active_desc[25];

/******************************************************************************/

void save_crypto_make_hashtable(ubyte simple_salt)
{
    asm volatile ("call ASM_save_crypto_make_hashtable\n"
        :  : "a" (simple_salt));
}

ulong save_crypto_transform1(ubyte a1)
{
    ulong ret;
    asm volatile ("call ASM_save_crypto_transform1\n"
        : "=r" (ret) : "a" (a1));
    return ret;
}

ulong save_crypto_transform2(ubyte a1)
{
    ulong ret;
    asm volatile ("call ASM_save_crypto_transform2\n"
        : "=r" (ret) : "a" (a1));
    return ret;
}

ulong save_crypto_transform3(ubyte a1)
{
    ulong ret;
    asm volatile ("call ASM_save_crypto_transform3\n"
        : "=r" (ret) : "a" (a1));
    return ret;
}

u32 save_buffer_hash(ubyte *save_buf, u32 save_buf_len)
{
    u32 *cpos;
    u32 clen, hash;
    u32 i;

    if (save_buf_len & 3)
        clen = save_buf_len + 4;
    else
        clen = save_buf_len;
    clen >>= 2;
    cpos = (u32 *)save_game_buffer;

    hash = *cpos;
    for (i = 1; i < clen; i++) {
        cpos++;
        hash ^= *cpos;
    }

    return hash;
}

void save_crypto_docrypt(ubyte *save_buf, u32 save_buf_len)
{
    ubyte *gbpos;
    ubyte *gbend;

    gbpos = save_game_buffer;
    gbend = &save_game_buffer[4 * (save_buf_len >> 2)];
    while (gbpos < gbend)
    {
        u32 key, keysel;
        u32 *cryptpos;
        keysel = (save_crypto_data_state[1] << 8) | (save_crypto_data_state[0] << 16) | save_crypto_data_state[2];
        cryptpos = (u32 *)gbpos;
        switch (keysel)
        {
        case 0x00001:
        case 0x10100:
            key = save_crypto_transform1(1);
            key ^= save_crypto_transform2(1);
            key ^= save_crypto_transform3(0);
            break;
        case 0x00100:
        case 0x10001:
            key = save_crypto_transform1(1);
            key ^= save_crypto_transform2(0);
            key ^= save_crypto_transform3(1);
            break;
        case 0x00000:
        case 0x10101:
            key = save_crypto_transform1(1);
            key ^= save_crypto_transform2(1);
            key ^= save_crypto_transform3(1);
            break;
        case 0x00101:
        case 0x10000:
            key = save_crypto_transform1(0);
            key ^= save_crypto_transform2(1);
            key ^= save_crypto_transform3(1);
            break;
        default:
            key = 0;
            break;
        }
        gbpos += 4;
        *cryptpos ^= key;
    }
}

TbBool save_game_decrypt_and_verify(u32 fmtver, TbBool simple_salt, ubyte *save_buf, u32 save_buf_len, u32 decrypt_verify)
{
    if (fmtver >= 9)
    {
        // Decrypt the file
        save_crypto_make_hashtable(simple_salt);
        save_crypto_docrypt(save_buf, save_buf_len);
    }

    // Verify data
    if (save_buffer_hash(save_buf, save_buf_len) != decrypt_verify)
        return false;

    return true;
}

u32 save_game_encrypt(u32 fmtver, TbBool simple_salt, ubyte *save_buf, u32 save_buf_len)
{
    u32 decrypt_verify;

    decrypt_verify = save_buffer_hash(save_buf, save_buf_len);

    save_crypto_make_hashtable(simple_salt);
    save_crypto_docrypt(save_buf, save_buf_len);

    return decrypt_verify;
}

void resave_salt_to_keys(void)
{
    char locstr[DISKPATH_SIZE];
    PathInfo *pinfo;
    TbFileHandle fh;
    int i;

    pinfo = &game_dirs[DirPlace_Savegame];
    snprintf(locstr, sizeof(locstr), "%s/keys.dat", pinfo->directory);
    fh = LbFileOpen(locstr, Lb_FILE_MODE_OLD);
    if (fh == INVALID_FILE)
    {
        return;
    }
    i = sizeof(save_mortal_salt);
    // Add salts at end, storing all salts ever used with this copy
    LbFileSeek(fh, 0, Lb_FILE_SEEK_END);
    LbFileWrite(fh, &save_mortal_salt, i);
    LbFileClose(fh);
}

TbResult reload_salt_from_keys(void)
{
    char locstr[DISKPATH_SIZE];
    PathInfo *pinfo;
    TbFileHandle fh;
    int i;

    pinfo = &game_dirs[DirPlace_Savegame];
    snprintf(locstr, sizeof(locstr), "%s/keys.dat", pinfo->directory);
    if (!LbFileExists(locstr))
    {
        return Lb_FAIL;
    }
    fh = LbFileOpen(locstr, Lb_FILE_MODE_READ_ONLY);
    if (fh == INVALID_FILE)
    {
        return Lb_FAIL;
    }
    i = sizeof(save_mortal_salt);
    LbFileSeek(fh, -i, Lb_FILE_SEEK_END);
    LbFileRead(fh, &save_mortal_salt, i);
    LbFileClose(fh);
    return Lb_SUCCESS;
}

void apply_user_settings(void)
{
    apply_user_gfx_settings();
    apply_user_sfx_settings();
}

void set_default_user_settings(void)
{
    set_default_game_keys();
    set_default_player_control();
    set_default_sfx_settings();
    set_default_gfx_settings();
    set_default_visual_prefernces();
    set_default_audio_tracks();
    apply_user_settings();
}

void read_user_settings(void)
{
    char fname[DISKPATH_SIZE];
    TbFileHandle fh;
    u32 fmtver;
    TbBool read_mortal_salt_backup;
    int i;

    read_mortal_salt_backup = false;
    fh = INVALID_FILE;
    get_user_settings_fname(fname, login_name);
    if (login_name[0] == '\0')
        read_mortal_salt_backup = true;

    if (LbFileExists(fname))
        fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);

    // Try default settings file instead
    if ((fh == INVALID_FILE) && (strlen(login_name) > 0))
    {
        get_user_settings_fname(fname, "");
        if (LbFileExists(fname))
            fh = LbFileOpen(fname, Lb_FILE_MODE_READ_ONLY);
        read_mortal_salt_backup = true;
    }

    if (fh == INVALID_FILE)
    {
        LOGSYNC_F("Build-in defaults used, as could not open \"%s\" file", fname);
        set_default_user_settings();
        read_mortal_salt_backup = true;
    } else
    {
        ushort locflags;

        assert(sizeof(locflags) == sizeof(ingame.UserFlags));

        if (LbFileLengthHandle(fh) > 126)
            LbFileRead(fh, &fmtver, sizeof(u32));
        else
            fmtver = 0;

        if (fmtver == 0) {
            // In original game, the last key (agent 4 select) is not saved at all
            set_default_game_keys();
            LbFileRead(fh, kbkeys, 23 * sizeof(ushort));
            LbFileRead(fh, jskeys, 23 * sizeof(ushort));
        } else if (fmtver == 1) {
            set_default_game_keys();
            LbFileRead(fh, kbkeys, 29 * sizeof(ushort));
            LbFileRead(fh, jskeys, 29 * sizeof(ushort));
        } else if (fmtver == 2) {
            set_default_game_keys();
            LbFileRead(fh, kbkeys, 50 * sizeof(ushort));
            LbFileRead(fh, jskeys, 50 * sizeof(ushort));
        } else {
#ifdef MORE_GAME_KEYS
            if (fmtver != 3)
#else
            if (fmtver != 2)
#endif
                LOGWARN("Settings may be invalid, as \"%s\" has unrecognized format version %d", fname, (int)fmtver);
            LbFileRead(fh, kbkeys, GKey_KEYS_COUNT * sizeof(ushort));
            LbFileRead(fh, jskeys, GKey_KEYS_COUNT * sizeof(ushort));
        }

        LbFileRead(fh, &ctl_joystick_type, sizeof(ctl_joystick_type));
        LbFileRead(fh, &players[local_player_no].DoubleMode,
          sizeof(players[local_player_no].DoubleMode));
        for (i = 0; i != 4; i++)
        {
            PlayerInfo *p_locplayer;
            p_locplayer = &players[local_player_no];
            LbFileRead(fh, &p_locplayer->UserInput[i].ControlMode,
              sizeof(p_locplayer->UserInput[i].ControlMode));
        }
        LbFileRead(fh, &startscr_samplevol, sizeof(startscr_samplevol));
        LbFileRead(fh, &startscr_midivol, sizeof(startscr_midivol));
        LbFileRead(fh, &startscr_cdvolume, sizeof(startscr_cdvolume));
        LbFileRead(fh, &game_gfx_advanced_lights, sizeof(game_gfx_advanced_lights));
        LbFileRead(fh, &game_billboard_movies, sizeof(game_billboard_movies));
        LbFileRead(fh, &game_gfx_deep_radar, sizeof(game_gfx_deep_radar));
        LbFileRead(fh, &ingame.DetailLevel, sizeof(ingame.DetailLevel));
        LbFileRead(fh, &game_high_resolution, sizeof(game_high_resolution));
        LbFileRead(fh, &game_projector_speed, sizeof(game_projector_speed));
        LbFileRead(fh, &game_perspective, sizeof(game_perspective));
        LbFileRead(fh, &ingame.PanelPermutation, sizeof(ingame.PanelPermutation));
        LbFileRead(fh, &ingame.TrenchcoatPreference, sizeof(ingame.TrenchcoatPreference));
        LbFileRead(fh, &ingame.CDTrack, sizeof(ingame.CDTrack));
        LbFileRead(fh, &ingame.DangerTrack, sizeof(ingame.DangerTrack));
        LbFileRead(fh, &ingame.UseMultiMedia, sizeof(ingame.UseMultiMedia));
        LbFileRead(fh, &locflags, sizeof(locflags));
        locflags &= ~UsrF_Cheats; // Cheats are per-session setting, not per-user
        ingame.UserFlags |= locflags; // Do not replace these, add to them
        LbFileRead(fh, &save_mortal_salt, sizeof(save_mortal_salt));
        LbFileClose(fh);

        apply_user_settings();
    }

    if (read_mortal_salt_backup)
    {
        // Read mortal game encryption salt from backup
        reload_salt_from_keys();
    }

    i = -1;
    if (ctl_joystick_type)
        i = joy_setup_device(&joy, ctl_joystick_type);
    if (i != 1)
        ctl_joystick_type = JTyp_NONE;
}

TbBool save_user_settings(void)
{
    char fname[DISKPATH_SIZE];
    TbFileHandle fh;
    u32 fmtver;
    int i;

    get_user_settings_fname(fname, login_name);
#ifdef MORE_GAME_KEYS
    fmtver = 3;
#else
    fmtver = 2;
#endif

    fh = LbFileOpen(fname, Lb_FILE_MODE_NEW);
    if (fh == INVALID_FILE)
        return 1;

    LbFileWrite(fh, &fmtver, sizeof(fmtver));
    LbFileWrite(fh, kbkeys, GKey_KEYS_COUNT * sizeof(ushort));
    LbFileWrite(fh, jskeys, GKey_KEYS_COUNT * sizeof(ushort));
    LbFileWrite(fh, &ctl_joystick_type, sizeof(ctl_joystick_type));
    LbFileWrite(fh, &players[local_player_no].DoubleMode,
      sizeof(players[local_player_no].DoubleMode));
    for (i = 0; i < 4; i++)
    {
        PlayerInfo *p_locplayer;
        p_locplayer = &players[local_player_no];
        LbFileWrite(fh, &p_locplayer->UserInput[i].ControlMode,
          sizeof(p_locplayer->UserInput[i].ControlMode));
    }
    LbFileWrite(fh, &startscr_samplevol, sizeof(startscr_samplevol));
    LbFileWrite(fh, &startscr_midivol, sizeof(startscr_midivol));
    LbFileWrite(fh, &startscr_cdvolume, sizeof(startscr_cdvolume));
    LbFileWrite(fh, &game_gfx_advanced_lights, sizeof(game_gfx_advanced_lights));
    LbFileWrite(fh, &game_billboard_movies, sizeof(game_billboard_movies));
    LbFileWrite(fh, &game_gfx_deep_radar, sizeof(game_gfx_deep_radar));
    LbFileWrite(fh, &ingame.DetailLevel, sizeof(ingame.DetailLevel));
    LbFileWrite(fh, &game_high_resolution, sizeof(game_high_resolution));
    LbFileWrite(fh, &game_projector_speed, sizeof(game_projector_speed));
    LbFileWrite(fh, &game_perspective, sizeof(game_perspective));
    LbFileWrite(fh, &ingame.PanelPermutation, sizeof(ingame.PanelPermutation));
    LbFileWrite(fh, &ingame.TrenchcoatPreference, sizeof(ingame.TrenchcoatPreference));
    LbFileWrite(fh, &ingame.CDTrack, sizeof(ingame.CDTrack));
    LbFileWrite(fh, &ingame.DangerTrack, sizeof(ingame.DangerTrack));
    LbFileWrite(fh, &ingame.UseMultiMedia, sizeof(ingame.UseMultiMedia));
    LbFileWrite(fh, &ingame.UserFlags, sizeof(ingame.UserFlags));
    LbFileWrite(fh, &save_mortal_salt, sizeof(save_mortal_salt));
    LbFileClose(fh);
    return 0;
}

int save_game_write(ubyte slot, char *desc)
{
    char locstr[32];
    PlayerInfo *p_locplyr;
    TbFileHandle fh;
    u32 gblen, fmtver;
    u32 decrypt_verify;
    TbBool simple_salt;
    int i;

    fmtver = 12;
    gblen = 0;
    memcpy(&save_game_buffer[gblen], &ingame.Credits, sizeof(ingame.Credits));
    assert(sizeof(ingame.Credits) == 4);
    gblen += sizeof(ingame.Credits);

    memcpy(&save_game_buffer[gblen], &cryo_agents, sizeof(cryo_agents));
    assert(sizeof(cryo_agents) == 389);
    gblen += sizeof(cryo_agents);

    memcpy(&save_game_buffer[gblen], &research, sizeof(research));
    assert(sizeof(research) == 1372);
    gblen += sizeof(research);

    p_locplyr = &players[local_player_no];
    memcpy(&save_game_buffer[gblen], p_locplyr, sizeof(PlayerInfo));
    assert(sizeof(PlayerInfo) == 426);
    gblen += sizeof(PlayerInfo);

    memcpy(&save_game_buffer[gblen], &global_date, sizeof(global_date));
    assert(sizeof(global_date) == 5);
    gblen += sizeof(global_date);

    memcpy(&save_game_buffer[gblen], &research_curr_wep_date, sizeof(research_curr_wep_date));
    assert(sizeof(research_curr_wep_date) == 5);
    gblen += sizeof(research_curr_wep_date);

    memcpy(&save_game_buffer[gblen], &research_curr_mod_date, sizeof(research_curr_mod_date));
    assert(sizeof(research_curr_mod_date) == 5);
    gblen += sizeof(research_curr_mod_date);
    assert(gblen == 2206);

    save_game_buffer[gblen + 0] = next_email;
    save_game_buffer[gblen + 2] = next_brief;
    save_game_buffer[gblen + 4] = old_mission_brief;
    save_game_buffer[gblen + 6] = open_brief;
    save_game_buffer[gblen + 8] = next_ref;
    save_game_buffer[gblen + 10] = new_mail;
    save_game_buffer[gblen + 11] = background_type;
    gblen += 12;

    memcpy(&save_game_buffer[gblen], &mission_status[open_brief], sizeof(mission_status[0]));
    assert(sizeof(mission_status[0]) == 40);
    gblen += sizeof(mission_status[0]);

    memcpy(&save_game_buffer[gblen], email_store, sizeof(struct EmailItem) * next_email);
    assert(sizeof(struct EmailItem) == 5);
    gblen += sizeof(struct EmailItem) * next_email;
    memcpy(&save_game_buffer[gblen], brief_store, sizeof(struct EmailItem) * next_brief);
    gblen += sizeof(struct EmailItem) * next_brief;
    memcpy(&save_game_buffer[gblen], newmail_store, sizeof(struct EmailItem) * new_mail);
    gblen += sizeof(struct EmailItem) * new_mail;

    memcpy(&save_game_buffer[gblen], &ingame.MissionStatus, sizeof(ingame.MissionStatus));
    assert(sizeof(ingame.MissionStatus) == 2);
    gblen += sizeof(ingame.MissionStatus);

    memcpy(&save_game_buffer[gblen], mission_open, sizeof(mission_open));
    assert(sizeof(mission_open) == 2 * 50);
    gblen += sizeof(mission_open);

    memcpy(&save_game_buffer[gblen], mission_state, sizeof(mission_state));
    assert(sizeof(mission_state) == 2 * 50);
    gblen += sizeof(mission_state);

    memcpy(&save_game_buffer[gblen], &next_mission, sizeof(next_mission));
    assert(sizeof(next_mission) == 2);
    gblen += sizeof(next_mission);

    for (i = 1; i < next_mission; i++)
    {
        struct Mission *p_missi;
        p_missi = &mission_list[i];
        memcpy(&save_game_buffer[gblen], p_missi->SpecialTrigger, sizeof(p_missi->SpecialTrigger));
        gblen += sizeof(p_missi->SpecialTrigger);
        save_game_buffer[gblen] = p_missi->Complete;
        gblen += 1;
    }

    memcpy(&save_game_buffer[gblen], login_name, sizeof(login_name));
    gblen += sizeof(login_name);

    for (i = 0; i < num_cities; i++)
    {
        save_game_buffer[gblen] = cities[i].Info;
        gblen += 1;
    }

    save_game_buffer[gblen] = ingame.AutoResearch;
    gblen += 1;

    if (((ingame.Flags & GamF_MortalGame) != 0) || (desc[0] == '\0'))
    {
        struct Campaign *p_campgn;
        p_campgn = &campaigns[background_type];
        sprintf(desc, "%s %02d:%02d:%02d NC", p_campgn->TextName,
          global_date.Day, global_date.Month, global_date.Year);
        LbStringToUpper(desc);
    }
  
    while ((gblen & 7) != 0)
    {
        save_game_buffer[gblen] = 0;
        gblen += 1;
    }

    if ((ingame.Flags & GamF_MortalGame) != 0)
    {
        save_mortal_salt = time(0);
        resave_salt_to_keys();
    }

    simple_salt = ((ingame.Flags & GamF_MortalGame) == 0);
    decrypt_verify = save_game_encrypt(fmtver, simple_salt, save_game_buffer, gblen);

    if ((ingame.Flags & GamF_MortalGame) != 0)
        sprintf(locstr, "qdata/savegame/synwarsm.sav");
    else if (slot >= 9)
        sprintf(locstr, "qdata/savegame/swars%03d.sav", slot - 1);
    else
        sprintf(locstr, "qdata/savegame/synwars%d.sav", slot - 1);

    fh = LbFileOpen(locstr, Lb_FILE_MODE_NEW);
    if (fh == INVALID_FILE) {
        return 1;
    }
    LbFileWrite(fh, desc, 25);
    LbFileWrite(fh, &gblen, sizeof(gblen));
    LbFileWrite(fh, &fmtver, sizeof(fmtver));
    LbFileWrite(fh, save_game_buffer, gblen);
    LbFileWrite(fh, &decrypt_verify, sizeof(decrypt_verify));
    LbFileClose(fh);

    save_user_settings();

    return 0;
}

void load_save_slot_names(void)
{
    char locstr[DISKPATH_SIZE];
    int i;

    for (i = 0; i < SAVE_SLOTS_VISIBLE_COUNT + 1; i++)
    {
        TbFileHandle fh;
        int slot;

        // the last slot is for mortal game
        if (i == SAVE_SLOTS_VISIBLE_COUNT)
            slot = 0;
        else
            slot = save_slot_base + i + 1;
        get_saved_game_fname(locstr, slot);

        if (!LbFileExists(locstr)) {
            save_slot_names[i][0] = '\0';
            continue;
        }

        fh = LbFileOpen(locstr, Lb_FILE_MODE_READ_ONLY);
        if (fh == INVALID_FILE) {
            save_slot_names[i][0] = '\0';
            continue;
        }

        if (LbFileRead(fh, save_slot_names[i], 25) != 25)
            save_slot_names[i][0] = '\0';
        LbFileClose(fh);
    }
}

ubyte load_game(int slot, char *desc)
{
    char locstr[DISKPATH_SIZE];
    u32 gblen, fmtver, decrypt_verify;
    TbFileHandle fh;
    TbBool ok;

    get_saved_game_fname(locstr, slot);

    fh = LbFileOpen(locstr, Lb_FILE_MODE_READ_ONLY);
    if (fh == INVALID_FILE)
        return 1;
    LbFileRead(fh, desc, 25);
    LbFileRead(fh, &gblen, 4);
    LbFileRead(fh, &fmtver, sizeof(u32));
    LbFileRead(fh, save_game_buffer, gblen);
    LbFileRead(fh, &decrypt_verify, 4);
    LbFileClose(fh);

    ok = save_game_decrypt_and_verify(fmtver, slot != 0, save_game_buffer, gblen, decrypt_verify);
    if (!ok) return 2;

    gblen = 0;
    memcpy(&ingame.Credits, &save_game_buffer[gblen], sizeof(ingame.Credits));
    assert(sizeof(ingame.Credits) == 4);
    gblen += sizeof(ingame.Credits);

    if (fmtver >= 5)
    {
        ushort cryo_no;
        memcpy(&cryo_agents, &save_game_buffer[gblen], sizeof(struct AgentInfo));
        gblen += sizeof(struct AgentInfo);
        for (cryo_no = 0; cryo_no < CRYO_PODS_MAX_COUNT; cryo_no++)
        {
            // Remove bad mod flags
            cybmod_fix_all(&cryo_agents.Mods[cryo_no]);
            // Check weapons count, reset bad amounts of consumable weapons
            sanitize_weapon_quantities(&cryo_agents.Weapons[cryo_no],
              &cryo_agents.FourPacks[cryo_no]);
        }
    }
    else
    {
        ushort cryo_no;
        memcpy(&cryo_agents, &save_game_buffer[gblen], offsetof(struct AgentInfo, FourPacks));
        gblen += offsetof(struct AgentInfo, FourPacks);
        memcpy(&cryo_agents.NumAgents, &save_game_buffer[gblen], 1);
        gblen += 1;
        for (cryo_no = 0; cryo_no < CRYO_PODS_MAX_COUNT; cryo_no++)
        {
            // Remove bad mod flags
            cybmod_fix_all(&cryo_agents.Mods[cryo_no]);
            // Check weapons count, reset bad amounts of consumable weapons
            sanitize_weapon_quantities(&cryo_agents.Weapons[cryo_no],
              &cryo_agents.FourPacks[cryo_no]);
        }
    }

    if (fmtver >= 3)
    {
        assert(sizeof(struct ResearchInfo) == 1372);
        memcpy(&research, &save_game_buffer[gblen], sizeof(struct ResearchInfo));
        gblen += sizeof(struct ResearchInfo);
    }
    else
    {
        int i, k;
        // Old version has one byte progress
        for (i = 0; i < 32; i++)
        {
            for (k = 0; k < 10; k++)
            {
                research.WeaponProgress[i][k] = save_game_buffer[gblen];
                gblen++;
            }
        }
        for (i = 0; i < 32; i++)
        {
            for (k = 0; k < 10; k++)
            {
                research.ModProgress[i][k] = save_game_buffer[gblen];
                gblen++;
            }
        }
        i = sizeof(struct ResearchInfo) - offsetof(struct ResearchInfo, WeaponDaysDone);
        assert(i == 92);
        memcpy(research.WeaponDaysDone, &save_game_buffer[gblen], i);
        gblen += 732; // and not 92? we don't have old saves, so cannot verify.
    }

    if (fmtver >= 7)
    {
        PlayerInfo *p_locplayer;
        int i;
        assert(sizeof(PlayerInfo) == 426);
        p_locplayer = &players[local_player_no];
        memcpy(p_locplayer, &save_game_buffer[gblen], sizeof(PlayerInfo));
        gblen += sizeof(PlayerInfo);
        // Remove bad mod flags
        for (i = 0; i < 4; i++)
        {
            cybmod_fix_all(&p_locplayer->Mods[i]);
        }
    }
    else if (fmtver >= 4)
    {
        PlayerInfo *p_locplayer;
        int i;
        p_locplayer = &players[local_player_no];

        i = sizeof(PlayerInfo) - offsetof(PlayerInfo, WepDelays);
        assert(i == 282);
        memcpy(p_locplayer, &save_game_buffer[gblen], i);
        gblen += i;
        // The old struct matches until WepDelays
        i = 48;
        memcpy(p_locplayer->WepDelays, &save_game_buffer[gblen], i);
        gblen += i;
        // Remove bad mod flags
        for (i = 0; i < 4; i++)
        {
            cybmod_fix_all(&p_locplayer->Mods[i]);
        }
    }
    else
    {
        PlayerInfo *p_locplayer;
        int i;

        p_locplayer = &players[local_player_no];

        i = sizeof(PlayerInfo) - offsetof(PlayerInfo, FourPacks);
        assert(i == 262);
        memcpy(p_locplayer, &save_game_buffer[gblen], i);
        gblen += i;
        // The old struct matches until FourPacks
        i = 8;
        memcpy(p_locplayer->FourPacks, &save_game_buffer[gblen], i);
        gblen += i;

        for (i = 0; i < 4; i++)
        {
            // Remove bad mod flags
            cybmod_fix_all(&p_locplayer->Mods[i]);
            // Reset bad amounts of consumable weapons
            if (weapons_has_weapon(p_locplayer->Weapons[i], WEP_NUCLGREN))
                p_locplayer->FourPacks[WFRPK_NUCLGREN][i] = 1;
            if (weapons_has_weapon(p_locplayer->Weapons[i], WEP_ELEMINE))
                p_locplayer->FourPacks[WFRPK_ELEMINE][i] = 1;
            if (weapons_has_weapon(p_locplayer->Weapons[i], WEP_EXPLMINE))
                p_locplayer->FourPacks[WFRPK_EXPLMINE][i] = 1;
            if (weapons_has_weapon(p_locplayer->Weapons[i], WEP_KOGAS))
                p_locplayer->FourPacks[WFRPK_KOGAS][i] = 1;
            if (weapons_has_weapon(p_locplayer->Weapons[i], WEP_CRAZYGAS))
                p_locplayer->FourPacks[WFRPK_CRAZYGAS][i] = 1;
        }

        for (i = 0; i < LOCAL_USERS_MAX_COUNT; i++)
        {
            p_locplayer->UserVX[i] = 0;
            p_locplayer->UserVZ[i] = 0;
        }

        player_agents_clear_weapon_delays(local_player_no);
    }
    players_sync_from_cryo();

    memcpy(&global_date, &save_game_buffer[gblen], sizeof(struct SynTime));
    gblen += sizeof(struct SynTime);
    memcpy(&research_curr_wep_date, &save_game_buffer[gblen], sizeof(struct SynTime));
    gblen += sizeof(struct SynTime);
    memcpy(&research_curr_mod_date, &save_game_buffer[gblen], sizeof(struct SynTime));
    gblen += sizeof(struct SynTime);

    next_email = save_game_buffer[gblen + 0];
    next_brief = save_game_buffer[gblen + 2];
    old_mission_brief = save_game_buffer[gblen + 4];
    open_brief = save_game_buffer[gblen + 6];
    next_ref = save_game_buffer[gblen + 8];
    new_mail = save_game_buffer[gblen + 10];
    background_type = save_game_buffer[gblen + 11];
    gblen += 12;

    load_missions(background_type);

    if (fmtver >= 12)
    {
        int i;
        i = sizeof(struct MissionStatus);
        assert(i == 40);
        memcpy(&mission_status[open_brief], &save_game_buffer[gblen], i);
        gblen += i;
    }
    else if (fmtver >= 10)
    {
        struct MissionStatus *p_mistat;
        int i;
        i = sizeof(struct MissionStatus) - offsetof(struct MissionStatus, Expenditure);
        assert(i == 32);
        p_mistat = &mission_status[open_brief];
        memcpy(p_mistat, &save_game_buffer[gblen], i);
        gblen += i;
        gblen += 2;
        p_mistat->AgentsLost = save_game_buffer[gblen];
        gblen++;
        p_mistat->AgentsGained = save_game_buffer[gblen];
        gblen++;
    }
    else
    {
        // Mission status block did not existed in this version
    }

    memcpy(email_store, &save_game_buffer[gblen], 5 * next_email);
    gblen += 5 * next_email;
    memcpy(brief_store, &save_game_buffer[gblen], 5 * next_brief);
    gblen += 5 * next_brief;
    memcpy(newmail_store, &save_game_buffer[gblen], 5 * new_mail);
    gblen += 5 * new_mail;

    {
        int i;
        memcpy(&ingame.MissionStatus, &save_game_buffer[gblen], sizeof(ingame.MissionStatus));
        gblen += sizeof(ingame.MissionStatus);
        i = sizeof(mission_open);
        assert(i == 100);
        memcpy(mission_open, &save_game_buffer[gblen], i);
        gblen += i;
        i = sizeof(mission_state);
        assert(i == 100);
        memcpy(mission_state, &save_game_buffer[gblen], i);
        gblen += i;
    }

    if (fmtver < 8)
    {
        // Deprecated data
        gblen += 32;
    }

    if (fmtver >= 12)
    {
        memcpy(&next_mission, &save_game_buffer[gblen], sizeof(next_mission));
        gblen += 2;
    } else
    {
        next_mission = save_game_buffer[gblen];
        gblen += 2;
    }


    {
        int i;
        for (i = 1; i < next_mission; i++)
        {
            struct Mission *p_missi;
            p_missi = &mission_list[i];
            memcpy(p_missi->SpecialTrigger, &save_game_buffer[gblen], sizeof(p_missi->SpecialTrigger));
            gblen += sizeof(p_missi->SpecialTrigger);
            if (fmtver > 1)
            {
                p_missi->Complete = save_game_buffer[gblen];
                gblen++;
            }
        }
    }

    if (fmtver >= 6)
    {
        int i;
        i = 16;
        memcpy(login_name, &save_game_buffer[gblen], i);
        gblen += i;
    }
    else
    {
        if (login_name[0] == '\0')
          strcpy(login_name, "ANON");
    }

    if (fmtver >= 11)
    {
        int i;
        for (i = 0; i < num_cities; i++)
        {
            cities[i].Info = save_game_buffer[gblen];
            gblen++;
            recount_city_credit_reward(i);
        }
        ingame.AutoResearch = save_game_buffer[gblen];
        gblen++;
    }

    read_user_settings();
    login_control__Money = ingame.Credits;
    ingame.CashAtStart = ingame.Credits;
    ingame.Expenditure = 0;
    return 0;
}

/******************************************************************************/
