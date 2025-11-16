/*
 * Indicator Management Module
 * 
 * Provides indicator loading and UI value setting functions.
 * Handles loading of technical indicators and updating UI display values.
 * 
 * This module:
 * - Loads BBS (Bollinger Bands Stop) indicators for multiple timeframes
 * - Initializes strategy parameters and risk settings
 * - Sets UI values for display in trading interface
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"
#include "strategies/autobbs/shared/indicators/IndicatorManagement.h"

// BBS indicator constants
#define BBS_PERIOD 20                       // Bollinger Bands period
#define BBS_DEVIATIONS 2                    // Bollinger Bands standard deviations
#define DEFAULT_RISK 1.0                    // Default risk multiplier
#define DEFAULT_TRADE_MODE 1                // Default trade mode

/**
 * Loads indicators for strategy execution.
 * 
 * Initializes and loads all required indicators:
 * - Primary ATR for risk calculations
 * - BBS (Bollinger Bands Stop) indicators for all timeframes:
 *   * Primary rates (execution timeframe)
 *   * Secondary rates (typically 15M)
 *   * Hourly (1H)
 *   * Four-hourly (4H)
 *   * Daily
 * - Strategy parameters from settings
 * - Risk limits
 * 
 * After loading indicators, calls workoutExecutionTrend() to determine
 * the execution trend based on the loaded indicators.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to populate
 * @param pBase_Indicators Base indicators structure (not modified, kept for API consistency)
 * @return SUCCESS on success, error code on failure
 */
AsirikuyReturnCode loadIndicators(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	// Load primary ATR
	pIndicators->primaryATR = iAtr(B_PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	// Load BBS indicators for all timeframes
	iBBandStop(B_PRIMARY_RATES, BBS_PERIOD, BBS_DEVIATIONS, 
	           &pIndicators->bbsTrend_primary, &pIndicators->bbsStopPrice_primary, &pIndicators->bbsIndex_primary);
	iBBandStop(B_SECONDARY_RATES, BBS_PERIOD, BBS_DEVIATIONS, 
	           &pIndicators->bbsTrend_secondary, &pIndicators->bbsStopPrice_secondary, &pIndicators->bbsIndex_secondary);
	iBBandStop(B_HOURLY_RATES, BBS_PERIOD, BBS_DEVIATIONS, 
	           &pIndicators->bbsTrend_1H, &pIndicators->bbsStopPrice_1H, &pIndicators->bbsIndex_1H);
	iBBandStop(B_FOURHOURLY_RATES, BBS_PERIOD, BBS_DEVIATIONS, 
	           &pIndicators->bbsTrend_4H, &pIndicators->bbsStopPrice_4H, &pIndicators->bbsIndex_4H);
	iBBandStop(B_DAILY_RATES, BBS_PERIOD, BBS_DEVIATIONS, 
	           &pIndicators->bbsTrend_Daily, &pIndicators->bbsStopPrice_Daily, &pIndicators->bbsIndex_Daily);

	// Initialize strategy parameters
	pIndicators->adjust = (double)parameter(AUTOBBS_ADJUSTPOINTS);
	pIndicators->risk = DEFAULT_RISK;
	pIndicators->entrySignal = 0;
	pIndicators->exitSignal = EXIT_NONE;
	pIndicators->executionTrend = 0;
	pIndicators->orderManagement = 0;
	pIndicators->tpMode = (int)parameter(AUTOBBS_LONG_SHORT_MODE);
	pIndicators->splitTradeMode = (int)parameter(AUTOBBS_TREND_MODE);
	pIndicators->stopLossPrice = 0;
	pIndicators->macroTrend = (int)parameter(AUTOBBS_MACRO_TREND);
	pIndicators->side = (int)parameter(AUTOBBS_ONE_SIDE);
	pIndicators->executionRateTF = (int)parameter(AUTOBBS_EXECUTION_RATES);

	pIndicators->tradeMode = DEFAULT_TRADE_MODE;

	// Set risk limits (negative values indicate limits)
	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1.0;
	pIndicators->limitRiskPNL = pParams->settings[ACCOUNT_RISK_PERCENT] * -1.0;

	pIndicators->total_lose_pips = 0;

	// Calculate execution trend based on loaded indicators
	workoutExecutionTrend(pParams, pIndicators, pBase_Indicators);

	return returnCode;
}

/**
 * Sets UI values for display in the trading interface.
 * 
 * Updates the user interface with current indicator values based on
 * the AUTOBBS_TREND_MODE setting. Different modes display different
 * sets of values.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure
 * @return SUCCESS on success
 * 
 * UI values set (varies by trend mode):
 * - DailyTrend, dailyTrend_Phase: Daily trend information
 * - entrySignal, ExecutionTrend: Entry/execution signals
 * - DailyS, DailyPivot: Support and pivot levels
 * - stopLossPrice: Current stop loss level
 * - dailyATR, weeklyATR: ATR values
 * - pDailyHigh, pDailyLow: Predicted daily high/low
 * - bbsTrend_excution, bbsStopPrice_excution: BBS execution indicators
 * - AccountRisk, strategyRisk: Risk metrics
 * - riskPNL, riskPNLNLP: PNL risk metrics
 * - StrategyVolRisk, strategyMarketVolRisk: Volatility risk metrics
 */
AsirikuyReturnCode setUIValues(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	switch ((int)parameter(AUTOBBS_TREND_MODE))
	{
	case 15:
	case 16:
		// Special UI values for trend modes 15 and 16
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_secondary);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_secondary);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		addValueToUI("riskPNL", pIndicators->riskPNL);
		addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	default:
		// Standard UI values for other trend modes
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		addValueToUI("riskPNL", pIndicators->riskPNL);
		addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	}
	
	return SUCCESS;
}
