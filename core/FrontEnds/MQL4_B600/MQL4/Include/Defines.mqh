/**
 * @file
 * @brief     Contains common definitions used throughout the MQL4 Interface.
 * @details   The definitions include common constants, bitmasks, array index labels, error codes, and strategy IDs.
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

//#property copyright "Copyright © 2012, Asirikuy Community"
//#property link      "http://www.asirikuy.com"

#define WEEKEND_BAR_MULTIPLIER       1.2       // Multiply the required daily bars by this value to accommodate Saturday or Sunday bars being merged.
#define TAKE_PROFIT_MULTIPLIER       10        // The multiplier of the TP level when using an internal TP.
#define EPSILON                      0.0000001 // An arbitrarily small number used to compare doubles.
#define DAYS_PER_WEEK                7
#define HOURS_PER_DAY                24
#define SECONDS_PER_DAY              86400
#define MILLISECONDS_PER_MINUTE      60000
#define TOTAL_RATES_ARRAYS           10
#define TOTAL_RATES_HISTORY_TYPES    6         // Time, Open, High, Low, Close, Volume.
#define TOTAL_ORDER_TYPES            6         // OP_BUY, OP_BUYSTOP, OP_BUYLIMIT, OP_SELL, OP_SELLSTOP, OP_SELLLIMIT.
#define ORDER_BALANCE                6         // OrderType for deposits and withdrawals.
#define ORDER_CREDIT                 7         // OrderType for bonus credit from the broker.
#define ALL_ORDER_TICKETS            -1        // Constant used when a trading signal is to be applied to all open trades.

#define UTC                          "UTC"                       // Use this as the reference broker name if the trading strategy is optimized for Coordinated Universal Time.
#define ALPARI_UK_HISTORIC_DATA      "Alpari (UK) Historic Data" // Use this as the reference broker name if the trading strategy is optimized for Alpari UK data.
#define FOREX_HISTORICAL_DATA        "Forex Historical Data"     // Use this as the reference broker name if the trading strategy is optimized for Forex Historical data.

// Historic data identifiers
#define BROKER_DATA_ID               0
#define ALPARI_UK_HISTORIC_DATA_ID   1
#define FOREX_HISTORICAL_DATA_ID     2
#define TOTAL_DATA_IDS               3

// Strategy Identifiers
#define WATUKUSHAY_FE_BB             0
#define WATUKUSHAY_FE_CCI            1
#define ATIPAQ                       2
#define AYOTL                        3
#define COATL                        4
#define COMITL_BB                    5
#define COMITL_KC                    6
#define COMITL_PA                    7
#define GODS_GIFT_ATR                8
#define QALLARYI                     9
#define QUIMICHI                     10
#define SAPAQ                        11
#define ASIRIKUY_BRAIN               12
#define TEYACANANI                   13
#define WATUKUSHAY_FE_RSI            14
#define RUPHAY                       15
#define TEST_EA                      16
#define EURCHF_GRID                  17
#define KANTU                        18
#define RECORD_BARS                  19
#define MUNAY                        20
#define RENKOTEST                    21
#define KANTUML                      22
#define KANTURL                      23
#define KELPIE                       24
#define BBS                          25
#define TOTAL_STRATEGY_IDS           26

// Return codes from Asirikuy Framework DLL
#define SUCCESS                      0
#define INVALID_CURRENCY             3000
#define UNKNOWN_SYMBOL               3001
#define SYMBOL_TOO_SHORT             3002
#define PARSE_SYMBOL_FAILED          3003
#define NO_CONVERSION_SYMBOLS        3004
#define NULL_POINTER                 3005
#define INVALID_STRATEGY             3006
#define NOT_ENOUGH_MARGIN            3007
#define SPREAD_TOO_WIDE              3008
#define NOT_ENOUGH_RATES_DATA        3009
#define WORST_CASE_SCENARIO          3010
#define NORMALIZE_BARS_FAILED        3011
#define INIT_LOG_FAILED              3012
#define DEINIT_LOG_FAILED            3013
#define ZERO_DIVIDE                  3014
#define TA_LIB_ERROR                 3015
#define INVALID_PARAMETER            3017
#define NN_TRAINING_FAILED           3018
#define UNKNOWN_TIMEZONE             3019
#define LOCAL_TZ_MISMATCH            3020
#define BROKER_TZ_MISMATCH           3021
#define TOO_MANY_INSTANCES           3022
#define INVALID_CONFIG               3023
#define MISSING_CONFIG               3024
#define INIT_XML_FAILED              3025
#define UNKNOWN_INSTANCE_ID          3026
#define INSUFFICIENT_MEMORY          3027
#define WAIT_FOR_INIT                3028

// Mql4 Interface error codes
#define ERROR_INSUFFICIENT_BARS      3500
#define ERROR_INVALID_INSTANCE_ID    3501
#define ERROR_DUPLICATE_INSTANCE_ID  3502
#define ERROR_NO_CONVERSION_SYMBOL   3503
#define ERROR_INCORRECT_TIMEFRAME    3504
#define ERROR_BAD_HISTORIC_DATA_ID   3505
#define ERROR_INVALID_MODE           3506

struct charArray
{
   char a[256];
   
};

// Strategy operational modes
#define DISABLE                      0
#define ENABLE                       1
#define MONITOR                      2

// Parameter Set Management Modes
#define PSMM_NONE                    0
#define PSMM_ABSOLUTE_PROFIT         1
#define PSMM_PROFIT_DRAWDOWN_RATIO   2
#define PSMM_PROFIT_RANKS            3

// Instance Management Modes
#define IMM_NONE                     0
#define IMM_GAME_THEORY              1
#define IMM_EQUITY_CURVE_AVERAGE     2
#define IMM_NEURAL_NETWORK           3

// SystemSettings array
#define IDX_MAX_SLIPPAGE             0
#define IDX_DIGITS                   1
#define IDX_UI_X_COORDINATE          2
#define IDX_UI_Y_COORDINATE          3
#define IDX_ENABLE_SCREENSHOTS       4
#define IDX_RANDOMIZED_INSTANCE_ID   5
#define IDX_USE_INSTANCE_BALANCE     6
#define IDX_LAST_INSTANCE_BALANCE    7
#define IDX_NEXT_CLOSED_ORDER_INDEX  8
#define IDX_OPEN_PRICE_COMPLIANT     9
#define IDX_HISTORIC_DATA_ID         10
#define SYSTEM_SETTINGS_ARRAY_SIZE   11

// Strategy strings array indexes
#define IDX_SYSTEM_NAME              0
#define IDX_STRATEGY_NAME            1
#define IDX_FRAMEWORK_VERSION        2
#define IDX_TRADE_SYMBOL             3
#define IDX_BASE_CONVERSION_SYMBOL   4
#define IDX_QUOTE_CONVERSION_SYMBOL  5
#define IDX_ACCOUNT_CURRENCY         6
#define IDX_BROKER_NAME              7
#define IDX_REFERENCE_BROKER_NAME    8
#define IDX_CUSTOM_ORDER_COMMENT     9
#define STRATEGY_STRINGS_ARRAY_SIZE  10

// StrategyResults array
#define IDX_TICKET_NUMBER            0
#define IDX_TRADING_SIGNALS          1
#define IDX_LOTS                     2
#define IDX_ENTRY_PRICE              3
#define IDX_BROKER_SL                4
#define IDX_BROKER_TP                5
#define IDX_INTERNAL_SL              6
#define IDX_INTERNAL_TP              7
#define IDX_USE_TRAILING             8
#define IDX_EXPIRATION_TIME          9
#define RESULTS_ARRAY_SIZE           10

// OrderInfo array indexes
#define IDX_ORDER_TICKET             0
#define IDX_ORDER_INSTANCE_ID        1
#define IDX_ORDER_TRADE_TYPE         2
#define IDX_ORDER_OPEN_TIME          3
#define IDX_ORDER_CLOSE_TIME         4
#define IDX_ORDER_STOPLOSS           5
#define IDX_ORDER_TAKEPROFIT         6
#define IDX_ORDER_EXPIRATION         7
#define IDX_ORDER_OPEN_PRICE         8
#define IDX_ORDER_CLOSE_PRICE        9
#define IDX_ORDER_LOTS               10
#define IDX_ORDER_PROFIT             11
#define IDX_ORDER_COMMISSION         12
#define IDX_ORDER_SWAP               13
#define IDX_ORDER_IS_OPEN            14
#define TOTAL_ORDER_INFO_INDEXES     15

// AccountInfo array indexes
#define IDX_ACCOUNT_NUMBER           0
#define IDX_BALANCE                  1
#define IDX_EQUITY                   2
#define IDX_MARGIN                   3
#define IDX_LEVERAGE                 4
#define IDX_CONTRACT_SIZE            5
#define IDX_MINIMUM_STOP             6
#define IDX_STOPOUT_PERCENT          7
#define IDX_TOTAL_TRADE_RISK_PERCENT 8
#define IDX_LARGEST_DD_PERCENT       9
#define ACCOUNT_INFO_ARRAY_SIZE      10

// ProfitDrawdown array indexes
#define IDX_ORDERS_HISTORY_TOTAL     0
#define IDX_CURRENT_PROFIT_PERCENT   1
#define IDX_LARGEST_PROFIT_PERCENT   2
#define IDX_CURRENT_DRAWDOWN_PERCENT 3
#define IDX_LARGEST_DRAWDOWN_PERCENT 4
#define IDX_LARGEST_PROFIT_TIME      5
#define IDX_LARGEST_DRAWDOWN_TIME    6
#define PROFIT_DRAWDOWN_ARRAY_SIZE   7

// BidAsk array indexes
#define IDX_BID                      0
#define IDX_ASK                      1
#define IDX_BASE_CONVERSION_BID      2
#define IDX_BASE_CONVERSION_ASK      3
#define IDX_QUOTE_CONVERSION_BID     4
#define IDX_QUOTE_CONVERSION_ASK     5
#define BID_ASK_ARRAY_SIZE           6

// RatesInfo array indexes
#define IDX_IS_ENABLED               0
#define IDX_REQUIRED_TIMEFRAME       1
#define IDX_TOTAL_BARS_REQUIRED      2
#define IDX_ACTUAL_TIMEFRAME         3
#define IDX_RATES_ARRAY_SIZE         4
#define IDX_POINT                    5
#define IDX_DIGITS_RATES             6
#define RATES_INFO_ARRAY_SIZE        7

// Strategy settings array indexes
#define IDX_ADDITIONAL_PARAM_1       0
#define IDX_ADDITIONAL_PARAM_2       1
#define IDX_ADDITIONAL_PARAM_3       2
#define IDX_ADDITIONAL_PARAM_4       3
#define IDX_ADDITIONAL_PARAM_5       4
#define IDX_ADDITIONAL_PARAM_6       5
#define IDX_ADDITIONAL_PARAM_7       6
#define IDX_ADDITIONAL_PARAM_8       7
#define IDX_ADDITIONAL_PARAM_9       8
#define IDX_ADDITIONAL_PARAM_10      9
#define IDX_ADDITIONAL_PARAM_11      10
#define IDX_ADDITIONAL_PARAM_12      11
#define IDX_ADDITIONAL_PARAM_13      12
#define IDX_ADDITIONAL_PARAM_14      13
#define IDX_ADDITIONAL_PARAM_15      14
#define IDX_ADDITIONAL_PARAM_16      15
#define IDX_ADDITIONAL_PARAM_17      16
#define IDX_ADDITIONAL_PARAM_18      17
#define IDX_ADDITIONAL_PARAM_19      18
#define IDX_ADDITIONAL_PARAM_20      19
#define IDX_ADDITIONAL_PARAM_21      20
#define IDX_ADDITIONAL_PARAM_22      21
#define IDX_ADDITIONAL_PARAM_23      22
#define IDX_ADDITIONAL_PARAM_24      23
#define IDX_ADDITIONAL_PARAM_25      24
#define IDX_ADDITIONAL_PARAM_26      25
#define IDX_ADDITIONAL_PARAM_27      26
#define IDX_ADDITIONAL_PARAM_28      27
#define IDX_ADDITIONAL_PARAM_29      28
#define IDX_ADDITIONAL_PARAM_30      29
#define IDX_ADDITIONAL_PARAM_31      30
#define IDX_ADDITIONAL_PARAM_32      31
#define IDX_ADDITIONAL_PARAM_33      32
#define IDX_ADDITIONAL_PARAM_34      33
#define IDX_ADDITIONAL_PARAM_35      34
#define IDX_ADDITIONAL_PARAM_36      35
#define IDX_ADDITIONAL_PARAM_37      36
#define IDX_ADDITIONAL_PARAM_38      37
#define IDX_ADDITIONAL_PARAM_39      38
#define IDX_ADDITIONAL_PARAM_40      39
#define IDX_ADDITIONAL_PARAM_41      40
#define IDX_USE_SL			         41
#define IDX_USE_TP      			 42
#define IDX_RUN_EVERY_TICK           43
#define IDX_INSTANCE_MANAGEMENT      44
#define IDX_MAX_OPEN_ORDERS          45
#define IDX_IS_BACKTESTING           46
#define IDX_DISABLE_COMPOUNDING      47
#define IDX_TIMED_EXIT_BARS          48
#define IDX_ORIGINAL_ACCOUNT_EQUITY  49
#define IDX_OPERATIONAL_MODE         50
#define IDX_STRATEGY_INSTANCE_ID     51
#define IDX_INTERNAL_STRATEGY_ID     52
#define IDX_STRATEGY_TIMEFRAME       53
#define IDX_WFO_PSET_MANAGEMENT      54
#define IDX_ANALYSIS_WINDOW_SIZE     55
#define IDX_PARAMETER_SET_POOL       56
#define IDX_ACCOUNT_RISK_PERCENT     57
#define IDX_MAX_DRAWDOWN_PERCENT     58
#define IDX_MAX_SPREAD_PRICE         59
#define IDX_SL_ATR_MULTIPLIER        60
#define IDX_TP_ATR_MULTIPLIER        61
#define IDX_ATR_AVERAGING_PERIOD     62
#define IDX_ORDERINFO_ARRAY_SIZE     63
#define STRATEGY_SETTINGS_ARRAY_SIZE 64

// Trading signal bit masks
#define SIGNAL_NONE                  0x00000000
#define SIGNAL_OPEN_BUY              0x00000001
#define SIGNAL_CLOSE_BUY             0x00000002
#define SIGNAL_UPDATE_BUY            0x00000004
#define SIGNAL_OPEN_BUYLIMIT         0x00000008
#define SIGNAL_CLOSE_BUYLIMIT        0x00000010
#define SIGNAL_UPDATE_BUYLIMIT       0x00000020
#define SIGNAL_OPEN_BUYSTOP          0x00000040
#define SIGNAL_CLOSE_BUYSTOP         0x00000080
#define SIGNAL_UPDATE_BUYSTOP        0x00000100
#define SIGNAL_OPEN_SELL             0x00010000
#define SIGNAL_CLOSE_SELL            0x00020000
#define SIGNAL_UPDATE_SELL           0x00040000
#define SIGNAL_OPEN_SELLLIMIT        0x00080000
#define SIGNAL_CLOSE_SELLLIMIT       0x00100000
#define SIGNAL_UPDATE_SELLLIMIT      0x00200000
#define SIGNAL_OPEN_SELLSTOP         0x00400000
#define SIGNAL_CLOSE_SELLSTOP        0x00800000
#define SIGNAL_UPDATE_SELLSTOP       0x01000000

// Trading hours bit masks
#define MASK_TRADE_HOUR_NONE         0x00000000
#define MASK_TRADE_HOUR_0            0x00000001
#define MASK_TRADE_HOUR_1            0x00000002
#define MASK_TRADE_HOUR_2            0x00000004
#define MASK_TRADE_HOUR_3            0x00000008
#define MASK_TRADE_HOUR_4            0x00000010
#define MASK_TRADE_HOUR_5            0x00000020
#define MASK_TRADE_HOUR_6            0x00000040
#define MASK_TRADE_HOUR_7            0x00000080
#define MASK_TRADE_HOUR_8            0x00000100
#define MASK_TRADE_HOUR_9            0x00000200
#define MASK_TRADE_HOUR_10           0x00000400
#define MASK_TRADE_HOUR_11           0x00000800
#define MASK_TRADE_HOUR_12           0x00001000
#define MASK_TRADE_HOUR_13           0x00002000
#define MASK_TRADE_HOUR_14           0x00004000
#define MASK_TRADE_HOUR_15           0x00008000
#define MASK_TRADE_HOUR_16           0x00010000
#define MASK_TRADE_HOUR_17           0x00020000
#define MASK_TRADE_HOUR_18           0x00040000
#define MASK_TRADE_HOUR_19           0x00080000
#define MASK_TRADE_HOUR_20           0x00100000
#define MASK_TRADE_HOUR_21           0x00200000
#define MASK_TRADE_HOUR_22           0x00400000
#define MASK_TRADE_HOUR_23           0x00800000

#define ORDER_FAILURE               -1

