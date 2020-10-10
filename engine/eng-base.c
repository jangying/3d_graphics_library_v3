/** 3Dgpl3 ************************************************\
 * Functions for the base of polygonal 3D engine.         *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_               Depth array;                 *
 *  _PAINTER_                Back to front order.         *
 *                                                        *
 * Defines:                                               *
 *  M_init_rendering         Set type of rendering;       *
 *  M_init_camera            Current camera geometry;     *
 *  M_sort_elements          Polygons in _PAINTER_;       *
 *                                                        *
 *  M_light_polygon          Shading using normals;       *
 *  M_render_polygon         Rendering in perspective;    *
 *                                                        *
 * Internals:                                             *
 *  MI_construct_tuples      Gets X Y Z R G B Tx Ty;      *
 *  MI_tmapping_vectors      Computes U V for tmapping.   *
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
#include "../light/light.h"                 /* G_light_vertex */
#include <stdlib.h>                         /* NULL */
#include <limits.h>                         /* INT_MIN and INT_MAX */
#include <string.h>                         /* memcpy */

int M_force_linear_tmapping=0;              /* when the polygons are small */
int M_rendering_type;                       /* types that can be rendered */
unsigned char M_camera_gam;                 /* needed for surfaces */
int M_camera_x,M_camera_y,M_camera_z;
int M_camera_log_focus;                     /* camera parameters */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Constructing polygon list.                  *
 * ---------                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MI_construct_tuples(int *from,int *to,
                         int *vertices,int *colors,int *textures,
                         int dimension,int length,int *min,int *max)
{
 int i,index;

 min[0]=min[1]=min[2]=INT_MAX;              /* polygon's extend */
 max[0]=max[1]=max[2]=INT_MIN; 

 for(i=0;i<length;i++)
 {
  memcpy(to,vertices+((index=(*from++))*T_LNG_VECTOR),sizeof(int)*3);

  if(*to>max[0]) max[0]=*to;                /* determining polygon's extend */
  if(*to<min[0]) min[0]=*to;
  to++;
  if(*to>max[1]) max[1]=*to;
  if(*to<min[1]) min[1]=*to;
  to++;
  if(*to>max[2]) max[2]=*to;
  if(*to<min[2]) min[2]=*to;                /* by searching max/min */
  to++;

  if(dimension>=M_LNG_SHADED)               /* color present ? */
  {
   if(colors==NULL)                         /* stored consecutively */
   {
    *to++=(*(from++))<<M_P;                 /* store R G B */
    *to++=(*(from++))<<M_P;
    *to++=(*(from++))<<M_P;
   }
   else
   {
    memcpy(to,colors+(index*3),sizeof(int)*3);
    *to++<<=M_P;
    *to++<<=M_P;
    *to++<<=M_P;
    from+=3;                                /* skip empty space for R G B */
   }
   if(dimension==M_LNG_TEXTURED)            /* texture present? */
   {
    if(textures==NULL)                      /* stored consecutively */
    {
     *to++=(*(from++))<<M_P;
     *to++=(*(from++))<<M_P;                /* store Tx,Ty */
    }
    else
    {
     *to++=textures[index*2]<<M_P;
     *to++=textures[index*2+1]<<M_P;
     from+=2;
    }
   }
   else from+=2;                            /* Tx Ty */
  }
  else from+=5;                             /* R G B Tx Ty */
 }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Finding u,v texture orientation vectors.    *
 * ---------                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void MI_tmapping_vectors(int type,int *p1,int *p2,int *p3,
                         int log_texture_space_size,
                         int *u,int *v)
{
 int a[T_MAX_MATRIX_SIZE][T_MAX_MATRIX_SIZE];
 int b[T_MAX_MATRIX_SIZE][T_MAX_MATRIX_SIZE];
 int x[T_MAX_MATRIX_SIZE][T_MAX_MATRIX_SIZE];

 if(type)                                   /* regular shape */
 {
  if(type&M_QUAD_XY)
  {
   T_vector(p1,p2,u,3);                     /* X Y Z */
   T_vector(p2,p3,v,3);                     /* direct order */
  }
  else
  {
   T_vector(p2,p1,u,3);                     /* X Y Z */
   T_vector(p3,p2,v,3);                     /* reversed order */
  }
 }
 else                                       /* non regular shape */
 {
  T_vector(p1+M_IDX_TEXTURE,p2+M_IDX_TEXTURE,a[0],2);
  T_vector(p2+M_IDX_TEXTURE,p3+M_IDX_TEXTURE,a[1],2);
  T_vector(p1,p2,b[0],3);                   /* X Y Z */
  T_vector(p2,p3,b[1],3);                   /* setting system of equations */

  T_linear_solve(a,b,x,2,3,M_P,log_texture_space_size);

  memcpy(u,x[0],sizeof(int)*3);
  memcpy(v,x[1],sizeof(int)*3);             /* X Y Z */
 }
}

/**********************************************************\
 * Set rendering options.                                 *
 *                                                        *
 * SETS: M_rendering_type                                 *
 * -----                                                  *
\**********************************************************/

void M_init_rendering(int type)
{
 M_rendering_type=type;                     /* allowed rendering methods */
}

/**********************************************************\
 * Set the camera parameters, orientation and position    *
 * of the projection plane and perspective focus.         *
 *                                                        *
 * SETS: M_camera_...                                     *
 * -----                                                  *
\**********************************************************/

void M_init_camera(unsigned char alp,unsigned char bet,unsigned char gam,
                   int x,int y,int z,int log_focus)
{
 T_set_world_rotation((unsigned char)(256-alp),(unsigned char)(256-bet),
                      (unsigned char)(256-gam)
                     );
 M_camera_gam=(unsigned char)256-gam;
 M_camera_x=-x;
 M_camera_y=-y;
 M_camera_z=-z;
 M_camera_log_focus=log_focus;              /* other parameter of the camera */
}

/**********************************************************\
 * Sorting at the same time an array of vertices and an   *
 * array of indices.                                      *
\**********************************************************/

#if defined(_PAINTER_)
void M_sort_elements(int *vertices,int dimension,int *indices,int number)
{
 int i,j,tmp;

 for(i=number-1;i>0;i--)                    /* buble sorting indexes */
 {
  for(j=0;j<i;j++)
  {
   if(vertices[indices[j]*dimension]<vertices[indices[(j+1)]*dimension])
   {                                        /* render the ones further away */
    tmp=indices[j]; indices[j]=indices[j+1]; indices[j+1]=tmp;
   }                                        /* earlier */
  }
 }
}
#endif

/**********************************************************\
 * Lighting a polygon.                                    *
\**********************************************************/

void M_light_polygon(struct M_polygon *poly,int *vertices,int *normals)
{
 int i,*vertex,*normal_idxs;

 poly->m_red=0;
 poly->m_green=0;
 poly->m_blue=0;                            /* init flat intensity */

 vertex=poly->m_vertices;
 normal_idxs=poly->m_normals;
 for(i=0;i<=poly->m_no_edges;i++,vertex+=M_LNG_POLYGON_VERTEX,normal_idxs++)
 {
  L_light_vertex(vertex+1,vertices+vertex[0]*T_LNG_VECTOR,
                 normals+(*normal_idxs*T_LNG_VECTOR)
                );
  poly->m_red+=vertex[1];
  poly->m_green+=vertex[2];
  poly->m_blue+=vertex[3];                  /* polygon's illumination */
 }

 poly->m_red/=(poly->m_no_edges+1);
 poly->m_green/=(poly->m_no_edges+1);       /* computing intensities for */
 poly->m_blue/=(poly->m_no_edges+1);        /* dirty rendering options */
}

/**********************************************************\
 * Rendering a generic polygon in perspective.            *
 * Firstly vertex tuples are created, they are            *
 * of length M_LNG_WHATEVER, they are being pyramide      *
 * clipped and Z clipped, hence tuples are transformed    *
 * into perspective acquiring length of G_LNG_WHATEVER,   *
 * (Z might be discarded for _PAINTER_, that's why),      *
 * culling is done, and then method dependent             *
 * (FLAT, SHADED) rendering is done. Specifically for     *
 * perspective a function to get mapping vectors will     *
 * be invoked.                                            *
\**********************************************************/

void M_render_polygon(struct M_polygon *poly,
                      int *vertices,int *colors,int *textures)
{
 int original1[M_MAX_POLYGON_VERTICES*C_MAX_DIMENSIONS];
 int original2[M_MAX_POLYGON_VERTICES*C_MAX_DIMENSIONS];
 int perspective[M_MAX_POLYGON_VERTICES*C_MAX_DIMENSIONS];
 int vector1[T_LNG_VECTOR],vector2[T_LNG_VECTOR];
 int *clipped,number,cnd;
 int i,min[3],max[3];                       /* extends X Y Z */

 switch(M_rendering_type)
 {
  case M_POINT:
  {
   number=poly->m_no_edges;
   MI_construct_tuples((int*)poly->m_vertices,original1,
                       vertices,colors,textures,
                       M_LNG_FLAT,number+1,min,max);
   if((cnd=C_volume_clipping(min,max))!=0)  /* using extends */
   {
    if(cnd==-1)
    {
     number=C_polygon_z_clipping(original1,clipped=original2,
                                 M_LNG_FLAT,number);
    }
    else clipped=original1;                 /* source is of M_LNG_FLAT */
    T_perspective(clipped,perspective,M_LNG_FLAT,number+1,M_camera_log_focus);
                                            /* result is of G_LNG_FLAT */
    if(T_normal_z_negative(perspective,perspective+G_LNG_FLAT,
                                       perspective+G_LNG_FLAT*2))
    {
     for(i=0;i<number;i++)
     { 
      G_point(perspective+i*G_LNG_FLAT,poly->m_red,poly->m_green,poly->m_blue);
     }
    }
   }
  } break;

  case M_WIRE:                              /* X Y Z, a wireframe */
  {
   number=poly->m_no_edges;
   MI_construct_tuples((int*)poly->m_vertices,original1,
                       vertices,colors,textures,
                       M_LNG_FLAT,number+1,min,max);
   if((cnd=C_volume_clipping(min,max))!=0)  /* using extends */
   {
    if(cnd==-1)
    {
     number=C_polygon_z_clipping(original1,clipped=original2,
                                 M_LNG_FLAT,number);
    }
    else clipped=original1;                 /* source is of M_LNG_FLAT */
    T_perspective(clipped,perspective,M_LNG_FLAT,number+1,M_camera_log_focus);
                                            /* result is of G_LNG_FLAT */
    if(T_normal_z_negative(perspective,perspective+G_LNG_FLAT,
                                       perspective+G_LNG_FLAT*2))
    {
     for(i=0;i<number;i++)
     {
      G_line(perspective+i*G_LNG_FLAT,perspective+(i+1)*G_LNG_FLAT,
             poly->m_red,poly->m_green,poly->m_blue);
     }
    }
   }
  } break;

  case M_FLAT:                              /* X Y Z, flat shaded polygon */
  {
   number=poly->m_no_edges;
   MI_construct_tuples((int*)poly->m_vertices,original1,
                       vertices,colors,textures,
                       M_LNG_FLAT,number+1,min,max);
   if((cnd=C_volume_clipping(min,max))!=0)  /* using extends */
   {
    if(cnd==-1)
    {
     number=C_polygon_z_clipping(original1,clipped=original2,
                                 M_LNG_FLAT,number);
    }
    else clipped=original1;                 /* source is of M_LNG_FLAT */
    T_perspective(clipped,perspective,M_LNG_FLAT,number+1,M_camera_log_focus);
                                            /* result is of G_LNG_FLAT */
    if(T_normal_z_negative(perspective,perspective+G_LNG_FLAT,
                                       perspective+G_LNG_FLAT*2))
    {
     G_flat_polygon(perspective,number,
                    poly->m_red,poly->m_green,poly->m_blue);
    }
   }
  } break;

  case M_TEXTURED:                          /* X Y Z [I] or [R G B] Tx Ty */
  {                                         /* texture mapped polygon */
   if(poly->m_texture!=NULL)
   {
    number=poly->m_no_edges;
    MI_construct_tuples((int*)poly->m_vertices,original1,
                        vertices,colors,textures,
                        M_LNG_TEXTURED,number+1,min,max);
    if((cnd=C_volume_clipping(min,max))!=0) /* using extends */
    {
     if(cnd==-1) number=C_polygon_z_clipping(original1,clipped=original2,
                                             M_LNG_TEXTURED,number);
     else clipped=original1;                /* source is of M_LNG_TEXTURED */

     T_perspective(clipped,perspective,M_LNG_TEXTURED,number+1,
                   M_camera_log_focus);
                                            /* result is of G_LNG_TEXTURED */
     if(T_normal_z_negative(perspective,perspective+G_LNG_TEXTURED,
                                        perspective+G_LNG_TEXTURED*2))
     {
      if((!M_force_linear_tmapping)&&
         (clipped[2]<M_Z_MAPPING_SWITCH))   /* non-linear one */
      {                                     /* original is of M_LNG_TEXTURED */
       MI_tmapping_vectors(poly->m_type&M_QUAD,
                           original1,
                           original1+M_LNG_TEXTURED,
                           original1+M_LNG_TEXTURED*2,
                           poly->m_log_texture_space_size,
                           vector1,vector2);
       G_prp_textured_polygon(perspective,number,clipped,
                              vector1,vector2,
                              poly->m_log_texture_space_size,
                              poly->m_texture);
      }
      else                                  /* linear mapping */
      {
       G_lin_textured_polygon(perspective,number,poly->m_texture);
      }
     }
    }
    break;
   }                                        /* if no texture do shaded */
  }

  case M_SHADED:                            /* X Y Z [I] or [R G B] */
  {                                         /* Gourand shaded polygons */
   number=poly->m_no_edges;
   MI_construct_tuples((int*)poly->m_vertices,original1,
                       vertices,colors,textures,
                       M_LNG_SHADED,number+1,min,max);
   if((cnd=C_volume_clipping(min,max))!=0)  /* using extends */
   {
    if(cnd==-1)
    {
     number=C_polygon_z_clipping(original1,clipped=original2,
                                 M_LNG_SHADED,number);
    }
    else clipped=original1;                 /* source is of M_LNG_SHADED */

    T_perspective(clipped,perspective,M_LNG_SHADED,number+1,M_camera_log_focus);
                                            /* result is of G_LNG_SHADED */
    if(T_normal_z_negative(perspective,perspective+G_LNG_SHADED,
                                       perspective+G_LNG_SHADED*2))
    {
     G_shaded_polygon(perspective,number);
    }
   }
  } break;
 }
}

/**********************************************************/
