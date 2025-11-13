/*
That is the base strategy in my auto strategies. 
It mainly provide the trend classification and support, resistance levels. 
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE


// Forward declaration
static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Base_Indicators * pIndicators);

AsirikuyReturnCode runBase(StrategyParams* pParams, Base_Indicators * pIndicators)
{
	loadIndicators(pParams, pIndicators);	
	return SUCCESS;
}

//Get the previous 10 weeks high or low. 
static AsirikuyReturnCode loadMonthlyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	int shift0Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index]);
	iSRLevels(pParams, pIndicators, B_WEEKLY_RATES, shift1Index, 8, &(pIndicators->monthlyHigh), &(pIndicators->monthlyLow));

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, 10weeksHigh=%lf, 10weeksLow = %lf",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, weeklyHLTrend = %ld,weeklyMATrend=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weeklyHLTrend, pIndicators->weeklyMATrend);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, weekly3RulesTrend = %ld,weeklyHigh=%lf, weeklyLow = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->weekly3RulesTrend, pIndicators->weeklyHigh, pIndicators->weeklyLow);

	workoutWeeklyTrend(pParams, pIndicators);
	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, weeklyTrend=%ld, weeklySupport = %lf,weeklyResistance = %lf,weeklyTP=%lf",
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
		// ���������ߵ͵�ܽ�������?=0.5 daily ATR�����Ǻõ��볡�źţ���λ���䡣
		// ���������ĸߵ͵�Զ������> 0.5 daily ATR, �Ͳ��Ǻܺõ��볡�źţ���λ���롣
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, intradayTrend = %ld,intradyIndex=%ld",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, dailyHLTrend = %ld,dailyMATrend=%ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyHLTrend, pIndicators->dailyMATrend);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, daily3RulesTrend = %ld,dailyHigh=%lf, dailyLow = %lf",
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
	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, dailyTrend=%ld, dailySupport = %lf,dailyResistance = %lf��dailyTP=%lf",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, MA1H200M = %lf,MA4H200M=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->ma1H200M, pIndicators->ma4H200M);

	iPivot(B_DAILY_RATES,1, &(pIndicators->dailyPivot),
		&(pIndicators->dailyS1), &(pIndicators->dailyR1),
		&(pIndicators->dailyS2), &(pIndicators->dailyR2),
		&(pIndicators->dailyS3), &(pIndicators->dailyR3));


	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, dailyPivot = %lf,dailyS1=%lf, dailyR1 = %lf,dailyS2=%lf, dailyR2 = %lf,dailyS3=%lf, dailyR3 = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->dailyPivot, pIndicators->dailyS1, pIndicators->dailyR1, pIndicators->dailyS2, pIndicators->dailyR2, pIndicators->dailyS3, pIndicators->dailyR3);

	if (pIndicators->strategy_mode > 0)
	{
		iPivot(B_WEEKLY_RATES, 1, &(pIndicators->weeklyPivot),
			&(pIndicators->weeklyS1), &(pIndicators->weeklyR1),
			&(pIndicators->weeklyS2), &(pIndicators->weeklyR2),
			&(pIndicators->weeklyS3), &(pIndicators->weeklyR3));

		fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, weeklyPivot = %lf,weeklyS1=%lf, weeklyR1 = %lf,weeklyS2=%lf, weeklyR2 = %lf,weeklyS3=%lf, weeklyR3 = %lf",
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

static BOOL isValidSupport(double position,double dailyATR)
{
	if (position <= 1.5 * dailyATR && position > 0.5 * dailyATR)
		return TRUE;

	return FALSE;
}

AsirikuyReturnCode workoutDailyTrend(StrategyParams* pParams, Base_Indicators* pIndicators)
{	
	double preLow = iLow(B_DAILY_RATES, 1);
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preClose = iClose(B_DAILY_RATES, 1);
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	pIndicators->dailyTrend = pIndicators->daily3RulesTrend + pIndicators->dailyHLTrend + pIndicators->dailyMATrend;

	if (pIndicators->dailyMATrend == 0 && pIndicators->daily3RulesTrend == 0)
		pIndicators->dailyTrend_Phase = RANGE_PHASE;
	if (pIndicators->daily3RulesTrend == UP && pIndicators->dailyMATrend != 2)
		pIndicators->dailyTrend_Phase = BEGINNING_UP_PHASE;
	else if (pIndicators->daily3RulesTrend == DOWN && pIndicators->dailyMATrend != -2)
		pIndicators->dailyTrend_Phase = BEGINNING_DOWN_PHASE;
	else if (pIndicators->dailyMATrend == 2 && pIndicators->dailyHLTrend >= 0)
		pIndicators->dailyTrend_Phase = MIDDLE_UP_PHASE;
	else if (pIndicators->dailyMATrend == 2 && pIndicators->dailyHLTrend < 0)
		pIndicators->dailyTrend_Phase = MIDDLE_UP_RETREAT_PHASE;
	else if (pIndicators->dailyMATrend == -2 && pIndicators->dailyHLTrend <= 0)
		pIndicators->dailyTrend_Phase = MIDDLE_DOWN_PHASE;
	else if (pIndicators->dailyMATrend == -2 && pIndicators->dailyHLTrend > 0)
		pIndicators->dailyTrend_Phase = MIDDLE_DOWN_RETREAT_PHASE;
	else
		pIndicators->dailyTrend_Phase = RANGE_PHASE;

	// �����������?��֧������λ�á�
	if (pIndicators->dailyTrend_Phase == RANGE_PHASE || pIndicators->dailyTrend == RANGE)
	{
		// Support
		if (isValidSupport(preClose - pIndicators->dailyLow, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->dailyLow;
		else if (isValidSupport(preClose - iLow(B_WEEKLY_RATES, 0), pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = iLow(B_WEEKLY_RATES, 0);
		else
			pIndicators->dailyS = preClose - pIndicators->dailyATR;

		// Resistance
		if (isValidSupport(pIndicators->dailyHigh - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyR = pIndicators->dailyHigh;
		else if (isValidSupport(iHigh(B_WEEKLY_RATES, 0) - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyR = iHigh(B_WEEKLY_RATES, 0);
		else
			pIndicators->dailyR = preClose + pIndicators->dailyATR;

		pIndicators->dailyTP = pIndicators->dailyR2;
	}
	else if (pIndicators->dailyTrend > RANGE)
	{
		if (isValidSupport(preClose - pIndicators->dailyLow, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->dailyLow;
		else if (isValidSupport(preClose - pIndicators->ma1H200M, pIndicators->dailyATR) == TRUE)			
			pIndicators->dailyS = pIndicators->ma1H200M;		
		else if (isValidSupport(preClose - preLow, pIndicators->dailyATR) == TRUE)			
			pIndicators->dailyS = preLow;			
		else if (isValidSupport(pParams->bidAsk.ask[0] - currentLow, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = currentLow;
		else
			pIndicators->dailyS = preClose - pIndicators->dailyATR;		
		
		pIndicators->dailyR = pIndicators->weeklyHigh;
		if (preClose - pIndicators->dailyHigh < 0)
			pIndicators->dailyR = pIndicators->dailyHigh;
		else if (preClose - pIndicators->ma4H200M < 0)
			pIndicators->dailyR = min(pIndicators->weeklyHigh, pIndicators->ma4H200M);

		if (preClose - pIndicators->dailyR > 0)
			pIndicators->dailyR = pIndicators->monthlyHigh;

		// No resistance in two months.
		if (preClose - pIndicators->dailyR > 0)
			pIndicators->dailyR = -1;

		pIndicators->dailyTP = pIndicators->dailyR2;
		
	}
	else if (pIndicators->dailyTrend < RANGE)
	{

		if (isValidSupport(pIndicators->dailyHigh - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->dailyHigh;
		else if (isValidSupport(pIndicators->ma1H200M - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = pIndicators->ma1H200M;
		else if (isValidSupport(preHigh - preClose, pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = preHigh;
		else if (isValidSupport(currentHigh - pParams->bidAsk.bid[0], pIndicators->dailyATR) == TRUE)
			pIndicators->dailyS = currentHigh;
		else
			pIndicators->dailyS = preClose + pIndicators->dailyATR;

		pIndicators->dailyR = pIndicators->weeklyLow;
		if (preClose - pIndicators->dailyLow > 0)
			pIndicators->dailyR = pIndicators->dailyLow;
		else if (preClose - pIndicators->ma4H200M > 0)
			pIndicators->dailyR = max(pIndicators->weeklyLow, pIndicators->ma4H200M);
		
		if (preClose - pIndicators->dailyR < 0)
			pIndicators->dailyR = pIndicators->monthlyLow;

		// No resistance in two months.
		if (preClose - pIndicators->dailyR < 0)
			pIndicators->dailyR = -1;

		pIndicators->dailyTP = pIndicators->dailyS2;
	}

	return SUCCESS;
}

AsirikuyReturnCode workoutWeeklyTrend(StrategyParams* pParams, Base_Indicators* pIndicators)
{
	double preLow = iLow(B_WEEKLY_RATES, 1);
	double preHigh = iHigh(B_WEEKLY_RATES, 1);
	double preClose = iClose(B_WEEKLY_RATES, 1);
	double currentLow = iLow(B_WEEKLY_RATES, 0);
	double currentHigh = iHigh(B_WEEKLY_RATES, 0);

	if (pIndicators->weeklyMATrend == 0 && pIndicators->weekly3RulesTrend == 0)
		pIndicators->weeklyTrend_Phase = RANGE_PHASE;
	if (pIndicators->weekly3RulesTrend == UP && pIndicators->weeklyMATrend != 2)
		pIndicators->weeklyTrend_Phase = BEGINNING_UP_PHASE;
	else if (pIndicators->weekly3RulesTrend == DOWN && pIndicators->weeklyMATrend != -2)
		pIndicators->weeklyTrend_Phase = BEGINNING_DOWN_PHASE;
	else if (pIndicators->weeklyMATrend == 2 && pIndicators->weeklyHLTrend >= 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_UP_PHASE;
	else if (pIndicators->weeklyMATrend == 2 && pIndicators->weeklyHLTrend < 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_UP_RETREAT_PHASE;
	else if (pIndicators->weeklyMATrend == -2 && pIndicators->weeklyHLTrend <= 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_DOWN_PHASE;
	else if (pIndicators->weeklyMATrend == -2 && pIndicators->weeklyHLTrend > 0)
		pIndicators->weeklyTrend_Phase = MIDDLE_DOWN_RETREAT_PHASE;
	else
		pIndicators->weeklyTrend_Phase = RANGE_PHASE;

	// �����������?��֧������λ�á�
	pIndicators->weeklyTrend = pIndicators->weekly3RulesTrend + pIndicators->weeklyHLTrend + pIndicators->weeklyMATrend;

	if (pIndicators->weeklyTrend_Phase == RANGE_PHASE || pIndicators->weeklyTrend == RANGE)
	{

		// Support
		if (isValidSupport(preClose - pIndicators->weeklyLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = pIndicators->weeklyLow;
		else if (isValidSupport(iLow(B_WEEKLY_RATES, 0) - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = iLow(B_WEEKLY_RATES, 0);
		else
			pIndicators->weeklyS = preClose - pIndicators->weeklyATR * 0.666;

		// Resistance
		if (isValidSupport(pIndicators->weeklyHigh - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyR = pIndicators->weeklyHigh;
		else if (isValidSupport(iHigh(B_WEEKLY_RATES, 0) - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyR = iHigh(B_WEEKLY_RATES, 0);
		else
			pIndicators->weeklyR = preClose + pIndicators->weeklyATR * 0.666;

		pIndicators->weeklyTP = pIndicators->weeklyR2;

	}
	else if (pIndicators->weeklyTrend > RANGE)
	{

		if (isValidSupport(preClose - pIndicators->weeklyLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = pIndicators->weeklyLow;		
		//else if (isValidSupport(preClose - pIndicators->ma4H200M, pIndicators->weeklyATR) == TRUE)
		//	pIndicators->weeklyS = pIndicators->ma4H200M;
		else if (isValidSupport(preClose - preLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = preLow;
		else if (isValidSupport(pParams->bidAsk.bid[0] - currentLow, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = currentLow;
		else
			pIndicators->weeklyS = preClose - pIndicators->weeklyATR * 0.666;


		pIndicators->weeklyR = pIndicators->monthlyHigh;
		// No resistance in two months.
		if (preClose - pIndicators->weeklyR > 0)
			pIndicators->weeklyR = -1;

		pIndicators->weeklyTP = pIndicators->weeklyR2;

	}
	else if (pIndicators->weeklyTrend < RANGE)
	{
		if (isValidSupport(pIndicators->weeklyHigh - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = pIndicators->weeklyHigh;
		//else if (isValidSupport(pIndicators->ma4H200M - preClose, pIndicators->weeklyATR) == TRUE)
		//	pIndicators->weeklyS = pIndicators->ma4H200M;
		else if (isValidSupport(preHigh - preClose, pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = preHigh;
		else if (isValidSupport(currentHigh - pParams->bidAsk.ask[0], pIndicators->weeklyATR) == TRUE)
			pIndicators->weeklyS = currentHigh;
		else
			pIndicators->weeklyS = preClose + pIndicators->weeklyATR* 0.666;

		pIndicators->weeklyR = pIndicators->monthlyLow;

		// No resistance in two months.
		if (preClose - pIndicators->weeklyR < 0)
			pIndicators->weeklyR = -1;

		pIndicators->weeklyTP = pIndicators->weeklyS2;
	}

	return SUCCESS;
}

AsirikuyReturnCode iSRLevels_close(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	//int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	//int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;

	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].close, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].close, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}
	return SUCCESS;
}

AsirikuyReturnCode iSRLevels(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	//int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	//int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;
	
	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	return SUCCESS;
}

AsirikuyReturnCode iSRLevels_WithIndex(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex,int shift, double *pHigh, double *pLow,int *pHighIndex,int *pLowIndex)
{
	TA_RetCode retCode;
	//int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	//int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;	
	int highIndex, lowIndex;

	retCode = TA_MIN(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}
		
	retCode = TA_MAX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);
	
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	retCode = TA_MININDEX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLowIndex);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MININDEX()", retCode);
	}

	retCode = TA_MAXINDEX(shfitIndex, shfitIndex, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHighIndex);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAXINDEX()", retCode);
	}

	return SUCCESS;
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend_HL_preDays(int ratesArrayIndex, int *trend,int preDays,int index)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	preHigh1 = iHigh(ratesArrayIndex, 1 + preDays - index);
	preHigh2 = iHigh(ratesArrayIndex, 2 + preDays - index);

	preClose1 = iClose(ratesArrayIndex, 1 + preDays - index);
	preClose2 = iClose(ratesArrayIndex, 2 + preDays - index);

	preLow1 = iLow(ratesArrayIndex, 1 + preDays - index);
	preLow2 = iLow(ratesArrayIndex, 2 + preDays - index);

	if (preHigh1 > preHigh2 &&
		//preLow1 > preLow2 &&
		preClose1 > preClose2)
	{
		*trend = UP_WEAK;
	}

	if (//preHigh1 < preHigh2 &&
		preLow1 < preLow2 &&
		preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}

	return SUCCESS;
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
//����Ƿ�����ߵ���ߣ��͵���ߣ���������Ҳ�Ǹ��ߣ��Ǿ���UP
AsirikuyReturnCode iTrend_HL(int ratesArrayIndex,int *trend,int index)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	preHigh1 = iHigh(ratesArrayIndex, 1 - index);
	preHigh2 = iHigh(ratesArrayIndex, 2 - index);
	
	preClose1 = iClose(ratesArrayIndex, 1 - index);
	preClose2 = iClose(ratesArrayIndex, 2 - index);
	
	preLow1 = iLow(ratesArrayIndex, 1 - index);
	preLow2 = iLow(ratesArrayIndex, 2 - index);
	
	if (preHigh1 > preHigh2 &&
		//preLow1 > preLow2 &&
		preClose1 > preClose2 )
	{
		*trend = UP_WEAK;
	}
	
	if (//preHigh1 < preHigh2 &&
		preLow1 < preLow2 &&
		preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}	
		
	return SUCCESS;
}

int getMATrend_SignalBase(int rateShort,int rateLong,int ratesArrayIndex,int maxBars)
{
	int i = 0, j, maTrend, maTrend_Prev;
	double adjust = iAtr(ratesArrayIndex, 20, 1);

	maTrend = getMATrendBase(rateShort,rateLong,adjust, ratesArrayIndex, 1);
	if (maTrend == 0)
		//if (fabs(maTrend) < 2)
		return 0;

	for (i = 1; i < maxBars; i++)
	{
		maTrend_Prev = getMATrendBase(rateShort,rateLong,adjust, ratesArrayIndex, i + 1);
		
		if (maTrend > 0 && maTrend_Prev < 0)
		{			
			return 1;
		}

		if (maTrend < 0 && maTrend_Prev > 0)
		{
			return -1;
		}
	}
		
	return 0;
}

int getMATrend_Signal(int ratesArrayIndex)
{
	return getMATrend_SignalBase(50, 200, ratesArrayIndex,24);
}

int getMATrendBase(int rateShort,int rateLong,double iATR, int ratesArrayIndex, int index)
{
	double maShort, maLong;
	int trend;

	maShort = iMA(3, ratesArrayIndex, rateShort, index);
	maLong = iMA(3, ratesArrayIndex, rateLong, index);

	if (maShort > maLong)
	{
		trend = 1;
		if (maShort - maLong >= iATR)
			trend = 2;
	}
	else if (maShort < maLong)
	{
		trend = -1;
		if (maLong - maShort >= iATR)
			trend = -2;
	}
	else
		trend = 0;
	return trend;
}

int getMATrend(double iATR, int ratesArrayIndex, int index)
{
	return getMATrendBase(50, 200, iATR, ratesArrayIndex, index);
}

// Daily: 1H 200M , 50M
// Weekly: 1H 200M, 50M
AsirikuyReturnCode iTrend_MA(double iATR,int ratesArrayIndex,int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, ratesArrayIndex, 50, 1);
	ma200M = iMA(3, ratesArrayIndex, 200, 1);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

int iTrendMA_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex,int signal)
{
	int trend[100] = { 0 };
	int i = 0;
	double ma50M, ma200M;	
	int truningIndex = 100;

	for (i = 1; i < 100; i++)
	{
		trend[i] = getMATrend(iAtr(ratesArrayIndex, 20, 1), ratesArrayIndex, i);
		if ( (signal > 0 && trend[i] != UP_NORMAL) ||
			(signal < 0 && trend[i] != DOWN_NORMAL))
		{
			truningIndex = i;
			break;
		}
	}

	return truningIndex;

}


AsirikuyReturnCode iTrend_MA_WeeklyBar_For4H(double iATR, int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, B_WEEKLY_RATES, 1, 1);
	ma200M = iMA(3, B_WEEKLY_RATES, 6, 1);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend_MA_DailyBar_For1H(double iATR, int *trend,int index)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, B_DAILY_RATES, 2, 1-index);
	ma200M = iMA(3, B_DAILY_RATES, 8, 1 - index);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

AsirikuyReturnCode iTrend3Rules_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int        outBegIdx, outNBElement;
	double	  boxHigh[100] = { 0 }, boxLow[100] = { 0 };
	int trend[100] = { 0 };
	int i = 0;

	retCode = TA_MIN(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	for (i = 0; i < outNBElement; i++)
	{
		// Check close price. 
		trend[i] = RANGE;
		if (boxHigh[i] == 0 || boxLow[i] == 0)
			break;

		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) > boxHigh[i]) //�ܹ������ϡ� c3 > max(h1,h2) or c3 > max(c1,c2) && h3>max(h1,h2) && l3>max(l1,l2)
			trend[i] = UP_NORMAL;

		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) < boxLow[i]) //�ܹ������ϡ�
			trend[i] = DOWN_NORMAL;

		if (i - 1 >= 0 && trend[i - 1] == UP && trend[i] == RANGE)
			trend[i] = RANGE;

		if (i - 1 >= 0 && trend[i - 1] == DOWN && trend[i] == RANGE)
			trend[i] = RANGE;

		*pTrend = trend[i];
	}	

	return SUCCESS;
	
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend3Rules_preDays(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend,int preDays,int index)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 - preDays;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2 - preDays;
	int       outBegIdx, outNBElement;
	double	  boxHigh, boxLow;

	retCode = TA_MIN(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	*pTrend = RANGE;
	if (iClose(ratesArrayIndex, 1 - index) > boxHigh) //�ܹ������ϡ� c3 > max(h1,h2)
		*pTrend = UP;

	if (iClose(ratesArrayIndex, 1 - index) < boxLow) //�ܹ������ϡ�
		*pTrend = DOWN;

	return SUCCESS;

}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend3Rules(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend,int index)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int       outBegIdx, outNBElement;
	double	  boxHigh, boxLow;	

	retCode = TA_MIN(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	*pTrend = RANGE;
	if (iClose(ratesArrayIndex, 1 - index) > boxHigh) //�ܹ������ϡ� c3 > max(h1,h2)
			*pTrend = UP;

	if (iClose(ratesArrayIndex, 1 - index) < boxLow) //�ܹ������ϡ�
			*pTrend = DOWN;

	return SUCCESS;

}

AsirikuyReturnCode base_ModifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice)
{	
	int tpMode = 0;

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
			modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
			modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	return SUCCESS;
}


//Predict daily ATR range. And it should be on SOD mode and Intraday Mode
//Forcast UP range and DOWN range according to S2/R2 and Weekly RANGE. 
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
	
	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, shortDailyATR = %f,mediumDailyATR=%f,longDailyATR=%f,minATR=%f,maxATR=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyATR, mediumDailyATR, longDailyATR, minATR, maxATR);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, ATR0 = %f,ATR1=%f,ATR2=%f,ATR3=%f,ATR4=%f",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameWeekDay=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameWeekDay);

	pATR = min(pATR, pATRSameWeekDay);
	//pATR = min(pATR, minATR);
	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameWeekDay=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, pATRSameWeekDay, ATR0);

	//pATR = max(ATR0, pATR);

	//2. ʹ���ܲ������������ơ�

	//remainingATRInWeek = pIndicators->weeklyATR - iAtr(B_WEEKLY_RATES, 1, 0);


	//3. ʹ��pivot�����?���������£����з��ദ��
	
	//pUpATR = (pIndicators->dailyR2 - min(lastDailyClose, intradayLow));
	//pDownATR = (max(lastDailyClose, intradayHigh) - pIndicators->dailyR2);

	
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pDailyPredictATR=%f,pDailyATR=%f,pDailyHigh=%f,pDailyLow=%f, pDailyTrend = %ld,pMaxDailyHigh=%lf,pMaxDailyLow=%lf",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, shortWeeklyATR = %f,mediumWeeklyATR=%f,longWeeklyATR=%f,minATR=%f,maxATR=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyATR, mediumWeeklyATR, longWeeklyATR, minATR, maxATR);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, ATR0 = %f,ATR1=%f,ATR2=%f,ATR3=%f",
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
		
	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pATR=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, ATR0);

	//Get average atr on the same week day.
	//pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) + iAtr(B_WEEKLY_RATES, 1, 12) + iAtr(B_WEEKLY_RATES, 1, 16)) / 4;
	pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) ) / 2;

	pATR = min(pATR, pATRSameMonthWeek);
	pATR = min(pATR, minATR);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameMonthWeek=%f,ATR0=%f ",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pWeeklyPredictATR=%f,pWeeklyATR=%f,pWeeklyHigh=%f,pWeeklyLow=%f, pWeeklyTrend = %ld,pMaxWeeklyHigh=%f,pMaxWeeklyLow=%f",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, shortWeeklyATR = %f,mediumWeeklyATR=%f,longWeeklyATR=%f,minATR=%f,maxATR=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyATR, mediumWeeklyATR, longWeeklyATR, minATR, maxATR);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s, ATR0 = %f,ATR1=%f,ATR2=%f,ATR3=%f",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pATR=%f,ATR0=%f ",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pATR, ATR0);

	//Get average atr on the same week day.
	pATRSameMonthWeek = (iAtr(B_WEEKLY_RATES, 1, 4) + iAtr(B_WEEKLY_RATES, 1, 8) + iAtr(B_WEEKLY_RATES, 1, 12) + iAtr(B_WEEKLY_RATES, 1, 16)) / 4;	

	pATR = min(pATR, pATRSameMonthWeek);
	pATR = min(pATR, minATR);

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pATR=%f,pATRSameMonthWeek=%f,ATR0=%f ",
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

	fprintf(stderr, "[DEBUG] System InstanceID = %d, BarTime = %s,pWeeklyPredictATR=%f,pWeeklyATR=%f,pWeeklyHigh=%f,pWeeklyLow=%f, pWeeklyTrend = %ld,pMaxWeeklyHigh=%f,pMaxWeeklyLow=%f",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->pWeeklyPredictATR, pIndicators->pWeeklyATR, pIndicators->pWeeklyHigh, pIndicators->pWeeklyLow, pIndicators->pWeeklyTrend, pIndicators->pMaxWeeklyHigh, pIndicators->pMaxWeeklyLow);

	//setPredictDailyATR((int)pParams->settings[STRATEGY_INSTANCE_ID], pIndicators->pDailyATR, (BOOL)pParams->settings[IS_BACKTESTING]);
}


void closeAllLimitPreviousDayOrders(StrategyParams* pParams)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	closeAllBuyLimitOrdersEasy(currentTime);
	closeAllSellLimitOrdersEasy(currentTime);
}

BOOL isNewDay(StrategyParams* pParams, time_t currentTime)
{
	time_t virtualOrderEntryTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	char       timeString2[MAX_TIME_STRING_SIZE] = "";

	virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

	safe_gmtime(&timeInfo1, currentTime);
	safe_gmtime(&timeInfo2, virtualOrderEntryTime);

	safe_timeString(timeString1, currentTime);
	safe_timeString(timeString2, virtualOrderEntryTime);

	fprintf(stderr, "[DEBUG] Last order update time = %s,current time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime, currentTime) / 3600);

	if (virtualOrderEntryTime != -1 && timeInfo1.tm_mday != timeInfo2.tm_mday && timeInfo1.tm_min >= 15) // New day
	{
		fprintf(stderr, "[DEBUG] Move to a new day.\n");
		//setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
		return TRUE;
	}
	return FALSE;
}
