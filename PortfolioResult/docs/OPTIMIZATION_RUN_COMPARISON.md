# Optimization Run Comparison Analysis

## Executive Summary

This document compares two optimization runs performed on December 1, 2025:
- **Run 1 (152053)**: Aggressive portfolio with higher returns but higher risk
- **Run 2 (153637)**: Conservative portfolio with better risk-adjusted metrics

**Key Finding**: Run 2 explored 69.8% more combinations (438 vs 258) and found a more conservative solution with significantly better downside protection, though at the cost of lower absolute returns.

---

## Run Configuration

Both runs used identical configuration:
- **Strategies to optimize**: 13 strategies
- **Predefined strategies**: 0
- **Risk multipliers**: [0.2, 0.4, 1.0, 1.4, 1.8, 2.0, 2.2]
- **Total search space**: 2,147,483,647 combinations (7^13)
- **Optimization factors**:
  - max_max_dd: 16.0
  - max_ulcerIndex: 5.0
  - min_max_dd: 8.0

**Key Difference**: Run 2 tested **438 combinations** vs Run 1's **258 combinations** (+69.8% more exploration)

---

## Comprehensive Metrics Comparison

| Metric | Run 1 (152053) | Run 2 (153637) | Difference | Winner | Analysis |
|--------|----------------|----------------|------------|--------|----------|
| **Martin Ratio** | 13.2108 | 13.1854 | -0.0254 (-0.19%) | Run 1 ✓ | Nearly identical, Run 1 slightly better |
| **Total Return** | 11.2340x | 5.1833x | -6.0507x (-53.86%) | Run 1 ✓ | Run 1 has 2.17x higher return |
| **Max Drawdown** | 15.97% | 11.48% | -4.49% (-28.11%) | Run 2 ✓ | Run 2 has 28% lower drawdown |
| **Max DD Length** | 132.5 days | 144.8 days | +12.4 days (+9.35%) | Run 1 ✓ | Run 1 recovers faster |
| **Profit Factor** | 1.4275 | 1.4451 | +0.0176 (+1.23%) | Run 2 ✓ | Run 2 slightly better |
| **CAGR** | 64.44% | 40.26% | -24.18% (-37.52%) | Run 1 ✓ | Run 1 has 60% higher CAGR |
| **CAGR/MaxDD** | 4.0358 | 3.5072 | -0.5286 (-13.10%) | Run 1 ✓ | Run 1 better return per unit risk |
| **Sharpe Ratio** | 1.6986 | 1.7657 | +0.0671 (+3.95%) | Run 2 ✓ | Run 2 better risk-adjusted return |
| **Ulcer Index** | 4.8782 | 3.0536 | -1.8246 (-37.40%) | Run 2 ✓ | Run 2 significantly better downside risk |
| **Best Trade** | $67,929 | $23,364 | -$44,565 (-65.61%) | Run 1 ✓ | Run 1 has larger winning trades |
| **Worst Trade** | -$28,805 | -$8,393 | +$20,412 (-70.86%) | Run 2 ✓ | Run 2 has much smaller losses |
| **Risk/Reward** | 1.4208 | 1.4341 | +0.0133 (+0.94%) | Run 2 ✓ | Run 2 slightly better |

---

## Risk Allocation Comparison

### Strategy-by-Strategy Risk Changes

| Strategy | Run 1 Risk | Run 2 Risk | Change | Impact |
|----------|------------|------------|--------|--------|
| 200003 | 2.20x | 2.20x | = Same | No change |
| 200009 | 1.40x | 1.40x | = Same | No change |
| 500007 | 2.20x | 2.20x | = Same | No change |
| 841005 | 1.00x | **0.40x** | ↓ -0.60x | **Significantly reduced** |
| 842001 | 0.40x | 0.40x | = Same | No change |
| 860001 | 2.20x | **1.40x** | ↓ -0.80x | **Reduced** |
| 860002 | 0.20x | 0.20x | = Same | No change |
| 860006 | 2.00x | **1.00x** | ↓ -1.00x | **Halved** |
| 860007 | 0.40x | **0.20x** | ↓ -0.20x | Reduced |
| 860008 | 1.40x | **1.00x** | ↓ -0.40x | Reduced |
| 860011 | 0.20x | 0.20x | = Same | No change |
| 860013 | 2.00x | **1.00x** | ↓ -1.00x | **Halved** |
| 900002 | 0.40x | **0.20x** | ↓ -0.20x | Reduced |

**Summary**:
- **7 strategies** reduced risk (none increased)
- **6 strategies** unchanged
- **Total absolute difference**: 4.20x risk reduction
- **Average risk multiplier**: Run 1 = 1.23x, Run 2 = 0.91x (-26.2%)

