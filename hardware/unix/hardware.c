/** 3Dgpl3 ************************************************\
 * Header for UNIX/X11 hardware specific functions.       *
 *                                                        *
 * Defines:                                               *
 *  HW_init_screen           Opening output surface;      *
 *  HW_pixel                 Pack and store;              *
 *  HW_blit                  Bitmap onto the screen;      *
 *  HW_close_screen          Closing output;              *
 *                                                        *
 *  HW_init_event_loop       Runiing for events;          *
 *  HW_error                 Quiting with a message;      *
 *  HW_close_event_loop      Quiting running.             *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* harware dependent functions */
#include "../graphics/graphics.h"           /* G_c_buffer */
#include "../clipper/clipper.h"             /* C_init_clipping */
#include "../trans/trans.h"                 /* T_init_math */
#include <X11/Xlib.h>                       /* most of X stuff */
#include <X11/Xutil.h>                      /* XSizeHints etc */
#include <stdarg.h>                         /* var args */
#include <stdio.h>                          /* stderr */
#include <stdlib.h>                         /* exit */

/**********************************************************/

int HW_screen_x_size;                       /* screen dimensions */
int HW_screen_y_size;
int HW_image_size;                          /* number of pixels */
int HW_pixel_size;                          /* in bytes */

int HW_red_size;                            /* how many bits is there */
int HW_green_size;                          /* occupied by each color */
int HW_blue_size;
int HW_red_mask;                            /* which bits are occupied */
int HW_green_mask;                          /* by color */
int HW_blue_mask;
int HW_red_shift;                           /* how colors are packed into */
int HW_green_shift;                         /* the bitmap */
int HW_blue_shift;

Display *HW_display;                        /* x server connection */
Visual *HW_visual;                          /* display's visual */
int HW_screen;                              /* deafualt screen */
GC HW_gc;                                   /* default graphical conext */
Window HW_window;                           /* window being created */
Window HW_rootw;                            /* the very root window */
XImage *HW_c_buffer;                        /* the thing containing bitmap */
int HW_running;                             /* event loop still running */

/**********************************************************\
 * Creating a window.                                     *
\**********************************************************/

void HW_init_screen(char* window_title,int size_x,int size_y)
{
 XTextProperty name;
 XSizeHints *size;
 int i;

 HW_screen_x_size=size_x;                   /* screen size */
 HW_screen_y_size=size_y;
 HW_image_size=HW_screen_x_size*HW_screen_y_size;

 C_init_clipping(0,0,HW_screen_x_size-1,HW_screen_y_size-1);
 T_init_math();

 if((HW_display=XOpenDisplay(NULL))==NULL)
  HW_error("(Hardware) Can't open specified display.\n");

 HW_screen=DefaultScreen(HW_display);
 HW_visual=DefaultVisual(HW_display,HW_screen);
 HW_rootw=RootWindow(HW_display,HW_screen);
 HW_gc=DefaultGC(HW_display,HW_screen);

 HW_window=XCreateSimpleWindow(HW_display,HW_rootw,0,0,
			       HW_screen_x_size,HW_screen_y_size,
			       CopyFromParent,CopyFromParent,CopyFromParent);

 XStringListToTextProperty((char**)&window_title,1,&name);
 size=XAllocSizeHints();                    /* window manager hints */
 size->flags=PMinSize | PMaxSize;
 size->min_width=size->max_width=HW_screen_x_size;
 size->min_height=size->max_height=HW_screen_y_size;
 XSetWMProperties(HW_display,HW_window,&name,&name,NULL,0,size,NULL,NULL);

 HW_pixel_size=DefaultDepth(HW_display,HW_screen)/8;
 if((HW_pixel_size!=2)&&(HW_pixel_size!=3)&&(HW_pixel_size!=4))
  HW_error("(Hardware) 16bpp, 24bpp or 32bpp expected.");

 G_init_graphics();

 HW_c_buffer=XCreateImage(HW_display,HW_visual,HW_pixel_size*8,ZPixmap,
                          0,(char*)G_c_buffer,HW_screen_x_size,
                          HW_screen_y_size,HW_pixel_size*8,
                          HW_screen_x_size*HW_pixel_size);

 HW_red_mask=HW_c_buffer->red_mask;
 HW_green_mask=HW_c_buffer->green_mask;
 HW_blue_mask=HW_c_buffer->blue_mask;

 while((HW_red_mask&0x1)==0) HW_red_mask>>=1;
 while((HW_green_mask&0x1)==0) HW_green_mask>>=1; 
 while((HW_blue_mask&0x1)==0) HW_blue_mask>>=1; 

 HW_red_size=HW_green_size=HW_blue_size=0;
 for(i=0;i<8;i++)                           /* finding how many bits */
 {
  if(HW_red_mask>>i) HW_red_size++;
  if(HW_green_mask>>i) HW_green_size++;
  if(HW_blue_mask>>i) HW_blue_size++;
 }
 HW_red_shift=HW_green_size+HW_blue_size;   /* finding how to pack colors */
 HW_green_shift=HW_blue_size;               /* into a pixel */
 HW_blue_shift=0;

 XMapWindow(HW_display,HW_window);
 XSync(HW_display,False);
 XSelectInput(HW_display,HW_window,KeyPressMask);
}

