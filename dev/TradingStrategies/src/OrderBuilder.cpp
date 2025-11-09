// OrderBuilder.cpp
// Implementation of OrderBuilder class

#include "OrderBuilder.hpp"
#include <cmath>

namespace trading {

OrderBuilder::OrderBuilder()
    : orderType_(BUY)
    , entryPrice_(0.0)
    , stopLoss_(0.0)
    , takeProfit_(0.0)
    , lotSize_(1.0)
    , magicNumber_(0)
    , priceAdjustment_(0.0)
    , splitStrategy_(SPLIT_NONE)
    , atr_(0.0)
    , dailyHigh_(0.0)
    , dailyLow_(0.0)
    , pivot_(0.0)
    , trendPhase_(0)
    , resistance1_(0.0)
    , support1_(0.0) {
}

OrderBuilder::~OrderBuilder() {
}

void OrderBuilder::reset() {
    orderType_ = BUY;
    entryPrice_ = 0.0;
    stopLoss_ = 0.0;
    takeProfit_ = 0.0;
    lotSize_ = 1.0;
    magicNumber_ = 0;
    priceAdjustment_ = 0.0;
    splitStrategy_ = SPLIT_NONE;
    atr_ = 0.0;
    dailyHigh_ = 0.0;
    dailyLow_ = 0.0;
    pivot_ = 0.0;
    trendPhase_ = 0;
    resistance1_ = 0.0;
    support1_ = 0.0;
}

OrderBuilder& OrderBuilder::setOrderType(OrderType type) {
    orderType_ = type;
    return *this;
}

OrderBuilder& OrderBuilder::setEntryPrice(double price) {
    entryPrice_ = price;
    return *this;
}

OrderBuilder& OrderBuilder::setStopLoss(double stopLoss) {
    stopLoss_ = stopLoss;
    return *this;
}

OrderBuilder& OrderBuilder::setTakeProfit(double takeProfit) {
    takeProfit_ = takeProfit;
    return *this;
}

OrderBuilder& OrderBuilder::setLotSize(double lots) {
    lotSize_ = lots;
    return *this;
}

OrderBuilder& OrderBuilder::setMagicNumber(int magic) {
    magicNumber_ = magic;
    return *this;
}

OrderBuilder& OrderBuilder::setPriceAdjustment(double adjustment) {
    priceAdjustment_ = adjustment;
    return *this;
}

OrderBuilder& OrderBuilder::setSplitStrategy(SplitStrategy strategy) {
    splitStrategy_ = strategy;
    return *this;
}

OrderBuilder& OrderBuilder::setATR(double atr) {
    atr_ = atr;
    return *this;
}

OrderBuilder& OrderBuilder::setDailyHigh(double high) {
    dailyHigh_ = high;
    return *this;
}

OrderBuilder& OrderBuilder::setDailyLow(double low) {
    dailyLow_ = low;
    return *this;
}

OrderBuilder& OrderBuilder::setPivot(double pivot) {
    pivot_ = pivot;
    return *this;
}

OrderBuilder& OrderBuilder::setTrendPhase(int phase) {
    trendPhase_ = phase;
    return *this;
}

OrderBuilder& OrderBuilder::setResistance1(double r1) {
    resistance1_ = r1;
    return *this;
}

OrderBuilder& OrderBuilder::setSupport1(double s1) {
    support1_ = s1;
    return *this;
}

bool OrderBuilder::validate() const {
    // Basic validation
    if (lotSize_ <= 0.0) {
        return false;
    }
    
    // For limit/stop orders, entry price is required
    if ((orderType_ == BUY_LIMIT || orderType_ == SELL_LIMIT ||
         orderType_ == BUY_STOP || orderType_ == SELL_STOP) && 
        entryPrice_ <= 0.0) {
        return false;
    }
    
    // Stop loss validation (optional but must be valid if set)
    if (stopLoss_ < 0.0) {
        return false;
    }
    
    // Take profit validation (optional but must be valid if set)
    if (takeProfit_ < 0.0) {
        return false;
    }
    
    // For buy orders, stop loss must be below entry price
    if (isBuyOrder() && stopLoss_ > 0.0) {
        double entryRef = (entryPrice_ > 0.0) ? entryPrice_ : stopLoss_;
        if (stopLoss_ >= entryRef) {
            return false;
        }
    }
    
    // For sell orders, stop loss must be above entry price
    if (isSellOrder() && stopLoss_ > 0.0) {
        double entryRef = (entryPrice_ > 0.0) ? entryPrice_ : stopLoss_;
        if (stopLoss_ <= entryRef) {
            return false;
        }
    }
    
    return true;
}

int OrderBuilder::build(std::vector<OrderSpec>& orders) const {
    orders.clear();
    
    if (!validate()) {
        return 0;
    }
    
    // Build orders based on splitting strategy
    switch (splitStrategy_) {
        case SPLIT_NONE:
            buildSingleOrder(orders);
            break;
        case SPLIT_SHORT_TERM:
            buildShortTermSplit(orders);
            break;
        case SPLIT_LONG_TERM:
            buildLongTermSplit(orders);
            break;
        case SPLIT_KEYK:
            buildKeyKSplit(orders);
            break;
        case SPLIT_ATR:
            buildATRSplit(orders);
            break;
        case SPLIT_LIMIT:
            buildLimitSplit(orders);
            break;
        case SPLIT_WEEKLY_BEGINNING:
            buildWeeklyBeginningSplit(orders);
            break;
        case SPLIT_WEEKLY_SHORT_TERM:
            buildWeeklyShortTermSplit(orders);
            break;
        case SPLIT_WEEKLY_TRADING:
            buildWeeklyTradingSplit(orders);
            break;
        case SPLIT_4H_SWING:
            build4HSwingSplit(orders);
            break;
        case SPLIT_4H_SWING_100P:
            build4HSwing100PSplit(orders);
            break;
        case SPLIT_4H_SWING_SHELLINGTON:
            build4HSwingShellingtonSplit(orders);
            break;
        case SPLIT_ICHIMOKU_DAILY:
            buildIchimokuDailySplit(orders);
            break;
        case SPLIT_ICHIMOKU_WEEKLY:
            buildIchimokuWeeklySplit(orders);
            break;
        default:
            buildSingleOrder(orders);
            break;
    }
    
    return static_cast<int>(orders.size());
}

// ===== Splitting Logic Implementations =====

void OrderBuilder::buildSingleOrder(std::vector<OrderSpec>& orders) const {
    OrderSpec spec;
    spec.orderType = orderType_;
    spec.openPrice = entryPrice_;
    spec.stopLoss = stopLoss_;
    spec.takeProfit = takeProfit_;
    spec.lotSize = lotSize_;
    spec.magicNumber = magicNumber_;
    orders.push_back(spec);
}

void OrderBuilder::buildShortTermSplit(std::vector<OrderSpec>& orders) const {
    // Short-term: 3 orders with tight profit targets
    // 30% at 1x risk, 40% at 1.5x risk, 30% at 2x risk
    
    double stopDistance = std::fabs(entryPrice_ - stopLoss_);
    
    // Order 1: 30% at 1x risk
    OrderSpec spec1;
    spec1.orderType = orderType_;
    spec1.openPrice = entryPrice_;
    spec1.stopLoss = stopLoss_;
    spec1.lotSize = lotSize_ * 0.3;
    spec1.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec1.takeProfit = entryPrice_ + stopDistance;
    } else {
        spec1.takeProfit = entryPrice_ - stopDistance;
    }
    orders.push_back(spec1);
    
