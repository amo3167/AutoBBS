/**
 * @file
 * @brief     Common functions used by trading strategies and other libraries.
 * @details   Any functions that are used by more than 1 trading strategy are kept here to avoid code duplication.
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

#property copyright "Copyright © 2012, Asirikuy Community"
#property link      "http://www.asirikuy.com"
#property library

#include "../include/WinUser32.mqh"
#include "../include/stdlib.mqh"
#include "../include/Defines.mqh"
#include "../include/OrderManager.mqh"
#include "../include/UserInterface.mqh"
#include "../include/AsirikuyFramework.mqh"
#include "../include/OrderWrapper.mqh"

bool c_isUsingMode(int mode, int totalInternalStrategies, double& strategySettings[][])
{
  for(int i = 0; i < totalInternalStrategies; i++)
  {
    if(strategySettings[i][IDX_OPERATIONAL_MODE] == mode)
    {
      return(true);
    }
  }
  
  return(false);
}

// Return the Asirikuy Framework version as a string
string c_getFrameworkVersion()
{
  int    major [1] = {0};
  int    minor [1] = {0};
  int    bugfix[1] = {0};
  getFrameworkVersion(major, minor, bugfix);
  return(StringConcatenate("F", major[0], ".", minor[0], ".", bugfix[0]));
}

void c_copyRatesArrays(
  int     strategyIndex, 
  charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
  double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE], 
  MqlRates& rates_0[], 
  MqlRates& rates_1[], 
  MqlRates& rates_2[], 
  MqlRates& rates_3[], 
  MqlRates& rates_4[], 
  MqlRates& rates_5[], 
  MqlRates& rates_6[], 
  MqlRates& rates_7[], 
  MqlRates& rates_8[], 
  MqlRates& rates_9[]) 
{

  int barHours[TOTAL_RATES_ARRAYS];

  if(ratesInformation[strategyIndex][0][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][0][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][0].a), ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][0].a), ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][0][IDX_RATES_ARRAY_SIZE], rates_0);
    barHours[0] = TimeHour(rates_0[0].time);
  }
  if(ratesInformation[strategyIndex][1][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][1][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][1].a), ratesInformation[strategyIndex][1][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][1].a),ratesInformation[strategyIndex][1][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][1][IDX_RATES_ARRAY_SIZE], rates_1);
    barHours[1] = TimeHour(rates_1[0].time);
  }
  if(ratesInformation[strategyIndex][2][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][2][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][2].a), ratesInformation[strategyIndex][2][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][2].a), ratesInformation[strategyIndex][2][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][2][IDX_RATES_ARRAY_SIZE], rates_2);   
    barHours[2] = TimeHour(rates_2[0].time);
  }
  if(ratesInformation[strategyIndex][3][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][3][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][3].a), ratesInformation[strategyIndex][3][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][3].a), ratesInformation[strategyIndex][3][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][3][IDX_RATES_ARRAY_SIZE], rates_3);
    barHours[3] = TimeHour(rates_3[0].time);
  }
  if(ratesInformation[strategyIndex][4][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][4][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][4].a), ratesInformation[strategyIndex][4][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][4].a), ratesInformation[strategyIndex][4][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][4][IDX_RATES_ARRAY_SIZE], rates_4);
    barHours[4] = TimeHour(rates_4[0].time);
  }
  if(ratesInformation[strategyIndex][5][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][5][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][5].a), ratesInformation[strategyIndex][5][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][5].a), ratesInformation[strategyIndex][5][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][5][IDX_RATES_ARRAY_SIZE], rates_5);
    barHours[5] = TimeHour(rates_5[0].time);
  }
  if(ratesInformation[strategyIndex][6][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][6][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][6].a), ratesInformation[strategyIndex][6][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][6].a), ratesInformation[strategyIndex][6][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][6][IDX_RATES_ARRAY_SIZE], rates_6);
    barHours[6] = TimeHour(rates_6[0].time);
  }
  if(ratesInformation[strategyIndex][7][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][7][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][7].a), ratesInformation[strategyIndex][7][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][7].a), ratesInformation[strategyIndex][7][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][7][IDX_RATES_ARRAY_SIZE], rates_7);
    barHours[7] = TimeHour(rates_7[0].time);
  }
  if(ratesInformation[strategyIndex][8][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][8][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][8].a), ratesInformation[strategyIndex][8][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][8].a), ratesInformation[strategyIndex][8][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][8][IDX_RATES_ARRAY_SIZE], rates_8);
    barHours[8] = TimeHour(rates_8[0].time);
  }
  if(ratesInformation[strategyIndex][9][IDX_IS_ENABLED] != false)
  {
    ratesInformation[strategyIndex][9][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][9].a), ratesInformation[strategyIndex][9][IDX_ACTUAL_TIMEFRAME]);
    CopyRates(CharArrayToString(ratesSymbols[strategyIndex][9].a), ratesInformation[strategyIndex][9][IDX_ACTUAL_TIMEFRAME], 0, ratesInformation[strategyIndex][9][IDX_RATES_ARRAY_SIZE], rates_9);
    barHours[9] = TimeHour(rates_9[0].time);
  }

  for(int i = 0; i < TOTAL_RATES_ARRAYS; i++)
  {
    if(ratesInformation[strategyIndex][i][IDX_IS_ENABLED] == false)
    {
      continue;
    }
  
    if(  (ratesInformation[strategyIndex][i][IDX_ACTUAL_TIMEFRAME] == PERIOD_H4)
      && (barHours[i] != 0)
      && (barHours[i] != 4)
      && (barHours[i] != 8)
      && (barHours[i] != 12)
      && (barHours[i] != 16)
      && (barHours[i] != 20))
    {
      Print("Warning: 4 hour bars start at non-standard hours, please use a 1 Hour timeframe and NORMAL_TEST mode to normalize the bars and ensure consistent results.");
    }
  }
}

void c_setCommonStrategySettings(int totalInternalStrategies, charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], double maxSpreadPips, bool disableCompounding)
{
  for(int i = 0; i < totalInternalStrategies; i++)
  {
    int    digits = MarketInfo(CharArrayToString(strategyStrings[i][IDX_TRADE_SYMBOL].a), MODE_DIGITS);
    double point  = MarketInfo(CharArrayToString(strategyStrings[i][IDX_TRADE_SYMBOL].a), MODE_POINT);
    
    strategySettings[i][IDX_MAX_SPREAD_PRICE]        = c_pipsToPoints(maxSpreadPips, digits) * point;
    strategySettings[i][IDX_ORIGINAL_ACCOUNT_EQUITY] = AccountEquity();
    strategySettings[i][IDX_IS_BACKTESTING]          = IsTesting();
    strategySettings[i][IDX_DISABLE_COMPOUNDING]     = disableCompounding;
  }
}

void c_setCommonStrategyStrings(int totalInternalStrategies, charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE])
{
  charArray accountCurrency   [1] ;
         StringToCharArray(AccountInfoString(ACCOUNT_CURRENCY), accountCurrency[0].a) ;
  charArray baseConversionSymbol [1];
  StringToCharArray("1",  baseConversionSymbol [0].a); /* Must allocate enough space to store the strings */
  charArray quoteConversionSymbol[1]; /* Different initial values used to prevent the compiler from optimizing */
  StringToCharArray("2",  quoteConversionSymbol[0].a);
  
  for(int i = 0; i < totalInternalStrategies; i++)
  {
    charArray symbol[1] ;
              symbol[0] = strategyStrings[i][IDX_TRADE_SYMBOL];
              
    mql5_getConversionSymbols(symbol, accountCurrency, baseConversionSymbol, quoteConversionSymbol);
    
    StringToCharArray(c_getFrameworkVersion(),  strategyStrings[i][IDX_FRAMEWORK_VERSION].a );
    
    strategyStrings[i][IDX_ACCOUNT_CURRENCY]        = accountCurrency[0];
    strategyStrings[i][IDX_BASE_CONVERSION_SYMBOL]  = baseConversionSymbol[0];
    strategyStrings[i][IDX_QUOTE_CONVERSION_SYMBOL] = quoteConversionSymbol[0];
    
    if(!IsTesting())
    {
      StringToCharArray(AccountInfoString(ACCOUNT_COMPANY),  strategyStrings[i][IDX_BROKER_NAME].a);
    }
    else if(systemSettings[i][IDX_HISTORIC_DATA_ID] == ALPARI_UK_HISTORIC_DATA_ID)
    {
      StringToCharArray(ALPARI_UK_HISTORIC_DATA,  strategyStrings[i][IDX_BROKER_NAME].a);
    }
    else if(systemSettings[i][IDX_HISTORIC_DATA_ID] == FOREX_HISTORICAL_DATA_ID)
    {
      StringToCharArray(FOREX_HISTORICAL_DATA,  strategyStrings[i][IDX_BROKER_NAME].a);
    }
    else //if(systemSettings[i][IDX_HISTORIC_DATA_ID] == BROKER_DATA_ID)
    {    
      StringToCharArray(TerminalInfoString(TERMINAL_COMPANY),  strategyStrings[i][IDX_BROKER_NAME].a);
    }
  }
}

