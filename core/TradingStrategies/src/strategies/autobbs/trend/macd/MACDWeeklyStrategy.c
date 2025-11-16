/*
 * MACD Weekly Strategy Module
 * 
 * Provides MACD Weekly strategy execution functions.
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
#include "strategies/autobbs/trend/macd/MACDWeeklyStrategy.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"

AsirikuyReturnCode workoutExecutionTrend_MACD_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   weeklyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double fast, slow;
	double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_WEEKLY_RATES, 5, 1);
	/* int index1, index2, index3; */
	double level = 0;
	double volume1, volume2, volume_ma_3;

	currentTime = pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index_Weekly];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart

	if (pBase_Indicators->weeklyTrend_Phase > 0)
		weeklyTrend = 1;
	else if (pBase_Indicators->weeklyTrend_Phase < 0)
		weeklyTrend = -1;
	else
		weeklyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 25;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	//ma50Daily = iMA(3, B_DAILY_RATES, 50, 1);
	//preDailyClose = iClose(B_DAILY_RATES, 1);

	if (strstr(pParams->tradeSymbol, "XAU") != NULL)
		level = 10; // XAUUSD
	else if (strstr(pParams->tradeSymbol, "JPY") != NULL)
		level = 0; //GBPJPY
	else
		level = 0.0005; //EURUSD

	//if (timeInfo1.tm_hour == 1) // 1:00 start, avoid the first hour
	{
		//Volume indicator....
		// preVolume > MA: current volume > past average volume
		volume1 = iVolume(B_WEEKLY_RATES, 1);
		volume2 = iVolume(B_WEEKLY_RATES, 2);
		volume_ma_3 = iMA(4, B_WEEKLY_RATES, 3, 1);

		//Load MACD
		fast = iMACD(B_WEEKLY_RATES, 5, 10, 5, 0, 1);
		slow = iMACD(B_WEEKLY_RATES, 5, 10, 5, 1, 1);


		preFast = iMACD(B_WEEKLY_RATES, 5, 10, 5, 0, 2);
		preSlow = iMACD(B_WEEKLY_RATES, 5, 10, 5, 1, 2);


		logWarning("System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 1), fast, slow);
		logWarning("System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 2), preFast, preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		//if (iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR)
		//{
		//	logWarning("System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, iAtr(B_WEEKLY_RATES, 1, 0), pBase_Indicators->pWeeklyPredictMaxATR);
		//	return FALSE;
		//}

		if (fast > level
			//&& slow > 0 
			&& weeklyTrend > 0 
			&& fast > slow
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR;
			//pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;


			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			if (//fast > slow && preFast <= preSlow &&				
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//&& atr5 > pIndicators->entryPrice * 0.01 * 0.55
				//&& getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < 4
				&& volume1 > volume2
				&& fast > preFast
				)
			{
				pIndicators->entrySignal = 1;

				//if (pParams->orderInfo[orderIndex].type == BUY &&
				//	pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
				//{
				//	pIndicators->tradeMode = 2;
				//}
			}
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (fast < (level * -1)
			//&& slow < 0 
			&& weeklyTrend < 0
			&& fast < slow
			//			&& fast < preFast			
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR;
			//pIndicators->stopLossPrice = pIndicators->entryPrice + 1.5 * pBase_Indicators->dailyATR;

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//&& atr5 > pIndicators->entryPrice * 0.01 * 0.55
				//&& getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < 4
				&& volume1 > volume2
				&& fast < preFast
				)
			{
				pIndicators->entrySignal = -1;

				//if (pParams->orderInfo[orderIndex].type == SELL 
				//	&&  pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
				//{					
				//	pIndicators->tradeMode = 2;
				//}
			}

			pIndicators->exitSignal = EXIT_BUY;

		}


		// Exit signal from daily chart: enter range

		if (fast > slow && preFast <= preSlow) // Exit SELL		
			pIndicators->exitSignal = EXIT_SELL;

		if (fast < slow && preFast >= preSlow) // Exit SELL		
			pIndicators->exitSignal = EXIT_BUY;

		//if (dailyTrend == 0)		
		//	pIndicators->exitSignal = EXIT_ALL;
	}

	return SUCCESS;
}
