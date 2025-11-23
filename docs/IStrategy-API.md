# IStrategy Interface Documentation

## Overview

`IStrategy` is a pure virtual interface (abstract base class) that defines the contract for all trading strategy implementations. It follows the Strategy pattern, allowing different trading algorithms to be swapped at runtime through polymorphism.

**Namespace:** `trading`  
**Header:** `include/IStrategy.hpp`  
**Type:** Pure Virtual Interface (Abstract Base Class)

---

## Design Philosophy

### Interface Segregation

IStrategy defines only the essential methods that ALL strategies must implement:
- `execute()` - Core strategy logic
- `getName()` - Human-readable identification
- `getId()` - Programmatic identification
- `validate()` - Pre-execution validation

### Strategy Pattern Benefits

1. **Runtime Polymorphism**: Strategies can be selected at runtime based on configuration
2. **Testability**: Easy to mock strategies for unit testing
3. **Extensibility**: New strategies added without modifying existing code
4. **Encapsulation**: Each strategy encapsulates its own logic

### Return Code Compatibility

Uses `AsirikuyReturnCode` enum for C API compatibility, allowing seamless integration with the legacy C codebase.

---

## Interface Definition

```cpp
namespace trading {

class IStrategy {
public:
    virtual ~IStrategy() {}
    
    virtual AsirikuyReturnCode execute(const StrategyContext& context) = 0;
    virtual std::string getName() const = 0;
    virtual StrategyId getId() const = 0;
    virtual bool validate(const StrategyContext& context) const = 0;
    
protected:
    static AsirikuyReturnCode toReturnCode(bool success);
};

} // namespace trading
```

---

## Method Reference

### Destructor

```cpp
virtual ~IStrategy() {}
```

**Purpose:** Ensures proper cleanup of derived classes through base pointer.

**Note:** Virtual destructor is mandatory for interfaces to prevent resource leaks when deleting derived objects through base pointers.

---

### execute()

```cpp
virtual AsirikuyReturnCode execute(const StrategyContext& context) = 0;
```

**Purpose:** Main entry point for strategy execution.

**Parameters:**
- `context` - Strategy execution context (read-only) containing market data, account info, and results structure

**Returns:** `AsirikuyReturnCode`
- `SUCCESS` (0) - Strategy executed successfully
- `STRATEGY_INVALID_PARAMETERS` - Invalid inputs detected
- `STRATEGY_FAILED_TO_LOAD_INDICATORS` - Indicator loading failed
- `STRATEGY_INTERNAL_ERROR` - Internal strategy error
- `STRATEGY_NO_TRADING_SIGNAL` - No signal generated (informational, not an error)

**Pure Virtual:** Must be implemented by all derived classes.

**Execution Flow:**
1. Validate inputs (or rely on validate() being called first)
2. Load required indicators
3. Analyze market conditions
4. Generate trading signals
5. Write signals to `context.getResults()`
6. Place/modify/close orders (if applicable)
7. Return status code

**Example Implementation:**
```cpp
class MyStrategy : public IStrategy {
public:
    AsirikuyReturnCode execute(const StrategyContext& context) {
        // Get market data
        double bid = context.getBid(0);
        double ask = context.getAsk(0);
        
        // Analyze conditions
        const Rates& daily = context.getRates(B_DAILY_RATES);
        bool bullish = daily.close[0] > daily.close[1];
        
        // Generate signal
        StrategyResults* results = context.getResults();
        if (bullish && someCondition) {
            results[0].tradingSignals = BUY_SIGNAL;
            results[0].lots = calculateLots(context);
            results[0].entryPrice = ask;
        }
        
        return SUCCESS;
    }
};
```

**Thread Safety:** NOT thread-safe. Each thread should have its own strategy instance.

---

### getName()

```cpp
virtual std::string getName() const = 0;
```

**Purpose:** Returns human-readable strategy name for logging and UI display.

