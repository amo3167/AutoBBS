# TradingStrategies Refactoring Plan
## Python Integration - Removing DLL Dependencies

## Executive Summary

This document outlines refactoring options for the `TradingStrategies` C library to enable Python integration via broker REST APIs, removing the need for Windows DLL support.

**Current State:**
- C library built as static library (`StaticLib`)
- Called through `AsirikuyFrameworkAPI` which provides DLL wrappers for MQL4/MQL5
- Uses file-based I/O for UI communication (`StrategyUserInterface.c`)
- Uses `__stdcall` calling convention for DLL compatibility
- File-based state persistence (heartbeat, trading info, UI values)

**Target State:**
- Clean C API callable from Python
- No DLL dependencies
- Optional file-based I/O (can be replaced with callbacks)
- Standard C calling convention
- Python bindings via ctypes or CFFI

---

## Option 1: Minimal Refactor - Python ctypes Wrapper (Recommended)

### Overview
Create a thin Python wrapper using ctypes that calls the existing C library directly. Minimal changes to C code.

### Changes Required

#### 1.1 Create Python-Friendly C API (`include/TradingStrategiesPythonAPI.h`)

```c
#ifndef TRADING_STRATEGIES_PYTHON_API_H_
#define TRADING_STRATEGIES_PYTHON_API_H_

#include "AsirikuyDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Python-friendly API for TradingStrategies
 * Uses standard C calling convention (no __stdcall)
 */

/* Simplified input structure for Python */
typedef struct {
    /* Strategy identification */
    int strategy_id;              // INTERNAL_STRATEGY_ID
    int instance_id;              // STRATEGY_INSTANCE_ID
    
    /* Market data */
    char* symbol;                 // Trade symbol
    double bid;
    double ask;
    time_t current_time;
    
    /* Account info */
    double account_balance;
    double account_equity;
    char* account_currency;
    
    /* Rates data - flattened arrays for Python */
    int rates_count;              // Number of rates per timeframe
    double* rates_time;           // Time array
    double* rates_open;            // Open prices
    double* rates_high;            // High prices
    double* rates_low;             // Low prices
    double* rates_close;           // Close prices
    double* rates_volume;          // Volume
    
    /* Settings array - flattened for Python */
    double* settings;             // Settings array
    int settings_count;            // Number of settings
    
    /* Open orders */
    int orders_count;
    int* order_numbers;
    int* order_types;              // OrderType enum values
    double* order_open_prices;
    double* order_stop_loss;
    double* order_take_profit;
    int* order_statuses;           // OrderStatus enum values
    time_t* order_timestamps;
} PythonStrategyInput;

/* Output structure for Python */
typedef struct {
    int return_code;              // AsirikuyReturnCode
    
    /* Order signals */
    int signals_count;
    int* signal_types;             // OrderType enum
    double* signal_prices;
    double* signal_stop_loss;
    double* signal_take_profit;
    int* signal_actions;           // OPEN, MODIFY, CLOSE
    
    /* UI values (optional) */
    int ui_values_count;
    char** ui_names;
    double* ui_values;
    
    /* Strategy state (optional) */
    char* status_message;
} PythonStrategyOutput;

/**
 * Main entry point for Python
 * Allocates and manages StrategyParams internally
 */
int trading_strategies_run(
    const PythonStrategyInput* input,
    PythonStrategyOutput* output
);

/**
 * Free resources allocated by trading_strategies_run
 */
void trading_strategies_free_output(PythonStrategyOutput* output);

/**
 * Get error message for return code
 */
const char* trading_strategies_get_error_message(int return_code);

#ifdef __cplusplus
}
#endif

#endif /* TRADING_STRATEGIES_PYTHON_API_H_ */
```

#### 1.2 Implementation (`src/TradingStrategiesPythonAPI.c`)

