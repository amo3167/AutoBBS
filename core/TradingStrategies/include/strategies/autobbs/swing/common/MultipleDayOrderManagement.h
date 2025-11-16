#pragma once

#ifndef MULTIPLE_DAY_ORDER_MANAGEMENT_H_
#define MULTIPLE_DAY_ORDER_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enter order for multiple day trading strategies.
 * Used by GBPJPY_MultipleDay and MultipleDay_V2 strategies.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param riskCapBuy Risk cap for buy orders
 * @param riskCapSell Risk cap for sell orders
 * @param isSameDayClosedOrder Whether this is a same-day closed order
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode enterOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double riskCapBuy, double riskCapSell, BOOL isSameDayClosedOrder);

/**
 * Modify order for multiple day trading strategies.
 * Used by GBPJPY_MultipleDay and MultipleDay_V2 strategies.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param latestOrderIndex Index of the latest order to modify
 * @param openOrderHigh High price of the open order
 * @param openOrderLow Low price of the open order
 * @param floatingTP Floating take profit level
 * @param takeProfitMode Take profit mode (0 or other)
 * @param isLongTerm Whether this is a long-term order
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode modifyOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, double openOrderHigh, double openOrderLow, double floatingTP, int takeProfitMode, BOOL isLongTerm);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MULTIPLE_DAY_ORDER_MANAGEMENT_H_ */

