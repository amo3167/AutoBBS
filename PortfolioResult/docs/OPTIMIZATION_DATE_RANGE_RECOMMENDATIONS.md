# Optimization Date Range & Problematic Strategy Handling

## Problem Statement

Strategies **841005** and **842001** experienced significant drawdowns in **2020** due to market reactions (likely COVID-19 related). When optimizing from **2018-01-01**, we need to:
1. Exclude or minimize the impact of the 2020 drawdown period
2. Optimize based on more recent, relevant market conditions
3. Maintain robust portfolio performance

---

## Solution Options

### Option 1: Use Start Date Parameter (Recommended)

**Best for**: Excluding problematic historical periods entirely

**How it works**: The optimizer accepts an optional 4th argument `startDate` that filters all trades to only include those after the specified date.

**Command**:
```bash
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5.config \
  2018-01-01
```

**Advantages**:
- ✅ Completely excludes 2020 drawdown from optimization
- ✅ Optimizes based on 2018+ data only
- ✅ Simple to implement - just add date parameter
- ✅ No need to modify config files

**Disadvantages**:
- ⚠️ Reduces historical data available for optimization
- ⚠️ May miss important market regime information

**When to use**: When you want to optimize based on recent market conditions and exclude known problematic periods.

---

### Option 2: Use Preset Config (Predefined Strategies)

**Best for**: Keeping problematic strategies but with reduced risk

**How it works**: Set 841005 and 842001 as predefined strategies with lower risk multipliers in the preset config file. These strategies will maintain their fixed risk throughout optimization.

**Step 1**: Create/Edit `portfolioOptimizePreset5.config`:
```csv
StrategyID,Risk
841005,0.4
842001,0.4
```

**Step 2**: You can include these strategies in the main optimize config OR exclude them - either works:
- **If included in optimize config**: The preset value takes precedence, strategy is automatically excluded from optimization
- **If excluded from optimize config**: Strategy is only in preset, uses preset value

**Step 3**: Run optimizer:
```bash
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5.config \
  2018-01-01
```

**Advantages**:
- ✅ Includes strategies but with controlled risk
- ✅ Other strategies can still be optimized
- ✅ Flexible - can adjust risk levels independently
- ✅ Can combine with startDate for double protection

**Disadvantages**:
- ⚠️ Requires manual risk setting (may not be optimal)
- ⚠️ Still includes 2020 data (unless combined with startDate)

**When to use**: When you want to keep the strategies in the portfolio but reduce their impact.

**Recommended Risk Levels**:
- **Very Conservative**: 0.2x (20% of base risk)
- **Conservative**: 0.4x (40% of base risk)
- **Moderate**: 0.6x (60% of base risk)

---

### Option 3: Use Factor Config (Constraint-Based Filtering)

**Best for**: Filtering out portfolios with high drawdowns

**How it works**: Set tighter constraints in the factor config to filter out portfolios that exceed acceptable drawdown thresholds.

**Edit `portfolioOptimizeFactor5.config`**:
```csv
Factor,value
min_max_dd,8
max_max_dd,12
max_ulcerIndex,3.5
```

**Current settings** (from your runs):
- max_max_dd: 16.0
- max_ulcerIndex: 5.0

**Recommended tighter settings**:
- max_max_dd: 12.0 (reduced from 16.0)
- max_ulcerIndex: 3.5 (reduced from 5.0)

**Advantages**:
- ✅ Filters out high-risk portfolios automatically
- ✅ Works at portfolio level (not individual strategies)
- ✅ Can be combined with other methods

**Disadvantages**:
- ⚠️ May filter out otherwise good portfolios
- ⚠️ Doesn't specifically target 2020 period
- ⚠️ Less precise than date filtering

**When to use**: As a safety net to ensure selected portfolios meet risk criteria.

---

### Option 4: Hybrid Approach (Recommended for Best Results)

**Best for**: Maximum protection with optimal results

**Combination**:
1. **Start Date**: 2018-01-01 (exclude pre-2018 and problematic 2020 period)
2. **Preset Config**: Set 841005 and 842001 to conservative risk (0.4x)
3. **Factor Config**: Use moderate constraints (max_max_dd: 12, max_ulcerIndex: 3.5)

