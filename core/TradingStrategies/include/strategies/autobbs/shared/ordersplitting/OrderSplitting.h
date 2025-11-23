#pragma once

#ifndef ORDER_SPLITTING_H_
#define ORDER_SPLITTING_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Order Splitting Module
 * 
 * Provides functions for splitting single trade signals into multiple orders.
 * Handles various order splitting strategies based on trade mode and market conditions.
 */

/**
 * Split buy orders based on split trade mode.
 * Routes to appropriate buy order splitting function.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitBuyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders based on split trade mode.
 * Routes to appropriate sell order splitting function.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit price
 * @param stopLoss Stop loss price
 */
void splitSellOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Filter execution timeframe based on time.
 * Determines which timeframe to use for execution based on current time.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return Shift index or SUCCESS
 */
int filterExcutionTF_ByTime(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Filter execution timeframe.
 * Determines which timeframe to use for execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return Shift index or SUCCESS
 */
int filterExcutionTF(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ORDER_SPLITTING_H_ */

