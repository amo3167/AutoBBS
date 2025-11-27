/*
 * MultipleDay Strategy Module
 * 
 * Provides multiple-day trading strategy execution functions for swing trading.
 * 
 * Main Functions:
 * - workoutExecutionTrend_MultipleDay: General multiple-day strategy supporting multiple symbols
 * 
 * Supported Symbols:
 * - Commodities: XAGUSD, XAUUSD
 * - Cryptocurrencies: BTCUSD, ETHUSD
 * - Forex: GBPJPY, GBPUSD, AUDUSD
 * 
 * Strategy Features:
 * - MACD-based trend analysis
 * - Bollinger Bands for entry signals
 * - Position adding logic for long-term trends
 * - Symbol-specific risk management and filtering
 * - Uses enterOrder_MultipleDay and modifyOrder_MultipleDay for order management
 * 
 * Source: Based on SwingStrategy.refactor.c (refactor branch)
 */

#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayStrategy.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayHelpers.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayOrderManagement.h"
#include "strategies/autobbs/swing/daytrading/DayTradingHelpers.h"
#include <math.h>
#include <string.h>

/* Bollinger Bands constants */
#define BBANDS_PERIOD 50      /* Bollinger Bands period */
#define BBANDS_DEVIATIONS 2   /* Bollinger Bands standard deviations */
#define BBANDS_UPPER_BAND 0   /* Upper band index */
#define BBANDS_LOWER_BAND 2   /* Lower band index */

/**
 * General multiple-day trading strategy execution.
 * 
 * Source: SwingStrategy.refactor.c (refactor branch)
 * Advanced implementation with support for multiple symbols:
 * - XAGUSD, XAUUSD, BTCUSD, ETHUSD, GBPJPY, GBPUSD, AUDUSD
 * - MACD-based trend analysis
 * - Position adding logic for long-term trends
 * - Uses enterOrder_MultipleDay and modifyOrder_MultipleDay for order management
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing MA trend and ATR
 * @return SUCCESS on success
 */
