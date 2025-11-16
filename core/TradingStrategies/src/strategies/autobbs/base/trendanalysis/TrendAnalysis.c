/*
 * Trend Analysis Module
 * 
 * Provides functions for analyzing trends using various methods:
 * - High/Low trend analysis (iTrend_HL)
 * - Moving Average trend analysis (iTrend_MA, getMATrend)
 * - Three Rules trend analysis (iTrend3Rules)
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/base/trendanalysis/TrendAnalysis.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE
AsirikuyReturnCode iTrend_HL_preDays(int ratesArrayIndex, int *trend,int preDays,int index)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	preHigh1 = iHigh(ratesArrayIndex, 1 + preDays - index);
	preHigh2 = iHigh(ratesArrayIndex, 2 + preDays - index);

	preClose1 = iClose(ratesArrayIndex, 1 + preDays - index);
	preClose2 = iClose(ratesArrayIndex, 2 + preDays - index);

	preLow1 = iLow(ratesArrayIndex, 1 + preDays - index);
	preLow2 = iLow(ratesArrayIndex, 2 + preDays - index);

	if (preHigh1 > preHigh2 &&
		//preLow1 > preLow2 &&
		preClose1 > preClose2)
	{
		*trend = UP_WEAK;
	}

	if (//preHigh1 < preHigh2 &&
		preLow1 < preLow2 &&
		preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}

	return SUCCESS;
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
//����Ƿ�����ߵ���ߣ��͵���ߣ���������Ҳ�Ǹ��ߣ��Ǿ���UP
AsirikuyReturnCode iTrend_HL(int ratesArrayIndex,int *trend,int index)
{
	double preHigh1, preHigh2;
	double preLow1, preLow2;
	double preClose1, preClose2;

	*trend = RANGE;
	preHigh1 = iHigh(ratesArrayIndex, 1 - index);
	preHigh2 = iHigh(ratesArrayIndex, 2 - index);
	
	preClose1 = iClose(ratesArrayIndex, 1 - index);
	preClose2 = iClose(ratesArrayIndex, 2 - index);
	
	preLow1 = iLow(ratesArrayIndex, 1 - index);
	preLow2 = iLow(ratesArrayIndex, 2 - index);
	
	if (preHigh1 > preHigh2 &&
		//preLow1 > preLow2 &&
		preClose1 > preClose2 )
	{
		*trend = UP_WEAK;
	}
	
	if (//preHigh1 < preHigh2 &&
		preLow1 < preLow2 &&
		preClose1 < preClose2)
	{
		*trend = DOWN_WEAK;
	}	
		
	return SUCCESS;
}

int getMATrend_SignalBase(int rateShort,int rateLong,int ratesArrayIndex,int maxBars)
{
	int i = 0, j, maTrend, maTrend_Prev;
	double adjust = iAtr(ratesArrayIndex, 20, 1);

	maTrend = getMATrendBase(rateShort,rateLong,adjust, ratesArrayIndex, 1);
	if (maTrend == 0)
		//if (fabs(maTrend) < 2)
		return 0;

	for (i = 1; i < maxBars; i++)
	{
		maTrend_Prev = getMATrendBase(rateShort,rateLong,adjust, ratesArrayIndex, i + 1);
		
		if (maTrend > 0 && maTrend_Prev < 0)
		{			
			return 1;
		}

		if (maTrend < 0 && maTrend_Prev > 0)
		{
			return -1;
		}
	}
		
	return 0;
}

int getMATrend_Signal(int ratesArrayIndex)
{
	return getMATrend_SignalBase(50, 200, ratesArrayIndex,24);
}

int getMATrendBase(int rateShort,int rateLong,double iATR, int ratesArrayIndex, int index)
{
	double maShort, maLong;
	int trend;

	maShort = iMA(3, ratesArrayIndex, rateShort, index);
	maLong = iMA(3, ratesArrayIndex, rateLong, index);

	if (maShort > maLong)
	{
		trend = 1;
		if (maShort - maLong >= iATR)
			trend = 2;
	}
	else if (maShort < maLong)
	{
		trend = -1;
		if (maLong - maShort >= iATR)
			trend = -2;
	}
	else
		trend = 0;
	return trend;
}

int getMATrend(double iATR, int ratesArrayIndex, int index)
{
	return getMATrendBase(50, 200, iATR, ratesArrayIndex, index);
}

// Daily: 1H 200M , 50M
// Weekly: 1H 200M, 50M
AsirikuyReturnCode iTrend_MA(double iATR,int ratesArrayIndex,int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, ratesArrayIndex, 50, 1);
	ma200M = iMA(3, ratesArrayIndex, 200, 1);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

int iTrendMA_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex,int signal)
{
	int trend[100] = { 0 };
	int i = 0;
	double ma50M, ma200M;	
	int truningIndex = 100;

	for (i = 1; i < 100; i++)
	{
		trend[i] = getMATrend(iAtr(ratesArrayIndex, 20, 1), ratesArrayIndex, i);
		if ( (signal > 0 && trend[i] != UP_NORMAL) ||
			(signal < 0 && trend[i] != DOWN_NORMAL))
		{
			truningIndex = i;
			break;
		}
	}

	return truningIndex;

}


AsirikuyReturnCode iTrend_MA_WeeklyBar_For4H(double iATR, int *trend)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, B_WEEKLY_RATES, 1, 1);
	ma200M = iMA(3, B_WEEKLY_RATES, 6, 1);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend_MA_DailyBar_For1H(double iATR, int *trend,int index)
{
	double ma50M, ma200M;

	*trend = RANGE;
	ma50M = iMA(3, B_DAILY_RATES, 2, 1-index);
	ma200M = iMA(3, B_DAILY_RATES, 8, 1 - index);
	if (ma50M - ma200M > 0.5 * iATR)
		*trend = UP_NORMAL;

	if (ma200M - ma50M > 0.5 * iATR)
		*trend = DOWN_NORMAL;

	return SUCCESS;
}

AsirikuyReturnCode iTrend3Rules_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int        outBegIdx, outNBElement;
	double	  boxHigh[100] = { 0 }, boxLow[100] = { 0 };
	int trend[100] = { 0 };
	int i = 0;

	retCode = TA_MIN(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift - 1, shift1Index - 1, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	for (i = 0; i < outNBElement; i++)
	{
		// Check close price. 
		trend[i] = RANGE;
		if (boxHigh[i] == 0 || boxLow[i] == 0)
			break;

		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) > boxHigh[i]) //�ܹ������ϡ� c3 > max(h1,h2) or c3 > max(c1,c2) && h3>max(h1,h2) && l3>max(l1,l2)
			trend[i] = UP_NORMAL;

		if (iClose(ratesArrayIndex, shift1Index - (i + outBegIdx)) < boxLow[i]) //�ܹ������ϡ�
			trend[i] = DOWN_NORMAL;

		if (i - 1 >= 0 && trend[i - 1] == UP && trend[i] == RANGE)
			trend[i] = RANGE;

		if (i - 1 >= 0 && trend[i - 1] == DOWN && trend[i] == RANGE)
			trend[i] = RANGE;

		*pTrend = trend[i];
	}	

	return SUCCESS;
	
}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend3Rules_preDays(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend,int preDays,int index)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1 - preDays;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2 - preDays;
	int       outBegIdx, outNBElement;
	double	  boxHigh, boxLow;

	retCode = TA_MIN(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	*pTrend = RANGE;
	if (iClose(ratesArrayIndex, 1 - index) > boxHigh) //�ܹ������ϡ� c3 > max(h1,h2)
		*pTrend = UP;

	if (iClose(ratesArrayIndex, 1 - index) < boxLow) //�ܹ������ϡ�
		*pTrend = DOWN;

	return SUCCESS;

}

/*
If index = 1 -> current day (EOD)
If index == 0 -> previous day (SOD)
*/
AsirikuyReturnCode iTrend3Rules(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend,int index)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int       outBegIdx, outNBElement;
	double	  boxHigh, boxLow;	

	retCode = TA_MIN(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, &boxLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index - 1 + index, shift1Index - 1 + index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, &boxHigh);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}

	*pTrend = RANGE;
	if (iClose(ratesArrayIndex, 1 - index) > boxHigh) //�ܹ������ϡ� c3 > max(h1,h2)
			*pTrend = UP;

	if (iClose(ratesArrayIndex, 1 - index) < boxLow) //�ܹ������ϡ�
			*pTrend = DOWN;

	return SUCCESS;

}
