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

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

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


	//至少有1/3 空间
	
	if (pIndicators->tradeMode == 1)
	{
		if (gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{
				takePrice = gap / 3;
				
				lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap to 2 % risk
				//這裏使用估計止損50點，然後最大風險是2%
				//Cap the max risk
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, BUY, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * 7);
				lots = min(lots_max, lots);

				if (takePrice >= 1.5) //XAUUSD		
					openSingleLongEasy(takePrice, stopLoss, lots, 0);
				

				if (pIndicators->entrySignal == 1 && pIndicators->bbsIndex_excution != shift0Index_Primary - 1)
				{
					if (pIndicators->subTradeMode == 0)
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
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

	//至少有1/3 空间
	if (pIndicators->tradeMode == 1)
	{
		if (gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
		{
			orderCountToday = getOrderCountTodayEasy(currentTime);
			if (orderCountToday == 0)
			{

				takePrice = gap / 3;
				lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

				// Cap to 2 % risk
				//這裏使用估計止損50點，然後最大風險是2%
				//Cap the max risk
				lots_max = calculateOrderSizeWithSpecificRisk(pParams, SELL, pIndicators->entryPrice, stopLoss, pParams->settings[ACCOUNT_RISK_PERCENT] * 7);
				lots = min(lots_max, lots);

				if (takePrice >= 1.5) //XAUUSD
					openSingleShortEasy(takePrice, stopLoss, lots, 0);

				if (pIndicators->entrySignal == 1 && pIndicators->bbsIndex_excution != shift0Index_Primary - 1)
				{
					if (pIndicators->subTradeMode == 0)
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS break out.",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
					else if (pIndicators->subTradeMode == 2)
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entry long trade on retreated signal in BBS Retreat.",
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
	// 1：1 			
	
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
	// 1：1 			
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
void splitSellOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1：1 			
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
	// 1：1 			
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
	// 1：1 			
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = fabs(pBase_Indicators->weeklyR2 - pIndicators->adjust - pIndicators->entryPrice);
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
void splitSellOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1：1 or S2?
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->weeklyS2 + pIndicators->adjust));
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/*
第一单： 日内出场
第二单： 周内出场
*/
void splitBuyOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;	
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;
	
	double dailyGap = pDailyHigh - pIndicators->entryPrice;	
	double lots_singal;

	//double pWeeklyATR = pBase_Indicators->pWeeklyATR;
	//double pWeeklyHigh = pBase_Indicators->pWeeklyHigh;
	//double pWeeklyLow = pBase_Indicators->pWeeklyLow;

	//double weeklyGap = pWeeklyHigh - pIndicators->entryPrice;
	
	//pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->strategyMaxRisk

	//lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss)* pIndicators->risk;
	//lots_singal = lots_singal * (pIndicators->strategyMaxRisk - pIndicators->strategyRiskWithoutLockedProfit) / pIndicators->strategyMaxRisk;
	 
	// 1：1 			
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
第一单： 日内出场
第二单： 周内出场
*/
void splitSellOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pIndicators->entryPrice - pDailyLow;

	double lots_singal;

	//double pWeeklyATR = pBase_Indicators->pWeeklyATR;
	//double pWeeklyHigh = pBase_Indicators->pWeeklyHigh;
	//double pWeeklyLow = pBase_Indicators->pWeeklyLow;

	//double weeklyGap = pIndicators->entryPrice - pWeeklyLow;
	//lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, stopLoss)* pIndicators->risk;
	//lots_singal = lots_singal * (pIndicators->strategyMaxRisk - pIndicators->strategyRiskWithoutLockedProfit) / pIndicators->strategyMaxRisk;

	// 1：1 			
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
出现信号，止盈100点
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
出现信号，止盈100点
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
出现信号，止盈是一半的周波幅,或者是日波幅
*/
void splitBuyOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double pDailyATR = pBase_Indicators->pDailyATR;
	double pDailyHigh = pBase_Indicators->pDailyHigh;
	double pDailyLow = pBase_Indicators->pDailyLow;

	double dailyGap = pDailyHigh - pIndicators->entryPrice;

	double lots, lots_singal;

	time_t currentTime;
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pIndicators->isEnableBuyMinLotSize == TRUE)
		lots_singal = pIndicators->minLotSize;
	else
		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, max(stopLoss,pBase_Indicators->dailyATR*1.5)) * pIndicators->risk;

	//takePrice = pIndicators->takePrice;
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
		
	openSingleLongEasy(takePrice, stopLoss, lots_singal, 0);

}

/*
出现信号，止盈是一半的周波幅,或者是日波幅
*/
void splitSellOrders_4HSwing_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
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
	if (pIndicators->isEnableSellMinLotSize == TRUE)
		lots_singal = pIndicators->minLotSize;
	else
		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, max(stopLoss, pBase_Indicators->dailyATR*1.5)) * pIndicators->risk;
	
	//takePrice = pIndicators->takePrice;
	takePrice = max(pIndicators->takePrice, pIndicators->riskCap * stopLoss);
	

	openSingleShortEasy(takePrice, stopLoss, lots_singal, 0);
}


void splitBuyOrders_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	double lots;
	
	lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, stopLoss) * pIndicators->risk;

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
	double reminding, main;
	double test;
	
	if (pIndicators->tradeMode == 1)
	{
		//takePrice = 0;
		takePrice = pIndicators->riskCap * 1.5* pBase_Indicators->dailyATR;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, 1.5* pBase_Indicators->dailyATR) * pIndicators->risk;

	}
	else
	{
		takePrice = 0;
		lots = pParams->orderInfo[0].lots;
		stopLoss = abs(pParams->orderInfo[0].stopLoss - pIndicators->entryPrice);
	}

	if (pIndicators->riskCap > 0 && lots >= pIndicators->minLotSize)
	{
		lots = roundUp(lots, pIndicators->volumeStep);
		if (lots / pIndicators->volumeStep > 5)
		{		
			//takePrice = 1 * 1.5*  pBase_Indicators->dailyATR;
			openSingleLongEasy(takePrice, stopLoss, lots - pIndicators->minLotSize, 0);
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
		}
		else
			openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
	else
		openSingleLongEasy(takePrice, stopLoss, lots, 0);

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
	}
	else
	{
		takePrice = 0;
		lots = pParams->orderInfo[0].lots;
		stopLoss = abs(pParams->orderInfo[0].stopLoss - pIndicators->entryPrice);
	}
	
	if (pIndicators->riskCap > 0)
	{
		lots = roundUp(lots, pIndicators->volumeStep);
		if (lots / pIndicators->volumeStep > 5)
		{	
			//takePrice = 1 * 1.5* pBase_Indicators->dailyATR;;
			openSingleShortEasy(takePrice, stopLoss, lots - pIndicators->minLotSize, 0);
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, pIndicators->minLotSize, 0);
		}
		else
			openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
	else
		openSingleShortEasy(takePrice, stopLoss, lots, 0);

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
	// 1：1 			
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

	// 1：1 			
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

	//至少有1/3 空间
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

	//至少有1/3 空间
	if (gap >= pATR / 2)
	{		
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

void splitBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);
	double preClose = iClose(B_DAILY_RATES, 1);
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE)
	{
		//Fin 38.2%				
		openPrice = preHigh - (preHigh - preLow) * 0.382 + pIndicators->adjust;

		if (!isSameDaySamePriceBuyLimitOrderEasy(openPrice, currentTime))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		//Fin 50%
		openPrice = preHigh - (preHigh - preLow) * 0.5 + pIndicators->adjust;
		if (!isSameDaySamePriceBuyLimitOrderEasy(openPrice, currentTime))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{
		// Pivot 			
		openPrice = pBase_Indicators->dailyPivot + pIndicators->adjust;
		if (!isSameDaySamePriceBuyLimitOrderEasy(openPrice, currentTime))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

}
void splitSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);
	double preClose = iClose(B_DAILY_RATES, 1);

	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
	{
		//Fin 38.2%				
		openPrice = preLow + (preHigh - preLow) * 0.382 - pIndicators->adjust;
		if (!isSameDaySamePriceSellLimitOrderEasy(openPrice, currentTime))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		//Fin 50%
		openPrice = preLow + (preHigh - preLow) * 0.5 - pIndicators->adjust;
		if (!isSameDaySamePriceSellLimitOrderEasy(openPrice, currentTime))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
		// Pivot 		
		openPrice = pBase_Indicators->dailyPivot - pIndicators->adjust;
		if (!isSameDaySamePriceSellLimitOrderEasy(openPrice, currentTime))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
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

	double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow;
	double ATR0;
	double volume1, volume2;
	double fast, slow;
	double preFast, preSlow;

	double preClose1, preClose2,preClose3, preClose4, preClose5;
	
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;

	//Check out if Key K is triggered. If yes, take piority with other strategies., 
	//TODO 需要跳过riskPNL 的测试？？？？
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
		//if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal		
		//	workoutExecutionTrend_KongJian(pParams, pIndicators, pBase_Indicators);
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
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, tradeMode = %ld",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->tradeMode);

	//	pIndicators->tradeMode = 0;
	//}

	//如果MACD不支持，就不要做日内短单
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
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, volume1 = %lf, volume2 = %lf,tradeMode = %ld",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, volume1, volume2, pIndicators->tradeMode);

	//	pIndicators->tradeMode = 0;
	//}

	//在17点后，如果日波幅还没有出来，日内单就先出场。 当然，需要考虑美联储议息会议的日子吗？
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

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO, pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);

		if (ATR0_EURO < pIndicators->atr_euro_range)
			closeAllCurrentDayShortTermOrdersEasy(1, currentTime);					
	}


	////关掉所有日内单，当日波幅到了最大值
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
	//	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ATR0 =%lf,pDailyMaxATR = %lf close all current short term orders",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, pBase_Indicators->pDailyMaxATR);
	//	closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	//}


	//利用MACD Exit long term trades

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
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Exit Long term short trade:System InstanceID = %d, BarTime = %s, fast =%lf,slow = %lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, fast, slow);

			closeAllLongTermShortsEasy();
		}

		if (fast < slow ||
			(preClose5 > preClose4 && preClose5 > preClose3 && preClose5 > preClose2 && preClose5 > preClose1)) //If no new high in the last 4 days, close long term long trades
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"Exit Long term long trade:System InstanceID = %d, BarTime = %s, fast =%lf,slow = %lf",
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


	
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
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
			&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3))
			|| (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			))
		{
			pIndicators->entrySignal = 1;

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_BBS_BreakOut.",
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
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_BBS_BreakOut.",
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

	//满足多重通道当日动态的趋势分析
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
1. 需要考虑15MBBS support?
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

	//满足多重通道当日动态的趋势分析
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
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_Pivot.",
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

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_Pivot.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		}

		//Override the entry signal if the current daily high has been higher than stop loss price.
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
			pIndicators->entrySignal = 0;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}

