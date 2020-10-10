/** 3Dgpl3 ************************************************\
 * Functions to manage groups of objects.                 *
 *                                                        *
 * Defines:                                               *
 *  M_init_group             All objects in the group;    *
 *  M_light_group            All objects in the group;    *
 *  M_render_group           Rendering several objects.   *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../engine/engine.h"               /* 3D engine */
#include "../trans/trans.h"                 /* 3D transformations */
#include "../light/light.h"                 /* L_init_lighting */

/**********************************************************\
 * Initialising object in the group.                      *
\**********************************************************/

void M_init_group(struct M_group *group)
{
 int i;

 for(i=0;i<group->m_no_objects;i++)         /* initialize objects */
 {
  if(group->m_objects[i]->m_type==M_POLYGON_OBJECT)
   M_init_polygon_object((struct M_polygon_object*)group->m_objects[i]);
  else
   M_init_bicubic_object((struct M_bicubic_object*)group->m_objects[i]);
 }
}

/**********************************************************\
 * Lighting group of objects.                             *
\**********************************************************/

void M_light_group(struct M_group *group)
{
 int i;
 int *centres=group->m_centres;
 unsigned char *orientations=group->m_orientations;

 L_init_lights(group->m_lights,group->m_no_lights);

 for(i=0;i<group->m_no_objects;i++,centres+=T_LNG_VECTOR,
     orientations+=T_LNG_VECTOR)
 {
  if(group->m_objects[i]->m_type==M_POLYGON_OBJECT)
   M_light_polygon_object((struct M_polygon_object*)group->m_objects[i],
                          centres[0],centres[1],centres[2],
                          orientations[0],orientations[1],orientations[2]);
  else
   M_light_bicubic_object((struct M_bicubic_object*)group->m_objects[i],
                          centres[0],centres[1],centres[2],
                          orientations[0],orientations[1],orientations[2]);
 }
}

/**********************************************************\
 * Rendering a group of objects in reversed order.        *
\**********************************************************/

void M_render_group(struct M_group *group,
                    int x,int y,int z)
{
 int numbers[M_MAX_GROUP_OBJECTS];          /* indices to objects */
 int vxes[M_MAX_GROUP_OBJECTS*T_LNG_VECTOR];/* X Y Z */
 int i,pos;
 int *centres=vxes;
 unsigned char *orientations=group->m_orientations;

 if(group->m_no_objects>=M_MAX_GROUP_OBJECTS)
  HW_error("(Engine) Too many object per group.\n");

 for(i=0;i<group->m_no_objects;i++) numbers[i]=i;

#if defined(_PAINTER_)                      /* +2 so that to get to Z */
 T_world_rotation(group->m_centres,vxes,group->m_no_objects);
 M_sort_elements(vxes+2,T_LNG_VECTOR,numbers,group->m_no_objects);
#endif

 T_translation(group->m_centres,vxes,group->m_no_objects,x,y,z);

 for(i=0;i<group->m_no_objects;i++)
 {
  pos=T_LNG_VECTOR*numbers[i];              /* index of proper parameters */
  if(group->m_objects[numbers[i]]->m_type==M_POLYGON_OBJECT)
   M_render_polygon_object((struct M_polygon_object*)group->m_objects[numbers[i]],
                           centres[pos],centres[pos+1],centres[pos+2],
                           orientations[pos],orientations[pos+1],
                           orientations[pos+2]);
  else
   M_render_bicubic_object((struct M_bicubic_object*)group->m_objects[numbers[i]],
                           centres[pos],centres[pos+1],centres[pos+2],
                           orientations[pos],orientations[pos+1],
                           orientations[pos+2]);
 }
}

/**********************************************************/
