#pragma once

#ifndef ATR_PREDICTION_H_
#define ATR_PREDICTION_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ATR Prediction Module
 * 
 * Provides functions for predicting Average True Range (ATR)
 * for daily and weekly timeframes, including longer-term predictions.
 */

void predictDailyATR(StrategyParams* pParams, Base_Indicators* pIndicators);
void predictWeeklyATR(StrategyParams* pParams, Base_Indicators* pIndicators);
void predictWeeklyATR_LongerTerm(StrategyParams* pParams, Base_Indicators* pIndicators);

#ifdef __cplusplus
}
#endif

#endif /* ATR_PREDICTION_H_ */

