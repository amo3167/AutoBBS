# Monitoring System Upgrade Plan: File-Based → Database + Web Dashboard

## Overview

This document outlines the upgrade plan for `asirikuy_monitor_v05_real` to add:
1. **Database Integration** - Store monitoring data in PostgreSQL
2. **File-to-DB Pipeline** - Feed existing file-based monitoring to database
3. **Local Web Dashboard** - FastAPI web server for real-time monitoring

**Current State**: File-based only (heartbeat files, log files)  
**Target State**: Database-backed with file fallback + web dashboard

---

## Part 1: Current Architecture

### 1.1 Current File-Based Monitoring

**What It Monitors:**
1. **Heartbeat Files** (`.hb` files)
   - Location: `path/MQL4/Files/*.hb` (MT4) or `path/tmp/*.hb` (AT)
   - Format: Contains timestamp of last heartbeat
   - Check: If timestamp > `monitoringInterval * 2.5` seconds ago → alert

2. **Log Files** (Framework logs)
   - Location: `path/MQL4/Logs/AsirikuyFramework.log` (MT4) or `path/log/{accountNumber}AsirikuyFramework.log` (AT)
   - Check: Last line contains `Error`, `Emergency`, or `Critical` → alert

**Current Flow:**
```
Monitoring Loop
    ↓
Check Heartbeat Files (.hb)
    ↓
Check Log Files (.log)
    ↓
Send Email Alert (if issues found)
    ↓
Sleep (monitoringInterval)
```

**Limitations:**
- ❌ No persistent storage
- ❌ No historical data
- ❌ No queryable monitoring history
- ❌ No web interface

---

## Part 2: Upgrade Architecture

### 2.1 Hybrid Approach: File + Database

**Strategy**: Keep file-based monitoring as primary source, feed to database

