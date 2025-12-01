# Start Date Selection Guide for Portfolio Optimization

## Executive Summary

This document compares three optimization runs using Config #5 with different start dates:
- **2018-01-01**: 7 years of data (most comprehensive)
- **2021-01-01**: 4 years of data (post-COVID focus)
- **2023-01-01**: 2 years of data (most recent only)

**Key Finding**: Each start date produces significantly different results, with trade-offs between data volume, risk-adjusted returns, and absolute returns.

---

## Comprehensive Metrics Comparison

| Metric | 2018-01-01 | 2021-01-01 | 2023-01-01 | Winner | Analysis |
|--------|------------|------------|------------|--------|----------|
| **Total Return** | 113.69x | 11.23x | 9.68x | 2018 ✓ | 2018 has 10x higher return |
| **Max Drawdown** | 15.66% | 13.83% | 15.11% | 2021 ✓ | 2021 has lowest drawdown |
| **Max DD Length** | 137.8 days | 118.0 days | 103.9 days | 2023 ✓ | 2023 recovers fastest |
| **Profit Factor** | 1.44 | 1.46 | 1.69 | 2023 ✓ | 2023 significantly better |
| **CAGR** | 83.28% | 64.44% | 120.78% | 2023 ✓ | 2023 has 45% higher CAGR |
| **CAGR/MaxDD** | 5.32 | 4.66 | 7.99 | 2023 ✓ | 2023 best return per risk |
| **Sharpe Ratio** | 2.08 | 1.89 | 2.26 | 2023 ✓ | 2023 best risk-adjusted |
| **Ulcer Index** | 3.83 | 3.98 | 3.57 | 2023 ✓ | 2023 lowest downside risk |
| **Martin Ratio** | 21.76 | 16.18 | 33.84 | 2023 ✓ | 2023 55% better than 2018 |
| **Best Trade** | $627,969 | $53,255 | $121,975 | 2018 ✓ | 2018 has largest wins |
| **Worst Trade** | -$241,505 | -$24,306 | -$21,647 | 2023 ✓ | 2023 smallest losses |
| **Risk/Reward** | 1.52 | 1.48 | 1.68 | 2023 ✓ | 2023 best ratio |
| **Total Trades** | 4,539 | 2,862 | 1,710 | 2018 ✓ | 2018 most trades |

---

## Detailed Analysis by Start Date

### 2018-01-01 (7 Years of Data) - "Comprehensive Historical"

**Strengths**:
- ✅ **Highest absolute return**: 113.69x (10x higher than others)
- ✅ **Most trades**: 4,539 (best statistical significance)
- ✅ **Comprehensive data**: Includes pre-COVID, COVID, and recovery periods
- ✅ **Good Martin Ratio**: 21.76 (solid risk-adjusted return)
- ✅ **Large winning trades**: $627,969 best trade

**Weaknesses**:
- ✗ **Higher drawdown**: 15.66% (vs 13.83% for 2021)
- ✗ **Larger worst trade**: -$241,505 (vs -$21,647 for 2023)
- ✗ **Lower risk-adjusted metrics**: Sharpe 2.08 vs 2.26 for 2023
- ✗ **Lower profit factor**: 1.44 vs 1.69 for 2023

**Data Period**: 2018-2024
- Includes: Pre-COVID normal markets (2018-2019)
- Includes: COVID crash and recovery (2020-2021)
- Includes: Recent market conditions (2022-2024)

**Best For**:
- Long-term investors seeking maximum returns
- Those who want comprehensive historical validation
- Investors comfortable with higher drawdowns
- When statistical significance is important (more trades)

---

### 2021-01-01 (4 Years of Data) - "Post-COVID Focus"

**Strengths**:
- ✅ **Lowest drawdown**: 13.83% (best downside protection)
- ✅ **Balanced metrics**: Good middle ground between 2018 and 2023
- ✅ **Moderate data**: 4 years provides good balance
- ✅ **Excludes COVID crash**: Focuses on post-COVID market

**Weaknesses**:
- ✗ **Lowest absolute return**: 11.23x (vs 113.69x for 2018)
- ✗ **Lowest CAGR**: 64.44% (vs 120.78% for 2023)
- ✗ **Lowest Martin Ratio**: 16.18 (vs 33.84 for 2023)
- ✗ **Lower profit factor**: 1.46 (vs 1.69 for 2023)

**Data Period**: 2021-2024
- Includes: Post-COVID recovery period
- Includes: Recent market volatility (2022-2024)
- Excludes: Pre-COVID period (2018-2019)
- Excludes: COVID crash (2020)

