/*
 * Time Management Module
 * 
 * Provides functions for time-based trading restrictions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"

BOOL DailyTrade_Limit_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	time_t currentTime;
	struct tm timeInfo1;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	//Ignore any trade on time range:
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		if (timeInfo1.tm_hour >= 10)
		{
			sprintf(pIndicators->status, "Ignore trading after 10\n");
			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			closeAllLimitAndStopOrdersEasy(currentTime);
			return FALSE;
		}
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		if (timeInfo1.tm_hour >= 8 && timeInfo1.tm_hour <= 14)
		{
			sprintf(pIndicators->status, "Ignore trading between 8-14\n");
			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			closeAllLimitAndStopOrdersEasy(currentTime);
			return FALSE;
		}
	}
	return TRUE;
}