```
┌─────────────────────────────────────────────────────────────┐
│         Enhanced Monitoring System                            │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Monitoring Core (checker.py)                 │  │
│  │  - Reads heartbeat files (.hb)                      │  │
│  │  - Reads log files (.log)                          │  │
│  │  - Detects issues                                  │  │
│  └───────────────────┬────────────────────────────────┘  │
│                      │                                       │
│                      │ Feed to Database                     │
│                      │                                       │
│  ┌───────────────────▼────────────────────────────────┐  │
│  │         Database Layer (PostgreSQL)                 │  │
│  │  - Stores heartbeat events                         │  │
│  │  - Stores error logs                               │  │
│  │  - Stores alert history                            │  │
│  │  - Historical data                                 │  │
│  └───────────────────┬────────────────────────────────┘  │
│                      │                                       │
│                      │ Query for Dashboard                  │
│                      │                                       │
│  ┌───────────────────▼────────────────────────────────┐  │
│  │         Web Dashboard (FastAPI)                     │  │
│  │  - REST API (read from DB)                         │  │
│  │  - WebSocket (real-time updates)                   │  │
│  │  - HTML Dashboard                                  │  │
│  └────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Key Design Decisions

1. **Keep File-Based Monitoring**
   - ✅ Files are the source of truth (from MT4/AT)
   - ✅ Database is secondary (for storage/querying)
   - ✅ If database fails, monitoring still works
   - ✅ Files act as backup

2. **Feed Files to Database**
   - ✅ Read files → Parse → Store in DB
   - ✅ Store heartbeat events
   - ✅ Store error logs
   - ✅ Store alert history

3. **Local Web Dashboard**
   - ✅ FastAPI web server (localhost)
   - ✅ Reads from database (not files directly)
   - ✅ Real-time updates via WebSocket
   - ✅ Historical charts from database

---

## Part 3: Database Schema

### 3.1 Tables

```sql
-- Heartbeat events
CREATE TABLE heartbeat_events (
    id SERIAL PRIMARY KEY,
    account_name VARCHAR(255) NOT NULL,
    instance_id VARCHAR(255) NOT NULL,
    timestamp TIMESTAMP NOT NULL,
    file_path VARCHAR(512),
    time_diff_seconds FLOAT,
    is_healthy BOOLEAN,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_heartbeat_account_timestamp ON heartbeat_events(account_name, timestamp DESC);
CREATE INDEX idx_heartbeat_healthy ON heartbeat_events(is_healthy, timestamp DESC);

-- Error logs
CREATE TABLE error_logs (
    id SERIAL PRIMARY KEY,
    account_name VARCHAR(255) NOT NULL,
    severity VARCHAR(50) NOT NULL,  -- ERROR, CRITICAL, EMERGENCY
    message TEXT NOT NULL,
    log_file_path VARCHAR(512),
    log_line_number INTEGER,
    timestamp TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_error_account_timestamp ON error_logs(account_name, timestamp DESC);
CREATE INDEX idx_error_severity ON error_logs(severity, timestamp DESC);

-- Alert history
CREATE TABLE alert_history (
    id SERIAL PRIMARY KEY,
    account_name VARCHAR(255) NOT NULL,
    alert_type VARCHAR(50) NOT NULL,  -- heartbeat_failure, error_detected
    alert_message TEXT NOT NULL,
    alert_channel VARCHAR(50),  -- email, slack, telegram
    alert_sent BOOLEAN DEFAULT FALSE,
    alert_sent_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_alert_account_timestamp ON alert_history(account_name, timestamp DESC);
CREATE INDEX idx_alert_type ON alert_history(alert_type, timestamp DESC);

-- Monitoring checks (for metrics)
CREATE TABLE monitoring_checks (
    id SERIAL PRIMARY KEY,
    account_name VARCHAR(255) NOT NULL,
    check_type VARCHAR(50) NOT NULL,  -- heartbeat, error_log
    check_result VARCHAR(50) NOT NULL,  -- success, failure, timeout
    check_duration_ms INTEGER,
    timestamp TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_check_account_timestamp ON monitoring_checks(account_name, timestamp DESC);
```

### 3.2 Database Layer Implementation

```python
# database.py
from sqlalchemy import create_engine, Column, Integer, String, Text, Float, Boolean, DateTime, Index
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from datetime import datetime, timedelta

Base = declarative_base()

class HeartbeatEvent(Base):
    __tablename__ = 'heartbeat_events'
    
    id = Column(Integer, primary_key=True)
    account_name = Column(String(255), nullable=False)
    instance_id = Column(String(255), nullable=False)
    timestamp = Column(DateTime, nullable=False)
    file_path = Column(String(512))
    time_diff_seconds = Column(Float)
    is_healthy = Column(Boolean)
    created_at = Column(DateTime, default=datetime.utcnow)

class ErrorLog(Base):
    __tablename__ = 'error_logs'
    
    id = Column(Integer, primary_key=True)
    account_name = Column(String(255), nullable=False)
    severity = Column(String(50), nullable=False)
    message = Column(Text, nullable=False)
    log_file_path = Column(String(512))
    log_line_number = Column(Integer)
    timestamp = Column(DateTime, nullable=False)
    created_at = Column(DateTime, default=datetime.utcnow)

class AlertHistory(Base):
    __tablename__ = 'alert_history'
    
    id = Column(Integer, primary_key=True)
    account_name = Column(String(255), nullable=False)
    alert_type = Column(String(50), nullable=False)
    alert_message = Column(Text, nullable=False)
    alert_channel = Column(String(50))
    alert_sent = Column(Boolean, default=False)
    alert_sent_at = Column(DateTime)
    created_at = Column(DateTime, default=datetime.utcnow)

class MonitoringCheck(Base):
    __tablename__ = 'monitoring_checks'
    
    id = Column(Integer, primary_key=True)
    account_name = Column(String(255), nullable=False)
    check_type = Column(String(50), nullable=False)
    check_result = Column(String(50), nullable=False)
    check_duration_ms = Column(Integer)
    timestamp = Column(DateTime, nullable=False)
    created_at = Column(DateTime, default=datetime.utcnow)

class MonitoringDatabase:
    def __init__(self, db_url='postgresql://user:pass@localhost/monitoring'):
        self.engine = create_engine(db_url)
        Base.metadata.create_all(self.engine)
        Session = sessionmaker(bind=self.engine)
        self.session = Session()
    
    def store_heartbeat(self, account_name, instance_id, timestamp, file_path, time_diff, is_healthy):
        """Store heartbeat event"""
        event = HeartbeatEvent(
            account_name=account_name,
            instance_id=instance_id,
            timestamp=timestamp,
            file_path=file_path,
            time_diff_seconds=time_diff,
            is_healthy=is_healthy
        )
        self.session.add(event)
        self.session.commit()
        return event.id
    
    def store_error(self, account_name, severity, message, log_file_path, log_line_number, timestamp):
        """Store error log"""
        error = ErrorLog(
            account_name=account_name,
            severity=severity,
            message=message,
            log_file_path=log_file_path,
            log_line_number=log_line_number,
            timestamp=timestamp
        )
        self.session.add(error)
        self.session.commit()
        return error.id
    
    def store_alert(self, account_name, alert_type, alert_message, alert_channel=None):
        """Store alert history"""
        alert = AlertHistory(
            account_name=account_name,
            alert_type=alert_type,
            alert_message=alert_message,
            alert_channel=alert_channel,
            alert_sent=True,
            alert_sent_at=datetime.utcnow()
        )
        self.session.add(alert)
        self.session.commit()
        return alert.id
    
    def store_check(self, account_name, check_type, check_result, check_duration_ms):
        """Store monitoring check"""
        check = MonitoringCheck(
            account_name=account_name,
            check_type=check_type,
            check_result=check_result,
            check_duration_ms=check_duration_ms,
            timestamp=datetime.utcnow()
        )
        self.session.add(check)
        self.session.commit()
        return check.id
    
    def get_latest_heartbeat(self, account_name, instance_id=None):
        """Get latest heartbeat for account"""
        query = self.session.query(HeartbeatEvent).filter(
            HeartbeatEvent.account_name == account_name
        )
        if instance_id:
            query = query.filter(HeartbeatEvent.instance_id == instance_id)
        return query.order_by(HeartbeatEvent.timestamp.desc()).first()
    
    def get_recent_errors(self, account_name, minutes=5):
        """Get recent errors for account"""
        cutoff = datetime.utcnow() - timedelta(minutes=minutes)
        return self.session.query(ErrorLog).filter(
            ErrorLog.account_name == account_name,
            ErrorLog.timestamp >= cutoff
        ).order_by(ErrorLog.timestamp.desc()).all()
    
    def get_heartbeat_history(self, account_name, hours=24):
        """Get heartbeat history for account"""
        cutoff = datetime.utcnow() - timedelta(hours=hours)
        return self.session.query(HeartbeatEvent).filter(
            HeartbeatEvent.account_name == account_name,
            HeartbeatEvent.timestamp >= cutoff
        ).order_by(HeartbeatEvent.timestamp.asc()).all()
    
    def get_alert_history(self, account_name, hours=24):
        """Get alert history for account"""
        cutoff = datetime.utcnow() - timedelta(hours=hours)
        return self.session.query(AlertHistory).filter(
            AlertHistory.account_name == account_name,
            AlertHistory.created_at >= cutoff
        ).order_by(AlertHistory.created_at.desc()).all()
```

---

## Part 4: Enhanced Monitoring Core

### 4.1 Upgraded checker.py

```python
# checker_enhanced.py
from colorama import init, Fore, Back, Style
import logging
import argparse
import sys
import os
from time import strftime, sleep
from datetime import datetime, timedelta
import time
from database import MonitoringDatabase
from alerting import AlertManager

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config-file')
    parser.add_argument('-v', '--version', nargs='*')
    parser.add_argument('--db-url', help='Database URL (optional, defaults to config)')
    args = parser.parse_args()

    if args.version is not None:
        print("Asirikuy Monitor v0.06 (Enhanced with DB)")
        return

    # Colorama Initialization
    init(autoreset=True)
    
    # Load configuration
    if args.config_file:
        configFilePath = args.config_file
    else:
        configFilePath = './config/checker.config'

    config = readConfigFile(configFilePath)
    
    # Configuration
    monitoringInterval = float(config.get('general', 'monitoringInterval'))
    openDay = int(config.get('general', 'weekOpenDay'))
    openHour = int(config.get('general', 'weekOpenHour'))
    closeDay = int(config.get('general', 'weekCloseDay'))
    closeHour = int(config.get('general', 'weekCloseHour'))
    useEmail = int(config.get('general', 'useEmail'))
    fromEmail = config.get('general', 'fromEmail')
    toEmail = config.get('general', 'toEmail')
    emailLogin = config.get('general', 'emailLogin')
    emailPassword = config.get('general', 'emailPassword')
    smtpServer = config.get('general', 'smtpServer')
    
    # Database configuration (optional)
    db_url = args.db_url or config.get('general', 'db_url', fallback=None)
    use_database = config.getboolean('general', 'use_database', fallback=False) if db_url else False
    
    # Initialize database (if enabled)
    db = None
    if use_database and db_url:
        try:
            db = MonitoringDatabase(db_url)
            print(f"{Fore.GREEN}Database connected: {db_url}")
        except Exception as e:
            print(f"{Fore.YELLOW}Warning: Database connection failed: {e}")
            print(f"{Fore.YELLOW}Continuing with file-based monitoring only")
            use_database = False
    
    # Initialize alerting
    alert_manager = AlertManager(config, db)
    
    accountSections = [s.strip() for s in config.get('accounts', 'accounts').split(',')]
    
    # Logging
    LOG_FILENAME = "./log/monitor.log"
    logging.basicConfig(
        filename=LOG_FILENAME,
        filemode='a',
        format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
        datefmt='%H:%M:%S',
        level=logging.ERROR
    )
    logger = logging.getLogger('Monitor')
    logger.info("Monitor starting (Enhanced with DB)")
    
    lastError = [""] * len(accountSections)
    
    while True:
        clearScreen()
        
        localTime = datetime.now()
        currentGmTime = time.gmtime()
        hour = time.localtime().tm_hour
        day = time.localtime().tm_wday
        
        print(f"Local Hour = {hour}")
        print(f"Local Day = {day}")
        
        # Only monitor during trading hours
        if ((hour > openHour and day == openDay) or 
            (hour < closeHour and day == closeDay) or 
            (day > openDay and day < closeDay)):
            
            for i, accountSection in enumerate(accountSections):
                path = config.get(accountSection, 'path')
                frontEnd = config.get(accountSection, 'frontend')
                accountNumber = config.get(accountSection, 'accountNumber')
                
                # Check heartbeat files
                check_start = time.time()
                
                if frontEnd == "MT4":
                    heartbeat_dir = os.path.join(path, 'MQL4', 'Files')
                else:
                    heartbeat_dir = os.path.join(path, 'tmp')
                
                heartbeat_healthy = True
                
                if os.path.exists(heartbeat_dir):
                    os.chdir(heartbeat_dir)
                    
                    for file in os.listdir("."):
                        if file.endswith(".hb"):
                            instance_id = file.replace('.hb', '')
                            
                            try:
                                with open(file, 'r') as fp:
                                    data = fp.readlines()
                                
                                if len(data) >= 2:
                                    date_str = data[1].strip()
                                    heartbeat_time = datetime.strptime(date_str, "%d/%m/%y %H:%M")
                                    
                                    utc_datetime = datetime.utcnow()
                                    time_diff = abs((utc_datetime - heartbeat_time).total_seconds())
                                    
                                    output = f"Checking {file}..."
                                    print(output)
                                    logger.info(output)
                                    
                                    output = f'Current Time: {localTime}, UTC Time: {utc_datetime}, Last heartbeat: {heartbeat_time}, Difference: {time_diff} secs (threshold: {monitoringInterval * 2.5})'
                                    print(output)
                                    logger.info(output)
                                    
                                    # Store in database
                                    if use_database and db:
                                        db.store_heartbeat(
                                            account_name=accountSection,
                                            instance_id=instance_id,
                                            timestamp=heartbeat_time,
                                            file_path=os.path.join(heartbeat_dir, file),
                                            time_diff=time_diff,
                                            is_healthy=(time_diff <= monitoringInterval * 2.5)
                                        )
                                    
                                    # Check if heartbeat is too old
                                    if time_diff > monitoringInterval * 2.5:
                                        heartbeat_healthy = False
                                        output = f"Heart-Beat problem for instance {file}"
                                        print(Style.BRIGHT + Fore.RED + output)
                                        logger.error(output)
                                        
                                        # Store alert in database
                                        if use_database and db:
                                            db.store_alert(
                                                account_name=accountSection,
                                                alert_type='heartbeat_failure',
                                                alert_message=f'Heartbeat timeout: {time_diff} seconds (threshold: {monitoringInterval * 2.5})',
                                                alert_channel='email'
                                            )
                                        
                                        # Send alert
                                        if useEmail == 1:
                                            output = "Sending email message about heart beat..."
                                            print(output)
                                            logger.error(output)
                                            alert_manager.send_alert(
                                                account=accountSection,
                                                alert_type='heartbeat_failure',
                                                message=f"Heart-Beat problem for instance {file}, system not updating for more than {monitoringInterval * 2.5} secs"
                                            )
                                        
                                        # Kill MT4 if needed
                                        if frontEnd == "MT4":
                                            logger.error("Killing MT4")
                                            subprocess.call("taskkill /f /im terminal.exe")
                                        
                                        break
                                    
                            except Exception as e:
                                logger.error(f"Error processing heartbeat file {file}: {e}")
                                heartbeat_healthy = False
                
                # Store monitoring check
                check_duration = int((time.time() - check_start) * 1000)
                if use_database and db:
                    db.store_check(
                        account_name=accountSection,
                        check_type='heartbeat',
                        check_result='success' if heartbeat_healthy else 'failure',
                        check_duration_ms=check_duration
                    )
                
                # Check error logs
                check_start = time.time()
                
                if frontEnd == "MT4":
                    log_file = os.path.join(path, 'MQL4', 'Logs', 'AsirikuyFramework.log')
                else:
                    log_file = os.path.join(path, 'log', f'{accountNumber}AsirikuyFramework.log')
                
                if os.path.exists(log_file):
                    try:
                        with open(log_file, 'r') as f:
                            data = f.readlines()
                        
                        if data:
                            last_line = data[-1]
                            
                            # Check for errors
                            if any(keyword in last_line for keyword in ["Error", "Emergency", "Critical"]):
                                # Determine severity
                                if "Emergency" in last_line:
                                    severity = "EMERGENCY"
                                elif "Critical" in last_line:
                                    severity = "CRITICAL"
                                else:
                                    severity = "ERROR"
                                
                                output = f"Error detected on account {accountSection}"
                                print(Style.BRIGHT + Fore.RED + output)
                                logger.error(output)
                                print(last_line)
                                logger.error(last_line)
                                
                                # Store error in database
                                if use_database and db:
                                    db.store_error(
                                        account_name=accountSection,
                                        severity=severity,
                                        message=last_line.strip(),
                                        log_file_path=log_file,
                                        log_line_number=len(data),
                                        timestamp=datetime.utcnow()
                                    )
                                
                                # Send alert if new error
                                if useEmail == 1 and lastError[i] != last_line:
                                    output = "Sending email message about error..."
                                    logger.error(output)
                                    lastError[i] = last_line
                                    
                                    # Store alert in database
                                    if use_database and db:
                                        db.store_alert(
                                            account_name=accountSection,
                                            alert_type='error_detected',
                                            alert_message=last_line.strip(),
                                            alert_channel='email'
                                        )
                                    
                                    alert_manager.send_alert(
                                        account=accountSection,
                                        alert_type='error_detected',
                                        message=last_line.strip()
                                    )
                                
                                # Kill MT4 if needed
                                if frontEnd == "MT4" and "Error" in last_line:
                                    logger.error("Killing MT4")
                                    subprocess.call("taskkill /f /im terminal.exe")
                    
                    except Exception as e:
                        logger.error(f"Error reading log file {log_file}: {e}")
                
                # Store monitoring check
                check_duration = int((time.time() - check_start) * 1000)
                if use_database and db:
                    db.store_check(
                        account_name=accountSection,
                        check_type='error_log',
                        check_result='success',
                        check_duration_ms=check_duration
                    )
        
        output = f'Last execution {strftime("%a, %d %b %Y %X")}, sleeping for {monitoringInterval} secs.'
        print((Style.BRIGHT + Back.GREEN + output).ljust(100))
        logger.info(output)
        sleep(monitoringInterval)

if __name__ == '__main__':
    main()
```

---

## Part 5: Local Web Dashboard

### 5.1 FastAPI Web Server

```python
# dashboard.py
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
import uvicorn
import asyncio
import json
from datetime import datetime, timedelta
from database import MonitoringDatabase
from typing import List, Optional

app = FastAPI(title="Asirikuy Monitor Dashboard")

# Database connection
db = None

@app.on_event("startup")
async def startup():
    global db
    # Initialize database connection
    db_url = os.getenv('MONITORING_DB_URL', 'postgresql://user:pass@localhost/monitoring')
    db = MonitoringDatabase(db_url)

@app.get("/")
async def dashboard():
    """Main dashboard page"""
    html_content = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>Asirikuy Monitor Dashboard</title>
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        <style>
            body { font-family: Arial, sans-serif; margin: 20px; }
            .status-card { border: 1px solid #ddd; padding: 15px; margin: 10px; border-radius: 5px; }
            .healthy { background-color: #d4edda; border-color: #c3e6cb; }
            .unhealthy { background-color: #f8d7da; border-color: #f5c6cb; }
            .chart-container { width: 100%; height: 300px; margin: 20px 0; }
        </style>
    </head>
    <body>
        <h1>Asirikuy Monitor Dashboard</h1>
        <div id="status"></div>
        <div id="heartbeat-chart" class="chart-container">
            <canvas id="heartbeatChart"></canvas>
        </div>
        <div id="errors"></div>
        <script>
            const ws = new WebSocket('ws://localhost:8000/ws');
            const heartbeatChart = new Chart(document.getElementById('heartbeatChart'), {
                type: 'line',
                data: { labels: [], datasets: [{ label: 'Time Diff (seconds)', data: [] }] },
                options: { responsive: true, maintainAspectRatio: false }
            });
            
            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                updateDashboard(data);
            };
            
            function updateDashboard(data) {
                // Update status cards
                const statusDiv = document.getElementById('status');
                statusDiv.innerHTML = data.accounts.map(acc => `
                    <div class="status-card ${acc.healthy ? 'healthy' : 'unhealthy'}">
                        <h3>${acc.name}</h3>
                        <p>Status: ${acc.healthy ? 'Healthy' : 'Unhealthy'}</p>
                        <p>Last Heartbeat: ${acc.last_heartbeat || 'N/A'}</p>
                        <p>Time Diff: ${acc.time_diff || 'N/A'} seconds</p>
                    </div>
                `).join('');
                
                // Update heartbeat chart
                if (data.heartbeat_history) {
                    heartbeatChart.data.labels = data.heartbeat_history.map(h => h.timestamp);
                    heartbeatChart.data.datasets[0].data = data.heartbeat_history.map(h => h.time_diff_seconds);
                    heartbeatChart.update();
                }
                
                // Update errors
                const errorsDiv = document.getElementById('errors');
                if (data.recent_errors && data.recent_errors.length > 0) {
                    errorsDiv.innerHTML = '<h2>Recent Errors</h2>' + data.recent_errors.map(err => `
                        <div class="status-card unhealthy">
                            <p><strong>${err.severity}</strong>: ${err.message}</p>
                            <p>Time: ${err.timestamp}</p>
                        </div>
                    `).join('');
                }
            }
        </script>
    </body>
    </html>
    """
    return HTMLResponse(content=html_content)

@app.get("/api/status")
async def get_status():
    """Get current monitoring status for all accounts"""
    accounts = []
    
    # Get all accounts from database
    # For now, return mock data - would query from config or database
    return {"accounts": accounts, "timestamp": datetime.utcnow().isoformat()}

@app.get("/api/accounts/{account_name}/heartbeat")
async def get_heartbeat_history(account_name: str, hours: int = 24):
    """Get heartbeat history for account"""
    if not db:
        return {"error": "Database not initialized"}
    
    history = db.get_heartbeat_history(account_name, hours=hours)
    return {
        "account": account_name,
        "history": [
            {
                "timestamp": h.timestamp.isoformat(),
                "time_diff_seconds": h.time_diff_seconds,
                "is_healthy": h.is_healthy
            }
            for h in history
        ]
    }

@app.get("/api/accounts/{account_name}/errors")
async def get_recent_errors(account_name: str, minutes: int = 60):
    """Get recent errors for account"""
    if not db:
        return {"error": "Database not initialized"}
    
    errors = db.get_recent_errors(account_name, minutes=minutes)
    return {
        "account": account_name,
        "errors": [
            {
                "severity": e.severity,
                "message": e.message,
                "timestamp": e.timestamp.isoformat(),
                "log_file": e.log_file_path
            }
            for e in errors
        ]
    }

@app.get("/api/accounts/{account_name}/alerts")
async def get_alert_history(account_name: str, hours: int = 24):
    """Get alert history for account"""
    if not db:
        return {"error": "Database not initialized"}
    
    alerts = db.get_alert_history(account_name, hours=hours)
    return {
        "account": account_name,
        "alerts": [
            {
                "type": a.alert_type,
                "message": a.alert_message,
                "channel": a.alert_channel,
                "sent_at": a.alert_sent_at.isoformat() if a.alert_sent_at else None,
                "created_at": a.created_at.isoformat()
            }
            for a in alerts
        ]
    }

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket for real-time updates"""
    await websocket.accept()
    try:
        while True:
            # Get current status from database
            data = await get_current_status()
            await websocket.send_json(data)
            await asyncio.sleep(1)  # Update every second
    except WebSocketDisconnect:
        pass

async def get_current_status():
    """Get current monitoring status"""
    # Query database for latest status
    # This would be implemented based on your account configuration
    return {
        "accounts": [],
        "heartbeat_history": [],
        "recent_errors": [],
        "timestamp": datetime.utcnow().isoformat()
    }

if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8000)
```

### 5.2 Configuration Update

```ini
# checker.config (enhanced)
[general]
monitoringInterval = 60
useEmail = 1
fromEmail = amo3167@gmail.com
toEmail = amo3167@gmail.com
emailLogin = amo3167@gmail.com
emailPassword = Poilkjmnb1
smtpServer = smtp.gmail.com:587
weekOpenDay = 0
weekOpenHour = 6
weekCloseDay = 5
weekCloseHour = 7

# Database configuration (new)
use_database = yes
db_url = postgresql://user:password@localhost/monitoring

# Web dashboard configuration (new)
enable_dashboard = yes
dashboard_port = 8000
dashboard_host = 127.0.0.1

[accounts]
accounts = pepperstone

[pepperstone]
accountNumber = 505225
path = C:\Users\amo3167\AppData\Roaming\MetaQuotes\Terminal\3294B546D50FEEDA6BF3CFC7CF858DB7
frontend = MT4
```

---

## Part 6: Implementation Plan

### 6.1 Phase 1: Database Integration (1-2 weeks)

**Goal**: Add database support while keeping file-based monitoring

**Tasks**:
1. Set up PostgreSQL database
2. Create database schema (tables, indexes)
3. Implement `database.py` (MonitoringDatabase class)
4. Update `checker.py` to store data in database
5. Test database storage
6. Verify file-based monitoring still works

**Deliverables**:
- ✅ Database schema created
- ✅ Database layer implemented
- ✅ Monitoring data stored in database
- ✅ File-based monitoring still works

### 6.2 Phase 2: Web Dashboard (2-3 weeks)

**Goal**: Create local web dashboard

**Tasks**:
1. Set up FastAPI web server
2. Create REST API endpoints
3. Create HTML dashboard
4. Add WebSocket for real-time updates
5. Add charts (Chart.js)
6. Test dashboard functionality

**Deliverables**:
- ✅ Web dashboard accessible at `http://localhost:8000`
- ✅ REST API endpoints working
- ✅ WebSocket real-time updates
- ✅ Historical charts

### 6.3 Phase 3: Python 3 Migration (1 week)

**Goal**: Migrate from Python 2 to Python 3

**Tasks**:
1. Update print statements
2. Update imports (ConfigParser → configparser)
3. Update string handling
4. Test compatibility
5. Update dependencies

**Deliverables**:
- ✅ Python 3 compatible
- ✅ All features working
- ✅ Dependencies updated

**Total Timeline**: 4-6 weeks

---

## Part 7: Benefits of This Approach

### 7.1 Hybrid File + Database

**Advantages:**
- ✅ **Backward Compatible**: File-based monitoring still works
- ✅ **Resilient**: If database fails, monitoring continues
- ✅ **Historical Data**: Database stores all monitoring events
- ✅ **Queryable**: Can query historical data
- ✅ **Scalable**: Database handles large amounts of data

### 7.2 Local Web Dashboard

**Advantages:**
- ✅ **Real-time Monitoring**: WebSocket updates
- ✅ **Historical Charts**: Visualize trends
- ✅ **Easy Access**: Browser-based interface
- ✅ **No External Dependencies**: Runs locally
- ✅ **Secure**: Localhost only (not exposed to internet)

### 7.3 Incremental Upgrade

**Advantages:**
- ✅ **Low Risk**: Can disable database if needed
- ✅ **Gradual Migration**: Can add features incrementally
- ✅ **Testing**: Can test database separately
- ✅ **Rollback**: Can revert to file-only if needed

---

## Part 8: Usage

### 8.1 Running Enhanced Monitor

```bash
# With database (default if configured)
python checker_enhanced.py -c config/checker.config

# Without database (file-based only)
python checker_enhanced.py -c config/checker.config --no-db

# With custom database URL
python checker_enhanced.py -c config/checker.config --db-url postgresql://user:pass@localhost/monitoring
```

### 8.2 Running Web Dashboard

```bash
# Start dashboard server
python dashboard.py

# Or with uvicorn
uvicorn dashboard:app --host 127.0.0.1 --port 8000

# Access dashboard
# Open browser: http://localhost:8000
```

### 8.3 Monitoring Flow

```
1. Monitor reads files (.hb, .log)
   ↓
2. Detects issues (heartbeat timeout, errors)
   ↓
3. Stores events in database (if enabled)
   ↓
4. Sends alerts (email, etc.)
   ↓
5. Dashboard reads from database
   ↓
6. Dashboard displays real-time status
```

---

## Part 9: Summary

### Current State
- ✅ File-based monitoring (heartbeat files, log files)
- ✅ Email alerting
- ✅ Python 2

### Target State
- ✅ **File-based monitoring** (kept as primary source)
- ✅ **Database storage** (feeds from files)
- ✅ **Local web dashboard** (reads from database)
- ✅ **Python 3** (migrated)
- ✅ **Email alerting** (enhanced with database tracking)

### Key Features
1. **Hybrid Approach**: Files → Database → Dashboard
2. **Backward Compatible**: File monitoring still works
3. **Local Dashboard**: FastAPI web server (localhost)
4. **Real-time Updates**: WebSocket
5. **Historical Data**: Queryable from database

### Timeline
- **Phase 1**: Database integration (1-2 weeks)
- **Phase 2**: Web dashboard (2-3 weeks)
- **Phase 3**: Python 3 migration (1 week)
- **Total**: 4-6 weeks

---

**Document Status**: Implementation Plan
**Last Updated**: December 2024
**Related Documents**:
- `MONITORING_SYSTEM_ANALYSIS.md` - Current system analysis
- `PYTHON_INTEGRATION_DESIGN.md` - Integration design requirements

