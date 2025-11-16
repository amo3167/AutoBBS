/*
 * Ichimoko Weekly Strategy Module
 * 
 * Provides Ichimoko Weekly strategy execution functions.
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
#include "strategies/autobbs/trend/ichimoko/IchimokoWeeklyStrategy.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Weekly_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preWeeklyClose, preWeeklyClose1, preWeeklyClose2, preDailyClose;
	double shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
	double weekly_baseline = 0.0, weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	double freeMargin = 0.0;

	int openOrderCount = 0;
	int openOrderCountInCurrentWeek = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 33;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->volumeStep = 0.1;

	targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK);
	strategyVolRisk = pIndicators->strategyMaxRisk;

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);
	preWeeklyClose2 = iClose(B_WEEKLY_RATES, 3);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);


	if (timeInfo1.tm_hour >= 1) // 1:00 start, avoid the first hour
	{
		// Calculate daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		logInfo("System InstanceID = %d, BarTime = %s, weeklyHigh =%lf, weeklyLow=%lf, weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, weeklyHigh, weeklyLow, weekly_baseline);

		logInfo("System InstanceID = %d, BarTime = %s, shortWeeklyHigh =%lf, shortWeeklyLow=%lf, weekly_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyHigh, shortWeeklyLow, weekly_baseline_short);

		openOrderCount = getOrderCountEasy();
		openOrderCountInCurrentWeek = getOrderCountForCurrentWeekEasy(currentTime);

		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice / 2;

		//pIndicators->stopLossPrice = weekly_baseline - pBase_Indicators->weeklyATR * 0.25;
		//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);
				
		pBase_Indicators->weeklyATR = iAtr(B_WEEKLY_RATES, 20, 1);

		if (//preWeeklyClose > weekly_baseline						
			//&& preWeeklyClose > weekly_baseline_short
			//&& weekly_baseline_short > weekly_baseline		
			//&& 
			(	(preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1 && weekly_baseline - pIndicators->entryPrice > 0) ||
				(pIndicators->entryPrice > weekly_baseline && pIndicators->entryPrice < weekly_baseline_short)
			)
			//preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, 0.25 * pBase_Indicators->weeklyATR)
			)
		{
			pIndicators->entrySignal = 1;
			if (weekly_baseline - pIndicators->entryPrice > 2 * pBase_Indicators->weeklyATR)
			{
				pIndicators->risk = 4;
			}
			else if (weekly_baseline - pIndicators->entryPrice > pBase_Indicators->weeklyATR)
			{
				pIndicators->risk = 3;
			}
			else if (weekly_baseline - pIndicators->entryPrice > 0)
			{
				pIndicators->risk = 2;
			}
			else
				pIndicators->risk = 1;
		}

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pIndicators->entrySignal != 0 && openOrderCountInCurrentWeek > 0 )
	{
		logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d openOrderCountInCurrentWeek=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount, openOrderCountInCurrentWeek);

		pIndicators->entrySignal = 0;
	}

	if (pIndicators->riskPNL < -5)
	{
		logWarning("System InstanceID = %d, BarTime = %s pIndicators->riskPNL=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL);
	}

	freeMargin = caculateFreeMarginEasy();
	
	if (pIndicators->entrySignal != 0 && freeMargin / pIndicators->entryPrice < 1)
	{
		logWarning("System InstanceID = %d, BarTime = %s freeMargin=%lf, Times=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, freeMargin, freeMargin / pIndicators->entryPrice);
		pIndicators->entrySignal = 0;
	}

	if (pIndicators->entrySignal != 0 && pIndicators->riskPNL < -20)
	{
		pIndicators->entrySignal = 0;
	}
	
	// when floating profit is too high, fe 10%
	if (pIndicators->riskPNL > targetPNL)
	{
		logWarning("System InstanceID = %d, BarTime = %s closeWinningPositionsEasy pIndicators->riskPNL=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL);

		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}


	return SUCCESS;
}