void c_setCommonSystemSettings(int     totalInternalStrategies, 
                               charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
                               double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
                               double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
                               double  maxSlippagePips, 
                               bool    enableScreenshots, 
                               bool    useInstanceBalance, 
                               double  initInstanceBalance) 
{
  for(int i = 0; i < totalInternalStrategies; i++)
  {
    systemSettings[i][IDX_DIGITS]                  = MarketInfo(CharArrayToString(strategyStrings[i][IDX_TRADE_SYMBOL].a), MODE_DIGITS);
    systemSettings[i][IDX_MAX_SLIPPAGE]            = c_pipsToPoints(maxSlippagePips, systemSettings[i][IDX_DIGITS]);
    systemSettings[i][IDX_ENABLE_SCREENSHOTS]      = enableScreenshots;
    systemSettings[i][IDX_RANDOMIZED_INSTANCE_ID]  = 0;
    systemSettings[i][IDX_USE_INSTANCE_BALANCE]    = useInstanceBalance;
    systemSettings[i][IDX_LAST_INSTANCE_BALANCE]   = initInstanceBalance;
    systemSettings[i][IDX_NEXT_CLOSED_ORDER_INDEX] = 0;
  }
}

void c_setCommonRatesInfoSettings(int totalInternalStrategies, double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE]) 
{
  for(int i = 0; i < totalInternalStrategies; i++)
  {
    for(int j = 0; j < TOTAL_RATES_ARRAYS; j++)
    {
      if(ratesInformation[i][j][IDX_IS_ENABLED] == false)
      {
        continue;
      }
      
      // For strategy timeframes greater than 1 hour the rates timeframe is reduced to 1 hour to enable normalization
      if(ratesInformation[i][j][IDX_REQUIRED_TIMEFRAME] > PERIOD_H1)
      {
        ratesInformation[i][j][IDX_ACTUAL_TIMEFRAME] = PERIOD_H1;
      }
      else
      {
        ratesInformation[i][j][IDX_ACTUAL_TIMEFRAME] = ratesInformation[i][j][IDX_REQUIRED_TIMEFRAME];
      }
    
      ratesInformation[i][j][IDX_POINT] = MarketInfo(CharArrayToString(ratesSymbols[i][j].a), MODE_POINT);
      ratesInformation[i][j][IDX_DIGITS_RATES] = MarketInfo(CharArrayToString(ratesSymbols[i][j].a), MODE_DIGITS);
      
      // Increase the required bars for historic analysis
      if((strategySettings[i][IDX_WFO_PSET_MANAGEMENT] != PSMM_NONE) || (strategySettings[i][IDX_INSTANCE_MANAGEMENT] == IMM_GAME_THEORY))
      {
        ratesInformation[i][j][IDX_TOTAL_BARS_REQUIRED] += strategySettings[i][IDX_ANALYSIS_WINDOW_SIZE] * PERIOD_D1 / ratesInformation[i][j][IDX_REQUIRED_TIMEFRAME];
      }
    }
  }
}


