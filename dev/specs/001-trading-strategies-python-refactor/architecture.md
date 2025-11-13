# Architecture: TradingStrategies Python Integration

## Overview

This document describes the technical architecture for integrating the TradingStrategies C library with Python, enabling Python trading platforms to use the existing C strategy logic via broker REST APIs.

## System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    Python Trading Platform                       │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Broker REST API Client                                   │  │
│  │  • Get market data                                        │  │
│  │  • Get account info                                       │  │
│  │  • Place/modify/close orders                              │  │
│  └────────────────────┬─────────────────────────────────────┘  │
│                       │                                         │
│  ┌────────────────────▼─────────────────────────────────────┐  │
│  │  Trading Platform Logic                                   │  │
│  │  • Strategy orchestration                                 │  │
│  │  • Risk management                                        │  │
│  │  • Order execution                                        │  │
│  └────────────────────┬─────────────────────────────────────┘  │
│                       │                                         │
│  ┌────────────────────▼─────────────────────────────────────┐  │
│  │  Python Wrapper (trading_strategies.py)                   │  │
│  │  • ctypes bindings                                        │  │
│  │  • Data conversion (Python ↔ C)                            │  │
│  │  • Error handling                                         │  │
│  └────────────────────┬─────────────────────────────────────┘  │
└───────────────────────┼─────────────────────────────────────────┘
                        │
                        │ ctypes FFI
                        │
┌───────────────────────▼─────────────────────────────────────────┐
│                    C Library Layer                               │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  TradingStrategiesPythonAPI (C API)                       │  │
│  │  • Python-friendly structures                             │  │
│  │  • Input/output conversion                                │  │
│  │  • Memory management                                      │  │
│  └────────────────────┬─────────────────────────────────────┘  │
│                       │                                         │
│  ┌────────────────────▼─────────────────────────────────────┐  │
│  │  TradingStrategies (Core Library)                         │  │
│  │  • Strategy execution logic                              │  │
│  │  • Technical analysis                                    │  │
│  │  • Risk calculations                                     │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Component Architecture

### 1. Python Wrapper Layer

**File**: `python/trading_strategies.py`

**Responsibilities**:
- Load shared library dynamically
- Map C types to Python types (ctypes)
- Convert Python data structures to C structures
- Convert C results back to Python
- Handle errors and exceptions
- Provide Pythonic API

**Key Classes**:
```python
class TradingStrategies:
    """Main wrapper class"""
    def run_strategy(...) -> Tuple[int, List[TradingSignal], Dict, str]
    
@dataclass
class TradingSignal:
    """Trading signal from strategy"""
    action: str
    order_type: str
    price: float
    stop_loss: float
    take_profit: float
```

**Data Flow**:
1. Python code calls `run_strategy()` with Python objects
2. Wrapper converts to C structures
3. Calls C function via ctypes
4. Converts C results back to Python objects
5. Returns Python-friendly results

### 2. C API Layer

**File**: `src/TradingStrategiesPythonAPI.c`

**Responsibilities**:
- Provide Python-friendly C API
- Convert PythonStrategyInput to StrategyParams
- Call existing runStrategy() function
- Convert StrategyResults to PythonStrategyOutput
- Manage memory for Python interop
- Provide error messages

**Key Functions**:
```c
int trading_strategies_run(
    const PythonStrategyInput* input,
    PythonStrategyOutput* output
);

void trading_strategies_free_output(PythonStrategyOutput* output);

const char* trading_strategies_get_error_message(int return_code);
```

**Data Flow**:
1. Receives PythonStrategyInput (flattened arrays)
2. Converts to StrategyParams (nested structures)
3. Calls runStrategy(StrategyParams*)
4. Converts StrategyResults to PythonStrategyOutput
5. Returns results

### 3. Core Strategy Layer

**File**: `src/AsirikuyStrategies.c` (existing, unchanged)

**Responsibilities**:
- Execute trading strategies
- Calculate indicators
- Generate signals
- Manage strategy state

**Key Functions**:
```c
AsirikuyReturnCode runStrategy(StrategyParams* pParams);
```

**Note**: This layer remains unchanged - we're adding a wrapper, not modifying strategy logic.

## Data Structure Mapping

### Input Mapping (Python → C)

```
Python                          C
─────────────────────────────────────────────────────────────
MarketData.symbol          →   StrategyParams.tradeSymbol
MarketData.bid/ask         →   StrategyParams.bidAsk
AccountInfo.balance        →   StrategyParams.accountInfo.balance
AccountInfo.equity         →   StrategyParams.accountInfo.equity
rates_data["M1"]           →   StrategyParams.ratesBuffers[PRIMARY_RATES]
settings dict              →   StrategyParams.settings[] (array)
open_orders list           →   StrategyParams.orderInfo[] (array)
```

### Output Mapping (C → Python)

```
C                              Python
─────────────────────────────────────────────────────────────
StrategyResults.signals    →   List[TradingSignal]
StrategyResults.uiValues   →   Dict[str, float]
StrategyResults.status     →   str
AsirikuyReturnCode         →   int (return code)
```

