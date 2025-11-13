/*
* 15M time frame order management strategy
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "ComLib.h"
#include "StrategyUserInterface.h"
#include "TrendStrategy.h"
#include "AsirikuyTime.h" /* added for safe_gmtime/safe_timeString prototypes */
#include "InstanceStates.h"

/* Forward declarations for static helper functions to avoid C4013 */
static BOOL move_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, double stopLossLevel);
static BOOL entryBuyRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder);
static BOOL entrySellRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder);
static int isRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex);
static BOOL DailyTrade_Limit_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_DailyOpen(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

void splitBuyOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double lots;
	double miniLots = 0.01;
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

//	openSingleLongEasy(takePrice_primary, stopLoss, lots - miniLots, pIndicators->risk);
	//takePrice_primary = 0;
	openSingleLongEasy(pIndicators->takePrice, stopLoss, lots, pIndicators->risk);
}

void splitSellOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{	
	double lots;
	double miniLots = 0.01;
	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

	//openSingleShortEasy(takePrice_primary, stopLoss, lots - miniLots, pIndicators->risk);
	//takePrice_primary = 0;
	openSingleShortEasy(pIndicators->takePrice, stopLoss, lots, pIndicators->risk);
}


void splitBuyOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pHigh - pIndicators->entryPrice;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	double riskcap = parameter(AUTOBBS_RISK_CAP);
	double lots_max;

	int orderCountToday = 0;

	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// Take 1/3 of the gap as take profit
	
	if (pIndicators->tradeMode == 1)
	{
		if (gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{
				takePrice = gap /3;
				
				lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap to 2 % risk
				// If using calculated stop loss 50 points, then cap the max risk to 2%
				//Cap the max risk
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, BUY, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * 7);
				lots = min(lots_max, lots);

				//if (takePrice >= 1.5) //XAUUSD		
					openSingleLongEasy(takePrice, stopLoss, lots, 0);
				


				if (pIndicators->entrySignal == 1 
					//&& pIndicators->bbsIndex_excution != shift0Index_Primary - 1
					)
				{
					if (pIndicators->subTradeMode == 0)
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				}
			}
		}
	}
	else
	{		
		if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
		{		
			takePrice = stopLoss;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/2);
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk/2);		
		}
	}


	
}
void splitSellOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pIndicators->entryPrice - pLow;
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	time_t currentTime;
	struct tm timeInfo1;

	double riskcap = parameter(AUTOBBS_RISK_CAP);
	double lots_max;

	int orderCountToday = 0;

	char       timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Take 1/3 of the gap as take profit
	if (pIndicators->tradeMode == 1)
	{
		if (gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{

				takePrice = gap /3;
				lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap to 2 % risk
				// If using calculated stop loss 50 points, then cap the max risk to 2%
				//Cap the max risk
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, SELL, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * 7);
				lots = min(lots_max, lots);

				//if (takePrice >= 1.5) //XAUUSD
					openSingleShortEasy(takePrice, stopLoss, lots, 0);



				if (pIndicators->entrySignal == -1 
					//&& pIndicators->bbsIndex_excution != shift0Index_Primary - 1
					)
				{
					if (pIndicators->subTradeMode == 0)
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
				}
			}
		}
	}
	else
	{
		if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
		{
			takePrice = stopLoss;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		}
	}

}

void splitBuyOrders_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	
	takePrice = takePrice_primary;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 2 * takePrice_primary;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 0;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);
	
}
void splitSellOrders_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	takePrice = takePrice_primary;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 2 * takePrice_primary;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 0;

	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

}

void splitBuyOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
void splitSellOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		//takePrice = 2 * takePrice_primary;
		//openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitBuyOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR2)
	{
		takePrice = takePrice_primary;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		//takePrice = 2 * takePrice_primary;
		//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
			takePrice = 3 * takePrice_primary;
		else
			takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}
void splitSellOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS2)
	{
		takePrice = takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

		takePrice = 2 * takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
			takePrice = 3 * takePrice_primary;
		else
			takePrice = 0;

		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);
	}
}


void splitBuyOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = fabs(pBase_Indicators->weeklyR2 - pIndicators->adjust - pIndicators->entryPrice);
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
void splitSellOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1 to 1 (risk/reward ratio) or S2?
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->weeklyS2 + pIndicators->adjust));
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/*
һ ڳ
ڶ ڳ
*/
void splitBuyOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;	
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;
	
	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	//double pWeeklyATR = pBase_Indicators->pWeeklyATR;
	//double pWeeklyHigh = pBase_Indicators->pWeeklyHigh;
	//double pWeeklyLow = pBase_Indicators->pWeeklyLow;

	//double weeklyGap = pWeeklyHigh - pIndicators->entryPrice;
	
	//pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->strategyMaxRisk

	//lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss)* pIndicators->risk;
	//lots_singal = lots_singal * (pIndicators->strategyMaxRisk - pIndicators->strategyRiskWithoutLockedProfit) / pIndicators->strategyMaxRisk;
	 
	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice <= pBase_Indicators->dailyR1 && (int)pParams->settings[TIMEFRAME] == 15)
	{

		takePrice = dailyGap;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleLongEasy(takePrice, stopLoss, lots_singal/2, 0);

		takePrice = atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleLongEasy(takePrice, stopLoss, lots_singal / 2, 0);
	}
}

/*
һ ڳ
ڶ ڳ
*/
void splitSellOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	//double pWeeklyATR = pBase_Indicators->pWeeklyATR;
	//double pWeeklyHigh = pBase_Indicators->pWeeklyHigh;
	//double pWeeklyLow = pBase_Indicators->pWeeklyLow;

	//double weeklyGap = pIndicators->entryPrice - pWeeklyLow;
	//lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, stopLoss)* pIndicators->risk;
	//lots_singal = lots_singal * (pIndicators->strategyMaxRisk - pIndicators->strategyRiskWithoutLockedProfit) / pIndicators->strategyMaxRisk;

	// 1 to 1 (risk/reward ratio) 			
	if (pIndicators->entryPrice >= pBase_Indicators->dailyS1 && (int)pParams->settings[TIMEFRAME] == 15)		
	{
		
		takePrice = dailyGap;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleShortEasy(takePrice, stopLoss, lots_singal / 2, 0);

		takePrice = atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleShortEasy(takePrice, stopLoss, lots_singal / 2, 0);
	}
}

/*
źţֹӯ100
*/
void splitBuyOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	double lots, lots_singal;

	double total_pre_lost = 0;
	int lostTimes;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	takePrice = min(takePrice, pBase_Indicators->dailyATR);
		

	lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;

	lostTimes = getLossTimesFromNowsEasy(currentTime, &total_pre_lost);

	if (lostTimes <= 1)
		lots = lots_singal;
	else
		lots = total_pre_lost / takePrice;

	//if (strstr(pParams->tradeSymbol, "JPY") != NULL)
	//{
	//	if (lostTimes <= 1)
	//		lots = lots_singal;
	//	else
	//		lots = total_pre_lost / takePrice;
	//}
	//else
	//{
	//	//lots = lots_singal * pow(2, lostTimes);

	//	if (lostTimes <= 0)
	//		lots = lots_singal;
	//	else if (lostTimes == 1)
	//		//lots = total_pre_lost / takePrice + lots_singal;			
	//		lots = lots_singal * 2.5;
	//	else	
	//		lots = total_pre_lost / takePrice;
	//}

	openSingleLongEasy(takePrice, stopLoss, lots, 0);

	
}

/*
źţֹӯ100
*/
void splitSellOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	double lots, lots_singal;

	double total_pre_lost = 0;
	int lostTimes;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	takePrice = min(takePrice, pBase_Indicators->dailyATR);

	lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

	lostTimes = getLossTimesFromNowsEasy(currentTime, &total_pre_lost);

	if (lostTimes <= 1)
		lots = lots_singal;
	else
		lots = total_pre_lost / takePrice;

	//if (strstr(pParams->tradeSymbol, "JPY") != NULL)
	//{
	//	if (lostTimes <= 1)
	//		lots = lots_singal;
	//	else
	//		lots = total_pre_lost / takePrice;
	//}
	//else
	//{
	//	//lots = lots_singal * pow(2, lostTimes);

	//	if (lostTimes <= 0)
	//		lots = lots_singal;
	//	else if (lostTimes == 1)
	//		lots = lots_singal * 2.5;
	//	else 
	//		lots = total_pre_lost / takePrice;
	//}


	openSingleShortEasy(takePrice, stopLoss, lots, 0);
	
}


/*
źţֹӯһܲ,ղ
*/
void splitBuyOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	double lots_singal;

	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pIndicators->isEnableBuyMinLotSize == TRUE)
		lots_singal = pIndicators->minLotSize;
	else{
		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, max(stopLoss, pBase_Indicators->dailyATR*1.5)) * pIndicators->risk;

	}

	lots_singal = roundUp(lots_singal, pIndicators->minLotSize);

	//takePrice = pIndicators->takePrice;
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
		
	openSingleLongEasy(takePrice, stopLoss, lots_singal, 0);
	

}

/*
źţֹӯһܲ,ղ
*/
void splitSellOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	double lots_singal;

	double total_pre_lost = 0;
	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	if (pIndicators->isEnableSellMinLotSize == TRUE)
		lots_singal = pIndicators->minLotSize;
	else
		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, max(stopLoss, pBase_Indicators->dailyATR*1.5)) * pIndicators->risk;
	
	//takePrice = pIndicators->takePrice;
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
	lots_singal = roundUp(lots_singal, pIndicators->minLotSize);

	openSingleShortEasy(takePrice, stopLoss, lots_singal, 0);
}

void splitBuyOrders_Ichimoko_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss);
	lots = roundUp(lots, pIndicators->volumeStep) * pIndicators->risk;

	//Long term
	if (pIndicators->tradeMode == 1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots / 3 * 2, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleLongEasy(takePrice, stopLoss, lots / 3, 0);
	}



}

void splitBuyOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss);
	lots = roundUp(lots, pIndicators->volumeStep) * pIndicators->risk;

	//Long term
	if (pIndicators->tradeMode == 1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, lots / 3 * 2, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleLongEasy(takePrice, stopLoss, lots / 3, 0);
	}
	

	
}
void splitSellOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, stopLoss) * pIndicators->risk;

	if (pIndicators->tradeMode == 1)
	{
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, lots / 3 * 2, 0);
		takePrice = pBase_Indicators->dailyATR;
		openSingleShortEasy(takePrice, stopLoss, lots / 3, 0);
	}
}

void splitBuyOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	if (pIndicators->tradeMode == 1)
	{
		//takePrice = 0;
		takePrice = pIndicators->riskCap * 1.5* pBase_Indicators->dailyATR;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, 1.5* pBase_Indicators->dailyATR) * pIndicators->risk;

		if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			if (lots / pIndicators->volumeStep > 5)
			{		
				//takePrice = 1 * 1.5*  pBase_Indicators->dailyATR;
				//openSingleLongEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize)/2, 0);

				//takePrice = max(1,pIndicators->riskCap/2) * 1.5* pBase_Indicators->dailyATR;
				//openSingleLongEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize)/2, 0);

				openSingleLongEasy(takePrice, stopLoss, lots - pIndicators->minLotSize, 0);

				takePrice = 0;
				openSingleLongEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
				openSingleLongEasy(takePrice, stopLoss, lots, 0);
		}
		else
			openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
	{
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice)* pIndicators->risk;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);		
	}

	//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
}
void splitSellOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	if (pIndicators->tradeMode == 1)
	{
		//takePrice = 0;
		takePrice = pIndicators->riskCap * 1.5* pBase_Indicators->dailyATR;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, 1.5* pBase_Indicators->dailyATR) * pIndicators->risk;


		if (pIndicators->riskCap > 0)
		{
			lots = roundUp(lots, pIndicators->volumeStep);
			if (lots / pIndicators->volumeStep > 5)
			{
				//takePrice = 1 * 1.5* pBase_Indicators->dailyATR;;
				//openSingleShortEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize)/2, 0);

				//takePrice = max(1,pIndicators->riskCap/2) * 1.5* pBase_Indicators->dailyATR;
				//openSingleShortEasy(takePrice, stopLoss, (lots - pIndicators->minLotSize)/2, 0);

				openSingleShortEasy(takePrice, stopLoss, lots - pIndicators->minLotSize, 0);

				takePrice = 0;
				openSingleShortEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
			}
			else
				openSingleShortEasy(takePrice, stopLoss, lots, 0);
		}
		else
			openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else{
		takePrice = stopLoss;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice)* pIndicators->risk;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	//openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	
}

void splitBuyOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;

	if (pIndicators->tradeMode == 1)
		takePrice = 0;
	else
		takePrice = pBase_Indicators->weeklyATR;

	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, 1.5* pBase_Indicators->weeklyATR) * pIndicators->risk;
	openSingleLongEasy(takePrice, stopLoss, lots, 0);

	//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
}
void splitSellOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	if (pIndicators->tradeMode == 1)
		takePrice = 0;
	else
		takePrice = pBase_Indicators->weeklyATR;

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, 1.5* pBase_Indicators->weeklyATR) * pIndicators->risk;
	openSingleShortEasy(takePrice, stopLoss, lots, 0);

	//openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);

}


void splitBuyOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	//double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);
	// 1 to 1 (risk/reward ratio) 			
	if ((pIndicators->entryPrice <= pBase_Indicators->dailyR1 && (int)pParams->settings[TIMEFRAME] == 15) ||
		(pIndicators->entryPrice <= pBase_Indicators->dailyR2 && (int)pParams->settings[TIMEFRAME] == 5) ||
		(pIndicators->entryPrice <= pBase_Indicators->dailyR2 && (int)pParams->settings[TIMEFRAME] == 30))
	{

		takePrice = 0.8* atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		takePrice = atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		//takePrice = atr - (pIndicators->entryPrice - intradayLow);
		//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}


}
void splitSellOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	//double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);	

	// 1 to 1 (risk/reward ratio) 			
	if ((pIndicators->entryPrice >= pBase_Indicators->dailyS1 && (int)pParams->settings[TIMEFRAME] == 15) ||
		(pIndicators->entryPrice >= pBase_Indicators->dailyS2 && (int)pParams->settings[TIMEFRAME] == 5) ||
		(pIndicators->entryPrice >= pBase_Indicators->dailyS2 && (int)pParams->settings[TIMEFRAME] == 30))
	{
		//takePrice = atr - (intradayHigh - pIndicators->entryPrice);
		takePrice = 0.8* atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}

void splitBuyOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double gap = pHigh - pIndicators->entryPrice;

	// Take 1/3 of the gap as take profit
	if (gap >= pATR / 2)
	{
		//takePrice = 2 * takePrice_primary;		
		//openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk /2 );
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
void splitSellOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double gap = pIndicators->entryPrice - pLow;

	// Take 1/3 of the gap as take profit
	if (gap >= pATR / 2)
	{		
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitBuyRangeOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	double lots;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

	openSingleLongEasy(pIndicators->takePrice, pIndicators->stopLoss, lots, pIndicators->risk);
	
}

void splitSellRangeOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double lots;

	lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, pIndicators->takePrice) * pIndicators->risk;

	openSingleShortEasy(pIndicators->takePrice, pIndicators->stopLoss, lots, pIndicators->risk);

}

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
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, lots=%lf",
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

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, lots=%lf",
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


AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;	
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	struct tm timeInfo1;

	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int execution_tf, euro_index_rate, count;
	double ATR0_EURO = 10;

	double intradayClose = iClose(B_PRIMARY_RATES, 0);
	
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;

	//Check out if Key K is triggered. If yes, take piority with other strategies., 
	//TODO: Need to add riskPNL strategy, need to implement
	if ((int)parameter(AUTOBBS_KEYK) == 1)
	{
		workoutExecutionTrend_KeyK(pParams, pIndicators, pBase_Indicators);
		if (pIndicators->entrySignal != 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == 1 && pBase_Indicators->dailyTrend_Phase < 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == -1 && pBase_Indicators->dailyTrend_Phase > 0)
			return SUCCESS;
	}

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:		
		if (pIndicators->tradeMode == 0)
			workoutExecutionTrend_DailyOpen(pParams, pIndicators, pBase_Indicators, TRUE);
		else
			workoutExecutionTrend_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);

		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal		
			workoutExecutionTrend_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, FALSE);

		pIndicators->splitTradeMode = 0;	

		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		if (pIndicators->tradeMode == 0)
			workoutExecutionTrend_DailyOpen(pParams, pIndicators, pBase_Indicators, TRUE);
		else
			workoutExecutionTrend_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);

		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal			
			workoutExecutionTrend_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);

		if ((int)parameter(AUTOBBS_LONG_SHORT_MODE) == 1)
			pIndicators->splitTradeMode = 0;		
		else
			pIndicators->splitTradeMode = 6;		

		break;
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
		workoutExecutionTrend_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators);
		break;
	case RANGE_PHASE:
		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			if (pBase_Indicators->weeklyMATrend == RANGE)
				pIndicators->exitSignal = EXIT_ALL;				
			else
			{
				//closeAllWithNegativeEasy(1, currentTime, 3);

				if (totalOpenOrders(pParams, BUY) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->dailyS) + pIndicators->adjust;
					modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
				}
				if (totalOpenOrders(pParams, SELL) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->dailyR) + pIndicators->adjust;
					modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
				}
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	if (pIndicators->entrySignal == 1 && iClose(B_DAILY_RATES, 1) < iMA(3, B_DAILY_RATES, 50, 1))
		pIndicators->entrySignal = 0;

	if (pIndicators->entrySignal == -1 && iClose(B_DAILY_RATES, 1) > iMA(3, B_DAILY_RATES, 50, 1))
		pIndicators->entrySignal = 0;

	//Asia hours
	//if (timeInfo1.tm_hour < 8 )
	//{
	//	pIndicators->tradeMode = 0;
	//}

	//if ((int)parameter(AUTOBBS_TP_MODE) < 0
	//	//|| fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) >= pBase_Indicators->dailyATR
	//	)
	//{
	//	pIndicators->tradeMode = 0;
	//}


	//if ((pIndicators->entrySignal == 1 && pBase_Indicators->weeklyMATrend != UP_NORMAL)
	//	|| (pIndicators->entrySignal == -1 && pBase_Indicators->weeklyMATrend != DOWN_NORMAL)
	//	)
	//{
	//	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, tradeMode = %ld",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->tradeMode);

	//	pIndicators->tradeMode = 0;
	//}

	// If MACD doesn't support, then don't trade short term
	//fast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 1);

	//if ((pIndicators->entrySignal == 1 && fast <= 0)
	//	|| (pIndicators->entrySignal == -1 && fast >= 0)
	//	)
	//volume1 = iVolume(B_DAILY_RATES, 1);
	//volume2 = iVolume(B_DAILY_RATES, 2);
	//
	//if ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
	//	&& pIndicators->entrySignal != 0 && volume1 > volume2)
	//{
	//	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, volume1 = %lf, volume2 = %lf,tradeMode = %ld",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, volume1, volume2, pIndicators->tradeMode);

	//	pIndicators->tradeMode = 0;
	//}

	// After 17:00, if the current day hasn't exceeded the intraday range, then need to close all short term orders
	if (timeInfo1.tm_hour >= 17)
	{
		execution_tf = (int)pParams->settings[TIMEFRAME];
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);

		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO, pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);

		if (ATR0_EURO < pIndicators->atr_euro_range)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);					
	}


	// Calculate intraday support/resistance levels and daily range
	//// 1: current day; 2: current week

	//count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
	//if (count > 1)
	//	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index - 1, count, &intradayHigh, &intradayLow);
	//else
	//	return SUCCESS;

	//intradayLow = min(close_prev1, intradayLow);
	//intradayHigh = max(close_prev1, intradayHigh);
	//ATR0 = fabs(intradayHigh - intradayLow);
	//
	//if (ATR0 > pBase_Indicators->pDailyMaxATR)
	//{
	//	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ATR0 =%lf,pDailyMaxATR = %lf close all current short term orders",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, pBase_Indicators->pDailyMaxATR);
	//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	//}


	// Use MACD to exit long term trades

	//Load MACD
	/*
	if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min <= 3)
	{
		fast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 1);
		slow = iMACD(B_DAILY_RATES, 5, 10, 5, 1, 1);

		//preFast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 2);
		//preSlow = iMACD(B_DAILY_RATES, 5, 10, 5, 1, 2);

		preClose1 = iClose(B_DAILY_RATES, 1);
		preClose2 = iClose(B_DAILY_RATES, 2);
		preClose3 = iClose(B_DAILY_RATES, 3);
		preClose4 = iClose(B_DAILY_RATES, 4);
		preClose5 = iClose(B_DAILY_RATES, 5);

		if (fast > slow ||
			(preClose5 < preClose4 && preClose5 < preClose3 && preClose5 < preClose2 && preClose5 < preClose1)) //If no new low in the last 4 days, close long term short trades
		{
			fprintf(stderr, "[WARNING] Exit Long term short trade:System InstanceID = %d, BarTime = %s, fast =%lf,slow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, fast, slow);

			closeAllLongTermShortsEasy();
		}

		if (fast < slow ||
			(preClose5 > preClose4 && preClose5 > preClose3 && preClose5 > preClose2 && preClose5 > preClose1)) //If no new high in the last 4 days, close long term long trades
		{
			fprintf(stderr, "[WARNING] Exit Long term long trade:System InstanceID = %d, BarTime = %s, fast =%lf,slow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, fast, slow);
			closeAllLongTermLongsEasy();
		}
	}
	*/
	profitManagement(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int  shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int  shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	double keyHigh, keyLow;

	pIndicators->risk = 2;
	pIndicators->splitTradeMode = 9;
	pIndicators->tpMode = 0;

	keyHigh = iHigh(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);
	keyLow = iLow(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);

	if (pBase_Indicators->intradayTrend == 1)
	{
		if (pIndicators->side >= 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->exitSignal = EXIT_SELL;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = keyLow;
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyLow) >= 0.5 * pBase_Indicators->dailyATR)
				pIndicators->risk = 1;

			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = 1;
			}
		}
		else
			pBase_Indicators->intradayTrend = 0;
	}

	if (pBase_Indicators->intradayTrend == -1)
	{
		if (pIndicators->side <= 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->exitSignal = EXIT_BUY;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = keyHigh;
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyHigh) >= 0.5 * pBase_Indicators->dailyATR)
				pIndicators->risk = 1;

			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = -1;
			}
		}
		else
			pBase_Indicators->intradayTrend = 0;
	}


	return returnCode;

}

AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	
	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	//pIndicators->tradeMode = 1;
		
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{		
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);
		
		if (pParams->bidAsk.ask[0] - intradayLow > pBase_Indicators->dailyATR / 3 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
		{
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;		
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
	
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);
		if (intradayHigh - pParams->bidAsk.bid[0] > pBase_Indicators->dailyATR / 3 &&!isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
		{			
			pIndicators->entrySignal = -1;
		}	
		pIndicators->exitSignal = EXIT_BUY;
	}
	
	

	
	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,BOOL ignored)
{	
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double breakingHigh, breakingLow;
	double intraHigh = 99999, intraLow  = -99999;
	int count = 0;

	double baseline = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= 19 && timeInfo1.tm_min >= 25)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
		closeAllWithNegativeEasy(1, currentTime,3);
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);	

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 3;

	//pIndicators->tradeMode = 1;	

	pIndicators->subTradeMode = 0;

	//if (timeInfo1.tm_hour < 5)
	//	pIndicators->risk = 0.5;

	//TODO: 
	// if price is retreated back between the high low of breaking 15M bar, if it has a good room to entry
	breakingHigh = iHigh(B_PRIMARY_RATES, shift0Index_Primary - pIndicators->bbsIndex_excution);
	breakingLow = iLow(B_PRIMARY_RATES, shift0Index_Primary - pIndicators->bbsIndex_excution);
	count = shift1Index - pIndicators->bbsIndex_excution;
	if (count >= 2)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index, count, &intraHigh, &intraLow);

	baseline = iMA(3, B_DAILY_RATES, 50, 1);
	
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1
			&& iClose(B_DAILY_RATES,1) > baseline
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			))
		{
			pIndicators->entrySignal = 1;

			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_BBS_BreakOut.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);


		}
		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1
			&& iClose(B_DAILY_RATES, 1) < baseline
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)))
			)
		{
			pIndicators->entrySignal = -1;
			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_BBS_BreakOut.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;

	}


	return SUCCESS;
}

/*
*/
AsirikuyReturnCode workoutExecutionTrend_DailyOpen(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);


	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= 19 && timeInfo1.tm_min >= 25)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
		closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 6;


	//pIndicators->tradeMode = 1;

	// Enter order on key support/resistance levels
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pBase_Indicators->dailyTrend >= 3
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust			
			&& timeInfo1.tm_hour == 1
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3, currentTime)))
			)
			pIndicators->entrySignal = 1;

		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pBase_Indicators->dailyTrend <= -3
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& timeInfo1.tm_hour == 1
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3, currentTime))
			))
			pIndicators->entrySignal = -1;

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

/*
1. Ҫ15MBBS support?
*/
AsirikuyReturnCode workoutExecutionTrend_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators,BOOL ignored)
{	
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);

	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	
	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= 19 && timeInfo1.tm_min >= 25)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
		closeAllWithNegativeEasy(1, currentTime, 3);
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 6;
	pIndicators->subTradeMode = 1;
	
	//pIndicators->tradeMode = 1;

	// Enter order on key support/resistance levels
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0 ))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& preLow < pBase_Indicators->dailyPivot
			&& preClose >  pBase_Indicators->dailyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && pIndicators->bbsTrend_excution == 1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3, currentTime)))
			)
		{
			pIndicators->entrySignal = 1;
			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_Pivot.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily low has been lower than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& preHigh > pBase_Indicators->dailyPivot
			&& preClose <  pBase_Indicators->dailyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && pIndicators->bbsTrend_excution == -1 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->pDailyMaxATR / 3, currentTime))
			))
		{
			pIndicators->entrySignal = -1;

			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_Pivot.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

// Use BBS, entry on 15M BBS, stop loss at key support/resistance levels, and only S2/R2 only
// Risk management and position control, don't enter if risk too high
// Close all orders at 23:45
AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString[MAX_TIME_STRING_SIZE] = "";

	double breakingHigh, breakingLow;
	double intraHigh = 99999, intraLow = -99999;
	int count = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (XAUUSD_IsKeyDate(pParams, pIndicators, pBase_Indicators) == TRUE && timeInfo1.tm_hour >= 19 && timeInfo1.tm_min >= 25)
	{
		closeAllWithNegativeEasy(5, currentTime, 3);
	}
	else
		closeAllWithNegativeEasy(1, currentTime, 3);
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 0;

	pIndicators->subTradeMode = 2;

	//pIndicators->tradeMode = 1;

	//TODO: 
	// if price is retreated back between the high low of breaking 15M bar, if it has a good room to entry
	breakingHigh = iHigh(B_PRIMARY_RATES, shift0Index - pIndicators->bbsIndex_excution);
	breakingLow = iLow(B_PRIMARY_RATES, shift0Index - pIndicators->bbsIndex_excution);
	count = shift1Index - pIndicators->bbsIndex_excution;
	if (count >= 2)
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index, count, &intraHigh, &intraLow);

	// Enter order on key support/resistance levels
	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_RETREAT_PHASE )
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == 1
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
		{
			pIndicators->entrySignal = 1;
			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_MIDDLE_RETREAT_PHASE.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE )
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1
			&& (
			pIndicators->bbsIndex_excution == shift1Index
			|| (intraHigh < breakingHigh && intraLow > breakingLow)
			)
			&& pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
			)
		{
			pIndicators->entrySignal = -1;
			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_MIDDLE_RETREAT_PHASE.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		pIndicators->exitSignal = EXIT_BUY;

	}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int    shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	//int    shift1Index = pParams->ratesBuffers->rates[B_HOURLY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(2, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 10;


	if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}
	return SUCCESS;
}
AsirikuyReturnCode workoutExecutionTrend_Weekly_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	double preHigh = iHigh(B_SECONDARY_RATES, 1);
	double preLow = iLow(B_SECONDARY_RATES, 1);
	double preClose = iClose(B_SECONDARY_RATES, 1);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(2, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);


	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 11;

	// Enter order on key support/resistance levels
	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		if (pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& preLow < pBase_Indicators->weeklyPivot
			&& preClose >  pBase_Indicators->weeklyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		if (pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& preHigh > pBase_Indicators->weeklyPivot
			&& preClose <  pBase_Indicators->weeklyPivot
			&& timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}
// workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE is declared and defined in StrategyComLib.c
// Removed duplicate definition - using the one from StrategyComLib.c
AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double stopLoss;

	//Check out if Key K is triggered. If yes, take piority with other strategies.
	if ((int)parameter(AUTOBBS_KEYK) == 1)
	{
		workoutExecutionTrend_KeyK(pParams, pIndicators, pBase_Indicators);
		if (pIndicators->entrySignal != 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == 1 && pBase_Indicators->weeklyTrend_Phase < 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == -1 && pBase_Indicators->weeklyTrend_Phase > 0)
			return SUCCESS;
	}

	switch (pBase_Indicators->weeklyTrend_Phase)
	{
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:
		workoutExecutionTrend_Weekly_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal		
			workoutExecutionTrend_Weekly_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
		pIndicators->splitTradeMode = 11;
		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		workoutExecutionTrend_Weekly_Pivot(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal		
			workoutExecutionTrend_Weekly_BBS_BreakOut(pParams, pIndicators, pBase_Indicators, TRUE);
		pIndicators->splitTradeMode = 7;
		break;
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
		workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
		break;
	case RANGE_PHASE:
		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			if (totalOpenOrders(pParams, BUY) > 0)
			{
				stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->weeklyS) + pIndicators->adjust;
				modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
			}
			if (totalOpenOrders(pParams, SELL) > 0)
			{
				stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->weeklyR) + pIndicators->adjust;
				modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	profitManagementWeekly(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int orderIndex = -1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	//4H filter
	if ((timeInfo1.tm_hour-1) % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 20;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		movement = fabs(high_4H - low_4H);

		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // Should be based on current market conditions, 40% weekly ATR
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
		// Enter order on key support/resistance levels

		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - 20);

			//side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			
			if( (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY) ))
			{
				pIndicators->entrySignal = 1;
			}
	
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;

			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + 20);

			//side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			if ( (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL) ))
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;

		}

		if (pIndicators->entrySignal != 0 &&
			fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) > pBase_Indicators->pWeeklyPredictATR
			)
		{
			sprintf(pIndicators->status, "current weekly price gap %lf is greater than pWeeklyPredictATR %lf",
				fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice), pBase_Indicators->pWeeklyPredictATR);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}

		//profitManagement_base(pParams, pIndicators, pBase_Indicators);
	}
	return SUCCESS;
}


AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int orderIndex = 0;


	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		pIndicators->stopLoss = 2.5;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		pIndicators->stopLoss = pBase_Indicators->pWeeklyPredictMaxATR;
	}
	//4H filter
	if (timeInfo1.tm_hour % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 20;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		movement = fabs(high_4H - low_4H);

		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;		

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // Should be based on current market conditions, 40% weekly ATR
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
		// Enter order on key support/resistance levels
		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pIndicators->stopLoss);
				
			// First turning point
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY))
			{
				pIndicators->entrySignal = 1;
			}
			
			//// First order in trend, take 1/3
			//if (pIndicators->bbsIndex_4H == shift1Index_4H
			//	&& pIndicators->entrySignal == 0 
			//	&& pParams->orderInfo[orderIndex].isOpen == FALSE &&  getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < 3)
			//{
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
	
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pIndicators->stopLoss);

			// First turning point
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL))
			{
				pIndicators->entrySignal = -1;
			}
			
			//// First order in trend, take 1/3
			//if (pIndicators->bbsIndex_4H == shift1Index_4H
			//	&& pIndicators->entrySignal == 0 
			//	&& pParams->orderInfo[orderIndex].isOpen == FALSE && getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < 3)
			//{
			//	pIndicators->entrySignal = -1;
			//}


			pIndicators->exitSignal = EXIT_BUY;

		}

		//profitManagement_base(pParams, pIndicators, pBase_Indicators);
	}
	return SUCCESS;
}

/*
Ϊʹ15MBBS볡
4HؼKM50,M200
ƶֹûexit.
ֹӯڶ̬ߵ͵ATR
ܲԤⲨֹͣ볡ƶֹ
*/
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);
	

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	
	//closeAllWithNegativeEasy(1, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	//pIndicators->splitTradeMode = 0;
	//pIndicators->tpMode = 1;

	// ATR mode
	pIndicators->splitTradeMode = 19;
	pIndicators->tpMode = 3;

	//if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
	//	readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
		
	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	movement = fabs(high_4H - low_4H);
	if (pIndicators->atr_euro_range == 0)
		pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;
	
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, high_4H %lf low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
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
	// Enter order on key support/resistance levels

	
	if (pIndicators->bbsTrend_4H == 1)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pBase_Indicators->dailyS);
		
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			//&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4, currentTime)
			&& trend_4H == 1
			//&& fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = 1;
		}

		//if (pIndicators->entrySignal == 0 && pIndicators->bbsIndex_4H == shift1Index_4H
		//	&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4, currentTime)
		//	&& trend_MA < 0 && trend_KeyK == 1)
		//{
		//	pIndicators->entrySignal = 1;			
		//}

		// Stop loss:
		// If daily trend is range, reduce position size
		if (dailyTrend == 0
			&& pIndicators->entryPrice >= pBase_Indicators->dailyS
			&& fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) >= pBase_Indicators->dailyATR * 0.5)
			pIndicators->risk = 0.5;

		if (pIndicators->stopLossPrice > pIndicators->entryPrice) //invalid price. current price is above stop loss price. No entry trade.
			pIndicators->entrySignal = 0;

		//// If weekly and daily trend not consistent, should stop? Use 50% risk?
		//if (dailyTrend < 0)
		//	pIndicators->risk = 0.5;

		// Follow 4H trend change.
		//pIndicators->exitSignal = -1;	


	}

	if (pIndicators->bbsTrend_4H == -1)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
		if (dailyTrend == 0)
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyR);
		else
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pBase_Indicators->dailyS);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			//&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
			&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4, currentTime)
			&& trend_4H == -1
			//&& fabs(iHigh(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
			&& iAtr(B_WEEKLY_RATES, 1, 0) <= pBase_Indicators->pWeeklyPredictATR)
		{
			pIndicators->entrySignal = -1;
		}



		if (dailyTrend == 0 && pIndicators->entryPrice <= pBase_Indicators->dailyR
			&& fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) >= pBase_Indicators->dailyATR * 0.5)
			pIndicators->risk = 0.5;

		//// If weekly and daily trend not consistent, should stop? Use 50% risk?
		//if (dailyTrend > 0)
		//	pIndicators->entrySignal = 0;


		if (pIndicators->stopLossPrice < pIndicators->entryPrice) //invalid price. current price is above stop loss price. No entry trade.
			pIndicators->risk = 0.5;


		// Follow 4H trend change.
		//pIndicators->exitSignal = 1;
	}
	
	// If RiksPNL < 0, 
	// Stop loss not less than 0.5 ATR
	// Risk = 0.5

	//if (pIndicators->strategyRiskWithoutLockedProfit < 0 && fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) <= pBase_Indicators->dailyATR * 0.5)
	//{
	//		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, strategyRiskWithoutLockedProfit %lf GapToSL= %lf, dailyATR=%lf",
	//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRiskWithoutLockedProfit, fabs(pIndicators->stopLossPrice - pIndicators->entryPrice), pBase_Indicators->dailyATR );

	//	pIndicators->risk = 0.5;
	//}
	//if (pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->limitRiskPNL
	//	&& ((pIndicators->entryPrice > preHigh - (preHigh - preLow) * 0.382 && pIndicators->entrySignal == 1)
	//	|| (pIndicators->entryPrice < preLow + (preHigh - preLow) * 0.382 && pIndicators->entrySignal == -1)))
	//{
	//	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, strategyRisk %lf limitRiskPNL %lf, entrySignal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRisk, pIndicators->limitRiskPNL, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}


AsirikuyReturnCode workoutExecutionTrend_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1,timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose;	
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = 2.5;
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1;
	double highPrice, lowPrice;
	BOOL isMoveTP = TRUE;	
	int closeHour = 23, startHour = 2;	
	int trend = UNKNOWN;
	double ma20M,ma50M, ma200M;
		
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 31;	
	pIndicators->risk = 1;
	pIndicators->tpMode = 4;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	ma20M = iMA(3, B_PRIMARY_RATES, 20, 1);
	ma50M = iMA(3, B_PRIMARY_RATES, 50, 1);
	ma200M = iMA(3, B_PRIMARY_RATES, 200, 1);


	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
	
	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	if (pIndicators->fast > 0
		&& preDailyClose > dailyBaseLine		
		&& pIndicators->fast > pIndicators->slow
		&& pIndicators->fast > pIndicators->preFast
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& preDailyClose < dailyBaseLine	
		&& pIndicators->fast < pIndicators->slow
		&& pIndicators->fast < pIndicators->preFast
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		pBase_Indicators->shellingtonInTrend = 1;

	if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		pBase_Indicators->shellingtonInTrend = -1;

	if (pBase_Indicators->dailyTrend > 0 && pBase_Indicators->mACDInTrend == 1				
		)
	{
		trend = UP;
		//if (pBase_Indicators->shellingtonInTrend == 1)
		//{
		//	pIndicators->risk = 2;
		//}
	}
	if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1			
		)
	{
		trend = DOWN;
		//if (pBase_Indicators->shellingtonInTrend == -1)
		//{
		//	pIndicators->risk = 2;
		//}
	}

	if (pBase_Indicators->dailyTrend_Phase == 0)
		trend = RANGE;


	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, orderIndex,5*60,&highPrice, &lowPrice);

	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;
	pIndicators->executionTrend = 0;

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	//{
	//	if (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->ma_Signal < 0 && getOrderCountEasy() > 1)
	//	{
	//		pIndicators->exitSignal = EXIT_BUY;
	//		return SUCCESS;
	//	}
	//	if (pParams->orderInfo[orderIndex].type == SELL &&  pBase_Indicators->ma_Signal > 0 && getOrderCountEasy() > 1)
	//	{
	//		pIndicators->exitSignal = EXIT_SELL;
	//		return SUCCESS;
	//	}
	//	
	//}

	if (trend == UP)
	{		
	
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		if (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			if (getMATrend_SignalBase(20, 50, B_PRIMARY_RATES, 24)> 0 && pIndicators->entryPrice > ma200M)
			{					
				splitBuyOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, pBase_Indicators->dailyATR/2, pBase_Indicators->pDailyMaxATR);
			}
		}
		else
		{
			if (getOrderCountEasy() == 1)
			{
				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				modifyTradeEasy_new(BUY, -1, fabs(pIndicators->entryPrice - pIndicators->stopLossPrice), -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		
		pIndicators->exitSignal = EXIT_SELL;		
			
	}
	else if (trend == DOWN)
	{		
		pIndicators->entryPrice = pParams->bidAsk.bid[0];	
	
		if (orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			if (getMATrend_SignalBase(20, 50, B_PRIMARY_RATES, 24) < 0 && pIndicators->entryPrice < ma200M)
			{				
				splitSellOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, pBase_Indicators->dailyATR / 2, pBase_Indicators->pDailyMaxATR);
			}
		}
		else
		{
			if (getOrderCountEasy() == 1)
			{
				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				modifyTradeEasy_new(SELL, -1, fabs(pIndicators->entryPrice - pIndicators->stopLossPrice), -1, 0, pIndicators->stopMovingBackSL);
			}
		}
		
		pIndicators->exitSignal = EXIT_BUY;
			
	}
	else{
		pIndicators->exitSignal = EXIT_ALL;
	}


	return SUCCESS;
}

