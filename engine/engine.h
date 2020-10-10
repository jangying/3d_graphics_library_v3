#if ((!defined(_Z_BUFFER_)) && (!defined(_PAINTER_)))
 #error Either _Z_BUFFER_ or _PAINTER_ must be declared.
#endif
#if !defined(_ENGINE_H_)
#define _ENGINE_H_

/** 3Dgpl3 ************************************************\
 * Header for the polygonal 3D engine.                    *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_               Depth array;                 *
 *  _PAINTER_                Back-front order.            *
 *                                                        *
 * Files:                                                 *
 *  eng-base.c               Polymorphic polygon;         *
 *  eng-poly.c               Polygonal object;            *
 *  eng-bcub.c               Bicubic patches;             *
 *  eng-surf.c               Landscape object;            *
 *  eng-grup.c               Group of objects.            *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../graphics/graphics.h"           /* G_MAX_POLYGON_VERTICES */
#include "../trans/trans.h"                 /* T_LNG_VECTOR */

extern unsigned char M_camera_gam;          /* needed for surfaces */
extern int M_camera_x,M_camera_y,M_camera_z;
extern int M_camera_log_focus;              /* camera parameters */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * The following semi-internal structures are            *
 * used by the structures representing various 3D        *
 * objects:                                              *
 *                                                       *
 *  M_polygon                A polygon;                  *
 *  M_bicubic                A patch;                    *
 *  M_polygon_object_order   A BSP tree;                 *
 *  M_surface_cell           Element of the surface.     *
 *                                                       *
 * This external structures model various 3D objects:    *
 *                                                       *
 *  M_polygon_object         Polygonal solid;            *
 *  M_bicubic_object         Bicubic solid;              *
 *  M_surface_object         A landscape;                *
 *  M_group                  Set of models.              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define M_IDX_TEXTURE      6                /* where Tx is */

#define M_LNG_FLAT         3                /* X Y Z */
#define M_LNG_SHADED       6                /* X Y Z R G B */
#define M_LNG_TEXTURED     8                /* X Y Z R G B Tx Ty */

#define M_P              G_P                /* fixed point precision */
#define M_P_BI             8                /* for bicubic rasterization */

#define M_POINT         0x01                /* a set of points */
#define M_WIRE          0x02                /* a wire frame */
#define M_FLAT          0x04                /* constant color polygon */
#define M_SHADED        0x08                /* Gouraud shaded polygon */
#define M_TEXTURED      0x10                /* texture mapped polygon */

#define M_Z_MAPPING_SWITCH     1024         /* when switch linear/non-linear */
extern int M_force_linear_tmapping;         /* when the polygons are small */

void M_init_rendering(int type);            /* highest drawable polygon type */
void M_init_camera(unsigned char alp,unsigned char bet,unsigned char gam,
                   int x,int y,int z,int log_focus
                  );                        /* parameters of the camera */
#ifdef _PAINTER_
void M_sort_elements(int *vertices,int dimension,int *indices,int number);
#endif

/**********************************************************\
 * Polygon, either actual plane patch or part of some     *
 * curved surface approximation. Has a color and a        *
 * texture. The illumination intensities for the whole    *
 * polygon or per vertex are set by object shading        *
 * function. Vertices point to an array each element of   *
 * which contains info relative to a vertex.              *
 *                                                        *
 *  +--------------------------+                          *
 *  | m_type                   |   M_PLANNAR | M_CURVED   *
 *  | m_red m_green m_blue     |                          *
 *  |                          |                          *
 *  | m_log_texture_space_size | +-------------------+    *
 *  | m_texture----------------->|G_texture structure|    *
 *  |                          | +-------------------+    *
 *  |                          |                          *
 *  | m_no_edges               | +-----+-----+--+--+      *
 *  | m_vertices---------------->|Index| RGB |Tx|Ty|      * 
 *  |                          | | - - - - - - - - |      *
 *  |                          | |       ...       |      * 
 *  |                          | +-----------------+      *
 *  |                          |                          *
 *  |                          | +-----+                  *
 *  | m_normals----------------->|Index|                  *
 *  +--------------------------+ |- - -|                  *
 *                               | ... |                  *
 *                               +-----+                  *
\**********************************************************/

#define M_PLANNAR              0x01         /* polygon models a plane */
#define M_CURVED               0x02         /* polygon models a surface */

#define M_QUAD_XY              0x04         /* easy to find texture vectors */
#define M_QUAD_MINUS_XY        0x08         /* same but order reversed */
#define M_QUAD                 0x0c         /* any of two above */
#define M_NOT_QUAD             0xf3         /* to get rid of M_QUAD in splits */

#define M_MAX_POLYGON_VERTICES G_MAX_POLYGON_VERTICES

#define M_IDX_POLYGON_TEXTURE  4            /* where texture is in the list */
#define M_LNG_POLYGON_VERTEX   6            /* Idx R G B Tx Ty */