**Returns:** `std::string` - Descriptive name (e.g., "MACD Daily Strategy", "RecordBars", "Screening")

**Pure Virtual:** Must be implemented by all derived classes.

**Guidelines:**
- Use clear, descriptive names
- Keep names concise (3-5 words)
- Use consistent naming convention
- Include timeframe if relevant (e.g., "Daily", "Weekly")

**Example Implementation:**
```cpp
class MACDDailyStrategy : public IStrategy {
public:
    std::string getName() const {
        return "MACD Daily Strategy";
    }
};
```

**Usage:**
```cpp
IStrategy* strategy = factory.createStrategy(AUTOBBS);
pantheios_log(PANTHEIOS_SEV_INFO, 
              "Executing strategy: ", strategy->getName().c_str());
```

---

### getId()

```cpp
virtual StrategyId getId() const = 0;
```

**Purpose:** Returns unique strategy identifier for programmatic routing.

**Returns:** `StrategyId` - One of:
- `RECORD_BARS` (19) - Data recording strategy
- `TAKEOVER` (26) - Takeover strategy
- `SCREENING` (27) - Market screening/analysis
- `AUTOBBS` (29) - AutoBBS daily dispatcher
- `AUTOBBSWEEKLY` (30) - AutoBBS weekly dispatcher
- `TRENDLIMIT` (31) - Trend limit strategy

**Pure Virtual:** Must be implemented by all derived classes.

**Usage:**
- Strategy factory registration
- Strategy routing/dispatch
- Logging and debugging
- Results tracking

**Example Implementation:**
```cpp
class RecordBarsStrategy : public IStrategy {
public:
    StrategyId getId() const {
        return RECORD_BARS;
    }
};
```

**Note:** ID must match the value in `INTERNAL_STRATEGY_ID` setting from MQL code.

---

### validate()

```cpp
virtual bool validate(const StrategyContext& context) const = 0;
```

**Purpose:** Validates strategy parameters and execution context before running strategy logic.

**Parameters:**
- `context` - Strategy execution context to validate

**Returns:** `bool`
- `true` - Validation passed, safe to execute
- `false` - Validation failed, should NOT execute

**Pure Virtual:** Must be implemented by all derived classes.

**Validation Checks:**
- Required parameters are set
- Parameter values within valid ranges
- Market data is available
- Sufficient historical bars
- Account information accessible
- Indicators can be loaded

**Example Implementation:**
```cpp
class MyStrategy : public IStrategy {
public:
    bool validate(const StrategyContext& context) const {
        // Check symbol
        if (context.getSymbol() == NULL || *context.getSymbol() == '\0') {
            return false;
        }
        
        // Check market data
        try {
            double bid = context.getBid(0);
            double ask = context.getAsk(0);
            if (bid <= 0.0 || ask <= 0.0 || ask < bid) {
                return false;
            }
        } catch (const std::exception&) {
            return false;
        }
        
        // Check required settings
        if (context.getSetting(MAX_OPEN_ORDERS) <= 0) {
            return false;
        }
        
        // Strategy-specific validation
        if (context.getSetting(MY_CUSTOM_PARAM) < minimumValue) {
            return false;
        }
        
        return true;
    }
};
```

**Best Practice:** Call validate() before execute() to fail fast:
```cpp
if (!strategy->validate(context)) {
    return STRATEGY_INVALID_PARAMETERS;
}
return strategy->execute(context);
```

---

### toReturnCode() (Protected Helper)

```cpp
static AsirikuyReturnCode toReturnCode(bool success);
```

**Purpose:** Utility method to convert boolean success/failure to return code.

**Parameters:**
- `success` - true for success, false for failure

**Returns:** `AsirikuyReturnCode`
- `SUCCESS` if success is true
- `STRATEGY_INTERNAL_ERROR` if success is false

**Scope:** Protected - available to derived classes only

