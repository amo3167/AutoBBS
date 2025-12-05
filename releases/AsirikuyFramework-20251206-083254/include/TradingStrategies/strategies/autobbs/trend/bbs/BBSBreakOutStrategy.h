#pragma once

#ifndef BBS_BREAKOUT_STRATEGY_H_
#define BBS_BREAKOUT_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * BBS BreakOut strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (kept for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Weekly BBS BreakOut strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (kept for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Weekly_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BBS_BREAKOUT_STRATEGY_H_ */

