"""
Platform detection and utilities
Provides cross-platform OS detection and platform-specific helpers
"""

import os
import sys
import platform
from typing import Literal, Optional
from enum import Enum

# Platform type enum
class PlatformType(Enum):
    """Supported platform types"""
    WINDOWS = "windows"
    MACOS = "macos"
    LINUX = "linux"
    UNKNOWN = "unknown"

def detect_platform() -> PlatformType:
    """
    Detect the current operating system platform
    
    Returns:
        PlatformType: The detected platform (WINDOWS, MACOS, LINUX, or UNKNOWN)
    """
    system = platform.system().lower()
    
    if system == 'windows':
        return PlatformType.WINDOWS
    elif system == 'darwin':
        return PlatformType.MACOS
    elif system == 'linux':
        return PlatformType.LINUX
    else:
        return PlatformType.UNKNOWN

def get_platform_name() -> str:
    """
    Get human-readable platform name
    
    Returns:
        str: Platform name (Windows, macOS, Linux, or Unknown)
    """
    platform_type = detect_platform()
    return platform_type.value.capitalize()

def is_windows() -> bool:
    """Check if running on Windows"""
    return detect_platform() == PlatformType.WINDOWS

def is_macos() -> bool:
    """Check if running on macOS"""
    return detect_platform() == PlatformType.MACOS

def is_linux() -> bool:
    """Check if running on Linux"""
    return detect_platform() == PlatformType.LINUX

def is_unix() -> bool:
    """Check if running on Unix-like system (macOS or Linux)"""
    return is_macos() or is_linux()

def get_mt4_process_name() -> str:
    """
    Get the MT4 process name for the current platform
    
    Returns:
        str: Process name ('terminal.exe' for Windows, 'terminal' for Unix)
    """
    if is_windows():
        return 'terminal.exe'
    else:
        return 'terminal'

def get_platform_info() -> dict:
    """
    Get detailed platform information
    
    Returns:
        dict: Platform information including:
            - platform: Platform type (windows/macos/linux/unknown)
            - system: System name from platform.system()
            - release: OS release version
            - version: OS version string
            - machine: Machine architecture
            - processor: Processor type
    """
    return {
        'platform': detect_platform().value,
        'system': platform.system(),
        'release': platform.release(),
        'version': platform.version(),
        'machine': platform.machine(),
        'processor': platform.processor(),
    }

def get_platform_error_message(operation: str, error: Exception) -> str:
    """
    Generate platform-specific error messages
    
    Args:
        operation: Description of the operation that failed
        error: The exception that occurred
    
    Returns:
        str: Platform-specific error message with helpful context
    """
    platform_type = detect_platform()
    base_message = f"Error during {operation}: {str(error)}"
    
    if platform_type == PlatformType.WINDOWS:
        suggestions = [
            "Check if you have administrator privileges",
            "Verify the file/path exists and is accessible",
            "Check Windows Event Viewer for system errors"
        ]
    elif platform_type == PlatformType.MACOS:
        suggestions = [
            "Check if you have necessary permissions",
            "Verify the file/path exists and is accessible",
            "Check Console.app for system errors"
        ]
    elif platform_type == PlatformType.LINUX:
        suggestions = [
            "Check if you have necessary permissions (sudo may be required)",
            "Verify the file/path exists and is accessible",
            "Check system logs: journalctl -xe or /var/log/syslog"
        ]
    else:
        suggestions = [
            "Check if you have necessary permissions",
            "Verify the file/path exists and is accessible"
        ]
    
    suggestion_text = "\n  - ".join(suggestions)
    return f"{base_message}\n\nPlatform-specific suggestions:\n  - {suggestion_text}"

def get_path_separator() -> str:
    """
    Get the path separator for the current platform
    
    Returns:
        str: Path separator ('\\' for Windows, '/' for Unix)
    """
    return os.sep

def get_line_separator() -> str:
    """
    Get the line separator for the current platform
    
    Returns:
        str: Line separator ('\\r\\n' for Windows, '\\n' for Unix)
    """
    return os.linesep

def format_platform_path(path: str) -> str:
    """
    Format a path for the current platform
    
    Args:
        path: Path string (may contain forward or backslashes)
    
    Returns:
        str: Path formatted for current platform
    """
    # Normalize path separators
    if is_windows():
        return path.replace('/', '\\')
    else:
        return path.replace('\\', '/')

