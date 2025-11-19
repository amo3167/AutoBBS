/*
 * Order Splitting Module
 * 
 * Provides functions for splitting single trade signals into multiple orders.
 * Handles various order splitting strategies based on trade mode and market conditions.
 * 
 * This module serves as the central dispatcher for order splitting strategies.
 * It routes order splitting requests to specific implementations based on
 * the splitTradeMode parameter, which is set by individual strategy execution functions.
 * 
 * Order splitting strategies include:
 * - Limit orders (Fibonacci retracement levels)
 * - Medium term (3 orders with different risk/reward ratios)
 * - Daily swing (with loss recovery)
 * - Various strategy-specific splitters (BBS, MACD, Ichimoku, etc.)
 */

#include <math.h>
#include "strategies/autobbs/base/Base.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"
#include "strategies/autobbs/trend/weekly/WeeklyOrderSplitting.h"
#include "strategies/autobbs/trend/misc/KeyKOrderSplitting.h"
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"
#include "strategies/autobbs/trend/shortterm/ShortTermOrderSplitting.h"
#include "strategies/autobbs/swing/daytrading/DayTradingOrderSplitting.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayOrderManagement.h"
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"

// Fibonacci retracement levels
#define FIBONACCI_38_2_PERCENT 0.382
#define FIBONACCI_50_PERCENT 0.5

// Risk distribution constants
#define RISK_SPLIT_HALF 2.0              // Split risk in half (50%)
#define RISK_SPLIT_QUARTER 4.0           // Split risk into quarter (25%)

// Take profit ratio constants
#define TP_RATIO_1_TO_1 1                // 1:1 risk/reward ratio
#define TP_RATIO_2_TO_1 2                // 2:1 risk/reward ratio
#define TP_RATIO_3_TO_1 3                // 3:1 risk/reward ratio

// Daily swing constants
#define DAILY_SWING_TAKE_PRICE 3.0       // Fixed take price for daily swing (3 pips)
#define MAX_EQUITY_RISK_PERCENT 0.015     // Maximum 1.5% of equity risk
#define PERCENT_TO_DECIMAL 100.0          // Conversion factor (100% = 1.0)

/**
 * Splits buy orders for limit order strategy.
 * 
 * Places buy limit orders at Fibonacci retracement levels (38.2% and 50%)
 * or at daily pivot, depending on the trend phase.
 * 
 * For BEGINNING_UP_PHASE:
 * - Order 1: 38.2% Fibonacci level with 50% risk
 * - Order 2: 50% Fibonacci level with 100% risk
 * 
 * For MIDDLE_UP_PHASE:
 * - Order: Daily pivot level with 100% risk
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing pivot and trend phase
 * @param takePrice_primary Primary take profit value
 * @param stopLoss Stop loss value
 */
static void splitBuyOrders_Limit(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE)
	{
		// Fibonacci 38.2% retracement level
		openPrice = preLow + (preHigh - preLow) * FIBONACCI_38_2_PERCENT + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		// Fibonacci 50% retracement level
		openPrice = preLow + (preHigh - preLow) * FIBONACCI_50_PERCENT + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1.0);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{
		// Daily pivot level
		openPrice = pBase_Indicators->dailyPivot + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1.0);
	}
}

/**
 * Splits sell orders for limit order strategy.
 * 
 * Places sell limit orders at Fibonacci retracement levels (38.2% and 50%)
 * or at daily pivot, depending on the trend phase.
 * 
 * For BEGINNING_DOWN_PHASE:
 * - Order 1: 38.2% Fibonacci level with 50% risk
 * - Order 2: 50% Fibonacci level with 100% risk
 * 
 * For MIDDLE_DOWN_PHASE:
 * - Order: Daily pivot level with 100% risk
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing pivot and trend phase
 * @param takePrice_primary Primary take profit value
 * @param stopLoss Stop loss value
 */
static void splitSellOrders_Limit(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
	{
		// Fibonacci 38.2% retracement level
		openPrice = preLow + (preHigh - preLow) * FIBONACCI_38_2_PERCENT - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		// Fibonacci 50% retracement level
		openPrice = preLow + (preHigh - preLow) * FIBONACCI_50_PERCENT - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1.0);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
		// Daily pivot level
		openPrice = pBase_Indicators->dailyPivot - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1.0);
	}
}

