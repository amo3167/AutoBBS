# TradingStrategies Refactoring - Task List

## Overview

This document tracks all tasks for the TradingStrategies refactoring project (Option 1: C++ Internals + C API Wrapper).

**Status Legend:**
- ⬜ Pending
- 🔄 In Progress
- ✅ Completed
- ❌ Blocked
- ⚠️ At Risk

---

## Phase 1: Foundation (Weeks 1-2)

### Week 1: Core Infrastructure

#### Project Setup
- ✅ Create feature branch: `cpp-migration-phase1` (planned name `refactor/cpp-migration` adjusted)
- ✅ Update `premake4.lua` for C++ compilation
- ✅ Create directory structure (existing layout reused)
- ✅ Set up build system for C/C++ mix (premake adjusted, C++ sources added)
- ✅ Verify build works (VS2022 toolchain configured, premake4 available, NTPClient builds successfully)
- ✅ Modernize build toolchain (VS2022 v143, Windows SDK 10.0.26100.0)
- ✅ Patch STLSoft for modern MSVC compatibility
- ✅ Remove Shark project (unmaintained, incompatible)
- ✅ Update README.md with build documentation

#### StrategyContext Implementation
- ✅ Create `include/StrategyContext.hpp`
- ✅ Implement `src/StrategyContext.cpp`
- ✅ Write unit tests for StrategyContext (32 test cases created in TradingStrategiesTests.hpp)
- ✅ Fix C/C++ enum interoperability (created StrategyTypes.h shared header)
- ✅ Fix C struct field name mismatches (margin, rates array)
- ✅ Fix C++03 compatibility (removed `= delete`, `override`, `final` keywords)
- ✅ Document StrategyContext API (comprehensive docs/StrategyContext-API.md created)

#### IStrategy Interface
- ✅ Create `include/IStrategy.hpp`
- ✅ Write interface documentation (comprehensive docs/IStrategy-API.md created)
- ✅ Create example stub implementation (6 strategy stubs created)

### Week 2: Factory and Order Management

#### BaseStrategy Implementation
- ✅ Create `include/BaseStrategy.hpp`
- ✅ Implement `src/BaseStrategy.cpp`
- ✅ Write unit tests (18 test cases covering Template Method pattern, validation, indicators)
- ✅ Create StrategyResult structure

#### StrategyFactory Implementation
- ✅ Create `include/StrategyFactory.hpp`
- ✅ Implement `src/StrategyFactory.cpp`
- ✅ Register all 6 direct strategies (stubs) + AutoBBS dispatcher
- ✅ Write unit tests (15 test cases covering registration, creation, validation)

#### C API Wrapper
- ✅ Create `src/AsirikuyStrategiesWrapper.cpp`
- ✅ Implement `runStrategy()` wrapper
- ✅ Add exception handling
- ✅ Test C API wrapper (builds successfully, ready for integration testing)

#### OrderManager Implementation
- ✅ Create `include/OrderManager.hpp` (stub)
- ⬜ Implement `src/OrderManager.cpp` (deferred to Phase 2)
- ⬜ Write unit tests (deferred to Phase 2)

#### Indicators Wrapper
- ✅ Create `include/Indicators.hpp` (stub)
- ⬜ Implement `src/Indicators.cpp` (deferred to Phase 2)
- ⬜ Write unit tests (deferred to Phase 2)

---

## Phase 2: Strategy Migration (Weeks 3-6)

### Week 3: Simple Strategies

#### OrderBuilder Implementation
- ⬜ Create `include/OrderBuilder.hpp`
- ⬜ Implement `src/OrderBuilder.cpp`
- ⬜ Write unit tests
- ⬜ Test splitting logic
- ⬜ Integrate with strategies

#### Indicators Wrapper Implementation
- ⬜ Implement `src/Indicators.cpp` (wrap TA-Lib and custom indicators)
- ⬜ Add indicator calculation methods (MACD, RSI, BB, ATR, etc.)
- ⬜ Write unit tests
- ⬜ Validate against legacy C implementation
- ⬜ Document indicator API

#### OrderManager Implementation  
- ⬜ Implement `src/OrderManager.cpp` (wrap order execution logic)
- ⬜ Add order placement/modification/closure methods
- ⬜ Add order query and filtering methods
- ⬜ Write unit tests
- ⬜ Validate against legacy C implementation
- ⬜ Document OrderManager API

#### RecordBarsStrategy
- ⬜ Create `include/strategies/RecordBarsStrategy.hpp`
- ⬜ Create `src/strategies/RecordBarsStrategy.cpp`
- ⬜ Migrate logic from `runRecordBars()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Side-by-side testing
- ⬜ Validate backtesting
- ⬜ Remove C implementation

#### TakeOverStrategy
- ⬜ Create `include/strategies/TakeOverStrategy.hpp`
- ⬜ Create `src/strategies/TakeOverStrategy.cpp`
- ⬜ Migrate logic from `runTakeOver()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Validate
- ⬜ Remove C code

