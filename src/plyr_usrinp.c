/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file plyr_usrinp.c
 *     Players User Input handling.
 * @par Purpose:
 *     Implement functions for storing user input and building packets
 *     which will later influence the game world.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Dec 2024 - 01 Feb 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "plyr_usrinp.h"

#include <assert.h>
#include "bfkeybd.h"
#include "bfjoyst.h"
#include "ssampply.h"

#include "display.h"
#include "engintrns.h"
#include "game.h"
#include "game_options.h"
#include "game_speed.h"
#include "keyboard.h"
#include "packet.h"
#include "player.h"
#include "sound.h"
#include "swlog.h"
#include "thing.h"
#include "vehicle.h"
/******************************************************************************/

short get_agent_move_direction_delta_x(const struct SpecialUserInput *p_usrinp)
{
    return (sbyte)(p_usrinp->Bits >> 0);
}

void set_agent_move_direction_delta_x(struct SpecialUserInput *p_usrinp, short dt)
{
    if (dt > 127)
        dt = 127;
    if (dt < -127)
        dt = -127;
    //p_usrinp->Bits &= ~(0xFF << 0); -- no need, should be always clear before set
    p_usrinp->Bits |= (((ubyte)dt) << 0);
}

short get_agent_move_direction_delta_z(const struct SpecialUserInput *p_usrinp)
{
    return (sbyte)(p_usrinp->Bits >> 8);
}

void set_agent_move_direction_delta_z(struct SpecialUserInput *p_usrinp, short dt)
{
    if (dt > 127)
        dt = 127;
    if (dt < -127)
        dt = -127;
    //p_usrinp->Bits &= ~(0xFF << 8); -- no need, should be always clear before set
    p_usrinp->Bits |= (((ubyte)dt) << 8);
}

void do_user_input_bits_direction_clear(struct SpecialUserInput *p_usrinp)
{
    p_usrinp->Bits &= ~(0xFF << 0);
    p_usrinp->Bits &= ~(0xFF << 8);
}

void do_user_input_bits_control_clear_all(struct SpecialUserInput *p_usrinp)
{
    p_usrinp->Bits &= ~SpUIn_AllControlBits;
}

void do_user_input_bits_control_clear_nonmove(struct SpecialUserInput *p_usrinp)
{
    p_usrinp->Bits &= ~SpUIn_AllNonMoveBits;
}

void do_user_input_bits_direction_from_kbd(struct SpecialUserInput *p_usrinp)
{
    short dt;

    if (get_agent_move_direction_delta_x(p_usrinp) == 0) {
        dt = (is_gamekey_kbd_pressed(GKey_RIGHT) & 1) - (is_gamekey_kbd_pressed(GKey_LEFT) & 1);
        set_agent_move_direction_delta_x(p_usrinp, dt);
    }
    if (get_agent_move_direction_delta_z(p_usrinp) == 0) {
        dt = (is_gamekey_kbd_pressed(GKey_UP) & 1) - (is_gamekey_kbd_pressed(GKey_DOWN) & 1);
        set_agent_move_direction_delta_z(p_usrinp, dt);
    }
}

void do_user_input_bits_direction_from_joy(struct SpecialUserInput *p_usrinp, ubyte channel)
{
    short dt;

    if (get_agent_move_direction_delta_x(p_usrinp) == 0) {
        dt = joy.DigitalX[channel];
        set_agent_move_direction_delta_x(p_usrinp, dt);
    }
    if (get_agent_move_direction_delta_z(p_usrinp) == 0) {
        dt = joy.DigitalY[channel];
        set_agent_move_direction_delta_z(p_usrinp, dt);
    }
}

/** Input function for a user controlling the cyborgs via keyboard only.
 */
void do_user_input_bits_actions_from_kbd(struct SpecialUserInput *p_usrinp)
{
    if (is_gamekey_kbd_pressed(GKey_FIRE)) {
        p_usrinp->Bits |= SpUIn_DoTrigger;
    }
    if (is_gamekey_kbd_pressed(GKey_CHANGE_MD_WP)) {
        p_usrinp->Bits |= SpUIn_ChangeMoodOrWep;
    }
    if (is_gamekey_kbd_pressed(GKey_CHANGE_AGENT)) {
        p_usrinp->Bits |= SpUIn_ChangeAgent;
    }
    if (is_gamekey_kbd_pressed(GKey_DROP_WEAPON)) {
        clear_gamekey_kbd_pressed(GKey_DROP_WEAPON);
        p_usrinp->Bits |= SpUIn_DoDropOrGoOut;
    }
    if (is_gamekey_kbd_pressed(GKey_SELF_DESTRUCT)) {
        clear_gamekey_kbd_pressed(GKey_SELF_DESTRUCT);
        p_usrinp->Bits |= SpUIn_SelfDestruct;
    }
}

