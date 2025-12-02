/*
 * BBS Swing Strategy Module
 * 
 * Provides BBS (Bollinger Bands Stop) Swing strategy execution functions.
 * This module implements 4H swing trading strategies that use Bollinger Bands
 * on the 4-hour timeframe combined with daily/weekly trend analysis.
 * 
 * Strategies included:
 * - 4H BBS Swing XAUUSD BoDuan: Specialized for XAUUSD with KeyK pattern detection
 * - 4H BBS Swing BoDuan: General BoDuan strategy with symbol-specific stop loss
 * - 4H BBS Swing: Standard 4H swing strategy with trend filtering
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
#include "strategies/autobbs/trend/bbs/BBSSwingStrategy.h"
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"

// Strategy mode constants
#define SPLIT_TRADE_MODE_4H_SWING_100P 20      // Split trade mode for 4H swing 100P
#define TP_MODE_DAILY_ATR 3                    // Take profit mode: daily ATR
#define SPLIT_TRADE_MODE_4H_SWING 19           // Split trade mode for 4H swing

// Time constants for 4H bar detection
#define HOURS_PER_4H_BAR 4                     // Hours per 4H bar
#define MINUTE_THRESHOLD_FOR_4H_BAR 3          // Minute threshold for 4H bar detection

// ATR and movement constants
#define ATR_PERIOD_FOR_MA_TREND 20             // ATR period for MA trend calculation
#define WEEKLY_ATR_FACTOR_FOR_RANGE 0.4        // Factor for weekly ATR range (40%)
#define KEYK_CLOSE_THRESHOLD_DIVISOR 3          // Divisor for KeyK close price threshold
#define RISK_REDUCTION_RANGE_TREND 0.5          // Risk reduction for range trend (50%)
#define ATR_FACTOR_FOR_RANGE_RISK 0.5          // ATR factor for range trend risk check

// Stop loss constants
#define GBPJPY_STOP_LOSS_PIPS 2.5              // Stop loss for GBPJPY (pips)
#define XAUUSD_STOP_LOSS_PIPS 20               // Stop loss for XAUUSD (pips)

// Symbol constants
#define SYMBOL_GBPJPY "GBPJPY"
#define SYMBOL_GBPAUD "GBPAUD"
#define SYMBOL_XAUUSD "XAUUSD"
// ATR divisor for pending order checks
#define ATR_DIVISOR_FOR_PENDING_4H 4           // ATR divisor for checking pending orders

/**
 * @brief Symbol-specific configuration structure for BBS Swing BoDuan strategy.
 * 
 * This structure holds all symbol-specific parameters that control
 * the behavior of the BBS Swing BoDuan strategy for different trading instruments.
 */
typedef struct {
	/* Stop loss configuration */
	double stopLoss;                            /* Stop loss value (pips or ATR-based) */
	
	/* Timing configuration */
	int hourOffset;                            /* 4H bar start hour offset (0 = standard, 1 = XAUUSD style) */
	
	/* ATR configuration */
	double weeklyATRFactor;                    /* Weekly ATR factor for range detection (e.g., 0.4 = 40%) */
	
	/* Validation flags */
	BOOL enableWeeklyATRValidation;            /* Enable weekly ATR validation */
} BBSSymbolConfig;

/**
 * @brief Initializes symbol-specific configuration for BBS Swing BoDuan strategy.
 * 
 * This function configures all symbol-specific parameters based on the trading
 * symbol. Each symbol has unique characteristics that require different stop loss
 * settings, timing configurations, and validation rules.
 * 
 * Symbol-Specific Trading Logic:
 * 
 * XAUUSD (Gold):
 * - Stop loss: 20 pips (fixed)
 * - Hour offset: 1 (triggers at 1, 5, 9, 13, 17, 21)
 * - Weekly ATR factor: 0.4 (40% of predicted weekly ATR)
 * - Weekly ATR validation: Enabled (validates entry against weekly price gap)
 * 
 * GBPJPY (Major Forex):
 * - Stop loss: 2.5 pips (fixed)
 * - Hour offset: 0 (standard 4H boundaries: 0, 4, 8, 12, 16, 20)
 * - Weekly ATR factor: 0.4 (40% of predicted weekly ATR)
 * - Weekly ATR validation: Disabled
 * 
 * GBPAUD (Forex):
 * - Stop loss: Weekly max ATR (dynamic)
 * - Hour offset: 0 (standard 4H boundaries)
 * - Weekly ATR factor: 0.4 (40% of predicted weekly ATR)
 * - Weekly ATR validation: Disabled
 * 
 * Other symbols:
 * - Uses default values (stop loss: 0, hour offset: 0, weekly ATR factor: 0.4, validation: disabled)
 * 
 * @param pConfig Configuration structure to populate
 * @param pParams Strategy parameters containing symbol information
 * @param pBase_Indicators Base indicators (for ATR calculations)
 */
