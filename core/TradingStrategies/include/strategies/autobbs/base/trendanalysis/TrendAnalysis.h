#pragma once

#ifndef TREND_ANALYSIS_H_
#define TREND_ANALYSIS_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Trend Analysis Module
 * 
 * Provides functions for analyzing trends using various methods:
 * - High/Low trend analysis (iTrend_HL)
 * - Moving Average trend analysis (iTrend_MA, getMATrend)
 * - Three Rules trend analysis (iTrend3Rules)
 */

AsirikuyReturnCode iTrend_HL(int ratesArrayIndex, int *trend, int index);
AsirikuyReturnCode iTrend_HL_preDays(int ratesArrayIndex, int *trend, int preDays, int index);

int getMATrend(double iATR, int ratesArrayIndex, int index);
int getMATrendBase(int rateShort, int rateLong, double iATR, int ratesArrayIndex, int index);
int getMATrend_Signal(int ratesArrayIndex);
int getMATrend_SignalBase(int rateShort, int rateLong, int ratesArrayIndex, int maxBars);

AsirikuyReturnCode iTrend_MA(double iATR, int ratesArrayIndex, int *trend);
AsirikuyReturnCode iTrend_MA_WeeklyBar_For4H(double iATR, int *trend);
AsirikuyReturnCode iTrend_MA_DailyBar_For1H(double iATR, int *trend, int index);
int iTrendMA_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int signal);

AsirikuyReturnCode iTrend3Rules(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend, int index);
AsirikuyReturnCode iTrend3Rules_preDays(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend, int preDays, int index);
AsirikuyReturnCode iTrend3Rules_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend);

#ifdef __cplusplus
}
#endif

#endif /* TREND_ANALYSIS_H_ */