/*
1. Check Trend by MACD and Shellington and default
2. If all of them are same, it think it will be in strong trend.
3. It will entre order on key supports. (R1/S1 or pivot or ATR daily range)
3. TP is 1H ATR(20)
4. SL is 3 * TP
5. Lot size is caculated by TP, Risk = 0.3%
6. How to close order ealier? (By time?)
*/
AsirikuyReturnCode workoutExecutionTrend_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2, timeInfo3;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose,preDailyHigh,preDailyLow,preDailyOpen;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double moveTPLimit = (double)parameter(AUTOBBS_KEYK);
	int autoMode = (int)parameter(AUTOBBS_IS_AUTO_MODE);	
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1;
	double highPrice, lowPrice;
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);
	BOOL isMoveTPInNewDay = TRUE;
	int closeHour = 23, startHour = pIndicators->startHour;
	BOOL isEnableRangeTrade = FALSE;
	int trend = UNKNOWN;
	int truningPointIndex = -1, minPointIndex = -1;
	double isMACDBeili = FALSE;
	double totalLossPoint = 0;
	int totalLossTimes = 0;
	double realTakePrice;
	BOOL isEnableMACDSlow = TRUE;
	BOOL isEnableShellingtonTrend = FALSE;	
	BOOL isEnableFlatTrend = FALSE;
	BOOL isEnableWeeklyATR = TRUE;
	BOOL isEnableTooFar = FALSE;
	int stopHour = 23;		
	double tooFarLimit = 1;
	int barState = BAR_UNKNOWN;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;
	double rsi = 0.0, rsiLow = 20.0, rsiHigh = 80.0;
	int tradingDays = 10;
	BOOL isEnableRSI = FALSE;
	BOOL isEnableDoubleEntry = FALSE, isEnableDoubleEntry2 = FALSE;
	int rangeType = 0;
	int atrTimes = 20;
	BOOL isEnableTooBigSpread = FALSE;
	double ma960M = 0.0;
	int count = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 4;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;
	pIndicators->isEnableLimitSR1 = FALSE;

	pIndicators->takePriceLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);

	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, 5 * 60, orderIndex, &highPrice, &lowPrice);

	pIndicators->adjust = fabs(pParams->bidAsk.bid[0] - pParams->bidAsk.ask[0]);	

	count = timeInfo1.tm_hour * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]);

	ma960M = iMA(3, B_PRIMARY_RATES, 960, 1+ count);
	
	if (timeInfo1.tm_wday == 0)
		atrTimes = 50;

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,ask=%lf,bid=%lf,adjsut=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,pParams->bidAsk.ask[0], pParams->bidAsk.bid[0], pIndicators->adjust);


	if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		startHour = 3;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		
		isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
	}
	else if (strstr(pParams->tradeSymbol, "USDJPY") != NULL)
	{
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		isEnableMACDSlow = FALSE;

	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		if (timeInfo1.tm_wday == 3)
			pIndicators->risk = 0.6;

		startHour = 3;

		//isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
		isCloseOrdersEOD = TRUE;
		isEnableRSI = TRUE; // Not confirm it will be much better. Need manully control the risk if RSI or too much??
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{		

		if (timeInfo1.tm_wday == 3)
			pIndicators->risk = 0.6;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;
		
		startHour = 3;
		pIndicators->startHourOnLimt = 8;

		stopHour = 22;

		// filter non-farm payroll day
		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
		{

			strcpy(pIndicators->status, "Filter Non-farm day\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}


		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		if (timeInfo1.tm_wday == 4)
			pIndicators->risk = 0.5;

		isEnableShellingtonTrend = TRUE;
		pIndicators->isEnableLimitSR1 = TRUE;
		isEnableTooFar = FALSE; 
	
		isEnableDoubleEntry = TRUE;

		isEnableDoubleEntry2 = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "EURGBP") != NULL)
	{
		startHour = 3;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		isEnableMACDSlow = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, "EURUSD") != NULL)
	{
		startHour = 3;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		isEnableMACDSlow = FALSE;

		isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
	}
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	{
		pIndicators->adjust = pBase_Indicators->dailyATR * 0.01;

		startHour = 0;
		pIndicators->startHourOnLimt = startHour;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		//if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 6)
		//	pIndicators->risk = 0.6;

		if (timeInfo1.tm_wday == 2 || timeInfo1.tm_wday == 3 || timeInfo1.tm_wday == 4)
			pIndicators->risk = 0.5;

		if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 1)
		{
			if (fabs(pBase_Indicators->dailyTrend) >= 6
				&& iAtr(B_DAILY_RATES, 1, 1) < 0.7 * pBase_Indicators->pDailyATR
				)
				pIndicators->risk = 0.5;
		}

		//if (timeInfo1.tm_wday == 6 && timeInfo1.tm_hour == 16 && timeInfo1.tm_min >= 50)
		//{
		//	closeAllLimitAndStopOrdersEasy(currentTime);			
		//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		//	return SUCCESS;
		//}

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}
		//if (iAtr(B_DAILY_RATES, 1, 1) >= pBase_Indicators->dailyATR * 2)
		//{
		//	pIndicators->risk = 0.6;
		//}
		//if (fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) >= pBase_Indicators->pWeeklyPredictATR / 3)
		//{
		//	strcpy(pIndicators->status, "Previous day Close is more than pWeeklyPredictATR / 3.\n");

		//	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		//	return SUCCESS;
		//}

		isEnableWeeklyATR = FALSE;

		pIndicators->isEnableLimitSR1 = TRUE;

		//if (timeInfo1.tm_wday == 6)
		//	stopHour = 16;

		tooFarLimit = 2;

		isEnableRangeTrade = FALSE;

		isEnableDoubleEntry = TRUE;

		isEnableTooFar = TRUE;

		fastMAPeriod = 7;
		slowMAPeriod = 14;
		signalMAPeriod = 7;

		tradingDays = 14;
	}
	else if (strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		if (fabs(pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]) > pIndicators->adjust * 1.5){

			isEnableTooBigSpread = TRUE;
		}
		
		pIndicators->adjust = pBase_Indicators->dailyATR * 0.01;

		startHour = 0;
		pIndicators->startHourOnLimt = startHour;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		//if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 6)
		//	pIndicators->risk = 0.6;

		if (timeInfo1.tm_wday == 2 || timeInfo1.tm_wday == 4)
			pIndicators->risk = 0.5;

		if (timeInfo1.tm_wday == 0 || timeInfo1.tm_wday == 1)
		{
			if (fabs(pBase_Indicators->dailyTrend) >= 6
				&& iAtr(B_DAILY_RATES, 1, 1) < 0.7 * pBase_Indicators->pDailyATR
				)
				pIndicators->risk = 0.5;
		}

		//if (timeInfo1.tm_wday == 6 && timeInfo1.tm_hour == 16 && timeInfo1.tm_min >= 50)
		//{
		//	closeAllLimitAndStopOrdersEasy(currentTime);			
		//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		//	return SUCCESS;
		//}

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}
		//if (iAtr(B_DAILY_RATES, 1, 1) >= pBase_Indicators->dailyATR * 2)
		//{
		//	pIndicators->risk = 0.6;
		//}
		//if (fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) >= pBase_Indicators->pWeeklyPredictATR / 3)
		//{
		//	strcpy(pIndicators->status, "Previous day Close is more than pWeeklyPredictATR / 3.\n");

		//	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

		//	return SUCCESS;
		//}

		isEnableWeeklyATR = FALSE;

		pIndicators->isEnableLimitSR1 = TRUE;

		//if (timeInfo1.tm_wday == 6)
		//	stopHour = 16;

		tooFarLimit = 2;

		isEnableRangeTrade = FALSE;

		isEnableDoubleEntry = TRUE;

		isEnableTooFar = TRUE;

		fastMAPeriod = 7;
		slowMAPeriod = 14;
		signalMAPeriod = 7;

		tradingDays = 14;


	

	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{		
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		stopHour = 17;

		isEnableMACDSlow = FALSE;
		isEnableFlatTrend = TRUE;
		isEnableTooFar = TRUE;
		//isEnableRangeTrade = TRUE;
		pIndicators->startHourOnLimt = pIndicators->startHour;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE)		
		pIndicators->adjust = 0;	
		

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
		startShift = 0;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	rsi = iRSI(B_DAILY_RATES, tradingDays, 1);
	//isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	preDailyHigh = iHigh(B_DAILY_RATES, startShift);
	preDailyLow = iLow(B_DAILY_RATES, startShift);
	preDailyOpen = iOpen(B_DAILY_RATES, startShift);

	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	//if (preDailyClose - preDailyOpen > (preDailyHigh - preDailyLow) / 3)
	//	barState = BULL;
	//else if (preDailyOpen - preDailyClose > (preDailyHigh - preDailyLow) / 3)
	//	barState = BEAR;
	//else if (preDailyClose > preDailyOpen && preDailyOpen - preDailyLow > (preDailyHigh - preDailyLow) * 0.66)
	//	barState = BULL;
	//else if (preDailyClose < preDailyOpen && preDailyHigh - preDailyOpen >(preDailyHigh - preDailyLow) * 0.66)
	//	barState = BEAR;
	//else if (fabs((preDailyHigh - preDailyLow) / 2 - (preDailyClose - preDailyOpen) / 2) < (preDailyHigh - preDailyLow) / 10)
	//	barState = STAR;

	//if (barState == STAR)
	//{
	//	if (daily_baseline_short - preDailyClose >=  2 * pBase_Indicators->dailyATR)
	//		barState = BULL;
	//	if (preDailyClose - daily_baseline_short >=  2 * pBase_Indicators->dailyATR)
	//		barState = BEAR;
	//}

	if (pIndicators->fast > 0
		&& (isEnableMACDSlow || pIndicators->fast > pIndicators->slow)
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& (isEnableMACDSlow || pIndicators->fast < pIndicators->slow)
		&& preDailyClose < dailyBaseLine
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	pBase_Indicators->shellingtonInTrend = trend_4H;

	//if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
	//	pBase_Indicators->shellingtonInTrend = 1;

	//if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
	//	pBase_Indicators->shellingtonInTrend = -1;

	if (fabs(iMA(3, B_DAILY_RATES, 20, 1) - iMA(3, B_DAILY_RATES, 20, 5)) / pBase_Indicators->dailyATR <= 0.05)
		pBase_Indicators->flatTrend = 1;
	else
		pBase_Indicators->flatTrend = 0;
	
	if (pBase_Indicators->dailyTrend > 0 && 
		pBase_Indicators->mACDInTrend == 1 //&& isMACDBeili == FALSE		
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == 1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BULL
		)
	{
		trend = UP;	
	}
	else if (pBase_Indicators->dailyTrend < 0 && 
		pBase_Indicators->mACDInTrend == -1 //&& isMACDBeili == FALSE	
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == -1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BEAR
		)
	{
		trend = DOWN;
	}
	//else if (pBase_Indicators->dailyTrend == 0 && pBase_Indicators->mACDInTrend == 0 && pBase_Indicators->shellingtonInTrend == 0)
	//	trend = RANGE;
	else
		trend = RANGE;

	//Override pBase_Indicators->maTrend if too close	
	//pBase_Indicators->maTrend = getMATrend(pIndicators->adjust, B_PRIMARY_RATES, 1);
	//if (pBase_Indicators->maTrend == 1)
	//{
	//	if (pIndicators->bbsTrend_secondary <= 0)
	//		pBase_Indicators->maTrend = 0;
	//}
	//if (pBase_Indicators->maTrend == -1)
	//{
	//	if (pIndicators->bbsTrend_secondary >= 0)
	//		pBase_Indicators->maTrend = 0;
	//}
		
	// Need to set stop loss

	// AUTOBBS_RANGE=1 ATR Range order
	// AUTOBBS_STARTHOUR = 8 start hour
	// AUTOBBS_IS_AUTO_MODE = 1, use R1, S1 Stop order Stop order
	//AUTOBBS_IS_AUTO_MODE = 3, cancel all orders
	//AUTOBBS_RISK_CAP=2 : stopLossLevel, by default is 2

	pIndicators->takePrice = iAtr(B_HOURLY_RATES, atrTimes, 1);
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	////////////////////
	//Check pending order and save in the virutal ordder info

	//readVirtualOrderInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderInfo);

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& (pParams->orderInfo[orderIndex].type == SELL || pParams->orderInfo[orderIndex].type == BUY)
	//	&& orderInfo.ticket > 0
	//	&& orderInfo.ticket != pParams->orderInfo[orderIndex].ticket
	//	&& orderInfo.openPrice == 
	//	)
	//{
	//	saveVirutalOrdergInfo((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->orderInfo[orderIndex]);	
	//}

	if (isEnableRSI && (rsi >= rsiHigh || rsi <= rsiLow) 
		//&& (pIndicators->fast >= 0.012 || pIndicators->fast <=-0.012)
		)
	{
		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,skip trading on rsi=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, rsi);

		return SUCCESS;
		//pIndicators->risk = 0.5;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen)
	{
		if (pParams->orderInfo[orderIndex].type == BUY
			&& iHigh(B_PRIMARY_RATES, 1) >= pParams->orderInfo[orderIndex].takeProfit)
		{
			closeLongEasy(pParams->orderInfo[orderIndex].ticket);
		}

		if (pParams->orderInfo[orderIndex].type == SELL
			&& iLow(B_PRIMARY_RATES, 1) <= pParams->orderInfo[orderIndex].takeProfit)
		{
			closeShortEasy(pParams->orderInfo[orderIndex].ticket);
		}

	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && isEnableTooBigSpread == TRUE)
	{
		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,isEnableTooBigSpread=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isEnableTooBigSpread);
		
		closeAllLimitAndStopOrdersEasy(currentTime);
		return SUCCESS;
	}
	///////////////////

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 3 || timeInfo1.tm_hour >= stopHour)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		if (isCloseOrdersEOD == TRUE && timeInfo1.tm_hour == 23 && timeInfo1.tm_min >= 50)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

		//If the order is executed on the spot of stopHour, just close it immedeiatly. 
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen)
		{
			safe_gmtime(&timeInfo3, pParams->orderInfo[orderIndex].openTime);
			if ((timeInfo3.tm_hour == stopHour && timeInfo3.tm_min < 5) || (timeInfo3.tm_hour == stopHour-1 && timeInfo3.tm_min > 55))
			{
				if (pParams->orderInfo[orderIndex].type == SELL)
					closeShortEasy(pParams->orderInfo[orderIndex].ticket);
				if (pParams->orderInfo[orderIndex].type == BUY)
					closeLongEasy(pParams->orderInfo[orderIndex].ticket);
			}
			
		}
		if (trend == UP)
			entryBuyRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, TRUE, FALSE);
		if (trend == DOWN)
			entrySellRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, TRUE, FALSE);

		return SUCCESS;
	}

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& ((pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0) 
	//	|| (pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)))
	//{		
	//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	//	return SUCCESS;
	//}


	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,startHour=%d,AUTOBBS_IS_AUTO_MODE=%d,isEnableRangeTrade=%d,pBase_dailyHigh=%lf,dailyLow=%lf,pDailyMaxATR=%lf,hourATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->startHour, (int)parameter(AUTOBBS_IS_AUTO_MODE), (int)parameter(AUTOBBS_RANGE), iHigh(B_DAILY_RATES, 0), iLow(B_DAILY_RATES, 0), pBase_Indicators->pDailyMaxATR, iAtr(B_HOURLY_RATES, 20, 1));
	

	totalLossTimes = getLossTimesInDayCloseOrderEasy(currentTime, &totalLossPoint);
	if (totalLossTimes >= 2)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		return SUCCESS;
	}

	if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour == pIndicators->startHourOnLimt && timeInfo1.tm_min < 7)
	{
		closeAllLimitAndStopOrdersEasy(currentTime); 
	}
	
	rangeType = isRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex);
	
	if ((isEnableDoubleEntry2 == TRUE || trend == UP)
		&& rangeType == 2
		&& pIndicators->bbsTrend_primary == 1
		&& pParams->orderInfo[orderIndex].isOpen == FALSE
		&& getWinTimesInDayEasy(currentTime) < 1)
	{
		splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
		resetTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID]);
		return SUCCESS;
	}

	if ((isEnableDoubleEntry2 == TRUE || trend == DOWN)
		&& rangeType == -2
		&& pIndicators->bbsTrend_primary == -1
		&& pParams->orderInfo[orderIndex].isOpen == FALSE
		&& getWinTimesInDayEasy(currentTime) < 1)
	{
		splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		resetTradingInfo((int)pParams->settings[STRATEGY_INSTANCE_ID]);
		return SUCCESS;
	}

	if (timeInfo1.tm_hour >= startHour)
	{
		//if (iHigh(B_WEEKLY_RATES, 0) - iLow(B_WEEKLY_RATES, 0) >= pBase_Indicators->weeklyATR * 0.8)
		//	autoMode = 0;


		if (trend == UP)
		{
			//TODO: check if run too far and have reverse daily bar.
			if (isEnableTooFar == TRUE && (ma960M < pBase_Indicators->dailyS2 || preDailyClose - ma960M > 0.9 * pBase_Indicators->dailyATR))
			{
				if (preDailyClose < preDailyOpen) // Bear bar
				{
					fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,ma960M=%lf, dailyS2=%lf run too far and reduce risk to 0.5.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ma960M, pBase_Indicators->dailyS2);

					//pIndicators->risk = 0.5;
					return SUCCESS;
				}
			}

			if(isEnableDoubleEntry == TRUE && rangeType == 1
				&& pIndicators->bbsTrend_primary == 1
				&& pParams->orderInfo[orderIndex].isOpen == FALSE
				&& getWinTimesInDayEasy(currentTime) < 1)
			{
				splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
				return SUCCESS;
			}


			entryBuyRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour,TRUE,TRUE);

			if (DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
				return SUCCESS;

			if (pBase_Indicators->dailyTrend <= 0)
				autoMode = 0;

			if (isEnableWeeklyATR == TRUE && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
				autoMode = 0;

			if (iLow(B_DAILY_RATES, 0) < pBase_Indicators->dailyS1)
			//if (pBase_Indicators->dailyR1 + pIndicators->takePrice - iLow(B_DAILY_RATES, 0) > pBase_Indicators->dailyATR)
				autoMode = 0;


			if ((pBase_Indicators->maTrend > 0 && timeInfo1.tm_hour >= pIndicators->startHour)
				//|| (iOpen(B_DAILY_RATES, 0) - pBase_Indicators->dailyPivot >= iAtr(B_HOURLY_RATES,20,1) && timeInfo1.tm_hour >= pIndicators->startHour)
				|| (pIndicators->bbsTrend_secondary > 0 && timeInfo1.tm_hour < pIndicators->startHour)				
				)
			{

				if (iHigh(B_DAILY_RATES, 0) - iLow(B_DAILY_RATES, 0) < pBase_Indicators->pDailyMaxATR)
					splitBuyOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);

			}
			else
				closeAllLimitAndStopOrdersEasy(currentTime);		
			
			//if (
			//	iClose(B_DAILY_RATES, 1) > iOpen(B_DAILY_RATES,1) &&
			//	iClose(B_DAILY_RATES, 2) > iOpen(B_DAILY_RATES, 2) &&
			//	iClose(B_DAILY_RATES, 3) > iOpen(B_DAILY_RATES, 3) &&
			//	(orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
			//	&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, 20, 1)) < 1
			//	//&& getWinTimesInDayEasy(currentTime) < 1		
			//	&& timeInfo1.tm_hour <= 15
			//	&& iLow(B_DAILY_RATES, 0) <= iHigh(B_DAILY_RATES, 0) - pBase_Indicators->dailyATR/2				
			//	&& (pIndicators->bbsTrend_primary == 1)
			//	)
			//{
			//	pIndicators->risk = 1;
			//	splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
			//}

			

		}
		else if (trend == DOWN)
		{
			//TODO: check if run too far and have reverse daily bar.
			if (isEnableTooFar == TRUE &&(ma960M > pBase_Indicators->dailyR2 || ma960M - preDailyClose > 0.9* pBase_Indicators->dailyATR))
			{
				if (preDailyClose > preDailyOpen) // Bull bar
				{
					fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,ma960M= %lf,dailyR2=%lf run too far and reduce risk to 0.5.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ma960M, pBase_Indicators->dailyR2);

					//pIndicators->risk = 0.5;
					return SUCCESS;
				}
			}

			if (isEnableDoubleEntry == TRUE && rangeType == -1
				&& pIndicators->bbsTrend_primary == -1
				&& pParams->orderInfo[orderIndex].isOpen == FALSE
				&& getWinTimesInDayEasy(currentTime) < 1)
			{				
				splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
				return SUCCESS;
			}

			entrySellRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, TRUE, TRUE);

			if (DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
				return SUCCESS;

			//pIndicators->executionTrend = -1;
			//pIndicators->entrySignal = -1;
			//pIndicators->exitSignal = EXIT_BUY;
			if (pBase_Indicators->dailyTrend >= 0)
				autoMode = 0;

			if (isEnableWeeklyATR == TRUE && pParams->bidAsk.ask[0] < pBase_Indicators->weeklyS2)
				autoMode = 0;

			if (iHigh(B_DAILY_RATES, 0) > pBase_Indicators->dailyR1)
			//if (iHigh(B_DAILY_RATES, 0) - (pBase_Indicators->dailyS1 - pIndicators->takePrice)> pBase_Indicators->dailyATR)
				autoMode = 0;

			if ((pBase_Indicators->maTrend < 0 && timeInfo1.tm_hour >= pIndicators->startHour)
				//|| (pBase_Indicators->dailyPivot - iOpen(B_DAILY_RATES, 0) >= iAtr(B_HOURLY_RATES, 20,1) && timeInfo1.tm_hour >= pIndicators->startHour)
				|| (pIndicators->bbsTrend_secondary < 0 && timeInfo1.tm_hour < pIndicators->startHour)
				&& DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators)
				)
			{

				if (iHigh(B_DAILY_RATES, 0) - iLow(B_DAILY_RATES, 0) < pBase_Indicators->pDailyMaxATR)
					splitSellOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);
			}
			else
				closeAllLimitAndStopOrdersEasy(currentTime);

			//if (
			//	iClose(B_DAILY_RATES, 1) < iOpen(B_DAILY_RATES, 1) &&
			//	iClose(B_DAILY_RATES, 2) < iOpen(B_DAILY_RATES, 2) &&
			//	iClose(B_DAILY_RATES, 3) < iOpen(B_DAILY_RATES, 3) &&
			//	(orderIndex < 0 || pParams->orderInfo[orderIndex].isOpen == FALSE)
			//	&& getWinTimesInDaywithSamePriceEasy(currentTime, pParams->bidAsk.ask[0], 3 * iAtr(B_HOURLY_RATES, 20, 1)) < 1
			//	//&& getWinTimesInDayEasy(currentTime) < 1		
			//	&& timeInfo1.tm_hour <= 15				
			//	&& iHigh(B_DAILY_RATES, 0) >= iLow(B_DAILY_RATES, 0) + pBase_Indicators->dailyATR/2
			//	&& pIndicators->bbsTrend_primary == -1
			//	)
			//{
			//	pIndicators->risk = 1;
			//	splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
			//}

		}
		else if (trend == RANGE  && isEnableRangeTrade == TRUE)
		{
			pIndicators->risk = 0.5;
			entryBuyRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, FALSE, TRUE);
			entrySellRangeOrder(pParams, pIndicators, pBase_Indicators, orderIndex, stopHour, FALSE, TRUE);
		}

	}


	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen 
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 4 * 60
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) <= 6 * 60
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		)
	{
		//By default, AUTOBBS_TP_MODE = 1
		//If AUTOBBS_RANGE = 1 and hour >15, should set isMoveTP = 0, no need move TP.
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		if (timeInfo2.tm_hour >= 15
			&& (
			(pParams->orderInfo[orderIndex].type == BUY && pParams->orderInfo[orderIndex].openPrice < iHigh(B_DAILY_RATES, 0) - pBase_Indicators->pDailyMaxATR)
			|| (pParams->orderInfo[orderIndex].type == SELL && pParams->orderInfo[orderIndex].openPrice > iLow(B_DAILY_RATES, 0) + pBase_Indicators->pDailyMaxATR)
			)
			)
			isMoveTP = 0;

		//Move stopLoss when 1 TP is hit.
		
		realTakePrice = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / stopLossLevel;

		if (pParams->orderInfo[orderIndex].type == BUY)
		{			
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice;
			pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;

			//if (pParams->bidAsk.ask[0] >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit > pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			if (pIndicators->entryPrice < pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = 1;				
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iHigh(B_PRIMARY_RATES, 1) >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			{
				fprintf(stderr, "[INFO] closeLong type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeLongEasy(pParams->orderInfo[orderIndex].ticket);				
			}

			return SUCCESS;
		}

		if (pParams->orderInfo[orderIndex].type == SELL)
		{			
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice* stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice;
			pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;

			//if (pParams->bidAsk.bid[0] <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit< pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			if (pIndicators->entryPrice > pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = -1;
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iLow(B_PRIMARY_RATES, 1) <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			{
				fprintf(stderr, "[INFO] closeShort type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeShortEasy(pParams->orderInfo[orderIndex].ticket);				
			}

			return SUCCESS;
		}

		if (isMoveTP == 1 || timeInfo1.tm_yday != timeInfo2.tm_yday)
		{	

			if (pParams->orderInfo[orderIndex].type == BUY		
				&& pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust < pParams->orderInfo[orderIndex].takeProfit
				&& pParams->orderInfo[orderIndex].openPrice - lowPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{

				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust;
				pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;
				return SUCCESS;
			}

			if (pParams->orderInfo[orderIndex].type == SELL
				&& pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust > pParams->orderInfo[orderIndex].takeProfit
				&& highPrice - pParams->orderInfo[orderIndex].openPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust;
				pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;
				return SUCCESS;
			}

		}
	}

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL 
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen		
		&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 9 * 60)
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		//&& timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour <= 16
		)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		if (timeInfo2.tm_hour >= 15 && timeInfo2.tm_hour <= 16)		
		{
			if (timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour <= 16)
			{
				move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, 1.0);
			}
			else if (timeInfo1.tm_hour > 16)
			{
				move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, stopLossLevel);
			}
						
		}
		

	}

	//if ((strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	//	&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 9 * 60)
	//	&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
	//	)
	//{
	//	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

	// 1 to 1 (risk/reward ratio))
	//	{
	// 1 to 1 (risk/reward ratio))
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, 1.0);
	//		}
	//		else if (timeInfo1.tm_hour > 16)
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, stopLossLevel);
	//		}

	//	}


	//}

	//if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL 
	//	&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE || difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 9 * 60)
	//	&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
	//	)
	//{
	//	safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

	//	if (timeInfo2.tm_hour >= 8 && timeInfo2.tm_hour <= 14)
	//	{
	//		if (timeInfo1.tm_hour >= 8 && timeInfo1.tm_hour <= 14)
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, 1.0);
	//		}
	//		else if (timeInfo1.tm_hour > 14)
	//		{
	//			move_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex, stopLossLevel);
	//		}

	//	}


	//}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Limit_BreakOutOnPivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2, timeInfo3;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose, preDailyHigh, preDailyLow, preDailyOpen;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double moveTPLimit = (double)parameter(AUTOBBS_KEYK);
	int autoMode = (int)parameter(AUTOBBS_IS_AUTO_MODE);	
	BOOL isCloseOrdersEOD = FALSE;
	int orderIndex = -1,closeOrderIndex = -1;
	double highPrice, lowPrice;
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);
	BOOL isMoveTPInNewDay = TRUE;
	int closeHour = 23, startHour = pIndicators->startHour;
	BOOL isEnableRangeTrade = FALSE;
	int trend = UNKNOWN;
	int truningPointIndex = -1, minPointIndex = -1;
	double isMACDBeili = FALSE;
	double totalLossPoint = 0;
	int totalLossTimes = 0;
	double realTakePrice;
	BOOL isEnableMACDSlow = TRUE;
	BOOL isEnableShellingtonTrend = FALSE;
	BOOL isEnableFlatTrend = FALSE;
	BOOL isEnableWeeklyATR = TRUE;
	BOOL isEnableTooFar = FALSE;
	int stopHour = 23;
	double tooFarLimit = 1;
	int barState = BAR_UNKNOWN;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0;
	double daily_baseline = 0.0, daily_baseline_short = 0.0;
	double rsi = 0.0;
	BOOL isEnableDoubleEntry = FALSE, isEnableDoubleEntry2 = FALSE;
	int rangeType = 0;
	Order_Turning_Info orderTurningInfo;
	int atrTime = 20;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 4;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;
	pIndicators->isEnableLimitSR1 = FALSE;

	pIndicators->takePriceLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);

	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getLastestOrderIndexExceptLimitAndStopOrdersEasy(B_PRIMARY_RATES,FALSE);
	closeOrderIndex = getLastestOrderIndexExceptLimitAndStopOrdersEasy(B_PRIMARY_RATES, TRUE);
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, 5 * 60, orderIndex, &highPrice, &lowPrice);

	pIndicators->adjust = fabs(pParams->bidAsk.bid[0] - pParams->bidAsk.ask[0]);

    if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		if (timeInfo1.tm_wday == 3)
			pIndicators->risk = 0.6;

		startHour = 3;

		//isEnableFlatTrend = TRUE;
		//isEnableShellingtonTrend = TRUE;

		pIndicators->startHourOnLimt = startHour;
		isCloseOrdersEOD = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{

		if (timeInfo1.tm_wday == 3)
			pIndicators->risk = 0.6;

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		startHour = 0;
		pIndicators->startHourOnLimt = 8;

		stopHour = 22;

		// filter non-farm payroll day

		if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
		{

			strcpy(pIndicators->status, "Filter Non-farm day\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		if (XAUUSD_not_full_trading_day(pParams, pIndicators, pBase_Indicators) == TRUE)
		{
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			return SUCCESS;
		}



		isEnableShellingtonTrend = TRUE;		
		isEnableTooFar = FALSE;
	}	
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		pIndicators->adjust = pBase_Indicators->dailyATR * 0.01;

		startHour = 0;
		pIndicators->startHourOnLimt = startHour;
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		if (timeInfo1.tm_wday == 1 || timeInfo1.tm_wday == 2)
			pIndicators->risk = 0.5;

		//filter christmas eve and new year eve
		if (timeInfo1.tm_mon == 11 && (timeInfo1.tm_mday == 24 || timeInfo1.tm_mday == 31))
		{
			strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			return SUCCESS;
		}

		isEnableWeeklyATR = FALSE;

		//pIndicators->isEnableLimitSR1 = TRUE;

		//if (timeInfo1.tm_wday == 6)
		//	stopHour = 16;

		tooFarLimit = 2;

		isEnableRangeTrade = FALSE;
		fastMAPeriod = 7;
		slowMAPeriod = 14;
		signalMAPeriod = 7;

		if (timeInfo1.tm_wday == 0)
			atrTime = 50;

	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			&& (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
			|| pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
			)
			isCloseOrdersEOD = TRUE;

		stopHour = 17;

		isEnableMACDSlow = FALSE;
		isEnableFlatTrend = TRUE;

		//isEnableRangeTrade = TRUE;
		pIndicators->startHourOnLimt = pIndicators->startHour;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE)
		pIndicators->adjust = 0;


	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
		startShift = 0;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	//rsi = iRSI(B_DAILY_RATES, 14, 1);
	//isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	preDailyHigh = iHigh(B_DAILY_RATES, startShift);
	preDailyLow = iLow(B_DAILY_RATES, startShift);
	preDailyOpen = iOpen(B_DAILY_RATES, startShift);

	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;


	if (pIndicators->fast > 0
		&& (isEnableMACDSlow || pIndicators->fast > pIndicators->slow)
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& (isEnableMACDSlow || pIndicators->fast < pIndicators->slow)
		&& preDailyClose < dailyBaseLine
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	pBase_Indicators->shellingtonInTrend = trend_4H;

	if (fabs(iMA(3, B_DAILY_RATES, 20, 1) - iMA(3, B_DAILY_RATES, 20, 5)) / pBase_Indicators->dailyATR <= 0.05)
		pBase_Indicators->flatTrend = 1;
	else
		pBase_Indicators->flatTrend = 0;

	if (pBase_Indicators->dailyTrend > 0 &&
		pBase_Indicators->mACDInTrend == 1 //&& isMACDBeili == FALSE		
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == 1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BULL
		)
	{
		trend = UP;
	}
	else if (pBase_Indicators->dailyTrend < 0 &&
		pBase_Indicators->mACDInTrend == -1 //&& isMACDBeili == FALSE	
		&& (isEnableShellingtonTrend == FALSE || pBase_Indicators->shellingtonInTrend == -1)
		&& (isEnableFlatTrend == FALSE || pBase_Indicators->flatTrend == 0)
		//&& barState == BEAR
		)
	{
		trend = DOWN;
	}
	else
		trend = RANGE;

	pIndicators->takePrice = iAtr(B_HOURLY_RATES, atrTime, 1);
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
	//	&& iAtr(B_DAILY_RATES, 1, 0) >= pBase_Indicators->pDailyMaxATR)
	//{
	//	if (pParams->orderInfo[orderIndex].type == SELL)
	//		closeShortEasy(pParams->orderInfo[orderIndex].ticket);
	//	if (pParams->orderInfo[orderIndex].type == BUY)
	//		closeLongEasy(pParams->orderInfo[orderIndex].ticket);
	//}

	if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 3 || timeInfo1.tm_hour >= stopHour)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		if (isCloseOrdersEOD == TRUE && timeInfo1.tm_hour == 23 && timeInfo1.tm_min >= 50)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

		//If the order is executed on the spot of stopHour, just close it immedeiatly. 
		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen)
		{
			safe_gmtime(&timeInfo3, pParams->orderInfo[orderIndex].openTime);
			if ((timeInfo3.tm_hour == stopHour && timeInfo3.tm_min < 5) || (timeInfo3.tm_hour == stopHour - 1 && timeInfo3.tm_min > 55))
			{
				if (pParams->orderInfo[orderIndex].type == SELL)
					closeShortEasy(pParams->orderInfo[orderIndex].ticket);
				if (pParams->orderInfo[orderIndex].type == BUY)
					closeLongEasy(pParams->orderInfo[orderIndex].ticket);
			}

		}

		return SUCCESS;
	}

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s,startHour=%d,AUTOBBS_IS_AUTO_MODE=%d,isEnableRangeTrade=%d,pBase_dailyHigh=%lf,dailyLow=%lf,pDailyMaxATR=%lf,hourATR=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->startHour, (int)parameter(AUTOBBS_IS_AUTO_MODE), (int)parameter(AUTOBBS_RANGE), iHigh(B_DAILY_RATES, 0), iLow(B_DAILY_RATES, 0), pBase_Indicators->pDailyMaxATR, iAtr(B_HOURLY_RATES, 20, 1));
	totalLossTimes = getLossTimesInDayCloseOrderEasy(currentTime, &totalLossPoint);
	if (totalLossTimes >= 1)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
		return SUCCESS;
	}

	if (pIndicators->isEnableLimitSR1 == TRUE && timeInfo1.tm_hour == pIndicators->startHourOnLimt && timeInfo1.tm_min < 7)
	{
		closeAllLimitAndStopOrdersEasy(currentTime);
	}

	iTrend3Rules(pParams, pIndicators, B_DAILY_RATES, 2, &(pBase_Indicators->daily3RulesTrend), 0);

	///////////////////////////////////////////////	

	if (orderIndex < 0)
	{
		orderTurningInfo.isTurning = TRUE;
		saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
	}
	
	if (readTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo) == -1)
	{
		orderTurningInfo.isTurning = TRUE;
		saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
	}
	
	if (orderTurningInfo.isTurning == TRUE
		//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE
		&& closeOrderIndex >=0 
		&& pParams->orderInfo[closeOrderIndex].profit > 0
		)
	{
		if (fabs(pParams->orderInfo[closeOrderIndex].closePrice - pParams->orderInfo[closeOrderIndex].openPrice) / pIndicators->takePrice >= 2)
		{
			orderTurningInfo.type = pParams->orderInfo[closeOrderIndex].type;
			orderTurningInfo.isTurning = FALSE;
			saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
		}
	}
	else if (orderTurningInfo.isTurning == FALSE
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		&& pParams->orderInfo[orderIndex].type != orderTurningInfo.type
		)
	{		
		orderTurningInfo.type = pParams->orderInfo[orderIndex].type;
		orderTurningInfo.isTurning = TRUE;
		saveTurningPoint((int)pParams->settings[STRATEGY_INSTANCE_ID], &orderTurningInfo);
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && orderTurningInfo.isTurning == FALSE)
		pIndicators->takePriceLevel = 1;
	///////////////////////////////////////////////

	if (timeInfo1.tm_hour >= startHour)
	{

		//if (DailyTrade_Limit_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
		//	return SUCCESS;

		if (trend == UP)
		{
			if (pBase_Indicators->daily3RulesTrend == UP)
			{
				autoMode = 0;


				splitBuyOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);
			}
		}
		
		if (trend == DOWN)
		{
			if (pBase_Indicators->daily3RulesTrend == DOWN)
			{
				autoMode = 0;
				splitSellOrders_Limit(pParams, pIndicators, pBase_Indicators, autoMode, pIndicators->takePrice, pIndicators->stopLoss);
			}
		}

	}

	//move_tailing_stop_loss(pParams, pIndicators, pBase_Indicators, orderIndex);


	if ((BOOL)pParams->settings[IS_BACKTESTING] == TRUE 
		&& orderTurningInfo.isTurning == FALSE
		&& orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) >= 4 * 60
		&& difftime(currentTime, pParams->orderInfo[orderIndex].openTime) <= 6 * 60
		&& (pParams->orderInfo[orderIndex].type == BUY || pParams->orderInfo[orderIndex].type == SELL)
		)
	{
		//By default, AUTOBBS_TP_MODE = 1
		//If AUTOBBS_RANGE = 1 and hour >15, should set isMoveTP = 0, no need move TP.
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);

		if (timeInfo2.tm_hour >= 15
			&& (
			(pParams->orderInfo[orderIndex].type == BUY && pParams->orderInfo[orderIndex].openPrice < iHigh(B_DAILY_RATES, 0) - pBase_Indicators->pDailyMaxATR)
			|| (pParams->orderInfo[orderIndex].type == SELL && pParams->orderInfo[orderIndex].openPrice > iLow(B_DAILY_RATES, 0) + pBase_Indicators->pDailyMaxATR)
			)
			)
			isMoveTP = 0;

		//Move stopLoss when 1 TP is hit.

		realTakePrice = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / stopLossLevel;

		if (pParams->orderInfo[orderIndex].type == BUY)
		{
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice;
			pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;

			//if (pParams->bidAsk.ask[0] >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit > pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			if (pIndicators->entryPrice < pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = 1;
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iHigh(B_PRIMARY_RATES, 1) >= pParams->orderInfo[orderIndex].openPrice + realTakePrice)
			{
				fprintf(stderr, "[INFO] closeLong type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeLongEasy(pParams->orderInfo[orderIndex].ticket);
			}

			return SUCCESS;
		}

		if (pParams->orderInfo[orderIndex].type == SELL)
		{
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice* stopLossLevel;

			pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice;
			pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;

			//if (pParams->bidAsk.bid[0] <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			//if (pParams->orderInfo[orderIndex].takeProfit< pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			if (pIndicators->entryPrice > pIndicators->takeProfitPrice)
			{
				pIndicators->executionTrend = -1;
			}
			//else
			//{
			//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
			//}
			else if (iLow(B_PRIMARY_RATES, 1) <= pParams->orderInfo[orderIndex].openPrice - realTakePrice)
			{
				fprintf(stderr, "[INFO] closeShort type = %d, ticket = %d", (int)pParams->orderInfo[orderIndex].type, (int)pParams->orderInfo[orderIndex].ticket);
				closeShortEasy(pParams->orderInfo[orderIndex].ticket);
			}

			return SUCCESS;
		}

		if (isMoveTP == 1 || timeInfo1.tm_yday != timeInfo2.tm_yday)
		{

			if (pParams->orderInfo[orderIndex].type == BUY
				&& pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust < pParams->orderInfo[orderIndex].takeProfit
				&& pParams->orderInfo[orderIndex].openPrice - lowPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{

				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->adjust;
				pIndicators->takePrice = pIndicators->takeProfitPrice - pIndicators->entryPrice;
				return SUCCESS;
			}

			if (pParams->orderInfo[orderIndex].type == SELL
				&& pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust > pParams->orderInfo[orderIndex].takeProfit
				&& highPrice - pParams->orderInfo[orderIndex].openPrice >  moveTPLimit * fabs(pParams->orderInfo[orderIndex].openPrice - pParams->orderInfo[orderIndex].stopLoss)
				)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				pIndicators->takeProfitPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->adjust;
				pIndicators->takePrice = pIndicators->entryPrice - pIndicators->takeProfitPrice;
				return SUCCESS;
			}

		}
	}

	

	
	return SUCCESS;
}

