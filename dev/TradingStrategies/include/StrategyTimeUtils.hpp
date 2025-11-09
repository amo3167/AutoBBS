#ifndef STRATEGY_TIME_UTILS_HPP
#define STRATEGY_TIME_UTILS_HPP

#include <string>
#include <ctime>

namespace trading {
namespace timeutils {
    /**
     * Reimplemented GMT conversion (thread-safe, no reliance on legacy safe_gmtime).
     * Uses std::gmtime_r when available, otherwise emulates with std::gmtime and copy.
     */
    bool gmtBreakdown(std::time_t ts, std::tm &outTm);

    /**
     * Format a timestamp using a strftime pattern; returns empty string if pattern fails.
     */
    std::string formatTime(std::time_t ts, const char *pattern);

    /**
     * UI-friendly default time string (mirrors legacy safe_timeString usage: " %d/%m/%y %H:%M").
     */
    std::string timeToUIString(std::time_t ts);
}
}

#endif // STRATEGY_TIME_UTILS_HPP
