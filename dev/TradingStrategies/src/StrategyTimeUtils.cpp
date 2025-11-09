#include "StrategyTimeUtils.hpp"
#include <cstring>

#if defined(_WIN32)
  #define HAS_GMTIME_S 1
  #include <cstdlib>
#endif

namespace trading { namespace timeutils {

bool gmtBreakdown(std::time_t ts, std::tm &outTm) {
#if HAS_GMTIME_S
    errno_t err = gmtime_s(&outTm, &ts);
    return err == 0;
#elif defined(__unix__) || defined(__APPLE__)
    return gmtime_r(&ts, &outTm) != nullptr;
#else
    // Fallback (not thread-safe, but we copy immediately)
    std::tm *temp = std::gmtime(&ts);
    if (!temp) return false;
    outTm = *temp;
    return true;
#endif
}

std::string formatTime(std::time_t ts, const char *pattern) {
    std::tm tm{};
    if (!gmtBreakdown(ts, tm)) return std::string();
    char buffer[64];
    if (std::strftime(buffer, sizeof(buffer), pattern, &tm) == 0) {
        return std::string();
    }
    return std::string(buffer);
}

std::string timeToUIString(std::time_t ts) {
    // Legacy default pattern from RecordBars string mode
    return formatTime(ts, " %d/%m/%y %H:%M");
}

}} // namespace trading::timeutils
