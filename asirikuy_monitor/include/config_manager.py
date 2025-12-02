"""
Configuration management for Asirikuy Monitor
Provides config validation, default values, and schema
"""

import os
import configparser
import logging
from pathlib import Path
from typing import Optional, Dict, Any, List

logger = logging.getLogger(__name__)


# Default configuration values
DEFAULT_CONFIG = {
    'general': {
        'monitoringInterval': '60',
        'useEmail': '0',
        'fromEmail': '',
        'toEmail': '',
        'emailLogin': '',
        'emailPassword': '',
        'smtpServer': 'smtp.gmail.com:587',
        'weekOpenDay': '0',
        'weekOpenHour': '6',
        'weekCloseDay': '5',
        'weekCloseHour': '7',
        'logLevel': 'INFO',
        'logFile': './log/monitor.log',
        'logMaxBytes': '10485760',  # 10 MB
        'logBackupCount': '5',
        'logConsole': 'false',
        'logConsoleLevel': 'WARNING',
    }
}


def validate_config(config: configparser.RawConfigParser) -> List[str]:
    """
    Validate configuration file
    
    Args:
        config: ConfigParser instance
    
    Returns:
        List of validation error messages (empty if valid)
    """
    errors = []
    
    # Check required sections
    if not config.has_section('general'):
        errors.append("Missing required section: [general]")
    
    if not config.has_section('accounts'):
        errors.append("Missing required section: [accounts]")
    
    # Validate general section
    if config.has_section('general'):
        required_keys = ['monitoringInterval', 'useEmail', 'weekOpenDay', 
                        'weekOpenHour', 'weekCloseDay', 'weekCloseHour']
        for key in required_keys:
            if not config.has_option('general', key):
                errors.append(f"Missing required option: general.{key}")
        
        # Validate monitoringInterval
        if config.has_option('general', 'monitoringInterval'):
            try:
                interval = float(config.get('general', 'monitoringInterval'))
                if interval <= 0:
                    errors.append("monitoringInterval must be greater than 0")
            except ValueError:
                errors.append("monitoringInterval must be a number")
        
        # Validate email settings if useEmail is enabled
        if config.has_option('general', 'useEmail'):
            try:
                use_email = int(config.get('general', 'useEmail'))
                if use_email == 1:
                    email_keys = ['fromEmail', 'toEmail', 'emailLogin', 
                                 'emailPassword', 'smtpServer']
                    for key in email_keys:
                        if not config.has_option('general', key):
                            errors.append(f"Missing required email option: general.{key}")
            except ValueError:
                errors.append("useEmail must be 0 or 1")
        
        # Validate day/hour ranges
        if config.has_option('general', 'weekOpenDay'):
            try:
                day = int(config.get('general', 'weekOpenDay'))
                if day < 0 or day > 6:
                    errors.append("weekOpenDay must be between 0 and 6")
            except ValueError:
                errors.append("weekOpenDay must be a number")
        
        if config.has_option('general', 'weekOpenHour'):
            try:
                hour = int(config.get('general', 'weekOpenHour'))
                if hour < 0 or hour > 23:
                    errors.append("weekOpenHour must be between 0 and 23")
            except ValueError:
                errors.append("weekOpenHour must be a number")
    
    # Validate accounts section
    if config.has_section('accounts'):
        if not config.has_option('accounts', 'accounts'):
            errors.append("Missing required option: accounts.accounts")
        else:
            account_list = config.get('accounts', 'accounts')
            if not account_list or not account_list.strip():
                errors.append("accounts.accounts cannot be empty")
            else:
                # Validate each account section
                account_names = [s.strip() for s in account_list.split(',')]
                for account_name in account_names:
                    if not config.has_section(account_name):
                        errors.append(f"Missing account section: [{account_name}]")
                    else:
                        # Validate account section
                        required_account_keys = ['accountNumber', 'path', 'frontend']
                        for key in required_account_keys:
                            if not config.has_option(account_name, key):
                                errors.append(f"Missing required option: {account_name}.{key}")
                        
                        # Validate frontend
                        if config.has_option(account_name, 'frontend'):
                            frontend = config.get(account_name, 'frontend')
                            if frontend not in ['MT4', 'AT']:
                                errors.append(f"Invalid frontend for {account_name}: {frontend} (must be MT4 or AT)")
    
    return errors


def load_config_with_defaults(config_path: str) -> configparser.RawConfigParser:
    """
    Load configuration file with default values applied
    
    Args:
        config_path: Path to configuration file (str or Path)
    
    Returns:
        ConfigParser instance with defaults applied
    
    Raises:
        FileNotFoundError: If config file doesn't exist
        ValueError: If config is invalid
    """
    config_file = Path(config_path)
    if not config_file.exists():
        raise FileNotFoundError(f"Configuration file not found: {config_path}")
    
    config = configparser.RawConfigParser()
    config.read(str(config_file))
    
    # Apply defaults for missing values
    for section, options in DEFAULT_CONFIG.items():
        if not config.has_section(section):
            config.add_section(section)
        
        for key, default_value in options.items():
            if not config.has_option(section, key):
                config.set(section, key, default_value)
                logger.debug(f"Applied default value: {section}.{key} = {default_value}")
    
    # Validate configuration
    errors = validate_config(config)
    if errors:
        error_msg = "Configuration validation failed:\n" + "\n".join(f"  - {e}" for e in errors)
        raise ValueError(error_msg)
    
    return config


def get_config_value(config: configparser.RawConfigParser, section: str, 
                     key: str, default: Optional[str] = None) -> str:
    """
    Get configuration value with optional default
    
    Args:
        config: ConfigParser instance
        section: Section name
        key: Option key
        default: Default value if not found
    
    Returns:
        Configuration value or default
    """
    if config.has_option(section, key):
        return config.get(section, key)
    elif default is not None:
        return default
    else:
        raise ValueError(f"Configuration option {section}.{key} not found and no default provided")

