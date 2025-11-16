/*
 * Indicator Loaders Module
 * 
 * Provides functions for loading indicators from different timeframes.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"

/* Include dependent modules */
#include "strategies/autobbs/base/supportresistance/SupportResistance.h"
#include "strategies/autobbs/base/trendanalysis/TrendAnalysis.h"
#include "strategies/autobbs/base/trendcalculators/TrendCalculators.h"
#include "strategies/autobbs/base/atrprediction/ATRPrediction.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

//Get the previous 10 weeks high or low. 
static AsirikuyReturnCode loadMonthlyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);
	iSRLevels(pParams, pIndicators, B_WEEKLY_RATES, shift1Index, 8, &(pIndicators->monthlyHigh), &(pIndicators->monthlyLow));

	logDebug("System InstanceID = %d, BarTime = %s, 10weeksHigh=%lf, 10weeksLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->monthlyHigh, pIndicators->monthlyLow);
	return SUCCESS;
}

AsirikuyReturnCode loadWeeklyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{	
	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;		
	int shift1Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);
		
	iTrend3Rules(pParams, pIndicators, B_WEEKLY_RATES, 2, &(pIndicators->weekly3RulesTrend),0);
	iTrend_HL(B_WEEKLY_RATES, &(pIndicators->weeklyHLTrend),0);
	if (pIndicators->weeklyMAMode == 0)
		iTrend_MA(pIndicators->weeklyATR, B_FOURHOURLY_RATES, &(pIndicators->weeklyMATrend));
	else
		iTrend_MA_WeeklyBar_For4H(pIndicators->weeklyATR, &(pIndicators->weeklyMATrend));

	iSRLevels(pParams, pIndicators, B_WEEKLY_RATES, shift1Index,2, &(pIndicators->weeklyHigh), &(pIndicators->weeklyLow));

	logDebug("System InstanceID = %d, BarTime = %s, weeklyHLTrend = %ld,weeklyMATrend=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyHLTrend, pIndicators->weeklyMATrend);

	logDebug("System InstanceID = %d, BarTime = %s, weekly3RulesTrend = %ld,weeklyHigh=%lf, weeklyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weekly3RulesTrend, pIndicators->weeklyHigh, pIndicators->weeklyLow);

	workoutWeeklyTrend(pParams, pIndicators);
	logDebug("System InstanceID = %d, BarTime = %s, weeklyTrend=%ld, weeklySupport = %lf,weeklyResistance = %lf,weeklyTP=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyTrend, pIndicators->weeklyS, pIndicators->weeklyR, pIndicators->weeklyTP);

	predictWeeklyATR(pParams, pIndicators);

	return SUCCESS;

}
static AsirikuyReturnCode loadIntradayKeyKIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	double prePrimaryHigh, prePrimaryLow, prePrimaryClose, primaryMovement;
	int i;
	int  shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime,tepmTime;
	struct tm timeInfo1,timeInfo2;

	char timeString[MAX_TIME_STRING_SIZE] = "";
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->intradayTrend = 0;
	pIndicators->intradyIndex = 0;

	// Look back for today's primary bar and check out if there is Key K. 
	// If yes, it will change the intraday trend, also it should return the index. 
	for (i = 1; i < 290; i++)
	{
		tepmTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index - i];
		safe_gmtime(&timeInfo2, tepmTime);

		if (timeInfo1.tm_yday != timeInfo2.tm_yday)
			break;

		pIndicators->intradyIndex = shift0Index - i;

		prePrimaryHigh = iHigh(B_PRIMARY_RATES, i);
		prePrimaryLow = iLow(B_PRIMARY_RATES, i);
		prePrimaryClose = iClose(B_PRIMARY_RATES, i);		
			

		primaryMovement = fabs(prePrimaryHigh - prePrimaryLow);
		// ߵ͵ܽ?=0.5 daily ATRǺõ볡źţλ䡣
		// ĸߵ͵Զ> 0.5 daily ATR, ͲǺܺõ볡źţλ롣
		// No entry signals here
		if (primaryMovement >= 0.5 * pIndicators->dailyATR)
		{
			if (fabs(prePrimaryHigh - prePrimaryClose) < primaryMovement / 3)
			{
				pIndicators->intradayTrend = 1;
				break;
			}
			if (fabs(prePrimaryLow - prePrimaryClose) < primaryMovement / 3)
			{
				pIndicators->intradayTrend = -1;
				break;
			}
		}
	}

	logDebug("System InstanceID = %d, BarTime = %s, intradayTrend = %ld,intradyIndex=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->intradayTrend, pIndicators->intradyIndex);


	return returnCode;
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
static AsirikuyReturnCode loadDailyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators,int index)
{	
	int shift0Index = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;	
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int pre3KTrend, preHLTrend;
	
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_DAILY_RATES].time[shift0Index]);

	iTrend3Rules(pParams, pIndicators, B_DAILY_RATES, 2, &(pIndicators->daily3RulesTrend),index);
	iTrend_HL(B_DAILY_RATES, &(pIndicators->dailyHLTrend),index);
	//iTrend_MA(pIndicators->dailyATR,B_HOURLY_RATES, &(pIndicators->dailyMATrend));
	iTrend_MA_DailyBar_For1H(pIndicators->dailyATR, &(pIndicators->dailyMATrend),index);

	iSRLevels(pParams, pIndicators, B_DAILY_RATES, shift1Index-index,2, &(pIndicators->dailyHigh), &(pIndicators->dailyLow));

	logDebug("System InstanceID = %d, BarTime = %s, dailyHLTrend = %ld,dailyMATrend=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyHLTrend, pIndicators->dailyMATrend);

	logDebug("System InstanceID = %d, BarTime = %s, daily3RulesTrend = %ld,dailyHigh=%lf, dailyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->daily3RulesTrend, pIndicators->dailyHigh, pIndicators->dailyLow);

	
	if (pIndicators->dailyHLTrend == 1 && pIndicators->daily3RulesTrend == 0)
	{
		iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 1,index);
		if (pre3KTrend == UP)
			pIndicators->daily3RulesTrend = UP;
		else 
		{
			iTrend_HL_preDays(B_DAILY_RATES, &preHLTrend,1,index);
			if (preHLTrend == 1 && pre3KTrend == 0)
			{
				iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 2,index);
				if (pre3KTrend == UP)
					pIndicators->daily3RulesTrend = UP;
			}
		}
	}

	if (pIndicators->dailyHLTrend == -1 && pIndicators->daily3RulesTrend == 0)
	{
		iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 1,index);
		if (pre3KTrend == DOWN)
			pIndicators->daily3RulesTrend = DOWN;
		else
		{
			iTrend_HL_preDays(B_DAILY_RATES, &preHLTrend, 1,index);
			if (preHLTrend == -1 && pre3KTrend == 0)
			{
				iTrend3Rules_preDays(pParams, pIndicators, B_DAILY_RATES, 2, &pre3KTrend, 2,index);
				if (pre3KTrend == DOWN)
					pIndicators->daily3RulesTrend = DOWN;
			}
		}
	}

	workoutDailyTrend(pParams, pIndicators);
	logDebug("System InstanceID = %d, BarTime = %s, dailyTrend=%ld, dailySupport = %lf,dailyResistance = %lfdailyTP=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyTrend, pIndicators->dailyS, pIndicators->dailyR, pIndicators->dailyTP);

	predictDailyATR(pParams, pIndicators);

	return SUCCESS;

}



