/**
 * @file
 * @brief     Common defines, enums, and structs used by multiple projects.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @author    Daniel Fernandez (Assisted with design and code styling)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#ifndef ASIRIKUY_DEFINES_H_
#define ASIRIKUY_DEFINES_H_
#pragma once

#if defined _WIN32 || defined _WIN64
	#define strtok_r strtok_s
	#define _CRT_RAND_S
	#include <stdlib.h>
	#define rand_r rand_s
#endif

#include <time.h>
#include <stdint.h>

#define EPSILON					 0.000000001 /* Near zero comparative define*/

#define MAX_Kantu_Systems         20      /* The maximum number of kantu rules for each instance. */

#define WEEKEND_BAR_MULTIPLIER   1.2     /* Multiply the required bars by this value to accommodate Saturday or Sunday bars being removed or merged */
#define MIN_SYMBOL_LENGTH        7       /* Eg. "EURUSD" = 6 chars + 1 for the null terminator */
#define MAX_ERROR_STRING_SIZE    100     /* Character limit for error strings */
#define MAX_OUTPUT_ERROR_STRING_SIZE    300     /* Character limit for error strings */
#define MAX_TIMEZONE_NAME_SIZE   50      /* Character limit for timezone names */
#define MAX_TIME_STRING_SIZE     30      /* Eg. "Sun Jan 01 00:00:00 1900" */
#define MAX_FILE_PATH_CHARS      256     /* Character limit for filenames and paths */
#define MAX_FILENAME_EXT_CHARS   10      /* Character limit for filename extensions */
#define MAX_PARAMETER_NAME_SIZE  40      /* Character limit for parameter names */
#define MAX_PARAMETERS           200     /* Maximum number of parameters for a trading strategy */
#define STANDARD_INDICATOR_SHIFT 1       /* Use shift 1 rather than shift 0 to eliminate current bar dependence */
#define MAX_INSTANCES            200     /* It's unlikely that anyone will run more strategy instances than this. */
#define MAX_RATES_BUFFERS        10      /* The maximum number of rates buffers for each instance. */
#define DEFAULT_RATES_BUF_EXT    100     /* The rates buffer extension. Higher values improve framework speed. Lower values reduce RAM usage */
#define LOCAL_TIMEZONE_STRING    "Local" /* This string is used to retrieve local timezone info from the timezone config file */
#define UTC_TIMEZONE_STRING      "UTC"   /* This string is used to retrieve UTC timezone info from the timezone config file */
#define TM_EPOCH_YEAR            1900    /* In ANSI C (time.h) when using struct tm, years are counted from 1900. i.e. Year 1 = 1901. */
#define NTP_EPOCH_YEAR           1900    /* In the network time protocol specification years are counted from 1900. */
#define UNIX_EPOCH_YEAR          1970    /* In UNIX based systems years are counted from 1970. time_t is also widely accepted to use this epoch */
#define ALL_ORDER_TICKETS        -1      /* A special constant for order tickets used when a trading signal is to be applied to all open trades. */
#define PRIMARY_RATES_INDEX      0       /* This rates index used for the main symbol and timeframe being traded. */
#define TICK_DATA_STORAGE_LIMIT  500	 /* Maximum number of ticks to store */


/* Days of the week */
#define EVERY_DAY -1
#define SUNDAY    0
#define MONDAY    1
#define TUESDAY   2
#define WEDNESDAY 3
#define THURSDAY  4
#define FRIDAY    5
#define SATURDAY  6
/********************/

/* Define time unit conversions */
#define SECONDS_PER_MINUTE   60
#define MINUTES_PER_HOUR     60
#define HOURS_PER_DAY        24
#define SECONDS_PER_HOUR     3600
#define SECONDS_PER_DAY      86400
#define MINUTES_PER_DAY      1440
#define MINUTES_PER_WEEK     10080
#define DAYS_PER_WEEK        7
#define DAYS_PER_YEAR        365
#define DAYS_PER_LEAP_YEAR   366
#define SECONDS_1900_TO_1970 2208988800
#define EPOCH_WEEK_OFFSET    259200 /* Number of seconds between the unix epoch and the following Sunday midnight. */

#define DAY_OF_WEEK(time)  (((time / SECONDS_PER_DAY) + THURSDAY) % DAYS_PER_WEEK) /* Thursday was the day of the week on the unix epoch: 01/01/1970. */
#define LEAPYEAR(year)     (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)     (LEAPYEAR(year) ? DAYS_PER_LEAP_YEAR : DAYS_PER_YEAR)

static const int DAYS_PER_MONTH[2][12] = 
{
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};
/********************************/

/* Define NULL pointer */
#ifndef NULL
#  ifdef __cplusplus
#    define NULL 0
#  else
#    define NULL ((void *)0)
#  endif
#endif
/***********************/

/* Define booleans */
#ifndef FALSE
#  define FALSE 0
#endif

#ifndef TRUE
#  define TRUE  1
#endif

typedef int BOOL;
/*******************/

