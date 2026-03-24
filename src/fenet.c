/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file fenet.c
 *     Front-end desktop and menu system, multiplayer screen.
 * @par Purpose:
 *     Implement functions for multiplayer screen in front-end desktop.
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
#include "fenet.h"

#include <assert.h>

#include "bffont.h"
#include "bfkeybd.h"
#include "bfmemut.h"
#include "bfscrcopy.h"
#include "bfsprite.h"
#include "bfstrut.h"
#include "bftext.h"
#include "bfutility.h"
#include "ssampply.h"

#include "campaign.h"
#include "dos.h"
#include "guiboxes.h"
#include "guitext.h"
#include "display.h"
#include "femain.h"
#include "feshared.h"
#include "game_options.h"
#include "game_save.h"
#include "game_speed.h"
#include "game_sprts.h"
#include "game.h"
#include "keyboard.h"
#include "mydraw.h"
#include "network.h"
#include "packetfe.h"
#include "purpldrw.h"
#include "purpldrwlst.h"
#include "player.h"
#include "sound.h"
#include "swlog.h"
#include "util.h"
/******************************************************************************/

struct ScreenButton net_INITIATE_button = {0};
struct ScreenButton net_groups_LOGON_button = {0};
struct ScreenButton unkn8_EJECT_button = {0};
struct ScreenButton net_protocol_select_button = {0};
struct ScreenButton net_unkn40_button = {0};
struct ScreenButton net_SET2_button = {0};
struct ScreenButton net_SET_button = {0};
struct ScreenBox net_groups_box = {0};
struct ScreenBox net_users_box = {0};
struct ScreenBox net_faction_box = {0};
struct ScreenBox net_team_box = {0};
struct ScreenBox net_benefits_box = {0};
struct ScreenBox net_comms_box = {0};
struct ScreenBox net_grpaint = {0};
struct ScreenBox net_protocol_box = {0};
struct ScreenButton net_protocol_option_button = {0};

extern char net_unkn40_text[];
extern char net_baudrate_text[8];
extern char net_proto_param_text[8];
extern ubyte byte_155174; // = 166;
extern ubyte byte_155175[];
extern ubyte byte_155180; // = 109;
extern ubyte byte_155181[];
extern int unkn_rate; // = 19200;
extern int serial_speeds[8];
extern char net_baudrate_text[8];
extern ubyte byte_1C47EA;
extern ubyte byte_1C4805;
extern ubyte byte_1C4806;
extern ubyte byte_1C4994;
extern ubyte net_autostart_done;
extern ubyte byte_155170[4];
extern char net_unkn1_text[25];
extern char byte_1811E2[16];
extern uint32_t sessionlist_last_update[20];
extern ubyte byte_1C6D48;
extern struct TbNetworkSessionList unkstruct04_arr[20];

ubyte ac_do_net_protocol_option(ubyte click);
ubyte ac_do_net_unkn40(ubyte click);
ubyte ac_do_serial_speed_switch(ubyte click);
ubyte ac_do_net_SET2(ubyte click);
ubyte ac_do_net_SET(ubyte click);
ubyte ac_do_net_INITIATE(ubyte click);
ubyte ac_do_net_groups_LOGON(ubyte click);
ubyte ac_do_unkn8_EJECT(ubyte click);
ubyte ac_show_net_benefits_box(struct ScreenBox *box);
ubyte ac_show_net_grpaint(struct ScreenBox *box);
ubyte ac_show_net_comms_box(struct ScreenBox *box);
ubyte ac_do_net_protocol_select(ubyte click);
ubyte ac_show_net_protocol_box(struct ScreenBox *box);

void ac_purple_unkn1_data_to_screen(void);
void ac_purple_unkn3_data_to_screen(void);

void net_sessionlist_clear(void)
{
    ushort i;

    assert(sizeof(struct TbNetworkSession) == 40);
    assert(sizeof(struct TbNetworkPlayer) == 22);
    assert(sizeof(struct TbNetworkSessionList) == 40+22*8+2);

    for (i = 0; i < sizeof(unkstruct04_arr)/sizeof(unkstruct04_arr[0]); i++)
        LbMemorySet(&unkstruct04_arr[i], 0, sizeof(struct TbNetworkSessionList));

    byte_1C6D48 = 0;
}

void net_sessionlist_remove(int sess_no)
{
    int nxt_sess_no;
    struct TbNetworkSessionList *p_cur_nslist;
    struct TbNetworkSessionList *p_nxt_nslist;

    p_cur_nslist = &unkstruct04_arr[sess_no];
    p_nxt_nslist = &unkstruct04_arr[sess_no + 1];
    for (nxt_sess_no = sess_no + 1; nxt_sess_no < byte_1C6D48; nxt_sess_no++)
    {
        LbMemoryCopy(p_cur_nslist, p_nxt_nslist, sizeof(struct TbNetworkSessionList));
        sessionlist_last_update[nxt_sess_no - 1] = sessionlist_last_update[nxt_sess_no];
        ++p_cur_nslist;
        ++p_nxt_nslist;
    }
    nxt_sess_no = --byte_1C6D48;
    LbMemorySet(&unkstruct04_arr[nxt_sess_no], 0, sizeof(struct TbNetworkSessionList));
    sessionlist_last_update[nxt_sess_no] = 0;
}

void net_sessionlist_update_latest_one(void)
{
    struct TbNetworkSessionList locsesslst;
    struct TbNetworkSessionList *p_nslist;
    TbBool sess_found;
    int sess_no;

    if (LbNetworkSessionList(&locsesslst, 1) != 1) {
        LOGSYNC("No session to update");
        return;
    }

    sess_found = false;

    for (sess_no = 0; sess_no < byte_1C6D48; sess_no++)
    {
        p_nslist = &unkstruct04_arr[sess_no];
        if (strcmp(p_nslist->Session.Name, locsesslst.Session.Name) == 0)
        {
            sess_found = 1;
            break;
        }
    }
    if (!sess_found) {
        sess_no = byte_1C6D48;
        byte_1C6D48++;
    }
    LOGSYNC("Updating session %d", sess_no);
    p_nslist = &unkstruct04_arr[sess_no];
    LbMemoryCopy(p_nslist, &locsesslst, sizeof(struct TbNetworkSessionList));
    sessionlist_last_update[sess_no] = dos_clock();
}

void net_unkn2_names_clear(void)
{
    ushort plyr;

    for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++) {
        unkn2_names[plyr][0] = '\0';
    }
}

void net_service_gui_switch(void)
{
    const char *text;

    switch (nsvc.I.Type)
    {
    case NetSvc_IPX:
        net_protocol_option_button.Text = net_proto_param_text;
        net_protocol_option_button.CallBackFn = ac_do_net_protocol_option;
        text = gui_strings[GSTR_NET_PROTO_NAMES + nsvc.I.Type];
        net_protocol_select_button.Text = text;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        net_protocol_option_button.Text = net_baudrate_text;
        net_protocol_option_button.CallBackFn = ac_do_serial_speed_switch;
        if (byte_1C4A6F)
            text = gui_strings[GSTR_NET_PROTO_MODEM_NAMES - NetSvc_COM1 + nsvc.I.Type];
        else
            text = gui_strings[GSTR_NET_PROTO_NAMES + nsvc.I.Type];
        net_protocol_select_button.Text = text;
        break;
    }
}

void net_service_switch(ushort svctp)
{
    nsvc.I.Type = svctp;
    if (nsvc.I.Type == NetSvc_IPX) {
        net_sessionlist_clear();
        net_unkn2_names_clear();
    }
    net_service_gui_switch();
}

/** Restart the network service after an option change.
 */
TbBool net_service_restart(void)
{
    LOGSYNC("Restart");
    net_sessionlist_clear();
    net_unkn2_names_clear();

    if (LbNetworkServiceStart(&nsvc.I) != Lb_SUCCESS)
    {
        LOGERR("Failed on LbNetworkServiceStart");
        alert_box_text_fmt("%s", gui_strings[568]);
        return false;
    }
    net_service_started = 1;
    return true;
}

/** Stop the network service if it was started.
 */
TbBool net_service_stop(void)
{
    if (net_service_started)
    {
        LbNetworkReset();
        net_service_started = 0;
        return true;
    }
    return false;
}

/** Initialize network sessions, create local session.
 *
 * Requires the network service to be started before.
 */
TbBool net_sessions_init(void)
{
    LOGSYNC("Prep local session");
    LbMemoryCopy(nsvc.S.Name, login_name, min(sizeof(nsvc.S.Name),sizeof(login_name)));
    nsvc.S.MaxPlayers = 8;
    nsvc.S.HostPlayerNumber = 0;
    if (LbNetworkSessionCreate(&nsvc.S, nsvc.S.Name) != Lb_SUCCESS)
    {
        LOGERR("Failed on LbNetworkSessionCreate");
        alert_box_text_fmt("%s", gui_strings[579]);
        return false;
    }
    return true;
}

