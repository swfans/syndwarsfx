/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file game_sprts.c
 *     Lists of sprites used by the game.
 * @par Purpose:
 *     Defines references to heads, tails and data buffers of 2D sprites.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 02 May 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "game_sprts.h"

#include <assert.h>
#include "bffile.h"
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfsprite.h"

#include "frame_sprani.h"

#include "game_data.h"
#include "game_options.h"
#include "game.h"
#include "swlog.h"

/******************************************************************************/

struct TbSprite *pointer_sprites;
struct TbSprite *pointer_sprites_end;
ubyte *pointer_data;
ubyte *pointer_data_end;
short pointer_sprites_scale = 1;

struct TbSprite *pop1_sprites;
struct TbSprite *pop1_sprites_end;
ubyte *pop1_data;
ubyte *pop1_data_end;
short pop1_sprites_scale = 1;

struct TbSprite *small_font;
struct TbSprite *small_font_end;
ubyte *small_font_data;
ubyte *small_font_data_end;

struct TbSprite *small2_font;
struct TbSprite *small2_font_end;
ubyte *small2_font_data;

struct TbSprite *small_med_font;
struct TbSprite *small_med_font_end;
ubyte *small_med_font_data;

struct TbSprite *med_font;
struct TbSprite *med_font_end;
ubyte *med_font_data;

struct TbSprite *med2_font;
struct TbSprite *med2_font_end;
ubyte *med2_font_data;

struct TbSprite *big_font;
struct TbSprite *big_font_end;
ubyte *big_font_data;

struct TbSprite *fe_icons_sprites;
struct TbSprite *fe_icons_sprites_end;
ubyte *fe_icons_sprites_data;

struct TbSprite *wepicons_sprites;
struct TbSprite *wepicons_sprites_end;
ubyte *wepicons_sprites_data;

struct TbSprite *fe_mouseptr_sprites;
struct TbSprite *fe_mouseptr_sprites_end;
ubyte *fe_mouseptr_sprites_data;

struct TbSprite *fepanel_sprites;
struct TbSprite *fepanel_sprites_end;
ubyte *fepanel_sprites_data;

ubyte *m_sprites_data;
ubyte *m_sprites_data_end;
/******************************************************************************/

/** Load sprites of given style and detail, to a preallocated area of specific size.
 *
 * Loads files to areas between given start and end pointers. Expects the loaded
 * files to fit before the given end pointers.
 * For DAT file, decreases the end pointer, setting it to the end of loaded data.
 * For TAB file, clears any extra area above loaded data until the end pointer.
 *
 * The end pointers are expected to be initially set to max supported data size.
 */
static inline TbResult load_sprites_with_detail(ubyte *p_dat, ubyte **pp_dat_end,
  ubyte *p_spr, ubyte *p_spr_end, const char *dir, const char *name,
  short styleno, short detail)
{
    char locstr[DISKPATH_SIZE];
    long len;
    TbResult ret;

    sprintf(locstr, "%s/%s%hd-%hd.tab", dir, name, styleno, detail);
    if (!LbFileExists(locstr)) {
        LOGSYNC("Could not find \"%s\" file", locstr);
        LbMemorySet(p_spr, 0, p_spr_end - p_spr);
        return Lb_FAIL;
    }

    ret = Lb_SUCCESS;

    sprintf(locstr, "%s/%s%hd-%hd.dat", dir, name, styleno, detail);
    len = LbFileLoadAt(locstr, p_dat);
    if (len == -1) {
        ret = Lb_FAIL;
        len = 0;
    }
    else if (p_dat + len > (*pp_dat_end)) {
        ret = Lb_OK;
        LOGERR("Load caused overflow by %d bytes, file \"%s\"", p_dat + len - (*pp_dat_end), locstr);
    }
    else {
        *pp_dat_end = p_dat + len;
    }

    sprintf(locstr, "%s/%s%hd-%hd.tab", dir, name, styleno, detail);
    len = LbFileLoadAt(locstr, p_spr);
    if (len == -1) {
        ret = Lb_FAIL;
        LbMemorySet(p_spr, 0, p_spr_end - p_spr);
    }
    else if (p_spr + len > p_spr_end) {
        if (ret == Lb_SUCCESS) ret = Lb_OK;
        LOGERR("Load caused overflow by %d bytes, file \"%s\"", p_spr + len - p_spr_end, locstr);
        // Clear the overflown data
        LbMemorySet(p_spr + len, 0, p_spr + len - p_spr_end);
    }
    else if (p_spr + len < p_spr_end) {
        if (ret == Lb_SUCCESS) ret = Lb_OK;
        // If loaded file is smaller than allocated space, clear the rest; we do not access
        // data pointer for zero-sized sprites, so this is enough as safety measure
        LbMemorySet(p_spr + len, 0, p_spr_end - p_spr - len);
    }

    return ret;
}

