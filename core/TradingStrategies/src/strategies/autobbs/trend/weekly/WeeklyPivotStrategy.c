/*
 * Weekly Pivot Strategy Module
 * 
 * Provides Weekly Pivot strategy execution functions.
 * This strategy enters trades based on price action around weekly pivot levels,
 * looking for pivot bounces or breakouts with confirmation from previous bar.
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

/* Trading hours constants */
#define TRADING_START_HOUR 2   /* Start trading hour (2 AM) */
#define TRADING_END_HOUR 22    /* End trading hour (10 PM) */

/* Order splitting mode constant */
#define SPLIT_MODE_WEEKLY_PIVOT 11  /* Weekly pivot order splitting mode */

/**
 * Weekly Pivot strategy execution.
 * 
 * This strategy enters trades when price bounces off or breaks through weekly pivot levels.
 * Entry conditions:
 * - BUY: Price above weekly support, previous low < pivot, previous close > pivot
 * - SELL: Price below weekly support, previous high > pivot, previous close < pivot
 * 
 * Additional filters:
 * - Trading hours: 2 AM to 10 PM
 * - Prevents duplicate orders at same price within same week
 * - Stop loss: Weekly support/resistance or weekly ATR distance
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing weekly pivot and trend phase
 * @param ignored Unused parameter (kept for API compatibility)
 * @return SUCCESS on success
 */
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

	/* Close all negative positions (cleanup) */
	closeAllWithNegativeEasy(2, currentTime, 3);

	/* Filter execution timeframe */
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	/* Set default risk and take profit mode */
	pIndicators->risk = 1.0;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = SPLIT_MODE_WEEKLY_PIVOT;

	/* BUY signal: Enter on key support/resistance levels for uptrend */
	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		/* Stop loss: Minimum of weekly support or entry minus weekly ATR */
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		/* Entry conditions:
		 * - Price above weekly support (with adjustment)
		 * - Previous bar low was below pivot (bounce setup)
		 * - Previous bar close was above pivot (confirmation)
		 * - Within trading hours (2 AM to 10 PM)
		 * - No duplicate pending order at same price this week
		 */
		if (pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& preLow < pBase_Indicators->weeklyPivot
			&& preClose > pBase_Indicators->weeklyPivot
			&& timeInfo1.tm_hour >= TRADING_START_HOUR && timeInfo1.tm_hour <= TRADING_END_HOUR
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3.0, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;
	}

	/* SELL signal: Enter on key support/resistance levels for downtrend */
	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		/* Stop loss: Maximum of weekly support or entry plus weekly ATR */
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		/* Entry conditions:
		 * - Price below weekly support (with adjustment)
		 * - Previous bar high was above pivot (bounce setup)
		 * - Previous bar close was below pivot (confirmation)
		 * - Within trading hours (2 AM to 10 PM)
		 * - No duplicate pending order at same price this week
		 */
		if (pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& preHigh > pBase_Indicators->weeklyPivot
			&& preClose < pBase_Indicators->weeklyPivot
			&& timeInfo1.tm_hour >= TRADING_START_HOUR && timeInfo1.tm_hour <= TRADING_END_HOUR
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3.0, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}
