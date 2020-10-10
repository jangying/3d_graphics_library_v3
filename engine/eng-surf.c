/** 3Dgpl3 ************************************************\
 * Functions for surface objects.                         *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_              Depth array;                  *
 *  _PAINTER_               Back-front order.             *
 *                                                        *
 * Defines:                                               *
 *  M_init_surface_object   Normals and order list;       *
 *  M_light_surface_object  Calculates intensities;       *
 *  M_render_surface_object Renders a height field.       *
 *                                                        *
 * Internals:                                             *
 *  MI_render_surface_cell  One cell of the surface.      *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* fast memory moves */
#include "../graphics/graphics.h"           /* 2D rendering */
#include "../trans/trans.h"                 /* 3D transformations */
#include "../engine/engine.h"               /* 3D engine */
#include "../light/light.h"                 /* L_light_vertex */
#include <stdlib.h>                         /* NULL */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                       *
 * v2->   2           3     Using static data to render  *
 *        *---------->*     a surface cell as either one *
 * ^Z     ^ \      #2 |     polygon with vertices 2-3-1-0*
 * |  X   |     \     |     or as two triangles with     *
 * +-->   | #1      \ V     vertices 2-3-1 and 1-0-2.    *
 *        *<----------*                                  *
 * v1->   0           1                                  *
 *                                                       *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int M_flat_v[30]=                           /* static data for a flat */
{
 2,0,0,0,0,0,
 3,0,0,0,M_TEXTURE_LENGTH,0,
 1,0,0,0,M_TEXTURE_LENGTH,M_TEXTURE_LENGTH,
 0,0,0,0,0,M_TEXTURE_LENGTH,
 2,0,0,0,0,0
};
struct M_polygon M_flat={M_QUAD_XY,         /* type */
                         0,0,0,             /* color */
                         M_LOG_TEXTURE_LENGTH,NULL,4,M_flat_v,NULL
                        };
int M_curved_1v[24]=                        /* static data for curved */
{
 1,0,0,0,M_TEXTURE_LENGTH,M_TEXTURE_LENGTH,
 0,0,0,0,0,M_TEXTURE_LENGTH,
 2,0,0,0,0,0,
 1,0,0,0,M_TEXTURE_LENGTH,M_TEXTURE_LENGTH
};
struct M_polygon M_curved_1={M_QUAD_MINUS_XY,
                             0,0,0,         /* color */
                             M_LOG_TEXTURE_LENGTH,NULL,3,M_curved_1v,NULL
                            };
int M_curved_2v[24]=                        /* curved, second triangle */
{
 2,0,0,0,0,0,
 3,0,0,0,M_TEXTURE_LENGTH,0,
 1,0,0,0,M_TEXTURE_LENGTH,M_TEXTURE_LENGTH,
 2,0,0,0,0,0
};
struct M_polygon M_curved_2={M_QUAD_XY,     /* type */
                             0,0,0,         /* color */
                             M_LOG_TEXTURE_LENGTH,NULL,3,M_curved_2v,NULL
                            };

int M_direction[8]={0,1,1,1,1,0,0,0};       /* oreder of triangs in M_CURVED */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Rendering one cell of a surface.            *
 * ---------                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MI_render_surface_cell(struct M_surface_cell *cell,
                            int *v1,int *c1,
                            int *v2,int *c2)
{
 int tmp_vert[T_LNG_VECTOR*4];              /* X Y Z */
 int tmp_colr[12];                          /* intensity or R G B */

 memcpy(tmp_vert,v1,T_LNG_VECTOR*2*sizeof(int));
 memcpy(tmp_vert+T_LNG_VECTOR*2,v2,T_LNG_VECTOR*2*sizeof(int));
 memcpy(tmp_colr,c1,6*sizeof(int));
 memcpy(tmp_colr+6,c2,6*sizeof(int));

 if(cell->m_type&M_PLANNAR)
 {
  M_flat.m_type|=cell->m_type;              /* creating a polygon */
  M_flat.m_red=cell->m_red_1;
  M_flat.m_green=cell->m_green_1;
  M_flat.m_blue=cell->m_blue_1;
  M_flat.m_texture=cell->m_texture;

  M_render_polygon(&M_flat,tmp_vert,tmp_colr,NULL);
 }
 else
 {
  M_curved_1.m_type|=cell->m_type;
  M_curved_2.m_type|=cell->m_type;
  M_curved_1.m_red=cell->m_red_1;
  M_curved_1.m_green=cell->m_green_1;
  M_curved_1.m_blue=cell->m_blue_1;
  M_curved_2.m_red=cell->m_red_2;
  M_curved_2.m_green=cell->m_green_2;
  M_curved_2.m_blue=cell->m_blue_2;
  M_curved_1.m_texture=M_curved_2.m_texture=cell->m_texture;

  if(M_direction[M_camera_gam>>5])          /* resolving triangles' order */
  {
   M_render_polygon(&M_curved_1,tmp_vert,tmp_colr,NULL);
   M_render_polygon(&M_curved_2,tmp_vert,tmp_colr,NULL);
  }
  else
  {
   M_render_polygon(&M_curved_2,tmp_vert,tmp_colr,NULL);
   M_render_polygon(&M_curved_1,tmp_vert,tmp_colr,NULL);
  }
 }
}