struct M_polygon                            /* describes one polygon */
{
 int m_type;                                /* M_PLANNAR | M_CURVED */
 int m_red,m_green,m_blue;                  /* intensity as RGB */

 int m_log_texture_space_size;              /* mapping scale */
 struct G_texture *m_texture;               /* raw data with size */

 int m_no_edges;                            /* number of edges in the polygn */
 int *m_vertices;                           /* array of indices */
 int *m_normals;                            /* normals for the vertices */
};

void M_light_polygon(struct M_polygon *polygon,int *vertices,int *normals);
void M_render_polygon(struct M_polygon *polygon,int *vertices,
                      int *colors,int *textures);

/**********************************************************\
 * Represents a single bicubic Bezier patch. It is shaded *
 * using descretely precalculated normals (appears to be  *
 * not that expensive compared to other methods). And     *
 * rasterized through turning the controls, finding the   *
 * mesh and rasterizing the small polygons one at a time. *
 *                                                        *
 *  +--------------------------+                          *
 *  | m_log_render_size        |                          *
 *  |                          |                          *
 *  | m_log_texture_space_size | +-------------------+    *
 *  | m_texture----------------->|G_texture structure|    *
 *  |                          | +-------------------+    *
 *  |                          |                          *
 *  |                   +---+---+---+                     *
 *  | m_controls        | X | Y | Z |                     *
 *  |                   |- - - - - -|                     *
 *  |                   | ...16...  |                     *
 *  |                   +-----------+                     *
 *  |                          |                          *
 *  |                          | +---+---+---+            *
 *  | m_normals----------------->| X | Y | Z |            *
 *  |                          | |- - - - - -|            *
 *  |                          | |    ...    |            *
 *  |                          | +-----------+            *
 *  |                          |                          *
 *  |                          | +-------+                * 
 *  | m_intensities------------->| R G B |                *
 *  +--------------------------+ | - - - |                *
 *                               |  ...  |                *
 *                               +-------+                *
 *                                                        *
\**********************************************************/

#define M_MAX_RENDER_SIZE 33                /* 2**5+1 */

struct M_bicubic                            /* describes one patch */
{
 int m_log_render_size;                     /* rasterized into polygons */

 int m_log_texture_space_size;              /* mapping scale */
 struct G_texture* m_texture;               /* raw data with size */

 int m_controls[16*T_LNG_VECTOR];           /* array of indices */
 int *m_normals;                            /* normals for shading */
 int *m_intensities;                        /* shading intensities */
};

void M_init_bicubic(struct M_bicubic *bicubic);
void M_light_bicubic(struct M_bicubic *bicubic,
                     int x,int y,int z,
                     unsigned char alp, 
                     unsigned char bet,
                     unsigned char gam);
void M_render_bicubic(struct M_bicubic *bicubic,
                      int x,int y,int z,
                      unsigned char alp,
                      unsigned char bet,
                      unsigned char gam);

/**********************************************************\
 * Polygon order object, a BSP tree.                      *
 *                                                        *
 *  +-----------------+    +-----------+                  *
 *  | m_root-------------->| M_polygon |                  *
 *  |                 |    +-----------+                  *
 *  |                 |    +------------------------+     *
 *  | m_positive---------->| M_polygon_object_order |     *
 *  |                 |    +------------------------+     *
 *  |                 |    +------------------------+     *
 *  | m_negative---------->| M_polygon_object_order |     *
 *  +-----------------+    +------------------------+     *
 *                                                        *
\**********************************************************/

struct M_polygon_object_order
{
 struct M_polygon *m_root;                  /* root polygon */
 struct M_polygon_object_order *m_positive; /* one sub tree */
 struct M_polygon_object_order *m_negative; /* another one */
};

/**********************************************************\
 * Surface cell, specifies a single cell of a landscape   *
 * object. Can model either plannar area or curved. In    *
 * the latter case modeled by two triangles instead of    *
 * a single rectangle.                                    *
 *                                                        *
 *  +----------------------------+                        *
 *  | m_type                     | M_PLANNAR | M_CURVED   *
 *  |                            |    +-----------+       *
 *  | m_texture---------------------->| G_texture |       *
 *  |                            |    | structure |       *
 *  | m_red_1 m_green_1 m_blue_1 |    +-----------+       * 
 *  | m_red_2 m_green_2 m_blue_2 |                        *
 *  +----------------------------+                        *
 *                                                        *
\**********************************************************/

struct M_surface_cell
{
 int m_type;                                /* M_PLANNAR | M_CURVED */

 struct G_texture *m_texture;               /* if any */

 int m_red_1,m_green_1,m_blue_1;
 int m_red_2,m_green_2,m_blue_2;
};

