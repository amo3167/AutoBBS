# Option 1: Local Web Server - Complete Specification

**Version**: 1.0  
**Date**: December 2024  
**Status**: Specification

---

## Executive Summary

This document specifies the **Local Web Server** deployment option for the Trading Management System. This is the recommended initial approach, providing a complete web-based dashboard running on the same machine as MetaTrader 4/5, with direct file access, real-time monitoring, optimization, backtesting, reporting, and AI-powered alerting.

---

## 1. Architecture Overview

### 1.1 System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│         Local Windows Machine                                    │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │         Web Server (FastAPI/Django)                       │  │
│  │  - Dashboard UI (React/Vue.js)                            │  │
│  │  - REST API (FastAPI)                                     │  │
│  │  - WebSocket (Real-time updates)                          │  │
│  │  - Port: 8000 (HTTP) / 8443 (HTTPS)                      │  │
│  └───────────────────┬──────────────────────────────────────┘  │
│                      │                                           │
│        ┌─────────────┼─────────────┬─────────────┐              │
│        │             │             │             │              │
│  ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐     │
│  │  Monitor  │ │ Optimizer │ │ Backtest  │ │   AI      │     │
│  │  Service  │ │  Service  │ │  Service  │ │  Service  │     │
│  └─────┬─────┘ └─────┬─────┘ └─────┬─────┘ └─────┬─────┘     │
│        │             │             │             │              │
│        └─────────────┼─────────────┼─────────────┘              │
│                      │             │                             │
│  ┌───────────────────▼─────────────▼────────────────────────┐  │
│  │         Database (PostgreSQL Local)                        │  │
│  │  - Account data                                            │  │
│  │  - Trade history                                           │  │
│  │  - Optimization results                                    │  │
│  │  - Backtest results                                        │  │
│  │  - Alert history                                           │  │
│  │  - AI model data                                           │  │
│  └───────────────────┬──────────────────────────────────────┘  │
│                      │                                           │
│  ┌───────────────────▼──────────────────────────────────────┐  │
│  │         File System (Direct Access)                        │  │
│  │  - MQL4/5 Files (.set, .log, .hb, .state, .xml)          │  │
│  │  - Configuration files                                     │  │
│  │  - Report files                                           │  │
│  └───────────────────┬──────────────────────────────────────┘  │
│                      │                                           │
│  ┌───────────────────▼──────────────────────────────────────┐  │
│  │         Framework Integration (C Library)                   │  │
│  │  - CTesterFrameworkAPI                                     │  │
│  │  - Strategy execution                                      │  │
│  │  - Portfolio management                                    │  │
│  └───────────────────┬──────────────────────────────────────┘  │
│                      │                                           │
│  ┌───────────────────▼──────────────────────────────────────┐  │
│  │         MetaTrader 4/5                                     │  │
│  │  - EAs running                                             │  │
│  │  - Live trading                                            │  │
│  │  - File generation (.set, .log, .hb, .state)              │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 Technology Stack

**Backend**:
- **Web Framework**: FastAPI (Python 3.8+)
- **Database**: PostgreSQL 14+ (local)
- **ORM**: SQLAlchemy
- **Task Queue**: Celery (for async tasks)
- **WebSocket**: WebSockets (FastAPI)

**Frontend**:
- **Framework**: React 18+ or Vue.js 3+
- **UI Library**: Material-UI or Vuetify
- **Charts**: Chart.js or Recharts
- **Real-time**: Socket.io client

**Services**:
- **Monitor**: Python (existing asirikuy_monitor)
- **Optimizer**: Python + C Library (CTesterFrameworkAPI)
- **Backtest**: Python + C Library (CTesterFrameworkAPI)
- **AI Service**: Python (OpenAI API, Claude API, or local LLM)

**Infrastructure**:
- **OS**: Windows 10/11
- **Web Server**: Uvicorn (ASGI)
- **Reverse Proxy**: Nginx (optional, for production)
- **SSL**: Let's Encrypt (for HTTPS)

