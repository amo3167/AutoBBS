/*
 * LimitHelper.c
 * 
 * Implementation of symbol-specific configuration helpers for the Limit trading strategy.
 * 
 * This module centralizes the configuration logic for different currency pairs and assets,
 * making the main strategy code cleaner and more maintainable.
 * 
 * Design principles:
 * - Each helper function configures parameters via output pointers
 * - Helpers respect user-configured overrides when available (e.g., pIndicators->startHour)
 * - Constants are clearly documented and grouped by category
 * - Risk adjustments are applied based on day of week and market conditions
 */

#include "Precompiled.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/trend/limit/LimitHelper.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "AsirikuyLogger.h"

/* ============================================================================
 * SYMBOL-SPECIFIC TIME CONSTANTS
 * ============================================================================ */

/** Stop hour for AUDUSD (5:00 PM broker time) */
#define STOP_HOUR_AUDUSD 17

/** Stop hour for BTCUSD (16:00 PM broker time) */
#define STOP_HOUR_BTCUSD 16

/** Start hour for major European/USD pairs (3:00 AM broker time) */
#define START_HOUR_GBPJPY_GBPUSD_EURGBP_EURUSD 3

/* ============================================================================
 * RISK ADJUSTMENT CONSTANTS
 * ============================================================================ */

/** Reduced risk factor for GBPUSD on Wednesday (0.6 = 60% of normal risk) */
#define RISK_REDUCED_GBPUSD_WEDNESDAY 0.6

/** Reduced risk factor for GBPUSD in weak months (0.5 = 50% of normal risk) */
#define RISK_REDUCED_GBPUSD_WEAK_MONTH 0.5

/** Reduced risk factor for GBPUSD on Friday (0.3 = 30% of normal risk) */
#define RISK_REDUCED_GBPUSD_FRIDAY 0.3

/** Reduced risk factor for AUDUSD on Tuesday (0.5 = 50% of normal risk) */
#define RISK_REDUCED_AUDUSD_TUESDAY 0.5

/** Reduced risk factor for AUDUSD in October (0.5 = 50% of normal risk) */
#define RISK_REDUCED_AUDUSD_OCTOBER 0.5

/** Reduced risk factor for BTCUSD on Thursday (0.3 = 30% of normal risk) */
#define RISK_REDUCED_BTCUSD_THURSDAY 0.3

/** Reduced risk factor for BTCUSD on Tuesday (0.5 = 50% of normal risk) */
#define RISK_REDUCED_BTCUSD_TUESDAY 0.5

/** Reduced risk factor for BTCUSD on Saturday (0.5 = 50% of normal risk) */
#define RISK_REDUCED_BTCUSD_SATURDAY 0.5

/* ============================================================================
 * DAY OF WEEK CONSTANTS
 * ============================================================================ */

/** Sunday (tm_wday value from struct tm) */
#define SUNDAY_WDAY 0
/** Monday (tm_wday value from struct tm) */
#define MONDAY_WDAY 1
/** Tuesday (tm_wday value from struct tm) */
#define TUESDAY_WDAY 2
/** Wednesday (tm_wday value from struct tm) */
#define WEDNESDAY_WDAY 3
/** Thursday (tm_wday value from struct tm) */
#define THURSDAY_WDAY 4
/** Friday (tm_wday value from struct tm) */
#define FRIDAY_WDAY 5
/** Saturday (tm_wday value from struct tm) */
#define SATURDAY_WDAY 6

/* ============================================================================
 * MONTH CONSTANTS (for seasonal adjustments)
 * ============================================================================ */

/** January (tm_mon value = 0) */
#define JANUARY_MONTH 0
/** February (tm_mon value = 1) */
#define FEBRUARY_MONTH 1
/** April (tm_mon value = 3, since January = 0) */
#define APRIL_MONTH 3
/** August (tm_mon value = 7) */
#define AUGUST_MONTH 7
/** October (tm_mon value = 9) */
#define OCTOBER_MONTH 9
/** December (tm_mon value = 11) */
#define DECEMBER_MONTH 11

/* ============================================================================
 * XAUUSD-SPECIFIC CONSTANTS
 * ============================================================================ */

