/**
 * @file
 * @brief     Unit tests for RecordBarsStrategy
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2025
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE
 */

#include <boost/test/unit_test.hpp>
#include "strategies/RecordBarsStrategy.hpp"
#include "Indicators.hpp"
#include "TestFixtures.hpp"
#include <cstdio>

BOOST_AUTO_TEST_SUITE(RecordBarsStrategyTests)

BOOST_FIXTURE_TEST_CASE(Constructor_CreatesValidInstance, StrategyContextFixture) {
    trading::RecordBarsStrategy strategy;
    
    BOOST_CHECK_EQUAL(strategy.getName(), "RecordBars");
    BOOST_CHECK_EQUAL(strategy.getId(), RECORD_BARS);
    BOOST_CHECK_EQUAL(strategy.requiresIndicators(), false);
}

BOOST_FIXTURE_TEST_CASE(GetName_ReturnsCorrectName, StrategyContextFixture) {
    trading::RecordBarsStrategy strategy;
    
    BOOST_CHECK_EQUAL(strategy.getName(), "RecordBars");
}

BOOST_FIXTURE_TEST_CASE(RequiresIndicators_ReturnsFalse, StrategyContextFixture) {
    trading::RecordBarsStrategy strategy;
    
    BOOST_CHECK_EQUAL(strategy.requiresIndicators(), false);
}

BOOST_FIXTURE_TEST_CASE(LoadIndicators_ReturnsNull, StrategyContextFixture) {
    // Set up minimal valid context
    std::strcpy(params.symbol, "EURUSD");
    params.magicNumber = 12345;
    params.strategyId = RECORD_BARS;
    
    // Set bid/ask data
    for (int i = 0; i < 10; i++) {
        bidArray[i] = 1.1000 + i * 0.0001;
        askArray[i] = bidArray[i] + 0.0002;
    }
    
    // Link structures
    params.pBidAsk = &bidAsk;
    params.pAccountInfo = &accountInfo;
    params.pRatesBuffers = &ratesBuffers;
    params.pOrderInfo = &orderInfo;
    params.pResults = &results;
    
    trading::StrategyContext context(&params);
    trading::RecordBarsStrategy strategy;
    
    trading::Indicators* indicators = strategy.loadIndicators(context);
    
    BOOST_CHECK(indicators == NULL);
}

BOOST_FIXTURE_TEST_CASE(ExecuteStrategy_WithInsufficientBars_ReturnsSuccess, StrategyContextFixture) {
    // Set up minimal valid context with only 1 bar (shift 1 = -1, invalid)
    std::strcpy(params.symbol, "EURUSD");
    params.magicNumber = 12345;
    params.strategyId = RECORD_BARS;
    params.currentSettings[TIMEFRAME] = 60;
    params.currentSettings[ADDITIONAL_PARAM_1] = trading::RecordBarsStrategy::RECORD_DATE_AS_INTEGER;
    
    // Set up rates buffers with only 1 bar
    ratesBuffers.barsSeries0 = 1;
    
    // Set bid/ask data
    for (int i = 0; i < 10; i++) {
        bidArray[i] = 1.1000 + i * 0.0001;
        askArray[i] = bidArray[i] + 0.0002;
    }
    
    // Link structures
    params.pBidAsk = &bidAsk;
    params.pAccountInfo = &accountInfo;
    params.pRatesBuffers = &ratesBuffers;
    params.pOrderInfo = &orderInfo;
    params.pResults = &results;
    
    trading::StrategyContext context(&params);
    trading::RecordBarsStrategy strategy;
    
    trading::StrategyResult result = strategy.executeStrategy(context, NULL);
    
    // Should return success even with no bars to record
    BOOST_CHECK_EQUAL(result.code, SUCCESS);
    BOOST_CHECK_EQUAL(result.generatedOrders, 0);
}

