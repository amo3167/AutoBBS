/**
 * @file WindowsStubs.c
 * @brief Stub implementations of AsirikuyEasyTrade functions for Windows builds
 * 
 * This file provides stub implementations for AsirikuyEasyTrade functions that depend on
 * curl library which is not available in the Windows build environment. These stubs allow
 * TradingStrategies to link successfully but will return error codes indicating the
 * functionality is not implemented.
 * 
 * For production use, build on Linux with full curl support.
 */

#include "Precompiled.h"
#include "AsirikuyDefines.h"

/* Define a custom error code for not implemented functions */
#define NOT_IMPLEMENTED INVALID_STRATEGY

/* Stub implementations returning error codes or safe defaults */

AsirikuyReturnCode initEasyTradeLibrary(StrategyParams* pInputParams) {
    return NOT_IMPLEMENTED;
}

double iRangeAverage(int ratesArrayIndex, int period, int shift) {
    return 0.0;
}

double iAtrSafeShiftZero(int period) {
    return 0.0;
}

double iAtrSafeShiftZeroWholeDays(int period) {
    return 0.0;
}

double iAtrDailyByHourInterval(int period, int firstHour, int lastHour) {
    return 0.0;
}

int findShift(int finalArrayIndex, int originalArrayIndex, int shift) {
    return 0;
}

double cOpen(int shift) {
    return 0.0;
}

double high(int shift) {
    return 0.0;
}

double low(int shift) {
    return 0.0;
}

double volume(int shift) {
    return 0.0;
}

double cClose(int shift) {
    return 0.0;
}

double iClose(int rateIndex, int shift) {
    return 0.0;
}

double iVolume(int rateIndex, int shift) {
    return 0.0;
}

double iOpen(int rateIndex, int shift) {
    return 0.0;
}

double iHigh(int rateIndex, int shift) {
    return 0.0;
}

double iLow(int rateIndex, int shift) {
    return 0.0;
}

double range(int shift) {
    return 0.0;
}

double body(int shift) {
    return 0.0;
}

double absBody(int shift) {
    return 0.0;
}

int Period(void) {
    return 0;
}

int Digits(void) {
    return 5;
}

double iAtrWholeDaysSimple(int rateIndex, int periodATR) {
    return 0.0;
}

int getHourShift(int rateIndex, int shift) {
    return 0;
}

AsirikuyReturnCode openSingleShortEasy(double takeProfit, double stopLoss, double lotSize, double risk) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode openSingleLongEasy(double takeProfit, double stopLoss, double lotSize, double risk) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode openSingleBuyStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode openSingleBuyLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize, double risk) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode openSingleSellStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode openSingleSellLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize, double risk) {
    return NOT_IMPLEMENTED;
}

int hour(void) {
    return 0;
}

int minute(void) {
    return 0;
}

int dayOfWeek(void) {
    return 0;
}

int month(void) {
    return 1;
}

int year(void) {
    return 2024;
}

int dayOfYear(void) {
    return 1;
}

int dayOfMonth(void) {
    return 1;
}

double parameter(int parameterIndex) {
    return 0.0;
}

void print(double valueToPrint) {
    /* No-op */
}

AsirikuyReturnCode checkOrders(double takeProfit, double stopLoss) {
    return NOT_IMPLEMENTED;
}

double iAtr(int ratesArrayIndex, int period, int shift) {
    return 0.0;
}

double iRSI(int ratesArrayIndex, int period, int shift) {
    return 0.0;
}

double iMA(int type, int ratesArrayIndex, int period, int shift) {
    return 0.0;
}

double iSTO(int ratesArrayIndex, int period, int k, int d, int signal, int shift) {
    return 0.0;
}

double iMACD(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int signal, int shift) {
    return 0.0;
}

double iMACDAll(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int shift, 
                double *pMacd, double *pMmacdSignal, double *pMacdHist) {
    if (pMacd) *pMacd = 0.0;
    if (pMmacdSignal) *pMmacdSignal = 0.0;
    if (pMacdHist) *pMacdHist = 0.0;
    return 0.0;
}

double iCCI(int ratesArrayIndex, int period, int shift) {
    return 0.0;
}

