/*
 * Multiple Day Order Management Module
 * 
 * Provides order entry and modification functions for multiple day trading strategies.
 * Used by GBPJPY_MultipleDay and MultipleDay_V2 strategies.
 * 
 * This module handles:
 * - Order modification based on profit targets and stop loss levels
 * - Entry signal generation using Bollinger Bands
 * - Risk adjustment based on daily ATR and price gaps
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

/* Bollinger Bands constants */
#define BBANDS_PERIOD 50                  /* Bollinger Bands period */
#define BBANDS_DEVIATIONS 2               /* Bollinger Bands standard deviations */
#define BBANDS_UPPER_BAND 0               /* Upper band index */
#define BBANDS_LOWER_BAND 2               /* Lower band index */

/* Risk adjustment constants */
#define RISK_ADJUSTMENT_THRESHOLD 0.5     /* Risk adjustment threshold (50%) */
#define RISK_FULL 1.0                     /* Full risk (100%) */
#define RISK_HALF 0.5                     /* Half risk (50%) */
#define RISK_MIN_VALUE 0.0                 /* Minimum risk value */

/* Take profit mode constants */
#define TP_MODE_CLOSE_ON_PROFIT 0         /* Close order when profit target reached */

/**
 * Modifies orders for multiple day trading strategies.
 * 
 * Handles take profit and stop loss modifications for open orders based on:
 * - Profit targets (takePrice)
 * - Floating take profit levels
 * - Stop loss levels (daily ATR)
 * - Take profit mode (close on profit or wait for reversal)
 * 
 * For SELL orders:
 * - Closes order if profit >= takePrice AND floating profit < floatingTP
 * - Sets exit signal if price moves against position by stopLoss amount
 * 
 * For BUY orders:
 * - Closes order if profit >= takePrice AND floating profit < floatingTP
 * - Sets exit signal if price moves against position by stopLoss amount
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing daily ATR
 * @param latestOrderIndex Index of the latest open order to modify
 * @param openOrderHigh Highest price since order was opened
 * @param openOrderLow Lowest price since order was opened
 * @param floatingTP Floating take profit level
 * @param takeProfitMode Take profit mode (0 = close on profit, 1 = wait for reversal)
 * @param isLongTerm TRUE for long-term orders, FALSE for short-term orders
 * @return SUCCESS on success
 */
