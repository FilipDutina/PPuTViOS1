#include "stream_controller.h"
#include "grafika.c"
#include "tables_parser.c"

static PatTable *patTable;
static PmtTable *pmtTable;
static EitTable *eitTable;

static pthread_cond_t statusCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t statusMutex = PTHREAD_MUTEX_INITIALIZER;

static int32_t sectionReceivedCallback(uint8_t *buffer);
static int32_t tunerStatusCallback(t_LockStatus status);

static uint32_t playerHandle = 0;
static uint32_t sourceHandle = 0;
static uint32_t streamHandleA = 0;
static uint32_t streamHandleV = 0;
static uint32_t filterHandle = 0;
static uint8_t threadExit = 0;
static bool changeChannel = false;
static int16_t programNumber = 0;
static ChannelInfo currentChannel;
static bool isInitialized = false;
static uint32_t volumeNumber = 0;
static bool muteIsPressed = false;
static bool initialPowerOn = true;

static struct timespec lockStatusWaitTime;
static struct timeval now;
static pthread_t scThread;
static pthread_cond_t demuxCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t demuxMutex = PTHREAD_MUTEX_INITIALIZER;

static void* streamControllerTask();
static void startChannel(int32_t channelNumber);

static uint32_t desiredFrequency = 0;
static int8_t bandwidth = 0;
static int16_t  audioPid = 0;
static int16_t  videoPid = 0;
char module[8];
char audioType[25];
char videoType[20];


StreamControllerError streamControllerInit()
{
	parseConfigFile();
	
	if(configFileIsValid())
	{
		printf("Config file is not valid!\n");
		return SC_ERROR;
	}
	
	graphicInit();
	
    if (pthread_create(&scThread, NULL, &streamControllerTask, NULL))
    {
        printf("Error creating input event task!\n");
        return SC_THREAD_ERROR;
    }
    
    return SC_NO_ERROR;
}

StreamControllerError streamControllerDeinit()
{
	graphicDeinit();
    if (!isInitialized) 
    {
        printf("\n%s : ERROR streamControllerDeinit() fail, module is not initialized!\n", __FUNCTION__);
        return SC_ERROR;
    }
    
    threadExit = 1;
    if (pthread_join(scThread, NULL))
    {
        printf("\n%s : ERROR pthread_join fail!\n", __FUNCTION__);
        return SC_THREAD_ERROR;
    }
  
    
    /* free demux filter */  
    Demux_Free_Filter(playerHandle, filterHandle);

	/* remove audio stream */
	Player_Stream_Remove(playerHandle, sourceHandle, streamHandleA);
    
    /* remove video stream */
    Player_Stream_Remove(playerHandle, sourceHandle, streamHandleV);
    
    /* close player source */
    Player_Source_Close(playerHandle, sourceHandle);
    
    /* deinitialize player */
    Player_Deinit(playerHandle);
    
    /* deinitialize tuner device */
    Tuner_Deinit();
    
    /* free allocated memory */  
    free(patTable);
    free(pmtTable);
    free(eitTable);
    
    /* set isInitialized flag */
    isInitialized = false;
    

    return SC_NO_ERROR;
}

StreamControllerError channelUp()
{   
    if (programNumber >= patTable->serviceInfoCount - 1)
    {
        programNumber = 1;
    } 
    else
    {
        programNumber++;
    }

    /* set flag to start current channel */
    changeChannel = true;
    
    //infoBannerInit = true;

    return SC_NO_ERROR;
}

StreamControllerError channelDown()
{
    if (programNumber <= 1)
    {
        programNumber = patTable->serviceInfoCount - 1;
    } 
    else
    {
        programNumber--;
    }
   
    /* set flag to start current channel */
    changeChannel = true;
    
    //infoBannerInit = true;

    return SC_NO_ERROR;
}

StreamControllerError changeChannelTo(int16_t channelNumber)
{
    programNumber = channelNumber;
 
    /* set flag to start current channel */
    changeChannel = true;

    return SC_NO_ERROR;
}

StreamControllerError changeVolumeDown()
{
	if(volumeNumber < 1)
	{
		volumeNumber = 0;	
	}
	else
		volumeNumber--;

	muteIsPressed = false;
	
	Player_Volume_Set(playerHandle, volumeNumber*CORECTION_FACTOR);

    return SC_NO_ERROR;
}

