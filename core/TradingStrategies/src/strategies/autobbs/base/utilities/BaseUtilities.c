/*
 * Base Utilities Module
 * 
 * Provides utility functions for Base strategy:
 * - Order modification
 * - Limit order management
 * - Day detection
 * 
 * These utilities are used across the Base strategy implementation
 * for common operations like detecting new trading days and managing
 * limit orders from previous days.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/utilities/BaseUtilities.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// Day detection constants
#define NEW_DAY_MINUTE_THRESHOLD 15        // Minimum minute of hour to consider it a new day
#define SECONDS_PER_HOUR 3600              // Seconds in one hour

/**
 * Modifies orders for a given order type (BUY or SELL).
 * 
 * Updates stop loss for all open orders of the specified type.
 * The take price parameter is not used (set to -1) as this function
 * only modifies stop loss levels.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param orderType Order type to modify (BUY or SELL)
 * @param stopLoss New stop loss level to apply
 * @param takePrice Take price (not used, kept for API consistency)
 * @return SUCCESS on success
 * 
 * Note: This function modifies all open orders of the specified type.
 * The tpMode is set to 0 (no take profit change) and the last parameter
 * is TRUE to indicate this is a new day modification.
 */
AsirikuyReturnCode base_ModifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice)
{	
	int tpMode = 0;

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
			modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode, TRUE);
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
			modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode, TRUE);
	}

	return SUCCESS;
}

/**
 * Closes all limit orders from the previous day.
 * 
 * This function is called to clean up limit orders that were placed
 * on the previous trading day. It closes both buy and sell limit orders
 * to prevent them from executing on the new day.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * 
 * Note: Uses the current time from the primary rates buffer to determine
 * which orders to close. This ensures limit orders don't carry over
 * between trading days.
 */
void closeAllLimitPreviousDayOrders(StrategyParams* pParams)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	
	// Close all buy and sell limit orders from previous day
	closeAllBuyLimitOrdersEasy(currentTime);
	closeAllSellLimitOrdersEasy(currentTime);
}

/**
 * Determines if the current time represents a new trading day.
 * 
 * Checks if we've moved to a new trading day by comparing:
 * 1. The current time's day of month with the last order update time's day
 * 2. Whether the current minute is >= 15 (to avoid false positives at midnight)
 * 
 * This is used to trigger day-specific operations like closing limit orders
 * and updating stop loss levels.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param currentTime Current time to check
 * @return TRUE if it's a new day, FALSE otherwise
 * 
 * Conditions for new day:
 * - Last order update time is valid (not -1)
 * - Day of month has changed (tm_mday differs)
 * - Current minute is >= 15 (to ensure we're past midnight transition)
 * 
 * Note: The 15-minute threshold prevents false positives during the
 * midnight hour when day transitions occur.
 */
BOOL isNewDay(StrategyParams* pParams, time_t currentTime)
{
	time_t virtualOrderEntryTime;
	struct tm timeInfo1, timeInfo2;
	char timeString1[MAX_TIME_STRING_SIZE] = "";
	char timeString2[MAX_TIME_STRING_SIZE] = "";

	virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

	safe_gmtime(&timeInfo1, currentTime);
	safe_gmtime(&timeInfo2, virtualOrderEntryTime);

	safe_timeString(timeString1, currentTime);
	safe_timeString(timeString2, virtualOrderEntryTime);

	// Log time difference for debugging
	logDebug("Last order update time = %s, current time = %s, Time Difference (hours) = %lf", 
	         timeString2, timeString1, difftime(currentTime, virtualOrderEntryTime) / SECONDS_PER_HOUR);

	// Check if it's a new day:
	// 1. Last update time must be valid (not -1)
	// 2. Day of month must have changed
	// 3. Current minute must be >= 15 (to avoid false positives at midnight)
	if (virtualOrderEntryTime != -1 && 
	    timeInfo1.tm_mday != timeInfo2.tm_mday && 
	    timeInfo1.tm_min >= NEW_DAY_MINUTE_THRESHOLD)
	{
		logDebug("Move to a new day.\n");
		return TRUE;
	}
	
	return FALSE;
}
