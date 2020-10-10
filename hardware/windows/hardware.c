/** 3Dgpl3 ************************************************\
 * Header for WINDOWS hardware specific functions.        *
 *                                                        *
 * Defines:                                               *
 *  HW_init_screen           Opening output surface;      *
 *  HW_pixel                 Pack and store;              *
 *  HW_blit                  Bitmap onto the screen;      *
 *  HW_close_screen          Closing output;              *
 *                                                        *
 *  HW_init_event_loop       Runing for events;           *
 *  HW_error                 Quiting with a message;      *
 *  HW_close_event_loop      Quiting running.             *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include "../hardware/hardware.h"           /* macros */
#include "../graphics/graphics.h"           /* G_c_buffer */
#include "../clipper/clipper.h"             /* C_init_clipping */
#include "../trans/trans.h"                 /* T_init_math */
#include <stdarg.h>                         /* var arg stuff */
#include <stdio.h>                          /* vsprintf */
#include <stdlib.h>                         /* exit */

/**********************************************************/

extern int main(int n,char **o);            /* real main (well...) */

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

HDC HW_mem;                                 /* memory device context */
HBITMAP HW_bmp;                             /* bitmap header */
RECT HW_rect;                               /* "client" area dimensions */
HWND HW_wnd;                                /* window */
HANDLE HW_instance;   
int HW_cmd_show;                            /* not sure why, but needed */
void (*HW_frame_function)(void)=NULL;
void (*HW_handler_function)(int key_code)=NULL;
void (*HW_idle_function)(void)=NULL;

/**********************************************************\
 * Creating a window.                                     *
\**********************************************************/

void HW_init_screen(char *window_title,int size_x,int size_y)
{
 PAINTSTRUCT ps;
 int i,remap;
 COLORREF cr,cr2;                           /* for determining */
 BYTE r,g,b;                                /* bits for each color */

 HW_screen_x_size=size_x;                   /* screen size */
 HW_screen_y_size=size_y;
 HW_image_size=HW_screen_x_size*HW_screen_y_size;

 C_init_clipping(0,0,HW_screen_x_size-1,HW_screen_y_size-1);
 T_init_math();

 HW_wnd=CreateWindow("3Dgpl3",window_title,WS_SYSMENU,
		     CW_USEDEFAULT,CW_USEDEFAULT,
		     HW_screen_x_size,
                     HW_screen_y_size+GetSystemMetrics(SM_CYCAPTION),
		     NULL,NULL,HW_instance,NULL);

 HW_mem=CreateCompatibleDC(BeginPaint(HW_wnd,&ps));
 if((GetDeviceCaps(ps.hdc,PLANES))!=1)
  HW_error("(Hardware) Direct RGB color expected.");

 HW_pixel_size=GetDeviceCaps(ps.hdc,BITSPIXEL)/8;
 if((HW_pixel_size!=2)&&(HW_pixel_size!=3)&&(HW_pixel_size!=4))
  HW_error("(Hardware) 16bpp, 24bpp or 32bpp expected.");

 G_init_graphics();

 HW_bmp=CreateCompatibleBitmap(ps.hdc,HW_screen_x_size,HW_screen_y_size);
 SelectObject(HW_mem,HW_bmp);

 cr2=0;                                     /* ugly way of doing something */
 HW_red_mask=HW_green_mask=HW_blue_mask=0;  /* trivial. Windows don't */
 for (i=1;i<256;i++)                        /* have any service to report */
 {                                          /* pixel bit layout */
  cr=SetPixel(HW_mem,0,0,RGB(i,i,i));
  if(GetRValue(cr)!=GetRValue(cr2)) HW_red_mask++;
  if(GetGValue(cr)!=GetGValue(cr2)) HW_green_mask++;
  if(GetBValue(cr)!=GetBValue(cr2)) HW_blue_mask++;       
  cr2=cr;                                   /* masks - which bits are masked */
 }                                          /* by every color */
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

 EndPaint(HW_wnd,&ps);

 HW_rect.left=HW_rect.top=0;
 HW_rect.right=HW_screen_x_size;
 HW_rect.bottom=HW_screen_y_size;

 ShowWindow(HW_wnd,HW_cmd_show);            /* generate messages */
 UpdateWindow(HW_wnd);
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
 * Rendering the bitmap into the window.                  *
\**********************************************************/

void HW_blit(void)
{
 PAINTSTRUCT ps;

 BeginPaint(HW_wnd,&ps);                    /* store into a bitmap */
 SetMapMode(ps.hdc,MM_TEXT);                /* blit a bitmap */
 SetBitmapBits(HW_bmp,HW_image_size*HW_pixel_size,(void*)G_c_buffer);
 BitBlt(ps.hdc,0,0,HW_screen_x_size,HW_screen_y_size,HW_mem,0,0,SRCCOPY);
 EndPaint(HW_wnd,&ps);
}

/**********************************************************\
 * Deallocating some stuff.                               *
\**********************************************************/

void HW_close_screen(void)
{
 DeleteDC(HW_mem);                          /* freeing resources */
 DeleteObject(HW_bmp);
}

/**********************************************************\
 * Running the event loop.                                *
\**********************************************************/

void HW_init_event_loop(void (*frame)(void),
                        void (*handler)(int key_code),
                        void (*idle)(void))
{
 MSG msg;

 HW_frame_function=frame;                   /* remebering which functions */
 HW_handler_function=handler;               /* to call */
 HW_idle_function=idle;

 HW_frame_function();                       /* draw first frame */
 InvalidateRect(HW_wnd,&HW_rect,TRUE);
 UpdateWindow(HW_wnd);

 while(1)                                   /* event loop */
 {
  if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))  /* get a message, don't block */
  {
   if(msg.message==WM_QUIT) break;          /* exit event received */
   TranslateMessage(&msg);
   DispatchMessage(&msg);                   /* message forwarded */
  }
  else
  {
   InvalidateRect(HW_wnd,&HW_rect,TRUE);    /* erase following by paint */
   UpdateWindow(HW_wnd);                    /* will be generated */
  }
 }
}

