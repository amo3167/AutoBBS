#pragma once

#ifndef TIME_MANAGEMENT_H_
#define TIME_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Checks if trading is allowed for the current time and symbol.
 * Implements time-based trading restrictions for specific symbols.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL DailyTrade_Limit_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TIME_MANAGEMENT_H_ */

