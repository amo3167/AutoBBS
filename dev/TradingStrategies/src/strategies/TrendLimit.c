/*
* Generate limit orders in strong trend.
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "AsirikuyTime.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE


typedef enum additionalSettings_t
{
	DAILY_TREND = ADDITIONAL_PARAM_1,
	ADJUSTPOINTS = ADDITIONAL_PARAM_2
} AdditionalSettings;

typedef struct indicators_t
{	
	int dailyTrend;		
	double preHigh;
	double preLow;
	double preClose;
	double adjust;
} Indicators;

// Forward declarations
static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators);
static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode runTrendLimit(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;
	Base_Indicators base_Indicators;

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	
	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] runTrendLimit() failed. pParams = NULL\n");
		return NULL_POINTER;
	}

	// Load the base indicators
	runBase(pParams, &base_Indicators);

	// Load the strategy specific indicator
	loadIndicators(pParams, &indicators);	

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	setUIValues(pParams, &indicators, &base_Indicators);

	returnCode = handleTradeEntries(pParams, &indicators, &base_Indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runBBS->handleTradeEntries()", returnCode);
	}

	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
		
	pIndicators->dailyTrend = (int)parameter(DAILY_TREND);
	pIndicators->adjust = (double)parameter(ADJUSTPOINTS);
	
	pIndicators->preHigh = iHigh(B_DAILY_RATES, 1);
	pIndicators->preLow = iLow(B_DAILY_RATES, 1);
	pIndicators->preClose = iClose(B_DAILY_RATES, 1);
		
	return returnCode;
}


static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	addValueToUI("DailyTrend", pIndicators->dailyTrend);	
	
	return SUCCESS;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	double	   stopLoss, takePrice = 0, openPrice= 0;

	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeEntries() failed. pParams = NULL\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeEntries() failed. pIndicators = NULL\n");
		return NULL_POINTER;
	}

	if (pBase_Indicators == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeEntries() failed. pBase_Indicators = NULL\n");
		return NULL_POINTER;
	}


	if (pBase_Indicators->dailyTrend >= 2)
	{		
		stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->dailyS + pIndicators->adjust);
		takePrice = 0;
		
		//����Ѿ����֣�����Ҫ����ֹ��
		base_ModifyOrders(pParams, BUY, stopLoss, -1);


		//ֻ������3�죬����3�ܹ���Ĺؼ�Kͻ�ƣ�����ʹ��Fin�ز��볡��
		if (pBase_Indicators->daily3RulesTrend == UP)
		{
			//Fin 38.2%				
			openPrice = pIndicators->preHigh - (pIndicators->preHigh - pIndicators->preLow) * 0.382 + pIndicators->adjust;

			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

			//Fin 50%
			openPrice = pIndicators->preHigh - (pIndicators->preHigh - pIndicators->preLow) * 0.5 + pIndicators->adjust;
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
		}
		else if (pBase_Indicators->dailyTrend >= UP_NORMAL)
		{
			// Pivot 		
			openPrice = pBase_Indicators->dailyPivot + pIndicators->adjust;
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
		}
	}	
		
	
	if (pBase_Indicators->dailyTrend <= -2)
	{		
		stopLoss = fabs(pBase_Indicators->dailyS - pParams->bidAsk.bid[0] + pIndicators->adjust);
		takePrice = 0;


		//����Ѿ����֣�����Ҫ����ֹ��
		base_ModifyOrders(pParams, SELL, stopLoss, -1);

		//ֻ������3�죬����3�ܹ���Ĺؼ�Kͻ�ƣ�����ʹ��Fin�ز��볡��
		if (pBase_Indicators->daily3RulesTrend == DOWN)
		{
			//Fin 38.2%				
			openPrice = pIndicators->preLow + (pIndicators->preHigh - pIndicators->preLow) * 0.382 - pIndicators->adjust;

			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

			//Fin 50%
			openPrice = pIndicators->preLow + (pIndicators->preHigh - pIndicators->preLow) * 0.5 - pIndicators->adjust;
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
		}
		else if (pBase_Indicators->dailyTrend <= DOWN_NORMAL)
		{
			// Pivot 		
			openPrice = pBase_Indicators->dailyPivot - pIndicators->adjust;
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
		}

		
	}
	return SUCCESS;
}
