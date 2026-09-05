/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file engintext.h
 *     Header file for engintext.c.
 * @par Purpose:
 *     Drawing text on screen within the game engine.
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
#ifndef ENGINTEXT_H
#define ENGINTEXT_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)


#pragma pack()
/******************************************************************************/
TbBool AppTextDrawMissionChatMessage(int posx, int *posy, int plyr, int timer,
  const char *text);

TbBool AppTextDrawMissionStatus(int posx, int posy, const char *text);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