ubyte do_net_protocol_option(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_protocol_option\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    short param, dt;

    net_service_stop();

    dt = 0x01;
    if ((lbShift & KMod_SHIFT) != 0)
        dt = 0x10;
    if ((lbShift & KMod_CONTROL) != 0)
        dt = 0x100;

    param = nsvc.I.Param;
    if (click)
    {
        param -= dt;
        if (param < 0)
            param = 2746;
    }
    else
    {
        param += dt;
        if (param > 2746)
            param = 0;
    }
    nsvc.I.Param = param;

    sprintf(net_proto_param_text, "%04x", (int)nsvc.I.Param);
    LbStringToUpper(net_proto_param_text);

    if (!net_service_restart()) {
        // Switch to a service which is always available
        net_service_switch(NetSvc_COM1);
    }

    return 1;
}

ubyte do_net_unkn40(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_unkn40\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    byte_1C4994 = (byte_1C4994 == 0);
    return 1;
}

short serial_speeds_idx(int boud_rate)
{
    short i;

    for (i = 0; i < (int)(sizeof(serial_speeds)/sizeof(serial_speeds[0])); i++)
    {
        if (serial_speeds[i] == boud_rate)
            break;
    }
    if (i >= (int)(sizeof(serial_speeds)/sizeof(serial_speeds[0])))
        i = 0;
    return i;
}

ubyte do_serial_speed_switch(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_serial_speed_switch\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    short cur_idx, nxt_idx;

    cur_idx = serial_speeds_idx(unkn_rate);

    if (click)
    {
        nxt_idx = cur_idx - 1;
        if (nxt_idx < 0)
            nxt_idx = (sizeof(serial_speeds)/sizeof(serial_speeds[0])) - 1;
    }
    else
    {
        nxt_idx = cur_idx + 1;
        if (nxt_idx >= (int)(sizeof(serial_speeds)/sizeof(serial_speeds[0])))
            nxt_idx = 0;
    }
    unkn_rate = serial_speeds[nxt_idx];
    sprintf(net_baudrate_text, "%d", unkn_rate);
    return 1;
}

ubyte do_net_SET2(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_SET2\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (!net_local_player_hosts_the_game() || login_control__State != LognCt_Unkn5)
        return 0;

    net_game_play_flags |= NGPF_Unkn02;
    net_schedule_game_options_sync();
    return 1;
}

ubyte do_net_SET(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_SET\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (!net_local_player_hosts_the_game() || login_control__State != LognCt_Unkn5)
        return 0;

    net_game_play_flags |= NGPF_Unkn01;
    net_schedule_game_options_sync();
    return 1;
}

ubyte net_unkn_func_32(void)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_net_unkn_func_32\n"
        : "=r" (ret) : );
    return ret;
#else
    int ret;
    TbBool modem_on_line;

    modem_on_line = 0;
    if (LbNetworkSessionActive())
        return 0;

    if (nsvc.I.Type == NetSvc_IPX)
        goto skip_modem_init;

    if (!net_service_restart())
        goto out_fail;

    LbNetworkSetBaud(unkn_rate);
    players[local_player_no].DoubleMode = 0;

    if (!byte_1C4A6F)
        goto skip_modem_init;

    if (LbNetworkInit() != Lb_SUCCESS) {
        LOGERR("Failed on LbNetworkInit");
        alert_box_text_fmt("%s", gui_strings[579]);
        goto out_fail;
    }

    ret = LbNetworkDial(net_unkn2_text);
    if (ret == -7 || ret == -1) {
        LOGERR("Failed on LbNetworkAnswer, ret=%d", ret);
        alert_box_text_fmt("%s", gui_strings[579]);
        modem_on_line = true;
        goto out_fail;
    }
    if (ret == 7) {
        LOGERR("Failed on LbNetworkAnswer, ret=%d", ret);
        alert_box_text_fmt("%s", gui_strings[579]);
        goto out_fail;
    }
    if (ret == 4 || ret == 5 || ret == 6) {
        LOGERR("Failed on LbNetworkAnswer, ret=%d", ret);
        alert_box_text_fmt("%s", gui_strings[579]);
        modem_on_line = true;
        goto out_fail;
    }
    if (ret == 1) {
        modem_on_line = true;
    }

skip_modem_init:
    if (!net_sessions_init())
        goto out_fail;

    login_control__State = LognCt_Unkn5;
    net_host_player_no = LbNetworkHostPlayerNumber();
    net_players_num = LbNetworkSessionNumberPlayers();
    byte_15516C = -1;
    byte_15516D = -1;

    if (nsvc.I.Type != NetSvc_IPX) {
        players[local_player_no].DoubleMode = 0;
    }
    load_missions(99);
    net_players_all_set_no_action();

    return 1;

out_fail:
    if (modem_on_line)
        LbNetworkHangUp();
    if (nsvc.I.Type == NetSvc_IPX)
    {
        if (!net_service_started) {
            // Switch to a service which is always available
            net_service_switch(NetSvc_COM1);
        }
    }
    else
    {
        net_service_stop();
    }
    return 0;
#endif
}

ubyte net_unkn_func_31(struct TbNetworkSession *p_nsession)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_net_unkn_func_31\n"
        : "=r" (ret) : "a" (p_nsession));
    return ret;
#else
    TbBool modem_on_line;
    int ret;

    modem_on_line = false;
    if (nsvc.I.Type == NetSvc_IPX)
      goto skip_modem_init;

    if (!net_service_restart())
        goto out_fail;

    LbNetworkSetBaud(unkn_rate);
    players[local_player_no].DoubleMode = 0;
    if (!byte_1C4A6F)
        goto skip_modem_init;

    if (LbNetworkInit() != Lb_SUCCESS) {
        LOGERR("Failed on LbNetworkInit");
        alert_box_text_fmt("%s", gui_strings[579]);
        goto out_fail;
    }

    ret = LbNetworkAnswer();
    if (ret == -7 || ret == -1) {
        LOGERR("Failed on LbNetworkAnswer, ret=%d", ret);
        alert_box_text_fmt("%s", gui_strings[579]);
        modem_on_line = true;
        goto out_fail;
    }
    if (ret == 7) {
        LOGERR("Failed on LbNetworkAnswer, ret=%d", ret);
        alert_box_text_fmt("%s", gui_strings[579]);
        goto out_fail;
    }
    if (ret == 4 || ret == 5 || ret == 6) {
        LOGERR("Failed on LbNetworkAnswer, ret=%d", ret);
        alert_box_text_fmt("%s", gui_strings[579]);
        modem_on_line = true;
        goto out_fail;
    }
    if (ret == 1) {
      modem_on_line = true;
    }

skip_modem_init:
    p_nsession->MaxPlayers = 8;
    if (LbNetworkSessionJoin(p_nsession, login_name) != Lb_SUCCESS)
    {
        LOGERR("Failed on LbNetworkSessionJoin");
        alert_box_text_fmt("%s", gui_strings[579]);
        goto out_fail;
    }
    login_control__State = LognCt_Unkn5;
    net_host_player_no = LbNetworkHostPlayerNumber();
    net_players_num = LbNetworkSessionNumberPlayers();
    byte_1C6D4A = 1;
    LbMemoryCopy(&nsvc.S, p_nsession, sizeof(struct TbNetworkSession));

    if (nsvc.I.Type != NetSvc_IPX) {
        players[local_player_no].DoubleMode = 0;
    }
    load_missions(99);
    net_players_all_set_no_action();

    return 1;

out_fail:
    if (modem_on_line)
        LbNetworkHangUp();
    if (nsvc.I.Type == NetSvc_IPX)
    {
        if (!net_service_started) {
            // Switch to a service which is always available
            net_service_switch(NetSvc_COM1);
        }
    }
    else
    {
        net_service_stop();
    }
    return 0;
#endif
}

void netgame_state_enter_5(void)
{
    PlayerIdx plyr;

    switch_net_screen_boxes_to_execute();
    init_variables();
    init_agents();
    srm_reset_research();
    login_control__State = LognCt_Unkn5;
    for (plyr = 0; plyr < 8; plyr++) {
        player_mission_agents_reset(plyr);
    }
}


ubyte do_net_INITIATE(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_INITIATE\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if (nsvc.I.Type == NetSvc_IPX && !net_service_started) {
        LOGWARN("Cannot init protocol %d - not ready", (int)nsvc.I.Type);
        return 0;
    }
    if (login_control__State == LognCt_Unkn6)
    {
        if (net_unkn_func_32())
        {
            netgame_state_enter_5();
        }
    }
    else if (login_control__State == LognCt_Unkn5)
    {
        int plyr;
        plyr = LbNetworkPlayerNumber();
        if (plyr == net_host_player_no)
        {
            if (login_control__City == -1) {
                LOGWARN("Cannot init protocol %d player %d - city not selected", (int)nsvc.I.Type, plyr);
                return 0;
            }
            net_schedule_local_player_reset();
            byte_15516D = -1;
            byte_15516C = -1;
        }
    }
    return 1;
}

ubyte do_net_groups_LOGON(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_groups_LOGON\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    if ((nsvc.I.Type == NetSvc_IPX) && !net_service_started) {
        LOGWARN("Cannot abort protocol %d - not ready", (int)nsvc.I.Type);
        return 0;
    }

    if (login_control__State == LognCt_Unkn5)
    {
        net_schedule_local_player_logout();
        byte_15516D = -1;
        byte_15516C = -1;
        switch_net_screen_boxes_to_initiate();
        net_unkn_func_33();
    }
    else if (login_control__State == LognCt_Unkn6)
    {
        struct TbNetworkSession *p_nsession;

        p_nsession = NULL;
        if (nsvc.I.Type == NetSvc_IPX)
        {
            if (byte_15516C != -1) {
                p_nsession = &unkstruct04_arr[byte_15516C].Session;
            }
        }
        else
        {
            p_nsession = &unkstruct04_arr[0].Session;
        }
        if (p_nsession != NULL)
        {
            if (net_unkn_func_31(p_nsession))
            {
                netgame_state_enter_5();
            }
        }
    }
    return 1;
}

