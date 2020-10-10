/** 3Dgpl3 ************************************************\
 * Sample application:                                    *
 *  Draws a group of polygonal and/or bicubic models.     *
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

unsigned char alp=0,bet=0,gam=0;            /* camera parameters */
int x=0,y=0,z=0;

struct M_group *group;                      /* group of objects */

/**********************************************************/

void handler(int key)                       /* event handler */
{
 switch(key)
 {
  case HW_KEY_ENTER:       HW_close_event_loop(); break;

  case HW_KEY_ARROW_RIGHT: gam+=5; break;
  case HW_KEY_ARROW_LEFT:  gam-=5; break;
  case HW_KEY_ARROW_UP:    bet+=5; break;
  case HW_KEY_ARROW_DOWN:  bet-=5; break;
  case 'X': case 'x':      alp+=5; break;
  case 'S': case 's':      alp-=5; break;
  case 'A': case 'a':      z-=15;   break;
  case 'Z': case 'z':      z+=15;   break;

  case 'P': case 'p':      M_init_rendering(M_POINT); break;
  case 'W': case 'w':      M_init_rendering(M_WIRE); break;
  case 'F': case 'f':      M_init_rendering(M_FLAT); break;
  case 'G': case 'g':      M_init_rendering(M_SHADED); break;
  case 'T': case 't':      M_init_rendering(M_TEXTURED); break;
 }
}

/**********************************************************/

void frame(void)                            /* rendering loop */
{
 int i;

 G_clear(0,0,0);
 M_init_camera(0,0,0,x,y,z,8);

 for(i=0;i<group->m_no_objects;i++)
 {
  group->m_orientations[i*4]=alp;           /* rotate the objects */
  group->m_orientations[i*4+1]=bet;     
  group->m_orientations[i*4+2]=gam;     
 }
 M_light_group(group);
 M_render_group(group,0,0,0);

 G_text(0,0,"<enter> <arrows,x,s,a,z> <p,w,f,g,t>",255,255,255);

 HW_blit();
}

/**********************************************************/

void idle(void)                             /* called when no events */
{                                           /* to process */
 frame();
}

/**********************************************************/

int main(int no, char **o)                  /* initialize, launch the loop */
{
 group=D_data("data/grup_app.dat");

 HW_init_screen("3Dgpl3 - Z buffer",300,300);

 M_init_rendering(M_TEXTURED);              /* rendering option */
 M_init_group(group);                       /* including light sources */

 HW_init_event_loop(frame,handler,idle);    /* run event loop */
 HW_close_screen();

 return(1);
}

/**********************************************************/
