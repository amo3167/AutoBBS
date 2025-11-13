
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSEF

// exitSignal_t is already defined in base.h

typedef enum additionalSettings_t
{
	AUTOBBS_ADJUSTPOINTS = ADDITIONAL_PARAM_1,
	AUTOBBS_TP_MODE = ADDITIONAL_PARAM_2,	 // BBS_TP_MODE = 0 (XAUUSD): The last trade will be no TP; BBS_TP_MODE=1: the last trade will be 1:3
	AUTOBBS_TREND_MODE = ADDITIONAL_PARAM_3, // 0: 4HBBS only; 1: Base+4HBBS, filter out the range.
	AUTOBBS_RISK_CAP = ADDITIONAL_PARAM_4,
	AUTOBBS_LONG_SHORT_MODE = ADDITIONAL_PARAM_5,  // 1: LONG TERM trade, TP = 4H StopLoss, 0: Short term trade: TP= primary stop loss, ie: 15M stop loss
	AUTOBBS_MAX_ACCOUNT_RISK = ADDITIONAL_PARAM_6, // 1: LONG TERM trade, TP = 4H StopLoss, 0: Short term trade: TP= primary stop loss, ie: 15M stop loss
	AUTOBBS_KEYK = ADDITIONAL_PARAM_7,			   // 1: use KeyK, 0: no KeyK
	AUTOBBS_RANGE = ADDITIONAL_PARAM_8,			   // 0: Exit all in Range, 1: Exit only when both weekly and daily ranges
	AUTOBBS_MAX_STRATEGY_RISK = ADDITIONAL_PARAM_9,
	AUTOBBS_MACRO_TREND = ADDITIONAL_PARAM_10,	   // Whether to follow trend or hedge based on macro trend
	AUTOBBS_EXECUTION_RATES = ADDITIONAL_PARAM_11, // Specify BBS execution timeframe, for example 15 or 30
	AUTOBBS_ONE_SIDE = ADDITIONAL_PARAM_12,		   // 1: Buy only, -1: Sell only, 0: either
	AUTOBBS_IS_AUTO_MODE = ADDITIONAL_PARAM_13	   // 1: apply for the filter for daily trading, 0: ignore the filter, that is manual mode
} AdditionalSettings;

typedef struct indicators_t
{
	// BBS indicators
	int side;
	int macroTrend;
	int executionTrend;
	int entrySignal;
	int exitSignal;
	int orderManagement;
	int tpMode;
	int splitTradeMode;
	int tradeMode;

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

	// double us_high;
	// double us_low;
	// double us_close;
	// double us_open;

} Indicators;

// Forward declarations
static AsirikuyReturnCode loadIndicators(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode setUIValues(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode handleTradeExits(StrategyParams* pParams, Indicators* pIndicators);
static AsirikuyReturnCode handleTradeEntries(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend_MIDDLE_PHASE(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, BOOL ignored);
static AsirikuyReturnCode workoutExecutionTrend_BEGINNING_PHASE(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, BOOL ignored);
static AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
static AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_PHASE(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, BOOL ignored);
AsirikuyReturnCode workoutExecutionTrend_Weekly_BEGINNING_PHASE(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, BOOL ignored);
AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, BOOL ignored);
static void profitManagementWeekly(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators);
// isNewDay is declared in base.h, so we use that one instead of defining a static version
void resetRateFile(int instanceId, BOOL isBackTesting)
{
    /* Reset rate file by saving 0 */
    saveRateFile(instanceId, 0, isBackTesting);
}
AsirikuyReturnCode closeAllBuyLimitOrdersEasy(time_t currentTime);
AsirikuyReturnCode closeAllSellLimitOrdersEasy(time_t currentTime);

static int filterExcutionTF_ByTime(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	struct tm timeInfo1;
	time_t currentTime;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	// Only work for 15M and always use secondary rates, in this case, it should be 15M
	if (timeInfo1.tm_hour <= 7) // it means 15-25M
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_secondary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_secondary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_secondary;
		shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

		if (timeInfo1.tm_min % pIndicators->executionRateTF >= 3)
			return SUCCESS;
	}
	else
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_primary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_primary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_primary;
		shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	}
	return shift1Index;
}

static int filterExcutionTF(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	struct tm timeInfo1;
	time_t currentTime;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	// Only work for 15M and always use secondary rates, in this case, it should be 15M
	if ((int)pParams->settings[TIMEFRAME] != pIndicators->executionRateTF) // it means 15-25M
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_secondary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_secondary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_secondary;
		shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;

		if (timeInfo1.tm_min % pIndicators->executionRateTF >= 3)
			return SUCCESS;
	}
	else
	{
		pIndicators->bbsTrend_excution = pIndicators->bbsTrend_primary;
		pIndicators->bbsIndex_excution = pIndicators->bbsIndex_primary;
		pIndicators->bbsStopPrice_excution = pIndicators->bbsStopPrice_primary;
		shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	}
	return shift1Index;
}

AsirikuyReturnCode runXAUUSD(StrategyParams *pParams)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	Indicators indicators;
	Base_Indicators base_Indicators;
	int rateErrorTimes = -1;
	BOOL isRateCheck = TRUE;

	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;

	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] runAutoBBS() failed. pParams = NULL\n\n\n");
		return NULL_POINTER;
	}

	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	if (strcmp(timeString, "19/07/17 11:00") == 0)
		fprintf(stderr, "[INFO] hit a point\n\n\n");

	if (strcmp(timeString, "29/09/17 16:30") == 0)
		fprintf(stderr, "[INFO] hit a point\n\n\n");

	if ((int)parameter(AUTOBBS_TREND_MODE) == 10) // Weekly Auto
		base_Indicators.weeklyMAMode = 0;
	else
		base_Indicators.weeklyMAMode = 0;

	// Read rateError.txt file to get validateSecondaryBarsGap error times.
	// If more than 3 times, skip checking.

	rateErrorTimes = readRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

	if (rateErrorTimes >= 2)
		isRateCheck = FALSE;

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE &&
		(validateCurrentTimeEasy(pParams, B_PRIMARY_RATES) > 0 ||
		 validateDailyBarsEasy(pParams, B_PRIMARY_RATES, B_DAILY_RATES) > 0 ||
		 validateHourlyBarsEasy(pParams, B_PRIMARY_RATES, B_HOURLY_RATES) > 0 ||
		 validateSecondaryBarsEasy(pParams, B_PRIMARY_RATES, B_SECONDARY_RATES, (int)parameter(AUTOBBS_EXECUTION_RATES), isRateCheck) > 0))
		return SUCCESS;

	// if passed, reset number to 0.
	if (isRateCheck == TRUE)
		resetRateFile((int)pParams->settings[STRATEGY_INSTANCE_ID], (BOOL)pParams->settings[IS_BACKTESTING]);

	if ((int)parameter(AUTOBBS_MACRO_TREND) * (int)parameter(AUTOBBS_ONE_SIDE) < 0)
	{
		fprintf(stderr, "[ERROR] Invalid paramenter config: System InstanceID = %d, BarTime = %s, AUTOBBS_MACRO_TREND= %d, AUTOBBS_ONE_SIDE=%d\n\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, (int)parameter(AUTOBBS_MACRO_TREND), (int)parameter(AUTOBBS_ONE_SIDE));
		return INVALID_CONFIG;
	}

	// Load the base indicators
	runBase(pParams, &base_Indicators);

	loadIndicators(pParams, &indicators, &base_Indicators);

	setUIValues(pParams, &indicators, &base_Indicators);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_primary=%ld,BBStopPrice_primary=%lf, BBSIndex_primary = %ld\n\n",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_primary, indicators.bbsStopPrice_primary, indicators.bbsIndex_primary);
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,bbsTrend_secondary=%ld,BBStopPrice_secondary=%lf, bbsIndex_secondary = %ld\n\n\n",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_secondary, indicators.bbsStopPrice_secondary, indicators.bbsIndex_secondary);
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_1H=%ld,BBStopPrice_1H=%lf, BBSIndex_1H = %ld\n\n\n",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, indicators.executionTrend, indicators.bbsTrend_1H, indicators.bbsStopPrice_1H, indicators.bbsIndex_1H);
	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ExecutionTrend = %ld,BBSTrend_4H=%ld,BBStopPrice_4H=%lf, BBSIndex_4H = %ld\n\n\n",
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

static AsirikuyReturnCode loadIndicators(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
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

static AsirikuyReturnCode setUIValues(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
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

static void profitManagement_base(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{

	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	// Control risk by Macro factor
	if (pIndicators->entrySignal == 1)
	{
		if (pIndicators->macroTrend < 0)
		{
			pIndicators->risk *= 0.5;
			pIndicators->strategyMaxRisk *= 0.5;
		}
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= 0.8;
			pIndicators->strategyMaxRisk *= 0.8;
		}
	}

	if (pIndicators->entrySignal == -1)
	{
		if (pIndicators->macroTrend > 0)
		{
			pIndicators->risk *= 0.5;
			pIndicators->strategyMaxRisk *= 0.5;
		}
		else if (pIndicators->macroTrend == 0)
		{
			pIndicators->risk *= 0.8;
			pIndicators->strategyMaxRisk *= 0.8;
		}
	}

	if (pIndicators->entrySignal != 0 && pIndicators->strategyRiskWithoutLockedProfit < pIndicators->strategyMaxRisk)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, strategyRisk %lf, strategyRiskWithoutLockedProfit %lf, skip this entry signal=%d\n\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyRisk, pIndicators->strategyRiskWithoutLockedProfit, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	if (pIndicators->riskPNL < pIndicators->limitRiskPNL && pIndicators->entrySignal != 0)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, PNL riks %lf, riskPNLWithoutLockedProfit %lf, skip this entry signal=%d\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->riskPNL, pIndicators->riskPNLWithoutLockedProfit, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}
}

static void profitManagement(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double targetPNL = parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	int noTPOrderDaysNumber = 0;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	safe_timeString(timeString, currentTime);

	profitManagement_base(pParams, pIndicators, pBase_Indicators);
	noTPOrderDaysNumber = getSamePricePendingNoTPOrdersEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3);

	if (noTPOrderDaysNumber >= 4 && pIndicators->entrySignal != 0)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, SamePricePendingNoTPOrdersDays %d, skip this entry signal=%d\n\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, noTPOrderDaysNumber, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Daily Profit on EOD?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->dailyR3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->dailyS3)
		closeAllCurrentDayShortTermOrdersEasy(1, currentTime);

	// Weekly Profit on EOW?
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2)
		closeAllCurrentDayShortTermOrdersEasy(2, currentTime);

	// when floating profit is too high, fe 10%
	if (pBase_Indicators->dailyTrend_Phase > 0 && pParams->bidAsk.ask[0] > pBase_Indicators->weeklyR2 && pIndicators->riskPNL > targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);
	else if (pBase_Indicators->dailyTrend_Phase < 0 && pParams->bidAsk.bid[0] < pBase_Indicators->weeklyS2 && pIndicators->riskPNL > targetPNL)
		closeWinningPositionsEasy(pIndicators->riskPNL, targetPNL);

	// closeAllWithNegativeEasy(3, currentTime, 3);
	// closeAllWithNegativeEasy(2, currentTime,3);

	//// work out the Max DD
	// pIndicators->strategyMaxDD = pIndicators->riskPNL - pIndicators->strategyRisk;
	// if (pIndicators->strategyMaxDD > parameter(AUTOBBS_MAX_STRATEGY_RISK) * 3)
	//{
	//	fprintf(stderr, ("[WARNING] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Max DD %lf\n\n",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->strategyMaxDD);

	//	closeWinningPositionsEasy(pIndicators->riskPNL, pIndicators->riskPNL);
	//}
}

