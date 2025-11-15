#pragma once

#ifndef MACD_DAILY_STRATEGY_H_
#define MACD_DAILY_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "ComLib.h"
#include "Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MACD Daily strategy execution (new version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * MACD Daily strategy execution (original version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MACD_DAILY_STRATEGY_H_ */

