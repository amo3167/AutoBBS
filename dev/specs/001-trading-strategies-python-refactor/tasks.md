# Tasks: TradingStrategies Python Integration Refactor

**Input**: Specification and plan from `/specs/001-trading-strategies-python-refactor/`
**Prerequisites**: spec.md (required), plan.md (required)

## Execution Flow
```
1. Load spec.md and plan.md from feature directory
   → Extract: requirements, architecture, timeline
2. Generate tasks by phase:
   → Phase 1: C API Implementation
   → Phase 2: Build System
   → Phase 3: Python Wrapper
   → Phase 4: Testing
   → Phase 5: Documentation
3. Apply task rules:
   → Different files = mark [P] for parallel
   → Same file = sequential (no [P])
   → Tests before implementation (TDD where possible)
4. Number tasks sequentially (T001, T002...)
5. Create dependency graph
6. Validate task completeness
```

## Format: `[ID] [P?] Description`
- **[P]**: Can run in parallel (different files, no dependencies)
- Include exact file paths in descriptions

## Phase 1: Setup & Planning

### Phase 1.1: Specification
- [x] T001 Create spec folder structure `specs/001-trading-strategies-python-refactor/`
- [x] T002 Create specification document `spec.md`
- [x] T003 Create implementation plan `plan.md`
- [x] T004 Create task breakdown `tasks.md`
- [x] T005 Create architecture document `architecture.md`
- [x] T006 Review and validate specification

### Phase 1.2: Environment Setup
- [ ] T007 [P] Set up development environment
- [ ] T008 [P] Review existing TradingStrategies codebase
- [ ] T009 [P] Identify all StrategyParams dependencies
- [x] T010 [P] Document settings array indices mapping
- [ ] T011 [P] Set up C testing framework (CUnit or similar)

## Phase 2: C API Implementation

### Phase 2.1: API Header Design
- [x] T012 Create `include/TradingStrategiesPythonAPI.h`
  - Define PythonStrategyInput structure
  - Define PythonStrategyOutput structure
  - Define function signatures
  - Add documentation comments

### Phase 2.2: Input Conversion Implementation
- [x] T013 Create `src/TradingStrategiesPythonAPI.c` skeleton
- [ ] T014 Implement `convert_python_input()` function
  - Convert symbol string
  - Convert bid/ask to BidAsk structure
  - Convert account info to AccountInfo structure
  - Convert rates arrays to RatesBuffers
  - Convert settings array
  - Convert orders array to OrderInfo
- [ ] T015 [P] Create unit tests `tests/TradingStrategiesPythonAPITests.c`
  - Test input conversion with minimal data
  - Test input conversion with full data
  - Test error cases (NULL pointers, invalid data)

### Phase 2.3: Output Conversion Implementation
- [ ] T016 Implement `convert_strategy_results()` function
  - Extract signals from StrategyResults
  - Extract UI values from StrategyResults
  - Allocate output arrays
  - Copy data to output structure
- [ ] T017 [P] Add output conversion tests
  - Test signal extraction
  - Test UI value extraction
  - Test memory allocation

### Phase 2.4: Main API Function
- [ ] T018 Implement `trading_strategies_run()` main function
  - Validate input parameters
  - Call convert_python_input()
  - Call existing runStrategy()
  - Call convert_strategy_results()
  - Handle errors
- [ ] T019 Implement `trading_strategies_free_output()` function
  - Free allocated arrays
  - Reset output structure
- [ ] T020 Implement `trading_strategies_get_error_message()` function
  - Map all AsirikuyReturnCode values
  - Return human-readable strings
- [ ] T021 [P] Add integration tests
  - Test full execution flow
  - Test error handling
  - Test memory management

### Phase 2.5: Memory Management
- [ ] T022 Review and fix memory management
  - Ensure all allocations are freed
  - Check for memory leaks (valgrind)
  - Document ownership rules
- [ ] T023 [P] Add memory leak tests
  - Test with valgrind
  - Test multiple calls
  - Test error paths

## Phase 3: Build System Updates

### Phase 3.1: Premake Configuration
- [x] T024 [P] Update `premake4.lua` for TradingStrategies
  - Change `kind` from `StaticLib` to `SharedLib`
  - Set `targetname` to `trading_strategies`
  - Add new source files
  - Configure library exports
  - Add dependencies (AsirikuyCommon, Log, OrderManager, etc.)
- [ ] T025 [P] Test Linux build
  - Build shared library (.so)
  - Verify exports
  - Test loading
- [ ] T026 [P] Test macOS build
  - Build shared library (.dylib)
  - Verify exports
  - Test loading
  - See BUILD_TEST.md for build instructions
- [ ] T027 [P] Test Windows build (optional)
  - Build shared library (.dll)
  - Verify exports
  - Test loading

### Phase 3.2: Build Documentation
- [ ] T028 Create build documentation
  - Document build requirements
  - Document build process
  - Document platform-specific notes

## Phase 4: Python Wrapper Development

### Phase 4.1: Basic Wrapper Structure
- [ ] T029 Create `python/trading_strategies.py` skeleton
- [ ] T030 Implement library loading
  - Cross-platform library path detection
  - Library loading with error handling
  - Version checking