    // Order 2: 40% at 1.5x risk
    OrderSpec spec2;
    spec2.orderType = orderType_;
    spec2.openPrice = entryPrice_;
    spec2.stopLoss = stopLoss_;
    spec2.lotSize = lotSize_ * 0.4;
    spec2.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec2.takeProfit = entryPrice_ + stopDistance * 1.5;
    } else {
        spec2.takeProfit = entryPrice_ - stopDistance * 1.5;
    }
    orders.push_back(spec2);
    
    // Order 3: 30% at 2x risk
    OrderSpec spec3;
    spec3.orderType = orderType_;
    spec3.openPrice = entryPrice_;
    spec3.stopLoss = stopLoss_;
    spec3.lotSize = lotSize_ * 0.3;
    spec3.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec3.takeProfit = entryPrice_ + stopDistance * 2.0;
    } else {
        spec3.takeProfit = entryPrice_ - stopDistance * 2.0;
    }
    orders.push_back(spec3);
}

void OrderBuilder::buildLongTermSplit(std::vector<OrderSpec>& orders) const {
    // Long-term: 2 orders
    // 50% with 2x risk TP, 50% without TP (trailing)
    
    double stopDistance = std::fabs(entryPrice_ - stopLoss_);
    
    // Only enter if pullback is significant (> ATR/2 from daily high for buys)
    bool enterTrade = false;
    if (isBuyOrder() && atr_ > 0.0 && dailyHigh_ > 0.0) {
        double gap = dailyHigh_ - entryPrice_;
        enterTrade = (gap >= atr_ / 2.0);
    } else if (isSellOrder() && atr_ > 0.0 && dailyLow_ > 0.0) {
        double gap = entryPrice_ - dailyLow_;
        enterTrade = (gap >= atr_ / 2.0);
    } else {
        enterTrade = true; // No pullback check needed
    }
    
    if (!enterTrade) {
        return; // No orders generated
    }
    
    // Single order with no TP (simplified from original 2-order split)
    OrderSpec spec;
    spec.orderType = orderType_;
    spec.openPrice = entryPrice_;
    spec.stopLoss = stopLoss_;
    spec.takeProfit = 0.0; // No TP - trailing stop
    spec.lotSize = lotSize_;
    spec.magicNumber = magicNumber_;
    orders.push_back(spec);
}

