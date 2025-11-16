
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/swing/SwingStrategy.h"
#include "strategies/autobbs/trend/TrendStrategy.h"
#include "StrategyUserInterface.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"  // For splitBuyOrders_ATR, splitBuyOrders_LongTerm
#include "strategies/autobbs/shared/execution/StrategyExecution.h"  // For workoutExecutionTrend
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"  // For splitBuyOrders, splitSellOrders

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// Note: All function declarations are provided by SwingStrategy.h, TrendStrategy.h,
// StrategyExecution.h, and OrderSplitting.h. No forward declarations needed here.

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
		logWarning("System InstanceID = %d, top up equity %lf ",
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

	strcpy(pIndicators->status, "No Error\n\n");

	workoutExecutionTrend(pParams, pIndicators, pBase_Indicators);

	return returnCode;
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators)
{
	double	   stopLoss, takePrice_primary = 0;
	int riskcap = (int)parameter(AUTOBBS_RISK_CAP);


	if (pParams == NULL)
	{
		logCritical("handleTradeEntries() failed. pParams = NULL\n\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		logCritical("handleTradeEntries() failed. pIndicators = NULL\n\n");
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
		logCritical("runAutoBBS() failed. pParams = NULL\n\n");
		return NULL_POINTER;
	}

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

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
		logWarning("System InstanceID = %d,BarTime = %s: validate time failure.", (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
		return SUCCESS;
	}

	//if passed, reset back to 0
	//if (rateErrorTimes >= 10)
	//	saveRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], 0, (BOOL)pParams->settings[IS_BACKTESTING]);

	if ((int)parameter(AUTOBBS_MACRO_TREND) * (int)parameter(AUTOBBS_ONE_SIDE) < 0)
	{
		logError("Invalid paramenter config: System InstanceID = %d, BarTime = %s, AUTOBBS_MACRO_TREND= %d��AUTOBBS_ONE_SIDE=%d",
			(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, (int)parameter(AUTOBBS_MACRO_TREND), (int)parameter(AUTOBBS_ONE_SIDE));
		return INVALID_CONFIG;
	}

	// Load the base indicators
	// if trend mode >=99, they dont need to load base indicators.
	if ((int)parameter(AUTOBBS_TREND_MODE) < 99)
		runBase(pParams, &base_Indicators);

	loadIndicators(pParams, &indicators, &base_Indicators);

	setUIValues(pParams, &indicators, &base_Indicators);


	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_primary=%ld,BBStopPrice_primary=%lf, BBSIndex_primary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_primary, indicators.bbsStopPrice_primary, indicators.bbsIndex_primary);
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,bbsTrend_secondary=%ld,BBStopPrice_secondary=%lf, bbsIndex_secondary = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_secondary, indicators.bbsStopPrice_secondary, indicators.bbsIndex_secondary);
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_1H=%ld,BBStopPrice_1H=%lf, BBSIndex_1H = %ld",
		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_1H, indicators.bbsStopPrice_1H, indicators.bbsIndex_1H);
	logDebug("System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_4H=%ld,BBStopPrice_4H=%lf, BBSIndex_4H = %ld",
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