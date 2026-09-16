/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file gmouse.c
 *     Platform-independent part of mouse handling routines.
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
#include "bfmouse.h"

#if defined(HAVE_CONFIG_H)
#  include "bfconfig.h"
#endif

#include "bfscreen.h"
#include "bfplanar.h"

TbResult LbMouseOnMove(struct TbPoint pos);

volatile TbBool lbMouseInstalled = false;
volatile TbBool lbMouseOffline = false;
volatile TbBool lbInteruptMouse = false;
volatile TbBool lbMouseAutoReset = true;
volatile TbBool lbMouseCapture = false;
volatile TbMouseToScreen lbMouseToScreen = &MouseToScreen;

TbResult mouseControl(TbMouseAction action, struct TbPoint *pos)
{
    struct TbPoint dstPos;

    if (!lbMouseInstalled)
        return Lb_OK;
    dstPos.x = pos->x;
    dstPos.y = pos->y;

    switch (action)
    {
    case MActn_MOUSEMOVE:
        lbMouseToScreen(&dstPos);
        LbMouseOnMove(dstPos);
        break;
    case MActn_LBUTTONDOWN:
        lbDisplay.MLeftButton = 1;
        if ( !lbDisplay.LeftButton )
        {
            lbMouseToScreen(&dstPos);
            LbMouseOnMove(dstPos);
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 0;
            lbDisplay.LeftButton = 1;
        }
        break;
    case MActn_LBUTTONUP:
        lbDisplay.MLeftButton = 0;
        if ( !lbDisplay.RLeftButton )
        {
            lbMouseToScreen(&dstPos);
            LbMouseOnMove(dstPos);
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 1;
        }
        break;
    case MActn_RBUTTONDOWN:
        lbDisplay.MRightButton = 1;
        if ( !lbDisplay.RightButton )
        {
            lbMouseToScreen(&dstPos);
            LbMouseOnMove(dstPos);
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 0;
            lbDisplay.RightButton = 1;
        }
        break;
    case MActn_RBUTTONUP:
        lbDisplay.MRightButton = 0;
        if ( !lbDisplay.RRightButton )
        {
            lbMouseToScreen(&dstPos);
            LbMouseOnMove(dstPos);
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 1;
        }
        break;
    case MActn_MBUTTONDOWN:
        lbDisplay.MMiddleButton = 1;
        if ( !lbDisplay.MiddleButton )
        {
            lbMouseToScreen(&dstPos);
            LbMouseOnMove(dstPos);
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.MiddleButton = 1;
            lbDisplay.RMiddleButton = 0;
        }
        break;
    case MActn_MBUTTONUP:
        lbDisplay.MMiddleButton = 0;
        if ( !lbDisplay.RMiddleButton )
        {
            lbMouseToScreen(&dstPos);
            LbMouseOnMove(dstPos);
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RMiddleButton = 1;
        }
        break;
    case MActn_WHEELMOVEUP:
#if defined(LB_ENABLE_MOUSE_WHEEL)
        lbDisplay.WheelPosition--;
        lbDisplay.WheelMoveUp++;
        lbDisplay.WheelMoveDown = 0;
#endif
        break;
    case MActn_WHEELMOVEDOWN:
#if defined(LB_ENABLE_MOUSE_WHEEL)
        lbDisplay.WheelPosition++;
        lbDisplay.WheelMoveUp = 0;
        lbDisplay.WheelMoveDown++;
#endif
        break;
    default:
        return Lb_OK;
    }
    return Lb_SUCCESS;
}

/******************************************************************************/
