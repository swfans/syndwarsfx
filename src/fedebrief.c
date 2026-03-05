/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fedebrief.c
 *     Front-end desktop and menu system, mission debrief screen.
 * @par Purpose:
 *     Implement functions for mission debrief screen in front-end desktop.
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
#include "fedebrief.h"

#include "bftext.h"
#include "bffont.h"
#include "bfutility.h"
#include <string.h>

#include "campaign.h"
#include "display.h"
#include "febrief.h"
#include "femain.h"
#include "guiboxes.h"
#include "guitext.h"
#include "game_options.h"
#include "game_sprts.h"
#include "game.h"
#include "keyboard.h"
#include "thing.h"
#include "misstat.h"
#include "mydraw.h"
#include "player.h"
#include "purpldrw.h"
#include "research.h"
#include "swlog.h"
/******************************************************************************/
#define SCIENTIST_LOST_REASONS_COUNT 20
#define MISSION_STATS_SECOND_COLUMN_X 300
#define PEOPLE_STATS_SECOND_COLUMN_X 300

struct DebriefReport curr_report;

struct ScreenBox debrief_mission_box = {0};
struct ScreenBox debrief_people_box = {0};

const ushort mod_group_type_strid[] = {74, 71, 72, 70, 73, };

// Shared boxes
extern struct ScreenTextBox world_city_info_box;

extern ushort word_1C4846[8];
extern ushort word_1C4856[8];

void show_debrief_screen(void)
{
#if 0
    asm volatile ("call ASM_show_debrief_screen\n"
        :  :  : "eax" );
#endif
    ubyte drawn;

    if ((game_projector_speed && is_heading_flag01()) ||
      (is_key_pressed(KC_SPACE, KMod_DONTCARE) && !edit_flag))
    {
        clear_key_pressed(KC_SPACE);
        skip_flashy_draw_debrief_screen_boxes();
    }
    drawn = 1;
    if (drawn) {
        drawn = draw_heading_box();
    }
    if (drawn) {
        //drawn = debrief_mission_box.DrawFn(&debrief_mission_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&debrief_mission_box), "g" (debrief_mission_box.DrawFn));
    }
    if (drawn) {
        //drawn = debrief_people_box.DrawFn(&debrief_people_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&debrief_people_box), "g" (debrief_people_box.DrawFn));
    }
    if (drawn) {
        //drawn = world_city_info_box.DrawFn(&world_city_info_box); -- incompatible calling convention
        asm volatile ("call *%2\n"
            : "=r" (drawn) : "a" (&world_city_info_box), "g" (world_city_info_box.DrawFn));
    }
}

void draw_mission_stats_names_column(struct ScreenBox *box,
  ubyte research_ln, ubyte scilost_ln)
{
    short fheight, lnheight, sepheight;
    short x, y;

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;
    sepheight = fheight + 4;

    if (research_ln + scilost_ln >= 4) // compact mode
        sepheight /= 2;

    // Row with names
    x = 20;
    y = sepheight;

    // Reference no
    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_MISSION_REF], 0);
    y += lnheight;

    // Status
    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_MISSION_STATUS], 0);
    y += lnheight;

    // City time
    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_CITY_NAME], 0);
    y += lnheight;

    // Mission time
    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_MISS_TIME_ELP], 0);
    y += lnheight;
    y += sepheight;

    // Income
    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_CREDS_INCOME], 0);
    y += lnheight;

    // Expenditure
    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_CREDS_EXPENDIT], 0);
    y += lnheight;

    if (research_ln > 0)
    {
        draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_RESEARCH_DONE], 0);
        y += lnheight * research_ln;
    }
    if (scilost_ln > 0)
    {
        // Scientists lost
        draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_SCIENTISTS_DEAD], 0);
        y += lnheight * scilost_ln;
    }
}

void draw_mission_stats_vals_static(struct ScreenBox *box,
  struct DebriefReport *p_rep, ubyte research_ln, ubyte scilost_ln)
{
    struct MissionStatus *p_mistat;
    const char *text;
    char locstr[40];
    short fheight, lnheight, sepheight;
    short x, y;

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;
    sepheight = fheight + 4;

