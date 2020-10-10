#if !defined(_CLIPPER_H_)
#define _CLIPPER_H_

/** 3Dgpl3 ************************************************\
 * Header for clipping functions.                         *
 *                                                        *
 * Files:                                                 *
 *  clipp-2d.c               3D volume and Z clipping;    *
 *  clipp-3d.c               2D plane clipping.           *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#define C_MAX_DIMENSIONS               8    /* X Y Z R G B Tx Ty */
#define C_Z_CLIPPING_MIN              10    /* where z plane is */

/**********************************************************/

void C_init_clipping(int minx,int miny,int maxx,int maxy);

int C_line_x_clipping(int **vertex1,int **vertex2,int dimension);
int C_line_y_clipping(int **vertex1,int **vertex2,int dimension);
int C_polygon_x_clipping(int *from,int *to,int dimension,int length);

int C_volume_clipping(int *min,int *max);
int C_line_z_clipping(int **vertex1,int **vertex2,int dimension);
int C_polygon_z_clipping(int *from,int *to,int dimension,int length);
int C_line_xyz_clipping(int **vertex1,int **vertex2,
                        int *by1,int *by2,int *by3,
                        int dimension);
int C_polygon_xyz_clipping(int *from,int *to,
                           int *by1,int *by2,int *by3,
                           int dimension,int length);

/**********************************************************/

#endif
