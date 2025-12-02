/*
 * ATR Prediction Module
 * 
 * Provides functions for predicting Average True Range (ATR)
 * for daily and weekly timeframes, including longer-term predictions.
 * 
 * The prediction algorithm uses historical ATR values to forecast future
 * volatility and price ranges, taking into account:
 * - Short, medium, and long-term ATR averages
 * - Same weekday/week patterns
 * - Support and resistance levels (S2/R2 pivots)
 * - Current intraday/intraweek price action
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/atrprediction/ATRPrediction.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// ATR prediction constants
#define DAILY_ATR_PERIODS 5              // Number of days for daily ATR prediction
#define WEEKLY_ATR_PERIODS 4              // Number of weeks for weekly ATR prediction
#define DAILY_ATR_MIN_FACTOR 0.5          // Minimum ATR factor (50% of minATR)
#define DAILY_ATR_MAX_FACTOR 1.3          // Maximum ATR factor (130% of minATR)
#define DAILY_ATR_THRESHOLD 0.7           // Threshold for using pMinATR vs average
#define WEEKLY_ATR_MIN_FACTOR 0.6          // Minimum ATR factor for weekly (60% of minATR)
#define WEEKLY_ATR_THRESHOLD 0.7           // Threshold for using pMinATR vs average
#define INIT_MIN_ATR 10000.0              // Initial minimum ATR value (high sentinel)
#define INIT_MAX_ATR -10000.0             // Initial maximum ATR value (low sentinel)

// ATR period configurations
#define DAILY_SHORT_ATR_PERIOD 2           // Short-term daily ATR period
#define DAILY_MEDIUM_ATR_PERIOD 5          // Medium-term daily ATR period
#define DAILY_LONG_ATR_PERIOD 20           // Long-term daily ATR period
#define WEEKLY_SHORT_ATR_PERIOD 2          // Short-term weekly ATR period
#define WEEKLY_MEDIUM_ATR_PERIOD 4         // Medium-term weekly ATR period
#define WEEKLY_LONG_ATR_PERIOD 8           // Long-term weekly ATR period
#define WEEKLY_LONGER_TERM_ATR_PERIOD 16   // Longer-term weekly ATR period

// Same weekday/week ATR calculation offsets
#define DAILY_SAME_WEEKDAY_OFFSETS 4        // Number of offsets for daily same weekday calculation
#define WEEKLY_SAME_MONTH_OFFSETS_SHORT 2   // Number of offsets for weekly same month (short)
#define WEEKLY_SAME_MONTH_OFFSETS_LONG 4    // Number of offsets for weekly same month (long)

/**
 * Predicts the daily ATR (Average True Range) and calculates predicted high/low prices.
 * 
 * Algorithm:
 * 1. Calculates short (2), medium (5), and long (20) period ATRs
 * 2. Predicts ATR using 5-day average minus recent ATR values
 * 3. Applies constraints to keep prediction within reasonable bounds
 * 4. Uses same weekday ATR average as additional constraint
 * 5. Calculates predicted high/low based on last close and intraday prices
 * 6. Constrains predictions within daily S2/R2 pivot levels
 * 7. Determines trend direction (VOL_IN_RANGE, VOL_BREAK_UP, VOL_BREAK_DOWN)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate with predictions
 * 
 * Output fields set in pIndicators:
 * - pDailyMinATR, pDailyMaxATR: Min/max ATR values
 * - pDailyATR, pDailyPredictATR: Predicted ATR
 * - pDailyHigh, pDailyLow: Predicted high/low prices
 * - pMaxDailyHigh, pMaxDailyLow: Maximum predicted high/low
 * - pDailyTrend: Trend classification (VOL_IN_RANGE, VOL_BREAK_UP, VOL_BREAK_DOWN)
 */
