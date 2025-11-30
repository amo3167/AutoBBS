/*
 * Limit Strategy Module
 * 
 * Provides Limit strategy execution functions with extensive symbol-specific
 * configurations. This module implements limit order strategies based on:
 * - MACD indicators for trend determination
 * - Daily/weekly support/resistance levels
 * - BBS (Bollinger Bands Stop) indicators
 * - Range order management
 * - Time-based trading restrictions
 * - Risk management and profit targets
 * 
 * Features:
 * - Symbol-specific parameter configurations (GBPJPY, USDJPY, GBPUSD, XAUUSD, EURGBP, EURUSD, BTCUSD, ETHUSD, AUDUSD)
 * - MACD-based entry/exit signals
 * - Range trading support
 * - RSI filtering
 * - Stop loss management with trailing stops
 * - Take profit management
 * - Date filtering (non-farm payroll, Christmas, New Year)
 * - Time-based trading windows
 * 
 * Strategy Modes:
 * - Standard limit orders at pivot/S1/R1 levels
 * - Range orders for mean reversion
 * - Stop orders for breakout trades
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/trend/limit/LimitStrategy.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"
#include "strategies/autobbs/trend/common/StopLossManagement.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"  // For splitBuyRangeOrders, splitSellRangeOrders
#include "strategies/autobbs/trend/limit/LimitHelper.h"

// Strategy configuration constants
#define SPLIT_TRADE_MODE_LIMIT 4             // Split trade mode for Limit strategy
#define TP_MODE_RATIO_1_TO_1 0              // Take profit mode: 1:1 ratio
#define DEFAULT_RISK_LEVEL 1                 // Default risk level

// MACD parameters (for standard forex pairs)
#define MACD_FAST_PERIOD_SHORT 5             // Short-term fast MA period for MACD
#define MACD_SLOW_PERIOD_SHORT 10            // Short-term slow MA period for MACD
#define MACD_SIGNAL_SHORT 5                  // Short-term signal MA period for MACD

// Time constants (general strategy timing)
#define DEFAULT_CLOSE_HOUR 23                // Default close hour for EOD operations
#define DEFAULT_STOP_HOUR 23                 // Default stop hour when not symbol-specific
#define LATE_HOUR_THRESHOLD 23               // Late hour threshold for start shift adjustment
#define LATE_MINUTE_THRESHOLD 30             // Late minute threshold for start shift adjustment

// ATR and calculation constants
#define ATR_PERIOD_4H_MA_TREND 20           // ATR period for 4H MA trend calculation
#define ATR_PERIOD_HOURLY_DEFAULT 20        // Default ATR period for hourly calculation
#define ATR_PERIOD_SUNDAY 50                // ATR period for Sunday (lower liquidity)
#define ATR_PERIOD_DAILY_TAKE_PRICE 1       // ATR period for daily take price calculation
#define DAILY_ATR_DIVISOR_FOR_PENDING_CHECK 3 // ATR divisor for pending order check

// Risk adjustment constants (strategy-level)
#define RISK_REDUCED_RANGE_TRADE 0.5         // Reduced risk for range trades

// RSI constants
#define RSI_LOW_THRESHOLD 20.0              // RSI low threshold (oversold)
#define RSI_HIGH_THRESHOLD 80.0             // RSI high threshold (overbought)
#define RSI_TRADING_DAYS_DEFAULT 10         // Default trading days for RSI calculation

// Day of week constants (used in main strategy logic)
#define SUNDAY_WDAY 0                       // Sunday (tm_wday == 0)

// Day of week constants (used in main strategy logic)
#define SUNDAY_WDAY 0                       // Sunday (tm_wday == 0)

// Stop loss and take profit constants
#define STOP_LOSS_LEVEL_1X 1.0              // Stop loss level multiplier (1x take profit)
#define STOP_LOSS_LEVEL_2X 2.0              // Stop loss level multiplier (2x take profit)
#define MINUTES_FOR_HIGH_LOW_LOOKBACK 5     // Minutes for high/low price lookback (5 * 60)
#define MINUTES_FOR_TP_MOVE_CHECK 4         // Minutes for TP move check (4 * 60)
#define MINUTES_FOR_TP_MOVE_CHECK_MAX 6     // Maximum minutes for TP move check (6 * 60)
#define MINUTES_FOR_STOP_LOSS_MOVE_XAUUSD 9 // Minutes for stop loss move (XAUUSD) (9 * 60)
#define HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_START 15 // Start hour for stop loss move (XAUUSD)
#define HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_END 16   // End hour for stop loss move (XAUUSD)

// Daily SR levels calculation
#define DAILY_SR_LEVELS_LONG 26             // Number of bars for long-term daily SR levels
#define DAILY_SR_LEVELS_SHORT 9             // Number of bars for short-term daily SR levels

// MA calculation constants
#define MA_PERIOD_DAILY_BASELINE 50         // MA period for daily baseline calculation
#define MA_PERIOD_FLAT_TREND_CHECK 20       // MA period for flat trend check
#define MA_PERIOD_960M 960                  // MA period for 960-minute calculation

// Loss management constants
#define MAX_LOSS_TIMES_THRESHOLD 2          // Maximum loss times threshold
#define TOO_FAR_LIMIT_DEFAULT 1             // Default too far limit

// Spread constants
#define SPREAD_MULTIPLIER_THRESHOLD 1.5     // Spread multiplier threshold

// Auto mode constants
#define AUTO_MODE_CANCEL_ALL_ORDERS 3       // Auto mode: cancel all orders
#define AUTO_MODE_DISABLED 0                // Auto mode disabled

// Entry mode constants
#define ENTRY_MODE_1_TO_1 1                 // Entry mode: 1:1 risk/reward ratio
#define ENTRY_MODE_ATR_RANGE 3              // Entry mode: ATR range

/**
 * @brief Executes Limit strategy.
 * 
 * This function implements a comprehensive limit order strategy with extensive
 * symbol-specific configurations. It determines entry and exit signals based on:
 * - Daily trend and MACD trend alignment
 * - BBS indicators
 * - Support/resistance levels
 * - Time-based restrictions
 * - Risk management rules
 * 
 * Entry Conditions:
 * - Trend determined by daily trend, MACD, and optional Shellington/flat trend filters
 * - Price action relative to pivot/S1/R1 levels
 * - Time window restrictions
 * - Range order conditions (if enabled)
 * 
 * Exit Conditions:
 * - Take profit hit
 * - Stop loss hit
 * - Time-based exit (stop hour)
 * - Trend reversal (if exit mode enabled)
 * 
 * Additional Features:
 * - Stop loss trailing (move to break-even, then trail)
 * - Take profit adjustment
 * - Range order management
 * - Loss limit protection
 * - Spread filtering
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate with entry/exit signals.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2, timeInfo3;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = MACD_FAST_PERIOD_SHORT, slowMAPeriod = MACD_SLOW_PERIOD_SHORT, signalMAPeriod = MACD_SIGNAL_SHORT;
	int startShift = 1;
	double preDailyClose, preDailyHigh, preDailyLow, preDailyOpen;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double moveTPLimit = (double)parameter(AUTOBBS_KEYK);
	int autoMode = (int)parameter(AUTOBBS_IS_AUTO_MODE);	
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1;
	double highPrice, lowPrice;
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);
	BOOL isMoveTPInNewDay = TRUE;
	int closeHour = DEFAULT_CLOSE_HOUR, startHour = pIndicators->startHour;
	BOOL isEnableRangeTrade = FALSE;
	int trend = UNKNOWN;
	int truningPointIndex = -1, minPointIndex = -1;
	double isMACDBeili = FALSE;
	double totalLossPoint = 0;
	int totalLossTimes = 0;
	double realTakePrice;
	BOOL isEnableMACDSlow = TRUE;
	BOOL isEnableShellingtonTrend = FALSE;	
	BOOL isEnableFlatTrend = FALSE;
	BOOL isEnableWeeklyATR = TRUE;
	BOOL isEnableTooFar = FALSE;
	int stopHour = DEFAULT_STOP_HOUR;		
	double tooFarLimit = TOO_FAR_LIMIT_DEFAULT;
	int barState = BAR_UNKNOWN;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;
	double rsi = 0.0, rsiLow = RSI_LOW_THRESHOLD, rsiHigh = RSI_HIGH_THRESHOLD;
	int tradingDays = RSI_TRADING_DAYS_DEFAULT;
	BOOL isEnableRSI = FALSE;
	BOOL isEnableDoubleEntry = FALSE, isEnableDoubleEntry2 = FALSE;
	int rangeType = 0;
	int atrTimes = ATR_PERIOD_HOURLY_DEFAULT;
	BOOL isEnableTooBigSpread = FALSE;
	double ma960M = 0.0;
	int count = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_LIMIT;
	pIndicators->tpMode = TP_MODE_RATIO_1_TO_1;
	pIndicators->executionTrend = 0;
	pIndicators->risk = DEFAULT_RISK_LEVEL;
	pIndicators->isEnableLimitSR1 = FALSE;

	pIndicators->takePriceLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);

	// Move to break-even if price moves to more than 2 * TP
	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	// Find the highest/lowest price after order is opened
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, MINUTES_FOR_HIGH_LOW_LOOKBACK * 60, orderIndex, &highPrice, &lowPrice);

	pIndicators->adjust = fabs(pParams->bidAsk.bid[0] - pParams->bidAsk.ask[0]);	

	count = timeInfo1.tm_hour * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);

	ma960M = iMA(3, B_PRIMARY_RATES, MA_PERIOD_960M, 1 + count);
	
	// Use longer ATR period on Sunday
	if (timeInfo1.tm_wday == SUNDAY_WDAY)
		atrTimes = ATR_PERIOD_SUNDAY;

	logInfo("System InstanceID = %d, BarTime = %s,ask=%lf,bid=%lf,adjsut=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,pParams->bidAsk.ask[0], pParams->bidAsk.bid[0], pIndicators->adjust);


	if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		configureLimitForGBPJPY(pParams, pIndicators, pBase_Indicators, orderIndex, &startHour, &isCloseOrdersEOD, &isEnableFlatTrend);
	}
	else if (strstr(pParams->tradeSymbol, "USDJPY") != NULL)
	{
		configureLimitForUSDJPY(pParams, pIndicators, pBase_Indicators, orderIndex, &isCloseOrdersEOD, &isEnableMACDSlow);
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		configureLimitForGBPUSD(pParams, pIndicators, pBase_Indicators, orderIndex, &timeInfo1, &startHour, &isCloseOrdersEOD, &isEnableRSI);
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		BOOL shouldReturn = FALSE;
		configureLimitForXAUUSD(pParams, pIndicators, pBase_Indicators, orderIndex, &timeInfo1, timeString,
			&startHour, &stopHour, &isCloseOrdersEOD, &isEnableShellingtonTrend, &isEnableTooFar,
			&isEnableDoubleEntry, &isEnableDoubleEntry2, &shouldReturn);
		if (shouldReturn)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "EURGBP") != NULL)
	{
		configureLimitForEURGBP(pParams, pIndicators, pBase_Indicators, orderIndex, &startHour, &isCloseOrdersEOD, &isEnableMACDSlow);
	}
	else if (strstr(pParams->tradeSymbol, "EURUSD") != NULL)
	{
		configureLimitForEURUSD(pParams, pIndicators, pBase_Indicators, orderIndex, &startHour, &isCloseOrdersEOD, &isEnableMACDSlow, &isEnableFlatTrend);
	}
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	{
		BOOL shouldReturn = FALSE;
		configureLimitForBTCUSD(pParams, pIndicators, pBase_Indicators, orderIndex, &timeInfo1, timeString,
			&startHour, &stopHour, &tooFarLimit, &isCloseOrdersEOD, &isEnableWeeklyATR, &isEnableRangeTrade,
			&isEnableDoubleEntry, &isEnableTooFar, &fastMAPeriod, &slowMAPeriod, &signalMAPeriod,
			&tradingDays, &shouldReturn);
		if (shouldReturn)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		BOOL shouldReturn = FALSE;
		configureLimitForETHUSD(pParams, pIndicators, pBase_Indicators, orderIndex, &timeInfo1, timeString,
			&startHour, &tooFarLimit, &isCloseOrdersEOD, &isEnableWeeklyATR, &isEnableRangeTrade,
			&isEnableTooBigSpread, &isEnableDoubleEntry, &isEnableTooFar, &fastMAPeriod, &slowMAPeriod,
			&signalMAPeriod, &tradingDays, &shouldReturn);
		if (shouldReturn)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		configureLimitForAUDUSD(pParams, pIndicators, pBase_Indicators, orderIndex, &timeInfo1, &stopHour, &isEnableMACDSlow, &isEnableFlatTrend, &isEnableTooFar, &isCloseOrdersEOD);
	}	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE)		
		pIndicators->adjust = 0;	
		

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	// Adjust start shift if late in the day
	if (timeInfo1.tm_hour >= LATE_HOUR_THRESHOLD && timeInfo1.tm_min >= LATE_MINUTE_THRESHOLD)
		startShift = 0;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	rsi = iRSI(B_DAILY_RATES, tradingDays, 1);
	//isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	preDailyHigh = iHigh(B_DAILY_RATES, startShift);
	preDailyLow = iLow(B_DAILY_RATES, startShift);
	preDailyOpen = iOpen(B_DAILY_RATES, startShift);

	dailyBaseLine = iMA(3, B_DAILY_RATES, MA_PERIOD_DAILY_BASELINE, startShift);

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, DAILY_SR_LEVELS_LONG, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, DAILY_SR_LEVELS_SHORT, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	if (pIndicators->fast > 0
		&& (isEnableMACDSlow || pIndicators->fast > pIndicators->slow)
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& (isEnableMACDSlow || pIndicators->fast < pIndicators->slow)
		&& preDailyClose < dailyBaseLine
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_4H_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	pBase_Indicators->shellingtonInTrend = trend_4H;

	// Check for flat trend: if 20-period MA hasn't moved much relative to ATR
	if (fabs(iMA(3, B_DAILY_RATES, MA_PERIOD_FLAT_TREND_CHECK, 1) - iMA(3, B_DAILY_RATES, MA_PERIOD_FLAT_TREND_CHECK, 5)) / pBase_Indicators->dailyATR <= 0.05)
		pBase_Indicators->flatTrend = 1;
	else
		pBase_Indicators->flatTrend = 0;
	
	if (pBase_Indicators->dailyTrend > 0 && 
		pBase_Indicators->mACDInTrend == 1 //&& isMACDBeili == FALSE		
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == 1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BULL
		)
	{
		trend = UP;	
	}
	else if (pBase_Indicators->dailyTrend < 0 && 
		pBase_Indicators->mACDInTrend == -1 //&& isMACDBeili == FALSE	
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == -1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BEAR
		)
	{
		trend = DOWN;
	}
	else
		trend = RANGE;

	pIndicators->takePrice = iAtr(B_HOURLY_RATES, atrTimes, ATR_PERIOD_DAILY_TAKE_PRICE);
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	if (isEnableRSI && (rsi >= rsiHigh || rsi <= rsiLow))
	{
		logInfo("System InstanceID = %d, BarTime = %s,skip trading on rsi=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, rsi);

		return SUCCESS;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen)
	{
		if (pParams->orderInfo[orderIndex].type == BUY
			&& iHigh(B_PRIMARY_RATES, 1) >= pParams->orderInfo[orderIndex].takeProfit)
		{
			closeLongEasy(pParams->orderInfo[orderIndex].ticket);
		}

		if (pParams->orderInfo[orderIndex].type == SELL
			&& iLow(B_PRIMARY_RATES, 1) <= pParams->orderInfo[orderIndex].takeProfit)
		{
			closeShortEasy(pParams->orderInfo[orderIndex].ticket);
		}

	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && isEnableTooBigSpread == TRUE)
	{
		logInfo("System InstanceID = %d, BarTime = %s,isEnableTooBigSpread=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isEnableTooBigSpread);
		
		closeAllLimitAndStopOrdersEasy(currentTime);
		return SUCCESS;
	}
	///////////////////

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == AUTO_MODE_CANCEL_ALL_ORDERS || timeInfo1.tm_hour >= stopHour)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		// Close orders at end of day if enabled
		if (isCloseOrdersEOD == TRUE && timeInfo1.tm_hour == DEFAULT_CLOSE_HOUR && timeInfo1.tm_min >= 50)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

		//If the order is executed on the spot of stopHour, just close it immedeiatly. 
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen)
		{
			safe_gmtime(&timeInfo3, pParams->orderInfo[orderIndex].openTime);
			if ((timeInfo3.tm_hour == stopHour && timeInfo3.tm_min < 5) || (timeInfo3.tm_hour == stopHour-1 && timeInfo3.tm_min > 55))
			{
				if (pParams->orderInfo[orderIndex].type == SELL)
					closeShortEasy(pParams->orderInfo[orderIndex].ticket);
				if (pParams->orderInfo[orderIndex].type == BUY)
					closeLongEasy(pParams->orderInfo[orderIndex].ticket);
			}
			
		}
		if (trend == UP)
			entryBuyRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, TRUE, FALSE);
		if (trend == DOWN)
			entrySellRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, TRUE, FALSE);

		return SUCCESS;
	}


	logInfo("System InstanceID = %d, BarTime = %s,startHour=%d,AUTOBBS_IS_AUTO_MODE=%d,isEnableRangeTrade=%d,pBase_dailyHigh=%lf,dailyLow=%lf,pDailyMaxATR=%lf,hourATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->startHour, (int)parameter(AUTOBBS_IS_AUTO_MODE), (int)parameter(AUTOBBS_RANGE), iHigh(B_DAILY_RATES, 0), iLow(B_DAILY_RATES, 0), pBase_Indicators->pDailyMaxATR, iAtr(B_HOURLY_RATES, ATR_PERIOD_HOURLY_DEFAULT, 1));
	

	totalLossTimes = getLossTimesInDayCloseOrderEasy(currentTime, &totalLossPoint);
	// Close all orders if loss times exceeds threshold
	if (totalLossTimes >= MAX_LOSS_TIMES_THRESHOLD)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		return SUCCESS;
	}

	if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour == pIndicators->startHourOnLimt && timeInfo1.tm_min < 7)
	{
		closeAllLimitAndStopOrdersEasy(currentTime); 
	}
	
	rangeType = isRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex);
	
	if ((isEnableDoubleEntry2 == TRUE || trend == UP)
		&& rangeType == 2
		&& pIndicators->bbsTrend_primary == 1
		&& pParams->orderInfo[orderIndex].isOpen == FALSE
		&& getWinTimesInDayEasy(currentTime) < 1)
	{
		splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
		resetTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID]);
		return SUCCESS;
	}

	if ((isEnableDoubleEntry2 == TRUE || trend == DOWN)
		&& rangeType == -2
		&& pIndicators->bbsTrend_primary == -1
		&& pParams->orderInfo[orderIndex].isOpen == FALSE
		&& getWinTimesInDayEasy(currentTime) < 1)
	{
		splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		resetTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID]);
		return SUCCESS;
	}

	if (timeInfo1.tm_hour >= startHour)
	{
		//if (iHigh(B_WEEKLY_RATES, 0) - iLow(B_WEEKLY_RATES, 0) >= pBase_Indicators->weeklyATR * 0.8)
		//	autoMode = 0;


		if (trend == UP)
		{
			//TODO: check if run too far and have reverse daily bar.
			if (isEnableTooFar == TRUE && (ma960M < pBase_Indicators->dailyS2 || preDailyClose - ma960M > 0.9 * pBase_Indicators->dailyATR))
			{
				if (preDailyClose < preDailyOpen) // Bear bar
				{
					logInfo("System InstanceID = %d, BarTime = %s,ma960M=%lf, dailyS2=%lf run too far and reduce risk to 0.5.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ma960M, pBase_Indicators->dailyS2);

					//pIndicators->risk = 0.5;
					return SUCCESS;
				}
			}

			if(isEnableDoubleEntry == TRUE && rangeType == 1
				&& pIndicators->bbsTrend_primary == 1
				&& pParams->orderInfo[orderIndex].isOpen == FALSE
				&& getWinTimesInDayEasy(currentTime) < 1)
			{
				splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
				return SUCCESS;
			}


			entryBuyRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour,TRUE,TRUE);

			if (DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
				return SUCCESS;

			if (pBase_Indicators->dailyTrend <= 0)
				autoMode = 0;

			if (isEnableWeeklyATR == TRUE && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
				autoMode = 0;

			if (iLow(B_DAILY_RATES, 0) < pBase_Indicators->dailyS1)
			//if (pBase_Indicators->dailyR1 + pIndicators->takePrice - iLow(B_DAILY_RATES, 0) > pBase_Indicators->dailyATR)
				autoMode = 0;


			if ((pBase_Indicators->maTrend > 0 && timeInfo1.tm_hour >= pIndicators->startHour)
				//|| (iOpen(B_DAILY_RATES, 0) - pBase_Indicators->dailyPivot >= iAtr(B_HOURLY_RATES,20,1) && timeInfo1.tm_hour >= pIndicators->startHour)
				|| (pIndicators->bbsTrend_secondary > 0 && timeInfo1.tm_hour < pIndicators->startHour)				
				)
			{

				if (iHigh(B_DAILY_RATES, 0) - iLow(B_DAILY_RATES, 0) < pBase_Indicators->pDailyMaxATR)
					splitBuyOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);

			}
			else
				closeAllLimitAndStopOrdersEasy(currentTime);		
			
			//if (
			//	iClose(B_DAILY_RATES, 1) > iOpen(B_DAILY_RATES,1) &&
			//	iClose(B_DAILY_RATES, 2) > iOpen(B_DAILY_RATES, 2) &&
			//	iClose(B_DAILY_RATES, 3) > iOpen(B_DAILY_RATES, 3) &&
			//	(orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
			//	&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, ATR_PERIOD_HOURLY_DEFAULT, 1)) < 1
			//	//&& getWinTimesInDayEasy(currentTime) < 1		
			//	&& timeInfo1.tm_hour <= 15
			//	&& iLow(B_DAILY_RATES, 0) <= iHigh(B_DAILY_RATES, 0) - pBase_Indicators->dailyATR/2				
			//	&& (pIndicators->bbsTrend_primary == 1)
			//	)
			//{
			//	pIndicators->risk = 1;
			//	splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
			//}

			

		}
		else if (trend == DOWN)
		{
			//TODO: check if run too far and have reverse daily bar.
			if (isEnableTooFar == TRUE &&(ma960M > pBase_Indicators->dailyR2 || ma960M - preDailyClose > 0.9* pBase_Indicators->dailyATR))
			{
				if (preDailyClose > preDailyOpen) // Bull bar
				{
					logInfo("System InstanceID = %d, BarTime = %s,ma960M= %lf,dailyR2=%lf run too far and reduce risk to 0.5.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ma960M, pBase_Indicators->dailyR2);

					//pIndicators->risk = 0.5;
					return SUCCESS;
				}
			}

			if (isEnableDoubleEntry == TRUE && rangeType == -1
				&& pIndicators->bbsTrend_primary == -1
				&& pParams->orderInfo[orderIndex].isOpen == FALSE
				&& getWinTimesInDayEasy(currentTime) < 1)
			{				
				splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
				return SUCCESS;
			}

			entrySellRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, TRUE, TRUE);

			if (DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
				return SUCCESS;

			//pIndicators->executionTrend = -1;
			//pIndicators->entrySignal = -1;
			//pIndicators->exitSignal = EXIT_BUY;
			if (pBase_Indicators->dailyTrend >= 0)
				autoMode = 0;

			if (isEnableWeeklyATR == TRUE && pParams->bidAsk.ask[0] < pBase_Indicators->weeklyS2)
				autoMode = 0;

			if (iHigh(B_DAILY_RATES, 0) > pBase_Indicators->dailyR1)
			//if (iHigh(B_DAILY_RATES, 0) - (pBase_Indicators->dailyS1 - pIndicators->takePrice)> pBase_Indicators->dailyATR)
				autoMode = 0;

			if ((pBase_Indicators->maTrend < 0 && timeInfo1.tm_hour >= pIndicators->startHour)
				//|| (pBase_Indicators->dailyPivot - iOpen(B_DAILY_RATES, 0) >= iAtr(B_HOURLY_RATES, 20,1) && timeInfo1.tm_hour >= pIndicators->startHour)
				|| (pIndicators->bbsTrend_secondary < 0 && timeInfo1.tm_hour < pIndicators->startHour)
				&& DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators)
				)
			{

				if (iHigh(B_DAILY_RATES, 0) - iLow(B_DAILY_RATES, 0) < pBase_Indicators->pDailyMaxATR)
					splitSellOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);
			}
			else
				closeAllLimitAndStopOrdersEasy(currentTime);

			//if (
			//	iClose(B_DAILY_RATES, 1) < iOpen(B_DAILY_RATES, 1) &&
			//	iClose(B_DAILY_RATES, 2) < iOpen(B_DAILY_RATES, 2) &&
			//	iClose(B_DAILY_RATES, 3) < iOpen(B_DAILY_RATES, 3) &&
			//	(orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
			//	&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, ATR_PERIOD_HOURLY_DEFAULT, 1)) < 1
			//	//&& getWinTimesInDayEasy(currentTime) < 1		
			//	&& timeInfo1.tm_hour <= 15				
			//	&& iHigh(B_DAILY_RATES, 0) >= iLow(B_DAILY_RATES, 0) + pBase_Indicators->dailyATR/2
			//	&& pIndicators->bbsTrend_primary == -1
			//	)
			//{
			//	pIndicators->risk = 1;
			//	splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
			//}

		}
		else if (trend == RANGE && isEnableRangeTrade == TRUE)
	{
		pIndicators->risk = RISK_REDUCED_RANGE_TRADE;
			entryBuyRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, FALSE, TRUE);
			entrySellRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, FALSE, TRUE);
		}

	}


	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen 
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 4 * 60
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) <= 6 * 60
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		)
	{
		//By default, AUTOBBS_TP_MODE = 1
		//If AUTOBBS_RANGE = 1 and hour >15, should set isMoveTP = 0, no need move TP.
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		if (timeInfo2.tm_hour >= 15
			&& (
			(pParams->orderInfo[orderIndex].type == BUY && pParams->orderInfo[orderIndex].openPrice < iHigh(B_DAILY_RATES, 0) - pBase_Indicators->pDailyMaxATR)
			|| (pParams->orderInfo[orderIndex].type == SELL && pParams->orderInfo[orderIndex].openPrice > iLow(B_DAILY_RATES, 0) + pBase_Indicators->pDailyMaxATR)
			)
			)
			isMoveTP = 0;

		//Move stopLoss when 1 TP is hit.
		
		realTakePrice = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / stopLossLevel;

		if (pParams->orderInfo[orderIndex].type == BUY)
		{			
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice;
			pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;

			//if (pParams->bidAsk.ask[0] >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit > pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			if (pIndicators->entryPrice < pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = 1;				
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iHigh(B_PRIMARY_RATES, 1) >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			{
				logInfo("closeLong type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeLongEasy(pParams->orderInfo[orderIndex].ticket);				
			}

			return SUCCESS;
		}

		if (pParams->orderInfo[orderIndex].type == SELL)
		{			
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice* stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice;
			pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;

			//if (pParams->bidAsk.bid[0] <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit< pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			if (pIndicators->entryPrice > pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = -1;
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iLow(B_PRIMARY_RATES, 1) <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			{
				logInfo("closeShort type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeShortEasy(pParams->orderInfo[orderIndex].ticket);				
			}

			return SUCCESS;
		}

		if (isMoveTP == 1 || timeInfo1.tm_yday != timeInfo2.tm_yday)
		{	

			if (pParams->orderInfo[orderIndex].type == BUY		
				&& pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust < pParams->orderInfo[orderIndex].takeProfit
				&& pParams->orderInfo[orderIndex].openPrice - lowPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{

				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust;
				pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;
				return SUCCESS;
			}

			if (pParams->orderInfo[orderIndex].type == SELL
				&& pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust > pParams->orderInfo[orderIndex].takeProfit
				&& highPrice - pParams->orderInfo[orderIndex].openPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust;
				pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;
				return SUCCESS;
			}

		}
	}

	// Move stop loss for XAUUSD orders opened during specific hours
	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL 
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen		
		&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= MINUTES_FOR_STOP_LOSS_MOVE_XAUUSD * 60)
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		// Move stop loss if order was opened between 15:00 and 16:00
		if (timeInfo2.tm_hour >= HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_START && timeInfo2.tm_hour <= HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_END)		
		{
			// Move stop loss to break-even if still within 15:00-16:00 window
			if (timeInfo1.tm_hour >= HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_START && timeInfo1.tm_hour <= HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_END)
			{
				move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, STOP_LOSS_LEVEL_1X);
			}
			// Move stop loss to full level if past 16:00
			else if (timeInfo1.tm_hour > HOUR_FOR_STOP_LOSS_MOVE_XAUUSD_END)
			{
				move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, stopLossLevel);
			}
						
		}
		

	}

	//if ((strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	//	&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 9 * 60)
	//	&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
	//	)
	//{
	//	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

	// 1 to 1 (risk/reward ratio))
	//	{
	// 1 to 1 (risk/reward ratio))
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, 1.0);
	//		}
	//		else if (timeInfo1.tm_hour > 16)
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, stopLossLevel);
	//		}

	//	}


	//}

	//if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL 
	//	&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 9 * 60)
	//	&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
	//	)
	//{
	//	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

	//	if (timeInfo2.tm_hour >= 8 && timeInfo2.tm_hour <= 14)
	//	{
	//		if (timeInfo1.tm_hour >= 8 && timeInfo1.tm_hour <= 14)
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, 1.0);
	//		}
	//		else if (timeInfo1.tm_hour > 14)
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, stopLossLevel);
	//		}

	//	}


	//}

	return SUCCESS;
}
