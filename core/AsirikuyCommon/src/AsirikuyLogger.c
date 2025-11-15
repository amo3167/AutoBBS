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
#include <errno.h>

// Maximum number of log files (support multiple loggers)
#define MAX_LOG_FILES 4

// Logger state
static FILE* gLogFiles[MAX_LOG_FILES] = {NULL, NULL, NULL, NULL};
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
  // Create test file to verify this function is called
  FILE* testFile = fopen("/tmp/asirikuyLoggerInit_called.log", "a");
  if(testFile != NULL)
  {
    fprintf(testFile, "asirikuyLoggerInit called: path='%s', severity=%d\n", 
            pLogFilePath ? pLogFilePath : "NULL", severityLevel);
    fclose(testFile);
  }
  
  // Debug: Log initialization attempt
  fprintf(stderr, "[DEBUG] asirikuyLoggerInit called: path='%s', severity=%d\n", 
          pLogFilePath ? pLogFilePath : "NULL", severityLevel);
  fflush(stderr);
  
  // Update severity level (use lowest/most restrictive severity if multiple loggers)
  // Lower numbers = more restrictive (only critical errors), higher numbers = less restrictive (everything)
  // If this is the first initialization or the new severity is more restrictive, use it
  if(!gInitialized || severityLevel < gSeverityLevel)
  {
    gSeverityLevel = severityLevel;
  }

  // Open log file if path provided
  if(pLogFilePath != NULL && strlen(pLogFilePath) > 0)
  {
    fprintf(stderr, "[DEBUG] asirikuyLoggerInit: Opening log file: %s\n", pLogFilePath);
    fflush(stderr);
    // Check if this log file is already open
    int i;
    for(i = 0; i < MAX_LOG_FILES; i++)
    {
      if(gLogFiles[i] != NULL)
      {
        // Check if this is the same file (simple string comparison)
        // Note: This is a simple check - in practice, we'd need to track file paths
        // For now, we'll allow multiple files to be opened
      }
    }
    
    // Find an empty slot for the new log file
    int slot = -1;
    for(i = 0; i < MAX_LOG_FILES; i++)
    {
      if(gLogFiles[i] == NULL)
      {
        slot = i;
        break;
      }
    }
    
    if(slot == -1)
    {
      // All slots full, replace the first one (or we could append to existing)
      fprintf(stderr, "[WARNING] Maximum log files (%d) reached. Reusing first slot.\n", MAX_LOG_FILES);
      if(gLogFiles[0] != NULL && gLogFiles[0] != stderr)
      {
        fclose(gLogFiles[0]);
      }
      slot = 0;
    }
    
    // Ensure directory exists
    ensureDirectoryExists(pLogFilePath);
    
    // Open log file in append mode
    gLogFiles[slot] = fopen(pLogFilePath, "a");
    if(gLogFiles[slot] == NULL)
    {
      // If append fails, try write mode
      gLogFiles[slot] = fopen(pLogFilePath, "w");
    }
    
    if(gLogFiles[slot] != NULL)
    {
      fprintf(stderr, "[DEBUG] asirikuyLoggerInit: Successfully opened log file in slot %d: %s\n", slot, pLogFilePath);
      fflush(stderr);
      // Write header (bypass severity check for initialization)
      char timestamp[32] = "";
      getTimestamp(timestamp, sizeof(timestamp));
      fprintf(gLogFiles[slot], "\n=== Asirikuy Logger Started ===\n");
      fprintf(gLogFiles[slot], "[%s] Log file: %s\n", timestamp, pLogFilePath);
      fprintf(gLogFiles[slot], "[%s] Severity level: %d (%s)\n", timestamp, severityLevel, getSeverityLabel(severityLevel));
      fprintf(gLogFiles[slot], "[%s] All messages with severity <= %d will be logged\n", timestamp, severityLevel);
      fflush(gLogFiles[slot]);
    }
    else
    {
      fprintf(stderr, "[WARNING] Failed to open log file: %s. Logging to stderr only.\n", pLogFilePath);
      fprintf(stderr, "[WARNING] Error details: errno=%d, path='%s'\n", errno, pLogFilePath);
      fflush(stderr);
      gLogFiles[slot] = NULL;
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
  
  // Write to all open log files
  int i;
  for(i = 0; i < MAX_LOG_FILES; i++)
  {
    if(gLogFiles[i] != NULL && gLogFiles[i] != stderr)
    {
      fprintf(gLogFiles[i], "%s", logLine);
      fflush(gLogFiles[i]); // Ensure immediate write
    }
  }
}