static void initializeSymbolConfig(BBSSymbolConfig* pConfig, StrategyParams* pParams, 
	Base_Indicators* pBase_Indicators)
{
	/* Initialize with default values */
	memset(pConfig, 0, sizeof(BBSSymbolConfig));
	
	pConfig->stopLoss = 0.0;  /* 0 = not set, will use default */
	pConfig->hourOffset = 0;  /* Standard 4H boundaries */
	pConfig->weeklyATRFactor = WEEKLY_ATR_FACTOR_FOR_RANGE;  /* Default: 0.4 = 40% */
	pConfig->enableWeeklyATRValidation = FALSE;
	
	/* Configure based on symbol */
	if (strstr(pParams->tradeSymbol, SYMBOL_XAUUSD) != NULL)
	{
		/* XAUUSD configuration
		 * Trading Logic:
		 * - Fixed stop loss: 20 pips (gold has high volatility)
		 * - Hour offset: 1 (starts 1 hour later than standard 4H boundaries)
		 * - Weekly ATR validation: Enabled (prevents entry when weekly gap is too large)
		 */
		pConfig->stopLoss = XAUUSD_STOP_LOSS_PIPS;
		pConfig->hourOffset = 1;
		pConfig->enableWeeklyATRValidation = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, SYMBOL_GBPJPY) != NULL)
	{
		/* GBPJPY configuration
		 * Trading Logic:
		 * - Fixed stop loss: 2.5 pips (tight stop for volatile pair)
		 * - Hour offset: 0 (standard 4H boundaries)
		 * - Weekly ATR validation: Disabled
		 */
		pConfig->stopLoss = GBPJPY_STOP_LOSS_PIPS;
		pConfig->hourOffset = 0;
		pConfig->enableWeeklyATRValidation = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, SYMBOL_GBPAUD) != NULL)
	{
		/* GBPAUD configuration
		 * Trading Logic:
		 * - Dynamic stop loss: Weekly max ATR (adapts to market volatility)
		 * - Hour offset: 0 (standard 4H boundaries)
		 * - Weekly ATR validation: Disabled
		 */
		pConfig->stopLoss = pBase_Indicators->pWeeklyPredictMaxATR;
		pConfig->hourOffset = 0;
		pConfig->enableWeeklyATRValidation = FALSE;
	}
	/* Other symbols use default values */
	
	/* Override with parameters if set (parameters take precedence over symbol defaults) */
	{
		double paramWeeklyATRFactor = parameter(AUTOBBS_BBS_WEEKLY_ATR_FACTOR);
		if (paramWeeklyATRFactor > 0.0)
			pConfig->weeklyATRFactor = paramWeeklyATRFactor;
		
		double paramHourOffset = parameter(AUTOBBS_BBS_4H_START_HOUR_OFFSET);
		if (paramHourOffset != 0.0)
			pConfig->hourOffset = (int)paramHourOffset;
		
		double paramWeeklyATRValidation = parameter(AUTOBBS_BBS_ENABLE_WEEKLY_ATR_VALIDATION);
		/* Always use parameter value: > 0 = TRUE, <= 0 = FALSE (ignores symbol default if parameter not set) */
		pConfig->enableWeeklyATRValidation = (paramWeeklyATRValidation > 0.0) ? TRUE : FALSE;
		
	}
}

