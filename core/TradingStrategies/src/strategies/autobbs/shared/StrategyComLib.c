
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "InstanceStates.h"
#include "AsirikuyLogger.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"  // For splitBuyOrders_ATR, splitBuyOrders_LongTerm
#include "strategies/autobbs/shared/ComLib.h"  // Provides AdditionalSettings and Indicators type definitions

/* Include profit management module */
#include "strategies/autobbs/shared/common/ProfitManagement.h"

/* Include order management module */
#include "strategies/autobbs/shared/ordermanagement/OrderManagement.h"

/* Include strategy execution module */
#include "strategies/autobbs/shared/execution/StrategyExecution.h"

/* Include indicator management module */
#include "strategies/autobbs/shared/indicators/IndicatorManagement.h"

/* Include order splitting module */
#include "strategies/autobbs/shared/ordersplitting/OrderSplitting.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

// exitSignal_t is already defined in base.h
// AdditionalSettings and Indicators are defined in ComLib.h

/**
 * StrategyComLib.c - Dispatcher for StrategyComLib modules
 * 
 * This file serves as a dispatcher that includes all module headers.
 * All function implementations have been extracted to their respective modules:
 *   - Profit management: shared/common/ProfitManagement.c
 *   - Order management: shared/ordermanagement/OrderManagement.c
 *   - Strategy execution: shared/execution/StrategyExecution.c
 *   - Indicator management: shared/indicators/IndicatorManagement.c
 *   - Order splitting: shared/ordersplitting/OrderSplitting.c
 * 
 * Note: The main strategy entry point is runAutoBBS() in AutoBBS.c.
 * Type definitions (AdditionalSettings, Indicators) are provided by ComLib.h.
 * This file only contains module includes and configuration defines.
 */

