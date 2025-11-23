/*
 * BBS Swing Strategy Module
 * 
 * Provides BBS (Bollinger Bands Stop) Swing strategy execution functions.
 * This module implements 4H swing trading strategies that use Bollinger Bands
 * on the 4-hour timeframe combined with daily/weekly trend analysis.
 * 
 * Strategies included:
 * - 4H BBS Swing XAUUSD BoDuan: Specialized for XAUUSD with KeyK pattern detection
 * - 4H BBS Swing BoDuan: General BoDuan strategy with symbol-specific stop loss
 * - 4H BBS Swing: Standard 4H swing strategy with trend filtering
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
#include "strategies/autobbs/trend/bbs/BBSSwingStrategy.h"
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"

// Strategy mode constants
#define SPLIT_TRADE_MODE_4H_SWING_100P 20      // Split trade mode for 4H swing 100P
#define TP_MODE_DAILY_ATR 3                    // Take profit mode: daily ATR
#define SPLIT_TRADE_MODE_4H_SWING 19           // Split trade mode for 4H swing

// Time constants for 4H bar detection
#define HOURS_PER_4H_BAR 4                     // Hours per 4H bar
#define MINUTE_THRESHOLD_FOR_4H_BAR 3          // Minute threshold for 4H bar detection

// ATR and movement constants
#define ATR_PERIOD_FOR_MA_TREND 20             // ATR period for MA trend calculation
#define WEEKLY_ATR_FACTOR_FOR_RANGE 0.4        // Factor for weekly ATR range (40%)
#define KEYK_CLOSE_THRESHOLD_DIVISOR 3          // Divisor for KeyK close price threshold
#define RISK_REDUCTION_RANGE_TREND 0.5          // Risk reduction for range trend (50%)
#define ATR_FACTOR_FOR_RANGE_RISK 0.5          // ATR factor for range trend risk check

// Stop loss constants
#define GBPJPY_STOP_LOSS_PIPS 2.5              // Stop loss for GBPJPY (pips)
#define XAUUSD_STOP_LOSS_PIPS 20               // Stop loss for XAUUSD (pips)

// Symbol constants
#define SYMBOL_GBPJPY "GBPJPY"
#define SYMBOL_GBPAUD "GBPAUD"

// ATR divisor for pending order checks
#define ATR_DIVISOR_FOR_PENDING_4H 4           // ATR divisor for checking pending orders

/**
 * @brief Executes 4H BBS Swing strategy for XAUUSD with BoDuan pattern detection.
 * 
 * This function implements a specialized 4H swing strategy for XAUUSD that:
 * - Detects KeyK patterns on 4H bars (close near high/low with significant movement)
 * - Uses MA trend on 4H timeframe
 * - Enters trades when 4H BBS trend aligns with KeyK/MA trend
 * - Validates entry against weekly ATR predictions
 * 
 * Algorithm:
 * 1. Determine daily trend direction.
 * 2. Filter execution timeframe.
 * 3. On 4H bar boundaries (hour % 4 == 0, minute < 3):
 *    a. Calculate MA trend using 4H ATR
 *    b. Calculate 4H bar movement (high - low)
 *    c. Set atr_euro_range to 40% of predicted weekly ATR if not set
 *    d. Detect KeyK pattern if movement >= atr_euro_range
 *    e. Determine 4H trend from MA trend and KeyK pattern
 *    f. Enter BUY if 4H trend and BBS trend are bullish
 *    g. Enter SELL if 4H trend and BBS trend are bearish
 * 4. Validate entry against weekly price gap.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int orderIndex = -1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	// Process on 4H bar boundaries
	if ((timeInfo1.tm_hour - 1) % HOURS_PER_4H_BAR == 0 && timeInfo1.tm_min < MINUTE_THRESHOLD_FOR_4H_BAR)
	{
		// ATR mode: use daily ATR for take profit
		pIndicators->splitTradeMode = SPLIT_TRADE_MODE_4H_SWING_100P;
		pIndicators->tpMode = TP_MODE_DAILY_ATR;

		// Calculate MA trend using 4H ATR
		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_FOR_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

		// Calculate 4H bar movement
		movement = fabs(high_4H - low_4H);

		// Set atr_euro_range to 40% of predicted weekly ATR if not already set
		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR * WEEKLY_ATR_FACTOR_FOR_RANGE;

		logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);

		// Detect KeyK pattern: close near high/low with significant movement
		if (movement >= pIndicators->atr_euro_range)
		{
			// Bullish KeyK: close near high
			if (fabs(high_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
			{
				trend_KeyK = 1;
			}
			// Bearish KeyK: close near low
			if (fabs(low_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
			{
				trend_KeyK = -1;
			}
		}

		// Determine 4H trend from MA trend and KeyK pattern
		if (trend_MA > 0 || trend_KeyK == 1)
			trend_4H = 1;
		if (trend_MA < 0 || trend_KeyK == -1)
			trend_4H = -1;

		// Enter BUY order if 4H trend and BBS trend are bullish
		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - XAUUSD_STOP_LOSS_PIPS);

			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			
			// Enter if no existing BUY order
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY))
			{
				pIndicators->entrySignal = 1;
			}

			pIndicators->exitSignal = EXIT_SELL;
		}

		// Enter SELL order if 4H trend and BBS trend are bearish
		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + XAUUSD_STOP_LOSS_PIPS);

			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL))
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}

		// Validate entry against weekly price gap
		if (pIndicators->entrySignal != 0 &&
			fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) > pBase_Indicators->pWeeklyPredictATR)
		{
			sprintf(pIndicators->status, "current weekly price gap %lf is greater than pWeeklyPredictATR %lf",
				fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice), pBase_Indicators->pWeeklyPredictATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}
	}
	return SUCCESS;
}

/**
 * @brief Executes 4H BBS Swing BoDuan strategy with symbol-specific stop loss.
 * 
 * This function implements a BoDuan (break segment) strategy that:
 * - Uses symbol-specific stop loss (GBPJPY: 2.5 pips, GBPAUD: weekly max ATR)
 * - Detects KeyK patterns on 4H bars
 * - Enters trades when 4H BBS trend aligns with KeyK/MA trend
 * 
 * Algorithm:
 * 1. Determine daily trend direction.
 * 2. Set symbol-specific stop loss.
 * 3. On 4H bar boundaries:
 *    a. Calculate MA trend and KeyK pattern
 *    b. Determine 4H trend
 *    c. Enter orders based on 4H and BBS trend alignment
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int orderIndex = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	// Set symbol-specific stop loss
	if (strstr(pParams->tradeSymbol, SYMBOL_GBPJPY) != NULL)
	{
		pIndicators->stopLoss = GBPJPY_STOP_LOSS_PIPS;
	}
	else if (strstr(pParams->tradeSymbol, SYMBOL_GBPAUD) != NULL)
	{
		pIndicators->stopLoss = pBase_Indicators->pWeeklyPredictMaxATR;
	}

	// Process on 4H bar boundaries
	if (timeInfo1.tm_hour % HOURS_PER_4H_BAR == 0 && timeInfo1.tm_min < MINUTE_THRESHOLD_FOR_4H_BAR)
	{
		// ATR mode: use daily ATR for take profit
		pIndicators->splitTradeMode = SPLIT_TRADE_MODE_4H_SWING_100P;
		pIndicators->tpMode = TP_MODE_DAILY_ATR;

		// Calculate MA trend using 4H ATR
		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_FOR_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

		// Calculate 4H bar movement
		movement = fabs(high_4H - low_4H);

		// Set atr_euro_range to 40% of predicted weekly ATR if not already set
		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR * WEEKLY_ATR_FACTOR_FOR_RANGE;

		logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);

		// Detect KeyK pattern: close near high/low with significant movement
		if (movement >= pIndicators->atr_euro_range)
		{
			// Bullish KeyK: close near high
			if (fabs(high_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
			{
				trend_KeyK = 1;
			}
			// Bearish KeyK: close near low
			if (fabs(low_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
			{
				trend_KeyK = -1;
			}
		}

		// Determine 4H trend from MA trend and KeyK pattern
		if (trend_MA > 0 || trend_KeyK == 1)
			trend_4H = 1;
		if (trend_MA < 0 || trend_KeyK == -1)
			trend_4H = -1;

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		// Enter BUY order if 4H trend and BBS trend are bullish
		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pIndicators->stopLoss);

			// Enter if no existing BUY order
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY))
			{
				pIndicators->entrySignal = 1;
			}

			pIndicators->exitSignal = EXIT_SELL;
		}

		// Enter SELL order if 4H trend and BBS trend are bearish
		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pIndicators->stopLoss);

			// Enter if no existing SELL order
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL))
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}
	}
	return SUCCESS;
}

/**
 * @brief Executes standard 4H BBS Swing strategy with trend filtering.
 * 
 * This function implements the standard 4H swing strategy that:
 * - Uses 4H BBS trend for entry signals
 * - Filters entries based on execution timeframe BBS trend
 * - Adjusts risk for range-bound markets
 * - Validates entries against weekly ATR
 * 
 * Algorithm:
 * 1. Determine daily trend direction.
 * 2. Filter execution timeframe.
 * 3. Calculate MA trend and KeyK pattern on 4H bars.
 * 4. Determine 4H trend.
 * 5. For BUY signals:
 *    a. Check if 4H BBS trend is bullish
 *    b. Validate against execution timeframe BBS trend
 *    c. Check weekly ATR constraint
 *    d. Adjust risk for range markets
 * 6. For SELL signals:
 *    a. Check if 4H BBS trend is bearish
 *    b. Validate against execution timeframe BBS trend
 *    c. Check weekly ATR constraint
 *    d. Adjust risk for range markets
 * 7. Apply profit management.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	// ATR mode: use daily ATR for take profit
	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_4H_SWING;
	pIndicators->tpMode = TP_MODE_DAILY_ATR;

	// Calculate MA trend using 4H ATR
	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_FOR_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

	// Calculate 4H bar movement
	movement = fabs(high_4H - low_4H);
	
	// Set atr_euro_range to 40% of predicted weekly ATR if not already set
	if (pIndicators->atr_euro_range == 0)
		pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR * WEEKLY_ATR_FACTOR_FOR_RANGE;

	logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);

	// Detect KeyK pattern: close near high/low with significant movement
	if (movement >= pIndicators->atr_euro_range)
	{
		// Bullish KeyK: close near high
		if (fabs(high_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
		{
			trend_KeyK = 1;
		}
		// Bearish KeyK: close near low
		if (fabs(low_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
		{
			trend_KeyK = -1;
		}
	}

	// Determine 4H trend from MA trend and KeyK pattern
	if (trend_MA > 0 || trend_KeyK == 1)
		trend_4H = 1;
	if (trend_MA < 0 || trend_KeyK == -1)
		trend_4H = -1;

	// Handle BUY signals when 4H BBS trend is bullish
	if (pIndicators->bbsTrend_4H == 1)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pBase_Indicators->dailyS);

		// Entry conditions: execution BBS trend bullish, 4H trend bullish, and weekly ATR constraint met
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_4H, currentTime)
			&& trend_4H == 1
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = 1;
		}

		// Reduce risk for range-bound markets
		if (dailyTrend == 0
			&& pIndicators->entryPrice >= pBase_Indicators->dailyS
			&& fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) >= pBase_Indicators->dailyATR * ATR_FACTOR_FOR_RANGE_RISK)
		{
			pIndicators->risk = RISK_REDUCTION_RANGE_TREND;
		}

		// Invalidate entry if stop loss is above entry price
		if (pIndicators->stopLossPrice > pIndicators->entryPrice)
		{
			pIndicators->entrySignal = 0;
		}
	}

	// Handle SELL signals when 4H BBS trend is bearish
	if (pIndicators->bbsTrend_4H == -1)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		
		// Set stop loss based on daily trend
		if (dailyTrend == 0)
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyR);
		else
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyS);

		// Entry conditions: execution BBS trend bearish, 4H trend bearish, and weekly ATR constraint met
		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_4H, currentTime)
			&& trend_4H == -1
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = -1;
		}

		// Reduce risk for range-bound markets
		if (dailyTrend == 0 && pIndicators->entryPrice <= pBase_Indicators->dailyR
			&& fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) >= pBase_Indicators->dailyATR * ATR_FACTOR_FOR_RANGE_RISK)
		{
			pIndicators->risk = RISK_REDUCTION_RANGE_TREND;
		}

		// Reduce risk if stop loss is below entry price (invalid configuration)
		if (pIndicators->stopLossPrice < pIndicators->entryPrice)
		{
			pIndicators->risk = RISK_REDUCTION_RANGE_TREND;
		}
	}

	// Apply profit management
	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}
