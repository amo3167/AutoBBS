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
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPJPY_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

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

	if (pBase_Indicators->maTrend > 0 )
	{
		upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);

		logWarning("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


		if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - 1;
			if ( //trend4H == 1 &&
				dailyTrend > 0 &&				
				pIndicators->winTimes == 0 && pIndicators->lossTimes <maxTradeTime && (side == SELL || side == NONE))
				pIndicators->entrySignal = 1;

			pIndicators->exitSignal = EXIT_SELL;
		}
	}

	if (pBase_Indicators->maTrend < 0)
	{
		lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);

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

// workoutExecutionTrend_MultipleDay is an alias to workoutExecutionTrend_MultipleDay_V2
AsirikuyReturnCode workoutExecutionTrend_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	return workoutExecutionTrend_MultipleDay_V2(pParams, pIndicators, pBase_Indicators);
}

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


/*
���ռ佻�׷�
�볡��
1.���µ�һ�죬if no existing open order, ���̺�,�����AӋ�������_���� ��limit orders.
2.������1%,two orders. One order is no TP. the other one with TP.
3.ֹ����0.40 * ATR(20)��
4.ֹӯ��0.40 * ATR(20)��

������
1. If TP order is closed, the no TP order will continue. and move SL on long term order to break event. And keep moving SL when 1 SL level(0.4 * ATR(20))
2. If both orders are closed. It will be back to clean state.
3. After that, if it is a strong move day, will enter pending orders on 1/2 projected ATR level. 

*/
//AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
//{
//	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
//	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
//	time_t currentTime;
//	struct tm timeInfo1;
//	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
//	double ATR0, Range, ATRWeekly0;
//	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1);
//	double openOrderHigh, openOrderLow;
//	int count, isOpen;	
//	char       timeString[MAX_TIME_STRING_SIZE] = "";
//	OrderType side;
//
//	Order_Info orderInfo;
//
//	double entryPrice;
//	int maxTradeTime = 2;
//	
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
//	safe_gmtime(&timeInfo1, currentTime);
//
//
//	safe_timeString(timeString, currentTime);
//
//	pIndicators->splitTradeMode = 21;
//	pIndicators->risk = 1;
//	pIndicators->tpMode = 0;
//
//	pBase_Indicators->maTrend = getMATrend(2, B_SECONDARY_RATES, 1);
//
//	Range = pBase_Indicators->dailyATR / 2;
//
//
//	logInfo("System InstanceID = %d, BarTime = %s, dailyATR = %lf",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->dailyATR);
//
//	// Start from 1AM
//	count = (timeInfo1.tm_hour-1)  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
//	if (count > 1)
//	{
//		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
//	}
//	else
//		return SUCCESS;
//
//	intradayLow = min(close_prev1, intradayLow);
//	intradayHigh = max(close_prev1, intradayHigh);
//	ATR0 = fabs(intradayHigh - intradayLow);
//
//	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);
//
//	// secondary rate is 5M , priarmy rate is 1M
//	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)
//	{
//		if (pParams->bidAsk.ask[0] - intradayLow >= Range)
//		{
//			pIndicators->executionTrend = 1;
//			pIndicators->entryPrice = pParams->bidAsk.ask[0];
//			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
//			pIndicators->entrySignal = 1;
//		}
//
//		if (intradayHigh - pParams->bidAsk.bid[0] >= Range)
//		{
//			pIndicators->executionTrend = -1;
//			pIndicators->entryPrice = pParams->bidAsk.bid[0];
//			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
//			pIndicators->entrySignal = -1;
//		}
//
//	}
//	else
//	{
//		side = getLastestOpenOrderTypeEasy_GBPJPY(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow);
//		if (side == SELL)
//		{
//
//			if (pParams->bidAsk.ask[0] - openOrderLow >= Range)				
//			{
//				pIndicators->executionTrend = 1;
//				pIndicators->entryPrice = pParams->bidAsk.ask[0];
//				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
//
//				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
//				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
//				if (pIndicators->lossTimes < 3 && pIndicators->winTimes == 0)
//				{
//					//pIndicators->risk = pow(2, pIndicators->lossTimes);
//					pIndicators->entrySignal = 1;
//				}
//				pIndicators->exitSignal = EXIT_SELL;
//			}
//
//		}
//
//		if (side == BUY)
//		{
//
//			if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)				
//			{
//				pIndicators->executionTrend = -1;
//				pIndicators->entryPrice = pParams->bidAsk.bid[0];
//				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
//				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
//				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
//				if (pIndicators->lossTimes < 3 && pIndicators->winTimes == 0)
//				{
//					//pIndicators->risk = pow(2, pIndicators->lossTimes);
//					pIndicators->entrySignal = -1;
//				}
//
//				pIndicators->exitSignal = EXIT_BUY;
//			}
//
//		}
//
//	}
//
//
//
//	return SUCCESS;
//}
//

