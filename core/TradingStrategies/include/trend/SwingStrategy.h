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

void splitBuyOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_Daily_Swing(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_ShortTerm_ATR_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

void splitBuyOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);
void splitSellOrders_ShortTerm_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss);

// XAUUSD
AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//Day trading
//Based on time and space
AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
//5M MA+ BBand
//Swing trading
AsirikuyReturnCode workoutExecutionTrend_MultipleDay(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//GBPJPY
//Day trading
AsirikuyReturnCode workoutExecutionTrend_GBPJPY_DayTrading_Ver2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

//Weekly swing trading
AsirikuyReturnCode workoutExecutionTrend_Weekly_Swing_New(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);


AsirikuyReturnCode workoutExecutionTrend_MACD_BEILI(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

AsirikuyReturnCode workoutExecutionTrend_MultipleDay_V2(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SWINGSTRATEGY_H_ */
