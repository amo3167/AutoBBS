/**
 * @file StrategyContext.hpp
 * @brief C++ wrapper for StrategyParams structure
 * @details Provides type-safe, object-oriented access to strategy parameters,
 *          market data, indicators, and order management. Part of Phase 1 C++ migration.
 * 
 * @author Phase 1 Migration Team
 * @date November 2025
 * @version 1.0.0
 */

#ifndef TRADING_STRATEGY_CONTEXT_HPP_
#define TRADING_STRATEGY_CONTEXT_HPP_
#pragma once

#include "AsirikuyDefines.h"
#include "StrategyTypes.h"
#include "strategies/Base.h"
#include <string>
#include <stdexcept>

namespace trading {

// Forward declarations
class Indicators;
class OrderManager;

/**
 * @brief C++ wrapper for StrategyParams providing clean, type-safe access
 * 
 * @details StrategyContext wraps the C StrategyParams structure and provides:
 *          - Type-safe accessors for all parameters
 *          - Lazy-loading of indicators and order manager
 *          - Exception-based error handling
 *          - RAII-style resource management
 * 
 * Design principles:
 * - Non-copyable (use move semantics if needed)
 * - Movable for efficiency
 * - Does NOT own the StrategyParams* (caller owns it)
 * - Lazy-loads heavy resources (Indicators, OrderManager)
 * - Const-correct accessors
 * 
 * Usage:
 * @code
 * StrategyParams* params = ...; // From C API
 * trading::StrategyContext ctx(params);
 * 
 * StrategyId id = ctx.getStrategyId();
 * double bid = ctx.getBid(0);
 * Indicators& indicators = ctx.getIndicators();
 * @endcode
 */
class StrategyContext {
public:
    /**
     * @brief Construct context from C StrategyParams
     * 
     * @param params Pointer to StrategyParams (must not be NULL)
     * @throws std::invalid_argument if params is NULL
     * 
     * @details Constructor validates the params pointer but does NOT
     *          take ownership. The caller must ensure params remains
     *          valid for the lifetime of StrategyContext.
     */
    explicit StrategyContext(StrategyParams* params);
    
    /**
     * @brief Destructor - cleans up lazy-loaded resources
     */
    ~StrategyContext();
    
    // Non-copyable (private and unimplemented in C++03)
private:
    StrategyContext(const StrategyContext&);
    StrategyContext& operator=(const StrategyContext&);
    
public:
    // Movable (use default move semantics)
    // Note: VS2010 has limited move semantics support, may need manual implementation
    #if _MSC_VER >= 1600 // VS2010 and later
    StrategyContext(StrategyContext&& other);
    StrategyContext& operator=(StrategyContext&& other);
    #endif
    
    // ========================================================================
    // Strategy Identification
    // ========================================================================
    
    /**
     * @brief Get the strategy ID
     * @return StrategyId Enum identifying which strategy to run
     */
    StrategyId getStrategyId() const;
    
    /**
     * @brief Get the trading symbol (e.g., "EURUSD", "GBPJPY")
     * @return const char* Symbol name (null-terminated string)
     */
    const char* getSymbol() const;
    
    /**
     * @brief Get current broker time
     * @return time_t Current time from broker
     */
    time_t getCurrentTime() const;
    
    // ========================================================================
    // Settings Access
    // ========================================================================
    
    /**
     * @brief Get a setting value by index
     * 
     * @param index SettingsIndex enum value
     * @return double Setting value
     * 
     * @details Settings include strategy parameters like lot sizes,
     *          risk percentages, timeframes, etc.
     */
    double getSetting(SettingsIndex index) const;
    
    /**
     * @brief Check if backtesting mode is enabled
     * @return bool true if backtesting, false if live/demo trading
     */
    bool isBacktesting() const;
    
    /**
     * @brief Get maximum number of open orders allowed
     * @return int Maximum open orders
     */
    int getMaxOpenOrders() const;
    
    // ========================================================================
    // Market Data Access
    // ========================================================================
    
    /**
     * @brief Get bid/ask structure
     * @return const BidAsk& Reference to bid/ask data
     */
    const BidAsk& getBidAsk() const;
    
    /**
     * @brief Get bid price at specific index
     * 
     * @param index Array index (0 = current, 1 = previous, etc.)
     * @return double Bid price
     * @throws std::out_of_range if index is invalid
     */
    double getBid(int index = 0) const;
    
