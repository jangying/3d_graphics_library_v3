/** 3Dgpl3 ************************************************\
 * 3D polygonal model builder.                            *
 *                                                        *
 * Defines:                                               *
 *  S_init_construction      Set texture, material;       *
 *  S_sphere                 Build a sphere;              *
 *  S_cylinder               Build a cylinder.            *
 *                                                        *
 * Internals:                                             *
 *  SI_add_vertex            Add a vertex to the model;   *
 *  SI_add_polygon           Add a polygon to the model.  *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../shape/shape.h"                 /* 3D engine */
#include "../trans/trans.h"                 /* T_LNG... */
#include <stdlib.h>                         /* malloc */
#include <math.h>                           /* sin/cos */

/**********************************************************/

struct G_texture* S_texture;                /* additional info */
int S_log_texture_space_size;               /* to use when constructing */
struct L_material* S_material=NULL;         /* models */

/**********************************************************/

int* S_vertices;                            /* temp storage for vertices */
int S_no_vertices;                          /* entered so far */
struct M_polygon** S_polygons;              /* temp storage for polygons */
int S_no_polygons;                          /* entered so far */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Adding a new vertex.                        *
 * ---------                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int SI_add_vertex(int x,int y,int z)
{
 int i;

 for(i=0;i<S_no_vertices;i++)               /* comparing X Y Z only */
 {
  if((S_vertices[i*T_LNG_VECTOR]==x)&&
     (S_vertices[i*T_LNG_VECTOR+1]==y)&&
     (S_vertices[i*T_LNG_VECTOR+2]==z)) return(i);
 }
 if(i>=M_MAX_OBJECT_VERTICES)
  HW_error("(Shape) Not enough internal storage for vertices.\n");
                                            /* X Y Z to one Tx Ty to another */
 S_vertices[S_no_vertices*T_LNG_VECTOR]=x;
 S_vertices[S_no_vertices*T_LNG_VECTOR+1]=y;
 S_vertices[S_no_vertices*T_LNG_VECTOR+2]=z;

 return(S_no_vertices++);                   /* index of where inserted */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Adding a new polygon.                       *
 * ---------                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void SI_add_polygon(int n1,int t1x,int t1y,
                    int n2,int t2x,int t2y,
                    int n3,int t3x,int t3y,
                    int n4,int t4x,int t4y)
{
 int triangle=0;
 int *vertices;
 struct M_polygon* tmp=(struct M_polygon*)malloc(sizeof(struct M_polygon));
 if(tmp==NULL) HW_error("(Shape) Not enough memory.\n");

 tmp->m_type=M_CURVED;
 tmp->m_red=tmp->m_green=tmp->m_blue=0;
 tmp->m_log_texture_space_size=S_log_texture_space_size;
 tmp->m_texture=S_texture;
 tmp->m_normals=NULL;

 if(n1==n2) { triangle=1; n2=n3; t2x=t3x; t2y=t3y; n3=n4; t3x=t4x; t3y=t4y; }
 if(n3==n4) triangle=1;
 if(triangle)
 {
  vertices=(int*)malloc(4*M_LNG_POLYGON_VERTEX*sizeof(int));
  if(vertices==NULL) HW_error("(Shape) Not enough memory.\n");
  
  vertices[0]=n1; vertices[6]=n2; vertices[12]=n3; vertices[18]=n1;
  vertices[1]=vertices[7]=vertices[13]=vertices[19]=0;
  vertices[2]=vertices[8]=vertices[14]=vertices[20]=0;
  vertices[3]=vertices[9]=vertices[15]=vertices[21]=0;
  vertices[4]=t1x; vertices[10]=t2x; vertices[16]=t3x; vertices[22]=t1x;
  vertices[5]=t1y; vertices[11]=t2y; vertices[17]=t3y; vertices[23]=t1y;

  tmp->m_no_edges=3;
  tmp->m_vertices=vertices;
 }
 else
 {
  vertices=(int*)malloc(5*M_LNG_POLYGON_VERTEX*sizeof(int));
  if(vertices==NULL) HW_error("(Shape) Not enough memory.\n");
  
  vertices[0]=n1; vertices[6]=n2; vertices[12]=n3; vertices[18]=n4; vertices[24]=n1;
  vertices[1]=vertices[7]=vertices[13]=vertices[19]=vertices[25]=0;
  vertices[2]=vertices[8]=vertices[14]=vertices[20]=vertices[26]=0;
  vertices[3]=vertices[9]=vertices[15]=vertices[21]=vertices[27]=0;
  vertices[4]=t1x; vertices[10]=t2x; vertices[16]=t3x; vertices[22]=t4x; vertices[28]=t1x;
  vertices[5]=t1y; vertices[11]=t2y; vertices[17]=t3y; vertices[23]=t4y; vertices[29]=t1y;

  tmp->m_no_edges=4;
  tmp->m_vertices=vertices;
 }
 S_polygons[S_no_polygons++]=tmp;
}

/**********************************************************\
 * Produces a model of a sphere.                          *
 *                                                        *
 * SETS: S_texture, S_log_texture_space_size, S_material  *
 * -----                                                  *
\**********************************************************/

void S_init_construction(struct G_texture* txtr,
                         int txtr_space_size,
                         struct L_material* mater)
{
 S_texture=txtr;
 S_log_texture_space_size=txtr_space_size;
 S_material=mater;
}

/**********************************************************\
 * Produces a model of a sphere.                          *
\**********************************************************/

struct M_polygon_object* S_sphere(int radius,int divs)
{
 int i,j,nj,y1,y2;
 int rrad1,rrad2,x11,x12;
 int x21,x22,z11,z12,z21,z22,tx1,tx2,ty1,ty2;

 struct M_polygon_object* tmp=(struct M_polygon_object*)
                      malloc(sizeof(struct M_polygon_object));

 if(tmp==NULL) HW_error("(Shape) Not enough memory.\n"); 

 tmp->m_type=M_POLYGON_OBJECT;
 tmp->m_material=S_material;
 tmp->m_order=NULL;
 tmp->m_no_normals=0;
 tmp->m_normals=NULL;

 S_vertices=(int*)malloc(M_MAX_OBJECT_VERTICES*T_LNG_VECTOR*sizeof(int));
 if(S_vertices==NULL) HW_error("(Shape) Not enough memory.\n"); 
 S_no_vertices=0;

 S_polygons=(struct M_polygon**)malloc(M_MAX_OBJECT_POLYGONS*sizeof(struct M_polygon*));
 if(S_polygons==NULL) HW_error("(Shape) Not enough memory.\n"); 
 S_no_polygons=0;

 for(i=0;i<divs;i++)
 {
  ty1=(i*256)/divs;
  ty2=((i+1)*256)/divs;
  y1=radius*cos(((M_PI_)*i)/(divs));
  y2=radius*cos(((M_PI_)*(i+1))/(divs));
  rrad1=radius*sin(((M_PI_)*i)/(divs));
  rrad2=radius*sin(((M_PI_)*(i+1))/(divs));

  for(j=0;j<divs*2;j++)
  {
   tx1=(j*256)/(divs*2);
   tx2=((j+1)*256)/(divs*2);
   if(j<divs*2-1) nj=j+1; else nj=0;
   x11=rrad1*cos((2*M_PI_*j)/(divs*2));
   z11=rrad1*sin((2*M_PI_*j)/(divs*2));
   x12=rrad1*cos((2*M_PI_*nj)/(divs*2));
   z12=rrad1*sin((2*M_PI_*nj)/(divs*2));

   x21=rrad2*cos((2*M_PI_*j)/(divs*2));
   z21=rrad2*sin((2*M_PI_*j)/(divs*2));
   x22=rrad2*cos((2*M_PI_*nj)/(divs*2));
   z22=rrad2*sin((2*M_PI_*nj)/(divs*2));

   SI_add_polygon(SI_add_vertex(x11,y1,z11),tx1,ty1,
                  SI_add_vertex(x12,y1,z12),tx2,ty1,
                  SI_add_vertex(x22,y2,z22),tx2,ty2,
                  SI_add_vertex(x21,y2,z21),tx1,ty2);
  }
 }

 tmp->m_vertices=(int*)malloc(S_no_vertices*T_LNG_VECTOR*sizeof(int));
 if(tmp->m_vertices==NULL) HW_error("(Shape) Not enough memory.\n"); 
 for(i=0;i<S_no_vertices*T_LNG_VECTOR;i++)
  tmp->m_vertices[i]=S_vertices[i];
 tmp->m_no_vertices=S_no_vertices;

 tmp->m_polygons=(struct M_polygon**)malloc(S_no_polygons*sizeof(struct M_polygon*));
 if(tmp->m_polygons==NULL) HW_error("(Shape) Not enough memory.\n"); 
 for(i=0;i<S_no_polygons;i++)
  tmp->m_polygons[i]=S_polygons[i];
 tmp->m_no_polygons=S_no_polygons;

 free(S_vertices);
 free(S_polygons);
 return(tmp);
}

/**********************************************************\
 * Produces a model of a cylinder.                        *
\**********************************************************/

struct M_polygon_object* S_cylinder(int length,int radius,int divs)
{
 int i,j,nj;
 int x11,x12;
 int x21,x22,z11,z12,z21,z22,tx1,tx2,ty1,ty2;

 struct M_polygon_object* tmp=(struct M_polygon_object*)
                      malloc(sizeof(struct M_polygon_object));

 if(tmp==NULL) HW_error("(Shape) Not enough memory.\n"); 

 tmp->m_type=M_POLYGON_OBJECT;
 tmp->m_material=S_material;
 tmp->m_order=NULL;
 tmp->m_no_normals=0;
 tmp->m_normals=NULL;

 S_vertices=(int*)malloc(M_MAX_OBJECT_VERTICES*T_LNG_VECTOR*sizeof(int));
 if(S_vertices==NULL) HW_error("(Shape) Not enough memory.\n"); 
 S_no_vertices=0;

 S_polygons=(struct M_polygon**)malloc(M_MAX_OBJECT_POLYGONS*sizeof(struct M_polygon*));
 if(S_polygons==NULL) HW_error("(Shape) Not enough memory.\n"); 
 S_no_polygons=0;

 ty1=0;
 ty2=255;

 for(j=0;j<divs*2;j++)
 {
  tx1=(j*256)/(divs*2);
  tx2=((j+1)*256)/(divs*2);
  if(j<divs*2-1) nj=j+1; else nj=0;
  x11=radius*cos((2*M_PI_*j)/(divs*2));
  z11=radius*sin((2*M_PI_*j)/(divs*2));
  x12=radius*cos((2*M_PI_*nj)/(divs*2));
  z12=radius*sin((2*M_PI_*nj)/(divs*2));

  x21=radius*cos((2*M_PI_*j)/(divs*2));
  z21=radius*sin((2*M_PI_*j)/(divs*2));
  x22=radius*cos((2*M_PI_*nj)/(divs*2));
  z22=radius*sin((2*M_PI_*nj)/(divs*2));

  SI_add_polygon(SI_add_vertex(x11,length/2,z11),tx1,ty1,
                 SI_add_vertex(x12,length/2,z12),tx2,ty1,
                 SI_add_vertex(x22,-length/2,z22),tx2,ty2,
                 SI_add_vertex(x21,-length/2,z21),tx1,ty2);
 }

 tmp->m_vertices=(int*)malloc(S_no_vertices*T_LNG_VECTOR*sizeof(int));
 if(tmp->m_vertices==NULL) HW_error("(Shape) Not enough memory.\n"); 
 for(i=0;i<S_no_vertices*T_LNG_VECTOR;i++)
  tmp->m_vertices[i]=S_vertices[i];
 tmp->m_no_vertices=S_no_vertices;

 tmp->m_polygons=(struct M_polygon**)malloc(S_no_polygons*sizeof(struct M_polygon*));
 if(tmp->m_polygons==NULL) HW_error("(Shape) Not enough memory.\n"); 
 for(i=0;i<S_no_polygons;i++)
  tmp->m_polygons[i]=S_polygons[i];
 tmp->m_no_polygons=S_no_polygons;

 free(S_vertices);
 free(S_polygons);
 return(tmp);
}

/**********************************************************/
