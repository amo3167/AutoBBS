# Phase 2: Strategy Migration - Detailed Plan

## Overview

**Duration**: 4 weeks  
**Goal**: Migrate 6 direct strategies + AutoBBS dispatcher + ~30+ workoutExecutionTrend_* functions from C to C++ classes

**Note**: After cleanup, only 6 direct strategies remain. AutoBBS dispatcher routes to ~30+ workoutExecutionTrend_* functions in TrendStrategy.c.

**Approach**: Incremental migration, one strategy at a time, with side-by-side testing

---

## Week 3: Simple Strategies (Foundation)

### Strategy: RecordBarsStrategy

#### Tasks
- [ ] Create `include/strategies/RecordBarsStrategy.hpp`
  - [ ] Inherit from BaseStrategy
  - [ ] Define class structure
- [ ] Create `src/strategies/RecordBarsStrategy.cpp`
  - [ ] Migrate logic from `runRecordBars()` in `RecordBars.c`
  - [ ] Implement `loadIndicators()`
  - [ ] Implement `executeStrategy()`
  - [ ] Implement `updateResults()`
  - [ ] Use OrderBuilder for orders
- [ ] Register in StrategyFactory
- [ ] Write unit tests
- [ ] Side-by-side testing (C vs C++)
- [ ] Validate backtesting results match
- [ ] Remove C implementation (or mark as deprecated)

#### Migration Checklist
- [ ] Logic migrated correctly
- [ ] Order generation matches C version
- [ ] Results update correctly
- [ ] Error handling works
- [ ] Performance acceptable

#### Deliverables
- RecordBarsStrategy class implemented
- Tests passing
- Validated against C version

---

### Strategy: TakeOverStrategy

#### Tasks
- [ ] Create `include/strategies/TakeOverStrategy.hpp`
- [ ] Create `src/strategies/TakeOverStrategy.cpp`
  - [ ] Migrate from `runTakeOver()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- TakeOverStrategy implemented and validated

---

### Strategy: ScreeningStrategy

#### Tasks
- [ ] Create `include/strategies/ScreeningStrategy.hpp`
- [ ] Create `src/strategies/ScreeningStrategy.cpp`
  - [ ] Migrate from `runScreening()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- ScreeningStrategy implemented and validated

---

### Strategy: TrendLimitStrategy

#### Tasks
- [ ] Create `include/strategies/TrendLimitStrategy.hpp`
- [ ] Create `src/strategies/TrendLimitStrategy.cpp`
  - [ ] Migrate from `runTrendLimit()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- TrendLimitStrategy implemented and validated

---

### Week 3 Completion

- [ ] 4 direct strategies migrated (RecordBars, TakeOver, Screening, TrendLimit)
- [ ] All tests passing
- [ ] All validations complete
- [ ] Pattern established for remaining strategies

---

## Week 4-5: Trend Strategy Breakdown

### Analysis Phase

#### Tasks
- [ ] Analyze `TrendStrategy.c` (~9,286 lines, post-cleanup)
- [ ] Identify all `workoutExecutionTrend_*` functions
- [ ] Group related functions:
  - [ ] MACD functions → MACD strategies
  - [ ] KeyK functions → KeyK strategies
  - [ ] BBS functions → BBS strategies
  - [ ] Pivot functions → Pivot strategies
  - [ ] Limit functions → Limit strategies
  - [ ] Ichimoku functions → Ichimoku strategies
- [ ] Identify order splitting functions
- [ ] Create breakdown document

#### Deliverables
- Strategy breakdown document
- Function grouping list
- Migration plan for TrendStrategy.c

---

### MACD Strategies

#### Tasks
- [ ] Create `include/strategies/MACDDailyStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_MACD_Daily()`
- [ ] Create `include/strategies/MACDWeeklyStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_MACD_Weekly()`
- [ ] Create `include/strategies/MACDDailyNewStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_MACD_Daily_New()`
- [ ] Extract common MACD logic to base class
- [ ] Implement all MACD strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate each strategy
- [ ] Remove C implementations

#### Deliverables
- All MACD strategies migrated
- Common logic extracted
- Tests and validation complete

---

### KeyK Strategy

#### Tasks
- [ ] Create `include/strategies/KeyKStrategy.hpp`
- [ ] Create `src/strategies/KeyKStrategy.cpp`
  - [ ] Migrate `workoutExecutionTrend_KeyK()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- KeyKStrategy implemented and validated

