/**
 * @file
 * @brief     The Easy Trading library implements function wrappers to make coding with F4 more intuitive
 * @details   This library implements functions similar to those in MT4, for example you can get the value of open by using open(1) instead of calling the pParams structure
 *
 * @author    Daniel Fernandez (Initial implementation)
 * @author    Morgan Doel (Assisted with design and review)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2013
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

#ifndef EASY_TRADE_WRAPPER_HPP_
#define EASY_TRADE_WRAPPER_HPP_

#if _MSC_VER
#define snprintf _snprintf
#define fmax max
#define fmin min
#endif

typedef enum ratesIndexes_t
{
  PRIMARY_RATES = 0,
  DAILY_RATES   = 1,
  HOURLY_RATES = 2,
  FOURHOURLY_RATES = 3
} RatesIndexes;

#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
*This function initializes the Easy trade library
*
* The function sets the pParams static variable as
* well as the array of strings and array of double
* necessary to draw the trading UI.
*
* @param StrategyParams* pInputParams
*   StrategyParams structure that will be used by the library
*
* @return AsirikuyReturnCode
*/
AsirikuyReturnCode initEasyTradeLibrary(StrategyParams* pInputParams);

/**
* This functions returns the average of the range within a past period
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int period
*   range period to be used
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return double
*	the value of the range average as a double
* 
*/
double iRangeAverage(int ratesArrayIndex, int period, int shift);

AsirikuyReturnCode addNewRenkoRates(int originalRatesIndex, int ratesIndex, double renkoSize);

AsirikuyReturnCode addNewConstantVolumeRates(int originalRatesIndex, int ratesIndex, int volumeRequired);

tickData addTickArray();

AsirikuyReturnCode saveTickData();

/**
* Returns the total open order count for the running system.
*
* @return int
*	total number of open orders for this system
* 
*/
int totalOrdersCount();

/**
* Returns the total long open order count for the running system.
*
* @return int
*	total number of open long orders for this system
* 
*/
int buyOrdersCount();

/**
* Returns the total short order count for the running system.
*
* @return int
*	total number of short orders for this system
* 
*/
int sellOrdersCount();

/**
* This functions returns the average of the range of the past daily periods
* taking into account the high/low of the present daily bar up to the last closed
* bar.
*
* @param int period
*   range period to be used
*
* @return double
*	the value of the range average as a double
* 
*/
double iRangeSafeShiftZero(int period);

/**
* This functions returns the average true range of the past daily periods, 
* reconstructing daily bars using a lesser time frame. For this function each 
* day is formed using the past 24 bars from the last closed
* hour. This means that you will always lookback X 24 bar periods.
*
* @param int period
*   period to be used
*
* @return double
*	the value of the range average as a double
* 
*/
double iAtrSafeShiftZeroWholeDays(int period);


/**
* This functions returns the average true range of the past daily periods, 
* reconstructing daily bars using a lesser time frame. For this function each 
* day is formed using data from the first array (which must be a 1H time frame)
* data from only a selected block of hours is used to form each daily candle.
*
* @param int period
*   period to be used
*
* @param int firstHour
*   first hour of the block of hours to be included in the analysis
*
* @param int lastHour
*   last hour of the block of hours to be included in the analysis
*
* @return double
*	the value of the range average as a double
* 
*/
double iAtrDailyByHourInterval(int period, int firstHour, int lastHour);