### Risk Level Distribution

**Run 1 (Aggressive)**:
- 0.2x: 2 strategies
- 0.4x: 3 strategies
- 1.0x: 1 strategy
- 1.4x: 2 strategies
- 2.0x: 2 strategies
- 2.2x: 3 strategies

**Run 2 (Conservative)**:
- 0.2x: 4 strategies (+2)
- 0.4x: 2 strategies (-1)
- 1.0x: 3 strategies (+2)
- 1.4x: 2 strategies (same)
- 2.0x: 0 strategies (-2)
- 2.2x: 2 strategies (-1)

**Key Observation**: Run 2 shifted from high-risk allocations (2.0x-2.2x) to moderate-risk allocations (0.2x-1.0x), particularly for strategies 841005, 860001, 860006, and 860013.

---

## Detailed Analysis

### 1. Martin Ratio Analysis

**Martin Ratio** = CAGR / Max Drawdown

- **Run 1**: 13.21 (64.44% / 15.97%)
- **Run 2**: 13.19 (40.26% / 11.48%)
- **Difference**: -0.19% (essentially identical)

**Insight**: Despite Run 1 having 60% higher CAGR, the Martin ratios are nearly identical because Run 1 also has 28% higher drawdown. This suggests both portfolios are similarly efficient in terms of return per unit of risk, but Run 2 achieves this with much lower absolute risk.

### 2. Profit Analysis

#### Total Return
- **Run 1**: 11.23x (1,023% return)
- **Run 2**: 5.18x (418% return)
- **Gap**: Run 1 returns 2.17x more

#### CAGR (Compound Annual Growth Rate)
- **Run 1**: 64.44% per year
- **Run 2**: 40.26% per year
- **Gap**: Run 1 grows 60% faster annually

#### Profit Factor
- **Run 1**: 1.4275
- **Run 2**: 1.4451 (+1.23%)
- **Winner**: Run 2 (slightly better)

**Insight**: Run 1 generates significantly higher absolute returns, but Run 2 has a slightly better profit factor, indicating more consistent profitability relative to losses.

### 3. Risk Analysis

#### Maximum Drawdown
- **Run 1**: 15.97%
- **Run 2**: 11.48%
- **Improvement**: 28.1% lower drawdown in Run 2

#### Ulcer Index (Downside Risk)
- **Run 1**: 4.88
- **Run 2**: 3.05
- **Improvement**: 37.4% better (lower is better)

#### Sharpe Ratio (Risk-Adjusted Return)
- **Run 1**: 1.70
- **Run 2**: 1.77 (+3.95%)
- **Winner**: Run 2

#### Worst Trade
- **Run 1**: -$28,805
- **Run 2**: -$8,393
- **Improvement**: 70.9% smaller worst loss

**Insight**: Run 2 provides significantly better downside protection with:
- 28% lower maximum drawdown
- 37% better Ulcer Index
- 71% smaller worst trade
- Better Sharpe ratio

### 4. Trade Quality Analysis

#### Best Trade
- **Run 1**: $67,929
- **Run 2**: $23,364
- **Gap**: Run 1 has 2.9x larger best trade

#### Risk/Reward Ratio
- **Run 1**: 1.4208
- **Run 2**: 1.4341 (+0.94%)
- **Winner**: Run 2 (slightly better)

**Insight**: Run 1 has larger individual winning trades, but Run 2 has slightly better overall risk/reward ratio, suggesting more consistent performance.

---

## Optimization Exploration Analysis

### Combinations Tested
- **Run 1**: 258 combinations
- **Run 2**: 438 combinations
- **Difference**: +180 combinations (+69.8% more exploration)

### Why More Exploration Matters

Run 2's additional exploration (438 vs 258 combinations) likely contributed to finding a more conservative solution. The algorithm had more opportunities to:
1. Explore lower-risk allocations
2. Find solutions with better risk-adjusted metrics
3. Discover portfolios with superior downside protection

**Hypothesis**: The additional exploration allowed the optimizer to find a local optimum in a different region of the solution space - one that prioritizes risk control over absolute returns.

---

## Portfolio Characteristics Summary

### Run 1 (152053) - Aggressive Portfolio

**Strengths**:
- ✓ 2.17x higher total return (11.23x vs 5.18x)
- ✓ 60% higher CAGR (64.4% vs 40.3%)
- ✓ Slightly better Martin ratio (13.21 vs 13.19)
- ✓ Faster drawdown recovery (132.5 vs 144.8 days)
- ✓ Larger winning trades ($67,929 vs $23,364)

