/*
 * Range Order Management Module
 * 
 * Provides functions for managing range orders (buy/sell orders based on pivot points).
 * Range orders are entered when price is near support/resistance levels (S2/R2) and
 * market conditions indicate a range-bound trading environment.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"

// Range order constants
#define RANGE_ORDER_ENABLED 1                    // AUTOBBS_RANGE parameter value to enable range orders
#define MIN_WIN_TIMES_FOR_RANGE_ORDER 1         // Maximum win times allowed before entering range order
#define RANGE_ORDER_START_HOUR 17                // Hour when range orders can be entered (5 PM)
#define ATR_MULTIPLIER_FOR_RANGE_CHECK 3         // Multiplier for ATR when checking win times
#define ATR_PERIOD_FOR_RANGE_CHECK 20           // ATR period for range order checks
#define MOVEMENT_THRESHOLD_MULTIPLIER 1.5        // Multiplier for movement threshold check
#ifndef SECONDS_PER_DAY
#define SECONDS_PER_DAY (60 * 60 * 24)          // Seconds in a day
#endif

/**
 * @brief Enters a buy range order when conditions are met.
 * 
 * A buy range order is entered when:
 * - Range trading is enabled (AUTOBBS_RANGE == 1)
 * - No existing open order at the orderIndex
 * - Win times at same price are less than threshold
 * - Daily low is significantly below daily high (range condition)
 * - Current ask price is below daily S2 (support level)
 * - Current hour is >= 17 (after 5 PM)
 * 
 * If conditions are met and isEnterOrder is TRUE, the order is split and entered.
 * If isOrderSignal is TRUE and hour >= stopHour, a pending signal is saved.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators.
 * @param pBase_Indicators Base indicators containing pivot levels.
 * @param orderIndex Index to check for existing orders (-1 if none).
 * @param stopHour Hour threshold for saving order signal.
 * @param isOrderSignal TRUE to save order signal to file.
 * @param isEnterOrder TRUE to actually enter the order.
 * @return TRUE if conditions are met, FALSE otherwise.
 */
BOOL entryBuyRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder)
{
	time_t currentTime;
	struct tm timeInfo1;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	Order_Info orderInfo;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	// Check range order conditions
	if ((int)parameter(AUTOBBS_RANGE) == RANGE_ORDER_ENABLED
		&& (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], ATR_MULTIPLIER_FOR_RANGE_CHECK * iAtr(B_HOURLY_RATES, ATR_PERIOD_FOR_RANGE_CHECK, 1)) < MIN_WIN_TIMES_FOR_RANGE_ORDER
		&& iLow(B_DAILY_RATES, 0) <= iHigh(B_DAILY_RATES, 0) - pBase_Indicators->pDailyMaxATR
		&& pParams->bidAsk.ask[0] < pBase_Indicators->dailyS2
		&& timeInfo1.tm_hour >= RANGE_ORDER_START_HOUR)
	{
		// Enter order if conditions are met and BBS trend is bullish or movement threshold reached
		if (isEnterOrder == TRUE && (pIndicators->bbsTrend_primary == 1
			|| iClose(B_PRIMARY_RATES, 1) - iLow(B_DAILY_RATES, 0) >= MOVEMENT_THRESHOLD_MULTIPLIER * iAtr(B_HOURLY_RATES, ATR_PERIOD_FOR_RANGE_CHECK, 1)))
		{
			splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
		}

		// Save order signal to file if requested
		if (isOrderSignal == TRUE && timeInfo1.tm_hour >= stopHour)
		{
			orderInfo.orderNumber = 0;
			orderInfo.type = BUY;
			orderInfo.orderStatus = PENDING;
			orderInfo.openPrice = 0;
			orderInfo.stopLossPrice = 0;
			orderInfo.takeProfitPrice = 0;
			orderInfo.timeStamp = currentTime;

			saveTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);
		}
	}
	return TRUE;
}

/**
 * @brief Enters a sell range order when conditions are met.
 * 
 * A sell range order is entered when:
 * - Range trading is enabled (AUTOBBS_RANGE == 1)
 * - No existing open order at the orderIndex
 * - Win times at same price are less than threshold
 * - Daily high is significantly above daily low (range condition)
 * - Current bid price is above daily R2 (resistance level)
 * - Current hour is >= 17 (after 5 PM)
 * 
 * If conditions are met and isEnterOrder is TRUE, the order is split and entered.
 * If isOrderSignal is TRUE and hour >= stopHour, a pending signal is saved.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators.
 * @param pBase_Indicators Base indicators containing pivot levels.
 * @param orderIndex Index to check for existing orders (-1 if none).
 * @param stopHour Hour threshold for saving order signal.
 * @param isOrderSignal TRUE to save order signal to file.
 * @param isEnterOrder TRUE to actually enter the order.
 * @return TRUE if conditions are met, FALSE otherwise.
 */
