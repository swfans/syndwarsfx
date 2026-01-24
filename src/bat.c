/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file bat.c
 *     Routines implementing Breakout minigame.
 * @par Purpose:
 *     Implement functions for handling breakout-like minigame.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 22 Sep 2025
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bat.h"

#include <string.h>
#include "bfgentab.h"
#include "bfkeybd.h"
#include "bfmemut.h"
#include "bfscreen.h"
#include "ssampply.h"

#include "engincolour.h"

#include "bigmap.h"
#include "game.h"
#include "game_options.h"
#include "keyboard.h"
#include "player.h"
#include "sound.h"
#include "swlog.h"
#include "thing.h"
/******************************************************************************/

#pragma pack(1)

/** Minigame screen line width.
 *
 * To allow the output buffer to be used as texture, it needs to have
 * size which is compatible with textures. The actual size which we will
 * utilize can be smaller.
 */
#define BAT_SCREEN_LINE_WIDTH 256

#define BAT_SCREEN_WIDTH 96
#define BAT_SCREEN_HEIGHT 64

#define BAT_BRICK_COLUMNS 12
#define BAT_BRICK_ROWS 10

#define BAT_BALLS_LIMIT 32
#define BAT_BONUSES_LIMIT 16

struct BreakoutLevel {
    char *lv_name;
    ubyte bricks[BAT_BRICK_COLUMNS * BAT_BRICK_ROWS];
};

struct BATBall;

struct BATBall {
    int X;
    int Y;
    int UnkDw2;
    int UnkDw3;
    int NextX;
    int NextY;
    int UnkDw6;
    int UnkDw7;
    struct BATBall *ListNext;
    struct BATBall **ListPvNxPtr;
};

struct BATBonus;

struct BATBonus {
    int X;
    int Y;
    int UnkDw2;
    int UnkDw3;
    struct BATBonus *ListNext;
    struct BATBonus **ListPvNxPtr;
};

enum BATStates {
  BATSt_Reset = 0,
  BATSt_LevelIntro,
  BATSt_Gameplay,
  BATSt_BallLost,
  BATSt_GameOver,
  BATSt_GameWon,
};

#pragma pack()

extern int bat_unknvalA_min;
extern int bat_unknvalB_min;
extern int bat_unknvalA_max;
extern int bat_unknvalB_max;
extern int BAT_data_1e26e8;
extern int BAT_game_won_timer;
extern int BAT_level_intro_timer;
extern int BAT_ball_lost_timer;
extern int BAT_game_over_timer;
extern int BAT_state;
extern int BAT_levelno;
extern int BAT_num_lives;
extern int BAT_score;
extern int BAT_bricks_remain;
extern ubyte *BAT_screen;
extern ubyte BAT_bricks[BAT_BRICK_COLUMNS * BAT_BRICK_ROWS];
extern int BAT_paddle_x;
extern int BAT_data_1e2798;
extern TbPixel BAT_lives_colour;

extern struct BATBall BAT_balls_store[BAT_BALLS_LIMIT];
/** Linked list of unused balls. */
extern struct BATBall *BAT_free_balls;
/** Linked list of existing balls. */
extern struct BATBall *BAT_balls_head;
extern TbPixel BAT_ball_colour;

extern struct BATBonus BAT_bonus_store[BAT_BONUSES_LIMIT];
/** Linked list of unused bonus items. */
extern struct BATBonus *BAT_free_bonuses;
/** Linked list of existing bonus items. */
extern struct BATBonus *BAT_bonuses_head;

extern struct BreakoutLevel BAT_levels[];

static void BAT_screen_clear_rect(int beg_x, int beg_y, int end_x, int end_y)
{
    ubyte *o;
    int y;

    o = BAT_screen + beg_y * BAT_SCREEN_LINE_WIDTH;
    for (y = beg_y; y < end_y; y++)
    {
        memset(o + beg_x, 0, end_x - beg_x);
        o += BAT_SCREEN_LINE_WIDTH;
    }
}

static void BAT_screen_clear(void)
{
    BAT_screen_clear_rect(0, 0, BAT_SCREEN_WIDTH, BAT_SCREEN_HEIGHT);
}

static void BAT_draw_remainig_lives(int pos_x, int pos_y)
{
    ubyte *o;
    int y;

    o = BAT_screen + pos_y * BAT_SCREEN_LINE_WIDTH + pos_x;
    for (y = 0; y < BAT_num_lives; y++, o -= 5)
    {
        TbPixel px;

        px = BAT_lives_colour;
        o[1] = px;
        o[2] = px;
        o[3] = px;
        o[4] = px;
    }
}

