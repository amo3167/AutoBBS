# PortfolioResult - Technical Specification

## Document Information
- **Version:** 1.0.0
- **Last Updated:** November 30, 2025
- **Status:** Active
- **Owner:** AutoBBS Trading Systems

---

## 1. Project Overview

### 1.1 Purpose
PortfolioResult is a portfolio analysis and optimization tool designed to:
- Aggregate multiple trading strategy backtests into unified portfolios
- Perform risk-based position sizing adjustments
- Optimize strategy allocations using brute-force parameter search
- Generate comprehensive portfolio performance reports
- Validate live trading execution against backtested results

### 1.2 Scope
**In Scope:**
- Portfolio construction from individual strategy results
- Risk-weighted position sizing
- Multi-dimensional optimization (up to 7 strategies)
- Statistical analysis and reporting (Sharpe ratio, drawdown, etc.)
- Historical data format conversion (MT4 ↔ NTS)
- Live order validation against backtest expectations

**Out of Scope:**
- Real-time trading execution
- Market data fetching
- Strategy development/backtesting (handled by AutoBBS)
- Risk management during live trading
- Trade order routing

### 1.3 Integration Points
- **Input:** AutoBBS CTester `results_[ID].txt` files
- **Output:** Portfolio statistics, equity curves, optimized allocations
- **External Systems:** MT4 (for live order validation)

---

## 2. System Architecture

### 2.1 High-Level Architecture
```
┌─────────────────────────────────────────────────────────┐
│                    AutoBBS CTester                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐             │
│  │Strategy A│  │Strategy B│  │Strategy C│  ...        │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘             │
│       │             │             │                     │
│       ▼             ▼             ▼                     │
│  results_A.txt results_B.txt results_C.txt             │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │      PortfolioResult              │
        │  ┌─────────────────────────────┐  │
        │  │  1. Read Individual Results │  │
        │  │  2. Apply Risk Adjustments  │  │
        │  │  3. Combine into Portfolio  │  │
        │  │  4. Calculate Statistics    │  │
        │  │  5. Optimize Allocations    │  │
        │  └─────────────────────────────┘  │
        └───────────────┬───────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────┐
        │           Outputs                 │
        │  • portfolioResult_adjusted.csv   │
        │  • portfolioStatistics_*.csv      │
        │  • Weekly/Monthly reports         │
        │  • Optimized risk allocations     │
        └───────────────────────────────────┘
```

### 2.2 Component Architecture
```
PortfolioResult/
├── CLI Layer (App.java)
│   ├── Command parsing
│   ├── Configuration loading
│   └── Workflow orchestration
│
├── Service Layer
│   ├── FileService
│   │   ├── CSV reading/writing
│   │   ├── Format conversions
│   │   └── Report generation
│   └── StatisticsService
│       ├── Portfolio calculations
│       ├── Performance metrics
│       └── Risk analytics
│
├── Model Layer
│   ├── ModelData (portfolio state)
│   ├── Results (trade data)
│   ├── Statistics (metrics)
│   └── Rates (price data)
│
└── Configuration
    ├── config.properties
    └── Risk allocation configs
```

---

## 3. Data Models

### 3.1 Results Model
Represents individual trade execution data.

```java
class Results {
    String strategyID;        // Unique strategy identifier (e.g., "860006")
    Date openTime;            // Trade entry timestamp
    Date closeTime;           // Trade exit timestamp
    double openPrice;         // Entry price
    double closePrice;        // Exit price (0 if still open)
    double stopLossPrice;     // Stop loss level
    double takeProfitPrice;   // Take profit level
    double lots;              // Position size
    double profit;            // P&L in base currency (0 if open)
    double balance;           // Account balance after trade
    String orderType;         // "BUY" or "SELL"
    String symbol;            // Trading pair (e.g., "GBPJPY")
}
```

### 3.2 Statistics Model
Portfolio performance metrics.

```java
class Statistics {
    Map<String, Double> strategyRisk;  // Risk allocation per strategy
    double totalReturn;                // Total profit
    double sharpeRatio;                // Risk-adjusted return
    double maxDrawdown;                // Largest peak-to-trough decline
    double winRate;                    // Percentage of winning trades
    double profitFactor;               // Gross profit / Gross loss
    double averageTrade;               // Mean profit per trade
    int totalTrades;                   // Trade count
    Date startDate;                    // Analysis start
    Date endDate;                      // Analysis end
    // ... additional metrics
}
```