StreamControllerError changeVolumeUp()
{
	volumeNumber++;
	
	if(volumeNumber > 10)
	{
		volumeNumber = 10;		
	}

	muteIsPressed = false;
	
	Player_Volume_Set(playerHandle, volumeNumber*CORECTION_FACTOR);

	return SC_NO_ERROR;
}

StreamControllerError volumeUp()
{
	if(volumeState < 10)
	{
		volumeState++;
		muteFlag = false;
	}
	
	//poziv funkcije za crtanje 
	/*funkciji za crtanje kao parametar saljes volumeState i na osnovu toga ce znati koju sliku da iscrta*/
	//printf("stream controller volumeUp\n");
	//drawVolumeState(volumeState);
	volumeInit = true;
		
	return SC_NO_ERROR;
}

StreamControllerError volumeDown()
{
	if(volumeState > 0)
	{
		volumeState--;
		muteFlag = false;
	}
	
	//pozviv funkcije za crtanje
	/*funkciji za crtanje kao parametar saljes volumeState i na osnovu toga ce znati koju sliku da iscrta*/
	//printf("stream controller volumeDown\n");
	//drawVolumeState(volumeState);
	volumeInit = true;
	
	return SC_NO_ERROR;
}

StreamControllerError changeVolumeToMute()
{	
	if(muteIsPressed)
	{	
		muteIsPressed = false;
		Player_Volume_Set(playerHandle, volumeNumber*CORECTION_FACTOR);	
	}
	else
	{
		muteIsPressed = true;
		Player_Volume_Set(playerHandle, 0);
	}	

	return SC_NO_ERROR;
}

StreamControllerError mute()
{
	if(muteFlag)
	{
		volumeInit = true;
		muteFlag = false;
	}
	else
	{
		
		volumeInit = true;
		muteFlag = true;
	}
	
	
	return SC_NO_ERROR;
}

StreamControllerError info()
{
	//showInfoBanner();
	infoBannerInit = true;
}

StreamControllerError getChannelInfo(ChannelInfo* channelInfo)
{
    if (channelInfo == NULL)
    {
        printf("\n Error wrong parameter\n", __FUNCTION__);
        return SC_ERROR;
    }
    
    channelInfo->programNumber = currentChannel.programNumber;
    channelInfo->audioPid = currentChannel.audioPid;
    channelInfo->videoPid = currentChannel.videoPid;
    
    return SC_NO_ERROR;
}

/* Sets filter to receive current channel PMT table
 * Parses current channel PMT table when it arrives
 * Creates streams with current channel audio and video pids
 */
