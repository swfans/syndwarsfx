/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file sjoyst.c
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
static SDL_GameController *sdl_controllers[MAX_JOYSTICK_COUNT] = {NULL};
static int sdl_num_controllers = 0;
/******************************************************************************/

static void devinput_clear(struct DevInput *dinp);

int JoySetInterrupt(short val)
{
    // SDL2 does not use hardware interrupts for joystick input
    return -1;
}

const char* joy_get_button_label(int button)
{
    //even though with sdl we can mix controllers we'll return labels of the first one
    // needs to distinguish between nintendo XYAB, xbox YXBA, symbols on playstation, etc.
    //SDL3 could use SDL_GamepadButtonLabel but SDL2 does not have it

    switch (button)
    {
        case CONTROLLER_BUTTON_BACK:
            return "BCK";
        case CONTROLLER_BUTTON_SELECT:
            return "SEL";
        case CONTROLLER_BUTTON_START:
            return "STRT";
        case CONTROLLER_BUTTON_LEFTSTICK:
            return "LSTK";
        case CONTROLLER_BUTTON_RIGHTSTICK:
            return "RSTK";
        case CONTROLLER_BUTTON_LEFTSHOULDER:
            return "LB";
        case CONTROLLER_BUTTON_RIGHTSHOULDER:
            return "RB";
        case CONTROLLER_BUTTON_DPAD_UP:
            return "DPU";
        case CONTROLLER_BUTTON_DPAD_DOWN:
            return "DPD";
        case CONTROLLER_BUTTON_DPAD_LEFT:
            return "DPL";
        case CONTROLLER_BUTTON_DPAD_RIGHT:
            return "DPR";
        case CONTROLLER_BUTTON_MISC1:
            return "MSC";
    }

    SDL_GameController *controller = sdl_controllers[0];
    SDL_GameControllerType ctrl_type = SDL_GameControllerGetType(controller);

    // TODO remove true condition when testing is done
    if (ctrl_type == SDL_CONTROLLER_TYPE_PS3 ||
        ctrl_type == SDL_CONTROLLER_TYPE_PS4 ||
        ctrl_type == SDL_CONTROLLER_TYPE_PS5 || true)
    {
        switch (button)
        {
            case CONTROLLER_BUTTON_A:
                return "X";
            case CONTROLLER_BUTTON_B:
                return "O";
            case CONTROLLER_BUTTON_X:
                return "[]"; // no real square symbol in font so using brackets
            case CONTROLLER_BUTTON_Y:
                return "A"; // A looks like triangle already in the font used
        }
    }
    else if (ctrl_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO ||
             ctrl_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT ||
             ctrl_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT ||
             ctrl_type == SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR)
    {
        switch (button)
        {
            case CONTROLLER_BUTTON_A:
                return "B";
            case CONTROLLER_BUTTON_B:
                return "A";
            case CONTROLLER_BUTTON_X:
                return "Y";
            case CONTROLLER_BUTTON_Y:
                return "X";
        }
    }
    else //Xbox and others
    {
        switch (button)
        {
            case CONTROLLER_BUTTON_A:
                return "A";
            case CONTROLLER_BUTTON_B:
                return "B";
            case CONTROLLER_BUTTON_X:
                return "X";
            case CONTROLLER_BUTTON_Y:
                return "Y";
        }
    }
    return "UNK";
}

int joy_get_device_name(char *textbuf)
{
    const char *name = sdl_controllers[0] ? SDL_GameControllerName(sdl_controllers[0]) : "No Controller";
    strncpy(textbuf, name, 52);
    textbuf[51] = '\0';
    return Lb_SUCCESS;
}

int joy_update_inputs(struct DevInput *dinp)
{
    if (dinp->Type == -1)
        return -1;
    
    // Clear all device states
    for (int i = 0; i < MAX_JOYSTICK_COUNT; i++)
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
    
    // Read state from all opened game controllers
    for (int i = 0; i < sdl_num_controllers; i++)
    {
        SDL_GameController *controller = sdl_controllers[i];
        if (!controller || !dinp->Init[i])
            continue;
        
        // Read analog stick axes (standardized)
        dinp->AnalogueX[i] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        dinp->AnalogueY[i] = -SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        dinp->AnalogueZ[i] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        dinp->AnalogueR[i] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        dinp->AnalogueU[i] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        dinp->AnalogueV[i] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        
        // Convert analog to digital
        dinp->DigitalX[i] = (dinp->AnalogueX[i] < -16384) ? -1 : (dinp->AnalogueX[i] > 16384) ? 1 : 0;
        dinp->DigitalY[i] = (dinp->AnalogueY[i] < -16384) ? -1 : (dinp->AnalogueY[i] > 16384) ? 1 : 0;
        dinp->DigitalZ[i] = (dinp->AnalogueZ[i] < -16384) ? -1 : (dinp->AnalogueZ[i] > 16384) ? 1 : 0;
        dinp->DigitalR[i] = (dinp->AnalogueR[i] < -16384) ? -1 : (dinp->AnalogueR[i] > 16384) ? 1 : 0;
        
        // Read D-pad
        dinp->HatX[i] = 0;
        dinp->HatY[i] = 0;
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
            dinp->HatX[i] = -1;
        else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
            dinp->HatX[i] = 1;
        
        if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
            dinp->HatY[i] = -1;
        else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
            dinp->HatY[i] = 1;
    }
    
    return 1;
}

