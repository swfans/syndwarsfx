/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file purpldrw.c
 *     Adding items to drawlist for purple projector screens.
 * @par Purpose:
 *     Implements functions for filling items within drawlist.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     22 Apr 2024 - 28 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "purpldrw.h"

#include "bfconfig.h"
#include "bfutility.h"
#include "bfmemut.h"
#include "bfscreen.h"
#include "bfsprite.h"
#include "bftext.h"
#include "poly.h"
#include "bfjoyst.h"
#include "ssampply.h"

#include "display.h"
#include "femain.h"
#include "game.h"
#include "game_sprts.h"
#include "guiboxes.h"
#include "guitext.h"
#include "keyboard.h"
#include "mydraw.h"
#include "purpldrwlst.h"
#include "sound.h"
#include "swlog.h"
/******************************************************************************/
enum ScrollBarBoxes {
    ScrlBB_AreaAbv = 0,	/**< empty area above handle */
    ScrlBB_HndlBox,	/**< draggable handle box */
    ScrlBB_AreaBlw,	/**< empty area below handle */
    ScrlBB_ArrowUp,	/**< up arrow at bottom */
    ScrlBB_ArrowDn,	/**< down arrow at bottom */
    ScrlBB_BoxesCount,
};

#define FLASHY_DRAW_REACH_TURNS 24
#define FLASHY_DRAW_OUTLN_TURNS 48

// TODO avoid referring to a specific box
extern struct ScreenTextBox brief_netscan_box;
extern struct ScreenTextBox world_city_info_box;
extern struct ScreenTextBox equip_display_box;
extern struct ScreenTextBox cryo_cybmod_list_box;
extern struct ScreenTextBox controls_list_box;

s32 purple_box_x1, purple_box_y1;
s32 purple_box_x2, purple_box_y2;

extern s32 dword_1DC50C, dword_1DC510;
extern s32 dword_1DC534, dword_1DC538;

static TbBool button_input_suppressed_by_modal_box(struct ScreenButton *p_btn)
{
    if (show_alert && button_is_modal_alert(p_btn))
        return true;
    return false;
}

void draw_line_purple_list(int x1, int y1, int x2, int y2, int colour)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Line.X1 = lbDisplay.GraphicsWindowX + x1;
    p_pditem->U.Line.Y1 = lbDisplay.GraphicsWindowY + y1;
    p_pditem->U.Line.X2 = lbDisplay.GraphicsWindowX + x2;
    p_pditem->U.Line.Y2 = lbDisplay.GraphicsWindowY + y2;
    p_pditem->U.Line.Colour = colour;
    p_pditem->Flags = lbDisplay.DrawFlags;

    if ((x1 == x2) || (y1 == y2))
        p_pditem->Type = PuDT_HVLINE;
    else
        p_pditem->Type = PuDT_LINE;
}

void draw_box_purple_list(int x, int y, uint width, uint height, TbPixel colour)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Box.X = lbDisplay.GraphicsWindowX + x;
    p_pditem->U.Box.Y = lbDisplay.GraphicsWindowY + y;
    p_pditem->U.Box.Width = width;
    p_pditem->U.Box.Colour = colour;
    p_pditem->U.Box.Height = height;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->Type = PuDT_BOX;
}

void draw_text_purple_list2(int x, int y, const char *text, ushort line)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Text.X = x;
    p_pditem->U.Text.Y = y;
    p_pditem->U.Text.WindowX = text_window_x1;
    p_pditem->U.Text.WindowY = text_window_y1;
    p_pditem->U.Text.Width = text_window_x2 - text_window_x1 + 1;
    p_pditem->U.Text.Height = text_window_y2 - text_window_y1 + 1;
    p_pditem->U.Text.Text = text;
    p_pditem->U.Text.Line = line;
    p_pditem->U.Text.Colour = lbDisplay.DrawColour;
    p_pditem->U.Text.Font = lbFontPtr;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->Type = PuDT_TEXT;
}

void draw_sprite_purple_list(int x, int y, const struct TbSprite *p_sprite)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Sprite.Sprite = p_sprite;
    p_pditem->U.Sprite.Colour = lbDisplay.DrawColour;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->U.Sprite.X = lbDisplay.GraphicsWindowX + x;
    p_pditem->U.Sprite.Y = lbDisplay.GraphicsWindowY + y;
    p_pditem->Type = PuDT_SPRITE;
}

void draw_trig_purple_list(int x2, int y2, int x3, int y3)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Line.Colour = vec_colour;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->U.Line.X1 = lbDisplay.GraphicsWindowX + x2;
    p_pditem->U.Line.Y1 = lbDisplay.GraphicsWindowY + y2;
    p_pditem->U.Line.X2 = lbDisplay.GraphicsWindowX + x3;
    p_pditem->U.Line.Y2 = lbDisplay.GraphicsWindowY + y3;
    p_pditem->Type = PuDT_HOLORAY;
}

void copy_box_purple_list(int x, int y, uint width, uint height)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Box.Colour = 0;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->U.Box.X = lbDisplay.GraphicsWindowX + x;
    p_pditem->U.Box.Y = lbDisplay.GraphicsWindowY + y;
    p_pditem->U.Box.Width = width;
    p_pditem->U.Box.Height = height;
    p_pditem->Type = PuDT_COPYBOX;
}

void draw_hotspot_purple_list(int x, int y)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Hotspot.X = lbDisplay.GraphicsWindowX + x;
    p_pditem->U.Hotspot.Y = lbDisplay.GraphicsWindowY + y;
    p_pditem->Type = PuDT_HOTSPOT;
}

/** Draw flashy projector effect, part before vectors reach target point.
 *
 * We draw every shape using two brush vectors; this function draws both.
 * DrawFlags are ignored because this function does not actually draw the
 * target interface item; but the value is unaffected by this function.
 */
void flashy_draw_projector_reaching_target_coords(int scr1_x, int scr1_y,
  int scr2_x, int scr2_y, short advance, short tot_len, TbPixel colour)
{
    short reach1_x, reach1_y;
    short reach2_x, reach2_y;
    ushort bkp_drwflags;

    bkp_drwflags = lbDisplay.DrawFlags;
    reach1_x = proj_origin.X + (advance * (scr1_x - proj_origin.X)) / tot_len;
    reach1_y = proj_origin.Y + (advance * (scr1_y - proj_origin.Y)) / tot_len;
    reach2_x = proj_origin.X + (advance * (scr2_x - proj_origin.X)) / tot_len;
    reach2_y = proj_origin.Y + (advance * (scr2_y - proj_origin.Y)) / tot_len;

    lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
    draw_line_purple_list(proj_origin.X, proj_origin.Y, reach1_x, reach1_y, colour);
    draw_line_purple_list(proj_origin.X, proj_origin.Y, reach2_x, reach2_y, colour);

    lbDisplay.DrawFlags = 0;
    {
        struct TbSprite *p_spr;
        ushort spr;

        spr = 11 + (colour != 247);

        p_spr = &fe_mouseptr_sprites[spr];
        draw_sprite_purple_list(reach1_x - 1, reach1_y - 1, p_spr);
        draw_sprite_purple_list(reach2_x - 1, reach2_y - 1, p_spr);
    }
    lbDisplay.DrawFlags = bkp_drwflags;
}

void flashy_draw_projector_reaching_target_box_coords(int box_x, int box_y,
  int box_w, int box_h, short advance, TbPixel colour)
{
    flashy_draw_projector_reaching_target_coords(box_x, box_y,
      box_x + box_w, box_y + box_h, advance, FLASHY_DRAW_REACH_TURNS, colour);
}

