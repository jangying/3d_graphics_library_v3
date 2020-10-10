#if ((!defined(_Z_BUFFER_)) && (!defined(_PAINTER_)))
 #error Either _Z_BUFFER_ or _PAINTER_ must be declared.
#endif
#if !defined(_GRAPHICS_H_)
#define _GRAPHICS_H_

/** 3Dgpl3 ************************************************\
 * Header for 2D graphics stuff.                          *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_               Depth array;                 *
 *  _PAINTER_                Back to front order;         *
 *                                                        *
 * Files:                                                 *
 *  grp-base.c               Basic graphics;              *
 *  grp-text.c               Text;                        *
 *  grp-poly.c               Polygon rendering.           * 
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* HW_pixel */

/**********************************************************/

#define G_P                     16          /* fixed point precision */
#define G_MAX_POLYGON_VERTICES  16          /* points in a polygon */

#if defined(_Z_BUFFER_)                     /* Z R G B Tx Ty */
 #define G_LNG_FLAT     3                   /* X Y Z */
 #define G_LNG_SHADED   6                   /* X Y Z R G B */
 #define G_LNG_TEXTURED 8                   /* X Y Z R G B Tx Ty */
#endif
#if defined(_PAINTER_)                      /* R G B Tx Ty */
 #define G_LNG_FLAT     2                   /* X Y */
 #define G_LNG_SHADED   5                   /* X Y R G B */
 #define G_LNG_TEXTURED 7                   /* X Y R G B Tx Ty */
#endif

/**********************************************************/

extern HW_pixel_ptr G_c_buffer;             /* color buffer */
#if defined(_Z_BUFFER_)
extern int *G_z_buffer;                     /* Z buffer */
#endif

/**********************************************************\
 * Texture, only square textures with the size            *
 * which is a power of two are allowed.                   *
 *                                                        *
 *  +--------------------+     +-------------+            *
 *  | g_red ------------------>| red channel |            *
 *  |                    |     +-------------+            *
 *  |                    |                                *
 *  |                    |     +---------------+          *
 *  | g_green ---------------->| green channel |          *
 *  |                    |     +---------------+          *
 *  |                    |                                *
 *  |                    |     +--------------+           *
 *  | g_blue ----------------->| blue channel |           *
 *  |                    |     +--------------+           *
 *  | g_log_texure_size  |                                *
 *  +--------------------+                                *
 *                                                        *
\**********************************************************/

struct G_texture                            /* texture map */
{
 unsigned char *g_red,*g_green,*g_blue;     /* color channels */
 int g_log_texture_size;                    /* size */
};

/**********************************************************/

void G_init_graphics(void);
void G_clear(int red,int green,int blue);
void G_point(int *vertex,int red,int green,int blue);
void G_line(int *vertex1,int *vertex2,int red,int green,int blue);

void G_text(int x,int y,char *txt,int red,int green,int blue);

void G_flat_polygon(int *edges,int length,int red,int green,int blue);
void G_shaded_polygon(int *edges,int length);
void G_lin_textured_polygon(int *edges,int length,struct G_texture* texture);
void G_prp_textured_polygon(int *edges,int length,
                            int *O,int *u,int *v,int texture_space_size,
                            struct G_texture* texture);

/*********************************************************/

#endif
