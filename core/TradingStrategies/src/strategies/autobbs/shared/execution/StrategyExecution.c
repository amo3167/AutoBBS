/*
 * Strategy Execution Module
 * 
 * Provides strategy execution functions for workoutExecutionTrend.
 * Contains all workoutExecutionTrend_* functions that determine entry/exit signals
 * based on various trading strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
// Include strategy-specific headers EARLY to ensure declarations are available before use
#include "strategies/autobbs/trend/bbs/BBSSwingStrategy.h"  // For workoutExecutionTrend_4HBBS_Swing, workoutExecutionTrend_4HBBS_Swing_BoDuan, workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan
#include "strategies/autobbs/trend/limit/LimitStrategy.h"  // For workoutExecutionTrend_Limit
#include "strategies/autobbs/trend/limit/LimitBBSStrategy.h"  // For workoutExecutionTrend_Limit_BBS, workoutExecutionTrend_Limit_BBS_LongTerm, workoutExecutionTrend_Limit_BreakOutOnPivot
#include "strategies/autobbs/trend/weekly/WeeklyAutoStrategy.h"  // For workoutExecutionTrend_WeeklyAuto
#include "strategies/autobbs/swing/weekly/WeeklyStrategy.h"  // For workoutExecutionTrend_Weekly_Swing_New, workoutExecutionTrend_WeeklyATR_Prediction
#include "strategies/autobbs/swing/daytrading/DayTradingStrategy.h"  // For workoutExecutionTrend_GBPJPY_DayTrading_Ver2, workoutExecutionTrend_DayTrading_ExecutionOnly
#include "strategies/autobbs/swing/multipleday/MultipleDayStrategy.h"  // For workoutExecutionTrend_MultipleDay, workoutExecutionTrend_MultipleDay_V2
#include "strategies/autobbs/trend/macd/MACDDailyStrategy.h"  // For workoutExecutionTrend_MACD_Daily_New, workoutExecutionTrend_MACD_Daily
#include "strategies/autobbs/trend/macd/MACDWeeklyStrategy.h"  // For workoutExecutionTrend_MACD_Weekly
#include "strategies/autobbs/trend/ichimoko/IchimokoDailyStrategy.h"  // For workoutExecutionTrend_Ichimoko_Daily_New, workoutExecutionTrend_Ichimoko_Daily_Index
#include "strategies/autobbs/trend/ichimoko/IchimokoWeeklyStrategy.h"  // For workoutExecutionTrend_Ichimoko_Weekly_Index
#include "strategies/autobbs/trend/misc/MiscStrategies.h"  // For workoutExecutionTrend_4H_Shellington
#include "strategies/autobbs/trend/shortterm/ShortTermStrategy.h"  // For workoutExecutionTrend_ShortTerm
#include "strategies/autobbs/swing/macd_beili/MACDBEILIStrategy.h"  // For workoutExecutionTrend_MACD_BEILI
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"
#include "strategies/autobbs/shared/common/ProfitManagement.h"
#include "strategies/autobbs/shared/ordermanagement/OrderManagement.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"

AsirikuyReturnCode workoutExecutionTrend(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	struct tm timeInfo;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int startHour = 0;

	if (strstr(pParams->tradeSymbol, "XAU") != NULL || strstr(pParams->tradeSymbol, "XTI") != NULL || strstr(pParams->tradeSymbol, "SpotCrude") != NULL || strstr(pParams->tradeSymbol, "XAG") != NULL || strstr(pParams->tradeSymbol, "XPD") != NULL)
		startHour = 1;

	safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	// Strategy risk
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
	case 25: // 
		pIndicators->tradeMode = 1;
		workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);
		break;
	case 26: 
		//pIndicators->tradeMode = 0;
		//workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_Limit_BBS_LongTerm(pParams, pIndicators, pBase_Indicators);
		break;
	case 27:
		workoutExecutionTrend_Ichimoko_Daily_Index(pParams, pIndicators, pBase_Indicators);
		break;	
	case 28:
		workoutExecutionTrend_Ichimoko_Daily_Index(pParams, pIndicators, pBase_Indicators);
		break;
	case 30:		
		//workoutExecutionTrend_4H_ShellingtonVer1(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_4H_Shellington(pParams, pIndicators, pBase_Indicators);
		break;
	case 31:
		workoutExecutionTrend_Ichimoko_Daily_New(pParams, pIndicators, pBase_Indicators);
		break;
	case 32:
		workoutExecutionTrend_MACD_Daily_New(pParams, pIndicators, pBase_Indicators);
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


	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && pParams->accountInfo.totalOpenTradeRiskPercent < parameter(AUTOBBS_MAX_ACCOUNT_RISK) * -1) //if account risk is more than 3%, stop entring trades.
	{
		logWarning("System InstanceID = %d, BarTime = %s, Over max riks %lf, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, parameter(AUTOBBS_MAX_ACCOUNT_RISK), pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	//Filter out macro trend
	if (pIndicators->side != 0 && pIndicators->entrySignal != 0 && pIndicators->side != pIndicators->entrySignal)
	{
		logWarning("System InstanceID = %d, BarTime = %s,Againt Side =%ld, skip this entry signal=%d",
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

// workoutExecutionTrend_KeyK is implemented in MiscStrategies.c - removed duplicate
#if 0
// 1% risk
AsirikuyReturnCode workoutExecutionTrend_KeyK(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	double keyHigh, keyLow;

	pIndicators->risk = 2;
	pIndicators->splitTradeMode = 9;
	pIndicators->tpMode = 0;

	keyHigh = iHigh(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);
	keyLow = iLow(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);

	if (pBase_Indicators->intradayTrend == 1)
	{
		if (pIndicators->side >= 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->exitSignal = EXIT_SELL;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = keyLow;
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyLow) >= 0.5 * pBase_Indicators->dailyATR)
				pIndicators->risk = 1;

			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = 1;
			}
		}
		else
			pBase_Indicators->intradayTrend = 0;
	}

	if (pBase_Indicators->intradayTrend == -1)
	{
		if (pIndicators->side <= 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->exitSignal = EXIT_BUY;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = keyHigh;
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyHigh) >= 0.5 * pBase_Indicators->dailyATR)
				pIndicators->risk = 1;

			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = -1;
			}
		}
		else
			pBase_Indicators->intradayTrend = 0;
	}

	return returnCode;
}
#endif

// workoutExecutionTrend_Auto_Hedge is implemented in HedgeStrategy.c - removed duplicate
#if 0
AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double stopLoss;
	double riskPNL;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
	case RANGE_PHASE:
		// Execute hedge strategy for range-bound trading
		workoutExecutionTrend_Hedge(pParams, pIndicators, pBase_Indicators);
		break;
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		break;
	}

	profitManagement(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}
#endif

// workoutExecutionTrend_Auto is implemented in MiscStrategies.c - removed duplicate
#if 0
AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double stopLoss;
	double riskPNL;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	struct tm timeInfo1;

	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int execution_tf, euro_index_rate, count;
	double ATR0_EURO = 10;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;

	// Check out if Key K is triggered. If yes, take piority with other strategies.,
	// TODO Need to test riskPNL parameter, disabled for now
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
		// workoutExecutionTrend_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal
			workoutExecutionTrend_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, FALSE);
		pIndicators->splitTradeMode = 0;

		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		workoutExecutionTrend_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, FALSE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal
			workoutExecutionTrend_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal
			workoutExecutionTrend_KongJian(pParams, pIndicators, pBase_Indicators);
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
				// closeAllWithNegativeEasy(1, currentTime, 3);

				if (totalOpenOrders(pParams, BUY) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->dailyS) + pIndicators->adjust;
					modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, FALSE);
				}
				if (totalOpenOrders(pParams, SELL) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->dailyR) + pIndicators->adjust;
					modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, FALSE);
				}
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	if (timeInfo1.tm_hour >= 17)
	{
		execution_tf = (int)pParams->settings[TIMEFRAME];
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);

		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);
	}

	return SUCCESS;
}

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
#endif

// workoutExecutionTrend_Hedge is implemented in HedgeStrategy.c - removed duplicate
#if 0
AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);

double down_gap = pIndicators->entryPrice - pBase_Indicators->pDailyLow;
double up_gap = pBase_Indicators->pDailyHigh - pIndicators->entryPrice;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

// closeAllWithNegativeEasy(1, currentTime, 3);
if (timeInfo1.tm_hour == 23 && timeInfo1.tm_min > 25)
{
	pIndicators->exitSignal = EXIT_ALL;
	return SUCCESS;
}

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 14;

// Asia hour, risk 50%
if (timeInfo1.tm_hour <= 8)
	pIndicators->risk = 0.5;

if (pBase_Indicators->dailyTrend_Phase > 0)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}
else if (pBase_Indicators->dailyTrend_Phase < 0)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	// Hedge strategy execution
	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}
else
{
	// [Comment removed - encoding corrupted]
	if (up_gap <= pBase_Indicators->pDailyATR / 3)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}
	else if (down_gap <= pBase_Indicators->pDailyATR / 3)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		// [Comment removed - encoding corrupted]
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;
	}
}
return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);
double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 0;

pIndicators->tradeMode = 1;

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pParams->bidAsk.ask[0] - intradayLow > pBase_Indicators->dailyATR / 3 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
	{
		pIndicators->entrySignal = 1;
	}
	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
{

	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);
	if (intradayHigh - pParams->bidAsk.bid[0] > pBase_Indicators->dailyATR / 3 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
	{
		pIndicators->entrySignal = -1;
	}
	pIndicators->exitSignal = EXIT_BUY;
}

	return SUCCESS;
}
#endif

// workoutExecutionTrend_KongJian is implemented in MiscStrategies.c - removed duplicate
#if 0
// It should always use 15M
AsirikuyReturnCode workoutExecutionTrend_BEGINNING_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
// shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 3;

pIndicators->tradeMode = 1;

// if (timeInfo1.tm_hour < 5)
//	pIndicators->risk = 0.5;

if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust && ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = 1;
	// Override the entry signal if the current daily low has been lower than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust && ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = -1;

	// Override the entry signal if the current daily high has been higher than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_BUY;
}
return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_MIDDLE_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);

double stopLoss;
double preHigh = iHigh(B_SECONDARY_RATES, 1);
double preLow = iLow(B_SECONDARY_RATES, 1);
double preClose = iClose(B_SECONDARY_RATES, 1);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
// shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 6;

pIndicators->tradeMode = 1;

// if (timeInfo1.tm_hour < 5)
//	pIndicators->risk = 0.5;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust && preLow < pBase_Indicators->dailyPivot && preClose > pBase_Indicators->dailyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
		//&& bbsTrend == 1
		&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = 1;

	// Override the entry signal if the current daily low has been lower than stop loss price.
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

	if (pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust && preHigh > pBase_Indicators->dailyPivot && preClose < pBase_Indicators->dailyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
		//&& bbsTrend == -1
		&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = -1;

	// Override the entry signal if the current daily high has been higher than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
// shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 0;

pIndicators->tradeMode = 1;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_RETREAT_PHASE)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666
		//&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
	)
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}
#endif

// workoutExecutionTrend_WeeklyAuto is implemented in WeeklyAutoStrategy.c - removed duplicate
#if 0
AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double stopLoss;

// Check out if Key K is triggered. If yes, take piority with other strategies.
if ((int)parameter(AUTOBBS_KEYK) == 1)
{
	workoutExecutionTrend_KeyK(pParams, pIndicators, pBase_Indicators);
	if (pIndicators->entrySignal != 0)
		return SUCCESS;
	if (pBase_Indicators->intradayTrend == 1 && pBase_Indicators->weeklyTrend_Phase < 0)
		return SUCCESS;
	if (pBase_Indicators->intradayTrend == -1 && pBase_Indicators->weeklyTrend_Phase > 0)
		return SUCCESS;
}

switch (pBase_Indicators->weeklyTrend_Phase)
{
case BEGINNING_UP_PHASE:
case BEGINNING_DOWN_PHASE:
	workoutExecutionTrend_Weekly_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal
		workoutExecutionTrend_Weekly_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	pIndicators->splitTradeMode = 11;
	break;
case MIDDLE_UP_PHASE:
case MIDDLE_DOWN_PHASE:
	workoutExecutionTrend_Weekly_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal
		workoutExecutionTrend_Weekly_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	pIndicators->splitTradeMode = 7;
	break;
case MIDDLE_UP_RETREAT_PHASE:
case MIDDLE_DOWN_RETREAT_PHASE:
	workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	break;
case RANGE_PHASE:
	if ((int)parameter(AUTOBBS_RANGE) == 1)
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
	else
		pIndicators->exitSignal = EXIT_ALL;
	break;
}

profitManagementWeekly(pParams, pIndicators, pBase_Indicators);

return SUCCESS;
}
#endif

// Weekly retreat strategy, only for EUR, AUD, crossing currencies
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
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

/*

1.ATR > 20
2.Close -  Close > 10
3.> 7 or 8?
4.3
5.10
6.15MA too close,  21H MA 
*/
static BOOL XAUUSD_DayTrading_Allow_Trade(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int count, asia_index_rate, euro_index_rate, execution_tf;
time_t currentTime;
struct tm timeInfo1;
double preHigh, preLow, preClose;
double pivot, S3, R3;
char timeString[MAX_TIME_STRING_SIZE] = "";
double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);
safe_timeString(timeString, currentTime);

