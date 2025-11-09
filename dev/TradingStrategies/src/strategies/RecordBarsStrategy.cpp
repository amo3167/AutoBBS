#include "strategies/RecordBarsStrategy.hpp"
#include "Indicators.hpp"
#include <cstring>
#include <cstdio>
#include <ctime>
#include <string>
#include <sstream>

// Include C functions we need to call
extern "C" {
    #include "StrategyUserInterface.h"
    #include "AsirikuyTime.h"
    #include "AsirikuyDefines.h"
}

namespace trading {

namespace {
    /// @brief Helper to convert time_t to std::string
    std::string timeToString(time_t timestamp) {
        char buffer[MAX_TIME_STRING_SIZE] = "";
        safe_timeString(buffer, timestamp);
        return std::string(buffer);
    }
}

RecordBarsStrategy::RecordBarsStrategy()
    : BaseStrategy(RECORD_BARS) {
}

std::string RecordBarsStrategy::getName() const {
    return "RecordBars";
}

Indicators* RecordBarsStrategy::loadIndicators(const StrategyContext& context) {
    // RecordBars doesn't use any indicators
    return NULL;
}

StrategyResult RecordBarsStrategy::executeStrategy(const StrategyContext& context,
                                                   Indicators* indicators) {
    StrategyResult result;
    result.code = SUCCESS;
    result.generatedOrders = 0;
    result.pnlImpact = 0.0;

    // Build file path for CSV output
    std::string filePath;
    char filePathBuffer[MAX_FILE_PATH_CHARS];
    if (!buildFilePath(context, filePathBuffer, MAX_FILE_PATH_CHARS)) {
        result.code = STRATEGY_FAILED_TO_RECORD_DATA;
        return result;
    }
    filePath = filePathBuffer;

    // Write the previous bar (shift 1) to CSV
    // We use shift 1 because shift 0 (current bar) is not yet complete
    int shift = context.getBarsTotal(0) - 2;  // shift 1 in array indexing
    if (shift < 0) {
        // Not enough bars yet - not an error, just nothing to record
        result.code = SUCCESS;
        return result;
    }

    if (!writeBarToCSV(filePath.c_str(), context, shift)) {
        result.code = STRATEGY_FAILED_TO_RECORD_DATA;
        return result;
    }

    result.code = SUCCESS;
    return result;
}

void RecordBarsStrategy::updateResults(const StrategyContext& context,
                                       const StrategyResult& result) {
    // RecordBars doesn't generate trading signals
    // No results to update
}

bool RecordBarsStrategy::buildFilePath(const StrategyContext& context, 
                                       char* filePath, size_t maxLen) const {
    char tempFolderPath[MAX_FILE_PATH_CHARS] = "";
    char timeframeStr[32];
    
    // Get temp folder path from C function
    if (requestTempFileFolderPath(tempFolderPath) != ASIRIKUY_SUCCESS) {
        return false;
    }

    // Get timeframe setting
    int timeframe = static_cast<int>(context.getSetting(TIMEFRAME));
    sprintf(timeframeStr, "%d", timeframe);

    // Build path: <tempfolder>/<symbol>_<timeframe>.csv
    if (std::strlen(tempFolderPath) + std::strlen(context.getSymbol()) + 
        std::strlen(timeframeStr) + 10 > maxLen) {
        return false;  // Path too long
    }

    std::strcpy(filePath, tempFolderPath);
    std::strcat(filePath, context.getSymbol());
    std::strcat(filePath, "_");
    std::strcat(filePath, timeframeStr);
    std::strcat(filePath, ".csv");

    return true;
}

bool RecordBarsStrategy::writeBarToCSV(const char* filePath, 
                                       const StrategyContext& context, 
                                       int shift) const {
    // Open file for appending
    FILE* fp = std::fopen(filePath, "a");
    if (fp == NULL) {
        return false;
    }

    // Seek to end
    std::fseek(fp, 0, SEEK_END);

    // Get record mode from ADDITIONAL_PARAM_1
    RecordMode mode = static_cast<RecordMode>(
        static_cast<int>(context.getSetting(ADDITIONAL_PARAM_1))
    );

    // Get bar data
    time_t timestamp = context.getTime(0, shift);
    double open = context.getOpen(0, shift);
    double high = context.getHigh(0, shift);
    double low = context.getLow(0, shift);
    double close = context.getClose(0, shift);
    double volume = context.getVolume(0, shift);

    // Format and write based on mode
    switch (mode) {
        case RECORD_DATE_AS_INTEGER:
            std::fprintf(fp, "%d, %lf, %lf, %lf, %lf, %lf\n",
                        static_cast<int>(timestamp), open, high, low, close, volume);
            break;

        case RECORD_DATE_AS_STRING: {
            char timeStr[MAX_TIME_STRING_SIZE];
            formatTimestamp(timestamp, mode, timeStr, MAX_TIME_STRING_SIZE);
            std::fprintf(fp, "%s, %lf, %lf, %lf, %lf, %lf\n",
                        timeStr, open, high, low, close, volume);
            break;
        }

        case RECORD_DATE_FOR_R: {
            char timeStr[MAX_TIME_STRING_SIZE];
            formatTimestamp(timestamp, mode, timeStr, MAX_TIME_STRING_SIZE);
            std::fprintf(fp, "%s, %lf, %lf, %lf, %lf, %lf\n",
                        timeStr, open, high, low, close, volume);
            break;
        }

        default:
            // Invalid mode - do nothing
            std::fclose(fp);
            return false;
    }

    std::fclose(fp);
    return true;
}

void RecordBarsStrategy::formatTimestamp(time_t timestamp, RecordMode mode,
                                        char* buffer, size_t maxLen) const {
    struct tm timeInfo;
    
    // Use safe_gmtime from AsirikuyTime.h
    safe_gmtime(&timeInfo, timestamp);

    switch (mode) {
        case RECORD_DATE_AS_STRING:
            // Format: dd/mm/yy HH:MM
            std::strftime(buffer, maxLen - 1, " %d/%m/%y %H:%M", &timeInfo);
            break;

        case RECORD_DATE_FOR_R:
            // Format: YYYY-MM-DD
            std::strftime(buffer, maxLen - 1, " %Y-%m-%d", &timeInfo);
            break;

        default:
            buffer[0] = '\0';
            break;
    }
}

} // namespace trading
