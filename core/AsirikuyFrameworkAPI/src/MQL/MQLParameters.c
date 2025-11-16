/**
 * @file
 * @brief     Functions for converting and copying MQL parameters.
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
#include "AsirikuyLogger.h"
#include "MQLParameters.h"
#include "ContiguousRatesCircBuf.h"
#include "TimeZoneOffsets.h"
#include "AsirikuyTime.h"
#include "Logging.h"
#include "CriticalSection.h"
#include "InstanceStates.h"
#include "TradingWeekBoundaries.h"

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
    logCritical("getOldTickVolume() Failed to find oldTickVolume for instance Id: %d\n", instanceId);
    returnCode = TOO_MANY_INSTANCES;
  }
  
  return returnCode;
}

static AsirikuyReturnCode copyBar(MQLVersion mqlVersion, const void* pSource, int sourceIndex, Rates* pDest, int destIndex, TZOffsets* tzOffsets, int ratesIndex)
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
    if(mqlVersion == MQL4)
    {
      pDest->time[destIndex] = getAdjustedBrokerTime(((Mql4Rates*)pSource)[sourceIndex].time, tzOffsets);
    }
    else
    {
      pDest->time[destIndex] = getAdjustedBrokerTime(((Mql5Rates*)pSource)[sourceIndex].time, tzOffsets);
    }
    // Debug logging - uncomment if needed
    // {
    //   char timeString[MAX_TIME_STRING_SIZE];
    //   logDebug("copyBar() time[%d] = %s,ratesIndex=%d\n", destIndex, safe_timeString(timeString, pDest->time[destIndex]), ratesIndex);
    // }
  }

  if(pDest->open)
  {
    if(mqlVersion == MQL4)
    {
      pDest->open[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].open;
    }
    else
    {
      pDest->open[destIndex] = ((Mql5Rates*)pSource)[sourceIndex].open;
    }
	// logDebug("copyBar() open[%d] = %f,ratesIndex=%d", destIndex, pDest->open[destIndex], ratesIndex);
  }

  if(pDest->high)
  {
    if(mqlVersion == MQL4)
    {
      pDest->high[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].high;
    }
    else
    {
      pDest->high[destIndex] = ((Mql5Rates*)pSource)[sourceIndex].high;
    }
	// logDebug("copyBar() high[%d] = %f,ratesIndex=%d", destIndex, pDest->high[destIndex], ratesIndex);
  }

  if(pDest->low)
  {
    if(mqlVersion == MQL4)
    {
      pDest->low[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].low;
    }
    else
    {
      pDest->low[destIndex] = ((Mql5Rates*)pSource)[sourceIndex].low;
    }
	// logDebug("copyBar() low[%d] = %f,ratesIndex=%d", destIndex, pDest->low[destIndex], ratesIndex);
  }

  if(pDest->close)
  {
    if(mqlVersion == MQL4)
    {
      pDest->close[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].close;
    }
    else
    {
      pDest->close[destIndex] = ((Mql5Rates*)pSource)[sourceIndex].close;
    }
	// logDebug("copyBar() close[%d] = %f,ratesIndex=%d", destIndex, pDest->close[destIndex], ratesIndex);
  }

  if(pDest->volume)
  {
    if(mqlVersion == MQL4)
    {
      pDest->volume[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].volume;
    }
    else
    {
      pDest->volume[destIndex] = (double)((Mql5Rates*)pSource)[sourceIndex].tick_volume;
    }
	// logDebug("copyBar() volume[%d] = %f,ratesIndex=%d", destIndex, pDest->volume[destIndex], ratesIndex);
  }

  return SUCCESS;
}

static AsirikuyReturnCode mergeBar(MQLVersion mqlVersion, int instanceId, int ratesIndex, const void* pSource, int sourceIndex, Rates* pDest, int destIndex, TZOffsets* tzOffsets)
{
  time_t mqlTime, destTime;
  OldTickVolume* pOldTickVolume;
  AsirikuyReturnCode returnCode = getOldTickVolume(instanceId, &pOldTickVolume);
  // Pantheios removed - using standard logging
  // pan_sev_t level = PANTHEIOS_SEV_DEBUG;

  //if (ratesIndex == 1) // Normally rateindex =1 means daily rate for BBS
	 // level = PANTHEIOS_SEV_INFORMATIONAL;

  if(returnCode != SUCCESS)
  {
    logAsirikuyError("mergeBar()", returnCode);
    return returnCode;
  }

  if(pSource == NULL)
  {
    logCritical("mergeBar() failed. pSource = NULL\n");
    return NULL_POINTER;
  }

  if(pDest == NULL)
  {
    logCritical("mergeBar() failed. pDest = NULL\n");
    return NULL_POINTER;
  }

  if(pDest->time == NULL)
  {
    logCritical("mergeBar() failed. pDest->time = NULL\n");
    return NULL_POINTER;
  }

  if(mqlVersion == MQL4)
  {
    mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pSource)[sourceIndex].time, tzOffsets);
  }
  else
  {
    mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pSource)[sourceIndex].time, tzOffsets);
  }
  destTime = pDest->time[destIndex];

  if(pDest->time)
  {
    if(mqlTime < destTime)
    {
      pDest->time[destIndex] = mqlTime;
    }

    // Debug logging - uncomment if needed
    // {
    //   char timeString[MAX_TIME_STRING_SIZE];
    //   logDebug("mergeBar() time[%d] = %s,ratesIndex=%d\n", destIndex, safe_timeString(timeString, pDest->time[destIndex]), ratesIndex);
    // }
  }

  if(pDest->open)
  {
    if(mqlTime < destTime)
    {
      if(mqlVersion == MQL4)
      {
        pDest->open[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].open;
      }
      else
      {
        pDest->open[destIndex] = ((Mql5Rates*)pSource)[sourceIndex].open;
      }
    }
	// logDebug("mergeBar() open[%d] = %lf,ratesIndex=%d", destIndex, pDest->open[destIndex], ratesIndex);
  }

  if(pDest->high)
  {
    double sourceHigh;

    if(mqlVersion == MQL4)
    {
      sourceHigh = ((Mql4Rates*)pSource)[sourceIndex].high;
    }
    else
    {
      sourceHigh = ((Mql5Rates*)pSource)[sourceIndex].high;
    }

    if(sourceHigh > pDest->high[destIndex])
    {
      pDest->high[destIndex] = sourceHigh;
    }
	// logDebug("mergeBar() high[%d] = %lf,ratesIndex=%d", destIndex, pDest->high[destIndex], ratesIndex);
  }

  if(pDest->low)
  {
    double sourceLow;

    if(mqlVersion == MQL4)
    {
      sourceLow = ((Mql4Rates*)pSource)[sourceIndex].low;
    }
    else
    {
      sourceLow = ((Mql5Rates*)pSource)[sourceIndex].low;
    }

    if(((sourceLow < pDest->low[destIndex]) && (sourceLow > 0)) || (pDest->low[destIndex] <= 0))
    {
      pDest->low[destIndex] = sourceLow;
    }
	// logDebug("mergeBar() low[%d] = %lf,ratesIndex=%d", destIndex, pDest->low[destIndex], ratesIndex);
  }

  if(pDest->close)
  {
    if(mqlTime > destTime)
    {
      if(mqlVersion == MQL4)
      {
        pDest->close[destIndex] = ((Mql4Rates*)pSource)[sourceIndex].close;
      }
      else
      {
        pDest->close[destIndex] = ((Mql5Rates*)pSource)[sourceIndex].close;
      }
    }
	// logDebug("mergeBar() close[%d] = %lf,ratesIndex=%d", destIndex, pDest->close[destIndex], ratesIndex);
  }

  if(pDest->volume)
  {
    double sourceVolume;

    if(mqlVersion == MQL4)
    {
      sourceVolume = ((Mql4Rates*)pSource)[sourceIndex].volume;
    }
    else
    {
      sourceVolume = (double)((Mql5Rates*)pSource)[sourceIndex].tick_volume;
    }

    if(pOldTickVolume->oldTime[ratesIndex] == -1)
    {
      pDest->volume[destIndex] = sourceVolume;
    }
    else
    {
      pDest->volume[destIndex] += sourceVolume;
      if(mqlTime == pOldTickVolume->oldTime[ratesIndex])
      {
        pDest->volume[destIndex] -= pOldTickVolume->oldVolume[ratesIndex];
      }
    }
    pOldTickVolume->oldTime[ratesIndex]   = mqlTime;
    pOldTickVolume->oldVolume[ratesIndex] = sourceVolume;
	// logDebug("mergeBar() volume[%d] = %lf,ratesIndex=%d", destIndex, pDest->volume[destIndex], ratesIndex);
  }

  return SUCCESS;
}

static AsirikuyReturnCode reprocessConvertedBar(StrategyParams* pParams,MQLVersion mqlVersion, int instanceId, void* pMqlRates, int ratesIndex, int ratesBufferIndex, Rates* pConvertedRates, int convertedRatesIndex, TZOffsets* tzOffsets)
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
  
  // logDebug(""reprocessConvertedBar() ratesIndex = %d, convertedRatesIndex = %d", ratesIndex, convertedRatesIndex);

  returnCode = copyBar(mqlVersion, pMqlRates, ratesBufferIndex, pConvertedRates, convertedRatesIndex, tzOffsets, ratesIndex);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("reprocessConvertedBar()", returnCode);
    return returnCode;
  }

  for(i = ratesBufferIndex - 1; i > 0; i--)
  {
    time_t mqlTime;

    if(mqlVersion == MQL4)
    {
      mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[i].time, tzOffsets);
    }
    else
    {
      mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[i].time, tzOffsets);
    }

    if(((mqlTime + epochOffset) / TIME_FRAME_IN_SECONDS) != ((pConvertedRates->time[convertedRatesIndex] + epochOffset) / TIME_FRAME_IN_SECONDS))
    {
      break;
    }

	if (!isValidTradingTime(pParams,mqlTime))
    {
      continue;
    }

	returnCode = mergeBar(mqlVersion, instanceId, ratesIndex, pMqlRates, i, pConvertedRates, convertedRatesIndex, tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("reprocessConvertedBar()", returnCode);
      return returnCode;
    }
  }

  return SUCCESS;
}

static AsirikuyReturnCode convertCurrentMqlBar(MQLVersion mqlVersion, StrategyParams* pParams, TZOffsets* tzOffsets, MqlRatesInfo* pMqlRatesInfo, void* pMqlRates, int ratesIndex)
{
  const int TIME_FRAME_IN_SECONDS = SECONDS_PER_MINUTE * pParams->ratesBuffers->rates[ratesIndex].info.timeframe;

  AsirikuyReturnCode returnCode;
  char   timeString[MAX_TIME_STRING_SIZE];
  char   timeString2[MAX_TIME_STRING_SIZE], timeString3[MAX_TIME_STRING_SIZE];
  int    mqlShift0Index = (int)pMqlRatesInfo->ratesArraySize - 1;
  int    mqlShift1Index = (int)pMqlRatesInfo->ratesArraySize - 2;
  int    convertedShift0Index = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  time_t epochOffset   = 0;
  time_t mqlTime0, mqlTime1;

  if(mqlVersion == MQL4)
  {
    mqlTime0 = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlShift0Index].time, tzOffsets);
    mqlTime1 = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlShift1Index].time, tzOffsets);
  }
  else
  {
    mqlTime0 = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlShift0Index].time, tzOffsets);
    mqlTime1 = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlShift1Index].time, tzOffsets);
  }

  if(mqlTime0 < 0 || mqlTime1 < 0)
  {
    logWarning("Discarding candle with invalid timestamp\n");
    return SUCCESS;
  }

  if(pParams->ratesBuffers->rates[ratesIndex].info.timeframe == MINUTES_PER_WEEK)
  {
    /* Offset the epoch to the beginning of the week */
    epochOffset += EPOCH_WEEK_OFFSET;
  }

  if (!isValidTradingTime(pParams,mqlTime0))
  {
	logWarning("convertCurrentMqlBar() Discarding unusable bar. Bar time = %s\n", safe_timeString(timeString, mqlTime0));
    return SUCCESS;
  }

  while (!isValidTradingTime(pParams,mqlTime1) && (mqlShift1Index > 0))
  {
    if(mqlVersion == MQL4)
    {
      mqlTime1 = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[--mqlShift1Index].time, tzOffsets);
    }
    else
    {
      mqlTime1 = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[--mqlShift1Index].time, tzOffsets);
    }
  }
   
  //���������ʧȥ����ʷ��hour bars,���Ͳ�������previous daily bar. 
  if(  ((mqlTime0 + epochOffset) / TIME_FRAME_IN_SECONDS) > ((mqlTime1 + epochOffset) / TIME_FRAME_IN_SECONDS)
    && (mqlTime0 != pParams->ratesBuffers->rates[ratesIndex].time[convertedShift0Index]))
  {  
	// logDebug("convertCurrentMqlBar() Testing .... strategyID= %d,ratesIndex=%d,mqlShift0Index=%d, mqlTime0=%s,mqlShift1Index =%d,mqlTime1=%s,convertedShift0Index=%d,converted bar time=%s", (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex, mqlShift0Index, safe_timeString(timeString, mqlTime0), mqlShift1Index, safe_timeString(timeString2, mqlTime1), convertedShift0Index, safe_timeString(timeString3, pParams->ratesBuffers->rates[ratesIndex].time[convertedShift0Index]));

	// ��̬������buffer
    incrementRatesOffset((int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex);

	// Add the latest one
	returnCode = copyBar(mqlVersion, pMqlRates, mqlShift0Index, &pParams->ratesBuffers->rates[ratesIndex], convertedShift0Index, tzOffsets,ratesIndex);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("convertCurrentMqlBar()", returnCode);
      return returnCode;
    }

	//reprocess the second latest bar
	returnCode = reprocessConvertedBar(pParams,mqlVersion, (int)pParams->settings[STRATEGY_INSTANCE_ID], pMqlRates, ratesIndex, mqlShift1Index, &pParams->ratesBuffers->rates[ratesIndex], (convertedShift0Index - 1), tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("convertCurrentMqlBar()", returnCode);
      return returnCode;
    }
  }
  else
  {
	 // pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"convertCurrentMqlBar()-> mergebar() Testing .... strategyID= %d,ratesIndex=%d,mqlShift0Index=%d, convertedShift0Index=%d", (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex, mqlShift0Index, convertedShift0Index);

	  returnCode = mergeBar(mqlVersion, (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex, pMqlRates, mqlShift0Index, &pParams->ratesBuffers->rates[ratesIndex], convertedShift0Index, tzOffsets);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("convertCurrentMqlBar()", returnCode);
      return returnCode;
    }
  }

  return SUCCESS;
}

