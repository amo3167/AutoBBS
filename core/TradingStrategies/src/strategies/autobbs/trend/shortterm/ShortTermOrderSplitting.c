/*
 * ShortTerm Order Splitting Module
 * 
 * Provides order splitting functions for ShortTerm strategies.
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
#include "strategies/autobbs/trend/shortterm/ShortTermOrderSplitting.h"

void splitBuyOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double lots;
	double miniLots = 0.01;
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

//	openSingleLongEasy(takePrice_primary, stopLoss, lots - miniLots, pIndicators->risk);
	//takePrice_primary = 0;
	openSingleLongEasy(pIndicators->takePrice, stopLoss, lots, pIndicators->risk);
}

void splitSellOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{	
	double lots;
	double miniLots = 0.01;
	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

	//openSingleShortEasy(takePrice_primary, stopLoss, lots - miniLots, pIndicators->risk);
	//takePrice_primary = 0;
	openSingleShortEasy(pIndicators->takePrice, stopLoss, lots, pIndicators->risk);
}

void splitBuyOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pHigh - pIndicators->entryPrice;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	double riskcap = parameter(AUTOBBS_RISK_CAP);
	double lots_max;

	int orderCountToday = 0;

	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// Take 1/3 of the gap as take profit
	
	if (pIndicators->tradeMode == 1)
	{
		if (gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{
				takePrice = gap /3;
				
				lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap to 2 % risk
				// If using calculated stop loss 50 points, then cap the max risk to 2%
				//Cap the max risk
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, BUY, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * 7);
				lots = min(lots_max, lots);

				//if (takePrice >= 1.5) //XAUUSD		
					openSingleLongEasy(takePrice, stopLoss, lots, 0);
				


				if (pIndicators->entrySignal == 1 
					//&& pIndicators->bbsIndex_excution != shift0Index_Primary - 1
					)
				{
					if (pIndicators->subTradeMode == 0)
						logWarning("System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						logWarning("System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				}
			}
		}
	}
	else
	{		
		if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
		{		
			takePrice = stopLoss;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/2);
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/2);		
		}
	}


	
}

void splitSellOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pIndicators->entryPrice - pLow;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	time_t currentTime;
	struct tm timeInfo1;

	double riskcap = parameter(AUTOBBS_RISK_CAP);
	double lots_max;

	int orderCountToday = 0;

	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Take 1/3 of the gap as take profit
	if (pIndicators->tradeMode == 1)
	{
		if (gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{

				takePrice = gap /3;
				lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap to 2 % risk
				// If using calculated stop loss 50 points, then cap the max risk to 2%
				//Cap the max risk
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, SELL, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * 7);
				lots = min(lots_max, lots);

				//if (takePrice >= 1.5) //XAUUSD
					openSingleShortEasy(takePrice, stopLoss, lots, 0);



				if (pIndicators->entrySignal == -1 
					//&& pIndicators->bbsIndex_excution != shift0Index_Primary - 1
					)
				{
					if (pIndicators->subTradeMode == 0)
						logWarning("System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						logWarning("System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
				}
			}
		}
	}
	else
	{
		if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
		{
			takePrice = stopLoss;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		}
	}

}
