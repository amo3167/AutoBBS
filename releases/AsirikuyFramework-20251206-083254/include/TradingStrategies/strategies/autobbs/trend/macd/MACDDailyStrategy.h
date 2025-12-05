#pragma once

#ifndef MACD_DAILY_STRATEGY_H_
#define MACD_DAILY_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MACD Daily strategy execution.
 * 
 * Implements a comprehensive MACD-based daily trading strategy with extensive
 * symbol-specific configurations and advanced features. Supports BTCUSD, ETHUSD,
 * SpotCrudeUSD, XAGUSD, and all major forex pairs.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 * 
 * @note This is the primary MACD Daily strategy implementation (AUTOBBS_TREND_MODE=23).
 *       Required for BTCUSD, ETHUSD, SpotCrudeUSD, and XAGUSD trading.
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MACD_DAILY_STRATEGY_H_ */