/** Input function for an extra user controlling the cyborgs via joystick only.
 */
void do_user_input_bits_actions_from_joy(struct SpecialUserInput *p_usrinp, ubyte channel)
{
    if (is_gamekey_joy_pressed(GKey_FIRE, channel)) {
        p_usrinp->Bits |= SpUIn_DoTrigger;
    }
    if (is_gamekey_joy_pressed(GKey_CHANGE_MD_WP, channel)) {
        p_usrinp->Bits |= SpUIn_ChangeMoodOrWep;
    }
    if (is_gamekey_joy_pressed(GKey_DROP_WEAPON, channel)) {
        clear_gamekey_joy_pressed(GKey_DROP_WEAPON, channel);
        p_usrinp->Bits |= SpUIn_DoDropOrGoOut;
    }
    if (is_gamekey_joy_pressed(GKey_SELF_DESTRUCT, channel)) {
        clear_gamekey_joy_pressed(GKey_SELF_DESTRUCT, channel);
        p_usrinp->Bits |= SpUIn_SelfDestruct;
    }
}

ubyte do_user_input_bits_actions_from_joy_and_kbd(struct SpecialUserInput *p_usrinp)
{
    ubyte did_inp;

    did_inp = GINPUT_NONE;
    if (is_gamekey_pressed(GKey_FIRE)) {
        p_usrinp->Bits |= SpUIn_DoTrigger;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_CHANGE_MD_WP)) {
        p_usrinp->Bits |= SpUIn_ChangeMoodOrWep;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_CHANGE_AGENT)) {
        p_usrinp->Bits |= SpUIn_ChangeAgent;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_GOTO_POINT)) {
        clear_gamekey_pressed(GKey_GOTO_POINT);
        p_usrinp->Bits |= SpUIn_GotoPoint;
        did_inp |= GINPUT_DIRECT;
    }
    // TODO remove hard-coded BACKSLASH and make sure GKey_GROUP works for all keyboard layouts
    if (is_key_pressed(KC_BACKSLASH, KMod_DONTCARE)) {
        clear_key_pressed(KC_BACKSLASH);
        p_usrinp->Bits |= SpUIn_GroupingInc;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_GROUP)) {
        clear_gamekey_pressed(GKey_GROUP);
        p_usrinp->Bits |= SpUIn_GroupingInc;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_DROP_WEAPON)) {
        clear_gamekey_pressed(GKey_DROP_WEAPON);
        p_usrinp->Bits |= SpUIn_DoDropOrGoOut;
        did_inp |= GINPUT_DIRECT;
    }
    if (is_gamekey_pressed(GKey_SELF_DESTRUCT)) {
        clear_gamekey_pressed(GKey_SELF_DESTRUCT);
        p_usrinp->Bits |= SpUIn_SelfDestruct;
        did_inp |= GINPUT_DIRECT;
    }
    return did_inp;
}

void update_agent_move_direction_deltas(struct SpecialUserInput *p_usrinp)
{
    short ax1, ax2;
    ushort delta;

    ax2 = get_agent_move_direction_delta_z(p_usrinp);
    ax1 = get_agent_move_direction_delta_x(p_usrinp);
    delta = 4 * (ax2 + 1) + (ax1 + 1);

    p_usrinp->DtZ = delta;
    if ((p_usrinp->DtX == delta) && ((gameturn & 0x7FFF) - p_usrinp->Turn < 7)) {
        p_usrinp->Turn = 0;
        p_usrinp->Bits |= SpUIn_DoActionFast;
    }
    if (ax1 || ax2) {
        p_usrinp->Turn = 0;
        p_usrinp->DtX = delta;
    } else {
        if (p_usrinp->Turn == 0)
            p_usrinp->Turn = gameturn & 0x7FFF;
        p_usrinp->Bits &= ~SpUIn_DoActionFast;
    }
}

