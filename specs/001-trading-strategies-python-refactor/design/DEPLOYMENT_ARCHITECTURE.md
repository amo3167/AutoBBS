# Deployment Architecture: Local vs Cloud Management System

## Overview

This document addresses deployment options for the Management System:
1. **Local Web Server** (Initial/Recommended) - Management system runs on same machine as MQL4/5
2. **Cloud Management System** (Future) - Management system in cloud, MQL4/5 on local machine
3. **File Transfer Strategies** - How to sync files between local and cloud

**Key Question**: Can Management System be in cloud? How to transfer local files?

---

## Part 1: Deployment Options

### 1.1 Option 1: Local Web Server (Recommended Initially)

**Architecture**:
```
┌─────────────────────────────────────────────────────────────┐
│         Local Windows Machine                                │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Management System (Local)                      │  │
│  │  - Web Dashboard (localhost:8000)                      │  │
│  │  - Database (PostgreSQL local)                         │  │
│  │  - File System (direct access)                         │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│        ┌─────────────┼─────────────┐                         │
│        │             │             │                         │
│  ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐                │
│  │  Monitor  │ │   ML      │ │ Framework │                │
│  │ (File+DB) │ │  Service  │ │  (C Lib)  │                │
│  └─────┬─────┘ └─────┬─────┘ └─────┬─────┘                │
│        │             │             │                         │
│        └─────────────┼─────────────┘                         │
│                      │                                       │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │         MQL4/5 Execution                              │  │
│  │  - MetaTrader 4/5                                     │  │
│  │  - EAs running                                        │  │
│  │  - Files: .set, .log, .hb, .state                    │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

**Advantages**:
- ✅ **Simple Setup**: Everything on one machine
- ✅ **Direct File Access**: No network transfer needed
- ✅ **Fast**: No latency, local database
- ✅ **Secure**: No external exposure
- ✅ **No Cloud Costs**: Runs locally
- ✅ **Easy to Start**: Can begin immediately

**Disadvantages**:
- ⚠️ **Single Point of Failure**: If machine goes down, everything stops
- ⚠️ **Limited Access**: Only accessible from local network (or VPN)
- ⚠️ **Resource Usage**: Uses local machine resources

**Access**:
- **Local**: `http://localhost:8000`
- **Same Network**: `http://192.168.1.100:8000` (local IP)
- **Remote**: VPN required for secure access

**File Access**:
- ✅ Direct file system access
- ✅ No transfer needed
- ✅ Real-time file updates

**Is this correct for initial setup?** ✅ **YES** - This is the recommended initial approach.

---

### 1.2 Option 2: Cloud Management System (Future)

**Architecture**:
```
┌─────────────────────────────────────────────────────────────┐
│         Cloud Server (AWS/Azure/GCP/VPS)                     │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Management System (Cloud)                      │  │
│  │  - Web Dashboard (public IP:8000)                    │  │
│  │  - Database (PostgreSQL cloud)                        │  │
│  │  - File Sync Agent                                    │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│                      │ API/File Sync                         │
│                      │                                       │
└──────────────────────┼───────────────────────────────────────┘
                       │
                       │ Internet/VPN
                       │
┌──────────────────────▼───────────────────────────────────────┐
│         Local Windows Machine                                │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         File Sync Agent (Local)                       │  │
│  │  - Syncs files to cloud                               │  │
│  │  - Receives commands from cloud                       │  │
│  └───────────────────┬──────────────────────────────────┘  │
│                      │                                       │
│  ┌───────────────────▼──────────────────────────────────┐  │
│  │         MQL4/5 Execution                              │  │
│  │  - MetaTrader 4/5                                     │  │
│  │  - EAs running                                        │  │
│  │  - Files: .set, .log, .hb, .state                    │  │
│  └──────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

**Advantages**:
- ✅ **Remote Access**: Access from anywhere
- ✅ **High Availability**: Cloud infrastructure
- ✅ **Scalability**: Can manage multiple local machines
- ✅ **Backup**: Cloud database backup
- ✅ **No Local Resource Usage**: Management runs in cloud

**Disadvantages**:
- ⚠️ **Complex Setup**: Requires file sync, API, security
- ⚠️ **Network Dependency**: Requires internet connection
- ⚠️ **Latency**: Network latency for file operations
- ⚠️ **Security**: Need secure connection (VPN/HTTPS)
- ⚠️ **Cloud Costs**: Monthly cloud hosting costs

**Access**:
- **Public**: `https://your-domain.com:8000` (with authentication)
- **VPN**: `https://vpn.your-domain.com:8000`

