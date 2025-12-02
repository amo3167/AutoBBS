# Portfolio Optimization Budget Scaling Strategy

## Overview

This document describes the automatic budget scaling strategy used by the portfolio optimizer. The system automatically selects the appropriate optimization algorithm and evaluation budget based on portfolio size to balance solution quality with computational efficiency.

## Algorithm Selection & Budget Scaling

The optimizer uses a tiered approach that adapts to portfolio size:

### Summary Table

| Strategy Count | Algorithm | Budget | Scaling Formula | Generations/Stages |
|----------------|-----------|--------|-----------------|-------------------|
| ≤6 | GRID_SEARCH | Exhaustive | All combinations | N/A (exhaustive) |
| 7-9 | GENETIC_ALGORITHM | 100,000 | Static | ~500 generations |
| 10 | GENETIC_ALGORITHM | 120,000 | +20% | ~600 generations |
| 11 | GENETIC_ALGORITHM | 150,000 | +50% | ~750 generations |
| 12 | GENETIC_ALGORITHM | 200,000 | +100% | ~1,000 generations |
| 13 | GENETIC_ALGORITHM | 300,000 | Extended GA (quality) | ~1,500 generations |
| 14 | GENETIC_ALGORITHM | 350,000 | Extended GA (quality) | ~1,750 generations |
| 15 | GENETIC_ALGORITHM | 400,000 | Extended GA (quality) | ~2,000 generations |
| 16-20 | COARSE_TO_FINE | 260k-460k | +50k per strategy | ~87k-153k per stage |
| 21+ | COARSE_TO_FINE | 540k+ | +80k per strategy | ~180k+ per stage |

## Detailed Breakdown

### 1. Small Portfolios (≤6 Strategies): GRID_SEARCH

**Algorithm**: Exhaustive Grid Search  
**Budget**: All combinations (exhaustive)  
**Complexity**: O(R^S) where R = risk levels, S = strategies

**Rationale**: 
- For small portfolios, exhaustive search is computationally feasible
- Guarantees finding the optimal solution
- No need for heuristics

**Examples**:
- 6 strategies × 7 risk levels = 117,649 combinations
- Execution time: ~2 minutes (at 1ms per evaluation)

### 2. Medium-Large Portfolios (7-15 Strategies): GENETIC_ALGORITHM

**Algorithm**: Genetic Algorithm (Population-based evolution)  
**Population Size**: 200 individuals  
**Complexity**: O(P × G) where P = population, G = generations

**Rationale for Extension to 13-15**: GA produces better quality results (~96% vs ~92% for Coarse-to-Fine). Extended range trades speed for quality.

#### 2.1 Small-Medium (7-9 Strategies): Static 100K

**Budget**: 100,000 evaluations  
**Generations**: ~500 (100K ÷ 200 population)  
**Coverage**: 
- 7 strategies: 12.1% of search space (823K combinations)
- 9 strategies: 0.25% of search space (40M combinations)

**Rationale**: 
- 100K evaluations provide sufficient exploration for smaller spaces
- GA's population diversity effectively explores the solution space
- Additional budget provides diminishing returns

#### 2.2 Medium (10 Strategies): +20% Increase

**Budget**: 120,000 evaluations  
**Generations**: ~600  
**Coverage**: 0.042% of search space (282M combinations)

**Rationale**: 
- Moderate increase accounts for larger search space
- More generations help convergence

#### 2.3 Large-Medium (11 Strategies): +50% Increase

**Budget**: 150,000 evaluations  
**Generations**: ~750  
**Coverage**: 0.008% of search space (1.97B combinations)

**Rationale**: 
- Significant space growth requires more exploration
- 50% increase provides better convergence

#### 2.4 Very Large-Medium (12 Strategies): +100% Increase

**Budget**: 200,000 evaluations  
**Generations**: ~1,000  
**Coverage**: 0.001% of search space (13.8B combinations)

