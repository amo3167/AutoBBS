#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "InstanceStates.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "AsirikuyTime.h"
#include "AsirikuyLogger.h"

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE

/* Include common strategy modules */
#include "strategies/autobbs/swing/common/MultipleDayOrderManagement.h"

/* Include Hedge strategy modules */
#include "strategies/autobbs/swing/hedge/HedgeStrategy.h"
#include "strategies/autobbs/swing/hedge/HedgeOrderSplitting.h"

/* Include DayTrading strategy modules */
#include "strategies/autobbs/swing/daytrading/DayTradingStrategy.h"
#include "strategies/autobbs/swing/daytrading/DayTradingOrderSplitting.h"
#include "strategies/autobbs/swing/daytrading/DayTradingHelpers.h"

/* Include MultipleDay strategy modules */
#include "strategies/autobbs/swing/multipleday/MultipleDayStrategy.h"
#include "strategies/autobbs/swing/multipleday/MultipleDayHelpers.h"

/* Include Weekly strategy modules */
#include "strategies/autobbs/swing/weekly/WeeklyStrategy.h"

/* Include MACD BEILI strategy modules */
#include "strategies/autobbs/swing/macd_beili/MACDBEILIStrategy.h"

// All strategy functions have been extracted to their respective modules.
// This file now serves as a dispatcher that includes all module headers.
// The actual implementations are in:
//   - swing/daytrading/DayTradingStrategy.c
//   - swing/multipleday/MultipleDayStrategy.c
//   - swing/weekly/WeeklyStrategy.c
//   - swing/hedge/HedgeStrategy.c
//   - swing/macd_beili/MACDBEILIStrategy.c