/** Draw flashy projector effect, a single line being drawn.
 *
 * DrawFlags are only honored for target interface line; brush vectors
 * are drawn ignoring DrawFlags, as they are not target interface items.
 */
void flashy_draw_projector_outline_partial_line(int scr1_x, int scr1_y,
  int scr2_x, int scr2_y, short advance, short tot_len, TbPixel colour)
{
    int progr_x, progr_y;
    int scr3_x, scr3_y;
    ushort bkp_drwflags;

    bkp_drwflags = lbDisplay.DrawFlags;
    progr_x = advance * (scr2_x - scr1_x) / tot_len;
    progr_y = advance * (scr2_y - scr1_y) / tot_len;
    scr3_x = scr1_x + progr_x;
    scr3_y = scr1_y + progr_y;

    // Draw part of the final result line
    if (progr_x != 0 || progr_y != 0)
    {
        int aftim1_x, aftim1_y;
        int aftim2_x, aftim2_y;

        aftim1_x = scr3_x;
        aftim1_y = scr3_y;
        if (progr_x <= 0)
            aftim2_x = min(aftim1_x + 15, scr1_x);
        else
            aftim2_x = max(aftim1_x - 15, scr1_x);
        if (progr_y <= 0)
            aftim2_y = min(aftim1_y + 15, scr1_y);
        else
            aftim2_y = max(aftim1_y - 15, scr1_y);

        vec_colour = colour;
        draw_trig_purple_list(aftim1_x, aftim1_y, aftim2_x, aftim2_y);
        draw_line_purple_list(scr1_x, scr1_y,
          scr3_x, scr3_y, colour);
    }
    // Draw the temporary projector line and dot
    {
        struct TbSprite *p_spr;
        ushort spr;

        spr = 11 + (colour != 247);

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_line_purple_list(proj_origin.X, proj_origin.Y,
          scr3_x, scr3_y, colour);

        lbDisplay.DrawFlags = 0;
        p_spr = &fe_mouseptr_sprites[spr];
        draw_sprite_purple_list(scr3_x - 1, scr3_y - 1, p_spr);
    }
    lbDisplay.DrawFlags = bkp_drwflags;
}

void flashy_draw_projector_box_horizontal_lines(short box_x, short box_y,
  short box_w, short box_h, short advance, short delta_p, TbPixel colour)
{
    flashy_draw_projector_outline_partial_line(box_x, box_y,
      box_x + box_w - delta_p, box_y,
      advance, FLASHY_DRAW_OUTLN_TURNS/2, colour);

    flashy_draw_projector_outline_partial_line(box_x + box_w, box_y + box_h,
      box_x, box_y + box_h,
      advance, FLASHY_DRAW_OUTLN_TURNS/2, colour);
}

void flashy_draw_projector_box_vertical_lines(short box_x, short box_y,
  short box_w, short box_h, short advance, short delta_p, TbPixel colour)
{
    // when vertical lines are drawn, horizonal are already there
    draw_line_purple_list(box_x, box_y + box_h, box_x + box_w, box_y + box_h, colour);
    if (delta_p != 0) {
        draw_line_purple_list(box_x, box_y, box_x + box_w - delta_p, box_y, colour);
        draw_line_purple_list(box_w + box_x - delta_p, box_y, box_w + box_x, box_y + delta_p, colour);
    } else {
        draw_line_purple_list(box_x, box_y, box_x + box_w, box_y, colour);
    }

    flashy_draw_projector_outline_partial_line(box_x + box_w, box_y + delta_p,
      box_x + box_w, box_y + box_h,
      advance, FLASHY_DRAW_OUTLN_TURNS/2, colour);

    flashy_draw_projector_outline_partial_line(box_x, box_y + box_h,
      box_x, box_y,
      advance, FLASHY_DRAW_OUTLN_TURNS/2, colour);
}

