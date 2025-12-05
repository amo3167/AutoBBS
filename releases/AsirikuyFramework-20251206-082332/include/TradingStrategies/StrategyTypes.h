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

#ifdef __cplusplus
}
#endif

#endif // STRATEGY_TYPES_H_