    if (research_ln + scilost_ln >= 4) // compact mode
        sepheight /= 2;

    // Row with values
    x = MISSION_STATS_SECOND_COLUMN_X;
    y = sepheight;
    p_mistat = &mission_status[p_rep->BriefNo];

    // Reference no
    snprintf(locstr, sizeof(locstr), "%hd", p_rep->RefNo);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%s", gui_strings[GSTR_ENM_MISSION_STATUS + 1 + p_rep->Status]);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprint_dh_time_duration(locstr, sizeof(locstr),
      p_mistat->CityDays, p_mistat->CityHours);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprint_dh_time_duration(locstr, sizeof(locstr),
      p_mistat->Days, p_mistat->Hours);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;
    y += sepheight;

    // Income
    snprintf(locstr, sizeof(locstr), "%ld C", p_rep->Income);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    // Expenditure
    snprintf(locstr, sizeof(locstr), "%ld C", p_rep->Expenditure);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;
}

void snprint_concat_comma_separated_weapons_list(char *out, ushort outlen, ulong weapons)
{
    WeaponType wtype;
    ushort strid;
    ushort pos;

    wtype = WEP_TYPES_COUNT;
    while (1)
    {
        struct Campaign *p_campgn;

        wtype = weapons_prev_weapon(weapons, wtype);
        if (wtype == WEP_NULL)
            break;

        if (strlen(out) > outlen - 4u)
            break;

        p_campgn = &campaigns[background_type];
        strid = p_campgn->WeaponsTextIdShift + wtype - 1;

        pos = strlen(out);
        if (pos == 0)
            snprintf(out, outlen, "%s", gui_strings[strid]);
        else
            snprintf(out + pos, outlen - pos, ", %s", gui_strings[strid]);
    }
}

void snprint_concat_comma_separated_cybmods_list(char *out, ushort outlen, ulong cybmods)
{
    ushort mtype, mgrouptype;
    ushort gt_strid;
    ushort mv_strid;
    ushort pos;

    mtype = MOD_TYPES_COUNT;
    while (1)
    {
        mtype = cybmodflags_prev_mod(cybmods, mtype);
        if (mtype == 0)
            break;

        if (strlen(out) > outlen - 4u)
            break;

        mgrouptype = cybmod_group_type(mtype);
        gt_strid = mod_group_type_strid[mgrouptype];
        if (mgrouptype == 4)
            mv_strid = 75;
        else
            mv_strid = 76;

        pos = strlen(out);
        if (pos == 0)
            snprintf(out, outlen, "%s %s %d",
              gui_strings[gt_strid], gui_strings[mv_strid], cybmod_version(mtype));
        else
            snprintf(out + pos, outlen - pos, ", %s %s %d",
              gui_strings[gt_strid], gui_strings[mv_strid], cybmod_version(mtype));
    }
}

short new_weapomn_mods_text_width(struct DebriefReport *p_rep)
{
    char locstr[80];

    lbFontPtr = med_font;

    locstr[0] = '\0';
    snprint_concat_comma_separated_weapons_list(locstr, sizeof(locstr), p_rep->WeaponsResearched);
    snprint_concat_comma_separated_cybmods_list(locstr, sizeof(locstr), p_rep->ModsResearched);

    return LbTextStringWidth(locstr);
}

/** Returns width of the scientists died line in debriefing.
 * Requires scientist_lost_reason to be filled before the call.
 */
short scientists_died_text_width(struct DebriefReport *p_rep)
{
    char locstr[80];

    lbFontPtr = med_font;

    snprintf(locstr, sizeof(locstr), "%d: %s", p_rep->ScientistsLost, scientist_lost_reason);

    return LbTextStringWidth(locstr);
}

void draw_mission_stats_vals_dynamic(struct ScreenBox *box,
  struct DebriefReport *p_rep, ubyte research_ln, ubyte scilost_ln)
{
    short fheight, lnheight, sepheight;
    short x, y;
    const char *text;
    char locstr[80];

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;
    sepheight = fheight + 4;

    if (research_ln + scilost_ln >= 4) // compact mode
        sepheight /= 2;

    x = MISSION_STATS_SECOND_COLUMN_X;
    y = sepheight + 4 * lnheight + sepheight + 2 * lnheight;

