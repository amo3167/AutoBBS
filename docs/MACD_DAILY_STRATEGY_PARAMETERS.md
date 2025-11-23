# MACD Daily Strategy - Optimizable Parameters

## Overview

The MACD Daily Strategy (`AUTOBBS_TREND_MODE=23`) is a comprehensive MACD-based trading strategy with extensive symbol-specific configurations. While many parameters are hardcoded per symbol, several key parameters can be optimized through the `.set` file.

## Strategy Identification

- **Strategy ID**: 29 (AutoBBS dispatcher)
- **Trend Mode**: 23 (`AUTOBBS_TREND_MODE=23`)
- **Source File**: `core/TradingStrategies/src/strategies/autobbs/trend/macd/MACDDailyStrategy.c`

## Optimizable Parameters

### 1. Risk Management Parameters

#### AUTOBBS_RISK_CAP (Index: 3)
- **Description**: Risk cap limit for the strategy
- **Used in**: All symbol configurations (BTCUSD, XAUUSD, SpotCrudeUSD, etc.)
- **Current Values**: 
  - BTCUSD: Uses parameter value
  - XAUUSD: Uses parameter value
  - SpotCrudeUSD: Uses parameter value
- **Optimization Range**: 0-10 (typical)
- **Impact**: Controls maximum risk per trade

#### AUTOBBS_MAX_ACCOUNT_RISK (Index: 5)
- **Description**: Maximum account risk percentage
- **Used in**: Risk calculations across all symbols
- **Current Values**: Varies by symbol (typically 20)
- **Optimization Range**: 10-30 (typical)
- **Impact**: Limits total account exposure

#### AUTOBBS_MAX_STRATEGY_RISK (Index: 8)
- **Description**: Maximum strategy-specific risk
- **Used in**: Strategy-level risk management
- **Current Values**: Typically 5-10
- **Optimization Range**: 1-15
- **Impact**: Limits risk for this specific strategy instance

### 2. Trading Parameters

#### AUTOBBS_ADJUSTPOINTS (Index: 0)
- **Description**: Adjustment points for entry/exit calculations
- **Used in**: General strategy adjustments
- **Current Values**: Varies (e.g., 10 for BTCUSD)
- **Optimization Range**: 0-50
- **Impact**: Fine-tunes entry/exit timing

#### AUTOBBS_RANGE (Index: 7)
- **Description**: BeiLi (divergence) turning point range check
- **Used in**: BeiLi detection filter
- **Current Values**: 
  - BTCUSD: 5 (hardcoded)
  - Most others: 10 (hardcoded)
  - Some: 10000 (default, effectively disabled)
- **Optimization Range**: 1-20 (for active BeiLi filtering)
- **Impact**: Controls sensitivity of divergence detection

#### AUTOBBS_EXECUTION_RATES (Index: 10)
- **Description**: Execution timeframe in minutes
- **Used in**: Timeframe filtering for execution
- **Current Values**: 
  - BTCUSD 1H: 60
  - BTCUSD 1D: 1440
- **Optimization Range**: 15, 30, 60, 240, 1440 (common timeframes)
- **Impact**: Determines when trades can be executed

### 3. Mode Parameters

#### AUTOBBS_TREND_MODE (Index: 2)
- **Description**: Strategy mode selector (must be 23 for MACD Daily)
- **Value**: Fixed at 23 (do not optimize)
- **Impact**: Routes to MACD Daily strategy

#### AUTOBBS_TP_MODE (Index: 1)
- **Description**: Take profit mode
- **Current Values**: 
  - BTCUSD: 1
- **Optimization Range**: 0-3
- **Impact**: Controls take profit calculation method

#### AUTOBBS_LONG_SHORT_MODE (Index: 4)
- **Description**: Long/short trading mode
- **Current Values**: Typically 1 (both directions)
- **Optimization Range**: 0-2
  - 0: Long only
  - 1: Both directions
  - 2: Short only
- **Impact**: Restricts trading direction

#### AUTOBBS_ONE_SIDE (Index: 11)
- **Description**: One-side trading restriction
- **Current Values**: Typically 0 (disabled)
- **Optimization Range**: 0-1
- **Impact**: Forces single-direction trading

### 4. Advanced Parameters

#### AUTOBBS_KEYK (Index: 6)
- **Description**: Key K parameter (advanced)
- **Current Values**: Typically 0
- **Optimization Range**: 0-10
- **Impact**: Advanced filtering parameter

