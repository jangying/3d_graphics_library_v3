/** 3Dgpl3 ************************************************\
 * Functions for bicubic patches.                         *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_               Depth array;                 *
 *  _PAINTER_                Back to front rendering.     *
 *                                                        *
 * Defines:                                               *
 *  M_init_bicubic           Allocate structure elements; *
 *  M_light_bicubic          Compute shading intensities; *
 *  M_render_bicubic         Rendering as set of polygons;*
 *                                                        *
 *  M_init_bicubic_object    Every patch in the object;   *
 *  M_light_bicubic_object   Light the patches;           *
 *  M_render_bicubic_object  Rendering as set of patches. *
 *                                                        *
 * Internals:                                             *
 *  MI_render_square         Single square of the patch;  *
 *  MI_find_bezier_points    Finding points on the curve; *
 *  MI_find_bezier_tangents  Finding the tengents.        *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../graphics/graphics.h"           /* 2D rendering */
#include "../trans/trans.h"                 /* 3D transformations */
#include "../clipper/clipper.h"             /* 2D/3D clipping */
#include "../engine/engine.h"               /* 3D engine */
#include "../light/light.h"                 /* lighting engine */
#include <stdlib.h>                         /* NULL */

int M_square_v[30]=                         /* static data for a square */
{
 2,0,0,0,0,0,
 3,0,0,0,0,0,
 1,0,0,0,0,0,
 0,0,0,0,0,0,
 2,0,0,0,0,0
};
struct M_polygon M_square={M_QUAD_XY,       /* type */
                           0,0,0,           /* color */
                           0,NULL,          /* texture */
                           4,M_square_v,NULL
                          };

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Rendering a single square of tesselated     *
 * --------- bicubic.                                    *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MI_render_square(int no1,int no2,int* vertices,int *intensities,
                      int *textures)
{
 int tmp_vert[T_LNG_VECTOR*4];              /* X Y Z */
 int tmp_colr[12];                          /* intensity or R G B */
 int tmp_texture[8];                        /* Tx Ty */

 memcpy(tmp_vert,vertices+no1*T_LNG_VECTOR,T_LNG_VECTOR*2*sizeof(int));
 memcpy(tmp_vert+T_LNG_VECTOR*2,vertices+no2*T_LNG_VECTOR,
        T_LNG_VECTOR*2*sizeof(int));
                                            /* getting rid of fractions */
 tmp_vert[0]>>=M_P_BI; tmp_vert[1]>>=M_P_BI; tmp_vert[2]>>=M_P_BI;
 tmp_vert[4]>>=M_P_BI; tmp_vert[5]>>=M_P_BI; tmp_vert[6]>>=M_P_BI;
 tmp_vert[8]>>=M_P_BI; tmp_vert[9]>>=M_P_BI; tmp_vert[10]>>=M_P_BI;
 tmp_vert[12]>>=M_P_BI; tmp_vert[13]>>=M_P_BI; tmp_vert[14]>>=M_P_BI;

 if(intensities!=NULL)
 {
  memcpy(tmp_colr,intensities+no1*3,6*sizeof(int));
  memcpy(tmp_colr+6,intensities+no2*3,6*sizeof(int));

  memcpy(tmp_texture,textures+no1*2,4*sizeof(int));
  memcpy(tmp_texture+4,textures+no2*2,4*sizeof(int));
 }

 M_square.m_red=tmp_colr[0];
 M_square.m_green=tmp_colr[1];
 M_square.m_blue=tmp_colr[2];

 M_render_polygon(&M_square,tmp_vert,tmp_colr,tmp_texture);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Filling an array with the coordinates of    *
 * --------- points on the Bezier curve cpecified        *
 *           through its controls b1,...,b4.             *
 *                                                       *
 * RECURSIVE: Calls itself twice.                        *
 * ----------                                            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MI_evaluate_bezier_points(int *b1,int *b2,int *b3,int *b4,
                              int *points,int dimension,int length,int top,
                              int rendering)
{
 int b12[T_LNG_VECTOR],b23[T_LNG_VECTOR],b34[T_LNG_VECTOR];
 int b123[T_LNG_VECTOR],b234[T_LNG_VECTOR],*b1234;

 if(top)                                    /* during top level call */
 {
  if(rendering)
  {                                         /* increasing precision */
   b1[0]<<=M_P_BI; b1[1]<<=M_P_BI; b1[2]<<=M_P_BI;
   b2[0]<<=M_P_BI; b2[1]<<=M_P_BI; b2[2]<<=M_P_BI;
   b3[0]<<=M_P_BI; b3[1]<<=M_P_BI; b3[2]<<=M_P_BI;
   b4[0]<<=M_P_BI; b4[1]<<=M_P_BI; b4[2]<<=M_P_BI;
  }
  memcpy(points,b1,3*sizeof(int));          /* setting border values */
  memcpy(points+(length-1)*dimension,b4,3*sizeof(int));
 }

 if(length>2)
 {
  b1234=points+(length/2)*dimension;

  b12[0]=(b1[0]+b2[0])/2; b12[1]=(b1[1]+b2[1])/2; b12[2]=(b1[2]+b2[2])/2;
  b23[0]=(b2[0]+b3[0])/2; b23[1]=(b2[1]+b3[1])/2; b23[2]=(b2[2]+b3[2])/2;
  b34[0]=(b3[0]+b4[0])/2; b34[1]=(b3[1]+b4[1])/2; b34[2]=(b3[2]+b4[2])/2;

  b123[0]=(b12[0]+b23[0])/2; b123[1]=(b12[1]+b23[1])/2; b123[2]=(b12[2]+b23[2])/2;
  b234[0]=(b23[0]+b34[0])/2; b234[1]=(b23[1]+b34[1])/2; b234[2]=(b23[2]+b34[2])/2;

  b1234[0]=(b123[0]+b234[0])/2;
  b1234[1]=(b123[1]+b234[1])/2;
  b1234[2]=(b123[2]+b234[2])/2;

  MI_evaluate_bezier_points(b1,b12,b123,b1234,points,dimension,length/2+1,0,rendering);
  MI_evaluate_bezier_points(b1234,b234,b34,b4,b1234,dimension,length/2+1,0,rendering);
 }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Filling an array with the tangents of the   *
 * --------- Bezier curve cpecified through its          *
 *           controls b1,...,b4.                         *
 *                                                       *
 * RECURSIVE: Calls itself twice.                        *
 * ----------                                            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MI_evaluate_bezier_tangents(int *b1,int *b2,int *b3,int *b4,
                                 int *tangents,int dimension,int length,
                                 int top)
{
 int b12[T_LNG_VECTOR],b23[T_LNG_VECTOR],b34[T_LNG_VECTOR];
 int b123[T_LNG_VECTOR],b234[T_LNG_VECTOR],b1234[T_LNG_VECTOR],*mid;

 if(top)                                    /* during top level call */
 {
  T_vector(b1,b2,tangents,3);               /* setting border values */
  T_vector(b3,b4,tangents+(length-1)*dimension,3);
 }

 if(length>2)
 {
  mid=tangents+(length/2)*dimension;

  b12[0]=(b1[0]+b2[0])/2; b12[1]=(b1[1]+b2[1])/2; b12[2]=(b1[2]+b2[2])/2;
  b23[0]=(b2[0]+b3[0])/2; b23[1]=(b2[1]+b3[1])/2; b23[2]=(b2[2]+b3[2])/2;
  b34[0]=(b3[0]+b4[0])/2; b34[1]=(b3[1]+b4[1])/2; b34[2]=(b3[2]+b4[2])/2;

  b123[0]=(b12[0]+b23[0])/2; b123[1]=(b12[1]+b23[1])/2; b123[2]=(b12[2]+b23[2])/2;
  b234[0]=(b23[0]+b34[0])/2; b234[1]=(b23[1]+b34[1])/2; b234[2]=(b23[2]+b34[2])/2;

  T_vector(b123,b234,mid,3);

  b1234[0]=(b123[0]+b234[0])/2;
  b1234[1]=(b123[1]+b234[1])/2;
  b1234[2]=(b123[2]+b234[2])/2;

  MI_evaluate_bezier_tangents(b1,b12,b123,b1234,tangents,dimension,length/2+1,0);
  MI_evaluate_bezier_tangents(b1234,b234,b34,b4,mid,dimension,length/2+1,0);
 }
}

/**********************************************************\
 * Allocating the intensity table based on the foreseen   *
 * number of polygons into which the patch will be        *
 * tesselated.                                            *
\**********************************************************/

void M_init_bicubic(struct M_bicubic *bicubic)
{
 int tangents1[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tangents2[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb1[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb2[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb3[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb4[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int *c=bicubic->m_controls;
 int i,size=(1<<bicubic->m_log_render_size)+1;

 if(size>=M_MAX_RENDER_SIZE)
  HW_error("(Engine) Can't subdivide a bicubic this much.\n");

 bicubic->m_normals=(int*)malloc(sizeof(int)*T_LNG_VECTOR*size*size);
 bicubic->m_intensities=(int*)malloc(sizeof(int)*3*size*size);
 if((bicubic->m_normals==NULL)||(bicubic->m_intensities==NULL))
  HW_error("(Engine) Not enough memory.\n");

 MI_evaluate_bezier_points(c,c+T_LNG_VECTOR,c+T_LNG_VECTOR*2,
                           c+T_LNG_VECTOR*3,tmpb1,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*4,c+T_LNG_VECTOR*5,c+T_LNG_VECTOR*6,
                           c+T_LNG_VECTOR*7,tmpb2,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*8,c+T_LNG_VECTOR*9,c+T_LNG_VECTOR*10,
                           c+T_LNG_VECTOR*11,tmpb3,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*12,c+T_LNG_VECTOR*13,c+T_LNG_VECTOR*14,
                           c+T_LNG_VECTOR*15,tmpb4,T_LNG_VECTOR,size,1,0);

 for(i=0;i<size;i++)
  MI_evaluate_bezier_tangents(tmpb1+i*T_LNG_VECTOR,tmpb2+i*T_LNG_VECTOR,
                              tmpb3+i*T_LNG_VECTOR,tmpb4+i*T_LNG_VECTOR,
                              tangents1+i*T_LNG_VECTOR,T_LNG_VECTOR*size,
                              size,1);

 MI_evaluate_bezier_points(c,c+T_LNG_VECTOR*4,c+T_LNG_VECTOR*8,
                           c+T_LNG_VECTOR*12,tmpb1,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*1,c+T_LNG_VECTOR*5,c+T_LNG_VECTOR*9,
                           c+T_LNG_VECTOR*13,tmpb2,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*2,c+T_LNG_VECTOR*6,c+T_LNG_VECTOR*10,
                           c+T_LNG_VECTOR*14,tmpb3,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*3,c+T_LNG_VECTOR*7,c+T_LNG_VECTOR*11,
                           c+T_LNG_VECTOR*15,tmpb4,T_LNG_VECTOR,size,1,0);

 for(i=0;i<size;i++)
  MI_evaluate_bezier_tangents(tmpb1+i*T_LNG_VECTOR,tmpb2+i*T_LNG_VECTOR,
                              tmpb3+i*T_LNG_VECTOR,tmpb4+i*T_LNG_VECTOR,
                              tangents2+i*size*T_LNG_VECTOR,T_LNG_VECTOR,
                              size,1);

 for(i=0;i<size*size;i++)
  T_normal_vectors(tangents1+i*T_LNG_VECTOR,tangents2+i*T_LNG_VECTOR,
                  bicubic->m_normals+i*T_LNG_VECTOR);
}

/**********************************************************\
 * Using precomputed normals to light the bicubic         *
\**********************************************************/

void M_light_bicubic(struct M_bicubic *bicubic,
                     int x,int y,int z,
                     unsigned char alp,
                     unsigned char bet,
                     unsigned char gam)
{
 int c[16*T_LNG_VECTOR];
 int points[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int normals[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb1[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb2[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb3[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb4[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int *intensity,*position,*normal;
 int i,size=(1<<bicubic->m_log_render_size)+1;

 if(size>M_MAX_RENDER_SIZE)
  HW_error("(Engine) Can't subdivide a bicubic this much.\n");

 T_set_self_rotation(alp,bet,gam);          /* shading happens in world space */
 T_self_rotation(bicubic->m_normals,normals,size*size);
 T_self_rotation(bicubic->m_controls,c,16);
 T_translation(c,c,16,x,y,z);

 MI_evaluate_bezier_points(c,c+T_LNG_VECTOR*4,c+T_LNG_VECTOR*8,
                           c+T_LNG_VECTOR*12,tmpb1,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*1,c+T_LNG_VECTOR*5,c+T_LNG_VECTOR*9,
                           c+T_LNG_VECTOR*13,tmpb2,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*2,c+T_LNG_VECTOR*6,c+T_LNG_VECTOR*10,
                           c+T_LNG_VECTOR*14,tmpb3,T_LNG_VECTOR,size,1,0);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*3,c+T_LNG_VECTOR*7,c+T_LNG_VECTOR*11,
                           c+T_LNG_VECTOR*15,tmpb4,T_LNG_VECTOR,size,1,0);

 for(i=0;i<size;i++)
  MI_evaluate_bezier_points(tmpb1+i*T_LNG_VECTOR,tmpb2+i*T_LNG_VECTOR,
                            tmpb3+i*T_LNG_VECTOR,tmpb4+i*T_LNG_VECTOR,
                            points+i*size*T_LNG_VECTOR,T_LNG_VECTOR,size,1,0);

 intensity=bicubic->m_intensities;
 position=points;
 normal=normals;
 for(i=0;i<size*size;i++)
 {
  L_light_vertex(intensity,position,normal);
  intensity+=3;
  position+=T_LNG_VECTOR;
  normal+=T_LNG_VECTOR;
 }
}

/**********************************************************\
 * Rendering a bicubic by tesselating it into polygons.   *
\**********************************************************/

void M_render_bicubic(struct M_bicubic *bicubic,
                      int x,int y,int z,
                      unsigned char alp,
                      unsigned char bet,
                      unsigned char gam)
{
 int c[16*T_LNG_VECTOR];
 int points[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int textures[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE*2];
 int numbers[M_MAX_RENDER_SIZE*M_MAX_RENDER_SIZE];
 int tmpb1[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb2[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb3[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int tmpb4[M_MAX_RENDER_SIZE*T_LNG_VECTOR];
 int *texture,incr,cur_x,cur_y,max;
 int i,j,size=(1<<bicubic->m_log_render_size)+1;

 if(size>M_MAX_RENDER_SIZE)
  HW_error("(Engine) Can't subdivide a bicubic this much.\n");

 T_set_self_rotation(alp,bet,gam);          /* the transformations */
 T_concatinate_self_world(x+M_camera_x,y+M_camera_y,z+M_camera_z);
 T_concatinated_rotation(bicubic->m_controls,c,16);

 incr=bicubic->m_log_texture_space_size-bicubic->m_log_render_size;

 if(incr<=0) HW_error("(Engine) Too small a texture to fit onto a bicubic.\n");

 M_square.m_log_texture_space_size=incr;
 M_square.m_texture=bicubic->m_texture;     /* transfer info to a square */

 incr=1<<incr;
 max=1<<bicubic->m_log_texture_space_size;
 texture=textures;
 for(i=0,cur_y=max;i<size;i++,cur_y-=incr)  /* building a set of texture */
 {                                          /* coordinates */
  for(j=0,cur_x=0;j<size;j++,cur_x+=incr)
  {
   *texture++=cur_x;
   *texture++=cur_y;
  }
 }

 MI_evaluate_bezier_points(c,c+T_LNG_VECTOR*4,c+T_LNG_VECTOR*8,
                           c+T_LNG_VECTOR*12,tmpb1,T_LNG_VECTOR,size,1,1);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*1,c+T_LNG_VECTOR*5,c+T_LNG_VECTOR*9,
                           c+T_LNG_VECTOR*13,tmpb2,T_LNG_VECTOR,size,1,1);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*2,c+T_LNG_VECTOR*6,c+T_LNG_VECTOR*10,
                           c+T_LNG_VECTOR*14,tmpb3,T_LNG_VECTOR,size,1,1);
 MI_evaluate_bezier_points(c+T_LNG_VECTOR*3,c+T_LNG_VECTOR*7,c+T_LNG_VECTOR*11,
                           c+T_LNG_VECTOR*15,tmpb4,T_LNG_VECTOR,size,1,1);

 for(i=0;i<size;i++)                        /* tesselating the bicubic */
  MI_evaluate_bezier_points(tmpb1+i*T_LNG_VECTOR,tmpb2+i*T_LNG_VECTOR,
                            tmpb3+i*T_LNG_VECTOR,tmpb4+i*T_LNG_VECTOR,
                            points+i*size*T_LNG_VECTOR,T_LNG_VECTOR,size,1,0
                           );               /* beware 0 at the end - */
                                            /* precision increased just once */
 for(i=0;i<size*size;i++) numbers[i]=i;     /* prepare for sorting */

#if defined(_PAINTER_)
 M_sort_elements(points+2,T_LNG_VECTOR,numbers,size*size);
#endif

 for(i=0;i<size*size;i++)                   /* rendering back to front */
  if((numbers[i]%size!=size-1)&&(numbers[i]/size!=size-1))
   MI_render_square(numbers[i],numbers[i]+size,points,
                    bicubic->m_intensities,textures);
}

/**********************************************************\
 * Initializing all patches in the object, computing      *
 * approximate centres of the patches.                    *
\**********************************************************/

void M_init_bicubic_object(struct M_bicubic_object *object)
{
 int i,j;

 object->m_centres=(int*)malloc(sizeof(int)*T_LNG_VECTOR*object->m_no_patches);
 if(object->m_centres==NULL)
  HW_error("(Engine) Not enough memory.\n");

 for(i=0;i<object->m_no_patches;i++)
 {
  M_init_bicubic(object->m_patches[i]);

  object->m_centres[i*T_LNG_VECTOR]=0;
  object->m_centres[i*T_LNG_VECTOR+1]=0;
  object->m_centres[i*T_LNG_VECTOR+2]=0;
  for(j=0;j<16;j++)
  {
   object->m_centres[i*T_LNG_VECTOR]+=object->m_patches[i]->m_controls[j*T_LNG_VECTOR];
   object->m_centres[i*T_LNG_VECTOR+1]+=object->m_patches[i]->m_controls[j*T_LNG_VECTOR+1];
   object->m_centres[i*T_LNG_VECTOR+2]+=object->m_patches[i]->m_controls[j*T_LNG_VECTOR+2];
  }
  object->m_centres[i*T_LNG_VECTOR]/=16;
  object->m_centres[i*T_LNG_VECTOR+1]/=16;
  object->m_centres[i*T_LNG_VECTOR+2]/=16;
 }
}

/**********************************************************\
 * Shading all patches in the object.                     *
\**********************************************************/

void M_light_bicubic_object(struct M_bicubic_object *object,
                            int x,int y,int z,
                            unsigned char alp,
                            unsigned char bet,
                            unsigned char gam)
{
 int i;

 L_init_material(object->m_material);

 for(i=0;i<object->m_no_patches;i++)
  M_light_bicubic(object->m_patches[i],x,y,z,alp,bet,gam);
}

/**********************************************************\
 * Rendering all patches. In the case of _PAINTER_        *
 * performing sorting of the patches centres first and    *
 * rendering in the back to front order.                  *
\**********************************************************/

void M_render_bicubic_object(struct M_bicubic_object *object,
                             int x,int y,int z,
                             unsigned char alp,
                             unsigned char bet,
                             unsigned char gam)
{
 int i;
#if defined(_PAINTER_)
 int centres[M_MAX_PATCHES*T_LNG_VECTOR];
#endif
 int numbers[M_MAX_PATCHES];
 int oldtmap=M_force_linear_tmapping;
 M_force_linear_tmapping=1;                 /* polygons will be small enough */

 if(object->m_no_patches>M_MAX_PATCHES)
  HW_error("(Engine) Too many bicubic patches per object.\n");

 for(i=0;i<object->m_no_patches;i++) numbers[i]=i;

#if defined(_PAINTER_)                      /* probably there are few patches */
 T_set_self_rotation(alp,bet,gam);          /* don't concatinate transforms */
 T_self_rotation(object->m_centres,centres,object->m_no_patches);
 T_world_rotation(centres,centres,object->m_no_patches);
 M_sort_elements(centres+2,T_LNG_VECTOR,numbers,object->m_no_patches);
#endif

 for(i=0;i<object->m_no_patches;i++)
  M_render_bicubic(object->m_patches[numbers[i]],x,y,z,alp,bet,gam);

 M_force_linear_tmapping=oldtmap;
}

/**********************************************************/