/**********************************************************\
 * Creating order array for a surface.                    *
 *                                                        *
 *      +-->              <--+       The order array      *
 *    2 |  +---+---+ | ---+  | 1     will be:             *
 *      V  | 6 | 7 |    8 |  V                            *
 *         +---+---+ | ---+          8,                   *
 *    - - - - - - - -+- - - - -      6,7,                 *
 *         | 3 | 4 | |  5 |          2,5,                 *
 *         +---+---+   ---+          0,1,3,4,             *
 *      ^  | 0 | 1 | |  2 |  ^                            *
 *    4 |  +---+---+   ---+  | 3                          *
 *      +-->         |    <--+                            *
 *                                                        *
 * Further, computing normals for all internal vertices   *
 * by averaging normals of neighboring polygons. Thus,    *
 * for vertex 4 the normal is that of an average of       *
 * polygons 0,1,2,3,4,5.                                  *
 *                            polygon:    vertices:       *
 *                 7                                      *
 *            6+---+---+8        1          2 1 4         *
 *             |\ 6|\  |         2          4 5 2         *
 *             | 5\|4 \|         3          3 4 1         *
 *            3+---4---+5        4          5 4 7         *
 *             |\ 3|\2 |         5          4 3 6         *
 *             |  \|1 \|         6          6 7 4         *
 *            0+---+---+2                                 *
 *                 1                                      *
 *                                                        *
\**********************************************************/

