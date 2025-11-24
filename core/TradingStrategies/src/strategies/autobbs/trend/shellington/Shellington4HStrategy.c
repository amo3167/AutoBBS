/*
 * Shellington 4H Strategy Module
 * 
 * Provides 4-hour timeframe swing trading strategy using Shellington indicator
 * (4H MA trend + BBS) for entry signals. This strategy uses moving averages on the
 * 4-hour timeframe combined with BBS (Bollinger Bands System) to determine trend
 * direction and entry points.
 * 
 * Features:
 * - Symbol-specific configurations (take price, win times, range settings)
 * - Weekly ATR control for risk management
 * - Weekly trend filtering
 * - Range-based entry filtering
 * - Support for multiple symbols: BTCUSD, ETHUSD, XAUUSD, XAUAUD, GBPJPY, GBPAUD,
 *   AUDUSD, AUDNZD, US500USD, NAS100USD, USTECUSD, XPDUSD, XAGUSD
 * 
 * Strategy Mode: AUTOBBS_TREND_MODE=30
 * Split Trade Mode: 27 (ATR mode)
 * TP Mode: 3 (daily ATR)
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "InstanceStates.h"
#include "strategies/autobbs/trend/shellington/Shellington4HStrategy.h"

// Define min/max macros for compatibility (if not already defined)
#ifndef min
#define min(a, b) fmin(a, b)
#endif
#ifndef max
#define max(a, b) fmax(a, b)
#endif

/**
 * @brief Executes 4H Shellington strategy based on 4H MA trend and BBS indicators.
 * 
 * This function implements a swing trading strategy that uses:
 * - 4-hour moving average trend (MA50/MA200)
 * - BBS (Bollinger Bands System) for entry confirmation
 * - Symbol-specific take profit and risk management
 * - Weekly ATR control and trend filtering
 * - Range-based entry filtering
 * 
 * Algorithm:
 * 1. Determine symbol-specific parameters (take price, win times, range settings)
 * 2. Calculate 4H MA trend using getMATrend()
 * 3. Enter BUY when: trend_4H == 1 && bbsTrend_4H == 1 && range conditions met
 * 4. Enter SELL when: trend_4H == -1 && bbsTrend_4H == -1 && range conditions met
 * 5. Apply weekly ATR and trend filters
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators to populate.
 * @param pBase_Indicators Base indicators containing trend and ATR data.
 * @return SUCCESS on success.
 */
