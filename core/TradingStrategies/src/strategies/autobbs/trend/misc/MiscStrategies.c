/*
 * Miscellaneous Strategy Module
 * 
 * Provides various miscellaneous strategy execution functions including:
 * - KeyK strategy: Uses intraday key high/low levels for entry signals
 * - KongJian strategy: Middle phase trading based on intraday price movement
 * - DailyOpen strategy: Enters trades at daily open based on trend phase
 * - Pivot strategy: Uses pivot point breakouts for entry signals
 * - Auto strategy: Dispatcher that routes to appropriate strategy based on trend phase
 * - MIDDLE_RETREAT_PHASE strategy: Enters on price retreats during middle phase
 * - ASI strategy: Uses Accumulation Swing Index for trend determination
 * 
 * NOTE: The 4H Shellington strategy has been moved to its own module:
 *       strategies/autobbs/trend/shellington/Shellington4HStrategy.c
 * 
 * NOTE: Range order functions (entryBuyRangeOrder, entrySellRangeOrder, isRangeOrder) are
 * now imported from RangeOrderManagement.c. The duplicate static functions have been removed.
 * 
 * NOTE: Time management function (DailyTrade_Limit_Allow_Trade) is now imported from
 * TimeManagement.c. The duplicate static function has been removed.
 * 
 * NOTE: Stop loss management function (move_stop_loss) is now imported from
 * StopLossManagement.c. The duplicate static function has been removed.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/trend/misc/MiscStrategies.h"
#include "strategies/autobbs/trend/bbs/BBSBreakOutStrategy.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"
#include "strategies/autobbs/trend/common/StopLossManagement.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"
#include "strategies/autobbs/trend/common/StopLossManagement.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"

// Strategy configuration constants
#define RISK_DOUBLE 2                        // Double risk (200%)
#define SPLIT_TRADE_MODE_KEYK 9              // Split trade mode for KeyK strategy
#define TP_MODE_RATIO_1_TO_1 0              // Take profit mode: 1:1 ratio
#define RISK_FULL 1                          // Full risk (100%)
#define SPLIT_TRADE_MODE_STANDARD 0          // Standard split trade mode
#define SPLIT_TRADE_MODE_DAILY_OPEN 6        // Split trade mode for daily open strategy

// ATR and distance constants
#define ATR_FACTOR_FOR_RISK_ADJUSTMENT 0.5  // ATR factor for risk adjustment (50%)
#define ATR_DIVISOR_FOR_ENTRY_CHECK 3       // ATR divisor for entry price check
#define ATR_DIVISOR_FOR_PENDING_CHECK 3     // ATR divisor for pending order check
#define ATR_FACTOR_FOR_RETREAT_DISTANCE 0.666 // ATR factor for retreat distance check (66.6%)

// Time constants
#define HOUR_BEFORE_END_OF_DAY 23           // Hour before end of day
#define DAILY_OPEN_HOUR 1                    // Hour for daily open entry
#define TRADING_START_HOUR 2                 // Start hour for trading
#define TRADING_END_HOUR 22                  // End hour for trading
#define EURO_SESSION_START_HOUR 17           // Start hour for Euro session
#define XAUUSD_KEY_DATE_HOUR 19              // Hour for XAUUSD key date check
#define XAUUSD_KEY_DATE_MINUTE 25            // Minute for XAUUSD key date check

// Trend strength constants
#define MIN_DAILY_TREND_STRENGTH_UP 3       // Minimum daily trend strength for UP entry
#define MIN_DAILY_TREND_STRENGTH_DOWN -3    // Minimum daily trend strength for DOWN entry

// Moving average period
#define MA_BASELINE_PERIOD 50               // MA period for baseline calculation

/**
 * @brief Executes KeyK strategy based on intraday key high/low levels.
 * 
 * This function uses intraday key high/low levels to determine entry signals.
 * It enters trades when the intraday trend aligns with the key level breakout.
 * 
 * Algorithm:
 * 1. Get key high/low from intraday index bar.
 * 2. For intraday uptrend:
 *    - Enter BUY if side >= 0 and intraday index matches shift1Index.
 *    - Adjust risk based on stop loss distance from daily low.
 * 3. For intraday downtrend:
 *    - Enter SELL if side <= 0 and intraday index matches shift1Index.
 *    - Adjust risk based on stop loss distance from daily high.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing intraday trend and index.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	double keyHigh, keyLow;

	pIndicators->risk = RISK_DOUBLE;
	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_KEYK;
	pIndicators->tpMode = TP_MODE_RATIO_1_TO_1;

	// Get key high/low from intraday index bar
	keyHigh = iHigh(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);
	keyLow = iLow(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);

	// Handle intraday uptrend
	if (pBase_Indicators->intradayTrend == 1)
	{
		if (pIndicators->side >= 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->exitSignal = EXIT_SELL;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = keyLow;
			
			// Reduce risk if stop loss is far from daily low
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyLow) >= ATR_FACTOR_FOR_RISK_ADJUSTMENT * pBase_Indicators->dailyATR)
				pIndicators->risk = RISK_FULL;

			// Enter if intraday index matches shift1Index
			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = 1;
			}
		}
		else
		{
			pBase_Indicators->intradayTrend = 0;
		}
	}

	// Handle intraday downtrend
	if (pBase_Indicators->intradayTrend == -1)
	{
		if (pIndicators->side <= 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->exitSignal = EXIT_BUY;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = keyHigh;
			
			// Reduce risk if stop loss is far from daily high
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyHigh) >= ATR_FACTOR_FOR_RISK_ADJUSTMENT * pBase_Indicators->dailyATR)
				pIndicators->risk = RISK_FULL;

			// Enter if intraday index matches shift1Index
			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = -1;
			}
		}
		else
		{
			pBase_Indicators->intradayTrend = 0;
		}
	}

	return returnCode;
}

/**
 * @brief Executes KongJian (space/range) strategy for middle phase trading.
 * 
 * This function enters trades during middle phase when price has moved significantly
 * from the intraday low/high, indicating a potential continuation of the trend.
 * 
 * Algorithm:
 * 1. Close negative positions.
 * 2. Filter execution timeframe.
 * 3. For MIDDLE_UP_PHASE:
 *    - Enter BUY if price has moved > 1/3 ATR from intraday low and hour < 23.
 * 4. For MIDDLE_DOWN_PHASE:
 *    - Enter SELL if price has moved > 1/3 ATR from intraday high and hour < 23.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend phase and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double intradayClose = iClose(B_DAILY_RATES, 0);
	double intradayHigh = iHigh(B_DAILY_RATES, 0);
	double intradayLow = iLow(B_DAILY_RATES, 0);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	
	pIndicators->risk = RISK_FULL;
	pIndicators->tpMode = TP_MODE_RATIO_1_TO_1;
	pIndicators->splitTradeMode = SPLIT_TRADE_MODE_STANDARD;

	// Handle MIDDLE_UP_PHASE
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{		
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);
		
		// Enter if price has moved significantly from intraday low and before end of day
		if (pParams->bidAsk.ask[0] - intradayLow > pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_ENTRY_CHECK 
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime) 
			&& timeInfo1.tm_hour < HOUR_BEFORE_END_OF_DAY)
		{
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;		
	}

	// Handle MIDDLE_DOWN_PHASE
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);
		
		// Enter if price has moved significantly from intraday high and before end of day
		if (intradayHigh - pParams->bidAsk.bid[0] > pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_ENTRY_CHECK 
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime) 
			&& timeInfo1.tm_hour < HOUR_BEFORE_END_OF_DAY)
		{			
			pIndicators->entrySignal = -1;
		}	
		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_DailyOpen(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);


	// Close negative positions with special handling for XAUUSD key dates
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= XAUUSD_KEY_DATE_HOUR && timeInfo1.tm_min >= XAUUSD_KEY_DATE_MINUTE)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
	{
		closeAllWithNegativeEasy(1, currentTime, 3);
	}

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 6;


	//pIndicators->tradeMode = 1;

	// Enter order on key support/resistance levels
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		// Entry conditions: strong trend, entry price above support, at daily open hour, BBS trend bullish
		if (pBase_Indicators->dailyTrend >= MIN_DAILY_TREND_STRENGTH_UP
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust			
			&& timeInfo1.tm_hour == DAILY_OPEN_HOUR
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime)))
			)
		{
			pIndicators->entrySignal = 1;
		}

		// Override entry signal if current daily low has been lower than stop loss price
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
		{
			pIndicators->entrySignal = 0;
		}

		pIndicators->exitSignal = EXIT_SELL;
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		// Entry conditions: strong trend, entry price below support, at daily open hour, BBS trend bearish
		if (pBase_Indicators->dailyTrend <= MIN_DAILY_TREND_STRENGTH_DOWN
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& timeInfo1.tm_hour == DAILY_OPEN_HOUR
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime))
			))
		{
			pIndicators->entrySignal = -1;
		}

		// Override entry signal if current daily high has been higher than stop loss price
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
		{
			pIndicators->entrySignal = 0;
		}

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,BOOL ignored)
{	
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	
	// Close negative positions with special handling for XAUUSD key dates
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= XAUUSD_KEY_DATE_HOUR && timeInfo1.tm_min >= XAUUSD_KEY_DATE_MINUTE)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
	{
		closeAllWithNegativeEasy(1, currentTime, 3);
	}
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 6;
	pIndicators->subTradeMode = 1;
	
	//pIndicators->tradeMode = 1;

	// Enter order on key support/resistance levels
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0 ))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& preLow < pBase_Indicators->dailyPivot
			&& preClose > pBase_Indicators->dailyPivot
			&& timeInfo1.tm_hour >= TRADING_START_HOUR && timeInfo1.tm_hour <= TRADING_END_HOUR
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime)))
			)
		{
			pIndicators->entrySignal = 1;
			logInfo("System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_Pivot.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& preHigh > pBase_Indicators->dailyPivot
			&& preClose < pBase_Indicators->dailyPivot
			&& timeInfo1.tm_hour >= TRADING_START_HOUR && timeInfo1.tm_hour <= TRADING_END_HOUR
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime))
			))
		{
			pIndicators->entrySignal = -1;

			logInfo("System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_Pivot.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;	
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	struct tm timeInfo1;

	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int execution_tf, euro_index_rate, count;
	double ATR0_EURO = 10;

	double intradayClose = iClose(B_PRIMARY_RATES, 0);
	
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;

	//Check out if Key K is triggered. If yes, take piority with other strategies., 
	//TODO: Need to add riskPNL strategy, need to implement
	if ((int)parameter(AUTOBBS_KEYK) == 1)
	{
		workoutExecutionTrend_KeyK(pParams, pIndicators, pBase_Indicators);
		if (pIndicators->entrySignal != 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == 1 && pBase_Indicators->dailyTrend_Phase < 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == -1 && pBase_Indicators->dailyTrend_Phase > 0)
			return SUCCESS;
	}

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:		
		if (pIndicators->tradeMode == 0)
			workoutExecutionTrend_DailyOpen(pParams, pIndicators, pBase_Indicators, TRUE);
		else
			workoutExecutionTrend_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);

		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal		
			workoutExecutionTrend_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, FALSE);

		pIndicators->splitTradeMode = 0;	

		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		if (pIndicators->tradeMode == 0)
			workoutExecutionTrend_DailyOpen(pParams, pIndicators, pBase_Indicators, TRUE);
		else
			workoutExecutionTrend_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);

		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal			
			workoutExecutionTrend_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);

		if ((int)parameter(AUTOBBS_LONG_SHORT_MODE) == 1)
			pIndicators->splitTradeMode = 0;		
		else
			pIndicators->splitTradeMode = 6;		

		break;
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
		workoutExecutionTrend_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators);
		break;
	case RANGE_PHASE:
		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			if (pBase_Indicators->weeklyMATrend == RANGE)
				pIndicators->exitSignal = EXIT_ALL;				
			else
			{
				//closeAllWithNegativeEasy(1, currentTime, 3);

				if (totalOpenOrders(pParams, BUY) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->dailyS) + pIndicators->adjust;
					modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
				}
				if (totalOpenOrders(pParams, SELL) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->dailyR) + pIndicators->adjust;
					modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
				}
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	// Validate entry signals against MA baseline
	if (pIndicators->entrySignal == 1 && iClose(B_DAILY_RATES, 1) < iMA(3, B_DAILY_RATES, MA_BASELINE_PERIOD, 1))
		pIndicators->entrySignal = 0;

	if (pIndicators->entrySignal == -1 && iClose(B_DAILY_RATES, 1) > iMA(3, B_DAILY_RATES, MA_BASELINE_PERIOD, 1))
		pIndicators->entrySignal = 0;

	//Asia hours
	//if (timeInfo1.tm_hour < 8 )
	//{
	//	pIndicators->tradeMode = 0;
	//}

	//if ((int)parameter(AUTOBBS_TP_MODE) < 0
	//	//|| fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) >= pBase_Indicators->dailyATR
	//	)
	//{
	//	pIndicators->tradeMode = 0;
	//}


	//if ((pIndicators->entrySignal == 1 && pBase_Indicators->weeklyMATrend != UP_NORMAL)
	//	|| (pIndicators->entrySignal == -1 && pBase_Indicators->weeklyMATrend != DOWN_NORMAL)
	//	)
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s, tradeMode = %ld",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->tradeMode);

	//	pIndicators->tradeMode = 0;
	//}

	// If MACD doesn't support, then don't trade short term
	//fast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 1);

	//if ((pIndicators->entrySignal == 1 && fast <= 0)
	//	|| (pIndicators->entrySignal == -1 && fast >= 0)
	//	)
	//volume1 = iVolume(B_DAILY_RATES, 1);
	//volume2 = iVolume(B_DAILY_RATES, 2);
	//
	//if ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
	//	&& pIndicators->entrySignal != 0 && volume1 > volume2)
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s, volume1 = %lf, volume2 = %lf,tradeMode = %ld",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, volume1, volume2, pIndicators->tradeMode);

	//	pIndicators->tradeMode = 0;
	//}

	// After Euro session start (17:00), if the current day hasn't exceeded the intraday range, close all short term orders
	if (timeInfo1.tm_hour >= EURO_SESSION_START_HOUR)
	{
		execution_tf = (int)pParams->settings[TIMEFRAME];
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - EURO_SESSION_START_HOUR) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (EURO_SESSION_START_HOUR - 1) * (60 / execution_tf) - 1;
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);

		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);

		logInfo("System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO, pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);

		if (ATR0_EURO < pIndicators->atr_euro_range)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);					
	}


	// Calculate intraday support/resistance levels and daily range
	//// 1: current day; 2: current week

	//count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
	//if (count > 1)
	//	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index - 1, count, &intradayHigh, &intradayLow);
	//else
	//	return SUCCESS;

	//intradayLow = min(close_prev1, intradayLow);
	//intradayHigh = max(close_prev1, intradayHigh);
	//ATR0 = fabs(intradayHigh - intradayLow);
	//
	//if (ATR0 > pBase_Indicators->pDailyMaxATR)
	//{
	//	logInfo("System InstanceID = %d, BarTime = %s, ATR0 =%lf,pDailyMaxATR = %lf close all current short term orders",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, pBase_Indicators->pDailyMaxATR);
	//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	//}


	// Use MACD to exit long term trades

	//Load MACD
	/*
	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min <= 3)
	{
		fast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 1);
		slow = iMACD(B_DAILY_RATES, 5, 10, 5, 1, 1);

		//preFast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 2);
		//preSlow = iMACD(B_DAILY_RATES, 5, 10, 5, 1, 2);

		preClose1 = iClose(B_DAILY_RATES, 1);
		preClose2 = iClose(B_DAILY_RATES, 2);
		preClose3 = iClose(B_DAILY_RATES, 3);
		preClose4 = iClose(B_DAILY_RATES, 4);
		preClose5 = iClose(B_DAILY_RATES, 5);

		if (fast > slow ||
			(preClose5 < preClose4 && preClose5 < preClose3 && preClose5 < preClose2 && preClose5 < preClose1)) //If no new low in the last 4 days, close long term short trades
		{
			logWarning("Exit Long term short trade:System InstanceID = %d, BarTime = %s, fast =%lf,slow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, fast, slow);

			closeAllLongTermShortsEasy();
		}

		if (fast < slow ||
			(preClose5 > preClose4 && preClose5 > preClose3 && preClose5 > preClose2 && preClose5 > preClose1)) //If no new high in the last 4 days, close long term long trades
		{
			logWarning("Exit Long term long trade:System InstanceID = %d, BarTime = %s, fast =%lf,slow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, fast, slow);
			closeAllLongTermLongsEasy();
		}
	}
	*/
	profitManagement(pParams, pIndicators, pBase_Indicators);


	

	
	return SUCCESS;
}

