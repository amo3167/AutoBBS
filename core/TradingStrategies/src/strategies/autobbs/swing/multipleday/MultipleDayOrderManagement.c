/*
 * Multiple Day Order Management Module
 * 
 * Provides order entry and modification functions for multiple day trading strategies.
 * Used by workoutExecutionTrend_MultipleDay strategy.
 * 
 * This module handles:
 * - Order modification based on profit targets and stop loss levels
 * - Entry signal generation using Bollinger Bands
 * - Risk adjustment based on daily ATR and price gaps
 */


#include <stdio.h>
#include <math.h>
#include "OrderManagement.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayOrderManagement.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayHelpers.h"
#include "strategies/autobbs/swing/daytrading/DayTradingHelpers.h"

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

	/* Log initial conditions for debugging */
	logInfo("System InstanceID = %d, BarTime = %s, enterOrder_MultipleDay: maTrend = %ld, dailyTrend = %ld, isSameDayClosedOrder = %d",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, pBase_Indicators->dailyTrend, (int)isSameDayClosedOrder);

	/* Check for BUY signal: MA trend up AND daily trend >= -1 */
	if (pBase_Indicators->maTrend > 0 && pBase_Indicators->dailyTrend >= -1)
	{
		upperBBand = iBBands(B_PRIMARY_RATES, BBANDS_PERIOD, BBANDS_DEVIATIONS, BBANDS_UPPER_BAND, 1);
		double preCloseBar = iClose(B_PRIMARY_RATES, 1);

		logWarning("System InstanceID = %d, BarTime = %s, BUY check: upperBBand = %lf, preCloseBar = %lf, condition met = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, preCloseBar, (int)(upperBBand > 0 && preCloseBar > upperBBand));

		/* Entry signal: price closes above upper Bollinger Band */
		if (upperBBand > 0 && preCloseBar > upperBBand)
		{
			logInfo("System InstanceID = %d, BarTime = %s, BUY BBand condition PASSED: upperBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, preCloseBar);

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
			pIndicators->riskCap = riskCapBuy;
			
			if (isSameDayClosedOrder == FALSE)
			{
				pIndicators->entrySignal = 1;
				/* Calculate gap: remaining ATR after current price movement */
				adjustGap = pBase_Indicators->dailyATR - (currentClose - currentLow);
				logInfo("System InstanceID = %d, BarTime = %s, BUY entrySignal SET to 1, adjustGap = %lf, dailyATR = %lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, adjustGap, pBase_Indicators->dailyATR);
			}
			else
			{
				logWarning("System InstanceID = %d, BarTime = %s, BUY entrySignal BLOCKED: isSameDayClosedOrder = TRUE",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
			}

			pIndicators->exitSignal = EXIT_SELL;
		}
		else
		{
			logInfo("System InstanceID = %d, BarTime = %s, BUY BBand condition FAILED: upperBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, preCloseBar);
		}
	}
	else
	{
		logInfo("System InstanceID = %d, BarTime = %s, BUY trend condition FAILED: maTrend = %ld (need >0), dailyTrend = %ld (need >=-1)",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, pBase_Indicators->dailyTrend);
	}

	/* Check for SELL signal: MA trend down AND daily trend <= 1 */
	if (pBase_Indicators->maTrend < 0 && pBase_Indicators->dailyTrend <= 1)
	{
		lowerBBand = iBBands(B_PRIMARY_RATES, BBANDS_PERIOD, BBANDS_DEVIATIONS, BBANDS_LOWER_BAND, 1);
		double preCloseBar = iClose(B_PRIMARY_RATES, 1);

		logWarning("System InstanceID = %d, BarTime = %s, SELL check: lowerBBand = %lf, preCloseBar = %lf, condition met = %d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, preCloseBar, (int)(lowerBBand > 0 && preCloseBar < lowerBBand));

		/* Entry signal: price closes below lower Bollinger Band */
		if (lowerBBand > 0 && preCloseBar < lowerBBand)
		{
			logInfo("System InstanceID = %d, BarTime = %s, SELL BBand condition PASSED: lowerBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, preCloseBar);

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
			pIndicators->riskCap = riskCapSell;

			if (isSameDayClosedOrder == FALSE)
			{
				pIndicators->entrySignal = -1;
				/* Calculate gap: remaining ATR after current price movement */
				adjustGap = pBase_Indicators->dailyATR - (currentHigh - currentClose);
				logInfo("System InstanceID = %d, BarTime = %s, SELL entrySignal SET to -1, adjustGap = %lf, dailyATR = %lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, adjustGap, pBase_Indicators->dailyATR);
			}
			else
			{
				logWarning("System InstanceID = %d, BarTime = %s, SELL entrySignal BLOCKED: isSameDayClosedOrder = TRUE",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
			}
			
			pIndicators->exitSignal = EXIT_BUY;
		}
		else
		{
			logInfo("System InstanceID = %d, BarTime = %s, SELL BBand condition FAILED: lowerBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, preCloseBar);
		}
	}
	else
	{
		logInfo("System InstanceID = %d, BarTime = %s, SELL trend condition FAILED: maTrend = %ld (need <0), dailyTrend = %ld (need <=1)",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, pBase_Indicators->dailyTrend);
	}

	/* Adjust risk based on gap between ATR and current price movement */
	if (pIndicators->entrySignal != 0)
	{
		/* Calculate risk adjustment: (gap - takePrice) / takePrice
		 * Positive values indicate room for profit, negative values indicate insufficient room */
		adjustRisk = min(1.0, (adjustGap - pIndicators->takePrice) / pIndicators->takePrice);

		logInfo("System InstanceID = %d, BarTime = %s, Risk adjustment: adjustGap = %lf, takePrice = %lf, adjustRisk = %lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, adjustGap, pIndicators->takePrice, adjustRisk);

		/* Set risk based on adjustment */
		if (adjustRisk > RISK_ADJUSTMENT_THRESHOLD)
		{
			/* Full risk if gap is more than 50% above take price */
			pIndicators->risk = RISK_FULL;
			logInfo("System InstanceID = %d, BarTime = %s, Risk set to FULL (risk = %lf)",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->risk);
		}
		else if (adjustRisk > RISK_MIN_VALUE)
		{
			/* Half risk if gap is positive but less than 50% above take price */
			pIndicators->risk = RISK_HALF;
			logInfo("System InstanceID = %d, BarTime = %s, Risk set to HALF (risk = %lf)",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->risk);
		}

		/* No entry if gap is insufficient (adjustRisk <= 0) */
		if (adjustRisk <= RISK_MIN_VALUE)
		{
			sprintf(pIndicators->status, "risk = %lf", adjustRisk);

			logWarning("System InstanceID = %d, BarTime = %s, Entry BLOCKED by risk adjustment: %s (adjustRisk = %lf <= RISK_MIN_VALUE = %lf)",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status, adjustRisk, RISK_MIN_VALUE);

			pIndicators->entrySignal = 0;

			return FALSE;
		}
	}
	else
	{
		logInfo("System InstanceID = %d, BarTime = %s, No entrySignal set (entrySignal = %d)",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
	}

	return SUCCESS;
}

/**
 * Split buy orders for multiple days swing strategy.
 * Used by splitTradeMode 22.
 * 
 * Based on the original implementation from SwingStrategy.c.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing takePrice, riskCap, tradeMode, etc.
 * @param pBase_Indicators Base indicators structure
 * @param takePrice_primary Primary take profit value (not used, uses pIndicators->takePrice)
 * @param stopLoss Stop loss value
 */
void splitBuyOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard;
	double gap = pHigh - pIndicators->entryPrice;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

	takePrice = 0;

	if (pIndicators->tradeMode == 1)
	{
		if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			if (lots / pIndicators->volumeStep > 5)
			{
				takePrice = pIndicators->riskCap * stopLoss;
				openSingleLongEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = (pIndicators->riskCap + 2) * stopLoss;
				openSingleLongEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = 0;
				openSingleLongEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
				openSingleLongEasy(takePrice, stopLoss, lots, 0);
		}
		else
			openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * Split sell orders for multiple days swing strategy.
 * Used by splitTradeMode 22.
 * 
 * Based on the original implementation from SwingStrategy.c.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing takePrice, riskCap, tradeMode, etc.
 * @param pBase_Indicators Base indicators structure
 * @param takePrice_primary Primary take profit value (not used, uses pIndicators->takePrice)
 * @param stopLoss Stop loss value
 */
void splitSellOrders_MultiDays_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal, lots_standard;
	double gap = pHigh - pIndicators->entryPrice;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int lostDays;
	double total_pre_lost = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

	takePrice = 0;

	if (pIndicators->tradeMode == 1)
	{
		if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			if (lots / pIndicators->volumeStep > 5)
			{
				takePrice = pIndicators->riskCap * stopLoss;
				openSingleShortEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = (pIndicators->riskCap + 2) * stopLoss;
				openSingleShortEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize) / 2, 0);

				takePrice = 0;
				openSingleShortEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
				openSingleShortEasy(takePrice, stopLoss, lots, 0);
		}
		else
			openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

/**
 * Setup GBPJPY entry signal for multiple day trading strategies.
 * 
 * Handles GBPJPY-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window
 * - Handles add position logic for long-term trends
 * - Applies GBPJPY trading filter
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param executionTrend Current execution trend (-1, 0, or 1)
 * @param oldestOpenOrderIndex Index of oldest open order
 * @param side Order side (BUY, SELL, or NONE)
 * @param isAddPosition TRUE if adding position, FALSE otherwise
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param preLow Previous bar low price
 * @param preHigh Previous bar high price
 * @param preClose Previous bar close price
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (entry signal set or filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupGBPJPYEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int executionTrend, int oldestOpenOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, BOOL* shouldSkip)
{
	double addPositionBaseLine;
	
	/* Initialize shouldSkip to FALSE (continue processing by default) */
	*shouldSkip = FALSE;

	/* Adjust range: increase range for better filtering */
	if ((int)parameter(AUTOBBS_RANGE) == 1 && executionTrend != 0)
		pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 2 * 0.8);

	pIndicators->stopLoss = pIndicators->atr_euro_range * 1.1;
	pIndicators->takePrice = max(0.3, pIndicators->atr_euro_range * 0.35);

	*floatingTP = pIndicators->takePrice;

	logWarning("System InstanceID = %d, BarTime = %s, pDailyPredictATR=%lf, pDailyMaxATR= %lf, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, pBase_Indicators->pDailyMaxATR, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

	/* Handle order modification during 1:00-1:15 time window */
	if (timeInfo1->tm_hour == 1 && timeInfo1->tm_min >= 0 && timeInfo1->tm_min <= 15 && oldestOpenOrderIndex >= 0)
	{
		if (side == BUY)
		{
			if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice > pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
			}
		}

		if (side == SELL)
		{
			if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
			}
		}
	}

	/* Handle add position logic */
	if (isAddPosition == TRUE)
	{
		pIndicators->tradeMode = 2;
		pIndicators->risk = 0.5;

		if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY)
		{
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			addPositionBaseLine = pBase_Indicators->dailyS1;
			if ((preLow < addPositionBaseLine && preClose > addPositionBaseLine) &&
				pIndicators->entryPrice - pIndicators->stopLoss > pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
				pIndicators->entryPrice <= pBase_Indicators->weeklyR2 &&
				!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			{
				pIndicators->executionTrend = 1;
				pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
				pIndicators->entrySignal = 1;
				*shouldSkip = TRUE;  /* Entry signal set, caller should exit early */
				return SUCCESS;
			}
		}

		if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL)
		{
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			addPositionBaseLine = pBase_Indicators->dailyR1;
			if ((preHigh > addPositionBaseLine && preClose < addPositionBaseLine) &&
				pIndicators->entryPrice + pIndicators->stopLoss < pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
				pIndicators->entryPrice >= pBase_Indicators->weeklyS2 &&
				!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			{
				pIndicators->executionTrend = -1;
				pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
				pIndicators->entrySignal = -1;
				*shouldSkip = TRUE;  /* Entry signal set, caller should exit early */
				return SUCCESS;
			}
		}
	}

	/* Apply GBPJPY trading filter */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
		isSameDayOrder == FALSE &&
		GBPJPY_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
	{
		*shouldSkip = TRUE;  /* Filter blocked trading, caller should exit early */
		return SUCCESS;
	}

	return SUCCESS;
}

/**
 * Setup XAUUSD entry signal for multiple day trading strategies.
 * 
 * Handles XAUUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window
 * - Handles add position logic for long-term trends
 * - Sets endHour for trading window
 * - Applies commodity trading filter
 * - Sets take profit mode
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param oldestOpenOrderIndex Index of oldest open order
 * @param side Order side (BUY, SELL, or NONE)
 * @param isAddPosition TRUE if adding position, FALSE otherwise
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param shouldFilter Whether to apply ATR-based filtering
 * @param preLow Previous bar low price
 * @param preHigh Previous bar high price
 * @param preClose Previous bar close price
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param takeProfitMode Output parameter: take profit mode (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (entry signal set or filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupXAUUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int oldestOpenOrderIndex, OrderType side, BOOL isAddPosition, BOOL isSameDayOrder, BOOL shouldFilter, double preLow, double preHigh, double preClose, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip)
{
	double addPositionBaseLine;
	
	/* Initialize shouldSkip to FALSE (continue processing by default) */
	*shouldSkip = FALSE;

	pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3);
	pIndicators->stopLoss = pIndicators->atr_euro_range * 0.93;
	pIndicators->takePrice = max(3, pIndicators->stopLoss * 0.4);

	*floatingTP = pIndicators->takePrice;
	logInfo("System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

	/* Handle order modification during 1:00-1:15 time window */
	if (timeInfo1->tm_hour == 1 && timeInfo1->tm_min >= 0 && timeInfo1->tm_min <= 15 && oldestOpenOrderIndex >= 0)
	{
		if (side == BUY)
		{
			if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice > pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice < 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice + pIndicators->stopLoss;
			}
		}

		if (side == SELL)
		{
			if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss &&
				pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice;
			}

			if (pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[oldestOpenOrderIndex].openPrice - pIndicators->stopLoss;
			}
		}
	}

	/* Handle add position logic */
	if (isAddPosition == TRUE)
	{
		pIndicators->tradeMode = 2;
		pIndicators->risk = 0.5;

		if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY)
		{
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			addPositionBaseLine = pBase_Indicators->dailyPivot;
			pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
			if ((preLow < addPositionBaseLine && preClose > addPositionBaseLine
				|| (timeInfo1->tm_hour == 1 && timeInfo1->tm_min < 5)) &&
				pIndicators->entryPrice - pIndicators->stopLoss > pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
				!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			{
				pIndicators->executionTrend = 1;
				pIndicators->entrySignal = 1;
				*shouldSkip = TRUE;  /* Entry signal set, caller should exit early */
				return SUCCESS;
			}
		}

		if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL)
		{
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			addPositionBaseLine = pBase_Indicators->dailyPivot;
			pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
			if ((preHigh > addPositionBaseLine && preClose < addPositionBaseLine
				|| (timeInfo1->tm_hour == 1 && timeInfo1->tm_min < 5)) &&
				pIndicators->entryPrice + pIndicators->stopLoss < pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
				!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			{
				pIndicators->executionTrend = -1;
				pIndicators->entrySignal = -1;
				*shouldSkip = TRUE;  /* Entry signal set, caller should exit early */
				return SUCCESS;
			}
		}
	}

	pIndicators->endHour = 23;

	/* Apply commodity trading filter (XAUUSD, XAGUSD, etc.) */
	/* If it's a manual takeover order, after entry, no need to filter */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
		isSameDayOrder == FALSE &&
		Commodity_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
	{
		*shouldSkip = TRUE;  /* Filter blocked trading, caller should exit early */
		return SUCCESS;
	}

	*takeProfitMode = 1;

	return SUCCESS;
}

/**
 * Setup XAGUSD entry signal for multiple day trading strategies.
 * 
 * Handles XAGUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window (uses latestOrderIndex)
 * - Applies commodity trading filter
 * - Sets take profit mode
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param shouldFilter Whether to apply ATR-based filtering
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param takeProfitMode Output parameter: take profit mode (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupXAGUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, BOOL shouldFilter, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip)
{
	/* Initialize shouldSkip to FALSE (continue processing by default) */
	*shouldSkip = FALSE;

	pIndicators->atr_euro_range = (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3;
	pIndicators->stopLoss = pIndicators->atr_euro_range * 0.93;
	pIndicators->takePrice = pIndicators->stopLoss * 0.4;

	*floatingTP = pIndicators->takePrice;
	logInfo("System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

	/* Handle order modification during 1:00-1:15 time window */
	if (timeInfo1->tm_hour == 1 && timeInfo1->tm_min >= 0 && timeInfo1->tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		if (side == BUY)
		{
			if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
			}
		}

		if (side == SELL)
		{
			if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss &&
				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}

			if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
			}
		}
	}

	/* Apply commodity trading filter (XAUUSD, XAGUSD, etc.) */
	/* If it's a manual takeover order, after entry, no need to filter */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
		isSameDayOrder == FALSE &&
		Commodity_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
	{
		*shouldSkip = TRUE;  /* Filter blocked trading, caller should exit early */
		return SUCCESS;
	}

	*takeProfitMode = 1;

	return SUCCESS;
}

/**
 * Setup BTCUSD/ETHUSD entry signal for multiple day trading strategies.
 * 
 * Handles BTCUSD/ETHUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window (uses latestOrderIndex)
 * - Applies BTCUSD/ETHUSD trading filter
 * - Sets take profit mode
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param shouldFilter Whether to apply ATR-based filtering
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param takeProfitMode Output parameter: take profit mode (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupCryptoEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, BOOL shouldFilter, struct tm* timeInfo1, const char* timeString, double* floatingTP, int* takeProfitMode, BOOL* shouldSkip)
{
	/* Initialize shouldSkip to FALSE (continue processing by default) */
	*shouldSkip = FALSE;

	pIndicators->atr_euro_range = (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 3;
	pIndicators->stopLoss = pIndicators->atr_euro_range * 0.93;
	pIndicators->takePrice = pIndicators->stopLoss * 0.4;

	*floatingTP = pIndicators->takePrice;
	logWarning("System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

	/* Handle order modification during 1:00-1:15 time window */
	if (timeInfo1->tm_hour == 1 && timeInfo1->tm_min >= 0 && timeInfo1->tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		if (side == BUY)
		{
			if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
			}
		}

		if (side == SELL)
		{
			if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss &&
				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}

			if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
			}
		}
	}

	/* Apply BTCUSD/ETHUSD trading filter */
	/* If it's a manual takeover order, after entry, no need to filter */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
		isSameDayOrder == FALSE &&
		BTCUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators, shouldFilter) == FALSE)
	{
		*shouldSkip = TRUE;  /* Filter blocked trading, caller should exit early */
		return SUCCESS;
	}

	*takeProfitMode = 1;

	return SUCCESS;
}

