/**
 * @file IStrategy.hpp
 * @brief Pure virtual interface for all trading strategies (C++ implementation)
 * @details Defines the contract that all strategy implementations must follow.
 *          Part of Phase 1: C++ migration with C API wrapper pattern.
 * 
 * @author Phase 1 Migration Team
 * @date November 2025
 * @version 1.0.0
 */

#ifndef TRADING_ISTRATEGY_HPP_
#define TRADING_ISTRATEGY_HPP_
#pragma once

#include "AsirikuyDefines.h"
#include "StrategyTypes.h"
#include <string>

namespace trading {

// Forward declarations
class StrategyContext;

/**
 * @brief Pure virtual interface for all trading strategies
 * 
 * @details This interface defines the contract that all strategy implementations
 *          must follow. It uses the Strategy pattern to allow different trading
 *          algorithms to be swapped at runtime.
 * 
 * Key design principles:
 * - Pure virtual interface (no implementation)
 * - Virtual destructor for proper cleanup
 * - Const-correct methods where applicable
 * - Returns AsirikuyReturnCode for C API compatibility
 * 
 * Usage example:
 * @code
 * class MACDStrategy : public IStrategy {
 *     AsirikuyReturnCode execute(const StrategyContext& context) override {
 *         // Implementation
 *     }
 *     // ... other methods
 * };
 * @endcode
 */
class IStrategy {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     * @details Ensures derived classes are properly destroyed
     */
    virtual ~IStrategy() {}
    
    /**
     * @brief Execute the trading strategy
     * 
     * @param context Strategy execution context containing parameters, 
     *                market data, and results
     * @return AsirikuyReturnCode SUCCESS if execution completed successfully,
     *                            error code otherwise
     * 
     * @details This is the main entry point for strategy execution. It should:
     *          1. Validate inputs
     *          2. Load required indicators
     *          3. Analyze market conditions
     *          4. Generate trading signals
     *          5. Place/modify/close orders
     *          6. Update results
     */
    virtual AsirikuyReturnCode execute(const StrategyContext& context) = 0;
    
    /**
     * @brief Get the human-readable name of the strategy
     * 
     * @return std::string Strategy name (e.g., "MACD Daily", "RecordBars")
     * 
     * @details Used for logging, debugging, and user interface display.
     *          Should return a consistent, descriptive name.
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Get the unique strategy identifier
     * 
     * @return StrategyId Enum value identifying this strategy
     *                    (e.g., AUTOBBS, RECORD_BARS, TAKEOVER)
     * 
     * @details This ID is used to:
     *          - Match strategy to execution request
     *          - Route strategy through factory
     *          - Identify strategy in logs and results
     */
    virtual StrategyId getId() const = 0;
    
    /**
     * @brief Validate strategy parameters and context
     * 
     * @param context Strategy execution context to validate
     * @return bool true if validation passed, false otherwise
     * 
     * @details Performs validation of:
     *          - Required parameters are set
     *          - Parameter values are within valid ranges
     *          - Market data is available
     *          - Indicators can be loaded
     * 
     * Should be called before execute() to fail fast on invalid inputs.
     */
    virtual bool validate(const StrategyContext& context) const = 0;
    
protected:
    /**
     * @brief Helper to convert boolean result to return code
     * 
     * @param success true for success, false for failure
     * @return AsirikuyReturnCode SUCCESS or STRATEGY_INTERNAL_ERROR
     * 
     * @details Utility method for derived classes to simplify return code generation
     */
    static AsirikuyReturnCode toReturnCode(bool success) {
        return success ? SUCCESS : static_cast<AsirikuyReturnCode>(STRATEGY_INTERNAL_ERROR);
    }
};

} // namespace trading

#endif // TRADING_ISTRATEGY_HPP_