bool c_init(
  int     totalInternalStrategies,
  int     useOrderWrapperInterface,
  string  frameworkConfig, 
  charArray& strategyStrings[][], 
  double& systemSettings[][], 
  double& strategySettings[][], 
  charArray& ratesSymbols[][], 
  double& ratesInformation[][][], 
  double& profitDrawdown[][], 
  double  maxSlippagePips, 
  double  maxSpreadPips, 
  bool    enableScreenshots, 
  bool    disableCompounding, 
  bool    useInstanceBalance, 
  double  initInstanceBalance, 
  int     fontSize, 
  color   systemInfoColor, 
  color   profitInfoColor, 
  color   tradeInfoColor, 
  color   errorInfoColor, 
  color   customInfoColor) 
{
  int i ;
  
  for(i = 0; i < totalInternalStrategies; i++)
  {
    static int previousReturnCode = SUCCESS;
    
    charArray configString[1];
    StringToCharArray(frameworkConfig, configString[0].a) ;
    
    int returnCode = initInstanceMQL5(strategySettings[i][IDX_STRATEGY_INSTANCE_ID], (IsTesting() || IsOptimization()), configString[0].a);
    
    if(returnCode != SUCCESS)
    {
      if((returnCode != previousReturnCode) && (returnCode != WAIT_FOR_INIT))
      {
        Alert("Error initializing framework. Instance ID = ", strategySettings[i][IDX_STRATEGY_INSTANCE_ID], ". Error = ", c_getErrorDescription(returnCode), ".");
        previousReturnCode = returnCode;
      }
      return(false);
    }
  }
  
  initOrderWrapper(useOrderWrapperInterface);

  c_setCommonStrategyStrings(totalInternalStrategies, strategyStrings, systemSettings, strategySettings);
  c_setCommonSystemSettings(totalInternalStrategies, strategyStrings, systemSettings, strategySettings, maxSlippagePips, enableScreenshots, useInstanceBalance, initInstanceBalance);
  c_setCommonStrategySettings(totalInternalStrategies, strategyStrings, systemSettings, strategySettings, maxSpreadPips, disableCompounding);
  c_setCommonRatesInfoSettings(totalInternalStrategies, strategySettings, ratesSymbols, ratesInformation);
  
  ui_init(totalInternalStrategies, strategyStrings, systemSettings, strategySettings, fontSize, systemInfoColor, profitInfoColor, tradeInfoColor, errorInfoColor, customInfoColor);
  
  ArrayInitialize(profitDrawdown, 0);
  
  int chartTimeframe = Period();
  
  for(i = 0; i < totalInternalStrategies; i++)
  {
    if((strategySettings[i][IDX_STRATEGY_INSTANCE_ID] < 0) && (strategySettings[i][IDX_OPERATIONAL_MODE] != DISABLE))
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_INVALID_INSTANCE_ID), systemSettings[i][IDX_UI_X_COORDINATE], systemSettings[i][IDX_UI_Y_COORDINATE], strategySettings[i][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: ", c_getErrorDescription(ERROR_INVALID_INSTANCE_ID));
      return(false);
    }
    
    if(chartTimeframe > strategySettings[i][IDX_STRATEGY_TIMEFRAME])
    {
      Alert("Setup error: The chart timeframe must be less than or equal to the strategy timeframe. Chart timeframe = ", chartTimeframe, ",  Strategy timeframe = ", strategySettings[i][IDX_STRATEGY_TIMEFRAME]);
      return(false);
    }
    else if(chartTimeframe > PERIOD_H1)
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_BAD_HISTORIC_DATA_ID), systemSettings[i][IDX_UI_X_COORDINATE], systemSettings[i][IDX_UI_Y_COORDINATE], strategySettings[i][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: The chart timeframe must be less than or equal to 60 minutes, even for daily or higher timeframe systems. Chart timeframe = ", chartTimeframe);
      return(false);
    }
  
    if((systemSettings[i][IDX_HISTORIC_DATA_ID] < 0) || (systemSettings[i][IDX_HISTORIC_DATA_ID] >= TOTAL_DATA_IDS))
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_BAD_HISTORIC_DATA_ID), systemSettings[i][IDX_UI_X_COORDINATE], systemSettings[i][IDX_UI_Y_COORDINATE], strategySettings[i][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: HISTORIC_DATA_ID is invalid. Please select a value between 0 and ", TOTAL_DATA_IDS - 1, ".");
      return(false);
    }
    
    if((strategySettings[i][IDX_OPERATIONAL_MODE] < DISABLE) || (strategySettings[i][IDX_OPERATIONAL_MODE] > MONITOR))
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_INVALID_MODE), systemSettings[i][IDX_UI_X_COORDINATE], systemSettings[i][IDX_UI_Y_COORDINATE], strategySettings[i][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: Valid operational modes include 0 - Disable, 1 - Enable, 2 - Monitor. Current mode = ");
      return(false);
    }
  }
  
  return(true);
}

// Return the absolute value of 1 pip
double c_pipValue(int digits)
{
  // Adjust for 4 and 5 digit brokers and for currencies with less digits after the decimal separator.
  // Warning: This works for the major and minor currency pairs including JPY but is not correct
  //          for all exotic pairs.
  if(digits >= 4)
  {
    return(0.0001);
  }
  
  return(0.01);
}

// Calculate the difference in pips between the order entry price and the SL price
double c_stopLossPriceToPips(int orderType, double orderEntryPrice, double stopLossPrice, int digits)
{
  if((orderType == OP_BUY) || (orderType == OP_BUYLIMIT) || (orderType == OP_BUYSTOP))
  {
    return(NormalizeDouble(orderEntryPrice - stopLossPrice, digits) / c_pipValue(digits));
  }
    
  if((orderType == OP_SELL) || (orderType == OP_SELLLIMIT) || (orderType == OP_SELLSTOP))
  {
	 return(NormalizeDouble(stopLossPrice - orderEntryPrice, digits) / c_pipValue(digits));
  }

  return(-1);
}

// Calculate the difference in pips between the order entry price and the TP price
double c_takeProfitPriceToPips(int orderType, double orderEntryPrice, double takeProfitPrice, int digits)
{
  if((orderType == OP_BUY) || (orderType == OP_BUYLIMIT) || (orderType == OP_BUYSTOP))
  {
    return(NormalizeDouble(takeProfitPrice - orderEntryPrice, digits) / c_pipValue(digits));
  }
    
  if((orderType == OP_SELL) || (orderType == OP_SELLLIMIT) || (orderType == OP_SELLSTOP))
  {
	 return(NormalizeDouble(orderEntryPrice - takeProfitPrice, digits) / c_pipValue(digits));
  }
  
  return(-1);
}

// Convert pips to points.
int c_pipsToPoints(int pips, int digits)
{
  // Warning: This function will work for majors and minors but 
  //          may not be appropriate for some exotic currency pairs
  if(digits == 3 || digits == 5)
  {
    return(pips * 10);
  }
  
  return(pips);
}