// isNewDay is already defined in Base.c and declared in base.h
// Using the global implementation instead of local static definition

static AsirikuyReturnCode modifyOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, OrderType orderType, double stopLoss, double takePrice)
{
	int tpMode = 0;
	int shift0Index;
	time_t currentTime;
	double stopLoss2;

	shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];

	if ((int)pParams->settings[TIMEFRAME] >= 60 && isNewDay(pParams, currentTime))
	{
		setLastOrderUpdateTime((int)pParams->settings[STRATEGY_INSTANCE_ID], pParams->ratesBuffers->rates[0].time[pParams->ratesBuffers->rates[0].info.arraySize - 1], (BOOL)pParams->settings[IS_BACKTESTING]);
		tpMode = 1;
	}

	if (orderType == BUY)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			if ((int)parameter(AUTOBBS_TREND_MODE) == 5) // Day Trading, override the stop loss to primary bbs on the new day.
			{
				// stopLoss2 = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_primary) + pIndicators->adjust;

				modifyTradeEasy_DayTrading(BUY, -1, stopLoss, pIndicators->bbsStopPrice_primary, -1, tpMode, currentTime, pIndicators->adjust, FALSE);
			}
			// [Comment removed - encoding corrupted]
			//{
			//	takePrice = adjustTakePrice_Weekly_Swing_Easy(B_HOURLY_RATES, pBase_Indicators->pWeeklyATR/3);
			//	modifyTradeEasy_new(BUY, -1, stopLoss, takePrice, tpMode, FALSE);
			// }
			else
				modifyTradeEasy_new(BUY, -1, stopLoss, -1, tpMode, FALSE); // New day TP change as
		}
	}

	if (orderType == SELL)
	{
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			if ((int)parameter(AUTOBBS_TREND_MODE) == 5) // Day Trading, override the stop loss to primary bbs on the new day.
			{
				// stopLoss2 = fabs(pIndicators->entryPrice - pIndicators->bbsStopPrice_primary) + pIndicators->adjust;
				modifyTradeEasy_DayTrading(SELL, -1, stopLoss, pIndicators->bbsStopPrice_primary, -1, tpMode, currentTime, pIndicators->adjust, FALSE);
			}
			// [Comment removed - encoding corrupted]
			//{
			//	takePrice = adjustTakePrice_Weekly_Swing_Easy(B_HOURLY_RATES, pBase_Indicators->pWeeklyATR / 3);
			//	modifyTradeEasy_new(SELL, -1, stopLoss, takePrice, tpMode, FALSE);
			// }
			else
				modifyTradeEasy_new(SELL, -1, stopLoss, -1, tpMode, FALSE); // New day TP change as
		}
	}
}

static void splitBuyOrders_Limit(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);
	double preClose = iClose(B_DAILY_RATES, 1);
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE)
	{
		// Fin 38.2%
		openPrice = preHigh - (preHigh - preLow) * 0.382 + pIndicators->adjust;

		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		// Fin 50%
		openPrice = preHigh - (preHigh - preLow) * 0.5 + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
	{
		// Pivot
		openPrice = pBase_Indicators->dailyPivot + pIndicators->adjust;
		if (!isSamePriceBuyLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}
}

static void splitSellOrders_Limit(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice = takePrice_primary;
	double openPrice;
	double preHigh = iHigh(B_DAILY_RATES, 1);
	double preLow = iLow(B_DAILY_RATES, 1);
	double preClose = iClose(B_DAILY_RATES, 1);

	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];

	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
	{
		// Fin 38.2%
		openPrice = preLow + (preHigh - preLow) * 0.382 - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 0.5);

		// Fin 50%
		openPrice = preLow + (preHigh - preLow) * 0.5 - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleBuyLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}

	if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
	{
		// Pivot
		openPrice = pBase_Indicators->dailyPivot - pIndicators->adjust;
		if (!isSamePriceSellLimitOrderEasy(openPrice, currentTime, 0.0))
			openSingleSellLimitEasy(openPrice, takePrice, stopLoss, 0, 1);
	}
}

