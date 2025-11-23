/**
 * @file
 * @brief     Functions for converting and copying C parameters.
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

#include "CTesterDefines.h"
#include "Precompiled.h"
#include "AsirikuyLogger.h"
#include "ContiguousRatesCircBuf.h"
#include "TimeZoneOffsets.h"
#include "Logging.h"
#include "CriticalSection.h"
#include "InstanceStates.h"
#include "TradingWeekBoundaries.h"
#include "CTesterParameters.h"
#include "MQLDefines.h"

typedef struct oldTickVolume_t
{
  int    instanceId;
  time_t oldTime[MAX_RATES_BUFFERS];
  double oldVolume[MAX_RATES_BUFFERS];
} OldTickVolume;

static AsirikuyReturnCode getOldTickVolume(int instanceId, OldTickVolume** ppOldTickVolume)
{
  static OldTickVolume oldTickVolume[MAX_INSTANCES];
  static BOOL oldTickVolumeInitialized = FALSE;

  AsirikuyReturnCode returnCode = SUCCESS;
  int i, j, instanceIndex = -1;

  enterCriticalSection();

  if(!oldTickVolumeInitialized)
  {
    for(i = 0; i < MAX_INSTANCES; i++)
    {
      oldTickVolume[i].instanceId = -1;

      for(j = 0; j < MAX_RATES_BUFFERS; j++)
      {
        oldTickVolume[i].oldTime[j]   = -1;
        oldTickVolume[i].oldVolume[j] = -1;
      }
    }

    oldTickVolumeInitialized = TRUE;
    logNotice("getOldTickVolume() Initialized old tick volume.\n");
  }

  for(i = 0; i < MAX_INSTANCES; i++)
  {
    if((oldTickVolume[i].instanceId == -1) || (oldTickVolume[i].instanceId == instanceId))
    {
      oldTickVolume[i].instanceId = instanceId;
      instanceIndex = i;
      break;
    }
  }

  leaveCriticalSection();
  
  if(instanceIndex >= 0)
  {
    *ppOldTickVolume = &oldTickVolume[instanceIndex];
  }
  else
  {
    *ppOldTickVolume = NULL;
    logCritical("getOldTickVolume() Failed to find oldTickVolume for instance Id: %d\n\n\n", instanceId);
    returnCode = TOO_MANY_INSTANCES;
  }
  
  return returnCode;
}

static AsirikuyReturnCode copyBarC(const CRates* pSource, Rates* pDest, int destIndex, TZOffsets* tzOffsets)
{
  if(pSource == NULL)
  {
    logCritical("copyBar() failed. pSource = NULL\n");
    return NULL_POINTER;
  }

  if(pDest == NULL)
  {
    logCritical("copyBar() failed. pDest = NULL\n");
    return NULL_POINTER;
  }

  if(pDest->time == NULL)
  {
    logCritical("copyBar() failed. pDest->time = NULL\n");
    return NULL_POINTER;
  }

  if(pDest->time)
  {
    pDest->time[destIndex] = getAdjustedBrokerTime(pSource->time, tzOffsets);
    // Debug logging - uncomment if needed
    // if(1)
    {
      char timeString[MAX_TIME_STRING_SIZE];
      // logDebug(""copyBar() time[%d] = %s\n\n", destIndex, safe_timeString(timeString, pDest->time[destIndex]));
    }
  }

  if(pDest->open)
  {
    pDest->open[destIndex] = pSource->open;
    // logDebug(""copyBar() open[%d] = %f\n\n", destIndex, pDest->open[destIndex]);
  }

  if(pDest->high)
  {
    pDest->high[destIndex] = pSource->high;
    // logDebug(""copyBar() high[%d] = %f\n\n", destIndex, pDest->high[destIndex]);
  }

  if(pDest->low)
  {
    pDest->low[destIndex] = pSource->low;
    // logDebug(""copyBar() low[%d] = %f\n\n", destIndex, pDest->low[destIndex]);
  }

  if(pDest->close)
  {
    pDest->close[destIndex] = pSource->close;
    // logDebug(""copyBar() close[%d] = %f\n\n", destIndex, pDest->close[destIndex]);
  }

  if(pDest->volume)
  {
    pDest->volume[destIndex] = pSource->volume;
    // logDebug(""copyBar() volume[%d] = %f\n\n", destIndex, pDest->volume[destIndex]);
  }

  return SUCCESS;
}

static AsirikuyReturnCode mergeBar(int instanceId, int ratesIndex, const CRates* pSource, Rates* pDest, int destIndex, TZOffsets* tzOffsets)
{
  time_t CTime, destTime;
  OldTickVolume* pOldTickVolume;
  AsirikuyReturnCode returnCode = getOldTickVolume(instanceId, &pOldTickVolume);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("mergeBar()\n\n", returnCode);
    return returnCode;
  }

  if(pSource == NULL)
  {
    logCritical("mergeBar() failed. pSource = NULL");
    return NULL_POINTER;
  }

  if(pDest == NULL)
  {
    logCritical("mergeBar() failed. pDest = NULL");
    return NULL_POINTER;
  }

  if(pDest->time == NULL)
  {
    logCritical("mergeBar() failed. pDest->time = NULL");
    return NULL_POINTER;
  }

  CTime  = getAdjustedBrokerTime(pSource->time, tzOffsets);
  destTime = pDest->time[destIndex];

  if(pDest->time)
  {
    if(CTime < destTime)
    {
      pDest->time[destIndex] = CTime;
    }

    // Debug logging - uncomment if needed
    // if(1)
    {
      char timeString[MAX_TIME_STRING_SIZE];
      // logDebug(""mergeBar() time[%d] = %s\n\n", destIndex, safe_timeString(timeString, pDest->time[destIndex]));
    }
  }

  if(pDest->open)
  {
    if(CTime < destTime)
    {
      pDest->open[destIndex] = pSource->open;
    }
    // logDebug(""mergeBar() open[%d] = %lf\n\n", destIndex, pDest->open[destIndex]);
  }

  if(pDest->high)
  {
    if(pSource->high > pDest->high[destIndex])
    {
      pDest->high[destIndex] = pSource->high;
    }
    // logDebug(""mergeBar() high[%d] = %lf\n\n", destIndex, pDest->high[destIndex]);
  }

  if(pDest->low)
  {
    if(((pSource->low < pDest->low[destIndex]) && (pSource->low > 0)) || (pDest->low[destIndex] <= 0))
    {
      pDest->low[destIndex] = pSource->low;
    }
    // logDebug(""mergeBar() low[%d] = %lf\n\n", destIndex, pDest->low[destIndex]);
  }

  if(pDest->close)
  {
    if(CTime > destTime)
    {
      pDest->close[destIndex] = pSource->close;
    }
    // logDebug(""mergeBar() close[%d] = %lf\n\n", destIndex, pDest->close[destIndex]);
  }

  if(pDest->volume)
  {
    if(pOldTickVolume->oldTime[ratesIndex] == -1)
    {
      pDest->volume[destIndex] = pSource->volume;
    }
    else
    {
      pDest->volume[destIndex] += pSource->volume;
      if(CTime == pOldTickVolume->oldTime[ratesIndex])
      {
        pDest->volume[destIndex] -= pOldTickVolume->oldVolume[ratesIndex];
      }
    }
    pOldTickVolume->oldTime[ratesIndex]   = CTime;
    pOldTickVolume->oldVolume[ratesIndex] = pSource->volume;
    // logDebug(""mergeBar() volume[%d] = %lf\n\n", destIndex, pDest->volume[destIndex]);
  }

  return SUCCESS;
}

static AsirikuyReturnCode reprocessConvertedBar(StrategyParams* pParams,int instanceId, CRates* pCRates, int ratesIndex, int ratesBufferIndex, Rates* pConvertedRates, int convertedRatesIndex, TZOffsets* tzOffsets)
{
  const int TIME_FRAME_IN_SECONDS = SECONDS_PER_MINUTE * pConvertedRates->info.timeframe;
  AsirikuyReturnCode returnCode;
  int i;
  time_t epochOffset = 0;

  if(pConvertedRates->info.timeframe == MINUTES_PER_WEEK)
  {
    /* Offset the epoch to the beginning of the week */
    epochOffset = EPOCH_WEEK_OFFSET;
  }
  
  // logDebug("reprocessConvertedBar() ratesIndex = %d, convertedRatesIndex = %d\n", ratesIndex, convertedRatesIndex);

  returnCode = copyBarC(&pCRates[ratesBufferIndex], pConvertedRates, convertedRatesIndex, tzOffsets);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("reprocessConvertedBar()\n\n", returnCode);
    return returnCode;
  }

  for(i = ratesBufferIndex - 1; i > 0; i--)
  {
    time_t CTime = getAdjustedBrokerTime(pCRates[i].time, tzOffsets);

    if(((CTime + epochOffset) / TIME_FRAME_IN_SECONDS) != ((pConvertedRates->time[convertedRatesIndex] + epochOffset) / TIME_FRAME_IN_SECONDS))
    {
      break;
    }

	if (!isValidTradingTime(pParams,CTime))
    {
      continue;
    }

    returnCode = mergeBar(instanceId, ratesIndex, &pCRates[i], pConvertedRates, convertedRatesIndex, tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("reprocessConvertedBar()\n\n", returnCode);
      return returnCode;
    }
  }

  return SUCCESS;
}

