/*
 * Order Splitting Module
 * 
 * Provides functions for splitting single trade signals into multiple orders.
 * Handles various order splitting strategies based on trade mode and market conditions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"  // For splitBuyOrders_ATR, splitBuyOrders_LongTerm
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"  // For splitBuyOrders_Ichimoko_Weekly, splitSellOrders_Ichimoko_Weekly
#include "strategies/autobbs/trend/weekly/WeeklyOrderSplitting.h"  // For splitBuyOrders_Weekly_ShortTerm, splitSellOrders_Weekly_ShortTerm, splitBuyOrders_WeeklyTrading, splitSellOrders_WeeklyTrading, splitBuyOrders_Weekly_Beginning, splitSellOrders_Weekly_Beginning
#include "strategies/autobbs/trend/misc/KeyKOrderSplitting.h"  // For splitBuyOrders_KeyK, splitSellOrders_KeyK
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"  // For splitBuyOrders_4HSwing, splitSellOrders_4HSwing, splitBuyOrders_4HSwing_100P, splitSellOrders_4HSwing_100P
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"  // For splitBuyOrders_MACDDaily, splitSellOrders_MACDDaily, splitBuyOrders_MACDWeekly, splitSellOrders_MACDWeekly
#include "strategies/autobbs/trend/shortterm/ShortTermOrderSplitting.h"  // For splitBuyOrders_ShortTerm_New, splitSellOrders_ShortTerm_New, splitBuyOrders_ShortTerm, splitSellOrders_ShortTerm
#include "strategies/autobbs/swing/daytrading/DayTradingOrderSplitting.h"  // For splitBuyOrders_Daily_Swing_ExecutionOnly, splitSellOrders_Daily_Swing_ExecutionOnly, splitSellOrders_Daily_Swing
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"


static void splitBuyOrders_Limit(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);

	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE)
	{
		// Fin 38.2%
		openPrice = preLow + (preHigh - preLow) * 0.382 + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		// Fin 50%
		openPrice = preLow + (preHigh - preLow) * 0.5 + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{
		// Pivot
		openPrice = pBase_Indicators->dailyPivot + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}
}

static void splitSellOrders_Limit(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);

	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
	{
		// Fin 38.2%
		openPrice = preLow + (preHigh - preLow) * 0.382 - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		// Fin 50%
		openPrice = preLow + (preHigh - preLow) * 0.5 - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
		// Pivot
		openPrice = pBase_Indicators->dailyPivot - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}
}

// splitBuyOrders_LongTerm is implemented in OrderSplittingUtilities.c - removed duplicate
// splitBuyOrders_Weekly_Beginning is implemented in WeeklyOrderSplitting.c - removed duplicate
// splitSellOrders_LongTerm is implemented in OrderSplittingUtilities.c - removed duplicate
// splitSellOrders_Weekly_Beginning is implemented in WeeklyOrderSplitting.c - removed duplicate

// Short & Long term trades
// split into 3 trades
// 50% 1:1
// 25% 2:1
// 25% no tp.
static void splitBuyOrders_MediumTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
	{
		takePrice = takePrice_primary;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
			takePrice = 2 * takePrice_primary;
		else
		{
			takePrice = fabs(pBase_Indicators->dailyR2 - pIndicators->adjust - pIndicators->entryPrice);
			if (takePrice > 2 * takePrice_primary)
				takePrice = 2 * takePrice_primary;
			if (takePrice < takePrice_primary)
				takePrice = takePrice_primary;
		}

		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
		{
			// Calculate take profit for third order (3:1 ratio)
			if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
				takePrice = 3 * takePrice_primary;
			else
			{
				takePrice = fabs(pBase_Indicators->dailyR3 - pIndicators->adjust - pIndicators->entryPrice);
				if (takePrice > 3 * takePrice_primary)
					takePrice = 3 * takePrice_primary;
			}

			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
		else
		{
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
	}
}

// Short & Long term trades
// split into 3 trades
// 50% 1:1
// 25% 2:1
// 25% no tp or 3:1
static void splitSellOrders_MediumTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
	{
		// 1:1 ratio
		takePrice = takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
		{
			takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS2 + pIndicators->adjust));
			if (takePrice > 2 * takePrice_primary)
				takePrice = 2 * takePrice_primary;
			if (takePrice < takePrice_primary)
				takePrice = takePrice_primary;
		}
		else
			takePrice = 2 * takePrice_primary;

		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
		{
			if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
			{
				takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS3 + pIndicators->adjust));
				if (takePrice > 3 * takePrice_primary)
					takePrice = 3 * takePrice_primary;
			}
			else
				takePrice = 3 * takePrice_primary;

			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
		else
		{
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
	}
}

void splitBuyOrders_Daily_Swing(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal;
	double gap = pHigh - pIndicators->entryPrice;

	takePrice = 3;

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice);

	lots = pIndicators->total_lose_pips / takePrice + lots_singal;

	// Cap lots to 1.5% of equity
	if (lots * takePrice * 100 / pParams->accountInfo.equity > 0.015)
		lots = pParams->accountInfo.equity * 0.015 / 100 / takePrice;

	openSingleLongEasy(takePrice, stopLoss, lots, 0);
}

// splitSellOrders_Daily_Swing is implemented in DayTradingOrderSplitting.c - removed duplicate

// splitBuyOrders_ATR is implemented in OrderSplittingUtilities.c - removed duplicate
// splitBuyOrders_ShortTerm is implemented in ShortTermOrderSplitting.c - removed duplicate

static void splitBuyOrders_ShortTerm_ATR_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// Hedge order when price is near the low (within 1/3 ATR)
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitSellOrders_ShortTerm_ATR_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// Hedge order when price is near the high (within 1/3 ATR)
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitBuyOrders_ShortTerm_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// Hedge order when price is near the low (within 1/3 ATR)
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitSellOrders_ShortTerm_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// Hedge order when price is near the high (within 1/3 ATR)
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

// splitBuyOrders_KeyK is implemented in KeyKOrderSplitting.c - removed duplicate
// splitSellOrders_ATR is implemented in OrderSplittingUtilities.c - removed duplicate
// splitSellOrders_ShortTerm is implemented in ShortTermOrderSplitting.c - removed duplicate
// splitSellOrders_KeyK is implemented in KeyKOrderSplitting.c - removed duplicate

// Stub implementations for missing order splitting functions
static void splitBuyOrders_MultiDays_GBPJPY_Swing(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	// TODO: Implement MultiDays GBPJPY Swing order splitting
	// For now, use Daily_Swing as fallback
	splitBuyOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
}

static void splitSellOrders_MultiDays_GBPJPY_Swing(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	// TODO: Implement MultiDays GBPJPY Swing order splitting
	// For now, use Daily_Swing as fallback
	splitSellOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
}

static void splitBuyOrders_MACD_BEILI(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	// TODO: Implement MACD BEILI order splitting
	// For now, use ShortTerm as fallback
	splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
}

static void splitSellOrders_MACD_BEILI(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	// TODO: Implement MACD BEILI order splitting
	// For now, use ShortTerm as fallback
	splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
}

void splitBuyOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode) {
		case 0:
			splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;		
		case 3:
			splitBuyOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 4:
			splitBuyOrders_Limit(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 5:
			break;
		case 6:			
			break;
		case 7:			
			break;		
		case 8:
			splitBuyOrders_WeeklyTrading(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 9:
			splitBuyOrders_KeyK(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 10:
			splitBuyOrders_Weekly_Beginning(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 11:
			splitBuyOrders_Weekly_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 12:
			splitBuyOrders_ATR(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 13:
			break;
		case 14:
			splitBuyOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 15:
			splitBuyOrders_ShortTerm_ATR_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 16:
			splitBuyOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 17:
			break;	
		case 18:
			break;
		case 19:
			splitBuyOrders_4HSwing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 20:
			splitBuyOrders_4HSwing_100P(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 21:
			break;
		case 22:
			break;			
		case 23:
			splitBuyOrders_MultiDays_GBPJPY_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 24:
			splitBuyOrders_MACDDaily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 25:
			splitBuyOrders_MACDWeekly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 26:
			splitBuyOrders_Ichimoko_Daily(pParams, pIndicators, pBase_Indicators, pBase_Indicators->pDailyATR, stopLoss);
			break;
		case 27:
			splitBuyOrders_4HSwing_Shellington(pParams, pIndicators, pBase_Indicators, pBase_Indicators->pDailyATR, stopLoss);
			break;
		case 28:
			splitBuyOrders_MACD_BEILI(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 29:
			splitBuyOrders_Daily_Swing_ExecutionOnly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 30:
			break;
		case 31:
			splitBuyOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 32:
			splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
			break;
		case 33:
			splitBuyOrders_Ichimoko_Weekly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;

	}
	
}

void splitSellOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode) {
	case 0:
		splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 3:
		splitSellOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
		case 4:
			splitSellOrders_Limit(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 5:
		break;
	case 8:
		splitSellOrders_WeeklyTrading(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 9:
		splitSellOrders_KeyK(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 10:
		splitSellOrders_Weekly_Beginning(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 11:
		splitSellOrders_Weekly_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 12:
		splitSellOrders_ATR(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 13:
		break;
	case 14:
		splitSellOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 15:
		splitSellOrders_ShortTerm_ATR_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 16:
		splitSellOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 17:
		break;	
	case 18:
		break;
	case 19:
		splitSellOrders_4HSwing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 20:
		splitSellOrders_4HSwing_100P(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 21:
		break;
	case 22:
		break;
	case 23:
		splitSellOrders_MultiDays_GBPJPY_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 24:
		splitSellOrders_MACDDaily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 25:
		splitSellOrders_MACDWeekly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 26:
		splitSellOrders_Ichimoko_Daily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 27:
		splitSellOrders_4HSwing_Shellington(pParams, pIndicators, pBase_Indicators, pBase_Indicators->pDailyATR, stopLoss);
		break;
	case 28:
		splitSellOrders_MACD_BEILI(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 29:
		splitSellOrders_Daily_Swing_ExecutionOnly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 30:
		break;
	case 31:
		splitSellOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 32:
		splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		break;
	}
}

// handleTradeEntries moved to shared/ordermanagement/OrderManagement.c

// handleTradeExits moved to shared/ordermanagement/OrderManagement.c
// workoutExecutionTrend moved to shared/execution/StrategyExecution.c (corrupted duplicate removed)
