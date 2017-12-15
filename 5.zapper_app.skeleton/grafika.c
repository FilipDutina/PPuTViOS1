#include "grafika.h"

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void drawKeycode(int32_t keycode)
{
    int32_t ret;
    int32_t i;
    IDirectFBFont *fontInterface = NULL;
    DFBFontDescription fontDesc;
    char keycodeString[4];
    
    
    /* clear the buffer before drawing */
    
    DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
    DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
    
    
     /* draw keycode */
    
	fontDesc.flags = DFDESC_HEIGHT;
	fontDesc.height = FONT_HEIGHT;
	
	DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
	DFBCHECK(primary->SetFont(primary, fontInterface));
	
	/* generate keycode string */
	sprintf(keycodeString,"%d",keycode);
    
    
    /*  draw the frame */
    
    for(i = 0x00; i < 0xff; i++)
    {
    	DFBCHECK(primary->SetColor(primary, 0x40, 0x10, 0x80, i));
		DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth/10, screenHeight/10));
		
		DFBCHECK(primary->SetColor(primary, 0x80, 0x40, 0x10, i));
		DFBCHECK(primary->FillRectangle(primary, FRAME_THICKNESS, FRAME_THICKNESS, screenWidth/10-2*FRAME_THICKNESS, screenHeight/10-2*FRAME_THICKNESS));
		
		/* draw the string */
		DFBCHECK(primary->SetColor(primary, 0x10, 0x80, 0x40, i));
		DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/20, screenHeight/15+FONT_HEIGHT/20, DSTF_CENTER));
		
		
		DFBCHECK(primary->Flip(primary, NULL, 0));
		
		i += 8;
    }
	
	 //DFBCHECK(primary->SetColor(/*surface to draw on*/ primary,
                              // /*red*/ 0x00,
                              // /*green*/ 0x00,
                              // /*blue*/ 0x00,
                              // /*alpha*/ 0xff));
    //DFBCHECK(primary->FillRectangle(/*surface to draw on*/ primary,
                               // /*upper left x coordinate*/ 0,
                              //  /*upper left y coordinate*/ 0,
                               // /*rectangle width*/ screenWidth,
                               // /*rectangle height*/ screenHeight));
	
	
    
    
    /* update screen */
    DFBCHECK(primary->Flip(primary, NULL, 0));
    
    
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
    printf("drawKeycode\n");
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
   
   
    /* wait 5 seconds before terminating*/
    //sleep(5);
   
   
    /*clean up*/
   
    primary->Release(primary);
    dfbInterface->Release(dfbInterface);
}


/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void drawChannelNumber(int32_t channelNumber)
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
    
    
    /* set the function for clearing screen */
    
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
    
    //do{
     //   ret = read(inputFileDesc,&event,(size_t)sizeof(struct input_event));
        
        /*  only react if we've read something and only on "press down" */
       // if(ret > 0){
           // if(event.value == 1){
                /* draw the keycode */
                drawKeycode(channelNumber);
           // }
        //}
   // }while(event.code != EXIT_BUTTON_KEYCODE);
	
    
    /* clean up */
    /*timer_delete(timerId);
	primary->Release(primary);
	dfbInterface->Release(dfbInterface);
	printf("draw\n");*/
}

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void drawVolumeState(uint8_t volumeState)
{
	static IDirectFBSurface *primary = NULL;
    IDirectFB *dfbInterface = NULL;
    static int screenWidth = 0;
    static int screenHeight = 0;
    DFBSurfaceDescription surfaceDesc;
    
    int32_t ret;
    int32_t i;
   
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
   
    
		                        
		                        
	/* draw image from file */
   
    IDirectFBImageProvider *provider;
    IDirectFBSurface *logoSurface = NULL;
    int32_t logoHeight, logoWidth;
   
    /* create the image provider for the specified file */
    if(volumeState == 0)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "mityzu.png", &provider));
    }
    else if(volumeState == 1)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_1.png", &provider));
    }
    else if(volumeState == 2)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_2.png", &provider));
    }
    else if(volumeState == 3)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_3.png", &provider));
    }
    else if(volumeState == 4)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_4.png", &provider));
    }
    else if(volumeState == 5)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_5.png", &provider));
    }
    else if(volumeState == 6)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_6.png", &provider));
    }
    else if(volumeState == 7)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_7.png", &provider));
    }
    else if(volumeState == 8)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_8.png", &provider));
    }
    else if(volumeState == 9)
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_9.png", &provider));
    }
    else
    {
    	DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, "volume_10.png", &provider));
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
   
   
    /* switch between the displayed and the work buffer (update the display) */
    DFBCHECK(primary->Flip(primary,
                           /*region to be updated, NULL for the whole surface*/NULL,
                           /*flip flags*/0));

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
}

/*

****************************************************************************************************************************************************************************************
****************************************************************************************************************************************************************************************

*/

void showInfoBanner(uint16_t programNumber, uint16_t audioPid, uint16_t videoPid)
{
	printf("showInfoBanner\n");
}
