/** 3Dgpl3 ************************************************\
 * Sample application:                                    *
 *  Draws a scene using ray-tracing.                      *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include <stdio.h>
#include <string.h>
#include "../hardware/hardware.h"        /* HW_ stuff */
#include "../data/data.h"                /* D_data */
#include "../trace/trace.h"              /* TR_ stuff */
#include "../graphics/graphics.h"        /* G_text */

struct TR_world *w;
char *message="Specular, shadows, reflections";

/**********************************************************/

void handler(int key)                    /* event handler */
{
 switch(key)
 {
  case HW_KEY_ENTER:       HW_close_event_loop(); break;

  case 'R': case 'r':      TR_init_rendering(TR_SPECULAR|TR_SHADOW|TR_REFLECT);
                           message="Specular, shadows, reflections";
                           break;
  case 'H': case 'h':      TR_init_rendering(TR_SPECULAR|TR_SHADOW);
                           message="Specular, shadows";
                           break;
  case 'S': case 's':      TR_init_rendering(TR_SPECULAR);
                           message="Specular";
                           break;
  case 'D': case 'd':      TR_init_rendering(TR_DIFFUSE);
                           message="Diffuse";
                           break;
  case 'A': case 'a':      TR_init_rendering(TR_AMBIENT);
                           message="Ambient only";
                           break;
 }
}

/**********************************************************/

void frame(void)                         /* rendering loop */
{
 G_clear(0,0,0);

 TR_init_camera(0,0,-256, 0,0,0, 1,0,0, 0,1,0);
 TR_trace_world(w,10);

 G_text(0,0,"<enter> <a,d,s,h,r>",255,255,255);
 G_text(0,8,message,255,255,255);

 HW_blit();
}

/**********************************************************/

void idle(void)
{
}

/**********************************************************/

int main(int no,char **o)                   /* initialize, launch the loop */
{
 w=D_data("data/trac_app.dat");

 TR_init_rendering(TR_SPECULAR|TR_SHADOW|TR_REFLECT);
 TR_init_world(w);

 HW_init_screen("3Dgpl3 - Ray tracer",300,300);
 HW_init_event_loop(frame,handler,idle);    /* run event loop */
 HW_close_screen();

 return(1);
}

/**********************************************************/
