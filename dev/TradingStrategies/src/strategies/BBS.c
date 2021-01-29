/*
* 15M time frame order management strategy
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum trend_t
{
	UP = 1,
	DOWN = -1
}trend;

typedef enum additionalSettings_t
{
	DAILY_TREND = ADDITIONAL_PARAM_1,
	BBS_PERIOD = ADDITIONAL_PARAM_2,
	BBS_DEVIATION = ADDITIONAL_PARAM_3,
	BBS_ADJUSTPOINTS = ADDITIONAL_PARAM_4,
	BBS_TP_MODE = ADDITIONAL_PARAM_5, // BBS_TP_MODE = 0 (XAUUSD): The last trade will be no TP; BBS_TP_MODE=1: the last trade will be 1:3 
	BBS_SL_MODE = ADDITIONAL_PARAM_6 //  BBS_SL_MODE = 0 default. If 1, use 1H BBS Stop loss price.If 2, use 4H BBS Stop loss price, 3: 2 days H/L
} AdditionalSettings;

typedef struct indicators_t
{
	int bbsTrend;
	int dailyTrend;
	double bbsStopPrice;
	int bbsIndex;
	double dailyATR;
	double primaryATR;
	double dailyPivot;
	double dailyS1;
	double dailyR1;
	double dailyS2;
	double dailyR2;
	double dailyS3;
	double dailyR3;
	double preHigh;
	double preLow;
	double preClose;
	double adjust;
	double twoDaysHigh;
	double twoDaysLow;
} Indicators;

AsirikuyReturnCode runBBS(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runBBS() failed. pParams = NULL");
		return NULL_POINTER;
	}

	loadIndicators(pParams, &indicators);

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	setUIValues(pParams, &indicators);

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, DailyTrend = %ld,BBSTrend=%ld,BBStopPrice=%lf, BBSIndex = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.dailyTrend, indicators.bbsTrend, indicators.bbsStopPrice, indicators.bbsIndex);
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preHigh = %lf,preLow=%lf, preClose = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.preHigh, indicators.preLow, indicators.preClose);
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Pivot = %lf,S1=%lf,R1=%lf, S2 = %lf, R2=%lf£¬ S3 = %lf, R3=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.dailyPivot, indicators.dailyS1, indicators.dailyR1, indicators.dailyS2, indicators.dailyR2, indicators.dailyS3, indicators.dailyR3);

	//if (strcmp(timeString, "15/08/17 20:45") == 0)
	//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, "hit a point");

	returnCode = handleTradeExits(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runBBS->handleTradeExits()", returnCode);
	}

	returnCode = handleTradeEntries(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runBBS->handleTradeEntries()", returnCode);
	}

	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	int        dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;
	int     newTrend,newIndex;
	double newStopLoss;
	pIndicators->dailyATR = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pIndicators->primaryATR = iAtr(PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	pIndicators->dailyTrend = (int)parameter(DAILY_TREND);
	iSRLevels(pParams, DAILY_RATES, 2, &(pIndicators->twoDaysHigh), &(pIndicators->twoDaysLow));

	switch ((int)parameter(BBS_SL_MODE))
	{
		case 1:
			iBBandStop(HOURLY_RATES, (int)parameter(BBS_PERIOD), (int)parameter(BBS_DEVIATION), &newTrend, &newStopLoss, &newIndex);

			//same trend
			if (pIndicators->bbsTrend == newTrend)
			{
				pIndicators->bbsStopPrice = newStopLoss;
			}
			break;
		case 2:
			//Use 4H BBS Stop loss price.
			iBBandStop(FOURHOURLY_RATES, (int)parameter(BBS_PERIOD), (int)parameter(BBS_DEVIATION), &newTrend, &newStopLoss, &newIndex);

			//same trend
			if (pIndicators->bbsTrend == newTrend)
			{
				pIndicators->bbsStopPrice = newStopLoss;
			}
			break;
		case 3:
			if (pIndicators->dailyTrend == 1)
				pIndicators->bbsStopPrice = pIndicators->twoDaysLow;			
			if (pIndicators->dailyTrend == -1)
				pIndicators->bbsStopPrice = pIndicators->twoDaysHigh;
			break;
		default:
			iBBandStop(PRIMARY_RATES, (int)parameter(BBS_PERIOD), (int)parameter(BBS_DEVIATION), &pIndicators->bbsTrend, &pIndicators->bbsStopPrice, &pIndicators->bbsIndex);
			break;
	}
	
	

	pIndicators->preHigh = iHigh(DAILY_RATES, 1);
	pIndicators->preLow = iLow(DAILY_RATES, 1);
	pIndicators->preClose = iClose(DAILY_RATES, 1);

	if (validateHourlyBarsEasy(pParams, PRIMARY_RATES, HOURLY_RATES) > 0 ||
		validateDailyBarsEasy(pParams, PRIMARY_RATES, DAILY_RATES) > 0 ||		
		validateCurrentTimeEasy(pParams, PRIMARY_RATES) > 0)
		return ERROR_IN_RATES_RETRIEVAL;

	//if (!validateDailyBarsEasy(pParams, PRIMARY_RATES,HOURLY_RATES))
	//	reWorkoutPivotFromPrimaryRate(pParams, pIndicators);

	pIndicators->dailyPivot = (pIndicators->preHigh + pIndicators->preLow + pIndicators->preClose) / 3;
	pIndicators->dailyS1 = 2 * pIndicators->dailyPivot - pIndicators->preHigh;
	pIndicators->dailyR1 = 2 * pIndicators->dailyPivot - pIndicators->preLow;
	pIndicators->dailyS2 = pIndicators->dailyPivot - (pIndicators->preHigh - pIndicators->preLow);
	pIndicators->dailyR2 = pIndicators->dailyPivot + (pIndicators->preHigh - pIndicators->preLow);
	pIndicators->dailyS3 = pIndicators->preLow - 2 * (pIndicators->preHigh - pIndicators->dailyPivot);
	pIndicators->dailyR3 = pIndicators->preHigh + 2 * (pIndicators->dailyPivot - pIndicators->preLow);

	pIndicators->adjust = (double)parameter(BBS_ADJUSTPOINTS);

	return returnCode;
}

static BOOL reWorkoutPivotFromPrimaryRate(StrategyParams* pParams, Indicators* pIndicators)
{
	time_t currentTime;
	struct tm timeInfo;
	char  timeString[MAX_TIME_STRING_SIZE] = "";	
	int   shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
	int   shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	int   startShift, startHourShift, start15MShift, missingbarAdjust = 0, timeFrame,shiftNo;
	int outBegIdx, outNBElement;
	TA_RetCode retCode;
	double low, high, close;
		
	currentTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo, currentTime);
	safe_timeString(timeString, currentTime);

	// If XAU missing 0:00 hourly bar
	if (strstr(pParams->tradeSymbol, "XAU") != NULL || strstr(pParams->tradeSymbol, "XTI") != NULL || strstr(pParams->tradeSymbol, "XAG") != NULL || strstr(pParams->tradeSymbol, "XPD") != NULL)
		missingbarAdjust = 1;

	timeFrame = (int)pParams->settings[TIMEFRAME];

	startHourShift = timeInfo.tm_hour - missingbarAdjust;

	start15MShift = (int)(timeInfo.tm_min / timeFrame);

	startShift = (int)(startHourShift * MINUTES_PER_HOUR / timeFrame) + start15MShift + 1;
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"current time = %s, startShift=%d,start15MShift=%d,startShift=%d", timeString, startHourShift, start15MShift, startShift);
	shiftNo = (int)((24 - missingbarAdjust) * MINUTES_PER_HOUR / timeFrame) ;

	retCode = TA_MIN(shift0Index - startShift, shift0Index - startShift, pParams->ratesBuffers->rates[PRIMARY_RATES].low, shiftNo, &outBegIdx, &outNBElement, &low);
	if (retCode != TA_SUCCESS)
	{
		logTALibError("TA_MIN()", retCode);
		return FALSE;
	}

	retCode = TA_MAX(shift0Index - startShift, shift0Index - startShift, pParams->ratesBuffers->rates[PRIMARY_RATES].high, shiftNo, &outBegIdx, &outNBElement, &high);

	if (retCode != TA_SUCCESS)
	{
		logTALibError("TA_MAX()", retCode);
		return FALSE;
	}

	close = iClose(PRIMARY_RATES, startShift);

	if (pIndicators->preHigh != high || pIndicators->preLow != low || pIndicators->preClose != close)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Pre daily bar mismatched: current time = %s, preHigh=%lf,high=%lf,PreLow=%lf,low=%lf,PreClose=%lf,close=%lf", timeString, pIndicators->preHigh, high, pIndicators->preLow, low, pIndicators->preClose, close);
		pIndicators->preHigh = high;
		pIndicators->preLow = low;
		pIndicators->preClose = close;
	}
	
	return TRUE;

}

////Add validation for preHigh, preLow and preClose from primary bars, 15M
//// Note: XAUUSD missing one hourly bar (00£º00£© every day.
//static BOOL validateDailyBars(StrategyParams* pParams, Indicators* pIndicators)
//{
//	time_t currentTime, currentDailyTime;
//	struct tm timeInfo,dailyTimeInfo;
//	char  timeString[MAX_TIME_STRING_SIZE] = "";
//	char  dailyTimeString[MAX_TIME_STRING_SIZE] = "";
//	int   shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;	
//	int   shiftDaily0Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 1;
//		
//	//Validate daily bars first
//	currentDailyTime = pParams->ratesBuffers->rates[DAILY_RATES].time[shiftDaily0Index];
//	safe_gmtime(&dailyTimeInfo, currentDailyTime);
//	safe_timeString(dailyTimeString, currentDailyTime);
//	
//	currentTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index];
//	safe_gmtime(&timeInfo, currentTime);	
//	safe_timeString(timeString, currentTime);
//	
//	
//	if (dailyTimeInfo.tm_yday != timeInfo.tm_yday)
//	{
//		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Current day not matached: current time = %s, current daily time =%s", timeString, dailyTimeString);
//
//		return FALSE;
//	}
//	return TRUE;
//
//
//}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators)
{
	addValueToUI("DailyTrend", pIndicators->dailyTrend);
	addValueToUI("BBSTrend", pIndicators->bbsTrend);
	addValueToUI("BBSStopPrice", pIndicators->bbsStopPrice);		
	addValueToUI("DailyPivot", pIndicators->dailyPivot);
	addValueToUI("DailyS1", pIndicators->dailyS1);
	addValueToUI("DailyR1", pIndicators->dailyR1);
	addValueToUI("DailyATR", pIndicators->dailyATR);
	addValueToUI("PrimaryATR", pIndicators->primaryATR);
	addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
	
	return SUCCESS;
}

static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, OrderType orderType,double stopLoss,double takePrice)
{
	int shift0Index, shift1Index;
	time_t currentTime;
	time_t virtualOrderEntryTime;
	struct tm timeInfo1, timeInfo2;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	char       timeString2[MAX_TIME_STRING_SIZE] = "";

	int tpMode = 0;

	if ((int)pParams->settings[TIMEFRAME] >= MINUTES_PER_HOUR) // 1H above time frame short term trade
	{
		virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

		shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
		shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
		currentTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index];
		safe_gmtime(&timeInfo1, currentTime);
		safe_gmtime(&timeInfo2, virtualOrderEntryTime);

		safe_timeString(timeString1, currentTime);
		safe_timeString(timeString2, virtualOrderEntryTime);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Last order update time = %s,current time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime, currentTime) / 3600);

		if (virtualOrderEntryTime != -1 && timeInfo1.tm_mday != timeInfo2.tm_mday && timeInfo1.tm_min >= 15) // New day
		{
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Move to a new day. Should modify order TP.");

			setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

			tpMode = 1;
		}
	}

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
			modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
			modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	return SUCCESS;	
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators)
{
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	double	   stopLoss, takePrice = 0;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pIndicators = NULL");
		return NULL_POINTER;
	}


	// When the trade is created from 23:45 to 00:20, increase the 3 times of spread for cut over
	if ((hour() == 23 && minute() > 40) || (hour() == 00 && minute() < 20)) 
		pIndicators->adjust = 3 * pIndicators->adjust;
	
	if (pIndicators->bbsTrend == 1 )
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->bbsStopPrice + pIndicators->adjust);

		//Open 3 orders
		// Risk per order is 0.5%, total 3 orders risk = 1%
		// 1. No TP, SL = BBS Stop 0.25%
		// 2. R2/S2, SL = BBS Stop 0.5%			
		// 2. R3/S3, SL = BBS Stop 0.25%			
		// If the order is moved to a new day, need to consider to treat them as longer term order, new TP will be either break event or 200M(1H) or 2 days(H,L)
		// Should use internal TP 
		if (pIndicators->bbsIndex == shift1Index && pIndicators->dailyTrend == 1)
		{
			// If the open price is above R1, skip it.
			// If 15M BBS, use R1/S1 filter; if 1H BBS, use R2/S2 filter, idealy should be Weekly R1/S1
			if ((pParams->bidAsk.ask[0] <= pIndicators->dailyR2 && (int)pParams->settings[TIMEFRAME] >= 60) || (pParams->bidAsk.ask[0] <= pIndicators->dailyR1 && (int)pParams->settings[TIMEFRAME] < 60))
			{

				if ((int)parameter(BBS_TP_MODE) == 0)
				{
					takePrice = stopLoss;
					openSingleLongEasy(takePrice, stopLoss, 0, 1);
				}
				else
				{
					takePrice = stopLoss;
					openSingleLongEasy(takePrice, stopLoss, 0, 0.5);

					takePrice = 2* stopLoss;
					openSingleLongEasy(takePrice, stopLoss, 0, 0.5);
				}
			}

		}
		else
		{
			if (totalOpenOrders(pParams, BUY) > 0)
				modifyOrders(pParams, BUY, stopLoss, -1);					
		}
		
		
	}
	if (pIndicators->bbsTrend == -1)
	{		
		stopLoss = fabs(pIndicators->bbsStopPrice - pParams->bidAsk.bid[0] + pIndicators->adjust);

		if (pIndicators->bbsIndex == shift1Index && pIndicators->dailyTrend == -1)
		{
			 //If the open price is above R1, skip it.
			if ((pParams->bidAsk.bid[0] >= pIndicators->dailyS1 && (int)pParams->settings[TIMEFRAME] <60) || (pParams->bidAsk.bid[0] >= pIndicators->dailyS2 && (int)pParams->settings[TIMEFRAME] >=60))
			{				

				if ((int)parameter(BBS_TP_MODE) == 0)
				{
					takePrice = stopLoss;
					openSingleShortEasy(takePrice, stopLoss, 0, 1);
				}
				else
				{
					takePrice = stopLoss;
					openSingleShortEasy(takePrice, stopLoss, 0, 0.5);

					takePrice = 2* stopLoss;
					openSingleShortEasy(takePrice, stopLoss, 0, 0.5);
				}
							
			}
		}
		else
		{
			if (totalOpenOrders(pParams, SELL) > 0)
				modifyOrders(pParams, SELL, stopLoss, -1);
		}

		
	}
	return SUCCESS;
}

static AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int  shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pParams = NULL");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeExits() failed. pIndicators = NULL");
		return NULL_POINTER;
	}

	if (pIndicators->bbsTrend == 1 && pIndicators->bbsIndex == shift1Index)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}
	}

	if (pIndicators->bbsTrend == -1 && pIndicators->bbsIndex == shift1Index)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
	}
	return SUCCESS;
}

static AsirikuyReturnCode iSRLevels(StrategyParams* pParams, int ratesArrayIndex, int shift, double *pHigh, double *pLow)
{
	TA_RetCode retCode;
	int shift0Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[ratesArrayIndex].info.arraySize - 2;
	int outBegIdx, outNBElement;

	retCode = TA_MIN(shift1Index, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].low, shift, &outBegIdx, &outNBElement, pLow);
	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MIN()", retCode);
	}

	retCode = TA_MAX(shift1Index, shift1Index, pParams->ratesBuffers->rates[ratesArrayIndex].high, shift, &outBegIdx, &outNBElement, pHigh);

	if (retCode != TA_SUCCESS)
	{
		return logTALibError("TA_MAX()", retCode);
	}
	return SUCCESS;
}