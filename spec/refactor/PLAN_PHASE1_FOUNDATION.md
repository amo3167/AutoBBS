# Phase 1: Foundation - Detailed Plan

## Overview

**Duration**: 2 weeks  
**Goal**: Establish core C++ infrastructure and C API wrapper layer

---

## Week 1: Core Infrastructure

### Day 1-2: Project Setup

#### Tasks
- [ ] Create feature branch: `refactor/cpp-migration`
- [ ] Update `premake4.lua` for C++ compilation
  - [ ] Change language from "C" to "C++"
  - [ ] Set C++ standard to C++17
  - [ ] Add C++ include paths
  - [ ] Configure C++ flags
- [ ] Create directory structure:
  ```
  include/
    StrategyContext.hpp
    IStrategy.hpp
    BaseStrategy.hpp
  src/
    StrategyContext.cpp
  ```
- [ ] Set up build system to compile both C and C++
- [ ] Verify build works with mixed C/C++

#### Deliverables
- Updated build configuration
- Directory structure created
- Build system verified

---

### Day 3-4: StrategyContext Implementation

#### Tasks
- [ ] Create `include/StrategyContext.hpp`
  - [ ] Define class structure
  - [ ] Add accessor methods for StrategyParams fields
  - [ ] Add lazy-loading for Indicators
  - [ ] Add OrderManager access
- [ ] Implement `src/StrategyContext.cpp`
  - [ ] Constructor with validation
  - [ ] All accessor implementations
  - [ ] Lazy-loading logic
- [ ] Write unit tests for StrategyContext
  - [ ] Test parameter access
  - [ ] Test lazy loading
  - [ ] Test error handling

#### Code Structure
```cpp
namespace trading {
    class StrategyContext {
        // Wraps StrategyParams with C++ interface
        // Provides type-safe accessors
        // Manages Indicators and OrderManager lifecycle
    };
}
```

#### Deliverables
- StrategyContext class implemented
- Unit tests passing
- Documentation

---

### Day 5: IStrategy Interface

#### Tasks
- [ ] Create `include/IStrategy.hpp`
  - [ ] Define pure virtual interface
  - [ ] Add execute() method
  - [ ] Add getName() and getId() methods
  - [ ] Add validate() method
- [ ] Write interface documentation
- [ ] Create example implementation (stub)

#### Deliverables
- IStrategy interface defined
- Interface documented
- Example stub implementation

---

## Week 2: Factory and Order Management

### Day 1-2: BaseStrategy Implementation

#### Tasks
- [ ] Create `include/BaseStrategy.hpp`
  - [ ] Inherit from IStrategy
  - [ ] Implement Template Method pattern
  - [ ] Define virtual methods for subclasses
- [ ] Implement `src/BaseStrategy.cpp`
  - [ ] Template method execute()
  - [ ] Default validation
  - [ ] Result handling
- [ ] Write unit tests
- [ ] Create StrategyResult structure

#### Deliverables
- BaseStrategy class implemented
- Template Method pattern working
- Unit tests passing

---

### Day 3-4: StrategyFactory Implementation

#### Tasks
- [ ] Create `include/StrategyFactory.hpp`
  - [ ] Define factory interface
  - [ ] Add create() methods
  - [ ] Add registry mechanism
- [ ] Implement `src/StrategyFactory.cpp`
  - [ ] Registry initialization
  - [ ] Strategy creation logic
  - [ ] Registration for all 6 direct strategies + AutoBBS dispatcher (stubs initially)
- [ ] Write unit tests
  - [ ] Test factory creation
  - [ ] Test invalid IDs
  - [ ] Test registry

#### Code Structure
```cpp
namespace trading {
    class StrategyFactory {
        static std::unique_ptr<IStrategy> create(StrategyId id);
        static void initializeRegistry();
        // Registry maps StrategyId to creator functions
    };
}
```

#### Deliverables
- StrategyFactory implemented
- All strategies registered (as stubs)
- Unit tests passing

---

### Day 5: C API Wrapper

#### Tasks
- [ ] Create `src/AsirikuyStrategiesWrapper.cpp`
  - [ ] Implement `runStrategy()` wrapper
  - [ ] Add exception handling
  - [ ] Convert C params to C++ context
  - [ ] Call factory and execute strategy
  - [ ] Convert results back
- [ ] Update `AsirikuyStrategies.c` to use wrapper (or keep both)
- [ ] Test C API wrapper
  - [ ] Test with null params
  - [ ] Test with invalid strategy ID
  - [ ] Test exception handling

