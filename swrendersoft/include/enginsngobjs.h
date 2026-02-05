/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file enginsngobjs.h
 *     Header file for enginsngobjs.c.
 * @par Purpose:
 *     Engine single objects required by the 3D engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     29 Sep 2023 - 02 Jan 2024
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef ENGINSNGOBJS_H
#define ENGINSNGOBJS_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

enum FaceGFlags {
    FGFlg_Unkn01     = 0x01,
    FGFlg_Unkn02     = 0x02,
    FGFlg_Unkn04     = 0x04,
    FGFlg_Unkn08     = 0x08,
    FGFlg_Unkn10     = 0x10,
    FGFlg_Unkn20     = 0x20,
    FGFlg_Unkn40     = 0x40,
    FGFlg_Unkn80     = 0x80,
};

struct SingleObjectFace3 { // sizeof=32
    short PointNo[3];
    ushort Texture;
    ubyte GFlags;
    ubyte Flags;
    ushort ExCol;
    ushort Object; /**< offs=0x0C Index of the SingleObject associated to this face. */
    short Shade0; /**< offs=0x0E Index of the Normal containing LightRatio for point 0. */
    short Shade1; /**< offs=0x10 Index of the Normal containing LightRatio for point 1. */
    short Shade2; /**< offs=0x12 Index of the Normal containing LightRatio for point 2. */
    ushort Light0; /**< offs=0x14 Index of the first QuickLight in a chain of lights affectin point 0. */
    ushort Light1; /**< offs=0x16 Index of the first QuickLight in a chain of lights affectin point 1. */
    ushort Light2; /**< offs=0x18 Index of the first QuickLight in a chain of lights affectin point 2. */
    ushort FaceNormal;
    ushort WalkHeader; /**< offs=0x1C Index of the WalkHeader created for this face. */
    ushort UnknTringl;
};

struct SingleObjectFace3OldV7 { // sizeof=48
    short PointNo[3];
    ushort Texture;
    ubyte GFlags;
    ubyte Flags;
    ushort ExCol;
    ushort Object; /**< offs=0x0C Index of the SingleObject associated to this face. */
    short Shade0;
    ushort UnkOFField_10;
    ushort UnkOFField_12;
    short Shade1;
    ushort UnkOFField_16;
    ushort UnkOFField_18;
    short Shade2;
    ushort UnkOFField_1C;
    ushort UnkOFField_1E;
    ushort Light0; /**< offs=0x20 Index of the first QuickLight in a chain of lights affectin point 0. */
    ushort Light1;
    ushort Light2;
    ushort FaceNormal; /**< offs=0x26 Index of the Normal vector created for this face. */
    ushort WalkHeader; /**< offs=0x28 Index of the WalkHeader created for this face. */
    ushort UnkOFField_2A;
    ushort UnkOFField_2C;
    ushort UnkOFField_2E;
};

struct SingleObjectFace4 { // sizeof=40
    short PointNo[4];
    ushort Texture;
    ubyte GFlags;
    ubyte Flags;
    ushort ExCol;
    ushort Object; /**< offs=0x0E Index of the SingleObject associated to this face. */
    short Shade0; /**< offs=0x10 Index of the Normal containing LightRatio for point 0. */
    short Shade1; /**< offs=0x12 Index of the Normal containing LightRatio for point 1. */
    short Shade2; /**< offs=0x14 Index of the Normal containing LightRatio for point 2. */
    short Shade3; /**< offs=0x16 Index of the Normal containing LightRatio for point 3. */
    short Light0; /**< offs=0x18 Index of the first QuickLight in a chain of lights affectin point 0. */
    short Light1; /**< offs=0x1A Index of the first QuickLight in a chain of lights affectin point 1. */
    short Light2; /**< offs=0x1C Index of the first QuickLight in a chain of lights affectin point 2. */
    short Light3; /**< offs=0x1E Index of the first QuickLight in a chain of lights affectin point 3. */
    ushort FaceNormal;
    ushort WalkHeader; /**< offs=0x22 Index of the WalkHeader created for this face. */
    ushort UnknTringl1;
    ushort UnknTringl2;
};

struct SingleObjectFace4OldV7 { // sizeof=60
    short PointNo[4];
    ushort Texture;
    ubyte GFlags;
    ubyte Flags;
    ubyte GFlags2;
    ubyte Flags2;
    ushort ExCol;
    ushort Object; /**< offs=0x10 Index of the SingleObject associated to this face. */
    short Shade0;
    short UnkOFField_14;
    short UnkOFField_16;
    short Shade1;
    short UnkOFField_1A;
    short UnkOFField_1C;
    short Shade2;
    short UnkOFField_20;
    short UnkOFField_22;
    short Shade3;
    short UnkOFField_26;
    short UnkOFField_28;
    short Light0;
    short Light1;
    short Light2;
    short Light3;
    ushort FaceNormal; /**< offs=0x32 Index of the Normal vector created for this face. */
    ushort WalkHeader; /**< offs=0x34 Index of the WalkHeader created for this face. */
    ushort UnkOFField_36;
    ushort UnkOFField_38;
    ushort UnkOFField_3A;
};

struct SinglePoint { // sizeof=10
    ushort PointOffset;
    short X;
    short Y;
    short Z;
    ubyte Pad1;
    ubyte Flags;
};

struct Normal { // sizeof=16
    int NX;
    int NY;
    int NZ;
    int LightRatio;
};

struct SingleObject { // sizeof=36
  ushort StartFace;
  ushort NumbFaces;
  ushort NextObject;
  ushort StartFace4;
  ushort NumbFaces4;
  short ThingNo;
  short OffsetX;
  short OffsetY;
  short OffsetZ;
  short ObjectNo;
  short MapX;
  short MapZ;
  ushort StartPoint;
  ushort EndPoint;
  ushort field_1C;
  ushort field_1E;
  ubyte field_20[3];
  ubyte field_23;
};

#pragma pack()
/******************************************************************************/
extern struct SingleObjectFace3 *game_object_faces;
extern ushort next_object_face;

extern struct SingleObjectFace4 *game_object_faces4;
extern ushort next_object_face4;

extern struct SinglePoint *game_object_points;
extern ushort next_object_point;

extern struct Normal *game_normals;
extern ushort next_normal;

extern struct SingleObject *game_objects;
extern ushort next_object;

/** Maps fields from old SingleObjectFace3 struct to the current one.
 */
void refresh_old_object_face_format(struct SingleObjectFace3 *p_objface,
  struct SingleObjectFace3OldV7 *p_oldobjface, u32 fmtver);

/** Maps fields from old SingleObjectFace4 struct to the current one.
 */
void refresh_old_object_face4_format(struct SingleObjectFace4 *p_objface4,
  struct SingleObjectFace4OldV7 *p_oldobjface4, u32 fmtver);

/** Sets some of the object faces flags based on other properties.
 */
void update_object_faces_flags(void);

TbBool face_is_blocking_walk(short face);

void unkn_object_shift_03(ushort objectno);
void unkn_object_shift_02(int norm1, int norm2, ushort objectno);

int get_height_on_face(int x, int z, ushort face);
int get_height_on_face_quad(int x, int z, ushort face);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
