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

/* Forward declaration */
struct MultipleDaySymbolConfig;

/**
 * @brief Unified signature for all symbol-specific entry signal setup functions.
 * 
 * All symbol-specific setup functions use this signature.
 * Unused parameters can be ignored by individual implementations.
 */
typedef AsirikuyReturnCode (*MultipleDayEntrySignalFunction)(const struct MultipleDaySymbolConfig*, StrategyParams*, Indicators*, Base_Indicators*, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);

/* Forward declarations - all use the unified signature */
AsirikuyReturnCode setupGBPJPYEntrySignal_MultipleDay(const struct MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);
AsirikuyReturnCode setupXAUUSDEntrySignal_MultipleDay(const struct MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);
AsirikuyReturnCode setupXAGUSDEntrySignal_MultipleDay(const struct MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);
AsirikuyReturnCode setupCryptoEntrySignal_MultipleDay(const struct MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);
AsirikuyReturnCode setupGBPUSDEntrySignal_MultipleDay(const struct MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);
AsirikuyReturnCode setupAUDUSDEntrySignal_MultipleDay(const struct MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip);

/**
 * @brief Symbol-specific configuration structure for Multiple Day strategy.
 * 
 * This structure holds all symbol-specific parameters that control
 * the behavior of the Multiple Day strategy for different trading instruments.
 */
typedef struct MultipleDaySymbolConfig {
	/* ATR Range Calculation */
	double atrRangeDivisor;                  /* Divisor for ATR range calculation (e.g., 3.0 or 2.0) */
	double atrRangeMultiplier;               /* Multiplier for ATR range (e.g., 0.8) */
	BOOL useMaxWithParameter;                /* Use max(parameter, calculated) for ATR range */
	BOOL adjustRangeWithTrend;                /* Adjust range based on execution trend and RANGE parameter */
	
	/* Risk Management */
	double stopLossMultiplier;               /* Stop loss multiplier (e.g., 0.93 or 1.1) */
	double takePriceMultiplier;              /* Take price multiplier (e.g., 0.4 or 0.35) */
	double takePriceMinValue;                 /* Minimum take price value (e.g., 3.0, 0.3, 0.003, 0.0015) */
	BOOL useTakePriceFromStopLoss;            /* TRUE: takePrice = stopLoss * multiplier, FALSE: takePrice = atr_euro_range * multiplier */
	
	/* Order Index Selection */
	BOOL useOldestOrderIndex;                 /* TRUE: use oldestOpenOrderIndex, FALSE: use latestOrderIndex */
	
	/* Order Modification Logic */
	BOOL useTwoTierStopLoss;                  /* TRUE: 2-tier stop loss (1x, 2x), FALSE: 3-tier (1x, 2x, 3x) */
	
	/* Add Position Logic */
	BOOL supportsAddPosition;                /* TRUE: supports add position logic */
	double addPositionBuyBaseline;            /* Baseline for add position BUY (dailyPivot, dailyS1, etc.) */
	double addPositionSellBaseline;           /* Baseline for add position SELL (dailyPivot, dailyR1, etc.) */
	BOOL addPositionCheckWeeklyLevels;        /* TRUE: check weeklyR2/weeklyS2 for add position */
	
	/* Trading Window */
	int endHour;                              /* End hour for trading window (e.g., 23) */
	
	/* Take Profit Mode */
	int takeProfitMode;                        /* Take profit mode (0 or 1) */
	
	/* Floating TP */
	double floatingTPValue;                   /* Floating TP value (0 = use takePrice, or specific value) */
	
	/* Filter Function */
	/* All filter functions use the same signature: (StrategyParams*, Indicators*, Base_Indicators*, BOOL shouldFilter)
	 * Functions that don't use shouldFilter can ignore it, but the parameter must be present for type compatibility
	 */
	BOOL (*filterFunction)(StrategyParams*, Indicators*, Base_Indicators*, BOOL shouldFilter);  /* Trading filter function pointer */
	
	/* Risk Cap Configuration */
	double riskCapBuyOffset;                  /* Offset for riskCapBuy (e.g., 0 or -2) */
	double riskCapSellValue;                  /* Value for riskCapSell (e.g., 0 or riskCapBuy - 2) */
	
	/* Entry Signal Setup Function */
	/* Unified function pointer for symbol-specific entry signal setup
	 * All functions use this unified signature - unused parameters can be ignored
	 */
	MultipleDayEntrySignalFunction setupEntrySignalFunction;
} MultipleDaySymbolConfig;

