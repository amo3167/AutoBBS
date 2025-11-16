/*
* TrendStrategy.c - Trend strategy functions
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "strategies/autobbs/base/Base.h"
#include "strategies/autobbs/shared/ComLib.h"
#include "StrategyUserInterface.h"
#include "strategies/autobbs/trend/TrendStrategy.h"
#include "AsirikuyTime.h" /* added for safe_gmtime/safe_timeString prototypes */
#include "InstanceStates.h"
#include "AsirikuyLogger.h"

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

/* Forward declarations - functions are now in their respective modules */
/* move_stop_loss is in StopLossManagement.h */
/* entryBuyRangeOrder, entrySellRangeOrder, isRangeOrder are in RangeOrderManagement.h */
/* DailyTrade_Limit_Allow_Trade is in TimeManagement.h */
AsirikuyReturnCode workoutExecutionTrend_DailyOpen(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE
