/*
 * Stop Loss Management Module
 * 
 * Provides functions for managing stop loss levels for trading orders.
 * This module handles dynamic stop loss adjustments based on take profit levels
 * and current market conditions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/common/StopLossManagement.h"

/**
 * @brief Moves stop loss level for an existing order based on take profit ratio.
 * 
 * This function adjusts the stop loss level for an open order based on the ratio
 * between the current take profit and the desired stop loss level. It calculates
 * a new stop loss that maintains the risk/reward ratio relative to the take profit.
 * 
 * Algorithm:
 * 1. Calculate the current take profit distance from entry price.
 * 2. For BUY orders: Set new stop loss = entry price - (take profit * stopLossLevel ratio).
 * 3. For SELL orders: Set new stop loss = entry price + (take profit * stopLossLevel ratio).
 * 4. Set executionTrend if the new stop loss differs significantly from current stop loss.
 * 
 * @param pParams Strategy parameters containing order information.
 * @param pIndicators Strategy indicators to update with new stop loss price.
 * @param pBase_Indicators Base indicators (not directly used).
 * @param orderIndex Index of the order to modify.
 * @param stopLossLevel Ratio factor for calculating new stop loss (typically 0.5-1.0).
 * @return TRUE on success.
 */
BOOL move_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, double stopLossLevel)
{
	double realTakePrice = fabs(pParams->orderInfo[orderIndex].takeProfit - pParams->orderInfo[orderIndex].openPrice);

	pIndicators->stopMovingBackSL = FALSE;
	
	// Handle BUY orders
	if (pParams->orderInfo[orderIndex].type == BUY)
	{
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		// Calculate new stop loss: entry price minus (take profit * stopLossLevel)
		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;
		pIndicators->takePrice = -1;

		// Set execution trend if stop loss change is significant
		if (fabs(pIndicators->stopLossPrice - pParams->orderInfo[orderIndex].stopLoss) > pIndicators->adjust)
			pIndicators->executionTrend = 1;
	}

	// Handle SELL orders
	if (pParams->orderInfo[orderIndex].type == SELL)
	{
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		// Calculate new stop loss: entry price plus (take profit * stopLossLevel)
		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice * stopLossLevel;
		pIndicators->takePrice = -1;

		// Set execution trend if stop loss change is significant
		if (fabs(pIndicators->stopLossPrice - pParams->orderInfo[orderIndex].stopLoss) > pIndicators->adjust)
			pIndicators->executionTrend = -1;
	}
	
	return TRUE;
}