/**
  * This functions adds a new daily rates array from the Yahoo financial
  * data repository. You specify the ticket of the instrument you want
  * to get, as well as the rateIndex you want to populate and the date
  * from where you want to add data. Rates are added up to currentBrokerTime.
  *
  * @param char* ratesName
  *   Ticket of the instrument to add. For example OIL for oil prices.
  *
  * @param time_t intFromDate
  *   Time from which you want to add daily rates. 
  *
  * @param int ratesIndex
  *   Index of the rates array you want to modify. It is normally > 2. 
  *
  * @return AsirikuyReturnCode
  *	returns SUCCESS if the rate index has been populated properly
  * 
  */
  AsirikuyReturnCode addNewDailyRates(char* ratesName, time_t intFromDate, int ratesIndex);

    /**
  * This functions adds a new daily rates array from the Quandl financial
  * data repository. You specify the dataset and ticker of the instrument you want
  * to get, as well as the rateIndex you want to populate and the date
  * from where you want to add data. Rates are added up to currentBrokerTime.
  *
  * @param char* dataset
  *   Quandl dataset where the instrument is located (AUTONOMY, NYSEARCA, etc)
  *
  * @param char* ratesName
  *   Ticket of the instrument to add. For example OIL for oil prices.
  *
  * @param time_t intFromDate
  *   Time from which you want to add daily rates. 
  *
  * @param int ratesIndex
  *   Index of the rates array you want to modify. It is normally > 2. 
  *
  * @return AsirikuyReturnCode
  *	returns SUCCESS if the rate index has been populated properly
  * 
  */
  AsirikuyReturnCode addNewDailyRatesQuandl(char* token, char* dataset, char* ratesName, time_t intFromDate, int ratesIndex);

    /**
  * This functions adds a new daily rates array from the Quandl financial
  * data repository. You specify the dataset and ticker of the instrument you want
  * to get, as well as the rateIndex you want to populate and the date
  * from where you want to add data. Rates are added up to currentBrokerTime. Note that you 
  * this function only populates the open value of the rates array (useful when getting data with only 1 value
  * for example interest or employment rates, which lack OHLCV).
  *
  * @param char* dataset
  *   Quandl dataset where the instrument is located (AUTONOMY, NYSEARCA, etc)
  *
  * @param char* ratesName
  *   Ticket of the instrument to add. For example OIL for oil prices.
  *
  * @param time_t intFromDate
  *   Time from which you want to add daily rates. 
  *
  * @param int ratesIndex
  *   Index of the rates array you want to modify. It is normally > 2. 
  *
  * @return AsirikuyReturnCode
  *	returns SUCCESS if the rate index has been populated properly
  * 
  */
  AsirikuyReturnCode addNewDailyRatesQuandlOpenOnly(char* token, char* dataset, char* ratesName, time_t intFromDate, int ratesIndex);

/**
* This functions returns the average true range of the past daily periods, 
* taking into account the high/low of the present daily bar up to the last closed
* bar.
*
* @param int period
*   period to be used
*
* @return double
*	the value of the range average as a double
* 
*/
double iAtrSafeShiftZero(int period);
/**
* Adds a value to the trading platforms UI.
*
* This functions writes an additional value to the
* static arrays of the EasyTrade library
* if there are more than 10 values the function returns false
*
* @param char* valueName
*   String value that will be assigned to the UI value
*
* @param double valueToAdd
*   Value to be added to the UI 
*
* @return BOOL
*   Returns whether or not addition was successful
*/
BOOL   addValueToUI(char* valueName, double valueToAdd);

/**
* returns opening time of a candle.
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return time_t
*   Returns candle opening time
*/
time_t openTime (int shift);

int findShift(int finalArrayIndex, int originalArrayIndex, int shift);

/**
* returns the open of a candle.
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle open value
*/
double cOpen (int shift);

/**
* returns the high of a candle.
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle high value
*/
double high (int shift);

/**
* returns the low of a candle.
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle low value
*/
double low  (int shift);


/**
* returns the tick volume of a candle.
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle volume value
*/
double volume  (int shift);

/**
* returns the close of a candle.
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle close value
*/
double cClose(int shift);

/**
* returns the close of a candle on a defined shift and rate Index.
*
* @param int rateIndex
*   Rate index used to find the candle
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle close value
*/
double iClose(int rateIndex, int shift);

double iVolume(int rateIndex, int shift);

