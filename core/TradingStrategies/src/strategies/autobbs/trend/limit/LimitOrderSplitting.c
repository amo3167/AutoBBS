/*
 * Limit Order Splitting Module
 * 
 * Provides order splitting functions for Limit strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"

void splitRangeBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double lots;
	double gap = iAtr(B_HOURLY_RATES, 20, 1);
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->stopLoss) * pIndicators->risk;


	pIndicators->entryPrice = pBase_Indicators->dailyPivot;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
		)
	{
		openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots/2,1);
	}

	pIndicators->entryPrice = pBase_Indicators->dailyS1;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
		)
	{
		openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots/2, 1);
	}
	

}

void splitRangeSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentPrice;
	double lots;
	double gap = iAtr(B_HOURLY_RATES, 20, 1);
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->stopLoss) * pIndicators->risk;

	currentPrice = pParams->bidAsk.ask[0];

	pIndicators->entryPrice = pBase_Indicators->dailyPivot;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
		)
	{
		openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots/2,1);
	}

	pIndicators->entryPrice = pBase_Indicators->dailyR1;
	if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
		&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
		)
	{
		openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePrice, pIndicators->stopLoss, lots/2, 1);
	}


	

}

void splitBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int mode,double takePrice_primary, double stopLoss)
{	
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentPrice;
	double lots;
	double gap = iAtr(B_HOURLY_RATES, 20, 1);
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;		

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];	
	
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;
	logInfo("System InstanceID = %d, BarTime = %s, lots=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lots);

	currentPrice = pParams->bidAsk.ask[0];		
	if (currentPrice <= pBase_Indicators->dailyPivot)
	{			
		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1				
			//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleBuyStopEasy(pIndicators->entryPrice + pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
		}

		if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour < 8)
		{
			pIndicators->entryPrice = pBase_Indicators->dailyS1;
			if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
				)
			{
				if (currentPrice > pIndicators->entryPrice)
					openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
				else
					openSingleBuyStopEasy(pIndicators->entryPrice + pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
			}
		}
	}
	else
	{

		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleBuyLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
		}

		if (mode == 1 
			//&& iLow(B_DAILY_RATES,0) < pBase_Indicators->dailyPivot
			&& getOrderCountTodayEasy(currentTime) > 0
			) 
		{
			pIndicators->entryPrice = pBase_Indicators->dailyR1;
			if (!isSamePriceBuyStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceBuyLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1				
				//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
				)
			{
				openSingleBuyStopEasy(pIndicators->entryPrice + pIndicators->adjust, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots);
			}

		}
			
	}

}

void splitSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int mode, double takePrice_primary, double stopLoss)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentPrice;
	double lots;
	double gap = iAtr(B_HOURLY_RATES, 20, 1);
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice)* pIndicators->risk;

	logInfo("System InstanceID = %d, BarTime = %s, lots=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, lots);

	currentPrice = pParams->bidAsk.bid[0];
	if (currentPrice > pBase_Indicators->dailyPivot)
	{			
		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleSellStopEasy(pIndicators->entryPrice - pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
		}

		if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour < 8)
		{			
			pIndicators->entryPrice = pBase_Indicators->dailyR1;
			if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1				
				)
			{
				if (currentPrice < pIndicators->entryPrice)
					openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
				else
					openSingleSellStopEasy(pIndicators->entryPrice - pIndicators->adjust, pIndicators->takePriceLevel* pIndicators->takePrice, pIndicators->stopLoss, lots);
			}
		}

	}
	else
	{
		pIndicators->entryPrice = pBase_Indicators->dailyPivot;
		if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
			&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
			&& timeInfo1.tm_hour >= pIndicators->startHourOnLimt
			)
		{
			openSingleSellLimitEasy(pIndicators->entryPrice, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots, 1);
		}

		if (mode == 1 
			 //&& iHigh(B_DAILY_RATES, 0) > pBase_Indicators->dailyPivot
			 && getOrderCountTodayEasy(currentTime) > 0) //enable stop order on S1 or R1
		{
			pIndicators->entryPrice = pBase_Indicators->dailyS1;
			if (!isSamePriceSellStopOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePriceSellLimitOrderEasy(pIndicators->entryPrice, currentTime, gap)
				&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, gap)
				&& getWinTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1				
				//&& getLossTimesInDaywithSamePriceEasy(currentTime, pIndicators->entryPrice, gap) < 1
				)
			{
				openSingleSellStopEasy(pIndicators->entryPrice - pIndicators->adjust, pIndicators->takePriceLevel * pIndicators->takePrice, pIndicators->stopLoss, lots);
			}

		}

	}


}
