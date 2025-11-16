/*
* Common libaray for autobbs
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include <stdio.h>

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

/* used to round a double to X decimal places */
double roundUp(double lots,double volumeStep)
{
	double adjustedLots = lots;
	double remainder = fmod(lots, volumeStep);
	if (remainder > volumeStep / 2)
		adjustedLots += volumeStep;
	return floor(adjustedLots / volumeStep) * volumeStep;
}

int filterExcutionTF(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	struct tm timeInfo1;
	time_t currentTime;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	//Only work for 15M and always use secondary rates, in this case, it should be 15M
	if ((int)pParams->settings[TIMEFRAME] != pIndicators->executionRateTF) //it means 15-25M		
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_secondary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_secondary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_secondary;
		shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

		//if (timeInfo1.tm_min % pIndicators->executionRateTF >= 3)
		//	return SUCCESS;
	}
	else
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_primary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_primary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_primary;
		shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	}
	return shift1Index;
}

double getCMFVolume(int index, int periods, int shift)
{
	double cmf_volume = 0.0;

	double dN_Sum = 0.0;
	double volume_sum = 0.0;
	double high, low, close, volume,open;
	int i = 0;
	for (i = shift; i< periods + shift; i++)
	{
		volume = iVolume(index, i);
		high = iHigh(index, i);
		low = iLow(index, i);
		close = iClose(index, i);
		open = iOpen(index, i);

		volume_sum += volume;
		if (high-low >0)
			dN_Sum += volume * ((close - low)-(high - close)) / (high - low);			
	}
	cmf_volume = dN_Sum / volume_sum;
	return cmf_volume;
}

double getCMFVolumeGap(int index, int fast_period, int slow_period, int shift)
{
	double	cmfVolumes[20] = {0.0};

	double cmf_gap = 0.0;
	double high, low, close, volume, open;
	int i = 0;
	double sum = 0.0;

	TA_RetCode taRetCode;
	int        outBegIdx, outNBElement;
	double	   fast_ma,slow_ma;	

	for (i = 0; i< 20; i++)
	{
		volume = iVolume(index, i);
		high = iHigh(index, i);
		low = iLow(index, i);
		close = iClose(index, i);
		open = iOpen(index, i);
				
		if (high - low >0)
			cmfVolumes[i] = fabs(volume * ((close - open) / (high - low)));
			//cmfVolumes[i] = abs(volume * ((close - low) - (high - close)) / (high - low));
	}
	
	//TA_MA(1, 19, cmfVolumes, fast_period, TA_MAType_SMA, &outBegIdx, &outNBElement, &fast_ma);
	//TA_MA(1, 19, cmfVolumes, slow_period, TA_MAType_SMA, &outBegIdx, &outNBElement, &slow_ma);

	for (i = shift; i < fast_period + shift; i++)
	{
		sum += cmfVolumes[i];
	}

	fast_ma = sum / fast_period;

	sum = 0.0;
	for (i = shift; i < slow_period + shift; i++)
	{
		sum += cmfVolumes[i];
	}

	slow_ma = sum / slow_period;

	cmf_gap = fast_ma / slow_ma - 1;
	return cmf_gap;
}



// handleTradeExits is implemented in OrderManagement.c - removed duplicate

