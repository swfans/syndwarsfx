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
        int32_t Yaw[MAX_JOYSTICK_COUNT];
        int32_t Roll[MAX_JOYSTICK_COUNT];
        int32_t Pitch[MAX_JOYSTICK_COUNT];
        int32_t AnalogueX[MAX_JOYSTICK_COUNT];
        int32_t AnalogueY[MAX_JOYSTICK_COUNT];
        int32_t AnalogueZ[MAX_JOYSTICK_COUNT];
        int32_t AnalogueU[MAX_JOYSTICK_COUNT];
        int32_t AnalogueV[MAX_JOYSTICK_COUNT];
        int32_t AnalogueR[MAX_JOYSTICK_COUNT];
        int32_t DigitalX[MAX_JOYSTICK_COUNT];
        int32_t DigitalY[MAX_JOYSTICK_COUNT];
        int32_t DigitalZ[MAX_JOYSTICK_COUNT];
        int32_t DigitalU[MAX_JOYSTICK_COUNT];
        int32_t DigitalV[MAX_JOYSTICK_COUNT];
        int32_t DigitalR[MAX_JOYSTICK_COUNT];
        int32_t MinXAxis[MAX_JOYSTICK_COUNT];
        int32_t MinYAxis[MAX_JOYSTICK_COUNT];
        int32_t MinZAxis[MAX_JOYSTICK_COUNT];
        int32_t MinUAxis[MAX_JOYSTICK_COUNT];
        int32_t MinVAxis[MAX_JOYSTICK_COUNT];
        int32_t MinRAxis[MAX_JOYSTICK_COUNT];
        int32_t MaxXAxis[MAX_JOYSTICK_COUNT];
        int32_t MaxYAxis[MAX_JOYSTICK_COUNT];
        int32_t MaxZAxis[MAX_JOYSTICK_COUNT];
        int32_t MaxUAxis[MAX_JOYSTICK_COUNT];
        int32_t MaxVAxis[MAX_JOYSTICK_COUNT];
        int32_t MaxRAxis[MAX_JOYSTICK_COUNT];
        int32_t XCentre[MAX_JOYSTICK_COUNT];
        int32_t YCentre[MAX_JOYSTICK_COUNT];
        int32_t ZCentre[MAX_JOYSTICK_COUNT];
        int32_t UCentre[MAX_JOYSTICK_COUNT];
        int32_t VCentre[MAX_JOYSTICK_COUNT];
        int32_t RCentre[MAX_JOYSTICK_COUNT];
        int32_t HatX[MAX_JOYSTICK_COUNT];
        int32_t HatY[MAX_JOYSTICK_COUNT];
        int32_t HatMax[MAX_JOYSTICK_COUNT];
        int32_t Buttons[MAX_JOYSTICK_COUNT];
        int32_t NumberOfButtons[MAX_JOYSTICK_COUNT];
        int32_t ConfigType[MAX_JOYSTICK_COUNT];
        int32_t MenuButtons[MAX_JOYSTICK_COUNT];
        int32_t Type;
        int32_t NumberOfDevices;
        int32_t DeviceType[MAX_JOYSTICK_COUNT];
        uint8_t Init[MAX_JOYSTICK_COUNT];
};

#pragma pack()
/******************************************************************************/
extern struct DevInput joy;

int JoySetInterrupt(short val);
int joy_get_device_name(char *textbuf);
int joy_update_inputs(struct DevInput *dinp);
int joy_refresh_devices(struct DevInput *dinp);
int joy_setup_device(struct DevInput *dinp, int jtype);
int joy_driver_init(struct DevInput *dinp);
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
    CONTROLLER_BUTTON_SELECT,
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
    
    /*
    paddle support would require changing short to int32_t for jskeys in gamecode
    CONTROLLER_BUTTON_PADDLE1,  // Xbox Elite paddle P1 (upper left, facing the back)
    CONTROLLER_BUTTON_PADDLE2,  // Xbox Elite paddle P3 (upper right, facing the back)
    CONTROLLER_BUTTON_PADDLE3,  // Xbox Elite paddle P2 (lower left, facing the back)
    CONTROLLER_BUTTON_PADDLE4,  // Xbox Elite paddle P4 (lower right, facing the back)
    CONTROLLER_BUTTON_TOUCHPAD, // PS4/PS5 touchpad button
    */
    CONTROLLER_BUTTON_MAX
} GameControllerButton;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
