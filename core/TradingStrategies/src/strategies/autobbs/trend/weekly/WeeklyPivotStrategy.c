/*
 * Weekly Pivot Strategy Module
 * 
 * Provides Weekly Pivot strategy execution functions.
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
#include "strategies/autobbs/trend/weekly/WeeklyPivotStrategy.h"

AsirikuyReturnCode workoutExecutionTrend_Weekly_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(2, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);


	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 11;

	// Enter order on key support/resistance levels
	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		if (pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& preLow < pBase_Indicators->weeklyPivot
			&& preClose >  pBase_Indicators->weeklyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		if (pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& preHigh > pBase_Indicators->weeklyPivot
			&& preClose <  pBase_Indicators->weeklyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}