// Use BBS, entry on 15M BBS, stop loss at key support/resistance levels, and only S2/R2 only
// Risk management and position control, don't enter if risk too high
// Close all orders at 23:45

AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	double breakingHigh, breakingLow;
	double intraHigh = 99999, intraLow = -99999;
	int count = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Close negative positions with special handling for XAUUSD key dates
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= XAUUSD_KEY_DATE_HOUR && timeInfo1.tm_min >= XAUUSD_KEY_DATE_MINUTE)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
	{
		closeAllWithNegativeEasy(1, currentTime, 3);
	}
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	pIndicators->subTradeMode = 2;

	//pIndicators->tradeMode = 1;

	//TODO: 
	// if price is retreated back between the high low of breaking 15M bar, if it has a good room to entry
	breakingHigh = iHigh(B_PRIMARY_RATES, shift0Index - pIndicators->bbsIndex_excution);
	breakingLow = iLow(B_PRIMARY_RATES, shift0Index - pIndicators->bbsIndex_excution);
	count = shift1Index - pIndicators->bbsIndex_excution;
	if (count >= 2)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index, count, &intraHigh, &intraLow);

	// Enter order on key support/resistance levels
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_RETREAT_PHASE )
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * ATR_FACTOR_FOR_RETREAT_DISTANCE
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime)
			)
		{
			pIndicators->entrySignal = 1;
			logInfo("System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_MIDDLE_RETREAT_PHASE.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE )
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * ATR_FACTOR_FOR_RETREAT_DISTANCE
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / ATR_DIVISOR_FOR_PENDING_CHECK, currentTime)
			)
		{
			pIndicators->entrySignal = -1;
			logInfo("System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_MIDDLE_RETREAT_PHASE.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		pIndicators->exitSignal = EXIT_BUY;

	}

	return SUCCESS;
}

// workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE is declared and defined in WeeklyAutoStrategy.c
// Removed duplicate definition - using the one from WeeklyAutoStrategy.c

/*
 * Strategy description:
 * - Uses 15M BBS breakout
 * - 4H timeframe with MA50 and MA200
 * - Stop loss: no exit on stop loss
 * - Take profit: second level high/low ATR
 * - Predicted stop loss breakout movement
 */

/*
1. Check Trend by MACD and Shellington and default
2. If all of them are same, it think it will be in strong trend.
3. It will entre order on key supports. (R1/S1 or pivot or ATR daily range)
3. TP is 1H ATR(20)
4. SL is 3 * TP
5. Lot size is caculated by TP, Risk = 0.3%
6. How to close order ealier? (By time?)
*/

// NOTE: This is a duplicate function name - the first workoutExecutionTrend_Auto at line 319
// is a simpler dispatcher. This extended version should be renamed or merged.
// For now, renamed to avoid compilation error.
AsirikuyReturnCode workoutExecutionTrend_Auto_Extended(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose, preDailyHigh, preDailyLow, preDailyOpen;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int shift1Index_Daily;
	int autoMode = 0;
	BOOL isCloseOrdersEOD = FALSE;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double macdMaxLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE); 
	int orderIndex = -1;
	double highPrice, lowPrice;
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);	
	BOOL isMoveTPInNewDay = TRUE;
	int closeHour = 23, startHour = pIndicators->startHour;
	BOOL isEnableRangeTrade = FALSE;
	int trend = UNKNOWN;
	int truningPointIndex = -1, minPointIndex = -1;
	double isMACDBeili = FALSE;
	double totalLossPoint = 0;
	int totalLossTimes = 0;
	double realTakePrice;
	BOOL isEnableMACDSlow = TRUE;
	BOOL isEnableShellingtonTrend = FALSE;	
	BOOL isEnableFlatTrend = FALSE;
	BOOL isEnableWeeklyATR = TRUE;
	BOOL isEnableTooFar = FALSE;
	int stopHour = 23;		
	double tooFarLimit = 1;
	int barState = BAR_UNKNOWN;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;
	double rsi = 0.0, rsiLow = 20.0, rsiHigh = 80.0;
	int tradingDays = 10;
	int atrTime = 20;
	Order_Turning_Info orderTurningInfo;
	int closeOrderIndex = -1;
	struct tm timeInfo2, timeInfo3;
	double moveTPLimit = 0.0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		if (XAUUSD_not_full_trading_day(pParams, pIndicators, pBase_Indicators) == TRUE)
		{
			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			return SUCCESS;
		}



		isEnableShellingtonTrend = TRUE;		
		isEnableTooFar = FALSE;
	}	
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		pIndicators->adjust = pBase_Indicators->dailyATR * 0.01;

		startHour = 0;
		pIndicators->startHourOnLimt = startHour;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		if (timeInfo1.tm_wday == 1 || timeInfo1.tm_wday == 2)
			pIndicators->risk = 0.5;

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		isEnableWeeklyATR = FALSE;

		//pIndicators->isEnableLimitSR1 = TRUE;

		//if (timeInfo1.tm_wday == 6)
		//	stopHour = 16;

		tooFarLimit = 2;

		isEnableRangeTrade = FALSE;
		fastMAPeriod = 7;
		slowMAPeriod = 14;
		signalMAPeriod = 7;

		if (timeInfo1.tm_wday == 0)
			atrTime = 50;

	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		stopHour = 17;

		isEnableMACDSlow = FALSE;
		isEnableFlatTrend = TRUE;

		//isEnableRangeTrade = TRUE;
		pIndicators->startHourOnLimt = pIndicators->startHour;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE)
		pIndicators->adjust = 0;


	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
		startShift = 0;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	//rsi = iRSI(B_DAILY_RATES, 14, 1);
	//isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	preDailyHigh = iHigh(B_DAILY_RATES, startShift);
	preDailyLow = iLow(B_DAILY_RATES, startShift);
	preDailyOpen = iOpen(B_DAILY_RATES, startShift);

	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;


	if (pIndicators->fast > 0
		&& (isEnableMACDSlow || pIndicators->fast > pIndicators->slow)
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& (isEnableMACDSlow || pIndicators->fast < pIndicators->slow)
		&& preDailyClose < dailyBaseLine
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	pBase_Indicators->shellingtonInTrend = trend_4H;

	if (fabs(iMA(3, B_DAILY_RATES, 20, 1) - iMA(3, B_DAILY_RATES, 20, 5)) / pBase_Indicators->dailyATR <= 0.05)
		pBase_Indicators->flatTrend = 1;
	else
		pBase_Indicators->flatTrend = 0;

	if (pBase_Indicators->dailyTrend > 0 &&
		pBase_Indicators->mACDInTrend == 1 //&& isMACDBeili == FALSE		
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == 1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BULL
		)
	{
		trend = UP;
	}
	else if (pBase_Indicators->dailyTrend < 0 &&
		pBase_Indicators->mACDInTrend == -1 //&& isMACDBeili == FALSE	
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == -1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BEAR
		)
	{
		trend = DOWN;
	}
	else
		trend = RANGE;

	pIndicators->takePrice = iAtr(B_HOURLY_RATES, atrTime, 1);
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& iAtr(B_DAILY_RATES, 1, 0) >= pBase_Indicators->pDailyMaxATR)
	//{
	//	if (pParams->orderInfo[orderIndex].type == SELL)
	//		closeShortEasy(pParams->orderInfo[orderIndex].ticket);
	//	if (pParams->orderInfo[orderIndex].type == BUY)
	//		closeLongEasy(pParams->orderInfo[orderIndex].ticket);
	//}

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 3 || timeInfo1.tm_hour >= stopHour)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		if (isCloseOrdersEOD == TRUE && timeInfo1.tm_hour == 23 && timeInfo1.tm_min >= 50)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

		//If the order is executed on the spot of stopHour, just close it immedeiatly. 
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen)
		{
			safe_gmtime(&timeInfo3, pParams->orderInfo[orderIndex].openTime);
			if ((timeInfo3.tm_hour == stopHour && timeInfo3.tm_min < 5) || (timeInfo3.tm_hour == stopHour - 1 && timeInfo3.tm_min > 55))
			{
				if (pParams->orderInfo[orderIndex].type == SELL)
					closeShortEasy(pParams->orderInfo[orderIndex].ticket);
				if (pParams->orderInfo[orderIndex].type == BUY)
					closeLongEasy(pParams->orderInfo[orderIndex].ticket);
			}

		}

		return SUCCESS;
	}

	logInfo("System InstanceID = %d, BarTime = %s,startHour=%d,AUTOBBS_IS_AUTO_MODE=%d,isEnableRangeTrade=%d,pBase_dailyHigh=%lf,dailyLow=%lf,pDailyMaxATR=%lf,hourATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->startHour, (int)parameter(AUTOBBS_IS_AUTO_MODE), (int)parameter(AUTOBBS_RANGE), iHigh(B_DAILY_RATES, 0), iLow(B_DAILY_RATES, 0), pBase_Indicators->pDailyMaxATR, iAtr(B_HOURLY_RATES, 20, 1));
	totalLossTimes = getLossTimesInDayCloseOrderEasy(currentTime, &totalLossPoint);
	if (totalLossTimes >= 1)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		return SUCCESS;
	}

	if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour == pIndicators->startHourOnLimt && timeInfo1.tm_min < 7)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
	}

	iTrend3Rules(pParams, pBase_Indicators, B_DAILY_RATES, 2, &(pBase_Indicators->daily3RulesTrend), 0);

	///////////////////////////////////////////////	

	if (orderIndex < 0)
	{
		orderTurningInfo.isTurning = TRUE;
		saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
	}
	
	if (readTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo) == -1)
	{
		orderTurningInfo.isTurning = TRUE;
		saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
	}
	
	if (orderTurningInfo.isTurning == TRUE
		//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE
		&& closeOrderIndex >=0 
		&& pParams->orderInfo[closeOrderIndex].profit > 0
		)
	{
		if (fabs(pParams->orderInfo[closeOrderIndex].closePrice - pParams->orderInfo[closeOrderIndex].openPrice) / pIndicators->takePrice >= 2)
		{
			orderTurningInfo.type = pParams->orderInfo[closeOrderIndex].type;
			orderTurningInfo.isTurning = FALSE;
			saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
		}
	}
	else if (orderTurningInfo.isTurning == FALSE
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		&& pParams->orderInfo[orderIndex].type != orderTurningInfo.type
		)
	{		
		orderTurningInfo.type = pParams->orderInfo[orderIndex].type;
		orderTurningInfo.isTurning = TRUE;
		saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && orderTurningInfo.isTurning == FALSE)
		pIndicators->takePriceLevel = 1;
	///////////////////////////////////////////////

	if (timeInfo1.tm_hour >= startHour)
	{

		//if (DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		//	return SUCCESS;

		if (trend == UP)
		{
			if (pBase_Indicators->daily3RulesTrend == UP)
			{
				autoMode = 0;


				splitBuyOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);
			}
		}
		
		if (trend == DOWN)
		{
			if (pBase_Indicators->daily3RulesTrend == DOWN)
			{
				autoMode = 0;
				splitSellOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);
			}
		}

	}

	//move_tailing_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex);


	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE 
		&& orderTurningInfo.isTurning == FALSE
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 4 * 60
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) <= 6 * 60
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		)
	{
		//By default, AUTOBBS_TP_MODE = 1
		//If AUTOBBS_RANGE = 1 and hour >15, should set isMoveTP = 0, no need move TP.
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		if (timeInfo2.tm_hour >= 15
			&& (
			(pParams->orderInfo[orderIndex].type == BUY && pParams->orderInfo[orderIndex].openPrice < iHigh(B_DAILY_RATES, 0) - pBase_Indicators->pDailyMaxATR)
			|| (pParams->orderInfo[orderIndex].type == SELL && pParams->orderInfo[orderIndex].openPrice > iLow(B_DAILY_RATES, 0) + pBase_Indicators->pDailyMaxATR)
			)
			)
			isMoveTP = 0;

		//Move stopLoss when 1 TP is hit.

		realTakePrice = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / stopLossLevel;

		if (pParams->orderInfo[orderIndex].type == BUY)
		{
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice;
			pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;

			//if (pParams->bidAsk.ask[0] >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit > pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			if (pIndicators->entryPrice < pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = 1;
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iHigh(B_PRIMARY_RATES, 1) >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			{
				logInfo("closeLong type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeLongEasy(pParams->orderInfo[orderIndex].ticket);
			}

			return SUCCESS;
		}

		if (pParams->orderInfo[orderIndex].type == SELL)
		{
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice* stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice;
			pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;

			//if (pParams->bidAsk.bid[0] <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit< pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			if (pIndicators->entryPrice > pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = -1;
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iLow(B_PRIMARY_RATES, 1) <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			{
				logInfo("closeShort type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeShortEasy(pParams->orderInfo[orderIndex].ticket);
			}

			return SUCCESS;
		}

		if (isMoveTP == 1 || timeInfo1.tm_yday != timeInfo2.tm_yday)
		{

			if (pParams->orderInfo[orderIndex].type == BUY
				&& pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust < pParams->orderInfo[orderIndex].takeProfit
				&& pParams->orderInfo[orderIndex].openPrice - lowPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{

				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust;
				pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;
				return SUCCESS;
			}

			if (pParams->orderInfo[orderIndex].type == SELL
				&& pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust > pParams->orderInfo[orderIndex].takeProfit
				&& highPrice - pParams->orderInfo[orderIndex].openPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust;
				pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;
				return SUCCESS;
			}

		}
	}

	

	
	return SUCCESS;
}

static BOOL move_tailing_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex)
{
	OrderType side = pParams->orderInfo[orderIndex].type;

	if (side == BUY)
	{
		if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > 2* pIndicators->takePrice 
			//&& pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * pIndicators->takePrice
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}
		//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 3 * pIndicators->takePrice
		//	&& pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = 1;
		//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->takePrice;
		//}
		//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 4 * pIndicators->takePrice
		//	//&& pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = 1;
		//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + 2 * pIndicators->takePrice;
		//}
	}

	if (side == SELL)
	{
		if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > 2 * pIndicators->takePrice
			//&& pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->takePrice
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}
		//else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->takePrice
		//	&& pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = -1;
		//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->takePrice;
		//}
		//else if(pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 4 * pIndicators->takePrice
		//	//&& pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = -1;
		//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - 2 * pIndicators->takePrice;
		//}

	}
	return TRUE;
}



