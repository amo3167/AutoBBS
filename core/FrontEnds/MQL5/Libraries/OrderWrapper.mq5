/**
 * @file
 * @brief     A virtual order manager (NFA compliant) used for trading on NFA compliant brokers.
 * @details   OrderWrapper, wraps around normal Order management functions to enable virtual order pooling and net positioning execution.
 * 
 * @author    Daniel Fernandez (Assisted with design and code styling)
 * @author    Maxim Feinshtein (Assisted with design ideas)
 * @author    Friedhelm        (Assisted with design ideas)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to instsall and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
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

#include <Trade\Trade.mqh>
#include <Defines.mqh>
#include <OrderManager.mqh>

#define ARROW_COLOR_BUY         Blue
#define ARROW_COLOR_SELL        Green
#define MASTER_INSTANCE_ID    123
#define SLIPPAGE              4
#define NO_SYNC               -1
#define SUCCESS               1
#define ERROR_OPENING_FILE    -1
#define MUTEX_SYNC_TIMEOUT    20
#define MUTEX_SLTP_TIMEOUT    30

int selectedPosition;
ulong historyTicket;
int selectedTicket;
int selectedType;
int selectionMode;
string selectedSymbol;
double selectedStopLoss;
double selectedTakeProfit;
int selectedInstanceID;
int selectedEntryTime;
int selectedCloseTime;
double selectedClosePrice;
double selectedOpenPrice;
double selectedVolume;
double selectedSwap;
double selectedProfit;
datetime selectedExpiration;

ENUM_ORDER_TYPE convertOrderType( int type)
{

   switch(type)
     {
      case OP_BUY : return(ORDER_TYPE_BUY);
      case OP_SELL: return(ORDER_TYPE_SELL);
      default: return(-1);
     }

}

bool initOrderWrapper(bool interfaceUsage) export
{

GlobalVariableSet("useOrderWrapper", 0);

if (interfaceUsage)
   {
   Print("Initializing order wrapper...");
   GlobalVariableSet("syncMutex", TimeCurrent()-MUTEX_SYNC_TIMEOUT-5);
   GlobalVariableSet("checkMutex", TimeCurrent()-MUTEX_SYNC_TIMEOUT-5);
   GlobalVariableSet("useOrderWrapper", 1);
   }

return(SUCCESS);

}

int nOrderTicket() export
{ 

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetInteger(POSITION_IDENTIFIER));

   if (selectionMode == MODE_HISTORY)
   return((HistoryDealGetInteger(historyTicket, DEAL_POSITION_ID)));
}

return(selectedTicket);
}

string nOrderSymbol() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetString(POSITION_SYMBOL));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetString(historyTicket, DEAL_SYMBOL));
}

return(selectedSymbol);
}

datetime nOrderExpiration() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   return(0);
}

return(selectedExpiration);
}

int nOrderType() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   {
   ENUM_POSITION_TYPE mql5Type = PositionGetInteger(POSITION_TYPE);
   
   if (mql5Type == POSITION_TYPE_BUY)
   return(OP_BUY);
   
   if (mql5Type == POSITION_TYPE_SELL)
   return(OP_SELL);
   }

   if (selectionMode == MODE_HISTORY)
   {
   ENUM_POSITION_TYPE mql5Type = HistoryDealGetInteger(historyTicket, DEAL_TYPE);
   
   if (mql5Type == POSITION_TYPE_BUY)
   return(OP_BUY);
   
   if (mql5Type == POSITION_TYPE_SELL)
   return(OP_SELL);
   }

}

   return(selectedType);
}

double nOrderLots() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_VOLUME));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetDouble(historyTicket, DEAL_VOLUME));
}

   return(selectedVolume);
}

double nOrderProfit() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_PROFIT));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetDouble(historyTicket, DEAL_PROFIT));
}

   return(selectedProfit);
}

double nOrderSwap() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_SWAP));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetDouble(historyTicket, DEAL_SWAP));
}

   return(selectedSwap);
}

double nOrderCommission() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_COMMISSION));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetDouble(historyTicket, DEAL_COMMISSION));
}

   return(0);
}

int nOrderMagicNumber() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetInteger(POSITION_MAGIC));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetInteger(historyTicket, DEAL_MAGIC));
}

   return(selectedInstanceID);
}

double nOrderStopLoss() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_SL));

   if (selectionMode == MODE_HISTORY)
   return(0);
}

   return(selectedStopLoss);
}

double nOrderTakeProfit() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_TP));

   if (selectionMode == MODE_HISTORY)
   return(0);
}
   return(selectedTakeProfit);
}

double nOrderOpenPrice() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_PRICE_OPEN));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetDouble(historyTicket, DEAL_PRICE));
}

   return(selectedOpenPrice);
}

double nOrderClosePrice() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetDouble(POSITION_PRICE_CURRENT));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetDouble(historyTicket, DEAL_PRICE));
}

   return(selectedClosePrice);
}

int nOrderCloseTime() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(0);

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetInteger(historyTicket, DEAL_TIME));
}

   return(selectedCloseTime);
}

int nOrderOpenTime() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
{
   if (selectionMode == MODE_TRADES)
   return(PositionGetInteger(POSITION_TIME));

   if (selectionMode == MODE_HISTORY)
   return(HistoryDealGetInteger(historyTicket, DEAL_TIME));
}

   return(selectedEntryTime);
}


//
bool nOrderSelect(int position, int selectionType, int selectionModeIn) export
{

   int ticket[], instanceID[], type[], entryTime[], closeTime[];
   double closePrice[], entryPrice[], volume[];
   int ticketModHistory[], instanceIDModHistory[], typeModHistory[], entryTimeModHistory[], closeTimeModHistory[];
   double closePriceModHistory[], entryPriceModHistory[], volumeModHistory[];
   string symbol[], symbolModHistory[];
   double stopLoss[], takeProfit[];
   int i, handle, handle2;
   
 if (GlobalVariableGet("useOrderWrapper") != 1)
 {
 selectionMode = selectionModeIn;
 
   if (selectionMode == MODE_TRADES)
   return(PositionSelect(Symbol()));

   if (selectionMode == MODE_HISTORY)
   {  
      bool selection = HistorySelectByPosition(i);
      historyTicket = HistoryDealGetTicket(i);
      return(selection );
   }
 }

 if (selectionModeIn == MODE_TRADES)
 {
  i = 0;

   handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
   if(handle > 0)
       {
         while(FileIsEnding(handle)==false || i+1 < position )  // While the file pointer..
         {
     
         ArrayResize(symbol, i+1);
         ArrayResize(entryTime, i+1);
         ArrayResize(ticket, i+1);
         ArrayResize(instanceID, i+1);
         ArrayResize(stopLoss, i+1);
         ArrayResize(takeProfit, i+1);
         ArrayResize(entryPrice, i+1);
         ArrayResize(volume, i+1);
         ArrayResize(type, i+1);
     
         ticket[i] = StringToInteger(FileReadString(handle));
         entryTime[i] = StringToInteger(FileReadString(handle));
         symbol[i] = FileReadString(handle);
         type[i]   = StringToInteger(FileReadString(handle));
         volume[i] = StringToDouble(FileReadString(handle));
         entryPrice[i] = StringToDouble(FileReadString(handle));
         stopLoss[i] = StringToDouble(FileReadString(handle));
         takeProfit[i] = StringToDouble(FileReadString(handle));
         instanceID[i] = StringToInteger(FileReadString(handle));
         
         //Print( " Order  ", i, " ticket ", ticket[i], " Open time ", entryTime[i], " symbol ", symbol[i], " type ", type[i], " volume ", volume[i]) ;
     
         i++;
     
         }
         
         selectedSymbol     = symbol[position];
         selectedPosition   = position;
         selectedTicket     = ticket[position];
         selectedType       = type[position] ;
         selectedEntryTime  = entryTime[position] ;
         selectedStopLoss   = stopLoss[position];
         selectedTakeProfit = takeProfit[position];
         selectedInstanceID = instanceID[position] ;  
         selectedVolume     = volume[position];  
         selectedClosePrice = 0;
         selectedCloseTime  = 0;   
         selectedProfit     = 0;
         selectedSwap       = 0;
         
         FileClose(handle);
         
         return(true);
      }
    
         else
   
         return(false);
   }  
   
   
if (selectionModeIn == MODE_HISTORY)
 {
  i = 0;

   handle2=FileOpen("virtualOrderHistory.csv",FILE_CSV|FILE_READ,';');
  
   if(handle2 > 0)
       {
       
         while(FileIsEnding(handle2)==false || i+1 < position )  // While the file pointer..
         {
     
         ArrayResize(symbolModHistory, i+1);
         ArrayResize(typeModHistory, i+1);
         ArrayResize(entryTimeModHistory, i+1);
         ArrayResize(ticketModHistory, i+1);
         ArrayResize(instanceIDModHistory, i+1);
         ArrayResize(closeTimeModHistory, i+1);
         ArrayResize(closePriceModHistory, i+1);
         ArrayResize(entryPriceModHistory, i+1);
         ArrayResize(volumeModHistory, i+1);
         
         ticketModHistory[i]     = StringToInteger(FileReadString(handle2));
         entryTimeModHistory[i]  = StringToInteger(FileReadString(handle2));
         closeTimeModHistory[i]  = StringToInteger(FileReadString(handle2));
         entryPriceModHistory[i] = StringToDouble(FileReadString(handle2));
         closePriceModHistory[i] = StringToDouble(FileReadString(handle2));
         symbolModHistory[i]     = FileReadString(handle2);
         typeModHistory[i]       = StringToInteger(FileReadString(handle2));
         instanceIDModHistory[i] = StringToInteger(FileReadString(handle2));
         volumeModHistory[i]     = StringToDouble(FileReadString(handle2));
    //     Print("9 -- ", volumeModHistory[i]);
         
         i++;
     
         }
         
         selectedSymbol     = symbolModHistory[position];
         selectedPosition   = position;
         selectedTicket     = ticketModHistory[position];
         selectedType       = typeModHistory[position] ;
         selectedEntryTime  = entryTimeModHistory[position] ;
         selectedStopLoss   = 0;
         selectedTakeProfit = 0;
         selectedInstanceID = instanceIDModHistory[position] ; 
         selectedVolume     = volumeModHistory[position];
         selectedClosePrice = closePriceModHistory[position];
         selectedCloseTime  = closeTimeModHistory[position];  
         selectedProfit     = 0;
         selectedSwap       = 0;
         
         FileClose(handle2);
         
         return(true);
      }
    
         else
   
         return(false);
   }  
   
 return(false);  

}

//wraps OrdersTotal()
int nOrdersTotal() export
{

 if (GlobalVariableGet("useOrderWrapper") != 1)
 return(PositionsTotal());

int i = 0;

int handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     
     i++;
     
     }
     FileClose(handle);
     return(i);
    }
    
   else
   
   return(ERROR_OPENING_FILE);

}

//wraps OrdersHistoryTotal()
int nOrdersHistoryTotal() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
 return(HistoryDealsTotal());

int i = 0;

int handle=FileOpen("virtualOrderHistory.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     
     i++;
     
     }
     FileClose(handle);
     return(i-1);
    }
    
   else
   
   return(ERROR_OPENING_FILE);

}

//add an order to the trading history
bool savePositionToHistory(int ticket, int entryTime, int closeTime, double entryPrice, double closePrice, string symbol, int type, int instanceID, double volume) export
{

int handle=FileOpen("virtualOrderHistory.csv",FILE_CSV|FILE_READ|FILE_WRITE,';');
  
  if(handle > 0)
    {
     FileSeek(handle, 0, SEEK_END);
     
     if (ticket != 0)
     FileWrite(handle, ticket, entryTime, closeTime, entryPrice, closePrice, symbol, type, instanceID, volume);
     
     FileClose(handle);
     return(true);
    }
    
   else
   
   return(false);

}

//add order to virtual orders pool
int addToVitualOrders(string symbol, int type, double volume, double entryPrice, double stoploss, double takeprofit, int instanceID) export
{

int lastTicketNumber = 0;

if (GlobalVariableCheck("lastTicketNumber"))
lastTicketNumber = GlobalVariableGet("lastTicketNumber");

int newTicket = lastTicketNumber + 1 ;
int handle;

handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ|FILE_WRITE,';');
  
  if(handle > 0)
    {
     FileSeek(handle, 0, SEEK_END);
     
     if (newTicket != 0)
     FileWrite(handle, newTicket, TimeCurrent(), symbol, type, volume, entryPrice, stoploss, takeprofit, instanceID);
     
     FileClose(handle);
     GlobalVariableSet("lastTicketNumber", newTicket);
     return(newTicket);
    }
    
   else
   
   return(ERROR_OPENING_FILE);

}

// this function eliminates transactions for which the
// SL or TP has been reached (effectively acting as a close)
bool checkForSLorTP() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(SUCCESS);

int i = 0;

string symbol[];
int ticket[], instanceID[], type[], entryTime[];
double stopLoss[], takeProfit[], entryPrice[], volume[];
bool isTradeOpen;
int syncMutex = GlobalVariableGet("syncMutex");
int result ;

if (TimeCurrent()-syncMutex < MUTEX_SLTP_TIMEOUT)
return(SUCCESS);

GlobalVariableSet("syncMutex", TimeCurrent());


int handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     ArrayResize(symbol, i+1);
     ArrayResize(entryTime, i+1);
     ArrayResize(ticket, i+1);
     ArrayResize(instanceID, i+1);
     ArrayResize(stopLoss, i+1);
     ArrayResize(takeProfit, i+1);
     ArrayResize(entryPrice, i+1);
     ArrayResize(type, i+1);
     ArrayResize(volume, i+1);
     
     ticket[i] = StringToInteger(FileReadString(handle));
     entryTime[i] = StringToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StringToInteger(FileReadString(handle));
     volume[i] = StringToDouble(FileReadString(handle));
     entryPrice[i] = StringToDouble(FileReadString(handle));
     stopLoss[i] = StringToDouble(FileReadString(handle));
     takeProfit[i] = StringToDouble(FileReadString(handle));
     instanceID[i] = StringToInteger(FileReadString(handle));
     
     i++;
     
     }
     FileClose(handle);
    }
    
   else
   {
   return(ERROR_OPENING_FILE);
   }
   

   
 handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_WRITE,';');
  
  if(handle > 0)
    {
     for( i = 0 ; i < ArraySize(ticket); i++)
     {
     
     isTradeOpen = true;
     
     if (   type[i] == OP_BUY && 
          ( 
            (MarketInfo(symbol[i],MODE_ASK) <= stopLoss[i] && stopLoss[i] != 0) ||
            (MarketInfo(symbol[i],MODE_ASK) >= takeProfit[i]&& takeProfit[i] != 0) 
           )
        )
        {
         savePositionToHistory(ticket[i], entryTime[i], TimeCurrent(), entryPrice[i], MarketInfo(symbol[i],MODE_ASK), symbol[i], type[i], instanceID[i], volume[i]);
         isTradeOpen = false;
        }
        
     if (   type[i] == OP_SELL && 
          ( 
            (MarketInfo(symbol[i],MODE_BID) >= stopLoss[i] && stopLoss[i] != 0)||
            (MarketInfo(symbol[i],MODE_BID) <= takeProfit[i]&& takeProfit[i] != 0) 
           )
        )
        {
         savePositionToHistory(ticket[i], entryTime[i], TimeCurrent(), entryPrice[i], MarketInfo(symbol[i],MODE_BID), symbol[i], type[i], instanceID[i], volume[i]);
         isTradeOpen = false;
        }
       
      if (isTradeOpen && ticket[i] != 0)
      {
      FileSeek(handle, 0, SEEK_END);
      FileWrite(handle, ticket[i], entryTime[i], symbol[i], type[i], volume[i], entryPrice[i], stopLoss[i], takeProfit[i], instanceID[i]);
      }
     
     }
     
     FileClose(handle);
     
    }  else {
   
   return(ERROR_OPENING_FILE);
   }
   
   return(SUCCESS);

}



// OrderSend wrapper, sends a virtual order ticket and calls synchronization after
// a brief delay to allow for pooling
int nOrderSend(string symbol, int type, double volume, double entryPrice, int slippage, double stoploss, double takeprofit, string comment, int instanceID, datetime expiration, color entryColor) export
{

int ticket = -1 ;
MqlTradeRequest request={0};
MqlTradeResult result={0};
MqlTick last_tick;
SymbolInfoTick(_Symbol,last_tick);
double Bid=last_tick.bid;
double Ask=last_tick.ask;

if (GlobalVariableGet("useOrderWrapper") != 1)
{

   if (type == -1)
   return(-1);

   ENUM_ORDER_TYPE newType = convertOrderType(type);
   
   if (newType == ORDER_TYPE_SELL)
   {   
   request.action=TRADE_ACTION_DEAL;         // setting a pending order
   request.magic=instanceID;                  // ORDER_MAGIC
   request.symbol=symbol;                      // symbol
   request.volume=volume;                          // volume in 0.1 lots
   request.sl=stoploss;                                // Stop Loss is not specified
   request.tp=takeprofit;                                // Take Profit is not specified     
   request.type= newType; 
   request.price= Bid;  // open price
   }
   
   if (newType == ORDER_TYPE_BUY)
   {   
   request.action=TRADE_ACTION_DEAL;         // setting a pending order
   request.magic=instanceID;                  // ORDER_MAGIC
   request.symbol=symbol;                      // symbol
   request.volume=volume;                          // volume in 0.1 lots
   request.sl=stoploss;                                // Stop Loss is not specified
   request.tp=takeprofit;                                // Take Profit is not specified     
   request.type= newType; 
   request.price= Ask;  // open price
   }
      
   OrderSend(request, result);

return(result.retcode);
}

   ticket = addToVitualOrders(symbol, type, volume, entryPrice, stoploss, takeprofit, instanceID);
   
   return(ticket); 

}

// syncs positioning so that net positioning matches virtual positioning
int syncPositioning() export
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(SUCCESS);


int i = 0, j = 0;

string symbol[];
int ticket[], instanceID[], type[], entryTime[];
double stopLoss[], takeProfit[], entryPrice[], volume[], netVolume, netRealVolume;
bool isTradeOpen;
int syncMutex = GlobalVariableGet("syncMutex");
int result ;

if (TimeCurrent()-syncMutex < MUTEX_SYNC_TIMEOUT)
return(SUCCESS);

GlobalVariableSet("syncMutex", TimeCurrent());

int handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     ArrayResize(symbol, i+1);
     ArrayResize(entryTime, i+1);
     ArrayResize(ticket, i+1);
     ArrayResize(instanceID, i+1);
     ArrayResize(stopLoss, i+1);
     ArrayResize(takeProfit, i+1);
     ArrayResize(entryPrice, i+1);
     ArrayResize(volume, i+1);
     ArrayResize(type, i+1);
     
     ticket[i] = StringToInteger(FileReadString(handle));
     entryTime[i] = StringToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StringToInteger(FileReadString(handle));
     volume[i] = StringToDouble(FileReadString(handle));
     entryPrice[i] = StringToDouble(FileReadString(handle));
     stopLoss[i] = StringToDouble(FileReadString(handle));
     takeProfit[i] = StringToDouble(FileReadString(handle));
     instanceID[i] = StringToInteger(FileReadString(handle));
     
     i++;
     
     }
     FileClose(handle);
    }
    
   else
   
   {
   return(ERROR_OPENING_FILE);
   }

netVolume = 0;  

for( i = 0 ; i < ArraySize(ticket); i++)
     {
     
         if (symbol[i] == Symbol())
         {
         
            if (type[j] == OP_BUY)
            netVolume += volume[j];
         
            if (type[j] == OP_SELL)
            netVolume -= volume[j];
            
         }
     }
     
     netRealVolume = 0;
     MqlTradeRequest request={0};
     MqlTick last_tick;
     MqlTradeResult orderResult={0};
     SymbolInfoTick(_Symbol,last_tick);
     double Bid=last_tick.bid;
     double Ask=last_tick.ask;
     
     PositionSelect(Symbol());
     
     if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY)
     netRealVolume = PositionGetDouble(POSITION_VOLUME);
     
     if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_SELL)
     netRealVolume = -PositionGetDouble(POSITION_VOLUME);
         
     if (netVolume < netRealVolume)
     {
     
      request.action=TRADE_ACTION_DEAL;         // setting a pending order
      request.magic=MASTER_INSTANCE_ID;                  // ORDER_MAGIC
      request.symbol=Symbol();                      // symbol
      request.volume= MathAbs(netVolume-netRealVolume)  ;    // volume in 0.1 lots
      request.sl=0;                                // Stop Loss is not specified
      request.tp=0;                                // Take Profit is not specified     
      request.type= ORDER_TYPE_SELL;                                   // order type   
      request.price= Bid;  // open price
      OrderSend(request, orderResult);
     }
     
     if (netVolume > netRealVolume)
     {
         
      request.action=TRADE_ACTION_DEAL;         // setting a pending order
      request.magic=MASTER_INSTANCE_ID;                  // ORDER_MAGIC
      request.symbol=Symbol();                      // symbol
      request.volume= MathAbs(netVolume-netRealVolume)  ;        // volume in 0.1 lots
      request.sl=0;                                // Stop Loss is not specified
      request.tp=0;                                // Take Profit is not specified     
      request.type= ORDER_TYPE_BUY;                                   // order type   
      request.price= Ask;  // open price
      OrderSend(request, orderResult);
     }
         
     //refreash and check
     PositionSelect(Symbol());
     
     if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY)
     netRealVolume = PositionGetDouble(POSITION_VOLUME);
     
     if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_SELL)
     netRealVolume = -PositionGetDouble(POSITION_VOLUME);    
     
     if (netVolume == netRealVolume)
     {
     return(SUCCESS);
     }
     
return(NO_SYNC);
}

// this function closes an order, it wraps the OrderClose function
bool nOrderClose(int ticketToClose, double volumeToClose, double orderClosePrice, int slippage, color closeColor) export
{

CTrade trade;

if (GlobalVariableGet("useOrderWrapper") != 1)
{
return(trade.PositionClose(Symbol(),3));
}

int i = 0;

string symbol[];
int ticket[], instanceID[], type[], entryTime[];
double stopLoss[], takeProfit[], entryPrice[], volume[];
bool isTradeOpen;


int handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     ArrayResize(symbol, i+1);
     ArrayResize(entryTime, i+1);
     ArrayResize(ticket, i+1);
     ArrayResize(instanceID, i+1);
     ArrayResize(stopLoss, i+1);
     ArrayResize(takeProfit, i+1);
     ArrayResize(entryPrice, i+1);
     ArrayResize(volume, i+1);
     ArrayResize(type, i+1);
     
     ticket[i] = StringToInteger(FileReadString(handle));
     entryTime[i] = StringToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StringToInteger(FileReadString(handle));
     volume[i] = StringToDouble(FileReadString(handle));
     entryPrice[i] = StringToDouble(FileReadString(handle));
     stopLoss[i] = StringToDouble(FileReadString(handle));
     takeProfit[i] = StringToDouble(FileReadString(handle));
     instanceID[i] = StringToInteger(FileReadString(handle));
     
     i++;
     
     }
     FileClose(handle);
    }
    
   else
   
   return(ERROR_OPENING_FILE);
   

   
 int handle2=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_WRITE,';');
  
  if(handle2 > 0)
    {
     for( i = 0 ; i < ArraySize(ticket); i++)
     {
     
     isTradeOpen = true;
     
     if (  ticket[i] == ticketToClose && type[i] == OP_BUY )
        {
         savePositionToHistory(ticket[i], entryTime[i], TimeCurrent(), entryPrice[i], MarketInfo(symbol[i],MODE_BID), symbol[i], type[i], instanceID[i], volume[i]);
         isTradeOpen = false;
        }
        
     if (  ticket[i] == ticketToClose && type[i] == OP_SELL )
        {
         savePositionToHistory(ticket[i], entryTime[i], TimeCurrent(), entryPrice[i], MarketInfo(symbol[i],MODE_ASK), symbol[i], type[i], instanceID[i], volume[i]);
         isTradeOpen = false;
        }
       
      if (isTradeOpen && ticket[i] != 0)
      {
      FileSeek(handle, 0, SEEK_END);
      FileWrite(handle2, ticket[i], entryTime[i], symbol[i], type[i], volume[i], entryPrice[i], stopLoss[i], takeProfit[i], instanceID[i]);
      }
     
     }
     
     FileClose(handle2);
     
    }  else {
   
   return(ERROR_OPENING_FILE);
   }
   
   return(SUCCESS);

}

// this function modifies an order, it wraps the OrderModify function
bool nOrderModify(int ticketToModify, double newEntryPrice, double newStopLoss, double newTakeProfit, datetime newExpiration, color modificationColor) export
{

CTrade trade;

if (GlobalVariableGet("useOrderWrapper") != 1)
return(trade.PositionModify(Symbol(),newStopLoss,newTakeProfit));

int i = 0;

string symbol[];
int ticket[], instanceID[], type[], entryTime[];
double stopLoss[], takeProfit[], entryPrice[], volume[];
bool isTradeOpen;


int handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     ArrayResize(symbol, i+1);
     ArrayResize(entryTime, i+1);
     ArrayResize(ticket, i+1);
     ArrayResize(instanceID, i+1);
     ArrayResize(stopLoss, i+1);
     ArrayResize(takeProfit, i+1);
     ArrayResize(entryPrice, i+1);
     ArrayResize(volume, i+1);
     
     ticket[i] = StringToInteger(FileReadString(handle));
     entryTime[i] = StringToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StringToInteger(FileReadString(handle));
     volume[i] = StringToDouble(FileReadString(handle));
     entryPrice[i] = StringToDouble(FileReadString(handle));
     stopLoss[i] = StringToDouble(FileReadString(handle));
     takeProfit[i] = StringToDouble(FileReadString(handle));
     instanceID[i] = StringToInteger(FileReadString(handle));
     
     i++;
     
     }
     FileClose(handle);
    }
    
   else
   
   return(ERROR_OPENING_FILE);
   

   
 handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_WRITE,';');
  
  if(handle > 0)
    {
     for( i = 0 ; i < ArraySize(ticket); i++)
     {
     
     if (ticketToModify == ticket[i] && ticket[i] != 0)
      {
      FileSeek(handle, 0, SEEK_END);
      FileWrite(handle, ticket[i], entryTime[i], symbol[i], type[i], volume[i], entryPrice[i], newStopLoss, newTakeProfit, instanceID[i]);
      
      } else {
      
      if (ticket[i] != 0)
         {
         FileSeek(handle, 0, SEEK_END);
         FileWrite(handle, ticket[i], entryTime[i], symbol[i], type[i], volume[i], entryPrice[i], stopLoss[i], takeProfit[i], instanceID[i]);
         }
      }
     
     }
     
     FileClose(handle);
     
    }  else {
   
   return(ERROR_OPENING_FILE);
   }
   
   return(SUCCESS);

}