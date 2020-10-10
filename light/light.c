/** 3Dgpl3 ************************************************\
 * Header for lighting and material.                      *
 *                                                        *
 * Defines:                                               *
 *  L_init_lighting          Set lighting model;          *
 *  L_init_material          Set current material;        *
 *  L_light_vertex           Light one vertex.            *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include <stdlib.h>                         /* NULL */
#include "../light/light.h"                 /* L_ etc. */

/**********************************************************/

struct L_material* L_material;              /* current material */

struct L_light** L_lights;                  /* array of lights */
int L_no_lights;

/**********************************************************\
 * Setting current list of lightsources.                  *
 *                                                        *
 * SETS: L_lights, L_no_lights.                           *
 * -----                                                  *
\**********************************************************/

void L_init_lights(struct L_light** lights,int no_lights)
{
 if(lights==NULL) HW_error("(Light) No lights.\n");

 L_lights=lights;
 L_no_lights=no_lights;
}

/**********************************************************\
 * Setting current material.                              *
 *                                                        *
 * SETS: L_material.                                      *
 * -----                                                  *
\**********************************************************/

void L_init_material(struct L_material* mat)
{
 if(mat==NULL) HW_error("(Light) No material.\n");

 L_material=mat;
}

/**********************************************************\
 * Lighting a single vertex using a list of light sources *
\**********************************************************/

void L_light_vertex(int *color,int *vertex,int *normal)
{
 int j,light_vector[T_LNG_VECTOR],prd;
 struct L_light* light;

 color[0]=0;                                /* RGB */
 color[1]=0;
 color[2]=0;

 for(j=0;j<L_no_lights;j++)
 {
  light=L_lights[j];
  switch(light->l_type)
  {
   case L_AMBIENT:                          /* uniform illumination */
   {
    color[0]+=(light->l_red*L_material->l_amb_red)>>8;
    color[1]+=(light->l_green*L_material->l_amb_green)>>8;
    color[2]+=(light->l_blue*L_material->l_amb_blue)>>8;
   }
   break;

   case L_POINT:                            /* depends on the unit vector */
   {
    T_unit_vector(vertex,light->l_parameter,light_vector);
    prd=T_scalar_product(light_vector,normal);
    if(prd<0) break;

    color[0]+=(((prd*light->l_red)>>T_P_NORMAL)*L_material->l_dif_red)>>8;
    color[1]+=(((prd*light->l_green)>>T_P_NORMAL)*L_material->l_dif_green)>>8;
    color[2]+=(((prd*light->l_blue)>>T_P_NORMAL)*L_material->l_dif_blue)>>8;
   }
   break;

   case L_DIRECT:                           /* depends on the normal */
   {
    prd=-T_scalar_product(light->l_parameter,normal);
    if(prd<0) break;

    color[0]+=(((prd*light->l_red)>>T_P_NORMAL)*L_material->l_dif_red)>>8;
    color[1]+=(((prd*light->l_green)>>T_P_NORMAL)*L_material->l_dif_green)>>8;
    color[2]+=(((prd*light->l_blue)>>T_P_NORMAL)*L_material->l_dif_blue)>>8;
   }
   break;
  }
 }
}

/**********************************************************/
