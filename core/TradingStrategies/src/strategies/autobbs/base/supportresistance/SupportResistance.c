/*
 * Support/Resistance Module
 * 
 * Provides functions for calculating support and resistance levels
 * using Technical Analysis Library (TALib) functions.
 * 
 * Support and resistance levels are calculated by finding the minimum
 * (support) and maximum (resistance) prices over a specified period.
 * These levels help identify potential price reversal points.
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

/**
 * Calculates support and resistance levels based on close prices.
 * 
 * Uses TALib TA_MIN and TA_MAX functions to find the minimum and maximum
 * close prices over the specified period. This is useful when you want
 * support/resistance based on closing prices rather than high/low extremes.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use (B_DAILY_RATES, B_WEEKLY_RATES, etc.)
 * @param shiftIndex Starting index in the rates array (typically shift1Index)
 * @param shift Number of periods to look back for min/max calculation
 * @param pHigh Output parameter: pointer to store the resistance level (maximum close)
 * @param pLow Output parameter: pointer to store the support level (minimum close)
 * @return SUCCESS on success, error code on TALib failure
 * 
 * Example:
 *   iSRLevels_close(pParams, pIndicators, B_DAILY_RATES, shift1Index, 10, &high, &low);
 *   // Finds the highest and lowest close prices over the last 10 days
 */
AsirikuyReturnCode iSRLevels_close(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shiftIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	int outBegIdx, outNBElement;

	// Find minimum close price (support level)
	retCode = TA_MIN(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].close, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	// Find maximum close price (resistance level)
	retCode = TA_MAX(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].close, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}
	
	return SUCCESS;
}

/**
 * Calculates support and resistance levels based on high/low prices.
 * 
 * Uses TALib TA_MIN and TA_MAX functions to find the minimum low price
 * (support) and maximum high price (resistance) over the specified period.
 * This is the standard method for calculating support/resistance levels.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use (B_DAILY_RATES, B_WEEKLY_RATES, etc.)
 * @param shiftIndex Starting index in the rates array (typically shift1Index)
 * @param shift Number of periods to look back for min/max calculation
 * @param pHigh Output parameter: pointer to store the resistance level (maximum high)
 * @param pLow Output parameter: pointer to store the support level (minimum low)
 * @return SUCCESS on success, error code on TALib failure
 * 
 * Example:
 *   iSRLevels(pParams, pIndicators, B_WEEKLY_RATES, shift1Index, 8, &high, &low);
 *   // Finds the highest high and lowest low over the last 8 weeks
 */
AsirikuyReturnCode iSRLevels(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shiftIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	int outBegIdx, outNBElement;
	
	// Find minimum low price (support level)
	retCode = TA_MIN(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	// Find maximum high price (resistance level)
	retCode = TA_MAX(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	return SUCCESS;
}

/**
 * Calculates support and resistance levels with index information.
 * 
 * Similar to iSRLevels(), but also returns the array indices where the
 * minimum and maximum values were found. This is useful when you need
 * to know not just the price levels, but also when they occurred.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use (B_DAILY_RATES, B_WEEKLY_RATES, etc.)
 * @param shiftIndex Starting index in the rates array (typically shift1Index)
 * @param shift Number of periods to look back for min/max calculation
 * @param pHigh Output parameter: pointer to store the resistance level (maximum high)
 * @param pLow Output parameter: pointer to store the support level (minimum low)
 * @param pHighIndex Output parameter: pointer to store the index where maximum high was found
 * @param pLowIndex Output parameter: pointer to store the index where minimum low was found
 * @return SUCCESS on success, error code on TALib failure
 * 
 * Example:
 *   iSRLevels_WithIndex(pParams, pIndicators, B_DAILY_RATES, shift1Index, 20, 
 *                       &high, &low, &highIdx, &lowIdx);
 *   // Finds high/low levels and their indices over the last 20 days
 */
AsirikuyReturnCode iSRLevels_WithIndex(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shiftIndex, int shift, double *pHigh, double *pLow, int *pHighIndex, int *pLowIndex)
{
	TA_RetCode retCode;
	int outBegIdx, outNBElement;

	// Find minimum low price (support level)
	retCode = TA_MIN(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}
		
	// Find maximum high price (resistance level)
	retCode = TA_MAX(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);
	
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	// Find the index where minimum low was found
	retCode = TA_MININDEX(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLowIndex);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MININDEX()", retCode);
	}

	// Find the index where maximum high was found
	retCode = TA_MAXINDEX(shiftIndex, shiftIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHighIndex);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAXINDEX()", retCode);
	}

	return SUCCESS;
}