/**
* returns the open of a candle on a defined shift and rate Index.
*
* @param int rateIndex
*   Rate index used to find the candle
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle open value
*/
double iOpen(int rateIndex, int shift);

/**
* returns the high of a candle on a defined shift and rate Index.
*
* @param int rateIndex
*   Rate index used to find the candle
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle high value
*/
double iHigh(int rateIndex, int shift);

/**
* returns the low of a candle on a defined shift and rate Index.
*
* @param int rateIndex
*   Rate index used to find the candle
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle low value
*/
double iLow(int rateIndex, int shift);

/**
* returns the opening time of a candle on a defined shift and rate Index.
*
* @param int rateIndex
*   Rate index used to find the candle
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle low value
*/
time_t iOpenTime(int rateIndex, int shift);

/**
* returns the range of a candle (|high-low|).
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle range value
*/
double range(int shift);

/**
* returns the body of a candle (open-close).
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle body value
*/
double body (int shift);

/**
* returns the absolute of the body of a candle (|open-close|).
*
* @param int shift
*   Shift of the candle requested relative to the 
*   end of the array (as in MT4)
*
* @return double
*   Returns candle body value
*/
double absBody (int shift);


/**
* returns the period of the primary rates array
*
* @return int
*   Returns the period value in minutes
*/
int Period();


/**
* returns the number of decimal points of the primary rates array
*
* @return int
*   Returns the number of decimal points
*/
int Digits();

/**
* Calculates a simplified ATR that approximates the daily ATR using lower time frames
* without the need to loop through daily spaces. Always calculated on shift 0 (safe).
*
* @param int period ATR
*   Period to be used for the calculation of this indicator.
*
* @return double
*   Returns the ATR proxy calculation
*/
double iAtrWholeDaysSimple(int rateIndex, int periodATR);

int getHourShift(int rateIndex, int shift);

/**
* Enters a single additional buy stop order. No action is taken in addition to this
*
* @param double entryPrice
*   desired entry price for the stop order.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double lotSize
*   order volume to be used. If set to 0 volume will be calculated using default risk management.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openSingleBuyStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize);

/**
* Enters a single additional buy limit order. No action is taken in addition to this
*
* @param double entryPrice
*   desired entry price for the stop order.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double lotSize
*   order volume to be used. If set to 0 volume will be calculated using default risk management.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openSingleBuyLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize,double risk);

/**
* Enters a single additional sell stop order. No action is taken in addition to this
*
* @param double entryPrice
*   desired entry price for the stop order.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double lotSize
*   order volume to be used. If set to 0 volume will be calculated using default risk management.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openSingleSellStopEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize);

/**
* Enters a single additional sell limit order. No action is taken in addition to this
*
* @param double entryPrice
*   desired entry price for the stop order.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double lotSize
*   order volume to be used. If set to 0 volume will be calculated using default risk management.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openSingleSellLimitEasy(double entryPrice, double takeProfit, double stopLoss, double lotSize,double risk);

  /**
  * Calculates a simplified ATR that approximates the daily ATR using lower time frames
  * without the need to loop through daily spaces. Allows for a calculation on a specified shift.
  *
  * @param int periodATR
  *   Period to be used for the calculation of this indicator.
  *
  * @param int shift
  *   Specified shift for the desired ATR calculation in number of bars in the past.
  *
  * @return double
  *   Returns the ATR proxy calculation
  */
  double iAtrWholeDaysSimpleShift(int rateIndex, int periodATR, int shift);

/**
* returns the hour value for the currently opened candle.
*
* @return int
*   Returns hour value
*/
int    hour();

/**
* returns the minute value for the currently opened candle.
*
* @return int
*   Returns the minute value
*/
int	   minute();

/**
* returns the day of the week value for the currently opened candle. 
* (0 = Sunday, 1 = Monday, etc)
*
* @return int
*   Returns the day of the week value
*/
int    dayOfWeek();

