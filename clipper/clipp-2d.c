/** 3Dgpl3 ************************************************\
 * 2D line and polygon clipping.                          *
 *                                                        *
 * Defines:                                               *
 *  C_init_clipping          Setting bounds;              *
 *  C_line_x_clipping        Clipping a line horizontally;*
 *  C_line_y_clipping        Clipping a line vertically;  *
 *  C_polygon_x_clipping     Horizontal polygon cliping.  *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../clipper/clipper.h"             /* 2D macros */
#include <string.h>                         /* memcpy */

/**********************************************************/

int C_2D_clipping;                          /* type of performed clipping */
int C_x_clipping_min;                       /* clipping bounds */
int C_x_clipping_max;                       
int C_y_clipping_min;
int C_y_clipping_max;

/**********************************************************\
 * Setting bounds for 2D clipping.                        *
 *                                                        *
 * SETS: C_x_clipping_min,...,C_y_clipping_max            *
 * -----                                                  *
\**********************************************************/

void C_init_clipping(int minx,int miny,int maxx,int maxy)
{
 C_x_clipping_min=minx;                     /* no error checking... */
 C_y_clipping_min=miny;
 C_x_clipping_max=maxx;
 C_y_clipping_max=maxy;
}

/**********************************************************\
 * Line clipping using binary search technique.           *
 *                                                        *
 * RETUNRNS: 0 when line is compleately outside;          *
 * --------- 1 otherwise.                                 *
 *                                                        *
 * SETS:  C_2D_clipping to 1 when first or both points    *
 * -----  were changed, 0 otherwise.                      *
\**********************************************************/

int C_line_x_clipping(int **vertex1,int **vertex2,int dimension)
{
 int i;
 int whereto;
 int *l,*r,*m,*t;                           /* left right midle and tmp */
 static int g_store0[C_MAX_DIMENSIONS];     /* static stores for clipped vxes */
 static int g_store1[C_MAX_DIMENSIONS];
 static int g_store2[C_MAX_DIMENSIONS];
 static int g_store3[C_MAX_DIMENSIONS];
 static int g_store4[C_MAX_DIMENSIONS];
 static int g_store5[C_MAX_DIMENSIONS];
 int **vmn,**vmx;                           /* so that *vmn[0] < *vmx[0] */
 int swap;                                  /* were coordinates swaped? */

 C_2D_clipping=0;                           /* default no clipping yet */

 if((*vertex1)[0]<(*vertex2)[0])
 { swap=0; vmn=vertex1; vmx=vertex2; }      /* so that *vmn[0] < *vmx[0] */
 else
 { swap=1; vmn=vertex2; vmx=vertex1; }

 if(((*vmn)[0]>=C_x_clipping_max)||((*vmx)[0]<=C_x_clipping_min)) return(0);
 else
 {
  if((*vmn)[0]<=C_x_clipping_min)           /* clipping */
  {
   memcpy(m=g_store0,*vmn,dimension*sizeof(int));
   memcpy(r=g_store1,*vmx,dimension*sizeof(int));
   l=g_store2;

   whereto=1;
   while(m[0]!=C_x_clipping_min)
   {
    if(whereto==1) { t=l; l=m; m=t; }
    else           { t=r; r=m; m=t; }
    for(i=0;i<dimension;i++) m[i]=(l[i]+r[i])>>1;
    whereto=m[0]<C_x_clipping_min;
   }
   *vmn=m;                                  /* that is why m[] is static */
   C_2D_clipping=swap^1;
  }

  if((*vmx)[0]>=C_x_clipping_max)           /* clipping */
  {
   memcpy(l=g_store3,*vmn,dimension*sizeof(int));
   memcpy(m=g_store4,*vmx,dimension*sizeof(int));
   r=g_store5;

   whereto=0;
   while(m[0]!=C_x_clipping_max)
   {
    if(whereto==1) { t=l; l=m; m=t; }
    else           { t=r; r=m; m=t; }
    for(i=0;i<dimension;i++) m[i]=(l[i]+r[i])>>1;
    whereto=m[0]<C_x_clipping_max;
   }
   *vmx=m;                                  /* that is why m[] is static */
   C_2D_clipping|=swap&1;
  }
 }
 return(1);                                 /* partialy or not clipped */
}

/**********************************************************\
 * Line clipping using binary search technique.           *
 *                                                        *
 * RETUNRNS: 0 when line is compleately outside;          *
 * --------- 1 otherwise.                                 *
 *                                                        *
 * SETS:  C_2D_clipping to 1 when first or both points    *
 * -----  were changed, 0 otherwise.                      *
\**********************************************************/

