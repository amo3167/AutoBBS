#pragma once

#ifndef MULTIPLE_DAY_STRATEGY_H_
#define MULTIPLE_DAY_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GBPJPY MultipleDay strategy execution (old version).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_MultipleDay_old(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * GBPJPY MultipleDay strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * MultipleDay strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode workoutExecutionTrend_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * MultipleDay strategy execution (Version 2).
 * Supports multiple symbols (XAUUSD, XAGUSD, BTCUSD, GBPJPY, GBPUSD).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode workoutExecutionTrend_MultipleDay_V2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MULTIPLE_DAY_STRATEGY_H_ */

