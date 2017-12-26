/****************************************************************************
*
* Univerzitet u Novom Sadu, Fakultet tehnickih nauka
* Katedra za Računarsku Tehniku i Računarske Komunikacije
*
* -----------------------------------------------------
* Ispitni zadatak iz predmeta:
*
* PROGRAMSKA PODRSKA U TELEVIZIJI I OBRADI SLIKE
* -----------------------------------------------------
* Aplikacija za TV prijemnik
* -----------------------------------------------------
*
* \file grafika.c
* \brief
* Modul u kome se nalaze funkcije za iscrtavanje grafickih elemenata na ekran.
* Decembar 2017
*
* @Author Filip Dutina
* \notes
*****************************************************************************/

#include "grafika.h"
#include "stream_controller.h"

/* volume timer */
static timer_t volumeTimerId;
static struct sigevent volumeSignalEvent;
static struct itimerspec volumeTimerSpec;
static struct itimerspec volumeTimerSpecOld;

/* program info timer */
static timer_t programInfoTimerId;
static struct sigevent programInfoSignalEvent;
static struct itimerspec programInfoTimerSpec;
static struct itimerspec programInfoTimerSpecOld;


static IDirectFBSurface *primary = NULL;
static IDirectFB *dfbInterface = NULL;
static IDirectFBFont *fontInterface = NULL;
static DFBFontDescription fontDesc;
static ChannelInfo currentChannel;
static IDirectFBImageProvider *provider;
static IDirectFBSurface *logo_surface[11];
static DFBSurfaceDescription surfaceDesc;
static DFBRegion audioBannerFlipRegion;
static DFBRegion programInfoBannerFlipRegion;

static pthread_t renderLoopThread;
static pthread_mutex_t fnpMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static void* renderLoop();

static bool volumeInit = false;
static bool infoBannerInit = false;
static bool radioInit = false;
static int32_t graphicInitialized = 0; 
static int32_t screenWidth = 0;
static int32_t screenHeight = 0;
static int32_t program_count = 7;


char ime[100];
char opis[100];


void graphicInit()
{

	volumeSignalEvent.sigev_notify = SIGEV_THREAD;
	volumeSignalEvent.sigev_notify_function = (void*)volumeTimer;
	volumeSignalEvent.sigev_value.sival_ptr = NULL;
	volumeSignalEvent.sigev_notify_attributes = NULL;	
	timer_create(/*sistemski sat za merenje vremena*/ CLOCK_REALTIME,                
             /*podešavanja timer-a*/ &volumeSignalEvent,                      
            /*mesto gde će se smestiti ID novog timer-a*/ &volumeTimerId);
	
	programInfoSignalEvent.sigev_notify = SIGEV_THREAD;
	programInfoSignalEvent.sigev_notify_function = (void*)programInfoTimer;
	programInfoSignalEvent.sigev_value.sival_ptr = NULL;
	programInfoSignalEvent.sigev_notify_attributes = NULL;	
	timer_create(/*sistemski sat za merenje vremena*/ CLOCK_REALTIME,                
             /*podešavanja timer-a*/ &programInfoSignalEvent,                      
            /*mesto gde će se smestiti ID novog timer-a*/ &programInfoTimerId);		


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
    
    /* create font */
    fontDesc.flags = DFDESC_HEIGHT;
	fontDesc.height = 30;
	
	DFBCHECK(dfbInterface->CreateFont(dfbInterface, "/home/galois/fonts/DejaVuSans.ttf", &fontDesc, &fontInterface));
	DFBCHECK(primary->SetFont(primary, fontInterface));
    
	uint8_t i;
	
	for(i = 0;i <= 10; i++)
	{
		char file_name[20];
		sprintf(file_name,"volume_%u.png",i);
		/* create the image provider for the specified file */
		DFBCHECK(dfbInterface->CreateImageProvider(dfbInterface, file_name, &provider));
		/* get surface descriptor for the surface where the image will be rendered */
		DFBCHECK(provider->GetSurfaceDescription(provider, &surfaceDesc));
		/* create the surface for the image */
		DFBCHECK(dfbInterface->CreateSurface(dfbInterface, &surfaceDesc, logo_surface+i));
		/* render the image to the surface */
		DFBCHECK(provider->RenderTo(provider, logo_surface[i], NULL));

		/* cleanup the provider after rendering the image to the surface */
		provider->Release(provider);
	}
	
	
	programInfoBannerFlipRegion.x1 = screenWidth/2 - 500;
	programInfoBannerFlipRegion.y1 = screenHeight/2 + 300;
	programInfoBannerFlipRegion.x2 = screenWidth/2 + 500;
	programInfoBannerFlipRegion.y2 = screenHeight/2 + 700;
	
	audioBannerFlipRegion.x1 = screenWidth - screenWidth/3;
	audioBannerFlipRegion.y1 = 0;
	audioBannerFlipRegion.x2 = screenWidth;
	audioBannerFlipRegion.y2 = screenHeight/5 + 20;
	
	graphicInitialized = 1;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&renderLoopThread, NULL, renderLoop, NULL);
}

