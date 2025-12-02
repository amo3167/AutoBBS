# Optimizer Usage Guide

## Overview

The `optimizer.c` module provides parameter optimization capabilities for trading strategies using two methods:
- **Brute Force**: Exhaustive search through all parameter combinations
- **Genetic Algorithm**: Evolutionary search using the Gaul library

The optimizer supports multi-symbol portfolio optimization with parallel execution via OpenMP (single machine) or MPI (distributed computing).

## Main API

### Function Signature

```c
int __stdcall runOptimizationMultipleSymbols(
    OptimizationParam    *optimizationParams,      // Array of parameters to optimize
    int                  numOptimizedParams,      // Number of parameters
    OptimizationType     optimizationType,         // OPTI_BRUTE_FORCE or OPTI_GENETIC
    GeneticOptimizationSettings optimizationSettings, // GA configuration
    int                  numThreads,               // Number of parallel threads
    double*              pInSettings,             // Base strategy settings array
    char**               pInTradeSymbol,          // Array of symbol names
    char*                pInAccountCurrency,      // Account currency
    char*                pInBrokerName,           // Broker name
    char*                pInRefBrokerName,        // Reference broker name
    double*              pInAccountInfo,          // Account information
    TestSettings*        testSettings,            // Test configuration
    CRatesInfo**         pRatesInfo,              // Rate information
    int                  numCandles,              // Number of candles
    int                  numSymbols,              // Number of symbols
    ASTRates***          pRates,                  // Historical rate data
    double               minLotSize,              // Minimum lot size
    void                 (*optimizationUpdate)(TestResult, double*, int), // Callback
    void                 (*optimizationFinished)(),                      // Callback
    char                 **error                  // Error output
);
```

## Data Structures

### OptimizationParam

Defines a parameter range to optimize:

```c
typedef struct optimization_param_t {
    int    index;  // Parameter index in settings array
    double start; // Starting value
    double step;  // Step size
    double stop;  // Ending value
} OptimizationParam;
```

**Example:**
```c
OptimizationParam params[2];
params[0].index = AUTOBBS_RISK_CAP;
params[0].start = 1.0;
params[0].step = 1.0;
params[0].stop = 10.0;  // Tests: 1, 2, 3, ..., 10

params[1].index = AUTOBBS_MAX_ACCOUNT_RISK;
params[1].start = 10.0;
params[1].step = 5.0;
params[1].stop = 30.0;  // Tests: 10, 15, 20, 25, 30
```

### OptimizationType

```c
typedef enum optimization_type {
    OPTI_BRUTE_FORCE = 0,  // Exhaustive search
    OPTI_GENETIC = 1       // Genetic algorithm
} OptimizationType;
```

### OptimizationGoal

Fitness metric to optimize:

```c
typedef enum optimization_goal {
    OPTI_GOAL_PROFIT        = 0,  // Maximize profit
    OPTI_GOAL_MAX_DD       = 1,  // Minimize max drawdown
    OPTI_GOAL_MAX_DD_LENGTH = 2, // Minimize drawdown duration
    OPTI_GOAL_PF           = 3,  // Maximize profit factor
    OPTI_GOAL_R2           = 4,  // Maximize R² (linearity)
    OPTI_GOAL_ULCER_INDEX  = 5,  // Minimize ulcer index
    OPTI_GOAL_SHARPE       = 6,  // Maximize Sharpe ratio
    OPTI_GOAL_CAGR_TO_MAXDD = 7, // Maximize CAGR/MaxDD ratio
    OPTI_GOAL_CAGR         = 8   // Maximize CAGR
} OptimizationGoal;
```

### GeneticOptimizationSettings

Configuration for genetic algorithm optimization:

```c
typedef struct genetic_optimization_settings_t {
    int    population;              // Population size (e.g., 30-100)
    double crossoverProbability;    // 0.0-1.0 (e.g., 0.90)
    double mutationProbability;     // 0.0-1.0 (e.g., 0.20)
    double migrationProbability;    // 0.0-1.0 (for distributed GA)
    int    evolutionaryMode;       // 0=Darwin, 1-3=Lamarck, 4-12=Baldwin
    int    elitismMode;             // 0-4 (survival strategy)
    int    mutationMode;            // 0-3 (mutation type)
    int    crossoverMode;           // 0-4 (crossover type)
    int    maxGenerations;           // Max generations (0 = unlimited)
    int    stopIfConverged;          // Stop when population converges
    int    discardAssymetricSets;    // Discard asymmetric long/short results
    int    minTradesAYear;           // Minimum trades per year filter
    int    optimizationGoal;         // Fitness metric (OptimizationGoal enum)
} GeneticOptimizationSettings;
```

#### Evolutionary Modes

- **0 - Darwin**: Standard genetic algorithm
- **1-3 - Lamarck**: Parents/Children/All inherit learned traits
- **4-12 - Baldwin**: Parents/Children/All use learned traits without inheritance

#### Elitism Modes

- **0**: Unknown
- **1**: Parents survive
- **2**: One parent survives
- **3**: Parents die
- **4**: Rescore parents

#### Mutation Modes

- **0**: Single point drift
- **1**: Single point randomize
- **2**: Multipoint
- **3**: All point

#### Crossover Modes

- **0**: Single points
- **1**: Double points
- **2**: Mean
- **3**: Mixing
- **4**: Allele mixing

## Optimization Modes

### Brute Force

- **Method**: Tests all parameter combinations exhaustively
- **Limitation**: Maximum 10 million combinations
- **Use Case**: Small parameter spaces (< 10M combinations)
- **Parallelization**: OpenMP (multi-threaded) or MPI (distributed)

**Example Calculation:**
```
Parameter 1: 10 values (1.0 to 10.0, step 1.0)
Parameter 2: 5 values (10.0 to 30.0, step 5.0)
Total combinations: 10 × 5 = 50 combinations
```

### Genetic Algorithm

- **Method**: Evolutionary search using Gaul library
- **Advantages**: Handles large parameter spaces efficiently
- **Features**:
  - Population-based search
  - Convergence detection
  - Early stopping
  - Configurable selection, crossover, and mutation
- **Parallelization**: OpenMP (multi-threaded) or MPI (distributed)

**Parameter Mapping:**
- Chromosomes use integer values 1-100
- Values are mapped to actual parameter ranges via `mapParamValue()`

## Fitness Calculation

The optimizer calculates fitness based on the selected goal:

| Goal | Fitness Formula |
|------|----------------|
| Profit | `finalBalance - initialBalance` |
| Max DD | `initialBalance / maxDDDepth` |
| Max DD Length | `(yearsTraded × 365) / maxDDLength` |
| Profit Factor | `profitFactor` |
| R² | `r2` |
| Ulcer Index | `10 / ulcerIndex` |
| Sharpe | `sharpeRatio` |
| CAGR/MaxDD | `cagr / maxDDDepth` |

## Filtering Criteria

The optimizer automatically filters out invalid results:

1. **Negative Balance**: Results with `finalBalance < initialBalance` get fitness = 0
2. **Asymmetric Sets**: If `discardAssymetricSets = 1`, filters sets where:
   ```
   |numShorts - numLongs| > 0.5 × min(numShorts, numLongs)
   ```
3. **Low Trade Frequency**: Filters sets with:
   ```
   totalTrades / yearsTraded < minTradesAYear
   ```

## Callbacks

### optimizationUpdate

Called after each iteration with test results:

```c
void optimizationUpdate(TestResult testResult, double* settings, int numSettings) {
    // testResult: Contains all backtest metrics
    // settings: Array of [index, value, index, value, ...] pairs
    // numSettings: Number of optimized parameters
}
```

### optimizationFinished

Called when optimization completes:

```c
void optimizationFinished() {
    // Cleanup or final processing
}
```

## Control Functions

### stopOptimization

Stop optimization prematurely:

```c
void __stdcall stopOptimization();
```

Sets internal flag that causes optimization to stop at the next generation/iteration.

## Parallel Execution

### OpenMP (Single Machine)

- Multi-threaded execution on a single machine
- Set `numThreads` to number of CPU cores
- Automatically detected if `_OPENMP` is defined

### MPI (Distributed)

- Distributed execution across multiple machines/nodes
- Automatically detected via environment variables:
  - `OMPI_COMM_WORLD_RANK` (OpenMPI)
  - `PMI_RANK` (MPICH)
- Main thread (rank 0) coordinates, slaves (rank > 0) execute

## Configuration Example

From `vast.config.portfolio.XAU.15M.set`:

```ini
[optimization]
optimize = 0                    # Enable optimization (0/1)
optimizationtype = 0            # 0=Brute Force, 1=Genetic
numcores = 1                    # Number of threads
optimizationgoal = 0            # 0=Profit, 1=MaxDD, etc.
population = 30                 # GA population size
maxgenerations = 10000          # Max generations (0=unlimited)
crossoverprobability = 0.90     # Crossover rate
mutationprobability = 0.20      # Mutation rate
migrationprobability = 0.00     # Migration rate (MPI)
evolutionarymode = 0            # 0=Darwin
elitismmode = 0                 # Elitism strategy
mutationmode = 0                # Mutation type
crossovermode = 0               # Crossover type
```

## Usage Workflow

1. **Define Parameters**: Create `OptimizationParam` array with parameter ranges
2. **Configure Settings**: Set up `GeneticOptimizationSettings` (for GA)
3. **Prepare Data**: Load market data (`ASTRates`, `CRatesInfo`, etc.)
4. **Set Callbacks**: Implement `optimizationUpdate` and `optimizationFinished`
5. **Run Optimization**: Call `runOptimizationMultipleSymbols()`
6. **Process Results**: Handle results in `optimizationUpdate` callback

## Python Integration

See `ctester/asirikuy_strategy_tester.py` and `ctester/ui/mainwindow.py` for complete Python examples using ctypes.

**Key Python Structures:**
```python
class OptimizationParam(Structure):
    _fields_ = [
        ("index", c_int),
        ("start", c_double),
        ("step", c_double),
        ("stop", c_double)
    ]

class GeneticOptimizationSettings(Structure):
    _fields_ = [
        ("population", c_int),
        ("crossoverProbability", c_double),
        ("mutationProbability", c_double),
        # ... (see include/asirikuy.py for full definition)
    ]
```

## Best Practices

1. **Start Small**: Test with small parameter ranges first
2. **Use Brute Force**: For < 1M combinations, brute force is simpler
3. **Genetic Algorithm**: For large spaces, use GA with 30-100 population
4. **Convergence**: Enable `stopIfConverged` to save time
5. **Filtering**: Use `discardAssymetricSets` and `minTradesAYear` to filter bad results
6. **Parallelization**: Use all available CPU cores for faster results
7. **Goal Selection**: Choose optimization goal that matches your trading objectives

## Limitations

- **Brute Force**: Maximum 10 million combinations
- **Memory**: Each iteration allocates memory for market data copies
- **Time**: Large optimizations can take hours or days
- **Convergence**: GA may converge to local optima

## Troubleshooting

### Optimization Stops Early
- Check if `maxGenerations` is set too low
- Verify convergence detection isn't triggering prematurely
- Check if `stopOptimization()` was called

### No Results
- Verify parameter ranges are valid
- Check if filtering criteria are too strict
- Ensure market data is loaded correctly

### Poor Performance
- Increase `numThreads` for parallel execution
- Use MPI for distributed computing
- Consider reducing parameter space

## Related Files

- `core/CTesterFrameworkAPI/src/optimizer.c` - Implementation
- `core/CTesterFrameworkAPI/include/optimizer.h` - API definitions
- `ctester/asirikuy_strategy_tester.py` - Python integration example
- `ctester/ui/mainwindow.py` - GUI integration example