#### AUTOBBS_MACRO_TREND (Index: 9)
- **Description**: Macro trend filter
- **Current Values**: Typically 0 (disabled)
- **Optimization Range**: 0-1
- **Impact**: Enables macro trend filtering

#### AUTOBBS_IS_ATREURO_RANGE (Index: 13)
- **Description**: ATR Euro range parameter
- **Current Values**: Typically 0 or 8
- **Optimization Range**: 0-20
- **Impact**: ATR-based range calculations

#### AUTOBBS_STARTHOUR (Index: 14)
- **Description**: Start hour for trading (0-23)
- **Current Values**: 
  - Default: 1
  - XAUUSD: Uses parameter value
- **Optimization Range**: 0-23
- **Impact**: Restricts trading to specific hours

#### AUTOBBS_IS_AUTO_MODE (Index: 12)
- **Description**: Auto mode flag
- **Current Values**: Typically 1 (enabled)
- **Optimization Range**: 0-1
- **Impact**: Enables automatic parameter adjustments

## Symbol-Specific Hardcoded Parameters

The following parameters are **hardcoded per symbol** and **cannot be optimized** through the set file:

### MACD Parameters (Hardcoded)
- **Fast MA Period**: 
  - BTCUSD: 5 (before 2021-01-18) or 7 (after)
  - Most others: 5
  - AUDNZD: 12
- **Slow MA Period**:
  - BTCUSD: 10 (before 2021-01-18) or 14 (after)
  - Most others: 10
  - AUDNZD: 26
- **Signal MA Period**:
  - BTCUSD: 5 (before 2021-01-18) or 7 (after)
  - Most others: 5
  - AUDNZD: 9

### Entry Thresholds (Hardcoded)
- **Level**: Symbol-specific (e.g., BTCUSD: 0.5% of price, XAUUSD: 2.0)
- **Max Level**: Symbol-specific (e.g., BTCUSD: 5% of price)
- **Hist Level**: Typically 0.01

### Risk Multipliers (Hardcoded)
- **Stop Loss Multiplier**: 
  - BTCUSD: 1.8
  - XAUUSD: Varies
  - Most: 1.8
- **Max Risk**: 
  - BTCUSD: 1.5
  - XAUUSD: 2.0
  - Most: 1.5-2.0

### Filter Flags (Hardcoded)
- Volume Control
- BeiLi Detection
- Slow Line Filter
- ATR Filter
- CMF Volume Filter
- CMF Volume Gap Filter
- Max Level Checks
- Weekly Baseline
- End-of-Day Entry
- Next Day Bar Filter
- Sunday Skip (crypto)

## Recommended Optimization Parameters for BTCUSD

Based on the source code analysis, here are the most impactful parameters to optimize for BTCUSD:

### High Impact Parameters

1. **AUTOBBS_RISK_CAP** (Index: 3)
   - Range: 1.0 - 10.0
   - Step: 1.0
   - Impact: Directly controls risk per trade

2. **AUTOBBS_MAX_ACCOUNT_RISK** (Index: 5)
   - Range: 10.0 - 30.0
   - Step: 5.0
   - Impact: Controls total account exposure

3. **AUTOBBS_MAX_STRATEGY_RISK** (Index: 8)
   - Range: 1.0 - 10.0
   - Step: 1.0
   - Impact: Strategy-specific risk limit

### Medium Impact Parameters

4. **AUTOBBS_RANGE** (Index: 7)
   - Range: 5.0 - 15.0
   - Step: 5.0
   - Note: Currently hardcoded to 5 for BTCUSD, but parameter is read
   - Impact: BeiLi detection sensitivity

5. **AUTOBBS_ADJUSTPOINTS** (Index: 0)
   - Range: 5.0 - 20.0
   - Step: 5.0
   - Impact: Entry/exit timing adjustments

6. **AUTOBBS_STARTHOUR** (Index: 14)
   - Range: 0 - 6 (early hours)
   - Step: 1
   - Impact: Trading hour restrictions

### Lower Impact Parameters

7. **AUTOBBS_EXECUTION_RATES** (Index: 10)
   - Values: 60 (1H) or 1440 (1D)
   - Note: Should match your timeframe
   - Impact: Execution timing

8. **AUTOBBS_TP_MODE** (Index: 1)
   - Range: 0 - 3
   - Impact: Take profit calculation

## Parameter Index Reference

