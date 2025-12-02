# Specification 001: TradingStrategies Integration & Refactoring

## 📋 Overview

**Project**: TradingStrategies C Library - Python Integration & Code Refactoring
**Spec ID**: 001
**Created**: November 13, 2024
**Status**: In Progress - Build System Complete
**Parent Spec**: N/A (Standalone refactoring initiative)
**Last Updated**: November 13, 2024

## 🎯 Objectives

### Primary Goals
1. **Integrate with existing CTester (Python 2 backtester)** - Connect AsirikuyFrameworkAPI with existing Python 2 CTester system
2. **Integrate with existing Live Trading Platform (Python 2)** - Connect AsirikuyFrameworkAPI with existing Python 2 live trading system
3. **Refactor TradingStrategies for maintainability** - Improve code organization, structure, and manageability (C++ or Python)
4. **Future: Python API Interface** - Consider adding cleaner Python API interface (deferred)

### Problem Statement
- Current implementation is built as Windows DLL for MQL4/MQL5 integration
- Existing Python 2 systems (CTester and Live Trading Platform) need to integrate with TradingStrategies
- TradingStrategies codebase has maintainability issues (monolithic files, complex dependencies)
- AsirikuyFrameworkAPI exists with CTester interface (`c_runStrategy`) that works from Python
- Need to modernize and refactor codebase while maintaining backward compatibility

### Recent Progress (November 2024)
- ✅ **macOS Build System**: Successfully built `AsirikuyFrameworkAPI` as shared library (`.dylib`) on macOS ARM64
- ✅ **Pantheios Removal**: Replaced all Pantheios logging with standard `fprintf(stderr, ...)` calls throughout codebase
- ✅ **Cross-Platform Support**: Library now builds on macOS (`.dylib`), with Linux (`.so`) and Windows (`.dll`) support planned
- ✅ **Dependencies Resolved**: Fixed MiniXML integration, Boost linking, and removed Windows-specific dependencies
- ✅ **Python Loading Verified**: Confirmed AsirikuyFrameworkAPI loads successfully from Python, `__stdcall` ignored on macOS
- ✅ **Build Documentation**: Created comprehensive `README_BUILD.md` with build instructions

### Success Criteria
1. **CTester Integration**: Existing Python 2 CTester successfully calls AsirikuyFrameworkAPI
2. **Live Trading Integration**: Existing Python 2 Live Trading Platform successfully calls AsirikuyFrameworkAPI
3. **Code Maintainability**: TradingStrategies refactored into manageable, well-organized modules
4. **Backward Compatibility**: Existing strategy logic unchanged, all tests pass
5. **Performance**: No significant performance degradation vs. current implementation
6. **Documentation**: Complete integration guides and API documentation

## 🏗️ Architecture

### Current Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                    Current Architecture                        │
├─────────────────────────────────────────────────────────────┤
│  MQL4/MQL5 Frontend        CTester (Python 2)  Live Trading │
│  ┌──────────────┐         ┌──────────────┐   ┌──────────┐ │
│  │ MQL API      │         │ CTester API  │   │ Live     │ │
│  │ (mql4/5_     │         │ (Python 2)   │   │ Trading  │ │
│  │  runStrategy)│         │              │   │ (Python 2)│ │
│  └──────┬───────┘         └──────┬───────┘   └─────┬─────┘ │
│         │                         │                  │       │
│         └─────────────────────────┼──────────────────┘       │
│                                   │                          │
│  ┌───────────────────────────────▼──────────────────────┐ │
│  │  AsirikuyFrameworkAPI (Shared Library)               │ │
│  │  • Windows: AsirikuyFrameworkAPI.dll                │ │
│  │  • macOS: libAsirikuyFrameworkAPI.dylib ✅          │ │
│  │  • Linux: libAsirikuyFrameworkAPI.so (planned)      │ │
│  │  • CTester Interface (c_runStrategy)                │ │
│  │  • __stdcall ignored on macOS/Linux                 │ │
│  └────────────────────┬─────────────────────────────────┘ │
│                       │                                    │
│  ┌────────────────────▼─────────────────────────────────┐ │
│  │  TradingStrategies (Shared Library)                  │ │
│  │  • Strategy execution logic                         │ │
│  │  • File-based I/O (UI, heartbeat, state)            │ │
│  │  • StrategyParams → StrategyResults                  │ │
│  │  • Component library (used by AsirikuyFrameworkAPI) │ │
│  │  • ⚠️ Needs refactoring for maintainability         │ │
│  └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Target Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                    Target Architecture                        │
├─────────────────────────────────────────────────────────────┤
│  MQL4/MQL5 Frontend        CTester (Python 2)  Live Trading │
│  ┌──────────────┐         ┌──────────────┐   ┌──────────┐ │
│  │ MQL API      │         │ CTester API  │   │ Live     │ │
│  │              │         │ (Python 2)   │   │ Trading  │ │
│  │              │         │ ✅ Integrated│   │ ✅ Integrated│
│  └──────┬───────┘         └──────┬───────┘   └─────┬─────┘ │
│         │                         │                  │       │
│         └─────────────────────────┼──────────────────┘       │
│                                   │                          │
│  ┌───────────────────────────────▼──────────────────────┐ │
│  │  AsirikuyFrameworkAPI (Shared Library)               │ │
│  │  • Cross-platform (Windows, macOS, Linux)          │ │
│  │  • CTester Interface (c_runStrategy)                │ │
│  │  • Python 2 compatible                              │ │
│  └────────────────────┬─────────────────────────────────┘ │
│                       │                                    │
│  ┌────────────────────▼─────────────────────────────────┐ │
│  │  TradingStrategies (Refactored)                       │ │
│  │  • Modular structure (strategy-based modules)        │ │
│  │  • Improved maintainability                           │ │
│  │  • Strategy execution logic (unchanged)              │ │
│  │  • Optional: C++ or Python refactoring              │ │
│  └─────────────────────────────────────────────────────┘ │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐ │
│  │  Future: Python API Interface (Optional)              │ │
│  │  • TradingStrategiesPythonAPI                         │ │
│  │  • Python-friendly data structures                    │ │
│  │  • Standard C calling convention                      │ │
│  └──────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Technology Stack
- **C Library**: Standard C (C99), no platform-specific code (Pantheios removed)
- **Python**: Python 3.8+ (migrated from Python 2.7+)
- **Future Python**: Python 3.8+ (for new Python API interface)
- **Build System**: premake4 (existing, generates Makefiles for gmake)
- **Logging**: Standard `fprintf(stderr, ...)` (replaced Pantheios)
- **XML Parsing**: MiniXML (vendor/MiniXML)
- **Refactoring Options**: C++ or Python (to be determined)
- **Testing**: CUnit (C), pytest (Python)
- **Documentation**: Doxygen (C), Sphinx (Python)

