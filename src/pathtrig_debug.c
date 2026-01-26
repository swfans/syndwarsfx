/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file pathtrig.c
 *     Pathfinding triangulation support required by the 3D engine.
 * @par Purpose:
 *     Implement functions for handling pathfinding triangulation.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Sep 2023 - 02 Jan 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pathtrig.h"

#include <assert.h>
#include "bfmemory.h"
#include "bfmemut.h"
#include "bfline.h"
#include "bfpng.h"

#include "bigmap.h"
#include "display.h"
#include "enginsngobjs.h"
#include "lvwalk.h"
#include "triangls.h"
#include "tngcolisn.h"
#include "game.h"
#include "swlog.h"
/******************************************************************************/

static void print_walk_items_for_face(short face)
{
    struct WalkHeader *p_walk_head;
    ushort wh, wi;

    if (face > 0)
    {
        struct SingleObjectFace3 *p_face;

        p_face = &game_object_faces[face];
        wh = p_face->WalkHeader;
    }
    else if (face < 0)
    {
        struct SingleObjectFace4 *p_face;

        p_face = &game_object_faces4[-face];
        wh = p_face->WalkHeader;
    } else
    {
        return;
    }
    p_walk_head = &game_walk_headers[wh];
    for (wi = p_walk_head->StartItem;
      wi < p_walk_head->StartItem + p_walk_head->Count; wi++) {
        LOGSYNC_F("face %d walkface %d", (int)face, (int)game_walk_items[wi]);
    }
}

void print_walk_items(void)
{
    short face;

    for (face = 1; face < next_object_face; face++)
    {
        if ((game_object_faces[face].GFlags & 0x04) != 0)
            print_walk_items_for_face(face);
    }
    for (face = 1; face < next_object_face4; face++)
    {
        if ((game_object_faces4[face].GFlags & 0x04) != 0)
            print_walk_items_for_face(-face);
    }
}

void print_mapel_collision_columns(void)
{
    ushort tile_x, tile_z;

    for (tile_x = 0; tile_x < MAP_TILE_WIDTH; tile_x++)
    {
        for (tile_z = 0; tile_z < MAP_TILE_HEIGHT; tile_z++)
        {
            struct MyMapElement *p_mapel;
            struct ColColumn *p_ccol;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            if (p_mapel->ColumnHead == 0) continue;
            p_ccol = &game_col_columns[p_mapel->ColumnHead];

            LOGSYNC_F("%02d,%02d qbits %04x %04x %04x %04x", (int)tile_x, (int)tile_z,
              (int)p_ccol->QBits[0], (int)p_ccol->QBits[1],
              (int)p_ccol->QBits[2], (int)p_ccol->QBits[3]);
        }
    }
}

void print_collision_vects(void)
{
    ushort tile_x, tile_z;

    for (tile_x = 0; tile_x < MAP_TILE_WIDTH; tile_x++)
    {
        for (tile_z = 0; tile_z < MAP_TILE_HEIGHT; tile_z++)
        {
            struct MyMapElement *p_mapel;
            int cv;

            p_mapel = &game_my_big_map[MAP_TILE_WIDTH * tile_z + tile_x];
            cv = p_mapel->ColHead;
            while (cv != 0)
            {
                struct ColVectList *p_cvlist;
                struct ColVect *p_colvect;

                p_cvlist = &game_col_vects_list[cv];
                p_colvect = &game_col_vects[p_cvlist->Vect];


                LOGSYNC_F("%02d,%02d ColVectList Obj %d Vect %d Face %hd P1=%hd,%hd,%hd P2=%hd,%hd,%hd",
                  (int)tile_x, (int)tile_z, (int)p_cvlist->Object,
                  (int)p_cvlist->Vect, p_colvect->Face,
                  p_colvect->X1, p_colvect->Y1, p_colvect->Z1,
                  p_colvect->X2, p_colvect->Y2, p_colvect->Z2);
                cv = p_cvlist->NextColList & 0x7FFF;
            }
        }
    }
}

void print_triangulation_trigs(void)
{
    int tri;

    for (tri = 0; tri < triangulation[0].max_Triangles; tri++)
    {
        struct TrTriangle *p_tri;
        struct TrPoint *p_pt0;
        struct TrPoint *p_pt1;
        struct TrPoint *p_pt2;

        if (!tri_is_allocated(tri))
            continue;
        p_tri = &triangulation[0].Triangles[tri];
        p_pt0 = &triangulation[0].Points[p_tri->point[0]];
        p_pt1 = &triangulation[0].Points[p_tri->point[1]];
        p_pt2 = &triangulation[0].Points[p_tri->point[2]];

        LOGSYNC_F("Tri pt0(%d,%d) pt1(%d,%d) pt2(%d,%d) jump=%d, solid=%d, enter=0x%x",
          (int)p_pt0->x, (int)p_pt0->y,
          (int)p_pt1->x, (int)p_pt1->y,
          (int)p_pt2->x, (int)p_pt2->y,
          //(int)p_tri->tri[0], (int)p_tri->tri[1], (int)p_tri->tri[2],
          (int)p_tri->jump, (int)p_tri->solid, (int)p_tri->enter);
    }
}

void print_triangulation_points(void)
{
    int pt;

    for (pt = 0; pt < triangulation[0].max_Points; pt++)
    {
        struct TrPoint *p_pt;

        if (!point_is_allocated(pt))
            continue;
        p_pt = &triangulation[0].Points[pt];

        LOGSYNC_F("Pt(%d,%d)",
          (int)p_pt->x, (int)p_pt->y);
    }
}

void print_triangulation(void)
{
    print_triangulation_points();
    print_triangulation_trigs();
}

