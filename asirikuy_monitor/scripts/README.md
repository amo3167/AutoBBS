# Asirikuy Monitor - Deployment Scripts

This directory contains deployment and management scripts for the Asirikuy Monitor.

## Installation Scripts

### Unix/macOS/Linux

```bash
./scripts/install.sh
```

**Features:**
- Checks Python version (requires 3.8+)
- Creates virtual environment
- Installs dependencies from `requirements.txt`
- Verifies installation
- Creates log directory
- Sets up config file from example

### Windows

```cmd
scripts\install.bat
```

Same functionality as Unix version, adapted for Windows.

## Uninstall Scripts

### Unix/macOS/Linux

```bash
./scripts/uninstall.sh
```

**Features:**
- Removes virtual environment
- Optionally removes log files
- Optionally removes config files
- Preserves source code

### Windows

```cmd
scripts\uninstall.bat
```

Same functionality as Unix version.

## Startup Scripts

### Unix/macOS/Linux

```bash
# Default config
./scripts/run_monitor.sh

# Custom config
./scripts/run_monitor.sh config/my_config.config
```

**Features:**
- Virtual environment detection and activation
- Error handling with automatic restart
- Startup logging to `log/startup.log`
- Config file parameter support
- Platform information logging

### Windows

```cmd
REM Default config
scripts\run_monitor.bat

REM Custom config
scripts\run_monitor.bat config\my_config.config
```

Same functionality as Unix version.

## Systemd Service

For Linux systems with systemd, use the provided service file:

```bash
# Copy service file
sudo cp scripts/asirikuy-monitor.service /etc/systemd/system/

# Edit service file (update paths and user)
sudo nano /etc/systemd/system/asirikuy-monitor.service

# Reload systemd
sudo systemctl daemon-reload

# Enable service (start on boot)
sudo systemctl enable asirikuy-monitor.service

# Start service
sudo systemctl start asirikuy-monitor.service

# Check status
sudo systemctl status asirikuy-monitor.service

# View logs
sudo journalctl -u asirikuy-monitor.service -f
```

**Service Features:**
- Automatic restart on failure (10-second delay)
- Logging to systemd journal
- Security settings (NoNewPrivileges, PrivateTmp)
- Network dependency

## Legacy Startup Scripts

The following scripts in the project root are updated for Python 3:

- `runAsirikyuMonitor.bat` - Default config
- `runAsirikyuMonitor_AT.bat` - AT config
- `runAsirikyuMonitor_AT_Real.bat` - AT Real config

These scripts now include:
- Python 3 support
- Virtual environment detection
- Error handling and restart logic
- Startup logging

## Package Installation

The project can also be installed as a Python package:

```bash
# Install in development mode
pip install -e .

# Install in production mode
pip install .

# Install with dev dependencies
pip install -e ".[dev]"
```

This uses the `pyproject.toml` configuration for setuptools.

## Troubleshooting

### Virtual Environment Not Found

If scripts complain about missing virtual environment:

```bash
# Create virtual environment manually
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt
```

### Permission Denied

Make scripts executable (Unix):

```bash
chmod +x scripts/*.sh
```

### Python Not Found

Ensure Python 3.8+ is installed and in PATH:

```bash
# Check Python version
python3 --version

# Check if python3 is in PATH
which python3  # Unix
where python3  # Windows
```

## See Also

- `README.md` - Main project documentation
- `DEPLOYMENT.md` - Comprehensive deployment guide
- `API.md` - API documentation

