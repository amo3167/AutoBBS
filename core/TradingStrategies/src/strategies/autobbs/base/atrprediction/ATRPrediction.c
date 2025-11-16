/*
 * ATR Prediction Module
 * 
 * Provides functions for predicting Average True Range (ATR)
 * for daily and weekly timeframes, including longer-term predictions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/atrprediction/ATRPrediction.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE
void predictDailyATR(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double shortDailyATR, mediumDailyATR, longDailyATR;
	double ATR0, ATR1, ATR2, ATR3, ATR4;	
	double minATR = 10000, maxATR = -10000, pMinATR, pMaxATR, pATRSameWeekDay = 10000;
	double pATR;
	double pLow = -10000, pHigh = 10000, pSODLow = -10000, pSODHigh = 10000;
	double pMaxLow = -10000, pMaxHigh = 10000;
	double lastDailyClose = iClose(B_DAILY_RATES, 1);
	double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);

	int shift0Index = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_DAILY_RATES].time[shift0Index]);

	ATR0 = iAtr(B_DAILY_RATES, 1, 0);
	ATR1 = iAtr(B_DAILY_RATES, 1, 1);
	ATR2 = iAtr(B_DAILY_RATES, 1, 2);
	ATR3 = iAtr(B_DAILY_RATES, 1, 3);
	ATR4 = iAtr(B_DAILY_RATES, 1, 4);

	shortDailyATR = iAtr(B_DAILY_RATES, 2, 1);
	mediumDailyATR = iAtr(B_DAILY_RATES, 5, 1);
	longDailyATR = iAtr(B_DAILY_RATES, 20, 1);

	minATR = min(minATR, shortDailyATR);
	minATR = min(minATR, mediumDailyATR);
	minATR = min(minATR, longDailyATR);
	pIndicators->pDailyMinATR = minATR;

	maxATR = max(maxATR, shortDailyATR);
	maxATR = max(maxATR, mediumDailyATR);
	maxATR = max(maxATR, longDailyATR);
	pIndicators->pDailyMaxATR = maxATR;
	
	logDebug("System InstanceID = %d, BarTime = %s, shortDailyATR = %f,mediumDailyATR=%f,longDailyATR=%f,minATR=%f,maxATR=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyATR, mediumDailyATR, longDailyATR, minATR, maxATR);

	logDebug("System InstanceID = %d, BarTime = %s, ATR0 = %f,ATR1=%f,ATR2=%f,ATR3=%f,ATR4=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, ATR1, ATR2, ATR3, ATR4);

	// 1. ʹ��ƽ��5�첨��
	pMinATR = minATR * 5 - ATR1 - ATR2 - ATR3 - ATR4;
	pMaxATR = maxATR * 5 - ATR1 - ATR2 - ATR3 - ATR4;

	if (pMinATR < minATR*0.5)
		pMinATR = minATR*0.5;
	else if (pMinATR > minATR*1.3)
		pMinATR = minATR*1.3;

	if (pMaxATR < minATR*0.5)
		pMaxATR = minATR*0.5;
	else if (pMaxATR > minATR*1.3)
		pMaxATR = minATR*1.3;

	if (pMinATR> minATR*0.7)
		pATR = pMinATR;
	else
		pATR = (pMinATR + pMaxATR) / 2;

	pATR = min(pATR, minATR);
	//Get average atr on the same week day.
	pATRSameWeekDay = (iAtr(B_DAILY_RATES, 1, 5) + iAtr(B_DAILY_RATES, 1, 10) + iAtr(B_DAILY_RATES, 1, 15) + iAtr(B_DAILY_RATES, 1, 20)) / 4;

	logDebug("System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameWeekDay=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameWeekDay);

	pATR = min(pATR, pATRSameWeekDay);
	//pATR = min(pATR, minATR);
	logDebug("System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameWeekDay=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameWeekDay, ATR0);

	
	pLow = max(lastDailyClose, intradayHigh) - pMaxATR;
	pHigh = min(lastDailyClose, intradayLow) + pMaxATR;


	if (pLow < pIndicators->dailyS2)
		pLow = pIndicators->dailyS2;


	if (pHigh > pIndicators->dailyR2)
		pHigh = pIndicators->dailyR2;

		
	pIndicators->pDailyATR = pATR;
	pIndicators->pDailyPredictATR = pATR;
	pIndicators->pDailyHigh = pHigh;
	pIndicators->pDailyLow = pLow;

	if (ATR0 <= pATR)
		pIndicators->pDailyTrend = VOL_IN_RANGE;
	else
	{
		pIndicators->pDailyATR = ATR0;
		pIndicators->pDailyHigh = intradayHigh;
		pIndicators->pDailyLow = intradayLow;
		if (intradayClose > lastDailyClose)
			pIndicators->pDailyTrend = VOL_BREAK_UP;
		else
			pIndicators->pDailyTrend = VOL_BREAK_DOWN;
	}

	if (pIndicators->pDailyLow > pIndicators->dailyS2)
		pMaxLow = pIndicators->dailyS2;
	else
		pMaxLow = max(lastDailyClose, intradayHigh) - pATR;

	if (pIndicators->pDailyHigh < pIndicators->dailyR2)
		pMaxHigh = pIndicators->dailyR2;
	else
		pMaxHigh = min(lastDailyClose, intradayLow) + pATR;

	pIndicators->pMaxDailyHigh = pMaxHigh;
	pIndicators->pMaxDailyLow = pMaxLow;

	logDebug("System InstanceID = %d, BarTime = %s,pDailyPredictATR=%f,pDailyATR=%f,pDailyHigh=%f,pDailyLow=%f, pDailyTrend = %ld,pMaxDailyHigh=%lf,pMaxDailyLow=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pDailyPredictATR, pIndicators->pDailyATR, pIndicators->pDailyHigh, pIndicators->pDailyLow, pIndicators->pDailyTrend, pIndicators->pMaxDailyHigh, pIndicators->pMaxDailyLow);

	//setPredictDailyATR((int)pParams->settings[STRATEGY_INSTANCE_ID], pIndicators->pDailyATR, (BOOL)pParams->settings[IS_BACKTESTING]);
}



void predictWeeklyATR(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double shortWeeklyATR, mediumWeeklyATR, longWeeklyATR;
	double ATR0, ATR1, ATR2, ATR3;
	double minATR = 10000, maxATR = -10000, pMinATR, pMaxATR, pATRSameMonthWeek;
	double pATR;
	double pMaxLow = -10000, pMaxHigh = 10000;
	double pLow = -10000, pHigh = 10000, pSODLow = -10000, pSODHigh = 10000;
	double lastWeekClose = iClose(B_WEEKLY_RATES, 1);
	double intraWeekClose = iClose(B_WEEKLY_RATES, 0), intraWeekHigh = iHigh(B_WEEKLY_RATES, 0), intraWeekLow = iLow(B_WEEKLY_RATES, 0);

	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);

	ATR0 = iAtr(B_WEEKLY_RATES, 1, 0);
	ATR1 = iAtr(B_WEEKLY_RATES, 1, 1);
	ATR2 = iAtr(B_WEEKLY_RATES, 1, 2);
	ATR3 = iAtr(B_WEEKLY_RATES, 1, 3);	

	shortWeeklyATR = iAtr(B_WEEKLY_RATES, 2, 1);
	mediumWeeklyATR = iAtr(B_WEEKLY_RATES, 4, 1);
	longWeeklyATR = iAtr(B_WEEKLY_RATES, 8, 1);

	minATR = min(minATR, shortWeeklyATR);
	minATR = min(minATR, mediumWeeklyATR);
	minATR = min(minATR, longWeeklyATR);

	maxATR = max(maxATR, shortWeeklyATR);
	maxATR = max(maxATR, mediumWeeklyATR);
	maxATR = max(maxATR, longWeeklyATR);
	
	pIndicators->pWeeklyPredictMaxATR = maxATR;

	logDebug("System InstanceID = %d, BarTime = %s, shortWeeklyATR = %f,mediumWeeklyATR=%f,longWeeklyATR=%f,minATR=%f,maxATR=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyATR, mediumWeeklyATR, longWeeklyATR, minATR, maxATR);

	logDebug("System InstanceID = %d, BarTime = %s, ATR0 = %f,ATR1=%f,ATR2=%f,ATR3=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, ATR1, ATR2, ATR3);

	// 1. ʹ��ƽ��4�ܲ���
	pMinATR = minATR * 4 - ATR1 - ATR2 - ATR3;
	pMaxATR = maxATR * 4 - ATR1 - ATR2 - ATR3;

	if (pMinATR < minATR*0.6)
		pMinATR = minATR*0.6;
	else if (pMinATR > minATR*1.3)
		pMinATR = minATR*1.3;

	if (pMaxATR < minATR*0.6)
		pMaxATR = minATR*0.6;
	else if (pMaxATR > minATR*1.3)
		pMaxATR = minATR*1.3;

	if (pMinATR> minATR*0.7)
		pATR = pMinATR;
	else
		pATR = (pMinATR + pMaxATR) / 2;
		
	logDebug("System InstanceID = %d, BarTime = %s,pATR=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, ATR0);

	//Get average atr on the same week day.
	//pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) + iAtr(B_WEEKLY_RATES, 1, 12) + iAtr(B_WEEKLY_RATES, 1, 16)) / 4;
	pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) ) / 2;

	pATR = min(pATR, pATRSameMonthWeek);
	pATR = min(pATR, minATR);

	logDebug("System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameMonthWeek=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameMonthWeek, ATR0);
	
	//3. ʹ��pivot�����?���������£����з��ദ��

	//4. Cap weekly ATR within 300

		
	pLow = max(lastWeekClose, intraWeekHigh) - pATR;
	pHigh = min(lastWeekClose, intraWeekLow) + pATR;


	if (pLow < pIndicators->weeklyS2)
		pLow = pIndicators->weeklyS2;

	if (pHigh > pIndicators->weeklyR2)
		pHigh = pIndicators->weeklyR2;

	pIndicators->pWeeklyPredictATR = pATR;
	pIndicators->pWeeklyATR = pATR;
	pIndicators->pWeeklyHigh = pHigh;
	pIndicators->pWeeklyLow = pLow;

	if (ATR0 <= pATR)
		pIndicators->pWeeklyTrend = VOL_IN_RANGE;
	else
	{
		pIndicators->pWeeklyATR = ATR0;
		pIndicators->pWeeklyHigh = intraWeekHigh;
		pIndicators->pWeeklyLow = intraWeekLow;
		if (intraWeekClose > lastWeekClose)
			pIndicators->pWeeklyTrend = VOL_BREAK_UP;
		else
			pIndicators->pWeeklyTrend = VOL_BREAK_DOWN;
	}
	
	pMaxLow = min(pIndicators->pWeeklyLow, pIndicators->weeklyS2);

	pMaxHigh = max(pIndicators->pWeeklyHigh, pIndicators->weeklyR2);

	pIndicators->pMaxWeeklyHigh = pMaxHigh;
	pIndicators->pMaxWeeklyLow = pMaxLow;

	logDebug("System InstanceID = %d, BarTime = %s,pWeeklyPredictATR=%f,pWeeklyATR=%f,pWeeklyHigh=%f,pWeeklyLow=%f, pWeeklyTrend = %ld,pMaxWeeklyHigh=%f,pMaxWeeklyLow=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pWeeklyPredictATR, pIndicators->pWeeklyATR, pIndicators->pWeeklyHigh, pIndicators->pWeeklyLow, pIndicators->pWeeklyTrend, pIndicators->pMaxWeeklyHigh, pIndicators->pMaxWeeklyLow);

	//setPredictDailyATR((int)pParams->settings[STRATEGY_INSTANCE_ID], pIndicators->pDailyATR, (BOOL)pParams->settings[IS_BACKTESTING]);
}


void predictWeeklyATR_LongerTerm(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double shortWeeklyATR, mediumWeeklyATR, longWeeklyATR;
	double ATR0, ATR1, ATR2, ATR3;
	double minATR = 10000, maxATR = -10000, pMinATR, pMaxATR, pATRSameMonthWeek;
	double pATR, pUpATR, pDownATR;
	double pMaxLow = -10000, pMaxHigh = 10000;
	double pLow = -10000, pHigh = 10000, pSODLow = -10000, pSODHigh = 10000;
	double lastWeekClose = iClose(B_WEEKLY_RATES, 1);
	double intraWeekClose = iClose(B_WEEKLY_RATES, 0), intraWeekHigh = iHigh(B_WEEKLY_RATES, 0), intraWeekLow = iLow(B_WEEKLY_RATES, 0);

	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);

	ATR0 = iAtr(B_WEEKLY_RATES, 1, 0);
	ATR1 = iAtr(B_WEEKLY_RATES, 1, 1);
	ATR2 = iAtr(B_WEEKLY_RATES, 1, 2);
	ATR3 = iAtr(B_WEEKLY_RATES, 1, 3);

	shortWeeklyATR = iAtr(B_WEEKLY_RATES, 2, 1);
	mediumWeeklyATR = iAtr(B_WEEKLY_RATES, 4, 1);
	longWeeklyATR = iAtr(B_WEEKLY_RATES, 16, 1);

	minATR = min(minATR, shortWeeklyATR);
	minATR = min(minATR, mediumWeeklyATR);
	minATR = min(minATR, longWeeklyATR);

	maxATR = max(maxATR, shortWeeklyATR);
	maxATR = max(maxATR, mediumWeeklyATR);
	maxATR = max(maxATR, longWeeklyATR);

	pIndicators->pWeeklyPredictMaxATR = maxATR;

	logDebug("System InstanceID = %d, BarTime = %s, shortWeeklyATR = %f,mediumWeeklyATR=%f,longWeeklyATR=%f,minATR=%f,maxATR=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyATR, mediumWeeklyATR, longWeeklyATR, minATR, maxATR);

	logDebug("System InstanceID = %d, BarTime = %s, ATR0 = %f,ATR1=%f,ATR2=%f,ATR3=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, ATR1, ATR2, ATR3);

	// 1. ʹ��ƽ��4�ܲ���
	pMinATR = minATR * 4 - ATR1 - ATR2 - ATR3;
	pMaxATR = maxATR * 4 - ATR1 - ATR2 - ATR3;

	if (pMinATR < minATR*0.6)
		pMinATR = minATR*0.6;
	else if (pMinATR > minATR*1.3)
		pMinATR = minATR*1.3;

	if (pMaxATR < minATR*0.6)
		pMaxATR = minATR*0.6;
	else if (pMaxATR > minATR*1.3)
		pMaxATR = minATR*1.3;

	if (pMinATR> minATR*0.7)
		pATR = pMinATR;
	else
		pATR = (pMinATR + pMaxATR) / 2;

	logDebug("System InstanceID = %d, BarTime = %s,pATR=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, ATR0);

	//Get average atr on the same week day.
	pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) + iAtr(B_WEEKLY_RATES, 1, 12) + iAtr(B_WEEKLY_RATES, 1, 16)) / 4;	

	pATR = min(pATR, pATRSameMonthWeek);
	pATR = min(pATR, minATR);

	logDebug("System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameMonthWeek=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameMonthWeek, ATR0);

	//3. ʹ��pivot�����?���������£����з��ദ��

	//4. Cap weekly ATR within 300


	pLow = max(lastWeekClose, intraWeekHigh) - pATR;
	pHigh = min(lastWeekClose, intraWeekLow) + pATR;


	if (pLow < pIndicators->weeklyS2)
		pLow = pIndicators->weeklyS2;

	if (pHigh > pIndicators->weeklyR2)
		pHigh = pIndicators->weeklyR2;

	pIndicators->pWeeklyPredictATR = pATR;
	pIndicators->pWeeklyATR = pATR;
	pIndicators->pWeeklyHigh = pHigh;
	pIndicators->pWeeklyLow = pLow;

	if (ATR0 <= pATR)
		pIndicators->pWeeklyTrend = VOL_IN_RANGE;
	else
	{
		pIndicators->pWeeklyATR = ATR0;
		pIndicators->pWeeklyHigh = intraWeekHigh;
		pIndicators->pWeeklyLow = intraWeekLow;
		if (intraWeekClose > lastWeekClose)
			pIndicators->pWeeklyTrend = VOL_BREAK_UP;
		else
			pIndicators->pWeeklyTrend = VOL_BREAK_DOWN;
	}

	pMaxLow = min(pIndicators->pWeeklyLow, pIndicators->weeklyS2);

	pMaxHigh = max(pIndicators->pWeeklyHigh, pIndicators->weeklyR2);

	pIndicators->pMaxWeeklyHigh = pMaxHigh;
	pIndicators->pMaxWeeklyLow = pMaxLow;

	logDebug("System InstanceID = %d, BarTime = %s,pWeeklyPredictATR=%f,pWeeklyATR=%f,pWeeklyHigh=%f,pWeeklyLow=%f, pWeeklyTrend = %ld,pMaxWeeklyHigh=%f,pMaxWeeklyLow=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pWeeklyPredictATR, pIndicators->pWeeklyATR, pIndicators->pWeeklyHigh, pIndicators->pWeeklyLow, pIndicators->pWeeklyTrend, pIndicators->pMaxWeeklyHigh, pIndicators->pMaxWeeklyLow);

	//setPredictDailyATR((int)pParams->settings[STRATEGY_INSTANCE_ID], pIndicators->pDailyATR, (BOOL)pParams->settings[IS_BACKTESTING]);
}


// closeAllLimitPreviousDayOrders is implemented in BaseUtilities.c - removed duplicate

// isNewDay is implemented in BaseUtilities.c - removed duplicate declaration
