/*
 * Profit Management Module
 * 
 * Provides profit management functions for strategy execution.
 * Handles risk control, profit taking, and order closure based on profit targets.
 * 
 * This module implements three levels of profit management:
 * - Base: Risk control by macro factor and entry signal validation
 * - Standard: Daily/weekly profit taking and order closure
 * - Weekly: Weekly-specific profit management
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/common/ProfitManagement.h"

// Risk adjustment factors
#define RISK_REDUCTION_OPPOSITE_TREND 0.5   // Reduce risk by 50% when trading against macro trend
#define RISK_REDUCTION_NEUTRAL_TREND 0.8    // Reduce risk by 20% when macro trend is neutral
#define TARGET_PNL_MULTIPLIER 3.0           // Target PNL multiplier (3x max strategy risk)
#define NO_TP_ORDER_DAYS_THRESHOLD 4        // Maximum days for pending orders without TP
#define ATR_DIVISOR_FOR_NO_TP_CHECK 3.0     // ATR divisor for checking same price pending orders

/**
 * Base profit management function.
 * 
 * Controls risk by macro factor and validates entry signals based on risk limits.
 * This function:
 * 1. Adjusts risk based on macro trend alignment
 * 2. Validates entry signals against strategy risk limits
 * 3. Validates entry signals against PNL risk limits
 * 
 * Risk adjustment rules:
 * - If trading against macro trend: reduce risk by 50%
 * - If macro trend is neutral: reduce risk by 20%
 * - If trading with macro trend: use full risk
 * 
 * Entry signal validation:
 * - Entry signal is cancelled if strategy risk exceeds limits
 * - Entry signal is cancelled if PNL risk is below minimum threshold
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure (not modified, kept for API consistency)
 */
void profitManagement_base(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	// Adjust risk based on macro trend alignment for BUY signals
	if (pIndicators->entrySignal == 1)
	{
		// Reduce risk if trading against macro trend (macro trend is down)
		if (pIndicators->macroTrend < 0)
		{
			pIndicators->risk *= RISK_REDUCTION_OPPOSITE_TREND;
			pIndicators->strategyMaxRisk *= RISK_REDUCTION_OPPOSITE_TREND;
		}
		// Reduce risk if macro trend is neutral
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= RISK_REDUCTION_NEUTRAL_TREND;
			pIndicators->strategyMaxRisk *= RISK_REDUCTION_NEUTRAL_TREND;
		}
	}

	// Adjust risk based on macro trend alignment for SELL signals
	if (pIndicators->entrySignal == -1)
	{
		// Reduce risk if trading against macro trend (macro trend is up)
		if (pIndicators->macroTrend > 0)
		{
			pIndicators->risk *= RISK_REDUCTION_OPPOSITE_TREND;
			pIndicators->strategyMaxRisk *= RISK_REDUCTION_OPPOSITE_TREND;
		}
		// Reduce risk if macro trend is neutral
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= RISK_REDUCTION_NEUTRAL_TREND;
			pIndicators->strategyMaxRisk *= RISK_REDUCTION_NEUTRAL_TREND;
		}
	}

	// Validate entry signal: cancel if strategy risk exceeds limits
	if (pIndicators->entrySignal != 0 && 
	    pIndicators->strategyRiskWithoutLockedProfit < pIndicators->strategyMaxRisk)
	{
		logWarning("System InstanceID = %d, BarTime = %s, strategyRisk %lf < strategyRiskWithoutLockedProfit %lf, skip this entry signal = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyMaxRisk, pIndicators->strategyRiskWithoutLockedProfit, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Validate entry signal: cancel if PNL risk is below minimum threshold
	if (pIndicators->riskPNL < pIndicators->limitRiskPNL && pIndicators->entrySignal != 0)
	{
		logWarning("System InstanceID = %d, BarTime = %s, PNL risk %lf < riskPNLWithoutLockedProfit %lf, skip this entry signal = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}
}

/**
 * Weekly profit management function.
 * 
 * Handles weekly profit taking and order closure based on weekly support/resistance levels.
 * This function:
 * 1. Calls base profit management for risk control
 * 2. Closes short-term orders at end of week (EOW) if price exceeds weekly R3/S3
 * 3. Takes profit when floating profit exceeds target (3x max strategy risk)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing trend phases and support/resistance
 */
void profitManagementWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * TARGET_PNL_MULTIPLIER;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	// Apply base profit management (risk control)
	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	// Close short-term orders at end of week (EOW) if price exceeds weekly R3/S3
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR3)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS3)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);

	// Take profit when floating profit exceeds target (3x max strategy risk)
	if (pBase_Indicators->weeklyTrend_Phase > 0
		&& pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2
		&& pIndicators->riskPNL > targetPNL)
	{
		logWarning("System InstanceID = %d, BarTime = %s, PNL risk %lf, riskPNLWithoutLockedProfit %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit);

		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}
	else if (pBase_Indicators->weeklyTrend_Phase < 0
		&& pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2
		&& pIndicators->riskPNL > targetPNL)
	{
		logWarning("System InstanceID = %d, BarTime = %s, PNL risk %lf, riskPNLWithoutLockedProfit %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit);
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}
}

/**
 * Standard profit management function.
 * 
 * Handles daily/weekly profit taking and order closure based on support/resistance levels.
 * This function:
 * 1. Calls base profit management for risk control
 * 2. Validates entry signals based on pending orders without TP
 * 3. Closes short-term orders at end of day (EOD) if price exceeds daily R3/S3
 * 4. Closes short-term orders at end of week (EOW) if price exceeds weekly R2/S2
 * 5. Takes profit when floating profit exceeds target (3x max strategy risk)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing trend phases and support/resistance
 */
void profitManagement(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * TARGET_PNL_MULTIPLIER;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	int noTPOrderDaysNumber = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	// Apply base profit management (risk control)
	profitManagement_base(pParams, pIndicators, pBase_Indicators);
	
	// Check for pending orders without TP at same price level
	noTPOrderDaysNumber = getSamePricePendingNoTPOrdersEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_NO_TP_CHECK);

	// Cancel entry signal if too many days with pending orders without TP
	if (noTPOrderDaysNumber >= NO_TP_ORDER_DAYS_THRESHOLD && pIndicators->entrySignal != 0)
	{
		logWarning("System InstanceID = %d, BarTime = %s, SamePricePendingNoTPOrdersDays %d, skip this entry signal = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, noTPOrderDaysNumber, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Daily profit taking at end of day (EOD) if price exceeds daily R3/S3
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->dailyR3)
	{
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, over dailyR3 skip this entry signal = %d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	}
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->dailyS3)
	{
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, under dailyS3 skip this entry signal = %d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	}

	// Weekly profit taking at end of week (EOW) if price exceeds weekly R2/S2
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
	{
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, over weeklyR2 skip this entry signal = %d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2)
	{
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, under weeklyS2 skip this entry signal = %d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}

	// Take profit when floating profit exceeds target (3x max strategy risk)
	if (pBase_Indicators->dailyTrend_Phase > 0
		&& pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2
		&& pIndicators->riskPNL > targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	else if (pBase_Indicators->dailyTrend_Phase < 0
		&& pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2
		&& pIndicators->riskPNL > targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
}