// Get bid & ask prices for the trade symbol and conversion symbols.
void c_getBidAsk(string tradingSymbol, string accountCurrency, string baseConversionSymbol, string quoteConversionSymbol, double& bidAsk[])
{
  static bool checkAccountCurrency = true;
  static bool getConversionRates   = true;
  
  if(checkAccountCurrency)
  {
    if(StringFind(tradingSymbol, accountCurrency) >= 0)
    {
      getConversionRates = false;
    }
      
    checkAccountCurrency = false;
  }
  
  bidAsk[IDX_BID] = MarketInfo(tradingSymbol, MODE_BID);
  bidAsk[IDX_ASK] = MarketInfo(tradingSymbol, MODE_ASK);
  
  if(!getConversionRates)
  {
    return;
  }
    
  bidAsk[IDX_BASE_CONVERSION_BID]  = MarketInfo(baseConversionSymbol, MODE_BID);
  bidAsk[IDX_BASE_CONVERSION_ASK]  = MarketInfo(baseConversionSymbol, MODE_ASK);
  bidAsk[IDX_QUOTE_CONVERSION_BID] = MarketInfo(quoteConversionSymbol, MODE_BID);
  bidAsk[IDX_QUOTE_CONVERSION_ASK] = MarketInfo(quoteConversionSymbol, MODE_ASK);
  
  // If the bid and ask are unavailable use the open price of the current bar instead.
  if((bidAsk[IDX_BASE_CONVERSION_BID] == 0) || (bidAsk[IDX_BASE_CONVERSION_ASK] == 0))
  {
    bidAsk[IDX_BASE_CONVERSION_BID] = iOpen(baseConversionSymbol, 0, 0);
    bidAsk[IDX_BASE_CONVERSION_ASK] = iOpen(baseConversionSymbol, 0, 0);
  }
  
  if((bidAsk[IDX_QUOTE_CONVERSION_BID] == 0) || (bidAsk[IDX_QUOTE_CONVERSION_ASK] == 0))
  {
    bidAsk[IDX_QUOTE_CONVERSION_BID] = iOpen(quoteConversionSymbol, 0, 0);
    bidAsk[IDX_QUOTE_CONVERSION_ASK] = iOpen(quoteConversionSymbol, 0, 0);
  }
}

// Return the total open orders for each order type.
void c_countOpenOrders(string symbol, int strategyInstanceId, int& ordersCount[])
{

int i;

  for(i = 0; i < TOTAL_ORDER_TYPES; i++)
  {
    ordersCount[i] = 0;
  }
    
  int total = nOrdersTotal();
  
  for(i = 0; i < total; i++) 
  {
    nOrderSelect(i, SELECT_BY_POS, MODE_TRADES);
    
    if(nOrderSymbol() == symbol && nOrderMagicNumber() == strategyInstanceId)
    {
      ordersCount[nOrderType()]++;
    }
  }
}

// Get information about the most recent orders. If there are no open orders then info will
// be taken from the most recently closed orders. If there are no closed orders either then all values are set to 0.
void c_getOrderInfo(string symbol, int strategyIndex, double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], double& orderInfo[][TOTAL_ORDER_INFO_INDEXES])
{
  int orderCount        = 0;
  int totalOpenOrders   = nOrdersTotal();
  int totalClosedOrders = nOrdersHistoryTotal();
  int i;
  
  for(i = totalOpenOrders - 1; (i >= 0) && (orderCount < strategySettings[strategyIndex][IDX_ORDERINFO_ARRAY_SIZE]); i--)
  {
    nOrderSelect(i, SELECT_BY_POS, MODE_TRADES);
    
    if(  (nOrderSymbol() == symbol)
      && (MathAbs(nOrderMagicNumber() - strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID]) < EPSILON)
      && ((nOrderType() == OP_BUY) || (nOrderType() == OP_SELL)))
    {
      orderInfo[orderCount][IDX_ORDER_TICKET]      = nOrderTicket();
      orderInfo[orderCount][IDX_ORDER_INSTANCE_ID] = nOrderMagicNumber();
      orderInfo[orderCount][IDX_ORDER_TRADE_TYPE]  = nOrderType();
      orderInfo[orderCount][IDX_ORDER_OPEN_TIME]   = nOrderOpenTime();
      orderInfo[orderCount][IDX_ORDER_CLOSE_TIME]  = nOrderCloseTime();
      orderInfo[orderCount][IDX_ORDER_STOPLOSS]    = nOrderStopLoss();
      orderInfo[orderCount][IDX_ORDER_TAKEPROFIT]  = nOrderTakeProfit();
      orderInfo[orderCount][IDX_ORDER_EXPIRATION]  = nOrderExpiration();
      orderInfo[orderCount][IDX_ORDER_OPEN_PRICE]  = nOrderOpenPrice();
      orderInfo[orderCount][IDX_ORDER_CLOSE_PRICE] = nOrderClosePrice();
      orderInfo[orderCount][IDX_ORDER_LOTS]        = nOrderLots();
      orderInfo[orderCount][IDX_ORDER_PROFIT]      = nOrderProfit();
      orderInfo[orderCount][IDX_ORDER_COMMISSION]  = nOrderCommission();
      orderInfo[orderCount][IDX_ORDER_SWAP]        = 0;
      orderInfo[orderCount][IDX_ORDER_IS_OPEN]     = true;
      orderCount++;
    }
  }
  
  for(i = totalClosedOrders - 1; (i >= 0) && (orderCount < strategySettings[strategyIndex][IDX_ORDERINFO_ARRAY_SIZE]); i--)
  {
    nOrderSelect(i, SELECT_BY_POS, MODE_HISTORY);
    
    if(  (nOrderSymbol() == symbol)
      && (MathAbs(nOrderMagicNumber() - strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID]) < EPSILON)
      && ((nOrderType() == OP_BUY) || (nOrderType() == OP_SELL)))
    {
      orderInfo[orderCount][IDX_ORDER_TICKET]      = nOrderTicket();
      orderInfo[orderCount][IDX_ORDER_INSTANCE_ID] = nOrderMagicNumber();
      orderInfo[orderCount][IDX_ORDER_TRADE_TYPE]  = nOrderType();
      orderInfo[orderCount][IDX_ORDER_OPEN_TIME]   = nOrderOpenTime();
      orderInfo[orderCount][IDX_ORDER_CLOSE_TIME]  = nOrderCloseTime();
      orderInfo[orderCount][IDX_ORDER_STOPLOSS]    = nOrderStopLoss();
      orderInfo[orderCount][IDX_ORDER_TAKEPROFIT]  = nOrderTakeProfit();
      orderInfo[orderCount][IDX_ORDER_EXPIRATION]  = nOrderExpiration();
      orderInfo[orderCount][IDX_ORDER_OPEN_PRICE]  = nOrderOpenPrice();
      orderInfo[orderCount][IDX_ORDER_CLOSE_PRICE] = nOrderClosePrice();
      orderInfo[orderCount][IDX_ORDER_LOTS]        = nOrderLots();
      orderInfo[orderCount][IDX_ORDER_PROFIT]      = nOrderProfit();
      orderInfo[orderCount][IDX_ORDER_COMMISSION]  = nOrderCommission();
      orderInfo[orderCount][IDX_ORDER_SWAP]        = 0;
      orderInfo[orderCount][IDX_ORDER_IS_OPEN]     = false;
      orderCount++;
    }
  }
  
  if(orderCount < strategySettings[strategyIndex][IDX_ORDERINFO_ARRAY_SIZE])
  {
    for(i = orderCount; i < strategySettings[strategyIndex][IDX_ORDERINFO_ARRAY_SIZE]; i++)
    {
      orderInfo[i][IDX_ORDER_TICKET]      = 0;
      orderInfo[i][IDX_ORDER_INSTANCE_ID] = 0;
      orderInfo[i][IDX_ORDER_TRADE_TYPE]  = 0;
      orderInfo[i][IDX_ORDER_OPEN_TIME]   = 0;
      orderInfo[i][IDX_ORDER_CLOSE_TIME]  = 0;
      orderInfo[i][IDX_ORDER_STOPLOSS]    = 0;
      orderInfo[i][IDX_ORDER_TAKEPROFIT]  = 0;
      orderInfo[i][IDX_ORDER_EXPIRATION]  = 0;
      orderInfo[i][IDX_ORDER_OPEN_PRICE]  = 0;
      orderInfo[i][IDX_ORDER_CLOSE_PRICE] = 0;
      orderInfo[i][IDX_ORDER_LOTS]        = 0;
      orderInfo[i][IDX_ORDER_PROFIT]      = 0;
      orderInfo[i][IDX_ORDER_COMMISSION]  = 0;
      orderInfo[i][IDX_ORDER_SWAP]        = 0;
      orderInfo[i][IDX_ORDER_IS_OPEN]     = 0;
    }
  }
}

