#ifndef TAKEOVERSTRATEGY_HPP
#define TAKEOVERSTRATEGY_HPP

#include "BaseStrategy.hpp"

namespace trading {

/// @brief TakeOver strategy - manages stop losses for existing open orders
/// @details Takes over manual or existing orders and dynamically adjusts stop losses
///          based on various DSL (Dynamic Stop Loss) modes including daily high/low,
///          moving averages, Bollinger Band stops, and ATR-based stops.
class TakeOverStrategy : public BaseStrategy {
public:
    /// @brief DSL (Dynamic Stop Loss) types for order management
    enum DslType {
        EXIT_DSL_NONE = 0,        ///< Smart auto DSL mode (2-day HL or 1H MA200)
        EXIT_DSL_1_DAY_HL = 1,    ///< Use previous day high/low
        EXIT_DSL_2_DAY_HL = 2,    ///< Use 2-day high/low
        EXIT_DSL_1H_M200 = 3,     ///< Use 1H MA200
        EXIT_DSL_BBS = 4,          ///< Use Bollinger Band stop
        EXIT_DSL_DAILY_ATR = 5    ///< Use Daily ATR
    };

    /// @brief Order direction trend
    enum Trend {
        DOWN = -1,
        UP = 1
    };

    TakeOverStrategy();
    
    std::string getName() const;
    
protected:
    /// @brief Load required indicators
    /// @param context Strategy context
    /// @return NULL (indicators loaded internally)
    Indicators* loadIndicators(const StrategyContext& context);

    /// @brief Execute strategy - manage existing orders
    /// @param context Strategy execution context
    /// @param indicators Loaded indicators (unused, can be NULL)
    /// @return Strategy result with success/failure code
    StrategyResult executeStrategy(const StrategyContext& context, Indicators* indicators);
    
    /// @brief Update results (no-op for TakeOver)
    /// @param context Strategy execution context
    /// @param result Strategy execution result
    void updateResults(const StrategyContext& context, const StrategyResult& result);
    
    /// @brief This strategy doesn't require Indicators class
    /// @return false
    bool requiresIndicators() const { return false; }

private:
    /// @brief Internal indicators structure
    struct TakeOverIndicators {
        double position;              ///< Position setting from parameters
        int bbsTrend;                 ///< Bollinger Band stop trend
        double bbsStopPrice;          ///< Bollinger Band stop price
        int bbsIndex;                 ///< Bollinger Band stop index
        double dailyATR;              ///< Daily ATR value
        double preHigh;               ///< Previous day high
        double preLow;                ///< Previous day low
        double preClose;              ///< Previous day close
        double pre2DaysHigh;          ///< 2-day high
        double pre2DaysLow;           ///< 2-day low
        double movingAverage200M;     ///< 1H MA200
        double buyStopLossPrice;      ///< Buy order stop loss price
        double sellStopLossPrice;     ///< Sell order stop loss price
        double adjust;                ///< Adjustment points
        DslType dslType;              ///< DSL type from settings
    };

    /// @brief Load all required indicators
    /// @param context Strategy context
    /// @param indicators Output indicators structure
    void loadTakeOverIndicators(const StrategyContext& context, TakeOverIndicators& indicators) const;

    /// @brief Modify orders with new stop loss levels
    /// @param context Strategy context
    /// @param indicators Indicator values
    /// @param orderType Order type (BUY or SELL)
    /// @return true on success
    bool modifyOrders(const StrategyContext& context, const TakeOverIndicators& indicators, int orderType) const;

    /// @brief Check if should exit based on BBS trend reversal
    /// @param context Strategy context
    /// @param indicators Indicator values
    /// @return true if should exit
    bool shouldExitOnBBSTrend(const StrategyContext& context, const TakeOverIndicators& indicators) const;
};

} // namespace trading

#endif // TAKEOVERSTRATEGY_HPP