// Long term trades.
// split into 2 trades
// 50% 2:1
// 50% no tp.
static void splitBuyOrders_LongTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pHigh - pIndicators->entryPrice;

	// Pullback over 1/3 range
	if (gap >= pATR / 2)
	{
		// takePrice = 2 * takePrice_primary;
		// openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk /2 );
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

static void splitBuyOrders_Weekly_Beginning(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1:1 ratio
	if (pIndicators->entryPrice <= pBase_Indicators->weeklyR1)
	{
		takePrice = 0;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

// Long term trades.
// split into 2 trades
// 50% 2:1
// 50% no tp.
static void splitSellOrders_LongTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pIndicators->entryPrice - pLow;

	// Pullback over 1/3 range
	if (gap >= pATR / 2)
	{
		// takePrice = 2 * takePrice_primary;
		// openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

static void splitSellOrders_Weekly_Beginning(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// 1:1 ratio
	if (pIndicators->entryPrice >= pBase_Indicators->weeklyS1)
	{
		// takePrice = 2 * takePrice_primary;
		// openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = 0;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
	}
}

// Short & Long term trades
// split into 3 trades
// 50% 1:1
// 25% 2:1
// 25% no tp.
static void splitBuyOrders_MediumTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
	{
		takePrice = takePrice_primary;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
			takePrice = 2 * takePrice_primary;
		else
		{
			takePrice = fabs(pBase_Indicators->dailyR2 - pIndicators->adjust - pIndicators->entryPrice);
			if (takePrice > 2 * takePrice_primary)
				takePrice = 2 * takePrice_primary;
			if (takePrice < takePrice_primary)
				takePrice = takePrice_primary;
		}

		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
		{
			// [Comment removed - encoding corrupted]
			if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
				takePrice = 3 * takePrice_primary;
			else
			{
				takePrice = fabs(pBase_Indicators->dailyR3 - pIndicators->adjust - pIndicators->entryPrice);
				if (takePrice > 3 * takePrice_primary)
					takePrice = 3 * takePrice_primary;
			}

			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
		else
		{
			takePrice = 0;
			openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
	}
}

// Short & Long term trades
// split into 3 trades
// 50% 1:1
// 25% 2:1
// 25% no tp or 3:1
static void splitSellOrders_MediumTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
	{
		// 1:1 ratio
		takePrice = takePrice_primary;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
		{
			takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS2 + pIndicators->adjust));
			if (takePrice > 2 * takePrice_primary)
				takePrice = 2 * takePrice_primary;
			if (takePrice < takePrice_primary)
				takePrice = takePrice_primary;
		}
		else
			takePrice = 2 * takePrice_primary;

		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);

		if ((int)parameter(AUTOBBS_TP_MODE) == 1)
		{
			if (pBase_Indicators->dailyPivot - pIndicators->entryPrice > 0)
			{
				takePrice = fabs(pIndicators->entryPrice - (pBase_Indicators->dailyS3 + pIndicators->adjust));
				if (takePrice > 3 * takePrice_primary)
					takePrice = 3 * takePrice_primary;
			}
			else
				takePrice = 3 * takePrice_primary;

			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
		else
		{
			takePrice = 0;
			openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 4);
		}
	}
}

static void splitBuyOrders_Daily_Swing(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal;
	double gap = pHigh - pIndicators->entryPrice;

	// if (gap >= 3)
	{
		// if (gap * 2 / 3 < 2)
		//	takePrice = 2;
		// else
		//	takePrice = min(gap * 2 / 3, 3);

		takePrice = 3;

		lots_singal = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice);

		// lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		// if (pIndicators->total_lose_pips > takePrice * lots)
		{
			lots = pIndicators->total_lose_pips / takePrice + lots_singal;

			// [Comment removed - encoding corrupted]
			if (lots * takePrice * 100 / pParams->accountInfo.equity > 0.015)
				lots = pParams->accountInfo.equity * 0.015 / 100 / takePrice;
		}

		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitSellOrders_Daily_Swing(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots, lots_singal;
	double gap = pIndicators->entryPrice - pLow;

	// if (gap >= 3)
	{
		// if (gap * 2 / 3 < 2)
		//	takePrice = 2;
		// else
		//	takePrice = min(gap * 2 / 3, 3);

		takePrice = 3;
		lots_singal = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice);
		// lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

		// if (pIndicators->total_lose_pips > takePrice * lots)
		{
			lots = pIndicators->total_lose_pips / takePrice + lots_singal;

			// [Comment removed - encoding corrupted]
			if (lots * takePrice * 100 / pParams->accountInfo.equity > 0.015)
				lots = pParams->accountInfo.equity * 0.015 / 100 / takePrice;
		}

		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitBuyOrders_ATR(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	// double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);
	//  1:1 ratio
	if ((pIndicators->entryPrice <= pBase_Indicators->dailyR1 && (int)pParams->settings[TIMEFRAME] == 15) ||
		(pIndicators->entryPrice <= pBase_Indicators->dailyR2 && (int)pParams->settings[TIMEFRAME] == 5) ||
		(pIndicators->entryPrice <= pBase_Indicators->dailyR2 && (int)pParams->settings[TIMEFRAME] == 30))
	{

		takePrice = 0.8 * atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		takePrice = atr;
		openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);

		// takePrice = atr - (pIndicators->entryPrice - intradayLow);
		// openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		// openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}

// Short term trades only
static void splitBuyOrders_ShortTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pHigh - pIndicators->entryPrice;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime;
	struct tm timeInfo1;
	double riskcap = parameter(AUTOBBS_RISK_CAP);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	// Pullback over 1/3 range

	if (pIndicators->tradeMode == 1 && gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
	{

		takePrice = gap / 3;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk;
		// Cap to 2 % risk
		// if (stopLoss / takePrice * pParams->settings[ACCOUNT_RISK_PERCENT] > riskcap)
		//	lots = lots * riskcap / (stopLoss / takePrice * pParams->settings[ACCOUNT_RISK_PERCENT]);
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}

	{
		if (pIndicators->entryPrice <= pBase_Indicators->dailyR1)
		{
			takePrice = 0;
			if (pIndicators->tradeMode == 1)
				openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
			else
				openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk);
		}
	}
}

static void splitBuyOrders_ShortTerm_ATR_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// [Comment removed - encoding corrupted]
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitSellOrders_ShortTerm_ATR_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pMaxDailyHigh;
	double pLow = pBase_Indicators->pMaxDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// [Comment removed - encoding corrupted]
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitBuyOrders_ShortTerm_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// [Comment removed - encoding corrupted]
	if (down_gap <= pATR / 3)
	{
		takePrice = up_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, BUY, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleLongEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitSellOrders_ShortTerm_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double down_gap = pIndicators->entryPrice - pLow;
	double up_gap = pHigh - pIndicators->entryPrice;

	// [Comment removed - encoding corrupted]
	if (up_gap <= pATR / 3)
	{
		takePrice = down_gap / 4;
		if (up_gap <= pATR / 4)
			pIndicators->risk *= 2;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk * 3;
		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}
}

static void splitBuyOrders_KeyK(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;
	// [Comment removed - encoding corrupted]

	takePrice = takePrice_primary;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 2 * takePrice_primary;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 0;
	openSingleLongEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);
}

static void splitSellOrders_ATR(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double atr, double stopLoss)
{
	double takePrice;
	// double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);

	// [Comment removed - encoding corrupted]
	if ((pIndicators->entryPrice >= pBase_Indicators->dailyS1 && (int)pParams->settings[TIMEFRAME] == 15) ||
		(pIndicators->entryPrice >= pBase_Indicators->dailyS2 && (int)pParams->settings[TIMEFRAME] == 5) ||
		(pIndicators->entryPrice >= pBase_Indicators->dailyS2 && (int)pParams->settings[TIMEFRAME] == 30))
	{
		// takePrice = atr - (intradayHigh - pIndicators->entryPrice);
		takePrice = 0.8 * atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
		takePrice = atr;
		openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
	}
}

// Short term trades only
// split into 3 trades
// 33% 1:1
// 33% 2:1
// 33% no tp or 3:1
static void splitSellOrders_ShortTerm(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	double takePrice;
	double pATR = pBase_Indicators->pDailyATR;
	double pHigh = pBase_Indicators->pDailyHigh;
	double pLow = pBase_Indicators->pDailyLow;
	double lots;
	double gap = pIndicators->entryPrice - pLow;
	int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;

	time_t currentTime;
	struct tm timeInfo1;

	double riskcap = parameter(AUTOBBS_RISK_CAP);

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
	safe_gmtime(&timeInfo1, currentTime);

	// [Comment removed - encoding corrupted]
	if (pIndicators->tradeMode == 1 && gap >= pATR / 2 && fabs(iClose(B_DAILY_RATES, 1) - iClose(B_DAILY_RATES, 2)) < pBase_Indicators->dailyATR)
	{
		takePrice = gap / 3;
		lots = calculateOrderSize(pParams, SELL, pIndicators->entryPrice, takePrice) * pIndicators->risk;

		//// Cap to 2 % risk
		// if (stopLoss / takePrice * pParams->settings[ACCOUNT_RISK_PERCENT] > riskcap)
		//	lots = lots * riskcap / (stopLoss / takePrice * pParams->settings[ACCOUNT_RISK_PERCENT]);

		openSingleShortEasy(takePrice, stopLoss, lots, 0);
	}

	{
		if (pIndicators->entryPrice >= pBase_Indicators->dailyS1)
		{
			takePrice = 0;
			if (pIndicators->tradeMode == 1)
				openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 2);
			else
				openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk);
		}
	}
}

static void splitSellOrders_KeyK(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{
	double takePrice;

	takePrice = takePrice_primary;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 2 * takePrice_primary;
	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);

	takePrice = 0;

	openSingleShortEasy(takePrice, stopLoss, 0, pIndicators->risk / 3);
}

static void splitBuyOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode)
	{
	case 0:
		splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 3:
		splitBuyOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 9:
		splitBuyOrders_KeyK(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 12:
		splitBuyOrders_ATR(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 14:
		splitBuyOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 15:
		splitBuyOrders_ShortTerm_ATR_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	default:
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE)
			splitBuyOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);

		if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
			splitBuyOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	}
}

