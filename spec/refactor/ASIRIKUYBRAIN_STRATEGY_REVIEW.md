# AsirikuyBrain Strategy Review

## Executive Summary

**Strategy ID**: 12 (ASIRIKUY_BRAIN)  
**File**: `dev/TradingStrategies/src/strategies/AsirikuyBrain.c` (922 lines)  
**Status**: ✅ **ACTIVE** - Registered in `AsirikuyStrategies.c`  
**Frontend Support**: ✅ MQL4 frontend available  
**Dependencies**: **CRITICAL** - FANN library, DevIL library, AsirikuyImageProcessorCWrapper

---

## Strategy Overview

**Type**: Neural Network Committee/Ensemble Trading Strategy  
**Timeframes**: Daily (primary), Hourly (for some components)  
**Concept**: A sophisticated neural network ensemble strategy that combines three different NN training systems (Paqarin, Sunqu, and Tapuy) in a committee approach. Requires all three NNs to agree before entering trades.

### Key Characteristics
- **Three NN Systems**: Paqarin, Sunqu, and Tapuy (can be enabled/disabled individually)
- **Committee Approach**: All three NNs must agree for trade entry
- **Real-Time Training**: Trains neural networks on-the-fly during execution
- **Image Processing**: Tapuy uses image-based neural network (converts charts to images)
- **Ensemble Predictions**: Combines predictions from multiple neural networks
- **Computationally Intensive**: Very resource-heavy due to real-time training

---

## Strategy Logic

### 1. Three Neural Network Systems

#### **Paqarin** (Price Action Neural Network)
- **Input**: Logarithmic returns of open prices over `INPUT_BARS_PAQARIN` bars
- **Architecture**: 3 layers (input, hidden = input*4, output = 4)
- **Output**: 4 outputs (down move, up move, stop loss hit down, stop loss hit up)
- **Training**: Uses RPROP algorithm, trains on historical data
- **Committee Size**: Configurable (`COMMITTEE_SIZE_PAQARIN`)
- **Prediction Logic**: 
  - If `output[0] > output[1]` AND `output[3] < 0.2` → Down prediction
  - If `output[1] > output[0]` AND `output[2] < 0.2` → Up prediction
  - Otherwise → Disagreement

#### **Sunqu** (Price Prediction Neural Network)
- **Input**: Close prices over `INPUT_BARS_SUNQU` bars
- **Architecture**: 3 layers (input, hidden = input, output = 1)
- **Output**: Single output (predicted price)
- **Training**: Uses RPROP algorithm, trains on historical data
- **Committee Size**: Configurable (`COMMITTEE_SIZE_SUNQU`)
- **Prediction Logic**: 
  - Compares two predictions (with current close vs. with current open)
  - If `firstOutput < secondOutput` → Down prediction
  - If `firstOutput > secondOutput` → Up prediction
  - Otherwise → Disagreement

#### **Tapuy** (Image-Based Neural Network)
- **Input**: Chart image converted to pixel array (`CANVAS_SIZE_TAPUY` x `CANVAS_SIZE_TAPUY`)
- **Architecture**: 3 layers (input = canvas², hidden = canvas²/5, output = 2)
- **Output**: 2 outputs (down move probability, up move probability)
- **Training**: Uses RPROP algorithm, trains on chart images
- **Image Processing**: 
  - Draws chart using `drawChartForNN_tester()`
  - Loads PNG image using DevIL library
  - Converts image pixels to neural network inputs
- **Prediction Logic**: 
  - If `output[0] > output[1]` → Down prediction
  - If `output[1] > output[0]` → Up prediction
  - Otherwise → Disagreement

### 2. Committee Prediction Logic

**Prediction Values**:
- `PREDICTION_NONE` (-1) = No prediction
- `PREDICTION_ALL_UP_MOVE` (0) = All NNs predict up
- `PREDICTION_ALL_DOWN_MOVE` (1) = All NNs predict down
- `PREDICTION_DISAGREEMENT` (2) = NNs disagree

**Entry Logic**:
- **Long Entry**: All three NNs predict `PREDICTION_ALL_UP_MOVE`
- **Short Entry**: All three NNs predict `PREDICTION_ALL_DOWN_MOVE`
- **No Entry**: Any disagreement or mixed signals

**Fallback Logic** (if one NN is disabled):
- If `USE_PAQARIN == FALSE`: Copies prediction from Sunqu or Tapuy
- If `USE_SUNQU == FALSE`: Copies prediction from Paqarin or Tapuy
- If `USE_TAPUY == FALSE`: Copies prediction from Sunqu or Paqarin

