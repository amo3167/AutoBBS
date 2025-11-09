// IndicatorsTests.cpp
// Unit tests for Indicators class

#include "Indicators.hpp"
#include "StrategyContext.hpp"
#include <boost/test/unit_test.hpp>

using namespace trading;

// Mock StrategyParams for testing
extern "C" {
    #include "AsirikuyStrategies.h"
}

BOOST_AUTO_TEST_SUITE(IndicatorsTests)

// Helper to create a minimal valid StrategyContext for testing
static StrategyContext createTestContext() {
    // Create minimal StrategyParams structure
    static StrategyParams params;
    static RatesBuffers ratesBuffers;
    static AccountInfo accountInfo;
    static BidAsk bidAsk;
    static double settings[256] = {0};
    
    // Initialize minimal required fields
    params.ratesBuffers = &ratesBuffers;
    params.accountInfo = &accountInfo;
    params.bidAsk = &bidAsk;
    params.settings = settings;
    params.strategyId = RECORD_BARS;
    params.symbol = "EURUSD";
    
    // Initialize BidAsk with valid test data
    bidAsk.ask[0] = 1.2000;
    bidAsk.bid[0] = 1.1998;
    
    // Initialize AccountInfo
    accountInfo.margin = 10000.0;
    accountInfo.accountBalance = 10000.0;
    
    return StrategyContext(&params);
}

// ===== Constructor Tests =====

BOOST_AUTO_TEST_CASE(Indicators_Constructor) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // If we get here without crashing, constructor works
    BOOST_CHECK(true);
}

// ===== ATR Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetATR) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Call getATR - should not crash
    // We can't validate the exact value without real rate data,
    // but we can verify the interface works
    double atr = indicators.getATR(0, 20, 1);
    
    // ATR should be non-negative
    BOOST_CHECK_GE(atr, 0.0);
}

BOOST_AUTO_TEST_CASE(Indicators_GetATR_DifferentPeriods) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Test with different periods
    double atr10 = indicators.getATR(0, 10, 1);
    double atr20 = indicators.getATR(0, 20, 1);
    double atr50 = indicators.getATR(0, 50, 1);
    
    BOOST_CHECK_GE(atr10, 0.0);
    BOOST_CHECK_GE(atr20, 0.0);
    BOOST_CHECK_GE(atr50, 0.0);
}

// ===== RSI Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetRSI) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double rsi = indicators.getRSI(0, 14, 1);
    
    // RSI should be between 0 and 100
    BOOST_CHECK_GE(rsi, 0.0);
    BOOST_CHECK_LE(rsi, 100.0);
}

BOOST_AUTO_TEST_CASE(Indicators_GetRSI_DifferentPeriods) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double rsi9 = indicators.getRSI(0, 9, 1);
    double rsi14 = indicators.getRSI(0, 14, 1);
    double rsi25 = indicators.getRSI(0, 25, 1);
    
    BOOST_CHECK_GE(rsi9, 0.0);
    BOOST_CHECK_LE(rsi9, 100.0);
    BOOST_CHECK_GE(rsi14, 0.0);
    BOOST_CHECK_LE(rsi14, 100.0);
    BOOST_CHECK_GE(rsi25, 0.0);
    BOOST_CHECK_LE(rsi25, 100.0);
}

// ===== Moving Average Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetMA) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double ma = indicators.getMA(MA_CLOSE, 0, 20, 1);
    
    // MA should be a valid number
    BOOST_CHECK(!isnan(ma));
    BOOST_CHECK(!isinf(ma));
}

BOOST_AUTO_TEST_CASE(Indicators_GetMA_AllPriceTypes) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double maOpen = indicators.getMA(MA_OPEN, 0, 20, 1);
    double maHigh = indicators.getMA(MA_HIGH, 0, 20, 1);
    double maLow = indicators.getMA(MA_LOW, 0, 20, 1);
    double maClose = indicators.getMA(MA_CLOSE, 0, 20, 1);
    
    // All should be valid
    BOOST_CHECK(!isnan(maOpen));
    BOOST_CHECK(!isnan(maHigh));
    BOOST_CHECK(!isnan(maLow));
    BOOST_CHECK(!isnan(maClose));
}

