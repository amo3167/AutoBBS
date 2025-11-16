// TakeOverStrategy.hpp
// Strategy for taking over existing positions

#ifndef TAKEOVERSTRATEGY_HPP
#define TAKEOVERSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// Strategy for managing takeover of existing positions
/// Analyzes and manages positions opened by other strategies or manual trading
class TakeOverStrategy : public BaseStrategy {
public:
    TakeOverStrategy();
    
    std::string getName() const;
    
protected:
    /// Execute the takeover strategy
    /// Evaluates existing positions and generates management signals
    /// @param context Strategy execution context
    /// @param indicators Loaded indicators for analysis
    /// @return Strategy result with success/failure code
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators);
    
    /// Update results structure with strategy output
    void updateResults(const StrategyContext& context, const StrategyResult& result);
    
    /// This strategy requires indicators for position evaluation
    /// @return true
    bool requiresIndicators() const;
};

} // namespace trading

#endif // TAKEOVERSTRATEGY_HPP