AsirikuyReturnCode workoutExecutionTrend_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime, openTime;
	struct tm timeInfo1, timeInfo2;
	double ATR0;
	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;
	Order_Info orderInfo;
	double entryPrice;
	int maxTradeTime = 1, latestOrderIndex = 0, orderCountToday = 0,oldestOpenOrderIndex = 0;
	double upperBBand, lowerBBand;

	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow, intradayCloseHigh, intradayCloseLow;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	double range;
	double floatingTP;
	double daily_baseline = 0.0, dailyHigh = 0.0, dailyLow = 0.0, preDailyClose;
	int executionTrend;
	
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;

	BOOL isSameDayOrder = FALSE, isSameDayClosedOrder = FALSE, isLastOrderProfit = FALSE;
	BOOL isPreviousDayOrder = FALSE;
	BOOL shouldFilter = TRUE;
	int takeProfitMode = 0;

	double riskCapBuy = 0;
	double riskCapSell = 0;

	int winningOrdersToday = 0;
	
	BOOL isAddPosition = FALSE;

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);
	double preOpen = iOpen(B_PRIMARY_RATES, 1);

	double ATR20 = iAtr(B_DAILY_RATES, 20, 1);
	double pMaxATR = max(pBase_Indicators->pDailyATR, ATR20);

	double addPositionBaseLine;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 22;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->tradeMode = 1;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		executionTrend = -1;
	else
		executionTrend = 0;

	pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);


	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	// Calculate intraday high/low levels
	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
		iSRLevels_close(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayCloseHigh, &intradayCloseLow);
	}
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	pIndicators->atr0 = fabs(intradayHigh - intradayLow);
	ATR0 = pIndicators->atr0;
	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	// Get order information
	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);	
	oldestOpenOrderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);

	if (oldestOpenOrderIndex >= 0){
		side = pParams->orderInfo[oldestOpenOrderIndex].type;

		safe_gmtime(&timeInfo2, pParams->orderInfo[oldestOpenOrderIndex].openTime);
		if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
			isSameDayOrder = TRUE;
		if (isSameDayOrder == TRUE && timeInfo2.tm_hour < pIndicators->startHour)
		{
	logWarning("System InstanceID = %d, BarTime = %s, same day opentime  %d is less than %d. It should be a manual take over order.", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo2.tm_hour, pIndicators->startHour);
			isSameDayOrder = FALSE;
		}
	}
	else
		side = NONE;

	// Check if latest order was closed today
	// If the last order was from previous day and closed with profit, adjust stop loss to previous high
	// 1. Add new position
	// 2. If price returns to previous entry, add position
	// 3. Cancel previous order, enter 15-minute order
	if (latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].isOpen == FALSE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[latestOrderIndex].openTime);

		//if (timeInfo1.tm_yday == timeInfo2.tm_yday + 1
		//	|| (timeInfo1.tm_yday == 0 && timeInfo2.tm_yday == 365 && timeInfo1.tm_year == timeInfo2.tm_year + 1))
		//	isPreviousDayOrder = TRUE;

		//if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
		//	isSameDayOrder = TRUE;

		safe_gmtime(&timeInfo2, pParams->orderInfo[latestOrderIndex].closeTime);

		if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
		{
			isSameDayClosedOrder = TRUE;
		}

		if (pParams->orderInfo[latestOrderIndex].profit >= 0)
		{
			isLastOrderProfit = TRUE;
		}
	}


	// TODO: Need to modify BASE to support new trend calculation that requires time
	// Currently choosing to open on second day, which has weekend issues
	if (oldestOpenOrderIndex >= 0 && timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
		//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{

		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			preDailyClose = iClose(B_DAILY_RATES, 0);

			iTrend_MA_DailyBar_For1H(0, &(pBase_Indicators->dailyMATrend), 1);
			//iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily-1, 26, &dailyHigh, &dailyLow);
			//daily_baseline = (dailyHigh + dailyLow) / 2;
			//daily_baseline = iMA(3, B_DAILY_RATES, 50, 0);

			if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY &&
				(executionTrend < 0 ||
				(executionTrend == 0 &&
				//	preDailyClose < daily_baseline &&
				pBase_Indicators->dailyMATrend < 0
				)
				)
				)
				pIndicators->exitSignal = EXIT_ALL;
			if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL &&
				(executionTrend > 0 ||
				(executionTrend == 0 &&
				//preDailyClose > daily_baseline &&
				pBase_Indicators->dailyMATrend > 0
				)
				)
				)
				pIndicators->exitSignal = EXIT_ALL;
			return SUCCESS;
		}
		else{
			if (pParams->orderInfo[oldestOpenOrderIndex].isOpen == TRUE && timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
				//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
			{

				if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY &&
					//pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 0
					executionTrend <= 0
					)
					pIndicators->exitSignal = EXIT_ALL;
				if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL &&
					//pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 0
					executionTrend >= 0
					)
					pIndicators->exitSignal = EXIT_ALL;
				return SUCCESS;
			}
		}

	}

	if (oldestOpenOrderIndex >= 0 &&
		((pParams->orderInfo[oldestOpenOrderIndex].type == BUY && pParams->orderInfo[oldestOpenOrderIndex].stopLoss - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= -2 * pIndicators->adjust) ||
		(pParams->orderInfo[oldestOpenOrderIndex].type == SELL &&  pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->orderInfo[oldestOpenOrderIndex].stopLoss >= -2 * pIndicators->adjust))
		){
	logInfo("System InstanceID = %d, BarTime = %s,stopLoss =%lf. it is ok to add new positions in a long term trend now.", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->orderInfo[oldestOpenOrderIndex].stopLoss);
		isAddPosition = TRUE;
	}

	if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		BOOL shouldSkip = FALSE;

		AsirikuyReturnCode result = setupXAGUSDEntrySignal_MultipleDay(
			pParams, pIndicators, pBase_Indicators,
			latestOrderIndex, side,
			isSameDayOrder, shouldFilter,
			&timeInfo1, timeString,
			&floatingTP, &takeProfitMode, &shouldSkip);
		
		if (result != SUCCESS)
			return result;
		
		/* If function indicates we should skip (filter blocked), exit early */
		if (shouldSkip == TRUE)
			return SUCCESS;
	}

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		riskCapBuy = parameter(AUTOBBS_RISK_CAP);
		riskCapSell = riskCapBuy - 2;

		BOOL shouldSkip = FALSE;

		AsirikuyReturnCode result = setupXAUUSDEntrySignal_MultipleDay(
			pParams, pIndicators, pBase_Indicators,
			oldestOpenOrderIndex, side,
			isAddPosition, isSameDayOrder, shouldFilter,
			preLow, preHigh, preClose,
			&timeInfo1, timeString,
			&floatingTP, &takeProfitMode, &shouldSkip);
		
		if (result != SUCCESS)
			return result;
		
		/* If function indicates we should skip (entry signal set or filter blocked), exit early */
		if (shouldSkip == TRUE)
			return SUCCESS;
	}
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		riskCapBuy = parameter(AUTOBBS_RISK_CAP);
		riskCapSell = 0;

		BOOL shouldSkip = FALSE;

		AsirikuyReturnCode result = setupCryptoEntrySignal_MultipleDay(
			pParams, pIndicators, pBase_Indicators,
			latestOrderIndex, side,
			isSameDayOrder, shouldFilter,
			&timeInfo1, timeString,
			&floatingTP, &takeProfitMode, &shouldSkip);
		
		if (result != SUCCESS)
			return result;
		
		/* If function indicates we should skip (filter blocked), exit early */
		if (shouldSkip == TRUE)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		riskCapBuy = parameter(AUTOBBS_RISK_CAP);
		riskCapSell = 0;

		BOOL shouldSkip = FALSE;

		AsirikuyReturnCode result = setupGBPJPYEntrySignal_MultipleDay(
			pParams, pIndicators, pBase_Indicators,
			executionTrend, oldestOpenOrderIndex, side,
			isAddPosition, isSameDayOrder,
			preLow, preHigh, preClose,
			&timeInfo1, timeString,
			&floatingTP, &shouldSkip);
		
		if (result != SUCCESS)
			return result;
		
		/* If function indicates we should skip (entry signal set or filter blocked), exit early */
		if (shouldSkip == TRUE)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		BOOL shouldSkip = FALSE;

		AsirikuyReturnCode result = setupGBPUSDEntrySignal_MultipleDay(
			pParams, pIndicators, pBase_Indicators,
			executionTrend, latestOrderIndex, side,
			isSameDayOrder,
			&timeInfo1, timeString,
			&floatingTP, &shouldSkip);
		
		if (result != SUCCESS)
			return result;
		
		/* If function indicates we should skip (filter blocked), exit early */
		if (shouldSkip == TRUE)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		BOOL shouldSkip = FALSE;

		AsirikuyReturnCode result = setupAUDUSDEntrySignal_MultipleDay(
			pParams, pIndicators, pBase_Indicators,
			latestOrderIndex, side,
			isSameDayOrder,
			&timeInfo1, timeString,
			&floatingTP, &shouldSkip);
		
		if (result != SUCCESS)
			return result;
		
		/* If function indicates we should skip (filter blocked), exit early */
		if (shouldSkip == TRUE)
			return SUCCESS;
	}

	if (side == NONE)
	{
		enterOrder_MultipleDay(pParams, pIndicators, pBase_Indicators, riskCapBuy, riskCapSell, isSameDayClosedOrder);
	}
	else
	{
		// Modify existing orders
		modifyOrder_MultipleDay(pParams, pIndicators, pBase_Indicators, oldestOpenOrderIndex, intradayHigh, intradayLow, floatingTP, takeProfitMode, TRUE);
	}


	

	return SUCCESS;
}


