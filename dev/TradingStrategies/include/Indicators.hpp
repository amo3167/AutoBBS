// Indicators.hpp
// Wrapper class for indicator calculations

#ifndef INDICATORS_HPP
#define INDICATORS_HPP

#include "StrategyContext.hpp"

namespace trading {

/// Moving Average signal types for MACD
enum MACDSignal {
    MACD_MAIN = 0,     ///< Main MACD line
    MACD_SIGNAL = 1,   ///< Signal line
    MACD_HISTOGRAM = 2 ///< Histogram
};

/// Bollinger Bands buffer types
enum BBandsBuffer {
    BB_LOWER = 0,  ///< Lower band
    BB_MIDDLE = 1, ///< Middle band (SMA)
    BB_UPPER = 2   ///< Upper band
};

/// Stochastic signal types
enum StochasticSignal {
    STOCH_MAIN = 0,   ///< Main line (%K)
    STOCH_SIGNAL = 1  ///< Signal line (%D)
};

/// Moving Average price types
enum MAPrice {
    MA_OPEN = 0,   ///< Open price
    MA_HIGH = 1,   ///< High price
    MA_LOW = 2,    ///< Low price
    MA_CLOSE = 3   ///< Close price
};

/// Wrapper for technical indicator calculations
/// Wraps EasyTrade C functions and provides clean C++ interface
/// All indicator methods use the rate arrays from StrategyContext
class Indicators {
public:
    /// Constructor
    /// @param context StrategyContext containing rate arrays
    explicit Indicators(const StrategyContext& context);
    
    ~Indicators();
    
    /// Calculate ATR (Average True Range)
    /// @param ratesIndex Rate array index (e.g., B_PRIMARY_RATES, B_DAILY_RATES)
    /// @param period ATR period
    /// @param shift Bar shift (0 = current bar)
    /// @return ATR value
    double getATR(int ratesIndex, int period, int shift) const;
    
    /// Calculate RSI (Relative Strength Index)
    /// @param ratesIndex Rate array index
    /// @param period RSI period
    /// @param shift Bar shift
    /// @return RSI value (0-100)
    double getRSI(int ratesIndex, int period, int shift) const;
    
    /// Calculate Moving Average
    /// @param priceType Price type to use (OPEN, HIGH, LOW, CLOSE)
    /// @param ratesIndex Rate array index
    /// @param period MA period
    /// @param shift Bar shift
    /// @return MA value
    double getMA(MAPrice priceType, int ratesIndex, int period, int shift) const;
    
    /// Calculate MACD (Moving Average Convergence Divergence)
    /// @param ratesIndex Rate array index
    /// @param fastPeriod Fast MA period
    /// @param slowPeriod Slow MA period
    /// @param signalPeriod Signal MA period
    /// @param signal Which buffer to return (MAIN, SIGNAL, HISTOGRAM)
    /// @param shift Bar shift
    /// @return MACD value for selected buffer
    double getMACD(int ratesIndex, int fastPeriod, int slowPeriod, 
                   int signalPeriod, MACDSignal signal, int shift) const;
    
    /// Calculate all MACD values at once
    /// @param ratesIndex Rate array index
    /// @param fastPeriod Fast MA period
    /// @param slowPeriod Slow MA period
    /// @param signalPeriod Signal MA period
    /// @param shift Bar shift
    /// @param[out] macd MACD main line
    /// @param[out] signal Signal line
    /// @param[out] histogram Histogram
    /// @return MACD main line value
    double getMACDAll(int ratesIndex, int fastPeriod, int slowPeriod, 
                      int signalPeriod, int shift, 
                      double* macd, double* signal, double* histogram) const;
    
    /// Calculate Bollinger Bands
    /// @param ratesIndex Rate array index
    /// @param period BB period
    /// @param deviation Standard deviation multiplier
    /// @param buffer Which band to return (LOWER, MIDDLE, UPPER)
    /// @param shift Bar shift
    /// @return Band value
    double getBollingerBands(int ratesIndex, int period, double deviation, 
                             BBandsBuffer buffer, int shift) const;
    
    /// Calculate Bollinger Bands stop level
    /// @param ratesIndex Rate array index
    /// @param period BB period
    /// @param deviation Standard deviation multiplier
    /// @param[out] trend Trend direction (-1, 0, 1)
    /// @param[out] stopPrice Stop price level
    /// @param[out] index Bar index of trend start
    /// @return Band stop value
    double getBBandStop(int ratesIndex, int period, double deviation,
                        int* trend, double* stopPrice, int* index) const;
    
    /// Calculate Stochastic Oscillator
    /// @param ratesIndex Rate array index
    /// @param period K period
    /// @param k K value
    /// @param d D value
    /// @param signal Which line to return (MAIN or SIGNAL)
    /// @param shift Bar shift
    /// @return Stochastic value (0-100)
    double getStochastic(int ratesIndex, int period, int k, int d, 
                         StochasticSignal signal, int shift) const;
    
    /// Calculate Standard Deviation
    /// @param ratesIndex Rate array index
    /// @param priceType Price type to use
    /// @param period Period
    /// @param shift Bar shift
    /// @return Standard deviation value
    double getStdDev(int ratesIndex, MAPrice priceType, int period, int shift) const;
    
    /// Calculate CCI (Commodity Channel Index)
    /// @param ratesIndex Rate array index
    /// @param period CCI period
    /// @param shift Bar shift
    /// @return CCI value
    double getCCI(int ratesIndex, int period, int shift) const;
    
    /// Get High price
    /// @param ratesIndex Rate array index
    /// @param shift Bar shift
    /// @return High price
    double getHigh(int ratesIndex, int shift) const;
    
    /// Get Low price
    /// @param ratesIndex Rate array index
    /// @param shift Bar shift
    /// @return Low price
    double getLow(int ratesIndex, int shift) const;
    
    /// Get Open price
    /// @param ratesIndex Rate array index
    /// @param shift Bar shift
    /// @return Open price
    double getOpen(int ratesIndex, int shift) const;
    
    /// Get Close price
    /// @param ratesIndex Rate array index
    /// @param shift Bar shift
    /// @return Close price
    double getClose(int ratesIndex, int shift) const;
    
private:
    const StrategyContext& context_;
    
    // Non-copyable (C++03 idiom)
    Indicators(const Indicators&);
    Indicators& operator=(const Indicators&);
};

} // namespace trading

#endif // INDICATORS_HPP
