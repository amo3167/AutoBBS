/*
 * KeyK Order Splitting Module
 * 
 * Provides order splitting functions for KeyK strategy.
 * KeyK strategy splits orders into three parts with different take profit levels:
 * - First order: 1x take profit, 1/3 risk
 * - Second order: 2x take profit, 1/3 risk
 * - Third order: No take profit (let it run), 1/3 risk
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/trend/misc/KeyKOrderSplitting.h"

// Risk distribution constants
#define RISK_SPLIT_THIRD (1.0 / 3.0)          // Split risk into thirds (33.33% each)
#define TP_MULTIPLIER_FIRST_ORDER 1.0         // First order take profit multiplier
#define TP_MULTIPLIER_SECOND_ORDER 2.0        // Second order take profit multiplier
#define TP_MULTIPLIER_THIRD_ORDER 0.0         // Third order: no take profit (let it run)

/**
 * @brief Splits buy orders for KeyK strategy into three parts.
 * 
 * This function opens three buy orders with different take profit levels:
 * - First order: takePrice_primary, 1/3 of risk
 * - Second order: 2 * takePrice_primary, 1/3 of risk
 * - Third order: No take profit (0), 1/3 of risk
 * 
 * This allows for partial profit taking while letting the third order run
 * for potentially larger gains.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing risk and entry price.
 * @param pBase_Indicators Base indicators (not directly used).
 * @param takePrice_primary Primary take profit for first order.
 * @param stopLoss Stop loss value for all orders.
 */
void splitBuyOrders_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	// First order: 1x take profit, 1/3 risk
	takePrice = takePrice_primary * TP_MULTIPLIER_FIRST_ORDER;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_THIRD);

	// Second order: 2x take profit, 1/3 risk
	takePrice = takePrice_primary * TP_MULTIPLIER_SECOND_ORDER;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_THIRD);

	// Third order: No take profit (let it run), 1/3 risk
	takePrice = TP_MULTIPLIER_THIRD_ORDER;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_THIRD);
}

/**
 * @brief Splits sell orders for KeyK strategy into three parts.
 * 
 * This function opens three sell orders with different take profit levels:
 * - First order: takePrice_primary, 1/3 of risk
 * - Second order: 2 * takePrice_primary, 1/3 of risk
 * - Third order: No take profit (0), 1/3 of risk
 * 
 * This allows for partial profit taking while letting the third order run
 * for potentially larger gains.
 * 
 * @param pParams Strategy parameters.
 * @param pIndicators Strategy indicators containing risk and entry price.
 * @param pBase_Indicators Base indicators (not directly used).
 * @param takePrice_primary Primary take profit for first order.
 * @param stopLoss Stop loss value for all orders.
 */
void splitSellOrders_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	// First order: 1x take profit, 1/3 risk
	takePrice = takePrice_primary * TP_MULTIPLIER_FIRST_ORDER;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_THIRD);

	// Second order: 2x take profit, 1/3 risk
	takePrice = takePrice_primary * TP_MULTIPLIER_SECOND_ORDER;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_THIRD);

	// Third order: No take profit (let it run), 1/3 risk
	takePrice = TP_MULTIPLIER_THIRD_ORDER;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk * RISK_SPLIT_THIRD);
}
