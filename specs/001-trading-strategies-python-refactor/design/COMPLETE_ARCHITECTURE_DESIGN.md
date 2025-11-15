# Complete Architecture Design: Framework + Execution + Monitor + Management + ML

## Overview

This document outlines the **complete architecture** for the AutoBBS trading system, including:
- **Framework** (C library - unchanged)
- **MQL4/5 Execution** (unchanged)
- **Monitor** (upgraded: File + DB + Web)
- **Management System** (new: File + DB + Web, operations, auto backtesting, portfolio management, strategy fine-tuning)
- **ML Service** (new: parameter optimization, signal generation)

**Current Management**: Google Remote Desktop to local desktop  
**Target Management**: Web-based management interface (replaces remote desktop)

---

## Part 1: Complete Architecture Overview

### 1.1 System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        AutoBBS Trading System                                │
│                                                                              │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                    Management Layer (NEW)                            │  │
│  │  - Web Dashboard (FastAPI)                                          │  │
│  │  - Operations Management                                            │  │
│  │  - Auto Backtesting                                                 │  │
│  │  - Portfolio Management                                             │  │
│  │  - Strategy Fine-tuning                                             │  │
│  │  - Database (PostgreSQL)                                            │  │
│  └───────────────────┬──────────────────────────────────────────────────┘  │
│                      │                                                       │
│        ┌─────────────┼─────────────┐                                         │
│        │             │             │                                         │
│  ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐                                │
│  │  Monitor  │ │   ML      │ │ Framework │                                │
│  │  (File+   │ │  Service  │ │  (C Lib)  │                                │
│  │   DB+Web) │ │           │ │           │                                │
│  └─────┬─────┘ └─────┬─────┘ └─────┬─────┘                                │
│        │             │             │                                         │
│        └─────────────┼─────────────┘                                         │
│                      │                                                       │
│  ┌───────────────────▼──────────────────────────────────────────────────┐  │
│  │              Execution Layer                                         │  │
│  │  - MQL4/5 EAs (Windows)                                            │  │
│  │  - Python Workers (Linux/macOS/Windows)                             │  │
│  │  - CTester (Backtesting)                                            │  │
│  └─────────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 Component Breakdown

| Component | Current State | Target State | Status |
|-----------|---------------|--------------|--------|
| **Framework** | ✅ C Library | ✅ C Library (unchanged) | ✅ Complete |
| **MQL4/5 Execution** | ✅ Windows EAs | ✅ Windows EAs (unchanged) | ✅ Complete |
| **Monitor** | ⚠️ File-based | ✅ File + DB + Web | ⏳ Upgrade |
| **Management** | ❌ Remote Desktop | ✅ Web-based (File + DB + Web) | ⏳ New |
| **ML Service** | ❌ None | ✅ Python ML Service | ⏳ New |

---

## Part 2: Component Details

### 2.1 Framework (C Library) - UNCHANGED

**Role**: Core trading logic executor

**Responsibilities**:
- Strategy execution
- Signal generation
- State management
- Order management logic

**Interface**:
- C API (via `AsirikuyFrameworkAPI.dll/so/dylib`)
- Called by MQL4/5 EAs or Python workers

**Status**: ✅ **No changes needed**

---

### 2.2 Execution Layer - UNCHANGED

#### 2.2.1 MQL4/5 Execution (Windows)

**Role**: Live trading on Windows

**Components**:
- MQL4/5 Expert Advisors (EAs)
- MetaTrader 4/5 platform
- Order execution via broker

**Flow**:
```
MQL4/5 EA
    ↓
Calls Framework (AsirikuyFrameworkAPI.dll)
    ↓
Gets trading signals
    ↓
Executes orders via MetaTrader
```

**Status**: ✅ **No changes needed**

#### 2.2.2 Python Workers (Optional)

**Role**: Live trading on Linux/macOS/Windows

