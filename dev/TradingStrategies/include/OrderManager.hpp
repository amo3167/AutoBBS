#ifndef ORDERMANAGER_HPP
#define ORDERMANAGER_HPP

#include "StrategyContext.hpp"
#include "StrategyTypes.h"

namespace asirikuy {

/// @brief Wraps order management functions from the C OrderManagement library
/// @details Provides C++ interface for order sizing, margin checks, and order lifecycle management
class OrderManager {
public:
    /// @brief Construct OrderManager with strategy context
    /// @param context Reference to strategy context containing parameters and state
    explicit OrderManager(StrategyContext& context);

    // Order counting and queries
    
    /// @brief Get total number of open orders
    /// @param orderType Type filter (BUY, SELL, or ALL for both)
    /// @return Count of open orders matching type
    int getTotalOpenOrders(OrderType orderType) const;
    
    /// @brief Get total number of closed orders
    /// @param orderType Type filter (BUY, SELL, or ALL for both)
    /// @return Count of closed orders matching type
    int getTotalClosedOrders(OrderType orderType) const;

    // Order sizing and risk
    
    /// @brief Calculate order size based on strategy risk parameters
    /// @param orderType Order type (BUY or SELL)
    /// @param entryPrice Entry price for the order
    /// @param stopLoss Stop loss price (absolute price, not distance)
    /// @return Order size in lots
    double calculateOrderSize(OrderType orderType, double entryPrice, double stopLoss) const;
    
    /// @brief Calculate order size with specific risk amount
    /// @param orderType Order type (BUY or SELL)
    /// @param entryPrice Entry price for the order
    /// @param stopLoss Stop loss price (absolute price, not distance)
    /// @param risk Risk amount (e.g., 0.02 for 2% risk)
    /// @return Order size in lots
    double calculateOrderSizeWithRisk(OrderType orderType, double entryPrice, double stopLoss, double risk) const;
    
    /// @brief Check if enough free margin exists to open order
    /// @param orderType Order type (BUY or SELL)
    /// @param lotSize Lot size to check
    /// @return true if enough margin, false otherwise
    bool hasEnoughFreeMargin(OrderType orderType, double lotSize) const;
    
    /// @brief Calculate maximum loss per lot for an order
    /// @param orderType Order type (BUY or SELL)
    /// @param entryPrice Entry price for the order
    /// @param stopLoss Stop loss price (absolute price, not distance)
    /// @return Maximum loss per lot in account currency
    double getMaxLossPerLot(OrderType orderType, double entryPrice, double stopLoss) const;

    // Order lifecycle - open/update/close
    
    /// @brief Open new long trade or update existing one
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param stopLoss Stop loss distance in price units
    /// @param takeProfit Take profit distance in price units
    /// @param risk Risk amount (e.g., 0.02 for 2% risk)
    /// @param useInternalSL Use internal stop loss tracking
    /// @param useInternalTP Use internal take profit tracking
    /// @return true on success, false on failure
    bool openOrUpdateLongTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                               double takeProfit, double risk, bool useInternalSL, bool useInternalTP);
    
