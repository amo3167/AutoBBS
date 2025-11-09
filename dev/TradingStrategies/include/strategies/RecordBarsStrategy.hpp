#ifndef RECORDBARSSTRATEGY_HPP
#define RECORDBARSSTRATEGY_HPP

#include "BaseStrategy.hpp"
#include <ctime>
#include <cstdio>

namespace trading {

/// @brief Bar recorder strategy - records each bar's OHLCV data to CSV file
/// @details Simple strategy that writes bar data to CSV for analysis. Supports
///          three date formats: integer timestamp, string date, and R-compatible format.
class RecordBarsStrategy : public BaseStrategy {
public:
    /// @brief Record date format modes
    enum RecordMode {
        RECORD_DATE_AS_INTEGER = 0,  ///< Unix timestamp (integer)
        RECORD_DATE_AS_STRING = 1,   ///< Human-readable date (dd/mm/yy HH:MM)
        RECORD_DATE_FOR_R = 2        ///< R-compatible format (YYYY-MM-DD)
    };

    /// @brief Constructor
    RecordBarsStrategy();

    /// @brief Get strategy name
    /// @return Strategy name string
    std::string getName() const;

protected:
    /// @brief Load required indicators (none required for RecordBars)
    /// @param context Strategy context
    /// @return NULL (no indicators needed)
    Indicators* loadIndicators(const StrategyContext& context);

    /// @brief Execute strategy logic - record bar to CSV
    /// @param context Strategy context
    /// @param indicators Indicators instance (unused)
    /// @return StrategyResult with success/failure status
    StrategyResult executeStrategy(const StrategyContext& context, Indicators* indicators);

    /// @brief Update results (no-op for RecordBars)
    /// @param context Strategy context
    /// @param result Strategy result
    void updateResults(const StrategyContext& context, const StrategyResult& result);

    /// @brief Indicates that this strategy doesn't require indicators
    /// @return false
    bool requiresIndicators() const { return false; }

private:
    /// @brief Build CSV file path for recording
    /// @param context Strategy context
    /// @param filePath Output buffer for file path
    /// @param maxLen Maximum buffer length
    /// @return true on success, false on error
    bool buildFilePath(const StrategyContext& context, char* filePath, size_t maxLen) const;

    /// @brief Write bar data to CSV file
    /// @param filePath Path to CSV file
    /// @param context Strategy context
    /// @param shift Bar index to record (typically shift 1 = previous closed bar)
    /// @return true on success, false on file I/O error
    bool writeBarToCSV(const char* filePath, const StrategyContext& context, int shift) const;

    /// @brief Format timestamp for CSV output
    /// @param timestamp Unix timestamp
    /// @param mode Record mode (determines format)
    /// @return Formatted timestamp string
    std::string formatTimestamp(time_t timestamp, RecordMode mode) const;
};

} // namespace trading

#endif // RECORDBARSSTRATEGY_HPP
