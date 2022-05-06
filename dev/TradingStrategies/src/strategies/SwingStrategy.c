#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "ComLib.h"
#include "StrategyUserInterface.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE


void splitBuyOrders_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1：1 			
	if (pIndicators->entryPrice <= pBase_Indicators->dailyR2)
	{
		takePrice = fabs(pBase_Indicators->dailyR2 - pIndicators->adjust/2 - pIndicators->entryPrice);
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);		
	}
}

void splitSellOrders_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;	
	if (pIndicators->entryPrice >= pBase_Indicators->dailyS2)
	{
		takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS2 + pIndicators->adjust / 2));
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitBuyOrders_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;	
	double pATR = pBase_Indicators->pWeeklyATR ;
	double pHigh = pBase_Indicators->pWeeklyHigh;
	double pLow = pBase_Indicators->pWeeklyLow;
	double lots, lots_singal;
	double gap = pHigh - pIndicators->entryPrice;

	{
		takePrice = pATR / 3;
		//takePrice = 1;
		//lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		//openSingleLongEasy(takePrice, stopLoss, lots, 0);

		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice);

		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		//if (pIndicators->lossTimes > 1)
		//	lots = pIndicators->total_lose_pips / takePrice;
		//else if (pIndicators->total_lose_pips > takePrice * lots)
		//{
		//	lots = pIndicators->total_lose_pips / takePrice + lots_singal;
		//}

		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

void splitSellOrders_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pWeeklyATR ;
	double pHigh = pBase_Indicators->pWeeklyHigh;
	double pLow = pBase_Indicators->pWeeklyLow;
	double lots, lots_singal;
	double gap = pIndicators->entryPrice- pLow;

	{	
		//takePrice = 1;
		takePrice = pATR / 3;
		//lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		//openSingleShortEasy(takePrice, stopLoss, lots, 0);

		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice);

		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		//if (pIndicators->lossTimes > 1)
		//	lots = pIndicators->total_lose_pips / takePrice;
		//else if (pIndicators->total_lose_pips > takePrice * lots)
		//{
		//	lots = pIndicators->total_lose_pips / takePrice + lots_singal;
		//}

		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}

}

void splitBuyOrders_Daily_Swing_Fix(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 3;

	lots_singal = pParams->accountInfo.equity * 1 * pIndicators->risk / 100000;

	//if (pIndicators->lossTimes < 3)
	//	lots = lots_singal * pow(2, pIndicators->lossTimes);

	if (pIndicators->lossTimes < 2)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;
	
	lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	if (lostDays >= 1)
		lots *= (lostDays + 1);

	lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	lots = min(lots, lots_standard);
		
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		takePrice = min(gap, 3);
	else if (gap > 0)
		takePrice = max(gap * 2 / 3, 3);

	if (takePrice >= 1)
		openSingleLongEasy(takePrice, stopLoss, lots, 0);

}

void splitSellOrders_Daily_Swing_Fix(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 3;

	lots_singal = pParams->accountInfo.equity * 1 * pIndicators->risk / 100000;

	//if (pIndicators->lossTimes < 3)
	//	lots = lots_singal * pow(2, pIndicators->lossTimes);

	if (pIndicators->lossTimes < 2)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	
	if (lostDays >= 1)
		lots *= (lostDays + 1);

	lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	lots = min(lots, lots_standard);

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		takePrice = min(gap, 3);
	else if (gap > 0)
		takePrice = max(gap * 2 / 3, 3);

	if (takePrice >= 1)
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
}

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

		////這裏使用估計止損50點，然後最大風險是2%
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
		////這裏使用估計止損50點，然後最大風險是2%
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

void splitBuyOrders_Daily_GBPJPY_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard,lots_max;
	double gap = pHigh - pIndicators->entryPrice;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
		
	takePrice = 0.3;

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	if (pIndicators->lossTimes <2)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	//if (lostDays >= 1)
	//	lots *= (lostDays + 0.5);

	//lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	//lots = min(lots, lots_standard);

	////這裏使用估計止損50點，然後最大風險是2%
	////Cap the max risk
	//lots_max = calculateOrderSizeWithSpecificRisk(pParams, BUY, pIndicators->entryPrice, 0.5, 2);

	//lots = min(lots, lots_max);

	if (gap > 0.1)
		takePrice = min(gap, 0.3);

	takePrice = 0;
	openSingleLongEasy(takePrice, stopLoss, lots, 1);

}

void splitSellOrders_Daily_GBPJPY_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 0.3;

	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	
	if (pIndicators->lossTimes < 2)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	//if (lostDays >= 1)
	//	lots *= (lostDays + 0.5);

	//lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	//lots = min(lots, lots_standard);

	////這裏使用估計止損50點，然後最大風險是2%
	////Cap the max risk
	//lots_max = calculateOrderSizeWithSpecificRisk(pParams, SELL, pIndicators->entryPrice, 0.5, 2);

	//lots = min(lots, lots_max);

	if (gap > 0.1)
		takePrice = min(gap, 0.3);
	takePrice = 0;

	openSingleShortEasy(takePrice, stopLoss, lots, 1);

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



void splitBuyOrders_Daily_XAUUSD_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 3;

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	if (pIndicators->lossTimes < 2)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	//if (lostDays >= 1)
	//	lots *= (lostDays + 0.5);

	//lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	//lots = min(lots, lots_standard);

	if (gap > 1)
	{
		takePrice = min(gap, 3);
		//lots = 3 / takePrice * lots;
	}

	openSingleLongEasy(takePrice, stopLoss, lots, 0);

}

void splitSellOrders_Daily_XAUUSD_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 3;

	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	if (pIndicators->lossTimes < 2)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	//lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	//if (lostDays >= 1)
	//	lots *= (lostDays + 0.5);

	//lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	//lots = min(lots, lots_standard);

	if (gap > 1)
	{
		takePrice = min(gap, 3);
		//lots = 3 / takePrice * lots;
	}

	openSingleShortEasy(takePrice, stopLoss, lots, 0);

}

//void splitBuyOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
//{
//	double takePrice;
//	double pATR = pBase_Indicators->pDailyATR;
//	double pHigh = pBase_Indicators->pDailyHigh;
//	double pLow = pBase_Indicators->pDailyLow;
//	double lots, lots_singal, lots_standard;
//	double gap = pHigh - pIndicators->entryPrice;
//	time_t currentTime;
//	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
//	int lostDays;
//	double total_pre_lost = 0;
//
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
//
//	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;	
//
//	takePrice = pIndicators->takePrice;
//	openSingleLongEasy(takePrice, stopLoss, lots-0.01, 0);
//
//	takePrice = 0;
//	openSingleLongEasy(takePrice, stopLoss, 0.01, 0);
//
//}
//
//void splitSellOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
//{
//	double takePrice;
//	double pATR = pBase_Indicators->pDailyATR;
//	double pHigh = pBase_Indicators->pDailyHigh;
//	double pLow = pBase_Indicators->pDailyLow;
//	double lots, lots_singal, lots_standard;
//	double gap = pHigh - pIndicators->entryPrice;
//	time_t currentTime;
//	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
//	int lostDays;
//	double total_pre_lost = 0;
//
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
//
//	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;
//
//	takePrice = pIndicators->takePrice;
//	openSingleShortEasy(takePrice, stopLoss, lots - 0.01, 0);
//
//	takePrice = 0;
//	openSingleShortEasy(takePrice, stopLoss, 0.01, 0);
//
//}

void splitBuyOrders_MultiDays_Swing_V2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	takePrice = stopLoss;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);

	takePrice = 0;
	openSingleLongEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);

}

void splitSellOrders_MultiDays_Swing_V2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	takePrice = stopLoss;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);

	takePrice = 0;
	openSingleShortEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);

}

void splitBuyOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;

	takePrice = 0;

	if (pIndicators->tradeMode == 1){
		if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			if (lots / pIndicators->volumeStep > 5)
			{
				takePrice = pIndicators->riskCap * stopLoss;
				openSingleLongEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = (pIndicators->riskCap + 2) * stopLoss;
				//takePrice = 0;
				openSingleLongEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = 0;
				openSingleLongEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
				openSingleLongEasy(takePrice, stopLoss, lots, 0);
		}
		else
			openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else{
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}

	//openSingleLongEasy(takePrice, stopLoss, lots, 0);

}

void splitSellOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;

	takePrice = 0;

	if (pIndicators->tradeMode == 1){

		if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			if (lots / pIndicators->volumeStep > 5)
			{

				takePrice = pIndicators->riskCap * stopLoss;
				openSingleShortEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = (pIndicators->riskCap + 2) * stopLoss;
				//takePrice = 0;
				openSingleShortEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = 0;
				openSingleShortEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
				openSingleShortEasy(takePrice, stopLoss, lots, 0);
		}
		else
			openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else{
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice)* pIndicators->risk;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}

	//openSingleShortEasy(takePrice, stopLoss, lots, 0);

}
void splitBuyOrders_MultiDays_GBPJPY_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 0.3;

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	if (pIndicators->lossTimes == 0)
		lots = lots_singal;
	else if (pIndicators->lossTimes == 1)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	if (lostDays >= 2)
		lots *= (lostDays + 1);

	lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	lots = min(lots, lots_standard);

	openSingleLongEasy(takePrice, stopLoss, lots, 0);

}

void splitSellOrders_MultiDays_GBPJPY_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
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

	takePrice = 0.3;

	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	if (pIndicators->lossTimes == 0)
		lots = lots_singal;
	else if (pIndicators->lossTimes == 1)
		lots = pIndicators->total_lose_pips / takePrice + lots_singal;
	else
		lots = pIndicators->total_lose_pips / takePrice;

	lostDays = getLossTimesInPreviousDaysEasy(currentTime, &total_pre_lost);
	if (lostDays >= 2)
		lots *= (lostDays + 1);

	lots_standard = (pIndicators->total_lose_pips + total_pre_lost) / takePrice + lots_singal;

	lots = min(lots, lots_standard);


	openSingleShortEasy(takePrice, stopLoss, lots, 0);

}


void splitBuyOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	//至少有1/3 空间
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		//if (up_gap <= pATR / 4)
		//	pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

void splitSellOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	//至少有1/3 空间
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		//if (up_gap <= pATR / 4)
		//	pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

void splitBuyOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	//至少有1/3 空间
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);		
	}
}

void splitSellOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	//至少有1/3 空间
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

void splitBuyOrders_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	
	takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;	

	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);

}

void splitSellOrders_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;	
	takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;

	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);

}


AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	workoutExecutionTrend_ATR_Hedge(pParams, pIndicators, pBase_Indicators);

	//switch (pBase_Indicators->dailyTrend_Phase)
	//{
	//case MIDDLE_UP_RETREAT_PHASE:
	//case MIDDLE_DOWN_RETREAT_PHASE:
	//case RANGE_PHASE:
	//	//逆势空间对冲交易
	//	//workoutExecutionTrend_Hedge(pParams, pIndicators, pBase_Indicators);
	//	//if (pIndicators->entrySignal == 0) 
	//		workoutExecutionTrend_ATR_Hedge(pParams, pIndicators, pBase_Indicators);
	//	break;
	//case BEGINNING_UP_PHASE:
	//case BEGINNING_DOWN_PHASE:
	//case MIDDLE_UP_PHASE:
	//case MIDDLE_DOWN_PHASE:
	//	//workoutExecutionTrend_ATR_Hedge(pParams, pIndicators, pBase_Indicators);
	//	break;
	//}

	//profitManagement(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}

//空间极限回归交易
AsirikuyReturnCode workoutExecutionTrend_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double down_gap;
	double up_gap;

	double ATR0 = iAtr(B_DAILY_RATES, 1, 0);
	double ATR20 = iAtr(B_DAILY_RATES, 20, 1);
	double pMaxATR = max(pBase_Indicators->pDailyATR, ATR20);

	double preOpen = iOpen(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);
	double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);
	BOOL isOpen = FALSE;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	if (timeInfo1.tm_hour == 23 && timeInfo1.tm_min > 25)
	{
		pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}
	//closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 15;

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_RETREAT_PHASE || pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE || pBase_Indicators->dailyTrend_Phase == RANGE)
	{
		//至少有1/3 空间,Sell		
		if (pBase_Indicators->pMaxDailyHigh - pParams->bidAsk.bid[0] <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;

			if (ATR0 >= pMaxATR && preClose < preOpen && intradayHigh - intradayClose >= ATR0 * 0.1
				&& timeInfo1.tm_hour < 23
				&& !hasSameDayOrderEasy(currentTime, &isOpen)
				//&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = -1;

			pIndicators->exitSignal = EXIT_BUY;
		}
		else if (pParams->bidAsk.ask[0] - pBase_Indicators->pMaxDailyLow <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

			//逆势对冲交易
			if (ATR0 >= pMaxATR && preClose > preOpen && intradayClose - intradayLow >= ATR0 * 0.1
				&& timeInfo1.tm_hour < 23
				&& !hasSameDayOrderEasy(currentTime,&isOpen)
				//&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = 1;

			pIndicators->exitSignal = EXIT_SELL;
		}
	}
	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double down_gap = pIndicators->entryPrice - pBase_Indicators->pDailyLow;
	double up_gap = pBase_Indicators->pDailyHigh - pIndicators->entryPrice;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//closeAllWithNegativeEasy(1, currentTime, 3);
	if (timeInfo1.tm_hour == 23 && timeInfo1.tm_min > 25)
	{
		pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 14;

	//Asia hour, risk 50%
	if (timeInfo1.tm_hour <= 8)
		pIndicators->risk = 0.5;

	if (pBase_Indicators->dailyTrend_Phase > 0)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& timeInfo1.tm_hour < 23
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;

	}
	else if (pBase_Indicators->dailyTrend_Phase < 0)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		//逆势对冲交易
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& timeInfo1.tm_hour < 23
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;
	}
	else
	{
		//至少有1/3 空间,Sell
		if (up_gap <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
				&& timeInfo1.tm_hour < 23
				&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = -1;

			pIndicators->exitSignal = EXIT_BUY;
		}
		else if (down_gap <= pBase_Indicators->pDailyATR / 3)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			//逆势对冲交易
			if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
				&& timeInfo1.tm_hour < 23
				&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
				)
				pIndicators->entrySignal = 1;

			pIndicators->exitSignal = EXIT_SELL;
		}
	}
	return SUCCESS;
}

