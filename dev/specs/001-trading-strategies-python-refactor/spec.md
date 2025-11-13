# Specification 001: TradingStrategies Python Integration Refactor

## 📋 Overview

**Project**: TradingStrategies C Library - Python Integration Refactoring
**Spec ID**: 001
**Created**: November 13, 2024
**Status**: Draft
**Parent Spec**: N/A (Standalone refactoring initiative)

## 🎯 Objectives

### Primary Goal
Refactor the TradingStrategies C library to enable Python integration via broker REST APIs, removing Windows DLL dependencies while maintaining existing strategy logic and functionality.

### Problem Statement
- Current implementation is built as Windows DLL for MQL4/MQL5 integration
- AsirikuyFrameworkAPI exists with CTester interface (`c_runStrategy`) that can be called from Python, but:
  - Uses `__stdcall` calling convention (Windows DLL-specific)
  - Has CTester-specific structures (COrderInfo, CRates, CRatesInfo)
  - More complex parameter conversion
  - Not optimized for Python/broker REST API use case
- Need cleaner Python integration optimized for broker REST APIs
- File-based I/O creates dependencies that complicate Python integration

### Success Criteria
1. **Python Integration**: Successfully call TradingStrategies from Python using ctypes
2. **No DLL Dependencies**: Remove all Windows DLL-specific code and calling conventions
3. **API Clarity**: Provide clean, well-documented Python API
4. **Backward Compatibility**: Maintain existing strategy logic without modification
5. **Performance**: No significant performance degradation vs. current implementation
6. **Maintainability**: Code structure is easier to manage going forward

## 🏗️ Architecture

### Current Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                    Current Architecture                        │
├─────────────────────────────────────────────────────────────┤
│  MQL4/MQL5 Frontend        CTester (Python)    jforex (unused)│
│  ┌──────────────┐         ┌──────────────┐    ┌──────────┐  │
│  │ MQL API      │         │ CTester API  │    │ jforex   │  │
│  │ (mql4/5_     │         │ (c_run       │    │ API      │  │
│  │  runStrategy)│         │  Strategy)   │    │          │  │
│  └──────┬───────┘         └──────┬───────┘    └────┬─────┘  │
│         │                         │                  │        │
│         └─────────────────────────┼──────────────────┘        │
│                                   │                            │
│  ┌───────────────────────────────▼──────────────────────┐   │
│  │  AsirikuyFrameworkAPI (DLL Wrapper)                  │   │
│  │  • __stdcall calling convention                       │   │
│  │  • Platform-specific parameter conversion             │   │
│  │  • MQL/CTester/jforex structures                      │   │
│  └────────────────────┬─────────────────────────────────┘   │
│                       │                                        │
│  ┌────────────────────▼─────────────────────────────────┐   │
│  │  TradingStrategies (Static Library)                 │   │
│  │  • Strategy execution logic                         │   │
│  │  • File-based I/O (UI, heartbeat, state)            │   │
│  │  • StrategyParams → StrategyResults                 │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Architecture Decision: Two Options

#### Option A: Use Existing AsirikuyFrameworkAPI (CTester Interface)
**Pros:**
- ✅ Already exists and works
- ✅ Already called from Python (CTester)
- ✅ No new code needed
- ✅ Maintains single API layer

**Cons:**
- ❌ Uses `__stdcall` (Windows DLL convention, not ideal for cross-platform)
- ❌ CTester-specific structures (COrderInfo, CRates, CRatesInfo)
- ❌ More complex parameter conversion
- ❌ Tied to Framework API structure
- ❌ Not optimized for Python/broker REST API use case

#### Option B: New TradingStrategiesPythonAPI (Recommended)
**Pros:**
- ✅ Cleaner, Python-focused API
- ✅ Standard C calling convention (cross-platform)
- ✅ Simpler, Python-friendly data structures
- ✅ Direct to TradingStrategies (one less layer)
- ✅ Better suited for broker REST API integration
- ✅ No `__stdcall` dependency

**Cons:**
- ❌ Requires new implementation
- ❌ Duplicates some conversion logic (but simpler)

### Target Architecture (Option B - Recommended)
```
┌─────────────────────────────────────────────────────────────┐
│                    Target Architecture                        │
├─────────────────────────────────────────────────────────────┤
│  Python Trading Platform (Broker REST API)                   │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Python Wrapper (trading_strategies.py)              │   │
│  │  • ctypes bindings                                    │   │
│  │  • Data structure conversion                         │   │
│  │  • Error handling                                    │   │
│  └────────────────────┬─────────────────────────────────┘   │
│                       │                                        │
│  ┌────────────────────▼─────────────────────────────────┐   │
│  │  TradingStrategiesPythonAPI (C API)                   │   │
│  │  • Standard C calling convention (no __stdcall)      │   │
│  │  • Python-friendly data structures                  │   │
│  │  • Input/output conversion                           │   │
│  └────────────────────┬─────────────────────────────────┘   │
│                       │                                        │
│  ┌────────────────────▼─────────────────────────────────┐   │
│  │  TradingStrategies (Shared Library)                   │   │
│  │  • Strategy execution logic (unchanged)              │   │
│  │  • Optional file I/O (can be disabled)              │   │
│  │  • StrategyParams → StrategyResults                  │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘

Note: AsirikuyFrameworkAPI remains for MQL4/MQL5 support
```

