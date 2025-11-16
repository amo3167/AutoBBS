/*
 * BBS Swing Strategy Module
 * 
 * Provides BBS Swing strategy execution functions.
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

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

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

	//4H filter
	if ((timeInfo1.tm_hour-1) % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 20;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		movement = fabs(high_4H - low_4H);

		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;

		logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // Should be based on current market conditions, 40% weekly ATR
		{
			if (fabs(high_4H - close_4H) < movement / 3)
			{
				trend_KeyK = 1;
			}
			if (fabs(low_4H - close_4H) < movement / 3)
			{
				trend_KeyK = -1;
			}
		}

		if (trend_MA > 0 || trend_KeyK == 1)
			trend_4H = 1;
		if (trend_MA < 0 || trend_KeyK == -1)
			trend_4H = -1;
		// Enter order on key support/resistance levels

		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - 20);

			//side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			
			if( (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY) ))
			{
				pIndicators->entrySignal = 1;
			}
	
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;

			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + 20);

			//side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			if ( (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL) ))
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;

		}

		if (pIndicators->entrySignal != 0 &&
			fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) > pBase_Indicators->pWeeklyPredictATR
			)
		{
			sprintf(pIndicators->status, "current weekly price gap %lf is greater than pWeeklyPredictATR %lf",
				fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice), pBase_Indicators->pWeeklyPredictATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}

		//profitManagement_base(pParams, pIndicators, pBase_Indicators);
	}
	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

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

	if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		pIndicators->stopLoss = 2.5;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		pIndicators->stopLoss = pBase_Indicators->pWeeklyPredictMaxATR;
	}
	//4H filter
	if (timeInfo1.tm_hour % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 20;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		movement = fabs(high_4H - low_4H);

		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;		

		logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // Should be based on current market conditions, 40% weekly ATR
		{
			if (fabs(high_4H - close_4H) < movement / 3)
			{
				trend_KeyK = 1;
			}
			if (fabs(low_4H - close_4H) < movement / 3)
			{
				trend_KeyK = -1;
			}
		}

		if (trend_MA > 0 || trend_KeyK == 1)
			trend_4H = 1;
		if (trend_MA < 0 || trend_KeyK == -1)
			trend_4H = -1;
		// Enter order on key support/resistance levels
		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pIndicators->stopLoss);
				
			// First turning point
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY))
			{
				pIndicators->entrySignal = 1;
			}
			
			//// First order in trend, take 1/3
			//if (pIndicators->bbsIndex_4H == shift1Index_4H
			//	&& pIndicators->entrySignal == 0 
			//	&& pParams->orderInfo[orderIndex].isOpen == FALSE &&  getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < 3)
			//{
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
	
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pIndicators->stopLoss);

			// First turning point
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL))
			{
				pIndicators->entrySignal = -1;
			}
			
			//// First order in trend, take 1/3
			//if (pIndicators->bbsIndex_4H == shift1Index_4H
			//	&& pIndicators->entrySignal == 0 
			//	&& pParams->orderInfo[orderIndex].isOpen == FALSE && getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < 3)
			//{
			//	pIndicators->entrySignal = -1;
			//}


			pIndicators->exitSignal = EXIT_BUY;

		}

		//profitManagement_base(pParams, pIndicators, pBase_Indicators);
	}
	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

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

	
	//closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//pIndicators->splitTradeMode = 0;
	//pIndicators->tpMode = 1;

	// ATR mode
	pIndicators->splitTradeMode = 19;
	pIndicators->tpMode = 3;

	//if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
	//	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
		
	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	movement = fabs(high_4H - low_4H);
	if (pIndicators->atr_euro_range == 0)
		pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;
	
	logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


	if (movement >= pIndicators->atr_euro_range) // 100 pips
	{
		if (fabs(high_4H - close_4H) < movement / 3)
		{
			trend_KeyK = 1;
		}
		if (fabs(low_4H - close_4H) < movement / 3)
		{
			trend_KeyK = -1;
		}
	}

	if (trend_MA > 0 || trend_KeyK == 1)
		trend_4H = 1;
	if (trend_MA < 0 || trend_KeyK == -1)
		trend_4H = -1;
	// Enter order on key support/resistance levels

	
	if (pIndicators->bbsTrend_4H == 1)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pBase_Indicators->dailyS);
		
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			//&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4, currentTime)
			&& trend_4H == 1
			//&& fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = 1;
		}

		//if (pIndicators->entrySignal == 0 && pIndicators->bbsIndex_4H == shift1Index_4H
		//	&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4, currentTime)
		//	&& trend_MA < 0 && trend_KeyK == 1)
		//{
		//	pIndicators->entrySignal = 1;			
		//}

		// Stop loss:
		// If daily trend is range, reduce position size
		if (dailyTrend == 0
			&& pIndicators->entryPrice >= pBase_Indicators->dailyS
			&& fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) >= pBase_Indicators->dailyATR * 0.5)
			pIndicators->risk = 0.5;

		if (pIndicators->stopLossPrice > pIndicators->entryPrice) //invalid price. current price is above stop loss price. No entry trade.
			pIndicators->entrySignal = 0;

		//// If weekly and daily trend not consistent, should stop? Use 50% risk?
		//if (dailyTrend < 0)
		//	pIndicators->risk = 0.5;

		// Follow 4H trend change.
		//pIndicators->exitSignal = -1;	


	}

	if (pIndicators->bbsTrend_4H == -1)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		if (dailyTrend == 0)
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyR);
		else
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyS);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			//&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4, currentTime)
			&& trend_4H == -1
			//&& fabs(iHigh(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = -1;
		}



		if (dailyTrend == 0 && pIndicators->entryPrice <= pBase_Indicators->dailyR
			&& fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) >= pBase_Indicators->dailyATR * 0.5)
			pIndicators->risk = 0.5;

		//// If weekly and daily trend not consistent, should stop? Use 50% risk?
		//if (dailyTrend > 0)
		//	pIndicators->entrySignal = 0;


		if (pIndicators->stopLossPrice < pIndicators->entryPrice) //invalid price. current price is above stop loss price. No entry trade.
			pIndicators->risk = 0.5;


		// Follow 4H trend change.
		//pIndicators->exitSignal = 1;
	}
	
	// If RiksPNL < 0, 
	// Stop loss not less than 0.5 ATR
	// Risk = 0.5

	//if (pIndicators->strategyRiskWithoutLockedProfit < 0 && fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) <= pBase_Indicators->dailyATR * 0.5)
	//{
	//		logWarning("System InstanceID = %d, BarTime = %s, strategyRiskWithoutLockedProfit %lf GapToSL= %lf, dailyATR=%lf",
	//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRiskWithoutLockedProfit, fabs(pIndicators->stopLossPrice - pIndicators->entryPrice), pBase_Indicators->dailyATR );

	//	pIndicators->risk = 0.5;
	//}
	//if (pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->limitRiskPNL
	//	&& ((pIndicators->entryPrice > preHigh - (preHigh - preLow) * 0.382 && pIndicators->entrySignal == 1)
	//	|| (pIndicators->entryPrice < preLow + (preHigh - preLow) * 0.382 && pIndicators->entrySignal == -1)))
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s, strategyRisk %lf limitRiskPNL %lf, entrySignal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRisk, pIndicators->limitRiskPNL, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}
