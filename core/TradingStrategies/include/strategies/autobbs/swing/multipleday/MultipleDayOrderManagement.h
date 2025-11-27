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
 * Used by workoutExecutionTrend_MultipleDay strategy.
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
 * Used by workoutExecutionTrend_MultipleDay strategy.
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

/**
 * Split buy orders for multiple days swing strategy.
 * Used by splitTradeMode 22.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit value (not used, calculated from gap)
 * @param stopLoss Stop loss value
 */
void splitBuyOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Split sell orders for multiple days swing strategy.
 * Used by splitTradeMode 22.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param takePrice_primary Primary take profit value (not used, calculated from gap)
 * @param stopLoss Stop loss value
 */
void splitSellOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

/**
 * Setup GBPJPY entry signal for multiple day trading strategies.
 * 
 * Handles GBPJPY-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window
 * - Handles add position logic for long-term trends
 * - Applies GBPJPY trading filter
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param executionTrend Current execution trend (-1, 0, or 1)
 * @param oldestOpenOrderIndex Index of oldest open order
 * @param side Order side (BUY, SELL, or NONE)
 * @param isAddPosition TRUE if adding position, FALSE otherwise
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param preLow Previous bar low price
 * @param preHigh Previous bar high price
 * @param preClose Previous bar close price
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (entry signal set or filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupGBPJPYEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int executionTrend, int oldestOpenOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, BOOL* shouldSkip);

/**
 * Setup XAUUSD entry signal for multiple day trading strategies.
 * 
 * Handles XAUUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window
 * - Handles add position logic for long-term trends
 * - Sets endHour for trading window
 * - Applies commodity trading filter
 * - Sets take profit mode
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param oldestOpenOrderIndex Index of oldest open order
 * @param side Order side (BUY, SELL, or NONE)
 * @param isAddPosition TRUE if adding position, FALSE otherwise
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param shouldFilter Whether to apply ATR-based filtering
 * @param preLow Previous bar low price
 * @param preHigh Previous bar high price
 * @param preClose Previous bar close price
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param takeProfitMode Output parameter: take profit mode (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (entry signal set or filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupXAUUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int oldestOpenOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);

/**
 * Setup XAGUSD entry signal for multiple day trading strategies.
 * 
 * Handles XAGUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window (uses latestOrderIndex)
 * - Applies commodity trading filter
 * - Sets take profit mode
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param shouldFilter Whether to apply ATR-based filtering
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param takeProfitMode Output parameter: take profit mode (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupXAGUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, BOOL shouldFilter, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);

/**
 * Setup BTCUSD/ETHUSD entry signal for multiple day trading strategies.
 * 
 * Handles BTCUSD/ETHUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window (uses latestOrderIndex)
 * - Applies BTCUSD/ETHUSD trading filter
 * - Sets take profit mode
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param shouldFilter Whether to apply ATR-based filtering
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param takeProfitMode Output parameter: take profit mode (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupCryptoEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, BOOL shouldFilter, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);

/**
 * Setup GBPUSD entry signal for multiple day trading strategies.
 * 
 * Handles GBPUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window with 3-tier stop loss (uses latestOrderIndex)
 * - Applies GBPUSD trading filter
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param executionTrend Current execution trend (-1, 0, or 1)
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupGBPUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int executionTrend, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, struct tm* timeInfo1, const char* timeString, double* floatingTP, BOOL* shouldSkip);

/**
 * Setup AUDUSD entry signal for multiple day trading strategies.
 * 
 * Handles AUDUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window with 3-tier stop loss (uses latestOrderIndex)
 * - Applies GBPUSD trading filter (shared with GBPUSD)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupAUDUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, struct tm* timeInfo1, const char* timeString, double* floatingTP, BOOL* shouldSkip);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MULTIPLE_DAY_ORDER_MANAGEMENT_H_ */

