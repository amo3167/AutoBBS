/*
 * Weekly Order Splitting Module
 * 
 * Provides order splitting functions for Weekly strategies.
 * These functions handle order entry and splitting based on weekly pivot levels
 * (R1, R2, S1, S2) for different trading phases.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/weekly/WeeklyOrderSplitting.h"

/* Risk allocation constants */
#define RISK_DIVISOR_2 2.0  /* Divide risk by 2 */
#define RISK_DIVISOR_3 3.0  /* Divide risk by 3 */
#define TP_MULTIPLIER_2 2.0 /* Multiply take price by 2 */
#define TP_MULTIPLIER_3 3.0 /* Multiply take price by 3 */

/**
 * Split buy orders for weekly beginning phase strategy.
 * 
 * Used during BEGINNING_UP_PHASE or BEGINNING_DOWN_PHASE.
 * Entry condition: Entry price must be <= weekly R1 (resistance level 1).
 * Order: Single order with no take profit (takePrice = 0).
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing weekly R1 level
 * @param takePrice_primary Primary take profit target (unused, set to 0)
 * @param stopLoss Stop loss distance
 */
void splitBuyOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	
	/* Entry condition: Price must be at or below weekly R1 */
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		/* No take profit - let position run */
		takePrice = 0.0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/**
 * Split sell orders for weekly beginning phase strategy.
 * 
 * Used during BEGINNING_UP_PHASE or BEGINNING_DOWN_PHASE.
 * Entry condition: Entry price must be >= weekly S1 (support level 1).
 * Order: Single order with no take profit (takePrice = 0).
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing weekly S1 level
 * @param takePrice_primary Primary take profit target (unused, set to 0)
 * @param stopLoss Stop loss distance
 */
void splitSellOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	
	/* Entry condition: Price must be at or above weekly S1 */
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		/* No take profit - let position run */
		takePrice = 0.0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/**
 * Split buy orders for weekly short-term strategy.
 * 
 * Used during MIDDLE_UP_PHASE or MIDDLE_DOWN_PHASE.
 * Entry condition: Entry price must be <= weekly R2 (resistance level 2).
 * Order splitting:
 * - First order: takePrice_primary, risk/2
 * - Second order: 3x takePrice_primary (if TP_MODE enabled) or 0, risk/2
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing weekly R2 level
 * @param takePrice_primary Primary take profit target
 * @param stopLoss Stop loss distance
 */
void splitBuyOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	/* Entry condition: Price must be at or below weekly R2 */
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR2)
	{
		/* First order: Primary take profit, half risk */
		takePrice = takePrice_primary;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_DIVISOR_2);

		/* Second order: 3x take profit (if TP_MODE enabled) or no TP, half risk */
		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
			takePrice = TP_MULTIPLIER_3 * takePrice_primary;
		else
			takePrice = 0.0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_DIVISOR_2);
	}
}

/**
 * Split sell orders for weekly short-term strategy.
 * 
 * Used during MIDDLE_UP_PHASE or MIDDLE_DOWN_PHASE.
 * Entry condition: Entry price must be >= weekly S2 (support level 2).
 * Order splitting:
 * - First order: takePrice_primary, risk/3
 * - Second order: 2x takePrice_primary, risk/3
 * - Third order: 3x takePrice_primary (if TP_MODE enabled) or 0, risk/3
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price and risk
 * @param pBase_Indicators Base indicators structure containing weekly S2 level
 * @param takePrice_primary Primary take profit target
 * @param stopLoss Stop loss distance
 */
void splitSellOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	
	/* Entry condition: Price must be at or above weekly S2 */
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS2)
	{
		/* First order: Primary take profit, one-third risk */
		takePrice = takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_DIVISOR_3);

		/* Second order: 2x take profit, one-third risk */
		takePrice = TP_MULTIPLIER_2 * takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_DIVISOR_3);

		/* Third order: 3x take profit (if TP_MODE enabled) or no TP, one-third risk */
		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
			takePrice = TP_MULTIPLIER_3 * takePrice_primary;
		else
			takePrice = 0.0;

		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / RISK_DIVISOR_3);
	}
}

/**
 * Split buy orders for weekly trading strategy.
 * 
 * General weekly trading strategy for buy orders.
 * Entry condition: Entry price must be <= weekly R1 (resistance level 1).
 * Take profit: Distance to weekly R2 minus adjustment.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price, risk, and adjust
 * @param pBase_Indicators Base indicators structure containing weekly R1 and R2 levels
 * @param takePrice_primary Primary take profit target (unused, calculated internally)
 * @param stopLoss Stop loss distance
 */
void splitBuyOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	
	/* Entry condition: Price must be at or below weekly R1 */
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		/* Take profit: Distance from entry to weekly R2, adjusted */
		takePrice = fabs(pBase_Indicators->weeklyR2 - pIndicators->adjust - pIndicators->entryPrice);
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

/**
 * Split sell orders for weekly trading strategy.
 * 
 * General weekly trading strategy for sell orders.
 * Entry condition: Entry price must be >= weekly S1 (support level 1).
 * Take profit: Distance from entry to weekly S2 plus adjustment.
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure containing entry price, risk, and adjust
 * @param pBase_Indicators Base indicators structure containing weekly S1 and S2 levels
 * @param takePrice_primary Primary take profit target (unused, calculated internally)
 * @param stopLoss Stop loss distance
 */
void splitSellOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	
	/* Entry condition: Price must be at or above weekly S1 */
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		/* Take profit: Distance from entry to weekly S2, adjusted */
		takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->weeklyS2 + pIndicators->adjust));
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}
