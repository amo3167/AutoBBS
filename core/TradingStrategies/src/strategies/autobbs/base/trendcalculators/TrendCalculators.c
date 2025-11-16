/*
 * Trend Calculators Module
 * 
 * Provides functions for calculating daily and weekly trends,
 * including support/resistance levels and trend phases.
 * 
 * This module combines multiple trend indicators (3-rules, High/Low, MA)
 * to determine overall trend direction and phase, then calculates
 * appropriate support/resistance levels based on the trend state.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/trendcalculators/TrendCalculators.h"
#include "strategies/autobbs/base/supportresistance/SupportResistance.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// Support/Resistance validation constants
#define SUPPORT_MIN_FACTOR 0.5              // Minimum distance factor (50% of ATR)
#define SUPPORT_MAX_FACTOR 1.5              // Maximum distance factor (150% of ATR)
#define WEEKLY_ATR_FACTOR 0.666             // Weekly ATR factor (2/3 of ATR)
#define NO_RESISTANCE_MARKER -1.0           // Marker value indicating no resistance found

// Trend phase constants
#define STRONG_MA_TREND_UP 2                // Strong uptrend from MA
#define STRONG_MA_TREND_DOWN -2             // Strong downtrend from MA

/**
 * Validates if a distance represents a valid support or resistance level.
 * 
 * A valid support/resistance level must be within a reasonable distance
 * from the reference price. Too close (< 50% ATR) or too far (> 150% ATR)
 * are considered invalid.
 * 
 * Note: Despite the name "isValidSupport", this function is used for
 * both support and resistance validation.
 * 
 * @param position Distance from reference price to potential support/resistance
 * @param dailyATR Daily ATR value to use for validation
 * @return TRUE if position is between 50% and 150% of ATR, FALSE otherwise
 */
static BOOL isValidSupport(double position, double dailyATR)
{
	if (position <= SUPPORT_MAX_FACTOR * dailyATR && position > SUPPORT_MIN_FACTOR * dailyATR)
		return TRUE;

	return FALSE;
}

/**
 * Calculates daily trend, phase, and support/resistance levels.
 * 
 * Combines three trend indicators (3-rules, High/Low, MA) to determine:
 * - Overall daily trend (sum of all indicators)
 * - Trend phase (beginning, middle, retreat, range)
 * - Support and resistance levels based on trend direction
 * - Take profit levels
 * 
 * Algorithm:
 * 1. Calculate combined trend from 3-rules, HL, and MA trends
 * 2. Determine trend phase based on indicator combinations
 * 3. Calculate support/resistance based on trend direction:
 *    - Range phase: Use daily/weekly high/low or ATR-based levels
 *    - Uptrend: Support from daily low, MA, or previous low; Resistance from weekly/monthly highs
 *    - Downtrend: Support from daily high, MA, or previous high; Resistance from weekly/monthly lows
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success
 * 
 * Output fields set in pIndicators:
 * - dailyTrend: Combined trend value (sum of 3-rules, HL, and MA trends)
 * - dailyTrend_Phase: Trend phase (RANGE_PHASE, BEGINNING_UP_PHASE, etc.)
 * - dailyS: Daily support level
 * - dailyR: Daily resistance level (or -1 if no resistance found)
 * - dailyTP: Daily take profit level
 */
