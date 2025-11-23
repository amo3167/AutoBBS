/*
 * BBS Order Splitting Module
 * 
 * Provides order splitting functions for BBS (Bollinger Bands Stop) Swing strategies.
 * 
 * This module implements various order splitting strategies for 4H swing trading:
 * - Standard 4H Swing: Splits orders into two equal parts with different take profit levels
 * - 4H Swing 100P: Uses weekly ATR prediction for take profit, adjusts lot size based on loss history
 * - 4H Swing Shellington: Uses Shellington indicator with risk cap and minimum lot size controls
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"

// Timeframe constants
#define TIMEFRAME_15M 15

// Risk distribution constants
#define RISK_SPLIT_HALF 2.0              // Split risk in half (50/50)
#define WEEKLY_ATR_DIVISOR 2.0           // Weekly ATR divisor for take profit calculation
#define DAILY_ATR_MULTIPLIER 1.5         // Daily ATR multiplier for stop loss calculation
#define LOSS_TIMES_THRESHOLD 1            // Threshold for loss times before adjusting lot size

/**
 * Splits buy orders for 4H Swing strategy.
 * 
 * Creates two buy orders with equal risk distribution:
 * - First order: Take profit = daily gap (distance to daily high)
 * - Second order: Take profit = ATR value
 * 
 * Only executes if entry price is below daily R1 and timeframe is 15M.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing daily levels
 * @param atr ATR value to use for second order's take profit
 * @param stopLoss Stop loss value for both orders
 */
void splitBuyOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	// Only execute if entry price is below daily R1 and timeframe is 15M
	if (pIndicators->entryPrice <= pBase_Indicators->dailyR1 && (int)pParams->settings[TIMEFRAME] == TIMEFRAME_15M)
	{
		// First order: Take profit = daily gap (distance to daily high)
		takePrice = dailyGap;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_HALF);

		// Second order: Take profit = ATR value
		takePrice = atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_HALF);
	}
}

/**
 * Splits sell orders for 4H Swing strategy.
 * 
 * Creates two sell orders with equal risk distribution:
 * - First order: Take profit = daily gap (distance to daily low)
 * - Second order: Take profit = ATR value
 * 
 * Only executes if entry price is above daily S1 and timeframe is 15M.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing daily levels
 * @param atr ATR value to use for second order's take profit
 * @param stopLoss Stop loss value for both orders
 */
void splitSellOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyLow = pBase_Indicators->pDailyLow;
	double dailyGap = pIndicators->entryPrice - pDailyLow;

	// Only execute if entry price is above daily S1 and timeframe is 15M
	if (pIndicators->entryPrice >= pBase_Indicators->dailyS1 && (int)pParams->settings[TIMEFRAME] == TIMEFRAME_15M)
	{
		// First order: Take profit = daily gap (distance to daily low)
		takePrice = dailyGap;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_HALF);

		// Second order: Take profit = ATR value
		takePrice = atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_SPLIT_HALF);
	}
}

/**
 * Splits buy orders for 4H Swing 100P strategy.
 * 
 * Uses weekly ATR prediction for take profit calculation and adjusts lot size
 * based on recent loss history. The take profit is set to half of weekly predicted ATR,
 * capped at daily ATR.
 * 
 * Lot size calculation:
 * - If loss times <= 1: Use standard calculated lot size
 * - If loss times > 1: Use lot size based on total previous losses
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param atr ATR value (not used in this function, kept for API consistency)
 * @param stopLoss Stop loss value
 */
void splitBuyOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots, lots_singal;
	double total_pre_lost = 0;
	int lostTimes;
	time_t currentTime;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	// Calculate take profit: half of weekly predicted ATR, capped at daily ATR
	takePrice = pBase_Indicators->pWeeklyPredictATR / WEEKLY_ATR_DIVISOR;
	takePrice = min(takePrice, pBase_Indicators->dailyATR);

	// Calculate standard lot size
	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;

	// Get loss history
	lostTimes = getLossTimesFromNowsEasy(currentTime, &total_pre_lost);

	// Adjust lot size based on loss history
	if (lostTimes <= LOSS_TIMES_THRESHOLD)
		lots = lots_singal;
	else
		lots = total_pre_lost / takePrice;

	openSingleLongEasy(takePrice, stopLoss, lots, 0);
}

