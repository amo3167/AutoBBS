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

#define ARROW_COLOR_BUY         Blue
#define ARROW_COLOR_SELL        Green
#define MASTER_INSTANCE_ID      123
#define SLIPPAGE                4
#define SUCCESS                 1
#define ERROR_OPENING_FILE      -1
#define MUTEX_SYNC_TIMEOUT      20
#define MUTEX_SLTP_TIMEOUT      30

int selectedPosition;
int selectedTicket;
int selectedType;
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

bool initOrderWrapper(bool interfaceUsage)
{

GlobalVariableSet("useOrderWrapper", 0);

if (interfaceUsage)
   {
   Print("Initializing order wrapper...");
   GlobalVariableSet("syncMutex", TimeCurrent()-MUTEX_SYNC_TIMEOUT-5);
   GlobalVariableSet("checkMutex", TimeCurrent()-MUTEX_SYNC_TIMEOUT-5);
   GlobalVariableSet("useOrderWrapper", 1);
   }
   
if (IsTesting()) {
int hh = FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_WRITE,';');
if (hh>0) FileClose(hh);
hh = FileOpen("virtualOrderHistory.csv",FILE_CSV|FILE_WRITE,';');
if (hh>0) FileClose(hh);
}

return(SUCCESS);

}

int nOrderTicket()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderTicket());

return(selectedTicket);
}

string nOrderSymbol()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderSymbol());

return(selectedSymbol);
}

datetime nOrderExpiration()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderExpiration());

return(selectedExpiration);
}

int nOrderType()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderType());

   return(selectedType);
}

double nOrderLots()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderLots());

   return(selectedVolume);
}

double nOrderProfit()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderProfit());

   return(selectedProfit);
}

double nOrderSwap()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderSwap());

   return(selectedSwap);
}

double nOrderCommission()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderCommission());

   return(0);
}

int nOrderMagicNumber()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderMagicNumber());

   return(selectedInstanceID);
}

double nOrderStopLoss()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderStopLoss());

   return(selectedStopLoss);
}

double nOrderTakeProfit()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderTakeProfit());

   return(selectedTakeProfit);
}

double nOrderOpenPrice()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderOpenPrice());

   return(selectedOpenPrice);
}

double nOrderClosePrice()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderClosePrice());

   return(selectedClosePrice);
}

int nOrderCloseTime()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderCloseTime());

   return(selectedCloseTime);
}

int nOrderOpenTime()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderOpenTime());

   return(selectedEntryTime);
}


//
bool nOrderSelect(int position, int selectionType, int selectionMode)
{

   int ticket[], instanceID[], type[], entryTime[], closeTime[];
   double closePrice[], entryPrice[], volume[];
   int ticketModHistory[], instanceIDModHistory[], typeModHistory[], entryTimeModHistory[], closeTimeModHistory[];
   double closePriceModHistory[], entryPriceModHistory[], volumeModHistory[];
   string symbol[], symbolModHistory[];
   double stopLoss[], takeProfit[];
   int i, handle, handle2;
   
 if (GlobalVariableGet("useOrderWrapper") != 1)
 return(OrderSelect(position, selectionType, selectionMode));

 if (selectionMode == MODE_TRADES)
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
     
         ticket[i] = StrToInteger(FileReadString(handle));
         entryTime[i] = StrToInteger(FileReadString(handle));
         symbol[i] = FileReadString(handle);
         type[i]   = StrToInteger(FileReadString(handle));
         volume[i] = StrToDouble(FileReadString(handle));
         entryPrice[i] = StrToDouble(FileReadString(handle));
         stopLoss[i] = StrToDouble(FileReadString(handle));
         takeProfit[i] = StrToDouble(FileReadString(handle));
         instanceID[i] = StrToInteger(FileReadString(handle));
         
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
         
         return(TRUE);
      }
    
         else
   
         return(FALSE);
   }  
   
   
if (selectionMode == MODE_HISTORY)
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
         
         ticketModHistory[i]     = StrToInteger(FileReadString(handle2));
         entryTimeModHistory[i]  = StrToInteger(FileReadString(handle2));
         closeTimeModHistory[i]  = StrToInteger(FileReadString(handle2));
         entryPriceModHistory[i] = StrToDouble(FileReadString(handle2));
         closePriceModHistory[i] = StrToDouble(FileReadString(handle2));
         symbolModHistory[i]     = FileReadString(handle2);
         typeModHistory[i]       = StrToInteger(FileReadString(handle2));
         instanceIDModHistory[i] = StrToInteger(FileReadString(handle2));
         volumeModHistory[i]     = StrToDouble(FileReadString(handle2));
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
         
         return(TRUE);
      }
    
         else
   
         return(FALSE);
   }  
   
   

}

//wraps OrdersTotal()
int nOrdersTotal()
{

 if (GlobalVariableGet("useOrderWrapper") != 1)
 return(OrdersTotal());

int i = 0;

int handle=FileOpen("virtualOrderLog.csv",FILE_CSV|FILE_READ,';');
  
  if(handle > 0)
    {
     while(FileIsEnding(handle)==false)  // While the file pointer..
     {
     
     i++;
     
     int number=FileReadString(handle);
     int ticket=FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
     FileReadString(handle);
    
          
     }
     
     FileClose(handle);
     return(i);
    }
    
   else
   
   return(0);

}

//wraps OrdersHistoryTotal()
int nOrdersHistoryTotal()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
 return(OrdersHistoryTotal());

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
     return(i);
    }
    
   else
   
   return(0);

}

//add an order to the trading history
bool savePositionToHistory(int ticket, int entryTime, int closeTime, double entryPrice, double closePrice, string symbol, int type, int instanceID, double volume)
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
int addToVitualOrders(string symbol, int type, double volume, double entryPrice, double stoploss, double takeprofit, int instanceID) 
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
bool checkForSLorTP()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(SUCCESS);

int i = 0;

string symbol[];
int ticket[], instanceID[], type[], entryTime[];
double stopLoss[], takeProfit[], entryPrice[], volume[];
bool isTradeOpen;
int checkMutex = GlobalVariableGet("checkMutex");
int result ;

if (TimeCurrent()-checkMutex < MUTEX_SLTP_TIMEOUT)
return(SUCCESS);

GlobalVariableSet("checkMutex", TimeCurrent());


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
     
     ticket[i] = StrToInteger(FileReadString(handle));
     entryTime[i] = StrToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StrToInteger(FileReadString(handle));
     volume[i] = StrToDouble(FileReadString(handle));
     entryPrice[i] = StrToDouble(FileReadString(handle));
     stopLoss[i] = StrToDouble(FileReadString(handle));
     takeProfit[i] = StrToDouble(FileReadString(handle));
     instanceID[i] = StrToInteger(FileReadString(handle));
     
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
     
     RefreshRates();
     isTradeOpen = true;
     
     if (   type[i] == OP_BUY && 
          ( 
            (MarketInfo(symbol[i],MODE_BID) <= stopLoss[i] && stopLoss[i] != 0) ||
            (MarketInfo(symbol[i],MODE_BID) >= takeProfit[i]&& takeProfit[i] != 0) 
           )
        )
        {
         savePositionToHistory(ticket[i], entryTime[i], TimeCurrent(), entryPrice[i], MarketInfo(symbol[i],MODE_BID), symbol[i], type[i], instanceID[i], volume[i]);
         isTradeOpen = false;
        }
        
     if (   type[i] == OP_SELL && 
          ( 
            (MarketInfo(symbol[i],MODE_ASK) >= stopLoss[i] && stopLoss[i] != 0)||
            (MarketInfo(symbol[i],MODE_ASK) <= takeProfit[i]&& takeProfit[i] != 0) 
           )
        )
        {
         savePositionToHistory(ticket[i], entryTime[i], TimeCurrent(), entryPrice[i], MarketInfo(symbol[i],MODE_ASK), symbol[i], type[i], instanceID[i], volume[i]);
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
int nOrderSend(string symbol, int type, double volume, double entryPrice, int slippage, double stoploss, double takeprofit, string comment, int instanceID, datetime expiration, color entryColor)
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderSend(symbol, type, volume, entryPrice, slippage, stoploss, takeprofit, comment, instanceID, expiration, entryColor ));

   int ticket = addToVitualOrders(symbol, type, volume, entryPrice, stoploss, takeprofit, instanceID);
   
   return(ticket); 

}

// syncs positioning so that net positioning matches virtual positioning
int syncPositioning()
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(SUCCESS);


int i = 0, j = 0;

string symbol[];
double volumeToOpen;
int ticket[], instanceID[], type[], entryTime[];
double stopLoss[], takeProfit[], entryPrice[], volume[], netVolume[], netRealVolume;
bool isTradeOpen;
int syncMutex = GlobalVariableGet("syncMutex");
int result ;

if (TimeCurrent()-syncMutex < MUTEX_SYNC_TIMEOUT && !IsTesting())
return(SUCCESS);

if (!IsTesting())
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
     
     ticket[i] = StrToInteger(FileReadString(handle));
     entryTime[i] = StrToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StrToInteger(FileReadString(handle));
     volume[i] = StrToDouble(FileReadString(handle));
     entryPrice[i] = StrToDouble(FileReadString(handle));
     stopLoss[i] = StrToDouble(FileReadString(handle));
     takeProfit[i] = StrToDouble(FileReadString(handle));
     instanceID[i] = StrToInteger(FileReadString(handle));
     
     i++;
     
     }
     FileClose(handle);
    }
    
   else
   
   {
   return(ERROR_OPENING_FILE);
   }
   
ArrayResize(netVolume, ArraySize(ticket));   

for( i = 0 ; i < ArraySize(ticket); i++)
     {
     
     netVolume[i] = 0;
     
     for( j = 0 ; j < ArraySize(ticket); j++)    
     { 
         if (symbol[i] == symbol[j])
         {
         
        // Print("Symbol ", symbol[j], " Volume ", volume[j], " type ", type[j]);
         
            if (type[j] == OP_BUY)
            netVolume[i] += volume[j];
         
            if (type[j] == OP_SELL)
            netVolume[i] -= volume[j];
            
         }
     }
     
     
     netRealVolume = 0;

        for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
         
            if (symbol[i] == OrderSymbol())
            {
               if (OrderType() == OP_BUY)
               netRealVolume += OrderLots() ;
               
               if (OrderType() == OP_SELL)
               netRealVolume -= OrderLots() ;
            }
         
         }
         
     //refreash rates to avoid requotes    
     RefreshRates();
     
     if (netRealVolume > netVolume[i] && netRealVolume > 0) 
     {
     
     for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
         
            if (symbol[i] == OrderSymbol() && OrderType() == OP_BUY)
            {
               if (OrderLots() > netRealVolume-netVolume[i] && netRealVolume != netVolume[i])
               {
                if (OrderCloseOK(OrderTicket(), (netRealVolume-netVolume[i]),  MarketInfo(symbol[i],MODE_BID), SLIPPAGE, ARROW_COLOR_BUY))
                netRealVolume -= netRealVolume-netVolume[i] ;
               }
               
               if (OrderLots() <= netRealVolume-netVolume[i] && netRealVolume != netVolume[i])
               {
                if (OrderCloseOK(OrderTicket(), OrderLots(),  MarketInfo(symbol[i],MODE_BID), SLIPPAGE, ARROW_COLOR_BUY))
                netRealVolume -= OrderLots() ;
               }
               
            }
         
         }
     }
         
     if (netRealVolume > netVolume[i] && netRealVolume <= 0) 
     {
     
     volumeToOpen = NormalizeDouble(MathAbs(netVolume[i]-netRealVolume),2);    
     
     if (volumeToOpen >= MarketInfo(Symbol(),MODE_MINLOT))    
     result = OrderSend(symbol[i], OP_SELL, volumeToOpen, MarketInfo(symbol[i],MODE_BID), SLIPPAGE, 0, 0, "", MASTER_INSTANCE_ID, 0, ARROW_COLOR_SELL) ;
   //  Print("trying");
     if (result > 0)
     netRealVolume -= netRealVolume-netVolume[i] ;
     }
         
     if (netRealVolume < netVolume[i] && netRealVolume < 0) 
     {
     
     for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
         
            if (symbol[i] == OrderSymbol() && OrderType() == OP_SELL)
            {
               if (OrderLots() > netVolume[i]-netRealVolume && netRealVolume != netVolume[i])
               {
                if (OrderCloseOK(OrderTicket(), (netVolume[i] - netRealVolume),  MarketInfo(symbol[i],MODE_ASK), SLIPPAGE, ARROW_COLOR_SELL))
                netRealVolume += (netVolume[i] - netRealVolume) ;
               }
               
               if (OrderLots() <= netVolume[i]-netRealVolume && netRealVolume != netVolume[i])
               {
                if (OrderCloseOK(OrderTicket(), OrderLots(),  MarketInfo(symbol[i],MODE_ASK), SLIPPAGE, ARROW_COLOR_SELL))
                netRealVolume += OrderLots() ;
               }
               
            }
         
         }
     }
         
     if (netRealVolume < netVolume[i] && netRealVolume >= 0) 
     {
     
    // Print( " Order ", i, " ticket ", ticket[i], " Open time ", entryTime[i], " symbol ", symbol[i], " type ", type[i], " volume ", MathAbs(netVolume[i]-netRealVolume)) ;
     volumeToOpen = NormalizeDouble(MathAbs(netVolume[i]-netRealVolume),2);    
     
     if (volumeToOpen >= MarketInfo(Symbol(),MODE_MINLOT))    
     result = OrderSend(symbol[i], OP_BUY, volumeToOpen, MarketInfo(symbol[i],MODE_ASK), SLIPPAGE, 0, 0, "", MASTER_INSTANCE_ID, 0, ARROW_COLOR_SELL) ;
     
   //  Print(GetLastError());
     
     if (result > 0)
     netRealVolume += MathAbs(netVolume[i]-netRealVolume) ;
     }
     
     }
     
     
     // now we need to cycle over open positions to sync the other side of the coin
     //
     //
     //
     //
     ///////////////
     
     double RealVolume[];
     string RealSymbols[];
     double netVolumeTemp = 0;
     string evaluatedSymbol;
     bool addSymbol;
     
     for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
            
            addSymbol = true ;
         
         for( i = 0 ; i < ArraySize(RealSymbols); i++) 
          {
            if ( RealSymbols[i] == OrderSymbol())
            addSymbol = false;
          }
          
          if (addSymbol)
          {
          ArrayResize(RealSymbols, ArraySize(RealSymbols)+1);
          RealSymbols[ArraySize(RealSymbols)-1] = OrderSymbol();
          }
         
         }
     
     ArrayResize(RealVolume, ArraySize(RealSymbols));
     
     for( i = 0 ; i < ArraySize(RealSymbols); i++)
     {
     
     evaluatedSymbol = RealSymbols[i] ;
     
     netVolumeTemp = 0;
     RealVolume[i] = 0;
     
     for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
         
            if (evaluatedSymbol == OrderSymbol())
            {
               if (OrderType() == OP_BUY)
               RealVolume[i] += OrderLots() ;
               
               if (OrderType() == OP_SELL)
               RealVolume[i] -= OrderLots() ;
            }
         
         }
     
     
     
     for( j = 0 ; j < ArraySize(ticket); j++)    
          {        
               if (symbol[j] == evaluatedSymbol && type[j] == OP_BUY)
               netVolumeTemp += volume[j] ;
               
               if (symbol[j] == evaluatedSymbol && type[j] == OP_SELL)
               netVolumeTemp -= volume[j] ;
            }
            
      
     //refreash rates to avoid requotes    
     RefreshRates();
     
     if (RealVolume[i]  > netVolumeTemp && RealVolume[i] > 0) 
     {
     
     for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
            
            //Print(evaluatedSymbol, "   ", OrderSymbol(), "   ", OrderType() == OP_BUY);
            //Print(RealVolume[i], "   ", netVolumeTemp);
         
            if (evaluatedSymbol == OrderSymbol() && OrderType() == OP_BUY)
            {
               if (OrderLots() > RealVolume[i] -netVolumeTemp && RealVolume[i]  != netVolumeTemp)
               {
                if (OrderCloseOK(OrderTicket(), (netRealVolume-netVolume[i]),  MarketInfo(evaluatedSymbol,MODE_BID), SLIPPAGE, ARROW_COLOR_BUY))
                RealVolume[i]  -= RealVolume[i] -netVolumeTemp ;
           
               }
               
               if (OrderLots() <= RealVolume[i] -netVolumeTemp &&RealVolume[i]  != netVolumeTemp)
               {
                if (OrderCloseOK(OrderTicket(), OrderLots(),  MarketInfo(evaluatedSymbol,MODE_BID), SLIPPAGE, ARROW_COLOR_BUY))
                RealVolume[i]  -= OrderLots() ;
               }
               
            }
         
         }
     }
        
         
     if (RealVolume[i]  < netVolumeTemp && RealVolume[i]  < 0) 
     {
     
     for( j = 0 ; j < OrdersTotal(); j++)    
        { 
            OrderSelect(j, SELECT_BY_POS, MODE_TRADES);
         
            if (evaluatedSymbol == OrderSymbol() && OrderType() == OP_SELL)
            {
               if (OrderLots() > netVolumeTemp-RealVolume[i]  && RealVolume[i]  != netVolumeTemp)
               {
                if (OrderCloseOK(OrderTicket(), (netVolumeTemp - RealVolume[i] ),  MarketInfo(evaluatedSymbol,MODE_ASK), SLIPPAGE, ARROW_COLOR_SELL))
                RealVolume[i]  -= (netVolumeTemp - RealVolume[i]) ;
               }
               
               if (OrderLots() <= netVolumeTemp-RealVolume[i]  && RealVolume[i]  != netVolumeTemp)
               {
                if (OrderCloseOK(OrderTicket(), OrderLots(),  MarketInfo(evaluatedSymbol,MODE_ASK), SLIPPAGE, ARROW_COLOR_SELL))
                RealVolume[i]  += OrderLots() ;
               }
               
            }
         
         }
     }
     
   //   Print("Final syncing on this round ", evaluatedSymbol , " Virtual Positioning ", netVolumeTemp, " net Real Positioning ", RealVolume[i] );
   
     }
     
     
return(SUCCESS);
}

// this function closes an order, it wraps the OrderClose function
bool nOrderClose(int ticketToClose, double volumeToClose, double orderClosePrice, int slippage, color closeColor)
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderClose(ticketToClose, volumeToClose, orderClosePrice, slippage, closeColor ));

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
     
     ticket[i] = StrToInteger(FileReadString(handle));
     entryTime[i] = StrToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StrToInteger(FileReadString(handle));
     volume[i] = StrToDouble(FileReadString(handle));
     entryPrice[i] = StrToDouble(FileReadString(handle));
     stopLoss[i] = StrToDouble(FileReadString(handle));
     takeProfit[i] = StrToDouble(FileReadString(handle));
     instanceID[i] = StrToInteger(FileReadString(handle));
     
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
     
     RefreshRates();
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
bool nOrderModify(int ticketToModify, double newEntryPrice, double newStopLoss, double newTakeProfit, datetime newExpiration, color modificationColor)
{

if (GlobalVariableGet("useOrderWrapper") != 1)
return(OrderModify(ticketToModify, newEntryPrice, newStopLoss, newTakeProfit, newExpiration, modificationColor));

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
     ArrayResize(type, i+1);
     ArrayResize(instanceID, i+1);
     ArrayResize(stopLoss, i+1);
     ArrayResize(takeProfit, i+1);
     ArrayResize(entryPrice, i+1);
     ArrayResize(volume, i+1);
     
     ticket[i] = StrToInteger(FileReadString(handle));
     entryTime[i] = StrToInteger(FileReadString(handle));
     symbol[i] = FileReadString(handle);
     type[i]   = StrToInteger(FileReadString(handle));
     volume[i] = StrToDouble(FileReadString(handle));
     entryPrice[i] = StrToDouble(FileReadString(handle));
     stopLoss[i] = StrToDouble(FileReadString(handle));
     takeProfit[i] = StrToDouble(FileReadString(handle));
     instanceID[i] = StrToInteger(FileReadString(handle));
     
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
     
     RefreshRates();
     
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

bool OrderCloseOK(int ticket, double lots, double price, int slippage, color Color=CLR_NONE) 
{
   double volumeToClose = NormalizeDouble(lots, 2);
   if (volumeToClose >= MarketInfo(Symbol(),MODE_MINLOT)) {
   if (!OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) return(FALSE);
   if (OrderCloseTime()>0) return(FALSE);
   return(OrderClose(ticket, volumeToClose, price, slippage, Color));
   }
   else return(FALSE);
}