## 📊 Scope & Features

### Phase 1: Integration (Priority 1)

#### 1.1 CTester Integration (Python 3)
- **Objective**: Integrate existing Python 3 CTester with AsirikuyFrameworkAPI
- **Tasks**:
  - ✅ Review existing CTester Python 3 codebase (migrated from Python 2)
  - Create Python 3 wrapper for AsirikuyFrameworkAPI using ctypes
  - Map CTester data structures to CTesterDefines.h structures
  - Test integration with existing CTester backtesting workflows
  - Handle Python 3 string/bytes encoding
  - Document integration process

#### 1.2 Live Trading Platform Integration (Python 3)
- **Objective**: Integrate existing Python 3 Live Trading Platform with AsirikuyFrameworkAPI
- **Tasks**:
  - Review existing Live Trading Platform Python 3 codebase (migration pending)
  - Create Python 3 wrapper for AsirikuyFrameworkAPI using ctypes
  - Map broker REST API responses to CTester structures
  - Implement real-time strategy execution
  - Handle order management and position tracking
  - Test with live broker connections
  - Document integration process

### Phase 2: Refactoring (Priority 2)

#### 2.1 TradingStrategies Code Analysis
- **Objective**: Analyze current codebase structure and identify refactoring opportunities
- **Tasks**:
  - Code complexity analysis (cyclomatic complexity, file sizes)
  - Dependency analysis (identify coupling issues)
  - Identify monolithic files (e.g., TrendStrategy.c with 8,988 lines)
  - Document current architecture and pain points
  - Create refactoring plan

#### 2.2 Refactoring Strategy Decision
- **Options**:
  - **Option A: C++ Refactoring**
    - Convert C code to C++ for better OOP support
    - Use classes for strategy encapsulation
    - Leverage STL for data structures
    - Pros: Better performance, type safety, modern C++ features
    - Cons: Requires C++ compiler, more complex build system
  - **Option B: Python Refactoring**
    - Rewrite strategies in Python
    - Use C library for performance-critical parts
    - Pros: Easier maintenance, faster development, better testing
    - Cons: Performance overhead, requires Python runtime
  - **Option C: Hybrid Approach**
    - Keep core logic in C
    - Refactor into smaller, well-organized C modules
    - Add Python bindings for configuration and testing
    - Pros: Best of both worlds, minimal risk
    - Cons: More complex architecture

#### 2.3 Refactoring Implementation
- **Objective**: Refactor TradingStrategies into maintainable modules
- **Tasks**:
  - Split monolithic files into strategy-based modules
  - Extract common code to shared modules
  - Improve code organization and naming
  - Add comprehensive unit tests
  - Maintain backward compatibility
  - Update documentation

### Phase 3: Future Enhancements (Priority 3)