```c
#include "TradingStrategiesPythonAPI.h"
#include "AsirikuyStrategies.h"
#include "StrategyUserInterface.h"
#include <stdlib.h>
#include <string.h>

/* Convert Python input to StrategyParams */
static AsirikuyReturnCode convert_python_input(
    const PythonStrategyInput* input,
    StrategyParams* params
) {
    /* Implementation to convert flattened arrays to StrategyParams */
    /* Allocate RatesBuffers, populate settings, etc. */
    return SUCCESS;
}

/* Convert StrategyResults to Python output */
static AsirikuyReturnCode convert_strategy_results(
    StrategyParams* params,
    PythonStrategyOutput* output
) {
    /* Extract signals, UI values, etc. from StrategyResults */
    return SUCCESS;
}

int trading_strategies_run(
    const PythonStrategyInput* input,
    PythonStrategyOutput* output
) {
    StrategyParams params;
    AsirikuyReturnCode result;
    
    /* Validate input */
    if (!input || !output) {
        return NULL_POINTER;
    }
    
    /* Convert Python input to StrategyParams */
    result = convert_python_input(input, &params);
    if (result != SUCCESS) {
        return result;
    }
    
    /* Run strategy */
    result = runStrategy(&params);
    
    /* Convert results to Python output */
    if (result == SUCCESS) {
        convert_strategy_results(&params, output);
    }
    
    /* Cleanup */
    clearStrategyResults(&params);
    
    output->return_code = result;
    return result;
}

const char* trading_strategies_get_error_message(int return_code) {
    /* Map return codes to error messages */
    switch (return_code) {
        case SUCCESS: return "Success";
        case NULL_POINTER: return "Null pointer error";
        /* ... */
        default: return "Unknown error";
    }
}
```

#### 1.3 Python Wrapper (`python/trading_strategies.py`)

```python
import ctypes
import os
from ctypes import Structure, POINTER, c_int, c_double, c_char_p, c_void_p
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass

# Load shared library
_lib_path = os.path.join(os.path.dirname(__file__), '..', 'build', 'libtrading_strategies.so')
_lib = ctypes.CDLL(_lib_path)

# Define structures
class PythonStrategyInput(Structure):
    _fields_ = [
        ('strategy_id', c_int),
        ('instance_id', c_int),
        ('symbol', c_char_p),
        ('bid', c_double),
        ('ask', c_double),
        ('current_time', c_int),
        ('account_balance', c_double),
        ('account_equity', c_double),
        ('account_currency', c_char_p),
        ('rates_count', c_int),
        ('rates_time', POINTER(c_double)),
        ('rates_open', POINTER(c_double)),
        ('rates_high', POINTER(c_double)),
        ('rates_low', POINTER(c_double)),
        ('rates_close', POINTER(c_double)),
        ('rates_volume', POINTER(c_double)),
        ('settings', POINTER(c_double)),
        ('settings_count', c_int),
        ('orders_count', c_int),
        ('order_numbers', POINTER(c_int)),
        ('order_types', POINTER(c_int)),
        ('order_open_prices', POINTER(c_double)),
        ('order_stop_loss', POINTER(c_double)),
        ('order_take_profit', POINTER(c_double)),
        ('order_statuses', POINTER(c_int)),
        ('order_timestamps', POINTER(c_int)),
    ]

class PythonStrategyOutput(Structure):
    _fields_ = [
        ('return_code', c_int),
        ('signals_count', c_int),
        ('signal_types', POINTER(c_int)),
        ('signal_prices', POINTER(c_double)),
        ('signal_stop_loss', POINTER(c_double)),
        ('signal_take_profit', POINTER(c_double)),
        ('signal_actions', POINTER(c_int)),
        ('ui_values_count', c_int),
        ('ui_names', POINTER(c_char_p)),
        ('ui_values', POINTER(c_double)),
        ('status_message', c_char_p),
    ]

# Define function signatures
_lib.trading_strategies_run.argtypes = [
    POINTER(PythonStrategyInput),
    POINTER(PythonStrategyOutput)
]
_lib.trading_strategies_run.restype = c_int

_lib.trading_strategies_get_error_message.argtypes = [c_int]
_lib.trading_strategies_get_error_message.restype = c_char_p

@dataclass
class TradingSignal:
    """Trading signal from strategy"""
    action: str  # 'OPEN', 'MODIFY', 'CLOSE'
    order_type: str  # 'BUY', 'SELL'
    price: float
    stop_loss: float
    take_profit: float

class TradingStrategies:
    """Python wrapper for TradingStrategies C library"""
    
    def __init__(self):
        self._lib = _lib
    
    def run_strategy(
        self,
        strategy_id: int,
        symbol: str,
        market_data: Dict,
        account_info: Dict,
        settings: Dict,
        open_orders: List[Dict]
    ) -> Tuple[int, List[TradingSignal], Dict]:
        """
        Run trading strategy
        
        Returns:
            (return_code, signals, ui_values)
        """
        # Prepare input structure
        input_data = self._prepare_input(
            strategy_id, symbol, market_data,
            account_info, settings, open_orders
        )
        
        # Prepare output structure
        output = PythonStrategyOutput()
        
        # Call C function
        result = self._lib.trading_strategies_run(
            ctypes.byref(input_data),
            ctypes.byref(output)
        )
        
        # Extract results
        signals = self._extract_signals(output)
        ui_values = self._extract_ui_values(output)
        
        return result, signals, ui_values
    
    def _prepare_input(self, ...):
        """Convert Python data to C structures"""
        # Implementation
        pass
```