static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{	
	int shift0Index = pParams->ratesBuffers->rates[B_HOURLY_RATES].info.arraySize - 1;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	time_t currentTime;
	struct tm timeInfo1;
	int index = 0;

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_HOURLY_RATES].time[shift0Index]);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	pIndicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	if (pIndicators->strategy_mode > 0)
		pIndicators->weeklyATR = iAtr(B_WEEKLY_RATES, 4, 1);

	pIndicators->ma1H50M = iMA(3, B_HOURLY_RATES, 50, 1);
	pIndicators->ma1H200M = iMA(3, B_HOURLY_RATES, 200, 1);

	if (pIndicators->strategy_mode > 0)
	{
		pIndicators->ma4H50M = iMA(3, B_FOURHOURLY_RATES, 50, 1);
		pIndicators->ma4H200M = iMA(3, B_FOURHOURLY_RATES, 200, 1);
	}

	logDebug("System InstanceID = %d, BarTime = %s, MA1H200M = %lf,MA4H200M=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->ma1H200M, pIndicators->ma4H200M);

	iPivot(B_DAILY_RATES,1, &(pIndicators->dailyPivot),
		&(pIndicators->dailyS1), &(pIndicators->dailyR1),
		&(pIndicators->dailyS2), &(pIndicators->dailyR2),
		&(pIndicators->dailyS3), &(pIndicators->dailyR3));


	logDebug("System InstanceID = %d, BarTime = %s, dailyPivot = %lf,dailyS1=%lf, dailyR1 = %lf,dailyS2=%lf, dailyR2 = %lf,dailyS3=%lf, dailyR3 = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyPivot, pIndicators->dailyS1, pIndicators->dailyR1, pIndicators->dailyS2, pIndicators->dailyR2, pIndicators->dailyS3, pIndicators->dailyR3);

	if (pIndicators->strategy_mode > 0)
	{
		iPivot(B_WEEKLY_RATES, 1, &(pIndicators->weeklyPivot),
			&(pIndicators->weeklyS1), &(pIndicators->weeklyR1),
			&(pIndicators->weeklyS2), &(pIndicators->weeklyR2),
			&(pIndicators->weeklyS3), &(pIndicators->weeklyR3));

		logDebug("System InstanceID = %d, BarTime = %s, weeklyPivot = %lf,weeklyS1=%lf, weeklyR1 = %lf,weeklyS2=%lf, weeklyR2 = %lf,weeklyS3=%lf, weeklyR3 = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyPivot, pIndicators->weeklyS1, pIndicators->weeklyR1, pIndicators->weeklyS2, pIndicators->weeklyR2, pIndicators->weeklyS3, pIndicators->weeklyR3);
	}

	if (pIndicators->strategy_mode > 0)
	{
		loadMonthlyIndicators(pParams, pIndicators);
		loadWeeklyIndicators(pParams, pIndicators);
	}
	
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
	{
		index = 1;
	}

	loadDailyIndicators(pParams, pIndicators,index);
	loadIntradayKeyKIndicators(pParams, pIndicators);
	

	//Those two are used for weekly swing only. They are inactive now.
	//TODO: should be clean up.
	pIndicators->maTrend = getMATrend(iAtr(B_PRIMARY_RATES, 20, 1), B_PRIMARY_RATES, 1);
	pIndicators->ma_Signal = getMATrend_Signal(B_PRIMARY_RATES);

	
	return SUCCESS;
}

// Export loadIndicators for use by BaseCore (make it non-static)
AsirikuyReturnCode loadIndicators_Internal(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	return loadIndicators(pParams, pIndicators);
}

