/**
 * @file
 * @brief     Read local and broker timezone information from file.
 * @details   This includes daylight savings start/end times, GMT offsets, and trading week start/end times.
 * 
 * @author    Friedhelm Duesterhoeft (Initial implementation)
 * @author    Morgan Doel (Ported broker timezone offsets to the F4 framework)
 * @author    Daniel Fernandez (Assisted with design and code styling)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#include "Precompiled.h"
#include "Broker-tz.h"

#define TIMEZONE_CONFIG_FILENAME "broker-tz.csv"
#define MAX_RECORDS              100
#define MAX_LINE_LENGTH          500

#if defined _WIN32 || defined _WIN64
#define strtok_r strtok_s
#endif

static TimezoneInfo timezoneRecords[MAX_RECORDS];

AsirikuyReturnCode getTimezoneInfo(const char* pName, TimezoneInfo** pTimezoneInfo)
{
  int i;

  if(pName == NULL)
  {
    fprintf(stderr, "[CRITICAL] getTimezoneInfo() failed. pName = NULL\n");
    return NULL_POINTER;
  }

  if(pTimezoneInfo == NULL)
  {
    fprintf(stderr, "[CRITICAL] getTimezoneInfo() failed. pTimezoneInfo = NULL\n");
    return NULL_POINTER;
  }
  
  for(i = 0; i < MAX_RECORDS; i++)
  {
    if(strcmp(pName, timezoneRecords[i].name) == 0)
    {
      fprintf(stderr, "[DEBUG] getTimezoneInfo() Found matching record for \"%s\". Standard GMT offset = %d, DST GMT offset = %d\n", pName, timezoneRecords[i].gmtOffsetStd, timezoneRecords[i].gmtOffsetDS);
      *pTimezoneInfo = &timezoneRecords[i];
      return SUCCESS;
    }
  }

  fprintf(stderr, "[ERROR] getTimezoneInfo() Failed to find timezone information for \"%s\"\n", pName);
  return UNKNOWN_TIMEZONE;
}

static void removeQuotes(char *pLine)
{
  const char nullTerminator = '\0';
  const char quote          = '"';
  char *temp;
  temp = pLine;

  if(pLine == NULL)
  {
    fprintf(stderr, "[CRITICAL] removeQuotes() failed. pLine = NULL\n");
    return;
  }

  while(*temp != nullTerminator)
  {
    if(*temp == quote)
    {
      while(*temp != nullTerminator)
      {
        *temp = *(temp + 1);
        if(*temp != nullTerminator)
        {
          temp++;
        }
      }
      temp = pLine;
    }
    temp++;
  }
}

static AsirikuyReturnCode parseLine(int recordsIndex, char* pLine)
{
  char *token;
  char *strtokSafe;

  if(pLine == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseLine() failed. pLine = NULL\n");
    return NULL_POINTER;
  }

  
  fprintf(stderr, "[DEBUG] parseLine() Parsing timezone config file.\n");
  removeQuotes(pLine);

  token = strtok_r(pLine, ";", &strtokSafe);
  strcpy(timezoneRecords[recordsIndex].name, token);

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].startMonth = atoi(token);
  if((timezoneRecords[recordsIndex].startMonth < 0) || (timezoneRecords[recordsIndex].startMonth > 12))
  {
    fprintf(stderr, "[ERROR] parseLine() Start Month = %s. Please use an integer from 1 to 12. 0 is also acceptable if there is no DST change.\n", token);
    return INVALID_CONFIG;
  }
  timezoneRecords[recordsIndex].startMonth -= 1; /* In timezone info file months start from 1. In C/C++ struct tm months start from 0. */

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].startNth = atoi(token);
  if((timezoneRecords[recordsIndex].startNth < 0) || (timezoneRecords[recordsIndex].startNth > 4))
  {
    fprintf(stderr, "[ERROR] parseLine() Start Nth = %s. Please use an integer from 0 to 4. 0 means the last specified weekday of the month.\n", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].startDay = atoi(token);
  if((timezoneRecords[recordsIndex].startDay < 0) || (timezoneRecords[recordsIndex].startDay > 6))
  {
    fprintf(stderr, "[ERROR] parseLine() Start Day = %s. Please use an integer from 0 to 6. 0 = Sunday.\n", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ":", &strtokSafe);
  timezoneRecords[recordsIndex].startHour = atoi(token);
  if((timezoneRecords[recordsIndex].startHour < 0) || (timezoneRecords[recordsIndex].startHour > 23))
  {
    fprintf(stderr, "[ERROR] parseLine() Start Hour = %s. Please use an integer from 0 to 23.", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ";", &strtokSafe);
  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].endMonth = atoi(token);
  if((timezoneRecords[recordsIndex].endMonth < 0) || (timezoneRecords[recordsIndex].endMonth > 12))
  {
    fprintf(stderr, "[ERROR] parseLine() End Month = %s. Please use an integer from 1 to 12. 0 is also acceptable if there is no DST change.", token);
    return INVALID_CONFIG;
  }
  timezoneRecords[recordsIndex].endMonth -= 1; /* In timezone info file months start from 1. In C/C++ struct tm months start from 0. */

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].endNth = atoi(token);
  if((timezoneRecords[recordsIndex].endNth < 0) || (timezoneRecords[recordsIndex].endNth > 4))
  {
    fprintf(stderr, "[ERROR] parseLine() End Nth = %s. Please use an integer from 0 to 4. 0 means the last specified weekday of the month.", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].endDay = atoi(token);
  if((timezoneRecords[recordsIndex].endDay < 0) || (timezoneRecords[recordsIndex].endDay > 6))
  {
    fprintf(stderr, "[ERROR] parseLine() End Day = %s. Please use an integer from 0 to 6. 0 = Sunday.", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ":", &strtokSafe);
  timezoneRecords[recordsIndex].endHour = atoi(token);
  if((timezoneRecords[recordsIndex].endHour < 0) || (timezoneRecords[recordsIndex].endHour > 23))
  {
    fprintf(stderr, "[ERROR] parseLine() End Hour = %s. Please use an integer from 0 to 23.", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ";", &strtokSafe);
  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].gmtOffsetStd = atoi(token);
  if((timezoneRecords[recordsIndex].gmtOffsetStd < -15) || (timezoneRecords[recordsIndex].gmtOffsetStd > +15))
  {
    fprintf(stderr, "[ERROR] parseLine() GMT Offset(normal) = %s. Please use an integer from -15 to 15.", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].gmtOffsetDS = atoi(token);
  if((timezoneRecords[recordsIndex].gmtOffsetDS < -15) || (timezoneRecords[recordsIndex].gmtOffsetDS > +15))
  {
    fprintf(stderr, "[ERROR] parseLine() GMT Offset(DST) = %s. Please use an integer from -15 to 15.", token);
    return INVALID_CONFIG;
  }

  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].weekStartDay = atoi(token);
  token = strtok_r(NULL, ":", &strtokSafe);
  timezoneRecords[recordsIndex].weekStartHour = atoi(token);
  token = strtok_r(NULL, ";", &strtokSafe);
  token = strtok_r(NULL, ";", &strtokSafe);
  timezoneRecords[recordsIndex].weekendDay = atoi(token);
  token = strtok_r(NULL, ":", &strtokSafe);
  timezoneRecords[recordsIndex].weekendHour = atoi(token);

  return SUCCESS;
}

AsirikuyReturnCode parseTimezoneConfig(const char* pFileName)
{
  char  line[MAX_LINE_LENGTH];
  int   recordsIndex = 0;
  FILE* fileHandle;

  if(pFileName == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseTimezoneConfig() failed. pFilePath = NULL\n");
    return NULL_POINTER;
  }

  fileHandle = fopen(pFileName, "r");

  if(fileHandle == NULL)
  {
    fprintf(stderr, "[CRITICAL] parseTimezoneConfig() failed. fileHandle = NULL");
    return NULL_POINTER;
  }

  /* The first line contains column labels. Skip it. */
  fgets(line, MAX_LINE_LENGTH, fileHandle);

  /* Parse all the records */
  while(fgets(line, MAX_LINE_LENGTH, fileHandle) != NULL)
  {
    line[strcspn(line, "\n")] = '\0';
    if (line[0] == '\0')
    {
      continue;
    }

    parseLine(recordsIndex, line);
    recordsIndex++;
  }

  return SUCCESS;
}