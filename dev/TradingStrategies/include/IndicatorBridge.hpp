/**
 * @file IndicatorBridge.hpp
 * @brief C++ wrappers for C indicator functions
 * @details Provides type-safe C++ wrappers around EasyTrade C indicator functions,
 *          enabling C++ strategies to call legacy C indicators during migration.
 * 
 * @author Phase 3 Migration Team
 * @date November 2025
 * @version 1.0.0
 */

#ifndef TRADING_INDICATOR_BRIDGE_HPP_
#define TRADING_INDICATOR_BRIDGE_HPP_
#pragma once

#include "BridgeConstants.hpp"

// Forward declare C functions we'll wrap
extern "C" {
    // Price access functions
    double iHigh(int rateIndex, int shift);
    double iLow(int rateIndex, int shift);
    double iClose(int rateIndex, int shift);
    
    // Indicator calculation functions
    double iAtr(int ratesArrayIndex, int period, int shift);
    double iMA(int type, int ratesArrayIndex, int period, int shift);
    double iBBandStop(int ratesArrayIndex, int bb_period, double bb_deviation, 
                      int* signal, double* stopPrice, int* index);
}

namespace trading {

/**
 * @brief C++ wrapper functions for C indicator library
 * @details These functions provide a clean C++ interface to the legacy C indicator
 *          functions from EasyTradeCWrapper. They handle type conversions and
 *          provide better error handling where appropriate.
 */
namespace indicators {

/**
 * @brief Get high price at specified bar
 * @param rateIndex Rates array index (e.g., DAILY_RATES, PRIMARY_RATES)
 * @param shift Bar shift from current (0 = current bar, 1 = previous, etc.)
 * @return High price at specified bar
 * @throws May return error values from underlying C function
 */
inline double getHigh(int rateIndex, int shift) {
    return ::iHigh(rateIndex, shift);
}

/**
 * @brief Get low price at specified bar
 * @param rateIndex Rates array index (e.g., DAILY_RATES, PRIMARY_RATES)
 * @param shift Bar shift from current (0 = current bar, 1 = previous, etc.)
 * @return Low price at specified bar
 * @throws May return error values from underlying C function
 */
inline double getLow(int rateIndex, int shift) {
    return ::iLow(rateIndex, shift);
}

/**
 * @brief Get close price at specified bar
 * @param rateIndex Rates array index (e.g., DAILY_RATES, PRIMARY_RATES)
 * @param shift Bar shift from current (0 = current bar, 1 = previous, etc.)
 * @return Close price at specified bar
 * @throws May return error values from underlying C function
 */
inline double getClose(int rateIndex, int shift) {
    return ::iClose(rateIndex, shift);
}

/**
 * @brief Calculate Average True Range indicator
 * @param ratesArrayIndex Rates array index (e.g., DAILY_RATES, PRIMARY_RATES)
 * @param period ATR period (typical: 14)
 * @param shift Bar shift from current (0 = current bar, 1 = previous, etc.)
 * @return ATR value at specified bar
 * @throws May return error values from underlying C function
 */
inline double getATR(int ratesArrayIndex, int period, int shift) {
    return ::iAtr(ratesArrayIndex, period, shift);
}

/**
 * @brief Calculate Moving Average indicator
 * @param priceType Price type to use (0=close, 1=open, 2=high, 3=low, 4=median, etc.)
 * @param ratesArrayIndex Rates array index (e.g., DAILY_RATES, PRIMARY_RATES)
 * @param period MA period
 * @param shift Bar shift from current (0 = current bar, 1 = previous, etc.)
 * @return MA value at specified bar
 * @throws May return error values from underlying C function
 * 
 * @note Price type constants:
 *       - 0 = Close price
 *       - 1 = Open price
 *       - 2 = High price
 *       - 3 = Low price
 */
inline double getMA(int priceType, int ratesArrayIndex, int period, int shift) {
    return ::iMA(priceType, ratesArrayIndex, period, shift);
}

/**
 * @brief Calculate Bollinger Bands Stop indicator
 * @details Calculates trend direction and stop price levels based on Bollinger Bands.
 *          The stop price acts as a trailing stop that adjusts with price movement.
 * 
 * @param ratesArrayIndex Rates array index (e.g., DAILY_RATES, PRIMARY_RATES)
 * @param bb_period Bollinger Bands period (typical: 20)
 * @param bb_deviation Standard deviation multiplier (typical: 2.0)
 * @param[out] trend Output: Trend direction (-1 = down, 0 = neutral, 1 = up)
 * @param[out] stopPrice Output: Stop price level based on BB
 * @param[out] index Output: Array index where current trend started
 * @return BB stop value (typically 0)
 * @throws May return error values from underlying C function
 * 
 * @note The trend and stopPrice parameters are modified by this function
 */
inline double getBBandStop(int ratesArrayIndex, int bb_period, double bb_deviation,
                          int* trend, double* stopPrice, int* index) {
    return ::iBBandStop(ratesArrayIndex, bb_period, bb_deviation, 
                       trend, stopPrice, index);
}

} // namespace indicators
} // namespace trading

#endif // TRADING_INDICATOR_BRIDGE_HPP_
