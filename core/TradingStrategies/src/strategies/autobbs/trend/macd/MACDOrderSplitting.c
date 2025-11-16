/*
 * MACD Order Splitting Module
 * 
 * Provides order splitting functions for MACD Daily and Weekly strategies.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"

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

