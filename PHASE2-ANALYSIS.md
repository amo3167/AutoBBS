# Phase 2: API Issues Analysis - TrendStrategy.c

**Generated**: November 8, 2025  
**Branch**: refactor  
**Total Warnings**: 30 (TrendStrategy.c only)

---

## Executive Summary

Phase 2 warnings indicate **API design inconsistencies** between function declarations and implementations. These are more serious than Phase 1 (unused variables) as they affect runtime behavior:

- **Function signature mismatches** (C4028/C4029): Prototypes don't match implementations
- **Parameter count mismatches** (C4020): Functions called with wrong number of arguments  
- **Type incompatibilities** (C4133/C4047/C4024): Pointer types don't match
- **Implicit conversions** (C4244): double→int data loss without explicit casts

**Risk Assessment**: 
- ⚠️ **MEDIUM-HIGH**: These warnings can cause undefined behavior, data corruption, or crashes
- All fixes must be tested carefully to avoid breaking trading logic
- Some may require changing function signatures in header files

---

## Warning Categories Breakdown

### 1. C4028 + C4029: Function Signature Mismatch (4 warnings, 2 functions)

**Lines**: 937, 1027  
**Functions**: `splitBuyOrders_Limit`, `splitSellOrders_Limit`

#### Problem:
```c
// Header declaration (TrendStrategy.h line 49-50):
void splitBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, 
                          Base_Indicators* pBase_Indicators, 
                          double takePrice_primary, double stopLoss);

// Actual implementation (TrendStrategy.c line 937):
void splitBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, 
                          Base_Indicators* pBase_Indicators, 
                          int mode,                          // ← EXTRA PARAMETER
                          double takePrice_primary, double stopLoss)
```

#### Analysis:
- Implementation has **4th parameter `int mode`** not in declaration
- Formal parameter 4 (`takePrice_primary`) is at different position than declared
- This is parameter position mismatch #4

#### Root Cause:
Declaration and implementation got out of sync during development. The `mode` parameter was added to implementation but header wasn't updated.

#### Fix Strategy:
**Option A** (Safer): Remove `int mode` from implementation if unused  
**Option B**: Add `int mode` to header declaration if needed elsewhere

#### Impact Analysis:
Need to search all call sites to determine if `mode` parameter is ever used meaningfully.

---

### 2. C4020: Too Many Actual Parameters (4 warnings, 3 functions)

**Lines**: 2701, 2724, 4203, 4610  
**Functions**: `getMATrend_SignalBase` (2x), `readTurningPoint` (1x), `readTradingInfo` (1x)

#### Problem A: `getMATrend_SignalBase`

```c
// Header declaration (Base.h line 239):
int getMATrend_SignalBase(int rateShort, int rateLong, int ratesArrayIndex);

// Actual calls (TrendStrategy.c lines 2701, 2724):
getMATrend_SignalBase(20, 50, B_PRIMARY_RATES, 24)  // ← 4 args, expects 3
```

**Analysis**: Passing 4 arguments but function only takes 3. Extra argument `24` is ignored/causes stack corruption.

#### Problem B: `readTurningPoint`

```c
// Header declaration (StrategyUserInterface.h line 94):
int readTurningPoint(int instanceID);

// Actual call (TrendStrategy.c line 4203):
readTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo)  // ← 2 args, expects 1
```

**Analysis**: Passing 2 arguments but function only takes 1. The `&orderTurningInfo` pointer is pushed but not consumed.

#### Problem C: `readTradingInfo`

Similar pattern - extra argument(s) passed that function doesn't accept.

#### Root Cause:
API evolved but call sites weren't updated, or call sites anticipated future API that wasn't implemented.

#### Fix Strategy:
1. Check function implementations to see if they SHOULD accept extra parameters
2. If yes: Update declarations
3. If no: Remove extra arguments from call sites

---

### 3. C4133: Incompatible Pointer Types (7 warnings, 5 locations)

**Lines**: 4193, 5780, 6758, 6801 (2x), 8723, 9145

#### Problem A: Indicators* vs Base_Indicators* (5 warnings)