/**
* returns the month of the year value for the currently opened candle. 
*
* @return int
*   Returns the month value
*/
int    month();

/**
* returns the year value for the currently opened candle. 
*
* @return int
*   Returns the year value
*/
int    year();

/**
* returns the day of the year value for the currently opened candle. 
*
* @return int
*   Returns the day of the year value
*/
int    dayOfYear();

/**
* returns the day of the month value for the currently opened candle. 
*
* @return int
*   Returns the day of the month value
*/
int    dayOfMonth();

/**
* Returns a parameter from the pParams structure
* according to the selected index
*
* @return double
*   Returns requested parameter value
*/
double parameter(int parameterIndex);

/**
* This function prints a double value
* to the pantheious log
*
*/
void   print(double valueToPrint);

/**
* Checks orders to see if SL/TP values have been assigned 
* correctly (they are not zero) and assigns them if they have not.
* Please note that the EasyTrade library assumes internal TP and hard SL.
* This is done to preserve OpenPrice Only simulation compliance in MT4/5. 
*
* @param double takeProfit
*   takeprofit value to be assigned in case an order does not have it.
*
* @param double stopLoss
*   stoploss value to be assigned in case an order does not have it.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode   checkOrders(double takeProfit, double stopLoss);

/**
* This is a wrapper for the TA-lib ATR indicator that simplifies its use
* making it similar to the MQL4 function
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int period
*   ATR period to be used
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return AsirikuyReturnCode
* 
*/
double iAtr(int ratesArrayIndex, int period, int shift);

/**
* This is a wrapper for the TA-lib RSI indicator that simplifies its use
* making it similar to the MQL4 function
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int period
*   period to be used
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return AsirikuyReturnCode
* 
*/
double iRSI(int ratesArrayIndex, int period, int shift);

/**
* This is a wrapper for the TA-lib MA indicator that simplifies its use
* making it similar to the MQL4 function
*
*
* @param int type
*   selects the array to be used for the MA calculation (open=0, high=1, low=2, close=3).
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int period
*   period to be used
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return AsirikuyReturnCode
* 
*/
double iMA(int type, int ratesArrayIndex, int period, int shift);

/**
* This is a wrapper for the TA-lib stochastics oscillator indicator that simplifies its use
* making it similar to the MQL4 function
*
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int period
*   period to be used
*
* @param int k
*   K Period for the stochastics calculation
*
* @param int d
*   d period for the stochastics calculation
*
* @param int signal
*   indicates which indicator buffer to return (0=k, 1=d)
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return AsirikuyReturnCode
* 
*/
double iSTO(int ratesArrayIndex, int period, int k, int d, int signal, int shift);

double iSMI(int ratesArrayIndex, int period_Q, int period_R, int period_S, int signal, int shift);

/**
* This is a wrapper for the TA-lib MACD indicator that simplifies its use
* making it similar to the MQL4 function
*
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int fastPeriod
*   fast MA period to be used for the MACD calculation
*
* @param int slowPeriod
*   slow MA period to be used for the MACD calculation
*
* @param int signalPeriod
*   signal MA period to be used for the MACD calculation
*
* @param int signal
*   indicates which indicator buffer to return (0=MACD, 1=histogram, 2=signal)
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return AsirikuyReturnCode
* 
*/
double iMACD(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int signal, int shift);

double iMACDAll(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int shift, double *pMacd, double *pMmacdSignal, double *pMacdHist);

