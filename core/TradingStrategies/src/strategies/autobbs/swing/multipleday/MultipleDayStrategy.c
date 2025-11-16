/*
 * MultipleDay Strategy Module
 * 
 * Provides multiple-day trading strategy execution functions.
 * Includes GBPJPY and general MultipleDay strategies.
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
#include "strategies/autobbs/swing/multipleday/MultipleDayStrategy.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayHelpers.h"
#include "strategies/autobbs/swing/common/MultipleDayOrderManagement.h"

/* Bollinger Bands constants */
#define BBANDS_PERIOD 50      /* Bollinger Bands period */
#define BBANDS_DEVIATIONS 2   /* Bollinger Bands standard deviations */
#define BBANDS_UPPER_BAND 0   /* Upper band index */
#define BBANDS_LOWER_BAND 2   /* Lower band index */

/**
 * GBPJPY multiple-day trading strategy execution (Old version).
 * 
 * Strategy logic:
 * - Uses splitTradeMode 23
 * - Entry based on Bollinger Bands breakout:
 *   * BUY: Price closes above upper BBand AND MA trend up AND daily trend > 0
 *   * SELL: Price closes below lower BBand AND MA trend down AND daily trend < 0
 * - Filters: Only enters if winTimes == 0 and lossTimes < maxTradeTime
 * - Uses weekly 4H trend signal (commented out)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing MA trend and daily trend
 * @return SUCCESS on success
 */
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_MultipleDay_old(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double Range;
	double adjust = 0.1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;
	Order_Info orderInfo;
	double entryPrice;
	int maxTradeTime = 2, latestOrderIndex = 0, orderCountToday = 0;
	double upperBBand, lowerBBand;
	int trend4H = weeklyTrend4HSwingSignal(pParams, pIndicators, pBase_Indicators);
	int   dailyTrend;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	// ������ͼ��ʷ�����Ʒ�����
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;


	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 23;
	pIndicators->risk = 1.0;
	pIndicators->tpMode = 0;

	/* Check if trading is allowed (auto mode filter) */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPJPY_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	/* Get latest order side if exists */
	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	if (latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		side = pParams->orderInfo[latestOrderIndex].type;
	else
		side = NONE;

	/* BUY signal: MA trend up AND price closes above upper Bollinger Band */
	if (pBase_Indicators->maTrend > 0)
	{
		upperBBand = iBBands(B_PRIMARY_RATES, BBANDS_PERIOD, BBANDS_DEVIATIONS, BBANDS_UPPER_BAND, 1);

		logWarning("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


		if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - 1.0;
			/* Entry conditions: daily trend up, no wins, losses < maxTradeTime, no existing BUY order */
			if (dailyTrend > 0 &&
				pIndicators->winTimes == 0 && pIndicators->lossTimes < maxTradeTime && (side == SELL || side == NONE))
				pIndicators->entrySignal = 1;

			pIndicators->exitSignal = EXIT_SELL;
		}
	}

	/* SELL signal: MA trend down AND price closes below lower Bollinger Band */
	if (pBase_Indicators->maTrend < 0)
	{
		lowerBBand = iBBands(B_PRIMARY_RATES, BBANDS_PERIOD, BBANDS_DEVIATIONS, BBANDS_LOWER_BAND, 1);

		logWarning("System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

		if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + 1;
			if ( //trend4H == -1 &&
				dailyTrend < 0  &&				
				pIndicators->winTimes == 0 && pIndicators->lossTimes < maxTradeTime && (side == BUY || side == NONE))

				pIndicators->entrySignal = -1;

			pIndicators->exitSignal = EXIT_BUY;
		}
	}




	return SUCCESS;
}

/**
 * General multiple-day trading strategy execution.
 * 
 * This is an alias to workoutExecutionTrend_MultipleDay_V2.
 * Uses enterOrder_MultipleDay and modifyOrder_MultipleDay functions
 * from MultipleDayOrderManagement module for order entry and modification.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing MA trend and ATR
 * @return SUCCESS on success
 */
AsirikuyReturnCode workoutExecutionTrend_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	return workoutExecutionTrend_MultipleDay_V2(pParams, pIndicators, pBase_Indicators);
}

