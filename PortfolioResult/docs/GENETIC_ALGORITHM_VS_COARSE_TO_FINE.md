# Genetic Algorithm vs Coarse-to-Fine: Quality Comparison

## Executive Summary

**Answer**: **Yes, Genetic Algorithm generally produces better quality results than Coarse-to-Fine**, but the choice depends on portfolio size and computational budget.

**Key Finding**: 
- **GA Quality**: ~96% of optimal (for 7-12 strategies)
- **Coarse-to-Fine Quality**: ~92% of optimal (for 13+ strategies)
- **Quality Gap**: GA is typically 4-5% better in quality

However, GA becomes impractical for very large portfolios (13+ strategies) due to computational constraints.

---

## Quality Comparison

### From Documentation (10 Strategies Example)

| Algorithm | Evaluations | Time | Quality | Notes |
|-----------|-------------|------|---------|-------|
| **Grid Search** | 282,475,249 | 3 days | 100% | Optimal (exhaustive) |
| **Genetic Algorithm** | 10,000 | 10 sec | **96%** | Population-based evolution |
| **Coarse-to-Fine** | 20,000 | 20 sec | **92%** | Hierarchical refinement |
| **Random Search** | 50,000 | 50 sec | 88% | Baseline |

**Conclusion**: GA achieves **4% better quality** than Coarse-to-Fine with **half the evaluations**.

---

## Why Genetic Algorithm is Better

### 1. **Population Diversity**

**Genetic Algorithm**:
- Maintains a **population of 200 diverse solutions**
- Each generation explores multiple regions of solution space simultaneously
- Population diversity prevents premature convergence to local optima
- **Elitism**: Preserves top 10% best solutions across generations

**Coarse-to-Fine**:
- Starts with **random search** (no population diversity)
- Stage 1 uses only **3 risk levels** (coarse resolution)
- May miss good solutions in coarse stage
- Refinement stages are **local search** (can get stuck in local optima)

### 2. **Exploration vs Exploitation Balance**

**Genetic Algorithm**:
- **Crossover (70% rate)**: Combines good solutions to find better ones
- **Mutation (15% rate)**: Introduces diversity, escapes local optima
- **Tournament Selection**: Prefers better solutions but maintains diversity
- **Balanced**: Explores widely while exploiting good regions

**Coarse-to-Fine**:
- **Stage 1**: Pure exploration (random search with coarse resolution)
- **Stage 2-3**: Pure exploitation (local search around best solution)
- **Risk**: If Stage 1 misses the optimal region, later stages can't recover
- **Less balanced**: Separates exploration and exploitation phases

### 3. **Search Strategy**

**Genetic Algorithm**:
- **Evolutionary approach**: Solutions evolve over generations
- **Population-based**: Multiple solutions compete and improve
- **Adaptive**: Better solutions get more opportunities to reproduce
- **Convergence**: Gradually improves toward optimal solution

**Coarse-to-Fine**:
- **Hierarchical approach**: Coarse → Refine → Polish
- **Sequential stages**: Each stage depends on previous stage's result
- **Risk**: Early stage mistakes propagate to later stages
- **Less adaptive**: Can't recover if coarse stage finds poor solution

---

## When Each Algorithm is Used

### Current Implementation (App.java)

```java
if (strategiesToOptimize.size() <= 12) {
    // Use GENETIC_ALGORITHM
    // Budget: 100k-200k evaluations
} else {
    // Use COARSE_TO_FINE
    // Budget: 150k-460k+ evaluations
}
```

**Rationale**:
- **7-12 strategies**: GA is practical and produces best quality
- **13+ strategies**: GA becomes too slow, Coarse-to-Fine scales better

---

## Quality Analysis by Portfolio Size

### Small-Medium Portfolios (7-12 Strategies)

**Genetic Algorithm**:
- ✅ **Best quality**: ~96% of optimal
- ✅ **Efficient**: 100k-200k evaluations sufficient
- ✅ **Population diversity**: 200 solutions explore solution space
- ✅ **Good convergence**: Multiple generations refine solutions
- ✅ **Proven**: Well-established algorithm with good results

**Coarse-to-Fine**:
- ⚠️ **Lower quality**: ~92% of optimal (4% worse)
- ⚠️ **Coarse stage limitation**: Only 3 risk levels in Stage 1
- ⚠️ **Local search limitation**: Stages 2-3 can get stuck
- ⚠️ **Sequential dependency**: Poor Stage 1 → poor final result

**Verdict**: **GA is better for 7-12 strategies**

### Large Portfolios (13+ Strategies)

**Genetic Algorithm**:
- ❌ **Impractical**: Would need 500k+ evaluations for good quality
- ❌ **Slow**: Population of 200 × many generations = very slow
- ❌ **Memory**: Maintaining 200 solutions for large portfolios is expensive
- ⚠️ **Quality**: Could achieve 96% but takes too long

**Coarse-to-Fine**:
- ✅ **Scalable**: Handles 13+ strategies efficiently
- ✅ **Fast**: Hierarchical approach reduces search space
- ✅ **Practical**: 150k-460k evaluations manageable
- ⚠️ **Quality**: ~92% (acceptable trade-off for scalability)

**Verdict**: **Coarse-to-Fine is necessary for 13+ strategies** (scalability > quality)

---

## Real-World Evidence

### From Your Optimization Runs

