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

/** Highest amount of frames per game turn which can be asked for. Ten is
 * 160 frames a second, which no machine tried comes near; the bound is there
 * to catch a mistyped setting, not to express a limit of the drawing. */
#define FRAMES_PER_TURN_MAX 10

/** Amount of frames drawn per game turn. A value of 1 means one drawn frame
 * per game turn, which is the original behaviour. */
extern ushort render_frames_per_turn;

/** How many frames the turn being played is actually going to draw. Equal to
 * render_frames_per_turn while the in-mission view is on screen, and 1
 * everywhere else, so that menus and other screens keep a full length turn. */
extern ushort render_frames_this_turn;

/** Starts a drawn frame: advances drawturn, and moves the animation clock by
 * the share of the game turn this frame stands for. Frames are numbered from
 * 0 within the turn; the last one lands the clock exactly on the next turn,
 * so that a turn always adds up to one whatever the amount of frames. */
void render_clock_next_frame(ushort frame, ushort nframes);

/** Sets the animation clock to the given turn, on a mission starting. */
void render_clock_set_turn(ulong turn);

/** Whether the frame being drawn is the one which also carries the game turn
 * forward. False for the extra frames drawn within a turn, so that anything
 * animated from within the drawing code keeps its original pace. */
extern TbBool frame_advances_state;

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
