# CTesterFrameworkAPI

## Overview

CTesterFrameworkAPI is a shared library that provides backtesting and optimization capabilities for trading strategies. It serves as the bridge between Python test scripts and the Asirikuy Framework, enabling comprehensive strategy testing and parameter optimization.

## Architecture

```
Python Scripts (asirikuy_strategy_tester.py)
    ↓
CTesterFrameworkAPI (libCTesterFrameworkAPI.dylib)
    ↓
AsirikuyFrameworkAPI (libAsirikuyFrameworkAPI.dylib)
    ↓
TradingStrategies (libtrading_strategies.dylib)
```

## Components

### 1. Core API (`CTesterFrameworkAPI.c`)
- **Initialization**: `initCTesterFramework()` - Initializes the framework with logging
- **Version**: `getCTesterFrameworkVersion()` - Returns framework version

### 2. Backtesting (`tester.c`)
- **Main Function**: `runPortfolioTest()` - Executes backtesting for one or more trading systems
- **Features**:
  - Multi-symbol portfolio testing
  - Order management (open, modify, close)
  - Stop loss and take profit handling
  - Swap interest calculation
  - Statistical analysis (CAGR, Sharpe, Max DD, etc.)
  - Real-time progress callbacks

### 3. Optimization (`optimizer.c`)
- **Main Function**: `runOptimizationMultipleSymbols()` - Runs parameter optimization
- **Optimization Types**:
  - **Brute Force**: Tests all parameter combinations
  - **Genetic Algorithm**: Uses GAUL library for evolutionary optimization
- **Features**:
  - Multi-threaded execution (OpenMP)
  - MPI support for distributed computing
  - Multiple optimization goals (Profit, Max DD, Sharpe, CAGR/MaxDD, etc.)
  - Convergence detection
  - Early termination support

### 4. Historical Data (`historics.c`)
- **Function**: `readHistoricFile()` - Reads OHLC data from CSV files
- **Note**: Currently has parsing issues that need to be fixed

## API Reference

### Initialization

```c
int initCTesterFramework(char* pAsirikuyTesterLog, int severityLevel);
```

Initializes the CTester framework with logging.

**Parameters**:
- `pAsirikuyTesterLog`: Path to log file
- `severityLevel`: Log severity (0-7, where 4=Warning, 7=Debug)

**Returns**: Error code (0 = SUCCESS)

### Backtesting

```c
TestResult runPortfolioTest(
    int testId,
    double** pInSettings,
    char** pInTradeSymbol,
    char* pInAccountCurrency,
    char* pInBrokerName,
    char* pInRefBrokerName,
    double** pInAccountInfo,
    TestSettings* testSettings,
    CRatesInfo** pRatesInfo,
    int numCandles,
    int numSystems,
    ASTRates*** pRates,
    double minLotSize,
    void (*testUpdate)(int testId, double percentageOfTestCompleted, COrderInfo lastOrder, double currentBalance, char* symbol),
    void (*testFinished)(TestResult testResults),
    void (*signalUpdate)(TradeSignal signal)
);
```

Executes a backtest for one or more trading systems.

**Returns**: `TestResult` structure containing:
- `totalTrades`: Number of trades executed
- `finalBalance`: Final account balance
- `cagr`: Compound Annual Growth Rate
- `sharpe`: Sharpe ratio
- `maxDDDepth`: Maximum drawdown depth (%)
- `maxDDLength`: Maximum drawdown length (seconds)
- `pf`: Profit factor
- `r2`: R-squared (regression quality)
- `ulcerIndex`: Ulcer index
- `numShorts`, `numLongs`: Trade counts by direction

### Optimization

```c
int runOptimizationMultipleSymbols(
    OptimizationParam* optimizationParams,
    int numOptimizedParams,
    OptimizationType optimizationType,
    GeneticOptimizationSettings optimizationSettings,
    int numThreads,
    double* pInSettings,
    char** pInTradeSymbol,
    char* pInAccountCurrency,
    char* pInBrokerName,
    char* pInRefBrokerName,
    double* pInAccountInfo,
    TestSettings* testSettings,
    CRatesInfo** pRatesInfo,
    int numCandles,
    int numSymbols,
    ASTRates*** pRates,
    double minLotSize,
    void (*optimizationUpdate)(TestResult testResult, double* settings, int numSettings),
    void (*optimizationFinished)(),
    char** error
);
```

Runs parameter optimization using brute force or genetic algorithm.

**Optimization Goals**:
- `OPTI_GOAL_PROFIT`: Maximize final balance
- `OPTI_GOAL_MAX_DD`: Minimize maximum drawdown
- `OPTI_GOAL_CAGR_TO_MAXDD`: Maximize CAGR/MaxDD ratio (risk-adjusted return)
- `OPTI_GOAL_SHARPE`: Maximize Sharpe ratio
- And more...

**Returns**: 1 on success, 0 on failure

## Data Structures

### TestResult
Contains comprehensive backtest statistics:
- Trade counts and balances
- Risk metrics (Max DD, Sharpe, Ulcer Index)
- Performance metrics (CAGR, Profit Factor, R²)

### OptimizationParam
Defines a parameter to optimize:
- `index`: Parameter index in settings array
- `start`: Starting value
- `step`: Step size
- `stop`: Ending value

### GeneticOptimizationSettings
Configures genetic algorithm:
- `population`: Population size
- `crossoverProbability`: Crossover rate (0.0-1.0)
- `mutationProbability`: Mutation rate (0.0-1.0)
- `maxGenerations`: Maximum generations (0 = unlimited)
- `optimizationGoal`: Goal to optimize for

## Threading and Parallelization

### OpenMP Support
- Multi-threaded optimization using OpenMP
- Thread-safe framework initialization
- Dynamic scheduling for load balancing

### MPI Support
- Distributed optimization across multiple nodes
- Automatic work distribution
- Master-slave architecture

## Dependencies

- **AsirikuyFrameworkAPI**: Core framework functionality
- **Gaul**: Genetic algorithm library
- **MPI**: Message Passing Interface (optional)
- **OpenMP**: Parallel processing (optional)

## Build Configuration

See `premake4.lua` for build configuration:
- Shared library output
- Platform-specific settings (macOS, Linux, Windows)
- Library linking and include paths

## Known Issues

1. **historics.c**: Has parsing issues and memory leaks - needs refactoring
2. **Memory Management**: Some functions need better error handling
3. **Error Messages**: Some error paths could provide more detail

## Usage Example

See `ctester/asirikuy_strategy_tester.py` for Python integration examples.

## Version

Current version: 1.2.1 (defined in `version.h`)