ubyte flashy_draw_purple_shape(struct ScreenShape *p_shape)
{
    short i;
    ubyte pt1, pt2;

    pt1 = 0;
    pt2 = p_shape->PtLen >> 1;

    if ((p_shape->Flags & 0x02) != 0) {
        p_shape->Timer = -1;
        p_shape->Flags &= ~(0x80|0x02|0x01);
    }
    if ((p_shape->Flags & 0x01) != 0) {
        p_shape->Timer = 0;
        p_shape->Flags &= ~0x01;
    }

    // Handle sound
    if (p_shape->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        if (p_shape->Timer > FLASHY_DRAW_REACH_TURNS)
        {
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target shape
    if (p_shape->Timer <= FLASHY_DRAW_REACH_TURNS)
    {
        short advance;

        advance = p_shape->Timer;
        flashy_draw_projector_reaching_target_coords(p_shape->PtX[pt1], p_shape->PtY[pt1],
          p_shape->PtX[pt2], p_shape->PtY[pt2], advance, FLASHY_DRAW_REACH_TURNS, p_shape->Colour);
        p_shape->Timer += p_shape->DrawSpeed;
        return 0;
    }
    // Draw shape progressively appearing
    if (p_shape->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        int pnts_advance, frct_advance, frct_between_pts;
        ushort n;

        //TODO instead of dividing by points, we should divide by length - how many fractional
        //draws each line has should be determined from its length
        frct_between_pts = FLASHY_DRAW_OUTLN_TURNS / pt2;
        pnts_advance = (p_shape->Timer - FLASHY_DRAW_REACH_TURNS) / frct_between_pts;
        frct_advance = (p_shape->Timer - FLASHY_DRAW_REACH_TURNS) % frct_between_pts;
        vec_colour = p_shape->Colour;

        for (i = 0; i < pnts_advance; i++)
        {
            n = pt1 + i;
            draw_line_purple_list(p_shape->PtX[n+0], p_shape->PtY[n+0],
              p_shape->PtX[n+1], p_shape->PtY[n+1], p_shape->Colour);
            n = pt2 + i;
            draw_line_purple_list(p_shape->PtX[n+0], p_shape->PtY[n+0],
              p_shape->PtX[n+1], p_shape->PtY[n+1], p_shape->Colour);
        }

        n = pt1 + pnts_advance;
        flashy_draw_projector_outline_partial_line(p_shape->PtX[n+0], p_shape->PtY[n+0],
          p_shape->PtX[n+1], p_shape->PtY[n+1], frct_advance, frct_between_pts, p_shape->Colour);

        n = pt2 + pnts_advance;
        flashy_draw_projector_outline_partial_line(p_shape->PtX[n+0], p_shape->PtY[n+0],
          p_shape->PtX[n+1], p_shape->PtY[n+1], frct_advance, frct_between_pts, p_shape->Colour);

        p_shape->Timer += p_shape->DrawSpeed;
        if (p_shape->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
            return 2;
        else
            return 3;
    }

    for (i = 0; i < pt2 - pt1; i++)
    {
        ushort n;

        n = pt1 + i;
        draw_line_purple_list(p_shape->PtX[n+0], p_shape->PtY[n+0],
          p_shape->PtX[n+1], p_shape->PtY[n+1], p_shape->Colour);
        n = pt2 + i;
        draw_line_purple_list(p_shape->PtX[n+0], p_shape->PtY[n+0],
          p_shape->PtX[n+1], p_shape->PtY[n+1], p_shape->Colour);
    }
    return 3;
}

ubyte flashy_draw_purple_box(struct ScreenBox *p_box)
{
    short box_w, box_h;

    box_w = p_box->Width - 1;
    box_h = p_box->Height - 1;

    if ((p_box->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_box->Timer = -1;
        p_box->Timer2 = -1;
        p_box->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
    }
    if ((p_box->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_box->Timer2 = 0;
        p_box->Flags &= ~(GBxFlg_Unkn0001|GBxFlg_Unkn0080);
        p_box->Timer = p_box->Timer2;
        p_box->Flags |= GBxFlg_Unkn0080;
    }

    // Handle sound
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        if (p_box->Timer > FLASHY_DRAW_REACH_TURNS)
        {
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS)
    {
        short advance;

        advance = p_box->Timer;
        flashy_draw_projector_reaching_target_box_coords(p_box->X, p_box->Y,
          box_w, box_h, advance, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw horizontal lines
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS/2)
    {
        short advance;

        advance = p_box->Timer - FLASHY_DRAW_REACH_TURNS;
        flashy_draw_projector_box_horizontal_lines(p_box->X, p_box->Y,
          box_w, box_h, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        short advance;

        advance = p_box->Timer - FLASHY_DRAW_REACH_TURNS - FLASHY_DRAW_OUTLN_TURNS/2;
        flashy_draw_projector_box_vertical_lines(p_box->X, p_box->Y,
          box_w, box_h, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 2;
    }

    // Now as drawing animation finished, the background became static
    // Draw and store the static background
    if ((p_box->Flags & GBxFlg_BkCopied) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X - 3, p_box->Y - 3,
          p_box->Width + 6, p_box->Height + 6, p_box->BGColour);
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X, p_box->Y,
          p_box->Width, p_box->Height, p_box->Colour);
        lbDisplay.DrawFlags = 0;

        if ((p_box->Flags & GBxFlg_NoBkCopy) == 0)
        {
            copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
              p_box->Width + 6, p_box->Height + 6);
            p_box->Flags |= GBxFlg_BkCopied;
        }
    }
    if (p_box->SpecialDrawFn != NULL) {
          p_box->SpecialDrawFn(p_box);
    }
    return 3;
}

void my_set_text_window_for_text_box(struct ScreenTextBox *p_box)
{
    short scr_scroll_w, scr_scroll_h;

    if ((p_box->Flags & GBxFlg_RadioBtn) != 0)
        scr_scroll_w = 12;
    else
        scr_scroll_w = 0;

    if ((p_box->Buttons[0] != NULL) || (p_box->Infos[0] != NULL) || (p_box == &brief_netscan_box))
        scr_scroll_h = p_box->ScrollWindowHeight + 2;
    else
        scr_scroll_h = p_box->ScrollWindowHeight + 23;

    my_set_text_window(p_box->X + 4, p_box->Y + p_box->ScrollWindowOffset + 4,
      p_box->Width - 8 - scr_scroll_w, scr_scroll_h);
}

/** Get amount of lines within currently set text window.
 */
short get_text_box_window_lines_visible(struct ScreenTextBox *p_box)
{
    short text_window_h;
    text_window_h = text_window_y2 - text_window_y1 - 1;
    return (text_window_h + 2) / p_box->LineHeight;
}

/** Get amount of lines within given text box.
 * This function alters my_text_window and the current font.
 */
short get_text_box_lines_visible(struct ScreenTextBox *p_box)
{
    my_set_text_window_for_text_box(p_box);

    lbFontPtr = p_box->Font;
    if (p_box->LineHeight == 0)
        p_box->LineHeight = p_box->LineSpacing + my_char_height('A');

    return get_text_box_window_lines_visible(p_box);
}

TbBool flashy_draw_text(int x, int y, const char *text, ubyte speed, int top_line, short *textpos, int cyan_flag)
{
    char *gtext;
    int text_tot_len, chunk_end;
    int i, pos;
    sbyte chunk_no;

    if (text == NULL) {
        return 1;
    }
    text_tot_len = strlen(text);
    pos = *textpos;
    if (pos >= text_tot_len) {
        draw_text_purple_list2(x, y, text, top_line);
        return 1;
    }
    gtext = (char *)back_buffer + text_buf_pos;
    if (pos > 0)
        LbMemoryCopy(gtext, text, pos);
    if (pos <= 0)
        pos = 0;
    if (cyan_flag)
    {
        chunk_no = 12;
        gtext[pos] = 27;
        pos += 1;
        for (i = 0; i < 4; i++)
        {
            chunk_end = i + *textpos;
            if ((chunk_end >= 0) && (chunk_end < text_tot_len))
            {
              gtext[pos + 0] = 14;
              gtext[pos + 1] = chunk_no;
              gtext[pos + 2] = text[i + *textpos];
              pos += 3;
            }
            ++chunk_no;
        }
    }
    else
    {
        chunk_no = 6;
        gtext[pos] = 27;
        pos += 1;
        for (i = 0; i < 6; i++)
        {
            chunk_end = i + *textpos;
            if ((chunk_end >= 0) && (chunk_end < text_tot_len))
            {
              gtext[pos + 0] = 14;
              gtext[pos + 1] = chunk_no;
              gtext[pos + 2] = text[i + *textpos];
              pos += 3;
            }
            ++chunk_no;
        }
    }
    gtext[pos + 0] = 28;
    gtext[pos + 1] = 0;
    pos += 2;
    text_buf_pos += pos;

    *textpos += speed;
    draw_text_purple_list2(x, y, gtext, top_line);
    play_sample_using_heap(0, 114, 127, 64, 100, 0, 3);
    return 0;
}

ubyte flashy_draw_purple_text_box_text(struct ScreenTextBox *p_box)
{
    ubyte text_drawn;

    lbDisplay.DrawFlags = 0;
    if ((p_box->Flags & GBxFlg_TextRight) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_RIGHT;
    }
    else if ((p_box->Flags & GBxFlg_TextCenter) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    }

    text_drawn = true;

    if ((p_box->Flags & GBxFlg_TextCopied) != 0)
        return text_drawn;

    if (p_box->DrawTextFn != NULL)
    {
        ubyte drawn;
        drawn = p_box->DrawTextFn(p_box);
        text_drawn = text_drawn && drawn;
    }
    else if (p_box->Text != NULL)
    {
        ubyte drawn;
        drawn = flashy_draw_text(0,  0, p_box->Text, p_box->TextSpeed, p_box->TextTopLine,
              &p_box->TextFadePos, p_box->DrawTextFn != NULL);
        text_drawn = text_drawn && drawn;
    }

    if (text_drawn && ((p_box->Flags & GBxFlg_RadioBtn) == 0))
    {
          copy_box_purple_list(p_box->X - 3, p_box->Y - 3, p_box->Width + 6, p_box->Height + 6);
          p_box->Flags |= GBxFlg_TextCopied;
    }

    return text_drawn;
}

ubyte flashy_draw_purple_text_box_children(struct ScreenTextBox *p_box)
{
    ushort i;
    ubyte all_drawn;

    all_drawn = 1;
    lbDisplay.DrawFlags = 0;
    for (i = 0; i < 2; i++)
    {
        struct ScreenButton *p_button;
        struct ScreenInfoBox *p_info;

        p_button = p_box->Buttons[i];
        if (p_button != NULL) {
            ubyte drawn;
            drawn = p_button->DrawFn(p_button);
            all_drawn = all_drawn && drawn;
        }
        p_info = p_box->Infos[i];
        if (p_info != NULL) {
            ubyte drawn;
            drawn = p_info->DrawFn(p_info);
            all_drawn = all_drawn && drawn;
        }
    }
    return all_drawn;
}

void purple_text_box_set_scroll_vertical_pos(struct ScreenTextBox *p_box, short scroll_pos)
{
    if (scroll_pos < 0)
        scroll_pos = 0;
    else if (scroll_pos + p_box->ScrollBarSize > p_box->ScrollWindowHeight)
        scroll_pos = p_box->ScrollWindowHeight - p_box->ScrollBarSize;

    p_box->ScrollBarPos = scroll_pos;
    p_box->TextTopLine = p_box->Lines * p_box->ScrollBarPos / p_box->ScrollWindowHeight;
}

void init_scroll_bar_for_text_box(struct ScreenBox *ar_scroll_bar, struct ScreenTextBox *p_box)
{
    init_screen_box(&ar_scroll_bar[ScrlBB_AreaAbv], p_box->X + p_box->Width - 12,
      p_box->Y + p_box->ScrollWindowOffset + 5,
      6, p_box->ScrollBarPos, p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_HndlBox], p_box->X + p_box->Width - 12,
      p_box->Y + p_box->ScrollWindowOffset + 5 + p_box->ScrollBarPos,
      6, p_box->ScrollBarSize, p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_AreaBlw], p_box->X + p_box->Width - 12,
      p_box->Y + p_box->ScrollWindowOffset + 5 + p_box->ScrollBarPos + p_box->ScrollBarSize,
      6, p_box->ScrollWindowHeight + 1 - (p_box->ScrollBarPos + p_box->ScrollBarSize), p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_ArrowUp], p_box->X + p_box->Width - 13,
      p_box->Y + p_box->ScrollWindowHeight + p_box->ScrollWindowOffset + 9,
      8, 9, p_box->DrawSpeed);

    init_screen_box(&ar_scroll_bar[ScrlBB_ArrowDn], p_box->X + p_box->Width - 13,
      p_box->Y + p_box->ScrollWindowHeight + p_box->ScrollWindowOffset + 18,
      8, 9, p_box->DrawSpeed);
}

void input_purple_text_box_wth_scroll(struct ScreenTextBox *p_box, struct ScreenBox *ar_scroll_bar)
{
    short lines_visible;
    TbBool no_keyboard_inputs;

    lines_visible = get_text_box_window_lines_visible(p_box);
    //TODO make this into a flag within the box
    no_keyboard_inputs = (p_box == &controls_list_box);

    if (lbDisplay.MLeftButton && ((p_box->Flags & GBxFlg_IsPushed) != 0))
    {
        short scroll_pos;

        scroll_pos = mouse_move_position_vertical_scrollbar_over_text_box(p_box);
        purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) != 0) && (p_box->ScrollBarPos >= 0))
    {
        // Clicking the scroll bar area above the handle
        if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_AreaAbv]))
        {
            if (lbDisplay.LeftButton)
            {
                short scroll_pos;

                scroll_pos = p_box->ScrollBarPos - (lines_visible - 1) * p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }
        // Clicking the scroll bar area below the handle
        else if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_AreaBlw]))
        {
            if (lbDisplay.LeftButton)
            {
                short scroll_pos;

                scroll_pos = p_box->ScrollBarPos + (lines_visible - 1) * p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }

        // Input draging the scroll bar handle
        if (((p_box->Flags & GBxFlg_IsPushed) != 0) || mouse_move_over_box(&ar_scroll_bar[ScrlBB_HndlBox]))
        {
            if (lbDisplay.LeftButton)
            {
                int ms_y;
                lbDisplay.LeftButton = 0;
                p_box->Flags |= GBxFlg_IsPushed;
                ms_y = lbDisplay.ScreenMode == 1 ? 2 * lbDisplay.MouseY : lbDisplay.MouseY;
                p_box->GrabPos = ms_y - p_box->ScrollBarPos;
                play_sample_using_heap(0, 125, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
            }
        }

        if (mouse_move_over_box(p_box) && ((p_box->Flags & GBxFlg_IsPushed) == 0))
        {
            // Using keyboard to scroll
            if (no_keyboard_inputs)
            {
                // User requested disable - the same keys are probably used for something else
            }
            else if (is_key_pressed(KC_UP, KMod_DONTCARE) || is_key_pressed(KC_DOWN, KMod_DONTCARE) ||
              is_key_pressed(KC_PGUP, KMod_DONTCARE) || is_key_pressed(KC_PGDOWN, KMod_DONTCARE))
            {
                short scroll_pos, delta;

                scroll_pos = p_box->ScrollBarPos;

                delta = 0;
                if (is_key_pressed(KC_DOWN, KMod_DONTCARE))
                    delta = p_box->ScrollWindowHeight / p_box->Lines;
                else if (is_key_pressed(KC_UP, KMod_DONTCARE))
                    delta = - p_box->ScrollWindowHeight / p_box->Lines;
                scroll_pos += delta;

                delta = 0;
                if (is_key_pressed(KC_PGDOWN, KMod_DONTCARE))
                    delta = p_box->ScrollWindowHeight * (lines_visible - 1) / p_box->Lines;
                else if (is_key_pressed(KC_PGUP, KMod_DONTCARE))
                    delta = - (lines_visible - 1) * p_box->ScrollWindowHeight / p_box->Lines;
                scroll_pos += delta;

                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
#if defined(LB_ENABLE_MOUSE_WHEEL)
            // Using mouse wheel to scroll
            if (lbDisplay.WheelMoveUp || lbDisplay.WheelMoveDown)
            {
                short scroll_pos, delta;

                scroll_pos = p_box->ScrollBarPos;

                delta = 0;
                if (lbDisplay.WheelMoveDown) {
                    delta = p_box->ScrollWindowHeight / p_box->Lines * lbDisplay.WheelMoveDown;
                    lbDisplay.WheelMoveDown = 0;
                }
                else if (lbDisplay.WheelMoveUp) {
                    delta = - p_box->ScrollWindowHeight / p_box->Lines * lbDisplay.WheelMoveUp;
                    lbDisplay.WheelMoveUp = 0;
                }
                scroll_pos += delta;

                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
#endif
        }

        // Input from up arrow in bottom part of the scroll bar
        if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_ArrowUp]))
        {
            if (lbDisplay.MLeftButton || joy.Buttons[0])
            {
                short scroll_pos;

                if (lbDisplay.LeftButton)
                    play_sample_using_heap(0, 125, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
                lbDisplay.LeftButton = 0;
                p_box->Flags |= GBxFlg_IsRPushed;
                ar_scroll_bar[ScrlBB_ArrowUp].Flags |= GBxFlg_IsRPushed;

                scroll_pos = p_box->ScrollBarPos - p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }

        // Input from down arrow in bottom part of the scroll bar
        if (mouse_move_over_box(&ar_scroll_bar[ScrlBB_ArrowDn]))
        {
            if (lbDisplay.MLeftButton || joy.Buttons[0])
            {
                short scroll_pos;

                if (lbDisplay.LeftButton)
                    play_sample_using_heap(0, 125, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1);
                lbDisplay.LeftButton = 0;
                p_box->Flags |= GBxFlg_IsRPushed;
                ar_scroll_bar[ScrlBB_ArrowDn].Flags |= GBxFlg_IsRPushed;

                scroll_pos = p_box->ScrollBarPos + p_box->ScrollWindowHeight / p_box->Lines;
                purple_text_box_set_scroll_vertical_pos(p_box, scroll_pos);
            }
        }
    }

    if (!lbDisplay.MLeftButton && !joy.Buttons[0])
        p_box->Flags &= ~(GBxFlg_IsRPushed|GBxFlg_IsPushed);
}

ubyte flashy_draw_purple_text_box(struct ScreenTextBox *p_box)
{
    short box_w, box_h;
    short lines_visible;
    // text drawing callback never sets GBxFlg_TextCopied (maybe make this into a box flag?)
    TbBool text_remains_dynamic;

    text_remains_dynamic = (p_box == &world_city_info_box) ||
      (p_box == &equip_display_box) || (p_box == &cryo_cybmod_list_box);
    my_set_text_window_for_text_box(p_box);

    lbFontPtr = p_box->Font;
    byte_197160 = p_box->LineSpacing;
    if (p_box->LineHeight == 0)
        p_box->LineHeight = byte_197160 + my_char_height('A');

    lines_visible = get_text_box_window_lines_visible(p_box);

    box_w = p_box->Width - 1;
    box_h = p_box->Height - 1;

    if ((p_box->Flags & GBxFlg_Unkn0001) != 0)
    {
        int i;

        p_box->Timer = 0;
        p_box->Flags &= ~(GBxFlg_Unkn0001|GBxFlg_Unkn0080);
        p_box->Flags |= GBxFlg_Unkn0080;

        for (i = 0; i < 2; i++)
        {
            struct ScreenButton *p_button;
            struct ScreenInfoBox *p_info;

            p_button = p_box->Buttons[i];
            if (p_button != NULL)
                p_button->Flags |= GBxFlg_Unkn0001;

            p_info = p_box->Infos[i];
            if (p_info != NULL)
                p_info->Flags |= GBxFlg_Unkn0001;
        }
    }

    if ((p_box->Flags & GBxFlg_Unkn0002) != 0)
    {
        const char *p_text;
        int i;

        p_box->Flags &= ~(GBxFlg_Unkn0002|GBxFlg_Unkn0001);
        p_box->Timer = -1;

        p_text = p_box->Text;
        if (p_text != NULL)
            p_box->TextFadePos = my_str_len(p_text);
        else
            p_box->TextFadePos = -5;

        for (i = 0; i < 2; i++)
        {
            struct ScreenButton *p_button;
            struct ScreenInfoBox *p_info;

            p_button = p_box->Buttons[i];
            if (p_button != NULL)
                p_button->Flags |= GBxFlg_Unkn0002;

            p_info = p_box->Infos[i];
            if (p_info != NULL)
                p_info->Flags |= GBxFlg_Unkn0002;
        }
    }

    // Handle sound
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        if (p_box->Timer > FLASHY_DRAW_REACH_TURNS)
        {
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS)
    {
        short advance;

        advance = p_box->Timer;
        flashy_draw_projector_reaching_target_box_coords(p_box->X, p_box->Y,
          box_w, box_h, advance, p_box->Colour1);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw horizontal lines
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS/2)
    {
        short advance;

        advance = p_box->Timer - FLASHY_DRAW_REACH_TURNS;
        flashy_draw_projector_box_horizontal_lines(p_box->X, p_box->Y,
          box_w, box_h, advance, 0, p_box->Colour1);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        short advance;

        advance = p_box->Timer - FLASHY_DRAW_REACH_TURNS - FLASHY_DRAW_OUTLN_TURNS/2;
        flashy_draw_projector_box_vertical_lines(p_box->X, p_box->Y,
          box_w, box_h, advance, 0, p_box->Colour1);
        p_box->Timer += p_box->DrawSpeed;
        return 2;
    }

    if ((p_box->Flags & GBxFlg_RadioBtn) == 0)
    {
        if ((p_box->Flags & GBxFlg_Unkn0080) != 0) {
            p_box->TextFadePos = -5;
            p_box->Flags &= ~GBxFlg_Unkn0080;
        }
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) != 0) && ((p_box->Flags & GBxFlg_Unkn0080) != 0))
    {
        if (p_box->Lines == 0) {
            p_box->Lines = my_count_lines(p_box->Text);
        }
        p_box->ScrollBarSize = p_box->ScrollWindowHeight * lines_visible / p_box->Lines;
        if ((p_box->ScrollBarSize < p_box->ScrollWindowHeight) ||
          ((p_box->Flags & GBxFlg_IsMouseOver) != 0))
        {
            if (p_box->ScrollBarSize >= p_box->ScrollWindowHeight)
                p_box->ScrollBarSize = p_box->ScrollWindowHeight;
            p_box->ScrollBarPos = 0;
        }
        else
        {
            p_box->Flags &= ~GBxFlg_RadioBtn;
        }
        p_box->TextTopLine = 0;
        p_box->Flags &= ~(GBxFlg_IsPushed|GBxFlg_Unkn0080);
        if (p_box->Timer != 255) {
            p_box->TextFadePos = -5;
        }
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) == 0) &&
      (((p_box->Flags & GBxFlg_BkCopied) != 0) || ((p_box->Flags & GBxFlg_NoBkCopy) != 0)))
    {
        // If childern drawing has also finished, it is safe to cap the scrollbar hole
        if (((p_box->Flags & GBxFlg_TextCopied) != 0) || text_remains_dynamic)
        {
            if ((p_box->Width > 13) && (p_box->ScrollWindowHeight > 0) && (p_box->ScrollWindowOffset >= 0))
            {
                lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
                draw_box_purple_list(p_box->X + p_box->Width - 13,
                  p_box->Y + p_box->ScrollWindowOffset + 4,
                  8, p_box->ScrollWindowHeight + 2,
                  p_box->BGColour);
            }
        }
        flashy_draw_purple_text_box_text(p_box);
        flashy_draw_purple_text_box_children(p_box);
        return 3;
    }

    struct ScreenBox scroll_bar[ScrlBB_BoxesCount];

    init_scroll_bar_for_text_box(scroll_bar, p_box);

    input_purple_text_box_wth_scroll(p_box, scroll_bar);

    // Now as drawing animation finished, the background became static
    // Draw and store the static background
    if ((p_box->Flags & GBxFlg_BkCopied) == 0)
    {
        short w, h;
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        // main area, left of scrollbar
        w = max(p_box->Width - 16, 0);
        draw_box_purple_list(p_box->X - 3, p_box->Y - 3,
          w + 6, p_box->Height + 6, p_box->BGColour);
        // strip on right of scrollbar
        if (p_box->Width >= 5)
            draw_box_purple_list(p_box->X + p_box->Width - 5,  p_box->Y - 3,
              8, p_box->Height + 6, p_box->BGColour);
        if (p_box->Width > 13)
        {
            // above the scrollbar
            draw_box_purple_list(p_box->X + p_box->Width - 13, p_box->Y - 3,
              8, p_box->ScrollWindowOffset + 7, p_box->BGColour);
            // below the scrollbar
            h = max(p_box->ScrollWindowHeight + 3 + p_box->ScrollWindowOffset, 1);
            if (p_box->Height > h)
                draw_box_purple_list(p_box->X + p_box->Width - 13,
                  p_box->Y + h + 3,
                  8, p_box->Height - h,
                  p_box->BGColour);
        }
        // box outline
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X, p_box->Y, p_box->Width, p_box->Height, p_box->Colour1);
        lbDisplay.DrawFlags = 0;

        if ((p_box->Flags & GBxFlg_NoBkCopy) == 0)
        {
            copy_box_purple_list(p_box->X - 3, p_box->Y - 3,
              p_box->Width + 6, p_box->Height + 6);
            p_box->Flags |= GBxFlg_BkCopied;
        }
    }

    if (((p_box->Flags & GBxFlg_RadioBtn) != 0) && (p_box->ScrollBarPos >= 0))
    {
        struct TbSprite *p_spr;

        // Draw the scroll bar handle
        draw_box_purple_list(scroll_bar[ScrlBB_HndlBox].X, scroll_bar[ScrlBB_HndlBox].Y,
          scroll_bar[ScrlBB_HndlBox].Width, scroll_bar[ScrlBB_HndlBox].Height, 174);

        // Draw scroll arrows
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((scroll_bar[ScrlBB_ArrowUp].Flags & GBxFlg_IsRPushed) != 0)
            lbDisplay.DrawFlags = 0;
        if (p_box->Text != NULL)
            lbDisplay.DrawFlags |= 0x8000;

        p_spr = &fe_mouseptr_sprites[9];
        draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowUp].X, scroll_bar[ScrlBB_ArrowUp].Y, p_spr);

        lbDisplay.DrawFlags = 0;

        if (mouse_move_over_box(&scroll_bar[ScrlBB_ArrowUp]))
        {
            p_spr = &fe_mouseptr_sprites[13];
            draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowUp].X, scroll_bar[ScrlBB_ArrowUp].Y, p_spr);
        }

        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((scroll_bar[ScrlBB_ArrowDn].Flags & GBxFlg_IsRPushed) != 0)
            lbDisplay.DrawFlags = 0;
        if (p_box->Text != NULL)
            lbDisplay.DrawFlags |= 0x8000;

        p_spr = &fe_mouseptr_sprites[10];
        draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowDn].X, scroll_bar[ScrlBB_ArrowDn].Y, p_spr);

        lbDisplay.DrawFlags = 0;

        if (mouse_move_over_box(&scroll_bar[ScrlBB_ArrowDn]))
        {
            p_spr = &fe_mouseptr_sprites[14];
            draw_sprite_purple_list(scroll_bar[ScrlBB_ArrowDn].X, scroll_bar[ScrlBB_ArrowDn].Y, p_spr);
        }
    }

    flashy_draw_purple_text_box_text(p_box);
    flashy_draw_purple_text_box_children(p_box);
    return 3;
}

