#pragma once

#ifndef INDICATOR_MANAGEMENT_H_
#define INDICATOR_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Indicator Management Module
 * 
 * Provides indicator loading and UI value setting functions.
 * Handles loading of technical indicators and updating UI display values.
 */

/**
 * Load and initialize indicators.
 * Calculates ATR, BBS indicators, and initializes strategy parameters.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Set UI display values.
 * Updates the user interface with current indicator and strategy values.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return AsirikuyReturnCode
 */
AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INDICATOR_MANAGEMENT_H_ */

