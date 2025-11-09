// OrderBuilderTests.cpp
// Unit tests for OrderBuilder class

#include "OrderBuilder.hpp"
#include <boost/test/unit_test.hpp>

using namespace trading;

BOOST_AUTO_TEST_SUITE(OrderBuilderTests)

// ===== Basic Functionality Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_DefaultConstructor) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    // Default builder should fail validation (no entry price for limit orders)
    builder.setOrderType(BUY_LIMIT);
    BOOST_CHECK(!builder.validate());
    
    // Market order with default values should validate
    builder.reset();
    builder.setOrderType(BUY);
    BOOST_CHECK(builder.validate());
}

BOOST_AUTO_TEST_CASE(OrderBuilder_Reset) {
    OrderBuilder builder;
    builder.setOrderType(SELL)
           .setEntryPrice(1.2000)
           .setStopLoss(1.2100)
           .setTakeProfit(1.1900)
           .setLotSize(2.0);
    
    builder.reset();
    std::vector<OrderSpec> orders;
    
    // After reset, should be back to defaults
    builder.setOrderType(BUY);
    BOOST_CHECK(builder.validate());
}

BOOST_AUTO_TEST_CASE(OrderBuilder_FluentInterface) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    // Test method chaining
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setTakeProfit(1.2100)
                       .setLotSize(1.0)
                       .setSplitStrategy(SPLIT_NONE)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_CHECK_EQUAL(orders.size(), 1u);
}

// ===== Validation Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_ValidateNegativeLotSize) {
    OrderBuilder builder;
    builder.setOrderType(BUY).setLotSize(-1.0);
    BOOST_CHECK(!builder.validate());
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ValidateZeroLotSize) {
    OrderBuilder builder;
    builder.setOrderType(BUY).setLotSize(0.0);
    BOOST_CHECK(!builder.validate());
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ValidateLimitOrderNeedsPrice) {
    OrderBuilder builder;
    builder.setOrderType(BUY_LIMIT).setLotSize(1.0);
    BOOST_CHECK(!builder.validate()); // No entry price
    
    builder.setEntryPrice(1.2000);
    BOOST_CHECK(builder.validate()); // Now valid
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ValidateStopOrderNeedsPrice) {
    OrderBuilder builder;
    builder.setOrderType(BUY_STOP).setLotSize(1.0);
    BOOST_CHECK(!builder.validate()); // No entry price
    
    builder.setEntryPrice(1.2000);
    BOOST_CHECK(builder.validate()); // Now valid
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ValidateBuyStopLossBelowEntry) {
    OrderBuilder builder;
    builder.setOrderType(BUY)
           .setEntryPrice(1.2000)
           .setStopLoss(1.2100) // Above entry - invalid
           .setLotSize(1.0);
    
    BOOST_CHECK(!builder.validate());
    
    builder.setStopLoss(1.1900); // Below entry - valid
    BOOST_CHECK(builder.validate());
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ValidateSellStopLossAboveEntry) {
    OrderBuilder builder;
    builder.setOrderType(SELL)
           .setEntryPrice(1.2000)
           .setStopLoss(1.1900) // Below entry - invalid
           .setLotSize(1.0);
    
    BOOST_CHECK(!builder.validate());
    
    builder.setStopLoss(1.2100); // Above entry - valid
    BOOST_CHECK(builder.validate());
}

// ===== Single Order Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_SingleBuyOrder) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setTakeProfit(1.2100)
                       .setLotSize(1.5)
                       .setMagicNumber(12345)
                       .setSplitStrategy(SPLIT_NONE)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_REQUIRE_EQUAL(orders.size(), 1u);
    
    BOOST_CHECK_EQUAL(orders[0].orderType, BUY);
    BOOST_CHECK_CLOSE(orders[0].openPrice, 1.2000, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].stopLoss, 1.1950, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.2100, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].lotSize, 1.5, 0.0001);
    BOOST_CHECK_EQUAL(orders[0].magicNumber, 12345);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_SingleSellOrder) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(SELL)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.2050)
                       .setTakeProfit(1.1900)
                       .setLotSize(2.0)
                       .setSplitStrategy(SPLIT_NONE)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_REQUIRE_EQUAL(orders.size(), 1u);
    
    BOOST_CHECK_EQUAL(orders[0].orderType, SELL);
    BOOST_CHECK_CLOSE(orders[0].openPrice, 1.2000, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].stopLoss, 1.2050, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.1900, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].lotSize, 2.0, 0.0001);
}

