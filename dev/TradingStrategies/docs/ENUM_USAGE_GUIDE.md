# Strategy Enumeration Usage Guide

## Overview

This document explains how to use the `SplitTradeMode` and `TakeProfitMode` enumerations defined in `StrategyTypes.h` to replace magic numbers in strategy code.

## Enumerations

### SplitTradeMode

Defines how a strategy splits position entries across multiple orders.

```c
#include "StrategyTypes.h"

// Instead of:
pIndicators->splitTradeMode = 19;

// Use:
pIndicators->splitTradeMode = STM_ATR_4H;
```

**Available Values**:

| Enum Constant | Value | Description |
|---------------|-------|-------------|
| `STM_DEFAULT` | 0 | Single order, no splitting |
| `STM_BREAKOUT` | 3 | BBS breakout: partial at resistance, remainder trailing |
| `STM_LIMIT` | 4 | Limit/range-bound: multiple entries at S/R levels |
| `STM_PIVOT` | 5 | Pivot-based: tiered entries at daily pivot levels |
| `STM_RETREAT` | 6 | Middle retreat phase: pullback entry layering |
| `STM_KEYK` | 7 | KeyK reversal pattern: 1/3 splits at TP multiples |
| `STM_SHELLINGTON` | 9 | Shellington indicator-based split |
| `STM_SHORTTERM` | 10 | Short-term momentum: 2-part split |
| `STM_WEEKLY_BEGIN` | 11 | Weekly beginning phase: single entry at R1/S1 |
| `STM_ATR_4H` | 19 | 4H ATR-based swing: daily gap + ATR TP targets |
| `STM_ATR_4H_ALT` | 20 | 4H ATR alternative: 40% weekly ATR threshold |
| `STM_MACD_SHORT` | 31 | MACD short-term: break-even ladder |

### TakeProfitMode

Specifies how the strategy calculates take-profit levels.

```c
// Instead of:
pIndicators->tpMode = 3;

// Use:
pIndicators->tpMode = TP_ATR_RANGE;
```

**Available Values**:

| Enum Constant | Value | Description |
|---------------|-------|-------------|
| `TP_STATIC` | 0 | Fixed TP set manually or via R:R ratio |
| `TP_ATR_RANGE` | 3 | TP based on ATR multiples (dynamic volatility) |
| `TP_LADDER_BE` | 4 | Ladder: partial close at TP, move to break-even |

## Migration Examples

### Before (Magic Numbers)

```c
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(
    StrategyParams* pParams,
    Indicators* pIndicators,
    Base_Indicators* pBase_Indicators)
{
    // Magic numbers - unclear intent
    pIndicators->splitTradeMode = 20;
    pIndicators->tpMode = 3;
    
    // ... strategy logic ...
}
```

### After (Named Constants)

```c
#include "StrategyTypes.h"

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(
    StrategyParams* pParams,
    Indicators* pIndicators,
    Base_Indicators* pBase_Indicators)
{
    // Clear intent - self-documenting code
    pIndicators->splitTradeMode = STM_ATR_4H_ALT;
    pIndicators->tpMode = TP_ATR_RANGE;
    
    // ... strategy logic ...
}
```

## Switch Statement Pattern

When processing different modes in order management code:

```c
#include "StrategyTypes.h"

void executeSplitOrders(StrategyParams* pParams, Indicators* pIndicators) {
    switch (pIndicators->splitTradeMode) {
        case STM_DEFAULT:
            // Single order logic
            break;
            
        case STM_BREAKOUT:
            // Breakout split logic
            break;
            
        case STM_ATR_4H:
        case STM_ATR_4H_ALT:
            // ATR-based swing split logic
            break;
            
        default:
            pantheios_logprintf(PANTHEIOS_SEV_WARNING,
                "Unknown splitTradeMode: %d", pIndicators->splitTradeMode);
            break;
    }
}
```

## C++ Usage

The enums are wrapped in `extern "C"` so they work seamlessly in C++ code:

```cpp
#include "StrategyTypes.h"

void TrendStrategy::execute(StrategyContext& context) {
    Indicators* indicators = context.getIndicators();
    
    // Type-safe enum usage
    indicators->splitTradeMode = STM_ATR_4H;
    indicators->tpMode = TP_ATR_RANGE;
    
    // Compile-time type checking prevents invalid assignments
    // indicators->splitTradeMode = 999;  // Would compile but lose type safety
    // indicators->splitTradeMode = TP_STATIC;  // Type mismatch caught by compiler
}
```

## Logging and Debugging

For readable log output, consider creating helper functions:

```c
const char* splitTradeModeToString(int mode) {
    switch (mode) {
        case STM_DEFAULT: return "DEFAULT";
        case STM_BREAKOUT: return "BREAKOUT";
        case STM_LIMIT: return "LIMIT";
        case STM_PIVOT: return "PIVOT";
        case STM_RETREAT: return "RETREAT";
        case STM_KEYK: return "KEYK";
        case STM_SHELLINGTON: return "SHELLINGTON";
        case STM_SHORTTERM: return "SHORTTERM";
        case STM_WEEKLY_BEGIN: return "WEEKLY_BEGIN";
        case STM_ATR_4H: return "ATR_4H";
        case STM_ATR_4H_ALT: return "ATR_4H_ALT";
        case STM_MACD_SHORT: return "MACD_SHORT";
        default: return "UNKNOWN";
    }
}

// Usage in logging:
pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL,
    "Strategy using splitTradeMode: %s (%d)",
    splitTradeModeToString(pIndicators->splitTradeMode),
    pIndicators->splitTradeMode);
```

## Benefits

1. **Self-Documenting Code**: `STM_ATR_4H` is clearer than `19`
2. **IDE Support**: Autocomplete shows available options
3. **Maintainability**: Changing a mode's numeric value requires updating only one place
4. **Type Safety**: Compiler can catch some misuse patterns
5. **Searchability**: Easy to find all uses of a specific mode

## Related Files

- **Definition**: `dev/TradingStrategies/include/StrategyTypes.h`
- **Documentation**: `spec/refactor/API_DISCOVERY.md` (Section 4: TrendStrategy Empirical Pattern Catalogue)
- **Usage Examples**: `dev/TradingStrategies/src/strategies/TrendStrategy.c`

## Version History

- **2025-11-09**: Initial enum definitions added
  - `SplitTradeMode` with 12 constants
  - `TakeProfitMode` with 3 constants
  - Documented in API_DISCOVERY.md with empirical usage patterns
