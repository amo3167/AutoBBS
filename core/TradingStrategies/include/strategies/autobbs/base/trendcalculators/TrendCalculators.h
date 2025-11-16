#pragma once

#ifndef TREND_CALCULATORS_H_
#define TREND_CALCULATORS_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Trend Calculators Module
 * 
 * Provides functions for calculating daily and weekly trends,
 * including support/resistance levels and trend phases.
 */

AsirikuyReturnCode workoutDailyTrend(StrategyParams* pParams, Base_Indicators* pIndicators);
AsirikuyReturnCode workoutWeeklyTrend(StrategyParams* pParams, Base_Indicators* pIndicators);

#ifdef __cplusplus
}
#endif

#endif /* TREND_CALCULATORS_H_ */