/**
 * Splits buy orders for medium-term strategy.
 * 
 * Creates 3 orders with different risk/reward ratios:
 * - Order 1: 50% risk, 1:1 ratio (TP = stopLoss)
 * - Order 2: 25% risk, 2:1 ratio (TP = 2 * stopLoss or dailyR2)
 * - Order 3: 25% risk, 3:1 ratio (TP = 3 * stopLoss or dailyR3) OR no TP
 * 
 * Only executes if entry price is below daily R1.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing daily pivot and resistance levels
 * @param takePrice_primary Primary take profit value (used as base for calculations)
 * @param stopLoss Stop loss value
 */
static void splitBuyOrders_MediumTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
	{
		// Order 1: 50% risk, 1:1 ratio
		takePrice = takePrice_primary;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_HALF);

		// Order 2: 25% risk, 2:1 ratio
		if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
		{
			takePrice = TP_RATIO_2_TO_1 * takePrice_primary;
		}
		else
		{
			// Use daily R2 if available, otherwise use 2:1 ratio
			takePrice = fabs(pBase_Indicators->dailyR2 - pIndicators->adjust - pIndicators->entryPrice);
			if (takePrice > TP_RATIO_2_TO_1 * takePrice_primary)
				takePrice = TP_RATIO_2_TO_1 * takePrice_primary;
			if (takePrice < takePrice_primary)
				takePrice = takePrice_primary;
		}
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_QUARTER);

		// Order 3: 25% risk, 3:1 ratio OR no TP
		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
		{
			// Calculate take profit for third order (3:1 ratio)
			if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
			{
				takePrice = TP_RATIO_3_TO_1 * takePrice_primary;
			}
			else
			{
				// Use daily R3 if available, otherwise use 3:1 ratio
				takePrice = fabs(pBase_Indicators->dailyR3 - pIndicators->adjust - pIndicators->entryPrice);
				if (takePrice > TP_RATIO_3_TO_1 * takePrice_primary)
					takePrice = TP_RATIO_3_TO_1 * takePrice_primary;
			}
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_QUARTER);
		}
		else
		{
			// No take profit for third order
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_QUARTER);
		}
	}
}

/**
 * Splits sell orders for medium-term strategy.
 * 
 * Creates 3 orders with different risk/reward ratios:
 * - Order 1: 50% risk, 1:1 ratio (TP = stopLoss)
 * - Order 2: 25% risk, 2:1 ratio (TP = 2 * stopLoss or dailyS2)
 * - Order 3: 25% risk, 3:1 ratio (TP = 3 * stopLoss or dailyS3) OR no TP
 * 
 * Only executes if entry price is above daily S1.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing daily pivot and support levels
 * @param takePrice_primary Primary take profit value (used as base for calculations)
 * @param stopLoss Stop loss value
 */
static void splitSellOrders_MediumTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
	{
		// Order 1: 50% risk, 1:1 ratio
		takePrice = takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_HALF);

		// Order 2: 25% risk, 2:1 ratio
		if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
		{
			// Use daily S2 if available
			takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS2 + pIndicators->adjust));
			if (takePrice > TP_RATIO_2_TO_1 * takePrice_primary)
				takePrice = TP_RATIO_2_TO_1 * takePrice_primary;
			if (takePrice < takePrice_primary)
				takePrice = takePrice_primary;
		}
		else
		{
			takePrice = TP_RATIO_2_TO_1 * takePrice_primary;
		}
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_QUARTER);

		// Order 3: 25% risk, 3:1 ratio OR no TP
		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
		{
			if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
			{
				// Use daily S3 if available
				takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS3 + pIndicators->adjust));
				if (takePrice > TP_RATIO_3_TO_1 * takePrice_primary)
					takePrice = TP_RATIO_3_TO_1 * takePrice_primary;
			}
			else
			{
				takePrice = TP_RATIO_3_TO_1 * takePrice_primary;
			}
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_QUARTER);
		}
		else
		{
			// No take profit for third order
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_QUARTER);
		}
	}
}


/**
 * Stub implementation for MACD BEILI order splitting.
 * 
 * TODO: Implement proper MACD BEILI order splitting logic.
 * For now, uses ShortTerm as fallback.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure
 * @param takePrice_primary Primary take profit value
 * @param stopLoss Stop loss value
 */
static void splitBuyOrders_MACD_BEILI(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	// TODO: Implement MACD BEILI order splitting
	// For now, use ShortTerm as fallback
	splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
}

