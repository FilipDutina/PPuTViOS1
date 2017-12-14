#ifndef __GRAFIKA_H__
#define __GRAFIKA_H__

#include <stdio.h>
#include <directfb.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#define FRAME_THICKNESS 10
#define FONT_HEIGHT 50
//#define EXIT_BUTTON_KEYCODE 102


/* helper macro for error checking */
#define DFBCHECK(x...)                                      \
{                                                           \
DFBResult err = x;                                          \
                                                            \
if (err != DFB_OK)                                          \
  {                                                         \
    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );  \
    DirectFBErrorFatal( #x, err );                          \
  }                                                         \
}

static timer_t timerId;
static IDirectFBSurface *primary = NULL;
static IDirectFB *dfbInterface = NULL;
static int32_t screenWidth = 0;
static int32_t screenHeight = 0;

static struct itimerspec timerSpec;
static struct itimerspec timerSpecOld;

static void drawKeycode(int32_t keycode);
static void wipeScreen(union sigval signalArg);
static void draw(int32_t channelNumber);
static void drawVolume(char sign);



#endif /* __GRAFIKA_H__ */
