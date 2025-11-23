/**
 * @file DayTradingOrderSplitting.c
 * @brief Day Trading Order Splitting Module
 * 
 * Provides order splitting functions for daily swing trading strategies.
 * These functions are used by splitTradeMode 16 to calculate lot sizes and
 * take profit levels based on daily price gaps and trend phases.
 * 
 * @author AutoBBS Team
 */

#include <math.h>
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "InstanceStates.h"
#include "strategies/autobbs/swing/daytrading/DayTradingOrderSplitting.h"

/**
 * Split buy orders for daily swing strategy.
 * 
 * Calculates lot sizes and take profit levels for buy orders based on:
 * - Daily price gap (high - entry price)
 * - Daily trend phase (range vs trending)
 * - Loss history (loss times and total lose pips)
 * 
 * The function uses a base take profit of 3 pips, which is adjusted based on
 * the gap and trend phase. Lot sizes are calculated to recover previous losses
 * while maintaining risk management.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators containing entry price, risk, and loss history
 * @param pBase_Indicators Base indicators containing daily trend phase and price levels
 * @param takePrice_primary Primary take profit price (unused, kept for API compatibility)
 * @param stopLoss Stop loss price for the orders
 * 
 * @note Used by splitTradeMode 16
 */
void splitBuyOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pHigh = pBase_Indicators->pDailyHigh;
	double lots;
	double lots_signal;
	double gap = pHigh - pIndicators->entryPrice;

	// Base take profit is 3 pips
	takePrice = 3.0;

	// Calculate signal lot size based on base take profit
	lots_signal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;

	// Calculate total lot size to recover losses
	// If loss times < 2, add signal lot; otherwise, only recover losses
	if (pIndicators->lossTimes < 2)
	{
		lots = pIndicators->total_lose_pips / takePrice + lots_signal;
	}
	else
	{
		lots = pIndicators->total_lose_pips / takePrice;
	}

	// Adjust take profit based on gap and trend phase
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	{
		// In range phase: use minimum of gap or 3 pips
		takePrice = min(gap, 3.0);
	}
	else if (gap > 0.0)
	{
		// In trending phase: use maximum of (gap * 2/3) or 3 pips
		takePrice = max(gap * 2.0 / 3.0, 3.0);
	}

	// Open order if take profit is at least 1 pip
	if (takePrice >= 1.0)
	{
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * Split sell orders for daily swing strategy.
 * 
 * Calculates lot sizes and take profit levels for sell orders based on:
 * - Daily price gap (entry price - low)
 * - Daily trend phase (range vs trending)
 * - Loss history (loss times and total lose pips)
 * 
 * The function uses a base take profit of 3 pips, which is adjusted based on
 * the gap and trend phase. Lot sizes are calculated to recover previous losses
 * while maintaining risk management.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators containing entry price, risk, and loss history
 * @param pBase_Indicators Base indicators containing daily trend phase and price levels
 * @param takePrice_primary Primary take profit price (unused, kept for API compatibility)
 * @param stopLoss Stop loss price for the orders
 * 
 * @note Used by splitTradeMode 16
 */
void splitSellOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double lots_signal;
	double gap = pIndicators->entryPrice - pLow;

	// Base take profit is 3 pips
	takePrice = 3.0;

	// Calculate signal lot size based on base take profit
	lots_signal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

	// Calculate total lot size to recover losses
	// If loss times < 2, add signal lot; otherwise, only recover losses
	if (pIndicators->lossTimes < 2)
	{
		lots = pIndicators->total_lose_pips / takePrice + lots_signal;
	}
	else
	{
		lots = pIndicators->total_lose_pips / takePrice;
	}

	// Adjust take profit based on gap and trend phase
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	{
		// In range phase: use minimum of gap or 3 pips
		takePrice = min(gap, 3.0);
	}
	else if (gap > 0.0)
	{
		// In trending phase: use maximum of (gap * 2/3) or 3 pips
		takePrice = max(gap * 2.0 / 3.0, 3.0);
	}

	// Open order if take profit is at least 1 pip
	if (takePrice >= 1.0)
	{
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}
