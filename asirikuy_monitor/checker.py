"""
Asirikuy Monitor - Main monitoring script
Monitors trading platform heartbeats and log files for errors
"""

from colorama import init, Fore, Back, Style
import logging
import argparse
import ctypes
import sys
import os
import logging.handlers
from time import strftime
from pathlib import Path
from typing import Optional, List
from include.asirikuy import *
from include.misc import *
from include.process_manager import kill_mt4_processes
from include.retry_mechanism import retry_file_operation, retry_email_send, graceful_degradation
from include.config_manager import load_config_with_defaults, validate_config
from include.platform_utils import (
    detect_platform, get_platform_name, get_platform_info,
    get_platform_error_message, is_windows, is_macos, is_linux
)
from include.version import get_version_string, get_full_version_info

# Telegram notification support
try:
    from include.telegram_notifier import send_telegram_alert
    TELEGRAM_AVAILABLE = True
except ImportError:
    TELEGRAM_AVAILABLE = False
    send_telegram_alert = None  # Placeholder to avoid NameError
import datetime as dt
import time
import calendar
import subprocess

# Load environment variables from .env file if it exists
try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    # python-dotenv not installed, continue without it
    pass

# Constants
DEFAULT_CONFIG_PATH = Path('./config/checker.config')
DEFAULT_LOG_FILE = Path('./log/monitor.log')


def version() -> None:
    """
    Print version information and exit
    """
    platform_info = get_platform_info()
    version_info = get_full_version_info()
    print(get_version_string())
    print(f"Version: {version_info['version']} ({version_info['version_date']})")
    print(f"Platform: {get_platform_name()} ({platform_info['system']} {platform_info['release']})")


