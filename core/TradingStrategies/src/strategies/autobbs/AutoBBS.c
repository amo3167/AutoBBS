/*
 * AutoBBS - Main Entry Point
 * 
 * This is the main entry point for the AutoBBS trading strategy system.
 * It orchestrates the entire strategy execution flow:
 * 
 * 1. Loads base indicators (trend, support/resistance, ATR predictions)
 * 2. Loads strategy-specific indicators (BBS, execution signals)
 * 3. Validates market data and configuration
 * 4. Handles trade exits (closing positions)
 * 5. Handles trade entries (opening new positions or modifying existing ones)
 * 6. Updates UI with current strategy state
 * 
 * The strategy supports multiple trading modes (trend, swing, day trading, etc.)
 * controlled by the AUTOBBS_TREND_MODE parameter.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/swing/SwingStrategy.h"
#include "strategies/autobbs/trend/TrendStrategy.h"
#include "StrategyUserInterface.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// Time constants for spread adjustment during day transition
#define DAY_TRANSITION_START_HOUR 23
#define DAY_TRANSITION_START_MINUTE 40
#define DAY_TRANSITION_END_HOUR 0
#define DAY_TRANSITION_END_MINUTE 20
#define SPREAD_MULTIPLIER_DURING_TRANSITION 3

// BBS indicator constants
#define BBS_PERIOD 20
#define BBS_DEVIATIONS 2

// Default values
#define DEFAULT_RISK 1.0
#define DEFAULT_TRADE_MODE 1
#define DEFAULT_MAX_TRADE_TIME 3
#define DEFAULT_MIN_LOT_SIZE 0.01
#define DEFAULT_VOLUME_STEP 0.01

// Strategy mode constants
#define GBPJPY_DAILY_SWING_MODE 16
#define STRATEGY_MODE_BASE 0
#define STRATEGY_MODE_FULL 1
#define BASE_INDICATORS_THRESHOLD 99

// Risk cap constants
#define RISK_CAP_DEFAULT 0

/**
 * Sets UI values for display in the trading interface.
 * 
 * Updates the user interface with current indicator values based on
 * the AUTOBBS_TREND_MODE setting. Different modes display different
 * sets of values optimized for their specific trading approach.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure
 * @return SUCCESS on success
 */
