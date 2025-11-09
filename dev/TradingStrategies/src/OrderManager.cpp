#include "OrderManager.hpp"

// Include C order management functions
extern "C" {
    #include "OrderManagement.h"
    #include "AsirikuyDefines.h"
}

namespace asirikuy {

OrderManager::OrderManager(StrategyContext& context)
    : m_context(context) {
}

// Order counting and queries

int OrderManager::getTotalOpenOrders(OrderType orderType) const {
    return totalOpenOrders(m_context.getParams(), orderType);
}

int OrderManager::getTotalClosedOrders(OrderType orderType) const {
    return totalClosedOrders(m_context.getParams(), orderType);
}

// Order sizing and risk

double OrderManager::calculateOrderSize(OrderType orderType, double entryPrice, double stopLoss) const {
    return ::calculateOrderSize(m_context.getParams(), orderType, entryPrice, stopLoss);
}

double OrderManager::calculateOrderSizeWithRisk(OrderType orderType, double entryPrice, 
                                               double stopLoss, double risk) const {
    return calculateOrderSizeWithSpecificRisk(m_context.getParams(), orderType, entryPrice, stopLoss, risk);
}

bool OrderManager::hasEnoughFreeMargin(OrderType orderType, double lotSize) const {
    return isEnoughFreeMargin(m_context.getParams(), orderType, lotSize) == TRUE;
}

double OrderManager::getMaxLossPerLot(OrderType orderType, double entryPrice, double stopLoss) const {
    return maxLossPerLot(m_context.getParams(), orderType, entryPrice, stopLoss);
}

// Order lifecycle - open/update/close

bool OrderManager::openOrUpdateLongTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                                        double takeProfit, double risk, bool useInternalSL, bool useInternalTP) {
    AsirikuyReturnCode result = ::openOrUpdateLongTrade(
        m_context.getParams(), ratesIndex, resultsIndex, stopLoss, takeProfit, risk,
        useInternalSL ? TRUE : FALSE, useInternalTP ? TRUE : FALSE);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::openOrUpdateShortTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                                         double takeProfit, double risk, bool useInternalSL, bool useInternalTP) {
    AsirikuyReturnCode result = ::openOrUpdateShortTrade(
        m_context.getParams(), ratesIndex, resultsIndex, stopLoss, takeProfit, risk,
        useInternalSL ? TRUE : FALSE, useInternalTP ? TRUE : FALSE);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::updateLongTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                                   double takeProfit, bool useInternalSL, bool useInternalTP) {
    AsirikuyReturnCode result = ::updateLongTrade(
        m_context.getParams(), ratesIndex, resultsIndex, stopLoss, takeProfit,
        useInternalSL ? TRUE : FALSE, useInternalTP ? TRUE : FALSE);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::updateShortTrade(int ratesIndex, int resultsIndex, double stopLoss, 
                                    double takeProfit, bool useInternalSL, bool useInternalTP) {
    AsirikuyReturnCode result = ::updateShortTrade(
        m_context.getParams(), ratesIndex, resultsIndex, stopLoss, takeProfit,
        useInternalSL ? TRUE : FALSE, useInternalTP ? TRUE : FALSE);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::closeLongTrade(int resultsIndex) {
    AsirikuyReturnCode result = ::closeLongTrade(m_context.getParams(), resultsIndex);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::closeShortTrade(int resultsIndex) {
    AsirikuyReturnCode result = ::closeShortTrade(m_context.getParams(), resultsIndex);
    return result == ASIRIKUY_SUCCESS;
}

// Order validation and monitoring

bool OrderManager::areOrdersCorrect(double stopLoss, double takeProfit) const {
    return ::areOrdersCorrect(m_context.getParams(), stopLoss, takeProfit) == TRUE;
}

bool OrderManager::checkInternalStopLoss(int ratesIndex, int resultsIndex, double internalSL) {
    AsirikuyReturnCode result = checkInternalSL(m_context.getParams(), ratesIndex, resultsIndex, internalSL);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::checkInternalTakeProfit(int ratesIndex, int resultsIndex, double internalTP) {
    AsirikuyReturnCode result = checkInternalTP(m_context.getParams(), ratesIndex, resultsIndex, internalTP);
    return result == ASIRIKUY_SUCCESS;
}

bool OrderManager::checkTimedExit(int ratesIndex, int resultsIndex, bool usingInternalSL, bool usingInternalTP) {
    AsirikuyReturnCode result = ::checkTimedExit(
        m_context.getParams(), ratesIndex, resultsIndex,
        usingInternalSL ? TRUE : FALSE, usingInternalTP ? TRUE : FALSE);
    return result == ASIRIKUY_SUCCESS;
}

// Trailing stops and dynamic management

bool OrderManager::trailOpenTrades(int ratesIndex, double trailStart, double trailDistance, 
                                   bool useInternalSL, bool useInternalTP) {
    AsirikuyReturnCode result = ::trailOpenTrades(
        m_context.getParams(), ratesIndex, trailStart, trailDistance,
        useInternalSL ? TRUE : FALSE, useInternalTP ? TRUE : FALSE);
    return result == ASIRIKUY_SUCCESS;
}

int OrderManager::getOrderAge(int ratesIndex) const {
    return ::getOrderAge(m_context.getParams(), ratesIndex);
}

int OrderManager::getOrderAgeByPosition(int ratesIndex, int position) const {
    return getOrderAgeByPosition(m_context.getParams(), ratesIndex, position);
}

// Advanced stop loss/take profit calculations

double OrderManager::calculateEllipticalStopLoss(double takeProfit, int maxHoldingTime, 
                                                 double z, int orderBarsAge) const {
    return CalculateEllipticalStopLoss(m_context.getParams(), takeProfit, maxHoldingTime, z, orderBarsAge);
}

double OrderManager::calculateEllipticalTakeProfit(double takeProfit, int maxHoldingTime, 
                                                   double z, int orderBarsAge) const {
    return CalculateEllipticalTakeProfit(m_context.getParams(), takeProfit, maxHoldingTime, z, orderBarsAge);
}

} // namespace asirikuy