**Components**:
- Python trading workers (`atrader.py`)
- Broker API integration (OANDA, JForex)
- Framework integration via `ctypes`

**Flow**:
```
Python Worker
    ↓
Calls Framework (libAsirikuyFrameworkAPI.so/dylib)
    ↓
Gets trading signals
    ↓
Executes orders via Broker API
```

**Status**: ✅ **Exists, can be enhanced**

#### 2.2.3 CTester (Backtesting)

**Role**: Strategy backtesting

**Components**:
- CTester Python script
- Historical data loading
- Framework integration

**Status**: ✅ **No changes needed**

---

### 2.3 Monitor - UPGRADE

**Role**: System health and error monitoring

**Current**: File-based only  
**Target**: File + DB + Web

**Components**:
1. **Monitoring Core** (`checker.py`)
   - Reads heartbeat files (`.hb`)
   - Reads log files (`.log`)
   - Detects issues

2. **Database Layer** (NEW)
   - PostgreSQL storage
   - Stores heartbeat events
   - Stores error logs
   - Stores alert history

3. **Web Dashboard** (NEW)
   - FastAPI web server
   - Real-time status display
   - Historical charts
   - Alert management

**Flow**:
```
Files (.hb, .log) → Monitor reads → Store in DB → Dashboard displays
```

**Status**: ⏳ **Upgrade in progress** (see `MONITORING_UPGRADE_PLAN.md`)

---

### 2.4 Management System - NEW

**Role**: Centralized management for all operations

**Components**:

#### 2.4.1 Web Dashboard (FastAPI)

**Features**:
- **Operations Management**
  - Start/stop trading systems
  - View system status
  - Manage configurations
  - View logs

- **Auto Backtesting**
  - Schedule backtests
  - View backtest results
  - Compare strategies
  - Generate reports

- **Portfolio Management**
  - View all accounts
  - View all strategies
  - View portfolio performance
  - Risk management

- **Strategy Fine-tuning**
  - Parameter optimization
  - Walk-forward optimization
  - Strategy comparison
  - Performance analysis

**Access**: Web browser (replaces Google Remote Desktop)

#### 2.4.2 Database Layer (PostgreSQL)

**Tables**:
- `accounts` - Trading accounts
- `strategies` - Strategy configurations
- `backtests` - Backtest results
- `trades` - Trade history
- `performance_metrics` - Performance data
- `optimization_runs` - Optimization results
- `monitoring_events` - Monitoring data (from Monitor)

#### 2.4.3 File System Integration

**Purpose**: Interface with existing file-based systems

**Integration Points**:
- Read MQL4/5 configuration files (`.set` files)
- Read Framework configuration (`AsirikuyConfig.xml`)
- Read backtest results (CSV, XML)
- Write optimized parameters back to files

**Flow**:
```
Management System
    ↓
Reads/Writes Files (configs, results)
    ↓
Stores in Database
    ↓
Displays in Web Dashboard
```

---

### 2.5 ML Service - NEW

**Role**: Machine learning for strategy optimization and signal generation

**Components**:

#### 2.5.1 Parameter Optimization

**Features**:
- Train models on historical performance
- Optimize strategy parameters
- Generate optimal parameter sets
- Walk-forward optimization

**Integration**:
```
ML Service
    ↓
Optimizes Parameters
    ↓
Writes to Database/Config Files
    ↓
Framework uses optimized parameters
```

#### 2.5.2 Signal Generation (Optional)

**Features**:
- Generate trading signals using ML
- Pattern recognition
- Market regime detection

**Integration**:
```
ML Service
    ↓
Generates Signals
    ↓
Passes to Framework
    ↓
Framework validates and executes
```

**Status**: ⏳ **New component** (see `FRAMEWORK_ML_UPGRADE_DESIGN.md`)

---

## Part 3: Data Flow

### 3.1 Live Trading Flow

