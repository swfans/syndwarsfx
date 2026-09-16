/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet, Genewars or Dungeon Keeper.
/******************************************************************************/
/** @file bfmouse.h
 *     Header file for smouse.cpp.
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
#ifndef BFLIBRARY_BFMOUSE_H_
#define BFLIBRARY_BFMOUSE_H_

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct TbSprite;
struct TbPoint;

#define NORMAL_MOUSE_MOVE_RATIO 256

enum MouseAction {
    MActn_NONE = 0,
    MActn_MOUSEMOVE,
    MActn_LBUTTONDOWN,
    MActn_LBUTTONUP,
    MActn_RBUTTONDOWN,
    MActn_RBUTTONUP,
    MActn_MBUTTONDOWN,
    MActn_MBUTTONUP,
    MActn_WHEELMOVEUP,
    MActn_WHEELMOVEDOWN,
};

typedef ubyte TbMouseAction;

typedef void (*TbMouseToScreen)(struct TbPoint *);

extern volatile TbBool lbMouseInstalled;
extern volatile TbBool lbMouseOffline;

/** Move the mouse on screen outside of standard screen swaps.
 * If the OS uses interrupts or async events to control mouse position, this
 * option allows to update the cursor on screen every time mouse position is
 * updated, rather than wait for the next swap.
 */
extern volatile TbBool lbInteruptMouse;

/** Auto-resets mouse position to the center of the screen.
 * If the OS mouse travels in any direction, it will reset;
 * advertised mouse position will be computed using shift from
 * that central point of the screen. This allows unbounded mouse
 * movement in-game (useful for FPP view).
 */
extern volatile TbBool lbMouseAutoReset;
extern volatile TbMouseToScreen lbMouseToScreen;

/** Confines the OS mouse pointer to the graphics window.
 * Stores the requested capture state; use LbMouseChangeCapture()
 * to change it, so that the state gets applied to existing window.
 */
extern volatile TbBool lbMouseCapture;

TbResult LbMousePlace(void);
TbResult LbMouseRemove(void);

/** Update mouse state as screen swap begins.
 *
 * Places the mouse cursor on screen. Requires graphics screen surface
 * to be unlocked.
 */
TbResult LbMouseOnBeginSwap(void);

/** Return mouse state to normal as screen swap ends.
 *
 * Removes the mouse cursor from screen. Requires graphics screen surface
 * to be unlocked.
 */
TbResult LbMouseOnEndSwap(void);

/**
 * Set offset within pointer sprite where the mouse actually points at.
 *  Also called LbMouseSetPointerHotspot().
 */
TbResult LbMouseChangeSpriteOffset(long hot_x, long hot_y);

/**
 * Get offset within pointer sprite where the mouse actually points at.
 *  Also called GetPointerHotspot().
 */
TbResult LbMouseGetSpriteOffset(long *hot_x, long *hot_y);

TbResult LbMouseChangeSprite(const struct TbSprite *pointer_spr);

/**
 * Changes mouse movement ratio.
 * Note that this function can be run even before mouse setup. Still, the factor
 *  will be reset during the installation - so use it after LbMouseSetup().
 *
 * @param ratio_x Movement ratio in X direction; NORMAL_MOUSE_MOVE_RATIO
 *   means unchanged ratio from OS.
 * @param ratio_y Movement ratio in Y direction; NORMAL_MOUSE_MOVE_RATIO
 *   means unchanged ratio from OS.
 * @return Lb_SUCCESS if the ratio values were of correct range and have been set.
 */
TbResult LbMouseChangeMoveRatio(long ratio_x, long ratio_y);

/**
 * Enables or disables confining the mouse pointer to the graphics window.
 * Note that this function can be run even before screen setup. The state
 * is stored, and applied to the window when it gets created.
 *
 * Without capture, on a multi-monitor desktop the pointer may leave
 * the game area, and a click on another screen defocuses the window.
 *
 * @param enable True to capture the pointer, false to release it.
 * @return Lb_SUCCESS if the state was stored and applied, if applicable.
 */
TbResult LbMouseChangeCapture(TbBool enable);

/**
 * Returns whether mouse setup was successfully executed or not.
 */
TbBool LbMouseIsInstalled(void);

/**
 * Initializes the mouse input support.
 *
 * @param pointer_spr Mouse pointer sprite to be used.
 * @param ratio_x Movement ratio in X direction; NORMAL_MOUSE_MOVE_RATIO
 *   means unchanged ratio from OS.
 * @param ratio_y Movement ratio in Y direction; NORMAL_MOUSE_MOVE_RATIO
 *   means unchanged ratio from OS.
 * @return Lb_SUCCESS if the setup succeeded.
 */
TbResult LbMouseSetup(const struct TbSprite *pointer_spr, int ratio_x, int ratio_y);
TbResult LbMouseReset(void);
TbResult LbMouseSuspend(void);
TbResult LbMouseSetWindow(long x, long y, long width, long height);
TbResult LbMouseSetPosition(long x, long y);

/** Default callback for converting raw mouse coords into screen coords.
 *
 *  Computes shift in mouse position, and uses it with internal move ration
 *  to determine relative shift expected on screen. Then replaces the given
 *  pos with one pointed by that shift.
 *
 * @param pos The source 2D point, and where result is placed.
 */
void MouseToScreen(struct TbPoint *pos);

/* MInt()/MEvent() function used to read mouse events is defined
 * internally, in platform-dependent manner. */

/** Platform-independent mouse control handler.
 *
 *  Can be used to react on mouse changes, when called within
 *  platform-dependent handler.
 */
TbResult mouseControl(TbMouseAction action, struct TbPoint *pos);

#ifdef __cplusplus
};
#endif

#endif // BFLIBRARY_BFMOUSE_H_
/******************************************************************************/