double spread(void) {
    return 0.0;
}

AsirikuyReturnCode openOrUpdateShortEasy(double takeProfit, double stopLoss, double risk) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode openOrUpdateLongEasy(double takeProfit, double stopLoss, double risk) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode modifyTradeEasy(int orderType, int orderTicket, double stopLoss, double takeProfit) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeLongEasy(int orderTicket) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeShortEasy(int orderTicket) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeAllLongs(void) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeAllShorts(void) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeAllLimitAndStopOrdersEasy(time_t currentTime) {
    return NOT_IMPLEMENTED;
}

int barsCount(int ratesArrayIndex) {
    return 0;
}

int totalOrdersCount(void) {
    return 0;
}

int buyOrdersCount(void) {
    return 0;
}

int sellOrdersCount(void) {
    return 0;
}

int hasOpenOrder(void) {
    return 0;
}

int hasSameDayOrderEasy(time_t currentTime, int *pIsOpen) {
    if (pIsOpen) *pIsOpen = 0;
    return 0;
}

int hasSameWeekOrderEasy(time_t currentTime, int *pIsOpen) {
    if (pIsOpen) *pIsOpen = 0;
    return 0;
}

double getLastestOrderPriceEasy(int rateIndex, int *pIsOpen) {
    if (pIsOpen) *pIsOpen = 0;
    return 0.0;
}

int getLastestOrderIndexEasy(int rateIndex) {
    return -1;
}

int getLastestOrderTypeEasy(int rateIndex) {
    return -1;
}

int getLastestOrderTypeXAUUSDEasy(int rateIndex) {
    return -1;
}

int getOldestOpenOrderIndexEasy(int rateIndex) {
    return -1;
}

int getOrderCountTodayEasy(time_t currentTime) {
    return 0;
}

int getOrderCountForCurrentWeekEasy(time_t currentTime) {
    return 0;
}

int getOrderCountEasy(void) {
    return 0;
}

int getLossTimesInWeekEasy(time_t currentTime, double *total_lost_pips) {
    if (total_lost_pips) *total_lost_pips = 0.0;
    return 0;
}

int getWinTimesInWeekEasy(time_t currentTime) {
    return 0;
}

int getLossTimesInDayEasy(time_t currentTime, double *total_lost_pips) {
    if (total_lost_pips) *total_lost_pips = 0.0;
    return 0;
}

int getLossTimesInDayCloseOrderEasy(time_t currentTime, double *total_lost_pips) {
    if (total_lost_pips) *total_lost_pips = 0.0;
    return 0;
}

int getLossTimesFromNowsEasy(time_t currentTime, double *pTotal_Lost_Pips) {
    if (pTotal_Lost_Pips) *pTotal_Lost_Pips = 0.0;
    return 0;
}

int getWinTimesInDaywithSamePriceEasy(time_t currentTime, double openPrice, double limit) {
    return 0;
}

double caculateFreeMarginEasy(void) {
    return 0.0;
}

double isSameDaySamePricePendingOrderEasy(double entryPrice, double limit, time_t currentTime) {
    return 0.0;
}

double isSamePriceBuyStopOrderEasy(double entryPrice, time_t currentTime, double gap) {
    return 0.0;
}

double isSamePriceSellStopOrderEasy(double entryPrice, time_t currentTime, double gap) {
    return 0.0;
}

int iMACDTrendBeiLiEasy(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, 
                        int startShift, double macdLimit, int orderType, int *pTruningPointIndex, 
                        double *pTurningPoint, int *pMinPointIndex, double *pMinPoint) {
    if (pTruningPointIndex) *pTruningPointIndex = 0;
    if (pTurningPoint) *pTurningPoint = 0.0;
    if (pMinPointIndex) *pMinPointIndex = 0;
    if (pMinPoint) *pMinPoint = 0.0;
    return 0;
}

double iASIEasy(int ratesArrayIndex, int mode, int length, int smooth, double *outBull, double *outBear) {
    if (outBull) *outBull = 0.0;
    if (outBear) *outBear = 0.0;
    return 0.0;
}

int getSameSideWonTradesInCurrentTrendEasy(int rateIndex, int type) {
    return 0;
}

