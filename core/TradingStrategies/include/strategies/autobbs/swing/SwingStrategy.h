#pragma once

#ifndef SWINGSTRATEGY_H_
#define SWINGSTRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SwingStrategy.h - Convenience header for all swing strategy modules
 * 
 * This header includes all swing strategy module headers for backward compatibility.
 * Individual modules can be included directly for better organization.
 * 
 * All functions have been extracted to their respective modules:
 * - Common utilities: swing/common/
 * - Hedge strategies: swing/hedge/
 * - DayTrading strategies: swing/daytrading/
 * - MultipleDay strategies: swing/multipleday/
 * - Weekly strategies: swing/weekly/
 * - MACD BEILI strategies: swing/macd_beili/
 */

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SWINGSTRATEGY_H_ */
