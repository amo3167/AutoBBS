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

const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = PANTHEIOS_LITERAL_STRING("AsirikuyCTester");

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
  /*AsirikuyReturnCode result;
  AsirikuyConfig config;*/
  char pantheiosLogPath[MAX_FILE_PATH_CHARS] = "";

  seedRand();

  strcat(pantheiosLogPath, pAsirikuyTesterLog);

  pantheios_init();
  pantheios_be_file_setFilePath((PAN_CHAR_T*)pantheiosLogPath, 0, 0, PANTHEIOS_BEID_ALL);
  pantheios_fe_simple_setSeverityCeiling(severityLevel);
  pantheios_logputs(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"Pantheios initialized.");

  pantheios_logputs(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"CTesterframework initialization complete.");
  
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

