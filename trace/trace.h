#if !defined(_TRACE_H_)
#define _TRACE_H_

/** 3Dgpl3 ************************************************\
 * Header for the 3D ray tracing engine.                  *
 *                                                        *
 * Files:                                                 *
 *  trace.c                 Ray tracing engine.           *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../vector/vector.h"               /* V_LNG_VECTOR */

/**********************************************************\
 * Material. Contains reflection coeficients.             *
 *                                                        *
 *  +--------------------+                                *
 *  |             +---+---+---+                           *
 *  | tr_ambient  | R | G | B |                           *
 *  |             +---+---+---+                           *
 *  |                    |                                *
 *  |             +---+---+---+                           *
 *  | tr_diffuse  | R | G | B |                           *
 *  |             +---+---+---+                           *
 *  |                    |                                *
 *  | tr_specular        |                                *
 *  | tr_exponent        |                                *
 *  | tr_reflect         |                                *
 *  +--------------------+                                *
 *                                                        *
\**********************************************************/

struct TR_matter
{
 float tr_ambient[V_LNG_VECTOR];            /* coefs of ambient reflection */
 float tr_diffuse[V_LNG_VECTOR];            /* coefs of diffuse reflection */
 float tr_specular;                         /* coef of specular reflection */
 float tr_exponent;                         /* specular exponent */
 float tr_reflect;                          /* recursive ray */
};

/**********************************************************\
 * Light source. Represents a point light equal into all  *
 * directions.                                            *
 *                                                        *
 *  +--------------------+                                *
 *  |              +---+---+---+                          *
 *  | tr_centre    | X | Y | Z |                          *
 *  |              +---+---+---+                          *
 *  |                    |                                *
 *  |              +---+---+---+                          *
 *  | tr_intensity | R | G | B |                          *
 *  |              +---+---+---+                          *
 *  +--------------------+                                *
 *                                                        *
\**********************************************************/

struct TR_point_light
{
 float tr_centre[V_LNG_VECTOR];             /* point light source */
 float tr_intensity[V_LNG_VECTOR];
};

/**********************************************************\
 * Ray. Represented through origin and direction vector.  *
 *                                                        *
 *  +--------------------+                                *
 *  |               +---+---+---+                         *
 *  | tr_start      | X | Y | Z |                         *
 *  |               +---+---+---+                         *
 *  |                    |                                *
 *  |               +---+---+---+                         *
 *  | tr_codirected | X | Y | Z |                         *
 *  |               +---+---+---+                         *
 *  +--------------------+                                *
 *                                                        *
\**********************************************************/

struct TR_ray
{
 float tr_start[V_LNG_VECTOR];              /* origin of the ray */
 float tr_codirected[V_LNG_VECTOR];         /* a co-directed vector */
};

/**********************************************************\
 * Sphere. Represented through centre and radius.         *
 *                                                        *
 *  +--------------------+                                *
 *  | tr_type            |            TR_SPHERE           *
 *  |             +-------------+                         *
 *  | tr_material | TR_material |                         *
 *  |             +-------------+                         *
 *  |                    |                                *
 *  |               +---+---+---+                         *
 *  | tr_centre     | X | Y | Z |                         *
 *  |               +---+---+---+                         *
 *  | tr_radius          |                                *
 *  +--------------------+                                *
 *                                                        *
\**********************************************************/

#define TR_SPHERE 0x1

struct TR_generic_object                    /* for storage in the world */
{
 int tr_type;
 struct TR_matter tr_material;              /* material it is made of */
};

struct TR_sphere
{
 int tr_type;                               /* TR_CYLINDER */
 struct TR_matter tr_material;              /* material it is made of */

 float tr_centre[V_LNG_VECTOR];             /* sphere's centre */
 float tr_radius;
};

void TR_sphere_init(struct TR_sphere *s);
float TR_sphere_intersect(struct TR_ray *r,struct TR_sphere *s);
float *TR_sphere_normal(float *normal,float *where,
                        struct TR_sphere *s);

/**********************************************************\
 * Cylinder. Represented through origin, direction,       *
 *           radius.                                      *
 *                                                        *
 *  +--------------------+                                *
 *  | tr_type            |            TR_CYLINDER         *
 *  |             +-------------+                         *
 *  | tr_material | TR_material |                         *
 *  |             +-------------+                         *
 *  |                    |                                *
 *  |               +---+---+---+                         *
 *  | tr_origin     | X | Y | Z |                         *
 *  |               +---+---+---+                         *
 *  |                    |                                *
 *  |               +---+---+---+                         *
 *  | tr_direction  | X | Y | Z |                         *
 *  |               +---+---+---+                         *
 *  | tr_radius          |                                *
 *  +--------------------+                                *
 *                                                        *
\**********************************************************/