AsirikuyReturnCode iBBands(int ratesArrayIndex, int period, double deviation, int shift, 
                           double *pUpper, double *pMiddle, double *pLower) {
    if (pUpper) *pUpper = 0.0;
    if (pMiddle) *pMiddle = 0.0;
    if (pLower) *pLower = 0.0;
    return NOT_IMPLEMENTED;
}

/* Additional stub implementations for missing functions */

AsirikuyReturnCode saveTickData(void) {
    return NOT_IMPLEMENTED;
}

time_t openTime(int shift) {
    return 0;
}

void addValueToUI(const char* label, double value) {
    /* No-op */
}

AsirikuyReturnCode iPivot(int ratesArrayIndex, int shift, double *pPivot, double *pS1, 
                          double *pR1, double *pS2, double *pR2, double *pS3, double *pR3) {
    if (pPivot) *pPivot = 0.0;
    if (pS1) *pS1 = 0.0;
    if (pR1) *pR1 = 0.0;
    if (pS2) *pS2 = 0.0;
    if (pR2) *pR2 = 0.0;
    if (pS3) *pS3 = 0.0;
    if (pR3) *pR3 = 0.0;
    return NOT_IMPLEMENTED;
}

double iBBandStop(int ratesArrayIndex, int period, double deviation, int shift) {
    return 0.0;
}

AsirikuyReturnCode validateHourlyBarsEasy(StrategyParams* pParams, int primary_rate, int hourly_rate) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode validateDailyBarsEasy(StrategyParams* pParams, int primary_rate, int daily_rate) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode validateSecondaryBarsEasy(StrategyParams* pParams, int primary_rate, int daily_rate, 
                                             int secondary_tf, int rateErrorTimes) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode validateCurrentTimeEasy(StrategyParams* pParams, int primary_rate) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode modifyTradeEasy_new(int orderType, int orderTicket, double stopLoss, double takeProfit, 
                                       int tpMode, int stopMovingbackSL) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode modifyTradeEasy_DayTrading(int orderType, int orderTicket, double stopLoss1, 
                                              double stopLoss2, double takeProfit, int tpMode, 
                                              time_t currentTime, double adjust, int stopMovingbackSL) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeAllBuyLimitOrdersEasy(time_t currentTime) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeAllSellLimitOrdersEasy(time_t currentTime) {
    return NOT_IMPLEMENTED;
}

double isSamePricePendingOrderEasy(double entryPrice, double limit) {
    return 0.0;
}

AsirikuyReturnCode getHighLowEasy(int ratesArrayIndex, int shfitIndex, int shift, 
                                  double *pHigh, double *pLow) {
    if (pHigh) *pHigh = 0.0;
    if (pLow) *pLow = 0.0;
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeWinningPositionsEasy(double total, double target) {
    return NOT_IMPLEMENTED;
}

AsirikuyReturnCode closeAllCurrentDayShortTermOrdersEasy(int tradeMode, time_t currentTime) {
    return NOT_IMPLEMENTED;
}

int getSamePricePendingNoTPOrdersEasy(double entryPrice, double limit) {
    return 0;
}

double caculateStrategyRiskEasy(int ignoreLockedProfit) {
    return 0.0;
}

double caculateStrategyPNLEasy(int ignoreLockedProfit) {
    return 0.0;
}

double caculateStrategyVolRiskEasy(double dailyATR) {
    return 0.0;
}

double isSameWeekSamePricePendingOrderEasy(double entryPrice, double limit, time_t currentTime) {
    return 0.0;
}

int getWinTimesInDayEasy(time_t currentTime) {
    return 0;
}

AsirikuyReturnCode closeAllWithNegativeEasy(int tradeMode, time_t currentTime, int days) {
    return NOT_IMPLEMENTED;
}

int getLastestOpenOrderTypeEasy(int rateIndex) {
    return -1;
}

double isSamePriceBuyLimitOrderEasy(double entryPrice, time_t currentTime, double gap) {
    return 0.0;
}

double isSamePriceSellLimitOrderEasy(double entryPrice, time_t currentTime, double gap) {
    return 0.0;
}

/* curl functions for Windows stub */
time_t curl_getdate(const char *p, time_t *unused) {
    return 0;
}