void startChannel(int32_t channelNumber)
{
	printf("pocetak startChannel-a\n");

    /* free PAT table filter */
    Demux_Free_Filter(playerHandle, filterHandle);
    
    /* set demux filter for receive PMT table of program */
    if(Demux_Set_Filter(playerHandle, patTable->patServiceInfoArray[channelNumber].pid, 0x02, &filterHandle))
	{
		printf("\n%s : ERROR Demux_Set_Filter() fail\n", __FUNCTION__);
        return;
	}
    
    /* wait for a PMT table to be parsed*/
    pthread_mutex_lock(&demuxMutex);
	if (ETIMEDOUT == pthread_cond_wait(&demuxCond, &demuxMutex))
	{
		printf("\n%s : ERROR Lock timeout exceeded!\n", __FUNCTION__);
        streamControllerDeinit();
	}
	pthread_mutex_unlock(&demuxMutex);
	printf("pmt\n");
	
	printf("channelNumber: %d\n", channelNumber);
	if(channelNumber < 5)
	{
		printf("eit1\n");
		 /* free EIT table filter */
		Demux_Free_Filter(playerHandle, filterHandle);
		
		/* set demux filter for receive EIT table of program */
		if(Demux_Set_Filter(playerHandle, 0x12, 0x4e, &filterHandle))
		{
			printf("\n%s : ERROR Demux_Set_Filter() fail\n", __FUNCTION__);
		    return;
		}
		printf("eit2\n");
		/* wait for a EIT table to be parsed*/
		pthread_mutex_lock(&demuxMutex);
		if (ETIMEDOUT == pthread_cond_wait(&demuxCond, &demuxMutex))
		{
			printf("\n%s : ERROR Lock timeout exceeded!\n", __FUNCTION__);
		    streamControllerDeinit();
		}
		pthread_mutex_unlock(&demuxMutex);
		printf("eit3\n");
	}
    
    /* get audio and video pids */
    int16_t audioPid = -1;
    int16_t videoPid = -1;
    uint8_t i = 0;
    bool txt = false;
    for (i = 0; i < pmtTable->elementaryInfoCount; i++)
    {
        if (((pmtTable->pmtElementaryInfoArray[i].streamType == 0x1) || (pmtTable->pmtElementaryInfoArray[i].streamType == 0x2) || (pmtTable->pmtElementaryInfoArray[i].streamType == 0x1b))
            && (videoPid == -1))
        {
            videoPid = pmtTable->pmtElementaryInfoArray[i].elementaryPid;
        } 
        else if (((pmtTable->pmtElementaryInfoArray[i].streamType == 0x3) || (pmtTable->pmtElementaryInfoArray[i].streamType == 0x4))
            && (audioPid == -1))
        {
            audioPid = pmtTable->pmtElementaryInfoArray[i].elementaryPid;
        }
        if(pmtTable->pmtElementaryInfoArray[i].streamType == 0x6)
        {
        	txt = true;
        }
    }


    if (videoPid != -1) 
    {
        /* remove previous video stream */
        if (streamHandleV != 0)
        {
		    Player_Stream_Remove(playerHandle, sourceHandle, streamHandleV);
            streamHandleV = 0;
        }

        /* create video stream */
        if(Player_Stream_Create(playerHandle, sourceHandle, videoPid, VIDEO_TYPE_MPEG2, &streamHandleV))
        {
            printf("\n%s : ERROR Cannot create video stream\n", __FUNCTION__);
            streamControllerDeinit();
        }
    }
    else
    {
    	Player_Stream_Remove(playerHandle, sourceHandle, streamHandleV);
        streamHandleV = 0;
        MV_PE_ClearScreen(playerHandle, 1);
        radioInit = true;
        printf("inicijalizacija radija\n");
    }

    if (audioPid != -1)
    {   
        /* remove previos audio stream */
        if (streamHandleA != 0)
        {
            Player_Stream_Remove(playerHandle, sourceHandle, streamHandleA);
            streamHandleA = 0;
        }

	    /* create audio stream */
        if(Player_Stream_Create(playerHandle, sourceHandle, audioPid, AUDIO_TYPE_MPEG_AUDIO, &streamHandleA))
        {
            printf("\n%s : ERROR Cannot create audio stream\n", __FUNCTION__);
            streamControllerDeinit();
        }
    }
    
    /* store current channel info */
    currentChannel.programNumber = channelNumber;
    currentChannel.audioPid = audioPid;
    currentChannel.videoPid = videoPid;
    currentChannel.txt = txt;
    
    infoBannerInit = true;
    printf("usao u start channel\n");
}