/*
MACD BEILI: ��Ҫ���ڳ�������ʱ����ཻ�ף����������1
Buy:
Entry:
1. The highest MCAD (10) in the last 5 days, > limit
2. MACD(5) < MACD(10) : cross down
3. Win/Loss >=1
4. Stop loss is the highest price in the last 5 days
5. Take price is just greater than ichimoki(slow)

Exit:
1. MACD cross up
2. Hit the moving stop loss

Short:
Entry: 
1. The lowest MCAD (10) in the last 5 days, < limit
2. MACD(5) > MACD(10) : cross up
3. Win/Loss >=1
4. Stop loss is the lowest price in the last 5 days
5. Take price is just less than ichimoki(slow)

Exit:
1. MACD cross down
2. Hit the moving stop loss
*/



// workoutExecutionTrend_DayTrading_ExecutionOnly_Old is implemented in DayTradingStrategy.c - removed duplicate
#if 0
AsirikuyReturnCode workoutExecutionTrend_DayTrading_ExecutionOnly_Old(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0, Range;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.03;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;

	Order_Info orderInfo;

	double entryPrice;

	int latestOrderIndex = 0, orderCountToday = 0;
		
	int euro_index_rate;
	double ATREuroPeriod = 0.0;

	BOOL hasCurrentDayOrder = FALSE;

	int asia_index_rate = 0;
	double asia_ATR = 0;
	int executionTrend;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 29;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		executionTrend = -1;
	else
		executionTrend = 0;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		pIndicators->stopLoss = pIndicators->atr_euro_range;		
		pIndicators->takePrice = pIndicators->stopLoss * 0.6;
		pIndicators->startHour = 9;
		pIndicators->endHour = 15;
	}

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	Range = pIndicators->stopLoss;
	
	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{
		if (hasOpenOrder())
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	if (timeInfo1.tm_hour < pIndicators->startHour)
		return SUCCESS;

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);
	

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

	if ((timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30) || pIndicators->winTimes > 0)
	{
		if (hasOpenOrder()==TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	asia_index_rate = shift1Index_primary - ((timeInfo1.tm_hour - pIndicators->startHour) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]));

	count = (pIndicators->startHour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);
		
	asia_ATR = fabs(pIndicators->asia_high - pIndicators->asia_low);

	hasCurrentDayOrder = hasSameDayOrderEasy(currentTime, &isOpen);
	if(hasCurrentDayOrder == TRUE && isOpen == FALSE)
		return SUCCESS;


	if (hasCurrentDayOrder == FALSE)
	{

		////����5M���ߵ�֧��
		//if (pParams->bidAsk.ask[0] - intradayLow >= Range && intradayHigh - pParams->bidAsk.bid[0] < Range 
		//	&& ( (asia_ATR > Range && iClose(B_DAILY_RATES, 0) - iOpen(B_DAILY_RATES, 0) > 0)
		//	|| asia_ATR <= Range)
		//	&& executionTrend > 0 						
		//	&& timeInfo1.tm_hour <= pIndicators->endHour			
		//	)
		//{
		//	pIndicators->executionTrend = 1;
		//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
		//	pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
		//	pIndicators->entrySignal = 1;
		//}

		if (pParams->bidAsk.ask[0] - intradayLow >= Range && intradayHigh - pParams->bidAsk.bid[0] < Range			
			&& executionTrend == 1
			&& pBase_Indicators->maTrend > 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = intradayLow;
			pIndicators->entrySignal = 1;
		}

		//if (intradayHigh - pParams->bidAsk.bid[0] >= Range 	&& pParams->bidAsk.ask[0] - intradayLow < Range 
		//	&&(( asia_ATR > Range && iClose(B_DAILY_RATES, 0) - iOpen(B_DAILY_RATES, 0) < 0)
		//	|| asia_ATR <= Range )			
		//	&& executionTrend < 0 
		//	&& timeInfo1.tm_hour <= pIndicators->endHour			
		//	)
		//{
		//	pIndicators->executionTrend = -1;
		//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
		//	pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
		//	pIndicators->entrySignal = -1;
		//}

		if (intradayHigh - pParams->bidAsk.bid[0] >= Range 	&& pParams->bidAsk.ask[0] - intradayLow < Range			
			&& executionTrend == 0 && pBase_Indicators->maTrend < 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = intradayHigh;
			pIndicators->entrySignal = -1;
		}
	}
	else
	{

		side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);

		//����Ѿ�����һ��trade���Ǿ��ǵڶ��ν��ף�Ҫʹ�����ڵĸߵ͵�
		//�������2��trades,�Ǿ��ǵ����ν����ˣ�ʹ��orderHigh, orderLow.
		orderCountToday = getOrderCountTodayEasy(currentTime);
		//orderCountToday = getOrderCountTodayExcludeBreakeventOrdersEasy(currentTime,0.1);
		if (orderCountToday == 1)
		{
			openOrderHigh = intradayHigh;
			openOrderLow = intradayLow;

		}
				
		if (orderCountToday >= 1)
		{
			//entryPrice = getLastestOrderPriceEasy(B_PRIMARY_RATES, &isOpen);
			//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			//����Բ�����ͬʱ�������open order,��������index,Ӧ�ö���0			 
			//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)


			if (isOpen == TRUE)
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
						logWarning("System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
						return SUCCESS;
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
						logWarning("System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
						return SUCCESS;
					}
				}
			}
		}

		if (side == SELL)
		{

			if (isOpen == TRUE)
			{
				if (orderCountToday == 2)
				{
					openOrderLow = min(openOrderLow, pParams->orderInfo[latestOrderIndex].openPrice);
				}


				if (pParams->bidAsk.ask[0] - openOrderLow >= Range)					
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0)
					{
						//pIndicators->risk = pow(2, pIndicators->lossTimes);
						//pIndicators->entrySignal = 1;
					}
					pIndicators->exitSignal = EXIT_SELL;
				}
			}

		}

		if (side == BUY)
		{
			if (isOpen == TRUE)
			{

				if (orderCountToday == 2)
				{
					openOrderHigh = max(openOrderHigh, pParams->orderInfo[latestOrderIndex].openPrice);
				}

				if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)					
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;					
					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0)
					{
						//pIndicators->risk = pow(2, pIndicators->lossTimes);
						//pIndicators->entrySignal = -1;
					}

					pIndicators->exitSignal = EXIT_BUY;
				}

			}
		}

	}

	return SUCCESS;
}
#endif

