/*
 * BBS Order Splitting Module
 * 
 * Provides order splitting functions for BBS Swing strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"

void splitBuyOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;	
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;
	
	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	//double pWeeklyATR = pBase_Indicators->pWeeklyATR;
	//double pWeeklyHigh = pBase_Indicators->pWeeklyHigh;
	//double pWeeklyLow = pBase_Indicators->pWeeklyLow;

	//double weeklyGap = pWeeklyHigh - pIndicators->entryPrice;
	
	//pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->strategyMaxRisk

	//lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss)* pIndicators->risk;
	//lots_singal = lots_singal * (pIndicators->strategyMaxRisk - pIndicators->strategyRiskWithoutLockedProfit) / pIndicators->strategyMaxRisk;
	 
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice <= pBase_Indicators->dailyR1 && (int)pParams->settings[TIMEFRAME] == 15)
	{

		takePrice = dailyGap;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleLongEasy(takePrice, stopLoss, lots_singal/2, 0);

		takePrice = atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleLongEasy(takePrice, stopLoss, lots_singal / 2, 0);
	}
}

/*
һ ڳ
ڶ ڳ
*/
void splitSellOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	//double pWeeklyATR = pBase_Indicators->pWeeklyATR;
	//double pWeeklyHigh = pBase_Indicators->pWeeklyHigh;
	//double pWeeklyLow = pBase_Indicators->pWeeklyLow;

	//double weeklyGap = pIndicators->entryPrice - pWeeklyLow;
	//lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, stopLoss)* pIndicators->risk;
	//lots_singal = lots_singal * (pIndicators->strategyMaxRisk - pIndicators->strategyRiskWithoutLockedProfit) / pIndicators->strategyMaxRisk;

	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice >= pBase_Indicators->dailyS1 && (int)pParams->settings[TIMEFRAME] == 15)		
	{
		
		takePrice = dailyGap;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleShortEasy(takePrice, stopLoss, lots_singal / 2, 0);

		takePrice = atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleShortEasy(takePrice, stopLoss, lots_singal / 2, 0);
	}
}

/*
źţֹӯ100
*/
void splitBuyOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	double lots, lots_singal;

	double total_pre_lost = 0;
	int lostTimes;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	takePrice = min(takePrice, pBase_Indicators->dailyATR);
		

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	lostTimes = getLossTimesFromNowsEasy(currentTime, &total_pre_lost);

	if (lostTimes <= 1)
		lots = lots_singal;
	else
		lots = total_pre_lost / takePrice;

	//if (strstr(pParams->tradeSymbol, "JPY") != NULL)
	//{
	//	if (lostTimes <= 1)
	//		lots = lots_singal;
	//	else
	//		lots = total_pre_lost / takePrice;
	//}
	//else
	//{
	//	//lots = lots_singal * pow(2, lostTimes);

	//	if (lostTimes <= 0)
	//		lots = lots_singal;
	//	else if (lostTimes == 1)
	//		//lots = total_pre_lost / takePrice + lots_singal;			
	//		lots = lots_singal * 2.5;
	//	else	
	//		lots = total_pre_lost / takePrice;
	//}

	openSingleLongEasy(takePrice, stopLoss, lots, 0);

	
}

/*
źţֹӯ100
*/
void splitSellOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	double lots, lots_singal;

	double total_pre_lost = 0;
	int lostTimes;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	takePrice = min(takePrice, pBase_Indicators->dailyATR);

	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

	lostTimes = getLossTimesFromNowsEasy(currentTime, &total_pre_lost);

	if (lostTimes <= 1)
		lots = lots_singal;
	else
		lots = total_pre_lost / takePrice;

	//if (strstr(pParams->tradeSymbol, "JPY") != NULL)
	//{
	//	if (lostTimes <= 1)
	//		lots = lots_singal;
	//	else
	//		lots = total_pre_lost / takePrice;
	//}
	//else
	//{
	//	//lots = lots_singal * pow(2, lostTimes);

	//	if (lostTimes <= 0)
	//		lots = lots_singal;
	//	else if (lostTimes == 1)
	//		lots = lots_singal * 2.5;
	//	else 
	//		lots = total_pre_lost / takePrice;
	//}


	openSingleShortEasy(takePrice, stopLoss, lots, 0);
	
}


/*
źţֹӯһܲ,ղ
*/
void splitBuyOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	double lots_singal;

	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pIndicators->isEnableBuyMinLotSize == TRUE)
		lots_singal = pIndicators->minLotSize;
	else{
		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, max(stopLoss, pBase_Indicators->dailyATR*1.5)) * pIndicators->risk;

	}

	lots_singal = roundUp(lots_singal, pIndicators->minLotSize);

	//takePrice = pIndicators->takePrice;
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
		
	openSingleLongEasy(takePrice, stopLoss, lots_singal, 0);
	

}

/*
źţֹӯһܲ,ղ
*/
void splitSellOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	double lots_singal;

	double total_pre_lost = 0;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	if (pIndicators->isEnableSellMinLotSize == TRUE)
		lots_singal = pIndicators->minLotSize;
	else
		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, max(stopLoss, pBase_Indicators->dailyATR*1.5)) * pIndicators->risk;
	
	//takePrice = pIndicators->takePrice;
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
	lots_singal = roundUp(lots_singal, pIndicators->minLotSize);

	openSingleShortEasy(takePrice, stopLoss, lots_singal, 0);
}

