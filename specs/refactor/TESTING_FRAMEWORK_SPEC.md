# Testing Framework Specification

## Overview

This document specifies the testing framework and infrastructure for the C++ migration of TradingStrategies module.

**Framework**: Google Test (gtest) + Google Mock (gmock)  
**Test Types**: Unit tests, integration tests, validation tests  
**Coverage Target**: >90%

---

## Testing Framework Selection

### Google Test (Recommended)

**Pros**:
- Industry standard for C++ testing
- Rich assertion library
- Death tests for error cases
- Parameterized tests
- Good Visual Studio integration
- Active development

**Cons**:
- Requires C++11 (VS2010 has limited support)
- Needs workarounds for VS2010

**Verdict**: **Use Google Test 1.8.1** (last version with good VS2010 support)

### Alternative: Catch2

**Pros**:
- Header-only (easy integration)
- Modern C++ (but can work with C++03)

**Cons**:
- Less mature than gtest
- Fewer features

---

## Google Test Setup for VS2010

### Step 1: Download Google Test

**Version**: 1.8.1 (last version with VS2010 support)

**Location**: Place in `vendor/GoogleTest/`

```powershell
# Download and extract to vendor/GoogleTest/
# https://github.com/google/googletest/releases/tag/release-1.8.1
```

### Step 2: Build Google Test Library

**Build Script**: `vendor/GoogleTest/build_vs2010.bat`

```batch
@echo off
REM Build Google Test for Visual Studio 2010

cd googletest
mkdir build
cd build

cmake .. -G "Visual Studio 10 2010" ^
    -DBUILD_SHARED_LIBS=OFF ^
    -Dgtest_force_shared_crt=ON

msbuild gtest.sln /p:Configuration=Release /p:Platform=Win32
msbuild gtest.sln /p:Configuration=Debug /p:Platform=Win32

echo Google Test built successfully
echo Libraries: Release\gtest.lib, Debug\gtestd.lib
```

### Step 3: Update premake4.lua for Tests

```lua
-- Add test project
project "TradingStrategiesTests"
  location("../../build/" .. _ACTION .. "/projects")
  kind "ConsoleApp"
  language "C++"
  
  files{
    "../../../dev/TradingStrategies/tests/**.cpp",
    "../../../dev/TradingStrategies/tests/**.h"
  }
  
  includedirs{
    "../../../dev/TradingStrategies/include",
    "../../../dev/TradingStrategies/src",
    "../../../vendor/GoogleTest/googletest/include",
    "../../../vendor/GoogleTest/googlemock/include"
  }
  
  links{
    "TradingStrategies"  -- Link to main library
  }
  
  -- Link Google Test
  configuration "Debug"
    libdirs { "../../../vendor/GoogleTest/googletest/build/Debug" }
    links { "gtestd", "gtest_maind" }
    
  configuration "Release"
    libdirs { "../../../vendor/GoogleTest/googletest/build/Release" }
    links { "gtest", "gtest_main" }
```

---

## Test Directory Structure

```
TradingStrategies/
└── tests/
    ├── main.cpp                     # Test runner entry point
    ├── mocks/
    │   ├── MockStrategyParams.hpp   # Mock for StrategyParams
    │   └── MockIndicators.hpp       # Mock for Indicators
    ├── unit/
    │   ├── StrategyContextTest.cpp
    │   ├── StrategyFactoryTest.cpp
    │   ├── OrderBuilderTest.cpp
    │   ├── OrderManagerTest.cpp
    │   └── strategies/
    │       ├── RecordBarsStrategyTest.cpp
    │       ├── MACDDailyStrategyTest.cpp
    │       └── ...
    ├── integration/
    │   ├── CApiWrapperTest.cpp
    │   ├── StrategyExecutionTest.cpp
    │   └── OrderExecutionTest.cpp
    └── validation/
        ├── BacktestingValidationTest.cpp
        ├── PerformanceTest.cpp
        └── MemoryTest.cpp
```

---

## Test Implementation Examples

### Example 1: Unit Test for StrategyContext

`tests/unit/StrategyContextTest.cpp`:

```cpp
#include <gtest/gtest.h>
#include "StrategyContext.hpp"
#include "AsirikuyDefines.h"

// Test fixture
class StrategyContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize StrategyParams
        memset(&params, 0, sizeof(StrategyParams));
        params.settings[INTERNAL_STRATEGY_ID] = AUTOBBS;
        strcpy(params.tradeSymbol, "EURUSD");
        params.bidAsk.bid[0] = 1.1000;
        params.bidAsk.ask[0] = 1.1002;
        params.bidAsk.arraySize = 1;
    }
    
    StrategyParams params;
};

TEST_F(StrategyContextTest, Constructor_ValidParams_Success) {
    // Arrange & Act
    trading::StrategyContext ctx(&params);
    
    // Assert
    EXPECT_EQ(ctx.getStrategyId(), AUTOBBS);
    EXPECT_STREQ(ctx.getSymbol(), "EURUSD");
}

TEST_F(StrategyContextTest, GetBid_ValidIndex_ReturnsCorrectValue) {
    // Arrange
    trading::StrategyContext ctx(&params);
    
    // Act
    double bid = ctx.getBid(0);
    
    // Assert
    EXPECT_DOUBLE_EQ(bid, 1.1000);
}

TEST_F(StrategyContextTest, GetBid_InvalidIndex_ThrowsException) {
    // Arrange
    trading::StrategyContext ctx(&params);
    
    // Act & Assert
    EXPECT_THROW(ctx.getBid(10), std::out_of_range);
}

TEST_F(StrategyContextTest, Constructor_NullParams_ThrowsException) {
    // Act & Assert
    EXPECT_THROW(trading::StrategyContext ctx(NULL), std::invalid_argument);
}
```

### Example 2: Mock for Testing

`tests/mocks/MockIndicators.hpp`:

```cpp
#pragma once

#include <gmock/gmock.h>
#include "Indicators.hpp"

namespace testing {

class MockIndicators : public trading::Indicators {
public:
    MOCK_CONST_METHOD0(getDailyTrend, Trend());
    MOCK_CONST_METHOD0(getDailyATR, double());
    MOCK_CONST_METHOD0(getDailyPivot, double());
    MOCK_METHOD1(loadDailyIndicators, bool(const trading::StrategyContext&));
};

} // namespace testing
```

### Example 3: Integration Test

`tests/integration/CApiWrapperTest.cpp`:

```cpp
#include <gtest/gtest.h>
#include "AsirikuyStrategies.h"

class CApiWrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test params
        memset(&params, 0, sizeof(StrategyParams));
        // ... initialize params
    }
    
    StrategyParams params;
};

TEST_F(CApiWrapperTest, runStrategy_ValidParams_ReturnsSuccess) {
    // Arrange
    params.settings[INTERNAL_STRATEGY_ID] = RECORD_BARS;
    
    // Act
    AsirikuyReturnCode result = runStrategy(&params);
    
    // Assert
    EXPECT_EQ(result, SUCCESS);
}

TEST_F(CApiWrapperTest, runStrategy_NullParams_ReturnsError) {
    // Act
    AsirikuyReturnCode result = runStrategy(NULL);
    
    // Assert
    EXPECT_EQ(result, NULL_POINTER);
}
```

### Example 4: Parameterized Test

```cpp
class StrategyFactoryTest : public ::testing::TestWithParam<StrategyId> {};

TEST_P(StrategyFactoryTest, Create_ValidId_ReturnsNonNull) {
    // Arrange
    StrategyId id = GetParam();
    
    // Act
    auto strategy = trading::StrategyFactory::create(id);
    
    // Assert
    ASSERT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getId(), id);
}

INSTANTIATE_TEST_CASE_P(
    AllStrategies,
    StrategyFactoryTest,
    ::testing::Values(
        RECORD_BARS,
        TAKEOVER,
        SCREENING,
        AUTOBBS,
        AUTOBBSWEEKLY,
        TRENDLIMIT
    )
);
```

---

## Test Main Entry Point

`tests/main.cpp`:

```cpp
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Optional: Add custom test listener
    // Optional: Filter tests
    
    int result = RUN_ALL_TESTS();
    
    // Keep console open on Windows
    #ifdef _WIN32
    system("pause");
    #endif
    
    return result;
}
```

---

## Running Tests

### From Visual Studio

1. Build TradingStrategiesTests project
2. Right-click → Debug → Start New Instance
3. View output in console

### From Command Line