AsirikuyReturnCode workoutExecutionTrend_ASI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double asiBull, asiBear;

	int orderIndex;
	double stopLoss;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	//load pBase_Indicators
	pBase_Indicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;
	stopLoss = pBase_Indicators->pDailyMaxATR;

	logWarning("System InstanceID = %d, BarTime = %s, asiBull =%lf, asiBear=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, asiBull, asiBear);


	if (asiBull > asiBear)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

		if (orderIndex >= 0 &&
			pParams->orderInfo[orderIndex].type == BUY &&
			pParams->orderInfo[orderIndex].isOpen == TRUE &&
			pIndicators->stopLossPrice > pParams->orderInfo[orderIndex].openPrice)
		{

			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}

		if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
			&& iClose(B_DAILY_RATES, 1) > iClose(B_DAILY_RATES,2))
		{
			pIndicators->entrySignal = 1;
		}

		pIndicators->exitSignal = EXIT_SELL;
	}

	if (asiBull < asiBear)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];

		pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

		if (orderIndex >= 0 &&
			pParams->orderInfo[orderIndex].type == SELL &&
			pParams->orderInfo[orderIndex].isOpen == TRUE &&
			pIndicators->stopLossPrice < pParams->orderInfo[orderIndex].openPrice)
		{

			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}

		if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
			&& iClose(B_DAILY_RATES, 1) < iClose(B_DAILY_RATES, 2))
		{
			pIndicators->entrySignal = -1;
		}
		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

