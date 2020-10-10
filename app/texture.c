/** 3Dgpl3 ************************************************\
 * Sample application:                                    *
 *  Texture demo.                                         *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "stdlib.h"                         /* rand() */
#include "../hardware/hardware.h"           /* HW_ functions */
#include "../graphics/graphics.h"           /* G_ functions */
#include "../data/data.h"                   /* D_data */
#include "../engine/engine.h"               /* M_ functions */
#include "../light/light.h"                 /* L_init_lights */

struct M_polygon_object *obj;

unsigned char alp=0,bet=0,gam=0;            /* camera parameters */
int x=0,y=0,z=-200;

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

  case 'L': case 'l':      M_force_linear_tmapping=!M_force_linear_tmapping;
                           break;
 }
}

/**********************************************************/

void frame(void)                            /* called to display a frame */
{
 G_clear(0,0,0);                            /* clear the frame */
 M_init_camera(0,0,0,x,y,z,8);

 M_render_polygon_object(obj,0,0,0,alp,bet,gam);

 G_text(0,0,"<enter> <arrows,x,s,a,z> <p,w,f,g,t>",255,255,255);
 if(M_force_linear_tmapping) G_text(0,8,"Linear mapping",255,255,255);
 else G_text(0,8,"Perspective mapping",255,255,255);

 HW_blit();                                 /* display the image */
}

/**********************************************************/

void idle(void)                             /* called when no events */
{                                           /* to process */
 frame();
}

/**********************************************************/

int main(int no, char **o)                  
{
 obj=D_data("data/txtr_app.dat");

 HW_init_screen("3Dgpl3 - Texture",300,300);

 M_init_rendering(M_TEXTURED);              /* rendering option */
 M_init_polygon_object(obj);

 HW_init_event_loop(frame,handler,idle);    /* run event loop */
 HW_close_screen();

 return(1);
}

/**********************************************************/