/**
 * GBPJPY multiple-day trading strategy execution (Current version).
 * 
 * Enhanced version with improved order management:
 * - Uses enterOrder_MultipleDay and modifyOrder_MultipleDay functions
 * - Entry based on Bollinger Bands breakout with risk adjustment
 * - Order modification based on profit targets and stop loss levels
 * - Handles both long-term and short-term orders
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing MA trend and ATR
 * @return SUCCESS on success
 */
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double ATR0;
	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;
	Order_Info orderInfo;
	double entryPrice;
	int maxTradeTime = 1, latestOrderIndex = 0, orderCountToday = 0;
	double upperBBand, lowerBBand;

	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	double range;


	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 22;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	pIndicators->stopLoss = 0.75;
	pIndicators->takePrice = 0.3;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		pIndicators->executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		pIndicators->executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		pIndicators->executionTrend = -1;
	else
		pIndicators->executionTrend = 0;

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPJPY_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

	//�������ڸߵ͵�
	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
	}
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	pIndicators->atr0 = fabs(intradayHigh - intradayLow);
	ATR0 = pIndicators->atr0;
	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	//1. check MA 5M
	//2. check BBand
	//3. if have open order, not 

	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	if (latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		side = pParams->orderInfo[latestOrderIndex].type;
	else
		side = NONE;

	if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
	{
		if (pParams->orderInfo[latestOrderIndex].type == BUY &&
			//pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 0
			pIndicators->executionTrend <= 0
			)
			pIndicators->exitSignal = EXIT_ALL;
		if (pParams->orderInfo[latestOrderIndex].type == SELL &&
			//pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 0
			pIndicators->executionTrend >= 0
			)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	if (side == NONE)
	{
		if (pBase_Indicators->maTrend > 0)
		{
			upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);

			logInfo("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


			if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
				if (pIndicators->winTimes == 0 && pIndicators->lossTimes < maxTradeTime && (side == SELL || side == NONE))
					pIndicators->entrySignal = 1;

				pIndicators->exitSignal = EXIT_SELL;
			}

		}

		if (pBase_Indicators->maTrend < 0)
		{
			lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);

			logInfo("System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

			if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
				if (pIndicators->winTimes == 0 && pIndicators->lossTimes < maxTradeTime && (side == BUY || side == NONE))

					pIndicators->entrySignal = -1;

				pIndicators->exitSignal = EXIT_BUY;
			}
		}
	}
	else
	{
		openOrderHigh = intradayHigh;
		openOrderLow = intradayLow;
		if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		{
			entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			if (side == SELL)
			{
				if (entryPrice - openOrderLow > pIndicators->takePrice &&
					entryPrice - pParams->bidAsk.ask[0] < pIndicators->takePrice
					)
				{
					//closeAllWithNegativeEasy(5, currentTime, 3); //close them intraday on break event
					closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
					logInfo("System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
					return SUCCESS;
				}

				if (pParams->bidAsk.ask[0] - openOrderLow >= pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
					pIndicators->exitSignal = EXIT_SELL;
				}
			}

			if (side == BUY)
			{
				//�������20��󣬳��ֵ�һ��5M�����ߣ��볡
				if (openOrderHigh - entryPrice > pIndicators->takePrice &&
					pParams->bidAsk.bid[0] - entryPrice < pIndicators->takePrice
					)
				{
					//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
					closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
					logInfo("System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
					return SUCCESS;
				}

				if (openOrderHigh - pParams->bidAsk.bid[0] >= pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
					pIndicators->exitSignal = EXIT_BUY;
				}
			}
		}
	}




	return SUCCESS;
}

/**
 * Multiple-day trading strategy execution (Version 2).
 * 
 * Enhanced version with additional features:
 * - Improved order management with floating take profit
 * - Better risk adjustment based on daily ATR and price gaps
 * - Support for both GBPJPY and GBPUSD filtering
 * - Enhanced order modification logic
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing MA trend and ATR
 * @return SUCCESS on success
 */
AsirikuyReturnCode workoutExecutionTrend_MultipleDay_V2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
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
	int maxTradeTime = 1, latestOrderIndex = 0, orderCountToday = 0, oldestOpenOrderIndex = 0;
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

	double times;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 30;
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

	//pIndicators->executionTrend = executionTrend;
	//�������ڸߵ͵�
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
	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	oldestOpenOrderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);

	if (oldestOpenOrderIndex >= 0){
		side = pParams->orderInfo[oldestOpenOrderIndex].type;

		safe_gmtime(&timeInfo2, pParams->orderInfo[oldestOpenOrderIndex].openTime);
		if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
			isSameDayOrder = TRUE;
		if (isSameDayOrder == TRUE && timeInfo2.tm_hour < pIndicators->startHour)
		{
			logWarning("System InstanceID = %d, BarTime = %s, same day opentime  %d is less than %d. It should be a manual take over order.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo2.tm_hour, pIndicators->startHour);
			isSameDayOrder = FALSE;
		}
	}
	else
		side = NONE;

	// TODO: ��Ҫ�޸�BASE,֧�����������¼���Trend��������Ҫʱ�䡣
	//��ʱѡ���ڵڶ��쿪���볡��ʵ�����е������⣬������ĩ���յ����⡣
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

	
	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{

		//pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3);
		//pIndicators->stopLoss = pIndicators->atr_euro_range*0.93;
		//pIndicators->takePrice = max(3, pIndicators->stopLoss * 0.4);

		pIndicators->stopLoss = pBase_Indicators->dailyATR * 0.4;
		pIndicators->atr_euro_range = pIndicators->stopLoss;
		pIndicators->takePrice = pIndicators->stopLoss;

		floatingTP = pIndicators->takePrice;
		logWarning("System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);
		

		// filter ��ũ
		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
		{

			strcpy(pIndicators->status, "Filter Non-farm day\n");

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		if (timeInfo1.tm_hour < 15)
			return SUCCESS;

		takeProfitMode = 1;

		riskCapBuy = 1;
		riskCapSell = 1;
	
	
	}

	if (side == NONE)
	{
		enterOrder_MultipleDay(pParams, pIndicators, pBase_Indicators, riskCapBuy, riskCapSell, FALSE);
	}
	else
	{
		if (getOrderCountEasy() == 1) 
		{

			if (side == BUY)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}

			if (side == SELL)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}

			if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && oldestOpenOrderIndex >= 0)
			{
				if (side == BUY)
				{
					times = (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice) / pIndicators->stopLoss;

					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice + ((int)(times - 1))* pIndicators->stopLoss;

				}

				if (side == SELL)
				{
					times = (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0]) / pIndicators->stopLoss;
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice - ((int)(times - 1))* pIndicators->stopLoss;
					
				}
			}
		}
		//modifyOrder_MultipleDay(pParams, pIndicators, pBase_Indicators, oldestOpenOrderIndex, intradayHigh, intradayLow, floatingTP, takeProfitMode, TRUE);
	}	

	return SUCCESS;
}

/* Note: Removed commented-out functions - duplicates are implemented in DayTradingStrategy.c */

static BOOL XAUUSD_DayTrading_Allow_Trade_Ver4(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int startTradingTime = pIndicators->startHour;
	//int startTradingTime = 2;
	double ATRWeekly0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	execution_tf = (int)pParams->settings[TIMEFRAME];


	// filter ��ũ
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	{

		logWarning("System InstanceID = %d, BarTime = %s, Filter Non-farm day",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

		return FALSE;
	}

	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
		return FALSE;

	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 10)
	{
		logWarning("System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
		return FALSE;
	}

	return TRUE;
}

