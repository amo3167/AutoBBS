/*
 * Day Trading Strategy Module
 * 
 * Provides day trading strategy execution functions.
 * Includes XAUUSD, GBPJPY, and ExecutionOnly strategies.
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
#include "StrategyUserInterface.h"
#include "strategies/autobbs/swing/daytrading/DayTradingStrategy.h"
#include "strategies/autobbs/swing/daytrading/DayTradingHelpers.h"
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow, primary_close_pre1 = iClose(B_PRIMARY_RATES, 1),secondary_close_pre1 = iClose(B_SECONDARY_RATES,1);
	double ATR0, Range, ATR0_EURO = 10, Range_Limit;
	double close_prev1 = iClose(B_DAILY_RATES, 1);
	double openOrderHigh, openOrderLow;

	int count;
	double adjust = 0.15;
	double stopLoss = pBase_Indicators->dailyATR*1.2;
	BOOL isOpen;
	OrderType side;

	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double gap;
	double ATRWeekly0;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int MATrend_1H, noNewTradeSignal = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	

	//Fix lots
	//pIndicators->splitTradeMode = 18;
	//Dyanmic lots
	pIndicators->splitTradeMode = 16;

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	{
		Range = pIndicators->atr_euro_range * 2 / 3;		
		pIndicators->risk = 1;
	}
	else
	{
		Range = pIndicators->atr_euro_range;
		
		//if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
		//	pIndicators->risk = 1.5;
		//else
		pIndicators->risk = 1;
	}

	stopLoss = 10;

	safe_timeString(timeString, currentTime);

	closeAllWithNegativeEasy(1, currentTime, 3);


	pBase_Indicators->maTrend = getMATrend(2, B_SECONDARY_RATES, 1);

	if (XAUUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

	ATRWeekly0 = iAtr(B_WEEKLY_RATES, 1, 0);


	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);


	if ( (int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && ATRWeekly0 > pBase_Indicators->pWeeklyPredictMaxATR)
		return SUCCESS;

	//if (pBase_Indicators->pDailyPredictATR >= 10 && ATRWeekly0 < pBase_Indicators->pWeeklyPredictATR)
	//	pIndicators->risk = 2;

	count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count > 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh, &intradayLow);
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);


	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf, asia_close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low, pIndicators->asia_close);

	if (timeInfo1.tm_hour >= 17)
	{
		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);

		logInfo("System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO,pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);
	}

	//��17�c�����������С����������ƣ�����80�㡣������� 0.67*0.8
	if (ATR0_EURO < Range)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)		
			pIndicators->exitSignal = EXIT_ALL;
			
		return SUCCESS;
	}
	
	// secondary rate is 15M , priarmy rate is 5M��ʹ��1M?
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)  //���û�п���, �Ϳ��Կ���
	{
		if (((ATR0 >= 4 && pBase_Indicators->dailyTrend_Phase == RANGE_PHASE) || pBase_Indicators->dailyTrend_Phase != RANGE_PHASE)
			&& ATR0_EURO > Range && pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && timeInfo1.tm_hour < 22)//˳��
		{
			gap = pHigh - pParams->bidAsk.ask[0];

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
			pIndicators->entrySignal = 1;
			pIndicators->lossTimes = 0;
		}
		else if (((ATR0 >= 4 && pBase_Indicators->dailyTrend_Phase == RANGE_PHASE) || pBase_Indicators->dailyTrend_Phase != RANGE_PHASE)
			&& ATR0_EURO > Range && pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && timeInfo1.tm_hour < 22)//˳��
		{
			gap = pParams->bidAsk.bid[0] - pLow;

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
			pIndicators->entrySignal = -1;
			pIndicators->lossTimes = 0;
		}
		else //���ƣ�ʹ�ÿռ佻�׷�,ͻ�������̵ĸߵ͵�
		{

			//ע�⣬����Ҳ���Գ���˳�Ƶ������
			//���������ƣ�����Ҫ15M�����̼ۡ��������secondary rate
			if (pBase_Indicators->maTrend > 0)
			{
				if (ATR0_EURO > Range && primary_close_pre1 > pIndicators->asia_high && timeInfo1.tm_hour < 22)
				{
					gap = pHigh - pParams->bidAsk.ask[0];

					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
					pIndicators->entrySignal = 1;
					pIndicators->lossTimes = 0;
				}

				if (ATR0_EURO > Range && secondary_close_pre1 < pIndicators->asia_low && timeInfo1.tm_hour < 22)
				{
					gap = pParams->bidAsk.bid[0] - pLow;

					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
					pIndicators->entrySignal = -1;
					pIndicators->lossTimes = 0;
				}
			}

			if (pBase_Indicators->maTrend < 0)
			{
				if (ATR0_EURO > Range && secondary_close_pre1 > pIndicators->asia_high && timeInfo1.tm_hour < 22)
				{
					gap = pHigh - pParams->bidAsk.ask[0];

					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
					pIndicators->entrySignal = 1;
					pIndicators->lossTimes = 0;
				}

				if (ATR0_EURO > Range && primary_close_pre1 < pIndicators->asia_low && timeInfo1.tm_hour < 22)
				{
					gap = pParams->bidAsk.bid[0] - pLow;

					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
					pIndicators->entrySignal = -1;
					pIndicators->lossTimes = 0;
				}
			}
		}
	}
	else
	{

		side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
		if (side == SELL)
		{
			pIndicators->executionTrend = -1;
			logInfo("System InstanceID = %d, BarTime = %s, Side = SELL isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

			//С��80�� ����ʹ�ÿռ䡣
			//1. ��5Mͻ��������볡
			//2. ����������ǵ�һ���Ļ�����Ҫ��15���Ӻ�ʹ��15 BBS Trend ���
			//3. ���û��BBS Trend��֧�֣���close trade,���볡���ȴ���һ�λ��ᣨ���ʱ�򣬲����볡��
			//4. ���ʱ�䵽�������̣�����ʹ��15MBBS
			//5. ����ռ䲻��30�㣬���ʱ�򣬲�Ҫð���볡���ȴ��ص���֪����30��Ŀռ�Ϊֹ����15MBBS��֧���¡�			
			gap = pHigh - pParams->bidAsk.ask[0];

			if (ATR0 >= Range && 
				pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && 
				pIndicators->bbsIndex_secondary == shift1Index_secondary
				) 
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
			}
			else
			{
				if (pBase_Indicators->maTrend > 0)
				{
					if (primary_close_pre1 - adjust > openOrderHigh)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (secondary_close_pre1 + adjust < openOrderLow)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								logWarning("System InstanceID = %d, BarTime = %s, Enter a sell trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}

				if (pBase_Indicators->maTrend < 0)
				{
					if (secondary_close_pre1 - adjust > openOrderHigh)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (primary_close_pre1 + adjust < openOrderLow)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								logWarning("System InstanceID = %d, BarTime = %s, Enter a sell trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}
			}

		}

		if (side == BUY)
		{
			pIndicators->executionTrend = 1;
			logInfo("System InstanceID = %d, BarTime = %s, Side = BUY isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

			gap = pParams->bidAsk.bid[0] - pLow;

			if (ATR0 >= Range
				&& pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 &&
				pIndicators->bbsIndex_secondary == shift1Index_secondary 
				)
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
			}
			else
			{
				if (pBase_Indicators->maTrend > 0)
				{
					if (secondary_close_pre1 + adjust < openOrderLow)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (primary_close_pre1 - adjust > openOrderHigh)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								logWarning("System InstanceID = %d, BarTime = %s, Enter a buy trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}

				if (pBase_Indicators->maTrend < 0)
				{
					if (primary_close_pre1 + adjust < openOrderLow)
					{
						XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
					}

					if (isOpen == FALSE)
					{
						if (secondary_close_pre1 - adjust > openOrderHigh)
						{
							XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
							if (pIndicators->entrySignal != 0)
								logWarning("System InstanceID = %d, BarTime = %s, Enter a buy trade again.",
								(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						}
					}
				}
			}
		}

		// XAUUSD_Daily_Stop_Check was removed - function no longer exists
		//if (pIndicators->entrySignal == 0 
		//	&& timeInfo1.tm_hour >= 10 && timeInfo1.tm_hour <=17
		//	)
		//	XAUUSD_Daily_Stop_Check(pParams, pIndicators, pBase_Indicators);


		////��17�c�����������С����������ƣ�����80�㡣������� 0.67*0.8
		//if (ATR0_EURO < Range)
		//{
		//	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
		//	{
		//		//pIndicators->exitSignal = EXIT_ALL;

		//		//closeAllWithNegativeEasy(5, currentTime, 3);

		//		if ((pIndicators->bbsTrend_secondary == -1 && side == BUY)
		//			|| (pIndicators->bbsTrend_secondary == 1 && side == SELL)
		//			)
		//		{
		//			logWarning("System InstanceID = %d, BarTime = %s, Exiting a trade on after 17H side = %ld.",
		//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, side);

		//			pIndicators->exitSignal = EXIT_ALL;
		//		}
		//	}
		//	return SUCCESS;
		//}

		//��21�c������І��ӣ�ʹ��BBS15ƽ��
		if (pIndicators->entrySignal == 0 && timeInfo1.tm_hour >= 21)
		{
			if (isOpen == TRUE)
			{
				if ((pIndicators->bbsTrend_secondary == -1 && side == BUY)
					|| (pIndicators->bbsTrend_secondary == 1 && side == SELL)
					)
				{
					logWarning("System InstanceID = %d, BarTime = %s, Exiting a trade on after 21H side = %ld.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,side);

					pIndicators->exitSignal = EXIT_ALL;
				}
			}
		}
	}


	MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);

	//if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 3)
	if (pIndicators->entrySignal != 0)
	{
		// too close ��ũ
		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1
			&& abs(pBase_Indicators->maTrend) <= 1 &&
			((pBase_Indicators->maTrend > 0 && MATrend_1H < 0)
			|| (pBase_Indicators->maTrend < 0 && MATrend_1H > 0)
			))
		{
			noNewTradeSignal = 1;
			logWarning("System InstanceID = %d, BarTime = %s, 15M Trend = %ld, 1H Trend = %ld,noNewTradeSignal=%ld",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, MATrend_1H, noNewTradeSignal);
			pIndicators->entrySignal = 0;
		}

		//if (gap < 3)
		//{
		//	logWarning("System InstanceID = %d, BarTime = %s, gap=%lf is lesss than 3",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, gap);
		//	pIndicators->entrySignal = 0;
		//}
	}

	

	//if (pIndicators->entrySignal == 0 && ATR0 > 8)
	//{
	//	HalfPoint = intradayHigh - ATR0 * 2 /3;

	//	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
	//	{
	//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == BUY
	//			&& primary_close_pre1 < HalfPoint)
	//		{
	//			logWarning("System InstanceID = %d, BarTime = %s, Exiting a buy order",
	//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString) ;
	//			pIndicators->exitSignal = EXIT_BUY;
	//		}
	//		

	//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == SELL
	//			&& primary_close_pre1 > HalfPoint)
	//		{
	//			logWarning("System InstanceID = %d, BarTime = %s, Exiting a sell order",
	//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
	//			pIndicators->exitSignal = EXIT_SELL;
	//		}
	//	}
	//}

	//if (pIndicators->entrySignal != 0 && pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity  > 0.01)
	//{
	//	logWarning("System InstanceID = %d, BarTime = %s, Daily Loss=%lf, skip this entry signal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	return SUCCESS;
}
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;	
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0, Range;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.03;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;

	Order_Info orderInfo;

	double entryPrice;

	int latestOrderIndex = 0, orderCountToday = 0;

	BOOL isBreakEvent = FALSE;
	int euro_index_rate;
	double ATREuroPeriod = 0.0;
	int executionTrend;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 17;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		executionTrend = -1;
	else
		executionTrend = 0;

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	//Range = 0.5;
	Range = min(0.5, pBase_Indicators->pDailyPredictATR / 2);
	//if (pBase_Indicators->pDailyPredictATR < 1.2)
	//	Range = 0.5;
	//else if (pBase_Indicators->pDailyPredictATR < 1.5)
	//	Range = 0.6;
	//else
	//	Range = 0.7;

	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{
		if (hasOpenOrder())
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && GBPJPY_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		return SUCCESS;

	//�������ڸߵ͵�
	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
	}
	else
		return SUCCESS;	

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	ATR0 = fabs(intradayHigh - intradayLow);

	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	//���� close all orders before EOD
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
	{
		if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}	


	// secondary rate is 5M , priarmy rate is 1M
	if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE)
	{
		//��80���ڣ������볡����������ˣ��ͷ������ս��ס�
		//if (ATR0 >= 1.0)
		//	return SUCCESS;

		//����5M���ߵ�֧��
		if (pParams->bidAsk.ask[0] - intradayLow >= Range 
			&& intradayHigh - pParams->bidAsk.bid[0] < Range
			//&& pBase_Indicators->maTrend > 0
			&& (executionTrend > 0 || (executionTrend == 0 && pBase_Indicators->maTrend > 0))
			&& timeInfo1.tm_hour <= 15		
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;			
			pIndicators->entrySignal = 1;
		}

		if (intradayHigh - pParams->bidAsk.bid[0] >= Range 
			&& pParams->bidAsk.ask[0] - intradayLow < Range
			//&& pBase_Indicators->maTrend < 0
			&& (executionTrend < 0 || (executionTrend == 0 && pBase_Indicators->maTrend < 0))
			&& timeInfo1.tm_hour <= 15
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
			pIndicators->entrySignal = -1;
		}		

	}
	else
	{
		if (pIndicators->winTimes > 0){
			if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
				pIndicators->exitSignal = EXIT_ALL;
			return SUCCESS;
		}

		//latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		//������Կ���getlatestorder, ��������open order����Ϊ���ܻ�ƽ��
		//side = getLastestOpenOrderTypeEasy_GBPJPY(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow);
		side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow,&isOpen);
		
		//����Ѿ�����һ��trade���Ǿ��ǵڶ��ν��ף�Ҫʹ�����ڵĸߵ͵�
		//�������2��trades,�Ǿ��ǵ����ν����ˣ�ʹ��orderHigh, orderLow.
		orderCountToday = getOrderCountTodayEasy(currentTime);
		//orderCountToday = getOrderCountTodayExcludeBreakeventOrdersEasy(currentTime,0.1);
		if (orderCountToday == 1)
		{
			openOrderHigh = intradayHigh;
			openOrderLow = intradayLow;

		}
			

		//�ӵ�һ����ʼ��close negative if ��ӯ����20���
		if (orderCountToday >= 1)
		{				
			//entryPrice = getLastestOrderPriceEasy(B_PRIMARY_RATES, &isOpen);
			//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			//����Բ�����ͬʱ�������open order,��������index,Ӧ�ö���0			 
			//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
			if (isOpen == TRUE)
			{			
				entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				if (side == SELL)
				{
					//�������20��󣬳��ֵ�һ��5M�����ߣ��볡��
					if (entryPrice - openOrderLow > 0.3 &&
						//entryPrice - pParams->bidAsk.ask[0] < 0
						//iClose(B_SECONDARY_RATES, 1) > iOpen(B_SECONDARY_RATES,1)
						entryPrice - pParams->bidAsk.ask[0] < 0.3
						//pParams->bidAsk.ask[0] - openOrderLow > 0.2
						)						
					{
						//closeAllWithNegativeEasy(5, currentTime, 3); //close them intraday on break event
						closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
						logWarning("System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
						return SUCCESS;
					}
				}

				if (side == BUY)
				{
					//�������20��󣬳��ֵ�һ��5M�����ߣ��볡
					if (openOrderHigh - entryPrice > 0.3 && 
						//entryPrice - pParams->bidAsk.bid[0] > 0
						//iClose(B_SECONDARY_RATES, 1) < iOpen(B_SECONDARY_RATES, 1)
						pParams->bidAsk.bid[0] - entryPrice < 0.3
						//openOrderHigh - pParams->bidAsk.bid[0] > 0.2
						)
					{
						//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
						closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
						logWarning("System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
						return SUCCESS;
					}
				}
			}			
		}	
		
		//if (orderCountToday == 1 && pParams->orderInfo[latestOrderIndex].ticket != 0 && pParams->orderInfo[latestOrderIndex].isOpen == FALSE && pParams->orderInfo[latestOrderIndex].profit < 0
		//	&& fabs(pParams->orderInfo[latestOrderIndex].openPrice - pParams->orderInfo[latestOrderIndex].closePrice) <= 0.1)
		//	isBreakEvent = TRUE;

		if (side == SELL)
		{
			
			if (isOpen == TRUE)
			{
				if (orderCountToday == 2)
				{
					openOrderLow = min(openOrderLow, pParams->orderInfo[latestOrderIndex].openPrice);
				}


				if (pParams->bidAsk.ask[0] - openOrderLow >= Range)
					//if (pParams->bidAsk.ask[0] - intradayLow >= Range)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

					//pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
					//pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0 )
					{
						pIndicators->risk = pow(2, pIndicators->lossTimes);
						pIndicators->entrySignal = 1;
					}
					pIndicators->exitSignal = EXIT_SELL;
				}
			}

		}

		if (side == BUY)
		{
			if (isOpen == TRUE )
			{

				if (orderCountToday == 2)
				{
					openOrderHigh = max(openOrderHigh, pParams->orderInfo[latestOrderIndex].openPrice);
				}

				if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)
					//if (intradayHigh - pParams->bidAsk.bid[0] >= Range)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
					//pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
					//pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0)
					{
						pIndicators->risk = pow(2, pIndicators->lossTimes);
						pIndicators->entrySignal = -1;
					}

					pIndicators->exitSignal = EXIT_BUY;
				}

			}
		}

	}


	//���潻�׵���Ϣ
	//saveTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo, (BOOL)pParams->settings[IS_BACKTESTING]);

	return SUCCESS;
}
AsirikuyReturnCode workoutExecutionTrend_DayTrading_ExecutionOnly_Old(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0, Range;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.03;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;

	Order_Info orderInfo;

	double entryPrice;

	int latestOrderIndex = 0, orderCountToday = 0;
		
	int euro_index_rate;
	double ATREuroPeriod = 0.0;

	BOOL hasCurrentDayOrder = FALSE;

	int asia_index_rate = 0;
	double asia_ATR = 0;
	int executionTrend;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 29;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		executionTrend = -1;
	else
		executionTrend = 0;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		pIndicators->stopLoss = pIndicators->atr_euro_range;		
		pIndicators->takePrice = pIndicators->stopLoss * 0.6;
		pIndicators->startHour = 9;
		pIndicators->endHour = 15;
	}

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	Range = pIndicators->stopLoss;
	
	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{
		if (hasOpenOrder())
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	if (timeInfo1.tm_hour < pIndicators->startHour)
		return SUCCESS;

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);
	

	//�������ڸߵ͵�
	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
	}
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	pIndicators->atr0 = fabs(intradayHigh - intradayLow);
	ATR0 = pIndicators->atr0;
	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	if ((timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30) || pIndicators->winTimes > 0)
	{
		if (hasOpenOrder()==TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	asia_index_rate = shift1Index_primary - ((timeInfo1.tm_hour - pIndicators->startHour) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]));

	count = (pIndicators->startHour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);
		
	asia_ATR = fabs(pIndicators->asia_high - pIndicators->asia_low);

	hasCurrentDayOrder = hasSameDayOrderEasy(currentTime, &isOpen);
	if(hasCurrentDayOrder == TRUE && isOpen == FALSE)
		return SUCCESS;


	if (hasCurrentDayOrder == FALSE)
	{

		////����5M���ߵ�֧��
		//if (pParams->bidAsk.ask[0] - intradayLow >= Range && intradayHigh - pParams->bidAsk.bid[0] < Range 
		//	&& ( (asia_ATR > Range && iClose(B_DAILY_RATES, 0) - iOpen(B_DAILY_RATES, 0) > 0)
		//	|| asia_ATR <= Range)
		//	&& executionTrend > 0 						
		//	&& timeInfo1.tm_hour <= pIndicators->endHour			
		//	)
		//{
		//	pIndicators->executionTrend = 1;
		//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
		//	pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;
		//	pIndicators->entrySignal = 1;
		//}

		if (pParams->bidAsk.ask[0] - intradayLow >= Range && intradayHigh - pParams->bidAsk.bid[0] < Range			
			&& executionTrend == 1
			&& pBase_Indicators->maTrend > 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = intradayLow;
			pIndicators->entrySignal = 1;
		}

		//if (intradayHigh - pParams->bidAsk.bid[0] >= Range 	&& pParams->bidAsk.ask[0] - intradayLow < Range 
		//	&&(( asia_ATR > Range && iClose(B_DAILY_RATES, 0) - iOpen(B_DAILY_RATES, 0) < 0)
		//	|| asia_ATR <= Range )			
		//	&& executionTrend < 0 
		//	&& timeInfo1.tm_hour <= pIndicators->endHour			
		//	)
		//{
		//	pIndicators->executionTrend = -1;
		//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
		//	pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;
		//	pIndicators->entrySignal = -1;
		//}

		if (intradayHigh - pParams->bidAsk.bid[0] >= Range 	&& pParams->bidAsk.ask[0] - intradayLow < Range			
			&& executionTrend == 0 && pBase_Indicators->maTrend < 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = intradayHigh;
			pIndicators->entrySignal = -1;
		}
	}
	else
	{

		side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);

		//����Ѿ�����һ��trade���Ǿ��ǵڶ��ν��ף�Ҫʹ�����ڵĸߵ͵�
		//�������2��trades,�Ǿ��ǵ����ν����ˣ�ʹ��orderHigh, orderLow.
		orderCountToday = getOrderCountTodayEasy(currentTime);
		//orderCountToday = getOrderCountTodayExcludeBreakeventOrdersEasy(currentTime,0.1);
		if (orderCountToday == 1)
		{
			openOrderHigh = intradayHigh;
			openOrderLow = intradayLow;

		}
				
		if (orderCountToday >= 1)
		{
			//entryPrice = getLastestOrderPriceEasy(B_PRIMARY_RATES, &isOpen);
			//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			//����Բ�����ͬʱ�������open order,��������index,Ӧ�ö���0			 
			//if (pParams->orderInfo[latestOrderIndex].isOpen == TRUE)


			if (isOpen == TRUE)
			{				

				entryPrice = pParams->orderInfo[latestOrderIndex].openPrice;
				if (side == SELL)
				{				
					if (entryPrice - openOrderLow > pIndicators->takePrice &&
						entryPrice - pParams->bidAsk.ask[0] < pIndicators->takePrice
						)
					{
						//closeAllWithNegativeEasy(5, currentTime, 3); //close them intraday on break event
						closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
						logWarning("System InstanceID = %d, BarTime = %s, closing sell order: entryPrice =%lf, openOrderLow=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderLow);
						return SUCCESS;
					}
				}

				if (side == BUY)
				{
					//�������20��󣬳��ֵ�һ��5M�����ߣ��볡
					if (openOrderHigh - entryPrice > pIndicators->takePrice &&
						pParams->bidAsk.bid[0] - entryPrice < pIndicators->takePrice
						)
					{
						//closeAllWithNegativeEasy(5, currentTime, 3);//close them intraday on break event
						closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
						logWarning("System InstanceID = %d, BarTime = %s, closing buy order: entryPrice =%lf, openOrderHigh=%lf",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, entryPrice, openOrderHigh);
						return SUCCESS;
					}
				}
			}
		}

		if (side == SELL)
		{

			if (isOpen == TRUE)
			{
				if (orderCountToday == 2)
				{
					openOrderLow = min(openOrderLow, pParams->orderInfo[latestOrderIndex].openPrice);
				}


				if (pParams->bidAsk.ask[0] - openOrderLow >= Range)					
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;

					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0)
					{
						//pIndicators->risk = pow(2, pIndicators->lossTimes);
						//pIndicators->entrySignal = 1;
					}
					pIndicators->exitSignal = EXIT_SELL;
				}
			}

		}

		if (side == BUY)
		{
			if (isOpen == TRUE)
			{

				if (orderCountToday == 2)
				{
					openOrderHigh = max(openOrderHigh, pParams->orderInfo[latestOrderIndex].openPrice);
				}

				if (openOrderHigh - pParams->bidAsk.bid[0] >= Range)					
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR;					
					if (pIndicators->lossTimes < pIndicators->maxTradeTime && pIndicators->winTimes == 0)
					{
						//pIndicators->risk = pow(2, pIndicators->lossTimes);
						//pIndicators->entrySignal = -1;
					}

					pIndicators->exitSignal = EXIT_BUY;
				}

			}
		}

	}

	return SUCCESS;
}
AsirikuyReturnCode workoutExecutionTrend_DayTrading_ExecutionOnly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0, Range;
	double close_prev1 = iClose(B_DAILY_RATES, 1), high_prev1 = iHigh(B_DAILY_RATES, 1), low_prev1 = iLow(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

	double openOrderHigh, openOrderLow;
	int count, isOpen;
	double adjust = 0.03;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	OrderType side;

	Order_Info orderInfo;

	double entryPrice;

	int latestOrderIndex = 0, orderCountToday = 0;

	int euro_index_rate;
	double ATREuroPeriod = 0.0;

	BOOL hasCurrentDayOrder = FALSE;

	int asia_index_rate = 0;
	double asia_ATR = 0;
	int executionTrend;

	double upperBBand, lowerBBand;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);


	safe_timeString(timeString, currentTime);


	pIndicators->splitTradeMode = 29;
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		executionTrend = -1;
	else
		executionTrend = 0;

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		pIndicators->stopLoss = pIndicators->atr_euro_range;
		pIndicators->takePrice = pIndicators->stopLoss * 0.6;
		pIndicators->startHour = 9;
		pIndicators->endHour = 15;
	}

	pBase_Indicators->maTrend = getMATrend(iAtr(B_SECONDARY_RATES, 20, 1), B_SECONDARY_RATES, 1);

	Range = pIndicators->stopLoss;

	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min == 0)
	{
		if (hasOpenOrder())
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	if (timeInfo1.tm_hour < pIndicators->startHour)
		return SUCCESS;

	logInfo("System InstanceID = %d, BarTime = %s, pDailyPredictATR = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR);


	//�������ڸߵ͵�
	count = timeInfo1.tm_hour  * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);
	if (count > 1)
	{
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary, count, &intradayHigh, &intradayLow);
	}
	else
		return SUCCESS;

	intradayLow = min(close_prev1, intradayLow);
	intradayHigh = max(close_prev1, intradayHigh);
	pIndicators->atr0 = fabs(intradayHigh - intradayLow);
	ATR0 = pIndicators->atr0;
	logInfo("System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);

	if ((timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30) || pIndicators->winTimes > 0)
	{
		if (hasOpenOrder() == TRUE)
			pIndicators->exitSignal = EXIT_ALL;
		return SUCCESS;
	}

	asia_index_rate = shift1Index_primary - ((timeInfo1.tm_hour - pIndicators->startHour) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]));

	count = (pIndicators->startHour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) - 1;
	if (count >= 1)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
	else
		return FALSE;

	pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
	pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
	pIndicators->asia_open = close_prev1;
	pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);


	logInfo("System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low);

	asia_ATR = fabs(pIndicators->asia_high - pIndicators->asia_low);

	hasCurrentDayOrder = hasSameDayOrderEasy(currentTime, &isOpen);
	if (hasCurrentDayOrder == TRUE && isOpen == FALSE)
		return SUCCESS;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	if (pIndicators->fast > 0 && pIndicators->fast > pIndicators->preFast && pIndicators->fast > pIndicators->slow)
		executionTrend = 1;
	else if (pIndicators->fast < 0 && pIndicators->fast < pIndicators->preFast && pIndicators->fast < pIndicators->slow)
		executionTrend = -1;
	else
		executionTrend = 0;

	if (hasCurrentDayOrder == FALSE)
	{

		
		if (
			pParams->bidAsk.ask[0] - intradayLow >= Range && intradayHigh - pParams->bidAsk.bid[0] < Range && 
			executionTrend == 1
			&& pBase_Indicators->maTrend > 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			upperBBand = iBBands(B_PRIMARY_RATES, 50, 2, 0, 1);

			logWarning("System InstanceID = %d, BarTime = %s, upperBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, upperBBand, iClose(B_PRIMARY_RATES, 1));


			if (upperBBand > 0 && iClose(B_PRIMARY_RATES, 1) > upperBBand)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->stopLossPrice = intradayLow;
				pIndicators->entrySignal = 1;
			}
		}

		
		if (
			intradayHigh - pParams->bidAsk.bid[0] >= Range 	&& pParams->bidAsk.ask[0] - intradayLow < Range	&& 
			executionTrend == -1 && pBase_Indicators->maTrend < 0
			&& timeInfo1.tm_hour <= pIndicators->endHour
			)
		{
			lowerBBand = iBBands(B_PRIMARY_RATES, 50, 2, 2, 1);

			logWarning("System InstanceID = %d, BarTime = %s, lowerBBand = %lf, preCloseBar = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lowerBBand, iClose(B_PRIMARY_RATES, 1));

			if (lowerBBand > 0 && iClose(B_PRIMARY_RATES, 1) < lowerBBand)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->stopLossPrice = intradayHigh;
				pIndicators->entrySignal = -1;
			}
		}
	}
	

	return SUCCESS;
}
