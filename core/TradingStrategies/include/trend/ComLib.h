#pragma once

#ifndef COMLIB_H_
#define COMLIB_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
	typedef enum additionalSettings_t
	{
		AUTOBBS_ADJUSTPOINTS = ADDITIONAL_PARAM_1,
		AUTOBBS_TP_MODE = ADDITIONAL_PARAM_2,// BBS_TP_MODE = 0 (XAUUSD): The last trade will be no TP; BBS_TP_MODE=1: the last trade will be 1:3 
		AUTOBBS_TREND_MODE = ADDITIONAL_PARAM_3, // 0: 4HBBS only; 1: Base+4HBBS, filter out the range.
		AUTOBBS_RISK_CAP = ADDITIONAL_PARAM_4,
		AUTOBBS_LONG_SHORT_MODE = ADDITIONAL_PARAM_5, // 1: LONG TERM trade, TP = 4H StopLoss, 0: Short term trade: TP= primary stop loss, ie: 15M stop loss
		AUTOBBS_MAX_ACCOUNT_RISK = ADDITIONAL_PARAM_6, // 1: LONG TERM trade, TP = 4H StopLoss, 0: Short term trade: TP= primary stop loss, ie: 15M stop loss
		AUTOBBS_KEYK = ADDITIONAL_PARAM_7, // 1: use KeyK, 0: no KeyK
		AUTOBBS_RANGE = ADDITIONAL_PARAM_8, // 0: Exit all in Range, 1: Exit only when both weekly and daily ranges
		AUTOBBS_MAX_STRATEGY_RISK = ADDITIONAL_PARAM_9,
		AUTOBBS_MACRO_TREND = ADDITIONAL_PARAM_10, //��������ĳ������ƵĻ��������
		AUTOBBS_EXECUTION_RATES = ADDITIONAL_PARAM_11, // ��ȷָ��BBS��ִ��ʱ���ܣ�����15��30
		AUTOBBS_ONE_SIDE = ADDITIONAL_PARAM_12,  // 1: Buy only, -1: Sell only, 0: either	
		AUTOBBS_IS_AUTO_MODE = ADDITIONAL_PARAM_13,  // 1: apply for the filter for daily trading, 0: ignore the filter, that is manual mode
		AUTOBBS_IS_ATREURO_RANGE = ADDITIONAL_PARAM_14, //XAUUSD һ����8
		AUTOBBS_STARTHOUR = ADDITIONAL_PARAM_15, //�ֹ�ѡ����Կ�ʼʱ�䡣
		AUTOBBS_VIRTUAL_BALANCE_TOPUP = ADDITIONAL_PARAM_16  // virtual balance top up
	} AdditionalSettings;


	typedef struct indicators_t
	{
		//BBS indicators	
		int side;
		int macroTrend;
		int executionTrend;
		int entrySignal;
		int exitSignal;
		int orderManagement;
		int tpMode;
		int splitTradeMode;
		int tradeMode;
		int subTradeMode;

		int bbsTrend_excution;
		double bbsStopPrice_excution;
		int bbsIndex_excution;

		int bbsTrend_primary;
		double bbsStopPrice_primary;
		int bbsIndex_primary;

		int bbsTrend_secondary;
		double bbsStopPrice_secondary;
		int bbsIndex_secondary;

		int bbsTrend_1H;
		double bbsStopPrice_1H;
		int bbsIndex_1H;

		int bbsTrend_4H;
		double bbsStopPrice_4H;
		int bbsIndex_4H;

		int bbsTrend_Daily;
		double bbsStopPrice_Daily;
		int bbsIndex_Daily;

		double entryPrice;
		double stopLossPrice;
		double takeProfitPrice;
		double risk;

		double primaryATR;
		double adjust;

		double strategyMarketVolRisk;
		double strategyRisk;
		double strategyRiskWithoutLockedProfit;
		double riskPNL;
		double riskPNLWithoutLockedProfit;
		double limitRiskPNL;
		double strategyMaxRisk;
		double strategyMaxDD;

		int executionRateTF;

		double total_lose_pips;
		int lossTimes;
		int winTimes;

		double asia_high;
		double asia_low;
		double asia_close;
		double asia_open;

		double euro_high;
		double euro_low;
		double euro_close;
		double euro_open;

		double atr0;

		double atr_euro_range;
		double takePrice;		
		double takeprice2;
		double stopLoss;
		int startHour;
		int startHourOnLimt;
		int endHour;
		int maxTradeTime;

		BOOL stopMovingBackSL;

		double virtualBalanceTopup;

		//MACD output
		double fast,preFast;
		double slow,preSlow;
		double cmfVolume;
		double CMFVolumeGap;
		double volume1, volume2;

		//Ichikomo output
		double daily_baseline;
		double daily_baseline_short;

		//Risk reward 
		double riskCap;

		double minLotSize;

		double volumeStep;

		BOOL isEnableBuyMinLotSize;
		BOOL isEnableSellMinLotSize;
		BOOL isEnableLimitSR1;
		
		char status[MAX_OUTPUT_ERROR_STRING_SIZE];

		double takePriceLevel;

	} Indicators;
	

	int filterExcutionTF(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
	
	void profitManagement_base(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
	void profitManagementWeekly(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
	void profitManagement(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);


	AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, Indicators* pIndicators);
	AsirikuyReturnCode modifyOrders(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double stopLoss, double takePrice);

	void traceLatestOpenStopLoss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, double minTP, double traceSL);

	int weeklyTrend4HSwingSignal(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

	BOOL XAUUSD_IsKeyDate(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

	double getCMFVolume(int index, int periods, int shift);
	double getCMFVolumeGap(int index, int fast_period, int slow_period, int shift);

	BOOL isNextdayMACDPostiveBar(int startShift);
	BOOL isNextdayMACDPostiveBar2(StrategyParams* pParams,int orderIndex,int startShift);
	double roundUp(double lots, double volumeStep);

	AsirikuyReturnCode getHighestHourlyClosePrice(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int rate_index, int orderIndex, double * highPrice, double * lowPrice);
	AsirikuyReturnCode getHighLowPrice(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int rate_index,int timeFrame, int orderIndex, double * highPrice, double * lowPrice);
	AsirikuyReturnCode addMoreOrdersOnLongTermTrend(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int oldestOpenOrderIndex);

	BOOL XAUUSD_not_full_trading_day(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* COMLIB_H_ */
