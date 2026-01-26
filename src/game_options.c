/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_options.c
 *     Game options storage and manipulation.
 * @par Purpose:
 *     Variables and functions for handling ingame settings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 02 Nov 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "game_options.h"

#include <assert.h>

#include "display.h"
#include "engindrwlstm.h"
#include "engindrwlstx.h"
#include "femain.h"
#include "game.h"
#include "guitext.h"
#include "sound.h"
#include "swlog.h"

/******************************************************************************/


/******************************************************************************/

const char *game_option_desc(int option_no)
{
    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
        return gui_strings[464];
    case GOpt_HighResolution:
        return gui_strings[469];
    case GOpt_DetailLevel:
        return gui_strings[470];
    case GOpt_CameraPerspective:
        return gui_strings[471];
    case GOpt_AdvancedLights:
        return gui_strings[481];
    case GOpt_BillboardMovies:
        return gui_strings[480];
    case GOpt_DeepRadar:
        return gui_strings[514];
    case GOpt_PanelPermutation:
        return gui_strings[522];
    case GOpt_TrenchcoatPreference:
        return gui_strings[523];
    case GOpt_PaletteBrightness:
        return gui_strings[524];
    case GOpt_SampleVolume:
        return gui_strings[419];
    case GOpt_DangerVolume:
        return gui_strings[420];
    case GOpt_CDAVolume:
        return gui_strings[516];
    case GOpt_CDATrack:
        return gui_strings[528];
    case GOpt_DangerTrack:
        return gui_strings[529];
    case GOpt_UseMultiMedia:
        return gui_strings[530];
    case GOpt_ScannerPulse:
        return gui_strings[823];
    case GOpt_TranspObjSurfaceColr:
        return gui_strings[824];
    case GOpt_TranspObjLineColr:
        return gui_strings[825];
    default:
        return "";
    }
}

short game_option_get(int option_no)
{
    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
        return game_projector_speed;
    case GOpt_HighResolution:
        return game_high_resolution;
    case GOpt_DetailLevel:
        return ingame.DetailLevel;
    case GOpt_CameraPerspective:
        return game_perspective;
    case GOpt_AdvancedLights:
        return ((ingame.Flags & GamF_AdvLights) != 0);
    case GOpt_BillboardMovies:
        return ((ingame.Flags & GamF_BillboardMovies) != 0);
    case GOpt_DeepRadar:
        return ((ingame.Flags & GamF_DeepRadar) != 0);
    case GOpt_UseMultiMedia:
        return (ingame.UseMultiMedia);
    case GOpt_ScannerPulse:
        return ((ingame.Flags & GamF_NoScannerBeep) != 0);
    case GOpt_PanelPermutation:
        return ingame.PanelPermutation;
    case GOpt_TrenchcoatPreference:
        return ingame.TrenchcoatPreference;
    case GOpt_PaletteBrightness:
        return user_sel_brightness;
    case GOpt_SampleVolume:
        return startscr_samplevol;
    case GOpt_DangerVolume:
        return startscr_midivol;
    case GOpt_CDAVolume:
        return startscr_cdvolume;
    case GOpt_CDATrack:
        return ingame.CDTrack;
    case GOpt_DangerTrack:
        return ingame.DangerTrack;
    case GOpt_TranspObjSurfaceColr:
        return deep_radar_surface_col;
    case GOpt_TranspObjLineColr:
        return deep_radar_line_col;
    default:
        LOGERR("Option %d get not supported", option_no);
        break;
    }
    return 0;
}

void game_option_set(int option_no, int sval)
{
    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
        game_projector_speed = sval;
        break;
    case GOpt_HighResolution:
        game_high_resolution = sval;
        break;
    case GOpt_DetailLevel:
        ingame.DetailLevel = sval;
        break;
    case GOpt_CameraPerspective:
        game_perspective = sval;
        break;
    case GOpt_AdvancedLights:
        if (sval)
            ingame.Flags |= GamF_AdvLights;
        else
            ingame.Flags &= ~GamF_AdvLights;
        break;
    case GOpt_BillboardMovies:
        if (sval)
            ingame.Flags |= GamF_BillboardMovies;
        else
            ingame.Flags &= ~GamF_BillboardMovies;
        break;
    case GOpt_DeepRadar:
        if (sval)
            ingame.Flags |= GamF_DeepRadar;
        else
            ingame.Flags &= ~GamF_DeepRadar;
        break;
    case GOpt_UseMultiMedia:
        ingame.UseMultiMedia = sval;
        break;
    case GOpt_ScannerPulse:
        if (sval)
            ingame.Flags |= GamF_NoScannerBeep;
        else
            ingame.Flags &= ~GamF_NoScannerBeep;
        break;
    case GOpt_PanelPermutation:
        ingame.PanelPermutation = sval;
        break;
    case GOpt_TrenchcoatPreference:
        ingame.TrenchcoatPreference = sval;
        break;
    case GOpt_PaletteBrightness:
        user_sel_brightness = sval;
        break;
    case GOpt_SampleVolume:
        startscr_samplevol = sval;
        break;
    case GOpt_DangerVolume:
        startscr_midivol = sval;
        break;
    case GOpt_CDAVolume:
        startscr_cdvolume = sval;
        break;
    case GOpt_CDATrack:
        ingame.CDTrack = sval;
        break;
    case GOpt_DangerTrack:
        ingame.DangerTrack = sval;
        break;
    case GOpt_TranspObjSurfaceColr:
        deep_radar_surface_col = sval;
        break;
    case GOpt_TranspObjLineColr:
        deep_radar_line_col = sval;
        break;
    default:
        LOGERR("Option %d set not supported", option_no);
        break;
    }
}

