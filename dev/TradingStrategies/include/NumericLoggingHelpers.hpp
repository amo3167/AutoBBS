#ifndef NUMERIC_LOGGING_HELPERS_HPP
#define NUMERIC_LOGGING_HELPERS_HPP

/** @file NumericLoggingHelpers.h
 *  @brief C++ helper class for numeric-to-string conversion in logging contexts
 *
 *  Reduces sprintf/snprintf boilerplate throughout strategy code by providing
 *  reusable formatters with std::stringstream. Compatible with Pantheios.
 *
 *  Usage:
 *      using LogFmt = NumericLogFormatter;
 *      pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
 *          (PAN_CHAR_T*)"executionTrend=%s", LogFmt::integer(trend));
 *
 *      pantheios_logprintf(PANTHEIOS_SEV_DEBUG,
 *          (PAN_CHAR_T*)"entryPrice=%s, stopLoss=%s, mode=%s",
 *          LogFmt::decimal(entryPrice, 5),
 *          LogFmt::decimal(stopLoss, 5),
 *          LogFmt::splitTradeMode(mode));
 */

#include <string>
#include <sstream>
#include <iomanip>

/** @brief Thread-safe numeric formatter using std::stringstream
 *
 *  Static methods return thread-local std::string objects for zero-copy
 *  formatting. Each thread maintains rotating buffer of strings to support
 *  multiple formatters in single log statement.
 */
class NumericLogFormatter {
public:
    /** @brief Format integer to string
     *  @param value Integer value
     *  @return C-string pointer (valid until next formatter call)
     */
    static const char* integer(int value);
    
    /** @brief Format double with specified precision
     *  @param value Double value
     *  @param precision Decimal places (default 2)
     *  @return C-string pointer (valid until next formatter call)
     */
    static const char* decimal(double value, int precision = 2);
    
    /** @brief Format boolean as "true"/"false"
     *  @param value Boolean value
     *  @return Static string literal
     */
    static const char* boolean(bool value);
    
    /** @brief Format SplitTradeMode enum to human-readable name
     *  @param mode Enum value (e.g., 19 → "STM_ATR_4H")
     *  @return String name or numeric fallback
     */
    static const char* splitTradeMode(int mode);
    
    /** @brief Format TakeProfitMode enum to human-readable name
     *  @param mode Enum value (e.g., 3 → "TP_ATR_RANGE")
     *  @return String name or numeric fallback
     */
    static const char* takeProfitMode(int mode);

private:
    static constexpr size_t BUFFER_COUNT = 8;
    
    /** @brief Get next thread-local string buffer (rotating) */
    static std::string& getNextBuffer();
};

// Convenience alias for shorter syntax
using LogFmt = NumericLogFormatter;

#endif /* NUMERIC_LOGGING_HELPERS_HPP */