TbBool mouse_down_over_unkn1(short x, short y, short width, short height)
{
    short ms_x, ms_y;

    ms_y = (lbDisplay.GraphicsScreenHeight < 400) ? 2 * lbDisplay.MouseY : lbDisplay.MouseY;
    ms_x = (lbDisplay.GraphicsScreenHeight < 400) ? 2 * lbDisplay.MouseX : lbDisplay.MouseX;
    if ((ms_y >= y) && (ms_y <= y + height))
    {
        short sum_min, sum_max;

        sum_min = x + y + height;
        sum_max = x + y + width;
        return (ms_x + ms_y >= sum_min) && (ms_x + ms_y <= sum_max);
    }
    return false;
}

void show_net_benefits_sub1(short x0, short y0, TbPixel colour)
{
    ushort spridx;
    short dx, dy;
    short i;

    dx = x0;
    dy = y0;
    spridx = 114;
    for (i = 0; i < 8; i++)
    {
        struct TbSprite *p_sprite;
        short delta;

        p_sprite = &fe_icons_sprites[spridx];
        lbDisplay.DrawColour = colour;
        draw_sprite_purple_list(dx, dy, p_sprite);
        if (i < 2)
            delta = p_sprite->SWidth - 11;
        else if (i < 5)
            delta = p_sprite->SWidth - 25;
        else
            delta = p_sprite->SWidth - 5;
        dx += delta;
        if (i == 0)
            delta = 2;
        else if (i == 1)
            delta = -14;
        else
            delta = 0;
        dy += delta;
        spridx++;
    }
}

void show_net_benefits_sub2(short x0, short y0, TbPixel *colours)
{
    ushort spridx;
    short dx, dy;
    short i;

    dx = x0;
    dy = y0;
    spridx = 114;
    for (i = 0; i < 8; i++)
    {
        struct TbSprite *p_sprite;
        short delta;

        p_sprite = &fe_icons_sprites[spridx];
        if (i < login_control__TechLevel)
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = colours[i];
            draw_sprite_purple_list(dx, dy, p_sprite);
            lbDisplay.DrawFlags = 0;
        }
        if (mouse_down_over_unkn1(dx, dy, p_sprite->SWidth, p_sprite->SHeight))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                if (net_local_player_hosts_the_game() && ((net_game_play_flags & NGPF_Unkn02) == 0)
                  && (login_control__State == LognCt_Unkn5))
                    login_control__TechLevel = i + 1;
            }
        }
        if (i < 2)
            delta = p_sprite->SWidth - 11;
        else if (i < 5)
            delta = p_sprite->SWidth - 25;
        else
            delta = p_sprite->SWidth - 5;
        dx += delta;
        if (i == 0)
            delta = 2;
        else if (i == 1)
            delta = -14;
        else
            delta = 0;
        dy += delta;
        spridx++;
    }
}

void show_net_benefits_sub3(struct ScreenBox *box)
{
    struct ScreenBoxBase box1 = {box->X + 5, box->Y + 16, 9, 14};

    lbDisplay.DrawFlags = (0x8000|Lb_SPRITE_TRANSPAR4);
    if (mouse_move_over_box(&box1))
    {
        if (lbDisplay.LeftButton)
        {
            if (mouse_down_over_box(&box1))
            {
                lbDisplay.LeftButton = 0;
                if (net_local_player_hosts_the_game() && ((net_game_play_flags & NGPF_Unkn02) == 0)
                  && (login_control__State == LognCt_Unkn5))
                {
                    login_control__TechLevel--;
                    if (login_control__TechLevel < 1)
                        login_control__TechLevel = 1;
                }
            }
        }
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    }
    draw_sprite_purple_list(box1.X, box1.Y, &fe_icons_sprites[108]);
}

void show_net_benefits_sub4(struct ScreenBox *box)
{
    struct ScreenBoxBase box2 = {box->X + 242, box->Y + 5, 9, 14};

    lbDisplay.DrawFlags = (0x8000|Lb_SPRITE_TRANSPAR4);
    if (mouse_move_over_box(&box2))
    {
        if (lbDisplay.LeftButton)
        {
            if (mouse_down_over_box(&box2))
            {
                lbDisplay.LeftButton = 0;
                if (net_local_player_hosts_the_game() && ((net_game_play_flags & NGPF_Unkn02) == 0)
                    && (login_control__State == LognCt_Unkn5))
                {
                    login_control__TechLevel++;
                    if (login_control__TechLevel > 8)
                        login_control__TechLevel = 8;
                }
            }
        }
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    }
    draw_sprite_purple_list(box2.X - 7, box2.Y, &fe_icons_sprites[109]);
}

ubyte get_current_starting_cash_level(void)
{
    int i, lv_curr;

    lv_curr = 0;
    for (i = 0; i < 8; i++)
    {
      if (login_control__Money == starting_cash_amounts[i])
          break;
      lv_curr++;
    }
    return lv_curr;
}

uint reinit_starting_credits(sbyte change)
{
#if 0
    ulong ret;
    asm volatile ("call ASM_reinit_starting_credits\n"
        : "=r" (ret) : "a" (change));
    return ret;
#endif
  int lv, lv_curr;
  uint creds;

    lv_curr = get_current_starting_cash_level();
    lv = lv_curr + change;
    if (lv < 0)
        lv = 0;
    if (lv > 7)
        lv = 7;

    creds = starting_cash_amounts[lv];
    login_control__Money = creds;
    ingame.Credits = creds;
    ingame.CashAtStart = creds;
    ingame.Expenditure = 0;

    return creds;
}

void show_net_benefits_sub5(short x0, short y0, TbPixel *colours)
{
    ushort spridx;
    short dx, dy;
    short i;

    dx = x0;
    dy = y0;
    spridx = 114;
    for (i = 0; i < 8; i++)
    {
        struct TbSprite *p_sprite;
        short delta;

        p_sprite = &fe_icons_sprites[spridx];
        if (login_control__Money >= starting_cash_amounts[i])
        {
            lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
            lbDisplay.DrawColour = colours[i];
            draw_sprite_purple_list(dx, dy, p_sprite);
            lbDisplay.DrawFlags = 0;
        }
        if (mouse_down_over_unkn1(dx, dy, p_sprite->SWidth, p_sprite->SHeight))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                if (net_local_player_hosts_the_game() && ((net_game_play_flags & NGPF_Unkn01) == 0)
                  && (login_control__State == LognCt_Unkn5))
                {
                    login_control__Money = starting_cash_amounts[i];
                    ingame.Credits = login_control__Money;
                }
            }
        }
        if (i < 2)
            delta = p_sprite->SWidth - 11;
        else if (i < 5)
            delta = p_sprite->SWidth - 25;
        else
            delta = p_sprite->SWidth - 5;
        dx += delta;
        if (i == 0)
            delta = 2;
        else if (i == 1)
            delta = -14;
        else
            delta = 0;
        dy += delta;
        spridx++;
    }
}

void show_net_benefits_sub6(struct ScreenBox *box)
{
    struct ScreenBoxBase box1 = {box->X + 5, box->Y + 47, 9, 14};

    lbDisplay.DrawFlags = (0x8000|Lb_SPRITE_TRANSPAR4);
    if (mouse_move_over_box(&box1))
    {
        if (lbDisplay.LeftButton)
        {
            if (mouse_down_over_box(&box1))
            {
                lbDisplay.LeftButton = 0;
                if (net_local_player_hosts_the_game() && ((net_game_play_flags & NGPF_Unkn01) == 0)
                    && (login_control__State == LognCt_Unkn5))
                {
                    reinit_starting_credits(-1);
                }
            }
        }
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    }
    lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
    lbDisplay.DrawColour = 87;
    draw_sprite_purple_list(box1.X, box1.Y, &fe_icons_sprites[108]);
}

void show_net_benefits_sub7(struct ScreenBox *box)
{
    struct ScreenBoxBase box2 = {box->X + 242, box->Y + 36, 9, 14};

    lbDisplay.DrawFlags = (0x8000|Lb_SPRITE_TRANSPAR4);
    if (mouse_move_over_box(&box2))
    {
        if (lbDisplay.LeftButton)
        {
            if (mouse_down_over_box(&box2))
            {
                lbDisplay.LeftButton = 0;
                if (net_local_player_hosts_the_game() && ((net_game_play_flags & NGPF_Unkn01) == 0)
                    && (login_control__State == LognCt_Unkn5))
                {
                    reinit_starting_credits(1);
                }
            }
        }
        lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    }
    lbDisplay.DrawFlags |= Lb_TEXT_ONE_COLOR;
    lbDisplay.DrawColour = 87;
    draw_sprite_purple_list(box2.X - 7, box2.Y, &fe_icons_sprites[109]);
}


