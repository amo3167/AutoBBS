#pragma once

#ifndef WEEKLY_ORDER_SPLITTING_H_
#define WEEKLY_ORDER_SPLITTING_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Split buy orders for Weekly Beginning strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for Weekly Beginning strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitSellOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split buy orders for Weekly ShortTerm strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for Weekly ShortTerm strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitSellOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split buy orders for Weekly Trading strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for Weekly Trading strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitSellOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WEEKLY_ORDER_SPLITTING_H_ */