#### 1.4 Build Configuration Changes

**Update `premake4.lua`:**
```lua
project "TradingStrategies"
  location("../../build/" .. _ACTION .. "/projects")
  kind "SharedLib"  -- Changed from StaticLib
  language "C"
  targetname "trading_strategies"  -- Library name
  files{
    "**.h", 
    "**.c", 
    "**.hpp"
  }
  -- ... rest of config
```

### Pros
- ✅ Minimal changes to existing C code
- ✅ Fast to implement
- ✅ Maintains existing strategy logic
- ✅ Easy to test incrementally
- ✅ Can keep file-based I/O as optional

### Cons
- ⚠️ Still requires file I/O abstraction layer
- ⚠️ Memory management complexity (Python ↔ C)
- ⚠️ Need to handle array conversions

### Effort Estimate
- **Implementation**: 2-3 days
- **Testing**: 1-2 days
- **Total**: 3-5 days

---

## Option 2: Moderate Refactor - Callback-Based I/O

### Overview
Replace file-based I/O with callback functions that Python can provide. More flexible but requires more changes.

### Changes Required

#### 2.1 Add Callback Interface (`include/StrategyCallbacks.h`)

```c
#ifndef STRATEGY_CALLBACKS_H_
#define STRATEGY_CALLBACKS_H_

#include "AsirikuyDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Callback function types */
typedef void (*UIValuesCallback)(
    int instance_id,
    const char** names,
    const double* values,
    int count,
    void* user_data
);

typedef void (*HeartbeatCallback)(
    int instance_id,
    time_t timestamp,
    void* user_data
);

typedef void (*TradingInfoCallback)(
    int instance_id,
    const Order_Info* order_info,
    void* user_data
);

typedef AsirikuyReturnCode (*ReadTradingInfoCallback)(
    int instance_id,
    Order_Info* order_info,
    void* user_data
);

/* Callback registry */
typedef struct {
    UIValuesCallback ui_callback;
    HeartbeatCallback heartbeat_callback;
    TradingInfoCallback trading_info_callback;
    ReadTradingInfoCallback read_trading_info_callback;
    void* user_data;
} StrategyCallbacks;

/* Set callbacks (NULL to disable) */
void set_strategy_callbacks(const StrategyCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif /* STRATEGY_CALLBACKS_H_ */
```

#### 2.2 Modify StrategyUserInterface.c

```c
/* Add at top of file */
static StrategyCallbacks g_callbacks = {0};

void set_strategy_callbacks(const StrategyCallbacks* callbacks) {
    if (callbacks) {
        g_callbacks = *callbacks;
    } else {
        memset(&g_callbacks, 0, sizeof(g_callbacks));
    }
}

/* Modify saveUserInterfaceValues */
AsirikuyReturnCode saveUserInterfaceValues(...) {
    /* If callback is set, use it instead of file I/O */
    if (g_callbacks.ui_callback) {
        g_callbacks.ui_callback(
            instanceID,
            userInterfaceVariableNames,
            userInterfaceValues,
            userInterfaceElementsCount,
            g_callbacks.user_data
        );
        return SUCCESS;
    }
    
    /* Fall back to file I/O for backward compatibility */
    /* ... existing file I/O code ... */
}
```

### Pros
- ✅ More flexible - Python controls I/O
- ✅ No file system dependencies
- ✅ Better for testing
- ✅ Can still support file I/O as fallback

### Cons
- ⚠️ More code changes required
- ⚠️ Need to update all file I/O calls
- ⚠️ Thread-safety considerations

### Effort Estimate
- **Implementation**: 4-5 days
- **Testing**: 2-3 days
- **Total**: 6-8 days

---

## Option 3: Full Refactor - Pure C API with No File I/O

### Overview
Completely remove file-based I/O and make everything return-based. Most Python-friendly but requires significant refactoring.

### Changes Required

#### 3.1 New API Design

- Remove all file I/O functions
- Return UI values in StrategyResults
- Return state in StrategyResults
- Make StrategyResults the single source of output

#### 3.2 Modify StrategyResults Structure

```c
typedef struct strategyResults_t {
    /* Existing fields */
    /* ... */
    
    /* New fields for Python */
    char status_message[256];
    double ui_values[TOTAL_UI_VALUES];
    char ui_names[TOTAL_UI_VALUES][64];
    int ui_values_count;
    
    /* State persistence (optional) */
    Order_Info last_order_info;
    time_t last_heartbeat;
} StrategyResults;
```