void* streamControllerTask()
{
    gettimeofday(&now,NULL);
    lockStatusWaitTime.tv_sec = now.tv_sec+10;

    /* allocate memory for PAT table section */
    patTable=(PatTable*)malloc(sizeof(PatTable));
    if(patTable==NULL)
    {
		printf("\n%s : ERROR Cannot allocate memory\n", __FUNCTION__);
        return (void*) SC_ERROR;
	}  
    memset(patTable, 0x0, sizeof(PatTable));

    /* allocate memory for PMT table section */
    pmtTable=(PmtTable*)malloc(sizeof(PmtTable));
    if(pmtTable==NULL)
    {
		printf("\n%s : ERROR Cannot allocate memory\n", __FUNCTION__);
        return (void*) SC_ERROR;
	}  
    memset(pmtTable, 0x0, sizeof(PmtTable));
    
    /* allocate memory for EIT table section */
    eitTable=(EitTable*)malloc(sizeof(EitTable));
    if(eitTable==NULL)
    {
		printf("\n%s : ERROR Cannot allocate memory for EIT\n", __FUNCTION__);
        return (void*) SC_ERROR;
	}  
    memset(eitTable, 0x0, sizeof(EitTable));
       
    /* initialize tuner device */
    if(Tuner_Init())
    {
        printf("\n%s : ERROR Tuner_Init() fail\n", __FUNCTION__);
        free(patTable);
        free(pmtTable);
        free(eitTable);
        return (void*) SC_ERROR;
    }
    
    /* register tuner status callback */
    if(Tuner_Register_Status_Callback(tunerStatusCallback))
    {
		printf("\n%s : ERROR Tuner_Register_Status_Callback() fail\n", __FUNCTION__);
	}
    
    /* lock to frequency */
   	if(!Tuner_Lock_To_Frequency(desiredFrequency, bandwidth, DVB_T))
    {
        printf("\n%s: INFO Tuner_Lock_To_Frequency(): %d Hz - success!\n",__FUNCTION__,DESIRED_FREQUENCY);
    }
    else
    {
        printf("\n%s: ERROR Tuner_Lock_To_Frequency(): %d Hz - fail!\n",__FUNCTION__,DESIRED_FREQUENCY);
        free(patTable);
        free(pmtTable);
        free(eitTable);
        Tuner_Deinit();
        return (void*) SC_ERROR;
    }
    
    /* wait for tuner to lock */
    pthread_mutex_lock(&statusMutex);
    if(ETIMEDOUT == pthread_cond_timedwait(&statusCondition, &statusMutex, &lockStatusWaitTime))
    {
        printf("\n%s : ERROR Lock timeout exceeded!\n",__FUNCTION__);
        free(patTable);
        free(pmtTable);
        free(eitTable);
        Tuner_Deinit();
        return (void*) SC_ERROR;
    }
    pthread_mutex_unlock(&statusMutex);
   
    /* initialize player */
    if(Player_Init(&playerHandle))
    {
		printf("\n%s : ERROR Player_Init() fail\n", __FUNCTION__);
		free(patTable);
        free(pmtTable);
        free(eitTable);
        Tuner_Deinit();
        return (void*) SC_ERROR;
	}
	
	/* open source */
	if(Player_Source_Open(playerHandle, &sourceHandle))
    {
		printf("\n%s : ERROR Player_Source_Open() fail\n", __FUNCTION__);
		free(patTable);
        free(pmtTable);
        free(eitTable);
		Player_Deinit(playerHandle);
        Tuner_Deinit();
        return (void*) SC_ERROR;	
	}

	/* set PAT pid and tableID to demultiplexer */
	if(Demux_Set_Filter(playerHandle, 0x00, 0x00, &filterHandle))
	{
		printf("\n%s : ERROR Demux_Set_Filter() fail\n", __FUNCTION__);
	}
	
	/* register section filter callback */
    if(Demux_Register_Section_Filter_Callback(sectionReceivedCallback))
    {
		printf("\n%s : ERROR Demux_Register_Section_Filter_Callback() fail\n", __FUNCTION__);
	}

    pthread_mutex_lock(&demuxMutex);
	if (ETIMEDOUT == pthread_cond_wait(&demuxCond, &demuxMutex))
	{
		printf("\n%s:ERROR Lock timeout exceeded!\n", __FUNCTION__);
        free(patTable);
        free(pmtTable);
        free(eitTable);
		Player_Deinit(playerHandle);
        Tuner_Deinit();
        return (void*) SC_ERROR;
	}
	pthread_mutex_unlock(&demuxMutex);
    
    
    
    
    /* start current channel */
    startChannel(programNumber);
    
    /* set isInitialized flag */
    isInitialized = true;

    while(!threadExit)
    {
        if (changeChannel)
        {
            changeChannel = false;
            startChannel(programNumber);
        }
    }
}

