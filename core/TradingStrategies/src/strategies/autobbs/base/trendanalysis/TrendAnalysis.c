/*
 * Trend Analysis Module
 * 
 * Provides functions for analyzing trends using various methods:
 * - High/Low trend analysis: Compares highs, lows, and closes between periods
 * - Moving Average trend analysis: Uses MA crossovers with ATR thresholds
 * - Three Rules trend analysis: Uses box pattern (min/max) to determine trend
 * 
 * Trend detection is fundamental to trading strategies as it helps identify
 * market direction and potential reversal points.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/trendanalysis/TrendAnalysis.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// Trend analysis constants
#define MA_SHORT_PERIOD 50                 // Short-term moving average period
#define MA_LONG_PERIOD 200                 // Long-term moving average period
#define MA_MODE_SMA 3                      // Simple moving average mode
#define ATR_PERIOD_FOR_MA 20               // ATR period for MA trend calculations
#define ATR_THRESHOLD_FACTOR 0.5           // ATR threshold factor (50% of ATR)
#define MAX_LOOKBACK_BARS 100              // Maximum bars for lookback analysis
#define MAX_SIGNAL_BARS 24                 // Maximum bars for signal detection
#define WEEKLY_MA_SHORT_PERIOD 1           // Short MA period for weekly bars (4H timeframe)
#define WEEKLY_MA_LONG_PERIOD 6            // Long MA period for weekly bars (4H timeframe)
#define DAILY_MA_SHORT_PERIOD 2            // Short MA period for daily bars (1H timeframe)
#define DAILY_MA_LONG_PERIOD 8             // Long MA period for daily bars (1H timeframe)

/**
 * Calculates High/Low trend for previous days.
 * 
 * Compares highs, lows, and closes between two previous periods to determine
 * trend direction. Uses weak trend detection (UP_WEAK/DOWN_WEAK) based on
 * high/low and close comparisons.
 * 
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param trend Output parameter: pointer to store trend direction
 * @param preDays Number of days to look back
 * @param index Index parameter: 1 = current day (EOD), 0 = previous day (SOD)
 * @return SUCCESS on success
 * 
 * Trend values:
 * - RANGE: No clear trend
 * - UP_WEAK: Weak uptrend (higher high and higher close)
 * - DOWN_WEAK: Weak downtrend (lower low and lower close)
 */
AsirikuyReturnCode iTrend_HL_preDays(int ratesArrayIndex, int *trend, int preDays, int index)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	
	// Get high, low, and close for two previous periods
	preHigh1 = iHigh(ratesArrayIndex, 1 + preDays - index);
	preHigh2 = iHigh(ratesArrayIndex, 2 + preDays - index);
	preClose1 = iClose(ratesArrayIndex, 1 + preDays - index);
	preClose2 = iClose(ratesArrayIndex, 2 + preDays - index);
	preLow1 = iLow(ratesArrayIndex, 1 + preDays - index);
	preLow2 = iLow(ratesArrayIndex, 2 + preDays - index);

	// Weak uptrend: higher high and higher close
	if (preHigh1 > preHigh2 && preClose1 > preClose2)
	{
		*trend = UP_WEAK;
	}

	// Weak downtrend: lower low and lower close
	if (preLow1 < preLow2 && preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}

	return SUCCESS;
}

/**
 * Calculates High/Low trend based on current or previous day.
 * 
 * Compares highs, lows, and closes between two periods to determine trend.
 * Uses weak trend detection based on high/low and close comparisons.
 * 
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param trend Output parameter: pointer to store trend direction
 * @param index Index parameter: 1 = current day (EOD), 0 = previous day (SOD)
 * @return SUCCESS on success
 * 
 * Trend values:
 * - RANGE: No clear trend
 * - UP_WEAK: Weak uptrend (higher high and higher close)
 * - DOWN_WEAK: Weak downtrend (lower low and lower close)
 * 
 * Note: The algorithm checks if the high is higher and close is higher for uptrend,
 * or if the low is lower and close is lower for downtrend.
 */
