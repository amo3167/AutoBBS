// StrategyFactory.hpp
// Factory class for creating trading strategy instances based on StrategyId
// Uses registry pattern for extensibility

#ifndef STRATEGYFACTORY_HPP
#define STRATEGYFACTORY_HPP

#include "IStrategy.hpp"
#include "StrategyTypes.h"
#include <map>

namespace trading {

/// Factory for creating trading strategy instances
/// Uses registry pattern to allow runtime strategy creation based on ID
class StrategyFactory {
public:
    /// Function pointer type for strategy creator functions
    typedef IStrategy* (*StrategyCreator)();
    
    /// Get singleton instance of factory
    /// @return Reference to the singleton factory instance
    static StrategyFactory& getInstance();
    
    /// Register a strategy creator function for a given ID
    /// @param id Strategy identifier
    /// @param creator Function that creates and returns a new strategy instance
    /// @return true if registration succeeded, false if ID already registered
    bool registerStrategy(StrategyId id, StrategyCreator creator);
    
    /// Create a strategy instance for the given ID
    /// @param id Strategy identifier
    /// @return Pointer to newly created strategy, or NULL if ID not registered
    /// @note Caller is responsible for deleting the returned strategy
    IStrategy* createStrategy(StrategyId id);
    
    /// Check if a strategy is registered
    /// @param id Strategy identifier
    /// @return true if the strategy ID is registered, false otherwise
    bool isRegistered(StrategyId id) const;
    
    /// Get count of registered strategies
    /// @return Number of registered strategies
    size_t getRegisteredCount() const;
    
private:
    // Private constructor for singleton
    StrategyFactory();
    
    // Non-copyable (C++03 idiom)
    StrategyFactory(const StrategyFactory&);
    StrategyFactory& operator=(const StrategyFactory&);
    
    /// Registry mapping strategy IDs to creator functions
    std::map<StrategyId, StrategyCreator> registry_;
    
    /// Initialize factory with all known strategies
    void initializeRegistry();
};

} // namespace trading

#endif // STRATEGYFACTORY_HPP