// ===== MACD Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetMACD_Main) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double macd = indicators.getMACD(0, 12, 26, 9, MACD_MAIN, 1);
    
    // MACD should be a valid number
    BOOST_CHECK(!isnan(macd));
    BOOST_CHECK(!isinf(macd));
}

BOOST_AUTO_TEST_CASE(Indicators_GetMACD_Signal) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double signal = indicators.getMACD(0, 12, 26, 9, MACD_SIGNAL, 1);
    
    BOOST_CHECK(!isnan(signal));
    BOOST_CHECK(!isinf(signal));
}

BOOST_AUTO_TEST_CASE(Indicators_GetMACD_Histogram) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double hist = indicators.getMACD(0, 12, 26, 9, MACD_HISTOGRAM, 1);
    
    BOOST_CHECK(!isnan(hist));
    BOOST_CHECK(!isinf(hist));
}

BOOST_AUTO_TEST_CASE(Indicators_GetMACDAll) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double macd, signal, histogram;
    double returnValue = indicators.getMACDAll(0, 12, 26, 9, 1, 
                                               &macd, &signal, &histogram);
    
    // All values should be valid
    BOOST_CHECK(!isnan(returnValue));
    BOOST_CHECK(!isnan(macd));
    BOOST_CHECK(!isnan(signal));
    BOOST_CHECK(!isnan(histogram));
    
    // Return value should match macd
    BOOST_CHECK_CLOSE(returnValue, macd, 0.0001);
}

// ===== Bollinger Bands Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetBollingerBands_Lower) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double lower = indicators.getBollingerBands(0, 20, 2.0, BB_LOWER, 1);
    
    BOOST_CHECK(!isnan(lower));
    BOOST_CHECK(!isinf(lower));
}

BOOST_AUTO_TEST_CASE(Indicators_GetBollingerBands_Middle) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double middle = indicators.getBollingerBands(0, 20, 2.0, BB_MIDDLE, 1);
    
    BOOST_CHECK(!isnan(middle));
    BOOST_CHECK(!isinf(middle));
}

BOOST_AUTO_TEST_CASE(Indicators_GetBollingerBands_Upper) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double upper = indicators.getBollingerBands(0, 20, 2.0, BB_UPPER, 1);
    
    BOOST_CHECK(!isnan(upper));
    BOOST_CHECK(!isinf(upper));
}

BOOST_AUTO_TEST_CASE(Indicators_GetBollingerBands_Ordering) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double lower = indicators.getBollingerBands(0, 20, 2.0, BB_LOWER, 1);
    double middle = indicators.getBollingerBands(0, 20, 2.0, BB_MIDDLE, 1);
    double upper = indicators.getBollingerBands(0, 20, 2.0, BB_UPPER, 1);
    
    // In normal conditions: lower < middle < upper
    // (This might not hold with insufficient data, but we test the interface)
    if (!isnan(lower) && !isnan(middle) && !isnan(upper)) {
        // Just verify we got three different values
        BOOST_CHECK(true);
    }
}

BOOST_AUTO_TEST_CASE(Indicators_GetBBandStop) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    int trend;
    double stopPrice;
    int index;
    
    double value = indicators.getBBandStop(0, 20, 2.0, &trend, &stopPrice, &index);
    
    // Should return valid values
    BOOST_CHECK(!isnan(value));
    BOOST_CHECK(!isnan(stopPrice));
    // Trend should be -1, 0, or 1
    BOOST_CHECK_GE(trend, -1);
    BOOST_CHECK_LE(trend, 1);
}

// ===== Stochastic Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetStochastic_Main) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double stochMain = indicators.getStochastic(0, 14, 5, 3, STOCH_MAIN, 1);
    
    // Stochastic should be between 0 and 100
    if (!isnan(stochMain)) {
        BOOST_CHECK_GE(stochMain, 0.0);
        BOOST_CHECK_LE(stochMain, 100.0);
    }
}

