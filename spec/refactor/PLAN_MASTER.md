# TradingStrategies Refactoring - Master Plan

## Overview

This document provides the master plan for refactoring the TradingStrategies module from C to C++ (Option 1: C++ Internals + C API Wrapper).

**Duration**: 8 weeks  
**Approach**: Spec-driven, incremental migration  
**Goal**: Modernize internals while maintaining 100% DLL compatibility

---

## Plan Structure

This master plan is broken down into detailed phase plans:

1. **[Phase 1 Plan](./PLAN_PHASE1_FOUNDATION.md)** - Foundation (Weeks 1-2)
2. **[Phase 2 Plan](./PLAN_PHASE2_MIGRATION.md)** - Strategy Migration (Weeks 3-6)
3. **[Phase 3 Plan](./PLAN_PHASE3_CLEANUP.md)** - Cleanup (Week 7)
4. **[Phase 4 Plan](./PLAN_PHASE4_TESTING.md)** - Testing (Week 8)

---

## High-Level Timeline

```
Week 1: Core Infrastructure Setup
Week 2: Factory & Order Management
Week 3: Simple Strategy Migration
Week 4-5: Trend Strategy Breakdown
Week 6: Remaining Strategies
Week 7: Cleanup & Optimization
Week 8: Testing & Validation
```

---

## Key Principles

1. **Zero Breaking Changes**: DLL exports remain unchanged
2. **Incremental Migration**: One strategy at a time
3. **Side-by-Side Testing**: C and C++ versions run in parallel
4. **Spec-Driven**: Follow detailed specifications
5. **Test-Driven**: Write tests before/during migration

---

## Success Criteria

- [ ] All 6 direct strategies migrated to C++ (RECORD_BARS, TAKEOVER, SCREENING, AUTOBBS, AUTOBBSWEEKLY, TRENDLIMIT)
- [ ] AutoBBS dispatcher migrated (routes to ~30+ workoutExecutionTrend_* functions)
- [ ] All workoutExecutionTrend_* functions in TrendStrategy.c migrated to C++ classes
- [ ] DLL exports unchanged
- [ ] Backtesting results match original
- [ ] Performance within 5% of original
- [ ] 90%+ test coverage
- [ ] Zero memory leaks
- [ ] All documentation updated

---

## Risk Management

### Risks
1. **Breaking Changes**: Mitigated by maintaining C API
2. **Performance Regression**: Mitigated by profiling and optimization
3. **Migration Time**: Mitigated by incremental approach
4. **Team Learning Curve**: Mitigated by documentation and code reviews

### Rollback Plan
- Keep C implementations until migration validated
- Use feature flags to switch between C/C++ versions
- Maintain ability to revert individual strategies

---

## Dependencies

### External Dependencies
- C++17 compiler support
- Updated build system (premake4)
- Testing framework setup

### Internal Dependencies
- StrategyParams structure (unchanged)
- Base_Indicators structure (unchanged)
- Order management APIs (wrapped, not changed)

---

## Deliverables

### Phase 1
- Core C++ infrastructure
- C API wrapper
- Factory implementation
- Order management classes

### Phase 2
- All strategies migrated
- Monolithic files broken down
- Comprehensive unit tests

### Phase 3
- Old C code removed
- Optimized C++ code
- Updated documentation

### Phase 4
- Full test suite
- Validation reports
- Performance benchmarks

---

## Communication Plan

### Weekly Reviews
- Monday: Plan review and adjustments
- Friday: Progress review and demo

### Documentation Updates
- Update spec as needed
- Document decisions and trade-offs
- Maintain migration log

---

## Next Steps

1. Review and approve master plan
2. Set up project branch: `refactor/cpp-migration`
3. Begin Phase 1 (see [Phase 1 Plan](./PLAN_PHASE1_FOUNDATION.md))

---

**Document Version**: 1.1  
**Last Updated**: 2025-11-09  
**Status**: Active Planning