//日内BBS, 入场是15MBBS,止损是关键支撑位置 而且都是 S2/R2 only
//可以加入总风险控制，避免连续入场。
//还有必须 close 在日内23：45
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

	//满足多重通道当日动态的趋势分析
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
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, enter long trade in workoutExecutionTrend_MIDDLE_RETREAT_PHASE.",
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
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, enter short trade in workoutExecutionTrend_MIDDLE_RETREAT_PHASE.",
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

	//满足多重通道当日动态的趋势分析
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
AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);

	closeAllWithNegativeEasy(2, currentTime, 3);

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->risk = 1;
	pIndicators->tpMode = 0;
	pIndicators->splitTradeMode = 5;

	//满足多重通道当日动态的趋势分析
	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_RETREAT_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->weeklyS) <= pBase_Indicators->weeklyATR * 0.666
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index
			&& pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust
			&& fabs(pIndicators->entryPrice - pBase_Indicators->weeklyS) <= pBase_Indicators->weeklyATR * 0.666
			&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}

	return SUCCESS;
}
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
	BOOL isOpen;
	OrderType side;
	double openOrderHigh, openOrderLow;

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

	// 满足日图历史的趋势分析。
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

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, high_4H %lf： low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // 100 pips,这应该是根据当时的波幅而定,40% weekly ATR
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
		//满足多重通道当日动态的趋势分析

		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - 20);

			//side = getLastestOrderTypeXAUUSDEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
			orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
			
			if( (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY) )
				&& fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR
				)
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
			if ( (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL) )
				&& fabs(iHigh(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR
				)
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;

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
	BOOL isOpen;
	OrderType side;
	double openOrderHigh, openOrderLow;

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

	// 满足日图历史的趋势分析。
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

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, high_4H %lf： low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // 100 pips,这应该是根据当时的波幅而定,40% weekly ATR
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
		//满足多重通道当日动态的趋势分析
		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		if (trend_4H == 1 && pIndicators->bbsTrend_4H == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->bbsStopPrice_4H;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pIndicators->stopLoss);
				
			//趋势转折的第一单
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != BUY))
			{
				pIndicators->entrySignal = 1;
			}
			
			////在一波趋势中，最多做3次
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

			//趋势转折的第一单
			if (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].type != SELL))
			{
				pIndicators->entrySignal = -1;
			}
			
			////在一波趋势中，最多做3次
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
这策略是以周趋势为基础，使用15MBBS入场。
趋势是4H关键K和M50,M200交叉决定。
移动止损，没有主动exit.
止盈是日内动态高低点和ATR
如果周波幅大于预测波幅，就停止入场，继续移动止损。
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
	

	// 满足日图历史的趋势分析。
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

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE)
		readWeeklyATRFile(pParams->tradeSymbol, &(pBase_Indicators->pWeeklyPredictATR), &(pBase_Indicators->pWeeklyPredictMaxATR), (BOOL)pParams->settings[IS_BACKTESTING]);
		
	trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

	movement = fabs(high_4H - low_4H);
	if (pIndicators->atr_euro_range == 0)
		pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;
	
	pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, high_4H %lf： low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
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
	//满足多重通道当日动态的趋势分析

	
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

		//区间震荡:
		//如果在区间上沿，仓位减半
		if (dailyTrend == 0
			&& pIndicators->entryPrice >= pBase_Indicators->dailyS
			&& fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) >= pBase_Indicators->dailyATR * 0.5)
			pIndicators->risk = 0.5;

		if (pIndicators->stopLossPrice > pIndicators->entryPrice) //invalid price. current price is above stop loss price. No entry trade.
			pIndicators->entrySignal = 0;

		////如果周和日图的趋势不一致，是否停止?还是50% risk?
		//if (dailyTrend < 0)
		//	pIndicators->risk = 0.5;

		// 退出如果4H方向改变。
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

		////如果周和日图的趋势不一致，是否停止?还是50% risk?
		//if (dailyTrend > 0)
		//	pIndicators->entrySignal = 0;


		if (pIndicators->stopLossPrice < pIndicators->entryPrice) //invalid price. current price is above stop loss price. No entry trade.
			pIndicators->risk = 0.5;


		// 退出如果4H方向改变。		
		//pIndicators->exitSignal = 1;
	}
	
	// If RiksPNL < 0, 
	// 离止损不到0.5 ATR
	// Risk = 0.5

	//if (pIndicators->strategyRiskWithoutLockedProfit < 0 && fabs(pIndicators->stopLossPrice - pIndicators->entryPrice) <= pBase_Indicators->dailyATR * 0.5)
	//{
	//		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, strategyRiskWithoutLockedProfit %lf： GapToSL= %lf, dailyATR=%lf",
	//			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRiskWithoutLockedProfit, fabs(pIndicators->stopLossPrice - pIndicators->entryPrice), pBase_Indicators->dailyATR );

	//	pIndicators->risk = 0.5;
	//}
	//if (pIndicators->strategyRiskWithoutLockedProfit  < pIndicators->limitRiskPNL
	//	&& ((pIndicators->entryPrice > preHigh - (preHigh - preLow) * 0.382 && pIndicators->entrySignal == 1)
	//	|| (pIndicators->entryPrice < preLow + (preHigh - preLow) * 0.382 && pIndicators->entrySignal == -1)))
	//{
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, strategyRisk %lf： limitRiskPNL %lf, entrySignal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRisk, pIndicators->limitRiskPNL, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}


AsirikuyReturnCode workoutExecutionTrend_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int    shift1Index_15M = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1;
	char       timeString1[MAX_TIME_STRING_SIZE] = "";
	double currentLow = iLow(B_DAILY_RATES, 0);
	double currentHigh = iHigh(B_DAILY_RATES, 0);


	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);

	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		pIndicators->executionTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		pIndicators->executionTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		pIndicators->executionTrend = -1;
	else
		pIndicators->executionTrend = 0;

	//if (pIndicators->executionTrend == 0 )
	//	pIndicators->exitSignal = EXIT_ALL;


	if (pIndicators->executionTrend == 1)
	{
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
		{
			pIndicators->entrySignal = 1;
			pIndicators->stopLossPrice = pBase_Indicators->dailyLow;
			pIndicators->tpMode = 2;
			pIndicators->splitTradeMode = 4;
		}

		pIndicators->exitSignal = EXIT_SELL;

	}

	if (pIndicators->executionTrend == -1)
	{
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
		{
			pIndicators->entrySignal = -1;
			pIndicators->stopLossPrice = pBase_Indicators->dailyHigh;
			pIndicators->tpMode = 2;
			pIndicators->splitTradeMode = 4;
		}
		pIndicators->exitSignal = EXIT_BUY;
	}
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

	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, asiBull =%lf, asiBear=%lf",
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
	int index1, index2, index3;
	double level = 0, histLevel = 0, maxLevel = 0;
	

	double preClose1, preClose2, preClose3, preClose4, preClose5;
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

	double preWeeklyClose, preWeeklyClose1;
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

	int truningPointIndex = -1;

	//double atr5Limit = pParams->bidAsk.ask[0] * 0.01 *0.55;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// 满足日图历史的趋势分析。

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

	//如果是23H，可以检查是否可以入场
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


	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 交易， 避开开盘的一个小时,同时只是新的一天的第一个小时
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
		//preVolume > MA(10): 当前的volum > 过去10天的平均volume
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		//volume_ma_10 = iMA(4, B_DAILY_RATES, 10, startShift);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf",
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


		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
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

					//如果是在趋势的后端，不做。
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
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex))
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					//如果方向改变的第一个trade,离baseline 超过1 ATR(20),不做。
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}


			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//	&& pParams->orderInfo[orderIndex].openPrice - pIndicators->entryPrice > abs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / 2) //如果有单，就回调加仓。 
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

					//如果是在趋势的后端，不做。
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
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL,&truningPointIndex))
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					//如果方向改变的第一个trade,离baseline 超过1 ATR(20),不做。
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}
			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//	&& pIndicators->entryPrice - pParams->orderInfo[orderIndex].openPrice > abs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / 2) //如果有单，就回调加仓。 
			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = -1;
			//}

			pIndicators->exitSignal = EXIT_BUY;

		}


		//加入日图的出场信号:进入range
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

