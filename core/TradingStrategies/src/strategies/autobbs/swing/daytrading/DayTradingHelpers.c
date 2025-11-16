/*
 * Day Trading Helpers Module
 * 
 * Provides helper functions for day trading strategies.
 * Includes trade filtering and entry logic.
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
#include "StrategyUserInterface.h"
#include "strategies/autobbs/swing/daytrading/DayTradingHelpers.h"
BOOL XAUUSD_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate,euro_index_rate,execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);	
	double MATrend_1H, MATrend_15M;
	int startTradingTime = pIndicators->startHour;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	//execution_tf = pIndicators->executionRateTF;
	execution_tf = (int)pParams->settings[TIMEFRAME];

	//if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	//	startTradingTime = 15;

	// filter US rate day
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators))
		return FALSE;

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = (startTradingTime - 1) * (60 / execution_tf) - 1;
	if (count > 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	if (timeInfo1.tm_hour >= 17) //17:00 �������޽���ʱ��
	{
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		//count must > 1, otherwise, cause TA_MIN error 
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);
	}
	//// Workout Euro time bar( 10 - 15)
	//if (timeInfo1.tm_hour >= 10 && timeInfo1.tm_hour < 17)
	//{
	//	count = (timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
	//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
	//	pIndicators->euro_open = pIndicators->asia_close;
	//	pIndicators->euro_close = iClose(B_PRIMARY_RATES, 1);
	//}

	//if (timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour < 24)
	//{
	//	count = (timeInfo1.tm_hour - 15) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
	//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->us_high), &(pIndicators->us_low));
	//	pIndicators->us_open = pIndicators->euro_close;
	//	pIndicators->us_close = iClose(B_PRIMARY_RATES, 1);
	//}

	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

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

	
	//MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);
	//MATrend_15M = getMATrend(2, B_SECONDARY_RATES, 1);

	//if (MATrend_1H * MATrend_15M <0) //ͬ����
	//{

	//	logWarning("System InstanceID = %d, BarTime = %s filter out small range: MATrend_1H=%lf,MATrend_15M=%lf",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, MATrend_1H, MATrend_15M);
	//	return FALSE;		
	//}

	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators))
		return FALSE;
	

	return TRUE;
}

void XAUUSD_DayTrading_Entry(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double ATR0_EURO, double stopLoss, double Range)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
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
		if (ATR0_EURO > Range && pIndicators->lossTimes < 2 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
		{
			//pIndicators->risk = pow(2, pIndicators->lossTimes);
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

		if (ATR0_EURO > Range && pIndicators->lossTimes < 2 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
		{
			//pIndicators->risk = pow(2, pIndicators->lossTimes);
			pIndicators->entrySignal = -1;
		}

		pIndicators->exitSignal = EXIT_BUY;
	}
}
BOOL XAUUSD_DayTrading_Allow_Trade_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL shouldFilter)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int startTradingTime = pIndicators->startHour;
	//int startTradingTime = 2;
	double ATRWeekly0,ATRDaily0;
	double weeklyATR = (pBase_Indicators->pWeeklyPredictMaxATR + pBase_Indicators->pWeeklyPredictATR) / 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	
	execution_tf = (int)pParams->settings[TIMEFRAME];

	ATRDaily0 = iAtr(B_DAILY_RATES, 1, 0);
	if (timeInfo1.tm_hour < startTradingTime || 
		(timeInfo1.tm_hour >= pIndicators->endHour 
		//&& ATRDaily0 > pBase_Indicators->pDailyMaxATR
		)
	  )
	{
		
		return FALSE;
	}

	// filter ��ũ
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	{
	
		strcpy(pIndicators->status, "Filter Non-farm day\n");

		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;	
	}

	
	if (XAUUSD_not_full_trading_day(pParams, pIndicators, pBase_Indicators) == TRUE)
	{		
		logWarning("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
		return FALSE;
	}


	//asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	//count = (startTradingTime-1) * (60 / execution_tf) - 1;
	//if (count >= 1)
	//	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	//else
	//	return FALSE;

	//pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	//pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	//pIndicators->asia_open = close_prev1;
	//pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

	//
	//logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
	//	(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	//if (fabs(pIndicators->asia_high - pIndicators->asia_low) > pIndicators->atr_euro_range*1.2)
	//	return FALSE;
	if (shouldFilter == FALSE)
		return TRUE;

	//if ((pBase_Indicators->dailyTrend == 6 || pBase_Indicators->dailyTrend == -6))
	//	return TRUE;

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf,pWeeklyPredictATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR, pBase_Indicators->pWeeklyPredictATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
	{
		sprintf(pIndicators->status, "pDailyPredictATR %lf is less than atr_euro_range %lf",
			pBase_Indicators->pDailyPredictATR, pIndicators->atr_euro_range);

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 10)	
	{
		sprintf(pIndicators->status, "ATRWeekly0 %lf is greater than pWeeklyPredictMaxATR %lf and pDailyPredictATR > 10",
			ATRWeekly0, pBase_Indicators->pWeeklyPredictMaxATR, pBase_Indicators->pDailyPredictATR);

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	if (iAtr(B_DAILY_RATES, 1, 1) - max(20, pBase_Indicators->pWeeklyPredictATR / 2) >= 0) //�ղ���������С���ܲ���
	{
		sprintf(pIndicators->status, "ATR1 %lf is greater than half of pWeeklyPredictATR %lf",
			iAtr(B_DAILY_RATES, 1, 1), max(20, pBase_Indicators->pWeeklyPredictATR / 2));

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	if (fabs(close_prev1 - close_prev2) >= max(10, pBase_Indicators->pWeeklyPredictATR / 3)) //������������һ�����С�ܲ���
	{
		sprintf(pIndicators->status, "Previous close gap %lf is greater than third of pWeeklyPredictATR %lf",
			fabs(close_prev1 - close_prev2), max(10, pBase_Indicators->pWeeklyPredictATR / 3));

		logInfo("System InstanceID = %d, BarTime = %s, %s",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		return FALSE;
	}

	return TRUE;
}
BOOL GBPJPY_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1, timeInfoClose;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);	
	//int startTradingTime = 8;
	int startTradingTime = pIndicators->startHour;
	double ATRWeekly0;
	double targetProfit = 0,weeklyPNL = 0;
	int orderIndex = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	//if (pBase_Indicators->pDailyPredictATR >= 1)
	//	pIndicators->maxTradeTime = 2;
	//else
	//	pIndicators->maxTradeTime = 1;

	pIndicators->maxTradeTime = 1;
	

	execution_tf = (int)pParams->settings[TIMEFRAME];
	
	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);


	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
	//if (ATRWeekly0 < pBase_Indicators->pWeeklyPredictATR)
	//{
	//	if (pBase_Indicators->pDailyPredictATR >= 1.2)
	//	{
	//		startTradingTime = 8;
	//		//pIndicators->risk = 2;
	//	}

	//}

	if (timeInfo1.tm_hour < startTradingTime)
		return FALSE;

	
	asia_index_rate = shift1Index - ((timeInfo1.tm_hour - startTradingTime) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = startTradingTime * (60 / execution_tf) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	//if (fabs(pIndicators->asia_high - pIndicators->asia_low) > 0.8)
	//	return FALSE;

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
		return FALSE;
	
	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 1)
	{
		logWarning("System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
		return FALSE;
	}
	

	if (iAtr(B_DAILY_RATES, 1, 1) >= max(2, pBase_Indicators->pWeeklyPredictATR)) //�ղ���������С���ܲ���
		return FALSE;
	if (fabs(close_prev1 - close_prev2) >= max(1, pBase_Indicators->pWeeklyPredictATR / 2)) //������������һ�����С�ܲ���
		return FALSE;

	//Filter out wenesday ??? Need to check out more data....
	//if (timeInfo1.tm_wday == 3 && pIndicators->risk == 1 )
	//	return FALSE;

	////If weekly target meet, stop trading this week. target is 1% for now
	//// target profit / 3 * 10
	//targetProfit = pParams->settings[ACCOUNT_RISK_PERCENT] * 10 / 3;
	//weeklyPNL = caculateStrategyWeeklyPNLEasy(currentTime);

	//if (weeklyPNL >= targetProfit)
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s, weeklyPNL = %lf,targetProfit = %lf",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, weeklyPNL, targetProfit);
	//	return FALSE;
	//}


	return TRUE;
}
BOOL XAUUSD_DayTrading_Allow_Trade_Ver4(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, asia_index_rate, euro_index_rate, execution_tf;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int startTradingTime = pIndicators->startHour;
	//int startTradingTime = 2;
	double ATRWeekly0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	execution_tf = (int)pParams->settings[TIMEFRAME];


	// filter ��ũ
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
	{

		logWarning("System InstanceID = %d, BarTime = %s, Filter Non-farm day",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

		return FALSE;
	}

	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);

	logInfo("System InstanceID = %d, BarTime = %s, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);

	if (pBase_Indicators->pDailyPredictATR < pIndicators->atr_euro_range)
		return FALSE;

	if (ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR && pBase_Indicators->pDailyPredictATR < 10)
	{
		logWarning("System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictATR = %lf, pWeeklyPredictMaxATR=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, ATRWeekly0, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR);
		return FALSE;
	}

	return TRUE;
}