| Parameter | Index | Type | Optimizable | Notes |
|-----------|-------|------|-------------|-------|
| AUTOBBS_ADJUSTPOINTS | 0 | double | ✅ Yes | Entry/exit adjustments |
| AUTOBBS_TP_MODE | 1 | int | ✅ Yes | Take profit mode |
| AUTOBBS_TREND_MODE | 2 | int | ❌ No | Must be 23 for MACD Daily |
| AUTOBBS_RISK_CAP | 3 | double | ✅ Yes | Risk cap limit |
| AUTOBBS_LONG_SHORT_MODE | 4 | int | ✅ Yes | Trading direction |
| AUTOBBS_MAX_ACCOUNT_RISK | 5 | double | ✅ Yes | Account risk limit |
| AUTOBBS_KEYK | 6 | double | ✅ Yes | Key K parameter |
| AUTOBBS_RANGE | 7 | double | ⚠️ Partial | Hardcoded for some symbols |
| AUTOBBS_MAX_STRATEGY_RISK | 8 | double | ✅ Yes | Strategy risk limit |
| AUTOBBS_MACRO_TREND | 9 | int | ✅ Yes | Macro trend filter |
| AUTOBBS_EXECUTION_RATES | 10 | int | ✅ Yes | Execution timeframe |
| AUTOBBS_ONE_SIDE | 11 | int | ✅ Yes | One-side trading |
| AUTOBBS_IS_AUTO_MODE | 12 | int | ✅ Yes | Auto mode flag |
| AUTOBBS_IS_ATREURO_RANGE | 13 | double | ✅ Yes | ATR Euro range |
| AUTOBBS_STARTHOUR | 14 | int | ✅ Yes | Start hour (0-23) |

## Optimization Strategy

### Phase 1: Risk Parameters (Start Here)
Focus on risk management parameters first:
- AUTOBBS_RISK_CAP
- AUTOBBS_MAX_ACCOUNT_RISK
- AUTOBBS_MAX_STRATEGY_RISK

### Phase 2: Trading Parameters
Once risk is optimized, fine-tune trading behavior:
- AUTOBBS_ADJUSTPOINTS
- AUTOBBS_RANGE (if not hardcoded)
- AUTOBBS_STARTHOUR

### Phase 3: Advanced Parameters
Finally, optimize advanced settings:
- AUTOBBS_TP_MODE
- AUTOBBS_MACRO_TREND
- AUTOBBS_IS_ATREURO_RANGE

## Limitations

1. **MACD Periods**: Fast, slow, and signal periods are hardcoded per symbol and cannot be optimized
2. **Entry Thresholds**: Level, maxLevel, and histLevel are symbol-specific and hardcoded
3. **Filter Flags**: Volume control, BeiLi, ATR filters, etc. are hardcoded per symbol
4. **Stop Loss Multiplier**: Hardcoded per symbol (typically 1.8)
5. **Max Risk**: Hardcoded per symbol (typically 1.5-2.0)

## Notes

- The strategy uses `parameter(AUTOBBS_RISK_CAP)` to read the risk cap value
- Most symbol-specific configurations are set in `initializeSymbolConfig()` function
- BTCUSD has special time-based MACD parameters (changes after 2021-01-18)
- Some parameters like `AUTOBBS_RANGE` are hardcoded for specific symbols but the parameter function still exists

## Example Optimization Setup

For BTCUSD MACD Daily strategy, a good starting optimization would be:

```ini
# High impact risk parameters
AUTOBBS_RISK_CAP=0
AUTOBBS_RISK_CAP,F = 1
AUTOBBS_RISK_CAP,1 = 1.0
AUTOBBS_RISK_CAP,2 = 1.0
AUTOBBS_RISK_CAP,3 = 10.0

AUTOBBS_MAX_ACCOUNT_RISK=20
AUTOBBS_MAX_ACCOUNT_RISK,F = 1
AUTOBBS_MAX_ACCOUNT_RISK,1 = 10.0
AUTOBBS_MAX_ACCOUNT_RISK,2 = 5.0
AUTOBBS_MAX_ACCOUNT_RISK,3 = 30.0

AUTOBBS_MAX_STRATEGY_RISK=5
AUTOBBS_MAX_STRATEGY_RISK,F = 1
AUTOBBS_MAX_STRATEGY_RISK,1 = 1.0
AUTOBBS_MAX_STRATEGY_RISK,2 = 1.0
AUTOBBS_MAX_STRATEGY_RISK,3 = 10.0
```

This focuses on the three most impactful risk management parameters that are actually used by the strategy.