2. 日图的移动止损。 
如果dailyTrend = 0, 或者 MACD出现相反信号

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
	int    shiftPreDayBar = shift1Index -1;
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	//double fast, slow;
	//double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	//double ma50Daily, preDailyClose;
	int index1, index2, index3;
	double level = 0, histLevel = 0, maxLevel = 0;
	//double volume1,volume2, volume_ma_10;

	double preClose1, preClose2, preClose3, preClose4, preClose5;
	double ma20Daily, preDailyClose;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;
	double dailyBaseLine;

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

	BOOL isEnableASI = FALSE;

	BOOL isEnableLate = FALSE;

	BOOL isEnableNextdayBar = FALSE;

	BOOL isEnableNoStopLoss = FALSE;

	double preWeeklyClose, preWeeklyClose1;
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
	int truningPointIndex = -1;

	//double rangeHigh, rangeLow;
	int range = 10000;

	//double atr5Limit = pParams->bidAsk.ask[0] * 0.01 *0.55;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);


	// 满足日图历史的趋势分析。
	
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

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	{

		level = 0.005 * pParams->bidAsk.ask[0];
		maxLevel = 0.05 * pParams->bidAsk.ask[0];
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;

		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;
		isWeeklyBaseLine = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

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
	}
	else if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
	{

		level = 0.35;// min(0.35, 0.0053 * pParams->bidAsk.ask[0]);
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

		pIndicators->minLotSize = 0.5;
		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);

		pIndicators->volumeStep = 0.5;
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

		isEnableLate = FALSE;		

		//isEnableASI = TRUE;

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
		isVolumeControl = TRUE;
		isEnableBeiLi = TRUE;

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

		isEnableLate = FALSE;

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
		isEnableCMFVolume = TRUE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;

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
		maxLevel =0.008;
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

		isEnableMaxLevel = TRUE;

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

	//如果是23H，可以检查是否可以入场
	if (isEnableEntryEOD == TRUE && timeInfo1.tm_hour == 23)
	{
		startShift = 0;		
		macdLimit = level /2 ;

		if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
		{
			isDailyOnly = FALSE;
		}
	}

	preBarTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shiftPreDayBar];
	safe_gmtime(&timeInfoPreBar, preBarTime);

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1 
		&& (isDailyOnly == FALSE || timeInfo1.tm_mday != timeInfoPreBar.tm_mday)
		) // 1:00 交易， 避开开盘的一个小时,同时只是新的一天的第一个小时
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
		//preVolume > MA(10): 当前的volum > 过去10天的平均volume
		pIndicators->volume1 = iVolume(B_DAILY_RATES, startShift);
		pIndicators->volume2 = iVolume(B_DAILY_RATES, startShift + 1);
		//volume_ma_10 = iMA(4, B_DAILY_RATES, 10, startShift);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, cmfVolume =%lf, CMFVolumeGap=%lf, weekly_baseline=%lf, weekly_baseline_short=%lf,volume1=%lf,volume2=%lf",
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
		

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift), pIndicators->fast, pIndicators->slow);
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, startShift + 1), pIndicators->preFast, pIndicators->preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		pIndicators->stopLoss = stopLoss;

		if (pIndicators->fast > level
			&& (isEnableSlow == FALSE || pIndicators->slow > 0)
			//&& dailyTrend > 0 			
			&& preDailyClose > dailyBaseLine
			&& pIndicators->fast - pIndicators->slow > macdLimit
			) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
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
			
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

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
				&& (isEnableASI == FALSE|| asiBull > asiBear)				
				)
			{

				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if ( timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{
					pIndicators->entrySignal = 1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 > pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap > 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume > 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap > 0 && pIndicators->cmfVolume >  0 && pIndicators->volume1 > pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;


					//如果是在趋势的后端，不做。
					if (isEnableLate == TRUE && preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
						&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
						&& orderIndex >= 0  && pParams->orderInfo[orderIndex].type == BUY												
						)
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, it is late for 5 days",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableMaxLevel == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& pIndicators->fast > maxLevel
						)
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0 
						//&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, BUY, &truningPointIndex) //&& iClose(B_DAILY_RATES,1) < rangeHigh
						&& truningPointIndex - 1 <= range
						)
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					//如果方向改变的第一个trade,离baseline 超过1 ATR(20),不做。
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}

				
			}			
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//	&& pParams->orderInfo[orderIndex].openPrice - pIndicators->entryPrice > abs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / 2) //如果有单，就回调加仓。 
			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = 1;
			//}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (pIndicators->fast < (level * -1)
			&& (isEnableSlow == FALSE || pIndicators->slow < 0)
			//&& dailyTrend < 0			
			&& preDailyClose < dailyBaseLine
			&& pIndicators->slow - pIndicators->fast > macdLimit
