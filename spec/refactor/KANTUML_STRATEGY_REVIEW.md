# KantuML Strategy Review

## Executive Summary

**Strategy ID**: 22 (KANTU_ML)  
**File**: `dev/TradingStrategies/src/strategies/KantuML.c` (414 lines)  
**Status**: ✅ **ACTIVE** - Registered in `AsirikuyStrategies.c`  
**Frontend Support**: ✅ MQL4 frontend available  
**Dependencies**: `KantuStrategies.c` (shared library), `AsirikuyMachineLearningCWrapper.hpp` (ML algorithms)

---

## Strategy Overview

**Type**: Machine Learning Based Trading Strategy  
**Timeframes**: Any (supports daily and intraday)  
**Concept**: A machine learning strategy that uses ensemble predictions from multiple ML algorithms (Linear Regression, Neural Network, K-Nearest Neighbor) to generate trading signals. Can generate prediction files or use pre-generated binary files.

### Key Characteristics
- **Machine Learning Algorithms**: Supports 3 ML algorithms (Linear Regression, Neural Network, K-Nearest Neighbor)
- **Ensemble Predictions**: Uses pre-generated binary files with ensemble predictions
- **Binary File Generation**: Can generate prediction files for backtesting/optimization
- **Dynamic Stop Loss (DSL)**: 7 different DSL types (same as Kantu)
- **Trailing Stop**: Supports trailing stop loss functionality
- **AFMTL**: Supports "Always Follow Moving Trailing Loss" mode
- **Trading Hour Filter**: Can restrict trading to specific hours

---

## Strategy Logic

### 1. Prediction Source

**Two Modes**:

#### Mode 1: Use Pre-Generated Binary Files
- If `SELECT_KANTU_SYSTEM_FILE_ML != 0`, loads predictions from binary file
- Calls `getKantuSignalML()` to read ensemble predictions
- Binary files contain pre-computed predictions from ML algorithms

#### Mode 2: Generate Predictions On-The-Fly
- If `SELECT_KANTU_SYSTEM_FILE_ML == 0`, generates predictions in real-time
- Uses one of three ML algorithms based on `ML_ALGO_TYPE_ML`:
  - **LINEAR_REGRESSION** (0): `LR_Prediction_i_simpleReturn_o_mlemse()`
  - **NEURAL_NETWORK** (1): `NN_Prediction_i_simpleReturn_o_mlemse()`
  - **K_NEAREST_NEIGHBOR** (2): `KNN_Prediction_i_simpleReturn_o_mlemse()`
- Saves predictions to binary file for later use
- File naming: `{SYMBOL}{LEARNING_PERIOD}{BARS_USED}{FRONTIER}{ALGO_TYPE}.bin`

### 2. Ensemble Prediction Values

**Prediction Values**:
- `0` = No signal (hold/exit)
- `1` = Long signal (buy)
- `2` = Short signal (sell)

**Ensemble Logic** (from `getKantuSignalML()`):
- Combines predictions from multiple ML algorithms
- Returns `1` if all algorithms predict long
- Returns `2` if all algorithms predict short
- Returns `0` otherwise (mixed signals)

### 3. Entry Logic

**Entry Signals**:
- **Long Entry**: `ensemblePrediction == 1` → Opens/updates long trade
- **Short Entry**: `ensemblePrediction == 2` → Opens/updates short trade
- **No Entry**: `ensemblePrediction == 0` → No new trades

**Trading Hour Filter**:
- Can restrict trading to specific hours via `TRADING_HOUR_ML`
- If `TRADING_HOUR_ML == -1`, trades at any hour
- Otherwise, only trades when `hour() == TRADING_HOUR_ML`

### 4. Dynamic Stop Loss (DSL)

**7 DSL Types** (same as Kantu, via `DSL_TYPE_ML`):