### 3.3 Rates Model
OHLCV price data for historical analysis.

```java
class Rates {
    Date datetime;        // Bar timestamp
    double open;          // Open price
    double high;          // High price
    double low;           // Low price
    double close;         // Close price
    long volume;          // Volume (if available)
}
```

---

## 4. Core Algorithms

### 4.1 Portfolio Construction

**Input:** 
- Individual strategy results: `List<Results>` per strategy
- Risk allocations: `Map<String, Double>` (strategyID → risk multiplier)

**Process:**
1. **Load Results:** Read each `results_[ID].txt` file
2. **Adjust Position Sizes:** Multiply each strategy's lot size by its risk allocation
3. **Merge Trades:** Combine all adjusted trades chronologically
4. **Calculate Running Balance:** Compute cumulative equity curve
5. **Generate Reports:** Weekly/monthly aggregations

**Algorithm:**
```
for each strategy S:
    results_S = readCSV("results_" + S.id + ".txt")
    riskMultiplier = riskAllocations[S.id]
    
    for each trade T in results_S:
        T.lots = T.lots * riskMultiplier
        T.profit = T.profit * riskMultiplier
    
    mergedResults.addAll(results_S)

mergedResults.sortBy(openTime)
balance = INIT_BALANCE

for each trade T in mergedResults:
    balance = balance + T.profit
    T.balance = balance
```

### 4.2 Portfolio Optimization

**Objective:** Find risk allocation that maximizes Sharpe ratio (or custom metric).

**Method:** Brute-force grid search

**Algorithm:**
```
riskLevels = [0.2, 0.4, 1.0, 1.4, 1.8, 2.0, 2.2]
strategies = [S1, S2, S3, S4, S5, S6]
bestResult = null
maxSharpe = -∞

for r1 in riskLevels:
    for r2 in riskLevels:
        for r3 in riskLevels:
            for r4 in riskLevels:
                for r5 in riskLevels:
                    for r6 in riskLevels:
                        allocation = {S1: r1, S2: r2, ..., S6: r6}
                        portfolio = constructPortfolio(allocation)
                        stats = calculateStatistics(portfolio)
                        
                        if stats.sharpeRatio > maxSharpe:
                            maxSharpe = stats.sharpeRatio
                            bestResult = stats

return bestResult.allocation
```

**Complexity:** O(L^S) where L = risk levels, S = strategies
- 5 strategies × 5 levels = 3,125 combinations
- 6 strategies × 7 levels = 117,649 combinations
- 7 strategies × 7 levels = 823,543 combinations

### 4.3 Sharpe Ratio Calculation

**Formula:**
```
Sharpe Ratio = (Mean Return - Risk-Free Rate) / Standard Deviation of Returns

Where:
- Mean Return: Average daily/weekly/monthly return
- Risk-Free Rate: Typically 0% for trading strategies
- Standard Deviation: Volatility of returns
```

**Implementation:**
```java
double calculateSharpeRatio(List<Double> returns) {
    double mean = returns.stream()
        .mapToDouble(Double::doubleValue)
        .average()
        .orElse(0.0);
    
    double variance = returns.stream()
        .mapToDouble(r -> Math.pow(r - mean, 2))
        .average()
        .orElse(0.0);
    
    double stdDev = Math.sqrt(variance);
    
    return stdDev > 0 ? mean / stdDev : 0.0;
}
```

### 4.4 Maximum Drawdown Calculation

**Definition:** Largest peak-to-trough decline in equity curve.

**Algorithm:**
```java
double calculateMaxDrawdown(List<Double> equityCurve) {
    double maxDrawdown = 0.0;
    double peak = equityCurve.get(0);
    
    for (double equity : equityCurve) {
        if (equity > peak) {
            peak = equity;
        }
        
        double drawdown = (peak - equity) / peak;
        maxDrawdown = Math.max(maxDrawdown, drawdown);
    }
    
    return maxDrawdown * 100; // Return as percentage
}
```

---

## 5. File Formats

### 5.1 Input: Strategy Results (`results_[ID].txt`)

**Format:** CSV with headers
```csv
OpenTime,CloseTime,OpenPrice,ClosePrice,StopLoss,TakeProfit,Lots,Profit,Balance,OrderType,Symbol,StrategyID
2020-01-02 00:05:00,2020-01-02 08:15:00,1.3045,1.3067,1.3025,1.3085,0.10,22.00,10022.00,BUY,GBPUSD,860006
2020-01-02 14:30:00,2020-01-02 20:45:00,1.3078,1.3055,1.3098,1.3045,0.10,-23.00,9999.00,SELL,GBPUSD,860006
```