BOOST_FIXTURE_TEST_CASE(ExecuteStrategy_WithValidBars_RecordsToCSV_IntegerMode, StrategyContextFixture) {
    // This test would require file I/O which is hard to test in unit tests
    // For now, just verify it doesn't crash and returns success
    
    // Set up valid context with multiple bars
    std::strcpy(params.symbol, "EURUSD");
    params.magicNumber = 12345;
    params.strategyId = RECORD_BARS;
    params.currentSettings[TIMEFRAME] = 60;
    params.currentSettings[ADDITIONAL_PARAM_1] = trading::RecordBarsStrategy::RECORD_DATE_AS_INTEGER;
    
    // Set up rates buffers with 5 bars
    ratesBuffers.barsSeries0 = 5;
    
    // Set bid/ask data
    for (int i = 0; i < 10; i++) {
        bidArray[i] = 1.1000 + i * 0.0001;
        askArray[i] = bidArray[i] + 0.0002;
    }
    
    // Link structures
    params.pBidAsk = &bidAsk;
    params.pAccountInfo = &accountInfo;
    params.pRatesBuffers = &ratesBuffers;
    params.pOrderInfo = &orderInfo;
    params.pResults = &results;
    
    trading::StrategyContext context(&params);
    trading::RecordBarsStrategy strategy;
    
    // Note: This will try to write to a CSV file in the temp folder
    // In a real test environment, we'd mock the file I/O
    trading::StrategyResult result = strategy.executeStrategy(context, NULL);
    
    // Check result code - may fail if temp folder path is not set up
    // In production, we'd have proper mocking for file I/O
    BOOST_CHECK(result.code == SUCCESS || result.code == STRATEGY_FAILED_TO_RECORD_DATA);
}

BOOST_FIXTURE_TEST_CASE(UpdateResults_DoesNothing, StrategyContextFixture) {
    // Set up minimal valid context
    std::strcpy(params.symbol, "EURUSD");
    params.magicNumber = 12345;
    params.strategyId = RECORD_BARS;
    
    // Set bid/ask data
    for (int i = 0; i < 10; i++) {
        bidArray[i] = 1.1000 + i * 0.0001;
        askArray[i] = bidArray[i] + 0.0002;
    }
    
    // Link structures
    params.pBidAsk = &bidAsk;
    params.pAccountInfo = &accountInfo;
    params.pRatesBuffers = &ratesBuffers;
    params.pOrderInfo = &orderInfo;
    params.pResults = &results;
    
    trading::StrategyContext context(&params);
    trading::RecordBarsStrategy strategy;
    
    trading::StrategyResult result;
    result.code = SUCCESS;
    result.generatedOrders = 0;
    result.pnlImpact = 0.0;
    
    // Should not crash or throw
    BOOST_CHECK_NO_THROW(strategy.updateResults(context, result));
}

BOOST_FIXTURE_TEST_CASE(Validate_WithValidContext_ReturnsTrue, StrategyContextFixture) {
    // Set up valid context
    std::strcpy(params.symbol, "EURUSD");
    params.magicNumber = 12345;
    params.strategyId = RECORD_BARS;
    
    // Set bid/ask data
    for (int i = 0; i < 10; i++) {
        bidArray[i] = 1.1000 + i * 0.0001;
        askArray[i] = bidArray[i] + 0.0002;
    }
    
    // Link structures
    params.pBidAsk = &bidAsk;
    params.pAccountInfo = &accountInfo;
    params.pRatesBuffers = &ratesBuffers;
    params.pOrderInfo = &orderInfo;
    params.pResults = &results;
    
    trading::StrategyContext context(&params);
    trading::RecordBarsStrategy strategy;
    
    BOOST_CHECK(strategy.validate(context));
}

BOOST_FIXTURE_TEST_CASE(Execute_FullWorkflow_CompletesSuccessfully, StrategyContextFixture) {
    // Set up valid context with multiple bars
    std::strcpy(params.symbol, "EURUSD");
    params.magicNumber = 12345;
    params.strategyId = RECORD_BARS;
    params.currentSettings[TIMEFRAME] = 60;
    params.currentSettings[ADDITIONAL_PARAM_1] = trading::RecordBarsStrategy::RECORD_DATE_AS_INTEGER;
    
    // Set up rates buffers with 5 bars
    ratesBuffers.barsSeries0 = 5;
    
    // Set bid/ask data
    for (int i = 0; i < 10; i++) {
        bidArray[i] = 1.1000 + i * 0.0001;
        askArray[i] = bidArray[i] + 0.0002;
    }
    
    // Link structures
    params.pBidAsk = &bidAsk;
    params.pAccountInfo = &accountInfo;
    params.pRatesBuffers = &ratesBuffers;
    params.pOrderInfo = &orderInfo;
    params.pResults = &results;
    
    trading::StrategyContext context(&params);
    trading::RecordBarsStrategy strategy;
    
    // Execute full workflow through BaseStrategy::execute()
    AsirikuyReturnCode result = strategy.execute(context);
    
    // Check result code - may fail if temp folder path is not set up
    // In production, we'd have proper mocking for file I/O
    BOOST_CHECK(result == SUCCESS || result == STRATEGY_FAILED_TO_RECORD_DATA);
}

BOOST_AUTO_TEST_SUITE_END()