void predictDailyATR(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double shortDailyATR, mediumDailyATR, longDailyATR;
	double ATR0, ATR1, ATR2, ATR3, ATR4;
	double minATR = INIT_MIN_ATR;
	double maxATR = INIT_MAX_ATR;
	double pMinATR, pMaxATR, pATRSameWeekDay = INIT_MIN_ATR;
	double pATR;
	double pLow = INIT_MAX_ATR;
	double pHigh = INIT_MIN_ATR;
	double pMaxLow = INIT_MAX_ATR;
	double pMaxHigh = INIT_MIN_ATR;
	
	// Get current and previous day prices
	double lastDailyClose = iClose(B_DAILY_RATES, 1);
	double intradayClose = iClose(B_DAILY_RATES, 0);
	double intradayHigh = iHigh(B_DAILY_RATES, 0);
	double intradayLow = iLow(B_DAILY_RATES, 0);

	int shift0Index = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_DAILY_RATES].time[shift0Index]);

	// Get recent ATR values (current and 4 previous days)
	ATR0 = iAtr(B_DAILY_RATES, 1, 0);
	ATR1 = iAtr(B_DAILY_RATES, 1, 1);
	ATR2 = iAtr(B_DAILY_RATES, 1, 2);
	ATR3 = iAtr(B_DAILY_RATES, 1, 3);
	ATR4 = iAtr(B_DAILY_RATES, 1, 4);

	// Calculate short, medium, and long-term ATR averages
	shortDailyATR = iAtr(B_DAILY_RATES, DAILY_SHORT_ATR_PERIOD, 1);
	mediumDailyATR = iAtr(B_DAILY_RATES, DAILY_MEDIUM_ATR_PERIOD, 1);
	longDailyATR = iAtr(B_DAILY_RATES, DAILY_LONG_ATR_PERIOD, 1);

	// Find min and max ATR across all periods
	minATR = min(minATR, shortDailyATR);
	minATR = min(minATR, mediumDailyATR);
	minATR = min(minATR, longDailyATR);
	pIndicators->pDailyMinATR = minATR;

	maxATR = max(maxATR, shortDailyATR);
	maxATR = max(maxATR, mediumDailyATR);
	maxATR = max(maxATR, longDailyATR);
	pIndicators->pDailyMaxATR = maxATR;
	
	logDebug("System InstanceID = %d, BarTime = %s, shortDailyATR = %f, mediumDailyATR = %f, longDailyATR = %f, minATR = %f, maxATR = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyATR, mediumDailyATR, longDailyATR, minATR, maxATR);

	logDebug("System InstanceID = %d, BarTime = %s, ATR0 = %f, ATR1 = %f, ATR2 = %f, ATR3 = %f, ATR4 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, ATR1, ATR2, ATR3, ATR4);

	// Predict ATR using 5-day average minus recent ATR values
	pMinATR = minATR * DAILY_ATR_PERIODS - ATR1 - ATR2 - ATR3 - ATR4;
	pMaxATR = maxATR * DAILY_ATR_PERIODS - ATR1 - ATR2 - ATR3 - ATR4;

	// Constrain predicted ATR within reasonable bounds
	if (pMinATR < minATR * DAILY_ATR_MIN_FACTOR)
		pMinATR = minATR * DAILY_ATR_MIN_FACTOR;
	else if (pMinATR > minATR * DAILY_ATR_MAX_FACTOR)
		pMinATR = minATR * DAILY_ATR_MAX_FACTOR;

	if (pMaxATR < minATR * DAILY_ATR_MIN_FACTOR)
		pMaxATR = minATR * DAILY_ATR_MIN_FACTOR;
	else if (pMaxATR > minATR * DAILY_ATR_MAX_FACTOR)
		pMaxATR = minATR * DAILY_ATR_MAX_FACTOR;

	// Choose predicted ATR based on threshold
	if (pMinATR > minATR * DAILY_ATR_THRESHOLD)
		pATR = pMinATR;
	else
		pATR = (pMinATR + pMaxATR) / 2.0;

	pATR = min(pATR, minATR);

	// Get average ATR on the same weekday (5, 10, 15, 20 days ago)
	pATRSameWeekDay = (iAtr(B_DAILY_RATES, 1, 5) + iAtr(B_DAILY_RATES, 1, 10) + 
	                   iAtr(B_DAILY_RATES, 1, 15) + iAtr(B_DAILY_RATES, 1, 20)) / DAILY_SAME_WEEKDAY_OFFSETS;

	logDebug("System InstanceID = %d, BarTime = %s, pATR = %f, pATRSameWeekDay = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameWeekDay);

	// Apply same weekday constraint
	pATR = min(pATR, pATRSameWeekDay);

	logDebug("System InstanceID = %d, BarTime = %s, pATR = %f, pATRSameWeekDay = %f, ATR0 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameWeekDay, ATR0);

	// Calculate predicted high/low based on last close and intraday prices
	pLow = max(lastDailyClose, intradayHigh) - pMaxATR;
	pHigh = min(lastDailyClose, intradayLow) + pMaxATR;

	// Constrain predictions within daily S2/R2 pivot levels
	if (pLow < pIndicators->dailyS2)
		pLow = pIndicators->dailyS2;

	if (pHigh > pIndicators->dailyR2)
		pHigh = pIndicators->dailyR2;

	// Set initial predicted values
	pIndicators->pDailyATR = pATR;
	pIndicators->pDailyPredictATR = pATR;
	pIndicators->pDailyHigh = pHigh;
	pIndicators->pDailyLow = pLow;

	// Determine trend based on current ATR vs predicted ATR
	if (ATR0 <= pATR)
	{
		// Volume in range - current volatility is within predicted range
		pIndicators->pDailyTrend = VOL_IN_RANGE;
	}
	else
	{
		// Volume breakout - current volatility exceeds prediction
		pIndicators->pDailyATR = ATR0;
		pIndicators->pDailyHigh = intradayHigh;
		pIndicators->pDailyLow = intradayLow;
		
		if (intradayClose > lastDailyClose)
			pIndicators->pDailyTrend = VOL_BREAK_UP;
		else
			pIndicators->pDailyTrend = VOL_BREAK_DOWN;
	}

	// Calculate maximum predicted high/low
	if (pIndicators->pDailyLow > pIndicators->dailyS2)
		pMaxLow = pIndicators->dailyS2;
	else
		pMaxLow = max(lastDailyClose, intradayHigh) - pATR;

	if (pIndicators->pDailyHigh < pIndicators->dailyR2)
		pMaxHigh = pIndicators->dailyR2;
	else
		pMaxHigh = min(lastDailyClose, intradayLow) + pATR;

	pIndicators->pMaxDailyHigh = pMaxHigh;
	pIndicators->pMaxDailyLow = pMaxLow;

	logDebug("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %f, pDailyATR = %f, pDailyHigh = %f, pDailyLow = %f, pDailyTrend = %ld, pMaxDailyHigh = %lf, pMaxDailyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pDailyPredictATR, pIndicators->pDailyATR, pIndicators->pDailyHigh, pIndicators->pDailyLow, pIndicators->pDailyTrend, pIndicators->pMaxDailyHigh, pIndicators->pMaxDailyLow);
}

/**
 * Predicts the weekly ATR (Average True Range) and calculates predicted high/low prices.
 * 
 * Algorithm:
 * 1. Calculates short (2), medium (4), and long (8) period weekly ATRs
 * 2. Predicts ATR using 4-week average minus recent ATR values
 * 3. Applies constraints to keep prediction within reasonable bounds
 * 4. Uses same month week ATR average as additional constraint
 * 5. Calculates predicted high/low based on last week close and intraweek prices
 * 6. Constrains predictions within weekly S2/R2 pivot levels
 * 7. Determines trend direction (VOL_IN_RANGE, VOL_BREAK_UP, VOL_BREAK_DOWN)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate with predictions
 * 
 * Output fields set in pIndicators:
 * - pWeeklyPredictMaxATR: Maximum predicted ATR
 * - pWeeklyATR, pWeeklyPredictATR: Predicted ATR
 * - pWeeklyHigh, pWeeklyLow: Predicted high/low prices
 * - pMaxWeeklyHigh, pMaxWeeklyLow: Maximum predicted high/low
 * - pWeeklyTrend: Trend classification (VOL_IN_RANGE, VOL_BREAK_UP, VOL_BREAK_DOWN)
 */
void predictWeeklyATR(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double shortWeeklyATR, mediumWeeklyATR, longWeeklyATR;
	double ATR0, ATR1, ATR2, ATR3;
	double minATR = INIT_MIN_ATR;
	double maxATR = INIT_MAX_ATR;
	double pMinATR, pMaxATR, pATRSameMonthWeek;
	double pATR;
	double pMaxLow = INIT_MAX_ATR;
	double pMaxHigh = INIT_MIN_ATR;
	double pLow = INIT_MAX_ATR;
	double pHigh = INIT_MIN_ATR;
	
	// Get current and previous week prices
	double lastWeekClose = iClose(B_WEEKLY_RATES, 1);
	double intraWeekClose = iClose(B_WEEKLY_RATES, 0);
	double intraWeekHigh = iHigh(B_WEEKLY_RATES, 0);
	double intraWeekLow = iLow(B_WEEKLY_RATES, 0);

	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);

	// Get recent ATR values (current and 3 previous weeks)
	ATR0 = iAtr(B_WEEKLY_RATES, 1, 0);
	ATR1 = iAtr(B_WEEKLY_RATES, 1, 1);
	ATR2 = iAtr(B_WEEKLY_RATES, 1, 2);
	ATR3 = iAtr(B_WEEKLY_RATES, 1, 3);

	// Calculate short, medium, and long-term weekly ATR averages
	shortWeeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_SHORT_ATR_PERIOD, 1);
	mediumWeeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_MEDIUM_ATR_PERIOD, 1);
	longWeeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_LONG_ATR_PERIOD, 1);

	// Find min and max ATR across all periods
	minATR = min(minATR, shortWeeklyATR);
	minATR = min(minATR, mediumWeeklyATR);
	minATR = min(minATR, longWeeklyATR);

	maxATR = max(maxATR, shortWeeklyATR);
	maxATR = max(maxATR, mediumWeeklyATR);
	maxATR = max(maxATR, longWeeklyATR);
	
	pIndicators->pWeeklyPredictMaxATR = maxATR;

	logDebug("System InstanceID = %d, BarTime = %s, shortWeeklyATR = %f, mediumWeeklyATR = %f, longWeeklyATR = %f, minATR = %f, maxATR = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyATR, mediumWeeklyATR, longWeeklyATR, minATR, maxATR);

	logDebug("System InstanceID = %d, BarTime = %s, ATR0 = %f, ATR1 = %f, ATR2 = %f, ATR3 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, ATR1, ATR2, ATR3);

	// Predict ATR using 4-week average minus recent ATR values
	pMinATR = minATR * WEEKLY_ATR_PERIODS - ATR1 - ATR2 - ATR3;
	pMaxATR = maxATR * WEEKLY_ATR_PERIODS - ATR1 - ATR2 - ATR3;

	// Constrain predicted ATR within reasonable bounds
	if (pMinATR < minATR * WEEKLY_ATR_MIN_FACTOR)
		pMinATR = minATR * WEEKLY_ATR_MIN_FACTOR;
	else if (pMinATR > minATR * DAILY_ATR_MAX_FACTOR)
		pMinATR = minATR * DAILY_ATR_MAX_FACTOR;

	if (pMaxATR < minATR * WEEKLY_ATR_MIN_FACTOR)
		pMaxATR = minATR * WEEKLY_ATR_MIN_FACTOR;
	else if (pMaxATR > minATR * DAILY_ATR_MAX_FACTOR)
		pMaxATR = minATR * DAILY_ATR_MAX_FACTOR;

	// Choose predicted ATR based on threshold
	if (pMinATR > minATR * WEEKLY_ATR_THRESHOLD)
		pATR = pMinATR;
	else
		pATR = (pMinATR + pMaxATR) / 2.0;
		
	logDebug("System InstanceID = %d, BarTime = %s, pATR = %f, ATR0 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, ATR0);

	// Get average ATR on the same month week (4 and 8 weeks ago)
	pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8)) / WEEKLY_SAME_MONTH_OFFSETS_SHORT;

	pATR = min(pATR, pATRSameMonthWeek);
	pATR = min(pATR, minATR);

	logDebug("System InstanceID = %d, BarTime = %s, pATR = %f, pATRSameMonthWeek = %f, ATR0 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameMonthWeek, ATR0);

	// Calculate predicted high/low based on last week close and intraweek prices
	pLow = max(lastWeekClose, intraWeekHigh) - pATR;
	pHigh = min(lastWeekClose, intraWeekLow) + pATR;

	// Constrain predictions within weekly S2/R2 pivot levels
	if (pLow < pIndicators->weeklyS2)
		pLow = pIndicators->weeklyS2;

	if (pHigh > pIndicators->weeklyR2)
		pHigh = pIndicators->weeklyR2;

	// Set initial predicted values
	pIndicators->pWeeklyPredictATR = pATR;
	pIndicators->pWeeklyATR = pATR;
	pIndicators->pWeeklyHigh = pHigh;
	pIndicators->pWeeklyLow = pLow;

	// Determine trend based on current ATR vs predicted ATR
	if (ATR0 <= pATR)
	{
		// Volume in range - current volatility is within predicted range
		pIndicators->pWeeklyTrend = VOL_IN_RANGE;
	}
	else
	{
		// Volume breakout - current volatility exceeds prediction
		pIndicators->pWeeklyATR = ATR0;
		pIndicators->pWeeklyHigh = intraWeekHigh;
		pIndicators->pWeeklyLow = intraWeekLow;
		
		if (intraWeekClose > lastWeekClose)
			pIndicators->pWeeklyTrend = VOL_BREAK_UP;
		else
			pIndicators->pWeeklyTrend = VOL_BREAK_DOWN;
	}
	
	// Calculate maximum predicted high/low
	pMaxLow = min(pIndicators->pWeeklyLow, pIndicators->weeklyS2);
	pMaxHigh = max(pIndicators->pWeeklyHigh, pIndicators->weeklyR2);

	pIndicators->pMaxWeeklyHigh = pMaxHigh;
	pIndicators->pMaxWeeklyLow = pMaxLow;

	logDebug("System InstanceID = %d, BarTime = %s, pWeeklyPredictATR = %f, pWeeklyATR = %f, pWeeklyHigh = %f, pWeeklyLow = %f, pWeeklyTrend = %ld, pMaxWeeklyHigh = %f, pMaxWeeklyLow = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pWeeklyPredictATR, pIndicators->pWeeklyATR, pIndicators->pWeeklyHigh, pIndicators->pWeeklyLow, pIndicators->pWeeklyTrend, pIndicators->pMaxWeeklyHigh, pIndicators->pMaxWeeklyLow);
}

/**
 * Predicts the weekly ATR for longer-term analysis using a 16-week period.
 * 
 * Similar to predictWeeklyATR() but uses a longer-term ATR period (16 weeks)
 * and calculates same month week average using 4 data points instead of 2.
 * 
 * Algorithm:
 * 1. Calculates short (2), medium (4), and long (16) period weekly ATRs
 * 2. Predicts ATR using 4-week average minus recent ATR values
 * 3. Applies constraints to keep prediction within reasonable bounds
 * 4. Uses same month week ATR average (4, 8, 12, 16 weeks ago) as constraint
 * 5. Calculates predicted high/low based on last week close and intraweek prices
 * 6. Constrains predictions within weekly S2/R2 pivot levels
 * 7. Determines trend direction (VOL_IN_RANGE, VOL_BREAK_UP, VOL_BREAK_DOWN)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate with predictions
 * 
 * Output fields set in pIndicators:
 * - pWeeklyPredictMaxATR: Maximum predicted ATR
 * - pWeeklyATR, pWeeklyPredictATR: Predicted ATR
 * - pWeeklyHigh, pWeeklyLow: Predicted high/low prices
 * - pMaxWeeklyHigh, pMaxWeeklyLow: Maximum predicted high/low
 * - pWeeklyTrend: Trend classification (VOL_IN_RANGE, VOL_BREAK_UP, VOL_BREAK_DOWN)
 */
void predictWeeklyATR_LongerTerm(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double shortWeeklyATR, mediumWeeklyATR, longWeeklyATR;
	double ATR0, ATR1, ATR2, ATR3;
	double minATR = INIT_MIN_ATR;
	double maxATR = INIT_MAX_ATR;
	double pMinATR, pMaxATR, pATRSameMonthWeek;
	double pATR;
	double pMaxLow = INIT_MAX_ATR;
	double pMaxHigh = INIT_MIN_ATR;
	double pLow = INIT_MAX_ATR;
	double pHigh = INIT_MIN_ATR;
	
	// Get current and previous week prices
	double lastWeekClose = iClose(B_WEEKLY_RATES, 1);
	double intraWeekClose = iClose(B_WEEKLY_RATES, 0);
	double intraWeekHigh = iHigh(B_WEEKLY_RATES, 0);
	double intraWeekLow = iLow(B_WEEKLY_RATES, 0);

	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);

	// Get recent ATR values (current and 3 previous weeks)
	ATR0 = iAtr(B_WEEKLY_RATES, 1, 0);
	ATR1 = iAtr(B_WEEKLY_RATES, 1, 1);
	ATR2 = iAtr(B_WEEKLY_RATES, 1, 2);
	ATR3 = iAtr(B_WEEKLY_RATES, 1, 3);

	// Calculate short, medium, and longer-term weekly ATR averages
	shortWeeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_SHORT_ATR_PERIOD, 1);
	mediumWeeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_MEDIUM_ATR_PERIOD, 1);
	longWeeklyATR = iAtr(B_WEEKLY_RATES, WEEKLY_LONGER_TERM_ATR_PERIOD, 1);

	// Find min and max ATR across all periods
	minATR = min(minATR, shortWeeklyATR);
	minATR = min(minATR, mediumWeeklyATR);
	minATR = min(minATR, longWeeklyATR);

	maxATR = max(maxATR, shortWeeklyATR);
	maxATR = max(maxATR, mediumWeeklyATR);
	maxATR = max(maxATR, longWeeklyATR);

	pIndicators->pWeeklyPredictMaxATR = maxATR;

	logDebug("System InstanceID = %d, BarTime = %s, shortWeeklyATR = %f, mediumWeeklyATR = %f, longWeeklyATR = %f, minATR = %f, maxATR = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyATR, mediumWeeklyATR, longWeeklyATR, minATR, maxATR);

	logDebug("System InstanceID = %d, BarTime = %s, ATR0 = %f, ATR1 = %f, ATR2 = %f, ATR3 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, ATR1, ATR2, ATR3);

	// Predict ATR using 4-week average minus recent ATR values
	pMinATR = minATR * WEEKLY_ATR_PERIODS - ATR1 - ATR2 - ATR3;
	pMaxATR = maxATR * WEEKLY_ATR_PERIODS - ATR1 - ATR2 - ATR3;

	// Constrain predicted ATR within reasonable bounds
	if (pMinATR < minATR * WEEKLY_ATR_MIN_FACTOR)
		pMinATR = minATR * WEEKLY_ATR_MIN_FACTOR;
	else if (pMinATR > minATR * DAILY_ATR_MAX_FACTOR)
		pMinATR = minATR * DAILY_ATR_MAX_FACTOR;

	if (pMaxATR < minATR * WEEKLY_ATR_MIN_FACTOR)
		pMaxATR = minATR * WEEKLY_ATR_MIN_FACTOR;
	else if (pMaxATR > minATR * DAILY_ATR_MAX_FACTOR)
		pMaxATR = minATR * DAILY_ATR_MAX_FACTOR;

	// Choose predicted ATR based on threshold
	if (pMinATR > minATR * WEEKLY_ATR_THRESHOLD)
		pATR = pMinATR;
	else
		pATR = (pMinATR + pMaxATR) / 2.0;

	logDebug("System InstanceID = %d, BarTime = %s, pATR = %f, ATR0 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, ATR0);

	// Get average ATR on the same month week (4, 8, 12, 16 weeks ago)
	pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) + 
	                     iAtr(B_WEEKLY_RATES, 1, 12) + iAtr(B_WEEKLY_RATES, 1, 16)) / WEEKLY_SAME_MONTH_OFFSETS_LONG;

	pATR = min(pATR, pATRSameMonthWeek);
	pATR = min(pATR, minATR);

	logDebug("System InstanceID = %d, BarTime = %s, pATR = %f, pATRSameMonthWeek = %f, ATR0 = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameMonthWeek, ATR0);

	// Calculate predicted high/low based on last week close and intraweek prices
	pLow = max(lastWeekClose, intraWeekHigh) - pATR;
	pHigh = min(lastWeekClose, intraWeekLow) + pATR;

	// Constrain predictions within weekly S2/R2 pivot levels
	if (pLow < pIndicators->weeklyS2)
		pLow = pIndicators->weeklyS2;

	if (pHigh > pIndicators->weeklyR2)
		pHigh = pIndicators->weeklyR2;

	// Set initial predicted values
	pIndicators->pWeeklyPredictATR = pATR;
	pIndicators->pWeeklyATR = pATR;
	pIndicators->pWeeklyHigh = pHigh;
	pIndicators->pWeeklyLow = pLow;

	// Determine trend based on current ATR vs predicted ATR
	if (ATR0 <= pATR)
	{
		// Volume in range - current volatility is within predicted range
		pIndicators->pWeeklyTrend = VOL_IN_RANGE;
	}
	else
	{
		// Volume breakout - current volatility exceeds prediction
		pIndicators->pWeeklyATR = ATR0;
		pIndicators->pWeeklyHigh = intraWeekHigh;
		pIndicators->pWeeklyLow = intraWeekLow;
		
		if (intraWeekClose > lastWeekClose)
			pIndicators->pWeeklyTrend = VOL_BREAK_UP;
		else
			pIndicators->pWeeklyTrend = VOL_BREAK_DOWN;
	}

	// Calculate maximum predicted high/low
	pMaxLow = min(pIndicators->pWeeklyLow, pIndicators->weeklyS2);
	pMaxHigh = max(pIndicators->pWeeklyHigh, pIndicators->weeklyR2);

	pIndicators->pMaxWeeklyHigh = pMaxHigh;
	pIndicators->pMaxWeeklyLow = pMaxLow;

	logDebug("System InstanceID = %d, BarTime = %s, pWeeklyPredictATR = %f, pWeeklyATR = %f, pWeeklyHigh = %f, pWeeklyLow = %f, pWeeklyTrend = %ld, pMaxWeeklyHigh = %f, pMaxWeeklyLow = %f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pWeeklyPredictATR, pIndicators->pWeeklyATR, pIndicators->pWeeklyHigh, pIndicators->pWeeklyLow, pIndicators->pWeeklyTrend, pIndicators->pMaxWeeklyHigh, pIndicators->pMaxWeeklyLow);
}
