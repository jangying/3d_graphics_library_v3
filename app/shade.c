/** 3Dgpl3 ************************************************\
 * Sample application:                                    *
 *  Draws spheres and cylinders under different shading.  *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* HW_ functions */
#include "../graphics/graphics.h"           /* G_ functions */
#include "../data/data.h"                   /* D_data */
#include "../engine/engine.h"               /* M_ functions */
#include "../light/light.h"                 /* L_init_lights */
#include "../shape/shape.h"                 /* S_sphere etc. */

struct M_group *group;                      /* group of objects */
char *message="Gouraud shaded and textured";

/**********************************************************/

void handler(int key)                       /* event handler */
{
 switch(key)
 {
  case HW_KEY_ENTER:       HW_close_event_loop(); break;

  case 'P': case 'p':      M_init_rendering(M_POINT); message="Key points";
                           break;
  case 'W': case 'w':      M_init_rendering(M_WIRE); message="Wire frame";
                           break;
  case 'F': case 'f':      M_init_rendering(M_FLAT); message="Flat shaded";
                           break;
  case 'G': case 'g':      M_init_rendering(M_SHADED); message="Gouraud shaded";
                           break;
  case 'T': case 't':      M_init_rendering(M_TEXTURED); 
                           message="Gouraud shaded and textured";
                           break;

 }
}

/**********************************************************/

void frame(void)                            /* rendering loop */
{
 G_clear(0,0,0);

 M_render_group(group,0,0,0);

 G_text(0,0,"<enter> <p,w,f,g,t>",255,255,255);
 G_text(0,8,message,255,255,255);

 HW_blit();
}

/**********************************************************/

void idle(void)                             /* called when no events */
{                                           /* to process */
 frame();
}

/**********************************************************/

struct L_material mat={255,255,255,255,255,255};

int main(int no, char **o)                  /* initialize, launch the loop */
{
 group=D_data("data/shad_app.dat");

 HW_init_screen("3Dgpl3 - Shading",300,300);

 S_init_construction(D_data("data/rust.pic"),7,&mat);
 group->m_objects[0]=(struct M_generic_object*)S_sphere(90,5);
 group->m_objects[1]=(struct M_generic_object*)S_cylinder(300,20,6);
 group->m_objects[2]=(struct M_generic_object*)S_sphere(120,8);

 M_init_rendering(M_TEXTURED);              /* rendering option */
 M_init_group(group);                       /* including light sources */
 M_light_group(group);
 M_init_camera(0,0,0,0,0,0,8);

 HW_init_event_loop(frame,handler,idle);    /* run event loop */
 HW_close_screen();

 return(1);
}

/**********************************************************/
