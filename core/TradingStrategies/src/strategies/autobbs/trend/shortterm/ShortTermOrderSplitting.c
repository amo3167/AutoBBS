/*
 * ShortTerm Order Splitting Module
 * 
 * Provides order splitting functions for ShortTerm strategies.
 * This module handles order splitting for short-term trading based on
 * gap analysis and ATR calculations.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "strategies/autobbs/trend/shortterm/ShortTermOrderSplitting.h"

// Order splitting constants
#define MIN_LOT_SIZE 0.01                    // Minimum lot size
#define GAP_TAKE_PROFIT_DIVISOR 3            // Divisor for gap take profit calculation (1/3 of gap)
#define ATR_GAP_THRESHOLD_FACTOR 0.5         // ATR factor for gap threshold (50%)
#define RISK_SPLIT_HALF 0.5                  // Risk split for two orders (50% each)
#define MAX_RISK_MULTIPLIER 7                // Maximum risk multiplier for lot size capping

/**
 * @brief Splits buy orders for short-term strategy (new version).
 * 
 * This function opens a single buy order with calculated lot size based on
 * entry price, take price, and risk. The take price is set from indicators.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, take price, and risk.
 * @param pBase_Indicators Base indicators (not directly used).
 * @param takePrice_primary Primary take profit (not used in this function).
 * @param stopLoss Stop loss value.
 */
void splitBuyOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double lots;
	double miniLots = MIN_LOT_SIZE;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;
	openSingleLongEasy(pIndicators->takePrice, stopLoss, lots, pIndicators->risk);
}

/**
 * @brief Splits sell orders for short-term strategy (new version).
 * 
 * This function opens a single sell order with calculated lot size based on
 * entry price, take price, and risk. The take price is set from indicators.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, take price, and risk.
 * @param pBase_Indicators Base indicators (not directly used).
 * @param takePrice_primary Primary take profit (not used in this function).
 * @param stopLoss Stop loss value.
 */
void splitSellOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double lots;
	double miniLots = MIN_LOT_SIZE;
	
	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;
	openSingleShortEasy(pIndicators->takePrice, stopLoss, lots, pIndicators->risk);
}

/**
 * @brief Splits buy orders for short-term strategy (legacy version).
 * 
 * This function implements gap-based order splitting:
 * - If tradeMode == 1: Opens order when gap >= 50% of predicted ATR and daily close change is small
 * - Take profit = 1/3 of gap
 * - Lot size is capped to maximum risk (7% of account risk)
 * - If tradeMode != 1: Opens two orders at R1 level (50% risk each)
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, and trade mode.
 * @param pBase_Indicators Base indicators containing predicted ATR and daily levels.
 * @param takePrice_primary Primary take profit (not used in this function).
 * @param stopLoss Stop loss value.
 */
void splitBuyOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pHigh - pIndicators->entryPrice;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	double riskcap = parameter(AUTOBBS_RISK_CAP);
	double lots_max;
	int orderCountToday = 0;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Gap-based order splitting (tradeMode == 1)
	if (pIndicators->tradeMode == 1)
	{
		// Open order if gap is significant and daily close change is small
		if (gap >= pATR * ATR_GAP_THRESHOLD_FACTOR && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{
				// Take profit = 1/3 of gap
				takePrice = gap / GAP_TAKE_PROFIT_DIVISOR;

				lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap lot size to maximum risk (7% of account risk)
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, BUY, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * MAX_RISK_MULTIPLIER);
				lots = min(lots_max, lots);

				openSingleLongEasy(takePrice, stopLoss, lots, 0);

				// Log entry signal
				if (pIndicators->entrySignal == 1)
				{
					if (pIndicators->subTradeMode == 0)
						logWarning("System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						logWarning("System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
				}
			}
		}
	}
	// Standard order splitting (tradeMode != 1)
	else
	{
		// Open two orders at R1 level if entry price is below R1
		if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
		{
			// First order: take profit = stop loss, 50% risk
			takePrice = stopLoss;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_HALF);
			
			// Second order: no take profit, 50% risk
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_HALF);
		}
	}
}

/**
 * @brief Splits sell orders for short-term strategy (legacy version).
 * 
 * This function implements gap-based order splitting:
 * - If tradeMode == 1: Opens order when gap >= 50% of predicted ATR and daily close change is small
 * - Take profit = 1/3 of gap
 * - Lot size is capped to maximum risk (7% of account risk)
 * - If tradeMode != 1: Opens two orders at S1 level (50% risk each)
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, and trade mode.
 * @param pBase_Indicators Base indicators containing predicted ATR and daily levels.
 * @param takePrice_primary Primary take profit (not used in this function).
 * @param stopLoss Stop loss value.
 */
void splitSellOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pIndicators->entryPrice - pLow;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	double riskcap = parameter(AUTOBBS_RISK_CAP);
	double lots_max;
	int orderCountToday = 0;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Gap-based order splitting (tradeMode == 1)
	if (pIndicators->tradeMode == 1)
	{
		// Open order if gap is significant and daily close change is small
		if (gap >= pATR * ATR_GAP_THRESHOLD_FACTOR && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{
				// Take profit = 1/3 of gap
				takePrice = gap / GAP_TAKE_PROFIT_DIVISOR;
				lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap lot size to maximum risk (7% of account risk)
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, SELL, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * MAX_RISK_MULTIPLIER);
				lots = min(lots_max, lots);

				openSingleShortEasy(takePrice, stopLoss, lots, 0);

				// Log entry signal
				if (pIndicators->entrySignal == -1)
				{
					if (pIndicators->subTradeMode == 0)
						logWarning("System InstanceID = %d, BarTime = %s, entry short trade on retreated signal in BBS break out.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						logWarning("System InstanceID = %d, BarTime = %s, entry short trade on retreated signal in BBS Retreat.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
				}
			}
		}
	}
	// Standard order splitting (tradeMode != 1)
	else
	{
		// Open two orders at S1 level if entry price is above S1
		if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
		{
			// First order: take profit = stop loss, 50% risk
			takePrice = stopLoss;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_HALF);
			
			// Second order: no take profit, 50% risk
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_HALF);
		}
	}
}
