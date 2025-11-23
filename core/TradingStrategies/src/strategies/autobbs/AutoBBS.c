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

#include <string.h>
#include <math.h>
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"
#include "strategies/autobbs/shared/indicators/IndicatorManagement.h"
#if defined _WIN32 || defined _WIN64
#include <windows.h>
#elif defined __APPLE__ || defined __linux__
#include <sys/time.h>
#endif

// Timing helper function for performance measurement
static double getCurrentTimeMs(void)
{
#if defined _WIN32 || defined _WIN64
	LARGE_INTEGER frequency, counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return (double)counter.QuadPart * 1000.0 / (double)frequency.QuadPart;
#elif defined __APPLE__ || defined __linux__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
#else
	return 0.0;
#endif
}

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

// setUIValues is implemented in IndicatorManagement.c (declared in IndicatorManagement.h)

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

static AsirikuyReturnCode loadIndicatorsAutoBBS(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	double originEquity = 0.0;
	double risk = DEFAULT_RISK;

	pIndicators->primaryATR = iAtr(B_PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	iBBandStop(B_PRIMARY_RATES, BBS_PERIOD, BBS_DEVIATIONS, &pIndicators->bbsTrend_primary, &pIndicators->bbsStopPrice_primary, &pIndicators->bbsIndex_primary);
	iBBandStop(B_SECONDARY_RATES, BBS_PERIOD, BBS_DEVIATIONS, &pIndicators->bbsTrend_secondary, &pIndicators->bbsStopPrice_secondary, &pIndicators->bbsIndex_secondary);
	iBBandStop(B_HOURLY_RATES, BBS_PERIOD, BBS_DEVIATIONS, &pIndicators->bbsTrend_1H, &pIndicators->bbsStopPrice_1H, &pIndicators->bbsIndex_1H);
	iBBandStop(B_FOURHOURLY_RATES, BBS_PERIOD, BBS_DEVIATIONS, &pIndicators->bbsTrend_4H, &pIndicators->bbsStopPrice_4H, &pIndicators->bbsIndex_4H);
	iBBandStop(B_DAILY_RATES, BBS_PERIOD, BBS_DEVIATIONS, &pIndicators->bbsTrend_Daily, &pIndicators->bbsStopPrice_Daily, &pIndicators->bbsIndex_Daily);

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

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1.0;
	pIndicators->limitRiskPNL = pParams->settings[ACCOUNT_RISK_PERCENT] * -1.0;

	pIndicators->total_lose_pips = 0;

	pIndicators->maxTradeTime = DEFAULT_MAX_TRADE_TIME;
	pIndicators->startHour = (int)parameter(AUTOBBS_STARTHOUR);
	pIndicators->stopMovingBackSL = TRUE;

	pIndicators->volume1 = 0.0;
	pIndicators->volume2 = 0.0;
	pIndicators->cmfVolume = 0.0;
	pIndicators->CMFVolumeGap = 0.0;
	pIndicators->fast = 0.0;
	pIndicators->slow = 0.0;
	pIndicators->preFast = 0.0;
	pIndicators->preSlow = 0.0;

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
	double baseStartTime = getCurrentTimeMs();
	if ((int)parameter(AUTOBBS_TREND_MODE) < BASE_INDICATORS_THRESHOLD)
		runBase(pParams, &base_Indicators);
	double baseEndTime = getCurrentTimeMs();
	double baseDuration = baseEndTime - baseStartTime;
	if (baseDuration > 10.0) {
		logInfo("[TIMING] runBase took %.3f ms (instanceId=%d, barTime=%s)", 
			baseDuration, (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	}

	// Load strategy-specific indicators
	double indicatorsStartTime = getCurrentTimeMs();
	loadIndicatorsAutoBBS(pParams, &indicators, &base_Indicators);
	double indicatorsEndTime = getCurrentTimeMs();
	double indicatorsDuration = indicatorsEndTime - indicatorsStartTime;
	if (indicatorsDuration > 10.0) {
		logInfo("[TIMING] loadIndicators took %.3f ms (instanceId=%d, barTime=%s)", 
			indicatorsDuration, (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	}

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
	double exitsStartTime = getCurrentTimeMs();
	returnCode = handleTradeExits(pParams, &indicators);
	double exitsEndTime = getCurrentTimeMs();
	double exitsDuration = exitsEndTime - exitsStartTime;
	if (exitsDuration > 10.0) {
		logInfo("[TIMING] handleTradeExits took %.3f ms (instanceId=%d, barTime=%s)", 
			exitsDuration, (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	}
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeExits()", returnCode);
	}

	// Handle trade entries (open/modify positions)
	double entriesStartTime = getCurrentTimeMs();
	returnCode = handleTradeEntries(pParams, &indicators, &base_Indicators);
	double entriesEndTime = getCurrentTimeMs();
	double entriesDuration = entriesEndTime - entriesStartTime;
	if (entriesDuration > 10.0) {
		logInfo("[TIMING] handleTradeEntries took %.3f ms (instanceId=%d, barTime=%s)", 
			entriesDuration, (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	}
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeEntries()", returnCode);
	}

	return SUCCESS;
}
