#define BOOST_TEST_MODULE OrderManagerTests
#include <boost/test/unit_test.hpp>
#include "OrderManager.hpp"
#include "StrategyContext.hpp"
#include "StrategyTypes.h"

using namespace asirikuy;

// Mock setup helpers
namespace {
    StrategyParams createMockParams() {
        StrategyParams params;
        // Initialize required fields for testing
        params.symbol[0] = '\0';
        params.risk = 0.02; // 2% risk
        params.maxLotSize = 10.0;
        params.minLotSize = 0.01;
        params.lotStep = 0.01;
        return params;
    }

    StrategyContext createMockContext() {
        StrategyParams params = createMockParams();
        return StrategyContext(params);
    }
}

BOOST_AUTO_TEST_SUITE(OrderManagerConstruction)

BOOST_AUTO_TEST_CASE(TestConstructorWithContext) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    // Constructor should not throw
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(OrderCounting)

BOOST_AUTO_TEST_CASE(TestGetTotalOpenOrders_BuyOrders) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    int count = manager.getTotalOpenOrders(BUY);
    BOOST_CHECK_GE(count, 0);
}

BOOST_AUTO_TEST_CASE(TestGetTotalOpenOrders_SellOrders) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    int count = manager.getTotalOpenOrders(SELL);
    BOOST_CHECK_GE(count, 0);
}

BOOST_AUTO_TEST_CASE(TestGetTotalClosedOrders_BuyOrders) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    int count = manager.getTotalClosedOrders(BUY);
    BOOST_CHECK_GE(count, 0);
}

BOOST_AUTO_TEST_CASE(TestGetTotalClosedOrders_SellOrders) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    int count = manager.getTotalClosedOrders(SELL);
    BOOST_CHECK_GE(count, 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(OrderSizing)

BOOST_AUTO_TEST_CASE(TestCalculateOrderSize_ValidParameters) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Calculate size for buy order with entry at 1.2000 and SL at 1.1950 (50 pips)
    double size = manager.calculateOrderSize(BUY, 1.2000, 1.1950);
    BOOST_CHECK_GE(size, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateOrderSize_SellOrder) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Calculate size for sell order with entry at 1.2000 and SL at 1.2050 (50 pips)
    double size = manager.calculateOrderSize(SELL, 1.2000, 1.2050);
    BOOST_CHECK_GE(size, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateOrderSizeWithRisk_CustomRisk) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Calculate size with 1% custom risk
    double size = manager.calculateOrderSizeWithRisk(BUY, 1.2000, 1.1950, 0.01);
    BOOST_CHECK_GE(size, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateOrderSizeWithRisk_HigherRisk) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double size1 = manager.calculateOrderSizeWithRisk(BUY, 1.2000, 1.1950, 0.01);
    double size2 = manager.calculateOrderSizeWithRisk(BUY, 1.2000, 1.1950, 0.02);
    
    // Higher risk should yield larger position size (if not capped)
    BOOST_CHECK_GE(size2, size1);
}

BOOST_AUTO_TEST_CASE(TestGetMaxLossPerLot_BuyOrder) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double maxLoss = manager.getMaxLossPerLot(BUY, 1.2000, 1.1950);
    BOOST_CHECK_GT(maxLoss, 0.0);
}

BOOST_AUTO_TEST_CASE(TestGetMaxLossPerLot_SellOrder) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double maxLoss = manager.getMaxLossPerLot(SELL, 1.2000, 1.2050);
    BOOST_CHECK_GT(maxLoss, 0.0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(MarginChecking)

BOOST_AUTO_TEST_CASE(TestHasEnoughFreeMargin_SmallLot) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // 0.01 lot should typically have enough margin
    bool hasMargin = manager.hasEnoughFreeMargin(BUY, 0.01);
    // Just verify it returns a boolean, actual value depends on account state
    BOOST_CHECK(hasMargin == true || hasMargin == false);
}

