/*
 * MultipleDay Helpers Module
 * 
 * Provides helper functions for multiple-day trading strategies.
 * Includes trade filtering functions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayHelpers.h"

BOOL GBPJPY_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1, timeInfoClose;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->maxTradeTime = 1;

	execution_tf = (int)pParams->settings[TIMEFRAME];
	
	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1) {
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
		logInfo("System InstanceID = %d, BarTime = %s, asia_index_rate = %d, count=%d, asia_high = %lf,asia_low = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, asia_index_rate, count, pIndicators->asia_high, pIndicators->asia_low);
	}
	else
		return FALSE;

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range * 0.94)
	{
		sprintf(pIndicators->status, "Intraday ATR(H-L) %lf is greater than euro atr range %lf",
			fabs(pIndicators->asia_high - pIndicators->asia_low), pIndicators->atr_euro_range * 0.94);

		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}
	

	if (pBase_Indicators->pDailyPredictATR < (double)parameter(AUTOBBS_IS_ATREURO_RANGE))
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than euro atr range %lf",
			pBase_Indicators->pDailyPredictATR, (double)parameter(AUTOBBS_IS_ATREURO_RANGE));

		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}
	
	return TRUE;
}

BOOL GBPUSD_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1, timeInfoClose;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->maxTradeTime = 1;

	execution_tf = (int)pParams->settings[TIMEFRAME];


	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;


	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range * 0.94)
		return FALSE;


	if (pBase_Indicators->pDailyPredictATR < (double)parameter(AUTOBBS_IS_ATREURO_RANGE))
		return FALSE;


	return TRUE;
}