static void splitSellOrders(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators, double takePrice_primary, double stopLoss)
{

	switch (pIndicators->splitTradeMode)
	{
	case 0:
		splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 9:
		splitSellOrders_KeyK(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 12:
		splitSellOrders_ATR(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 14:
		splitSellOrders_ShortTerm_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	case 15:
		splitSellOrders_ShortTerm_ATR_Hedge(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	default:
		if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE)
			splitSellOrders_LongTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);

		if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
			splitSellOrders_ShortTerm(pParams, pIndicators, pBase_Indicators, takePrice_primary, stopLoss);
		break;
	}
}

static AsirikuyReturnCode handleTradeEntries(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double stopLoss, takePrice_primary = 0;
	int riskcap = (int)parameter(AUTOBBS_RISK_CAP);

	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeEntries() failed. pParams = NULL\n\n\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeEntries() failed. pIndicators = NULL\n\n\n");
		return NULL_POINTER;
	}

	// When the trade is created from 23:45 to 00:20, increase the 3 times of spread for cut over
	if ((hour() == 23 && minute() > 40) || (hour() == 00 && minute() < 20))
		pIndicators->adjust = 3 * pIndicators->adjust;

	stopLoss = fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) + pIndicators->adjust;

	switch (pIndicators->tpMode)
	{
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

// closeAllLimitPreviousDayOrders is declared in base.h and defined in Base.c
// Removed duplicate definition - using the one from Base.c

static AsirikuyReturnCode handleTradeExits(StrategyParams *pParams, Indicators *pIndicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;

	if (pParams == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeExits() failed. pParams = NULL\n\n");
		return NULL_POINTER;
	}

	if (pIndicators == NULL)
	{
		fprintf(stderr, "[CRITICAL] handleTradeExits() failed. pIndicators = NULL\n\n");
		return NULL_POINTER;
	}

	// alwasy close all pre day limit orders.
	closeAllLimitPreviousDayOrders(pParams);

	switch (pIndicators->exitSignal)
	{
	case EXIT_SELL:
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}
		break;
	case EXIT_BUY:
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
		break;
	case EXIT_ALL:
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			closeAllShorts();
		}

		if (totalOpenOrders(pParams, BUY) > 0)
		{
			closeAllLongs();
		}
		break;
	}

	return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	struct tm timeInfo;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int startHour = 0;

	if (strstr(pParams->tradeSymbol, "XAU") != NULL)
		startHour = 1;

	safe_gmtime(&timeInfo, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);
	safe_timeString(timeString, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]);

	// Strategy risk
	pIndicators->strategyMarketVolRisk = caculateStrategyVolRiskEasy(pBase_Indicators->dailyATR);

	pIndicators->strategyRisk = caculateStrategyRiskEasy(FALSE);
	pIndicators->strategyRiskWithoutLockedProfit = caculateStrategyRiskEasy(TRUE);

	pIndicators->riskPNL = caculateStrategyPNLEasy(FALSE);
	pIndicators->riskPNLWithoutLockedProfit = caculateStrategyPNLEasy(TRUE);

	switch ((int)parameter(AUTOBBS_TREND_MODE))
	{
	case 9:
		workoutExecutionTrend_Auto(pParams, pIndicators, pBase_Indicators);
		break;
	case 14:
		workoutExecutionTrend_Auto_Hedge(pParams, pIndicators, pBase_Indicators);
		break;
	case 15:
		workoutExecutionTrend_XAUUSD_DayTrading(pParams, pIndicators, pBase_Indicators);
		break;
	}

	if ((BOOL)pParams->settings[IS_BACKTESTING] == FALSE && pParams->accountInfo.totalOpenTradeRiskPercent < parameter(AUTOBBS_MAX_ACCOUNT_RISK) * -1) // if account risk is more than 3%, stop entring trades.
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, Over max riks %lf, skip this entry signal=%d\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, parameter(AUTOBBS_MAX_ACCOUNT_RISK), pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	// Filter out macro trend
	if (pIndicators->side != 0 && pIndicators->entrySignal != 0 && pIndicators->side != pIndicators->entrySignal)
	{
		fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s,Againt Side =%ld, skip this entry signal=%d\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->side, pIndicators->entrySignal);
		pIndicators->entrySignal = 0;
	}

	//// Dont enter trade on the new day bar, it is too risky and not reliable.
	// if (pIndicators->entrySignal != 0 && timeInfo.tm_hour == startHour && timeInfo.tm_min == 0)
	//{
	//	fprintf(stderr, ("[WARNING] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s Not allowed to trade on the firt bar of new day, skip this entry signal=%d\n\n",
	//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->entrySignal);
	//	pIndicators->entrySignal = 0;
	// }

	return SUCCESS;
}

// 1% risk
static AsirikuyReturnCode workoutExecutionTrend_KeyK(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	AsirikuyReturnCode returnCode = SUCCESS;
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	double keyHigh, keyLow;

	pIndicators->risk = 2;
	pIndicators->splitTradeMode = 9;
	pIndicators->tpMode = 0;

	keyHigh = iHigh(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);
	keyLow = iLow(B_PRIMARY_RATES, shift0Index - pBase_Indicators->intradyIndex);

	if (pBase_Indicators->intradayTrend == 1)
	{
		if (pIndicators->side >= 0)
		{
			pIndicators->executionTrend = 1;
			pIndicators->exitSignal = EXIT_SELL;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = keyLow;
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyLow) >= 0.5 * pBase_Indicators->dailyATR)
				pIndicators->risk = 1;

			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = 1;
			}
		}
		else
			pBase_Indicators->intradayTrend = 0;
	}

	if (pBase_Indicators->intradayTrend == -1)
	{
		if (pIndicators->side <= 0)
		{
			pIndicators->executionTrend = -1;
			pIndicators->exitSignal = EXIT_BUY;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = keyHigh;
			if (fabs(pIndicators->stopLossPrice - pBase_Indicators->dailyHigh) >= 0.5 * pBase_Indicators->dailyATR)
				pIndicators->risk = 1;

			if (pBase_Indicators->intradyIndex == shift1Index)
			{
				pIndicators->entrySignal = -1;
			}
		}
		else
			pBase_Indicators->intradayTrend = 0;
	}

	return returnCode;
}

static AsirikuyReturnCode workoutExecutionTrend_Auto_Hedge(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double stopLoss;
	double riskPNL;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	safe_timeString(timeString, currentTime);

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
	case RANGE_PHASE:
		// Execute hedge strategy for range-bound trading
		workoutExecutionTrend_Hedge(pParams, pIndicators, pBase_Indicators);
		break;
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		break;
	}

	profitManagement(pParams, pIndicators, pBase_Indicators);

	return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_Auto(StrategyParams *pParams, Indicators *pIndicators, Base_Indicators *pBase_Indicators)
{
	double stopLoss;
	double riskPNL;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	time_t currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
	struct tm timeInfo1;

	double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);
	int execution_tf, euro_index_rate, count;
	double ATR0_EURO = 10;
	double preHigh, preLow, preClose;
	double pivot, S3, R3;

	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	pIndicators->strategyMaxRisk = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;
	pIndicators->limitRiskPNL = pParams->settings[AUTOBBS_MAX_STRATEGY_RISK] * -1;

	// Check out if Key K is triggered. If yes, take piority with other strategies.,
	// TODO Need to test riskPNL parameter, disabled for now
	if ((int)parameter(AUTOBBS_KEYK) == 1)
	{
		workoutExecutionTrend_KeyK(pParams, pIndicators, pBase_Indicators);
		if (pIndicators->entrySignal != 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == 1 && pBase_Indicators->dailyTrend_Phase < 0)
			return SUCCESS;
		if (pBase_Indicators->intradayTrend == -1 && pBase_Indicators->dailyTrend_Phase > 0)
			return SUCCESS;
	}

	switch (pBase_Indicators->dailyTrend_Phase)
	{
	case BEGINNING_UP_PHASE:
	case BEGINNING_DOWN_PHASE:
		// workoutExecutionTrend_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators);
		workoutExecutionTrend_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal
			workoutExecutionTrend_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, FALSE);
		pIndicators->splitTradeMode = 0;

		break;
	case MIDDLE_UP_PHASE:
	case MIDDLE_DOWN_PHASE:
		workoutExecutionTrend_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, FALSE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal
			workoutExecutionTrend_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
		if (pIndicators->entrySignal == 0) // Try 15M BBS if no entry signal
			workoutExecutionTrend_KongJian(pParams, pIndicators, pBase_Indicators);
		if ((int)parameter(AUTOBBS_LONG_SHORT_MODE) == 1)
			pIndicators->splitTradeMode = 0;
		else
			pIndicators->splitTradeMode = 6;

		break;
	case MIDDLE_UP_RETREAT_PHASE:
	case MIDDLE_DOWN_RETREAT_PHASE:
		workoutExecutionTrend_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators);
		break;
	case RANGE_PHASE:
		if ((int)parameter(AUTOBBS_RANGE) == 1)
		{
			if (pBase_Indicators->weeklyMATrend == RANGE)
				pIndicators->exitSignal = EXIT_ALL;
			else
			{
				// closeAllWithNegativeEasy(1, currentTime, 3);

				if (totalOpenOrders(pParams, BUY) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->dailyS) + pIndicators->adjust;
					modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, FALSE);
				}
				if (totalOpenOrders(pParams, SELL) > 0)
				{
					stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->dailyR) + pIndicators->adjust;
					modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, FALSE);
				}
			}
		}
		else
			pIndicators->exitSignal = EXIT_ALL;
		break;
	}

	if (timeInfo1.tm_hour >= 17)
	{
		execution_tf = (int)pParams->settings[TIMEFRAME];
		euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

		count = (17 - 1) * (60 / execution_tf) - 1;
		iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
		pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
		pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
		pIndicators->euro_open = close_prev1;
		pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);

		ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);
	}

	return SUCCESS;
}

