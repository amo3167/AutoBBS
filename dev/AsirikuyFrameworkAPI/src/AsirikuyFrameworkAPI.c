/**
 * @file
 * @brief     Entry point for the AsirikuyFramework.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @author    Daniel Fernandez (Assisted with design and code styling)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#include "Precompiled.h"

#if defined __APPLE__ || defined __linux__
  #include <sys/time.h>
  #include <unistd.h>
#endif

#include <ta_libc.h>

#include "AsirikuyFrameworkAPI.h"
#include "FrameworkVersion.h"
#include "AsirikuyConfig.h"
#include "StrategyUserInterface.h"
#include "Broker-tz.h"
#include "TimeZoneOffsets.h"
#include "ContiguousRatesCircBuf.h"
#include "Logging.h"
#include "EquityLog.h"
#include "CriticalSection.h"
#include "InstanceStates.h"
#include "NTPCWrapper.hpp"
#include "TradingWeekBoundaries.h"

#define LOG_FILENAME "AsirikuyFramework.log"

// Pantheios removed - using standard fprintf for logging
// const PAN_CHAR_T PANTHEIOS_FE_PROCESS_IDENTITY[] = PANTHEIOS_LITERAL_STRING("AsirikuyFramework");

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

static void sleepMilliseconds(int milliseconds)
{
#if defined _WIN32 || defined _WIN64
  Sleep(milliseconds);
#elif defined __APPLE__ || defined __linux__
  usleep(milliseconds * 1000);
#else
  #error "Unsupported operating system"
#endif
}

static int initFramework(char* pAsirikuyConfig, char* pAccountName)
{
  static BOOL initialized  = FALSE;
  static BOOL initializing = FALSE;
  AsirikuyReturnCode result;
  TA_RetCode retCode;
  AsirikuyConfig config;
  char brokerTZPath[MAX_FILE_PATH_CHARS] = "";
  // Pantheios removed - using standard fprintf for logging
  // char pantheiosLogPath[MAX_FILE_PATH_CHARS] = "";

  if(!initialized)
  {
    enterCriticalSection();
	 if(initializing)
    {
      leaveCriticalSection();
      return (int)WAIT_FOR_INIT;
    }
    initializing = TRUE;
    leaveCriticalSection();
  }

  if(initialized)
  {
    initializing = FALSE;
    return (int)SUCCESS;
  }

  if(pAsirikuyConfig == NULL)
  {
    initializing = FALSE;
    return (int)INVALID_CONFIG;
  }

  strcpy(config.configFileName, pAsirikuyConfig);
  // Pantheios removed - severity level no longer needed
  // config.loggingConfig.severityLevel = PANTHEIOS_SEV_NOTICE;
  config.ratesBufferExtension = DEFAULT_RATES_BUF_EXT;
  result = parseConfigFile(&config);
  if(result != SUCCESS)
  {
    initializing = FALSE;
    return (int)result;
  }

  seedRand();

  // Pantheios removed - using standard fprintf for logging
  // strcat(pantheiosLogPath, config.loggingConfig.logFolder);
  // strcat(pantheiosLogPath, "/");
  // strcat(pantheiosLogPath, pAccountName);
  // strcat(pantheiosLogPath, LOG_FILENAME);
  // pantheios_init();
  // pantheios_be_file_setFilePath((PAN_CHAR_T*)pantheiosLogPath, 0, 0, PANTHEIOS_BEID_ALL);
  // pantheios_fe_simple_setSeverityCeiling(config.loggingConfig.severityLevel);
  fprintf(stderr, "[NOTICE] AsirikuyFramework initialized.\n");

  retCode = TA_Initialize();
  if(retCode != TA_SUCCESS)
  {
    logTALibError("initFramework()", retCode);
    initializing = FALSE;
    return (int)TA_LIB_ERROR;
  }
  fprintf(stderr, "[NOTICE] TA-Lib initialized.\n");

  strcat(brokerTZPath, config.configFilePaths.configFolderPath);
  strcat(brokerTZPath, "/");
  strcat(brokerTZPath, config.configFilePaths.brokerTzFileName);
  result = parseTimezoneConfig(brokerTZPath);
  if(result != SUCCESS)
  {
    logAsirikuyError("initFramework()", result);
    initializing = FALSE;
    return (int)result;
  }
  fprintf(stderr, "[NOTICE] Loaded broker timezone configuration.\n");

  setExtendedBufferSize(config.ratesBufferExtension);
  resetAllRatesBuffers();
  fprintf(stderr, "[NOTICE] Rates buffers initialized.\n");

  initEquityLog(config.loggingConfig.enableEquityLog, config.loggingConfig.logFolder);
  initializeInstanceStates(config.tempFileFolderPath);

  initExtendedEntryBarLog(config.loggingConfig.enableEntryLog, config.loggingConfig.entryBarNumber,config.loggingConfig.logFolder);

  setNtpUpdateInterval(config.ntpConfig.updateInterval);
  setNtpTimeout(config.ntpConfig.timeout);
  setTotalNtpReferenceTimes(config.ntpConfig.totalReferenceTimes);
  fprintf(stderr, "[NOTICE] NTPClient initialized.\n");

  setTempFileFolderPath(config.tempFileFolderPath);
  setTradingWeekBoundaries(config.cropMondayHours, config.cropFridayHours);

  initialized  = TRUE;
  initializing = FALSE;
  fprintf(stderr, "[NOTICE] Framework initialization complete.\n");
  return (int)SUCCESS;
}

static int initInstance(int instanceId, int isTesting, char* pAsirikuyConfig, char* pAccountName)
{
  int returnCode = initFramework(pAsirikuyConfig, pAccountName);
  if(returnCode != SUCCESS)
  {
    return returnCode;
  }

  closeEquityLog();
  resetInstanceBuffer(instanceId);

  if(isTesting)
  {
    resetInstanceState(instanceId);
  }
  else
  {
    loadInstanceState(instanceId);
  }

  fprintf(stderr, "[NOTICE] Initialized instance ID: %d.\n", instanceId);

  return returnCode;
}

#ifdef __cplusplus
extern "C" {
#endif

  int __stdcall initInstanceMQL4(int instanceId, int isTesting, Mql4String* pAsirikuyConfig)
  {
    int returnCode = initInstance(instanceId, isTesting, pAsirikuyConfig->string, "");
    if(returnCode != SUCCESS)
    {
      return returnCode;
    }

    return SUCCESS;
  }

  int __stdcall initInstanceMQL5(int instanceId, int isTesting, char* pAsirikuyConfig)
  {
    int returnCode = initInstance(instanceId, isTesting, pAsirikuyConfig, "");
    if(returnCode != SUCCESS)
    {
      return returnCode;
    }

    return SUCCESS;
  }

  int __stdcall initInstanceC(int instanceId, int isTesting, char* pAsirikuyConfig, char* pAccountName)
  {
    int returnCode = initInstance(instanceId, isTesting, pAsirikuyConfig, pAccountName);
    if(returnCode != SUCCESS)
    {
      return returnCode;
    }

    return SUCCESS;
  }

  void __stdcall deinitInstance(int instanceId)
  {
    closeEquityLog();
    resetInstanceBuffer(instanceId);
  }

  void __stdcall getFrameworkVersion(int* pMajor, int* pMinor, int* pBugfix)
  {
    *pMajor  = VERSION_MAJOR;
    *pMinor  = VERSION_MINOR;
    *pBugfix = VERSION_BUGFIX;
  }

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined _WIN32 || defined _WIN64
BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID reserved)
{
  switch(reasonForCall)
  {
  case DLL_PROCESS_ATTACH:
    {
      initCriticalSection();
      break;
    }
  case DLL_PROCESS_DETACH:
    {
      deinitCriticalSection();
      break;
    }
  case DLL_THREAD_ATTACH:
    {
      break;
    }
  case DLL_THREAD_DETACH:
    {
      break;
    }
  default:
    {
      break;
    }
  }

  return TRUE;
}
#elif defined __APPLE__ || defined __linux__
void __attribute__ ((constructor)) load(void);
void __attribute__ ((destructor)) unload(void);

/* Called when the library is loaded and before dlopen() returns */
void load(void)
{
  initCriticalSection();
}

/* Called when the library is unloaded and before dlclose() returns */
void unload(void)
{
  deinitCriticalSection();
}

#else
#  error "Unsupported operating system"
#endif
