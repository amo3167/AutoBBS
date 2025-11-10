/**
 * @file BridgeConstants.hpp
 * @brief Constants and enums for C/C++ bridge layer
 * @details Provides constants needed by C++ strategies to interface with C indicator functions
 *          and maintain compatibility during migration.
 * 
 * @author Phase 3 Migration Team
 * @date November 2025
 * @version 1.0.0
 */

#ifndef TRADING_BRIDGE_CONSTANTS_HPP_
#define TRADING_BRIDGE_CONSTANTS_HPP_
#pragma once

#include "AsirikuyDefines.h"

namespace trading {

// ============================================================================
// Rates Array Indexes
// ============================================================================

/**
 * @brief Rates array indexes for accessing different timeframes
 * @details These map to the BaseRatesIndexes enum but provide C++ namespace
 */
enum RatesIndex {
    DAILY_RATES = 0,        ///< Daily timeframe
    WEEKLY_RATES = 1,       ///< Weekly timeframe
    HOURLY_RATES = 2,       ///< Hourly timeframe (H1)
    PRIMARY_RATES = 3,      ///< Primary trading timeframe (typically M15 or M30)
    FOUR_HOUR_RATES = 4,    ///< 4-hour timeframe
    MONTHLY_RATES = 5       ///< Monthly timeframe
};

// ============================================================================
// Strategy Error Codes
// ============================================================================

/**
 * @brief Additional error codes specific to strategy execution
 * @details These extend the base AsirikuyReturnCode enum for strategy-specific errors
 */
enum StrategyErrorCode {
    STRATEGY_FAILED_TO_MODIFY_ORDERS = 101   ///< Failed to modify existing orders
};

// ============================================================================
// Pantheios Logging Constants
// ============================================================================

// Note: Pantheios logging constants are defined in pantheios/pantheios.h
// No need to redefine them here - they will be available when pantheios is included

// ============================================================================
// Order Type Constants (for compatibility)
// ============================================================================

// Note: OrderType enum already defined in AsirikuyDefines.h with values:
// BUY=0, SELL=1, BUYLIMIT=2, SELLLIMIT=3, BUYSTOP=4, SELLSTOP=5

// ============================================================================
// Indicator Calculation Modes
// ============================================================================

/**
 * @brief Moving average calculation modes
 */
enum MAMode {
    MA_MODE_SMA = 0,    ///< Simple Moving Average
    MA_MODE_EMA = 1,    ///< Exponential Moving Average
    MA_MODE_SMMA = 2,   ///< Smoothed Moving Average
    MA_MODE_LWMA = 3    ///< Linear Weighted Moving Average
};

/**
 * @brief Price calculation modes for indicators
 */
enum PriceMode {
    PRICE_CLOSE = 0,     ///< Close price
    PRICE_OPEN = 1,      ///< Open price
    PRICE_HIGH = 2,      ///< High price
    PRICE_LOW = 3,       ///< Low price
    PRICE_MEDIAN = 4,    ///< Median price (High+Low)/2
    PRICE_TYPICAL = 5,   ///< Typical price (High+Low+Close)/3
    PRICE_WEIGHTED = 6   ///< Weighted price (High+Low+Close+Close)/4
};

} // namespace trading

// ============================================================================
// Global Constants (for compatibility with C code)
// ============================================================================

// Import rates indexes into global namespace
using trading::DAILY_RATES;
using trading::WEEKLY_RATES;
using trading::HOURLY_RATES;
using trading::PRIMARY_RATES;
using trading::FOUR_HOUR_RATES;
using trading::MONTHLY_RATES;

#endif // TRADING_BRIDGE_CONSTANTS_HPP_
