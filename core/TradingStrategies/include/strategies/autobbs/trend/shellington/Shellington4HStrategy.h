#pragma once

#ifndef SHELLINGTON_4H_STRATEGY_H_
#define SHELLINGTON_4H_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 4H Shellington strategy execution.
 * 
 * Implements a 4-hour timeframe swing trading strategy using Shellington indicator
 * (4H MA trend + BBS) for entry signals. This strategy uses moving averages on the
 * 4-hour timeframe combined with BBS (Bollinger Bands System) to determine trend
 * direction and entry points.
 * 
 * Features:
 * - Symbol-specific configurations (take price, win times, range settings)
 * - Weekly ATR control for risk management
 * - Weekly trend filtering
 * - Range-based entry filtering
 * - Support for multiple symbols: BTCUSD, ETHUSD, XAUUSD, XAUAUD, GBPJPY, GBPAUD,
 *   AUDUSD, AUDNZD, US500USD, NAS100USD, USTECUSD, XPDUSD, XAGUSD
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 * 
 * @note This strategy corresponds to AUTOBBS_TREND_MODE=30.
 *       Uses split trade mode 27 (ATR mode) and TP mode 3 (daily ATR).
 */
AsirikuyReturnCode workoutExecutionTrend_4H_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SHELLINGTON_4H_STRATEGY_H_ */

