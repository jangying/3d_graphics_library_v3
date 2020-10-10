/** 3Dgpl3 ************************************************\
 * 2D graphics and 2D clipping extentions for shaded      *
 * and textured polygons (has to be linked with regular   *
 * routines).                                             *
 *                                                        *
 * Ifdefs:                                                *
 *  _Z_BUFFER_               Depth array;                 *
 *  _PAINTER_                Back front order.            *
 *                                                        *
 * Defines:                                               *
 *  G_flat_polygon           Regular polygon;             *
 *  G_shaded_polygon         Shaded polygon;              *
 *  G_lin_textured_polygon   Textured polygon (approx);   *
 *  G_prp_textured_polygon   Textured polygon (true).     *
 *                                                        *
 * Internals:                                             *
 *  GI_scan                  Scanning an edge;            *
 *  GI_boarder_array_init    Init left/right boundaries.  *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* hardware specific stuff */
#include "../clipper/clipper.h"             /* 2D clipping */
#include "../graphics/graphics.h"           /* 2D macros */
#include "../engine/engine.h"               /* M_camera_log_focus */
#include <limits.h>                         /* INT_MAX and INT_MIN */

/**********************************************************/

#define G_LINEAR 32                         /* interpolate for */

int G_miny,G_maxy;                          /* vertical boundaries */

int G_x_start[HW_MAX_SCREEN_SIZE];          /* polygon's */
int G_x_end[HW_MAX_SCREEN_SIZE];            /* horizontal boundaries */
HW_fixed G_0_start[HW_MAX_SCREEN_SIZE];     /* [32-G_P].[G_P] values */
HW_fixed G_0_end[HW_MAX_SCREEN_SIZE];       /* the thingie is to work faster */
HW_fixed G_1_start[HW_MAX_SCREEN_SIZE];     /* then multidimensional array */
HW_fixed G_1_end[HW_MAX_SCREEN_SIZE];       /* hope so, */
HW_fixed G_2_start[HW_MAX_SCREEN_SIZE];
HW_fixed G_2_end[HW_MAX_SCREEN_SIZE];       /* space for interpolating */
HW_fixed G_3_start[HW_MAX_SCREEN_SIZE];     /* Z R G B Tx Ty */
HW_fixed G_3_end[HW_MAX_SCREEN_SIZE];
HW_fixed G_4_start[HW_MAX_SCREEN_SIZE];
HW_fixed G_4_end[HW_MAX_SCREEN_SIZE];
HW_fixed G_5_start[HW_MAX_SCREEN_SIZE];
HW_fixed G_5_end[HW_MAX_SCREEN_SIZE];

HW_fixed *G_start[C_MAX_DIMENSIONS]=
{G_0_start,G_1_start,G_2_start,G_3_start,G_4_start,G_5_start};
HW_fixed *G_end[C_MAX_DIMENSIONS]=
{G_0_end,G_1_end,G_2_end,G_3_end,G_4_end,G_5_end};

#if defined(_Z_BUFFER_)                     /* Z R G B Tx Ty */
 #define G_Z_INDX_START  G_0_start          /* RGB color, z-buffer */
 #define G_R_INDX_START  G_1_start
 #define G_G_INDX_START  G_2_start
 #define G_B_INDX_START  G_3_start
 #define G_TX_INDX_START G_4_start
 #define G_TY_INDX_START G_5_start

 #define G_Z_INDX_END  G_0_end              /* RGB color, z-buffer */
 #define G_R_INDX_END  G_1_end
 #define G_G_INDX_END  G_2_end
 #define G_B_INDX_END  G_3_end
 #define G_TX_INDX_END G_4_end
 #define G_TY_INDX_END G_5_end
