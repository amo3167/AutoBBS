#pragma once

#ifndef WEEKLY_PIVOT_STRATEGY_H_
#define WEEKLY_PIVOT_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Weekly Pivot strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (kept for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Weekly_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WEEKLY_PIVOT_STRATEGY_H_ */

