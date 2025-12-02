"""
Email security utilities
Provides email validation, sanitization, and rate limiting
"""

import re
import time
import logging
import html
from collections import defaultdict
from datetime import datetime, timedelta

logger = logging.getLogger(__name__)

# Email validation regex (RFC 5322 compliant, simplified)
EMAIL_REGEX = re.compile(
    r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
)

# Rate limiting: track emails sent per recipient
_email_rate_limiter = defaultdict(list)
_rate_limit_window = 300  # 5 minutes in seconds
_max_emails_per_window = 5  # Maximum emails per recipient per window


def validate_email_address(email):
    """
    Validate email address format
    
    Args:
        email: Email address string
        
    Returns:
        bool: True if valid, False otherwise
    """
    if not email or not isinstance(email, str):
        return False
    
    # Basic format check
    if not EMAIL_REGEX.match(email.strip()):
        return False
    
    # Additional checks
    email = email.strip()
    
    # Check length (RFC 5321: local part max 64, domain max 255, total max 254)
    if len(email) > 254:
        return False
    
    # Check for consecutive dots
    if '..' in email:
        return False
    
    # Check local and domain parts
    parts = email.split('@')
    if len(parts) != 2:
        return False
    
    local_part, domain_part = parts
    
    # Local part checks
    if len(local_part) > 64 or len(local_part) == 0:
        return False
    
    # Domain part checks
    if len(domain_part) > 255 or len(domain_part) == 0:
        return False
    
    # Domain must have at least one dot
    if '.' not in domain_part:
        return False
    
    # Domain cannot start or end with dot or hyphen
    if domain_part.startswith('.') or domain_part.endswith('.'):
        return False
    if domain_part.startswith('-') or domain_part.endswith('-'):
        return False
    
    return True


def sanitize_email_content(content):
    """
    Sanitize email content to prevent injection attacks
    
    Args:
        content: Email content string
        
    Returns:
        str: Sanitized content
    """
    if not content or not isinstance(content, str):
        return ""
    
    # Remove null bytes
    content = content.replace('\x00', '')
    
    # Escape HTML entities to prevent HTML injection
    content = html.escape(content)
    
    # Limit length to prevent DoS (max 10KB per email)
    max_length = 10240
    if len(content) > max_length:
        logger.warning(f"Email content truncated from {len(content)} to {max_length} characters")
        content = content[:max_length] + "... [truncated]"
    
    # Remove control characters except newlines and tabs
    content = ''.join(char for char in content if ord(char) >= 32 or char in '\n\t')
    
    return content


def sanitize_email_subject(subject):
    """
    Sanitize email subject line
    
    Args:
        subject: Email subject string
        
    Returns:
        str: Sanitized subject
    """
    if not subject or not isinstance(subject, str):
        return "Asirikuy Monitor Alert"
    
    # Remove null bytes
    subject = subject.replace('\x00', '')
    
    # Limit length (RFC 5322: max 78 characters recommended)
    max_length = 78
    if len(subject) > max_length:
        subject = subject[:max_length] + "..."
    
    # Remove control characters
    subject = ''.join(char for char in subject if ord(char) >= 32 or char in '\n\t')
    
    # Remove newlines from subject
    subject = subject.replace('\n', ' ').replace('\r', ' ')
    
    return subject.strip()


def check_rate_limit(recipient_email):
    """
    Check if email can be sent to recipient based on rate limiting
    
    Args:
        recipient_email: Recipient email address
        
    Returns:
        tuple: (bool, str) - (allowed, reason)
    """
    if not recipient_email:
        return False, "Invalid recipient email"
    
    current_time = time.time()
    recipient = recipient_email.lower().strip()
    
    # Clean old entries outside the time window
    _email_rate_limiter[recipient] = [
        timestamp for timestamp in _email_rate_limiter[recipient]
        if current_time - timestamp < _rate_limit_window
    ]
    
    # Check if rate limit exceeded
    if len(_email_rate_limiter[recipient]) >= _max_emails_per_window:
        oldest_timestamp = min(_email_rate_limiter[recipient])
        wait_time = _rate_limit_window - (current_time - oldest_timestamp)
        return False, f"Rate limit exceeded. Wait {int(wait_time)} seconds before sending to {recipient}"
    
    # Record this email attempt
    _email_rate_limiter[recipient].append(current_time)
    
    return True, "OK"


def reset_rate_limit(recipient_email=None):
    """
    Reset rate limit for a specific recipient or all recipients
    
    Args:
        recipient_email: Recipient email (None to reset all)
    """
    if recipient_email:
        recipient = recipient_email.lower().strip()
        if recipient in _email_rate_limiter:
            del _email_rate_limiter[recipient]
            logger.info(f"Rate limit reset for {recipient}")
    else:
        _email_rate_limiter.clear()
        logger.info("Rate limit reset for all recipients")


def get_rate_limit_status(recipient_email):
    """
    Get current rate limit status for a recipient
    
    Args:
        recipient_email: Recipient email address
        
    Returns:
        dict: Status information
    """
    if not recipient_email:
        return {"error": "Invalid recipient email"}
    
    recipient = recipient_email.lower().strip()
    current_time = time.time()
    
    # Clean old entries
    _email_rate_limiter[recipient] = [
        timestamp for timestamp in _email_rate_limiter[recipient]
        if current_time - timestamp < _rate_limit_window
    ]
    
    count = len(_email_rate_limiter[recipient])
    remaining = _max_emails_per_window - count
    
    return {
        "recipient": recipient,
        "emails_sent": count,
        "max_allowed": _max_emails_per_window,
        "remaining": remaining,
        "window_seconds": _rate_limit_window,
        "reset_in_seconds": _rate_limit_window - (current_time - min(_email_rate_limiter[recipient])) if count > 0 else 0
    }

