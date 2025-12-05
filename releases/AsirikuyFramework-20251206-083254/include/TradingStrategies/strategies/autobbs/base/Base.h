
#ifndef BASE_H_
#define BASE_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bar_t
{
	BAR_UNKNOWN = 0,
	BULL = 1,
	BEAR = 2,
	STAR = 3
	
}bar;

typedef enum trend_t
{
	UP = 3,
	UP_NORMAL = 2,
	UP_WEAK = 1,
	DOWN = -3,
	DOWN_NORMAL = -2,
	DOWN_WEAK = -1,
	RANGE = 0,
	UNKNOWN = -100
}trend;

typedef enum trend_phase_t
{	
	RANGE_PHASE = 0,
	BEGINNING_UP_PHASE = 1,
	MIDDLE_UP_PHASE = 2,
	MIDDLE_UP_RETREAT_PHASE = 3,
	FINNAL_UP_PHASE = 4,
	BEGINNING_DOWN_PHASE = -1,
	MIDDLE_DOWN_PHASE = -2,
	MIDDLE_DOWN_RETREAT_PHASE = -3,
	FINNAL_DOWN_PHASE = -4
}trend_phase;

typedef enum predict_trend_t
{
	VOL_IN_RANGE = 0,
	VOL_BREAK_UP = 1,
	VOL_BREAK_DOWN = 2
	
}predict_trend;

typedef enum baseRatesIndexes_t
{
	B_PRIMARY_RATES = 0,
	B_SECONDARY_RATES = 1,
	B_HOURLY_RATES = 2,
	B_FOURHOURLY_RATES = 3,
	B_DAILY_RATES = 4,
	B_WEEKLY_RATES = 5,
	B_MONTHLY_RATES = 6
} BaseRatesIndexes;

typedef struct base_Indicators_t
{
	//Intraday 
	int intradayTrend;
	int intradyIndex;

	int maTrend;
	int ma_Signal;

	int mACDInTrend;
	int shellingtonInTrend;
	int flatTrend;
	
	int dailyTrend;
	int dailyTrend_Phase;	
	int daily3RulesTrend;
	int dailyMATrend;
	int dailyHLTrend;
	int weeklyTrend;
	int weeklyTrend_Phase;
	int weekly3RulesTrend;
	int weeklyMATrend;
	int weeklyHLTrend;
	double dailyS;
	double dailyR;
	double dailyTP;
	double dailyHigh;
	double dailyLow;
	double weeklyS;
	double weeklyR;
	double weeklyTP;
	double weeklyHigh;
	double weeklyLow;
	double monthlyHigh;
	double monthlyLow;
	double monthlyS;
	double monthlyR;
	double monthlyTP;
	double ma1H200M;
	double ma1H50M;
	double ma4H200M;
	double ma4H50M;
	double maDaily200M;
	double maDaily50M;

	double dailyATR;
	double weeklyATR;
	double monthlyATR;
	
	double pDailyATR;
	double pDailyMaxATR;
	double pDailyMinATR;
	double pDailyPredictATR;	
	double pDailyHigh;
	double pDailyLow;
	int    pDailyTrend;
	double pMaxDailyHigh;
	double pMaxDailyLow;


	double pWeeklyATR;
	double pWeeklyPredictATR;
	double pWeeklyPredictMaxATR;
	double pWeeklyHigh;
	double pWeeklyLow;
	int    pWeeklyTrend;
	double pMaxWeeklyHigh;
	double pMaxWeeklyLow;


	// Daily pivot point
	double dailyPivot;
	double dailyS1;
	double dailyR1;
	double dailyS2;
	double dailyR2;
	double dailyS3;
	double dailyR3;

	//Weekly pivot points
	double weeklyPivot;
	double weeklyS1;
	double weeklyR1;
	double weeklyS2;
	double weeklyR2;
	double weeklyS3;
	double weeklyR3;

	//Monthly  pivot points
	double monthlyPivot;
	double monthlyS1;
	double monthlyR1;
	double monthlyS2;
	double monthlyR2;
	double monthlyS3;
	double monthlyR3;

	int   weeklyMAMode; // 0: 4H 200M, 1: Weekly Rate

	int   strategy_mode; // 0: daily strategy; 1: weekly strategy

} Base_Indicators;

typedef enum exitSignal_t
{
	EXIT_BUY = 0,
	EXIT_SELL = 1,
	EXIT_ALL = 2,
	EXIT_BUY_SHORT = 3,
	EXIT_SELL_SHORT = 4,
	EXIT_NONE = 3
}ExitSignal;

typedef enum orderStatus_t
{
	RESET = 0,
	PENDING = 1,
	DONE = 2,
	PILOTING = 3,
	CANCELLED = 4
}OrderStatus;

typedef struct order_info_t
{
	int orderNumber;	
	OrderType type;
	double openPrice;
	double stopLossPrice;
	double takeProfitPrice;
	OrderStatus orderStatus;
	time_t timeStamp;

} Order_Info;

typedef struct order_turning_info_t
{	
	OrderType type;
	BOOL isTurning;

} Order_Turning_Info;

AsirikuyReturnCode runBase(StrategyParams* pParams, Base_Indicators * pIndicators);
AsirikuyReturnCode base_ModifyOrders(StrategyParams* pParams, OrderType orderType, double stopLoss, double takePrice);
int getMATrend(double iATR, int ratesArrayIndex, int index);
int getMATrendBase(int rateShort,int rateLong,double iATR, int ratesArrayIndex, int index);
AsirikuyReturnCode iSRLevels(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow);
AsirikuyReturnCode iSRLevels_close(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow);
AsirikuyReturnCode iSRLevels_WithIndex(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow, int *pHighIndex, int *pLowIndex);

void closeAllLimitPreviousDayOrders(StrategyParams* pParams);

BOOL isNewDay(StrategyParams* pParams, time_t currentTime);

AsirikuyReturnCode iTrend3Rules(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend,int index);
AsirikuyReturnCode iTrend3Rules_preDays(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend, int preDays, int index);
AsirikuyReturnCode iTrend_MA(double iATR, int ratesArrayIndex, int *trend);
int iTrendMA_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int signal);
AsirikuyReturnCode iTrend_HL(int ratesArrayIndex, int *trend, int index);
AsirikuyReturnCode iTrend_MA_WeeklyBar_For4H(double iATR, int *trend);
AsirikuyReturnCode iTrend_MA_DailyBar_For1H(double iATR, int *trend,int index);
AsirikuyReturnCode iTrend3Rules_LookBack(StrategyParams* pParams, Base_Indicators* pIndicators, int ratesArrayIndex, int shift, int * pTrend);
AsirikuyReturnCode iTrend_HL_preDays(int ratesArrayIndex, int *trend, int preDays,int index);

void predictDailyATR(StrategyParams* pParams, Base_Indicators* pIndicators);
void predictWeeklyATR(StrategyParams* pParams, Base_Indicators* pIndicators);
void predictWeeklyATR_LongerTerm(StrategyParams* pParams, Base_Indicators* pIndicators);

AsirikuyReturnCode loadWeeklyIndicators(StrategyParams* pParams, Base_Indicators* pIndicators);
AsirikuyReturnCode workoutWeeklyTrend(StrategyParams* pParams, Base_Indicators* pIndicators);
AsirikuyReturnCode workoutDailyTrend(StrategyParams* pParams, Base_Indicators* pIndicators);

int getMATrend_Signal(int ratesArrayIndex);
int getMATrend_SignalBase(int rateShort, int rateLong, int ratesArrayIndex, int maxBars);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif 
