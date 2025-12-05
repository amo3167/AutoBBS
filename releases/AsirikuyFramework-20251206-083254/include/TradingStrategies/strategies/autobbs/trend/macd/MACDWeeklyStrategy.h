#pragma once

#ifndef MACD_WEEKLY_STRATEGY_H_
#define MACD_WEEKLY_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MACD Weekly strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MACD_WEEKLY_STRATEGY_H_ */

