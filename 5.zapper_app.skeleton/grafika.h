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

#include "stream_controller.h"

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

static ChannelInfo currentChannel;

static timer_t timerId;
static IDirectFBSurface *primary = NULL;
static IDirectFB *dfbInterface = NULL;
static int32_t screenWidth = 0;
static int32_t screenHeight = 0;

static struct itimerspec timerSpec;
static struct itimerspec timerSpecOld;

static void drawKeycode(int32_t keycode);
static void wipeScreen(union sigval signalArg);
static void drawChannelNumber(int32_t channelNumber);
static void drawVolumeState(uint8_t volumeState);
static void drawRadio();
static void showInfoBanner(uint16_t programNumber, uint16_t audioPid, uint16_t videoPid);



#endif /* __GRAFIKA_H__ */
