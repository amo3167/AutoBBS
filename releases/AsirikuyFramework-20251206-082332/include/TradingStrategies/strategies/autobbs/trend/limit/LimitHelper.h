/*
 * LimitHelper.h
 * 
 * Symbol-specific configuration helpers for the Limit trading strategy.
 * 
 * This module provides configuration functions for different currency pairs and assets,
 * encapsulating symbol-specific parameter initialization, risk adjustments, and
 * time-based restrictions.
 * 
 * Each configuration function follows a consistent pattern:
 * - Apply symbol-specific risk adjustments based on market conditions (day of week, etc.)
 * - Set time window parameters (start hour, stop hour, startHourOnLimit)
 * - Configure strategy behavior flags (isCloseOrdersEOD, isEnableRSI, etc.)
 * - Apply parameter overrides from user configuration when available
 */

#ifndef LIMIT_HELPER_H
#define LIMIT_HELPER_H

#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"

/**
 * @brief Configure Limit strategy parameters for AUDUSD.
 * 
 * Applies AUDUSD-specific configuration:
 * - Conditional end-of-day order closing based on maTrend reversal
 * - Configurable stop hour with fallback to 17:00
 * - Disables MACD slow filter
 * - Enables flat trend and too-far filters
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend check)
 * @param orderIndex Current order index (-1 if no open order)
 * @param stopHour Output: configured stop hour
 * @param isEnableMACDSlow Output: MACD slow filter flag (set to FALSE)
 * @param isEnableFlatTrend Output: flat trend filter flag (set to TRUE)
 * @param isEnableTooFar Output: too-far filter flag (set to TRUE)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 */
void configureLimitForAUDUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, int *stopHour, BOOL *isEnableMACDSlow, BOOL *isEnableFlatTrend, BOOL *isEnableTooFar, BOOL *isCloseOrdersEOD);

/**
 * @brief Configure Limit strategy parameters for GBPUSD.
 * 
 * Applies GBPUSD-specific configuration:
 * - Reduces risk to 0.6 on Wednesdays (typically higher volatility)
 * - Configurable start hour with fallback to 03:00
 * - Always closes orders at end of day
 * - Enables RSI filtering
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (unused but kept for consistency)
 * @param orderIndex Current order index (unused but kept for consistency)
 * @param timeInfo Current time structure for day-of-week checks
 * @param startHour Output: configured start hour
 * @param isCloseOrdersEOD Output: end-of-day close flag (set to TRUE)
 * @param isEnableRSI Output: RSI filter flag (set to TRUE)
 */
void configureLimitForGBPUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableRSI);

/**
 * @brief Configure Limit strategy parameters for GBPJPY.
 * 
 * Applies GBPJPY-specific configuration:
 * - Sets start hour to 03:00 (European market open)
 * - Conditional end-of-day order closing on maTrend reversal
 * - Enables flat trend filter (avoid ranging markets)
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend check)
 * @param orderIndex Current order index (-1 if no open order)
 * @param startHour Output: configured start hour (set to 3)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableFlatTrend Output: flat trend filter flag (set to TRUE)
 */
void configureLimitForGBPJPY(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableFlatTrend);

/**
 * @brief Configure Limit strategy parameters for USDJPY.
 * 
 * Applies USDJPY-specific configuration:
 * - Conditional end-of-day order closing on maTrend reversal
 * - Disables MACD slow filter for more responsive entries
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend check)
 * @param orderIndex Current order index (-1 if no open order)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableMACDSlow Output: MACD slow filter flag (set to FALSE)
 */
void configureLimitForUSDJPY(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, BOOL *isCloseOrdersEOD, BOOL *isEnableMACDSlow);

