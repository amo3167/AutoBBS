/**
 * @file
 * @brief     Functions for manipulating timezones
 * 
 * @author    Morgan Doel (Initial implementation)
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

#include "Precompiled.hpp"
#include "Timezones.hpp"
#include "AsirikuyTime.h"

static time_t createTimeNthSpecifiedWeekday(int year, int month, int nthSpecifiedWeekDayOfMonth, int weekDay, int hour)
{
  int i = 0, checkWeekDay = 0;
  time_t time = 0;
  struct tm timeInfo;
  struct tm* pTimeInfo = &timeInfo;

  pTimeInfo->tm_year  = year - 1900;
  pTimeInfo->tm_mon   = month;
  pTimeInfo->tm_mday  = 1;
  pTimeInfo->tm_hour  = hour;
  pTimeInfo->tm_min   = 0;
  pTimeInfo->tm_sec   = 0;
  pTimeInfo->tm_isdst = 0;
  pTimeInfo->tm_wday  = 0;
  pTimeInfo->tm_yday  = 0;

#if defined _WIN32 || defined _WIN64
  time = _mkgmtime(pTimeInfo);
#else
  time = timegm(pTimeInfo);
#endif
  if(time == -1)
  {
    return 0;
  }
  checkWeekDay = DAY_OF_WEEK(time);

  while(i < nthSpecifiedWeekDayOfMonth)
  {
    if(checkWeekDay == weekDay)
    {
      i++;
    }
    else
    {
      time += SECONDS_PER_DAY;
      checkWeekDay++;
      if(checkWeekDay > 6)
      {
        checkWeekDay = 0;
      }
    }
  }

  return time;
}

static time_t createTimeLastSpecifiedWeekday(int year, int month, int weekDay, int hour)
{
  int i = 0, checkWeekDay = 0;
  time_t time = 0;
  struct tm timeInfo;
  struct tm* pTimeInfo = &timeInfo;

  pTimeInfo->tm_year  = year - 1900;
  pTimeInfo->tm_mon   = month;
  pTimeInfo->tm_mday  = 1;
  pTimeInfo->tm_hour  = hour;
  pTimeInfo->tm_min   = 0;
  pTimeInfo->tm_sec   = 0;
  pTimeInfo->tm_isdst = 0;
  pTimeInfo->tm_wday  = 0;
  pTimeInfo->tm_yday  = 0;

  pTimeInfo->tm_mon++;
#if defined _WIN32 || defined _WIN64
  time = _mkgmtime(pTimeInfo);
#else
  time = timegm(pTimeInfo);
#endif
  if(time == -1)
  {
    return 0;
  }
  checkWeekDay = DAY_OF_WEEK(time);
  time -= SECONDS_PER_DAY;
  checkWeekDay--;
  if(checkWeekDay < 0)
  {
    checkWeekDay = 6;
  }

  while(checkWeekDay != weekDay)
  {
    time -= SECONDS_PER_DAY;
    checkWeekDay -= 1;
    if(checkWeekDay < 0)
    {
      checkWeekDay = 6;
    }
  }

  return time;
}

static time_t createTime(int year, int month, int nthSpecifiedWeekDayOfMonth, int weekDay, int hour)
{
  if(nthSpecifiedWeekDayOfMonth > 0) /* For example, 1st Sunday of the month. */
  {
    return createTimeNthSpecifiedWeekday(year, month, nthSpecifiedWeekDayOfMonth, weekDay, hour);
  }
  else /* (nthSpecifiedWeekDayOfMonth <= 0) - Use the last specified weekday of the month. */
  {
    return createTimeLastSpecifiedWeekday(year, month, weekDay, hour);
  }
}

AsirikuyReturnCode getDSTStartEndDays(time_t currentTime, TimezoneInfo* pTzInfo, int* pDstStartDay, int* pDstEndDay)
{
  char      timeString[MAX_TIME_STRING_SIZE] = "";
  struct tm timeInfo;
  int       currentYear;
  time_t    dstStartTime;
  time_t    dstEndTime;

  if(pTzInfo == NULL)
  {
    fprintf(stderr, "[CRITICAL] getDSTStartEndDays() failed. pTzInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pDstStartDay == NULL)
  {
    fprintf(stderr, "[CRITICAL] getDSTStartEndDays() failed. pDstStartDay = NULL\n");
    return NULL_POINTER;
  }

  if(pDstEndDay == NULL)
  {
    fprintf(stderr, "[CRITICAL] getDSTStartEndDays() failed. pDstEndDay = NULL\n");
    return NULL_POINTER;
  }

  safe_gmtime(&timeInfo, currentTime);
  currentYear   = timeInfo.tm_year + TM_EPOCH_YEAR;
  dstStartTime  = createTime(currentYear, pTzInfo->startMonth, pTzInfo->startNth, pTzInfo->startDay, pTzInfo->startHour);
  safe_gmtime(&timeInfo, dstStartTime);
  *pDstStartDay = timeInfo.tm_yday;
  fprintf(stderr, "[DEBUG] getDSTStartEndDays() DST start time = %s. DST start day = %d\n", safe_timeString(timeString, dstStartTime), *pDstStartDay);

  dstEndTime    = createTime(currentYear, pTzInfo->endMonth, pTzInfo->endNth, pTzInfo->endDay, pTzInfo->endHour);
  safe_gmtime(&timeInfo, dstEndTime);
  *pDstEndDay   = timeInfo.tm_yday;
  fprintf(stderr, "[DEBUG] getDSTStartEndDays() DST end time = %s. DST end day = %d\n", safe_timeString(timeString, dstEndTime), *pDstEndDay);

  return SUCCESS;
}

BOOL isDST(int dstStartDay, int dstEndDay, int dstStartHour, int dstEndHour, int dayOfYear, int currentHour)
{
  // Ignore the start and end hour since daylight savings happens on weekends when there is no trading anyway.

  if(dstStartDay < dstEndDay)
  {
    if((dayOfYear >= dstStartDay) && (dayOfYear < dstEndDay))
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
  else
  {
    if((dayOfYear >= dstStartDay) || (dayOfYear < dstEndDay))
    {
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }
}

time_t utcToTimezone(time_t currentTime, TimezoneInfo tzInfo)
{
  struct tm timeInfo;
  int       dstStartDay;
  int       dstEndDay;

  if(getDSTStartEndDays(currentTime, &tzInfo, &dstStartDay, &dstEndDay) != SUCCESS)
  {
    return 0;
  }

  safe_gmtime(&timeInfo, currentTime);
  if(isDST(dstStartDay, dstEndDay, tzInfo.startHour, tzInfo.endHour, timeInfo.tm_yday, timeInfo.tm_hour))
  {
    currentTime += tzInfo.gmtOffsetDS * SECONDS_PER_HOUR;
  }
  else
  {
    currentTime += tzInfo.gmtOffsetStd * SECONDS_PER_HOUR;
  }

  return currentTime;
}
