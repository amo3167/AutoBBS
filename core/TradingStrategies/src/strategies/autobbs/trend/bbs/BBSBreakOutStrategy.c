/*
 * BBS BreakOut Strategy Module
 * 
 * Provides BBS BreakOut strategy execution functions.
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
#include "strategies/autobbs/trend/bbs/BBSBreakOutStrategy.h"

AsirikuyReturnCode workoutExecutionTrend_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,BOOL ignored)
{	
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double breakingHigh, breakingLow;
	double intraHigh = 99999, intraLow  = -99999;
	int count = 0;

	double baseline = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= 19 && timeInfo1.tm_min >= 25)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
		closeAllWithNegativeEasy(1, currentTime,3);
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);	

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 3;

	//pIndicators->tradeMode = 1;	

	pIndicators->subTradeMode = 0;

	//if (timeInfo1.tm_hour < 5)
	//	pIndicators->risk = 0.5;

	//TODO: 
	// if price is retreated back between the high low of breaking 15M bar, if it has a good room to entry
	breakingHigh = iHigh(B_PRIMARY_RATES, shift0Index_Primary - pIndicators->bbsIndex_excution);
	breakingLow = iLow(B_PRIMARY_RATES, shift0Index_Primary - pIndicators->bbsIndex_excution);
	count = shift1Index - pIndicators->bbsIndex_excution;
	if (count >= 2)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index, count, &intraHigh, &intraLow);

	baseline = iMA(3, B_DAILY_RATES, 50, 1);
	
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1
			&& iClose(B_DAILY_RATES,1) > baseline
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			))
		{
			pIndicators->entrySignal = 1;

			logInfo("System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_BBS_BreakOut.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);


		}
		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1
			&& iClose(B_DAILY_RATES, 1) < baseline
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)))
			)
		{
			pIndicators->entrySignal = -1;
			logInfo("System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_BBS_BreakOut.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;

	}


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	//int    shift1Index = pParams->ratesBuffers->rates[B_HOURLY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(2, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 10;


	if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}
	return SUCCESS;
}
