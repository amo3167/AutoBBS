// AutoBBSStrategy.cpp
// Implementation of AutoBBSStrategy dispatcher

#include "strategies/AutoBBSStrategy.hpp"
#include "StrategyContext.hpp"

namespace trading {

AutoBBSStrategy::AutoBBSStrategy() 
    : BaseStrategy(AUTOBBS) 
{
}

std::string AutoBBSStrategy::getName() const {
    return "AutoBBS";
}

StrategyResult AutoBBSStrategy::executeStrategy(
    const StrategyContext& context,
    Indicators* indicators)
{
    StrategyResult result;
    
    // TODO: Implement dispatcher logic to route to sub-strategies
    // Based on strategy_mode setting in context
    // For now, return success as a stub
    result.code = SUCCESS;
    
    return result;
}

void AutoBBSStrategy::updateResults(
    const StrategyContext& context,
    const StrategyResult& result)
{
    // TODO: Write results back to context.getResults()
}

bool AutoBBSStrategy::requiresIndicators() const {
    return true;
}

} // namespace trading