#define TR_CYLINDER 0x2

struct TR_cylinder
{
 int tr_type;                               /* TR_SPHERE */
 struct TR_matter tr_material;              /* material it is made of */

 float tr_origine[V_LNG_VECTOR];            /* cylinder's origin */
 float tr_direction[V_LNG_VECTOR];
 float tr_radius;
};

void TR_cylinder_init(struct TR_cylinder *s);
float TR_cylinder_intersect(struct TR_ray *r,struct TR_cylinder *s);
float *TR_cylinder_normal(float *normal,float *where,
                          struct TR_cylinder *s,
                          struct TR_ray *r,float t);

/**********************************************************\
 * Polygon. Represented through its vertex set.           *
 *                                                        *
 *  +--------------------+                                *
 *  | tr_type            |                TR_POLYGON      *
 *  |             +-------------+                         *
 *  | tr_material | TR_material |                         *
 *  |             +-------------+                         *
 *  |                    |                                *
 *  |             +---+---+---+                           *
 *  | tr_normal   | X | Y | Z |                           *
 *  |             +---+---+---+                           *
 *  |                    |         +---+---+---+---+      *
 *  | tr_edges ------------------->| A | B | C | D |      *
 *  |                    |         +---+---+---+---+      *
 *  |                    |         |      ...      |      *
 *  |                    |         +---------------+      *
 *  |                    |                                *
 *  | tr_no_vertices     |         +---+---+---+          *
 *  | tr_vertices ---------------->| X | Y | Z |          *
 *  |                    |         +---+---+---+          *
 *  |                    |         |    ...    |          *
 *  +--------------------+         +-----------+          *
 *                                                        *
\**********************************************************/

#define TR_POLYGON 0x3

struct TR_polygon
{
 int tr_type;
 struct TR_matter tr_material;              /* material it is made of */

 float tr_normal[V_LNG_VECTOR];
 float *tr_edges;                           /* for intersection calcs */

 int tr_no_vertices;                        /* first vertex equals last */
 float *tr_vertices;
};

void TR_polygon_init(struct TR_polygon *p);
float TR_polygon_intersect(struct TR_ray *r,struct TR_polygon *p);
float *TR_polygon_normal(float *normal,float *where,
                         struct TR_polygon *p);

/**********************************************************\
 * World. A set of objects, a set of lights and a viewer. *
 * The latter always views along Z axis.                  *
 *                                                        *
 *  +--------------------+                                *
 *  |               +---+---+---+                         *
 *  | tr_ambient    | R | G | B |                         *
 *  |               +---+---+---+                         *
 *  | tr_no_point_lights |                                *
 *  |                    |     +---+---+-- ----+          *
 *  | tr_point_lights -------->| O | O |  ...  |          *
 *  |                    |     +-|-+-|-+- -----+          *
 *  |                    |       V   V                    *
 *  |            +----------------+ +----------------+    *
 *  |            | TR_point_light | | TR_point_light |    *
 *  |            +----------------+ +----------------+    *
 *  | tr_no_objects      |                                *
 *  |                    |     +---+---+-- ----+          *
 *  | tr_object -------------->| O | O |  ...  |          *
 *  |                    |     +-|-+-|-+- -----+          *
 *  +--------------------+       V   V                    *
 *                    +-----------+ +-----------+         *
 *                    | TR_object | | TR-object |         *
 *                    +-----------+ +-----------+         *
\**********************************************************/

#define TR_MAX_POINT_LIGHTS 10
#define TR_MAX_SPHERES      10

struct TR_world
{
 float tr_ambient[V_LNG_VECTOR];            /* illumination of the world */
 int tr_no_point_lights;
 struct TR_point_light **tr_point_lights;

 int tr_no_objects;                         /* number of objects */
 struct TR_generic_object **tr_objects;
};

#define TR_AMBIENT  0x1                     /* only ambient illumination */
#define TR_DIFFUSE  0x2                     /* only diffuse illumination */
#define TR_SPECULAR 0x4                     /* plus all of the above */
#define TR_SHADOW   0x8                     /* shoot shadow rays */
#define TR_REFLECT  0x10                    /* shoot reflection rays */

void TR_init_rendering(int type);
void TR_init_camera(float viewer_x,float viewer_y,float viewer_z,
                    float screen_x,float screen_y,float screen_z,
                    float screen_ux,float screen_uy,float screen_uz,
                    float screen_vx,float screen_vy,float screen_vz);
void TR_init_world(struct TR_world *w);
void TR_trace_world(struct TR_world *w,int depth);

/**********************************************************/

#endif