//1. 在5M突破区间后，入场
//2. 但是如果不是第一单的话，需要在15分钟后，使用15 BBS Trend 检查
//3. 如果没有BBS Trend的支持，就close trade,先离场，等待下一次机会（这个时候，不能入场）
//4. 同时，使用total lose risk control < 0.6%
static void XAUUSD_Daily_Stop_Check(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime, openTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double diffMins;	
	int count;
	double intradayHigh, intradayLow;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	//这里简单处理，如果有单，都是使用0 的单子

	if (pParams->orderInfo[0].ticket != 0 && pParams->orderInfo[0].isOpen == TRUE)
	{		

		// Must pass 15M
		openTime = pParams->orderInfo[0].openTime;
		safe_gmtime(&timeInfo2, pParams->orderInfo[0].openTime);

		diffMins = difftime(currentTime, openTime) / 60;

		if (diffMins == 15)
		{

			//count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
			//count = count - 3; // minus 15M
			//if (count > 1)
			//	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh, &intradayLow);
			//pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, count = %ld,intradayHigh = %lf,intradayLow=%lf",
			//	(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, count,intradayHigh, intradayLow);

			if (pParams->orderInfo[0].type == BUY && pIndicators->bbsTrend_secondary == -1
				//( (pBase_Indicators->maTrend < 0 && iClose(B_PRIMARY_RATES, 1)<intradayHigh) //逆勢
				//|| (pBase_Indicators->maTrend >  0 && pIndicators->bbsTrend_secondary == -1) //顺势
				//	)
					)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_BUY;
				}
			}
			if (pParams->orderInfo[0].type == SELL && pIndicators->bbsTrend_secondary == 1
				//((pBase_Indicators->maTrend < 0 && iClose(B_PRIMARY_RATES, 1) > intradayLow) //逆勢
				//|| (pBase_Indicators->maTrend >  0 && pIndicators->bbsTrend_secondary == 1) //顺势
				//)
				)				
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_SELL;
				}
			}



		}
	}
}

/*
不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 8?
4.最多交易3次，日内
5.10点之前，不交易
*/
static BOOL GBPJPY_DayTrading_Allow_Trade_Old(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

	currentTime = pParams->ratesBuffers->rates[B_SECONDARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	execution_tf = pIndicators->executionRateTF;

	if (timeInfo1.tm_hour < 10)
		return FALSE;

	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = (10 - 1) * (60 / execution_tf) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_SECONDARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	if (fabs(pIndicators->asia_high - pIndicators->asia_low) >= 0.8)
		return FALSE;

	if (iAtr(B_DAILY_RATES, 1, 1) >= 1.5)
		return FALSE;
	if (fabs(close_prev1 - close_prev2) >= 0.8)
		return FALSE;

	preHigh = iHigh(B_DAILY_RATES, 2);
	preLow = iLow(B_DAILY_RATES, 2);
	preClose = iClose(B_DAILY_RATES, 2);

	pivot = (preHigh + preLow + preClose) / 3;
	S3 = preLow - 2 * (preHigh - pivot);
	R3 = preHigh + 2 * (pivot - preLow);

	if (close_prev1 > R3 || close_prev1 < S3)
		return FALSE;

	return TRUE;
}

void XAUUSD_BBS_StopLoss_Trend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, minGap, hourGap;
	time_t currentTime, openTime;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	double high, low, takePrice, stopLoss;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	
	if (pParams->orderInfo[0].ticket != 0 && pParams->orderInfo[0].isOpen == TRUE)
	{
		openTime = pParams->orderInfo[0].openTime;
		safe_gmtime(&timeInfo2, openTime);

		minGap = (int)((timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf + 0.5);
		hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);
		count = hourGap + minGap;

		if (count > 1 && getHighLowEasy(B_PRIMARY_RATES, shift1Index, count, &high, &low) == SUCCESS)
		{
			if (pParams->orderInfo[0].type == BUY)
			{
				// 开始跟踪止损（止盈）
				if (pBase_Indicators->dailyTrend_Phase != RANGE_PHASE && pIndicators->bbsTrend_secondary == -1)
				{
					
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, exit a buy trade in a trend on 15M BBS stop",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

					pIndicators->exitSignal = EXIT_ALL;
				}
			}

			if (pParams->orderInfo[0].type == SELL)
			{
				if (pBase_Indicators->dailyTrend_Phase != RANGE_PHASE && pIndicators->bbsTrend_secondary == 1)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, exit a sell trade in a trend on 15M BBS stop",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

					pIndicators->exitSignal = EXIT_ALL;
				}
			}
		}
	}
}

/*
不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 7 or 8?
4.最多交易3次，日内
5.10点之前，不交易
6.如果15MA too close, 例如 2，1H MA 方向不支持
*/
static BOOL XAUUSD_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate,euro_index_rate,execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);	
	double MATrend_1H, MATrend_15M;
	int startTradingTime = pIndicators->startHour;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	//execution_tf = pIndicators->executionRateTF;
	execution_tf = (int)pParams->settings[TIMEFRAME];

	//if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	//	startTradingTime = 15;

	// filter US rate day
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators))
		return FALSE;

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = (startTradingTime - 1) * (60 / execution_tf) - 1;
	if (count > 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	if (timeInfo1.tm_hour >= 17) //17:00 进入美洲交易时段
	{
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		//count must > 1, otherwise, cause TA_MIN error 
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);
	}
	//// Workout Euro time bar( 10 - 15)
	//if (timeInfo1.tm_hour >= 10 && timeInfo1.tm_hour < 17)
	//{
	//	count = (timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
	//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
	//	pIndicators->euro_open = pIndicators->asia_close;
	//	pIndicators->euro_close = iClose(B_PRIMARY_RATES, 1);
	//}

	//if (timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour < 24)
	//{
	//	count = (timeInfo1.tm_hour - 15) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
	//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->us_high), &(pIndicators->us_low));
	//	pIndicators->us_open = pIndicators->euro_close;
	//	pIndicators->us_close = iClose(B_PRIMARY_RATES, 1);
	//}

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	if (fabs(pIndicators->asia_high - pIndicators->asia_low) >= 7.5)
		return FALSE; 

	if (iAtr(B_DAILY_RATES, 1, 1) >= 20)
		return FALSE;
	if (fabs(close_prev1 - close_prev2) >= 10)
		return FALSE;

	preHigh = iHigh(B_DAILY_RATES, 2);
	preLow = iLow(B_DAILY_RATES, 2);
	preClose = iClose(B_DAILY_RATES, 2);

	pivot = (preHigh + preLow + preClose) / 3;	 
	S3 = preLow - 2 * (preHigh - pivot);
	R3 = preHigh + 2 * (pivot - preLow);
	
	if (close_prev1 > R3 || close_prev1 < S3)
		return FALSE;

	
	//MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);
	//MATrend_15M = getMATrend(2, B_SECONDARY_RATES, 1);

	//if (MATrend_1H * MATrend_15M <0) //同方向
	//{

	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s filter out small range: MATrend_1H=%lf,MATrend_15M=%lf",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, MATrend_1H, MATrend_15M);
	//	return FALSE;		
	//}

	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators))
		return FALSE;
	

	return TRUE;
}

static void XAUUSD_DayTrading_Entry(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double ATR0_EURO, double stopLoss, double Range)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	if (orderType == SELL)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

		pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
		pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
		if (ATR0_EURO > Range && pIndicators->lossTimes < 2 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
		{
			//pIndicators->risk = pow(2, pIndicators->lossTimes);
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;
	}


	if (orderType == BUY)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
		pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
		pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

		if (ATR0_EURO > Range && pIndicators->lossTimes < 2 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
		{
			//pIndicators->risk = pow(2, pIndicators->lossTimes);
			pIndicators->entrySignal = -1;
		}

		pIndicators->exitSignal = EXIT_BUY;
	}
}