// Return the total percentage of the account that would be lost if all open orders hit their stoploss.
// This information is used to protect against a stopout.
double c_totalOpenTradeRiskPercent()
{
  double totalRisk      = 0;
  double accountBalance = AccountInfoDouble(ACCOUNT_BALANCE);
  int    totalOrders    = nOrdersTotal();
  
  for(int i = 0; i < totalOrders; i++)
  {
    if(nOrderSelect(i, SELECT_BY_POS, MODE_TRADES))
    {
      totalRisk += c_orderOpenBalance(nOrderTicket(), accountBalance) * 0.01 * GlobalVariableGet(nOrderMagicNumber() + "_AccountRiskPercent");
    }
  }
  
  return(100 * totalRisk / accountBalance);
}

// Return the margin level at which the broker will start closing orders.
double c_stopoutPercent()
{
  static int stopoutMode = -1, stopoutLevel = -1;
  if((stopoutMode == -1) || (stopoutLevel == -1))
  {
    stopoutMode  = AccountInfoInteger(ACCOUNT_MARGIN_SO_MODE);
    stopoutLevel = AccountInfoDouble(ACCOUNT_MARGIN_SO_SO);
  }
  
  if(stopoutMode == 0)
  {
    return(stopoutLevel);
  }
  
  if(stopoutMode == 1)
  {
    return(stopoutLevel / AccountInfoDouble(ACCOUNT_EQUITY));
  }
    
  return(0);
}

int c_getRandomInstanceId()
{
  int count;

  // The following if statement creates or increases
  // the "count" variable which is then used as a part of the "seed"
  // for the random number generator, this count ensures that 
  // random numbers remain unique and duplicates identified even if the instances are started
  // at exactly the same time on the same instrument.
  if(GlobalVariableCheck("rdn_gen_count"))
  {
    count = GlobalVariableGet("rdn_gen_count");
    if(count < 100)
    {
      GlobalVariableSet("rdn_gen_count", count + 1);
    }
    if(count >= 100)
    {
      GlobalVariableSet("rdn_gen_count", 1);
    }
  }
  else
  {
    GlobalVariableSet("rdn_gen_count", 1);
    count = 1 ;
  }

  // Random number generator seed, current time, Ask and counter are used
  MathSrand(TimeLocal() * MarketInfo(Symbol(), MODE_ASK) * count);
	
  // Generate and return a random number
  return(MathRand());
}

// Verifies that the tag, generated during initialization, has changed.
bool c_isUniqueInstanceId(int strategyIndex, double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE])
{
  // Retrieve instance ID as string to search for global variable
  string instanceIDTag = DoubleToString(strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID], 0);

  if(systemSettings[strategyIndex][IDX_RANDOMIZED_INSTANCE_ID] < EPSILON)
  {
    // Assign a random instance ID
    systemSettings[strategyIndex][IDX_RANDOMIZED_INSTANCE_ID] = c_getRandomInstanceId();
    GlobalVariableSet(instanceIDTag, systemSettings[strategyIndex][IDX_RANDOMIZED_INSTANCE_ID]);
  }
  
  // Assign the value of the global variable
  double retrievedInstanceID = GlobalVariableGet(instanceIDTag);

  // Check whether the tag has changed from what it had originally been assigned to
  if(MathAbs(systemSettings[strategyIndex][IDX_RANDOMIZED_INSTANCE_ID] - retrievedInstanceID) >= EPSILON)
  {
    // Return false if a duplicate instance is found
    return(false);
  }

  // Reassigning global variable, this does not change the variable's value,
  // however it needs to be done since unmodified variables are deleted
  // after 4 weeks. This "regeneration" avoids deletion.
  GlobalVariableSet(instanceIDTag, retrievedInstanceID);
	
  return(true);
}

double c_orderOpenBalance(int ticket, double orderCloseBalance)
{
  if(!nOrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES))
  {
    return(-1);
  }
    
  datetime orderOpenTime    = nOrderOpenTime();
  datetime orderCloseTime   = nOrderCloseTime();
  double   orderProfit      = nOrderProfit() + nOrderCommission();
  double   orderOpenBalance = orderCloseBalance;
    
  int total = nOrdersHistoryTotal();
  for(int i = total - 1; i >= 0; i--)
  {
    if(!nOrderSelect(i,SELECT_BY_POS,MODE_HISTORY))
    {
      continue;
    }
        
    if(nOrderCloseTime() <= orderOpenTime)
    {
      break;
    }
      
    if(nOrderCloseTime() < orderCloseTime)
    {
      orderOpenBalance -= nOrderProfit() + nOrderCommission();
    }  
  }
  
  nOrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES);
  return(orderOpenBalance);
}