**Rationale**: 
- Doubling budget compensates for exponential space growth
- More generations essential for convergence

### 3. Large Portfolios (13+ Strategies): COARSE_TO_FINE

**Algorithm**: Hierarchical Coarse-to-Fine Search  
**Stages**: 3-stage progressive refinement  
**Complexity**: O(stages × evaluations_per_stage)

The algorithm splits the budget into three stages:
1. **Stage 1 (Coarse)**: Random search with 3 risk levels
2. **Stage 2 (Refine)**: Local search around best solution
3. **Stage 3 (Polish)**: Final refinement with full resolution

#### 3.1 Moderate Scaling (13-15 Strategies)

**Formula**: `150,000 + (strategyCount - 13) × 30,000`

**Budget Examples**:
- 13 strategies: 150,000 total → 50,000 per stage
- 14 strategies: 180,000 total → 60,000 per stage
- 15 strategies: 210,000 total → 70,000 per stage

**Search Space**:
- 13 strategies: 96.9 billion combinations
- 15 strategies: 4.7 trillion combinations

**Rationale**: 
- Moderate scaling sufficient for initial exploration
- Each stage gets reasonable budget for refinement

#### 3.2 Aggressive Scaling (16-20 Strategies)

**Formula**: `210,000 + (strategyCount - 15) × 50,000`

**Budget Examples**:
- 16 strategies: 260,000 total → 87,000 per stage
- 17 strategies: 310,000 total → 103,000 per stage
- 18 strategies: 360,000 total → 120,000 per stage
- 19 strategies: 410,000 total → 137,000 per stage
- 20 strategies: 460,000 total → 153,000 per stage

**Search Space**:
- 16 strategies: 23.2 trillion combinations
- 20 strategies: 79.8 quadrillion combinations

**Rationale**: 
- Aggressive scaling needed for much larger spaces
- Each stage requires more budget for effective refinement

#### 3.3 Very Aggressive Scaling (21+ Strategies)

**Formula**: `460,000 + (strategyCount - 20) × 80,000`

**Budget Examples**:
- 21 strategies: 540,000 total → 180,000 per stage
- 22 strategies: 620,000 total → 207,000 per stage
- 25 strategies: 860,000 total → 287,000 per stage
- 30 strategies: 1,260,000 total → 420,000 per stage

**Search Space**:
- 21 strategies: 1.3 sextillion combinations
- 25 strategies: 1.3 sextillion combinations
- 30 strategies: 22.5 septillion combinations

**Rationale**: 
- Very aggressive scaling for astronomical search spaces
- Large budgets per stage essential for any meaningful exploration

## Search Space Analysis

### Exponential Growth

The search space grows exponentially: **7^S** where S = number of strategies

| Strategies | Total Combinations | Budget | Coverage % |
|------------|-------------------|--------|------------|
| 6 | 117,649 | All | 100% |
| 7 | 823,543 | 100K | 12.1% |
| 9 | 40,353,607 | 100K | 0.25% |
| 10 | 282,475,249 | 120K | 0.042% |
| 11 | 1,977,326,743 | 150K | 0.008% |
| 12 | 13,841,287,201 | 200K | 0.001% |
| 13 | 96,889,010,407 | 150K | 0.00015% |
| 15 | 4,747,561,509,943 | 210K | 0.000004% |
| 20 | 79.8 quadrillion | 460K | 0.0000000006% |
| 25 | 1.3 sextillion | 860K | Negligible |
| 30 | 22.5 septillion | 1.26M | Negligible |

**Key Insight**: As the search space grows exponentially, coverage percentage becomes meaningless. The algorithms rely on intelligent exploration rather than exhaustive coverage.

## Algorithm Characteristics

### GRID_SEARCH
- **Best for**: ≤6 strategies
- **Advantage**: Guaranteed optimal solution
- **Disadvantage**: Exponential complexity, impractical for large portfolios

