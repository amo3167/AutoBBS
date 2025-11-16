/*
 * Strategy Execution Module
 * 
 * Provides strategy execution functions for workoutExecutionTrend.
 * Contains all workoutExecutionTrend_* functions that determine entry/exit signals
 * based on various trading strategies.
 * 
 * This module serves as the central dispatcher for all AutoBBS trading strategies.
 * It routes execution to specific strategy implementations based on the
 * AUTOBBS_TREND_MODE parameter, calculates risk metrics, and applies filters
 * for account risk limits and side restrictions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
// Include strategy convenience headers - these include all individual strategy headers
#include "strategies/autobbs/trend/TrendStrategy.h"  // Includes all trend strategy headers (BBS, Limit, MACD, Ichimoko, Weekly, ShortTerm, Misc)
#include "strategies/autobbs/swing/SwingStrategy.h"  // Includes all swing strategy headers (Weekly, DayTrading, MultipleDay, MACD_BEILI)
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/common/ProfitManagement.h"
#include "strategies/autobbs/shared/ordermanagement/OrderManagement.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"

// Commodity symbol prefixes that use start hour = 1
#define COMMODITY_SYMBOL_XAU "XAU"
#define COMMODITY_SYMBOL_XTI "XTI"
#define COMMODITY_SYMBOL_SPOTCRUDE "SpotCrude"
#define COMMODITY_SYMBOL_XAG "XAG"
#define COMMODITY_SYMBOL_XPD "XPD"
#define COMMODITY_START_HOUR 1
#define DEFAULT_START_HOUR 0

/**
 * Main strategy execution dispatcher.
 * 
 * This is the central entry point for all AutoBBS trading strategies. It:
 * 1. Calculates strategy risk metrics (market volatility risk, strategy risk, PNL risk)
 * 2. Routes execution to specific strategy functions based on AUTOBBS_TREND_MODE
 * 3. Applies account risk limits (prevents entry if account risk exceeds limit)
 * 4. Applies side filters (prevents entry if signal conflicts with AUTOBBS_ONE_SIDE setting)
 * 
 * @param pParams Strategy parameters containing rates, settings, and account info
 * @param pIndicators Strategy indicators structure to populate with execution signals
 * @param pBase_Indicators Base indicators structure containing trend and ATR data
 * @return SUCCESS on success, error code on failure
 * 
 * Strategy modes (AUTOBBS_TREND_MODE):
 * - 0: 4H BBS Swing
 * - 2: Limit orders
 * - 3: Limit orders with breakout on pivot
 * - 9: Limit orders with BBS
 * - 10: Weekly Auto
 * - 13: Weekly Swing New
 * - 14: Auto Hedge
 * - 15: XAUUSD Day Trading
 * - 16: GBPJPY Day Trading Ver2
 * - 17: Weekly ATR Prediction
 * - 18: 4H BBS Swing BoDuan
 * - 19: Day Trading Execution Only
 * - 20: 4H BBS Swing XAUUSD BoDuan
 * - 21: Multiple Day
 * - 22: Multiple Day V2
 * - 23: MACD Daily
 * - 24: MACD Weekly
 * - 25: Auto (with tradeMode = 1)
 * - 26: Limit BBS Long Term
 * - 27: Ichimoku Daily Index
 * - 28: Ichimoku Daily Index (duplicate)
 * - 30: 4H Shellington
 * - 31: Ichimoku Daily New
 * - 32: (removed - use 23 for MACD Daily)
 * - 33: MACD BEILI
 * - 34: Short Term
 * - 35: Ichimoku Weekly Index
 * - 36: (reserved/empty)
 * - 101: Ichimoku Daily Index Regression Test
 * - 102: ASI
 */
