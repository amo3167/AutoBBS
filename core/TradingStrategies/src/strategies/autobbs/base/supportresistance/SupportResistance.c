/*
 * Support/Resistance Module
 * 
 * Provides functions for calculating support and resistance levels
 * using Technical Analysis Library (TALib) functions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/supportresistance/SupportResistance.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

AsirikuyReturnCode iSRLevels_close(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	//int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	//int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;

	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].close, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].close, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}
	return SUCCESS;
}

AsirikuyReturnCode iSRLevels(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	//int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	//int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;
	
	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	return SUCCESS;
}

AsirikuyReturnCode iSRLevels_WithIndex(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex,int shift, double *pHigh, double *pLow,int *pHighIndex,int *pLowIndex)
{
	TA_RetCode retCode;
	//int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	//int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;	
	int highIndex, lowIndex;

	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}
		
	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);
	
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	retCode = TA_MININDEX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLowIndex);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MININDEX()", retCode);
	}

	retCode = TA_MAXINDEX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHighIndex);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAXINDEX()", retCode);
	}

	return SUCCESS;
}