#### 3.1 Python API Interface (Future)
- **Objective**: Create cleaner Python API interface (deferred)
- **Tasks**:
  - Design Python-friendly API (TradingStrategiesPythonAPI)
  - Implement standard C calling convention (no __stdcall)
  - Create Python 3 wrapper with modern features
  - Add NumPy integration for rates data
  - Document API and provide examples

## 🔧 Technical Requirements

### Functional Requirements

#### FR-001: CTester Integration
- **Description**: Existing Python 2 CTester successfully calls AsirikuyFrameworkAPI
- **Input**: CTester test data (historical rates, settings, account info)
- **Output**: Strategy signals and test results
- **Error Handling**: Proper error propagation from C to Python
- **Compatibility**: Python 2.7+ with ctypes

#### FR-002: Live Trading Integration
- **Description**: Existing Python 2 Live Trading Platform successfully calls AsirikuyFrameworkAPI
- **Input**: Real-time market data from broker REST API
- **Output**: Trading signals and order management
- **Error Handling**: Robust error handling for network and API failures
- **Compatibility**: Python 2.7+ with ctypes

#### FR-003: Code Refactoring
- **Description**: TradingStrategies refactored into maintainable modules
- **Structure**: Strategy-based modules, shared utilities
- **Size**: No file > 2000 lines (target: < 1000 lines)
- **Complexity**: Reduced cyclomatic complexity
- **Testing**: Comprehensive unit test coverage

#### FR-004: Backward Compatibility
- **Description**: All existing functionality preserved
- **Validation**: All existing tests pass
- **API**: AsirikuyFrameworkAPI interface unchanged
- **Behavior**: Strategy results identical to current implementation

### Non-Functional Requirements

#### NFR-001: Performance
- **Target**: No more than 5% performance overhead vs. current implementation
- **Measurement**: Benchmark strategy execution time
- **Acceptance**: Strategy execution time < 1.05x baseline

#### NFR-002: Python 2 Compatibility
- **Requirement**: Full compatibility with Python 2.7+
- **Validation**: Test with existing CTester and Live Trading Platform
- **Acceptance**: All integration tests pass

#### NFR-003: Code Quality
- **Requirement**: Improved maintainability metrics
- **Metrics**: Reduced file sizes, complexity, coupling
- **Acceptance**: All files < 2000 lines, complexity < 20

#### NFR-004: Documentation
- **Requirement**: Complete integration and API documentation
- **Coverage**: Integration guides, API reference, examples
- **Acceptance**: All public APIs documented

## 📅 Timeline

### Phase 1: Integration (Weeks 1-3)

#### Week 1: CTester Integration
- **Duration**: 5-7 days
- **Deliverables**:
  - Python 2 wrapper for AsirikuyFrameworkAPI
  - CTester integration code
  - Integration tests
  - Documentation

#### Week 2: Live Trading Integration
- **Duration**: 5-7 days
- **Deliverables**:
  - Python 2 wrapper for Live Trading Platform
  - Broker API integration
  - Real-time execution tests
  - Documentation

#### Week 3: Integration Testing & Polish
- **Duration**: 3-5 days
- **Deliverables**:
  - Comprehensive integration tests
  - Performance validation
  - Bug fixes
  - Final documentation

### Phase 2: Refactoring (Weeks 4-8)

#### Week 4: Code Analysis
- **Duration**: 3-5 days
- **Deliverables**:
  - Code complexity analysis
  - Dependency analysis
  - Refactoring plan
  - Architecture documentation

#### Week 5: Refactoring Decision
- **Duration**: 2-3 days
- **Deliverables**:
  - Refactoring approach decision (C++/Python/Hybrid)
  - Detailed implementation plan
  - Risk assessment

#### Weeks 6-8: Refactoring Implementation
- **Duration**: 15-20 days
- **Deliverables**:
  - Refactored code modules
  - Unit tests
  - Integration tests
  - Documentation updates

### Phase 3: Future Enhancements (Future)

#### Python API Interface (Deferred)
- **Duration**: 2-3 weeks
- **Status**: Future consideration
- **Deliverables**:
  - TradingStrategiesPythonAPI implementation
  - Python 3 wrapper
  - Documentation

**Total Estimated Duration**: 8-12 weeks (Phases 1-2)

## ⚠️ Risk Assessment

### Technical Risks

#### Risk-001: Python 2 Compatibility
- **Severity**: Medium
- **Description**: Python 2 is end-of-life, may have compatibility issues
- **Mitigation**: Test thoroughly, document known issues, plan Python 3 migration
- **Contingency**: Consider Python 3 migration path

#### Risk-002: Integration Complexity
- **Severity**: Medium
- **Description**: Existing Python 2 code may have complex dependencies
- **Mitigation**: Thorough code review, incremental integration, comprehensive testing
- **Contingency**: Create adapter layer if needed

