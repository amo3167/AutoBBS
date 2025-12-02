/** @file  tester.h
 @brief Backtesting execution
 */

/** int runTest (struct ASTRateInfo *rates, void (*testUpdate)(double percentageOfTestCompleted));
 @brief performs a Strategy test
 @param rates Array of HSTRateInfo struct contanining the OHLC data
 @param testUpdate Callback function for updates on the strategy test
 @param testFinished Callback function for notifiying the end of the test
 @param error description if any
 */

typedef enum signalType {
	SIGNAL_BUY = 0,
	SIGNAL_SELL = 1,
	SIGNAL_MODIFY = 2,
	SIGNAL_CLOSE = 3,
	SIGNAL_CLOSE_SL = 4,
	SIGNAL_CLOSE_TP = 5
} SignalType;

typedef struct tradeSignal_t {
	int testId; 
	int no;
	int time; 
	SignalType type;
	int orderId;
	double lots;
	double price;
	double sl;
	double tp;
	double profit;
	double balance; 
} TradeSignal;

typedef struct testResult_t {
	int testId;
	int totalTrades; 
	double cagr;
	double sharpe;
	double martin;
	double risk_reward;
	double winning;
	double finalBalance;
	double maxDDDepth;
	double maxDDLength;
	double pf;
	double r2;
	double ulcerIndex;
	double avgTradeDuration;
	int numShorts;
	int numLongs;
	double yearsTraded;
	char   symbol[5000]; 
} TestResult;

typedef struct testSettings_t{
	double spread;
	int fromDate;
	int toDate;
	int is_calculate_expectancy;
} TestSettings;

typedef struct statistic_item_t
{
  int	 time;
  double balance;
  double profit;
} StatisticItem;

#pragma once

#include "CTesterFrameworkDefines.h"

#define MAX_ORDERS 200
#define MIN_STATISTICS_SIZE 200

#ifdef __cplusplus
extern "C" {
#endif

TestResult __stdcall runPortfolioTest (
	int				testId,
	double**		pInSettings,
	char**			pInTradeSymbol,
    char*			pInAccountCurrency,
    char*			pInBrokerName,
	char*			pInRefBrokerName,
    double**			pInAccountInfo,
	TestSettings	*testSettings,
	CRatesInfo**	pRatesInfo,
	int				numCandles,
	int				numSystems,
    ASTRates***		pRates,
	double			minLotSize,
	void			(*testUpdate)(int testId, double percentageOfTestCompleted, COrderInfo lastOrder, double currentBalance, char* symbol), 
	void			(*testFinished)(TestResult testResults), 
	void			(*signalUpdate)(TradeSignal signal)
	);

#ifdef __cplusplus
} /* extern "C" */
#endif