void OrderBuilder::buildKeyKSplit(std::vector<OrderSpec>& orders) const {
    // KeyK: Simple 1:1 split
    // 50% at 1x risk, 50% at 2x risk
    
    double stopDistance = std::fabs(entryPrice_ - stopLoss_);
    
    // Order 1: 50% at 1x risk
    OrderSpec spec1;
    spec1.orderType = orderType_;
    spec1.openPrice = entryPrice_;
    spec1.stopLoss = stopLoss_;
    spec1.lotSize = lotSize_ * 0.5;
    spec1.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec1.takeProfit = entryPrice_ + stopDistance;
    } else {
        spec1.takeProfit = entryPrice_ - stopDistance;
    }
    orders.push_back(spec1);
    
    // Order 2: 50% at 2x risk
    OrderSpec spec2;
    spec2.orderType = orderType_;
    spec2.openPrice = entryPrice_;
    spec2.stopLoss = stopLoss_;
    spec2.lotSize = lotSize_ * 0.5;
    spec2.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec2.takeProfit = entryPrice_ + stopDistance * 2.0;
    } else {
        spec2.takeProfit = entryPrice_ - stopDistance * 2.0;
    }
    orders.push_back(spec2);
}

void OrderBuilder::buildATRSplit(std::vector<OrderSpec>& orders) const {
    // ATR-based splitting: 2 orders
    // 60% at 1x ATR, 40% at 2x ATR
    
    if (atr_ <= 0.0) {
        // Fallback to single order if ATR not provided
        buildSingleOrder(orders);
        return;
    }
    
    // Order 1: 60% at 1x ATR
    OrderSpec spec1;
    spec1.orderType = orderType_;
    spec1.openPrice = entryPrice_;
    spec1.stopLoss = stopLoss_;
    spec1.lotSize = lotSize_ * 0.6;
    spec1.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec1.takeProfit = entryPrice_ + atr_;
    } else {
        spec1.takeProfit = entryPrice_ - atr_;
    }
    orders.push_back(spec1);
    
    // Order 2: 40% at 2x ATR
    OrderSpec spec2;
    spec2.orderType = orderType_;
    spec2.openPrice = entryPrice_;
    spec2.stopLoss = stopLoss_;
    spec2.lotSize = lotSize_ * 0.4;
    spec2.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec2.takeProfit = entryPrice_ + atr_ * 2.0;
    } else {
        spec2.takeProfit = entryPrice_ - atr_ * 2.0;
    }
    orders.push_back(spec2);
}