```c
// Lines 4193, 5780, 6758, 8723, 9145:
someFunction(pParams, pIndicators, pBase_Indicators);
//           ^         ^pIndicators is Indicators*
// But function expects Base_Indicators* for that parameter position
```

**Analysis**: Passing `Indicators*` where `Base_Indicators*` expected. These are different struct types - serious type mismatch.

#### Problem B: double* vs int* (2 warnings at line 6801)

```c
// Line 6801:
someFunction(..., &doubleVar1, &doubleVar2);
// Function expects: int*, int*
// Passing: double*, double*
```

**Analysis**: Critical type mismatch - sizeof(double) = 8 bytes, sizeof(int) = 4 bytes. Will cause memory corruption when function writes int values to double* locations.

#### Root Cause:
Wrong variables passed, or function signature is incorrect.

#### Fix Strategy:
1. Identify the called functions
2. Check their declarations
3. Either:
   - Pass correct variable types
   - OR cast (dangerous - only if intentional)
   - OR fix function to accept correct types

---

### 4. C4047 + C4024: Pointer Indirection Mismatch (8 warnings, 4 locations)

**Lines**: 4200, 4206, 4219, 4230  
**Function**: `saveTurningPoint`

#### Problem:

```c
// Header declaration (StrategyUserInterface.h line 93):
AsirikuyReturnCode saveTurningPoint(int instanceID, BOOL isTurning);
//                                                   ^^^^ BOOL value

// Actual calls (TrendStrategy.c):
saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
//                                                              ^^^^^^^^^^^^^^^^^
//                                                              Order_Turning_Info* pointer
```

**Analysis**: 
- Function expects **`BOOL` value** (1 byte scalar)
- Caller passes **`Order_Turning_Info*` pointer** (4/8 byte address)
- C4047: Different indirection levels (value vs pointer)
- C4024: Type mismatch (BOOL vs struct pointer)

#### Root Cause:
API mismatch - either:
1. Function should take `Order_Turning_Info*` instead of `BOOL`
2. OR caller should pass `orderTurningInfo.isTurning` instead of `&orderTurningInfo`

#### Fix Strategy:
Check function implementation:
- If function actually needs full struct: Change declaration to accept `Order_Turning_Info*`
- If function only needs boolean flag: Change call sites to pass `orderTurningInfo.isTurning`

---

### 5. C4244: Type Conversion Data Loss (7 warnings)

**Lines**: 9428, 9443, 9459, 9522, 9523, 9848, 9849  
**Pattern**: `int variable = double expression`

#### Problem A: MACD Period Assignments (lines 9428, 9443, 9459)

```c
// Line 9428 context (inside GBPJPY symbol block):
int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
// These are declared as int and assigned literal int values
// No conversion warning should occur here...

// ACTUAL ISSUE: Need to find the real line 9428 assignment
```

**Note**: The literal integer assignments shown don't cause C4244. Need to find actual double→int conversions.

#### Problem B: Time Calculations (lines 9522, 9523, 9848, 9849)

```c
// Variable declarations (line 9367):
int close_index_rate = -1, diff4Hours, diffDays, diffWeeks;
//  ^^^ All declared as int

// Assignments causing warnings (lines 9522-9523):
diff4Hours = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60* 60 *4);
//           ^^^^^^^^ difftime() returns double, dividing by int still gives double
//           Assigning double result to int variable → C4244

diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 24);
//         ^^^^^^^^ Same issue

diffWeeks = (timeInfo1.tm_wday + 1 + diffDays) / 7;
//          ^^^^^ diffDays is int, but intermediate calculation might be double

close_index_rate = shift1Index_4H - (diff4Hours - diffWeeks * 2 * 6);
//                 ^^^ All ints, no warning expected here
```

**Analysis**:
- `difftime()` returns `double` (seconds as floating point)
- Dividing by integer literals keeps result as `double`
- Assigning to `int` variables truncates fractional part
- Lines 9848-9849 have identical pattern

#### Root Cause:
Variables declared as `int` but should be `double` for time calculations, OR explicit cast needed if truncation is intentional.

#### Fix Strategy:
**Option A** (Recommended): Change variable types to match calculation:
```c
double diff4Hours, diffDays, diffWeeks;
int close_index_rate;  // Keep as int since used as array index
```

