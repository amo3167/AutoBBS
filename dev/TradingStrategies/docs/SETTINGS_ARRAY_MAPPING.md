# Settings Array Mapping

This document maps Python settings dictionary keys to C settings array indices.

## Settings Array Structure

The `StrategyParams->settings` array is indexed using the `settingsIndex_t` enum defined in `AsirikuyDefines.h`.

## Mapping Table

| Python Key | C Index | Enum Name | Type | Description |
|------------|---------|-----------|------|-------------|
| `ADDITIONAL_PARAM_1` | 0 | ADDITIONAL_PARAM_1 | double | Additional parameter 1 |
| `ADDITIONAL_PARAM_2` | 1 | ADDITIONAL_PARAM_2 | double | Additional parameter 2 |
| ... | ... | ... | ... | ... |
| `ADDITIONAL_PARAM_40` | 39 | ADDITIONAL_PARAM_40 | double | Additional parameter 40 |
| `IS_SPREAD_BETTING` | 40 | IS_SPREAD_BETTING | double | Spread betting flag |
| `USE_SL` | 41 | USE_SL | double | Use stop loss flag |
| `USE_TP` | 42 | USE_TP | double | Use take profit flag |
| `RUN_EVERY_TICK` | 43 | RUN_EVERY_TICK | double | Run every tick flag |
| `INSTANCE_MANAGEMENT` | 44 | INSTANCE_MANAGEMENT | double | Instance management mode |
| `MAX_OPEN_ORDERS` | 45 | MAX_OPEN_ORDERS | double | Maximum open orders |
| `IS_BACKTESTING` | 46 | IS_BACKTESTING | double | Backtesting flag (0 or 1) |
| `DISABLE_COMPOUNDING` | 47 | DISABLE_COMPOUNDING | double | Disable compounding flag |
| `TIMED_EXIT_BARS` | 48 | TIMED_EXIT_BARS | double | Timed exit bars |
| `ORIGINAL_EQUITY` | 49 | ORIGINAL_EQUITY | double | Original account equity |
| `OPERATIONAL_MODE` | 50 | OPERATIONAL_MODE | double | Operational mode (0=DISABLE, 1=ENABLE, 2=MONITOR) |
| `STRATEGY_INSTANCE_ID` | 51 | STRATEGY_INSTANCE_ID | double | Strategy instance ID |
| `INTERNAL_STRATEGY_ID` | 52 | INTERNAL_STRATEGY_ID | double | Internal strategy ID |
| `TIMEFRAME` | 53 | TIMEFRAME | double | Strategy timeframe |
| `WFO_PSET_MANAGEMENT` | 54 | WFO_PSET_MANAGEMENT | double | Walk-forward optimization parameter set management |
| `ANALYSIS_WINDOW_SIZE` | 55 | ANALYSIS_WINDOW_SIZE | double | Analysis window size |
| `PARAMETER_SET_POOL` | 56 | PARAMETER_SET_POOL | double | Parameter set pool |
| `ACCOUNT_RISK_PERCENT` | 57 | ACCOUNT_RISK_PERCENT | double | Account risk percentage |
| `MAX_DRAWDOWN_PERCENT` | 58 | MAX_DRAWDOWN_PERCENT | double | Maximum drawdown percentage |
| `MAX_SPREAD` | 59 | MAX_SPREAD_PRICE | double | Maximum spread (in price units) |
| `SL_ATR_MULTIPLIER` | 60 | SL_ATR_MULTIPLIER | double | Stop loss ATR multiplier |
| `TP_ATR_MULTIPLIER` | 61 | TP_ATR_MULTIPLIER | double | Take profit ATR multiplier |
| `ATR_AVERAGING_PERIOD` | 62 | ATR_AVERAGING_PERIOD | double | ATR averaging period |
| `ORDERINFO_ARRAY_SIZE` | 63 | ORDERINFO_ARRAY_SIZE | double | Order info array size |

## Array Size

Total array size: **64** elements (`STRATEGY_SETTINGS_ARRAY_SIZE`)

## Usage in Python

```python
settings = {
    "INTERNAL_STRATEGY_ID": 29.0,  # AUTOBBS
    "STRATEGY_INSTANCE_ID": 0.0,
    "OPERATIONAL_MODE": 1.0,  # MODE_ENABLE
    "IS_BACKTESTING": 0.0,
    "ACCOUNT_RISK_PERCENT": 2.0,
    "MAX_OPEN_ORDERS": 1.0,
    "TIMEFRAME": 1.0,  # M1
    "MAX_SPREAD": 0.0003,
    "SL_ATR_MULTIPLIER": 2.0,
    "TP_ATR_MULTIPLIER": 3.0,
    # ... more settings
}
```

## Conversion Function

The conversion function will map Python dictionary keys to array indices:

```c
static int get_setting_index(const char* key) {
    // Map string key to enum index
    if (strcmp(key, "INTERNAL_STRATEGY_ID") == 0) return INTERNAL_STRATEGY_ID;
    if (strcmp(key, "STRATEGY_INSTANCE_ID") == 0) return STRATEGY_INSTANCE_ID;
    // ... etc
}
```

## Notes

- All values are stored as `double` in the C array
- Boolean flags use 0.0 (false) or 1.0 (true)
- Enum values are stored as their numeric values
- Missing settings will default to 0.0

