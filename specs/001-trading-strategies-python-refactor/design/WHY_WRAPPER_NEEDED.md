# Why Do We Need a Wrapper?

## Current Situation

**CTester currently integrates with CTesterFrameworkAPI directly** - no wrapper needed!

Looking at `ctester/asirikuy_strategy_tester.py`:
```python
# Line 65-69: Direct library loading
if (system == "Windows"):
    astdll = loadLibrary('CTesterFrameworkAPI')
elif (system == "Linux"):
    astdll = loadLibrary('libCTesterFrameworkAPI.so')
elif (system == "Darwin"):
    astdll = loadLibrary('libCTesterFrameworkAPI.dylib')

# Line 573-578: Direct function call
astdll.runPortfolioTest.restype = TestResult
result = astdll.runPortfolioTest(
    c_int(1), ctypes.pointer(settings), ...
)
```

**This works perfectly fine!** CTester can and does integrate with CTesterFrameworkAPI directly.

## Why Migrate to AsirikuyFrameworkAPI?

The project goal is to **migrate from CTesterFrameworkAPI to AsirikuyFrameworkAPI**. Here's why:

### 1. **Unified API Architecture**

- **CTesterFrameworkAPI**: Separate wrapper library specifically for CTester
- **AsirikuyFrameworkAPI**: Main framework API that supports multiple frontends:
  - MQL4/MQL5 (MetaTrader)
  - JForex
  - CTester (via `c_runStrategy`)
  - Live Trading (future)

**Goal**: Use one unified API instead of maintaining separate APIs.

### 2. **Code Maintenance**

- **CTesterFrameworkAPI**: Separate codebase that needs maintenance
- **AsirikuyFrameworkAPI**: Main framework that's actively maintained

**Goal**: Reduce maintenance burden by using single API.

### 3. **Cross-Platform Support**

- **CTesterFrameworkAPI**: May have platform-specific issues
- **AsirikuyFrameworkAPI**: Built with cross-platform support (Windows, macOS, Linux)

**Goal**: Better cross-platform compatibility.

## The Problem: Different Execution Models

### CTesterFrameworkAPI (Current)
```c
// Batch execution - executes entire test in one call
TestResult runPortfolioTest(
    int numSystems,
    double** settings,
    char** symbols,
    // ... all parameters ...
    int numCandles,           // All bars at once
    ASTRates*** rates,        // All bars at once
    void (*testUpdate)(...),  // Callbacks for updates
    void (*testFinished)(...),
    void (*signalUpdate)(...)
);
```

**Execution Model**: 
- Single function call
- Executes entire test (all bars)
- Uses callbacks for updates
- Returns complete TestResult at end

### AsirikuyFrameworkAPI (Target)
```c
// Per-bar execution - executes one bar at a time
int c_runStrategy(
    double* settings,
    char* symbol,
    // ... parameters ...
    int* currentBrokerTime,    // Current bar time
    CRates* rates_0,           // Current bar only
    CRates* rates_1,           // Current bar only
    // ...
    double* results            // Output for this bar
);
```

**Execution Model**:
- Called once per bar
- Executes one bar/candle at a time
- Returns results directly (no callbacks)
- Must be called in a loop

## Why We Need a Wrapper

The wrapper is needed to **bridge the execution model difference**:

### Without Wrapper (Direct Integration)
```python
# This won't work - AsirikuyFrameworkAPI needs per-bar execution
result = asirikuy_dll.c_runStrategy(...)  # Only executes ONE bar!
# How do we execute all bars? We need a loop!
```

### With Wrapper (Bridged Integration)
```python
# Wrapper provides CTester-compatible interface
def runPortfolioTest(...):
    # Initialize instances
    for instance_id in range(numSystems):
        initInstanceC(instance_id, ...)
    
    # Execute per-bar loop
    for bar_index in range(numCandles):
        for instance_id in range(numSystems):
            # Prepare current bar data
            current_rates = rates[bar_index]
            
            # Call strategy for this bar
            c_runStrategy(
                settings[instance_id],
                current_rates,  # Current bar only
                ...
            )
            
            # Process results
            # Update orders, balance, statistics
            # Generate callbacks (testUpdate, signalUpdate)
    
    # Calculate final statistics
    # Return TestResult
    
    # Cleanup
    for instance_id in range(numSystems):
        deinitInstance(instance_id)
```

## Alternative: Keep Using CTesterFrameworkAPI

**You could keep using CTesterFrameworkAPI directly!** There's no technical requirement to migrate.

### Pros of Keeping CTesterFrameworkAPI:
- ✅ Already works
- ✅ No migration effort
- ✅ No wrapper needed
- ✅ Batch execution is simpler

### Cons of Keeping CTesterFrameworkAPI:
- ❌ Separate codebase to maintain
- ❌ May not be actively developed
- ❌ Less unified architecture
- ❌ Potential platform compatibility issues

## Decision Point

**Question**: Do we want to migrate to AsirikuyFrameworkAPI?

### Option A: Migrate (Current Plan)
- **Requires**: Wrapper to bridge execution models
- **Benefit**: Unified API, better maintenance
- **Effort**: Medium (wrapper implementation)

### Option B: Keep CTesterFrameworkAPI
- **Requires**: Nothing - already works
- **Benefit**: No migration effort
- **Effort**: None

## Recommendation

**If the goal is to unify the architecture**, then migration makes sense and the wrapper is necessary.

**If CTesterFrameworkAPI works fine and there's no need to unify**, then you can skip the migration and keep using CTesterFrameworkAPI directly.

The wrapper is only needed **if** you want to migrate from CTesterFrameworkAPI to AsirikuyFrameworkAPI.