/*
空间与BBS15的结合日内黄金交易法
亚洲时段： 
1- 10am

在有单子的情况下，在当日波幅小于 80p,必须使用空间法。

不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 7 or 8?
4.最多交易3次，日内
5.如果前一天到了S3 or R3

日内趋势：
MA15(50) > MA15(200): UP
MA15(50) < MA15(200): DOWN

入场:
1. 如果在10点的时候，顺势, 就马上入场
2. 如果10点的时候，逆势， 就等待到以下情况：
	2.1 如果5M的收盘突破亚洲时段的高低点，就入场
	2.2 直到顺势的15MBBS 突破， 入场

单量：
0.3 risk

出场：
日内23：30，必须离场

如果顺势： 必须5M突破日内的高低点，才出现信号
如果逆势： 到了BBS15的stoploss, 就出现信号。因为就变成顺势的BBS突破。

问题：
这个策略有问题，可能需要改为
5M 均线为指引，不是15MBBS
1M 空间突破


*/
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow, primary_close_pre1 = iClose(B_PRIMARY_RATES, 1),secondary_close_pre1 = iClose(B_SECONDARY_RATES,1);
	double ATR0, Range, ATR0_EURO = 10, Range_Limit;
	double close_prev1 = iClose(B_DAILY_RATES, 1);
	double openOrderHigh, openOrderLow;

	int count;
	double adjust = 0.15;
	double stopLoss = pBase_Indicators->dailyATR*1.2;
	BOOL isOpen;
	OrderType side;

	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double gap;
	double ATRWeekly0;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int MATrend_1H, noNewTradeSignal = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	

	//Fix lots
	//pIndicators->splitTradeMode = 18;
	//Dyanmic lots
	pIndicators->splitTradeMode = 16;

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	{
		Range = pIndicators->atr_euro_range * 2 / 3;		
		pIndicators->risk = 1;
	}
	else
	{
		Range = pIndicators->atr_euro_range;
		
		//if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
		//	pIndicators->risk = 1.5;
		//else
		pIndicators->risk = 1;
	}

	stopLoss = 10;

	safe_timeString(timeString, currentTime);

	closeAllWithNegativeEasy(1, currentTime, 3);


	pBase_Indicators->maTrend = getMATrend(2, B_SECONDARY_RATES, 1);

	if (XAUUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
		readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
	else
	{
		pBase_Indicators->pWeeklyPredictATR = 20;
		pBase_Indicators->pWeeklyPredictMaxATR = 28;
	}

	if ( (int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR)
		return SUCCESS;

	//if (pBase_Indicators->pDailyPredictATR >= 10 && ATRWeekly0 < pBase_Indicators->pWeeklyPredictATR)
	//	pIndicators->risk = 2;

	count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count > 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh, &intradayLow);
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf, asia_close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low, pIndicators->asia_close);

	if (timeInfo1.tm_hour >= 17)
	{
		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO,pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);
	}

	//在17點，如果波幅很小，如果又趋势，总是80点。如果区间 0.67*0.8
	if (ATR0_EURO < Range)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)		
			pIndicators->exitSignal = EXIT_ALL;
			
		return SUCCESS;
	}
	
	// secondary rate is 15M , priarmy rate is 5M，使用1M?
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)  //如果没有开仓, 就可以开仓
	{
		if (((ATR0 >= 4 && pBase_Indicators->dailyTrend_Phase == RANGE_PHASE) || pBase_Indicators->dailyTrend_Phase != RANGE_PHASE)
			&& ATR0_EURO > Range && pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && timeInfo1.tm_hour < 22)//顺势
		{
			gap = pHigh - pParams->bidAsk.ask[0];

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
			pIndicators->entrySignal = 1;
			pIndicators->lossTimes = 0;
		}
		else if (((ATR0 >= 4 && pBase_Indicators->dailyTrend_Phase == RANGE_PHASE) || pBase_Indicators->dailyTrend_Phase != RANGE_PHASE)
			&& ATR0_EURO > Range && pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && timeInfo1.tm_hour < 22)//顺势
		{
			gap = pParams->bidAsk.bid[0] - pLow;

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
			pIndicators->entrySignal = -1;
			pIndicators->lossTimes = 0;
		}
		else //逆势，使用空间交易法,突破亚洲盘的高低点
		{

			//注意，这里也可以出现顺势的情况。
			//如果真的逆势，就需要15M的收盘价。这里就是secondary rate
			if (pBase_Indicators->maTrend > 0)
			{
				if (ATR0_EURO > Range && primary_close_pre1 > pIndicators->asia_high && timeInfo1.tm_hour < 22)
				{
					gap = pHigh - pParams->bidAsk.ask[0];

					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
					pIndicators->entrySignal = 1;
					pIndicators->lossTimes = 0;
				}

				if (ATR0_EURO > Range && secondary_close_pre1 < pIndicators->asia_low && timeInfo1.tm_hour < 22)
				{
					gap = pParams->bidAsk.bid[0] - pLow;

					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
					pIndicators->entrySignal = -1;
					pIndicators->lossTimes = 0;
				}
			}

			if (pBase_Indicators->maTrend < 0)
			{
				if (ATR0_EURO > Range && secondary_close_pre1 > pIndicators->asia_high && timeInfo1.tm_hour < 22)
				{
					gap = pHigh - pParams->bidAsk.ask[0];

					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
					pIndicators->entrySignal = 1;
					pIndicators->lossTimes = 0;
				}

				if (ATR0_EURO > Range && primary_close_pre1 < pIndicators->asia_low && timeInfo1.tm_hour < 22)
				{
					gap = pParams->bidAsk.bid[0] - pLow;

					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
					pIndicators->entrySignal = -1;
					pIndicators->lossTimes = 0;
				}
			}
		}
	}
	else
	{

		side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
		if (side == SELL)
		{
			pIndicators->executionTrend = -1;
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Side = SELL isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

			//小于80， 必须使用空间。
			//1. 在5M突破区间后，入场
			//2. 但是如果不是第一单的话，需要在15分钟后，使用15 BBS Trend 检查
			//3. 如果没有BBS Trend的支持，就close trade,先离场，等待下一次机会（这个时候，不能入场）
			//4. 如果时间到了美洲盘，必须使用15MBBS
			//5. 如果空间不够30点，这个时候，不要冒险入场，等待回调，知道有30点的空间为止，在15MBBS的支持下。			
			gap = pHigh - pParams->bidAsk.ask[0];

			if (ATR0 >= Range && 
				pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && 
				pIndicators->bbsIndex_secondary == shift1Index_secondary
				) 
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
			}
			else
			{
				if (pBase_Indicators->maTrend > 0)
				{
					if (primary_close_pre1 - adjust > openOrderHigh)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (secondary_close_pre1 + adjust < openOrderLow)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Enter a sell trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}

				if (pBase_Indicators->maTrend < 0)
				{
					if (secondary_close_pre1 - adjust > openOrderHigh)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (primary_close_pre1 + adjust < openOrderLow)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Enter a sell trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}
			}

		}

		if (side == BUY)
		{
			pIndicators->executionTrend = 1;
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Side = BUY isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

			gap = pParams->bidAsk.bid[0] - pLow;

			if (ATR0 >= Range
				&& pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 &&
				pIndicators->bbsIndex_secondary == shift1Index_secondary 
				)
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
			}
			else
			{
				if (pBase_Indicators->maTrend > 0)
				{
					if (secondary_close_pre1 + adjust < openOrderLow)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (primary_close_pre1 - adjust > openOrderHigh)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Enter a buy trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}

				if (pBase_Indicators->maTrend < 0)
				{
					if (primary_close_pre1 + adjust < openOrderLow)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (secondary_close_pre1 - adjust > openOrderHigh)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Enter a buy trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}
			}
		}

		if (pIndicators->entrySignal == 0 
			&& timeInfo1.tm_hour >= 10 && timeInfo1.tm_hour <=17
			)
			XAUUSD_Daily_Stop_Check(pParams, pIndicators, pBase_Indicators);


		////在17點，如果波幅很小，如果又趋势，总是80点。如果区间 0.67*0.8
		//if (ATR0_EURO < Range)
		//{
		//	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
		//	{
		//		//pIndicators->exitSignal = EXIT_ALL;

		//		//closeAllWithNegativeEasy(5, currentTime, 3);

		//		if ((pIndicators->bbsTrend_secondary == -1 && side == BUY)
		//			|| (pIndicators->bbsTrend_secondary == 1 && side == SELL)
		//			)
		//		{
		//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a trade on after 17H side = %ld.",
		//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, side);

		//			pIndicators->exitSignal = EXIT_ALL;
		//		}
		//	}
		//	return SUCCESS;
		//}

		//在21點后，如果有單子，使用BBS15平仓
		if (pIndicators->entrySignal == 0 && timeInfo1.tm_hour >= 21)
		{
			if (isOpen == TRUE)
			{
				if ((pIndicators->bbsTrend_secondary == -1 && side == BUY)
					|| (pIndicators->bbsTrend_secondary == 1 && side == SELL)
					)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a trade on after 21H side = %ld.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,side);

					pIndicators->exitSignal = EXIT_ALL;
				}
			}
		}
	}


	MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);

	//if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 3)
	if (pIndicators->entrySignal != 0)
	{
		// too close 非农
		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1
			&& abs(pBase_Indicators->maTrend) <= 1 &&
			((pBase_Indicators->maTrend > 0 && MATrend_1H < 0)
			|| (pBase_Indicators->maTrend < 0 && MATrend_1H > 0)
			))
		{
			noNewTradeSignal = 1;
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, 15M Trend = %ld, 1H Trend = %ld,noNewTradeSignal=%ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, MATrend_1H, noNewTradeSignal);
			pIndicators->entrySignal = 0;
		}

		//if (gap < 3)
		//{
		//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, gap=%lf is lesss than 3",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, gap);
		//	pIndicators->entrySignal = 0;
		//}
	}

	

	//if (pIndicators->entrySignal == 0 && ATR0 > 8)
	//{
	//	HalfPoint = intradayHigh - ATR0 * 2 /3;

	//	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
	//	{
	//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == BUY
	//			&& primary_close_pre1 < HalfPoint)
	//		{
	//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a buy order",
	//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString) ;
	//			pIndicators->exitSignal = EXIT_BUY;
	//		}
	//		

	//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == SELL
	//			&& primary_close_pre1 > HalfPoint)
	//		{
	//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a sell order",
	//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	//			pIndicators->exitSignal = EXIT_SELL;
	//		}
	//	}
	//}

	//if (pIndicators->entrySignal != 0 && pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity  > 0.01)
	//{
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Daily Loss=%lf, skip this entry signal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	return SUCCESS;
}

/*
纯空间交易法
条件：
1. 日波幅至少80-100点
2. 目标是30点
3. 过半是50点
4. 不能是在很小的过度行情中，因为波幅可能小于80点。
5. 执行时间框架是1分钟

入场：
1.开盘后，等待波幅到了50点。
2.按照50点后的方向，开出单子。 仓量是1%
3.止损是80点。
4.止盈是30点。

出场：
1. 顺利止盈出场
2. 当从低点走出50点后，认为方向错了，平掉原来单子，开出新的单子。 
3. 仓量是包括原来单子的亏损， 还要1%盈利。
4.止损是80点。
5.止盈是30点。
6.在23：30的时候，如果亏损，就要离场。如果不亏，就平保到明天

*/
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0, Range,ATRWeekly0;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1),close_prev2 = iClose(B_DAILY_RATES, 2);
	
	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.03;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
		readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
	else
	{
		pBase_Indicators->pWeeklyPredictATR = 2.5; 
		pBase_Indicators->pWeeklyPredictMaxATR = 3;
	}

	//closeAllWithNegativeEasy(1, currentTime, 3);

	pIndicators->splitTradeMode = 17;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	Range = 0.5;

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);

	//开盘60分钟，因为开始时候，点差很大。
	//不要入场
	if (timeInfo1.tm_hour == 0)
		return SUCCESS;

	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) ;
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);		
	}
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
		return SUCCESS;
	
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR)
		return SUCCESS;

	if (pBase_Indicators->pDailyPredictATR >= 1 && ATRWeekly0 < pBase_Indicators->pWeeklyPredictATR)
		pIndicators->risk = 2;

	if (iAtr(B_DAILY_RATES, 1, 1) >= max(2, pBase_Indicators->pWeeklyPredictATR)) //日波幅到了最小的周波幅
		return SUCCESS;
	if (fabs(close_prev1 - close_prev2) >= max(1, pBase_Indicators->pWeeklyPredictATR /2)) //日升跌幅到了一半的最小周波幅
		return SUCCESS;


	//必须 close all orders before EOD
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >=30)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
			pIndicators->exitSignal = EXIT_ALL; 
		return SUCCESS;
	}

	// secondary rate is 5M , priarmy rate is 1M
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)  
	{	
		if (ATR0 <= 0.8 && pParams->bidAsk.ask[0] - intradayLow >= Range)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
			pIndicators->entrySignal = 1;
		}

		if (ATR0 <= 0.8 && intradayHigh - pParams->bidAsk.bid[0] >= Range)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
			pIndicators->entrySignal = -1;
		}
		
	}
	else
	{
		side = getLastestOpenOrderTypeEasy_GBPJPY(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow);
		if (side == SELL)
		//if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == SELL)
		{
			
			if (pParams->bidAsk.ask[0] - openOrderLow >= Range)
			//if (pParams->bidAsk.ask[0] - intradayLow >= Range)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < 3 && pIndicators->winTimes == 0)
				{
					//pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = 1;
				}
				pIndicators->exitSignal = EXIT_SELL;
			}

		}

		if (side == BUY)
		//if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == BUY)
		{
			
			if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)
			//if (intradayHigh - pParams->bidAsk.bid[0] >= Range)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 )
				{
					//pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = -1;
				}

				pIndicators->exitSignal = EXIT_BUY;
			}
			
		}

	}



	return SUCCESS;
}

/*
不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 0.8?
4.最多交易3次，日内
5.10点之前，不交易
*/
static BOOL XAUUSD_DayTrading_Allow_Trade_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL shouldFilter)
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
	double ATRWeekly0,ATRDaily0;
	double weeklyATR = (pBase_Indicators->pWeeklyPredictMaxATR + pBase_Indicators->pWeeklyPredictATR) / 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	
	execution_tf = (int)pParams->settings[TIMEFRAME];

	ATRDaily0 = iAtr(B_DAILY_RATES, 1, 0);
	if (timeInfo1.tm_hour < startTradingTime || 
		(timeInfo1.tm_hour >= pIndicators->endHour 
		//&& ATRDaily0 > pBase_Indicators->pDailyMaxATR
		)
	  )
	{
		
		return FALSE;
	}

	// filter 非农
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	{
	
		strcpy(pIndicators->status, "Filter Non-farm day");

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;	
	}

	
	if (XAUUSD_not_full_trading_day(pParams, pIndicators, pBase_Indicators) == TRUE)
	{		
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
		return FALSE;
	}


	//asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	//count = (startTradingTime-1) * (60 / execution_tf) - 1;
	//if (count >= 1)
	//	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	//else
	//	return FALSE;

	//pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	//pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	//pIndicators->asia_open = close_prev1;
	//pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	//
	//pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
	//	(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	//if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range*1.2)
	//	return FALSE;
	if (shouldFilter == FALSE)
		return TRUE;

	//if ((pBase_Indicators->dailyTrend == 6 || pBase_Indicators->dailyTrend == -6))
	//	return TRUE;

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf,pWeeklyPredictATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR, pBase_Indicators->pWeeklyPredictATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than atr_euro_range %lf",
			pBase_Indicators->pDailyPredictATR, pIndicators->atr_euro_range);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 10)	
	{
		sprintf(pIndicators->status, "ATRWeekly0 %lf is greater than pWeeklyPredictMaxATR %lf and pDailyPredictATR > 10",
			ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR, pBase_Indicators->pDailyPredictATR);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	if (iAtr(B_DAILY_RATES, 1, 1) - max(20, pBase_Indicators->pWeeklyPredictATR / 2) >= 0) //日波幅到了最小的周波幅
	{
		sprintf(pIndicators->status, "ATR1 %lf is greater than half of pWeeklyPredictATR %lf",
			iAtr(B_DAILY_RATES, 1, 1), max(20, pBase_Indicators->pWeeklyPredictATR / 2));

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	if (fabs(close_prev1 - close_prev2) >= max(10, pBase_Indicators->pWeeklyPredictATR / 3)) //日升跌幅到了一半的最小周波幅
	{
		sprintf(pIndicators->status, "Previous close gap %lf is greater than third of pWeeklyPredictATR %lf",
			fabs(close_prev1 - close_prev2), max(10, pBase_Indicators->pWeeklyPredictATR / 3));

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	return TRUE;
}

static BOOL BTCUSD_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL shouldFilter)
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
	double ATRWeekly0;
	double weeklyATR = (pBase_Indicators->pWeeklyPredictMaxATR + pBase_Indicators->pWeeklyPredictATR) / 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	execution_tf = (int)pParams->settings[TIMEFRAME];


	//// filter 非农
	//if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	//{

	//	strcpy(pIndicators->status, "Filter Non-farm day");

	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

	//	return FALSE;
	//}

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	if (shouldFilter == FALSE)
		return TRUE;

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
		readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
	
	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf,pWeeklyPredictATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR, pBase_Indicators->pWeeklyPredictATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than atr_euro_range %lf",
			pBase_Indicators->pDailyPredictATR, pIndicators->atr_euro_range);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	//if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR)
	//{
	//	sprintf(pIndicators->status, "ATRWeekly0 %lf is greater than pWeeklyPredictMaxATR %lf",
	//		ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR);

	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

	//	return FALSE;
	//}

	if (iAtr(B_DAILY_RATES, 1, 1) >= pBase_Indicators->pWeeklyPredictATR / 2) //日波幅到了最小的周波幅
	{
		sprintf(pIndicators->status, "ATR1 %lf is greater than half of pWeeklyPredictATR %lf",
			iAtr(B_DAILY_RATES, 1, 1), pBase_Indicators->pWeeklyPredictATR / 2);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}
	if (fabs(close_prev1 - close_prev2) >= pBase_Indicators->pWeeklyPredictATR / 3) //日升跌幅到了一半的最小周波幅
	{
		sprintf(pIndicators->status, "Previous close gap %lf is greater than third of pWeeklyPredictATR %lf",
			fabs(close_prev1 - close_prev2), pBase_Indicators->pWeeklyPredictATR / 3);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	return TRUE;
}