// pParams->orderInfo[0]
void traceLatestOpenStopLoss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double minTP, double traceSL)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int count, minGap, hourGap;
	time_t currentTime, openTime;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	struct tm timeInfo1, timeInfo2;
	int  execution_tf = (int)pParams->settings[TIMEFRAME];
	double high, low, takePrice,stopLoss;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (pParams->orderInfo[0].ticket != 0 && pParams->orderInfo[0].isOpen == TRUE)
	{
		openTime = pParams->orderInfo[0].openTime;
		safe_gmtime(&timeInfo2, openTime);

		minGap = (int)((timeInfo1.tm_min - timeInfo2.tm_min) / execution_tf + 0.5);
		hourGap = (timeInfo1.tm_hour - timeInfo2.tm_hour) * (60 / execution_tf);
		count = hourGap + minGap;

		if (count > 1 && getHighLowEasy(B_PRIMARY_RATES, shift1Index, count, &high, &low) == SUCCESS)
		{
			if (pParams->orderInfo[0].type == BUY)
			{
				// ��ʼ����ֹ��ֹӯ��
				if (pBase_Indicators->dailyTrend_Phase != RANGE_PHASE && high - pParams->orderInfo[0].openPrice >= minTP)
				{

					takePrice = pParams->orderInfo[0].takeProfit - pParams->bidAsk.ask[0];
					//stopLoss = pParams->bidAsk.ask[0] - (high - traceSL);
					//ƽ����
					stopLoss = pParams->bidAsk.ask[0] - pParams->orderInfo[0].openPrice;

					logWarning("System InstanceID = %d, BarTime = %s, starting tracing SL for trend orders. dailyTrend_Phase =%ld,floating profit = %lf,takePrice=%lf,stopLoss=%lf",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->dailyTrend_Phase, high - pParams->bidAsk.ask[0], takePrice, stopLoss);

					if (takePrice > 0 && stopLoss> 0)
						modifyTradeEasy(BUY, pParams->orderInfo[0].ticket, stopLoss, takePrice);
					
				}
			}

			if (pParams->orderInfo[0].type == SELL)
			{
				if (pBase_Indicators->dailyTrend_Phase != RANGE_PHASE && pParams->orderInfo[0].openPrice - low >= minTP)
				{
					takePrice = pParams->bidAsk.bid[0] - pParams->orderInfo[0].takeProfit;
					//stopLoss = low + traceSL - pParams->bidAsk.bid[0];
					//ƽ����
					stopLoss = pParams->orderInfo[0].openPrice - pParams->bidAsk.bid[0];

					logWarning("System InstanceID = %d, BarTime = %s, starting tracing SL for trend orders. dailyTrend_Phase =%ld,floating profit = %lf,takePrice=%lf,stopLoss=%lf",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->dailyTrend_Phase, pParams->bidAsk.bid[0] - low, takePrice, stopLoss);

					if (takePrice > 0 && stopLoss> 0)
						modifyTradeEasy(SELL, pParams->orderInfo[0].ticket, stopLoss, takePrice);
				}
			}
		}
	}
}

BOOL isNextdayMACDPostiveBar2(StrategyParams* pParams, int orderIndex,int startShift)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime, orderOpenTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";	
	BOOL result = TRUE;
	double preDayOpen, preDayOpen1, preDayClose, preDayClose1, preDayRange, preDayRange1;
	double diffHours;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	orderOpenTime = pParams->orderInfo[orderIndex].openTime;
	safe_gmtime(&timeInfo2, orderOpenTime);

	preDayOpen = iOpen(B_DAILY_RATES, startShift);
	preDayClose = iClose(B_DAILY_RATES, startShift);
	preDayRange = preDayOpen - preDayClose;

	preDayOpen1 = iOpen(B_DAILY_RATES, startShift + 1);
	preDayClose1 = iClose(B_DAILY_RATES, startShift + 1);
	preDayRange1 = preDayOpen1 - preDayClose1;

	if (timeInfo2.tm_wday == 5)
	{		
		orderOpenTime += 24 * 2 * 60 * 60;	
		safe_gmtime(&timeInfo2, orderOpenTime);
	}
	
	diffHours = difftime(currentTime, orderOpenTime) / (60 * 60);

	//�۸����෴
	//�������һ��
	if (
		((timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_yday == timeInfo2.tm_yday && timeInfo1.tm_hour == 23 && diffHours > 20)
		|| ((timeInfo1.tm_year != timeInfo2.tm_year || timeInfo1.tm_yday != timeInfo2.tm_yday) && diffHours > 22 && diffHours < 26)
		)	&& 
		preDayRange * preDayRange1 < 0
		//&& fabs(preDayRange) > fabs(preDayRange1)/4
		)
	{
		result = FALSE;
	}

	return result;
}

