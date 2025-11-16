#pragma once

#ifndef DAY_TRADING_HELPERS_H_
#define DAY_TRADING_HELPERS_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if XAUUSD day trading is allowed.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL XAUUSD_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Check if XAUUSD day trading is allowed (Version 2).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param shouldFilter Whether to apply filtering
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL XAUUSD_DayTrading_Allow_Trade_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL shouldFilter);

/**
 * Check if XAUUSD day trading is allowed (Version 4).
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL XAUUSD_DayTrading_Allow_Trade_Ver4(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * Check if GBPJPY day trading is allowed.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @return TRUE if trading is allowed, FALSE otherwise
 */
BOOL GBPJPY_DayTrading_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

/**
 * XAUUSD day trading entry logic.
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators
 * @param pBase_Indicators Base indicators
 * @param orderType Order type (BUY or SELL)
 * @param ATR0_EURO Euro session ATR
 * @param stopLoss Stop loss value
 * @param Range Range value
 */
void XAUUSD_DayTrading_Entry(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double ATR0_EURO, double stopLoss, double Range);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DAY_TRADING_HELPERS_H_ */