void M_init_surface_object(struct M_surface_object *object)
{
 int i,j;
 int vertex_sz=object->m_total_size+1;      /* one more than cells */
 int v0[T_LNG_VECTOR],v1[T_LNG_VECTOR],v2[T_LNG_VECTOR];
 int v3[T_LNG_VECTOR],v4[T_LNG_VECTOR],v5[T_LNG_VECTOR];
 int v6[T_LNG_VECTOR],v7[T_LNG_VECTOR],v8[T_LNG_VECTOR];
 int n1[T_LNG_VECTOR],n2[T_LNG_VECTOR],n3[T_LNG_VECTOR];
 int n4[T_LNG_VECTOR],n5[T_LNG_VECTOR],n6[T_LNG_VECTOR];

#if defined(_PAINTER_)
 int display_sz=object->m_display_size+1;
 int midl=(display_sz-1)/2;
 int *order=object->m_orders=(int*)malloc(display_sz*display_sz*3*sizeof(int));
 if(order==NULL) HW_error("(Engine) Not enough memory.\n");

 for(j=display_sz-2;j>midl;j--)
 {
  for(i=display_sz-2;i>midl;i--)            /* from right top */
  {
   *order++=i+j*display_sz;                 /* smallest peace first */
   *order++=i+(j+1)*display_sz;
   *order++=i+j*display_sz;
  }
 }

 for(j=display_sz-2;j>midl;j--)
 {
  for(i=0;i<=midl;i++)                      /* from left top */
  {
   *order++=i+j*display_sz;                 /* first rectangular */
   *order++=i+(j+1)*display_sz;
   *order++=i+j*display_sz;
  }
 }

 for(j=0;j<=midl;j++)
 {
  for(i=display_sz-2;i>midl;i--)            /* from right bottom */
  {
   *order++=i+j*display_sz;                 /* second rectangular */
   *order++=i+(j+1)*display_sz;
   *order++=i+j*display_sz;
  }
 }

 for(j=0;j<=midl;j++)
 {
  for(i=0;i<=midl;i++)                      /* from left bottom */
  {
   *order++=i+j*display_sz;                 /* bigger one last */
   *order++=i+(j+1)*display_sz;
   *order++=i+j*display_sz;
  }
 }
#endif

 v3[2]=v4[2]=v5[2]=v1[0]=v4[0]=v7[0]=0;
 v0[2]=v1[2]=v2[2]=v0[0]=v3[0]=v6[0]=object->m_cell_length;
 v6[2]=v7[2]=v8[2]=v2[0]=v5[0]=v8[0]=-object->m_cell_length;

 object->m_normals=(int*)malloc(sizeof(int)*vertex_sz*vertex_sz*T_LNG_VECTOR);
 if(object->m_normals==NULL)
  HW_error("(Engine) Not enough memory.");

 for(i=0;i<vertex_sz*vertex_sz;i++)
 {
  object->m_normals[i*T_LNG_VECTOR]=0;
  object->m_normals[i*T_LNG_VECTOR+1]=128;  /* by default pointing up */
  object->m_normals[i*T_LNG_VECTOR+2]=0;
 }

 for(j=1;j<vertex_sz-1;j++)
 {
  for(i=1;i<vertex_sz-1;i++)                /* flat shading */
  {
   v0[1]=object->m_vertices[((j-1)*vertex_sz+i-1)*M_LNG_SURFACE_VERTEX];
   v1[1]=object->m_vertices[((j-1)*vertex_sz+i)*M_LNG_SURFACE_VERTEX];
   v2[1]=object->m_vertices[((j-1)*vertex_sz+i+1)*M_LNG_SURFACE_VERTEX];

   v3[1]=object->m_vertices[(j*vertex_sz+i-1)*M_LNG_SURFACE_VERTEX];
   v4[1]=object->m_vertices[(j*vertex_sz+i)*M_LNG_SURFACE_VERTEX];
   v5[1]=object->m_vertices[(j*vertex_sz+i+1)*M_LNG_SURFACE_VERTEX];

   v6[1]=object->m_vertices[((j+1)*vertex_sz+i-1)*M_LNG_SURFACE_VERTEX];
   v7[1]=object->m_vertices[((j+1)*vertex_sz+i)*M_LNG_SURFACE_VERTEX];
   v8[1]=object->m_vertices[((j+1)*vertex_sz+i+1)*M_LNG_SURFACE_VERTEX];

   T_normal_plane(v2,v1,v4,n1);
   T_normal_plane(v4,v5,v2,n2);
   T_normal_plane(v3,v4,v1,n3);
   T_normal_plane(v5,v4,v7,n4);
   T_normal_plane(v4,v3,v6,n5);
   T_normal_plane(v6,v7,v4,n6);             /* normals for neighboring polygons */

   object->m_normals[(j*vertex_sz+i)*T_LNG_VECTOR]=(n1[0]+n2[0]+n3[0]+n4[0]+n5[0]+n6[0])/6;
   object->m_normals[(j*vertex_sz+i)*T_LNG_VECTOR+1]=(n1[1]+n2[1]+n3[1]+n4[1]+n5[1]+n6[1])/6;
   object->m_normals[(j*vertex_sz+i)*T_LNG_VECTOR+2]=(n1[2]+n2[2]+n3[2]+n4[2]+n5[2]+n6[2])/6;
  }
 }
}

/**********************************************************\
 * Computing shading per vertex and further per polygon   *
 * for dirty rendering options.                           *
\**********************************************************/

void M_light_surface_object(struct M_surface_object* object)
{
 int i,j,position[T_LNG_VECTOR],vertex_sz=object->m_total_size+1;
 struct M_surface_cell *cell;
 int *normal,*vertex,*v0,*v1,*v2,*v3;
 int init_x,init_z;

 L_init_material(object->m_material);

 vertex=object->m_vertices;
 normal=object->m_normals;
 init_x=0;                                  /* initial value for X */
 init_z=0;                                  /* initial value for Z */

 for(j=0,position[2]=init_z;j<vertex_sz;j++,position[2]+=object->m_cell_length)
 {
  for(i=0,position[0]=init_x;i<vertex_sz;i++,position[0]+=object->m_cell_length)
  {
   position[1]=vertex[0];                   /* copy Y */
   L_light_vertex(vertex+1,position,normal);

   vertex+=M_LNG_SURFACE_VERTEX;
   normal+=T_LNG_VECTOR;
  }
 }

 for(j=0;j<vertex_sz-1;j++)
 {
  for(i=0;i<vertex_sz-1;i++)
  {
   cell=&object->m_cells[(vertex_sz-1)*j+i];

   v0=object->m_vertices+M_LNG_SURFACE_VERTEX*(vertex_sz*j+i);
   v1=object->m_vertices+M_LNG_SURFACE_VERTEX*(vertex_sz*j+i+1);
   v2=object->m_vertices+M_LNG_SURFACE_VERTEX*(vertex_sz*(j+1)+i);
   v3=object->m_vertices+M_LNG_SURFACE_VERTEX*(vertex_sz*(j+1)+i+1);
   cell->m_red_1=(v1[1]+v0[1]+v2[1])/3;
   cell->m_green_1=(v1[2]+v0[2]+v2[2])/3;
   cell->m_blue_1=(v1[3]+v0[3]+v2[3])/3;

   cell->m_red_2=(v2[1]+v3[1]+v1[1])/3;
   cell->m_green_2=(v2[2]+v3[2]+v1[2])/3;
   cell->m_blue_2=(v2[3]+v3[3]+v1[3])/3;
  }
 }
}

