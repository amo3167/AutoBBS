/**
 * @file
 * @brief     Unit tests for BaseStrategy
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2025
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE
 */

#include <boost/test/unit_test.hpp>
#include "BaseStrategy.hpp"
#include "Indicators.hpp"
#include "TestFixtures.hpp"

BOOST_AUTO_TEST_SUITE(BaseStrategyTests)

// Mock concrete strategy for testing BaseStrategy behavior
class MockStrategy : public trading::BaseStrategy {
public:
    MockStrategy() : trading::BaseStrategy(RECORD_BARS), 
        executeStrategyCalled(false),
        updateResultsCalled(false),
        loadIndicatorsCalled(false),
        shouldReturnNull(false),
        executeReturnCode(SUCCESS) {}
    
    std::string getName() const {
        return "MockStrategy";
    }
    
    // Test flags
    mutable bool executeStrategyCalled;
    mutable bool updateResultsCalled;
    mutable bool loadIndicatorsCalled;
    bool shouldReturnNull;
    AsirikuyReturnCode executeReturnCode;
    
protected:
    trading::Indicators* loadIndicators(const trading::StrategyContext& context) {
        loadIndicatorsCalled = true;
        if (shouldReturnNull) {
            return NULL;
        }
        return new trading::Indicators();
    }
    
    trading::StrategyResult executeStrategy(
        const trading::StrategyContext& context,
        trading::Indicators* indicators)
    {
        executeStrategyCalled = true;
        trading::StrategyResult result;
        result.code = executeReturnCode;
        result.generatedOrders = 2;
        result.pnlImpact = 150.5;
        result.message = "Test execution";
        return result;
    }
    
    void updateResults(
        const trading::StrategyContext& context,
        const trading::StrategyResult& result)
    {
        updateResultsCalled = true;
    }
    
    bool requiresIndicators() const {
        return true;
    }
};

// ============================================================================
// Constructor and ID Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(constructor_sets_strategy_id)
{
    MockStrategy strategy;
    BOOST_CHECK_EQUAL(strategy.getId(), RECORD_BARS);
}

BOOST_AUTO_TEST_CASE(getName_returns_strategy_name)
{
    MockStrategy strategy;
    BOOST_CHECK_EQUAL(strategy.getName(), "MockStrategy");
}

// ============================================================================
// Template Method Pattern Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(execute_calls_hooks_in_correct_order)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    AsirikuyReturnCode result = strategy.execute(ctx);
    
    // Verify all hooks were called
    BOOST_CHECK(strategy.loadIndicatorsCalled);
    BOOST_CHECK(strategy.executeStrategyCalled);
    BOOST_CHECK(strategy.updateResultsCalled);
    BOOST_CHECK_EQUAL(result, SUCCESS);
}

BOOST_AUTO_TEST_CASE(execute_returns_success_when_strategy_succeeds)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    strategy.executeReturnCode = SUCCESS;
    
    AsirikuyReturnCode result = strategy.execute(ctx);
    
    BOOST_CHECK_EQUAL(result, SUCCESS);
}

BOOST_AUTO_TEST_CASE(execute_returns_error_when_strategy_fails)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    strategy.executeReturnCode = INVALID_PARAMETER;
    
    AsirikuyReturnCode result = strategy.execute(ctx);
    
    BOOST_CHECK_EQUAL(result, INVALID_PARAMETER);
}

// ============================================================================
// Validation Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(execute_validates_context_before_execution)
{
    StrategyContextFixture fixture;
    // Set invalid symbol
    fixture.params.tradeSymbol[0] = '\0';
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    AsirikuyReturnCode result = strategy.execute(ctx);
    
    // Should fail validation and not execute strategy
    BOOST_CHECK_EQUAL(result, static_cast<AsirikuyReturnCode>(STRATEGY_INVALID_PARAMETERS));
    BOOST_CHECK(!strategy.executeStrategyCalled);
}

BOOST_AUTO_TEST_CASE(validate_returns_false_for_null_symbol)
{
    StrategyContextFixture fixture;
    fixture.params.tradeSymbol[0] = '\0';
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    bool isValid = strategy.validate(ctx);
    
    BOOST_CHECK(!isValid);
}

BOOST_AUTO_TEST_CASE(validate_returns_false_for_invalid_bid)
{
    StrategyContextFixture fixture;
    fixture.bidAsk.bid[0] = 0.0;  // Invalid
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    bool isValid = strategy.validate(ctx);
    
    BOOST_CHECK(!isValid);
}

BOOST_AUTO_TEST_CASE(validate_returns_false_for_invalid_ask)
{
    StrategyContextFixture fixture;
    fixture.bidAsk.ask[0] = 0.0;  // Invalid
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    bool isValid = strategy.validate(ctx);
    
    BOOST_CHECK(!isValid);
}

