/*
 * Indicator Loaders Module
 * 
 * Provides functions for loading indicators from different timeframes.
 * 
 * This module loads various technical indicators including:
 * - Monthly indicators (10-week high/low)
 * - Weekly indicators (trend, support/resistance, ATR prediction)
 * - Daily indicators (trend, support/resistance, ATR prediction)
 * - Intraday KeyK indicators (for detecting intraday reversal patterns)
 * - Moving averages and pivot points
 * 
 * The main entry point is loadIndicators_Internal() which orchestrates
 * loading all indicators in the correct order.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"

/* Include dependent modules */
#include "strategies/autobbs/base/supportresistance/SupportResistance.h"
#include "strategies/autobbs/base/trendanalysis/TrendAnalysis.h"
#include "strategies/autobbs/base/trendcalculators/TrendCalculators.h"
#include "strategies/autobbs/base/atrprediction/ATRPrediction.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// Indicator loading constants
#define MONTHLY_SR_PERIODS 8              // Number of weeks for monthly support/resistance
#define WEEKLY_SR_PERIODS 2                // Number of weeks for weekly support/resistance
#define DAILY_SR_PERIODS 2                 // Number of days for daily support/resistance
#define MAX_INTRADAY_BARS_TO_SEARCH 290    // Maximum bars to search back for intraday KeyK pattern
#define KEYK_MOVEMENT_THRESHOLD 0.5        // Movement threshold as fraction of daily ATR (50%)
#define KEYK_CLOSE_THRESHOLD 3.0            // Close position threshold as fraction of movement (1/3)
#define EOD_HOUR_THRESHOLD 23              // End of day hour threshold
#define EOD_MINUTE_THRESHOLD 30            // End of day minute threshold
#define WEEKLY_ATR_PERIOD 4                // Period for weekly ATR calculation
#define MA_PERIOD_50 50                    // 50-period moving average
#define MA_PERIOD_200 200                  // 200-period moving average
#define MA_MODE_SMA 3                      // Simple moving average mode

/**
 * Loads monthly indicators (10-week high/low).
 * 
 * Calculates support/resistance levels based on the previous 8 weeks
 * of weekly data to determine monthly high and low levels.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success, error code on failure
 * 
 * Output fields set in pIndicators:
 * - monthlyHigh: Monthly high level (10-week high)
 * - monthlyLow: Monthly low level (10-week low)
 */
static AsirikuyReturnCode loadMonthlyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);
	
	// Calculate monthly high/low using 8-week support/resistance levels
	iSRLevels(pParams, pIndicators, B_WEEKLY_RATES, shift1Index, MONTHLY_SR_PERIODS, 
	          &(pIndicators->monthlyHigh), &(pIndicators->monthlyLow));

	logDebug("System InstanceID = %d, BarTime = %s, 10weeksHigh = %lf, 10weeksLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->monthlyHigh, pIndicators->monthlyLow);
	
	return SUCCESS;
}

/**
 * Loads weekly indicators including trends, support/resistance, and ATR prediction.
 * 
 * Calculates:
 * - 3-rules trend
 * - High/Low trend
 * - Moving average trend (based on weeklyMAMode)
 * - Support/resistance levels
 * - Weekly trend (via workoutWeeklyTrend)
 * - Weekly ATR prediction
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success, error code on failure
 * 
 * Output fields set in pIndicators:
 * - weekly3RulesTrend: 3-rules trend direction
 * - weeklyHLTrend: High/Low trend direction
 * - weeklyMATrend: Moving average trend direction
 * - weeklyHigh, weeklyLow: Support/resistance levels
 * - weeklyTrend: Combined weekly trend
 * - weeklyS, weeklyR, weeklyTP: Support, resistance, take profit levels
 * - Weekly ATR prediction fields (via predictWeeklyATR)
 */
