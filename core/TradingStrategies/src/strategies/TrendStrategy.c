/*
* TrendStrategy.c - Trend strategy functions
*/
#include "Precompiled.h"
#include "OrderManagement.h"
#include "Logging.h"
#include "EasyTradeCWrapper.hpp"
#include "base.h"
#include "ComLib.h"
#include "StrategyUserInterface.h"
#include "TrendStrategy.h"
#include "AsirikuyTime.h" /* added for safe_gmtime/safe_timeString prototypes */
#include "InstanceStates.h"
#include "AsirikuyLogger.h"

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

/* Forward declarations for static helper functions to avoid C4013 */
static BOOL move_stop_loss(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, double stopLossLevel);
static BOOL entryBuyRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder);
static BOOL entrySellRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex, int stopHour, BOOL isOrderSignal, BOOL isEnterOrder);
static int isRangeOrder(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, int orderIndex);
static BOOL DailyTrade_Limit_Allow_Trade(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators);
AsirikuyReturnCode workoutExecutionTrend_DailyOpen(StrategyParams* pParams, Indicators* pIndicators, Base_Indicators * pBase_Indicators, BOOL ignored);

#define USE_INTERNAL_SL FALSE
#define USE_INTERNAL_TP FALSE