static AsirikuyReturnCode convertCurrentCBar(StrategyParams* pParams, TZOffsets* tzOffsets, CRatesInfo* pCRatesInfo, CRates* pCRates, int ratesIndex)
{
  const int TIME_FRAME_IN_SECONDS = SECONDS_PER_MINUTE * pParams->ratesBuffers->rates[ratesIndex].info.timeframe;

  AsirikuyReturnCode returnCode;
  char   timeString[MAX_TIME_STRING_SIZE];
  int    CShift0Index = (int)pCRatesInfo->ratesArraySize - 1;
  int    CShift1Index = (int)pCRatesInfo->ratesArraySize - 2;
  int    convertedShift0Index = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  time_t epochOffset   = 0;
  time_t CTime0      = getAdjustedBrokerTime(pCRates[CShift0Index].time, tzOffsets);
  time_t CTime1      = getAdjustedBrokerTime(pCRates[CShift1Index].time, tzOffsets);

   if (CTime0 < 0 || CTime1 < 0)
  {
	   // logDebug("Discarding candle with invalid timestamp\n");
	  return SUCCESS;
  }

  if(pParams->ratesBuffers->rates[ratesIndex].info.timeframe == MINUTES_PER_WEEK)
  {
    /* Offset the epoch to the beginning of the week */
    epochOffset += EPOCH_WEEK_OFFSET;
  }

  if (!isValidTradingTime(pParams,CTime0))
  {
    // logDebug(""convertCurrentCBar() Discarding unusable bar. Bar time = %s\n\n", safe_timeString(timeString, CTime0));
    return SUCCESS;
  }

  while (!isValidTradingTime(pParams,CTime1) && (CShift1Index > 0))
  {
    CTime1 = getAdjustedBrokerTime(pCRates[--CShift1Index].time, tzOffsets);
  }

  if(  ((CTime0 + epochOffset) / TIME_FRAME_IN_SECONDS) > ((CTime1 + epochOffset) / TIME_FRAME_IN_SECONDS)
    && (CTime0 != pParams->ratesBuffers->rates[ratesIndex].time[convertedShift0Index]))
  {
    incrementRatesOffset((int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex);

    returnCode = copyBarC(&pCRates[CShift0Index], &pParams->ratesBuffers->rates[ratesIndex], convertedShift0Index, tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("convertCurrentCBar()\n\n", returnCode);
      return returnCode;
    }

	returnCode = reprocessConvertedBar(pParams,(int)pParams->settings[STRATEGY_INSTANCE_ID], pCRates, ratesIndex, CShift1Index, &pParams->ratesBuffers->rates[ratesIndex], (convertedShift0Index - 1), tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("convertCurrentCBar()\n\n", returnCode);
      return returnCode;
    }
  }
  else
  {
    returnCode = mergeBar((int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex, &pCRates[CShift0Index], &pParams->ratesBuffers->rates[ratesIndex], convertedShift0Index, tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("convertCurrentCBar()\n\n", returnCode);
      return returnCode;
    }
  }

  return SUCCESS;
}

static AsirikuyReturnCode fillEmptyRatesBuffer(StrategyParams* pParams, TZOffsets* tzOffsets, CRatesInfo* pCRatesInfo, CRates* pCRates, int ratesIndex)
{
  const int TIME_FRAME_IN_SECONDS = SECONDS_PER_MINUTE * pParams->ratesBuffers->rates[ratesIndex].info.timeframe;

  AsirikuyReturnCode returnCode;
  char   timeString[MAX_TIME_STRING_SIZE] = "";
  time_t CTime;
  time_t epochOffset            = 0;
  int convertedRatesBufferIndex = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  int CRatesBufferIndex       = (int)pCRatesInfo->ratesArraySize - 1;

  if(pParams->ratesBuffers->rates[ratesIndex].info.timeframe == MINUTES_PER_WEEK)
  {
    /* Offset the epoch to the beginning of the week */
    epochOffset = EPOCH_WEEK_OFFSET;
  }

  for(; (CRatesBufferIndex >= 0) && (convertedRatesBufferIndex >= 0); convertedRatesBufferIndex--)
  {
    CTime = getAdjustedBrokerTime(pCRates[CRatesBufferIndex].time, tzOffsets);

	while (!isValidTradingTime(pParams,CTime))
    {
      if(--CRatesBufferIndex < 0)
      {
        pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
        return SUCCESS;
      }

	  CTime = getAdjustedBrokerTime(pCRates[CRatesBufferIndex].time, tzOffsets);

      // logDebug(""fillEmptyRatesBuffer() Discarding unusuable bar. Bar time = %s\n\n", safe_timeString(timeString, CTime));
    }

	returnCode = copyBarC(&pCRates[CRatesBufferIndex], &pParams->ratesBuffers->rates[ratesIndex], convertedRatesBufferIndex, tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("fillEmptyRatesBuffer()\n\n", returnCode);
      return returnCode;
    }

    if(--CRatesBufferIndex < 0)
    {
      pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
      return SUCCESS;
    }

    CTime = getAdjustedBrokerTime(pCRates[CRatesBufferIndex].time, tzOffsets);

	while (!isValidTradingTime(pParams,CTime))
    {
      if(--CRatesBufferIndex < 0)
      {
        pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
        return SUCCESS;
      }

	  CTime = getAdjustedBrokerTime(pCRates[CRatesBufferIndex].time, tzOffsets);

      // logDebug(""fillEmptyRatesBuffer() Discarding unusable bar. Bar time = %s\n\n", safe_timeString(timeString, CTime));
    }

    while((CRatesBufferIndex >= 0) && (((CTime + epochOffset) / TIME_FRAME_IN_SECONDS) == ((pParams->ratesBuffers->rates[ratesIndex].time[convertedRatesBufferIndex] + epochOffset) / TIME_FRAME_IN_SECONDS)))
    {
      returnCode = mergeBar((int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex, &pCRates[CRatesBufferIndex], &pParams->ratesBuffers->rates[ratesIndex], convertedRatesBufferIndex, tzOffsets);
      if(returnCode != SUCCESS)
      {
        logAsirikuyError("fillEmptyRatesBuffer()\n\n", returnCode);
        return returnCode;
      }
      
      if(--CRatesBufferIndex < 0)
      {
        pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
        return SUCCESS;
      }

      CTime = getAdjustedBrokerTime(pCRates[CRatesBufferIndex].time, tzOffsets);

	  while (!isValidTradingTime(pParams,CTime))
      {
        if(--CRatesBufferIndex < 0)
        {
          pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
          return SUCCESS;
        }
        // logDebug(""fillEmptyRatesBuffer() Discarding unusable bar. Bar time = %s\n\n", safe_timeString(timeString, CTime));
		
		CTime = getAdjustedBrokerTime(pCRates[CRatesBufferIndex].time, tzOffsets);
	  }
    }
  }

  pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
  return SUCCESS;
}

AsirikuyReturnCode convertRatesArrayC(StrategyParams* pParams, TZOffsets* tzOffsets, CRatesInfo* pCRatesInfo, CRates* pCRates, int ratesIndex)
{
  if(pCRatesInfo->isEnabled && pParams->ratesBuffers->rates[ratesIndex].info.isEnabled)
  {
    if(!pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull)
    {
      // logDebug(""convertRatesArray() Filling empty rates buffer.");
      return fillEmptyRatesBuffer(pParams, tzOffsets, pCRatesInfo, pCRates, ratesIndex);
    }
    else
    {
      // logDebug(""convertRatesArray() Converting new C bar.");
      return convertCurrentCBar(pParams, tzOffsets, pCRatesInfo, pCRates, ratesIndex);
    }
  }

  return SUCCESS;
}

AsirikuyReturnCode convertRatesArraysC(
  StrategyParams* pParams, 
  TZOffsets*      pTZOffsets,
  CRatesInfo*   pCRatesInfo,
  CRates*      pCRates_0,
  CRates*      pCRates_1,
  CRates*      pCRates_2,
  CRates*      pCRates_3,
  CRates*      pCRates_4,
  CRates*      pCRates_5,
  CRates*      pCRates_6,
  CRates*      pCRates_7,
  CRates*      pCRates_8,
  CRates*      pCRates_9)
{
  AsirikuyReturnCode result;
  RatesInfo ratesInfo[MAX_RATES_BUFFERS];
  int i;

  /* Copy the rates info */
  for(i = 0; i < MAX_RATES_BUFFERS; i++)
  {
    if(!pCRatesInfo[i].isEnabled)
    {
      ratesInfo[i].isEnabled    = FALSE;
      ratesInfo[i].isBufferFull = FALSE;
      ratesInfo[i].timeframe    = 0;
      ratesInfo[i].arraySize    = 0;
      ratesInfo[i].point        = 0;
	    ratesInfo[i].digits       = 0;
      continue;
    }

    if((int)pCRatesInfo[i].ratesArraySize < (int)(pCRatesInfo[i].totalBarsRequired * WEEKEND_BAR_MULTIPLIER * pCRatesInfo[i].requiredTimeframe / pCRatesInfo[i].actualTimeframe))
    {
      logError("convertRatesArrays() failed. Not enough rates data. C rates index = %d, array size = %d, required = %d\n\n", i, (int)pCRatesInfo[i].ratesArraySize, (int)(pCRatesInfo[i].totalBarsRequired * WEEKEND_BAR_MULTIPLIER * pCRatesInfo[i].requiredTimeframe / pCRatesInfo[i].actualTimeframe));
      return NOT_ENOUGH_RATES_DATA;
    }

    ratesInfo[i].isEnabled    = TRUE;
    ratesInfo[i].isBufferFull = FALSE;
    ratesInfo[i].timeframe    = (int)pCRatesInfo[i].requiredTimeframe;
    ratesInfo[i].arraySize    = (int)pCRatesInfo[i].totalBarsRequired;
    ratesInfo[i].point        = pCRatesInfo[i].point;
	  ratesInfo[i].digits       = (int)pCRatesInfo[i].digits;
  }

  /* Allocate memory for all rates data */
  result = allocateRates(&pParams->ratesBuffers, (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesInfo);
  if(result != SUCCESS)
  {
    logAsirikuyError("convertRatesArrays()\n\n", result);
    return result;
  }

  if(pParams->ratesBuffers->rates[0].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[0], pCRates_0, 0);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index0 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[1].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[1], pCRates_1, 1);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index1 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[2].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[2], pCRates_2, 2);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index2 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[3].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[3], pCRates_3, 3);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index3 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[4].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[4], pCRates_4, 4);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index4 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[5].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[5], pCRates_5, 5);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index5 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[6].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[6], pCRates_6, 6);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index6 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[7].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[7], pCRates_7, 7);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index7 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[8].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[8], pCRates_8, 8);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index8 \n\n", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[9].info.isEnabled)
  {
    result = convertRatesArrayC(pParams, pTZOffsets, &pCRatesInfo[9], pCRates_9, 9);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index9 \n\n", result);
      return result;
    }
  }

  return SUCCESS;
}

/**
* Copy C bid and ask prices into a BidAsk structure.
*
* @param BidAsk* pBidAsk
*   The structure into which the C bid and ask prices are
*   to be copied.
*
* @param double* CBidAsk
*   An array of 6 C bid and ask prices to be copied.
*   Indexes are defined by enum BidAskIndex.
*
*/
void copyBidAskC(BidAsk* pBidAsk, double* pCBidAsk)
{
  pBidAsk->arraySize          = 1; /* There are no historic bid & ask feeds in MetaTrader4 so we only need 1 array index each for the current bid & ask. */
  pBidAsk->bid                = &pCBidAsk[IDX_BID];
  pBidAsk->ask                = &pCBidAsk[IDX_ASK];
  pBidAsk->baseConversionBid  = pCBidAsk[IDX_BASE_CONVERSION_BID];
  pBidAsk->baseConversionAsk  = pCBidAsk[IDX_BASE_CONVERSION_ASK];
  pBidAsk->quoteConversionBid = pCBidAsk[IDX_QUOTE_CONVERSION_BID];
  pBidAsk->quoteConversionAsk = pCBidAsk[IDX_QUOTE_CONVERSION_ASK];
}

/**
* Copy C account information into an AccountInfo structure.
*
* @param AccountInfo* pInfo
*   The structure into which the C account information is
*   to be copied.
*
* @param char* accountCurrency
*   A string containing the 3 character C account currency code.
*   It is copied into the AccountInfo structure.
*
* @param double* CAccountInfo
*   An array containing C account information to be copied.
*   Indexes are defined by enum AccountInfoIndex.
*
*/
void copyAccountInfoC(AccountInfo* pInfo, char* pAccountCurrency, char* pBrokerName, char* pReferenceName, double* pCAccountInfo)
{
  // Debug: Log what we're receiving
  logDebug("copyAccountInfoC() pBrokerName = '%s' (length=%zu)", pBrokerName ? pBrokerName : "(NULL)", pBrokerName ? strlen(pBrokerName) : 0);
  logDebug("copyAccountInfoC() pReferenceName = '%s' (length=%zu)", pReferenceName ? pReferenceName : "(NULL)", pReferenceName ? strlen(pReferenceName) : 0);
  
  pInfo->brokerName                = pBrokerName;
  pInfo->referenceName             = pReferenceName;
  pInfo->currency                  = pAccountCurrency;
  pInfo->accountNumber             = (int)pCAccountInfo[IDX_ACCOUNT_NUMBER];
  pInfo->balance                   = pCAccountInfo[IDX_BALANCE];
  pInfo->equity                    = pCAccountInfo[IDX_EQUITY];
  pInfo->margin                    = pCAccountInfo[IDX_MARGIN];
  pInfo->leverage                  = pCAccountInfo[IDX_LEVERAGE];
  pInfo->contractSize              = pCAccountInfo[IDX_CONTRACT_SIZE];
  pInfo->minimumStop               = pCAccountInfo[IDX_MINIMUM_STOP];
  pInfo->stopoutPercent            = pCAccountInfo[IDX_STOPOUT_PERCENT];
  pInfo->totalOpenTradeRiskPercent = pCAccountInfo[IDX_TOTAL_OPEN_TRADE_RISK_PERCENT];
  pInfo->largestDrawdownPercent    = pCAccountInfo[IDX_LARGEST_DRAWDOWN_PERCENT];
}

