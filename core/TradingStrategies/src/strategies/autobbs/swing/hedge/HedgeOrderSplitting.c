/*
 * Hedge Order Splitting Module
 * 
 * Provides order splitting functions for hedge strategies.
 * Used by splitTradeMode 14.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/swing/hedge/HedgeOrderSplitting.h"

/**
 * Split buy orders for short-term ATR hedge strategy.
 * Used by splitTradeMode 14 (ATR variant).
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing daily ATR and price ranges
 * @param takePrice_primary Primary take profit target (unused, calculated internally)
 * @param stopLoss Stop loss distance
 */
void splitBuyOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	/* Range reversal: Enter buy when price is within 1/3 ATR of lower range */
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * Split sell orders for short-term ATR hedge strategy.
 * Used by splitTradeMode 14 (ATR variant).
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing daily ATR and price ranges
 * @param takePrice_primary Primary take profit target (unused, calculated internally)
 * @param stopLoss Stop loss distance
 */
void splitSellOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	/* Range reversal: Enter sell when price is within 1/3 ATR of upper range */
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * Split buy orders for short-term hedge strategy.
 * Used by splitTradeMode 14.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing daily ATR and price ranges
 * @param takePrice_primary Primary take profit target (unused, calculated internally)
 * @param stopLoss Stop loss distance
 */
void splitBuyOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	/* Range reversal: Enter buy when price is within 1/3 ATR of lower range */
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		/* Double risk if up gap is very small (within 1/4 ATR) */
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * Split sell orders for short-term hedge strategy.
 * Used by splitTradeMode 14.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing daily ATR and price ranges
 * @param takePrice_primary Primary take profit target (unused, calculated internally)
 * @param stopLoss Stop loss distance
 */
void splitSellOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	/* Range reversal: Enter sell when price is within 1/3 ATR of upper range */
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		/* Double risk if up gap is very small (within 1/4 ATR) */
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

