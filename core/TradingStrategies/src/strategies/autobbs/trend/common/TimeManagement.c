/*
 * Time Management Module
 * 
 * Provides functions for time-based trading restrictions.
 * This module enforces trading time windows for specific currency pairs
 * to avoid periods of low liquidity or high volatility.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"

// Trading time restrictions
#define CRYPTO_RESTRICTION_HOUR 10              // Hour after which crypto trading is restricted
#define GBPUSD_RESTRICTION_START_HOUR 8          // Start hour for GBPUSD trading restriction
#define GBPUSD_RESTRICTION_END_HOUR 14           // End hour for GBPUSD trading restriction

// Symbol prefixes
#define SYMBOL_BTCUSD "BTCUSD"
#define SYMBOL_ETHUSD "ETHUSD"
#define SYMBOL_GBPUSD "GBPUSD"

/**
 * @brief Checks if trading is allowed based on time restrictions for specific symbols.
 * 
 * This function enforces time-based trading restrictions:
 * - Crypto pairs (BTCUSD, ETHUSD): No trading after 10:00
 * - GBPUSD: No trading between 08:00 and 14:00
 * 
 * If trading is not allowed, all limit and stop orders are closed and FALSE is returned.
 * 
 * @param pParams Strategy parameters containing trade symbol and settings.
 * @param pIndicators Strategy indicators to update with status message.
 * @param pBase_Indicators Base indicators (not directly used).
 * @return TRUE if trading is allowed, FALSE if restricted.
 */
BOOL DailyTrade_Limit_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	time_t currentTime;
	struct tm timeInfo1;
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	char timeString[MAX_TIME_STRING_SIZE] = "";

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	// Crypto pairs: No trading after 10:00
	if (strstr(pParams->tradeSymbol, SYMBOL_BTCUSD) != NULL || strstr(pParams->tradeSymbol, SYMBOL_ETHUSD) != NULL)
	{
		if (timeInfo1.tm_hour >= CRYPTO_RESTRICTION_HOUR)
		{
			sprintf(pIndicators->status, "Ignore trading after 10\n");
			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
			closeAllLimitAndStopOrdersEasy(currentTime);
			return FALSE;
		}
	}
	// GBPUSD: No trading between 08:00 and 14:00
	else if (strstr(pParams->tradeSymbol, SYMBOL_GBPUSD) != NULL)
	{
		if (timeInfo1.tm_hour >= GBPUSD_RESTRICTION_START_HOUR && timeInfo1.tm_hour <= GBPUSD_RESTRICTION_END_HOUR)
		{
			sprintf(pIndicators->status, "Ignore trading between 8-14\n");
			logWarning("System InstanceID = %d, BarTime = %s, %s",
				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);

			closeAllLimitAndStopOrdersEasy(currentTime);
			return FALSE;
		}
	}
	return TRUE;
}
