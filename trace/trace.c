/** 3Dgpl3 ************************************************\
 * Simple 3D ray tracing engine.                          *
 *                                                        *
 * Defines:                                               *
 *  TR_init_rendering        Set rendering option;        *
 *  TR_init_camera           Set camera parameters;       *
 *                                                        *
 *  TR_sphere_init           Actualy, does nothing;       *
 *  TR_sphere_intersect      Closest of the two;          *
 *  TR_sphere_normal         From a point;                *
 *  TR_cylinder_init         Initializes type;            *
 *  TR_cylinder_intersect    Closest of the two;          *
 *  TR_cylinder_normal       From a point;                *
 *  TR_polygon_init          Compute the plane equation;  *
 *  TR_polygon_intersect     t of the intersection;       *
 *  TR_polygon_normal        Always constant;             *
 *                                                        *
 *  TR_init_world            Init all objects;            *
 *  TR_trace_world           Building an image.           *
 *                                                        *
 * Internals:                                             *
 *  TRI_make_ray_point       Construct from two points;   *
 *  TRI_make_ray_vector      From a point and a vector;   *
 *  TRI_on_ray               Point from ray coef;         *
 *  TRI_light                Local illumination;          *
 *  TRI_shadow_ray           If a lightsource is shadowed;*
 *  TRI_direct_ray           Computes one pixel's color.  *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../graphics/graphics.h"           /* G_point */
#include "../vector/vector.h"               /* linear algebra related */
#include "../trace/trace.h"                 /* self definition */
#include <math.h>                           /* sqrt */
#include <stdlib.h>                         /* malloc */

/**********************************************************/

