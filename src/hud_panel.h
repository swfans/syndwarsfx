/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file hud_panel.h
 *     Header file for hud_panel.c.
 * @par Purpose:
 *     Ingame Heads-Up Display panel draw.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     19 Apr 2022 - 27 Aug 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef HUD_PANEL_H
#define HUD_PANEL_H

#include "bftypes.h"
#include "game_bstype.h"
#include "guiboxes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define PANEL_STATE_NORMAL        0
#define PANEL_STATE_WEP_SEL_ONE   1
#define PANEL_STATE_WEP_SEL_GRP   5
#define PANEL_STATE_MOOD_SET_ONE  9
#define PANEL_STATE_MOOD_SET_GRP 13
#define PANEL_STATE_SEND_MESSAGE 17

enum PanelType {
    PanT_NONE = 0,
    PanT_AgentBadge,
    PanT_AgentMood,
    PanT_AgentHealth,
    PanT_AgentEnergy,
    PanT_AgentWeapon,
    PanT_AgentMedi,
    PanT_UNKN07,
    PanT_WeaponEnergy,
    PanT_UNKN09,
    PanT_Grouping,
};

enum PanelFlags {
    PanF_ENABLED = 0x0001,
    PanF_SPRITES_IN_LINE_HORIZ = 0x0020,
    PanF_SPRITES_IN_LINE_VERTC = 0x0040,
    PanF_SPR_TOGGLED_ON = 0x0080,
    PanF_RESIZE_MIDDLE_SPR = 0x0100,
    PanF_REPOSITION_HORIZ = 0x0200,
    PanF_REPOSITION_VERTC = 0x0400,
    PanF_REPOSITION_WITH_PARENT = 0x0800,
    PanF_STRECH_TO_PARENT_SIZE = 0x1000,
    PanF_REPOSITION_TO_AFTER = 0x2000,
};

enum PanelShift {
    PaSh_AGENT_UNUSED0 = 0,
    PaSh_AGENT_WEAPON_TO_LIST = 4,
    PaSh_GROUP_PANE_TO_THERMAL_BOX = 11,
    PaSh_GROUP_PANE_TO_THERMAL_SPR = 12,
    PaSh_GROUP_PANE_AGENTS = 13,
    PaSh_WEP_CURR_BTN_TO_SYMBOL = 16,
    PaSh_WEP_FRST_BTN_TO_SYMBOL = 17,
    PaSh_WEP_NEXT_BTN_TO_SYMBOL = 18,
    PaSh_WEP_CURR_BTN_TO_DECOR =  19,
    PaSh_WEP_FRST_BTN_TO_DECOR = 20,
    PaSh_WEP_NEXT_BTN_TO_DECOR = 21,
    PaSh_WEP_NEXT_DISTANCE = 22,
    PaSh_WEP_CURR_BUTTON_AREA = 23,
    PaSh_WEP_NEXT_BUTTON_AREA = 24,
    PaSh_WEP_FOURPACK_BASE_SH = 25,
    PaSh_WEP_FOURPACK_SIZE = 26,
    PaSh_WEP_FOURPACK_SLOTS = 27,
};

struct GamePanel
{
    /** Rectange representing area active for mouse events and sprite drawing. */
    struct ScreenRect pos;
    /** Rectange representing area of dynamically generated content (text/value/bar). */
    struct ScreenRect dyn;
    /** Panel sprite decoration; only first is used, unless flags state otherwise. */
    short Spr[3];
    /** Panel sprite decorations to switch to on toggle event, or additional sprites
     * to draw on top. */
    short ExtraSpr[3];
    short OwningPanel;
    short SprWidth;
    short SprHeight;
    ushort Use;
    ushort Flags;
    ubyte ID;
    ubyte Type;
};

struct Thing;
struct TbPoint;

#pragma pack()
/******************************************************************************/
extern struct GamePanel *game_panel;
extern struct TbPoint *game_panel_shifts;
extern ubyte byte_153198;
extern long scanner_unkn370;

extern char player_message_text[8][128];
extern ubyte player_message_timer[8];
extern long scanner_unkn3CC;

/** Scale factor for figuring out GUI size.
 */
extern short gui_scale;

int SCANNER_objective_info_height(void);

void draw_new_panel(void);

TbResult load_pop_sprites_for_current_mode(void);

TbResult load_small_font_for_current_ingame_mode(void);

/** Returns if a game panel is active, considering the target which it controls.
 */
TbBool panel_active_based_on_target(short panel);

/** Returns if current mouse move position is over the given game panel.
 */
TbBool mouse_move_over_panel(short panel);

TbBool mouse_over_infrared_slant_box(short panel);

TbBool process_panel_state(void);
TbBool check_panel_button(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