BOOST_AUTO_TEST_CASE(validate_returns_false_when_ask_less_than_bid)
{
    StrategyContextFixture fixture;
    fixture.bidAsk.bid[0] = 1.2000;
    fixture.bidAsk.ask[0] = 1.1999;  // Ask < Bid is invalid
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    bool isValid = strategy.validate(ctx);
    
    BOOST_CHECK(!isValid);
}

BOOST_AUTO_TEST_CASE(validate_returns_true_for_valid_context)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    
    bool isValid = strategy.validate(ctx);
    
    BOOST_CHECK(isValid);
}

// ============================================================================
// Indicator Loading Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(execute_fails_if_indicators_required_but_not_loaded)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    MockStrategy strategy;
    strategy.shouldReturnNull = true;  // loadIndicators returns NULL
    
    AsirikuyReturnCode result = strategy.execute(ctx);
    
    BOOST_CHECK_EQUAL(result, static_cast<AsirikuyReturnCode>(STRATEGY_FAILED_TO_LOAD_INDICATORS));
    BOOST_CHECK(!strategy.executeStrategyCalled);
}

// Mock strategy that doesn't require indicators
class MockStrategyNoIndicators : public MockStrategy {
protected:
    bool requiresIndicators() const {
        return false;
    }
};

BOOST_AUTO_TEST_CASE(execute_succeeds_if_indicators_not_required)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    MockStrategyNoIndicators strategy;
    strategy.shouldReturnNull = true;  // loadIndicators returns NULL
    
    AsirikuyReturnCode result = strategy.execute(ctx);
    
    // Should succeed because indicators not required
    BOOST_CHECK_EQUAL(result, SUCCESS);
    BOOST_CHECK(strategy.executeStrategyCalled);
}

// ============================================================================
// StrategyResult Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(strategy_result_defaults_to_success)
{
    trading::StrategyResult result;
    
    BOOST_CHECK_EQUAL(result.code, SUCCESS);
    BOOST_CHECK_EQUAL(result.generatedOrders, 0);
    BOOST_CHECK_EQUAL(result.pnlImpact, 0.0);
    BOOST_CHECK(result.message.empty());
}

BOOST_AUTO_TEST_CASE(strategy_result_can_store_execution_data)
{
    trading::StrategyResult result;
    result.code = INVALID_PARAMETER;
    result.generatedOrders = 5;
    result.pnlImpact = 250.75;
    result.message = "Test message";
    
    BOOST_CHECK_EQUAL(result.code, INVALID_PARAMETER);
    BOOST_CHECK_EQUAL(result.generatedOrders, 5);
    BOOST_CHECK_CLOSE(result.pnlImpact, 250.75, 0.001);
    BOOST_CHECK_EQUAL(result.message, "Test message");
}

// ============================================================================
// Integration Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(execute_passes_indicators_to_executeStrategy)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    // Custom mock to verify indicators are passed
    class IndicatorCheckStrategy : public MockStrategy {
    public:
        mutable bool indicatorsWereNotNull;
        
        IndicatorCheckStrategy() : indicatorsWereNotNull(false) {}
        
    protected:
        trading::StrategyResult executeStrategy(
            const trading::StrategyContext& context,
            trading::Indicators* indicators)
        {
            indicatorsWereNotNull = (indicators != NULL);
            return MockStrategy::executeStrategy(context, indicators);
        }
    };
    
    IndicatorCheckStrategy strategy;
    strategy.execute(ctx);
    
    BOOST_CHECK(strategy.indicatorsWereNotNull);
}

BOOST_AUTO_TEST_CASE(execute_passes_result_to_updateResults)
{
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    // Custom mock to verify result is passed
    class ResultCheckStrategy : public MockStrategy {
    public:
        mutable int receivedGeneratedOrders;
        mutable double receivedPnlImpact;
        
        ResultCheckStrategy() : receivedGeneratedOrders(-1), receivedPnlImpact(-1.0) {}
        
    protected:
        void updateResults(
            const trading::StrategyContext& context,
            const trading::StrategyResult& result)
        {
            receivedGeneratedOrders = result.generatedOrders;
            receivedPnlImpact = result.pnlImpact;
            MockStrategy::updateResults(context, result);
        }
    };
    
    ResultCheckStrategy strategy;
    strategy.executeReturnCode = SUCCESS;
    strategy.execute(ctx);
    
    // Verify result data was passed through
    BOOST_CHECK_EQUAL(strategy.receivedGeneratedOrders, 2);
    BOOST_CHECK_CLOSE(strategy.receivedPnlImpact, 150.5, 0.001);
}

BOOST_AUTO_TEST_SUITE_END()
