/**
 * @file
 * @brief     Handles storage and retrieval of timezone offsets.
 * @details   The local, broker, and reference offsets are stored
 * @details   for each day of the year. Separate sets of offsets are
 * @details   kept for each trading instance.
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
#include "TimeZoneOffsets.h"
#include "Broker-tz.h"

#include <AsirikuyTime.h>
#include <Logging.h>
#include <CriticalSection.h>
#include <NTPCWrapper.hpp>
#include <Timezones.hpp>

static BOOL isMatchingTime(const time_t time1, const time_t time2)
{
  /* Less than half an hour difference is acceptable. */
  return (abs((int)time1 - (int)time2) < (SECONDS_PER_HOUR / 2));
}

static AsirikuyReturnCode checkTZValidity(time_t brokerTime, time_t referenceTime, TZOffsets* timeOffsets)
{
  char timeString[MAX_TIME_STRING_SIZE];
  time_t localTimeUTC       = time(NULL);
  time_t adjustedLocalTime  = getAdjustedLocalTime(localTimeUTC, timeOffsets);
  time_t adjustedBrokerTime = getAdjustedBrokerTime(brokerTime, timeOffsets);
  
  if(!isMatchingTime(adjustedLocalTime, referenceTime))
  {
    fprintf(stderr, "[ERROR] checkTZValidity() Local Timezone mismatch");
    fprintf(stderr, "[ERROR] checkTZValidity() Local time(UTC)      = %s", safe_timeString(timeString, localTimeUTC));
    fprintf(stderr, "[ERROR] checkTZValidity() broker time          = %s", safe_timeString(timeString, brokerTime));
    fprintf(stderr, "[ERROR] checkTZValidity() reference time       = %s", safe_timeString(timeString, referenceTime));
    fprintf(stderr, "[ERROR] checkTZValidity() adjusted local time  = %s", safe_timeString(timeString, adjustedLocalTime));
    fprintf(stderr, "[ERROR] checkTZValidity() adjusted broker time = %s", safe_timeString(timeString, adjustedBrokerTime));
    return LOCAL_TZ_MISMATCH;
  }
  else if(!isMatchingTime(adjustedBrokerTime, referenceTime))
  {
    fprintf(stderr, "[ERROR] checkTZValidity() Broker Timezone mismatch");
    fprintf(stderr, "[ERROR] checkTZValidity() Local time(UTC)      = %s", safe_timeString(timeString, localTimeUTC));
    fprintf(stderr, "[ERROR] checkTZValidity() broker time          = %s", safe_timeString(timeString, brokerTime));
    fprintf(stderr, "[ERROR] checkTZValidity() reference time       = %s", safe_timeString(timeString, referenceTime));
    fprintf(stderr, "[ERROR] checkTZValidity() adjusted local time  = %s", safe_timeString(timeString, adjustedLocalTime));
    fprintf(stderr, "[ERROR] checkTZValidity() adjusted broker time = %s", safe_timeString(timeString, adjustedBrokerTime));
    return BROKER_TZ_MISMATCH;
  }
  else if(1)  // Always log debug messages (replaced Pantheios check)
  {
    fprintf(stderr, "[DEBUG] checkTZValidity() Local time(UTC)      = %s", safe_timeString(timeString, localTimeUTC));
    fprintf(stderr, "[DEBUG] checkTZValidity() broker time          = %s", safe_timeString(timeString, brokerTime));
    fprintf(stderr, "[DEBUG] checkTZValidity() reference time       = %s", safe_timeString(timeString, referenceTime));
    fprintf(stderr, "[DEBUG] checkTZValidity() adjusted local time  = %s", safe_timeString(timeString, adjustedLocalTime));
    fprintf(stderr, "[DEBUG] checkTZValidity() adjusted broker time = %s", safe_timeString(timeString, adjustedBrokerTime));
  }
  
  return SUCCESS;
}

AsirikuyReturnCode calculateOffsets(time_t currentTime, int *pTZOffsets, TimezoneInfo *pTZInfo)
{
  int i, dstStartDay, dstEndDay;
  struct tm timeInfo;
  char timeString[MAX_TIME_STRING_SIZE] = "";

  if(pTZOffsets == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateTimeOffsets() failed. pTZOffsets = NULL");
    return NULL_POINTER;
  }

  if(pTZInfo == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateTimeOffsets() failed. pTZInfo = NULL");
    return NULL_POINTER;
  }

  getDSTStartEndDays(currentTime, pTZInfo, &dstStartDay, &dstEndDay);
  safe_gmtime(&timeInfo, currentTime);

  for(i = 0; i <= DAYS_PER_LEAP_YEAR; i++)
  {
    if(isDST(dstStartDay, dstEndDay, pTZInfo->startHour, pTZInfo->endHour, i, timeInfo.tm_hour))
    {
      pTZOffsets[i] = pTZInfo->gmtOffsetDS;
    }
    else
    {
      pTZOffsets[i] = pTZInfo->gmtOffsetStd;
    }
  }

  return SUCCESS;
}

