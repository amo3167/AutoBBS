#pragma once

#ifndef MULTIPLE_DAY_HELPERS_H_
#define MULTIPLE_DAY_HELPERS_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if GBPJPY multiple days trading is allowed.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL GBPJPY_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Check if GBPUSD multiple days trading is allowed.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL GBPUSD_MultipleDays_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Check if commodity multiple days trading is allowed (XAUUSD, XAGUSD, etc.).
 * 
 * Enhanced filtering criteria for multiple-day strategies:
 * - Must be within trading hours (startHour to endHour)
 * - Filters out Non-Farm Payroll (NFP) days (first Friday of month)
 * - Filters out non-full trading days
 * - Daily predicted ATR must be >= euro ATR range
 * - Weekly ATR must be within predicted range
 * - Daily ATR must be < max(20, weeklyPredictATR/2)
 * - Daily close gap must be < max(10, weeklyPredictATR/3)
 * 
 * @param pParams Strategy parameters containing rates and settings
 * @param pIndicators Strategy indicators structure to modify
 * @param pBase_Indicators Base indicators structure containing ATR predictions
 * @param shouldFilter If FALSE, skips ATR-based filtering
 * @return TRUE if trading is allowed, FALSE otherwise
 * 
 * @note Used by multiple-day strategies for commodity symbols (XAUUSD, XAGUSD)
 */
BOOL Commodity_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL shouldFilter);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MULTIPLE_DAY_HELPERS_H_ */

