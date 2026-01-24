/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file keyboard.c
 *     Buffered handler for keystrokes.
 * @par Purpose:
 *     Taps keyboard event handler and stores each key pressed.
 * @par Comment:
 *     When keys are handled outside the event, it is possible that some
 *     keypresses will not be registered, making typing text frustrating.
 * @author   Tomasz Lis
 * @date     10 Oct 2012 - 13 Oct 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"

#include "bfmemut.h"
#include "bfkeybd.h"
#include "bfjoyst.h"

#include "game.h"
#include "game_data.h"
#include "guitext.h"

/******************************************************************************/

#define KEYBOARD_BUFFER_SIZE 16
#define GAMEKEY_JOY_BUTTONS_MAX 4

#pragma pack(1)

ushort kbkeys[GKey_KEYS_COUNT];

ushort jskeys[GKey_KEYS_COUNT];

ulong buffered_keys[KEYBOARD_BUFFER_SIZE];
ulong buffered_keys_read_index;
ulong buffered_keys_write_index;

#pragma pack()

ubyte is_key_pressed(TbKeyCode key, TbKeyMods kmodif)
{
    if ((kmodif == KMod_NONE) && (lbShift != KMod_NONE))
        return 0;
    if ((kmodif == KMod_DONTCARE) || (kmodif & lbShift) == kmodif)
        return lbKeyOn[key];
    return 0;
}

void clear_key_pressed(TbKeyCode key)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (key >= sizeof(lbKeyOn)/sizeof(lbKeyOn[0]))
        return;
#pragma GCC diagnostic pop
    lbKeyOn[key] = 0;
    if (key == lbInkey) {
        lbInkey = KC_UNASSIGNED;
        lbExtendedKeyPress = 0;
    }
}

void simulate_key_press(TbKeyCode key)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (key >= sizeof(lbKeyOn)/sizeof(lbKeyOn[0]))
        return;
#pragma GCC diagnostic pop
    if (key == KC_UNASSIGNED)
        return;
    lbKeyOn[key] = 1;
    if (lbInkey == KC_UNASSIGNED) {
        lbInkey = key;
    }
}

ubyte is_joy_pressed(ushort jkeys, ubyte channel)
{
    return (jkeys && jkeys == joy.Buttons[channel]);
}

ubyte is_joy_pressed_any(ubyte channel)
{
    return (joy.Buttons[channel] != 0);
}

ushort get_joy_pressed_key(ubyte channel)
{
    uint jbtn_pressed;
    ushort jbtn, buttons_num, pressed_count;
    ushort jskey_flags;

    jskey_flags = 0;
    buttons_num = joy.NumberOfButtons[channel];
    jbtn_pressed = joy.Buttons[channel];
    pressed_count = 0;
    for (jbtn = 0; jbtn < buttons_num; jbtn++)
    {
          if (pressed_count >= GAMEKEY_JOY_BUTTONS_MAX)
              break;
          if ((jbtn_pressed & (1 << jbtn)) != 0) {
              jskey_flags |= (1 << jbtn);
              pressed_count++;
          }
    }
    return jskey_flags;
}

void clear_joy_pressed(ushort jkeys, ubyte channel)
{
    if (channel >= sizeof(joy.Buttons[0])/sizeof(joy.Buttons[0]))
        return;
    joy.Buttons[channel] &= ~jkeys;
}

void sprint_joy_key(char *ostr, int buttons_num, ushort jkeys)
{
    int tx_len;
    ushort jbtn, pressed_count;

    tx_len = 0;
    pressed_count = 0;
    for (jbtn = 0; jbtn < buttons_num; jbtn++)
    {
        if (pressed_count >= GAMEKEY_JOY_BUTTONS_MAX)
            break;
        if (((1 << jbtn) & jkeys) != 0)
        {
            if (tx_len > 0)
                ostr[tx_len++] = '+';
            if (jbtn >= 9)
            {
                uint n;
                n = jbtn + 1;
                ostr[tx_len++] = '0' + (n / 10);
                ostr[tx_len++] = '0' + (n % 10);
            }
            else
            {
                char c;
                c = '1' + jbtn;
                ostr[tx_len++] = c;
            }
            ++pressed_count;
        }
    }
    ostr[tx_len] = '\0';
}