```
┌─────────────────────────────────────────────────────────────┐
│         Live Trading Flow                                    │
│                                                              │
│  1. MQL4/5 EA (or Python Worker)                           │
│     ├── Gets market data                                     │
│     ├── Gets account info                                    │
│     ├── Gets open orders                                    │
│     │                                                       │
│  2. Calls Framework                                         │
│     ├── Prepares data structures                            │
│     ├── Calls mql5_runStrategy()                            │
│     ├── Gets trading signals                                │
│     │                                                       │
│  3. Executes Orders                                         │
│     ├── MQL4/5: Via MetaTrader                             │
│     ├── Python: Via Broker API                              │
│     │                                                       │
│  4. Monitor Watches                                         │
│     ├── Checks heartbeat files                              │
│     ├── Checks log files                                    │
│     ├── Stores in database                                  │
│     │                                                       │
│  5. Management System                                       │
│     ├── Reads from database                                 │
│     ├── Displays in web dashboard                           │
│     └── Sends alerts                                        │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Backtesting Flow

```
┌─────────────────────────────────────────────────────────────┐
│         Backtesting Flow                                     │
│                                                              │
│  1. Management System                                       │
│     ├── User schedules backtest                             │
│     ├── Configures parameters                                │
│     ├── Selects historical data                             │
│     │                                                       │
│  2. CTester Executes                                        │
│     ├── Loads historical data                               │
│     ├── Calls Framework                                     │
│     ├── Runs backtest                                       │
│     ├── Generates results                                   │
│     │                                                       │
│  3. Results Stored                                          │
│     ├── CSV files (existing)                                │
│     ├── Database (new)                                      │
│     │                                                       │
│  4. Management System                                       │
│     ├── Reads results from database                         │
│     ├── Displays in web dashboard                           │
│     ├── Generates reports                                   │
└─────────────────────────────────────────────────────────────┘
```

### 3.3 Strategy Optimization Flow

```
┌─────────────────────────────────────────────────────────────┐
│         Strategy Optimization Flow                            │
│                                                              │
│  1. Management System                                       │
│     ├── User initiates optimization                         │
│     ├── Configures optimization parameters                   │
│     ├── Selects historical data                             │
│     │                                                       │
│  2. ML Service                                              │
│     ├── Trains optimization model                           │
│     ├── Runs optimization (genetic algorithm or ML)        │
│     ├── Generates optimal parameters                        │
│     │                                                       │
│  3. Results Stored                                          │
│     ├── Database (optimization results)                     │
│     ├── Config files (optimized parameters)                 │
│     │                                                       │
│  4. Management System                                       │
│     ├── Displays optimization results                       │
│     ├── User reviews and approves                           │
│     ├── Applies to live trading (if approved)              │
└─────────────────────────────────────────────────────────────┘
```

---

## Part 4: Management System Architecture

### 4.1 Management System Components

```
┌─────────────────────────────────────────────────────────────┐
│              Management System                                │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Web Dashboard (FastAPI)                      │  │
│  │  - REST API                                           │  │
│  │  - WebSocket (real-time)                             │  │
│  │  - HTML/CSS/JavaScript                                │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│        ┌─────────────┼─────────────┐                         │
│        │             │             │                         │
│  ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐                │
│  │Operations │ │Backtesting │ │Portfolio  │                │
│  │ Manager   │ │ Manager    │ │ Manager   │                │
│  └─────┬─────┘ └─────┬─────┘ └─────┬─────┘                │
│        │             │             │                         │
│        └─────────────┼─────────────┘                         │
│                      │                                       │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │         Database Layer (PostgreSQL)                   │  │
│  │  - Accounts, Strategies, Trades, Performance          │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │         File System Integration                       │  │
│  │  - Read/write config files (.set, .xml)             │  │
│  │  - Read backtest results (CSV, XML)                 │  │
│  │  - Write optimized parameters                        │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Operations Manager

**Purpose**: Manage trading system operations

