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
        long Yaw[16];
        long Roll[16];
        long Pitch[16];
        long AnalogueX[16];
        long AnalogueY[16];
        long AnalogueZ[16];
        long AnalogueU[16];
        long AnalogueV[16];
        long AnalogueR[16];
        long DigitalX[16];
        long DigitalY[16];
        long DigitalZ[16];
        long DigitalU[16];
        long DigitalV[16];
        long DigitalR[16];
        long MinXAxis[16];
        long MinYAxis[16];
        long MinZAxis[16];
        long MinUAxis[16];
        long MinVAxis[16];
        long MinRAxis[16];
        long MaxXAxis[16];
        long MaxYAxis[16];
        long MaxZAxis[16];
        long MaxUAxis[16];
        long MaxVAxis[16];
        long MaxRAxis[16];
        long XCentre[16];
        long YCentre[16];
        long ZCentre[16];
        long UCentre[16];
        long VCentre[16];
        long RCentre[16];
        long HatX[16];
        long HatY[16];
        long HatMax[16];
        unsigned long Buttons[16];
        long NumberOfButtons[16];
        long ConfigType[16];
        long MenuButtons[16];
        long Type;
        long NumberOfDevices;
        long DeviceType[16];
        unsigned char Init[16];
};

#pragma pack()
/******************************************************************************/
extern struct DevInput joy;

int JoySetInterrupt(short val);
int joy_get_device_name(char *textbuf);
int joy_update_inputs(struct DevInput *dinp);
int joy_refresh_devices(struct DevInput *dinp);
int joy_setup_device(struct DevInput *dinp, int jtype);
int joy_driver_init();
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