TbKeyMods get_gamekey_modifier(GameKey gkey)
{
    TbKeyMods kmodif;

    switch (gkey)
    {
    case GKey_SELF_DESTRUCT:
        kmodif = KMod_ALT;
        break;
    case GKey_TRANS_OBJ_SURF_COL:
    case GKey_TRANS_OBJ_LINE_COL:
    case GKey_GAMESPEED_INC:
    case GKey_GAMESPEED_DEC:
        kmodif = KMod_CONTROL;
        break;
    case GKey_CAMERA_PERSPECTV:
        kmodif = KMod_NONE;
        break;
    default:
        kmodif = KMod_DONTCARE;
        break;
    }
    return kmodif;
}

ubyte is_gamekey_kbd_pressed(GameKey gkey)
{
    TbKeyCode kkey;
    TbKeyMods kmodif;

    kkey = kbkeys[gkey];
    kmodif = get_gamekey_modifier(gkey);

    return is_key_pressed(kkey, kmodif);
}

ubyte is_gamekey_joy_pressed(GameKey gkey, ubyte channel)
{
    ushort jkeys;

    jkeys = jskeys[gkey];

    return is_joy_pressed(jkeys, channel);
}

ubyte is_gamekey_pressed(GameKey gkey)
{
    return is_gamekey_kbd_pressed(gkey) || is_gamekey_joy_pressed(gkey, 0);
}

void clear_gamekey_kbd_pressed(GameKey gkey)
{
    TbKeyCode kkey;

    kkey = kbkeys[gkey];
    clear_key_pressed(kkey);
}

void clear_gamekey_joy_pressed(GameKey gkey, ubyte channel)
{
    ushort jkeys;

    jkeys = jskeys[gkey];
    clear_joy_pressed(jkeys, channel);
}

void clear_gamekey_pressed(GameKey gkey)
{
    if (is_gamekey_kbd_pressed(gkey))
    {
        clear_gamekey_kbd_pressed(gkey);
    }
    else
    {
        clear_gamekey_joy_pressed(gkey, 0);
    }
}

void set_gamekey_kbd(GameKey gkey, TbKeyCode key)
{
    kbkeys[gkey] = key;
}

void set_gamekey_joy(GameKey gkey, ushort jkey)
{
    jskeys[gkey] = jkey;
}

ubyte switch_keycode_to_name_code_on_national_keyboard(ubyte keyno)
{
    ubyte rkey;
    rkey = keyno;
    switch (language_3str[0])
    {
    case 'g':
        // Y and Z keys reversed
        if (keyno == KC_Y)
        {
          rkey = KC_Z;
        }
        else if (keyno == KC_Z)
        {
          rkey = KC_Y;
        }
        break;
    case 'f':
        if (keyno == KC_SEMICOLON)
        {
          rkey = KC_M;
        }
        else if (keyno == KC_Z)
        {
          rkey = KC_W;
        }
        else if (keyno == KC_M)
        {
          rkey = KC_COMMA;
        }
        else if (keyno == KC_COMMA)
        {
          rkey = KC_SEMICOLON;
        }
        else if (keyno == KC_A)
        {
          rkey = KC_Q;
        }
        else if (keyno == KC_W)
        {
          rkey = KC_Z;
        }
        else if (keyno == KC_Q)
        {
          rkey = KC_A;
        }
        break;
    default:
        break;
    }
    return rkey;
}

void sprint_gamekey_combination_kbd(char *ostr, GameKey gkey)
{
    ushort keyno;
    TbKeyMods kmodif;

    keyno = kbkeys[gkey];
    kmodif = get_gamekey_modifier(gkey);
    if (keyno == KC_UNASSIGNED)
    {
        ostr[0] = '\0';
        return;
    }

    if (lbKeyNames[keyno] == NULL)
    {
        sprintf(ostr, "FOO %d", (int)keyno);
        return;
    }

    keyno = switch_keycode_to_name_code_on_national_keyboard(keyno);

    switch (kmodif)
    {
    case KMod_ALT:
        sprintf(ostr, "ALT+%s", lbKeyNames[keyno]);
        break;
    case KMod_CONTROL:
        sprintf(ostr, "CTRL+%s", lbKeyNames[keyno]);
        break;
    default:
        sprintf(ostr, "%s", lbKeyNames[keyno]);
        break;
    }
}

void sprint_gamekey_combination_joy(char *ostr, GameKey gkey)
{
    if ((gkey >= GKey_UP) && (gkey <= GKey_RIGHT))
    {
        strncpy(ostr, gui_strings[GSTR_SYS_GAME_KEYS + 7 + (gkey - GKey_UP)], 50);
    }
    else
    {
        sprint_joy_key(ostr, joy.NumberOfButtons[0], jskeys[gkey]);
    }
}

