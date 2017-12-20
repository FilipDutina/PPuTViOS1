#ifndef __STREAM_CONTROLLER_H__
#define __STREAM_CONTROLLER_H__

#include <stdio.h>
#include "tables.h"
#include "tdp_api.h"
#include "tables.h"
#include "pthread.h"
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#define DESIRED_FREQUENCY 754000000	        /* Tune frequency in Hz */
#define BANDWIDTH 8    				        /* Bandwidth in Mhz */
#define CORECTION_FACTOR 160400000

/* po defaultu utisano (za sada) */
static uint8_t volumeState = 0;
static bool muteFlag = false;

/**
 * @brief Structure that defines stream controller error
 */
typedef enum _StreamControllerError
{
    SC_NO_ERROR = 0,
    SC_ERROR,
    SC_THREAD_ERROR
}StreamControllerError;

/**
 * @brief Structure that defines channel info
 */
typedef struct _ChannelInfo
{
    int16_t programNumber;
    int16_t audioPid;
    int16_t videoPid;
    bool txt;
}ChannelInfo;

/**
 * @brief Initializes stream controller module
 *
 * @return stream controller error code
 */
StreamControllerError streamControllerInit();

/**
 * @brief Deinitializes stream controller module
 *
 * @return stream controller error code
 */
StreamControllerError streamControllerDeinit();

/**
 * @brief Channel up
 *
 * @return stream controller error
 */
StreamControllerError channelUp();

/**
 * @brief Channel down
 *
 * @return stream controller error
 */
StreamControllerError channelDown();

/**
 * @brief Channel to specific channel number
 *
 * @return stream controller error
 */
StreamControllerError changeChannelTo(int16_t channelNumber);

/**
 * @brief Volume up
 *
 * @return stream controller error
 */
StreamControllerError changeVolumeUp();

/**
 * @brief Volume down
 *
 * @return stream controller error
 */
StreamControllerError changeVolumeDown();

/**
 * @brief Volume to mute
 *
 * @return stream controller error
 */
StreamControllerError changeVolumeToMute();

/**
 * @brief Parse config file
 *
 * @return stream controller error
 */
StreamControllerError parseConfigFile();

/**
 * @brief Checks if config file is valid
 *
 * @return stream controller error
 */
StreamControllerError configFileIsValid();

/**
 * @brief Returns current channel info
 *
 * @param [out] channelInfo - channel info structure with current channel info
 * @return stream controller error code
 */
StreamControllerError getChannelInfo(ChannelInfo* channelInfo);
 
StreamControllerError volumeUp();
 
StreamControllerError volumeDown();
 
StreamControllerError mute();

StreamControllerError info();

StreamControllerError getChannelInfo(ChannelInfo* channelInfo);

#endif /* __STREAM_CONTROLLER_H__ */
