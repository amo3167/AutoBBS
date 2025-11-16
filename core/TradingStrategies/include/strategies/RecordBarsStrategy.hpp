// RecordBarsStrategy.hpp
// Strategy for recording price bars data

#ifndef RECORDBARSSTRATEGY_HPP
#define RECORDBARSSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// Strategy for recording historical price bars
/// Records OHLCV data for later analysis and backtesting
class RecordBarsStrategy : public BaseStrategy {
public:
    RecordBarsStrategy();
    
    std::string getName() const;
    
protected:
    /// Execute the record bars strategy
    /// Records price bar data to results structure
    /// @param context Strategy execution context
    /// @param indicators Loaded indicators (unused for this strategy)
    /// @return Strategy result with success/failure code
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators);
    
    /// Update results structure with strategy output
    /// @param context Strategy execution context
    /// @param result Strategy execution result
    void updateResults(
        const StrategyContext& context,
        const StrategyResult& result);
    
    /// This strategy does not require indicators
    /// @return false
    bool requiresIndicators() const;
};

} // namespace trading

#endif // RECORDBARSSTRATEGY_HPP