---

## 2. Core Features

### 2.1 Monitoring Dashboard

**Purpose**: Real-time monitoring of trading systems, accounts, and strategies.

**Features**:
1. **Account Overview**
   - Account balance, equity, margin
   - Open positions
   - Daily/weekly/monthly P&L
   - Win rate, profit factor
   - Real-time updates (WebSocket)

2. **Heartbeat Monitoring**
   - Instance status (online/offline)
   - Last heartbeat timestamp
   - Time since last update
   - Alert on heartbeat failure
   - Visual indicators (green/red)

3. **Error Detection**
   - Real-time error log monitoring
   - Error categorization (Error, Emergency, Critical)
   - Error history and trends
   - Alert notifications
   - Error details and stack traces

4. **Strategy Status**
   - Active strategies per account
   - Strategy performance metrics
   - Strategy configuration
   - Start/stop controls
   - Strategy logs

5. **System Health**
   - CPU, memory, disk usage
   - Database connection status
   - Service status (Monitor, Optimizer, Backtest, AI)
   - Network status
   - File system access

**UI Components**:
- Dashboard cards (metrics)
- Real-time charts (account equity, P&L)
- Status indicators
- Alert notifications
- Data tables (positions, errors, logs)

---

### 2.2 Optimization Service

**Purpose**: Run parameter optimization for trading strategies.

**Features**:
1. **Optimization Setup**
   - Strategy selection
   - Parameter ranges (min, max, step)
   - Optimization method (Genetic Algorithm, Brute Force, Grid Search)
   - Objective function (Sharpe Ratio, Profit Factor, Max Drawdown, etc.)
   - Time period selection
   - Symbol selection

2. **Optimization Execution**
   - Queue optimization jobs
   - Progress tracking (real-time)
   - Parallel execution (multi-core)
   - Pause/resume/cancel
   - Resource management (CPU, memory)

3. **Results Management**
   - Optimization results table
   - Best parameter sets
   - Results comparison
   - Export results (CSV, JSON)
   - Visualization (parameter space, fitness landscape)

4. **Integration**
   - Direct file access (.set files)
   - Strategy configuration management
   - Apply optimized parameters
   - Save optimization presets

**API Endpoints**:
- `POST /api/optimization/start` - Start optimization
- `GET /api/optimization/{id}/status` - Get status
- `GET /api/optimization/{id}/results` - Get results
- `POST /api/optimization/{id}/cancel` - Cancel optimization
- `GET /api/optimization/history` - Get optimization history

---

### 2.3 Backtesting Service

**Purpose**: Run backtests on historical data.

**Features**:
1. **Backtest Setup**
   - Strategy selection
   - Parameter configuration
   - Historical data selection (date range, symbol)
   - Initial capital
   - Commission and slippage settings
   - Timeframe selection

2. **Backtest Execution**
   - Queue backtest jobs
   - Progress tracking (real-time)
   - Parallel execution (multiple strategies)
   - Pause/resume/cancel
   - Resource management

3. **Results Analysis**
   - Performance metrics (Total Return, Sharpe Ratio, Max Drawdown, etc.)
   - Equity curve visualization
   - Trade analysis (win rate, average win/loss, etc.)
   - Monthly/yearly breakdown
   - Drawdown analysis
   - Trade distribution charts

4. **Report Generation**
   - PDF reports
   - HTML reports
   - CSV exports
   - Chart exports (PNG, SVG)
   - Customizable report templates

**API Endpoints**:
- `POST /api/backtest/start` - Start backtest
- `GET /api/backtest/{id}/status` - Get status
- `GET /api/backtest/{id}/results` - Get results
- `GET /api/backtest/{id}/report` - Generate report
- `POST /api/backtest/{id}/cancel` - Cancel backtest
- `GET /api/backtest/history` - Get backtest history

---

### 2.4 Reporting & Analytics

**Purpose**: Generate comprehensive reports and analytics.