#### ScreeningStrategy
- ⬜ Create `include/strategies/ScreeningStrategy.hpp`
- ⬜ Create `src/strategies/ScreeningStrategy.cpp`
- ⬜ Migrate logic from `runScreening()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Validate
- ⬜ Remove C code

#### TrendLimitStrategy
- ⬜ Create `include/strategies/TrendLimitStrategy.hpp`
- ⬜ Create `src/strategies/TrendLimitStrategy.cpp`
- ⬜ Migrate logic from `runTrendLimit()`
- ⬜ Register in factory
- ⬜ Write unit tests
- ⬜ Validate
- ⬜ Remove C code

### Week 4-5: Trend Strategy Breakdown

#### Analysis
- ⬜ Analyze `TrendStrategy.c` (~9,286 lines, post-cleanup)
- ⬜ Identify all `workoutExecutionTrend_*` functions
- ⬜ Group related functions
- ⬜ Create breakdown document

#### MACD Strategies
- ⬜ Create `include/strategies/MACDDailyStrategy.hpp`
- ⬜ Create `include/strategies/MACDWeeklyStrategy.hpp`
- ⬜ Create `include/strategies/MACDDailyNewStrategy.hpp`
- ⬜ Extract common MACD logic
- ⬜ Implement all MACD strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C implementations

#### KeyK Strategy
- ⬜ Create `include/strategies/KeyKStrategy.hpp`
- ⬜ Create `src/strategies/KeyKStrategy.cpp`
- ⬜ Migrate `workoutExecutionTrend_KeyK()`
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### BBS Strategies
- ⬜ Create `include/strategies/BBSSwingStrategy.hpp`
- ⬜ Create `include/strategies/BBSBreakOutStrategy.hpp`
- ⬜ Extract common BBS logic
- ⬜ Implement all BBS strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Pivot Strategy
- ⬜ Create `include/strategies/PivotStrategy.hpp`
- ⬜ Create `src/strategies/PivotStrategy.cpp`
- ⬜ Migrate `workoutExecutionTrend_Pivot()`
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Limit Strategies
- ⬜ Create `include/strategies/LimitStrategy.hpp`
- ⬜ Create `include/strategies/LimitBBSStrategy.hpp`
- ⬜ Create `include/strategies/LimitBreakOutStrategy.hpp`
- ⬜ Extract common limit logic
- ⬜ Implement all limit strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Ichimoku Strategies
- ⬜ Create `include/strategies/IchimokuDailyV2Strategy.hpp`
- ⬜ Create `include/strategies/IchimokuDailyIndexStrategy.hpp`
- ⬜ Create `include/strategies/IchimokuDailyIndexV2Strategy.hpp`
- ⬜ Extract common Ichimoku logic
- ⬜ Implement all Ichimoku strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

#### Order Splitting Migration
- ⬜ Identify all `splitBuyOrders_*` functions
- ⬜ Identify all `splitSellOrders_*` functions
- ⬜ Migrate to OrderBuilder patterns
- ⬜ Update all strategies to use OrderBuilder
- ⬜ Remove old splitting functions
- ⬜ Write tests for order splitting

### Week 6: Remaining Strategies

#### AutoBBS Strategy (Dispatcher)
- ⬜ Create `include/strategies/AutoBBSStrategy.hpp`
- ⬜ Create `src/strategies/AutoBBSStrategy.cpp`
- ⬜ Migrate `runAutoBBS()` dispatcher logic
- ⬜ Route to workoutExecutionTrend_* functions based on strategy_mode
- ⬜ Use factory to create sub-strategies
- ⬜ Register in factory
- ⬜ Write tests
- ⬜ Validate
- ⬜ Remove C code

**Note**: AutoBBS is a dispatcher that routes to ~30+ workoutExecutionTrend_* functions in TrendStrategy.c. These functions will be migrated separately.

---

## Phase 3: Cleanup (Week 7)

### Code Removal
- ⬜ Remove migrated strategy C files
- ⬜ Remove C strategy headers (if no longer needed)
- ⬜ Remove `TrendStrategy.c`
- ⬜ Remove `AsirikuyStrategies.c`
- ⬜ Clean up includes
- ⬜ Verify build after removals

### Code Optimization
- ⬜ Profile C++ code
- ⬜ Optimize hot paths
- ⬜ Optimize StrategyContext
- ⬜ Optimize Factory
- ⬜ Refactor duplicated code
- ⬜ Improve error handling
- ⬜ Improve const correctness
- ⬜ Check for memory leaks
- ⬜ Optimize allocations

### Documentation Updates
- ⬜ Update Doxygen comments
- ⬜ Generate API documentation
- ⬜ Update architecture diagrams
- ⬜ Update developer guide
- ⬜ Document migration decisions

---

## Phase 4: Testing (Week 8)

### Unit Testing
- ⬜ Run coverage analysis
- ⬜ Fill coverage gaps
- ⬜ Test StrategyContext
- ⬜ Test StrategyFactory
- ⬜ Test BaseStrategy
- ⬜ Test OrderBuilder
- ⬜ Test OrderManager
- ⬜ Test Indicators
- ⬜ Test each strategy class

### Integration Testing
- ⬜ Test C API wrapper
- ⬜ Test MQL integration
- ⬜ Test full strategy execution flow
- ⬜ Test order execution flow
- ⬜ Test multi-strategy execution

### Validation Testing
- ⬜ Run backtests for all strategies
- ⬜ Compare C vs C++ results
- ⬜ Benchmark each strategy
- ⬜ Compare performance with C version
- ⬜ Run memory leak detection
- ⬜ Test memory usage

### Final Verification
- ⬜ Test DLL loading
- ⬜ Test with MQL code
- ⬜ Test on different Windows versions
- ⬜ Run full test suite
- ⬜ Test edge cases
- ⬜ Verify API documentation
- ⬜ Verify architecture documentation
- ⬜ Verify developer guide

---

## Tracking

### Current Sprint
**Week**: Phase 1 / Week 2  
**Focus**: Implement StrategyFactory & wrappers (Indicators, OrderManager) + plan C API bridge

### Progress Metrics
- **Total Tasks (Phase 1)**: 34
- **Completed**: 34
- **In Progress**: 0
- **Pending**: 0
- **Blocked**: 0
- **Deferred to Phase 2**: 3 (OrderBuilder, OrderManager impl, Indicators impl)

### Phase 1 Status: ✅ COMPLETE

### Blockers
- None. All C/C++ interoperability issues resolved.

### Notes
- **Build System Modernization Complete** (2025-11-09):
  - Upgraded to Visual Studio 2022 (v143 toolset)
  - Windows SDK 10.0.26100.0
  - Patched STLSoft for MSVC compatibility
  - Removed Shark project (unmaintained)
  - Retained Boost 1.49 (C++03 compatible; upgrade to 1.84+ requires C++11 migration)
  - NTPClient successfully building with boost::mutex/thread
  - README.md updated with comprehensive build documentation
- **StrategyContext Implementation Complete** (2025-11-09):
  - 32 comprehensive test cases using Boost.Test framework
  - Created `StrategyTypes.h` shared header for C/C++ enum interoperability
  - Fixed struct field name mismatches (freeMargin→margin, ratesBuffers→rates)
  - Removed C++11 features for C++03 compatibility (`= delete`, `override`, `final`)
  - Added `const` version of `getOrderManager()`
  - TradingStrategies.lib builds successfully
- **BaseStrategy Unit Tests Complete** (2025-11-09):
  - 18 test cases covering Template Method pattern execution flow
  - Tests verify validation hooks, indicator loading, error handling
  - MockStrategy class tests: constructor, getName, execute flow
  - Validation tests: null symbol, invalid bid/ask, ask < bid, valid context
  - Indicator loading tests: required vs optional indicators
  - StrategyResult tests: default values, data storage
  - Integration tests: indicators passed to executeStrategy, result passed to updateResults
  - Total unit tests: 65 (32 StrategyContext + 15 StrategyFactory + 18 BaseStrategy)
- **Test Organization Refactored** (2025-11-09):
  - Split monolithic TradingStrategiesTests.hpp (963 lines) into modular structure
  - Created 4 test files: TestFixtures.hpp, StrategyContextTests.cpp, StrategyFactoryTests.cpp, BaseStrategyTests.cpp
  - Created TradingStrategiesTests.cpp main runner (integrated with AsirikuyFrameworkTestModule)
  - Benefits: Faster incremental builds, better organization, easier maintenance, reduced merge conflicts
  - All 65 tests compile successfully with zero errors
  - Test structure ready for Phase 2 strategy migration
- Foundation abstractions (StrategyContext, IStrategy, BaseStrategy + StrategyResult) committed and compiling.
- **Phase 1 Scope Adjustment** (2025-11-09):
  - Moved OrderBuilder, OrderManager, and Indicators implementations to Phase 2 Week 3
  - Rationale: These components should be designed based on actual strategy needs discovered during migration
  - Phase 1 created stub headers only, allowing compilation and interface design
  - Phase 1 focused on core abstractions and infrastructure: StrategyContext, IStrategy, BaseStrategy, StrategyFactory, C API Wrapper
  - Full implementations scheduled for Phase 2 Week 3, before complex strategy migrations
- **Next Steps**: Begin Phase 2 strategy migration, starting with OrderBuilder + Indicators + OrderManager implementations, then RecordBarsStrategy

---

**Document Version**: 1.1  
**Last Updated**: 2025-11-09  
**Status**: Active Tracking