**Fields:**
- `OpenTime`: Entry timestamp (YYYY-MM-DD HH:MM:SS)
- `CloseTime`: Exit timestamp (0 or blank if still open)
- `OpenPrice`: Entry price
- `ClosePrice`: Exit price (0 if open)
- `StopLoss`: Stop loss level
- `TakeProfit`: Take profit level
- `Lots`: Position size (standard lots)
- `Profit`: P&L in base currency (0 if open)
- `Balance`: Account balance after trade
- `OrderType`: "BUY" or "SELL"
- `Symbol`: Trading pair
- `StrategyID`: Unique identifier

### 5.2 Configuration: Risk Allocation (`portfoliorisk*.config`)

**Format:** CSV with headers
```csv
StrategyID,Risk
841005,0.2
860006,2.0
860007,1.0
860011,0.4
```

**Fields:**
- `StrategyID`: Must match results file suffix
- `Risk`: Multiplier (1.0 = base case, 2.0 = double size, 0.5 = half size)

### 5.3 Output: Portfolio Statistics (`portfolioStatistics_adjusted.csv`)

**Format:** CSV with multiple columns
```csv
StrategyAllocation,TotalReturn,SharpeRatio,MaxDrawdown,WinRate,ProfitFactor,AvgTrade,TotalTrades
841005:0.2|860006:2.0|860007:1.0,15234.50,1.85,18.5,68.5,2.15,87.45,174
```

### 5.4 Output: Equity Curve (`portfolioResult_adjusted.csv`)

**Format:** CSV with daily balances
```csv
Date,Balance,DailyProfit,RunningProfit
2020-01-02,10050.00,50.00,50.00
2020-01-03,10125.00,75.00,125.00
2020-01-04,10085.00,-40.00,85.00
```

---

## 6. Operational Modes

### 6.1 Mode: `run`
**Purpose:** Analyze portfolio with fixed risk allocations.

**Command:**
```bash
java -jar PortfolioResult.jar run portfoliorisk1.config false [startDate]
```

**Workflow:**
1. Read risk allocation config
2. Load strategy result files
3. Apply risk adjustments
4. Merge into portfolio
5. Calculate statistics
6. Generate reports

**Outputs:**
- `portfolioResult_adjusted.csv`
- `portfolioWeeklyResult_adjusted.csv`
- `portfolioMonthlyResult_adjusted.csv`
- `portfolioStatistics_adjusted.csv`

### 6.2 Mode: `optimizer`
**Purpose:** Find optimal risk allocations via brute-force search.

**Command:**
```bash
java -jar PortfolioResult.jar optimizer portfoliorisk1.config preset1.config factor1.config [startDate]
```

**Workflow:**
1. Read strategy list to optimize
2. Read predefined (fixed) strategies
3. Generate all risk combinations
4. Test each combination
5. Rank by Sharpe ratio
6. Run best combination
7. Output all results

**Outputs:**
- `portfolioStatistics_optimize_adjusted.csv` (all combinations)
- Standard portfolio outputs for best combination

### 6.3 Mode: `runCustom`
**Purpose:** Create combined strategy from sub-strategies.

**Command:**
```bash
java -jar PortfolioResult.jar runCustom BTCUSD x x [startDate]
```

**Predefined Combinations:**
- `Limit`: 200002, 200003, 200005, 200007, 200009
- `BTCUSD`: 300002, 200002
- `GBPJPY`: 860006, 841005, 860002
- `XAUUSD`: 860007, 842001, 860003

**Workflow:**
1. Load predefined strategy combination
2. Merge sub-strategy results
3. Output as single new strategy
4. Generate portfolio reports

**Outputs:**
- `results_[newStrategyID].txt` (combined strategy)
- Standard portfolio reports

### 6.4 Mode: `MT4Rate` / `MT4RateMerge`
**Purpose:** Historical data format conversion and merging.

**Commands:**
```bash
# Convert MT4 → NTS format
java -jar PortfolioResult.jar MT4Rate input.csv output.csv

# Merge new data into existing
java -jar PortfolioResult.jar MT4RateMerge existing.csv new.csv error.txt timeframe
```