**Features**:
1. **Performance Reports**
   - Account performance summary
   - Strategy performance comparison
   - Risk metrics
   - Drawdown analysis
   - Monthly/yearly reports

2. **Trade Analysis**
   - Trade history
   - Trade distribution (by symbol, time, day of week)
   - Win/loss analysis
   - Trade duration analysis
   - Profit/loss distribution

3. **Custom Reports**
   - Report builder (drag-and-drop)
   - Custom metrics
   - Scheduled reports (daily, weekly, monthly)
   - Email/Telegram delivery
   - Report templates

4. **Visualizations**
   - Equity curves
   - Drawdown charts
   - Trade distribution charts
   - Performance heatmaps
   - Correlation matrices

**API Endpoints**:
- `GET /api/reports/performance` - Get performance report
- `GET /api/reports/trades` - Get trade analysis
- `POST /api/reports/generate` - Generate custom report
- `GET /api/reports/{id}/download` - Download report
- `GET /api/reports/templates` - Get report templates

---

### 2.5 AI-Powered Alerting

**Purpose**: Intelligent alerting using LLM (Large Language Models) for summarization, analysis, and actionable suggestions.

**Features**:
1. **Alert Summarization**
   - Summarize multiple alerts into concise reports
   - Group related alerts with context
   - Daily/weekly alert summaries
   - Executive summaries for management

2. **Intelligent Analysis**
   - Analyze alert patterns and trends
   - Identify root causes from error logs
   - Context-aware alert interpretation
   - Historical pattern recognition

3. **Actionable Suggestions**
   - AI-generated recommendations for alert resolution
   - Suggested actions based on alert context
   - Best practice recommendations
   - Troubleshooting guidance

4. **Natural Language Alerts**
   - AI-generated alert descriptions (human-readable)
   - Context-aware alert messages
   - Multi-language support
   - Personalized alert formatting

5. **Report Generation**
   - AI-assisted report writing
   - Executive summaries
   - Technical analysis summaries
   - Custom report templates with AI content

6. **Alert Channels**
   - Email notifications (with AI summaries)
   - Telegram notifications (with AI summaries)
   - Dashboard notifications
   - Webhook integrations
   - SMS (optional)

**AI Models**:
- **LLM for Summarization**: GPT-4, Claude, or local LLM (Llama, Mistral)
- **LLM for Suggestions**: GPT-4, Claude API for actionable recommendations
- **LLM for Analysis**: GPT-4, Claude API for pattern analysis and root cause identification
- **NLP**: GPT-4/Claude API for natural language generation and alert descriptions

**Note**: This system uses LLM models for text generation, summarization, and suggestions. It does NOT use ML models for trading predictions, anomaly detection, or market analysis. The focus is on intelligent text processing and human-readable insights.

**API Endpoints**:
- `GET /api/alerts` - Get alerts
- `POST /api/alerts/{id}/acknowledge` - Acknowledge alert
- `POST /api/alerts/rules` - Create alert rule
- `GET /api/alerts/history` - Get alert history
- `POST /api/alerts/test` - Test alert rule

---

## 3. Database Schema

### 3.1 Core Tables

**accounts**
- `id` (PK)
- `account_number`
- `broker`
- `balance`
- `equity`
- `margin`
- `currency`
- `created_at`
- `updated_at`

**strategies**
- `id` (PK)
- `name`
- `description`
- `type` (EA, Script, Indicator)
- `file_path`
- `config_path`
- `status` (active, inactive, error)
- `created_at`
- `updated_at`

**trades**
- `id` (PK)
- `account_id` (FK)
- `strategy_id` (FK)
- `symbol`
- `type` (buy, sell)
- `volume`
- `open_price`
- `close_price`
- `open_time`
- `close_time`
- `profit`
- `commission`
- `swap`

**optimizations**
- `id` (PK)
- `strategy_id` (FK)
- `name`
- `status` (queued, running, completed, failed)
- `parameters` (JSON)
- `results` (JSON)
- `started_at`
- `completed_at`
- `created_at`

