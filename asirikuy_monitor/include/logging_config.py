"""
Enhanced logging configuration for Asirikuy Monitor
Provides log rotation, configurable levels, and structured logging
"""

import logging
import logging.handlers
from pathlib import Path


def setup_logging(log_file_path='./log/monitor.log', 
                  log_level=logging.INFO,
                  max_bytes=10*1024*1024,  # 10 MB
                  backup_count=5,
                  enable_console=False,
                  console_level=logging.WARNING):
    """
    Set up enhanced logging with rotation and multiple handlers
    
    Args:
        log_file_path: Path to log file
        log_level: Logging level for file handler (default: INFO)
        max_bytes: Maximum log file size before rotation (default: 10MB)
        backup_count: Number of backup log files to keep (default: 5)
        enable_console: Enable console logging (default: False)
        console_level: Logging level for console handler (default: WARNING)
    
    Returns:
        logging.Logger: Configured logger instance
    """
    # Create log directory if it doesn't exist
    log_file = Path(log_file_path)
    log_dir = log_file.parent
    if log_dir and not log_dir.exists():
        log_dir.mkdir(parents=True, exist_ok=True)
    
    # Get or create logger
    logger = logging.getLogger('Monitor')
    logger.setLevel(logging.DEBUG)  # Set to lowest level, handlers will filter
    
    # Remove existing handlers to avoid duplicates
    logger.handlers.clear()
    
    # Create formatters
    detailed_formatter = logging.Formatter(
        '%(asctime)s.%(msecs)03d [%(levelname)-8s] %(name)s: %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )
    
    simple_formatter = logging.Formatter(
        '%(asctime)s [%(levelname)s] %(message)s',
        datefmt='%H:%M:%S'
    )
    
    # File handler with rotation
    file_handler = logging.handlers.RotatingFileHandler(
        str(log_file),
        maxBytes=max_bytes,
        backupCount=backup_count,
        encoding='utf-8'
    )
    file_handler.setLevel(log_level)
    file_handler.setFormatter(detailed_formatter)
    logger.addHandler(file_handler)
    
    # Console handler (optional)
    if enable_console:
        console_handler = logging.StreamHandler()
        console_handler.setLevel(console_level)
        console_handler.setFormatter(simple_formatter)
        logger.addHandler(console_handler)
    
    return logger


def get_log_level_from_string(level_str):
    """
    Convert string to logging level
    
    Args:
        level_str: String representation of log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
    
    Returns:
        int: Logging level constant
    """
    level_map = {
        'DEBUG': logging.DEBUG,
        'INFO': logging.INFO,
        'WARNING': logging.WARNING,
        'ERROR': logging.ERROR,
        'CRITICAL': logging.CRITICAL,
    }
    
    level_str = level_str.upper().strip()
    return level_map.get(level_str, logging.INFO)


def configure_logging_from_config(config, log_file_path='./log/monitor.log'):
    """
    Configure logging from config file
    
    Args:
        config: ConfigParser instance
        log_file_path: Default log file path
    
    Returns:
        logging.Logger: Configured logger instance
    """
    # Get log settings from config (with defaults)
    try:
        log_level_str = config.get('general', 'logLevel', fallback='INFO')
        log_level = get_log_level_from_string(log_level_str)
    except Exception:
        log_level = logging.INFO
    
    try:
        max_bytes = config.getint('general', 'logMaxBytes', fallback=10*1024*1024)  # 10 MB default
    except Exception:
        max_bytes = 10*1024*1024
    
    try:
        backup_count = config.getint('general', 'logBackupCount', fallback=5)
    except Exception:
        backup_count = 5
    
    try:
        enable_console = config.getboolean('general', 'logConsole', fallback=False)
    except Exception:
        enable_console = False
    
    try:
        console_level_str = config.get('general', 'logConsoleLevel', fallback='WARNING')
        console_level = get_log_level_from_string(console_level_str)
    except Exception:
        console_level = logging.WARNING
    
    # Override log file path if specified in config
    try:
        log_file_path = config.get('general', 'logFile', fallback=log_file_path)
    except Exception:
        pass
    
    return setup_logging(
        log_file_path=log_file_path,
        log_level=log_level,
        max_bytes=max_bytes,
        backup_count=backup_count,
        enable_console=enable_console,
        console_level=console_level
    )


def log_structured(logger, level, event_type, **kwargs):
    """
    Log structured data (key-value pairs)
    
    Args:
        logger: Logger instance
        level: Log level (logging.INFO, logging.ERROR, etc.)
        event_type: Type of event (e.g., 'heartbeat_check', 'error_detected')
        **kwargs: Additional key-value pairs to log
    """
    # Format as: EVENT_TYPE key1=value1 key2=value2 ...
    parts = [f"EVENT={event_type}"]
    for key, value in kwargs.items():
        # Sanitize value for logging (convert to string, limit length)
        str_value = str(value)
        if len(str_value) > 200:
            str_value = str_value[:200] + "..."
        parts.append(f"{key}={str_value}")
    
    message = " ".join(parts)
    logger.log(level, message)

