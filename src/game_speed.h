/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_speed.h
 *     Header file for game_speed.c.
 * @par Purpose:
 *     Control of the game speed.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 02 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GAME_SPEED_H
#define GAME_SPEED_H

#include "bftypes.h"
#include "game_bstype.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
extern ulong curr_tick_time;
extern ulong prev_tick_time;
extern GameTurn gameturn;
extern GameTurn prev_gameturn;

/** Counter of frames drawn to the screen.
 *
 * Advances once per drawn frame. It is the counter for things which only
 * exist while drawing - marking which elements were already drawn within the
 * current frame, for instance. Nothing in the simulation reads it, so drawing
 * code should use it rather than `gameturn` whenever it only needs to tell
 * one frame from the next.
 */
extern GameTurn drawturn;
extern ulong turns_delta;
extern ushort fifties_per_gameturn;

/** Amount of frames per second drawn by the game, and also aount of game
 * turns per second. */
extern ushort game_num_fps;

/** Opens a drawn frame: advances drawturn, and moves the animation clock on
 * by the given amount.
 *
 * The amount is in the units of render_anim_turn, so RENDER_ANIM_TURN_UNIT is
 * one animation turn - the pace the game has always drawn at, one animation
 * frame per game turn. Smaller amounts spread an animation frame over several
 * drawn frames, larger ones run animations faster, and zero opens a frame
 * which shows the animations exactly where the previous one left them.
 */
void render_clock_next_frame(u32 anim_turn_incr);

/** Places the animation clock on the given animation turn, on a mission or
 * the outro starting.
 */
void render_clock_set_turn(ulong turn);

/**
 * Handles game speed control inputs.
 * @return Returns true if packet was created, false otherwise.
 */
ubyte get_speed_control_inputs(void);

void wait_next_gameturn(void);

TbBool display_needs_redraw_this_turn(void);
void update_tick_time(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