/**********************************************************\
 * Polygonal object, a set of polygons on the common      *
 * vertex set. Also carries the set of normals, which     *
 * are, like the vertices, referenced by their respective *
 * polygons. When a BSP tree pointer is non NULL, the     *
 * tree is used in hidden surface removal.                *
 *                                                        *
 *  +-----------------+                                   *
 *  | m_type          |   M_POLYGON_OBJECT                *
 *  |                 |   +--------------------+          *
 *  | m_material -------->|L_material structure|          *
 *  |                 |   +--------------------+          *
 *  |                 |   +------------------------+      *
 *  | m_order- - - - - - >| M_polygon_object_order |      *
 *  |                 |   +------------------------+      *
 *  |                 |                                   *
 *  | m_no_polygons   |   +---+---+  - -+                 *
 *  | m_polygons--------->| o | o | ... |                 *
 *  |                 |   +-|-+-|-+ -  -+                 *
 *  |                 |     V   V                         *
 *  |              +---------+ +---------+                *
 *  |              |M_polygon| |M_polygon|                *
 *  |              +---------+ +---------+                *
 *  |                 |                                   *
 *  | m_no_vertices   |   +---+---+---+                   *
 *  | m_vertices--------->| X | Y | Z |                   *
 *  |                 |   |- - - - - -|                   *
 *  |                 |   |    ...    |                   *
 *  |                 |   +-----------+                   *
 *  |                 |                                   *
 *  | m_no_normals    |   +---+---+---+                   *
 *  | m_normals---------->| X | Y | Z |                   *
 *  +-----------------+   |- - - - - -|                   *
 *                        |    ...    |                   *
 *                        +-----------+                   *
 *                                                        *
\**********************************************************/

#define M_POLYGON_OBJECT 0x1                /* to id it in a group */

#define M_MAX_OBJECT_VERTICES 1024          /* size of tmp structures */
#define M_MAX_OBJECT_POLYGONS  256

struct M_polygon_object
{
 int m_type;                                /* always M_POLYGON_OBJECT */
 struct L_material* m_material;             /* material coefs */

 struct M_polygon_object_order *m_order;    /* BSP tree, if any */
 int m_no_polygons;
 struct M_polygon **m_polygons;             /* array of polygons */

 int m_no_vertices;
 int *m_vertices;                           /* array of coordinates */

 int m_no_normals;
 int *m_normals;                            /* array of coordinates */
};

void M_init_polygon_object(struct M_polygon_object *object);
void M_light_polygon_object(struct M_polygon_object *object,
                            int x,int y,int z,
                            unsigned char alp,
                            unsigned char bet,
                            unsigned char gam
                           );               /* object moves and rotates */
void M_render_polygon_object(struct M_polygon_object *object,
                             int x,int y,int z,
                             unsigned char alp,
                             unsigned char bet,
                             unsigned char gam);

/**********************************************************\
 * Represents a collection of bicubic patches, hidden     *
 * surface elemination with _PAINTER_ option achieved     *
 * by back to front ordering of the patches (since all    *
 * patches are roughly the same size, should work).       *
 *                                                        *
 *  +--------------------------+                          *
 *  | m_type                   | M_BICUBIC_OBJECT         *
 *  |                          |  +--------------------+  *
 *  | m_material ---------------->|L_material structure|  *
 *  |                          |  +--------------------+  *
 *  |                          |                          *
 *  | m_no_patches             |  +---+---+  - -+         *
 *  | m_patches------------------>| o | o | ... |         *
 *  |                          |  +-|-+-|-+ -  -+         *
 *  |                          |    V   V                 *
 *  |                      +---------+ +---------+        *
 *  |                      | M_patch | | M_patch |        *
 *  |                      +---------+ +---------+        *
 *  |                          |                          *
 *  |                          |  +---+---+---+           *
 *  | m_centres ----------------->| X | Y | Z |           *
 *  |                          |  +- - - - - -+           *
 *  +--------------------------+  |    ...    |           *
 *                                +-----------+           *
 *                                                        *
\**********************************************************/

#define M_BICUBIC_OBJECT 0x2                /* to id it in a group */

#define M_MAX_PATCHES    128                /* upper for tmp structures */

struct M_bicubic_object                     /* describes one polygon */
{
 int m_type;                                /* always M_POLYGON_OBJECT */
 struct L_material* m_material;             /* material coefs */

 int m_no_patches;
 struct M_bicubic **m_patches;              /* pointers to patches */
 int *m_centres;                            /* centres of the patches */
};

void M_init_bicubic_object(struct M_bicubic_object *bicubic);
void M_light_bicubic_object(struct M_bicubic_object *object,
                            int x,int y,int z,
                            unsigned char alp,
                            unsigned char bet,
                            unsigned char gam);
void M_render_bicubic_object(struct M_bicubic_object *object,
                             int x,int y,int z,
                             unsigned char alp,
                             unsigned char bet,
                             unsigned char gam);