#### Code Structure
```cpp
extern "C" {
    AsirikuyReturnCode runStrategy(StrategyParams* pParams) {
        try {
            trading::StrategyContext ctx(pParams);
            auto strategy = trading::StrategyFactory::create(ctx.getStrategyId());
            return strategy->execute(ctx);
        } catch (...) {
            return INTERNAL_ERROR;
        }
    }
}
```

#### Deliverables
- C API wrapper implemented
- Exception handling working
- Integration tests passing

---

## Week 2 Continued: Order Management

### Day 6-7: OrderBuilder Implementation

#### Tasks
- [ ] Create `include/OrderBuilder.hpp`
  - [ ] Define Order structure
  - [ ] Implement builder pattern
  - [ ] Add split options
- [ ] Implement `src/OrderBuilder.cpp`
  - [ ] Fluent interface methods
  - [ ] Split logic (multiple, ATR, risk-based)
  - [ ] Build method
- [ ] Write unit tests
  - [ ] Test order building
  - [ ] Test splitting
  - [ ] Test validation

#### Deliverables
- OrderBuilder class implemented
- Split logic working
- Unit tests passing

---

### Day 8-9: OrderManager Implementation

#### Tasks
- [ ] Create `include/OrderManager.hpp`
  - [ ] Define order management interface
  - [ ] Add order execution methods
  - [ ] Add order modification methods
- [ ] Implement `src/OrderManager.cpp`
  - [ ] Wrap existing order management functions
  - [ ] Add C++ convenience methods
- [ ] Write unit tests

#### Deliverables
- OrderManager class implemented
- Integration with existing order functions
- Unit tests passing

---

### Day 10: Indicators Wrapper

#### Tasks
- [ ] Create `include/Indicators.hpp`
  - [ ] Wrap Base_Indicators structure
  - [ ] Wrap Indicators structure
  - [ ] Add convenience methods
- [ ] Implement `src/Indicators.cpp`
  - [ ] Constructor
  - [ ] Indicator loading methods
  - [ ] Accessor methods
- [ ] Write unit tests

#### Deliverables
- Indicators wrapper implemented
- Indicator loading working
- Unit tests passing

---

## Phase 1 Completion Criteria

### Functional Requirements
- [ ] StrategyContext wraps StrategyParams correctly
- [ ] IStrategy interface defined and documented
- [ ] BaseStrategy implements Template Method pattern
- [ ] StrategyFactory can create all strategies (stubs)
- [ ] C API wrapper delegates to C++ correctly
- [ ] OrderBuilder can build and split orders
- [ ] OrderManager wraps order functions
- [ ] Indicators wrapper loads indicators

### Quality Requirements
- [ ] All unit tests passing
- [ ] Code compiles without warnings
- [ ] Documentation complete
- [ ] Code reviewed

### Integration Requirements
- [ ] Build system works with C/C++ mix
- [ ] DLL exports unchanged
- [ ] Can call C++ strategies from C API
- [ ] Exception handling works correctly

---

## Testing Strategy

### Unit Testing
- Test each class independently
- Mock dependencies where needed
- Test error cases

### Integration Testing
- Test C API → C++ flow
- Test factory creation
- Test order building and execution

### Validation Testing
- Compare C++ stub strategies with C versions
- Verify identical behavior (when implemented)

---

## Risks and Mitigation

### Risk: Build System Issues
- **Mitigation**: Test build early, fix issues immediately

### Risk: C/C++ Interop Problems
- **Mitigation**: Use extern "C" correctly, test thoroughly

### Risk: Exception Handling
- **Mitigation**: Catch all exceptions at C boundary, test edge cases

---

## Deliverables Summary

1. **Code**
   - StrategyContext class
   - IStrategy interface
   - BaseStrategy class
   - StrategyFactory with registry
   - C API wrapper
   - OrderBuilder class
   - OrderManager class
   - Indicators wrapper

2. **Tests**
   - Unit tests for all classes
   - Integration tests for C API
   - Validation tests

3. **Documentation**
   - API documentation
   - Architecture diagrams
   - Migration guide

4. **Build System**
   - Updated premake4.lua
   - C++ compilation working
   - Mixed C/C++ build verified

---

## Next Phase

After Phase 1 completion, proceed to:
- **[Phase 2: Strategy Migration](./PLAN_PHASE2_MIGRATION.md)**

---

**Document Version**: 1.1  
**Last Updated**: 2025-11-09  
**Status**: In Progress (foundation implemented; build verification blocked)

### Current Status Notes
- Foundation core abstractions implemented: `StrategyContext`, `IStrategy`, `BaseStrategy`, `StrategyResult`.
- `premake4.lua` modified for C++ compilation; solution regeneration/build pending due to missing `premake4` and `msbuild` in environment.
- Next actionable: implement `StrategyFactory` and stub strategies; then C API wrapper integration and unit test harness.