/**********************************************************\
 * Rendering a part of a bigger surface.                  *
\**********************************************************/

void M_render_surface_object(struct M_surface_object *object,
                             int xcell,int zcell)
{
 int i,j,k;
 int tmp_colors[M_MAX_SURFACE_VERTICES*M_MAX_SURFACE_VERTICES*3];
 int tmp_vertices[M_MAX_SURFACE_VERTICES*M_MAX_SURFACE_VERTICES*3];
 struct M_surface_cell *tmp_cells[M_MAX_SURFACE_VERTICES*M_MAX_SURFACE_VERTICES];

 int *d_vert=tmp_vertices;                  /* destanation pointer */
 int *d_colr=tmp_colors;
 struct M_surface_cell **d_cell=tmp_cells;

 int init_x,init_z,cur_x,cur_z;
 int *s_vert,*order;                        /* source pointer */
 struct M_surface_cell *s_cell;

 int cell_size=object->m_display_size;
 int vrtx_size=cell_size+1;                 /* one more than edges */
 int midl=cell_size/2;
 int total_size=object->m_total_size;       /* number of nodes along a side */
 int cell_length=object->m_cell_length;

 int next_line_cell=total_size-vrtx_size;
 int next_line_vert=(next_line_cell+1)*M_LNG_SURFACE_VERTEX;

 if(vrtx_size>=M_MAX_SURFACE_VERTICES)
  HW_error("(Engine) Can't handle this many vertices in a surface.");

 if((zcell-midl<0)||(xcell-midl<0)||
    (zcell+midl>=total_size)||(xcell+midl>total_size)
   )
  HW_error("(Engine) Attempt to display .");

 s_vert=object->m_vertices+(((zcell-midl)*(total_size+1)+xcell-midl)*
        M_LNG_SURFACE_VERTEX);
 s_cell=object->m_cells+(zcell-midl)*total_size+xcell-midl;
 init_x=-(cell_length*midl+M_camera_x);     /* initial values */
 init_z=-(cell_length*midl+M_camera_z);

 for(j=0,cur_z=init_z;j<vrtx_size;j++,cur_z+=cell_length,
     s_vert+=next_line_vert,s_cell+=next_line_cell)
  for(i=0,cur_x=init_x;i<vrtx_size;i++,cur_x+=cell_length,s_cell++)
  {
   *d_vert++=cur_x;                         /* X */
   *d_vert++=(*s_vert++)+M_camera_y;        /* Y */
   *d_vert++=cur_z;                         /* Z */
    d_vert++;                               /* 4th element */

   *d_colr++=*s_vert++;                     /* Intensity or R */
   *d_colr++=*s_vert++;                     /* G */
   *d_colr++=*s_vert++;                     /* B */
   *d_cell++=s_cell;                        /* cell */
  }

 T_world_rotation(tmp_vertices,tmp_vertices,vrtx_size*vrtx_size);

 if((order=object->m_orders)!=NULL)         /* rendering in order */
 {
  for(i=0,cell_size=cell_size*cell_size;i<cell_size;i++,order+=3)
  {
   MI_render_surface_cell(tmp_cells[*order],
                          tmp_vertices+*(order+1)*T_LNG_VECTOR,
                          tmp_colors+*(order+1)*3,
                          tmp_vertices+*(order+2)*T_LNG_VECTOR,
                          tmp_colors+*(order+2)*3);
  }
 }
 else                                       /* just dumping everything */
 {
  for(k=0,j=0;j<vrtx_size-1;j++,k++)
  {
   for(i=0;i<vrtx_size-1;i++,k++)
   {
    MI_render_surface_cell(tmp_cells[k],
                           tmp_vertices+(k+vrtx_size)*T_LNG_VECTOR,
                           tmp_colors+(k+vrtx_size)*3,
                           tmp_vertices+k*T_LNG_VECTOR,
                           tmp_colors+k*3);
   }
  }
 }
}

/**********************************************************/
