// AutoBBSStrategy.hpp
// Dispatcher strategy for AutoBBS system - routes to sub-strategies

#ifndef AUTOBBSSTRATEGY_HPP
#define AUTOBBSSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// Main AutoBBS dispatcher strategy
/// Routes to specific sub-strategies based on strategy_mode setting
/// This is a dispatcher that delegates to ~30+ trend-based strategies
class AutoBBSStrategy : public BaseStrategy {
public:
    AutoBBSStrategy();
    
    std::string getName() const;
    
protected:
    /// Execute the AutoBBS dispatcher
    /// Routes to appropriate sub-strategy based on configuration
    /// @param context Strategy execution context containing strategy_mode setting
    /// @param indicators Loaded indicators (passed to sub-strategies)
    /// @return Strategy result from selected sub-strategy
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators);
    
    /// Update results structure with strategy output
    void updateResults(const StrategyContext& context, const StrategyResult& result);
    
    /// This strategy requires indicators (for sub-strategies)
    /// @return true
    bool requiresIndicators() const;
};

} // namespace trading

#endif // AUTOBBSSTRATEGY_HPP