#if 0
// workoutExecutionTrend_DayTrading_ExecutionOnly is implemented in DayTradingStrategy.c - removed duplicate
AsirikuyReturnCode workoutExecutionTrend_DayTrading_ExecutionOnly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0, Range;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.03;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;

	Order_Info orderInfo;

	double entryPrice;

	int latestOrderIndex = 0, orderCountToday = 0;

	int euro_index_rate;
	double ATREuroPeriod = 0.0;

	BOOL hasCurrentDayOrder = FALSE;

	int asia_index_rate = 0;
	double asia_ATR = 0;
	int executionTrend;

	double upperBBand, lowerBBand;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 29;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		executionTrend = -1;
	else
		executionTrend = 0;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		pIndicators->stopLoss = pIndicators->atr_euro_range;
		pIndicators->takePrice = pIndicators->stopLoss * 0.6;
		pIndicators->startHour = 9;
		pIndicators->endHour = 15;
	}

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	Range = pIndicators->stopLoss;

	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{
		if (hasOpenOrder())
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	if (timeInfo1.tm_hour < pIndicators->startHour)
		return SUCCESS;

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);


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

	if ((timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30) || pIndicators->winTimes > 0)
	{
		if (hasOpenOrder() == TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	asia_index_rate = shift1Index_primary - ((timeInfo1.tm_hour - pIndicators->startHour) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]));

	count = (pIndicators->startHour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	asia_ATR = fabs(pIndicators->asia_high - pIndicators->asia_low);

	hasCurrentDayOrder = hasSameDayOrderEasy(currentTime, &isOpen);
	if (hasCurrentDayOrder == TRUE && isOpen == FALSE)
		return SUCCESS;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	if (pIndicators->fast > 0 && pIndicators->fast > pIndicators->preFast && pIndicators->fast > pIndicators->slow)
		executionTrend = 1;
	else if (pIndicators->fast < 0 && pIndicators->fast < pIndicators->preFast && pIndicators->fast < pIndicators->slow)
		executionTrend = -1;
	else
		executionTrend = 0;

	if (hasCurrentDayOrder == FALSE)
	{

		
		if (
			pParams->bidAsk.ask[0] - intradayLow >= Range && intradayHigh - pParams->bidAsk.bid[0] < Range && 
			executionTrend == 1
			&& pBase_Indicators->maTrend > 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);

			logWarning("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


			if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = intradayLow;
				pIndicators->entrySignal = 1;
			}
		}

		
		if (
			intradayHigh - pParams->bidAsk.bid[0] >= Range 	&& pParams->bidAsk.ask[0] - intradayLow < Range	&& 
			executionTrend == -1 && pBase_Indicators->maTrend < 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);

			logWarning("System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

			if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = intradayHigh;
				pIndicators->entrySignal = -1;
			}
		}
	}
	

	return SUCCESS;
}
#endif

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