**Best For**:
- Investors who want to avoid COVID crash period
- Those seeking moderate risk with balanced returns
- When you want post-COVID market focus
- Conservative investors prioritizing lower drawdown

---

### 2023-01-01 (2 Years of Data) - "Most Recent Only"

**Strengths**:
- ✅ **Best risk-adjusted returns**: Martin Ratio 33.84 (55% better than 2018)
- ✅ **Highest CAGR**: 120.78% (45% higher than 2018)
- ✅ **Best Sharpe Ratio**: 2.26 (best risk-adjusted return)
- ✅ **Best Profit Factor**: 1.69 (17% better than others)
- ✅ **Lowest Ulcer Index**: 3.57 (best downside risk control)
- ✅ **Smallest worst trade**: -$21,647 (90% smaller than 2018)
- ✅ **Fastest recovery**: 103.9 days drawdown length

**Weaknesses**:
- ✗ **Limited data**: Only 2 years (may not be representative)
- ✗ **Fewest trades**: 1,710 (less statistical significance)
- ✗ **Lower absolute return**: 9.68x (vs 113.69x for 2018)
- ✗ **May overfit**: Recent conditions may not persist

**Data Period**: 2023-2024
- Includes: Most recent market conditions only
- Excludes: COVID period (2020-2022)
- Excludes: Pre-COVID period (2018-2019)
- **Warning**: Very limited historical data

**Best For**:
- Short-term traders focusing on current market regime
- Those prioritizing risk-adjusted metrics over absolute returns
- When recent market conditions are most relevant
- Risk-averse investors (best downside protection)

---

## Risk Allocation Comparison

### Strategies with Consistent Allocations Across All Dates

| Strategy | 2018 | 2021 | 2023 | Variation |
|----------|------|------|------|------------|
| 200003 | 2.20 | 2.20 | 2.20 | 0.00 |
| 500003 | 2.20 | 2.20 | 2.20 | 0.00 |
| 500007 | 2.20 | 2.20 | 2.20 | 0.00 |
| 841005 | 0.40 | 0.40 | 0.40 | 0.00 |

**Insight**: These 4 strategies show consistent optimal allocation regardless of start date, suggesting they're robust across different market conditions.

### Strategies with High Variation

| Strategy | 2018 | 2021 | 2023 | Variation | Analysis |
|----------|------|------|------|------------|----------|
| 842001 | 0.20 | 0.40 | **2.20** | 2.00 | Very sensitive to date range |
| 860007 | 0.40 | 0.20 | **2.20** | 2.00 | Very sensitive to date range |
| 860001 | 2.00 | 2.00 | **0.20** | 1.80 | Recent period shows different behavior |
| 860013 | 1.80 | 1.40 | **0.20** | 1.60 | Recent period shows different behavior |

**Insight**: These strategies show high sensitivity to the date range, particularly 842001 and 860007 which go from very low (0.2-0.4) to maximum (2.2) risk depending on the period.

---

## Recommendations by Use Case

### 1. Maximum Returns (Long-Term Investment)

**Recommended**: **2018-01-01**

**Rationale**:
- Highest absolute return (113.69x)
- Most comprehensive historical validation
- Most trades (4,539) for statistical significance
- Good Martin Ratio (21.76) despite higher drawdown

**Trade-offs**:
- Accept higher drawdown (15.66%)
- Accept larger worst trade (-$241,505)
- Portfolio optimized for long-term growth

**Best For**: Long-term investors, maximum growth seekers, those with high risk tolerance

---

### 2. Best Risk-Adjusted Returns

**Recommended**: **2023-01-01**

**Rationale**:
- Best Martin Ratio (33.84) - 55% better than 2018
- Best Sharpe Ratio (2.26)
- Best Profit Factor (1.69)
- Lowest Ulcer Index (3.57)
- Smallest worst trade (-$21,647)

**Trade-offs**:
- Limited data (only 2 years)
- Fewer trades (1,710) - less statistical significance
- May overfit to recent conditions
- Lower absolute return (9.68x)

**Best For**: Risk-averse investors, short-term traders, those prioritizing risk-adjusted metrics

**Warning**: Limited historical data may not be representative of future performance. Consider this a "recent market regime" optimization.

---

### 3. Balanced Approach (Moderate Risk)

**Recommended**: **2021-01-01**

**Rationale**:
- Lowest drawdown (13.83%)
- Balanced metrics across the board
- Excludes COVID crash period
- 4 years of data provides good balance