/**
 * Setup GBPUSD entry signal for multiple day trading strategies.
 * 
 * Handles GBPUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window with 3-tier stop loss (uses latestOrderIndex)
 * - Applies GBPUSD trading filter
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param executionTrend Current execution trend (-1, 0, or 1)
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupGBPUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int executionTrend, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, struct tm* timeInfo1, const char* timeString, double* floatingTP, BOOL* shouldSkip)
{
	/* Initialize shouldSkip to FALSE (continue processing by default) */
	*shouldSkip = FALSE;

	/* Adjust range: increase range for better filtering */
	if ((int)parameter(AUTOBBS_RANGE) == 1 && executionTrend != 0)
		pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 2 * 0.8);

	pIndicators->stopLoss = pIndicators->atr_euro_range * 1.1;
	pIndicators->takePrice = max(0.003, pIndicators->atr_euro_range * 0.35);

	*floatingTP = 0;

	logWarning("System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

	/* Handle order modification during 1:00-1:15 time window with 3-tier stop loss */
	if (timeInfo1->tm_hour == 1 && timeInfo1->tm_min >= 0 && timeInfo1->tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		if (side == BUY)
		{
			if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 3 * pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 4 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + 2 * pIndicators->stopLoss;
			}
		}

		if (side == SELL)
		{
			if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}
			else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss &&
				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
			}
			else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss &&
				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - 2 * pIndicators->stopLoss;
			}
		}
	}

	/* Apply GBPUSD trading filter */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
		GBPUSD_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
	{
		*shouldSkip = TRUE;  /* Filter blocked trading, caller should exit early */
		return SUCCESS;
	}

	return SUCCESS;
}

