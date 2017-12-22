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
#include <string.h>

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


static void* volumeTimer();
static void* programInfoTimer();


/**
 * @brief  Inicijalizacija grafike.
 * 
 * @param  [in]   
 * @param  [out]  
 * @return tables 
 */
static void graphicInit();
/**
 * @brief  Deinicijalizacija grafike.
 * 
 * @param  [in]   
 * @param  [out]  
 * @return tables 
 */
static void graphicDeinit();

/**
 * @brief  Crtanje grafike za zvuk.
 * 
 * @param  [in]   
 * @param  [out]  
 * @return tables 
 */
static void drawVolumeState(uint8_t volumeState);
/**
 * @brief  Crtanje grafike za radio.
 * 
 * @param  [in]   
 * @param  [out]  
 * @return tables 
 */
static void drawRadio();
/**
 * @brief  Prikazivanje informacija za dati kanal.
 * 
 * @param  [in]   
 * @param  [out]  
 * @return tables 
 */
static void showProgramInfoBanner();

#endif /* __GRAFIKA_H__ */