void graphicDeinit()
{
	graphicInitialized = 0;
	
	pthread_join(renderLoopThread, NULL);  
	
	/*Clean*/
	timer_delete(volumeTimerId);
	timer_delete(programInfoTimerId);
	primary->Release(primary);
	dfbInterface->Release(dfbInterface);
	pthread_mutex_destroy(&mutex);

}


void* renderLoop()
{

	while(graphicInitialized)
	{
		pthread_mutex_lock(&mutex);
		    
		DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
		DFBCHECK(primary->FillRectangle(primary, 0, 0, screenWidth, screenHeight));
		
		pthread_mutex_unlock(&mutex);
		
		if(radioInit)
		{
			pthread_mutex_lock(&mutex);
	
			drawRadio();
			DFBCHECK(primary->Flip(primary, NULL, 0));
			
			printf("CRTAJ RADIO!\n");
		
			pthread_mutex_unlock(&mutex);
		}	
		
		if(volumeInit)
		{
			pthread_mutex_lock(&mutex);
	
			if(muteFlag)
			{
				drawVolumeState(0);
			}
			else
			{
				drawVolumeState(volumeState);
			}
			DFBCHECK(primary->Flip(primary, &audioBannerFlipRegion, 0));

			printf("CRTAJ VOLUME!\n");

			pthread_mutex_unlock(&mutex);
		}
		
		if(infoBannerInit)
		{
			pthread_mutex_lock(&mutex);

			if(!radioInit)
			{
				wipeScreen();
			}
	
			showProgramInfoBanner();
			DFBCHECK(primary->Flip(primary, &programInfoBannerFlipRegion, 0));
				
			printf("CRTAJ INFO BANNER!\n");
				
			pthread_mutex_unlock(&mutex); 
			
			radioInit = false;                           
		}
	}
}

void wipeScreen()
{
	DFBCHECK(primary->SetColor(/*surface to draw on*/ primary,
                               /*red*/ 0x00,
                               /*green*/ 0x00,
                               /*blue*/ 0x00,
                               /*alpha*/ 0x00));	//alpha setovana na nulu
    DFBCHECK(primary->FillRectangle(/*surface to draw on*/ primary,
		                        /*upper left x coordinate*/ 0,
		                        /*upper left y coordinate*/ 0,
		                        /*rectangle width*/ 200,
		                        /*rectangle height*/ 200));
		                       
	DFBCHECK(primary->Flip(primary, NULL, 0));
}


void drawRadio()
{
    /* clear the screen before drawing anything (draw black full screen rectangle)*/
    DFBCHECK(primary->SetColor(/*surface to draw on*/ primary,
                               /*red*/ 0x00,
                               /*green*/ 0x00,
                               /*blue*/ 0x00,
                               /*alpha*/ 0x00));	//alpha setovana na nulu
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
   
    /* draw the text */
    DFBCHECK(primary->DrawString(primary,
                                 /*text to be drawn*/ "RADIO FM",
                                 /*number of bytes in the string, -1 for NULL terminated strings*/ -1,
                                 /*x coordinate of the lower left corner of the resulting text*/ 100,
                                 /*y coordinate of the lower left corner of the resulting text*/ 100,
                                 /*in case of multiple lines, allign text to left*/ DSTF_LEFT));      
                    

}

void drawVolumeState(uint8_t volumeState)
{
	int32_t ret;

    DFBCHECK(primary->Blit(primary,
                           /*source surface*/ logo_surface[volumeState],
                           /*source region, NULL to blit the whole surface*/ NULL,
                           /*destination x coordinate of the upper left corner of the image*/screenWidth - 225,
                           /*destination y coordinate of the upper left corner of the image*/20));
   
    /* set the timer for clearing the screen */
    memset(&volumeTimerSpec,0,sizeof(volumeTimerSpec));
    
    /* specify the timer timeout time */
    volumeTimerSpec.it_value.tv_sec = 3;
    volumeTimerSpec.it_value.tv_nsec = 0;
    
    /* set the new timer specs */
    ret = timer_settime(volumeTimerId,0,&volumeTimerSpec,&volumeTimerSpecOld);
    if(ret == -1){
        printf("Error setting timer in %s!\n", __FUNCTION__);
    }
    
    volumeInit = false;
}