#endif
#if defined(_PAINTER_)                      /* R G B Tx Ty */
 #define G_R_INDX_START  G_0_start          /* RGB colou, painter */
 #define G_G_INDX_START  G_1_start
 #define G_B_INDX_START  G_2_start
 #define G_TX_INDX_START G_3_start
 #define G_TY_INDX_START G_4_start

 #define G_R_INDX_END  G_0_end              /* RGB colou, painter */
 #define G_G_INDX_END  G_1_end
 #define G_B_INDX_END  G_2_end
 #define G_TX_INDX_END G_3_end
 #define G_TY_INDX_END G_4_end
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Scan converting a N dimensional line.       *
 * ---------                                             *
 * SETS: G_x_start,G_x_end,G_start,G_end,G_miny,G_maxy   *
 * -----                                                 *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void GI_scan(int *edges,int dimension,int skip)
{
 HW_fixed cur_v[C_MAX_DIMENSIONS];          /* initial values for Z+ dims */
 HW_fixed inc_v[C_MAX_DIMENSIONS];          /* increment for Z+ dimensions */
 int dx,dy,long_d,short_d;
 int d,add_dh,add_dl;
 int inc_xh,inc_yh,inc_xl,inc_yl;
 int x,y,i,j;
 int *v1,*v2;                               /* first and second vertices */

 v1=edges; edges+=skip; v2=edges;           /* length ints in each */

 if(C_line_y_clipping(&v1,&v2,dimension))   /* vertical clipping */
 {
  dx=*v2++; dy=*v2++;                       /* extracting 2D coordinates */
  x=*v1++; y=*v1++;                         /* v2/v1 point remaining dim-2 */
  dimension-=2;

  if(y<G_miny) G_miny=y;
  if(y>G_maxy) G_maxy=y;
  if(dy<G_miny) G_miny=dy;
  if(dy>G_maxy) G_maxy=dy;                  /* updating vertical size */

  dx-=x; dy-=y;                             /* ranges */

  if(dx<0){dx=-dx; inc_xh=-1; inc_xl=-1;}   /* making sure dx and dy >0 */
  else    {        inc_xh=1;  inc_xl=1; }   /* adjusting increments */
  if(dy<0){dy=-dy; inc_yh=-1; inc_yl=-1;}
  else    {        inc_yh=1;  inc_yl=1; }

  if(dx>dy){long_d=dx;short_d=dy;inc_yl=0;} /* long range,&make sure either */
  else     {long_d=dy;short_d=dx;inc_xl=0;} /* x or y is changed in L case */

  d=2*short_d-long_d;                       /* initial value of d */
  add_dl=2*short_d;                         /* d adjustment for H case */
  add_dh=2*(short_d-long_d);                /* d adjustment for L case */

#if defined(_Z_BUFFER_)
  cur_v[0]=((HW_fixed)v1[0])<<G_P;          /* Z */
  if(long_d>0)
   inc_v[0]=(((HW_fixed)(v2[0]-v1[0]))<<G_P)/long_d;
  i=1;                                      /* the rest */
#endif
#if defined(_PAINTER_)
  i=0;                                      /* all */
#endif

  for(;i<dimension;i++)                     /* for all remaining dimensions */
  {
   cur_v[i]=((HW_fixed)v1[i]);
   if(long_d>0)
    inc_v[i]=((HW_fixed)(v2[i]-v1[i]))/long_d;
  }

  for(i=0;i<=long_d;i++)                    /* for all points in long range */
  {
   if(x<G_x_start[y])                       /* further than rightmost */
   {
    G_x_start[y]=x;                         /* the begining of scan line */
    for(j=0;j<dimension;j++)
     G_start[j][y]=cur_v[j];                /* all other dimensions */
   }

   if(G_x_end[y]<x)                         /* further the leftmost */
   {
    G_x_end[y]=x;                           /* the end of scan line */
    for(j=0;j<dimension;j++)
     G_end[j][y]=cur_v[j];                  /* and for other dimension */
   }

   if(d>=0){x+=inc_xh;y+=inc_yh;d+=add_dh;} /* previous point was H type */
   else    {x+=inc_xl;y+=inc_yl;d+=add_dl;} /* previous point was L type */
   for(j=0;j<dimension;j++)
    cur_v[j]+=inc_v[j];                     /* for all other dimensions */
  }
 }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * INTERNAL: Initialization of polygon boundaries.       *
 * ---------                                             *
 * SETS: G_miny,G_maxy,G_x_start,G_x_end                 *
 * -----                                                 *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void GI_boarder_array_init(void)
{
 int i;

 G_miny=INT_MAX;                            /* polygon starts here */
 G_maxy=INT_MIN;                            /* polygon ends here */

 for(i=0;i<HW_screen_y_size;i++)            /* initializing the arrays */
 {
  G_x_start[i]=INT_MAX;
  G_x_end[i]=INT_MIN;
 }
}

/**********************************************************\
 * Rendering a polygon.                                   *
 * Accepts a stream of two-tuples X Y (_PAINTER_) or      *
 * three-tuples X Y Z (_Z_BUFFER_).                       *
\**********************************************************/

void G_flat_polygon(int *edges,int length,int red,int green,int blue)
{
 int new_edges[G_MAX_POLYGON_VERTICES*G_LNG_FLAT];
 int new_length,i;                          /* although no edges there yet */
 long pos;
 HW_pixel_ptr adr_c;                        /* position in the Color buffer */
 int beg,end,span;
#if defined(_Z_BUFFER_)
 int *adr_z;                                /* position in the Z buffer */
 HW_fixed cur_z,inc_z;                      /* current deapth */
#endif

 GI_boarder_array_init();                   /* initializing the arrays */

 new_length=C_polygon_x_clipping(edges,new_edges,G_LNG_FLAT,length);

 for(i=0;i<new_length;i++)                  /* Searching polygon boarders */
  GI_scan(&new_edges[i*G_LNG_FLAT],G_LNG_FLAT,G_LNG_FLAT);

 if(G_miny<G_maxy)                          /* nothing to do? */
 {
  pos=G_miny*HW_screen_x_size;

  for(;G_miny<G_maxy;G_miny++,pos+=HW_screen_x_size)
  {                                         /* rendering all lines */
   adr_c=G_c_buffer+(pos+(beg=G_x_start[G_miny]))*HW_pixel_size;
#if defined(_Z_BUFFER_)
   adr_z=G_z_buffer+pos+beg;                /* corresponding position in Z buffer */
#endif
   end=G_x_end[G_miny];                     /* ends here */
   span=end-beg;

#if defined(_Z_BUFFER_)
   cur_z=G_Z_INDX_START[G_miny];
   if(span!=0)
    inc_z=(G_Z_INDX_END[G_miny]-cur_z)/span;

   for(;beg<=end;beg++,adr_c+=HW_pixel_size,adr_z++,cur_z+=inc_z)
   {                                        /* render this lines */
    if(*adr_z>(cur_z>>G_P))                 /* Z buffer check */
    {
     *adr_z=cur_z>>G_P;                     /* store new deapth here */
     HW_pixel(adr_c,red,green,blue);
    }
   }
#endif
#if defined(_PAINTER_)
   for(;beg<=end;beg++,adr_c+=HW_pixel_size)
    HW_pixel(adr_c,red,green,blue);         /* drawing a strip */
#endif
  }
 }
}

/**********************************************************\
 * Rendering an Interpolatively shaded polygon.           *
 * Accepts a stream of 5-tuples ( X Y R G B )             *
 * similarely it is ( X Y Z etc. ) if _Z_BUFFER_          *
\**********************************************************/

void G_shaded_polygon(int *edges,int length)
{
 int new_edges[G_MAX_POLYGON_VERTICES*G_LNG_SHADED];
 int new_length,i;
 long pos;
 HW_pixel_ptr adr_c;
 int beg,end,span;
 HW_fixed cur_r,inc_r,cur_g,inc_g,cur_b,inc_b;
#if defined(_Z_BUFFER_)
 int *adr_z;                                /* position in the Z buffer */
 HW_fixed cur_z,inc_z;                      /* current depth */
#endif

 GI_boarder_array_init();                   /* initializing the array */

 new_length=C_polygon_x_clipping(edges,new_edges,G_LNG_SHADED,length);

 for(i=0;i<new_length;i++)                  /* Searching polygon boarders */
  GI_scan(&new_edges[i*G_LNG_SHADED],G_LNG_SHADED,G_LNG_SHADED);

 if(G_miny<G_maxy)                          /* nothing to do? */
 {
  pos=G_miny*HW_screen_x_size;

  for(;G_miny<G_maxy;G_miny++,pos+=HW_screen_x_size)
  {                                         /* rendering all lines */
   adr_c=G_c_buffer+(pos+(beg=G_x_start[G_miny]))*HW_pixel_size;
#if defined(_Z_BUFFER_)
   adr_z=G_z_buffer+pos+beg;                /* corresponding place in Z buffer */
#endif
   end=G_x_end[G_miny];                     /* ends here */
   span=end-beg;
   if(span==0) span=1;                      /* not to divide by 0 */

   cur_r=G_R_INDX_START[G_miny];
   inc_r=(G_R_INDX_END[G_miny]-cur_r)/span;
   cur_g=G_G_INDX_START[G_miny];
   inc_g=(G_G_INDX_END[G_miny]-cur_g)/span;
   cur_b=G_B_INDX_START[G_miny];
   inc_b=(G_B_INDX_END[G_miny]-cur_b)/span;
#if defined(_Z_BUFFER_)
   cur_z=G_Z_INDX_START[G_miny];
   inc_z=(G_Z_INDX_END[G_miny]-cur_z)/span;

   for(;beg<=end;beg++,adr_c+=HW_pixel_size,adr_z++)     
   {                                        /* render this lines */
    if(*adr_z>(cur_z>>G_P))                 /* Z buffer check */
    {
     *adr_z=cur_z>>G_P;                     /* store new deapth here */
#endif
#if defined(_PAINTER_)
   for(;beg<=end;beg++,adr_c+=HW_pixel_size)/* same, no Z check */
   {
    {
#endif
     HW_pixel(adr_c,cur_r>>G_P,cur_g>>G_P,cur_b>>G_P);
    }
    cur_r+=inc_r;
    cur_g+=inc_g;
    cur_b+=inc_b;                           /* increment RGB */
#if defined(_Z_BUFFER_)
    cur_z+=inc_z;                           /* increment Z */
#endif
   }
  }
 }
}

/**********************************************************\
 * Rendering a linerely textured polygon.                 *
 * Accepts a stream of 7-tuples ( X Y R G B U V)          *
 * similarely it is one more ( X Y Z etc. ) if _Z_BUFFER_ *
\**********************************************************/

void G_lin_textured_polygon(int *edges,int length,struct G_texture* texture)
{
 int new_edges[G_MAX_POLYGON_VERTICES*G_LNG_TEXTURED];
 int new_length,i,ta; 
 long pos;
 HW_pixel_ptr adr_c;
 int beg,end,span;
 HW_fixed cur_tx,inc_tx;                    /* current position inside */
 HW_fixed cur_ty,inc_ty;                    /* the texture */
 HW_fixed cur_r,inc_r,cur_g,inc_g,cur_b,inc_b;
#if defined(_Z_BUFFER_)
 int *adr_z;                                /* position in the Z buffer */
 HW_fixed cur_z,inc_z;                      /* current deapth */
#endif
 int log_size=texture->g_log_texture_size;
 HW_fixed txtrmasc=(0x1<<(log_size+G_P))-0x1;

 GI_boarder_array_init();                   /* initializing the array */

 new_length=C_polygon_x_clipping(edges,new_edges,G_LNG_TEXTURED,length);

 for(i=0;i<new_length;i++)
  GI_scan(&new_edges[i*G_LNG_TEXTURED],G_LNG_TEXTURED,G_LNG_TEXTURED);

 if(G_miny<G_maxy)                          /* nothing to do? */
 {
  pos=G_miny*HW_screen_x_size;

  for(;G_miny<G_maxy;G_miny++,pos+=HW_screen_x_size)
  {                                         /* rendering all lines */
   adr_c=G_c_buffer+(pos+(beg=G_x_start[G_miny]))*HW_pixel_size;
#if defined(_Z_BUFFER_)
   adr_z=G_z_buffer+pos+beg;                /* corresponding place in Z buffer */
#endif
   end=G_x_end[G_miny];                     /* ends here */
   span=end-beg;
   if(span==0) span=1;                      /* not to divide by 0 */

   cur_tx=G_TX_INDX_START[G_miny];
   inc_tx=(G_TX_INDX_END[G_miny]-cur_tx)/span;
   cur_ty=G_TY_INDX_START[G_miny];
   inc_ty=(G_TY_INDX_END[G_miny]-cur_ty)/span;
   cur_r=G_R_INDX_START[G_miny];
   inc_r=(G_R_INDX_END[G_miny]-cur_r)/span;
   cur_g=G_G_INDX_START[G_miny];
   inc_g=(G_G_INDX_END[G_miny]-cur_g)/span;
   cur_b=G_B_INDX_START[G_miny];
   inc_b=(G_B_INDX_END[G_miny]-cur_b)/span;
#if defined(_Z_BUFFER_)
   cur_z=G_Z_INDX_START[G_miny];
   inc_z=(G_Z_INDX_END[G_miny]-cur_z)/span;

   for(;beg<=end;beg++,adr_c+=HW_pixel_size,adr_z++)
   {
    cur_tx&=txtrmasc; cur_ty&=txtrmasc;

    if(*adr_z>(cur_z>>G_P))                 /* Z buffer check */
    {
     *adr_z=cur_z>>G_P;
#endif
#if defined(_PAINTER_)
   for(;beg<=end;beg++,adr_c+=HW_pixel_size)/* render all lines: */
   {
    cur_tx&=txtrmasc;
    cur_ty&=txtrmasc;                       /* wrap around */
    {
#endif
     ta=((cur_ty>>G_P)<<log_size)+(cur_tx>>G_P);
                                            /* position in the texture */

     HW_pixel(adr_c,(texture->g_red[ta]*(cur_r>>G_P))>>8,
                    (texture->g_green[ta]*(cur_g>>G_P))>>8,
                    (texture->g_blue[ta]*(cur_b>>G_P))>>8);

     cur_r+=inc_r;
     cur_g+=inc_g;
     cur_b+=inc_b;
    }
    cur_tx+=inc_tx;
    cur_ty+=inc_ty;                         /* new position inside texture */
#if defined(_Z_BUFFER_)
    cur_z+=inc_z;
#endif
   }
  }
 }
}

/**********************************************************\
 * Rendering a perspectively textured polygon.            *
 * The applied texture is a square bitmap with:           *
 * Accepts a stream of 7-tuples ( X Y R G B U V)          *
 * similarely it is one more ( X Y Z etc. ) if _Z_BUFFER_ *
\**********************************************************/

void G_prp_textured_polygon(int *edges,int length,
                            int *O,int *u,int *v,int texture_space_size,
                            struct G_texture* texture)
{
 int new_edges[G_MAX_POLYGON_VERTICES*G_LNG_TEXTURED];
 int new_length,i,ta;
 HW_fixed Vx,Vy,Vz;
 HW_fixed Ux,Uy,Uz;                         /* extracting vectors */
 HW_fixed x0,y0,z0;
 HW_fixed ui,uj,uc;
 HW_fixed vi,vj,vc;
 HW_fixed zi,zj,zc;                         /* back to texture coeficients */
 HW_fixed v0,u0;
 HW_fixed xx,yy,zz,zzz;
 long pos;
 HW_pixel_ptr adr_c;
 int beg,end,span;
 HW_fixed cur_tx,inc_tx,end_tx,cur_ty,inc_ty,end_ty;
 HW_fixed cur_r,inc_r,cur_g,inc_g,cur_b,inc_b;
#if defined(_Z_BUFFER_)
 int *adr_z;                                /* position in the Z buffer */
 HW_fixed cur_z,inc_z;                      /* current deapth */
#endif
 int x,y;
 int log_size=texture->g_log_texture_size;
 HW_fixed txtrmasc=(0x1<<(log_size+G_P))-0x1;

 GI_boarder_array_init();                   /* initializing the array */

 new_length=C_polygon_x_clipping(edges,new_edges,G_LNG_TEXTURED,length);

 for(i=0;i<new_length;i++)
  GI_scan(&new_edges[i*G_LNG_TEXTURED],G_LNG_SHADED,G_LNG_TEXTURED);

 if(G_miny<G_maxy)                          /* nothing to do? */
 {
  x0=O[0]; y0=O[1]; z0=O[2];                /* X Y Z */
  u0=O[M_LNG_SHADED];
  v0=O[M_LNG_SHADED+1];                     /* world point <-> texture point */

  Vx=v[0]; Vy=v[1]; Vz=v[2];
  Ux=u[0]; Uy=u[1]; Uz=u[2];                /* extracting vectors */

  ui=(Vz*y0)-(Vy*z0);
  uj=(Vx*z0)-(Vz*x0);
  uc=(Vy*x0)-(Vx*y0);
  vi=(Uy*z0)-(Uz*y0);
  vj=(Uz*x0)-(Ux*z0);
  vc=(Ux*y0)-(Uy*x0);
  zi=(Vy*Uz)-(Vz*Uy);
  zj=(Vz*Ux)-(Vx*Uz);
  zc=(Vx*Uy)-(Vy*Ux);                       /* back to texture coefs */

  pos=G_miny*HW_screen_x_size;

  for(;G_miny<G_maxy;G_miny++,pos+=HW_screen_x_size)
  {                                         /* rendering all lines */
   adr_c=G_c_buffer+(pos+(beg=G_x_start[G_miny]))*HW_pixel_size;
#if defined(_Z_BUFFER_)
   adr_z=G_z_buffer+pos+beg;                /* corresponding place in Z buffer */
#endif
   end=G_x_end[G_miny];                     /* ends here */
   span=end-beg;
   if(span==0) span=1;                      /* not to divide by 0 */

   beg-=HW_screen_x_size/2;
   x=beg;
   end-=HW_screen_x_size/2;
   y=G_miny-HW_screen_y_size/2;             /* pure perspective space */

   cur_r=G_R_INDX_START[G_miny];
   inc_r=(G_R_INDX_END[G_miny]-cur_r)/span;
   cur_g=G_G_INDX_START[G_miny];
   inc_g=(G_G_INDX_END[G_miny]-cur_g)/span;
   cur_b=G_B_INDX_START[G_miny];
   inc_b=(G_B_INDX_END[G_miny]-cur_b)/span;
#if defined(_Z_BUFFER_)
   cur_z=G_Z_INDX_START[G_miny];
   inc_z=(G_Z_INDX_END[G_miny]-cur_z)/span;
#endif

   xx=((y*uj)>>M_camera_log_focus)+uc;
   yy=((y*vj)>>M_camera_log_focus)+vc;
   zz=((y*zj)>>M_camera_log_focus)+zc;      /* valid for the whole run */

   if((zzz=zz+((x*zi)>>M_camera_log_focus))!=0)
   {
    end_tx=((((xx+((x*ui)>>M_camera_log_focus))<<texture_space_size)/zzz)<<G_P)+u0;
    end_ty=((((yy+((x*vi)>>M_camera_log_focus))<<texture_space_size)/zzz)<<G_P)+v0;
   } else { end_tx=end_ty=0; }              /* not important actually */

   for(;beg<=end;)
   {
    x+=G_LINEAR; if(x>end) x=end;           /* size of linear run */
    cur_tx=end_tx;
    cur_ty=end_ty;

    if((zzz=zz+((x*zi)>>M_camera_log_focus))!=0)
    {
     end_tx=((((xx+((x*ui)>>M_camera_log_focus))<<texture_space_size)/zzz)<<G_P)+u0;
     end_ty=((((yy+((x*vi)>>M_camera_log_focus))<<texture_space_size)/zzz)<<G_P)+v0;
    } else { end_tx=end_ty=0; }             /* not important to what */

    if((span=x-beg)!=0)                     /* ends here */
    {
     inc_tx=(end_tx-cur_tx)/span;
     inc_ty=(end_ty-cur_ty)/span;
    } else { inc_tx=inc_ty=0; }             /* not important to what */

#if defined(_Z_BUFFER_)
    for(;beg<=x;beg++,adr_c+=HW_pixel_size,adr_z++)
    {
     cur_tx&=txtrmasc;
     cur_ty&=txtrmasc;

     if(*adr_z>(cur_z>>G_P))                /* Z buffer check */
     {
      *adr_z=cur_z>>G_P;
#endif
#if defined(_PAINTER_)
    for(;beg<=x;beg++,adr_c+=HW_pixel_size) /* linear run */
    {
     cur_tx&=txtrmasc;
     cur_ty&=txtrmasc;                      /* wrap around */
     {
#endif
     ta=((cur_ty>>G_P)<<log_size)+(cur_tx>>G_P);
                                            /* position in the texture */
     HW_pixel(adr_c,(texture->g_red[ta]*(cur_r>>G_P))>>8,
                    (texture->g_green[ta]*(cur_g>>G_P))>>8,
                    (texture->g_blue[ta]*(cur_b>>G_P))>>8);

      cur_r+=inc_r;
      cur_g+=inc_g;
      cur_b+=inc_b;
     }
     cur_tx+=inc_tx;
     cur_ty+=inc_ty;                        /* new position in the texture */
#if defined(_Z_BUFFER_)
     cur_z+=inc_z;
#endif
    }
   }
  }
 }
}

/**********************************************************/
