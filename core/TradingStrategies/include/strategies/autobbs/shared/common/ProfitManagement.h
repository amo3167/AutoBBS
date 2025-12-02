#pragma once

#ifndef PROFIT_MANAGEMENT_H_
#define PROFIT_MANAGEMENT_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Profit Management Module
 * 
 * Provides profit management functions for strategy execution.
 * Handles risk control, profit taking, and order closure based on profit targets.
 */

/**
 * Base profit management function.
 * Controls risk by macro factor and validates entry signals based on risk limits.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 */
void profitManagement_base(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Standard profit management function.
 * Handles daily/weekly profit taking and order closure based on support/resistance levels.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 */
void profitManagement(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Weekly profit management function.
 * Handles weekly profit taking and order closure based on weekly support/resistance levels.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 */
void profitManagementWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PROFIT_MANAGEMENT_H_ */