/*
不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 0.8?
4.最多交易2次，日内
5.3点之前，不交易
6.优化：
如果在8点前已经入场，risk = 1,而且是浮亏的话，就只做一次交易
如果在8点前已经入场，risk = 1,而且第一次已经亏了，只做一次，不再入场。
如果是周三，risk =1的话，也可以做一次
如果前一天，波幅很大，也可以做一次。
*/
static BOOL GBPJPY_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1, timeInfoClose;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);	
	//int startTradingTime = 8;
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;
	double targetProfit = 0,weeklyPNL = 0;
	int orderIndex = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	//if (pBase_Indicators->pDailyPredictATR >= 1)
	//	pIndicators->maxTradeTime = 2;
	//else
	//	pIndicators->maxTradeTime = 1;

	pIndicators->maxTradeTime = 1;
	

	execution_tf = (int)pParams->settings[TIMEFRAME];
	
	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
		readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
	else
	{
		pBase_Indicators->pWeeklyPredictATR = 2.5;
		pBase_Indicators->pWeeklyPredictMaxATR = 3;
	}

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
	//if (ATRWeekly0 < pBase_Indicators->pWeeklyPredictATR)
	//{
	//	if (pBase_Indicators->pDailyPredictATR >= 1.2)
	//	{
	//		startTradingTime = 8;
	//		//pIndicators->risk = 2;
	//	}

	//}

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	//if (fabs(pIndicators->asia_high - pIndicators->asia_low) > 0.8)
	//	return FALSE;

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
		return FALSE;
	
	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 1)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
		return FALSE;
	}
	

	if (iAtr(B_DAILY_RATES, 1, 1) >= max(2, pBase_Indicators->pWeeklyPredictATR)) //日波幅到了最小的周波幅
		return FALSE;
	if (fabs(close_prev1 - close_prev2) >= max(1, pBase_Indicators->pWeeklyPredictATR / 2)) //日升跌幅到了一半的最小周波幅
		return FALSE;

	//Filter out wenesday ??? Need to check out more data....
	//if (timeInfo1.tm_wday == 3 && pIndicators->risk == 1 )
	//	return FALSE;

	////If weekly target meet, stop trading this week. target is 1% for now
	//// target profit / 3 * 10
	//targetProfit = pParams->settings[ACCOUNT_RISK_PERCENT] * 10 / 3;
	//weeklyPNL = caculateStrategyWeeklyPNLEasy(currentTime);

	//if (weeklyPNL >= targetProfit)
	//{
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, weeklyPNL = %lf,targetProfit = %lf",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, weeklyPNL, targetProfit);
	//	return FALSE;
	//}


	return TRUE;
}

/*
不交易的情况：
1.前一天的ATR > 20
2.前一天的Close -  前两天的Close的距离 > 10
3.亚洲时段的波幅> 0.8?
4.最多交易2次，日内
5.3点之前，不交易
6.优化：
如果在8点前已经入场，risk = 1,而且是浮亏的话，就只做一次交易
如果在8点前已经入场，risk = 1,而且第一次已经亏了，只做一次，不再入场。
如果是周三，risk =1的话，也可以做一次
如果前一天，波幅很大，也可以做一次。
*/
static BOOL GBPJPY_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1, timeInfoClose;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	//int startTradingTime = 8;
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;
	double targetProfit = 0, weeklyPNL = 0;
	int orderIndex = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->maxTradeTime = 1;

	execution_tf = (int)pParams->settings[TIMEFRAME];

	
	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range * 0.94)
	{
		sprintf(pIndicators->status, "Intraday ATR(H-L) %lf is greater than euro atr range %lf",
			fabs(pIndicators->asia_high - pIndicators->asia_low), pIndicators->atr_euro_range * 0.94);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}
	

	if (pBase_Indicators->pDailyPredictATR < (double)parameter(AUTOBBS_IS_ATREURO_RANGE))
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than euro atr range %lf",
			pBase_Indicators->pDailyPredictATR, (double)parameter(AUTOBBS_IS_ATREURO_RANGE));

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}
	
	return TRUE;
}

static BOOL GBPUSD_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1, timeInfoClose;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	//int startTradingTime = 8;
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;
	double targetProfit = 0, weeklyPNL = 0;
	int orderIndex = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->maxTradeTime = 1;

	execution_tf = (int)pParams->settings[TIMEFRAME];


	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;


	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range * 0.94)
		return FALSE;


	if (pBase_Indicators->pDailyPredictATR < (double)parameter(AUTOBBS_IS_ATREURO_RANGE))
		return FALSE;


	return TRUE;
}
/*
空间，时间，均线 结合的交易法
条件：
1. 日波幅至少80-100点
2. 目标是30点
3. 过半是50点
4. 不能是在很小的过度行情中，因为波幅可能小于80点。
5. 执行时间框架是1分钟
6. 加入时间,如果亚洲盘的时候，50点出现，并且5M的均线支持，就入场。
如果不支持，就等待到欧洲盘。
当欧洲盘开始的时候，也是不是马上入场，如果50点已经出，就等待回调。
如果没有出，等待50点，不需要均线支持了。

均线的支持，只适用于是第一次入场，后面都依靠空间交易。

具体实现：
使用文件保留交易的信息：
高点
低点
目前open的单子号
风险
是否回调？

入场：
1.亚洲盘波幅不能大于80点
2.10am（9am)开始入场，
3.开盘后，等待波幅到了50点。
2.按照50点后的方向，开出单子。 仓量是1%
3.止损是80点。
4.止盈是30点。

出场：
1. 顺利止盈出场
2. 当从低点走出50点后，认为方向错了，平掉原来单子，开出新的单子。
3. 仓量是包括原来单子的亏损， 还要1%盈利。
4.止损是80点。
5.止盈是30点。
6.在23：30的时候，如果亏损，就要离场。如果不亏，就平保到明天

*/
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
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

	BOOL isBreakEvent = FALSE;
	int euro_index_rate;
	double ATREuroPeriod = 0.0;
	int executionTrend;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 17;
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

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	//Range = 0.5;
	Range = min(0.5, pBase_Indicators->pDailyPredictATR / 2);
	//if (pBase_Indicators->pDailyPredictATR < 1.2)
	//	Range = 0.5;
	//else if (pBase_Indicators->pDailyPredictATR < 1.5)
	//	Range = 0.6;
	//else
	//	Range = 0.7;

	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{
		if (hasOpenOrder())
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPJPY_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

	//计算日内高低点
	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
	}
	else
		return SUCCESS;	

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	//必须 close all orders before EOD
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}	


	// secondary rate is 5M , priarmy rate is 1M
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)
	{
		//在80点内，必须入场，如果放弃了，就放弃当日交易。
		//if (ATR0 >= 1.0)
		//	return SUCCESS;

		//加入5M均线的支持
		if (pParams->bidAsk.ask[0] - intradayLow >= Range 
			&& intradayHigh - pParams->bidAsk.bid[0] < Range
			//&& pBase_Indicators->maTrend > 0
			&& (executionTrend > 0 || (executionTrend == 0 && pBase_Indicators->maTrend > 0))
			&& timeInfo1.tm_hour <= 15		
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;			
			pIndicators->entrySignal = 1;
		}

		if (intradayHigh - pParams->bidAsk.bid[0] >= Range 
			&& pParams->bidAsk.ask[0] - intradayLow < Range
			//&& pBase_Indicators->maTrend < 0
			&& (executionTrend < 0 || (executionTrend == 0 && pBase_Indicators->maTrend < 0))
			&& timeInfo1.tm_hour <= 15
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
			pIndicators->entrySignal = -1;
		}		

	}
	else
	{
		if (pIndicators->winTimes > 0){
			if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
				pIndicators->exitSignal = EXIT_ALL;
			return SUCCESS;
		}

		//latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		//这里可以考虑getlatestorder, 不单单是open order，因为可能会平保
		//side = getLastestOpenOrderTypeEasy_GBPJPY(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow);
		side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow,&isOpen);
		
		//如果已经有了一个trade，那就是第二次交易，要使用日内的高低点
		//如果有了2个trades,那就是第三次交易了，使用orderHigh, orderLow.
		orderCountToday = getOrderCountTodayEasy(currentTime);
		//orderCountToday = getOrderCountTodayExcludeBreakeventOrdersEasy(currentTime,0.1);
		if (orderCountToday == 1)
		{
			openOrderHigh = intradayHigh;
			openOrderLow = intradayLow;

		}
			

		//从第一单开始：close negative if 浮盈大于20点后
		if (orderCountToday >= 1)
		{				
			//entryPrice = getLastestOrderPriceEasy(B_PRIMARY_RATES, &isOpen);
			//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			//这策略不允许同时开出多个open order,所以这里index,应该都是0			 
			//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
			if (isOpen == TRUE)
			{			
				entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				if (side == SELL)
				{
					//如果过了20点后，出现第一个5M的阳线，离场。
					if (entryPrice - openOrderLow > 0.3 &&
						//entryPrice - pParams->bidAsk.ask[0] < 0
						//iClose(B_SECONDARY_RATES, 1) > iOpen(B_SECONDARY_RATES,1)
						entryPrice - pParams->bidAsk.ask[0] < 0.3
						//pParams->bidAsk.ask[0] - openOrderLow > 0.2
						)						
					{
						//closeAllWithNegativeEasy(5, currentTime, 3); //close them intraday on break event
						closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
						return SUCCESS;
					}
				}

				if (side == BUY)
				{
					//如果过了20点后，出现第一个5M的阴线，离场
					if (openOrderHigh - entryPrice > 0.3 && 
						//entryPrice - pParams->bidAsk.bid[0] > 0
						//iClose(B_SECONDARY_RATES, 1) < iOpen(B_SECONDARY_RATES, 1)
						pParams->bidAsk.bid[0] - entryPrice < 0.3
						//openOrderHigh - pParams->bidAsk.bid[0] > 0.2
						)
					{
						//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
						closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
						return SUCCESS;
					}
				}
			}			
		}	
		
		//if (orderCountToday == 1 && pParams->orderInfo[latestOrderIndex].ticket != 0 && pParams->orderInfo[latestOrderIndex].isOpen == FALSE && pParams->orderInfo[latestOrderIndex].profit < 0
		//	&& fabs(pParams->orderInfo[latestOrderIndex].openPrice - pParams->orderInfo[latestOrderIndex].closePrice) <= 0.1)
		//	isBreakEvent = TRUE;

		if (side == SELL)
		{
			
			if (isOpen == TRUE)
			{
				if (orderCountToday == 2)
				{
					openOrderLow = min(openOrderLow, pParams->orderInfo[latestOrderIndex].openPrice);
				}


				if (pParams->bidAsk.ask[0] - openOrderLow >= Range)
					//if (pParams->bidAsk.ask[0] - intradayLow >= Range)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

					//pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
					//pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0 )
					{
						pIndicators->risk = pow(2, pIndicators->lossTimes);
						pIndicators->entrySignal = 1;
					}
					pIndicators->exitSignal = EXIT_SELL;
				}
			}

		}

		if (side == BUY)
		{
			if (isOpen == TRUE )
			{

				if (orderCountToday == 2)
				{
					openOrderHigh = max(openOrderHigh, pParams->orderInfo[latestOrderIndex].openPrice);
				}

				if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)
					//if (intradayHigh - pParams->bidAsk.bid[0] >= Range)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
					//pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
					//pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0)
					{
						pIndicators->risk = pow(2, pIndicators->lossTimes);
						pIndicators->entrySignal = -1;
					}

					pIndicators->exitSignal = EXIT_BUY;
				}

			}
		}

	}


	//保存交易的信息
	//saveTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo, (BOOL)pParams->settings[IS_BACKTESTING]);

	return SUCCESS;
}


