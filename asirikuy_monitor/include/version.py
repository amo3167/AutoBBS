"""
Version information for Asirikuy Monitor
Centralized version management
"""

# Version information
__version__ = "0.05"
__version_info__ = (0, 5, 0)
__version_date__ = "2024-12-02"

# Project information
__project_name__ = "Asirikuy Monitor"
__author__ = "Asirikuy Team"
__description__ = "A monitoring tool for trading platforms (MT4 and AT)"

def get_version() -> str:
    """
    Get the version string
    
    Returns:
        str: Version string (e.g., "0.05")
    """
    return __version__

def get_version_info() -> tuple:
    """
    Get the version as a tuple
    
    Returns:
        tuple: Version tuple (major, minor, patch)
    """
    return __version_info__

def get_version_string() -> str:
    """
    Get a formatted version string
    
    Returns:
        str: Formatted version string (e.g., "Asirikuy Monitor v0.05")
    """
    return f"{__project_name__} v{__version__}"

def get_full_version_info() -> dict:
    """
    Get full version information
    
    Returns:
        dict: Dictionary with version information
    """
    return {
        'version': __version__,
        'version_info': __version_info__,
        'version_date': __version_date__,
        'project_name': __project_name__,
        'author': __author__,
        'description': __description__
    }