**Features**:
1. **System Control**
   - Start/stop MQL4/5 EAs
   - Start/stop Python workers
   - Restart systems on failure
   - View system status

2. **Configuration Management**
   - View/edit strategy configurations
   - View/edit Framework config
   - Apply configuration changes
   - Configuration versioning

3. **Log Management**
   - View real-time logs
   - Search historical logs
   - Filter by severity
   - Export logs

4. **Health Monitoring**
   - System health status
   - Resource usage (CPU, memory)
   - Network connectivity
   - Broker API connectivity

**Implementation**:
```python
# operations_manager.py
class OperationsManager:
    def start_ea(self, account_name, strategy_name):
        """Start MQL4/5 EA"""
        # Start EA via MetaTrader API or file system
        
    def stop_ea(self, account_name, strategy_name):
        """Stop MQL4/5 EA"""
        
    def restart_system(self, account_name):
        """Restart trading system"""
        
    def get_system_status(self, account_name):
        """Get system status"""
        # Query database, check files, check processes
```

### 4.3 Auto Backtesting Manager

**Purpose**: Automated backtesting management

**Features**:
1. **Backtest Scheduling**
   - Schedule recurring backtests
   - Schedule one-time backtests
   - Schedule optimization runs
   - Schedule walk-forward optimization

2. **Backtest Execution**
   - Queue backtest jobs
   - Execute via CTester
   - Monitor progress
   - Handle failures

3. **Results Management**
   - Store results in database
   - Compare results
   - Generate reports
   - Export results

4. **Historical Data Management**
   - Manage historical data files
   - Validate data quality
   - Update data automatically

**Implementation**:
```python
# backtesting_manager.py
class BacktestingManager:
    def schedule_backtest(self, strategy_id, start_date, end_date, config):
        """Schedule backtest"""
        # Create backtest job
        # Queue for execution
        
    def execute_backtest(self, backtest_id):
        """Execute backtest via CTester"""
        # Call CTester Python script
        # Monitor progress
        # Store results
        
    def get_backtest_results(self, backtest_id):
        """Get backtest results from database"""
        
    def compare_backtests(self, backtest_ids):
        """Compare multiple backtest results"""
```

### 4.4 Portfolio Manager

**Purpose**: Manage multiple accounts and strategies

**Features**:
1. **Account Management**
   - View all accounts
   - Account status
   - Account performance
   - Account configuration

2. **Strategy Management**
   - View all strategies
   - Strategy status
   - Strategy performance
   - Strategy allocation

3. **Portfolio View**
   - Combined performance
   - Risk metrics
   - Correlation analysis
   - Diversification metrics

4. **Risk Management**
   - Portfolio risk limits
   - Per-account risk limits
   - Per-strategy risk limits
   - Risk alerts

**Implementation**:
```python
# portfolio_manager.py
class PortfolioManager:
    def get_all_accounts(self):
        """Get all trading accounts"""
        
    def get_account_performance(self, account_name, period):
        """Get account performance metrics"""
        
    def get_portfolio_performance(self, period):
        """Get combined portfolio performance"""
        
    def get_risk_metrics(self):
        """Get portfolio risk metrics"""
```

### 4.5 Strategy Fine-tuning Manager

**Purpose**: Strategy parameter optimization and fine-tuning

**Features**:
1. **Parameter Optimization**
   - Genetic algorithm optimization
   - ML-based optimization
   - Grid search
   - Walk-forward optimization

2. **Optimization Management**
   - Schedule optimization runs
   - Monitor optimization progress
   - Compare optimization results
   - Apply optimized parameters

3. **Performance Analysis**
   - Strategy performance metrics
   - Parameter sensitivity analysis
   - Overfitting detection
   - Out-of-sample testing