// execution_tf = pIndicators->executionRateTF;
execution_tf = (int)pParams->settings[TIMEFRAME];

if (timeInfo1.tm_hour < 10)
	return FALSE;

asia_index_rate = shift1Index - ((timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

count = (10 - 1) * (60 / execution_tf) - 1;
iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
pIndicators->asia_open = close_prev1;
pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

if (timeInfo1.tm_hour >= 17) // [Comment removed - encoding corrupted]
{
	euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = (17 - 1) * (60 / execution_tf) - 1;
	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
	pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
	pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
	pIndicators->euro_open = close_prev1;
	pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);
}
//// Workout Euro time bar( 10 - 15)
// if (timeInfo1.tm_hour >= 10 && timeInfo1.tm_hour < 17)
//{
//	count = (timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
//	pIndicators->euro_open = pIndicators->asia_close;
//	pIndicators->euro_close = iClose(B_PRIMARY_RATES, 1);
// }

// if (timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour < 24)
//{
//	count = (timeInfo1.tm_hour - 15) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->us_high), &(pIndicators->us_low));
//	pIndicators->us_open = pIndicators->euro_close;
//	pIndicators->us_close = iClose(B_PRIMARY_RATES, 1);
// }

if (fabs(pIndicators->asia_high - pIndicators->asia_low) >= 7.5)
	return FALSE;

if (iAtr(B_DAILY_RATES, 1, 1) >= 20)
	return FALSE;
if (fabs(close_prev1 - close_prev2) >= 10)
	return FALSE;

preHigh = iHigh(B_DAILY_RATES, 2);
preLow = iLow(B_DAILY_RATES, 2);
preClose = iClose(B_DAILY_RATES, 2);

pivot = (preHigh + preLow + preClose) / 3;
S3 = preLow - 2 * (preHigh - pivot);
R3 = preHigh + 2 * (pivot - preLow);

if (close_prev1 > R3 || close_prev1 < S3)
	return FALSE;

// [Comment removed - encoding corrupted]
// if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
//{
//	fprintf(stderr, ("[INFO] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s No Trade on  Non Farm PayRoll day\n\n",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
//	return FALSE;
// }

return TRUE;
}

static void XAUUSD_DayTrading_Entry(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double ATR0_EURO, double stopLoss, double Range)
{
int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
safe_gmtime(&timeInfo1, currentTime);

if (orderType == SELL)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
	if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
	{
		pIndicators->risk = pow(2, pIndicators->lossTimes);
		pIndicators->entrySignal = 1;
	}
	pIndicators->exitSignal = EXIT_SELL;
}

if (orderType == BUY)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
	if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
	{
		pIndicators->risk = pow(2, pIndicators->lossTimes);
		pIndicators->entrySignal = -1;
	}

	pIndicators->exitSignal = EXIT_BUY;
}
}

// workoutExecutionTrend_XAUUSD_DayTrading is implemented in DayTradingStrategy.c
// Removed duplicate/broken implementation - using the one from DayTradingStrategy.c