/**
  * This is a wrapper for the TA-lib                                                                       vgBollinger Band indicator that simplifies its use
  * making it similar to the MQL4 function
  *
  *
  * @param int ratesArrayIndex
  *   index of the rates array to use, selecting the timeframe.
  *
  * @param int bb_period
  *   The MA period used in the BB calculation.
  *
  * @param double bb_deviation
  *   Standard deviation multiplier to be used in the BB calculation.
  *
  * @param int signal
  *   indicates which indicator buffer to return (0=LOWER, 1=MIDDLE, 2=UPPER)
  *
  * @param int shift
  *   shift to be used for indicator calculation
  *
  * @return AsirikuyReturnCode
  * 
  */
  double iBBands(int ratesArrayIndex, int bb_period, double bb_deviation, int signal, int shift);

  double iBBandStop(int ratesArrayIndex, int bb_period, double bb_deviation, int * signal, double * stopPrice,int *index);

/**
  * This is a wrapper for the TA-lib standard deviation indicator that simplifies its use
  * making it similar to the MQL4 function. It returns the standard deviation of prices of chosen type.
  *
  * @param int ratesArrayIndex
  *   index of the rates array to use, selecting the timeframe.
  *
  * @param int period
  *   period to be used
  *
  * @param int type
  *   controls which values to use for std calculation (open=0, high=1, low=2, close=3)
  *
  * @param int shift
  *   shift to be used for indicator calculation
  *
  * @return AsirikuyReturnCode
  * 
  */
  double iStdev(int ratesArrayIndex, int type, int period, int shift);

/**
* This is a wrapper for the TA-lib CCI indicator that simplifies its use
* making it similar to the MQL4 function
*
* @param int ratesArrayIndex
*   index of the rates array to use, selecting the timeframe.
*
* @param int period
*   period to be used
*
* @param int shift
*   shift to be used for indicator calculation
*
* @return AsirikuyReturnCode
* 
*/
double iCCI(int ratesArrayIndex, int period, int shift);

/**
* Returns the spread value for the current symbol.
*
* @return double
*   Returns the current spread of the traded symbol.
*/
double spread();

