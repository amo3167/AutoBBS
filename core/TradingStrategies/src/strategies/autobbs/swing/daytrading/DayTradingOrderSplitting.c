/*
 * Day Trading Order Splitting Module
 * 
 * Provides order splitting functions for day trading strategies.
 * Used by splitTradeMode 16 and 18.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "InstanceStates.h"
#include "strategies/autobbs/swing/daytrading/DayTradingOrderSplitting.h"
// splitBuyOrders_Daily_Swing is implemented in OrderSplitting.c - removed duplicate
#if 0
// Removed duplicate function body - see OrderSplitting.c for implementation
void splitBuyOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard, lots_max;
	double gap = pHigh - pIndicators->entryPrice;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	//if (gap >= 1)
	{
		//if (gap * 2 / 3 < 2)
		//	takePrice = 2;
		//else
		//	takePrice = min(gap * 2 / 3, 3);

		takePrice = 3;

		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;

		//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);

		//lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;	
		//if (pIndicators->total_lose_pips > takePrice * lots)
		
		if (pIndicators->lossTimes < 2)
			lots = pIndicators->total_lose_pips / takePrice + lots_singal;
		else
			lots = pIndicators->total_lose_pips / takePrice;		
		

		//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
		//if (lostDays >= 1)
		//	lots *= (lostDays + 0.5);


		//lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

		//lots = min(lots, lots_standard);

		////�@�Yʹ�ù�Ӌֹ�p50�c��Ȼ������L�U��2%
		////Cap the max risk
		//lots_max = calculateOrderSizeWithSpecificRisk(pParams, BUY, pIndicators->entryPrice, 5, 2);

		//lots = min(lots, lots_max);


		if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
			takePrice = min(gap, 3);
		else if (gap > 0)
			takePrice = max(gap * 2 / 3, 3);

		if (takePrice >= 1)
		{
			//lots = 3 / takePrice * lots;
			openSingleLongEasy(takePrice, stopLoss, lots, 0);
		}
	}
}
#endif

void splitSellOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard,lots_max;
	double gap = pIndicators->entryPrice - pLow;

	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	//if (gap >= 1)
	{
		
		takePrice = 3;
		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice)* pIndicators->risk;
		//lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;		

		//lots = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;
		if (pIndicators->lossTimes < 2)
			lots = pIndicators->total_lose_pips / takePrice + lots_singal;
		else
			lots = pIndicators->total_lose_pips / takePrice;
					
		//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
		//if (lostDays >= 1)							
		//	lots *= (lostDays + 0.5);

		//lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

		//lots = min(lots, lots_standard);
		//
		////�@�Yʹ�ù�Ӌֹ�p50�c��Ȼ������L�U��2%
		////Cap the max risk
		//lots_max = calculateOrderSizeWithSpecificRisk(pParams, SELL, pIndicators->entryPrice, 5, 2);

		//lots = min(lots, lots_max);

		if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
			takePrice = min(gap,3);
		else if (gap > 0)
			takePrice = max(gap*2/3, 3);

		if (takePrice >= 1)
		{
			//lots = 3 / takePrice * lots;
			openSingleShortEasy(takePrice, stopLoss, lots, 0);
		}
	}
	

}

void splitBuyOrders_Daily_Swing_ExecutionOnly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard;
	double gap = pHigh - pIndicators->entryPrice;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
		
	//lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;

	//takePrice = max(pBase_Indicators->pDailyMaxATR - pIndicators->atr0, pIndicators->takePrice);	
	//takePrice = 0;
	//openSingleLongEasy(takePrice, stopLoss, lots_singal, 0);
	takePrice = gap - pIndicators->adjust;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);

}

void splitSellOrders_Daily_Swing_ExecutionOnly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard;
	double gap = pHigh - pIndicators->entryPrice;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
		
	//lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;

	////takePrice = max(pBase_Indicators->pDailyMaxATR - pIndicators->atr0, pIndicators->takePrice);	
	//takePrice = 0;

	//openSingleShortEasy(takePrice, stopLoss, lots_singal, 0);
	takePrice = gap - pIndicators->adjust;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);

}
