#pragma once

#ifndef RANGE_ORDER_MANAGEMENT_H_
#define RANGE_ORDER_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Checks if a buy range order should be entered.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param orderIndex Index of the order (-1 if no order)
 * @param stopHour Hour to stop entering orders
 * @param isOrderSignal Whether to save order signal
 * @param isEnterOrder Whether to enter the order
 * @return TRUE if order should be entered, FALSE otherwise
 */
BOOL entryBuyRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder);

/**
 * Checks if a sell range order should be entered.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param orderIndex Index of the order (-1 if no order)
 * @param stopHour Hour to stop entering orders
 * @param isOrderSignal Whether to save order signal
 * @param isEnterOrder Whether to enter the order
 * @return TRUE if order should be entered, FALSE otherwise
 */
BOOL entrySellRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder);

/**
 * Checks if an order is a range order.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param orderIndex Index of the order to check
 * @return 1 for buy range order, -1 for sell range order, 2 for pending buy, -2 for pending sell, 0 otherwise
 */
int isRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RANGE_ORDER_MANAGEMENT_H_ */