/**
 * @brief Initializes symbol-specific configuration for Multiple Day strategy.
 * 
 * This function configures all symbol-specific parameters based on the trading
 * symbol. Each symbol has unique characteristics that require different ATR
 * calculations, risk management settings, and filtering logic.
 * 
 * @param pConfig Configuration structure to populate
 * @param pParams Strategy parameters containing symbol information
 * @param pBase_Indicators Base indicators (for reference)
 * @param executionTrend Current execution trend (-1, 0, or 1)
 */
void initializeMultipleDaySymbolConfig(MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Base_Indicators* pBase_Indicators, int executionTrend);

/**
 * @brief Unified function to setup entry signal for any symbol.
 * 
 * This function handles the common pattern for all symbols:
 * - Sets risk caps from config
 * - Calls the appropriate symbol-specific setup function
 * - Handles error checking and early exit
 * 
 * @param pConfig Symbol configuration
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param executionTrend Current execution trend
 * @param oldestOpenOrderIndex Index of oldest open order
 * @param latestOrderIndex Index of latest order
 * @param side Order side
 * @param isAddPosition TRUE if adding position
 * @param isSameDayOrder TRUE if order opened same day
 * @param shouldFilter Whether to apply filtering
 * @param preLow Previous bar low
 * @param preHigh Previous bar high
 * @param preClose Previous bar close
 * @param timeInfo1 Current time info
 * @param timeString Current time string
 * @param floatingTP Output: floating take profit
 * @param takeProfitMode Output: take profit mode
 * @param riskCapBuy Output: risk cap for buy orders
 * @param riskCapSell Output: risk cap for sell orders
 * @param shouldSkip Output: TRUE if caller should exit early
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupEntrySignal_MultipleDay(const MultipleDaySymbolConfig* pConfig, StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators, int executionTrend, int oldestOpenOrderIndex, int latestOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, double* riskCapBuy, double* riskCapSell, BOOL* shouldSkip);

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
 * Uses unified signature - unused parameters (latestOrderIndex, shouldFilter, takeProfitMode) are ignored.
 */

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
 * Uses unified signature - unused parameter (latestOrderIndex) is ignored.
 */

/**
 * Setup XAGUSD entry signal for multiple day trading strategies.
 * 
 * Handles XAGUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window (uses latestOrderIndex)
 * - Applies commodity trading filter
 * - Sets take profit mode
 * 
 * Uses unified signature - unused parameters (oldestOpenOrderIndex, isAddPosition, preLow, preHigh, preClose) are ignored.
 */

/**
 * Setup BTCUSD/ETHUSD entry signal for multiple day trading strategies.
 * 
 * Handles BTCUSD/ETHUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window (uses latestOrderIndex)
 * - Applies BTCUSD/ETHUSD trading filter
 * - Sets take profit mode
 * 
 * Uses unified signature - unused parameters (oldestOpenOrderIndex, isAddPosition, preLow, preHigh, preClose) are ignored.
 */

/**
 * Setup GBPUSD entry signal for multiple day trading strategies.
 * 
 * Handles GBPUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window with 3-tier stop loss (uses latestOrderIndex)
 * - Applies GBPUSD trading filter
 * 
 * Uses unified signature - unused parameters (oldestOpenOrderIndex, isAddPosition, shouldFilter, preLow, preHigh, preClose, takeProfitMode) are ignored.
 */

/**
 * Setup AUDUSD entry signal for multiple day trading strategies.
 * 
 * Handles AUDUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window with 3-tier stop loss (uses latestOrderIndex)
 * - Applies GBPUSD trading filter (shared with GBPUSD)
 * 
 * Uses unified signature - unused parameters (oldestOpenOrderIndex, isAddPosition, shouldFilter, preLow, preHigh, preClose, takeProfitMode) are ignored.
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MULTIPLE_DAY_ORDER_MANAGEMENT_H_ */

