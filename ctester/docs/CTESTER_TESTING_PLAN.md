# CTester Testing & Documentation Plan

**Date**: December 2024  
**Tasks**: T034-T035

## Overview

This document outlines the plan for testing CTester and creating integration documentation.

## T034: Run Existing CTester Test Suite

### Current Test Infrastructure

CTester has a diagnostic test suite located in:
- **Test Runner**: `ctester/ast_diagnostics.py`
- **Test Configs**: `ctester/config/diagnostics/` (expected location)

### Test Cases

The diagnostic suite includes 8 test cases:

1. **Diagnostic 1**: Teyacanani standard test
2. **Diagnostic 2**: Sapaq USD/JPY standard test
3. **Diagnostic 3**: Quimichi multi-pair (EUR/USD, GBP/USD, USD/JPY) standard test
4. **Diagnostic 4**: Portfolio test (Sapaq USD/JPY + Watukushay FE BB EUR/USD + GG ATR GBP/USD)
5. **Diagnostic 5**: Watukushay FE BB optimization, brute force (single core)
6. **Diagnostic 6**: Watukushay FE BB optimization, brute force (dual core)
7. **Diagnostic 7**: Watukushay FE RSI optimization, genetics (single core)
8. **Diagnostic 8**: Watukushay FE RSI optimization, genetics (dual core)

### Issues to Address

1. **Python 2 → Python 3 Migration**: `ast_diagnostics.py` uses Python 2 syntax
   - Uses `#!/usr/bin/python` (should be `python3`)
   - Uses `python` command (should be `python3`)
   - May need other Python 3 compatibility fixes

2. **Test Config Files**: Verify diagnostic config files exist
   - Expected: `config/diagnostics/ast_diagnostic1.config` through `ast_diagnostic8.config`
   - May need to create missing configs

3. **Dependencies**: Ensure all required dependencies are available
   - Historical data files
   - Set files
   - Library files (libCTesterFrameworkAPI.so/dylib)

### Steps for T034

1. **Update Test Runner for Python 3**
   - Change shebang to `#!/usr/bin/env python3`
   - Update `python` commands to `python3`
   - Test syntax validation

2. **Verify Test Infrastructure**
   - Check if diagnostic config files exist
   - Verify historical data files are available
   - Check set files are present
   - Verify library can be loaded

3. **Run Test Suite**
   - Execute `ast_diagnostics.py`
   - Capture output and results
   - Document any failures

4. **Analyze Results**
   - Compare results with expected baseline
   - Identify any regressions
   - Document test execution time

5. **Fix Issues**
   - Fix any Python 3 compatibility issues
   - Fix any test failures
   - Update test configs if needed

6. **Documentation**
   - Document test execution process
   - Document test results
   - Document any known issues

## T035: Create CTester Integration Documentation

### Documentation Structure

1. **Integration Overview**
   - Architecture overview
   - Component relationships
   - Data flow

2. **Usage Guide**
   - Installation instructions
   - Configuration setup
   - Running tests
   - Running optimizations
   - Interpreting results

3. **Integration Process**
   - How CTester integrates with CTesterFrameworkAPI
   - Library loading
   - Function calls
   - Data structure mapping

4. **Troubleshooting**
   - Common issues
   - Error messages and solutions
   - Debugging tips
   - Performance optimization

5. **Code Examples**
   - Basic test execution
   - Portfolio testing
   - Optimization examples
   - Custom callback examples

### Documentation Files

1. **`CTESTER_USAGE_GUIDE.md`**
   - User-facing documentation
   - How to use CTester
   - Configuration examples
   - Command-line options

2. **`CTESTER_INTEGRATION_GUIDE.md`**
   - Developer-facing documentation
   - Integration architecture
   - API reference
   - Extension points

3. **`CTESTER_TROUBLESHOOTING.md`**
   - Common problems and solutions
   - Error reference
   - Debugging guide

### Steps for T035

1. **Create Usage Guide**
   - Installation instructions
   - Configuration guide
   - Running examples
   - Output interpretation

2. **Create Integration Guide**
   - Architecture documentation
   - API reference
   - Code examples
   - Extension guide

3. **Create Troubleshooting Guide**
   - Common issues
   - Error messages
   - Solutions
   - Debugging tips

4. **Add Code Examples**
   - Basic examples
   - Advanced examples
   - Best practices

5. **Review and Polish**
   - Review documentation
   - Fix typos
   - Improve clarity
   - Add diagrams if needed

## Testing Checklist

### Pre-Testing
- [ ] Update `ast_diagnostics.py` for Python 3
- [ ] Verify diagnostic config files exist
- [ ] Verify historical data files available
- [ ] Verify set files available
- [ ] Verify library can be loaded

### Test Execution
- [ ] Run diagnostic test 1
- [ ] Run diagnostic test 2
- [ ] Run diagnostic test 3
- [ ] Run diagnostic test 4
- [ ] Run diagnostic test 5
- [ ] Run diagnostic test 6
- [ ] Run diagnostic test 7
- [ ] Run diagnostic test 8

### Post-Testing
- [ ] Document test results
- [ ] Compare with baseline (if available)
- [ ] Document any failures
- [ ] Fix identified issues
- [ ] Re-run failed tests

## Documentation Checklist

- [ ] Usage guide created
- [ ] Integration guide created
- [ ] Troubleshooting guide created
- [ ] Code examples added
- [ ] Documentation reviewed
- [ ] Links and references verified

## Success Criteria

### T034
- ✅ All diagnostic tests run successfully
- ✅ No Python 3 compatibility issues
- ✅ Test results documented
- ✅ Any failures identified and fixed

### T035
- ✅ Complete usage guide
- ✅ Complete integration guide
- ✅ Troubleshooting guide with common issues
- ✅ Code examples for common use cases
- ✅ Documentation is clear and accurate

## Next Steps

1. Start with T034: Update and run test suite
2. Document test results
3. Create documentation (T035)
4. Review and finalize