int32_t sectionReceivedCallback(uint8_t *buffer)
{
    uint8_t tableId = *buffer;  
    if(tableId==0x00)
    {
        //printf("\n%s -----PAT TABLE ARRIVED-----\n",__FUNCTION__);
        
        if(parsePatTable(buffer,patTable)==TABLES_PARSE_OK)
        {
            //printPatTable(patTable);
            pthread_mutex_lock(&demuxMutex);
		    pthread_cond_signal(&demuxCond);
		    pthread_mutex_unlock(&demuxMutex);
            
        }
    } 
    else if (tableId==0x02)
    {
        //printf("\n%s -----PMT TABLE ARRIVED-----\n",__FUNCTION__);
        
        if(parsePmtTable(buffer,pmtTable)==TABLES_PARSE_OK)
        {
            //printPmtTable(pmtTable);
            pthread_mutex_lock(&demuxMutex);
		    pthread_cond_signal(&demuxCond);
		    pthread_mutex_unlock(&demuxMutex);
        }
    }
    else if (tableId == 0x4e)
    {
		memset(eitTable, 0x0, sizeof(EitTable));
    	if(parseEitTable(buffer, eitTable)==TABLES_PARSE_OK)
        {
        	if(eitTable->eitTableHeader.serviceId == pmtTable->pmtHeader.programNumber && eitTable->eitElementaryInfoArray[0].runningStatus == 0x4/*(programNumber + CHANNEL_SERVICE_ID)*/)	/* + 490 */
        	{
        		/*sta mi treba da bih ispisao ime i opis emisije????*/
				strcpy(ime, eitTable->eitElementaryInfoArray[0].descriptor.eventNameChar);
				strcpy(opis, eitTable->eitElementaryInfoArray[0].descriptor.descriptionChar);
				
				printf("ovo1111 gledaj: %s\n", eitTable->eitElementaryInfoArray[0].descriptor.eventNameChar);
				printf("ovo2222 gledaj: %s\n", eitTable->eitElementaryInfoArray[0].descriptor.descriptionChar);
        	
        		printf("\n%s -----EIT TABLE ARRIVED-----\n",__FUNCTION__);
		        printEitTable(eitTable);
		        pthread_mutex_lock(&demuxMutex);
				pthread_cond_signal(&demuxCond);
				pthread_mutex_unlock(&demuxMutex);
		    }
        }
    
    }
    return 0;
}

int32_t tunerStatusCallback(t_LockStatus status)
{
    if(status == STATUS_LOCKED)
    {
        pthread_mutex_lock(&statusMutex);
        pthread_cond_signal(&statusCondition);
        pthread_mutex_unlock(&statusMutex);
        printf("\n%s -----TUNER LOCKED-----\n",__FUNCTION__);
    }
    else
    {
        printf("\n%s -----TUNER NOT LOCKED-----\n",__FUNCTION__);
    }
    return 0;
}

StreamControllerError parseConfigFile()
{
	FILE* file;
	
	file = fopen("config.txt","r");

	while (!feof(file))
	{
		if (fscanf(file,"DESIRED_FREQUENCY:%d\nBANDWIDTH:%d\nMODULE:%s\nAUDIO PID:%d\nVIDEO PID:%d\nAUDIO TYPE:%s\nVIDEO TYPE:%s\nPROGRAM NUMBER:%d ",&desiredFrequency, &bandwidth, module, &audioPid, &videoPid, audioType, videoType, &programNumber))
		{
			printf("Config file successfully parsed!\n");
			break;
		}
		else
		{
			printf("Check config file!\n");
			return SC_ERROR;
		}
	}

	fclose(file);

	//printf("%d\n%d\n%s\n%d\n%d\n%s\n%s\n%d\n",desiredFrequency, bandwidth, module, audioPid, videoPid, audioType, videoType, programNumber);	

	return SC_NO_ERROR;
}

StreamControllerError configFileIsValid()
{
	if(desiredFrequency != (uint32_t) 754000000)
	{
		printf("Check frequency\n");
		return SC_ERROR;
	}
	if(bandwidth != (uint8_t) 8)
	{
		printf("Check bandwidth\n");
		return SC_ERROR;
	}
	if(strcmp("DVB_T", module) != 0)
	{	
		printf("Check module\n");
		return SC_ERROR;
	}
	if(strcmp("VIDEO_TYPE_MPEG2", audioType) != 0)
	{
		printf("Check audio type\n");
		return SC_ERROR;
	}
	if(strcmp("AUDIO_TYPE_MPEG_AUDIO", videoType) != 0)
	{
		printf("Check video type\n");
		return SC_ERROR;
	}	
	
	if(programNumber<=0 || programNumber >7)
	{
		printf("Program number not in range! [1 - 7]\n");
		return SC_ERROR;
	}
	if(programNumber == 1 && audioPid != 103 && videoPid != 101)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
	if(programNumber == 2 && audioPid != 203 && videoPid != 201)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
	if(programNumber == 3 && audioPid != 1003 && videoPid != 1001)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
	if(programNumber == 4 && audioPid != 1503 && videoPid != 1501)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
	if(programNumber == 5 && audioPid != 2001 && videoPid != -1)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
	if(programNumber == 6 && audioPid != 2011 && videoPid != -1)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
	if(programNumber == 7 && audioPid != 2021 && videoPid != -1)
	{
		printf("Audio or video pid don't match with program number\n");
		return SC_ERROR;
	}
		
	return SC_NO_ERROR;
}