// XAUUSD Daily Stop Check - checks if orders should be closed after 15 minutes
static void XAUUSD_Daily_Stop_Check(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
	int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
	int shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
	time_t currentTime, openTime;
	struct tm timeInfo1, timeInfo2;
	char timeString[MAX_TIME_STRING_SIZE] = "";
	double diffMins;

	currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
	safe_gmtime(&timeInfo1, currentTime);
	safe_timeString(timeString, currentTime);

	if (pParams->orderInfo[0].ticket != 0 && pParams->orderInfo[0].isOpen == TRUE)
	{
		// Must pass 15M
		openTime = pParams->orderInfo[0].openTime;
		safe_gmtime(&timeInfo2, pParams->orderInfo[0].openTime);

		diffMins = difftime(currentTime, openTime) / 60;

		if (diffMins == 15)
		{
			if (pParams->orderInfo[0].type == BUY && pIndicators->bbsTrend_secondary == -1)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld\n\n",
										(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_BUY;
				}
			}
			if (pParams->orderInfo[0].type == SELL && pIndicators->bbsTrend_secondary == 1)
			{
				pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
				pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
				if (pIndicators->lossTimes > 0 && pIndicators->winTimes == 0)
				{
					fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, lossTimes = %ld,winTimes = %ld,orderType=%ld, bbsTrend_secondary=%ld\n\n",
										(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->lossTimes, pIndicators->winTimes, pParams->orderInfo[0].type, pIndicators->bbsTrend_secondary);
					pIndicators->exitSignal = EXIT_SELL;
				}
			}
		}
	}
}

/*
XAUUSD 15M
1/2 risk  
1/2ATR,

1/2ATRstop 

*/
static AsirikuyReturnCode workoutExecutionTrend_XAUUSD_Daily_Swing(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);
double ATR0 = iAtr(B_DAILY_RATES, 1, 0);
int signal = 0;
double intradayHigh_exclude_current_bar = intradayHigh, intradayLow_exclude_current_bar = intradayLow;
int count;
double openOrderHigh, openOrderLow;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
safe_gmtime(&timeInfo1, currentTime);

// closeAllWithNegativeEasy(1, currentTime, 3);

pIndicators->splitTradeMode = 16;
pIndicators->risk = 1;
pIndicators->tpMode = 0;

if (pBase_Indicators->pDailyPredictATR < 10)
	return SUCCESS;

count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
if (count > 1)
	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh_exclude_current_bar, &intradayLow_exclude_current_bar);
// iSRLevels_close(pParams, pIndicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh_exclude_current_bar, &intradayLow_exclude_current_bar);

// if (ATR0 >= pBase_Indicators->pDailyATR / 2)

if (ATR0 < 5)
{
	if (timeInfo1.tm_hour == 7 && timeInfo1.tm_min < 3 && hasOpenOrder() == FALSE) // 7am
	{
		if (pIndicators->bbsTrend_primary == 1)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.2;
			pIndicators->entrySignal = 1;
		}
		else if (pIndicators->bbsTrend_primary == -1)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.2;
			pIndicators->entrySignal = -1;
		}
		return SUCCESS;
	}
}
else
{
	// pIndicators->risk = 0.5;
	if (hasOpenOrder() == FALSE)
	{
		if (pParams->bidAsk.ask[0] - intradayLow > 5) // Buy signal
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.2;

			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 10 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;
		}

		if (intradayHigh - pParams->bidAsk.bid[0] > 5) // Sell signal
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.2;
			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 10 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}
	}
	else
	{

		if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == SELL && pParams->bidAsk.ask[0] > openOrderHigh) // Buy signal
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - pBase_Indicators->dailyATR * 1.2;

			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = 1;
			}
			pIndicators->exitSignal = EXIT_SELL;
		}

		if (getLastestOpenOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow) == BUY && pParams->bidAsk.bid[0] < openOrderLow) // Sell signal
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + pBase_Indicators->dailyATR * 1.2;
			pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
			pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
			if (pIndicators->lossTimes < 5 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 23)
			{
				pIndicators->risk = pow(2, pIndicators->lossTimes);
				pIndicators->entrySignal = -1;
			}

			pIndicators->exitSignal = EXIT_BUY;
		}
	}
}
return SUCCESS;
}

// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// UP signal: Low < pivot and Close > pivot
// DOWN: signal: High> pivot and Close < pivot
// PrimaryRate is 1H
static AsirikuyReturnCode workoutExecutionTrend_WeeklyPivot(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

double stopLoss;
double preHigh = iHigh(B_PRIMARY_RATES, 1);
double preLow = iLow(B_PRIMARY_RATES, 1);
double preClose = iClose(B_PRIMARY_RATES, 1);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

if (pBase_Indicators->weeklyTrend_Phase == RANGE_PHASE)
	pIndicators->executionTrend = 0;
else if (pBase_Indicators->weeklyTrend > 0)
	pIndicators->executionTrend = 1;
else if (pBase_Indicators->weeklyTrend < 0)
	pIndicators->executionTrend = -1;
else
	pIndicators->executionTrend = 0;

closeAllWithNegativeEasy(2, currentTime, 3);

if (pIndicators->executionTrend == 0)
{
	if (totalOpenOrders(pParams, BUY) > 0)
	{
		stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->weeklyS) + pIndicators->adjust;
		modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, FALSE);
	}
	if (totalOpenOrders(pParams, SELL) > 0)
	{
		stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->weeklyR) + pIndicators->adjust;
		modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, FALSE);
	}
}

// [Comment removed - encoding corrupted]
if (pIndicators->executionTrend == 1)
{
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;

	if (pIndicators->entryPrice > pIndicators->stopLossPrice + pIndicators->adjust && preLow < pBase_Indicators->weeklyPivot && preClose > pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pIndicators->executionTrend == -1)
{
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;

	if (pIndicators->entryPrice < pIndicators->stopLossPrice - pIndicators->adjust && preHigh > pBase_Indicators->weeklyPivot && preClose < pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
		//&& pBase_Indicators->weeklyTrend_Phase < 0
		&& !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_Hedge(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);

double down_gap = pIndicators->entryPrice - pBase_Indicators->pDailyLow;
double up_gap = pBase_Indicators->pDailyHigh - pIndicators->entryPrice;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

// closeAllWithNegativeEasy(1, currentTime, 3);
if (timeInfo1.tm_hour == 23 && timeInfo1.tm_min > 25)
{
	pIndicators->exitSignal = EXIT_ALL;
	return SUCCESS;
}

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 14;

// Asia hour, risk 50%
if (timeInfo1.tm_hour <= 8)
	pIndicators->risk = 0.5;

if (pBase_Indicators->dailyTrend_Phase > 0)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}
else if (pBase_Indicators->dailyTrend_Phase < 0)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	// Hedge strategy execution
	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}
else
{
	// [Comment removed - encoding corrupted]
	if (up_gap <= pBase_Indicators->pDailyATR / 3)
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

		if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			pIndicators->entrySignal = -1;

		pIndicators->exitSignal = EXIT_BUY;
	}
	else if (down_gap <= pBase_Indicators->pDailyATR / 3)
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pBase_Indicators->dailyS;
		pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

		// [Comment removed - encoding corrupted]
		if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && timeInfo1.tm_hour < 23 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
			pIndicators->entrySignal = 1;

		pIndicators->exitSignal = EXIT_SELL;
	}
}
return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_KongJian(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);
double intradayClose = iClose(B_DAILY_RATES, 0), intradayHigh = iHigh(B_DAILY_RATES, 0), intradayLow = iLow(B_DAILY_RATES, 0);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 0;