static AsirikuyReturnCode fillEmptyRatesBuffer(MQLVersion mqlVersion, StrategyParams* pParams, TZOffsets* tzOffsets, MqlRatesInfo* pMqlRatesInfo, void* pMqlRates, int ratesIndex)
{
  const int TIME_FRAME_IN_SECONDS = SECONDS_PER_MINUTE * pParams->ratesBuffers->rates[ratesIndex].info.timeframe;

  AsirikuyReturnCode returnCode;
  char   timeString[MAX_TIME_STRING_SIZE] = "";
  char   timeString2[MAX_TIME_STRING_SIZE] = "";
  time_t mqlTime;
  time_t epochOffset            = 0;
  int convertedRatesBufferIndex = pParams->ratesBuffers->rates[ratesIndex].info.arraySize - 1;
  int mqlRatesBufferIndex       = (int)pMqlRatesInfo->ratesArraySize - 1;

  if(pParams->ratesBuffers->rates[ratesIndex].info.timeframe == MINUTES_PER_WEEK)
  {
    /* Offset the epoch to the beginning of the week */
    epochOffset = EPOCH_WEEK_OFFSET;
  }

  for(; (mqlRatesBufferIndex >= 0) && (convertedRatesBufferIndex >= 0); convertedRatesBufferIndex--)
  {
    if(mqlVersion == MQL4)
    {
      mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
    }
    else
    {
      mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
    }

	while (!isValidTradingTime(pParams,mqlTime))
    {
      if(--mqlRatesBufferIndex < 0)
      {
        pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
        return SUCCESS;
      }
	  logWarning("fillEmptyRatesBuffer() Discarding unusuable bar. Bar time = %s\n", safe_timeString(timeString, mqlTime));
    
	  if(mqlVersion == MQL4)
		{
			mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
		}
		else
		{
		mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
		}
	
	}
	
	// Copy current bar
	returnCode = copyBar(mqlVersion, pMqlRates, mqlRatesBufferIndex, &pParams->ratesBuffers->rates[ratesIndex], convertedRatesBufferIndex, tzOffsets, ratesIndex);
    if(returnCode != SUCCESS)
    {
      logAsirikuyError("fillEmptyRatesBuffer()", returnCode);
      return returnCode;
    }

	// Move to previous bar
    if(--mqlRatesBufferIndex < 0)
    {
      pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
      return SUCCESS;
    }

    if(mqlVersion == MQL4)
    {
      mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
    }
    else
    {
      mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
    }

	while (!isValidTradingTime(pParams,mqlTime))
    {
      if(--mqlRatesBufferIndex < 0)
      {
        pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
        return SUCCESS;
      }

    	logWarning("fillEmptyRatesBuffer() Discarding unusable bar. Bar time = %s\n", safe_timeString(timeString, mqlTime));
    
	  if(mqlVersion == MQL4)
		{
			mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
		}
		else
		{
			mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
		}
	}
	// logDebug("Testing.... strategyID= %d,ratesIndex=%d,  mqlRatesBufferIndex =%d, mqlTime=%s, converted bar time=%s", (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex,mqlRatesBufferIndex, safe_timeString(timeString, mqlTime), safe_timeString(timeString2, pParams->ratesBuffers->rates[ratesIndex].time[convertedRatesBufferIndex]));

    while((mqlRatesBufferIndex >= 0) && (((mqlTime + epochOffset) / TIME_FRAME_IN_SECONDS) == ((pParams->ratesBuffers->rates[ratesIndex].time[convertedRatesBufferIndex] + epochOffset) / TIME_FRAME_IN_SECONDS)))
    {
		// logDebug("fillEmptyRatesBuffer() mergebar.....strategyID =%d, Bar time = %s, ratesIndex=%d, mqlRatesBufferIndex=%d,convertedRatesBufferIndex=%d", (int)pParams->settings[STRATEGY_INSTANCE_ID], safe_timeString(timeString, mqlTime), ratesIndex, mqlRatesBufferIndex, convertedRatesBufferIndex);

      returnCode = mergeBar(mqlVersion, (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesIndex, pMqlRates, mqlRatesBufferIndex, &pParams->ratesBuffers->rates[ratesIndex], convertedRatesBufferIndex, tzOffsets);
      if(returnCode != SUCCESS)
      {
        logAsirikuyError("fillEmptyRatesBuffer()", returnCode);
        return returnCode;
      }
      
      if(--mqlRatesBufferIndex < 0)
      {
        pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
        return SUCCESS;
      }

      if(mqlVersion == MQL4)
      {
        mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
      }
      else
      {
        mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
      }

	  while (!isValidTradingTime(pParams,mqlTime))
      {
        if(--mqlRatesBufferIndex < 0)
        {
          pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
          return SUCCESS;
        }

		logWarning("fillEmptyRatesBuffer() Discarding unusable bar. Bar time = %s\n", safe_timeString(timeString, mqlTime));
      
		if(mqlVersion == MQL4)
		{
			mqlTime = getAdjustedBrokerTime(((Mql4Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
		}
		else
		{
			mqlTime = getAdjustedBrokerTime(((Mql5Rates*)pMqlRates)[mqlRatesBufferIndex].time, tzOffsets);
		}
	  }
    }
  }

  pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull = TRUE;
  return SUCCESS;
}

AsirikuyReturnCode convertRatesArray(MQLVersion mqlVersion, StrategyParams* pParams, TZOffsets* tzOffsets, MqlRatesInfo* pMqlRatesInfo, void* pMqlRates, int ratesIndex)
{
  if(pMqlRatesInfo->isEnabled && pParams->ratesBuffers->rates[ratesIndex].info.isEnabled)
  {
    if(!pParams->ratesBuffers->rates[ratesIndex].info.isBufferFull)
    {
      // logDebug(""convertRatesArray() Filling empty rates buffer.");
      return fillEmptyRatesBuffer(mqlVersion, pParams, tzOffsets, pMqlRatesInfo, pMqlRates, ratesIndex);
    }
    else
    {
      // logDebug(""convertRatesArray() Converting new MQL4 bar.");
      return convertCurrentMqlBar(mqlVersion, pParams, tzOffsets, pMqlRatesInfo, pMqlRates, ratesIndex);
    }
  }

  return SUCCESS;
}

AsirikuyReturnCode convertRatesArrays(
  MQLVersion      mqlVersion,
  StrategyParams* pParams, 
  TZOffsets*      pTZOffsets,
  MqlRatesInfo*   pMqlRatesInfo,
  void*           pMqlRates_0,
  void*           pMqlRates_1,
  void*           pMqlRates_2,
  void*           pMqlRates_3,
  void*           pMqlRates_4,
  void*           pMqlRates_5,
  void*           pMqlRates_6,
  void*           pMqlRates_7,
  void*           pMqlRates_8,
  void*           pMqlRates_9)
{
  AsirikuyReturnCode result;
  RatesInfo ratesInfo[MAX_RATES_BUFFERS];
  int i;

  /* Copy the rates info */
  for(i = 0; i < MAX_RATES_BUFFERS; i++)
  {
    if(!pMqlRatesInfo[i].isEnabled)
    {
      ratesInfo[i].isEnabled    = FALSE;
      ratesInfo[i].isBufferFull = FALSE;
      ratesInfo[i].timeframe    = 0;
      ratesInfo[i].arraySize    = 0;
      ratesInfo[i].point        = 0;
	    ratesInfo[i].digits       = 0;
      continue;
    }

    if((int)pMqlRatesInfo[i].ratesArraySize < (int)(pMqlRatesInfo[i].totalBarsRequired * WEEKEND_BAR_MULTIPLIER * pMqlRatesInfo[i].requiredTimeframe / pMqlRatesInfo[i].actualTimeframe))
    {
      logError("convertRatesArrays() failed. Not enough rates data. Mql rates index = %d, array size = %d, required = %d\n", i, (int)pMqlRatesInfo[i].ratesArraySize, (int)(pMqlRatesInfo[i].totalBarsRequired * WEEKEND_BAR_MULTIPLIER * pMqlRatesInfo[i].requiredTimeframe / pMqlRatesInfo[i].actualTimeframe));
      return NOT_ENOUGH_RATES_DATA;
    }

    ratesInfo[i].isEnabled    = TRUE;
    ratesInfo[i].isBufferFull = FALSE;
    ratesInfo[i].timeframe    = (int)pMqlRatesInfo[i].requiredTimeframe;
    ratesInfo[i].arraySize    = (int)pMqlRatesInfo[i].totalBarsRequired;
    ratesInfo[i].point        = pMqlRatesInfo[i].point;
	  ratesInfo[i].digits       = (int)pMqlRatesInfo[i].digits;
  }

  /* Allocate memory for all rates data */
  result = allocateRates(&pParams->ratesBuffers, (int)pParams->settings[STRATEGY_INSTANCE_ID], ratesInfo);
  if(result != SUCCESS)
  {
    logAsirikuyError("convertRatesArrays()", result);
    return result;
  }

  if(pParams->ratesBuffers->rates[0].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[0], pMqlRates_0, 0);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index0 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[1].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[1], pMqlRates_1, 1);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index1 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[2].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[2], pMqlRates_2, 2);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index2 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[3].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[3], pMqlRates_3, 3);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index3 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[4].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[4], pMqlRates_4, 4);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index4 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[5].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[5], pMqlRates_5, 5);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index5 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[6].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[6], pMqlRates_6, 6);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index6 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[7].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[7], pMqlRates_7, 7);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index7 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[8].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[8], pMqlRates_8, 8);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index8 ", result);
      return result;
    }
  }

  if(pParams->ratesBuffers->rates[9].info.isEnabled)
  {
    result = convertRatesArray(mqlVersion, pParams, pTZOffsets, &pMqlRatesInfo[9], pMqlRates_9, 9);
    if(result != SUCCESS)
    {
      logAsirikuyError("convertRatesArrays() index9 ", result);
      return result;
    }
  }

  return SUCCESS;
}