static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	switch ((int)parameter(AUTOBBS_TREND_MODE))
	{
	case 0:
		// Standard trend mode UI values
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("dailyTP", pBase_Indicators->dailyTP);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("bbsTrend_4H", pIndicators->bbsTrend_4H);
		addValueToUI("bbsStopPrice_4H", pIndicators->bbsStopPrice_4H);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 2:
	case 3:
		// MACD and Shellington trend modes
		addValueToUI("MacdTrend", pBase_Indicators->mACDInTrend);
		addValueToUI("ShellingtonTrend", pBase_Indicators->shellingtonInTrend);
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("flatTrend", pBase_Indicators->flatTrend);
		addValueToUI("dailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS1", pBase_Indicators->dailyS1);
		addValueToUI("DailyR1", pBase_Indicators->dailyR1);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("maxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("takeProfit", pIndicators->takePrice);
		addValueToUI("stopLoss", pIndicators->stopLoss);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 5:
		// Day trading mode
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("BBSTrend_primary", pIndicators->bbsTrend_primary);
		addValueToUI("BBSStopPrice_primary", pIndicators->bbsStopPrice_primary);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 6:
		// Alternative trend mode
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 9:
		// MACD with ATR euro range mode
		addValueToUI("MacdTrend", pBase_Indicators->mACDInTrend);
		addValueToUI("ShellingtonTrend", pBase_Indicators->shellingtonInTrend);
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("dailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS1", pBase_Indicators->dailyS1);
		addValueToUI("DailyR1", pBase_Indicators->dailyR1);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("maxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("takeProfit", pIndicators->takePrice);
		addValueToUI("stopLoss", pIndicators->stopLoss);
		addValueToUI("macdMaxLevel", (double)parameter(AUTOBBS_IS_ATREURO_RANGE));
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 10:
		// Weekly trend mode
		addValueToUI("weeklyTrend", pBase_Indicators->weeklyTrend);
		addValueToUI("weeklyTrend_Phase", pBase_Indicators->weeklyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("weeklyS", pBase_Indicators->weeklyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("weeklyPivot", pBase_Indicators->weeklyPivot);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 15:
		// Swing trading mode
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_secondary);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_secondary);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 16:
	case 19:
		// GBPJPY Daily Swing and 4H Swing modes
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("pDailyPredictATR", pBase_Indicators->pDailyPredictATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 17:
		// ATR prediction mode
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->pDailyPredictATR);
		addValueToUI("dailyMaxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("weeklyATR", pBase_Indicators->pWeeklyPredictATR);
		addValueToUI("weeklyMaxATR", pBase_Indicators->pWeeklyPredictMaxATR);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		break;
	case 18:
	case 20:
		// 4H Swing and Weekly Auto modes
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("atr_euro_range", pIndicators->atr_euro_range);
		addValueToUI("pWeeklyPredictATR", pBase_Indicators->pWeeklyPredictATR);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 21:
	case 22:
		// Multiple day trading modes
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("atr_euro_range", pIndicators->atr_euro_range);
		addValueToUI("stopLoss", pIndicators->stopLoss);
		addValueToUI("takePrice", pIndicators->takePrice);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 23:
		// MACD mode
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("volume1", pIndicators->volume1);
		addValueToUI("volume2", pIndicators->volume2);
		addValueToUI("cmfVolume", pIndicators->cmfVolume);
		addValueToUI("CMFVolumeGap", pIndicators->CMFVolumeGap);
		addValueToUI("fast", pIndicators->fast);
		addValueToUI("preFast", pIndicators->preFast);
		addValueToUI("slow", pIndicators->slow);
		addValueToUI("preSlow", pIndicators->preSlow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 26:
		// MACD with Shellington mode
		addValueToUI("MacdTrend", pBase_Indicators->mACDInTrend);
		addValueToUI("ShellingtonTrend", pBase_Indicators->shellingtonInTrend);
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("maxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 30:
		// Shellington low risk mode
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 31:
		// Ichimoku mode
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("daily_baseline", pIndicators->daily_baseline);
		addValueToUI("daily_baseline_short", pIndicators->daily_baseline_short);
		addValueToUI("cmfVolume", pIndicators->cmfVolume);
		addValueToUI("fast", pIndicators->fast);
		addValueToUI("slow", pIndicators->slow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	default:
		// Default UI values for unknown modes
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
		break;
	}

	return SUCCESS;
}

/**
 * Loads indicators for strategy execution.
 * 
 * Initializes and loads all required indicators:
 * - Primary ATR for risk calculations
 * - BBS (Bollinger Bands Stop) indicators for all timeframes
 * - Strategy parameters from settings
 * - Risk limits and virtual balance adjustments
 * 
 * After loading indicators, calls workoutExecutionTrend() to determine
 * the execution trend based on the loaded indicators.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to populate
 * @param pBase_Indicators Base indicators structure (not modified, kept for API consistency)
 * @return SUCCESS on success, error code on failure
 */
static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	double originEquity = 0.0;
	double risk = DEFAULT_RISK;

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
	pIndicators->atr_euro_range = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);

	// Set risk limits (negative values indicate limits)
	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1.0;
	pIndicators->limitRiskPNL = pParams->settings[ACCOUNT_RISK_PERCENT] * -1.0;

	pIndicators->total_lose_pips = 0;
	pIndicators->maxTradeTime = DEFAULT_MAX_TRADE_TIME;
	pIndicators->startHour = (int)parameter(AUTOBBS_STARTHOUR);
	pIndicators->stopMovingBackSL = TRUE;

	// Initialize volume indicators
	pIndicators->volume1 = 0.0;
	pIndicators->volume2 = 0.0;
	pIndicators->cmfVolume = 0.0;
	pIndicators->CMFVolumeGap = 0.0;
	pIndicators->fast = 0.0;
	pIndicators->slow = 0.0;
	pIndicators->preFast = 0.0;
	pIndicators->preSlow = 0.0;

	// Initialize Ichimoku indicators
	pIndicators->daily_baseline = 0.0;
	pIndicators->daily_baseline_short = 0.0;

	// Handle virtual balance top-up (for testing/adjustment)
	pIndicators->virtualBalanceTopup = (double)parameter(AUTOBBS_VIRTUAL_BALANCE_TOPUP);
	if (pIndicators->virtualBalanceTopup > 0)
	{
		logWarning("System InstanceID = %d, top up equity %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], pIndicators->virtualBalanceTopup);
		
		// Adjust equity and recalculate risk percentages
		originEquity = pParams->accountInfo.equity;
		pParams->accountInfo.equity += pIndicators->virtualBalanceTopup;

		risk = readRiskFile((BOOL)pParams->settings[IS_BACKTESTING]);
		pParams->accountInfo.equity = pParams->accountInfo.equity * risk;

		// Adjust risk percentage to account for equity change
		pParams->accountInfo.totalOpenTradeRiskPercent = pParams->accountInfo.totalOpenTradeRiskPercent / (pParams->accountInfo.equity / originEquity);
	}

	// Initialize order size parameters
	pIndicators->riskCap = RISK_CAP_DEFAULT;
	pIndicators->minLotSize = DEFAULT_MIN_LOT_SIZE;
	pIndicators->volumeStep = DEFAULT_VOLUME_STEP;
	pIndicators->isEnableBuyMinLotSize = FALSE;
	pIndicators->isEnableSellMinLotSize = FALSE;

	// Initialize status message
	memset(pIndicators->status, '\0', MAX_OUTPUT_ERROR_STRING_SIZE);
	strcpy(pIndicators->status, "No Error\n\n");

	// Calculate execution trend based on loaded indicators
	workoutExecutionTrend(pParams, pIndicators, pBase_Indicators);

	return returnCode;
}

/**
 * Handles trade entries based on execution signals.
 * 
 * Processes entry signals and either:
 * - Opens new orders via splitBuyOrders/splitSellOrders if entrySignal is set
 * - Modifies existing orders via modifyOrders if orders already exist
 * 
 * Also adjusts spread during day transition period (23:40-00:20) to account
 * for increased volatility during market close/open.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure
 * @param pBase_Indicators Base indicators structure
 * @return SUCCESS on success, error code on failure
 */
static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss, takePrice_primary = 0;
	int riskcap = (int)parameter(AUTOBBS_RISK_CAP);

	if (pParams == NULL)
	{
		logCritical("handleTradeEntries() failed. pParams = NULL\n\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		logCritical("handleTradeEntries() failed. pIndicators = NULL\n\n");
		return NULL_POINTER;
	}

	// Increase spread adjustment during day transition (23:40-00:20) for market close/open volatility
	if ((hour() == DAY_TRANSITION_START_HOUR && minute() > DAY_TRANSITION_START_MINUTE) || 
	    (hour() == DAY_TRANSITION_END_HOUR && minute() < DAY_TRANSITION_END_MINUTE))
	{
		pIndicators->adjust = SPREAD_MULTIPLIER_DURING_TRANSITION * pIndicators->adjust;
	}

	// Calculate stop loss
	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice);

	// Calculate take profit based on TP mode
	switch (pIndicators->tpMode)
	{
	case 0:
		// TP = stop loss (1:1 risk/reward)
		takePrice_primary = stopLoss;
		break;
	case 1:
		// TP = distance to secondary BBS stop price
		takePrice_primary = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_secondary) + pIndicators->adjust;
		pIndicators->risk = pIndicators->risk * min(stopLoss / takePrice_primary, riskcap);
		break;
	case 2:
		// No TP
		takePrice_primary = 0;
		break;
	case 3:
		// TP = daily ATR
		takePrice_primary = pBase_Indicators->dailyATR;
		break;
	case 4:
		// TP = distance to take profit price
		takePrice_primary = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);
		break;
	default:
		// Default: TP = stop loss
		takePrice_primary = stopLoss;
		break;
	}

	// Handle BUY orders
	if (pIndicators->executionTrend == 1)
	{
		if (pIndicators->entrySignal == 1)
			// Open new buy orders
			splitBuyOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		else if (totalOpenOrders(pParams, BUY) > 0)
			// Modify existing buy orders
			modifyOrders(pParams, pIndicators, pBase_Indicators, BUY, stopLoss, -1);
	}

	// Handle SELL orders
	if (pIndicators->executionTrend == -1)
	{
		if (pIndicators->entrySignal == -1)
			// Open new sell orders
			splitSellOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		else if (totalOpenOrders(pParams, SELL) > 0)
			// Modify existing sell orders
			modifyOrders(pParams, pIndicators, pBase_Indicators, SELL, stopLoss, -1);
	}

	return SUCCESS;
}

/**
 * Main entry point for AutoBBS strategy execution.
 * 
 * This is the primary function called by the Asirikuy framework to execute
 * the AutoBBS trading strategy. It orchestrates the entire strategy flow:
 * 
 * 1. Validates parameters and market data
 * 2. Loads base indicators (if needed)
 * 3. Loads strategy-specific indicators
 * 4. Sets UI values for display
 * 5. Handles trade exits (closes positions)
 * 6. Handles trade entries (opens/modifies positions)
 * 
 * @param pParams Strategy parameters containing rates, settings, and account info
 * @return SUCCESS on success, error code on failure
 * 
 * Error codes:
 * - NULL_POINTER: pParams is NULL
 * - INVALID_CONFIG: Invalid parameter configuration detected
 * - Other error codes from called functions
 */
AsirikuyReturnCode runAutoBBS(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;
	Base_Indicators base_Indicators;
	int rateErrorTimes = -1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;

	if (pParams == NULL)
	{
		logCritical("runAutoBBS() failed. pParams = NULL\n\n");
		return NULL_POINTER;
	}

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	// Set strategy mode based on trend mode
	// Mode 16 (GBPJPY Daily Swing) uses base mode (only daily indicators)
	// Other modes use full mode (all indicators)
	if ((int)parameter(AUTOBBS_TREND_MODE) == GBPJPY_DAILY_SWING_MODE)
		base_Indicators.strategy_mode = STRATEGY_MODE_BASE;
	else
		base_Indicators.strategy_mode = STRATEGY_MODE_FULL;

	// Read rate error count from file
	// If more than threshold, skip rate validation
	rateErrorTimes = readRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

	// Validate market data (only in live trading, not backtesting)
	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && (int)pParams->settings[TIMEFRAME] >= 5 &&
		(
		validateCurrentTimeEasy(pParams, B_PRIMARY_RATES) > 0 ||
		validateDailyBarsEasy(pParams, B_PRIMARY_RATES, B_DAILY_RATES) > 0 ||
		validateHourlyBarsEasy(pParams, B_PRIMARY_RATES, B_HOURLY_RATES) > 0 ||
		validateSecondaryBarsEasy(pParams, B_PRIMARY_RATES, B_SECONDARY_RATES, (int)parameter(AUTOBBS_EXECUTION_RATES), rateErrorTimes) > 0
		))
	{
		logWarning("System InstanceID = %d, BarTime = %s: validate time failure.", 
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		return SUCCESS;
	}

	// Validate parameter configuration
	// AUTOBBS_MACRO_TREND and AUTOBBS_ONE_SIDE must have same sign
	if ((int)parameter(AUTOBBS_MACRO_TREND) * (int)parameter(AUTOBBS_ONE_SIDE) < 0)
	{
		logError("Invalid parameter config: System InstanceID = %d, BarTime = %s, AUTOBBS_MACRO_TREND = %d, AUTOBBS_ONE_SIDE = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, (int)parameter(AUTOBBS_MACRO_TREND), (int)parameter(AUTOBBS_ONE_SIDE));
		return INVALID_CONFIG;
	}

	// Load base indicators (if trend mode < 99)
	// Modes >= 99 don't need base indicators
	if ((int)parameter(AUTOBBS_TREND_MODE) < BASE_INDICATORS_THRESHOLD)
		runBase(pParams, &base_Indicators);

	// Load strategy-specific indicators
	loadIndicators(pParams, &indicators, &base_Indicators);

	// Update UI with current values
	setUIValues(pParams, &indicators, &base_Indicators);

	// Log debug information
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld, BBSTrend_primary = %ld, BBStopPrice_primary = %lf, BBSIndex_primary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_primary, indicators.bbsStopPrice_primary, indicators.bbsIndex_primary);
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld, bbsTrend_secondary = %ld, BBStopPrice_secondary = %lf, bbsIndex_secondary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_secondary, indicators.bbsStopPrice_secondary, indicators.bbsIndex_secondary);
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld, BBSTrend_1H = %ld, BBStopPrice_1H = %lf, BBSIndex_1H = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_1H, indicators.bbsStopPrice_1H, indicators.bbsIndex_1H);
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld, BBSTrend_4H = %ld, BBStopPrice_4H = %lf, BBSIndex_4H = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_4H, indicators.bbsStopPrice_4H, indicators.bbsIndex_4H);

	// Handle trade exits first (close positions)
	returnCode = handleTradeExits(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeExits()", returnCode);
	}

	// Handle trade entries (open/modify positions)
	returnCode = handleTradeEntries(pParams, &indicators, &base_Indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeEntries()", returnCode);
	}

	return SUCCESS;
}
