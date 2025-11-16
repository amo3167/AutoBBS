/*
 * Stop Loss Management Module
 * 
 * Provides functions for managing stop loss levels for trading orders.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/common/StopLossManagement.h"

BOOL move_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, double stopLossLevel)
{
	//double realTakePrice = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / oldStopLossLevel;
	double realTakePrice = fabs(pParams->orderInfo[orderIndex].takeProfit - pParams->orderInfo[orderIndex].openPrice);

	pIndicators->stopMovingBackSL = FALSE;
	if (pParams->orderInfo[orderIndex].type == BUY)
	{
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;

		pIndicators->takePrice = -1;

		if (fabs(pIndicators->stopLossPrice- pParams->orderInfo[orderIndex].stopLoss) > pIndicators->adjust)
			pIndicators->executionTrend = 1;
		
	}

	if (pParams->orderInfo[orderIndex].type == SELL)
	{
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice * stopLossLevel;

		pIndicators->takePrice = -1;

		if (fabs(pIndicators->stopLossPrice- pParams->orderInfo[orderIndex].stopLoss) > pIndicators->adjust)
			pIndicators->executionTrend = -1;		
	}
	return TRUE;
}

