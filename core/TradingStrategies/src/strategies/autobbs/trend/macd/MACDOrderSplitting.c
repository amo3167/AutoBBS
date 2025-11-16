/*
 * MACD Order Splitting Module
 * 
 * Provides order splitting functions for MACD Daily and Weekly strategies.
 * These functions handle the distribution of orders across multiple take profit
 * levels based on risk management parameters and trade mode settings.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"

// Constants for take profit and risk calculations
#define TP_MULTIPLIER_DAILY_ATR 1.5        // Take profit multiplier for daily ATR (1.5x)
#define MIN_LOT_SIZE_THRESHOLD 5            // Minimum lot size threshold for splitting orders
#define TP_MODE_STANDARD 1                  // Standard trade mode (1)
#define TP_MODE_1_TO_1 0                   // 1:1 risk/reward trade mode (0)

/**
 * @brief Splits buy orders for MACD Daily strategy.
 * 
 * This function creates buy orders based on trade mode:
 * - Mode 1: Uses risk cap and daily ATR for take profit calculation.
 *   If lot size exceeds threshold, splits into two orders (one with TP, one without).
 * - Mode 0: Uses 1:1 risk/reward ratio (take profit = stop loss).
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing daily ATR.
 * @param atr Average True Range (not directly used, dailyATR is used instead).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitBuyOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	if (pIndicators->tradeMode == TP_MODE_STANDARD)
	{
		// Calculate take profit based on risk cap and daily ATR
		takePrice = pIndicators->riskCap * TP_MULTIPLIER_DAILY_ATR * pBase_Indicators->dailyATR;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, TP_MULTIPLIER_DAILY_ATR * pBase_Indicators->dailyATR) * pIndicators->risk;

		// If risk cap is set and lot size meets minimum, consider splitting
		if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			
			// If lot size exceeds threshold, split into two orders
			if (lots / pIndicators->volumeStep > MIN_LOT_SIZE_THRESHOLD)
			{
				// Main order with take profit
				openSingleLongEasy(takePrice, stopLoss, lots - pIndicators->minLotSize, 0);

				// Secondary order without specific take profit (managed by trailing stop)
				takePrice = 0;
				openSingleLongEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
			{
				openSingleLongEasy(takePrice, stopLoss, lots, 0);
			}
		}
		else
		{
			openSingleLongEasy(takePrice, stopLoss, lots, 0);
		}
	}
	else
	{
		// Mode 0: 1:1 risk/reward ratio
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);		
	}
}

/**
 * @brief Splits sell orders for MACD Daily strategy.
 * 
 * This function creates sell orders based on trade mode:
 * - Mode 1: Uses risk cap and daily ATR for take profit calculation.
 *   If lot size exceeds threshold, splits into two orders (one with TP, one without).
 * - Mode 0: Uses 1:1 risk/reward ratio (take profit = stop loss).
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing daily ATR.
 * @param atr Average True Range (not directly used, dailyATR is used instead).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitSellOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	if (pIndicators->tradeMode == TP_MODE_STANDARD)
	{
		// Calculate take profit based on risk cap and daily ATR
		takePrice = pIndicators->riskCap * TP_MULTIPLIER_DAILY_ATR * pBase_Indicators->dailyATR;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, TP_MULTIPLIER_DAILY_ATR * pBase_Indicators->dailyATR) * pIndicators->risk;

		// If risk cap is set, consider splitting
		if (pIndicators->riskCap > 0)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			
			// If lot size exceeds threshold, split into two orders
			if (lots / pIndicators->volumeStep > MIN_LOT_SIZE_THRESHOLD)
			{
				// Main order with take profit
				openSingleShortEasy(takePrice, stopLoss, lots - pIndicators->minLotSize, 0);

				// Secondary order without specific take profit (managed by trailing stop)
				takePrice = 0;
				openSingleShortEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
			{
				openSingleShortEasy(takePrice, stopLoss, lots, 0);
			}
		}
		else
		{
			openSingleShortEasy(takePrice, stopLoss, lots, 0);
		}
	}
	else
	{
		// Mode 0: 1:1 risk/reward ratio
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * @brief Splits buy orders for MACD Weekly strategy.
 * 
 * This function creates a single buy order:
 * - Mode 1: No specific take profit (0), managed by trailing stop or other exit logic.
 * - Mode 0: Take profit set to weekly ATR.
 * 
 * Lot size is calculated based on 1.5x weekly ATR for risk management.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing weekly ATR.
 * @param atr Average True Range (not directly used, weeklyATR is used instead).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitBuyOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;

	if (pIndicators->tradeMode == TP_MODE_STANDARD)
	{
		takePrice = 0; // No specific take profit, managed by trailing stop
	}
	else
	{
		takePrice = pBase_Indicators->weeklyATR;
	}

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, TP_MULTIPLIER_DAILY_ATR * pBase_Indicators->weeklyATR) * pIndicators->risk;
	openSingleLongEasy(takePrice, stopLoss, lots, 0);
}

/**
 * @brief Splits sell orders for MACD Weekly strategy.
 * 
 * This function creates a single sell order:
 * - Mode 1: No specific take profit (0), managed by trailing stop or other exit logic.
 * - Mode 0: Take profit set to weekly ATR.
 * 
 * Lot size is calculated based on 1.5x weekly ATR for risk management.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing weekly ATR.
 * @param atr Average True Range (not directly used, weeklyATR is used instead).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitSellOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	if (pIndicators->tradeMode == TP_MODE_STANDARD)
	{
		takePrice = 0; // No specific take profit, managed by trailing stop
	}
	else
	{
		takePrice = pBase_Indicators->weeklyATR;
	}

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, TP_MULTIPLIER_DAILY_ATR * pBase_Indicators->weeklyATR) * pIndicators->risk;
	openSingleShortEasy(takePrice, stopLoss, lots, 0);
}

