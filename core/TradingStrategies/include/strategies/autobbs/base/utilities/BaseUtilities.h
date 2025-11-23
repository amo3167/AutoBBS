#pragma once

#ifndef BASE_UTILITIES_H_
#define BASE_UTILITIES_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Base Utilities Module
 * 
 * Provides utility functions for Base strategy:
 * - Order modification
 * - Limit order management
 * - Day detection
 */

AsirikuyReturnCode base_ModifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice);
void closeAllLimitPreviousDayOrders(StrategyParams* pParams);
BOOL isNewDay(StrategyParams* pParams, time_t currentTime);

#ifdef __cplusplus
}
#endif

#endif /* BASE_UTILITIES_H_ */