    if (research_ln > 0)
    {
        locstr[0] = '\0';
        snprint_concat_comma_separated_weapons_list(locstr, sizeof(locstr), p_rep->WeaponsResearched);
        snprint_concat_comma_separated_cybmods_list(locstr, sizeof(locstr), p_rep->ModsResearched);
        text = loctext_to_gtext(locstr);

        // If cannot fit in expected num of lines, use smaller font
        if (LbTextStringWidth(text) > research_ln * (box->Width - 8 - MISSION_STATS_SECOND_COLUMN_X))
            lbFontPtr = small_med_font;
        draw_text_purple_list2(x, y, text, 0);
        lbFontPtr = med_font;
        y += lnheight * research_ln;
    }

    if (scilost_ln > 0)
    {
        snprintf(locstr, sizeof(locstr), "%d: %s", p_rep->ScientistsLost, scientist_lost_reason);
        text = loctext_to_gtext(locstr);

        if (LbTextStringWidth(text) > scilost_ln * (box->Width - 8 - MISSION_STATS_SECOND_COLUMN_X)) {
            // Cannot fit in expected num of lines - divide the text into
            // amount and reason, and print the latter small
            char *text2;
            short dx, dy;

            text2 = strchr(text, ' ');
            text2[0] = '\0';
            text2++;
            draw_text_purple_list2(x, y, text, 0);
            dx = LbTextStringWidth(text) + 4;
            lbFontPtr = small_med_font;
            dy = fheight - my_char_height('A'); // print at same level as previous test
            draw_text_purple_list2(x + dx, y + dy, text2, 0);
            lbFontPtr = med_font;
        } else {
            draw_text_purple_list2(x, y, text, 0);
        }
        y += lnheight * scilost_ln;
    }
}

void debrief_report_fill(struct DebriefReport *p_rep)
{
    p_rep->RefNo = byte_1C4AA3;
    p_rep->BriefNo = open_brief;
    p_rep->Status = ingame.MissionStatus;
    p_rep->Income = (ingame.Credits + ingame.Expenditure) - ingame.CashAtStart;
    p_rep->Expenditure = ingame.Expenditure;
    LOGSYNC("Research completed wep=0x%lx mod=0x%lx", new_weapons_researched, new_mods_researched);
    p_rep->WeaponsResearched = new_weapons_researched;
    p_rep->ModsResearched = new_mods_researched;
    p_rep->ScientistsLost = scientists_lost;
    p_rep->SciLostReason = LbRandomAnyShort() % SCIENTIST_LOST_REASONS_COUNT;
}

ubyte show_mission_stats(struct ScreenBox *box)
{
    struct DebriefReport *p_rep;
    ubyte research_ln, scilost_ln;

    p_rep = &curr_report;
    research_ln = 0;
    scilost_ln = 0;

    // TODO maybe fill the report at higher level?
    if ((box->Flags & GBxFlg_BkgndDrawn) == 0) {
        debrief_report_fill(p_rep);
        ingame.CashAtStart = ingame.Credits;
        ingame.Expenditure = 0;

        if (p_rep->ScientistsLost > 0) {
            load_scientist_lost_reason(p_rep->SciLostReason, back_buffer + text_buf_pos);
        }
    }

    my_set_text_window(box->X + 4, box->Y + 4, box->Width - 8, box->Height - 8);

    if ((p_rep->WeaponsResearched != 0) || (p_rep->ModsResearched != 0)) {
        short textw;
        research_ln += 1;
        textw = new_weapomn_mods_text_width(p_rep);
        if (textw > box->Width - 8 - MISSION_STATS_SECOND_COLUMN_X)
            research_ln += 1;
        if (textw > 2 * (box->Width - 8 - MISSION_STATS_SECOND_COLUMN_X) + 8)
            research_ln += 1;
    }
    if (p_rep->ScientistsLost > 0) {
        short textw;
        scilost_ln += 1;
        // use two lines for death reason only if we have unused space
        if (research_ln <= 2) {
            textw = scientists_died_text_width(p_rep);
            if (textw > box->Width - 8 - MISSION_STATS_SECOND_COLUMN_X)
                scilost_ln += 1;
        }
    }

    if ((box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        draw_mission_stats_names_column(box, research_ln, scilost_ln);

        draw_mission_stats_vals_static(box, p_rep, research_ln, scilost_ln);

        box->Flags |= GBxFlg_BkgndDrawn;
        copy_box_purple_list(box->X, box->Y, box->Width, box->Height);
    }

    draw_mission_stats_vals_dynamic(box, p_rep, research_ln, scilost_ln);
    return 0;
}

void draw_mission_people_stats_names_column(struct ScreenBox *box,
  struct DebriefReport *p_rep)
{
    int fheight, lnheight;
    short x, y;

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;

    // Row with names
    x = 20;
    y = lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_PERSD_CIVIL], 0);
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_PERSD_MILIT], 0);
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_PERSD_ADVERS], 0);
    y += lnheight;
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_KILLD_CIVIL], 0);
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_KILLD_MILIT], 0);
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_KILLD_ADVERS], 0);
    y += lnheight;
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_CYBORGS_LOST], 0);
    y += lnheight;

    draw_text_purple_list2(x, y, gui_strings[GSTR_MTR_CYBORGS_GAIN], 0);
    y += lnheight;
}