/**
* Copy C order information into an OrderInfo structure.
*
* @param OrderInfo* pOrderInfo
*   The array of structures into which the C order
*   information is to be copied.
*
* @param double* pCOrderInfo
*   The C order information arrays to be copied.
*
* @param int orderInfoArraySize
*   The size of the first dimension of pCOrderInfo.
*
*/
void copyOrderInfoC(OrderInfo* pOrderInfo, COrderInfo* pCOrderInfo, int orderInfoArraySize)
{
  int i;
  for(i = 0; i < orderInfoArraySize; i++)
  {
    pOrderInfo[i].ticket      =       (int)pCOrderInfo[i].ticket;
    pOrderInfo[i].instanceId  =       (int)pCOrderInfo[i].instanceId;
    pOrderInfo[i].type        = (OrderType)pCOrderInfo[i].type;
    pOrderInfo[i].openTime    =    (time_t)pCOrderInfo[i].openTime;
    pOrderInfo[i].closeTime   =    (time_t)pCOrderInfo[i].closeTime;
    pOrderInfo[i].stopLoss    =     (float)pCOrderInfo[i].stopLoss;
    pOrderInfo[i].takeProfit  =     (float)pCOrderInfo[i].takeProfit;
    pOrderInfo[i].expiriation =    (time_t)pCOrderInfo[i].expiriation;
    pOrderInfo[i].openPrice   =     (float)pCOrderInfo[i].openPrice;
    pOrderInfo[i].closePrice  =     (float)pCOrderInfo[i].closePrice;
    pOrderInfo[i].lots        =     (float)pCOrderInfo[i].lots;
    pOrderInfo[i].profit      =     (float)pCOrderInfo[i].profit;
    pOrderInfo[i].commission  =     (float)pCOrderInfo[i].commission;
    pOrderInfo[i].swap        =     (float)pCOrderInfo[i].swap;

    pOrderInfo[i].isOpen = TRUE;
    if(!pCOrderInfo[i].isOpen)
    {
      pOrderInfo[i].isOpen = FALSE;
    }
  }
}

