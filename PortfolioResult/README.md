# PortfolioResult

A Java-based portfolio analysis and optimization tool for trading strategies. This application processes backtest results from multiple trading strategies, combines them into portfolios, and performs risk-based optimization to find optimal strategy allocations.

## Overview

PortfolioResult is designed to:
- Aggregate results from multiple trading strategies
- Adjust position sizes based on risk parameters
- Generate portfolio-level statistics and reports
- Optimize strategy allocations using brute-force parameter search
- Compare live MT4 orders against backtested results
- Convert and merge historical rate data between formats

## Project Structure

```
PortfolioResult/
├── src/
│   ├── main/
│   │   ├── java/
│   │   │   ├── PortfolioResult/PortfolioResult/
│   │   │   │   ├── App.java              # Main entry point
│   │   │   │   └── ConfigReader.java     # Configuration reader
│   │   │   ├── model/                    # Data models
│   │   │   │   ├── DailyCheck.java
│   │   │   │   ├── ModelData.java        # Core data model
│   │   │   │   ├── Rates.java            # OHLCV rate data
│   │   │   │   ├── Results.java          # Trade result data
│   │   │   │   └── Statistics.java       # Portfolio statistics
│   │   │   └── service/                  # Business logic
│   │   │       ├── FileService.java      # CSV I/O operations
│   │   │       └── StatisticsService.java # Statistical calculations
│   │   └── resources/
│   │       └── config.properties         # Application configuration
│   └── test/
│       └── java/                         # Unit tests
├── batch/                                # Batch processing results
├── history/                              # Historical rate data
├── portfoliorisk*.config                 # Risk allocation configs
├── portfolioOptimize*.config             # Optimization configs
└── pom.xml                               # Maven build configuration
```

## Features

### 1. Portfolio Analysis (`run` mode)
Combines multiple strategy backtests into a single portfolio with risk-adjusted position sizing.

**Usage:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar run portfoliorisk1.config false [startDate]
```

**Parameters:**
- `portfoliorisk1.config` - Risk allocation configuration file
- `false` - Skip open order checking (true/false)
- `startDate` (optional) - Start date in YYYY-MM-DD format (default: 2000-01-01)

**Output:**
- `portfolioResult_adjusted.csv` - Combined portfolio equity curve
- `portfolioWeeklyResult_adjusted.csv` - Weekly aggregated results
- `portfolioMonthlyResult_adjusted.csv` - Monthly aggregated results
- `portfolioStatistics_adjusted.csv` - Portfolio performance metrics

### 2. Custom Strategy Combination (`runCustom` mode)
Creates a combined strategy from multiple sub-strategies.

**Usage:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar runCustom Limit [unused] [unused] [startDate]
```

**Supported combinations:**
- `Limit` - Combines limit order strategies (200002, 200003, 200005, 200007, 200009)
- `BTCUSD` - Bitcoin strategies (300002, 200002)
- `GBPJPY` - GBP/JPY strategies (860006, 841005, 860002)
- `XAUUSD` - Gold strategies (860007, 842001, 860003)
- And more...

**Output:**
- `results_[newStrategyId].txt` - Combined strategy results file

### 3. Portfolio Optimization (`optimizer` mode)
Performs brute-force optimization across multiple risk allocation levels.

**Usage:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer portfoliorisk1.config portfolioOptimizePreset1.config portfolioOptimizeFactor1.config [startDate]
```

**Parameters:**
- Config 1: Strategies to optimize
- Config 2: Predefined (fixed) strategy allocations
- Config 3: Factor multipliers for optimization

**Optimization levels:**
```
Risk multipliers: [0.2, 0.4, 1.0, 1.4, 1.8, 2.0, 2.2]
Combinations: 7^6 = ~117,649 portfolio variations tested
```

**Output:**
- `portfolioStatistics_optimize_adjusted.csv` - All tested combinations ranked by performance
- Automatically runs the best-performing combination

### 4. Two-Level Optimization (`optimizerLevel2` mode)
Optimized version for smaller strategy sets (5 strategies max).

**Usage:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar optimizerLevel2
```

**Fixed strategies:** 500002, 400003, 400004, 860011, 900002

### 5. Historical Data Processing

**MT4 Rate Conversion:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar MT4Rate mt4_file.csv nts_file.csv
```
Converts MT4 CSV format to NTS (internal) format.

**MT4 Rate Merging:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar MT4RateMerge existing.csv new.csv error.txt timeframe
```
Merges new historical data with existing data, with validation.

**Symbol-specific timezone adjustments:**
```bash
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar XAGUSD_60M
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar USTEC_60M
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar EURUSD_5M
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar EURGBP_5M
```

## Configuration Files

### Risk Configuration (`portfoliorisk*.config`)
Defines strategy risk allocations:

```csv
StrategyID,Risk
841005,0.2
860006,2.0
860007,1.0
```

- **StrategyID**: Unique strategy identifier matching `results_[ID].txt` files
- **Risk**: Relative risk allocation (position size multiplier)

### Input Files
Expected in `Batch/` directory:
- `results_[strategyID].txt` - Individual strategy backtest results

### Output Files
All generated to `Batch/` directory:
- Portfolio equity curves (CSV)
- Weekly/monthly aggregations (CSV)
- Statistics and rankings (CSV)

## Building the Project

### Prerequisites
- Java 8 or higher
- Maven 3.x

### Build Commands

**Clean and package:**
```bash
mvn clean package
```

