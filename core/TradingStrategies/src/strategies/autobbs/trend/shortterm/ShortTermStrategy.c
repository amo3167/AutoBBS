/*
 * ShortTerm Strategy Module
 * 
 * Provides ShortTerm strategy execution functions.
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

AsirikuyReturnCode workoutExecutionTrend_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1,timeInfo2;
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
	double stopLossLevel = 2.5;
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1;
	double highPrice, lowPrice;
	BOOL isMoveTP = TRUE;	
	int closeHour = 23, startHour = 2;	
	int trend = UNKNOWN;
	double ma20M,ma50M, ma200M;
		
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 31;	
	pIndicators->risk = 1;
	pIndicators->tpMode = 4;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	ma20M = iMA(3, B_PRIMARY_RATES, 20, 1);
	ma50M = iMA(3, B_PRIMARY_RATES, 50, 1);
	ma200M = iMA(3, B_PRIMARY_RATES, 200, 1);


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
		&& preDailyClose > dailyBaseLine		
		&& pIndicators->fast > pIndicators->slow
		&& pIndicators->fast > pIndicators->preFast
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& preDailyClose < dailyBaseLine	
		&& pIndicators->fast < pIndicators->slow
		&& pIndicators->fast < pIndicators->preFast
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
		//if (pBase_Indicators->shellingtonInTrend == 1)
		//{
		//	pIndicators->risk = 2;
		//}
	}
	if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1			
		)
	{
		trend = DOWN;
		//if (pBase_Indicators->shellingtonInTrend == -1)
		//{
		//	pIndicators->risk = 2;
		//}
	}

	if (pBase_Indicators->dailyTrend_Phase == 0)
		trend = RANGE;


	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, orderIndex,5*60,&highPrice, &lowPrice);

	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;
	pIndicators->executionTrend = 0;

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	//{
	//	if (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->ma_Signal < 0 && getOrderCountEasy() > 1)
	//	{
	//		pIndicators->exitSignal = EXIT_BUY;
	//		return SUCCESS;
	//	}
	//	if (pParams->orderInfo[orderIndex].type == SELL &&  pBase_Indicators->ma_Signal > 0 && getOrderCountEasy() > 1)
	//	{
	//		pIndicators->exitSignal = EXIT_SELL;
	//		return SUCCESS;
	//	}
	//	
	//}

	if (trend == UP)
	{		
	
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		if (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			if (getMATrend_SignalBase(20, 50, B_PRIMARY_RATES, 24)> 0 && pIndicators->entryPrice > ma200M)
			{					
				splitBuyOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, pBase_Indicators->dailyATR/2, pBase_Indicators->pDailyMaxATR);
			}
		}
		else
		{
			if (getOrderCountEasy() == 1)
			{
				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				modifyTradeEasy_new(BUY, -1, fabs(pIndicators->entryPrice - pIndicators->stopLossPrice), -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		
		pIndicators->exitSignal = EXIT_SELL;		
			
	}
	else if (trend == DOWN)
	{		
		pIndicators->entryPrice = pParams->bidAsk.bid[0];	
	
		if (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			if (getMATrend_SignalBase(20, 50, B_PRIMARY_RATES, 24) < 0 && pIndicators->entryPrice < ma200M)
			{				
				splitSellOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, pBase_Indicators->dailyATR / 2, pBase_Indicators->pDailyMaxATR);
			}
		}
		else
		{
			if (getOrderCountEasy() == 1)
			{
				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				modifyTradeEasy_new(SELL, -1, fabs(pIndicators->entryPrice - pIndicators->stopLossPrice), -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		
		pIndicators->exitSignal = EXIT_BUY;
			
	}
	else{
		pIndicators->exitSignal = EXIT_ALL;
	}


	return SUCCESS;
}
