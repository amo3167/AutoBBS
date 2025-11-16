/*
 * Ichimoko Daily Strategy Module
 * 
 * Provides Ichimoko Daily strategy execution functions.
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
#include "strategies/autobbs/trend/ichimoko/IchimokoDailyStrategy.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	int openOrderCount = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	//Long term: tradeMode = 1
	//Short term: tradeMode = 0
	pIndicators->tradeMode = 1;

	if (pIndicators->tradeMode == 1)
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
		strategyVolRisk = pIndicators->strategyMaxRisk;
	}
	else
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
	}

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preDailyClose1 = iClose(B_DAILY_RATES, 2);
	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday)
			dailyOnly = 0;
	}

	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= 1) // 1:00 start, avoid the first hour
	{


		// Calculate daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		logInfo("System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

		logInfo("System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

		logInfo("System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
			preDailyClose > daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			
			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO: Need to adjust
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose > preDailyClose1
				&& preDailyClose > daily_baseline_short
				&& daily_baseline_short > daily_baseline
				&& preWeeklyClose > weekly_baseline
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
			preDailyClose < daily_baseline
			) // Sell
		{

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose < preDailyClose1
				&& preDailyClose < daily_baseline_short
				&& daily_baseline_short < daily_baseline
				&& preWeeklyClose < weekly_baseline
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = -1;
			}
			pIndicators->exitSignal = EXIT_BUY;

		}


	}

	openOrderCount = getOrderCountEasy();

	if (pIndicators->entrySignal != 0 && openOrderCount >= 3)
	{
		logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d ",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

		pIndicators->entrySignal = 0;
	}
	
	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	//targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	//// when floating profit is too high, fe 10%
	//if (pIndicators->riskPNL >targetPNL)
	//	closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shiftPreDayBar = shift1Index - 1;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preWeeklyClose, preWeeklyClose1;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double weekly_baseline = 0.0,weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;
		
	double dailyMA20 = 0.0;
	int openOrderCount = 0;

	double preHist1;
	double fast1;
	double slow1;
	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	int latestOrderIndex = 0;

	double preMonthHigh = 0.0, preMonthLow = 0.0;
	double rangeHigh = 0.0, rangeLow = 0.0;

	BOOL isEnableRange = TRUE;
	int range = 30;
	int orderCount = 3;
	BOOL isProfitManaged = TRUE;
	BOOL isWeeklyBaseLine = TRUE;
	BOOL isExitFromShortBaseLine = FALSE;
	BOOL isMACDBeiLi = TRUE;
	BOOL is3KBreak = FALSE;
	BOOL isMaxLevel = FALSE;
	BOOL isEnableCMFVolume = FALSE;
		
	double maxLevel = 0.0;
	int pre3KTrend;

	double exitBaseLine = 0.0;

	double level = 0.0;

	BOOL isDailyOnly = TRUE;
	BOOL isEnableEntryEOD = FALSE;
	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	//Long term: tradeMode = 1
	//Short term: tradeMode = 0
	pIndicators->tradeMode = 1;

	//latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (pIndicators->tradeMode == 1)
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
		strategyVolRisk = pIndicators->strategyMaxRisk;
	}
	else
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
	}

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preDailyClose1 = iClose(B_DAILY_RATES, 2);

	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday)
			dailyOnly = 0;
	}

	// If previous day close price not exceed 0.2 daily ATR, wait for next day	
	// Calculate daily and weekly baseline
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	pIndicators->daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	pIndicators->daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	logInfo("System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, pIndicators->daily_baseline);

	logInfo("System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, pIndicators->daily_baseline_short);

	logInfo("System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);


	if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableRange = TRUE;
		range = 30;
		isWeeklyBaseLine = FALSE;

		orderCount = 3;

		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;
		if (strcmp(pParams->accountInfo.brokerName, "International Capital Markets Pty. Ltd.") == 0)
			shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		level = 0.1;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = FALSE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;

		pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		shiftPreDayBar = shift1Index;

	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 1;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;

		pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//level = 0.0008;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;

		//pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		isDailyOnly = TRUE;

	}
	else if (strstr(pParams->tradeSymbol, "USDSGD") != NULL)
	{
		//level = 0.0008;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;
				
		pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		isDailyOnly = TRUE;
		
	}
	else if (strstr(pParams->tradeSymbol, "GER30EUR") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = TRUE;
		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;
	}

	// After 23:00, check if need to enter
	//if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	//{		
	//
	//}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, 1);

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, &fast1, &slow1, &preHist1);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;


	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= 1 
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		)
	{

		dailyMA20 = iMA(3, B_DAILY_RATES, 20, 1);

		iTrend3Rules_preDays(pParams, pBase_Indicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);


		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily - 1, range, &rangeHigh, &rangeLow);

		//load pBase_Indicators
		//pBase_Indicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		
		//get the last month high low 
		preMonthHigh = iHigh(B_MONTHLY_RATES, 1);
		preMonthLow = iLow(B_MONTHLY_RATES, 1);

		openOrderCount = getOrderCountEasy();

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
			preDailyClose > pIndicators->daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = pIndicators->daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO: Need to adjust

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose > preDailyClose1
				&& preDailyClose > pIndicators->daily_baseline_short
				&& pIndicators->daily_baseline_short - pIndicators->daily_baseline > 0 * pBase_Indicators->dailyATR
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose > weekly_baseline && (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
				)
				&& (isEnableRange == FALSE || preDailyClose > rangeHigh)
				&& (isMACDBeiLi == FALSE || (fast1 > slow1 && fast1 > level))
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume > 0)
				)
			{
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;


			if (isMACDBeiLi == TRUE
				&& pIndicators->entrySignal != 0
				&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
			{
				logWarning("System InstanceID = %d, BarTime = %s, MACD BeiLi",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				pIndicators->entrySignal = 0;
			}

			//Exit the last trade when break fast line
			if (isExitFromShortBaseLine == TRUE && preDailyClose < exitBaseLine)
			{
				pIndicators->exitSignal = EXIT_BUY;
			}

		}

		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
			preDailyClose < pIndicators->daily_baseline
			) // Sell
		{

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = pIndicators->daily_baseline + pBase_Indicators->dailyATR * 0.25;

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose < preDailyClose1
				&& preDailyClose < pIndicators->daily_baseline_short
				&& pIndicators->daily_baseline - pIndicators->daily_baseline_short  > 0 * pBase_Indicators->dailyATR
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline
				&& (weekly_baseline_short < weekly_baseline || pre3KTrend == DOWN))
				)
				&& (isEnableRange == FALSE || preDailyClose < rangeLow)
				&& (isMACDBeiLi == FALSE || (fast1 < -1 * level && fast1 < slow1))
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume < 0)
				)
			{
				pIndicators->entrySignal = -1;
			}
			pIndicators->exitSignal = EXIT_BUY;

			if (isMACDBeiLi == TRUE
				&& pIndicators->entrySignal != 0
				&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
			{
				logWarning("System InstanceID = %d, BarTime = %s, MACD BeiLi",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				pIndicators->entrySignal = 0;
			}


			if (isExitFromShortBaseLine == TRUE  && preDailyClose > exitBaseLine)
			{
				pIndicators->exitSignal = EXIT_SELL;
			}

		}


		if (pIndicators->entrySignal != 0 && openOrderCount >= orderCount)
		{
			logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d ",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

			pIndicators->entrySignal = 0;
		}

		if (isProfitManaged == TRUE)
		{
			if (pIndicators->entrySignal != 0 && openOrderCount == 0)
			{
				logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d 0.5 risk",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

				pIndicators->risk = 0.5;
			}

			profitManagement_base(pParams, pIndicators, pBase_Indicators);


			targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
			//// when floating profit is too high, fe 10%
			if (pIndicators->riskPNL > targetPNL)
				closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
		}
	}
	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preWeeklyClose, preWeeklyClose1;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	double weeklyMA50 = 0.0;
	double dailyMA20 = 0.0;
	int openOrderCount = 0;

	double preHist1;
	double fast1;
	double slow1;
	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	int latestOrderIndex = 0;

	double preMonthHigh = 0.0, preMonthLow = 0.0;
	double rangeHigh = 0.0, rangeLow = 0.0;

	BOOL isEnableRange = TRUE;
	int range = 30;
	int orderCount = 3;
	BOOL isProfitManaged = TRUE;
	BOOL isWeeklyBaseLine = TRUE;
	BOOL isExitFromShortBaseLine = FALSE;
	BOOL isMACDBeiLi = TRUE;
	BOOL is3KBreak = FALSE;
	BOOL isMaxLevel = FALSE;
	BOOL isEnableCMFVolume = FALSE;

	double cmfVolume = 0.0;
	double maxLevel = 0.0;
	int pre3KTrend;

	double exitBaseLine = 0.0;

	double level = 0.0;

	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	//Long term: tradeMode = 1
	//Short term: tradeMode = 0
	pIndicators->tradeMode = 1;

	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (pIndicators->tradeMode == 1)
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
		strategyVolRisk = pIndicators->strategyMaxRisk;
	}
	else
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
	}

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preDailyClose1 = iClose(B_DAILY_RATES, 2);
	//preDailyHigh1 = iHigh(B_DAILY_RATES, 2);
	//preDailyLow1 = iLow(B_DAILY_RATES, 2);
	//preDailyHigh2 = iHigh(B_DAILY_RATES, 3);
	//preDailyLow2 = iLow(B_DAILY_RATES, 3);

	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday)
			dailyOnly = 0;
	}

	// If previous day close price not exceed 0.2 daily ATR, wait for next day	
	// Calculate daily and weekly baseline
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	logInfo("System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

	logInfo("System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

	logInfo("System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);

	weeklyMA50 = iMA(3, B_WEEKLY_RATES, 50, 1);
	dailyMA20 = iMA(3, B_DAILY_RATES, 20, 1);

	if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableRange = TRUE;
		range = 30;
		isWeeklyBaseLine = FALSE;

		orderCount = 3;

		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		level = 0.1;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = FALSE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;

		daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);
		
	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;

		daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);
	}
	else if (strstr(pParams->tradeSymbol, "USDSGD") != NULL)
	{
		//level = 0.0008;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;

		daily_baseline = iMA(3,B_DAILY_RATES, 50, 1);
	}
	else if (strstr(pParams->tradeSymbol, "GER30EUR") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = TRUE;
		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;				
	}

	cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, 1);

	iTrend3Rules_preDays(pParams, pBase_Indicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);
	

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily - 1, range, &rangeHigh, &rangeLow);

	//load pBase_Indicators
	pBase_Indicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, &fast1, &slow1, &preHist1);

	//get the last month high low 
	preMonthHigh = iHigh(B_MONTHLY_RATES, 1);
	preMonthLow = iLow(B_MONTHLY_RATES, 1);

	openOrderCount = getOrderCountEasy();

	if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
		preDailyClose > daily_baseline
		) // Buy
	{

		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO: Need to adjust
		//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);

		//Option 1:
		if (
			dailyOnly == 1
			&& preDailyClose > preDailyClose1
			&& preDailyClose > daily_baseline_short
			&& daily_baseline_short - daily_baseline > 0 * pBase_Indicators->dailyATR
			//&& preWeeklyClose > weeklyMA50
			&& (isWeeklyBaseLine == FALSE || 
				(preWeeklyClose > weekly_baseline	&&  (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
				)
			&& (isEnableRange == FALSE || preDailyClose > rangeHigh)
			&& (isMACDBeiLi == FALSE || (fast1 > slow1 && fast1 > level))
			&& (isEnableCMFVolume == FALSE || cmfVolume > 0)
			)
		{
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;


		if (isMACDBeiLi == TRUE
			&& pIndicators->entrySignal != 0
			//&& latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].type == BUY
			&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
		{
			logWarning("System InstanceID = %d, BarTime = %s, MACD BeiLi",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			pIndicators->entrySignal = 0;
		}

		//Exit the last trade when break fast line
		if (isExitFromShortBaseLine == TRUE && preDailyClose < exitBaseLine )
		{
			//if (openOrderCount > 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
			//	closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
			pIndicators->exitSignal = EXIT_BUY;
		}

		//if (is3KBreak == TRUE && pre3KTrend == DOWN)
		//{			
		//	pIndicators->exitSignal = EXIT_BUY;
		//}
	}

	if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
		preDailyClose < daily_baseline
		) // Sell
	{

		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];

		pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
		//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);

		//Option 1:
		if (
			dailyOnly == 1
			&& preDailyClose < preDailyClose1
			&& preDailyClose < daily_baseline_short
			&& daily_baseline - daily_baseline_short  > 0 * pBase_Indicators->dailyATR
			//&& preWeeklyClose < weeklyMA50
			&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline 				
				&& (weekly_baseline_short < weekly_baseline ||	pre3KTrend == DOWN))
				)
			&& (isEnableRange == FALSE || preDailyClose < rangeLow)
			&& (isMACDBeiLi == FALSE || (fast1 < -1 * level && fast1 < slow1))
			&& (isEnableCMFVolume == FALSE || cmfVolume < 0)
			)
		{
			pIndicators->entrySignal = -1;
		}
		pIndicators->exitSignal = EXIT_BUY;

		if (isMACDBeiLi == TRUE
			&& pIndicators->entrySignal != 0			
			&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
		{
			logWarning("System InstanceID = %d, BarTime = %s, MACD BeiLi",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			pIndicators->entrySignal = 0;
		}


		if (isExitFromShortBaseLine == TRUE  && preDailyClose > exitBaseLine)
		{
			//if (openOrderCount > 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
			//	closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
			pIndicators->exitSignal = EXIT_SELL;
		}

		//if (is3KBreak == TRUE && pre3KTrend == UP )
		//{
		//	pIndicators->exitSignal = EXIT_SELL;
		//}
	}

	
	if (pIndicators->entrySignal != 0 && openOrderCount >= orderCount)
	{
		logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d ",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

		pIndicators->entrySignal = 0;
	}

	if (isProfitManaged == TRUE)
	{
		if (pIndicators->entrySignal != 0 && openOrderCount == 0)
		{
			logWarning("System InstanceID = %d, BarTime = %s openOrderCount=%d 0.5 risk",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

			pIndicators->risk = 0.5;
		}

		profitManagement_base(pParams, pIndicators, pBase_Indicators);


		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		//// when floating profit is too high, fe 10%
		if (pIndicators->riskPNL > targetPNL)
			closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}
	return SUCCESS;
}