void BAT_draw_char(int x, int y, char chr, ubyte col)
{
    asm volatile (
      "call ASM_BAT_draw_char\n"
        : : "a" (x), "d" (y), "b" (chr), "c" (col));
}

static void BAT_print(short pos_x, short pos_y, const char *str, ubyte coll)
{
    char chr;
    ubyte colour;

    colour = colour_lookup[coll];
    chr = *str;
    while (chr != '\0')
    {
        if (chr != ' ')
            BAT_draw_char(pos_x, pos_y, chr, colour);
        chr = *++str;
        pos_x += 4;
    }
}

static void BAT_draw_score_and_level(void)
{
    char locstr[32];

    sprintf(locstr, "%d", BAT_score);
    BAT_print(1, 1, locstr, ColLU_WHITE);
    sprintf(locstr, "%d", BAT_levelno);
    BAT_print(BAT_SCREEN_WIDTH / 2 - 2 * strlen(locstr), 1, locstr, ColLU_WHITE);
}

static void BAT_draw_win_message(void)
{
    const char *str;

    str = "CONGRATULATIONS";
    BAT_print(18, 10, str, ColLU_YELLOW);
    str = "BONUS GAME COMPLETE";
    BAT_print(10, 16, str, ColLU_YELLOW);
    str = "BONUS ITEM";
    BAT_print(28, 30, str, ColLU_WHITE);
    str = "AWARDED";
    BAT_print(34, 38, str, ColLU_WHITE);
}

static void BAT_draw_lost_life_message(void)
{
    const char *str;

    str = "BAD LUCK!";
    BAT_print(30, 52, str, 8);
}

static void BAT_draw_game_over_message(void)
{
    const char *str;

    str = "GAME OVER";
    BAT_print(30, 52, str, 8);
}

static void BAT_draw_level_intro_text(void)
{
    char locstr[64];

    if (BAT_level_intro_timer < 60)
          sprintf(locstr, "%s", BAT_levels[BAT_levelno - 1].lv_name);
    else
          sprintf(locstr, "ENTERING LEVEL : %d", BAT_levelno);
    BAT_print(BAT_SCREEN_WIDTH / 2 - 2 * strlen(locstr) + 1, 52, locstr, 8);
}

static void BAT_input_paddle(void)
{
    int ms_x, scr_x;

    ms_x = lbDisplay.MMouseX;
    scr_x = lbDisplay.GraphicsScreenWidth;
    BAT_paddle_x = BAT_SCREEN_WIDTH / 2 + ((ms_x - scr_x/2) * BAT_SCREEN_WIDTH) / (scr_x / 2);

    if (BAT_paddle_x < BAT_data_1e2798 + 1)
        BAT_paddle_x = BAT_data_1e2798 + 1;
    if (BAT_paddle_x + BAT_data_1e2798 > 95)
        BAT_paddle_x = 95 - BAT_data_1e2798;
}

/** Inputs active only during a level play.
 */
static void BAT_input_level(void)
{
      if (is_key_pressed(KC_N, KMod_SHIFT))
      {
          BAT_bricks_remain = 0;
          BAT_num_lives++;
      }
}

static void BAT_bricks_set_rect(int beg_c, int beg_r, int end_c, int end_r, ubyte val)
{
    int c, r;
    for (r = beg_r; r < end_r; r++)
    {
        for (c = beg_c; c < end_c; c++)
            BAT_bricks[BAT_BRICK_COLUMNS * r + c] = val;
    }
}

static void BAT_bricks_clear(void)
{
    BAT_bricks_set_rect(0, 0, BAT_BRICK_COLUMNS, BAT_BRICK_ROWS, 0x80);
}

void BAT_unknsub_22(void)
{
    asm volatile ("call ASM_BAT_unknsub_22\n"
        :  :  : "eax" );
}

static void BAT_ball_colour_fade(void)
{
    int fade_lv;

    if (BAT_level_intro_timer > 60)
    {
        fade_lv = 32 - 2 * (90 - BAT_level_intro_timer);
        if (fade_lv < 0)
            fade_lv = 0;
        BAT_ball_colour = pixmap.fade_table[256 * fade_lv + colour_lookup[1]];
        BAT_unknsub_22();
        BAT_unknsub_22();
        BAT_unknsub_22();
        BAT_unknsub_22();
    }
    else
    {
        BAT_ball_colour = colour_lookup[1];
    }
}

