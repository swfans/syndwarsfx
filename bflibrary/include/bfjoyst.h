/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bfjoyst.h
 *     Header file for sjoyst.c.
 * @par Purpose:
 *     Joystick support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Feb 2010 - 11 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_JOYST_H
#define BFLIB_JOYST_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

 #define MAX_JOYSTICK_COUNT 16

struct DevInput {
        long Yaw[MAX_JOYSTICK_COUNT];
        long Roll[MAX_JOYSTICK_COUNT];
        long Pitch[MAX_JOYSTICK_COUNT];
        long AnalogueX[MAX_JOYSTICK_COUNT];
        long AnalogueY[MAX_JOYSTICK_COUNT];
        long AnalogueZ[MAX_JOYSTICK_COUNT];
        long AnalogueU[MAX_JOYSTICK_COUNT];
        long AnalogueV[MAX_JOYSTICK_COUNT];
        long AnalogueR[MAX_JOYSTICK_COUNT];
        long DigitalX[MAX_JOYSTICK_COUNT];
        long DigitalY[MAX_JOYSTICK_COUNT];
        long DigitalZ[MAX_JOYSTICK_COUNT];
        long DigitalU[MAX_JOYSTICK_COUNT];
        long DigitalV[MAX_JOYSTICK_COUNT];
        long DigitalR[MAX_JOYSTICK_COUNT];
        long MinXAxis[MAX_JOYSTICK_COUNT];
        long MinYAxis[MAX_JOYSTICK_COUNT];
        long MinZAxis[MAX_JOYSTICK_COUNT];
        long MinUAxis[MAX_JOYSTICK_COUNT];
        long MinVAxis[MAX_JOYSTICK_COUNT];
        long MinRAxis[MAX_JOYSTICK_COUNT];
        long MaxXAxis[MAX_JOYSTICK_COUNT];
        long MaxYAxis[MAX_JOYSTICK_COUNT];
        long MaxZAxis[MAX_JOYSTICK_COUNT];
        long MaxUAxis[MAX_JOYSTICK_COUNT];
        long MaxVAxis[MAX_JOYSTICK_COUNT];
        long MaxRAxis[MAX_JOYSTICK_COUNT];
        long XCentre[MAX_JOYSTICK_COUNT];
        long YCentre[MAX_JOYSTICK_COUNT];
        long ZCentre[MAX_JOYSTICK_COUNT];
        long UCentre[MAX_JOYSTICK_COUNT];
        long VCentre[MAX_JOYSTICK_COUNT];
        long RCentre[MAX_JOYSTICK_COUNT];
        long HatX[MAX_JOYSTICK_COUNT];
        long HatY[MAX_JOYSTICK_COUNT];
        long HatMax[MAX_JOYSTICK_COUNT];
        unsigned long Buttons[MAX_JOYSTICK_COUNT];
        long NumberOfButtons[MAX_JOYSTICK_COUNT];
        long ConfigType[MAX_JOYSTICK_COUNT];
        long MenuButtons[MAX_JOYSTICK_COUNT];
        long Type;
        long NumberOfDevices;
        long DeviceType[MAX_JOYSTICK_COUNT];
        unsigned char Init[MAX_JOYSTICK_COUNT];
};

#pragma pack()
/******************************************************************************/
extern struct DevInput joy;

int JoySetInterrupt(short val);
int joy_get_device_name(char *textbuf);
int joy_update_inputs(struct DevInput *dinp);
int joy_refresh_devices(struct DevInput *dinp);
int joy_setup_device(struct DevInput *dinp, int jtype);
int joy_driver_init(void);
int joy_driver_shutdown(void);
const char* joy_get_button_label(int button);


typedef enum
{
    CONTROLLER_BUTTON_INVALID = -1,
    CONTROLLER_BUTTON_A,
    CONTROLLER_BUTTON_B,
    CONTROLLER_BUTTON_X,
    CONTROLLER_BUTTON_Y,
    CONTROLLER_BUTTON_BACK,
    CONTROLLER_BUTTON_GUIDE,
    CONTROLLER_BUTTON_START,
    CONTROLLER_BUTTON_LEFTSTICK,
    CONTROLLER_BUTTON_RIGHTSTICK,
    CONTROLLER_BUTTON_LEFTSHOULDER,
    CONTROLLER_BUTTON_RIGHTSHOULDER,
    CONTROLLER_BUTTON_DPAD_UP,
    CONTROLLER_BUTTON_DPAD_DOWN,
    CONTROLLER_BUTTON_DPAD_LEFT,
    CONTROLLER_BUTTON_DPAD_RIGHT,
    CONTROLLER_BUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    CONTROLLER_BUTTON_PADDLE1,  // Xbox Elite paddle P1 (upper left, facing the back)
    CONTROLLER_BUTTON_PADDLE2,  // Xbox Elite paddle P3 (upper right, facing the back)
    CONTROLLER_BUTTON_PADDLE3,  // Xbox Elite paddle P2 (lower left, facing the back)
    CONTROLLER_BUTTON_PADDLE4,  // Xbox Elite paddle P4 (lower right, facing the back)
    CONTROLLER_BUTTON_TOUCHPAD, // PS4/PS5 touchpad button
    CONTROLLER_BUTTON_TRIGGER_LEFT,
    CONTROLLER_BUTTON_TRIGGER_RIGHT,
    CONTROLLER_BUTTON_RIGHT_THUMB_LEFT,
    CONTROLLER_BUTTON_RIGHT_THUMB_RIGHT,
    CONTROLLER_BUTTON_RIGHT_THUMB_UP,
    CONTROLLER_BUTTON_RIGHT_THUMB_DOWN,
    CONTROLLER_BUTTON_MAX,
} GameControllerButton;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