BOOL isNextdayMACDPostiveBar(int startShift)
{
	BOOL result = TRUE;
	trend trend, trend1;
	//double atr = iAtr(B_DAILY_RATES, 1, startShift), atr1 = iAtr(B_DAILY_RATES, 1, startShift+1);
	double preDayOpen, preDayOpen1, preDayClose, preDayClose1,preDayRange,preDayRange1;
	preDayOpen = iOpen(B_DAILY_RATES, startShift);
	preDayClose = iClose(B_DAILY_RATES, startShift);
	preDayRange = preDayOpen - preDayClose;

	//if (fabs(preDayRange) < atr* 0.1)
	//	trend = RANGE;	
	//else if (preDayRange > 0)
	//	trend = UP;
	//else
	//	trend = DOWN;


	preDayOpen1 = iOpen(B_DAILY_RATES, startShift+1);
	preDayClose1 = iClose(B_DAILY_RATES, startShift+1);
	preDayRange1 = preDayOpen1 - preDayClose1;

	//if (fabs(preDayRange1) < atr1* 0.1)
	//	trend1 = RANGE;
	//else if (preDayRange1 > 0)
	//	trend1 = UP;
	//else
	//	trend1 = DOWN;

	//�۸����෴
	//�������һ��
	//it need to have a range 
	if (
		preDayRange * preDayRange1 < 0
		//&& fabs(preDayRange) > fabs(preDayRange1)/4
		)
	{
		result = FALSE;
	}

	return result;	
}

BOOL XAUUSD_IsKeyDate(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	time_t keyDates[1000] = { 0 };
	int i = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	readXAUUSDKeyNewsDateFile(keyDates);

	for (i = 0; i < 1000; i++)
	{
		safe_gmtime(&timeInfo2, keyDates[i]);
		if (timeInfo1.tm_year == timeInfo2.tm_year && timeInfo1.tm_mon == timeInfo2.tm_mon && timeInfo1.tm_mday == timeInfo2.tm_mday) // same date
		{
			return TRUE;
		}

		if (keyDates[i] == 0)
			return FALSE;
	}

	return FALSE;

}

