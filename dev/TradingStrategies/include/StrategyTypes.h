/**
 * @file StrategyTypes.h
 * @brief Shared type definitions for C/C++ interoperability
 * @details Contains enum definitions and constants needed by both
 *          C strategy implementations and C++ wrapper code.
 * 
 * @author Phase 1 Migration Team
 * @date November 2025
 * @version 1.0.0
 */

#ifndef STRATEGY_TYPES_H_
#define STRATEGY_TYPES_H_
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Strategy identifier enum
 * 
 * @details Maps to INTERNAL_STRATEGY_ID setting value.
 *          These IDs must match the values used in MQL code.
 */
typedef enum strategyId_t
{
  RECORD_BARS       = 19,  ///< Record bars strategy (data collection)
  TAKEOVER          = 26,  ///< Takeover strategy
  SCREENING         = 27,  ///< Screening strategy (analysis only)
  AUTOBBS           = 29,  ///< AutoBBS daily dispatcher
  AUTOBBSWEEKLY     = 30,  ///< AutoBBS weekly dispatcher
  TRENDLIMIT        = 31   ///< Trend limit strategy
} StrategyId;

/**
 * @brief Count of base rates indexes
 * @details Used for array bounds checking in getRates()
 */
#define BASE_RATES_INDEXES_COUNT 7

/**
 * @brief Error code enum for strategies
 * @details Used by IStrategy::execute() return values
 */
typedef enum strategyErrorCode_t
{
  STRATEGY_SUCCESS                = 0,   ///< Strategy executed successfully
  STRATEGY_INVALID_PARAMETERS     = 1,   ///< Invalid input parameters
  STRATEGY_FAILED_TO_LOAD_INDICATORS = 2, ///< Indicator loading failed
  STRATEGY_INTERNAL_ERROR         = 3,   ///< Internal strategy error
  STRATEGY_NO_TRADING_SIGNAL      = 4    ///< No signal generated (not an error)
} StrategyErrorCode;

/**
 * @brief Position splitting mode enumeration
 * 
 * @details Defines how a strategy splits position entries across multiple
 *          orders with different take-profit targets or sizing logic.
 *          These values match empirical usage patterns found in TrendStrategy.c
 *          and related complex multi-timeframe strategies.
 * 
 *          Usage: Set pIndicators->splitTradeMode to one of these values
 *          before order entry functions process the signal.
 * 
 * @note Values are non-contiguous to preserve legacy numeric constants.
 */
typedef enum splitTradeMode_t
{
  STM_DEFAULT         = 0,   ///< Single order, no splitting
  STM_BREAKOUT        = 3,   ///< BBS breakout: partial at resistance, remainder trailing
  STM_LIMIT           = 4,   ///< Limit/range-bound: multiple entries at S/R levels
  STM_PIVOT           = 5,   ///< Pivot-based: tiered entries at daily pivot levels
  STM_RETREAT         = 6,   ///< Middle retreat phase: pullback entry layering
  STM_KEYK            = 7,   ///< KeyK reversal pattern: 1/3 splits at TP multiples
  STM_SHELLINGTON     = 9,   ///< Shellington indicator-based split (trend confirmation)
  STM_SHORTTERM       = 10,  ///< Short-term momentum: 2-part split (gap close + ATR)
  STM_WEEKLY_BEGIN    = 11,  ///< Weekly beginning phase: single entry at R1/S1
  STM_ATR_4H          = 19,  ///< 4H ATR-based swing: daily gap + ATR TP targets
  STM_ATR_4H_ALT      = 20,  ///< 4H ATR alternative: 40% weekly ATR threshold entry
  STM_MACD_SHORT      = 31   ///< MACD short-term: break-even ladder on advancement
} SplitTradeMode;

/**
 * @brief Take-profit target mode enumeration
 * 
 * @details Specifies how the strategy calculates take-profit levels.
 *          Works in conjunction with splitTradeMode to determine final
 *          order parameters.
 * 
 *          Usage: Set pIndicators->tpMode to control TP calculation schema
 *          within order management functions.
 * 
 * @note Values match observed usage in workoutExecutionTrend_* functions.
 */
typedef enum takeProfitMode_t
{
  TP_STATIC         = 0,   ///< Fixed TP set manually or via R:R ratio
  TP_ATR_RANGE      = 3,   ///< TP based on ATR multiples (dynamic volatility scaling)
  TP_LADDER_BE      = 4    ///< Ladder: partial close at TP, move remainder to break-even
} TakeProfitMode;

#ifdef __cplusplus
}
#endif

#endif // STRATEGY_TYPES_H_
