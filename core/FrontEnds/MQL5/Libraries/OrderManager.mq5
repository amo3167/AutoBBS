/**
 * @file
 * @brief     Handles the opening, modification, and closing of orders.
 * @details   OrderManager places orders with the broker through the MetaTrader4 platform using the supplied API.
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

#include "../include/Defines.mqh"
#include "../include/OrderWrapper.mqh"

#define LONG_OPEN_COLOR    Blue
#define LONG_CLOSE_COLOR   DarkBlue
#define SHORT_OPEN_COLOR   Red
#define SHORT_CLOSE_COLOR  Maroon

#define ENABLE_ORDER_MUTEX 1      // 1 = use, 0 = don't use. Using this mutex avoids 'ERR_TRADE_CONTEXT_BUSY'. Systems will wait until the trade context is available.
#define SLEEP_TIME         10     // Milliseconds. Time between each check of the mutex status.


bool WindowScreenShot(string filename,
                          int size_x,
                          int size_y,
                          int start_bar=-1,
                          int chart_scale=-1,
                          int chart_mode=-1) export
  {
   if(chart_scale>0 && chart_scale<=5)
      ChartSetInteger(0,CHART_SCALE,chart_scale);
   switch(chart_mode)
     {
      case 0: ChartSetInteger(0,CHART_MODE,CHART_BARS);
      case 1: ChartSetInteger(0,CHART_MODE,CHART_CANDLES);
      case 2: ChartSetInteger(0,CHART_MODE,CHART_LINE);
     }
   if(start_bar<0)
      return(ChartScreenShot(0,filename,size_x,size_y,ALIGN_RIGHT));
   else
      return(ChartScreenShot(0,filename,size_x,size_y,ALIGN_LEFT));
  }

double MarketInfo(string symbol,
                      int type) export
  {
  
  MqlTick last_tick;
  SymbolInfoTick(_Symbol,last_tick);
  double Bid=last_tick.bid;
  double Ask=last_tick.ask;
  
   switch(type)
     {
      case MODE_LOW:
         return(SymbolInfoDouble(symbol,SYMBOL_LASTLOW));
      case MODE_HIGH:
         return(SymbolInfoDouble(symbol,SYMBOL_LASTHIGH));
      case MODE_TIME:
         return(SymbolInfoInteger(symbol,SYMBOL_TIME));
      case MODE_BID:
         return(Bid);
      case MODE_ASK:
         return(Ask);
      case MODE_POINT:
         return(SymbolInfoDouble(symbol,SYMBOL_POINT));
      case MODE_DIGITS:
         return(SymbolInfoInteger(symbol,SYMBOL_DIGITS));
      case MODE_SPREAD:
         return(SymbolInfoInteger(symbol,SYMBOL_SPREAD));
      case MODE_STOPLEVEL:
         return(SymbolInfoInteger(symbol,SYMBOL_TRADE_STOPS_LEVEL));
      case MODE_LOTSIZE:
         return(SymbolInfoDouble(symbol,SYMBOL_TRADE_CONTRACT_SIZE));
      case MODE_TICKVALUE:
         return(SymbolInfoDouble(symbol,SYMBOL_TRADE_TICK_VALUE));
      case MODE_TICKSIZE:
         return(SymbolInfoDouble(symbol,SYMBOL_TRADE_TICK_SIZE));
      case MODE_SWAPLONG:
         return(SymbolInfoDouble(symbol,SYMBOL_SWAP_LONG));
      case MODE_SWAPSHORT:
         return(SymbolInfoDouble(symbol,SYMBOL_SWAP_SHORT));
      case MODE_STARTING:
         return(0);
      case MODE_EXPIRATION:
         return(0);
      case MODE_TRADEALLOWED:
         return(0);
      case MODE_MINLOT:
         return(SymbolInfoDouble(symbol,SYMBOL_VOLUME_MIN));
      case MODE_LOTSTEP:
         return(SymbolInfoDouble(symbol,SYMBOL_VOLUME_STEP));
      case MODE_MAXLOT:
         return(SymbolInfoDouble(symbol,SYMBOL_VOLUME_MAX));
      case MODE_SWAPTYPE:
         return(SymbolInfoInteger(symbol,SYMBOL_SWAP_MODE));
      case MODE_PROFITCALCMODE:
         return(SymbolInfoInteger(symbol,SYMBOL_TRADE_CALC_MODE));
      case MODE_MARGINCALCMODE:
         return(0);
      case MODE_MARGININIT:
         return(0);
      case MODE_MARGINMAINTENANCE:
         return(0);
      case MODE_MARGINHEDGED:
         return(0);
      case MODE_MARGINREQUIRED:
         return(0);
      case MODE_FREEZELEVEL:
         return(SymbolInfoInteger(symbol,SYMBOL_TRADE_FREEZE_LEVEL));

      default: return(0);
     }
   return(0);
  }

int om_openOrder(int     orderType, 
                 int     strategyIndex, 
                 int     resultsIndex, 
                 double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
                 double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
                 charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
                 double& strategyResults[][RESULTS_ARRAY_SIZE]) export
{ 
  string comment = CharArrayToString(strategyStrings[strategyIndex][IDX_CUSTOM_ORDER_COMMENT].a);
  if(comment == "")
  {
    StringConcatenate(comment, CharArrayToString(strategyStrings[strategyIndex][IDX_STRATEGY_NAME].a), " Version ", CharArrayToString(strategyStrings[strategyIndex][IDX_STRATEGY_NAME].a));
  }
  
  string   symbol             = CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a);
  int      strategyInstanceId = strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID];
  int      digits             = systemSettings[strategyIndex][IDX_DIGITS];
  int      slippage           = systemSettings[strategyIndex][IDX_MAX_SLIPPAGE];
  bool     enableScreenShots  = systemSettings[strategyIndex][IDX_ENABLE_SCREENSHOTS];
  double   lots               = strategyResults[resultsIndex][IDX_LOTS];
  double   entryPrice         = strategyResults[resultsIndex][IDX_ENTRY_PRICE];
  double   stopLoss           = strategyResults[resultsIndex][IDX_BROKER_SL];
  double   takeProfit         = strategyResults[resultsIndex][IDX_BROKER_TP];
  datetime expiration         = strategyResults[resultsIndex][IDX_EXPIRATION_TIME];
  int      result             = SUCCESS;
  
  if(om_lockOrderMutex(strategyInstanceId, strategySettings[strategyIndex][IDX_STRATEGY_TIMEFRAME]))
  {
    double orderSize       = om_normalizedLots(symbol, lots);
    double price           = NormalizeDouble(entryPrice, digits);
    double stopLossPrice   = 0;
    double takeProfitPrice = 0;
    color  orderColor      = LONG_OPEN_COLOR;
    
    if((orderType == OP_SELL) || (orderType == OP_SELLLIMIT) || (orderType == OP_SELLSTOP))
    {
      orderColor = SHORT_OPEN_COLOR;
    }
    
    if(orderType == OP_BUY)
    {
      price = MarketInfo(symbol, MODE_ASK);
    }
    else if(orderType == OP_SELL)
    {
      price = MarketInfo(symbol, MODE_BID);
    }

    if(orderType == OP_BUYLIMIT || orderType == OP_BUYSTOP)
    {
      stopLossPrice   = NormalizeDouble(entryPrice - stopLoss, digits);
      takeProfitPrice = NormalizeDouble(entryPrice + takeProfit , digits);
    }
    else if(orderType == OP_SELLLIMIT || orderType == OP_SELLSTOP)
    {
      stopLossPrice   = NormalizeDouble(entryPrice + stopLoss, digits);
      takeProfitPrice = NormalizeDouble(entryPrice - takeProfit, digits);
    }
    
  
    int tradeTicket = nOrderSend(symbol, orderType, orderSize, price, slippage, stopLossPrice, takeProfitPrice, comment, strategyInstanceId, expiration, orderColor);
    om_unlockOrderMutex();
    
    if(tradeTicket == -1)
    {
      result = GetLastError();
    }
    else
    {
      if(enableScreenShots && (tradeTicket >= 0))
      {
      
      string filename = "";
      StringConcatenate(filename, AccountInfoInteger(ACCOUNT_LOGIN), "_", strategyInstanceId, "_", tradeTicket, ".gif");
      
        if(!WindowScreenShot(filename,1024,768))
        {
          result = GetLastError();
        }
      }
      
      strategyResults[resultsIndex][IDX_TRADING_SIGNALS] = 0; // Prevent the possibility of the new order being immediately closed from the old signal
      result = om_handleOpenOrders(strategyIndex, systemSettings, strategySettings, strategyStrings, strategyResults);
    }
  }
  
  return(result);
}

int om_handleOpenOrders(int     strategyIndex, 
                        double& systemSettings[][SYSTEM_SETTINGS_ARRAY_SIZE], 
                        double& strategySettings[][STRATEGY_SETTINGS_ARRAY_SIZE], 
                        charArray& strategyStrings[][STRATEGY_STRINGS_ARRAY_SIZE], 
                        double& strategyResults[][RESULTS_ARRAY_SIZE]) export
{
  string   symbol             = CharArrayToString(strategyStrings[strategyIndex][IDX_TRADE_SYMBOL].a);
  int      strategyInstanceId = strategySettings[strategyIndex][IDX_STRATEGY_INSTANCE_ID];
  int      digits             = systemSettings[strategyIndex][IDX_DIGITS];
  int      slippage           = systemSettings[strategyIndex][IDX_MAX_SLIPPAGE];
  int      error              = SUCCESS;
  int      total              = nOrdersTotal();
  int      resultsIndex       = 0;

  for(int i = total - 1; i >= 0; i--)
  {
    if(!nOrderSelect(i, SELECT_BY_POS, MODE_TRADES))
    {
      continue;
    }
    if((nOrderSymbol() != symbol) || (nOrderMagicNumber() != strategyInstanceId))
    {
      continue;
    }
    
    for(int j = strategySettings[strategyIndex][IDX_MAX_OPEN_ORDERS]-1; j >= 0; j--)
    {
      if((strategyResults[j][IDX_TICKET_NUMBER] == nOrderTicket()) || (strategyResults[j][IDX_TICKET_NUMBER] == ALL_ORDER_TICKETS))
      {
        resultsIndex = j;
      }
    }

    if(om_lockOrderMutex(strategyInstanceId, strategySettings[strategyIndex][IDX_STRATEGY_TIMEFRAME]))
    {
      int result = SUCCESS, orderType = nOrderType();
      
      if( (orderType == OP_BUY)
       || (orderType == OP_BUYLIMIT)
       || (orderType == OP_BUYSTOP))
      {
        result = om_handleBuyOrder(symbol, digits, slippage, resultsIndex, strategyResults);
      }
      else if((orderType == OP_SELL)
       ||     (orderType == OP_SELLLIMIT)
       ||     (orderType == OP_SELLSTOP))
      {
        result = om_handleSellOrder(symbol, digits, slippage, resultsIndex, strategyResults);
      }
      
      if(result != SUCCESS)
      {
        error = result;
      }
        
      om_unlockOrderMutex();
    }
  }
  
  return(error);
}

int om_handleBuyOrder(string symbol, int digits, int slippage, int resultsIndex, double& strategyResults[][RESULTS_ARRAY_SIZE]) export
{
  double   minimumStop        = MarketInfo(symbol, MODE_STOPLEVEL) * MarketInfo(symbol, MODE_POINT);
  double   bid                = MarketInfo(symbol, MODE_BID);
  double   ask                = MarketInfo(symbol, MODE_ASK);
  double   spread             = ask - bid;
  int      orderTicket        = nOrderTicket();
  int      orderType          = nOrderType();
  double   orderLots          = nOrderLots();
  double   orderOpenPrice     = nOrderOpenPrice();
  double   orderClosePrice    = nOrderClosePrice();
  double   orderStopLoss      = nOrderStopLoss();
  double   orderTakeProfit    = nOrderTakeProfit();
  datetime orderExpiration    = nOrderExpiration();
  double   newStopLossPrice   = orderStopLoss;
  double   newTakeProfitPrice = orderTakeProfit;
  int      newExpiration      = orderExpiration;
  bool     succeeded          = true;
  int      tradingSignals     = strategyResults[resultsIndex][IDX_TRADING_SIGNALS];
  int      brokerSL           = strategyResults[resultsIndex][IDX_BROKER_SL];
  int      brokerTP           = strategyResults[resultsIndex][IDX_BROKER_TP];
  int      expiration         = strategyResults[resultsIndex][IDX_EXPIRATION_TIME];
  int      entryPrice         = strategyResults[resultsIndex][IDX_ENTRY_PRICE];
  int      useTrailing        = strategyResults[resultsIndex][IDX_USE_TRAILING];

  if (brokerSL == 0 && brokerTP == 0 && (tradingSignals & SIGNAL_CLOSE_BUY == 0))
  return(SUCCESS);
  
  if(brokerSL > 0)
  {
    if(brokerSL < (minimumStop + spread))
    {
      brokerSL = minimumStop + spread;
    }
    newStopLossPrice = NormalizeDouble(ask - brokerSL, digits);
  }
  
  if(brokerTP > 0)
  {
    if(brokerTP < (minimumStop + spread))
    {
      brokerTP = minimumStop + spread;
    }
    newTakeProfitPrice = NormalizeDouble(ask + brokerTP, digits);
  }
  
  if(expiration > 0)
  {
    newExpiration = expiration;
  }
  
  if( ((orderType == OP_BUYLIMIT) && ((tradingSignals & SIGNAL_CLOSE_BUYLIMIT) != 0))
   || ((orderType == OP_BUYSTOP)  && ((tradingSignals & SIGNAL_CLOSE_BUYSTOP)  != 0)))
  {
    // Close signal
   // succeeded = OrderDelete(orderTicket, LONG_CLOSE_COLOR);
  }
  else if((orderType == OP_BUY)
   && (tradingSignals & SIGNAL_CLOSE_BUY != 0))
  {
    // Close signal
    succeeded = nOrderClose(orderTicket, orderLots, orderClosePrice, slippage, LONG_CLOSE_COLOR);
  }
  else if(((orderType == OP_BUYLIMIT) && ((tradingSignals & SIGNAL_UPDATE_BUYLIMIT) != 0))
   ||     ((orderType == OP_BUYSTOP)  && ((tradingSignals & SIGNAL_UPDATE_BUYSTOP)  != 0))
   ||     ((orderType == OP_BUY)      && ((tradingSignals & SIGNAL_UPDATE_BUY)      != 0)))
  {
    double newOrderOpenPrice = entryPrice;
    
    if(  (MathAbs(orderStopLoss   - newStopLossPrice)   > EPSILON)
      || (MathAbs(orderTakeProfit - newTakeProfitPrice) > EPSILON)
      || (MathAbs(orderExpiration - newExpiration)      > EPSILON))
    {
      // Act on update signal
      succeeded = nOrderModify(orderTicket, newOrderOpenPrice, newStopLossPrice, newTakeProfitPrice, newExpiration, LONG_OPEN_COLOR);
    }
  }
  else if((orderStopLoss   == 0) 
    &&    (orderTakeProfit == 0) 
    &&    (orderExpiration == 0))
  {
    // Set initial SL, TP, and expiration
    succeeded = nOrderModify(orderTicket, orderOpenPrice, newStopLossPrice, newTakeProfitPrice, newExpiration, LONG_OPEN_COLOR);
  }
  else if((useTrailing > 0)
    &&   ((MathAbs(orderStopLoss   - newStopLossPrice)   > EPSILON)
    ||    (MathAbs(orderTakeProfit - newTakeProfitPrice) > EPSILON)
    ||    (MathAbs(orderExpiration - newExpiration)      > EPSILON)))
  {
    // Update trailing stop, take profit, or expiration time.
    succeeded = nOrderModify(orderTicket, orderOpenPrice, newStopLossPrice, newTakeProfitPrice, newExpiration, LONG_OPEN_COLOR);
  }
  
  if(!succeeded)
  {
    return(GetLastError());
  }
    
  return(SUCCESS);
}

int om_handleSellOrder(string symbol, int digits, int slippage, int resultsIndex, double& strategyResults[][RESULTS_ARRAY_SIZE]) export
{
  double   minimumStop        = MarketInfo(symbol, MODE_STOPLEVEL) * MarketInfo(symbol, MODE_POINT);
  double   bid                = MarketInfo(symbol, MODE_BID);
  double   ask                = MarketInfo(symbol, MODE_ASK);
  double   spread             = ask - bid;
  int      orderTicket        = nOrderTicket();
  int      orderType          = nOrderType();
  double   orderLots          = nOrderLots();
  double   orderOpenPrice     = nOrderOpenPrice();
  double   orderClosePrice    = nOrderClosePrice();
  double   orderStopLoss      = nOrderStopLoss();
  double   orderTakeProfit    = nOrderTakeProfit();
  datetime orderExpiration    = nOrderExpiration();
  double   newStopLossPrice   = orderStopLoss;
  double   newTakeProfitPrice = orderTakeProfit;
  double   newExpiration      = orderExpiration;
  bool     succeeded          = true;
  int      tradingSignals     = strategyResults[resultsIndex][IDX_TRADING_SIGNALS];
  int      brokerSL           = strategyResults[resultsIndex][IDX_BROKER_SL];
  int      brokerTP           = strategyResults[resultsIndex][IDX_BROKER_TP];
  int      expiration         = strategyResults[resultsIndex][IDX_EXPIRATION_TIME];
  int      entryPrice         = strategyResults[resultsIndex][IDX_ENTRY_PRICE];
  int      useTrailing        = strategyResults[resultsIndex][IDX_USE_TRAILING];

  if (brokerSL == 0 && brokerTP == 0 && (tradingSignals & SIGNAL_CLOSE_SELL == 0))
  return(SUCCESS);
  
  if(brokerSL > 0)
  {
    if(brokerSL < (minimumStop + spread))
    {
      brokerSL = minimumStop + spread;
    }
    newStopLossPrice = NormalizeDouble(bid + brokerSL, digits);
  }
  
  if(brokerTP > 0)
  {
    if(brokerTP < (minimumStop + spread))
    {
      brokerTP = minimumStop + spread;
    }
    newTakeProfitPrice = NormalizeDouble(bid - brokerTP, digits);
  }
  
  if(expiration > 0)
  {
    newExpiration = expiration;
  }
  
  if( ((orderType == OP_SELLLIMIT) && ((tradingSignals & SIGNAL_CLOSE_SELLLIMIT) != 0))
   || ((orderType == OP_SELLSTOP)  && ((tradingSignals & SIGNAL_CLOSE_SELLSTOP)  != 0)))
  {
    // Close signal
  //  succeeded = nOrderDelete(orderTicket, SHORT_CLOSE_COLOR);
  }
  else if((orderType == OP_SELL)
   && (tradingSignals & SIGNAL_CLOSE_SELL != 0))
  { 
    // Close signal
    succeeded = nOrderClose(orderTicket, orderLots, orderClosePrice, slippage, SHORT_CLOSE_COLOR);
  }
  else if(((orderType == OP_SELLLIMIT) && ((tradingSignals & SIGNAL_UPDATE_SELLLIMIT) != 0))
   ||     ((orderType == OP_SELLSTOP)  && ((tradingSignals & SIGNAL_UPDATE_SELLSTOP)  != 0))
   ||     ((orderType == OP_SELL)      && ((tradingSignals & SIGNAL_UPDATE_SELL)      != 0)))
  {
    double newOrderOpenPrice = entryPrice;
    
    if(  (MathAbs(orderStopLoss   - newStopLossPrice)   > EPSILON)
      || (MathAbs(orderTakeProfit - newTakeProfitPrice) > EPSILON)
      || (MathAbs(orderExpiration - newExpiration)      > EPSILON))
    {
      // Act on update signal
      succeeded = nOrderModify(orderTicket, newOrderOpenPrice, newStopLossPrice, newTakeProfitPrice, newExpiration, SHORT_OPEN_COLOR);
    }
  }
  else if((orderStopLoss   == 0) 
    &&    (orderTakeProfit == 0) 
    &&    (orderExpiration == 0))
  {
    // Set initial SL, TP, and expiration
    succeeded = nOrderModify(orderTicket, orderOpenPrice, newStopLossPrice, newTakeProfitPrice, newExpiration, SHORT_OPEN_COLOR);
  }
  else if((useTrailing > 0)
    &&   ((MathAbs(orderStopLoss   - newStopLossPrice)   > EPSILON)
    ||    (MathAbs(orderTakeProfit - newTakeProfitPrice) > EPSILON)
    ||    (MathAbs(orderExpiration - newExpiration)      > EPSILON)))
  {
    // Update trailing stop, take profit, or expiration time.
    succeeded = nOrderModify(orderTicket, orderOpenPrice, newStopLossPrice, newTakeProfitPrice, newExpiration, SHORT_OPEN_COLOR);
  }
  
  if(!succeeded)
  {
    return(GetLastError());
  }
  
  return(SUCCESS);
}

double om_normalizedLots(string symbol, double lots) export
{
  static string prevSymbol = "";
  static double minLots    = -1;
  static double maxLots    = -1;
  static double lotStep    = -1;
  
  if(symbol != prevSymbol)
  {
    prevSymbol = symbol;
    minLots    = MarketInfo(symbol, MODE_MINLOT);
    maxLots    = MarketInfo(symbol, MODE_MAXLOT);
    lotStep    = MarketInfo(symbol, MODE_LOTSTEP);
  }
  
   // Use MathFloor so risk is not increased
   lots = MathFloor(lots / lotStep) * lotStep;

   // Don't increase lots to minLots because it increases risk
   if(lots < minLots)
   {
     return(0); 
   }
   
   if(lots > maxLots)
   {
     return(maxLots);
   }
   
   return(lots);
}

bool om_createOrderMutex(int systemTimeframe) export
{
  int tickCount = GetTickCount();
  
  while(!GlobalVariableCheck("OrderMutex"))
  {
    if(GetLastError() == 0)
    {
      GlobalVariableSet("OrderMutex", 0.0);
    }
    else
    {
      Sleep(SLEEP_TIME);
    }  
    
    if(IsStopped()) 
    {
      return(false);
    }  
    
    if((GetTickCount() - tickCount) > (systemTimeframe * MILLISECONDS_PER_MINUTE))
    {
      return(false);
    }
  }
  
  return(true);
}

bool om_lockOrderMutex(double strategyInstanceId, int systemTimeframe) export
{
  if(MQL5InfoInteger(MQL5_TESTING) || !ENABLE_ORDER_MUTEX)
  {
    return(true);
  }
    
  if(!om_createOrderMutex(systemTimeframe))
  {
    return(false);
  }
    
  int tickCount = GetTickCount();
  
  while(!GlobalVariableSetOnCondition("OrderMutex", strategyInstanceId, 0.0))
  {
    Sleep(SLEEP_TIME);
    
    if(IsStopped())
    {
      return(false);
    }
      
    if((GetTickCount() - tickCount) > (systemTimeframe * MILLISECONDS_PER_MINUTE))
    {
      Alert("Error: Mutex Timeout. Currently held by: ", DoubleToString(GlobalVariableGet("OrderMutex"), 0));
      return(false);
    }
  }
  
  return(true);
}

void om_unlockOrderMutex() export
{
  if(MQL5InfoInteger(MQL5_TESTING) || !ENABLE_ORDER_MUTEX)
  {
    return;
  }
  
  while(!GlobalVariableSet("OrderMutex", 0.0))
  {
    Sleep(SLEEP_TIME);
    
    if(IsStopped())
    {
      Alert("Error: Bad shutdown. Please delete the global variable: \'OrderMutex\'.");
      break;
    }
  }
}