**File Access**:
- ⚠️ Requires file sync agent
- ⚠️ Network transfer needed
- ⚠️ Not real-time (depends on sync frequency)

---

## Part 2: File Transfer Strategies

### 2.1 Strategy 1: File Sync Agent (Recommended for Cloud)

**Purpose**: Sync local files to cloud management system

**Architecture**:
```
Local Machine
    ↓
File Sync Agent (runs locally)
    ├── Watches file changes (.set, .log, .hb, .state)
    ├── Uploads to cloud via API
    ├── Downloads config changes from cloud
    └── Applies changes to local files
    ↓
Cloud Management System
    ├── Receives file updates
    ├── Stores in database
    └── Serves via web dashboard
```

**Implementation**:
```python
# file_sync_agent.py
import watchdog
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import requests
import time

class FileSyncHandler(FileSystemEventHandler):
    def __init__(self, cloud_api_url, account_name):
        self.cloud_api_url = cloud_api_url
        self.account_name = account_name
        self.api_key = os.getenv('SYNC_API_KEY')
    
    def on_modified(self, event):
        """File modified - sync to cloud"""
        if event.is_directory:
            return
        
        file_path = event.src_path
        
        # Only sync relevant files
        if any(file_path.endswith(ext) for ext in ['.set', '.log', '.hb', '.state', '.xml']):
            self.sync_file_to_cloud(file_path)
    
    def sync_file_to_cloud(self, file_path):
        """Upload file to cloud"""
        try:
            with open(file_path, 'rb') as f:
                files = {'file': f}
                data = {
                    'account_name': self.account_name,
                    'file_path': file_path,
                    'timestamp': time.time()
                }
                headers = {'Authorization': f'Bearer {self.api_key}'}
                
                response = requests.post(
                    f'{self.cloud_api_url}/api/sync/upload',
                    files=files,
                    data=data,
                    headers=headers
                )
                
                if response.status_code == 200:
                    print(f"Synced {file_path} to cloud")
                else:
                    print(f"Failed to sync {file_path}: {response.status_code}")
        except Exception as e:
            print(f"Error syncing {file_path}: {e}")

def main():
    # Watch MQL4/5 directories
    watch_dirs = [
        'C:/Users/amo3167/AppData/Roaming/MetaQuotes/Terminal/.../MQL4/Files',
        'C:/Users/amo3167/AppData/Roaming/MetaQuotes/Terminal/.../MQL4/Logs',
        # ... other directories
    ]
    
    cloud_api_url = os.getenv('CLOUD_API_URL', 'https://your-cloud-server.com')
    account_name = os.getenv('ACCOUNT_NAME', 'pepperstone')
    
    event_handler = FileSyncHandler(cloud_api_url, account_name)
    observer = Observer()
    
    for watch_dir in watch_dirs:
        if os.path.exists(watch_dir):
            observer.schedule(event_handler, watch_dir, recursive=True)
    
    observer.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
```

### 2.2 Strategy 2: API-Based File Access

**Purpose**: Cloud management system requests files via API

**Architecture**:
```
Cloud Management System
    ↓
Requests file via API
    ↓
Local File Sync Agent
    ↓
Reads file from local filesystem
    ↓
Returns file content via API
    ↓
Cloud Management System
    ↓
Stores in database / displays
```