void OrderBuilder::buildLimitSplit(std::vector<OrderSpec>& orders) const {
    // Limit order splitting based on Fibonacci retracements
    // Places limit orders at 38.2% and 50% retracement levels
    
    if (dailyHigh_ <= 0.0 || dailyLow_ <= 0.0) {
        return; // Need daily high/low for Fibonacci calculations
    }
    
    // Trend phases (defined in legacy code):
    // 1 = BEGINNING_UP_PHASE, 2 = MIDDLE_UP_PHASE
    // -1 = BEGINNING_DOWN_PHASE, -2 = MIDDLE_DOWN_PHASE
    
    if (isBuyOrder()) {
        if (trendPhase_ == 1) { // BEGINNING_UP_PHASE
            // 38.2% retracement
            double openPrice1 = dailyHigh_ - (dailyHigh_ - dailyLow_) * 0.382 + priceAdjustment_;
            OrderSpec spec1;
            spec1.orderType = BUY_LIMIT;
            spec1.openPrice = openPrice1;
            spec1.stopLoss = stopLoss_;
            spec1.takeProfit = takeProfit_;
            spec1.lotSize = lotSize_ * 0.5;
            spec1.magicNumber = magicNumber_;
            orders.push_back(spec1);
            
            // 50% retracement
            double openPrice2 = dailyHigh_ - (dailyHigh_ - dailyLow_) * 0.5 + priceAdjustment_;
            OrderSpec spec2;
            spec2.orderType = BUY_LIMIT;
            spec2.openPrice = openPrice2;
            spec2.stopLoss = stopLoss_;
            spec2.takeProfit = takeProfit_;
            spec2.lotSize = lotSize_;
            spec2.magicNumber = magicNumber_;
            orders.push_back(spec2);
        } else if (trendPhase_ == 2 && pivot_ > 0.0) { // MIDDLE_UP_PHASE
            // Pivot level
            OrderSpec spec;
            spec.orderType = BUY_LIMIT;
            spec.openPrice = pivot_ + priceAdjustment_;
            spec.stopLoss = stopLoss_;
            spec.takeProfit = takeProfit_;
            spec.lotSize = lotSize_;
            spec.magicNumber = magicNumber_;
            orders.push_back(spec);
        }
    } else if (isSellOrder()) {
        if (trendPhase_ == -1) { // BEGINNING_DOWN_PHASE
            // 38.2% retracement
            double openPrice1 = dailyLow_ + (dailyHigh_ - dailyLow_) * 0.382 - priceAdjustment_;
            OrderSpec spec1;
            spec1.orderType = SELL_LIMIT;
            spec1.openPrice = openPrice1;
            spec1.stopLoss = stopLoss_;
            spec1.takeProfit = takeProfit_;
            spec1.lotSize = lotSize_ * 0.5;
            spec1.magicNumber = magicNumber_;
            orders.push_back(spec1);
            
            // 50% retracement
            double openPrice2 = dailyLow_ + (dailyHigh_ - dailyLow_) * 0.5 - priceAdjustment_;
            OrderSpec spec2;
            spec2.orderType = SELL_LIMIT;
            spec2.openPrice = openPrice2;
            spec2.stopLoss = stopLoss_;
            spec2.takeProfit = takeProfit_;
            spec2.lotSize = lotSize_;
            spec2.magicNumber = magicNumber_;
            orders.push_back(spec2);
        } else if (trendPhase_ == -2 && pivot_ > 0.0) { // MIDDLE_DOWN_PHASE
            // Pivot level
            OrderSpec spec;
            spec.orderType = SELL_LIMIT;
            spec.openPrice = pivot_ - priceAdjustment_;
            spec.stopLoss = stopLoss_;
            spec.takeProfit = takeProfit_;
            spec.lotSize = lotSize_;
            spec.magicNumber = magicNumber_;
            orders.push_back(spec);
        }
    }
}

