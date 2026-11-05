/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file matrix.h
 *     Header file for matrix.c.
 * @par Purpose:
 *     Routines implementing matrix support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef MATRIX_H
#define MATRIX_H

#include "bftypes.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

#define LOCAL_MATS_COUNT 100

struct M33 {
  int R[3][3];
};

struct M31 {
  int R[3];
};

#pragma pack()
/******************************************************************************/
extern struct M33 local_mats[LOCAL_MATS_COUNT];
extern ushort next_local_mat;

void matrix_identity_fill(struct M33 *p_matx);

/** Transpose mat1 into mat2.
 */
void matrix_transpose_copy(struct M33 *p_mat1, struct M33 *p_mat2);

/** Rotates columns 0 and 2 of a 3x3 matrix, for yaw part of Euler decomposition.
 */
void matrix_rotate_yaw_copy(const struct M33 *p_src, struct M33 *p_dst,
  int cos_v, int sin_v);

/** Rotates columns 0 and 1 of a 3x3 matrix, for pitch part of Euler decomposition.
 */
void matrix_rotate_pitch_copy(const struct M33 *p_src, struct M33 *p_dst,
  int cos_v, int sin_v);

/** Rotates columns 1 and 2 of a 3x3 matrix, for roll part of Euler decomposition.
 */
void matrix_rotate_roll_copy(const struct M33 *p_src, struct M33 *p_dst,
  int cos_v, int sin_v);

void matrix_transform(struct M31 *p_result, struct M33 *p_trans, struct M31 *p_source);
void rotate_object_axis(struct M33 *p_base, short xangle, short yangle, short zangle);

void vec_cross_prod(struct M31 *p_vecr, struct M31 *p_vec1, struct M31 *p_vec2);
void object_vec_normalisation(struct M33 *p_vec, ubyte col);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