// ===== Short-Term Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_ShortTermSplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950) // 50 pips SL
                       .setLotSize(1.0)
                       .setSplitStrategy(SPLIT_SHORT_TERM)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 3);
    BOOST_REQUIRE_EQUAL(orders.size(), 3u);
    
    // Order 1: 30% at 1x risk (50 pips TP)
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.2050, 0.0001);
    
    // Order 2: 40% at 1.5x risk (75 pips TP)
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.4, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].takeProfit, 1.2075, 0.0001);
    
    // Order 3: 30% at 2x risk (100 pips TP)
    BOOST_CHECK_CLOSE(orders[2].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[2].takeProfit, 1.2100, 0.0001);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ShortTermSplit_Sell) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(SELL)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.2050) // 50 pips SL
                       .setLotSize(1.0)
                       .setSplitStrategy(SPLIT_SHORT_TERM)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 3);
    BOOST_REQUIRE_EQUAL(orders.size(), 3u);
    
    // Order 1: 30% at 1x risk (50 pips TP)
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.1950, 0.0001);
    
    // Order 2: 40% at 1.5x risk (75 pips TP)
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.4, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].takeProfit, 1.1925, 0.0001);
    
    // Order 3: 30% at 2x risk (100 pips TP)
    BOOST_CHECK_CLOSE(orders[2].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[2].takeProfit, 1.1900, 0.0001);
}

// ===== Long-Term Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_LongTermSplit_WithPullback) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0100) // 100 pips ATR
                       .setDailyHigh(1.2060) // 60 pips pullback (> ATR/2 = 50 pips)
                       .setSplitStrategy(SPLIT_LONG_TERM)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_REQUIRE_EQUAL(orders.size(), 1u);
    
    // Single order with no TP (trailing stop)
    BOOST_CHECK_CLOSE(orders[0].lotSize, 1.0, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 0.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_LongTermSplit_NoPullback) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0100) // 100 pips ATR
                       .setDailyHigh(1.2040) // 40 pips pullback (< ATR/2 = 50 pips)
                       .setSplitStrategy(SPLIT_LONG_TERM)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 0); // No orders generated
    BOOST_CHECK_EQUAL(orders.size(), 0u);
}

// ===== KeyK Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_KeyKSplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1900) // 100 pips SL
                       .setLotSize(1.0)
                       .setSplitStrategy(SPLIT_KEYK)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_REQUIRE_EQUAL(orders.size(), 2u);
    
    // Order 1: 50% at 1x risk
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.5, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.2100, 0.0001);
    
    // Order 2: 50% at 2x risk
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.5, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].takeProfit, 1.2200, 0.0001);
}

// ===== ATR-Based Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_ATRSplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0080) // 80 pips ATR
                       .setSplitStrategy(SPLIT_ATR)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_REQUIRE_EQUAL(orders.size(), 2u);
    
    // Order 1: 60% at 1x ATR
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.6, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.2080, 0.0001);
    
    // Order 2: 40% at 2x ATR
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.4, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].takeProfit, 1.2160, 0.0001);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_ATRSplit_NoATR) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setSplitStrategy(SPLIT_ATR)
                       .build(orders);
    
    // Should fallback to single order if ATR not provided
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_REQUIRE_EQUAL(orders.size(), 1u);
}

// ===== Limit Order Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_LimitSplit_BuyBeginningUpPhase) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY_LIMIT)
                       .setStopLoss(1.1850)
                       .setTakeProfit(1.2100)
                       .setLotSize(1.0)
                       .setDailyHigh(1.2000)
                       .setDailyLow(1.1900)
                       .setTrendPhase(1) // BEGINNING_UP_PHASE
                       .setPriceAdjustment(0.0010)
                       .setSplitStrategy(SPLIT_LIMIT)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_REQUIRE_EQUAL(orders.size(), 2u);
    
    // Order 1: 50% at 38.2% retracement
    // 1.2000 - (1.2000 - 1.1900) * 0.382 + 0.0010 = 1.1963
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.5, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].openPrice, 1.1963, 0.0001);
    
    // Order 2: 100% at 50% retracement
    // 1.2000 - (1.2000 - 1.1900) * 0.5 + 0.0010 = 1.1960
    BOOST_CHECK_CLOSE(orders[1].lotSize, 1.0, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].openPrice, 1.1960, 0.0001);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_LimitSplit_BuyMiddleUpPhase) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY_LIMIT)
                       .setStopLoss(1.1850)
                       .setTakeProfit(1.2100)
                       .setLotSize(1.0)
                       .setPivot(1.1950)
                       .setTrendPhase(2) // MIDDLE_UP_PHASE
                       .setPriceAdjustment(0.0010)
                       .setSplitStrategy(SPLIT_LIMIT)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_REQUIRE_EQUAL(orders.size(), 1u);
    
    // Single order at pivot + adjustment
    BOOST_CHECK_CLOSE(orders[0].openPrice, 1.1960, 0.0001);
}