**backtests**
- `id` (PK)
- `strategy_id` (FK)
- `name`
- `status` (queued, running, completed, failed)
- `parameters` (JSON)
- `results` (JSON)
- `report_path`
- `started_at`
- `completed_at`
- `created_at`

**alerts**
- `id` (PK)
- `type` (error, warning, info, critical)
- `category` (monitor, optimization, backtest, system)
- `title`
- `message`
- `ai_analysis` (JSON)
- `status` (new, acknowledged, resolved)
- `priority`
- `created_at`
- `acknowledged_at`
- `resolved_at`

**heartbeats**
- `id` (PK)
- `account_id` (FK)
- `instance_id`
- `timestamp`
- `status` (online, offline)
- `created_at`

**errors**
- `id` (PK)
- `account_id` (FK)
- `strategy_id` (FK)
- `level` (error, emergency, critical)
- `message`
- `stack_trace`
- `file_path`
- `line_number`
- `created_at`

---

## 4. API Specification

### 4.1 REST API Structure

**Base URL**: `http://localhost:8000/api`

**Authentication**: JWT tokens

**Response Format**: JSON

**Error Format**:
```json
{
  "error": {
    "code": "ERROR_CODE",
    "message": "Human-readable error message",
    "details": {}
  }
}
```

### 4.2 Key Endpoints

**Monitoring**:
- `GET /api/monitor/accounts` - Get all accounts
- `GET /api/monitor/accounts/{id}` - Get account details
- `GET /api/monitor/heartbeats` - Get heartbeat status
- `GET /api/monitor/errors` - Get errors
- `GET /api/monitor/system/health` - Get system health

**Optimization**:
- `POST /api/optimization/start` - Start optimization
- `GET /api/optimization/{id}` - Get optimization details
- `GET /api/optimization/{id}/results` - Get results
- `POST /api/optimization/{id}/cancel` - Cancel optimization

**Backtesting**:
- `POST /api/backtest/start` - Start backtest
- `GET /api/backtest/{id}` - Get backtest details
- `GET /api/backtest/{id}/results` - Get results
- `GET /api/backtest/{id}/report` - Generate report

**Reports**:
- `GET /api/reports/performance` - Performance report
- `GET /api/reports/trades` - Trade analysis
- `POST /api/reports/generate` - Generate custom report

**Alerts**:
- `GET /api/alerts` - Get alerts
- `POST /api/alerts/rules` - Create alert rule
- `POST /api/alerts/{id}/acknowledge` - Acknowledge alert

### 4.3 WebSocket Events

**Client → Server**:
- `subscribe:accounts` - Subscribe to account updates
- `subscribe:heartbeats` - Subscribe to heartbeat updates
- `subscribe:errors` - Subscribe to error updates
- `subscribe:optimization:{id}` - Subscribe to optimization updates
- `subscribe:backtest:{id}` - Subscribe to backtest updates

**Server → Client**:
- `account:update` - Account data updated
- `heartbeat:update` - Heartbeat status updated
- `error:new` - New error detected
- `optimization:progress` - Optimization progress update
- `backtest:progress` - Backtest progress update
- `alert:new` - New alert generated

---

## 5. User Interface

### 5.1 Dashboard Layout

**Header**:
- Logo/Branding
- Navigation menu
- User profile
- Notifications bell
- Settings

**Sidebar Navigation**:
- Dashboard (Home)
- Monitor
- Optimization
- Backtesting
- Reports
- Alerts
- Settings

**Main Content Area**:
- Dynamic content based on route
- Cards, charts, tables
- Real-time updates

### 5.2 Key Pages

**1. Dashboard (Home)**
- Account overview cards
- Performance charts
- Recent alerts
- System health status
- Quick actions

**2. Monitor Page**
- Account list/table
- Heartbeat status
- Error log
- Strategy status
- Real-time charts

**3. Optimization Page**
- Optimization setup form
- Active optimizations list
- Results table
- Parameter visualization
- History

