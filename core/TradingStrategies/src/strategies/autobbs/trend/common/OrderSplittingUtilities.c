/*
 * Order Splitting Utilities Module
 * 
 * Provides shared utility order splitting functions used by multiple strategies.
 * These functions are called from AutoBBS.c and various strategy modules.
 */

#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"

/**
 * Split buy orders using ATR-based take profit levels.
 * Used by splitTradeMode 12.
 */
void splitBuyOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if ((pIndicators->entryPrice <= pBase_Indicators->dailyR1 && (int)pParams->settings[TIMEFRAME] == 15) ||
		(pIndicators->entryPrice <= pBase_Indicators->dailyR2 && (int)pParams->settings[TIMEFRAME] == 5) ||
		(pIndicators->entryPrice <= pBase_Indicators->dailyR2 && (int)pParams->settings[TIMEFRAME] == 30))
	{
		takePrice = 0.8* atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}

/**
 * Split sell orders using ATR-based take profit levels.
 * Used by splitTradeMode 12.
 */
void splitSellOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if ((pIndicators->entryPrice >= pBase_Indicators->dailyS1 && (int)pParams->settings[TIMEFRAME] == 15) ||
		(pIndicators->entryPrice >= pBase_Indicators->dailyS2 && (int)pParams->settings[TIMEFRAME] == 5) ||
		(pIndicators->entryPrice >= pBase_Indicators->dailyS2 && (int)pParams->settings[TIMEFRAME] == 30))
	{
		takePrice = 0.8* atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}

/**
 * Split buy orders for long-term trading.
 * Used by splitTradeMode 3.
 */
void splitBuyOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double gap = pHigh - pIndicators->entryPrice;

	// Take 1/3 of the gap as take profit
	if (gap >= pATR / 2)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/**
 * Split sell orders for long-term trading.
 * Used by splitTradeMode 3.
 */
void splitSellOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double gap = pIndicators->entryPrice - pLow;

	// Take 1/3 of the gap as take profit
	if (gap >= pATR / 2)
	{		
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/**
 * Split buy range orders.
 * Used by RangeOrderManagement.c and LimitStrategy.c.
 */
void splitBuyRangeOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	double lots;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;
	openSingleLongEasy(pIndicators->takePrice, pIndicators->stopLoss, lots, pIndicators->risk);
}

/**
 * Split sell range orders.
 * Used by RangeOrderManagement.c and LimitStrategy.c.
 */
void splitSellRangeOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double lots;

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;
	openSingleShortEasy(pIndicators->takePrice, pIndicators->stopLoss, lots, pIndicators->risk);
}