pIndicators->tradeMode = 1;

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pParams->bidAsk.ask[0] - intradayLow > pBase_Indicators->dailyATR / 3 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
	{
		pIndicators->entrySignal = 1;
	}
	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE)
{

	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);
	if (intradayHigh - pParams->bidAsk.bid[0] > pBase_Indicators->dailyATR / 3 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime) && timeInfo1.tm_hour < 23)
	{
		pIndicators->entrySignal = -1;
	}
	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

// It should always use 15M
static AsirikuyReturnCode workoutExecutionTrend_BEGINNING_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
// shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 3;

pIndicators->tradeMode = 1;

// if (timeInfo1.tm_hour < 5)
//	pIndicators->risk = 0.5;

if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust && ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = 1;
	// Override the entry signal if the current daily low has been lower than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust && ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = -1;

	// Override the entry signal if the current daily high has been higher than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_BUY;
}
return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_MIDDLE_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
double currentLow = iLow(B_DAILY_RATES, 0);
double currentHigh = iHigh(B_DAILY_RATES, 0);

double stopLoss;
double preHigh = iHigh(B_SECONDARY_RATES, 1);
double preLow = iLow(B_SECONDARY_RATES, 1);
double preClose = iClose(B_SECONDARY_RATES, 1);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
// shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 6;

pIndicators->tradeMode = 1;

// if (timeInfo1.tm_hour < 5)
//	pIndicators->risk = 0.5;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust && preLow < pBase_Indicators->dailyPivot && preClose > pBase_Indicators->dailyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
		//&& bbsTrend == 1
		&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = 1;

	// Override the entry signal if the current daily low has been lower than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_UP_PHASE && currentLow < pBase_Indicators->dailyS - pIndicators->adjust && pIndicators->entrySignal == 1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->dailyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust && preHigh > pBase_Indicators->dailyPivot && preClose < pBase_Indicators->dailyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22
		//&& bbsTrend == -1
		&& ((pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3)) || (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_PHASE && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))))
		pIndicators->entrySignal = -1;

	// Override the entry signal if the current daily high has been higher than stop loss price.
	if (pBase_Indicators->dailyTrend_Phase == BEGINNING_DOWN_PHASE && currentHigh > pBase_Indicators->dailyS + pIndicators->adjust && pIndicators->entrySignal == -1)
		pIndicators->entrySignal = 0;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
static AsirikuyReturnCode workoutExecutionTrend_MIDDLE_RETREAT_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);
// shift1Index = filterExcutionTF_ByTime(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 0;

pIndicators->tradeMode = 1;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->dailyTrend_Phase == MIDDLE_UP_RETREAT_PHASE)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->dailyS + pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666 && !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->dailyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->dailyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->dailyS - pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->dailyS) <= pBase_Indicators->dailyATR * 0.666
		//&& !isSameDaySamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 3, currentTime)
	)
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_WeeklyAuto(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double stopLoss;

// Check out if Key K is triggered. If yes, take piority with other strategies.
if ((int)parameter(AUTOBBS_KEYK) == 1)
{
	workoutExecutionTrend_KeyK(pParams, pIndicators, pBase_Indicators);
	if (pIndicators->entrySignal != 0)
		return SUCCESS;
	if (pBase_Indicators->intradayTrend == 1 && pBase_Indicators->weeklyTrend_Phase < 0)
		return SUCCESS;
	if (pBase_Indicators->intradayTrend == -1 && pBase_Indicators->weeklyTrend_Phase > 0)
		return SUCCESS;
}

switch (pBase_Indicators->weeklyTrend_Phase)
{
case BEGINNING_UP_PHASE:
case BEGINNING_DOWN_PHASE:
	workoutExecutionTrend_Weekly_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal
		workoutExecutionTrend_Weekly_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	pIndicators->splitTradeMode = 11;
	break;
case MIDDLE_UP_PHASE:
case MIDDLE_DOWN_PHASE:
	workoutExecutionTrend_Weekly_MIDDLE_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	if (pIndicators->entrySignal == 0) // Try 1H BBS if no entry signal
		workoutExecutionTrend_Weekly_BEGINNING_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	pIndicators->splitTradeMode = 7;
	break;
case MIDDLE_UP_RETREAT_PHASE:
case MIDDLE_DOWN_RETREAT_PHASE:
	workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(pParams, pIndicators, pBase_Indicators, TRUE);
	break;
case RANGE_PHASE:
	if ((int)parameter(AUTOBBS_RANGE) == 1)
	{
		if (totalOpenOrders(pParams, BUY) > 0)
		{
			stopLoss = fabs(pParams->bidAsk.ask[0] - pBase_Indicators->weeklyS) + pIndicators->adjust;
			modifyTradeEasy_new(BUY, -1, stopLoss, -1, 0, FALSE);
		}
		if (totalOpenOrders(pParams, SELL) > 0)
		{
			stopLoss = fabs(pParams->bidAsk.bid[0] - pBase_Indicators->weeklyR) + pIndicators->adjust;
			modifyTradeEasy_new(SELL, -1, stopLoss, -1, 0, FALSE);
		}
	}
	else
		pIndicators->exitSignal = EXIT_ALL;
	break;
}

profitManagementWeekly(pParams, pIndicators, pBase_Indicators);

return SUCCESS;
}

// Weekly retreat strategy, only for EUR, AUD, crossing currencies
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
// [Comment removed - encoding corrupted]
static AsirikuyReturnCode workoutExecutionTrend_WeeklyRetreat(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

if (pBase_Indicators->weeklyTrend_Phase > 1 && pBase_Indicators->dailyTrend_Phase <= 0)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_primary == 1 && pIndicators->bbsIndex_primary == shift1Index_primary && pIndicators->entryPrice > pBase_Indicators->ma4H200M + pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->ma4H200M) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase < -1 && pBase_Indicators->dailyTrend_Phase > 0)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_primary == -1 && pIndicators->bbsIndex_primary == shift1Index_primary && pIndicators->entryPrice < pIndicators->stopLossPrice - pIndicators->adjust && fabs(pIndicators->entryPrice - pIndicators->stopLossPrice) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index]))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_BEGINNING_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
// int    shift1Index = pParams->ratesBuffers->rates[B_HOURLY_RATES].info.arraySize - 2;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
char timeString1[MAX_TIME_STRING_SIZE] = "";

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(2, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 10;

if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase == BEGINNING_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}
return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index_Primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

double stopLoss;

double preHigh = iHigh(B_SECONDARY_RATES, 1);
double preLow = iLow(B_SECONDARY_RATES, 1);
double preClose = iClose(B_SECONDARY_RATES, 1);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_Primary];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(2, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 11;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust && preLow < pBase_Indicators->weeklyPivot && preClose > pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust && preHigh > pBase_Indicators->weeklyPivot && preClose < pBase_Indicators->weeklyPivot && timeInfo1.tm_hour >= 2 && timeInfo1.tm_hour <= 22 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

AsirikuyReturnCode workoutExecutionTrend_Weekly_MIDDLE_RETREAT_PHASE(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored)
{
double closestR;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

closeAllWithNegativeEasy(2, currentTime, 3);

shift1Index = filterExcutionTF(pParams, pIndicators, pBase_Indicators);

pIndicators->risk = 1;
pIndicators->tpMode = 0;
pIndicators->splitTradeMode = 5;

// [Comment removed - encoding corrupted]
if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_UP_RETREAT_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase > 0))
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = min(pIndicators->stopLossPrice, pIndicators->entryPrice - pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == 1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice > pBase_Indicators->weeklyS + pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->weeklyS) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = 1;

	pIndicators->exitSignal = EXIT_SELL;
}

if (pBase_Indicators->weeklyTrend_Phase == MIDDLE_DOWN_RETREAT_PHASE || (ignored && pBase_Indicators->weeklyTrend_Phase < 0))
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->weeklyS;
	pIndicators->stopLossPrice = max(pIndicators->stopLossPrice, pIndicators->entryPrice + pBase_Indicators->weeklyATR);

	if (pIndicators->bbsTrend_excution == -1 && pIndicators->bbsIndex_excution == shift1Index && pIndicators->entryPrice < pBase_Indicators->weeklyS - pIndicators->adjust && fabs(pIndicators->entryPrice - pBase_Indicators->weeklyS) <= pBase_Indicators->weeklyATR * 0.666 && !isSameWeekSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->weeklyATR / 3, currentTime))
		pIndicators->entrySignal = -1;

	pIndicators->exitSignal = EXIT_BUY;
}