// Look at the order history and calculate the profit and drawdown statistics for a trading strategy.
// This information is displayed on the UI and can also be used to detect a worst case scenario and halt trading.
void c_calculateProfitAndDrawdown(string symbol, int strategyIndex, int strategyInstanceId, double &profitDrawdown[][PROFIT_DRAWDOWN_ARRAY_SIZE])
{
  int      newOrdersHistoryTotal  = nOrdersHistoryTotal();
  double   orderCloseBalance      = AccountInfoDouble(ACCOUNT_BALANCE);
  datetime currentTime            = TimeCurrent();
  
  if(profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_TIME] == 0)
  {
    profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_TIME] = currentTime;
  }
    
  if(profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_TIME] == 0)
  {
    profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_TIME] = currentTime;
  }
  
  for(int i = newOrdersHistoryTotal - 1; i >= profitDrawdown[strategyIndex][IDX_ORDERS_HISTORY_TOTAL]; i--)
  {
    if(!nOrderSelect(i,SELECT_BY_POS,MODE_HISTORY))
    {
      continue;
    }

    if(nOrderMagicNumber()!= strategyInstanceId)
    {
      continue;
    }
    
    datetime orderOpenTime      = nOrderOpenTime();
    datetime orderCloseTime     = nOrderCloseTime();
    double   orderProfit        = nOrderProfit() + nOrderCommission();
             orderCloseBalance -= orderProfit;
    double   orderOpenBalance   = c_orderOpenBalance(nOrderTicket(), orderCloseBalance);
    
    profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT] += 100 * orderProfit / orderOpenBalance;
    
    if(profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT] > profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_PERCENT])
    {
      profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_PERCENT] = profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT];
      profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_TIME]    = currentTime;
    }
          
    profitDrawdown[strategyIndex][IDX_CURRENT_DRAWDOWN_PERCENT] = profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_PERCENT] - profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT];
    
    if(profitDrawdown[strategyIndex][IDX_CURRENT_DRAWDOWN_PERCENT] > profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_PERCENT])
    {
      profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_PERCENT] = profitDrawdown[strategyIndex][IDX_CURRENT_DRAWDOWN_PERCENT];
      profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_TIME]    = currentTime;
    }
  }
  
  profitDrawdown[strategyIndex][IDX_ORDERS_HISTORY_TOTAL] = newOrdersHistoryTotal;
}

// Return true if the settings are valid, otherwise return false.
bool c_validateSystemSettings(int strategyIndex, 
                              double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
                              double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
                              charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
                              charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
                              double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE]) 
{
  if(!strategySettings[strategyIndex][IDX_IS_BACKTESTING]) 
  {
    if(!c_isUniqueInstanceId(strategyIndex, systemSettings, strategySettings))
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_DUPLICATE_INSTANCE_ID), systemSettings[strategyIndex][IDX_UI_X_COORDINATE], systemSettings[strategyIndex][IDX_UI_Y_COORDINATE], strategySettings[strategyIndex][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: ", c_getErrorDescription(ERROR_DUPLICATE_INSTANCE_ID));
      return(false);
    }
  }
  
  int requiredBars = 0;
  for(int i = 0; i < TOTAL_RATES_ARRAYS; i++)
  {
    if(ratesInformation[strategyIndex][i][IDX_IS_ENABLED] == false)
    {
      continue;
    }
    
    // Ensure there is enough bars for upconverting to the required timeframe and removing weekend bars
    requiredBars = ratesInformation[strategyIndex][i][IDX_TOTAL_BARS_REQUIRED] * WEEKEND_BAR_MULTIPLIER * ratesInformation[strategyIndex][i][IDX_REQUIRED_TIMEFRAME] / ratesInformation[strategyIndex][i][IDX_ACTUAL_TIMEFRAME];
    
    if(iBars(CharArrayToString(ratesSymbols[strategyIndex][i].a), ratesInformation[strategyIndex][i][IDX_ACTUAL_TIMEFRAME]) < requiredBars)
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_INSUFFICIENT_BARS), systemSettings[strategyIndex][IDX_UI_X_COORDINATE], systemSettings[strategyIndex][IDX_UI_Y_COORDINATE], strategySettings[strategyIndex][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: ", c_getErrorDescription(ERROR_INSUFFICIENT_BARS));
      return(false);
    }
  }

  if(StringFind(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), CharArrayToString(strategyStrings[strategyIndex][IDX_ACCOUNT_CURRENCY].a)) == -1)
  {
    if(  iOpen(CharArrayToString(strategyStrings[strategyIndex][IDX_BASE_CONVERSION_SYMBOL].a), ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME], 0)  <= 0
      && iOpen(CharArrayToString(strategyStrings[strategyIndex][IDX_QUOTE_CONVERSION_SYMBOL].a), ratesInformation[strategyIndex][0][IDX_ACTUAL_TIMEFRAME], 0) <= 0)
    {
      ui_setErrorInfo(c_getErrorDescription(ERROR_NO_CONVERSION_SYMBOL), systemSettings[strategyIndex][IDX_UI_X_COORDINATE], systemSettings[strategyIndex][IDX_UI_Y_COORDINATE], strategySettings[strategyIndex][IDX_OPERATIONAL_MODE]);
      Alert("Setup error: ", c_getErrorDescription(ERROR_NO_CONVERSION_SYMBOL));
      return(false);
    }
  }

  return(true);
}

int c_getOpenOrderType(int tradingSignals)
{
//Print(tradingSignals, "   ", SIGNAL_OPEN_BUY);
//Print(tradingSignals, "   ", SIGNAL_OPEN_SELL);

  if((tradingSignals & SIGNAL_OPEN_BUY) != 0)
  {
    return(OP_BUY);
  }
  else if((tradingSignals & SIGNAL_OPEN_SELL) != 0)
  {
    return(OP_SELL);
  }
  else if((tradingSignals & SIGNAL_OPEN_BUYLIMIT) != 0)
  {
    return(OP_BUYLIMIT);
  }
  else if((tradingSignals & SIGNAL_OPEN_SELLLIMIT) != 0)
  {
    return(OP_SELLLIMIT);
  }
  else if((tradingSignals & SIGNAL_OPEN_BUYSTOP) != 0)
  {
    return(OP_BUYSTOP);
  }
  else if((tradingSignals & SIGNAL_OPEN_SELLSTOP) != 0)
  {
    return(OP_SELLSTOP);
  }
  else
  {
    return(-1);
  }
}