/**********************************************************\
 * Quiting with a message.                                *
\**********************************************************/

void HW_error(char *s,...)
{
 char str[256];
 va_list lst;

 va_start(lst,s);
 vsprintf(str,s,lst);                       /* forming the message */
 va_end(lst);

 MessageBox(NULL,str,"3Dgpl",MB_OK|MB_ICONSTOP|MB_SYSTEMMODAL);

 HW_close_event_loop();
 exit(0);                                   /* above might not be enough */
}

/**********************************************************\
 * Quitting the event loop.                               *
\**********************************************************/

void HW_close_event_loop(void)
{
 PostMessage(HW_wnd,WM_CLOSE,0,0L);         /* telling ourselves to quit */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Windows main callback function. It is being called    *
 * as a result of executing our event loop.              *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

long FAR PASCAL WndProc(HWND hWnd,UINT message,
			WPARAM wParam,LPARAM lParam)
{
 switch(message)
 {
  case WM_PAINT:      if(HW_frame_function!=NULL) HW_idle_function();
                      break;
  case WM_ERASEBKGND: return(1L);           /* don't erase background */
  case WM_DESTROY:    PostQuitMessage(0);
                      break;
  case WM_KEYDOWN:    if(HW_handler_function!=NULL) HW_handler_function(wParam);
                      if(HW_frame_function!=NULL) 
                      {
                       HW_frame_function();
                       InvalidateRect(HW_wnd,&HW_rect,TRUE);
                       UpdateWindow(HW_wnd);
                      }

                      break;
  default:            return(DefWindowProc(hWnd,message,wParam,lParam));
 }
 return(0L);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Windows Main function. It'll prepare a window and     *
 * call our main.                                        *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpszCmdLine,int nCmdShow)
{
 WNDCLASS w;
 int n;
 char *start,*end;                          /* to get parametrs */
 char *o[32];                               /* to pass parameters */

 HW_cmd_show=nCmdShow;
 if((HW_instance=hPrevInstance)==NULL)
 {
  w.style=CS_HREDRAW|CS_VREDRAW;
  w.lpfnWndProc=(LPVOID)WndProc;
  w.cbClsExtra=0;
  w.cbWndExtra=0;
  w.hInstance=hInstance;
  w.hIcon=NULL;
  w.hCursor=NULL;
  w.hbrBackground=GetStockObject(WHITE_BRUSH);
  w.lpszMenuName=NULL;
  w.lpszClassName="3Dgpl3";

  if(! RegisterClass(&w))
   return FALSE;
 }

 n=0;
 o[n++]="";
 start=lpszCmdLine;                         /* starting from here */
 while((end=strchr(start,' '))!=NULL)
 {
  if(n>=32) 
   HW_error("(Hardware) Way too many command line options.\n");
  if(end!=start) o[n++]=start;              /* ignore empty ones */
  *end=0;                                   /* end of line */
  start=end+1;
 }
 if(strlen(start)>0) o[n++]=start;          /* the very last one */

 return(main(n,o));                         /* here, humane way to start */
}

/**********************************************************/