static BOOL move_tailing_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex)
{
	OrderType side = pParams->orderInfo[orderIndex].type;

	if (side == BUY)
	{
		if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > 2* pIndicators->takePrice 
			//&& pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * pIndicators->takePrice
			)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}
		//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 3 * pIndicators->takePrice
		//	&& pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = 1;
		//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->takePrice;
		//}
		//else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 4 * pIndicators->takePrice
		//	//&& pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = 1;
		//	pIndicators->entryPrice = pParams->bidAsk.ask[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + 2 * pIndicators->takePrice;
		//}
	}

	if (side == SELL)
	{
		if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > 2 * pIndicators->takePrice
			//&& pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->takePrice
			)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}
		//else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * pIndicators->takePrice
		//	&& pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = -1;
		//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->takePrice;
		//}
		//else if(pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 4 * pIndicators->takePrice
		//	//&& pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 4 * pIndicators->takePrice
		//	)
		//{
		//	pIndicators->executionTrend = -1;
		//	pIndicators->entryPrice = pParams->bidAsk.bid[0];
		//	pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - 2 * pIndicators->takePrice;
		//}

	}
	return TRUE;
}

static BOOL move_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, double stopLossLevel)
{

	//double realTakePrice = fabs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / oldStopLossLevel;
	double realTakePrice = fabs(pParams->orderInfo[orderIndex].takeProfit - pParams->orderInfo[orderIndex].openPrice);

	pIndicators->stopMovingBackSL = FALSE;
	if (pParams->orderInfo[orderIndex].type == BUY)
	{
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice * stopLossLevel;

		pIndicators->takePrice = -1;

		if (fabs(pIndicators->stopLossPrice- pParams->orderInfo[orderIndex].stopLoss) > pIndicators->adjust)
			pIndicators->executionTrend = 1;
		
	}

	if (pParams->orderInfo[orderIndex].type == SELL)
	{
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice * stopLossLevel;

		pIndicators->takePrice = -1;

		if (fabs(pIndicators->stopLossPrice- pParams->orderInfo[orderIndex].stopLoss) > pIndicators->adjust)
			pIndicators->executionTrend = -1;		
	}
	return TRUE;
}

