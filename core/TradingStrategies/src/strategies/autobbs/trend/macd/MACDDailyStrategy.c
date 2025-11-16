/*
 * MACD Daily Strategy Module (New Version)
 * 
 * Provides MACD Daily strategy execution functions.
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
#include "strategies/autobbs/trend/macd/MACDDailyStrategy.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"

AsirikuyReturnCode workoutExecutionTrend_MACD_Daily_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shiftPreDayBar = shift1Index - 1;
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";	
	int orderIndex;
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);	
	double level = 0, histLevel = 0, maxLevel = 0;
	

	double ma20Daily, preDailyClose;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;

	//double cmfVolume = 0.0;
	BOOL isVolumeControl = TRUE;
	BOOL isEnableBeiLi = TRUE;
	BOOL isEnableSlow = TRUE;
	BOOL isEnableATR = TRUE;
	BOOL isEnableCMFVolume = FALSE;
	BOOL isEnableCMFVolumeGap = FALSE;
	BOOL isEnableMaxLevel = FALSE;

	BOOL isVolumeControlRisk = FALSE;
	BOOL isCMFVolumeRisk = FALSE;
	BOOL isCMFVolumeGapRisk = FALSE;
	BOOL isAllVolumeRisk = FALSE;

	BOOL isWeeklyBaseLine = FALSE;

	BOOL isDailyOnly = TRUE;

	BOOL isMACDZeroExit = FALSE;

	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int pre3KTrend;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	//double CMFVolumeGap = 0.0;
	double stopLoss = pBase_Indicators->pDailyMaxATR;
	double maxRisk = 2;

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;

	int truningPointIndex = -1, minPointIndex = -1;	
	double turningPoint, minPoint;

	//double atr5Limit = pParams->bidAsk.ask[0] * 0.01 *0.55;

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

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	ma20Daily = iMA(3, B_DAILY_RATES, 20, startShift);
	preDailyClose = iClose(B_DAILY_RATES, startShift);

	if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
	{

		level = 0.35;
		//maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;
		isWeeklyBaseLine = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = TRUE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = FALSE;

		isEnableEntryEOD = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 10;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		isCMFVolumeGapRisk = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		maxRisk = 2;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

	}
	else if (strstr(pParams->tradeSymbol, "XAUEUR") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 8;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;

		isEnableSlow = FALSE;

		isEnableATR = TRUE;		
		isEnableCMFVolume = TRUE;		

		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 2;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		isMACDZeroExit = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "GBPCHF") != NULL)
	{
		level = 0.002; //GBPJPY
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.5;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "EURJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = TRUE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "CADJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = TRUE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		level = 0.001; //GBPAUD
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		isAllVolumeRisk = TRUE;

		//stopLoss = pBase_Indicators->dailyATR * 1.5;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		level = 0.001; //GBPUSD
		maxLevel = 0.007;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		level = 0.0025; //GBPJPY
		maxLevel = 0;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 12;
		slowMAPeriod = 26;
		signalMAPeriod = 9;
	}
	else
	{
		level = 0; //EURUSD
		maxLevel = 0.005;

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}

	// After 23:00, check if need to enter
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;
		macdLimit = level / 2;

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);


	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= 1
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 start, avoid the first hour
	{

		preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
		iTrend3Rules_preDays(pParams, pBase_Indicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, startShift);

		pIndicators->CMFVolumeGap = getCMFVolumeGap(B_DAILY_RATES, 1, fastMAPeriod, startShift);

		//Volume indicator....
		// preVolume > MA: current volume > past average volume
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		//volume_ma_10 = iMA(4, B_DAILY_RATES, 10, startShift);

		logWarning("System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->cmfVolume, pIndicators->CMFVolumeGap, weekly_baseline, weekly_baseline_short, pIndicators->volume1, pIndicators->volume2);

		//Load MACD
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);

		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;


		logWarning("System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		logWarning("System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift + 1), pIndicators->preFast, pIndicators->preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);



		if (pIndicators->fast > level
			&& (isEnableSlow == FALSE || pIndicators->slow > 0)
			//&& dailyTrend > 0 			
			&& preDailyClose > ma20Daily
			&& pIndicators->fast - pIndicators->slow > macdLimit
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pDailyMaxATR);
			
			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

			if (orderIndex >= 0 && 
				pParams->orderInfo[orderIndex].type == BUY &&
				pParams->orderInfo[orderIndex].isOpen == TRUE &&
				pIndicators->stopLossPrice > pParams->orderInfo[orderIndex].openPrice)
			{				

				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
			}
			
			//pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;


			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			if (//fast > slow && preFast <= preSlow &&				
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast > pIndicators->preFast
				&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
				&& (isVolumeControl == FALSE || pIndicators->volume1 > pIndicators->volume2)
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume > 0)
				&& (isEnableCMFVolumeGap == FALSE || pIndicators->CMFVolumeGap > 0)
				//&& volume1 > volume_ma_10
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose > weekly_baseline && (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
				)
				&& pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) <= 0.2 * pBase_Indicators->dailyATR
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);
				if (timeInfo1.tm_mday != timeInfo2.tm_mday)
				{
					pIndicators->entrySignal = 1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap > 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume > 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap > 0 && pIndicators->cmfVolume >  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					//if (pParams->orderInfo[orderIndex].type == BUY &&
					//	pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
					//{
					//	pIndicators->tradeMode = 2;
					//}

					// If price retreats back, add position
					if (preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
						&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						)
					{
						pIndicators->entrySignal = 0;
					}

					if (isEnableMaxLevel == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& pIndicators->fast > maxLevel
						)
					{
						logWarning("System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint,&minPointIndex,&minPoint))
					{
						logWarning("System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}


			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (pIndicators->fast < (level * -1)
			&& (isEnableSlow == FALSE || pIndicators->slow < 0)
			//&& dailyTrend < 0			
			&& preDailyClose < ma20Daily
			&& pIndicators->slow - pIndicators->fast > macdLimit
			//			&& fast < preFast			
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pDailyMaxATR);

			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

			if (orderIndex >= 0 && 
				pParams->orderInfo[orderIndex].type == SELL &&
				pParams->orderInfo[orderIndex].isOpen == TRUE &&
				pIndicators->stopLossPrice < pParams->orderInfo[orderIndex].openPrice)
			{

				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
			}

			//pIndicators->stopLossPrice = pIndicators->entryPrice + 1.5 * pBase_Indicators->dailyATR;

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast < pIndicators->preFast
				&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
				&& (isVolumeControl == FALSE || pIndicators->volume1 > pIndicators->volume2)
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume < 0)
				&& (isEnableCMFVolumeGap == FALSE || pIndicators->CMFVolumeGap > 0)
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline
				&& (weekly_baseline_short < weekly_baseline || pre3KTrend == DOWN))
				)
				&& iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice <= 0.2 * pBase_Indicators->dailyATR
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);
				if (timeInfo1.tm_mday != timeInfo2.tm_mday)
				{

					pIndicators->entrySignal = -1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 < pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap < 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume < 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap < 0 && pIndicators->cmfVolume <  0 && pIndicators->volume1 < pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					//if (pParams->orderInfo[orderIndex].type == SELL 
					//	&&  pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
					//{					
					//	pIndicators->tradeMode = 2;
					//}

					// If price retreats back, add position
					if (preHist1 < (histLevel*-1) && preHist2 < (histLevel*-1) && preHist3 < (histLevel*-1) && preHist4 < (histLevel*-1) && preHist5 < (histLevel*-1)
						&& fast1 < (level*-1) && fast2 < (level*-1) && fast3 < (level*-1) && fast4 < (level*-1) && fast5 < (level*-1)
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						)
					{
						pIndicators->entrySignal = 0;
					}

					if (isEnableMaxLevel == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& pIndicators->fast < (maxLevel*-1)
						)
					{
						logWarning("System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
					{
						logWarning("System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}
			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = -1;
			//}

			pIndicators->exitSignal = EXIT_BUY;

		}


		// Exit signal from daily chart: enter range
		if (isMACDZeroExit == FALSE)
		{
			if (pIndicators->fast - pIndicators->slow > macdLimit && pIndicators->preFast <= pIndicators->preSlow) // Exit SELL		
				pIndicators->exitSignal = EXIT_SELL;

			if (pIndicators->slow - pIndicators->fast > macdLimit && pIndicators->preFast >= pIndicators->preSlow) // Exit SELL		
				pIndicators->exitSignal = EXIT_BUY;
		}
		else{
			if (pIndicators->fast > 0) // Exit SELL		
				pIndicators->exitSignal = EXIT_SELL;

			if (pIndicators->fast < 0) // Exit SELL		
				pIndicators->exitSignal = EXIT_BUY;
		}
		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == BUY && preDailyClose < ma20Daily)
		//	pIndicators->exitSignal = EXIT_BUY;

		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == SELL && preDailyClose > ma20Daily)
		//	pIndicators->exitSignal = EXIT_SELL;
		//if (dailyTrend == 0)		
		//	pIndicators->exitSignal = EXIT_ALL;
	}


	//profit managgement
	// if profit is > 1 ATR, exit 50%
	// if proift is > 2 ATR, exit 100%

	//if MACD fast > 10 or < -10, exit proit
	//if (fast > maxLevel)
	//	pIndicators->exitSignal = EXIT_BUY;

	//if (fast < maxLevel * -1)
	//	pIndicators->exitSignal = EXIT_SELL;


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shiftPreDayBar = shift1Index_Daily - 2;
	int   dailyTrend; 
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	//double fast, slow;
	//double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	//double ma50Daily, preDailyClose;
	double level = 0, histLevel = 0, maxLevel = 0;
	double nextMACDRange = 0;
	double volume_ma_5;

	double ma20Daily, preDailyClose;
	/* Restored historical MACD component buffers (previous removal caused C2065 errors).
	 * Keep for computations later in function; if some become truly unused, consider pruning with care. */
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;

	int startHour = 1;

	//double cmfVolume = 0.0;
	BOOL isVolumeControl = TRUE;
	BOOL isEnableBeiLi = TRUE;
	BOOL isEnableSlow = TRUE;
	BOOL isEnableATR = TRUE;
	BOOL isEnableCMFVolume = FALSE;
	BOOL isEnableCMFVolumeGap = FALSE;
	BOOL isEnableMaxLevelBuy = FALSE;
	BOOL isEnableMaxLevelSell = FALSE;

	BOOL isVolumeControlRisk = FALSE;
	BOOL isCMFVolumeRisk = FALSE;
	BOOL isCMFVolumeGapRisk = FALSE;
	BOOL isAllVolumeRisk = FALSE;

	BOOL isWeeklyBaseLine = FALSE;

	BOOL isDailyOnly = TRUE;

	BOOL isEnableASI = FALSE;

	BOOL isEnableLate = FALSE;

	BOOL isEnableNextdayBar = FALSE;

	BOOL isEnableNoStopLoss = FALSE;

	BOOL isEnableMaxLevelRiskControl = FALSE;

	BOOL isMACDBeili = FALSE;

	BOOL isEnableMaxLevel = FALSE;
	
	//int oldestOpenOrderIndex = -1;

	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int pre3KTrend;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	//double CMFVolumeGap = 0.0;
	double stopLoss = pBase_Indicators->pDailyMaxATR;
	double maxRisk = 2;

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;
	double asiBull, asiBear;
	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;
	double atrRange;

	//double rangeHigh, rangeLow;
	int range = 10000;

	double highHourlyClosePrice;
	double lowHourlyClosePrice;

	atrRange = 0.01 * 0.55;

	//double atr5Limit = pParams->bidAsk.ask[0] * 0.01 *0.55;

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

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	ma20Daily = iMA(3, B_DAILY_RATES, 20, startShift);
	preDailyClose = iClose(B_DAILY_RATES, startShift);

	dailyBaseLine = ma20Daily;

	//isVolumeControl from AUTOBBS_RANGE	

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//if (pParams->bidAsk.ask[0] < 10000)
		//	level = 0.005 * pParams->bidAsk.ask[0];		
		//else if (pParams->bidAsk.ask[0] < 20000)
		//	level = max(50, 0.0035 * pParams->bidAsk.ask[0]);
		//else if (pParams->bidAsk.ask[0] < 30000)
		//	level = max(75, 0.003 * pParams->bidAsk.ask[0]);
		//else
		//	level = max(90, 0.0025 * pParams->bidAsk.ask[0]);	

		level = 0.005 * pParams->bidAsk.ask[0];

		maxLevel = 0.05 * pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = TRUE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = FALSE;
		isEnableMaxLevelSell = FALSE;
		isWeeklyBaseLine = FALSE;
		
		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		if (timeInfo1.tm_year > (2021 - 1900) ||
			(timeInfo1.tm_year == (2021 - 1900) && timeInfo1.tm_mon > 0) ||
			(timeInfo1.tm_year == (2021 - 1900) && timeInfo1.tm_mon == 0 && timeInfo1.tm_mday >= 18))
		{
			fastMAPeriod = 7;
			slowMAPeriod = 14;
			signalMAPeriod = 7;
		}


		stopLoss = max(stopLoss, pBase_Indicators->dailyATR * 1.8);
		//stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		isEnableLate = FALSE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		range = 5;
		
		isEnableMaxLevelRiskControl = FALSE;
		
	}
	else if (strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL)
	{
		level = 0.35;// min(0.35, 0.0053 * pParams->bidAsk.ask[0]);
		maxLevel = 0.01 * pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = FALSE;
		isEnableMaxLevel = TRUE;

		isWeeklyBaseLine = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = TRUE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = FALSE;

		isEnableEntryEOD = TRUE;

		pIndicators->minLotSize = 0.01;
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		pIndicators->volumeStep = 0.01;


		range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
	{		
		level = 0.35;// min(0.35, 0.0053 * pParams->bidAsk.ask[0]);
		maxLevel = 0.01* pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = FALSE;
		isEnableMaxLevel = TRUE;

		isWeeklyBaseLine = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = TRUE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = FALSE;

		isEnableEntryEOD = TRUE;

		pIndicators->minLotSize = 0.5;
		pIndicators->volumeStep = 0.5;

		//pIndicators->minLotSize = 0.01;
		//pIndicators->volumeStep = 0.01;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		

		range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = max(10, ((pParams->bidAsk.ask[0] - 1500) / 300) + 10);


		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		isCMFVolumeGapRisk = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		maxRisk = 2;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;

		isEnableLate = FALSE;		

		//isEnableASI = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		range = 10;

		startHour = pIndicators->startHour;
		
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{

		level = 0.05;
		maxLevel = max(0.2, (0.4 * (pParams->bidAsk.ask[0] - 15) / 5.0 ) + 0.2);

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		atrRange = 0.01; //1%

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		//isEnableMaxLevel = FALSE;

		isCMFVolumeGapRisk = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;
		
		maxRisk = 1.5;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;

		isEnableLate = FALSE;

		//isEnableASI = TRUE;

		//nextMACDRange = 0.2;

		isEnableNextdayBar = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		range = 10;

	}
	else if (strstr(pParams->tradeSymbol, "XAUEUR") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 8;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = TRUE;
		//isVolumeControl = (int)parameter(AUTOBBS_RANGE);
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = TRUE;
		isEnableCMFVolume = TRUE;
		isEnableCMFVolumeGap = FALSE;
		
		//isEnableMaxLevel = FALSE;
		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = FALSE;
		isEnableMaxLevel = FALSE;
		
		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 2;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		isEnableLate = FALSE;

		nextMACDRange = 0.2;

		isEnableNextdayBar = TRUE;
				
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		range = 10;
				
	}
	else if (strstr(pParams->tradeSymbol, "GBPCHF") != NULL)
	{
		level = 0.002; //GBPJPY
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		//isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.5;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = TRUE;
		isEnableCMFVolumeGap = FALSE;

		//isEnableMaxLevel = FALSE;

		isDailyOnly = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "EURJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "CADJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = TRUE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		//isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		level = 0.001; //GBPAUD
		maxLevel =0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		isAllVolumeRisk = FALSE;

		//stopLoss = pBase_Indicators->dailyATR * 1.5;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		//isEnableNextdayBar = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		//isEnableNoStopLoss = TRUE;

		range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		level = 0.001; //GBPUSD
		maxLevel = 0.007;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;

		isEnableNextdayBar = TRUE;

		//pIndicators->riskCap = 1.9;

		//dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		range = 10;

	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		level = 0.0025; //GBPJPY
		maxLevel = 0;		
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;		

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 12;
		slowMAPeriod = 26;
		signalMAPeriod = 9;
	}
	else
	{
		level = 0; //EURUSD
		maxLevel = 0.005;

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}

	// After 23:00, check if need to enter
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;		
		macdLimit = level / 2;

		//if (timeInfo1.tm_wday == 5)
		//{
		//	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
		//		macdLimit = 0;
		//}

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL || strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= startHour
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 start, avoid the first hour
	{		

		preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
		iTrend3Rules_preDays(pParams, pBase_Indicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, startShift);

		pIndicators->CMFVolumeGap = getCMFVolumeGap(B_DAILY_RATES, 1, fastMAPeriod, startShift);

		//iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily - 1, range-1, &rangeHigh, &rangeLow);

		//Volume indicator....
		// preVolume > MA: current volume > past average volume
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		volume_ma_5 = iMA(4, B_DAILY_RATES, 5, startShift);

		logInfo("System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf,volume_ma_5=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->cmfVolume, pIndicators->CMFVolumeGap, weekly_baseline, weekly_baseline_short, pIndicators->volume1, pIndicators->volume2, volume_ma_5);

		//Load MACD
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);
				
		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;
		

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
		//oldestOpenOrderIndex = orderIndex;
		//oldestOpenOrderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);

		pIndicators->stopLoss = stopLoss;

		//Find the highest close price after order is opened
		getHighestHourlyClosePrice(pParams, pIndicators, pBase_Indicators, B_HOURLY_RATES, orderIndex, &highHourlyClosePrice, &lowHourlyClosePrice);

		if (isEnableMaxLevelRiskControl == TRUE && pIndicators->fast > maxLevel)
			pIndicators->risk = 0.5;

		logInfo("System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		logInfo("System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift + 1), pIndicators->preFast, pIndicators->preSlow);

		logInfo("System InstanceID = %d, BarTime = %s, highHourlyClosePrice =%lf, lowHourlyClosePrice=%lf, stopLoss=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, highHourlyClosePrice, lowHourlyClosePrice, stopLoss);

		if (pIndicators->fast > 0						
			&& preDailyClose > dailyBaseLine
			&& pIndicators->fast - pIndicators->slow > macdLimit
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			
			pIndicators->stopLossPrice = max(highHourlyClosePrice,pIndicators->entryPrice) - stopLoss;

			if (isEnableNoStopLoss == TRUE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{
				//Not moving stop loss
				pIndicators->executionTrend = 0;				
				{


					if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 2 * pIndicators->stopLoss
						)
					{
						pIndicators->executionTrend = 1;
						pIndicators->entryPrice = pParams->bidAsk.ask[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					}
					else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 2 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * pIndicators->stopLoss)
					{
						pIndicators->executionTrend = 1;
						pIndicators->entryPrice = pParams->bidAsk.ask[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->stopLoss;
					}
				}
								
			}
			
			logInfo("System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			
			if (//fast > slow && preFast <= preSlow &&				
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//oldestOpenOrderIndex < 0 
				&& pIndicators->fast > pIndicators->preFast		
				)
			{
				
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				logInfo("System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if ( timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = 1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2 )
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap > 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume > 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap > 0 && pIndicators->cmfVolume >  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;					

					if (isEnableSlow == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->slow <= 0)
					{
						sprintf(pIndicators->status, "slow %lf is not greater than level 0.",
							pIndicators->slow);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (pIndicators->entrySignal != 0 &&
						pIndicators->fast <= level)
					{
						sprintf(pIndicators->status, "fast %lf is not greater than level %lf.",
							pIndicators->fast,level);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					// If price retreats back, add position
					if (isEnableLate == TRUE && preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
						&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						)
					{
						strcpy(pIndicators->status, "it is late for 5 days\n");

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolumeGap == TRUE && 
						pIndicators->entrySignal != 0 &&
						pIndicators->CMFVolumeGap <= 0)
					{
						sprintf(pIndicators->status,"CMFVolumeGap %lf is not greater than 0",
							pIndicators->CMFVolumeGap);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolume == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->cmfVolume <= 0)
					{
						sprintf(pIndicators->status,"cmfVolume %lf is not greater than 0",
							pIndicators->cmfVolume);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isVolumeControl == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->volume1 <= pIndicators->volume2)
					{
						sprintf(pIndicators->status,"volume1 %lf is not greater than volume2 %lf",
							pIndicators->volume1, pIndicators->volume2);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						preWeeklyClose <= weekly_baseline
						)
					{
						sprintf(pIndicators->status, "preWeeklyClose %lf is not greater than weekly baseline %lf.",
							preWeeklyClose, weekly_baseline);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						weekly_baseline_short <= weekly_baseline && pre3KTrend != UP
						)
					{
						sprintf(pIndicators->status, "Weekly_baseline_short %lf is less than weekly_baseline %lf and pre3KTrend %lf is not UP.",
							weekly_baseline_short, weekly_baseline, pre3KTrend);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					
					isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

					if (isEnableMaxLevelBuy == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& pIndicators->fast > maxLevel //&& truningPointIndex - 1 <= range
						&& (isEnableMaxLevel == FALSE || minPoint >= level)
						)
					{	

						sprintf(pIndicators->status, "MACD %lf exceeds max level %lf",
							pIndicators->fast, maxLevel);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0 
						&& isMACDBeili == TRUE
						&& (minPoint >= level || truningPointIndex - 1 <= range)
						//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						//&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint) //&& iClose(B_DAILY_RATES,1) < rangeHigh						
						)
					{
						strcpy(pIndicators->status, "MACD BeiLi");								

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
						
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}

				
			}			
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (pIndicators->fast < 0			
			&& preDailyClose < dailyBaseLine
			&& pIndicators->slow - pIndicators->fast > macdLimit
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];	
						
			pIndicators->stopLossPrice = min(lowHourlyClosePrice, pIndicators->entryPrice) + stopLoss;

			if (isEnableNoStopLoss == TRUE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{
				//Not moving stop loss
				pIndicators->executionTrend = 0;
				
				{
					if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss
						)
					{
						pIndicators->executionTrend = -1;
						pIndicators->entryPrice = pParams->bidAsk.bid[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					}
					else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
					{
						pIndicators->executionTrend = -1;
						pIndicators->entryPrice = pParams->bidAsk.bid[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->stopLoss;
					}
				}
			}

			logInfo("System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//oldestOpenOrderIndex < 0 
				&& pIndicators->fast < pIndicators->preFast			
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				logWarning("System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{

					pIndicators->entrySignal = -1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap < 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume < 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap < 0 && pIndicators->cmfVolume <  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					if (isEnableSlow == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->slow >= 0)
					{
						sprintf(pIndicators->status, "slow %lf is not less than level 0.",
							pIndicators->slow);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (pIndicators->entrySignal != 0 &&
						pIndicators->fast >= -1 * level)
					{
						sprintf(pIndicators->status, "fast %lf is not less than level %lf.",
							pIndicators->fast, -1* level);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					// If price retreats back, add position
					if (isEnableLate == TRUE && preHist1 < (histLevel*-1) && preHist2 < (histLevel*-1) && preHist3 < (histLevel*-1) && preHist4 < (histLevel*-1) && preHist5 < (histLevel*-1)
						&& fast1 < (level*-1) && fast2 < (level*-1) && fast3 < (level*-1) && fast4 < (level*-1) && fast5 < (level*-1)
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						)
					{
						strcpy(pIndicators->status, "it is late for 5 days\n");

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolumeGap == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->CMFVolumeGap > 0)
					{
						sprintf(pIndicators->status,"CMFVolumeGap %lf is not less than 0",
							pIndicators->CMFVolumeGap);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolume == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->cmfVolume > 0)
					{
						sprintf(pIndicators->status,"cmfVolume %lf is not less than 0",
							pIndicators->cmfVolume);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isVolumeControl == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->volume1 <= pIndicators->volume2)
					{
						sprintf(pIndicators->status,"volume1 %lf is not greater than volume2 %lf",
							pIndicators->volume1, pIndicators->volume2);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						preWeeklyClose >= weekly_baseline
						)
					{
						sprintf(pIndicators->status, "preWeeklyClose %lf is not less than weekly baseline %lf.",
							preWeeklyClose, weekly_baseline);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						weekly_baseline_short >= weekly_baseline && pre3KTrend != DOWN
						)
					{
						sprintf(pIndicators->status, "Weekly_baseline_short %lf is greater than weekly_baseline %lf and pre3KTrend %lf is not DOWN",
							weekly_baseline_short, weekly_baseline, pre3KTrend);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

					if (isEnableMaxLevelSell == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& pIndicators->fast < (maxLevel*-1)
						&& (isEnableMaxLevel == FALSE || minPoint <= -1 * level)
						)
					{

						sprintf(pIndicators->status,"MACD %lf exceeds max level %lf",
							pIndicators->fast,maxLevel*-1);

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& isMACDBeili == TRUE
						&& (minPoint <= -1 * level || truningPointIndex - 1 <= range)
						//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						//&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint) //&& iClose(B_DAILY_RATES, 1) > rangeLow						
						)
					{
						strcpy(pIndicators->status, "MACD BeiLi");								

						logWarning("System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
						
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}
			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = -1;
			//}

			pIndicators->exitSignal = EXIT_BUY;

		}

		if (isEnableATR == TRUE &&
			pIndicators->entrySignal != 0 &&
			atr5 <= pIndicators->entryPrice * atrRange)
		{
			sprintf(pIndicators->status, "atr5 %lf is not greater than %lf.",
				atr5, pIndicators->entryPrice * atrRange);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (
			pIndicators->entrySignal != 0					
			&& isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar(startShift) == FALSE
			&& ( nextMACDRange == 0 || fabs(pIndicators->fast) < nextMACDRange)
			//orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//&& isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar2(pParams, orderIndex, startShift) == FALSE
			)
		{

			sprintf(pIndicators->status,"Nextday MACD Bar %lf is negative value %lf.",
				fabs(pIndicators->fast), nextMACDRange);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
			//pIndicators->exitSignal = EXIT_ALL;
		}

		if (pIndicators->entrySignal > 0  &&
			pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) > 0.2 * pBase_Indicators->dailyATR)
		{
			sprintf(pIndicators->status, "Open price gap %lf is not less than %lf",
				pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift), 0.2 * pBase_Indicators->dailyATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal < 0 &&
			iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice > 0.2 * pBase_Indicators->dailyATR)
		{
			sprintf(pIndicators->status, "Open price gap %lf is not less than %lf",
				iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice, 0.2 * pBase_Indicators->dailyATR);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal != 0 && (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && DAY_OF_WEEK(currentTime) == SUNDAY)
		{
			sprintf(pIndicators->status, "System InstanceID = %d, BarTime = %s, skip to entry a trade on Sunday.\n",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		// Exit signal from daily chart: enter range

		if (pIndicators->fast - pIndicators->slow > macdLimit && pIndicators->preFast <= pIndicators->preSlow) // Exit SELL		
		pIndicators->exitSignal = EXIT_SELL;

		if (pIndicators->slow - pIndicators->fast > macdLimit && pIndicators->preFast >= pIndicators->preSlow) // Exit SELL		
		pIndicators->exitSignal = EXIT_BUY;

		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == BUY && preDailyClose < ma20Daily)
		//	pIndicators->exitSignal = EXIT_BUY;

		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == SELL && preDailyClose > ma20Daily)
		//	pIndicators->exitSignal = EXIT_SELL;
		//if (dailyTrend == 0)		
		//	pIndicators->exitSignal = EXIT_ALL;
	}
	

	//profit managgement
	// if profit is > 1 ATR, exit 50%
	// if proift is > 2 ATR, exit 100%

	//if MACD fast > 10 or < -10, exit proit
	//if (fast > maxLevel)
	//	pIndicators->exitSignal = EXIT_BUY;

	//if (fast < maxLevel * -1)
	//	pIndicators->exitSignal = EXIT_SELL;

	//If average = 2.3, max = 3

	//If the long term order move to break event, and try to keep adding position.	

	//if (oldestOpenOrderIndex >= 0 &&
	//	pIndicators->executionTrend != 0 &&
	//	(pIndicators->exitSignal != EXIT_BUY || pIndicators->exitSignal != EXIT_SELL || pIndicators->exitSignal != EXIT_ALL) &&
	//	(pIndicators->entrySignal != 1 || pIndicators->entrySignal != -1))
	//{
	//	//Modify MACD orders.
	//	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) + pIndicators->adjust;
	//	modifyAllOrdersOnSameDateEasy(oldestOpenOrderIndex, stopLoss, -1, pIndicators->stopMovingBackSL);

	//	//Remove modify signal
	//	pIndicators->executionTrend = 0;

	//	if ((pParams->orderInfo[oldestOpenOrderIndex].type == BUY && pParams->orderInfo[oldestOpenOrderIndex].stopLoss - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= -2 * pIndicators->adjust) ||
	//		(pParams->orderInfo[oldestOpenOrderIndex].type == SELL &&  pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->orderInfo[oldestOpenOrderIndex].stopLoss >= -2 * pIndicators->adjust))
	//	{
	//		logWarning("System InstanceID = %d, BarTime = %s,stopLoss =%lf. it is ok to add new positions in a long term trend now.",
	//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->orderInfo[oldestOpenOrderIndex].stopLoss);

	//		pIndicators->tradeMode = 2;
	//		pIndicators->risk = 0.5;
	//		pIndicators->executionTrend = 0;

	//		addMoreOrdersOnLongTermTrend(pParams, pIndicators, pBase_Indicators, oldestOpenOrderIndex);
	//	}
	//	
	//}

	return SUCCESS;
}