ubyte show_net_benefits_box(struct ScreenBox *box)
{
    ubyte drawn = 1;

    my_set_text_window(box->X + 4, box->Y + 4, box->Width - 8, box->Height - 8);
    if ((box->Flags & GBxFlg_TextCopied) == 0)
    {
        lbFontPtr = med2_font;
        draw_text_purple_list2(30, 1, gui_strings[401], 0);
        draw_text_purple_list2(27, 32, gui_strings[402], 0);
        lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
        show_net_benefits_sub1(box->X + 8, box->Y + 16, byte_155174);
        show_net_benefits_sub1(box->X + 8, box->Y + 47, byte_155180);
        lbDisplay.DrawFlags = 0;
        copy_box_purple_list(box->X - 3, box->Y - 3,
            box->Width + 6, box->Height + 6);
        box->Flags |= GBxFlg_TextCopied;
    }

    show_net_benefits_sub2(box->X + 8, box->Y + 16, byte_155175);
    show_net_benefits_sub3(box);
    show_net_benefits_sub4(box);

    lbDisplay.DrawFlags = 0;

    show_net_benefits_sub5(box->X + 8, box->Y + 47, byte_155181);
    show_net_benefits_sub6(box);
    show_net_benefits_sub7(box);

    lbDisplay.DrawFlags = 0;
    if (net_local_player_hosts_the_game() && (login_control__State == LognCt_Unkn5))
    {
        drawn = net_SET2_button.DrawFn(&net_SET2_button);
        drawn = net_SET_button.DrawFn(&net_SET_button);
    }
    return drawn;
}

void purple_unkn1_data_to_screen(void)
{
    memcpy(dword_1C6DE4, dword_1C6DE8, 255 * 96);
}

void purple_unkn3_data_to_screen(void)
{
    LbScreenSetGraphicsWindow(net_grpaint.X + 4, net_grpaint.Y + 4,
      255, 96);
    LbScreenCopy(dword_1C6DE8, lbDisplay.GraphicsWindowPtr, lbDisplay.GraphicsWindowHeight);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
}

void purple_unkn4_data_to_screen(void)
{
    LbScreenSetGraphicsWindow(net_grpaint.X + 4, net_grpaint.Y + 4,
      255, 96);
    LbScreenCopy(dword_1C6DE4, lbDisplay.GraphicsWindowPtr, lbDisplay.GraphicsWindowHeight);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.GraphicsScreenWidth,
        lbDisplay.GraphicsScreenHeight);
}

void net_grpaint_draw_op(short scr_x2, short scr_y2, ubyte colno, sbyte op, ubyte a5)
{
    asm volatile (
      "push %4\n"
      "call ASM_net_grpaint_draw_op\n"
        : : "a" (scr_x2), "d" (scr_y2), "b" (colno), "c" (op), "g" (a5));
}

void net_grpaint_clear_op(void)
{
    draw_flic_purple_list(ac_purple_unkn1_data_to_screen);
}

ubyte show_net_grpaint(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_grpaint\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    int i;
    short dy;
    short ln_height;

    if ((p_box->Flags & 0x1000) == 0)
    {
        lbDisplay.DrawFlags = 0x0010;
        draw_box_purple_list(p_box->X + 263, p_box->Y + 4, 0xDu, 0x61u, 243);
        lbDisplay.DrawFlags = 0;
        dy = 0;
        for (i = 0; i < 4; i++)
        {
            draw_box_purple_list(p_box->X + 265, p_box->Y + dy + 6, 9, 21, byte_155170[i]);
            dy += 24;
        }
        draw_flic_purple_list(ac_purple_unkn3_data_to_screen);
        if (login_control__State != LognCt_Unkn5)
            draw_flic_purple_list(ac_purple_unkn1_data_to_screen);

        copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
          p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= 0x1000;
    }

    dy = 0;
    ln_height = 24;
    draw_flic_purple_list(purple_unkn4_data_to_screen);
    for (i = 0; i < 4; i++)
    {
        if (mouse_down_over_box_coords(p_box->X + 265, p_box->Y + dy + 6, p_box->X + 274, p_box->Y + dy + ln_height + 6))
        {
            if (lbDisplay.LeftButton)
            {
                lbDisplay.LeftButton = 0;
                play_sample_using_heap(0, 111, FULL_VOL, EQUL_PAN, NORM_PTCH, 0, 2u);
                byte_1C47EA = i;
            }
        }
      dy += ln_height;
    }

    if (mouse_move_over_box_coords(p_box->X + 4, p_box->Y + 4 , p_box->X + 259, p_box->Y + 100))
    {
        short plyr;
        ubyte paint_action;

        plyr = LbNetworkPlayerNumber();
        paint_action = NPAct_NONE;
        if (lbDisplay.MLeftButton)
        {
            lbDisplay.LeftButton = 0;
            if ((lbShift & 0x01) != 0)
                paint_action = NPAct_GrPaintDrawPt;
            else
                paint_action = NPAct_GrPaintDrawLn;
        }
        else if (net_player_no_action_scheduled(plyr))
        {
            paint_action = NPAct_GrPaintPt1Upd;
        }
        if (paint_action != NPAct_NONE)
        {
            short pos_x, pos_y;

            pos_x = mouse_move_position_horizonal_over_bar_coords(p_box->X + 4, p_box->Width);
            pos_y = mouse_move_position_vertical_over_bar_coords(p_box->Y + 4, p_box->Height);
            net_schedule_player_grpaint_action_sync(paint_action, pos_x, pos_y, byte_1C47EA);
        }
        if (lbDisplay.MRightButton)
        {
            lbDisplay.RightButton = 0;
            if (net_local_player_hosts_the_game())
                net_schedule_player_grpaint_clear_sync();
        }
    }
    return 0;
}

ubyte show_net_comms_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_comms_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    char plyrname[20];
    char locstr[40];
    int i;
    short dx;
    short tx_height;

    if (byte_1C4805)
    {
        byte_1C4805 = 0;
        net_unkn1_text[0] = '\0';
    }
    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    if ((p_box->Flags & 0x1000) != 0)
    {
        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
    }
    else
    {
        lbFontPtr = med2_font;
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        draw_text_purple_list2(0, 0, gui_strings[393], 0);
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X + 4, p_box->Y + 18, p_box->Width - 8, 61, 56);
        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
        draw_box_purple_list(p_box->X + 4, p_box->Y + 87, p_box->Width - 8, tx_height + 6, 56);
        lbDisplay.DrawFlags = 0;
        copy_box_purple_list(p_box->X - 3, p_box->Y -  3, p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= 0x1000;
        reset_buffered_keys();
    }

    if (login_control__State != 5)
    {
        edit_flag = 0;
        return 0;
    }

    edit_flag = 1;
    dx = 14;
    for (i = 0; i < 5; i++)
    {
        if ((LbNetworkPlayerName(plyrname, byte_1C6DDC[i]) == 1) &&
          (net_players[i].field_0[0] != '\0'))
        {
            const char *text;

            plyrname[7] = '\0';
            snprintf(locstr, sizeof(locstr), "%s: %s", plyrname, net_players[i].field_0);
            text = loctext_to_gtext(locstr);
            draw_text_purple_list2(2, dx + 5, text, 0);
            dx += tx_height + 4;
        }
    }

    if (user_read_value(net_unkn1_text, 20, 0) && (login_control__State == 5)
      && (net_unkn1_text[0] != '\0'))
    {
        net_schedule_player_chat_message_sync(net_unkn1_text);
        byte_1C4805 = 1;
    }
    draw_text_purple_list2(2, 86, net_unkn1_text, 0);

    if ((gameturn & 1) != 0)
    {
        const struct TbSprite *p_spr;

        p_spr = LbFontCharSprite(lbFontPtr, 45);
        tx_height = my_string_width(net_unkn1_text);
        draw_sprite_purple_list(p_box->X + 6 + tx_height, p_box->Y + 92, p_spr);
    }
    return 0;
}

ubyte do_net_protocol_select(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_net_protocol_select\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    short proto;
    short pos_x;

    net_service_stop();

    pos_x = net_protocol_select_button.X - 12 + net_protocol_select_button.Width + 4;

    proto = nsvc.I.Type;
    if (click)
    {
        proto--;
        if (proto <= NetSvc_NONE)
        {
            proto = NetSvc_COM4;
            if (data_1c4a70)
            {
                net_protocol_select_button.X -= 12;
                net_unkn40_button.X = pos_x;
                byte_1C4A6F = 1;
            }
        }
        else if (proto == NetSvc_IPX) // IPX needs to be accepted
        {
            if (byte_1C4A6F)
            {
                byte_1C4A6F = 0;
                proto = NetSvc_COM4;
                net_protocol_select_button.X += 12;
            }
        }
    }
    else
    {
        proto++;
        if (proto > NetSvc_COM4)
        {
            if (byte_1C4A6F || !data_1c4a70)
            {
                proto = NetSvc_IPX;
                if (byte_1C4A6F)
                {
                    byte_1C4A6F = 0;
                    net_protocol_select_button.X += 12;
                }
            }
            else
            {
                proto = NetSvc_COM1;
                net_protocol_select_button.X -= 12;
                net_unkn40_button.X = pos_x;
                byte_1C4A6F = 1;
            }
        }
    }

    net_service_switch(proto);

    switch (nsvc.I.Type)
    {
    case NetSvc_NONE:
    default:
        break;
    case NetSvc_IPX:
        if (!net_service_restart()) {
            // Switch to a service which is always available
            net_service_switch(NetSvc_COM1);
            break;
        }
        byte_15516C = -1;
        break;
    case NetSvc_COM1:
    case NetSvc_COM2:
    case NetSvc_COM3:
    case NetSvc_COM4:
        byte_15516C = 0;
        break;
    }
    return 1;
}