BOOL entrySellRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder)
{
	time_t currentTime;
	struct tm timeInfo1;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	Order_Info orderInfo;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	// Check range order conditions
	if ((int)parameter(AUTOBBS_RANGE) == RANGE_ORDER_ENABLED
		&& (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.bid[0], ATR_MULTIPLIER_FOR_RANGE_CHECK * iAtr(B_HOURLY_RATES, ATR_PERIOD_FOR_RANGE_CHECK, 1)) < MIN_WIN_TIMES_FOR_RANGE_ORDER
		&& iHigh(B_DAILY_RATES, 0) >= iLow(B_DAILY_RATES, 0) + pBase_Indicators->pDailyMaxATR
		&& pParams->bidAsk.bid[0] > pBase_Indicators->dailyR2
		&& timeInfo1.tm_hour >= RANGE_ORDER_START_HOUR)
	{
		// Enter order if conditions are met and BBS trend is bearish or movement threshold reached
		if (isEnterOrder == TRUE && (pIndicators->bbsTrend_primary == -1
			|| iHigh(B_DAILY_RATES, 0) - iClose(B_PRIMARY_RATES, 1) >= MOVEMENT_THRESHOLD_MULTIPLIER * iAtr(B_HOURLY_RATES, ATR_PERIOD_FOR_RANGE_CHECK, 1)))
		{
			splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		}

		// Save order signal to file if requested
		if (isOrderSignal == TRUE && timeInfo1.tm_hour >= stopHour)
		{
			orderInfo.orderNumber = 0;
			orderInfo.type = SELL;
			orderInfo.orderStatus = PENDING;
			orderInfo.openPrice = 0;
			orderInfo.stopLossPrice = 0;
			orderInfo.takeProfitPrice = 0;
			orderInfo.timeStamp = currentTime;

			saveTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);
		}
	}
	return TRUE;
}

/**
 * @brief Checks if an order is a range order based on pivot levels and timing.
 * 
 * This function determines if a closed order was a range order by checking:
 * - Range trading is enabled
 * - Order is closed with negative profit
 * - Order was opened on a different day after 17:00
 * - Order entry price was below R2 (for BUY) or above R2 (for SELL)
 * 
 * It also checks for pending range order signals saved in the trading info file.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators.
 * @param pBase_Indicators Base indicators containing pivot levels.
 * @param orderIndex Index of the order to check.
 * @return 1 if BUY range order, -1 if SELL range order, 2 if pending BUY, -2 if pending SELL, 0 otherwise.
 */
int isRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex)
{
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	double pivot, s1, s2, s3, r1, r2, r3;
	double diffDays;
	Order_Info orderInfo;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	
	iPivot(B_DAILY_RATES, 2, &pivot, &s1, &r1, &s2, &r2, &s3, &r3);
		
	// Check if closed order was a range order
	if ((int)parameter(AUTOBBS_RANGE) == RANGE_ORDER_ENABLED
		&& pParams->orderInfo[orderIndex].isOpen == FALSE
		&& pParams->orderInfo[orderIndex].profit < 0)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].openTime) / SECONDS_PER_DAY;

		iPivot(B_DAILY_RATES, 2, &pivot, &s1, &r1, &s2, &r2, &s3, &r3);

		// Check if order was opened on different day after 17:00 and price was near R2
		if (diffDays < 1 && timeInfo1.tm_yday != timeInfo2.tm_yday && timeInfo2.tm_hour >= RANGE_ORDER_START_HOUR)
		{
			if (pParams->orderInfo[orderIndex].type == BUY && pParams->orderInfo[orderIndex].openPrice < r2)
			{
				return 1;  // BUY range order
			}
				 
			if (pParams->orderInfo[orderIndex].type == SELL && pParams->orderInfo[orderIndex].openPrice > r2)
			{
				return -1;  // SELL range order
			}
		}
	}

	// Check for pending range order signals
	readTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);

	if (orderInfo.orderStatus == PENDING)
	{
		diffDays = difftime(currentTime, orderInfo.timeStamp) / SECONDS_PER_DAY;
		if (diffDays < 1)
		{
			if (orderInfo.type == BUY)
				return 2;  // Pending BUY range order
			if (orderInfo.type == SELL)
				return -2;  // Pending SELL range order
		}
	}
			
	return 0;  // Not a range order
}