return SUCCESS;
}

static AsirikuyReturnCode workoutExecutionTrend_ManualBBS(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR, movement = 0;
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
int shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int dailyTrend;
time_t currentTime;
struct tm timeInfo1;
int bbsTrend, bbsIndex;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);

// [Comment removed - encoding corrupted]
if (pBase_Indicators->dailyTrend_Phase == RANGE_PHASE)
	dailyTrend = 0;
else if (pBase_Indicators->dailyTrend > 0)
	dailyTrend = 1;
else if (pBase_Indicators->dailyTrend < 0)
	dailyTrend = -1;
else
	dailyTrend = 0;

pIndicators->splitTradeMode = 19;
pIndicators->tpMode = 1;

if (pIndicators->bbsTrend_primary == 1)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;

	if (pIndicators->bbsIndex_primary == shift1Index && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
	{
		pIndicators->entrySignal = 1;
	}
	pIndicators->exitSignal = EXIT_BUY;
}

if (pIndicators->bbsTrend_primary == -1)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pBase_Indicators->dailyS;

	if (pIndicators->bbsIndex_primary == shift1Index && !isSamePricePendingOrderEasy(pIndicators->entryPrice, pBase_Indicators->dailyATR / 4))
	{
		pIndicators->entrySignal = -1;
	}
	pIndicators->exitSignal = EXIT_SELL;
}
}

/*

1.ATR > 20
2.Close -  Close > 10
3.> 7 or 8?
4.3
5.10
6.15MA too close,  21H MA 
*/
static BOOL XAUUSD_DayTrading_Allow_Trade(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
int shift0Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int count, asia_index_rate, euro_index_rate, execution_tf;
time_t currentTime;
struct tm timeInfo1;
double preHigh, preLow, preClose;
double pivot, S3, R3;
char timeString[MAX_TIME_STRING_SIZE] = "";
double close_prev1 = iClose(B_DAILY_RATES, 1), close_prev2 = iClose(B_DAILY_RATES, 2);

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index];
safe_gmtime(&timeInfo1, currentTime);
safe_timeString(timeString, currentTime);

// execution_tf = pIndicators->executionRateTF;
execution_tf = (int)pParams->settings[TIMEFRAME];

if (timeInfo1.tm_hour < 10)
	return FALSE;

asia_index_rate = shift1Index - ((timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

count = (10 - 1) * (60 / execution_tf) - 1;
iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, asia_index_rate, count, &(pIndicators->asia_high), &(pIndicators->asia_low));
pIndicators->asia_low = min(close_prev1, pIndicators->asia_low);
pIndicators->asia_high = max(close_prev1, pIndicators->asia_high);
pIndicators->asia_open = close_prev1;
pIndicators->asia_close = iClose(B_PRIMARY_RATES, asia_index_rate);

if (timeInfo1.tm_hour >= 17) // [Comment removed - encoding corrupted]
{
	euro_index_rate = shift1Index - ((timeInfo1.tm_hour - 17) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf));

	count = (17 - 1) * (60 / execution_tf) - 1;
	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, euro_index_rate, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
	pIndicators->euro_low = min(close_prev1, pIndicators->euro_low);
	pIndicators->euro_high = max(close_prev1, pIndicators->euro_high);
	pIndicators->euro_open = close_prev1;
	pIndicators->euro_close = iClose(B_PRIMARY_RATES, euro_index_rate);
}
//// Workout Euro time bar( 10 - 15)
// if (timeInfo1.tm_hour >= 10 && timeInfo1.tm_hour < 17)
//{
//	count = (timeInfo1.tm_hour - 10) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->euro_high), &(pIndicators->euro_low));
//	pIndicators->euro_open = pIndicators->asia_close;
//	pIndicators->euro_close = iClose(B_PRIMARY_RATES, 1);
// }

// if (timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour < 24)
//{
//	count = (timeInfo1.tm_hour - 15) * (60 / execution_tf) + (int)(timeInfo1.tm_min / execution_tf) - 1;
//	iSRLevels(pParams, pIndicators, B_PRIMARY_RATES, shift1Index, count, &(pIndicators->us_high), &(pIndicators->us_low));
//	pIndicators->us_open = pIndicators->euro_close;
//	pIndicators->us_close = iClose(B_PRIMARY_RATES, 1);
// }

if (fabs(pIndicators->asia_high - pIndicators->asia_low) >= 7.5)
	return FALSE;

if (iAtr(B_DAILY_RATES, 1, 1) >= 20)
	return FALSE;
if (fabs(close_prev1 - close_prev2) >= 10)
	return FALSE;

preHigh = iHigh(B_DAILY_RATES, 2);
preLow = iLow(B_DAILY_RATES, 2);
preClose = iClose(B_DAILY_RATES, 2);

pivot = (preHigh + preLow + preClose) / 3;
S3 = preLow - 2 * (preHigh - pivot);
R3 = preHigh + 2 * (pivot - preLow);

if (close_prev1 > R3 || close_prev1 < S3)
	return FALSE;

// [Comment removed - encoding corrupted]
// if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1)
//{
//	fprintf(stderr, ("[INFO] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s No Trade on  Non Farm PayRoll day\n\n",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
//	return FALSE;
// }

return TRUE;
}

static void XAUUSD_DayTrading_Entry(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators, OrderType orderType, double ATR0_EURO, double stopLoss, double Range)
{
int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1;
time_t currentTime;
struct tm timeInfo1;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
safe_gmtime(&timeInfo1, currentTime);

if (orderType == SELL)
{
	pIndicators->executionTrend = 1;
	pIndicators->entryPrice = pParams->bidAsk.ask[0];
	pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;

	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
	if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
	{
		pIndicators->risk = pow(2, pIndicators->lossTimes);
		pIndicators->entrySignal = 1;
	}
	pIndicators->exitSignal = EXIT_SELL;
}

if (orderType == BUY)
{
	pIndicators->executionTrend = -1;
	pIndicators->entryPrice = pParams->bidAsk.bid[0];
	pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
	pIndicators->lossTimes = getLossTimesInDayEasy(currentTime, &pIndicators->total_lose_pips);
	pIndicators->winTimes = getWinTimesInDayEasy(currentTime);
	if (ATR0_EURO > Range && pIndicators->lossTimes < 3 && pIndicators->winTimes == 0 && timeInfo1.tm_hour < 22)
	{
		pIndicators->risk = pow(2, pIndicators->lossTimes);
		pIndicators->entrySignal = -1;
	}

	pIndicators->exitSignal = EXIT_BUY;
}
}

