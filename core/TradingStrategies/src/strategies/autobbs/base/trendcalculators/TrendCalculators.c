/*
 * Trend Calculators Module
 * 
 * Provides functions for calculating daily and weekly trends,
 * including support/resistance levels and trend phases.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/trendcalculators/TrendCalculators.h"
#include "strategies/autobbs/base/supportresistance/SupportResistance.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

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

	// ?֧λá
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

	// ?֧λá
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