**Weaknesses**:
- ✗ 28% higher maximum drawdown (15.97% vs 11.48%)
- ✗ 37% worse Ulcer Index (4.88 vs 3.05)
- ✗ 71% larger worst trade (-$28,805 vs -$8,393)
- ✗ Lower Sharpe ratio (1.70 vs 1.77)
- ✗ Lower profit factor (1.43 vs 1.45)

**Risk Profile**: High risk, high return
**Average Risk Multiplier**: 1.23x

### Run 2 (153637) - Conservative Portfolio

**Strengths**:
- ✓ 28% lower maximum drawdown (11.48% vs 15.97%)
- ✓ 37% better Ulcer Index (3.05 vs 4.88)
- ✓ Better Sharpe ratio (1.77 vs 1.70)
- ✓ Better profit factor (1.45 vs 1.43)
- ✓ 71% smaller worst trade (-$8,393 vs -$28,805)
- ✓ Better risk/reward ratio (1.43 vs 1.42)

**Weaknesses**:
- ✗ 54% lower total return (5.18x vs 11.23x)
- ✗ 38% lower CAGR (40.3% vs 64.4%)
- ✗ Slightly lower Martin ratio (13.19 vs 13.21)
- ✗ Longer drawdown recovery (144.8 vs 132.5 days)
- ✗ Smaller winning trades ($23,364 vs $67,929)

**Risk Profile**: Moderate risk, moderate return
**Average Risk Multiplier**: 0.91x (-26.2% vs Run 1)

---

## Recommendations

### Choose Run 1 (Aggressive) If:
1. **High risk tolerance**: You can accept 15.97% drawdowns
2. **Maximum returns priority**: You want 2.17x higher total return
3. **Long-term horizon**: You can wait out drawdown periods
4. **Capital availability**: You have sufficient capital to handle larger losses

**Best for**: Aggressive investors seeking maximum growth

### Choose Run 2 (Conservative) If:
1. **Risk management priority**: You want 28% lower drawdown
2. **Downside protection**: You need 71% smaller worst losses
3. **Risk-adjusted returns**: You prefer better Sharpe ratio (1.77 vs 1.70)
4. **Consistency**: You value steady, controlled growth

**Best for**: Moderate risk investors prioritizing capital preservation

### Hybrid Approach:
Consider a **middle ground** by:
1. Using Run 2's conservative allocations as a base
2. Selectively increasing risk on top performers (200003, 200009, 500007)
3. Monitoring and adjusting based on market conditions

---

## Key Insights

### 1. Exploration Impact
Run 2's 69.8% more exploration (438 vs 258 combinations) found a fundamentally different solution - one that prioritizes risk control. This suggests:
- The solution space has multiple local optima
- More exploration can reveal better risk-adjusted solutions
- The optimizer may benefit from even more exploration for 13+ strategies

### 2. Risk-Return Tradeoff
The comparison clearly demonstrates the classic risk-return tradeoff:
- **Run 1**: Higher risk (15.97% DD) → Higher return (11.23x)
- **Run 2**: Lower risk (11.48% DD) → Lower return (5.18x)

However, Run 2 achieves similar Martin ratio with much better downside metrics.

### 3. Portfolio Construction Differences
Run 2 reduced risk on 7 strategies, particularly:
- **841005**: 1.0x → 0.4x (60% reduction)
- **860001**: 2.2x → 1.4x (36% reduction)
- **860006**: 2.0x → 1.0x (50% reduction)
- **860013**: 2.0x → 1.0x (50% reduction)

These reductions significantly improved downside protection while maintaining competitive risk-adjusted returns.

### 4. Martin Ratio Similarity
Despite vastly different return profiles, both portfolios have nearly identical Martin ratios (13.21 vs 13.19). This suggests:
- Both solutions are near-optimal for their risk profiles
- The optimizer is effectively balancing return and risk
- The choice between them is truly a risk tolerance decision

---

## Conclusion

Both optimization runs found high-quality solutions, but with different risk-return profiles:

- **Run 1** maximizes returns for risk-tolerant investors
- **Run 2** optimizes risk-adjusted returns for moderate-risk investors

The choice depends entirely on **risk tolerance** and **investment objectives**. For most institutional and conservative investors, **Run 2's superior downside protection** (28% lower drawdown, 37% better Ulcer Index, 71% smaller worst trade) makes it the preferred choice despite lower absolute returns.

---

**Generated**: December 1, 2025  
**Analysis Period**: Both runs optimized 13 strategies  
**Total Combinations Explored**: Run 1 = 258, Run 2 = 438

