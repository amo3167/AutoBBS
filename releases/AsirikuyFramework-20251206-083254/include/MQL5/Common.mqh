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

#include "Defines.mqh"
#import "Common.ex5"


// Tell MetaTrader4 to generate a tick. This causes the start() funciton to be called.
void   c_generateTick();

// Return true if the settings are valid, otherwise return false.
bool   c_validateSystemSettings(int strategyIndex, 
                                double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
                                double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
                                charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
                                charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
                                double& ratesInfo[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE]);

// Converts an error code into a descriptive string
string c_getErrorDescription(int errorCode);

// Convert pips to points.
int    c_pipsToPoints(int pips, int digits);

// Return the absolute value of 1 pip
double c_pipValue(int digits);

// convert time frames to MQL5 standard
ENUM_TIMEFRAMES TFMigrate(int tf);

// convert MQL5 times back to ints from MT4
int TFMigrateToMQL4(int tf);

// Calculate the difference in pips between the order entry price and the SL price
double c_stopLossPriceToPips(int orderType, double orderEntryPrice, double stopLossPrice, int digits);

// Calculate the difference in pips between the order entry price and the TP price
double c_takeProfitPriceToPips(int orderType, double orderEntryPrice, double takeProfitPrice, int digits);

// Initialize the framework DLL, the settings arrays, and the user interface. Returns TRUE if successful. Otherwise returns FALSE.
bool   c_init(
  int     totalInternalStrategies,
  string  frameworkConfig, 
  charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
  double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
  double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
  charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
  double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE], 
  double& profitDrawdown[][PROFIT_DRAWDOWN_ARRAY_SIZE], 
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
  color   customInfoColor);

// Run a strategy
void   c_runStrategy(
  int     strategyIndex, 
  double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
  double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
  charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
  double& profitDrawdown[][PROFIT_DRAWDOWN_ARRAY_SIZE], 
  charArray& ratesSymbols[][TOTAL_RATES_ARRAYS], 
  double& ratesInformation[][TOTAL_RATES_ARRAYS][RATES_INFO_ARRAY_SIZE]);
  
#import