/**
* Copy MQL4 bid and ask prices into a BidAsk structure.
*
* @param BidAsk* pBidAsk
*   The structure into which the MQL4 bid and ask prices are
*   to be copied.
*
* @param double* mqlBidAsk
*   An array of 6 MQL4 bid and ask prices to be copied.
*   Indexes are defined by enum BidAskIndex.
*
*/
void copyBidAsk(BidAsk* pBidAsk, double* pMqlBidAsk)
{
  pBidAsk->arraySize          = 1; /* There are no historic bid & ask feeds in MetaTrader4 so we only need 1 array index each for the current bid & ask. */
  pBidAsk->bid                = &pMqlBidAsk[IDX_BID];
  pBidAsk->ask                = &pMqlBidAsk[IDX_ASK];
  pBidAsk->baseConversionBid  = pMqlBidAsk[IDX_BASE_CONVERSION_BID];
  pBidAsk->baseConversionAsk  = pMqlBidAsk[IDX_BASE_CONVERSION_ASK];
  pBidAsk->quoteConversionBid = pMqlBidAsk[IDX_QUOTE_CONVERSION_BID];
  pBidAsk->quoteConversionAsk = pMqlBidAsk[IDX_QUOTE_CONVERSION_ASK];
}

/**
* Copy MQL4 account information into an AccountInfo structure.
*
* @param AccountInfo* pInfo
*   The structure into which the MQL4 account information is
*   to be copied.
*
* @param char* accountCurrency
*   A string containing the 3 character MQL4 account currency code.
*   It is copied into the AccountInfo structure.
*
* @param double* mqlAccountInfo
*   An array containing MQL4 account information to be copied.
*   Indexes are defined by enum AccountInfoIndex.
*
*/
void copyAccountInfo(AccountInfo* pInfo, char* pAccountCurrency, char* pBrokerName, char* pReferenceName, double* pMqlAccountInfo)
{
  pInfo->brokerName                = pBrokerName;
  pInfo->referenceName             = pReferenceName;
  pInfo->currency                  = pAccountCurrency;
  pInfo->accountNumber             = (int)pMqlAccountInfo[IDX_ACCOUNT_NUMBER];
  pInfo->balance                   = pMqlAccountInfo[IDX_BALANCE];
  pInfo->equity                    = pMqlAccountInfo[IDX_EQUITY];
  pInfo->margin                    = pMqlAccountInfo[IDX_MARGIN];
  pInfo->leverage                  = pMqlAccountInfo[IDX_LEVERAGE];
  pInfo->contractSize              = pMqlAccountInfo[IDX_CONTRACT_SIZE];
  pInfo->minimumStop               = pMqlAccountInfo[IDX_MINIMUM_STOP];
  pInfo->stopoutPercent            = pMqlAccountInfo[IDX_STOPOUT_PERCENT];
  pInfo->totalOpenTradeRiskPercent = pMqlAccountInfo[IDX_TOTAL_OPEN_TRADE_RISK_PERCENT];
  pInfo->largestDrawdownPercent    = pMqlAccountInfo[IDX_LARGEST_DRAWDOWN_PERCENT];
}

