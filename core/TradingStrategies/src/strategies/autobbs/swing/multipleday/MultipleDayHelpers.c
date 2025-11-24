/**
 * @file MultipleDayHelpers.c
 * @brief Multiple-Day Trading Helpers Module
 * 
 * Provides helper functions for multiple-day trading strategies.
 * Includes trade filtering functions for GBPJPY, GBPUSD, XAUUSD, and other commodities.
 * 
 * @author AutoBBS Team
 */

#include <math.h>
#include <string.h>
#include <stdio.h>
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayHelpers.h"

/**
 * Check if GBPJPY multiple days trading is allowed.
 * 
 * Filtering criteria:
 * - Must be after start trading hour
 * - Asia session high/low must be calculated successfully
 * - Intraday ATR (H-L) must be <= 94% of euro ATR range
 * - Daily predicted ATR must be >= euro ATR range
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL GBPJPY_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count;
	int asia_index_rate;
	int execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1);
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->maxTradeTime = 1;

	execution_tf = (int)pParams->settings[TIMEFRAME];
	
	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf, pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	// Calculate Asia session index
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
		logInfo("System InstanceID = %d, BarTime = %s, asia_index_rate = %d, count = %d, asia_high = %lf, asia_low = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, asia_index_rate, count, pIndicators->asia_high, pIndicators->asia_low);
	}
	else
	{
		return FALSE;
	}

	// Update Asia session levels with previous day's close
	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf, asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	// Filter: Intraday ATR (H-L) must be <= 94% of euro ATR range
	if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range * 0.94)
	{
		sprintf(pIndicators->status, "Intraday ATR(H-L) %lf is greater than euro atr range %lf",
			fabs(pIndicators->asia_high - pIndicators->asia_low), pIndicators->atr_euro_range * 0.94);

		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	// Filter: Daily predicted ATR must be >= euro ATR range
	if (pBase_Indicators->pDailyPredictATR < (double)parameter(AUTOBBS_IS_ATREURO_RANGE))
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than euro atr range %lf",
			pBase_Indicators->pDailyPredictATR, (double)parameter(AUTOBBS_IS_ATREURO_RANGE));

		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	return TRUE;
}

/**
 * Check if GBPUSD multiple days trading is allowed.
 * 
 * Filtering criteria:
 * - Must be after start trading hour
 * - Asia session high/low must be calculated successfully
 * - Intraday ATR (H-L) must be <= 94% of euro ATR range
 * - Daily predicted ATR must be >= euro ATR range
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL GBPUSD_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count;
	int asia_index_rate;
	int execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1);
	int startTradingTime = pIndicators->startHour;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->maxTradeTime = 1;

	execution_tf = (int)pParams->settings[TIMEFRAME];

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	// Calculate Asia session index
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	}
	else
	{
		return FALSE;
	}

	// Update Asia session levels with previous day's close
	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf, asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	// Filter: Intraday ATR (H-L) must be <= 94% of euro ATR range
	if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range * 0.94)
		return FALSE;

	// Filter: Daily predicted ATR must be >= euro ATR range
	if (pBase_Indicators->pDailyPredictATR < (double)parameter(AUTOBBS_IS_ATREURO_RANGE))
		return FALSE;

	return TRUE;
}

/**
 * Check if commodity multiple days trading is allowed (XAUUSD, XAGUSD, etc.).
 * 
 * Enhanced filtering criteria for multiple-day strategies:
 * - Must be within trading hours (startHour to endHour)
 * - Filters out Non-Farm Payroll (NFP) days (first Friday of month)
 * - Filters out non-full trading days
 * - Daily predicted ATR must be >= euro ATR range
 * - Weekly ATR must be within predicted range
 * - Daily ATR must be < max(20, weeklyPredictATR/2)
 * - Daily close gap must be < max(10, weeklyPredictATR/3)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param shouldFilter If FALSE, skips ATR-based filtering
 * @return TRUE if trading is allowed, FALSE otherwise
 * 
 * @note Used by multiple-day strategies for commodity symbols (XAUUSD, XAGUSD)
 */
BOOL Commodity_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL shouldFilter)
{
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1);
	double close_prev2 = iClose(B_DAILY_RATES, 2);
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;
	double ATRDaily0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	
	execution_tf = (int)pParams->settings[TIMEFRAME];

	ATRDaily0 = iAtr(B_DAILY_RATES, 1, 0);

	// Filter: Must be within trading hours
	if (timeInfo1.tm_hour < startTradingTime || 
		(timeInfo1.tm_hour >= pIndicators->endHour))
	{
		return FALSE;
	}

	// Filter: Non-Farm Payroll (NFP) day (first Friday of month)
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	{
		strcpy(pIndicators->status, "Filter Non-farm day\n");

		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;	
	}

	// Filter: Skip non-full trading days
	if (XAUUSD_not_full_trading_day(pParams, pIndicators, pBase_Indicators) == TRUE)
	{
		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
		return FALSE;
	}

	// Skip ATR-based filtering if shouldFilter is FALSE
	if (shouldFilter == FALSE)
		return TRUE;

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf, ATRWeekly0 = %lf, pWeeklyPredictMaxATR = %lf, pWeeklyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR, pBase_Indicators->pWeeklyPredictATR);

	// Filter: Daily predicted ATR must be >= euro ATR range
	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than atr_euro_range %lf",
			pBase_Indicators->pDailyPredictATR, pIndicators->atr_euro_range);

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	// Filter: Weekly ATR must be within predicted range if daily ATR is low
	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 10.0)	
	{
		sprintf(pIndicators->status, "ATRWeekly0 %lf is greater than pWeeklyPredictMaxATR %lf and pDailyPredictATR < 10",
			ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR);

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	// Filter: Daily ATR must be < max(20, weeklyPredictATR/2)
	if (iAtr(B_DAILY_RATES, 1, 1) - max(20.0, pBase_Indicators->pWeeklyPredictATR / 2.0) >= 0)
	{
		sprintf(pIndicators->status, "ATR1 %lf is greater than half of pWeeklyPredictATR %lf",
			iAtr(B_DAILY_RATES, 1, 1), max(20.0, pBase_Indicators->pWeeklyPredictATR / 2.0));

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	// Filter: Daily close gap must be < max(10, weeklyPredictATR/3)
	if (fabs(close_prev1 - close_prev2) >= max(10.0, pBase_Indicators->pWeeklyPredictATR / 3.0))
	{
		sprintf(pIndicators->status, "Previous close gap %lf is greater than third of pWeeklyPredictATR %lf",
			fabs(close_prev1 - close_prev2), max(10.0, pBase_Indicators->pWeeklyPredictATR / 3.0));

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	return TRUE;
}