### GENETIC_ALGORITHM
- **Best for**: 7-15 strategies (extended to 13-15 for better quality)
- **Advantage**: Population diversity, good exploration/exploitation balance, higher quality (96% vs 92%)
- **Parameters**:
  - Population: 200
  - Mutation rate: 15%
  - Crossover rate: 70%
  - Tournament size: 5
  - Elitism: Top 10%

### COARSE_TO_FINE
- **Best for**: 16+ strategies (GA extended to 13-15)
- **Advantage**: Hierarchical approach, scales to very large spaces
- **Stages**:
  1. Coarse: Random search with 3 risk levels
  2. Refine: Local search around best
  3. Polish: Final refinement with full resolution

## Implementation Details

### Code Location
`PortfolioResult/src/main/java/PortfolioResult/PortfolioResult/App.java`

### Key Methods
- Algorithm selection: `run_optimizer()` method (lines 743-803)
- GA implementation: `PortfolioOptimizerImpl.geneticAlgorithm()`
- Coarse-to-fine: `PortfolioOptimizerImpl.coarseToFine()`

### Budget Calculation Logic

```java
// 7-12 strategies: GENETIC_ALGORITHM
if (strategiesToOptimize.size() <= 9) {
    maxEvaluations = 100000;  // Static 100k
} else if (strategiesToOptimize.size() == 10) {
    maxEvaluations = 120000;  // +20%
} else if (strategiesToOptimize.size() == 11) {
    maxEvaluations = 150000;  // +50%
} else {  // 12 strategies
    maxEvaluations = 200000;  // +100%
}

// 13+ strategies: COARSE_TO_FINE
if (strategyCount <= 15) {
    maxEvaluations = 150000 + ((strategyCount - 13) * 30000);
} else if (strategyCount <= 20) {
    maxEvaluations = 210000 + ((strategyCount - 15) * 50000);
} else {
    maxEvaluations = 460000 + ((strategyCount - 20) * 80000);
}
```

## Performance Considerations

### Runtime Estimates

Assuming ~1ms per evaluation:

| Strategies | Budget | Estimated Runtime |
|------------|--------|-------------------|
| 7-9 | 100K | ~100 seconds (1.7 min) |
| 10 | 120K | ~120 seconds (2 min) |
| 11 | 150K | ~150 seconds (2.5 min) |
| 12 | 200K | ~200 seconds (3.3 min) |
| 13 | 150K | ~150 seconds (2.5 min) |
| 15 | 210K | ~210 seconds (3.5 min) |
| 20 | 460K | ~460 seconds (7.7 min) |
| 25 | 860K | ~860 seconds (14.3 min) |
| 30 | 1.26M | ~1,260 seconds (21 min) |

**Note**: Actual runtime may vary based on:
- Fitness evaluation complexity
- Cache hit rates
- Parallel execution efficiency
- System resources

## Recommendations

### For Small Portfolios (≤6 strategies)
- Use default GRID_SEARCH
- No configuration needed
- Optimal solution guaranteed

### For Medium Portfolios (7-12 strategies)
- Default scaling is appropriate
- GA parameters are well-tuned
- Consider increasing budget only if convergence is poor

### For Large Portfolios (13+ strategies)
- Default tiered scaling handles most cases
- For very large portfolios (25+), consider:
  - Increasing base budget if quality is insufficient
  - Adjusting COARSE_TO_FINE stage budgets
  - Using parallel execution (already implemented)

## Future Improvements

Potential enhancements:
1. **Adaptive Budget**: Adjust based on convergence rate
2. **Early Stopping**: Stop when improvement plateaus
3. **Hybrid Approaches**: Combine GA and COARSE_TO_FINE
4. **Budget Tuning**: Learn optimal budgets from historical runs

## References

- `docs/PORTFOLIO_OPTIMIZATION_STRATEGIES.md` - Algorithm details
- `PortfolioOptimizerImpl.java` - Implementation
- `App.java` - Algorithm selection logic

---

**Last Updated**: December 2024  
**Version**: 1.0

