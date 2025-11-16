/*
 * Ichimoko Order Splitting Module
 * 
 * Provides order splitting functions for Ichimoko Daily and Weekly strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"

void splitBuyOrders_Ichimoko_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss);
	lots = roundUp(lots, pIndicators->volumeStep) * pIndicators->risk;

	//Long term
	if (pIndicators->tradeMode == 1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots / 3 * 2, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleLongEasy(takePrice, stopLoss, lots / 3, 0);
	}
}

void splitBuyOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss);
	lots = roundUp(lots, pIndicators->volumeStep) * pIndicators->risk;

	//Long term
	if (pIndicators->tradeMode == 1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots / 3 * 2, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleLongEasy(takePrice, stopLoss, lots / 3, 0);
	}
}

void splitSellOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, stopLoss) * pIndicators->risk;

	if (pIndicators->tradeMode == 1)
	{
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, lots / 3 * 2, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleShortEasy(takePrice, stopLoss, lots / 3, 0);
	}
}