AsirikuyReturnCode iTrend_HL(int ratesArrayIndex, int *trend, int index)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	
	// Get high, low, and close for two periods
	preHigh1 = iHigh(ratesArrayIndex, 1 - index);
	preHigh2 = iHigh(ratesArrayIndex, 2 - index);
	preClose1 = iClose(ratesArrayIndex, 1 - index);
	preClose2 = iClose(ratesArrayIndex, 2 - index);
	preLow1 = iLow(ratesArrayIndex, 1 - index);
	preLow2 = iLow(ratesArrayIndex, 2 - index);
	
	// Weak uptrend: higher high and higher close
	if (preHigh1 > preHigh2 && preClose1 > preClose2)
	{
		*trend = UP_WEAK;
	}
	
	// Weak downtrend: lower low and lower close
	if (preLow1 < preLow2 && preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}
		
	return SUCCESS;
}

/**
 * Detects moving average trend signal by looking for crossovers.
 * 
 * Searches backwards through bars to find when the MA trend changed direction.
 * Returns 1 for bullish crossover (MA trend changed from negative to positive),
 * -1 for bearish crossover (MA trend changed from positive to negative),
 * or 0 if no crossover found within the search period.
 * 
 * @param rateShort Short-term MA period
 * @param rateLong Long-term MA period
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param maxBars Maximum number of bars to search back
 * @return 1 for bullish crossover, -1 for bearish crossover, 0 if no crossover
 */
int getMATrend_SignalBase(int rateShort, int rateLong, int ratesArrayIndex, int maxBars)
{
	int i = 0;
	int maTrend, maTrend_Prev;
	double adjust = iAtr(ratesArrayIndex, ATR_PERIOD_FOR_MA, 1);

	// Get current MA trend
	maTrend = getMATrendBase(rateShort, rateLong, adjust, ratesArrayIndex, 1);
	if (maTrend == 0)
		return 0;

	// Search backwards for trend change
	for (i = 1; i < maxBars; i++)
	{
		maTrend_Prev = getMATrendBase(rateShort, rateLong, adjust, ratesArrayIndex, i + 1);
		
		// Bullish crossover: trend changed from negative to positive
		if (maTrend > 0 && maTrend_Prev < 0)
		{			
			return 1;
		}

		// Bearish crossover: trend changed from positive to negative
		if (maTrend < 0 && maTrend_Prev > 0)
		{
			return -1;
		}
	}
		
	return 0;
}

/**
 * Detects MA trend signal using default periods (50/200) and 24-bar lookback.
 * 
 * @param ratesArrayIndex Index of the rates buffer to use
 * @return 1 for bullish crossover, -1 for bearish crossover, 0 if no crossover
 */
int getMATrend_Signal(int ratesArrayIndex)
{
	return getMATrend_SignalBase(MA_SHORT_PERIOD, MA_LONG_PERIOD, ratesArrayIndex, MAX_SIGNAL_BARS);
}

/**
 * Base function for calculating moving average trend.
 * 
 * Compares short-term and long-term moving averages. If the difference exceeds
 * the ATR threshold, returns a strong trend signal (2 or -2), otherwise returns
 * a weak trend signal (1 or -1).
 * 
 * @param rateShort Short-term MA period
 * @param rateLong Long-term MA period
 * @param iATR ATR value to use as threshold
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param index Bar index to calculate trend for
 * @return Trend value: 2 = strong uptrend, 1 = weak uptrend, 0 = range, -1 = weak downtrend, -2 = strong downtrend
 */