### Pros
- ✅ Cleanest API
- ✅ No side effects
- ✅ Fully testable
- ✅ Best for Python integration

### Cons
- ⚠️ Most work required
- ⚠️ Breaking changes to existing code
- ⚠️ Need to refactor all strategies

### Effort Estimate
- **Implementation**: 7-10 days
- **Testing**: 3-5 days
- **Total**: 10-15 days

---

## Recommended Approach: Hybrid (Option 1 + Option 2)

### Phase 1: Create Python API (Option 1)
1. Create `TradingStrategiesPythonAPI.h/c`
2. Implement Python wrapper
3. Keep file I/O as-is (optional)

### Phase 2: Add Callbacks (Option 2)
1. Add callback interface
2. Make file I/O optional (use callbacks if provided)
3. Update Python wrapper to use callbacks

### Benefits
- ✅ Incremental migration
- ✅ Can test each phase
- ✅ Maintains backward compatibility
- ✅ Flexible for future needs

---

## Implementation Steps (Recommended Approach)

### Step 1: Create Python API Layer
1. Create `include/TradingStrategiesPythonAPI.h`
2. Create `src/TradingStrategiesPythonAPI.c`
3. Implement conversion functions (Python ↔ C structures)
4. Update `premake4.lua` to build as shared library

### Step 2: Build System Updates
1. Change `kind` from `StaticLib` to `SharedLib`
2. Add Python build target (optional)
3. Create Python package structure

### Step 3: Python Wrapper
1. Create `python/trading_strategies.py`
2. Implement ctypes bindings
3. Add Python-friendly data structures
4. Add error handling

### Step 4: Testing
1. Unit tests for conversion functions
2. Integration tests with Python
3. Performance testing

### Step 5: Documentation
1. API documentation
2. Python usage examples
3. Migration guide

---

## File Structure After Refactoring

```
TradingStrategies/
├── include/
│   ├── TradingStrategiesPythonAPI.h  # NEW
│   ├── StrategyCallbacks.h           # NEW (optional)
│   ├── AsirikuyStrategies.h
│   ├── StrategyAPI.h
│   ├── StrategyUserInterface.h
│   └── strategies/
│       └── ...
├── src/
│   ├── TradingStrategiesPythonAPI.c  # NEW
│   ├── StrategyCallbacks.c          # NEW (optional)
│   ├── AsirikuyStrategies.c
│   ├── StrategyUserInterface.c      # Modified (optional callbacks)
│   └── strategies/
│       └── ...
├── python/                           # NEW
│   ├── trading_strategies.py
│   ├── __init__.py
│   └── examples/
│       └── basic_usage.py
├── tests/
│   └── TradingStrategiesTests.hpp
├── premake4.lua                      # Modified
└── REFACTORING_PLAN.md               # This file
```

---

## Dependencies to Consider

### Required
- Python 3.8+ (for ctypes)
- C compiler (GCC/Clang)
- Build system (premake4)

### Optional
- CFFI (alternative to ctypes, more Pythonic)
- NumPy (for efficient array handling)
- pytest (for testing)

---

## Migration Checklist

- [ ] Create Python API header
- [ ] Implement Python API C code
- [ ] Update build system (premake4.lua)
- [ ] Build shared library
- [ ] Create Python wrapper
- [ ] Test basic strategy execution
- [ ] Test with real market data
- [ ] Performance testing
- [ ] Documentation
- [ ] Remove DLL-specific code (if any)
- [ ] Update tests

---

## Questions to Resolve

1. **File I/O**: Keep as optional fallback or remove completely?
2. **State Persistence**: How should strategy state be persisted in Python?
3. **Thread Safety**: Are strategies thread-safe? Need for concurrent execution?
4. **Error Handling**: How detailed should error messages be?
5. **Performance**: Any performance requirements for Python integration?
6. **Backward Compatibility**: Need to maintain compatibility with existing MQL code?

---

## Next Steps

1. **Review this plan** and choose approach
2. **Clarify requirements** (file I/O, state persistence, etc.)
3. **Create implementation branch**
4. **Start with Phase 1** (Python API layer)
5. **Iterate and test**

---

## References

- Current API: `AsirikuyStrategies.h`
- Strategy entry point: `runStrategy(StrategyParams*)`
- File I/O: `StrategyUserInterface.c`
- Framework API: `AsirikuyFrameworkAPI/`

