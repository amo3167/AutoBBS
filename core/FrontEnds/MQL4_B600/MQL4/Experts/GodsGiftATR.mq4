/**
 * @file
 * @brief     The God's Gift ATR trading system.
 * @details   God's Gift ATR uses a 1 hour timeframe and enters trades on trend retracements.
 * 
 * @author    Daniel Fernandez (Original idea, initial implementation, and optimization)
 * @author    Maxim Feinshtein (Contributed to initial implementation, code styling, error handling, and user interface)
 * @author    Morgan Doel (Ported God's Gift ATR to the F4 framework)
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

#property copyright "Copyright © 2012, Asirikuy Community"
#property link      "http://www.asirikuy.com"

#include "../include/Common.mqh"
#include "../include/Defines.mqh"
#include "../include/UserInterface.mqh"
#include "../include/AsirikuyFramework.mqh"

#define SYSTEM_NAME "GodsGiftATR"
#define TOTAL_INTERNAL_STRATEGIES 1

extern string COMMENTS              = "Click \"Modify expert\" to view comments inside the source code.";
extern string SECTION_1             = "################# General Settings #################";
extern string FRAMEWORK_CONFIG      = "./MQL4/Files/AsirikuyConfig.xml";
extern bool   USE_ORDER_WRAPPER     = false;  // Use the NFA-compliant virtual order manager instead of the default MT4 implementation
extern bool   ENABLE_SCREENSHOTS    = false;  // Save a screenshot whenever a new trade is opened.
extern int    HISTORIC_DATA_ID      = 0;      // Select the data being used for backtesting. Broker Data = 0, Alpari UK Historic Data = 1, Forex Historical Data = 2.
extern double MAX_SLIPPAGE_PIPS     = 2;      // Max slippage in pips. Trades will not be opened or closed if the price slips more than this value.
extern int    UI_FONT_SIZE          = 12;     // Font size used by the user interface.
extern color  UI_SYSTEM_INFO_COLOR  = White;  // System name, version, and copyright will be displayed in this color on the user interface.
extern color  UI_PROFIT_INFO_COLOR  = Yellow; // Current Profit and drawdown and the largest historic profit and drawdown will be displayed in this color on the user interface.
extern color  UI_TRADE_INFO_COLOR   = Orange; // Trade size, TP, and SL will be displayed in this color on the user interface.
extern color  UI_ERROR_INFO_COLOR   = Red;    // Information about the last error and the time it occured will be displayed in this color on the user interface.
extern color  UI_CUSTOM_INFO_COLOR  = SkyBlue;// Customized UI information for each strategy. This can be used for things like indicator values etc.
extern string CUSTOM_ORDER_COMMENT  = "";     // Specify a custom comment for all orders opened by this strategy. If left blank the default comment with be used: "StrategyName Version F4.x.x".

extern string SECTION_2             = "############## Common Strategy Settings ##############";
extern int    OPERATIONAL_MODE      = 0;      // DISABLE = 0, ENABLE = 1, MONITOR = 2. In MONITOR mode current orders are managed but no new orders are opened.
extern bool   RUN_EVERY_TICK        = false;  // If this is set to false the strategy will only run once per bar.
extern int    STRATEGY_INSTANCE_ID  = -1;     // Unique identifier for an instance of the trading strategy. Also called a "Magic Number".
extern int    STRATEGY_TIMEFRAME    = 0;      // 1hour = 60, 4hour = 240, daily = 1440.
extern double ACCOUNT_RISK_PERCENT  = 0.0;    // Percentage of account risked per trade. Warning: Losses will sometimes exceed this value!
extern double MAX_DRAWDOWN_PERCENT  = 0.0;    // Worst case scenario derived from monticarlo simulations using the above risk setting.
extern double MAX_SPREAD_PIPS       = 0.0;    // Max spread in pips. Trades will not be opened when the spread is wider than this value.
extern bool   DISABLE_COMPOUNDING   = false;  // Remove the effect of compounding profits. Calculate order sizes as if the account equity never changes.
extern bool   USE_INSTANCE_BALANCE  = false;  // Each strategy will use is own virtual balance and trade as if there are no other strategies running on the same account.
extern double INIT_INSTANCE_BALANCE = 0;      // Set the initial instance balance. To reset the instance balance at a later date simply select a new instance ID.
extern int    MAX_OPEN_ORDERS       = 1;      // The maximum number of orders the instance is allowed to have open at a time.
extern int    TIMED_EXIT_BARS       = 0;      // Exit a trade if this number of bars has elapsed since it was opened. Set to 0 to disable this feature.
extern double SL_ATR_MULTIPLIER     = 0.0;    // The stoploss is set to this multiple of the average true range.
extern double TP_ATR_MULTIPLIER     = 0.0;    // The takeprofit is set to this multiple of the average true range.
extern int    ATR_AVERAGING_PERIOD  = 0;      // Averaging period for the average true range.

extern string SECTION_3             = "############## Additional Strategy Settings ##############";

extern double  BB_PERIOD				      = 0; // Period of the Bollinger Band indicator 
extern double  BB_DEVIATION				   = 0 ; // Bollinger Band deviation as multiples of the standard deviation
extern double  KC_PERIOD			      = 0; // Keltner channel period
extern double  KC_ATR_DISTANCE			  = 0 ; // Keltner channel ATR distance multiplier
extern double  SHORT_TERM_STOCH_PERIOD    =0 ; // Short term stochastic oscillator period.
extern double  MEDIUM_TERM_STOCH_PERIOD   = 0; // Medium term stochastic oscillator period.
extern double  LONG_TERM_STOCH_PERIOD     = 0; // Long term stochastic oscillator period.
extern double  SHORT_STOCH_EXIT_SYMMETRY  = 0; // Extreme level of short term stochastic required for exits.
extern double  MED_STOCH_EXIT_SYMMETRY    = 0; // Retracement level of medium term stochastic required for exits.
extern double  LONG_STOCH_EXIT_SYMMETRY   = 0; // Retracement level of long term stochastic required for exits.
extern double  SHORT_STOCH_ENTRY_SYMMETRY = 0; // Short term stochastic retracement level for opening orders.
extern double  MED_STOCH_ENTRY_SYMMETRY   = 0; // Entry threshold for trend determination for the medium term stochastic oscillators.
extern double  LONG_STOCH_ENTRY_SYMMETRY  = 0; // Entry threshold for trend determination for the long term stochastic oscillators.
extern int     EURUSD_TIME_FILTER		  = 0;  // Use or avoid using the EUR/USD monthly time filter (0 avoid, 1 use)

bool g_initializedSettings;

int init()
{
  g_initializedSettings = false;
  
  return(0);
}

int deinit()
{
  deinitInstance(STRATEGY_INSTANCE_ID);
  return(0);
}

int start()
{
  static charArray strategyStrings [TOTAL_INTERNAL_STRATEGIES][STRATEGY_STRINGS_ARRAY_SIZE];
  static double systemSettings  [TOTAL_INTERNAL_STRATEGIES][SYSTEM_SETTINGS_ARRAY_SIZE];
  static double strategySettings[TOTAL_INTERNAL_STRATEGIES][STRATEGY_SETTINGS_ARRAY_SIZE];
  static double profitDrawdown  [TOTAL_INTERNAL_STRATEGIES][PROFIT_DRAWDOWN_ARRAY_SIZE];
  static charArray ratesSymbols    [TOTAL_INTERNAL_STRATEGIES][TOTAL_RATES_ARRAYS];
  static double ratesInformation[TOTAL_INTERNAL_STRATEGIES][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE];

  if(!g_initializedSettings)
  {
    g_initializedSettings = initialize(systemSettings, strategySettings, strategyStrings, profitDrawdown, ratesSymbols, ratesInformation);
    if(!g_initializedSettings)
    {
      return(0);
    }
  }
  
  for(int i = 0; i < TOTAL_INTERNAL_STRATEGIES; i++)
  {
    if((strategySettings[i][IDX_OPERATIONAL_MODE] != DISABLE) && c_validateSystemSettings(i, systemSettings, strategySettings, strategyStrings, ratesSymbols, ratesInformation))
    {
      c_runStrategy(i, systemSettings, strategySettings, strategyStrings, profitDrawdown, ratesSymbols, ratesInformation);
    }
  }

  return(0);
}

bool initialize(double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], double& profitDrawdown[][PROFIT_DRAWDOWN_ARRAY_SIZE], charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE])
{
  StringToCharArray(SYSTEM_NAME, strategyStrings [0][IDX_STRATEGY_NAME].a) ;
  StringToCharArray(Symbol(), strategyStrings [0][IDX_TRADE_SYMBOL].a)     ;
  StringToCharArray(ALPARI_UK_HISTORIC_DATA, strategyStrings [0][IDX_REFERENCE_BROKER_NAME].a) ;
  StringToCharArray(CUSTOM_ORDER_COMMENT, strategyStrings [0][IDX_CUSTOM_ORDER_COMMENT].a) ;
  systemSettings  [0][IDX_HISTORIC_DATA_ID]      = HISTORIC_DATA_ID;
  systemSettings  [0][IDX_UI_X_COORDINATE]       = 0;
  systemSettings  [0][IDX_UI_Y_COORDINATE]       = 0;
  systemSettings  [0][IDX_OPEN_PRICE_COMPLIANT]  = true;
  strategySettings[0][IDX_OPERATIONAL_MODE]      = OPERATIONAL_MODE;
  strategySettings[0][IDX_RUN_EVERY_TICK]        = RUN_EVERY_TICK;
  strategySettings[0][IDX_STRATEGY_INSTANCE_ID]  = STRATEGY_INSTANCE_ID;
  strategySettings[0][IDX_INTERNAL_STRATEGY_ID]  = GODS_GIFT_ATR;
  strategySettings[0][IDX_STRATEGY_TIMEFRAME]    = STRATEGY_TIMEFRAME;
  strategySettings[0][IDX_ACCOUNT_RISK_PERCENT]  = ACCOUNT_RISK_PERCENT;
  strategySettings[0][IDX_MAX_DRAWDOWN_PERCENT]  = MAX_DRAWDOWN_PERCENT;
  strategySettings[0][IDX_MAX_OPEN_ORDERS]       = MAX_OPEN_ORDERS;
  strategySettings[0][IDX_TIMED_EXIT_BARS]       = TIMED_EXIT_BARS;
  strategySettings[0][IDX_USE_SL]     			 = 1;
  strategySettings[0][IDX_USE_TP]     			 = 1;
  strategySettings[0][IDX_SL_ATR_MULTIPLIER]     = SL_ATR_MULTIPLIER;
  strategySettings[0][IDX_TP_ATR_MULTIPLIER]     = TP_ATR_MULTIPLIER;
  strategySettings[0][IDX_ATR_AVERAGING_PERIOD]  = ATR_AVERAGING_PERIOD;
  strategySettings[0][IDX_ORDERINFO_ARRAY_SIZE]  = 3;
  strategySettings[0][IDX_ADDITIONAL_PARAM_1]     = BB_PERIOD;
  strategySettings[0][IDX_ADDITIONAL_PARAM_2]    = BB_DEVIATION;
  strategySettings[0][IDX_ADDITIONAL_PARAM_3]    = KC_PERIOD;
  strategySettings[0][IDX_ADDITIONAL_PARAM_4]    = KC_ATR_DISTANCE;
  strategySettings[0][IDX_ADDITIONAL_PARAM_5]    = SHORT_TERM_STOCH_PERIOD;
  strategySettings[0][IDX_ADDITIONAL_PARAM_6]    = MEDIUM_TERM_STOCH_PERIOD;
  strategySettings[0][IDX_ADDITIONAL_PARAM_7]    = LONG_TERM_STOCH_PERIOD;
  strategySettings[0][IDX_ADDITIONAL_PARAM_8]    = SHORT_STOCH_EXIT_SYMMETRY;
  strategySettings[0][IDX_ADDITIONAL_PARAM_9]    = MED_STOCH_EXIT_SYMMETRY;
  strategySettings[0][IDX_ADDITIONAL_PARAM_10]   = LONG_STOCH_EXIT_SYMMETRY;
  strategySettings[0][IDX_ADDITIONAL_PARAM_11]   = SHORT_STOCH_ENTRY_SYMMETRY;
  strategySettings[0][IDX_ADDITIONAL_PARAM_12]   = MED_STOCH_ENTRY_SYMMETRY;
  strategySettings[0][IDX_ADDITIONAL_PARAM_13]   = LONG_STOCH_ENTRY_SYMMETRY;
  strategySettings[0][IDX_ADDITIONAL_PARAM_14]   = EURUSD_TIME_FILTER;
  StringToCharArray(Symbol(), ratesSymbols[0][0].a ) ;
  ratesInformation[0][0][IDX_IS_ENABLED]         = true;
  ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = STRATEGY_TIMEFRAME;
  ratesInformation[0][0][IDX_TOTAL_BARS_REQUIRED]= 1000;
  StringToCharArray(Symbol(), ratesSymbols[0][1].a ) ;
  ratesInformation[0][1][IDX_IS_ENABLED]         = true;
  ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_D1;
  ratesInformation[0][1][IDX_TOTAL_BARS_REQUIRED]= ATR_AVERAGING_PERIOD + 2;
  
  if(!IsLibrariesAllowed() || !IsDllsAllowed())
  {
    static bool librariesAlertDisplayed = false;
    if(!librariesAlertDisplayed)
    {
      Alert("Please enable external experts and dll imports and then restart ", SYSTEM_NAME, ".");
      librariesAlertDisplayed = true;
    }
    return(false);
  }
  
  return(c_init(TOTAL_INTERNAL_STRATEGIES, USE_ORDER_WRAPPER, FRAMEWORK_CONFIG, strategyStrings, systemSettings, strategySettings, ratesSymbols, ratesInformation, profitDrawdown, MAX_SLIPPAGE_PIPS, MAX_SPREAD_PIPS, 
    ENABLE_SCREENSHOTS, DISABLE_COMPOUNDING, USE_INSTANCE_BALANCE, INIT_INSTANCE_BALANCE, UI_FONT_SIZE, UI_SYSTEM_INFO_COLOR, UI_PROFIT_INFO_COLOR, UI_TRADE_INFO_COLOR, UI_ERROR_INFO_COLOR, UI_CUSTOM_INFO_COLOR));
}