AsirikuyReturnCode workoutExecutionTrend(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	struct tm timeInfo;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int startHour = DEFAULT_START_HOUR;

	// Set start hour for commodity symbols (XAU, XTI, SpotCrude, XAG, XPD)
	if (strstr(pParams->tradeSymbol, COMMODITY_SYMBOL_XAU) != NULL || 
	    strstr(pParams->tradeSymbol, COMMODITY_SYMBOL_XTI) != NULL || 
	    strstr(pParams->tradeSymbol, COMMODITY_SYMBOL_SPOTCRUDE) != NULL || 
	    strstr(pParams->tradeSymbol, COMMODITY_SYMBOL_XAG) != NULL || 
	    strstr(pParams->tradeSymbol, COMMODITY_SYMBOL_XPD) != NULL)
	{
		startHour = COMMODITY_START_HOUR;
	}

	safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	// Calculate strategy risk metrics
	pIndicators->strategyMarketVolRisk = caculateStrategyVolRiskEasy(pBase_Indicators->dailyATR);
	pIndicators->strategyRisk = caculateStrategyRiskEasy(FALSE);
	pIndicators->strategyRiskWithoutLockedProfit = caculateStrategyRiskEasy(TRUE);
	pIndicators->riskPNL = caculateStrategyPNLEasy(FALSE);
	pIndicators->riskPNLWithoutLockedProfit = caculateStrategyPNLEasy(TRUE);

	switch ((int)parameter(AUTOBBS_TREND_MODE)) {
	case 0:
		workoutExecutionTrend_4HBBS_Swing(pParams, pIndicators, pBase_Indicators);
		break;	
	case 2:
		workoutExecutionTrend_Limit(pParams, pIndicators, pBase_Indicators);	
		break;
	case 3:
		workoutExecutionTrend_Limit_BreakOutOnPivot(pParams, pIndicators, pBase_Indicators);
		break;
	case 9:		
		workoutExecutionTrend_Limit_BBS(pParams, pIndicators, pBase_Indicators);
		break;
	case 10:
		workoutExecutionTrend_WeeklyAuto(pParams, pIndicators, pBase_Indicators);
		break;
	case 13:
		workoutExecutionTrend_Weekly_Swing_New(pParams, pIndicators, pBase_Indicators);
		break;
	case 14:
		workoutExecutionTrend_Auto_Hedge(pParams, pIndicators, pBase_Indicators);
		break;
	case 15:
		workoutExecutionTrend_XAUUSD_DayTrading(pParams, pIndicators, pBase_Indicators);
		break;
	case 16:
		workoutExecutionTrend_GBPJPY_DayTrading_Ver2(pParams, pIndicators, pBase_Indicators);
		break;
	case 17:
		workoutExecutionTrend_WeeklyATR_Prediction(pParams, pIndicators, pBase_Indicators);
		break;
	case 18:
		workoutExecutionTrend_4HBBS_Swing_BoDuan(pParams, pIndicators, pBase_Indicators);
		break;
	case 19:		
		workoutExecutionTrend_DayTrading_ExecutionOnly(pParams, pIndicators, pBase_Indicators);
		break;
	case 20:
		workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(pParams, pIndicators, pBase_Indicators);
		break;
	case 21:				
		workoutExecutionTrend_MultipleDay(pParams, pIndicators, pBase_Indicators);
		break;
	case 22:		
		workoutExecutionTrend_MultipleDay_V2(pParams, pIndicators, pBase_Indicators);
		break;
	case 23:
		workoutExecutionTrend_MACD_Daily(pParams, pIndicators, pBase_Indicators);
		break;
	case 24:
		workoutExecutionTrend_MACD_Weekly(pParams, pIndicators, pBase_Indicators);
		break;
	case 25:
		// Auto strategy with tradeMode = 1
		pIndicators->tradeMode = 1;
		workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);
		break;
	case 26:
		// Limit BBS Long Term strategy
		workoutExecutionTrend_Limit_BBS_LongTerm(pParams, pIndicators, pBase_Indicators);
		break;
	case 27:
		workoutExecutionTrend_Ichimoko_Daily_Index(pParams, pIndicators, pBase_Indicators);
		break;	
	case 28:
		workoutExecutionTrend_Ichimoko_Daily_Index(pParams, pIndicators, pBase_Indicators);
		break;
	case 30:
		// 4H Shellington strategy (using Ver2, Ver1 is deprecated)
		workoutExecutionTrend_4H_Shellington(pParams, pIndicators, pBase_Indicators);
		break;
	case 31:
		workoutExecutionTrend_Ichimoko_Daily_New(pParams, pIndicators, pBase_Indicators);
		break;
	case 33:
		workoutExecutionTrend_MACD_BEILI(pParams, pIndicators, pBase_Indicators);
		break;
	case 34:
		workoutExecutionTrend_ShortTerm(pParams, pIndicators, pBase_Indicators);
		break;
	case 35:
		workoutExecutionTrend_Ichimoko_Weekly_Index(pParams, pIndicators, pBase_Indicators);
		break;
	case 36:
		break;
	case 101:
		workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test(pParams, pIndicators, pBase_Indicators);
		break;
	case 102:
		workoutExecutionTrend_ASI(pParams, pIndicators, pBase_Indicators);
		break;
	}


	// Apply account risk limit filter (only in live trading, not backtesting)
	// If account risk exceeds maximum allowed, prevent new entries
	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && 
	    pParams->accountInfo.totalOpenTradeRiskPercent < parameter(AUTOBBS_MAX_ACCOUNT_RISK) * -1.0)
	{
		logWarning("System InstanceID = %d, BarTime = %s, Over max risk %lf, skip this entry signal = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, parameter(AUTOBBS_MAX_ACCOUNT_RISK), pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Filter out entry signals that conflict with AUTOBBS_ONE_SIDE setting
	// If side is restricted (1 = buy only, -1 = sell only) and entry signal conflicts, block entry
	if (pIndicators->side != 0 && pIndicators->entrySignal != 0 && pIndicators->side != pIndicators->entrySignal)
	{
		logWarning("System InstanceID = %d, BarTime = %s, Against Side = %ld, skip this entry signal = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->side, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	//// Dont enter trade on the new day bar, it is too risky and not reliable.
	//if (pIndicators->entrySignal != 0 && timeInfo.tm_hour == startHour && timeInfo.tm_min == 0)
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s Not allowed to trade on the firt bar of new day, skip this entry signal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	return SUCCESS;
}

// workoutExecutionTrend_Auto is implemented in MiscStrategies.c - removed duplicate

// XAUUSD Daily Stop Check - checks if orders should be closed after 15 minutes
static void XAUUSD_Daily_Stop_Check(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime, openTime;
	struct tm timeInfo1, timeInfo2;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double diffMins;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (pParams->orderInfo[0].ticket != 0 && pParams->orderInfo[0].isOpen == TRUE)
	{
		// Must pass 15M
		openTime = pParams->orderInfo[0].openTime;
		safe_gmtime(&timeInfo2, pParams->orderInfo[0].openTime);

		diffMins = difftime(currentTime, openTime) / 60;

		if (diffMins == 15)
		{
			if (pParams->orderInfo[0].type == BUY && pIndicators->bbsTrend_secondary == -1)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					logWarning("System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld\n\n",
										(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_BUY;
				}
			}
			if (pParams->orderInfo[0].type == SELL && pIndicators->bbsTrend_secondary == 1)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					logWarning("System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld\n\n",
										(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_SELL;
				}
			}
		}
	}
}

/*
XAUUSD 15M
1/2 risk  
1/2ATR,

1/2ATRstop 

*/
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_Daily_Swing(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);
double ATR0 = iAtr(B_DAILY_RATES, 1, 0);
int signal = 0;
double intradayHigh_exclude_current_bar = intradayHigh, intradayLow_exclude_current_bar = intradayLow;
int count;
double openOrderHigh, openOrderLow;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
safe_gmtime(&timeInfo1, currentTime);

// closeAllWithNegativeEasy(1, currentTime, 3);

pIndicators->splitTradeMode = 16;
pIndicators->risk = 1;
pIndicators->tpMode = 0;

if (pBase_Indicators->pDailyPredictATR < 10)
	return SUCCESS;

count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
if (count > 1)
	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh_exclude_current_bar, &intradayLow_exclude_current_bar);
// iSRLevels_close(pParams, pIndicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh_exclude_current_bar, &intradayLow_exclude_current_bar);

// if (ATR0 >= pBase_Indicators->pDailyATR / 2)

if (ATR0 < 5)
{
	if (timeInfo1.tm_hour == 7 && timeInfo1.tm_min < 3 && hasOpenOrder() == FALSE) // 7am
	{
		if (pIndicators->bbsTrend_primary == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.2;
			pIndicators->entrySignal = 1;
		}
		else if (pIndicators->bbsTrend_primary == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.2;
			pIndicators->entrySignal = -1;
		}
		return SUCCESS;
	}
}
else
{
	// pIndicators->risk = 0.5;
	if (hasOpenOrder() == FALSE)
	{
		if (pParams->bidAsk.ask[0] - intradayLow > 5) // Buy signal
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.2;

			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 10 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;
		}

		if (intradayHigh - pParams->bidAsk.bid[0] > 5) // Sell signal
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.2;
			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 10 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}
	}
	else
	{

		if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == SELL && pParams->bidAsk.ask[0] > openOrderHigh) // Buy signal
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.2;

			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;
		}

		if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == BUY && pParams->bidAsk.bid[0] < openOrderLow) // Sell signal
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.2;
			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}
	}
}
return SUCCESS;
}