**Trade-offs**:
- Lower absolute return (11.23x)
- Lower risk-adjusted metrics than 2023
- Lower absolute return than 2018

**Best For**: Moderate risk investors, those avoiding COVID period, balanced portfolios

---

### 4. Hybrid Approach (Recommended)

**Recommended**: **Use 2018-01-01 with tighter factor constraints**

**Rationale**:
- Get comprehensive historical data (7 years)
- Use factor config to filter out high-risk portfolios
- Achieve balance between returns and risk

**Implementation**:
1. Use start date: **2018-01-01**
2. Use tighter factor config:
   ```csv
   Factor,value
   min_max_dd,8
   max_max_dd,13
   max_ulcerIndex,3.8
   ```
3. This filters portfolios with drawdown > 13% (similar to 2021 run)

**Expected Result**:
- More comprehensive data than 2021/2023
- Better risk control than pure 2018 run
- Good balance of returns and risk

---

## Key Insights

### 1. Data Period Impact

The start date significantly impacts results:
- **2018**: Optimizes for long-term growth (includes all periods)
- **2021**: Optimizes for post-COVID conditions (excludes crash)
- **2023**: Optimizes for recent market regime only (very limited)

### 2. Risk Allocation Sensitivity

Some strategies show high sensitivity to date range:
- **842001**: 0.2x (2018) → 2.2x (2023) - **10x variation!**
- **860007**: 0.2x (2021) → 2.2x (2023) - **10x variation!**

This suggests these strategies perform very differently in different market regimes.

### 3. Robust Strategies

Some strategies are consistent across all dates:
- **200003, 500003, 500007**: Always 2.2x (maximum risk)
- **841005**: Always 0.4x (conservative)

These are robust across different market conditions.

### 4. Statistical Significance

- **2018**: 4,539 trades - Most statistically significant
- **2021**: 2,862 trades - Moderate significance
- **2023**: 1,710 trades - Least statistically significant

More trades = more reliable optimization results.

---

## Decision Matrix

| Your Priority | Recommended Start Date | Why |
|---------------|----------------------|-----|
| **Maximum Returns** | 2018-01-01 | Highest absolute return (113.69x) |
| **Best Risk-Adjusted** | 2023-01-01 | Best Martin Ratio (33.84), Sharpe (2.26) |
| **Lowest Drawdown** | 2021-01-01 | Lowest max drawdown (13.83%) |
| **Most Data** | 2018-01-01 | 7 years, 4,539 trades |
| **Recent Market Focus** | 2023-01-01 | Most recent 2 years only |
| **Avoid COVID Period** | 2021-01-01 | Excludes 2020 crash |
| **Balanced Approach** | 2018-01-01 + Tighter Factors | Comprehensive data with risk control |

---

## Final Recommendation

### Primary Recommendation: **2018-01-01 with Tighter Factor Constraints**

**Why**:
1. **Comprehensive data**: 7 years provides best historical validation
2. **Statistical significance**: 4,539 trades vs 1,710 for 2023
3. **Risk control**: Use factor config to limit drawdown to ~13-14%
4. **Best of both worlds**: Historical validation + risk control

**Implementation**:
```bash
# Use 2018-01-01 start date
# Create portfolioOptimizeFactor5_tight.config:
Factor,value
min_max_dd,8
max_max_dd,13
max_ulcerIndex,3.8

# Run:
java -jar PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfolioOptimize1.config \
  portfolioOptimizePreset5.config \
  portfolioOptimizeFactor5_tight.config \
  2018-01-01
```

### Alternative: **2023-01-01 for Short-Term Trading**

**Why**:
- Best risk-adjusted metrics (Martin 33.84)
- Optimized for current market regime
- Best downside protection

**Warning**: Limited data (2 years) may not be representative. Use for short-term strategies only.

---

## Summary Table

| Start Date | Data Years | Trades | Total Return | Max DD | Martin Ratio | Best For |
|------------|------------|--------|--------------|--------|--------------|----------|
| **2018-01-01** | 7 | 4,539 | 113.69x | 15.66% | 21.76 | Long-term, maximum returns |
| **2021-01-01** | 4 | 2,862 | 11.23x | 13.83% | 16.18 | Balanced, avoid COVID |
| **2023-01-01** | 2 | 1,710 | 9.68x | 15.11% | 33.84 | Short-term, risk-adjusted |

---

**Last Updated**: December 2024  
**Related Documents**: 
- `OPTIMIZATION_DATE_RANGE_RECOMMENDATIONS.md`
- `OPTIMIZATION_RUN_COMPARISON.md`