int weeklyTrend4HSwingSignal(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;

	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double movement = 0;
	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	movement = fabs(high_4H - low_4H);
	if (pIndicators->atr_euro_range == 0)
		pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;

	logWarning("System InstanceID = %d, BarTime = %s, high_4H %lf�� low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


	if (movement >= pIndicators->atr_euro_range) // 100 pips
	{
		if (fabs(high_4H - close_4H) < movement / 3)
		{
			trend_KeyK = 1;
		}
		if (fabs(low_4H - close_4H) < movement / 3)
		{
			trend_KeyK = -1;
		}
	}

	if (trend_MA > 0 || trend_KeyK == 1)
		trend_4H = 1;
	if (trend_MA < 0 || trend_KeyK == -1)
		trend_4H = -1;

	return trend_4H;
}

// modifyOrders is implemented in OrderManagement.c - removed duplicate
#if 0
AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double stopLoss, double takePrice)
{
	int tpMode = 0;
	int shift0Index;
	time_t currentTime;
	
	shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	if ((int)parameter(AUTOBBS_TREND_MODE) == 33 || (int)parameter(AUTOBBS_TREND_MODE) == 2 
		|| (int)parameter(AUTOBBS_TREND_MODE) == 3
		|| (int)parameter(AUTOBBS_TREND_MODE) == 34) //MACD BEILI, need to move take profit price
	{
		takePrice = pIndicators->takePrice;

		logInfo("ModifyOrders: takePrice = %lf,takePrice=%lf\n", takePrice, pIndicators->takePrice);

	}
	

	if ((int)parameter(AUTOBBS_TREND_MODE) == 15 )
	{

		//traceLatestOpenStopLoss(pParams, pIndicators, pBase_Indicators, 3, 0.5);
	}
	else
	{
		if ((int)pParams->settings[TIMEFRAME] >= 60 && isNewDay(pParams, currentTime))
		{
			setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
			tpMode = 1;
		}


		if (orderType == BUY)
		{
			if (totalOpenOrders(pParams, BUY) > 0)
			{
				if ((int)parameter(AUTOBBS_TREND_MODE) == 5) //Day Trading, override the stop loss to primary bbs on the new day.
				{
					//stopLoss2 = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_primary) + pIndicators->adjust;

					modifyTradeEasy_DayTrading(BUY, -1, stopLoss, pIndicators->bbsStopPrice_primary, -1, tpMode, currentTime, pIndicators->adjust, pIndicators->stopMovingBackSL);
				}
				else
					modifyTradeEasy_new(BUY, -1, stopLoss, takePrice, tpMode, pIndicators->stopMovingBackSL); // New day TP change as
			}
		}

		if (orderType == SELL)
		{
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				if ((int)parameter(AUTOBBS_TREND_MODE) == 5) //Day Trading, override the stop loss to primary bbs on the new day.
				{
					//stopLoss2 = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_primary) + pIndicators->adjust;
					modifyTradeEasy_DayTrading(SELL, -1, stopLoss, pIndicators->bbsStopPrice_primary, -1, tpMode, currentTime, pIndicators->adjust, pIndicators->stopMovingBackSL);
				}
				//else if ((int)parameter(AUTOBBS_TREND_MODE) == 13) //�ڿ��ֺ�TP�ǵ͵㵽�ߵ�ľ��롣
				//{
				//	takePrice = adjustTakePrice_Weekly_Swing_Easy(B_HOURLY_RATES, pBase_Indicators->pWeeklyATR / 3);
				//	modifyTradeEasy_new(SELL, -1, stopLoss, takePrice, tpMode);
				//}
				else if ((int)parameter(AUTOBBS_TREND_MODE) != 15)
					modifyTradeEasy_new(SELL, -1, stopLoss, takePrice, tpMode, pIndicators->stopMovingBackSL); // New day TP change as
			}
		}
	}

	return SUCCESS;

}
#endif

AsirikuyReturnCode getHighestHourlyClosePrice(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int rate_index, int orderIndex, double * highPrice, double * lowPrice)
{
	
	int  shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int  openBar = 0;
	int count;
	time_t currentTime;
	struct tm timeInfo1; 
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	int seconds;
	
	*highPrice = -999999.0;
	*lowPrice = 999999.0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (orderIndex >=0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		count = (int)difftime(currentTime, pParams->orderInfo[orderIndex].openTime) / (60 * 60);

		openBar = shift1Index - count;

		if (count >= 1)
			iSRLevels_close(pParams, pBase_Indicators, rate_index, shift1Index, 2 * count, highPrice, lowPrice);
		else
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}

AsirikuyReturnCode getHighLowPrice(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int rate_index, int timeFrame,int orderIndex, double * highPrice, double * lowPrice)
{

	int  shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int  openBar = 0;
	int count;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	*highPrice = -999999.0;
	*lowPrice = 999999.0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL))
	{		
		count = (int)difftime(currentTime, pParams->orderInfo[orderIndex].openTime) / timeFrame;

		openBar = shift1Index - count;

		if (count >= 1)
			iSRLevels(pParams, pBase_Indicators, rate_index, shift1Index, count, highPrice, lowPrice);
		else
			return FALSE;
	}
	else
		return FALSE;

	return TRUE;
}


