/*
 * MACD BEILI Strategy Module
 * 
 * Provides MACD BEILI strategy execution functions.
 * MACD BEILI is a counter-trend trading strategy used during reversal times.
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
#include "strategies/autobbs/swing/macd_beili/MACDBEILIStrategy.h"

AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;

	int    shiftPreDayBar = shift1Index - 1;
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int orderIndex;
	int index1, index2, index3;
	double level = 0, histLevel = 0, maxLevel = 0;
	double preClose1, preClose2, preClose3, preClose4, preClose5;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5, fastMin, fastMax;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double dailyRangHigh = 0.0, dailyRangeLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	double stopLoss = pBase_Indicators->pDailyMaxATR;

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;
	BOOL isDailyOnly = TRUE;
	double takeProfitPrice, takeProfit;

	double maxRisk = 1;

	double ma5Daily;

	BOOL isEnableFlatBars = FALSE;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// ������ͼ��ʷ�����Ʒ�����

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 28;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 0.9; //precetage				

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.7; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		//isEnableFlatBars = TRUE;

	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.7; //GBPJPY						

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		//isEnableFlatBars = TRUE;

	}
	else
	{
		level = 0; //EURUSD		

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}

	//�����23H�����Լ���Ƿ�����볡
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;
		macdLimit = 0;

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	//����������գ����ܳ��� 0.2 daily ATR,�ȴ�����
	if (timeInfo1.tm_hour >= 1
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 ���ף� �ܿ����̵�һ��Сʱ,ͬʱֻ���µ�һ��ĵ�һ��Сʱ
	{

		//The last 5 days High and Low
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 5, &dailyRangHigh, &dailyRangeLow);

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		logWarning("System InstanceID = %d, BarTime = %s, shortDailyHigh=%lf,shortDailyLow=%lf,daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline);

		//Load MACD
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);

		//Use the min 
		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;

		fastMin = 100.0;
		fastMin = min(fastMin, fast1);
		fastMin = min(fastMin, fast2);
		fastMin = min(fastMin, fast3);
		fastMin = min(fastMin, fast4);
		fastMin = min(fastMin, fast5);

		fastMax = -100.0;
		fastMax = max(fastMax, fast1);
		fastMax = max(fastMax, fast2);
		fastMax = max(fastMax, fast3);
		fastMax = max(fastMax, fast4);
		fastMax = max(fastMax, fast5);


		logInfo("System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf,fastMax=%lf,fastMin=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow, fastMax, fastMin);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		if (
			pIndicators->fast - pIndicators->slow > macdLimit // cross up
			//pIndicators->fast < 0
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{				
				//if (fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) > 1.5 *pIndicators->adjust &&
				//	pIndicators->entryPrice - pParams->orderInfo[orderIndex].openPrice > fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice)){
				//	//stopLoss = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice);
				//	//stopLoss = fabs(iLow(B_DAILY_RATES, 1) - pIndicators->entryPrice);	
				//	stopLoss = pIndicators->entryPrice - pParams->orderInfo[orderIndex].openPrice;
				//}
				//else
				//	stopLoss = 9999999;
				stopLoss = pIndicators->entryPrice - pBase_Indicators->dailyS;
			}
			else{
				stopLoss = fabs(pIndicators->entryPrice - dailyRangeLow) + 0.2 * pBase_Indicators->dailyATR;
			}

			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

			pIndicators->takeProfitPrice = daily_baseline - 0.2 * pBase_Indicators->dailyATR;
			takeProfit = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);

			//takeProfit = min(takeProfit, 2 * stopLoss);
			//pIndicators->takeProfitPrice = pIndicators->entryPrice + takeProfit;

			logInfo("System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (
				fastMin <= (-1 * level) * pIndicators->entryPrice / 100				
				//&& pIndicators->preFast <  pIndicators->fast
				//&& pIndicators->fast <= (-1 * level) /3
				&& iClose(B_DAILY_RATES, 1) > iClose(B_DAILY_RATES, 2) 
				&& iClose(B_DAILY_RATES, 1) - iOpen(B_DAILY_RATES, 1) >= 0.5*(iHigh(B_DAILY_RATES, 1) - iLow(B_DAILY_RATES,1))
				&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) <= 0.2 * pBase_Indicators->dailyATR
				&& takeProfit > 0.8* stopLoss // > 1				
				)
			{

				//if (fastMin < (-1.5 * level))
				//	maxRisk = 1.5;
				//else if (fastMin < (-2 * level))
				//	maxRisk = 2;


				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				logWarning("System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = 1;

					pIndicators->risk = maxRisk;
				}

			}
			
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (
			pIndicators->slow - pIndicators->fast > macdLimit
			//pIndicators->fast > 0
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{	
				//if (fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) > 1.5 *pIndicators->adjust &&
				//	pParams->orderInfo[orderIndex].openPrice - pIndicators->entryPrice > fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice)){
				//	//stopLoss = fabs(iHigh(B_DAILY_RATES, 1) - pIndicators->entryPrice);					
				//	stopLoss = pParams->orderInfo[orderIndex].openPrice - pIndicators->entryPrice;
				//}
				//else
				//	stopLoss = 9999999;
				stopLoss = pBase_Indicators->dailyS - pIndicators->entryPrice;
			}
			else{
				stopLoss = fabs(pIndicators->entryPrice - dailyRangHigh) + 0.2 * pBase_Indicators->dailyATR;
			}

			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

			pIndicators->takeProfitPrice = daily_baseline + 0.2 * pBase_Indicators->dailyATR;
			takeProfit = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);

			//takeProfit = min(takeProfit, 2 * stopLoss);
			//pIndicators->takeProfitPrice = pIndicators->entryPrice - takeProfit;

			logInfo("System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (
				fastMax >= level * pIndicators->entryPrice / 100
				//&& fastMin > 0.1 * pIndicators->entryPrice / 100
				//&& pIndicators->preFast >  pIndicators->fast
				//&& pIndicators->fast >= level / 3
				&& iClose(B_DAILY_RATES, 1) < iClose(B_DAILY_RATES, 2)
				&& iOpen(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 1) >= 0.5*(iHigh(B_DAILY_RATES, 1) - iLow(B_DAILY_RATES, 1))
				&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice <= 0.2 * pBase_Indicators->dailyATR
				&& takeProfit > 0.8*stopLoss // > 1						
				)
			{

				//if (fastMax > 1.5* level)
				//	maxRisk = 1.5;
				//else if (fastMax > 2 * level)
				//	maxRisk = 2.0;

				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				logWarning("System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = -1;
					pIndicators->risk = maxRisk;
				}

			}

			
			pIndicators->exitSignal = EXIT_BUY;

		}

		ma5Daily = iMA(3, B_DAILY_RATES, 4, startShift + 1);

		if (isEnableFlatBars == TRUE
			&& pIndicators->entrySignal != 0
			&& fabs(iClose(B_DAILY_RATES, startShift + 1) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			&& fabs(iClose(B_DAILY_RATES, startShift + 2) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			&& fabs(iClose(B_DAILY_RATES, startShift + 3) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			&& fabs(iClose(B_DAILY_RATES, startShift + 4) - ma5Daily) < 0.33 * pBase_Indicators->dailyATR
			)
		{
			logWarning("System InstanceID = %d, BarTime = %s, skip entry signal due to flat bars",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			pIndicators->entrySignal = 0;
		}

	}

	return SUCCESS;
}