**Implementation**:
```python
# strategy_tuning_manager.py
class StrategyTuningManager:
    def optimize_parameters(self, strategy_id, optimization_config):
        """Optimize strategy parameters"""
        # Call ML service or genetic algorithm
        # Store results in database
        
    def get_optimization_results(self, optimization_id):
        """Get optimization results"""
        
    def apply_optimized_parameters(self, optimization_id, strategy_id):
        """Apply optimized parameters to strategy"""
        # Update config files
        # Update database
```

---

## Part 5: Database Schema

### 5.1 Complete Database Schema

```sql
-- Accounts
CREATE TABLE accounts (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL UNIQUE,
    broker_name VARCHAR(255),
    account_number VARCHAR(255),
    account_type VARCHAR(50),  -- demo, real
    frontend VARCHAR(50),  -- MT4, MT5, Python
    path VARCHAR(512),
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Strategies
CREATE TABLE strategies (
    id SERIAL PRIMARY KEY,
    account_id INTEGER REFERENCES accounts(id),
    name VARCHAR(255) NOT NULL,
    strategy_type VARCHAR(50),  -- AUTOBBS, Trend, Swing, etc.
    instance_id INTEGER,
    config_file_path VARCHAR(512),
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Trades (from Monitor + Execution)
CREATE TABLE trades (
    id SERIAL PRIMARY KEY,
    account_id INTEGER REFERENCES accounts(id),
    strategy_id INTEGER REFERENCES strategies(id),
    ticket INTEGER,
    symbol VARCHAR(50),
    order_type VARCHAR(50),  -- BUY, SELL, etc.
    volume FLOAT,
    open_price FLOAT,
    close_price FLOAT,
    stop_loss FLOAT,
    take_profit FLOAT,
    open_time TIMESTAMP,
    close_time TIMESTAMP,
    profit FLOAT,
    is_open BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Backtests
CREATE TABLE backtests (
    id SERIAL PRIMARY KEY,
    strategy_id INTEGER REFERENCES strategies(id),
    start_date TIMESTAMP,
    end_date TIMESTAMP,
    status VARCHAR(50),  -- pending, running, completed, failed
    config JSONB,
    results_file_path VARCHAR(512),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP
);

-- Backtest Results
CREATE TABLE backtest_results (
    id SERIAL PRIMARY KEY,
    backtest_id INTEGER REFERENCES backtests(id),
    final_balance FLOAT,
    initial_balance FLOAT,
    total_trades INTEGER,
    winning_trades INTEGER,
    losing_trades INTEGER,
    profit_factor FLOAT,
    sharpe_ratio FLOAT,
    max_drawdown FLOAT,
    cagr FLOAT,
    r2 FLOAT,
    results_data JSONB,  -- Full results data
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Optimization Runs
CREATE TABLE optimization_runs (
    id SERIAL PRIMARY KEY,
    strategy_id INTEGER REFERENCES strategies(id),
    optimization_type VARCHAR(50),  -- genetic, ml, grid_search
    status VARCHAR(50),  -- pending, running, completed, failed
    config JSONB,
    best_parameters JSONB,
    best_fitness FLOAT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP
);

-- Performance Metrics (time-series)
CREATE TABLE performance_metrics (
    id SERIAL PRIMARY KEY,
    account_id INTEGER REFERENCES accounts(id),
    strategy_id INTEGER REFERENCES strategies(id),
    timestamp TIMESTAMP NOT NULL,
    equity FLOAT,
    balance FLOAT,
    profit FLOAT,
    drawdown FLOAT,
    open_trades INTEGER,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Monitoring Events (from Monitor)
CREATE TABLE monitoring_events (
    id SERIAL PRIMARY KEY,
    account_id INTEGER REFERENCES accounts(id),
    event_type VARCHAR(50),  -- heartbeat, error, alert
    severity VARCHAR(50),  -- INFO, WARNING, ERROR, CRITICAL
    message TEXT,
    metadata JSONB,
    timestamp TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes
CREATE INDEX idx_trades_account_strategy ON trades(account_id, strategy_id);
CREATE INDEX idx_trades_open_time ON trades(open_time);
CREATE INDEX idx_performance_account_timestamp ON performance_metrics(account_id, timestamp DESC);
CREATE INDEX idx_monitoring_account_timestamp ON monitoring_events(account_id, timestamp DESC);
```

