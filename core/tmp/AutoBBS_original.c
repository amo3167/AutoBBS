
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "ComLib.h"
#include "SwingStrategy.h"
#include "TrendStrategy.h"
#include "StrategyUserInterface.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSEF

static void splitBuyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode) {
		case 0:
			splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;		
		case 3:
			splitBuyOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 4:
			splitBuyOrders_Limit(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 5:		
			splitBuyOrders_DayTrading(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 6:			
			break;
		case 7:			
			break;		
		case 8:
			splitBuyOrders_WeeklyTrading(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 9:
			splitBuyOrders_KeyK(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 10:
			splitBuyOrders_Weekly_Beginning(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 11:
			splitBuyOrders_Weekly_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 12:
			splitBuyOrders_ATR(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 13:
			splitBuyOrders_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 14:
			splitBuyOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 15:
			splitBuyOrders_ShortTerm_ATR_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 16:
			splitBuyOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 17:
			splitBuyOrders_Daily_GBPJPY_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;	
		case 18:
			splitBuyOrders_Daily_Swing_Fix(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 19:
			splitBuyOrders_4HSwing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 20:
			splitBuyOrders_4HSwing_100P(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 21:
			splitBuyOrders_Daily_XAUUSD_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 22:
			splitBuyOrders_MultiDays_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;			
		case 23:
			splitBuyOrders_MultiDays_GBPJPY_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 24:
			splitBuyOrders_MACDDaily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 25:
			splitBuyOrders_MACDWeekly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 26:
			splitBuyOrders_Ichimoko_Daily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 27:
			splitBuyOrders_4HSwing_Shellington(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 28:
			splitBuyOrders_MACD_BEILI(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 29:
			splitBuyOrders_Daily_Swing_ExecutionOnly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 30:
			splitBuyOrders_MultiDays_Swing_V2(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 31:
			splitBuyOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;
		case 32:
			splitBuyRangeOrders(pParams, pIndicators, pBase_Indicators);
			break;
		case 33:
			splitBuyOrders_Ichimoko_Weekly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
			break;

	}
	
}

static void splitSellOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode) {
	case 0:
		splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 3:
		splitSellOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 4:
		splitSellOrders_Limit(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 5:	
		splitSellOrders_DayTrading(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;	
	case 8:
		splitSellOrders_WeeklyTrading(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 9:
		splitSellOrders_KeyK(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 10:
		splitSellOrders_Weekly_Beginning(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 11:
		splitSellOrders_Weekly_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 12:
		splitSellOrders_ATR(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 13:
		splitSellOrders_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 14:
		splitSellOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 15:
		splitSellOrders_ShortTerm_ATR_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 16:
		splitSellOrders_Daily_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 17:
		splitSellOrders_Daily_GBPJPY_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;	
	case 18:
		splitSellOrders_Daily_Swing_Fix(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 19:
		splitSellOrders_4HSwing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 20:
		splitSellOrders_4HSwing_100P(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 21:
		splitSellOrders_Daily_XAUUSD_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 22:
		splitSellOrders_MultiDays_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 23:
		splitSellOrders_MultiDays_GBPJPY_Swing(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 24:
		splitSellOrders_MACDDaily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 25:
		splitSellOrders_MACDWeekly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 26:
		splitSellOrders_Ichimoko_Daily(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 27:
		splitSellOrders_4HSwing_Shellington(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 28:
		splitSellOrders_MACD_BEILI(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 29:
		splitSellOrders_Daily_Swing_ExecutionOnly(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 30:
		splitSellOrders_MultiDays_Swing_V2(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 31:
		splitSellOrders_ShortTerm_New(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 32:
		splitSellRangeOrders(pParams, pIndicators, pBase_Indicators);
		break;
	}
}

static AsirikuyReturnCode workoutExecutionTrend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	struct tm timeInfo;
	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int   startHour = 0;

	if (strstr(pParams->tradeSymbol, "XAU") != NULL || strstr(pParams->tradeSymbol, "XTI") != NULL || strstr(pParams->tradeSymbol, "SpotCrude") != NULL || strstr(pParams->tradeSymbol, "XAG") != NULL || strstr(pParams->tradeSymbol, "XPD") != NULL)
		startHour = 1;

	safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	//Strategy risk
	pIndicators->strategyMarketVolRisk = caculateStrategyVolRiskEasy(pBase_Indicators->dailyATR);

	pIndicators->strategyRisk = caculateStrategyRiskEasy(FALSE);
	pIndicators->strategyRiskWithoutLockedProfit = caculateStrategyRiskEasy(TRUE);

	pIndicators->riskPNL = caculateStrategyPNLEasy(FALSE);
	pIndicators->riskPNLWithoutLockedProfit = caculateStrategyPNLEasy(TRUE);

	switch ((int)parameter(AUTOBBS_TREND_MODE)) {
	case 0:
		workoutExecutionTrend_4HBBS_Swing(pParams, pIndicators, pBase_Indicators);
		break;	
	case 2:
		workoutExecutionTrend_Limit(pParams, pIndicators, pBase_Indicators);	
		break;
	case 3:
		workoutExecutionTrend_Limit_BreakOutOnPivot(pParams, pIndicators, pBase_Indicators);
		break;
	case 9:		
		workoutExecutionTrend_Limit_BBS(pParams, pIndicators, pBase_Indicators);
		break;
	case 10:
		workoutExecutionTrend_WeeklyAuto(pParams, pIndicators, pBase_Indicators);
		break;
	case 13:
		workoutExecutionTrend_Weekly_Swing_New(pParams, pIndicators, pBase_Indicators);
		break;
	case 14:
		workoutExecutionTrend_Auto_Hedge(pParams, pIndicators, pBase_Indicators);
		break;
	case 15:
		workoutExecutionTrend_XAUUSD_DayTrading(pParams, pIndicators, pBase_Indicators);
		break;
	case 16:
		//workoutExecutionTrend_GBPJPY_DayTrading(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_GBPJPY_DayTrading_Ver2(pParams, pIndicators, pBase_Indicators);
		break;
	case 17:
		workoutExecutionTrend_WeeklyATR_Prediction(pParams, pIndicators, pBase_Indicators);
		break;
	case 18:
		workoutExecutionTrend_4HBBS_Swing_BoDuan(pParams, pIndicators, pBase_Indicators);
		break;
	case 19:		
		workoutExecutionTrend_DayTrading_ExecutionOnly(pParams, pIndicators, pBase_Indicators);
		break;
	case 20:
		workoutExecutionTrend_4HBBS_Swing_XAUUSD_BoDuan(pParams, pIndicators, pBase_Indicators);
		break;
	case 21:				
		workoutExecutionTrend_MultipleDay(pParams, pIndicators, pBase_Indicators);
		break;
	case 22:		
		workoutExecutionTrend_MultipleDay_V2(pParams, pIndicators, pBase_Indicators);
		break;
	case 23:
		workoutExecutionTrend_MACD_Daily(pParams, pIndicators, pBase_Indicators);
		break;
	case 24:
		workoutExecutionTrend_MACD_Weekly(pParams, pIndicators, pBase_Indicators);
		break;
	case 25: // 
		pIndicators->tradeMode = 1;
		workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);
		break;
	case 26: 
		//pIndicators->tradeMode = 0;
		//workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_Limit_BBS_LongTerm(pParams, pIndicators, pBase_Indicators);
		break;
	case 27:
		workoutExecutionTrend_Ichimoko_Daily_V2(pParams, pIndicators, pBase_Indicators);
		break;	
	case 28:
		workoutExecutionTrend_Ichimoko_Daily_Index(pParams, pIndicators, pBase_Indicators);
		break;
	case 29:
		workoutExecutionTrend_Test(pParams, pIndicators, pBase_Indicators);
		break;
	//case 30:
	//	workoutExecutionTrend_Ichimoko_Daily_Index_V2(pParams, pIndicators, pBase_Indicators);		
	//	break;
	case 30:		
		//workoutExecutionTrend_4H_ShellingtonVer1(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_4H_Shellington(pParams, pIndicators, pBase_Indicators);		
		break;
	case 31:
		workoutExecutionTrend_Ichimoko_Daily_New(pParams, pIndicators, pBase_Indicators);
		break;
	case 32:
		workoutExecutionTrend_MACD_Daily_New(pParams, pIndicators, pBase_Indicators);
		break;
	case 33:
		workoutExecutionTrend_MACD_BEILI(pParams, pIndicators, pBase_Indicators);
		break;
	case 34:
		workoutExecutionTrend_ShortTerm(pParams, pIndicators, pBase_Indicators);
		break;
	case 35:
		workoutExecutionTrend_Ichimoko_Weekly_Index(pParams, pIndicators, pBase_Indicators);
		break;
	case 36:
		//workoutExecutionTrend_BTCUSD_DayTrading_Ver2(pParams, pIndicators, pBase_Indicators);
		break;
	case 101:
		workoutExecutionTrend_Ichimoko_Daily_Index_Regression_Test(pParams, pIndicators, pBase_Indicators);
		break;
	case 102:
		workoutExecutionTrend_ASI(pParams, pIndicators, pBase_Indicators);
		break;
	case 103:
		workoutExecutionTrend_MACD_BEILI_Daily_Regression(pParams, pIndicators, pBase_Indicators);
		break;
	}


	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && pParams->accountInfo.totalOpenTradeRiskPercent < parameter(AUTOBBS_MAX_ACCOUNT_RISK) * -1) //if account risk is more than 3%, stop entring trades.
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Over max riks %lf�� skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, parameter(AUTOBBS_MAX_ACCOUNT_RISK), pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	//Filter out macro trend
	if (pIndicators->side != 0 && pIndicators->entrySignal != 0 && pIndicators->side != pIndicators->entrySignal)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s,Againt Side =%ld�� skip this entry signal=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->side, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	//// Dont enter trade on the new day bar, it is too risky and not reliable.
	//if (pIndicators->entrySignal != 0 && timeInfo.tm_hour == startHour && timeInfo.tm_min == 0)
	//{
	//	pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s Not allowed to trade on the firt bar of new day�� skip this entry signal=%d",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	//}

	return SUCCESS;
}

static AsirikuyReturnCode setUIValues(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{

	switch ((int)parameter(AUTOBBS_TREND_MODE)) {
	case 0:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("dailyTP", pBase_Indicators->dailyTP);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("bbsTrend_4H", pIndicators->bbsTrend_4H);
		addValueToUI("bbsStopPrice_4H", pIndicators->bbsStopPrice_4H);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);		
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		break;
	case 2:
	case 3:
		addValueToUI("MacdTrend", pBase_Indicators->mACDInTrend);
		addValueToUI("ShellingtonTrend", pBase_Indicators->shellingtonInTrend);
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("flatTrend", pBase_Indicators->flatTrend);
		addValueToUI("dailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS1", pBase_Indicators->dailyS1);
		addValueToUI("DailyR1", pBase_Indicators->dailyR1);
		addValueToUI("DailyS", pBase_Indicators->dailyS);		
		addValueToUI("maxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("takeProfit", pIndicators->takePrice);
		addValueToUI("stopLoss", pIndicators->stopLoss);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);

		break;
	case 5:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("BBSTrend_primary", pIndicators->bbsTrend_primary);
		addValueToUI("BBSStopPrice_primary", pIndicators->bbsStopPrice_primary);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		//addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		//addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		//addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
	case 6:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		//addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		//addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		//addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	case 9:
		addValueToUI("MacdTrend", pBase_Indicators->mACDInTrend);
		addValueToUI("ShellingtonTrend", pBase_Indicators->shellingtonInTrend);
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);		
		addValueToUI("dailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS1", pBase_Indicators->dailyS1);
		addValueToUI("DailyR1", pBase_Indicators->dailyR1);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("maxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("takeProfit", pIndicators->takePrice);
		addValueToUI("stopLoss", pIndicators->stopLoss);
		addValueToUI("macdMaxLevel", (double)parameter(AUTOBBS_IS_ATREURO_RANGE));
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		break;
	case 10:
		addValueToUI("weeklyTrend", pBase_Indicators->weeklyTrend);
		addValueToUI("weeklyTrend_Phase", pBase_Indicators->weeklyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("weeklyS", pBase_Indicators->weeklyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("weeklyPivot", pBase_Indicators->weeklyPivot);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		//addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		//addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		//addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	case 15:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_secondary);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_secondary);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		//addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		//addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		//addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	case 16:
	case 19:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);		
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);		
		addValueToUI("pDailyPredictATR", pBase_Indicators->pDailyPredictATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		//addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		//addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		//addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	case 17:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);		
		addValueToUI("dailyATR", pBase_Indicators->pDailyPredictATR);
		addValueToUI("dailyMaxATR", pBase_Indicators->pDailyMaxATR);
		addValueToUI("weeklyATR", pBase_Indicators->pWeeklyPredictATR);
		addValueToUI("weeklyMaxATR", pBase_Indicators->pWeeklyPredictMaxATR);	
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		break;
	case 18:
	case 20:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("atr_euro_range", pIndicators->atr_euro_range);
		addValueToUI("pWeeklyPredictATR", pBase_Indicators->pWeeklyPredictATR);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 21: //Multday. Inking	
	case 22: //Multday. Inking		
		addValueToUI("entrySignal", pIndicators->entrySignal);		
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("atr_euro_range", pIndicators->atr_euro_range);
		addValueToUI("stopLoss", pIndicators->stopLoss);
		addValueToUI("takePrice", pIndicators->takePrice);		
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 23: //MACD		
		addValueToUI("entrySignal", pIndicators->entrySignal);		
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);		
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);	
		addValueToUI("volume1", pIndicators->volume1);
		addValueToUI("volume2", pIndicators->volume2);
		addValueToUI("cmfVolume", pIndicators->cmfVolume);
		addValueToUI("CMFVolumeGap", pIndicators->CMFVolumeGap);
		addValueToUI("fast", pIndicators->fast);
		addValueToUI("preFast", pIndicators->preFast);
		addValueToUI("slow", pIndicators->slow);		
		addValueToUI("preSlow", pIndicators->preSlow);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);	
		addValueToUI(pIndicators->status, 0);
		break;
	case 26:
		addValueToUI("MacdTrend", pBase_Indicators->mACDInTrend);
		addValueToUI("ShellingtonTrend", pBase_Indicators->shellingtonInTrend);
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("maxATR", pBase_Indicators->pDailyMaxATR);		
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);

		break;
	case 30: //Shellington low risk	
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);						
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI(pIndicators->status, 0);
		break;
	case 31: //Ichikomo
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);	
		addValueToUI("daily_baseline", pIndicators->daily_baseline);
		addValueToUI("daily_baseline_short", pIndicators->daily_baseline_short);
		addValueToUI("cmfVolume", pIndicators->cmfVolume);		
		addValueToUI("fast", pIndicators->fast);		
		addValueToUI("slow", pIndicators->slow);		
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
//		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		break;
	default:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("entrySignal", pIndicators->entrySignal);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("dailyATR", pBase_Indicators->dailyATR);
		addValueToUI("weeklyATR", pBase_Indicators->weeklyATR);
		addValueToUI("pDailyHigh", pBase_Indicators->pDailyHigh);
		addValueToUI("pDailyLow", pBase_Indicators->pDailyLow);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_excution);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_excution);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		//addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		//addValueToUI("riskPNL", pIndicators->riskPNL);
		//addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		//addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		//addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	}

	return SUCCESS;
}

static AsirikuyReturnCode loadIndicators(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	double originEquity = 0.0;
	double risk = 1.0;

	pIndicators->primaryATR = iAtr(B_PRIMARY_RATES, (int)parameter(ATR_AVERAGING_PERIOD), 1);

	iBBandStop(B_PRIMARY_RATES, 20, 2, &pIndicators->bbsTrend_primary, &pIndicators->bbsStopPrice_primary, &pIndicators->bbsIndex_primary);
	iBBandStop(B_SECONDARY_RATES, 20, 2, &pIndicators->bbsTrend_secondary, &pIndicators->bbsStopPrice_secondary, &pIndicators->bbsIndex_secondary);
	iBBandStop(B_HOURLY_RATES, 20, 2, &pIndicators->bbsTrend_1H, &pIndicators->bbsStopPrice_1H, &pIndicators->bbsIndex_1H);
	iBBandStop(B_FOURHOURLY_RATES, 20, 2, &pIndicators->bbsTrend_4H, &pIndicators->bbsStopPrice_4H, &pIndicators->bbsIndex_4H);
	iBBandStop(B_DAILY_RATES, 20, 2, &pIndicators->bbsTrend_Daily, &pIndicators->bbsStopPrice_Daily, &pIndicators->bbsIndex_Daily);

	pIndicators->adjust = (double)parameter(AUTOBBS_ADJUSTPOINTS);
	pIndicators->risk = 1;
	pIndicators->entrySignal = 0;
	pIndicators->exitSignal = EXIT_NONE;
	pIndicators->executionTrend = 0;
	pIndicators->orderManagement = 0;
	pIndicators->tpMode = (int)parameter(AUTOBBS_LONG_SHORT_MODE);
	pIndicators->splitTradeMode = (int)parameter(AUTOBBS_TREND_MODE);
	pIndicators->stopLossPrice = 0;
	pIndicators->macroTrend = (int)parameter(AUTOBBS_MACRO_TREND);
	pIndicators->side = (int)parameter(AUTOBBS_ONE_SIDE);
	pIndicators->executionRateTF = (int)parameter(AUTOBBS_EXECUTION_RATES);

	pIndicators->tradeMode = 1;
	pIndicators->atr_euro_range = (double)parameter(AUTOBBS_IS_ATREURO_RANGE);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[ACCOUNT_RISK_PERCENT] * -1;

	pIndicators->total_lose_pips = 0;

	pIndicators->maxTradeTime = 3; //��������Σ�����?

	pIndicators->startHour = (int) parameter(AUTOBBS_STARTHOUR);

	pIndicators->stopMovingBackSL = TRUE;

	pIndicators->volume1 = 0.0;
	pIndicators->volume2 = 0.0;
	pIndicators->cmfVolume = 0.0;
	pIndicators->CMFVolumeGap = 0.0;
	pIndicators->fast = 0.0;
	pIndicators->slow = 0.0;
	pIndicators->preFast = 0.0;
	pIndicators->preSlow = 0.0;

	pIndicators->daily_baseline = 0.0;
	pIndicators->daily_baseline_short = 0.0;

	//Override equity
	pIndicators->virtualBalanceTopup = (double)parameter(AUTOBBS_VIRTUAL_BALANCE_TOPUP);
	if (pIndicators->virtualBalanceTopup > 0)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d, top up equity %lf ",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], pIndicators->virtualBalanceTopup);
		//update totalOpenTradeRiskPercent
		originEquity = pParams->accountInfo.equity;
		pParams->accountInfo.equity += pIndicators->virtualBalanceTopup;

		risk = readRiskFile((BOOL)pParams->settings[IS_BACKTESTING]);
		pParams->accountInfo.equity = pParams->accountInfo.equity * risk;

		pParams->accountInfo.totalOpenTradeRiskPercent = pParams->accountInfo.totalOpenTradeRiskPercent / (pParams->accountInfo.equity / originEquity);

	}
	
	pIndicators->riskCap = 0.0;
	pIndicators->minLotSize = 0.01;
	pIndicators->volumeStep = 0.01;
	pIndicators->isEnableBuyMinLotSize = FALSE;
	pIndicators->isEnableSellMinLotSize = FALSE;

	memset(pIndicators->status, '\0', MAX_OUTPUT_ERROR_STRING_SIZE);

	strcpy(pIndicators->status, "No Error");

	workoutExecutionTrend(pParams, pIndicators, pBase_Indicators);

	return returnCode;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double	   stopLoss, takePrice_primary = 0;
	int riskcap = (int)parameter(AUTOBBS_RISK_CAP);


	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pParams = NULL");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"handleTradeEntries() failed. pIndicators = NULL");
		return NULL_POINTER;
	}

	// When the trade is created from 23:45 to 00:20, increase the 3 times of spread for cut over
	if ((hour() == 23 && minute() > 40) || (hour() == 00 && minute() < 20))
		pIndicators->adjust = 3 * pIndicators->adjust;

	//stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) + pIndicators->adjust;
	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice);

	switch (pIndicators->tpMode) {
	case 0:
		takePrice_primary = stopLoss;
		break;
	case 1:
		takePrice_primary = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_secondary) + pIndicators->adjust;		
		pIndicators->risk = pIndicators->risk * min(stopLoss / takePrice_primary, riskcap);
		break;
	case 2:
		takePrice_primary = 0;
		break;
	case 3:
		takePrice_primary = pBase_Indicators->dailyATR;
		break;
	case 4:
		takePrice_primary = fabs(pIndicators->entryPrice - pIndicators->takeProfitPrice);
		break;
	default:
		takePrice_primary = stopLoss;
		break;
	}

	if (pIndicators->executionTrend == 1)
	{
		if (pIndicators->entrySignal == 1)
			splitBuyOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		else if (totalOpenOrders(pParams, BUY) > 0)
			modifyOrders(pParams, pIndicators, pBase_Indicators, BUY, stopLoss, -1);
	}

	if (pIndicators->executionTrend == -1)
	{
		if (pIndicators->entrySignal == -1)
			splitSellOrders(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		else if (totalOpenOrders(pParams, SELL) > 0)
			modifyOrders(pParams, pIndicators, pBase_Indicators, SELL, stopLoss, -1);

	}

	return SUCCESS;
}

AsirikuyReturnCode runAutoBBS(StrategyParams* pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;
	Base_Indicators base_Indicators;
	int rateErrorTimes = -1;
	BOOL isRateCheck = TRUE;
	

	char       timeString[MAX_TIME_STRING_SIZE] = "";
	int        shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;

	if (pParams == NULL)
	{
		pantheios_logputs(PANTHEIOS_SEV_CRITICAL, (PAN_CHAR_T*)"runAutoBBS() failed. pParams = NULL");
		return NULL_POINTER;
	}

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	if (strcmp(timeString, "09/03/23 01:00") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, "hit a point");

	if (strcmp(timeString, "22/11/21 15:00") == 0)
		pantheios_logprintf(PANTHEIOS_SEV_DEBUG, "hit a point");

	if ((int)parameter(AUTOBBS_TREND_MODE) == 16) // GBPJPY Daily Swing strategy, �����ֻ��Ҫ���ڵ�ָ��?
		base_Indicators.strategy_mode = 0;
	else
		base_Indicators.strategy_mode = 1;

	// Read rateError.txt file to get validateSecondaryBarsGap error times. 
	// If more than 3 times, skip checking. 
	rateErrorTimes = readRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);


	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && (int)pParams->settings[TIMEFRAME] >= 5 &&			
		(
		validateCurrentTimeEasy(pParams, B_PRIMARY_RATES) > 0 ||
		validateDailyBarsEasy(pParams, B_PRIMARY_RATES, B_DAILY_RATES) > 0 ||
		validateHourlyBarsEasy(pParams, B_PRIMARY_RATES, B_HOURLY_RATES) > 0 ||
		validateSecondaryBarsEasy(pParams, B_PRIMARY_RATES, B_SECONDARY_RATES, (int)parameter(AUTOBBS_EXECUTION_RATES), rateErrorTimes) > 0
		)
		)
	{
		pantheios_logprintf(PANTHEIOS_SEV_WARNING, (PAN_CHAR_T*)"System InstanceID = %d,BarTime = %s: validate time failure.", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		return SUCCESS;
	}

	//if passed, reset back to 0
	//if (rateErrorTimes >= 10)
	//	saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);

	if ((int)parameter(AUTOBBS_MACRO_TREND) * (int)parameter(AUTOBBS_ONE_SIDE) < 0)
	{
		pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Invalid paramenter config: System InstanceID = %d, BarTime = %s, AUTOBBS_MACRO_TREND= %d��AUTOBBS_ONE_SIDE=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, (int)parameter(AUTOBBS_MACRO_TREND), (int)parameter(AUTOBBS_ONE_SIDE));
		return INVALID_CONFIG;
	}

	// Load the base indicators
	// if trend mode >=99, they dont need to load base indicators.
	if ((int)parameter(AUTOBBS_TREND_MODE) < 99)
		runBase(pParams, &base_Indicators);

	loadIndicators(pParams, &indicators, &base_Indicators);

	setUIValues(pParams, &indicators, &base_Indicators);


	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_primary=%ld,BBStopPrice_primary=%lf, BBSIndex_primary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_primary, indicators.bbsStopPrice_primary, indicators.bbsIndex_primary);
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,bbsTrend_secondary=%ld,BBStopPrice_secondary=%lf, bbsIndex_secondary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_secondary, indicators.bbsStopPrice_secondary, indicators.bbsIndex_secondary);
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_1H=%ld,BBStopPrice_1H=%lf, BBSIndex_1H = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_1H, indicators.bbsStopPrice_1H, indicators.bbsIndex_1H);
	pantheios_logprintf(PANTHEIOS_SEV_DEBUG, (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_4H=%ld,BBStopPrice_4H=%lf, BBSIndex_4H = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_4H, indicators.bbsStopPrice_4H, indicators.bbsIndex_4H);

	returnCode = handleTradeExits(pParams, &indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeExits()", returnCode);
	}

	returnCode = handleTradeEntries(pParams, &indicators, &base_Indicators);
	if (returnCode != SUCCESS)
	{
		return logAsirikuyError("runAutoBBS->handleTradeEntries()", returnCode);
	}

	return SUCCESS;
}