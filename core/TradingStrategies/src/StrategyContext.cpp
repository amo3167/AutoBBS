/**
 * @file StrategyContext.cpp
 * @brief Implementation of StrategyContext wrapper
 * @details Wraps C StrategyParams with C++ interface
 * 
 * @author Phase 1 Migration Team
 * @date November 2025
 */

#include "StrategyContext.hpp"
#include <cstring>
#include <cmath>

// Forward declarations for classes we'll implement later
// For now, we'll use simple placeholder classes
namespace trading {

// Placeholder for Indicators class (will be implemented in Phase 1 Week 2)
class Indicators {
public:
    explicit Indicators(StrategyParams* params) : params_(params) {}
private:
    StrategyParams* params_;
};

// Placeholder for OrderManager class (will be implemented in Phase 1 Week 2)
class OrderManager {
public:
    explicit OrderManager(StrategyParams* params) : params_(params) {}
private:
    StrategyParams* params_;
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

StrategyContext::StrategyContext(StrategyParams* params)
    : params_(params)
    , indicators_(NULL)
    , orderManager_(NULL)
{
    validateParams();
}

StrategyContext::~StrategyContext() {
    // Clean up lazy-loaded resources
    if (indicators_ != NULL) {
        delete indicators_;
        indicators_ = NULL;
    }
    if (orderManager_ != NULL) {
        delete orderManager_;
        orderManager_ = NULL;
    }
    // Note: We do NOT delete params_ as we don't own it
}

#if _MSC_VER >= 1600 // VS2010 and later
StrategyContext::StrategyContext(StrategyContext&& other)
    : params_(other.params_)
    , indicators_(other.indicators_)
    , orderManager_(other.orderManager_)
{
    // Transfer ownership of lazy-loaded resources
    other.params_ = NULL;
    other.indicators_ = NULL;
    other.orderManager_ = NULL;
}

StrategyContext& StrategyContext::operator=(StrategyContext&& other) {
    if (this != &other) {
        // Clean up existing resources
        delete indicators_;
        delete orderManager_;
        
        // Transfer ownership
        params_ = other.params_;
        indicators_ = other.indicators_;
        orderManager_ = other.orderManager_;
        
        other.params_ = NULL;
        other.indicators_ = NULL;
        other.orderManager_ = NULL;
    }
    return *this;
}
#endif

// ============================================================================
// Strategy Identification
// ============================================================================

StrategyId StrategyContext::getStrategyId() const {
    return static_cast<StrategyId>(
        static_cast<int>(params_->settings[INTERNAL_STRATEGY_ID])
    );
}

const char* StrategyContext::getSymbol() const {
    return params_->tradeSymbol;
}

time_t StrategyContext::getCurrentTime() const {
    return params_->currentBrokerTime;
}

// ============================================================================
// Settings Access
// ============================================================================

double StrategyContext::getSetting(SettingsIndex index) const {
    return params_->settings[index];
}

bool StrategyContext::isBacktesting() const {
    return (static_cast<int>(params_->settings[IS_BACKTESTING]) != 0);
}

int StrategyContext::getMaxOpenOrders() const {
    return static_cast<int>(params_->settings[MAX_OPEN_ORDERS]);
}

// ============================================================================
// Market Data Access
// ============================================================================

const BidAsk& StrategyContext::getBidAsk() const {
    return params_->bidAsk;
}

double StrategyContext::getBid(int index) const {
    if (index < 0 || index >= params_->bidAsk.arraySize) {
        throw std::out_of_range("Bid index out of range");
    }
    return params_->bidAsk.bid[index];
}

double StrategyContext::getAsk(int index) const {
    if (index < 0 || index >= params_->bidAsk.arraySize) {
        throw std::out_of_range("Ask index out of range");
    }
    return params_->bidAsk.ask[index];
}

double StrategyContext::getSpread() const {
    return getAsk(0) - getBid(0);
}

// ============================================================================
// Account Information
// ============================================================================

const AccountInfo& StrategyContext::getAccountInfo() const {
    return params_->accountInfo;
}

double StrategyContext::getBalance() const {
    return params_->accountInfo.balance;
}

double StrategyContext::getEquity() const {
    return params_->accountInfo.equity;
}

double StrategyContext::getMargin() const {
    return params_->accountInfo.margin;
}

// ============================================================================
// Price Data
// ============================================================================

RatesBuffers* StrategyContext::getRatesBuffers() const {
    return params_->ratesBuffers;
}

const Rates& StrategyContext::getRates(BaseRatesIndexes index) const {
    if (index < 0 || index >= BASE_RATES_INDEXES_COUNT) {
        throw std::out_of_range("Rates index out of range");
    }
    return params_->ratesBuffers->rates[index];
}

// ============================================================================
// Order Information
// ============================================================================

OrderInfo* StrategyContext::getOrderInfo() const {
    return params_->orderInfo;
}

// ============================================================================
// Results
// ============================================================================

StrategyResults* StrategyContext::getResults() const {
    return params_->results;
}

// ============================================================================
// Indicators (Lazy-Loaded)
// ============================================================================

Indicators& StrategyContext::getIndicators() {
    ensureIndicatorsLoaded();
    return *indicators_;
}

const Indicators& StrategyContext::getIndicators() const {
    ensureIndicatorsLoaded();
    return *indicators_;
}

void StrategyContext::ensureIndicatorsLoaded() const {
    if (indicators_ == NULL) {
        // Note: Using const_cast here is safe because we're implementing lazy loading
        // The indicators_ member is mutable
        StrategyContext* mutableThis = const_cast<StrategyContext*>(this);
        mutableThis->indicators_ = new Indicators(params_);
    }
}

// ============================================================================
// Order Management (Lazy-Loaded)
// ============================================================================

OrderManager& StrategyContext::getOrderManager() {
    ensureOrderManagerCreated();
    return *orderManager_;
}

const OrderManager& StrategyContext::getOrderManager() const {
    ensureOrderManagerCreated();
    return *orderManager_;
}

void StrategyContext::ensureOrderManagerCreated() const {
    if (orderManager_ == NULL) {
        StrategyContext* mutableThis = const_cast<StrategyContext*>(this);
        mutableThis->orderManager_ = new OrderManager(params_);
    }
}

// ============================================================================
// Raw Access
// ============================================================================

StrategyParams* StrategyContext::getRawParams() const {
    return params_;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

void StrategyContext::validateParams() const {
    if (params_ == NULL) {
        throw std::invalid_argument("StrategyParams cannot be NULL");
    }
    
    // Additional validation could be added here:
    // - Check symbol is not empty
    // - Check bid/ask are valid
    // etc.
}

} // namespace trading
