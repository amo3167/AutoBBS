/*
 * KeyK Order Splitting Module
 * 
 * Provides order splitting functions for KeyK strategy.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/misc/KeyKOrderSplitting.h"

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