AsirikuyReturnCode loadWeeklyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{	
	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;		
	int shift1Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);
		
	// Calculate weekly trends
	iTrend3Rules(pParams, pIndicators, B_WEEKLY_RATES, 2, &(pIndicators->weekly3RulesTrend), 0);
	iTrend_HL(B_WEEKLY_RATES, &(pIndicators->weeklyHLTrend), 0);
	
	// Calculate moving average trend based on mode
	if (pIndicators->weeklyMAMode == 0)
		iTrend_MA(pIndicators->weeklyATR, B_FOURHOURLY_RATES, &(pIndicators->weeklyMATrend));
	else
		iTrend_MA_WeeklyBar_For4H(pIndicators->weeklyATR, &(pIndicators->weeklyMATrend));

	// Calculate weekly support/resistance levels
	iSRLevels(pParams, pIndicators, B_WEEKLY_RATES, shift1Index, WEEKLY_SR_PERIODS, 
	          &(pIndicators->weeklyHigh), &(pIndicators->weeklyLow));

	logDebug("System InstanceID = %d, BarTime = %s, weeklyHLTrend = %ld, weeklyMATrend = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyHLTrend, pIndicators->weeklyMATrend);

	logDebug("System InstanceID = %d, BarTime = %s, weekly3RulesTrend = %ld, weeklyHigh = %lf, weeklyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weekly3RulesTrend, pIndicators->weeklyHigh, pIndicators->weeklyLow);

	// Calculate combined weekly trend and support/resistance
	workoutWeeklyTrend(pParams, pIndicators);
	logDebug("System InstanceID = %d, BarTime = %s, weeklyTrend = %ld, weeklySupport = %lf, weeklyResistance = %lf, weeklyTP = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyTrend, pIndicators->weeklyS, pIndicators->weeklyR, pIndicators->weeklyTP);

	// Predict weekly ATR
	predictWeeklyATR(pParams, pIndicators);

	return SUCCESS;
}

/**
 * Loads intraday KeyK indicators to detect intraday reversal patterns.
 * 
 * Searches backwards through today's primary bars to find KeyK patterns.
 * A KeyK pattern is detected when:
 * - The bar movement (high-low) >= 50% of daily ATR
 * - The close is within 1/3 of the high (bullish KeyK) or low (bearish KeyK)
 * 
 * This function sets the intraday trend and the index where the KeyK was found.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success, error code on failure
 * 
 * Output fields set in pIndicators:
 * - intradayTrend: Trend direction (1 = bullish, -1 = bearish, 0 = none)
 * - intradyIndex: Index of the bar where KeyK pattern was found (0 if not found)
 *                 Note: Field name has typo but kept for consistency with struct definition
 */
static AsirikuyReturnCode loadIntradayKeyKIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	double prePrimaryHigh, prePrimaryLow, prePrimaryClose, primaryMovement;
	int i;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime, tempTime;
	struct tm timeInfo1, timeInfo2;

	char timeString[MAX_TIME_STRING_SIZE] = "";
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Initialize intraday indicators
	pIndicators->intradayTrend = 0;
	pIndicators->intradyIndex = 0;

	// Search backwards through today's primary bars to find KeyK pattern
	// KeyK pattern indicates a potential intraday reversal
	for (i = 1; i < MAX_INTRADAY_BARS_TO_SEARCH; i++)
	{
		tempTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index - i];
		safe_gmtime(&timeInfo2, tempTime);

		// Stop searching if we've moved to a different day
		if (timeInfo1.tm_yday != timeInfo2.tm_yday)
			break;

		pIndicators->intradyIndex = shift0Index - i;

		prePrimaryHigh = iHigh(B_PRIMARY_RATES, i);
		prePrimaryLow = iLow(B_PRIMARY_RATES, i);
		prePrimaryClose = iClose(B_PRIMARY_RATES, i);

		primaryMovement = fabs(prePrimaryHigh - prePrimaryLow);
		
		// KeyK pattern detection:
		// - Movement must be >= 50% of daily ATR to be significant
		// - Close must be within 1/3 of high (bullish) or low (bearish)
		// - This indicates a potential reversal pattern
		if (primaryMovement >= KEYK_MOVEMENT_THRESHOLD * pIndicators->dailyATR)
		{
			// Bullish KeyK: close near high (within 1/3 of movement from high)
			if (fabs(prePrimaryHigh - prePrimaryClose) < primaryMovement / KEYK_CLOSE_THRESHOLD)
			{
				pIndicators->intradayTrend = 1;
				break;
			}
			
			// Bearish KeyK: close near low (within 1/3 of movement from low)
			if (fabs(prePrimaryLow - prePrimaryClose) < primaryMovement / KEYK_CLOSE_THRESHOLD)
			{
				pIndicators->intradayTrend = -1;
				break;
			}
		}
	}

	logDebug("System InstanceID = %d, BarTime = %s, intradayTrend = %ld, intradyIndex = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->intradayTrend, pIndicators->intradyIndex);

	return returnCode;
}

/**
 * Loads daily indicators including trends, support/resistance, and ATR prediction.
 * 
 * Calculates:
 * - 3-rules trend (with fallback to previous days if needed)
 * - High/Low trend
 * - Moving average trend
 * - Support/resistance levels
 * - Daily trend (via workoutDailyTrend)
 * - Daily ATR prediction
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @param index Index parameter: 1 = current day (EOD), 0 = previous day (SOD)
 * @return SUCCESS on success, error code on failure
 * 
 * Output fields set in pIndicators:
 * - daily3RulesTrend: 3-rules trend direction (with fallback logic)
 * - dailyHLTrend: High/Low trend direction
 * - dailyMATrend: Moving average trend direction
 * - dailyHigh, dailyLow: Support/resistance levels
 * - dailyTrend: Combined daily trend
 * - dailyS, dailyR, dailyTP: Support, resistance, take profit levels
 * - Daily ATR prediction fields (via predictDailyATR)
 */
static AsirikuyReturnCode loadDailyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators, int index)
{	
	int shift0Index = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;	
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int pre3KTrend, preHLTrend;
	
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_DAILY_RATES].time[shift0Index]);

	// Calculate daily trends
	iTrend3Rules(pParams, pIndicators, B_DAILY_RATES, 2, &(pIndicators->daily3RulesTrend), index);
	iTrend_HL(B_DAILY_RATES, &(pIndicators->dailyHLTrend), index);
	iTrend_MA_DailyBar_For1H(pIndicators->dailyATR, &(pIndicators->dailyMATrend), index);

	// Calculate daily support/resistance levels
	iSRLevels(pParams, pIndicators, B_DAILY_RATES, shift1Index - index, DAILY_SR_PERIODS, 
	          &(pIndicators->dailyHigh), &(pIndicators->dailyLow));

	logDebug("System InstanceID = %d, BarTime = %s, dailyHLTrend = %ld, dailyMATrend = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyHLTrend, pIndicators->dailyMATrend);

	logDebug("System InstanceID = %d, BarTime = %s, daily3RulesTrend = %ld, dailyHigh = %lf, dailyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->daily3RulesTrend, pIndicators->dailyHigh, pIndicators->dailyLow);

	// Fallback logic: If HL trend is up/down but 3-rules trend is neutral,
	// check previous days to determine trend direction
	if (pIndicators->dailyHLTrend == 1 && pIndicators->daily3RulesTrend == 0)
	{
		// Check previous day's 3-rules trend
		iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 1, index);
		if (pre3KTrend == UP)
		{
			pIndicators->daily3RulesTrend = UP;
		}
		else
		{
			// Check previous day's HL trend
			iTrend_HL_preDays(B_DAILY_RATES, &preHLTrend, 1, index);
			if (preHLTrend == 1 && pre3KTrend == 0)
			{
				// Check 2 days ago's 3-rules trend
				iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 2, index);
				if (pre3KTrend == UP)
					pIndicators->daily3RulesTrend = UP;
			}
		}
	}

	if (pIndicators->dailyHLTrend == -1 && pIndicators->daily3RulesTrend == 0)
	{
		// Check previous day's 3-rules trend
		iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 1, index);
		if (pre3KTrend == DOWN)
		{
			pIndicators->daily3RulesTrend = DOWN;
		}
		else
		{
			// Check previous day's HL trend
			iTrend_HL_preDays(B_DAILY_RATES, &preHLTrend, 1, index);
			if (preHLTrend == -1 && pre3KTrend == 0)
			{
				// Check 2 days ago's 3-rules trend
				iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 2, index);
				if (pre3KTrend == DOWN)
					pIndicators->daily3RulesTrend = DOWN;
			}
		}
	}

	// Calculate combined daily trend and support/resistance
	workoutDailyTrend(pParams, pIndicators);
	logDebug("System InstanceID = %d, BarTime = %s, dailyTrend = %ld, dailySupport = %lf, dailyResistance = %lf, dailyTP = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyTrend, pIndicators->dailyS, pIndicators->dailyR, pIndicators->dailyTP);

	// Predict daily ATR
	predictDailyATR(pParams, pIndicators);

	return SUCCESS;
}

