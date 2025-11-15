/**
 * Standalone test for AsirikuyLogger multi-file support
 * Compile: gcc -o test_logger_multifile test_logger_multifile.c -I../core/AsirikuyCommon/include -L../bin/gmake/x64/Debug -lAsirikuyCommon -ldl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/AsirikuyCommon/include/AsirikuyLogger.h"

int main(int argc, char* argv[])
{
    printf("=== Testing AsirikuyLogger Multi-File Support ===\n\n");
    
    // Test 1: Initialize first log file
    printf("Test 1: Initializing first log file (test1.log)...\n");
    int result1 = asirikuyLoggerInit("./log/test1.log", LOG_DEBUG);
    printf("Result: %d\n", result1);
    logInfo("This is a test message to log file 1\n");
    
    // Test 2: Initialize second log file
    printf("\nTest 2: Initializing second log file (test2.log)...\n");
    int result2 = asirikuyLoggerInit("./log/test2.log", LOG_DEBUG);
    printf("Result: %d\n", result2);
    logInfo("This is a test message to log file 2\n");
    
    // Test 3: Initialize third log file
    printf("\nTest 3: Initializing third log file (test3.log)...\n");
    int result3 = asirikuyLoggerInit("./log/test3.log", LOG_DEBUG);
    printf("Result: %d\n", result3);
    logInfo("This is a test message to log file 3\n");
    
    // Test 4: Log to all files
    printf("\nTest 4: Logging message that should appear in all 3 files...\n");
    logInfo("This message should appear in all log files\n");
    
    printf("\n=== Test Complete ===\n");
    printf("Check log/test1.log, log/test2.log, and log/test3.log\n");
    
    return 0;
}

