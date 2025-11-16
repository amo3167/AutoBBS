/*
 * Hedge Strategy Module
 * 
 * Provides hedge trading strategy execution functions.
 * Range reversal trading strategies.
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
#include "strategies/autobbs/swing/hedge/HedgeStrategy.h"

/**
 * Auto Hedge strategy execution.
 * Wrapper that calls workoutExecutionTrend_Hedge.
 */
AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	// workoutExecutionTrend_ATR_Hedge was removed - call workoutExecutionTrend_Hedge instead
	workoutExecutionTrend_Hedge(pParams, pIndicators, pBase_Indicators);

	workoutExecutionTrend_Hedge(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}

/**
 * Hedge strategy execution.
 * Range reversal trading strategy.
 */
AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double down_gap = pIndicators->entryPrice - pBase_Indicators->pDailyLow;
	double up_gap = pBase_Indicators->pDailyHigh - pIndicators->entryPrice;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//closeAllWithNegativeEasy(1, currentTime, 3);
	if (timeInfo1.tm_hour == 23 && timeInfo1.tm_min > 25)
	{
		pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 14;

	//Asia hour, risk 50%
	if (timeInfo1.tm_hour <= 8)
		pIndicators->risk = 0.5;

	if (pBase_Indicators->dailyTrend_Phase > 0)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& timeInfo1.tm_hour < 23
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;

	}
	else if (pBase_Indicators->dailyTrend_Phase < 0)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& timeInfo1.tm_hour < 23
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;
	}
	else
	{
		//Range reversal: if up_gap <= 1/3 ATR, Sell
		if (up_gap <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
				&& timeInfo1.tm_hour < 23
				&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = -1;

			pIndicators->exitSignal = EXIT_BUY;
		}
		else if (down_gap <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
				&& timeInfo1.tm_hour < 23
				&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = 1;

			pIndicators->exitSignal = EXIT_SELL;
		}
	}
	return SUCCESS;
}

