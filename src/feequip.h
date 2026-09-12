/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file feequip.h
 *     Header file for feequip.c.
 * @par Purpose:
 *     Front-end desktop and menu system, equipment screen.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 22 Oct 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef FEEQUIP_H
#define FEEQUIP_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum DisplayBoxContentTypes {
  DiBoxCt_ANIM = 0,		/**< Shows animation presentation of the selected item */
  DiBoxCt_TEXT,			/**< Shows verbose text description the selected item */
};

struct ScreenShape;
struct ScreenBoxBase;

#pragma pack()
/******************************************************************************/
/** Weapon selected in the equipment screen, 0 for none.
 */
extern ubyte selected_weapon;
extern sbyte selected_agent;
extern ubyte display_box_content;
extern ubyte mo_weapon;

ubyte show_equipment_screen(void);

ubyte flashy_draw_draw_equip_agent_name_shape(struct ScreenShape *shape, ubyte gbstate);
void draw_equip_agent_name_shape(struct ScreenShape *shape, ubyte gbstate);
void equip_update_for_selected_weapon(void);

void draw_text_property_bk(struct ScreenBoxBase *box, const char *text);
void draw_text_property_lv(struct ScreenBoxBase *box, const char *text);

/** Draw a background part of a bar of rectangles.
 */
void draw_discrete_rects_bar_bk(struct ScreenBoxBase *box, const char *text, TbPixel color);
/** Draw a foreground part of a bar of rectangles, including the fill level.
 */
void draw_discrete_rects_bar_lv(struct ScreenBoxBase *box, int lv, int lv_max, TbPixel *colors);

TbBool mouse_over_agent_panel_shape(struct ScreenShape *shape);
ubyte flashy_draw_agent_panel_shape(struct ScreenShape *shape, ubyte gbstate);
void draw_agent_panel_shape(struct ScreenShape *shape, ushort spridx, ubyte gbstate);

void update_equip_cost_text(void);
void update_cybmod_cost_text(void);

void switch_shared_equip_screen_buttons_to_equip(void);
void switch_equip_offer_to_buy(void);
void switch_equip_offer_to_sell(void);
void check_buy_sell_button(void);
void init_weapon_anim(ubyte weapon);
void display_box_content_state_switch(void);

void init_equip_screen_boxes(void);

/** Reset players part of equip screen state.
 *
 * Resets all parts of equip screen which are set by the player
 * or related to players game progress.
 */
void reset_equip_screen_player_state(void);

void reset_equip_screen_boxes_flags(void);
void set_flag01_equip_screen_boxes(void);

void init_equip_screen_shapes(void);

TbBool dragged_weapon_can_drop_on_research(void);
void dragged_weapon_drop_on_research(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
