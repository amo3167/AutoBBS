#pragma once

#ifndef DAY_TRADING_ORDER_SPLITTING_H_
#define DAY_TRADING_ORDER_SPLITTING_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Split buy orders for daily swing strategy.
 * Used by splitTradeMode 16.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price (unused)
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for daily swing strategy.
 * Used by splitTradeMode 16.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price (unused)
 * @param stopLoss Stop loss price
 */
void splitSellOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split buy orders for daily swing execution only strategy.
 * Used by splitTradeMode 18.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price (unused)
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_Daily_Swing_ExecutionOnly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for daily swing execution only strategy.
 * Used by splitTradeMode 18.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price (unused)
 * @param stopLoss Stop loss price
 */
void splitSellOrders_Daily_Swing_ExecutionOnly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DAY_TRADING_ORDER_SPLITTING_H_ */