//			&& fast < preFast			
			) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];	

			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
			
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

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, entryPrice=%lf, preclose=%lf",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, iClose(B_DAILY_RATES, startShift));

			if (//fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				&& pIndicators->fast < pIndicators->preFast
				&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
				&& (isVolumeControl == FALSE || pIndicators->volume1 > pIndicators->volume2)
				&& (isEnableCMFVolume == FALSE || pIndicators->cmfVolume < 0)
				&& (isEnableCMFVolumeGap == FALSE || pIndicators->CMFVolumeGap < 0)
				&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline
				&& (weekly_baseline_short < weekly_baseline || pre3KTrend == DOWN))
				)
				&& iClose(B_DAILY_RATES, startShift) - pIndicators->entryPrice <= 0.2 * pBase_Indicators->dailyATR
				&& (isEnableASI == FALSE || asiBull < asiBear)				
				)
			{
				safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, timeInfo1.tm_mday =%ld, timeInfo2.tm_mday%ld",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, timeInfo1.tm_mday, timeInfo2.tm_mday);

				if (timeInfo1.tm_mday != timeInfo2.tm_mday || timeInfo1.tm_mon != timeInfo2.tm_mon)
				{

					pIndicators->entrySignal = -1;

					if ((isVolumeControlRisk == TRUE && pIndicators->volume1 < pIndicators->volume2)
						|| (isCMFVolumeGapRisk == TRUE && pIndicators->CMFVolumeGap < 0)
						|| (isCMFVolumeRisk == TRUE && pIndicators->cmfVolume < 0)
						|| (isAllVolumeRisk == TRUE && pIndicators->CMFVolumeGap < 0 && pIndicators->cmfVolume <  0 && pIndicators->volume1 < pIndicators->volume2)
						)
						pIndicators->risk = maxRisk;

					//如果是在趋势的后端，不做。
					if (isEnableLate == TRUE && preHist1 < (histLevel*-1) && preHist2 < (histLevel*-1) && preHist3 < (histLevel*-1) && preHist4 < (histLevel*-1) && preHist5 < (histLevel*-1)
						&& fast1 < (level*-1) && fast2 < (level*-1) && fast3 < (level*-1) && fast4 < (level*-1) && fast5 < (level*-1)
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL						
						)
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, it is late for 5 days",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
						pIndicators->entrySignal = 0;
					}

					if (isEnableMaxLevel == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& pIndicators->fast < (maxLevel*-1)
						)
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD exceeds max level",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					if (isEnableBeiLi == TRUE
						&& pIndicators->entrySignal != 0
						&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
						&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0, SELL, &truningPointIndex) //&& iClose(B_DAILY_RATES, 1) > rangeLow
						&& truningPointIndex - 1 <= range
						)
					{
						pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

						pIndicators->entrySignal = 0;
					}

					//如果方向改变的第一个trade,离baseline 超过1 ATR(20),不做。
					//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY && fabs(preDailyClose - ma20Daily) >= iAtr(B_DAILY_RATES, 20, 1))
					//	pIndicators->entrySignal = 0;
				}
			}
			//else if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//	&& pIndicators->entryPrice - pParams->orderInfo[orderIndex].openPrice > abs(pParams->orderInfo[orderIndex].stopLoss - pParams->orderInfo[orderIndex].openPrice) / 2) //如果有单，就回调加仓。 
			//{
			//	pIndicators->tradeMode = 2;
			//	pIndicators->entrySignal = -1;
			//}

			pIndicators->exitSignal = EXIT_BUY;

		}

		if (
			pIndicators->entrySignal != 0							
			&& isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar(startShift) == FALSE
			//orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
			//&& isEnableNextdayBar == TRUE && isNextdayMACDPostiveBar2(pParams, orderIndex, startShift) == FALSE
			)
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Nextday MACD Bar is negative.",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
			pIndicators->entrySignal = 0;
			//pIndicators->exitSignal = EXIT_ALL;
		}

		//加入日图的出场信号:进入range

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


	return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_MACD_Daily_ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double fast, slow;
	double preFast, preSlow;
	int orderIndex;
	double ma50Daily,preDailyClose;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。
	if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
		dailyTrend = 0;
	else if (pBase_Indicators->dailyTrend > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	ma50Daily = iMA(3, B_DAILY_RATES, 50, 1);
	preDailyClose = iClose(B_DAILY_RATES, 1);
	if (timeInfo1.tm_hour == 1) // 1:00 交易， 避开开盘的一个小时。
	{

		//Load MACD
		fast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 1);
		slow = iMACD(B_DAILY_RATES, 5, 10, 5, 1, 1);


		preFast = iMACD(B_DAILY_RATES, 5, 10, 5, 0, 2);
		preSlow = iMACD(B_DAILY_RATES, 5, 10, 5, 1, 2);


		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, 1), fast, slow);
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_DAILY_RATES, 2), preFast, preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);


		if (preDailyClose > ma50Daily) // Buy
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

			if (fast > 0 && slow > 0 && fast > slow && preFast <= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				)
			{
				pIndicators->entrySignal = 1;
			}

			pIndicators->exitSignal = EXIT_SELL;

		}

		if (preDailyClose < ma50Daily) // Sell
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pBase_Indicators->dailyS;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

			if (fast < 0 && slow < 0 && fast < slow && preFast >= preSlow &&
				(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				)
			{
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;

		}


		//加入日图的出场信号:进入range

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
这里只是用于回测，只有日图使用。
pBase_Indicators为空。
B_PRIMARY_RATES 是日图。
*/
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily_Chart_RegressionTest(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift0Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 1;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double fast, slow;
	double preFast, preSlow;
	int orderIndex;
	double atr5 = iAtr(B_PRIMARY_RATES, 5, 1);
	int index1, index2, index3;
	double level = 0, histLevel = 0, maxLevel = 0;
	double volume1, volume2, volume_ma_10;

	double preClose1, preClose2, preClose3, preClose4, preClose5;
	double ma20Daily, preDailyClose;
	double dailyBaseLine;
	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;

	double cmfVolume = 0.0;
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

	BOOL isMACDZeroExit = FALSE;

	BOOL isEnableASI = FALSE;

	BOOL isEnableLate = FALSE;

	double preWeeklyClose, preWeeklyClose1;		
	double shortDailyHigh = 0.0, shortDailyLow = 0.0, dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0, shortWeeklyHigh = 0.0, shortWeeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0, weekly_baseline_short = 0.0;
	int pre3KTrend;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;
	double CMFVolumeGap = 0.0;
	double maxRisk = 1.5;

	int startShift = 1;

	double asiBull, asiBear;
	int truningPointIndex = -1;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->splitTradeMode = 24;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	pIndicators->stopMovingBackSL = FALSE;

	ma20Daily = iMA(3, B_DAILY_RATES, 20, 1);
	preDailyClose = iClose(B_DAILY_RATES, 1);

	dailyBaseLine = ma20Daily;

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	{

		level = 0.005 * pParams->bidAsk.ask[0];
		maxLevel = 0.05 * pParams->bidAsk.ask[0];
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

		//stopLoss = pBase_Indicators->dailyATR * 1.8;

		maxRisk = 1.5;

		//isDailyOnly = FALSE;

		//shiftPreDayBar = shift1Index;

		pIndicators->stopMovingBackSL = TRUE;

		//isEnableEntryEOD = TRUE;

		isEnableLate = FALSE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, startShift);

		pIndicators->riskCap = parameter(AUTOBBS_RISK_CAP);
	}
	else if (strstr(pParams->tradeSymbol, "XTIUSD") != NULL)
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
		
	}
	else if (strstr(pParams->tradeSymbol, "GER30EUR") != NULL)
	{
		level = 30;
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

		isWeeklyBaseLine = FALSE;

		maxRisk = 2;
		
	}
	else if (strstr(pParams->tradeSymbol, "XAGUSD") != NULL)
	{
		level = 0.3; // XAUUSD
		maxLevel = 0.5;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = FALSE;

		isCMFVolumeGapRisk = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isWeeklyBaseLine = TRUE;

		maxRisk = 2;

	}
	else if (strstr(pParams->tradeSymbol, "XPDUSD") != NULL)
	{
		level = 2; // XAUUSD
		maxLevel = 10;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isEnableMaxLevel = TRUE;

		isCMFVolumeGapRisk = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		isWeeklyBaseLine = FALSE;

		maxRisk = 2;

		isMACDZeroExit = TRUE;

		isEnableASI = TRUE;
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

	}
	else if (strstr(pParams->tradeSymbol, "GBPCHF") != NULL)
	{
		level = 0.002; //GBPJPY
		maxLevel = 0.008;
		histLevel = 0.01;
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

		isAllVolumeRisk = TRUE;
		//isCMFVolumeRisk = TRUE;
			 
		isEnableMaxLevel = TRUE;

		isWeeklyBaseLine = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		pIndicators->stopMovingBackSL = TRUE;

		//isMACDZeroExit = TRUE;

		//isEnableASI = TRUE;

		dailyBaseLine = iMA(3, B_DAILY_RATES, 50, 1); // 50 Daily MA
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

		isWeeklyBaseLine = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		pIndicators->stopMovingBackSL = TRUE;

		//isMACDZeroExit = TRUE;

		isEnableASI = TRUE;
		
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

		isWeeklyBaseLine = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;
	}
	else if (strstr(pParams->tradeSymbol, "USDSGD") != NULL)
	{
		level = 0.001; //USDSGD
		maxLevel = 0.008;
		histLevel = 0.01;
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		

		//isEnableSlow = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;
		isEnableCMFVolume = FALSE;
		isEnableCMFVolumeGap = FALSE;

		isAllVolumeRisk = FALSE;
		//isCMFVolumeRisk = TRUE;

		isEnableMaxLevel = FALSE;

		isWeeklyBaseLine = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		pIndicators->stopMovingBackSL = TRUE;

		//dailyBaseLine = iMA(3, B_DAILY_RATES, 50, 1); // 50 Daily MA
	}
	else
	{
		level = 0; //EURUSD
		maxLevel = 0.005;
	}

	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
	iTrend3Rules_preDays(pParams, pIndicators, B_MONTHLY_RATES, 2, &pre3KTrend, 1,0);

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	cmfVolume = getCMFVolume(B_DAILY_RATES, fastMAPeriod, startShift);
	CMFVolumeGap = getCMFVolumeGap(B_DAILY_RATES, 1, fastMAPeriod, startShift);

	//Volume indicator....
	//preVolume > MA(10): 当前的volum > 过去10天的平均volume
	volume1 = iVolume(B_PRIMARY_RATES, startShift);
	volume2 = iVolume(B_PRIMARY_RATES, startShift+1);
	volume_ma_10 = iMA(4, B_PRIMARY_RATES, 10, startShift);

	//Load MACD
	iMACDAll(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift, &fast1, &slow1, &preHist1);
	iMACDAll(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 1, &fast2, &slow2, &preHist2);
	iMACDAll(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 2, &fast3, &slow3, &preHist3);
	iMACDAll(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 3, &fast4, &slow4, &preHist4);
	iMACDAll(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, startShift + 4, &fast5, &slow5, &preHist5);

	fast = fast1;
	slow = slow1;
	preFast = fast2;
	preSlow = slow2;


	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_PRIMARY_RATES, startShift), fast, slow);
	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_PRIMARY_RATES, startShift+1), preFast, preSlow);

	orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

	//load pBase_Indicators
	pBase_Indicators->dailyATR = iAtr(B_PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), startShift);
	pBase_Indicators->pDailyMaxATR = 1.5 * pBase_Indicators->dailyATR;

	iASIEasy(B_DAILY_RATES, 0, 10, 5, &asiBull, &asiBear);

	if (fast > level
		&& (isEnableSlow == FALSE || slow > 0)
		&& preDailyClose > dailyBaseLine
		&& fast > slow
		) // Buy
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];

		pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->pDailyMaxATR;
				
		if (//fast > slow && preFast <= preSlow &&				
			(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
			&& fast > preFast
			&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
			&& (isVolumeControl == FALSE || volume1 > volume2)
			&& (isEnableCMFVolume == FALSE || cmfVolume > 0)
			&& (isEnableCMFVolumeGap == FALSE || CMFVolumeGap > 0)
			&& (isWeeklyBaseLine == FALSE ||
			(preWeeklyClose > weekly_baseline && (weekly_baseline_short > weekly_baseline || pre3KTrend == UP))
			)
			&& (isEnableASI == FALSE || asiBull > asiBear)
			)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday)
			{
				pIndicators->entrySignal = 1;

				if ((isVolumeControlRisk == TRUE && volume1 > volume2)
					|| (isCMFVolumeGapRisk == TRUE && CMFVolumeGap > 0)
					|| (isCMFVolumeRisk == TRUE && cmfVolume > 0)
					|| (isAllVolumeRisk == TRUE && CMFVolumeGap > 0 && cmfVolume >  0 && volume1 > volume2)
					)
					pIndicators->risk = maxRisk;

				//如果是在趋势的后端，不做。
				//不要入场，在发生背离的时候

				if (isEnableLate == TRUE && preHist1 > histLevel && preHist2 > histLevel && preHist3 > histLevel && preHist4 > histLevel && preHist5 > histLevel
					&& fast1 > level && fast2 > level && fast3 > level && fast4 > level && fast5 > level
					&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
					)
				{
					pIndicators->entrySignal = 0;
				}

				if (isEnableMaxLevel == TRUE
					&& pIndicators->entrySignal != 0
					&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
					&& fast > maxLevel
					)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD exceeds max level",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

					pIndicators->entrySignal = 0;
				}

				if (isEnableBeiLi == TRUE
					&& pIndicators->entrySignal != 0
					&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == BUY
					&& iMACDTrendBeiLiEasy(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex))
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

					pIndicators->entrySignal = 0;
				}
			}

		}
		pIndicators->exitSignal = EXIT_SELL;

	}

	if (fast < (level * -1)
		&& (isEnableSlow == FALSE || slow < 0)
		//&& dailyTrend < 0			
		&& preDailyClose < dailyBaseLine
		&& fast < slow
		//			&& fast < preFast			
		) // Sell
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->pDailyMaxATR;

		if (//fast < slow && preFast >= preSlow &&
			(orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
			&& fast < preFast
			&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
			&& (isVolumeControl == FALSE || volume1 > volume2)
			&& (isEnableCMFVolume == FALSE || cmfVolume < 0)
			&& (isEnableCMFVolumeGap == FALSE || CMFVolumeGap > 0)		
			&& (isWeeklyBaseLine == FALSE ||
				(preWeeklyClose < weekly_baseline
				&& (weekly_baseline_short < weekly_baseline || pre3KTrend == DOWN))
				)
			&& (isEnableASI == FALSE || asiBull < asiBear)
			)
		{
			safe_gmtime(&timeInfo2, pParams->orderInfo[orderIndex].closeTime);
			if (timeInfo1.tm_mday != timeInfo2.tm_mday)
			{
				pIndicators->entrySignal = -1;
			}

			if ((isVolumeControlRisk == TRUE && volume1 < volume2)
				|| (isCMFVolumeGapRisk == TRUE && CMFVolumeGap < 0)
				|| (isCMFVolumeRisk == TRUE && cmfVolume < 0)
				|| (isAllVolumeRisk == TRUE && CMFVolumeGap < 0 && cmfVolume < 0 && volume1 < volume2)
				)
				pIndicators->risk = maxRisk;

			//如果是在趋势的后端，不做。
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
				&& fast < (maxLevel*-1)
				)
			{
				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD exceeds max level",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				pIndicators->entrySignal = 0;
			}

			if (isEnableBeiLi == TRUE
				&& pIndicators->entrySignal != 0
				&& orderIndex >= 0 && pParams->orderInfo[orderIndex].type == SELL
				&& iMACDTrendBeiLiEasy(B_PRIMARY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex))
			{
				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				pIndicators->entrySignal = 0;
			}
		}

		pIndicators->exitSignal = EXIT_BUY;

	}

	//加入日图的出场信号:进入range
	if (isMACDZeroExit == FALSE)
	{
		if (fast > slow && preFast <= preSlow) // Exit SELL		
			pIndicators->exitSignal = EXIT_SELL;

		if (fast < slow && preFast >= preSlow) // Exit SELL		
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
	int index1, index2, index3;
	double level = 0;
	double volume1, volume2, volume_ma_3;

	currentTime = pParams->ratesBuffers->rates[B_WEEKLY_RATES].time[shift0Index_Weekly];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。

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

	//if (timeInfo1.tm_hour == 1) // 1:00 交易， 避开开盘的一个小时。
	{
		//Volume indicator....
		//preVolume > MA(3): 当前的volum > 过去3天的平均volume
		volume1 = iVolume(B_WEEKLY_RATES, 1);
		volume2 = iVolume(B_WEEKLY_RATES, 2);
		volume_ma_3 = iMA(4, B_WEEKLY_RATES, 3, 1);

		//Load MACD
		fast = iMACD(B_WEEKLY_RATES, 5, 10, 5, 0, 1);
		slow = iMACD(B_WEEKLY_RATES, 5, 10, 5, 1, 1);


		preFast = iMACD(B_WEEKLY_RATES, 5, 10, 5, 0, 2);
		preSlow = iMACD(B_WEEKLY_RATES, 5, 10, 5, 1, 2);


		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, fast=%lf, slow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 1), fast, slow);
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, preClose =%lf, preFast=%lf, preSlow=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, iClose(B_WEEKLY_RATES, 2), preFast, preSlow);

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		//if (iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR)
		//{
		//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, pDailyPredictATR =%lf, ATRWeekly0 = %lf,pWeeklyPredictMaxATR=%lf",
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


		//加入日图的出场信号:进入range

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
Weekly Ichimoko baseline 决定年内方向。