int getMATrendBase(int rateShort, int rateLong, double iATR, int ratesArrayIndex, int index)
{
	double maShort, maLong;
	int trend;

	maShort = iMA(MA_MODE_SMA, ratesArrayIndex, rateShort, index);
	maLong = iMA(MA_MODE_SMA, ratesArrayIndex, rateLong, index);

	if (maShort > maLong)
	{
		trend = 1;  // Weak uptrend
		// Strong uptrend if difference exceeds ATR threshold
		if (maShort - maLong >= iATR)
			trend = 2;
	}
	else if (maShort < maLong)
	{
		trend = -1;  // Weak downtrend
		// Strong downtrend if difference exceeds ATR threshold
		if (maLong - maShort >= iATR)
			trend = -2;
	}
	else
	{
		trend = 0;  // Range (MAs are equal)
	}
	
	return trend;
}

/**
 * Calculates MA trend using default periods (50/200).
 * 
 * @param iATR ATR value to use as threshold
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param index Bar index to calculate trend for
 * @return Trend value: 2 = strong uptrend, 1 = weak uptrend, 0 = range, -1 = weak downtrend, -2 = strong downtrend
 */
int getMATrend(double iATR, int ratesArrayIndex, int index)
{
	return getMATrendBase(MA_SHORT_PERIOD, MA_LONG_PERIOD, iATR, ratesArrayIndex, index);
}

/**
 * Calculates moving average trend using 50/200 MAs with ATR threshold.
 * 
 * Determines trend based on the relationship between 50-period and 200-period
 * moving averages. If the difference exceeds 50% of ATR, a trend is detected.
 * 
 * @param iATR ATR value to use as threshold
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param trend Output parameter: pointer to store trend direction
 * @return SUCCESS on success
 * 
 * Trend values:
 * - RANGE: No clear trend (MA difference < 50% of ATR)
 * - UP_NORMAL: Uptrend (MA50 > MA200 and difference > 50% of ATR)
 * - DOWN_NORMAL: Downtrend (MA200 > MA50 and difference > 50% of ATR)
 */