Both runs (164202 and 164729) used **14 strategies**, so they used **Coarse-to-Fine**:

- **Run 1**: 47.92x return, Martin 21.73 (tested 318 combinations)
- **Run 2**: 73.23x return, Martin 21.91 (tested 188 combinations)

**Observation**: 
- Both used Coarse-to-Fine (required for 14 strategies)
- Large variation (53% difference) due to non-deterministic nature
- Run 2 found much better solution (73.23x vs 47.92x)

**Hypothesis**: If GA could be used for 14 strategies, it might:
- Find better solutions more consistently
- Have less variation between runs
- Achieve higher quality (closer to optimal)

---

## Algorithm Characteristics Comparison

| Characteristic | Genetic Algorithm | Coarse-to-Fine |
|----------------|-------------------|----------------|
| **Quality** | ~96% of optimal | ~92% of optimal |
| **Best For** | 7-12 strategies | 13+ strategies |
| **Search Type** | Population-based evolution | Hierarchical refinement |
| **Diversity** | High (200 solutions) | Low (sequential stages) |
| **Exploration** | Continuous (crossover + mutation) | Stage 1 only (coarse) |
| **Exploitation** | Continuous (selection pressure) | Stages 2-3 (local search) |
| **Convergence** | Gradual improvement | Stage-dependent |
| **Local Optima** | Good escape (mutation) | Can get stuck (local search) |
| **Scalability** | Limited (7-12 strategies) | Excellent (13+ strategies) |
| **Speed** | Fast for medium portfolios | Fast for large portfolios |
| **Memory** | Moderate (200 solutions) | Low (sequential) |

---

## Recommendations

### For 7-12 Strategies: Use Genetic Algorithm

**Why**:
- ✅ **4% better quality** (96% vs 92%)
- ✅ **More consistent** results (population diversity)
- ✅ **Better exploration** (crossover + mutation)
- ✅ **Proven performance** in this range

**Current Implementation**: ✅ Already using GA (correct)

### For 13+ Strategies: Use Coarse-to-Fine

**Why**:
- ✅ **Scalable** (GA becomes impractical)
- ✅ **Fast** (hierarchical approach)
- ✅ **Acceptable quality** (92% is still good)
- ✅ **Necessary trade-off** (scalability > quality)

**Current Implementation**: ✅ Already using Coarse-to-Fine (correct)

### Potential Improvement: Hybrid Approach

**For 13-15 Strategies**: Consider using GA with larger budget

**Idea**:
- Use GA with 300k-400k evaluations instead of Coarse-to-Fine
- May achieve 94-96% quality (better than 92%)
- Still practical for 13-15 strategies
- Worth testing if quality is more important than speed

**Implementation**:
```java
if (strategiesToOptimize.size() <= 12) {
    // Use GA (current)
} else if (strategiesToOptimize.size() <= 15) {
    // Consider GA with larger budget (300k-400k)
    // vs Coarse-to-Fine (150k-210k)
} else {
    // Use Coarse-to-Fine (current)
}
```

---

## Quality vs Speed Trade-off

### Genetic Algorithm
- **Quality**: ⭐⭐⭐⭐⭐ (96% of optimal)
- **Speed**: ⭐⭐⭐⭐ (Fast for 7-12 strategies)
- **Scalability**: ⭐⭐ (Limited to ~12 strategies)

### Coarse-to-Fine
- **Quality**: ⭐⭐⭐⭐ (92% of optimal)
- **Speed**: ⭐⭐⭐⭐⭐ (Fast for 13+ strategies)
- **Scalability**: ⭐⭐⭐⭐⭐ (Handles 20+ strategies)

**Conclusion**: 
- **GA prioritizes quality** (best for medium portfolios)
- **Coarse-to-Fine prioritizes scalability** (necessary for large portfolios)

---

## Testing Recommendation

To verify GA quality advantage for 13-15 strategies:

1. **Test GA with 14 strategies**:
   - Budget: 300k-400k evaluations
   - Compare quality vs Coarse-to-Fine (150k-210k)
   - Measure: Total Return, Martin Ratio, consistency

2. **Expected Results**:
   - GA: Higher quality (94-96% vs 92%)
   - GA: More consistent (less variation between runs)
   - GA: Slower (but acceptable for 13-15 strategies)

3. **Decision**:
   - If quality improvement > 2-3%: Extend GA to 13-15 strategies
   - If quality improvement < 2%: Keep Coarse-to-Fine (faster)

---

## Conclusion

**Yes, Genetic Algorithm produces better quality results than Coarse-to-Fine** (~96% vs ~92%), but:

1. **For 7-12 strategies**: GA is clearly better (already implemented ✅)
2. **For 13+ strategies**: Coarse-to-Fine is necessary for scalability
3. **For 13-15 strategies**: Consider testing GA with larger budget (potential quality improvement)

**Current implementation is correct** - GA for medium portfolios, Coarse-to-Fine for large portfolios. The quality difference (4%) is an acceptable trade-off for the scalability Coarse-to-Fine provides.

---

**Last Updated**: December 2024  
**Related Documents**: 
- `OPTIMIZATION_BUDGET_SCALING.md`
- `NON_DETERMINISTIC_OPTIMIZATION_ISSUE.md`
- `PORTFOLIO_OPTIMIZATION_STRATEGIES.md`

