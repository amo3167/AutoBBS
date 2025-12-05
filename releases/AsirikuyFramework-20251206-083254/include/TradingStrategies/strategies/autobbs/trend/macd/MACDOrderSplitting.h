#pragma once

#ifndef MACD_ORDER_SPLITTING_H_
#define MACD_ORDER_SPLITTING_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Split buy orders for MACD Daily strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value (unused, kept for compatibility)
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split sell orders for MACD Daily strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value (unused, kept for compatibility)
 * @param stopLoss Stop loss price
 */
void splitSellOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split buy orders for MACD Weekly strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value (unused, kept for compatibility)
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split sell orders for MACD Weekly strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value (unused, kept for compatibility)
 * @param stopLoss Stop loss price
 */
void splitSellOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MACD_ORDER_SPLITTING_H_ */