/**
 * @brief Configure Limit strategy parameters for XAUUSD (Gold).
 * 
 * Applies XAUUSD-specific configuration:
 * - Risk reduction: 0.6 on Wednesday, 0.5 on Thursday (high volatility days)
 * - Time windows: start 03:00, stop 22:00, limit orders start 08:00
 * - Date filtering: Non-farm payroll (first Friday), Christmas Eve, New Year's Eve
 * - Enables Shellington trend filter and double entry modes
 * - Enables SR1 limit orders
 * - Disables too-far filter
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend check)
 * @param orderIndex Current order index (-1 if no open order)
 * @param timeInfo Current time structure for date/day checks
 * @param timeString Time string for logging
 * @param startHour Output: configured start hour (set to 3)
 * @param stopHour Output: configured stop hour (set to 22)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableShellingtonTrend Output: Shellington trend filter (set to TRUE)
 * @param isEnableTooFar Output: too-far filter flag (set to FALSE)
 * @param isEnableDoubleEntry Output: double entry mode flag (set to TRUE)
 * @param isEnableDoubleEntry2 Output: double entry mode 2 flag (set to TRUE)
 * @param shouldReturn Output: set to TRUE if date filtered and execution should stop
 * @return SUCCESS (check shouldReturn flag for early return indication)
 */
AsirikuyReturnCode configureLimitForXAUUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, const char* timeString, int *startHour, int *stopHour,
    BOOL *isCloseOrdersEOD, BOOL *isEnableShellingtonTrend, BOOL *isEnableTooFar,
    BOOL *isEnableDoubleEntry, BOOL *isEnableDoubleEntry2, BOOL *shouldReturn);

/**
 * @brief Configure Limit strategy parameters for EURGBP.
 * 
 * Applies EURGBP-specific configuration:
 * - Sets start hour to 03:00 (European market open)
 * - Conditional end-of-day order closing on maTrend reversal
 * - Disables MACD slow filter for more responsive entries
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend check)
 * @param orderIndex Current order index (-1 if no open order)
 * @param startHour Output: configured start hour (set to 3)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableMACDSlow Output: MACD slow filter flag (set to FALSE)
 */
void configureLimitForEURGBP(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableMACDSlow);

/**
 * @brief Configure Limit strategy parameters for EURUSD.
 * 
 * Applies EURUSD-specific configuration:
 * - Sets start hour to 03:00 (European market open)
 * - Conditional end-of-day order closing on maTrend reversal
 * - Disables MACD slow filter for more responsive entries
 * - Enables flat trend filter (avoid ranging markets)
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend check)
 * @param orderIndex Current order index (-1 if no open order)
 * @param startHour Output: configured start hour (set to 3)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableMACDSlow Output: MACD slow filter flag (set to FALSE)
 * @param isEnableFlatTrend Output: flat trend filter flag (set to TRUE)
 */
void configureLimitForEURUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableMACDSlow, BOOL *isEnableFlatTrend);

/**
 * @brief Configure Limit strategy parameters for BTCUSD (Bitcoin).
 * 
 * Applies BTCUSD-specific configuration:
 * - Adjust spread: 1% of daily ATR (crypto-specific)
 * - Risk reduction: 0.5 on Tue-Thu (weekday volatility), 0.5 on Sun/Mon if strong trend + low ATR
 * - Time windows: start 00:00 (24/7 crypto markets)
 * - Date filtering: Christmas Eve, New Year's Eve
 * - Custom MACD parameters: 7/14/7 (faster for crypto)
 * - RSI period: 14 days
 * - Too-far limit: 2 (wider range for crypto volatility)
 * - Enables SR1 limit orders, double entry, too-far filter
 * - Disables weekly ATR and range trade
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend and ATR checks)
 * @param orderIndex Current order index (-1 if no open order)
 * @param timeInfo Current time structure for date/day checks
 * @param timeString Time string for logging
 * @param startHour Output: configured start hour (set to 0)
 * @param tooFarLimit Output: too-far limit (set to 2)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableWeeklyATR Output: weekly ATR flag (set to FALSE)
 * @param isEnableRangeTrade Output: range trade flag (set to FALSE)
 * @param isEnableDoubleEntry Output: double entry mode flag (set to TRUE)
 * @param isEnableTooFar Output: too-far filter flag (set to TRUE)
 * @param fastMAPeriod Output: MACD fast MA period (set to 7)
 * @param slowMAPeriod Output: MACD slow MA period (set to 14)
 * @param signalMAPeriod Output: MACD signal MA period (set to 7)
 * @param tradingDays Output: RSI trading days (set to 14)
 * @param shouldReturn Output: set to TRUE if date filtered and execution should stop
 * @return SUCCESS (check shouldReturn flag for early return indication)
 */