AsirikuyReturnCode workoutDailyTrend(StrategyParams* pParams, Base_Indicators* pIndicators)
{	
	double preLow = iLow(B_DAILY_RATES, 1);
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preClose = iClose(B_DAILY_RATES, 1);
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	// Calculate combined trend from all three indicators
	pIndicators->dailyTrend = pIndicators->daily3RulesTrend + pIndicators->dailyHLTrend + pIndicators->dailyMATrend;

	// Determine trend phase based on indicator combinations
	if (pIndicators->dailyMATrend == 0 && pIndicators->daily3RulesTrend == 0)
		pIndicators->dailyTrend_Phase = RANGE_PHASE;
	else if (pIndicators->daily3RulesTrend == UP && pIndicators->dailyMATrend != STRONG_MA_TREND_UP)
		pIndicators->dailyTrend_Phase = BEGINNING_UP_PHASE;
	else if (pIndicators->daily3RulesTrend == DOWN && pIndicators->dailyMATrend != STRONG_MA_TREND_DOWN)
		pIndicators->dailyTrend_Phase = BEGINNING_DOWN_PHASE;
	else if (pIndicators->dailyMATrend == STRONG_MA_TREND_UP && pIndicators->dailyHLTrend >= 0)
		pIndicators->dailyTrend_Phase = MIDDLE_UP_PHASE;
	else if (pIndicators->dailyMATrend == STRONG_MA_TREND_UP && pIndicators->dailyHLTrend < 0)
		pIndicators->dailyTrend_Phase = MIDDLE_UP_RETREAT_PHASE;
	else if (pIndicators->dailyMATrend == STRONG_MA_TREND_DOWN && pIndicators->dailyHLTrend <= 0)
		pIndicators->dailyTrend_Phase = MIDDLE_DOWN_PHASE;
	else if (pIndicators->dailyMATrend == STRONG_MA_TREND_DOWN && pIndicators->dailyHLTrend > 0)
		pIndicators->dailyTrend_Phase = MIDDLE_DOWN_RETREAT_PHASE;
	else
		pIndicators->dailyTrend_Phase = RANGE_PHASE;

	// Calculate support and resistance levels based on trend phase
	if (pIndicators->dailyTrend_Phase == RANGE_PHASE || pIndicators->dailyTrend == RANGE)
	{
		// Range phase: Use daily/weekly high/low or ATR-based levels
		
		// Support: Check daily low, weekly low, or ATR-based level
		if (isValidSupport(preClose - pIndicators->dailyLow, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->dailyLow;
		else if (isValidSupport(preClose - iLow(B_WEEKLY_RATES, 0), pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = iLow(B_WEEKLY_RATES, 0);
		else
			pIndicators->dailyS = preClose - pIndicators->dailyATR;

		// Resistance: Check daily high, weekly high, or ATR-based level
		if (isValidSupport(pIndicators->dailyHigh - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyR = pIndicators->dailyHigh;
		else if (isValidSupport(iHigh(B_WEEKLY_RATES, 0) - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyR = iHigh(B_WEEKLY_RATES, 0);
		else
			pIndicators->dailyR = preClose + pIndicators->dailyATR;

		pIndicators->dailyTP = pIndicators->dailyR2;
	}
	else if (pIndicators->dailyTrend > RANGE)
	{
		// Uptrend: Support from multiple sources, resistance from higher timeframes
		
		// Support: Priority order - daily low, MA200, previous low, current low, or ATR-based
		if (isValidSupport(preClose - pIndicators->dailyLow, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->dailyLow;
		else if (isValidSupport(preClose - pIndicators->ma1H200M, pIndicators->dailyATR) == TRUE)			
			pIndicators->dailyS = pIndicators->ma1H200M;		
		else if (isValidSupport(preClose - preLow, pIndicators->dailyATR) == TRUE)			
			pIndicators->dailyS = preLow;			
		else if (isValidSupport(pParams->bidAsk.ask[0] - currentLow, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = currentLow;
		else
			pIndicators->dailyS = preClose - pIndicators->dailyATR;		
		
		// Resistance: Priority order - weekly high, daily high, MA200, monthly high, or no resistance
		pIndicators->dailyR = pIndicators->weeklyHigh;
		if (preClose - pIndicators->dailyHigh < 0)
			pIndicators->dailyR = pIndicators->dailyHigh;
		else if (preClose - pIndicators->ma4H200M < 0)
			pIndicators->dailyR = min(pIndicators->weeklyHigh, pIndicators->ma4H200M);

		// Check monthly high if price is above weekly resistance
		if (preClose - pIndicators->dailyR > 0)
			pIndicators->dailyR = pIndicators->monthlyHigh;

		// No resistance found in two months - mark as -1
		if (preClose - pIndicators->dailyR > 0)
			pIndicators->dailyR = NO_RESISTANCE_MARKER;

		pIndicators->dailyTP = pIndicators->dailyR2;
	}
	else if (pIndicators->dailyTrend < RANGE)
	{
		// Downtrend: Support from higher levels, resistance from lower timeframes

		// Support: Priority order - daily high, MA200, previous high, current high, or ATR-based
		if (isValidSupport(pIndicators->dailyHigh - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->dailyHigh;
		else if (isValidSupport(pIndicators->ma1H200M - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->ma1H200M;
		else if (isValidSupport(preHigh - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = preHigh;
		else if (isValidSupport(currentHigh - pParams->bidAsk.bid[0], pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = currentHigh;
		else
			pIndicators->dailyS = preClose + pIndicators->dailyATR;

		// Resistance: Priority order - weekly low, daily low, MA200, monthly low, or no resistance
		pIndicators->dailyR = pIndicators->weeklyLow;
		if (preClose - pIndicators->dailyLow > 0)
			pIndicators->dailyR = pIndicators->dailyLow;
		else if (preClose - pIndicators->ma4H200M > 0)
			pIndicators->dailyR = max(pIndicators->weeklyLow, pIndicators->ma4H200M);
		
		// Check monthly low if price is below weekly resistance
		if (preClose - pIndicators->dailyR < 0)
			pIndicators->dailyR = pIndicators->monthlyLow;

		// No resistance found in two months - mark as -1
		if (preClose - pIndicators->dailyR < 0)
			pIndicators->dailyR = NO_RESISTANCE_MARKER;

		pIndicators->dailyTP = pIndicators->dailyS2;
	}

	return SUCCESS;
}

/**
 * Calculates weekly trend, phase, and support/resistance levels.
 * 
 * Similar to workoutDailyTrend() but for weekly timeframe. Combines
 * three trend indicators to determine weekly trend direction and phase,
 * then calculates appropriate support/resistance levels.
 * 
 * Algorithm:
 * 1. Determine trend phase based on indicator combinations
 * 2. Calculate combined trend from all indicators
 * 3. Calculate support/resistance based on trend direction:
 *    - Range phase: Use weekly high/low or ATR-based levels (using 2/3 ATR factor)
 *    - Uptrend: Support from weekly low or previous low; Resistance from monthly high
 *    - Downtrend: Support from weekly high or previous high; Resistance from monthly low
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure to populate
 * @return SUCCESS on success
 * 
 * Output fields set in pIndicators:
 * - weeklyTrend: Combined trend value (sum of 3-rules, HL, and MA trends)
 * - weeklyTrend_Phase: Trend phase (RANGE_PHASE, BEGINNING_UP_PHASE, etc.)
 * - weeklyS: Weekly support level
 * - weeklyR: Weekly resistance level (or -1 if no resistance found)
 * - weeklyTP: Weekly take profit level
 */
AsirikuyReturnCode workoutWeeklyTrend(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double preLow = iLow(B_WEEKLY_RATES, 1);
	double preHigh = iHigh(B_WEEKLY_RATES, 1);
	double preClose = iClose(B_WEEKLY_RATES, 1);
	double currentLow = iLow(B_WEEKLY_RATES, 0);
	double currentHigh = iHigh(B_WEEKLY_RATES, 0);

	// Determine trend phase based on indicator combinations
	if (pIndicators->weeklyMATrend == 0 && pIndicators->weekly3RulesTrend == 0)
		pIndicators->weeklyTrend_Phase = RANGE_PHASE;
	else if (pIndicators->weekly3RulesTrend == UP && pIndicators->weeklyMATrend != STRONG_MA_TREND_UP)
		pIndicators->weeklyTrend_Phase = BEGINNING_UP_PHASE;
	else if (pIndicators->weekly3RulesTrend == DOWN && pIndicators->weeklyMATrend != STRONG_MA_TREND_DOWN)
		pIndicators->weeklyTrend_Phase = BEGINNING_DOWN_PHASE;
	else if (pIndicators->weeklyMATrend == STRONG_MA_TREND_UP && pIndicators->weeklyHLTrend >= 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_UP_PHASE;
	else if (pIndicators->weeklyMATrend == STRONG_MA_TREND_UP && pIndicators->weeklyHLTrend < 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_UP_RETREAT_PHASE;
	else if (pIndicators->weeklyMATrend == STRONG_MA_TREND_DOWN && pIndicators->weeklyHLTrend <= 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_DOWN_PHASE;
	else if (pIndicators->weeklyMATrend == STRONG_MA_TREND_DOWN && pIndicators->weeklyHLTrend > 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_DOWN_RETREAT_PHASE;
	else
		pIndicators->weeklyTrend_Phase = RANGE_PHASE;

	// Calculate combined trend from all three indicators
	pIndicators->weeklyTrend = pIndicators->weekly3RulesTrend + pIndicators->weeklyHLTrend + pIndicators->weeklyMATrend;

	// Calculate support and resistance levels based on trend phase
	if (pIndicators->weeklyTrend_Phase == RANGE_PHASE || pIndicators->weeklyTrend == RANGE)
	{
		// Range phase: Use weekly high/low or ATR-based levels (using 2/3 ATR factor)

		// Support: Check weekly low or ATR-based level
		if (isValidSupport(preClose - pIndicators->weeklyLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = pIndicators->weeklyLow;
		else if (isValidSupport(iLow(B_WEEKLY_RATES, 0) - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = iLow(B_WEEKLY_RATES, 0);
		else
			pIndicators->weeklyS = preClose - pIndicators->weeklyATR * WEEKLY_ATR_FACTOR;

		// Resistance: Check weekly high or ATR-based level
		if (isValidSupport(pIndicators->weeklyHigh - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyR = pIndicators->weeklyHigh;
		else if (isValidSupport(iHigh(B_WEEKLY_RATES, 0) - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyR = iHigh(B_WEEKLY_RATES, 0);
		else
			pIndicators->weeklyR = preClose + pIndicators->weeklyATR * WEEKLY_ATR_FACTOR;

		pIndicators->weeklyTP = pIndicators->weeklyR2;
	}
	else if (pIndicators->weeklyTrend > RANGE)
	{
		// Uptrend: Support from weekly low or previous low; Resistance from monthly high

		// Support: Priority order - weekly low, previous low, current low, or ATR-based
		if (isValidSupport(preClose - pIndicators->weeklyLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = pIndicators->weeklyLow;		
		else if (isValidSupport(preClose - preLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = preLow;
		else if (isValidSupport(pParams->bidAsk.bid[0] - currentLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = currentLow;
		else
			pIndicators->weeklyS = preClose - pIndicators->weeklyATR * WEEKLY_ATR_FACTOR;

		// Resistance: Monthly high, or no resistance if price is above
		pIndicators->weeklyR = pIndicators->monthlyHigh;
		
		// No resistance found in two months - mark as -1
		if (preClose - pIndicators->weeklyR > 0)
			pIndicators->weeklyR = NO_RESISTANCE_MARKER;

		pIndicators->weeklyTP = pIndicators->weeklyR2;
	}
	else if (pIndicators->weeklyTrend < RANGE)
	{
		// Downtrend: Support from weekly high or previous high; Resistance from monthly low

		// Support: Priority order - weekly high, previous high, current high, or ATR-based
		if (isValidSupport(pIndicators->weeklyHigh - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = pIndicators->weeklyHigh;
		else if (isValidSupport(preHigh - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = preHigh;
		else if (isValidSupport(currentHigh - pParams->bidAsk.ask[0], pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = currentHigh;
		else
			pIndicators->weeklyS = preClose + pIndicators->weeklyATR * WEEKLY_ATR_FACTOR;

		// Resistance: Monthly low, or no resistance if price is below
		pIndicators->weeklyR = pIndicators->monthlyLow;

		// No resistance found in two months - mark as -1
		if (preClose - pIndicators->weeklyR < 0)
			pIndicators->weeklyR = NO_RESISTANCE_MARKER;

		pIndicators->weeklyTP = pIndicators->weeklyS2;
	}

	return SUCCESS;
}
