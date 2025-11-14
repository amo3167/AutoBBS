#include "CTesterFrameworkDefines.h"
#include "gaul.h"
#include "Precompiled.h"
#include <stdlib.h>
//#include <vld.h>

#ifdef _OPENMP
	#include <omp.h>
#else
	typedef int omp_int_t;
	__inline omp_int_t omp_get_thread_num() { return 0;}
	__inline omp_int_t omp_get_max_threads() { return 1;}
	__inline omp_int_t omp_get_num_procs() { return 1; }
#endif

#if HAVE_MPI == 1
	#include "mpi.h"
#endif

#define MAXIMUM_PARAMETER_COMBINATIONS 10000000
static int stopOpti = 0;

//Global copy of the parameters
OptimizationParam	*globalOptimizationParams;
int					globalNumOptimizedParams, globalNumThreads, globalNumCandles, globalExecUnderMPI;				
OptimizationType	globalOptimizationType;
GeneticOptimizationSettings globalOptimizationSettings;
double				*globalSettings, *globalAccountInfo, globalInitialBalance;
TestSettings*		globalTestSettings;
char				*globalTradeSymbol, *globalAccountCurrency, *globalBrokerName, *globalRefBrokerName;
double				globalMinLotSize;
CRatesInfo*			globalRatesInfo;
CRatesInfo**		globalMultiRatesInfo;
ASTRates***			globalRates;
char**				globalMultiTradeSymbol;
void				(*globalOptimizationUpdate)(TestResult testResult, double* settings, int numSettings);
int					currentIteration = 0, globalNumSymbols;
double				generationDifferences[5] = {-1};				


static boolean generationHook(int generation, population *pop)
{
	int i;
	double averageDifference, standardDeviation, generationDifferencesFull ;

	//Stop optmization if we get to max number of generations
	if (globalOptimizationSettings.maxGenerations > 0 && generation > globalOptimizationSettings.maxGenerations - 1)
	{
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"Max num of generations (%d) reached!", globalOptimizationSettings.maxGenerations);
		return FALSE;
	}

	//Stop optimization if population converges
	if (globalOptimizationSettings.stopIfConverged)
	{
		generationDifferencesFull = 1;
		averageDifference = 0;
		standardDeviation = 0;

		for (i=4;i>0;i--){
			generationDifferences[i] = generationDifferences[i-1];
		}

		generationDifferences[0] = ga_get_entity_from_rank(pop,0)->fitness;

		for (i=0;i<5;i++){
			averageDifference += generationDifferences[i]/5;
		}

		for (i=0;i<5;i++){
			standardDeviation += (generationDifferences[i]-averageDifference)*(generationDifferences[i]-averageDifference)/5 ;
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Past %d generation -> best fit = %lf", i, generationDifferences[i]);
			if(generationDifferences[i] < 0) generationDifferencesFull = 0;
		}

		standardDeviation = sqrt(standardDeviation);

		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Best fit average = %lf, stdDev = %lf", averageDifference, standardDeviation);

		if ( standardDeviation < 0.05*averageDifference && generationDifferencesFull == 1){
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"Solutions have converged!");
		return FALSE;
		}
	
	}

	//Stop optimization if stopOptimization was called
	if (stopOpti == 1)
	{
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"stopOptimization was called -> Stoping optimization");
		return FALSE;
	}
	
	pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"Generation %d started", generation +1);
	return TRUE;	/* TRUE indicates that evolution should continue. */
}

//Calculates total number of combinations for given optimization parameters
int getParameterSetsNumber (int startIndex, int numOptimizedParams, OptimizationParam *optimizationParams){
	int i, numSteps = 1;

	for(i=0; i<numOptimizedParams; i++){
		numSteps = numSteps * ((int)((optimizationParams[i].stop - optimizationParams[i].start) / optimizationParams[i].step) + 1);
	}
	return numSteps;
}

