# Strategy Results Copy Scripts

Scripts to copy strategy backtest results from `ctester/tmp` to `PortfolioResult/batch` with proper naming conventions.

## Overview

These scripts automate the process of copying strategy results from CTester output directories to the PortfolioResult batch folder for portfolio analysis.

### Files Copied

For each strategy folder (e.g., `AUDUSD_200003`), the script copies:

| Source File | Destination File | Description |
|-------------|------------------|-------------|
| `allStatistics_200003.csv` | `allStatistics_200003.csv` | Strategy statistics summary |
| `results.txt` | `results_200003.txt` | Detailed trade results |
| `results.png` | `results_200003.png` | Equity curve visualization |

The strategy ID (e.g., `200003`) is automatically extracted from the folder name.

## Usage

### Unix/macOS/Linux - Bash Script

**Copy a single strategy:**
```bash
./scripts/copy_strategy_results.sh AUDUSD_200003
```

**Copy all strategies:**
```bash
./scripts/copy_strategy_results.sh
```

**From any directory:**
```bash
cd /path/to/AutoBBS
./scripts/copy_strategy_results.sh GBPJPY_860006
```

### Windows - Batch Script

**Copy a single strategy:**
```cmd
scripts\copy_strategy_results.bat AUDUSD_200003
```

**Copy all strategies:**
```cmd
scripts\copy_strategy_results.bat
```

## Examples

### Example 1: Copy Single Strategy After Backtest

```bash
# Run a backtest
cd ctester
python asirikuy_strategy_tester.py --strategy 200003 --symbol AUDUSD

# Copy results to portfolio
cd ..
./scripts/copy_strategy_results.sh AUDUSD_200003
```

Output:
```
==================================
Strategy Results Copy Script
==================================

Processing: AUDUSD_200003 -> Strategy ID: 200003
  ✓ Copied allStatistics_200003.csv
  ✓ Copied results.txt -> results_200003.txt
  ✓ Copied results.png -> results_200003.png
  ✓ Copied 3 file(s)

Done!
```

### Example 2: Bulk Copy All Strategies

After running multiple backtests:

```bash
./scripts/copy_strategy_results.sh
```

Output:
```
==================================
Strategy Results Copy Script
==================================

Scanning for strategy folders in /path/to/AutoBBS/ctester/tmp

Processing: AUDUSD_200003 -> Strategy ID: 200003
  ✓ Copied 3 file(s)

Processing: BTCUSD_200002 -> Strategy ID: 200002
  ✓ Copied 3 file(s)

Processing: GBPJPY_860006 -> Strategy ID: 860006
  ✓ Copied 3 file(s)

...

==================================
✓ Processed 15 strategy folders
Results copied to: /path/to/AutoBBS/PortfolioResult/batch

Done!
```

### Example 3: Copy Optimization Results

The script handles `_optimize` folders:

```bash
./scripts/copy_strategy_results.sh GBPJPY_860006_optimize
```

The strategy ID (860006) is correctly extracted even with the `_optimize` suffix.

## Supported Folder Naming Patterns

The script automatically detects strategy IDs from these folder name patterns:

- `SYMBOL_STRATEGYID` (e.g., `AUDUSD_200003`)
- `SYMBOL_STRATEGYID_optimize` (e.g., `GBPJPY_860006_optimize`)
- Any folder with a 6-digit strategy ID

Strategy IDs must be exactly 6 digits (e.g., `200003`, `860006`, `900002`).

## Integration with Portfolio Workflow

### Complete Workflow

1. **Run Backtests**
   ```bash
   cd ctester
   python asirikuy_strategy_tester.py --strategy 200003 --symbol AUDUSD
   python asirikuy_strategy_tester.py --strategy 860006 --symbol GBPJPY
   python asirikuy_strategy_tester.py --strategy 860007 --symbol XAUUSD
   ```

2. **Copy Results**
   ```bash
   cd ..
   ./scripts/copy_strategy_results.sh
   ```

