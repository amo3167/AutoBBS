/**
 * @file
 * @brief     Unit tests for StrategyFactory
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2025
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE
 */

#include <boost/test/unit_test.hpp>
#include "StrategyFactory.hpp"
#include "IStrategy.hpp"
#include "TestFixtures.hpp"

BOOST_AUTO_TEST_SUITE(StrategyFactoryTests)

// ============================================================================
// Singleton Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(getInstance_returns_same_instance)
{
    trading::StrategyFactory& factory1 = trading::StrategyFactory::getInstance();
    trading::StrategyFactory& factory2 = trading::StrategyFactory::getInstance();
    
    // Should be same address
    BOOST_CHECK_EQUAL(&factory1, &factory2);
}

// ============================================================================
// Registration Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(all_six_strategies_are_registered)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    // Verify all 6 strategies are registered
    BOOST_CHECK_EQUAL(factory.getRegisteredCount(), 6);
}

BOOST_AUTO_TEST_CASE(isRegistered_returns_true_for_all_strategy_ids)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    BOOST_CHECK(factory.isRegistered(RECORD_BARS));
    BOOST_CHECK(factory.isRegistered(TAKEOVER));
    BOOST_CHECK(factory.isRegistered(SCREENING));
    BOOST_CHECK(factory.isRegistered(TRENDLIMIT));
    BOOST_CHECK(factory.isRegistered(AUTOBBS));
    BOOST_CHECK(factory.isRegistered(AUTOBBSWEEKLY));
}

BOOST_AUTO_TEST_CASE(isRegistered_returns_false_for_unregistered_id)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    // Use an invalid ID (casting to StrategyId for test)
    StrategyId invalidId = static_cast<StrategyId>(999);
    BOOST_CHECK(!factory.isRegistered(invalidId));
}

// ============================================================================
// Strategy Creation Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(createStrategy_returns_non_null_for_RECORD_BARS)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy = factory.createStrategy(RECORD_BARS);
    
    BOOST_REQUIRE(strategy != NULL);
    BOOST_CHECK_EQUAL(strategy->getId(), RECORD_BARS);
    BOOST_CHECK_EQUAL(strategy->getName(), "RecordBars");
    
    delete strategy;
}

BOOST_AUTO_TEST_CASE(createStrategy_returns_non_null_for_TAKEOVER)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy = factory.createStrategy(TAKEOVER);
    
    BOOST_REQUIRE(strategy != NULL);
    BOOST_CHECK_EQUAL(strategy->getId(), TAKEOVER);
    BOOST_CHECK_EQUAL(strategy->getName(), "TakeOver");
    
    delete strategy;
}

BOOST_AUTO_TEST_CASE(createStrategy_returns_non_null_for_SCREENING)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy = factory.createStrategy(SCREENING);
    
    BOOST_REQUIRE(strategy != NULL);
    BOOST_CHECK_EQUAL(strategy->getId(), SCREENING);
    BOOST_CHECK_EQUAL(strategy->getName(), "Screening");
    
    delete strategy;
}

BOOST_AUTO_TEST_CASE(createStrategy_returns_non_null_for_TRENDLIMIT)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy = factory.createStrategy(TRENDLIMIT);
    
    BOOST_REQUIRE(strategy != NULL);
    BOOST_CHECK_EQUAL(strategy->getId(), TRENDLIMIT);
    BOOST_CHECK_EQUAL(strategy->getName(), "TrendLimit");
    
    delete strategy;
}

BOOST_AUTO_TEST_CASE(createStrategy_returns_non_null_for_AUTOBBS)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy = factory.createStrategy(AUTOBBS);
    
    BOOST_REQUIRE(strategy != NULL);
    BOOST_CHECK_EQUAL(strategy->getId(), AUTOBBS);
    BOOST_CHECK_EQUAL(strategy->getName(), "AutoBBS");
    
    delete strategy;
}

BOOST_AUTO_TEST_CASE(createStrategy_returns_non_null_for_AUTOBBSWEEKLY)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy = factory.createStrategy(AUTOBBSWEEKLY);
    
    BOOST_REQUIRE(strategy != NULL);
    BOOST_CHECK_EQUAL(strategy->getId(), AUTOBBSWEEKLY);
    BOOST_CHECK_EQUAL(strategy->getName(), "AutoBBS Weekly");
    
    delete strategy;
}

BOOST_AUTO_TEST_CASE(createStrategy_returns_null_for_unregistered_id)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    StrategyId invalidId = static_cast<StrategyId>(999);
    trading::IStrategy* strategy = factory.createStrategy(invalidId);
    
    BOOST_CHECK(strategy == NULL);
}

// ============================================================================
// Strategy Creation Independence Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(createStrategy_returns_different_instances_on_repeated_calls)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    
    trading::IStrategy* strategy1 = factory.createStrategy(RECORD_BARS);
    trading::IStrategy* strategy2 = factory.createStrategy(RECORD_BARS);
    
    BOOST_REQUIRE(strategy1 != NULL);
    BOOST_REQUIRE(strategy2 != NULL);
    
    // Should be different instances
    BOOST_CHECK_NE(strategy1, strategy2);
    
    delete strategy1;
    delete strategy2;
}

// ============================================================================
// Strategy Validation Tests
// ============================================================================

BOOST_AUTO_TEST_CASE(created_strategies_can_validate_context)
{
    trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
    StrategyContextFixture fixture;
    trading::StrategyContext ctx(&fixture.params);
    
    // Test each strategy can validate
    trading::IStrategy* strategies[] = {
        factory.createStrategy(RECORD_BARS),
        factory.createStrategy(TAKEOVER),
        factory.createStrategy(SCREENING),
        factory.createStrategy(TRENDLIMIT),
        factory.createStrategy(AUTOBBS),
        factory.createStrategy(AUTOBBSWEEKLY)
    };
    
    for (size_t i = 0; i < 6; ++i) {
        BOOST_REQUIRE(strategies[i] != NULL);
        
        // Validation should pass with valid fixture data
        bool isValid = strategies[i]->validate(ctx);
        BOOST_CHECK(isValid);
        
        delete strategies[i];
    }
}

BOOST_AUTO_TEST_SUITE_END()
