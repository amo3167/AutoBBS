# Baseline Metrics - Current C Implementation

## Overview

This document captures performance and code metrics of the current C implementation (pre-refactoring) to establish a baseline for comparison during and after the C++ migration.

**Date**: November 9, 2025  
**Branch**: refactor  
**Commit**: [To be filled during measurement]

---

## Code Metrics

### Current Codebase Structure

#### TradingStrategies Module
- **Language**: C
- **Build System**: Premake4 → Visual Studio 2010
- **Total Strategies**: 6 direct + AutoBBS dispatcher (routes to ~30+ functions)
- **Source Files**: 10 files in `src/strategies/`
- **Largest File**: TrendStrategy.c (~9,286 lines)

#### File Inventory

| File | Lines | Purpose |
|------|-------|---------|
| `AsirikuyStrategies.c` | 334 | Main dispatcher (6 strategies) |
| `RecordBars.c` | ~500 | RECORD_BARS strategy |
| `TakeOver.c` | ~800 | TAKEOVER strategy |
| `Screening.c` | ~600 | SCREENING strategy |
| `AutoBBS.c` | ~1,200 | AutoBBS dispatcher |
| `TrendLimit.c` | ~400 | TRENDLIMIT strategy |
| `TrendStrategy.c` | ~9,286 | ~30+ workoutExecutionTrend_* functions |
| `StrategyComLib.c` | ~2,293 | Common strategy utilities |
| `SwingStrategy.c` | ~5,424 | Swing strategy implementations |
| `InstanceStates.c` | ~200 | Instance state management |

**Total Lines of Code**: ~20,000+ lines (estimated)

### Strategy ID Mapping

Current strategy IDs (from `AsirikuyStrategies.c`):

```c
typedef enum strategyId_t {
  RECORD_BARS       = 19,
  TAKEOVER          = 26,
  SCREENING         = 27,
  AUTOBBS           = 29,
  AUTOBBSWEEKLY     = 30,
  TRENDLIMIT        = 31
} StrategyId;
```

### AutoBBS Dispatcher Complexity

AutoBBS (ID: 29, 30) is a **dispatcher** that routes to workoutExecutionTrend_* functions based on `strategy_mode` parameter.

**Estimated Functions**: ~30+ workoutExecutionTrend_* implementations in TrendStrategy.c
- MACD variants (Daily, Weekly, etc.)
- KeyK strategies
- BBS (Bollinger Band Squeeze) variants
- Pivot strategies
- Limit strategies
- Ichimoku variants

---

## Performance Metrics

### Test Environment

**To be measured with:**
- Hardware: [CPU, RAM details]
- OS: Windows 10/11
- Build: Release x32 (Visual Studio 2010)
- Compiler: MSVC 10.0
- Test Data: Historical data (1 year, 1-minute bars)

### Execution Time Benchmarks

**To be measured for each strategy:**

| Strategy | Avg Execution Time (ms) | Std Dev (ms) | Min (ms) | Max (ms) |
|----------|------------------------|--------------|----------|----------|
| RECORD_BARS | TBD | TBD | TBD | TBD |
| TAKEOVER | TBD | TBD | TBD | TBD |
| SCREENING | TBD | TBD | TBD | TBD |
| AUTOBBS (MACD_Daily) | TBD | TBD | TBD | TBD |
| AUTOBBSWEEKLY | TBD | TBD | TBD | TBD |
| TRENDLIMIT | TBD | TBD | TBD | TBD |

**Test Methodology**:
1. Load historical data (EURUSD, 1 year, M1 timeframe)
2. Run strategy 1,000 iterations
3. Measure time per execution using high-resolution timer
4. Calculate mean, std dev, min, max
5. Exclude first 10 runs (warmup)

### Memory Usage

**To be measured:**

| Metric | Value |
|--------|-------|
| DLL Size | TBD KB |
| Runtime Memory (baseline) | TBD MB |
| Runtime Memory (peak) | TBD MB |
| Memory Leaks Detected | TBD (use valgrind/DrMemory) |
| Heap Allocations per Strategy Execution | TBD |

### Build Metrics

| Metric | Current C |
|--------|-----------|
| Build Time (Clean) | TBD seconds |
| Build Time (Incremental) | TBD seconds |
| DLL Size | TBD KB |
| Compiler Warnings | TBD (currently: C4013, C4101) |
| Linker Warnings | 0 (LNK4006 resolved) |

---

## Code Quality Metrics

### Complexity Metrics

**To be measured with tool (e.g., SourceMonitor, Lizard):**

| Metric | Value |
|--------|-------|
| Cyclomatic Complexity (avg) | TBD |
| Cyclomatic Complexity (max) | TBD |
| Function Count | TBD |
| Avg Function Length (LOC) | TBD |
| Max Function Length (LOC) | TBD |
| Code Duplication % | TBD |

