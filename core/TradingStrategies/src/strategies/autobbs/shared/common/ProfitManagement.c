/*
 * Profit Management Module
 * 
 * Provides profit management functions for strategy execution.
 * Handles risk control, profit taking, and order closure based on profit targets.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/common/ProfitManagement.h"

void profitManagement_base(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{

	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	// Control risk by Macro factor
	if (pIndicators->entrySignal == 1)
	{
		if (pIndicators->macroTrend < 0)
		{
			pIndicators->risk *= 0.5;
			pIndicators->strategyMaxRisk *= 0.5;
		}
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= 0.8;
			pIndicators->strategyMaxRisk *= 0.8;
		}
	}

	if (pIndicators->entrySignal == -1)
	{
		if (pIndicators->macroTrend > 0)
		{
			pIndicators->risk *= 0.5;
			pIndicators->strategyMaxRisk *= 0.5;
		}
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= 0.8;
			pIndicators->strategyMaxRisk *= 0.8;
		}
	}

	if (pIndicators->entrySignal != 0 && pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->strategyMaxRisk)
	{
		logWarning("System InstanceID = %d, BarTime = %s, strategyRisk %lf�� strategyRiskWithoutLockedProfit %lf, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyMaxRisk, pIndicators->strategyRiskWithoutLockedProfit, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	if (pIndicators->riskPNL <  pIndicators->limitRiskPNL && pIndicators->entrySignal != 0)
	{
		logWarning("System InstanceID = %d, BarTime = %s, PNL riks %lf��riskPNLWithoutLockedProfit %lf, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}


}

void profitManagementWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	safe_timeString(timeString, currentTime);

	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	// Weekly Profit on EOW?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR3)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS3)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);


	// when floating profit is too high, fe 10%
	if (pBase_Indicators->weeklyTrend_Phase > 0
		&& pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2
		&& pIndicators->riskPNL > targetPNL)
	{
		logWarning("System InstanceID = %d, BarTime = %s, PNL riks %lf��riskPNLWithoutLockedProfit %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit);

		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}
	else if (pBase_Indicators->weeklyTrend_Phase < 0
		&& pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2
		&& pIndicators->riskPNL > targetPNL)
	{
		logWarning("System InstanceID = %d, BarTime = %s, PNL riks %lf��riskPNLWithoutLockedProfit %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit);
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}

	//closeAllWithNegativeEasy(4, currentTime, 3);

}

void profitManagement(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	//double targetPNL = parameter(ACCOUNT_RISK_PERCENT) * 3;
	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	int   noTPOrderDaysNumber = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	safe_timeString(timeString, currentTime);

	profitManagement_base(pParams, pIndicators, pBase_Indicators);
	noTPOrderDaysNumber = getSamePricePendingNoTPOrdersEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3);

	if (noTPOrderDaysNumber >= 4 && pIndicators->entrySignal != 0)
	{
		logWarning("System InstanceID = %d, BarTime = %s, SamePricePendingNoTPOrdersDays %d, skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, noTPOrderDaysNumber, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Daily Profit on EOD?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->dailyR3)
	{
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, over dailyR3 skip this entry signal=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	}
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->dailyS3)
	{
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, under dailyS3 skip this entry signal=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	}
	


	// Weekly Profit on EOW?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
	{
		//pIndicators->tradeMode = 0;
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, over weeklyR2 skip this entry signal=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2)
	{
		//pIndicators->tradeMode = 0;
		if (pIndicators->entrySignal != 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s, under weeklyS2 skip this entry signal=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
			pIndicators->entrySignal = 0;
		}
		else
			closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}


	// when floating profit is too high, fe 10%
	if (pBase_Indicators->dailyTrend_Phase > 0
		&& pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2
		&& pIndicators->riskPNL >targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	else if (pBase_Indicators->dailyTrend_Phase < 0
		&& pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2
		&& pIndicators->riskPNL > targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	//closeAllWithNegativeEasy(3, currentTime, 3);
	//closeAllWithNegativeEasy(2, currentTime,3);

	//// work out the Max DD
	//pIndicators->strategyMaxDD = pIndicators->riskPNL - pIndicators->strategyRisk;
	//if (pIndicators->strategyMaxDD > parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3)
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s, Max DD %lf",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyMaxDD);

	//	closeWinningPositionsEasy(pIndicators->riskPNL, pIndicators->riskPNL);
	//}

}