ubyte show_net_protocol_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_protocol_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    const char *text;
    short tx_height, tx_width;
    short scr_x, scr_y;
    ubyte drawn;

    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);

    if (login_control__State == 5)
    {
        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
        lbDisplay.DrawColour = 87;

        scr_y = 2;
        text = gui_strings[506];
        draw_text_purple_list2(4, scr_y, text, 0);

        scr_y += tx_height + 4;
        text = gui_strings[505];
        draw_text_purple_list2(4, scr_y, text, 0);

        scr_y += tx_height + 4;
        text = gui_strings[515];
        draw_text_purple_list2(4, scr_y, text, 0);

        scr_y += tx_height + 4;
        text = gui_strings[538];
        draw_text_purple_list2(4, scr_y, text, 0);
        lbDisplay.DrawFlags |= (0x8000 | 0x0040);

        scr_y = 2;
        if ((net_game_play_flags & NGPF_Unkn04) != 0)
            text = gui_strings[479];
        else
            text = gui_strings[478];
        tx_width = my_string_width(text);
        draw_text_purple_list2(165 - (tx_width >> 1), scr_y, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;
        if (net_local_player_hosts_the_game())
        {
            if (mouse_down_over_box_coords(text_window_x1 + 4, text_window_y1 + scr_y - 2,
              text_window_x1 + (tx_width >> 1) + 165, text_window_y1 + tx_height + scr_y + 2))
            {
                if (lbDisplay.LeftButton)
                {
                    lbDisplay.LeftButton = 0;
                    if ((net_game_play_flags & NGPF_Unkn04) != 0)
                        net_game_play_flags &= ~NGPF_Unkn04;
                    else
                        net_game_play_flags |= NGPF_Unkn04;
                    net_schedule_game_options_sync();
                }
            }
        }

        scr_y += tx_height + 4;
        lbDisplay.DrawFlags |= 0x8000u;
        if ((net_game_play_flags & NGPF_Unkn08) != 0)
            text = gui_strings[479];
        else
            text = gui_strings[478];
        tx_width = my_string_width(text);
        draw_text_purple_list2(165 - (tx_width >> 1), scr_y, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;
        if (net_local_player_hosts_the_game())
        {
            if (mouse_down_over_box_coords(text_window_x1 + 4, text_window_y1 + scr_y - 2,
              text_window_x1 + (tx_width >> 1) + 165, text_window_y1 + scr_y + tx_height + 2))
            {
                if (lbDisplay.LeftButton)
                {
                    lbDisplay.LeftButton = 0;
                    if ((net_game_play_flags & NGPF_Unkn08) != 0)
                        net_game_play_flags &= ~NGPF_Unkn08;
                    else
                        net_game_play_flags |= NGPF_Unkn08;
                    net_schedule_game_options_sync();
                }
            }
        }

        scr_y += tx_height + 4;
        lbDisplay.DrawFlags |= 0x8000;
        if ((net_game_play_flags & NGPF_Unkn10) != 0)
            text = gui_strings[479];
        else
            text = gui_strings[478];
        tx_width = my_string_width(text);
        draw_text_purple_list2(165 - (tx_width >> 1), scr_y, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;
        if (net_local_player_hosts_the_game())
        {
            if (mouse_down_over_box_coords(text_window_x1 + 4, text_window_y1 + scr_y - 2,
              text_window_x1 + (tx_width >> 1) + 165, text_window_y1 + scr_y + tx_height + 2))
            {
                if (lbDisplay.LeftButton)
                {
                    lbDisplay.LeftButton = 0;
                    if ((net_game_play_flags & NGPF_Unkn10) != 0)
                        net_game_play_flags &= ~NGPF_Unkn10;
                    else
                        net_game_play_flags |= NGPF_Unkn10;
                    net_schedule_game_options_sync();
                }
            }
        }

        scr_y += tx_height + 4;
        lbDisplay.DrawFlags |= 0x8000;
        if ((net_game_play_flags & NGPF_Unkn20) != 0)
            text = gui_strings[479];
        else
            text = gui_strings[478];
        tx_width = my_string_width(text);
        draw_text_purple_list2(165 - (tx_width >> 1), scr_y, text, 0);
        lbDisplay.DrawFlags &= ~0x8000;
        if (net_local_player_hosts_the_game())
        {
            if (mouse_down_over_box_coords(text_window_x1 + 4, text_window_y1 + scr_y - 2,
              text_window_x1 + (tx_width >> 1) + 165, text_window_y1 + scr_y + tx_height + 2))
            {
                if (lbDisplay.LeftButton)
                {
                    lbDisplay.LeftButton = 0;
                    if ((net_game_play_flags & NGPF_Unkn20) != 0)
                        net_game_play_flags &= ~NGPF_Unkn20;
                    else
                        net_game_play_flags |= NGPF_Unkn20;
                    net_schedule_game_options_sync();
                }
            }
        }
        lbDisplay.DrawFlags &= ~0x0040;
    }
    else
    {
        TbBool draw_option;

        lbFontPtr = med2_font;
        tx_height = my_char_height('A');
        draw_option = true;

        if (nsvc.I.Type == NetSvc_IPX)
        {
            tx_width = 5 * LbTextCharWidth('A');
            scr_x = ((p_box->Width - tx_width) >> 1) - 4;
            lbDisplay.DrawFlags |= 0x0004;
            draw_box_purple_list(text_window_x1 + scr_x, text_window_y1 + 22, tx_width + 4, tx_height + 4, 243);
            lbDisplay.DrawFlags &= ~0x0004;

            lbDisplay.DrawFlags |= 0x8000;
            draw_text_purple_list2(scr_x + 2, 24, net_proto_param_text, 0);
            lbDisplay.DrawFlags &= ~0x8000;

            if (byte_1C4806 == 0)
            {
                tx_width = LbTextStringWidth(net_proto_param_text);
                if (mouse_down_over_box_coords(text_window_x1 + scr_x + 2, text_window_y1 + 22,
                  text_window_x1 + scr_x + 2 + tx_width, text_window_y1 + tx_height + 24 + 2))
                {
                    if (lbDisplay.LeftButton)
                    {
                        lbDisplay.LeftButton = 0;
                        byte_1C4806 = 1;
                        reset_buffered_keys();
                    }
                }
            }

            if (byte_1C4806 == 1)
            {
                if ((gameturn & 2) != 0)
                {
                    const struct TbSprite *p_spr;
                    p_spr = LbFontCharSprite(lbFontPtr, 45);
                    tx_width = LbTextStringWidth(net_proto_param_text);
                    draw_sprite_purple_list(text_window_x1 + scr_x + 2 + tx_width, text_window_y1 + 27, p_spr);
                }
                if (user_read_value(net_proto_param_text, 4, 3))
                {
                    uint addr;

                    byte_1C4806 = 0;
                    addr = 0;
                    sscanf(net_proto_param_text, "%04x", &addr);
                    nsvc.I.Param = addr;
                    LbNetworkSetupIPXAddress(addr);

                    if (!net_service_restart()) {
                        // Switch to a service which is always available
                        net_service_switch(NetSvc_COM1);
                    }
                }
            }
            draw_option = false;
        }
        else
        {
            if (byte_1C4A6F)
            {
              drawn = net_unkn40_button.DrawFn(&net_unkn40_button);
              if (drawn == 3)
              {
                  scr_y = net_unkn40_button.Y + 3;
                  scr_x = net_unkn40_button.X + 3;
                  draw_line_purple_list(scr_x +  0, scr_y + 0, scr_x +  3, scr_y + 0, 174);
                  draw_line_purple_list(scr_x +  3, scr_y + 0, scr_x +  3, scr_y + 9, 174);
                  draw_line_purple_list(scr_x +  3, scr_y + 9, scr_x +  6, scr_y + 9, 174);
                  draw_line_purple_list(scr_x +  6, scr_y + 9, scr_x +  6, scr_y + 0, 174);
                  draw_line_purple_list(scr_x +  6, scr_y + 0, scr_x +  9, scr_y + 0, 174);
                  draw_line_purple_list(scr_x +  9, scr_y + 0, scr_x +  9, scr_y + 9, 174);
                  draw_line_purple_list(scr_x +  9, scr_y + 9, scr_x + 12, scr_y + 9, 174);
                  draw_line_purple_list(scr_x + 12, scr_y + 9, scr_x + 12, scr_y + 0, 174);
                  draw_line_purple_list(scr_x + 12, scr_y + 0, scr_x + 15, scr_y + 0, 174);
              }
              my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
              if (!byte_1C4994) // Instead of option box, draw keyboard input
              {
                  tx_width = 15 * LbTextCharWidth('A');
                  scr_x = ((p_box->Width - tx_width) >> 1) - 4;
                  scr_y = 22;
                  lbDisplay.DrawFlags |= 0x0004;
                  draw_box_purple_list(scr_x + text_window_x1, text_window_y1 + scr_y, tx_width + 4, tx_height + 4, 243);
                  lbDisplay.DrawFlags &= ~0x0004;
                  lbDisplay.DrawFlags |= 0x8000;
                  draw_text_purple_list2(scr_x + 2, scr_y + 2, net_unkn2_text, 0);
                  lbDisplay.DrawFlags &= ~0x8000;
                  if (!byte_1C4806)
                  {
                      tx_width = LbTextStringWidth(net_unkn2_text);
                      if (mouse_down_over_box_coords(text_window_x1 + scr_x + 2, text_window_y1 + scr_y,
                        text_window_x1 + scr_x + 2 + tx_width, text_window_y1 + scr_y + tx_height + 2 + 2))
                      {
                          if (lbDisplay.LeftButton)
                          {
                              lbDisplay.LeftButton = 0;
                              byte_1C4806 = 1;
                              reset_buffered_keys();
                          }
                      }
                  }
                  if (byte_1C4806 == 1)
                  {
                      if ((gameturn & 2) != 0)
                      {
                          const struct TbSprite *p_spr;
                          p_spr = LbFontCharSprite(lbFontPtr, 45);
                          tx_width = LbTextStringWidth(net_unkn2_text);
                          draw_sprite_purple_list(text_window_x1 + scr_x + 2 + tx_width, text_window_y1 + 27, p_spr);
                      }
                      if (user_read_value(net_unkn2_text, 0xEu, 2))
                      {
                          byte_1C4806 = 0;
                      }
                  }
                  draw_option = false;
              }
              byte_1C4806 = 0;
            }
        }
        if (draw_option) {
            net_protocol_option_button.DrawFn(&net_protocol_option_button);
        }
        net_protocol_select_button.DrawFn(&net_protocol_select_button);
    }
    return 0;
}

ubyte show_net_faction_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_faction_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short tx_height;
    short scr_y;
    int i;

    my_set_text_window(p_box->X + 2, p_box->Y + 4, p_box->Width - 4, p_box->Height - 8);

    if ((p_box->Flags & 0x1000) == 0)
    {
        lbFontPtr = med2_font;
        tx_height = my_char_height('A');
        lbDisplay.DrawFlags = 0x0100;
        draw_text_purple_list2(0, 0, gui_strings[392], 0);
        scr_y = tx_height + 10;

        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
        lbDisplay.DrawFlags = 0x0004;

        for (i = 0; i < 2; i++)
        {
            draw_box_purple_list(p_box->X + 4, p_box->Y + 4 + scr_y,
              p_box->Width - 8, tx_height + 6, 56);
            scr_y += tx_height + 9;
        }
        lbDisplay.DrawFlags = 0;

        copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
          p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= 0x1000;
    }

    lbFontPtr = small_med_font;
    tx_height = my_char_height('A');

    scr_y = 20;
    for (i = 0; i < 2; i++)
    {
        if (byte_181183 == i)
        {
            lbDisplay.DrawFlags = (0x0040 | 0x0100);
            lbDisplay.DrawColour = 87;
        }
        else
        {
            lbDisplay.DrawFlags = 0x0100;
        }
        lbDisplay.DrawFlags |= 0x8000;
        draw_text_purple_list2(0, scr_y + 3, gui_strings[394 + i], 0);
        lbDisplay.DrawFlags &= ~0x8000;

        if (mouse_down_over_box_coords(text_window_x1, text_window_y1 - 2,
           text_window_x2, text_window_y1 + scr_y + tx_height + 2))
        {
            if (lbDisplay.LeftButton)
            {
              lbDisplay.LeftButton = 0;
              byte_181183 = i;
              net_schedule_player_faction_change_sync();
            }
        }
        scr_y += tx_height + 9;
    }
    lbDisplay.DrawFlags = 0;
    return 0;
}

ubyte show_net_team_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_team_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    short scr_y;
    short tx_height;
    int i;

    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);

    if ((p_box->Flags & 0x1000) != 0)
    {
        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
    }
    else
    {
        lbFontPtr = med2_font;
        tx_height = my_char_height('A');
        scr_y = 0;
        lbDisplay.DrawFlags = 0x0100;
        draw_text_purple_list2(0, scr_y, gui_strings[391], 0);
        scr_y += tx_height + 3;

        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
        lbDisplay.DrawFlags = 0x0004;

        for (i = 0; i < 4; i++)
        {
            draw_box_purple_list(p_box->X + 4, p_box->Y + 4 + scr_y,
              p_box->Width - 8, tx_height + 4, 56);
            scr_y += tx_height + 5;
        }
        lbDisplay.DrawFlags = 0;
        copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
          p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= 0x1000;
    }

    scr_y = 13;
    for (i = 0; i < 4; i++)
    {
        if (byte_181189 == i + 1)
        {
            lbDisplay.DrawFlags = 0x140;
            lbDisplay.DrawColour = 87;
        }
        else
        {
            lbDisplay.DrawFlags = 0x100;
        }
        lbDisplay.DrawFlags |= 0x8000;
        draw_text_purple_list2(0, scr_y + 2, gui_strings[397 + i], 0);
        lbDisplay.DrawFlags &= ~0x8000;

        if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y - 2,
           text_window_x2, text_window_y1 + scr_y + tx_height + 2))
        {
            if (lbDisplay.LeftButton)
            {
              lbDisplay.LeftButton = 0;
              if (byte_181189 == i + 1)
                  byte_181189 = 0;
              else
                  byte_181189 = i + 1;
              net_schedule_player_team_change_sync();
            }
        }
        scr_y += tx_height + 5;
    }
    lbDisplay.DrawFlags = 0;
    return 0;
}