int game_option_min(int option_no)
{
    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
    case GOpt_HighResolution:
    case GOpt_DetailLevel:
        return 0;
    case GOpt_CameraPerspective:
        return 0;
    case GOpt_AdvancedLights:
    case GOpt_BillboardMovies:
    case GOpt_DeepRadar:
    case GOpt_UseMultiMedia:
    case GOpt_ScannerPulse:
        return 0;
    case GOpt_PanelPermutation:
        if (ingame.PanelPermutation < 0)
            return OPT_PANEL_PERMUT_MIN;
        else
            return OPT_PANEL_PERMUT_ALPHA_MIN;
    case GOpt_TrenchcoatPreference:
        return OPT_TRENCHCOAT_PREF_MIN;
    case GOpt_PaletteBrightness:
        return OPT_PAL_BRIGHTNESS_MIN;
    case GOpt_SampleVolume:
    case GOpt_DangerVolume:
    case GOpt_CDAVolume:
        return 0;
    case GOpt_CDATrack:
        return OPT_CD_TRACK_MIN;
    case GOpt_DangerTrack:
        return OPT_DANGER_TRACK_MIN;
    case GOpt_TranspObjSurfaceColr:
    case GOpt_TranspObjLineColr:
        return 0;
    default:
        break;
    }
    return 0;
}

int game_option_max(int option_no)
{
    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
    case GOpt_HighResolution:
    case GOpt_DetailLevel:
        return 1;
    case GOpt_CameraPerspective:
        return 5;
    case GOpt_AdvancedLights:
    case GOpt_BillboardMovies:
    case GOpt_DeepRadar:
    case GOpt_UseMultiMedia:
    case GOpt_ScannerPulse:
        return 1;
    case GOpt_PanelPermutation:
        if (ingame.PanelPermutation < 0)
            return OPT_PANEL_PERMUT_MAX;
        else
            return OPT_PANEL_PERMUT_ALPHA_MAX;
    case GOpt_TrenchcoatPreference:
        return OPT_TRENCHCOAT_PREF_MAX;
    case GOpt_PaletteBrightness:
        return OPT_PAL_BRIGHTNESS_MAX;
    case GOpt_SampleVolume:
    case GOpt_DangerVolume:
    case GOpt_CDAVolume:
        return STARTSCR_VOLUME_MAX;
    case GOpt_CDATrack:
        return OPT_CD_TRACK_MAX;
    case GOpt_DangerTrack:
        return OPT_DANGER_TRACK_MAX;
    case GOpt_TranspObjSurfaceColr:
    case GOpt_TranspObjLineColr:
        return 255;
    default:
        break;
    }
    return 0;
}

void game_option_toggle(int option_no)
{
    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
        if (game_projector_speed == 1)
            game_projector_speed = 0;
        else
            game_projector_speed = 1;
        break;
    case GOpt_HighResolution:
        if (game_high_resolution == 1)
            game_high_resolution = 0;
        else
            game_high_resolution = 1;
        break;
    case GOpt_DetailLevel:
        if (ingame.DetailLevel == 1)
            ingame.DetailLevel = 0;
        else
            ingame.DetailLevel = 1;
        break;
    case GOpt_CameraPerspective:
        if (game_perspective == 5)
            game_perspective = 0;
        else
            game_perspective = 5;
        break;
    case GOpt_AdvancedLights:
        if ((ingame.Flags & GamF_AdvLights) == 0)
            ingame.Flags |= GamF_AdvLights;
        else
            ingame.Flags &= ~GamF_AdvLights;
        break;
    case GOpt_BillboardMovies:
        if ((ingame.Flags & GamF_BillboardMovies) == 0)
            ingame.Flags |= GamF_BillboardMovies;
        else
            ingame.Flags &= ~GamF_BillboardMovies;
        break;
    case GOpt_DeepRadar:
        if ((ingame.Flags & GamF_DeepRadar) == 0)
            ingame.Flags |= GamF_DeepRadar;
        else
            ingame.Flags &= ~GamF_DeepRadar;
        break;
    case GOpt_UseMultiMedia:
        if (ingame.UseMultiMedia == 1)
            ingame.UseMultiMedia = 0;
        else
            ingame.UseMultiMedia = 1;
        break;
    case GOpt_ScannerPulse:
        if ((ingame.Flags & GamF_NoScannerBeep) == 0)
            ingame.Flags |= GamF_NoScannerBeep;
        else
            ingame.Flags &= ~GamF_NoScannerBeep;
        break;
    default:
        LOGERR("Option %d toggle not supported", option_no);
        break;
    }
}

