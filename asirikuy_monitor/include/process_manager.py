"""
Cross-platform process management utilities
Replaces Windows-specific taskkill command
"""

import psutil
import logging
from include.platform_utils import get_mt4_process_name, get_platform_error_message, get_platform_name

logger = logging.getLogger(__name__)

def kill_process_by_name(process_name):
    """
    Kill all processes matching the given name (cross-platform)
    
    Args:
        process_name: Name of the process to kill (e.g., 'terminal.exe', 'terminal')
    
    Returns:
        int: Number of processes killed
    """
    killed_count = 0
    
    try:
        # Get all running processes
        for proc in psutil.process_iter(['pid', 'name']):
            try:
                # Check if process name matches (case-insensitive)
                proc_name = proc.info['name']
                if proc_name and process_name.lower() in proc_name.lower():
                    logger.info(f"Killing process: {proc_name} (PID: {proc.info['pid']})")
                    proc.kill()
                    killed_count += 1
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                # Process may have already terminated or we don't have permission
                pass
            except Exception as e:
                logger.warning(f"Error checking process {proc.info.get('name', 'unknown')}: {e}")
        
        if killed_count == 0:
            logger.warning(f"No processes found matching '{process_name}'")
        else:
            logger.info(f"Successfully killed {killed_count} process(es) matching '{process_name}'")
            
    except Exception as e:
        logger.error(f"Error killing processes: {e}")
    
    return killed_count

def kill_mt4_processes():
    """
    Kill MT4 terminal processes (cross-platform)
    Uses platform detection to determine correct process name
    """
    killed = 0
    platform_name = get_platform_name()
    
    # Get platform-specific MT4 process name
    primary_name = get_mt4_process_name()
    logger.info(f"Detected platform: {platform_name}, looking for MT4 process: {primary_name}")
    
    # Try common MT4 process names (platform-specific first, then fallbacks)
    if get_mt4_process_name() == 'terminal.exe':
        # Windows: try .exe first, then without extension
        process_names = ['terminal.exe', 'terminal', 'metaeditor.exe', 'metaeditor']
    else:
        # Unix: try without extension first
        process_names = ['terminal', 'metaeditor', 'terminal.exe', 'metaeditor.exe']
    
    for name in process_names:
        killed += kill_process_by_name(name)
    
    if killed == 0:
        logger.warning(f"No MT4 processes found on {platform_name}. Expected process name: {primary_name}")
    
    return killed

