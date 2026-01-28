/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file plyr_usrinp.h
 *     Header file for plyr_usrinp.c.
 * @par Purpose:
 *     Players User Input handling.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Dec 2024 - 01 Feb 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef PLYR_USRINP_H
#define PLYR_USRINP_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum SpecialUserInputBits {
    /** Trigger pressed; this can do multiple actions: weapon fire, pickup,
     *  entering vehicle or move to point. */
    SpUIn_DoTrigger        = 0x00010000,
    /** A mode where X axis changes mood, Y axis changes weapon. */
    SpUIn_ChangeMoodOrWep  = 0x00020000,
    SpUIn_Unkn00040000     = 0x00040000,
    SpUIn_Unkn00080000     = 0x00080000,
    /** Switch control to next agent. */
    SpUIn_ChangeAgent      = 0x00100000,
    SpUIn_Unkn00200000     = 0x00200000,
    SpUIn_GotoPoint        = 0x00400000,
    /** Increase agents grouping, or dissolve group if at max. */
    SpUIn_GroupingInc      = 0x00800000,
    SpUIn_Unkn01000000     = 0x01000000,
    SpUIn_Unkn02000000     = 0x02000000,
    SpUIn_Unkn04000000     = 0x04000000,
    SpUIn_Unkn08000000     = 0x08000000,
    SpUIn_Unkn10000000     = 0x10000000,
    /** Activate agent self-destruct sequence. */
    SpUIn_SelfDestruct     = 0x20000000,
    /** Drop weapon, or go out of a vehicle. */
    SpUIn_DoDropOrGoOut    = 0x40000000,
    /** Use fast version of the next action (ie. run to target). */
    SpUIn_DoActionFast     = 0x80000000,
    SpUIn_AllNonMoveBits   = 0x7FFF0000,
    SpUIn_AllControlBits   = 0xFFFF0000,
};

enum SpecialUserInputControlModes {
    UInpCtr_Keyboard = 0,
    UInpCtr_Mouse,
    UInpCtr_Joystick0,
    UInpCtr_Joystick1,
    UInpCtr_Joystick2,
    UInpCtr_Joystick3,
    UInpCtr_MODES_COUNT,
    UInpCtrF_Unkn2000 = 0x2000,
    UInpCtrF_Unkn4000 = 0x4000,
    UInpCtrF_Unkn8000 = 0x8000,
};

#define UInpCtr_AllFlagsMask (UInpCtrF_Unkn2000|UInpCtrF_Unkn4000|UInpCtrF_Unkn8000)

/** Per-local-player input mapping struct.
 *
 * The game allows multiple people to play locally, using different input
 * devices. This struct allows inputs from each such person to be translated
 * into universal format, and then used to influence the game world.
 */
struct SpecialUserInput
{
  ulong Bits;
  sbyte DtX;
  sbyte DtZ;
  ushort Turn; /* from do_user_interface(), this seem to be game turn of last action */
  ubyte LastDir; /* probably wrong place */
  ubyte CurDir; /* probably wrong place */
  ushort MapX[3];
  ushort MapY[3];
  ushort MapZ[3];
  ushort OnFace;
  ushort ControlMode;
  ushort MapIndex;
};

#pragma pack()
/******************************************************************************/
void do_user_input_bits_direction_clear(struct SpecialUserInput *p_usrinp);
void do_user_input_bits_control_clear_all(struct SpecialUserInput *p_usrinp);
void do_user_input_bits_control_clear_nonmove(struct SpecialUserInput *p_usrinp);
void do_user_input_bits_direction_from_kbd(struct SpecialUserInput *p_usrinp);
void do_user_input_bits_direction_from_joy(struct SpecialUserInput *p_usrinp, ubyte channel);
short get_agent_move_direction_delta_x(const struct SpecialUserInput *p_usrinp);
short get_agent_move_direction_delta_z(const struct SpecialUserInput *p_usrinp);
void update_agent_move_direction_deltas(struct SpecialUserInput *p_usrinp);

/** Clear state of user inputs to allow for clean missison start.
 */
void reset_user_input(void);

/** Initialize selected control scheme for local users.
 *
 * Finds and setups mouse user, resets controls state for mission start.
 */
void init_user_input_local_controls(void);

void do_user_input_bits_actions_from_kbd(struct SpecialUserInput *p_usrinp);
void do_user_input_bits_actions_from_joy(struct SpecialUserInput *p_usrinp, ubyte channel);

/** Input function for a single user controlling the cyborgs via both keyboard and one joystick.
 */
ubyte do_user_input_bits_actions_from_joy_and_kbd(struct SpecialUserInput *p_usrinp);

ubyte input_user_control_agent(ushort plyr, short dmuser);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
