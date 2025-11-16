/*
 * Limit BBS Strategy Module
 * 
 * Provides Limit BBS strategy execution functions.
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
#include "strategies/autobbs/trend/limit/LimitBBSStrategy.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"

AsirikuyReturnCode workoutExecutionTrend_Limit_BBS(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int    orderIndex = -1;
	int    closeHour = 23, startHour = 2;
	int    stopHour = 22;
	BOOL   isEnableShellingtonTrend = FALSE;
	BOOL   isEnableTooFar = FALSE;
	BOOL   isEnableDoubleEntry = FALSE;
	BOOL   isEnableDoubleEntry2 = FALSE;
	BOOL   isEnableMACDSlow = TRUE;
	BOOL   isEnableFlatTrend = FALSE;
	BOOL   isCloseOrdersEOD = FALSE;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
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
	}

	// Additional variable declarations needed for the rest of the function
	struct tm timeInfo2;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double macdMaxLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE); 
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);	
	int trend = UNKNOWN;
	double realTakePrice;
	int exitMode = (int)parameter(AUTOBBS_IS_AUTO_MODE);

	// Re-initialize time variables after the if-else chain
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 4;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;
	
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

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	if (pIndicators->fast > 0
		&& pIndicators->fast > pIndicators->slow
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& pIndicators->fast < pIndicators->slow
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

	if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		pBase_Indicators->shellingtonInTrend = 1;

	if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		pBase_Indicators->shellingtonInTrend = -1;

	if (pBase_Indicators->dailyTrend > 0 && pBase_Indicators->mACDInTrend == 1
		)
	{
		trend = UP;
	}
	else if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1
		)
	{
		trend = DOWN;
	}
	else 
		trend = RANGE;

	// Need to set stop loss

	// AUTOBBS_RANGE=1 ATR Range order
	// AUTOBBS_STARTHOUR = 8 start hour
	// AUTOBBS_IS_AUTO_MODE = 1, use R1, S1 Stop order Stop order
	//AUTOBBS_IS_AUTO_MODE = 3, cancel all orders
	//AUTOBBS_RISK_CAP=2 : stopLossLevel, by default is 2

	//if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	//{
	//	
	//	isCloseOrdersEOD = TRUE;		
	//}
	//else if (strstr(pParams->tradeSymbol, "EURUSD") != NULL)
	//{
	//	
	//	isCloseOrdersEOD = TRUE;
	//}
	//else if (strstr(pParams->tradeSymbol, "EURGBP") != NULL)
	//{
	//	
	//	isCloseOrdersEOD = TRUE;
	//}
	//else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	//{		
	//	isCloseOrdersEOD = TRUE;
	//}


	pIndicators->takePrice = pBase_Indicators->dailyATR / 3;
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);
	//getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, pIndicators->executionRateTF * 60, orderIndex, &highPrice, &lowPrice);

	// By default isMoveTP = 1
	// But when AUTOBBS_IS_AUTO_MODE == 1, we should set isMoveTP = 0 because BBS30 will exit orders.
	if (isMoveTP == 1)
	{
		if (orderIndex >= 0)
		{
			realTakePrice = fabs(pParams->orderInfo[orderIndex].takeProfit - pParams->orderInfo[orderIndex].openPrice) / stopLossLevel;

			safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
				
			if (pParams->orderInfo[orderIndex].type == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= realTakePrice && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 2 * realTakePrice
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					exitMode = 0;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 2 * realTakePrice  && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * realTakePrice)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice;
					exitMode = 0;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 3 * realTakePrice)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + 2* realTakePrice;
					exitMode = 0;					
				}
			}

			if (pParams->orderInfo[orderIndex].type == SELL)
			{
				if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= realTakePrice && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * realTakePrice
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					exitMode = 0;
				}
				else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * realTakePrice && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * realTakePrice)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice;
					exitMode = 0;
				}
				else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * realTakePrice)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - 2 * realTakePrice;
					exitMode = 0;
				}
			}

		}
	}


	if (trend == UP)
	{
		
		//pIndicators->executionTrend = 1;
		if ((orderIndex < 0)
			&& pIndicators->fast < macdMaxLevel
			&& timeInfo1.tm_hour >= pIndicators->startHour
			&& pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index)
			splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);

		if (exitMode == 1 && pIndicators->bbsTrend_excution == -1)
			closeAllLongs();
		else
			closeAllShorts();
	}
	else if (trend == DOWN)
	{
		//pIndicators->executionTrend = -1;
		if ((orderIndex < 0) 
			&& pIndicators->fast > -1 * macdMaxLevel
			&& timeInfo1.tm_hour >= pIndicators->startHour
			&& pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index)
			splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);

		if (exitMode == 1 && pIndicators->bbsTrend_excution == 1)
			closeAllShorts();
		else
			closeAllLongs();
	}
	else
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}

		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
	}


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Limit_BBS_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double macdMaxLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);	
	int orderIndex = -1;
	double highPrice, lowPrice;
	BOOL isMoveTP = TRUE;	
	int closeHour = 23, startHour = 2;	
	int trend = UNKNOWN;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 31;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;

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

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	if (pIndicators->fast > 0
		&& pIndicators->fast > pIndicators->slow
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& pIndicators->fast < pIndicators->slow
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

	if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		pBase_Indicators->shellingtonInTrend = 1;

	if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		pBase_Indicators->shellingtonInTrend = -1;

	if (pBase_Indicators->dailyTrend > 0 && pBase_Indicators->mACDInTrend == 1
		)
	{
		trend = UP;
	}
	else if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1
		)
	{
		trend = DOWN;
	}
	else
		trend = RANGE;

	
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, pIndicators->executionRateTF * 60, orderIndex, &highPrice, &lowPrice);
	pIndicators->takePrice = pBase_Indicators->dailyATR;
	//pIndicators->takePrice = 0;
	logInfo("System InstanceID = %d, BarTime = %s, trend =%d, bbsTrend_excution=%d",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, trend, pIndicators->bbsTrend_excution);
	if (trend == UP)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = max(highPrice, pIndicators->entryPrice) - pBase_Indicators->pDailyMaxATR;		
		
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->fast < macdMaxLevel
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR/3)
			)
			pIndicators->entrySignal = 1;

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && pIndicators->bbsTrend_excution == -1)
			pIndicators->exitSignal = EXIT_ALL;
		else
			pIndicators->exitSignal = EXIT_SELL;
	}
	else if (trend == DOWN)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = min(lowPrice, pIndicators->entryPrice) + pBase_Indicators->pDailyMaxATR;

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->fast > -1 *  macdMaxLevel
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)
			)
			pIndicators->entrySignal = -1;

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && pIndicators->bbsTrend_excution == 1)
			pIndicators->exitSignal = EXIT_ALL;
		else
			pIndicators->exitSignal = EXIT_BUY;
	}
	else
		pIndicators->exitSignal = EXIT_ALL;


	return SUCCESS;
}

// workoutExecutionTrend_Limit_BreakOutOnPivot - stub implementation
// TODO: Implement full functionality based on original requirements
AsirikuyReturnCode workoutExecutionTrend_Limit_BreakOutOnPivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	// Temporary stub - returns SUCCESS to allow linking
	// This function should implement limit order breakout on pivot strategy
	logWarning("workoutExecutionTrend_Limit_BreakOutOnPivot: Stub implementation - not yet fully implemented");
	return SUCCESS;
}
