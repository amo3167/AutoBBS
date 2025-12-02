/*
 * Ichimoko Order Splitting Module
 * 
 * Provides order splitting functions for Ichimoko Daily and Weekly strategies.
 * These functions handle the distribution of orders across multiple take profit
 * levels based on trade mode (long-term vs short-term).
 */

#include "EasyTradeCWrapper.hpp"
#include "OrderManagement.h"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"

// Trade mode constants
#define TRADE_MODE_LONG_TERM 1              // Long-term trade mode
#define TRADE_MODE_SHORT_TERM 0             // Short-term trade mode

// Lot size distribution constants
#define LOT_SIZE_MAJORITY_FRACTION 2.0/3.0  // Majority of lots (2/3)
#define LOT_SIZE_MINORITY_FRACTION 1.0/3.0  // Minority of lots (1/3)

/**
 * @brief Splits buy orders for Ichimoko Weekly strategy.
 * 
 * This function creates buy orders based on trade mode:
 * - Mode 1 (Long-term): Single order with no specific take profit (0).
 * - Mode 0 (Short-term): Two orders - 2/3 lots with no TP, 1/3 lots with daily ATR as TP.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing daily ATR.
 * @param atr Average True Range (not directly used).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitBuyOrders_Ichimoko_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss);
	lots = roundUp(lots, pIndicators->volumeStep) * pIndicators->risk;

	// Long-term mode: single order without specific take profit
	if (pIndicators->tradeMode == TRADE_MODE_LONG_TERM)
	{
		takePrice = 0; // No specific take profit, managed by trailing stop
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		// Short-term mode: split into two orders
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots * LOT_SIZE_MAJORITY_FRACTION, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleLongEasy(takePrice, stopLoss, lots * LOT_SIZE_MINORITY_FRACTION, 0);
	}
}

/**
 * @brief Splits buy orders for Ichimoko Daily strategy.
 * 
 * This function creates buy orders based on trade mode:
 * - Mode 1 (Long-term): Single order with no specific take profit (0).
 * - Mode 0 (Short-term): Two orders - 2/3 lots with no TP, 1/3 lots with daily ATR as TP.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing daily ATR.
 * @param atr Average True Range (not directly used).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitBuyOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss);
	lots = roundUp(lots, pIndicators->volumeStep) * pIndicators->risk;

	// Long-term mode: single order without specific take profit
	if (pIndicators->tradeMode == TRADE_MODE_LONG_TERM)
	{
		takePrice = 0; // No specific take profit, managed by trailing stop
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		// Short-term mode: split into two orders
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots * LOT_SIZE_MAJORITY_FRACTION, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleLongEasy(takePrice, stopLoss, lots * LOT_SIZE_MINORITY_FRACTION, 0);
	}
}

/**
 * @brief Splits sell orders for Ichimoko Daily strategy.
 * 
 * This function creates sell orders based on trade mode:
 * - Mode 1 (Long-term): Single order with no specific take profit (0).
 * - Mode 0 (Short-term): Two orders - 2/3 lots with no TP, 1/3 lots with daily ATR as TP.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing entry price, risk, trade mode, etc.
 * @param pBase_Indicators Base indicators containing daily ATR.
 * @param atr Average True Range (not directly used).
 * @param stopLoss Stop loss distance from entry price.
 */
void splitSellOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, stopLoss) * pIndicators->risk;

	// Long-term mode: single order without specific take profit
	if (pIndicators->tradeMode == TRADE_MODE_LONG_TERM)
	{
		takePrice = 0; // No specific take profit, managed by trailing stop
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		// Short-term mode: split into two orders
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, lots * LOT_SIZE_MAJORITY_FRACTION, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleShortEasy(takePrice, stopLoss, lots * LOT_SIZE_MINORITY_FRACTION, 0);
	}
}