ubyte flashy_draw_purple_info_box(struct ScreenInfoBox *p_box)
{
    short box_w, box_h;

    box_w = p_box->Width - 1;
    box_h = p_box->Height - 1;

    if ((p_box->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_box->Timer = -1;
        p_box->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
    }
    if ((p_box->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_box->Timer = 0;
        p_box->Flags &= ~(GBxFlg_Unkn0001|GBxFlg_Unkn0080);
        p_box->Flags |= GBxFlg_Unkn0080;
    }

    // Handle sound
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        if (p_box->Timer > FLASHY_DRAW_REACH_TURNS)
        {
            // Projector drawing sound
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS)
    {
        short advance;

        advance = p_box->Timer;
        flashy_draw_projector_reaching_target_box_coords(p_box->X, p_box->Y,
          box_w, box_h, advance, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw horizontal lines
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS/2)
    {
        short advance;

        advance = p_box->Timer - FLASHY_DRAW_REACH_TURNS;
        flashy_draw_projector_box_horizontal_lines(p_box->X, p_box->Y,
          box_w, box_h, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_box->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        short advance;

        advance = p_box->Timer - FLASHY_DRAW_REACH_TURNS - FLASHY_DRAW_OUTLN_TURNS/2;
        flashy_draw_projector_box_vertical_lines(p_box->X, p_box->Y,
          box_w, box_h, advance, 0, p_box->Colour);
        p_box->Timer += p_box->DrawSpeed;
        return 2;
    }

    // Now as drawing animation finished, the background became static
    // Draw and store the static background
    if ((p_box->Flags & GBxFlg_BkCopied) == 0)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        draw_box_purple_list(p_box->X - 1, p_box->Y - 1,
          p_box->Width + 2, p_box->Height + 2, p_box->BGColour);
        lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
        draw_box_purple_list(p_box->X, p_box->Y,
          p_box->Width, p_box->Height, p_box->Colour);
        lbDisplay.DrawFlags = 0;

        if ((p_box->Flags & GBxFlg_NoBkCopy) == 0)
        {
            copy_box_purple_list(p_box->X - 1, p_box->Y - 1,
              p_box->Width + 2, p_box->Height + 2);
            p_box->Flags |= GBxFlg_BkCopied;
        }
    }
    if (p_box->DrawTextFn != NULL) {
        p_box->DrawTextFn(p_box);
    }
    return 3;
}

ubyte flashy_draw_purple_button(struct ScreenButton *p_btn)
{
    short box_w, box_h;
    TbKeyCode akey;
    TbBool mouse_over, event_from_key;

    my_set_text_window(p_btn->X + 2, p_btn->Y + 2, p_btn->Width - 18, p_btn->Height - 4);
    lbFontPtr = p_btn->Font;
    box_w = p_btn->Width - 1;
    box_h = p_btn->Height - 1;

    if ((p_btn->Flags & GBxFlg_Unkn0002) != 0)
    {
        p_btn->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0002|GBxFlg_Unkn0001);
        p_btn->Timer = -1;
        if (p_btn->Text != NULL)
            p_btn->TextFadePos = strlen(p_btn->Text);
        else
            p_btn->TextFadePos = -5;
    }
    if ((p_btn->Flags & GBxFlg_Unkn0001) != 0)
    {
        p_btn->Flags &= ~(GBxFlg_Unkn0080|GBxFlg_Unkn0001);
        p_btn->Timer = 0;
        p_btn->Flags |= GBxFlg_Unkn0080;
    }

    // Handle sound
    if (p_btn->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        if (p_btn->Timer > FLASHY_DRAW_REACH_TURNS)
        {
            // Projector drawing sound
            if (!IsSamplePlaying(0, 4, 0))
                play_sample_using_heap(0, 110, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
        }
    }

    // Draw lines from projector origin reaching target rectangle
    if (p_btn->Timer <= FLASHY_DRAW_REACH_TURNS)
    {
        short advance;

        advance = p_btn->Timer;
        flashy_draw_projector_reaching_target_box_coords(p_btn->X, p_btn->Y,
          box_w, box_h, advance, p_btn->Colour);
        p_btn->Timer += p_btn->DrawSpeed;
        return 0;
    }

    if (!mouse_move_over_box(p_btn))
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;

    // Draw horizontal lines
    if (p_btn->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS/2)
    {
        short advance, delta_p;

        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
            delta_p = 5;
        else
            delta_p = 0;
        advance = p_btn->Timer - FLASHY_DRAW_REACH_TURNS;
        flashy_draw_projector_box_horizontal_lines(p_btn->X, p_btn->Y,
          box_w, box_h, advance, delta_p, p_btn->Colour);
        p_btn->Timer += p_btn->DrawSpeed;
        return 0;
    }

    // Draw vertical lines
    if (p_btn->Timer <= FLASHY_DRAW_REACH_TURNS + FLASHY_DRAW_OUTLN_TURNS)
    {
        short advance, delta_p;

        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
            delta_p = 5;
        else
            delta_p = 0;
        advance = p_btn->Timer - FLASHY_DRAW_REACH_TURNS - FLASHY_DRAW_OUTLN_TURNS/2;
        flashy_draw_projector_box_vertical_lines(p_btn->X, p_btn->Y,
          box_w, box_h, advance, delta_p, p_btn->Colour);
        p_btn->Timer += p_btn->DrawSpeed;
        return 2;
    }

    // Mouse input
    if (mouse_down_over_box(p_btn))
    {
        if (lbDisplay.LeftButton)
        {
            lbDisplay.LeftButton = 0;
            p_btn->Flags |= GBxFlg_IsPushed;
        }
        if (lbDisplay.RightButton)
        {
            lbDisplay.RightButton = 0;
            p_btn->Flags |= GBxFlg_IsRPushed;
        }
    }
    event_from_key = false;
    akey = p_btn->AccelKey  & 0xFF;
    if ((akey == KC_RETURN || akey == KC_ESCAPE) &&
      is_key_pressed(akey, KMod_DONTCARE))
    {
        clear_key_pressed(akey);
        p_btn->Flags |= GBxFlg_IsPushed;
        event_from_key = true;
    } else
    if (is_key_pressed(akey, KMod_ALT))
    {
        clear_key_pressed(akey);
        event_from_key = true;
        p_btn->Flags |= GBxFlg_IsPushed;
    } else
    if (is_key_pressed(akey, KMod_CONTROL))
    {
        clear_key_pressed(akey);
        p_btn->Flags |= GBxFlg_IsRPushed;
        event_from_key = true;
    }

    mouse_over = mouse_move_over_box(p_btn);
    if (mouse_over)
    {
        if ((p_btn->Flags & GBxFlg_IsMouseOver) == 0)
        {
            // Mouse over sound
            play_sample_using_heap(0, 123, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 1u);
            p_btn->Flags |= GBxFlg_IsMouseOver;
        }
    }
    else
    {
        p_btn->Flags &= ~GBxFlg_IsMouseOver;

    }

    ubyte box_b;
    ubyte ret;

    ret = 3;
    box_b = p_btn->Border;

    if (!mouse_over && !event_from_key)
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((p_btn->Flags & GBxFlg_RadioBtn) != 0 && p_btn->RadioValue == *p_btn->Radio)
            lbDisplay.DrawFlags = 0;
        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
        {
            int v15, v16;

            v15 = box_b;
            v16 = box_w;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              box_b + (box_w) - 4, box_b + 5, p_btn->BGColour);
            lbDisplay.DrawFlags |= 0x8000;
            draw_box_purple_list(p_btn->X - v15, p_btn->Y + 5,
              box_w + 1 + 2 * v15, v15 + box_h - 4, p_btn->BGColour);
            lbDisplay.DrawFlags &= ~0x8000;
            draw_triangle_purple_list(v16 + p_btn->X - 4, p_btn->Y - v15,
              v16 + p_btn->X + 1 + v15, p_btn->Y + 5,
              v16 + p_btn->X - 4, p_btn->Y + 5, p_btn->BGColour);
            lbDisplay.DrawFlags |= Lb_SPRITE_OUTLINE;
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X, box_h + p_btn->Y,
              p_btn->X + box_w, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X + box_w - 5, p_btn->Y, p_btn->Colour);
            draw_line_purple_list(p_btn->X + v16, p_btn->Y + 5,
              p_btn->X + v16, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X + v16 - 5, p_btn->Y,
              p_btn->X + v16, p_btn->Y + 5, p_btn->Colour);
        }
        else
        {
            int v17, v18;
            lbDisplay.DrawFlags |= 0x8000;
            v17 = box_h + 1;
            v18 = box_w + 1;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              v18 + 2 * box_b, 2 * box_b + v17, p_btn->BGColour);
            lbDisplay.DrawFlags &= ~(Lb_SPRITE_OUTLINE|0x8000);
            lbDisplay.DrawFlags |= Lb_SPRITE_OUTLINE;
            draw_box_purple_list(p_btn->X, p_btn->Y, v18, v17, p_btn->Colour);
        }
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((p_btn->Flags & (GBxFlg_IsPushed|GBxFlg_IsRPushed)) != 0)
            p_btn->Flags &= ~(GBxFlg_IsPushed|GBxFlg_IsRPushed);
        if (((p_btn->Flags & GBxFlg_RadioBtn) != 0) && (p_btn->RadioValue == *p_btn->Radio))
            lbDisplay.DrawFlags = 0;
    }
    else
    {
        lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
        if ((p_btn->Flags & (GBxFlg_IsPushed|GBxFlg_IsRPushed)) != 0)
            lbDisplay.DrawFlags = 0;
        if ((p_btn->Flags & GBxFlg_RadioBtn) != 0 && p_btn->RadioValue == *p_btn->Radio)
            lbDisplay.DrawFlags = 0;
        if (lbDisplay.MLeftButton || joy.Buttons[0])
        {
            lbDisplay.LeftButton = 0;
            ret = 4;
            lbDisplay.DrawFlags = 0;
            p_btn->Flags |= GBxFlg_IsPushed;
        }
        else if ((p_btn->Flags & GBxFlg_IsPushed) != 0)
        {
            if (!button_input_suppressed_by_modal_box(p_btn))
            {
                ushort smpl_id;

                if (p_btn->CallBackFn != NULL)
                {
                    ubyte clicked;
                    clicked = p_btn->CallBackFn(0);
                    if (clicked)
                        smpl_id = 111;
                    else
                        smpl_id = 129;
                }
                else
                {
                    if ((p_btn->Flags & GBxFlg_RadioBtn) != 0)
                        // Click sound for do action button
                        smpl_id = 111;
                    else
                        // Click sound for cancel / abort button
                        smpl_id = 129;
                }
                play_sample_using_heap(0, smpl_id, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);

                if ((p_btn->Flags & GBxFlg_RadioBtn) != 0)
                    *p_btn->Radio = p_btn->RadioValue;
            }
            p_btn->Flags &= ~GBxFlg_IsPushed;
        }

        if (lbDisplay.MRightButton)
        {
            lbDisplay.RightButton = 0;
            ret = 5;
            lbDisplay.DrawFlags = 0;
            p_btn->Flags |= GBxFlg_IsRPushed;
        }
        else if ((p_btn->Flags & GBxFlg_IsRPushed) != 0)
        {
            if (!button_input_suppressed_by_modal_box(p_btn))
            {
                ushort smpl_id;

                if (p_btn->CallBackFn != NULL)
                {
                    ubyte clicked;
                    clicked = p_btn->CallBackFn(1);
                    if (clicked)
                        smpl_id = 111;
                    else
                        smpl_id = 129;
                }
                else
                {
                    // Click sound for cancel / abort button
                    smpl_id = 129;
                }
                play_sample_using_heap(0, smpl_id, FULL_VOL, EQUL_PAN, NORM_PTCH, LOOP_NO, 2u);

                if ((p_btn->Flags & GBxFlg_RadioBtn) != 0)
                  *p_btn->Radio = p_btn->RadioValue;
            }
            p_btn->Flags &= ~GBxFlg_IsRPushed;
        }
        if ((p_btn->Flags & GBxFlg_Unkn0010) != 0)
        {
            int v13, v14;

            if ((p_btn->Flags & GBxFlg_Unkn0020) != 0)
              lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            v13 = box_b;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              box_b + (box_w) - 4, box_b + 5, p_btn->BGColour);
            lbDisplay.DrawFlags |= 0x8000;
            draw_box_purple_list(p_btn->X - v13, p_btn->Y + 5,
              2 * v13 + box_w + 1, box_h - 4 + v13, p_btn->BGColour);
            lbDisplay.DrawFlags &= ~0x8000;
            draw_triangle_purple_list(p_btn->X + (box_w) - 4, p_btn->Y - v13,
              v13 + p_btn->X + (box_w) + 1, p_btn->Y + 5,
              p_btn->X + (box_w) - 4, p_btn->Y + 5, p_btn->BGColour);
            lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
            if ((p_btn->Flags & GBxFlg_Unkn0040) != 0)
              lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            v14 = box_w;
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(p_btn->X, box_h + p_btn->Y,
              p_btn->X + (box_w), box_h + p_btn->Y, p_btn->Colour);
            draw_line_purple_list(p_btn->X, p_btn->Y,
              p_btn->X + (box_w) - 5, p_btn->Y, p_btn->Colour);
            draw_line_purple_list(v14 + p_btn->X, p_btn->Y + 5,
              v14 + p_btn->X, p_btn->Y + box_h, p_btn->Colour);
            draw_line_purple_list(v14 + p_btn->X - 5, p_btn->Y,
              v14 + p_btn->X, p_btn->Y + 5, p_btn->Colour);
            lbDisplay.DrawFlags = 0;
        }
        else
        {
            if ((p_btn->Flags & GBxFlg_Unkn0020) != 0)
                lbDisplay.DrawFlags = Lb_SPRITE_TRANSPAR4;
            lbDisplay.DrawFlags |= 0x8000;
            draw_box_purple_list(p_btn->X - box_b, p_btn->Y - box_b,
              2 * box_b + (box_w) + 1, box_h + 1 + 2 * box_b, p_btn->BGColour);
            lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE;
            if ((p_btn->Flags & GBxFlg_Unkn0040) != 0)
              lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            draw_box_purple_list(p_btn->X, p_btn->Y, box_w + 1, box_h + 1, p_btn->Colour);
            lbDisplay.DrawFlags = 0;
        }
    }

    if ((p_btn->Flags & GBxFlg_TextRight) != 0)
    {
          lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_RIGHT;
    }
    else if ((p_btn->Flags & GBxFlg_TextCenter) != 0)
    {
          lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
    }
    if (p_btn->DrawTextFn != NULL) {
        p_btn->DrawTextFn(p_btn);
    }
    lbDisplay.DrawFlags = 0;
    return ret;
}