// UP signal: Low < pivot and Close > pivot
// DOWN: signal: High> pivot and Close < pivot
// PrimaryRate is 1H
AsirikuyReturnCode workoutExecutionTrend_WeeklyPivot(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

double stopLoss;
double preHigh = iHigh(B_PRIMARY_RATES, 1);
double preLow = iLow(B_PRIMARY_RATES, 1);
double preClose = iClose(B_PRIMARY_RATES, 1);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

if (pBase_Indicators->weeklyTrend_Phase == RANGE_PHASE)
	pIndicators->executionTrend = 0;
else if (pBase_Indicators->weeklyTrend > 0)
	pIndicators->executionTrend = 1;
else if (pBase_Indicators->weeklyTrend < 0)
	pIndicators->executionTrend = -1;
else
	pIndicators->executionTrend = 0;

closeAllWithNegativeEasy(2, currentTime, 3);

if (pIndicators->executionTrend == 0)
{
	if (totalOpenOrders(pParams, BUY) > 0)
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->weeklyS) + pIndicators->adjust;
		modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, FALSE);
	}
	if (totalOpenOrders(pParams, SELL) > 0)
	{
		stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->weeklyR) + pIndicators->adjust;
		modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, FALSE);
	}
}

// [Comment removed - encoding corrupted]
if (pIndicators->executionTrend == 1)
{
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;

	if (pIndicators->entryPrice > pIndicators->stopLossPrice + pIndicators->adjust && preLow < pBase_Indicators->weeklyPivot && preClose > pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pIndicators->executionTrend == -1)
{
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;

	if (pIndicators->entryPrice < pIndicators->stopLossPrice - pIndicators->adjust && preHigh > pBase_Indicators->weeklyPivot && preClose < pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
		//&& pBase_Indicators->weeklyTrend_Phase < 0
		&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

	return SUCCESS;
}

// workoutExecutionTrend_WeeklyPivot - kept for potential future use
// workoutExecutionTrend_Hedge is implemented in HedgeStrategy.c - removed duplicate

// workoutExecutionTrend_KongJian is implemented in MiscStrategies.c - removed duplicate

// workoutExecutionTrend_WeeklyAuto is implemented in WeeklyAutoStrategy.c - removed duplicate

// Weekly retreat strategy, only for EUR, AUD, crossing currencies
AsirikuyReturnCode workoutExecutionTrend_WeeklyRetreat(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

if (pBase_Indicators->weeklyTrend_Phase > 1 && pBase_Indicators->dailyTrend_Phase <= 0)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_primary == 1 && pIndicators->bbsIndex_primary == shift1Index_primary && pIndicators->entryPrice > pBase_Indicators->ma4H200M + pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->ma4H200M) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase < -1 && pBase_Indicators->dailyTrend_Phase > 0)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_primary == -1 && pIndicators->bbsIndex_primary == shift1Index_primary && pIndicators->entryPrice < pIndicators->stopLossPrice - pIndicators->adjust && fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_BEGINNING_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
// int    shift1Index = pParams->ratesBuffers->rates[B_HOURLY_RATES].info.arraySize - 2;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(2, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 10;

if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}
return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

