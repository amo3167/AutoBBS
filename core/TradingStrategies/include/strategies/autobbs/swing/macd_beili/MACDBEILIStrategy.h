#pragma once

#ifndef MACD_BEILI_STRATEGY_H_
#define MACD_BEILI_STRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MACD BEILI strategy execution.
 * 
 * MACD BEILI: Mainly used for counter-trend trading during reversal times, with risk 1.
 * 
 * Buy Entry:
 * 1. The highest MACD (10) in the last 5 days, > limit
 * 2. MACD(5) < MACD(10) : cross down
 * 3. Win/Loss >=1
 * 4. Stop loss is the highest price in the last 5 days
 * 5. Take price is just greater than ichimoku(slow)
 * 
 * Exit:
 * 1. MACD cross up
 * 2. Hit the moving stop loss
 * 
 * Short Entry:
 * 1. The lowest MACD (10) in the last 5 days, < limit
 * 2. MACD(5) > MACD(10) : cross up
 * 3. Win/Loss >=1
 * 4. Stop loss is the lowest price in the last 5 days
 * 5. Take price is just less than ichimoku(slow)
 * 
 * Exit:
 * 1. MACD cross down
 * 2. Hit the moving stop loss
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode SUCCESS or error code
 */
AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MACD_BEILI_STRATEGY_H_ */