static void BAT_reset_free_balls(void)
{
    int i;

    BAT_free_balls = &BAT_balls_store[0];
    BAT_balls_store[0].ListPvNxPtr = &BAT_free_balls;
    for (i = 0; i < BAT_BALLS_LIMIT - 1; i++)
    {
        BAT_balls_store[i].ListNext = &BAT_balls_store[i + 1];
        BAT_balls_store[i + 1].ListPvNxPtr = &BAT_balls_store[i].ListNext;
    }
    BAT_balls_store[i].ListNext = NULL;
    BAT_balls_head = NULL;
}

static void BAT_reset_free_bonuses(void)
{
    int i;

    BAT_free_bonuses = &BAT_bonus_store[0];
    BAT_bonus_store[0].ListPvNxPtr = &BAT_free_bonuses;
    for (i = 0; i < BAT_BONUSES_LIMIT - 1; i++)
    {
        BAT_bonus_store[i].ListNext = &BAT_bonus_store[i + 1];
        BAT_bonus_store[i + 1].ListPvNxPtr = &BAT_bonus_store[i].ListNext;
    }
    BAT_bonus_store[i].ListNext = 0;
    BAT_bonuses_head = NULL;
}

static void BAT_start_new_game(void)
{
    BAT_state = BATSt_LevelIntro;
    BAT_levelno = 1;
    BAT_score = 0;
    BAT_num_lives = 2;
    BAT_level_intro_timer = 90;
}

static struct BATBall *new_ball(void)
{
    struct BATBall *curr;
    struct BATBall *next;

    curr = BAT_free_balls;
    if (curr == NULL) {
        LOGERR("BAT: no more free balls");
        return NULL;
    }

    next = curr->ListNext;
    BAT_free_balls = next;
    if (next != NULL)
        next->ListPvNxPtr = &BAT_free_balls;

    next = BAT_balls_head;
    if (next != NULL) {
          curr->ListNext = next;
          next->ListPvNxPtr = &curr->ListNext;
    } else {
          curr->ListNext = NULL;
    }
    curr->ListPvNxPtr = &BAT_balls_head;
    BAT_balls_head = curr;

    return curr;
}

static void BAT_create_starting_ball(void)
{
    struct BATBall *ball;

    ball = new_ball();

    if (ball != NULL)
    {
        ushort rnd;

        ball->X = (BAT_SCREEN_WIDTH / 2) << 8;
        ball->Y = (49) << 8;
        rnd = rand();
        ball->UnkDw7 = 0;
        ball->UnkDw6 = ((rnd & 7) << 8) - 768;
    }
}

int BAT_unknsub_27(void)
{
#if 0
    int ret;
    asm volatile (
      "call ASM_BAT_unknsub_27\n"
        : "=r" (ret) : );
    return ret;
#endif
    int tile_x, tile_z;

    for (tile_x = bat_unknvalA_min; tile_x <= bat_unknvalA_max; tile_x++)
    {
        for (tile_z = bat_unknvalB_min; tile_z <= bat_unknvalB_max; tile_z++)
        {
            struct MyMapElement *p_mapel;
            short thing;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            thing = p_mapel->Child;

            while (thing != 0)
            {
                if (thing <= 0)
                {
                    struct SimpleThing *p_sthing;
                    p_sthing = &sthings[thing];
                    thing = p_sthing->Next;
                }
                else
                {
                    struct Thing *p_thing;
                    p_thing = &things[thing];
                    thing = p_thing->Next;

                    if ((p_thing->Type == TT_PERSON)
                      && ((p_thing->Flag & TngF_PlayerAgent) != 0)
                      && ((p_thing->Flag & TngF_Destroyed) == 0)
                      && (p_thing->U.UPerson.CurrentWeapon == WEP_PERSUADRTRN))
                    {
                        ingame.Flags |= GamF_BillboardBAT;
                        return 1;
                    }
                }
            }
        }
    }
    ingame.Flags &= ~GamF_BillboardBAT;
    return 0;
}

void BAT_unknsub_21(void)
{
    asm volatile ("call ASM_BAT_unknsub_21\n"
        :  :  : "eax" );
}

void breakout_play_sub1(void)
{
    asm volatile ("call ASM_breakout_play_sub1\n"
        :  :  : "eax" );
}

void breakout_play_sub2(void)
{
    asm volatile ("call ASM_breakout_play_sub2\n"
        :  :  : "eax" );
}

void BAT_bricks_load(int a1)
{
    asm volatile ("call ASM_BAT_bricks_load\n"
        :  :  : "eax" );
}