## Memory Management

### Ownership Rules

1. **Input Data**: Python owns all input data. C library receives pointers but does not free them.

2. **Output Data**: C library allocates output arrays. Python must call `trading_strategies_free_output()` to free them.

3. **Internal Data**: C library manages all internal allocations (StrategyParams, etc.).

### Memory Flow

```
Python allocates input arrays
    ↓
Pass pointers to C
    ↓
C allocates output arrays
    ↓
Return to Python
    ↓
Python processes results
    ↓
Python calls free_output()
    ↓
C frees output arrays
```

### Example Pattern

```python
# Python side
input_data = prepare_input(...)
output = PythonStrategyOutput()

try:
    result = lib.trading_strategies_run(input_data, output)
    signals = extract_signals(output)
    return signals
finally:
    lib.trading_strategies_free_output(output)
```

## Error Handling

### Error Flow

```
C Function returns AsirikuyReturnCode
    ↓
Python wrapper checks return code
    ↓
If error:
    - Get error message from C
    - Raise Python exception
    - Include error details
If success:
    - Extract results
    - Return to caller
```

### Error Mapping

```python
class TradingStrategiesError(Exception):
    """Base exception for TradingStrategies"""
    pass

class NullPointerError(TradingStrategiesError):
    """Null pointer error"""
    pass

class InvalidParameterError(TradingStrategiesError):
    """Invalid parameter error"""
    pass
```

## Build Architecture

### Build Process

```
premake4.lua
    ↓
Generate build files
    ↓
Compile C sources
    ↓
Link shared library
    ↓
libtrading_strategies.so/.dylib/.dll
```

### Dependencies

```
TradingStrategies (shared library)
    ├── AsirikuyCommon (static)
    ├── AsirikuyTechnicalAnalysis (static)
    ├── OrderManager (static, if used)
    └── Log (static, optional)
```

### Library Structure

```
libtrading_strategies.so
    ├── TradingStrategiesPythonAPI functions
    ├── TradingStrategies core functions
    └── Dependencies (linked statically or dynamically)
```

## Thread Safety

### Current State
- Strategy execution is not thread-safe (uses global state)
- File I/O is not thread-safe

### Recommendations
- Use one strategy instance per thread
- Or use mutex/lock for concurrent access
- Document thread-safety guarantees

### Future Consideration
- Make strategies thread-safe (separate refactoring)
- Use thread-local storage for state

## Performance Considerations

### Overhead Sources

1. **Data Conversion**: Python → C structure conversion
   - **Impact**: Low (one-time per call)
   - **Mitigation**: Minimize conversions, cache structures

2. **Memory Allocation**: Allocating output arrays
   - **Impact**: Low (strategy execution dominates)
   - **Mitigation**: Reuse allocations where possible

3. **Function Call Overhead**: ctypes FFI overhead
   - **Impact**: Very low (nanoseconds)
   - **Mitigation**: None needed

### Performance Targets

- **Overhead**: < 5% vs. direct C calls
- **Strategy Execution**: Dominates total time (>95%)
- **Conversion**: < 1ms for typical data sizes

## Security Considerations

### Input Validation

1. **Python Side**: Validate all inputs before calling C
2. **C Side**: Validate pointers and array bounds
3. **Both**: Check for NULL pointers, invalid ranges

### Memory Safety

1. **Bounds Checking**: Validate array sizes
2. **Pointer Validation**: Check for NULL before dereference
3. **Buffer Overflow**: Use safe string functions

## Testing Architecture

### Test Layers

```
┌─────────────────────────────────────────┐
│  Integration Tests (Python)              │
│  • End-to-end tests                    │
│  • Real data tests                     │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│  Python Unit Tests                      │
│  • Wrapper function tests              │
│  • Data conversion tests               │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│  C Unit Tests                           │
│  • API function tests                  │
│  • Conversion function tests          │
└─────────────────────────────────────────┘
```

### Test Data Flow

```
Mock Data (Python)
    ↓
Python Wrapper
    ↓
C API
    ↓
Strategy Execution (mocked or real)
    ↓
Results
    ↓
Validation
```

## Deployment Architecture

### Distribution

```
TradingStrategies Package
    ├── libtrading_strategies.so/.dylib/.dll
    ├── python/
    │   ├── trading_strategies.py
    │   ├── __init__.py
    │   └── example_usage.py
    └── docs/
        ├── API.md
        └── usage_guide.md
```

### Installation

1. Build shared library
2. Install Python package
3. Set library path (or use rpath)
4. Test installation

## Future Enhancements

### Potential Improvements

1. **CFFI Alternative**: Replace ctypes with CFFI for better performance
2. **NumPy Integration**: Direct NumPy array support
3. **Async Support**: Asynchronous strategy execution
4. **Callback Interface**: Full callback-based I/O
5. **Multiple Timeframes**: Native multi-timeframe support

### Migration Path

1. Keep DLL support (if needed)
2. Add Python API (this spec)
3. Migrate users to Python API
4. Deprecate DLL API
5. Remove DLL API