```powershell
# Build tests
msbuild build\vs2010\projects\TradingStrategiesTests.vcxproj /p:Configuration=Release

# Run tests
bin\vs2010\x32\Release\TradingStrategiesTests.exe

# Run with filter
bin\vs2010\x32\Release\TradingStrategiesTests.exe --gtest_filter=StrategyContext*

# Run with verbose output
bin\vs2010\x32\Release\TradingStrategiesTests.exe --gtest_print_time=1
```

---

## Test Coverage

### Using OpenCppCoverage (Windows)

```powershell
# Install OpenCppCoverage
choco install opencppcoverage

# Run tests with coverage
OpenCppCoverage.exe `
    --sources "dev\TradingStrategies\src" `
    --sources "dev\TradingStrategies\include" `
    --export_type html:coverage_report `
    -- bin\vs2010\x32\Debug\TradingStrategiesTests.exe
```

### Coverage Targets

- **Unit Tests**: >90% line coverage
- **Integration Tests**: All API entry points covered
- **Validation Tests**: All strategies validated

---

## Continuous Integration

### GitHub Actions Workflow (Future)

`.github/workflows/cpp-tests.yml`:

```yaml
name: C++ Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-2019
    steps:
      - uses: actions/checkout@v2
      
      - name: Setup MSBuild
        uses: microsoft/setup-msbuild@v1
      
      - name: Build
        run: |
          .\premake4.exe vs2010
          msbuild build\vs2010\AsirikuyFramework.sln /p:Configuration=Release
      
      - name: Run Tests
        run: |
          bin\vs2010\x32\Release\TradingStrategiesTests.exe --gtest_output=xml:test_results.xml
      
      - name: Publish Test Results
        uses: EnricoMi/publish-unit-test-result-action@v1
        with:
          files: test_results.xml
```

---

## Test Types

### 1. Unit Tests

**Purpose**: Test individual classes/functions in isolation

**Examples**:
- StrategyContext accessors
- StrategyFactory creation
- OrderBuilder order generation
- Indicators loading

**Characteristics**:
- Fast (< 1ms per test)
- No external dependencies
- Use mocks for dependencies

### 2. Integration Tests

**Purpose**: Test component interaction

**Examples**:
- C API → C++ flow
- Strategy execution flow
- Order execution flow
- Factory → Strategy creation

**Characteristics**:
- Moderate speed (< 100ms per test)
- Real dependencies where possible
- Test full workflows

### 3. Validation Tests

**Purpose**: Validate C++ matches C behavior

**Examples**:
- Side-by-side execution comparison
- Backtesting result comparison
- Order generation comparison

**Characteristics**:
- Slow (seconds per test)
- Use real historical data
- Compare outputs bit-by-bit

### 4. Performance Tests

**Purpose**: Measure and compare performance

**Examples**:
- Strategy execution time
- Memory usage
- Throughput benchmarks

**Characteristics**:
- Very slow (minutes per test)
- Run separately from unit tests
- Generate performance reports

---

## Best Practices

### Test Naming

```cpp
// Pattern: MethodName_Scenario_ExpectedBehavior
TEST(StrategyContext, GetBid_ValidIndex_ReturnsCorrectValue)
TEST(StrategyContext, GetBid_InvalidIndex_ThrowsException)
TEST(StrategyFactory, Create_InvalidId_ReturnsNull)
```

### Arrange-Act-Assert Pattern

```cpp
TEST(Example, Test) {
    // Arrange: Set up test data
    StrategyParams params;
    // ...
    
    // Act: Execute the code under test
    auto result = doSomething(&params);
    
    // Assert: Verify results
    EXPECT_EQ(result, expectedValue);
}
```

### Test Fixtures for Common Setup

```cpp
class MyTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup
    }
    
    void TearDown() override {
        // Common cleanup
    }
    
    // Common members
    StrategyParams params;
};
```

---

## Phase 1 Testing Tasks

- [ ] Set up Google Test framework
- [ ] Build Google Test library for VS2010
- [ ] Update premake4.lua with test project
- [ ] Create test directory structure
- [ ] Implement test main entry point
- [ ] Write first test (example)
- [ ] Verify test execution works
- [ ] Document how to run tests

---

## Next Steps

1. **Week 1, Day 5**: Set up Google Test framework
2. **Week 2**: Write tests as C++ classes are implemented
3. **Phase 2**: Write tests for each migrated strategy
4. **Phase 4**: Comprehensive testing and validation

---

**Document Version**: 1.0  
**Status**: Ready for Implementation  
**Last Updated**: November 9, 2025  
**Owner**: Phase 1 Implementation Team
