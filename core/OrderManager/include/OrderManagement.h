/**
 * @file
 * @brief     Order management functions including order size and margin requirement calculations.
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

#ifndef ORDER_MANAGEMENT_H_
#define ORDER_MANAGEMENT_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int totalOpenOrders(StrategyParams* pParams, OrderType orderType);
int totalClosedOrders(StrategyParams* pParams, OrderType orderType);

/**
* Calculates the size of a new order.
*
* @param const StrategyParams* pParams
*   The strategy parameters necessary for calculating the order size.
*
* @param OrderType orderType
*   The type of order to be opened.
*
* @param double entryPrice
*   The new orders entry price.
*
* @param double stopLoss
*   The new orders stoploss in currency units. (not pips or points)
*
* @return double
*   The size of the new order in lots.
*/
double calculateOrderSize(const StrategyParams* pParams, OrderType orderType, double entryPrice, double stopLoss);

double calculateOrderSizeWithSpecificRisk(const StrategyParams* pParams, OrderType orderType, double entryPrice, double stopLoss, double risk);

/**
* Calculates the size of a new order.
*
* @param const StrategyParams* pParams
*   The strategy parameters necessary for calculating the required margin.
*
* @param OrderType orderType
*   The type of order to be opened.
*
* @param double lotSize
*   The lot size of the order to be opened.
*
* @return BOOL
*   TRUE if there is enough free margin to safely open a new trade; FALSE if there isn't.
*/
BOOL isEnoughFreeMargin(const StrategyParams* pParams, OrderType orderType, double lotSize);

/**
* Checks if any trades have exceeded their stop loss and generates an exit signal.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double internalSL
*   The value of the internalSL in currency units. In other words, the difference between the
*   order open/update price and the SL price.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode checkInternalSL(StrategyParams* pParams, int ratesIndex, int resultsIndex, double internalSL);

/**
* Checks if any trades have exceeded their take profit and generates an exit signal.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode checkInternalTP(StrategyParams* pParams, int ratesIndex, int resultsIndex, double internalTP);

/**
* Checks if any trades have exceeded their maximum time and generates an exit signal.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @param BOOL usingInternalSL
*   TRUE if the strategy uses internal stoploss. Otherwise FALSE.
*
* @param BOOL usingInternalTP
*   TRUE if the strategy uses internal takeprofit. Otherwise FALSE.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode checkTimedExit(StrategyParams* pParams, int ratesIndex, int resultsIndex, BOOL usingInternalSL, BOOL usingInternalTP);

/**
* Generates a trading signal to close any long trades.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int resultsIndex
*   Index of the results array.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode closeLongTrade(StrategyParams* pParams, int resultsIndex);

/**
* Generates a trading signal to close any short trades.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int resultsIndex
*   Index of the results array.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode closeShortTrade(StrategyParams* pParams, int resultsIndex);

/**
* If a long trade already exists it is updated. Otherwise a new long trade is opened.
* A trading signal to close any short trades is generated and any close signals for long
* trades are removed.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double stopLoss
*   The value of the stop loss in currency units. In other words, the difference between the
*   order open/update price and the SL price.
*
* @param double takeProfit
*   The value of the take profit in currency units. In other words, the difference between the
*   order open/update price and the TP price.
*
* @param BOOL useInternalSL
*   If true then use an internal SL. Otherwise just use a broker SL.
*
* @param BOOL useInternalTP
*   If true then use an internal TP. Otherwise just use a broker TP.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode openOrUpdateLongTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit,double risk, BOOL useInternalSL, BOOL useInternalTP);

/**
* If a short trade already exists it is updated. Otherwise a new short trade is opened.
* A trading signal to close any long trades is generated and any close signals for short
* trades are removed.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double stopLoss
*   The value of the stop loss in currency units. In other words, the difference between the
*   order open/update price and the SL price.
*
* @param double takeProfit
*   The value of the take profit in currency units. In other words, the difference between the
*   order open/update price and the TP price.
*
* @param BOOL useInternalSL
*   If true then use an internal SL. Otherwise just use a broker SL.
*
* @param BOOL useInternalTP
*   If true then use an internal TP. Otherwise just use a broker TP.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode openOrUpdateShortTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, double risk,BOOL useInternalSL, BOOL useInternalTP);

/**
* Helps you check whether the SL and TP of a trade are set properly
* 
* @param StrategyParams* pParams
* contains the order structure to evaluate
*
* @param stopLoss double
* is a stopLoss used (not checked if zero)
*
* @param takeProfit double
* is a takeProfit used (not checked if zero)
*
* @return BOOL
*   TRUE if orders are correct. FALSE if orders are not correct.
*/
BOOL areOrdersCorrect(StrategyParams* pParams, double stopLoss, double takeProfit);

/**
* Sets SL and TP broker variables to use within trades
* 
* @param StrategyParams* pParams
*   Contains the order structure to evaluate
*
* @param int ratesIndex
*   Index of the rates array
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double stopLoss
*   StopLoss value in absolute price units to be used
*
* @param double takeProfit
*   TakeProfit value in absolute price units to be used
*
* @param BOOL userInternalSL
*   Tells the function if an internalSL is used
*
* @param BOOL userInternalTP
*   Tells the function if an internalTP is used
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode setStops(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, BOOL useInternalSL, BOOL useInternalTP);

/**
* Calculates the age of a currently open trade from the last entry signal type modification 
* 
* @param StrategyParams* pParams
* contains the order structure to evaluate
*
* @param int ratesIndex
* rate index value
*
*/
int getOrderAge(StrategyParams* pParams, int ratesIndex);

