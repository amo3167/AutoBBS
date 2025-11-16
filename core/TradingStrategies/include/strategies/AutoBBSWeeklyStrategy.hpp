// AutoBBSWeeklyStrategy.hpp
// Weekly version of AutoBBS dispatcher strategy

#ifndef AUTOBBSWEEKLYSTRATEGY_HPP
#define AUTOBBSWEEKLYSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// Weekly AutoBBS dispatcher strategy
/// Similar to AutoBBSStrategy but operates on weekly timeframes
/// Routes to weekly-specific sub-strategies based on configuration
class AutoBBSWeeklyStrategy : public BaseStrategy {
public:
    AutoBBSWeeklyStrategy();
    
    std::string getName() const;
    
protected:
    /// Execute the weekly AutoBBS dispatcher
    /// Routes to appropriate weekly sub-strategy
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

#endif // AUTOBBSWEEKLYSTRATEGY_HPP