AsirikuyReturnCode addMoreOrdersOnLongTermTrend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int oldestOpenOrderIndex)
{

	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (pParams->orderInfo[oldestOpenOrderIndex].type == BUY){

		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		if (
			(preLow < pBase_Indicators->dailyPivot && preClose >  pBase_Indicators->dailyPivot ||
			(timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 5)) &&
			pIndicators->entryPrice - pIndicators->stopLoss > pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
			!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)){
			pIndicators->executionTrend = 1;
			pIndicators->stopLossPrice = pIndicators->entryPrice - pIndicators->stopLoss;
			pIndicators->entrySignal = 1;
			return SUCCESS;
		}
	}

	if (pParams->orderInfo[oldestOpenOrderIndex].type == SELL
		){

		pIndicators->entryPrice = pParams->bidAsk.bid[0];		
		if ((preHigh > pBase_Indicators->dailyPivot && preClose < pBase_Indicators->dailyPivot ||
			(timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 5)) &&
			pIndicators->entryPrice + pIndicators->stopLoss < pParams->orderInfo[oldestOpenOrderIndex].stopLoss &&
			!isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)){
			pIndicators->executionTrend = -1;
			pIndicators->stopLossPrice = pIndicators->entryPrice + pIndicators->stopLoss;
			pIndicators->entrySignal = -1;
			return SUCCESS;
		}
	}

	return SUCCESS;
}

BOOL XAUUSD_not_full_trading_day(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime, adjustTime;
	struct tm timeInfo1, adjustTimeInfo;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int secondsPerWeek = 7 * 24 * 60 * 60;
	BOOL isFilter = FALSE;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	//Martin holiday 3st week Monday on Jan
	if (timeInfo1.tm_mon == 0 && timeInfo1.tm_wday == 1
		&& timeInfo1.tm_mday >= 2 * 7 && timeInfo1.tm_mday <= 3 * 7)
	{
		//adjustTime = currentTime - 3 * secondsPerWeek;
		//safe_gmtime(&adjustTimeInfo, adjustTime);
		//if (adjustTimeInfo.tm_mon == 11)
		{
			strcpy(pIndicators->status, "Filter Martin holiday.\n\n");
			isFilter = TRUE;
		}

	}
	//Washington holiday 3st Monday
	if (timeInfo1.tm_mon == 1 && timeInfo1.tm_wday == 1
		&& timeInfo1.tm_mday >= 2 * 7 && timeInfo1.tm_mday <= 3 * 7)
	{
		strcpy(pIndicators->status, "Filter Washington holiday.\n\n");
		isFilter = TRUE;
	}
	//Good Friday holiday from KeyDate file
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators))
	{
		strcpy(pIndicators->status, "Filter GoodFriday holiday or adjusted US Independent day.\n\n");
		isFilter = TRUE;
	}
	//Memorial holiday Last Monday
	if (timeInfo1.tm_mon == 4 && timeInfo1.tm_wday == 1
		&& timeInfo1.tm_mday >= 31 - 7 && timeInfo1.tm_mday <= 31)
	{
		strcpy(pIndicators->status, "Filter Memorial holiday.\n\n");
		isFilter = TRUE;
	}
	//US Independent day holiday on 04/07
	if (timeInfo1.tm_mon == 6 && timeInfo1.tm_mday == 4)
	{
		strcpy(pIndicators->status, "Filter US Independent day .\n\n");
		isFilter = TRUE;
	}
	//Labour holiday 1st Monday
	if (timeInfo1.tm_mon == 8 && timeInfo1.tm_wday == 1
		&& timeInfo1.tm_mday >= 1 && timeInfo1.tm_mday <= 7)
	{
		strcpy(pIndicators->status, "Filter Labour holiday.\n\n");
		isFilter = TRUE;
	}
	//Thanksgiving holiday 4st Thursday on NOV
	if (timeInfo1.tm_mon == 10 && timeInfo1.tm_wday == 4
		&& timeInfo1.tm_mday >= 3 * 7 && timeInfo1.tm_mday <= 4 * 7)
	{
		strcpy(pIndicators->status, "Filter thanksgiving holiday.\n\n");
		isFilter = TRUE;
	}
	//filter christmas eve and new year eve
	if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
	{
		strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n\n");
		isFilter = TRUE;
	}

	return isFilter;
}