/**
* Enters a short trade or - if a long trade is opened - closes this trade.
* the function assumes an open/close/update functionality in order to ensure
* trade chain independence. Please note that the EasyTrade library assumes internal TP and hard SL.
* This is done to preserve OpenPrice Only simulation compliance in MT4/5.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openOrUpdateShortEasy(double takeProfit, double stopLoss,double risk);

/**
* Enters a long trade or - if a short trade is opened - closes this trade.
* the function assumes an open/close/update functionality in order to ensure
* trade chain independence. Please note that the EasyTrade library assumes internal TP and hard SL.
* This is done to preserve OpenPrice Only simulation compliance in MT4/5.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openOrUpdateLongEasy(double takeProfit, double stopLoss,double risk);

AsirikuyReturnCode modifyTradeEasy_DayTrading(int orderType, int orderTicket, double stopLoss1, double stopLoss2, double takeProfit, int tpMode, time_t currentTime, double adjust, BOOL stopMovingbackSL);

AsirikuyReturnCode modifyTradeEasy_new(int orderType, int orderTicket, double stopLoss, double takeProfit, int tpMode, BOOL stopMovingbackSL);

AsirikuyReturnCode modifyAllOrdersOnSameDateEasy(int orderIndex, double stopLoss, double takePrice, BOOL stopMovingbackSL);

AsirikuyReturnCode closeWinningPositionsEasy(double total, double target);
AsirikuyReturnCode closeAllCurrentDayShortTermOrdersEasy(int tradeMode, time_t currentTime);

double caculateStrategyRiskEasy(BOOL isIgnoredLockedProfit);
double caculateStrategyPNLEasy(BOOL isIgnoredLockedProfit);
double caculateStrategyVolRiskEasy(double dailyATR);
double caculateStrategyVolRiskForNoTPOrdersEasy(double dailyATR);

double isSamePricePendingOrderEasy(double entryPrice, double limit);
double isSameDaySamePricePendingOrderEasy(double entryPrice, double limit, time_t currentTime);
double isSameWeekSamePricePendingOrderEasy(double entryPrice, double limit, time_t currentTime);
double isSameDaySamePricePendingOrderEasy_with_TP(double entryPrice, double limit, time_t currentTime);

double isSamePriceBuyLimitOrderEasy(double entryPrice, time_t currentTime,double gap);
double isSamePriceSellLimitOrderEasy(double entryPrice, time_t currentTime,double gap);
double isSamePriceBuyStopOrderEasy(double entryPrice, time_t currentTime,double gap);
double isSamePriceSellStopOrderEasy(double entryPrice, time_t currentTime,double gap);

int getSamePricePendingNoTPOrdersEasy(double entryPrice, double limit);
AsirikuyReturnCode validateHourlyBarsEasy(StrategyParams* pParams, int primary_rate, int hourly_rate);
AsirikuyReturnCode validateDailyBarsEasy(StrategyParams* pParams, int primary_rate, int daily_rate);
AsirikuyReturnCode validateSecondaryBarsEasy(StrategyParams* pParams, int primary_rate, int daily_rate, int secondary_tf, int rateErrorTimes);
AsirikuyReturnCode validateCurrentTimeEasy(StrategyParams* pParams, int primary_rate);

int getLossTimesInWeekEasy(time_t currentTime, double * total_lost_pips);
int getWinTimesInWeekEasy(time_t currentTime);

int getLossTimesInDayExcludeBreakeventOrdersEasy(time_t currentTime, double * total_lost_pips, double points);
int getLossTimesInDayEasy(time_t currentTime, double * total_lost_pips);
int getWinTimesInDayEasy(time_t currentTime);
int getLossTimesInDayCloseOrderEasy(time_t currentTime, double * total_lost_pips);

int getOrderCountTodayExcludeBreakeventOrdersEasy(time_t currentTime, double points);
int getOrderCountTodayEasy(time_t currentTime);
int getOrderCountForCurrentWeekEasy(time_t currentTime);
int getOrderCountEasy();

double caculateFreeMarginEasy();

int iMACDTrendBeiLiEasy(int ratesArrayIndex, int fastPeriod, int slowPeriod, int signalPeriod, int startShift, double macdLimit, OrderType orderType, int *pTruningPointIndex, double *pTurningPoint, int * pMinPointIndex, double *pMinPoint);

double adjustTakePrice_Weekly_Swing_Easy(int ratesArrayIndex, double init_takePrice);
/**
* Modifies the specified trade to change the takeProfit and stopLoss values
*
* @param int orderType
*   should contain the orderType of the order to be modified (BUY or SELL)
*
* @param int orderTicket
*   should contain the orderTicket value for the position to be modified (SELECT_ALL_TRADES modifies all positions)
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode modifyTradeEasy(int orderType, int orderTicket, double stopLoss, double takeProfit);

/**
* Enters a single additional short trade. No action is taken in addition to this
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double lotSize
*   order volume to be used. If set to 0 volume will be calculated using default risk management.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openSingleShortEasy(double takeProfit, double stopLoss, double lotSize,double risk);

/**
* Enters a single additional long trade. No action is taken in addition to this
*
* @param double takeProfit
*   takeprofit value to be assigned or updated.
*
* @param double stopLoss
*   stoploss value to be assigned or updated.
*
* @param double lotSize
*   order volume to be used. If set to 0 volume will be calculated using default risk management.
*
* @return AsirikuyReturnCode
* 
*/
AsirikuyReturnCode openSingleLongEasy(double takeProfit, double stopLoss, double lotSize,double risk);

AsirikuyReturnCode closeAllBuyLimitOrdersEasy(time_t currentTime);
AsirikuyReturnCode closeAllSellLimitOrdersEasy(time_t currentTime);
AsirikuyReturnCode closeAllBuyStopOrdersEasy(time_t currentTime);
AsirikuyReturnCode closeAllSellStopOrdersEasy(time_t currentTime);

AsirikuyReturnCode closeAllWithNegativeEasy(int tradeMode, time_t currentTime,int days);
AsirikuyReturnCode closeAllLimitAndStopOrdersEasy(time_t currentTime);