---

### BBS Strategies

#### Tasks
- [ ] Create `include/strategies/BBSSwingStrategy.hpp`
  - [ ] Migrate BBS swing logic
- [ ] Create `include/strategies/BBSBreakOutStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_BBS_BreakOut()`
- [ ] Extract common BBS logic
- [ ] Implement all BBS strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- All BBS strategies migrated
- Common logic extracted

---

### Pivot Strategy

#### Tasks
- [ ] Create `include/strategies/PivotStrategy.hpp`
- [ ] Create `src/strategies/PivotStrategy.cpp`
  - [ ] Migrate `workoutExecutionTrend_Pivot()`
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- PivotStrategy implemented and validated

---

### Limit Strategies

#### Tasks
- [ ] Create `include/strategies/LimitStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Limit()`
- [ ] Create `include/strategies/LimitBBSStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Limit_BBS()`
- [ ] Create `include/strategies/LimitBreakOutStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Limit_BreakOutOnPivot()`
- [ ] Extract common limit logic
- [ ] Implement all limit strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- All limit strategies migrated

---

### Ichimoku Strategies

#### Tasks
- [ ] Create `include/strategies/IchimokuDailyV2Strategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Ichimoko_Daily_V2()`
- [ ] Create `include/strategies/IchimokuDailyIndexStrategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Ichimoko_Daily_Index()`
- [ ] Create `include/strategies/IchimokuDailyIndexV2Strategy.hpp`
  - [ ] Migrate `workoutExecutionTrend_Ichimoko_Daily_Index_V2()`
- [ ] Extract common Ichimoku logic
- [ ] Implement all Ichimoku strategies
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- All Ichimoku strategies migrated

---

### Order Splitting Functions Migration

#### Tasks
- [ ] Identify all `splitBuyOrders_*` and `splitSellOrders_*` functions
- [ ] Migrate to OrderBuilder patterns
- [ ] Update all strategies to use OrderBuilder
- [ ] Remove old splitting functions
- [ ] Write tests for order splitting

#### Deliverables
- Order splitting migrated to OrderBuilder
- All strategies using new pattern

---

## Week 6: Remaining Strategies

### AutoBBS Strategy (Dispatcher)

#### Tasks
- [ ] Create `include/strategies/AutoBBSStrategy.hpp`
- [ ] Create `src/strategies/AutoBBSStrategy.cpp`
  - [ ] Migrate `runAutoBBS()` dispatcher logic
  - [ ] Use factory to create sub-strategies
  - [ ] Route based on strategy_mode
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- AutoBBSStrategy implemented as dispatcher

---

### AutoBBS Strategy (Dispatcher)

**Note**: AutoBBS is a dispatcher that routes to workoutExecutionTrend_* functions based on strategy_mode. The actual strategy implementations are in TrendStrategy.c.

#### Tasks
- [ ] Create `include/strategies/AutoBBSStrategy.hpp`
- [ ] Create `src/strategies/AutoBBSStrategy.cpp`
  - [ ] Migrate `runAutoBBS()` dispatcher logic
  - [ ] Use factory to create sub-strategies based on strategy_mode
  - [ ] Route to appropriate workoutExecutionTrend_* function
- [ ] Register in factory
- [ ] Write tests
- [ ] Validate
- [ ] Remove C code

#### Deliverables
- AutoBBSStrategy implemented as dispatcher

#### Pattern for Each
1. Create header and source files
2. Migrate logic from C function
3. Register in factory
4. Write tests
5. Validate
6. Remove C code

---

## Migration Template

