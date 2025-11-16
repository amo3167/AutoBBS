// StrategyFactory.cpp
// Implementation of StrategyFactory with registry pattern

#include "StrategyFactory.hpp"
#include "strategies/RecordBarsStrategy.hpp"
#include "strategies/TakeOverStrategy.hpp"
#include "strategies/ScreeningStrategy.hpp"
#include "strategies/TrendLimitStrategy.hpp"
#include "strategies/AutoBBSStrategy.hpp"
#include "strategies/AutoBBSWeeklyStrategy.hpp"

namespace trading {

// Creator functions for each strategy
static IStrategy* createRecordBarsStrategy() {
    return new RecordBarsStrategy();
}

static IStrategy* createTakeOverStrategy() {
    return new TakeOverStrategy();
}

static IStrategy* createScreeningStrategy() {
    return new ScreeningStrategy();
}

static IStrategy* createTrendLimitStrategy() {
    return new TrendLimitStrategy();
}

static IStrategy* createAutoBBSStrategy() {
    return new AutoBBSStrategy();
}

static IStrategy* createAutoBBSWeeklyStrategy() {
    return new AutoBBSWeeklyStrategy();
}

StrategyFactory::StrategyFactory() {
    initializeRegistry();
}

StrategyFactory& StrategyFactory::getInstance() {
    static StrategyFactory instance;
    return instance;
}

bool StrategyFactory::registerStrategy(StrategyId id, StrategyCreator creator) {
    if (creator == NULL) {
        return false;
    }
    
    // Check if already registered
    if (registry_.find(id) != registry_.end()) {
        return false;
    }
    
    registry_[id] = creator;
    return true;
}

IStrategy* StrategyFactory::createStrategy(StrategyId id) {
    std::map<StrategyId, StrategyCreator>::const_iterator it = registry_.find(id);
    
    if (it == registry_.end()) {
        return NULL;
    }
    
    return it->second();
}

bool StrategyFactory::isRegistered(StrategyId id) const {
    return registry_.find(id) != registry_.end();
}

size_t StrategyFactory::getRegisteredCount() const {
    return registry_.size();
}

void StrategyFactory::initializeRegistry() {
    // Register all known strategies
    registerStrategy(RECORD_BARS, createRecordBarsStrategy);
    registerStrategy(TAKEOVER, createTakeOverStrategy);
    registerStrategy(SCREENING, createScreeningStrategy);
    registerStrategy(TRENDLIMIT, createTrendLimitStrategy);
    registerStrategy(AUTOBBS, createAutoBBSStrategy);
    registerStrategy(AUTOBBSWEEKLY, createAutoBBSWeeklyStrategy);
}

} // namespace trading
