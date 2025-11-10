#include "Precompiled.h"
#include "NumericLoggingHelpers.hpp"
#include "StrategyTypes.h"
#include <sstream>
#include <iomanip>
#include <vector>

/* -------------------------------------------------------------------------
 * Numeric Logging Helpers Implementation (C++)
 * -------------------------------------------------------------------------
 * Thread-local std::string buffer rotation scheme provides:
 * - Type-safe formatting via std::stringstream
 * - Zero heap allocation after initial buffer creation
 * - Support for multiple formatters in single log statement
 *
 * Example:
 *     pantheios_logprintf(...,
 *         LogFmt::integer(a), LogFmt::decimal(b, 5), LogFmt::splitTradeMode(c));
 *
 * Each formatter call uses next buffer slot (wraps after BUFFER_COUNT).
 * ------------------------------------------------------------------------- */

// Thread-local storage for rotating string buffers
thread_local std::vector<std::string> g_formatBuffers(NumericLogFormatter::BUFFER_COUNT);
thread_local size_t g_currentBufferIndex = 0;

std::string& NumericLogFormatter::getNextBuffer() {
    std::string& buffer = g_formatBuffers[g_currentBufferIndex];
    g_currentBufferIndex = (g_currentBufferIndex + 1) % BUFFER_COUNT;
    return buffer;
}

const char* NumericLogFormatter::integer(int value) {
    std::string& buffer = getNextBuffer();
    std::ostringstream oss;
    oss << value;
    buffer = oss.str();
    return buffer.c_str();
}

const char* NumericLogFormatter::decimal(double value, int precision) {
    std::string& buffer = getNextBuffer();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    buffer = oss.str();
    return buffer.c_str();
}

const char* NumericLogFormatter::boolean(bool value) {
    return value ? "true" : "false";
}

const char* NumericLogFormatter::splitTradeMode(int mode) {
    switch (mode) {
        case STM_DEFAULT: return "STM_DEFAULT";
        case STM_BREAKOUT: return "STM_BREAKOUT";
        case STM_LIMIT: return "STM_LIMIT";
        case STM_PIVOT: return "STM_PIVOT";
        case STM_RETREAT: return "STM_RETREAT";
        case STM_KEYK: return "STM_KEYK";
        case STM_SHELLINGTON: return "STM_SHELLINGTON";
        case STM_SHORTTERM: return "STM_SHORTTERM";
        case STM_WEEKLY_BEGIN: return "STM_WEEKLY_BEGIN";
        case STM_ATR_4H: return "STM_ATR_4H";
        case STM_ATR_4H_ALT: return "STM_ATR_4H_ALT";
        case STM_MACD_SHORT: return "STM_MACD_SHORT";
        default: return integer(mode); // Fallback to numeric
    }
}

const char* NumericLogFormatter::takeProfitMode(int mode) {
    switch (mode) {
        case TP_STATIC: return "TP_STATIC";
        case TP_ATR_RANGE: return "TP_ATR_RANGE";
        case TP_LADDER_BE: return "TP_LADDER_BE";
        default: return integer(mode);
    }
}
