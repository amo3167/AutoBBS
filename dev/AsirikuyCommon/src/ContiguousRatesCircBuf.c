/**
 * @file
 * @brief     Contains buffers for storing normalized rates data for all strategy instances.
 * @details   By storing rates data instead of copying it on every tick the framework runs much faster.
 * @details   This is different from a regular circular buffer because it needs to store arrays
 * @details   in a contiguous way to be compatible with TALib. The buffers look like this after each new bar:
 * @details   [0000111100] 1 = actual data, 0 = unused
 * @details   [0000011110] increment buffer pointer and add the new bar (no need to copy anything).
 * @details   [0000001111] reached end of buffer
 * @details   [1111000000] loop to start of buffer (must copy all data to the front of the buffer)
 * @details   [0111100000] increment buffer pointer and add the new bar as normal.
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
#include "ContiguousRatesCircBuf.h"
#include "TimeZoneOffsets.h"
#include "CriticalSection.h"

static int gExtendedBufferSize = DEFAULT_RATES_BUF_EXT;
static RatesBuffers gRatesBuffers[MAX_INSTANCES];

void setExtendedBufferSize(int size)
{
  gExtendedBufferSize = size;

  if(gExtendedBufferSize < 1)
  {
    gExtendedBufferSize = 1;
  }
}

static void initRatesBuffer(int instanceIndex, int ratesIndex)
{
  int i;

  gRatesBuffers[instanceIndex].instanceId = -1;

  for(i = 0; i < MAX_RATES_BUFFERS; i++)
  {
    Rates* rates = &gRatesBuffers[instanceIndex].rates[i];

    gRatesBuffers[instanceIndex].bufferOffsets[i] = 0;
    rates->info.isEnabled     = FALSE;
    rates->info.isBufferFull  = FALSE;
    rates->info.timeframe     = 0;
    rates->info.arraySize     = 0;
    rates->info.point         = 0;
    rates->time               = NULL;
    rates->open               = NULL;
    rates->high               = NULL;
    rates->low                = NULL;
    rates->close              = NULL;
    rates->volume             = NULL;
  }
}

static void resetRatesOffset(int instanceIndex, int ratesIndex)
{
  Rates* pRates = &gRatesBuffers[instanceIndex].rates[ratesIndex];
  int oldIndex, newIndex, oldOffset = gRatesBuffers[instanceIndex].bufferOffsets[ratesIndex];
  gRatesBuffers[instanceIndex].bufferOffsets[ratesIndex] = 0;

  //fprintf(stderr, (PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"resetRatesOffset() instanceId = %d, ratesIndex = %d, oldOffset = %d", gRatesBuffers[instanceIndex].instanceId, ratesIndex, oldOffset);

  /* Shift the pointers to the front of the buffers */
  if(pRates->time)   
  {
   pRates->time -= oldOffset;
  }
  if(pRates->open)   
  {
    pRates->open -= oldOffset;
  }
  if(pRates->high)   
  {
    pRates->high -= oldOffset;
  }
  if(pRates->low)    
  {
    pRates->low -= oldOffset;
  }
  if(pRates->close)  
  {
    pRates->close -= oldOffset;
  }
  if(pRates->volume) 
  {
    pRates->volume -= oldOffset;
  }

  /* Shift the data to the front of the buffers */
  for(newIndex = 0; newIndex < pRates->info.arraySize; newIndex++)
  {
    oldIndex = newIndex + oldOffset;

    if(pRates->time)
    {
      pRates->time[newIndex] = pRates->time[oldIndex];
    }
    if(pRates->open)
    {
      pRates->open[newIndex] = pRates->open[oldIndex];
    }
    if(pRates->high)
    {
      pRates->high[newIndex] = pRates->high[oldIndex];
    }
    if(pRates->low)
    {
      pRates->low[newIndex] = pRates->low[oldIndex];
    }
    if(pRates->close)
    {
      pRates->close[newIndex] = pRates->close[oldIndex];
    }
    if(pRates->volume)
    {
      pRates->volume[newIndex] = pRates->volume[oldIndex];
    }
  }
}

