#pragma once

#ifndef MISC_STRATEGIES_H_
#define MISC_STRATEGIES_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * KeyK strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * KongJian strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * DailyOpen strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_DailyOpen(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Pivot strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Auto strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * MIDDLE_RETREAT_PHASE strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * ASI strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_ASI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * 4H Shellington strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_4H_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MISC_STRATEGIES_H_ */