**4. Backtesting Page**
- Backtest setup form
- Active backtests list
- Results visualization
- Report viewer
- History

**5. Reports Page**
- Report templates
- Custom report builder
- Report history
- Scheduled reports
- Export options

**6. Alerts Page**
- Alert list (filterable)
- Alert details
- Alert rules
- Alert history
- AI analysis

**7. Settings Page**
- Account configuration
- Notification settings
- AI model configuration
- System settings
- User management

---

## 6. Security

### 6.1 Authentication & Authorization

- **Authentication**: JWT tokens
- **Password**: Bcrypt hashing
- **Session**: Token-based (no server-side sessions)
- **Roles**: Admin, User, Viewer
- **Permissions**: Role-based access control (RBAC)

### 6.2 Data Security

- **Database**: Encrypted at rest
- **API**: HTTPS (TLS 1.3)
- **Secrets**: Environment variables
- **File Access**: Path validation, restricted directories
- **Input Validation**: All inputs sanitized

### 6.3 Network Security

- **Local Access**: `localhost:8000` (default)
- **Network Access**: Optional (firewall rules)
- **HTTPS**: Optional (for production)
- **CORS**: Configured for allowed origins
- **Rate Limiting**: API rate limiting

---

## 7. Performance Requirements

### 7.1 Response Times

- **API Responses**: < 200ms (p95)
- **Dashboard Load**: < 2 seconds
- **Real-time Updates**: < 100ms latency
- **Optimization Start**: < 1 second
- **Backtest Start**: < 1 second

### 7.2 Scalability

- **Concurrent Users**: 10+ (local)
- **Concurrent Optimizations**: 4+ (multi-core)
- **Concurrent Backtests**: 4+ (multi-core)
- **Database Connections**: Connection pooling
- **File Operations**: Async I/O

### 7.3 Resource Usage

- **Memory**: < 2GB (idle), < 4GB (active)
- **CPU**: < 20% (idle), < 80% (active)
- **Disk**: < 10GB (database + files)
- **Network**: Minimal (local only)

---

## 8. Deployment

### 8.1 Development Environment

**Development Platform**: macOS (MacBook) or Windows
- ✅ **macOS Development**: Fully supported for development
  - Python 3.8+ (works on macOS)
  - PostgreSQL 14+ (works on macOS via Homebrew)
  - Node.js 18+ (works on macOS)
  - FastAPI, React/Vue.js (cross-platform)
  - All development tools work on macOS
- ✅ **Windows Development**: Also supported
- ⚠️ **C Library Testing**: Requires Windows for final testing (CTesterFrameworkAPI)

**Development Setup (macOS)**:
```bash
# Install PostgreSQL
brew install postgresql@14

# Install Python dependencies
pip install -r requirements.txt

# Install Node.js (via Homebrew or nvm)
brew install node

# Run development server
python -m uvicorn backend.main:app --reload

# Run frontend dev server
cd frontend && npm run dev
```

### 8.2 Production Environment

**Production Platform**: Windows 10/11 (required)
- Windows 10/11 (required for MetaTrader 4/5)
- Python 3.8+
- PostgreSQL 14+ (local installation)
- Node.js 18+ (for frontend build)
- 8GB+ RAM
- 50GB+ disk space

**Why Windows for Production**:
- MetaTrader 4/5 runs only on Windows
- Direct file access to MT4/5 directories
- C Library (CTesterFrameworkAPI) compiled for Windows
- Trading platform integration requires Windows

### 8.3 Cross-Platform Development Strategy

**Development Workflow**:
1. **Develop on macOS**: Write code, test API, test frontend
2. **Test on Windows**: Final testing with MT4/5 integration
3. **Deploy on Windows**: Production deployment

