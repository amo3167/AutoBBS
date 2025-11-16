/*
 * Multiple Day Order Management Module
 * 
 * Provides order entry and modification functions for multiple day trading strategies.
 * Used by GBPJPY_MultipleDay and MultipleDay_V2 strategies.
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
#include "strategies/autobbs/swing/common/MultipleDayOrderManagement.h"

/**
 * Modify order for multiple day trading strategies.
 * Handles take profit and stop loss modifications for open orders.
 */
AsirikuyReturnCode modifyOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, double openOrderHigh, double openOrderLow, double floatingTP, int takeProfitMode, BOOL isLongTerm)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double entryPrice = 0;
	OrderType side;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	side = pParams->orderInfo[latestOrderIndex].type;

	if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;

		logWarning("System InstanceID = %d, BarTime = %s, takeProfitMode =%d, lastClose=%lf, lastOpen=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, takeProfitMode, iClose(B_PRIMARY_RATES, 1), iOpen(B_PRIMARY_RATES, 1));

		if (side == SELL)
		{
			if (entryPrice - openOrderLow > pIndicators->takePrice &&
				entryPrice - pParams->bidAsk.ask[0] < floatingTP
				&& (takeProfitMode == 0 || iClose(B_PRIMARY_RATES, 1) > iOpen(B_PRIMARY_RATES, 1))
				)
			{
				if (isLongTerm == TRUE){
					pIndicators->exitSignal = EXIT_SELL;
				}
				else{
					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
				}
				logWarning("System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
				return SUCCESS;
			}

			if (pParams->bidAsk.ask[0] - openOrderLow >= pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
				pIndicators->exitSignal = EXIT_SELL;
			}
		}

		if (side == BUY)
		{
			if (openOrderHigh - entryPrice > pIndicators->takePrice &&
				pParams->bidAsk.bid[0] - entryPrice < floatingTP
				&& (takeProfitMode == 0 || iClose(B_PRIMARY_RATES, 1) < iOpen(B_PRIMARY_RATES, 1))
				)
			{
				if (isLongTerm == TRUE)
					pIndicators->exitSignal = EXIT_BUY;
				else 
					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

				logWarning("System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
				return SUCCESS;
			}

			if (openOrderHigh - pParams->bidAsk.bid[0] >= pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
				pIndicators->exitSignal = EXIT_BUY;
			}
		}
	}
	return SUCCESS;
}

/**
 * Enter order for multiple day trading strategies.
 * Uses Bollinger Bands to determine entry signals.
 */
AsirikuyReturnCode enterOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double riskCapBuy, double riskCapSell, BOOL isSameDayClosedOrder)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double upperBBand, lowerBBand;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double currentClose = iClose(B_DAILY_RATES, 0);
	double adjustGap;
	double adjustRisk;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	if (pBase_Indicators->maTrend > 0
		&& pBase_Indicators->dailyTrend >= -1
		)
	{
		upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);

		logWarning("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


		if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;

			pIndicators->riskCap = riskCapBuy;
			
			if (isSameDayClosedOrder == FALSE){
				pIndicators->entrySignal = 1;
				adjustGap = pBase_Indicators->dailyATR - (currentClose - currentLow);
								
			}

			pIndicators->exitSignal = EXIT_SELL;
		}

	}

	if (pBase_Indicators->maTrend < 0
		&& pBase_Indicators->dailyTrend <= 1
		)
	{
		lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);

		logWarning("System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

		if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;

			pIndicators->riskCap = riskCapSell;

			if (isSameDayClosedOrder == FALSE){
				pIndicators->entrySignal = -1;

				adjustGap = pBase_Indicators->dailyATR - (currentHigh - currentClose);

			}
			
			pIndicators->exitSignal = EXIT_BUY;
		}
	}

	

	if (pIndicators->entrySignal != 0)
	{
		adjustRisk = min(1, (adjustGap - pIndicators->takePrice) / pIndicators->takePrice);

		if (adjustRisk > 0.5)
			pIndicators->risk = 1;
		else if (adjustRisk > 0)
			pIndicators->risk = 0.5;

		if (adjustRisk <= 0)
		{
			sprintf(pIndicators->status, "risk = %lf", adjustRisk);

			logInfo("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;

			return FALSE;
		}
	}

	return SUCCESS;
}