For each strategy, follow this template:

### 1. Analysis
- [ ] Read C implementation
- [ ] Identify dependencies
- [ ] Identify order splitting logic
- [ ] Identify indicator usage

### 2. Design
- [ ] Design class structure
- [ ] Identify base class (BaseStrategy or custom)
- [ ] Plan order building
- [ ] Plan indicator loading

### 3. Implementation
- [ ] Create header file
- [ ] Create source file
- [ ] Migrate logic
- [ ] Use OrderBuilder
- [ ] Use Indicators wrapper

### 4. Testing
- [ ] Write unit tests
- [ ] Side-by-side testing
- [ ] Backtesting validation
- [ ] Performance check

### 5. Integration
- [ ] Register in factory
- [ ] Update documentation
- [ ] Code review
- [ ] Remove C code

---

## Validation Strategy

### Side-by-Side Testing

For each strategy:
```cpp
void validateStrategy(StrategyId id, StrategyParams* params) {
    // Run C version
    StrategyParams cParams = *params;
    AsirikuyReturnCode cResult = runStrategy_C(id, &cParams);
    
    // Run C++ version
    StrategyParams cppParams = *params;
    AsirikuyReturnCode cppResult = runStrategy(&cppParams);
    
    // Compare
    assert(cResult == cppResult);
    assert(compareResults(&cParams.results, &cppParams.results));
}
```

### Backtesting Validation

- Run full backtest with C version
- Run full backtest with C++ version
- Compare:
  - Trade entries/exits
  - Order sizes
  - Stop loss/take profit levels
  - Equity curves
  - Performance metrics

---

## Phase 2 Completion Criteria

### Functional Requirements
- [ ] All 6 direct strategies migrated to C++ (RecordBars, TakeOver, Screening, AutoBBS, AutoBBSWeekly, TrendLimit)
- [ ] AutoBBS dispatcher migrated
- [ ] All ~30+ workoutExecutionTrend_* functions migrated to C++ classes
- [ ] All strategies registered in factory
- [ ] TrendStrategy.c broken down into classes
- [ ] Order splitting migrated to OrderBuilder
- [ ] All strategies use Indicators wrapper

### Quality Requirements
- [ ] All unit tests passing
- [ ] All strategies validated against C versions
- [ ] Backtesting results match (within rounding)
- [ ] Performance within 5% of original
- [ ] Code reviewed

### Code Quality
- [ ] No code duplication
- [ ] Largest file < 500 lines
- [ ] Clear class hierarchy
- [ ] Well-documented

---

## Risks and Mitigation

### Risk: Large Strategy Files
- **Mitigation**: Break down into smaller classes, extract common logic

### Risk: Complex Logic Migration
- **Mitigation**: Migrate incrementally, test frequently, compare outputs

### Risk: Performance Regression
- **Mitigation**: Profile each strategy, optimize hot paths

### Risk: Missing Edge Cases
- **Mitigation**: Comprehensive testing, side-by-side validation

---

## Deliverables Summary

1. **Code**
   - All 6 direct strategy classes implemented
   - AutoBBS dispatcher implemented
   - All ~30+ workoutExecutionTrend_* functions migrated to C++ classes
   - TrendStrategy.c broken down
   - Order splitting migrated
   - All strategies registered

2. **Tests**
   - Unit tests for all strategies
   - Side-by-side validation tests
   - Backtesting validation

3. **Documentation**
   - Strategy migration log
   - Class documentation
   - Architecture updates

---

## Next Phase

After Phase 2 completion, proceed to:
- **[Phase 3: Cleanup](./PLAN_PHASE3_CLEANUP.md)**

---

**Document Version**: 1.1  
**Last Updated**: 2025-11-09  
**Status**: Ready for Implementation (pending StrategyFactory availability)

### Current Notes
- Phase 2 migration sequenced to start once `StrategyFactory` and C API wrapper are in place.
- Early priorities: `RecordBarsStrategy` migration (as the simplest unit) and `OrderBuilder` integration for order-splitting standardization.