void c_processOrders(int strategyIndex, double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], double& strategyResults[][RESULTS_ARRAY_SIZE])
{
  int returnCode = om_handleOpenOrders(strategyIndex, systemSettings, strategySettings, strategyStrings, strategyResults);
  c_processReturnCode(returnCode, strategyIndex, systemSettings, strategySettings);
  
  int i;
  
  // Check for new trade open signals
  for(i = 0; i < strategySettings[strategyIndex][IDX_MAX_OPEN_ORDERS]; i++)
  {
    int tradingSignals = strategyResults[i][IDX_TRADING_SIGNALS];
    int orderType      = c_getOpenOrderType(tradingSignals);

    if(orderType != -1)
    {
      returnCode = om_openOrder(orderType, strategyIndex, i, systemSettings, strategySettings, strategyStrings, strategyResults);
      c_processReturnCode(returnCode, strategyIndex, systemSettings, strategySettings);
    }
  }
  
  for(i = 0; i < strategySettings[strategyIndex][IDX_MAX_OPEN_ORDERS]; i++)
 { 
      strategyResults[i][IDX_BROKER_SL] = 0;
      strategyResults[i][IDX_BROKER_TP] = 0;
 }
 
}

double c_getInstanceBalance(int strategyIndex, double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE])
{
  for (; systemSettings[strategyIndex][IDX_NEXT_CLOSED_ORDER_INDEX] < nOrdersHistoryTotal(); systemSettings[strategyIndex][IDX_NEXT_CLOSED_ORDER_INDEX]++) 
  {
    if(nOrderSelect(systemSettings[strategyIndex][IDX_NEXT_CLOSED_ORDER_INDEX], SELECT_BY_POS, MODE_HISTORY)) 
    {
      if(nOrderMagicNumber() == strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID])
      {
        systemSettings[strategyIndex][IDX_LAST_INSTANCE_BALANCE] += nOrderProfit() + nOrderCommission();
      }
    }     
  }
  
  return(systemSettings[strategyIndex][IDX_LAST_INSTANCE_BALANCE]);
}

void c_updateStrategyParameters(double& bidAsk[], double& orderInfo[][TOTAL_ORDER_INFO_INDEXES], int& currentBrokerTime[], int& ordersCount[], int strategyIndex, double& accountInfo[], double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], double& profitDrawdown[][PROFIT_DRAWDOWN_ARRAY_SIZE], charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE])
{
  // Renew the global variable so that it never expires.
  GlobalVariableSet(strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID] + "_AccountRiskPercent", strategySettings[strategyIndex][IDX_ACCOUNT_RISK_PERCENT]);
 
  c_calculateProfitAndDrawdown(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), strategyIndex, strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID], profitDrawdown);
  c_getOrderInfo(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), strategyIndex, strategySettings, orderInfo);
  c_countOpenOrders(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID], ordersCount);
  c_getBidAsk(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), CharArrayToString(strategyStrings[strategyIndex][IDX_ACCOUNT_CURRENCY].a), CharArrayToString(strategyStrings[strategyIndex][IDX_BASE_CONVERSION_SYMBOL].a), CharArrayToString(strategyStrings[strategyIndex][IDX_QUOTE_CONVERSION_SYMBOL].a), bidAsk);
  
  currentBrokerTime[0]                      = TimeCurrent();
  
  accountInfo[IDX_ACCOUNT_NUMBER]           =  AccountInfoInteger(ACCOUNT_LOGIN);
  accountInfo[IDX_EQUITY]                   =  AccountInfoDouble(ACCOUNT_EQUITY);
  accountInfo[IDX_MARGIN]                   =  AccountInfoDouble(ACCOUNT_MARGIN);
  accountInfo[IDX_BALANCE]                  = AccountInfoDouble(ACCOUNT_BALANCE);
  accountInfo[IDX_LEVERAGE]                 = AccountInfoInteger(ACCOUNT_LEVERAGE);
  accountInfo[IDX_CONTRACT_SIZE]            = MarketInfo(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), MODE_LOTSIZE);
  accountInfo[IDX_MINIMUM_STOP]             = MarketInfo(CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a), MODE_STOPLEVEL) * ratesInformation[strategyIndex][0][IDX_POINT];
  accountInfo[IDX_STOPOUT_PERCENT]          = c_stopoutPercent();
  accountInfo[IDX_TOTAL_TRADE_RISK_PERCENT] = c_totalOpenTradeRiskPercent();
  accountInfo[IDX_LARGEST_DD_PERCENT]       = profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_PERCENT];
  
  if(systemSettings[strategyIndex][IDX_USE_INSTANCE_BALANCE] != 0)
  {
    accountInfo[IDX_BALANCE] = c_getInstanceBalance(strategyIndex, systemSettings, strategySettings);
    accountInfo[IDX_EQUITY]  = accountInfo[IDX_BALANCE];
  }
  
  for(int i = 0; i < TOTAL_RATES_ARRAYS; i++)
  {
    if(ratesInformation[strategyIndex][i][IDX_IS_ENABLED] != false)
    {
      ratesInformation[strategyIndex][i][IDX_RATES_ARRAY_SIZE] = iBars(CharArrayToString(ratesSymbols[strategyIndex][i].a), ratesInformation[strategyIndex][i][IDX_ACTUAL_TIMEFRAME]);
    }
  }
}