2. Entry:

For the first trading signal:
if Trend > 0 && daily close price > daily baseline, buy signal
if Trend < 0 && daily close price < daily baseline, sell signal

If 价格离base line > 1.5 daily ATR, 等待一天。

For the continuous trading signal on daily open:
Option 1:
if close higher: close(1) - close (2) > 25% daily ATR(20) -> buy signal
if close closer: close(1) - close (2) < 25% daily ATR(20) *-1  -> sell signal

Option 2:
根据AutoBBS 的trend phase:
if trend phase is up, buy signal
if trend phase is down, sell signal 

Filter:
if price > weekly R2/S2, dont trade
if current weekly ATR > weekly max ATR, dont trade

Option 3: Only it is on a strong trend.
if price retreats back to 4H MA50 Plus 0.15%, buy or sell signal. 

2. 日图的移动止损。

If price move against to base line, exit.

SL: it will be max(baseline + 20% ATR(20), max weekly ATR） 

3. No TP

But it will be subject to profit managment, like start to close profit trade if the floading profit is more than 5 % on this strategy. 

4. Risk = 0.5% each trade.
*/
AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
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
	double dailyHigh = 0.0, dailyLow = 0.0,weeklyHigh = 0.0,weeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 1.5;
	double strategyMarketVolRisk = 0.0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。

	if (pBase_Indicators->dailyTrend_Phase > 0)
		dailyTrend = 1;
	else if (pBase_Indicators->dailyTrend_Phase < 0)
		dailyTrend = -1;
	else
		dailyTrend = 0;

	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;

	pIndicators->tradeMode = 1;

	
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

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1) // 1:00 交易， 避开开盘的一个小时。
	{


		//计算daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);


		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&			
			dailyOnly == 1 
			&& preDailyClose > daily_baseline 
			//&& preWeeklyClose > weekly_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				preDailyClose > preDailyClose1		
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{				
				pIndicators->entrySignal = 1;
				//if (pBase_Indicators->dailyTrend <= 0)
				//	pIndicators->risk = 0.5;
		
				//如果是在趋势的后端，不做。
				if (pIndicators->entryPrice > pBase_Indicators->weeklyR2 
					||	iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR 
					//||	pBase_Indicators->dailyTrend_Phase == 0
					)
				{
					pIndicators->entrySignal = 0;
				}

			}
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
			dailyOnly == 1 
			&& preDailyClose < daily_baseline 
			//&& preWeeklyClose < weekly_baseline 
			) // Sell
		{

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];

			pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (				
				preDailyClose < preDailyClose1
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = -1;
				//if (pBase_Indicators->dailyTrend >= 0)
				//	pIndicators->risk = 0.5;

				//如果是在趋势的后端，不做。
				if (pIndicators->entryPrice < pBase_Indicators->weeklyS2 
					|| iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR 
					//|| pBase_Indicators->dailyTrend_Phase == 0
					)
				{
					pIndicators->entrySignal = 0;
				}

			}
			pIndicators->exitSignal = EXIT_BUY;

		}

		
	}

	//3K reverse exit trades
	strategyMarketVolRisk = caculateStrategyVolRiskForNoTPOrdersEasy(pBase_Indicators->pWeeklyPredictMaxATR);

	if (pBase_Indicators->daily3RulesTrend == UP && strategyMarketVolRisk < pIndicators->strategyMaxRisk / 3 * 2 )
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,daily3RulesTrend = %ld, strategyMarketVolRisk =%lf, strategyMaxRisk=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->daily3RulesTrend, strategyMarketVolRisk, pIndicators->strategyMaxRisk);
		pIndicators->exitSignal = EXIT_SELL;
	}
	if (pBase_Indicators->daily3RulesTrend == DOWN && strategyMarketVolRisk  < pIndicators->strategyMaxRisk / 3 * 2)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,daily3RulesTrend = %ld, strategyMarketVolRisk =%lf, strategyMaxRisk=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->daily3RulesTrend, strategyMarketVolRisk, pIndicators->strategyMaxRisk);
		pIndicators->exitSignal = EXIT_BUY;
	}

	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	// Daily Profit on EOD?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->dailyR3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->dailyS3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);


	// Weekly Profit on EOW?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
	{
		//pIndicators->tradeMode = 0;
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2)
	{
		//pIndicators->tradeMode = 0;
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}

	// when floating profit is too high, fe 10%
	if (pIndicators->riskPNL >targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	return SUCCESS;
}