---

## Part 6: Web Dashboard Implementation

### 6.1 Dashboard Structure

```
Web Dashboard (FastAPI)
├── Operations Tab
│   ├── System Status
│   ├── Start/Stop Controls
│   ├── Configuration Editor
│   └── Log Viewer
├── Backtesting Tab
│   ├── Schedule Backtest
│   ├── Backtest Results
│   ├── Comparison View
│   └── Reports
├── Portfolio Tab
│   ├── Account Overview
│   ├── Strategy Performance
│   ├── Risk Metrics
│   └── Correlation Analysis
├── Strategy Tuning Tab
│   ├── Parameter Optimization
│   ├── Optimization Results
│   ├── Performance Analysis
│   └── Apply Parameters
└── Monitoring Tab
    ├── Real-time Status
    ├── Historical Charts
    ├── Alert History
    └── Error Logs
```

### 6.2 FastAPI Implementation

```python
# management_dashboard.py
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
import uvicorn
from database import ManagementDatabase
from operations_manager import OperationsManager
from backtesting_manager import BacktestingManager
from portfolio_manager import PortfolioManager
from strategy_tuning_manager import StrategyTuningManager

app = FastAPI(title="AutoBBS Management Dashboard")

# Initialize components
db = ManagementDatabase()
ops_manager = OperationsManager(db)
backtest_manager = BacktestingManager(db)
portfolio_manager = PortfolioManager(db)
tuning_manager = StrategyTuningManager(db)

@app.get("/")
async def dashboard():
    """Main dashboard page"""
    return HTMLResponse(content=open('templates/dashboard.html').read())

# Operations API
@app.get("/api/operations/status")
async def get_operations_status():
    """Get system operations status"""
    return ops_manager.get_all_systems_status()

@app.post("/api/operations/start/{account_name}")
async def start_system(account_name: str):
    """Start trading system"""
    return ops_manager.start_system(account_name)

@app.post("/api/operations/stop/{account_name}")
async def stop_system(account_name: str):
    """Stop trading system"""
    return ops_manager.stop_system(account_name)

# Backtesting API
@app.post("/api/backtesting/schedule")
async def schedule_backtest(backtest_config: dict):
    """Schedule backtest"""
    return backtest_manager.schedule_backtest(backtest_config)

@app.get("/api/backtesting/results/{backtest_id}")
async def get_backtest_results(backtest_id: int):
    """Get backtest results"""
    return backtest_manager.get_backtest_results(backtest_id)

# Portfolio API
@app.get("/api/portfolio/accounts")
async def get_all_accounts():
    """Get all accounts"""
    return portfolio_manager.get_all_accounts()

@app.get("/api/portfolio/performance")
async def get_portfolio_performance(period: str = "24h"):
    """Get portfolio performance"""
    return portfolio_manager.get_portfolio_performance(period)

# Strategy Tuning API
@app.post("/api/tuning/optimize")
async def optimize_strategy(optimization_config: dict):
    """Optimize strategy parameters"""
    return tuning_manager.optimize_parameters(optimization_config)

@app.get("/api/tuning/results/{optimization_id}")
async def get_optimization_results(optimization_id: int):
    """Get optimization results"""
    return tuning_manager.get_optimization_results(optimization_id)

# WebSocket for real-time updates
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket for real-time updates"""
    await websocket.accept()
    try:
        while True:
            # Get current status
            data = {
                "operations": await ops_manager.get_status(),
                "portfolio": await portfolio_manager.get_current_performance(),
                "monitoring": await get_monitoring_status(),
                "timestamp": datetime.utcnow().isoformat()
            }
            await websocket.send_json(data)
            await asyncio.sleep(1)
    except WebSocketDisconnect:
        pass

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
```

