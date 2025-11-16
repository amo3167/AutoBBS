/*
 * Range Order Management Module
 * 
 * Provides functions for managing range orders (buy/sell orders based on pivot points).
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"  // For splitBuyRangeOrders, splitSellRangeOrders

BOOL entryBuyRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder)
{
	time_t currentTime;
	struct tm timeInfo1;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	Order_Info orderInfo;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	if ((int)parameter(AUTOBBS_RANGE) == 1
		&& (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, 20, 1)) < 1
		//&& getWinTimesInDayEasy(currentTime) < 1
		&& iLow(B_DAILY_RATES, 0) <= iHigh(B_DAILY_RATES, 0) - pBase_Indicators->pDailyMaxATR
		&& pParams->bidAsk.ask[0] < pBase_Indicators->dailyS2
		&& timeInfo1.tm_hour >= 17
		)

	{
		if (isEnterOrder == TRUE &&( pIndicators->bbsTrend_primary == 1
			|| iClose(B_PRIMARY_RATES, 1) - iLow(B_DAILY_RATES, 0) >= 1.5 * iAtr(B_HOURLY_RATES, 20, 1))
			)
		{			
			splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
		}

		if (isOrderSignal == TRUE 
			&& timeInfo1.tm_hour >= stopHour //&& timeInfo1.tm_min == 0
			)
		{
			//Save signal in the file
			orderInfo.orderNumber = 0;
			orderInfo.type = BUY;
			orderInfo.orderStatus = PENDING;
			orderInfo.openPrice = 0;
			orderInfo.stopLossPrice = 0;
			orderInfo.takeProfitPrice = 0;
			orderInfo.timeStamp = currentTime;

			saveTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);
		}
	}
	return TRUE;
}

BOOL entrySellRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder)
{
	time_t currentTime;
	struct tm timeInfo1;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	Order_Info orderInfo;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	if ((int)parameter(AUTOBBS_RANGE) == 1
		&& (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.bid[0], 3 * iAtr(B_HOURLY_RATES, 20, 1)) < 1
		//&& getWinTimesInDayEasy(currentTime) < 1
		&& iHigh(B_DAILY_RATES, 0) >= iLow(B_DAILY_RATES, 0) + pBase_Indicators->pDailyMaxATR
		&& pParams->bidAsk.bid[0] > pBase_Indicators->dailyR2
		&& timeInfo1.tm_hour >= 17
		)

	{
		if (isEnterOrder == TRUE && (pIndicators->bbsTrend_primary == -1
			|| iHigh(B_DAILY_RATES, 0) - iClose(B_PRIMARY_RATES, 1) >= 1.5*iAtr(B_HOURLY_RATES, 20, 1))
			)
		{			
			splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		}

		if (isOrderSignal == TRUE 
			&& timeInfo1.tm_hour >= stopHour //&& timeInfo1.tm_min == 0
			)
		{
			//Save signal in the file
			orderInfo.orderNumber = 0;
			orderInfo.type = SELL;
			orderInfo.orderStatus = PENDING;
			orderInfo.openPrice = 0;
			orderInfo.stopLossPrice = 0;
			orderInfo.takeProfitPrice = 0;
			orderInfo.timeStamp = currentTime;

			saveTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);
		}
	}
	return TRUE;
}

int isRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex)
{
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	double pivot, s1, s2, s3, r1, r2, r3;
	double diffDays;
	Order_Info orderInfo;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	
	iPivot(B_DAILY_RATES, 2, &pivot, &s1, &r1, &s2, &r2, &s3, &r3);
		
	if ((int)parameter(AUTOBBS_RANGE) == 1
		&& pParams->orderInfo[orderIndex].isOpen == FALSE
		&& pParams->orderInfo[orderIndex].profit < 0
		)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].openTime) / (60 * 60 * 24);

		iPivot(B_DAILY_RATES, 2, &pivot, &s1, &r1, &s2, &r2, &s3, &r3);


		if (diffDays < 1 && timeInfo1.tm_yday != timeInfo2.tm_yday && timeInfo2.tm_hour >= 17)
		{
			if (pParams->orderInfo[orderIndex].type == BUY && pParams->orderInfo[orderIndex].openPrice < r2)
			{				
				return 1;
			}
				 
			if (pParams->orderInfo[orderIndex].type == SELL && pParams->orderInfo[orderIndex].openPrice > r2)
			{				
				return -1;
			}	
				
		}
	}

	readTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);

	if (orderInfo.orderStatus == PENDING)
	{
		diffDays = difftime(currentTime, orderInfo.timeStamp) / (60 * 60 * 24);
		if (diffDays < 1)
		{
			if (orderInfo.type == BUY)
				return 2;
			if (orderInfo.type == SELL)
				return -2;
		}
	}
			
	return 0;

}