/*
BBS15

1- 10am

 80p,


1.ATR > 20
2.Close -  Close > 10
3.> 7 or 8?
4.3
5.S3 or R3


MA15(50) > MA15(200): UP
MA15(50) < MA15(200): DOWN

:
1. 10, 
2. 10 
2.1 5M
2.2 15MBBS  


0.3 risk


2330

 5M
 BBS15stoploss, BBS

*/
static AsirikuyReturnCode workoutExecutionTrend_XAUUSD_DayTrading(StrategyParams * pParams, Indicators * pIndicators, Base_Indicators * pBase_Indicators)
{
double closestR;
int shift0Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 1, shift1Index_primary = pParams->ratesBuffers->rates[B_PRIMARY_RATES].info.arraySize - 2;
int shift0Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 1, shift1Index_secondary = pParams->ratesBuffers->rates[B_SECONDARY_RATES].info.arraySize - 2;
time_t currentTime;
struct tm timeInfo1;
double intradayClose = iClose(B_PRIMARY_RATES, 0), intradayHigh, intradayLow, primary_close_pre1 = iClose(B_PRIMARY_RATES, 1);
double ATR0, Range, ATR0_EURO = 10;
double close_prev1 = iClose(B_DAILY_RATES, 1);
double openOrderHigh, openOrderLow, HalfPoint;

int count;
double adjust = 0.15;
double stopLoss = pBase_Indicators->dailyATR * 1.2;
BOOL isOpen;
OrderType side;

char timeString[MAX_TIME_STRING_SIZE] = "";
int MATrend_1H, noNewTradeSignal = 0;

currentTime = pParams->ratesBuffers->rates[B_PRIMARY_RATES].time[shift0Index_primary];
safe_gmtime(&timeInfo1, currentTime);

Range = 8;
stopLoss = 10;

safe_timeString(timeString, currentTime);

closeAllWithNegativeEasy(1, currentTime, 3);

pIndicators->splitTradeMode = 16;
pIndicators->risk = 1;
pIndicators->tpMode = 0;

pBase_Indicators->maTrend = getMATrend(2, B_SECONDARY_RATES, 1);

// if ((int)parameter(AUTOBBS_IS_AUTO_MODE) && XAUUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
if (XAUUSD_DayTrading_Allow_Trade(pParams, pIndicators, pBase_Indicators) == FALSE)
	return SUCCESS;

count = (timeInfo1.tm_hour - 1) * (60 / (int)pParams->settings[TIMEFRAME]) + (int)(timeInfo1.tm_min / (int)pParams->settings[TIMEFRAME]) - 1;
if (count > 1)
	iSRLevels(pParams, pBase_Indicators, B_PRIMARY_RATES, shift1Index_primary - 1, count, &intradayHigh, &intradayLow);

intradayLow = min(close_prev1, intradayLow);
intradayHigh = max(close_prev1, intradayHigh);
ATR0 = fabs(intradayHigh - intradayLow);

fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ATR0 = %lf,IntraDaily High = %lf, Low=%lf, Close=%lf\n\n",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0, intradayHigh, intradayLow, intradayClose);

fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, asia_high = %lf,asia_low = %lf, asia_close=%lf\n\n",
					(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->asia_high, pIndicators->asia_low, pIndicators->asia_close);

if (timeInfo1.tm_hour >= 17)
{
	ATR0_EURO = fabs(pIndicators->euro_high - pIndicators->euro_low);

	fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, ATR0_EURO =%lf,euro_high = %lf,euro_low = %lf, euro_close=%lf\n\n",
						(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, ATR0_EURO, pIndicators->euro_high, pIndicators->euro_low, pIndicators->euro_close);
}

if (ATR0_EURO < Range
	//|| ATR0 > pBase_Indicators->pDailyMaxATR
)
{
	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
		pIndicators->exitSignal = EXIT_ALL;

	// closeAllWithNegativeEasy(5, currentTime, 3);
	return SUCCESS;
}

// secondary rate is 15M , priarmy rate is 5M
if (hasSameDayOrderEasy(currentTime, &isOpen) == FALSE) // [Comment removed - encoding corrupted]
{
	if (ATR0_EURO > Range && pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && timeInfo1.tm_hour < 22) // [Comment removed - encoding corrupted]
	{
		pIndicators->executionTrend = 1;
		pIndicators->entryPrice = pParams->bidAsk.ask[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
		pIndicators->entrySignal = 1;
	}
	else if (ATR0_EURO > Range && pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && timeInfo1.tm_hour < 22) // [Comment removed - encoding corrupted]
	{
		pIndicators->executionTrend = -1;
		pIndicators->entryPrice = pParams->bidAsk.bid[0];
		pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
		pIndicators->entrySignal = -1;
	}
	else // [Comment removed - encoding corrupted]
	{
		if (ATR0_EURO > Range && primary_close_pre1 > pIndicators->asia_high && timeInfo1.tm_hour < 22)
		{
			pIndicators->executionTrend = 1;
			pIndicators->entryPrice = pParams->bidAsk.ask[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice - stopLoss;
			pIndicators->entrySignal = 1;
		}

		if (ATR0_EURO > Range && primary_close_pre1 < pIndicators->asia_low && timeInfo1.tm_hour < 22)
		{
			pIndicators->executionTrend = -1;
			pIndicators->entryPrice = pParams->bidAsk.bid[0];
			pIndicators->stopLossPrice = pIndicators->entryPrice + stopLoss;
			pIndicators->entrySignal = -1;
		}
	}
}
else
{

	side = getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen);
	if (side == SELL)
	{
		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, Side = SELL isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

		// [Comment removed - encoding corrupted]
		// [Comment removed - encoding corrupted]
		// [Comment removed - encoding corrupted]
		// [Comment removed - encoding corrupted]

		if (ATR0 >= Range &&
			(pBase_Indicators->maTrend > 0 && pIndicators->bbsTrend_secondary == 1 && pIndicators->bbsIndex_secondary == shift1Index_secondary) // [Comment removed - encoding corrupted]
		)
		{
			XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
		}
		else
		{

			if (primary_close_pre1 - adjust > openOrderHigh)
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
			}

			if (isOpen == FALSE)
			{
				if (primary_close_pre1 + adjust < openOrderLow)
				{
					XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
					if (pIndicators->entrySignal != 0)
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, Enter a sell trade again.\n\n",
											(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
				}
			}
		}
	}

	if (side == BUY)
	{
		fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, Side = BUY isOpen=%ld, ATR0 = %lf,openOrderHigh = %lf,openOrderLow = %lf\n\n",
							(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, isOpen, ATR0, openOrderHigh, openOrderLow);

		if (ATR0 >= Range &&
			(pBase_Indicators->maTrend < 0 && pIndicators->bbsTrend_secondary == -1 && pIndicators->bbsIndex_secondary == shift1Index_secondary) // [Comment removed - encoding corrupted]
		)
		{
			XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
		}
		else
		{
			if (primary_close_pre1 + adjust < openOrderLow)
			{
				XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, BUY, ATR0_EURO, stopLoss, Range);
			}

			if (isOpen == FALSE)
			{
				if (primary_close_pre1 - adjust > openOrderHigh)
				{
					XAUUSD_DayTrading_Entry(pParams, pIndicators, pBase_Indicators, SELL, ATR0_EURO, stopLoss, Range);
					if (pIndicators->entrySignal != 0)
						fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, Enter a buy trade again.\n\n",
											(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
				}
			}
		}
	}

	if (pIndicators->entrySignal == 0 && timeInfo1.tm_hour >= 15 && timeInfo1.tm_hour <= 17)
		XAUUSD_Daily_Stop_Check(pParams, pIndicators, pBase_Indicators);

	// [Comment removed - encoding corrupted]
	if (pIndicators->entrySignal == 0 && timeInfo1.tm_hour >= 21)
	{
		if (isOpen == TRUE)
		{
			if ((pIndicators->bbsTrend_secondary == -1 && side == BUY) || (pIndicators->bbsTrend_secondary == 1 && side == SELL))
			{
				fprintf(stderr, "[WARNING] System InstanceID = %d, BarTime = %s, Exiting a trade on after 21H side = %ld.\n\n",
									(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, side);

				pIndicators->exitSignal = EXIT_ALL;
			}
		}
	}
}

MATrend_1H = getMATrend(4, B_HOURLY_RATES, 1);

// if (timeInfo1.tm_hour == 1 && timeInfo1.tm_min < 3)
if (pIndicators->entrySignal != 0)
{
	// too close
	if (timeInfo1.tm_wday == 5 && timeInfo1.tm_mday - 7 < 1 && abs(pBase_Indicators->maTrend) <= 1 &&
		((pBase_Indicators->maTrend > 0 && MATrend_1H < 0) || (pBase_Indicators->maTrend < 0 && MATrend_1H > 0)))
	{
		noNewTradeSignal = 1;
                fprintf(stderr, "[INFO] System InstanceID = %d, BarTime = %s, 15M Trend = %ld, 1H Trend = %ld,noNewTradeSignal=%ld\n\n\n",
                                                        (int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pBase_Indicators->maTrend, MATrend_1H, noNewTradeSignal);
		pIndicators->entrySignal = 0;
	}
}

// if (pIndicators->entrySignal == 0 && ATR0 > 8)
//{
//	HalfPoint = intradayHigh - ATR0 * 2 /3;

//	if (hasSameDayOrderEasy(currentTime, &isOpen) == TRUE && isOpen == TRUE)
//	{
//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == BUY
//			&& primary_close_pre1 < HalfPoint)
//		{
//			fprintf(stderr, ("[WARNING] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a buy order\n\n",
//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString) ;
//			pIndicators->exitSignal = EXIT_BUY;
//		}
//

//		if (getLastestOrderTypeEasy(B_PRIMARY_RATES, &openOrderHigh, &openOrderLow, &isOpen) == SELL
//			&& primary_close_pre1 > HalfPoint)
//		{
//			fprintf(stderr, ("[WARNING] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Exiting a sell order\n\n",
//				(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString);
//			pIndicators->exitSignal = EXIT_SELL;
//		}
//	}
//}

// if (pIndicators->entrySignal != 0 && pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity  > 0.01)
//{
//	fprintf(stderr, ("[WARNING] (PAN_CHAR_T*)"System InstanceID = %d, BarTime = %s, Daily Loss=%lf, skip this entry signal=%d\n\n",
//		(int)pParams->settings[STRATEGY_INSTANCE_ID], timeString, pIndicators->total_lose_pips * 100 / pParams->accountInfo.equity, pIndicators->entrySignal);
//	pIndicators->entrySignal = 0;
// }

return SUCCESS;
}