---

## Part 7: File System Integration

### 7.1 File System Interface

**Purpose**: Bridge between file-based systems and database

**Integration Points**:

1. **Configuration Files**
   - Read MQL4/5 `.set` files
   - Read Framework `AsirikuyConfig.xml`
   - Write optimized parameters back to files

2. **Backtest Results**
   - Read CTester CSV/XML results
   - Store in database
   - Display in dashboard

3. **Log Files**
   - Read Framework log files
   - Store in database
   - Display in dashboard

**Implementation**:
```python
# file_system_integration.py
class FileSystemIntegration:
    def read_set_file(self, file_path):
        """Read MQL4/5 .set file"""
        # Parse INI format
        # Return as dictionary
        
    def write_set_file(self, file_path, config_dict):
        """Write MQL4/5 .set file"""
        # Convert dictionary to INI format
        # Write to file
        
    def read_framework_config(self, file_path):
        """Read Framework XML config"""
        # Parse XML
        # Return as dictionary
        
    def write_framework_config(self, file_path, config_dict):
        """Write Framework XML config"""
        # Convert dictionary to XML
        # Write to file
        
    def read_backtest_results(self, file_path):
        """Read CTester backtest results"""
        # Parse CSV/XML
        # Return structured data
        
    def sync_to_database(self, account_name):
        """Sync file-based data to database"""
        # Read all config files
        # Read all results files
        # Store in database
```

---

## Part 8: ML Service Integration

### 8.1 ML Service Architecture

```
┌─────────────────────────────────────────────────────────────┐
│              ML Service                                      │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Parameter Optimizer                           │  │
│  │  - Trains on historical performance                  │  │
│  │  - Optimizes strategy parameters                     │  │
│  │  - Generates optimal parameter sets                  │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │         Signal Generator (Optional)                 │  │
│  │  - Generates trading signals                        │  │
│  │  - Pattern recognition                              │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │         Management System Integration                │  │
│  │  - Receives optimization requests                   │  │
│  │  - Returns optimized parameters                    │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 8.2 ML Service API

```python
# ml_service.py
from fastapi import FastAPI
from database import ManagementDatabase

app = FastAPI(title="ML Service")

db = ManagementDatabase()

@app.post("/api/ml/optimize")
async def optimize_parameters(request: OptimizationRequest):
    """Optimize strategy parameters"""
    # Train model on historical data
    # Optimize parameters
    # Return optimal parameters
    
@app.post("/api/ml/generate-signal")
async def generate_signal(request: SignalRequest):
    """Generate trading signal"""
    # Analyze market data
    # Generate signal
    # Return signal
```

---

## Part 9: Remote Desktop Replacement

### 9.1 Current Setup

**Google Remote Desktop**:
- Connects to local desktop
- Manual operations
- No automation
- Limited access

### 9.2 Web-Based Management (Replacement)

**Web Dashboard**:
- ✅ Browser-based (any device)
- ✅ Automated operations
- ✅ Real-time monitoring
- ✅ Historical data
- ✅ Mobile-friendly

**Access**:
- Local: `http://localhost:8000`
- Remote: `http://your-server-ip:8000` (with authentication)
- VPN: Secure access via VPN

**Security**:
- Authentication (username/password)
- HTTPS (SSL/TLS)
- Role-based access control
- Audit logging

---

## Part 10: Implementation Roadmap

### 10.1 Phase 1: Monitor Upgrade (4-6 weeks)

**Goal**: Upgrade Monitor to File + DB + Web

**Tasks**:
1. Database integration (1-2 weeks)
2. Web dashboard (2-3 weeks)
3. Python 3 migration (1 week)

**Deliverables**:
- ✅ Database-backed monitoring
- ✅ Web dashboard for monitoring
- ✅ Real-time status display