/**
* Copy MQL order information into an OrderInfo structure.
*
* @param OrderInfo* pOrderInfo
*   The array of structures into which the MQL order
*   information is to be copied.
*
* @param double* pMqlOrderInfo
*   The MQL order information arrays to be copied.
*
* @param int orderInfoArraySize
*   The size of the first dimension of pMqlOrderInfo.
*
*/
void copyOrderInfo(OrderInfo* pOrderInfo, MqlOrderInfo* pMqlOrderInfo, int orderInfoArraySize)
{
  int i;
  for(i = 0; i < orderInfoArraySize; i++)
  {
    pOrderInfo[i].ticket      =       (int)pMqlOrderInfo[i].ticket;
    pOrderInfo[i].instanceId  =       (int)pMqlOrderInfo[i].instanceId;
    pOrderInfo[i].type        = (OrderType)pMqlOrderInfo[i].type;
    pOrderInfo[i].openTime    =    (time_t)pMqlOrderInfo[i].openTime;
    pOrderInfo[i].closeTime   =    (time_t)pMqlOrderInfo[i].closeTime;
    pOrderInfo[i].stopLoss    =     (float)pMqlOrderInfo[i].stopLoss;
    pOrderInfo[i].takeProfit  =     (float)pMqlOrderInfo[i].takeProfit;
    pOrderInfo[i].expiriation =    (time_t)pMqlOrderInfo[i].expiriation;
    pOrderInfo[i].openPrice   =     (float)pMqlOrderInfo[i].openPrice;
    pOrderInfo[i].closePrice  =     (float)pMqlOrderInfo[i].closePrice;
    pOrderInfo[i].lots        =     (float)pMqlOrderInfo[i].lots;
    pOrderInfo[i].profit      =     (float)pMqlOrderInfo[i].profit;
    pOrderInfo[i].commission  =     (float)pMqlOrderInfo[i].commission;
    pOrderInfo[i].swap        =     (float)pMqlOrderInfo[i].swap;

    pOrderInfo[i].isOpen = TRUE;
    if(!pMqlOrderInfo[i].isOpen)
    {
      pOrderInfo[i].isOpen = FALSE;
    }
  }
}