typedef enum asirikuyReturnCode_t
{
  SUCCESS               = 0,
  INVALID_CURRENCY      = 3000,
  UNKNOWN_SYMBOL        = 3001,
  SYMBOL_TOO_SHORT      = 3002,
  PARSE_SYMBOL_FAILED   = 3003,
  NO_CONVERSION_SYMBOLS = 3004,
  NULL_POINTER          = 3005,
  INVALID_STRATEGY      = 3006,
  NOT_ENOUGH_MARGIN     = 3007,
  SPREAD_TOO_WIDE       = 3008,
  NOT_ENOUGH_RATES_DATA = 3009,
  WORST_CASE_SCENARIO   = 3010,
  NORMALIZE_BARS_FAILED = 3011,
  INIT_LOG_FAILED       = 3012,
  DEINIT_LOG_FAILED     = 3013,
  ZERO_DIVIDE           = 3014,
  TA_LIB_ERROR          = 3015,
  INVALID_TIME_OFFSET   = 3016,
  INVALID_PARAMETER     = 3017,
  NN_TRAINING_FAILED    = 3018,
  UNKNOWN_TIMEZONE      = 3019,
  LOCAL_TZ_MISMATCH     = 3020,
  BROKER_TZ_MISMATCH    = 3021,
  TOO_MANY_INSTANCES    = 3022,
  INVALID_CONFIG        = 3023,
  MISSING_CONFIG        = 3024,
  INIT_XML_FAILED       = 3025,
  UNKNOWN_INSTANCE_ID   = 3026,
  INSUFFICIENT_MEMORY   = 3027,
  WAIT_FOR_INIT         = 3028,
  ERROR_IN_RATES_RETRIEVAL = 3029,
  BID_ASK_IS_ZERO		= 3030,
  FILE_WRITING_ERROR	= 3031,  
} AsirikuyReturnCode;

typedef enum orderType_t
{
  NONE      = -1,
  BUY       = 0,
  SELL      = 1,
  BUYLIMIT  = 2,
  SELLLIMIT = 3,
  BUYSTOP   = 4,
  SELLSTOP  = 5
} OrderType;

typedef enum appliedPrice_t
{
  PRICE_CLOSE    = 0,
  PRICE_OPEN     = 1,
  PRICE_HIGH     = 2,
  PRICE_LOW      = 3,
  PRICE_MEDIAN   = 4,
  PRICE_TYPICAL  = 5,
  PRICE_WEIGHTED = 6
} AppliedPrice;

typedef enum operationalMode_t
{
  MODE_DISABLE = 0,
  MODE_ENABLE  = 1,
  MODE_MONITOR = 2
} OperationalMode;

typedef enum parameterSetManagementMode_t
{
  PSMM_NONE                  = 0,
  PSMM_ABSOLUTE_PROFIT       = 1,
  PSMM_PROFIT_DRAWDOWN_RATIO = 2,
  PSMM_PROFIT_RANKS          = 3
} ParameterSetManagementMode;

typedef enum instanceManagementMode_t
{
  IMM_NONE                 = 0,
  IMM_GAME_THEORY          = 1,
  IMM_EQUITY_CURVE_AVERAGE = 2,
  IMM_NEURAL_NETWORK       = 3
} InstanceManagementMode;

typedef enum settingsIndex_t
{
  ADDITIONAL_PARAM_1   = 0,
  ADDITIONAL_PARAM_2   = 1,
  ADDITIONAL_PARAM_3   = 2,
  ADDITIONAL_PARAM_4   = 3,
  ADDITIONAL_PARAM_5   = 4,
  ADDITIONAL_PARAM_6   = 5,
  ADDITIONAL_PARAM_7   = 6,
  ADDITIONAL_PARAM_8   = 7,
  ADDITIONAL_PARAM_9   = 8,
  ADDITIONAL_PARAM_10  = 9,
  ADDITIONAL_PARAM_11  = 10,
  ADDITIONAL_PARAM_12  = 11,
  ADDITIONAL_PARAM_13  = 12,
  ADDITIONAL_PARAM_14  = 13,
  ADDITIONAL_PARAM_15  = 14,
  ADDITIONAL_PARAM_16  = 15,
  ADDITIONAL_PARAM_17  = 16,
  ADDITIONAL_PARAM_18  = 17,
  ADDITIONAL_PARAM_19  = 18,
  ADDITIONAL_PARAM_20  = 19,
  ADDITIONAL_PARAM_21  = 20,
  ADDITIONAL_PARAM_22  = 21,
  ADDITIONAL_PARAM_23  = 22,
  ADDITIONAL_PARAM_24  = 23,
  ADDITIONAL_PARAM_25  = 24,
  ADDITIONAL_PARAM_26  = 25,
  ADDITIONAL_PARAM_27  = 26,
  ADDITIONAL_PARAM_28  = 27,
  ADDITIONAL_PARAM_29  = 28,
  ADDITIONAL_PARAM_30  = 29,
  ADDITIONAL_PARAM_31  = 30,
  ADDITIONAL_PARAM_32  = 31,
  ADDITIONAL_PARAM_33  = 32,
  ADDITIONAL_PARAM_34  = 33,
  ADDITIONAL_PARAM_35  = 34,
  ADDITIONAL_PARAM_36  = 35,
  ADDITIONAL_PARAM_37  = 36,
  ADDITIONAL_PARAM_38  = 37,
  ADDITIONAL_PARAM_39  = 38,
  ADDITIONAL_PARAM_40  = 39,
  IS_SPREAD_BETTING    = 40,
  USE_SL			   = 41,
  USE_TP               = 42,
  RUN_EVERY_TICK       = 43,
  INSTANCE_MANAGEMENT  = 44,
  MAX_OPEN_ORDERS      = 45,
  IS_BACKTESTING       = 46,
  DISABLE_COMPOUNDING  = 47,
  TIMED_EXIT_BARS      = 48,
  ORIGINAL_EQUITY      = 49,
  OPERATIONAL_MODE     = 50,
  STRATEGY_INSTANCE_ID = 51,
  INTERNAL_STRATEGY_ID = 52,
  TIMEFRAME            = 53,
  SAVE_TICK_DATA       = 54,
  ANALYSIS_WINDOW_SIZE = 55,
  PARAMETER_SET_POOL   = 56,
  ACCOUNT_RISK_PERCENT = 57,
  MAX_DRAWDOWN_PERCENT = 58,
  MAX_SPREAD           = 59,
  SL_ATR_MULTIPLIER    = 60,
  TP_ATR_MULTIPLIER    = 61,
  ATR_AVERAGING_PERIOD = 62,
  ORDERINFO_ARRAY_SIZE = 63
} SettingsIndex;