int getWinTimesInDaywithSamePriceEasy(time_t currentTime, double openPrice, double limit);
int getLossTimesInDaywithSamePriceEasy(time_t currentTime, double openPrice, double limit);

/**
* Closes all active long trades.
*
* @return AsirikuyReturnCode
*/
AsirikuyReturnCode closeAllLongs();

AsirikuyReturnCode closeAllLongTermLongsEasy();

/**
* Closes long with specified ticket
*
* @return AsirikuyReturnCode
*/
AsirikuyReturnCode closeLongEasy(int orderTicket);

/**
* Closes short with specified ticket
*
* @return AsirikuyReturnCode
*/
AsirikuyReturnCode closeShortEasy(int orderTicket);

/**
* Closes all active short trades.
*
* @return AsirikuyReturnCode
*/
AsirikuyReturnCode closeAllShorts();

AsirikuyReturnCode closeAllLongTermShortsEasy();

/**
* Displays total number of bars loaded.
*
* @param int ratesArrayIndex
*   index of the array where bars will be counted
*
* @return int
*   Returns the total number of bars on the history data
*/
int barsCount(int ratesArrayIndex);

/**
* Retrieve a pointer to the StrategyParams structure currently used by the easyTrade library.
*
* @return StrategyParams*
*   Returns a pointer to a StrategyParams structure.
*/
StrategyParams* getParams();

AsirikuyReturnCode iPivot(int ratesArrayIndex, int shift, double *pPivot, double *pS1, double *pR1, double *pS2, double * pR2, double *pS3, double *pR3);

AsirikuyReturnCode addDistortedArray(int sourceRateIndex, int ratesIndex, double maxDistorsion);
AsirikuyReturnCode freeRates(int ratesIndex);

int hasOpenOrder();
int hasSameDayOrderExcludeBreakeventOrdersEasy(time_t currentTime, BOOL * pIsOpen, double points);
int hasSameDayOrderEasy(time_t currentTime, BOOL *pIsOpen);
int hasSameWeekOrderEasy(time_t currentTime, BOOL *pIsOpen);
OrderType getLastestOpenOrderTypeEasy(int rateIndex, double *pHigh, double *pLow);
OrderType getLastestOpenOrderTypeEasy_GBPJPY(int rateIndex, double *pHigh, double *pLow);
OrderType getLastestOrderTypeEasy(int rateIndex, double *pHigh, double *pLow, BOOL * pIsOpen);
OrderType getLastestOrderTypeXAUUSDEasy(int rateIndex, double *pHigh, double *pLow, BOOL * pIsOpen);

double getLastestOrderPriceEasy(int rateIndex, BOOL * pIsOpen);
int getLastestOrderIndexEasy(int rateIndex);

int getLossTimesFromNowsEasy(time_t currentTime, double *pTotal_Lost_Pips);

int getLossTimesInPreviousDaysEasy(time_t currentTime, double *pTotal_Lost_Pips);
double hasSameDayDayTradingOrderEasy(int rateIndex, OrderInfo * pOrder, double * pHigh, double * pLow);

AsirikuyReturnCode getHighLowEasy(int ratesArrayIndex, int shfitIndex, int shift, double *pHigh, double *pLow);

int getSameSideTradesInCurrentTrendEasy(int rateIndex, OrderType type);
int getSameSideWonTradesInCurrentTrendEasy(int rateIndex, OrderType type);

double caculateStrategyWeeklyPNLEasy(time_t currentTime);

double iASIEasy(int ratesArrayIndex, int mode, int length, int smooth, double * outBull, double *outBear);

int getOldestOpenOrderIndexEasy(int rateIndex);

int getLastestOrderIndexExceptLimitAndStopOrdersEasy(int rateIndex, BOOL isClosedOnly);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EASY_TRADE_WRAPPER_HPP_ */