static BOOL entryBuyRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder)
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

static BOOL entrySellRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder)
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

static int isRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex)
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
static BOOL DailyTrade_Limit_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	time_t currentTime;
	struct tm timeInfo1;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	//Ignore any trade on time range:
	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		if (timeInfo1.tm_hour >= 10)
		{
			sprintf(pIndicators->status, "Ignore trading after 10\n");
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			closeAllLimitAndStopOrdersEasy(currentTime);
			return FALSE;
		}
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		if (timeInfo1.tm_hour >= 8 && timeInfo1.tm_hour <= 14)
		{
			sprintf(pIndicators->status, "Ignore trading between 8-14\n");
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			closeAllLimitAndStopOrdersEasy(currentTime);
			return FALSE;
		}
	}
	return TRUE;
}

AsirikuyReturnCode workoutExecutionTrend_Limit_BBS(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double macdMaxLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE); 
	int orderIndex = -1;
	int isMoveTP = (int)parameter(AUTOBBS_TP_MODE);	
	int closeHour = 23, startHour = 2;	
	int trend = UNKNOWN;
	double realTakePrice;
	int exitMode = (int)parameter(AUTOBBS_IS_AUTO_MODE);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 4;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;
	
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
		startShift = 0;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	if (pIndicators->fast > 0
		&& pIndicators->fast > pIndicators->slow
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& pIndicators->fast < pIndicators->slow
		&& preDailyClose < dailyBaseLine
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		pBase_Indicators->shellingtonInTrend = 1;

	if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		pBase_Indicators->shellingtonInTrend = -1;

	if (pBase_Indicators->dailyTrend > 0 && pBase_Indicators->mACDInTrend == 1
		)
	{
		trend = UP;
	}
	else if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1
		)
	{
		trend = DOWN;
	}
	else 
		trend = RANGE;

	// Need to set stop loss

	// AUTOBBS_RANGE=1 ATR Range order
	// AUTOBBS_STARTHOUR = 8 start hour
	// AUTOBBS_IS_AUTO_MODE = 1, use R1, S1 Stop order Stop order
	//AUTOBBS_IS_AUTO_MODE = 3, cancel all orders
	//AUTOBBS_RISK_CAP=2 : stopLossLevel, by default is 2

	//if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	//{
	//	
	//	isCloseOrdersEOD = TRUE;		
	//}
	//else if (strstr(pParams->tradeSymbol, "EURUSD") != NULL)
	//{
	//	
	//	isCloseOrdersEOD = TRUE;
	//}
	//else if (strstr(pParams->tradeSymbol, "EURGBP") != NULL)
	//{
	//	
	//	isCloseOrdersEOD = TRUE;
	//}
	//else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	//{		
	//	isCloseOrdersEOD = TRUE;
	//}


	pIndicators->takePrice = pBase_Indicators->dailyATR / 3;
	pIndicators->stopLoss = stopLossLevel * pIndicators->takePrice;
	pIndicators->stopLossPrice = 0; // No moving 
	pIndicators->stopMovingBackSL = TRUE;
	pIndicators->entrySignal = 0;

	//Move to break event if it moves to more than 2 *TP 
	orderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);
	//getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, pIndicators->executionRateTF * 60, orderIndex, &highPrice, &lowPrice);

	// By default isMoveTP = 1
	// But when AUTOBBS_IS_AUTO_MODE == 1, we should set isMoveTP = 0 because BBS30 will exit orders.
	if (isMoveTP == 1)
	{
		if (orderIndex >= 0)
		{
			realTakePrice = fabs(pParams->orderInfo[orderIndex].takeProfit - pParams->orderInfo[orderIndex].openPrice) / stopLossLevel;

			safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
				
			if (pParams->orderInfo[orderIndex].type == BUY)
			{
				if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= realTakePrice && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 2 * realTakePrice
					)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					exitMode = 0;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 2 * realTakePrice  && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * realTakePrice)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + realTakePrice;
					exitMode = 0;
				}
				else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 3 * realTakePrice)
				{
					pIndicators->executionTrend = 1;
					pIndicators->entryPrice = pParams->bidAsk.ask[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + 2* realTakePrice;
					exitMode = 0;					
				}
			}

			if (pParams->orderInfo[orderIndex].type == SELL)
			{
				if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= realTakePrice && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * realTakePrice
					)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					exitMode = 0;
				}
				else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * realTakePrice && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * realTakePrice)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - realTakePrice;
					exitMode = 0;
				}
				else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 3 * realTakePrice)
				{
					pIndicators->executionTrend = -1;
					pIndicators->entryPrice = pParams->bidAsk.bid[0];
					pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - 2 * realTakePrice;
					exitMode = 0;
				}
			}

		}
	}


	if (trend == UP)
	{
		
		//pIndicators->executionTrend = 1;
		if ((orderIndex < 0)
			&& pIndicators->fast < macdMaxLevel
			&& timeInfo1.tm_hour >= pIndicators->startHour
			&& pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index)
			splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);

		if (exitMode == 1 && pIndicators->bbsTrend_excution == -1)
			closeAllLongs();
		else
			closeAllShorts();
	}
	else if (trend == DOWN)
	{
		//pIndicators->executionTrend = -1;
		if ((orderIndex < 0) 
			&& pIndicators->fast > -1 * macdMaxLevel
			&& timeInfo1.tm_hour >= pIndicators->startHour
			&& pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index)
			splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);

		if (exitMode == 1 && pIndicators->bbsTrend_excution == 1)
			closeAllShorts();
		else
			closeAllLongs();
	}
	else
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}

		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
	}


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Limit_BBS_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);
	double preHist1, preHist2;
	double fast1, fast2;
	double slow1, slow2;
	double dailyBaseLine;
	int fastMAPeriod = 5, slowMAPeriod = 10, signalMAPeriod = 5;
	int startShift = 1;
	double preDailyClose;
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	int entryMode = 1; // 1 to 1 (risk/reward ratio) 3: ATR range
	double stopLossLevel = (double)parameter(AUTOBBS_RISK_CAP);
	double macdMaxLevel = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);	
	int orderIndex = -1;
	double highPrice, lowPrice;
	BOOL isMoveTP = TRUE;	
	int closeHour = 23, startHour = 2;	
	int trend = UNKNOWN;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 31;
	pIndicators->tpMode = 0;

	pIndicators->executionTrend = 0;

	pIndicators->risk = 1;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
	if (timeInfo1.tm_hour >= 23 && timeInfo1.tm_min >= 30)
		startShift = 0;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;
	pIndicators->preFast = fast2;
	pIndicators->preSlow = slow2;

	preDailyClose = iClose(B_DAILY_RATES, startShift);
	dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

	pBase_Indicators->mACDInTrend = 0;
	pBase_Indicators->shellingtonInTrend = 0;

	if (pIndicators->fast > 0
		&& pIndicators->fast > pIndicators->slow
		&& preDailyClose > dailyBaseLine
		) // Buy
	{
		pBase_Indicators->mACDInTrend = 1;
	}

	if (pIndicators->fast < 0
		&& pIndicators->fast < pIndicators->slow
		&& preDailyClose < dailyBaseLine
		)//Sell
	{
		pBase_Indicators->mACDInTrend = -1;
	}

	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	if (trend_MA > 0)
		trend_4H = 1;
	else if (trend_MA < 0)
		trend_4H = -1;

	if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		pBase_Indicators->shellingtonInTrend = 1;

	if (trend_4H == -1 && pIndicators->bbsTrend_4H == -1)
		pBase_Indicators->shellingtonInTrend = -1;

	if (pBase_Indicators->dailyTrend > 0 && pBase_Indicators->mACDInTrend == 1
		)
	{
		trend = UP;
	}
	else if (pBase_Indicators->dailyTrend < 0 && pBase_Indicators->mACDInTrend == -1
		)
	{
		trend = DOWN;
	}
	else
		trend = RANGE;

	
	//Find the highest close price after order is opened			
	getHighLowPrice(pParams, pIndicators, pBase_Indicators, B_PRIMARY_RATES, pIndicators->executionRateTF * 60, orderIndex, &highPrice, &lowPrice);
	pIndicators->takePrice = pBase_Indicators->dailyATR;
	//pIndicators->takePrice = 0;
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, trend =%d, bbsTrend_excution=%d",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, trend, pIndicators->bbsTrend_excution);
	if (trend == UP)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = max(highPrice, pIndicators->entryPrice) - pBase_Indicators->pDailyMaxATR;		
		
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->fast < macdMaxLevel
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR/3)
			)
			pIndicators->entrySignal = 1;

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && pIndicators->bbsTrend_excution == -1)
			pIndicators->exitSignal = EXIT_ALL;
		else
			pIndicators->exitSignal = EXIT_SELL;
	}
	else if (trend == DOWN)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = min(lowPrice, pIndicators->entryPrice) + pBase_Indicators->pDailyMaxATR;

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->fast > -1 *  macdMaxLevel
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)
			)
			pIndicators->entrySignal = -1;

		if ((int)parameter(AUTOBBS_IS_AUTO_MODE) == 1 && pIndicators->bbsTrend_excution == 1)
			pIndicators->exitSignal = EXIT_ALL;
		else
			pIndicators->exitSignal = EXIT_BUY;
	}
	else
		pIndicators->exitSignal = EXIT_ALL;


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_ASI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double asiBull, asiBear;

	int orderIndex;
	double stopLoss;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	//load pBase_Indicators
	pBase_Indicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;
	stopLoss = pBase_Indicators->pDailyMaxATR;

	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, asiBull =%lf, asiBear=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, asiBull, asiBear);


	if (asiBull > asiBear)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

		if (orderIndex >= 0 &&
			pParams->orderInfo[orderIndex].type == BUY &&
			pParams->orderInfo[orderIndex].isOpen == TRUE &&
			pIndicators->stopLossPrice > pParams->orderInfo[orderIndex].openPrice)
		{

			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}

		if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
			&& iClose(B_DAILY_RATES, 1) > iClose(B_DAILY_RATES,2))
		{
			pIndicators->entrySignal = 1;
		}

		pIndicators->exitSignal = EXIT_SELL;
	}

	if (asiBull < asiBear)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];

		pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

		if (orderIndex >= 0 &&
			pParams->orderInfo[orderIndex].type == SELL &&
			pParams->orderInfo[orderIndex].isOpen == TRUE &&
			pIndicators->stopLossPrice < pParams->orderInfo[orderIndex].openPrice)
		{

			pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
		}

		if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
			&& iClose(B_DAILY_RATES, 1) < iClose(B_DAILY_RATES, 2))
		{
			pIndicators->entrySignal = -1;
		}
		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_MACD_Daily_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shiftPreDayBar = shift1Index - 1;
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";	
	int orderIndex;
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);	
	double level = 0, histLevel = 0, maxLevel = 0;
	

	double ma20Daily, preDailyClose;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;

	//double cmfVolume = 0.0;
	BOOL isVolumeControl = TRUE;
	BOOL isEnableBeiLi = TRUE;
	BOOL isEnableSlow = TRUE;
	BOOL isEnableATR = TRUE;
	BOOL isEnableCMFVolume = FALSE;
	BOOL isEnableCMFVolumeGap = FALSE;
	BOOL isEnableMaxLevel = FALSE;

	BOOL isVolumeControlRisk = FALSE;
	BOOL isCMFVolumeRisk = FALSE;
	BOOL isCMFVolumeGapRisk = FALSE;
	BOOL isAllVolumeRisk = FALSE;

	BOOL isWeeklyBaseLine = FALSE;

	BOOL isDailyOnly = TRUE;

	BOOL isMACDZeroExit = FALSE;

	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int pre3KTrend;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	//double CMFVolumeGap = 0.0;
	double stopLoss = pBase_Indicators->pDailyMaxATR;
	double maxRisk = 2;

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;

	int truningPointIndex = -1, minPointIndex = -1;	
	double turningPoint, minPoint;

	//double atr5Limit = pParams->bidAsk.ask[0] * 0.01 *0.55;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// Determine trend direction from daily chart

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	ma20Daily = iMA(3, B_DAILY_RATES, 20, startShift);
	preDailyClose = iClose(B_DAILY_RATES, startShift);

	if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
	{

		level = 0.35;
		//maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;
		isWeeklyBaseLine = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = TRUE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = FALSE;

		isEnableEntryEOD = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 10;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		isCMFVolumeGapRisk = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		maxRisk = 2;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

	}
	else if (strstr(pParams->tradeSymbol, "XAUEUR") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 8;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;

		isEnableSlow = FALSE;

		isEnableATR = TRUE;		
		isEnableCMFVolume = TRUE;		

		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 2;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		isMACDZeroExit = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "GBPCHF") != NULL)
	{
		level = 0.002; //GBPJPY
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.5;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "EURJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = TRUE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "CADJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = TRUE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		level = 0.001; //GBPAUD
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		isAllVolumeRisk = TRUE;

		//stopLoss = pBase_Indicators->dailyATR * 1.5;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		level = 0.001; //GBPUSD
		maxLevel = 0.007;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevel = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		level = 0.0025; //GBPJPY
		maxLevel = 0;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 12;
		slowMAPeriod = 26;
		signalMAPeriod = 9;
	}
	else
	{
		level = 0; //EURUSD
		maxLevel = 0.005;

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}

	// After 23:00, check if need to enter
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;
		macdLimit = level / 2;

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);


	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= 1
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 start, avoid the first hour
	{

		preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
		iTrend3Rules_preDays(pParams, pIndicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, startShift);

		pIndicators->CMFVolumeGap = getCMFVolumeGap(B_DAILY_RATES, 1, fastMAPeriod, startShift);

		//Volume indicator....
		// preVolume > MA: current volume > past average volume
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		//volume_ma_10 = iMA(4, B_DAILY_RATES, 10, startShift);

		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->cmfVolume, pIndicators->CMFVolumeGap, weekly_baseline, weekly_baseline_short, pIndicators->volume1, pIndicators->volume2);

		//Load MACD
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);

		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;


		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift + 1), pIndicators->preFast, pIndicators->preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);



		if (pIndicators->fast > level
			&& (isEnableSlow == FALSE || pIndicators->slow > 0)
			//&& dailyTrend > 0 			
			&& preDailyClose > ma20Daily
			&& pIndicators->fast - pIndicators->slow > macdLimit
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pDailyMaxATR);
			
			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

			if (orderIndex >= 0 && 
				pParams->orderInfo[orderIndex].type == BUY &&
				pParams->orderInfo[orderIndex].isOpen == TRUE &&
				pIndicators->stopLossPrice > pParams->orderInfo[orderIndex].openPrice)
			{				

				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
			}
			
			//pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;


			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			if (//fast > slow && preFast <= preSlow &&				
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast > pIndicators->preFast
				&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
				&& (isVolumeControl == FALSE || pIndicators->volume1 > pIndicators->volume2)
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume > 0)
				&& (isEnableCMFVolumeGap == FALSE || pIndicators->CMFVolumeGap > 0)
				//&& volume1 > volume_ma_10
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose > weekly_baseline && (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
				)
				&& pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) <= 0.2 * pBase_Indicators->dailyATR
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);
				if (timeInfo1.tm_mday != timeInfo2.tm_mday)
				{
					pIndicators->entrySignal = 1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap > 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume > 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap > 0 && pIndicators->cmfVolume >  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					//if (pParams->orderInfo[orderIndex].type == BUY &&
					//	pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
					//{
					//	pIndicators->tradeMode = 2;
					//}

					// If price retreats back, add position
					if (preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
						&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						)
					{
						pIndicators->entrySignal = 0;
					}

					if (isEnableMaxLevel == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& pIndicators->fast > maxLevel
						)
					{
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint,&minPointIndex,&minPoint))
					{
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}


			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (pIndicators->fast < (level * -1)
			&& (isEnableSlow == FALSE || pIndicators->slow < 0)
			//&& dailyTrend < 0			
			&& preDailyClose < ma20Daily
			&& pIndicators->slow - pIndicators->fast > macdLimit
			//			&& fast < preFast			
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pDailyMaxATR);

			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;

			if (orderIndex >= 0 && 
				pParams->orderInfo[orderIndex].type == SELL &&
				pParams->orderInfo[orderIndex].isOpen == TRUE &&
				pIndicators->stopLossPrice < pParams->orderInfo[orderIndex].openPrice)
			{

				pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
			}

			//pIndicators->stopLossPrice = pIndicators->entryPrice + 1.5 * pBase_Indicators->dailyATR;

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast < pIndicators->preFast
				&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
				&& (isVolumeControl == FALSE || pIndicators->volume1 > pIndicators->volume2)
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume < 0)
				&& (isEnableCMFVolumeGap == FALSE || pIndicators->CMFVolumeGap > 0)
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline
				&& (weekly_baseline_short < weekly_baseline || pre3KTrend == DOWN))
				)
				&& iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice <= 0.2 * pBase_Indicators->dailyATR
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);
				if (timeInfo1.tm_mday != timeInfo2.tm_mday)
				{

					pIndicators->entrySignal = -1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 < pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap < 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume < 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap < 0 && pIndicators->cmfVolume <  0 && pIndicators->volume1 < pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					//if (pParams->orderInfo[orderIndex].type == SELL 
					//	&&  pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
					//{					
					//	pIndicators->tradeMode = 2;
					//}

					// If price retreats back, add position
					if (preHist1 < (histLevel*-1) && preHist2 < (histLevel*-1) && preHist3 < (histLevel*-1) && preHist4 < (histLevel*-1) && preHist5 < (histLevel*-1)
						&& fast1 < (level*-1) && fast2 < (level*-1) && fast3 < (level*-1) && fast4 < (level*-1) && fast5 < (level*-1)
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						)
					{
						pIndicators->entrySignal = 0;
					}

					if (isEnableMaxLevel == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& pIndicators->fast < (maxLevel*-1)
						)
					{
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
					{
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}
			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = -1;
			//}

			pIndicators->exitSignal = EXIT_BUY;

		}


		// Exit signal from daily chart: enter range
		if (isMACDZeroExit == FALSE)
		{
			if (pIndicators->fast - pIndicators->slow > macdLimit && pIndicators->preFast <= pIndicators->preSlow) // Exit SELL		
				pIndicators->exitSignal = EXIT_SELL;

			if (pIndicators->slow - pIndicators->fast > macdLimit && pIndicators->preFast >= pIndicators->preSlow) // Exit SELL		
				pIndicators->exitSignal = EXIT_BUY;
		}
		else{
			if (pIndicators->fast > 0) // Exit SELL		
				pIndicators->exitSignal = EXIT_SELL;

			if (pIndicators->fast < 0) // Exit SELL		
				pIndicators->exitSignal = EXIT_BUY;
		}
		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == BUY && preDailyClose < ma20Daily)
		//	pIndicators->exitSignal = EXIT_BUY;

		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == SELL && preDailyClose > ma20Daily)
		//	pIndicators->exitSignal = EXIT_SELL;
		//if (dailyTrend == 0)		
		//	pIndicators->exitSignal = EXIT_ALL;
	}


	//profit managgement
	// if profit is > 1 ATR, exit 50%
	// if proift is > 2 ATR, exit 100%

	//if MACD fast > 10 or < -10, exit proit
	//if (fast > maxLevel)
	//	pIndicators->exitSignal = EXIT_BUY;

	//if (fast < maxLevel * -1)
	//	pIndicators->exitSignal = EXIT_SELL;


	return SUCCESS;
}