//Maps an optimization value to a valid value from the 1-100 scale
double mapParamValue (int value, OptimizationParam optParam){
	double *posibleValues;
	int i;
	int numPosibleValues = (int)fabs((optParam.stop-optParam.start)/optParam.step) + 1;

	posibleValues = (double*)malloc(sizeof(double) * numPosibleValues);
	for (i=0; i<numPosibleValues; i++){
		posibleValues[i] = optParam.start + optParam.step * i;
	}
	return posibleValues[(int)numPosibleValues * value / 100];
}

/* Calculate fitness function */
boolean testFitnessMultipleSymbols(population *pop, entity *entity)
{
	double **localSettings, *currentSet, chromosomeMappedValue;
	TestSettings *localTestSettings;
	CRatesInfo **localRatesInfo;
	AccountInfo **localAccountInfo;
	TestResult testResult = {0};
	int k, n, chromosomeValue, localCurrentIteration;
	int testId;
	char **localSymbol;
	ASTRates ***localRates;

	#pragma omp critical
	{
		currentIteration++;
		localCurrentIteration = currentIteration;
	}
	
	#ifdef _OPENMP
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Starting Iteration %d on thread %d", localCurrentIteration, omp_get_thread_num());	
	#else
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Starting Iteration %d", localCurrentIteration);
	#endif

	entity->fitness = 0.0;

	for (n=0;n<globalNumSymbols;n++)
	{

	//Make copies of modified arrays and variables
	localSettings    = (double**)malloc(1 * sizeof(double*));
	localSettings[0] = (double*)malloc(64 * sizeof(double));
	memcpy(localSettings[0], globalSettings, 64 * sizeof(double));

	localSymbol = (char**)malloc( 1*sizeof(char*));
	localSymbol[0] = (char*)malloc(256*sizeof(char*));
	strcpy( localSymbol[0], globalMultiTradeSymbol[n] );

	currentSet = (double*)malloc(globalNumOptimizedParams * 2 * sizeof(double));

	localRatesInfo    = (CRatesInfo**)malloc(1 * sizeof(CRatesInfo*));
	localRatesInfo[0] = (CRatesInfo*)malloc(10 * sizeof(CRatesInfo));
	memcpy(localRatesInfo[0], globalMultiRatesInfo[n], 10 * sizeof(CRatesInfo));

	localRates = (ASTRates ***)malloc(1 * sizeof(ASTRates**));
	localRates[0] = (ASTRates **)malloc(10 * sizeof(ASTRates*));

	pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"SavingRates");
	for (k=0;k<10;k++){
		if (localRatesInfo[0][k].totalBarsRequired > 0) {
			localRates[0][k] = (ASTRates *)malloc(globalNumCandles * sizeof(ASTRates));
			memcpy (localRates[0][k], globalRates[n][k], globalNumCandles * sizeof(ASTRates));
		}
	}
	pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"endSavingRates");

	localAccountInfo = (AccountInfo**)malloc(1 * sizeof(AccountInfo*));
	localAccountInfo[0] = (AccountInfo*)malloc(1 * sizeof(AccountInfo));
	memcpy (localAccountInfo[0], globalAccountInfo, sizeof(AccountInfo));

	localTestSettings = (TestSettings*)malloc(1 * sizeof(TestSettings));
	memcpy (localTestSettings, &globalTestSettings[n], sizeof(TestSettings));

	for (k = 0; k < pop->len_chromosomes; k++)
    {
		chromosomeValue = ((int*)entity->chromosome[0])[k];
		chromosomeMappedValue = mapParamValue(chromosomeValue, globalOptimizationParams[k]);
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Iteration: %d. Gen %d mapped to %lf from gen value %d", localCurrentIteration, k, chromosomeMappedValue, chromosomeValue);
		localSettings[0][globalOptimizationParams[k].index] = chromosomeMappedValue;
		currentSet[k*2] = (double)globalOptimizationParams[k].index;
		currentSet[k*2+1] = chromosomeMappedValue;
    }

	testId = 1;

	#ifdef _OPENMP
	testId = omp_get_thread_num();	
	#endif

	localSettings[0][STRATEGY_INSTANCE_ID] = (testId+1)+2*(n+1);
	
	testResult = runPortfolioTest(testId+1, localSettings, localSymbol, globalAccountCurrency, globalBrokerName, globalRefBrokerName, (double **)localAccountInfo, 
						localTestSettings, localRatesInfo, globalNumCandles, 1, localRates, globalMinLotSize, NULL, NULL, NULL);

	if (testResult.r2 < 0) testResult.r2 = 0;

	globalOptimizationUpdate(testResult, currentSet, globalNumOptimizedParams);	

	switch (globalOptimizationSettings.optimizationGoal){
		case (OPTI_GOAL_PROFIT):
			entity->fitness += testResult.finalBalance-localAccountInfo[0]->balance;
			break;
		case (OPTI_GOAL_MAX_DD):
			entity->fitness += globalInitialBalance/testResult.maxDDDepth; //fitness calculated against initial balance
			break;
		case (OPTI_GOAL_MAX_DD_LENGTH):
			entity->fitness += (double)(testResult.yearsTraded*365)/testResult.maxDDLength; //fitness calculated against total days of test
			break;
		case (OPTI_GOAL_PF):
			entity->fitness += testResult.pf;
			break;
		case (OPTI_GOAL_R2):
			entity->fitness += testResult.r2;
			break;
		case (OPTI_GOAL_ULCER_INDEX):
			entity->fitness += 10/testResult.ulcerIndex;
			break;
		case (OPTI_GOAL_SHARPE):
			entity->fitness += testResult.sharpe;
			break;
		case (OPTI_GOAL_CAGR_TO_MAXDD):
			entity->fitness += testResult.cagr/testResult.maxDDDepth;
			break;
		default:
			pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Optimization Goal %d not supported", globalOptimizationSettings.optimizationGoal);
			return false;
	}

	if (testResult.finalBalance-localAccountInfo[0]->balance < 0){ 
		entity->fitness = 0.0;
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Iteration %d gave negative balance ... killing it", localCurrentIteration);
	}
	
	if (globalOptimizationSettings.discardAssymetricSets && fabs(testResult.numShorts - testResult.numLongs) > 0.5*min(testResult.numShorts, testResult.numLongs)){
		entity->fitness = 0.0;
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Iteration %d gave assymetric results (%d longs %d shorts) ... killing it", localCurrentIteration, testResult.numShorts, testResult.numLongs);
	}

	if (testResult.totalTrades/testResult.yearsTraded < globalOptimizationSettings.minTradesAYear){
		entity->fitness = 0.0;
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Iteration %d gave less than %d trades a year in average... killing it", localCurrentIteration, globalOptimizationSettings.minTradesAYear);
	}

	for (k=0;k<10;k++){
			if (localRatesInfo[0][k].totalBarsRequired > 0) {
			free(localRates[0][k]); localRates[0][k] = NULL;
			}
	}

	free(localTestSettings); localTestSettings = NULL;
	free(localSymbol[0]); localSymbol[0] = NULL;
	free(localSymbol); localSymbol = NULL;
	free(localSettings[0]); localSettings[0] = NULL;
	free(localSettings); localSettings = NULL;
	free(currentSet); currentSet = NULL;
	free(localRatesInfo[0]); localRatesInfo[0] = NULL;
	free(localRatesInfo); localRatesInfo = NULL;
	free(localRates[0]); localRates[0] = NULL;
	free(localRates); localRates = NULL;
	free(localAccountInfo[0]); localAccountInfo[0] = NULL;
	free(localAccountInfo); localAccountInfo = NULL;
	}

	return TRUE;
}