- [ ] T031 Define C structure mappings (ctypes)
  - PythonStrategyInput Structure
  - PythonStrategyOutput Structure
  - Function signatures

### Phase 4.2: Data Conversion
- [ ] T032 Implement Python → C input conversion
  - Convert MarketData to C structure
  - Convert AccountInfo to C structure
  - Convert settings dict to array
  - Convert rates data to arrays
  - Convert orders list to arrays
- [ ] T033 Implement C → Python output conversion
  - Extract signals to Python list
  - Extract UI values to Python dict
  - Extract status message
- [ ] T034 Create Python data classes
  - TradingSignal dataclass
  - MarketData dataclass
  - AccountInfo dataclass

### Phase 4.3: Error Handling & Polish
- [ ] T035 Implement error handling
  - Map return codes to exceptions
  - Implement error message retrieval
  - Add exception classes
- [ ] T036 Add documentation
  - Add docstrings to all functions
  - Add type hints
  - Add usage examples in docstrings
- [ ] T037 Create helper functions
  - get_error_message()
  - get_version()
  - init/cleanup functions

### Phase 4.4: Example Code
- [ ] T038 Create `python/example_usage.py`
  - Basic usage example
  - Broker API integration example
  - Error handling example
- [ ] T039 Create `python/__init__.py`
  - Package initialization
  - Public API exports

## Phase 5: Testing & Validation

### Phase 5.1: C Unit Tests
- [ ] T040 [P] Complete C unit test suite
  - All input conversion functions
  - All output conversion functions
  - Error handling
  - Edge cases
- [ ] T041 [P] Run C unit tests
  - Verify all tests pass
  - Check coverage
  - Fix any failures

### Phase 5.2: Python Unit Tests
- [ ] T042 [P] Create `python/tests/test_trading_strategies.py`
  - Test library loading
  - Test input conversion
  - Test output conversion
  - Test error handling
  - Test with mock data
- [ ] T043 [P] Run Python unit tests
  - Verify all tests pass
  - Check coverage
  - Fix any failures

### Phase 5.3: Integration Tests
- [ ] T044 [P] Create integration tests
  - End-to-end test with real data
  - Test multiple strategy calls
  - Test error recovery
  - Test memory management
- [ ] T045 [P] Run integration tests
  - Verify all tests pass
  - Performance validation
  - Memory leak validation

### Phase 5.4: Performance & Memory Testing
- [ ] T046 [P] Performance benchmarking
  - Compare with baseline
  - Measure overhead
  - Profile hot paths
- [ ] T047 [P] Memory leak testing
  - Run valgrind on C code
  - Test Python wrapper memory
  - Verify no leaks
- [ ] T048 [P] Cross-platform testing
  - Test on Linux
  - Test on macOS
  - Test on Windows (if applicable)

## Phase 6: Documentation & Cleanup

### Phase 6.1: API Documentation
- [ ] T049 [P] Create C API documentation (Doxygen)
  - Document all functions
  - Document data structures
  - Add usage examples
- [ ] T050 [P] Create Python API documentation (Sphinx)
  - Document all classes
  - Document all functions
  - Add usage examples

### Phase 6.2: Usage Guides
- [ ] T051 [P] Create usage guide
  - Basic usage
  - Advanced usage
  - Common patterns
- [ ] T052 [P] Create integration guide
  - Broker API integration
  - Error handling patterns
  - Best practices
- [ ] T053 [P] Create migration guide
  - From DLL to Python
  - Common issues
  - Troubleshooting

### Phase 6.3: Code Cleanup
- [ ] T054 [P] Code cleanup
  - Format code
  - Remove unused code
  - Add comments
  - Review code style
- [ ] T055 [P] Final code review
  - Review all changes
  - Check for issues
  - Verify completeness

## Task Dependencies

```
T001 → T002 → T003 → T004 → T005 → T006
T007, T008, T009, T010, T011 (parallel setup)
T012 → T013 → T014 → T015
T016 → T017
T018 → T019 → T020 → T021
T022 → T023
T024 → T025, T026, T027 (parallel builds)
T029 → T030 → T031 → T032 → T033 → T034
T035 → T036 → T037
T038 → T039
T040 → T041
T042 → T043
T044 → T045
T046, T047, T048 (parallel testing)
T049, T050, T051, T052, T053 (parallel docs)
T054 → T055
```

## Parallel Execution Examples

### Example 1: Early Parallel Work
```
T007, T008, T009, T010, T011 can run in parallel
(Environment setup, code review, documentation)
```

### Example 2: Build System Parallel
```
T025, T026, T027 can run in parallel
(Linux, macOS, Windows builds)
```

### Example 3: Testing Parallel
```
T046, T047, T048 can run in parallel
(Performance, memory, cross-platform testing)
```

### Example 4: Documentation Parallel
```
T049, T050, T051, T052, T053 can run in parallel
(All documentation tasks)
```

## Critical Path

The critical path is:
```
T001 → T012 → T013 → T014 → T016 → T018 → T024 → T029 → T032 → T033 → T044 → T055
```

This represents the minimum sequence to get a working Python integration.

## Notes

- Tasks marked [P] can be done in parallel
- TDD approach: Write tests before implementation where possible
- All file paths are relative to `TradingStrategies/` directory
- Estimated total: ~55 tasks
- Estimated effort: 80-100 story points