AsirikuyReturnCode getTimeOffsets(time_t currentBrokerTime, AccountInfo* pAccountInfo, BOOL isBackTesting, int instanceId, TZOffsets* pTZOffsets)
{
  AsirikuyReturnCode returnCode = SUCCESS;
  TimezoneInfo *localTZ, *brokerTZ, *referenceTZ;

  if(pAccountInfo == NULL)
  {
    fprintf(stderr, "[CRITICAL] getTimeOffsets() failed. pAccountInfo = NULL");
    return NULL_POINTER;
  }

  if(pTZOffsets == NULL)
  {
    fprintf(stderr, "[CRITICAL] getTimeOffsets() failed. pTZOffsets = NULL");
    return NULL_POINTER;
  }
  
  returnCode = getTimezoneInfo(LOCAL_TIMEZONE_STRING, &localTZ);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("getTimeOffsets()", returnCode);
    return returnCode;
  }
  
  returnCode = getTimezoneInfo(pAccountInfo->brokerName, &brokerTZ);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("getTimeOffsets()", returnCode);
    return returnCode;
  }
  
  returnCode = getTimezoneInfo(pAccountInfo->referenceName, &referenceTZ);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("getTimeOffsets()", returnCode);
    return returnCode;
  }
  
  fprintf(stderr, "[DEBUG] getTimeOffsets() Calculating local time offsets.");
  returnCode = calculateOffsets(currentBrokerTime, pTZOffsets->localTZOffsets, localTZ);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("getTimeOffsets()", returnCode);
    return returnCode;
  }
  
  fprintf(stderr, "[DEBUG] getTimeOffsets() Calculating broker time offsets.");
  returnCode = calculateOffsets(currentBrokerTime, pTZOffsets->brokerTZOffsets, brokerTZ);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("getTimeOffsets()", returnCode);
    return returnCode;
  }
  
  fprintf(stderr, "[DEBUG] getTimeOffsets() Calculating reference time offsets.");
  returnCode = calculateOffsets(currentBrokerTime, pTZOffsets->referenceTZOffsets, referenceTZ);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("getTimeOffsets()", returnCode);
    return returnCode;
  }

  if(!isBackTesting)
  {
    returnCode = checkTZValidity(currentBrokerTime, utcToTimezone(queryRandomNTPServer(), *referenceTZ), pTZOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("getTimeOffsets()", returnCode);
      return returnCode;
    }
  }

  return returnCode;
}

time_t getAdjustedBrokerTime(time_t brokerTime, TZOffsets* pTZOffsets)
{
  time_t adjustedBrokerTime = brokerTime;
  struct tm timeInfo;

  if(pTZOffsets == NULL)
  {
    fprintf(stderr, "[CRITICAL] getAdjustedBrokerTime() failed. pTZOffsets = NULL");
    return NULL_POINTER;
  }
  
  safe_gmtime(&timeInfo, brokerTime);
  adjustedBrokerTime += ((pTZOffsets->referenceTZOffsets[timeInfo.tm_yday] - pTZOffsets->brokerTZOffsets[timeInfo.tm_yday]) * SECONDS_PER_HOUR);

  if(1)  // Always log debug messages (replaced Pantheios check)
  {
    char sourceTime[MAX_TIME_STRING_SIZE] = "";
    char destTime[MAX_TIME_STRING_SIZE] = "";
    fprintf(stderr, "[DEBUG] getAdjustedBrokerTime() Day of year = %d, Reference offset = %d. Broker offset = %d", timeInfo.tm_yday, pTZOffsets->referenceTZOffsets[timeInfo.tm_yday], pTZOffsets->brokerTZOffsets[timeInfo.tm_yday]);
    fprintf(stderr, "[DEBUG] getAdjustedBrokerTime() Original time = %s. Adjusted time = %s", safe_timeString(sourceTime, brokerTime), safe_timeString(destTime, adjustedBrokerTime));
  }

  return adjustedBrokerTime;
}

time_t getAdjustedLocalTime(time_t localTimeUTC, TZOffsets* pTZOffsets)
{
  time_t adjustedLocalTime = localTimeUTC;
  struct tm timeInfo;

  if(pTZOffsets == NULL)
  {
    fprintf(stderr, "[CRITICAL] getAdjustedLocalTime() failed. pTZOffsets = NULL");
    return NULL_POINTER;
  }

  safe_gmtime(&timeInfo, localTimeUTC);
  adjustedLocalTime += (pTZOffsets->referenceTZOffsets[timeInfo.tm_yday] * SECONDS_PER_HOUR);

  if(1)  // Always log debug messages (replaced Pantheios check)
  {
    char sourceTime[MAX_TIME_STRING_SIZE] = "";
    char destTime[MAX_TIME_STRING_SIZE] = "";
    fprintf(stderr, "[DEBUG] getAdjustedLocalTime() Day of year = %d, Reference offset = %d. Local offset = %d", timeInfo.tm_yday, pTZOffsets->referenceTZOffsets[timeInfo.tm_yday], pTZOffsets->localTZOffsets[timeInfo.tm_yday]);
    fprintf(stderr, "[DEBUG] getAdjustedLocalTime() Original time = %s. Adjusted time = %s", safe_timeString(sourceTime, localTimeUTC), safe_timeString(destTime, adjustedLocalTime));
  }

  return adjustedLocalTime;
}