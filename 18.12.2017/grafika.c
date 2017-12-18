#include "grafika.h"
#include "stream_controller.h"

static pthread_t renderLoopThread;
static pthread_mutex_t mutex;

static void* renderLoop();

static int32_t graphicInit = 0; 

static bool volumeInit = false;
static bool infoBannerInit = false; 

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void* renderLoop()
{
	while(graphicInit)
	{
		pthread_mutex_lock(&mutex);
		if(graphicInit == 0)
		{
			pthread_mutex_unlock(&mutex);
			return;
		}	
		pthread_mutex_unlock(&mutex);
		
		if(volumeInit)
		{
			pthread_mutex_lock(&mutex);
			drawVolumeState(volumeState);
			pthread_mutex_unlock(&mutex);
			printf("usao u volumeInit\n");
		}
		if(infoBannerInit)
		{
			pthread_mutex_lock(&mutex);
			showInfoBanner();
			pthread_mutex_unlock(&mutex);
			printf("usao u infoBannerInit\n");
		}
	}
}

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void init()
{
	DFBSurfaceDescription surfaceDesc;
    
    /* remote device name */
    static const char* dev = "/dev/input/event0";
    static int32_t inputFileDesc;
    char deviceName[20];
    struct input_event event;
    
    /* structure for timer specification */
    struct sigevent signalEvent;
    
    int32_t ret;
    

    /* initialize DirectFB */
    
	DFBCHECK(DirectFBInit(NULL, NULL));
	DFBCHECK(DirectFBCreate(&dfbInterface));
	DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));
	
    /* create primary surface with double buffering enabled */
    
	surfaceDesc.flags = DSDESC_CAPS;
	surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
	DFBCHECK (dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary));
    
    
    /* fetch the screen size */
    DFBCHECK (primary->GetSize(primary, &screenWidth, &screenHeight));
    
    
    /* clear the screen before drawing anything (draw black full screen rectangle)*/
    
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0xff));
	DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
    
    
    /* create timer */
    signalEvent.sigev_notify = SIGEV_THREAD; /* tell the OS to notify you about timer by calling the specified function */
    signalEvent.sigev_notify_function = wipeScreen; /* function to be called when timer runs out */
    signalEvent.sigev_value.sival_ptr = NULL; /* thread arguments */
    signalEvent.sigev_notify_attributes = NULL; /* thread attributes (e.g. thread stack size) - if NULL default attributes are applied */
    ret = timer_create(/*clock for time measuring*/CLOCK_REALTIME,
                       /*timer settings*/&signalEvent,
                       /*where to store the ID of the newly created timer*/&timerId);
    if(ret == -1){
        printf("Error creating timer, abort!\n");
        primary->Release(primary);
        dfbInterface->Release(dfbInterface);
    }
    
    
    /* open device for reading remote key events */
    
    inputFileDesc = open(dev,O_RDWR);
    ioctl(inputFileDesc, EVIOCGNAME(sizeof(deviceName)), deviceName);
	printf("RC device opened succesfully [%s]\n", deviceName);
	
	graphicInit = 1;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&renderLoopThread, NULL, renderLoop, NULL);
	printf("pozdrav iz inita!\n");
}

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void deInit()
{
	pthread_mutex_lock(&mutex);
	graphicInit = 0;
	pthread_mutex_unlock(&mutex);
	
	pthread_join(renderLoopThread, NULL);  
	
	/*Clean*/
	primary->Release(primary);
	dfbInterface->Release(dfbInterface);
	
	pthread_mutex_destroy(&mutex);
	printf("pozdrav iz deInita!\n");
}

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void wipeScreen(union sigval signalArg){
    int32_t ret;
	
	/* clear screen */
	DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
	DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
	
	/* update screen */
	DFBCHECK(primary->Flip(primary, NULL, 0));
	
	/* stop the timer */
	memset(&timerSpec,0,sizeof(timerSpec));
	ret = timer_settime(timerId,0,&timerSpec,&timerSpecOld);
	if(ret == -1){
	    printf("Error setting timer in %s!\n", __FUNCTION__);
	}
	printf("wipeScreen\n");
	
	if(currentChannel.videoPid == -1)
	{
		drawRadio();
	}
}


