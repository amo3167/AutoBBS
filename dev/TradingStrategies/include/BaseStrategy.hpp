/**
 * @file BaseStrategy.hpp
 * @brief Abstract base class implementing Template Method pattern for strategies.
 * @details Provides skeleton for strategy execution: validate → loadIndicators → executeStrategy → updateResults.
 *          Derived classes override the protected hook methods while execute() stays final.
 *
 * Phase 1 C++ Migration
 */
#ifndef TRADING_BASE_STRATEGY_HPP_
#define TRADING_BASE_STRATEGY_HPP_
#pragma once

#include "IStrategy.hpp"
#include "StrategyContext.hpp"
#include "StrategyTypes.h"
#include <string>

namespace trading {

// Forward declarations (real implementations later in Phase 1 Week 2)
class Indicators;

/**
 * @brief StrategyResult structure for passing internal execution outcome.
 */
struct StrategyResult {
    AsirikuyReturnCode code;          ///< Execution return code
    int generatedOrders;              ///< Number of orders generated
    double pnlImpact;                 ///< Estimated PnL impact of execution
    std::string message;              ///< Optional textual message

    StrategyResult() : code(SUCCESS), generatedOrders(0), pnlImpact(0.0) {}
};

/**
 * @brief Abstract base strategy implementing the Template Method pattern.
 */
class BaseStrategy : public IStrategy {
public:
    explicit BaseStrategy(StrategyId id) : id_(id) {}
    virtual ~BaseStrategy() {}

    // Non-copyable (private and unimplemented in C++03)
private:
    BaseStrategy(const BaseStrategy&);
    BaseStrategy& operator=(const BaseStrategy&);

public:
    // Execute - final wrapper implementing algorithm skeleton
    AsirikuyReturnCode execute(const StrategyContext& context) /* override final */ {
        // 1. Validate inputs
        if (!validate(context)) {
            return static_cast<AsirikuyReturnCode>(STRATEGY_INVALID_PARAMETERS);
        }

        // 2. Load indicators (may be unused depending on strategy)
        Indicators* indicators = loadIndicators(context);
        if (requiresIndicators() && indicators == NULL) {
            return static_cast<AsirikuyReturnCode>(STRATEGY_FAILED_TO_LOAD_INDICATORS);
        }

        // 3. Strategy-specific logic
        StrategyResult result = executeStrategy(context, indicators);

        // 4. Update results (write back to StrategyParams results structure)
        updateResults(context, result);

        return result.code;
    }

    StrategyId getId() const /* override */ { return id_; }

    // Default validate can be overridden
    bool validate(const StrategyContext& context) const override {
        // Basic sanity checks
        if (context.getSymbol() == NULL || *context.getSymbol() == '\0') {
            return false;
        }
        // Bid/Ask sanity (avoid zero or negative)
        try {
            double bid = context.getBid(0);
            double ask = context.getAsk(0);
            if (bid <= 0.0 || ask <= 0.0 || ask - bid < 0) {
                return false;
            }
        } catch (...) {
            return false;
        }
        return true;
    }

protected:
    // Hook: load indicators (default: no indicators)
    virtual Indicators* loadIndicators(const StrategyContext& context) { return NULL; }

    // Hook: execute core strategy logic
    virtual StrategyResult executeStrategy(const StrategyContext& context, Indicators* indicators) = 0;

    // Hook: write back results (orders, PnL, custom outputs)
    virtual void updateResults(const StrategyContext& context, const StrategyResult& result) = 0;

    // Hook: whether indicators are required (affects error handling)
    virtual bool requiresIndicators() const { return false; }

private:
    StrategyId id_;
};

} // namespace trading

#endif // TRADING_BASE_STRATEGY_HPP_