| Type | Value | Formula | Description |
|------|-------|---------|-------------|
| **DSL_NONE** | 0 | N/A | No dynamic stop loss |
| **DSL_LINEAR** | 1 | `newSL = orderAge*(-stopLoss/BE_BARS)+stopLoss` | Linear progression to breakeven |
| **DSL_LOG** | 2 | `newSL = stopLoss+(stopLoss/(-log(BE_BARS)))*log(orderAge+1)` | Logarithmic progression |
| **DSL_PARABOLIC** | 3 | `newSL = stopLoss+(-stopLoss/(BE_BARS²))*orderAge²` | Parabolic progression |
| **DSL_SQUARE** | 4 | `newSL = stopLoss+(-stopLoss/√BE_BARS)*√orderAge` | Square root progression |
| **DSL_SQUARE_WITH_SKIP** | 5 | Square root + skip logic based on bar direction | Square root with bar filtering |
| **DSL_X4** | 6 | `newSL = stopLoss+(-stopLoss/BE_BARS⁴)*orderAge⁴` | Power of 4 progression |
| **DSL_X025** | 7 | `newSL = stopLoss+(-stopLoss/BE_BARS^0.25)*orderAge^0.25` | Power of 0.25 progression |

**DSL Implementation**:
- Inline DSL formulas (not using `setKantuDSL()` like regular Kantu)
- More complex calculations with spread adjustments
- Uses `cOpen(orderAge)` for entry price reference

### 5. Trailing Stop Loss

**Trailing Stop Logic**:
- Uses `TL_ATR_MULTIPLIER_ML` to calculate trailing distance
- For SELL orders: Moves stop loss when `cOpen(orderAge) - ask[0] > trailingStop`
- For BUY orders: Moves stop loss when `bid[0] - cOpen(orderAge) > trailingStop`
- Preserves take profit when modifying

### 6. AFMTL (Always Follow Moving Trailing Loss)

**AFMTL Mode** (via `USE_AFMTL_ML`):
- When enabled, continuously moves stop loss closer to entry price
- For SELL: Moves SL when `ask[0] + stopLoss < currentStopLoss`
- For BUY: Moves SL when `bid[0] - stopLoss > currentStopLoss`
- Uses original stop loss value as reference

### 7. Risk Management

**Stop Loss Calculation**:
```
stopLoss = atr * SL_ATR_MULTIPLIER
```

**Take Profit Calculation**:
```
takeProfit = atr * TP_ATR_MULTIPLIER
```

**Minimum Stop Loss**:
- JPY pairs: 0.15 pips
- Other pairs: 0.0015 (15 pips)

**ATR Calculation**:
- Uses `iAtrWholeDaysSimple()` for ATR calculation
- Always uses whole days calculation (different from Kantu)

---

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `SELECT_KANTU_SYSTEM_FILE_ML` | int | Load pre-generated binary prediction file (0 = generate on-the-fly) |
| `LEARNING_PERIOD_ML` | int | Number of bars for ML learning period |
| `BARS_USED_ML` | int | Number of bars used as input features |
| `TRADING_HOUR_ML` | int | Hour of day to trade (0-23, or -1 for any hour) |
| `DSL_BREAKEVEN_BARS_ML` | int | Number of bars to reach breakeven for DSL |
| `FRONTIER_ML` | int | Frontier parameter for ML algorithms |
| `DSL_TYPE_ML` | int | Dynamic Stop Loss type (0-7) |
| `ML_ALGO_TYPE_ML` | int | ML algorithm type (0=LR, 1=NN, 2=KNN) |
| `TL_ATR_MULTIPLIER_ML` | double | Trailing stop loss as ATR multiple |
| `USE_AFMTL_ML` | BOOL | Enable Always Follow Moving Trailing Loss |
| `TIMED_EXIT_BARS_ML` | int | Timed exit bars (not used in current implementation) |
| `DAY_FILTER_ML` | int | Day filter (not used in current implementation) |

---

## Code Quality Assessment

### ✅ Strengths
1. **Machine Learning Integration**: Unique ML-based trading approach
2. **Binary File Support**: Efficient prediction storage/loading
3. **Ensemble Predictions**: Combines multiple ML algorithms
4. **Flexible**: Can generate or load predictions
5. **Advanced DSL**: Sophisticated dynamic stop loss system
6. **UI Integration**: Exposes key values to user interface

### ⚠️ Issues Found

1. **Incorrect Header Documentation** (`KantuML.c` lines 3-4):
   - Says "The Munay trading system" instead of "KantuML"
   - Says "Munay is a machine learning prototype strategy" instead of describing KantuML
   - **Impact**: Confusing documentation, copy-paste error

2. **Incorrect Header File Documentation** (`KantuML.h` lines 3-4):
   - Says "The Teyacanani trading system" instead of "KantuML"
   - Says "Teyacanani uses a 1 hour timeframe" instead of describing KantuML
   - **Impact**: Very confusing documentation, multiple copy-paste errors