void c_runStrategy(int strategyIndex, double& systemSettings[][], double& strategySettings[][], charArray& strategyStrings[][], double& profitDrawdown[][], charArray& ratesSymbols[][], double& ratesInformation[][][])
{
  charArray symbol[1];
  charArray currency[1];
  charArray brokerName[1];
  charArray refBrokerName[1];  
  int    brokerTime[1];
  int    ordersCount[TOTAL_ORDER_TYPES];
  double orderInfo[][TOTAL_ORDER_INFO_INDEXES];
  double accountInfo[ACCOUNT_INFO_ARRAY_SIZE];
  double bidAsk[BID_ASK_ARRAY_SIZE];
  double results[][RESULTS_ARRAY_SIZE];
  double settings[STRATEGY_SETTINGS_ARRAY_SIZE];
  double ratesInfo[TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE];
  MqlRates rates_0[];
  MqlRates rates_1[];
  MqlRates rates_2[];
  MqlRates rates_3[];
  MqlRates rates_4[];
  MqlRates rates_5[];
  MqlRates rates_6[];
  MqlRates rates_7[];
  MqlRates rates_8[];
  MqlRates rates_9[];
  int i ;
  
  //sync orderwrapper first
  syncPositioning();
  checkForSLorTP();
  
  // Copy the rates array pointers
  c_copyRatesArrays(strategyIndex, ratesSymbols, ratesInformation, rates_0, rates_1, rates_2, rates_3, rates_4, rates_5, rates_6, rates_7, rates_8, rates_9);
  
  // Set the orderInfo array to the required size
    ArrayResize(orderInfo, strategySettings[strategyIndex][IDX_ORDERINFO_ARRAY_SIZE]);
    ArrayResize(results, strategySettings[strategyIndex][IDX_MAX_OPEN_ORDERS]);

  
  // Strings are copied to separate arrays so they can be passed as parameters to the DLL
  symbol       [0] = strategyStrings[strategyIndex][IDX_TRADE_SYMBOL];
  currency     [0] = strategyStrings[strategyIndex][IDX_ACCOUNT_CURRENCY];
  brokerName   [0] = strategyStrings[strategyIndex][IDX_BROKER_NAME];
  refBrokerName[0] = strategyStrings[strategyIndex][IDX_REFERENCE_BROKER_NAME];

  // Strategy settings are copied to a single dimensional array to be passed to the DLL
  for(i = 0; i < STRATEGY_SETTINGS_ARRAY_SIZE; i++)
  {
    settings[i] = strategySettings[strategyIndex][i];
  }
  
  // Rates information is copied to a two dimensional array to be passed to the DLL
  for(i = 0; i < TOTAL_RATES_ARRAYS; i++)
  {
    for(int j = 0; j < RATES_INFO_ARRAY_SIZE; j++)
    {
      ratesInfo[i][j] = ratesInformation[strategyIndex][i][j];
    }
  }
  
  // Update the other parameters that will be passed to the AsirikuyFramework DLL
  c_updateStrategyParameters(bidAsk, orderInfo, brokerTime, ordersCount, strategyIndex, accountInfo, systemSettings, strategySettings, strategyStrings, profitDrawdown, ratesSymbols, ratesInformation);
  
  // Call the AsirikuyFramework DLL
  int returnCode = mql5_runStrategy(
    settings, symbol, currency, brokerName, refBrokerName, brokerTime, ordersCount, orderInfo, accountInfo, bidAsk, 
    ratesInfo, rates_0, rates_1, rates_2, rates_3, rates_4, rates_5, rates_6, rates_7, rates_8, rates_9, results);
    
  c_processReturnCode(returnCode, strategyIndex, systemSettings, strategySettings);
  
  // Open, Modify, and Close orders
  c_processOrders(strategyIndex, systemSettings, strategySettings, strategyStrings, results);
  
  // Update the user interface
  ui_setStrategyInfo(CharArrayToString(symbol[0].a), strategyIndex, systemSettings, strategySettings, profitDrawdown, results);
}

void c_processReturnCode(int returnCode, int strategyIndex, double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE])
{
  if(returnCode != SUCCESS)
  {
    if(strategySettings[strategyIndex][IDX_IS_BACKTESTING] != 0)
    {
      Print(c_getErrorDescription(returnCode));
    }
        
    ui_setErrorInfo(c_getErrorDescription(returnCode), systemSettings[strategyIndex][IDX_UI_X_COORDINATE], systemSettings[strategyIndex][IDX_UI_Y_COORDINATE], strategySettings[strategyIndex][IDX_OPERATIONAL_MODE]);
  }
}

string c_getErrorDescription(int errorCode)
{
  switch(errorCode)
  {
    // Errors returned from the AsirikuyFramework DLL    
    case SUCCESS              : return("Success");
    case INVALID_CURRENCY     : return("Invalid Currency");
    case UNKNOWN_SYMBOL       : return("Unknown Symbol");
    case SYMBOL_TOO_SHORT     : return("Symbol too short");
    case PARSE_SYMBOL_FAILED  : return("Failed to parse symbol");
    case NO_CONVERSION_SYMBOLS: return("No conversion symbols found");
    case NULL_POINTER         : return("Null pointer");
    case INVALID_STRATEGY     : return("Invalid strategy");
    case NOT_ENOUGH_MARGIN    : return("Not enough margin");
    case SPREAD_TOO_WIDE      : return("Spread too wide");
    case NOT_ENOUGH_RATES_DATA: return("Not enough rates data");
    case WORST_CASE_SCENARIO  : return("Worst case scenario");
    case NORMALIZE_BARS_FAILED: return("Failed to normalize rates");
    case INIT_LOG_FAILED      : return("Failed to initialize log");
    case DEINIT_LOG_FAILED    : return("Failed to close log");
    case ZERO_DIVIDE          : return("Zero divide");
    case TA_LIB_ERROR         : return("TA Lib error");
    case INVALID_PARAMETER    : return("Invalid parameter");
    case NN_TRAINING_FAILED   : return("Neural net training failed");
    case LOCAL_TZ_MISMATCH    : return("Local timezone mismatch");
    case BROKER_TZ_MISMATCH   : return("Broker timezone mismatch");
    case TOO_MANY_INSTANCES   : return("Maximum instances exceeded");
    case INVALID_CONFIG       : return("Framework config is invalid");
    case MISSING_CONFIG       : return("Can\'t find the config file");
    case INIT_XML_FAILED      : return("Failed to init xml parser");
    case UNKNOWN_INSTANCE_ID  : return("Unknown instance Id");
    case INSUFFICIENT_MEMORY  : return("Not enough RAM available");
    case WAIT_FOR_INIT        : return("Not yet initialized");

    // mql4 interface errors
    case ERROR_INSUFFICIENT_BARS    : return("Too few bars on chart.");
    case ERROR_INVALID_INSTANCE_ID  : return("InstanceId must be >= 0.");
    case ERROR_DUPLICATE_INSTANCE_ID: return("Duplicate instance id.");
    case ERROR_NO_CONVERSION_SYMBOL : return("No conversion symbol.");
    case ERROR_INCORRECT_TIMEFRAME  : return("Incorrect timeframe.");
    case ERROR_BAD_HISTORIC_DATA_ID : return("Invalid historic data ID.");
    case ERROR_INVALID_MODE         : return("Invalid operational mode.");
    
    // stdlib errors
    default: return(errorCode);
  }
}

