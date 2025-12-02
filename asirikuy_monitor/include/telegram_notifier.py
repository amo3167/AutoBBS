"""
Telegram Bot Notifier for Asirikuy Monitor
Provides instant push notifications via Telegram Bot API
"""

import logging
import requests
from typing import List, Optional
from urllib.parse import quote

logger = logging.getLogger(__name__)

# Telegram Bot API endpoint
TELEGRAM_API_URL = "https://api.telegram.org/bot{token}/sendMessage"


def send_telegram_message(
    bot_token: str,
    chat_id: str,
    message: str,
    parse_mode: str = "HTML",
    disable_notification: bool = False
) -> bool:
    """
    Send a message via Telegram Bot API
    
    Args:
        bot_token: Telegram bot token (from @BotFather)
        chat_id: Telegram chat ID (user or group)
        message: Message text to send
        parse_mode: Message parsing mode ("HTML", "Markdown", or "MarkdownV2")
        disable_notification: If True, sends silently (no sound)
    
    Returns:
        bool: True if message sent successfully, False otherwise
    
    Raises:
        ValueError: If bot_token or chat_id is invalid
        requests.RequestException: If API request fails
    """
    if not bot_token or not bot_token.strip():
        raise ValueError("Telegram bot token is required")
    
    if not chat_id or not chat_id.strip():
        raise ValueError("Telegram chat ID is required")
    
    if not message or not message.strip():
        logger.warning("Empty message, skipping Telegram notification")
        return False
    
    # Sanitize message (escape HTML if using HTML parse mode)
    if parse_mode == "HTML":
        # Basic HTML escaping for special characters
        message = message.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    
    # Build API URL
    api_url = TELEGRAM_API_URL.format(token=bot_token)
    
    # Prepare payload
    payload = {
        "chat_id": chat_id,
        "text": message,
        "parse_mode": parse_mode,
        "disable_notification": disable_notification
    }
    
    try:
        # Send request
        response = requests.post(
            api_url,
            json=payload,
            timeout=10  # 10 second timeout
        )
        
        # Check response
        response.raise_for_status()
        result = response.json()
        
        # Verify success
        if result.get("ok"):
            logger.info(f"Telegram message sent successfully to chat {chat_id}")
            return True
        else:
            error_description = result.get("description", "Unknown error")
            logger.error(f"Telegram API error: {error_description}")
            return False
            
    except requests.exceptions.Timeout:
        logger.error("Telegram API request timed out")
        return False
    except requests.exceptions.RequestException as e:
        logger.error(f"Telegram API request failed: {e}")
        return False
    except ValueError as e:
        logger.error(f"Invalid Telegram response: {e}")
        return False
    except Exception as e:
        logger.error(f"Unexpected error sending Telegram message: {e}")
        return False


def send_telegram_alert(
    bot_token: str,
    chat_id: str,
    title: str,
    message: str,
    alert_type: str = "INFO"
) -> bool:
    """
    Send a formatted alert message via Telegram
    
    Args:
        bot_token: Telegram bot token
        chat_id: Telegram chat ID
        title: Alert title
        message: Alert message body
        alert_type: Alert type ("INFO", "WARNING", "ERROR", "CRITICAL")
    
    Returns:
        bool: True if message sent successfully, False otherwise
    """
    # Emoji mapping for alert types
    emoji_map = {
        "INFO": "ℹ️",
        "WARNING": "⚠️",
        "ERROR": "❌",
        "CRITICAL": "🚨"
    }
    
    emoji = emoji_map.get(alert_type.upper(), "📢")
    
    # Format message with HTML
    formatted_message = f"<b>{emoji} {title}</b>\n\n{message}"
    
    # Use HTML parse mode for formatting
    return send_telegram_message(
        bot_token=bot_token,
        chat_id=chat_id,
        message=formatted_message,
        parse_mode="HTML",
        disable_notification=(alert_type.upper() == "INFO")
    )


def send_telegram_to_multiple(
    bot_token: str,
    chat_ids: List[str],
    message: str,
    parse_mode: str = "HTML"
) -> dict:
    """
    Send message to multiple Telegram chats
    
    Args:
        bot_token: Telegram bot token
        chat_ids: List of Telegram chat IDs
        message: Message text
        parse_mode: Message parsing mode
    
    Returns:
        dict: Results for each chat ID {"chat_id": success (bool)}
    """
    results = {}
    
    for chat_id in chat_ids:
        if chat_id and chat_id.strip():
            success = send_telegram_message(
                bot_token=bot_token,
                chat_id=chat_id,
                message=message,
                parse_mode=parse_mode
            )
            results[chat_id] = success
        else:
            logger.warning(f"Invalid chat ID skipped: {chat_id}")
            results[chat_id] = False
    
    return results


def validate_telegram_config(bot_token: str, chat_id: str) -> tuple[bool, str]:
    """
    Validate Telegram configuration by testing API connection
    
    Args:
        bot_token: Telegram bot token
        chat_id: Telegram chat ID
    
    Returns:
        tuple: (is_valid: bool, error_message: str)
    """
    if not bot_token or not bot_token.strip():
        return False, "Bot token is required"
    
    if not chat_id or not chat_id.strip():
        return False, "Chat ID is required"
    
    # Test with a simple message
    test_message = "✅ Asirikuy Monitor: Telegram notification test"
    try:
        success = send_telegram_message(
            bot_token=bot_token,
            chat_id=chat_id,
            message=test_message,
            disable_notification=True
        )
        if success:
            return True, "Telegram configuration is valid"
        else:
            return False, "Failed to send test message"
    except Exception as e:
        return False, f"Telegram validation error: {str(e)}"