AsirikuyReturnCode allocateRates(RatesBuffers** ppRatesBuffer, int instanceId, RatesInfo* pRatesInfo)
{
  int instanceIndex, ratesIndex, ratesValueIndex;

  for(instanceIndex = 0; instanceIndex < MAX_INSTANCES; instanceIndex++)
  {
    if(gRatesBuffers[instanceIndex].instanceId == instanceId)
    {
      /* Rates are already allocated for this instance */
      *ppRatesBuffer = &gRatesBuffers[instanceIndex];
      return SUCCESS;
    }
  }

  enterCriticalSection();
  {
    /* Find the first unused index */
    for(instanceIndex = 0; instanceIndex < MAX_INSTANCES; instanceIndex++)
    {
      if(gRatesBuffers[instanceIndex].instanceId == -1)
      {
        gRatesBuffers[instanceIndex].instanceId = instanceId;
        break;
      }
    }

    if(instanceIndex >= MAX_INSTANCES)
    {
      leaveCriticalSection();
      return TOO_MANY_INSTANCES;
    }

    for(ratesIndex = 0; ratesIndex < MAX_RATES_BUFFERS; ratesIndex++)
    {
      Rates* pRates = &gRatesBuffers[instanceIndex].rates[ratesIndex];

      if(!pRatesInfo[ratesIndex].isEnabled)
      {
        continue;
      }

      pRates->info.isEnabled    = pRatesInfo[ratesIndex].isEnabled;
      pRates->info.isBufferFull = pRatesInfo[ratesIndex].isBufferFull;
      pRates->info.timeframe    = pRatesInfo[ratesIndex].timeframe;
      pRates->info.arraySize    = pRatesInfo[ratesIndex].arraySize;
      pRates->info.point        = pRatesInfo[ratesIndex].point;
	  pRates->info.digits       = pRatesInfo[ratesIndex].digits;

      pRates->time   = (time_t*)malloc((pRates->info.arraySize + gExtendedBufferSize) * sizeof(time_t));
      pRates->open   = (double*)malloc((pRates->info.arraySize + gExtendedBufferSize) * sizeof(double));
      pRates->high   = (double*)malloc((pRates->info.arraySize + gExtendedBufferSize) * sizeof(double));
      pRates->low    = (double*)malloc((pRates->info.arraySize + gExtendedBufferSize) * sizeof(double));
      pRates->close  = (double*)malloc((pRates->info.arraySize + gExtendedBufferSize) * sizeof(double));
      pRates->volume = (double*)malloc((pRates->info.arraySize + gExtendedBufferSize) * sizeof(double));

      for(ratesValueIndex = 0; ratesValueIndex < pRates->info.arraySize; ratesValueIndex++)
      {
        pRates->time  [ratesValueIndex] = 0;
        pRates->open  [ratesValueIndex] = 0;
        pRates->high  [ratesValueIndex] = 0;
        pRates->low   [ratesValueIndex] = 0;
        pRates->close [ratesValueIndex] = 0;
        pRates->volume[ratesValueIndex] = 0;
      }
    }

    *ppRatesBuffer = &gRatesBuffers[instanceIndex];
  }
  leaveCriticalSection();

  return SUCCESS;
}

static void resetRatesBuffer(int instanceIndex, int ratesIndex)
{
  Rates* pRates = &gRatesBuffers[instanceIndex].rates[ratesIndex];

  resetRatesOffset(instanceIndex, ratesIndex);

  if(pRates->time)
  {
    free(pRates->time);
  }
  if(pRates->open)
  {
    free(pRates->open);
  }
  if(pRates->high)
  {
    free(pRates->high);
  }
  if(pRates->low)
  {
    free(pRates->low);
  }
  if(pRates->close)
  {
    free(pRates->close);
  }
  if(pRates->volume)
  {
    free(pRates->volume);
  }

  /* re-initialize rates buffer - sets all pointers to NULL */
  initRatesBuffer(instanceIndex, ratesIndex);
  return;
}

void resetInstanceBuffer(int instanceId)
{
  int i, j;
  for(i = 0; i < MAX_INSTANCES; i++)
  {
    for(j = 0; j < MAX_RATES_BUFFERS; j++)
    {
      if(gRatesBuffers[i].instanceId == instanceId)
      {
        resetRatesBuffer(i, j);
        gRatesBuffers[i].instanceId = -1;
      }
    }
  }
}

void resetAllRatesBuffers()
{
  int i, j;
  for(i = 0; i < MAX_INSTANCES; i++)
  {
    for(j = 0; j < MAX_RATES_BUFFERS; j++)
    {
      resetRatesBuffer(i, j);
      gRatesBuffers[i].instanceId = -1;
    }
  }
}



AsirikuyReturnCode incrementRatesOffset(int instanceId, int ratesIndex)
{
  int instanceIndex;

  for(instanceIndex = 0; instanceIndex < MAX_INSTANCES; instanceIndex++)
  {
    if(gRatesBuffers[instanceIndex].instanceId == instanceId)
    {
      break;
    }
  }

  if(instanceIndex >= MAX_INSTANCES)
  {
    fprintf(stderr, "[CRITICAL] incrementRatesOffset() failed. instanceId: %d does not have a rates buffer allocated\n", instanceId);
    return UNKNOWN_INSTANCE_ID;
  }

  gRatesBuffers[instanceIndex].bufferOffsets[ratesIndex]++;
  gRatesBuffers[instanceIndex].rates[ratesIndex].time++;
  gRatesBuffers[instanceIndex].rates[ratesIndex].open++;
  gRatesBuffers[instanceIndex].rates[ratesIndex].high++;
  gRatesBuffers[instanceIndex].rates[ratesIndex].low++;
  gRatesBuffers[instanceIndex].rates[ratesIndex].close++;
  gRatesBuffers[instanceIndex].rates[ratesIndex].volume++;

  if(gRatesBuffers[instanceIndex].bufferOffsets[ratesIndex] >= gExtendedBufferSize)
  {
    resetRatesOffset(instanceIndex, ratesIndex);
  }
  
  return SUCCESS;
}

AsirikuyReturnCode copyRatesBuffer(Rates* pDest, Rates* pSrc)
{
  if(pDest == NULL)
  {
    fprintf(stderr, "[CRITICAL] copyRatesPointers() failed. pDest = NULL\n");
    return NULL_POINTER;
  }

  if(pSrc == NULL)
  {
    fprintf(stderr, "[CRITICAL] copyRatesPointers() failed. pSrc = NULL\n");
    return NULL_POINTER;
  }

  pDest->info.timeframe = pSrc->info.timeframe;
  pDest->info.arraySize = pSrc->info.arraySize;
  pDest->info.point     = pSrc->info.point;
  pDest->time           = pSrc->time;
  pDest->open           = pSrc->open;
  pDest->high           = pSrc->high;
  pDest->low            = pSrc->low;
  pDest->close          = pSrc->close;
  pDest->volume         = pSrc->volume;

  return SUCCESS;
}