/*
Run on daily chart.
1. Entry: MACD continue trade 
when both slow and fast are above 0 and the fast cross up slow, buy signal
when both slow and fast are under 0 and the fast cross down slow, sell signal

2. ͼƶֹ 
dailyTrend = 0,  MACD෴ź

3. No TP

4. Risk = 2% 
*/
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{	
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shiftPreDayBar = shift1Index_Daily - 2;
	int   dailyTrend; 
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	//double fast, slow;
	//double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	//double ma50Daily, preDailyClose;
	double level = 0, histLevel = 0, maxLevel = 0;
	double nextMACDRange = 0;
	double volume_ma_5;

	double ma20Daily, preDailyClose;
	/* Restored historical MACD component buffers (previous removal caused C2065 errors).
	 * Keep for computations later in function; if some become truly unused, consider pruning with care. */
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;

	int startHour = 1;

	//double cmfVolume = 0.0;
	BOOL isVolumeControl = TRUE;
	BOOL isEnableBeiLi = TRUE;
	BOOL isEnableSlow = TRUE;
	BOOL isEnableATR = TRUE;
	BOOL isEnableCMFVolume = FALSE;
	BOOL isEnableCMFVolumeGap = FALSE;
	BOOL isEnableMaxLevelBuy = FALSE;
	BOOL isEnableMaxLevelSell = FALSE;

	BOOL isVolumeControlRisk = FALSE;
	BOOL isCMFVolumeRisk = FALSE;
	BOOL isCMFVolumeGapRisk = FALSE;
	BOOL isAllVolumeRisk = FALSE;

	BOOL isWeeklyBaseLine = FALSE;

	BOOL isDailyOnly = TRUE;

	BOOL isEnableASI = FALSE;

	BOOL isEnableLate = FALSE;

	BOOL isEnableNextdayBar = FALSE;

	BOOL isEnableNoStopLoss = FALSE;

	BOOL isEnableMaxLevelRiskControl = FALSE;

	BOOL isMACDBeili = FALSE;

	BOOL isEnableMaxLevel = FALSE;
	
	//int oldestOpenOrderIndex = -1;

	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int pre3KTrend;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	//double CMFVolumeGap = 0.0;
	double stopLoss = pBase_Indicators->pDailyMaxATR;
	double maxRisk = 2;

	BOOL isEnableEntryEOD = FALSE;
	int startShift = 1;
	double macdLimit = 0.0;
	double asiBull, asiBear;
	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;
	double atrRange;

	//double rangeHigh, rangeLow;
	int range = 10000;

	double highHourlyClosePrice;
	double lowHourlyClosePrice;

	atrRange = 0.01 * 0.55;

	//double atr5Limit = pParams->bidAsk.ask[0] * 0.01 *0.55;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// Determine trend direction from daily chart
	
	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	ma20Daily = iMA(3, B_DAILY_RATES, 20, startShift);
	preDailyClose = iClose(B_DAILY_RATES, startShift);

	dailyBaseLine = ma20Daily;

	//isVolumeControl from AUTOBBS_RANGE	

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//if (pParams->bidAsk.ask[0] < 10000)
		//	level = 0.005 * pParams->bidAsk.ask[0];		
		//else if (pParams->bidAsk.ask[0] < 20000)
		//	level = max(50, 0.0035 * pParams->bidAsk.ask[0]);
		//else if (pParams->bidAsk.ask[0] < 30000)
		//	level = max(75, 0.003 * pParams->bidAsk.ask[0]);
		//else
		//	level = max(90, 0.0025 * pParams->bidAsk.ask[0]);	

		level = 0.005 * pParams->bidAsk.ask[0];

		maxLevel = 0.05 * pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = TRUE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = FALSE;
		isEnableMaxLevelSell = FALSE;
		isWeeklyBaseLine = FALSE;
		
		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		if (timeInfo1.tm_year > (2021 - 1900) ||
			(timeInfo1.tm_year == (2021 - 1900) && timeInfo1.tm_mon > 0) ||
			(timeInfo1.tm_year == (2021 - 1900) && timeInfo1.tm_mon == 0 && timeInfo1.tm_mday >= 18))
		{
			fastMAPeriod = 7;
			slowMAPeriod = 14;
			signalMAPeriod = 7;
		}


		stopLoss = max(stopLoss, pBase_Indicators->dailyATR * 1.8);
		//stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		isEnableLate = FALSE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		range = 5;
		
		isEnableMaxLevelRiskControl = FALSE;
		
	}
	else if (strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL)
	{
		level = 0.35;// min(0.35, 0.0053 * pParams->bidAsk.ask[0]);
		maxLevel = 0.01 * pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = FALSE;
		isEnableMaxLevel = TRUE;

		isWeeklyBaseLine = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = TRUE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = FALSE;

		isEnableEntryEOD = TRUE;

		pIndicators->minLotSize = 0.01;
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		pIndicators->volumeStep = 0.01;


		range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
	{		
		level = 0.35;// min(0.35, 0.0053 * pParams->bidAsk.ask[0]);
		maxLevel = 0.01* pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = FALSE;
		isEnableMaxLevel = TRUE;

		isWeeklyBaseLine = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		isDailyOnly = TRUE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = FALSE;

		isEnableEntryEOD = TRUE;

		pIndicators->minLotSize = 0.5;
		pIndicators->volumeStep = 0.5;

		//pIndicators->minLotSize = 0.01;
		//pIndicators->volumeStep = 0.01;

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
		

		range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = max(10, ((pParams->bidAsk.ask[0] - 1500) / 300) + 10);


		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		isCMFVolumeGapRisk = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		maxRisk = 2;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;

		isEnableLate = FALSE;		

		//isEnableASI = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		range = 10;

		startHour = pIndicators->startHour;
		
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{

		level = 0.05;
		maxLevel = max(0.2, (0.4 * (pParams->bidAsk.ask[0] - 15) / 5.0 ) + 0.2);

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		atrRange = 0.01; //1%

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		//isEnableMaxLevel = FALSE;

		isCMFVolumeGapRisk = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;
		
		maxRisk = 1.5;

		isDailyOnly = FALSE;

		shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;

		isEnableLate = FALSE;

		//isEnableASI = TRUE;

		//nextMACDRange = 0.2;

		isEnableNextdayBar = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		range = 10;

	}
	else if (strstr(pParams->tradeSymbol, "XAUEUR") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 8;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = TRUE;
		//isVolumeControl = (int)parameter(AUTOBBS_RANGE);
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = TRUE;
		isEnableCMFVolume = TRUE;
		isEnableCMFVolumeGap = FALSE;
		
		//isEnableMaxLevel = FALSE;
		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = FALSE;
		isEnableMaxLevel = FALSE;
		
		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 2;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = TRUE;

		isEnableLate = FALSE;

		nextMACDRange = 0.2;

		isEnableNextdayBar = TRUE;
				
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		range = 10;
				
	}
	else if (strstr(pParams->tradeSymbol, "GBPCHF") != NULL)
	{
		level = 0.002; //GBPJPY
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		//isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "AUDJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.5;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = TRUE;
		isEnableCMFVolumeGap = FALSE;

		//isEnableMaxLevel = FALSE;

		isDailyOnly = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "EURJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "CADJPY") != NULL)
	{
		level = 0.1; //GBPJPY
		maxLevel = 0.9;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = TRUE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = TRUE;

		//isEnableMaxLevel = FALSE;


		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		level = 0.001; //GBPAUD
		maxLevel =0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		isAllVolumeRisk = FALSE;

		//stopLoss = pBase_Indicators->dailyATR * 1.5;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		//isEnableNextdayBar = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		//isEnableNoStopLoss = TRUE;

		range = 10;
	}
	else if (strstr(pParams->tradeSymbol, "GBPUSD") != NULL)
	{
		level = 0.001; //GBPUSD
		maxLevel = 0.007;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevelBuy = TRUE;
		isEnableMaxLevelSell = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		maxRisk = 1.5;

		isDailyOnly = FALSE;
		pIndicators->stopMovingBackSL = TRUE;

		isEnableEntryEOD = FALSE;

		isEnableNextdayBar = TRUE;

		//pIndicators->riskCap = 1.9;

		//dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		range = 10;

	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		level = 0.0025; //GBPJPY
		maxLevel = 0;		
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;		

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 12;
		slowMAPeriod = 26;
		signalMAPeriod = 9;
	}
	else
	{
		level = 0; //EURUSD
		maxLevel = 0.005;

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;

		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isDailyOnly = TRUE;
	}

	// After 23:00, check if need to enter
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;		
		macdLimit = level / 2;

		//if (timeInfo1.tm_wday == 5)
		//{
		//	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
		//		macdLimit = 0;
		//}

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL || strstr(pParams->tradeSymbol, "SpotCrudeUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= startHour
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 start, avoid the first hour
	{		

		preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
		iTrend3Rules_preDays(pParams, pIndicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, startShift);

		pIndicators->CMFVolumeGap = getCMFVolumeGap(B_DAILY_RATES, 1, fastMAPeriod, startShift);

		//iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily - 1, range-1, &rangeHigh, &rangeLow);

		//Volume indicator....
		// preVolume > MA: current volume > past average volume
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		volume_ma_5 = iMA(4, B_DAILY_RATES, 5, startShift);

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf,volume_ma_5=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->cmfVolume, pIndicators->CMFVolumeGap, weekly_baseline, weekly_baseline_short, pIndicators->volume1, pIndicators->volume2, volume_ma_5);

		//Load MACD
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);
				
		pIndicators->fast = fast1;
		pIndicators->slow = slow1;
		pIndicators->preFast = fast2;
		pIndicators->preSlow = slow2;
		

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
		//oldestOpenOrderIndex = orderIndex;
		//oldestOpenOrderIndex = getOldestOpenOrderIndexEasy(B_PRIMARY_RATES);

		pIndicators->stopLoss = stopLoss;

		//Find the highest close price after order is opened
		getHighestHourlyClosePrice(pParams, pIndicators, pBase_Indicators, B_HOURLY_RATES, orderIndex, &highHourlyClosePrice, &lowHourlyClosePrice);

		if (isEnableMaxLevelRiskControl == TRUE && pIndicators->fast > maxLevel)
			pIndicators->risk = 0.5;

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift + 1), pIndicators->preFast, pIndicators->preSlow);

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, highHourlyClosePrice =%lf, lowHourlyClosePrice=%lf, stopLoss=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, highHourlyClosePrice, lowHourlyClosePrice, stopLoss);

		if (pIndicators->fast > 0						
			&& preDailyClose > dailyBaseLine
			&& pIndicators->fast - pIndicators->slow > macdLimit
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			
			pIndicators->stopLossPrice = max(highHourlyClosePrice,pIndicators->entryPrice) - stopLoss;

			if (isEnableNoStopLoss == TRUE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{
				//Not moving stop loss
				pIndicators->executionTrend = 0;				
				{


					if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 2 * pIndicators->stopLoss
						)
					{
						pIndicators->executionTrend = 1;
						pIndicators->entryPrice = pParams->bidAsk.ask[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					}
					else if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice >= 2 * pIndicators->stopLoss && pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice < 3 * pIndicators->stopLoss)
					{
						pIndicators->executionTrend = 1;
						pIndicators->entryPrice = pParams->bidAsk.ask[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice + pIndicators->stopLoss;
					}
				}
								
			}
			
			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			
			if (//fast > slow && preFast <= preSlow &&				
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//oldestOpenOrderIndex < 0 
				&& pIndicators->fast > pIndicators->preFast		
				)
			{
				
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if ( timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = 1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2 )
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap > 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume > 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap > 0 && pIndicators->cmfVolume >  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;					

					if (isEnableSlow == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->slow <= 0)
					{
						sprintf(pIndicators->status, "slow %lf is not greater than level 0.",
							pIndicators->slow);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (pIndicators->entrySignal != 0 &&
						pIndicators->fast <= level)
					{
						sprintf(pIndicators->status, "fast %lf is not greater than level %lf.",
							pIndicators->fast,level);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					// If price retreats back, add position
					if (isEnableLate == TRUE && preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
						&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						)
					{
						strcpy(pIndicators->status, "it is late for 5 days\n");

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString,pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolumeGap == TRUE && 
						pIndicators->entrySignal != 0 &&
						pIndicators->CMFVolumeGap <= 0)
					{
						sprintf(pIndicators->status,"CMFVolumeGap %lf is not greater than 0",
							pIndicators->CMFVolumeGap);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolume == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->cmfVolume <= 0)
					{
						sprintf(pIndicators->status,"cmfVolume %lf is not greater than 0",
							pIndicators->cmfVolume);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isVolumeControl == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->volume1 <= pIndicators->volume2)
					{
						sprintf(pIndicators->status,"volume1 %lf is not greater than volume2 %lf",
							pIndicators->volume1, pIndicators->volume2);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						preWeeklyClose <= weekly_baseline
						)
					{
						sprintf(pIndicators->status, "preWeeklyClose %lf is not greater than weekly baseline %lf.",
							preWeeklyClose, weekly_baseline);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						weekly_baseline_short <= weekly_baseline && pre3KTrend != UP
						)
					{
						sprintf(pIndicators->status, "Weekly_baseline_short %lf is less than weekly_baseline %lf and pre3KTrend %lf is not UP.",
							weekly_baseline_short, weekly_baseline, pre3KTrend);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					
					isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

					if (isEnableMaxLevelBuy == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& pIndicators->fast > maxLevel //&& truningPointIndex - 1 <= range
						&& (isEnableMaxLevel == FALSE || minPoint >= level)
						)
					{	

						sprintf(pIndicators->status, "MACD %lf exceeds max level %lf",
							pIndicators->fast, maxLevel);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0 
						&& isMACDBeili == TRUE
						&& (minPoint >= level || truningPointIndex - 1 <= range)
						//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						//&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint) //&& iClose(B_DAILY_RATES,1) < rangeHigh						
						)
					{
						strcpy(pIndicators->status, "MACD BeiLi");								

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
						
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}

				
			}			
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (pIndicators->fast < 0			
			&& preDailyClose < dailyBaseLine
			&& pIndicators->slow - pIndicators->fast > macdLimit
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];	
						
			pIndicators->stopLossPrice = min(lowHourlyClosePrice, pIndicators->entryPrice) + stopLoss;

			if (isEnableNoStopLoss == TRUE && orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
			{
				//Not moving stop loss
				pIndicators->executionTrend = 0;
				
				{
					if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 2 * pIndicators->stopLoss
						)
					{
						pIndicators->executionTrend = -1;
						pIndicators->entryPrice = pParams->bidAsk.bid[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
					}
					else if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] >= 2 * pIndicators->stopLoss && pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] < 3 * pIndicators->stopLoss)
					{
						pIndicators->executionTrend = -1;
						pIndicators->entryPrice = pParams->bidAsk.bid[0];
						pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice - pIndicators->stopLoss;
					}
				}
			}

			fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//oldestOpenOrderIndex < 0 
				&& pIndicators->fast < pIndicators->preFast			
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{

					pIndicators->entrySignal = -1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap < 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume < 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap < 0 && pIndicators->cmfVolume <  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					if (isEnableSlow == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->slow >= 0)
					{
						sprintf(pIndicators->status, "slow %lf is not less than level 0.",
							pIndicators->slow);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (pIndicators->entrySignal != 0 &&
						pIndicators->fast >= -1 * level)
					{
						sprintf(pIndicators->status, "fast %lf is not less than level %lf.",
							pIndicators->fast, -1* level);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					// If price retreats back, add position
					if (isEnableLate == TRUE && preHist1 < (histLevel*-1) && preHist2 < (histLevel*-1) && preHist3 < (histLevel*-1) && preHist4 < (histLevel*-1) && preHist5 < (histLevel*-1)
						&& fast1 < (level*-1) && fast2 < (level*-1) && fast3 < (level*-1) && fast4 < (level*-1) && fast5 < (level*-1)
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						)
					{
						strcpy(pIndicators->status, "it is late for 5 days\n");

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolumeGap == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->CMFVolumeGap > 0)
					{
						sprintf(pIndicators->status,"CMFVolumeGap %lf is not less than 0",
							pIndicators->CMFVolumeGap);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isEnableCMFVolume == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->cmfVolume > 0)
					{
						sprintf(pIndicators->status,"cmfVolume %lf is not less than 0",
							pIndicators->cmfVolume);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isVolumeControl == TRUE &&
						pIndicators->entrySignal != 0 &&
						pIndicators->volume1 <= pIndicators->volume2)
					{
						sprintf(pIndicators->status,"volume1 %lf is not greater than volume2 %lf",
							pIndicators->volume1, pIndicators->volume2);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						preWeeklyClose >= weekly_baseline
						)
					{
						sprintf(pIndicators->status, "preWeeklyClose %lf is not less than weekly baseline %lf.",
							preWeeklyClose, weekly_baseline);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					if (isWeeklyBaseLine == TRUE &&
						pIndicators->entrySignal != 0 &&
						weekly_baseline_short >= weekly_baseline && pre3KTrend != DOWN
						)
					{
						sprintf(pIndicators->status, "Weekly_baseline_short %lf is greater than weekly_baseline %lf and pre3KTrend %lf is not DOWN",
							weekly_baseline_short, weekly_baseline, pre3KTrend);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
					}

					isMACDBeili = iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint);

					if (isEnableMaxLevelSell == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& pIndicators->fast < (maxLevel*-1)
						&& (isEnableMaxLevel == FALSE || minPoint <= -1 * level)
						)
					{

						sprintf(pIndicators->status,"MACD %lf exceeds max level %lf",
							pIndicators->fast,maxLevel*-1);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& isMACDBeili == TRUE
						&& (minPoint <= -1 * level || truningPointIndex - 1 <= range)
						//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						//&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint) //&& iClose(B_DAILY_RATES, 1) > rangeLow						
						)
					{
						strcpy(pIndicators->status, "MACD BeiLi");								

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
						pIndicators->entrySignal = 0;
						
					}

					// First trade after trend change, if baseline exceeds 1 ATR(20), don't enter
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}
			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			// If there is profit and pullback, add position 

			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = -1;
			//}

			pIndicators->exitSignal = EXIT_BUY;

		}

		if (isEnableATR == TRUE &&
			pIndicators->entrySignal != 0 &&
			atr5 <= pIndicators->entryPrice * atrRange)
		{
			sprintf(pIndicators->status, "atr5 %lf is not greater than %lf.",
				atr5, pIndicators->entryPrice * atrRange);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (
			pIndicators->entrySignal != 0					
			&& isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar(startShift) == FALSE
			&& ( nextMACDRange == 0 || fabs(pIndicators->fast) < nextMACDRange)
			//orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//&& isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar2(pParams, orderIndex, startShift) == FALSE
			)
		{

			sprintf(pIndicators->status,"Nextday MACD Bar %lf is negative value %lf.",
				fabs(pIndicators->fast), nextMACDRange);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
			//pIndicators->exitSignal = EXIT_ALL;
		}

		if (pIndicators->entrySignal > 0  &&
			pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift) > 0.2 * pBase_Indicators->dailyATR)
		{
			sprintf(pIndicators->status, "Open price gap %lf is not less than %lf",
				pIndicators->entryPrice - iClose(B_DAILY_RATES, startShift), 0.2 * pBase_Indicators->dailyATR);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal < 0 &&
			iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice > 0.2 * pBase_Indicators->dailyATR)
		{
			sprintf(pIndicators->status, "Open price gap %lf is not less than %lf",
				iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice, 0.2 * pBase_Indicators->dailyATR);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal != 0 && (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL) && DAY_OF_WEEK(currentTime) == SUNDAY)
		{
			sprintf(pIndicators->status, "System InstanceID = %d, BarTime = %s, skip to entry a trade on Sunday.\n");

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			pIndicators->entrySignal = 0;
		}

		// Exit signal from daily chart: enter range

		if (pIndicators->fast - pIndicators->slow > macdLimit && pIndicators->preFast <= pIndicators->preSlow) // Exit SELL		
		pIndicators->exitSignal = EXIT_SELL;

		if (pIndicators->slow - pIndicators->fast > macdLimit && pIndicators->preFast >= pIndicators->preSlow) // Exit SELL		
		pIndicators->exitSignal = EXIT_BUY;

		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == BUY && preDailyClose < ma20Daily)
		//	pIndicators->exitSignal = EXIT_BUY;

		//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE && pParams->orderInfo[orderIndex].type == SELL && preDailyClose > ma20Daily)
		//	pIndicators->exitSignal = EXIT_SELL;
		//if (dailyTrend == 0)		
		//	pIndicators->exitSignal = EXIT_ALL;
	}
	

	//profit managgement
	// if profit is > 1 ATR, exit 50%
	// if proift is > 2 ATR, exit 100%

	//if MACD fast > 10 or < -10, exit proit
	//if (fast > maxLevel)
	//	pIndicators->exitSignal = EXIT_BUY;

	//if (fast < maxLevel * -1)
	//	pIndicators->exitSignal = EXIT_SELL;

	//If average = 2.3, max = 3

	//If the long term order move to break event, and try to keep adding position.	

	//if (oldestOpenOrderIndex >= 0 &&
	//	pIndicators->executionTrend != 0 &&
	//	(pIndicators->exitSignal != EXIT_BUY || pIndicators->exitSignal != EXIT_SELL || pIndicators->exitSignal != EXIT_ALL) &&
	//	(pIndicators->entrySignal != 1 || pIndicators->entrySignal != -1))
	//{
	//	//Modify MACD orders.
	//	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) + pIndicators->adjust;
	//	modifyAllOrdersOnSameDateEasy(oldestOpenOrderIndex, stopLoss, -1, pIndicators->stopMovingBackSL);

	//	//Remove modify signal
	//	pIndicators->executionTrend = 0;

	//	if ((pParams->orderInfo[oldestOpenOrderIndex].type == BUY && pParams->orderInfo[oldestOpenOrderIndex].stopLoss - pParams->orderInfo[oldestOpenOrderIndex].openPrice >= -2 * pIndicators->adjust) ||
	//		(pParams->orderInfo[oldestOpenOrderIndex].type == SELL &&  pParams->orderInfo[oldestOpenOrderIndex].openPrice - pParams->orderInfo[oldestOpenOrderIndex].stopLoss >= -2 * pIndicators->adjust))
	//	{
	//		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s,stopLoss =%lf. it is ok to add new positions in a long term trend now.",
	//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pParams->orderInfo[oldestOpenOrderIndex].stopLoss);

	//		pIndicators->tradeMode = 2;
	//		pIndicators->risk = 0.5;
	//		pIndicators->executionTrend = 0;

	//		addMoreOrdersOnLongTermTrend(pParams, pIndicators, pBase_Indicators, oldestOpenOrderIndex);
	//	}
	//	
	//}

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_MACD_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   weeklyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double fast, slow;
	double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_WEEKLY_RATES, 5, 1);
	/* int index1, index2, index3; */
	double level = 0;
	double volume1, volume2, volume_ma_3;

	currentTime = pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index_Weekly];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart

	if (pBase_Indicators->weeklyTrend_Phase > 0)
		weeklyTrend = 1;
	else if (pBase_Indicators->weeklyTrend_Phase < 0)
		weeklyTrend = -1;
	else
		weeklyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 25;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	//ma50Daily = iMA(3, B_DAILY_RATES, 50, 1);
	//preDailyClose = iClose(B_DAILY_RATES, 1);

	if (strstr(pParams->tradeSymbol, "XAU") != NULL)
		level = 10; // XAUUSD
	else if (strstr(pParams->tradeSymbol, "JPY") != NULL)
		level = 0; //GBPJPY
	else
		level = 0.0005; //EURUSD

	//if (timeInfo1.tm_hour == 1) // 1:00 start, avoid the first hour
	{
		//Volume indicator....
		// preVolume > MA: current volume > past average volume
		volume1 = iVolume(B_WEEKLY_RATES, 1);
		volume2 = iVolume(B_WEEKLY_RATES, 2);
		volume_ma_3 = iMA(4, B_WEEKLY_RATES, 3, 1);

		//Load MACD
		fast = iMACD(B_WEEKLY_RATES, 5, 10, 5, 0, 1);
		slow = iMACD(B_WEEKLY_RATES, 5, 10, 5, 1, 1);


		preFast = iMACD(B_WEEKLY_RATES, 5, 10, 5, 0, 2);
		preSlow = iMACD(B_WEEKLY_RATES, 5, 10, 5, 1, 2);


		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 1), fast, slow);
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 2), preFast, preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		//if (iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR)
		//{
		//	fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf",
		//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->pDailyPredictATR, iAtr(B_WEEKLY_RATES, 1, 0), pBase_Indicators->pWeeklyPredictMaxATR);
		//	return FALSE;
		//}

		if (fast > level
			//&& slow > 0 
			&& weeklyTrend > 0 
			&& fast > slow
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR;
			//pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR;


			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			if (//fast > slow && preFast <= preSlow &&				
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//&& atr5 > pIndicators->entryPrice * 0.01 * 0.55
				//&& getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < 4
				&& volume1 > volume2
				&& fast > preFast
				)
			{
				pIndicators->entrySignal = 1;

				//if (pParams->orderInfo[orderIndex].type == BUY &&
				//	pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
				//{
				//	pIndicators->tradeMode = 2;
				//}
			}
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (fast < (level * -1)
			//&& slow < 0 
			&& weeklyTrend < 0
			&& fast < slow
			//			&& fast < preFast			
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR;
			//pIndicators->stopLossPrice = pIndicators->entryPrice + 1.5 * pBase_Indicators->dailyATR;

			//pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//&& atr5 > pIndicators->entryPrice * 0.01 * 0.55
				//&& getSameSideTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < 4
				&& volume1 > volume2
				&& fast < preFast
				)
			{
				pIndicators->entrySignal = -1;

				//if (pParams->orderInfo[orderIndex].type == SELL 
				//	&&  pParams->orderInfo[orderIndex].profit > 0 && fabs(pParams->orderInfo[orderIndex].closePrice - pParams->orderInfo[orderIndex].openPrice) >= 1) //GBPJPY first, over 100 points
				//{					
				//	pIndicators->tradeMode = 2;
				//}
			}

			pIndicators->exitSignal = EXIT_BUY;

		}


		// Exit signal from daily chart: enter range

		if (fast > slow && preFast <= preSlow) // Exit SELL		
			pIndicators->exitSignal = EXIT_SELL;

		if (fast < slow && preFast >= preSlow) // Exit SELL		
			pIndicators->exitSignal = EXIT_BUY;

		//if (dailyTrend == 0)		
		//	pIndicators->exitSignal = EXIT_ALL;
	}

	return SUCCESS;
}



