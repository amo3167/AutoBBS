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

#include "Precompiled.h"
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

/**
 * @brief Symbol-specific configuration structure for Shellington 4H strategy.
 * 
 * This structure holds all symbol-specific parameters that control
 * the behavior of the Shellington 4H strategy for different trading instruments.
 */
typedef struct {
	/* Win times control */
	int buyWonTimes;                        /* Maximum buy win times before blocking entry */
	int sellWonTimes;                       /* Maximum sell win times before blocking entry */
	
	/* Take profit configuration */
	double takePriceMultiplier;             /* Multiplier for daily ATR to calculate take price */
	
	/* Risk management */
	int riskCap;                            /* Risk cap value (0 = disabled) */
	
	/* Filter flags */
	BOOL isEnableWeeklyATRControl;         /* Enable weekly ATR control filter */
	BOOL isEnableWeeklyTrend;               /* Enable weekly trend filter */
	BOOL isEnableRange;                     /* Enable range-based entry filtering */
	int range;                              /* Range period for SR levels calculation */
	
	/* Timing configuration */
	int startHour;                          /* Start hour for trading (0 = no restriction) */
	
	/* Lot size configuration */
	double minLotSize;                      /* Minimum lot size (0 = use default) */
	BOOL isEnableSellMinLotSize;           /* Enable minimum lot size for sell orders */
} ShellingtonSymbolConfig;

/**
 * @brief Initializes symbol-specific configuration for Shellington 4H strategy.
 * 
 * This function configures all symbol-specific parameters based on the trading
 * symbol. Each symbol has unique characteristics that require different take profit
 * levels, win times, and filtering logic.
 * 
 * Symbol-Specific Trading Logic:
 * 
 * BTCUSD/ETHUSD (Cryptocurrencies):
 * - High take profit: 5x daily ATR (crypto volatility)
 * - Buy win times: 5 (allow more consecutive wins)
 * - Sell win times: 1 (stricter for shorts)
 * - Risk cap: 2
 * - Range filtering: Enabled with 60-bar period
 * 
 * XAUUSD (Gold):
 * - Take profit: 4x daily ATR
 * - Win times: 1 for both buy and sell (balanced)
 * - Weekly ATR and trend filters enabled
 * - Start hour: 1 (avoid first hour)
 * 
 * XAUAUD (Gold in AUD):
 * - Take profit: 4x daily ATR
 * - Buy win times: 3, Sell win times: 1
 * - Weekly ATR and trend filters enabled
 * - Start hour: 1
 * 
 * GBPJPY (Major Forex):
 * - Take profit: 3x daily ATR
 * - Win times: 2 for both buy and sell
 * - Weekly ATR and trend filters enabled
 * 
 * GBPAUD (Forex):
 * - Take profit: 3x daily ATR
 * - Win times: 2 for both buy and sell
 * - Weekly ATR filter enabled
 * 
 * AUDUSD (Forex):
 * - Take profit: 3x daily ATR
 * - Win times: 1 for both buy and sell
 * - Weekly ATR filter enabled
 * 
 * AUDNZD (Forex):
 * - Take profit: 3x daily ATR
 * - Win times: 1 for both buy and sell
 * - Weekly trend filter enabled
 * - Range filtering: Enabled with 60-bar period
 * 
 * US500USD (Index):
 * - Take profit: 3x daily ATR
 * - Buy win times: 2, Sell win times: 1
 * - Weekly ATR filter enabled
 * - Minimum lot size: 0.1
 * - Sell min lot size enabled
 * - Start hour: 1
 * 
 * NAS100USD (Index):
 * - Take profit: 4x daily ATR
 * - Buy win times: 3, Sell win times: 1
 * - Weekly ATR filter enabled
 * - Minimum lot size: 0.1
 * - Sell min lot size enabled
 * - Start hour: 1
 * 
 * USTECUSD (Index):
 * - Take profit: 4x daily ATR
 * - Buy win times: 3, Sell win times: 1
 * - Weekly ATR filter enabled
 * - Minimum lot size: 1.0
 * - Sell min lot size enabled
 * - Start hour: 1
 * 
 * XPDUSD (Palladium):
 * - Take profit: 3x daily ATR
 * - Buy win times: 3, Sell win times: 1
 * - Weekly ATR filter enabled
 * - Range filtering: Enabled with 60-bar period
 * - Start hour: 1
 * 
 * XAGUSD (Silver):
 * - Take profit: 3x daily ATR
 * - Buy win times: 2, Sell win times: 1
 * - Weekly ATR filter enabled
 * - Range filtering: Enabled with 60-bar period
 * - Start hour: 1
 * 
 * @param pConfig Configuration structure to populate
 * @param pParams Strategy parameters containing symbol information
 * @param pIndicators Strategy indicators (for setting lot sizes, risk cap, take price)
 * @param pBase_Indicators Base indicators (for ATR calculations)
 * 
 * @note Common Parameters:
 * The following parameters can be set in config files to override symbol-specific defaults:
 * - AUTOBBS_SHELLINGTON_TP_MULTIPLIER: Take profit multiplier for daily ATR (e.g., 3.0)
 * - AUTOBBS_SHELLINGTON_BUY_WON_TIMES: Maximum buy win times before blocking entry
 * - AUTOBBS_SHELLINGTON_SELL_WON_TIMES: Maximum sell win times before blocking entry
 * 
 * These parameters work like AUTOBBS_RISK_CAP: if set in config file, they override
 * symbol-specific defaults for symbols that use common values (e.g., US500USD, XAGUSD).
 * If not set (0 or not in config), symbol-specific defaults are used.
 * 
 * Note: isEnableWeeklyATRControl defaults to TRUE for all symbols and is not configurable
 * via parameters since all symbols use the same value.
 */