ubyte label_text(struct ScreenButton *p_btn)
{
    short text_x, text_y;
    short text_w, text_h;

    lbFontPtr = p_btn->Font;
    text_w = my_string_width(p_btn->Text);
    text_h = my_char_height('A');
    text_x = p_btn->X + ((p_btn->Width - text_w) >> 1) + 1;
    text_y = p_btn->Y + ((p_btn->Height - text_h) >> 1);
    my_set_text_window(text_x, text_y,
      lbDisplay.GraphicsScreenWidth, text_y + text_h);
    if (p_btn->Flags & 0x80) {
        p_btn->TextFadePos = -3;
        p_btn->Flags &= ~GBxFlg_Unkn0080;
    }
    return flashy_draw_text(0, 0, p_btn->Text,
      p_btn->TextSpeed, p_btn->TextTopLine,
      &p_btn->TextFadePos, 0);
}

ubyte button_text(struct ScreenButton *p_btn)
{
    short text_x, text_y;
    short text_w, text_h;

    lbFontPtr = p_btn->Font;
    text_w = my_string_width(p_btn->Text);
    text_h = my_char_height('A');
    text_x = p_btn->X + ((p_btn->Width - text_w) >> 1) + 1;
    text_y = p_btn->Y + ((p_btn->Height - text_h) >> 1);
    my_set_text_window(text_x, text_y,
      lbDisplay.GraphicsScreenWidth, text_y + text_h);
    if (p_btn->Flags & 0x80) {
        p_btn->TextFadePos = -3;
        p_btn->Flags &= ~GBxFlg_Unkn0080;
    }
    return flashy_draw_text(0, 0, p_btn->Text,
      p_btn->TextSpeed, p_btn->TextTopLine,
      &p_btn->TextFadePos, 1);
}