AsirikuyReturnCode modifyOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, double openOrderHigh, double openOrderLow, double floatingTP, int takeProfitMode, BOOL isLongTerm)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double entryPrice = 0;
	OrderType side;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	side = pParams->orderInfo[latestOrderIndex].type;

	if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;

		logWarning("System InstanceID = %d, BarTime = %s, takeProfitMode = %d, lastClose = %lf, lastOpen = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, takeProfitMode, iClose(B_PRIMARY_RATES, 1), iOpen(B_PRIMARY_RATES, 1));

		/* Handle SELL orders */
		if (side == SELL)
		{
			/* Close order if profit target reached and conditions met */
			if (entryPrice - openOrderLow > pIndicators->takePrice &&
			    entryPrice - pParams->bidAsk.ask[0] < floatingTP &&
			    (takeProfitMode == TP_MODE_CLOSE_ON_PROFIT || iClose(B_PRIMARY_RATES, 1) > iOpen(B_PRIMARY_RATES, 1)))
			{
				if (isLongTerm == TRUE)
				{
					pIndicators->exitSignal = EXIT_SELL;
				}
				else
				{
					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
				}
				logWarning("System InstanceID = %d, BarTime = %s, closing sell order: entryPrice = %lf, openOrderLow = %lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
				return SUCCESS;
			}

			/* Set exit signal if stop loss level reached (price moved against position) */
			if (pParams->bidAsk.ask[0] - openOrderLow >= pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
				pIndicators->exitSignal = EXIT_SELL;
			}
		}

		/* Handle BUY orders */
		if (side == BUY)
		{
			/* Close order if profit target reached and conditions met */
			if (openOrderHigh - entryPrice > pIndicators->takePrice &&
			    pParams->bidAsk.bid[0] - entryPrice < floatingTP &&
			    (takeProfitMode == TP_MODE_CLOSE_ON_PROFIT || iClose(B_PRIMARY_RATES, 1) < iOpen(B_PRIMARY_RATES, 1)))
			{
				if (isLongTerm == TRUE)
				{
					pIndicators->exitSignal = EXIT_BUY;
				}
				else
				{
					closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
				}
				logWarning("System InstanceID = %d, BarTime = %s, closing buy order: entryPrice = %lf, openOrderHigh = %lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
				return SUCCESS;
			}

			/* Set exit signal if stop loss level reached (price moved against position) */
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
 * Enters orders for multiple day trading strategies.
 * 
 * Uses Bollinger Bands to determine entry signals:
 * - BUY signal: Price closes above upper Bollinger Band AND MA trend is up AND daily trend >= -1
 * - SELL signal: Price closes below lower Bollinger Band AND MA trend is down AND daily trend <= 1
 * 
 * Risk adjustment:
 * - Calculates adjustGap based on daily ATR and current price movement
 * - Adjusts risk based on gap: >50% gap = full risk, 0-50% gap = half risk, <=0 gap = no entry
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing MA trend and daily ATR
 * @param riskCapBuy Risk cap for buy orders
 * @param riskCapSell Risk cap for sell orders
 * @param isSameDayClosedOrder TRUE if order was closed on same day, FALSE otherwise
 * @return SUCCESS on success, FALSE if risk adjustment results in no entry
 */
AsirikuyReturnCode enterOrder_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double riskCapBuy, double riskCapSell, BOOL isSameDayClosedOrder)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double upperBBand, lowerBBand;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double currentClose = iClose(B_DAILY_RATES, 0);
	double adjustGap = 0.0;
	double adjustRisk;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	/* Check for BUY signal: MA trend up AND daily trend >= -1 */
	if (pBase_Indicators->maTrend > 0 && pBase_Indicators->dailyTrend >= -1)
	{
		upperBBand = iBBands(B_PRIMARY_RATES, BBANDS_PERIOD, BBANDS_DEVIATIONS, BBANDS_UPPER_BAND, 1);

		logWarning("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));

		/* Entry signal: price closes above upper Bollinger Band */
		if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
			pIndicators->riskCap = riskCapBuy;
			
			if (isSameDayClosedOrder == FALSE)
			{
				pIndicators->entrySignal = 1;
				/* Calculate gap: remaining ATR after current price movement */
				adjustGap = pBase_Indicators->dailyATR - (currentClose - currentLow);
			}

			pIndicators->exitSignal = EXIT_SELL;
		}
	}

	/* Check for SELL signal: MA trend down AND daily trend <= 1 */
	if (pBase_Indicators->maTrend < 0 && pBase_Indicators->dailyTrend <= 1)
	{
		lowerBBand = iBBands(B_PRIMARY_RATES, BBANDS_PERIOD, BBANDS_DEVIATIONS, BBANDS_LOWER_BAND, 1);

		logWarning("System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

		/* Entry signal: price closes below lower Bollinger Band */
		if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
			pIndicators->riskCap = riskCapSell;

			if (isSameDayClosedOrder == FALSE)
			{
				pIndicators->entrySignal = -1;
				/* Calculate gap: remaining ATR after current price movement */
				adjustGap = pBase_Indicators->dailyATR - (currentHigh - currentClose);
			}
			
			pIndicators->exitSignal = EXIT_BUY;
		}
	}

	/* Adjust risk based on gap between ATR and current price movement */
	if (pIndicators->entrySignal != 0)
	{
		/* Calculate risk adjustment: (gap - takePrice) / takePrice
		 * Positive values indicate room for profit, negative values indicate insufficient room */
		adjustRisk = min(1.0, (adjustGap - pIndicators->takePrice) / pIndicators->takePrice);

		/* Set risk based on adjustment */
		if (adjustRisk > RISK_ADJUSTMENT_THRESHOLD)
		{
			/* Full risk if gap is more than 50% above take price */
			pIndicators->risk = RISK_FULL;
		}
		else if (adjustRisk > RISK_MIN_VALUE)
		{
			/* Half risk if gap is positive but less than 50% above take price */
			pIndicators->risk = RISK_HALF;
		}

		/* No entry if gap is insufficient (adjustRisk <= 0) */
		if (adjustRisk <= RISK_MIN_VALUE)
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
