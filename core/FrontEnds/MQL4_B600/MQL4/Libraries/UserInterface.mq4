/**
 * @file
 * @brief     A generic and scalable user interface for use by all trading strategies.
 * @details   Prints strategy and error information onto the chart. Text size and color can be modified and any number of strategies can be displayed simultaniously.
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

#include "../Include/Defines.mqh"
#include "../Include/Common.mqh"
#include "../include/OrderWrapper.mqh"

#define FONT_NAME                     "Times New Roman"
#define STATIC_FONT_SIZE              12  // Font size for static elements that are not resized.
#define WINDOW_MARGIN                 15  // Area left blank around the edges of the window.
#define TEXT_SPACING_MULTIPLIER       1.5 // Multiple of font size. Controls the distance between lines of text.
#define CUSTOM_INFO_SIZE_MULTIPLIER   1.0 // Alter the size of the text for customized strategy information.
#define TOTAL_CUSTOM_INFO_VALUES      10  // 10 labels 10 values.
#define COLUMN_TWO_X_OFFSET           11  // Multiple of font size. Controls the distance between labels and values.
#define GRID_X_OFFSET                 24  // Multiple of font size. Controls the horizontal distance between strategies.
#define GRID_Y_OFFSET                 36  // Multiple of font size. Controls the virtical distance between strategies.

#define TOP_LEFT_CORNER               0
#define TOP_RIGHT_CORNER              1
#define BOTTOM_LEFT_CORNER            2
#define BOTTOM_RIGHT_CORNER           3

#define OBJECT_LABEL_COPYRIGHT        "labelCopyright"
#define OBJECT_LABEL_LINK             "labelLink"
#define OBJECT_LABEL_ORDER_WRAPPER    "labelOrderWrapper"

#define OBJECT_VALUE_ORDER_WRAPPER    "valueOrderWrapper"

#define OBJECT_LABEL_LAST_EXEC_TIME   "labelLastExecTime"

#define OBJECT_LABEL_SYSTEM_NAME      "labelSystemName"
#define OBJECT_LABEL_SYSTEM_VERSION   "labelSystemVersion"
#define OBJECT_LABEL_INSTANCE_ID      "labelInstanceId"
#define OBJECT_LABEL_PROFIT_LARGEST   "labelLargestProfit"
#define OBJECT_LABEL_PROFIT_CURRENT   "labelCurrentProfit"
#define OBJECT_LABEL_DRAWDOWN_LARGEST "labelLargestDrawdown"
#define OBJECT_LABEL_DRAWDOWN_CURRENT "labelCurrentDrawdown"
#define OBJECT_LABEL_TRADE_SIZE       "labelTradeSize"
#define OBJECT_LABEL_TAKE_PROFIT      "labelTakeProfit"
#define OBJECT_LABEL_STOP_LOSS        "labelStopLoss"
#define OBJECT_LABEL_ERROR            "labelError"
#define OBJECT_LABEL_ERROR_TIME       "labelErrorTime"

#define OBJECT_VALUE_SYSTEM_NAME      "valueSystemName"
#define OBJECT_VALUE_SYSTEM_VERSION   "valueSystemVersion"
#define OBJECT_VALUE_INSTANCE_ID      "valueInstanceId"
#define OBJECT_VALUE_PROFIT_LARGEST   "valueLargestProfit"
#define OBJECT_VALUE_PROFIT_CURRENT   "valueCurrentProfit"
#define OBJECT_VALUE_DRAWDOWN_LARGEST "valueLargestDrawdown"
#define OBJECT_VALUE_DRAWDOWN_CURRENT "valueCurrentDrawdown"
#define OBJECT_VALUE_TRADE_SIZE       "valueTradeSize"
#define OBJECT_VALUE_TAKE_PROFIT      "valueTakeProfit"
#define OBJECT_VALUE_STOP_LOSS        "valueStopLoss"
#define OBJECT_VALUE_ERROR            "valueError"
#define OBJECT_VALUE_ERROR_TIME       "valueErrorTime"

#define OBJECT_LABEL_EXT_UI1          "labelExtUI1"
#define OBJECT_LABEL_EXT_UI2          "labelExtUI2"
#define OBJECT_LABEL_EXT_UI3          "labelExtUI3"
#define OBJECT_LABEL_EXT_UI4          "labelExtUI4"
#define OBJECT_LABEL_EXT_UI5          "labelExtUI5"
#define OBJECT_LABEL_EXT_UI6          "labelExtUI6"
#define OBJECT_LABEL_EXT_UI7          "labelExtUI7"
#define OBJECT_LABEL_EXT_UI8          "labelExtUI8"
#define OBJECT_LABEL_EXT_UI9          "labelExtUI9"
#define OBJECT_LABEL_EXT_UI10         "labelExtUI10"

#define OBJECT_VALUE_EXT_UI1          "valueExtUI1"
#define OBJECT_VALUE_EXT_UI2          "valueExtUI2"
#define OBJECT_VALUE_EXT_UI3          "valueExtUI3"
#define OBJECT_VALUE_EXT_UI4          "valueExtUI4"
#define OBJECT_VALUE_EXT_UI5          "valueExtUI5"
#define OBJECT_VALUE_EXT_UI6          "valueExtUI6"
#define OBJECT_VALUE_EXT_UI7          "valueExtUI7"
#define OBJECT_VALUE_EXT_UI8          "valueExtUI8"
#define OBJECT_VALUE_EXT_UI9          "valueExtUI9"
#define OBJECT_VALUE_EXT_UI10         "valueExtUI10"

// Global variables
int   g_fontSize                  = 12;
color g_systemInfoColor           = White;
color g_profitInfoColor           = Yellow;
color g_tradeInfoColor            = Orange;
color g_errorInfoColor            = Red;
color g_customInfoColor           = SkyBlue;
color g_orderWrapperEnabledColor  = Green;
color g_orderWrapperDisabledColor = Red;

void ui_init(int     totalInternalStrategies, 
             charArray& strategyStrings[][], 
             double& systemSettings[][], 
             double& strategySettings[][], 
             int     fontSize, 
             color   systemInfoColor, 
             color   profitInfoColor, 
             color   tradeInfoColor, 
             color   errorInfoColor, 
             color   customInfoColor)
{
  if(IsTesting() && !IsVisualMode())
  {
    return;
  }
  
  // Clear any existing user interface objects.
  ui_deinit();
  
  g_fontSize        = fontSize;
  g_systemInfoColor = systemInfoColor;
  g_profitInfoColor = profitInfoColor;
  g_tradeInfoColor  = tradeInfoColor;
  g_errorInfoColor  = errorInfoColor;
  g_customInfoColor = customInfoColor;
  
  ObjectCreate (OBJECT_LABEL_COPYRIGHT, OBJ_LABEL, 0, 0, 0);
  ObjectSet    (OBJECT_LABEL_COPYRIGHT, OBJPROP_CORNER, BOTTOM_RIGHT_CORNER);
  ObjectSet    (OBJECT_LABEL_COPYRIGHT, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSet    (OBJECT_LABEL_COPYRIGHT, OBJPROP_YDISTANCE, WINDOW_MARGIN + (g_fontSize * TEXT_SPACING_MULTIPLIER));
  ObjectSetText(OBJECT_LABEL_COPYRIGHT, "Copyright © 2011, Asirikuy Community", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);
  
  ObjectCreate (OBJECT_LABEL_LINK, OBJ_LABEL, 0, 0, 0);
  ObjectSet    (OBJECT_LABEL_LINK, OBJPROP_CORNER, BOTTOM_RIGHT_CORNER);
  ObjectSet    (OBJECT_LABEL_LINK, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSet    (OBJECT_LABEL_LINK, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetText(OBJECT_LABEL_LINK, "http://www.asirikuy.com", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);
   
  ObjectCreate (OBJECT_LABEL_ORDER_WRAPPER, OBJ_LABEL, 0, 0, 0);
  ObjectSet    (OBJECT_LABEL_ORDER_WRAPPER, OBJPROP_CORNER, BOTTOM_LEFT_CORNER);
  ObjectSet    (OBJECT_LABEL_ORDER_WRAPPER, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSet    (OBJECT_LABEL_ORDER_WRAPPER, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetText(OBJECT_LABEL_ORDER_WRAPPER, "NFA compliant order wrapper:", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);
  
  ObjectCreate (OBJECT_LABEL_LAST_EXEC_TIME, OBJ_LABEL, 0, 0, 0);
  ObjectSet    (OBJECT_LABEL_LAST_EXEC_TIME, OBJPROP_CORNER, TOP_RIGHT_CORNER);
  ObjectSet    (OBJECT_LABEL_LAST_EXEC_TIME, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSet    (OBJECT_LABEL_LAST_EXEC_TIME, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetText(OBJECT_LABEL_LAST_EXEC_TIME, "", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);     

  ObjectCreate (OBJECT_VALUE_ORDER_WRAPPER, OBJ_LABEL, 0, 0, 0);
  ObjectSet    (OBJECT_VALUE_ORDER_WRAPPER, OBJPROP_CORNER, BOTTOM_LEFT_CORNER);
  ObjectSet    (OBJECT_VALUE_ORDER_WRAPPER, OBJPROP_XDISTANCE, WINDOW_MARGIN + 200);
  ObjectSet    (OBJECT_VALUE_ORDER_WRAPPER, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetText(OBJECT_VALUE_ORDER_WRAPPER, "Disabled", STATIC_FONT_SIZE, FONT_NAME, g_orderWrapperDisabledColor);

  for(int i = 0; i < totalInternalStrategies; i++)
  {
    if(strategySettings[i][IDX_OPERATIONAL_MODE] == DISABLE)
    {
      continue;
    }
    
    int    gridX         = systemSettings[i][IDX_UI_X_COORDINATE];
    int    gridY         = systemSettings[i][IDX_UI_Y_COORDINATE];
    string systemName    = CharArrayToString(strategyStrings[i][IDX_STRATEGY_NAME].a);
    string systemVersion = CharArrayToString(strategyStrings[i][IDX_FRAMEWORK_VERSION].a);
    string instanceId    = DoubleToStr(strategySettings[i][IDX_STRATEGY_INSTANCE_ID], 0);
    
    ObjectCreate(StringConcatenate(OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_INSTANCE_ID     , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_TRADE_SIZE      , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_STOP_LOSS       , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_ERROR           , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_ERROR_TIME      , gridX, gridY), OBJ_LABEL, 0, 0, 0);
  
    ObjectCreate(StringConcatenate(OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_INSTANCE_ID     , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_TRADE_SIZE      , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_STOP_LOSS       , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_ERROR           , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_ERROR_TIME      , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI1         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI2         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI3         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI4         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI5         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI6         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI7         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI8         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI9         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_LABEL_EXT_UI10        , gridX, gridY), OBJ_LABEL, 0, 0, 0);
  
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI1         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI2         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI3         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI4         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI5         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI6         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI7         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI8         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI9         , gridX, gridY), OBJ_LABEL, 0, 0, 0);
    ObjectCreate(StringConcatenate(OBJECT_VALUE_EXT_UI10        , gridX, gridY), OBJ_LABEL, 0, 0, 0);

    ObjectSet(StringConcatenate(OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_INSTANCE_ID     , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_TRADE_SIZE      , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_STOP_LOSS       , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_ERROR           , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_ERROR_TIME      , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
  
    ObjectSet(StringConcatenate(OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_INSTANCE_ID     , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_TRADE_SIZE      , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_STOP_LOSS       , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_ERROR           , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_ERROR_TIME      , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI1         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI2         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI3         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI4         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI5         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI6         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI7         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI8         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI9         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI10        , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
  
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI1         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI2         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI3         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI4         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI5         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI6         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI7         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI8         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI9         , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI10        , gridX, gridY), OBJPROP_CORNER, TOP_LEFT_CORNER);

    ObjectSet(StringConcatenate(OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_INSTANCE_ID     , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_TRADE_SIZE      , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_STOP_LOSS       , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_ERROR           , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_ERROR_TIME      , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
  
    ObjectSet(StringConcatenate(OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_INSTANCE_ID     , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_TRADE_SIZE      , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_STOP_LOSS       , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_ERROR           , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_ERROR_TIME      , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI1         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI2         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI3         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI4         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI5         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI6         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI7         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI8         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI9         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI10         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI1         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI2         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI3         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI4         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI5         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI6         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI7         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI8         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI9         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI10         , gridX, gridY), OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
   
    ObjectSet(StringConcatenate(OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 1));
    ObjectSet(StringConcatenate(OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 2));
    ObjectSet(StringConcatenate(OBJECT_LABEL_INSTANCE_ID     , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 3));
    ObjectSet(StringConcatenate(OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 4));
    ObjectSet(StringConcatenate(OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 5));
    ObjectSet(StringConcatenate(OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 6));
    ObjectSet(StringConcatenate(OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 7));
    ObjectSet(StringConcatenate(OBJECT_LABEL_TRADE_SIZE      , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 8));
    ObjectSet(StringConcatenate(OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 9));
    ObjectSet(StringConcatenate(OBJECT_LABEL_STOP_LOSS       , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 10));
    ObjectSet(StringConcatenate(OBJECT_LABEL_ERROR           , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 11));
    ObjectSet(StringConcatenate(OBJECT_LABEL_ERROR_TIME      , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 12));
  
    ObjectSet(StringConcatenate(OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 1));
    ObjectSet(StringConcatenate(OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 2));
    ObjectSet(StringConcatenate(OBJECT_VALUE_INSTANCE_ID     , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 3));
    ObjectSet(StringConcatenate(OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 4));
    ObjectSet(StringConcatenate(OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 5));
    ObjectSet(StringConcatenate(OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 6));
    ObjectSet(StringConcatenate(OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 7));
    ObjectSet(StringConcatenate(OBJECT_VALUE_TRADE_SIZE      , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 8));
    ObjectSet(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 9));
    ObjectSet(StringConcatenate(OBJECT_VALUE_STOP_LOSS       , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 10));
    ObjectSet(StringConcatenate(OBJECT_VALUE_ERROR           , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 11));
    ObjectSet(StringConcatenate(OBJECT_VALUE_ERROR_TIME      , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 12));
  
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI1         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI2         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 1));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI3         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 2));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI4         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 3));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI5         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 4));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI6         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 5));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI7         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 6));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI8         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 7));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI9         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 8));
    ObjectSet(StringConcatenate(OBJECT_LABEL_EXT_UI10        , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 9));
   
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI1         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI2         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 1));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI3         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 2));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI4         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 3));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI5         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 4));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI6         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 5));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI7         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 6));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI8         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 7));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI9         , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 8));
    ObjectSet(StringConcatenate(OBJECT_VALUE_EXT_UI10        , gridX, gridY), OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 9));
 
    ObjectSetText(StringConcatenate(OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY), "System name: "     , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY), "System version: "  , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_INSTANCE_ID     , gridX, gridY), "Instance ID:    "  , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY), "Largest profit: "  , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY), "Current profit: "  , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY), "Largest drawdown: ", g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY), "Current drawdown: ", g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_TRADE_SIZE      , gridX, gridY), "Trade size: "      , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY), "Take profit: "     , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_STOP_LOSS       , gridX, gridY), "Stop loss: "       , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_ERROR           , gridX, gridY), "Last error: "      , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_ERROR_TIME      , gridX, gridY), "Time of error: "   , g_fontSize                              , FONT_NAME, g_errorInfoColor);
  
    ObjectSetText(StringConcatenate(OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY), systemName          , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY), systemVersion       , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_INSTANCE_ID     , gridX, gridY), instanceId          , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY), "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY), "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY), "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY), "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_TRADE_SIZE      , gridX, gridY), ""                  , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY), ""                  , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_STOP_LOSS       , gridX, gridY), ""                  , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_ERROR           , gridX, gridY), ""                  , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_ERROR_TIME      , gridX, gridY), ""                  , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI1         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI2         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI3         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI4         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI5         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI6         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI7         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI8         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI9         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI10        , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI1         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI2         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI3         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI4         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI5         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI6         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI7         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI8         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI9         , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI10        , gridX, gridY), ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor); 
  
  }
}

void ui_deinit()
{
  ObjectsDeleteAll();
}

void ui_setErrorInfo(string error, int gridX, int gridY, int operationalMode)
{
  if(IsTesting() && !IsVisualMode())
  {
    return(0);
  }
  
  ObjectSetText(StringConcatenate(OBJECT_VALUE_ERROR, gridX, gridY), error, g_fontSize, FONT_NAME, g_errorInfoColor);
  
  if(error == "")
  {
    ObjectSetText(StringConcatenate(OBJECT_VALUE_ERROR_TIME, gridX, gridY), "", g_fontSize, FONT_NAME, g_errorInfoColor);
  }
  else
  {
    ObjectSetText(StringConcatenate(OBJECT_VALUE_ERROR_TIME, gridX, gridY), TimeToStr(TimeCurrent()), g_fontSize, FONT_NAME, g_errorInfoColor);
  }
  
  WindowRedraw();
}

void ui_setStrategyInfo(string symbol, int strategyIndex, double& systemSettings[][], double& strategySettings[][], double& profitDrawdown[][], double& strategyResults[])
{
  if(IsTesting() && !IsVisualMode())
  {
    return(0);
  }

  int gridX = systemSettings[strategyIndex][IDX_UI_X_COORDINATE];
  int gridY = systemSettings[strategyIndex][IDX_UI_Y_COORDINATE];
  
  string text = StringConcatenate(NormalizeDouble(profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_PERCENT], 1), "% (", TimeToStr(profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_TIME], TIME_DATE), ")");
  ObjectSetText(StringConcatenate(OBJECT_VALUE_PROFIT_LARGEST, gridX, gridY), text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  text = StringConcatenate(NormalizeDouble(profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT], 1), "%");
  ObjectSetText(StringConcatenate(OBJECT_VALUE_PROFIT_CURRENT, gridX, gridY), text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  text = StringConcatenate(NormalizeDouble(profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_PERCENT], 1), "% (", TimeToStr(profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_TIME], TIME_DATE), ")");
  ObjectSetText(StringConcatenate(OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY), text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  text = StringConcatenate(NormalizeDouble(profitDrawdown[strategyIndex][IDX_CURRENT_DRAWDOWN_PERCENT], 1), "%");
  ObjectSetText(StringConcatenate(OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY), text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  if(GlobalVariableGet("useOrderWrapper") == true)
  {
    ObjectSetText(OBJECT_VALUE_ORDER_WRAPPER, "Enabled", STATIC_FONT_SIZE, FONT_NAME, g_orderWrapperEnabledColor);
  }
  else
  {
    ObjectSetText(OBJECT_VALUE_ORDER_WRAPPER, "Disabled", STATIC_FONT_SIZE, FONT_NAME, g_orderWrapperDisabledColor);
  }
  
  if(OrdersTotal() <= 0)
  {
    ObjectSetText(StringConcatenate(OBJECT_VALUE_TRADE_SIZE , gridX, gridY), "", g_fontSize, FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT, gridX, gridY), "", g_fontSize, FONT_NAME, g_tradeInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_STOP_LOSS  , gridX, gridY), "", g_fontSize, FONT_NAME, g_tradeInfoColor);
  }
  
  for(int i = OrdersTotal() - 1; i >= 0; i--)
  {
    if(!OrderSelect(i, SELECT_BY_POS, MODE_TRADES))
    {
      continue;
    }
      
    if(OrderMagicNumber() != strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID])
    {
      continue;
    }
    
    text = StringConcatenate(DoubleToStr(OrderLots(), 2), " lots");
    ObjectSetText(StringConcatenate(OBJECT_VALUE_TRADE_SIZE, gridX, gridY) , text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    
    if(strategyResults[IDX_INTERNAL_SL] > EPSILON)
    {
      text = StringConcatenate(DoubleToStr(strategyResults[IDX_INTERNAL_SL] / c_pipValue(systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      ObjectSetText(StringConcatenate(OBJECT_VALUE_STOP_LOSS, gridX, gridY), text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    else if(OrderStopLoss() > EPSILON)
    {
      text = StringConcatenate(DoubleToStr(c_stopLossPriceToPips(OrderType(), OrderOpenPrice(), OrderStopLoss(), systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      ObjectSetText(StringConcatenate(OBJECT_VALUE_STOP_LOSS, gridX, gridY), text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    
    if(strategyResults[IDX_INTERNAL_TP] > EPSILON)
    {
      text = StringConcatenate(DoubleToStr(strategyResults[IDX_INTERNAL_TP] / c_pipValue(systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      ObjectSetText(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT, gridX, gridY), text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    else if(OrderTakeProfit() > EPSILON)
    {
      text = StringConcatenate(DoubleToStr(c_takeProfitPriceToPips(OrderType(), OrderOpenPrice(), OrderTakeProfit(), systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      ObjectSetText(StringConcatenate(OBJECT_VALUE_TAKE_PROFIT, gridX, gridY), text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    
    break;
  }
  
  int Handle = FileOpen(StringConcatenate(strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID], ".ui"), FILE_CSV|FILE_READ, ","); // File opening
  
  string extUILabels[TOTAL_CUSTOM_INFO_VALUES];
  string extUIValues[TOTAL_CUSTOM_INFO_VALUES];
  int j = 0;
  
  if(Handle > 0)
  {
    while(FileIsEnding(Handle) == false)  // While the file pointer..
    {
      extUILabels[j] = FileReadString(Handle);
      extUIValues[j] = FileReadString(Handle);
      
      if(StringLen(extUILabels[j]) < 2)
      {
        extUIValues[j] = "";
      }
      
      j++;
    }
    
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI1 , gridX, gridY), extUILabels[0], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI2 , gridX, gridY), extUILabels[1], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI3 , gridX, gridY), extUILabels[2], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI4 , gridX, gridY), extUILabels[3], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI5 , gridX, gridY), extUILabels[4], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI6 , gridX, gridY), extUILabels[5], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI7 , gridX, gridY), extUILabels[6], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI8 , gridX, gridY), extUILabels[7], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI9 , gridX, gridY), extUILabels[8], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_LABEL_EXT_UI10, gridX, gridY), extUILabels[9], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI1 , gridX, gridY), extUIValues[0], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI2 , gridX, gridY), extUIValues[1], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI3 , gridX, gridY), extUIValues[2], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI4 , gridX, gridY), extUIValues[3], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI5 , gridX, gridY), extUIValues[4], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI6 , gridX, gridY), extUIValues[5], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI7 , gridX, gridY), extUIValues[6], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI8 , gridX, gridY), extUIValues[7], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI9 , gridX, gridY), extUIValues[8], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    ObjectSetText(StringConcatenate(OBJECT_VALUE_EXT_UI10, gridX, gridY), extUIValues[9], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    FileClose(Handle);
  }
  
    int HandleHeartBeat = FileOpen(StringConcatenate(strategySettings[0][IDX_STRATEGY_INSTANCE_ID], "_heartBeat.hb"), FILE_CSV|FILE_READ, ","); // File opening
  
    string lastExecutionHour;
    string lastExecutionTime;
  
    if(HandleHeartBeat > 0)
    {
      lastExecutionHour = FileReadString(HandleHeartBeat);
      lastExecutionTime = FileReadString(HandleHeartBeat);
      ObjectSetText(OBJECT_LABEL_LAST_EXEC_TIME, StringConcatenate("Last DLL Execution Time : ", lastExecutionTime), STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);     
      FileClose(HandleHeartBeat);
    }

  WindowRedraw();
}

