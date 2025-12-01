"""
Retry mechanism for error recovery
Provides retry logic with exponential backoff for file operations and email sending
"""

import time
import logging
from functools import wraps
from typing import Callable, Any, Optional, Tuple

logger = logging.getLogger(__name__)


class RetryConfig:
    """Configuration for retry behavior"""
    def __init__(self, max_attempts=3, initial_delay=1.0, max_delay=60.0, 
                 exponential_base=2.0, retryable_exceptions=None):
        """
        Initialize retry configuration
        
        Args:
            max_attempts: Maximum number of retry attempts (default: 3)
            initial_delay: Initial delay in seconds before first retry (default: 1.0)
            max_delay: Maximum delay between retries in seconds (default: 60.0)
            exponential_base: Base for exponential backoff (default: 2.0)
            retryable_exceptions: Tuple of exception types to retry (None = all)
        """
        self.max_attempts = max_attempts
        self.initial_delay = initial_delay
        self.max_delay = max_delay
        self.exponential_base = exponential_base
        self.retryable_exceptions = retryable_exceptions or (Exception,)


def retry_with_backoff(config: Optional[RetryConfig] = None):
    """
    Decorator for retrying functions with exponential backoff
    
    Args:
        config: RetryConfig instance (uses default if None)
    
    Usage:
        @retry_with_backoff(RetryConfig(max_attempts=3))
        def my_function():
            # code that might fail
            pass
    """
    if config is None:
        config = RetryConfig()
    
    def decorator(func: Callable) -> Callable:
        @wraps(func)
        def wrapper(*args, **kwargs) -> Any:
            last_exception = None
            
            for attempt in range(1, config.max_attempts + 1):
                try:
                    return func(*args, **kwargs)
                except config.retryable_exceptions as e:
                    last_exception = e
                    
                    if attempt < config.max_attempts:
                        # Calculate delay with exponential backoff
                        delay = min(
                            config.initial_delay * (config.exponential_base ** (attempt - 1)),
                            config.max_delay
                        )
                        
                        logger.warning(
                            f"{func.__name__} failed (attempt {attempt}/{config.max_attempts}): {e}. "
                            f"Retrying in {delay:.2f} seconds..."
                        )
                        time.sleep(delay)
                    else:
                        logger.error(
                            f"{func.__name__} failed after {config.max_attempts} attempts: {e}"
                        )
            
            # All attempts failed, raise the last exception
            raise last_exception
        
        return wrapper
    return decorator


def retry_file_operation(operation: Callable, *args, max_attempts: int = 3, 
                         operation_name: str = "file operation", **kwargs) -> Any:
    """
    Retry a file operation with exponential backoff
    
    Args:
        operation: Function to retry
        *args: Arguments to pass to operation
        max_attempts: Maximum number of retry attempts
        operation_name: Name of operation for logging
        **kwargs: Keyword arguments to pass to operation
    
    Returns:
        Result of the operation
    
    Raises:
        Last exception if all attempts fail
    """
    config = RetryConfig(
        max_attempts=max_attempts,
        initial_delay=0.5,  # Shorter delay for file operations
        max_delay=5.0,
        retryable_exceptions=(IOError, OSError, PermissionError)
    )
    
    last_exception = None
    
    for attempt in range(1, config.max_attempts + 1):
        try:
            return operation(*args, **kwargs)
        except config.retryable_exceptions as e:
            last_exception = e
            
            if attempt < config.max_attempts:
                delay = min(
                    config.initial_delay * (config.exponential_base ** (attempt - 1)),
                    config.max_delay
                )
                
                logger.warning(
                    f"{operation_name} failed (attempt {attempt}/{max_attempts}): {e}. "
                    f"Retrying in {delay:.2f} seconds..."
                )
                time.sleep(delay)
            else:
                logger.error(
                    f"{operation_name} failed after {max_attempts} attempts: {e}"
                )
    
    raise last_exception


