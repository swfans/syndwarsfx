/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file guitext.c
 *     GUI text for menu and in-game display.
 * @par Purpose:
 *     Implement functions for handling text for game GUI, both in-game
 *     and in menu.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2023 - 22 Oct 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "guitext.h"

#include <string.h>
#include "bffile.h"
#include "bfmemory.h"
#include "bfmemut.h"

#include "cybmod.h"
#include "display.h"
#include "game_data.h"
#include "mydraw.h"
#include "swlog.h"
#include "wadfile.h"
#include "weapon.h"
/******************************************************************************/
char *gui_strings_data = NULL;
char *gui_strings_data_end = NULL;

char *weapon_text = NULL;
ushort weapon_text_index[WEP_TYPES_COUNT] = {0};
ushort cybmod_text_index[MOD_TYPES_COUNT] = {0};

u32 text_buf_pos = 0;

void snprint_dh_time_duration(char *out, ulong outlen, long ndays, short nhours)
{
    ulong days_strid;
    ulong hours_strid;
    if (ndays == 1) {
        days_strid = 625;
    } else {
        days_strid = 626;
    }
    if (nhours == 1) {
        hours_strid = 627;
    } else {
        hours_strid = 628;
    }

    snprintf(out, outlen, "%ld %s %hd %s", ndays, gui_strings[days_strid], nhours, gui_strings[hours_strid]);
}

const char *loctext_to_gtext(const char *ltext)
{
    char *gtext;
    u32 len;

    len = strlen(ltext) + 1;
    gtext = (char *)back_buffer + text_buf_pos;
    text_buf_pos += len;
    LbMemoryCopy(gtext, ltext, len);

    return gtext;
}

/** Read file with all the language-specific texts.
 */
void read_strings_file(void)
{
    char datfname[DISKPATH_SIZE];
    PathInfo *pinfo;
    int text_len;

    pinfo = &game_dirs[DirPlace_LangData];
    snprintf(datfname, DISKPATH_SIZE, "%s/guitext.dat", pinfo->directory);

    text_len = LbFileLength(datfname);
    if (text_len > 0) {
        gui_strings_data = (char *)LbMemoryAlloc(text_len);
        gui_strings_data[text_len - 1] = '\0';
    } else {
        gui_strings_data = NULL;
    }
    if (gui_strings_data != NULL) {
        gui_strings_data_end = gui_strings_data + text_len;
        LbFileLoadAt(datfname, gui_strings_data);
    } else {
        gui_strings_data_end = gui_strings_data;
        LOGERR("Failed checking strings file '%s', size=%d", datfname, text_len);
    }
}

TbBool create_strings_list(char **strings, char *strings_data, char *strings_data_end)
{
  int text_idx;
  char *text_ptr;
  char **text_arr;
  text_arr = strings;
  text_idx = STRINGS_MAX;
  text_ptr = strings_data;
  while (text_idx >= 0)
  {
      if (text_ptr >= strings_data_end) {
          break;
      }
      *text_arr = text_ptr;
      text_arr++;
      char chr_prev;
      do {
          chr_prev = *text_ptr;
          text_ptr++;
      } while ((chr_prev != '\0') && (text_ptr < strings_data_end));
      text_idx--;
  }
  if (text_idx < 0) {
      LOGERR("Overflow on listing text strings; something got corrupted (%d bad writes)", -text_idx);
      return false;
  }
  if (text_idx >= STRINGS_MAX) {
      LOGERR("Listing strings found no text entries");
      return false;
  }
  LOGSYNC("Listed text strings (%d entries)", STRINGS_MAX - text_idx);
  return true;
}

const char *weapon_description_text(ushort wtype)
{
    if (wtype >= WEP_TYPES_COUNT)
        return "";
    return &weapon_text[weapon_text_index[wtype]];
}

const char *cybmod_description_text(ushort mtype)
{
    if (mtype >= MOD_TYPES_COUNT)
        return "";
    return &weapon_text[cybmod_text_index[mtype]];
}

void init_weapon_text(void)
{
    char locstr[512];
    int weptxt_pos;
    int totlen;
    char *s;
    int i, n;

    totlen = load_file_alltext("textdata/wms.txt", weapon_text);
    if (totlen == Lb_FAIL)
        return;
    if (totlen >= weapon_text_len) {
        LOGERR("Insufficient memory for weapon_text - %d instead of %d", weapon_text_len, totlen);
        totlen = weapon_text_len - 1;
    }

    // TODO change the format to use our INI parser
    s = weapon_text;
    weapon_text[totlen] = '\0';

    for (i = 0; i < WEP_TYPES_COUNT; i++) {
        weapon_text_index[i] = totlen;
    }
    for (i = 0; i < MOD_TYPES_COUNT; i++) {
        cybmod_text_index[i] = totlen;
    }

    // section_start = s;
    weptxt_pos = 0;

    s = strchr(s, '[');
    s++;
    s = strchr(s, ']'); // position at start of WEAPONS section
    s++;

    s += 2;
    while (1)
    {
        if (*s == '[')
            break;

        // Read weapon name
        n = 0;
        while ((*s != '\r') && (*s != '\n'))
        {
            locstr[n] = *s++;
            n++;
        }
        locstr[n] = '\0';
        s += 2;

        // Recognize the weapon name
        for (i = 1; i < WEP_TYPES_COUNT; i++)
        {
            const char *codename;
            codename = weapon_codename(i);
            if (strcmp(codename, locstr) == 0) {
                break;
            }
        }
        if (i < WEP_TYPES_COUNT)
        {
            weapon_text_index[i] = weptxt_pos;

            while ((*s != '\r') && (*s != '\n')) {
                weapon_text[weptxt_pos] = *s++;
                weptxt_pos++;
            }
            weapon_text[weptxt_pos] = '\0';
            weptxt_pos++;
            s += 2;

            n = weapon_text_index[i];
            my_preprocess_text(&weapon_text[n]);
        } else {
            LOGERR("Weapon name not recognized: \"%s\"", locstr);
            if (s) s = strpbrk(s, "\r\n");
            if (s) s += 2;
        }
    }

    s = strchr(s, '[');
    s++;
    s = strchr(s, ']'); // position at start of MODS section
    s++;

    s += 2;
    while (1)
    {
        if ((*s == '[') || (*s == '\0'))
            break;

        // Read mod name
        n = 0;
        while ((*s != '\r') && (*s != '\n') && (*s != '\0'))
        {
            locstr[n] = *s++;
            n++;
        }
        locstr[n] = '\0';
        s += 2;

        for (i = 1; i < MOD_TYPES_COUNT; i++)
        {
            const char *codename;
            codename = cybmod_codename(i);
            if (strcmp(codename, locstr) == 0) {
                break;
            }
        }
        if (i < MOD_TYPES_COUNT)
        {
            cybmod_text_index[i] = weptxt_pos;

            while ((*s != '\r') && (*s != '\n') && (*s != '\0')) {
                weapon_text[weptxt_pos] = *s++;
                weptxt_pos++;
            }
            weapon_text[weptxt_pos] = '\0';
            weptxt_pos++;
            s += 2;

            n = cybmod_text_index[i];
            my_preprocess_text(&weapon_text[n]);
        } else {
            LOGERR("Cyb Mod name not recognized: \"%s\"", locstr);
            if (s) s = strpbrk(s, "\r\n");
            if (s) s += 2;
        }
    }
}
/******************************************************************************/