3. **Create Portfolio Configuration**
   ```bash
   cd PortfolioResult
   # Edit portfoliorisk1.config with strategy IDs and risk levels
   ```

4. **Run Portfolio Analysis**
   ```bash
   java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar \
     run portfoliorisk1.config false 2020-01-01
   ```

5. **Or Run Optimization**
   ```bash
   java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar \
     optimizer portfoliorisk1.config \
     portfolioOptimizePreset1.config \
     portfolioOptimizeFactor1.config
   ```

## Script Behavior

### File Handling

- **Overwrites existing files**: If `results_200003.txt` already exists, it will be overwritten
- **Creates batch directory**: If `PortfolioResult/batch/` doesn't exist, it will be created automatically
- **Skips missing files**: If a file is missing (e.g., no `results.png`), it's skipped without error
- **Validates directories**: Only processes folders with valid 6-digit strategy IDs

### Error Handling

The script checks for:
- ✓ Valid ctester/tmp directory
- ✓ Valid PortfolioResult directory
- ✓ Valid strategy ID format (6 digits)
- ✓ Source files exist before copying

### Skip Conditions

The script automatically skips:
- Non-directory items
- Folders without 6-digit strategy IDs
- Analysis files (e.g., `GBPUSD_200009_ANALYSIS.md`)
- Python scripts (e.g., `analyze_btcusd.py`)

## Troubleshooting

### "No files found to copy"

**Problem:** Strategy folder exists but no files were copied.

**Solution:** Check that the required files exist:
```bash
ls ctester/tmp/AUDUSD_200003/
# Should show: results.txt, results.png, allStatistics_200003.csv
```

### "Strategy folder not found"

**Problem:** Specified folder doesn't exist.

**Solution:** List available strategy folders:
```bash
ls ctester/tmp/ | grep -E '[0-9]{6}'
```

### "No strategy ID found"

**Problem:** Folder name doesn't contain a valid 6-digit strategy ID.

**Solution:** Ensure folder follows naming convention:
- ✓ `SYMBOL_123456`
- ✓ `SYMBOL_123456_optimize`
- ✗ `SYMBOL_12345` (only 5 digits)
- ✗ `analyze_btcusd.py` (not a strategy folder)

### Permission Denied (Unix/macOS)

**Problem:** Script is not executable.

**Solution:**
```bash
chmod +x scripts/copy_strategy_results.sh
```

## Advanced Usage

### Copy Only Recent Backtests

Copy strategies modified in the last 24 hours:

```bash
find ctester/tmp -type d -mtime -1 -name "*_[0-9][0-9][0-9][0-9][0-9][0-9]" | while read dir; do
    folder=$(basename "$dir")
    ./scripts/copy_strategy_results.sh "$folder"
done
```

### Verify All Results Before Portfolio Run

```bash
# Copy all results
./scripts/copy_strategy_results.sh

# List all copied strategies
cd PortfolioResult/batch
ls results_*.txt | sed 's/results_//;s/.txt//' | sort -n
```

### Batch Process Multiple Symbols

```bash
for symbol in AUDUSD GBPJPY XAUUSD; do
    for strategy_id in 200003 860006 860007; do
        folder="${symbol}_${strategy_id}"
        if [ -d "ctester/tmp/$folder" ]; then
            ./scripts/copy_strategy_results.sh "$folder"
        fi
    done
done
```

## Notes

- The script assumes it's located in the `scripts/` folder at the project root
- Paths are automatically calculated relative to the script location
- Both scripts (bash and batch) provide identical functionality
- Use the bash script on Unix/macOS/Linux, use the batch script on Windows

## See Also

- [PortfolioResult README](../PortfolioResult/README.md) - Portfolio analysis documentation
- [CTester README](../ctester/README.md) - Backtesting framework documentation
- [AutoBBS Documentation](../docs/README.md) - Complete system documentation
