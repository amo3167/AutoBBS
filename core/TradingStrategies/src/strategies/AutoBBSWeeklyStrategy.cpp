// AutoBBSWeeklyStrategy.cpp
// Implementation of AutoBBSWeeklyStrategy dispatcher

#include "strategies/AutoBBSWeeklyStrategy.hpp"
#include "StrategyContext.hpp"

namespace trading {

AutoBBSWeeklyStrategy::AutoBBSWeeklyStrategy() 
    : BaseStrategy(AUTOBBSWEEKLY) 
{
}

std::string AutoBBSWeeklyStrategy::getName() const {
    return "AutoBBS Weekly";
}

StrategyResult AutoBBSWeeklyStrategy::executeStrategy(
    const StrategyContext& context,
    Indicators* indicators)
{
    StrategyResult result;
    
    // TODO: Implement weekly dispatcher logic to route to sub-strategies
    // Based on strategy_mode setting in context
    // For now, return success as a stub
    result.code = SUCCESS;
    
    return result;
}

void AutoBBSWeeklyStrategy::updateResults(
    const StrategyContext& context,
    const StrategyResult& result)
{
    // TODO: Write results back to context.getResults()
}

bool AutoBBSWeeklyStrategy::requiresIndicators() const {
    return true;
}

} // namespace trading
