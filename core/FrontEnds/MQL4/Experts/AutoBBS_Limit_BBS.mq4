#property copyright "Copyright ?2012, Asirikuy Community"
#property link      "http://www.asirikuy.com"

#include "../include/Common.mqh"
#include "../include/Defines.mqh"
#include "../include/UserInterface.mqh"
#include "../include/AsirikuyFramework.mqh"

#define SYSTEM_NAME  "AutoBBS"
#define TOTAL_INTERNAL_STRATEGIES 1

extern string COMMENTS              = "Click \"Modify expert\" to view comments inside the source code.";
extern string SECTION_1             = "################# General Settings #################";
extern string FRAMEWORK_CONFIG      = "C:\\Users\\amo31\\AppData\\Roaming\\MetaQuotes\\Terminal\\188ED535E9CD0077B15997DB4A8036D27\\MQL4\\Files\\AsirikuyConfig.xml";
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
extern string CUSTOM_ORDER_COMMENT_1= "";     // Specify a custom comment for all orders opened by this strategy. If left blank the default comment with be used: "StrategyName Version F4.x.x".

extern string SECTION_2             = "############## Common Strategy Settings ##############";
extern int    STRATEGY_MODE         = 0;      // DISABLE = 0, ENABLE = 1, MONITOR = 2. In MONITOR mode current orders are managed but no new orders are opened.
extern bool   RUN_EVERY_TICK        = false;  // If this is set to false the strategy will only run once per bar.
extern int    STRATEGY_INSTANCE_ID  = 80001;     // Unique identifier for an instance of internal strategy 1.
extern int    STRATEGY_TIMEFRAME    = 15;      // 1hour = 60, 4hour = 240, daily = 1440.
extern double ACCOUNT_RISK_PERCENT  = 0.25;    // Percentage of account risked per trade. Warning: Losses will sometimes exceed this value!
extern double MAX_DRAWDOWN_PERCENT  = 100;    // Worst case scenario derived from monticarlo simulations using the above risk setting.
extern double MAX_SPREAD_PIPS       = 1;    // Max spread in pips. Trades will not be opened when the spread is wider than this value.
extern bool   DISABLE_COMPOUNDING   = false;  // Remove the effect of compounding profits. Calculate order sizes as if the account equity never changes.
extern bool   USE_INSTANCE_BALANCE  = false;  // Each strategy will use is own virtual balance and trade as if there are no other strategies running on the same account.
extern double INIT_INSTANCE_BALANCE = 0;      // Set the initial instance balance. To reset the instance balance at a later date simply select a new instance ID.
extern int    MAX_OPEN_ORDERS       = 50;      // The maximum number of orders the instance is allowed to have open at a time.
extern int    ATR_AVERAGING_PERIOD  = 20;
   