void __stdcall stopOptimization(){
	stopOpti = 1;
};

static void sleepMilliseconds(int milliseconds)
{
#if defined _WIN32 || defined _WIN64
  Sleep(milliseconds);
#elif defined __APPLE__ || defined __linux__
  usleep(milliseconds * 1000);
#else
  #error "Unsupported operating system"
#endif
}

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
)
{

	
	//General variables
	char error_t[MAX_ERROR_LENGTH];
	int i, j, finishCtr = 0, n, p;
	int numParamsInSet;
	int testId;

	//Brute force variables
	int numCombinations, maxSteps, steps/*, localNumCandles*/;
	double *sets, *combination, elem, **localSettings;
	/*ASTRates *localRates;*/
	CRatesInfo **localRatesInfo;
	AccountInfo **localAccountInfo;
	TestSettings *localTestSettings;
	ASTRates ***localRates;
	char **localSymbol;
	TestResult testResult;
	double *currentSet;

	//Genetic variables
	population *pop = NULL;
	void (*crossoverFunction)(population *pop, entity *mother, entity *father, entity *daughter, entity *son);
	void (*mutateFunction)(population *pop, entity *mother, entity *daughter);

	int myId = 0, numProcs = 1;

	stopOpti = 0;

	#if HAVE_MPI == 1
	//MPI variables
	globalExecUnderMPI = getenv("OMPI_COMM_WORLD_RANK") != NULL || getenv("PMI_RANK") != NULL;

	if (globalExecUnderMPI){	
		MPI_Comm_size(MPI_COMM_WORLD ,&numProcs);
		MPI_Comm_rank(MPI_COMM_WORLD ,&myId);
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"MPI enabled. Using %d cores", numProcs);
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"MPI thread %d up & running", myId);
	}
	#endif

	#ifdef _OPENMP
		#if defined _MSC_VER
		_putenv("OMP_STACKSIZE=256M");
		#else
		putenv("OMP_STACKSIZE=256M");
		#endif
		if(numThreads > omp_get_num_procs()) numThreads = omp_get_num_procs();
		omp_set_num_threads(numThreads);
		pantheios_logprintf(PANTHEIOS_SEV_NOTICE, (PAN_CHAR_T*)"OpenMP enabled. Using %d cores", numThreads);
	#endif

	numParamsInSet = numOptimizedParams;

	if (optimizationType == OPTI_BRUTE_FORCE){
		//Parameter space generation
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Calculating possible parameter combinations.");
		numCombinations = getParameterSetsNumber(0, numOptimizedParams, optimizationParams);
		pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Number of parameter combinations is %d", numCombinations);

		if (numCombinations > MAXIMUM_PARAMETER_COMBINATIONS){
			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Number of parameter combinations is too large (exceeds 10 million). Try a genetic optimization instead.");
			if(optimizationFinished != NULL) optimizationFinished();
			return true;
		}

		combination = (double*)malloc(numOptimizedParams * sizeof(double));
		sets = (double*)malloc(numCombinations * numOptimizedParams * sizeof(double));
		n = 0;
		
		while (finishCtr < numOptimizedParams) {
			finishCtr = 0;
			j = n;
			for(i=0; i<numOptimizedParams; i++){
				maxSteps = (int)((optimizationParams[i].stop - optimizationParams[i].start) / optimizationParams[i].step) + 1;
				steps = j % maxSteps;
				j = (int)(j / maxSteps);
				elem = optimizationParams[i].start + steps * optimizationParams[i].step;
				if(j > 0 && steps == 0) finishCtr++;
				combination[i] = elem;
			}
			if (finishCtr < numOptimizedParams){
				for(p=0; p<numOptimizedParams; p++){
					sets[n*numOptimizedParams + p] = combination[p];
				}
			}
			n++;
		}

		pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*)"Finished parameter generation, starting runs.");
		//Run the optimization for each set
		#pragma omp parallel for shared(pRates, pInSettings, pInAccountInfo, pRatesInfo) private(p, localTestSettings, localSettings, localRates, localSymbol, localAccountInfo, testResult, currentSet, localRatesInfo, n)
		
		testId = 1;

		for (i = 0 + myId; i<numCombinations; i += numProcs){
			//Stop optimization if stopOptimization was called
			if (stopOpti == 0)
			{
				#ifdef _OPENMP
					pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Starting Iteration %d on thread %d", i, omp_get_thread_num());	
				#else
					pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"Starting Iteration %d", i);
				#endif			
				

				for ( n=0; n< numSymbols; n++){

				localSettings = (double**)malloc(1 * sizeof(double*));
				localSettings[0] = (double*)malloc(64 * sizeof(double));
				memcpy(localSettings[0], pInSettings, 64 * sizeof(double));

				currentSet = (double*)malloc(numOptimizedParams * 2 * sizeof(double));
			
				localRatesInfo    = (CRatesInfo**)malloc(1 * sizeof(CRatesInfo*));
				localRatesInfo[0] = (CRatesInfo*)malloc(10 * sizeof(CRatesInfo));
				memcpy(localRatesInfo[0], pRatesInfo[n], 10 * sizeof(CRatesInfo));

				localSymbol = (char**)malloc(1 * sizeof(char*));
				localSymbol[0] = (char*)malloc(256*sizeof(char*));
				strcpy( localSymbol[0], pInTradeSymbol[n] );

				localRates    = (ASTRates ***)malloc(1 * sizeof(ASTRates**));
				localRates[0] = (ASTRates **)malloc(10 * sizeof(ASTRates*));
						
				for(p=0;p<10;p++){
					localRates[0][p] = (ASTRates *)malloc(numCandles * sizeof(ASTRates));
					
					if (localRatesInfo[0][p].totalBarsRequired > 0) {
					memcpy (localRates[0][p], pRates[n][p], numCandles * sizeof(ASTRates));
					}
				}

				localAccountInfo = (AccountInfo**)malloc(1 * sizeof(AccountInfo*));
				localAccountInfo[0] = (AccountInfo*)malloc(1 * sizeof(AccountInfo));
				memcpy (localAccountInfo[0], pInAccountInfo, sizeof(AccountInfo));

				localTestSettings = (TestSettings*)malloc(1*sizeof(TestSettings));
				memcpy (localTestSettings, &testSettings[n], sizeof(TestSettings));

				for(p=0; p<numOptimizedParams; p++){
					localSettings[0][optimizationParams[p].index] = sets[i*numOptimizedParams+p];
					currentSet[p*2] = (double)optimizationParams[p].index;
					currentSet[p*2+1] = (double)sets[i*numOptimizedParams+p];
					pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"localSettings[0][optimizationParams[p].index]= %lf, currentSet[p*2] =%lf,currentSet[p*2+1]=%lf", 
						localSettings[0][optimizationParams[p].index],currentSet[p*2],currentSet[p*2+1]);
				}

				//testId = 1;

				#ifdef _OPENMP
				testId = omp_get_thread_num();	
				#endif

				localSettings[0][STRATEGY_INSTANCE_ID] = (testId+1)+2*(n+1);

				pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*)"localSettings[0][ADDITIONAL_PARAM_8]= %lf", localSettings[0][ADDITIONAL_PARAM_8]);

				testResult = runPortfolioTest(testId++, localSettings, localSymbol, pInAccountCurrency, pInBrokerName, pInRefBrokerName, (double **)localAccountInfo, 
									localTestSettings, localRatesInfo, numCandles, 1, localRates, minLotSize, NULL, NULL, NULL);

				if(optimizationUpdate != NULL) optimizationUpdate(testResult, currentSet, numParamsInSet);

				for (p=0;p<10;p++){
						free(localRates[0][p]); localRates[0][p] = NULL;
					}

					free(localTestSettings); localTestSettings = NULL;
					free(localSymbol[0]); localSymbol[0] = NULL;
					free(localSymbol); localSymbol = NULL;
					free(localSettings[0]); localSettings[0] = NULL;
					free(localSettings); localSettings = NULL;
					free(currentSet); currentSet = NULL;
					free(localRatesInfo[0]); localRatesInfo[0] = NULL;
					free(localRatesInfo); localRatesInfo = NULL;
					free(localRates[0]); localRates[0] = NULL;
					free(localRates); localRates = NULL;
					free(localAccountInfo[0]); localAccountInfo[0] = NULL;
					free(localAccountInfo); localAccountInfo = NULL;

				}
			}

		}

		if(optimizationFinished != NULL) optimizationFinished();
		free(combination); combination = NULL;
		free(sets); sets = NULL;

		return true;
	}
	else if(optimizationType == OPTI_GENETIC){
		random_seed((int)(time(NULL)));
		log_init(LOG_NONE, NULL, NULL, FALSE);

		globalNumSymbols = numSymbols;

		//Make a global copy of the parameters
		globalSettings = (double*)malloc(64 * sizeof(double));
		memcpy(globalSettings, pInSettings, 64 * sizeof(double));
	
		
		globalMultiRatesInfo = (CRatesInfo**)malloc(numSymbols * sizeof(CRatesInfo*));
		globalRates = (ASTRates ***)malloc(numSymbols * sizeof(ASTRates**));

		for (n=0;n<numSymbols;n++){	

		globalMultiRatesInfo[n] = (CRatesInfo*)malloc(10 * sizeof(CRatesInfo));
		memcpy(globalMultiRatesInfo[n], pRatesInfo[n], 10 * sizeof(CRatesInfo));

		globalRates[n] = (ASTRates **)malloc(10 * sizeof(ASTRates));

			for(i=0;i<10;i++){
				if (globalMultiRatesInfo[n][i].totalBarsRequired > 0) {
				globalRates[n][i] = (ASTRates *)malloc(numCandles * sizeof(ASTRates));
				memcpy (globalRates[n][i], pRates[n][i], numCandles * sizeof(ASTRates));
				}
			}
		}

		globalAccountInfo = (double*)malloc(sizeof(AccountInfo));
		memcpy (globalAccountInfo, pInAccountInfo, sizeof(AccountInfo));
		globalOptimizationParams = (OptimizationParam*)malloc(sizeof (OptimizationParam) * numOptimizedParams);
		memcpy (globalOptimizationParams, optimizationParams, sizeof (OptimizationParam) * numOptimizedParams);

		globalTestSettings = (TestSettings*)malloc(numSymbols*sizeof(TestSettings));
		memcpy (globalTestSettings, testSettings, numSymbols*sizeof(TestSettings));
		
		globalNumThreads = numThreads;
		globalNumOptimizedParams = numOptimizedParams;
		globalNumCandles = numCandles;

		globalMultiTradeSymbol = (char **)malloc(numSymbols * sizeof(char*));

		for (n=0;n<numSymbols;n++){
		globalMultiTradeSymbol[n] = pInTradeSymbol[n];
		}

		globalAccountCurrency = pInAccountCurrency;
		globalMinLotSize = minLotSize;
		globalBrokerName = pInBrokerName;
		globalRefBrokerName = pInRefBrokerName;
		globalOptimizationSettings = optimizationSettings;
		globalOptimizationType = optimizationType;
		globalOptimizationUpdate = optimizationUpdate;
		globalInitialBalance = pInAccountInfo[IDX_BALANCE];

		switch (optimizationSettings.crossoverMode){
			case 0:
				crossoverFunction = ga_crossover_integer_singlepoints;
				break;
			case 1:
				crossoverFunction = ga_crossover_integer_doublepoints;
				break;
			case 2:
				crossoverFunction = ga_crossover_integer_mean;
				break;
			case 3:
				crossoverFunction = ga_crossover_integer_mixing;
				break;
			case 4:
				crossoverFunction = ga_crossover_integer_allele_mixing;
				break;
			default:
				pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*) "Crossover Mode %d not implemented\n", optimizationSettings.crossoverMode);
				return false;
		}

		switch (optimizationSettings.mutationMode){
			case 0:
				mutateFunction = ga_mutate_integer_singlepoint_drift;
				break;
			case 1:
				mutateFunction = ga_mutate_integer_singlepoint_randomize;
				break;
			case 2:
				mutateFunction = ga_mutate_integer_singlepoint_randomize;
				break;
			case 3:
				mutateFunction = ga_mutate_integer_allpoint;
				break;
			default:
				pantheios_logprintf(PANTHEIOS_SEV_ERROR, (PAN_CHAR_T*) "Mutate Mode %d not implemented\n", optimizationSettings.mutationMode);
				return false;
		}

		//MPI Child threads
		if (myId != 0){
			pop = ga_genesis_integer(
			   0,										/* const int              population_size */
			   1,										/* const int              num_chromo */
			   numParamsInSet,							/* const int              len_chromo */
			   generationHook,							/* GAgeneration_hook      generation_hook */
			   NULL,									/* GAiteration_hook       iteration_hook */
			   NULL,									/* GAdata_destructor      data_destructor */
			   NULL,									/* GAdata_ref_incrementor data_ref_incrementor */
			   testFitnessMultipleSymbols,           	/* GAevaluate             evaluate */
			   ga_seed_integer_random,					/* GAseed                 seed */
			   NULL,									/* GAadapt                adapt */
			   ga_select_one_sus,						/* GAselect_one           select_one */
			   ga_select_two_sus,						/* GAselect_two           select_two */
			   mutateFunction,							/* GAmutate				  mutate */
			   crossoverFunction,						/* GAcrossover			  crossover */
			   NULL,									/* GAreplace              replace */
			   NULL										/* void *                 userdata */
			);

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*) "Attaching slave with rank = %d", myId);
			ga_attach_mpi_slave( pop );
		}
		//Main thread for MPI and no MPI
		else {
			pop = ga_genesis_integer(
			   optimizationSettings.population,			/* const int              population_size */
			   1,										/* const int              num_chromo */
			   numParamsInSet,							/* const int              len_chromo */
			   generationHook,							/* GAgeneration_hook      generation_hook */
			   NULL,									/* GAiteration_hook       iteration_hook */
			   NULL,									/* GAdata_destructor      data_destructor */
			   NULL,									/* GAdata_ref_incrementor data_ref_incrementor */
			   testFitnessMultipleSymbols,   			/* GAevaluate             evaluate */
			   ga_seed_integer_random,					/* GAseed                 seed */
			   NULL,									/* GAadapt                adapt */
			   ga_select_one_sus,						/* GAselect_one           select_one */
			   ga_select_two_sus,						/* GAselect_two           select_two */
			   mutateFunction,							/* GAmutate				  mutate */
			   crossoverFunction,						/* GAcrossover			  crossover */
			   NULL,									/* GAreplace              replace */
			   NULL										/* void *                 userdata */
			);

			pantheios_logprintf(PANTHEIOS_SEV_INFORMATIONAL, (PAN_CHAR_T*) "Main thread genetic process with rank = %d", myId);

			ga_population_set_allele_min_integer(pop, 1);
			ga_population_set_allele_max_integer(pop, 100);

			ga_population_set_parameters(
			   pop,														/* population              *pop */
			   (ga_scheme_type)optimizationSettings.evolutionaryMode,	/* const ga_class_type     class */
			   (ga_elitism_type)optimizationSettings.elitismMode,		/* const ga_elitism_type   elitism */
			   optimizationSettings.crossoverProbability,				/* double                  crossover */
			   optimizationSettings.mutationProbability,				/* double                  mutation */
			   optimizationSettings.migrationProbability				/* double                  migration */
			);

			if(globalExecUnderMPI){
				ga_evolution_mpi(
				   pop,									/* population              *pop */
				   optimizationSettings.maxGenerations	/* const int               max_generations */
				);
			}
			else {
				ga_evolution(
				   pop,									/* population              *pop */
				   optimizationSettings.maxGenerations	/* const int               max_generations */
				);
			}

			ga_extinction(pop);	

			if(optimizationFinished != NULL) optimizationFinished();
			if(globalExecUnderMPI) ga_detach_mpi_slaves();
		}
		return true;

	}
	else{
		sprintf(error_t, "Optimization type not implemented");
		*error = (char*)malloc(strlen(error_t) + 1);
		strcpy(*error, error_t);
		if(optimizationFinished != NULL) optimizationFinished();
		return false;
	}

}