**Implementation**:
```python
# local_file_api.py (runs on local machine)
from fastapi import FastAPI, HTTPException
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
import os

app = FastAPI(title="Local File API")

security = HTTPBearer()

@app.get("/api/files/read")
async def read_file(file_path: str, credentials: HTTPAuthorizationCredentials = security):
    """Read file from local filesystem"""
    # Verify API key
    if credentials.credentials != os.getenv('LOCAL_API_KEY'):
        raise HTTPException(status_code=401, detail="Invalid API key")
    
    # Security: Only allow files in allowed directories
    allowed_dirs = [
        'C:/Users/amo3167/AppData/Roaming/MetaQuotes/Terminal/...',
        # ... other allowed directories
    ]
    
    if not any(file_path.startswith(d) for d in allowed_dirs):
        raise HTTPException(status_code=403, detail="File path not allowed")
    
    if not os.path.exists(file_path):
        raise HTTPException(status_code=404, detail="File not found")
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    return {
        "file_path": file_path,
        "content": content,
        "size": os.path.getsize(file_path),
        "modified": os.path.getmtime(file_path)
    }

@app.post("/api/files/write")
async def write_file(file_path: str, content: str, credentials: HTTPAuthorizationCredentials = security):
    """Write file to local filesystem"""
    # Verify API key
    if credentials.credentials != os.getenv('LOCAL_API_KEY'):
        raise HTTPException(status_code=401, detail="Invalid API key")
    
    # Security checks...
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    return {"status": "success", "file_path": file_path}
```

### 2.3 Strategy 3: Database-Only (No File Transfer)

**Purpose**: Store file contents in database, sync periodically

**Architecture**:
```
Local Machine
    ↓
File Sync Agent
    ├── Reads files periodically
    ├── Stores content in database
    └── Syncs database to cloud
    ↓
Cloud Management System
    ├── Reads from cloud database
    └── Displays in dashboard
```

**Advantages**:
- ✅ No file transfer needed
- ✅ Database replication
- ✅ Version history

**Disadvantages**:
- ⚠️ Not real-time
- ⚠️ Large files (logs) may be too big for database

---

## Part 3: Recommended Deployment Strategy

### 3.1 Phase 1: Local Web Server (Initial)

**Timeline**: Start immediately

**Setup**:
1. Install PostgreSQL locally
2. Run Management System on local machine
3. Access via `http://localhost:8000`
4. Direct file access (no transfer needed)

**Benefits**:
- ✅ Simple setup
- ✅ Fast development
- ✅ No network issues
- ✅ Can start immediately

**Access Options**:
- **Local**: `http://localhost:8000`
- **Same Network**: `http://192.168.1.100:8000` (if you want to access from other devices on same network)
- **Remote**: Set up VPN for secure remote access

### 3.2 Phase 2: Hybrid (Local + Cloud Sync)

**Timeline**: After Phase 1 is stable

**Setup**:
1. Keep Management System local (primary)
2. Add cloud backup/sync
3. Optional: Cloud read-only dashboard

**Architecture**:
```
Local Machine (Primary)
    ├── Management System (full access)
    ├── Direct file access
    └── Sync to cloud (backup)
    ↓
Cloud Server (Backup/Read-only)
    ├── Database replica
    ├── File backups
    └── Read-only dashboard (optional)
```

**Benefits**:
- ✅ Best of both worlds
- ✅ Local performance
- ✅ Cloud backup
- ✅ Optional remote access

### 3.3 Phase 3: Full Cloud (Future)

**Timeline**: When needed for multi-machine management

**Setup**:
1. Move Management System to cloud
2. File Sync Agent on local machine
3. Secure API communication

**Benefits**:
- ✅ Centralized management
- ✅ Multiple local machines
- ✅ High availability

---

## Part 4: File Transfer Implementation

### 4.1 File Sync Agent (Local → Cloud)

**Features**:
1. **File Watching**
   - Watches MQL4/5 directories
   - Detects file changes
   - Syncs to cloud

2. **Selective Sync**
   - Only syncs relevant files (`.set`, `.log`, `.hb`, `.state`, `.xml`)
   - Ignores temporary files
   - Configurable file patterns

3. **Incremental Sync**
   - Only syncs changed files
   - Checks file modification time
   - Compresses large files

4. **Bidirectional Sync**
   - Upload: Local → Cloud
   - Download: Cloud → Local (config changes)