    /// @brief Open new short trade or update existing one
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param stopLoss Stop loss distance in price units
    /// @param takeProfit Take profit distance in price units
    /// @param risk Risk amount (e.g., 0.02 for 2% risk)
    /// @param useInternalSL Use internal stop loss tracking
    /// @param useInternalTP Use internal take profit tracking
    /// @return true on success, false on failure
    bool openOrUpdateShortTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                                double takeProfit, double risk, bool useInternalSL, bool useInternalTP);
    
    /// @brief Update existing long trade stop loss and take profit
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param stopLoss Stop loss distance in price units
    /// @param takeProfit Take profit distance in price units
    /// @param useInternalSL Use internal stop loss tracking
    /// @param useInternalTP Use internal take profit tracking
    /// @return true on success, false on failure
    bool updateLongTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                        double takeProfit, bool useInternalSL, bool useInternalTP);
    
    /// @brief Update existing short trade stop loss and take profit
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param stopLoss Stop loss distance in price units
    /// @param takeProfit Take profit distance in price units
    /// @param useInternalSL Use internal stop loss tracking
    /// @param useInternalTP Use internal take profit tracking
    /// @return true on success, false on failure
    bool updateShortTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                         double takeProfit, bool useInternalSL, bool useInternalTP);
    
    /// @brief Close all long trades
    /// @param resultsIndex Index of results array
    /// @return true on success, false on failure
    bool closeLongTrade(int resultsIndex);
    
    /// @brief Close all short trades
    /// @param resultsIndex Index of results array
    /// @return true on success, false on failure
    bool closeShortTrade(int resultsIndex);

    // Order validation and monitoring
    
    /// @brief Check if stop loss and take profit are correctly set
    /// @param stopLoss Stop loss distance in price units
    /// @param takeProfit Take profit distance in price units
    /// @return true if orders are correct, false otherwise
    bool areOrdersCorrect(double stopLoss, double takeProfit) const;
    
    /// @brief Check if trades exceeded internal stop loss
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param internalSL Internal stop loss distance in price units
    /// @return true on success, false on failure
    bool checkInternalStopLoss(int ratesIndex, int resultsIndex, double internalSL);
    
    /// @brief Check if trades exceeded internal take profit
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param internalTP Internal take profit distance in price units
    /// @return true on success, false on failure
    bool checkInternalTakeProfit(int ratesIndex, int resultsIndex, double internalTP);
    
    /// @brief Check if trades exceeded maximum time
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param resultsIndex Index of results array
    /// @param usingInternalSL Using internal stop loss
    /// @param usingInternalTP Using internal take profit
    /// @return true on success, false on failure
    bool checkTimedExit(int ratesIndex, int resultsIndex, bool usingInternalSL, bool usingInternalTP);

    // Trailing stops and dynamic management
    
    /// @brief Trail stop loss for open trades
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param trailStart Distance price must move before trailing starts
    /// @param trailDistance Distance between new SL and current price
    /// @param useInternalSL Use internal stop loss tracking
    /// @param useInternalTP Use internal take profit tracking
    /// @return true on success, false on failure
    bool trailOpenTrades(int ratesIndex, double trailStart, double trailDistance, 
                        bool useInternalSL, bool useInternalTP);
    
    /// @brief Get age of current order in bars
    /// @param ratesIndex Index of rates array (typically 0)
    /// @return Order age in bars
    int getOrderAge(int ratesIndex) const;
    
    /// @brief Get age of specific order by position/ticket
    /// @param ratesIndex Index of rates array (typically 0)
    /// @param position Order ticket/position number
    /// @return Order age in bars
    int getOrderAgeByPosition(int ratesIndex, int position) const;

    // Advanced stop loss/take profit calculations
    
    /// @brief Calculate elliptical stop loss based on time decay
    /// @param takeProfit Take profit distance in price units
    /// @param maxHoldingTime Maximum bars to hold position
    /// @param z Confidence parameter (e.g., 2 for ~98% confidence)
    /// @param orderBarsAge Current order age in bars
    /// @return Elliptical stop loss distance
    double calculateEllipticalStopLoss(double takeProfit, int maxHoldingTime, 
                                      double z, int orderBarsAge) const;
    
    /// @brief Calculate elliptical take profit based on time decay
    /// @param takeProfit Target take profit distance in price units
    /// @param maxHoldingTime Maximum bars to hold position
    /// @param z Confidence parameter (e.g., 2 for ~98% confidence)
    /// @param orderBarsAge Current order age in bars
    /// @return Elliptical take profit distance
    double calculateEllipticalTakeProfit(double takeProfit, int maxHoldingTime, 
                                        double z, int orderBarsAge) const;

private:
    StrategyContext& m_context;
    
    // Non-copyable (C++03 idiom)
    OrderManager(const OrderManager&);
    OrderManager& operator=(const OrderManager&);
};

} // namespace asirikuy

#endif // ORDERMANAGER_HPP