/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void drawRadio()
{
	static IDirectFBSurface *primary = NULL;
    IDirectFB *dfbInterface = NULL;
    static int screenWidth = 0;
    static int screenHeight = 0;
    DFBSurfaceDescription surfaceDesc;
   
    /* initialize DirectFB */
   
    DFBCHECK(DirectFBInit(NULL, NULL));
    /* fetch the DirectFB interface */
    DFBCHECK(DirectFBCreate(&dfbInterface));
    /* tell the DirectFB to take the full screen for this application */
    DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));
   
   
    /* create primary surface with double buffering enabled */
   
    surfaceDesc.flags = DSDESC_CAPS;
    surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFBCHECK (dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary));
    
     /* fetch the screen size */
    DFBCHECK (primary->GetSize(primary, &screenWidth, &screenHeight));
   
   
    /* clear the screen before drawing anything (draw black full screen rectangle)*/
   
    DFBCHECK(primary->SetColor(/*surface to draw on*/ primary,
                               /*red*/ 0x00,
                               /*green*/ 0x00,
                               /*blue*/ 0x00,
                               /*alpha*/ 0xff));	//alpha setovana na nulu
    DFBCHECK(primary->FillRectangle(/*surface to draw on*/ primary,
		                        /*upper left x coordinate*/ 0,
		                        /*upper left y coordinate*/ 0,
		                        /*rectangle width*/ screenWidth,
		                        /*rectangle height*/ screenHeight));
                                
  	/* draw text */
  	
  	DFBCHECK(primary->SetColor(/*surface to draw on*/ primary,
                               /*red*/ 0xff,
                               /*green*/ 0xff,
                               /*blue*/ 0xff,
                               /*alpha*/ 0x00));

    IDirectFBFont *fontInterface = NULL;
    DFBFontDescription fontDesc;
   
    /* specify the height of the font by raising the appropriate flag and setting the height value */
    fontDesc.flags = DFDESC_HEIGHT;
    fontDesc.height = 80;
   
    /* create the font and set the created font for primary surface text drawing */
    DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
    DFBCHECK(primary->SetFont(primary, fontInterface));
   
    /* draw the text */
    DFBCHECK(primary->DrawString(primary,
                                 /*text to be drawn*/ "RADIO FM",
                                 /*number of bytes in the string, -1 for NULL terminated strings*/ -1,
                                 /*x coordinate of the lower left corner of the resulting text*/ 100,
                                 /*y coordinate of the lower left corner of the resulting text*/ 100,
                                 /*in case of multiple lines, allign text to left*/ DSTF_LEFT));
                                 
	/* switch between the displayed and the work buffer (update the display) */
    DFBCHECK(primary->Flip(primary,
                           /*region to be updated, NULL for the whole surface*/NULL,
                           /*flip flags*/0));
                           
    /*clean up*/
   
    primary->Release(primary);
    dfbInterface->Release(dfbInterface);
}


/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/