**Implementation**:
```python
# file_sync_agent.py
import os
import time
import hashlib
import requests
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from pathlib import Path

class FileSyncAgent:
    def __init__(self, config):
        self.cloud_api_url = config.cloud_api_url
        self.api_key = config.api_key
        self.account_name = config.account_name
        self.watch_dirs = config.watch_dirs
        self.sync_interval = config.sync_interval  # seconds
        self.last_sync = {}  # Track last sync time per file
    
    def sync_file(self, file_path, direction='upload'):
        """Sync file to/from cloud"""
        if direction == 'upload':
            return self.upload_file(file_path)
        else:
            return self.download_file(file_path)
    
    def upload_file(self, file_path):
        """Upload file to cloud"""
        try:
            # Check if file changed
            file_mtime = os.path.getmtime(file_path)
            if file_path in self.last_sync and self.last_sync[file_path] >= file_mtime:
                return  # File not changed, skip
            
            # Read file
            with open(file_path, 'rb') as f:
                file_content = f.read()
            
            # Calculate hash
            file_hash = hashlib.md5(file_content).hexdigest()
            
            # Upload to cloud
            files = {'file': (os.path.basename(file_path), file_content)}
            data = {
                'account_name': self.account_name,
                'file_path': file_path,
                'file_hash': file_hash,
                'timestamp': file_mtime
            }
            headers = {'Authorization': f'Bearer {self.api_key}'}
            
            response = requests.post(
                f'{self.cloud_api_url}/api/sync/upload',
                files=files,
                data=data,
                headers=headers,
                timeout=30
            )
            
            if response.status_code == 200:
                self.last_sync[file_path] = file_mtime
                return True
            else:
                print(f"Failed to upload {file_path}: {response.status_code}")
                return False
                
        except Exception as e:
            print(f"Error uploading {file_path}: {e}")
            return False
    
    def download_file(self, file_path):
        """Download file from cloud"""
        try:
            headers = {'Authorization': f'Bearer {self.api_key}'}
            params = {
                'account_name': self.account_name,
                'file_path': file_path
            }
            
            response = requests.get(
                f'{self.cloud_api_url}/api/sync/download',
                params=params,
                headers=headers,
                timeout=30
            )
            
            if response.status_code == 200:
                # Write file
                os.makedirs(os.path.dirname(file_path), exist_ok=True)
                with open(file_path, 'wb') as f:
                    f.write(response.content)
                return True
            else:
                print(f"Failed to download {file_path}: {response.status_code}")
                return False
                
        except Exception as e:
            print(f"Error downloading {file_path}: {e}")
            return False
    
    def start_watching(self):
        """Start watching file changes"""
        event_handler = FileChangeHandler(self)
        observer = Observer()
        
        for watch_dir in self.watch_dirs:
            if os.path.exists(watch_dir):
                observer.schedule(event_handler, watch_dir, recursive=True)
                print(f"Watching: {watch_dir}")
        
        observer.start()
        return observer

class FileChangeHandler(FileSystemEventHandler):
    def __init__(self, sync_agent):
        self.sync_agent = sync_agent
    
    def on_modified(self, event):
        if not event.is_directory:
            # Only sync relevant files
            if any(event.src_path.endswith(ext) for ext in ['.set', '.log', '.hb', '.state', '.xml']):
                self.sync_agent.sync_file(event.src_path, direction='upload')
    
    def on_created(self, event):
        if not event.is_directory:
            if any(event.src_path.endswith(ext) for ext in ['.set', '.log', '.hb', '.state', '.xml']):
                self.sync_agent.sync_file(event.src_path, direction='upload')
```

### 4.2 Cloud API Endpoints

