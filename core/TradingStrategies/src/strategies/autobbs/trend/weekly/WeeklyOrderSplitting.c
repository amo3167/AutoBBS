/*
 * Weekly Order Splitting Module
 * 
 * Provides order splitting functions for Weekly strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/weekly/WeeklyOrderSplitting.h"

void splitBuyOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitSellOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		//takePrice = 2 * takePrice_primary;
		//openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitBuyOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR2)
	{
		takePrice = takePrice_primary;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		//takePrice = 2 * takePrice_primary;
		//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
			takePrice = 3 * takePrice_primary;
		else
			takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}

void splitSellOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS2)
	{
		takePrice = takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

		takePrice = 2 * takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
			takePrice = 3 * takePrice_primary;
		else
			takePrice = 0;

		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);
	}
}

void splitBuyOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = fabs(pBase_Indicators->weeklyR2 - pIndicators->adjust - pIndicators->entryPrice);
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitSellOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) or S2?
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->weeklyS2 + pIndicators->adjust));
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
