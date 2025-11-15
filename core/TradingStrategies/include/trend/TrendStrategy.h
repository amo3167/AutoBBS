#pragma once

#ifndef TRENDSTRATEGY_H_
#define TRENDSTRATEGY_H_

#ifndef ASIRIKUY_DEFINES_H_
#include "AsirikuyDefines.h"
#include "ComLib.h"
#include "Base.h"
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
#include "trend/common/StopLossManagement.h"
#include "trend/common/RangeOrderManagement.h"
#include "trend/common/TimeManagement.h"
#include "trend/common/OrderSplittingUtilities.h"

/* Include MACD strategy modules */
#include "trend/macd/MACDDailyStrategy.h"
#include "trend/macd/MACDWeeklyStrategy.h"
#include "trend/macd/MACDOrderSplitting.h"

/* Include Ichimoko strategy modules */
#include "trend/ichimoko/IchimokoDailyStrategy.h"
#include "trend/ichimoko/IchimokoWeeklyStrategy.h"
#include "trend/ichimoko/IchimokoOrderSplitting.h"

/* Include BBS strategy modules */
#include "trend/bbs/BBSBreakOutStrategy.h"
#include "trend/bbs/BBSSwingStrategy.h"
#include "trend/bbs/BBSOrderSplitting.h"

/* Include Limit strategy modules */
#include "trend/limit/LimitStrategy.h"
#include "trend/limit/LimitBBSStrategy.h"
#include "trend/limit/LimitOrderSplitting.h"

/* Include Weekly strategy modules */
#include "trend/weekly/WeeklyAutoStrategy.h"
#include "trend/weekly/WeeklyPivotStrategy.h"
#include "trend/weekly/WeeklyOrderSplitting.h"

/* Include ShortTerm strategy modules */
#include "trend/shortterm/ShortTermStrategy.h"
#include "trend/shortterm/ShortTermOrderSplitting.h"

/* Include Miscellaneous strategy modules */
#include "trend/misc/MiscStrategies.h"
#include "trend/misc/KeyKOrderSplitting.h"

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRENDSTRATEGY_H_ */