**Compatibility**:
- ✅ Backend API: Cross-platform (Python, FastAPI)
- ✅ Frontend: Cross-platform (React/Vue.js, Node.js)
- ✅ Database: Cross-platform (PostgreSQL)
- ⚠️ C Library: Windows only (test on Windows)
- ⚠️ File paths: Use pathlib for cross-platform compatibility
- ⚠️ MT4/5 Integration: Windows only (test on Windows)

### 8.4 Installation Steps (Production - Windows)

1. Install PostgreSQL on Windows
2. Create database
3. Install Python dependencies
4. Run database migrations
5. Build frontend
6. Configure environment variables
7. Start services
8. Access dashboard

### 8.5 Development Setup (macOS)

**Prerequisites**:
- macOS 10.15+ (Catalina or later)
- Homebrew (package manager)
- Python 3.8+ (via Homebrew or pyenv)
- Node.js 18+ (via Homebrew or nvm)

**Installation Steps**:
```bash
# 1. Install PostgreSQL
brew install postgresql@14
brew services start postgresql@14

# 2. Create database
createdb management

# 3. Install Python dependencies
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# 4. Run database migrations
alembic upgrade head

# 5. Install Node.js dependencies
cd frontend
npm install

# 6. Configure environment variables
cp .env.example .env
# Edit .env with your settings

# 7. Start development servers
# Terminal 1: Backend
python -m uvicorn backend.main:app --reload --host 0.0.0.0 --port 8000

# Terminal 2: Frontend
cd frontend && npm run dev
```

**Development Notes**:
- ✅ All backend development can be done on macOS
- ✅ All frontend development can be done on macOS
- ✅ API testing works on macOS
- ⚠️ C Library integration testing requires Windows
- ⚠️ MT4/5 file access testing requires Windows
- ✅ Use Docker or VM for Windows testing if needed

### 8.6 Configuration

**Environment Variables**:
```bash
DATABASE_URL=postgresql://user:password@localhost/management
SECRET_KEY=your-secret-key
JWT_SECRET=your-jwt-secret
AI_API_KEY=your-ai-api-key (optional)
TELEGRAM_BOT_TOKEN=your-telegram-token (optional)
```

**Config File** (`config/management.config`):
```ini
[server]
host = 127.0.0.1
port = 8000
debug = false

[database]
url = postgresql://user:password@localhost/management

[files]
mt4_path = C:/Users/.../MetaQuotes/Terminal/...
mt5_path = C:/Users/.../MetaQuotes/Terminal/...

[ai]
enabled = true
provider = openai  # or local
api_key = your-api-key
model = gpt-4
```

---

## 9. Testing

### 9.1 Test Types

- **Unit Tests**: Backend services
- **Integration Tests**: API endpoints
- **E2E Tests**: Frontend workflows
- **Performance Tests**: Load testing
- **Security Tests**: Penetration testing

### 9.2 Test Coverage

- **Backend**: > 80%
- **Frontend**: > 70%
- **Critical Paths**: 100%

---

## 10. Documentation

### 10.1 Required Documentation

- **API Documentation**: OpenAPI/Swagger
- **User Guide**: Dashboard usage
- **Admin Guide**: Configuration, deployment
- **Developer Guide**: Architecture, contribution
- **Troubleshooting Guide**: Common issues

---

## 11. Future Enhancements

### 11.1 Phase 2 Features

- Cloud backup/sync
- Multi-machine support
- Advanced AI models
- Mobile app
- Advanced analytics

### 11.2 Phase 3 Features

- Full cloud deployment
- Distributed optimization
- Real-time market data
- Social trading features
- Advanced reporting

---

## 12. Success Criteria

### 12.1 Functional Requirements

- ✅ All core features working
- ✅ Real-time monitoring
- ✅ Optimization execution
- ✅ Backtest execution
- ✅ Report generation
- ✅ AI alerting

### 12.2 Non-Functional Requirements

- ✅ Performance targets met
- ✅ Security requirements met
- ✅ Documentation complete
- ✅ Test coverage met
- ✅ User acceptance

---

**Document Status**: Specification Complete  
**Next Steps**: Review, approval, implementation planning

