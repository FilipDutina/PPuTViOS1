#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <stdio.h>
#include <directfb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>


/* settings for channel number banner */
#define FRAME_THICKNESS 10
#define FONT_HEIGHT 50


static void drawKeycode(int32_t keycode); 

static void wipeScreen(union sigval signalArg);

static void draw();


#endif /* __GRAPHICS_H__ */