/*
Run on daily chart. Ichimoko base line only

1. Trend:
Weekly Ichimoko baseline 决定年内方向。

2. Entry:

For the first trading signal:
if Trend > 0 && daily close price > daily baseline, buy signal
if Trend < 0 && daily close price < daily baseline, sell signal

If 价格离base line > 1.5 daily ATR, 等待一天。

For the continuous trading signal on daily open:
Option 1:
if close higher: close(1) - close (2) > 25% daily ATR(20) -> buy signal
if close closer: close(1) - close (2) < 25% daily ATR(20) *-1  -> sell signal

Option 2:
根据AutoBBS 的trend phase:
if trend phase is up, buy signal
if trend phase is down, sell signal

Filter:
if price > weekly R2/S2, dont trade
if current weekly ATR > weekly max ATR, dont trade

Option 3: Only it is on a strong trend.
if price retreats back to 4H MA50 Plus 0.15%, buy or sell signal.

2. 日图的移动止损。

If price move against to base line, exit.

SL: it will be max(baseline + 20% ATR(20), max weekly ATR）

3. No TP

But it will be subject to profit managment, like start to close profit trade if the floading profit is more than 5 % on this strategy.

4. Risk = 0.5% each trade.
*/
AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_V2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
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
	double shortDailyHigh = 0.0, shortDailyLow = 0.0,dailyHigh = 0.0, dailyLow = 0.0, weeklyHigh = 0.0, weeklyLow = 0.0;
	double daily_baseline = 0.0, weekly_baseline = 0.0, daily_baseline_short = 0.0;
	int orderIndex;
	int dailyOnly = 1;

	double targetPNL = 0;
	double strategyMarketVolRisk = 0.0;
	double strategyVolRisk = 0.0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。

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

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1) // 1:00 交易， 避开开盘的一个小时。
	{


		//计算daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
			preDailyClose > daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25;
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
				//if (pBase_Indicators->dailyTrend <= 0)
				//	pIndicators->risk = 0.5;

				//如果是在趋势的后端，不做。
				if (pIndicators->entryPrice > pBase_Indicators->weeklyR2
					|| iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR
					//||	pBase_Indicators->dailyTrend_Phase == 0
					)
				{
					pIndicators->entrySignal = 0;
				}

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
				//if (pBase_Indicators->dailyTrend >= 0)
				//	pIndicators->risk = 0.5;

				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s entering short trade",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);

				//如果是在趋势的后端，不做。
				if (pIndicators->entryPrice < pBase_Indicators->weeklyS2
					|| iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR
					//|| pBase_Indicators->dailyTrend_Phase == 0
					)
				{
					pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s revert a short trade: entryPrice=%lf,weeklyS2=%lf,current week ATR=%lf,pWeeklyPredictMaxATR=%lf  ",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entryPrice, pBase_Indicators->weeklyS2, iAtr(B_WEEKLY_RATES, 1, 0), pBase_Indicators->pWeeklyPredictMaxATR);

					pIndicators->entrySignal = 0;
				}

			}
			pIndicators->exitSignal = EXIT_BUY;

		}


	}

	//3K reverse exit trades
	strategyMarketVolRisk = caculateStrategyVolRiskForNoTPOrdersEasy(pBase_Indicators->pWeeklyPredictMaxATR);

	if (pBase_Indicators->daily3RulesTrend == UP && strategyMarketVolRisk < strategyVolRisk)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,daily3RulesTrend = %ld, strategyMarketVolRisk =%lf, strategyMaxRisk=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->daily3RulesTrend, strategyMarketVolRisk, pIndicators->strategyMaxRisk);
		pIndicators->exitSignal = EXIT_SELL;
	}
	if (pBase_Indicators->daily3RulesTrend == DOWN && strategyMarketVolRisk  < strategyVolRisk)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,daily3RulesTrend = %ld, strategyMarketVolRisk =%lf, strategyMaxRisk=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->daily3RulesTrend, strategyMarketVolRisk, pIndicators->strategyMaxRisk);
		pIndicators->exitSignal = EXIT_BUY;
	}

	profitManagement_base(pParams, pIndicators, pBase_Indicators);

	// Daily Profit on EOD?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->dailyR3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->dailyS3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);


	// Weekly Profit on EOW?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
	{
		//pIndicators->tradeMode = 0;
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2)
	{
		//pIndicators->tradeMode = 0;
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	}

	targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	// when floating profit is too high, fe 10%
	if (pIndicators->riskPNL >targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	return SUCCESS;
}


AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_V3(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
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

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。

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

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1) // 1:00 交易， 避开开盘的一个小时。
	{


		//计算daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&			
			dailyOnly == 1
			&& preDailyClose > daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];

			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25;
			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				preDailyClose > preDailyClose1
				&& preDailyClose > daily_baseline_short
				&& daily_baseline_short > daily_baseline
				&& preWeeklyClose > weekly_baseline
				&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = 1;
				//if (pBase_Indicators->dailyTrend <= 0)
				//	pIndicators->risk = 0.5;

				//如果是在趋势的后端，不做。
				if (pIndicators->entryPrice > pBase_Indicators->weeklyR2
					|| iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR
					//||	pBase_Indicators->dailyTrend_Phase == 0
					)
				{
					pIndicators->entrySignal = 0;
				}

			}
			pIndicators->exitSignal = EXIT_SELL;

		}

		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
			dailyOnly == 1
			&& preDailyClose < daily_baseline
			) // Sell
		{

			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.ask[1];

			pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);

			//Option 1:
			if (
				preDailyClose < preDailyClose1
				&& preDailyClose < daily_baseline_short
				&& daily_baseline_short < daily_baseline
				&& preWeeklyClose < weekly_baseline
				&& (orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
				)
			{
				pIndicators->entrySignal = -1;
				//if (pBase_Indicators->dailyTrend >= 0)
				//	pIndicators->risk = 0.5;

				//如果是在趋势的后端，不做。
				if (pIndicators->entryPrice < pBase_Indicators->weeklyS2
					|| iAtr(B_WEEKLY_RATES, 1, 0) > pBase_Indicators->pWeeklyPredictMaxATR
					//|| pBase_Indicators->dailyTrend_Phase == 0
					)
				{
					pIndicators->entrySignal = 0;
				}

			}
			pIndicators->exitSignal = EXIT_BUY;

		}


	}


	return SUCCESS;
}

