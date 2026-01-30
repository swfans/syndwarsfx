/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file swindows.cpp
 *     Implementation of related functions.
 * @par Purpose:
 *     Unknown.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Nov 2008 - 05 Nov 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <unistd.h>
#include "bfwindows.h"
#include "bfscreen.h"
#include "bfscrsurf.h"
#include "bfmemut.h"
#include "bfmouse.h"
#include "bfpalette.h"
#include "privbflog.h"

#include <SDL/SDL.h>

#if defined(WIN32)
#  include <synchapi.h>
#endif

/** use atexit() to un-initialize SDL */
#define LBI_SDL_QUIT_USE_ATEXIT

volatile TbBool lbLibInitialised = false;
volatile TbBool lbUserQuit = false;
volatile TbBool lbAppActive;

TbIdleControl lbIdleHandlers[LB_IDLE_HANDLERS_MAX] = {0};

void LbRegisterStandardVideoModes(void);
TbResult MEvent(const SDL_Event *ev);
TbResult KEvent(const SDL_Event *ev);
TbResult LbIPaletteRestoreLost(void);

TbResult LbBaseInitialise(void)
{
#if 0
    char buf[32];
#endif
    // Clear global variables
    lbScreenInitialised = false;
    lbScreenSurface = NULL;
    lbDrawSurface = NULL;
    LbMemorySet(lbIdleHandlers, 0, sizeof(lbIdleHandlers));
    lbAppActive = true;
    LbMouseChangeMoveRatio(NORMAL_MOUSE_MOVE_RATIO, NORMAL_MOUSE_MOVE_RATIO);
    // Register default video modes
    if (lbScreenModeInfoNum == 0) {
        LbRegisterStandardVideoModes();
    }
    // SDL environment variables
#if 0
    //TODO - enable or remove; do we really want to control this instead of allowing users to use env vars?
    if (lbVideoDriver[0] != '\0') {
        sprintf(buf,"SDL_VIDEODRIVER=%s",lbVideoDriver);
        putenv(buf);
    }
#endif
    // Initialize SDL library
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE |
      SDL_INIT_JOYSTICK) < 0) {
        LOGERR("SDL init: %s",SDL_GetError());
        return Lb_FAIL;
    }
    SDL_ShowCursor(SDL_DISABLE);
    // Setup the atexit() call to un-initialize
#if defined(LBI_SDL_QUIT_USE_ATEXIT)
    atexit(SDL_Quit);
#endif
    lbLibInitialised = true;
    return Lb_SUCCESS;
}

TbResult LbInputRestate(void)
{
    return Lb_SUCCESS;
}

/** @internal
 * Triggers control function for given SDL window event.
 * @return SUCCESS if the event was processed, FAIL if it isn't supported,
 *         OK if no window event.
 */
TbResult WEvent(const SDL_Event *ev)
{
    //TbWindowAction action;

    switch (ev->type)
    {
    case SDL_ACTIVEEVENT:
        if (ev->active.state & SDL_APPACTIVE) {
            lbAppActive = (ev->active.gain != 0);
            LOGNO("Active = %d",(int)lbAppActive);
            LbInputRestate();
        }
        if ((lbAppActive) && (lbDisplay.Palette != NULL)) {
            // Switching windows in 8-bit modes often leads to palette loss
            LbIPaletteRestoreLost();
        }
        return Lb_SUCCESS;
    case SDL_SYSWMEVENT:
        break;
    }
    return Lb_OK;
}

void LbDoMultitasking(void)
{
#if defined(WIN32)
    Sleep(LB_LARGE_DELAY_TIME>>1); // This switches to other tasks
#else
    SDL_Delay(LB_LARGE_DELAY_TIME);
#endif
    LbWindowsControl();
}

static void LbI_ProcessEvent(const SDL_Event *ev)
{
    LOGNO("Starting for ev type %d", (int)ev->type);

    switch (ev->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        KEvent(ev);
        break;

    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        MEvent(ev);
        break;

    case SDL_ACTIVEEVENT:
    case SDL_SYSWMEVENT:
        WEvent(ev);
        break;

    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        //TODO INPUT make joypad support
        //JEvent(ev);
        break;

    case SDL_VIDEORESIZE:
        break;

    case SDL_VIDEOEXPOSE:
        break;

    case SDL_QUIT:
        lbUserQuit = true;
        break;
    }
}

TbBool LbWindowsControl(void)
{
    SDL_Event ev;
    int n;
    // process events until event queue is empty, but also
    // limit the events to avoid a freeze
    for (n = 256; n != 0; n--)
    {
        if (SDL_PollEvent(&ev) == 0)
            break;
        LbI_ProcessEvent(&ev);
    }
    for (n=0; n < LB_IDLE_HANDLERS_MAX; n++) {
        TbIdleControl cb;

        cb = lbIdleHandlers[n];
        if (cb == NULL)
            break;
        cb();
    }
    return !lbUserQuit;
}

TbResult LbRegisterIdleHandler(TbIdleControl cb)
{
    int n;

    for (n=0; ; n++) {
        if (n >= LB_IDLE_HANDLERS_MAX)
            return Lb_FAIL;
        if (lbIdleHandlers[n] == NULL)
            break;
    }
    lbIdleHandlers[n] = cb;
    return Lb_SUCCESS;
}

TbResult LbBaseReset(void)
{
    lbLibInitialised = false;
    LbMemorySet(lbIdleHandlers, 0, sizeof(lbIdleHandlers));
#if !defined(LBI_SDL_QUIT_USE_ATEXIT)
    SDL_Quit();
#endif
    return Lb_SUCCESS;
}

/******************************************************************************/