void draw_mission_people_stats_vals_column(struct ScreenBox *box,
  struct DebriefReport *p_rep)
{
    struct MissionStatus *p_mistat;
    const char *text;
    char locstr[40];
    int fheight, lnheight;
    short x, y;

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;

    // Row with values
    x = PEOPLE_STATS_SECOND_COLUMN_X;
    y = lnheight;
    p_mistat = &mission_status[p_rep->BriefNo];

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->SP.CivsPersuaded);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->SP.SecurityPersuaded);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->SP.EnemiesPersuaded);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->SP.CivsKilled);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->SP.SecurityKilled);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->SP.EnemiesKilled);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->AgentsLost);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;

    snprintf(locstr, sizeof(locstr), "%d", p_mistat->AgentsGained);
    text = loctext_to_gtext(locstr);
    draw_text_purple_list2(x, y, text, 0);
    y += lnheight;
}

void draw_mission_mp_players_names_column(struct ScreenBox *box,
  struct DebriefReport *p_rep)
{
    const char *text;
    char locstr[40];
    int fheight, lnheight;
    PlayerIdx plyr;
    int k;
    int used_num; // Player number counting existing player; starts at 1
    short x1, x2, x3;
    short y;

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;

    y = 2 * lnheight;
    used_num = 1;
    for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++)
    {
        k = byte_1C5C28[plyr];
        word_1C4856[k] = 0;
    }

    x1 = 20;
    x3 = 25;
    for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++)
    {
        int textw;

        if (unkn2_names[plyr][0] == '\0')
            continue;

        word_1C4846[plyr] = stats_mp_count_net_players_agents_kills(plyr);

        k = byte_1C5C28[plyr];
        if (k != 0)
            word_1C4856[k] += word_1C4846[plyr];

        snprintf(locstr, sizeof(locstr), "%d", used_num);
        text = loctext_to_gtext(locstr);
        textw = my_string_width(text);
        draw_text_purple_list2(x1 - textw, y, text, 0);
        x2 = 140 + 40 * (used_num - 1);
        draw_text_purple_list2(x2 - textw, lnheight, text, 0);
        text = unkn2_names[plyr];
        draw_text_purple_list2(x3, y, text, 0);
        y += lnheight;

        used_num++;
    }
}

void draw_mission_mp_players_vals_column(struct ScreenBox *box,
  struct DebriefReport *p_rep)
{
    const char *text;
    char locstr[40];
    int fheight, lnheight;
    short x, y;
    int i, k;

    lbDisplay.DrawFlags = 0;
    lbFontPtr = med_font;
    fheight = my_char_height('A');
    lnheight = fheight + 4;

    y = 2 * lnheight;
    lbDisplay.DrawColour = 87;

