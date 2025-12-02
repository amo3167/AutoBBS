#pragma once

#ifndef INDICATOR_LOADERS_H_
#define INDICATOR_LOADERS_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Indicator Loaders Module
 * 
 * Provides functions for loading indicators from different timeframes:
 * - Monthly indicators
 * - Weekly indicators
 * - Daily indicators
 * - Intraday KeyK indicators
 * - Main indicator loader (dispatcher)
 */

AsirikuyReturnCode loadWeeklyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators);

// Internal function for use by BaseCore dispatcher
AsirikuyReturnCode loadIndicators_Internal(StrategyParams* pParams, Base_Indicators* pIndicators);

#ifdef __cplusplus
}
#endif

#endif /* INDICATOR_LOADERS_H_ */