### 3. Trading Hour Filter

**Trading Hours**:
- If `TRADE_ON_HOUR == -1`: Trades at hour 0 or hour 4 on Monday
- Otherwise: Trades only at specified hour (`TRADE_ON_HOUR`)

### 4. Risk Management

**Stop Loss Calculation**:
```
stopLoss = atr * SL_ATR_MULTIPLIER
```

**Take Profit Calculation**:
```
takeProfit = atr * TP_ATR_MULTIPLIER
```

**Dynamic Stop Loss**:
- Uses `modifyTradeEasy()` to move stop loss closer to entry
- For SELL: Moves SL when `bid[0] + stopLoss < currentStopLoss`
- For BUY: Moves SL when `ask[0] - stopLoss > currentStopLoss`

### 5. Exit Logic

**Commented Out Exit Logic** (lines 885-909):
- There's commented-out code for closing trades on disagreement
- Currently, trades are only managed via stop loss/take profit
- No active exit logic based on NN predictions

---

## Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| `TRAINING_SETS_PER_EPOCH_PAQARIN` | int | Number of training samples per epoch for Paqarin |
| `TRAINING_EPOCHS_COUNT_PAQARIN` | int | Number of training epochs for Paqarin |
| `INPUT_BARS_PAQARIN` | int | Number of input bars for Paqarin |
| `COMMITTEE_SIZE_PAQARIN` | int | Number of neural networks in Paqarin committee |
| `TRAINING_SETS_PER_EPOCH_SUNQU` | int | Number of training samples per epoch for Sunqu |
| `TRAINING_EPOCHS_COUNT_SUNQU` | int | Number of training epochs for Sunqu |
| `INPUT_BARS_SUNQU` | int | Number of input bars for Sunqu |
| `COMMITTEE_SIZE_SUNQU` | int | Number of neural networks in Sunqu committee |
| `TRAINING_SETS_PER_EPOCH_TAPUY` | int | Number of training samples per epoch for Tapuy |
| `TRAINING_EPOCHS_COUNT_TAPUY` | int | Number of training epochs for Tapuy |
| `INPUT_BARS_TAPUY` | int | Number of input bars for Tapuy |
| `CANVAS_SIZE_TAPUY` | int | Canvas size for Tapuy image (width/height in pixels) |
| `SCALING_MODE_TAPUY` | int | Scaling mode for Tapuy (0=auto, 1=period-based) |
| `SAVE_HISTOGRAM_TAPUY` | BOOL | Save histogram images for Tapuy |
| `USE_HIGH_LOW_TAPUY` | BOOL | Use high/low prices in Tapuy chart |
| `USE_PAQARIN` | BOOL | Enable Paqarin NN |
| `USE_SUNQU` | BOOL | Enable Sunqu NN |
| `USE_TAPUY` | BOOL | Enable Tapuy NN |
| `TRADE_ON_HOUR` | int | Hour to trade (0-23, or -1 for default) |

---

## Code Quality Assessment

### ✅ Strengths
1. **Sophisticated Approach**: Advanced neural network ensemble strategy
2. **Flexible**: Can enable/disable individual NN systems
3. **Multiple Approaches**: Combines price-based and image-based NNs
4. **Committee System**: Reduces false signals through consensus
5. **Well-Structured**: Clear separation between training and prediction
6. **UI Integration**: Exposes predictions to user interface

### ⚠️ Critical Issues Found

1. **Incorrect Header Guard** (`AsirikuyBrain.h` lines 40-41):
   - Uses `PAQARIN_H_` instead of `ASIRIKUY_BRAIN_H_`
   - **Impact**: Potential header inclusion conflicts, copy-paste error

2. **Incorrect Header Documentation** (`AsirikuyBrain.h` lines 52-53):
   - Says "Runs the Paqarin trading strategy" instead of "AsirikuyBrain"
   - **Impact**: Confusing documentation, copy-paste error

3. **Duplicate fann_run() Call** (line 515-516):
   ```c
   fann_run(pNeuralNetwork, pInputs);  // Line 515 - unused result
   output = fann_run(pNeuralNetwork, pInputs);  // Line 516 - actual usage
   ```
   - **Impact**: Unnecessary computation, potential performance issue

4. **File I/O Error Handling**:
   - No error checking for `fopen()`, `fread()`, `fclose()` operations
   - No error checking for image loading operations
   - **Impact**: Potential crashes or undefined behavior

