#pragma once

#ifndef ORDER_SPLITTING_UTILITIES_H_
#define ORDER_SPLITTING_UTILITIES_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Split buy orders using ATR-based take profit levels.
 * Used by splitTradeMode 12.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value for calculating take profit
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split sell orders using ATR-based take profit levels.
 * Used by splitTradeMode 12.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value for calculating take profit
 * @param stopLoss Stop loss price
 */
void splitSellOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split buy orders for long-term trading.
 * Used by splitTradeMode 3.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price (unused)
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for long-term trading.
 * Used by splitTradeMode 3.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price (unused)
 * @param stopLoss Stop loss price
 */
void splitSellOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split buy range orders.
 * Used by RangeOrderManagement.c and LimitStrategy.c.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 */
void splitBuyRangeOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Split sell range orders.
 * Used by RangeOrderManagement.c and LimitStrategy.c.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 */
void splitSellRangeOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ORDER_SPLITTING_UTILITIES_H_ */