void showProgramInfoBanner()
{
	int32_t ret;
    char keycodeString[4];
    char keycodeString1[500];

	DFBCHECK(primary->SetColor(primary, 0x33, 0x33, 0x33, 0xff));
	DFBCHECK(primary->FillRectangle(primary, screenWidth/32 , screenHeight - 200, screenWidth, 350));
	//										gornji levi x, gornji levi y, sirina, visina

	DFBCHECK(primary->SetColor(primary, 0xff, 0x80, 0x40, 0xff));

	/* draw the string */
	sprintf(keycodeString,"%d", currentChannel.programNumber);
	DFBCHECK(primary->DrawString(primary,"PROGRAM NUMBER: ", -1, screenWidth/4 + 20,  screenHeight - 160, DSTF_LEFT));
	DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/4 + 400,  screenHeight - 160, DSTF_LEFT));

	sprintf(keycodeString,"%d", currentChannel.videoPid);
	DFBCHECK(primary->DrawString(primary,"VIDEO PID: ", -1, screenWidth/4 + 20,  screenHeight - 130, DSTF_LEFT));
	DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/4 + 400,  screenHeight - 130, DSTF_LEFT));

	sprintf(keycodeString,"%d", currentChannel.audioPid);
	DFBCHECK(primary->DrawString(primary,"AUDIO PID: ", -1, screenWidth/4 + 20,  screenHeight - 100, DSTF_LEFT));
	DFBCHECK(primary->DrawString(primary, keycodeString, -1, screenWidth/4 + 400,  screenHeight - 100, DSTF_LEFT));
	
	sprintf(keycodeString,"%d", currentChannel.audioPid);
	DFBCHECK(primary->DrawString(primary,"TELETEXT AVAILABLE: ", -1, screenWidth/4 + 20,  screenHeight - 70, DSTF_LEFT));
	
	if(currentChannel.txt)
	{
		DFBCHECK(primary->DrawString(primary, "YES", -1, screenWidth/4 + 400,  screenHeight - 70, DSTF_LEFT));
	}
	else
	{
		DFBCHECK(primary->DrawString(primary, "NO", -1, screenWidth/4 + 400,  screenHeight - 70, DSTF_LEFT));
	}
	
	if((currentChannel.programNumber >= 5) && (currentChannel.programNumber <= 7))
	{
		DFBCHECK(primary->DrawString(primary,"IME EMISIJE: ", -1, screenWidth/4 + 20,  screenHeight - 40, DSTF_LEFT));
		
		DFBCHECK(primary->DrawString(primary,"OPIS: ", -1, screenWidth/4 + 20,  screenHeight - 10, DSTF_LEFT));
	}
	else
	{
		sprintf(keycodeString1,"%s", ime);
		DFBCHECK(primary->DrawString(primary,"IME EMISIJE: ", -1, screenWidth/4 + 20,  screenHeight - 40, DSTF_LEFT));
		DFBCHECK(primary->DrawString(primary, keycodeString1, -1, screenWidth/4 + 400,  screenHeight - 40, DSTF_LEFT));
	
		sprintf(keycodeString1, "%s", opis);
		DFBCHECK(primary->DrawString(primary,"OPIS: ", -1, screenWidth/4 + 20,  screenHeight - 10, DSTF_LEFT));
		DFBCHECK(primary->DrawString(primary, keycodeString1, -1, screenWidth/4 + 400,  screenHeight - 10, DSTF_LEFT));
	}
	
		
    /* set the timer for clearing the screen */
    memset(&programInfoTimerSpec,0,sizeof(programInfoTimerSpec));
    
    /* specify the timer timeout time */
    programInfoTimerSpec.it_value.tv_sec = 5;
    programInfoTimerSpec.it_value.tv_nsec = 0;
    
    /* set the new timer specs */
    ret = timer_settime(programInfoTimerId,0,&programInfoTimerSpec,&programInfoTimerSpecOld);
    if(ret == -1){
        printf("Error setting timer in %s!\n", __FUNCTION__);
    }
    
    infoBannerInit = false;
}

static void* programInfoTimer()
{
	pthread_mutex_lock(&mutex);
		DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
		DFBCHECK(primary->FillRectangle(primary, screenWidth/4, screenHeight - 160, screenWidth/2, 320));
		DFBCHECK(primary->Flip(primary, &programInfoBannerFlipRegion, 0));
	pthread_mutex_unlock(&mutex);

	return (void*)0;
}


static void* volumeTimer()
{
	pthread_mutex_lock(&mutex);
		DFBCHECK(primary->SetColor(primary, 0x00, 0x00, 0x00, 0x00));
		DFBCHECK(primary->FillRectangle(primary, screenWidth/4, screenHeight - 160, screenWidth/2, 320));
		DFBCHECK(primary->Flip(primary, &audioBannerFlipRegion, 0));
	pthread_mutex_unlock(&mutex);
	
	return (void*)0;
}