void OrderBuilder::buildWeeklyBeginningSplit(std::vector<OrderSpec>& orders) const {
    // Weekly beginning: Single order with no TP if price below/above R1/S1
    
    if (isBuyOrder() && resistance1_ > 0.0 && entryPrice_ <= resistance1_) {
        OrderSpec spec;
        spec.orderType = orderType_;
        spec.openPrice = entryPrice_;
        spec.stopLoss = stopLoss_;
        spec.takeProfit = 0.0; // No TP
        spec.lotSize = lotSize_;
        spec.magicNumber = magicNumber_;
        orders.push_back(spec);
    } else if (isSellOrder() && support1_ > 0.0 && entryPrice_ >= support1_) {
        OrderSpec spec;
        spec.orderType = orderType_;
        spec.openPrice = entryPrice_;
        spec.stopLoss = stopLoss_;
        spec.takeProfit = 0.0; // No TP
        spec.lotSize = lotSize_;
        spec.magicNumber = magicNumber_;
        orders.push_back(spec);
    }
}

void OrderBuilder::buildWeeklyShortTermSplit(std::vector<OrderSpec>& orders) const {
    // Weekly short-term: Same as short-term split
    buildShortTermSplit(orders);
}

void OrderBuilder::buildWeeklyTradingSplit(std::vector<OrderSpec>& orders) const {
    // Weekly trading: Same as short-term split
    buildShortTermSplit(orders);
}

void OrderBuilder::build4HSwingSplit(std::vector<OrderSpec>& orders) const {
    // 4H swing: ATR-based splitting with multiple targets
    // 30% at 0.5x ATR, 30% at 1x ATR, 40% at 2x ATR
    
    if (atr_ <= 0.0) {
        buildSingleOrder(orders);
        return;
    }
    
    // Order 1: 30% at 0.5x ATR
    OrderSpec spec1;
    spec1.orderType = orderType_;
    spec1.openPrice = entryPrice_;
    spec1.stopLoss = stopLoss_;
    spec1.lotSize = lotSize_ * 0.3;
    spec1.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec1.takeProfit = entryPrice_ + atr_ * 0.5;
    } else {
        spec1.takeProfit = entryPrice_ - atr_ * 0.5;
    }
    orders.push_back(spec1);
    
    // Order 2: 30% at 1x ATR
    OrderSpec spec2;
    spec2.orderType = orderType_;
    spec2.openPrice = entryPrice_;
    spec2.stopLoss = stopLoss_;
    spec2.lotSize = lotSize_ * 0.3;
    spec2.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec2.takeProfit = entryPrice_ + atr_;
    } else {
        spec2.takeProfit = entryPrice_ - atr_;
    }
    orders.push_back(spec2);
    
    // Order 3: 40% at 2x ATR
    OrderSpec spec3;
    spec3.orderType = orderType_;
    spec3.openPrice = entryPrice_;
    spec3.stopLoss = stopLoss_;
    spec3.lotSize = lotSize_ * 0.4;
    spec3.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec3.takeProfit = entryPrice_ + atr_ * 2.0;
    } else {
        spec3.takeProfit = entryPrice_ - atr_ * 2.0;
    }
    orders.push_back(spec3);
}