### Technology Stack
- **C Library**: Standard C (C99), no platform-specific code
- **Python**: Python 3.8+ with ctypes
- **Build System**: premake4 (existing)
- **Testing**: CUnit (C), pytest (Python)
- **Documentation**: Doxygen (C), Sphinx (Python)

## 📊 Scope & Features

### In Scope
1. **Python API Layer**
   - Create `TradingStrategiesPythonAPI.h/c` with Python-friendly C API
   - Implement input/output structure conversion
   - Standard C calling convention (no `__stdcall`)
   - Memory management for Python interop

2. **Build System Updates**
   - Change from `StaticLib` to `SharedLib` in premake4.lua
   - Support cross-platform builds (Linux, macOS, Windows)
   - Generate shared library (.so, .dylib, .dll)

3. **Python Wrapper**
   - ctypes-based Python wrapper (`trading_strategies.py`)
   - Python-friendly data structures (dataclasses)
   - Error handling and exception mapping
   - Example usage and documentation

4. **File I/O Abstraction** (Optional)
   - Make file-based I/O optional (can be disabled)
   - Support callback-based I/O for Python integration
   - Maintain backward compatibility

5. **Documentation**
   - API documentation for C functions
   - Python usage examples
   - Integration guide for broker REST APIs
   - Migration guide from DLL to Python

### Out of Scope
- Modifying existing strategy logic
- Changing StrategyParams or StrategyResults structures (internal)
- Removing file I/O completely (keeping as optional)
- Creating C++ bindings
- Creating bindings for other languages
- Performance optimization (unless required)

## 🔧 Technical Requirements

### Functional Requirements

#### FR-001: Python API Function
- **Description**: Provide `trading_strategies_run()` function callable from Python
- **Input**: PythonStrategyInput structure with flattened arrays
- **Output**: PythonStrategyOutput structure with signals and UI values
- **Error Handling**: Return AsirikuyReturnCode with error messages
- **Memory**: Allocate output arrays, provide free function

#### FR-002: Data Structure Conversion
- **Description**: Convert between Python-friendly structures and internal StrategyParams
- **Input Conversion**: PythonStrategyInput → StrategyParams
  - Flattened arrays → RatesBuffers
  - Settings dictionary → settings array
  - Orders list → OrderInfo array
- **Output Conversion**: StrategyResults → PythonStrategyOutput
  - Extract signals, UI values, status messages

#### FR-003: Build System
- **Description**: Build TradingStrategies as shared library
- **Platforms**: Linux (.so), macOS (.dylib), Windows (.dll)
- **Dependencies**: Link against AsirikuyCommon, AsirikuyTechnicalAnalysis, etc.
- **Configuration**: Support debug/release builds

#### FR-004: Python Wrapper
- **Description**: Python module for calling C library
- **Loading**: Auto-detect library path across platforms
- **Types**: Map C types to Python types (ctypes)
- **Error Handling**: Convert return codes to Python exceptions
- **Documentation**: Docstrings for all public functions

#### FR-005: Error Messages
- **Description**: Human-readable error messages for all return codes
- **Function**: `trading_strategies_get_error_message(return_code)`
- **Coverage**: All AsirikuyReturnCode enum values
- **Language**: English (can be extended later)

### Non-Functional Requirements

#### NFR-001: Performance
- **Target**: No more than 5% performance overhead vs. direct C calls
- **Measurement**: Benchmark strategy execution time
- **Acceptance**: Strategy execution time < 1.1x baseline

#### NFR-002: Memory Management
- **Requirement**: No memory leaks in Python interop
- **Validation**: Use valgrind (Linux) or similar tools
- **Acceptance**: Zero memory leaks in test suite

#### NFR-003: Thread Safety
- **Requirement**: Library must be thread-safe (if strategies are)
- **Documentation**: Document thread-safety guarantees
- **Testing**: Multi-threaded test cases

#### NFR-004: Platform Support
- **Requirement**: Support Linux, macOS, Windows
- **Priority**: Linux and macOS (primary), Windows (secondary)
- **Testing**: Build and test on all platforms

#### NFR-005: Backward Compatibility
- **Requirement**: Existing strategy logic unchanged
- **Validation**: Compare outputs before/after refactor
- **Acceptance**: Identical strategy results

## 📅 Timeline

### Phase 1: Specification & Planning (Week 1)
- **Duration**: 3-5 days
- **Deliverables**:
  - Complete specification document
  - Implementation plan
  - Task breakdown
  - Architecture diagrams

### Phase 2: C API Implementation (Week 2-3)
- **Duration**: 8-10 days
- **Deliverables**:
  - TradingStrategiesPythonAPI.c implementation
  - Build system updates
  - C unit tests
  - Basic Python wrapper