def retry_email_send(email_func: Callable, *args, max_attempts: int = 3, **kwargs) -> Any:
    """
    Retry email sending with exponential backoff
    
    Args:
        email_func: Email sending function
        *args: Arguments to pass to email function
        max_attempts: Maximum number of retry attempts
        **kwargs: Keyword arguments to pass to email function
    
    Returns:
        Result of the email function
    
    Raises:
        Last exception if all attempts fail
    """
    import smtplib
    
    config = RetryConfig(
        max_attempts=max_attempts,
        initial_delay=2.0,  # Longer delay for network operations
        max_delay=30.0,
        retryable_exceptions=(
            smtplib.SMTPException,
            smtplib.SMTPAuthenticationError,
            ConnectionError,
            TimeoutError,
            OSError  # Network errors
        )
    )
    
    last_exception = None
    
    for attempt in range(1, config.max_attempts + 1):
        try:
            return email_func(*args, **kwargs)
        except config.retryable_exceptions as e:
            last_exception = e
            
            if attempt < config.max_attempts:
                delay = min(
                    config.initial_delay * (config.exponential_base ** (attempt - 1)),
                    config.max_delay
                )
                
                logger.warning(
                    f"Email send failed (attempt {attempt}/{max_attempts}): {e}. "
                    f"Retrying in {delay:.2f} seconds..."
                )
                time.sleep(delay)
            else:
                logger.error(
                    f"Email send failed after {max_attempts} attempts: {e}"
                )
    
    raise last_exception


def graceful_degradation(func: Callable, fallback: Callable = None, 
                        fallback_value: Any = None, log_error: bool = True) -> Any:
    """
    Execute function with graceful degradation on failure
    
    Args:
        func: Function to execute
        fallback: Fallback function to call on failure
        fallback_value: Value to return on failure if no fallback function
        log_error: Whether to log errors
    
    Returns:
        Result of func, fallback, or fallback_value
    """
    try:
        return func()
    except Exception as e:
        if log_error:
            logger.warning(f"Operation failed, using graceful degradation: {e}")
        
        if fallback:
            try:
                return fallback()
            except Exception as fallback_error:
                logger.error(f"Fallback also failed: {fallback_error}")
                if fallback_value is not None:
                    return fallback_value
                raise
        
        if fallback_value is not None:
            return fallback_value
        
        # If no fallback, re-raise the exception
        raise


class HealthCheck:
    """Health check status tracking"""
    def __init__(self):
        self.checks = {}
        self.last_check_time = {}
    
    def register_check(self, name: str, check_func: Callable[[], bool], 
                      timeout: float = 5.0):
        """
        Register a health check
        
        Args:
            name: Name of the health check
            check_func: Function that returns True if healthy, False otherwise
            timeout: Maximum time to wait for check (seconds)
        """
        self.checks[name] = {
            'func': check_func,
            'timeout': timeout
        }
    
    def run_check(self, name: str) -> Tuple[bool, Optional[str]]:
        """
        Run a specific health check
        
        Args:
            name: Name of the health check
        
        Returns:
            Tuple of (is_healthy, error_message)
        """
        if name not in self.checks:
            return False, f"Health check '{name}' not registered"
        
        check = self.checks[name]
        start_time = time.time()
        
        try:
            result = check['func']()
            elapsed = time.time() - start_time
            
            if elapsed > check['timeout']:
                return False, f"Health check timed out after {elapsed:.2f}s"
            
            self.last_check_time[name] = time.time()
            return bool(result), None
        except Exception as e:
            elapsed = time.time() - start_time
            return False, f"Health check failed: {e} (after {elapsed:.2f}s)"
    
    def run_all_checks(self) -> dict:
        """
        Run all registered health checks
        
        Returns:
            Dictionary of check results
        """
        results = {}
        for name in self.checks:
            is_healthy, error = self.run_check(name)
            results[name] = {
                'healthy': is_healthy,
                'error': error,
                'last_check': self.last_check_time.get(name)
            }
        return results
    
    def get_status(self) -> dict:
        """
        Get overall health status
        
        Returns:
            Dictionary with overall status and individual check results
        """
        results = self.run_all_checks()
        all_healthy = all(r['healthy'] for r in results.values())
        
        return {
            'overall': 'healthy' if all_healthy else 'unhealthy',
            'checks': results,
            'timestamp': time.time()
        }

