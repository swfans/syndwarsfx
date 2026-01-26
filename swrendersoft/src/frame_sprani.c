/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file frame_sprani.c
 *     List of sprite animation frames used by the game.
 * @par Purpose:
 *     Defines frames of animations, and how they consitts of sprite elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Feb 2024 - 22 Sep 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frame_sprani.h"

#include <assert.h>
#include "bfsprite.h"
#include "bfmemut.h"

#include "privrdlog.h"
/******************************************************************************/
struct Frame *frame2;

struct Frame *frame = NULL;
struct Frame *frame_end = NULL;

struct Element *melement_ani = NULL;
struct Element *mele_ani_end = NULL;

ushort *nstart_ani = NULL;
ushort *nstart_ani_end = NULL;

/******************************************************************************/

#if 0
/** Removes unused frames from the file.
 */
void consolidate_nsta_nfra(void)
{
    struct Frame *p_src_frame;
    struct Frame *p_frame;
    ushort frm, nxfrm;
    uint n_starts, sta;
    ushort stani;

    frm = 1;
    n_starts = (nstart_ani_end - nstart_ani);
    LOGSYNC("n_starts=%d", n_starts);
    for (sta = 1; (sta < n_starts - 1) && (frm <= 2000); sta++)
    {
        stani = nstart_ani[sta];
        if (stani == 0)
            continue;
        nstart_ani[sta] = frm;
        p_src_frame = &frame2[stani];
        p_frame = &frame[frm];
        LbMemoryCopy(p_frame, p_src_frame, sizeof(struct Frame));
        frm++;
        p_frame->Next = frm;
        for (nxfrm = p_src_frame->Next; nxfrm != stani; nxfrm = p_src_frame->Next)
        {
          p_src_frame = &frame2[nxfrm];
          p_frame = &frame[frm];
          LbMemoryCopy(p_frame, p_src_frame, sizeof(struct Frame));
          frm++;
          p_frame->Next = frm;
        }
        frame[frm - 1].Next = nstart_ani[sta];
    }
    save_range_to_file("data/nsta-0.ani", nstart_ani, nstart_ani_end);
    save_range_to_file("data/nfra-0.ani", frame, frame_end);
}
#endif

void shield_frames_init(void)
{
    shield_frm[0] = nstart_ani[984];
    shield_frm[1] = frame[frame[shield_frm[0]].Next].Next;
    shield_frm[2] = frame[frame[shield_frm[1]].Next].Next;
    shield_frm[3] = frame[frame[shield_frm[2]].Next].Next;
}

void shield_frames_cycle(void)
{
    ushort i;

    for (i = 0; i < 4; i++)
    {
        shield_frm[i] = frame[shield_frm[i]].Next;
    }
}

/******************************************************************************/
