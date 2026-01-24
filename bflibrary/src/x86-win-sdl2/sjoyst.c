/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_joyst.c
 *     Joystick support.
 * @par Purpose:
 *     Implement Joystick input support for games.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Feb 2010 - 11 Oct 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bfjoyst.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "privbflog.h"

#include <SDL.h>

/******************************************************************************/
// SDL2 joystick state
static SDL_Joystick *sdl_joysticks[16] = {NULL};
static int sdl_num_joysticks = 0;
/******************************************************************************/

int JoySetInterrupt(short val)
{
    // SDL2 does not use hardware interrupts for joystick input
    return -1;
}

int joy_get_device_name(char *textbuf)
{
    const char *name = sdl_joysticks[0] ? SDL_JoystickName(sdl_joysticks[0]) : "No Joystick";
    strncpy(textbuf, name, 52);
    textbuf[51] = '\0';
    return Lb_SUCCESS;
}

int joy_update_inputs(struct DevInput *dinp)
{
    if (dinp->Type == -1)
        return -1;
    
    // Clear all device states
    for (int i = 0; i < 16; i++)
    {
        dinp->HatMax[i] = 0;
        dinp->HatY[i] = 0;
        dinp->HatX[i] = 0;
        dinp->AnalogueZ[i] = 0;
        dinp->AnalogueY[i] = 0;
        dinp->AnalogueX[i] = 0;
        dinp->DigitalZ[i] = 0;
        dinp->DigitalY[i] = 0;
        dinp->DigitalX[i] = 0;
        dinp->AnalogueR[i] = 0;
        dinp->AnalogueV[i] = 0;
        dinp->AnalogueU[i] = 0;
        dinp->DigitalR[i] = 0;
        dinp->DigitalV[i] = 0;
        dinp->DigitalU[i] = 0;
    }
    
    // Update joystick events
    //SDL_JoystickUpdate();
    
    // Read state from all opened joysticks
    for (int i = 0; i < sdl_num_joysticks; i++)
    {
        SDL_Joystick *joy = sdl_joysticks[i];
        if (!joy || !dinp->Init[i])
            continue;
        
        // Read analog axes
        int num_axes = SDL_JoystickNumAxes(joy);
        if (num_axes > 16)
            num_axes = 16;
        for (int axis = 0; axis < num_axes; axis++) {
            dinp->AnalogueX[i] = SDL_JoystickGetAxis(joy, axis);
            dinp->DigitalX[i] = (dinp->AnalogueX[i] < -16384) ? -1 : (dinp->AnalogueX[i] > 16384) ? 1 : 0;
        }
        
        // Read hat (D-pad)
        int num_hats = SDL_JoystickNumHats(joy);
        if (num_hats > 0) {
            Uint8 hat_state = SDL_JoystickGetHat(joy, 0);
            dinp->HatX[i] = 0;
            dinp->HatY[i] = 0;
            
            if (hat_state & SDL_HAT_LEFT)
                dinp->HatX[i] = -1;
            else if (hat_state & SDL_HAT_RIGHT)
                dinp->HatX[i] = 1;
            
            if (hat_state & SDL_HAT_UP)
                dinp->HatY[i] = -1;
            else if (hat_state & SDL_HAT_DOWN)
                dinp->HatY[i] = 1;
        }
    }
    
    return 1;
}

int joy_refresh_devices(struct DevInput *dinp)
{
    // SDL2 implementation - reinitialize joysticks
    // First close existing ones
    for (int i = 0; i < sdl_num_joysticks; i++) {
        if (sdl_joysticks[i]) {
            SDL_JoystickClose(sdl_joysticks[i]);
            sdl_joysticks[i] = NULL;
        }
    }
    
    // Reopen all joysticks
    sdl_num_joysticks = SDL_NumJoysticks();
    if (sdl_num_joysticks > 16)
        sdl_num_joysticks = 16;
    
    for (int i = 0; i < sdl_num_joysticks; i++) {
        sdl_joysticks[i] = SDL_JoystickOpen(i);
        if (sdl_joysticks[i]) {
            dinp->Init[i] = 1;
            dinp->NumberOfDevices++;
        }
    }
    
    return 1;
}