short get_next_player_agent(ushort player)
{
    short ret;
    asm volatile ("call ASM_get_next_player_agent\n"
        : "=r" (ret) : "a" (player));
    return ret;
}

ubyte input_user_control_agent(ushort plyr, short dmuser)
{
    PlayerInfo *p_player;
    struct Packet *p_pckt;
    void (*loc_build_packet)(struct Packet *, ushort, ulong, long, long, long);
    struct Thing *p_dcthing;
    int dx, dy, dz;
    ThingIdx dcthing;

    p_player = &players[plyr];
    p_pckt = &packets[plyr];

    switch (dmuser)
    {
    case 0:
        loc_build_packet = build_packet;
        break;
    case 1:
        loc_build_packet = build_packet2;
        break;
    case 2:
        loc_build_packet = build_packet4;
        break;
    default:
        loc_build_packet = build_packet4;
        break;
    }

    dcthing = p_player->DirectControl[dmuser];

    if ((dcthing == 0) || (lbShift == KMod_SHIFT))
    {
        loc_build_packet(p_pckt, 0, dcthing, 0, 0, 0);
        return GINPUT_PACKET;
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_SelfDestruct) != 0)
    {
        p_player->UserInput[dmuser].Bits &= ~SpUIn_SelfDestruct;
        loc_build_packet(p_pckt, 255, dcthing, 0, 0, 0);
        return GINPUT_PACKET;
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_DoDropOrGoOut) != 0)
    {
        p_dcthing = &things[dcthing];
        if ((p_dcthing->Flag & TngF_InVehicle) != 0)
        {
            p_player->UserInput[dmuser].Bits &= ~SpUIn_DoDropOrGoOut;
            loc_build_packet(p_pckt, PAct_LEAVE_VEHICLE, dcthing,
              p_dcthing->U.UPerson.Vehicle, 0, 0);
            return GINPUT_PACKET;
        }
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_DoDropOrGoOut) != 0)
    {
        loc_build_packet(p_pckt, PAct_DROP_SELC_WEAPON_SECR, dcthing, 0, 0, 0);
        return GINPUT_PACKET;
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_GroupingInc) != 0)
    {
        int i, nagents;
        nagents = 0;
        for (i = 0; i < playable_agents; i++)
        {
            struct Thing *p_agent;
            p_agent = p_player->MyAgent[i];
            if ((p_agent->State == PerSt_PROTECT_PERSON) || (p_agent->State == PerSt_DEAD))
                nagents++;
        }
        if (nagents == playable_agents - 1) {
            loc_build_packet(p_pckt, PAct_PROTECT_TOGGLE, dcthing, 0, 0,  0);
        } else {
            loc_build_packet(p_pckt, PAct_PROTECT_INC, dcthing, 0, 0, 0);
        }
        return GINPUT_PACKET;
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_DoTrigger) != 0)
    {
        p_dcthing = &things[dcthing];
        if ((p_dcthing->Flag & TngF_Unkn08000000) != 0)
        {
            p_player->UserInput[dmuser].Bits &= ~SpUIn_DoTrigger;
            loc_build_packet(p_pckt, PAct_PICKUP, dcthing,
              p_dcthing->U.UPerson.StandOnThing, 0, 0);
            return GINPUT_PACKET;
        }
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_DoTrigger) != 0)
    {
        p_dcthing = &things[dcthing];
        if (!person_is_in_a_vehicle(p_dcthing) && person_is_standing_on_vehicle(p_dcthing))
        {
            struct Thing *p_vehicle;
            p_vehicle = &things[p_dcthing->U.UPerson.StandOnThing];

            if (can_i_enter_vehicle(p_dcthing, p_vehicle))
            {
                p_player->UserInput[dmuser].Bits &= ~SpUIn_DoTrigger;
                loc_build_packet(p_pckt, PAct_ENTER_VEHICLE, dcthing,
                  p_dcthing->U.UPerson.StandOnThing, 0, 0);
                return GINPUT_PACKET;
            }
            else
            {
                if ((debug_log_things & 0x01) != 0) {
                    LOGSYNC("Person %s %d state %d.%d cannot enter %s %d state %d.%d",
                      person_type_name(p_dcthing->SubType), (int)p_dcthing->ThingOffset,
                      p_dcthing->State, p_dcthing->SubState,
                      vehicle_type_name(p_vehicle->SubType), (int)p_vehicle->ThingOffset,
                      p_vehicle->State, p_vehicle->SubState);
                }
            }
        }
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_GotoPoint) != 0 || (p_player->State[dmuser] == 1))
    {
        if (process_send_person(plyr, dmuser))
        {
            loc_build_packet(p_pckt, PAct_AGENT_GOTO_GND_PT_ABS, dcthing, engn_xc, 0, engn_zc);
            return GINPUT_PACKET;
        }
        return GINPUT_NONE;
    }

    if ((p_player->UserInput[dmuser].Bits & SpUIn_ChangeAgent) != 0)
    {
        if (p_player->PrevState[dmuser] != PAct_SELECT_AGENT)
        {
            short next_player;
            next_player = get_next_player_agent(plyr);
            loc_build_packet(p_pckt, PAct_SELECT_AGENT, dcthing, next_player, 0, 0);
            return GINPUT_PACKET;
        }
        return GINPUT_NONE;
    }

    dy = 0;
    dx = get_agent_move_direction_delta_x(&p_player->UserInput[dmuser]) * 256;
    dz = get_agent_move_direction_delta_z(&p_player->UserInput[dmuser]) * 256;

    if ((p_player->UserInput[dmuser].Bits & SpUIn_ChangeMoodOrWep) != 0)
    {
        p_dcthing = &things[dcthing];
        if (dx > 0)
        {
            short val;
            val = p_dcthing->U.UPerson.Mood + 10;
            if (val > 100)
                val = 100;
            loc_build_packet(p_pckt, PAct_AGENT_SET_MOOD, dcthing, val, 0, 0);
            if (!IsSamplePlaying(0, 21, 0))
                play_sample_using_heap(0, 21, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 1);
            ingame.Flags |= GamF_Unkn00100000;
            return GINPUT_PACKET;
        }

        if (dx < 0)
        {
            short val;
            val = p_dcthing->U.UPerson.Mood - 10;
            if (val < -100)
                val = -100;
            loc_build_packet(p_pckt, PAct_AGENT_SET_MOOD, dcthing, val, 0, 0);
            if (!IsSamplePlaying(0, 21, 0))
                play_sample_using_heap(0, 21, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_4EVER, 1);
            ingame.Flags |= GamF_Unkn00100000;
            return GINPUT_PACKET;
        }

        if (dz < 0)
        {
            if (p_player->PrevState[dmuser] != PAct_SELECT_NEXT_WEAPON)
            {
                loc_build_packet(p_pckt, PAct_SELECT_NEXT_WEAPON, dcthing, 0, 0, 0);
                return GINPUT_PACKET;
            }
            return GINPUT_NONE;
        }

        if (dz > 0)
        {
            if (p_player->PrevState[dmuser] != PAct_SELECT_PREV_WEAPON)
            {
                loc_build_packet(p_pckt, PAct_SELECT_PREV_WEAPON, dcthing, 0, 0, 0);
                return GINPUT_PACKET;
            }
            return GINPUT_NONE;
        }

        p_player->PrevState[dmuser] = PAct_NONE;
        return GINPUT_DIRECT;
    }

    if (dx == 0 && dz == 0)
    {
        ushort flg;
        if ((p_player->UserInput[dmuser].Bits & SpUIn_DoTrigger) != 0)
            flg = 0x8000;
        else
            flg = 0x0;
        loc_build_packet(p_pckt, PAct_NONE | flg, dcthing, dx, dy, dz);
        return GINPUT_PACKET;
    }

    local_to_worldr(&dx, &dy, &dz);

    if ((p_player->UserInput[dmuser].Bits & SpUIn_DoActionFast) != 0)
    {
        ushort flg;
        if ((p_player->UserInput[dmuser].Bits & SpUIn_DoTrigger) != 0)
            flg = 0x8000;
        else
            flg = 0x0;
        loc_build_packet(p_pckt, PAct_AGENT_GOTO_GND_PT_REL_FF | flg, dcthing, dx, dy, dz);
        return GINPUT_PACKET;
    }
    else
    {
        ushort flg;
        if ((p_player->UserInput[dmuser].Bits & SpUIn_DoTrigger) != 0)
            flg = 0x8000;
        else
            flg = 0x0;
        loc_build_packet(p_pckt, PAct_AGENT_GOTO_GND_PT_REL | flg, dcthing, dx, dy, dz);
        return GINPUT_PACKET;
    }
    return GINPUT_NONE;
}

/******************************************************************************/
