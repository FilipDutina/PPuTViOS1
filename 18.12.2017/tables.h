#ifndef __TABLES_H__
#define __TABLES_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TABLES_MAX_NUMBER_OF_PIDS_IN_PAT    20 	    /* Max number of PMT pids in one PAT table */
#define TABLES_MAX_NUMBER_OF_ELEMENTARY_PID 20       /* Max number of elementary pids in one PMT table */
#define TABLES_MAX_NUMBER_OF_ELEMENTARY_PID_IN_EIT 20       /* Max number of elementary pids in one EIT table */

/**
 * @brief Enumeration of possible tables parser error codes
 */
typedef enum _ParseErrorCode
{
    TABLES_PARSE_ERROR = 0,                         /* TABLES_PARSE_ERROR */
	TABLES_PARSE_OK = 1                             /* TABLES_PARSE_OK */
}ParseErrorCode;

/**
 * @brief Structure that defines PAT Table Header
 */
typedef struct _PatHeader
{
    uint8_t     tableId;                            /* The type of table */
    uint8_t     sectionSyntaxIndicator;             /* The format of the table section to follow */
    uint16_t    sectionLength;                      /* The length of the table section beyond this field */
    uint16_t    transportStreamId;                  /* Transport stream identifier */
    uint8_t     versionNumber;                      /* The version number the private table section */
    uint8_t     currentNextIndicator;               /* Signals what a particular table will look like when it next changes */
    uint8_t     sectionNumber;                      /* Section number */
    uint8_t     lastSectionNumber;                  /* Signals the last section that is valid for a particular MPEG-2 private table */
}PatHeader;

/**
 * @brief Structure that defines PAT service info
 */
typedef struct _PatServiceInfo
{    
    uint16_t    programNumber;                      /* Identifies each service present in a transport stream */
    uint16_t    pid;                                /* Pid of Program Map table section or pid of Network Information Table  */
}PatServiceInfo;

/**
 * @brief Structure that defines PAT table
 */
typedef struct _PatTable
{    
    PatHeader patHeader;                                                     /* PAT Table Header */
    PatServiceInfo patServiceInfoArray[TABLES_MAX_NUMBER_OF_PIDS_IN_PAT];    /* Services info presented in PAT table */
    uint8_t serviceInfoCount;                                                /* Number of services info presented in PAT table */
}PatTable;

/**
 * @brief Structure that defines PMT table header
 */
typedef struct _PmtTableHeader
{
    uint8_t tableId;
    uint8_t sectionSyntaxIndicator;
    uint16_t sectionLength;
    uint16_t programNumber;
    uint8_t versionNumber;
    uint8_t currentNextIndicator;
    uint8_t sectionNumber;
    uint8_t lastSectionNumber;
    uint16_t pcrPid;
    uint16_t programInfoLength;
}PmtTableHeader;

/**
 * @brief Structure that defines PMT elementary info
 */
typedef struct _PmtElementaryInfo
{
    uint8_t streamType;
    uint16_t elementaryPid;
    uint16_t esInfoLength;
}PmtElementaryInfo;

/**
 * @brief Structure that defines PMT table
 */
typedef struct _PmtTable
{
    PmtTableHeader pmtHeader;
    PmtElementaryInfo pmtElementaryInfoArray[TABLES_MAX_NUMBER_OF_ELEMENTARY_PID];
    uint8_t elementaryInfoCount;
}PmtTable;

/**
 * @brief Structure that defines EIT table header
 */
typedef struct _EitTableHeader
{
	uint8_t tableId;
	uint16_t sectionLength;
	uint16_t serviceId;
}EitTableHeader;

/**
 * @brief Structure that describes EIT elementary info
 */
typedef struct _EitDescriptor
{
	uint8_t descriptorTag;
	uint8_t descriptorLength;
	uint8_t eventNameLength;
	char eventNameChar[1000];
	uint8_t descriptionLength;
	char descriptionChar[1000];
}EitDescriptor;

/**
 * @brief Structure that defines EIT elementary info
 */
