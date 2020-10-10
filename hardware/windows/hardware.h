#if !defined(_HARDWARE_H_)
#define _HARDWARE_H_

/** 3Dgpl3 ************************************************\
 * Header for WINDOWS hardware specific functions.        *
 *                                                        *
 * Files:                                                 *
 *  hardware.c              Hardware specific stuff.      *
 *                                                        *
 * 3Dgpl 3D graphics library Version 3.0                  *
 * Copyright (c) 1995-2000 Sergei Savchenko               *
 * Available under the terms of GNU Library General       *
 * Public License.                                        *
\**********************************************************/

#include <windows.h>                        /* MS-Windows API */

/**********************************************************/

#define main _main                          /* to accomodate windows */

#define HW_KEY_ARROW_LEFT  VK_LEFT
#define HW_KEY_ARROW_RIGHT VK_RIGHT
#define HW_KEY_ARROW_UP    VK_UP
#define HW_KEY_ARROW_DOWN  VK_DOWN
#define HW_KEY_PLUS        VK_ADD
#define HW_KEY_MINUS       VK_SUBTRACT
#define HW_KEY_ENTER       VK_RETURN
#define HW_KEY_SPACE       VK_SPACE
#define HW_KEY_TAB         VK_TAB           /* ids of some keys */

#define HW_MAX_SCREEN_SIZE 1024

extern int HW_screen_x_size;                /* screen dimensions */
extern int HW_screen_y_size;
extern int HW_image_size;                   /* number of pixels */
extern int HW_pixel_size;                   /* in bytes */

typedef char* HW_pixel_ptr;                 /* may not fit a char though */
typedef int HW_fixed;                       /* better be 32 bit machine */

/**********************************************************/

void HW_init_screen(char *window_title,int size_x,int size_y);
void HW_pixel(HW_pixel_ptr buf_address,int r,int g,int b);
void HW_blit(void);
void HW_close_screen(void);

void HW_init_event_loop(void (*frame)(void),
                        void (*handler)(int key_code),
                        void (*idle)(void));
void HW_error(char *s,...);
void HW_close_event_loop(void);

/**********************************************************/

#endif