AsirikuyReturnCode workoutExecutionTrend_4H_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, closeTimeInfo;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;

	int orderIndex = -1;
	int execution_tf, close_index_rate = -1, diff4Hours, diffDays, diffWeeks;

	int level = 0;
	BOOL isEnableWeeklyATRControl = TRUE;	
	BOOL isEnableWeeklyTrend = FALSE;
	int startHour = 0;
	int buyWonTimes = 0, sellWonTimes = 0;

	double atr5 = iAtr(B_DAILY_RATES, 5, 1);

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;

	int sameSideWonTradesInCurrentTrend;

	double rangeHigh = 0.0, rangeLow = 0.0;

	BOOL isEnableRange = TRUE;
	int range = 30;
	double preRangeClose;

	int turingIndexMA = -1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	execution_tf = (int)pParams->settings[TIMEFRAME];

	pIndicators->takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	pIndicators->takePrice = min(pIndicators->takePrice, pBase_Indicators->dailyATR);
	
	pIndicators->riskCap = 0;

	// Symbol-specific configurations
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//isEnableWeeklyATRControl = TRUE;
		//isEnableWeeklyTrend = TRUE;
		buyWonTimes = 5;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 5;

		pIndicators->riskCap = 2;

		isEnableRange = TRUE;
		range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XAUAUD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 3;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;

		startHour = 1;

		//pIndicators->riskCap = 2;

		//isEnableRange = TRUE;
		//range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 2;
		sellWonTimes = 2;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		buyWonTimes = 2;
		sellWonTimes = 2;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;

		//pIndicators->riskCap = 2;

		//isEnableRange = TRUE;
		//range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		//isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		//pIndicators->takePrice = 0;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;

		isEnableRange = TRUE;
		range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 2;
		sellWonTimes = 1;

		//pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;

		//if (strcmp(pParams->accountInfo.brokerName, "Pepperstone Group Limited") == 0)
		pIndicators->minLotSize = 0.1;	

		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "NAS100USD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
		buyWonTimes = 3;
		sellWonTimes = 1;

		//pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;
		//if (strcmp(pParams->accountInfo.brokerName, "Pepperstone Group Limited") == 0)
		pIndicators->minLotSize = 0.1;

		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "USTECUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
		buyWonTimes = 3;
		sellWonTimes = 1;

		pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;
		//if (strcmp(pParams->accountInfo.brokerName, "Pepperstone Group Limited") == 0)
		//pIndicators->minLotSize = 0.1;

		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 3;
		sellWonTimes = 1;

		isEnableRange = TRUE;
		range = 60;
		startHour = 1;
		//pIndicators->minLotSize = 0.01;
		//pIndicators->isEnableSellMinLotSize = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 2;
		sellWonTimes = 1;

		isEnableRange = TRUE;
		range = 60;
		startHour = 1;
		//pIndicators->minLotSize = 0.01;
		//pIndicators->isEnableSellMinLotSize = TRUE;
	}

	iSRLevels(pParams, pBase_Indicators, B_FOURHOURLY_RATES, shift1Index_4H - 1, range, &rangeHigh, &rangeLow);
	preRangeClose = iClose(B_FOURHOURLY_RATES, 1);

	
	//4H filter - only enter at 4-hour boundaries
	if ((timeInfo1.tm_hour - startHour) % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 27;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		if (trend_MA > 0 )
			trend_4H = 1;
		else if (trend_MA < 0)
			trend_4H = -1;
		// Enter order on key support/resistance levels

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
		

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			safe_gmtime(&closeTimeInfo, pParams->orderInfo[orderIndex].closeTime);

			//skip weekend, consider cross a new year.
			diff4Hours = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 4);
			diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 24);
			diffWeeks = (timeInfo1.tm_wday + 1 + diffDays) / 7;

			close_index_rate = shift1Index_4H - (diff4Hours - diffWeeks * 2 *6);

			logWarning("System InstanceID = %d, BarTime = %s,diff4Hours=%d,diffDays=%d,diffWeeks=%d,orderIndex=%d,close_index_rate=%d,bbsIndex_excution=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, diff4Hours, diffDays, diffWeeks, orderIndex, close_index_rate, pIndicators->bbsIndex_4H);
		}

		
		if (trend_4H == 1 //&& pBase_Indicators->weekly3RulesTrend == UP
			)
		{
			if (pIndicators->bbsTrend_4H == 1)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];

				pIndicators->stopLossPrice = min(pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1) - pBase_Indicators->pDailyATR * 0.5);
				//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pDailyATR * 1.5);

				//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
				//{
				//	if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > pIndicators->stopLoss )
				//	{	
				//		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				//	}					
				//}

				//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

				sameSideWonTradesInCurrentTrend = getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY);
				turingIndexMA = iTrendMA_LookBack(pParams, pBase_Indicators, B_FOURHOURLY_RATES, 1);

				logWarning("System InstanceID = %d, BarTime = %s,stopLossPrice=%lf,bbsStopPrice_4H=%lf,MA200 = %lf,MA50=%lf,turingIndexMA=%d,preRangeClose=%lf,rangeHigh=%lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->stopLossPrice, pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1), iMA(3, B_FOURHOURLY_RATES, 50, 1), turingIndexMA, preRangeClose, rangeHigh);

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					//&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& ((orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL) || pIndicators->bbsIndex_4H >= close_index_rate)
					&& (isEnableRange == FALSE || preRangeClose > rangeHigh 
					|| turingIndexMA >= range
					)
					//&& getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < buyWonTimes
					//&& (isEnableWeeklyTrend == FALSE || pBase_Indicators->weeklyTrend_Phase != RANGE)
					)
				{

					pIndicators->entrySignal = 1;

					if (pIndicators->entrySignal != 0 &&
						sameSideWonTradesInCurrentTrend >= buyWonTimes
						)
					{
						sprintf(pIndicators->status, "sameSideWonTradesInCurrentTrend %d is greater than buyWonTimes %d",
							sameSideWonTradesInCurrentTrend, buyWonTimes);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

				}				
				
			}
			pIndicators->exitSignal = EXIT_SELL;
			//else
			//	pIndicators->exitSignal = EXIT_BUY;

		}

		if (trend_4H == -1 //&& pBase_Indicators->weekly3RulesTrend == DOWN
			)
		{
			
			if (pIndicators->bbsTrend_4H == -1)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				if (pIndicators->isEnableSellMinLotSize == TRUE)
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->pDailyATR *0.1;
				else
					pIndicators->stopLossPrice = max(pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1) + pBase_Indicators->pDailyATR * 0.5);
				//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pDailyATR * 1.5);

				//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
				//{

				//	if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss )
				//	{
				//		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				//	}

				//}

				sameSideWonTradesInCurrentTrend = getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL);
				turingIndexMA = iTrendMA_LookBack(pParams, pBase_Indicators, B_FOURHOURLY_RATES, -1);

				logWarning("System InstanceID = %d, BarTime = %s,stopLossPrice=%lf,bbsStopPrice_4H=%lf,MA200 = %lf,MA50=%lf,turingIndexMA=%d,preRangeClose=%lf,rangeLow=%lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->stopLossPrice, pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1), iMA(3, B_FOURHOURLY_RATES, 50, 1), turingIndexMA, preRangeClose, rangeLow );

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					//&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& ((orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY) || pIndicators->bbsIndex_4H >= close_index_rate)	
					&& (isEnableRange == FALSE || preRangeClose < rangeLow 
					|| turingIndexMA >= range
					)
					//&& getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < sellWonTimes
					//&& (isEnableWeeklyTrend == FALSE || pBase_Indicators->weeklyTrend_Phase != RANGE)
					)
				{
					pIndicators->entrySignal = -1;	
										

					if (pIndicators->entrySignal != 0 &&
						sameSideWonTradesInCurrentTrend >= sellWonTimes
						)
					{
						sprintf(pIndicators->status, "sameSideWonTradesInCurrentTrend %d is greater than sellWonTimes %d",
							sameSideWonTradesInCurrentTrend, sellWonTimes);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

				}
				
			}
			pIndicators->exitSignal = EXIT_BUY;

		}
		//else
		//	pIndicators->exitSignal = EXIT_SELL;

		if (pIndicators->entrySignal != 0 &&
			isEnableWeeklyATRControl == TRUE &&
			fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) > pBase_Indicators->pWeeklyPredictATR
			)
		{
			sprintf(pIndicators->status, "current week movement %lf is greater than pWeeklyPredictATR %lf",
				fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice), pBase_Indicators->pWeeklyPredictATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal != 0 &&
			isEnableWeeklyTrend == TRUE &&
			pBase_Indicators->weeklyTrend_Phase == RANGE
			)
		{
			sprintf(pIndicators->status, "pBase_Indicators->weeklyTrend_Phase %d is in Range", pBase_Indicators->weeklyTrend_Phase);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}
	}
	return SUCCESS;
}