/**
* Copy MQL parameters into a StrategyParams structure
*
* @param StrategyParams* params
*   The structure into which all the MQL parameters are copied.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode convertMqlParameters(
  MQLVersion       mqlVersion,
  double*          pMqlSettings,
  char*            pMqlTradeSymbol,
  char*            pMqlAccountCurrency,
  char*            pMqlBrokerName,
  char*            pMqlRefBrokerName,
  int*             pMqlCurrentBrokerTime,
  int*             pMqlOpenOrdersCount,
  MqlOrderInfo*    pMqlOrderInfo,
  double*          pMqlAccountInfo,
  double*          pMqlBidAsk,
  MqlRatesInfo*    pMqlRatesInfo,
  void*            pMqlRates_0,
  void*            pMqlRates_1,
  void*            pMqlRates_2,
  void*            pMqlRates_3,
  void*            pMqlRates_4,
  void*            pMqlRates_5,
  void*            pMqlRates_6,
  void*            pMqlRates_7,
  void*            pMqlRates_8,
  void*            pMqlRates_9,
  StrategyResults* pMqlResults,
  StrategyParams*  pParams)
{
  AsirikuyReturnCode returnCode;
  TZOffsets tzOffsets;

  if(pMqlSettings == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlSettings = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlTradeSymbol == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlTradeSymbol = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlAccountCurrency == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlAccountCurrency = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlBrokerName == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlBrokerName = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlRefBrokerName == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlRefBrokerName = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlCurrentBrokerTime == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlCurrentBrokerTime = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlOpenOrdersCount == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlOpenOrdersCount = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlOrderInfo == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlOrderInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlAccountInfo == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlAccountInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlBidAsk == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlBidAsk = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlRatesInfo == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlRatesInfo = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlRates_0 == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlRates_0 = NULL\n");
    return NULL_POINTER;
  }

  if(pMqlResults == NULL)
  {
    logCritical("convertMqlParameters() failed. pMqlResults = NULL\n");
    return NULL_POINTER;
  }

  if(pParams == NULL)
  {
    logCritical("convertMqlParameters() failed. pParams = NULL\n");
    return NULL_POINTER;
  }


  pParams->tradeSymbol         = pMqlTradeSymbol;
  pParams->settings            = pMqlSettings;
  pParams->results             = pMqlResults;
  pParams->expertParameterInfo = getInstanceState((int)pParams->settings[STRATEGY_INSTANCE_ID])->parameterSpace;

  copyBidAsk(&pParams->bidAsk, pMqlBidAsk); 
  
  copyAccountInfo(&pParams->accountInfo, pMqlAccountCurrency, pMqlBrokerName, pMqlRefBrokerName, pMqlAccountInfo);
  copyOrderInfo(pParams->orderInfo, pMqlOrderInfo, (int)pParams->settings[ORDERINFO_ARRAY_SIZE]);
  
  /* Get the time offsets for each day of the year between the broker and reference broker. */
  returnCode = getTimeOffsets((time_t)*pMqlCurrentBrokerTime, &pParams->accountInfo, (BOOL)pParams->settings[IS_BACKTESTING], (int)pParams->settings[STRATEGY_INSTANCE_ID], &tzOffsets);
  if(returnCode != SUCCESS)
  {
    logAsirikuyError("convertMql4Parameters()", returnCode);
    return returnCode;
  }
  
  pParams->currentBrokerTime = getAdjustedBrokerTime((time_t)*pMqlCurrentBrokerTime, &tzOffsets);
  
  return convertRatesArrays(mqlVersion, pParams, &tzOffsets, pMqlRatesInfo, pMqlRates_0, pMqlRates_1, pMqlRates_2, pMqlRates_3, pMqlRates_4, pMqlRates_5, pMqlRates_6, pMqlRates_7, pMqlRates_8, pMqlRates_9);
}

AsirikuyReturnCode allocateOrderInfo(StrategyParams* pParams, int orderInfoArraySize)
{
  if(pParams == NULL)
  {
    logCritical("allocateOrderInfo() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  pParams->orderInfo = (OrderInfo*)malloc(orderInfoArraySize * sizeof(OrderInfo));
  
  return SUCCESS;
}

AsirikuyReturnCode freeOrderInfo(StrategyParams* pParams)
{
  if(pParams == NULL)
  {
    logCritical("freeOrderInfo() failed. pParams = NULL\n");
    return NULL_POINTER;
  }

  if(pParams->orderInfo)
  {
    free(pParams->orderInfo);
    pParams->orderInfo = NULL;
  }

  return SUCCESS;
}