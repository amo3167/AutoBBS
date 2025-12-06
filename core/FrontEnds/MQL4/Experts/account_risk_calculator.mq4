//+------------------------------------------------------------------+
//|                                                      my test.mq4 |
//|                        Copyright 2020, MetaQuotes Software Corp. |
//|                                             https://www.mql5.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2020, MetaQuotes Software Corp."
#property link      "https://www.mql5.com"
#property version   "1.00"
#property strict

input double   VirtualBalance=40000;      // File name 

struct CurrencyRisk
  {
   string symbol;     
   double longRiskInDollar;    
   double shortRiskInDollar;
  };


datetime LastActiontime;

CurrencyRisk riskByCurrency[15];

//+------------------------------------------------------------------+
//| Expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
  {
//---

   
//---
   return(INIT_SUCCEEDED);
  }
//+------------------------------------------------------------------+
//| Expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
  {
//---
   
  }
//+------------------------------------------------------------------+
//| Expert tick function                                             |
//+------------------------------------------------------------------+
void OnTick()
  {
   //Comparing LastActionTime with the current starting time for the candle
   if(LastActiontime!=Time[0]){
      //Code to execute once in the bar
      Print("This code is executed only once in the bar started ",Time[0]);
      
      calculateAccountRisk();
      
      LastActiontime=Time[0];
   }
  
//---
   
  }
  
  
void initializeCurrencyRiskArray() {
   riskByCurrency[0].symbol = "GBP";
   riskByCurrency[0].longRiskInDollar = 0;
   riskByCurrency[0].shortRiskInDollar = 0;
   riskByCurrency[1].symbol = "AUD";
   riskByCurrency[1].longRiskInDollar = 0;
   riskByCurrency[1].shortRiskInDollar = 0;
   riskByCurrency[2].symbol = "USD";
   riskByCurrency[2].longRiskInDollar = 0;
   riskByCurrency[2].shortRiskInDollar = 0;
   riskByCurrency[3].symbol = "XAU";
   riskByCurrency[3].longRiskInDollar = 0;
   riskByCurrency[3].shortRiskInDollar = 0;
   riskByCurrency[4].symbol = "XTI";
   riskByCurrency[4].longRiskInDollar = 0;
   riskByCurrency[4].shortRiskInDollar = 0;
   riskByCurrency[5].symbol = "JPY";
   riskByCurrency[5].longRiskInDollar = 0;
   riskByCurrency[5].shortRiskInDollar = 0;
   riskByCurrency[6].symbol = "EUR";
   riskByCurrency[6].longRiskInDollar = 0;
   riskByCurrency[6].shortRiskInDollar = 0;
   riskByCurrency[7].symbol = "CAD";
   riskByCurrency[7].longRiskInDollar = 0;
   riskByCurrency[7].shortRiskInDollar = 0;
   riskByCurrency[8].symbol = "SGD";
   riskByCurrency[8].longRiskInDollar = 0;
   riskByCurrency[8].shortRiskInDollar = 0;
   riskByCurrency[9].symbol = "NZD";
   riskByCurrency[9].longRiskInDollar = 0;
   riskByCurrency[9].shortRiskInDollar = 0;
   riskByCurrency[10].symbol = "CHF";
   riskByCurrency[10].longRiskInDollar = 0;
   riskByCurrency[10].shortRiskInDollar = 0;
   riskByCurrency[11].symbol = "NOK";
   riskByCurrency[11].longRiskInDollar = 0;
   riskByCurrency[11].shortRiskInDollar = 0;
   riskByCurrency[12].symbol = "XAG";
   riskByCurrency[12].longRiskInDollar = 0;
   riskByCurrency[12].shortRiskInDollar = 0;
   riskByCurrency[13].symbol = "XBR";
   riskByCurrency[13].longRiskInDollar = 0;
   riskByCurrency[13].shortRiskInDollar = 0;
   riskByCurrency[14].symbol = "BTC";
   riskByCurrency[14].longRiskInDollar = 0;
   riskByCurrency[14].shortRiskInDollar = 0;
}  
  
//+------------------------------------------------------------------+

void calculateAccountRisk() {
   initializeCurrencyRiskArray();

   int ordersTotal = OrdersTotal();
   if(ordersTotal > 0) {
      int ordersTotal = OrdersTotal();
      
      //ArrayResize(currencyRisks, ordersTotal, 0);
      
      //LOOK THROUGH ALL OPEN ORDERS
      for(int order_counter = 0; order_counter < OrdersTotal(); order_counter ++) {
         if(OrderSelect(order_counter, SELECT_BY_POS, MODE_TRADES)) {
            // 0 - buy
            // 1 - sell
            
            //todo should use price bid or price ask ?
            double priceBid=MarketInfo(OrderSymbol(),MODE_BID);
            
            string orderSymbol = OrderSymbol();
            double stopLoss = OrderStopLoss();
            double openPrice = OrderOpenPrice();
            double gap = 0;
            double riskInDollar = 0;
            double orderLot = OrderLots();
            int orderType = OrderType();
            
          if(orderType != OP_BUY && orderType != OP_SELL)
          {
            continue;
          }
            if (orderType == 0) {
               //Buy Order
               if (stopLoss > openPrice) {
                  //If SL is higher than open price, risk is zero
                  gap = 0;
               } else {
                  gap = openPrice - stopLoss;
               }
               
            }
            
            if (orderType == 1) {
               //Sell order
               if (stopLoss < openPrice) {
                  //If SL is below open price, risk is zero
                  gap = 0;
               } else {
                  gap = stopLoss - openPrice;
               }
               
            }
            
            int contractSize = (int)MarketInfo(OrderSymbol(), MODE_LOTSIZE);
            
             Print("contract size: " + contractSize);
            
            
            int factor = 100000;
            
            switch ((int)MarketInfo(OrderSymbol(), MODE_DIGITS)) {
               case 4:
                  factor = 10000;
                  break;
               case 3:
                  factor = 1000;
                  break;
               case 2:
                  factor = 100;
                  break;
               case 1:
                  factor = 10;
                  break;
               default: 
                  break;
            } 
            
            
            //if (StringFind(orderSymbol, "JPY", 0) > 0 || !isForexPair(orderSymbol)) {
               //If second currenty is JPY, then times 100
            //   factor = 100;
            //} 
            
            
            riskInDollar = MarketInfo(OrderSymbol(), MODE_TICKVALUE) * gap * factor * orderLot;
            
            double riskInPercentage = riskInDollar / (AccountBalance() + VirtualBalance);
            
            //Print("Order: " + OrderSymbol() + " " +  OrderType());
            OrderPrint();
            Print("Current Bid Price: " + priceBid);
            Print("Pip value:" + MarketInfo(OrderSymbol(), MODE_TICKVALUE));
            Print("Risk in Dollar: " + riskInDollar);
            Print("Risk in Percentage: " + riskInPercentage * 100 + "%");
            
            //Summarize risks per currency per direction
            addRiskToCurrencyRiskArray(orderSymbol, orderType, riskInDollar);
            
         }         
      }
      
      printRiskByCurrencyResult();
      
   }
}


void printRiskByCurrencyResult() {
      // Print out summarize account / currency risks
      string output;
      for (int i = 0; i<ArraySize(riskByCurrency); i++) {
         CurrencyRisk cr = riskByCurrency[i];
         if (cr.longRiskInDollar == 0 && cr.shortRiskInDollar == 0) {
            continue;
         }
         output = output + "\n\n" + cr.symbol + " risk Long: " + NormalizeDouble(cr.longRiskInDollar, 2) + " @ " + NormalizeDouble(cr.longRiskInDollar / (AccountBalance()+VirtualBalance) * 100, 2) + "%" + "  Short: " + NormalizeDouble(cr.shortRiskInDollar, 2) + " @ " + NormalizeDouble(cr.shortRiskInDollar /( AccountBalance()+VirtualBalance) * 100, 2) + "%";
         //Comment(cr.symbol + " Long risk: " + cr.longRiskInDollar + " Short risk: " + cr.shortRiskInDollar);
      }
      Comment(output);
}

void addRiskToCurrencyRiskArray(string symbol, int orderType, double riskInDollar) {
   for (int i=0; i<ArraySize(riskByCurrency); i++) {
   
      //CurrencyRisk cr = riskByCurrency[i];
      string firstCurrency = StringSubstr(symbol, 0, 3);
      string secondCurrency = StringSubstr(symbol, 3, 3);
      if (riskByCurrency[i].symbol == firstCurrency) {
         if (orderType == 0) {
            riskByCurrency[i].longRiskInDollar = riskByCurrency[i].longRiskInDollar + riskInDollar;
         } else {
            riskByCurrency[i].shortRiskInDollar = riskByCurrency[i].shortRiskInDollar + riskInDollar;
         }
      }
      
      if (riskByCurrency[i].symbol == secondCurrency) {
         if (orderType == 0) {
            riskByCurrency[i].shortRiskInDollar = riskByCurrency[i].shortRiskInDollar + riskInDollar;
         } else {
            riskByCurrency[i].longRiskInDollar = riskByCurrency[i].longRiskInDollar + riskInDollar;
         }
      }
   }
}


bool isForexPair(string symbol) {
   if (symbol == "XAUUSD" || symbol == "XTIUSD" || symbol == "XAUEUR") {
      return false;
   }   else {
      return true;
   }
} 


void doStuff() {
      Print(TimeCurrent());
      //Get account balance
      Print("Account balance = ",AccountBalance()+VirtualBalance);
      //Get current pip value
      Print("Symbol: " + Symbol());
      Print("Pip value: " + MarketInfo(Symbol(), MODE_TICKVALUE));
      Print("Lot Size: " + MarketInfo(Symbol(), MODE_LOTSIZE));
      
      //Print("indicator output - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 0, 1));
      //Print("indicator output - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 1, 1));
      //Print("indicator output - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 2, 1));
      //Print("indicator output - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 3, 1));
      //Print("indicator output - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 4, 1));
      //Print("indicator output - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 5, 1));
      Print("Bull - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 6, 2));
      Print("Bears - " + iCustom(Symbol(), PERIOD_D1, "absolute-strength-histogram", 0, 30, 1, 10, 0 ,3, 3, 7, 2));
}