static void initializeSymbolConfig(ShellingtonSymbolConfig* pConfig, StrategyParams* pParams, 
	Indicators* pIndicators, Base_Indicators* pBase_Indicators)
{
	/* Initialize with default values */
	memset(pConfig, 0, sizeof(ShellingtonSymbolConfig));
	
	/* Read common parameters (can be overridden by symbol-specific settings) */
	/* These parameters work like AUTOBBS_RISK_CAP: if set in config file, they override symbol-specific defaults */
	double paramTPMultiplier = parameter(AUTOBBS_SHELLINGTON_TP_MULTIPLIER);
	double paramBuyWonTimes = parameter(AUTOBBS_SHELLINGTON_BUY_WON_TIMES);
	double paramSellWonTimes = parameter(AUTOBBS_SHELLINGTON_SELL_WON_TIMES);
	
	/* Initialize with default values */
	pConfig->buyWonTimes = 0;
	pConfig->sellWonTimes = 0;
	pConfig->takePriceMultiplier = 0.0;  /* 0 = use default calculation */
	pConfig->riskCap = 0;
	pConfig->isEnableWeeklyATRControl = TRUE;  /* Default: TRUE */
	pConfig->isEnableWeeklyTrend = FALSE;
	pConfig->isEnableRange = TRUE;
	pConfig->range = 30;
	pConfig->startHour = 0;
	pConfig->minLotSize = 0.0;  /* 0 = use default */
	pConfig->isEnableSellMinLotSize = FALSE;

	/* Configure based on symbol */
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		/* Cryptocurrency configuration */
		pConfig->buyWonTimes = 5;
		pConfig->sellWonTimes = 1;
		pConfig->takePriceMultiplier = 5.0;
		pConfig->riskCap = 2;
		pConfig->isEnableRange = TRUE;
		pConfig->range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		/* Gold configuration */
		pConfig->takePriceMultiplier = (paramTPMultiplier > 0.0) ? paramTPMultiplier : 4.0;
		pConfig->buyWonTimes = (paramBuyWonTimes > 0.0) ? (int)paramBuyWonTimes : 1;
		pConfig->sellWonTimes = (paramSellWonTimes > 0.0) ? (int)paramSellWonTimes : 1;
		pConfig->startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XAUAUD") != NULL)
	{
		/* Gold in AUD configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->isEnableWeeklyTrend = TRUE;
		pConfig->buyWonTimes = 3;
		pConfig->sellWonTimes = 1;
		pConfig->takePriceMultiplier = 4.0;
		pConfig->startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		/* GBPJPY configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->isEnableWeeklyTrend = TRUE;
		pConfig->buyWonTimes = 2;
		pConfig->sellWonTimes = 2;
		pConfig->takePriceMultiplier = 3.0;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		/* GBPAUD configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->buyWonTimes = 2;
		pConfig->sellWonTimes = 2;
		pConfig->takePriceMultiplier = 3.0;
	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		/* AUDUSD configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->buyWonTimes = 1;
		pConfig->sellWonTimes = 1;
		pConfig->takePriceMultiplier = 3.0;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		/* AUDNZD configuration */
		pConfig->takePriceMultiplier = 3.0;
		pConfig->isEnableWeeklyTrend = TRUE;
		pConfig->buyWonTimes = 1;
		pConfig->sellWonTimes = 1;
		pConfig->isEnableRange = TRUE;
		pConfig->range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		/* US500USD configuration */
		/* Use parameters if set, otherwise use symbol-specific defaults */
		/* These 4 parameters are common across multiple symbols and can be configured via AUTOBBS_SHELLINGTON_* parameters */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->takePriceMultiplier = (paramTPMultiplier > 0.0) ? paramTPMultiplier : 3.0;
		pConfig->buyWonTimes = (paramBuyWonTimes > 0.0) ? (int)paramBuyWonTimes : 2;
		pConfig->sellWonTimes = (paramSellWonTimes > 0.0) ? (int)paramSellWonTimes : 1;
		pConfig->isEnableSellMinLotSize = TRUE;
		pConfig->minLotSize = 0.1;
		pConfig->startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "NAS100USD") != NULL)
	{
		/* NAS100USD configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->takePriceMultiplier = 4.0;
		pConfig->buyWonTimes = 3;
		pConfig->sellWonTimes = 1;
		pConfig->isEnableSellMinLotSize = TRUE;
		pConfig->minLotSize = 0.1;
		pConfig->startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "USTECUSD") != NULL)
	{
		/* USTECUSD configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->takePriceMultiplier = 4.0;
		pConfig->buyWonTimes = 3;
		pConfig->sellWonTimes = 1;
		pConfig->minLotSize = 1.0;
		pConfig->isEnableSellMinLotSize = TRUE;
		pConfig->startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		/* XPDUSD configuration */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->takePriceMultiplier = 3.0;
		pConfig->buyWonTimes = 3;
		pConfig->sellWonTimes = 1;
		pConfig->isEnableRange = TRUE;
		pConfig->range = 60;
		pConfig->startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		/* XAGUSD configuration */
		/* Use parameters if set, otherwise use symbol-specific defaults */
		/* These 4 parameters are common across multiple symbols and can be configured via AUTOBBS_SHELLINGTON_* parameters */
		/* isEnableWeeklyATRControl uses default (TRUE) - can be overridden by parameter */
		pConfig->takePriceMultiplier = (paramTPMultiplier > 0.0) ? paramTPMultiplier : 3.0;
		pConfig->buyWonTimes = (paramBuyWonTimes > 0.0) ? (int)paramBuyWonTimes : 2;
		pConfig->sellWonTimes = (paramSellWonTimes > 0.0) ? (int)paramSellWonTimes : 1;
		pConfig->isEnableRange = TRUE;
		pConfig->range = 60;
		pConfig->startHour = 1;
	}
	
	/* Apply configuration to indicators */
	if (pConfig->takePriceMultiplier > 0.0)
	{
		pIndicators->takePrice = pBase_Indicators->dailyATR * pConfig->takePriceMultiplier;
	}
	
	if (pConfig->riskCap > 0)
	{
		pIndicators->riskCap = pConfig->riskCap;
	}
	
	if (pConfig->minLotSize > 0.0)
	{
		pIndicators->minLotSize = pConfig->minLotSize;
	}
	
	if (pConfig->isEnableSellMinLotSize == TRUE)
	{
		pIndicators->isEnableSellMinLotSize = TRUE;
	}
}

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

	int sameSideWonTradesInCurrentTrend;

	double rangeHigh = 0.0, rangeLow = 0.0;
	double preRangeClose;

	int turingIndexMA = -1;

	/* Symbol-specific configuration */
	ShellingtonSymbolConfig config;

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

	/* Default take price calculation */
	pIndicators->takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	pIndicators->takePrice = min(pIndicators->takePrice, pBase_Indicators->dailyATR);
	
	pIndicators->riskCap = 0;

	/* Initialize symbol-specific configuration */
	initializeSymbolConfig(&config, pParams, pIndicators, pBase_Indicators);
	
	/* Extract configuration values for use in main logic */
	int buyWonTimes = config.buyWonTimes;
	int sellWonTimes = config.sellWonTimes;
	BOOL isEnableWeeklyATRControl = config.isEnableWeeklyATRControl;
	BOOL isEnableWeeklyTrend = config.isEnableWeeklyTrend;
	BOOL isEnableRange = config.isEnableRange;
	int range = config.range;
	int startHour = config.startHour;

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