**Example Usage:**
```cpp
class MyStrategy : public IStrategy {
    AsirikuyReturnCode execute(const StrategyContext& context) {
        bool success = performAnalysis(context);
        return toReturnCode(success);
    }
};
```

---

## Implementation Guidelines

### Minimal Implementation

```cpp
class MinimalStrategy : public IStrategy {
public:
    AsirikuyReturnCode execute(const StrategyContext& context) {
        // Strategy logic here
        return SUCCESS;
    }
    
    std::string getName() const {
        return "Minimal Strategy";
    }
    
    StrategyId getId() const {
        return RECORD_BARS; // Or appropriate ID
    }
    
    bool validate(const StrategyContext& context) const {
        // Basic validation
        return context.getSymbol() != NULL;
    }
};
```

### Recommended Implementation

Use `BaseStrategy` instead of implementing IStrategy directly:

```cpp
class RecommendedStrategy : public BaseStrategy {
public:
    RecommendedStrategy() : BaseStrategy(RECORD_BARS) {}
    
    std::string getName() const {
        return "Recommended Strategy";
    }
    
protected:
    // Override template methods
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators) 
    {
        StrategyResult result;
        // Strategy logic here
        result.code = SUCCESS;
        return result;
    }
    
    bool requiresIndicators() const {
        return false;
    }
};
```

**Benefits of BaseStrategy:**
- Template Method pattern handles common flow
- Automatic indicator loading
- Built-in validation
- Consistent error handling
- Less boilerplate code

---

## Design Patterns

### Strategy Pattern

```cpp
// Context holds strategy
class StrategyExecutor {
    IStrategy* strategy_;
    
public:
    void setStrategy(IStrategy* strategy) {
        strategy_ = strategy;
    }
    
    AsirikuyReturnCode execute(const StrategyContext& context) {
        if (!strategy_->validate(context)) {
            return STRATEGY_INVALID_PARAMETERS;
        }
        return strategy_->execute(context);
    }
};
```

### Factory Pattern Integration

```cpp
class StrategyFactory {
public:
    IStrategy* createStrategy(StrategyId id) {
        switch(id) {
            case RECORD_BARS:
                return new RecordBarsStrategy();
            case TAKEOVER:
                return new TakeOverStrategy();
            // ... more strategies
            default:
                return NULL;
        }
    }
};
```

### Null Object Pattern

```cpp
class NullStrategy : public IStrategy {
public:
    AsirikuyReturnCode execute(const StrategyContext& context) {
        return SUCCESS; // Do nothing
    }
    
    std::string getName() const {
        return "Null Strategy";
    }
    
    StrategyId getId() const {
        return RECORD_BARS; // Placeholder
    }
    
    bool validate(const StrategyContext& context) const {
        return true; // Always valid
    }
};
```

---

## Error Handling

### Return Code Usage

```cpp
AsirikuyReturnCode code = strategy->execute(context);
switch(code) {
    case SUCCESS:
        pantheios_log(PANTHEIOS_SEV_INFO, "Strategy executed successfully");
        break;
    case STRATEGY_INVALID_PARAMETERS:
        pantheios_log(PANTHEIOS_SEV_ERROR, "Invalid parameters");
        break;
    case STRATEGY_FAILED_TO_LOAD_INDICATORS:
        pantheios_log(PANTHEIOS_SEV_ERROR, "Failed to load indicators");
        break;
    case STRATEGY_INTERNAL_ERROR:
        pantheios_log(PANTHEIOS_SEV_ERROR, "Internal strategy error");
        break;
    default:
        pantheios_log(PANTHEIOS_SEV_WARNING, "Unknown return code");
}
```

### Exception Handling

Strategies should catch exceptions internally:

```cpp
AsirikuyReturnCode MyStrategy::execute(const StrategyContext& context) {
    try {
        // Strategy logic that may throw
        double bid = context.getBid(0);
        // ... more logic
        return SUCCESS;
        
    } catch (const std::out_of_range& e) {
        pantheios_log(PANTHEIOS_SEV_ERROR, "Out of range: ", e.what());
        return STRATEGY_INVALID_PARAMETERS;
        
    } catch (const std::exception& e) {
        pantheios_log(PANTHEIOS_SEV_ERROR, "Exception: ", e.what());
        return STRATEGY_INTERNAL_ERROR;
    }
}
```

---

## Testing

### Mock Strategy for Testing

```cpp
class MockStrategy : public IStrategy {
public:
    bool executeCalled = false;
    bool validateCalled = false;
    AsirikuyReturnCode returnCode = SUCCESS;
    
    AsirikuyReturnCode execute(const StrategyContext& context) {
        executeCalled = true;
        return returnCode;
    }
    
    std::string getName() const {
        return "Mock Strategy";
    }
    
    StrategyId getId() const {
        return RECORD_BARS;
    }
    
    bool validate(const StrategyContext& context) const {
        validateCalled = true;
        return true;
    }
};

// Usage in tests
MockStrategy mock;
mock.returnCode = STRATEGY_INTERNAL_ERROR;
AsirikuyReturnCode code = mock.execute(context);
BOOST_CHECK(mock.executeCalled);
BOOST_CHECK_EQUAL(code, STRATEGY_INTERNAL_ERROR);
```

---

## Performance Considerations

1. **Virtual Dispatch Overhead**: Minimal (~5ns per call on modern CPUs)
2. **Memory Layout**: Interface adds one vtable pointer (4-8 bytes)
3. **Inlining**: Virtual methods cannot be inlined across compilation units
4. **Cache**: Keep strategy objects small; large state should be in context

---

## C API Integration

### Wrapper Function

```cpp
extern "C" {
    AsirikuyReturnCode runStrategy(StrategyParams* params) {
        try {
            // Create context
            trading::StrategyContext context(params);
            
            // Get strategy from factory
            StrategyId id = context.getStrategyId();
            IStrategy* strategy = factory.createStrategy(id);
            
            if (strategy == NULL) {
                return INVALID_STRATEGY;
            }
            
            // Validate and execute
            if (!strategy->validate(context)) {
                delete strategy;
                return INVALID_PARAMETERS;
            }
            
            AsirikuyReturnCode code = strategy->execute(context);
            
            delete strategy;
            return code;
            
        } catch (const std::exception& e) {
            pantheios_log(PANTHEIOS_SEV_CRITICAL, "Exception: ", e.what());
            return INTERNAL_ERROR;
        }
    }
}
```

---

## Migration from Legacy C

### Before (C)
```c
AsirikuyReturnCode runRecordBars(StrategyParams* params) {
    // All logic in one function
    if (params == NULL) return NULL_POINTER;
    if (params->settings[MAX_OPEN_ORDERS] <= 0) return INVALID_PARAMETER;
    
    // Record bars logic...
    return SUCCESS;
}
```

### After (C++)
```cpp
class RecordBarsStrategy : public BaseStrategy {
public:
    RecordBarsStrategy() : BaseStrategy(RECORD_BARS) {}
    
    std::string getName() const { return "RecordBars"; }
    
protected:
    StrategyResult executeStrategy(
        const StrategyContext& context,
        Indicators* indicators) 
    {
        StrategyResult result;
        // Record bars logic...
        result.code = SUCCESS;
        return result;
    }
};
```

---

## Related Documentation

- **BaseStrategy**: `docs/BaseStrategy-API.md` (recommended base class)
- **StrategyContext**: `docs/StrategyContext-API.md` (execution context)
- **StrategyFactory**: `docs/StrategyFactory-API.md` (strategy creation)
- **StrategyTypes**: `include/StrategyTypes.h` (shared enums)

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-11-09 | Initial interface definition |

---

## See Also

- Header file: `include/IStrategy.hpp`
- Base implementation: `include/BaseStrategy.hpp`
- Test examples: `tests/TradingStrategiesTests.hpp`