/**
 * Main indicator loader function that orchestrates loading all indicators.
 * 
 * Loads indicators in the following order:
 * 1. Basic indicators (ATR, moving averages, pivots)
 * 2. Monthly indicators (if strategy_mode > 0)
 * 3. Weekly indicators (if strategy_mode > 0)
 * 4. Daily indicators (with EOD/SOD detection)
 * 5. Intraday KeyK indicators
 * 6. MA trend indicators (for weekly swing strategies)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success, error code on failure
 */
static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{	
	int shift0Index = pParams->ratesBuffers->rates[B_HOURLY_RATES].info.arraySize - 1;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	time_t currentTime;
	struct tm timeInfo1;
	int index = 0;

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_HOURLY_RATES].time[shift0Index]);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	// Load basic ATR indicators
	pIndicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	if (pIndicators->strategy_mode > 0)
		pIndicators->weeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_ATR_PERIOD, 1);

	// Load moving averages
	pIndicators->ma1H50M = iMA(MA_MODE_SMA, B_HOURLY_RATES, MA_PERIOD_50, 1);
	pIndicators->ma1H200M = iMA(MA_MODE_SMA, B_HOURLY_RATES, MA_PERIOD_200, 1);

	if (pIndicators->strategy_mode > 0)
	{
		pIndicators->ma4H50M = iMA(MA_MODE_SMA, B_FOURHOURLY_RATES, MA_PERIOD_50, 1);
		pIndicators->ma4H200M = iMA(MA_MODE_SMA, B_FOURHOURLY_RATES, MA_PERIOD_200, 1);
	}

	logDebug("System InstanceID = %d, BarTime = %s, MA1H200M = %lf, MA4H200M = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->ma1H200M, pIndicators->ma4H200M);

	// Load daily pivot points
	iPivot(B_DAILY_RATES, 1, &(pIndicators->dailyPivot),
		&(pIndicators->dailyS1), &(pIndicators->dailyR1),
		&(pIndicators->dailyS2), &(pIndicators->dailyR2),
		&(pIndicators->dailyS3), &(pIndicators->dailyR3));

	logDebug("System InstanceID = %d, BarTime = %s, dailyPivot = %lf, dailyS1 = %lf, dailyR1 = %lf, dailyS2 = %lf, dailyR2 = %lf, dailyS3 = %lf, dailyR3 = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyPivot, pIndicators->dailyS1, pIndicators->dailyR1, pIndicators->dailyS2, pIndicators->dailyR2, pIndicators->dailyS3, pIndicators->dailyR3);

	// Load weekly pivot points (if strategy_mode > 0)
	if (pIndicators->strategy_mode > 0)
	{
		iPivot(B_WEEKLY_RATES, 1, &(pIndicators->weeklyPivot),
			&(pIndicators->weeklyS1), &(pIndicators->weeklyR1),
			&(pIndicators->weeklyS2), &(pIndicators->weeklyR2),
			&(pIndicators->weeklyS3), &(pIndicators->weeklyR3));

		logDebug("System InstanceID = %d, BarTime = %s, weeklyPivot = %lf, weeklyS1 = %lf, weeklyR1 = %lf, weeklyS2 = %lf, weeklyR2 = %lf, weeklyS3 = %lf, weeklyR3 = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyPivot, pIndicators->weeklyS1, pIndicators->weeklyR1, pIndicators->weeklyS2, pIndicators->weeklyR2, pIndicators->weeklyS3, pIndicators->weeklyR3);
	}

	// Load monthly and weekly indicators (if strategy_mode > 0)
	if (pIndicators->strategy_mode > 0)
	{
		loadMonthlyIndicators(pParams, pIndicators);
		loadWeeklyIndicators(pParams, pIndicators);
	}
	
	// Determine if we're at end of day (EOD) or start of day (SOD)
	// EOD: hour >= 23 and minute >= 30
	if (timeInfo1.tm_hour >= EOD_HOUR_THRESHOLD && timeInfo1.tm_min >= EOD_MINUTE_THRESHOLD)
	{
		index = 1;  // EOD - use current day indicators
	}

	// Load daily and intraday indicators
	loadDailyIndicators(pParams, pIndicators, index);
	loadIntradayKeyKIndicators(pParams, pIndicators);
	
	// Load MA trend indicators (used for weekly swing strategies)
	// TODO: These are currently inactive - consider cleanup if not needed
	pIndicators->maTrend = getMATrend(iAtr(B_PRIMARY_RATES, 20, 1), B_PRIMARY_RATES, 1);
	pIndicators->ma_Signal = getMATrend_Signal(B_PRIMARY_RATES);

	return SUCCESS;
}

/**
 * Public wrapper for loadIndicators() for use by BaseCore dispatcher.
 * 
 * This function is exported to allow Base.c to call the internal
 * loadIndicators() function without exposing it as a public API.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success, error code on failure
 */
AsirikuyReturnCode loadIndicators_Internal(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	return loadIndicators(pParams, pIndicators);
}