/*
Run on daily chart. Ichimoko base line only

1. Trend:
Weekly Ichimoko baseline 决定年内方向。

2. Entry:

For the first trading signal:
if Trend > 0 && daily close price > daily baseline, buy signal
if Trend < 0 && daily close price < daily baseline, sell signal

If 价格离base line > 1.5 daily ATR, 等待一天。

For the continuous trading signal on daily open:
Option 1:
if close higher: close(1) - close (2) > 25% daily ATR(20) -> buy signal
if close closer: close(1) - close (2) < 25% daily ATR(20) *-1  -> sell signal

Option 2:
根据AutoBBS 的trend phase:
if trend phase is up, buy signal
if trend phase is down, sell signal

Filter:
if price > weekly R2/S2, dont trade
if current weekly ATR > weekly max ATR, dont trade

Option 3: Only it is on a strong trend.
if price retreats back to 4H MA50 Plus 0.15%, buy or sell signal.

2. 日图的移动止损。

If price move against to base line, exit.

SL: it will be max(baseline + 20% ATR(20), max weekly ATR）

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

	// 满足日图历史的趋势分析。

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

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
	if (timeInfo1.tm_hour >= 1) // 1:00 交易， 避开开盘的一个小时。
	{


		//计算daily and weekly baseline
		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
		daily_baseline = (dailyHigh + dailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
		daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

		iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
		weekly_baseline = (weeklyHigh + weeklyLow) / 2;

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyOnly, preDailyClose, preDailyClose1, preWeeklyClose, pBase_Indicators->pWeeklyPredictMaxATR, weekly_baseline);

		if (//pBase_Indicators->weeklyTrend_Phase > 0 &&						
			preDailyClose > daily_baseline
			) // Buy
		{

			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			
			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO:需要调整？
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
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s openOrderCount=%d ",
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
	int   dailyTrend;
	time_t currentTime, preBarTime;
	struct tm timeInfo1, timeInfo2, timeInfoPreBar;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preDailyHigh1, preDailyHigh2, preDailyLow1, preDailyLow2;
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

	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;
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
	int truningPointIndex = -1;

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

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会	
	//计算daily and weekly baseline
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	pIndicators->daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	pIndicators->daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, pIndicators->daily_baseline);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, pIndicators->daily_baseline_short);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
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
		if (strcmp(pParams->accountInfo.brokerName, "International Capital Markets Pty Ltd.") == 0)
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
	else if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
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

	//如果是23H，可以检查是否可以入场
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


	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
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

			pIndicators->stopLossPrice = pIndicators->daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO:需要调整？		

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
				&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex))
			{
				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
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
				&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex))
			{
				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
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
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s openOrderCount=%d ",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

			pIndicators->entrySignal = 0;
		}

		if (isProfitManaged == TRUE)
		{
			if (pIndicators->entrySignal != 0 && openOrderCount == 0)
			{
				pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s openOrderCount=%d 0.5 risk",
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
Weekly Ichimoko baseline 决定年内方向。

2. Entry:

For the first trading signal:
if Trend > 0 && daily close price > daily baseline, buy signal
if Trend < 0 && daily close price < daily baseline, sell signal

If 价格离base line > 1.5 daily ATR, 等待一天。

For the continuous trading signal on daily open:
Option 1:
if close higher: close(1) - close (2) > 25% daily ATR(20) -> buy signal
if close closer: close(1) - close (2) < 25% daily ATR(20) *-1  -> sell signal

Option 2:
根据AutoBBS 的trend phase:
if trend phase is up, buy signal
if trend phase is down, sell signal

Filter:
if price > weekly R2/S2, dont trade
if current weekly ATR > weekly max ATR, dont trade

Option 3: Only it is on a strong trend.
if price retreats back to 4H MA50 Plus 0.15%, buy or sell signal.

2. 日图的移动止损。

If price move against to base line, exit.

SL: it will be max(baseline + 20% ATR(20), max weekly ATR）

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
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double preDailyClose, preDailyClose1;
	double preDailyHigh1, preDailyHigh2, preDailyLow1, preDailyLow2;
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

	double preHist1, preHist2, preHist3, preHist4, preHist5;
	double fast1, fast2, fast3, fast4, fast5;
	double slow1, slow2, slow3, slow4, slow5;
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

	int truningPointIndex = -1;

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

	//如果出现跳空，不能超过 0.2 daily ATR,等待机会	
	//计算daily and weekly baseline
	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 26, &dailyHigh, &dailyLow);
	daily_baseline = (dailyHigh + dailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_DAILY_RATES, shift1Index_Daily, 9, &shortDailyHigh, &shortDailyLow);
	daily_baseline_short = (shortDailyHigh + shortDailyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 26, &weeklyHigh, &weeklyLow);
	weekly_baseline = (weeklyHigh + weeklyLow) / 2;

	iSRLevels(pParams, pBase_Indicators, B_WEEKLY_RATES, shift1Index_Weekly, 9, &shortWeeklyHigh, &shortWeeklyLow);
	weekly_baseline_short = (shortWeeklyHigh + shortWeeklyLow) / 2;

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyHigh =%lf, dailyLow=%lf, daily_baseline=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, dailyHigh, dailyLow, daily_baseline);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, shortDailyHigh =%lf, shortDailyLow=%lf, daily_baseline_short=%lf",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, shortDailyHigh, shortDailyLow, daily_baseline_short);

	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, dailyOnly=%ld,preDailyClose =%lf, preDailyClose1=%lf, preWeeklyClose=%lf,pWeeklyPredictMaxATR =%lf,weekly_baseline=%lf",
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

		orderCount = 1;
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

		pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO:需要调整？
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
			&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex))
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
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
			&& iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, SELL, &truningPointIndex))
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, MACD BeiLi",
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
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s openOrderCount=%d ",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, openOrderCount);

		pIndicators->entrySignal = 0;
	}

	if (isProfitManaged == TRUE)
	{
		if (pIndicators->entrySignal != 0 && openOrderCount == 0)
		{
			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s openOrderCount=%d 0.5 risk",
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
1. Daily Chart
2. Baseline = MA50
3. Take profit: 3% 
4. Internal channel: 1%
 
*/
//AsirikuyReturnCode workoutExecutionTrend_Envelop_Stock(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
//{
//	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
//	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
//	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
//	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
//	int   dailyTrend;
//	time_t currentTime;
//	struct tm timeInfo1, timeInfo2;
//	char   timeString[MAX_TIME_STRING_SIZE] = "";
//	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
//	double preDailyClose, preDailyClose1;
//	double preWeeklyClose;
//	double upboundTP = 0.0, downboundTP = 0.0, upboundInternal = 0.0, downboundInternal = 0.0;
//	double tpEnvelope = 0.03, internalEnvelope = 0.01;
//
//	double targetPNL = 0;
//	double strategyMarketVolRisk = 0.0;
//	double strategyVolRisk = 0.0;
//
//	int openOrderCount = 0;
//
//	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
//	safe_gmtime(&timeInfo1, currentTime);
//	safe_timeString(timeString, currentTime);
//
//	// 满足日图历史的趋势分析。
//
//	if (pBase_Indicators->dailyTrend_Phase > 0)
//		dailyTrend = 1;
//	else if (pBase_Indicators->dailyTrend_Phase < 0)
//		dailyTrend = -1;
//	else
//		dailyTrend = 0;
//
//	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
//
//	pIndicators->splitTradeMode = 26;
//	pIndicators->tpMode = 3;
//	pIndicators->tradeMode = 1;
//
//	if (pIndicators->tradeMode == 1)
//	{
//		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
//		strategyVolRisk = pIndicators->strategyMaxRisk;
//	}
//	else
//	{
//		targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 2;
//		strategyVolRisk = pIndicators->strategyMaxRisk / 3 * 2;
//	}
//
//	preDailyClose = iClose(B_DAILY_RATES, 1);
//	preDailyClose1 = iClose(B_DAILY_RATES, 2);
//	preWeeklyClose = iClose(B_WEEKLY_RATES, 1);
//
//	openOrderCount = getOrderCountEasy();
//
//	//如果出现跳空，不能超过 0.2 daily ATR,等待机会
//	if (timeInfo1.tm_hour >= 1) // 1:00 交易， 避开开盘的一个小时。
//	{
//		
//		//Work out envelope up and down bound
//		upboundTP = pBase_Indicators->maDaily50M * (1 + tpEnvelope);
//		downboundTP = pBase_Indicators->maDaily50M * (1 - tpEnvelope);
//
//		upboundInternal = pBase_Indicators->maDaily50M * (1 + internalEnvelope);
//		downboundInternal = pBase_Indicators->maDaily50M * (1 - internalEnvelope);
//
//		if (preDailyClose > daily_baseline	) // Buy
//		{
//
//			pIndicators->executionTrend = 1;
//			pIndicators->entryPrice = pParams->bidAsk.ask[0];
//
//			pIndicators->stopLossPrice = daily_baseline - pBase_Indicators->dailyATR * 0.25; //TODO:需要调整？
//			pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR);
//
//			//Option 1:
//			if (
//				dailyOnly == 1
//				&& preDailyClose > preDailyClose1
//				&& preDailyClose > daily_baseline_short
//				&& daily_baseline_short > daily_baseline
//				&& preWeeklyClose > weekly_baseline
//				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
//				)
//			{
//				pIndicators->entrySignal = 1;
//			}
//
//			pIndicators->exitSignal = EXIT_SELL;
//
//		}
//
//		if (//pBase_Indicators->weeklyTrend_Phase < 0 &&			
//			preDailyClose < daily_baseline
//			) // Sell
//		{
//
//			pIndicators->executionTrend = -1;
//			pIndicators->entryPrice = pParams->bidAsk.bid[0];
//
//			pIndicators->stopLossPrice = daily_baseline + pBase_Indicators->dailyATR * 0.25;
//			pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pWeeklyPredictMaxATR);
//
//			//Option 1:
//			if (
//				dailyOnly == 1
//				&& preDailyClose < preDailyClose1
//				&& preDailyClose < daily_baseline_short
//				&& daily_baseline_short < daily_baseline
//				&& preWeeklyClose < weekly_baseline
//				//&& isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3) == FALSE
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
//	
//
//	if (pIndicators->entrySignal != 0 && openOrderCount >= 2)
//	{
//		pIndicators->entrySignal = 0;
//	}
//
//	profitManagement_base(pParams, pIndicators, pBase_Indicators);
//
//	return SUCCESS;
//}
/*
Test strategy
*/
AsirikuyReturnCode workoutExecutionTrend_Test(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_Daily = pParams->ratesBuffers->rates[B_DAILY_RATES].info.arraySize - 2;
	int    shift1Index_Weekly = pParams->ratesBuffers->rates[B_WEEKLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, timeInfo2;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。
		
	shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

	pIndicators->splitTradeMode = 26;
	pIndicators->tpMode = 3;
	pIndicators->tradeMode = 1;

	// Test Buy
	
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];

	
	pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->pWeeklyPredictMaxATR;
	pIndicators->entrySignal = 1;
	
	return SUCCESS;
}

