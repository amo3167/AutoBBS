// OrderBuilder.hpp
// Builder class for constructing and splitting trading orders

#ifndef ORDERBUILDER_HPP
#define ORDERBUILDER_HPP

#include "StrategyTypes.h"
#include <vector>

namespace trading {

/// Order splitting strategy types
enum SplitStrategy {
    SPLIT_NONE,              ///< Single order, no splitting
    SPLIT_SHORT_TERM,        ///< Short-term splitting (2-3 orders, tight TP)
    SPLIT_LONG_TERM,         ///< Long-term splitting (2 orders, one with TP, one trailing)
    SPLIT_KEYK,              ///< KeyK strategy splitting
    SPLIT_ATR,               ///< ATR-based splitting
    SPLIT_LIMIT,             ///< Limit order splitting (Fibonacci levels)
    SPLIT_WEEKLY_BEGINNING,  ///< Weekly beginning splitting
    SPLIT_WEEKLY_SHORT_TERM, ///< Weekly short-term splitting
    SPLIT_WEEKLY_TRADING,    ///< Weekly trading splitting
    SPLIT_4H_SWING,          ///< 4H swing splitting
    SPLIT_4H_SWING_100P,     ///< 4H swing 100% splitting
    SPLIT_4H_SWING_SHELLINGTON, ///< 4H swing Shellington splitting
    SPLIT_ICHIMOKU_DAILY,    ///< Ichimoku daily splitting
    SPLIT_ICHIMOKU_WEEKLY    ///< Ichimoku weekly splitting
};

/// Order specification for a single order within a split
struct OrderSpec {
    OrderType orderType;       ///< BUY, SELL, BUY_LIMIT, SELL_LIMIT, BUY_STOP, SELL_STOP
    double openPrice;          ///< Entry price (0 = market order)
    double stopLoss;           ///< Stop loss price
    double takeProfit;         ///< Take profit price (0 = no TP)
    double lotSize;            ///< Lot size (percentage of total risk)
    int magicNumber;           ///< Magic number for order identification
    
    OrderSpec()
        : orderType(BUY)
        , openPrice(0.0)
        , stopLoss(0.0)
        , takeProfit(0.0)
        , lotSize(1.0)
        , magicNumber(0) {
    }
};

/// Builder for constructing trading orders with splitting logic
class OrderBuilder {
public:
    OrderBuilder();
    ~OrderBuilder();
    
    /// Reset builder to initial state
    void reset();
    
    /// Set the order type (BUY, SELL, etc.)
    OrderBuilder& setOrderType(OrderType type);
    
    /// Set entry price (0 for market orders)
    OrderBuilder& setEntryPrice(double price);
    
    /// Set stop loss price
    OrderBuilder& setStopLoss(double stopLoss);
    
    /// Set take profit price (0 for no TP)
    OrderBuilder& setTakeProfit(double takeProfit);
    
    /// Set total lot size (before splitting)
    OrderBuilder& setLotSize(double lots);
    
    /// Set magic number for order identification
    OrderBuilder& setMagicNumber(int magic);
    
    /// Set price adjustment (for limit orders)
    OrderBuilder& setPriceAdjustment(double adjustment);
    
    /// Set splitting strategy
    OrderBuilder& setSplitStrategy(SplitStrategy strategy);
    
    /// Set ATR value (for ATR-based splitting)
    OrderBuilder& setATR(double atr);
    
    /// Set daily high/low (for Fibonacci splitting)
    OrderBuilder& setDailyHigh(double high);
    OrderBuilder& setDailyLow(double low);
    
    /// Set pivot point (for pivot-based splitting)
    OrderBuilder& setPivot(double pivot);
    
    /// Set daily trend phase (for limit order splitting)
    OrderBuilder& setTrendPhase(int phase);
    
    /// Set resistance/support levels (for weekly splitting)
    OrderBuilder& setResistance1(double r1);
    OrderBuilder& setSupport1(double s1);
    
    /// Validate order parameters
    /// @return true if all required parameters are valid
    bool validate() const;
    
    /// Build order specifications based on splitting strategy
    /// @param[out] orders Vector to receive the generated order specs
    /// @return Number of orders generated (0 on error)
    int build(std::vector<OrderSpec>& orders) const;
    
private:
    // Order parameters
    OrderType orderType_;
    double entryPrice_;
    double stopLoss_;
    double takeProfit_;
    double lotSize_;
    int magicNumber_;
    double priceAdjustment_;
    
    // Splitting parameters
    SplitStrategy splitStrategy_;
    double atr_;
    double dailyHigh_;
    double dailyLow_;
    double pivot_;
    int trendPhase_;
    double resistance1_;
    double support1_;
    
    // Splitting logic implementations
    void buildSingleOrder(std::vector<OrderSpec>& orders) const;
    void buildShortTermSplit(std::vector<OrderSpec>& orders) const;
    void buildLongTermSplit(std::vector<OrderSpec>& orders) const;
    void buildKeyKSplit(std::vector<OrderSpec>& orders) const;
    void buildATRSplit(std::vector<OrderSpec>& orders) const;
    void buildLimitSplit(std::vector<OrderSpec>& orders) const;
    void buildWeeklyBeginningSplit(std::vector<OrderSpec>& orders) const;
    void buildWeeklyShortTermSplit(std::vector<OrderSpec>& orders) const;
    void buildWeeklyTradingSplit(std::vector<OrderSpec>& orders) const;
    void build4HSwingSplit(std::vector<OrderSpec>& orders) const;
    void build4HSwing100PSplit(std::vector<OrderSpec>& orders) const;
    void build4HSwingShellingtonSplit(std::vector<OrderSpec>& orders) const;
    void buildIchimokuDailySplit(std::vector<OrderSpec>& orders) const;
    void buildIchimokuWeeklySplit(std::vector<OrderSpec>& orders) const;
    
    // Helper methods
    bool isBuyOrder() const;
    bool isSellOrder() const;
    double calculateFibonacciLevel(double level) const;
    
    // Non-copyable (C++03 idiom)
    OrderBuilder(const OrderBuilder&);
    OrderBuilder& operator=(const OrderBuilder&);
};

} // namespace trading

#endif // ORDERBUILDER_HPP
