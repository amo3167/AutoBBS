#pragma once

#ifndef SHORTTERM_ORDER_SPLITTING_H_
#define SHORTTERM_ORDER_SPLITTING_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Split buy orders for ShortTerm strategy (New version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for ShortTerm strategy (New version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitSellOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split buy orders for ShortTerm strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitBuyOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for ShortTerm strategy.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitSellOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SHORTTERM_ORDER_SPLITTING_H_ */

