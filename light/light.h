#if !defined(_LIGHT_H_)
#define _LIGHT_H_

/** 3Dgpl3 ************************************************\
 * Header for lighting and material.                      *
 *                                                        *
 * Files:                                                 *
 *  light.c                  Lighting and material;       *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* HW_fixed */
#include "../trans/trans.h"                 /* T_LNG_VECTOR */

/**********************************************************\
 * A light source, either a point light source or a       *
 * directional light source. In the former cases the      *
 * parameter describes the position in the latter case    *
 * the orientation vector.                                *
 *                                                        *
 *  +------------------------+                            *
 *  | l_type                 | M_AMBIENT|M_POINT|M_DIRECT *
 *  |                  +---+---+---+                      *
 *  | l_parameter      | X | Y | Z |                      *
 *  |                  +---+---+---+                      *
 *  | l_red l_green l_blue   |                            *
 *  +------------------------+                            *
 *                                                        *
\**********************************************************/

#define L_AMBIENT 0x1                       /* ambient light source */
#define L_POINT   0x2                       /* point light source */
#define L_DIRECT  0x4                       /* directional light source */

struct L_light                              /* describes a light source */
{
 int l_type;                                /* M_AMBIENT,M_POINT or M_DIRECT */

 int l_parameter[T_LNG_VECTOR];             /* location of the light */

 HW_fixed l_red;                            /* intensity as RGB */
 HW_fixed l_green;
 HW_fixed l_blue;
};

void L_init_lights(struct L_light** lights,int no_lights);

/**********************************************************\
 * Material. Contains reflection coeficients.             *
 *                                                        *
 *  +----------------------------------+                  *
 *  | l_amb_red l_amb_green l_amb_blue |                  *
 *  | l_dif_red l_dif_green l_dif_blue |                  *
 *  +----------------------------------+                  *
 *                                                        *
\**********************************************************/

struct L_material
{
 HW_fixed l_amb_red,l_amb_green,l_amb_blue; /* ambient reflection coefs */
 HW_fixed l_dif_red,l_dif_green,l_dif_blue; /* diffuse reflection coefs */
};

void L_init_material(struct L_material* mat);
void L_light_vertex(int *intensity,int* vertex,int* normal);

/**********************************************************/

#endif