ubyte info_box_text(struct ScreenInfoBox *p_box)
{
    short txwidth, lnheight;
    short scr1_x, scr1_y;
    short scr2_x, scr2_y;
    ubyte drawn;

    if ((p_box->Flags & 0x0080) != 0)
    {
        p_box->TextFadePos1 = -5;
        p_box->TextFadePos2 = -5;
        p_box->Flags &= ~0x0080;
    }
    {
        lbFontPtr = p_box->Font1;
        lnheight = my_char_height('A');
        scr1_x = p_box->X + 3;
        scr1_y = p_box->Y + 3;
        scr2_x = p_box->X + p_box->Width;
        scr2_y = p_box->Y + 3 + lnheight;
        my_set_text_window(scr1_x, scr1_y, scr2_x, scr2_y);
        drawn = flashy_draw_text(0, 0, p_box->Text1,
          p_box->TextSpeed, 0, &p_box->TextFadePos1, 0);
    }
    if (drawn != 0)
    {
        lbFontPtr = p_box->Font2;
        lnheight = my_char_height('A');
        scr2_x = p_box->X + p_box->Width;
        scr2_y = p_box->Y + p_box->Height;
        txwidth = my_string_width(p_box->Text2);
        scr1_x = scr2_x - txwidth - 2;
        scr1_y = p_box->Y + ((p_box->Height - 1 - lnheight) >> 1);
        my_set_text_window(scr1_x, scr1_y, scr2_x, scr2_y);
        drawn = flashy_draw_text(0, 0, p_box->Text2,
          p_box->TextSpeed, 0, &p_box->TextFadePos2, 0);
    }
    return drawn;
}