AsirikuyReturnCode configureLimitForBTCUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, const char* timeString, int *startHour, int *stopHour, double *tooFarLimit,
    BOOL *isCloseOrdersEOD, BOOL *isEnableWeeklyATR, BOOL *isEnableRangeTrade,
    BOOL *isEnableDoubleEntry, BOOL *isEnableTooFar, int *fastMAPeriod, int *slowMAPeriod,
    int *signalMAPeriod, int *tradingDays, BOOL *shouldReturn);

/**
 * @brief Configure Limit strategy parameters for ETHUSD (Ethereum).
 * 
 * Applies ETHUSD-specific configuration (similar to BTCUSD with minor differences):
 * - Spread filtering: checks if spread > 1.5x adjust (high spread protection)
 * - Adjust spread: 1% of daily ATR (crypto-specific)
 * - Risk reduction: 0.5 on Tue/Thu (different weekdays than BTC), 0.5 on Sun/Mon if strong trend + low ATR
 * - Time windows: start 00:00 (24/7 crypto markets)
 * - Date filtering: Christmas Eve, New Year's Eve
 * - Custom MACD parameters: 7/14/7 (faster for crypto)
 * - RSI period: 14 days
 * - Too-far limit: 2 (wider range for crypto volatility)
 * - Enables SR1 limit orders, double entry, too-far filter
 * - Disables weekly ATR and range trade
 * 
 * @param pParams Strategy parameters
 * @param pIndicators Strategy indicators to configure
 * @param pBase_Indicators Base indicators (used for maTrend and ATR checks)
 * @param orderIndex Current order index (-1 if no open order)
 * @param timeInfo Current time structure for date/day checks
 * @param timeString Time string for logging
 * @param startHour Output: configured start hour (set to 0)
 * @param tooFarLimit Output: too-far limit (set to 2)
 * @param isCloseOrdersEOD Output: end-of-day close flag (conditional)
 * @param isEnableWeeklyATR Output: weekly ATR flag (set to FALSE)
 * @param isEnableRangeTrade Output: range trade flag (set to FALSE)
 * @param isEnableTooBigSpread Output: too big spread filter flag (conditional)
 * @param isEnableDoubleEntry Output: double entry mode flag (set to TRUE)
 * @param isEnableTooFar Output: too-far filter flag (set to TRUE)
 * @param fastMAPeriod Output: MACD fast MA period (set to 7)
 * @param slowMAPeriod Output: MACD slow MA period (set to 14)
 * @param signalMAPeriod Output: MACD signal MA period (set to 7)
 * @param tradingDays Output: RSI trading days (set to 14)
 * @param shouldReturn Output: set to TRUE if date filtered and execution should stop
 * @return SUCCESS (check shouldReturn flag for early return indication)
 */
AsirikuyReturnCode configureLimitForETHUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, const char* timeString, int *startHour, double *tooFarLimit,
    BOOL *isCloseOrdersEOD, BOOL *isEnableWeeklyATR, BOOL *isEnableRangeTrade, BOOL *isEnableTooBigSpread,
    BOOL *isEnableDoubleEntry, BOOL *isEnableTooFar, int *fastMAPeriod, int *slowMAPeriod,
    int *signalMAPeriod, int *tradingDays, BOOL *shouldReturn);

#endif // LIMIT_HELPER_H
