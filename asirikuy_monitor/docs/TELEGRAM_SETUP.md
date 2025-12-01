# Telegram Bot Setup Guide

This guide will help you set up Telegram Bot notifications for Asirikuy Monitor.

## Why Telegram?

- ✅ **100% Free** - No limits, no paid tiers
- ✅ **Instant** - Real-time push notifications
- ✅ **Mobile App** - Get alerts on your phone
- ✅ **No Spam** - Direct delivery, no email filters
- ✅ **Easy Setup** - Just 5 minutes

---

## Step 1: Create a Telegram Bot

1. **Open Telegram** and search for `@BotFather`
2. **Start a chat** with BotFather
3. **Send command**: `/newbot`
4. **Follow prompts**:
   - Choose a name for your bot (e.g., "Asirikuy Monitor Bot")
   - Choose a username (must end in `bot`, e.g., `asirikuy_monitor_bot`)
5. **Save the bot token** - BotFather will give you a token like:
   ```
   123456789:ABCdefGHIjklMNOpqrsTUVwxyz
   ```
   ⚠️ **Keep this token secret!**

---

## Step 2: Get Your Chat ID

You need your Telegram chat ID to receive messages. There are two methods:

### Method 1: Using @userinfobot (Easiest)

1. Search for `@userinfobot` on Telegram
2. Start a chat and send `/start`
3. The bot will reply with your chat ID (a number like `123456789`)

### Method 2: Using API (Alternative)

1. Send a message to your bot (any message)
2. Visit this URL in your browser (replace `YOUR_BOT_TOKEN` with your actual token):
   ```
   https://api.telegram.org/botYOUR_BOT_TOKEN/getUpdates
   ```
3. Look for `"chat":{"id":123456789}` in the JSON response
4. The number is your chat ID

---

## Step 3: Configure Asirikuy Monitor

### Option A: Using Environment Variables (Recommended)

Create a `.env` file in the project root:

```bash
# Telegram Configuration
TELEGRAM_BOT_TOKEN=123456789:ABCdefGHIjklMNOpqrsTUVwxyz
TELEGRAM_CHAT_ID=123456789
```

### Option B: Using Config File

Edit `config/checker.config`:

```ini
[general]
# ... other settings ...

# Enable Telegram notifications
useTelegram = 1

# Telegram Bot credentials
telegramBotToken = 123456789:ABCdefGHIjklMNOpqrsTUVwxyz
telegramChatId = 123456789
```

---

## Step 4: Install Dependencies

Make sure the `requests` library is installed:

```bash
pip install requests
```

Or install all dependencies:

```bash
pip install -r requirements.txt
```

---

## Step 5: Test Configuration

Run the monitor with version command to verify:

```bash
python3 checker.py -v
```

You should see no errors about Telegram configuration.

---

## Step 6: Test Notification

Send a test message to your bot on Telegram. The monitor will send notifications when:

- ❌ **Heartbeat fails** - Trading system stops responding
- ❌ **Errors detected** - Critical errors in log files

---

## Notification Examples

### Heartbeat Alert
```
🚨 Account1 - Heartbeat Alert

⚠️ Heart-Beat problem for instance instance1.hb, system not updating for more than 150.0 secs
```

### Error Alert
```
❌ Account1 - Error Detected

❌ 2024-12-02 14:30:00 - Error: Connection failed
```

---

## Multiple Recipients

To send to multiple Telegram chats:

1. Get chat IDs for all recipients
2. Use comma-separated chat IDs in config:
   ```ini
   telegramChatId = 123456789,987654321,555555555
   ```

Or set multiple environment variables (requires code modification).

---

## Troubleshooting

### "Telegram notifier not available"
- **Solution**: Install requests library
  ```bash
  pip install requests
  ```

### "Bot token or chat ID not configured"
- **Solution**: Check your `.env` file or config file
- Make sure `useTelegram = 1` is set

### "Telegram API request failed"
- **Solution**: 
  - Verify bot token is correct
  - Verify chat ID is correct
  - Make sure you've sent at least one message to your bot
  - Check internet connection

### Not receiving messages
- **Solution**:
  - Make sure you've sent a message to your bot first
  - Verify chat ID is correct
  - Check bot token is correct
  - Check monitor logs for errors

---

## Security Best Practices

1. ✅ **Use environment variables** instead of config file for tokens
2. ✅ **Keep bot token secret** - don't commit to git
3. ✅ **Add `.env` to `.gitignore`** (already done)
4. ✅ **Use different bots** for different environments (dev/prod)

---

## Advanced: Group Chats

You can also send notifications to Telegram groups:

1. Add your bot to a Telegram group
2. Get the group chat ID:
   - Send a message in the group
   - Visit: `https://api.telegram.org/botYOUR_BOT_TOKEN/getUpdates`
   - Look for `"chat":{"id":-123456789}` (negative number for groups)
3. Use the group chat ID in your config

---

## Comparison: Email vs Telegram

| Feature | Email | Telegram |
|---------|-------|----------|
| **Speed** | Slow (minutes) | Instant (seconds) |
| **Setup** | Medium | Easy |
| **Cost** | Free | Free |
| **Spam** | Possible | No |
| **Mobile** | Yes | Yes (better) |
| **Formatting** | Basic | Rich (HTML) |

---

## Need Help?

- Check monitor logs: `log/monitor.log`
- Test bot token: Visit `https://api.telegram.org/botYOUR_BOT_TOKEN/getMe`
- Telegram Bot API docs: https://core.telegram.org/bots/api

---

**Enjoy instant notifications! 🚀**