void BAT_play(void)
{
#if 0
    asm volatile ("call ASM_BAT_play\n"
        :  :  : "eax" );
#else
    switch (BAT_state)
    {
    case BATSt_Reset:
        if (BAT_unknsub_27())
        {
            BAT_start_new_game();
            BAT_bricks_clear();
        }
        return;

    case BATSt_LevelIntro:
        BAT_screen_clear();
        BAT_level_intro_timer--;
        BAT_input_paddle();

        BAT_ball_colour_fade();
        breakout_play_sub1();
        BAT_draw_score_and_level();
        BAT_draw_remainig_lives(90, 2);
        BAT_unknsub_21();
        breakout_play_sub2();
        BAT_draw_level_intro_text();

        if (BAT_level_intro_timer == 60)
        {
            BAT_bricks_load(BAT_levelno);
            BAT_reset_free_balls();
            BAT_create_starting_ball();
        }
        if (BAT_level_intro_timer == 0)
        {
            BAT_state = BATSt_Gameplay;
        }
        break;

    case BATSt_Gameplay:
        BAT_screen_clear();
        BAT_input_paddle();

        BAT_unknsub_22();
        BAT_unknsub_22();
        BAT_unknsub_22();
        BAT_unknsub_22();
        breakout_play_sub1();
        BAT_draw_score_and_level();
        BAT_draw_remainig_lives(90, 2);
        BAT_unknsub_21();
        breakout_play_sub2();
        if (BAT_balls_head == NULL)
        {
            BAT_state = BATSt_BallLost;
            BAT_ball_lost_timer = 80;
            play_sample_using_heap(0, 73, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        }
        BAT_input_level();
        if (BAT_bricks_remain == 0)
        {
            if (BAT_levelno == 10)
            {
              BAT_state = BATSt_GameWon;
              BAT_game_won_timer = 150;
              rand();
              BAT_data_1e26e8 = 0;
            }
            else
            {
              BAT_state = BATSt_LevelIntro;
              BAT_level_intro_timer = 90;
            }
            BAT_bricks_clear();
            BAT_levelno++;
            play_sample_using_heap(0, 70, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 3u);
        }
        break;

    case BATSt_BallLost:
      BAT_screen_clear();
      BAT_ball_lost_timer--;
      BAT_input_paddle();

      breakout_play_sub1();
      BAT_draw_score_and_level();
      BAT_draw_remainig_lives(90, 2);
      BAT_unknsub_21();
      breakout_play_sub2();
      BAT_draw_lost_life_message();

      if (BAT_ball_lost_timer == 50)
      {
          if (BAT_num_lives) {
            BAT_num_lives--;
          } else {
            BAT_state = BATSt_GameOver;
            BAT_game_over_timer = 100;
          }
      }
      if (BAT_ball_lost_timer == 40)
      {
          BAT_create_starting_ball();
      }
      if (BAT_ball_lost_timer == 0)
      {
          BAT_state = BATSt_Gameplay;
      }
      break;

    case BATSt_GameOver:
      BAT_screen_clear();
      BAT_game_over_timer--;
      BAT_input_paddle();

      breakout_play_sub1();
      BAT_draw_score_and_level();
      BAT_draw_remainig_lives(90, 2);
      breakout_play_sub2();
      BAT_unknsub_21();
      BAT_draw_game_over_message();

      if (BAT_game_over_timer == 0)
      {
          BAT_bricks_clear();
          BAT_start_new_game();
      }
      break;

    case BATSt_GameWon:
      BAT_screen_clear();
      BAT_game_won_timer--;

      BAT_draw_win_message();
      player_agents_add_random_epidermises(&players[local_player_no]);

      if (BAT_game_won_timer == 0)
      {
          BAT_bricks_clear();
          BAT_reset_free_balls();
          BAT_start_new_game();
          ingame.UserFlags |= UsrF_WonBAT;
      }
      break;

    default:
      break;
    }
    if (!BAT_unknsub_27())
    {
        BAT_state = BATSt_Reset;
        BAT_bricks_clear();
        BAT_screen_clear();
        BAT_reset_free_balls();
        BAT_reset_free_bonuses();
    }
#endif
}

void BAT_unknsub_20(int a1, int a2, int a3, int a4, ubyte *p_screen)
{
    asm volatile (
      "push %4\n"
      "call ASM_BAT_unknsub_20\n"
        : : "a" (a1), "d" (a2), "b" (a3), "c" (a4), "g" (p_screen));
}

/******************************************************************************/
