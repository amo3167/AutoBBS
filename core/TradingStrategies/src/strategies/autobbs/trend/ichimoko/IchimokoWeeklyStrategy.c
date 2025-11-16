/*
 * Ichimoko Weekly Strategy Module
 * 
 * Provides Ichimoko Weekly strategy execution functions.
 * This strategy uses weekly support/resistance levels and price action
 * to determine entry signals for longer-term trend-following trades.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoWeeklyStrategy.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"

// Strategy configuration constants
#define SPLIT_TRADE_MODE_ICHIMOKO_WEEKLY 33 // Split trade mode for Ichimoko Weekly strategy
#define TP_MODE_DAILY_ATR 3                 // Take profit mode: daily ATR
#define TRADE_MODE_LONG_TERM 1              // Long-term trade mode

// Time constants
#define START_HOUR_DEFAULT 1                // Default start hour for trading

// Volume step
#define VOLUME_STEP_DEFAULT 0.1             // Default volume step for order sizing

// Weekly SR levels calculation
#define WEEKLY_SR_LEVELS_LONG 26             // Number of bars for long-term weekly SR levels
#define WEEKLY_SR_LEVELS_SHORT 9             // Number of bars for short-term weekly SR levels

// ATR calculation
#define ATR_PERIOD_WEEKLY 20                 // ATR period for weekly calculation
#define ATR_PERIOD_DAILY_CHECK 5             // ATR period for daily volatility check

// Risk adjustment constants
#define RISK_LEVEL_1 1                       // Base risk level
#define RISK_LEVEL_2 2                       // Risk level when gap > 0
#define RISK_LEVEL_3 3                       // Risk level when gap > weeklyATR
#define RISK_LEVEL_4 4                      // Risk level when gap > 2 * weeklyATR

// Stop loss and risk management
#define STOP_LOSS_FACTOR_ENTRY_PRICE 0.5     // Stop loss as fraction of entry price (50%)
#define ATR_MULTIPLIER_FOR_PENDING_CHECK 0.25 // ATR multiplier for pending order check (25%)
#define RISK_PNL_THRESHOLD_LOW -5            // Low risk PNL threshold for warning
#define RISK_PNL_THRESHOLD_HIGH -20          // High risk PNL threshold to block entry
#define FREE_MARGIN_THRESHOLD 1              // Minimum free margin threshold

/**
 * @brief Executes Ichimoko Weekly strategy.
 * 
 * This function implements a weekly Ichimoko-based trading strategy that:
 * 1. Calculates weekly support/resistance levels and baselines.
 * 2. Determines entry signals based on price action relative to weekly baselines.
 * 3. Adjusts risk based on distance from entry price to weekly baseline.
 * 4. Manages profit targets and risk limits.
 * 
 * Entry Conditions (BUY):
 * - Price action shows recovery pattern (preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1)
 *   OR entry price is between weekly baseline and short baseline.
 * - Weekly baseline is above entry price (positive gap).
 * - No pending orders at similar price.
 * 
 * Risk Adjustment:
 * - Risk = 4: Gap > 2 * weeklyATR
 * - Risk = 3: Gap > weeklyATR
 * - Risk = 2: Gap > 0
 * - Risk = 1: Otherwise
 * 
 * Additional Filters:
 * - Blocks entry if orders already exist in current week.
 * - Blocks entry if free margin is insufficient.
 * - Blocks entry if risk PNL < -20.
 * - Closes winning positions if risk PNL exceeds target.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate with entry/exit signals.
 * @param pBase_Indicators Base indicators containing weekly trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Weekly_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preWeeklyClose, preWeeklyClose1, preWeeklyClose2, preDailyClose;
	double shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
	double weekly_baseline = 0.0, weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	double freeMargin = 0.0;

	int openOrderCount = 0;
	int openOrderCountInCurrentWeek = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_ICHIMOKO_WEEKLY;
	pIndicators->tpMode = TP_MODE_DAILY_ATR;
	pIndicators->tradeMode = TRADE_MODE_LONG_TERM;
	pIndicators->volumeStep = VOLUME_STEP_DEFAULT;

	targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK);
	strategyVolRisk = pIndicators->strategyMaxRisk;

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);
	preWeeklyClose2 = iClose(B_WEEKLY_RATES, 3);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);


	if (timeInfo1.tm_hour >= START_HOUR_DEFAULT)
	{
		// Calculate weekly baseline from support/resistance levels
		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, WEEKLY_SR_LEVELS_LONG, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, WEEKLY_SR_LEVELS_SHORT, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		logInfo("System InstanceID = %d, BarTime = %s, weeklyHigh =%lf, weeklyLow=%lf, weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, weeklyHigh, weeklyLow, weekly_baseline);

		logInfo("System InstanceID = %d, BarTime = %s, shortWeeklyHigh =%lf, shortWeeklyLow=%lf, weekly_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyHigh, shortWeeklyLow, weekly_baseline_short);

		openOrderCount = getOrderCountEasy();
		openOrderCountInCurrentWeek = getOrderCountForCurrentWeekEasy(currentTime);

		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice * STOP_LOSS_FACTOR_ENTRY_PRICE;
				
		pBase_Indicators->weeklyATR = iAtr(B_WEEKLY_RATES, ATR_PERIOD_WEEKLY, 1);

		if (//preWeeklyClose > weekly_baseline						
			//&& preWeeklyClose > weekly_baseline_short
			//&& weekly_baseline_short > weekly_baseline		
			//&& 
			(	(preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1 && weekly_baseline - pIndicators->entryPrice > 0) ||
				(pIndicators->entryPrice > weekly_baseline && pIndicators->entryPrice < weekly_baseline_short)
			)
			//preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, ATR_MULTIPLIER_FOR_PENDING_CHECK * pBase_Indicators->weeklyATR)
			)
		{
			pIndicators->entrySignal = 1;
			
			// Adjust risk based on distance from entry price to weekly baseline
			if (weekly_baseline - pIndicators->entryPrice > 2 * pBase_Indicators->weeklyATR)
			{
				pIndicators->risk = RISK_LEVEL_4;
			}
			else if (weekly_baseline - pIndicators->entryPrice > pBase_Indicators->weeklyATR)
			{
				pIndicators->risk = RISK_LEVEL_3;
			}
			else if (weekly_baseline - pIndicators->entryPrice > 0)
			{
				pIndicators->risk = RISK_LEVEL_2;
			}
			else
			{
				pIndicators->risk = RISK_LEVEL_1;
			}
		}

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pIndicators->entrySignal != 0 && openOrderCountInCurrentWeek > 0 )
	{
		logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d openOrderCountInCurrentWeek=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount, openOrderCountInCurrentWeek);

		pIndicators->entrySignal = 0;
	}

	// Log warning if risk PNL is below low threshold
	if (pIndicators->riskPNL < RISK_PNL_THRESHOLD_LOW)
	{
		logWarning("System InstanceID = %d, BarTime = %s pIndicators->riskPNL=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL);
	}

	freeMargin = caculateFreeMarginEasy();
	
	// Block entry if free margin is insufficient
	if (pIndicators->entrySignal != 0 && freeMargin / pIndicators->entryPrice < FREE_MARGIN_THRESHOLD)
	{
		logWarning("System InstanceID = %d, BarTime = %s freeMargin=%lf, Times=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, freeMargin, freeMargin / pIndicators->entryPrice);
		pIndicators->entrySignal = 0;
	}

	// Block entry if risk PNL is below high threshold
	if (pIndicators->entrySignal != 0 && pIndicators->riskPNL < RISK_PNL_THRESHOLD_HIGH)
	{
		pIndicators->entrySignal = 0;
	}
	
	// when floating profit is too high, fe 10%
	if (pIndicators->riskPNL > targetPNL)
	{
		logWarning("System InstanceID = %d, BarTime = %s closeWinningPositionsEasy pIndicators->riskPNL=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL);

		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}


	return SUCCESS;
}