    for (i = 0; i < PLAYERS_LIMIT; i++)
    {
        struct MissionStatus *p_mistat;
        int textw;
        ushort plyr;

        if (unkn2_names[i][0] == '\0')
            continue;

        x = 140;

        plyr = (players[i].MyAgent[0]->U.UPerson.ComCur & 0x1C) >> 2;
        p_mistat = &mission_status[plyr];
        for (k = 0; k < PLAYERS_LIMIT; k++)
        {
            if (unkn2_names[k][0] == '\0')
                continue;

            if (k == i)
                lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
            snprintf(locstr, sizeof(locstr), "%d", (int)p_mistat->MP.AgentsKilled[k]);
            text = loctext_to_gtext(locstr);
            textw = my_string_width(text);
            draw_text_purple_list2(x - textw, y, text, 0);
            if (k == i)
                lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
            x += 40;
        }
        lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;

        snprintf(locstr, sizeof(locstr), "%d", word_1C4846[i]);
        text = loctext_to_gtext(locstr);
        textw = my_string_width(text);
        draw_text_purple_list2(x - textw, y, text, 0);

        k = byte_1C5C28[i];
        if (k != 0)
        {
            x += 1;
            draw_text_purple_list2(x, y, "/", 0);
            x += LbTextCharWidth('/') + 1;

            snprintf(locstr, sizeof(locstr), "%d", word_1C4856[k]);
            text = loctext_to_gtext(locstr);
            draw_text_purple_list2(x, y, text, 0);
        }
        lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
        y += lnheight;
    }

    x = 140;
    lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;

    for (i = 0; i != PLAYERS_LIMIT; i++)
    {
        int textw;
        ushort plyr;
        int n;

        if (unkn2_names[i][0] == '\0')
            continue;

        plyr = (players[k].MyAgent[0]->U.UPerson.ComCur & 0x1C) >> 2;
        n = stats_mp_count_players_agents_killed(plyr);
        snprintf(locstr, sizeof(locstr), "%d", n);
        text = loctext_to_gtext(locstr);
        textw = my_string_width(text);
        draw_text_purple_list2(x - textw, y, text, 0);
        x += 40;
    }
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
}

ubyte show_mission_people_stats(struct ScreenBox *box)
{
    struct DebriefReport *p_rep;

    p_rep = &curr_report;

    my_set_text_window(box->X + 4, box->Y + 4, box->Width - 8, box->Height - 8);

    if ((box->Flags & GBxFlg_BkgndDrawn) == 0)
    {
        if (screentype == SCRT_DEBRIEF)
        {
            draw_mission_people_stats_names_column(box, p_rep);
        }
        else
        {
            draw_mission_mp_players_names_column(box, p_rep);
        }
        box->Flags |= GBxFlg_BkgndDrawn;
        copy_box_purple_list(box->X, box->Y, box->Width, box->Height);
    }

    if (screentype == SCRT_DEBRIEF)
    {
        draw_mission_people_stats_vals_column(box, p_rep);
    }
    else
    {
        draw_mission_mp_players_vals_column(box, p_rep);
    }
    return 0;
}

void init_debrief_screen_boxes(void)
{
    ScrCoord start_x, start_y;

    init_screen_box(&debrief_mission_box, 7u, 72u, 518u, 172, 6);
    debrief_mission_box.SpecialDrawFn = show_mission_stats;

    init_screen_box(&debrief_people_box, 7u, 253u, 518u, 173, 6);
    debrief_people_box.SpecialDrawFn = show_mission_people_stats;

    // Reposition the components to current resolution

    // For this screen, we will just align to other pre-positioned boxes;
    // No need to compute positions from scratch
    start_x = heading_box.X;
    start_y = world_city_info_box.Y;

    // Expect world_city_info_box to be already set
    debrief_mission_box.X = start_x;
    debrief_mission_box.Y = start_y;

    debrief_people_box.X = start_x;
    debrief_people_box.Y = start_y + world_city_info_box.Height - debrief_people_box.Height;
}

void reset_debrief_screen_boxes_flags(void)
{
    debrief_people_box.Flags = GBxFlg_Unkn0001;
    debrief_mission_box.Flags = GBxFlg_Unkn0001;
}

void skip_flashy_draw_debrief_screen_boxes(void)
{
    skip_flashy_draw_heading_screen_boxes();
    debrief_people_box.Flags |= 0x0002;
    debrief_mission_box.Flags |= 0x0002;
    world_city_info_box.Flags |= 0x0002;
}

/******************************************************************************/