static void add_key_to_buffer(ubyte key)
{
    ulong new_write_index;

    buffered_keys[buffered_keys_write_index] = key;

    new_write_index = (buffered_keys_write_index + 1) % KEYBOARD_BUFFER_SIZE;

    if (new_write_index != buffered_keys_read_index)
        buffered_keys_write_index = new_write_index;
}

ulong next_buffered_key(void)
{
    ulong key;

    if (buffered_keys_read_index == buffered_keys_write_index)
        return 0;

    key = buffered_keys[buffered_keys_read_index];
    buffered_keys_read_index
      = (buffered_keys_read_index + 1) % KEYBOARD_BUFFER_SIZE;

  return key;
}

void reset_buffered_keys(void)
{
    buffered_keys_read_index  = 0;
    buffered_keys_write_index = 0;
}

TbResult handle_custom_key_press(TbKeyAction action, TbKeyCode code)
{
    if (action != KActn_KEYDOWN)
        return false;

    return Lb_OK;
}

TbResult KEventBufferedKeysUpdate(TbKeyAction action, TbKeyCode code)
{
    if (action == KActn_KEYDOWN)
    {
        if (!lbInkey_prefixed)
            add_key_to_buffer(code);
    }

    return handle_custom_key_press(action, code);
}

void init_buffered_keys(void)
{
    LbKeyboardCustomHandler(KEventBufferedKeysUpdate);
}

TbBool user_read_value(char *buf, ubyte len, ubyte type)
{
    TbBool ret;
    asm volatile (
      "call ASM_user_read_value\n"
        : "=r" (ret) : "a" (buf), "d" (len), "b" (type));
    return ret;
}

void input(void)
{
    uint16_t n;

    n = lbShift;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT])
        n |= KMod_SHIFT;
    else
        n &= ~KMod_SHIFT;
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL])
        n |= KMod_CONTROL;
    else
        n &= ~KMod_CONTROL;
    if (lbKeyOn[KC_RALT] || lbKeyOn[KC_LALT])
        n |= KMod_ALT;
    else
        n &= ~KMod_ALT;
    lbShift = n;
}

