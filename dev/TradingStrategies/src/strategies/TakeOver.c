/*
* Take Over manual orders strategy
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// trend_t is already defined in base.h, using that one

typedef enum additionalSettings_t
{
	BBS_PERIOD = ADDITIONAL_PARAM_1,
	BBS_DEVIATION = ADDITIONAL_PARAM_2,
	BBS_ADJUSTPOINTS = ADDITIONAL_PARAM_3,
	POSITION = ADDITIONAL_PARAM_4,
	DSL_TYPE = ADDITIONAL_PARAM_7
} AdditionalSettings;

typedef struct indicators_t
{
	double position;
	int bbsTrend;	
	double bbsStopPrice;
	int bbsIndex;
	double dailyATR;
	double preHigh;
	double preLow;
	double preClose;
	double pre2DaysHigh;
	double pre2DaysLow;
	double movingAverage200M;
	double buyStopLossPrice;
	double sellStopLossPrice;
	double adjust;
	int dsl_type;
} Indicators;

// Forward declarations
static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators);
static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators);
static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators, OrderType orderType);

typedef enum exitDslTypes_t
{
	EXIT_DSL_NONE = 0,
	EXIT_DSL_1DayHL = 1,
	EXIT_DSL_2DayHL = 2,
	EXIT_DSL_1HM200 = 3,
	EXIT_DSL_BBS = 4,
	EXIT_DSL_DailyATR = 5	
} dslTypes;

AsirikuyReturnCode runTakeOver(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	

	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] runTakeOver() failed. pParams = NULL\n");
		return NULL_POINTER;
	}

	loadIndicators(pParams, &indicators);	

	safe_timeString(timeString, pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index]);

	setUIValues(pParams, &indicators);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,BBSTrend=%ld,BBStopPrice=%lf, BBSIndex = %ld", 
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.bbsTrend, indicators.bbsStopPrice, indicators.bbsIndex);
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, preHigh = %lf,preLow=%lf, preClose = %lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.preHigh, indicators.preLow, indicators.preClose);
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, buySLP = %lf,sellSLP=%lf, DSL= %d",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.buyStopLossPrice, indicators.sellStopLossPrice, indicators.dsl_type);
	

//If using 1M BBS, exit when trend reverses
	if (indicators.dsl_type == EXIT_DSL_BBS && (int)pParams->settings[TIMEFRAME] == 1)
	{
		if (totalOpenOrders(pParams, BUY) > 0 && indicators.bbsTrend == -1 && indicators.bbsIndex == shift1Index && pParams->bidAsk.ask[0] >= indicators.position)
		{
			closeAllLongs();			
			return SUCCESS;
		}

		if (totalOpenOrders(pParams, SELL) > 0 && indicators.bbsTrend == 1 && indicators.bbsIndex == shift1Index && pParams->bidAsk.bid[0] <= indicators.position)
		{
			closeAllShorts();			
			return SUCCESS;
		}	
		
	}

	if (totalOpenOrders(pParams, BUY) > 0)
	{
		returnCode = modifyOrders(pParams, &indicators, BUY);
		if (returnCode != SUCCESS)
		{
			return logAsirikuyError("runTakeOver->modifyOrders()", returnCode);
		}
	}
	
	if (totalOpenOrders(pParams, SELL) > 0)
	{

		returnCode = modifyOrders(pParams, &indicators, SELL);
		if (returnCode != SUCCESS)
		{
			return logAsirikuyError("runTakeOver->modifyOrders()", returnCode);
		}
	}

	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	int        shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2, shift2Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 3;
	int        dailyShift1Index = pParams->ratesBuffers->rates[DAILY_RATES].info.arraySize - 2;
	
	pIndicators->dailyATR = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);	

	iBBandStop(PRIMARY_RATES, (int)parameter(BBS_PERIOD), (int)parameter(BBS_DEVIATION), &pIndicators->bbsTrend, &pIndicators->bbsStopPrice, &pIndicators->bbsIndex);
	
	pIndicators->position = (double)parameter(POSITION);

	pIndicators->preHigh = iHigh(DAILY_RATES, 1);
	pIndicators->preLow = iLow(DAILY_RATES, 1);
	pIndicators->preClose = iClose(DAILY_RATES, 1);

	pIndicators->pre2DaysHigh = pIndicators->preHigh;
	if (iHigh(DAILY_RATES, 2) > pIndicators->preHigh)
		pIndicators->pre2DaysHigh = iHigh(DAILY_RATES, 2);

	pIndicators->pre2DaysLow = pIndicators->preLow;
	if (iLow(DAILY_RATES, 2) < pIndicators->preLow)
		pIndicators->pre2DaysLow = iLow(DAILY_RATES, 2);	

	pIndicators->adjust = (double)parameter(BBS_ADJUSTPOINTS);
	pIndicators->dsl_type = (int)parameter(DSL_TYPE);

	pIndicators->movingAverage200M = iMA(3, HOURLY_RATES, 200, 1);

	switch (pIndicators->dsl_type)
	{
	case EXIT_DSL_NONE: // That is smart auto DSL mode. It will get the smart stop loss.
		//If the gap between 1Day and other two is too big, like more than 1 DailyATR, 
		pIndicators->buyStopLossPrice = pIndicators->pre2DaysLow;
		if (pIndicators->movingAverage200M > pIndicators->buyStopLossPrice)
			pIndicators->buyStopLossPrice = pIndicators->movingAverage200M;
		
		pIndicators->sellStopLossPrice = pIndicators->pre2DaysHigh;
		if (pIndicators->movingAverage200M < pIndicators->sellStopLossPrice)
			pIndicators->sellStopLossPrice = pIndicators->movingAverage200M;

		break;
	case EXIT_DSL_1DayHL:		
		pIndicators->buyStopLossPrice = pIndicators->preLow;
		pIndicators->sellStopLossPrice = pIndicators->preHigh;
		break;

	case EXIT_DSL_2DayHL:
		pIndicators->buyStopLossPrice = pIndicators->pre2DaysLow;
		pIndicators->sellStopLossPrice = pIndicators->pre2DaysHigh;
		break;
	case EXIT_DSL_1HM200:
		pIndicators->buyStopLossPrice = pIndicators->sellStopLossPrice = pIndicators->movingAverage200M;
		break;
	case EXIT_DSL_BBS:
		pIndicators->buyStopLossPrice = pIndicators->sellStopLossPrice = pIndicators->bbsStopPrice;
		break;
	case EXIT_DSL_DailyATR:
		pIndicators->buyStopLossPrice = pIndicators->sellStopLossPrice = pIndicators->dailyATR;
		break;	
	default: // No change SL
		pIndicators->buyStopLossPrice = pIndicators->sellStopLossPrice = -1;
		break;
	}


	return returnCode;
}


static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators)
{	
	
	addValueToUI("BBSStopPrice", pIndicators->bbsStopPrice);			
	addValueToUI("DailyATR", pIndicators->dailyATR);
	addValueToUI("1DayHigh", pIndicators->preHigh);
	addValueToUI("1DayLow", pIndicators->preLow);
	addValueToUI("2DaysHigh", pIndicators->pre2DaysHigh);
	addValueToUI("2DaysLow", pIndicators->pre2DaysLow);
	addValueToUI("1H200M", pIndicators->movingAverage200M);
	addValueToUI("DSLType", pIndicators->dsl_type);
	addValueToUI("BuySLP", pIndicators->buyStopLossPrice);
	addValueToUI("SellSLP", pIndicators->sellStopLossPrice);
	addValueToUI("Position", pIndicators->position);
	return SUCCESS;
}


static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators, OrderType orderType)
{	
	double stopLoss, takeProfit;	

	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeExits() failed. pParams = NULL\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeExits() failed. pIndicators = NULL\n");
		return NULL_POINTER;
	}


	if (orderType == BUY && pIndicators->buyStopLossPrice >0)
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->buyStopLossPrice + pIndicators->adjust);
		modifyTradeEasy(orderType, -1, stopLoss, -1); // It means only modify stop loss, not TP.		
	}

	if (orderType == SELL && pIndicators->sellStopLossPrice >0)
	{
		stopLoss = fabs(pIndicators->sellStopLossPrice - pParams->bidAsk.bid[0] + pIndicators->adjust);
		if (totalOpenOrders(pParams, SELL) > 0)
			modifyTradeEasy(SELL, -1, stopLoss, -1); // It means only modify stop loss, not TP.
		
	}

	return SUCCESS;

}