5. **Memory Management**:
   - Multiple `malloc()` calls without checking for NULL
   - Potential memory leaks if errors occur mid-function
   - **Impact**: Memory leaks, potential crashes

6. **NaN Check** (line 683-684):
   ```c
   if (output != output)  // NaN check
   output = 0;
   ```
   - Uses non-portable NaN check (should use `isnan()`)
   - **Impact**: May not work on all platforms

7. **Commented-Out Exit Logic** (lines 885-909):
   - Significant exit logic is commented out
   - **Impact**: Unclear if this is intentional or needs to be restored

8. **Real-Time Training Performance**:
   - Trains neural networks on every bar (if trading hour matches)
   - Very computationally expensive
   - **Impact**: High CPU usage, potential delays in execution

9. **Image File Management**:
   - Creates temporary PNG files but may not clean them up on errors
   - **Impact**: Disk space usage, potential file system issues

10. **Hardcoded Constants**:
    - Neural network hyperparameters are hardcoded (learning rate, momentum, etc.)
    - **Impact**: Not configurable, may need tuning for different markets

### 🔍 Code Complexity
- **Very High Complexity**: ~922 lines with complex NN training, image processing, and ensemble logic
- **Readability**: Moderate - Well-structured but dense with NN operations
- **Maintainability**: Low - Complex dependencies, real-time training, image processing
- **Dependencies**: Very High - Requires FANN, DevIL, image processor wrapper

### 📊 Performance Considerations
- **CPU Intensive**: Real-time neural network training is very expensive
- **Memory Intensive**: Multiple neural networks, image buffers, large arrays
- **I/O Intensive**: File operations for image generation and loading
- **Not Suitable for**: High-frequency trading, low-latency requirements

---

## Dependencies

### External Libraries (CRITICAL)
- **FANN (Fast Artificial Neural Network)**:
  - `doublefann.h` - Double precision FANN
  - `fann_train.h` - FANN training functions
  - Used for all three NN systems