void set_default_game_keys(void)
{
    LbMemorySet(jskeys, 0, sizeof(jskeys));
    jskeys[GKey_VIEW_SPIN_L]   = 1 << CONTROLLER_BUTTON_LEFTSHOULDER;
    jskeys[GKey_VIEW_SPIN_R]   = 1 << CONTROLLER_BUTTON_RIGHTSHOULDER;
    jskeys[GKey_FIRE]          = 1 << CONTROLLER_BUTTON_A;
    jskeys[GKey_CHANGE_MD_WP]  = 1 << CONTROLLER_BUTTON_B;
    jskeys[GKey_CHANGE_AGENT]  = 1 << CONTROLLER_BUTTON_X;
    jskeys[GKey_SELF_DESTRUCT] = (1 << CONTROLLER_BUTTON_A | 1 << CONTROLLER_BUTTON_B | 1 << CONTROLLER_BUTTON_X | 1 << CONTROLLER_BUTTON_Y);
    jskeys[GKey_GROUP]         = 1 << CONTROLLER_BUTTON_SELECT;
    jskeys[GKey_GOTO_POINT]    = 1 << CONTROLLER_BUTTON_Y;
    jskeys[GKey_DROP_WEAPON]   = (1 << CONTROLLER_BUTTON_A | 1 << CONTROLLER_BUTTON_DPAD_LEFT);

    jskeys[GKey_SEL_AGENT_1] = (1 << CONTROLLER_BUTTON_LEFTSHOULDER | 1 << CONTROLLER_BUTTON_DPAD_UP);
    jskeys[GKey_SEL_AGENT_2] = (1 << CONTROLLER_BUTTON_LEFTSHOULDER | 1 << CONTROLLER_BUTTON_DPAD_DOWN);
    jskeys[GKey_SEL_AGENT_4] = (1 << CONTROLLER_BUTTON_LEFTSHOULDER | 1 << CONTROLLER_BUTTON_DPAD_RIGHT);
    jskeys[GKey_SEL_AGENT_3] = (1 << CONTROLLER_BUTTON_LEFTSHOULDER | 1 << CONTROLLER_BUTTON_DPAD_LEFT);

    ctl_joystick_type = JTyp_SDL_GAMECONTROLLER;

    LbMemorySet(kbkeys, 0, sizeof(kbkeys));
    kbkeys[GKey_NONE] = KC_UNASSIGNED;
    kbkeys[GKey_VIEW_SPIN_L] = KC_DELETE;
    kbkeys[GKey_VIEW_SPIN_R] = KC_PGDOWN;
    kbkeys[GKey_FIRE] = KC_LCONTROL;
    kbkeys[GKey_CHANGE_MD_WP] = KC_LALT;
    kbkeys[GKey_CHANGE_AGENT] = KC_TAB;
    kbkeys[GKey_UP] = KC_UP;
    kbkeys[GKey_DOWN] = KC_DOWN;
    kbkeys[GKey_GOTO_POINT] = KC_RCONTROL;
    kbkeys[GKey_GROUP] = KC_OEM_102;
    kbkeys[GKey_LEFT] = KC_LEFT;
    kbkeys[GKey_ZOOM_OUT] = KC_HOME;
    kbkeys[GKey_SELF_DESTRUCT] = KC_D;
    kbkeys[GKey_RIGHT] = KC_RIGHT;
    kbkeys[GKey_ZOOM_IN] = KC_END;
    kbkeys[GKey_DROP_WEAPON] = KC_Z;
    kbkeys[GKey_PAUSE] = KC_P;
    kbkeys[GKey_VIEW_TILT_U] = KC_INSERT;
    kbkeys[GKey_KEY_CONTROL] = KC_K;
    kbkeys[GKey_VIEW_TILT_D] = KC_PGUP;
    kbkeys[GKey_SEL_AGENT_1] = KC_1;
    kbkeys[GKey_SEL_AGENT_2] = KC_2;
    kbkeys[GKey_SEL_AGENT_4] = KC_4;
    kbkeys[GKey_SEL_AGENT_3] = KC_3;
    kbkeys[GKey_TRANS_OBJECTS] = KC_B;
    kbkeys[GKey_SCANNER_PULSE] = KC_S;
    kbkeys[GKey_CAMERA_PERSPECTV] = KC_F;
    kbkeys[GKey_TRANS_OBJ_SURF_COL] = KC_J;
    kbkeys[GKey_TRANS_OBJ_LINE_COL] = KC_H;
    kbkeys[GKey_SCANNR_BRIGH_INC] = KC_NUMPAD6;
    kbkeys[GKey_SCANNR_BRIGH_DEC] = KC_NUMPAD4;
    kbkeys[GKey_DANGR_TRACK] = KC_NUMPAD0;
    kbkeys[GKey_MUSIC_TRACK] = KC_NUMPAD5;
    kbkeys[GKey_SOUND_VOL_INC] = KC_NUMPAD7;
    kbkeys[GKey_SOUND_VOL_DEC] = KC_NUMPAD1;
    kbkeys[GKey_DANGR_VOL_INC] = KC_NUMPAD8;
    kbkeys[GKey_DANGR_VOL_DEC] = KC_NUMPAD2;
    kbkeys[GKey_MUSIC_VOL_INC] = KC_NUMPAD9;
    kbkeys[GKey_MUSIC_VOL_DEC] = KC_NUMPAD3;
    kbkeys[GKey_MISSN_RESTART] = KC_R;
    kbkeys[GKey_GAMESPEED_INC] = KC_ADD;
    kbkeys[GKey_GAMESPEED_DEC] = KC_SUBTRACT;
    kbkeys[GKey_SCREENSHOT] = KC_M;
    kbkeys[GKey_SEL_WEP_1] = KC_5;
    kbkeys[GKey_SEL_WEP_2] = KC_6;
    kbkeys[GKey_SEL_WEP_3] = KC_7;
    kbkeys[GKey_SEL_WEP_4] = KC_8;
    kbkeys[GKey_SEL_WEP_5] = KC_9;
    kbkeys[GKey_SEL_WEP_6] = KC_0;
    kbkeys[GKey_USE_MEDIKIT] = KC_UNASSIGNED;
#ifdef MORE_GAME_KEYS
    kbkeys[GKey_SUPERSHIELD] = KC_UNASSIGNED;
    kbkeys[GKey_VIEW_THERMAL] = KC_UNASSIGNED;
#endif
}

/******************************************************************************/