/**
 * Stub implementation for MACD BEILI order splitting.
 * 
 * TODO: Implement proper MACD BEILI order splitting logic.
 * For now, uses ShortTerm as fallback.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure
 * @param takePrice_primary Primary take profit value
 * @param stopLoss Stop loss value
 */
static void splitSellOrders_MACD_BEILI(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	// TODO: Implement MACD BEILI order splitting
	// For now, use ShortTerm as fallback
	splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
}

/**
 * Main dispatcher for buy order splitting.
 * 
 * Routes buy order splitting requests to specific implementations based on
 * the splitTradeMode parameter set by strategy execution functions.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing splitTradeMode
 * @param pBase_Indicators Base indicators structure
 * @param takePrice_primary Primary take profit value
 * @param stopLoss Stop loss value
 * 
 * Split trade modes:
 * - 0: ShortTerm
 * - 3: LongTerm
 * - 4: Limit (Fibonacci retracement)
 * - 8: WeeklyTrading
 * - 9: KeyK
 * - 10: Weekly_Beginning
 * - 11: Weekly_ShortTerm
 * - 12: ATR
 * - 14: (Removed: ShortTerm_Hedge)
 * - 15: (Removed: ShortTerm_ATR_Hedge)
 * - 16: Daily_Swing
 * - 19: 4HSwing
 * - 20: 4HSwing_100P
 * - 24: MACDDaily
 * - 25: MACDWeekly
 * - 26: Ichimoko_Daily
 * - 27: 4HSwing_Shellington
 * - 28: MACD_BEILI
 * - 29: (Removed: Daily_Swing_ExecutionOnly)
 * - 31: ShortTerm_New
 * - 32: Range orders
 * - 33: Ichimoko_Weekly
 */
void splitBuyOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	switch (pIndicators->splitTradeMode)
	{
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
	case 6:
	case 7:
	case 13:
	case 17:
	case 18:
	case 21:
		// Reserved/empty modes
		break;
	case 22:
		splitBuyOrders_MultiDays_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 30:
		// Reserved/empty modes
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
	case 14:
		// Removed: splitBuyOrders_ShortTerm_Hedge
		break;
	case 15:
		// Removed: splitBuyOrders_ShortTerm_ATR_Hedge
		break;
	case 16:
		splitBuyOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 19:
		splitBuyOrders_4HSwing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 20:
		splitBuyOrders_4HSwing_100P(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
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
		// Removed: splitBuyOrders_Daily_Swing_ExecutionOnly
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

/**
 * Main dispatcher for sell order splitting.
 * 
 * Routes sell order splitting requests to specific implementations based on
 * the splitTradeMode parameter set by strategy execution functions.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing splitTradeMode
 * @param pBase_Indicators Base indicators structure
 * @param takePrice_primary Primary take profit value
 * @param stopLoss Stop loss value
 * 
 * Split trade modes (same as splitBuyOrders):
 * - 0: ShortTerm
 * - 3: LongTerm
 * - 4: Limit (Fibonacci retracement)
 * - 8: WeeklyTrading
 * - 9: KeyK
 * - 10: Weekly_Beginning
 * - 11: Weekly_ShortTerm
 * - 12: ATR
 * - 14: (Removed: ShortTerm_Hedge)
 * - 15: (Removed: ShortTerm_ATR_Hedge)
 * - 16: Daily_Swing
 * - 19: 4HSwing
 * - 20: 4HSwing_100P
 * - 24: MACDDaily
 * - 25: MACDWeekly
 * - 26: Ichimoko_Daily
 * - 27: 4HSwing_Shellington
 * - 28: MACD_BEILI
 * - 29: (Removed: Daily_Swing_ExecutionOnly)
 * - 31: ShortTerm_New
 * - 32: Range orders
 */
void splitSellOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	switch (pIndicators->splitTradeMode)
	{
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
		// Reserved/empty mode
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
	case 17:
	case 18:
	case 21:
		// Reserved/empty modes
		break;
	case 22:
		splitSellOrders_MultiDays_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 30:
		// Reserved/empty modes
		break;
	case 14:
		// Removed: splitSellOrders_ShortTerm_Hedge
		break;
	case 15:
		// Removed: splitSellOrders_ShortTerm_ATR_Hedge
		break;
	case 16:
		splitSellOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 19:
		splitSellOrders_4HSwing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 20:
		splitSellOrders_4HSwing_100P(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
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
		// Removed: splitSellOrders_Daily_Swing_ExecutionOnly
		break;
	case 31:
		splitSellOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 32:
		splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		break;
	}
}