int TR_rendering_type;                      /* rendering options */
float TR_viewer[V_LNG_VECTOR];              /* position of the viewer */
float TR_screen[V_LNG_VECTOR];              /* origine of the screen */
float TR_screen_u[V_LNG_VECTOR];            /* screen orientation vectors */
float TR_screen_v[V_LNG_VECTOR];

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Constructing a ray from two points.                   *
 *                                                       *
 * RETURNS: Constructed ray.                             *
 * --------                                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct TR_ray *TRI_make_ray_point(struct TR_ray *r,float *from,float *to)
{
 V_set(r->tr_start,from);
 V_difference(r->tr_codirected,to,from);
 return(r);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Constructing a ray from a point and a vector.         *
 *                                                       *
 * RETURNS: Constructed ray.                             *
 * --------                                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct TR_ray *TRI_make_ray_vector(struct TR_ray *r,float *from,float *vector)
{
 V_set(r->tr_start,from);
 V_set(r->tr_codirected,vector);
 return(r);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Returns point at distance t from the origine.         *
 *                                                       *
 * RETURNS: Constructed vertex.                          *
 * --------                                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

float *TRI_on_ray(float *point,struct TR_ray *r,float t)
{
 V_multiply(point,r->tr_codirected,t);
 V_sum(point,point,r->tr_start);
 return(point);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Computing illumination of a intersected surface point.*
 *                                                       *
 * RETURNS: An RGB triple.                               *
 * --------                                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

float *TRI_light(float *light,struct TR_point_light *l,
                              struct TR_matter *material,
                              float *normal,float *where,float *viewer)
{
 int i;
 float lightvector[V_LNG_VECTOR],viewvector[V_LNG_VECTOR],reflect[V_LNG_VECTOR];
 float diffuseratio,specularratio,specularfun,tmp[V_LNG_VECTOR];

 V_unit_vector(lightvector,where,l->tr_centre);
 V_unit_vector(viewvector,where,viewer);

 if((diffuseratio=V_scalar_product(normal,lightvector))>0)
 {
  if(TR_rendering_type&(TR_DIFFUSE|TR_SPECULAR))
  {
   light[0]+=l->tr_intensity[0]*material->tr_diffuse[0]*diffuseratio;
   light[1]+=l->tr_intensity[1]*material->tr_diffuse[1]*diffuseratio;
   light[2]+=l->tr_intensity[2]*material->tr_diffuse[2]*diffuseratio;
  }
                                            /* diffuse term */
  if(TR_rendering_type&TR_SPECULAR)
  {
   V_multiply(reflect,normal,2*diffuseratio);
   V_difference(reflect,reflect,lightvector);

   if((specularratio=V_scalar_product(reflect,viewvector))>0)
   {
    for(specularfun=1,i=0;i<material->tr_exponent;i++) specularfun*=specularratio;

    V_multiply(tmp,l->tr_intensity,material->tr_specular*specularfun);
    V_sum(light,light,tmp);
   }                                        /* specular term */
  }
 }
 return(light);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Casting a ray towards a lightsource to find out if    *
 * it is hidden by other objects or not.                 *
 *                                                       *
 * RETURNS: 1 light source visible; 0 otherwise.         *
 * --------                                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int TRI_shadow_ray(struct TR_world *w,
                   struct TR_point_light *l,
                   float *point,
                   int cur_obj)
{
 float t=0.0;
 int i;
 struct TR_ray r;

 TRI_make_ray_point(&r,point,l->tr_centre);
 for(i=0;i<w->tr_no_objects;i++)            /* finding intersection */
 {
  if(i!=cur_obj)
  {
   switch(w->tr_objects[i]->tr_type)
   {
    case TR_SPHERE:  
     t=TR_sphere_intersect(&r,(struct TR_sphere*)w->tr_objects[i]);
     break;
    case TR_CYLINDER:
     t=TR_cylinder_intersect(&r,(struct TR_cylinder*)w->tr_objects[i]);
     break;
    case TR_POLYGON: 
     t=TR_polygon_intersect(&r,(struct TR_polygon*)w->tr_objects[i]);
     break;
   }
   if((t>0)&&(t<=1)) return(1);             /* first intersection is enough */
  }
 }

 return(0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Casting a ray into the world, recursing to compute    *
 * environmental reflections.                            *
 *                                                       *
 * RETURNS: Illumination for the pixel.                  *
 * --------                                              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

float *TRI_direct_ray(float *light,struct TR_world *w,
                                   struct TR_ray *r,
                                   int cur_obj,
                                   int depth)
{
 int i,min=0,no_inter=0;
 float objt[TR_MAX_SPHERES],t=0.0;
 int obj[TR_MAX_SPHERES];
 struct TR_ray rr;
 float where[V_LNG_VECTOR];                 /* current intersection */
 float normal[V_LNG_VECTOR];                /* of the current intersection */
 float viewer[V_LNG_VECTOR],reflect[V_LNG_VECTOR],rlight[V_LNG_VECTOR];

 if(depth!=0)
 {
  for(i=0;i<w->tr_no_objects;i++)           /* finding intersection */
  {
   if(i!=cur_obj)                           /* with itself, no sense */
   {
    switch(w->tr_objects[i]->tr_type)
    {
     case TR_SPHERE:  
      t=TR_sphere_intersect(r,(struct TR_sphere*)w->tr_objects[i]);
      break;
     case TR_CYLINDER:  
      t=TR_cylinder_intersect(r,(struct TR_cylinder*)w->tr_objects[i]);
      break;
     case TR_POLYGON: 
      t=TR_polygon_intersect(r,(struct TR_polygon*)w->tr_objects[i]);
      break;
    }
    if(t>0)                                 /* not behind the ray */
    {
     objt[no_inter]=t; obj[no_inter++]=i;   /* a valid intersection */
    }
   }
  }

  if(no_inter!=0)                           /* if some objects intersected */
  {
   for(i=1;i<no_inter;i++)
    if(objt[min]>objt[i]) min=i;            /* finding closest intersection */

   light[0]+=w->tr_objects[obj[min]]->tr_material.tr_ambient[0]*w->tr_ambient[0];
   light[1]+=w->tr_objects[obj[min]]->tr_material.tr_ambient[1]*w->tr_ambient[1];
   light[2]+=w->tr_objects[obj[min]]->tr_material.tr_ambient[2]*w->tr_ambient[2];

   TRI_on_ray(where,r,objt[min]);           /* intersection's coordinate */

   switch(w->tr_objects[obj[min]]->tr_type)
   {
    case TR_SPHERE:
     TR_sphere_normal(normal,where,
                      (struct TR_sphere*)w->tr_objects[obj[min]]);
     break;
    case TR_CYLINDER:
     TR_cylinder_normal(normal,where,
                        (struct TR_cylinder*)w->tr_objects[obj[min]],
                        r,objt[min]);
     break;
    case TR_POLYGON: 
     TR_polygon_normal(normal,where,
                       (struct TR_polygon*)w->tr_objects[obj[min]]);
     break;
   }

   for(i=0;i<w->tr_no_point_lights;i++)     /* illumination from each light */
   {
    if((!TRI_shadow_ray(w,w->tr_point_lights[i],where,obj[min]))||
       (!(TR_rendering_type&TR_SHADOW)))
     TRI_light(light,w->tr_point_lights[i],
               &w->tr_objects[obj[min]]->tr_material,
               normal,where,TR_viewer);
   }

   if(TR_rendering_type&TR_REFLECT)
   {
    float tmp[V_LNG_VECTOR];

    V_unit_vector(viewer,where,TR_viewer);
    V_multiply(reflect,normal,V_scalar_product(normal,viewer)*2);
    V_difference(reflect,reflect,viewer);
    TRI_make_ray_vector(&rr,where,reflect); /* prepare recursive ray */

    TRI_direct_ray(V_zero(rlight),w,&rr,obj[min],depth-1);

    V_multiply(tmp,rlight,w->tr_objects[obj[min]]->tr_material.tr_reflect);
    V_sum(light,light,tmp);
   }
  }
 }
 return(light);
}

/**********************************************************\
 * Setting the rendering type.                            *
 *                                                        *
 * SETS: TR_rendering_type                                *
 * -----                                                  *
\**********************************************************/

void TR_init_rendering(int type)
{
 TR_rendering_type=type;
}

/**********************************************************\
 * Setting the camera parameter, where TR_viewer stores   *
 * position of the viewer's eye, TR_screen origine of the *
 * projection plane, TR_screen_u and TR_screen_v          *
 * orientation of the projection plane in the world       *
 * space.                                                 *
 *                                                        *
 * SETS: TR_viewer, TR_screen...                          *
 * -----                                                  *
\**********************************************************/

void TR_init_camera(float viewer_x,float viewer_y,float viewer_z,
                    float screen_x,float screen_y,float screen_z,
                    float screen_ux,float screen_uy,float screen_uz,
                    float screen_vx,float screen_vy,float screen_vz)
{
 V_vector_coordinates(TR_viewer,viewer_x,viewer_y,viewer_z);
 V_vector_coordinates(TR_screen,screen_x,screen_y,screen_z);
 V_vector_coordinates(TR_screen_u,screen_ux,screen_uy,screen_uz);
 V_vector_coordinates(TR_screen_v,screen_vx,screen_vy,screen_vz);
}

/**********************************************************\
 * Does nothing, for symmetry's sake.                     *
\**********************************************************/

void TR_sphere_init(struct TR_sphere *s)
{
 s->tr_type=TR_SPHERE;
}

/**********************************************************\
 * Finding intersection of a ray with a sphere.           *
 *                                                        *
 * RETURNS: Distance from the origine of the ray.         *
 * --------                                               *
\**********************************************************/

float TR_sphere_intersect(struct TR_ray *r,struct TR_sphere *s)
{
 float a,b,c,det;
 float d[V_LNG_VECTOR],t1,t2;

 a=V_scalar_product(r->tr_codirected,r->tr_codirected);
 b=2*V_scalar_product(r->tr_codirected,V_difference(d,r->tr_start,
                      s->tr_centre));
 c=V_scalar_product(d,d)-s->tr_radius*s->tr_radius;
 det=b*b-4*a*c;

 if(det<0) return(-1);                      /* no intersection */
 if(det==0) return(-b/(2*a));               /* one intersection */
 t1=(-b+sqrt(det))/(2*a);
 t2=(-b-sqrt(det))/(2*a);

 if(t1<t2) return(t1); else return(t2);     /* closest intersection */
}

/**********************************************************\
 * Computes sphere's normal for a point on a shpere.      *
 *                                                        *
 * RETURNS: The normal vector.                            *
 * --------                                               *
\**********************************************************/

float *TR_sphere_normal(float *normal,float *where,
                        struct TR_sphere *s)
{
 V_unit_vector(normal,s->tr_centre,where);  /* from the centre */
 return(normal);
}

/**********************************************************\
 * Does nothing, for symmetry's sake.                     *
\**********************************************************/

void TR_cylinder_init(struct TR_cylinder *cy)
{
 V_difference(cy->tr_direction,cy->tr_direction,cy->tr_origine);
 cy->tr_type=TR_CYLINDER;
}

/**********************************************************\
 * Finding intersection of a ray with a sphere.           *
 *                                                        *
 * RETURNS: Distance from the origine of the ray.         *
 * --------                                               *
\**********************************************************/

float TR_cylinder_intersect(struct TR_ray *r,struct TR_cylinder *cy)
{
 float a,b,c,det;
 float tmp[V_LNG_VECTOR],K[V_LNG_VECTOR],L[V_LNG_VECTOR],t1,t2;
 float i,j,cp;

 i=V_scalar_product(r->tr_codirected,cy->tr_direction)/
   V_scalar_product(cy->tr_direction,cy->tr_direction);

 j=(V_scalar_product(r->tr_start,cy->tr_direction)-
    V_scalar_product(cy->tr_origine,cy->tr_direction))/
   V_scalar_product(cy->tr_direction,cy->tr_direction);

 V_difference(K,r->tr_codirected,V_multiply(tmp,cy->tr_direction,i));

 V_difference(L,r->tr_start,cy->tr_origine);
 V_difference(L,L,V_multiply(tmp,cy->tr_direction,j));

 a=V_scalar_product(K,K);
 b=2*V_scalar_product(K,L);
 c=V_scalar_product(L,L)-cy->tr_radius*cy->tr_radius;
   
 det=b*b-4*a*c;
 
 if(det<0) return(-1);                      /* no intersection */
 if(det==0) 
 { 
  t1=-b/(2*a);

  cp=t1*i+j;                                /* parameter along cylinder */
  if((cp>=0)&&(cp<=1)) return(-b/(2*a));    /* one intersection */
  else return(-1);
 }

 t1=(-b+sqrt(det))/(2*a);
 t2=(-b-sqrt(det))/(2*a);

 if(t1>t2) t1=t2;                           /* closer of the two */

 cp=t1*i+j;                                 /* parameter along cylinder */
 if((cp>=0)&&(cp<=1)) return(t1);           /* one intersection */
 else return(-1);
}

/**********************************************************\
 * Computes cylinder's normal for a point on a cylinder.  *
 *                                                        *
 * RETURNS: The normal vector.                            *
 * --------                                               *
\**********************************************************/

float *TR_cylinder_normal(float *normal,float *where,
                        struct TR_cylinder *cy,
                        struct TR_ray *r,
                        float t)
{
 float Or[V_LNG_VECTOR],i,j;

 i=V_scalar_product(r->tr_codirected,cy->tr_direction)/
   V_scalar_product(cy->tr_direction,cy->tr_direction);

 j=(V_scalar_product(r->tr_start,cy->tr_direction)-
    V_scalar_product(cy->tr_origine,cy->tr_direction))/
   V_scalar_product(cy->tr_direction,cy->tr_direction);

 V_sum(Or,cy->tr_origine,V_multiply(Or,cy->tr_direction,t*i+j));

 V_unit_vector(normal,Or,where);
 return(normal);
}

/**********************************************************\
 * Computes plane equation and the equations delimiting   *
 * the edges.                                             *
\**********************************************************/

void TR_polygon_init(struct TR_polygon *p)
{
 int i;
 float a[V_LNG_VECTOR],b[V_LNG_VECTOR];

 p->tr_type=TR_POLYGON;
 p->tr_edges=(float*)malloc((p->tr_no_vertices)*4*sizeof(float));

 V_vector_points(a,&p->tr_vertices[V_LNG_VECTOR*2],
                 &p->tr_vertices[V_LNG_VECTOR]);
 V_vector_points(b,&p->tr_vertices[V_LNG_VECTOR],&p->tr_vertices[0]);
 V_vector_product(p->tr_normal,a,b);        /* normal to the plane */

 V_zero(a);                                 /* making it unit length */
 V_unit_vector(p->tr_normal,a,p->tr_normal);

 for(i=0;i<p->tr_no_vertices;i++)           /* finding equations for edges */
 {
  V_vector_points(a,&p->tr_vertices[i*V_LNG_VECTOR],
                  &p->tr_vertices[(i+1)*V_LNG_VECTOR]);
  V_vector_product(b,p->tr_normal,a);
  V_plane(&p->tr_edges[i*4],b,&p->tr_vertices[i*V_LNG_VECTOR]);
 }
}

/**********************************************************\
 * Finding intersection of a ray with a polygon.           *
 *                                                        *
 * RETURNS: Distance from the origine of the ray.         *
 * --------                                               *
\**********************************************************/

float TR_polygon_intersect(struct TR_ray *r,struct TR_polygon *p)
{
 float a[V_LNG_VECTOR],t,s1,s2;
 int i;

 V_difference(a,p->tr_vertices,r->tr_start);
 s1=V_scalar_product(a,p->tr_normal);
 s2=V_scalar_product(r->tr_codirected,p->tr_normal);

 if(s2==0) return(-1); else t=s1/s2;
 if(t<0) return(-1);

 TRI_on_ray(a,r,t);

 for(i=0;i<p->tr_no_vertices;i++)
  if(V_vertex_on_plane(&p->tr_edges[i*4],a)>0) return(-1);

 return(t);
}

/**********************************************************\
 * Returns polygon's normal.                              *
 *                                                        *
 * RETURNS: The Normal vector.                            *
 * --------                                               *
\**********************************************************/

float *TR_polygon_normal(float *normal,float *where,
                         struct TR_polygon *p)
{
 V_set(normal,p->tr_normal);
 return(normal);
}

/**********************************************************\
 * Initialisez all entities in the world.                 *
\**********************************************************/

void TR_init_world(struct TR_world *w)
{
 int i;

 for(i=0;i<w->tr_no_objects;i++)
 {
  switch(w->tr_objects[i]->tr_type)
  {
   case TR_SPHERE:   TR_sphere_init((struct TR_sphere*)w->tr_objects[i]);
                     break;
   case TR_CYLINDER: TR_cylinder_init((struct TR_cylinder*)w->tr_objects[i]);
                     break;
   case TR_POLYGON:  TR_polygon_init((struct TR_polygon*)w->tr_objects[i]);
                     break;
  }
 }
}

/**********************************************************\
 * Ray tracing a scene for all pixels.                    *
\**********************************************************/

void TR_trace_world(struct TR_world *w,int depth)
{
 int i,j;
 float *c,x,y;
 float light[V_LNG_VECTOR],point[V_LNG_VECTOR],tmp[V_LNG_VECTOR];
 int coord[V_LNG_VECTOR]={0,0,0};
 struct TR_ray r;

 for(i=0;i<HW_screen_x_size;i++)
 {
  for(j=0;j<HW_screen_y_size;j++)           /* for each pixel on screen */
  {
   coord[0]=i;
   coord[1]=j;                              /* screen coordinates */

   x=i-HW_screen_x_size/2;
   y=j-HW_screen_y_size/2;                  /* plane coordinates */

   V_multiply(point,TR_screen_u,x);
   V_multiply(tmp,TR_screen_v,y);
   V_sum(point,point,tmp);
   V_sum(point,point,TR_screen); 

   TRI_make_ray_point(&r,TR_viewer,point);
   c=TRI_direct_ray(V_zero(light),w,&r,-1,depth);
   G_point(coord,(int)(c[0]*256),(int)(c[1]*256),(int)(c[2]*256));
  }
 }
}

/**********************************************************/
