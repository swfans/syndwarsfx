/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bfjoyst.h
 *     Header file for bflib_joyst.c.
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

struct DevInput {
        int32_t Yaw[16];
        int32_t Roll[16];
        int32_t Pitch[16];
        int32_t AnalogueX[16];
        int32_t AnalogueY[16];
        int32_t AnalogueZ[16];
        int32_t AnalogueU[16];
        int32_t AnalogueV[16];
        int32_t AnalogueR[16];
        int32_t DigitalX[16];
        int32_t DigitalY[16];
        int32_t DigitalZ[16];
        int32_t DigitalU[16];
        int32_t DigitalV[16];
        int32_t DigitalR[16];
        int32_t MinXAxis[16];
        int32_t MinYAxis[16];
        int32_t MinZAxis[16];
        int32_t MinUAxis[16];
        int32_t MinVAxis[16];
        int32_t MinRAxis[16];
        int32_t MaxXAxis[16];
        int32_t MaxYAxis[16];
        int32_t MaxZAxis[16];
        int32_t MaxUAxis[16];
        int32_t MaxVAxis[16];
        int32_t MaxRAxis[16];
        int32_t XCentre[16];
        int32_t YCentre[16];
        int32_t ZCentre[16];
        int32_t UCentre[16];
        int32_t VCentre[16];
        int32_t RCentre[16];
        int32_t HatX[16];
        int32_t HatY[16];
        int32_t HatMax[16];
        int32_t Buttons[16];
        int32_t NumberOfButtons[16];
        int32_t ConfigType[16];
        int32_t MenuButtons[16];
        int32_t Type;
        int32_t NumberOfDevices;
        int32_t DeviceType[16];
        uint8_t Init[16];
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