### Known Issues in Current Implementation

1. **Monolithic Files**
   - TrendStrategy.c: 9,286 lines (difficult to maintain)
   - Multiple strategies in single file

2. **Code Duplication**
   - Order splitting logic duplicated across strategies
   - Indicator loading patterns repeated
   - Common entry/exit logic duplicated

3. **Manual Memory Management**
   - Potential for leaks
   - Error-prone cleanup paths

4. **Limited Type Safety**
   - Function pointers used for dispatch
   - Minimal compile-time checks

5. **Difficult Testing**
   - No unit test infrastructure
   - Integration testing only
   - Hard to test individual strategies in isolation

---

## Performance Targets for C++ Implementation

Based on baseline (to be filled), the C++ implementation should achieve:

### Performance Targets
- **Execution Time**: Within 5% of C version (or faster)
- **Memory Usage**: Within 10% of C version (or lower)
- **DLL Size**: Within 20% of C version (acceptable trade-off for C++ runtime)
- **Build Time**: Within 20% of C version

### Quality Targets
- **Cyclomatic Complexity**: Reduce average by 30%
- **Max Function Length**: < 100 lines (currently: ~500+ in some functions)
- **Code Duplication**: < 5% (currently: estimated 15-20%)
- **Test Coverage**: > 90% (currently: 0%)
- **Compiler Warnings**: 0 (currently: C4013, C4101)

---

## Measurement Tasks

### TODO: Baseline Measurements

- [ ] **Performance Benchmarks**
  - [ ] Set up test harness with high-resolution timer
  - [ ] Run RECORD_BARS 1,000 iterations
  - [ ] Run TAKEOVER 1,000 iterations
  - [ ] Run SCREENING 1,000 iterations
  - [ ] Run AUTOBBS variants 1,000 iterations
  - [ ] Run TRENDLIMIT 1,000 iterations
  - [ ] Document results

- [ ] **Memory Profiling**
  - [ ] Install DrMemory or valgrind
  - [ ] Run each strategy with memory profiler
  - [ ] Document baseline memory usage
  - [ ] Check for existing memory leaks
  - [ ] Document heap allocations per execution

- [ ] **Code Complexity Analysis**
  - [ ] Install SourceMonitor or Lizard
  - [ ] Run complexity analysis on all strategies
  - [ ] Document cyclomatic complexity metrics
  - [ ] Identify most complex functions (targets for refactoring)
  - [ ] Measure code duplication

- [ ] **Build Metrics**
  - [ ] Time clean build
  - [ ] Time incremental build
  - [ ] Measure final DLL size
  - [ ] Document compiler/linker warnings

---

## Validation Criteria

After C++ migration, validate that:

1. **Functional Correctness**
   - All strategies produce identical outputs (within rounding errors)
   - Backtesting results match C version
   - Order generation matches C version

2. **Performance Acceptance**
   - Execution time: C++ time ≤ C time × 1.05 (within 5%)
   - Memory usage: C++ memory ≤ C memory × 1.10 (within 10%)
   - No performance regressions for hot paths

3. **Quality Improvements**
   - Cyclomatic complexity reduced by 30%+
   - Test coverage > 90%
   - Zero compiler warnings
   - Code duplication < 5%

---

## Measurement Tools

### Recommended Tools

1. **Performance Profiling**
   - Windows Performance Analyzer
   - Very Sleepy (open-source profiler)
   - QueryPerformanceCounter API (high-res timer)

2. **Memory Profiling**
   - DrMemory (Windows, free)
   - Visual Studio Memory Profiler
   - Valgrind (if using WSL/Linux)

3. **Code Complexity**
   - SourceMonitor (free, Windows)
   - Lizard (open-source, cross-platform)
   - Visual Studio Code Metrics

4. **Code Duplication**
   - PMD CPD (Copy-Paste Detector)
   - SonarQube
   - Simian

---

## Next Steps

1. **Week 1, Day 1-2**: Complete all baseline measurements
2. **Document Results**: Fill in TBD values in this document
3. **Commit Baseline**: Commit this document with measurements
4. **Reference During Migration**: Compare C++ implementation against these baselines
5. **Final Validation**: Re-run all measurements after migration complete

---

## Notes

- Baseline measurements should be taken on same hardware for consistency
- Use Release build for performance measurements
- Take multiple measurements and use average/median
- Document any anomalies or outliers
- Keep raw data files for future reference

---

**Document Version**: 1.0  
**Status**: Template - Measurements Pending  
**Owner**: Phase 1 Implementation Team  
**Next Update**: After baseline measurements complete