int C_line_y_clipping(int **vertex1,int **vertex2,int dimension)
{
 int i;
 int whereto;
 int *l,*r,*m,*t;                           /* left right midle and tmp */
 static int g_store0[C_MAX_DIMENSIONS];     /* static stores for clipped vxes */
 static int g_store1[C_MAX_DIMENSIONS];
 static int g_store2[C_MAX_DIMENSIONS];
 static int g_store3[C_MAX_DIMENSIONS];
 static int g_store4[C_MAX_DIMENSIONS];
 static int g_store5[C_MAX_DIMENSIONS];
 int **vmn,**vmx;                           /* so that *vmn[1] < *vmx[1] */
 int swap;                                  /* were coordinates swaped? */

 C_2D_clipping=0;                           /* default no clipping yet */

 if((*vertex1)[1]<(*vertex2)[1])
 { swap=0; vmn=vertex1; vmx=vertex2; }      /* so that *vmn[1] < *vmx[1] */
 else
 { swap=1; vmn=vertex2; vmx=vertex1; }

 if(((*vmn)[1]>=C_y_clipping_max)||((*vmx)[1]<=C_y_clipping_min)) return(0);
 else
 {
  if((*vmn)[1]<=C_y_clipping_min)           /* clipping */
  {
   memcpy(m=g_store0,*vmn,dimension*sizeof(int));
   memcpy(r=g_store1,*vmx,dimension*sizeof(int));
   l=g_store2;

   whereto=1;
   while(m[1]!=C_y_clipping_min)
   {
    if(whereto==1) { t=l; l=m; m=t; }
    else           { t=r; r=m; m=t; }
    for(i=0;i<dimension;i++) m[i]=(l[i]+r[i])>>1;
    whereto=m[1]<C_y_clipping_min;
   }
   *vmn=m;                                  /* that is why m[] is static */
   C_2D_clipping=swap^1;
  }

  if((*vmx)[1]>=C_y_clipping_max)           /* clipping */
  {
   memcpy(l=g_store3,*vmn,dimension*sizeof(int));
   memcpy(m=g_store4,*vmx,dimension*sizeof(int));
   r=g_store5;

   whereto=0;
   while(m[1]!=C_y_clipping_max)
   {
    if(whereto==1) { t=l; l=m; m=t; }
    else           { t=r; r=m; m=t; }
    for(i=0;i<dimension;i++) m[i]=(l[i]+r[i])>>1;
    whereto=m[1]<C_y_clipping_max;
   }
   *vmx=m;                                  /* that is why m[] is static */
   C_2D_clipping|=swap&1;
  }
 }
 return(1);                                 /* partialy or not clipped */
}

/**********************************************************\
 * Creating a x-clipped polygon.                          *
 *                                                        *
 * RETURNS: number of elements in the clipped polygon.    *
 * -------- (0 when compleately behind view plane)        *
 *                                                        *
 *         |    |        1-2-3-4-5-6-1  -> 2'-3'-5'-6'    *
 *         |5'  |6'                                       *
 *      5*-*----*-*6     If first vertex in the line is   *
 *      /  |    |   \    clipped both vertices are        *
 *    4*   |    |    *1  copied.If no clipping or second  *
 *      \  |    |   /    one is clipped then only second  *
 *      3*-*----*-*2     one is copied. If both vertices  *
 *         |3'  |2'      are clipped, neither is copied.  *
 *                                                        *
\**********************************************************/

int C_polygon_x_clipping(int *from,int *to,int dimension,int length)
{
 int i;
 int *v1,*v2,new_lng=0;
 int *first_vrtx=to;                        /* begining of the source */

 for(i=0;i<length;i++)                      /* for all edges */
 {
  v1=(int*)from; from+=dimension; v2=(int*)from;

  if(C_line_x_clipping(&v1,&v2,dimension))  /* clipping */
  {
   if(C_2D_clipping)                        /* depends which one was clipped */
   {
    memcpy(to,v1,dimension*sizeof(int)); to+=dimension;
    memcpy(to,v2,dimension*sizeof(int)); to+=dimension;
    new_lng+=2;                             /* first point clipped */
   }
   else
   { 
    memcpy(to,v2,dimension*sizeof(int)); to+=dimension;
    new_lng++;                              /* second point clipped */
   }
  }
 }
 memcpy(to,first_vrtx,dimension*sizeof(int));

 return(new_lng);
}

/**********************************************************/