static inline TbResult load_any_sprites_up_to(const char *dir, const char *name,
  ushort min_sprites, struct TbSprite *p_sprites, struct TbSprite *p_sprites_end,
  ubyte *p_data, ubyte **pp_data_end, short *p_scale, short styleno, short max_detail)
{
    short detail;
    TbResult ret;

    if (p_sprites_end - p_sprites < min_sprites) {
        LOGERR("Preallocated area for %d '%s%hd' sprites is below expected minimum %d",
         p_sprites_end - p_sprites, name, styleno, min_sprites);
    }
    ret = Lb_FAIL;
    for (detail = max_detail; detail >= 0; detail--)
    {
        ubyte *loc_data_end;

        loc_data_end = *pp_data_end;
        ret = load_sprites_with_detail(p_data, &loc_data_end,
          (ubyte *)p_sprites, (ubyte *)p_sprites_end,
          dir, name, styleno, detail);
        if (ret != Lb_FAIL) {
            *pp_data_end = loc_data_end;
            break;
        }
    }
    if (detail < 0) {
        LOGERR("Some '%s%hd' sprites not loaded, tried detail %hd..0",
          name, styleno, max_detail);
        detail = 0;
    }
    if (p_scale != NULL)
        *p_scale = detail + 1;

    return ret;
}

TbResult load_sprites_mouse_pointers_up_to(const char *dir, short styleno, short max_detail)
{
    ubyte *loc_data_end = pointer_data_end;
    return load_any_sprites_up_to(dir, "pointr", 10, pointer_sprites, pointer_sprites_end,
      pointer_data, &loc_data_end, &pointer_sprites_scale, styleno, max_detail);
}

void setup_mouse_pointers(void)
{
    struct TbSprite *spr;

    if ((pointer_sprites == NULL) || (pointer_data == NULL)) {
        LOGERR("Pointer sprite memory not allocated, skipping");
        return;
    }
    LbSpriteSetup(pointer_sprites, pointer_sprites_end, pointer_data);
    // Make mouse pointer sprite 1 an empty (zero size) sprite
    spr = &pointer_sprites[1];
    spr->SWidth = 0;
    spr->SHeight = 0;
}

void reset_mouse_pointers(void)
{
    LbSpriteReset(pointer_sprites, pointer_sprites_end, pointer_data);
}