/**
 * Splits sell orders for 4H Swing 100P strategy.
 * 
 * Uses weekly ATR prediction for take profit calculation and adjusts lot size
 * based on recent loss history. The take profit is set to half of weekly predicted ATR,
 * capped at daily ATR.
 * 
 * Lot size calculation:
 * - If loss times <= 1: Use standard calculated lot size
 * - If loss times > 1: Use lot size based on total previous losses
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param atr ATR value (not used in this function, kept for API consistency)
 * @param stopLoss Stop loss value
 */
void splitSellOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots, lots_singal;
	double total_pre_lost = 0;
	int lostTimes;
	time_t currentTime;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	// Calculate take profit: half of weekly predicted ATR, capped at daily ATR
	takePrice = pBase_Indicators->pWeeklyPredictATR / WEEKLY_ATR_DIVISOR;
	takePrice = min(takePrice, pBase_Indicators->dailyATR);

	// Calculate standard lot size
	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

	// Get loss history
	lostTimes = getLossTimesFromNowsEasy(currentTime, &total_pre_lost);

	// Adjust lot size based on loss history
	if (lostTimes <= LOSS_TIMES_THRESHOLD)
		lots = lots_singal;
	else
		lots = total_pre_lost / takePrice;

	openSingleShortEasy(takePrice, stopLoss, lots, 0);
}

/**
 * Splits buy orders for 4H Swing Shellington strategy.
 * 
 * Uses Shellington indicator with risk cap and minimum lot size controls.
 * The stop loss is set to the maximum of the provided stop loss and 1.5x daily ATR.
 * Take profit is set to the maximum of the indicator's take price and risk cap * stop loss.
 * 
 * Lot size can be overridden by minimum lot size setting if enabled.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing daily ATR
 * @param atr ATR value (not used in this function, kept for API consistency)
 * @param stopLoss Stop loss value (will be adjusted to max(stopLoss, dailyATR * 1.5))
 */
void splitBuyOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots_singal;
	time_t currentTime;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	// Calculate lot size
	if (pIndicators->isEnableBuyMinLotSize == TRUE)
	{
		// Use minimum lot size if enabled
		lots_singal = pIndicators->minLotSize;
	}
	else
	{
		// Calculate lot size based on stop loss (use max of stopLoss or dailyATR * 1.5)
		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, 
		                                  max(stopLoss, pBase_Indicators->dailyATR * DAILY_ATR_MULTIPLIER)) * pIndicators->risk;
	}

	// Round lot size to minimum lot size
	lots_singal = roundUp(lots_singal, pIndicators->minLotSize);

	// Calculate take profit: max of indicator take price or risk cap * stop loss
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);

	openSingleLongEasy(takePrice, stopLoss, lots_singal, 0);
}

/**
 * Splits sell orders for 4H Swing Shellington strategy.
 * 
 * Uses Shellington indicator with risk cap and minimum lot size controls.
 * The stop loss is set to the maximum of the provided stop loss and 1.5x daily ATR.
 * Take profit is set to the maximum of the indicator's take price and risk cap * stop loss.
 * 
 * Lot size can be overridden by minimum lot size setting if enabled.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure containing daily ATR
 * @param atr ATR value (not used in this function, kept for API consistency)
 * @param stopLoss Stop loss value (will be adjusted to max(stopLoss, dailyATR * 1.5))
 */
void splitSellOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots_singal;
	time_t currentTime;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	// Calculate lot size
	if (pIndicators->isEnableSellMinLotSize == TRUE)
	{
		// Use minimum lot size if enabled
		lots_singal = pIndicators->minLotSize;
	}
	else
	{
		// Calculate lot size based on stop loss (use max of stopLoss or dailyATR * 1.5)
		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, 
		                                  max(stopLoss, pBase_Indicators->dailyATR * DAILY_ATR_MULTIPLIER)) * pIndicators->risk;
	}

	// Calculate take profit: max of indicator take price or risk cap * stop loss
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);

	// Round lot size to minimum lot size
	lots_singal = roundUp(lots_singal, pIndicators->minLotSize);

	openSingleShortEasy(takePrice, stopLoss, lots_singal, 0);
}