/*
跨日交易
需要认识行情
可能点：
1. 使用4H的趋势信号。
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


	// 满足日图历史的趋势分析。
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

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
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

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
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

	//计算日内高低点
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
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
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

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
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

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
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
					pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
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
				//如果过了20点后，出现第一个5M的阴线，离场
				if (openOrderHigh - entryPrice > pIndicators->takePrice &&
					pParams->bidAsk.bid[0] - entryPrice < pIndicators->takePrice
					)
				{
					//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
					closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
					pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
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

/*
使用在5M图
1. 使用50M, 200M均线的金叉，死叉
2. 定义出过度的区间： 
   . 必须突破近期的高低点（日内）
   . 交叉的差距必须10点以上（10% predicatedDailyATR)
3. 资金管理
  1. No TP, No SL. 根据均线的颜色出场。可以设置100点的保护止损。
  2. TP=30点，SL= 100点。每天都交易。 
     还是只做一波趋势？
*/

/*
只用于XAUUSD 15M
亚洲时段，1/2 risk 入场 试单。
如果到了1/2ATR,就按照当时方向，入场。如果之前的单子，不对，就止损离场，反手加倍入场。
到了欧洲，美洲时段：
如果没有到1/2ATR，就按照亚洲时段的区间边沿布局。如果突破边缘交易。使用stop 单？
如果之前错了，止损，反手加倍入场。
*/
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_Daily_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);
	double ATR0 = iAtr(B_DAILY_RATES, 1, 0);
	int signal = 0;
	double intradayHigh_exclude_current_bar = intradayHigh, intradayLow_exclude_current_bar = intradayLow;
	int count;
	double openOrderHigh, openOrderLow;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	//closeAllWithNegativeEasy(1, currentTime, 3);

	pIndicators->splitTradeMode = 16;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->pDailyPredictATR < 10)
		return SUCCESS;

	count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count > 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh_exclude_current_bar, &intradayLow_exclude_current_bar);
	else
		return SUCCESS;
	//iSRLevels_close(pParams, pIndicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh_exclude_current_bar, &intradayLow_exclude_current_bar);

	//if (ATR0 >= pBase_Indicators->pDailyATR / 2)

	if (ATR0 < 5)
	{
		if (timeInfo1.tm_hour == 7 && timeInfo1.tm_min < 3 && hasOpenOrder() == FALSE) // 7am
		{
			if (pIndicators->bbsTrend_primary == 1)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
				pIndicators->entrySignal = 1;
			}
			else if (pIndicators->bbsTrend_primary == -1)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
				pIndicators->entrySignal = -1;

			}
			return SUCCESS;
		}

	}
	else
	{
		//pIndicators->risk = 0.5;
		if (hasOpenOrder() == FALSE)
		{
			if (pParams->bidAsk.ask[0] - intradayLow > 5) // Buy signal
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;

				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < 10 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = 1;
				}
				pIndicators->exitSignal = EXIT_SELL;

			}

			if (intradayHigh - pParams->bidAsk.bid[0] > 5) // Sell signal
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < 10 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = -1;
				}

				pIndicators->exitSignal = EXIT_BUY;
			}
		}
		else
		{

			if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == SELL && pParams->bidAsk.ask[0] > openOrderHigh) // Buy signal
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;

				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = 1;
				}
				pIndicators->exitSignal = EXIT_SELL;

			}

			if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == BUY && pParams->bidAsk.bid[0] < openOrderLow) // Sell signal
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = -1;
				}

				pIndicators->exitSignal = EXIT_BUY;
			}

		}
	}
	return SUCCESS;
}

/*
使用M50,M200的金叉，死叉，adjust: ATR_primary(20)
如果需要改变方向，必须超过adjust。
一开始，每周一1am入场。
当方向改变后，加倍入场。
不要超过5次。
时间框架30M
*/
AsirikuyReturnCode workoutExecutionTrend_Weekly_Swing_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1_execution;
	time_t currentTime;
	struct tm timeInfo1;
	BOOL isOpen;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 13;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	shift1_execution = shift1Index_primary;

	//过滤掉脱欧的那个周，只是用于测试
	if (timeInfo1.tm_year == 116 && timeInfo1.tm_mon == 9 && timeInfo1.tm_mday >= 3 && timeInfo1.tm_mday <= 7)
		return SUCCESS;

	if (timeInfo1.tm_wday == 1 && timeInfo1.tm_hour == 0)
	{
		return SUCCESS;
	}


	if (hasSameWeekOrderEasy(currentTime, &isOpen) == FALSE)  //如果没有开仓, 就可以开仓
	{
		if (pBase_Indicators->maTrend > 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
			pIndicators->entrySignal = 1;
		}

		if (pBase_Indicators->maTrend < 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
			pIndicators->entrySignal = -1;

		}
	}
	else
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, maTrend = %ld,ma_Signal = %ld",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, pBase_Indicators->ma_Signal);

		if (pBase_Indicators->maTrend > 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
			if (pBase_Indicators->ma_Signal > 0)
			{
				pIndicators->lossTimes = getLossTimesInWeekEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInWeekEasy(currentTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes);

				if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = 1;
				}
			}
			pIndicators->exitSignal = EXIT_SELL;
		}
		else if (pBase_Indicators->maTrend < 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
			if (pBase_Indicators->ma_Signal < 0)
			{
				pIndicators->lossTimes = getLossTimesInWeekEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInWeekEasy(currentTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes);

				if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0)
				{
					pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = -1;
				}
			}
			pIndicators->exitSignal = EXIT_BUY;
		}
	}

	return SUCCESS;
}
AsirikuyReturnCode workoutExecutionTrend_Weekly_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1_execution;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 13;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	shift1_execution = shift1Index_primary;

	//if (timeInfo1.tm_min > 3)
	//	return SUCCESS;

	//过滤掉脱欧的那个周，只是用于测试
	if (timeInfo1.tm_year == 116 && timeInfo1.tm_mon == 9 && timeInfo1.tm_mday >= 3 && timeInfo1.tm_mday <= 7)
		return SUCCESS;


	//每周一，根据BBS Trend入场
	if (timeInfo1.tm_wday == 1)
	{
		if (timeInfo1.tm_hour == 0)
		{
			return SUCCESS;
		}

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min <3)
		{
			if (pBase_Indicators->maTrend > 0)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
				pIndicators->entrySignal = 1;
			}

			if (pBase_Indicators->maTrend < 0)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
				pIndicators->entrySignal = -1;

			}
			return SUCCESS;
		}
	}

	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, maTrend = %ld,ma_Signal = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, pBase_Indicators->ma_Signal);

	if (pBase_Indicators->maTrend > 0)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR*1.2;
		if (pBase_Indicators->ma_Signal > 0)
			//if (pIndicators->entryPrice - pBase_Indicators->ma1H200M > 1)
		{
			pIndicators->lossTimes = getLossTimesInWeekEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInWeekEasy(currentTime);

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes);


			if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = 1;
			}
		}
		pIndicators->exitSignal = EXIT_SELL;
	}
	else if (pBase_Indicators->maTrend < 0)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR*1.2;
		if (pBase_Indicators->ma_Signal < 0)
			//if (pBase_Indicators->ma1H200M - pIndicators->entryPrice > 1)
		{
			pIndicators->lossTimes = getLossTimesInWeekEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInWeekEasy(currentTime);

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes);


			if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = -1;
			}
		}
		pIndicators->exitSignal = EXIT_BUY;
	}


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_WeeklyATR_Prediction(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	predictWeeklyATR_LongerTerm(pParams, pBase_Indicators);
	
	savePredicatedWeeklyATR(pParams->tradeSymbol, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, (BOOL)pParams->settings[IS_BACKTESTING]);

	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pWeeklyPredictATR=%lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	return SUCCESS;
}


AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
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
	double adjust = 0.1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;



	

	int maxTradeTime = 2, latestOrderIndex = 0,orderCountToday = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 21;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	//Range = 5;
	Range = min(5, pBase_Indicators->pDailyPredictATR / 2);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && XAUUSD_DayTrading_Allow_Trade_Ver2(pParams, pIndicators, pBase_Indicators,TRUE) == FALSE)
		return SUCCESS;

	//计算日内高低点
	count = (timeInfo1.tm_hour-1)  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
	}
	else
		return SUCCESS;
	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);


	//必须 close all orders before EOD
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	// secondary rate is 5M , priarmy rate is 1M
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)
	{
		//在80点内，必须入场，如果放弃了，就放弃当日交易。
		if (ATR0 > 8)
			return SUCCESS;

		if (pParams->bidAsk.ask[0] - intradayLow >= Range
			&& pBase_Indicators->maTrend > 0
			)
		{

			//upperBBand = iBBands(B_SECONDARY_RATES, 50, 2, 0, 1);

			//if (upperBBand > 0 && iClose(B_SECONDARY_RATES, 1) > upperBBand)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
				pIndicators->entrySignal = 1;
			}
		}

		if (intradayHigh - pParams->bidAsk.bid[0] >= Range
			&& pBase_Indicators->maTrend < 0
			)
		{
			//lowerBBand = iBBands(B_SECONDARY_RATES, 50, 2, 2, 1);

			//
			//if (lowerBBand > 0 && iClose(B_SECONDARY_RATES, 1) < lowerBBand)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
				pIndicators->entrySignal = -1;
			}
		}

	}
	else
	{
		side = getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow);
		//如果已经有了一个trade，那就是第二次交易，要使用日内的高低点
		//如果有了2个trades,那就是第三次交易了，使用orderHigh, orderLow.
		orderCountToday = getOrderCountTodayEasy(currentTime);
		if (orderCountToday == 1)
		{
			openOrderHigh = intradayHigh;
			openOrderLow = intradayLow;			
		}

		////从第一单开始：close negative if 浮盈大于20点后,break event
		//if (getOrderCountTodayEasy(currentTime) >= 1)
		//{
		//	//entryPrice = getLastestOrderPriceEasy(B_PRIMARY_RATES, &isOpen);
		//	//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
		//	//这策略不允许同时开出多个open order,所以这里index,应该都是0			 
		//	if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		//	{
		//		entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;
		//		if (side == SELL)
		//		{
		//			if (entryPrice - openOrderLow > 2 && entryPrice - pParams->bidAsk.ask[0] < 0)						
		//				closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
		//		}

		//		if (side == BUY)
		//		{
		//			if (openOrderHigh - entryPrice > 2 && entryPrice - pParams->bidAsk.bid[0] > 0)						
		//				closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
		//		}
		//	}
		//}

		if (isOpen == TRUE && side == SELL)
		{
			if (orderCountToday == 2)
			{
				openOrderLow = min(openOrderLow, pParams->orderInfo[latestOrderIndex].openPrice);
			}
			if (pParams->bidAsk.ask[0] - openOrderLow >= Range)
				//if (pParams->bidAsk.ask[0] - intradayLow >= Range)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < maxTradeTime && pIndicators->winTimes == 0)
				{
					//pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = 1;
				}
				pIndicators->exitSignal = EXIT_SELL;
			}

		}

		if (isOpen == TRUE && side == BUY)
		{
			if (orderCountToday == 2)
			{
				openOrderHigh = max(openOrderHigh, pParams->orderInfo[latestOrderIndex].openPrice);
			}
			if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)
				//if (intradayHigh - pParams->bidAsk.bid[0] >= Range)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes < maxTradeTime && pIndicators->winTimes == 0)
				{
					//pIndicators->risk = pow(2, pIndicators->lossTimes);
					pIndicators->entrySignal = -1;
				}

				pIndicators->exitSignal = EXIT_BUY;
			}

		}

	}

		
	return SUCCESS;
}

