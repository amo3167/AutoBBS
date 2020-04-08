#pragma once

#ifndef SWINGSTRATEGY_H_
#define SWINGSTRATEGY_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "ComLib.h"
#include "Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void splitBuyOrders_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_Daily_Swing_Fix(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Daily_Swing_Fix(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_Daily_GBPJPY_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Daily_GBPJPY_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

//21: 1M XAUUSD 纯空间策略
void splitBuyOrders_Daily_XAUUSD_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Daily_XAUUSD_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

//22： 跨日交易
void splitBuyOrders_MultiDays_XAUUSD_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_MultiDays_XAUUSD_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);


void splitBuyOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void XAUUSD_BBS_StopLoss_Trend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

// XAUUSD
AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_XAUUSD_Daily_KongJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
//日内交易
//结合时间和空间
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
//纯空间
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//5M MA+ BBand
//跨日
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//GBPJPY
//日内交易
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//跨日
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//日内交易，但是加上时间控制，放弃亚洲盘。是可能放弃了一些日元机会，但是应该更加稳定。
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading_ShiJian(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//只计算周波幅
AsirikuyReturnCode workoutExecutionTrendY_WeeklyATR_Prediction(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//周的三波段交易
AsirikuyReturnCode workoutExecutionTrend_Weekly_Swing_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Weekly_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//空间极限回归交易
AsirikuyReturnCode workoutExecutionTrend_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI_Daily_Regression(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SWINGSTRATEGY_H_ */
