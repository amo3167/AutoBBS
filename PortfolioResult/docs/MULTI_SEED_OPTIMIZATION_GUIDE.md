# Multi-Seed Optimization Guide

## Overview

The `run_multiple_seeds.sh` script runs optimization with multiple random seeds (default: 1-10) and automatically finds the best result. This approach:

- ✅ **Explores different regions** of the solution space
- ✅ **Finds better solutions** than single-run optimization
- ✅ **Maintains reproducibility** (each seed is deterministic)
- ✅ **Automatically compares** and ranks all results

---

## Usage

### Basic Usage (Default: Config 5, 10 seeds)

```bash
./run_multiple_seeds.sh
```

### Custom Configuration

```bash
# Use config 7, default start date, 10 seeds
./run_multiple_seeds.sh 7

# Use config 5, custom start date, 10 seeds
./run_multiple_seeds.sh 5 2020-01-01

# Use config 5, custom start date, 20 seeds
./run_multiple_seeds.sh 5 2018-01-01 20
```

### Parameters

1. **Config Number** (optional, default: 5)
   - Uses `portfolioOptimize{num}.config`
   - Example: `5` → `portfolioOptimize5.config`

2. **Start Date** (optional, default: 2018-01-01)
   - Format: YYYY-MM-DD
   - Example: `2020-01-01`

3. **Number of Seeds** (optional, default: 10)
   - Runs optimization with seeds 1 to N
   - Example: `20` → runs seeds 1-20

---

## What the Script Does

1. **Runs Optimization** for each seed (1 to N)
2. **Collects Results** from each run
3. **Extracts Statistics** (Total Return, Martin Ratio, CAGR, Max DD)
4. **Ranks Results** by Martin Ratio (primary) and Total Return (secondary)
5. **Reports Best Result** with detailed statistics
6. **Saves All Results** to a timestamped directory

---

## Output

### Console Output

The script displays:
- Progress for each seed
- Top 5 results (sorted by Martin Ratio)
- Best result with full details
- Improvement over worst result

**Example Output**:
```
TOP 5 RESULTS (sorted by Martin Ratio):
----------------------------------------------------------------------------------------------------
Rank   Seed   Martin Ratio    Total Return    CAGR         Max DD      
----------------------------------------------------------------------------------------------------
1      7      21.950000       73.250000       73.25        12.93       
2      3      21.920000       65.450000       70.78        12.84       
3      5      21.900000       61.550000       69.44        12.61       
4      1      21.850000       59.770000       68.80        12.61       
5      9      21.820000       58.030000       68.16        12.61       

====================================================================================================
BEST RESULT:
====================================================================================================
  Seed: 7
  Martin Ratio: 21.950000
  Total Return: 73.250000x
  CAGR: 73.25%
  Max Drawdown: 12.93%
  Output Directory: batch/output/multi_seed_20251201_180000/seed_7
  Best Portfolio: batch/output/multi_seed_20251201_180000/seed_7/best_portfolio.csv
```

### Files Created

All results are saved to: `batch/output/multi_seed_YYYYMMDD_HHMMSS/`

**Files**:
- `summary.csv` - All results summary
- `best_results.csv` - Top 10 results ranked
- `seed_1/` through `seed_N/` - Complete output for each seed
- `seed_N.log` - Log file for each seed

---

## Understanding the Results

### Ranking Criteria

Results are ranked by:
1. **Martin Ratio** (primary) - Higher is better
2. **Total Return** (secondary) - Higher is better

**Why Martin Ratio?**
- Martin Ratio = CAGR / Ulcer Index
- Balances return against risk-adjusted drawdown
- Best single metric for portfolio quality

### Interpreting Results

**Good Results**:
- Martin Ratio > 20: Excellent
- Martin Ratio 15-20: Very Good
- Martin Ratio 10-15: Good

**Improvement**:
- If best seed has 5%+ better Martin Ratio than worst → Good exploration
- If all seeds similar → Solution space is well-explored

---

## Example Workflow

### Step 1: Run Multi-Seed Optimization

```bash
./run_multiple_seeds.sh 5 2018-01-01 10
```

### Step 2: Review Results

```bash
# View summary
cat batch/output/multi_seed_*/summary.csv

# View best results
cat batch/output/multi_seed_*/best_results.csv

# View best portfolio
cat batch/output/multi_seed_*/seed_*/best_portfolio.csv
```

### Step 3: Use Best Result

The script identifies the best seed. You can:
- Use that seed for future runs (reproducible)
- Copy the best portfolio configuration
- Analyze why that seed found a better solution

---

## Performance Considerations

### Time

- **Per Seed**: ~15-30 seconds (depends on portfolio size)
- **10 Seeds**: ~3-5 minutes total
- **20 Seeds**: ~6-10 minutes total

### Memory

- Uses `-Xmx4g` (4GB heap)
- Each run is independent (memory is freed between runs)

### Disk Space

- Each seed creates ~1-5MB of output
- 10 seeds: ~10-50MB total
- Results are kept in timestamped directories

---

## Tips

### 1. Start with 10 Seeds

10 seeds is usually sufficient to find good solutions:
- Good balance between exploration and time
- Most runs show clear best result

### 2. Increase Seeds for Important Optimizations

For critical optimizations, use 20-30 seeds:
- More thorough exploration
- Higher chance of finding best solution

### 3. Compare Different Start Dates

Run multi-seed with different start dates:
```bash
./run_multiple_seeds.sh 5 2018-01-01 10
./run_multiple_seeds.sh 5 2020-01-01 10
./run_multiple_seeds.sh 5 2021-01-01 10
```

### 4. Track Best Seeds

Keep a log of which seeds perform best:
- Some seeds consistently find better solutions
- May indicate good starting points for GA

---

## Troubleshooting

### Issue: Script Fails on First Seed

**Check**:
- Config files exist
- JAR file is built (`mvn clean package`)
- Sufficient disk space

### Issue: All Seeds Produce Same Result

**Possible Causes**:
- Constraints too strict (all solutions identical)
- Solution space too small
- Cache hit rate very high

**Solution**: Check constraint settings in factor config

### Issue: Results Vary Widely

**This is Normal**:
- Different seeds explore different regions
- Some seeds find better solutions
- This is why multi-seed is valuable!

---

## Comparison with Single Run

| Approach | Pros | Cons |
|----------|------|------|
| **Single Run** | Fast (~30 sec) | May miss better solutions |
| **Multi-Seed (10)** | Finds better solutions | Slower (~5 min) |
| **Multi-Seed (20)** | Very thorough | Slower (~10 min) |

**Recommendation**: Use multi-seed for important optimizations, single run for quick tests.

---

## Advanced Usage

### Run in Background

```bash
nohup ./run_multiple_seeds.sh 5 2018-01-01 20 > multi_seed.log 2>&1 &
```

### Custom Seed Range

Edit the script to use custom seeds:
```bash
# Change this line:
for seed in $(seq 1 $NUM_SEEDS); do

# To:
for seed in 1 5 10 15 20 25 30 35 40 45 50; do
```

### Parallel Execution

For faster execution, modify script to run seeds in parallel (advanced).

---

## Related Scripts

- `test_deterministic_seed.sh` - Verify deterministic behavior
- `run_optimizer.sh` - Single optimization run

---

**Last Updated**: December 2024  
**Related Documents**: 
- `DETERMINISTIC_SEED_TEST_RESULTS.md`
- `NON_DETERMINISTIC_OPTIMIZATION_ISSUE.md`