3. **Unused Parameters**:
   - `TIMED_EXIT_BARS_ML` - Defined but never used
   - `DAY_FILTER_ML` - Defined but never used
   - **Impact**: Dead code, potential confusion

4. **Complex DSL Logic**:
   - 7 different DSL formulas implemented inline (not using `setKantuDSL()`)
   - More complex than Kantu due to spread adjustments
   - Could benefit from refactoring into separate functions

5. **File I/O Issues**:
   - Infinite loop potential: `while (f == NULL)` and `while(is_file_closed != 0)`
   - No error handling for file operations
   - **Impact**: Potential hangs or crashes if file operations fail

6. **Magic Numbers**:
   - Hardcoded values: 0.15 (JPY min stop), 0.0015 (other pairs min stop)
   - Could be constants or parameters

7. **Code Duplication**:
   - DSL logic is duplicated from Kantu (but with inline implementation)
   - Trailing stop and AFMTL logic duplicated from Kantu
   - **Opportunity**: Could share code with Kantu

8. **Missing Error Handling**:
   - No error checking for ML prediction functions
   - No error handling for file operations
   - **Impact**: Potential crashes or incorrect behavior

### 🔍 Code Complexity
- **High Complexity**: ~414 lines with complex ML integration and DSL logic
- **Readability**: Moderate - ML logic is clear but DSL formulas are dense
- **Maintainability**: Moderate - Code duplication with Kantu, complex DSL logic
- **Dependencies**: High - depends on `KantuStrategies.c` and ML wrapper

---

## Dependencies

### Internal Dependencies
- `KantuStrategies.h` / `KantuStrategies.c` - **CRITICAL DEPENDENCY**
  - `getKantuSignalML()` - Loads ensemble predictions from binary files
- `AsirikuyMachineLearningCWrapper.hpp` - **CRITICAL DEPENDENCY**
  - `LR_Prediction_i_simpleReturn_o_mlemse()` - Linear Regression predictions
  - `NN_Prediction_i_simpleReturn_o_mlemse()` - Neural Network predictions
  - `KNN_Prediction_i_simpleReturn_o_mlemse()` - K-Nearest Neighbor predictions
- `AsirikuyStrategies.h` - Strategy framework
- `AsirikuyTime.h` - Time utilities
- `OrderSignals.h` - Order signal management
- `OrderManagement.h` - Order operations
- `StrategyUserInterface.h` - UI value display
- `EasyTradeCWrapper.hpp` - Easy trade wrapper

### External Dependencies
- `ta_libc.h` - Technical Analysis Library (not directly used)
- `stdlib.h` - Standard library functions (file I/O)
- `math.h` - Mathematical functions (log, sqrt, pow)

### Related Strategies
- **Kantu** (ID: 18) - Regular price action version (shares `KantuStrategies.c`)
- Both strategies share DSL logic concepts but implement differently

---

## Usage Status

### ✅ Active Usage
- **Registered**: Yes, in `AsirikuyStrategies.c` enum (ID: 22)
- **Case Handler**: Present in switch statement
- **Header File**: `KantuML.h` exists and is included
- **Frontend Files**: 
  - `FrontEnds/MQL4_B600/MQL4/Include/Defines.mqh` (KANTUML = 22)

### 📊 Integration Points
- Called via `runKantuML()` function pointer in strategy dispatcher
- Uses standard `StrategyParams` structure
- Follows standard Asirikuy strategy pattern

---

## Comparison with Similar Strategies

### Similar Strategies in Codebase
1. **Kantu** - Regular price action version (shares library, similar DSL logic)
2. **AsirikuyBrain** - Neural network-based strategy
3. **Coatl** - Genetic algorithm-based strategy

### Unique Features of KantuML
- **Machine Learning Algorithms**: Only strategy using Linear Regression, Neural Network, and KNN
- **Binary Prediction Files**: Only strategy that generates/loads binary prediction files
- **Ensemble Predictions**: Combines multiple ML algorithm predictions
- **Real-Time ML Generation**: Can generate ML predictions on-the-fly
- **ML Wrapper Integration**: Uses `AsirikuyMachineLearningCWrapper.hpp`

---

## Refactoring Considerations