/*
这是5M的均线策略，配合保利通道形成买入，观望，卖出信号。


好处： 
1. 避免日内剧烈波动
2. 适合一切快速品种，GBPJPY, XAUUSD.....

坏处： 止损不确定，可以比较大。

目标: 30点
止损：保护止损100点， 以卖出信号为准。

黄色离场。

实验跨日：

问题：
1. 要不要跨日？
2. 要不要20点平保？
3. 要不要仓量翻倍？
4. 要不要跨日翻倍？
5. 要不要引入日波幅的过滤？
6. 止盈的位置，要不要使用预测的日波幅？

跨日交易
*/
//AsirikuyReturnCode workoutExecutionTrend_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
//{
//	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
//	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
//	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
//	time_t currentTime, openTime;
//	struct tm timeInfo1,timeInfo2;
//	double ATR0;	
//	double openOrderHigh, openOrderLow;
//	int count, isOpen;
//	double adjust = 0.1;
//	char       timeString[MAX_TIME_STRING_SIZE] = "";
//	OrderType side;
//	Order_Info orderInfo;
//	double entryPrice;
//	int maxTradeTime = 1, latestOrderIndex = 0, orderCountToday = 0;
//	double upperBBand, lowerBBand;
//
//	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow,intradayCloseHigh,intradayCloseLow;
//	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
//	double range;
//	double floatingTP;
//	double daily_baseline = 0.0, dailyHigh = 0.0, dailyLow = 0.0, preDailyClose;	
//	int executionTrend;
//
//	BOOL isSameDayOrder = FALSE, isSameDayClosedOrder = FALSE,isLastOrderProfit = FALSE;
//	BOOL isPreviousDayOrder = FALSE;
//	BOOL shouldFilter = TRUE;
//	int takeProfitMode = 0;
//
//	double riskCapBuy = 0;
//	double riskCapSell = 0;
//
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
//	safe_gmtime(&timeInfo1, currentTime);
//
//
//	safe_timeString(timeString, currentTime);
//
//
//	pIndicators->splitTradeMode = 22;
//	pIndicators->risk = 1;
//	pIndicators->tpMode = 0;
//
//	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
//		executionTrend = 0;
//	else if (pBase_Indicators->dailyTrend > 0)
//		executionTrend = 1;
//	else if (pBase_Indicators->dailyTrend < 0)
//		executionTrend = -1;
//	else
//		executionTrend = 0;
//
//
//	//pIndicators->executionTrend = executionTrend;
//	//计算日内高低点
//	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
//	if (count > 1)
//	{
//		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
//		iSRLevels_close(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayCloseHigh, &intradayCloseLow);
//	}
//	else
//		return SUCCESS;
//
//	intradayLow = min(close_prev1, intradayLow);
//	intradayHigh = max(close_prev1, intradayHigh);
//	pIndicators->atr0 = fabs(intradayHigh - intradayLow);
//	ATR0 = pIndicators->atr0;
//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);
//
//	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
//	if (latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE){
//		side = pParams->orderInfo[latestOrderIndex].type;
//
//		safe_gmtime(&timeInfo2, pParams->orderInfo[latestOrderIndex].openTime);
//		if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
//			isSameDayOrder = TRUE;
//		if (isSameDayOrder == TRUE && timeInfo2.tm_hour < pIndicators->startHour)
//		{
//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, same day opentime  %d is less than %d. It should be a manual take over order.",
//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo2.tm_hour,pIndicators->startHour);
//			isSameDayOrder = FALSE;
//		}
//	}
//	else
//		side = NONE;
//
//	////如果上一次的单子是前一天，而且是止盈离场，而且是有趋势，而且收盘高过止盈价格
//	////1. 开盘入场
//	////2. 如果回到上次离场，入场
//	////3. 取消入场检查，可以15点后入场
//	//if (latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].isOpen == FALSE)
//	//{		
//	//	safe_gmtime(&timeInfo2, pParams->orderInfo[latestOrderIndex].openTime);
//
//	//	//if (timeInfo1.tm_yday == timeInfo2.tm_yday + 1
//	//	//	|| (timeInfo1.tm_yday == 0 && timeInfo2.tm_yday == 365 && timeInfo1.tm_year == timeInfo2.tm_year + 1))
//	//	//	isPreviousDayOrder = TRUE;
//
//	//	if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
//	//		isSameDayOrder = TRUE;
//
//	//	safe_gmtime(&timeInfo2, pParams->orderInfo[latestOrderIndex].closeTime);
//
//	//	if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
//	//	{
//	//		isSameDayClosedOrder = TRUE;			
//	//	}
//
//	//	if (pParams->orderInfo[latestOrderIndex].profit >= 0)
//	//	{
//	//		isLastOrderProfit = TRUE;
//	//	}
//	//}
//
//
//	// TODO: 需要修改BASE,支持在收盘重新计算Trend，但是需要时间。
//	//暂时选在在第二天开盘离场，实盘中有点差的问题，还有周末跳空的问题。
//	//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
//	//	if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
//	//{
//
//	//	
//	//		if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
//	//			if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE && timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
//	//		{
//
//	//			if (pParams->orderInfo[latestOrderIndex].type == BUY &&
//	//				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 0
//	//				executionTrend <= 0
//	//				)
//	//				pIndicators->exitSignal = EXIT_ALL;
//	//			if (pParams->orderInfo[latestOrderIndex].type == SELL &&
//	//				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 0
//	//				executionTrend >= 0
//	//				)
//	//				pIndicators->exitSignal = EXIT_ALL;
//	//			return SUCCESS;
//	//		}
//	//	
//	//	
//	//}
//
//	
//
//	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
//	{
//
//		//1. TP = SL = 0.33*ATR(20)
//		pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3);
//		pIndicators->stopLoss = pBase_Indicators->pDailyATR/3;
//		pIndicators->takePrice = pIndicators->stopLoss;
//		
//		floatingTP = pIndicators->takePrice;
//		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);
//
//		//if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
//		//{
//		//	if (side == BUY)
//		//	{
//		//		if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss)
//		//		{
//		//			pIndicators->executionTrend = 1;
//		//			pIndicators->entryPrice = pParams->bidAsk.ask[0];
//		//			pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
//		//		}
//		//		else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss 
//		//			//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss
//		//			)
//		//		{
//		//			pIndicators->executionTrend = 1;
//		//			pIndicators->entryPrice = pParams->bidAsk.ask[0];
//		//			pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
//		//		}
//		//	}
//
//		//	if (side == SELL)
//		//	{
//		//		if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
//		//		{
//		//			pIndicators->executionTrend = -1;
//		//			pIndicators->entryPrice = pParams->bidAsk.bid[0];
//		//			pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
//		//		}
//
//		//		if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss 
//		//			//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss
//		//			)
//		//		{
//		//			pIndicators->executionTrend = -1;
//		//			pIndicators->entryPrice = pParams->bidAsk.bid[0];
//		//			pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
//		//		}
//
//
//		//	}			
//		//}
//	
//			
//
//		//如果是当天的单子，入场后，不需要过滤。		
//		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1
//			&& isSameDayOrder == FALSE			
//			&& XAUUSD_DayTrading_Allow_Trade_Ver2(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
//		{			
//			return SUCCESS;
//		}
//
//		takeProfitMode = 1;
//		
//	}
//	
//	if (side == NONE)
//	{
//		//if (latestOrderIndex >= 0)
//		//	side = pParams->orderInfo[latestOrderIndex].type;
//
//		if (pBase_Indicators->maTrend > 0 
//			//&& pBase_Indicators->dailyTrend >=-1			
//			)
//		{
//			upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);
//
//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));
//
//
//			if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
//			{
//				pIndicators->executionTrend = 1;
//				pIndicators->entryPrice = pParams->bidAsk.ask[0];
//				pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
//
//				//if (pIndicators->winTimes == 0 && pIndicators->lossTimes < maxTradeTime && (side == SELL || side == NONE))
//				//if ((isSameDayOrder == FALSE && isLastOrderProfit == FALSE) || isSameDayClosedOrder == FALSE)
//				if (isSameDayClosedOrder == FALSE)
//					pIndicators->entrySignal = 1;
//
//				pIndicators->exitSignal = EXIT_SELL;
//			}
//
//		}
//
//		if (pBase_Indicators->maTrend < 0 
//			//&& pBase_Indicators->dailyTrend <=1			
//			)
//		{
//			lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);
//
//			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));
//
//			if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
//			{
//				pIndicators->executionTrend = -1;
//				pIndicators->entryPrice = pParams->bidAsk.bid[0];
//				pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
//
//				//if (pIndicators->winTimes == 0 && pIndicators->lossTimes < maxTradeTime && (side == BUY || side == NONE))				
//				//if ((isSameDayOrder == FALSE && isLastOrderProfit == FALSE) || isSameDayClosedOrder == FALSE)
//				if ( isSameDayClosedOrder == FALSE)
//					pIndicators->entrySignal = -1;
//
//				pIndicators->exitSignal = EXIT_BUY;
//			}
//		}
//	}
//	else
//	{
//		if (getOrderCountEasy() == 1){
//			if (side == BUY){
//				pIndicators->entryPrice = pParams->bidAsk.bid[0] - pIndicators->adjust;
//				pIndicators->executionTrend = 1;
//				pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
//			}
//			else{
//				pIndicators->entryPrice = pParams->bidAsk.ask[0] + pIndicators->adjust;
//				pIndicators->executionTrend = -1;
//				pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
//			}
//
//		}
//		
//	}
//
//
//	return SUCCESS;
//}

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

	//pIndicators->executionTrend = executionTrend;
	//计算日内高低点
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
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	//1. check MA 5M
	//2. check BBand
	//3. if have open order, not 

	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);	
	oldestOpenOrderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);

	if (oldestOpenOrderIndex >= 0){
		side = pParams->orderInfo[oldestOpenOrderIndex].type;

		safe_gmtime(&timeInfo2, pParams->orderInfo[oldestOpenOrderIndex].openTime);
		if (timeInfo1.tm_year == timeInfo2.tm_year &&  timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday)
			isSameDayOrder = TRUE;
		if (isSameDayOrder == TRUE && timeInfo2.tm_hour < pIndicators->startHour)
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, same day opentime  %d is less than %d. It should be a manual take over order.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo2.tm_hour, pIndicators->startHour);
			isSameDayOrder = FALSE;
		}
	}
	else
		side = NONE;

	//如果上一次的单子是前一天，而且是止盈离场，而且是有趋势，而且收盘高过止盈价格
	//1. 开盘入场
	//2. 如果回到上次离场，入场
	//3. 取消入场检查，可以15点后入场
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


	// TODO: 需要修改BASE,支持在收盘重新计算Trend，但是需要时间。
	//暂时选在在第二天开盘离场，实盘中有点差的问题，还有周末跳空的问题。
	if (oldestOpenOrderIndex>=0 && timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
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

	//oldestOpenOrderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);

	if (oldestOpenOrderIndex >= 0 &&
		((pParams->orderInfo[oldestOpenOrderIndex].type == BUY && pParams->orderInfo[oldestOpenOrderIndex].stopLoss - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= -2 * pIndicators->adjust) ||
		(pParams->orderInfo[oldestOpenOrderIndex].type == SELL &&  pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->orderInfo[oldestOpenOrderIndex].stopLoss >= -2 * pIndicators->adjust))
		){
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,stopLoss =%lf. it is ok to add new positions in a long term trend now.",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->orderInfo[oldestOpenOrderIndex].stopLoss);
		isAddPosition = TRUE;
	}

	if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{

		pIndicators->atr_euro_range = (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3;
		pIndicators->stopLoss = pIndicators->atr_euro_range*0.93;
		pIndicators->takePrice = pIndicators->stopLoss * 0.4;

		floatingTP = pIndicators->takePrice;
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		{
			if (side == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss
					//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
				}
			}

			if (side == SELL)
			{
				if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}

				if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss
					//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
				}
			}
		}

		//如果是当天的单子，入场后，不需要过滤。		
		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1
			&& isSameDayOrder == FALSE
			&& XAUUSD_DayTrading_Allow_Trade_Ver2(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
		{
			return SUCCESS;
		}

		takeProfitMode = 1;
	}

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{

		//if (timeInfo1.tm_year == 2019)
		pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3);
		pIndicators->stopLoss = pIndicators->atr_euro_range*0.93;
		pIndicators->takePrice = max(3, pIndicators->stopLoss * 0.4);

		floatingTP = pIndicators->takePrice;
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && oldestOpenOrderIndex>= 0)
		{
			if (side == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice < 2 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= 2 * pIndicators->stopLoss
					//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice + pIndicators->stopLoss;
				}
				//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 3 * pIndicators->stopLoss )
				//{
				//	pIndicators->executionTrend = 1;
				//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + 2* pIndicators->stopLoss;
				//}
			}

			if (side == SELL)
			{
				if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
				}

				if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss
					//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice - pIndicators->stopLoss;
				}
				//if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss)
				//{
				//	pIndicators->executionTrend = -1;
				//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - 2* pIndicators->stopLoss;
				//}

			}
		}

		//if (isPreviousDayOrder == TRUE && pParams->orderInfo[latestOrderIndex].profit > 0)
		//{
		//	if (pParams->orderInfo[latestOrderIndex].type == BUY && executionTrend > 0 && close_prev1 > pParams->orderInfo[latestOrderIndex].closePrice)
		//		shouldFilter = FALSE;
		//	if (pParams->orderInfo[latestOrderIndex].type == SELL && executionTrend < 0 && close_prev1 < pParams->orderInfo[latestOrderIndex].closePrice)
		//		shouldFilter = FALSE;
		//}
		//winningOrdersToday = getWinTimesInDayEasy(currentTime);

		if (isAddPosition == TRUE 					
			//winningOrdersToday == 0 &&
			//(hasSameDayOrderEasy(currentTime, &isOpen) == FALSE || isOpen == FALSE)
			){
			pIndicators->tradeMode = 2;
			pIndicators->risk = 0.5;
			

			if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY 
				){
				
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				addPositionBaseLine = pBase_Indicators->dailyPivot;
				pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
				if (
					(preLow < addPositionBaseLine && preClose >  addPositionBaseLine
					|| (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 5)) &&
					//pIndicators->fast > 0 &&
					//pIndicators->fast >= pIndicators->slow &&
					pIndicators->entryPrice - pIndicators->stopLoss > pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
					!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)){
					pIndicators->executionTrend = 1;
					//pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
					pIndicators->entrySignal = 1;
					return SUCCESS;
				}
			}

			if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL				
				){
				
				pIndicators->entryPrice = pParams->bidAsk.bid[0];	
				addPositionBaseLine = pBase_Indicators->dailyPivot;
				pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
				if ((preHigh > addPositionBaseLine && preClose < addPositionBaseLine
					|| (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 5 )) &&
					//pIndicators->fast < 0 &&
					//pIndicators->fast <= pIndicators->slow &&
					pIndicators->entryPrice + pIndicators->stopLoss < pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
					!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)){
					pIndicators->executionTrend = -1;
					//pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
					pIndicators->entrySignal = -1;
					return SUCCESS;
				}
			}

		}


		pIndicators->endHour = 23;
		//如果是当天的单子，入场后，不需要过滤。		
		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1
			&& isSameDayOrder == FALSE
			&& XAUUSD_DayTrading_Allow_Trade_Ver2(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
		{
			return SUCCESS;
		}

		takeProfitMode = 1;

		riskCapBuy = parameter(AUTOBBS_RISK_CAP);
		riskCapSell = riskCapBuy - 2;

		//riskCapBuy = 0;
		//riskCapSell = 0;

	}
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{

		pIndicators->atr_euro_range = (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3;
		pIndicators->stopLoss = pIndicators->atr_euro_range*0.93;
		pIndicators->takePrice = pIndicators->stopLoss * 0.4;

		floatingTP = pIndicators->takePrice;
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		{
			if (side == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss
					//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
				}
				//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 3 * pIndicators->stopLoss )
				//{
				//	pIndicators->executionTrend = 1;
				//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + 2* pIndicators->stopLoss;
				//}
			}

			if (side == SELL)
			{
				if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}

				if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss
					//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
				}
				//if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss)
				//{
				//	pIndicators->executionTrend = -1;
				//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - 2* pIndicators->stopLoss;
				//}

			}
		}

		riskCapBuy = parameter(AUTOBBS_RISK_CAP);
		riskCapSell = 0;

		//如果是当天的单子，入场后，不需要过滤。		
		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1
			&& isSameDayOrder == FALSE
			&& BTCUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
		{
			return SUCCESS;
		}

		takeProfitMode = 1;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{

		//pIndicators->stopLoss = pIndicators->atr_euro_range*0.88;

		//如果有趋势，加大Range
		if ((int)parameter(AUTOBBS_RANGE) == 1 && executionTrend != 0)
			pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 2 * 0.8);

		pIndicators->stopLoss = pIndicators->atr_euro_range* 1.1;
		pIndicators->takePrice = max(0.3, pIndicators->atr_euro_range * 0.35);

		floatingTP = pIndicators->takePrice;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,pDailyPredictATR=%lf, pDailyMaxATR= %lf,atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, pBase_Indicators->pDailyMaxATR, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && oldestOpenOrderIndex>=0)
		{
			if (side == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice > pIndicators->stopLoss
					//pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->orderInfo[oldestOpenOrderIndex].stopLoss
					//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
				}
				//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= 2 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice < 3 * pIndicators->stopLoss)
				//{
				//	pIndicators->executionTrend = 1;
				//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice + pIndicators->stopLoss;
				//}
				//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= 3 * pIndicators->stopLoss
				//	//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 4 * pIndicators->stopLoss
				//	)
				//{
				//	pIndicators->executionTrend = 1;
				//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice + 2 * pIndicators->stopLoss;
				//}
			}

			if (side == SELL)
			{
				if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss
					//pParams->orderInfo[oldestOpenOrderIndex].stopLoss - pParams->orderInfo[oldestOpenOrderIndex].openPrice
					//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
				}
				//else if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss && pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
				//{
				//	pIndicators->executionTrend = -1;
				//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice - pIndicators->stopLoss;
				//}
				//else if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss
				//	//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss
				//	)
				//{
				//	pIndicators->executionTrend = -1;
				//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
				//	pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice - 2 * pIndicators->stopLoss;
				//}

			}
		}

		riskCapBuy = parameter(AUTOBBS_RISK_CAP);
		riskCapSell = 0;

		if (isAddPosition == TRUE){
			pIndicators->tradeMode = 2;
			pIndicators->risk = 0.5;

			if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY){

				pIndicators->entryPrice = pParams->bidAsk.ask[0];		
				addPositionBaseLine = pBase_Indicators->dailyS1;
				if (
					(preLow <  addPositionBaseLine && preClose >addPositionBaseLine 
					//|| preLow <  pBase_Indicators->dailyPivot && preClose >pBase_Indicators->dailyPivot
					//|| (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 5)
					) &&
					pIndicators->entryPrice - pIndicators->stopLoss > pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
					pIndicators->entryPrice <= pBase_Indicators->weeklyR2 &&
					//pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice < riskCapBuy * pIndicators->stopLoss &&
					!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)){
					pIndicators->executionTrend = 1;
					pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
					pIndicators->entrySignal = 1;
					return SUCCESS;
				}
			}

			if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL
				){

				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				addPositionBaseLine = pBase_Indicators->dailyR1;
				if ((preHigh > addPositionBaseLine && preClose < addPositionBaseLine
					//|| preHigh > pBase_Indicators->dailyPivot && preClose < pBase_Indicators->dailyPivot
					//||(timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 5)
					) &&
					pIndicators->entryPrice + pIndicators->stopLoss < pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
					pIndicators->entryPrice >= pBase_Indicators->weeklyS2 &&
					!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)){
					pIndicators->executionTrend = -1;
					pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
					pIndicators->entrySignal = -1;
					return SUCCESS;
				}
			}

		}

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
			isSameDayOrder == FALSE &&
			GBPJPY_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{

		//pIndicators->stopLoss = pIndicators->atr_euro_range*0.88;

		//如果有趋势，加大Range
		if ((int)parameter(AUTOBBS_RANGE) == 1 && executionTrend != 0)
			pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 2 * 0.8);

		pIndicators->stopLoss = pIndicators->atr_euro_range* 1.1;
		pIndicators->takePrice = max(0.003, pIndicators->atr_euro_range * 0.35);

		//floatingTP = pIndicators->takePrice;
		floatingTP = 0;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		{
			if (side == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss
					//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 3 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 4 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + 2 * pIndicators->stopLoss;
				}
			}

			if (side == SELL)
			{
				if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss
					//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}
				else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
				}
				else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - 2 * pIndicators->stopLoss;
				}

			}
		}

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPUSD_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
			return SUCCESS;
	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{

		//pIndicators->stopLoss = pIndicators->atr_euro_range*0.88;

		//如果有趋势，加大Range
		//if ((int)parameter(AUTOBBS_RANGE) == 1 && pIndicators->executionTrend != 0)
		pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 2 * 0.8);

		pIndicators->stopLoss = pIndicators->atr_euro_range* 1.1;
		pIndicators->takePrice = max(0.0015, pIndicators->atr_euro_range * 0.35);

		//floatingTP = pIndicators->takePrice;
		floatingTP = 0;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

		if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min >= 0 && timeInfo1.tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
		{
			if (side == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss
					//&& pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 3 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 4 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + 2 * pIndicators->stopLoss;
				}
			}

			if (side == SELL)
			{
				if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss
					//&& pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				}
				else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
				}
				else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss && pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - 2 * pIndicators->stopLoss;
				}

			}
		}

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPUSD_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
			return SUCCESS;
	}

	if (side == NONE)
	{
		enterOrder_MultipleDay(pParams, pIndicators, pBase_Indicators, riskCapBuy, riskCapSell, isSameDayClosedOrder);
	}
	else
	{
		//side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);

		//orderCountToday = getOrderCountTodayEasy(currentTime);
		//if (orderCountToday == 1)
		//{
		//	openOrderHigh = intradayHigh;
		//	openOrderLow = intradayLow;

		//}

		modifyOrder_MultipleDay(pParams, pIndicators, pBase_Indicators, oldestOpenOrderIndex, intradayHigh, intradayLow, floatingTP, takeProfitMode, TRUE);
	}


	

	return SUCCESS;
}