/**
* Sets a trailing stop for all open trades for a given instance ID. This function is open price safe since it only does
* trailing comparisons on new bar open (using the opening value). This function also properly takes into account new entry
* prices if new trading signals in the same direction happen. 
* 
* @param StrategyParams* pParams
* contains the order structure to evaluate
*
* @param int ratesIndex
* rate index value
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double trailStart
* distance price has to move in our favour before trailing starts
*
* @param double trailDistance
* distance between new SL and current price 
*
* @param BOOL userInternalSL
* tells the function if an internalSL is used
*
* @param BOOL userInternalTP
* tells the function if an internalTP is used
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode trailOpenTrades(StrategyParams* pParams, int ratesIndex, double trailStart, double trailDistance, BOOL useInternalSL, BOOL useInternalTP);

/**
* If a long trade already exists it is updated with new SL and TP values. Note that this updating does not update the
* update value saved within the states.bin file because this update procedure does not correspond to a new entry signal (for this call openOrUpdate)
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double stopLoss
*   The value of the stop loss in currency units. In other words, the difference between the
*   order open/update price and the SL price.
*
* @param double takeProfit
*   The value of the take profit in currency units. In other words, the difference between the
*   order open/update price and the TP price.
*
* @param BOOL useInternalSL
*   If true then use an internal SL. Otherwise just use a broker SL.
*
* @param BOOL useInternalTP
*   If true then use an internal TP. Otherwise just use a broker TP.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode updateLongTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, BOOL useInternalSL, BOOL useInternalTP);

/**
* If a short trade already exists it is updated with new SL and TP values. Note that this updating does not update the
* update value saved within the states.bin file because this update procedure does not correspond to a new entry signal (for this call openOrUpdate)
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The index of the strategy's main rates. This is usually 0.
*
* @param int resultsIndex
*   Index of the results array.
*
* @param double stopLoss
*   The value of the stop loss in currency units. In other words, the difference between the
*   order open/update price and the SL price.
*
* @param double takeProfit
*   The value of the take profit in currency units. In other words, the difference between the
*   order open/update price and the TP price.
*
* @param BOOL useInternalSL
*   If true then use an internal SL. Otherwise just use a broker SL.
*
* @param BOOL useInternalTP
*   If true then use an internal TP. Otherwise just use a broker TP.
*
* @return enum AsirikuyReturnCode
*   An enum indicating success or the type of failure that occured.
*/
AsirikuyReturnCode updateShortTrade(StrategyParams* pParams, int ratesIndex, int resultsIndex, double stopLoss, double takeProfit, BOOL useInternalSL, BOOL useInternalTP);

/**
* Returns the elliptical stoploss value for a given position target (takeProfit) with a given expected holding time.
* The elliptical SL assumes that the SL should be tight at first, then relaxed to account for possible random
* walk volatility and then tight again due to a lower probability to reach a favorable outcome.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param double takeProfit
*   The value of the take profit in currency units. In other words, the difference between the
*   order open/update price and the TP price.
*
* @param double maxHoldingTime
*   The maximum number of bars you intend to hold this position.
*
* @param double z
*   Functional confidence parameter for the elliptical stopLoss, a value of 2 gives about 98% confidence.
*
* @param double orderBarsAge
*   The number of bars that have happened since the position was opened (use the getOrderAge function to obtain this value)
*
*/
double CalculateEllipticalStopLoss(StrategyParams* pParams, double takeProfit, int maxHoldingTime, double z, int orderBarsAge);


/**
* Returns the parabolic takeprofit value for a given position target (takeProfit) with a given expected holding time.
* The parabolic TP assumes that the TP should be tighter at first, then relaxed to account for possible random
* walk volatility and then tight again due to a lower probability to reach a favorable outcome.
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param double takeProfit
*   The value of the take profit in currency units. In other words, the difference between the
*   order open/update price and the TP price.
*
* @param double maxHoldingTime
*   The maximum number of bars you intend to hold this position.
*
* @param double z
*   Functional confidence parameter for the elliptical stopLoss, a value of 2 gives about 98% confidence.
*
* @param double orderBarsAge
*   The number of bars that have happened since the position was opened (use the getOrderAge function to obtain this value)
*
*/
double CalculateEllipticalTakeProfit(StrategyParams* pParams, double takeProfit, int maxHoldingTime, double z, int orderBarsAge);


/**
* Returns the age of a position (in bars) based on its trading ticket and its order entry time. 
*
* @param const StrategyParams* pParams
*   The strategy parameters
*
* @param int ratesIndex
*   The rate index of the rates array used to count bars towards the position age
*
* @param int position
*   Ticket of the position to be evaluated
*
*/
int getOrderAgeByPosition(StrategyParams* pParams, int ratesIndex, int position);


double maxLossPerLot(const StrategyParams* pParams, OrderType orderType, double entryPrice, double stopLoss);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ORDER_MANAGEMENT_H_ */

