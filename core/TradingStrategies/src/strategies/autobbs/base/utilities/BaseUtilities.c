/*
 * Base Utilities Module
 * 
 * Provides utility functions for Base strategy:
 * - Order modification
 * - Limit order management
 * - Day detection
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/utilities/BaseUtilities.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

AsirikuyReturnCode base_ModifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice)
{	
	int tpMode = 0;

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
			modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
			modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	return SUCCESS;
}


void closeAllLimitPreviousDayOrders(StrategyParams* pParams)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	closeAllBuyLimitOrdersEasy(currentTime);
	closeAllSellLimitOrdersEasy(currentTime);
}

BOOL isNewDay(StrategyParams* pParams, time_t currentTime)
{
	time_t virtualOrderEntryTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	char       timeString2[MAX_TIME_STRING_SIZE] = "";

	virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

	safe_gmtime(&timeInfo1, currentTime);
	safe_gmtime(&timeInfo2, virtualOrderEntryTime);

	safe_timeString(timeString1, currentTime);
	safe_timeString(timeString2, virtualOrderEntryTime);

	logDebug("Last order update time = %s,current time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime, currentTime) / 3600);

	if (virtualOrderEntryTime != -1 && timeInfo1.tm_mday != timeInfo2.tm_mday && timeInfo1.tm_min >= 15) // New day
	{
		logDebug("Move to a new day.\n");
		//setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
		return TRUE;
	}
	return FALSE;
}
