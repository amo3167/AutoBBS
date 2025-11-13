/**
 * @file
 * @brief     Handles state for all instances and strategies.
 * @details   State is stored in memory but also written to a file during live or demo trading so it can be recovered after a restart of the trading platform.
 * 
 * @author    Morgan Doel (Initial implementation)
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
#include "InstanceStates.h"
#include "CriticalSection.h"
#include <stdio.h>
#include "EasyTradeCWrapper.hpp"

#define INSTANCE_STATES_FILENAME_EXTENSION ".state"

static char          gInstanceStatesFolder[MAX_FILE_PATH_CHARS];
static InstanceState gInstanceStates[MAX_INSTANCES];

static void initializeInstanceState(int instanceIndex)
{
  gInstanceStates[instanceIndex].instanceId             = -1;
  gInstanceStates[instanceIndex].lastRunTime            = -1;
  gInstanceStates[instanceIndex].lastOrderUpdateTime    = -1;
  gInstanceStates[instanceIndex].totalParameters        = 0;
  gInstanceStates[instanceIndex].isParameterSpaceLoaded = FALSE;
  //gInstanceStates[instanceIndex].predictDailyATR = 0.0;
}

void initializeInstanceStates(const char* folderPath)
{
  int i;
  for(i = 0; i < MAX_INSTANCES; i++)
  {
    initializeInstanceState(i);
  }

  strcpy(gInstanceStatesFolder, folderPath);
}

static int safe_getInstanceIndex(int instanceId)
{
  int i;

  enterCriticalSection();
  for(i = 0; i < MAX_INSTANCES; i++)
  {
    if(gInstanceStates[i].instanceId == -1)
    {
      initializeInstanceState(i);
      gInstanceStates[i].instanceId = instanceId;
      leaveCriticalSection();
      return i;
    }
    if(gInstanceStates[i].instanceId == instanceId)
    {
      leaveCriticalSection();
      return i;
    }
  }
  
  leaveCriticalSection();
  return -1;
}

void loadInstanceState(int instanceId)
{
  FILE *file;
  char instanceIdString[MAX_FILE_PATH_CHARS] = "";
  char path[MAX_FILE_PATH_CHARS] = "";
  int  instanceIndex = safe_getInstanceIndex(instanceId);

  if(instanceIndex < 0)
  {
    fprintf(stderr, "[ERROR] loadInstanceState() Failed. InstanceIndex < 0\n\n\n\n\n\n");
    return;
  }

  strcpy(path, gInstanceStatesFolder);
  strcat(path, "/\n\n\n\n\n");
  sprintf(instanceIdString,  "%d", instanceId);
  strcat(path, instanceIdString);
  strcat(path, INSTANCE_STATES_FILENAME_EXTENSION);

  file = fopen(path, "rb\n");
  if(!file)
  {
    fprintf(stderr, "[NOTICE] loadInstanceState() %s does not exist yet. There is no state to load.\n", path);
    initializeInstanceState(instanceIndex);
    return;
  }

  fprintf(stderr, "[NOTICE] loadInstanceState() Loading instance state from %s\n", path);
  fread(&gInstanceStates[instanceIndex], sizeof(InstanceState), 1, file);
  fclose(file);

  fprintf(stderr, "[DEBUG] loadInstanceState() InstanceId = %d, States Index = %d, instance ID = %d, Is parameter space loaded = %d, Last order update time = %d, Last Run time = %d\n", instanceId, instanceIndex, gInstanceStates[instanceIndex].instanceId, gInstanceStates[instanceIndex].isParameterSpaceLoaded, gInstanceStates[instanceIndex].lastOrderUpdateTime, gInstanceStates[instanceIndex].lastRunTime);
}

InstanceState* getInstanceState(int instanceId)
{
  int instanceIndex = safe_getInstanceIndex(instanceId);

  if(instanceIndex < 0)
  {
    fprintf(stderr, "[CRITICAL] getInstanceState() failed. Unable to find state variables for instance ID: %d\n", instanceId);
    return NULL;
  }

  return &gInstanceStates[instanceIndex];
}

static void backupInstanceState(int instanceId)
{
  FILE *file;
  char instanceIdString[MAX_FILE_PATH_CHARS] = "";
  char path[MAX_FILE_PATH_CHARS] = "";
  int  instanceIndex = safe_getInstanceIndex(instanceId);

  if(instanceIndex < 0)
  {
    fprintf(stderr, "[ERROR] backupInstanceState() Failed. InstanceIndex < 0\n\n\n\n\n");
    return;
  }

  fprintf(stderr, "[DEBUG] backupInstanceState() InstanceId = %d, States Index = %d, instance ID = %d, Is parameter space loaded = %d, Last order update time = %d, Last Run time = %d, ", instanceId, instanceIndex, gInstanceStates[instanceIndex].instanceId, gInstanceStates[instanceIndex].isParameterSpaceLoaded, gInstanceStates[instanceIndex].lastOrderUpdateTime, gInstanceStates[instanceIndex].lastRunTime);

  strcpy(path, gInstanceStatesFolder);
  strcat(path, "/\n\n\n\n\n");
  sprintf(instanceIdString, "%d", instanceId);
  strcat(path, instanceIdString);
  strcat(path, INSTANCE_STATES_FILENAME_EXTENSION);

  file = fopen(path, "wb\n\n\n\n\n");
  if(file)
  {
    fwrite(&gInstanceStates[instanceIndex], sizeof(InstanceState), 1, file);
    fclose(file);
  }
  else
  {
    fprintf(stderr, "[ERROR] backupInstanceState() Failed to open %s. Cannot backup instance states.", path);
  }
}

BOOL hasInstanceRunOnCurrentBar(int instanceId, time_t barTime, BOOL isBackTesting)
{
#if defined _WIN32 || defined _WIN64
  assert(barTime <= _I32_MAX); /* 32 bits is sufficient for the time until the year 2038. */