---

## 7. Performance Specifications

### 7.1 Runtime Requirements
| Operation | Data Size | Expected Time |
|-----------|-----------|---------------|
| Portfolio Analysis | 1000 trades/strategy, 5 strategies | < 5 seconds |
| Optimization (5 strategies) | 3,125 combinations | < 2 minutes |
| Optimization (6 strategies) | 117,649 combinations | < 30 minutes |
| Data Conversion | 100,000 OHLCV bars | < 10 seconds |

### 7.2 Memory Requirements
| Operation | Heap Size | Notes |
|-----------|-----------|-------|
| Standard Portfolio | 256 MB | 5-10 strategies |
| Large Optimization | 1-2 GB | 100K+ combinations |
| Data Conversion | 512 MB | Large history files |

### 7.3 Scalability Limits
- **Max Strategies (optimizer):** 7 (823K combinations, ~2 hours)
- **Max Trades per Strategy:** 100,000 (memory constraint)
- **Max Portfolio History:** 20 years (typical: 5-10 years)

---

## 8. Quality Attributes

### 8.1 Accuracy
- **Calculation Precision:** Double (64-bit floating point)
- **Rounding:** 2 decimal places for currency, 4-5 for FX rates
- **Validation:** Results must match AutoBBS CTester within 0.01% tolerance

### 8.2 Reliability
- **Data Integrity:** CSV parsing with error handling
- **Validation:** Check for missing files, corrupt data
- **Recovery:** Log errors, continue with available data

### 8.3 Maintainability
- **Code Structure:** Service-oriented architecture
- **Documentation:** JavaDoc for all public methods
- **Testing:** Unit tests for critical calculations

### 8.4 Usability
- **CLI Interface:** Simple command-line arguments
- **Configuration:** CSV files (Excel-compatible)
- **Outputs:** Standard CSV format for analysis

---

## 9. Constraints & Assumptions

### 9.1 Technical Constraints
- **Java Version:** Java 8+ (currently targeting Java 8)
- **Dependencies:** Minimal (OpenCSV, Joda-Time/java.time, Log4j)
- **Platform:** Cross-platform (Windows, macOS, Linux)

### 9.2 Business Constraints
- **Input Format:** Fixed CSV structure from AutoBBS
- **Strategy IDs:** Must be unique integers
- **Currency:** Results in account base currency

### 9.3 Assumptions
- **Trade Data Quality:** Clean, complete data from backtests
- **Chronological Order:** Trades within reasonable time sequence
- **No Overlapping IDs:** Strategy IDs are unique across all files
- **Risk-Free Rate:** Assumed 0% for Sharpe ratio calculations

---

## 10. Security Considerations

### 10.1 Data Security
- **Access Control:** File system permissions
- **Data Storage:** Local filesystem only (no network/cloud)
- **Sensitive Data:** No passwords or API keys

### 10.2 Dependency Security
- **Log4j:** **CRITICAL** - Version 1.2.17 has known vulnerabilities
  - **Action Required:** Upgrade to Log4j 2.23.0+
- **Other Dependencies:** Review regularly for CVEs

---

## 11. Future Enhancements

### 11.1 Planned Features
- **Parallel Optimization:** Multi-threaded brute-force search
- **Advanced Metrics:** Sortino ratio, Calmar ratio, Monte Carlo simulation
- **Walk-Forward Analysis:** Out-of-sample validation
- **Correlation Analysis:** Strategy correlation matrix
- **Risk Parity:** Advanced allocation methods beyond fixed risk

### 11.2 Technical Debt
- **Test Coverage:** Current <10%, target 70%+
- **Hardcoded Paths:** Windows-specific paths in code
- **God Class:** App.java is 1000+ lines
- **Legacy Dependencies:** Log4j 1.2, JUnit 3, Joda-Time

---

## 12. References

### 12.1 Related Documents
- [README.md](../README.md) - User documentation
- [UPGRADE_RECOMMENDATIONS.md](../UPGRADE_RECOMMENDATIONS.md) - Technical upgrade path
- [TASKS.md](./TASKS.md) - Implementation task list

### 12.2 External Resources
- AutoBBS CTester Framework Documentation
- OpenCSV Library: http://opencsv.sourceforge.net/
- Java Time API: https://docs.oracle.com/javase/8/docs/api/java/time/package-summary.html

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2025-11-30 | GitHub Copilot | Initial specification |

