#if !defined(_VECTOR_)
#define _VECTOR_

/** 3Dgpl3 ************************************************\
 * Header for operations on vectors used by the ray       *
 * tracer.                                                *
 *                                                        *
 * Files:                                                 *
 *  vector.c                 Operations on vectors.       *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#define V_LNG_VECTOR 3                      /* X Y Z */

/**********************************************************/

float *V_zero(float *vec);
float *V_vector_coordinates(float *vector,float x,float y,float z);
float *V_vector_points(float *vector,float *from,float *to);
float *V_set(float *what,float *to);
float *V_multiply(float *result,float *vector,float m);
float V_scalar_product(float *a,float *b);
float* V_vector_product(float *product,float *a,float *b);
float *V_sum(float *sm,float *a,float *b);
float *V_difference(float *differ,float *a,float *b);
float *V_unit_vector(float *vector,float *from,float *to);
float *V_plane(float *plane,float *normal,float *origine);
float V_vertex_on_plane(float *plane,float *vertex);

/**********************************************************/

#endif
