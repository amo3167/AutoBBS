/**
 * @file
 * @brief     Indicators including Asirikuy specific, customized, non-standard, and standard indicators which are not available in TaLib.
 * 
 * @author    MetaQuotes Software Corp (Original implementation of the Ultimate Oscillator & Keltner Channels)
 * @author    Maxim Feinshtein (Improved the MetaQuotes version of the Ultimate Oscillator for use by Asirikuy)
 * @author    Morgan Doel (Ported the indicators to the F4 framework)
 * @author    Daniel Fernandez (Assisted with design and code styling)
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
#include "Indicators.h"
#include "MovingAverages.h"
#include "Logging.h"

AsirikuyReturnCode barsToPreviousTime(const time_t* barOpenTimes, time_t time, int shiftIndex, int* pOutBarNumber)
{
	int i = 0, barsToTime = 0;

	while ( barOpenTimes[shiftIndex-i] > time)
	{
		i++;
		barsToTime++;
	}

	*pOutBarNumber = barsToTime ;

	return(SUCCESS);
}

AsirikuyReturnCode calculateKeltnerChannels(const double* pHigh, const double* pLow, const double* pClose, int arraySize, int rangeMaPeriod, int typicalPriceMaPeriod, double distanceUpper, double distanceLower, int shift, double* pOutUpper, double* pOutMiddle, double* pOutLower)
{
  AsirikuyReturnCode result = SUCCESS;
  double averageRange = -1;

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateKeltnerChannels() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateKeltnerChannels() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateKeltnerChannels() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(pOutUpper == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateKeltnerChannels() failed. pOutUpper = NULL\n");
    return NULL_POINTER;
  }

  if(pOutMiddle == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateKeltnerChannels() failed. pOutMiddle = NULL\n");
    return NULL_POINTER;
  }

  if(pOutLower == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateKeltnerChannels() failed. pOutLower = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < (rangeMaPeriod + shift))
  {
    logAsirikuyError("calculateKeltnerChannels()", NOT_ENOUGH_RATES_DATA);
    return NOT_ENOUGH_RATES_DATA;
  }

  if(arraySize < (typicalPriceMaPeriod + shift + 1))
  {
    logAsirikuyError("calculateKeltnerChannels()", NOT_ENOUGH_RATES_DATA);
    return NOT_ENOUGH_RATES_DATA;
  }

  result = calculateAverageRange(pHigh, pLow, arraySize, rangeMaPeriod, shift, &averageRange);
  if(result != SUCCESS)
  {
    logAsirikuyError("calculateKeltnerChannels()", result);
    return result;
  }

  result = calculateAverageTypicalPrice(pHigh, pLow, pClose, arraySize, typicalPriceMaPeriod, shift, pOutMiddle);
  if(result != SUCCESS)
  {
    logAsirikuyError("calculateKeltnerChannels()", result);
    return result;
  }

  *pOutUpper = *pOutMiddle + (averageRange * distanceUpper);
  *pOutLower = *pOutMiddle - (averageRange * distanceLower);

   return SUCCESS;
}

AsirikuyReturnCode calculateUltimateOscillator(const double* pHigh, const double* pLow, const double* pClose, int arraySize, int fastPeriod, int middlePeriod, int slowPeriod, int fastK, int middleK, int slowK, int shift, double* pOutUltimateOscillator)
{
  TA_RetCode retCode;
  int i, outBegIdx, outNBElement, startIdx = arraySize - 1 - shift;
  double rawUltimateOscillator, trueLow, fastMa = 0, middleMa = 0, slowMa = 0, fastAtr = 0, middleAtr = 0, slowAtr = 0;

  if(pHigh == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateUltimateOscillator() failed. pHigh = NULL\n");
    return NULL_POINTER;
  }

  if(pLow == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateUltimateOscillator() failed. pLow = NULL\n");
    return NULL_POINTER;
  }

  if(pClose == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateUltimateOscillator() failed. pClose = NULL\n");
    return NULL_POINTER;
  }

  if(pOutUltimateOscillator == NULL)
  {
    fprintf(stderr, "[CRITICAL] calculateUltimateOscillator() failed. pOutUltimateOscillator = NULL\n");
    return NULL_POINTER;
  }

  if(arraySize < (slowPeriod + shift + 1))
  {
    logAsirikuyError("calculateUltimateOscillator()", NOT_ENOUGH_RATES_DATA);
    return NOT_ENOUGH_RATES_DATA;
  }

  retCode  = TA_ATR(startIdx, startIdx, pHigh, pLow, pClose, fastPeriod, &outBegIdx, &outNBElement, &fastAtr);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_ATR()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_ATR(startIdx, startIdx, pHigh, pLow, pClose, middlePeriod, &outBegIdx, &outNBElement, &middleAtr);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_ATR()", retCode);
    return TA_LIB_ERROR;
  }

  retCode  = TA_ATR(startIdx, startIdx, pHigh, pLow, pClose, slowPeriod, &outBegIdx, &outNBElement, &slowAtr);
  if(retCode != TA_SUCCESS)
  {
    logTALibError("TA_ATR()", retCode);
    return TA_LIB_ERROR;
  }

  for(i = startIdx; i > (startIdx - fastPeriod); i--)
  {
    trueLow = pLow[i];
    if(pClose[i-1] < trueLow)
    {
      trueLow = pClose[i-1];
    }

    fastMa += pClose[i] - trueLow;
  }

  middleMa = fastMa;

  for(; i > (startIdx - middlePeriod); i--)
  {
    trueLow = pLow[i];
    if(pClose[i-1] < trueLow)
    {
      trueLow = pClose[i-1];
    }

    middleMa += pClose[i] - trueLow;
  }

  slowMa  = middleMa;

  for(; i > (startIdx - slowPeriod); i--)
  {
    trueLow = pLow[i];
    if(pClose[i-1] < trueLow)
    {
      trueLow = pClose[i-1];
    }

    slowMa += pClose[i] - trueLow;
  }

  fastMa   /= fastPeriod;
  middleMa /= middlePeriod;
  slowMa   /= slowPeriod;

  rawUltimateOscillator   = (fastK * fastMa / fastAtr) + (middleK * middleMa / middleAtr) + (slowK * slowMa / slowAtr);
  *pOutUltimateOscillator = 100 * rawUltimateOscillator / (fastK + middleK + slowK);

  return SUCCESS;
}