AsirikuyReturnCode modifyOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, double openOrderHigh, double openOrderLow, double floatingTP, int takeProfitMode,BOOL isLongTerm){
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double entryPrice = 0;
	OrderType side;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	side = pParams->orderInfo[latestOrderIndex].type;

	if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, takeProfitMode =%d, lastClose=%lf, lastOpen=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, takeProfitMode, iClose(B_PRIMARY_RATES, 1), iOpen(B_PRIMARY_RATES, 1));

		if (side == SELL)
		{
			if (entryPrice - openOrderLow > pIndicators->takePrice &&
				entryPrice - pParams->bidAsk.ask[0] < floatingTP
				&& (takeProfitMode == 0 || iClose(B_PRIMARY_RATES, 1) > iOpen(B_PRIMARY_RATES, 1))
				)
			{
				//closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
				//pIndicators->exitSignal = EXIT_SELL;
				//closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
				if (isLongTerm == TRUE){
					pIndicators->exitSignal = EXIT_SELL;
				}
				else{
					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
				}
				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
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
			//如果过了20点后，出现第一个5M的阴线，离场
			if (openOrderHigh - entryPrice > pIndicators->takePrice &&
				pParams->bidAsk.bid[0] - entryPrice < floatingTP
				&& (takeProfitMode == 0 || iClose(B_PRIMARY_RATES, 1) < iOpen(B_PRIMARY_RATES, 1))
				)
			{
				//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
				//closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
				if (isLongTerm == TRUE)
					pIndicators->exitSignal = EXIT_BUY;
				else 
					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
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

AsirikuyReturnCode enterOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double riskCapBuy, double riskCapSell, BOOL isSameDayClosedOrder){
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double upperBBand, lowerBBand;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (pBase_Indicators->maTrend > 0
		&& pBase_Indicators->dailyTrend >= -1
		)
	{
		upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


		if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;

			pIndicators->riskCap = riskCapBuy;
			
			if (isSameDayClosedOrder == FALSE)
				pIndicators->entrySignal = 1;

			pIndicators->exitSignal = EXIT_SELL;
		}

	}

	if (pBase_Indicators->maTrend < 0
		&& pBase_Indicators->dailyTrend <= 1
		)
	{
		lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

		if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;

			pIndicators->riskCap = riskCapSell;

			if (isSameDayClosedOrder == FALSE)
				pIndicators->entrySignal = -1;

			pIndicators->exitSignal = EXIT_BUY;
		}
	}
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
	//计算日内高低点
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
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
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
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, same day opentime  %d is less than %d. It should be a manual take over order.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo2.tm_hour, pIndicators->startHour);
			isSameDayOrder = FALSE;
		}
	}
	else
		side = NONE;

	// TODO: 需要修改BASE,支持在收盘重新计算Trend，但是需要时间。
	//暂时选在在第二天开盘离场，实盘中有点差的问题，还有周末跳空的问题。
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
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);
		

		// filter 非农
		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
		{

			strcpy(pIndicators->status, "Filter Non-farm day");

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.");

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, %s",
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
纯空间交易法
入场：
1.在新的一天，if no existing open order, 开盘后,根據預計波幅，開出多 空limit orders.
2.仓量是1%,two orders. One order is no TP. the other one with TP.
3.止损是0.40 * ATR(20)。
4.止盈是0.40 * ATR(20)。

出场：
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
//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyATR = %lf",
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
//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
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
MACD BEILI: 主要是在超买超卖的时候，左侧交易，损益比至少1
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

AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI_Daily_Regression(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;

	int    shiftPreDayBar = shift1Index - 1;
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int orderIndex;	
	int index1, index2, index3;
	double level = 0, histLevel = 0, maxLevel = 0;
	double preClose1, preClose2, preClose3, preClose4, preClose5;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5,fastMin,fastMax;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double dailyRangHigh = 0.0, dailyRangeLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;	

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	double stopLoss = pBase_Indicators->pDailyMaxATR;	

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;
	BOOL isDailyOnly = TRUE;
	double takeProfitPrice,takeProfit;
	
	double maxRisk = 1;

	double ma5Daily;

	BOOL isEnableFlatBars = FALSE;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// 满足日图历史的趋势分析。

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 28;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 14.0; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 1.0; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;		

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;
		
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		//isEnableFlatBars = TRUE;

	}	
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.7; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		//isEnableFlatBars = TRUE;

	}
	else
	{
		level = 0; //EURUSD		

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}
		

	//The last 5 days High and Low
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 5, &dailyRangHigh, &dailyRangeLow);		

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh=%lf,shortDailyLow=%lf,daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline);

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);

	//Use the min 
	pIndicators->fast = fast1;
	pIndicators->slow = slow1;

	fastMin = 100.0;
	fastMin = min(fastMin, fast1);
	fastMin = min(fastMin, fast2);
	fastMin = min(fastMin, fast3);
	fastMin = min(fastMin, fast4);
	fastMin = min(fastMin, fast5);

	fastMax = -100.0;
	fastMax = max(fastMax, fast1);
	fastMax = max(fastMax, fast2);
	fastMax = max(fastMax, fast3);
	fastMax = max(fastMax, fast4);
	fastMax = max(fastMax, fast5);

	//load pBase_Indicators
	pBase_Indicators->dailyATR = iAtr(B_PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), startShift);
	pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;
	iTrend3Rules(pParams, pIndicators, B_DAILY_RATES, 2, &(pBase_Indicators->daily3RulesTrend),0);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf,fastMax=%lf,fastMin=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow, fastMax,fastMin);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (
		pIndicators->fast - pIndicators->slow > macdLimit // cross up
		) // Buy
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
		{
			stopLoss = pParams->orderInfo[orderIndex].stopLoss;
		}
		else{
			stopLoss = fabs(pIndicators->entryPrice - dailyRangeLow) + 0.2 * pBase_Indicators->dailyATR;
		}

		pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
			
		pIndicators->takeProfitPrice = daily_baseline - 0.2 * pBase_Indicators->dailyATR;
		takeProfit = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);

		//takeProfit = min(takeProfit, 2 * stopLoss);
		//pIndicators->takeProfitPrice = pIndicators->entryPrice + takeProfit;

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

		if (
			fastMin <= (-1 * level)
			//&& pIndicators->fast <= (-1 * level) /3
			&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))				
			&& pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) <= 0.2 * pBase_Indicators->dailyATR
			&& takeProfit > stopLoss // > 1				
			&&
			(
			pBase_Indicators->daily3RulesTrend == UP 
			//|| iClose(B_DAILY_RATES, startShift) > daily_baseline_short
				||
				stopLoss > 1.5 * pBase_Indicators->dailyATR
				)
			)
		{

			//if (fastMin < (-1.5 * level))
			//	maxRisk = 1.5;
			//else if (fastMin < (-2 * level))
			//	maxRisk = 2;
			

			safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

			if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
			{
				pIndicators->entrySignal = 1;		

				pIndicators->risk = maxRisk;
			}

		}			

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (
			pIndicators->slow - pIndicators->fast > macdLimit					
		) // Sell
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
		{
			stopLoss = pParams->orderInfo[orderIndex].stopLoss;
		}
		else{
			stopLoss = fabs(pIndicators->entryPrice - dailyRangHigh) + 0.2 * pBase_Indicators->dailyATR;
		}

		pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

		pIndicators->takeProfitPrice = daily_baseline + 0.2 * pBase_Indicators->dailyATR;
		takeProfit = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);
			
		//takeProfit = min(takeProfit, 2 * stopLoss);
		//pIndicators->takeProfitPrice = pIndicators->entryPrice - takeProfit;

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

		if (
			fastMax >= level
			//&& pIndicators->fast >= level / 3
			&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))				
			&& iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice <= 0.2 * pBase_Indicators->dailyATR
			&& takeProfit > stopLoss // > 1						
			&& (
				pBase_Indicators->daily3RulesTrend == DOWN 
				//|| iClose(B_DAILY_RATES, startShift) < daily_baseline_short
				|| 
				stopLoss > 1.5 * pBase_Indicators->dailyATR
				)
			)
		{

			//if (fastMax > 1.5* level)
			//	maxRisk = 1.5;
			//else if (fastMax > 2 * level)
			//	maxRisk = 2.0;

			safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

			if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
			{
				pIndicators->entrySignal = -1;
				pIndicators->risk = maxRisk;
			}
				
		}

		pIndicators->exitSignal = EXIT_BUY;

	}

	ma5Daily = iMA(3, B_DAILY_RATES, 4, startShift+1);

	if (isEnableFlatBars == TRUE
		&& pIndicators->entrySignal != 0			
		&& fabs(iClose(B_DAILY_RATES, startShift + 1) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
		&& fabs(iClose(B_DAILY_RATES, startShift + 2) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
		&& fabs(iClose(B_DAILY_RATES, startShift + 3) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
		&& fabs(iClose(B_DAILY_RATES, startShift + 4) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
		)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, skip entry signal due to flat bars",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

		pIndicators->entrySignal = 0;
	}

	

	return SUCCESS;
}


AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;

	int    shiftPreDayBar = shift1Index - 1;
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int orderIndex;
	int index1, index2, index3;
	double level = 0, histLevel = 0, maxLevel = 0;
	double preClose1, preClose2, preClose3, preClose4, preClose5;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5, fastMin, fastMax;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double dailyRangHigh = 0.0, dailyRangeLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	double stopLoss = pBase_Indicators->pDailyMaxATR;

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;
	BOOL isDailyOnly = TRUE;
	double takeProfitPrice, takeProfit;

	double maxRisk = 1;

	double ma5Daily;

	BOOL isEnableFlatBars = FALSE;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// 满足日图历史的趋势分析。

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 28;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 0.9; //precetage				

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.7; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		//isEnableFlatBars = TRUE;

	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.7; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		//isEnableFlatBars = TRUE;

	}
	else
	{
		level = 0; //EURUSD		

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}

	//如果是23H，可以检查是否可以入场
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;
		macdLimit = 0;

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 交易， 避开开盘的一个小时,同时只是新的一天的第一个小时
	{

		//The last 5 days High and Low
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 5, &dailyRangHigh, &dailyRangeLow);

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh=%lf,shortDailyLow=%lf,daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline);

		//Load MACD
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);

		//Use the min 
		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;

		fastMin = 100.0;
		fastMin = min(fastMin, fast1);
		fastMin = min(fastMin, fast2);
		fastMin = min(fastMin, fast3);
		fastMin = min(fastMin, fast4);
		fastMin = min(fastMin, fast5);

		fastMax = -100.0;
		fastMax = max(fastMax, fast1);
		fastMax = max(fastMax, fast2);
		fastMax = max(fastMax, fast3);
		fastMax = max(fastMax, fast4);
		fastMax = max(fastMax, fast5);


		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf,fastMax=%lf,fastMin=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow, fastMax, fastMin);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		if (
			pIndicators->fast - pIndicators->slow > macdLimit // cross up
			//pIndicators->fast < 0
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{				
				//if (fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) > 1.5 *pIndicators->adjust &&
				//	pIndicators->entryPrice - pParams->orderInfo[orderIndex].openPrice > fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice)){
				//	//stopLoss = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice);
				//	//stopLoss = fabs(iLow(B_DAILY_RATES, 1) - pIndicators->entryPrice);	
				//	stopLoss = pIndicators->entryPrice - pParams->orderInfo[orderIndex].openPrice;
				//}
				//else
				//	stopLoss = 9999999;
				stopLoss = pIndicators->entryPrice - pBase_Indicators->dailyS;
			}
			else{
				stopLoss = fabs(pIndicators->entryPrice - dailyRangeLow) + 0.2 * pBase_Indicators->dailyATR;
			}

			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

			pIndicators->takeProfitPrice = daily_baseline - 0.2 * pBase_Indicators->dailyATR;
			takeProfit = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);

			//takeProfit = min(takeProfit, 2 * stopLoss);
			//pIndicators->takeProfitPrice = pIndicators->entryPrice + takeProfit;

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (
				fastMin <= (-1 * level) * pIndicators->entryPrice / 100				
				//&& pIndicators->preFast <  pIndicators->fast
				//&& pIndicators->fast <= (-1 * level) /3
				&& iClose(B_DAILY_RATES, 1) > iClose(B_DAILY_RATES, 2) 
				&& iClose(B_DAILY_RATES, 1) - iOpen(B_DAILY_RATES, 1) >= 0.5*(iHigh(B_DAILY_RATES, 1) - iLow(B_DAILY_RATES,1))
				&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) <= 0.2 * pBase_Indicators->dailyATR
				&& takeProfit > 0.8* stopLoss // > 1				
				)
			{

				//if (fastMin < (-1.5 * level))
				//	maxRisk = 1.5;
				//else if (fastMin < (-2 * level))
				//	maxRisk = 2;


				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = 1;

					pIndicators->risk = maxRisk;
				}

			}
			
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (
			pIndicators->slow - pIndicators->fast > macdLimit
			//pIndicators->fast > 0
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{	
				//if (fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) > 1.5 *pIndicators->adjust &&
				//	pParams->orderInfo[orderIndex].openPrice - pIndicators->entryPrice > fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice)){
				//	//stopLoss = fabs(iHigh(B_DAILY_RATES, 1) - pIndicators->entryPrice);					
				//	stopLoss = pParams->orderInfo[orderIndex].openPrice - pIndicators->entryPrice;
				//}
				//else
				//	stopLoss = 9999999;
				stopLoss = pBase_Indicators->dailyS - pIndicators->entryPrice;
			}
			else{
				stopLoss = fabs(pIndicators->entryPrice - dailyRangHigh) + 0.2 * pBase_Indicators->dailyATR;
			}

			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

			pIndicators->takeProfitPrice = daily_baseline + 0.2 * pBase_Indicators->dailyATR;
			takeProfit = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);

			//takeProfit = min(takeProfit, 2 * stopLoss);
			//pIndicators->takeProfitPrice = pIndicators->entryPrice - takeProfit;

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (
				fastMax >= level * pIndicators->entryPrice / 100
				//&& fastMin > 0.1 * pIndicators->entryPrice / 100
				//&& pIndicators->preFast >  pIndicators->fast
				//&& pIndicators->fast >= level / 3
				&& iClose(B_DAILY_RATES, 1) < iClose(B_DAILY_RATES, 2)
				&& iOpen(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 1) >= 0.5*(iHigh(B_DAILY_RATES, 1) - iLow(B_DAILY_RATES, 1))
				&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice <= 0.2 * pBase_Indicators->dailyATR
				&& takeProfit > 0.8*stopLoss // > 1						
				)
			{

				//if (fastMax > 1.5* level)
				//	maxRisk = 1.5;
				//else if (fastMax > 2 * level)
				//	maxRisk = 2.0;

				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = -1;
					pIndicators->risk = maxRisk;
				}

			}

			
			pIndicators->exitSignal = EXIT_BUY;

		}

		ma5Daily = iMA(3, B_DAILY_RATES, 4, startShift + 1);

		if (isEnableFlatBars == TRUE
			&& pIndicators->entrySignal != 0
			&& fabs(iClose(B_DAILY_RATES, startShift + 1) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			&& fabs(iClose(B_DAILY_RATES, startShift + 2) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			&& fabs(iClose(B_DAILY_RATES, startShift + 3) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			&& fabs(iClose(B_DAILY_RATES, startShift + 4) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			)
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, skip entry signal due to flat bars",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			pIndicators->entrySignal = 0;
		}

	}

	return SUCCESS;
}

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

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);
	

	//计算日内高低点
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
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
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


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);
		
	asia_ATR = fabs(pIndicators->asia_high - pIndicators->asia_low);

	hasCurrentDayOrder = hasSameDayOrderEasy(currentTime, &isOpen);
	if(hasCurrentDayOrder == TRUE && isOpen == FALSE)
		return SUCCESS;


	if (hasCurrentDayOrder == FALSE)
	{

		////加入5M均线的支持
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

		//如果已经有了一个trade，那就是第二次交易，要使用日内的高低点
		//如果有了2个trades,那就是第三次交易了，使用orderHigh, orderLow.
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
			//这策略不允许同时开出多个open order,所以这里index,应该都是0			 
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
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
						return SUCCESS;
					}
				}

				if (side == BUY)
				{
					//如果过了20点后，出现第一个5M的阴线，离场
					if (openOrderHigh - entryPrice > pIndicators->takePrice &&
						pParams->bidAsk.bid[0] - entryPrice < pIndicators->takePrice
						)
					{
						//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
						closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
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

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);


	//计算日内高低点
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
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
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


	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
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

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
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

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
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


	// filter 非农
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	{

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Filter Non-farm day",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

		return FALSE;
	}

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
		readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
	else
	{
		pBase_Indicators->pWeeklyPredictATR = 20;
		pBase_Indicators->pWeeklyPredictMaxATR = 25;
	}
	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
		return FALSE;

	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 10)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
		return FALSE;
	}

	return TRUE;
}

