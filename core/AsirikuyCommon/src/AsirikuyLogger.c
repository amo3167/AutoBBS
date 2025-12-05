/**
 * @file
 * @brief     Simple file logger implementation with severity levels
 * 
 * @author    Auto (Implementation)
 * @version   F4.x.x
 * @date      2025
 */

#include <stdlib.h>
#include "AsirikuyLogger.h"
#include "AsirikuyDefines.h"
#include "CriticalSection.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
/* #include <chrono> is C++ only - use C standard time.h instead */
#include <errno.h>
#ifdef _WIN32
  #include <windows.h>
#endif

// Maximum number of log files (support multiple loggers)
#define MAX_LOG_FILES 4

// Logger state
static FILE* gLogFiles[MAX_LOG_FILES] = {NULL, NULL, NULL, NULL};
static char gLogFilePaths[MAX_LOG_FILES][MAX_FILE_PATH_CHARS] = {{0}}; // Track file paths to prevent duplicates
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

// Get current timestamp string (thread-safe)
static void getTimestamp(char* buffer, size_t bufferSize)
{
  time_t now;
  struct tm timeinfo;
  
  time(&now);
  
#if defined _WIN32 || defined _WIN64
  localtime_s(&timeinfo, &now);
  snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
#else
  localtime_r(&now, &timeinfo);
  strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
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
  
  // Thread-safe access to shared logger state
  enterCriticalSection();
  
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
    
    // Check if this log file is already open (prevent duplicates)
    int i;
    int existingSlot = -1;
    for(i = 0; i < MAX_LOG_FILES; i++)
    {
      if(gLogFiles[i] != NULL && strlen(gLogFilePaths[i]) > 0)
      {
        // Check if this is the same file path (case-sensitive string comparison)
        if(strcmp(gLogFilePaths[i], pLogFilePath) == 0)
        {
          existingSlot = i;
          fprintf(stderr, "[DEBUG] asirikuyLoggerInit: File already open in slot %d, reusing: %s\n", i, pLogFilePath);
          fflush(stderr);
          break;
        }
      }
    }
    
    // If file is already open, just update severity level and return
    if(existingSlot >= 0)
    {
      // File already open, no need to open again - just update severity if needed
      leaveCriticalSection();
      return 0;
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
        fflush(gLogFiles[0]); // Flush before closing
        fclose(gLogFiles[0]);
      }
      gLogFiles[0] = NULL;
      gLogFilePaths[0][0] = '\0'; // Clear the path
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
      // Store the file path to prevent duplicate opens
      strncpy(gLogFilePaths[slot], pLogFilePath, MAX_FILE_PATH_CHARS - 1);
      gLogFilePaths[slot][MAX_FILE_PATH_CHARS - 1] = '\0';
      
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
      gLogFilePaths[slot][0] = '\0'; // Clear the path
    }
  }

  gInitialized = TRUE;
  
  leaveCriticalSection();
  return 0;
}

void asirikuyLogMessage(int severity, const char* format, ...)
{
  // Thread-safe access to shared logger state
  enterCriticalSection();
  
  // Check if this severity level should be logged
  if(severity > gSeverityLevel)
  {
    leaveCriticalSection();
    return; // Skip logging for levels above the threshold
  }
  
  va_list args;
  char timestamp[32] = "";
  char messageBuffer[1024] = "";
  char logLine[1124] = "";
  
  // Get timestamp (thread-safe)
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
  
  // Write to all open log files (protected by critical section)
  // Use OS default buffering - no explicit flushes, let OS handle it
  int i;
  for(i = 0; i < MAX_LOG_FILES; i++)
  {
    if(gLogFiles[i] != NULL && gLogFiles[i] != stderr)
    {
      fprintf(gLogFiles[i], "%s", logLine);
      // No explicit flush - rely on OS buffering for best performance
    }
  }
  
  leaveCriticalSection();
}

