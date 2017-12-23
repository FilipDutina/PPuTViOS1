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
#include <string.h>

#define DESIRED_FREQUENCY 754000000	        /* Tune frequency in Hz */
#define BANDWIDTH 8    				        /* Bandwidth in Mhz */
#define CORECTION_FACTOR 160400000

#define CHANNEL_SERVICE_ID 489


/* po defaultu utisano */
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

typedef void(*streamControllerErrorCallback)();

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
 * @brief Returns current channel info
 *
 * @param [out] channelInfo - channel info structure with current channel info
 * @return stream controller error code
 */
StreamControllerError getChannelInfo(ChannelInfo* channelInfo);

/**
 * @brief Volume up (graphics)
 *
 * @return stream controller error
 */
StreamControllerError volumeUp();
 
/**
 * @brief Volume down (graphics)
 *
 * @return stream controller error
 */
StreamControllerError volumeDown();

/**
 * @brief Volume mute (graphics)
 *
 * @return stream controller error
 */
StreamControllerError mute();

/**
 * @brief Show info banner (graphics)
 *
 * @return stream controller error
 */
StreamControllerError info();

/**
 * @brief Checks if config file is valid
 *
 * @return stream controller error
 */
StreamControllerError configFileIsValid();

/**
 * @brief Initializes stream controller module
 *
 * @return stream controller error code
 */
StreamControllerError registerStreamControllerErrorCallback(streamControllerErrorCallback streamControllerErrorCallback);

StreamControllerError unregisterStreamControllerErrorCallback(streamControllerErrorCallback streamControllerErrorCallback);

StreamControllerError checkPids(int32_t channelNumber);

#endif /* __STREAM_CONTROLLER_H__ */