// ===== Weekly Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_WeeklyBeginningSplit_Buy) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.1980)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setResistance1(1.2000) // Entry below R1
                       .setSplitStrategy(SPLIT_WEEKLY_BEGINNING)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 1);
    BOOST_REQUIRE_EQUAL(orders.size(), 1u);
    
    // Single order with no TP
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 0.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_WeeklyBeginningSplit_BuyAboveR1) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2020)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setResistance1(1.2000) // Entry above R1
                       .setSplitStrategy(SPLIT_WEEKLY_BEGINNING)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 0); // No orders generated
}

// ===== 4H Swing Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_4HSwingSplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0100) // 100 pips ATR
                       .setSplitStrategy(SPLIT_4H_SWING)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 3);
    BOOST_REQUIRE_EQUAL(orders.size(), 3u);
    
    // Order 1: 30% at 0.5x ATR (50 pips)
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.2050, 0.0001);
    
    // Order 2: 30% at 1x ATR (100 pips)
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].takeProfit, 1.2100, 0.0001);
    
    // Order 3: 40% at 2x ATR (200 pips)
    BOOST_CHECK_CLOSE(orders[2].lotSize, 0.4, 0.0001);
    BOOST_CHECK_CLOSE(orders[2].takeProfit, 1.2200, 0.0001);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_4HSwing100PSplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0100)
                       .setSplitStrategy(SPLIT_4H_SWING_100P)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 4);
    BOOST_REQUIRE_EQUAL(orders.size(), 4u);
    
    // Equal 25% splits at 0.5x, 1x, 2x, 3x ATR
    for (size_t i = 0; i < 4; ++i) {
        BOOST_CHECK_CLOSE(orders[i].lotSize, 0.25, 0.0001);
    }
}

BOOST_AUTO_TEST_CASE(OrderBuilder_4HSwingShellingtonSplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0100)
                       .setSplitStrategy(SPLIT_4H_SWING_SHELLINGTON)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 3);
    BOOST_REQUIRE_EQUAL(orders.size(), 3u);
    
    // 40%, 30%, 30% splits
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.4, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.3, 0.0001);
    BOOST_CHECK_CLOSE(orders[2].lotSize, 0.3, 0.0001);
}

// ===== Ichimoku Split Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_IchimokuDailySplit) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY)
                       .setEntryPrice(1.2000)
                       .setStopLoss(1.1950)
                       .setLotSize(1.0)
                       .setATR(0.0080)
                       .setSplitStrategy(SPLIT_ICHIMOKU_DAILY)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_REQUIRE_EQUAL(orders.size(), 2u);
    
    // 50% at 1x ATR, 50% at 2x ATR
    BOOST_CHECK_CLOSE(orders[0].lotSize, 0.5, 0.0001);
    BOOST_CHECK_CLOSE(orders[0].takeProfit, 1.2080, 0.0001);
    
    BOOST_CHECK_CLOSE(orders[1].lotSize, 0.5, 0.0001);
    BOOST_CHECK_CLOSE(orders[1].takeProfit, 1.2160, 0.0001);
}

// ===== Error Handling Tests =====

BOOST_AUTO_TEST_CASE(OrderBuilder_BuildWithInvalidParams) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    // Invalid lot size
    int count = builder.setOrderType(BUY)
                       .setLotSize(-1.0)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 0);
    BOOST_CHECK_EQUAL(orders.size(), 0u);
}

BOOST_AUTO_TEST_CASE(OrderBuilder_LimitSplitWithoutDailyHighLow) {
    OrderBuilder builder;
    std::vector<OrderSpec> orders;
    
    int count = builder.setOrderType(BUY_LIMIT)
                       .setStopLoss(1.1950)
                       .setTakeProfit(1.2100)
                       .setLotSize(1.0)
                       .setTrendPhase(1)
                       .setSplitStrategy(SPLIT_LIMIT)
                       .build(orders);
    
    BOOST_CHECK_EQUAL(count, 0); // Can't calculate Fibonacci without high/low
}

BOOST_AUTO_TEST_SUITE_END()