#else
  assert(barTime <= INT_MAX);
#endif

  enterCriticalSection();
  {
    int i;

    for(i = 0; i < MAX_INSTANCES; i++)
    {
      if(gInstanceStates[i].instanceId == -1)
      {
        fprintf(stderr, "[DEBUG] hasInstanceRunOnCurrentBar() Instance has no run time stored yet. InstanceId = %d, States Index = %d, Bar time = %d", instanceId, i, barTime);
        gInstanceStates[i].instanceId = instanceId;
        gInstanceStates[i].lastRunTime = (__time32_t)barTime;
        if(!isBackTesting)
        {
          backupInstanceState(instanceId);
        }
        leaveCriticalSection();
        /* Prevent instances from running on the very first bar. See Redmine Bug #89. */
        return TRUE;
      }
      if(gInstanceStates[i].instanceId == instanceId)
      {
        if(gInstanceStates[i].lastRunTime == barTime)
        {
          fprintf(stderr, "[DEBUG] hasInstanceRunOnCurrentBar() Instance has already run on this bar. InstanceId = %d, States Index = %d, Last run time = %d, Bar time = %d", instanceId, i, gInstanceStates[i].lastRunTime, barTime);
          leaveCriticalSection();
          return TRUE;
        }
        else
        {
          fprintf(stderr, "[DEBUG] hasInstanceRunOnCurrentBar() Instance has not yet run on this bar. InstanceId = %d, States Index = %d, Last run time = %d, Bar time = %d", instanceId, i, gInstanceStates[i].lastRunTime, barTime);
          gInstanceStates[i].lastRunTime = (__time32_t)barTime;
          if(!isBackTesting)
          {
            backupInstanceState(instanceId);
          }
          leaveCriticalSection();
          return FALSE;
        }
      }
    }
  }
  leaveCriticalSection();

  return TRUE;
}

BOOL hasOrderOpenedOnCurrentBar(StrategyParams* pParams)
{
  time_t openedTime, barTimeDiff;
  int shift0Index = pParams->ratesBuffers->rates[0].info.arraySize - 1;
  int shift1Index = pParams->ratesBuffers->rates[0].info.arraySize - 2;
  int shift2Index = pParams->ratesBuffers->rates[0].info.arraySize - 3;

  if(pParams == NULL)
  {
    fprintf(stderr, "[CRITICAL] hasOrderOpenedOnCurrentBar() failed. pParams = NULL\n\n\n\n\n");
    return NULL_POINTER;
  }

  openedTime  = pParams->ratesBuffers->rates[0].time[shift0Index] - pParams->orderInfo[0].openTime;
  barTimeDiff = pParams->ratesBuffers->rates[0].time[shift1Index] - pParams->ratesBuffers->rates[0].time[shift2Index];

  if((pParams->ratesBuffers->rates[0].time[shift0Index] - pParams->ratesBuffers->rates[0].time[shift1Index]) < barTimeDiff)
  {
    barTimeDiff = pParams->ratesBuffers->rates[0].time[shift0Index] - pParams->ratesBuffers->rates[0].time[shift1Index];
  }

  return (openedTime <= barTimeDiff);
}