### For C++ Migration (Phase 2)
- **Complexity**: Very High - ML integration, file I/O, complex DSL logic
- **Estimated Effort**: Very High (ML dependencies, file I/O, DSL refactoring)
- **Class Structure**:
  ```cpp
  class KantuMLStrategy : public BaseStrategy {
    // ML predictor (strategy pattern for 3 algorithms)
    // Binary file manager
    // DSL manager (shared with Kantu)
    // Trailing stop manager (shared with Kantu)
  };
  ```

### Potential Improvements
1. **Fix Header Documentation**: Correct "Munay" and "Teyacanani" references to "KantuML"
2. **Extract DSL Logic**: Share DSL implementation with Kantu using `setKantuDSL()`
3. **File I/O Error Handling**: Add proper error handling for file operations
4. **Remove Infinite Loops**: Fix `while (f == NULL)` and `while(is_file_closed != 0)`
5. **Remove Unused Parameters**: Remove or implement `TIMED_EXIT_BARS_ML` and `DAY_FILTER_ML`
6. **Constants**: Extract magic numbers to named constants
7. **ML Error Handling**: Add error checking for ML prediction functions
8. **Code Sharing**: Extract common DSL/trailing stop logic shared with Kantu

### Refactoring Strategy
1. **Phase 1**: Fix header documentation (quick win)
2. **Phase 2**: Fix file I/O infinite loops and add error handling
3. **Phase 3**: Extract DSL logic to use `setKantuDSL()` (share with Kantu)
4. **Phase 4**: Extract trailing stop and AFMTL to shared module
5. **Phase 5**: Add ML error handling and validation
6. **Phase 6**: Optimize and add unit tests

---

## Recommendations

### ✅ **KEEP** - Recommended Reasons:
1. **Active Strategy**: Registered and available in frontends
2. **Unique Functionality**: Only ML-based strategy with ensemble predictions
3. **Sophisticated**: Advanced ML integration with multiple algorithms
4. **Well-Integrated**: Properly integrated with ML wrapper library
5. **High Value**: Provides ML-based trading capabilities

### ⚠️ **Fixes Needed**:
1. **Critical**: Fix header documentation in `KantuML.c` and `KantuML.h` (says "Munay" and "Teyacanani")
2. **Critical**: Fix infinite loops in file I/O operations
3. **High Priority**: Add error handling for file operations and ML predictions
4. **High Priority**: Extract DSL logic to use `setKantuDSL()` (share with Kantu)
5. **Medium Priority**: Remove or implement unused parameters
6. **Low Priority**: Extract magic numbers to constants
7. **Low Priority**: Extract common code shared with Kantu

### 📝 **If Keeping**:
- Include in Phase 2 migration (complex strategy, good refactoring candidate)
- Fix header documentation immediately
- Fix file I/O issues immediately (potential crashes)
- Plan for significant refactoring to improve maintainability
- Consider sharing DSL/trailing stop code with Kantu
- Add comprehensive error handling
- Add unit tests for ML integration

### 🗑️ **If Removing**:
- Would need to remove from `AsirikuyStrategies.c` enum and switch
- Remove `KantuML.h` include
- Delete `KantuML.c` and `KantuML.h` files
- **Note**: `KantuStrategies.c` is shared with `Kantu` - do not delete
- **Note**: `AsirikuyMachineLearningCWrapper.hpp` may be used elsewhere
- Remove MQL frontend files (optional, separate from core library)
- **Impact**: ~414 lines removed, ID 22 becomes available
- **Note**: This is a sophisticated, active ML strategy - removal should be carefully considered

---

## Conclusion

**KantuML is a sophisticated, active machine learning strategy with unique ensemble prediction capabilities. It's well-integrated with the ML wrapper library and provides significant value through ML-based trading signals. However, it has critical issues including incorrect documentation, potential infinite loops in file I/O, and code duplication with Kantu that should be addressed.**

**Recommendation**: ✅ **KEEP** with critical fixes and refactoring

**Priority Fixes**:
1. Fix header documentation in `KantuML.c` and `KantuML.h` (critical - says "Munay" and "Teyacanani")
2. Fix infinite loops in file I/O operations (critical - potential crashes)
3. Add error handling for file operations and ML predictions
4. Extract DSL logic to share with Kantu
5. Remove or implement unused parameters

---

*Review Date: 2024*  
*Reviewed By: AI Assistant*  
*Strategy Version: F4.x.x (2015)*

