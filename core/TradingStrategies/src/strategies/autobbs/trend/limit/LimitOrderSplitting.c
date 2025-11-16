/*
 * Limit Order Splitting Module
 * 
 * Provides order splitting functions for Limit strategies.
 * These functions handle the placement of limit and stop orders at key
 * support/resistance levels (pivot, S1, R1) for range trading strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"

// ATR calculation constants
#define ATR_PERIOD_HOURLY 20                 // ATR period for hourly calculation
#define ATR_DIVISOR_FOR_PENDING_CHECK 3       // ATR divisor for pending order check

// Order splitting constants
#define LOT_SIZE_SPLIT_HALF 2.0              // Divisor for splitting lots in half
#define MAX_WIN_TIMES_FOR_ENTRY 1            // Maximum win times allowed before entering order

// Time constants
#define SR1_ENABLE_HOUR_THRESHOLD 8          // Hour threshold for enabling SR1 orders

/**
 * @brief Splits buy range orders for Limit strategy.
 * 
 * This function places buy limit orders at two key levels:
 * 1. Daily pivot point
 * 2. Daily S1 (first support level)
 * 
 * Each order uses half of the calculated lot size. Orders are only placed if:
 * - No existing buy stop order at the same price
 * - No existing pending order at the same price
 * - Win times at the same price in the day is less than MAX_WIN_TIMES_FOR_ENTRY
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, take price, stop loss.
 * @param pBase_Indicators Base indicators containing daily pivot and S1 levels.
 */
void splitRangeBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double lots;
	double gap = iAtr(B_HOURLY_RATES, ATR_PERIOD_HOURLY, 1);
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->stopLoss) * pIndicators->risk;

	// Place buy limit order at daily pivot
	pIndicators->entryPrice = pBase_Indicators->dailyPivot;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
		)
	{
		openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots / LOT_SIZE_SPLIT_HALF, 1);
	}

	// Place buy limit order at daily S1
	pIndicators->entryPrice = pBase_Indicators->dailyS1;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
		)
	{
		openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots / LOT_SIZE_SPLIT_HALF, 1);
	}
	

}

/**
 * @brief Splits sell range orders for Limit strategy.
 * 
 * This function places sell limit orders at two key levels:
 * 1. Daily pivot point
 * 2. Daily R1 (first resistance level)
 * 
 * Each order uses half of the calculated lot size. Orders are only placed if:
 * - No existing buy stop order at the same price
 * - No existing pending order at the same price
 * - Win times at the same price in the day is less than MAX_WIN_TIMES_FOR_ENTRY
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, take price, stop loss.
 * @param pBase_Indicators Base indicators containing daily pivot and R1 levels.
 */
void splitRangeSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double currentPrice;
	double lots;
	double gap = iAtr(B_HOURLY_RATES, ATR_PERIOD_HOURLY, 1);
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->stopLoss) * pIndicators->risk;

	currentPrice = pParams->bidAsk.ask[0];

	// Place sell limit order at daily pivot
	pIndicators->entryPrice = pBase_Indicators->dailyPivot;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
		)
	{
		openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots / LOT_SIZE_SPLIT_HALF, 1);
	}

	// Place sell limit order at daily R1
	pIndicators->entryPrice = pBase_Indicators->dailyR1;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
		)
	{
		openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots / LOT_SIZE_SPLIT_HALF, 1);
	}


	

}

// Mode constants
#define MODE_STANDARD 0                      // Standard mode
#define MODE_ENABLE_R1_STOP 1                // Mode to enable stop orders on R1

/**
 * @brief Splits buy orders for Limit strategy.
 * 
 * This function places buy limit or stop orders based on current price relative to pivot:
 * - If current price <= pivot: Places stop order at pivot (if conditions met)
 * - If current price > pivot: Places limit order at pivot (if conditions met)
 * 
 * Additional logic:
 * - If mode == 1 and orders exist today: Places stop order at R1
 * - If SR1 is enabled and hour < 8: Places limit/stop order at S1 based on price
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, take price, stop loss, start hour, etc.
 * @param pBase_Indicators Base indicators containing daily pivot, S1, R1 levels.
 * @param mode Mode flag (0 = standard, 1 = enable R1 stop orders).
 * @param takePrice_primary Primary take price (not directly used, pIndicators->takePrice is used).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int mode, double takePrice_primary, double stopLoss)
{	
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentPrice;
	double lots;
	double gap = iAtr(B_HOURLY_RATES, 20, 1);
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;		

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];	
	
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;
	logInfo("System InstanceID = %d, BarTime = %s, lots=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lots);

	currentPrice = pParams->bidAsk.ask[0];		
	if (currentPrice <= pBase_Indicators->dailyPivot)
	{			
		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleBuyStopEasy(pIndicators->entryPrice + pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
		}

		if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour < SR1_ENABLE_HOUR_THRESHOLD)
		{
			pIndicators->entryPrice = pBase_Indicators->dailyS1;
			if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
				)
			{
				if (currentPrice > pIndicators->entryPrice)
					openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
				else
					openSingleBuyStopEasy(pIndicators->entryPrice + pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
			}
		}
	}
	else
	{

		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
			//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
		}

		if (mode == MODE_ENABLE_R1_STOP
			&& getOrderCountTodayEasy(currentTime) > 0
			) 
		{
			pIndicators->entryPrice = pBase_Indicators->dailyR1;
			if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
				)
			{
				openSingleBuyStopEasy(pIndicators->entryPrice + pIndicators->adjust, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots);
			}

		}
			
	}

}

/**
 * @brief Splits sell orders for Limit strategy.
 * 
 * This function places sell limit or stop orders based on current price relative to pivot:
 * - If current price > pivot: Places stop order at pivot (if conditions met)
 * - If current price <= pivot: Places limit order at pivot (if conditions met)
 * 
 * Additional logic:
 * - If mode == 1 and orders exist today: Places stop order at S1
 * - If SR1 is enabled and hour < 8: Places limit/stop order at R1 based on price
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, take price, stop loss, start hour, etc.
 * @param pBase_Indicators Base indicators containing daily pivot, S1, R1 levels.
 * @param mode Mode flag (0 = standard, 1 = enable S1 stop orders).
 * @param takePrice_primary Primary take price (not directly used, pIndicators->takePrice is used).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int mode, double takePrice_primary, double stopLoss)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double currentPrice;
	double lots;
	double gap = iAtr(B_HOURLY_RATES, ATR_PERIOD_HOURLY, 1);
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;

	logInfo("System InstanceID = %d, BarTime = %s, lots=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lots);

	currentPrice = pParams->bidAsk.bid[0];
	if (currentPrice > pBase_Indicators->dailyPivot)
	{			
		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleSellStopEasy(pIndicators->entryPrice - pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
		}

		if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour < SR1_ENABLE_HOUR_THRESHOLD)
		{			
			pIndicators->entryPrice = pBase_Indicators->dailyR1;
			if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
				)
			{
				if (currentPrice < pIndicators->entryPrice)
					openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
				else
					openSingleSellStopEasy(pIndicators->entryPrice - pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
			}
		}

	}
	else
	{
		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
		}

		if (mode == MODE_ENABLE_R1_STOP
			&& getOrderCountTodayEasy(currentTime) > 0)
		{
			pIndicators->entryPrice = pBase_Indicators->dailyS1;
			if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < MAX_WIN_TIMES_FOR_ENTRY
				)
			{
				openSingleSellStopEasy(pIndicators->entryPrice - pIndicators->adjust, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots);
			}

		}

	}


}
