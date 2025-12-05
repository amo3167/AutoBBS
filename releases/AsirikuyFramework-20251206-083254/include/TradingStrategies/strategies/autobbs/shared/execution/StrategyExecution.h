#pragma once

#ifndef STRATEGY_EXECUTION_H_
#define STRATEGY_EXECUTION_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Strategy Execution Module
 * 
 * Provides strategy execution functions for workoutExecutionTrend.
 * Contains all workoutExecutionTrend_* functions that determine entry/exit signals
 * based on various trading strategies.
 */

/**
 * Main strategy execution dispatcher.
 * Calculates risk metrics and dispatches to specific strategy based on AUTOBBS_TREND_MODE.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

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
 * XAUUSD Day Trading strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Hedge strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

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
 * XAUUSD Daily Swing strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Weekly Pivot strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_WeeklyPivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Beginning Phase strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_BEGINNING_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Middle Phase strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_MIDDLE_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Middle Retreat Phase strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Weekly Auto strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Weekly Retreat strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_WeeklyRetreat(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Manual BBS strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_ManualBBS(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Weekly Beginning Phase strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Weekly_BEGINNING_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Weekly Middle Phase strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

/**
 * Weekly Middle Retreat Phase strategy execution.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param ignored Ignored parameter (for compatibility)
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STRATEGY_EXECUTION_H_ */