/*
半自动的交易系统：
4H的shellington
使用保守的止损计算仓位。
没有加量反手单
*/
AsirikuyReturnCode workoutExecutionTrend_4H_ShellingtonVer1(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1,closeTimeInfo;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	BOOL isOpen;
	OrderType side;
	double openOrderHigh, openOrderLow;

	double preHigh = iHigh(B_PRIMARY_RATES, 1);
	double preLow = iLow(B_PRIMARY_RATES, 1);
	double preClose = iClose(B_PRIMARY_RATES, 1);

	double high_4H = iHigh(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double low_4H = iLow(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	double close_4H = iClose(B_FOURHOURLY_RATES, pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - pIndicators->bbsIndex_4H - 1);
	int trend_4H = 0, trend_KeyK = 0, trend_MA = 0;
	
	double fast1, slow1, preHist1;
	double fast2, slow2, preHist2;
	int orderIndex = -1;
	int execution_tf, close_index_rate = -1, diff4Hours, diffDays,diffWeeks;

	int level = 0;
	BOOL isVolumeControl = TRUE;
	BOOL isEnableBeiLi = TRUE;
	BOOL isEnableSlow = TRUE;
	BOOL isEnableATR = TRUE;
	BOOL isEnableWeeklyATRControl = TRUE;
	BOOL isEnableMACD = TRUE;

	int startHour = 0;

	int truningPointIndex = -1;

	double atr5 = iAtr(B_DAILY_RATES, 5, 1);
	double volume1, volume2;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。
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

	volume1 = iVolume(B_DAILY_RATES, 1);
	volume2 = iVolume(B_DAILY_RATES, 2);

	pIndicators->takePrice = pBase_Indicators->pWeeklyPredictATR / 2;
	pIndicators->takePrice = min(pIndicators->takePrice, pBase_Indicators->dailyATR);

	if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		level = 2; // XAUUSD
	
		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = TRUE;
		isEnableATR = FALSE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		//startHour = 1;
	}
	else if (strstr(pParams->tradeSymbol, "GBPJPY") != NULL)
	{
		level = 0.1; //GBPJPY		

		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;
		isEnableSlow = FALSE;
		isEnableATR = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

	}
	else if (strstr(pParams->tradeSymbol, "GBPAUD") != NULL)
	{
		isEnableMACD = FALSE;
		level = 0.001; 
		isVolumeControl = FALSE;
		isEnableBeiLi = FALSE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;

		isEnableWeeklyATRControl = TRUE;

		fastMAPeriod = 5;
		slowMAPeriod = 10;
		signalMAPeriod = 5;

		//pIndicators->takePrice = 0;
	}
	else if (strstr(pParams->tradeSymbol, "AUDNZD") != NULL)
	{
		level = 0.0025; //GBPJPY

		isVolumeControl = FALSE;
		isEnableBeiLi = TRUE;
		isEnableSlow = FALSE;
		isEnableATR = FALSE;

		fastMAPeriod = 12;
		slowMAPeriod = 26;
		signalMAPeriod = 9;
	}
	else
	{
		level = 0; //EURUSD
	}

	//4H filter	
	if ((timeInfo1.tm_hour - startHour) % 4 == 0 && timeInfo1.tm_min < 3)
	{
		// ATR mode
		pIndicators->splitTradeMode = 27;
		pIndicators->tpMode = 3;

		trend_MA = getMATrend(iAtr(B_FOURHOURLY_RATES, 20, 1), B_FOURHOURLY_RATES, 1);

		movement = fabs(high_4H - low_4H);

		if (pIndicators->atr_euro_range == 0)
			pIndicators->atr_euro_range = pBase_Indicators->pWeeklyPredictATR *0.4;

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, high_4H %lf： low_4H %lf, close_4H=%lf, pWeeklyPredictATR=%lf,pWeeklyPredictMaxATR=%lf,movement=%lf,atr_euro_range=%lf",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, high_4H, low_4H, close_4H, pBase_Indicators->pWeeklyPredictATR, pBase_Indicators->pWeeklyPredictMaxATR, movement, pIndicators->atr_euro_range);


		if (movement >= pIndicators->atr_euro_range) // 100 pips,这应该是根据当时的波幅而定,40% weekly ATR
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
		//满足多重通道当日动态的趋势分析

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			safe_gmtime(&closeTimeInfo, pParams->orderInfo[orderIndex].closeTime);
			 
			//skip weekend, consider cross a new year.

			
				
			diff4Hours = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60* 60 *4);
			diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 24);
			diffWeeks = (timeInfo1.tm_wday + 1 + diffDays) / 7;

			close_index_rate = shift1Index_4H - (diff4Hours - diffWeeks * 2 * 6);

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,diff4Hours=%d,diffDays=%d,diffWeeks=%d,orderIndex=%d,close_index_rate=%d,bbsIndex_excution=%d",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, diff4Hours, diffDays, diffWeeks,orderIndex, close_index_rate, pIndicators->bbsIndex_4H);
		}

		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, &fast1, &slow1, &preHist1);
		iMACDAll(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 2, &fast2, &slow2, &preHist2);

		if (trend_4H == 1)
		{
			if (pIndicators->bbsTrend_4H == 1)
			{
				pIndicators->executionTrend = 1;
				pIndicators->entryPrice = pParams->bidAsk.ask[0];
				
				pIndicators->stopLossPrice = min(pIndicators->bbsStopPrice_4H, pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.5);

				orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& pIndicators->bbsIndex_4H >= close_index_rate
					&& (isEnableMACD == FALSE || (fast1 > level && fast1 > slow1 && fast1 > fast2))
					&& (isEnableSlow == FALSE || slow1 > 0)
					&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
					&& (isVolumeControl == FALSE || volume1 > volume2)
					&& (isEnableBeiLi == FALSE						
					|| iMACDTrendBeiLiEasy(B_DAILY_RATES, fastMAPeriod, slowMAPeriod, signalMAPeriod, 1, 0.0, BUY, &truningPointIndex) == FALSE)
					)
				{

					pIndicators->entrySignal = 1;

				}
				

				pIndicators->exitSignal = EXIT_SELL;
			}
			else
			{
				pIndicators->exitSignal = EXIT_BUY;
			}
		}

		if (trend_4H == -1)
		{
			if (pIndicators->bbsTrend_4H == -1)
			{
				pIndicators->executionTrend = -1;
				pIndicators->entryPrice = pParams->bidAsk.bid[0];
				
				pIndicators->stopLossPrice = max(pIndicators->bbsStopPrice_4H, pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.5);


				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))					
					&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& pIndicators->bbsIndex_4H >= close_index_rate
					&& (isEnableMACD == FALSE || (fast1 < (-1 * level) && fast1 < slow1 && fast1 < fast2))					
					&& (isEnableSlow == FALSE || slow1 < 0)
					&& (isEnableATR == FALSE || atr5 > pIndicators->entryPrice * 0.01 * 0.55)
					&& (isVolumeControl == FALSE || volume1 > volume2)
					&& (isEnableBeiLi == FALSE
					|| iMACDTrendBeiLiEasy(B_DAILY_RATES, 5, 10, 5, 1, 0.0, SELL, &truningPointIndex) == FALSE)
					)
				{
					pIndicators->entrySignal = -1;					
				}

				pIndicators->exitSignal = EXIT_BUY;
			}
			else
				pIndicators->exitSignal = EXIT_SELL;
		}
		
	}
	return SUCCESS;
}

/*
以4H的均线为入场，出场
移动止损
没有止盈
*/
AsirikuyReturnCode workoutExecutionTrend_4H_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double movement = 0;
	int    shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int    shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	int    shift1Index_4H = pParams->ratesBuffers->rates[B_FOURHOURLY_RATES].info.arraySize - 2;
	int   dailyTrend;
	time_t currentTime;
	struct tm timeInfo1, closeTimeInfo;
	char   timeString[MAX_TIME_STRING_SIZE] = "";
	BOOL isOpen;
	OrderType side;
	double openOrderHigh, openOrderLow;

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
	double volume1, volume2;

	int fastMAPeriod = 12, slowMAPeriod = 26, signalMAPeriod = 9;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// 满足日图历史的趋势分析。
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

	if (strstr(pParams->tradeSymbol, "BTCUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 3;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 5;

		pIndicators->riskCap = 2;
	}
	else if (strstr(pParams->tradeSymbol, "XAUUSD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;
	}
	else if (strstr(pParams->tradeSymbol, "XAUAUD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 3;
		sellWonTimes = 1;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 4;

		//pIndicators->riskCap = 2;
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
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		//pIndicators->takePrice = 0;
		isEnableWeeklyTrend = TRUE;
		buyWonTimes = 1;
		sellWonTimes = 1;
	}
	else if (strstr(pParams->tradeSymbol, "US500USD") != NULL)
	{
		isEnableWeeklyATRControl = TRUE;
		pIndicators->takePrice = pBase_Indicators->dailyATR * 3;
		buyWonTimes = 2;
		sellWonTimes = 1;

		pIndicators->minLotSize = 1;
		pIndicators->isEnableSellMinLotSize = TRUE;
	}


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
		//满足多重通道当日动态的趋势分析

		orderIndex = getLastestOrderIndexEasy(B_PRIMARY_RATES);
		

		if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE)
		{
			safe_gmtime(&closeTimeInfo, pParams->orderInfo[orderIndex].closeTime);

			//skip weekend, consider cross a new year.
			diff4Hours = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 4);
			diffDays = difftime(currentTime, pParams->orderInfo[orderIndex].closeTime) / (60 * 60 * 24);
			diffWeeks = (timeInfo1.tm_wday + 1 + diffDays) / 7;

			close_index_rate = shift1Index_4H - (diff4Hours - diffWeeks * 2 *6);

			pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,diff4Hours=%d,diffDays=%d,diffWeeks=%d,orderIndex=%d,close_index_rate=%d,bbsIndex_excution=%d",
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

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& (pParams->orderInfo[orderIndex].type == SELL || pIndicators->bbsIndex_4H >= close_index_rate)
					&& getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, BUY) < buyWonTimes
					&& (isEnableWeeklyTrend == FALSE || pBase_Indicators->weeklyTrend_Phase != RANGE)
					)
				{

					pIndicators->entrySignal = 1;

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

				pIndicators->stopLossPrice = max(pIndicators->bbsStopPrice_4H, iMA(3, B_FOURHOURLY_RATES, 200, 1) + pBase_Indicators->pDailyATR * 0.5);
				//pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->pDailyATR * 1.5);

				//if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE)
				//{

				//	if (pParams->orderInfo[orderIndex].openPrice - pParams->bidAsk.bid[0] > pIndicators->stopLoss )
				//	{
				//		pIndicators->stopLossPrice = pParams->orderInfo[orderIndex].openPrice;
				//	}

				//}

				if ((orderIndex < 0 || (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == FALSE))
					&& (isEnableWeeklyATRControl == FALSE || fabs(iLow(B_WEEKLY_RATES, 0) - pIndicators->entryPrice) <= pBase_Indicators->pWeeklyPredictATR)
					&& (pParams->orderInfo[orderIndex].type == BUY || pIndicators->bbsIndex_4H >= close_index_rate)
					&& getSameSideWonTradesInCurrentTrendEasy(B_PRIMARY_RATES, SELL) < sellWonTimes
					&& (isEnableWeeklyTrend == FALSE || pBase_Indicators->weeklyTrend_Phase != RANGE)
					)
				{
					pIndicators->entrySignal = -1;					
				}
				
			}
			pIndicators->exitSignal = EXIT_BUY;

		}
		//else
		//	pIndicators->exitSignal = EXIT_SELL;

	}
	return SUCCESS;
}