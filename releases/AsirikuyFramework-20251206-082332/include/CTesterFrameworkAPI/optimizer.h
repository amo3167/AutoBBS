#pragma once

#include "CTesterFrameworkDefines.h"
#include "tester.h"

typedef struct optimization_param_t
{
  int	 index;
  double start;
  double step;
  double stop;
} OptimizationParam;

typedef enum optimization_type {
	OPTI_BRUTE_FORCE = 0,
	OPTI_GENETIC = 1
} OptimizationType;

typedef enum optimization_goal {
	OPTI_GOAL_PROFIT		= 0,
	OPTI_GOAL_MAX_DD		= 1,
	OPTI_GOAL_MAX_DD_LENGTH	= 2,
	OPTI_GOAL_PF			= 3,
	OPTI_GOAL_R2			= 4,
	OPTI_GOAL_ULCER_INDEX	= 5,
	OPTI_GOAL_SHARPE    	= 6,
	OPTI_GOAL_CAGR_TO_MAXDD = 7,
	OPTI_GOAL_CAGR			= 8
} OptimizationGoal;

typedef struct genetic_optimization_settings_t
{
	int population;
	double crossoverProbability;
	double mutationProbability;
	double migrationProbability;
	int evolutionaryMode;	//0 = Darwin, 1 = Lamarck Parents, 2 = Lamarck Children, 3 = Lamarck All, 4 = Baldwin Parents, 8 = Baldwin Children 12 = Baldwin All
	int elitismMode;		//0 = Unknown, 1 = Parents survive, 2 = One parent survives, 3 = Parents die, 4 = Rescore Parents
	int mutationMode;		//0 = Single point drift, 1 = Single point randomize, 2 = Multipoint, 3 = All point	
	int crossoverMode;		//0 = Single points, 1 = Double points, 3 = Mean, 4 = Mixing, 5 = Allele mixing
	int maxGenerations;		//0 no limit
	int stopIfConverged;
	int discardAssymetricSets;
	int minTradesAYear;
	int optimizationGoal;
} GeneticOptimizationSettings;

#ifdef __cplusplus
extern "C" {
#endif

void __stdcall stopOptimization(
);

int __stdcall runOptimizationMultipleSymbols(
	OptimizationParam	*optimizationParams,
	int					numOptimizedParams,
	OptimizationType	optimizationType,
	GeneticOptimizationSettings optimizationSettings,
	int					numThreads,
	double*				pInSettings,
	char**				pInTradeSymbol,
    char*				pInAccountCurrency,
    char*				pInBrokerName,
	char*				pInRefBrokerName,
    double*				pInAccountInfo,
	TestSettings*		testSettings,
	CRatesInfo**		pRatesInfo,
	int				    numCandles,
	int					numSymbols,
    ASTRates***			pRates,
	double				minLotSize,
	void				(*optimizationUpdate)(TestResult testResult, double* settings, int numSettings), 
	void				(*optimizationFinished)(), 
	char				**error
);

#ifdef __cplusplus
} /* extern "C" */
#endif