void draw_triangulation_palette(ubyte *pal)
{
    int i;
    const int m = 4;

    LbMemorySet(pal, 0, 256*3);
    for (i = 0; i < 16; i++)
    {
        pal[3 * (  0 + i) + 0] = m*i;
        pal[3 * ( 16 + i) + 1] = m*i;
        pal[3 * ( 32 + i) + 2] = m*i;

        pal[3 * ( 48 + i) + 0] = m*i;
        pal[3 * ( 48 + i) + 1] = m*i;

        pal[3 * ( 64 + i) + 0] = m*i;
        pal[3 * ( 64 + i) + 2] = m*i;

        pal[3 * ( 80 + i) + 1] = m*i;
        pal[3 * ( 80 + i) + 2] = m*i;

        pal[3 * ( 96 + i) + 0] = m*i;
        pal[3 * ( 96 + i) + 1] = 63;

        pal[3 * (112 + i) + 0] = 63;
        pal[3 * (112 + i) + 1] = m*i;

        pal[3 * (128 + i) + 0] = m*i;
        pal[3 * (128 + i) + 2] = 63;

        pal[3 * (144 + i) + 0] = 63;
        pal[3 * (144 + i) + 2] = m*i;

        pal[3 * (160 + i) + 1] = m*i;
        pal[3 * (160 + i) + 2] = 63;

        pal[3 * (176 + i) + 1] = 63;
        pal[3 * (176 + i) + 2] = m*i;

        pal[3 * (192 + i) + 0] = m*i;
        pal[3 * (192 + i) + 1] = 63;
        pal[3 * (192 + i) + 2] = 63;

        pal[3 * (208 + i) + 0] = 63;
        pal[3 * (208 + i) + 1] = m*i;
        pal[3 * (208 + i) + 2] = 63;

        pal[3 * (224 + i) + 0] = 63;
        pal[3 * (224 + i) + 1] = 63;
        pal[3 * (224 + i) + 2] = m*i;

        pal[3 * (240 + i) + 0] = m*i;
        pal[3 * (240 + i) + 1] = m*i;
        pal[3 * (240 + i) + 2] = m*i;
    }
}

void draw_triangulation_debug(int beg_x, int beg_y, int cor_shl)
{
    int tri;

    for (tri = 0; tri < triangulation[0].max_Triangles; tri++)
    {
        struct TrTriangle *p_tri;
        struct TrPoint *p_pt0;
        struct TrPoint *p_pt1;
        struct TrPoint *p_pt2;
        int x0, y0, x1, y1, x2, y2;
        ubyte col;

        if (!tri_is_allocated(tri))
            continue;
        p_tri = &triangulation[0].Triangles[tri];
        p_pt0 = &triangulation[0].Points[p_tri->point[0]];
        p_pt1 = &triangulation[0].Points[p_tri->point[1]];
        p_pt2 = &triangulation[0].Points[p_tri->point[2]];
        col = p_tri->solid;
        col = (col & 0x3F) | (col >> 2);
        if (col > 2) col--;

        x0 = (p_pt0->x >> cor_shl) + beg_x;
        y0 = (p_pt0->y >> cor_shl) + beg_y;
        x1 = (p_pt1->x >> cor_shl) + beg_x;
        y1 = (p_pt1->y >> cor_shl) + beg_y;
        x2 = (p_pt2->x >> cor_shl) + beg_x;
        y2 = (p_pt2->y >> cor_shl) + beg_y;
        { // Make the trig a little smaller on one side
            ushort iflg;
            iflg = 0;
            if ((x0 <= x1) && (x0 <= x2))
                iflg |= 0x01;
            if ((x1 <= x0) && (x1 <= x2))
                iflg |= 0x02;
            if ((x2 <= x0) && (x2 <= x1))
                iflg |= 0x04;

            if ((y0 <= y1) && (y0 <= y2))
                iflg |= 0x10;
            if ((y1 <= y0) && (y1 <= y2))
                iflg |= 0x20;
            if ((y2 <= y0) && (y2 <= y1))
                iflg |= 0x40;
            if (iflg & 0x01) x0++;
            if (iflg & 0x02) x1++;
            if (iflg & 0x04) x2++;
            if (iflg & 0x10) y0++;
            if (iflg & 0x20) y1++;
            if (iflg & 0x40) y2++;
        }
        lbDisplay.DrawFlags = 0;
        //LbDrawTriangle(x0, y0, x2, y2, x1, y1, col); // this function is buggy for some coords

        col = (p_tri->enter & 0x01) ? 192 : 15;
        LbDrawLine(x0, y0, x1, y1, col);
        col = (p_tri->enter & 0x02) ? 192 : 15;
        LbDrawLine(x1, y1, x2, y2, col);
        col = (p_tri->enter & 0x04) ? 192 : 15;
        LbDrawLine(x2, y2, x0, y0, col);
    }
}

void draw_triangulation_debug_image(ubyte *img, int beg_x, int beg_y, int img_w, int img_h, int cor_shl)
{
    struct ScreenBufBkp bkp;

    screen_switch_to_custom_buffer(&bkp, img, img_w, img_h);
    draw_triangulation_debug(beg_x, beg_y, cor_shl);
    screen_load_backup_buffer(&bkp);
}

void save_triangulation_png(const char *fname)
{
    int cor_shl = 4;
    int margin = 260;
    int img_dim = ((128 * 256) >> cor_shl) + 2 * margin;
    ubyte *pal;
    ubyte *img;

    pal = malloc(256*3);
    img = malloc(img_dim*img_dim);
    draw_triangulation_palette(pal);
    draw_triangulation_debug_image(img, margin, margin, img_dim, img_dim, cor_shl);
    LbPngSave(fname, img, img_dim, img_dim, pal, true);
    free(pal);
    free(img);
}

/******************************************************************************/
