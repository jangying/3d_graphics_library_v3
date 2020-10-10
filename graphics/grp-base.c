/** 3Dgpl3 ************************************************\
 * 2D basic graphics.                                     *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_               Depth array;                 *
 *  _PAINTER_                Back front order.            *
 *                                                        *
 * Defines:                                               *
 *  G_init_graphics          Initializing graphics;       *
 *  G_clear                  Clearing the bitmap;         *
 *  G_point                  Pixel into the bitmap;       *
 *  G_line                   Line into a bitmap.          *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* hardware specific stuff */
#include "../clipper/clipper.h"             /* 2D clipping routines */
#include "../graphics/graphics.h"           /* graphics functions */
#include <stdlib.h>                         /* malloc */
#include <limits.h>                         /* INT_MAX */

/**********************************************************/

HW_pixel_ptr G_c_buffer;                    /* the bitmap's bits */
#if defined(_Z_BUFFER_)
 int* G_z_buffer;                           /* Z buffer */
#endif

/**********************************************************\
 * Allocating space for the bitmap.                       *
 *                                                        *
 * RETURNS: Pointer to the allocated bitmap.              *
 * --------                                               *
 * SETS: G_c_buffer,G_c_buffer_size,G_z_buffer,           *
 * ----- G_z_buffer_size                                  *
\**********************************************************/

void G_init_graphics(void)
{
 G_c_buffer=(HW_pixel_ptr)malloc(HW_image_size*HW_pixel_size);
 if(G_c_buffer==NULL) HW_error("(Graphics) Not enough memory.\n");

#if defined(_Z_BUFFER_)                     /* for both pages */
 G_z_buffer=(int*)malloc(HW_image_size*sizeof(int));
 if(G_z_buffer==NULL) HW_error("(Graphics) Not enough memory.\n");
#endif
}

/**********************************************************\
 * Clearing the bitmap with the specified color.          *
\**********************************************************/

void G_clear(int red,int green,int blue)
{
 int i;
 
 for(i=0;i<HW_image_size*HW_pixel_size;i+=HW_pixel_size)
  HW_pixel(G_c_buffer+i,red,green,blue);

#if defined(_Z_BUFFER_)
 for(i=0;i<HW_image_size;i++) G_z_buffer[i]=INT_MAX;
#endif
}

/**********************************************************\
 * Rendering a dot.                                       *
\**********************************************************/

void G_point(int *vertex,int red,int green,int blue)
{
 long pos;

 if((vertex[0]>=0)&&(vertex[0]<HW_screen_x_size)&&
    (vertex[1]>=0)&&(vertex[1]<HW_screen_y_size))
 {
  pos=vertex[1]*HW_screen_x_size+vertex[0];

#if defined(_Z_BUFFER_)
  if(vertex[2]<G_z_buffer[pos])             /* doing Z check */
#endif

  HW_pixel(G_c_buffer+pos*HW_pixel_size,red,green,blue);
 }
}

/**********************************************************\
 * Rendering a line.                                      *
\**********************************************************/

void G_line(int *vertex1,int *vertex2,int red,int green,int blue)
{
 int inc_ah,inc_al;
 int i;
 int *v1,*v2,pos;
 int dx,dy,long_d,short_d;
 int d,add_dh,add_dl;
 int inc_xh,inc_yh,inc_xl,inc_yl;
 HW_pixel_ptr adr_c=G_c_buffer;
#if defined(_Z_BUFFER_)
 int* adr_z=G_z_buffer;
 int cur_z,inc_z;
#endif
 v1=(int*)vertex1;
 v2=(int*)vertex2;

 if(C_line_x_clipping(&v1,&v2,2))           /* horizontal clipping */
 {
  if(C_line_y_clipping(&v1,&v2,2))          /* vertical clipping */
  {
   dx=v2[0]-v1[0]; dy=v2[1]-v1[1];          /* ranges */

   if(dx<0){dx=-dx; inc_xh=-1; inc_xl=-1;}  /* making sure dx and dy >0 */
   else    {        inc_xh=1;  inc_xl=1; }  /* adjusting increments */

   if(dy<0){dy=-dy;inc_yh=-HW_screen_x_size;
                   inc_yl=-HW_screen_x_size;
           }                                /* to get to the neighboring */
   else    {       inc_yh= HW_screen_x_size;
                   inc_yl= HW_screen_x_size;
           }
   if(dx>dy){long_d=dx;short_d=dy;inc_yl=0;}/* long range,&make sure either */
   else     {long_d=dy;short_d=dx;inc_xl=0;}/* x or y is changed in L case */

   inc_ah=inc_xh+inc_yh;
   inc_al=inc_xl+inc_yl;                    /* increments for point address */

   d=2*short_d-long_d;                      /* initial value of d */
   add_dl=2*short_d;                        /* d adjustment for H case */
   add_dh=2*(short_d-long_d);               /* d adjustment for L case */
   pos=v1[1]*HW_screen_x_size+v1[0];
   adr_c+=pos*HW_pixel_size;                /* address of first point */

#if defined(_Z_BUFFER_)
   adr_z+=pos;                              /* same for z-buffer */
   cur_z=v1[2];
   if(long_d!=0) inc_z=(v2[2]-cur_z)/long_d;
#endif

   for(i=0;i<=long_d;i++)                   /* for all points in long range */
   {
#if defined(_Z_BUFFER_)
    if(cur_z<*adr_z)
#endif
    HW_pixel(adr_c,red,green,blue);         /* ploting a pixel */

    if(d>=0)
    {
     d+=add_dh;
     adr_c+=inc_ah*HW_pixel_size;           /* new in the color-buffer */
#if defined(_Z_BUFFER_)
     adr_z+=inc_ah;                         /* new in Z-buffer */
#endif
    }                                       /* previous point was H type */
    else
    {
     d+=add_dl;
     adr_c+=inc_al*HW_pixel_size;           /* new in the color-buffer */
#if defined(_Z_BUFFER_)
     adr_z+=inc_al;                         /* new in Z-buffer */
#endif
    }                                       /* previous point was L type */
#if defined(_Z_BUFFER_)
    cur_z+=inc_z;
#endif
   }
  }
 }
}

/**********************************************************/