extern string SECTION_3             = "############## Additional Strategy Settings ##############";
extern double AUTOBBS_ADJUSTPOINTS      = 1;   // Add this adjusted points on stop loss and take profit. It can be caculated from Primary ATR? Not sure which one is better?
extern int    AUTOBBS_MOVE_TP           = 0;   // If mode =1, the last trade will be 1:3, if 0, it will be no TP.
extern int    AUTOBBS_TREND_MODE         = 9;
extern double AUTOBBS_STOPLOSS_LEVEL           = 2.0;
extern int    AUTOBBS_LONG_SHORT_MODE    = 0;
extern double AUTOBBS_MAX_ACCOUNT_RISK    = 3;
extern int    AUTOBBS_KEYK               = 1;
extern int    AUTOBBS_ENABLE_ATR_TRADE   = 1;
extern double AUTOBBS_MAX_STRATEGY_RISK  = 1.5;
extern int    AUTOBBS_MACRO_TREND        = 0;
extern int    AUTOBBS_EXECUTION_RATES    = 15;
extern int    AUTOBBS_ONE_SIDE           = 0;
extern int    AUTOBBS_ENABLE_BBS_EXIT       = 1;
extern double AUTOBBS_MACD_EXIT_LEVEL   = 9999;
extern int    AUTOBBS_STARTHOUR          =15;
extern double AUTOBBS_VIRTUAL_BALANCE_TOPUP  =0;
extern string ORDER_TICKET_LIST     = "";   // seperated by ";" IE: 123456;223333;

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
  if(AUTOBBS_TREND_MODE == 0)  
  {
   StringToCharArray("AutoBBS Swing", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 2)  
  {
    StringToCharArray("AutoBBS LIMIT", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 5)  
  {
    StringToCharArray("AutoBBS DayTrading", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 6)  
  {
    StringToCharArray("AutoBBS Pivot", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 9)  
  {
    StringToCharArray("AutoBBS Auto BBS ShortTerm", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 10)  
  {
    StringToCharArray("AutoBBS Weekly Auto", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 15 ||AUTOBBS_TREND_MODE == 16)  
  {
    StringToCharArray("Daily Trading", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }  
  else if(AUTOBBS_TREND_MODE == 21)  
  {
    StringToCharArray("Multiple Days Trading", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  } 
  else if(AUTOBBS_TREND_MODE == 23)  
  {
    StringToCharArray("MACD Trading", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  } 
  else if(AUTOBBS_TREND_MODE == 26)  
  {
    StringToCharArray("AutoBBS Auto BBS LongTerm", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else if(AUTOBBS_TREND_MODE == 101)  
  {
    StringToCharArray("Ichikomo Daily Trading", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
  }
  else    
   StringToCharArray("AutoBBS", strategyStrings [0][IDX_STRATEGY_NAME].a) ; 
   
  StringToCharArray(Symbol(), strategyStrings [0][IDX_TRADE_SYMBOL].a)     ;
  StringToCharArray(ORDER_TICKET_LIST, strategyStrings [0][IDX_ORDER_TICKET_LIST].a)     ;
  StringToCharArray(ALPARI_UK_HISTORIC_DATA, strategyStrings [0][IDX_REFERENCE_BROKER_NAME].a) ;
  StringToCharArray(CUSTOM_ORDER_COMMENT_1, strategyStrings [0][IDX_CUSTOM_ORDER_COMMENT].a) ;
  systemSettings  [0][IDX_HISTORIC_DATA_ID]      = HISTORIC_DATA_ID;
  systemSettings  [0][IDX_UI_X_COORDINATE]       = 0;
  systemSettings  [0][IDX_UI_Y_COORDINATE]       = 0;
  systemSettings  [0][IDX_OPEN_PRICE_COMPLIANT]  = true;
  strategySettings[0][IDX_OPERATIONAL_MODE]      = STRATEGY_MODE;
  strategySettings[0][IDX_RUN_EVERY_TICK]        = RUN_EVERY_TICK;
  strategySettings[0][IDX_STRATEGY_INSTANCE_ID]  = STRATEGY_INSTANCE_ID;
  strategySettings[0][IDX_INTERNAL_STRATEGY_ID]  = AUTOBBS;
  strategySettings[0][IDX_STRATEGY_TIMEFRAME]    = STRATEGY_TIMEFRAME;
  strategySettings[0][IDX_ACCOUNT_RISK_PERCENT]  = ACCOUNT_RISK_PERCENT;
  strategySettings[0][IDX_MAX_DRAWDOWN_PERCENT]  = MAX_DRAWDOWN_PERCENT;
  strategySettings[0][IDX_ATR_AVERAGING_PERIOD]  = ATR_AVERAGING_PERIOD;
  strategySettings[0][IDX_MAX_OPEN_ORDERS]       = MAX_OPEN_ORDERS;
  strategySettings[0][IDX_ORDERINFO_ARRAY_SIZE]  = MAX_OPEN_ORDERS+1;
    
  strategySettings[0][IDX_ADDITIONAL_PARAM_1]    = AUTOBBS_ADJUSTPOINTS;
  strategySettings[0][IDX_ADDITIONAL_PARAM_2]    = AUTOBBS_MOVE_TP;
  strategySettings[0][IDX_ADDITIONAL_PARAM_3]    = AUTOBBS_TREND_MODE;
  strategySettings[0][IDX_ADDITIONAL_PARAM_4]    = AUTOBBS_STOPLOSS_LEVEL;
  strategySettings[0][IDX_ADDITIONAL_PARAM_5]    = AUTOBBS_LONG_SHORT_MODE;
  strategySettings[0][IDX_ADDITIONAL_PARAM_6]    = AUTOBBS_MAX_ACCOUNT_RISK;
  strategySettings[0][IDX_ADDITIONAL_PARAM_7]    = AUTOBBS_KEYK;
  strategySettings[0][IDX_ADDITIONAL_PARAM_8]    = AUTOBBS_ENABLE_ATR_TRADE;
  strategySettings[0][IDX_ADDITIONAL_PARAM_9]    = AUTOBBS_MAX_STRATEGY_RISK;
  strategySettings[0][IDX_ADDITIONAL_PARAM_10]   = AUTOBBS_MACRO_TREND;
  strategySettings[0][IDX_ADDITIONAL_PARAM_11]   = AUTOBBS_EXECUTION_RATES;
  strategySettings[0][IDX_ADDITIONAL_PARAM_12]   = AUTOBBS_ONE_SIDE;
  strategySettings[0][IDX_ADDITIONAL_PARAM_13]   = AUTOBBS_ENABLE_BBS_EXIT;
  strategySettings[0][IDX_ADDITIONAL_PARAM_14]   = AUTOBBS_MACD_EXIT_LEVEL;
  strategySettings[0][IDX_ADDITIONAL_PARAM_15]   = AUTOBBS_STARTHOUR;
  strategySettings[0][IDX_ADDITIONAL_PARAM_16]   = AUTOBBS_VIRTUAL_BALANCE_TOPUP;
      
  StringToCharArray(Symbol(), ratesSymbols[0][0].a ) ;
  ratesInformation[0][0][IDX_IS_ENABLED]         = true;
  ratesInformation[0][0][IDX_REQUIRED_TIMEFRAME] = STRATEGY_TIMEFRAME;
  ratesInformation[0][0][IDX_TOTAL_BARS_REQUIRED]= 400;
  
  StringToCharArray(Symbol(), ratesSymbols[0][1].a ) ;
  ratesInformation[0][1][IDX_IS_ENABLED]         = true;
  
  if( AUTOBBS_EXECUTION_RATES == 60)
  {
      ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_H1;
  }
  else if( AUTOBBS_EXECUTION_RATES == 30)
  {
      ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_M30;
  }
  else if( AUTOBBS_EXECUTION_RATES == 15)
  {
      ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_M15;
  }
  else
  {
      ratesInformation[0][1][IDX_REQUIRED_TIMEFRAME] = PERIOD_M5;
  };
   
  ratesInformation[0][1][IDX_TOTAL_BARS_REQUIRED]= 400;
  
  StringToCharArray(Symbol(), ratesSymbols[0][2].a ) ;
  ratesInformation[0][2][IDX_IS_ENABLED]         = true;
  ratesInformation[0][2][IDX_REQUIRED_TIMEFRAME] = PERIOD_H1;
  ratesInformation[0][2][IDX_TOTAL_BARS_REQUIRED]= 300;
  
  StringToCharArray(Symbol(), ratesSymbols[0][3].a ) ;
  ratesInformation[0][3][IDX_IS_ENABLED]         = true;
  ratesInformation[0][3][IDX_REQUIRED_TIMEFRAME] = PERIOD_H4;
  ratesInformation[0][3][IDX_TOTAL_BARS_REQUIRED]= 300;
  
  StringToCharArray(Symbol(), ratesSymbols[0][4].a ) ;
  ratesInformation[0][4][IDX_IS_ENABLED]         = true;
  ratesInformation[0][4][IDX_REQUIRED_TIMEFRAME] = PERIOD_D1;
  ratesInformation[0][4][IDX_TOTAL_BARS_REQUIRED]= 100;  
  
  StringToCharArray(Symbol(), ratesSymbols[0][5].a ) ;
  ratesInformation[0][5][IDX_IS_ENABLED]         = true;
  ratesInformation[0][5][IDX_REQUIRED_TIMEFRAME] = PERIOD_W1;
  ratesInformation[0][5][IDX_TOTAL_BARS_REQUIRED]= 30; 
  
  StringToCharArray(Symbol(), ratesSymbols[0][6].a ) ;
  ratesInformation[0][6][IDX_IS_ENABLED]         = true;
  ratesInformation[0][6][IDX_REQUIRED_TIMEFRAME] = PERIOD_MN1;
  ratesInformation[0][6][IDX_TOTAL_BARS_REQUIRED]= 5; 
  
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