TbResult load_sprites_fe_icons(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 170;
    p_buf = *pp_buf;
    fe_icons_sprites = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    fe_icons_sprites_end = (struct TbSprite *)p_buf;
    fe_icons_sprites_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "icons", min_sprites,
      fe_icons_sprites, fe_icons_sprites_end,
      fe_icons_sprites_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_fe_icons(void)
{
    LbSpriteSetup(fe_icons_sprites, fe_icons_sprites_end, fe_icons_sprites_data);
}

void reset_sprites_fe_icons(void)
{
    LbSpriteReset(fe_icons_sprites, fe_icons_sprites_end, fe_icons_sprites_data);
}

TbResult load_sprites_wepicons(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 34;
    p_buf = *pp_buf;
    wepicons_sprites = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    wepicons_sprites_end = (struct TbSprite *)p_buf;
    wepicons_sprites_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "wepicon", min_sprites,
      wepicons_sprites, wepicons_sprites_end,
      wepicons_sprites_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_wepicons(void)
{
    LbSpriteSetup(wepicons_sprites, wepicons_sprites_end, wepicons_sprites_data);
}

void reset_sprites_wepicons(void)
{
    LbSpriteReset(wepicons_sprites, wepicons_sprites_end, wepicons_sprites_data);
}

TbResult load_sprites_fe_mouse_pointers(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 15;
    p_buf = *pp_buf;
    fe_mouseptr_sprites = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    fe_mouseptr_sprites_end = (struct TbSprite *)p_buf;
    fe_mouseptr_sprites_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "mouse", min_sprites,
      fe_mouseptr_sprites, fe_mouseptr_sprites_end,
      fe_mouseptr_sprites_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL) {
        *pp_buf = p_buf;
    } else { // If load failed, still allocate the area for TAB file - the buffer is zero-filled
        *pp_buf = fe_mouseptr_sprites_data;
    }

    return ret;
}

void setup_sprites_fe_mouse_pointers(void)
{
    LbSpriteSetup(fe_mouseptr_sprites, fe_mouseptr_sprites_end, fe_mouseptr_sprites_data);
}

void reset_sprites_fe_mouse_pointers(void)
{
    LbSpriteReset(fe_mouseptr_sprites, fe_mouseptr_sprites_end, fe_mouseptr_sprites_data);
}

TbResult load_sprites_med_font(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 224;
    p_buf = *pp_buf;
    med_font = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    med_font_end = (struct TbSprite *)p_buf;
    med_font_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "fontc", min_sprites, med_font, med_font_end,
      med_font_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_med_font(void)
{
    LbSpriteSetup(med_font, med_font_end, med_font_data);
}

void reset_sprites_med_font(void)
{
    LbSpriteReset(med_font, med_font_end, med_font_data);
}

TbResult load_sprites_big_font(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 224;
    p_buf = *pp_buf;
    big_font = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    big_font_end = (struct TbSprite *)p_buf;
    big_font_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "fontc", min_sprites, big_font, big_font_end,
      big_font_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_big_font(void)
{
    LbSpriteSetup(big_font, big_font_end, big_font_data);
}

void reset_sprites_big_font(void)
{
    LbSpriteReset(big_font, big_font_end, big_font_data);
}

TbResult load_sprites_small_font_up_to(const char *dir, short max_detail)
{
    ubyte *loc_data_end = small_font_data_end;
    const short styleno = 0;
    return load_any_sprites_up_to(dir, "fontc", 224, small_font, small_font_end,
      small_font_data, &loc_data_end, NULL, styleno, max_detail);
}

void setup_sprites_small_font(void)
{
    LbSpriteSetup(small_font, small_font_end, small_font_data);
}

void reset_sprites_small_font(void)
{
    LbSpriteReset(small_font, small_font_end, small_font_data);
}

TbResult load_sprites_small_med_font(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 224;
    p_buf = *pp_buf;
    small_med_font = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    small_med_font_end = (struct TbSprite *)p_buf;
    small_med_font_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "fontc", min_sprites, small_med_font, small_med_font_end,
      small_med_font_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_small_med_font(void)
{
    LbSpriteSetup(small_med_font, small_med_font_end, small_med_font_data);
}

void reset_sprites_small_med_font(void)
{
    LbSpriteReset(small_med_font, small_med_font_end, small_med_font_data);
}

TbResult load_sprites_med2_font(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 224;
    p_buf = *pp_buf;
    med2_font = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    med2_font_end = (struct TbSprite *)p_buf;
    med2_font_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "fontc", min_sprites, med2_font, med2_font_end,
      med2_font_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_med2_font(void)
{
    LbSpriteSetup(med2_font, med2_font_end, med2_font_data);
}

void reset_sprites_med2_font(void)
{
    LbSpriteReset(med2_font, med2_font_end, med2_font_data);
}

TbResult load_sprites_small2_font(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 224;
    p_buf = *pp_buf;
    small2_font = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    small2_font_end = (struct TbSprite *)p_buf;
    small2_font_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "fontc", min_sprites, small2_font, small2_font_end,
      small2_font_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_small2_font(void)
{
    LbSpriteSetup(small2_font, small2_font_end, small2_font_data);
}

void reset_sprites_small2_font(void)
{
    LbSpriteReset(small2_font, small2_font_end, small2_font_data);
}

TbResult load_multicolor_sprites(const char *dir)
{
    char locstr[DISKPATH_SIZE];
    long len;
    TbResult ret;

    ret = Lb_OK;

    sprintf(locstr, "%s/mspr-%d.dat", dir, ingame.TrenchcoatPreference);
    len = LbFileLoadAt(locstr, m_sprites_data);
    if (len == -1) {
        ret = Lb_FAIL;
        len = 0;
    }
    // additional 512 bytes are always reserved by LbDataLoad()
    assert(m_sprites_data_end + 512 >= m_sprites_data + len);
    sprintf(locstr, "%s/mspr-%d.tab", dir, ingame.TrenchcoatPreference);
    len = LbFileLoadAt(locstr, m_sprites);
    if (len == -1) {
        ret = Lb_FAIL;
        len = 128 * sizeof(struct TbSprite);
        LbMemorySet(m_sprites, '\0', len);
    }
    m_sprites_end = (struct TbSprite *)((ubyte *)m_sprites + len);

    return ret;
}

void setup_multicolor_sprites(void)
{
    LbSpriteSetup(m_sprites, m_sprites_end, m_sprites_data);
}

void reset_multicolor_sprites(void)
{
    LbSpriteReset(m_sprites, m_sprites_end, m_sprites_data);
}

void debug_multicolor_sprite(int idx)
{
    int i;
    char strdata[100];
    char *str;
    struct TbSprite *spr;
    unsigned char *ptr;

    spr = &m_sprites[idx];
    str = strdata;
    sprintf(str, "spr %d width %d height %d ptr 0x%lx data",
      idx, (int)spr->SWidth, (int)spr->SHeight, (ulong)spr->Data);
    ptr = spr->Data;
    for (i = 0; i < 10; i++)
    {
        str = strdata + strlen(strdata);
        sprintf(str, " %02x", (int)*ptr);
        ptr++;
    }
    LOGDBG("m_sprites: %s", strdata);
}

TbResult load_sprites_fepanel(ubyte **pp_buf,
  const char *dir, short styleno, short max_detail)
{
    ubyte *p_buf;
    ushort min_sprites;
    TbResult ret;

    min_sprites = 150;
    p_buf = *pp_buf;
    fepanel_sprites = (struct TbSprite *)p_buf;
    p_buf += min_sprites * sizeof(struct TbSprite);
    fepanel_sprites_end = (struct TbSprite *)p_buf;
    fepanel_sprites_data = p_buf;
    p_buf += min_sprites * 4096 * (max_detail + 1) * (max_detail + 1);

    ret = load_any_sprites_up_to(dir, "panel", min_sprites, fepanel_sprites, fepanel_sprites_end,
      fepanel_sprites_data, &p_buf, NULL, styleno, max_detail);

    if (ret != Lb_FAIL)
        *pp_buf = p_buf;

    return ret;
}

void setup_sprites_fepanel(void)
{
    LbSpriteSetup(fepanel_sprites, fepanel_sprites_end, fepanel_sprites_data);
}

void reset_sprites_fepanel(void)
{
    LbSpriteReset(fepanel_sprites, fepanel_sprites_end, fepanel_sprites_data);
}

TbResult load_pop_sprites_up_to(const char *dir, const char *name, short styleno, short max_detail)
{
    ubyte *loc_data_end = pop1_data_end;
    return load_any_sprites_up_to(dir, name, 150, pop1_sprites, pop1_sprites_end,
      pop1_data, &loc_data_end, &pop1_sprites_scale, styleno, max_detail);
}

void setup_pop_sprites(void)
{
    LbSpriteSetup(pop1_sprites, pop1_sprites_end, pop1_data);
}

void reset_pop_sprites(void)
{
    LbSpriteReset(pop1_sprites, pop1_sprites_end, pop1_data);
}

/******************************************************************************/