/** Stop hour for XAUUSD (10:00 PM broker time) */
#define STOP_HOUR_XAUUSD 22
/** Start hour on limit for XAUUSD (8:00 AM broker time) */
#define START_HOUR_ON_LIMIT_XAUUSD 8
/** Reduced risk factor for XAUUSD on Wednesday */
#define RISK_REDUCED_XAUUSD_WEDNESDAY 0.6
/** Reduced risk factor for XAUUSD on Thursday */
#define RISK_REDUCED_XAUUSD_THURSDAY 0.5
/** Day range for non-farm payroll check (first Friday) */
#define NON_FARM_PAYROLL_DAY_RANGE 7
/** December month (tm_mon == 11) */
#define CHRISTMAS_MONTH 11
/** Christmas Eve day */
#define CHRISTMAS_EVE_DAY 24
/** New Year Eve day */
#define NEW_YEAR_EVE_DAY 31

/* ============================================================================
 * CRYPTO-SPECIFIC CONSTANTS
 * ============================================================================ */

/** Start hour for BTC/ETH (00:00 broker time) */
#define START_HOUR_BTC_ETH 0
/** Hour 04 (4:00 AM broker time - worst performing hour for BTCUSD) */
#define HOUR_04 4
/** Reduced risk factor for BTC/ETH on weekdays (Tue-Thu) */
#define RISK_REDUCED_BTC_ETH_WEEKDAYS 0.5
/** Too far limit for BTC/ETH */
#define TOO_FAR_LIMIT_BTC_ETH 2
/** Daily ATR multiplier for adjust calculation (1%) */
#define DAILY_ATR_MULTIPLIER_FOR_ADJUST 0.01
/** Spread multiplier threshold for too big spread check */
#define SPREAD_MULTIPLIER_THRESHOLD 1.5
/** MACD fast period for BTC/ETH */
#define MACD_FAST_PERIOD_BTC_ETH 7
/** MACD slow period for BTC/ETH */
#define MACD_SLOW_PERIOD_BTC_ETH 14
/** MACD signal period for BTC/ETH */
#define MACD_SIGNAL_BTC_ETH 7
/** RSI trading days for BTC/ETH */
#define RSI_TRADING_DAYS_BTC_ETH 14

/* ============================================================================
 * HELPER FUNCTION IMPLEMENTATIONS
 * ============================================================================ */

void configureLimitForAUDUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, int *stopHour, BOOL *isEnableMACDSlow, BOOL *isEnableFlatTrend, BOOL *isEnableTooFar, BOOL *isCloseOrdersEOD)
{
    /* Close orders at end of day if open order exists and maTrend has reversed against position */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
            || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* ====================
     * DAY-OF-WEEK RISK ADJUSTMENT
     * ==================== */
    
    /* Reduce risk on Tuesday based on comprehensive trade analysis:
     * - Tuesday: -$736.64 profit (worst day, 64.7% win rate)
     * - Friday: $8,977.77 profit (best day, 79.4% win rate)
     * - Monday: $5,322.00 profit (75.0% win rate)
     * - Wednesday: $4,998.00 profit (78.7% win rate)
     * Analysis period: 2018-2025, 270 trades
     * Reduce Tuesday position size to 50% to minimize exposure on weak day */
    if (timeInfo->tm_wday == TUESDAY_WDAY) {
        pIndicators->risk = RISK_REDUCED_AUDUSD_TUESDAY;
        logInfo("AUDUSD: Reducing risk to %.1f%% for Tuesday (worst performing day)",
                RISK_REDUCED_AUDUSD_TUESDAY * 100.0);
    }

    /* ====================
     * MONTHLY RISK ADJUSTMENT
     * ==================== */
    
    /* Reduce risk in October based on comprehensive trade analysis:
     * - October: -$2,045.89 profit (only negative month, 59.3% win rate)
     * - September: $5,610.13 profit (best month, 81.6% win rate)
     * - December: $5,320.75 profit (90.9% win rate)
     * Analysis period: 2018-2025, 270 trades
     * Reduce October position size to 50% to minimize exposure during weak month */
    if (timeInfo->tm_mon == OCTOBER_MONTH) {
        pIndicators->risk = RISK_REDUCED_AUDUSD_OCTOBER;
        logInfo("AUDUSD: Reducing risk to %.1f%% for October (worst performing month)",
                RISK_REDUCED_AUDUSD_OCTOBER * 100.0);
    }

    /* Use configured stopHour if provided (non-zero); otherwise fall back to AUDUSD default (17:00) */
    *stopHour = (pIndicators->stopHour != 0) ? pIndicators->stopHour : STOP_HOUR_AUDUSD;

    /* Configure strategy behavior flags for AUDUSD */
    *isEnableMACDSlow = FALSE;   // Disable MACD slow filter (more responsive)
    *isEnableFlatTrend = TRUE;   // Enable flat trend filter (avoid sideways markets)
    *isEnableTooFar = TRUE;      // Enable too-far filter (avoid chasing price)
    
    /* Note: isEnableRangeTrade is intentionally not modified here - controlled elsewhere */
    
    /* Set limit order start hour to user-configured or default start hour */
    pIndicators->startHourOnLimt = pIndicators->startHour;
}

void configureLimitForGBPUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableRSI)
{
    /* ====================
     * DAY-OF-WEEK RISK ADJUSTMENT
     * ==================== */
    
    /* Reduce risk on Friday based on comprehensive trade analysis:
     * - Friday: -$48.03 profit (only negative day, 61.1% win rate)
     * - Monday: $6,824.66 profit (best day, 69.6% win rate)
     * - Wednesday: $6,736.09 profit (71.4% win rate)
     * - Thursday: $6,516.15 profit (67.0% win rate)
     * Analysis period: 2018-2025, 423 trades
     * Reduce Friday position size to 30% to minimize exposure on weak day */
    // if (timeInfo->tm_wday == FRIDAY_WDAY) {
    //     pIndicators->risk = RISK_REDUCED_GBPUSD_FRIDAY;
    //     logInfo("GBPUSD: Reducing risk to %.1f%% for Friday (worst performing day)",
    //             RISK_REDUCED_GBPUSD_FRIDAY * 100.0);
    // }
    
    /* ====================
     * MONTHLY RISK ADJUSTMENT
     * ==================== */
    
    /* Reduce risk in historically weak months based on comprehensive trade analysis:
     * - December: -$2,310.71 average (worst month)
     * - August: -$893.08 average
     * - April: -$765.97 average
     * Analysis period: 2018-2025, 423 trades
     * Reduce position size to 50% during these months to limit exposure */
    if (timeInfo->tm_mon == DECEMBER_MONTH || 
        timeInfo->tm_mon == AUGUST_MONTH || 
        timeInfo->tm_mon == APRIL_MONTH) {
        pIndicators->risk = RISK_REDUCED_GBPUSD_WEAK_MONTH;
        logInfo("GBPUSD: Reducing risk to %.1f%% for weak month (tm_mon=%d)",
                RISK_REDUCED_GBPUSD_WEAK_MONTH * 100.0, timeInfo->tm_mon);
    }

    /* Use configured startHour if provided (non-zero); otherwise fall back to GBPUSD default (03:00) */
    *startHour = (pIndicators->startHour != 0) ? pIndicators->startHour : START_HOUR_GBPJPY_GBPUSD_EURGBP_EURUSD;

    /* Set limit order start hour to match general start hour */
    pIndicators->startHourOnLimt = *startHour;
    
    /* Always close GBPUSD orders at end of day to avoid overnight risk */
    *isCloseOrdersEOD = TRUE;
    
    /* Enable RSI filtering to avoid extreme overbought/oversold conditions
     * Note: This is experimental and may need manual risk adjustment in extreme RSI conditions */
    *isEnableRSI = TRUE;
}

void configureLimitForGBPJPY(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableFlatTrend)
{
    /* Set European market start hour (03:00 AM broker time) */
    *startHour = START_HOUR_GBPJPY_GBPUSD_EURGBP_EURUSD;
    
    /* Close orders at end of day if open order exists and maTrend has reversed against position
     * This protects against holding positions overnight when trend has changed */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* Enable flat trend filter to avoid trading in ranging/sideways markets */
    *isEnableFlatTrend = TRUE;
    
    /* Set limit order start hour to match general start hour */
    pIndicators->startHourOnLimt = *startHour;
}

void configureLimitForUSDJPY(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, BOOL *isCloseOrdersEOD, BOOL *isEnableMACDSlow)
{
    /* Close orders at end of day if open order exists and maTrend has reversed against position
     * USDJPY is sensitive to Asian session closures, so we protect against overnight trend reversals */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* Disable MACD slow filter for more responsive entries
     * USDJPY tends to have cleaner trends, so we don't need the extra confirmation */
    *isEnableMACDSlow = FALSE;
}

