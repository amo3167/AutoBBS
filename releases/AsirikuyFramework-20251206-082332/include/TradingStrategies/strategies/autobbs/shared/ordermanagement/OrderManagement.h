#pragma once

#ifndef ORDER_MANAGEMENT_H_
#define ORDER_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Order Management Module
 * 
 * Provides order management functions for strategy execution.
 * Handles trade entries, exits, and order modifications.
 */

/**
 * Modify existing orders (stop loss, take profit).
 * Updates stop loss and take profit for existing orders based on new day conditions.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param orderType Order type (BUY or SELL)
 * @param stopLoss New stop loss value
 * @param takePrice New take profit value (-1 to keep existing)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double stopLoss, double takePrice);

/**
 * Handle trade entries.
 * Processes entry signals and opens new orders or modifies existing ones.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Handle trade exits.
 * Processes exit signals and closes orders accordingly.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, Indicators* pIndicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ORDER_MANAGEMENT_H_ */