- **DevIL (Developer's Image Library)**:
  - `IL/il.h` - Image loading and processing
  - Used for Tapuy image-based NN
- **AsirikuyImageProcessorCWrapper**:
  - `drawChartForNN_tester()` - Draws chart images
  - `drawHistogramForNN_tester()` - Draws histogram images
  - Used for Tapuy image generation

### Internal Dependencies
- `AsirikuyStrategies.h` - Strategy framework
- `AsirikuyTime.h` - Time utilities
- `OrderSignals.h` - Order signal management
- `OrderManagement.h` - Order operations
- `StrategyUserInterface.h` - UI value display
- `EasyTradeCWrapper.hpp` - Easy trade wrapper
- `ta_libc.h` - Technical Analysis Library (ATR calculation)

### System Dependencies
- `sys/stat.h` - File system operations
- File system access for temporary image files

### Related Strategies
- **KantuML** - Also uses ML but simpler (pre-trained models)
- **Munay** - Also uses ML but different approach

---

## Usage Status

### ✅ Active Usage
- **Registered**: Yes, in `AsirikuyStrategies.c` enum (ID: 12)
- **Case Handler**: Present in switch statement
- **Header File**: `AsirikuyBrain.h` exists and is included
- **Frontend Files**: 
  - `FrontEnds/MQL4_B600/MQL4/Include/Defines.mqh` (ASIRIKUY_BRAIN = 12)

### 📊 Integration Points
- Called via `runAsirikuyBrain()` function pointer in strategy dispatcher
- Uses standard `StrategyParams` structure
- Follows standard Asirikuy strategy pattern

---

## Comparison with Similar Strategies

### Similar Strategies in Codebase
1. **KantuML** - Machine learning strategy (simpler, uses pre-generated predictions)
2. **Munay** - Machine learning strategy (different ML approach)

### Unique Features of AsirikuyBrain
- **Real-Time Training**: Only strategy that trains NNs on-the-fly
- **Image-Based NN**: Only strategy using image processing for NN inputs
- **Committee Approach**: Only strategy requiring all NNs to agree
- **Three NN Systems**: Most complex ML strategy in codebase
- **FANN Library**: Only strategy using FANN library directly

---

## Refactoring Considerations

### For C++ Migration (Phase 2)
- **Complexity**: Extremely High - NN training, image processing, ensemble logic
- **Estimated Effort**: Very High (NN abstractions, image processing, performance optimization)
- **Class Structure**:
  ```cpp
  class AsirikuyBrainStrategy : public BaseStrategy {
    // NN trainer (strategy pattern for 3 NN types)
    // Image processor wrapper
    // Committee manager
    // Prediction aggregator
  };
  ```

### Potential Improvements
1. **Fix Header Issues**: Correct header guard and documentation
2. **Remove Duplicate fann_run()**: Remove line 515
3. **Add Error Handling**: File I/O, memory allocation, image loading
4. **Memory Management**: Use RAII, smart pointers, proper cleanup
5. **NaN Check**: Use `isnan()` or `std::isnan()`
6. **Exit Logic**: Decide on commented-out exit logic (restore or remove)
7. **Performance**: Consider caching trained models, batch training
8. **Configuration**: Make NN hyperparameters configurable
9. **File Cleanup**: Ensure temporary files are cleaned up
10. **Testing**: Add unit tests for NN training and prediction logic

### Refactoring Strategy
1. **Phase 1**: Fix header issues (quick win)
2. **Phase 2**: Add error handling for file I/O and memory operations
3. **Phase 3**: Remove duplicate fann_run() call
4. **Phase 4**: Fix NaN check to use standard library
5. **Phase 5**: Decide on exit logic (restore or remove)
6. **Phase 6**: Optimize performance (caching, batch training)
7. **Phase 7**: Make hyperparameters configurable
8. **Phase 8**: Add comprehensive error handling and logging
9. **Phase 9**: Optimize and add unit tests

---

## Recommendations

### ✅ **KEEP** - Recommended Reasons:
1. **Active Strategy**: Registered and available in frontends
2. **Unique Functionality**: Only real-time NN training strategy
3. **Sophisticated**: Advanced ensemble approach with image processing
4. **High Value**: Provides cutting-edge ML-based trading capabilities
5. **Well-Integrated**: Properly integrated with ML libraries

### ⚠️ **Critical Fixes Needed**:
1. **Critical**: Fix header guard in `AsirikuyBrain.h` (`PAQARIN_H_` → `ASIRIKUY_BRAIN_H_`)
2. **Critical**: Fix header documentation (says "Paqarin" instead of "AsirikuyBrain")
3. **Critical**: Add error handling for file I/O operations
4. **Critical**: Add error handling for memory allocation
5. **High Priority**: Remove duplicate `fann_run()` call (line 515)
6. **High Priority**: Fix NaN check to use standard library
7. **High Priority**: Add proper memory cleanup on errors
8. **Medium Priority**: Decide on commented-out exit logic
9. **Medium Priority**: Add file cleanup for temporary images
10. **Low Priority**: Make NN hyperparameters configurable
11. **Low Priority**: Consider performance optimizations (caching, batch training)

### 📝 **If Keeping**:
- Include in Phase 2 migration (very complex strategy, significant refactoring needed)
- Fix header issues immediately
- Add comprehensive error handling
- Consider performance optimizations for production use
- Add unit tests for NN components
- Document NN architecture and training process
- Consider separating NN training from prediction (pre-train models)

### 🗑️ **If Removing**:
- Would need to remove from `AsirikuyStrategies.c` enum and switch
- Remove `AsirikuyBrain.h` include
- Delete `AsirikuyBrain.c` and `AsirikuyBrain.h` files
- **Note**: FANN and DevIL libraries may be used elsewhere - verify before removing
- **Note**: `AsirikuyImageProcessorCWrapper` may be used elsewhere - verify before removing
- Remove MQL frontend files (optional, separate from core library)
- **Impact**: ~922 lines removed, ID 12 becomes available
- **Note**: This is a sophisticated, active ML strategy - removal should be carefully considered

---

## Conclusion

**AsirikuyBrain is a sophisticated, active neural network ensemble strategy with unique real-time training and image processing capabilities. It's well-integrated with ML libraries and provides significant value through advanced ML-based trading signals. However, it has critical issues including incorrect header documentation, missing error handling, performance concerns, and code quality issues that should be addressed before production use.**

**Recommendation**: ✅ **KEEP** with critical fixes and significant refactoring

**Priority Fixes**:
1. Fix header guard and documentation in `AsirikuyBrain.h` (critical - says "Paqarin")
2. Add error handling for file I/O and memory operations (critical - potential crashes)
3. Remove duplicate `fann_run()` call (high priority - performance)
4. Fix NaN check to use standard library (high priority - portability)
5. Add proper memory cleanup on errors (high priority - memory leaks)
6. Decide on commented-out exit logic (medium priority - unclear behavior)
7. Consider performance optimizations (medium priority - CPU intensive)

---

*Review Date: 2024*  
*Reviewed By: AI Assistant*  
*Strategy Version: F4.x.x (2014)*