void devinput_clear(struct DevInput *dinp)
{
    memset(dinp, 0, sizeof(struct DevInput));
    dinp->MinXAxis[0] = 0x10000;
    dinp->MaxXAxis[0] = 0;
    dinp->MinYAxis[0] = 0x10000;
    dinp->MaxYAxis[0] = 0;
    dinp->MinZAxis[0] = 0x10000;
    dinp->MaxZAxis[0] = 0;
    dinp->MinRAxis[0] = 0x10000;
    dinp->MaxRAxis[0] = 0;
    dinp->MinUAxis[0] = 0x10000;
    dinp->MaxUAxis[0] = 0;
    dinp->MinVAxis[0] = 0x10000;
    dinp->MaxVAxis[0] = 0;
    dinp->HatMax[0] = 0;
    dinp->NumberOfDevices = 1;
}

int joy_setup_device(struct DevInput *dinp, int jtype)
{
    devinput_clear(dinp);
    
    sdl_num_joysticks = SDL_NumJoysticks();
    if (sdl_num_joysticks > 16)
        sdl_num_joysticks = 16;
    
    if (sdl_num_joysticks == 0) {
        dinp->Type = -1;
        return -1;
    }
    
    // Setup basic joystick parameters
    for (int i = 0; i < sdl_num_joysticks; i++) {
        if (sdl_joysticks[i]) {
            dinp->DeviceType[i] = 112;
            dinp->NumberOfButtons[i] = SDL_JoystickNumButtons(sdl_joysticks[i]);
            dinp->Init[i] = 1;
            dinp->ConfigType[i] = jtype;
            
            // Set axis ranges (SDL uses -32768 to 32767)
            dinp->MinXAxis[i] = -32768;
            dinp->MaxXAxis[i] = 32767;
            dinp->XCentre[i] = 0;
            
            dinp->MinYAxis[i] = -32768;
            dinp->MaxYAxis[i] = 32767;
            dinp->YCentre[i] = 0;
            
            dinp->MinZAxis[i] = -32768;
            dinp->MaxZAxis[i] = 32767;
            dinp->ZCentre[i] = 0;
        }
    }
    
    dinp->Type = jtype;
    dinp->NumberOfDevices = sdl_num_joysticks;
    
    return 1;
}


static int get_JoyId_by_instanceId(SDL_JoystickID instance_id)
{
    for (int i = 0; i < sdl_num_joysticks; i++) {
        if (SDL_JoystickInstanceID(sdl_joysticks[i]) == instance_id) {
            return i;
        }
    }
    return -1;
}

TbResult JEvent(const SDL_Event *ev)
{   
    
    int i;
    struct DevInput *dinp = &joy;
    switch (ev->type)
    {
    case SDL_JOYAXISMOTION:
    case SDL_JOYHATMOTION:
        // Currently handled in joy_update_inputs()
        break;
    case SDL_JOYBUTTONDOWN:
        i = get_JoyId_by_instanceId(ev->jbutton.which);
        dinp->Buttons[i] |= (1 << ev->jbutton.button);
        break;
    case SDL_JOYBUTTONUP:
        i = get_JoyId_by_instanceId(ev->jbutton.which);
        dinp->Buttons[i] &= ~(1 << ev->jbutton.button);
        break;
    }
        
    return Lb_OK;
}



/** Joystick drivers initialization.
 */
int joy_driver_init(void)
{
    // Initialize SDL joystick subsystem
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
        LOGERR("Failed to initialize SDL joystick subsystem: %s", SDL_GetError());
        return 0;
    }
    
    // Open all available joysticks
    sdl_num_joysticks = SDL_NumJoysticks();
    if (sdl_num_joysticks > 16)
        sdl_num_joysticks = 16;
    
    LOGDBG("Found %d joystick(s)", sdl_num_joysticks);
    
    for (int i = 0; i < sdl_num_joysticks; i++) {
        sdl_joysticks[i] = SDL_JoystickOpen(i);
        if (sdl_joysticks[i]) {
            LOGDBG("Opened joystick %d: %s", i, SDL_JoystickName(sdl_joysticks[i]));
        } else {
            LOGERR("Failed to open joystick %d: %s", i, SDL_GetError());
        }
    }
    
    return 1;
}

/** Joystick drivers shutdown.
 */
int joy_driver_shutdown(void)
{
    // Close all opened joysticks
    for (int i = 0; i < sdl_num_joysticks; i++) {
        if (sdl_joysticks[i]) {
            SDL_JoystickClose(sdl_joysticks[i]);
            sdl_joysticks[i] = NULL;
        }
    }
    sdl_num_joysticks = 0;
    
    // Shutdown SDL joystick subsystem
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    
    return 1;
}
/******************************************************************************/