```python
# cloud_management_api.py
from fastapi import FastAPI, UploadFile, File, HTTPException
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
import os

app = FastAPI(title="Cloud Management API")

security = HTTPBearer()

@app.post("/api/sync/upload")
async def upload_file(
    account_name: str,
    file_path: str,
    file_hash: str,
    file: UploadFile = File(...),
    credentials: HTTPAuthorizationCredentials = security
):
    """Receive file from local sync agent"""
    # Verify API key
    if credentials.credentials != os.getenv('SYNC_API_KEY'):
        raise HTTPException(status_code=401, detail="Invalid API key")
    
    # Store file
    storage_path = f"/storage/{account_name}/{file_path}"
    os.makedirs(os.path.dirname(storage_path), exist_ok=True)
    
    with open(storage_path, 'wb') as f:
        content = await file.read()
        f.write(content)
    
    # Store metadata in database
    db.store_file_metadata(account_name, file_path, file_hash, storage_path)
    
    return {"status": "success", "file_path": file_path}

@app.get("/api/sync/download")
async def download_file(
    account_name: str,
    file_path: str,
    credentials: HTTPAuthorizationCredentials = security
):
    """Send file to local sync agent"""
    # Verify API key
    if credentials.credentials != os.getenv('SYNC_API_KEY'):
        raise HTTPException(status_code=401, detail="Invalid API key")
    
    # Get file from storage
    storage_path = f"/storage/{account_name}/{file_path}"
    
    if not os.path.exists(storage_path):
        raise HTTPException(status_code=404, detail="File not found")
    
    return FileResponse(storage_path)
```

---

## Part 5: Security Considerations

### 5.1 Local Web Server Security

**Access Control**:
- ✅ Authentication (username/password)
- ✅ HTTPS (SSL/TLS) for local network
- ✅ Role-based access control
- ✅ Audit logging

**Network Security**:
- ✅ Firewall rules (only allow specific IPs)
- ✅ VPN for remote access
- ✅ No public internet exposure

### 5.2 Cloud Management System Security

**Access Control**:
- ✅ Strong authentication (JWT tokens)
- ✅ HTTPS (SSL/TLS) required
- ✅ API key authentication for file sync
- ✅ Role-based access control

**Data Security**:
- ✅ Encrypted file transfer (HTTPS)
- ✅ Encrypted database (at rest)
- ✅ Secure API keys (environment variables)
- ✅ File path validation (prevent directory traversal)

**Network Security**:
- ✅ VPN for file sync (recommended)
- ✅ IP whitelisting
- ✅ Rate limiting
- ✅ DDoS protection

---

## Part 6: Migration Path

### 6.1 Step 1: Start with Local (Recommended)

**Why**:
- ✅ Simplest setup
- ✅ Fastest to implement
- ✅ No network dependencies
- ✅ Can start immediately

**Setup**:
1. Install PostgreSQL locally
2. Run Management System on local machine
3. Access via `http://localhost:8000`
4. Direct file access

**Timeline**: Can start immediately

### 6.2 Step 2: Add Remote Access (Optional)

**Why**:
- ✅ Access from other devices
- ✅ Still local performance
- ✅ Secure via VPN

**Setup**:
1. Set up VPN (WireGuard, OpenVPN)
2. Configure firewall rules
3. Access via VPN: `http://192.168.1.100:8000`

**Timeline**: 1-2 days setup

### 6.3 Step 3: Add Cloud Backup (Optional)

**Why**:
- ✅ Data backup
- ✅ Disaster recovery
- ✅ Optional read-only cloud dashboard

**Setup**:
1. Set up cloud server (VPS)
2. Database replication
3. File backup sync

**Timeline**: 1-2 weeks

### 6.4 Step 4: Move to Cloud (Future)

**Why**:
- ✅ Centralized management
- ✅ Multiple local machines
- ✅ High availability

**Setup**:
1. Deploy Management System to cloud
2. Install File Sync Agent on local machine
3. Configure secure API communication

**Timeline**: 2-4 weeks

---

## Part 7: Recommended Initial Setup

### 7.1 Local Web Server Setup

**Architecture**:
```
Windows Machine (Local)
├── MetaTrader 4/5
│   ├── EAs running
│   └── Files (.set, .log, .hb, .state)
├── PostgreSQL (Local)
│   └── Database
├── Management System (Local)
│   ├── FastAPI web server (localhost:8000)
│   ├── Direct file access
│   └── Database access
└── Monitor (Local)
    ├── File monitoring
    └── Database storage
```

**Access**:
- **Local**: `http://localhost:8000`
- **Same Network**: `http://192.168.1.100:8000` (if you want to access from phone/tablet on same WiFi)

