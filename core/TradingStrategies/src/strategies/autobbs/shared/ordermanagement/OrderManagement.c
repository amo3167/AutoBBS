/*
 * Order Management Module
 * 
 * Provides order management functions for strategy execution.
 * Handles trade entries, exits, and order modifications.
 */

#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"
#include <math.h>

AsirikuyReturnCode modifyOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, OrderType orderType, double stopLoss, double takePrice)
{
	int tpMode = 0;
	int shift0Index;
	time_t currentTime;
	double stopLoss2;

	shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	if ((int)pParams->settings[TIMEFRAME] >= 60 && isNewDay(pParams, currentTime))
	{
		setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
		tpMode = 1;
	}

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			if ((int)parameter(AUTOBBS_TREND_MODE) == 5) // Day Trading, override the stop loss to primary bbs on the new day.
			{
				// stopLoss2 = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_primary) + pIndicators->adjust;

				modifyTradeEasy_DayTrading(BUY, -1, stopLoss, pIndicators->bbsStopPrice_primary, -1, tpMode, currentTime, pIndicators->adjust, FALSE);
			}
	
			//{
			//	takePrice = adjustTakePrice_Weekly_Swing_Easy(B_HOURLY_RATES, pBase_Indicators->pWeeklyATR/3);
			//	modifyTradeEasy_new(BUY, -1, stopLoss, takePrice, tpMode, FALSE);
			// }
			else
				modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode, FALSE); // New day TP change as
		}
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			if ((int)parameter(AUTOBBS_TREND_MODE) == 5) // Day Trading, override the stop loss to primary bbs on the new day.
			{
				// stopLoss2 = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_primary) + pIndicators->adjust;
				modifyTradeEasy_DayTrading(SELL, -1, stopLoss, pIndicators->bbsStopPrice_primary, -1, tpMode, currentTime, pIndicators->adjust, FALSE);
			}
			// [Comment removed - encoding corrupted]
			//{
			//	takePrice = adjustTakePrice_Weekly_Swing_Easy(B_HOURLY_RATES, pBase_Indicators->pWeeklyATR / 3);
			//	modifyTradeEasy_new(SELL, -1, stopLoss, takePrice, tpMode, FALSE);
			// }
			else
				modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode, FALSE); // New day TP change as
		}
	}

	return SUCCESS;
}
AsirikuyReturnCode handleTradeEntries(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double stopLoss, takePrice_primary = 0;
	int riskcap = (int)parameter(AUTOBBS_RISK_CAP);

	if (pParams == NULL)
	{
		logCritical("handleTradeEntries() failed. pParams = NULL");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		logCritical("handleTradeEntries() failed. pIndicators = NULL");
		return NULL_POINTER;
	}

	// When the trade is created from 23:45 to 00:20, increase the 3 times of spread for cut over
	if ((hour() == 23 && minute() > 40) || (hour() == 00 && minute() < 20))
		pIndicators->adjust = 3 * pIndicators->adjust;

	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) + pIndicators->adjust;

	switch (pIndicators->tpMode)
	{
	case 0:
		takePrice_primary = stopLoss;
		break;
	case 1:
		takePrice_primary = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_secondary) + pIndicators->adjust;
		pIndicators->risk = pIndicators->risk * min(stopLoss / takePrice_primary, riskcap);
		break;
	case 2:
		takePrice_primary = 0;
		break;
	case 3:
		takePrice_primary = pBase_Indicators->dailyATR;
		break;
	default:
		takePrice_primary = stopLoss;
		break;
	}

	if (pIndicators->executionTrend == 1)
	{
		if (pIndicators->entrySignal == 1)
			splitBuyOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		else if (totalOpenOrders(pParams, BUY) > 0)
			modifyOrders(pParams, pIndicators, pBase_Indicators, BUY, stopLoss, -1);
	}

	if (pIndicators->executionTrend == -1)
	{
		if (pIndicators->entrySignal == -1)
			splitSellOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		else if (totalOpenOrders(pParams, SELL) > 0)
			modifyOrders(pParams, pIndicators, pBase_Indicators, SELL, stopLoss, -1);
	}
	return SUCCESS;
}
AsirikuyReturnCode handleTradeExits(StrategyParams *pParams, Indicators *pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	if (pParams == NULL)
	{
		logCritical("handleTradeExits() failed. pParams = NULL\n\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		logCritical("handleTradeExits() failed. pIndicators = NULL\n\n");
		return NULL_POINTER;
	}

	// alwasy close all pre day limit orders.
	closeAllLimitPreviousDayOrders(pParams);

	switch (pIndicators->exitSignal)
	{
	case EXIT_SELL:
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}
		break;
	case EXIT_BUY:
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
		break;
	case EXIT_ALL:
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}

		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
		break;
	}

	return SUCCESS;
}