static const char *net_group_name_to_gtext(const char *name)
{
    char namestr[16];
    char locstr[28];

    snprintf(namestr, sizeof(namestr), "%s", name);
    fontstrtoupper(namestr);
    snprintf(locstr, sizeof(locstr), "%s'S %s", namestr, gui_strings[446]);
    return loctext_to_gtext(locstr);
}

ubyte show_net_groups_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_groups_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    const char *text;
    int i;
    short tx_height;
    short scr_y;

    if ((p_box->Flags & 0x1000) == 0)
    {
        short lines_height;
        my_set_text_window(p_box->X + 2, p_box->Y + 4, p_box->Width - 4, p_box->Height - 8);

        lbFontPtr = med2_font;
        tx_height = my_char_height('A');
        scr_y = 1;
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
        text = gui_strings[390];
        draw_text_purple_list2(0, scr_y, text, 0);
        lbDisplay.DrawFlags = 0;

        scr_y += tx_height + 4;
        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
        lines_height = 8 * tx_height;
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X + 4, p_box->Y + 4 + scr_y, p_box->Width - 8, lines_height + 34, 56);
        lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;

        copy_box_purple_list(p_box->X - 3, p_box->Y - 3, p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= 0x1000;
    }

    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    lbFontPtr = small_med_font;
    tx_height = my_char_height('A');

    scr_y = 19;
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    if (login_control__State == 5)
    {
        text = net_group_name_to_gtext(nsvc.S.Name);
        draw_text_purple_list2(0, scr_y, text, 0);
    }
    else
    {
        if (nsvc.I.Type == NetSvc_IPX)
        {
            struct TbNetworkSession *p_nsession;

            for (i = 0; i < byte_1C6D48; i++)
            {
                p_nsession = &unkstruct04_arr[i].Session;

                if (byte_15516C == i)
                {
                    lbDisplay.DrawFlags = (Lb_TEXT_HALIGN_CENTER | Lb_TEXT_ONE_COLOR);
                    lbDisplay.DrawColour = 87;
                }
                else
                {
                    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
                }
                text = net_group_name_to_gtext(p_nsession->Name);
                lbDisplay.DrawFlags |= 0x8000;
                draw_text_purple_list2(0, scr_y, text, 0);
                lbDisplay.DrawFlags &= ~0x8000;

                if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y - 2,
                  text_window_x2, text_window_y1 + scr_y + tx_height + 2))
                {
                    if (lbDisplay.LeftButton)
                    {
                        lbDisplay.LeftButton = 0;
                        if (byte_15516C == i)
                            byte_15516C = -1;
                        else
                            byte_15516C = i;
                    }
                }
                scr_y += tx_height + 4;
            }
        }
    }

    if (net_local_player_hosts_the_game())
    {
        net_INITIATE_button.DrawFn(&net_INITIATE_button);
        if (byte_15516D != -1) {
            unkn8_EJECT_button.DrawFn(&unkn8_EJECT_button);
        }
    }
    if ((byte_15516C != -1) || (login_control__State == 5))
    {
        net_groups_LOGON_button.DrawFn(&net_groups_LOGON_button);
    }
    return 0;
}

