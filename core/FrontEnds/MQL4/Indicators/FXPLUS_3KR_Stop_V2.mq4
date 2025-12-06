//+------------------------------------------------------------------+
//|                                           FXPLUS_3KR_Stop_V2.mq4 |
//|                             Copyright 2013,FXPLUS Trading Academy|
//|                                                www.fxplus.com.au |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013,FXPLUS Trading Academy"
#property link      "www.fxplus.com.au"

#property indicator_chart_window
#property indicator_buffers 4

//--- input parameters
extern int WingDings_Symbol = 108; //arrow style for blue dot

//to allow buffers when setting stop limit, in pips
//for sell limit(long),should put limit-buffer;
//for buy limit(short),should put limit+buffer
extern double buffer; 

//for setting up Ask/Bid spread, in pips
//for sell limit(long),should put limit+spread;
//for buy limit(short), should put limit-spread
int spread = 0; 

//---- buffers
double Buffer1[]; 
double Buffer2[];
double Buffer3[];
double Buffer4[];

double currentHighest = 0;
double currentLowest = 0;

//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int init()
  {
//----
   SetIndexStyle(0,DRAW_ARROW,STYLE_SOLID,2,Purple);
   SetIndexBuffer(0,Buffer1);
   SetIndexLabel(0,"Purple cross");
   
   SetIndexStyle(1,DRAW_HISTOGRAM,STYLE_SOLID,2,Red);
   SetIndexBuffer(1,Buffer2);
   SetIndexLabel(1,"Red Bar");
   
   //for long
   SetIndexStyle(2,DRAW_ARROW,STYLE_DOT,0.5,Red);
   SetIndexBuffer(2,Buffer3);
   SetIndexLabel(2,"Red Dot");
   SetIndexArrow(2,WingDings_Symbol);
   
   //for short
   SetIndexStyle(3,DRAW_ARROW,STYLE_DOT,0.5,Blue);
   SetIndexBuffer(3,Buffer4);
   SetIndexLabel(3,"Blue Dot");   
   SetIndexArrow(3,WingDings_Symbol);
//----
   return(0);
  }
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
int deinit()
  {
//----

//----
   return(0);
  }

//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
int start()
  {
//----
      int periods=2; //only compare the previous two columns before the current column
      int longOrShort = -1; //-1 means not indicated yet;1 means long;2 means short
 
      double short_stop_buy=0;
      double long_stop_sell=0;
      double stop_position=0;
      
      for(int i=Bars-3;i>=0;i--){
         double k1_low = iLow(NULL,0,i+1); 
         double k1_high = iHigh(NULL,0,i+1);
         double k2_low = iLow(NULL,0,i+2);
         double k2_high = iHigh(NULL,0,i+2);
         double k0_low = iLow(NULL,0,i);
         double k0_high = iHigh(NULL,0,i);
         
         if(longOrShort == -1){
            currentHighest = 0.0;
            currentLowest = 0.0;
            longOrShort = checkDirection(i);
            
            //init highest and lowest value for the new deal
            if(longOrShort != -1) {
               currentHighest = k2_high;
               currentLowest = k2_low;
            }
         }
         
         stop_position = getStopPosition(longOrShort,i,stop_position);
         
         if((longOrShort == 1 && stop_position >= k0_low) || 
            (longOrShort == 0 && stop_position <= k0_high)){
            
            Buffer1[i] = stop_position;
            stop_position = 0.0;
            currentHighest = 0.0;
            currentLowest = 0.0;
            
            if(longOrShort != checkDirection(i)){
               //init highest and lowest value for the new deal
               currentHighest = k2_high;
               currentLowest = k2_low;
               
               longOrShort = checkDirection(i);
               stop_position = getStopPosition(longOrShort,i,stop_position);
            }else{
               longOrShort = -1;
            }
         }
         
         if(longOrShort == 1 && stop_position > 0){
            Buffer3[i] = stop_position;
         } else if(longOrShort == 0 && stop_position > 0){
            Buffer4[i] = stop_position;
         }
                   
      }
   
//----
   return(0);
  }
  
  double getStopPosition(int longOrShort, int shift, double originalPosition){
      double k1_low = iLow(NULL,0,shift+1); 
      double k1_high = iHigh(NULL,0,shift+1);
      double k1_open = iOpen(NULL,0,shift+1);
      double k1_close = iClose(NULL,0,shift+1);
      double k2_low = iLow(NULL,0,shift+2);
      double k2_high = iHigh(NULL,0,shift+2);
      double k2_open = iOpen(NULL,0,shift+2);
      double k2_close = iClose(NULL,0,shift+2);
      
      double stopPosition = originalPosition;
      
      if(longOrShort == -1){
         stopPosition = 0.0;
      } else if(longOrShort == 1){
         //to long, only shift stop position if 
         //1)the first previous column got higher value than currentHighest value and 
         //2)got higher value than the second previous column, 
         //set the sell limit using second previous column's lowest value            
         if(k1_high>currentHighest && k1_high>k2_high && k1_low>k2_low){
            currentHighest = k1_high;
            stopPosition = k2_low - buffer/10000.0 + spread/10000.0;          
         } else if(k1_high>currentHighest && k1_high>k2_high){
            currentHighest = k1_high;
            stopPosition = k1_low - buffer/10000.0 + spread/10000.0; 
         }
         //if current stop position is more or equal than the new position,reserve the current stop position
         if(originalPosition>0 && originalPosition>=stopPosition){
            stopPosition = originalPosition;
         } 
         
         //set lowest value
         if(k1_low<currentLowest)
            currentLowest = k1_low;
            
      } else if(longOrShort == 0){
         //to short, if the first previous column got lower value than the second previous column,
         //set the buy limit using second previous column's highest value
         if(k1_low<currentLowest && k1_low<k2_low && k1_high<=k2_high){
            currentLowest = k1_low;
            stopPosition = k2_high + buffer/10000.0 - spread/10000.0;
         } else if(k1_low<currentLowest && k1_low<k2_low){
            currentLowest = k1_low;
            stopPosition = k1_high + buffer/10000.0 - spread/10000.0; 
         }
         //if current stop position is less or equal than the new position,reserve the current stop position
         if(originalPosition>0 && originalPosition<=stopPosition){
            stopPosition = originalPosition;
         }
         
         if(k1_high>currentHighest){
            currentHighest = k1_high;
         }
      } 
      return(stopPosition);
  
  }
  
  int checkDirection(int barIndex){
      int longOrShort = -1;
      double k1_low = iLow(NULL,0,barIndex+1); 
      double k1_high = iHigh(NULL,0,barIndex+1);
      double k1_open = iOpen(NULL,0,barIndex+1);
      double k1_close = iClose(NULL,0,barIndex+1);
      double k2_open = iOpen(NULL,0,barIndex+2);
      double k2_close = iClose(NULL,0,barIndex+2);
      double k2_low = iLow(NULL,0,barIndex+2);
      double k2_high = iHigh(NULL,0,barIndex+2);
      
      if(k1_high>k2_high && k1_low>=k2_low) longOrShort = 1; //long
   //   else if(k1_high>k2_high && k1_close>k2_open) longOrShort = 1; //long
      
      if(k1_low<k2_low && k1_high<=k2_high) longOrShort = 0; //short
      
      return(longOrShort);
  } 
  
//+------------------------------------------------------------------+