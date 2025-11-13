# Architecture Decision: Python Integration Path

## Decision Point

**Question**: Should Python trading platform call `AsirikuyFrameworkAPI` (existing CTester interface) or `TradingStrategiesPythonAPI` (new direct API)?

## Current State Analysis

### AsirikuyFrameworkAPI Structure

```
AsirikuyFrameworkAPI
├── MQL Interface (mql4_runStrategy, mql5_runStrategy)
│   └── Uses MQLDefines.h structures
├── CTester Interface (c_runStrategy) ← Currently used by Python
│   └── Uses CTesterDefines.h structures
│       ├── COrderInfo
│       ├── CRates
│       └── CRatesInfo
└── jforex Interface (jf_runStrategy) - unused
    └── Uses JForexDefines.h structures
```

### CTester Interface (`c_runStrategy`)

**Function Signature:**
```c
int __stdcall c_runStrategy(
    double*       pInSettings,
    char*         pInTradeSymbol,
    char*         pInAccountCurrency,
    char*         pInBrokerName,
    char*         pInRefBrokerName,
    int*          pInCurrentBrokerTime,
    int*          pInOpenOrdersCount,
    COrderInfo*   pInOrderInfo,
    double*       pInAccountInfo,
    double*       pInBidAsk,
    CRatesInfo*   pInRatesInfo,
    CRates*       pInRates_0,  // Multiple timeframes
    CRates*       pInRates_1,
    // ... up to pInRates_9
    double*       pOutResults
);
```

**Issues:**
1. **`__stdcall` calling convention**: Windows DLL-specific, not ideal for cross-platform
2. **CTester-specific structures**: COrderInfo, CRates, CRatesInfo are CTester-specific
3. **Complex conversion**: Multiple layers of conversion (CTester → Framework → TradingStrategies)
4. **Not Python-optimized**: Designed for CTester, not Python/broker REST APIs

## Option Comparison

### Option A: Use Existing AsirikuyFrameworkAPI (CTester Interface)

**Architecture:**
```
Python → AsirikuyFrameworkAPI.c_runStrategy() → TradingStrategies.runStrategy()
```

**Pros:**
- ✅ Already exists and works
- ✅ Already called from Python (CTester)
- ✅ No new code needed
- ✅ Maintains single API layer
- ✅ Can be used immediately

**Cons:**
- ❌ Uses `__stdcall` (Windows DLL convention)
  - Not ideal for cross-platform (Linux, macOS)
  - Requires platform-specific handling
- ❌ CTester-specific structures
  - COrderInfo, CRates, CRatesInfo are CTester-specific
  - Not intuitive for Python/broker REST API integration
- ❌ More complex parameter conversion
  - Python → CTester structures → Framework → StrategyParams
  - Multiple conversion layers
- ❌ Tied to Framework API structure
  - Changes to Framework API affect Python integration
  - Framework API designed for MQL/CTester, not Python

**Python Wrapper Complexity:**
```python
# Need to convert Python data to CTester structures
class COrderInfo(Structure):
    ticket: c_double
    instanceId: c_double
    type: c_double
    # ... CTester-specific fields

# Then call c_runStrategy with __stdcall
lib.c_runStrategy.argtypes = [...]
lib.c_runStrategy.restype = c_int
lib.c_runStrategy.__stdcall = True  # Platform-specific
```

### Option B: New TradingStrategiesPythonAPI (Recommended)

**Architecture:**
```
Python → TradingStrategiesPythonAPI.trading_strategies_run() → TradingStrategies.runStrategy()
```

**Pros:**
- ✅ Cleaner, Python-focused API
  - Designed specifically for Python integration
  - Python-friendly data structures
- ✅ Standard C calling convention
  - No `__stdcall` dependency
  - Cross-platform compatible (Linux, macOS, Windows)
- ✅ Simpler data structures
  - Direct mapping from Python types
  - No CTester-specific structures
- ✅ Direct to TradingStrategies
  - One less layer (no Framework API in path)
  - Simpler conversion logic
- ✅ Better suited for broker REST API integration
  - Natural mapping from REST API responses
  - Cleaner error handling

**Cons:**
- ❌ Requires new implementation
  - Need to create TradingStrategiesPythonAPI
  - Estimated 3-5 days of work
- ❌ Duplicates some conversion logic
  - But simpler than Framework API conversion
  - Can reuse conversion patterns

**Python Wrapper Complexity:**
```python
# Simple, Python-friendly structures
class PythonStrategyInput(Structure):
    strategy_id: c_int
    symbol: c_char_p
    bid: c_double
    ask: c_double
    # ... Python-friendly fields

# Standard C calling convention
lib.trading_strategies_run.argtypes = [...]
lib.trading_strategies_run.restype = c_int
# No __stdcall needed
```

## Recommendation: Option B (TradingStrategiesPythonAPI)

### Rationale

1. **Cross-Platform Compatibility**
   - `__stdcall` is Windows-specific
   - Standard C calling convention works everywhere
   - Better for Linux/macOS deployment

2. **Python/Broker REST API Optimization**
   - Direct mapping from REST API responses
   - No CTester-specific structures
   - Cleaner, more intuitive API

3. **Maintainability**
   - Separate API for Python use case
   - Doesn't affect MQL/CTester interfaces
   - Easier to evolve independently

4. **Simplicity**
   - One less conversion layer
   - Simpler data structures
   - Easier to understand and debug

5. **Future-Proof**
   - Can optimize for Python-specific needs
   - Can add Python-specific features
   - Not constrained by Framework API design

### Implementation Impact

**Option A (Use Existing):**
- **Effort**: 1-2 days (Python wrapper only)
- **Risk**: Low (existing code)
- **Maintainability**: Medium (tied to Framework API)

**Option B (New API):**
- **Effort**: 3-5 days (C API + Python wrapper)
- **Risk**: Low-Medium (new code, but simple)
- **Maintainability**: High (clean separation)

## Decision

**Selected Option**: **Option B - TradingStrategiesPythonAPI**

**Justification**: 
- Better long-term solution
- Cross-platform compatibility
- Python-optimized design
- Cleaner architecture
- Worth the additional 2-3 days of implementation

**Note**: AsirikuyFrameworkAPI remains unchanged and continues to support MQL4/MQL5 and CTester interfaces.

## Migration Path

1. **Phase 1**: Implement TradingStrategiesPythonAPI
2. **Phase 2**: Create Python wrapper
3. **Phase 3**: Test and validate
4. **Future**: Optionally deprecate CTester interface for Python (keep for CTester compatibility)

## Alternative: Hybrid Approach

If immediate Python support is needed:
1. Use AsirikuyFrameworkAPI (CTester) for initial implementation
2. Implement TradingStrategiesPythonAPI in parallel
3. Migrate to TradingStrategiesPythonAPI once ready

This allows immediate Python support while building the better long-term solution.