int joy_refresh_devices(struct DevInput *dinp)
{
    for (int i = 0; i < sdl_num_controllers; i++) {
        if (sdl_controllers[i]) {
            SDL_GameControllerClose(sdl_controllers[i]);
            sdl_controllers[i] = NULL;
        }
    }
    devinput_clear(dinp);
    
    // Reopen all game controllers
    sdl_num_controllers = 0;
    int num_joysticks = SDL_NumJoysticks();
    for (int i = 0; i < num_joysticks && sdl_num_controllers < MAX_JOYSTICK_COUNT; i++) {
        if (SDL_IsGameController(i)) {
            sdl_controllers[sdl_num_controllers] = SDL_GameControllerOpen(i);
            if (sdl_controllers[sdl_num_controllers]) {
                dinp->Init[sdl_num_controllers] = 1;
                dinp->NumberOfDevices++;
                sdl_num_controllers++;
            }
        }
    }
    
    return 1;
}

static void devinput_clear(struct DevInput *dinp)
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
    dinp->NumberOfDevices = 0;
}

int joy_setup_device(struct DevInput *dinp, int jtype)
{
    devinput_clear(dinp);
    
    if (sdl_num_controllers == 0) {
        dinp->Type = -1;
        return -1;
    }
    
    // Setup basic controller parameters
    for (int i = 0; i < sdl_num_controllers; i++) {
        if (sdl_controllers[i]) {
            dinp->NumberOfButtons[i] = SDL_CONTROLLER_BUTTON_MAX;
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
            
            dinp->MinRAxis[i] = -32768;
            dinp->MaxRAxis[i] = 32767;
            dinp->RCentre[i] = 0;
        }
    }
    
    dinp->Type = jtype;
    dinp->NumberOfDevices = sdl_num_controllers;
    
    return 1;
}


static int get_JoyId_by_instanceId(SDL_JoystickID instance_id)
{
    for (int i = 0; i < sdl_num_controllers; i++) {
        if (sdl_controllers[i]) {
            SDL_Joystick *joy = SDL_GameControllerGetJoystick(sdl_controllers[i]);
            if (joy && SDL_JoystickInstanceID(joy) == instance_id) {
                return i;
            }
        }
    }
    return -1;
}

static Uint8 joystickbutton_to_gamepadbutton(const Uint8 button, SDL_GameController *controller)
{
    // Find which controller button this joystick button corresponds to
    SDL_GameControllerButton ctrl_button = SDL_CONTROLLER_BUTTON_INVALID;
    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(controller, (SDL_GameControllerButton)i);
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON && bind.value.button == button)
        {
            ctrl_button = (SDL_GameControllerButton)i;
            break;
        }
    }
    return ctrl_button;

}

TbResult JEvent(const SDL_Event *ev)
{   
    int i;
    struct DevInput *dinp = &joy;
    switch (ev->type)
    {
    case SDL_CONTROLLERAXISMOTION:
        // Currently handled in joy_update_inputs()
        break;
    case SDL_CONTROLLERBUTTONDOWN:
        i = get_JoyId_by_instanceId(ev->cbutton.which);
        if (i >= 0 && i < MAX_JOYSTICK_COUNT)
            dinp->Buttons[i] |= (1 << ev->cbutton.button);
        break;
    case SDL_CONTROLLERBUTTONUP:
        i = get_JoyId_by_instanceId(ev->cbutton.which);
        if (i >= 0 && i < MAX_JOYSTICK_COUNT)
            dinp->Buttons[i] &= ~(1 << ev->cbutton.button);
        break;
    case SDL_JOYBUTTONDOWN:
        i = get_JoyId_by_instanceId(ev->jbutton.which);
        if (i >= 0 && i < MAX_JOYSTICK_COUNT)
            dinp->Buttons[i] |= (1 << joystickbutton_to_gamepadbutton(ev->jbutton.button,sdl_controllers[i]));
        break;
    case SDL_JOYBUTTONUP:
        i = get_JoyId_by_instanceId(ev->jbutton.which);
        if (i >= 0 && i < MAX_JOYSTICK_COUNT)
            dinp->Buttons[i] &= ~(1 << joystickbutton_to_gamepadbutton(ev->jbutton.button,sdl_controllers[i]));
        break;

    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
        joy_refresh_devices(dinp);
        break;
    }
        
    return Lb_OK;
}



/** Joystick drivers initialization.
 */
int joy_driver_init(void)
{
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        LOGERR("Failed to initialize SDL game controller subsystem: %s", SDL_GetError());
        return 0;
    }
    
    sdl_num_controllers = 0;
    int num_joysticks = SDL_NumJoysticks();
    
    LOGDBG("Found %d joystick(s)", num_joysticks);
    
    for (int i = 0; i < num_joysticks && sdl_num_controllers < MAX_JOYSTICK_COUNT; i++) {
        if (SDL_IsGameController(i)) {
            sdl_controllers[sdl_num_controllers] = SDL_GameControllerOpen(i);
            if (sdl_controllers[sdl_num_controllers]) {
                LOGDBG("Opened game controller %d: %s", sdl_num_controllers, 
                    SDL_GameControllerName(sdl_controllers[sdl_num_controllers]));
                sdl_num_controllers++;
            } else {
                LOGERR("Failed to open game controller %d: %s", i, SDL_GetError());
            }
        } else {
            LOGDBG("Joystick %d is not a game controller, skipping", i);
        }
    }
    
    return 1;
}

int joy_driver_shutdown(void)
{
    // Close all opened game controllers
    for (int i = 0; i < sdl_num_controllers; i++) {
        if (sdl_controllers[i]) {
            SDL_GameControllerClose(sdl_controllers[i]);
            sdl_controllers[i] = NULL;
        }
    }
    sdl_num_controllers = 0;
    
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    
    return 1;
}
/******************************************************************************/
