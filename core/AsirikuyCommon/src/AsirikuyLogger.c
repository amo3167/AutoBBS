/**
 * @file
 * @brief     Simple file logger implementation with severity levels
 * 
 * @author    Auto (Implementation)
 * @version   F4.x.x
 * @date      2025
 */

#include "Precompiled.h"
#include "AsirikuyLogger.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

// Logger state
static FILE* gLogFile = NULL;
static int gSeverityLevel = LOG_INFO; // Default to Info level
static BOOL gInitialized = FALSE;

// Get severity level label
static const char* getSeverityLabel(int severity)
{
  switch(severity)
  {
    case LOG_EMERGENCY: return "EMERGENCY";
    case LOG_ALERT:     return "ALERT";
    case LOG_CRITICAL:  return "CRITICAL";
    case LOG_ERROR:     return "ERROR";
    case LOG_WARNING:   return "WARNING";
    case LOG_NOTICE:    return "NOTICE";
    case LOG_INFO:      return "INFO";
    case LOG_DEBUG:     return "DEBUG";
    default:            return "UNKNOWN";
  }
}

// Get current timestamp string
static void getTimestamp(char* buffer, size_t bufferSize)
{
  time_t now;
  struct tm* timeinfo;
  
  time(&now);
  timeinfo = localtime(&now);
  
#if defined _WIN32 || defined _WIN64
  snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
           timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
  strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", timeinfo);
#endif
}

// Helper function to create directory if it doesn't exist
static void ensureDirectoryExists(const char* filePath)
{
  char dirPath[MAX_FILE_PATH_CHARS] = "";
  char* lastSlash = NULL;
  
  strncpy(dirPath, filePath, MAX_FILE_PATH_CHARS - 1);
  dirPath[MAX_FILE_PATH_CHARS - 1] = '\0';
  
  // Find last slash
  lastSlash = strrchr(dirPath, '/');
#if defined _WIN32 || defined _WIN64
  if(lastSlash == NULL)
  {
    lastSlash = strrchr(dirPath, '\\');
  }
#endif
  
  if(lastSlash != NULL)
  {
    *lastSlash = '\0';
    
    // Create directory
#if defined _WIN32 || defined _WIN64
    CreateDirectoryA(dirPath, NULL);
#elif defined __APPLE__ || defined __linux__
    char mkdirCmd[MAX_FILE_PATH_CHARS + 10] = "";
    snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p \"%s\"", dirPath);
    system(mkdirCmd);
#endif
  }
}

int asirikuyLoggerInit(const char* pLogFilePath, int severityLevel)
{
  if(gInitialized)
  {
    return 0; // Already initialized
  }

  gSeverityLevel = severityLevel;

  // Open log file if path provided
  if(pLogFilePath != NULL && strlen(pLogFilePath) > 0)
  {
    // Ensure directory exists
    ensureDirectoryExists(pLogFilePath);
    
    // Open log file in append mode
    gLogFile = fopen(pLogFilePath, "a");
    if(gLogFile == NULL)
    {
      // If append fails, try write mode
      gLogFile = fopen(pLogFilePath, "w");
    }
    
    if(gLogFile != NULL)
    {
      // Write header (bypass severity check for initialization)
      char timestamp[32] = "";
      getTimestamp(timestamp, sizeof(timestamp));
      fprintf(gLogFile, "\n=== Asirikuy Logger Started ===\n");
      fprintf(gLogFile, "[%s] Log file: %s\n", timestamp, pLogFilePath);
      fprintf(gLogFile, "[%s] Severity level: %d (%s)\n", timestamp, severityLevel, getSeverityLabel(severityLevel));
      fprintf(gLogFile, "[%s] All messages with severity <= %d will be logged\n", timestamp, severityLevel);
      fflush(gLogFile);
    }
    else
    {
      fprintf(stderr, "[WARNING] Failed to open log file: %s. Logging to stderr only.\n", pLogFilePath);
    }
  }

  gInitialized = TRUE;
  return 0;
}

void asirikuyLogMessage(int severity, const char* format, ...)
{
  // Check if this severity level should be logged
  if(severity > gSeverityLevel)
  {
    return; // Skip logging for levels above the threshold
  }
  
  va_list args;
  char timestamp[32] = "";
  char messageBuffer[1024] = "";
  char logLine[1124] = "";
  
  // Get timestamp
  getTimestamp(timestamp, sizeof(timestamp));
  
  // Format the message
  va_start(args, format);
  vsnprintf(messageBuffer, sizeof(messageBuffer) - 1, format, args);
  messageBuffer[sizeof(messageBuffer) - 1] = '\0';
  va_end(args);
  
  // Format the full log line with timestamp and severity
  snprintf(logLine, sizeof(logLine), "[%s] [%s] %s", 
           timestamp, getSeverityLabel(severity), messageBuffer);
  
  // Ensure newline
  size_t len = strlen(logLine);
  if(len > 0 && logLine[len - 1] != '\n')
  {
    if(len < sizeof(logLine) - 1)
    {
      logLine[len] = '\n';
      logLine[len + 1] = '\0';
    }
  }
  
  // Write to stderr (always, for critical messages)
  if(severity <= LOG_ERROR)
  {
    fprintf(stderr, "%s", logLine);
  }
  
  // Write to log file if open
  if(gLogFile != NULL)
  {
    fprintf(gLogFile, "%s", logLine);
    fflush(gLogFile); // Ensure immediate write
  }
}