/**
* Copy C parameters into a StrategyParams structure
*
* @param StrategyParams* params
*   The structure into which all the C parameters are copied.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode convertCParameters(
  double*          pCSettings,
  char*      pCTradeSymbol,
  char*      pCAccountCurrency,
  char*      pCBrokerName,
  char*      pCRefBrokerName,
  int*             pCCurrentBrokerTime,
  int*             pCOpenOrdersCount,
  COrderInfo*    pCOrderInfo,
  double*          pCAccountInfo,
  double*          pCBidAsk,
  CRatesInfo*    pCRatesInfo,
  CRates*       pCRates_0,
  CRates*       pCRates_1,
  CRates*       pCRates_2,
  CRates*       pCRates_3,
  CRates*       pCRates_4,
  CRates*       pCRates_5,
  CRates*       pCRates_6,
  CRates*       pCRates_7,
  CRates*       pCRates_8,
  CRates*       pCRates_9,
  StrategyResults* pCResults,
  StrategyParams*  pParams)
{
  AsirikuyReturnCode returnCode;
  TZOffsets tzOffsets;

  if(pCSettings == NULL)
  {
    logCritical("convertCParameters() failed. pCSettings = NULL");
    return NULL_POINTER;
  }

  if(pCTradeSymbol == NULL)
  {
    logCritical("convertCParameters() failed. pCTradeSymbol = NULL");
    return NULL_POINTER;
  }

  if(pCAccountCurrency == NULL)
  {
    logCritical("convertCParameters() failed. pCAccountCurrency = NULL");
    return NULL_POINTER;
  }

  if(pCBrokerName == NULL)
  {
    logCritical("convertCParameters() failed. pCBrokerName = NULL");
    return NULL_POINTER;
  }

  if(pCRefBrokerName == NULL)
  {
    logCritical("convertCParameters() failed. pCRefBrokerName = NULL");
    return NULL_POINTER;
  }

  if(pCCurrentBrokerTime == NULL)
  {
    logCritical("convertCParameters() failed. pCCurrentBrokerTime = NULL");
    return NULL_POINTER;
  }

  if(pCOpenOrdersCount == NULL)
  {
    logCritical("convertCParameters() failed. pCOpenOrdersCount = NULL");
    return NULL_POINTER;
  }

  if(pCOrderInfo == NULL)
  {
    logCritical("convertCParameters() failed. pCOrderInfo = NULL");
    return NULL_POINTER;
  }

  if(pCAccountInfo == NULL)
  {
    logCritical("convertCParameters() failed. pCAccountInfo = NULL");
    return NULL_POINTER;
  }

  if(pCBidAsk == NULL)
  {
    logCritical("convertCParameters() failed. pCBidAsk = NULL");
    return NULL_POINTER;
  }

  if(pCRatesInfo == NULL)
  {
    logCritical("convertCParameters() failed. pCRatesInfo = NULL");
    return NULL_POINTER;
  }

  if(pCRates_0 == NULL)
  {
    logCritical("convertCParameters() failed. pCRates_0 = NULL");
    return NULL_POINTER;
  }

  if(pCResults == NULL)
  {
    logCritical("convertCParameters() failed. pCResults = NULL");
    return NULL_POINTER;
  }

  if(pParams == NULL)
  {
    logCritical("convertCParameters() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pCTradeSymbol == NULL)
  {
    logCritical("convertCParameters() failed. pCTradeSymbol->string = NULL");
    return NULL_POINTER;
  }

  if(pCAccountCurrency == NULL)
  {
    logCritical("convertCParameters() failed. pCAccountCurrency->string = NULL");
    return NULL_POINTER;
  }

  if(pCBrokerName == NULL)
  {
    logCritical("convertCParameters() failed. pCBrokerName->string = NULL");
    return NULL_POINTER;
  }

  if(pCRefBrokerName == NULL)
  {
    logCritical("convertCParameters() failed. pCRefBrokerName->string = NULL");
    return NULL_POINTER;
  }

  pParams->tradeSymbol         = pCTradeSymbol;
  pParams->settings            = pCSettings;
  pParams->results             = pCResults;
  pParams->expertParameterInfo = getInstanceState((int)pParams->settings[STRATEGY_INSTANCE_ID])->parameterSpace;

  copyBidAskC(&pParams->bidAsk, pCBidAsk);
  copyAccountInfoC(&pParams->accountInfo, pCAccountCurrency, pCBrokerName, pCRefBrokerName, pCAccountInfo);
  copyOrderInfoC(pParams->orderInfo, pCOrderInfo, (int)pParams->settings[ORDERINFO_ARRAY_SIZE]);
  
  /* Get the time offsets for each day of the year between the broker and reference broker. */
  returnCode = getTimeOffsets((time_t)*pCCurrentBrokerTime, &pParams->accountInfo, (BOOL)pParams->settings[IS_BACKTESTING], (int)pParams->settings[STRATEGY_INSTANCE_ID], &tzOffsets);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("convertCParameters()\n\n", returnCode);
    return returnCode;
  }
  
  pParams->currentBrokerTime = getAdjustedBrokerTime((time_t)*pCCurrentBrokerTime, &tzOffsets);
  
  return convertRatesArraysC(pParams, &tzOffsets, pCRatesInfo, pCRates_0, pCRates_1, pCRates_2, pCRates_3, pCRates_4, pCRates_5, pCRates_6, pCRates_7, pCRates_8, pCRates_9);
}

AsirikuyReturnCode allocateOrderInfoC(StrategyParams* pParams, int orderInfoArraySize)
{
  if(pParams == NULL)
  {
    logCritical("allocateOrderInfo() failed. pParams = NULL");
    return NULL_POINTER;
  }

  pParams->orderInfo = (OrderInfo*)malloc(orderInfoArraySize * sizeof(OrderInfo));
  
  return SUCCESS;
}

AsirikuyReturnCode freeOrderInfoC(StrategyParams* pParams)
{
  if(pParams == NULL)
  {
    logCritical("freeOrderInfo() failed. pParams = NULL");
    return NULL_POINTER;
  }

  if(pParams->orderInfo)
  {
    free(pParams->orderInfo);
    pParams->orderInfo = NULL;
  }

  return SUCCESS;
}