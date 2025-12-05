#pragma once

#ifndef STOPLOSS_MANAGEMENT_H_
#define STOPLOSS_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Moves the stop loss for an existing order based on the take profit distance.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param orderIndex Index of the order to modify
 * @param stopLossLevel Stop loss level multiplier (0.0 to 1.0)
 * @return TRUE if stop loss was moved, FALSE otherwise
 */
BOOL move_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, double stopLossLevel);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STOPLOSS_MANAGEMENT_H_ */

