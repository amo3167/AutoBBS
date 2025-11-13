/**
 * @file
 * @brief     Functions for determining the start and end of trading week.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2013
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
#include "TradingWeekBoundaries.h"
#include "AsirikuyTime.h"

static int gCropMondayHours = 4;
static int gCropFridayHours = 4;

void setTradingWeekBoundaries(int cropMondayHours, int cropFridayHours)
{
  gCropMondayHours = cropMondayHours;
  gCropFridayHours = cropFridayHours;
}

BOOL isWeekend(time_t time)
{
   //TODO: need to overcome when moving DST, the monday 0:00 will become Sunday 23:00, but it is a valid time bar.
  //struct tm timeInfo;
  //safe_gmtime(&timeInfo, time);
  return ((DAY_OF_WEEK(time) == SATURDAY) || (DAY_OF_WEEK(time) == SUNDAY));
}

BOOL isOutsideTradingBoundaries(StrategyParams* pParams,time_t time)
{
	struct tm * timeStructure;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int   startHour = 0;

	//if (strstr(pParams->tradeSymbol, "XAU") != NULL)
	//	startHour = 1;

#if defined _MSC_VER
	timeStructure = gmtime(&time);
#else
	struct tm temp;
	timeStructure = gmtime_r(&time, &temp);
#endif

	safe_timeString(timeString, time);

	if (timeStructure->tm_hour < startHour  )
	{
		fprintf(stderr, "[DEBUG] isOutsideTradingBoundaries() PassedDate = %s\n", timeString);
		return TRUE;
	}

	return FALSE;
}


BOOL isOutsideTradingWeekBoundaries(time_t time)
{
	struct tm * timeStructure;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	#if defined _MSC_VER
		timeStructure = gmtime(&time);
	#else
		struct tm temp;
		timeStructure = gmtime_r(&time, &temp);
	#endif

	safe_timeString(timeString, time);

	fprintf(stderr, "[DEBUG] isOutsideTradingWeekBoundaries() PassedDate = %s Day of week: %d, Hour: %d\n", timeString, timeStructure->tm_wday, timeStructure->tm_hour);

	if((timeStructure->tm_wday == MONDAY) && (timeStructure->tm_hour < gCropMondayHours))
  {
		return TRUE;
  }
 
	if((timeStructure->tm_wday == FRIDAY) && (timeStructure->tm_hour > (23 - gCropFridayHours)))
  {
		return TRUE;
  }

	return FALSE;
}

BOOL isForexBrokerHoliday(time_t time)
{
  struct tm * timeStructure;

    #if defined _MSC_VER
		timeStructure = gmtime(&time);
	#else
		struct tm temp;
		timeStructure = gmtime_r(&time, &temp);
	#endif

	if((timeStructure->tm_mon == 11) && (timeStructure->tm_mday == 25))
  {
    /* Christmas Day */
		return TRUE;
  }
 
	if((timeStructure->tm_mon == 0) && (timeStructure->tm_mday == 1))
  {
    /* New Years Day */
		return TRUE;
  }

	return FALSE;
}

BOOL isValidTradingTime(StrategyParams* pParams,time_t time)
{
	//return !(isWeekend(time) || isForexBrokerHoliday(time) || isOutsideTradingBoundaries(pParams,time));
	return !(isWeekend(time) || isForexBrokerHoliday(time) || isOutsideTradingBoundaries(pParams, time)) || (isWeekend(time) 
		&& (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL));
}