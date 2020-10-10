/** 3Dgpl3 ************************************************\
 * Sample application:                                    *
 *  Creates a window, draws random points.                *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "stdlib.h"                         /* rand() */
#include "../hardware/hardware.h"           /* HW_ functions */
#include "../graphics/graphics.h"           /* G_ functions */

/**********************************************************/

void handler(int key)                       /* event handler */
{
 HW_close_event_loop();                     /* terminates if any key is */
}                                           /* pressed */

/**********************************************************/

void frame(void)                            /* called to display a frame */
{
 int v[3]={0,0,0};

 v[0]=rand()%HW_screen_x_size;              /* coordinates for a random */
 v[1]=rand()%HW_screen_y_size;              /* point */

 G_point(v,rand()%255,rand()%255,rand()%255);
 G_text(0,0,"<enter>",255,255,255);

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
 HW_init_screen("3Dgpl3 - Window example",300,300);

 G_clear(0,0,0);                            /* clear the frame */

 HW_init_event_loop(frame,handler,idle);    /* run event loop */
 HW_close_screen();

 return(1);
}

/**********************************************************/