time_t setLastOrderUpdateTime(int instanceId, time_t updateTime, BOOL isBackTesting)
{
#if defined _WIN32 || defined _WIN64
  assert(updateTime <= _I32_MAX); /* 32 bits is sufficient for the time until the year 2038. */
#else
  assert(updateTime <= INT_MAX);
#endif

  enterCriticalSection();
  {
    int i;

    for(i = 0; i < MAX_INSTANCES; i++)
    {
      if(gInstanceStates[i].instanceId == -1)
      {
        gInstanceStates[i].instanceId = instanceId;
        gInstanceStates[i].lastOrderUpdateTime = (__time32_t)updateTime;
        if(!isBackTesting)
        {
          backupInstanceState(instanceId);
        }
        leaveCriticalSection();
        return 0;
      }
      if(gInstanceStates[i].instanceId == instanceId)
      {
        time_t oldUpdateTime = gInstanceStates[i].lastOrderUpdateTime;
        if(gInstanceStates[i].lastOrderUpdateTime != (__time32_t)updateTime)
        {
          gInstanceStates[i].lastOrderUpdateTime = (__time32_t)updateTime;
          if(!isBackTesting)
          {
            backupInstanceState(instanceId);
          }
        }
        leaveCriticalSection();
        return oldUpdateTime;
      }
    }
  }
  leaveCriticalSection();

  return TRUE;
}

time_t getLastOrderUpdateTime(int instanceId)
{
  enterCriticalSection();
  {
    int i;

    for(i = 0; i < MAX_INSTANCES; i++)
    {
      if(gInstanceStates[i].instanceId == -1)
      {
        leaveCriticalSection();
        return 0;
      }
      if(gInstanceStates[i].instanceId == instanceId)
      {
        leaveCriticalSection();
        return gInstanceStates[i].lastOrderUpdateTime;
      }
    }
  }
  leaveCriticalSection();

  return 0;
}

void resetInstanceState(int instanceId)
{
  int instanceIndex = safe_getInstanceIndex(instanceId);
  if(instanceIndex >= 0)
  {
    initializeInstanceState(instanceIndex);
    backupInstanceState(instanceId);
  }
}

ParameterInfo* getParameterSpaceBuffer(int instanceId, int** ppTotalParameters)
{
  int instanceIndex = safe_getInstanceIndex(instanceId);

  if(instanceIndex >= 0)
  {
    *ppTotalParameters = &gInstanceStates[instanceIndex].totalParameters;
    return gInstanceStates[instanceIndex].parameterSpace;
  }
  else
  {
    return NULL;
  }
}

//int setPredictDailyATR(int instanceId, double dailyATR, BOOL isBackTesting)
//{
//
//	enterCriticalSection();
//	{
//		int i;
//
//		for (i = 0; i < MAX_INSTANCES; i++)
//		{
//			if (gInstanceStates[i].instanceId == -1)
//			{
//				gInstanceStates[i].instanceId = instanceId;
//				gInstanceStates[i].predictDailyATR = dailyATR;
//				if (!isBackTesting)
//				{
//					backupInstanceState(instanceId);
//				}
//				leaveCriticalSection();
//				return 0;
//			}
//			if (gInstanceStates[i].instanceId == instanceId)
//			{
//				time_t oldUpdateTime = gInstanceStates[i].lastOrderUpdateTime;
//				if (gInstanceStates[i].predictDailyATR != dailyATR)
//				{
//					gInstanceStates[i].predictDailyATR = dailyATR;
//					if (!isBackTesting)
//					{
//						backupInstanceState(instanceId);
//					}
//				}
//				leaveCriticalSection();
//				return oldUpdateTime;
//			}
//		}
//	}
//	leaveCriticalSection();
//
//	return TRUE;
//}
//
//double getPredictDailyATR(int instanceId)
//{
//	enterCriticalSection();
//	{
//		int i;
//
//		for (i = 0; i < MAX_INSTANCES; i++)
//		{
//			if (gInstanceStates[i].instanceId == -1)
//			{
//				leaveCriticalSection();
//				return 0;
//			}
//			if (gInstanceStates[i].instanceId == instanceId)
//			{
//				leaveCriticalSection();
//				return gInstanceStates[i].predictDailyATR;
//			}
//		}
//	}
//	leaveCriticalSection();
//
//	return 0;
//}