### Phase 3: Python Integration (Week 4)
- **Duration**: 5-7 days
- **Deliverables**:
  - Complete Python wrapper
  - Example usage code
  - Integration tests
  - Documentation

### Phase 4: Testing & Validation (Week 5)
- **Duration**: 3-5 days
- **Deliverables**:
  - Comprehensive test suite
  - Performance benchmarks
  - Memory leak validation
  - Cross-platform testing

### Phase 5: Documentation & Cleanup (Week 6)
- **Duration**: 2-3 days
- **Deliverables**:
  - API documentation
  - Usage guide
  - Migration guide
  - Code cleanup

**Total Estimated Duration**: 4-6 weeks

## ⚠️ Risk Assessment

### Technical Risks

#### Risk-001: Settings Array Mapping
- **Severity**: Medium
- **Description**: Mapping Python settings dict to C array indices may be error-prone
- **Mitigation**: Create comprehensive mapping table, validate at runtime
- **Contingency**: Use configuration file for mapping

#### Risk-002: Memory Management
- **Severity**: Medium
- **Description**: Python/C memory interop can cause leaks or crashes
- **Mitigation**: Thorough testing with valgrind, clear ownership rules
- **Contingency**: Use Python memory management where possible

#### Risk-003: Performance Overhead
- **Severity**: Low
- **Description**: Data conversion may add overhead
- **Mitigation**: Profile and optimize hot paths
- **Contingency**: Acceptable if < 5% overhead

#### Risk-004: Platform Compatibility
- **Severity**: Low
- **Description**: Shared library builds may differ across platforms
- **Mitigation**: Test on all target platforms early
- **Contingency**: Platform-specific build configurations

### Project Risks

#### Risk-005: Scope Creep
- **Severity**: Medium
- **Description**: Temptation to refactor strategy logic
- **Mitigation**: Strict scope boundaries, code review
- **Contingency**: Defer to separate refactoring initiative

#### Risk-006: Testing Coverage
- **Severity**: Medium
- **Description**: Incomplete testing may miss integration issues
- **Mitigation**: Comprehensive test plan, automated testing
- **Contingency**: Extended testing phase

## 📝 Dependencies

### Internal Dependencies
- **AsirikuyCommon**: Required for data structures and utilities
- **AsirikuyTechnicalAnalysis**: Required for indicator calculations
- **OrderManager**: Required for order management (if used)
- **Log**: Required for logging (optional)

### External Dependencies
- **Python 3.8+**: For Python wrapper
- **premake4**: For build system
- **C Compiler**: GCC/Clang (Linux/macOS), MSVC (Windows)

### Documentation Dependencies
- Understanding of StrategyParams structure
- Understanding of StrategyResults structure
- Settings array index mapping
- Strategy execution flow

## ✅ Validation Criteria

### Functional Validation
- [ ] Python can successfully load shared library
- [ ] `trading_strategies_run()` executes without errors
- [ ] Input conversion produces correct StrategyParams
- [ ] Output conversion extracts all signals and UI values
- [ ] Error messages are human-readable
- [ ] Memory is properly freed

### Integration Validation
- [ ] Python wrapper works with mock data
- [ ] Python wrapper works with real market data
- [ ] Broker API integration example works
- [ ] Multiple strategy instances can run concurrently (if thread-safe)

### Performance Validation
- [ ] Strategy execution time < 1.1x baseline
- [ ] Memory usage is reasonable
- [ ] No memory leaks detected

### Quality Validation
- [ ] Code passes linting/formatting checks
- [ ] All tests pass
- [ ] Documentation is complete
- [ ] Examples work as documented

## 📚 References

### Internal Documents
- `TradingStrategies/REFACTORING_PLAN.md` - Detailed refactoring options
- `TradingStrategies/IMPLEMENTATION_ROADMAP.md` - Implementation steps
- `AsirikuyCommon/include/AsirikuyDefines.h` - Data structure definitions
- `TradingStrategies/include/AsirikuyStrategies.h` - Strategy API

### External References
- Python ctypes documentation: https://docs.python.org/3/library/ctypes.html
- C interop best practices
- Shared library build guides

## 🔄 Future Considerations

### Potential Enhancements (Out of Scope)
1. **CFFI Alternative**: Consider CFFI instead of ctypes for better Python integration
2. **NumPy Integration**: Direct NumPy array support for rates data
3. **Async Support**: Asynchronous strategy execution
4. **State Persistence**: Better state management for Python
5. **Multiple Timeframes**: Native support for multiple timeframes in single call
6. **Callback Interface**: Full callback-based I/O (Phase 2)
7. **TrendStrategy.c Refactoring**: Split monolithic 8,988-line file into strategy-based modules
   - See `future_considerations/TrendStrategy-refactoring.md` for detailed plan
   - Extract common code to shared modules
   - Organize by strategy type (MACD, Ichimoko, BBS, etc.)
   - Improve maintainability and testability

### Migration Path
- Keep DLL support for MQL4/MQL5 (if needed)
- Gradually migrate to Python API
- Deprecate DLL API after migration complete
- Post-MVP: Refactor TrendStrategy.c for better code organization