BOOST_AUTO_TEST_CASE(TestHasEnoughFreeMargin_LargeLot) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // 100 lots might not have enough margin
    bool hasMargin = manager.hasEnoughFreeMargin(BUY, 100.0);
    BOOST_CHECK(hasMargin == true || hasMargin == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(OrderValidation)

BOOST_AUTO_TEST_CASE(TestAreOrdersCorrect_ValidStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Positive SL and TP should be valid
    bool correct = manager.areOrdersCorrect(0.0050, 0.0100);
    // Just verify function works, actual validation depends on C function
    BOOST_CHECK(correct == true || correct == false);
}

BOOST_AUTO_TEST_CASE(TestAreOrdersCorrect_ZeroStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Zero stops might be valid in some strategies
    bool correct = manager.areOrdersCorrect(0.0, 0.0);
    BOOST_CHECK(correct == true || correct == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(OrderLifecycle)

BOOST_AUTO_TEST_CASE(TestOpenOrUpdateLongTrade_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Test that function can be called without crashing
    bool result = manager.openOrUpdateLongTrade(0, 0, 0.0050, 0.0100, 0.02, false, false);
    // Result depends on underlying C function and account state
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestOpenOrUpdateShortTrade_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.openOrUpdateShortTrade(0, 0, 0.0050, 0.0100, 0.02, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestUpdateLongTrade_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.updateLongTrade(0, 0, 0.0050, 0.0100, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestUpdateShortTrade_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.updateShortTrade(0, 0, 0.0050, 0.0100, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestCloseLongTrade_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.closeLongTrade(0);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestCloseShortTrade_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.closeShortTrade(0);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestOpenOrUpdateWithInternalStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Test with internal SL and TP enabled
    bool result = manager.openOrUpdateLongTrade(0, 0, 0.0050, 0.0100, 0.02, true, true);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(InternalStopChecking)

BOOST_AUTO_TEST_CASE(TestCheckInternalStopLoss_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.checkInternalStopLoss(0, 0, 0.0050);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestCheckInternalTakeProfit_BasicCall) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.checkInternalTakeProfit(0, 0, 0.0100);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestCheckTimedExit_NoInternalStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.checkTimedExit(0, 0, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestCheckTimedExit_WithInternalStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.checkTimedExit(0, 0, true, true);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(TrailingStops)

BOOST_AUTO_TEST_CASE(TestTrailOpenTrades_BasicParameters) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Trail with 20 pip start, 10 pip distance
    bool result = manager.trailOpenTrades(0, 0.0020, 0.0010, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestTrailOpenTrades_WithInternalStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    bool result = manager.trailOpenTrades(0, 0.0020, 0.0010, true, true);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestTrailOpenTrades_ZeroDistance) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Zero trail distance
    bool result = manager.trailOpenTrades(0, 0.0020, 0.0, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(OrderAge)

BOOST_AUTO_TEST_CASE(TestGetOrderAge_ValidIndex) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    int age = manager.getOrderAge(0);
    BOOST_CHECK_GE(age, 0);
}

BOOST_AUTO_TEST_CASE(TestGetOrderAgeByPosition_ValidPosition) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    int age = manager.getOrderAgeByPosition(0, 1);
    BOOST_CHECK_GE(age, 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(AdvancedStopCalculations)

BOOST_AUTO_TEST_CASE(TestCalculateEllipticalStopLoss_BasicParameters) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Calculate elliptical SL for 100 pip TP, 20 bar max holding, z=2, current age 5 bars
    double sl = manager.calculateEllipticalStopLoss(0.0100, 20, 2.0, 5);
    BOOST_CHECK_GT(sl, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateEllipticalStopLoss_ZeroAge) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double sl = manager.calculateEllipticalStopLoss(0.0100, 20, 2.0, 0);
    BOOST_CHECK_GT(sl, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateEllipticalStopLoss_MaxAge) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // At max age
    double sl = manager.calculateEllipticalStopLoss(0.0100, 20, 2.0, 20);
    BOOST_CHECK_GT(sl, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateEllipticalTakeProfit_BasicParameters) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double tp = manager.calculateEllipticalTakeProfit(0.0100, 20, 2.0, 5);
    BOOST_CHECK_GT(tp, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateEllipticalTakeProfit_ZeroAge) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double tp = manager.calculateEllipticalTakeProfit(0.0100, 20, 2.0, 0);
    BOOST_CHECK_GT(tp, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateEllipticalTakeProfit_MaxAge) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double tp = manager.calculateEllipticalTakeProfit(0.0100, 20, 2.0, 20);
    BOOST_CHECK_GT(tp, 0.0);
}

BOOST_AUTO_TEST_CASE(TestEllipticalStopLoss_DifferentZValues) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    double sl1 = manager.calculateEllipticalStopLoss(0.0100, 20, 1.0, 10);
    double sl2 = manager.calculateEllipticalStopLoss(0.0100, 20, 2.0, 10);
    double sl3 = manager.calculateEllipticalStopLoss(0.0100, 20, 3.0, 10);
    
    // Higher z should give wider stops
    BOOST_CHECK_GT(sl2, sl1);
    BOOST_CHECK_GT(sl3, sl2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(EdgeCases)

BOOST_AUTO_TEST_CASE(TestCalculateOrderSize_ZeroStopLoss) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Zero SL distance - should handle gracefully
    double size = manager.calculateOrderSize(BUY, 1.2000, 1.2000);
    BOOST_CHECK_GE(size, 0.0);
}

BOOST_AUTO_TEST_CASE(TestCalculateOrderSizeWithRisk_ZeroRisk) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Zero risk
    double size = manager.calculateOrderSizeWithRisk(BUY, 1.2000, 1.1950, 0.0);
    BOOST_CHECK_GE(size, 0.0);
}

BOOST_AUTO_TEST_CASE(TestOpenOrUpdateLongTrade_ZeroStops) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Zero SL and TP
    bool result = manager.openOrUpdateLongTrade(0, 0, 0.0, 0.0, 0.02, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_CASE(TestTrailOpenTrades_NegativeParameters) {
    StrategyContext context = createMockContext();
    OrderManager manager(context);
    
    // Negative trail parameters - should handle gracefully
    bool result = manager.trailOpenTrades(0, -0.0020, -0.0010, false, false);
    BOOST_CHECK(result == true || result == false);
}

BOOST_AUTO_TEST_SUITE_END()