def main() -> None:
    """
    Main entry point for Asirikuy Monitor
    
    Parses command-line arguments, loads configuration, and starts monitoring loop
    """

    parser = argparse.ArgumentParser(
        description='Asirikuy Monitor - Trading platform monitoring tool'
    )
    parser.add_argument(
        '-c', '--config-file',
        type=str,
        help='Path to configuration file (default: ./config/checker.config)'
    )
    parser.add_argument(
        '-v', '--version',
        action='store_true',
        help='Show version information and exit'
    )
    args = parser.parse_args()

    if args.version:
        version()
        sys.exit(0)

    # Colorama Initialization
    init(autoreset=True)
    
    # Determine config file path
    if args.config_file:
        config_file_path = Path(args.config_file)
    else:
        config_file_path = DEFAULT_CONFIG_PATH

    # Load configuration with validation
    try:
        config = load_config_with_defaults(str(config_file_path))
    except FileNotFoundError as e:
        print(f"Error: Configuration file not found: {e}")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: Invalid configuration:\n{e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading config file: {e}")
        sys.exit(1)    

    # Set up enhanced logging first (before using logger)
    try:
        from include.logging_config import configure_logging_from_config
        logger = configure_logging_from_config(config)
        logger.info("Monitor starting with enhanced logging")
    except ImportError:
        # Fallback to basic logging if enhanced logging not available
        logging.basicConfig(
            filename=DEFAULT_LOG_FILE,
            filemode='a',
            format='%(asctime)s,%(msecs)d %(name)s %(levelname)s %(message)s',
            datefmt='%H:%M:%S',
            level=logging.ERROR
        )
        logger = logging.getLogger('Monitor')
        logger.info("Monitor starting with basic logging")

    monitoringInterval = float(config.get('general', 'monitoringInterval'))
    openDay =   int(config.get('general', 'weekOpenDay'))
    openHour =  int(config.get('general', 'weekOpenHour'))
    closeDay =  int(config.get('general', 'weekCloseDay'))
    closeHour = int(config.get('general', 'weekCloseHour'))
    useEmail = int(config.get('general', 'useEmail', fallback='0'))
    
    # Get email credentials from environment variables first, fall back to config file
    fromEmail = os.getenv('EMAIL_FROM', config.get('general', 'fromEmail', fallback=''))
    toEmail = os.getenv('EMAIL_TO', config.get('general', 'toEmail', fallback=''))
    emailLogin = os.getenv('EMAIL_LOGIN', config.get('general', 'emailLogin', fallback=''))
    emailPassword = os.getenv('EMAIL_PASSWORD', config.get('general', 'emailPassword', fallback=''))
    smtpServer = os.getenv('SMTP_SERVER', config.get('general', 'smtpServer', fallback=''))
    
    # Warn if using config file credentials (less secure)
    if not os.getenv('EMAIL_PASSWORD') and useEmail == 1:
        logger.warning("Using email password from config file. Consider using EMAIL_PASSWORD environment variable for better security.")
    
    # Get Telegram notification settings
    useTelegram = int(config.get('general', 'useTelegram', fallback='0'))
    telegramBotToken = os.getenv('TELEGRAM_BOT_TOKEN', config.get('general', 'telegramBotToken', fallback=''))
    telegramChatId = os.getenv('TELEGRAM_CHAT_ID', config.get('general', 'telegramChatId', fallback=''))
    
    # Validate Telegram configuration if enabled
    if useTelegram == 1 and TELEGRAM_AVAILABLE:
        if not telegramBotToken or not telegramChatId:
            logger.warning("Telegram notifications enabled but bot token or chat ID not configured. Disabling Telegram notifications.")
            useTelegram = 0
        else:
            logger.info("Telegram notifications enabled")
    elif useTelegram == 1 and not TELEGRAM_AVAILABLE:
        logger.warning("Telegram notifications enabled but Telegram module not available. Install requests library: pip install requests")
        useTelegram = 0
    
    # Get account sections
    account_sections: List[str] = [
        s.strip() for s in config.get('accounts', 'accounts').split(',')
    ]


    # Initialize error tracking
    last_error: List[str] = []
    for _ in account_sections:
        last_error.append("")

    while True:
        
        clearScreen()

        local_time = dt.datetime.now()
        hour = time.localtime().tm_hour
        day = time.localtime().tm_wday

        print(f"Local Hour = {hour}")
        print(f"Local Day = {day}")

        
        
        # Check if within trading hours
        is_trading_hours = (
            ((hour > openHour) and (day == openDay)) or
            ((hour < closeHour) and (day == closeDay)) or
            ((day > openDay) and (day < closeDay))
        )
        
        if is_trading_hours:
            account_index = 0
        
            for account_section in account_sections:
                try:
                    path = config.get(account_section, 'path')
                    frontend = config.get(account_section, 'frontend')
                    account_number = config.get(account_section, 'accountNumber')
                except Exception as e:
                    error_msg = get_platform_error_message(
                        f"reading config for account {account_section}",
                        e
                    )
                    logger.error(error_msg)
                    continue

                #check the heartbeat
                original_dir = os.getcwd()
                
                try:
                    # Use pathlib for cross-platform path handling
                    path_obj = Path(path)
                    if frontend == "MT4":
                        heartbeat_dir = path_obj / 'MQL4' / 'Files'
                    else:
                        heartbeat_dir = path_obj / 'tmp'
                    
                    if not heartbeat_dir.exists():
                        logger.warning(f"Heartbeat directory does not exist: {heartbeat_dir}")
                        continue
                    
                    os.chdir(str(heartbeat_dir))
                except Exception as e:
                    error_msg = get_platform_error_message(
                        f"changing to heartbeat directory for {account_section}",
                        e
                    )
                    logger.error(error_msg)
                    continue

                try:
                    # Use pathlib for listing files
                    for file_path in heartbeat_dir.iterdir():
                        if file_path.suffix != '.hb':
                            continue
                        
                        files = file_path.name
                        try:
                            # Use retry mechanism for file operations
                            def read_heartbeat_file():
                                with open(str(file_path), 'r', encoding='utf-8') as fp:
                                    return fp.readlines()
                            
                            data = retry_file_operation(
                                read_heartbeat_file,
                                max_attempts=3,
                                operation_name=f"Reading heartbeat file {files}"
                            )
                            
                            if len(data) < 2:
                                logger.warning(f"Heartbeat file {files} has insufficient data")
                                continue
                            
                            date = dt.datetime.strptime(data[1].strip(),"%d/%m/%y %H:%M")
                            output = f"Checking {files}..."
                            print (output)
                            logger.info(output)
                            
                            utc_datetime = dt.datetime.utcnow()                        
                            diff = abs(utc_datetime - date).total_seconds()
                            output = f'Current Time: {local_time}, Current UTC Time {utc_datetime}, Last heart-beat time: {date}, Difference in {monitoringInterval*2.5} secs: {diff}'
                            print(output)
                            logger.info(output)
                            
                            if diff > monitoringInterval*2.5:
                                output = f"Heart-Beat problem for instance {files}"
                                print (Style.BRIGHT + Fore.RED + output)
                                logger.error(output)
                                
                                try:
                                    os.unlink(files)
                                except Exception as e:
                                    logger.error(f"Error deleting heartbeat file {files}: {e}")
                                
                                if frontend == "MT4":
                                    logger.error("Killing MT4")
                                    try:
                                        killed = kill_mt4_processes()
                                        if killed == 0:
                                            logger.warning("No MT4 processes found to kill")
                                    except Exception as e:
                                        logger.error(f"Error killing MT4 processes: {e}")
                                
                                # Send notifications (email and/or Telegram)
                                alert_message = f"Heart-Beat problem for instance {files}, system not updating for more than {monitoringInterval*2.5} secs"
                                
                                if useEmail == 1:
                                    output = "Sending email message about heart beat..."
                                    print(output)
                                    logger.error(output)  
                                    try:
                                        retry_email_send(
                                            sendemail,
                                            fromEmail, toEmail, "", 
                                            f"{account_section} error message", 
                                            alert_message, 
                                            emailLogin, emailPassword, smtpServer,
                                            max_attempts=3
                                        )
                                    except Exception as e:
                                        logger.error(f"Error sending email for heartbeat problem after retries: {e}")
                                
                                if useTelegram == 1 and TELEGRAM_AVAILABLE and send_telegram_alert:
                                    output = "Sending Telegram notification about heart beat..."
                                    print(output)
                                    logger.error(output)
                                    try:
                                        send_telegram_alert(
                                            bot_token=telegramBotToken,
                                            chat_id=telegramChatId,
                                            title=f"{account_section} - Heartbeat Alert",
                                            message=f"⚠️ {alert_message}",
                                            alert_type="CRITICAL"
                                        )
                                    except Exception as e:
                                        logger.error(f"Error sending Telegram notification for heartbeat problem: {e}")
                                break
                        except ValueError as e:
                            logger.error(f"Error parsing heartbeat file {files}: {e}")
                            continue
                        except Exception as e:
                            logger.error(f"Error processing heartbeat file {files}: {e}")
                            continue
                except Exception as e:
                    logger.error(f"Error listing heartbeat files for {account_section}: {e}")
                finally:
                    try:
                        os.chdir(original_dir)
                    except Exception:
                        pass                               
                #log checking step

                try:
                    # Use pathlib for cross-platform path handling
                    path_obj = Path(path)
                    if frontend == "MT4":
                        fname = path_obj / 'MQL4' / 'Logs' / 'AsirikuyFramework.log'
                    else:
                        fname = path_obj / 'log' / f'{account_number}AsirikuyFramework.log'
                    
                    logger.info(f"Checking log file: {fname}")
                    
                    if not fname.exists():
                        logger.warning(f"Log file does not exist: {fname}")
                        continue
                    
                    try:
                        # Use retry mechanism for file operations
                        def read_log_file():
                            with open(str(fname), 'r', encoding='utf-8', errors='replace') as file:
                                return file.readlines()
                        
                        data = retry_file_operation(
                            read_log_file,
                            max_attempts=3,
                            operation_name=f"Reading log file {fname}"
                        )
                    except Exception as e:
                        logger.error(f"Error reading log file {fname} after retries: {e}")
                        continue

                    if len(data) == 0:
                        logger.debug(f"Log file {fname} is empty")
                        continue

                    #FMT = '%H:%M:%S.%f'
                    #lastLine = data[len(data)-1]
                    #lastTime = dt.datetime.strptime(lastLine.split(' ')[3], FMT);
                    
                    # look back the last 6 lines
                    # if they are within 10 seconds, will check error
                    #for index in range(len(data)-10,len(data)):
                        
                        #line = data[index]                        
                        #tdelta = lastTime - dt.datetime.strptime(line.split(' ')[3], FMT)
                        #print tdelta.total_seconds()                   
                        #print line

                    line = data[len(data) -1]
                    logger.info(f"Checking last line from {fname}: {line[:100]}...")  # Log first 100 chars
                    #print line
                    #if tdelta.total_seconds() < 5 and ( "Error" in line  or "Emergency" in line or "Critical" in line ):
                    if "Error" in line or "Emergency" in line or "Critical" in line:
                        output = f"Error detected on account {account_section}"
                        print(Style.BRIGHT + Fore.RED + output)
                        logger.error(output)
                        print(line)
                        logger.error(line)
                        
                        if frontend == "MT4" and "Error" in line:
                            logger.error("Killing MT4")
                            try:
                                killed = kill_mt4_processes()
                                if killed == 0:
                                    logger.warning("No MT4 processes found to kill")
                            except Exception as e:
                                logger.error(f"Error killing MT4 processes: {e}")
                        
                        if (useEmail == 1 or useTelegram == 1) and last_error[account_index] != data[len(data)-1]:
                            error_message = data[len(data)-1]
                            last_error[account_index] = error_message
                            
                            if useEmail == 1:
                                output = "Sending email message about error..."
                                logger.error(output)
                                try:
                                    retry_email_send(
                                        sendemail,
                                        fromEmail, toEmail, "", 
                                        f"{account_section} error message", 
                                        error_message, 
                                        emailLogin, emailPassword, smtpServer,
                                        max_attempts=3
                                    )
                                except Exception as e:
                                    logger.error(f"Error sending email for error detection after retries: {e}")
                            
                            if useTelegram == 1 and TELEGRAM_AVAILABLE and send_telegram_alert:
                                output = "Sending Telegram notification about error..."
                                logger.error(output)
                                try:
                                    send_telegram_alert(
                                        bot_token=telegramBotToken,
                                        chat_id=telegramChatId,
                                        title=f"{account_section} - Error Detected",
                                        message=f"❌ {error_message}",
                                        alert_type="ERROR"
                                    )
                                except Exception as e:
                                    logger.error(f"Error sending Telegram notification for error detection: {e}")
                        break
                except Exception as e:
                    logger.error(f"Error processing log file for {account_section}: {e}")
                    continue

                account_index += 1
                
        output = f'Last execution {strftime("%a, %d %b %Y %X")}, sleeping for {monitoringInterval} secs.'
        print((Style.BRIGHT + Back.GREEN + output).ljust(100))
        logger.info(output)
        time.sleep(monitoringInterval)


if __name__ == "__main__":
    main()

    
        
if __name__ == "__main__": main()
