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

// translation function between MQL4 and MQL5 ObjectSet functions
bool ObjectSetMQL4(string name,
                   int index,
                   double value) 
  {
   switch(index)
     {
      case OBJPROP_COLOR:
         ObjectSetInteger(0,name,OBJPROP_COLOR,(int)value);return(true);
      case OBJPROP_STYLE:
         ObjectSetInteger(0,name,OBJPROP_STYLE,(int)value);return(true);
      case OBJPROP_WIDTH:
         ObjectSetInteger(0,name,OBJPROP_WIDTH,(int)value);return(true);
      case OBJPROP_BACK:
         ObjectSetInteger(0,name,OBJPROP_BACK,(int)value);return(true);
      case OBJPROP_RAY:
         ObjectSetInteger(0,name,OBJPROP_RAY_RIGHT,(int)value);return(true);
      case OBJPROP_ELLIPSE:
         ObjectSetInteger(0,name,OBJPROP_ELLIPSE,(int)value);return(true);
      case OBJPROP_SCALE:
         ObjectSetDouble(0,name,OBJPROP_SCALE,value);return(true);
      case OBJPROP_ANGLE:
         ObjectSetDouble(0,name,OBJPROP_ANGLE,value);return(true);
      case OBJPROP_ARROWCODE:
         ObjectSetInteger(0,name,OBJPROP_ARROWCODE,(int)value);return(true);
      case OBJPROP_TIMEFRAMES:
         ObjectSetInteger(0,name,OBJPROP_TIMEFRAMES,(int)value);return(true);
      case OBJPROP_DEVIATION:
         ObjectSetDouble(0,name,OBJPROP_DEVIATION,value);return(true);
      case OBJPROP_FONTSIZE:
         ObjectSetInteger(0,name,OBJPROP_FONTSIZE,(int)value);return(true);
      case OBJPROP_CORNER:
         ObjectSetInteger(0,name,OBJPROP_CORNER,(int)value);return(true);
      case OBJPROP_XDISTANCE:
         ObjectSetInteger(0,name,OBJPROP_XDISTANCE,(int)value);return(true);
      case OBJPROP_YDISTANCE:
         ObjectSetInteger(0,name,OBJPROP_YDISTANCE,(int)value);return(true);
      case OBJPROP_LEVELCOLOR:
         ObjectSetInteger(0,name,OBJPROP_LEVELCOLOR,(int)value);return(true);
      case OBJPROP_LEVELSTYLE:
         ObjectSetInteger(0,name,OBJPROP_LEVELSTYLE,(int)value);return(true);
      case OBJPROP_LEVELWIDTH:
         ObjectSetInteger(0,name,OBJPROP_LEVELWIDTH,(int)value);return(true);

      default: return(false);
     }
   return(false);
  }
  
  bool ObjectSetTextMQL4(string name,
                       string text,
                       int font_size,
                       string font="",
                       color text_color=CLR_NONE)
  {
   int tmpObjType=(int)ObjectGetInteger(0,name,OBJPROP_TYPE);
   if(tmpObjType!=OBJ_LABEL && tmpObjType!=OBJ_TEXT) return(false);
   if(StringLen(text)>0 && font_size>0)
     {
      if(ObjectSetString(0,name,OBJPROP_TEXT,text)==true
         && ObjectSetInteger(0,name,OBJPROP_FONTSIZE,font_size)==true)
        {
         if((StringLen(font)>0)
            && ObjectSetString(0,name,OBJPROP_FONT,font)==false)
            return(false);
         if(text_color>-1
            && ObjectSetInteger(0,name,OBJPROP_COLOR,text_color)==false)
            return(false);
         return(true);
        }
      return(false);
     }
   return(false);
  }
  
  bool ObjectCreateMQL4(string name,
                      ENUM_OBJECT type,
                      int window,
                      datetime time1,
                      double price1,
                      datetime time2=0,
                      double price2=0,
                      datetime time3=0,
                      double price3=0)
  {
  
   bool result = ObjectCreate(0,name,type,window, time1,price1,time2,price2,time3,price3);
          
          return(result);
  }