/**********************************************************\
 * Packing a pixel into a bitmap.                         *
\**********************************************************/

void HW_pixel(char* buf_address,int red,int green,int blue)
{                                           /* adjust and clamp */
 if((red>>=(8-HW_red_size))>HW_red_mask) red=HW_red_mask;
 if((green>>=(8-HW_green_size))>HW_green_mask) green=HW_green_mask; 
 if((blue>>=(8-HW_blue_size))>HW_blue_mask) blue=HW_blue_mask; 

 switch(HW_pixel_size)                      /* depending on screen depth */ 
 {                                          /* pack and store */
  case 2:                                   /* 16bpp */
   (*(short*)buf_address)=(red<<HW_red_shift)|(green<<HW_green_shift)|
                          (blue<<HW_blue_shift); 
   break;
  case 3:                                   /* 24bpp */
   *buf_address=blue; *(buf_address+1)=green; *(buf_address+2)=red;
   break;
  case 4:                                   /* 32bpp */
   (*(int*)buf_address)=(red<<HW_red_shift)|(green<<HW_green_shift)|
                        (blue<<HW_blue_shift);
   break;
 }
}

/**********************************************************\
 * Blitting a bitmap into the allocated window.           *
\**********************************************************/

void HW_blit(void)
{
 XPutImage(HW_display,HW_window,HW_gc,
           HW_c_buffer,0,0,0,0,HW_screen_x_size,HW_screen_y_size);
}

/**********************************************************\
 * Deallocating a window.                                 *
\**********************************************************/

void HW_close_screen(void)
{
 XCloseDisplay(HW_display);
}

/**********************************************************\
 * The main event loop.                                   *
\**********************************************************/

void HW_init_event_loop(void (*frame)(void),
                        void (*handler)(int key_code),
                        void (*idle)(void))
{
 XEvent report;                             /* beware putting XKeyEvent here */
 int key;

 HW_running=1;
 frame();
 while(HW_running==1)
 {
  if(XCheckWindowEvent(HW_display,HW_window,KeyPressMask,&report)==1)
  {
   key=XKeycodeToKeysym(HW_display,((XKeyEvent*)&report)->keycode,0);
   if(key) handler(key); 
  }
  frame();
 }
}

/**********************************************************\
 * Quitng with a message.                                 *
\**********************************************************/

void HW_error(char *s,...)
{
 va_list lst;

 va_start(lst,s);
 vfprintf(stderr,s,lst);                    /* printing a message */
 va_end(lst);
 HW_close_event_loop();                     /* quiting processing */
 exit(0);
}

/**********************************************************\
 * Quiting the event loop.                                *
\**********************************************************/

void HW_close_event_loop(void)
{
 HW_running=0;
}

/**********************************************************/
