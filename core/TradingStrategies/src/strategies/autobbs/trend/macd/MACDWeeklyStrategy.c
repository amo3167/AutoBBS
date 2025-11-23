/*
 * MACD Weekly Strategy Module
 * 
 * Provides MACD Weekly strategy execution functions.
 * This strategy uses weekly MACD indicators to determine entry and exit signals
 * for longer-term trend-following trades.
 */

#include <string.h>
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/macd/MACDWeeklyStrategy.h"

// Strategy configuration constants
#define SPLIT_TRADE_MODE_MACD_WEEKLY 25    // Split trade mode for MACD Weekly strategy
#define TP_MODE_DAILY_ATR 3                // Take profit mode: daily ATR
#define TP_MODE_STANDARD 1                 // Standard trade mode

// MACD calculation parameters
#define MACD_FAST_PERIOD 5                 // Fast MA period for MACD
#define MACD_SLOW_PERIOD 10                // Slow MA period for MACD
#define MACD_SIGNAL_PERIOD 5               // Signal MA period for MACD

// MACD level thresholds (symbol-specific)
#define MACD_LEVEL_XAU 10                  // MACD level threshold for XAU symbols
#define MACD_LEVEL_JPY 0                   // MACD level threshold for JPY pairs
#define MACD_LEVEL_DEFAULT 0.0005          // Default MACD level threshold for other symbols

// Volume MA period
#define VOLUME_MA_PERIOD 3                 // Period for volume moving average

/**
 * @brief Executes MACD Weekly strategy.
 * 
 * This function implements a weekly MACD-based trading strategy that:
 * 1. Calculates weekly MACD indicators (fast, slow, signal).
 * 2. Determines entry signals based on MACD crossover and weekly trend.
 * 3. Sets stop loss based on predicted weekly max ATR.
 * 4. Uses volume confirmation for entry signals.
 * 
 * Entry Conditions (BUY):
 * - MACD fast > level threshold (symbol-specific)
 * - Weekly trend > 0
 * MACD fast > MACD slow (bullish crossover)
 * - Volume1 > Volume2 (volume confirmation)
 * - MACD fast > previous MACD fast (momentum)
 * 
 * Entry Conditions (SELL):
 * - MACD fast < -level threshold
 * - Weekly trend < 0
 * - MACD fast < MACD slow (bearish crossover)
 * - Volume1 > Volume2 (volume confirmation)
 * - MACD fast < previous MACD fast (momentum)
 * 
 * Exit Conditions:
 * - BUY exit: MACD fast > MACD slow AND previous MACD fast <= previous MACD slow
 * - SELL exit: MACD fast < MACD slow AND previous MACD fast >= previous MACD slow
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate with entry/exit signals.
 * @param pBase_Indicators Base indicators containing weekly trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   weeklyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double fast, slow;
	double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_WEEKLY_RATES, 5, 1);
	/* int index1, index2, index3; */
	double level = 0;
	double volume1, volume2, volume_ma_3;

	currentTime = pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index_Weekly];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart

	if (pBase_Indicators->weeklyTrend_Phase > 0)
		weeklyTrend = 1;
	else if (pBase_Indicators->weeklyTrend_Phase < 0)
		weeklyTrend = -1;
	else
		weeklyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_MACD_WEEKLY;
	pIndicators->tpMode = TP_MODE_DAILY_ATR;
	pIndicators->tradeMode = TP_MODE_STANDARD;

	// Set MACD level threshold based on symbol
	if (strstr(pParams->tradeSymbol, "XAU") != NULL)
		level = MACD_LEVEL_XAU;
	else if (strstr(pParams->tradeSymbol, "JPY") != NULL)
		level = MACD_LEVEL_JPY;
	else
		level = MACD_LEVEL_DEFAULT;

	//if (timeInfo1.tm_hour == 1) // 1:00 start, avoid the first hour
	{
		// Volume indicator: current volume > past average volume
		volume1 = iVolume(B_WEEKLY_RATES, 1);
		volume2 = iVolume(B_WEEKLY_RATES, 2);
		volume_ma_3 = iMA(4, B_WEEKLY_RATES, VOLUME_MA_PERIOD, 1);

		// Load MACD indicators
		fast = iMACD(B_WEEKLY_RATES, MACD_FAST_PERIOD, MACD_SLOW_PERIOD, MACD_SIGNAL_PERIOD, 0, 1);
		slow = iMACD(B_WEEKLY_RATES, MACD_FAST_PERIOD, MACD_SLOW_PERIOD, MACD_SIGNAL_PERIOD, 1, 1);

		// Previous period MACD values
		preFast = iMACD(B_WEEKLY_RATES, MACD_FAST_PERIOD, MACD_SLOW_PERIOD, MACD_SIGNAL_PERIOD, 0, 2);
		preSlow = iMACD(B_WEEKLY_RATES, MACD_FAST_PERIOD, MACD_SLOW_PERIOD, MACD_SIGNAL_PERIOD, 1, 2);


		logWarning("System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 1), fast, slow);
		logWarning("System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 2), preFast, preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		//if (iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR)
		//{
		//	logWarning("System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, iAtr(B_WEEKLY_RATES, 1, 0), pBase_Indicators->pWeeklyPredictMaxATR);
		//	return FALSE;
		//}

		// BUY signal conditions
		if (fast > level
			&& weeklyTrend > 0 
			&& fast > slow
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR;

			// Entry signal: no open orders, volume confirmation, and MACD momentum
			if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& volume1 > volume2
				&& fast > preFast
				)
			{
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;
		}

		// SELL signal conditions
		if (fast < (level * -1)
			&& weeklyTrend < 0
			&& fast < slow
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR;

			// Entry signal: no open orders, volume confirmation, and MACD momentum
			if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& volume1 > volume2
				&& fast < preFast
				)
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}


		// Exit signals: MACD crossover reversal
		if (fast > slow && preFast <= preSlow) // Exit SELL (close short positions)
			pIndicators->exitSignal = EXIT_SELL;

		if (fast < slow && preFast >= preSlow) // Exit BUY (close long positions)
			pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}