typedef struct _EitElementaryInfo
{
	uint8_t runningStatus;
	uint16_t descriptorsLoopLength;
	EitDescriptor descriptor;
}EitElementaryInfo;

/**
 * @brief Structure that defines EIT table
 */
typedef struct _EitTable
{
	EitTableHeader eitTableHeader;
	EitElementaryInfo eitElementaryInfoArray[TABLES_MAX_NUMBER_OF_ELEMENTARY_PID_IN_EIT];
	uint8_t elementaryInfoCount;
}EitTable;

/**
 * @brief  Parse PAT header.
 * 
 * @param  [in]   patHeaderBuffer Buffer that contains PAT header
 * @param  [out]  patHeader PAT header
 * @return tables error code
 */
static ParseErrorCode parsePatHeader(const uint8_t* patHeaderBuffer, PatHeader* patHeader);

/**
 * @brief  Parse PAT Service information.
 * 
 * @param  [in]   patServiceInfoBuffer Buffer that contains PAT Service info
 * @param  [out]  descriptor PAT Service info
 * @return tables error code
 */
static ParseErrorCode parsePatServiceInfo(const uint8_t* patServiceInfoBuffer, PatServiceInfo* patServiceInfo);

/**
 * @brief  Parse PAT Table.
 * 
 * @param  [in]   patSectionBuffer Buffer that contains PAT table section
 * @param  [out]  patTable PAT Table
 * @return tables error code
 */
static ParseErrorCode parsePatTable(const uint8_t* patSectionBuffer, PatTable* patTable);

/**
 * @brief  Print PAT Table
 * 
 * @param  [in]   patTable PAT table to be printed
 * @return tables error code
 */
static ParseErrorCode printPatTable(PatTable* patTable);

/**
 * @brief Parse pmt table
 *
 * @param [in]  pmtHeaderBuffer Buffer that contains PMT header
 * @param [out] pmtHeader PMT table header
 * @return tables error code
 */
static ParseErrorCode parsePmtHeader(const uint8_t* pmtHeaderBuffer, PmtTableHeader* pmtHeader);

/**
 * @brief Parse PMT elementary info
 *
 * @param [in]  pmtElementaryInfoBuffer Buffer that contains pmt elementary info
 * @param [out] PMT elementary info
 * @return tables error code
 */
static ParseErrorCode parsePmtElementaryInfo(const uint8_t* pmtElementaryInfoBuffer, PmtElementaryInfo* pmtElementaryInfo);

/**
 * @brief Parse PMT table
 *
 * @param [in]  pmtSectionBuffer Buffer that contains pmt table section
 * @param [out] pmtTable PMT table
 * @return tables error code
 */
static ParseErrorCode parsePmtTable(const uint8_t* pmtSectionBuffer, PmtTable* pmtTable);

/**
 * @brief Print PMT table
 *
 * @param [in] pmtTable PMT table
 * @return tables error code
 */
static ParseErrorCode printPmtTable(PmtTable* pmtTable);

/**
 * @brief Parse eit table
 *
 * @param [in]  eitHeaderBuffer Buffer that contains EIT header
 * @param [out] eitHeader eit table header
 * @return tables error code
 */
static ParseErrorCode parseEitHeader(const uint8_t* eitHeaderBuffer, EitTableHeader* eitHeader);

/**
 * @brief Parse EIT elementary info
 *
 * @param [in]  eitElementaryInfoBuffer Buffer that contains eit elementary info
 * @param [out] EIT elementary info
 * @return tables error code
 */
static ParseErrorCode parseEitElementaryInfo(const uint8_t* eitElementaryInfoBuffer, EitElementaryInfo* eitElementaryInfo);

/**
 * @brief Parse EIT table
 *
 * @param [in]  eitSectionBuffer Buffer that contains eit table section
 * @param [out] eitTable EIT table
 * @return tables error code
 */
static ParseErrorCode parseEitTable(const uint8_t* eitSectionBuffer, EitTable* eitTable);

/**
 * @brief Print EIT table
 *
 * @param [in] eitTable EIT table
 * @return tables error code
 */
static ParseErrorCode printEitTable(EitTable* eitTable);

#endif /* __TABLES_H__ */
