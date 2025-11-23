/**
 * @file
 * @brief     Unit tests for StrategyContext
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2025
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE
 */

#include <boost/test/unit_test.hpp>
#include "StrategyContext.hpp"
#include "TestFixtures.hpp"
#include <stdexcept>

// ============================================================================
// StrategyContext Constructor Tests
// ============================================================================

BOOST_AUTO_TEST_SUITE(StrategyContext_Tests)

BOOST_AUTO_TEST_CASE(constructor_with_null_params_throws)
{
    BOOST_CHECK_THROW(
        trading::StrategyContext ctx(NULL),
        std::invalid_argument
    );
}

BOOST_AUTO_TEST_CASE(constructor_with_valid_params_succeeds)
{
    StrategyContextFixture fixture;
    BOOST_CHECK_NO_THROW(
        trading::StrategyContext ctx(&fixture.params)
    );
}

// ============================================================================
// Strategy Identification Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getStrategyId_returns_correct_value)
{
    StrategyContextFixture fixture;
    fixture.params.settings[INTERNAL_STRATEGY_ID] = static_cast<double>(TAKEOVER);
    
    trading::StrategyContext ctx(&fixture.params);
    BOOST_CHECK_EQUAL(ctx.getStrategyId(), TAKEOVER);
}

BOOST_AUTO_TEST_CASE(getSymbol_returns_correct_symbol)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_EQUAL(std::string(ctx.getSymbol()), "EURUSD");
}

BOOST_AUTO_TEST_CASE(getCurrentTime_returns_correct_time)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_EQUAL(ctx.getCurrentTime(), 1234567890);
}

// ============================================================================
// Settings Access Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getSetting_returns_correct_value)
{
    StrategyContextFixture fixture;
    fixture.params.settings[MAX_OPEN_ORDERS] = 42.5;
    
    trading::StrategyContext ctx(&fixture.params);
    BOOST_CHECK_CLOSE(ctx.getSetting(MAX_OPEN_ORDERS), 42.5, 0.001);
}

BOOST_AUTO_TEST_CASE(isBacktesting_returns_true_when_enabled)
{
    StrategyContextFixture fixture;
    fixture.params.settings[IS_BACKTESTING] = 1.0;
    
    trading::StrategyContext ctx(&fixture.params);
    BOOST_CHECK(ctx.isBacktesting());
}

BOOST_AUTO_TEST_CASE(isBacktesting_returns_false_when_disabled)
{
    StrategyContextFixture fixture;
    fixture.params.settings[IS_BACKTESTING] = 0.0;
    
    trading::StrategyContext ctx(&fixture.params);
    BOOST_CHECK(!ctx.isBacktesting());
}

BOOST_AUTO_TEST_CASE(getMaxOpenOrders_returns_correct_value)
{
    StrategyContextFixture fixture;
    fixture.params.settings[MAX_OPEN_ORDERS] = 7.0;
    
    trading::StrategyContext ctx(&fixture.params);
    BOOST_CHECK_EQUAL(ctx.getMaxOpenOrders(), 7);
}

// ============================================================================
// Market Data Access Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getBidAsk_returns_reference)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    const BidAsk& bidAsk = ctx.getBidAsk();
    BOOST_CHECK_EQUAL(bidAsk.arraySize, 10);
}

BOOST_AUTO_TEST_CASE(getBid_returns_correct_value_at_index_0)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_CLOSE(ctx.getBid(0), 1.2000, 0.0001);
}

BOOST_AUTO_TEST_CASE(getBid_returns_correct_value_at_index_5)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_CLOSE(ctx.getBid(5), 1.2005, 0.0001);
}

BOOST_AUTO_TEST_CASE(getBid_throws_on_negative_index)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_THROW(ctx.getBid(-1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(getBid_throws_on_out_of_bounds_index)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_THROW(ctx.getBid(10), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(getAsk_returns_correct_value_at_index_0)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_CLOSE(ctx.getAsk(0), 1.2010, 0.0001);
}

BOOST_AUTO_TEST_CASE(getAsk_throws_on_out_of_bounds_index)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_THROW(ctx.getAsk(10), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(getSpread_calculates_correctly)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    double spread = ctx.getSpread();
    BOOST_CHECK_CLOSE(spread, 0.0010, 0.0001);
}

// ============================================================================
// Account Information Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getAccountInfo_returns_reference)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    const AccountInfo& info = ctx.getAccountInfo();
    BOOST_CHECK_CLOSE(info.balance, 10000.0, 0.01);
}

BOOST_AUTO_TEST_CASE(getBalance_returns_correct_value)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_CLOSE(ctx.getBalance(), 10000.0, 0.01);
}

BOOST_AUTO_TEST_CASE(getEquity_returns_correct_value)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_CLOSE(ctx.getEquity(), 10500.0, 0.01);
}

BOOST_AUTO_TEST_CASE(getMargin_returns_correct_value)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_CLOSE(ctx.getMargin(), 9500.0, 0.01);
}

// ============================================================================
// Price Data Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getRatesBuffers_returns_pointer)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    RatesBuffers* rates = ctx.getRatesBuffers();
    BOOST_CHECK(rates != NULL);
}

BOOST_AUTO_TEST_CASE(getRates_throws_on_negative_index)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_THROW(ctx.getRates(static_cast<BaseRatesIndexes>(-1)), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(getRates_throws_on_out_of_bounds_index)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_THROW(ctx.getRates(static_cast<BaseRatesIndexes>(BASE_RATES_INDEXES_COUNT)), std::out_of_range);
}

// ============================================================================
// Order Information Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getOrderInfo_returns_pointer)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    OrderInfo* info = ctx.getOrderInfo();
    BOOST_CHECK(info != NULL);
}

// ============================================================================
// Results Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getResults_returns_pointer)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    StrategyResults* results = ctx.getResults();
    BOOST_CHECK(results != NULL);
}

// ============================================================================
// Lazy-Loading Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getIndicators_creates_indicators_on_first_call)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    // Should not throw
    BOOST_CHECK_NO_THROW(ctx.getIndicators());
}

BOOST_AUTO_TEST_CASE(getIndicators_returns_same_instance_on_repeated_calls)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    trading::Indicators& indicators1 = ctx.getIndicators();
    trading::Indicators& indicators2 = ctx.getIndicators();
    
    // Should be same address
    BOOST_CHECK_EQUAL(&indicators1, &indicators2);
}

BOOST_AUTO_TEST_CASE(getOrderManager_creates_manager_on_first_call)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    // Should not throw
    BOOST_CHECK_NO_THROW(ctx.getOrderManager());
}

BOOST_AUTO_TEST_CASE(getOrderManager_returns_same_instance_on_repeated_calls)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    trading::OrderManager& manager1 = ctx.getOrderManager();
    trading::OrderManager& manager2 = ctx.getOrderManager();
    
    // Should be same address
    BOOST_CHECK_EQUAL(&manager1, &manager2);
}

// ============================================================================
// Raw Access Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getRawParams_returns_original_pointer)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    BOOST_CHECK_EQUAL(ctx.getRawParams(), &fixture.params);
}

BOOST_AUTO_TEST_SUITE_END()
