#include "strategies/TakeOverStrategy.hpp"
#include "Indicators.hpp"
#include "OrderManager.hpp"
#include <cmath>
#include <string>

// Include C functions we need to call
extern "C" {
    #include "StrategyUserInterface.h"
    #include "AsirikuyTime.h"
    #include "AsirikuyDefines.h"
    #include "Logging.h"
}

namespace trading {

namespace {
    /// @brief Helper to convert time_t to std::string
    std::string timeToString(time_t timestamp) {
        char buffer[MAX_TIME_STRING_SIZE];
        safe_timeString(buffer, timestamp);
        return std::string(buffer);
    }
}

TakeOverStrategy::TakeOverStrategy() 
    : BaseStrategy(TAKEOVER) {
}

std::string TakeOverStrategy::getName() const {
    return "TakeOver";
}

Indicators* TakeOverStrategy::loadIndicators(const StrategyContext& context) {
    // TakeOver uses internal indicators structure, not Indicators class
    return NULL;
}

StrategyResult TakeOverStrategy::executeStrategy(const StrategyContext& context,
                                                 Indicators* indicators) {
    StrategyResult result;
    result.code = SUCCESS;
    result.generatedOrders = 0;
    result.pnlImpact = 0.0;

    // Load all indicators
    TakeOverIndicators tkIndicators;
    loadTakeOverIndicators(context, tkIndicators);

    // Log indicator values
    int shift0Index = context.getBarsTotal(0) - 1;
    time_t currentTime = context.getTime(0, shift0Index);
    std::string timeString = timeToString(currentTime);

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, 
        (PAN_CHAR_T*)"TakeOver InstanceID=%d, BarTime=%s, BBSTrend=%d, BBStopPrice=%lf, BBSIndex=%d",
        (int)context.getSetting(STRATEGY_INSTANCE_ID), timeString.c_str(), 
        tkIndicators.bbsTrend, tkIndicators.bbsStopPrice, tkIndicators.bbsIndex);

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"TakeOver InstanceID=%d, BarTime=%s, preHigh=%lf, preLow=%lf, preClose=%lf",
        (int)context.getSetting(STRATEGY_INSTANCE_ID), timeString.c_str(),
        tkIndicators.preHigh, tkIndicators.preLow, tkIndicators.preClose);

    pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
        (PAN_CHAR_T*)"TakeOver InstanceID=%d, BarTime=%s, buySLP=%lf, sellSLP=%lf, DSL=%d",
        (int)context.getSetting(STRATEGY_INSTANCE_ID), timeString.c_str(),
        tkIndicators.buyStopLossPrice, tkIndicators.sellStopLossPrice, tkIndicators.dslType);

    // Check BBS exit condition for 1M timeframe
    if (shouldExitOnBBSTrend(context, tkIndicators)) {
        result.code = SUCCESS;
        return result;
    }

    // Get OrderManager instance
    OrderManager& orderMgr = OrderManager::getInstance();

    // Modify buy orders if any exist
    if (orderMgr.totalOpenOrders(BUY) > 0) {
        if (!modifyOrders(context, tkIndicators, BUY)) {
            result.code = STRATEGY_FAILED_TO_MODIFY_ORDERS;
            pantheios_logputs(PANTHEIOS_SEV_ERROR,
                (PAN_CHAR_T*)"TakeOver: Failed to modify buy orders");
        }
    }

    // Modify sell orders if any exist
    if (orderMgr.totalOpenOrders(SELL) > 0) {
        if (!modifyOrders(context, tkIndicators, SELL)) {
            result.code = STRATEGY_FAILED_TO_MODIFY_ORDERS;
            pantheios_logputs(PANTHEIOS_SEV_ERROR,
                (PAN_CHAR_T*)"TakeOver: Failed to modify sell orders");
        }
    }

    return result;
}

void TakeOverStrategy::updateResults(const StrategyContext& context,
                                     const StrategyResult& result) {
    // TakeOver doesn't generate new orders, only modifies existing ones
    // No results to update
}