    /**
     * @brief Get ask price at specific index
     * 
     * @param index Array index (0 = current, 1 = previous, etc.)
     * @return double Ask price
     * @throws std::out_of_range if index is invalid
     */
    double getAsk(int index = 0) const;
    
    /**
     * @brief Get spread (ask - bid)
     * @return double Current spread in pips
     */
    double getSpread() const;
    
    // ========================================================================
    // Account Information
    // ========================================================================
    
    /**
     * @brief Get account information structure
     * @return const AccountInfo& Reference to account data
     */
    const AccountInfo& getAccountInfo() const;
    
    /**
     * @brief Get account balance
     * @return double Account balance in account currency
     */
    double getBalance() const;
    
    /**
     * @brief Get account equity
     * @return double Account equity (balance + floating P/L)
     */
    double getEquity() const;
    
    /**
     * @brief Get margin used
     * @return double Margin currently used for open positions
     */
    double getMargin() const;
    
    // ========================================================================
    // Price Data (Rates/Bars)
    // ========================================================================
    
    /**
     * @brief Get rates buffers (OHLC data for all timeframes)
     * @return RatesBuffers* Pointer to rates data
     */
    RatesBuffers* getRatesBuffers() const;
    
    /**
     * @brief Get rates for specific timeframe
     * 
     * @param index BaseRatesIndexes enum (DAILY, WEEKLY, HOURLY, etc.)
     * @return const Rates& Reference to rates for that timeframe
     * @throws std::out_of_range if index is invalid
     */
    const Rates& getRates(BaseRatesIndexes index) const;
    
    // ========================================================================
    // Order Information
    // ========================================================================
    
    /**
     * @brief Get order information structure
     * @return OrderInfo* Pointer to current orders
     * 
     * @details The orders array size is determined by settings[ORDERINFO_ARRAY_SIZE].
     *          Iterate through array and check isOpen field to find active orders.
     */
    OrderInfo* getOrderInfo() const;
    
    // ========================================================================
    // Results
    // ========================================================================
    
    /**
     * @brief Get strategy results structure
     * @return StrategyResults* Pointer to results array
     * 
     * @details Results are written back to this structure for
     *          the MQL layer to read.
     */
    StrategyResults* getResults() const;
    
    // ========================================================================
    // Indicators (Lazy-Loaded)
    // ========================================================================
    
    /**
     * @brief Get indicators (lazy-loaded)
     * 
     * @return Indicators& Reference to indicators wrapper
     * 
     * @details Indicators are loaded on first access and cached.
     *          This avoids loading indicators for strategies that don't need them.
     */
    Indicators& getIndicators();
    
    /**
     * @brief Get indicators (const version)
     */
    const Indicators& getIndicators() const;
    
    // ========================================================================
    // Order Management (Lazy-Loaded)
    // ========================================================================
    
    /**
     * @brief Get order manager (lazy-loaded)
     * 
     * @return OrderManager& Reference to order management wrapper
     * 
     * @details OrderManager provides C++ interface for placing,
     *          modifying, and closing orders.
     */
    OrderManager& getOrderManager();
    
    /**
     * @brief Get order manager (const version)
     */
    const OrderManager& getOrderManager() const;
    
    // ========================================================================
    // Raw Access (for Compatibility During Migration)
    // ========================================================================
    
    /**
     * @brief Get raw StrategyParams pointer
     * 
     * @return StrategyParams* Raw pointer to C structure
     * 
     * @details Use this only when interfacing with legacy C code.
     *          Prefer using the type-safe accessors above.
     */
    StrategyParams* getRawParams() const;
    
private:
    // ========================================================================
    // Private Members
    // ========================================================================
    
    StrategyParams* params_;              ///< Pointer to C structure (NOT owned)
    mutable Indicators* indicators_;       ///< Lazy-loaded indicators (owned)
    mutable OrderManager* orderManager_;   ///< Lazy-loaded order manager (owned)
    
    // ========================================================================
    // Private Helper Methods
    // ========================================================================
    
    /**
     * @brief Ensure indicators are loaded
     * @details Called by getIndicators() on first access
     */
    void ensureIndicatorsLoaded() const;
    
    /**
     * @brief Ensure order manager is created
     * @details Called by getOrderManager() on first access
     */
    void ensureOrderManagerCreated() const;
    
    /**
     * @brief Validate params pointer
     * @throws std::invalid_argument if invalid
     */
    void validateParams() const;
};

} // namespace trading

#endif // TRADING_STRATEGY_CONTEXT_HPP_
