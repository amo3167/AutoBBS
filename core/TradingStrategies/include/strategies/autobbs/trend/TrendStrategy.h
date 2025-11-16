#pragma once

#ifndef TRENDSTRATEGY_H_
#define TRENDSTRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "strategies/autobbs/base/Base.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TrendStrategy.h - Convenience header for all trend strategy modules
 * 
 * This header includes all trend strategy module headers for backward compatibility.
 * Individual modules can be included directly for better organization.
 * 
 * All functions have been extracted to their respective modules:
 * - Common utilities: trend/common/
 * - MACD strategies: trend/macd/
 * - Ichimoko strategies: trend/ichimoko/
 * - BBS strategies: trend/bbs/
 * - Limit strategies: trend/limit/
 * - Weekly strategies: trend/weekly/
 * - ShortTerm strategies: trend/shortterm/
 * - Miscellaneous strategies: trend/misc/
 */

/* Include common strategy modules */
#include "strategies/autobbs/trend/common/StopLossManagement.h"
#include "strategies/autobbs/trend/common/RangeOrderManagement.h"
#include "strategies/autobbs/trend/common/TimeManagement.h"
#include "strategies/autobbs/trend/common/OrderSplittingUtilities.h"

/* Include MACD strategy modules */
#include "strategies/autobbs/trend/macd/MACDDailyStrategy.h"
#include "strategies/autobbs/trend/macd/MACDWeeklyStrategy.h"
#include "strategies/autobbs/trend/macd/MACDOrderSplitting.h"

/* Include Ichimoko strategy modules */
#include "strategies/autobbs/trend/ichimoko/IchimokoDailyStrategy.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoWeeklyStrategy.h"
#include "strategies/autobbs/trend/ichimoko/IchimokoOrderSplitting.h"

/* Include BBS strategy modules */
#include "strategies/autobbs/trend/bbs/BBSBreakOutStrategy.h"
#include "strategies/autobbs/trend/bbs/BBSSwingStrategy.h"
#include "strategies/autobbs/trend/bbs/BBSOrderSplitting.h"

/* Include Limit strategy modules */
#include "strategies/autobbs/trend/limit/LimitStrategy.h"
#include "strategies/autobbs/trend/limit/LimitBBSStrategy.h"
#include "strategies/autobbs/trend/limit/LimitOrderSplitting.h"

/* Include Weekly strategy modules */
#include "strategies/autobbs/trend/weekly/WeeklyAutoStrategy.h"
#include "strategies/autobbs/trend/weekly/WeeklyPivotStrategy.h"
#include "strategies/autobbs/trend/weekly/WeeklyOrderSplitting.h"

/* Include ShortTerm strategy modules */
#include "strategies/autobbs/trend/shortterm/ShortTermStrategy.h"
#include "strategies/autobbs/trend/shortterm/ShortTermOrderSplitting.h"

/* Include Miscellaneous strategy modules */
#include "strategies/autobbs/trend/misc/MiscStrategies.h"
#include "strategies/autobbs/trend/misc/KeyKOrderSplitting.h"

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRENDSTRATEGY_H_ */
