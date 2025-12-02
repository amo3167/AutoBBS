/*
 * BBS BreakOut Strategy Module
 * 
 * Provides BBS (Bollinger Bands Stop) BreakOut strategy execution functions.
 * This strategy enters trades when price breaks out of Bollinger Bands on the
 * execution timeframe, combined with daily/weekly trend analysis.
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

// Strategy configuration constants
#define RISK_FULL 1                           // Full risk (100%)
#define TP_MODE_RATIO_1_TO_1 0               // Take profit mode: 1:1 ratio
#define SPLIT_TRADE_MODE_LONG_TERM 3         // Split trade mode: Long term
#define MIN_BARS_FOR_SR_LEVELS 2             // Minimum bars needed for support/resistance calculation
#define MA_PERIOD_FOR_BASELINE 50            // Moving average period for baseline calculation
#define ATR_DIVISOR_FOR_PENDING_CHECK 3     // ATR divisor for checking pending orders

/**
 * @brief Executes BBS BreakOut strategy for daily timeframe.
 * 
 * This function implements a breakout strategy that enters trades when:
 * - BBS trend on execution timeframe matches daily trend phase
 * - Price breaks out of Bollinger Bands
 * - Daily close is above/below baseline (50-period MA)
 * - Entry price is above/below daily support/resistance levels
 * 
 * Algorithm:
 * 1. Close negative positions (with special handling for XAUUSD key dates).
 * 2. Filter execution timeframe to get valid bar index.
 * 3. Calculate breaking high/low from BBS index bar.
 * 4. Calculate intraday support/resistance levels if enough bars available.
 * 5. Calculate baseline using 50-period MA on daily chart.
 * 6. For BEGINNING_UP_PHASE:
 *    - Enter BUY if BBS trend is bullish, close > baseline, and entry conditions met.
 * 7. For BEGINNING_DOWN_PHASE:
 *    - Enter SELL if BBS trend is bearish, close < baseline, and entry conditions met.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @param ignored Whether to ignore trend phase restrictions (allows broader trend matching).
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{	
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double breakingHigh, breakingLow;
	double intraHigh = 99999, intraLow = -99999;
	int count = 0;
	double baseline = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Close negative positions with special handling for XAUUSD key dates
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= 19 && timeInfo1.tm_min >= 25)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
	{
		closeAllWithNegativeEasy(1, currentTime, 3);
	}
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = RISK_FULL;
	pIndicators->tpMode = TP_MODE_RATIO_1_TO_1;
	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_LONG_TERM;
	pIndicators->subTradeMode = 0;

	// Calculate breaking high/low from the BBS breakout bar
	breakingHigh = iHigh(B_PRIMARY_RATES, shift0Index_Primary - pIndicators->bbsIndex_excution);
	breakingLow = iLow(B_PRIMARY_RATES, shift0Index_Primary - pIndicators->bbsIndex_excution);
	
	// Calculate intraday support/resistance levels if enough bars available
	count = shift1Index - pIndicators->bbsIndex_excution;
	if (count >= MIN_BARS_FOR_SR_LEVELS)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index, count, &intraHigh, &intraLow);
	}

	// Calculate baseline using 50-period MA on daily chart
	baseline = iMA(3, B_DAILY_RATES, MA_PERIOD_FOR_BASELINE, 1);
	
	// Handle BEGINNING_UP_PHASE or broader uptrend
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		// Entry conditions: BBS trend bullish, close > baseline, and price breakout confirmed
		if (pIndicators->bbsTrend_excution == 1
			&& iClose(B_DAILY_RATES, 1) > baseline
			&& (pIndicators->bbsIndex_excution == shift1Index
			    || (intraHigh < breakingHigh && intraLow > breakingLow))
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime))))
		{
			pIndicators->entrySignal = 1;

			logInfo("System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_BBS_BreakOut.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}
		
		// Override entry signal if current daily low has been lower than stop loss price
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
		{
			pIndicators->entrySignal = 0;
		}

		pIndicators->exitSignal = EXIT_SELL;
	}

	// Handle BEGINNING_DOWN_PHASE or broader downtrend
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		// Entry conditions: BBS trend bearish, close < baseline, and price breakout confirmed
		if (pIndicators->bbsTrend_excution == -1
			&& iClose(B_DAILY_RATES, 1) < baseline
			&& (pIndicators->bbsIndex_excution == shift1Index
			    || (intraHigh < breakingHigh && intraLow > breakingLow))
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime))))
		{
			pIndicators->entrySignal = -1;
			logInfo("System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_BBS_BreakOut.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		// Override entry signal if current daily high has been higher than stop loss price
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
		{
			pIndicators->entrySignal = 0;
		}

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

/**
 * @brief Executes BBS BreakOut strategy for weekly timeframe.
 * 
 * This function implements a weekly breakout strategy that enters trades when:
 * - BBS trend on execution timeframe matches weekly trend phase
 * - Entry price is above/below weekly support/resistance levels
 * 
 * Algorithm:
 * 1. Close negative positions.
 * 2. Filter execution timeframe to get valid bar index.
 * 3. For BEGINNING_UP_PHASE:
 *    - Enter BUY if BBS trend is bullish and entry conditions met.
 * 4. For BEGINNING_DOWN_PHASE:
 *    - Enter SELL if BBS trend is bearish and entry conditions met.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @param ignored Whether to ignore trend phase restrictions (allows broader trend matching).
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_Weekly_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString1[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(2, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = RISK_FULL;
	pIndicators->tpMode = TP_MODE_RATIO_1_TO_1;
	pIndicators->splitTradeMode = 10;  // Weekly beginning split mode

	// Handle BEGINNING_UP_PHASE or broader uptrend
	if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		// Entry conditions: BBS trend bullish and entry price above weekly support
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime))
		{
			pIndicators->entrySignal = 1;
		}

		pIndicators->exitSignal = EXIT_SELL;
	}

	// Handle BEGINNING_DOWN_PHASE or broader downtrend
	if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		// Entry conditions: BBS trend bearish and entry price below weekly support
		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime))
		{
			pIndicators->entrySignal = -1;
		}

		pIndicators->exitSignal = EXIT_BUY;
	}
	
	return SUCCESS;
}
