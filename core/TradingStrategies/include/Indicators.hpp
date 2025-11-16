// Indicators.hpp
// Wrapper class for indicator calculations (stub implementation)

#ifndef INDICATORS_HPP
#define INDICATORS_HPP

namespace trading {

/// Wrapper for indicator calculations
/// Provides access to technical indicators (MACD, RSI, Bollinger Bands, etc.)
/// Currently a stub - full implementation in Phase 1 Week 2
class Indicators {
public:
    Indicators() {}
    ~Indicators() {}
    
private:
    // Non-copyable (C++03 idiom)
    Indicators(const Indicators&);
    Indicators& operator=(const Indicators&);
};

} // namespace trading

#endif // INDICATORS_HPP
