import smtplib
import logging

# Import email security utilities
try:
    from include.email_security import (
        validate_email_address,
        sanitize_email_content,
        sanitize_email_subject,
        check_rate_limit
    )
    EMAIL_SECURITY_AVAILABLE = True
except ImportError:
    EMAIL_SECURITY_AVAILABLE = False
    import warnings
    warnings.warn("Email security module not available, using basic email function")

logger = logging.getLogger(__name__)

def sendemail(from_addr, to_addr_list, cc_addr_list,
              subject, message,
              login, password,
              smtpserver):
    """
    Send email with security enhancements
    
    Args:
        from_addr: Sender email address
        to_addr_list: List of recipient email addresses (or single string)
        cc_addr_list: List of CC email addresses (or single string)
        subject: Email subject
        message: Email message body
        login: SMTP login username
        password: SMTP password
        smtpserver: SMTP server address (host:port)
    """
    # Normalize input types
    if isinstance(to_addr_list, str):
        to_addr_list = [to_addr_list]
    if isinstance(cc_addr_list, str):
        cc_addr_list = [cc_addr_list] if cc_addr_list else []
    
    # Validate and sanitize email addresses
    if EMAIL_SECURITY_AVAILABLE:
        # Validate sender
        if not validate_email_address(from_addr):
            logger.error(f"Invalid sender email address: {from_addr}")
            raise ValueError(f"Invalid sender email address: {from_addr}")
        
        # Validate recipients
        valid_to_list = []
        for addr in to_addr_list:
            if validate_email_address(addr):
                valid_to_list.append(addr)
            else:
                logger.warning(f"Invalid recipient email address: {addr}")
        
        if not valid_to_list:
            logger.error("No valid recipient email addresses")
            raise ValueError("No valid recipient email addresses")
        
        # Validate CC recipients
        valid_cc_list = []
        for addr in cc_addr_list:
            if validate_email_address(addr):
                valid_cc_list.append(addr)
            else:
                logger.warning(f"Invalid CC email address: {addr}")
        
        # Check rate limiting for each recipient
        for recipient in valid_to_list:
            allowed, reason = check_rate_limit(recipient)
            if not allowed:
                logger.warning(f"Email rate limit exceeded for {recipient}: {reason}")
                # Continue with other recipients, but log the issue
        
        # Sanitize content
        subject = sanitize_email_subject(subject)
        message = sanitize_email_content(message)
    else:
        # Basic validation without security module
        if not from_addr or '@' not in from_addr:
            raise ValueError("Invalid sender email address")
        if not to_addr_list or not any('@' in addr for addr in to_addr_list):
            raise ValueError("No valid recipient email addresses")
    
    # Parse SMTP server
    try:
        if ':' in smtpserver:
            smtp_host, smtp_port = smtpserver.split(':')
            smtp_port = int(smtp_port)
        else:
            smtp_host = smtpserver
            smtp_port = 587  # Default TLS port
    except Exception as e:
        logger.error(f"Invalid SMTP server format: {smtpserver}, error: {e}")
        raise ValueError(f"Invalid SMTP server format: {smtpserver}")
    
    # Build email headers
    header = f'From: {from_addr}\n'
    header += f'To: {",".join(valid_to_list if EMAIL_SECURITY_AVAILABLE else to_addr_list)}\n'
    if valid_cc_list if EMAIL_SECURITY_AVAILABLE else cc_addr_list:
        header += f'Cc: {",".join(valid_cc_list if EMAIL_SECURITY_AVAILABLE else cc_addr_list)}\n'
    header += f'Subject: {subject}\n\n'
    email_body = header + message
    
    # Send email
    try:
        server = smtplib.SMTP(smtp_host, smtp_port, timeout=30)
        server.starttls()
        server.login(login, password)
        problems = server.sendmail(
            from_addr,
            valid_to_list if EMAIL_SECURITY_AVAILABLE else to_addr_list,
            email_body
        )
        server.quit()
        
        if problems:
            logger.warning(f"Some emails failed to send: {problems}")
        else:
            logger.info(f"Email sent successfully to {valid_to_list if EMAIL_SECURITY_AVAILABLE else to_addr_list}")
    except smtplib.SMTPAuthenticationError as e:
        logger.error(f"SMTP authentication failed: {e}")
        raise
    except smtplib.SMTPException as e:
        logger.error(f"SMTP error: {e}")
        raise
    except Exception as e:
        logger.error(f"Error sending email: {e}")
        raise

