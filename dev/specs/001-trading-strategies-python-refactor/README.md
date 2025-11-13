# TradingStrategies Python Integration Refactor

## Quick Start

This specification describes the refactoring of the TradingStrategies C library to enable Python integration via broker REST APIs, removing Windows DLL dependencies.

## Specification Documents

- **[spec.md](./spec.md)** - Complete specification with requirements, architecture, and timeline
- **[plan.md](./plan.md)** - Detailed implementation plan with phases and milestones
- **[tasks.md](./tasks.md)** - Task breakdown with dependencies and parallel execution
- **[architecture.md](./architecture.md)** - Technical architecture and design details
- **[ARCHITECTURE_DECISION.md](./ARCHITECTURE_DECISION.md)** - Architecture decision: AsirikuyFrameworkAPI vs TradingStrategiesPythonAPI

## Overview

### Problem
The TradingStrategies C library is currently built as a Windows DLL for MQL4/MQL5 integration. We need to integrate it with Python trading platforms using broker REST APIs, which requires:
- Removing DLL dependencies
- Creating a Python-friendly API
- Maintaining existing strategy logic

### Solution
Create a new Python-friendly C API layer (`TradingStrategiesPythonAPI`) that wraps the existing strategy execution logic, enabling Python integration via ctypes while keeping all strategy code unchanged.

**Architecture Decision**: Use new `TradingStrategiesPythonAPI` instead of existing `AsirikuyFrameworkAPI` (CTester interface) for better cross-platform compatibility and Python optimization. See [ARCHITECTURE_DECISION.md](./ARCHITECTURE_DECISION.md) for detailed comparison.

### Key Features
- ✅ Standard C calling convention (no `__stdcall`)
- ✅ Python-friendly data structures
- ✅ Clean Python API using ctypes
- ✅ No changes to existing strategy logic
- ✅ Cross-platform support (Linux, macOS, Windows)

## Quick Reference

### Implementation Approach
**Option 1: Minimal Refactor** (Recommended)
- Create Python API wrapper layer
- Build as shared library
- Python ctypes integration
- **Effort**: 3-5 days
- **Risk**: Low

### Key Files
- `include/TradingStrategiesPythonAPI.h` - C API header
- `src/TradingStrategiesPythonAPI.c` - C API implementation
- `python/trading_strategies.py` - Python wrapper
- `python/example_usage.py` - Usage examples

### Timeline
- **Phase 1**: C API Implementation (8-10 days)
- **Phase 2**: Build System Updates (2-3 days)
- **Phase 3**: Python Wrapper (5-7 days)
- **Phase 4**: Testing & Validation (3-5 days)
- **Phase 5**: Documentation (2-3 days)

**Total**: 4-6 weeks

## Status

- ✅ Specification complete
- ✅ Implementation plan complete
- ✅ Task breakdown complete
- ✅ Architecture documented
- 📋 Ready for implementation

## Next Steps

1. Review specification documents
2. Set up development environment
3. Start Phase 1: C API Implementation
4. Follow task breakdown in `tasks.md`

## Questions?

Refer to:
- `spec.md` for detailed requirements
- `plan.md` for implementation timeline
- `architecture.md` for technical details
- `tasks.md` for task breakdown

## Future Enhancements

- **TrendStrategy.c Refactoring**: See `future_considerations/TrendStrategy-refactoring.md` for plan to refactor the monolithic 8,988-line file into maintainable strategy modules

