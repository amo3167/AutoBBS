/*
 * ShortTerm Strategy Module
 * 
 * Provides ShortTerm strategy execution functions.
 * This strategy uses MACD and Shellington (4H MA trend) indicators to determine
 * short-term trading opportunities based on daily trend alignment.
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
#include "strategies/autobbs/trend/shortterm/ShortTermStrategy.h"
#include "strategies/autobbs/trend/shortterm/ShortTermOrderSplitting.h"

// Strategy configuration constants
#define SPLIT_TRADE_MODE_SHORT_TERM 31        // Split trade mode for short-term strategy
#define TP_MODE_SPECIFIC_PRICE 4              // Take profit mode: specific price
#define RISK_FULL 1                            // Full risk (100%)

// MACD parameters
#define MACD_FAST_PERIOD 5                    // MACD fast EMA period
#define MACD_SLOW_PERIOD 10                   // MACD slow EMA period
#define MACD_SIGNAL_PERIOD 5                   // MACD signal line period
#define MACD_START_SHIFT 1                    // MACD start shift for historical data

// Moving average periods
#define MA_PERIOD_20 20                       // 20-period MA
#define MA_PERIOD_50 50                       // 50-period MA
#define MA_PERIOD_200 200                     // 200-period MA
#define MA_BASELINE_PERIOD 50                 // Baseline MA period for daily chart

// ATR and trend calculation
#define ATR_PERIOD_FOR_MA_TREND 20            // ATR period for MA trend calculation

// Time constants
#define CLOSE_HOUR 23                         // Close hour for end of day
#define START_HOUR 2                          // Start hour for trading

// Price lookback period (minutes)
#define HIGH_LOW_PRICE_LOOKBACK_MINUTES (5 * 60)  // 5 hours in minutes

/**
 * @brief Executes ShortTerm strategy based on MACD and Shellington indicators.
 * 
 * This function implements a short-term trading strategy that:
 * - Uses MACD on daily chart to determine trend direction
 * - Uses Shellington (4H MA trend + BBS) for additional confirmation
 * - Enters trades when daily trend, MACD trend, and Shellington trend align
 * - Manages stop loss by moving to breakeven after entry
 * 
 * Algorithm:
 * 1. Load MACD indicators from daily chart.
 * 2. Calculate MA trend on 4H timeframe.
 * 3. Determine Shellington trend (4H MA trend + BBS trend alignment).
 * 4. Determine overall trend:
 *    - UP: Daily trend > 0, MACD in uptrend
 *    - DOWN: Daily trend < 0, MACD in downtrend
 *    - RANGE: Daily trend phase == 0
 * 5. For UP trend:
 *    - Enter BUY if MA signal > 0 and entry price > MA200
 *    - Move stop loss to entry price if only one order exists
 * 6. For DOWN trend:
 *    - Enter SELL if MA signal < 0 and entry price < MA200
 *    - Move stop loss to entry price if only one order exists
 * 7. For RANGE trend: Exit all positions.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = MACD_FAST_PERIOD;
	int slowMAPeriod = MACD_SLOW_PERIOD;
	int signalMAPeriod = MACD_SIGNAL_PERIOD;
	int startShift = MACD_START_SHIFT;
	double preDailyClose;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1;  // 1:1 risk/reward ratio, 3: ATR range
	double stopLossLevel = 2.5;
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1;
	double highPrice, lowPrice;
	BOOL isMoveTP = TRUE;
	int closeHour = CLOSE_HOUR;
	int startHour = START_HOUR;
	int trend = UNKNOWN;
	double ma20M, ma50M, ma200M;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_SHORT_TERM;
	pIndicators->risk = RISK_FULL;
	pIndicators->tpMode = TP_MODE_SPECIFIC_PRICE;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	// Calculate moving averages on primary timeframe
	ma20M = iMA(3, B_PRIMARY_RATES, MA_PERIOD_20, 1);
	ma50M = iMA(3, B_PRIMARY_RATES, MA_PERIOD_50, 1);
	ma200M = iMA(3, B_PRIMARY_RATES, MA_PERIOD_200, 1);

	// Load MACD indicators from daily chart
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = iMA(3, B_DAILY_RATES, MA_BASELINE_PERIOD, startShift);

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	// Determine MACD trend direction
	if (pIndicators->fast > 0
		&& preDailyClose > dailyBaseLine
		&& pIndicators->fast > pIndicators->slow
		&& pIndicators->fast > pIndicators->preFast)
	{
		pBase_Indicators->mACDInTrend = 1;  // Bullish MACD
	}

	if (pIndicators->fast < 0
		&& preDailyClose < dailyBaseLine
		&& pIndicators->fast < pIndicators->slow
		&& pIndicators->fast < pIndicators->preFast)
	{
		pBase_Indicators->mACDInTrend = -1;  // Bearish MACD
	}

	// Calculate 4H MA trend
	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_FOR_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	// Determine Shellington trend (4H MA trend + BBS trend alignment)
	if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		pBase_Indicators->shellingtonInTrend = 1;

	if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		pBase_Indicators->shellingtonInTrend = -1;

	// Determine overall trend
	if (pBase_Indicators->dailyTrend > 0 && pBase_Indicators->mACDInTrend == 1)
	{
		trend = UP;
	}
	if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1)
	{
		trend = DOWN;
	}

	if (pBase_Indicators->dailyTrend_Phase == 0)
		trend = RANGE;

	// Get high/low prices after order is opened
	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, orderIndex, HIGH_LOW_PRICE_LOOKBACK_MINUTES, &highPrice, &lowPrice);

	pIndicators->stopLossPrice = 0;  // No moving stop loss initially
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;
	pIndicators->executionTrend = 0;

	// Handle UP trend
	if (trend == UP)
	{
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		// Enter new BUY order if no existing order
		if (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			// Enter if MA signal is bullish and entry price is above MA200
			if (getMATrend_SignalBase(MA_PERIOD_20, MA_PERIOD_50, B_PRIMARY_RATES, 24) > 0 && pIndicators->entryPrice > ma200M)
			{
				splitBuyOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, pBase_Indicators->dailyATR / 2, pBase_Indicators->pDailyMaxATR);
			}
		}
		else
		{
			// Move stop loss to entry price if only one order exists
			if (getOrderCountEasy() == 1)
			{
				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				modifyTradeEasy_new(BUY, -1, fabs(pIndicators->entryPrice - pIndicators->stopLossPrice), -1, 0, pIndicators->stopMovingBackSL);
			}
		}

		pIndicators->exitSignal = EXIT_SELL;
	}
	// Handle DOWN trend
	else if (trend == DOWN)
	{
		pIndicators->entryPrice = pParams->bidAsk.bid[0];

		// Enter new SELL order if no existing order
		if (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			// Enter if MA signal is bearish and entry price is below MA200
			if (getMATrend_SignalBase(MA_PERIOD_20, MA_PERIOD_50, B_PRIMARY_RATES, 24) < 0 && pIndicators->entryPrice < ma200M)
			{
				splitSellOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, pBase_Indicators->dailyATR / 2, pBase_Indicators->pDailyMaxATR);
			}
		}
		else
		{
			// Move stop loss to entry price if only one order exists
			if (getOrderCountEasy() == 1)
			{
				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				modifyTradeEasy_new(SELL, -1, fabs(pIndicators->entryPrice - pIndicators->stopLossPrice), -1, 0, pIndicators->stopMovingBackSL);
			}
		}

		pIndicators->exitSignal = EXIT_BUY;
	}
	// Handle RANGE trend: exit all positions
	else
	{
		pIndicators->exitSignal = EXIT_ALL;
	}

	return SUCCESS;
}
