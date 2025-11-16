/*
 * Indicator Management Module
 * 
 * Provides indicator loading and UI value setting functions.
 * Handles loading of technical indicators and updating UI display values.
 */

#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/shared/execution/StrategyExecution.h"
#include "strategies/autobbs/shared/indicators/IndicatorManagement.h"

AsirikuyReturnCode loadIndicators(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

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

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[ACCOUNT_RISK_PERCENT] * -1;

	pIndicators->total_lose_pips = 0;

	workoutExecutionTrend(pParams, pIndicators, pBase_Indicators);

	return returnCode;
}
AsirikuyReturnCode setUIValues(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{

	switch ((int)parameter(AUTOBBS_TREND_MODE))
	{
	case 15:
	case 16:
		addValueToUI("DailyTrend", pBase_Indicators->dailyTrend);
		addValueToUI("dailyTrend_Phase", pBase_Indicators->dailyTrend_Phase);
		addValueToUI("ExecutionTrend", pIndicators->executionTrend);
		addValueToUI("DailyPivot", pBase_Indicators->dailyPivot);
		addValueToUI("DailyS", pBase_Indicators->dailyS);
		addValueToUI("stopLossPrice", pIndicators->stopLossPrice);
		addValueToUI("bbsTrend_excution", pIndicators->bbsTrend_secondary);
		addValueToUI("bbsStopPrice_excution", pIndicators->bbsStopPrice_secondary);
		addValueToUI("AccountRisk", pParams->accountInfo.totalOpenTradeRiskPercent);
		addValueToUI("strategyRisk", pIndicators->strategyRisk);
		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		addValueToUI("riskPNL", pIndicators->riskPNL);
		addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
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
		addValueToUI("strategyRiskNLP", pIndicators->strategyRiskWithoutLockedProfit);
		addValueToUI("riskPNL", pIndicators->riskPNL);
		addValueToUI("riskPNLNLP", pIndicators->riskPNLWithoutLockedProfit);
		addValueToUI("StrategyVolRisk", pIndicators->riskPNL - pIndicators->strategyRisk);
		addValueToUI("strategyMarketVolRisk", pIndicators->strategyMarketVolRisk);
		break;
	}
	return SUCCESS;
}