int refresh_users_in_net_game(void)
{
    char locstr[16];
    int n;
    short plyr;

    n = 0;
    ingame.InNetGame_UNSURE = (1 << PLAYERS_LIMIT) - 1;
    for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++)
    {
        TbResult ret;
        ret = LbNetworkPlayerName(locstr, plyr);
        if (ret != Lb_SUCCESS)
        {
            unkn2_names[plyr][0] = '\0';
            ingame.InNetGame_UNSURE &= ~(1 << plyr);
            continue;
        }
        if (locstr[0] == '\0')
        {
            unkn2_names[plyr][0] = '\0';
            ingame.InNetGame_UNSURE &= ~(1 << plyr);
            continue;
        }
        strncpy(unkn2_names[plyr], locstr, sizeof(unkn2_names[0]));
        n++;
    }
    LOGSYNC("Net players %d", n);
    return n;
}

ubyte show_net_users_box(struct ScreenBox *p_box)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_show_net_users_box\n"
        : "=r" (ret) : "a" (p_box));
    return ret;
#endif
    const char *text;
    short plyr;
    short scr_x, scr_y;
    short tx_width, tx_height;

    my_set_text_window(p_box->X + 4, p_box->Y + 4, p_box->Width - 8, p_box->Height - 8);
    if ((p_box->Flags & 0x1000) == 0)
    {
        lbFontPtr = med2_font;
        tx_height = my_char_height('A');
        lbDisplay.DrawFlags = 0x0100;
        draw_text_purple_list2(0, 1, gui_strings[389], 0);
        lbDisplay.DrawFlags = 0;
        scr_y = tx_height + 8;
        lbFontPtr = small_med_font;
        tx_height = my_char_height('A');
        lbDisplay.DrawFlags = 0x0004;
        for (plyr = 0; plyr < 8; plyr++)
        {
            struct TbSprite *p_spr;
            p_spr = &fe_icons_sprites[138];
            draw_box_purple_list(p_box->X + 4, p_box->Y + scr_y + 4, 109, tx_height + 6, 56);
            draw_sprite_purple_list(p_box->X + 113, p_box->Y + scr_y + 4, p_spr);
            draw_box_purple_list(p_box->X + p_spr->SWidth + 113, p_box->Y + scr_y + 4, 15, tx_height + 6, 56);
            scr_y += tx_height + 9;
        }
        lbDisplay.DrawFlags = 0;

        copy_box_purple_list(p_box->X - 3, p_box->Y - 3, p_box->Width + 6, p_box->Height + 6);
        p_box->Flags |= 0x1000;
    }

    lbFontPtr = small_med_font;
    tx_height = my_char_height('A');
    scr_y = 18;
    if (login_control__State == 5)
    {
        refresh_users_in_net_game();

        for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++)
        {
            text = unkn2_names[plyr];
            if (text[0] == '\0')
            {
                continue;
            }
            if (byte_15516D == plyr)
            {
                lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
                lbDisplay.DrawColour = 87;
            }
            else
            {
                lbDisplay.DrawFlags = 0;
            }
            tx_width = my_string_width(text);
            scr_x = (110 - tx_width) >> 1;
            if (net_local_player_hosts_the_game())
                lbDisplay.DrawFlags |= 0x8000;
            text = loctext_to_gtext(text);
            draw_text_purple_list2(scr_x, scr_y + 3, text, 0);
            lbDisplay.DrawFlags &= ~0x8000;

            text = gui_strings[394 + group_types[plyr]];
            scr_x = 139 + ((64 - my_string_width(text)) >> 1);
            draw_text_purple_list2(scr_x, scr_y + 3, text, 0);
            if (byte_1C5C28[plyr])
            {
                struct TbSprite *p_spr, *p_dspr;
                p_spr = &fe_icons_sprites[138];
                p_dspr = &fe_icons_sprites[109 + byte_1C5C28[plyr]];
                draw_sprite_purple_list(p_box->X + (112 + p_spr->SWidth) + 4,
                  p_box->Y + 4 + scr_y + 2, p_dspr);
            }
            if (net_local_player_hosts_the_game())
            {
                if (mouse_down_over_box_coords(text_window_x1, text_window_y1 + scr_y + 1,
                  text_window_x2, text_window_y1 + tx_height + scr_y + 5))
                {
                    if (lbDisplay.LeftButton)
                    {
                        lbDisplay.LeftButton = 0;
                        if (byte_15516D == plyr)
                            byte_15516D = -1;
                        else
                            byte_15516D = plyr;
                    }
                }
            }
            scr_y += tx_height + 9;
        }
    }
    else if (byte_15516C != -1)
    {
        struct TbNetworkPlayer *p_netplyr_lst;
        p_netplyr_lst = unkstruct04_arr[byte_15516C].Player;
        for (plyr = 0; plyr < PLAYERS_LIMIT; plyr++)
        {
            const char *name;
            name = p_netplyr_lst[plyr].Name;
            if (name[0] != '\0')
            {
                tx_width = my_string_width(name);
                scr_x = (110 - tx_width) >> 1;
                text = loctext_to_gtext(name);
                draw_text_purple_list2(scr_x, scr_y + 3, text, 0);
                scr_y += tx_height + 9;
            }
        }
    }
    return 0;
}

void net_sessionlist_remove_old(void)
{
    int sess_no;

    for (sess_no = 0; sess_no < byte_1C6D48; sess_no++)
    {
        if (dos_clock() - sessionlist_last_update[sess_no] > 4 * DOS_CLOCKS_PER_SEC)
        {
            LOGSYNC("Retiring session %d", sess_no);
            net_sessionlist_remove(sess_no);
            sess_no--;
        }
    }
}

int net_unkn_func_30(void)
{
#if 0
    int ret;
    asm volatile ("call ASM_net_unkn_func_30\n"
        : "=r" (ret) : );
    return ret;
#endif
    int preval;

    if (byte_1C6D48 < 20)
    {
        net_sessionlist_update_latest_one();
        net_sessionlist_remove_old();
    }
    preval = byte_15516C;
    if (byte_15516C == -1 && byte_1C6D48)
        byte_15516C = 0;
    if (!byte_1C6D48)
        byte_15516C = -1;
    return preval;
}

ubyte do_unkn8_EJECT(ubyte click)
{
#if 0
    ubyte ret;
    asm volatile ("call ASM_do_unkn8_EJECT\n"
        : "=r" (ret) : "a" (click));
    return ret;
#endif
    int plyr;

    plyr = LbNetworkPlayerNumber();
    if (byte_15516D == plyr)
        return 0;
    net_schedule_player_eject_sync();
    return 1;
}

void show_netgame_unkn_case1(void)
{
#if 0
    asm volatile (
      "call ASM_show_netgame_unkn_case1\n"
        :  :  : "eax" );
    return;
#endif
    if (!net_autostart_done)
    {
        net_autostart_done = 1;

        nsvc.I.GameId = 0xD15C;
        nsvc.I.Param = 0;
        nsvc.I.Type = NetSvc_IPX;

        strncpy(byte_1811E2, login_name, sizeof(byte_1811E2));
        byte_1811E2[8] = '\0';

        if (!net_service_restart()) {
            // Switch to a service which is always available
            net_service_switch(NetSvc_COM1);
            return;
        }
    }
    net_protocol_box.DrawFn(&net_protocol_box);
    net_groups_box.DrawFn(&net_groups_box);
    net_users_box.DrawFn(&net_users_box);
    net_faction_box.DrawFn(&net_faction_box);
    net_team_box.DrawFn(&net_team_box);
    net_benefits_box.DrawFn(&net_benefits_box);
    net_comms_box.DrawFn(&net_comms_box);
    net_grpaint.DrawFn(&net_grpaint);

    if ((login_control__State == 6) && (nsvc.I.Type == NetSvc_IPX)) {
        net_unkn_func_30();
    }
}

