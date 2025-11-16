/*
 * Weekly Strategy Module
 * 
 * Provides weekly trading strategy execution functions.
 * Includes Weekly Swing and Weekly ATR Prediction strategies.
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
#include "strategies/autobbs/swing/weekly/WeeklyStrategy.h"

AsirikuyReturnCode workoutExecutionTrend_Weekly_Swing_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1_execution;
	time_t currentTime;
	struct tm timeInfo1;
	BOOL isOpen;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 13;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	shift1_execution = shift1Index_primary;

	//���˵���ŷ���Ǹ��ܣ�ֻ�����ڲ���
	if (timeInfo1.tm_year == 116 && timeInfo1.tm_mon == 9 && timeInfo1.tm_mday >= 3 && timeInfo1.tm_mday <= 7)
		return SUCCESS;

	if (timeInfo1.tm_wday == 1 && timeInfo1.tm_hour == 0)
	{
		return SUCCESS;
	}


	if (hasSameWeekOrderEasy(currentTime, &isOpen) == FALSE)  //���û�п���, �Ϳ��Կ���
	{
		if (pBase_Indicators->maTrend > 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
			pIndicators->entrySignal = 1;
		}

		if (pBase_Indicators->maTrend < 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
			pIndicators->entrySignal = -1;

		}
	}
	else
	{
		logWarning("System InstanceID = %d, BarTime = %s, maTrend = %ld,ma_Signal = %ld",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, pBase_Indicators->ma_Signal);

		if (pBase_Indicators->maTrend > 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
			if (pBase_Indicators->ma_Signal > 0)
			{
				pIndicators->lossTimes = getLossTimesInWeekEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInWeekEasy(currentTime);

				logWarning("System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes);

				if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = 1;
				}
			}
			pIndicators->exitSignal = EXIT_SELL;
		}
		else if (pBase_Indicators->maTrend < 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
			if (pBase_Indicators->ma_Signal < 0)
			{
				pIndicators->lossTimes = getLossTimesInWeekEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInWeekEasy(currentTime);

				logWarning("System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes);

				if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = -1;
				}
			}
			pIndicators->exitSignal = EXIT_BUY;
		}
	}

	return SUCCESS;
}
AsirikuyReturnCode workoutExecutionTrend_WeeklyATR_Prediction(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	predictWeeklyATR_LongerTerm(pParams, pBase_Indicators);
	
	savePredicatedWeeklyATR(pParams->tradeSymbol, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, (BOOL)pParams->settings[IS_BACKTESTING]);

	logWarning("System InstanceID = %d, BarTime = %s, pWeeklyPredictATR=%lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	return SUCCESS;
}
