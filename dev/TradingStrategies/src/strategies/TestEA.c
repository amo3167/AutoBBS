/*
* 15M time frame order management strategy
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "StrategyAPI.h" /* central prototypes */

/* Local static forward for modifyOrders only (others now in StrategyAPI.h) */
typedef struct indicators_t Indicators; 
static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice);

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

typedef enum trend_t
{
	UP = 1,
	DOWN = -1
}trend;

typedef enum additionalSettings_t
{
	TESTEA_ADJUSTPOINTS = ADDITIONAL_PARAM_1
	
} AdditionalSettings;

typedef struct indicators_t
{
	int bbsTrend;	
	double bbsStopPrice;
	int bbsIndex;
	double dailyATR;
	double primaryATR;
	double adjust;
} Indicators;

AsirikuyReturnCode runTestEA(StrategyParams* pParams)
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

	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, BBSTrend=%ld,BBStopPrice=%lf, BBSIndex = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,  indicators.bbsTrend, indicators.bbsStopPrice, indicators.bbsIndex);
	
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
	
	pIndicators->dailyATR = iAtr(DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pIndicators->primaryATR = iAtr(PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	iBBandStop(PRIMARY_RATES, 20, 2, &pIndicators->bbsTrend, &pIndicators->bbsStopPrice, &pIndicators->bbsIndex);
	
	pIndicators->adjust = (double)parameter(TESTEA_ADJUSTPOINTS);

	return returnCode;
}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators)
{	
	addValueToUI("BBSTrend", pIndicators->bbsTrend);
	addValueToUI("BBSStopPrice", pIndicators->bbsStopPrice);
	addValueToUI("DailyATR", pIndicators->dailyATR);
	addValueToUI("PrimaryATR", pIndicators->primaryATR);
	addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);

	return SUCCESS;
}

static AsirikuyReturnCode modifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice)
{
	/* Removed unused locals (previously triggering C4101). The commented block
	 * below relied on these variables; if that logic is reinstated, restore the
	 * declarations with proper usage.
	 */
	int tpMode = 0;

	//if ((int)pParams->settings[TIMEFRAME] >= MINUTES_PER_HOUR) // 1H above time frame short term trade
	//{
	//	virtualOrderEntryTime = getLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID]);

	//	shift0Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 1;
	//	shift1Index = pParams->ratesBuffers->rates[PRIMARY_RATES].info.arraySize - 2;
	//	currentTime = pParams->ratesBuffers->rates[PRIMARY_RATES].time[shift0Index];
	//	safe_gmtime(&timeInfo1, currentTime);
	//	safe_gmtime(&timeInfo2, virtualOrderEntryTime);

	//	safe_timeString(timeString1, currentTime);
	//	safe_timeString(timeString2, virtualOrderEntryTime);

	//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Last order update time = %s,current time = %s, Time Difference (hours) = %lf", timeString2, timeString1, difftime(virtualOrderEntryTime, currentTime) / 3600);

	//	if (virtualOrderEntryTime != -1 && timeInfo1.tm_mday != timeInfo2.tm_mday) // New day
	//	{
	//		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Move to a new day. Should modify order TP.");

	//		setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);

	//		tpMode = 1;
	//	}
	//}

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
			modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode,TRUE); // New day TP change as
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
			modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode, TRUE); // New day TP change as
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

	if (pIndicators->bbsTrend == 1)
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - pIndicators->bbsStopPrice + pIndicators->adjust);
		if (pIndicators->bbsIndex == shift1Index )
		{		
			//takePrice = stopLoss;
			//openSingleLongEasy(takePrice, stopLoss, 0, 0.5);

			takePrice = 2* stopLoss;
			openSingleLongEasy(takePrice, stopLoss, 0, 1);

			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, 1);
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

		if (pIndicators->bbsIndex == shift1Index)
		{
			
			//takePrice = stopLoss;
			//openSingleShortEasy(takePrice, stopLoss, 0, 0.5);
			takePrice = 2 * stopLoss;
			openSingleShortEasy(takePrice, stopLoss, 0, 1);

			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, 1);
			
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

	if (pIndicators->bbsTrend == 1)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}
	}

	if (pIndicators->bbsTrend == -1 )
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
	}
	return SUCCESS;
}