void init_net_screen_boxes(void)
{
    ScrCoord scr_w, scr_h, start_x, start_y;
    short space_w, space_h, border;

    // Border value represents how much the box background goes
    // out of the box area.
    border = 3;
    scr_w = lbDisplay.GraphicsWindowWidth;
#ifdef EXPERIMENTAL_MENU_CENTER_H
    scr_h = global_apps_bar_box.Y;
#else
    scr_h = 432;
#endif

    init_screen_box(&net_groups_box, 213u, 72u, 171u, 155, 6);
    init_screen_box(&net_users_box, 393u, 72u, 240u, 155, 6);

    init_screen_box(&net_faction_box, 213u, 236u, 73u, 67, 6);
    init_screen_box(&net_team_box, 295u, 236u, 72u, 67, 6);
    init_screen_box(&net_benefits_box, 376u, 236u, 257u, 67, 6);
    init_screen_box(&net_protocol_box, 7u, 252u, 197u, 51, 6);

    init_screen_box(&net_grpaint, 7u, 312u, 279u, 104, 6);
    init_screen_box(&net_comms_box, 295u, 312u, 336u, 104, 6);

    init_screen_button(&net_INITIATE_button, 218u, 185u, gui_strings[385], 6,
        med2_font, 1, 0);
    init_screen_button(&net_groups_LOGON_button, 218u, 206u, gui_strings[386],
        6, med2_font, 1, 0);
    init_screen_button(&unkn8_EJECT_button, 308u, 206u, gui_strings[403], 6,
        med2_font, 1, 0);

    init_screen_button(&net_SET2_button, 562u, 251u, gui_strings[440], 6,
        med2_font, 1, 0);
    init_screen_button(&net_SET_button, 562u, 284u, gui_strings[440], 6,
        med2_font, 1, 0);

    init_screen_button(&net_protocol_select_button, 37u, 256u, gui_strings[498],
        6, med2_font, 1, 0);
    init_screen_button(&net_unkn40_button, 37u, 256u, net_unkn40_text, 6,
        med2_font, 1, 0);
    init_screen_button(&net_protocol_option_button, 7u, 275u,
        net_proto_param_text, 6, med2_font, 1, 0);

    net_groups_LOGON_button.Width = 85;
    net_INITIATE_button.Width = 85;
    net_unkn40_button.Width = 21;
    net_protocol_select_button.Width = 157;
    net_protocol_option_button.Width = net_protocol_select_button.Width;
    net_protocol_option_button.CallBackFn = ac_do_net_protocol_option;
    net_INITIATE_button.CallBackFn = ac_do_net_INITIATE;
    net_faction_box.SpecialDrawFn = show_net_faction_box;
    net_team_box.SpecialDrawFn = show_net_team_box;
    net_groups_box.Flags |= GBxFlg_RadioBtn|GBxFlg_IsMouseOver;

    net_groups_box.SpecialDrawFn = show_net_groups_box;
    net_users_box.SpecialDrawFn = show_net_users_box;
    net_benefits_box.SpecialDrawFn = show_net_benefits_box;
    net_unkn40_button.CallBackFn = ac_do_net_unkn40;
    net_SET_button.CallBackFn = ac_do_net_SET;
    net_protocol_select_button.CallBackFn = ac_do_net_protocol_select;
    unkn8_EJECT_button.CallBackFn = ac_do_unkn8_EJECT;
    net_comms_box.SpecialDrawFn = show_net_comms_box;
    net_users_box.Flags |= GBxFlg_RadioBtn|GBxFlg_IsMouseOver;
    net_groups_LOGON_button.CallBackFn = ac_do_net_groups_LOGON;
    net_grpaint.SpecialDrawFn = show_net_grpaint;
    net_SET2_button.CallBackFn = ac_do_net_SET2;
    net_protocol_box.SpecialDrawFn = show_net_protocol_box;

    // Reposition the components to current resolution

    start_x = unkn13_SYSTEM_button.X + unkn13_SYSTEM_button.Width;
    // On the X axis, we're going for centering on the screen. So subtract the previous
    // button position two times - once for the left, and once to make the same space on
    // the right.
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - net_groups_box.Width - net_users_box.Width;

    start_y = system_screen_shared_header_box.Y + system_screen_shared_header_box.Height;
    // On the top, we're aligning to spilled border of previous box; same goes inside.
    // But on the bottom, we're aligning to hard border, without spilling. To compensate
    // for that, add pixels for such border to the space.
    space_h = scr_h - start_y - net_users_box.Height - net_benefits_box.Height - net_comms_box.Height + border;

    // There are 2 boxes to position in X axis, and no space is needed after - the
    // available empty space is divided into 2.
    net_groups_box.X = start_x + space_w / 2;
    // There are 3 boxes to position in Y axis, so space goes into 4 parts - before, between and after.
    net_groups_box.Y = start_y + space_h / 4;
    net_users_box.X = net_groups_box.X + net_groups_box.Width + space_w - space_w / 2;
    net_users_box.Y = net_groups_box.Y;

    // Next row - re-compute space in one dimension
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - net_faction_box.Width - net_team_box.Width - net_benefits_box.Width;

    net_faction_box.X = start_x + space_w / 3;
    net_faction_box.Y = net_groups_box.Y + net_groups_box.Height + space_h / 4;
    net_team_box.X = net_faction_box.X + net_faction_box.Width  + space_w / 3;
    net_team_box.Y = net_faction_box.Y;
    net_benefits_box.X = net_team_box.X + net_team_box.Width + space_w - 2 * (space_w / 3);
    net_benefits_box.Y = net_faction_box.Y;

    // The remaining components are positioned below the system menu
    start_x = unkn13_SYSTEM_button.X;

    net_protocol_box.X = start_x;
    net_protocol_box.Y = net_faction_box.Y + net_faction_box.Height - net_protocol_box.Height;

    // Next row - re-compute space in one dimension
    space_w = scr_w - start_x - unkn13_SYSTEM_button.X - net_grpaint.Width - net_comms_box.Width;

    net_grpaint.X = start_x;
    net_grpaint.Y = net_benefits_box.Y + net_benefits_box.Height + space_h / 4;
    net_comms_box.X = net_grpaint.X + net_grpaint.Width + space_w;
    net_comms_box.Y = net_grpaint.Y;

    // Two buttons on top of each other
    net_protocol_select_button.X = net_protocol_box.X +
      (net_protocol_box.Width - net_protocol_select_button.Width) / 2;
    net_protocol_select_button.Y = net_protocol_box.Y + net_protocol_box.Height - 43;
    net_unkn40_button.X = net_protocol_box.X +
      (net_protocol_box.Width - net_unkn40_button.Width) / 2;
    net_unkn40_button.Y = net_protocol_box.Y + net_protocol_box.Height - 43;

    net_protocol_option_button.X = net_protocol_box.X +
      (net_protocol_box.Width - net_protocol_option_button.Width) / 2;
    net_protocol_option_button.Y = net_protocol_box.Y + net_protocol_box.Height - 24;

    net_INITIATE_button.X = net_groups_box.X + 5;
    net_INITIATE_button.Y = net_groups_box.Y + net_groups_box.Height - 42;
    net_groups_LOGON_button.X = net_groups_box.X + 5;
    net_groups_LOGON_button.Y = net_groups_box.Y + net_groups_box.Height - 21;

    unkn8_EJECT_button.X = net_groups_box.X + net_groups_box.Width - 76;
    unkn8_EJECT_button.Y = net_groups_box.Y + net_groups_box.Height - 21;
    net_SET2_button.X = net_benefits_box.X + net_benefits_box.Width - 71;
    net_SET2_button.Y = net_benefits_box.Y + net_benefits_box.Height - 52;
    net_SET_button.X = net_benefits_box.X + net_benefits_box.Width - 71;
    net_SET_button.Y = net_benefits_box.Y + net_benefits_box.Height - 19;
}

void reset_net_screen_boxes_flags(void)
{
    net_users_box.Flags = GBxFlg_Unkn0001;
    net_groups_box.Flags = GBxFlg_Unkn0001;
    net_benefits_box.Flags = GBxFlg_Unkn0001;
    net_team_box.Flags = GBxFlg_Unkn0001;
    net_faction_box.Flags = GBxFlg_Unkn0001;
    net_comms_box.Flags = GBxFlg_Unkn0001;
    net_grpaint.Flags = GBxFlg_Unkn0001;
    net_protocol_box.Flags = GBxFlg_Unkn0001;
}

void reset_net_screen_EJECT_flags(void)
{
    unkn8_EJECT_button.Flags = GBxFlg_Unkn0001;
}

void set_flag01_net_screen_boxes(void)
{
    net_SET2_button.Flags |= GBxFlg_Unkn0001;
    net_SET_button.Flags |= GBxFlg_Unkn0001;
    net_INITIATE_button.Flags |= GBxFlg_Unkn0001;
    net_protocol_option_button.Flags |= GBxFlg_Unkn0001;
    net_protocol_select_button.Flags |= GBxFlg_Unkn0001;
    net_unkn40_button.Flags |= GBxFlg_Unkn0001;
    unkn8_EJECT_button.Flags |= GBxFlg_Unkn0001;
    net_groups_LOGON_button.Flags |= GBxFlg_Unkn0001;
}

void skip_flashy_draw_net_screen_boxes(void)
{
    net_INITIATE_button.Flags |= GBxFlg_Unkn0002;
    net_groups_LOGON_button.Flags |= GBxFlg_Unkn0002;
    net_SET_button.Flags |= GBxFlg_Unkn0002;
    net_SET2_button.Flags |= GBxFlg_Unkn0002;
    net_groups_box.Flags |= GBxFlg_Unkn0002;
    net_users_box.Flags |= GBxFlg_Unkn0002;
    net_faction_box.Flags |= GBxFlg_Unkn0002;
    net_team_box.Flags |= GBxFlg_Unkn0002;
    net_grpaint.Flags |= GBxFlg_Unkn0002;
    net_benefits_box.Flags |= GBxFlg_Unkn0002;
    net_protocol_box.Flags |= GBxFlg_Unkn0002;
    net_protocol_select_button.Flags |= GBxFlg_Unkn0002;
    net_unkn40_button.Flags |= GBxFlg_Unkn0002;
    net_protocol_option_button.Flags |= GBxFlg_Unkn0002;
    net_comms_box.Flags |= GBxFlg_Unkn0002;
}

void switch_net_screen_boxes_to_initiate(void)
{
    net_INITIATE_button.Flags = GBxFlg_Unkn0001;
    net_INITIATE_button.Text = gui_strings[385];
    net_groups_LOGON_button.Text = gui_strings[386];
}

void switch_net_screen_boxes_to_execute(void)
{
    const char *text;

    net_INITIATE_button.Text = gui_strings[387];
    if (byte_1C4A6F)
        text = gui_strings[520];
    else
        text = gui_strings[388];
    net_groups_LOGON_button.Text = text;
}

/******************************************************************************/