#### Risk-003: Refactoring Scope
- **Severity**: High
- **Description**: Refactoring may uncover hidden dependencies
- **Mitigation**: Comprehensive analysis, incremental refactoring, extensive testing
- **Contingency**: Phased approach, maintain compatibility layer

#### Risk-004: Performance Impact
- **Severity**: Low
- **Description**: Refactoring may introduce performance overhead
- **Mitigation**: Benchmark before/after, profile hot paths
- **Contingency**: Optimize critical paths if needed

### Project Risks

#### Risk-005: Python 2 End-of-Life
- **Severity**: ✅ Resolved
- **Description**: Python 2 is no longer supported
- **Mitigation**: ✅ Completed Python 3 migration for CTester
- **Status**: CTester migrated to Python 3.8+, Live Trading Platform migration pending

#### Risk-006: Existing Code Dependencies
- **Severity**: Medium
- **Description**: Existing Python 2 systems may have unknown dependencies
- **Mitigation**: Comprehensive code review, dependency analysis
- **Contingency**: Create compatibility layer

## 📝 Dependencies

### Internal Dependencies
- **AsirikuyFrameworkAPI**: Required for Python integration
- **TradingStrategies**: Core strategy logic (to be refactored)
- **AsirikuyCommon**: Required for data structures and utilities
- **AsirikuyTechnicalAnalysis**: Required for indicator calculations

### External Dependencies
- **Python 3.8+**: For CTester (migrated) and Live Trading Platform (migration pending)
- **premake4**: For build system (Docker wrapper available)
- **C Compiler**: GCC/Clang (Linux/macOS), MSVC (Windows)
- **Boost**: C++ libraries (if C++ refactoring chosen)
- **cURL**: HTTP client library
- **MiniXML**: XML parsing library

### Existing Systems
- **CTester (Python 2)**: Existing backtesting system (to be provided)
- **Live Trading Platform (Python 2)**: Existing live trading system (to be provided)

## ✅ Validation Criteria

### Integration Validation
- [ ] CTester successfully calls AsirikuyFrameworkAPI
- [ ] Live Trading Platform successfully calls AsirikuyFrameworkAPI
- [ ] All existing CTester tests pass
- [ ] All existing Live Trading Platform tests pass
- [ ] Performance meets requirements (< 5% overhead)

### Refactoring Validation
- [ ] All files < 2000 lines (target: < 1000 lines)
- [ ] Cyclomatic complexity reduced
- [ ] All unit tests pass
- [ ] All integration tests pass
- [ ] Strategy results identical to baseline
- [ ] Code coverage > 80%

### Quality Validation
- [ ] Code passes linting/formatting checks
- [ ] Documentation is complete
- [ ] Integration guides are complete
- [ ] Examples work as documented

## 📚 References

### Internal Documents
- `README_BUILD.md` - Build instructions for AsirikuyFrameworkAPI on macOS
- `INSTALL_PREMAKE4.md` - Premake4 installation guide
- `test_asirikuy_framework_api.py` - Python loading test script
- `AsirikuyCommon/include/AsirikuyDefines.h` - Data structure definitions
- `TradingStrategies/include/AsirikuyStrategies.h` - Strategy API

### External References
- Python 2 ctypes documentation
- Python 2 to Python 3 migration guide
- C++ refactoring best practices
- Code complexity metrics

## 🔄 Migration Path

### Phase 1: Integration (Immediate)
1. ✅ **Build System**: Shared library builds working on macOS
2. ✅ **Dependencies**: Pantheios removed, MiniXML integrated, Boost linking fixed
3. 🔄 **CTester Integration**: Integrate with existing Python 2 CTester
4. 🔄 **Live Trading Integration**: Integrate with existing Python 2 Live Trading Platform

### Phase 2: Refactoring (Short-term)
1. Analyze current codebase structure
2. Decide refactoring approach (C++/Python/Hybrid)
3. Implement refactoring incrementally
4. Maintain backward compatibility throughout

### Phase 3: Future Enhancements (Long-term)
1. Consider Python API interface (TradingStrategiesPythonAPI)
2. Plan Python 3 migration
3. Add modern Python features (async, type hints, etc.)

### Current Status Summary (November 13, 2024)
- ✅ **macOS Build**: Successfully built `libAsirikuyFrameworkAPI.dylib` (371KB) on macOS ARM64
- ✅ **Dependencies**: All Pantheios dependencies removed, replaced with standard `fprintf(stderr, ...)`
- ✅ **MiniXML**: Integrated and building correctly
- ✅ **Boost**: Linking fixed, using only required libraries
- ✅ **Python Loading**: Verified AsirikuyFrameworkAPI loads from Python, `__stdcall` works on macOS
- ✅ **Documentation**: Comprehensive build guide created
- 🔄 **Next Steps**: Integrate with existing Python 2 CTester and Live Trading Platform
