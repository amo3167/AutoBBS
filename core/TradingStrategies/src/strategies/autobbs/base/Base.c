/*
That is the base strategy in my auto strategies. 
It mainly provide the trend classification and support, resistance levels. 
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"

/* Include Base strategy modules */
#include "strategies/autobbs/base/indicatorloaders/IndicatorLoaders.h"
#include "strategies/autobbs/base/trendcalculators/TrendCalculators.h"
#include "strategies/autobbs/base/supportresistance/SupportResistance.h"
#include "strategies/autobbs/base/trendanalysis/TrendAnalysis.h"
#include "strategies/autobbs/base/atrprediction/ATRPrediction.h"
#include "strategies/autobbs/base/utilities/BaseUtilities.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

AsirikuyReturnCode runBase(StrategyParams* pParams, Base_Indicators * pIndicators)
{
	loadIndicators_Internal(pParams, pIndicators);	
	return SUCCESS;
}

/**
 * Base.c - Dispatcher for Base strategy modules
 * 
 * This file serves as a dispatcher that includes all module headers.
 * All function implementations have been extracted to their respective modules:
 *   - Indicator loading: base/indicatorloaders/IndicatorLoaders.c
 *   - Trend calculation: base/trendcalculators/TrendCalculators.c
 *   - Support/Resistance: base/supportresistance/SupportResistance.c
 *   - Trend analysis: base/trendanalysis/TrendAnalysis.c
 *   - ATR prediction: base/atrprediction/ATRPrediction.c
 *   - Utilities: base/utilities/BaseUtilities.c
 * 
 * Note: The main entry point is runBase() which calls loadIndicators_Internal().
 * All other functions are available through their respective module headers.
 */
