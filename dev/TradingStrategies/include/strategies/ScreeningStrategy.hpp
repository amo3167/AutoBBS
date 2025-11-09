// ScreeningStrategy.hpp
// Strategy for market screening and analysis

#ifndef SCREENINGSTRATEGY_HPP
#define SCREENINGSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// Strategy for screening markets and identifying trading opportunities
/// Performs multi-timeframe analysis and generates screening results
class ScreeningStrategy : public BaseStrategy {
public:
    ScreeningStrategy();
    
    std::string getName() const;
    
protected:
    /// Execute the screening strategy
    /// Analyzes market conditions across multiple timeframes
    /// @param context Strategy execution context
    /// @param indicators Loaded indicators for screening
    /// @return Strategy result with success/failure code
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators);
    
    /// Update results structure with strategy output
    void updateResults(const StrategyContext& context, const StrategyResult& result);
    
    /// This strategy requires indicators for market screening
    /// @return true
    bool requiresIndicators() const;
};

} // namespace trading

#endif // SCREENINGSTRATEGY_HPP