double stopLoss;

double preHigh = iHigh(B_SECONDARY_RATES, 1);
double preLow = iLow(B_SECONDARY_RATES, 1);
double preClose = iClose(B_SECONDARY_RATES, 1);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(2, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 11;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust && preLow < pBase_Indicators->weeklyPivot && preClose > pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust && preHigh > pBase_Indicators->weeklyPivot && preClose < pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(2, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 5;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_RETREAT_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->weeklyS) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->weeklyS) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_ManualBBS(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR, movement = 0;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int dailyTrend;
time_t currentTime;
struct tm timeInfo1;
int bbsTrend, bbsIndex;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

// [Comment removed - encoding corrupted]
if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	dailyTrend = 0;
else if (pBase_Indicators->dailyTrend > 0)
	dailyTrend = 1;
else if (pBase_Indicators->dailyTrend < 0)
	dailyTrend = -1;
else
	dailyTrend = 0;

pIndicators->splitTradeMode = 19;
pIndicators->tpMode = 1;

if (pIndicators->bbsTrend_primary == 1)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;

	if (pIndicators->bbsIndex_primary == shift1Index && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
	{
		pIndicators->entrySignal = 1;
	}
	pIndicators->exitSignal = EXIT_BUY;
}

if (pIndicators->bbsTrend_primary == -1)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;

	if (pIndicators->bbsIndex_primary == shift1Index && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
	{
		pIndicators->entrySignal = -1;
	}
	pIndicators->exitSignal = EXIT_SELL;
}

	return SUCCESS;
}

// XAUUSD_DayTrading_Allow_Trade and XAUUSD_DayTrading_Entry are implemented in DayTradingHelpers.c
// Removed duplicate implementations - using the ones from DayTradingHelpers.c

// workoutExecutionTrend_XAUUSD_DayTrading is implemented in DayTradingStrategy.c
// Removed duplicate/broken implementation - using the one from DayTradingStrategy.c
