/*
 * Limit Strategy Module
 * 
 * Provides Limit strategy execution functions.
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
#include "strategies/autobbs/trend/limit/LimitStrategy.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"
#include "strategies/autobbs/trend/common/StopLossManagement.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"  // For splitBuyRangeOrders, splitSellRangeOrders

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
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose,preDailyHigh,preDailyLow,preDailyOpen;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double moveTPLimit = (double)parameter(AUTOBBS_KEYK);
	int autoMode = (int)parameter(AUTOBBS_IS_AUTO_MODE);	
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1;
	double highPrice, lowPrice;
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);
	BOOL isMoveTPInNewDay = TRUE;
	int closeHour = 23, startHour = pIndicators->startHour;
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
	int stopHour = 23;		
	double tooFarLimit = 1;
	int barState = BAR_UNKNOWN;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;
	double rsi = 0.0, rsiLow = 20.0, rsiHigh = 80.0;
	int tradingDays = 10;
	BOOL isEnableRSI = FALSE;
	BOOL isEnableDoubleEntry = FALSE, isEnableDoubleEntry2 = FALSE;
	int rangeType = 0;
	int atrTimes = 20;
	BOOL isEnableTooBigSpread = FALSE;
	double ma960M = 0.0;
	int count = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 4;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;
	pIndicators->isEnableLimitSR1 = FALSE;

	pIndicators->takePriceLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);

	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, 5 * 60, orderIndex, &highPrice, &lowPrice);

	pIndicators->adjust = fabs(pParams->bidAsk.bid[0] - pParams->bidAsk.ask[0]);	

	count = timeInfo1.tm_hour * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);

	ma960M = iMA(3, B_PRIMARY_RATES, 960, 1+ count);
	
	if (timeInfo1.tm_wday == 0)
		atrTimes = 50;

	logInfo("System InstanceID = %d, BarTime = %s,ask=%lf,bid=%lf,adjsut=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,pParams->bidAsk.ask[0], pParams->bidAsk.bid[0], pIndicators->adjust);


	if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		startHour = 3;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		
		isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
	}
	else if (strstr(pParams->tradeSymbol, "USDJPY") != NULL)
	{
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		isEnableMACDSlow = FALSE;

	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		if (timeInfo1.tm_wday == 3)
			pIndicators->risk = 0.6;

		startHour = 3;

		//isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
		isCloseOrdersEOD = TRUE;
		isEnableRSI = TRUE; // Not confirm it will be much better. Need manully control the risk if RSI or too much??
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{		

		if (timeInfo1.tm_wday == 3)
			pIndicators->risk = 0.6;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;
		
		startHour = 3;
		pIndicators->startHourOnLimt = 8;

		stopHour = 22;

		// filter non-farm payroll day
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

		if (timeInfo1.tm_wday == 4)
			pIndicators->risk = 0.5;

		isEnableShellingtonTrend = TRUE;
		pIndicators->isEnableLimitSR1 = TRUE;
		isEnableTooFar = FALSE; 
	
		isEnableDoubleEntry = TRUE;

		isEnableDoubleEntry2 = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "EURGBP") != NULL)
	{
		startHour = 3;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		isEnableMACDSlow = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, "EURUSD") != NULL)
	{
		startHour = 3;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		isEnableMACDSlow = FALSE;

		isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
	}
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	{
		pIndicators->adjust = pBase_Indicators->dailyATR * 0.01;

		startHour = 0;
		pIndicators->startHourOnLimt = startHour;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		//if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 6)
		//	pIndicators->risk = 0.6;

		if (timeInfo1.tm_wday == 2 || timeInfo1.tm_wday == 3 || timeInfo1.tm_wday == 4)
			pIndicators->risk = 0.5;

		if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 1)
		{
			if (fabs(pBase_Indicators->dailyTrend) >= 6
				&& iAtr(B_DAILY_RATES, 1, 1) < 0.7 * pBase_Indicators->pDailyATR
				)
				pIndicators->risk = 0.5;
		}

		//if (timeInfo1.tm_wday == 6 && timeInfo1.tm_hour == 16 && timeInfo1.tm_min >= 50)
		//{
		//	closeAllLimitAndStopOrdersEasy(currentTime);			
		//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		//	return SUCCESS;
		//}

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}
		//if (iAtr(B_DAILY_RATES, 1, 1) >= pBase_Indicators->dailyATR * 2)
		//{
		//	pIndicators->risk = 0.6;
		//}
		//if (fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) >= pBase_Indicators->pWeeklyPredictATR / 3)
		//{
		//	strcpy(pIndicators->status, "Previous day Close is more than pWeeklyPredictATR / 3.\n");

		//	logWarning("System InstanceID = %d, BarTime = %s, %s",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		//	return SUCCESS;
		//}

		isEnableWeeklyATR = FALSE;

		pIndicators->isEnableLimitSR1 = TRUE;

		//if (timeInfo1.tm_wday == 6)
		//	stopHour = 16;

		tooFarLimit = 2;

		isEnableRangeTrade = FALSE;

		isEnableDoubleEntry = TRUE;

		isEnableTooFar = TRUE;

		fastMAPeriod = 7;
		slowMAPeriod = 14;
		signalMAPeriod = 7;

		tradingDays = 14;
	}
	else if (strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		if (fabs(pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]) > pIndicators->adjust * 1.5){

			isEnableTooBigSpread = TRUE;
		}
		
		pIndicators->adjust = pBase_Indicators->dailyATR * 0.01;

		startHour = 0;
		pIndicators->startHourOnLimt = startHour;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		//if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 6)
		//	pIndicators->risk = 0.6;

		if (timeInfo1.tm_wday == 2 || timeInfo1.tm_wday == 4)
			pIndicators->risk = 0.5;

		if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 1)
		{
			if (fabs(pBase_Indicators->dailyTrend) >= 6
				&& iAtr(B_DAILY_RATES, 1, 1) < 0.7 * pBase_Indicators->pDailyATR
				)
				pIndicators->risk = 0.5;
		}

		//if (timeInfo1.tm_wday == 6 && timeInfo1.tm_hour == 16 && timeInfo1.tm_min >= 50)
		//{
		//	closeAllLimitAndStopOrdersEasy(currentTime);			
		//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		//	return SUCCESS;
		//}

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}
		//if (iAtr(B_DAILY_RATES, 1, 1) >= pBase_Indicators->dailyATR * 2)
		//{
		//	pIndicators->risk = 0.6;
		//}
		//if (fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) >= pBase_Indicators->pWeeklyPredictATR / 3)
		//{
		//	strcpy(pIndicators->status, "Previous day Close is more than pWeeklyPredictATR / 3.\n");

		//	logWarning("System InstanceID = %d, BarTime = %s, %s",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		//	return SUCCESS;
		//}

		isEnableWeeklyATR = FALSE;

		pIndicators->isEnableLimitSR1 = TRUE;

		//if (timeInfo1.tm_wday == 6)
		//	stopHour = 16;

		tooFarLimit = 2;

		isEnableRangeTrade = FALSE;

		isEnableDoubleEntry = TRUE;

		isEnableTooFar = TRUE;

		fastMAPeriod = 7;
		slowMAPeriod = 14;
		signalMAPeriod = 7;

		tradingDays = 14;


	

	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{		
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		stopHour = 17;

		isEnableMACDSlow = FALSE;
		isEnableFlatTrend = TRUE;
		isEnableTooFar = TRUE;
		//isEnableRangeTrade = TRUE;
		pIndicators->startHourOnLimt = pIndicators->startHour;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE)		
		pIndicators->adjust = 0;	
		

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
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

	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	//if (preDailyClose - preDailyOpen > (preDailyHigh - preDailyLow) / 3)
	//	barState = BULL;
	//else if (preDailyOpen - preDailyClose > (preDailyHigh - preDailyLow) / 3)
	//	barState = BEAR;
	//else if (preDailyClose > preDailyOpen && preDailyOpen - preDailyLow > (preDailyHigh - preDailyLow) * 0.66)
	//	barState = BULL;
	//else if (preDailyClose < preDailyOpen && preDailyHigh - preDailyOpen >(preDailyHigh - preDailyLow) * 0.66)
	//	barState = BEAR;
	//else if (fabs((preDailyHigh - preDailyLow) / 2 - (preDailyClose - preDailyOpen) / 2) < (preDailyHigh - preDailyLow) / 10)
	//	barState = STAR;

	//if (barState == STAR)
	//{
	//	if (daily_baseline_short - preDailyClose >=  2 * pBase_Indicators->dailyATR)
	//		barState = BULL;
	//	if (preDailyClose - daily_baseline_short >=  2 * pBase_Indicators->dailyATR)
	//		barState = BEAR;
	//}

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

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	pBase_Indicators->shellingtonInTrend = trend_4H;

	//if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
	//	pBase_Indicators->shellingtonInTrend = 1;

	//if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
	//	pBase_Indicators->shellingtonInTrend = -1;

	if (fabs(iMA(3, B_DAILY_RATES, 20, 1) - iMA(3, B_DAILY_RATES, 20, 5)) / pBase_Indicators->dailyATR <= 0.05)
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
	//else if (pBase_Indicators->dailyTrend == 0 && pBase_Indicators->mACDInTrend == 0 && pBase_Indicators->shellingtonInTrend == 0)
	//	trend = RANGE;
	else
		trend = RANGE;

	//Override pBase_Indicators->maTrend if too close	
	//pBase_Indicators->maTrend = getMATrend(pIndicators->adjust, B_PRIMARY_RATES, 1);
	//if (pBase_Indicators->maTrend == 1)
	//{
	//	if (pIndicators->bbsTrend_secondary <= 0)
	//		pBase_Indicators->maTrend = 0;
	//}
	//if (pBase_Indicators->maTrend == -1)
	//{
	//	if (pIndicators->bbsTrend_secondary >= 0)
	//		pBase_Indicators->maTrend = 0;
	//}
		
	// Need to set stop loss

	// AUTOBBS_RANGE=1 ATR Range order
	// AUTOBBS_STARTHOUR = 8 start hour
	// AUTOBBS_IS_AUTO_MODE = 1, use R1, S1 Stop order Stop order
	//AUTOBBS_IS_AUTO_MODE = 3, cancel all orders
	//AUTOBBS_RISK_CAP=2 : stopLossLevel, by default is 2

	pIndicators->takePrice = iAtr(B_HOURLY_RATES, atrTimes, 1);
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	////////////////////
	//Check pending order and save in the virutal ordder info

	//readVirtualOrderInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& (pParams->orderInfo[orderIndex].type == SELL || pParams->orderInfo[orderIndex].type == BUY)
	//	&& orderInfo.ticket > 0
	//	&& orderInfo.ticket != pParams->orderInfo[orderIndex].ticket
	//	&& orderInfo.openPrice == 
	//	)
	//{
	//	saveVirutalOrdergInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->orderInfo[orderIndex]);	
	//}

	if (isEnableRSI && (rsi >= rsiHigh || rsi <= rsiLow) 
		//&& (pIndicators->fast >= 0.012 || pIndicators->fast <=-0.012)
		)
	{
		logInfo("System InstanceID = %d, BarTime = %s,skip trading on rsi=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, rsi);

		return SUCCESS;
		//pIndicators->risk = 0.5;
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

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 3 || timeInfo1.tm_hour >= stopHour)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		if (isCloseOrdersEOD == TRUE && timeInfo1.tm_hour == 23 && timeInfo1.tm_min >= 50)
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

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& ((pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0) 
	//	|| (pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)))
	//{		
	//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	//	return SUCCESS;
	//}


	logInfo("System InstanceID = %d, BarTime = %s,startHour=%d,AUTOBBS_IS_AUTO_MODE=%d,isEnableRangeTrade=%d,pBase_dailyHigh=%lf,dailyLow=%lf,pDailyMaxATR=%lf,hourATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->startHour, (int)parameter(AUTOBBS_IS_AUTO_MODE), (int)parameter(AUTOBBS_RANGE), iHigh(B_DAILY_RATES, 0), iLow(B_DAILY_RATES, 0), pBase_Indicators->pDailyMaxATR, iAtr(B_HOURLY_RATES, 20, 1));
	

	totalLossTimes = getLossTimesInDayCloseOrderEasy(currentTime, &totalLossPoint);
	if (totalLossTimes >= 2)
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
			//	&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, 20, 1)) < 1
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
			//	&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, 20, 1)) < 1
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
		else if (trend == RANGE  && isEnableRangeTrade == TRUE)
		{
			pIndicators->risk = 0.5;
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

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL 
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen		
		&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 9 * 60)
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		//&& timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour <= 16
		)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		if (timeInfo2.tm_hour >= 15 && timeInfo2.tm_hour <= 16)		
		{
			if (timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour <= 16)
			{
				move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, 1.0);
			}
			else if (timeInfo1.tm_hour > 16)
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
