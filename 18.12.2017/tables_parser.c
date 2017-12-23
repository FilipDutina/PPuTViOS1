#include "tables.h"

/*

****************************************************************************************************************************************************************************************
PAT TABLE
****************************************************************************************************************************************************************************************

*/

ParseErrorCode parsePatHeader(const uint8_t* patHeaderBuffer, PatHeader* patHeader)
{    
    if(patHeaderBuffer==NULL || patHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    patHeader->tableId = (uint8_t)* patHeaderBuffer; 
    if (patHeader->tableId != 0x00)
    {
        printf("\n%s : ERROR it is not a PAT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    lower8Bits = (uint8_t)(*(patHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    patHeader->sectionSyntaxIndicator = lower8Bits & 0x01;

    higher8Bits = (uint8_t) (*(patHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patHeader->sectionLength = all16Bits & 0x0FFF;
    
    higher8Bits = (uint8_t) (*(patHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patHeader->transportStreamId = all16Bits & 0xFFFF;
    
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    patHeader->versionNumber = lower8Bits & 0x1F;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 5));
    patHeader->currentNextIndicator = lower8Bits & 0x01;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 6));
    patHeader->sectionNumber = lower8Bits & 0xFF;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 7));
    patHeader->lastSectionNumber = lower8Bits & 0xFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePatServiceInfo(const uint8_t* patServiceInfoBuffer, PatServiceInfo* patServiceInfo)
{
    if(patServiceInfoBuffer==NULL || patServiceInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    higher8Bits = (uint8_t) (*(patServiceInfoBuffer));
    lower8Bits = (uint8_t) (*(patServiceInfoBuffer + 1));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patServiceInfo->programNumber = all16Bits & 0xFFFF; 

    higher8Bits = (uint8_t) (*(patServiceInfoBuffer + 2));
    lower8Bits = (uint8_t) (*(patServiceInfoBuffer + 3));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patServiceInfo->pid = all16Bits & 0x1FFF;
    
    return TABLES_PARSE_OK;
}

ParseErrorCode parsePatTable(const uint8_t* patSectionBuffer, PatTable* patTable)
{
    uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(patSectionBuffer==NULL || patTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parsePatHeader(patSectionBuffer,&(patTable->patHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing PAT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 12 /*PAT header size*/ - 3 /*Not in section length*/;
    currentBufferPosition = (uint8_t *)(patSectionBuffer + 8); /* Position after last_section_number */
    patTable->serviceInfoCount = 0; /* Number of services info presented in PAT table */
    
    while(parsedLength < patTable->patHeader.sectionLength)
    {
        if(patTable->serviceInfoCount > TABLES_MAX_NUMBER_OF_PIDS_IN_PAT - 1)
        {
            printf("\n%s : ERROR there is not enough space in PAT structure for Service info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
        }
        
        if(parsePatServiceInfo(currentBufferPosition, &(patTable->patServiceInfoArray[patTable->serviceInfoCount])) == TABLES_PARSE_OK)
        {
            currentBufferPosition += 4; /* Size from program_number to pid */
            parsedLength += 4; /* Size from program_number to pid */
            patTable->serviceInfoCount ++;
        }    
    }
    
    return TABLES_PARSE_OK;
}

ParseErrorCode printPatTable(PatTable* patTable)
{
    uint8_t i=0;
    
    if(patTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************PAT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",patTable->patHeader.tableId);
    printf("section_length           |      %d\n",patTable->patHeader.sectionLength);
    printf("transport_stream_id      |      %d\n",patTable->patHeader.transportStreamId);
    printf("section_number           |      %d\n",patTable->patHeader.sectionNumber);
    printf("last_section_number      |      %d\n",patTable->patHeader.lastSectionNumber);
    
    for (i=0; i<patTable->serviceInfoCount;i++)
    {
        printf("-----------------------------------------\n");
        printf("program_number           |      %d\n",patTable->patServiceInfoArray[i].programNumber);
        printf("pid                      |      %d\n",patTable->patServiceInfoArray[i].pid); 
    }
    printf("\n********************PAT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}

/*

****************************************************************************************************************************************************************************************
PMT TABLE
****************************************************************************************************************************************************************************************

*/

ParseErrorCode parsePmtHeader(const uint8_t* pmtHeaderBuffer, PmtTableHeader* pmtHeader)
{

    if(pmtHeaderBuffer==NULL || pmtHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    pmtHeader->tableId = (uint8_t)* pmtHeaderBuffer; 
    if (pmtHeader->tableId != 0x02)
    {
        printf("\n%s : ERROR it is not a PMT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    pmtHeader->sectionSyntaxIndicator = lower8Bits & 0x01;
    
    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->sectionLength = all16Bits & 0x0FFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->programNumber = all16Bits & 0xFFFF;
    
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    pmtHeader->versionNumber = lower8Bits & 0x1F;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 5));
    pmtHeader->currentNextIndicator = lower8Bits & 0x01;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 6));
    pmtHeader->sectionNumber = lower8Bits & 0xFF;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 7));
    pmtHeader->lastSectionNumber = lower8Bits & 0xFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 8));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 9));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->pcrPid = all16Bits & 0xFFFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 10));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 11));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->programInfoLength = all16Bits & 0x0FFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePmtElementaryInfo(const uint8_t* pmtElementaryInfoBuffer, PmtElementaryInfo* pmtElementaryInfo)
{
    if(pmtElementaryInfoBuffer==NULL || pmtElementaryInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    // TODO: implement
    
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer));
    pmtElementaryInfo->streamType = lower8Bits; 


    higher8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 1));	//3(reserved) + 5(PID)
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 2));	//8(PID)
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtElementaryInfo->elementaryPid = all16Bits & 0x1FFF;

  	higher8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 3)); //4(reserved) + 4(ES_info_length)
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 4));  //8(ES_info_length)
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtElementaryInfo->esInfoLength = all16Bits & 0x0FFF;
    

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePmtTable(const uint8_t* pmtSectionBuffer, PmtTable* pmtTable)
{
    uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(pmtSectionBuffer==NULL || pmtTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parsePmtHeader(pmtSectionBuffer,&(pmtTable->pmtHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing PMT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 12 + pmtTable->pmtHeader.programInfoLength /*PMT header size*/ + 4 /*CRC size*/ - 3 /*Not in section length*/;
    currentBufferPosition = (uint8_t *)(pmtSectionBuffer + 12 + pmtTable->pmtHeader.programInfoLength); /* Position after last descriptor */
    pmtTable->elementaryInfoCount = 0; /* Number of elementary info presented in PMT table */
    
    while(parsedLength < pmtTable->pmtHeader.sectionLength)
    {
        if(pmtTable->elementaryInfoCount > TABLES_MAX_NUMBER_OF_ELEMENTARY_PID - 1)
        {
            printf("\n%s : ERROR there is not enough space in PMT structure for elementary info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
        }
        
        if(parsePmtElementaryInfo(currentBufferPosition, &(pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount])) == TABLES_PARSE_OK)
        {
            currentBufferPosition += 5 + pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount].esInfoLength; /* Size from stream type to elemntary info descriptor*/
            parsedLength += 5 + pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount].esInfoLength; /* Size from stream type to elementary info descriptor */
            pmtTable->elementaryInfoCount++;
        }    
    }

    return TABLES_PARSE_OK;
}

ParseErrorCode printPmtTable(PmtTable* pmtTable)
{
    uint8_t i=0;
    
    if(pmtTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************PMT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",pmtTable->pmtHeader.tableId);
    printf("section_length           |      %d\n",pmtTable->pmtHeader.sectionLength);
    printf("program_number           |      %d\n",pmtTable->pmtHeader.programNumber);
    printf("section_number           |      %d\n",pmtTable->pmtHeader.sectionNumber);
    printf("last_section_number      |      %d\n",pmtTable->pmtHeader.lastSectionNumber);
    printf("program_info_legth       |      %d\n",pmtTable->pmtHeader.programInfoLength);
    
    for (i=0; i<pmtTable->elementaryInfoCount;i++)
    {
        printf("-----------------------------------------\n");
        printf("stream_type              |      %d\n",pmtTable->pmtElementaryInfoArray[i].streamType);
        printf("elementary_pid           |      %d\n",pmtTable->pmtElementaryInfoArray[i].elementaryPid);
    }
    printf("\n********************PMT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}

/*

****************************************************************************************************************************************************************************************
EIT TABLE
****************************************************************************************************************************************************************************************

*/

ParseErrorCode parseEitHeader(const uint8_t* eitHeaderBuffer, EitTableHeader* eitHeader)
{
	if(eitHeaderBuffer == NULL || eitHeader == NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    eitHeader->tableId = (uint8_t)* eitHeaderBuffer; 
    
    if (eitHeader->tableId != 0x4e)
    {
        printf("\n%s : ERROR it is not a EIT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    higher8Bits = (uint8_t) (*(eitHeaderBuffer + 1));	//uzmem 1 + 1 + 2 + 4
    lower8Bits = (uint8_t) (*(eitHeaderBuffer + 2));	//ovde uzmem ostalih 8 bita
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);	//spojim u sesnaestobitnu vrednost
    eitHeader->sectionLength = all16Bits & 0x0FFF;	//uzimam 12 bita; tolika mi je velicina sectionLength-a
    
    higher8Bits = (uint8_t) (*(eitHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(eitHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitHeader->serviceId = all16Bits & 0xFFFF;
    

	return TABLES_PARSE_OK;
}

ParseErrorCode parseEitElementaryInfo(const uint8_t* eitElementaryInfoBuffer, EitElementaryInfo* eitElementaryInfo)
{
	if(eitElementaryInfoBuffer == NULL || eitElementaryInfo == NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

	uint16_t i;
	uint16_t j;
	uint16_t k;
	
	
    lower8Bits = (uint8_t) (*(eitElementaryInfoBuffer + 10));
    lower8Bits = lower8Bits >> 5;
    eitElementaryInfo->runningStatus = lower8Bits & 0x7;


	higher8Bits = (uint8_t) (*(eitElementaryInfoBuffer + 10));	
    lower8Bits = (uint8_t) (*(eitElementaryInfoBuffer + 11));	
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitElementaryInfo->descriptorsLoopLength = all16Bits & 0x0FFF;
 
    
    k = 0;
    
    if(eitElementaryInfo->runningStatus == 0x4)
    {
		while(k < eitElementaryInfo->descriptorsLoopLength)
		{
			eitElementaryInfo->descriptor.descriptorTag = *(eitElementaryInfoBuffer + 12 + k);
			eitElementaryInfo->descriptor.descriptorLength = *(eitElementaryInfoBuffer + 12 + 1 + k);
		
			if(eitElementaryInfo->descriptor.descriptorTag == 0x4d)
			{
				//duzina imena emisije
				eitElementaryInfo->descriptor.eventNameLength = *(eitElementaryInfoBuffer + 12 + 5 + k);
				//dobavljanje char-ova koji cine ime emisije
				for(i = 0; i < eitElementaryInfo->descriptor.eventNameLength; i++)
				{
					eitElementaryInfo->descriptor.eventNameChar[i] = (char)(*(eitElementaryInfoBuffer + 12 + 6 + i + k));
				}
				//zavrsavanje stringa
				eitElementaryInfo->descriptor.eventNameChar[eitElementaryInfo->descriptor.eventNameLength] = '\0';
		
				//duzina opisa emisije
				eitElementaryInfo->descriptor.descriptionLength = *(eitElementaryInfoBuffer + 12 + 6 + eitElementaryInfo->descriptor.eventNameLength + k);
				//dobavljanje char-ova koji cine opis emisije
				for(j = 0; j < eitElementaryInfo->descriptor.descriptionLength; j++)
				{
					eitElementaryInfo->descriptor.descriptionChar[j] = (char)(*(eitElementaryInfoBuffer + 12 + 6 + eitElementaryInfo->descriptor.eventNameLength + 1 + j + k));
				}
				//zavrsavanje stringa
				eitElementaryInfo->descriptor.descriptionChar[eitElementaryInfo->descriptor.descriptionLength] = '\0';
			}
		
		
			k += eitElementaryInfo->descriptor.descriptorLength + 2;
		}
	}
	return TABLES_PARSE_OK;
}

ParseErrorCode parseEitTable(const uint8_t* eitSectionBuffer, EitTable* eitTable)
{
	uint8_t* currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(eitSectionBuffer == NULL || eitTable == NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parseEitHeader(eitSectionBuffer,&(eitTable->eitTableHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing EIT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 14 /*EIT header size*/ + 4 /*CRC size*/ - 3 /*Not in section length*/;
	currentBufferPosition = (uint8_t*)(eitSectionBuffer + 14); /*Position after reserved_future_use*/
	eitTable->elementaryInfoCount = 0; /* Number of elementary info presented in EIT table */
	
	while(parsedLength < eitTable->eitTableHeader.sectionLength)
	{
		if(eitTable->elementaryInfoCount > TABLES_MAX_NUMBER_OF_ELEMENTARY_PID_IN_EIT - 1)
		{
			printf("\n%s : ERROR there is not enough space in EIT structure for elementary info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
		}
		if(parseEitElementaryInfo(currentBufferPosition, &(eitTable->eitElementaryInfoArray[eitTable->elementaryInfoCount])) == TABLES_PARSE_OK)
		{
			currentBufferPosition += 12 + eitTable->eitElementaryInfoArray[eitTable->elementaryInfoCount].descriptorsLoopLength; // Size from stream type to elemntary info descriptor
			parsedLength += 12 + eitTable->eitElementaryInfoArray[eitTable->elementaryInfoCount].descriptorsLoopLength; // Size from stream type to elementary info descriptor 
			eitTable->elementaryInfoCount++;
		}
	}
	
	return TABLES_PARSE_OK;
}

ParseErrorCode printEitTable(EitTable* eitTable)
{
	uint8_t i = 0;
    
    if(eitTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************EIT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",eitTable->eitTableHeader.tableId);
    printf("section_length           |      %d\n",eitTable->eitTableHeader.sectionLength);
    printf("service_id           |      %d\n",eitTable->eitTableHeader.serviceId);
    
   
    printf("-----------------------------------------\n");
    printf("NAME :          |      %s\n",eitTable->eitElementaryInfoArray[i].descriptor.eventNameChar);
    printf("NAME LENGTH :          |      %d\n",eitTable->eitElementaryInfoArray[i].descriptor.eventNameLength);
    printf("DESCRIPTION :           |      %s\n",eitTable->eitElementaryInfoArray[i].descriptor.descriptionChar);
    printf("DESCRIPTION LENGTH :           |      %d\n",eitTable->eitElementaryInfoArray[i].descriptor.descriptionLength);
    
	
    printf("\n********************EIT TABLE SECTION********************\n");
    
return TABLES_PARSE_OK;
}