void ui_init(int     totalInternalStrategies, 
             charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
             double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
             double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
             int     fontSize, 
             color   systemInfoColor, 
             color   profitInfoColor, 
             color   tradeInfoColor, 
             color   errorInfoColor, 
             color   customInfoColor) export
{
  if(MQL5InfoInteger(MQL5_TESTING) && !MQL5InfoInteger(MQL5_VISUAL_MODE))
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
  
  ObjectCreateMQL4 (OBJECT_LABEL_COPYRIGHT, OBJ_LABEL, 0, 0, 0);
  ObjectSetMQL4    (OBJECT_LABEL_COPYRIGHT, OBJPROP_CORNER, BOTTOM_RIGHT_CORNER);
  ObjectSetMQL4    (OBJECT_LABEL_COPYRIGHT, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSetMQL4    (OBJECT_LABEL_COPYRIGHT, OBJPROP_YDISTANCE, WINDOW_MARGIN + (g_fontSize * TEXT_SPACING_MULTIPLIER));
  ObjectSetTextMQL4(OBJECT_LABEL_COPYRIGHT, "Copyright © 2011, Asirikuy Community", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);
  
  ObjectCreateMQL4 (OBJECT_LABEL_LINK, OBJ_LABEL, 0, 0, 0);
  ObjectSetMQL4    (OBJECT_LABEL_LINK, OBJPROP_CORNER, BOTTOM_RIGHT_CORNER);
  ObjectSetMQL4    (OBJECT_LABEL_LINK, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSetMQL4    (OBJECT_LABEL_LINK, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetTextMQL4(OBJECT_LABEL_LINK, "http://www.asirikuy.com", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);
   
  ObjectCreateMQL4 (OBJECT_LABEL_ORDER_WRAPPER, OBJ_LABEL, 0, 0, 0);
  ObjectSetMQL4    (OBJECT_LABEL_ORDER_WRAPPER, OBJPROP_CORNER, BOTTOM_LEFT_CORNER);
  ObjectSetMQL4    (OBJECT_LABEL_ORDER_WRAPPER, OBJPROP_XDISTANCE, WINDOW_MARGIN);
  ObjectSetMQL4    (OBJECT_LABEL_ORDER_WRAPPER, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetTextMQL4(OBJECT_LABEL_ORDER_WRAPPER, "NFA compliant order wrapper:", STATIC_FONT_SIZE, FONT_NAME, g_systemInfoColor);
  
  ObjectCreateMQL4 (OBJECT_VALUE_ORDER_WRAPPER, OBJ_LABEL, 0, 0, 0);
  ObjectSetMQL4    (OBJECT_VALUE_ORDER_WRAPPER, OBJPROP_CORNER, BOTTOM_LEFT_CORNER);
  ObjectSetMQL4    (OBJECT_VALUE_ORDER_WRAPPER, OBJPROP_XDISTANCE, WINDOW_MARGIN + 200);
  ObjectSetMQL4    (OBJECT_VALUE_ORDER_WRAPPER, OBJPROP_YDISTANCE, WINDOW_MARGIN);
  ObjectSetTextMQL4(OBJECT_VALUE_ORDER_WRAPPER, "Disabled", STATIC_FONT_SIZE, FONT_NAME, g_orderWrapperDisabledColor);

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
    string instanceId    = DoubleToString(strategySettings[i][IDX_STRATEGY_INSTANCE_ID], 0);
    string label = "";
    
    label = ""; StringConcatenate(label, OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label, OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label, OBJECT_LABEL_INSTANCE_ID     , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TRADE_SIZE      , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_STOP_LOSS       , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR           , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR_TIME      , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY); 
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_INSTANCE_ID     , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE      , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS       , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR           , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME      , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI1         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI2         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI3         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI4         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI5         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI6         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI7         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI8         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI9         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI10        , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI1         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI2         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI3         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI4         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI5         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI6         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI7         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI8         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI9         , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI10        , gridX, gridY);
    ObjectCreateMQL4(label, OBJ_LABEL, 0, 0, 0);

    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_INSTANCE_ID     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TRADE_SIZE      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_STOP_LOSS       , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR           , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR_TIME      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_INSTANCE_ID     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS       , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR           , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI1         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI2         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI3         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI4         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI5         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI6         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI7         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI8         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI9         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI10        , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI1         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI2         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI3         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI4         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI5         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI6         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI7         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI8         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI9         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI10        , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_CORNER, TOP_LEFT_CORNER);

    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_INSTANCE_ID     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TRADE_SIZE      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_STOP_LOSS       , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR           , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR_TIME      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_INSTANCE_ID     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS       , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR           , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI1         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI2         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI3         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI4         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI5         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI6         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI7         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI8         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI9         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI10         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI1         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI2         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI3         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI4         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI5         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI6         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI7         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI8         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI9         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI10         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_XDISTANCE, WINDOW_MARGIN + (gridX * g_fontSize * GRID_X_OFFSET) + (g_fontSize * COLUMN_TWO_X_OFFSET));
   
    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 1));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 2));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_INSTANCE_ID     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 3));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 4));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 5));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 6));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 7));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TRADE_SIZE      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 8));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 9));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_STOP_LOSS       , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 10));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR           , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 11));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR_TIME      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 12));
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 1));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 2));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_INSTANCE_ID     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 3));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 4));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 5));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 6));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 7));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 8));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 9));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS       , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 10));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR           , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 11));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME      , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 12));
  
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI1         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI2         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 1));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI3         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 2));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI4         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 3));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI5         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 4));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI6         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 5));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI7         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 6));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI8         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 7));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI9         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 8));
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI10        , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 9));
   
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI1         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI2         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 1));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI3         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 2));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI4         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 3));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI5         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 4));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI6         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 5));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI7         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 6));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI8         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 7));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI9         , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 8));
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI10        , gridX, gridY);
    ObjectSetMQL4(label, OBJPROP_YDISTANCE, WINDOW_MARGIN + (gridY * g_fontSize * GRID_Y_OFFSET) + (g_fontSize * TEXT_SPACING_MULTIPLIER * 13) + (g_fontSize * TEXT_SPACING_MULTIPLIER * CUSTOM_INFO_SIZE_MULTIPLIER * 9));
 
    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_NAME     , gridX, gridY);
    ObjectSetTextMQL4(label, "System name: "     , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetTextMQL4(label, "System version: "  , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_INSTANCE_ID     , gridX, gridY);
    ObjectSetTextMQL4(label, "Instance ID:    "  , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetTextMQL4(label, "Largest profit: "  , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetTextMQL4(label, "Current profit: "  , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetTextMQL4(label, "Largest drawdown: ", g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetTextMQL4(label, "Current drawdown: ", g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TRADE_SIZE      , gridX, gridY);
    ObjectSetTextMQL4(label, "Trade size: "      , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_TAKE_PROFIT     , gridX, gridY);
    ObjectSetTextMQL4(label, "Take profit: "     , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_STOP_LOSS       , gridX, gridY);
    ObjectSetTextMQL4(label, "Stop loss: "       , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR           , gridX, gridY);
    ObjectSetTextMQL4(label, "Last error: "      , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_ERROR_TIME      , gridX, gridY);
    ObjectSetTextMQL4(label, "Time of error: "   , g_fontSize                              , FONT_NAME, g_errorInfoColor);
  
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_NAME     , gridX, gridY);
    ObjectSetTextMQL4(label, systemName          , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_SYSTEM_VERSION  , gridX, gridY);
    ObjectSetTextMQL4(label, systemVersion       , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_INSTANCE_ID     , gridX, gridY);
    ObjectSetTextMQL4(label, instanceId          , g_fontSize                              , FONT_NAME, g_systemInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_LARGEST  , gridX, gridY);
    ObjectSetTextMQL4(label, "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_CURRENT  , gridX, gridY);
    ObjectSetTextMQL4(label, "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY);
    ObjectSetTextMQL4(label, "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY);
    ObjectSetTextMQL4(label, "0%"                , g_fontSize                              , FONT_NAME, g_profitInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE      , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT     , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize                              , FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS       , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR           , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME      , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize                              , FONT_NAME, g_errorInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI1         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI2         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI3         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI4         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI5         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI6         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI7         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI8         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI9         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI10        , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI1         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI2         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI3         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI4         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI5         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI6         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI7         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI8         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI9         , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI10        , gridX, gridY);
    ObjectSetTextMQL4(label, ""                  , g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
  }
}

int ObjectsDeleteAllMQL4(int window=-1,
                         int type=-1)
  {
   return(ObjectsDeleteAll(0,window,type));
  }

void ui_deinit() export
{
  ObjectsDeleteAllMQL4();
}

void ui_setErrorInfo(string error, int gridX, int gridY, int operationalMode) export
{
  if(MQL5InfoInteger(MQL5_TESTING) && !MQL5InfoInteger(MQL5_VISUAL_MODE))
  {
    return;
  }
  
  string label;
  
  label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR, gridX, gridY);
  ObjectSetTextMQL4(label, error, g_fontSize, FONT_NAME, g_errorInfoColor);
  
  if(error == "")
  {
    label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME, gridX, gridY);
    ObjectSetTextMQL4(label, "", g_fontSize, FONT_NAME, g_errorInfoColor);
  }
  else
  {
   label = ""; StringConcatenate(label,OBJECT_VALUE_ERROR_TIME, gridX, gridY);
    ObjectSetTextMQL4(label, TimeToString(TimeCurrent()), g_fontSize, FONT_NAME, g_errorInfoColor);
  }
  
  ChartRedraw(0);
}

void ui_setStrategyInfo(string symbol, int strategyIndex, double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], double& profitDrawdown[][PROFIT_DRAWDOWN_ARRAY_SIZE], double& strategyResults[][RESULTS_ARRAY_SIZE]) export
{
  if(MQL5InfoInteger(MQL5_TESTING) && !MQL5InfoInteger(MQL5_VISUAL_MODE))
  {
    return;
  }

  int gridX = systemSettings[strategyIndex][IDX_UI_X_COORDINATE];
  int gridY = systemSettings[strategyIndex][IDX_UI_Y_COORDINATE];
  
  string text;
  string label;
  
  text = ""; StringConcatenate(text,NormalizeDouble(profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_PERCENT], 1), "% (", TimeToString(profitDrawdown[strategyIndex][IDX_LARGEST_PROFIT_TIME], TIME_DATE), ")");
  label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_LARGEST, gridX, gridY);
  ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  text = ""; StringConcatenate(text,NormalizeDouble(profitDrawdown[strategyIndex][IDX_CURRENT_PROFIT_PERCENT], 1), "%");
  label = ""; StringConcatenate(label,OBJECT_VALUE_PROFIT_CURRENT, gridX, gridY);
  ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  text = ""; StringConcatenate(text,NormalizeDouble(profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_PERCENT], 1), "% (", TimeToString(profitDrawdown[strategyIndex][IDX_LARGEST_DRAWDOWN_TIME], TIME_DATE), ")");
  label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_LARGEST, gridX, gridY);
  ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  text = ""; StringConcatenate(label,NormalizeDouble(profitDrawdown[strategyIndex][IDX_CURRENT_DRAWDOWN_PERCENT], 1), "%");
  label = ""; StringConcatenate(label,OBJECT_VALUE_DRAWDOWN_CURRENT, gridX, gridY);
  ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_profitInfoColor);
  
  
  if(nOrdersTotal() <= 0)
  {
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE , gridX, gridY);
    ObjectSetTextMQL4(label, "", g_fontSize, FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT, gridX, gridY);
    ObjectSetTextMQL4(label, "", g_fontSize, FONT_NAME, g_tradeInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS  , gridX, gridY);
    ObjectSetTextMQL4(label, "", g_fontSize, FONT_NAME, g_tradeInfoColor);
  }
  
  for(int i = nOrdersTotal() - 1; i >= 0; i--)
  {
    if(!nOrderSelect(i, SELECT_BY_POS, MODE_TRADES))
    {
      continue;
    }
      
    if(nOrderMagicNumber() != strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID])
    {
      continue;
    }
    
    text = ""; StringConcatenate(text,DoubleToString(nOrderLots(), 2), " lots");
    label = ""; StringConcatenate(label,OBJECT_VALUE_TRADE_SIZE, gridX, gridY);
    ObjectSetTextMQL4(label , text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    
    if(strategyResults[0][IDX_INTERNAL_SL] > EPSILON)
    {
      text =  ""; StringConcatenate(text,DoubleToString(strategyResults[0][IDX_INTERNAL_SL] / c_pipValue(systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS, gridX, gridY);
      ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    else if(nOrderStopLoss() > EPSILON)
    {
      text = ""; StringConcatenate(text,DoubleToString(c_stopLossPriceToPips(nOrderType(), nOrderOpenPrice(), nOrderStopLoss(), systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      label = ""; StringConcatenate(label,OBJECT_VALUE_STOP_LOSS, gridX, gridY);
      ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    
    if(strategyResults[0][IDX_INTERNAL_TP] > EPSILON)
    {
      text = ""; StringConcatenate(text,DoubleToString(strategyResults[0][IDX_INTERNAL_TP] / c_pipValue(systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT, gridX, gridY);
      ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    else if(nOrderTakeProfit() > EPSILON)
    {
      text =  ""; StringConcatenate(label,DoubleToString(c_takeProfitPriceToPips(nOrderType(), nOrderOpenPrice(), nOrderTakeProfit(), systemSettings[strategyIndex][IDX_DIGITS]), 1), " pips");
      label = ""; StringConcatenate(label,OBJECT_VALUE_TAKE_PROFIT, gridX, gridY);
      ObjectSetTextMQL4(label, text, g_fontSize, FONT_NAME, g_tradeInfoColor);
    }
    
    break;
  }
  
  label = ""; StringConcatenate(label,strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID], ".ui");
  int Handle = FileOpen(label, FILE_CSV|FILE_READ, ","); // File opening
  
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
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI1 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[0], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI2 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[1], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI3 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[2], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI4 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[3], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI5 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[4], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI6 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[5], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI7 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[6], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI8 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[7], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI9 , gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[8], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_LABEL_EXT_UI10, gridX, gridY);
    ObjectSetTextMQL4(label, extUILabels[9], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI1 , gridX, gridY); 
    ObjectSetTextMQL4(label , extUIValues[0], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI2 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[1], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI3 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[2], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI4 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[3], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI5 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[4], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI6 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[5], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI7 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[6], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI8 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[7], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI9 , gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[8], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    label = ""; StringConcatenate(label,OBJECT_VALUE_EXT_UI10, gridX, gridY);
    ObjectSetTextMQL4(label, extUIValues[9], g_fontSize * CUSTOM_INFO_SIZE_MULTIPLIER, FONT_NAME, g_customInfoColor);
    
    FileClose(Handle);
  }

  ChartRedraw(0);
}