void drawVolumeState(uint8_t volumeState)
{
	int32_t ret;
    int32_t i;
    IDirectFBFont *fontInterface = NULL;
    DFBFontDescription fontDesc;
    char keycodeString[4];
    static int screenWidth = 0;
    static int screenHeight = 0;
    DFBSurfaceDescription surfaceDesc;
    
    uint16_t j;
    
    /* initialize DirectFB */
   
    DFBCHECK(DirectFBInit(NULL, NULL));
    /* fetch the DirectFB interface */
    DFBCHECK(DirectFBCreate(&dfbInterface));
    /* tell the DirectFB to take the full screen for this application */
    DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));
   
   
    /* create primary surface with double buffering enabled */
   
    surfaceDesc.flags = DSDESC_CAPS;
    surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFBCHECK (dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary));
    
     /* fetch the screen size */
    DFBCHECK (primary->GetSize(primary, &screenWidth, &screenHeight));
                          
	/* draw image from file */
   
    IDirectFBImageProvider *provider;
    IDirectFBSurface *logoSurface = NULL;
    int32_t logoHeight, logoWidth;
   	printf("drawVolumeState 2\n");
    /* create the image provider for the specified file */
    
    switch(volumeState)
    {
    	case 0:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "mityzu.png", &provider));
			break;
		case 1:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_1.png", &provider));
			break;
		case 2:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_2.png", &provider));
			break;
		case 3:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_3.png", &provider));
			break;
		case 4:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_4.png", &provider));
			break;
		case 5:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_5.png", &provider));
			break;
		case 6:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_6.png", &provider));
			break;
		case 7:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_7.png", &provider));
			break;
		case 8:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_8.png", &provider));
			break;
		case 9:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_9.png", &provider));
			break;
		case 10:
			DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_10.png", &provider));
			break;
    	
    }
    
    /* get surface descriptor for the surface where the image will be rendered */
    DFBCHECK(provider->GetSurfaceDescription(provider, &surfaceDesc));
    /* create the surface for the image */
    DFBCHECK(dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &logoSurface));
    /* render the image to the surface */
    DFBCHECK(provider->RenderTo(provider, logoSurface, NULL));
   
    /* cleanup the provider after rendering the image to the surface */
    provider->Release(provider);
   
    /* fetch the logo size and add (blit) it to the screen */
    DFBCHECK(logoSurface->GetSize(logoSurface, &logoWidth, &logoHeight));
    DFBCHECK(primary->Blit(primary,
                           /*source surface*/ logoSurface,
                           /*source region, NULL to blit the whole surface*/ NULL,
                           /*destination x coordinate of the upper left corner of the image*/screenWidth - logoWidth - 50,
                           /*destination y coordinate of the upper left corner of the image*/20));
   
   	DFBCHECK(primary->Flip(primary, NULL, 0));
    
	printf("drawVolumeState 3\n");
    /* set the timer for clearing the screen */
    
    memset(&timerSpec,0,sizeof(timerSpec));
    
    /* specify the timer timeout time */
    timerSpec.it_value.tv_sec = 3;
    timerSpec.it_value.tv_nsec = 0;
    
    /* set the new timer specs */
    ret = timer_settime(timerId,0,&timerSpec,&timerSpecOld);
    if(ret == -1){
        printf("Error setting timer in %s!\n", __FUNCTION__);
    }
    printf("volumeFunction\n");
    
    volumeInit = false;
}

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void showInfoBanner()
{
	int32_t ret;
    int32_t i;
    IDirectFBFont *fontInterface = NULL;
    DFBFontDescription fontDesc;
    char keycodeString[4];
    static int screenWidth = 0;
    static int screenHeight = 0;
    DFBSurfaceDescription surfaceDesc;
    
    uint16_t j;
    
    /* initialize DirectFB */
   
    DFBCHECK(DirectFBInit(NULL, NULL));
    /* fetch the DirectFB interface */
    DFBCHECK(DirectFBCreate(&dfbInterface));
    /* tell the DirectFB to take the full screen for this application */
    DFBCHECK(dfbInterface->SetCooperativeLevel(dfbInterface, DFSCL_FULLSCREEN));
   
   
    /* create primary surface with double buffering enabled */
   
    surfaceDesc.flags = DSDESC_CAPS;
    surfaceDesc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFBCHECK (dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, &primary));
    
     /* fetch the screen size */
    DFBCHECK (primary->GetSize(primary, &screenWidth, &screenHeight));
   
   
	fontDesc.flags = DFDESC_HEIGHT;
	fontDesc.height = 30;
	
	DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
	DFBCHECK(primary->SetFont(primary, fontInterface));
	
	
	/* fade in efekat */
	for(j = 0x00; j < 0xff; j++)
	{
		DFBCHECK(primary->SetColor(primary, 0x33, 0x33, 0x33, j));
		DFBCHECK(primary->FillRectangle(primary, screenWidth/4, screenHeight - 160, screenWidth/2, 250));

		DFBCHECK(primary->SetColor(primary, 0xff, 0x80, 0x40, j));
	
	
		/* draw the string */
		sprintf(keycodeString,"%d", currentChannel.programNumber);
		DFBCHECK(primary->DrawString(primary,"PROGRAM NUMBER: ", -1, screenWidth/4 + 20,  screenHeight - 130, DSTF_LEFT));
		DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/4 + 400,  screenHeight - 130, DSTF_LEFT));
	
		sprintf(keycodeString,"%d", currentChannel.videoPid);
		DFBCHECK(primary->DrawString(primary,"VIDEO PID: ", -1, screenWidth/4 + 20,  screenHeight - 90, DSTF_LEFT));
		DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/4 + 400,  screenHeight - 90, DSTF_LEFT));
	
		sprintf(keycodeString,"%d", currentChannel.audioPid);
		DFBCHECK(primary->DrawString(primary,"AUDIO PID: ", -1, screenWidth/4 + 20,  screenHeight - 50, DSTF_LEFT));
		DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/4 + 400,  screenHeight - 50, DSTF_LEFT));
	
		/* DODATI INFO O TRENUTNOJ EMISIJI */
		DFBCHECK(primary->DrawString(primary,"TRENUTNA EMISIJA (treba uraditi...): ", -1, screenWidth/4 + 20,  screenHeight - 10, DSTF_LEFT));
		
		/* update screen */
   		DFBCHECK(primary->Flip(primary, NULL, 0));
   		
   		j += 6;
	}

    /* set the timer for clearing the screen */
    
    memset(&timerSpec,0,sizeof(timerSpec));
    
    /* specify the timer timeout time */
    timerSpec.it_value.tv_sec = 5;
    timerSpec.it_value.tv_nsec = 0;
    
    /* set the new timer specs */
    ret = timer_settime(timerId,0,&timerSpec,&timerSpecOld);
    if(ret == -1){
        printf("Error setting timer in %s!\n", __FUNCTION__);
    }
	

	printf("showInfoBanner\n");
	infoBannerInit = false; 
}