**Step 1**: Create `portfolioOptimizePreset5.config`:
```csv
StrategyID,Risk
841005,0.4
842001,0.4
```

**Step 2**: Create `portfolioOptimizeFactor5_2018.config`:
```csv
Factor,value
min_max_dd,8
max_max_dd,12
max_ulcerIndex,3.5
```

**Step 3**: `portfolioOptimize1.config` can include or exclude 841005 and 842001:
- **If included**: Preset values automatically take precedence (they won't be optimized)
- **If excluded**: Only preset values are used

**Step 4**: Run optimizer:
```bash
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5_2018.config \
  2018-01-01
```

**Advantages**:
- ✅ Triple protection: date filter + preset risk + factor constraints
- ✅ Optimizes based on relevant period (2018+)
- ✅ Controls problematic strategies explicitly
- ✅ Filters out high-risk portfolios

**Disadvantages**:
- ⚠️ More complex setup
- ⚠️ May be overly conservative

---

## Detailed Configuration Guide

### Understanding Config Files

#### 1. Main Optimize Config (`portfolioOptimize1.config`)
**Purpose**: Lists strategies to optimize with their base risk allocations

**Format**:
```csv
StrategyID,Risk
200003,1.0
200009,1.0
500007,1.0
860001,1.0
...
```

**Note**: Do NOT include 841005 and 842001 here if using preset config.

#### 2. Preset Config (`portfolioOptimizePreset5.config`)
**Purpose**: Lists strategies with FIXED risk allocations (not optimized)

**Format**:
```csv
StrategyID,Risk
841005,0.4
842001,0.4
```

**Key Points**:
- These strategies are included in every portfolio test
- Their risk is NOT optimized
- They're excluded from the optimization search space
- **IMPORTANT**: If a strategy appears in BOTH optimize config AND preset config, the preset value takes precedence and the strategy is excluded from optimization

#### 3. Factor Config (`portfolioOptimizeFactor5.config`)
**Purpose**: Sets optimization constraints/filters

**Format**:
```csv
Factor,value
min_max_dd,8
max_max_dd,16
max_ulcerIndex,5.0
```

**Parameters**:
- `min_max_dd`: Minimum acceptable max drawdown (filters out too-conservative portfolios)
- `max_max_dd`: Maximum acceptable max drawdown (filters out too-risky portfolios)
- `max_ulcerIndex`: Maximum acceptable Ulcer Index (downside risk measure)

---

## Recommended Approach for 2018-01-01 Optimization

### Scenario: Optimize from 2018-01-01, Handle 2020 Drawdown

**Recommended**: **Option 4 (Hybrid Approach)**

**Rationale**:
1. **Start Date (2018-01-01)**: Excludes pre-2018 data and focuses on recent market conditions
2. **Preset Config**: Reduces risk on problematic strategies (841005, 842001) to 0.4x
3. **Factor Config**: Sets moderate constraints to filter high-risk portfolios

**Step-by-Step Setup**:

1. **Create Preset Config** (`portfolioOptimizePreset5.config`):
```csv
StrategyID,Risk
841005,0.4
842001,0.4
```

2. **Create Factor Config** (`portfolioOptimizeFactor5_2018.config`):
```csv
Factor,value
min_max_dd,8
max_max_dd,12
max_ulcerIndex,3.5
```

3. **Verify Main Config** (`portfolioOptimize1.config`):
   - Should include all strategies EXCEPT 841005 and 842001
   - Example:
```csv
StrategyID,Risk
200003,1.0
200009,1.0
500007,1.0
860001,1.0
860002,1.0
860006,1.0
860007,1.0
860008,1.0
860011,1.0
860013,1.0
900002,1.0
```

4. **Run Optimization**:
```bash
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5_2018.config \
  2018-01-01
```

---

## Alternative: Exclude Strategies Entirely

If 841005 and 842001 are too problematic, you can exclude them entirely:

**Option**: Simply don't include them in any config file

**Main Config** (`portfolioOptimize1.config`):
```csv
StrategyID,Risk
200003,1.0
200009,1.0
500007,1.0
860001,1.0
860002,1.0
860006,1.0
860007,1.0
860008,1.0
860011,1.0
860013,1.0
900002,1.0
```

**Preset Config**: Leave empty or don't use it

**Run**:
```bash
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5.config \
  2018-01-01
```

---

## Factor Config Tuning Guide

### Conservative Settings (Lower Risk)
```csv
Factor,value
min_max_dd,8
max_max_dd,10
max_ulcerIndex,3.0
```
**Use when**: Prioritizing capital preservation

### Moderate Settings (Balanced)
```csv
Factor,value
min_max_dd,8
max_max_dd,12
max_ulcerIndex,3.5
```
**Use when**: Balancing risk and return

### Aggressive Settings (Higher Risk Tolerance)
```csv
Factor,value
min_max_dd,8
max_max_dd,16
max_ulcerIndex,5.0
```
**Use when**: Maximizing returns, accepting higher drawdowns

---

## Preset Risk Level Guidelines

### For Problematic Strategies (841005, 842001)

| Risk Level | Multiplier | Use Case |
|------------|------------|----------|
| Very Low | 0.2x | Strategies with severe historical issues |
| Low | 0.4x | Strategies with moderate issues (recommended for 841005/842001) |
| Moderate | 0.6x | Strategies with minor issues |
| Normal | 1.0x | No special concerns |

**Recommendation for 841005/842001**: Start with **0.4x**, then adjust based on results.

---

## Date Range Considerations

### Why 2018-01-01?

**Advantages**:
- ✅ Excludes pre-2018 data (may be less relevant)
- ✅ Includes 2018-2019 (pre-COVID normal market)
- ✅ Includes 2020+ (post-COVID market conditions)
- ✅ Provides ~7 years of data (good for optimization)

**Alternative Dates**:
- **2019-01-01**: Excludes 2018, focuses on recent years
- **2020-01-01**: Excludes pre-COVID entirely (may be too restrictive)
- **2021-01-01**: Post-COVID only (very limited data)

**Recommendation**: **2018-01-01** provides good balance of data volume and relevance.

---

## Validation Steps

After optimization, validate the results:

1. **Check Max Drawdown**: Should be ≤ max_max_dd from factor config
2. **Check Ulcer Index**: Should be ≤ max_ulcerIndex from factor config
3. **Review 841005/842001 Risk**: Should match preset config values
4. **Check Date Range**: Verify only 2018+ trades are included
5. **Compare Metrics**: Compare with previous optimizations

---

## Example: Complete Setup

### File 1: `portfolioOptimize1.config`
```csv
StrategyID,Risk
200003,1.0
200009,1.0
500007,1.0
860001,1.0
860002,1.0
860006,1.0
860007,1.0
860008,1.0
860011,1.0
860013,1.0
900002,1.0
```

### File 2: `portfolioOptimizePreset5.config`
```csv
StrategyID,Risk
841005,0.4
842001,0.4
```

### File 3: `portfolioOptimizeFactor5_2018.config`
```csv
Factor,value
min_max_dd,8
max_max_dd,12
max_ulcerIndex,3.5
```

### Command:
```bash
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5_2018.config \
  2018-01-01
```

---

## Summary & Recommendations

### Best Approach: Hybrid (Option 4)

1. ✅ **Use startDate = 2018-01-01** to focus on relevant period
2. ✅ **Set 841005 and 842001 in preset config** with 0.4x risk
3. ✅ **Use moderate factor constraints** (max_max_dd: 12, max_ulcerIndex: 3.5)
4. ✅ **Exclude 841005/842001 from main optimize config**

### Quick Decision Matrix

| Scenario | Recommended Option |
|----------|-------------------|
| Want to exclude 2020 entirely | Option 1 (startDate only) |
| Want to keep strategies but reduce risk | Option 2 (preset config) |
| Want portfolio-level risk filtering | Option 3 (factor config) |
| Want maximum protection | Option 4 (hybrid - recommended) |
| Strategies too problematic | Exclude entirely |

---

**Last Updated**: December 2024  
**Related Documents**: 
- `OPTIMIZATION_BUDGET_SCALING.md`
- `OPTIMIZATION_RUN_COMPARISON.md`