/**
 * @brief Unified 4H BBS Swing BoDuan strategy with symbol-specific configuration.
 * 
 * This function implements a unified 4H swing BoDuan strategy that:
 * - Detects KeyK patterns on 4H bars (close near high/low with significant movement)
 * - Uses MA trend on 4H timeframe
 * - Enters trades when 4H BBS trend aligns with KeyK/MA trend
 * - Supports symbol-specific stop loss and timing configurations
 * - Optional weekly ATR validation
 * 
 * Algorithm:
 * 1. Determine daily trend direction.
 * 2. Filter execution timeframe.
 * 3. Configure symbol-specific stop loss.
 * 4. On 4H bar boundaries (configurable hour offset):
 *    a. Calculate MA trend using 4H ATR
 *    b. Calculate 4H bar movement (high - low)
 *    c. Set atr_euro_range using configurable weekly ATR factor
 *    d. Detect KeyK pattern if movement >= atr_euro_range
 *    e. Determine 4H trend from MA trend and KeyK pattern
 *    f. Enter BUY if 4H trend and BBS trend are bullish
 *    g. Enter SELL if 4H trend and BBS trend are bearish
 * 5. Optionally validate entry against weekly price gap.
 * 
 * Configuration Parameters:
 * - AUTOBBS_BBS_WEEKLY_ATR_FACTOR: Weekly ATR factor (default: 0.4)
 * - AUTOBBS_BBS_4H_START_HOUR_OFFSET: 4H bar start hour offset (default: 0, XAUUSD uses 1)
 * - AUTOBBS_BBS_ENABLE_WEEKLY_ATR_VALIDATION: Enable weekly ATR validation (default: 0, XAUUSD uses 1)
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int orderIndex = -1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	// Initialize symbol-specific configuration (includes parameter overrides)
	BBSSymbolConfig config;
	initializeSymbolConfig(&config, pParams, pBase_Indicators);
	
	// Extract configuration values for use in main logic
	double weeklyATRFactor = config.weeklyATRFactor;
	int hourOffset = config.hourOffset;
	int enableWeeklyATRValidation = config.enableWeeklyATRValidation ? 1 : 0;
	
	// Set stop loss from symbol configuration
	pIndicators->stopLoss = config.stopLoss;

	// Process on 4H bar boundaries (with configurable hour offset)
	if ((timeInfo1.tm_hour - hourOffset) % HOURS_PER_4H_BAR == 0 && timeInfo1.tm_min < MINUTE_THRESHOLD_FOR_4H_BAR)
	{
		// ATR mode: use daily ATR for take profit
		pIndicators->splitTradeMode = SPLIT_TRADE_MODE_4H_SWING_100P;
		pIndicators->tpMode = TP_MODE_DAILY_ATR;

		// Calculate MA trend using 4H ATR
		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_FOR_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

		// Calculate 4H bar movement
		movement = fabs(high_4H - low_4H);

		// Set atr_euro_range using configurable weekly ATR factor if not already set
		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR * weeklyATRFactor;

		logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);

		// Detect KeyK pattern: close near high/low with significant movement
		if (movement >= pIndicators->atr_euro_range)
		{
			// Bullish KeyK: close near high
			if (fabs(high_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
			{
				trend_KeyK = 1;
			}
			// Bearish KeyK: close near low
			if (fabs(low_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
			{
				trend_KeyK = -1;
			}
		}

		// Determine 4H trend from MA trend and KeyK pattern
		if (trend_MA > 0 || trend_KeyK == 1)
			trend_4H = 1;
		if (trend_MA < 0 || trend_KeyK == -1)
			trend_4H = -1;

		// Get order index once for efficiency
		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		// Enter BUY order if 4H trend and BBS trend are bullish
		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pIndicators->stopLoss);

			// Enter if no existing BUY order
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY))
			{
				pIndicators->entrySignal = 1;
			}

			pIndicators->exitSignal = EXIT_SELL;
		}

		// Enter SELL order if 4H trend and BBS trend are bearish
		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pIndicators->stopLoss);

			// Enter if no existing SELL order
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL))
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}

		// Optionally validate entry against weekly price gap
		if (enableWeeklyATRValidation && pIndicators->entrySignal != 0 &&
			fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) > pBase_Indicators->pWeeklyPredictATR)
		{
			sprintf(pIndicators->status, "current weekly price gap %lf is greater than pWeeklyPredictATR %lf",
				fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice), pBase_Indicators->pWeeklyPredictATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}
	}
	return SUCCESS;
}

/**
 * @brief Wrapper for 4H BBS Swing XAUUSD BoDuan strategy (backward compatibility).
 * 
 * This function is a wrapper that calls the unified workoutExecutionTrend_4HBBS_Swing_BoDuan
 * function. The unified function automatically detects XAUUSD and applies appropriate
 * configuration (hour offset=1, weekly ATR validation enabled).
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	// Call the unified function - it will auto-detect XAUUSD and apply appropriate settings
	return workoutExecutionTrend_4HBBS_Swing_BoDuan(pParams, pIndicators, pBase_Indicators);
}

/**
 * @brief Executes standard 4H BBS Swing strategy with trend filtering.
 * 
 * This function implements the standard 4H swing strategy that:
 * - Uses 4H BBS trend for entry signals
 * - Filters entries based on execution timeframe BBS trend
 * - Adjusts risk for range-bound markets
 * - Validates entries against weekly ATR
 * 
 * Algorithm:
 * 1. Determine daily trend direction.
 * 2. Filter execution timeframe.
 * 3. Calculate MA trend and KeyK pattern on 4H bars.
 * 4. Determine 4H trend.
 * 5. For BUY signals:
 *    a. Check if 4H BBS trend is bullish
 *    b. Validate against execution timeframe BBS trend
 *    c. Check weekly ATR constraint
 *    d. Adjust risk for range markets
 * 6. For SELL signals:
 *    a. Check if 4H BBS trend is bearish
 *    b. Validate against execution timeframe BBS trend
 *    c. Check weekly ATR constraint
 *    d. Adjust risk for range markets
 * 7. Apply profit management.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	// ATR mode: use daily ATR for take profit
	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_4H_SWING;
	pIndicators->tpMode = TP_MODE_DAILY_ATR;

	// Calculate MA trend using 4H ATR
	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, ATR_PERIOD_FOR_MA_TREND, 1), B_FOURHOURLY_RATES, 1);

	// Calculate 4H bar movement
	movement = fabs(high_4H - low_4H);
	
	// Set atr_euro_range to 40% of predicted weekly ATR if not already set
	if (pIndicators->atr_euro_range == 0)
		pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR * WEEKLY_ATR_FACTOR_FOR_RANGE;

	logInfo("System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);

	// Detect KeyK pattern: close near high/low with significant movement
	if (movement >= pIndicators->atr_euro_range)
	{
		// Bullish KeyK: close near high
		if (fabs(high_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
		{
			trend_KeyK = 1;
		}
		// Bearish KeyK: close near low
		if (fabs(low_4H - close_4H) < movement / KEYK_CLOSE_THRESHOLD_DIVISOR)
		{
			trend_KeyK = -1;
		}
	}

	// Determine 4H trend from MA trend and KeyK pattern
	if (trend_MA > 0 || trend_KeyK == 1)
		trend_4H = 1;
	if (trend_MA < 0 || trend_KeyK == -1)
		trend_4H = -1;

	// Handle BUY signals when 4H BBS trend is bullish
	if (pIndicators->bbsTrend_4H == 1)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pBase_Indicators->dailyS);

		// Entry conditions: execution BBS trend bullish, 4H trend bullish, and weekly ATR constraint met
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_4H, currentTime)
			&& trend_4H == 1
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = 1;
		}

		// Reduce risk for range-bound markets
		if (dailyTrend == 0
			&& pIndicators->entryPrice >= pBase_Indicators->dailyS
			&& fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) >= pBase_Indicators->dailyATR * ATR_FACTOR_FOR_RANGE_RISK)
		{
			pIndicators->risk = RISK_REDUCTION_RANGE_TREND;
		}

		// Invalidate entry if stop loss is above entry price
		if (pIndicators->stopLossPrice > pIndicators->entryPrice)
		{
			pIndicators->entrySignal = 0;
		}
	}

	// Handle SELL signals when 4H BBS trend is bearish
	if (pIndicators->bbsTrend_4H == -1)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		
		// Set stop loss based on daily trend
		if (dailyTrend == 0)
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyR);
		else
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyS);

		// Entry conditions: execution BBS trend bearish, 4H trend bearish, and weekly ATR constraint met
		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_4H, currentTime)
			&& trend_4H == -1
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = -1;
		}

		// Reduce risk for range-bound markets
		if (dailyTrend == 0 && pIndicators->entryPrice <= pBase_Indicators->dailyR
			&& fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) >= pBase_Indicators->dailyATR * ATR_FACTOR_FOR_RANGE_RISK)
		{
			pIndicators->risk = RISK_REDUCTION_RANGE_TREND;
		}

		// Reduce risk if stop loss is below entry price (invalid configuration)
		if (pIndicators->stopLossPrice < pIndicators->entryPrice)
		{
			pIndicators->risk = RISK_REDUCTION_RANGE_TREND;
		}
	}

	// Apply profit management
	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}