/*
Run on daily chart. Ichimoko base line only

1. Trend:
Weekly Ichimoko baseline ڷ

2. Entry:

For the first trading signal:
if Trend > 0 && daily close price > daily baseline, buy signal
if Trend < 0 && daily close price < daily baseline, sell signal

If ۸base line > 1.5 daily ATR, ȴһ졣

For the continuous trading signal on daily open:
Option 1:
if close higher: close(1) - close (2) > 25% daily ATR(20) -> buy signal
if close closer: close(1) - close (2) < 25% daily ATR(20) *-1  -> sell signal

Option 2:
AutoBBS trend phase:
if trend phase is up, buy signal
if trend phase is down, sell signal

Filter:
if price > weekly R2/S2, dont trade
if current weekly ATR > weekly max ATR, dont trade

Option 3: Only it is on a strong trend.
if price retreats back to 4H MA50 Plus 0.15%, buy or sell signal.

2. ͼƶֹ

If price move against to base line, exit.

SL: it will be max(baseline + 20% ATR(20), max weekly ATR

3. No TP

But it will be subject to profit managment, like start to close profit trade if the floading profit is more than 5 % on this strategy.

4. Risk = 0.5% each trade.
*/
AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preWeeklyClose;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	int openOrderCount = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	//Long term: tradeMode = 1
	//Short term: tradeMode = 0
	pIndicators->tradeMode = 1;

	if (pIndicators->tradeMode == 1)
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
		strategyVolRisk = pIndicators->strategyMaxRisk;
	}
	else
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
	}

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preDailyClose1 = iClose(B_DAILY_RATES, 2);
	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday)
			dailyOnly = 0;
	}

	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= 1) // 1:00 start, avoid the first hour
	{


		// Calculate daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
			preDailyClose > daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			
			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO: Need to adjust
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose > preDailyClose1
				&& preDailyClose > daily_baseline_short
				&& daily_baseline_short > daily_baseline
				&& preWeeklyClose > weekly_baseline
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
			preDailyClose < daily_baseline
			) // Sell
		{

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose < preDailyClose1
				&& preDailyClose < daily_baseline_short
				&& daily_baseline_short < daily_baseline
				&& preWeeklyClose < weekly_baseline
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = -1;
			}
			pIndicators->exitSignal = EXIT_BUY;

		}


	}

	openOrderCount = getOrderCountEasy();

	if (pIndicators->entrySignal != 0 && openOrderCount >= 3)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d ",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

		pIndicators->entrySignal = 0;
	}
	
	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	//targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	//// when floating profit is too high, fe 10%
	//if (pIndicators->riskPNL >targetPNL)
	//	closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shiftPreDayBar = shift1Index - 1;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preWeeklyClose, preWeeklyClose1;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double weekly_baseline = 0.0,weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;
		
	double dailyMA20 = 0.0;
	int openOrderCount = 0;

	double preHist1;
	double fast1;
	double slow1;
	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	int latestOrderIndex = 0;

	double preMonthHigh = 0.0, preMonthLow = 0.0;
	double rangeHigh = 0.0, rangeLow = 0.0;

	BOOL isEnableRange = TRUE;
	int range = 30;
	int orderCount = 3;
	BOOL isProfitManaged = TRUE;
	BOOL isWeeklyBaseLine = TRUE;
	BOOL isExitFromShortBaseLine = FALSE;
	BOOL isMACDBeiLi = TRUE;
	BOOL is3KBreak = FALSE;
	BOOL isMaxLevel = FALSE;
	BOOL isEnableCMFVolume = FALSE;
		
	double maxLevel = 0.0;
	int pre3KTrend;

	double exitBaseLine = 0.0;

	double level = 0.0;

	BOOL isDailyOnly = TRUE;
	BOOL isEnableEntryEOD = FALSE;
	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	//Long term: tradeMode = 1
	//Short term: tradeMode = 0
	pIndicators->tradeMode = 1;

	//latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (pIndicators->tradeMode == 1)
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
		strategyVolRisk = pIndicators->strategyMaxRisk;
	}
	else
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
	}

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preDailyClose1 = iClose(B_DAILY_RATES, 2);

	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday)
			dailyOnly = 0;
	}

	// If previous day close price not exceed 0.2 daily ATR, wait for next day	
	// Calculate daily and weekly baseline
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	pIndicators->daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	pIndicators->daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, pIndicators->daily_baseline);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, pIndicators->daily_baseline_short);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);


	if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableRange = TRUE;
		range = 30;
		isWeeklyBaseLine = FALSE;

		orderCount = 3;

		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;
		if (strcmp(pParams->accountInfo.brokerName, "International Capital Markets Pty. Ltd.") == 0)
			shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		level = 0.1;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = FALSE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;

		pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		shiftPreDayBar = shift1Index;

	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 1;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;

		pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		shiftPreDayBar = shift1Index;
	}
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//level = 0.0008;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;

		//pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		isDailyOnly = TRUE;

	}
	else if (strstr(pParams->tradeSymbol, "USDSGD") != NULL)
	{
		//level = 0.0008;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;
				
		pIndicators->daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);

		isDailyOnly = TRUE;
		
	}
	else if (strstr(pParams->tradeSymbol, "GER30EUR") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = TRUE;
		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = pIndicators->daily_baseline_short;

		is3KBreak = FALSE;
	}

	// After 23:00, check if need to enter
	//if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	//{		
	//
	//}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	pIndicators->cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, 1);

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, &fast1, &slow1, &preHist1);

	pIndicators->fast = fast1;
	pIndicators->slow = slow1;


	// If previous day close price not exceed 0.2 daily ATR, wait for next day
	if (timeInfo1.tm_hour >= 1 
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		)
	{

		dailyMA20 = iMA(3, B_DAILY_RATES, 20, 1);

		iTrend3Rules_preDays(pParams, pIndicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);


		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily - 1, range, &rangeHigh, &rangeLow);

		//load pBase_Indicators
		//pBase_Indicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
		//pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

		
		//get the last month high low 
		preMonthHigh = iHigh(B_MONTHLY_RATES, 1);
		preMonthLow = iLow(B_MONTHLY_RATES, 1);

		openOrderCount = getOrderCountEasy();

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
			preDailyClose > pIndicators->daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = pIndicators->daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO: Need to adjust

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose > preDailyClose1
				&& preDailyClose > pIndicators->daily_baseline_short
				&& pIndicators->daily_baseline_short - pIndicators->daily_baseline > 0 * pBase_Indicators->dailyATR
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose > weekly_baseline && (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
				)
				&& (isEnableRange == FALSE || preDailyClose > rangeHigh)
				&& (isMACDBeiLi == FALSE || (fast1 > slow1 && fast1 > level))
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume > 0)
				)
			{
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;


			if (isMACDBeiLi == TRUE
				&& pIndicators->entrySignal != 0
				&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
			{
				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD BeiLi",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				pIndicators->entrySignal = 0;
			}

			//Exit the last trade when break fast line
			if (isExitFromShortBaseLine == TRUE && preDailyClose < exitBaseLine)
			{
				pIndicators->exitSignal = EXIT_BUY;
			}

		}

		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
			preDailyClose < pIndicators->daily_baseline
			) // Sell
		{

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = pIndicators->daily_baseline + pBase_Indicators->dailyATR * 0.25;

			//Option 1:
			if (
				dailyOnly == 1
				&& preDailyClose < preDailyClose1
				&& preDailyClose < pIndicators->daily_baseline_short
				&& pIndicators->daily_baseline - pIndicators->daily_baseline_short  > 0 * pBase_Indicators->dailyATR
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline
				&& (weekly_baseline_short < weekly_baseline || pre3KTrend == DOWN))
				)
				&& (isEnableRange == FALSE || preDailyClose < rangeLow)
				&& (isMACDBeiLi == FALSE || (fast1 < -1 * level && fast1 < slow1))
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume < 0)
				)
			{
				pIndicators->entrySignal = -1;
			}
			pIndicators->exitSignal = EXIT_BUY;

			if (isMACDBeiLi == TRUE
				&& pIndicators->entrySignal != 0
				&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
			{
				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD BeiLi",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				pIndicators->entrySignal = 0;
			}


			if (isExitFromShortBaseLine == TRUE  && preDailyClose > exitBaseLine)
			{
				pIndicators->exitSignal = EXIT_SELL;
			}

		}


		if (pIndicators->entrySignal != 0 && openOrderCount >= orderCount)
		{
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d ",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

			pIndicators->entrySignal = 0;
		}

		if (isProfitManaged == TRUE)
		{
			if (pIndicators->entrySignal != 0 && openOrderCount == 0)
			{
				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d 0.5 risk",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

				pIndicators->risk = 0.5;
			}

			profitManagement_base(pParams, pIndicators, pBase_Indicators);


			targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
			//// when floating profit is too high, fe 10%
			if (pIndicators->riskPNL > targetPNL)
				closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
		}
	}
	return SUCCESS;
}

/*
Run on daily chart. Ichimoko base line only

1. Trend:
Weekly Ichimoko baseline ڷ

2. Entry:

For the first trading signal:
if Trend > 0 && daily close price > daily baseline, buy signal
if Trend < 0 && daily close price < daily baseline, sell signal

If ۸base line > 1.5 daily ATR, ȴһ졣

For the continuous trading signal on daily open:
Option 1:
if close higher: close(1) - close (2) > 25% daily ATR(20) -> buy signal
if close closer: close(1) - close (2) < 25% daily ATR(20) *-1  -> sell signal

Option 2:
AutoBBS trend phase:
if trend phase is up, buy signal
if trend phase is down, sell signal

Filter:
if price > weekly R2/S2, dont trade
if current weekly ATR > weekly max ATR, dont trade

Option 3: Only it is on a strong trend.
if price retreats back to 4H MA50 Plus 0.15%, buy or sell signal.

2. ͼƶֹ

If price move against to base line, exit.

SL: it will be max(baseline + 20% ATR(20), max weekly ATR

3. No TP

But it will be subject to profit managment, like start to close profit trade if the floading profit is more than 5 % on this strategy.

4. Risk = 0.5% each trade.
*/
AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preWeeklyClose, preWeeklyClose1;
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	double weeklyMA50 = 0.0;
	double dailyMA20 = 0.0;
	int openOrderCount = 0;

	double preHist1;
	double fast1;
	double slow1;
	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	int latestOrderIndex = 0;

	double preMonthHigh = 0.0, preMonthLow = 0.0;
	double rangeHigh = 0.0, rangeLow = 0.0;

	BOOL isEnableRange = TRUE;
	int range = 30;
	int orderCount = 3;
	BOOL isProfitManaged = TRUE;
	BOOL isWeeklyBaseLine = TRUE;
	BOOL isExitFromShortBaseLine = FALSE;
	BOOL isMACDBeiLi = TRUE;
	BOOL is3KBreak = FALSE;
	BOOL isMaxLevel = FALSE;
	BOOL isEnableCMFVolume = FALSE;

	double cmfVolume = 0.0;
	double maxLevel = 0.0;
	int pre3KTrend;

	double exitBaseLine = 0.0;

	double level = 0.0;

	int truningPointIndex = -1, minPointIndex = -1;
	double turningPoint, minPoint;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	//Long term: tradeMode = 1
	//Short term: tradeMode = 0
	pIndicators->tradeMode = 1;

	latestOrderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (pIndicators->tradeMode == 1)
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
		strategyVolRisk = pIndicators->strategyMaxRisk;
	}
	else
	{
		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
	}

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preDailyClose1 = iClose(B_DAILY_RATES, 2);
	//preDailyHigh1 = iHigh(B_DAILY_RATES, 2);
	//preDailyLow1 = iLow(B_DAILY_RATES, 2);
	//preDailyHigh2 = iHigh(B_DAILY_RATES, 3);
	//preDailyLow2 = iLow(B_DAILY_RATES, 3);

	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
	{
		safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].openTime);
		if (timeInfo1.tm_mday == timeInfo2.tm_mday)
			dailyOnly = 0;
	}

	// If previous day close price not exceed 0.2 daily ATR, wait for next day	
	// Calculate daily and weekly baseline
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);

	weeklyMA50 = iMA(3, B_WEEKLY_RATES, 50, 1);
	dailyMA20 = iMA(3, B_DAILY_RATES, 20, 1);

	if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableRange = TRUE;
		range = 30;
		isWeeklyBaseLine = FALSE;

		orderCount = 3;

		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		level = 0.1;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = FALSE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;

		daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);
		
	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;

		daily_baseline = iMA(3, B_DAILY_RATES, 50, 1);
	}
	else if (strstr(pParams->tradeSymbol, "USDSGD") != NULL)
	{
		//level = 0.0008;
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = FALSE;
		isExitFromShortBaseLine = FALSE;

		isEnableCMFVolume = TRUE;
		isMACDBeiLi = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;

		daily_baseline = iMA(3,B_DAILY_RATES, 50, 1);
	}
	else if (strstr(pParams->tradeSymbol, "GER30EUR") != NULL)
	{
		isEnableRange = FALSE;
		range = 30;

		orderCount = 3;
		isProfitManaged = TRUE;
		isWeeklyBaseLine = TRUE;
		isExitFromShortBaseLine = FALSE;

		isMACDBeiLi = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		exitBaseLine = daily_baseline_short;

		is3KBreak = FALSE;				
	}

	cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, 1);

	iTrend3Rules_preDays(pParams, pIndicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);
	

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily - 1, range, &rangeHigh, &rangeLow);

	//load pBase_Indicators
	pBase_Indicators->dailyATR = iAtr(B_DAILY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);
	pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

	//Load MACD
	iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, &fast1, &slow1, &preHist1);

	//get the last month high low 
	preMonthHigh = iHigh(B_MONTHLY_RATES, 1);
	preMonthLow = iLow(B_MONTHLY_RATES, 1);

	openOrderCount = getOrderCountEasy();

	if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
		preDailyClose > daily_baseline
		) // Buy
	{

		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO: Need to adjust
		//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);

		//Option 1:
		if (
			dailyOnly == 1
			&& preDailyClose > preDailyClose1
			&& preDailyClose > daily_baseline_short
			&& daily_baseline_short - daily_baseline > 0 * pBase_Indicators->dailyATR
			//&& preWeeklyClose > weeklyMA50
			&& (isWeeklyBaseLine == FALSE || 
				(preWeeklyClose > weekly_baseline	&&  (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
				)
			&& (isEnableRange == FALSE || preDailyClose > rangeHigh)
			&& (isMACDBeiLi == FALSE || (fast1 > slow1 && fast1 > level))
			&& (isEnableCMFVolume == FALSE || cmfVolume > 0)
			)
		{
			pIndicators->entrySignal = 1;
		}
		pIndicators->exitSignal = EXIT_SELL;


		if (isMACDBeiLi == TRUE
			&& pIndicators->entrySignal != 0
			//&& latestOrderIndex >= 0 && pParams->orderInfo[latestOrderIndex].type == BUY
			&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
		{
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD BeiLi",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			pIndicators->entrySignal = 0;
		}

		//Exit the last trade when break fast line
		if (isExitFromShortBaseLine == TRUE && preDailyClose < exitBaseLine )
		{
			//if (openOrderCount > 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
			//	closeLongEasy(pParams->orderInfo[latestOrderIndex].ticket);
			pIndicators->exitSignal = EXIT_BUY;
		}

		//if (is3KBreak == TRUE && pre3KTrend == DOWN)
		//{			
		//	pIndicators->exitSignal = EXIT_BUY;
		//}
	}

	if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
		preDailyClose < daily_baseline
		) // Sell
	{

		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];

		pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
		//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);

		//Option 1:
		if (
			dailyOnly == 1
			&& preDailyClose < preDailyClose1
			&& preDailyClose < daily_baseline_short
			&& daily_baseline - daily_baseline_short  > 0 * pBase_Indicators->dailyATR
			//&& preWeeklyClose < weeklyMA50
			&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline 				
				&& (weekly_baseline_short < weekly_baseline ||	pre3KTrend == DOWN))
				)
			&& (isEnableRange == FALSE || preDailyClose < rangeLow)
			&& (isMACDBeiLi == FALSE || (fast1 < -1 * level && fast1 < slow1))
			&& (isEnableCMFVolume == FALSE || cmfVolume < 0)
			)
		{
			pIndicators->entrySignal = -1;
		}
		pIndicators->exitSignal = EXIT_BUY;

		if (isMACDBeiLi == TRUE
			&& pIndicators->entrySignal != 0			
			&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex, &turningPoint, &minPointIndex, &minPoint))
		{
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, MACD BeiLi",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

			pIndicators->entrySignal = 0;
		}


		if (isExitFromShortBaseLine == TRUE  && preDailyClose > exitBaseLine)
		{
			//if (openOrderCount > 0 && pParams->orderInfo[latestOrderIndex].isOpen == TRUE)
			//	closeShortEasy(pParams->orderInfo[latestOrderIndex].ticket);
			pIndicators->exitSignal = EXIT_SELL;
		}

		//if (is3KBreak == TRUE && pre3KTrend == UP )
		//{
		//	pIndicators->exitSignal = EXIT_SELL;
		//}
	}

	
	if (pIndicators->entrySignal != 0 && openOrderCount >= orderCount)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d ",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

		pIndicators->entrySignal = 0;
	}

	if (isProfitManaged == TRUE)
	{
		if (pIndicators->entrySignal != 0 && openOrderCount == 0)
		{
			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d 0.5 risk",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

			pIndicators->risk = 0.5;
		}

		profitManagement_base(pParams, pIndicators, pBase_Indicators);


		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
		//// when floating profit is too high, fe 10%
		if (pIndicators->riskPNL > targetPNL)
			closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}
	return SUCCESS;
}


