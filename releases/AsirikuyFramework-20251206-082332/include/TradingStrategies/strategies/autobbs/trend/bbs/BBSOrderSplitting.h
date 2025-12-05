#pragma once

#ifndef BBS_ORDER_SPLITTING_H_
#define BBS_ORDER_SPLITTING_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Split buy orders for 4H BBS Swing strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split sell orders for 4H BBS Swing strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value
 * @param stopLoss Stop loss price
 */
void splitSellOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split buy orders for 4H BBS Swing strategy (100P version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split sell orders for 4H BBS Swing strategy (100P version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value
 * @param stopLoss Stop loss price
 */
void splitSellOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split buy orders for 4H BBS Swing strategy (Shellington version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

/**
 * Split sell orders for 4H BBS Swing strategy (Shellington version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param atr ATR value
 * @param stopLoss Stop loss price
 */
void splitSellOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BBS_ORDER_SPLITTING_H_ */

