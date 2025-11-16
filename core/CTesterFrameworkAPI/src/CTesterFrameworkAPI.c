//
//  ASTFrameworkAPI.c
//  ASTFrameworkAPI
//
//  Created by Jorge Ferrando on 12/04/13.
//  Copyright (c) 2013 Jorge Ferrando. All rights reserved.
//

#include "Precompiled.h"
#include "CTesterFrameworkDefines.h"
#include "Logging.h"
#include "AsirikuyLogger.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// Pantheios removed - using AsirikuyLogger for logging
// const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = PANTHEIOS_LITERAL_STRING("AsirikuyCTester");

void __stdcall getCTesterFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix){
	*pMajor  = AST_VERSION_MAJOR;
	*pMinor  = AST_VERSION_MINOR;
	*pBugfix = AST_VERSION_BUGFIX;
  }

static void seedRand()
{
#if defined _WIN32 || defined _WIN64
  srand(GetTickCount());
#elif defined __APPLE__ || defined __linux__
  struct timeval t1;
  gettimeofday(&t1, NULL);
  srand(t1.tv_usec * t1.tv_sec);
#else
#  error "Unsupported operating system"
#endif
}

static int startCTesterFramework(char* pAsirikuyTesterLog, int severityLevel)
{
  static BOOL initialized  = FALSE;
  static BOOL initializing = FALSE;

  if(initialized)
  {
    return (int)SUCCESS;
  }

  if(initializing)
  {
    return (int)WAIT_FOR_INIT;
  }

  initializing = TRUE;
  seedRand();

  // Initialize the common logger
  asirikuyLoggerInit(pAsirikuyTesterLog, severityLevel);

  logNotice("CTesterFramework initialization complete.");
  
  initialized = TRUE;
  initializing = FALSE;
  return (int)SUCCESS;
}

int __stdcall initCTesterFramework(char* pAsirikuyTesterLog, int severityLevel)
  {
    int returnCode = startCTesterFramework(pAsirikuyTesterLog, severityLevel);
    if(returnCode != SUCCESS)
    {
      return returnCode;
    }

    return SUCCESS;
  }