/*
ԶĽϵͳ
4Hshellington
ʹñصֹλ
ûмֵ
*/
/*
4HľΪ볡
ƶֹ
ûֹӯ
*/
AsirikuyReturnCode workoutExecutionTrend_4H_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, closeTimeInfo;
	char   timeString[MAX_TIME_STRING_SIZE] = "";

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;

	int orderIndex = -1;
	int execution_tf, close_index_rate = -1, diff4Hours, diffDays, diffWeeks;

	int level = 0;
	BOOL isEnableWeeklyATRControl = TRUE;	
	BOOL isEnableWeeklyTrend = FALSE;
	int startHour = 0;
	int buyWonTimes = 0, sellWonTimes = 0;

	double atr5 = iAtr(B_DAILY_RATES, 5, 1);

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;

	int sameSideWonTradesInCurrentTrend;

	double rangeHigh = 0.0, rangeLow = 0.0;

	BOOL isEnableRange = TRUE;
	int range = 30;
	double preRangeClose;

	int turingIndexMA = -1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Determine trend direction from daily chart
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	execution_tf = (int)pParams->settings[TIMEFRAME];

	pIndicators->takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	pIndicators->takePrice = min(pIndicators->takePrice, pBase_Indicators->dailyATR);
	
	pIndicators->riskCap = 0;

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL || strstr(pParams->tradeSymbol, "ETHUSD") != NULL)
	{
		//isEnableWeeklyATRControl = TRUE;
		//isEnableWeeklyTrend = TRUE;
		buyWonTimes = 5;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 5;

		pIndicators->riskCap = 2;

		isEnableRange = TRUE;
		range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XAUAUD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 3;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;

		startHour = 1;

		//pIndicators->riskCap = 2;

		//isEnableRange = TRUE;
		//range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 2;
		sellWonTimes = 2;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		buyWonTimes = 2;
		sellWonTimes = 2;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;

		//pIndicators->riskCap = 2;

		//isEnableRange = TRUE;
		//range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "AUDUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		//isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		//pIndicators->takePrice = 0;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;

		isEnableRange = TRUE;
		range = 60;
	}
	else if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 2;
		sellWonTimes = 1;

		//pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;

		//if (strcmp(pParams->accountInfo.brokerName, "Pepperstone Group Limited") == 0)
		pIndicators->minLotSize = 0.1;	

		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "NAS100USD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
		buyWonTimes = 3;
		sellWonTimes = 1;

		//pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;
		//if (strcmp(pParams->accountInfo.brokerName, "Pepperstone Group Limited") == 0)
		pIndicators->minLotSize = 0.1;

		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "USTECUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
		buyWonTimes = 3;
		sellWonTimes = 1;

		pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;
		//if (strcmp(pParams->accountInfo.brokerName, "Pepperstone Group Limited") == 0)
		//pIndicators->minLotSize = 0.1;

		startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 3;
		sellWonTimes = 1;

		isEnableRange = TRUE;
		range = 60;
		startHour = 1;
		//pIndicators->minLotSize = 0.01;
		//pIndicators->isEnableSellMinLotSize = TRUE;
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 2;
		sellWonTimes = 1;

		isEnableRange = TRUE;
		range = 60;
		startHour = 1;
		//pIndicators->minLotSize = 0.01;
		//pIndicators->isEnableSellMinLotSize = TRUE;
	}

	iSRLevels(pParams, pBase_Indicators, B_FOURHOURLY_RATES, shift1Index_4H - 1, range, &rangeHigh, &rangeLow);
	preRangeClose = iClose(B_FOURHOURLY_RATES, 1);

	
	//4H filter	
	if ((timeInfo1.tm_hour - startHour) % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 27;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		if (trend_MA > 0 )
			trend_4H = 1;
		else if (trend_MA < 0)
			trend_4H = -1;
		// Enter order on key support/resistance levels

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
		

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			safe_gmtime(&closeTimeInfo, pParams->orderInfo[orderIndex].closeTime);

			//skip weekend, consider cross a new year.
			diff4Hours = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 4);
			diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 24);
			diffWeeks = (timeInfo1.tm_wday + 1 + diffDays) / 7;

			close_index_rate = shift1Index_4H - (diff4Hours - diffWeeks * 2 *6);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s,diff4Hours=%d,diffDays=%d,diffWeeks=%d,orderIndex=%d,close_index_rate=%d,bbsIndex_excution=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, diff4Hours, diffDays, diffWeeks, orderIndex, close_index_rate, pIndicators->bbsIndex_4H);
		}

		
		if (trend_4H == 1 //&& pBase_Indicators->weekly3RulesTrend == UP
			)
		{
			if (pIndicators->bbsTrend_4H == 1)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];

				pIndicators->stopLossPrice = min(pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1) - pBase_Indicators->pDailyATR * 0.5);
				//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pDailyATR * 1.5);

				//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
				//{
				//	if (pParams->bidAsk.ask[0] - pParams->orderInfo[orderIndex].openPrice > pIndicators->stopLoss )
				//	{	
				//		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				//	}					
				//}

				//orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

				sameSideWonTradesInCurrentTrend = getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY);
				turingIndexMA = iTrendMA_LookBack(pParams, pBase_Indicators, B_FOURHOURLY_RATES, 1);

				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s,stopLossPrice=%lf,bbsStopPrice_4H=%lf,MA200 = %lf,MA50=%lf,turingIndexMA=%d,preRangeClose=%lf,rangeHigh=%lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->stopLossPrice, pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1), iMA(3, B_FOURHOURLY_RATES, 50, 1), turingIndexMA, preRangeClose, rangeHigh);

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					//&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& ((orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL) || pIndicators->bbsIndex_4H >= close_index_rate)
					&& (isEnableRange == FALSE || preRangeClose > rangeHigh 
					|| turingIndexMA >= range
					)
					//&& getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < buyWonTimes
					//&& (isEnableWeeklyTrend == FALSE || pBase_Indicators->weeklyTrend_Phase != RANGE)
					)
				{

					pIndicators->entrySignal = 1;

					if (pIndicators->entrySignal != 0 &&
						sameSideWonTradesInCurrentTrend >= buyWonTimes
						)
					{
						sprintf(pIndicators->status, "sameSideWonTradesInCurrentTrend %d is greater than buyWonTimes %d",
							sameSideWonTradesInCurrentTrend, buyWonTimes);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

				}				
				
			}
			pIndicators->exitSignal = EXIT_SELL;
			//else
			//	pIndicators->exitSignal = EXIT_BUY;

		}

		if (trend_4H == -1 //&& pBase_Indicators->weekly3RulesTrend == DOWN
			)
		{
			
			if (pIndicators->bbsTrend_4H == -1)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				if (pIndicators->isEnableSellMinLotSize == TRUE)
					pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->pDailyATR *0.1;
				else
					pIndicators->stopLossPrice = max(pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1) + pBase_Indicators->pDailyATR * 0.5);
				//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pDailyATR * 1.5);

				//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
				//{

				//	if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss )
				//	{
				//		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				//	}

				//}

				sameSideWonTradesInCurrentTrend = getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL);
				turingIndexMA = iTrendMA_LookBack(pParams, pBase_Indicators, B_FOURHOURLY_RATES, -1);

				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s,stopLossPrice=%lf,bbsStopPrice_4H=%lf,MA200 = %lf,MA50=%lf,turingIndexMA=%d,preRangeClose=%lf,rangeLow=%lf",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->stopLossPrice, pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1), iMA(3, B_FOURHOURLY_RATES, 50, 1), turingIndexMA, preRangeClose, rangeLow );

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					//&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& ((orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY) || pIndicators->bbsIndex_4H >= close_index_rate)	
					&& (isEnableRange == FALSE || preRangeClose < rangeLow 
					|| turingIndexMA >= range
					)
					//&& getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < sellWonTimes
					//&& (isEnableWeeklyTrend == FALSE || pBase_Indicators->weeklyTrend_Phase != RANGE)
					)
				{
					pIndicators->entrySignal = -1;	
										

					if (pIndicators->entrySignal != 0 &&
						sameSideWonTradesInCurrentTrend >= sellWonTimes
						)
					{
						sprintf(pIndicators->status, "sameSideWonTradesInCurrentTrend %d is greater than sellWonTimes %d",
							sameSideWonTradesInCurrentTrend, sellWonTimes);

						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

						pIndicators->entrySignal = 0;
					}

				}
				
			}
			pIndicators->exitSignal = EXIT_BUY;

		}
		//else
		//	pIndicators->exitSignal = EXIT_SELL;

		if (pIndicators->entrySignal != 0 &&
			isEnableWeeklyATRControl == TRUE &&
			fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) > pBase_Indicators->pWeeklyPredictATR
			)
		{
			sprintf(pIndicators->status, "current week movement %lf is greater than pWeeklyPredictATR %lf",
				fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice), pBase_Indicators->pWeeklyPredictATR);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}

		if (pIndicators->entrySignal != 0 &&
			isEnableWeeklyTrend == TRUE &&
			pBase_Indicators->weeklyTrend_Phase == RANGE
			)
		{
			sprintf(pIndicators->status, "pBase_Indicators->weeklyTrend_Phase %lf is in Range",
				pBase_Indicators->weeklyTrend_Phase);

			fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			pIndicators->entrySignal = 0;
		}
	}
	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Weekly_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preWeeklyClose, preWeeklyClose1, preWeeklyClose2, preDailyClose;
	double shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
	double weekly_baseline = 0.0, weekly_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	double freeMargin = 0.0;

	int openOrderCount = 0;
	int openOrderCountInCurrentWeek = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 33;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->volumeStep = 0.1;

	targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK);
	strategyVolRisk = pIndicators->strategyMaxRisk;

	preDailyClose = iClose(B_DAILY_RATES, 1);
	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);
	preWeeklyClose2 = iClose(B_WEEKLY_RATES, 3);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);


	if (timeInfo1.tm_hour >= 1) // 1:00 start, avoid the first hour
	{
		// Calculate daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, weeklyHigh =%lf, weeklyLow=%lf, weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, weeklyHigh, weeklyLow, weekly_baseline);

		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, shortWeeklyHigh =%lf, shortWeeklyLow=%lf, weekly_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyHigh, shortWeeklyLow, weekly_baseline_short);

		openOrderCount = getOrderCountEasy();
		openOrderCountInCurrentWeek = getOrderCountForCurrentWeekEasy(currentTime);

		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice / 2;

		//pIndicators->stopLossPrice = weekly_baseline - pBase_Indicators->weeklyATR * 0.25;
		//pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);
				
		pBase_Indicators->weeklyATR = iAtr(B_WEEKLY_RATES, 20, 1);

		if (//preWeeklyClose > weekly_baseline						
			//&& preWeeklyClose > weekly_baseline_short
			//&& weekly_baseline_short > weekly_baseline		
			//&& 
			(	(preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1 && weekly_baseline - pIndicators->entryPrice > 0) ||
				(pIndicators->entryPrice > weekly_baseline && pIndicators->entryPrice < weekly_baseline_short)
			)
			//preWeeklyClose1 < preWeeklyClose2 && preWeeklyClose > preWeeklyClose1
			&& !isSamePricePendingOrderEasy(pIndicators->entryPrice, 0.25 * pBase_Indicators->weeklyATR)
			)
		{
			pIndicators->entrySignal = 1;
			if (weekly_baseline - pIndicators->entryPrice > 2 * pBase_Indicators->weeklyATR)
			{
				pIndicators->risk = 4;
			}
			else if (weekly_baseline - pIndicators->entryPrice > pBase_Indicators->weeklyATR)
			{
				pIndicators->risk = 3;
			}
			else if (weekly_baseline - pIndicators->entryPrice > 0)
			{
				pIndicators->risk = 2;
			}
			else
				pIndicators->risk = 1;
		}

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pIndicators->entrySignal != 0 && openOrderCountInCurrentWeek > 0 )
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d openOrderCountInCurrentWeek=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount, openOrderCountInCurrentWeek);

		pIndicators->entrySignal = 0;
	}

	if (pIndicators->riskPNL < -5)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s pIndicators->riskPNL=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL);
	}

	freeMargin = caculateFreeMarginEasy();
	
	if (pIndicators->entrySignal != 0 && freeMargin / pIndicators->entryPrice < 1)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s freeMargin=%lf, Times=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, freeMargin, freeMargin / pIndicators->entryPrice);
		pIndicators->entrySignal = 0;
	}

	if (pIndicators->entrySignal != 0 && pIndicators->riskPNL < -20)
	{
		pIndicators->entrySignal = 0;
	}
	
	// when floating profit is too high, fe 10%
	if (pIndicators->riskPNL > targetPNL)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s closeWinningPositionsEasy pIndicators->riskPNL=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL);

		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	}


	return SUCCESS;
}
//
//AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Weekly_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
//{
//	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
//	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;	
//	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
//	int   dailyTrend;
//	time_t currentTime;
//	struct tm timeInfo1, timeInfo2;
//	char   timeString[MAX_TIME_STRING_SIZE] = "";
//	double atr5 = iAtr(B_DAILY_RATES, 5, 1);	
//	double preWeeklyClose, preWeeklyClose1, preWeeklyClose2, preDailyClose;
//	double shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
//	double weekly_baseline = 0.0, weekly_baseline_short = 0.0;
//	int orderIndex;
//	int dailyOnly = 1;
//
//	double targetPNL = 0;
//	double strategyMarketVolRisk = 0.0;
//	double strategyVolRisk = 0.0;
//
//	int openOrderCount = 0;
//	int openOrderCountInCurrentWeek = 0;
//
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
//	safe_gmtime(&timeInfo1, currentTime);
//	safe_timeString(timeString, currentTime);
//
//	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
//
//	pIndicators->splitTradeMode = 26;
//	pIndicators->tpMode = 3;
//
//	pIndicators->tradeMode = 1;
//
//	targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
//	strategyVolRisk = pIndicators->strategyMaxRisk;
//
//	preDailyClose = iClose(B_DAILY_RATES, 1);
//	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
//	preWeeklyClose1 = iClose(B_WEEKLY_RATES, 2);	
//	preWeeklyClose2 = iClose(B_WEEKLY_RATES, 3);
//
//	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
//
//
//	if (timeInfo1.tm_hour >= 1) // 1:00 start, avoid the first hour
//	{
// Calculate daily and weekly baseline
//		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
//		weekly_baseline = (weeklyHigh + weeklyLow) / 2;
//
//		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
//		weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;
//
//		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, weeklyHigh =%lf, weeklyLow=%lf, weekly_baseline=%lf",
//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, weeklyHigh, weeklyLow, weekly_baseline);
//
//		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, shortWeeklyHigh =%lf, shortWeeklyLow=%lf, weekly_baseline_short=%lf",
//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortWeeklyHigh, shortWeeklyLow, weekly_baseline_short);
//
//		openOrderCount = getOrderCountEasy();
//		openOrderCountInCurrentWeek = getOrderCountForCurrentWeekEasy(currentTime);
//
//		if (preWeeklyClose > weekly_baseline) // Buy
//		{
//
//			pIndicators->executionTrend = 1;
//			pIndicators->entryPrice = pParams->bidAsk.ask[0];
//
//			pIndicators->stopLossPrice = weekly_baseline - pBase_Indicators->weeklyATR * 0.25; 
//			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);
//
//			if (
//				dailyOnly == 1				
//				&& preWeeklyClose > preWeeklyClose1 
//				//&& preWeeklyClose1 < preWeeklyClose2
//				&& preWeeklyClose > weekly_baseline_short
//				&& weekly_baseline_short > weekly_baseline
//				&& preWeeklyClose > weekly_baseline				
//				)
//			{
//				pIndicators->entrySignal = 1;
//			}
//
//			pIndicators->exitSignal = EXIT_SELL;
//
//		}
//
//		if (preWeeklyClose < weekly_baseline) // Sell
//		{
//
//			pIndicators->executionTrend = -1;
//			pIndicators->entryPrice = pParams->bidAsk.bid[0];
//
//			pIndicators->stopLossPrice = weekly_baseline + pBase_Indicators->weeklyATR * 0.25;
//			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);
//
//			//Option 1:
//			if (
//				dailyOnly == 1
//				&& preWeeklyClose < preWeeklyClose1
//				//&& preWeeklyClose1 > preWeeklyClose2
//				&& preWeeklyClose1 < weekly_baseline_short
//				&& weekly_baseline_short < weekly_baseline
//				&& preWeeklyClose < weekly_baseline				
//				)
//			{
//				pIndicators->entrySignal = -1;
//			}
//
//			pIndicators->exitSignal = EXIT_BUY;
//
//		}
//
//
//	}
//
//	if (pIndicators->entrySignal != 0 && 
//		(openOrderCount >= 3 || openOrderCountInCurrentWeek > 0
//		)
//		)
//	{
//		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s openOrderCount=%d openOrderCountInCurrentWeek=%d",
//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount, openOrderCountInCurrentWeek);
//
//		pIndicators->entrySignal = 0;
//	}
//
//	profitManagement_base(pParams, pIndicators, pBase_Indicators);
//
//	return SUCCESS;
//}