AsirikuyReturnCode iTrend_MA(double iATR, int ratesArrayIndex, int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(MA_MODE_SMA, ratesArrayIndex, MA_SHORT_PERIOD, 1);
	ma200M = iMA(MA_MODE_SMA, ratesArrayIndex, MA_LONG_PERIOD, 1);
	
	// Uptrend: MA50 > MA200 and difference > 50% of ATR
	if (ma50M - ma200M > ATR_THRESHOLD_FACTOR * iATR)
		*trend = UP_NORMAL;

	// Downtrend: MA200 > MA50 and difference > 50% of ATR
	if (ma200M - ma50M > ATR_THRESHOLD_FACTOR * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

/**
 * Looks back through bars to find when MA trend changed.
 * 
 * Searches backwards to find the first bar where the MA trend doesn't match
 * the current signal. Useful for determining how long a trend has been active.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param signal Current trend signal (positive for uptrend, negative for downtrend)
 * @return Index of the bar where trend changed, or 100 if trend persisted throughout search
 */
int iTrendMA_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int signal)
{
	int trend[MAX_LOOKBACK_BARS] = { 0 };
	int i = 0;
	int turningIndex = MAX_LOOKBACK_BARS;

	// Calculate MA trend for each bar going backwards
	for (i = 1; i < MAX_LOOKBACK_BARS; i++)
	{
		trend[i] = getMATrend(iAtr(ratesArrayIndex, ATR_PERIOD_FOR_MA, 1), ratesArrayIndex, i);
		
		// Check if trend doesn't match signal
		if ((signal > 0 && trend[i] != UP_NORMAL) ||
			(signal < 0 && trend[i] != DOWN_NORMAL))
		{
			turningIndex = i;
			break;
		}
	}

	return turningIndex;
}

/**
 * Calculates MA trend for weekly bars using 4H timeframe.
 * 
 * Uses weekly bar data with shorter MA periods (1 and 6) to calculate trend
 * for 4-hour timeframe analysis. This is a specialized function for weekly
 * trend analysis on lower timeframes.
 * 
 * @param iATR ATR value to use as threshold
 * @param trend Output parameter: pointer to store trend direction
 * @return SUCCESS on success
 * 
 * Trend values:
 * - RANGE: No clear trend
 * - UP_NORMAL: Uptrend (MA1 > MA6 and difference > 50% of ATR)
 * - DOWN_NORMAL: Downtrend (MA6 > MA1 and difference > 50% of ATR)
 */
AsirikuyReturnCode iTrend_MA_WeeklyBar_For4H(double iATR, int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(MA_MODE_SMA, B_WEEKLY_RATES, WEEKLY_MA_SHORT_PERIOD, 1);
	ma200M = iMA(MA_MODE_SMA, B_WEEKLY_RATES, WEEKLY_MA_LONG_PERIOD, 1);
	
	// Uptrend: short MA > long MA and difference > 50% of ATR
	if (ma50M - ma200M > ATR_THRESHOLD_FACTOR * iATR)
		*trend = UP_NORMAL;

	// Downtrend: long MA > short MA and difference > 50% of ATR
	if (ma200M - ma50M > ATR_THRESHOLD_FACTOR * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

/**
 * Calculates MA trend for daily bars using 1H timeframe.
 * 
 * Uses daily bar data with shorter MA periods (2 and 8) to calculate trend
 * for 1-hour timeframe analysis. Supports EOD/SOD index parameter.
 * 
 * @param iATR ATR value to use as threshold
 * @param trend Output parameter: pointer to store trend direction
 * @param index Index parameter: 1 = current day (EOD), 0 = previous day (SOD)
 * @return SUCCESS on success
 * 
 * Trend values:
 * - RANGE: No clear trend
 * - UP_NORMAL: Uptrend (MA2 > MA8 and difference > 50% of ATR)
 * - DOWN_NORMAL: Downtrend (MA8 > MA2 and difference > 50% of ATR)
 */
AsirikuyReturnCode iTrend_MA_DailyBar_For1H(double iATR, int *trend, int index)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(MA_MODE_SMA, B_DAILY_RATES, DAILY_MA_SHORT_PERIOD, 1 - index);
	ma200M = iMA(MA_MODE_SMA, B_DAILY_RATES, DAILY_MA_LONG_PERIOD, 1 - index);
	
	// Uptrend: short MA > long MA and difference > 50% of ATR
	if (ma50M - ma200M > ATR_THRESHOLD_FACTOR * iATR)
		*trend = UP_NORMAL;

	// Downtrend: long MA > short MA and difference > 50% of ATR
	if (ma200M - ma50M > ATR_THRESHOLD_FACTOR * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

/**
 * Looks back through bars to find trend changes using Three Rules method.
 * 
 * Uses the Three Rules trend analysis method to look backwards and determine
 * when the trend changed. Calculates box patterns (min/max) for each period
 * and checks if closes break above or below these boxes.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param shift Number of periods for box calculation
 * @param pTrend Output parameter: pointer to store the trend at the lookback point
 * @return SUCCESS on success, error code on TALib failure
 */
AsirikuyReturnCode iTrend3Rules_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;
	double boxHigh[MAX_LOOKBACK_BARS] = { 0 };
	double boxLow[MAX_LOOKBACK_BARS] = { 0 };
	int trend[MAX_LOOKBACK_BARS] = { 0 };
	int i = 0;

	// Calculate minimum low (box low) for each period
	retCode = TA_MIN(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	// Calculate maximum high (box high) for each period
	retCode = TA_MAX(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	// Analyze trend for each period
	for (i = 0; i < outNBElement; i++)
	{
		trend[i] = RANGE;
		
		// Skip if invalid box values
		if (boxHigh[i] == 0 || boxLow[i] == 0)
			break;

		// Uptrend: close breaks above box high
		// Rule: Close > max(high1, high2) indicates uptrend
		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) > boxHigh[i])
			trend[i] = UP_NORMAL;

		// Downtrend: close breaks below box low
		// Rule: Close < min(low1, low2) indicates downtrend
		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) < boxLow[i])
			trend[i] = DOWN_NORMAL;

		// Maintain previous trend if current is range and previous was trending
		if (i - 1 >= 0 && trend[i - 1] == UP && trend[i] == RANGE)
			trend[i] = RANGE;

		if (i - 1 >= 0 && trend[i - 1] == DOWN && trend[i] == RANGE)
			trend[i] = RANGE;

		*pTrend = trend[i];
	}	

	return SUCCESS;
}

/**
 * Calculates Three Rules trend for previous days.
 * 
 * Uses box pattern (min/max of highs and lows) to determine trend direction.
 * If close breaks above the box high, trend is UP. If close breaks below
 * the box low, trend is DOWN.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param shift Number of periods for box calculation
 * @param pTrend Output parameter: pointer to store trend direction
 * @param preDays Number of days to look back
 * @param index Index parameter: 1 = current day (EOD), 0 = previous day (SOD)
 * @return SUCCESS on success, error code on TALib failure
 * 
 * Trend values:
 * - RANGE: Close is within the box (between min and max)
 * - UP: Close > max(highs) - uptrend
 * - DOWN: Close < min(lows) - downtrend
 */
AsirikuyReturnCode iTrend3Rules_preDays(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend, int preDays, int index)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 - preDays;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2 - preDays;
	int outBegIdx, outNBElement;
	double boxHigh, boxLow;

	// Calculate box low (minimum low) for the period
	retCode = TA_MIN(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	// Calculate box high (maximum high) for the period
	retCode = TA_MAX(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	// Determine trend based on close position relative to box
	*pTrend = RANGE;
	
	// Uptrend: close breaks above box high
	// Rule: Close > max(high1, high2) indicates uptrend
	if (iClose(ratesArrayIndex, 1 - index) > boxHigh)
		*pTrend = UP;

	// Downtrend: close breaks below box low
	// Rule: Close < min(low1, low2) indicates downtrend
	if (iClose(ratesArrayIndex, 1 - index) < boxLow)
		*pTrend = DOWN;

	return SUCCESS;
}

/**
 * Calculates Three Rules trend using box pattern analysis.
 * 
 * The "Three Rules" method uses a box pattern created by the minimum low
 * and maximum high over a specified period. The trend is determined by
 * whether the close price breaks above (uptrend) or below (downtrend) this box.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Base indicators structure (not used, kept for API consistency)
 * @param ratesArrayIndex Index of the rates buffer to use
 * @param shift Number of periods for box calculation (typically 2)
 * @param pTrend Output parameter: pointer to store trend direction
 * @param index Index parameter: 1 = current day (EOD), 0 = previous day (SOD)
 * @return SUCCESS on success, error code on TALib failure
 * 
 * Trend values:
 * - RANGE: Close is within the box (between min and max)
 * - UP: Close > max(highs) - uptrend breakout
 * - DOWN: Close < min(lows) - downtrend breakdown
 * 
 * Algorithm:
 * 1. Calculate box low = min(lows) over shift periods
 * 2. Calculate box high = max(highs) over shift periods
 * 3. Compare close to box:
 *    - If close > box high: UP trend
 *    - If close < box low: DOWN trend
 *    - Otherwise: RANGE
 */
AsirikuyReturnCode iTrend3Rules(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend, int index)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;
	double boxHigh, boxLow;

	// Calculate box low (minimum low) for the period
	retCode = TA_MIN(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	// Calculate box high (maximum high) for the period
	retCode = TA_MAX(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	// Determine trend based on close position relative to box
	*pTrend = RANGE;
	
	// Uptrend: close breaks above box high
	// Rule: Close > max(high1, high2) indicates uptrend
	if (iClose(ratesArrayIndex, 1 - index) > boxHigh)
		*pTrend = UP;

	// Downtrend: close breaks below box low
	// Rule: Close < min(low1, low2) indicates downtrend
	if (iClose(ratesArrayIndex, 1 - index) < boxLow)
		*pTrend = DOWN;

	return SUCCESS;
}
