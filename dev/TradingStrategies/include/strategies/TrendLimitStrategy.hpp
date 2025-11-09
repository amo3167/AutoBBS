// TrendLimitStrategy.hpp
// Trend following strategy with limit order management

#ifndef TRENDLIMITSTRATEGY_HPP
#define TRENDLIMITSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// Trend following strategy with limit order entry
/// Identifies trends and places limit orders for optimal entry prices
class TrendLimitStrategy : public BaseStrategy {
public:
    TrendLimitStrategy();
    
    std::string getName() const;
    
protected:
    /// Execute the trend limit strategy
    /// Analyzes trends and generates limit order signals
    /// @param context Strategy execution context
    /// @param indicators Loaded indicators for trend analysis
    /// @return Strategy result with success/failure code
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators);
    
    /// Update results structure with strategy output
    void updateResults(const StrategyContext& context, const StrategyResult& result);
    
    /// This strategy requires indicators for trend identification
    /// @return true
    bool requiresIndicators() const;
};

} // namespace trading

#endif // TRENDLIMITSTRATEGY_HPP