AsirikuyReturnCode configureLimitForXAUUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, const char* timeString, int *startHour, int *stopHour,
    BOOL *isCloseOrdersEOD, BOOL *isEnableShellingtonTrend, BOOL *isEnableTooFar,
    BOOL *isEnableDoubleEntry, BOOL *isEnableDoubleEntry2, BOOL *shouldReturn)
{
    *shouldReturn = FALSE;
    
    /* ====================
     * RISK MANAGEMENT
     * ==================== */
    
    /* Reduce risk on Wednesday (typically high volatility from news events) */
    if (timeInfo->tm_wday == WEDNESDAY_WDAY)
        pIndicators->risk = RISK_REDUCED_XAUUSD_WEDNESDAY;

    /* Further reduce risk on Thursday (rollover effects and end-of-week positioning) */
    if (timeInfo->tm_wday == THURSDAY_WDAY)
        pIndicators->risk = RISK_REDUCED_XAUUSD_THURSDAY;

    /* ====================
     * EOD CLOSE LOGIC
     * ==================== */
    
    /* Close orders at end of day if maTrend has reversed against position
     * Gold is highly sensitive to overnight gaps, so we close on trend reversal */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;
    
    /* ====================
     * TIME WINDOWS
     * ==================== */
    
    *startHour = START_HOUR_GBPJPY_GBPUSD_EURGBP_EURUSD;  // Start: 03:00 (European open)
    pIndicators->startHourOnLimt = START_HOUR_ON_LIMIT_XAUUSD;  // Limit orders: 08:00 (after Asian session)
    *stopHour = STOP_HOUR_XAUUSD;  // Stop: 22:00 (before major overnight moves)

    /* ====================
     * DATE FILTERING
     * ==================== */
    
    /* Filter non-farm payroll day (first Friday of the month)
     * NFP is extremely volatile for gold - avoid trading on this day */
    if (timeInfo->tm_wday == FRIDAY_WDAY && timeInfo->tm_mday - NON_FARM_PAYROLL_DAY_RANGE < 1)
    {
        strcpy(pIndicators->status, "Filter Non-farm day\n");
        logWarning("System InstanceID = %d, BarTime = %s, %s",
            (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
        *shouldReturn = TRUE;
        return SUCCESS;
    }

    /* Filter Christmas Eve and New Year's Eve (low liquidity, erratic moves) */
    if (timeInfo->tm_mon == CHRISTMAS_MONTH && (timeInfo->tm_mday == CHRISTMAS_EVE_DAY || timeInfo->tm_mday == NEW_YEAR_EVE_DAY))
    {
        strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");
        logWarning("System InstanceID = %d, BarTime = %s, %s",
            (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
        *shouldReturn = TRUE;
        return SUCCESS;
    }

    /* ====================
     * STRATEGY FLAGS
     * ==================== */
    
    *isEnableShellingtonTrend = TRUE;  // Use Shellington trend filter for gold's strong directional moves
    pIndicators->isEnableLimitSR1 = TRUE;  // Enable S1/R1 level limit orders
    *isEnableTooFar = FALSE;  // Don't use too-far filter (gold can trend strongly)
    *isEnableDoubleEntry = TRUE;  // Allow double entry on pullbacks
    *isEnableDoubleEntry2 = TRUE;  // Allow second double entry mode
    
    return SUCCESS;
}

void configureLimitForEURGBP(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableMACDSlow)
{
    /* Set European market start hour (03:00 AM broker time) */
    *startHour = START_HOUR_GBPJPY_GBPUSD_EURGBP_EURUSD;

    /* Close orders at end of day if maTrend has reversed against position
     * EURGBP is a cross pair with lower liquidity, protect against overnight reversals */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* Disable MACD slow filter for more responsive entries
     * EURGBP has clean directional moves during European session */
    *isEnableMACDSlow = FALSE;
}

void configureLimitForEURUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, int *startHour, BOOL *isCloseOrdersEOD, BOOL *isEnableMACDSlow, BOOL *isEnableFlatTrend)
{
    /* Set European market start hour (03:00 AM broker time) */
    *startHour = START_HOUR_GBPJPY_GBPUSD_EURGBP_EURUSD;

    /* Close orders at end of day if maTrend has reversed against position
     * EURUSD is highly liquid but sensitive to US session news */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* Disable MACD slow filter for more responsive entries */
    *isEnableMACDSlow = FALSE;
    
    /* Enable flat trend filter to avoid trading in ranging/consolidation periods
     * EURUSD frequently ranges during low-volume periods */
    *isEnableFlatTrend = TRUE;
    
    /* Set limit order start hour to match general start hour */
    pIndicators->startHourOnLimt = *startHour;
}

AsirikuyReturnCode configureLimitForBTCUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, const char* timeString, int *startHour, int *stopHour, double *tooFarLimit,
    BOOL *isCloseOrdersEOD, BOOL *isEnableWeeklyATR, BOOL *isEnableRangeTrade,
    BOOL *isEnableDoubleEntry, BOOL *isEnableTooFar, int *fastMAPeriod, int *slowMAPeriod,
    int *signalMAPeriod, int *tradingDays, BOOL *shouldReturn)
{
    *shouldReturn = FALSE;
    
    /* ====================
     * CRYPTO-SPECIFIC ADJUSTMENTS
     * ==================== */
    
    /* Use 1% of daily ATR for spread adjustment (crypto has wider spreads than forex) */
    pIndicators->adjust = pBase_Indicators->dailyATR * DAILY_ATR_MULTIPLIER_FOR_ADJUST;
    
    /* Configure Asian + London hours trading window (00:00-16:00)
     * Based on comprehensive analysis:
     * - Asian session (00-08): $6,167 profit, 67.7% win rate, $12.16 avg/trade
     * - London session (08-16): $3,106 profit, 72.6% win rate, $50.10 avg/trade (BEST!)
     * - NY session (16-24): -$779 profit, 56.5% win rate (LOSING)
     * 
     * startHour: When to START accepting new trades (00:00)
     * stopHour: When to STOP accepting new trades (16:00 - before NY open)
     * Note: This is entry-only filtering - existing trades continue to their natural exit
     * Analysis period: 2019-2025, 592 trades */
    /* Use configured startHour if provided (non-zero); otherwise fall back to BTCETH default (0:00) */
    *startHour = (pIndicators->startHour != 0) ? pIndicators->startHour : START_HOUR_BTC_ETH;

    /* Use configured stopHour if provided (non-zero); otherwise fall back to BTCUSD default (17:00) */
    *stopHour = (pIndicators->stopHour != 0) ? pIndicators->stopHour : STOP_HOUR_BTCUSD;
    pIndicators->startHourOnLimt = *startHour;

    /* ====================
     * EOD CLOSE LOGIC
     * ==================== */
    
    /* Close orders if maTrend has reversed against position
     * Even though crypto trades 24/7, we respect trend changes */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* ====================
     * RISK MANAGEMENT
     * ==================== */
    
    /* ====================
     * DAY-OF-WEEK FILTERING
     * ==================== */
    
    /* Trade only on Wednesday, Friday, and Saturday based on comprehensive analysis:
     * PROFITABLE DAYS (Trade these):
     * - Wednesday: $6,805.60 profit (75.3% win rate, 84 avg/trade)
     * - Friday: $3,473.11 profit (68.9% win rate)
     * - Saturday: $556.36 profit (66.7% win rate)
     * 
     * LOSING DAYS (Filter out):
     * - Sunday: -$949.30 (63.5% win rate)
     * - Monday: -$830.36 (66.7% win rate)
     * - Tuesday: -$817.98 (64.5% win rate)
     * - Thursday: $256.72 (marginal, 67.9% win rate)
     * 
     * Analysis period: 2019-2025, 592 trades
     * Strategy: Only trade the consistently profitable days */
    if (timeInfo->tm_wday != WEDNESDAY_WDAY && 
        timeInfo->tm_wday != FRIDAY_WDAY && 
        timeInfo->tm_wday != SATURDAY_WDAY)
    {
        strcpy(pIndicators->status, "Filter non-profitable days (only trade Wed/Fri/Sat).\\n");
        logInfo("BTCUSD: Filtering %s (only trading Wednesday, Friday, Saturday)",
                timeInfo->tm_wday == SUNDAY_WDAY ? "Sunday" :
                timeInfo->tm_wday == MONDAY_WDAY ? "Monday" :
                timeInfo->tm_wday == TUESDAY_WDAY ? "Tuesday" : "Thursday");
        *shouldReturn = TRUE;
        return SUCCESS;
    }

    /* ====================
     * TRADING HOURS FILTERING
     * ==================== */
    
    /* Filter hour 04 (4:00 AM) based on comprehensive analysis:
     * Hour 04: -$1,390 profit (worst hour, 58.8% win rate, -$81.79 avg/trade)
     * Analysis shows this hour consistently underperforms across all trading days
     * Analysis period: 2019-2025, 230 trades (17 trades during hour 04)
     * Strategy: Skip trading entirely during hour 04 to avoid this losing window */
    if (timeInfo->tm_hour == HOUR_04)
    {
        strcpy(pIndicators->status, "Filter hour 04 (worst performing hour).\\n");
        logInfo("BTCUSD: Filtering hour 04 (4:00 AM) - worst performing hour (-$1,390)");
        *shouldReturn = TRUE;
        return SUCCESS;
    }
    
    /* ====================
     * DATE FILTERING
     * ==================== */
    
    /* Filter January and February based on comprehensive analysis:
     * January: -$2,572 profit (58.6% win rate, -$88.69 avg/trade, 29 trades)
     * February: -$1,893 profit (52.9% win rate, -$111.33 avg/trade, 17 trades)
     * Combined loss: -$4,465 (only two negative months)
     * Analysis period: 2019-2025, 230 trades
     * Strategy: Avoid trading during these historically weak months */
    if (timeInfo->tm_mon == JANUARY_MONTH || timeInfo->tm_mon == FEBRUARY_MONTH)
    {
        strcpy(pIndicators->status, "Filter January/February (worst performing months).\\n");
        logInfo("BTCUSD: Filtering %s (worst performing month, combined -$4,465 loss)",
                timeInfo->tm_mon == JANUARY_MONTH ? "January" : "February");
        *shouldReturn = TRUE;
        return SUCCESS;
    }
    
    /* Filter Christmas Eve and New Year's Eve (extremely low liquidity even in crypto) */
    if (timeInfo->tm_mon == CHRISTMAS_MONTH && (timeInfo->tm_mday == CHRISTMAS_EVE_DAY || timeInfo->tm_mday == NEW_YEAR_EVE_DAY))
    {
        strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");
        logWarning("System InstanceID = %d, BarTime = %s, %s",
            (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
        *shouldReturn = TRUE;
        return SUCCESS;
    }

    /* ====================
     * STRATEGY CONFIGURATION
     * ==================== */
    
    *isEnableWeeklyATR = FALSE;  // Don't use weekly ATR (crypto moves faster than forex)
    pIndicators->isEnableLimitSR1 = TRUE;  // Enable S1/R1 level limit orders
    *tooFarLimit = TOO_FAR_LIMIT_BTC_ETH;  // Wider too-far limit (crypto is more volatile)
    *isEnableRangeTrade = FALSE;  // Don't trade ranges in crypto (tends to trend or chop violently)
    *isEnableDoubleEntry = FALSE;  // Disable double entry - analysis shows trades >8hrs lose money (-$6,891, 56% win vs <8hrs +$19,795, 80% win)
    *isEnableTooFar = TRUE;  // Enable too-far filter to avoid chasing
    
    /* ====================
     * CRYPTO-SPECIFIC MACD PARAMETERS
     * ==================== */
    
    /* Use faster MACD settings (7/14/7) for crypto's rapid price movements
     * Standard forex settings (5/10/5 or 12/26/9) are too slow for crypto */
    *fastMAPeriod = MACD_FAST_PERIOD_BTC_ETH;
    *slowMAPeriod = MACD_SLOW_PERIOD_BTC_ETH;
    *signalMAPeriod = MACD_SIGNAL_BTC_ETH;
    
    /* Use 14-day RSI period (standard for crypto) */
    *tradingDays = RSI_TRADING_DAYS_BTC_ETH;
    
    return SUCCESS;
}

AsirikuyReturnCode configureLimitForETHUSD(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators,
    int orderIndex, struct tm* timeInfo, const char* timeString, int *startHour, double *tooFarLimit,
    BOOL *isCloseOrdersEOD, BOOL *isEnableWeeklyATR, BOOL *isEnableRangeTrade, BOOL *isEnableTooBigSpread,
    BOOL *isEnableDoubleEntry, BOOL *isEnableTooFar, int *fastMAPeriod, int *slowMAPeriod,
    int *signalMAPeriod, int *tradingDays, BOOL *shouldReturn)
{
    *shouldReturn = FALSE;
    
    /* ====================
     * SPREAD FILTERING (ETH-specific)
     * ==================== */
    
    /* Enable spread filter if current spread exceeds 1.5x normal adjust
     * ETH can have erratic spreads during high volatility - protect against poor fills */
    if (fabs(pParams->bidAsk.ask[0] - pParams->bidAsk.bid[0]) > pIndicators->adjust * SPREAD_MULTIPLIER_THRESHOLD)
    {
        *isEnableTooBigSpread = TRUE;
    }
    
    /* ====================
     * CRYPTO-SPECIFIC ADJUSTMENTS
     * ==================== */
    
    /* Use 1% of daily ATR for spread adjustment (crypto has wider spreads than forex) */
    pIndicators->adjust = pBase_Indicators->dailyATR * DAILY_ATR_MULTIPLIER_FOR_ADJUST;
    
    /* Set 24-hour start time (crypto markets never close) */
    *startHour = START_HOUR_BTC_ETH;
    pIndicators->startHourOnLimt = *startHour;
    
    /* ====================
     * EOD CLOSE LOGIC
     * ==================== */
    
    /* Close orders if maTrend has reversed against position
     * ETH is highly correlated to BTC but can have independent moves */
    if (orderIndex >= 0 && pParams->orderInfo[orderIndex].isOpen == TRUE
        && (pParams->orderInfo[orderIndex].type == BUY && pBase_Indicators->maTrend < 0
        || pParams->orderInfo[orderIndex].type == SELL && pBase_Indicators->maTrend > 0)
        )
        *isCloseOrdersEOD = TRUE;

    /* ====================
     * RISK MANAGEMENT
     * ==================== */
    
    /* Reduce risk on Tue/Thu (different from BTC which uses Tue-Thu)
     * ETH has slightly different volatility patterns than BTC */
    if (timeInfo->tm_wday == TUESDAY_WDAY || timeInfo->tm_wday == THURSDAY_WDAY)
        pIndicators->risk = RISK_REDUCED_BTC_ETH_WEEKDAYS;

    /* Additional risk reduction on Sun/Mon if strong trend with low ATR
     * This indicates potential exhaustion after weekend moves */
    if (timeInfo->tm_wday == SUNDAY_WDAY || timeInfo->tm_wday == 1)
    {
        if (abs(pBase_Indicators->dailyTrend) >= 6
            && iAtr(B_DAILY_RATES, 1, 1) < 0.7 * pBase_Indicators->pDailyATR
            )
            pIndicators->risk = 0.5;
    }

    /* ====================
     * DATE FILTERING
     * ==================== */
    
    /* Filter Christmas Eve and New Year's Eve (extremely low liquidity even in crypto) */
    if (timeInfo->tm_mon == CHRISTMAS_MONTH && (timeInfo->tm_mday == CHRISTMAS_EVE_DAY || timeInfo->tm_mday == NEW_YEAR_EVE_DAY))
    {
        strcpy(pIndicators->status, "Filter Christmas and New Year Eve.\n");
        logWarning("System InstanceID = %d, BarTime = %s, %s",
            (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->status);
        *shouldReturn = TRUE;
        return SUCCESS;
    }

    /* ====================
     * STRATEGY CONFIGURATION
     * ==================== */
    
    *isEnableWeeklyATR = FALSE;  // Don't use weekly ATR (crypto moves faster than forex)
    pIndicators->isEnableLimitSR1 = TRUE;  // Enable S1/R1 level limit orders
    *tooFarLimit = TOO_FAR_LIMIT_BTC_ETH;  // Wider too-far limit (crypto is more volatile)
    *isEnableRangeTrade = FALSE;  // Don't trade ranges in crypto (tends to trend or chop violently)
    *isEnableDoubleEntry = TRUE;  // Allow double entry on pullbacks
    *isEnableTooFar = TRUE;  // Enable too-far filter to avoid chasing
    
    /* ====================
     * CRYPTO-SPECIFIC MACD PARAMETERS
     * ==================== */
    
    /* Use faster MACD settings (7/14/7) for crypto's rapid price movements
     * Standard forex settings (5/10/5 or 12/26/9) are too slow for crypto */
    *fastMAPeriod = MACD_FAST_PERIOD_BTC_ETH;
    *slowMAPeriod = MACD_SLOW_PERIOD_BTC_ETH;
    *signalMAPeriod = MACD_SIGNAL_BTC_ETH;
    
    /* Use 14-day RSI period (standard for crypto) */
    *tradingDays = RSI_TRADING_DAYS_BTC_ETH;
    
    return SUCCESS;
}