**Option B**: Add explicit casts if truncation is intentional:
```c
diff4Hours = (int)(difftime(...) / (60*60*4));
```

**Decision Factor**: Check how variables are used:
- If used in further floating-point calculations → Change to double
- If only used for integer counting/indexing → Add explicit casts

---

## Priority Fix Order

### Priority 1: Critical Safety Issues
1. **C4133 (line 6801)**: double* vs int* - Memory corruption risk
2. **C4047/C4024 (lines 4200, 4206, 4219, 4230)**: Pointer vs value mismatch

### Priority 2: Function Signature Fixes
3. **C4028/C4029 (lines 937, 1027)**: Fix splitBuyOrders/splitSellOrders signatures
4. **C4020 (lines 2701, 2724, 4203, 4610)**: Fix parameter counts

### Priority 3: Type Mismatches
5. **C4133 (lines 4193, 5780, 6758, 8723, 9145)**: Indicators* vs Base_Indicators*

### Priority 4: Implicit Conversions
6. **C4244 (lines 9428, 9443, 9459, 9522, 9523, 9848, 9849)**: Add explicit casts or fix types

---

## Investigation Checklist

Before fixing each category, investigate:

### For C4028/C4029 (splitBuyOrders/splitSellOrders):
- [ ] Search for all call sites of these functions
- [ ] Check if `mode` parameter is actually used in function body
- [ ] Determine if removing parameter breaks anything

### For C4020 (too many parameters):
- [ ] Find actual function implementations
- [ ] Check git history - was API recently changed?
- [ ] Verify if extra parameters should be added to declaration

### For C4133 (pointer type mismatches):
- [ ] Line 6801: Identify the function being called
- [ ] Check if variables should be int or double
- [ ] Lines 4193, 5780, etc.: Identify which parameter position is wrong

### For C4047/C4024 (saveTurningPoint):
- [ ] Read StrategyUserInterface.c implementation of saveTurningPoint
- [ ] Check if function internally expects full struct
- [ ] Look for other call sites to see correct usage pattern

### For C4244 (type conversions):
- [ ] Verify lines 9428, 9443, 9459 - might be false positives or need to find actual lines
- [ ] For lines 9522, 9523, 9848, 9849: Check usage of diff4Hours, diffDays, diffWeeks
- [ ] Determine if fractional seconds matter for logic

---

## Testing Strategy

After fixes:
1. **Compile clean**: Zero warnings
2. **Unit test**: If tests exist, run them
3. **Integration test**: Run strategy in backtest mode
4. **Regression test**: Compare before/after behavior on sample data

---

## Files to Examine

### Header Files:
- `dev/TradingStrategies/include/strategies/TrendStrategy.h`
- `dev/TradingStrategies/include/strategies/Base.h`
- `dev/TradingStrategies/include/StrategyUserInterface.h`

### Implementation Files:
- `dev/TradingStrategies/src/strategies/TrendStrategy.c` (primary)
- `dev/TradingStrategies/src/strategies/Base.c` (for getMATrend_SignalBase)
- `dev/TradingStrategies/src/StrategyUserInterface.c` (for saveTurningPoint/readTurningPoint)

### Search Patterns:
```powershell
# Find all call sites of problematic functions:
Select-String -Path *.c -Pattern "splitBuyOrders_Limit|splitSellOrders_Limit"
Select-String -Path *.c -Pattern "getMATrend_SignalBase"
Select-String -Path *.c -Pattern "saveTurningPoint|readTurningPoint"
Select-String -Path *.c -Pattern "readTradingInfo"
```

---

## Next Steps

1. **Review this analysis** with domain expert (if available) to understand intent
2. **Investigate each category** systematically using checklist above
3. **Create fix plan** with before/after for each warning
4. **Implement fixes** in order of priority
5. **Test thoroughly** before committing
6. **Document changes** in commit message

---

## Notes

- All line numbers based on current file state (10,296 lines in TrendStrategy.c)
- Some warnings may be interdependent - fixing one might reveal or fix others
- Consider creating backup branch before starting fixes
- This is trading software - incorrect fixes could lead to financial losses

**Recommended Approach**: Fix one category at a time, rebuild, test, commit, repeat.
