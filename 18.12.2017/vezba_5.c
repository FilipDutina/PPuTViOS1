#include "remote_controller.h"
#include "stream_controller.h"
#include "grafika.c"

static inline void textColor(int32_t attr, int32_t fg, int32_t bg)
{
   char command[13];

   /* command is the control command to the terminal */
   sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
   printf("%s", command);
}

/* macro function for error checking */
#define ERRORCHECK(x)                                                       \
{                                                                           \
if (x != 0)                                                                 \
 {                                                                          \
    textColor(1,1,0);                                                       \
    printf(" Error!\n File: %s \t Line: <%d>\n", __FILE__, __LINE__);       \
    textColor(0,7,0);                                                       \
    return -1;                                                              \
 }                                                                          \
}

static void remoteControllerCallback(uint16_t code, uint16_t type, uint32_t value);
static pthread_cond_t deinitCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t deinitMutex = PTHREAD_MUTEX_INITIALIZER;
static ChannelInfo channelInfo;
static void errorCallback();

int main()
{
	/* initialize remote controller module */
    ERRORCHECK(remoteControllerInit());
    
    /* register remote controller callback */
    ERRORCHECK(registerRemoteControllerCallback(remoteControllerCallback));
    
    /* initialize stream controller module */
    ERRORCHECK(streamControllerInit());
    
    /* register stream controller error callback */
	ERRORCHECK(registerStreamControllerErrorCallback(errorCallback));

    /* wait for a EXIT remote controller key press event */
    pthread_mutex_lock(&deinitMutex);
	if (ETIMEDOUT == pthread_cond_wait(&deinitCond, &deinitMutex))
	{
		printf("\n%s : ERROR Lock timeout exceeded!\n", __FUNCTION__);
	}
	pthread_mutex_unlock(&deinitMutex);
	
    /* unregister remote controller callback */
    ERRORCHECK(unregisterRemoteControllerCallback(remoteControllerCallback));

    /* deinitialize remote controller module */
    ERRORCHECK(remoteControllerDeinit());

    /* deinitialize stream controller module */
    ERRORCHECK(streamControllerDeinit());
    
    /* unregister stream controller error callback */
	ERRORCHECK(unregisterStreamControllerErrorCallback(errorCallback));
  
    return 0;
}

void remoteControllerCallback(uint16_t code, uint16_t type, uint32_t value)
{
    switch(code)
	{
		case KEYCODE_INFO:
            printf("\nInfo pressed\n");          
            if (getChannelInfo(&channelInfo) == SC_NO_ERROR)
            {
                printf("\n********************* Channel info *********************\n");
                printf("Program number: %d\n", channelInfo.programNumber);
                printf("Audio pid: %d\n", channelInfo.audioPid);
                printf("Video pid: %d\n", channelInfo.videoPid);
                printf("**********************************************************\n");
                info(channelInfo.programNumber, channelInfo.audioPid, channelInfo.videoPid);
            }
			break;
			
		case KEYCODE_P_PLUS:
			printf("\nCH+ pressed\n");
            channelUp();
			break;
			
		case KEYCODE_P_MINUS:
		    printf("\nCH- pressed\n");
            channelDown();
			break;
			
		case KEYCODE_VOLUME_UP:
			printf("\nVOL+ pressed\n");
			printf("case volume up\n");
			volumeUp();
			changeVolumeUp();
			break;
			
		case KEYCODE_VOLUME_DOWN:
			printf("\nVOL- pressed\n");
			printf("case volume down\n");
			volumeDown();
			changeVolumeDown();
			break;
			
		case KEYCODE_MUTE:
			printf("\nMUTE pressed\n");
			mute();
			changeVolumeToMute();
			break;
			
		case KEYCODE_EXIT:
			printf("\nExit pressed\n");
            pthread_mutex_lock(&deinitMutex);
		    pthread_cond_signal(&deinitCond);
		    pthread_mutex_unlock(&deinitMutex);
			break;
			
		case KEYCODE_0:
	    	printf("\n0 pressed\n");
        	printf("PROGRAM NOT AVAILABLE!\n");
			break;

		case KEYCODE_1:
	    	printf("\n1 pressed\n");
        	changeChannelTo(1);
			break;

		case KEYCODE_2:
	    	printf("\n2 pressed\n");
        	changeChannelTo(2);
			break;

		case KEYCODE_3:
	    	printf("\n3 pressed\n");
        	changeChannelTo(3);
			break;

		case KEYCODE_4:
	    	printf("\n4 pressed\n");
        	changeChannelTo(4);
			break;

		case KEYCODE_5:
	    	printf("\n5 pressed\n");
        	changeChannelTo(5);
			break;

		case KEYCODE_6:
	    	printf("\n6 pressed\n");
        	changeChannelTo(6);
			break;

		case KEYCODE_7:
	    	printf("\n7 pressed\n");
        	changeChannelTo(7);
			break;

		case KEYCODE_8:
	    	printf("\n8 pressed\n");
        	printf("PROGRAM NOT AVAILABLE!\n");
			break;

		case KEYCODE_9:
	    	printf("\n9 pressed\n");
        	printf("PROGRAM NOT AVAILABLE!\n");
			break;
		
		default:
			printf("\nPress P+, P-, V+, V-, mute, info or exit! \n\n");
	}
}

void errorCallback()
{
	/* Signal condition if pids don't match */
   	pthread_mutex_lock(&deinitMutex);
	pthread_cond_signal(&deinitCond);
    pthread_mutex_unlock(&deinitMutex);
}