**Run tests:**
```bash
mvn test
```

**Generate JavaDoc:**
```bash
mvn javadoc:javadoc
```

### Output Artifacts
- `target/PortfolioResult-1.0.0.jar` - Main JAR (requires dependencies)
- `target/PortfolioResult-1.0.0-jar-with-dependencies.jar` - Standalone executable JAR
- `target/lib/` - Dependency JARs

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| joda-time | 2.2 | Date/time operations |
| opencsv | 4.3 | CSV file parsing |
| log4j | 1.2.17 | Logging framework |
| junit | 3.8.1 | Testing framework |
| mockito-all | 1.10.19 | Mocking for tests |

## Usage Examples

### Example 1: Basic Portfolio Analysis
```bash
# Analyze portfolio with fixed risk allocations
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar run portfoliorisk1.config false

# Same, but from Jan 1, 2020 onwards
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar run portfoliorisk1.config false 2020-01-01
```

### Example 2: Portfolio Optimization
```bash
# Optimize 6 strategies with predefined allocations
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer \
  portfoliorisk1.config \
  portfolioOptimizePreset1.config \
  portfolioOptimizeFactor1.config
```

### Example 3: Create Combined Strategy
```bash
# Create a new "Limit" strategy from 5 sub-strategies
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar runCustom Limit x x
```

### Example 4: Live Order Checking
```bash
# Run portfolio and compare with live MT4 orders
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar run portfoliorisk1.config true
```
Requires:
- `history/OpenOrders.csv` - Current MT4 open orders
- `history/ClosedOrders.csv` - Recent MT4 closed orders
- `history/Equity.csv` - Current MT4 equity

## Key Metrics Calculated

**Portfolio Statistics:**
- Total Return
- Sharpe Ratio
- Maximum Drawdown
- Win Rate
- Profit Factor
- Average Trade
- Total Trades
- Risk-Adjusted Return

**Time-Based Analysis:**
- Daily equity curve
- Weekly aggregations
- Monthly aggregations
- Yearly performance

## Integration with AutoBBS

This tool is designed to work with the AutoBBS backtesting framework:

1. **Input**: AutoBBS generates `results_[strategyID].txt` files from CTester
2. **Processing**: PortfolioResult combines and optimizes these results
3. **Output**: Optimized risk allocations for live trading deployment

**Typical Workflow:**
```bash
# 1. Run AutoBBS backtests
cd /path/to/AutoBBS/ctester
./scripts/run_strategy1.sh
./scripts/run_strategy2.sh

# 2. Copy results to PortfolioResult
cp tmp/*/results_*.txt /path/to/PortfolioResult/Batch/

# 3. Run portfolio optimization
cd /path/to/PortfolioResult
java -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer ...

# 4. Review optimized allocations
# 5. Deploy to live trading with optimized risk levels
```

## Performance Considerations

**Optimization Runtime:**
- 5 strategies × 5 risk levels = 3,125 combinations (~1-2 minutes)
- 6 strategies × 7 risk levels = 117,649 combinations (~15-30 minutes)
- 7 strategies × 7 risk levels = 823,543 combinations (~1-2 hours)

**Memory Usage:**
- Typical: ~200-500 MB for standard portfolio analysis
- Large optimizations: ~1-2 GB for 100K+ combinations

**Recommendations:**
- Use `optimizerLevel2` for smaller strategy sets (faster)
- Filter by date range to reduce data volume
- Consider parallel processing for large optimizations (future enhancement)

## Troubleshooting

### Common Issues

**1. FileNotFoundException for results files**
```
Error: Could not find results_860006.txt
```
**Solution:** Ensure all strategy result files exist in `Batch/` directory.

**2. Date parsing errors**
```
Error: Unparseable date: "2020/01/01"
```
**Solution:** Use YYYY-MM-DD format (2020-01-01).

**3. OutOfMemoryError during optimization**
```
java.lang.OutOfMemoryError: Java heap space
```
**Solution:** Increase heap size:
```bash
java -Xmx2g -jar target/PortfolioResult-1.0.0-jar-with-dependencies.jar optimizer ...
```

**4. Wrong timezone for commodity data**
```
Warning: Rates not aligned properly
```
**Solution:** Use commodity-specific adjustment methods (`adjustRates_XAUUSD_60M`, etc.)

## Testing

**Run all tests:**
```bash
mvn test
```

**Current test coverage:**
- Basic AppTest (placeholder)
- Service layer tests recommended (future work)

## Logging

Configured via Log4j (1.2.17):
- Console output for immediate feedback
- File logging to `logs/` directory (if configured)
- Log levels: ERROR, WARN, INFO, DEBUG

**To enable debug logging:**
Create `log4j.properties` in `src/main/resources/`:
```properties
log4j.rootLogger=DEBUG, console
log4j.appender.console=org.apache.log4j.ConsoleAppender
log4j.appender.console.layout=org.apache.log4j.PatternLayout
log4j.appender.console.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n
```

## Future Enhancements & Recommendations

See [UPGRADE_RECOMMENDATIONS.md](./UPGRADE_RECOMMENDATIONS.md) for detailed upgrade path.

## License

[Specify license here]

## Contributors

- Original Author: amo3167
- Maintainer: [Your name]

## Support

For issues or questions:
1. Check this README
2. Review [UPGRADE_RECOMMENDATIONS.md](./UPGRADE_RECOMMENDATIONS.md)
3. Contact project maintainer