/**
 * Setup AUDUSD entry signal for multiple day trading strategies.
 * 
 * Handles AUDUSD-specific entry signal logic:
 * - Calculates stopLoss and takePrice based on atr_euro_range
 * - Handles order modification during 1:00-1:15 time window with 3-tier stop loss (uses latestOrderIndex)
 * - Applies GBPUSD trading filter (shared with GBPUSD)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param latestOrderIndex Index of latest order (used instead of oldestOpenOrderIndex)
 * @param side Order side (BUY, SELL, or NONE)
 * @param isSameDayOrder TRUE if order was opened same day, FALSE otherwise
 * @param timeInfo1 Current time information structure
 * @param timeString Current time string for logging
 * @param floatingTP Output parameter: floating take profit level (set by function)
 * @param shouldSkip Output parameter: TRUE if caller should exit early (filter blocked), FALSE to continue
 * @return SUCCESS on success
 */
AsirikuyReturnCode setupAUDUSDEntrySignal_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int latestOrderIndex, OrderType side, BOOL isSameDayOrder, struct tm* timeInfo1, const char* timeString, double* floatingTP, BOOL* shouldSkip)
{
	/* Initialize shouldSkip to FALSE (continue processing by default) */
	*shouldSkip = FALSE;

	/* Adjust range: increase range for better filtering */
	/* Note: executionTrend check is commented out in original code */
	pIndicators->atr_euro_range = max((double)parameter(AUTOBBS_IS_ATREURO_RANGE), (pBase_Indicators->pDailyPredictATR + pBase_Indicators->pDailyMaxATR) / 2 * 0.8);

	pIndicators->stopLoss = pIndicators->atr_euro_range * 1.1;
	pIndicators->takePrice = max(0.0015, pIndicators->atr_euro_range * 0.35);

	*floatingTP = 0;

	logWarning("System InstanceID = %d, BarTime = %s, atr_euro_range = %lf, stopLoss = %lf, takePrice =%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->atr_euro_range, pIndicators->stopLoss, pIndicators->takePrice);

	/* Handle order modification during 1:00-1:15 time window with 3-tier stop loss */
	if (timeInfo1->tm_hour == 1 && timeInfo1->tm_min >= 0 && timeInfo1->tm_min <= 15 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
	{
		if (side == BUY)
		{
			if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice > pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 2 * pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 3 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + pIndicators->stopLoss;
			}
			else if (pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice >= 3 * pIndicators->stopLoss &&
				pParams->bidAsk.ask[0] - pParams->orderInfo[latestOrderIndex].openPrice < 4 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice + 2 * pIndicators->stopLoss;
			}
		}

		if (side == SELL)
		{
			if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice;
			}
			else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss &&
				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - pIndicators->stopLoss;
			}
			else if (pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->stopLoss &&
				pParams->orderInfo[latestOrderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->stopLoss)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = pParams->orderInfo[latestOrderIndex].openPrice - 2 * pIndicators->stopLoss;
			}
		}
	}

	/* Apply GBPUSD trading filter (shared with AUDUSD) */
	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 &&
		GBPUSD_MultipleDays_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
	{
		*shouldSkip = TRUE;  /* Filter blocked trading, caller should exit early */
		return SUCCESS;
	}

	return SUCCESS;
}

