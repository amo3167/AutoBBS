/*
 * Weekly Auto Strategy Module
 * 
 * Provides Weekly Auto strategy execution functions.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/weekly/WeeklyAutoStrategy.h"
#include "strategies/autobbs/trend/weekly/WeeklyPivotStrategy.h"
#include "strategies/autobbs/trend/bbs/BBSBreakOutStrategy.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"

AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;

	//Check out if Key K is triggered. If yes, take piority with other strategies.
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
		workoutExecutionTrend_Weekly_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal		
			workoutExecutionTrend_Weekly_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
		pIndicators->splitTradeMode = 11;
		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		workoutExecutionTrend_Weekly_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal		
			workoutExecutionTrend_Weekly_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
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
				modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
			}
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->weeklyR) + pIndicators->adjust;
				modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	profitManagementWeekly(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}