**File Access**:
- ✅ Direct file system access
- ✅ No transfer needed
- ✅ Real-time updates

**Is this correct?** ✅ **YES** - This is the recommended initial approach.

### 7.2 Configuration

```ini
# management.config
[server]
host = 127.0.0.1  # localhost only
port = 8000
enable_remote_access = no  # Set to yes if you want same-network access

[database]
url = postgresql://user:password@localhost/management
# Local PostgreSQL

[files]
# Direct file paths (no sync needed)
mt4_path = C:/Users/amo3167/AppData/Roaming/MetaQuotes/Terminal/...
mt5_path = C:/Users/amo3167/AppData/Roaming/MetaQuotes/Terminal/...

[security]
# For local access, simple auth is OK
# For remote access, use HTTPS + strong auth
require_https = no  # Set to yes for remote access
```

---

## Part 8: Cloud Deployment (Future)

### 8.1 When to Move to Cloud

**Consider Cloud When**:
- ✅ Managing multiple local machines
- ✅ Need 24/7 availability
- ✅ Want centralized management
- ✅ Need remote access from anywhere

**Stay Local When**:
- ✅ Single machine
- ✅ Local access is sufficient
- ✅ Want simplicity
- ✅ Want to avoid cloud costs

### 8.2 Cloud Architecture

```
Cloud Server (VPS/AWS/Azure)
├── Management System
│   ├── Web Dashboard (public IP)
│   ├── Database (PostgreSQL)
│   └── File Storage
└── File Sync API

    ↓ (Secure API)

Local Windows Machine
├── File Sync Agent
│   ├── Watches file changes
│   ├── Uploads to cloud
│   └── Downloads configs
└── MQL4/5 Execution
    └── Files (.set, .log, .hb, .state)
```

### 8.3 File Transfer Flow

```
1. Local File Changes
   ↓
2. File Sync Agent detects
   ↓
3. Uploads to Cloud via HTTPS API
   ↓
4. Cloud stores in database/storage
   ↓
5. Management System reads from database
   ↓
6. Displays in web dashboard
```

**Reverse Flow** (Config Changes):
```
1. User changes config in Cloud Dashboard
   ↓
2. Cloud stores in database
   ↓
3. File Sync Agent polls for changes
   ↓
4. Downloads config file
   ↓
5. Writes to local filesystem
   ↓
6. MQL4/5 EA reloads config
```

---

## Part 9: Summary

### 9.1 Initial Setup (Recommended)

**Answer to your question**: ✅ **YES, you're correct!**

**Initial Setup**: Local Web Server
- ✅ Management System runs on same machine as MQL4/5
- ✅ Direct file access (no transfer needed)
- ✅ Access via `http://localhost:8000`
- ✅ Simple, fast, secure

**Why Local First**:
- ✅ Simplest setup
- ✅ No network dependencies
- ✅ Fastest performance
- ✅ Can start immediately
- ✅ No cloud costs

### 9.2 Future: Cloud Option

**Can Management System be in Cloud?** ✅ **YES**

**How to Transfer Files?**
1. **File Sync Agent** (recommended)
   - Watches local files
   - Uploads to cloud via API
   - Downloads config changes

2. **API-Based Access**
   - Cloud requests files via API
   - Local agent serves files

3. **Database-Only**
   - Store file contents in database
   - Sync database to cloud

### 9.3 Migration Path

**Phase 1**: Local Web Server (Start here)
- ✅ Management System local
- ✅ Direct file access
- ✅ `http://localhost:8000`

**Phase 2**: Add Remote Access (Optional)
- ✅ VPN for secure remote access
- ✅ Still local performance

**Phase 3**: Cloud Backup (Optional)
- ✅ Database replication
- ✅ File backups

**Phase 4**: Full Cloud (Future)
- ✅ Management System in cloud
- ✅ File Sync Agent on local
- ✅ Centralized management

---

**Document Status**: Deployment Architecture Guide
**Last Updated**: December 2024
**Related Documents**:
- `COMPLETE_ARCHITECTURE_DESIGN.md` - Complete architecture
- `MONITORING_UPGRADE_PLAN.md` - Monitor upgrade