void OrderBuilder::build4HSwing100PSplit(std::vector<OrderSpec>& orders) const {
    // 4H swing 100%: More aggressive ATR-based splitting
    // 25% at 0.5x ATR, 25% at 1x ATR, 25% at 2x ATR, 25% at 3x ATR
    
    if (atr_ <= 0.0) {
        buildSingleOrder(orders);
        return;
    }
    
    double lotSizes[] = {0.25, 0.25, 0.25, 0.25};
    double multipliers[] = {0.5, 1.0, 2.0, 3.0};
    
    for (int i = 0; i < 4; ++i) {
        OrderSpec spec;
        spec.orderType = orderType_;
        spec.openPrice = entryPrice_;
        spec.stopLoss = stopLoss_;
        spec.lotSize = lotSize_ * lotSizes[i];
        spec.magicNumber = magicNumber_;
        
        if (isBuyOrder()) {
            spec.takeProfit = entryPrice_ + atr_ * multipliers[i];
        } else {
            spec.takeProfit = entryPrice_ - atr_ * multipliers[i];
        }
        orders.push_back(spec);
    }
}

void OrderBuilder::build4HSwingShellingtonSplit(std::vector<OrderSpec>& orders) const {
    // 4H swing Shellington: Similar to 4H swing but with tighter targets
    // 40% at 0.5x ATR, 30% at 1x ATR, 30% at 1.5x ATR
    
    if (atr_ <= 0.0) {
        buildSingleOrder(orders);
        return;
    }
    
    double lotSizes[] = {0.4, 0.3, 0.3};
    double multipliers[] = {0.5, 1.0, 1.5};
    
    for (int i = 0; i < 3; ++i) {
        OrderSpec spec;
        spec.orderType = orderType_;
        spec.openPrice = entryPrice_;
        spec.stopLoss = stopLoss_;
        spec.lotSize = lotSize_ * lotSizes[i];
        spec.magicNumber = magicNumber_;
        
        if (isBuyOrder()) {
            spec.takeProfit = entryPrice_ + atr_ * multipliers[i];
        } else {
            spec.takeProfit = entryPrice_ - atr_ * multipliers[i];
        }
        orders.push_back(spec);
    }
}

void OrderBuilder::buildIchimokuDailySplit(std::vector<OrderSpec>& orders) const {
    // Ichimoku daily: ATR-based with 2 targets
    // 50% at 1x ATR, 50% at 2x ATR
    
    if (atr_ <= 0.0) {
        buildSingleOrder(orders);
        return;
    }
    
    // Order 1: 50% at 1x ATR
    OrderSpec spec1;
    spec1.orderType = orderType_;
    spec1.openPrice = entryPrice_;
    spec1.stopLoss = stopLoss_;
    spec1.lotSize = lotSize_ * 0.5;
    spec1.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec1.takeProfit = entryPrice_ + atr_;
    } else {
        spec1.takeProfit = entryPrice_ - atr_;
    }
    orders.push_back(spec1);
    
    // Order 2: 50% at 2x ATR
    OrderSpec spec2;
    spec2.orderType = orderType_;
    spec2.openPrice = entryPrice_;
    spec2.stopLoss = stopLoss_;
    spec2.lotSize = lotSize_ * 0.5;
    spec2.magicNumber = magicNumber_;
    
    if (isBuyOrder()) {
        spec2.takeProfit = entryPrice_ + atr_ * 2.0;
    } else {
        spec2.takeProfit = entryPrice_ - atr_ * 2.0;
    }
    orders.push_back(spec2);
}

void OrderBuilder::buildIchimokuWeeklySplit(std::vector<OrderSpec>& orders) const {
    // Ichimoku weekly: Same as Ichimoku daily
    buildIchimokuDailySplit(orders);
}

// ===== Helper Methods =====

bool OrderBuilder::isBuyOrder() const {
    return (orderType_ == BUY || orderType_ == BUY_LIMIT || orderType_ == BUY_STOP);
}

bool OrderBuilder::isSellOrder() const {
    return (orderType_ == SELL || orderType_ == SELL_LIMIT || orderType_ == SELL_STOP);
}

double OrderBuilder::calculateFibonacciLevel(double level) const {
    if (dailyHigh_ <= 0.0 || dailyLow_ <= 0.0) {
        return 0.0;
    }
    
    double range = dailyHigh_ - dailyLow_;
    if (isBuyOrder()) {
        return dailyHigh_ - range * level;
    } else {
        return dailyLow_ + range * level;
    }
}

} // namespace trading
