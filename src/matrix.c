/******************************************************************************/
// Syndicate Wars Fan Expansion, source port of the classic game from Bullfrog.
/******************************************************************************/
/** @file matrix.c
 *     Routines implementing matrix support.
 * @par Purpose:
 *     Implement functions for handling multi-dimensional matrices.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 May 2022 - 12 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "matrix.h"
#include "swlog.h"
/******************************************************************************/

struct M33 local_mats[LOCAL_MATS_COUNT];
ushort next_local_mat = 1;

/******************************************************************************/

void matrix_identity_fill(struct M33 *p_mat)
{
    asm volatile ("call ASM_matrix_identity_fill\n"
        : : "a" (p_mat));
}

void matrix_transpose_copy(struct M33 *p_mat1, struct M33 *p_mat2)
{
#if 0
    asm volatile ("call ASM_matrix_transpose_copy\n"
        :  : "a" (p_mat1),  "d" (p_mat2));
#endif
    int row, col;

    for (row = 0; row < 3; row++)
        for (col = 0; col < 3; col++)
            p_mat2->R[col][row] = p_mat1->R[row][col];
}

void rotate_object_axis(struct M33 *p_base, short xangle, short yangle, short zangle)
{
    asm volatile ("call ASM_rotate_object_axis\n"
        : : "a" (p_base), "d" (xangle), "b" (yangle), "c" (zangle));
}

void vec_cross_prod(struct M31 *vecr, struct M31 *vec1, struct M31 *vec2)
{
    asm volatile ("call ASM_vec_cross_prod\n"
        : : "a" (vecr), "d" (vec1), "b" (vec2));
}

void object_vec_normalisation(struct M33 *vec, ubyte col)
{
    asm volatile ("call ASM_object_vec_normalisation\n"
        : : "a" (vec), "d" (col));
}

void matrix_transform(struct M31 *p_result, struct M33 *p_trans, struct M31 *p_source)
{
    p_result->R[0] = p_trans->R[0][2] * p_source->R[2] + p_trans->R[0][0] * p_source->R[0] + p_source->R[1] * p_trans->R[0][1];
    p_result->R[1] = p_trans->R[1][2] * p_source->R[2] + p_trans->R[1][0] * p_source->R[0] + p_source->R[1] * p_trans->R[1][1];
    p_result->R[2] = p_trans->R[2][2] * p_source->R[2] + p_trans->R[2][1] * p_source->R[1] + p_source->R[0] * p_trans->R[2][0];
}

/******************************************************************************/