### 10.2 Phase 2: Management System Core (6-8 weeks)

**Goal**: Build core management system

**Tasks**:
1. Database schema (1 week)
2. File system integration (1-2 weeks)
3. Operations Manager (2 weeks)
4. Web dashboard framework (2-3 weeks)

**Deliverables**:
- ✅ Database schema
- ✅ File system integration
- ✅ Basic operations management
- ✅ Web dashboard framework

### 10.3 Phase 3: Management Features (8-10 weeks)

**Goal**: Add management features

**Tasks**:
1. Auto Backtesting Manager (2-3 weeks)
2. Portfolio Manager (2-3 weeks)
3. Strategy Tuning Manager (2-3 weeks)
4. Integration testing (1 week)

**Deliverables**:
- ✅ Auto backtesting
- ✅ Portfolio management
- ✅ Strategy fine-tuning
- ✅ Complete management system

### 10.4 Phase 4: ML Service (4-6 weeks)

**Goal**: Add ML service

**Tasks**:
1. ML Service framework (1-2 weeks)
2. Parameter optimization (2-3 weeks)
3. Integration with Management System (1 week)

**Deliverables**:
- ✅ ML Service
- ✅ Parameter optimization
- ✅ Integration with management

**Total Timeline**: 22-30 weeks (5.5-7.5 months)

---

## Part 11: Technology Stack

### 11.1 Core Components

| Component | Technology | Purpose |
|-----------|-----------|---------|
| **Framework** | C | Trading logic |
| **MQL4/5** | MQL4/5 | Windows execution |
| **Python Workers** | Python 3 | Cross-platform execution |
| **Monitor** | Python 3 | System monitoring |
| **Management** | Python 3 + FastAPI | Management system |
| **ML Service** | Python 3 | ML optimization |
| **Database** | PostgreSQL | Persistent storage |
| **Web Dashboard** | FastAPI + HTML/JS | Web interface |

### 11.2 Dependencies

**Python Packages**:
- `fastapi` - Web framework
- `sqlalchemy` - Database ORM
- `prometheus-client` - Metrics
- `websockets` - WebSocket support
- `scikit-learn` - ML (for ML service)
- `pandas` - Data analysis
- `numpy` - Numerical computing

---

## Part 12: Summary

### 12.1 Architecture Overview

**Complete System**:
1. ✅ **Framework** (C Library) - Unchanged
2. ✅ **MQL4/5 Execution** - Unchanged
3. ⏳ **Monitor** - Upgrade to File + DB + Web
4. ⏳ **Management System** - New (File + DB + Web)
5. ⏳ **ML Service** - New

### 12.2 Key Features

**Management System**:
- ✅ Web-based (replaces remote desktop)
- ✅ Operations management
- ✅ Auto backtesting
- ✅ Portfolio management
- ✅ Strategy fine-tuning
- ✅ File + DB integration

**Monitor**:
- ✅ File-based monitoring (kept)
- ✅ Database storage (new)
- ✅ Web dashboard (new)

**ML Service**:
- ✅ Parameter optimization
- ✅ Signal generation (optional)
- ✅ Integration with management

### 12.3 Benefits

1. **Centralized Management**
   - Single web interface for all operations
   - No need for remote desktop
   - Access from any device

2. **Automation**
   - Automated backtesting
   - Automated optimization
   - Automated monitoring

3. **Data Persistence**
   - All data in database
   - Historical analysis
   - Queryable data

4. **Scalability**
   - Multiple accounts
   - Multiple strategies
   - Portfolio view

---

**Document Status**: Complete Architecture Design
**Last Updated**: December 2024
**Related Documents**:
- `MONITORING_UPGRADE_PLAN.md` - Monitor upgrade details
- `FRAMEWORK_ML_UPGRADE_DESIGN.md` - ML service design
- `PYTHON_INTEGRATION_DESIGN.md` - Integration design