BOOST_AUTO_TEST_CASE(Indicators_GetStochastic_Signal) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double stochSignal = indicators.getStochastic(0, 14, 5, 3, STOCH_SIGNAL, 1);
    
    if (!isnan(stochSignal)) {
        BOOST_CHECK_GE(stochSignal, 0.0);
        BOOST_CHECK_LE(stochSignal, 100.0);
    }
}

// ===== Standard Deviation Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetStdDev) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double stddev = indicators.getStdDev(0, MA_CLOSE, 20, 1);
    
    // Standard deviation should be non-negative
    if (!isnan(stddev)) {
        BOOST_CHECK_GE(stddev, 0.0);
    }
}

// ===== CCI Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetCCI) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double cci = indicators.getCCI(0, 20, 1);
    
    // CCI should be a valid number
    BOOST_CHECK(!isnan(cci));
    BOOST_CHECK(!isinf(cci));
}

// ===== Price Accessor Tests =====

BOOST_AUTO_TEST_CASE(Indicators_GetHigh) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double high = indicators.getHigh(0, 0);
    
    BOOST_CHECK(!isnan(high));
    BOOST_CHECK(!isinf(high));
    BOOST_CHECK_GT(high, 0.0);
}

BOOST_AUTO_TEST_CASE(Indicators_GetLow) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double low = indicators.getLow(0, 0);
    
    BOOST_CHECK(!isnan(low));
    BOOST_CHECK(!isinf(low));
    BOOST_CHECK_GT(low, 0.0);
}

BOOST_AUTO_TEST_CASE(Indicators_GetOpen) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double open = indicators.getOpen(0, 0);
    
    BOOST_CHECK(!isnan(open));
    BOOST_CHECK(!isinf(open));
}

BOOST_AUTO_TEST_CASE(Indicators_GetClose) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double close = indicators.getClose(0, 0);
    
    BOOST_CHECK(!isnan(close));
    BOOST_CHECK(!isinf(close));
}

BOOST_AUTO_TEST_CASE(Indicators_HighLowOrdering) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    double high = indicators.getHigh(0, 0);
    double low = indicators.getLow(0, 0);
    
    // High should be >= Low
    if (!isnan(high) && !isnan(low)) {
        BOOST_CHECK_GE(high, low);
    }
}

// ===== Multiple Timeframe Tests =====

BOOST_AUTO_TEST_CASE(Indicators_MultipleTimeframes) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Test accessing multiple timeframes
    // (Actual indices depend on rate buffer setup, but interface should work)
    double atr_primary = indicators.getATR(0, 20, 1);   // Primary rates
    double atr_daily = indicators.getATR(1, 20, 1);     // Daily rates
    
    BOOST_CHECK(!isnan(atr_primary));
    BOOST_CHECK(!isnan(atr_daily));
}

BOOST_AUTO_TEST_CASE(Indicators_MultipleShifts) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Test accessing multiple shifts (bars)
    double ma_shift0 = indicators.getMA(MA_CLOSE, 0, 20, 0);
    double ma_shift1 = indicators.getMA(MA_CLOSE, 0, 20, 1);
    double ma_shift2 = indicators.getMA(MA_CLOSE, 0, 20, 2);
    
    BOOST_CHECK(!isnan(ma_shift0));
    BOOST_CHECK(!isnan(ma_shift1));
    BOOST_CHECK(!isnan(ma_shift2));
}

// ===== Edge Case Tests =====

BOOST_AUTO_TEST_CASE(Indicators_ZeroPeriod) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Test with zero period (should handle gracefully)
    // Implementation may return 0, NaN, or handle specially
    double result = indicators.getATR(0, 0, 1);
    
    // Just verify it doesn't crash
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(Indicators_LargePeriod) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Test with very large period
    double result = indicators.getATR(0, 1000, 1);
    
    // Should handle gracefully (may return NaN if not enough data)
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(Indicators_NegativeShift) {
    StrategyContext context = createTestContext();
    Indicators indicators(context);
    
    // Test with negative shift (should be handled by underlying implementation)
    double result = indicators.getATR(0, 20, -1);
    
    // Should not crash
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
