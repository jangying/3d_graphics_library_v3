#if !defined(_SHAPE_H_)
#define _SHAPE_H_

/** 3Dgpl3 ************************************************\
 * Header for 3-D model builder.                          *
 *                                                        *
 * Files:                                                 *
 *  shape.c                 3-D model builder;            *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../engine/engine.h"               /* G_MAX_POLYGON_VERTICES */

/**********************************************************/

#define M_PI_ 3.141592654

/**********************************************************/

void S_init_construction(struct G_texture* txtr,
                         int txtr_space_size,
                         struct L_material* mater);
struct M_polygon_object* S_sphere(int radius,int divs);
struct M_polygon_object* S_cylinder(int length,int radius,int divs);

/**********************************************************/

#endif