/**********************************************************\
 * Surface object, a height field with corresponding      *
 * sets of vertices and normals.                          *
 *                                                        *
 *  +--------------------+  +--------------------+        *
 *  | m_material ---------->|L_material structure|        *
 *  |                    |  +--------------------+        *
 *  |                    |                                *
 *  |                    | +------+---------+---------+   *
 *  | m_orders------------>|m_cell|m_vertex1|m_vertex2|   *
 *  |                    | +- - - - - - - - - - - - - +   *
 *  | m_total_size       | |            ...           |   *
 *  | m_display_size     | +--------------------------+   *
 *  | m_cell_length      |                                *
 *  |                    | +---+-------+                  *
 *  | m_vertices---------->| Y | R G B |                  *
 *  |                    | |- - - - - -|                  *
 *  |                    | |    ...    |                  *
 *  |                    | +-----------+                  *
 *  |                    |                                *
 *  |                    | +----------------+             *
 *  | m_cells------------->|M_surface_cell  |             *
 *  |                    | |structures      |             *
 *  |                    | |- - - - - - - - |             *
 *  |                    | |       ...      |             *
 *  |                    | +----------------+             *
 *  |                    |                                *
 *  |                    | +---+---+---+                  *
 *  | m_normals----------->| X | Y | Z |                  *
 *  +--------------------+ |- - - - - -|                  *
 *                         |    ...    |                  *
 *                         +-----------+                  *
 *                                                        *
\**********************************************************/

#define M_TEXTURE_LENGTH       127          /* size of texture in the cell */
#define M_LOG_TEXTURE_LENGTH     7          /* log of the number above */
#define M_MAX_SURFACE_VERTICES  20          /* maximum display size */
#define M_LNG_SURFACE_VERTEX     4          /* Y R G B */

struct M_surface_object
{
 struct L_material* m_material;             /* material coefs */

 int *m_orders;                             /* array of order indices */
 int m_total_size;                          /* dimension of the struct */
 int m_display_size;                        /* how many of those displayed */
 int m_cell_length;                         /* size for one cell */

 int *m_vertices;                           /* square array of vertices */
 struct M_surface_cell *m_cells;            /* square array of cells */
 int *m_normals;
};

void M_init_surface_object(struct M_surface_object *object);
void M_light_surface_object(struct M_surface_object *object);
void M_render_surface_object(struct M_surface_object *object,
                             int xcell,int zcell
                            );              /* render around [xc,zc] cell */

/**********************************************************\
 * Group of objects, each specified by position and       *
 * orientation. Also contains a set of light sources.     *
 *                                                        *
 *  +-------------------+                                 *
 *  | m_no_objects      |                                 *
 *  |                   |   +---+---+- -- --+             *
 *  | m_objects ----------->| o | o |  ...  |             *
 *  |                   |   +-|-+-|-+ - -- -+             *
 *  |                   |     V   V                       *
 *  |        +------------------+ +------------------+    *
 *  |        | M_generic_object | | M_generic_object |    *
 *  |        +------------------+ +------------------+    *
 *  |                   |                                 *
 *  |                   |   +---+---+---+                 *
 *  | m_centres ----------->| X | Y | Z |                 *
 *  |                   |   +- - - - - -+                 *
 *  |                   |   |    ...    |                 *
 *  |                   |   +-----------+                 *
 *  |                   |                                 *
 *  |                   |   +---+---+---+                 *
 *  | m_orientations ------>|alp|bet|gam|                 *
 *  |                   |   +- - - - - -+                 *
 *  |                   |   |    ...    |                 *
 *  |                   |   +-----------+                 *
 *  |                   |                                 *
 *  | m_no_lights       |   +---+---+-  --+               *
 *  | m_lights ------------>| o | o | ... |               *
 *  |                   |   +-|-+-|-+--  -+               *
 *  |                   |     V   V                       *
 *  |               +----------+ +---------+              *
 *  +---------------| L_light  | | L_light |              *
 *                  +----------+ +---------+              *
\**********************************************************/

#define M_MAX_GROUP_OBJECTS 32              /* defines size for tmp structs */

struct M_generic_object
{
 int m_type;                                /* polygonal or bicubic */
};

struct M_group
{
 int m_no_objects;
 struct M_generic_object **m_objects;       /* polygon or bicubic objects */
 int *m_centres;                            /* centres of the objects */
 unsigned char *m_orientations;             /* their orientations */

 int m_no_lights;                           /* lighting belongs to the group */
 struct L_light **m_lights; 
};

void M_init_group(struct M_group *group);   /* each object in the group */
void M_light_group(struct M_group *group);  /* group doesn't move */
void M_render_group(struct M_group *object,
                    int x,int y,int z);     /* render at x y z */

/**********************************************************/

#endif