typedef struct strategyResults_t
{
  double ticketNumber;
  double tradingSignals;
  double lots;
  double entryPrice;
  double brokerSL;
  double brokerTP;
  double internalSL;
  double internalTP;
  double useTrailingSL;
  double expirationTime;
} StrategyResults;

typedef struct bidAsk_t
{
  int      arraySize;
  double*  bid;
  double*  ask;
  double   baseConversionBid;
  double   baseConversionAsk;
  double   quoteConversionBid;
  double   quoteConversionAsk;
} BidAsk;

typedef struct accountInfo_t
{
  char*  brokerName;
  char*  referenceName;
  char*  currency;
  int    accountNumber;
  double balance;
  double equity;
  double margin;
  double leverage;
  double contractSize;
  double minimumStop;
  double stopoutPercent;
  double totalOpenTradeRiskPercent;
  double largestDrawdownPercent;

} AccountInfo;

typedef struct orderInfo_t
{
  int       ticket;
  int       instanceId;
  OrderType type;
  time_t    openTime;
  time_t    closeTime;
  float     stopLoss;
  float     takeProfit;
  time_t    expiriation;
  float     openPrice;
  float     closePrice;
  float     lots;
  float     profit;
  float     commission;
  float     swap;
  BOOL      isOpen;
} OrderInfo;

typedef struct ratesInfo_t
{
  BOOL     isEnabled;
  BOOL     isBufferFull;
  int      timeframe;
  int      arraySize;
  double   point;
  int      digits;
} RatesInfo;

typedef struct rates_t
{
  RatesInfo info;
  time_t*   time;
  double*   open;
  double*   high;
  double*   low;
  double*   close;
  double*   volume;
} Rates;

typedef struct tickData_t
{
  int		arraySize;
  int*      time;
  double*   bid;
  double*   ask;
} tickData;

typedef struct ratesBuffers_t
{
  int    instanceId;
  int    bufferOffsets[MAX_RATES_BUFFERS];
  Rates  rates[MAX_RATES_BUFFERS];
} RatesBuffers;

typedef struct timezoneInfo_t
{
  char   name[MAX_TIMEZONE_NAME_SIZE];
  int    startMonth;
  int    startNth;
  int    startDay;
  int    startHour;
  int    endMonth;
  int    endNth;
  int    endDay;
  int    endHour;
  int    gmtOffsetStd;
  int    gmtOffsetDS;
  int    weekStartDay;
  int    weekStartHour;
  int    weekendDay;
  int    weekendHour;
} TimezoneInfo;

typedef struct parameterSet_t
{
  int32_t    id;
  int32_t    totalParameters;
  float*     parameters;
  int32_t    totalOrders;
  OrderInfo* orders;
} ParameterSet;

typedef struct parameterInfo_t
{
  char  parameterName[MAX_PARAMETER_NAME_SIZE];
  float value;  
  BOOL  optimize;
  float startValue;
  float stepSize;
  float stopValue;
} ParameterInfo;

typedef struct strategyParams_t
{
  ParameterInfo*   expertParameterInfo;
  char*            tradeSymbol;
  time_t           currentBrokerTime;
  RatesBuffers*    ratesBuffers;
  BidAsk           bidAsk;
  AccountInfo      accountInfo;
  OrderInfo*       orderInfo;
  double*          settings;
  StrategyResults* results;  
} StrategyParams;

#endif /* ASIRIKUY_DEFINES_H_ */

