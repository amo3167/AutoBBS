/*
 * Weekly Auto Strategy Module
 * 
 * Provides Weekly Auto strategy execution functions.
 * This strategy adapts its behavior based on weekly trend phases, using different
 * entry methods (Pivot, BBS BreakOut) and order splitting modes depending on the phase.
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

/* Order splitting mode constants */
#define SPLIT_MODE_WEEKLY_BEGINNING 11  /* Beginning phase order splitting mode */
#define SPLIT_MODE_WEEKLY_MIDDLE 7      /* Middle phase order splitting mode */

/**
 * Weekly Auto strategy execution.
 * 
 * This strategy adapts its behavior based on the weekly trend phase:
 * - Beginning phases: Uses Pivot strategy, falls back to BBS BreakOut if no entry
 * - Middle phases: Uses Pivot strategy, falls back to BBS BreakOut if no entry
 * - Middle retreat phases: Uses special retreat phase strategy
 * - Range phase: Modifies stop loss to weekly support/resistance or exits all
 * 
 * Key K priority: If Key K is enabled and triggered, it takes priority over other strategies.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing weekly trend phase
 * @return SUCCESS on success
 */
AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;

	/* Check if Key K is triggered - if yes, take priority over other strategies */
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
		/* Try Pivot strategy first, fall back to BBS BreakOut if no entry signal */
		workoutExecutionTrend_Weekly_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0)
			workoutExecutionTrend_Weekly_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
		pIndicators->splitTradeMode = SPLIT_MODE_WEEKLY_BEGINNING;
		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		/* Try Pivot strategy first, fall back to BBS BreakOut if no entry signal */
		workoutExecutionTrend_Weekly_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0)
			workoutExecutionTrend_Weekly_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
		pIndicators->splitTradeMode = SPLIT_MODE_WEEKLY_MIDDLE;
		break;
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
		workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
		break;
	case RANGE_PHASE:
		/* Range phase: Modify stop loss to weekly support/resistance or exit all */
		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			/* Modify BUY orders: Set stop loss to weekly support */
			if (totalOpenOrders(pParams, BUY) > 0)
			{
				stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->weeklyS) + pIndicators->adjust;
				modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
			}
			/* Modify SELL orders: Set stop loss to weekly resistance */
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->weeklyR) + pIndicators->adjust;
				modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		else
		{
			/* Exit all positions if range trading is disabled */
			pIndicators->exitSignal = EXIT_ALL;
		}
		break;
	}

	profitManagementWeekly(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}
