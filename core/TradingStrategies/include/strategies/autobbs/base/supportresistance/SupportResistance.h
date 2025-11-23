#pragma once

#ifndef SUPPORT_RESISTANCE_H_
#define SUPPORT_RESISTANCE_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Support/Resistance Module
 * 
 * Provides functions for calculating support and resistance levels
 * using Technical Analysis Library (TALib) functions.
 */

AsirikuyReturnCode iSRLevels(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shiftIndex, int shift, double *pHigh, double *pLow);
AsirikuyReturnCode iSRLevels_close(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shiftIndex, int shift, double *pHigh, double *pLow);
AsirikuyReturnCode iSRLevels_WithIndex(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shiftIndex, int shift, double *pHigh, double *pLow, int *pHighIndex, int *pLowIndex);

#ifdef __cplusplus
}
#endif

#endif /* SUPPORT_RESISTANCE_H_ */