void draw_triangle_purple_list(int x1, int y1,
  int x2, int y2, int x3, int y3, TbPixel colour)
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Triangle.X1 = lbDisplay.GraphicsWindowX + x1;
    p_pditem->U.Triangle.Y1 = lbDisplay.GraphicsWindowY + y1;
    p_pditem->U.Triangle.X2 = lbDisplay.GraphicsWindowX + x2;
    p_pditem->U.Triangle.Y2 = lbDisplay.GraphicsWindowY + y2;
    p_pditem->U.Triangle.X3 = lbDisplay.GraphicsWindowX + x3;
    p_pditem->U.Triangle.Y3 = lbDisplay.GraphicsWindowY + y3;
    p_pditem->U.Triangle.Colour = colour;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->Type = PuDT_TRIANGLE;
}

void draw_flic_purple_list(void (*fn)())
{
    struct PurpleDrawItem *p_pditem;

    p_pditem = &purple_draw_list[purple_draw_index];
    purple_draw_index++;

    p_pditem->U.Flic.Function = fn;
    p_pditem->U.Flic.Colour = lbDisplay.DrawColour;
    p_pditem->Flags = lbDisplay.DrawFlags;
    p_pditem->Type = PuDT_FLIC;
}

void draw_noise_box_purple_list(int x, int y, ulong width, ulong height)
{
    struct PurpleDrawItem *p_pditem;

    draw_box_purple_list(x, y, width, height, 0);
    p_pditem = &purple_draw_list[purple_draw_index - 1];
    p_pditem->Type = PuDT_NOISEBOX;
}

/******************************************************************************/
