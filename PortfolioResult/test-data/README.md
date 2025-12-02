# Regression Test Data

This directory contains baseline (golden) test data for regression testing.

## Directory Structure

```
test-data/
├── baseline/              # Golden dataset (baseline results)
│   ├── baseline_portfolio_stats.csv
│   ├── baseline_optimization_results.csv
│   └── output/            # Logs from baseline creation
└── current/               # Current test results (for comparison)
    ├── current_portfolio_stats.csv
    ├── current_optimization_results.csv
    └── output/            # Logs from current test runs
```

## Usage

### Step 1: Create Baseline (Run Once)

Create the golden dataset from known-good results:

```bash
./scripts/run_regression_tests.sh --create-baseline
```

This will:
- Run portfolio simulation with `portfoliorisk5.config`
- Run optimization with `portfolioOptimize5.config`
- Save results to `baseline/` directory
- Generate baseline files for comparison

**Important**: Only create baseline when you have verified the results are correct!

### Step 2: Run Regression Tests

Compare current results against baseline:

```bash
./scripts/run_regression_tests.sh --test
```

Or simply:

```bash
./scripts/run_regression_tests.sh
```

This will:
- Run the same tests as baseline creation
- Compare results using `scripts/compare_results.py`
- Report any mismatches exceeding 0.01% tolerance
- Exit with code 0 if all tests pass, 1 if any fail

## Test Cases

### Test Case 1: Basic Portfolio
- **Config**: `portfolioRiskConfig/portfoliorisk5.config`
- **Start Date**: 2018-01-01
- **Output**: Portfolio statistics CSV
- **Validates**: Portfolio simulation accuracy

### Test Case 2: Optimization
- **Config**: `portfolioRiskConfig/portfolioOptimize5.config`
- **Preset**: `portfolioRiskConfig/portfolioOptimizePreset5.config`
- **Factors**: `portfolioRiskConfig/portfolioOptimizeFactor5.config`
- **Start Date**: 2018-01-01
- **Output**: Optimization results CSV
- **Validates**: Optimization algorithm correctness

## Comparison Tolerance

- **Default Tolerance**: 0.01% (0.0001)
- **Purpose**: Accounts for floating-point precision differences
- **Adjustable**: Pass tolerance as 5th argument to `compare_results.py`

## Metrics Compared

### Portfolio Statistics
- TotalReturn
- CAGR
- Max_DD
- Sharpe Ratio
- Martin Ratio
- Ulcer Index
- Profit Factor
- Win Rate
- And all other numeric fields

### Optimization Results
- Best portfolio metrics (first row)
- Total number of results
- Key performance indicators

## Updating Baseline

If code changes are intentional and results are expected to change:

1. **Review Changes**: Ensure changes are intentional
2. **Verify Results**: Manually verify new results are correct
3. **Update Baseline**: Run `--create-baseline` again
4. **Commit**: Commit updated baseline files to version control

## Troubleshooting

### Baseline Files Not Found
```bash
Error: Baseline files not found!
Run with --create-baseline first
```
**Solution**: Run `./scripts/run_regression_tests.sh --create-baseline`

### Mismatches Found
If tests fail:
1. Review the mismatch details in the output
2. Check if differences are expected (code changes)
3. If expected, update baseline
4. If unexpected, investigate the code changes

### Files Not Generated
- Check log files in `baseline/output/` or `current/output/`
- Verify JAR file exists: `target/PortfolioResult-1.0.0-jar-with-dependencies.jar`
- Verify config files exist in `portfolioRiskConfig/`

## Integration with CI/CD

To integrate with CI/CD pipeline:

```bash
# In CI script
./scripts/run_regression_tests.sh --test
if [ $? -ne 0 ]; then
    echo "Regression tests failed!"
    exit 1
fi
```

## Notes

- Baseline files should be committed to version control
- Do NOT commit current test results (they're temporary)
- Baseline represents the "golden" state of the system
- Update baseline only when results are intentionally changed