void TakeOverStrategy::loadTakeOverIndicators(const StrategyContext& context,
                                              TakeOverIndicators& indicators) const {
    // Get indicator values from C functions via Indicators helper
    indicators.dailyATR = iAtr(DAILY_RATES, 
        static_cast<int>(context.getSetting(ATR_AVERAGING_PERIOD)), 1);

    // Get Bollinger Band stop
    iBBandStop(PRIMARY_RATES,
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_1)),  // BBS_PERIOD
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_2)),  // BBS_DEVIATION
        &indicators.bbsTrend,
        &indicators.bbsStopPrice,
        &indicators.bbsIndex);

    // Get position from settings
    indicators.position = context.getSetting(ADDITIONAL_PARAM_4);  // POSITION

    // Get daily price data
    indicators.preHigh = iHigh(DAILY_RATES, 1);
    indicators.preLow = iLow(DAILY_RATES, 1);
    indicators.preClose = iClose(DAILY_RATES, 1);

    // Calculate 2-day high/low
    indicators.pre2DaysHigh = indicators.preHigh;
    if (iHigh(DAILY_RATES, 2) > indicators.preHigh) {
        indicators.pre2DaysHigh = iHigh(DAILY_RATES, 2);
    }

    indicators.pre2DaysLow = indicators.preLow;
    if (iLow(DAILY_RATES, 2) < indicators.preLow) {
        indicators.pre2DaysLow = iLow(DAILY_RATES, 2);
    }

    // Get adjustment and DSL type
    indicators.adjust = context.getSetting(ADDITIONAL_PARAM_3);  // BBS_ADJUSTPOINTS
    indicators.dslType = static_cast<DslType>(
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_7))  // DSL_TYPE
    );

    // Get 1H MA200
    indicators.movingAverage200M = iMA(3, HOURLY_RATES, 200, 1);

    // Calculate stop loss prices based on DSL type
    switch (indicators.dslType) {
        case EXIT_DSL_NONE:
            // Smart auto DSL mode - use 2-day HL or 1H MA200 (whichever is closer)
            indicators.buyStopLossPrice = indicators.pre2DaysLow;
            if (indicators.movingAverage200M > indicators.buyStopLossPrice) {
                indicators.buyStopLossPrice = indicators.movingAverage200M;
            }

            indicators.sellStopLossPrice = indicators.pre2DaysHigh;
            if (indicators.movingAverage200M < indicators.sellStopLossPrice) {
                indicators.sellStopLossPrice = indicators.movingAverage200M;
            }
            break;

        case EXIT_DSL_1_DAY_HL:
            indicators.buyStopLossPrice = indicators.preLow;
            indicators.sellStopLossPrice = indicators.preHigh;
            break;

        case EXIT_DSL_2_DAY_HL:
            indicators.buyStopLossPrice = indicators.pre2DaysLow;
            indicators.sellStopLossPrice = indicators.pre2DaysHigh;
            break;

        case EXIT_DSL_1H_M200:
            indicators.buyStopLossPrice = indicators.movingAverage200M;
            indicators.sellStopLossPrice = indicators.movingAverage200M;
            break;

        case EXIT_DSL_BBS:
            indicators.buyStopLossPrice = indicators.bbsStopPrice;
            indicators.sellStopLossPrice = indicators.bbsStopPrice;
            break;

        case EXIT_DSL_DAILY_ATR:
            indicators.buyStopLossPrice = indicators.dailyATR;
            indicators.sellStopLossPrice = indicators.dailyATR;
            break;

        default:
            // No change to SL
            indicators.buyStopLossPrice = -1;
            indicators.sellStopLossPrice = -1;
            break;
    }

    // Set UI values for display
    addValueToUI("BBSStopPrice", indicators.bbsStopPrice);
    addValueToUI("DailyATR", indicators.dailyATR);
    addValueToUI("1DayHigh", indicators.preHigh);
    addValueToUI("1DayLow", indicators.preLow);
    addValueToUI("2DaysHigh", indicators.pre2DaysHigh);
    addValueToUI("2DaysLow", indicators.pre2DaysLow);
    addValueToUI("1H200M", indicators.movingAverage200M);
    addValueToUI("DSLType", static_cast<double>(indicators.dslType));
    addValueToUI("BuySLP", indicators.buyStopLossPrice);
    addValueToUI("SellSLP", indicators.sellStopLossPrice);
    addValueToUI("Position", indicators.position);
}

bool TakeOverStrategy::modifyOrders(const StrategyContext& context,
                                   const TakeOverIndicators& indicators,
                                   int orderType) const {
    OrderManager& orderMgr = OrderManager::getInstance();
    double stopLoss = 0.0;

    if (orderType == BUY && indicators.buyStopLossPrice > 0) {
        // Calculate stop loss distance for buy orders
        stopLoss = std::fabs(context.getAsk(0) - indicators.buyStopLossPrice + indicators.adjust);
        
        // Modify all buy orders with new stop loss
        if (!orderMgr.modifyTradeEasy(BUY, -1, stopLoss, -1)) {
            return false;
        }
    }
    else if (orderType == SELL && indicators.sellStopLossPrice > 0) {
        // Calculate stop loss distance for sell orders
        stopLoss = std::fabs(indicators.sellStopLossPrice - context.getBid(0) + indicators.adjust);
        
        // Modify all sell orders with new stop loss
        if (!orderMgr.modifyTradeEasy(SELL, -1, stopLoss, -1)) {
            return false;
        }
    }

    return true;
}

bool TakeOverStrategy::shouldExitOnBBSTrend(const StrategyContext& context,
                                           const TakeOverIndicators& indicators) const {
    // Only check BBS exit for 1M timeframe
    if (indicators.dslType != EXIT_DSL_BBS || 
        static_cast<int>(context.getSetting(TIMEFRAME)) != 1) {
        return false;
    }

    OrderManager& orderMgr = OrderManager::getInstance();
    int shift1Index = context.getBarsTotal(0) - 2;

    // Exit long positions on BBS trend reversal to down
    if (orderMgr.totalOpenOrders(BUY) > 0 && 
        indicators.bbsTrend == DOWN && 
        indicators.bbsIndex == shift1Index &&
        context.getAsk(0) >= indicators.position) {
        
        orderMgr.closeAllLongs();
        pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"TakeOver: Closed all longs on BBS trend reversal");
        return true;
    }

    // Exit short positions on BBS trend reversal to up
    if (orderMgr.totalOpenOrders(SELL) > 0 && 
        indicators.bbsTrend == UP && 
        indicators.bbsIndex == shift1Index &&
        context.getBid(0) <= indicators.position) {
        
        orderMgr.closeAllShorts();
        pantheios_logputs(PANTHEIOS_SEV_INFORMATIONAL,
            (PAN_CHAR_T*)"TakeOver: Closed all shorts on BBS trend reversal");
        return true;
    }

    return false;
}

} // namespace trading