void game_option_dec(int option_no)
{
    int sval;

    switch (option_no)
    {
    // Toggle options (two values only)
    case GOpt_ProjectorSpeed:
    case GOpt_HighResolution:
    case GOpt_DetailLevel:
    case GOpt_CameraPerspective:
    case GOpt_AdvancedLights:
    case GOpt_BillboardMovies:
    case GOpt_DeepRadar:
    case GOpt_UseMultiMedia:
    case GOpt_ScannerPulse:
        game_option_toggle(option_no);
        break;
    // Linear options (with any value between some min and max)
    case GOpt_PanelPermutation:
    case GOpt_TrenchcoatPreference:
    case GOpt_PaletteBrightness:
    case GOpt_SampleVolume:
    case GOpt_DangerVolume:
    case GOpt_CDAVolume:
    case GOpt_CDATrack:
    case GOpt_DangerTrack:
    case GOpt_TranspObjSurfaceColr:
    case GOpt_TranspObjLineColr:
        sval = game_option_get(option_no);
        sval--;
        if (sval < game_option_min(option_no))
            sval = game_option_max(option_no);
        game_option_set(option_no, sval);
        break;
    default:
        break;
    }
}

void game_option_inc(int option_no)
{
    int sval;

    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
    case GOpt_HighResolution:
    case GOpt_DetailLevel:
    case GOpt_CameraPerspective:
    case GOpt_AdvancedLights:
    case GOpt_BillboardMovies:
    case GOpt_DeepRadar:
    case GOpt_UseMultiMedia:
    case GOpt_ScannerPulse:
        game_option_toggle(option_no);
        break;
    case GOpt_PanelPermutation:
    case GOpt_TrenchcoatPreference:
    case GOpt_PaletteBrightness:
    case GOpt_SampleVolume:
    case GOpt_DangerVolume:
    case GOpt_CDAVolume:
    case GOpt_CDATrack:
    case GOpt_DangerTrack:
    case GOpt_TranspObjSurfaceColr:
    case GOpt_TranspObjLineColr:
        sval = game_option_get(option_no);
        sval++;
        if (sval > game_option_max(option_no))
            sval = game_option_min(option_no);
        game_option_set(option_no, sval);
        break;
    default:
        break;
    }
}

void game_option_shift(int option_no, int amount)
{
    int limit, sval;

    switch (option_no)
    {
    case GOpt_ProjectorSpeed:
    case GOpt_HighResolution:
    case GOpt_DetailLevel:
    case GOpt_CameraPerspective:
    case GOpt_AdvancedLights:
    case GOpt_BillboardMovies:
    case GOpt_DeepRadar:
    case GOpt_UseMultiMedia:
    case GOpt_ScannerPulse:
        sval = game_option_get(option_no);
        if (((sval != 0) && (amount < 0)) || ((sval == 0) && (amount > 0)))
            game_option_toggle(option_no);
        break;
    case GOpt_PanelPermutation:
    case GOpt_TrenchcoatPreference:
    case GOpt_PaletteBrightness:
    case GOpt_SampleVolume:
    case GOpt_DangerVolume:
    case GOpt_CDAVolume:
    case GOpt_CDATrack:
    case GOpt_DangerTrack:
    case GOpt_TranspObjSurfaceColr:
    case GOpt_TranspObjLineColr:
        sval = game_option_get(option_no);
        sval += amount;
        limit = game_option_max(option_no);
        if (sval > limit)
            sval = limit;
        limit = game_option_min(option_no);
        if (sval < limit)
            sval = limit;
        game_option_set(option_no, sval);
        break;
    default:
        break;
    }
}

void set_default_gfx_settings(void)
{
    game_gfx_advanced_lights = 1;
    game_billboard_movies = 1;
    game_gfx_deep_radar = 0;
    game_high_resolution = true;
    game_projector_speed = 0;
    game_perspective = 5;
    deep_radar_surface_col = 216;
    deep_radar_line_col = 100;
}

void set_default_visual_prefernces(void)
{
    ingame.PanelPermutation = -2;
    ingame.TrenchcoatPreference = OPT_TRENCHCOAT_PREF_MIN;
    ingame.DetailLevel = 1;
    ingame.UseMultiMedia = 0;
}

void set_default_audio_tracks(void)
{
    ingame.DangerTrack = OPT_DANGER_TRACK_MIN;
    ingame.CDTrack = OPT_CD_TRACK_MIN;
}

void apply_user_gfx_settings(void)
{
    game_option_set(GOpt_AdvancedLights, game_gfx_advanced_lights);
    game_option_set(GOpt_BillboardMovies, game_billboard_movies);
    game_option_set(GOpt_DeepRadar, game_gfx_deep_radar);
    bang_set_detail(ingame.DetailLevel == 0);
}

void apply_user_sfx_settings(void)
{
    sfx_apply_samplevol();
    sfx_apply_midivol();
    sfx_apply_cdvolume();
}
/******************************************************************************/
