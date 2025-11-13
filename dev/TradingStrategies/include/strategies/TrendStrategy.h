#pragma once

#ifndef DAILYSTRATEGY_H_
#define DAILYSTRATEGY_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "ComLib.h"
#include "Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Daily chart order splitting functions
void splitBuyOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_ShortTerm_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_LongTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

// Weekly chart order splitting functions
void splitBuyOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Weekly_Beginning(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Weekly_ShortTerm(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_WeeklyTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);
void splitSellOrders_ATR(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

void splitBuyOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);
void splitSellOrders_4HSwing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

void splitBuyOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);
void splitSellOrders_4HSwing_100P(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

void splitBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int mode, double takePrice_primary, double stopLoss);
void splitSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int mode, double takePrice_primary, double stopLoss);

void splitBuyOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);
void splitSellOrders_MACDDaily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

void splitBuyOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);
void splitSellOrders_MACDWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double atr, double stopLoss);

void splitRangeBuyOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
void splitRangeSellOrders_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

// Daily chart strategy execution functions
AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_KeyK(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);
AsirikuyReturnCode workoutExecutionTrend_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);
AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//MACD continue trade on daily chart
AsirikuyReturnCode workoutExecutionTrend_MACD_Daily(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_MACD_Weekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);


// Weekly chart strategy execution functions
AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Weekly_BBS_BreakOut(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);
AsirikuyReturnCode workoutExecutionTrend_Weekly_Pivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);
AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

//GBPJPY only
AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_4HBBS_Swing_BoDuan(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_Limit(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Limit_BBS(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators* pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Limit_BreakOutOnPivot(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_Ichimoko_Daily_Index(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//4H shellington
AsirikuyReturnCode workoutExecutionTrend_4H_Shellington(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* AUTOBBS_H_ */
